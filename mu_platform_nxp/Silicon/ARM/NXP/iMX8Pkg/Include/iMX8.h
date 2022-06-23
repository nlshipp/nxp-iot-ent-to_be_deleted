/** @file
*
*  Header defining the iMX8 constants (Base addresses, sizes, flags)
*
*  Copyright (c), Microsoft Corporation. All rights reserved.
*  Copyright 2019-2020, 2022 NXP
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef __IMX8_H__
#define __IMX8_H__

//
// Memory mapping
//

// TODO: probably need to add references to all other hardware blocks,
// unlike imx6/7, AARCH64 uses the MMU and if it's not mapped you get an exception
#if defined(CPU_IMX8MM) || defined(CPU_IMX8MN)
#define ARM_PERIPHERALS_REGISTERS_PHYSICAL  0x31000000
#define ARM_PERIPHERALS_REGISTERS_LENGTH    0x00400000

/* AIPS1 - AIPS3 */
#define ARM_IP_BUS_REGISTERS_PHYSICAL       0x30000000
#define ARM_IP_BUS_REGISTERS_LENGTH         0x00C00000

/* PCIe registers */
#define PCIE_REG_REGISTER_PHYSICAL          0x33800000
#define PCIE_REG_REGISTER_LENGTH            0x00400000

/* PCIe mapped area */
#define PCIE_MAPPED_AREA_PHYSICAL           0x18000000
#define PCIE_MAPPED_AREA_LENGTH             0x08000000

/* USB1,2 and PCIE_PHY (part of AIPS4) */
#define ARM_USB_REG_REGISTER_PHYSICAL       0x32E40000
#define ARM_USB_REG_REGISTER_LENGTH         0x000D0000

#define ARM_GIC_REG_REGISTER_PHYSICAL       0x38800000
#define ARM_GIC_REG_REGISTER_LENGTH         0x00100000

#define ARM_LCDIF_REG_REGISTER_PHYSICAL     0x32E00000
#define ARM_LCDIF_REG_REGISTER_LENGTH       0x00100000

#define ARM_MIPI_DSI_REG_REGISTER_PHYSICAL  0x32E10000
#define ARM_MIPI_DSI_REG_REGISTER_LENGTH    0x00100000

#elif defined(CPU_IMX8MP)

/* PCIe mapped area */
#define PCIE_MAPPED_AREA_PHYSICAL           0x18000000
#define PCIE_MAPPED_AREA_LENGTH             0x08000000

/* AIPS1 - AIPS3, AIPS5 */
#define ARM_IP_BUS_REGISTERS_PHYSICAL       0x30000000
#define ARM_IP_BUS_REGISTERS_LENGTH         0x01000000

/* AIPS4 and LCDIFx, MIPI */
#define ARM_LCDIF1_REG_REGISTER_PHYSICAL    0x32E80000
#define ARM_LCDIF1_REG_REGISTER_LENGTH      0x00010000
#define ARM_LCDIF2_REG_REGISTER_PHYSICAL    0x32E90000
#define ARM_LCDIF2_REG_REGISTER_LENGTH      0x00010000
#define ARM_MIPI_DSI_REG_REGISTER_PHYSICAL  0x32E60000
#define ARM_MIPI_DSI_REG_REGISTER_LENGTH    0x00010000
#define ARM_MEDIA_BLK_REG_REGISTER_PHYSICAL 0x32EC0000
#define ARM_MEDIA_BLK_REG_REGISTER_LENGTH   0x00010000

/* PCIe, QSPI, GPU3D and GPU2D registers */
#define PCIE_REG_REGISTER_PHYSICAL          0x33800000
#define PCIE_REG_REGISTER_LENGTH            0x04808000

/* HSIO */
#define HSIO_REG_REGISTER_PHYSICAL          0x32F10000
#define HSIO_REG_REGISTER_LENGTH            0x08010000

/* USB1,2 */
#define ARM_USB_REG_REGISTER_PHYSICAL       0x38100000
#define ARM_USB_REG_REGISTER_LENGTH         0x00200000

/* GIC */
#define ARM_GIC_REG_REGISTER_PHYSICAL       0x38800000
#define ARM_GIC_REG_REGISTER_LENGTH         0x00100000

/* Audio DSP */
#define ARM_PERIPHERALS_REGISTERS_PHYSICAL  0x3B000000
#define ARM_PERIPHERALS_REGISTERS_LENGTH    0x01000000
#else // IMX8MQ

/* ARM Peripherals */
#define ARM_PERIPHERALS_REGISTERS_PHYSICAL  0x31000000
#define ARM_PERIPHERALS_REGISTERS_LENGTH    0x00400000

/* AIPS1 - AIPS3 */
#define ARM_IP_BUS_REGISTERS_PHYSICAL       0x30000000
#define ARM_IP_BUS_REGISTERS_LENGTH         0x01000000

/* PCIe1,2 registers */
#define PCIE_REG_REGISTER_PHYSICAL          0x33800000
#define PCIE_REG_REGISTER_LENGTH            0x00800000

/* PCIe1,2 mapped area */
#define PCIE_MAPPED_AREA_PHYSICAL           0x18000000
#define PCIE_MAPPED_AREA_LENGTH             0x10000000

/* USB1,2 registers */
#define ARM_USB_REG_REGISTER_PHYSICAL       0x38100000
#define ARM_USB_REG_REGISTER_LENGTH         0x00200000

/* GIC registers */
#define ARM_GIC_REG_REGISTER_PHYSICAL       0x38800000
#define ARM_GIC_REG_REGISTER_LENGTH         0x00100000

/* HDMI registers */
#define ARM_HDMI_REG_REGISTER_PHYSICAL       0x32C00000
#define ARM_HDMI_REG_REGISTER_LENGTH         0x00100000

#define ARM_DC_MSTx_REG_REGISTER_PHYSICAL    0x32E00000
#define ARM_DC_MSTx_REG_REGISTER_LENGTH      0x00040000

#endif

//
// Interrupts
//
#define TOTAL_PRIVATE_INTERRUPT     32
#define DEFINE_IMX8_INTERRUPT(a)    (a + TOTAL_PRIVATE_INTERRUPT)

//
// Serial debug port
//
#define UART1_BASE_ADDRESS   ((unsigned long)UART1_BASE_PTR)
#define UART2_BASE_ADDRESS   ((unsigned long)UART2_BASE_PTR)
#define UART3_BASE_ADDRESS   ((unsigned long)UART3_BASE_PTR)
#define UART4_BASE_ADDRESS   ((unsigned long)UART4_BASE_PTR)

#define UART_ADDRESS_SIZE    0x00010000 // 64KB

#define IMX_SERIAL_DBG_PORT_SUBTYPE     0x000C

#define SERIAL_DEBUG_PORT_INIT_MSG "\r\nUEFI Debug Serial Port Init\r\n"

//
// Clock Source
//
#if defined(CPU_IMX8MM) || defined(CPU_IMX8MN) || defined(CPU_IMX8MP)
#define SOC_OSC_FREQUENCY_REF_HZ  24000000  // Oscillator frequency 24Mhz
#else
#define SOC_OSC_FREQUENCY_REF_HZ  25000000  // Oscillator frequency 25Mhz
#endif

// SDMA (Smart DMA) controllers
#define SDMA1_BASE_ADDRESS      ((unsigned long)SDMAARM1_BASE_PTR)
#define SDMA1_IRQ               INT_SDMA1
#define SDMA2_BASE_ADDRESS      ((unsigned long)SDMAARM2_BASE_PTR)
#define SDMA2_IRQ               INT_SDMA2
#if defined(SDMAARM3_BASE_PTR)
#define SDMA3_BASE_ADDRESS      ((unsigned long)SDMAARM3_BASE_PTR)
#define SDMA3_IRQ               INT_SDMA3
#endif

//
// GPIO
//
#define IMX_GPIO_BASE               ((unsigned long)GPIO1_BASE_PTR)

//
// IOMux:
// IMX8M specific
//
#define IOMUXC_GPR_BASE_ADDRESS             ((unsigned long)IOMUXC_GPR_BASE_PTR)
#define IOMUXC_SW_MUX_PAD_BASE_ADDRESS      ((unsigned long)IOMUXC_BASE_PTR)
#define IOMUXC_SELECT_INPUT_BASE_ADDRESS    ((unsigned long)&IOMUXC_SELECT_INPUT_REG(IOMUXC_BASE_PTR,0))

/* Macro for GPC SIP */
#define IMX_SIP_GPC                     0xC2000000
/* Macro for fill SIP structure */
#define imx_fill_sip(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _str)   \
                        {                                       \
                            _str.Arg0 = _Arg0;                  \
                            _str.Arg1 = _Arg1;                  \
                            _str.Arg2 = _Arg2;                  \
                            _str.Arg3 = _Arg3;                  \
                            _str.Arg4 = _Arg4;                  \
                        };


/* TODO New IoMap.h BEGIN */

#define IOMUXC_MUX_SION_DISABLED    0x00000000
#define IOMUXC_MUX_SION_ENABLED     0x00000010
#define IOMUXC_MUX_ALT0             0x00000000
#define IOMUXC_MUX_ALT1             0x00000001
#define IOMUXC_MUX_ALT2             0x00000002
#define IOMUXC_MUX_ALT3             0x00000003
#define IOMUXC_MUX_ALT4             0x00000004
#define IOMUXC_MUX_ALT5             0x00000005
#define IOMUXC_MUX_ALT6             0x00000006
#define IOMUXC_MUX_ALT7             0x00000001

#define IOMUXC_LVTTL_DISABLED       0x00000000
#define IOMUXC_LVTTL_ENABLED        0x00000100
#define IOMUXC_PAD_HYS_DISABLED     0x00000000
#define IOMUXC_PAD_HYS_ENABLED      0x00000080
#define IOMUXC_PAD_PUE_DISABLED     0x00000000
#define IOMUXC_PAD_PUE_ENABLE       0x00000040
#define IOMUXC_PAD_ODE_DISABLED     0x00000000
#define IOMUXC_PAD_ODE_ENABLED      0x00000020
#define IOMUXC_PAD_SRE_SLOW         0x00000000
#define IOMUXC_PAD_SRE_MEDIUM       0x00000008
#define IOMUXC_PAD_SRE_FAST         0x00000010
#define IOMUXC_PAD_SRE_MAX          0x00000018

#define IOMUXC_PAD_DSE_DISABLED     0x00000000
#define IOMUXC_PAD_DSE_R0_DIV_1     0x00000001  // R0 divided by 1
#define IOMUXC_PAD_DSE_R0_DIV_2     0x00000002  // R0 divided by 2
#define IOMUXC_PAD_DSE_R0_DIV_3     0x00000003  // R0 divided by 3
#define IOMUXC_PAD_DSE_R0_DIV_4     0x00000004  // R0 divided by 4
#define IOMUXC_PAD_DSE_R0_DIV_5     0x00000005  // R0 divided by 5
#define IOMUXC_PAD_DSE_R0_DIV_6     0x00000006  // R0 divided by 6
#define IOMUXC_PAD_DSE_R0_DIV_7     0x00000007  // R0 divided by 7

#if defined(CPU_IMX8MP)
#define GPC_PU_PGC_SW_PUP_REQ_PCIE_SW_PUP_REQ_MASK (0x2U)
#endif

/* TODO New IoMap.h END */

#if defined(CPU_IMX8MM)
#include "iMX8MM/MIMX8MM6_ca53.h"
#include "iMX8DisplayMix.h"
#elif defined(CPU_IMX8MN)
#include "iMX8MN/MIMX8MN6_ca53.h"
#include "iMX8DisplayMix.h"
#elif defined(CPU_IMX8MP)
#include "iMX8MP/MIMX8MP8_ca53.h"
#else
#include "iMX8MQ/MIMX8MQ7_ca53.h"
#endif

#endif // __IMX8_PLATFORM_H__
