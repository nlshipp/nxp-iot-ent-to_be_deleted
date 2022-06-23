Known Issues and Limitations
====

Read through all hardware-related reference material and ensure that you have made all the necessary hardware modifications before using the software.

### Common known issues and workarounds for all supported i.MX Family SoC:
  - Boot
      - **Limitation**: In case multiple SD cards marked as bootable are inserted before power on target might fail to boot.

        **Workaround**: It is recommended to newly format the SD card using Diskpart.

      - **Limitation**: SD/eMMC is the only supported boot media

        **Workaround**: No workaround.
  - USB
      - **Limitation**: Only USB HOST mode is supported.

        **Workaround**: No workaround.
  - VS2017 build
      - **Limitation**: Build of iMXPlatform.sln fails if there are spaces in project path.

        **Workaround**: Place the imx-windows-iot directory in way its path doesn't contain spaces.
  - WSL build
      - **Limitation**: On older versions of Windows the OP-TEE Trusted OS build might fail inside WSL environment if the BSP is located in Windows file system.

        **Workaround**: Move sources to WSL root drive (eg. $HOME).
  - HAL Drivers
      - **Limitation**: The HAL Extensions must be signed be certificates provided by Microsoft. Unfortunately required cerificates that are included in WDK 1809 have expired.

        **Workaround**: Download the Microsoft Kits Package from Collaborate and use the "Windows OEM HAL Extension Test Cert 2017 (TEST ONLY)" and "Windows OEM Intermediate 2017 (TEST ONLY)" found in the EWDK.iso file or contact Microsoft for help.
  - Drivers
      - **Limitation**: The sdport.sys in the recommended Windows build 19044.1288.211006-0501.21h2_release could cause the blue bug check screen. This could be observed when encrypting volumes with Bitlocker or using Cfimager.

        **Workaround**: Patch the operating system with newer sport.sys included in bsp. When deploying bsp use `/patch_sdport` option as described in [Install Windows IOT Enterprise to eMMC]{#deploy-os-winpe} chapter. Please note that this driver is test signed and will cause blue screen in case of testsigning is disabled (`/test_signing` is not used). If testsigning is disabled do not use `/patch_sdport` and install KB5014023.
  - SDHC
      - **Limitation**: The imxusdhc.sys in crashdump mode doesn't read HW specific settings from ACPI so that these values are hardcoded in the SdhcSlotInitialize() method.

        **Workaround**: Keep these values synchronized with values in Dsdt-Sdhc.asl ACPI table.
  - TPM/BitLocker
      - **Limitation**: Windows does not start if BitLocker encrypted system drive (eMMC). There is failure on "Microsoft Windows Trusted Runtime Secure Service".

        **Workaround**: Patch firmware according to the following three steps. First apply patch: 'cd mu_platform_nxp/MU_BASECORE/ && git apply ../../patches/MU_BASECORE-TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE.patch && cd ../../'. Second build firmware: './buildme64.sh -b 8M -t all -c -fw -t secured_uefi', change '8M' for another SoC. See Building ARM64 Firmware chapter in User's Guide for more details. Third deploy the firmware, see Deploy boot firmware chapter in User's Guide.
  - signed_firmware
      - **Limitation**: Built signed_firmware.bin does not verify the FIT image signed.

        **Workaround**: Add CONFIG_IMX_HAB=y and CONFIG_FIT_SIGNATURE=y into uboot-imx/configs/imx8mq_evk_nt_uuu_defconfig for i.MX8MQ or imx8mm_evk_nt_uuu_defconfig for i.MX8MM and build signed_firmware.bin using command ./buildme64.sh -b 8M -t all -fw -t secured_uefi -t sign_images for i.MX8MQ, use -b 8Mm for i.MX8MM. See Enabling Secure Boot chapter in User's Guide.
  - fTMP
      - **Limitation**: "Physical Presence" is not enabled in UEFI.

        **Workaround**: MU_BASECORE-TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE patch must be applied.
  - RPMB
      - **Limitation**: Product Key is not kept during re-installation.

        **Workaround**: RPMB must be cleared e.g. using the following firmware version. In buildme64.sh, update line for make optee with "CFG_RPMB_RESET_FAT=y" ("make -s -j12 PLATFORM=imx PLATFORM_FLAVOR=\$optee_plat CFG_TEE_CORE_DEBUG=n CFG_RPMB_RESET_FAT=y ...") and use "-t secured_uefi" parameter for buildme64.sh.
  - uSDHC
      - **Limitation**: SDCARD insertion/removal can cause bug check if the delay between insertion/removal is too short.

        **Workaround**: Wait a few seconds before insertion/removal.

### Common known issues and workarounds for i.MX8M SoC:
  - Power management
      - **Limitation**: Device is unable to enter sleep and then wakeup. Device that is left unattended for longer period of time might hang.

        **Workaround**: Disable sleep.
  - UART
      - **Limitation**: The RTS-CTS hardware flow control is not available.

        **Workaround**: No workaround.
  - Display
      - **Limitation**: Display driver supports 1280 x 720 60 Hz resolution only.

        **Workaround**: No workaround.
  - Display
      - **Limitation**: The driver only supports HDMI monitor.

        **Workaround**: No workaround.
  - Display
      - **Limitation**: Monitor power-off is emulated by displaying a blank image.

        **Workaround**: No workaround.
  - GPU
      - **Limitation**: The driver doesn't support D3D9, so WPF (Windows Present Foundation) will not be accelerated.

        **Workaround**: No workaround.
  - Camera
      - **Limitation**: Only i.MX 8MQ EVK SCH-29615 rev. B4 is supported. Earlier boards use different I2C for camera.

        **Workaround**: For i.MX 8MQ EVK SCH-29615 rev. B3 chose I2C1 camera device instead of I2C2 in `mu_platform_nxp/NXP/MCIMX8M_EVK_4GB/AcpiTables/Dsdt-Camera_Ov5640.asl`.
  - UEFI
      - **Limitation**: USB is not enabled in UEFI by default.

        **Workaround**: Add "BLD_\*_CONFIG_USB" :"TRUE" into the "default profile" in "win10-iot-bsp/mu_platform_nxp/NXP/MCIMX8M_EVK_4GB/PlatformBuild.py", line 34.

### Common known issues and workarounds for i.MX8M Nano/Mini SoCs:
  - Display
      - **Limitation**: MIPI-DSI display interface supported with fixed resolution 1920x1080 60Hz. IMX-MIPI-HDMI converter from MIPI-DSI to HDMI display supported.

        **Workaround**: No workaround.

### Common known issues and workarounds for i.MX8M Nano/Mini/Plus SoCs:
  - UEFI
      - **Limitation**: USB is not supported in UEFI.

        **Workaround**: Use on board serial port and console application in order to access UEFI menu.

      - **Limitation**: DisplayMonitor power-off is emulated by displaying a blank image.

        **Workaround**: No workaround.
  - GPU
      - **Limitation**: The driver doesn't support D3D9, so WPF (Windows Present Foundation) will not be accelerated.

        **Workaround**: No workaround.

### Common known issues and workarounds for i.MX8M Mini SoC:
 - Power management
      - **Limitation**: Device is unable to enter sleep and then wakeup. Device that is left unattended for longer period of time might hang.

        **Workaround**: Disable sleep.
 - Power management
      - **Limitation**: Thermal management is not supported.

        **Workaround**: No workaround.
 - UART
      - **Limitation**: The RTS-CTS hardware flow control is not available.

        **Workaround**: No workaround.

### Common known issues and workarounds for i.MX8M Plus SoC:
  - Display
      - **Limitation**: LVDS0 (4 lines) display interface supported with fixed resolution 1280x720 60Hz. IMX-LVDS-HDMI converter from LVDS to HDMI supported.

        **Workaround**: UEFI firmware can be re-compiled for LVDS dual (8 lines) or MIPI-DSI display interface support with fixed resolution up to 1920x1080 60 Hz.
  - USB
      - **Limitation**: The USB host controller reports 64 bit DMA capability but it supports 32 bits only.

        **Workaround**: Use only 3GB of SDRAM
  - VPU
      - **Limitation**: The VPU is not supported yet.

        **Workaround**: No workaround.
  - Ethernet
      - **Limitation**: The ethernet QoS(ENET0, J8) device is not supported yet. The FEC(ENET1, J9) device can be used instead.

        **Workaround**: No workaround.
