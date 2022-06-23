#define PRE_29615_B3 0

Device (CAM0) // AVStream
{
  Name (_HID, "NXP0C21")  // _HID: Hardware ID
  Name (_UID, "0")  // _UID: Unique ID
  Name (_CCA, 0x0)

#if PRE_29615_B3
  // Uncomment for SCH-29615 PDF: SPF-29615 B3
  Name (_DEP, Package() {\_SB.I2C1.SNS0, \_SB.CSI0, \_SB.MIP0})
#else
  // Uncomment for SCH-29615 PDF: SPF-29615 B4
  Name (_DEP, Package() {\_SB.I2C2.SNS0, \_SB.CSI0, \_SB.MIP0})
#endif

  Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
  {
    Name (SBUF, ResourceTemplate ()
    {
    })
    Return (SBUF) /* \_SB.CAM0._CRS.SBUF */
  } 

  Name (_DSD, Package () {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {       
        Package (2) {"DeviceEndpoint1", "CSI1"},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

Device (CSI0) // CSI on SoC peripheral
{
  Name (_HID, "NXP0C11")  // _HID: Hardware ID
  Name (_UID, "0")  // _UID: Unique ID
  Name (_CCA, 0x0)

  Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
  {
    Name (SBUF, ResourceTemplate ()
    {
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 74 }
      Memory32Fixed (ReadWrite, 0x30A90000, FixedPcdGet32 (PcdMipiCsiDeviceMemorySize), )
    })
    Return (SBUF) /* \_SB.CSI0._CRS.SBUF */
  }

  Name (_DSD, Package () {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"CoreClockFrequencyHz", 133000000},
        Package (2) {"Csi1RegResId", 0},
        Package (2) {"CsiTwo8bitSensorMode", 1},
        Package (2) {"DeviceEndpoint0", "CSI1"},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

Device (MIP0) // MIPI Hw (On SoC peripheral)
{
  Name (_HID, "NXP0C13")  // _HID: Hardware ID
  Name (_UID, "0")  // _UID: Unique ID
  Name (_CCA, 0x0)

  Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
  {
    Name (SBUF, ResourceTemplate ()
    {
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 76 }
      Memory32Fixed (ReadWrite, 0x30A80000, FixedPcdGet32 (PcdMipiCsiDeviceMemorySize), )
      Memory32Fixed (ReadWrite, 0x30A70000, FixedPcdGet32 (PcdMipiCsiDeviceMemorySize), )
    })
    Return (SBUF) /* \_SB.MIP0._CRS.SBUF */
  }
  
  Method (RGPR, 0, NotSerialized)
  {
    Return (GR34)
  }
  
  Method (WGPR, 1, NotSerialized)
  {
    Store (Arg0, GR34)
  }
  
#define MIPI_RCR_ESC_RESET_BIT (1u << 2)
#define MIPI_RCR_PHY_REF_RESET_BIT (1u << 1)
#define MIPI_RCR_CORE_RESET_BIT (1u << 0)
  Method (WRST, 1, NotSerialized)
  {
    Local0 = MI1P
    Local0 &= 0xFFFFFFF8
    
    if (Arg0 > 0)
    {
        Local0 |= MIPI_RCR_ESC_RESET_BIT | MIPI_RCR_PHY_REF_RESET_BIT | MIPI_RCR_CORE_RESET_BIT
    }

    Store (Local0, MI1P)
  }

  Name (_DSD, Package () {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"PhyClockFrequencyHz", 100000000},
        Package (2) {"EscClockFrequencyHz", 66666667},
        Package (2) {"Mipi1RegResId", 1},
        Package (2) {"DeviceEndpoint0", "MIPI1"},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

#if PRE_29615_B3
// Uncomment for SCH-29615 PDF: SPF-29615 B3
Scope (\_SB.I2C1)
{
    Device (SNS0)
    {
        Name (_HID, "NXP0C14")  // _HID: Hardware ID
        Name (_UID, "0")  // _UID: Unique ID
        Name (_CCA, 0x0)

        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Name (SBUF, ResourceTemplate ()
            {
                I2CSerialBus(0x3c, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C1")

                GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 3 } // 0 * 32 + 3
                GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 6 } // 0 * 32 + 6
            })
            Return (SBUF) /* \_SB.CAM0._CRS.SBUF */
        }
        Name (_DSD, Package () {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package () {
                    Package () {"DeviceEndpoint0", "SNS0"},
                    Package () {"GpioRstResId", 1},
                    Package () {"GpioPwnResId", 0},
                    Package () {"I2cResId", 0},
                    Package () {"CameraClockFrequencyHz", 24000000},
                }
        })
       
        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
            Return (0x0F)
        }
    }
}

#else
// Uncomment for SCH-29615 PDF: SPF-29615 B4
Scope (\_SB.I2C2)
{
    Device (SNS0)
    {
        Name (_HID, "NXP0C14")  // _HID: Hardware ID
        Name (_UID, "0")  // _UID: Unique ID
        Name (_CCA, 0x0)

        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Name (SBUF, ResourceTemplate ()
            {
                I2CSerialBus(0x3c, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C2")

                GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 3 } // 0 * 32 + 3
                GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 6 } // 0 * 32 + 6
            })
            Return (SBUF) /* \_SB.CAM0._CRS.SBUF */
        }
        Name (_DSD, Package () {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package () {
                    Package () {"DeviceEndpoint0", "SNS0"},
                    Package () {"GpioRstResId", 1},
                    Package () {"GpioPwnResId", 0},
                    Package () {"I2cResId", 0},
                    Package () {"CameraClockFrequencyHz", 24000000},
                }
        })
       
        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
            Return (0x0F)
        }
    }
}

#endif
