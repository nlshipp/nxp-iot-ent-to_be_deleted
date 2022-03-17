Deploy boot firmware {#deploy-firmware-uuu}
----

There is prebuilt boot firmware (`firmware.bin`) included in both binary and source BSP packages.
This boot firmware could be deployed to both SD card and onboard eMMC. Boot source is determined by DIP switches.

### Firmware on eMMC
For eMMC deployment we recommend to use UUU (Universal Update Utility tool for NXP IMX chips).
The UUU tool requires U-boot with UUU download support enabled. Thus the BSP ships with UUU enabled U-Boot that is not recommended for final designs.
To flash boot firmware to the eMMC we recommend to download UUU tool 1.3.191 which was used for BSP testing from https://github.com/NXPmicro/mfgtools/releases/tag/uuu_1.3.191.

To deploy boot firmware to eMMC via USB download mode follow steps below:
1. Navigate to `firmware` directory. In the source package it is located in `imx-windows-iot/BSP/firmware`.
2. Download UUU tool and put it into `firmware\tools\uuu\uuu.exe`.
3. Begin preparing board by powering it off.
4. Switch the target board to USB download mode using DIP switches.
5. Connect a USB-C cable between your desktop and the board’s USB-C port.
6. Power on the board and run `flash_bootloader.cmd /device NXPEVK_iMX8M_4GB` inside `firmware` directory.
7. When the process finishes power the board off. Disconnect the USB-C cable connecting board to PC. Not disconnecting from PC may cause errors in later steps. 
8. Switch the board to eMMC boot mode. See [Basic Board Setup][]

UUU documentation can be found at [https://github.com/NXPmicro/mfgtools](https://github.com/NXPmicro/mfgtools).

### Firmware on SD card
Firmware on SD card is recommended for development to avoid dip switch manipulation.
The boot firmware could be deployed to SD card containing WinPE based installer as described in [Install Windows IOT Enterprise to eMMC][].
However WinPE instaler wipes SD card. Thus WinPe must be deployed first.
To deploy boot firmware to SD card from Windows we recommend to use Cfimager tool from https://www.nxp.com/webapp/Download?colCode=CF_IMAGER.

1. Navigate to `firmware` directory. In the source package it is located in `imx-windows-iot/BSP/firmware`.
2. Download NXP cfimager tool and put it into `firmware/cfimager.exe` or system `%PATH%`.
3. Power on the board and run `flash_bootloader.cmd /device NXPEVK_iMX8M_4GB /target_drive <SD card letter eg. f:>` inside `firmware` directory.
4. Begin preparing board by powering it off.
5. Switch the target board to boot form SD card using DIP switches. See [Basic Board Setup][]
