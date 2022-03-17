Deploy Windows IoT image to development board {#deploy-bsp}
====
This chapter will guide you through the deployment process of Windows operating system and boot firmware to the development board.

## Hardware prerequisites
  * NXP i.MX development board (additional information can be found in Supported boards table in i.MX_Win10_Release_Notes.pdf)
  * USB-C cable to connect from device to PC
  * microSD card (minimum 8GB)
 
## Software prerequisites
  * Binary drivers (either downloaded from NXP.com or built locally)
  * Windows IoT Enterprise version 2004 ARM64 OS image.
    [https://partner.microsoft.com/en-us/dashboard/collaborate/packages/12392]( https://partner.microsoft.com/en-us/dashboard/collaborate/packages/12392)
  * Windows ADK  1809. If you are not building firmware on the same PC, you can install Windows ADK for Windows 10, version 2004. [https://docs.microsoft.com/en-us/windows-hardware/get-started/adk-install](https://docs.microsoft.com/en-us/windows-hardware/get-started/adk-install)
  * Windows PE add-on for the ADK, version 2004. Regardless of whether you installed ADK version 1809 or 2004, you should install version 2004 of WinPE.
    [https://docs.microsoft.com/en-us/windows-hardware/get-started/adk-install](https://docs.microsoft.com/en-us/windows-hardware/get-started/adk-install)
