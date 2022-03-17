Known Issues/Limitations
====

Read through all hardware-related reference material and ensure that you have made all the necessary hardware modifications before using the software.

The imx-windows-bsp reference BSP has the following limitations:
 - SD/eMMC is the only supported boot media


Table: Common known issues and workarounds for i.MX Family SoC

| SoC      | Module     | Source   | Description      | Workaround       |
|:---------|:-----------|:---------|:-----------------|:-----------------|
| All SoC  | Boot             | Software | In case multiple SD cards marked as bootable are inserted before power on target might fail to boot.                                                                                          | It is recommended to newly format the SD card using Diskpart.       |
| All SoC  | UEFI/Boot        | Software | Build script/make files may generate incorrect binaries if clean build is not used.                                                                                                           | Use ./buildme64.sh script with "-c" parametr                        |
| All SoC  | USB              | Software | Only USB HOST mode is supported.                                                                                                                                                              | No workaround                                                       |
| All SoC  | VS2017 build     | Software | Build of iMXPlatform.sln fails if there are spaces in project path.                                                                                                                           | Place the imx-windows-iot directory in way its path doesn't contain spaces. |
| All SoC  | WSL build        | Software | On older versions of Windows the OP-TEE Trusted OS build might fail inside WSL environment if the BSP is located in Windows file system.                                                      | Move sources to WSL root drive (eg. $HOME).                         |
| All SoC  | Drivers          | Software | NXP precompiled drivers are signed by NXP test certificates. Some windows features may not work correctly.                                                                                    | Sign drivers by certificate provided by Software Publisher Certificate (SPC) from a commercial CA.|
| All SoC  | HAL Drivers      | Software | The HAL Extensions must be signed be certificates provided by Microsoft. Unfortunately required cerificates that are included in WDK 1809 have expired.                                       | Download the Microsoft Kits Package from Collaborate and use the “Windows OEM HAL Extension Test Cert 2017 (TEST ONLY)” and “Windows OEM Intermediate 2017 (TEST ONLY)” found in the EWDK.iso file or contact Microsoft for help. |
| All SoC  | Drivers          | Software | The sdport.sys in the recommended Windows build W10-19044.1288.211006 could cause the blue bug check screen. This could be observed when encrypting volumes with Bitlocker or using Cfimager. | Patch the operating system with newer sport.sys included in bsp. When deploying bsp use `/patch_sdport` option as described in [Install Windows IOT Enterprise to eMMC]{#deploy-os-winpe} chapter. |
| i.MX 8MQ | Power management | Software | Device is unable to enter sleep and then wakeup. Device that is left unattended for longer period of time might hang.                                                                         | Disable sleep.                                                      |
| i.MX 8MQ | Power management | Software | Windows may hang if SOC temperature reaches PSV level.                                                                                                                                        | Do not rise CPU and GPU clock. Do not lower temperature limit.      |
| i.MX 8MQ | UART             | Software | The RTS-CTS hardware flow control is not available.                                                                                                                                           | No workaround                                                       |
| i.MX 8MQ | Display          | Software | Display driver supports 1280 x 720 60 Hz resolution only.                                                                                                                                     | No workaround                                                       |
| i.MX 8MQ | Display          | Software | The driver only supports HDMI monitor.                                                                                                                                                        | No workaround                                                       |
| i.MX 8MQ | Display          | Software | Monitor power-off is emulated by displaying a blank image.                                                                                                                                    | No workaround                                                       |
| i.MX 8MQ | GPU              | Software | The driver doesn’t support D3D9, so WPF (Windows Present Foundation) will not be accelerated.                                                                                                 | No workaround                                                       |
