/*
* iMX8 MQ Differentiated System Description Table Fields (DSDT)
*
*  Copyright (c) 2018, Microsoft Corporation. All rights reserved.
*  Copyright 2020 NXP
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
*/

#include "Dsdt-Common.h"
#include "iMX8Platform.h"
#include <IndustryStandard/Acpi.h>
#include <Library/AcpiLib.h>
#include <Library/PcdLib.h>

DefinitionBlock("DsdtTable.aml", "DSDT", 5, "MSFT", "EDK2", 1) {
  Scope(_SB) {
    include("Dsdt-Platform.asl")
    /* Dependencies */
    include("Dsdt-Gpio.asl")
    include("Dsdt-I2c.asl")
    include("Dsdt-Spi.asl")
    include("Dsdt-Uart.asl")
    /* Others */
    include("Dsdt-Audio.asl")
    include("Dsdt-Camera_Ov5640.asl")
    include("Dsdt-Enet.asl")
    include("Dsdt-Gfx.asl")
    include("Dsdt-PCIe.asl")
    include("Dsdt-Pwm.asl")
    include("Dsdt-Rhp.asl")
    include("Dsdt-Sdhc.asl")
    include("Dsdt_TCPC.asl")
    include("Dsdt-Tmu.asl")
    include("Dsdt-TrEE.asl")
    include("Dsdt-Usb.asl")
    include("Dsdt-Vpu.asl")
  }
}
