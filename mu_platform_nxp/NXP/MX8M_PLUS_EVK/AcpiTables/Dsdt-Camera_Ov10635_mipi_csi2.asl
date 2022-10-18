/*
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the copyright holder nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

Device (CAM1) // AVStream
{
  Name (_HID, "NXP0C21")  // _HID: Hardware ID
  Name (_UID, "1")  // _UID: Unique ID
  Name (_CCA, 0x0)
  Name (_DEP, Package() {\_SB.I2C3.SNS1, \_SB.ISI1, \_SB.MIP1})


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
        Package (2) {"MipiEndpoint0", "\\DosDevices\\MIP1"},
        Package (2) {"CsiEndpoint0", "\\DosDevices\\ISI1"},
        Package (2) {"SnsEndpoint0", "\\DosDevices\\SNS1"},
        Package (2) {"CpuId", 0x87},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

Device (MIP1) // MIPI on SoC peripheral
{
  Name (_HID, "NXP0C15")  // _HID: Hardware ID
  Name (_UID, "1")  // _UID: Unique ID
  Name (_CCA, 0x0)

  Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
  {
    Name (SBUF, ResourceTemplate ()
    {
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 112 } // INT_MIPI_CSI1 80 + 32
      Memory32Fixed (ReadWrite, 0x32E50000, FixedPcdGet32 (PcdMipiCsiDeviceMemorySize), )
    })
    Return (SBUF) /* \_SB.MIP1._CRS.SBUF */
  }
  
  Method (RGPR, 1, NotSerialized)
  // Method RGPR: Read GPR register
  //   Arg  Description
  //   ---  -------------------------------
  //     0  Register offset (Access to registers that do not belong to this peripheral returns 1)
  //   Returns  Description
  //   ---  -------------------------------
  //     0  Status: 0 - Success, 1 - Failure (Invalid register)
  //     1  (Only on Success) Value red from the register
  {
#define IMX_DISP_MIX_GPR_GASKET_CTRL              0x60
#define IMX_DISP_MIX_GPR_DISP_MIX_GASKET_HSIZE    0x64
#define IMX_DISP_MIX_GPR_DISP_MIX_GASKET_VSIZE    0x68
#define IMX_DISP_MIX_GPR_GASKET_ISI_LINE_CNT      0x7C
#define IMX_DISP_MIX_GPR_GASKET_ISI_PIXEL_CNT     0x80
    local0 = 0
    
    Switch (ToInteger (Arg0)) {
        Case ( IMX_DISP_MIX_GPR_GASKET_CTRL ) {
            local0 = G1CT
        }
        Case ( IMX_DISP_MIX_GPR_DISP_MIX_GASKET_HSIZE ) {
            local0 = G1HS
        }
        Case ( IMX_DISP_MIX_GPR_DISP_MIX_GASKET_VSIZE ) {
            local0 = G1VS
        }
        Case ( IMX_DISP_MIX_GPR_GASKET_ISI_LINE_CNT ) {
            local0 = G1PC
        }
        Case ( IMX_DISP_MIX_GPR_GASKET_ISI_PIXEL_CNT ) {
            local0 = G1LC
        }
        default {
            Return(1)
        }
    }
    Name (RGPB, Package(2) {0x0, 0x0}) // Couldn't get "Return (Package(2) {0, local0})" to work, so I name the returned package first.
    Store(local0, Index(RGPB, 1))
    Return (RGPB)
  }
  
    Method (WGPR, 2, NotSerialized)
  // Method WGPR: Write GPR register
  //   Arg  Description
  //   ---  -------------------------------
  //     0  Register offset (Access to registers that do not belong to this peripheral returns 1)
  //     1  Value to be written
  //   Returns  Description
  //   ---  -------------------------------
  //     0  Status: 0 - Success, 1 - Failure (Invalid register)
  {
    local0 = Arg0
    local2 = 0

    Switch (ToInteger (local0)) {
        Case ( IMX_DISP_MIX_GPR_GASKET_CTRL ) {
            Store (Arg1, G1CT)
        }
        Case ( IMX_DISP_MIX_GPR_DISP_MIX_GASKET_HSIZE ) {
            Store (Arg1, G1HS)
        }
        Case ( IMX_DISP_MIX_GPR_DISP_MIX_GASKET_VSIZE ) {
            Store (Arg1, G1VS)
        }
        Case ( IMX_DISP_MIX_GPR_GASKET_ISI_LINE_CNT ) {
            Store (Arg1, G1PC)
        }
        Case ( IMX_DISP_MIX_GPR_GASKET_ISI_PIXEL_CNT ) {
            Store (Arg1, G1LC)
        }
        default {
          local2 = 1
        }
    }
    Return (local2)
  }
  
  Name (_DSD, Package () {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"PhyClockFrequencyHz", 100000000},
        Package (2) {"EscClockFrequencyHz", 66666667},
        Package (2) {"Mipi1RegResId", 0},
        Package (2) {"Isi1Ch1IsrResId", 0},
        Package (2) {"CpuId", 0x87},
        Package (2) {"PixelMode", 1}, // Dual mode - 16b mode
        Package (2) {"DeviceEndpoint0", "\\DosDevices\\MIP1"},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

Device (ISI1) // ISI on SoC peripheral
{
  Name (_HID, "NXP0C12")  // _HID: Hardware ID
  Name (_UID, "1")  // _UID: Unique ID
  Name (_CCA, 0x0)    // SDHC is not coherent

  Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
  {
    Name (SBUF, ResourceTemplate ()
    {

      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 74 } // INT_ISI_CH0 42 + 32
      Memory32Fixed (ReadWrite, 0x32E02000, FixedPcdGet32 (PcdIsiChannelMemorySize), ) // ISI
    })
    Return (SBUF) /* \_SB.ISI1._CRS.SBUF */
  } 

  Name (_DSD, Package () {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"CameraClockFrequencyHz", 20000000},
        Package (2) {"CoreClockFrequencyHz", 133000000},
        Package (2) {"PhyClockFrequencyHz", 100000000},
        Package (2) {"EscClockFrequencyHz", 66666667},
        //Desired MIPI CSI input data port. Posible values:
        //                                                  0 - MIPI-CSI2 instance #1
        //                                                  1 - MIPI-CSI2 instance #2
        Package (2) {"MipiCsiSrc", 1},
        Package (2) {"Isi1RegResId", 0},
        Package (2) {"Isi1Ch1IsrResId", 0},
        Package (2) {"CpuId", 0x87},
        Package (2) {"DeviceEndpoint0", "\\DosDevices\\ISI1"},
      }
  })

  Method (_STA, 0, NotSerialized)  // _STA: Status
  {
      Return (0x0F)
  }
}

Scope (\_SB.I2C3)
{
  Device (SNS1)
  {
    Name (_HID, "NXP0C16")  // _HID: Hardware ID
    Name (_UID, "1")  // _UID: Unique ID
    Name (_CCA, 0x0)  // SDHC is not coherent

    Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
    {
      Name (SBUF, ResourceTemplate ()
      {
        //MAX9286(Deserializer) device address = 0x6A
        I2CSerialBus(0x6A, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")

        //MAX9271(Serializer) default device address = 0x40
        I2CSerialBus(0x40, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        //MAX9271(Serializeres) unique device addresses to be programmed
        I2CSerialBus(0x41, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x42, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x43, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x44, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        //MAX9271(Serializer) broadcast device address = 0x40
        I2CSerialBus(0x45, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")

        //OV10635 Camera default address
        I2CSerialBus(0x30, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        //OV10635(Cameras) unique device addresses to be programmed
        I2CSerialBus(0x31, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x32, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x33, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")
        I2CSerialBus(0x34, ControllerInitiated, 100000, AddressingMode7Bit, "\\_SB.I2C3")

    #if !(FixedPcdGet32(PcdCsi1CameraOv10635) || FixedPcdGet32(PcdCsi1CameraOv5640))
        GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 43 } // CSI_PWDN routed to GPIO2_IO11 => (1 * 32 + 11)
        GpioIO(Exclusive, PullUp, 0, 1, IoRestrictionNone, "\\_SB.GPIO", 0, ResourceConsumer, , ) { 6 } // CSI_nRST routed to GPIO1_IO06 => (0 * 32 + 6)
    #endif
      })
      Return (SBUF) /* \_SB.SNS1._CRS.SBUF */
    }
    Name (_DSD, Package () {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package (2) {"DeviceEndpoint0", "\\DosDevices\\SNS1"},
          Package () {"I2cResIdMax9286",      0},
          Package () {"I2cResIdMax9271",      1},
          Package () {"I2cResIdMax9271_0",    2},
          Package () {"I2cResIdMax9271_1",    3},
          Package () {"I2cResIdMax9271_2",    4},
          Package () {"I2cResIdMax9271_3",    5},
          Package () {"I2cResIdMax9271_Broadcast", 6},
          Package () {"I2cResIdOv10635"  ,    7},
          Package () {"I2cResIdOv10635_0",    8},
          Package () {"I2cResIdOv10635_1",    9},
          Package () {"I2cResIdOv10635_2",    10},
          Package () {"I2cResIdOv10635_3",    11},
    #if !(FixedPcdGet32(PcdCsi1CameraOv10635) || FixedPcdGet32(PcdCsi1CameraOv5640))
          Package () {"GpioPwnResId", 0},
          Package () {"GpioRstResId", 1},
    #endif
          Package () {"CameraClockFrequencyHz", 24000000},
        }
    })
     
    Method (_STA, 0, NotSerialized)  // _STA: Status
    {
      Return (0x0F)
    }
  }
}