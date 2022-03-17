Basic Terminal Setup {#terminal-setup}
----
During the boot, you can check the U-Boot and UEFI firmware output on the host PC by using the serial interface console.
In the case you don't see any output on the connected display, for example, this might be helpful to confirm that the board is booting.
Common serial communication applications such as HyperTerminal, Tera Term, or PuTTY can be used on the host PC side.
The i.MX boards connect the host driver using the micro-B USB connector.

1. Connect the target and the PC using a cable mentioned above.
2. Open terminal on the PC. Configure serial to 921600 baud/s, 8 bit, one stop bit.

You may need to download drivers for USB to serial converter.
The USB to serial driver for FTDI chip can be found under [http://www.ftdichip.com/Drivers/VCP.htm](http://www.ftdichip.com/Drivers/VCP.htm).
The FTDI USB to serial converters provide up to four serial ports.
The USB to serial VCP (Virtual COM Port) driver for CP210x chip can be found under [https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers). 
The CP210x USB to serial converters provide up to two serial ports.

The order of serial ports differ between boards sometimes even Windows hosts differ in COM port order.
For MCIMX8M-EVK board user need to select the first port (COM) in terminal to read bootloader messages or debug kernel with WinDbg.
