Feature List per Board {#feature-list}
----

Table: Overview of the currently supported features for every board.


| Feature      |   MCIMX8M-EVK     |
|:-------------|:-----------------:|
| BSP name     | NXPEVK_IMX8M_4GB  |
| SD Card boot |         Y         |
| eMMC boot    |         Y         |
| Audio        |         Y         |
| GPIO         |         Y         |
| I2C          |         Y         |
| Ethernet     |         Y         |
| PWM          |         Y         |
| SD Card      |         Y         |
| eMMC         |         Y         |
| SPI (master) |        N/A        |
| Display      |         Y         |
| UART         |         Y*        |
| USB (host)   |         Y         |
| PCIe         |         Y         |
| TrEE         |         Y         |
| M4           |        N**        |
| Processor PM |         Y         |
| Device PM    |        N**        |
| LP standby   |        N**        |
| Display PM   |         Y         |
| DMA          |         Y         |

| Legend | Meaning                                                                       |
|:-------|:------------------------------------------------------------------------------|
| Y      | Enabled                                                                       |
| Y*     | To enable the UART, the kernel debugger must be disabled by running the following command on the device and rebooting. The UART exposed is the same UART that the kernel debugger uses. `bcdedit /debug off` |
| N/A    | Feature not applicable                                                        |
| N*     | Feature not enabled - feature is not available in default board configuration |
| N**    | Feature not enabled - feature is not currently supported                      |
| PM     | Power management                                                              |
| LP     | Low power                                                                     |

Not all features of a given subsystem maybe fully enabled and/or optimized. If you encounter issues with supported features, please open an issue.
