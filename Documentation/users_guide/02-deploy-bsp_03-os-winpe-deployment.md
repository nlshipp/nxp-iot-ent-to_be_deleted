Install Windows IOT Enterprise to eMMC {#deploy-os-winpe}
----

Since eMMC is soldered to the board, the only way to write to it is to boot manufacturing OS on the device, then write the OS image to eMMC. The manufacturing OS is booted from a removable storage such as USB or SD. In this document, we will list commands for an MCIMX8M-EVK board to show an example of an ARM64 board deployment. The tools and techniques can be adapted to other hardware designs.

For the manufacturing OS, we will use [Windows PE (WinPE)](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/winpe-intro), which stands for Windows Preinstallation Environment. Windows PE is a small Windows image that can boot without persistent storage, and contains tools to help install Windows such as `diskpart` and `dism`.

The high-level process we will follow is:
 1. Prepare a microSD card with the OS image.
 2. Install the prepared OS image to eMMC through microSD card.

### Preparing WinPE based installation media on a microSD card

#### Prepare installation with prebuilt drivers
 1. Download W21H2-X.X.X-imx-windows-bsp-binaries.zip and extract it.
 2. Open elevated command prompt and navigate to extracted `<imx-windows-bsp-binaries>\IoTEntOnNXP`.
 3. Continue to [Common steps][]

#### Prepare installation building drivers from sources yourself
 1. Build the drivers yourself as described in [Building Windows 10 IoT for NXP i.MX Processors][].
 2. After the BSP has been built open elevated command prompt and navigate to `<BSP_DIR>\imx-windows-iot\BSP\IoTEntOnNXP`.
 3. Continue to [Common steps][]

#### Common steps
 1. Download the Windows IoT Enterprise version ISO mentioned in [Software prerequisites][].
 2. After the ISO file have been downloaded, mount the ISO file and copy out the install.wim from `<DVD mount drive:>\sources\install.wim` to the `IoTEntOnNXP` directory.
 3. In the `IoTEntOnNXP` directory run `make-winpe-enterprise.cmd` in elevated prompt.
    This will create copy of selected install.wim image with injected i.MX drivers.
    Use `/disable_updates` to save storage space and `/disable_transparency` to improve UI responsiveness.
    Use `/patch_sdport` to update Sdport.sys. This option should to enabled in case you are creating installer for the W10-19044.1288.211006 Windows build. Certain operations with Sdport.sys cause the blue bug check screen.
    The `/image <WIM_PATH>` could be either absolute or relative to `IoTEntOnNXP\work`. Work might not exist yet. The script will create it and then enter it.
    
    ```bash
    make-winpe-enterprise.cmd /device NXPEVK_iMX8M_4GB /disable_updates /disable_transparency /patch_sdport /image ..\install.wim
    ```
    
 4. Now you can deploy the WinPE image to an SD card by `make-winpe-enterprise.cmd /apply <disk_number>`.
    The `disk_number` of your SD card can be obtained using `diskpart` or using `wmic diskdrive list brief` from the DeviceID string (`\\.\PHYSICALDRIVE<disk_number>`).
    
    ```bash
    make-winpe-enterprise.cmd /device NXPEVK_iMX8M_4GB /apply <disk_number>
    ```
    
 5. WinPE based Windows installer is now deployed on the SD card.

    Installer also renames the `EFI` folder at the root of SD card to `_efi`, which causes UEFI to skip the SD card at boot time.
    Bootloader will find the `EFI` directory only on eMMC, and boot from there. If you wish to boot into the WinPE installer again, you can rename `_efi` back to `EFI`.

 6. After you deploy boot firmware (see [Deploy boot firmware][]) you can insert the prepared SD card installer into your development board and power it on.
    Board will boot into the installer. Installer will flash Windows IOT Enterprise to the eMMC, then reboot into installed Windows.
