BSP change history
====
This chapter lists changes in releases, including new features and defect fixes.

## 1/2022 : W0.9.0
Private preview release for i.MX8M platform.
  - Supported boards:
    :   The existing BSP with support for MCIMX8M-EVK NXP board.

## 3/2022 : W0.9.1
Public preview release for i.MX8M platform.
  - Fixes:
      - eMMC driver:
        :   eMMC tuning parameters add to the Dsdt-Sdhc.asl.
      - BSP deployment:
        :   Removed invalid characters from make-winpe-enterprise.cmd.

## 4/2022 : W1.0.0
Public release for i.MX8M and i.MX8M Mini platforms.
  - Supported boards:
    :   MCIMX8M-EVK evaluation kit
    :   8MMINILPD4-EVK evaluation kit
  - New features:
      - VPU driver:
	:   Supported codecs HEVC, VP9, H.264, VP8. MPEG-2 and MPEG-4 codecs supported on i.MX8M only.
  - Fixes:
      - UART driver:
        :   The UART driver failure during uninstallation in the Device manager has been fixed.
      - I2C driver:
        :   The issue in iMXI2cRead function (when ReadBufferSize == 1) in UEFI has been fixed.
      -  buildme 64.sh:
        :   The script has been updated. Updates in UEFI source code were included in firmware.bin only if firmware was built with -c parameter (clean build).
      -  PCIe:
        :   PCIE ATU (Address Translation Unit) setup for PCIE BAR memory mapped registers in UEFI drivers has been fixed. After this fix, the system works as expected in UEFI and relevant Storage drivers appears in Windows.

## 6/2022 : W1.1.0
Public release for i.MX8M Nano and i.MX8M Plus platforms.
  - Supported boards:
    :   MCIMX8M-EVK evaluation kit
    :   8MMINILPD4-EVK evaluation kit
    :   8MNANOD4-EVK evaluation kit
    :   8MPLUSLPD4-EVK evaluation kit
  - New features:
      - Camera driver:
        :   OV5640 camera in J1502 connector has been supported on i.MX8M EVK board.
      - FlexCAN driver:
        :   FlexCAN device has been supported on i.MX8M Plus EVK by the imxcan.sys driver.
      - I2C driver:
        :   I2C expander (PCA6416) has been supported in iMX8BoardInit module and options "SelectCAN1InsteadOfI2C5" and "SelectCAN2InsteadOfPDMStream3" allows to configure corresponding selectors on the Base Board.
      - GPU driver:
        :   GPU driver has been updated to v1.1.
      - Debug drivers:
        :   WinDbg over Ethernet has been supported. WinDbg over ethernet requires the kd_8003_1fc9.dll library which is not distributed as a part of the BSP. Please contact Microsoft to get this library.
      - ENET driver:
        :   HW checksum offload has been supported in the NDIS miniport driver.
  - Fixes:
      - Audio driver:
        :   A failure during uninstallation in the Device manager has been fixed.
      - Display driver:
		:   IMX-LVDS-HDMI and IMX-MIPI-HDMI converters: If a native HDMI display resolution exceeds the upper limit, the fixed maximum available resolution is set instead. 1920x1080 60Hz in case of IMX-MIPI-HDMI and 1280x720 60 Hz in case of IMX-LVDS-HDMI.
      - SD driver:
  	    :   Configuration "fixed device" has been changed to "removable device" which allows to safely remove the SD card by the "Eject" option. 

