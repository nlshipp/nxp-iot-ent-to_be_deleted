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

/** @file
* iMX8 MP Differentiated System Description Table Fields (DSDT)
*/

#include "Dsdt-Common.h"
#include "iMX8Platform.h"
#include <IndustryStandard/Acpi.h>
#include <Library/AcpiLib.h>
#include <Library/PcdLib.h>

DefinitionBlock("DsdtTable.aml", "DSDT", 5, "MSFT", "EDK2", 1) {
  Scope(_SB) {
    include("Dsdt-Platform.asl")
    /* Shared general purpose registers */
    include("Dsdt-MediaBlkCtrl.asl")
    /* Dependencies */
    include("Dsdt-Gpio.asl")
    include("Dsdt-I2c.asl")
    include("Dsdt-Spi.asl")
    include("Dsdt-Uart.asl")
    /* Others */
    include("Dsdt-Sdhc.asl")
    include("Dsdt-TCPC.asl")
    include("Dsdt-Usb.asl")
    include("Dsdt-Rhp.asl")
    include("Dsdt-Enet.asl")
    include("Dsdt-Enet_QoS.asl")
    include("Dsdt-Audio.asl")
    include("Dsdt-AudioCodec.asl")
    include("Dsdt-TrEE.asl")
    include("Dsdt-PCIe.asl")
    include("Dsdt-Gfx.asl")
    include("Dsdt-HantroVpuLF.asl") /* Limited Feature */
    include("Dsdt-Pwm.asl")
    include("Dsdt-Tmu.asl")
    include("Dsdt-CAN.asl")
#if FixedPcdGet32(PcdCsi1CameraOv5640)
    include("Dsdt-Camera_Ov5640_mipi_csi1.asl")
#elif FixedPcdGet32(PcdCsi1CameraOv10635)
    include("Dsdt-Camera_Ov10635_mipi_csi1.asl")
#endif
#if FixedPcdGet32(PcdCsi2CameraOv5640)
    include("Dsdt-Camera_Ov5640_mipi_csi2.asl")
#elif FixedPcdGet32(PcdCsi2CameraOv10635)
    include("Dsdt-Camera_Ov10635_mipi_csi2.asl")
#endif
  }
}
