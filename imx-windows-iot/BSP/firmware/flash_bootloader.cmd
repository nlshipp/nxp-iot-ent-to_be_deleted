@ECHO OFF
:: Copyright 2022 NXP 
:: All rights reserved. 
::  
:: Redistribution and use in source and binary forms, with or without modification, 
:: are permitted provided that the following conditions are met: 
::
:: Redistributions of source code must retain the above copyright notice, this list 
:: of conditions and the following disclaimer. 
::
:: Redistributions in binary form must reproduce the above copyright notice, this 
:: list of conditions and the following disclaimer in the documentation and/or 
:: other materials provided with the distribution. 
::
:: Neither the name of the copyright holder nor the names of its 
:: contributors may be used to endorse or promote products derived from this 
:: software without specific prior written permission. 
::
:: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
:: ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
:: WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
:: DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
:: ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
:: (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
:: LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
:: ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
:: (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
:: SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
:: 

::
:: Run this script in order to deploy boot firmware.
::

setlocal enableextensions
setlocal enabledelayedexpansion

:: Clear options
set DEVICE_TYPE=
set TARGET_DRIVE=
set UUU_LOAD2M=
set FIRMWARE_DIR=
set FIRMWARE=

:GETOPTS
 if /I "%~1" == "/?" goto USAGE
 if /I "%~1" == "/Help" goto USAGE
 if /I "%~1" == "/device" set DEVICE_TYPE=%2& shift
 if /I "%~1" == "/directory" set FIRMWARE_DIR=%2& shift
 if /I "%~1" == "/target_drive" set TARGET_DRIVE=%2& shift
 if /I "%~1" == "/firmware" set FIRMWARE=%2& shift
 shift
if not (%1)==() goto GETOPTS

if "%FIRMWARE_DIR%" == "" (
  set FIRMWARE_DIR=%DEVICE_TYPE%
)

if not exist %FIRMWARE_DIR% (
  echo %FIRMWARE_DIR% does not exist.
  exit /b 1
)

if "%FIRMWARE%" == "" (
    set BIN_COUNT=0
    for /f "tokens=*" %%i in ('dir !FIRMWARE_DIR!\*.bin /b') do (
        set /a BIN_COUNT+=1
    )
    if !BIN_COUNT! == 0 (
        echo No bin file was found in !FIRMWARE_DIR!\
        exit /b 1
    )

    if !BIN_COUNT! NEQ 1 (
        set BIN_COUNT=0
        echo Multiple bin files were found in  !FIRMWARE_DIR!\.
        echo Specify which firmware to use:
        for /f "tokens=*" %%i in ('dir !FIRMWARE_DIR!\*.bin /b') do (
            set /a BIN_COUNT+=1
            echo !BIN_COUNT!. %%i
        )
        set /p BIN_IDX=Enter index:
        if !BIN_IDX! gtr !BIN_COUNT! (
            echo Selected index is out of range
            exit /b 1
        )
        if !BIN_IDX! leq 0 (
            echo Selected index is out of range
            exit /b 1
        )
    ) else (
        set BIN_IDX=1
    )
        
    set BIN_COUNT=0
    for /f "tokens=*" %%i in ('dir !FIRMWARE_DIR!\*.bin /b') do (
        set /a BIN_COUNT+=1
        if !BIN_COUNT! == !BIN_IDX! (
            echo Selected: "%%i"
            echo:
            set FIRMWARE=!FIRMWARE_DIR!\%%i
        )
    )
    
)


if not exist !FIRMWARE! (
  echo !FIRMWARE! does not exist.
  echo Please verify you used correct 
  echo script with the /device flag.
  goto:eof
)

echo Device:    %DEVICE_TYPE%
echo Firmware:  !FIRMWARE!
echo.

if "%DEVICE_TYPE%" == "MX8M_EVK" (
  :: 66*512=0x8400
  set CF_IMAGER_OFFSET=0x8400
) else (
  if "%DEVICE_TYPE%" == "MX8M_MINI_EVK" (
    set CF_IMAGER_OFFSET=0x8400
  ) else (
    if "%DEVICE_TYPE%" == "MX8M_NANO_EVK" (
      :: 64*512=0x8000
      set CF_IMAGER_OFFSET=0x8000
      set UUU_LOAD2M=1
    ) else (
      if "%DEVICE_TYPE%" == "MX8M_PLUS_EVK" (
        set CF_IMAGER_OFFSET=0x8000
        set UUU_LOAD2M=1
      ) else (
        echo Error. The /device %DEVICE_TYPE% is not valid.
        goto:eof
      )
    )
  )
)

:: Write firmware to a drive plugged into PC using Cfimager. (https://www.nxp.com/webapp/Download?colCode=CF_IMAGER&appType=license&location=null)
:: Alternatively dd if="!FIRMWARE!" of="\\.\PhysicalDrive%DISK_NUM%" bs=512 seek=%SEEK% skip=0 .
if "%TARGET_DRIVE%"=="" (
  goto:FLASH_eMMC
)
   
cfimager.exe -raw -offset %CF_IMAGER_OFFSET% -f !FIRMWARE! -d %TARGET_DRIVE% || ^
echo Failed to write boot firmware to the "%TARGET_DRIVE%". Check the NXP cfimager.exe is available (eg. in path), TARGET_DRIVE is present and is not write protected.

goto:eof
:: End of SD_CARD_DOWNLOAD

:FLASH_eMMC
:: Write firmware to onboard memory.
SET device_mode=0

call:CHECK_DEVICE_MODE device_mode

IF "%device_mode%" == "none" (
	ECHO Device not in download mode
	ECHO Maybe device not connect with PC or device in normal mode.
	pause
	goto:eof
)



ECHO Device in %device_mode% download mode, start flash!!
ECHO.
if not "%UUU_LOAD2M%" == "" (
  tools\uuu\uuu.exe -b emmc !FIRMWARE! >nul 2>nul
  :: LIBUSB_ERROR_PIPE error workaround
  tools\uuu\uuu.exe FB: ucmd setenv fastboot_buffer ${loadaddr}
  tools\uuu\uuu.exe FB: download -f !FIRMWARE!
  tools\uuu\uuu.exe FB: ucmd mmc partconf 2 0 1 0
  tools\uuu\uuu.exe FB: ucmd mmc dev 2 1
  tools\uuu\uuu.exe FB: ucmd mmc write ${loadaddr} 0x0 0x1FC0
) else (
  tools\uuu\uuu.exe -b emmc !FIRMWARE!
)
pause
goto:eof

:CHECK_DEVICE_MODE
tools\uuu\uuu.exe -lsusb | findstr /r /c:"FB"
IF %ERRORLEVEL% == 0  (
  SET %~1=fastboot
  goto:eof
)

tools\uuu\uuu.exe -lsusb 2>&1 | findstr /r /c:"SDP"
IF %ERRORLEVEL% == 0  (
  SET %~1=serial
  goto:eof
)
SET %~1=none
goto:eof
:USAGE
echo flash_bootloader.cmd /device <NAME> [/target_drive]
echo.
echo Flashes a firmware image
echo Options:
echo    /device                      {MX8M_EVK, MX8M_MINI_EVK, MX8M_NANO_EVK, MX8M_PLUS_EVK}
echo                                 Specifies the device
echo.
echo Optional options:
echo    /target_drive                Specifies whether the firmware binary should be written to this drive
echo                                 instead of internal memory of the development board. Useful for
echo                                 firmware development.
echo                                 Caution, do not overwrite your system drive.
echo.  /directory                    Specifies directory in which firmware.bin is located.
echo   /firmware                     Specifies absolute or relative path to firmware.
echo Examples:
echo.
echo Flash the Windows Enterprise firmware to the MMC of a EVK board.
echo    flash_bootloader.cmd /device MX8M_EVK
echo.
echo Flash the Windows Enterprise firmware to an SD card.
echo    flash_bootloader.cmd /device MX8M_EVK /target_drive f:
echo.
