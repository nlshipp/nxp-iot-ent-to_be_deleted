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

#ifndef _IMX_GPIO_H_
#define _IMX_GPIO_H_

#include "IMX8GpioPinNumbers.h"
#include "svc/pad/pad_api.h"
#include "svc/imx8qx_pads.h"

#ifndef SC_P_UNDEF
#define SC_P_UNDEF 255
#endif

#define IMX_PIN_ESAI0_FSR               IMX_PIN_NUM_GPIO0_IO00
#define IMX_PIN_ESAI0_FST               IMX_PIN_NUM_GPIO0_IO01
#define IMX_PIN_ESAI0_SCKR              IMX_PIN_NUM_GPIO0_IO02 
#define IMX_PIN_ESAI0_SCKT              IMX_PIN_NUM_GPIO0_IO03 
#define IMX_PIN_ESAI0_TX0               IMX_PIN_NUM_GPIO0_IO04 
#define IMX_PIN_ESAI0_TX1               IMX_PIN_NUM_GPIO0_IO05 
#define IMX_PIN_ESAI0_TX2_RX3           IMX_PIN_NUM_GPIO0_IO06 
#define IMX_PIN_ESAI0_TX3_RX2           IMX_PIN_NUM_GPIO0_IO07 
#define IMX_PIN_ESAI0_TX4_RX1           IMX_PIN_NUM_GPIO0_IO08 
#define IMX_PIN_ESAI0_TX5_RX0           IMX_PIN_NUM_GPIO0_IO09 
#define IMX_PIN_SPDIF0_RX               IMX_PIN_NUM_GPIO0_IO10 
#define IMX_PIN_SPDIF0_TX               IMX_PIN_NUM_GPIO0_IO11 
#define IMX_PIN_SPDIF0_EXT_CLK          IMX_PIN_NUM_GPIO0_IO12 
#define IMX_PIN_SPI3_SCK                IMX_PIN_NUM_GPIO0_IO13 
#define IMX_PIN_SPI3_SDO                IMX_PIN_NUM_GPIO0_IO14 
#define IMX_PIN_SPI3_SDI                IMX_PIN_NUM_GPIO0_IO15 
#define IMX_PIN_SPI3_CS0                IMX_PIN_NUM_GPIO0_IO16 
#define IMX_PIN_SPI3_CS1                IMX_PIN_NUM_GPIO0_IO17
#define IMX_PIN_MCLK_IN1                IMX_PIN_NUM_GPIO0_IO18 
#define IMX_PIN_MCLK_IN0                IMX_PIN_NUM_GPIO0_IO19 
#define IMX_PIN_MCLK_OUT0               IMX_PIN_NUM_GPIO0_IO20 
#define IMX_PIN_UART1_TX                IMX_PIN_NUM_GPIO0_IO21 
#define IMX_PIN_UART1_RX                IMX_PIN_NUM_GPIO0_IO22 
#define IMX_PIN_UART1_RTS_B             IMX_PIN_NUM_GPIO0_IO23
#define IMX_PIN_UART1_CTS_B             IMX_PIN_NUM_GPIO0_IO24 
#define IMX_PIN_SAI0_TXD                IMX_PIN_NUM_GPIO0_IO25 
#define IMX_PIN_SAI0_TXC                IMX_PIN_NUM_GPIO0_IO26 
#define IMX_PIN_SAI0_RXD                IMX_PIN_NUM_GPIO0_IO27 
#define IMX_PIN_SAI0_TXFS               IMX_PIN_NUM_GPIO0_IO28 
#define IMX_PIN_SAI1_RXD                IMX_PIN_NUM_GPIO0_IO29 
#define IMX_PIN_SAI1_RXC                IMX_PIN_NUM_GPIO0_IO30 
#define IMX_PIN_SAI1_RXFS               IMX_PIN_NUM_GPIO0_IO31 

#define IMX_PIN_SPI2_CS0                IMX_PIN_NUM_GPIO1_IO00 
#define IMX_PIN_SPI2_SDO                IMX_PIN_NUM_GPIO1_IO01 
#define IMX_PIN_SPI2_SDI                IMX_PIN_NUM_GPIO1_IO02 
#define IMX_PIN_SPI2_SCK                IMX_PIN_NUM_GPIO1_IO03 
#define IMX_PIN_SPI0_SCK                IMX_PIN_NUM_GPIO1_IO04 
#define IMX_PIN_SPI0_SDI                IMX_PIN_NUM_GPIO1_IO05 
#define IMX_PIN_SPI0_SDO                IMX_PIN_NUM_GPIO1_IO06 
#define IMX_PIN_SPI0_CS1                IMX_PIN_NUM_GPIO1_IO07 
#define IMX_PIN_SPI0_CS0                IMX_PIN_NUM_GPIO1_IO08 
#define IMX_PIN_ADC_IN1                 IMX_PIN_NUM_GPIO1_IO09 
#define IMX_PIN_ADC_IN0                 IMX_PIN_NUM_GPIO1_IO10 
#define IMX_PIN_ADC_IN3                 IMX_PIN_NUM_GPIO1_IO11 
#define IMX_PIN_ADC_IN2                 IMX_PIN_NUM_GPIO1_IO12 
#define IMX_PIN_ADC_IN5                 IMX_PIN_NUM_GPIO1_IO13 
#define IMX_PIN_ADC_IN4                 IMX_PIN_NUM_GPIO1_IO14 
#define IMX_PIN_FLEXCAN0_RX             IMX_PIN_NUM_GPIO1_IO15 
#define IMX_PIN_FLEXCAN0_TX             IMX_PIN_NUM_GPIO1_IO16 
#define IMX_PIN_FLEXCAN1_RX             IMX_PIN_NUM_GPIO1_IO17 
#define IMX_PIN_FLEXCAN1_TX             IMX_PIN_NUM_GPIO1_IO18 
#define IMX_PIN_FLEXCAN2_RX             IMX_PIN_NUM_GPIO1_IO19 
#define IMX_PIN_FLEXCAN2_TX             IMX_PIN_NUM_GPIO1_IO20 
#define IMX_PIN_UART0_RX                IMX_PIN_NUM_GPIO1_IO21 
#define IMX_PIN_UART0_TX                IMX_PIN_NUM_GPIO1_IO22 
#define IMX_PIN_UART2_TX                IMX_PIN_NUM_GPIO1_IO23 
#define IMX_PIN_UART2_RX                IMX_PIN_NUM_GPIO1_IO24 
#define IMX_PIN_MIPI_DSI0_I2C0_SCL      IMX_PIN_NUM_GPIO1_IO25 
#define IMX_PIN_MIPI_DSI0_I2C0_SDA      IMX_PIN_NUM_GPIO1_IO26 
#define IMX_PIN_MIPI_DSI0_GPIO0_00      IMX_PIN_NUM_GPIO1_IO27 
#define IMX_PIN_MIPI_DSI0_GPIO0_01      IMX_PIN_NUM_GPIO1_IO28 
#define IMX_PIN_MIPI_DSI1_I2C0_SCL      IMX_PIN_NUM_GPIO1_IO29 
#define IMX_PIN_MIPI_DSI1_I2C0_SDA      IMX_PIN_NUM_GPIO1_IO30 
#define IMX_PIN_MIPI_DSI1_GPIO0_00      IMX_PIN_NUM_GPIO1_IO31 

#define IMX_PIN_MIPI_DSI1_GPIO0_01      IMX_PIN_NUM_GPIO2_IO00 
#define IMX_PIN_PMIC_I2C_SCL            IMX_PIN_NUM_GPIO2_IO01 
#define IMX_PIN_PMIC_I2C_SDA            IMX_PIN_NUM_GPIO2_IO02 
#define IMX_PIN_SCU_GPIO0_00            IMX_PIN_NUM_GPIO2_IO03 

#define IMX_PIN_PMIC_INT_B              IMX_PIN_NUM_GPIO2_IO04
#define IMX_PIN_SCU_GPIO0_01            IMX_PIN_NUM_GPIO2_IO05 
#define IMX_PIN_SCU_PMIC_STANDBY        IMX_PIN_NUM_GPIO2_IO06 
#define IMX_PIN_SCU_BOOT_MODE0          IMX_PIN_NUM_GPIO2_IO07 
#define IMX_PIN_SCU_BOOT_MODE1          IMX_PIN_NUM_GPIO2_IO08 
#define IMX_PIN_SCU_BOOT_MODE2          IMX_PIN_NUM_GPIO2_IO09 
#define IMX_PIN_SCU_BOOT_MODE3          IMX_PIN_NUM_GPIO2_IO10 
#define IMX_PIN_CSI_D00                 IMX_PIN_NUM_GPIO2_IO11 
#define IMX_PIN_CSI_D01                 IMX_PIN_NUM_GPIO2_IO12 
#define IMX_PIN_CSI_D02                 IMX_PIN_NUM_GPIO2_IO13 
#define IMX_PIN_CSI_D03                 IMX_PIN_NUM_GPIO2_IO14 
#define IMX_PIN_CSI_D04                 IMX_PIN_NUM_GPIO2_IO15 
#define IMX_PIN_CSI_D05                 IMX_PIN_NUM_GPIO2_IO16 
#define IMX_PIN_CSI_D06                 IMX_PIN_NUM_GPIO2_IO17 
#define IMX_PIN_CSI_D07                 IMX_PIN_NUM_GPIO2_IO18 
#define IMX_PIN_CSI_HSYNC               IMX_PIN_NUM_GPIO2_IO19 
#define IMX_PIN_CSI_VSYNC               IMX_PIN_NUM_GPIO2_IO20

#define IMX_PIN_CSI_PCLK                IMX_PIN_NUM_GPIO3_IO00 
#define IMX_PIN_CSI_MCLK                IMX_PIN_NUM_GPIO3_IO01 
#define IMX_PIN_CSI_EN                  IMX_PIN_NUM_GPIO3_IO02 
#define IMX_PIN_CSI_RESET               IMX_PIN_NUM_GPIO3_IO03 
#define IMX_PIN_MIPI_CSI0_MCLK_OUT      IMX_PIN_NUM_GPIO3_IO04 
#define IMX_PIN_MIPI_CSI0_I2C0_SCL      IMX_PIN_NUM_GPIO3_IO05 
#define IMX_PIN_MIPI_CSI0_I2C0_SDA      IMX_PIN_NUM_GPIO3_IO06 
#define IMX_PIN_MIPI_CSI0_GPIO0_01      IMX_PIN_NUM_GPIO3_IO07 
#define IMX_PIN_MIPI_CSI0_GPIO0_00      IMX_PIN_NUM_GPIO3_IO08 
#define IMX_PIN_QSPI0A_DATA0            IMX_PIN_NUM_GPIO3_IO09 
#define IMX_PIN_QSPI0A_DATA1            IMX_PIN_NUM_GPIO3_IO10 
#define IMX_PIN_QSPI0A_DATA2            IMX_PIN_NUM_GPIO3_IO11 
#define IMX_PIN_QSPI0A_DATA3            IMX_PIN_NUM_GPIO3_IO12 
#define IMX_PIN_QSPI0A_DQS              IMX_PIN_NUM_GPIO3_IO13 
#define IMX_PIN_QSPI0A_SS0_B            IMX_PIN_NUM_GPIO3_IO14 
#define IMX_PIN_QSPI0A_SS1_B            IMX_PIN_NUM_GPIO3_IO15 
#define IMX_PIN_QSPI0A_SCLK             IMX_PIN_NUM_GPIO3_IO16 
#define IMX_PIN_QSPI0B_SCLK             IMX_PIN_NUM_GPIO3_IO17 
#define IMX_PIN_QSPI0B_DATA0            IMX_PIN_NUM_GPIO3_IO18 
#define IMX_PIN_QSPI0B_DATA1            IMX_PIN_NUM_GPIO3_IO19 
#define IMX_PIN_QSPI0B_DATA2            IMX_PIN_NUM_GPIO3_IO20 
#define IMX_PIN_QSPI0B_DATA3            IMX_PIN_NUM_GPIO3_IO21 
#define IMX_PIN_QSPI0B_DQS              IMX_PIN_NUM_GPIO3_IO22 
#define IMX_PIN_QSPI0B_SS0_B            IMX_PIN_NUM_GPIO3_IO23 
#define IMX_PIN_QSPI0B_SS1_B            IMX_PIN_NUM_GPIO3_IO24 

#define IMX_PIN_PCIE_CTRL0_PERST_B      IMX_PIN_NUM_GPIO4_IO00 
#define IMX_PIN_PCIE_CTRL0_CLKREQ_B     IMX_PIN_NUM_GPIO4_IO01 
#define IMX_PIN_PCIE_CTRL0_WAKE_B       IMX_PIN_NUM_GPIO4_IO02 
#define IMX_PIN_USB_SS3_TC0             IMX_PIN_NUM_GPIO4_IO03 
#define IMX_PIN_USB_SS3_TC1             IMX_PIN_NUM_GPIO4_IO04 
#define IMX_PIN_USB_SS3_TC2             IMX_PIN_NUM_GPIO4_IO05 
#define IMX_PIN_USB_SS3_TC3             IMX_PIN_NUM_GPIO4_IO06 
#define IMX_PIN_EMMC0_CLK               IMX_PIN_NUM_GPIO4_IO07 
#define IMX_PIN_EMMC0_CMD               IMX_PIN_NUM_GPIO4_IO08 
#define IMX_PIN_EMMC0_DATA0             IMX_PIN_NUM_GPIO4_IO09 
#define IMX_PIN_EMMC0_DATA1             IMX_PIN_NUM_GPIO4_IO10 
#define IMX_PIN_EMMC0_DATA2             IMX_PIN_NUM_GPIO4_IO11 
#define IMX_PIN_EMMC0_DATA3             IMX_PIN_NUM_GPIO4_IO12 
#define IMX_PIN_EMMC0_DATA4             IMX_PIN_NUM_GPIO4_IO13 
#define IMX_PIN_EMMC0_DATA5             IMX_PIN_NUM_GPIO4_IO14 
#define IMX_PIN_EMMC0_DATA6             IMX_PIN_NUM_GPIO4_IO15 
#define IMX_PIN_EMMC0_DATA7             IMX_PIN_NUM_GPIO4_IO16 
#define IMX_PIN_EMMC0_STROBE            IMX_PIN_NUM_GPIO4_IO17 
#define IMX_PIN_EMMC0_RESET_B           IMX_PIN_NUM_GPIO4_IO18 
#define IMX_PIN_USDHC1_RESET_B          IMX_PIN_NUM_GPIO4_IO19 
#define IMX_PIN_USDHC1_VSELECT          IMX_PIN_NUM_GPIO4_IO20 
#define IMX_PIN_USDHC1_WP               IMX_PIN_NUM_GPIO4_IO21 
#define IMX_PIN_USDHC1_CD_B             IMX_PIN_NUM_GPIO4_IO22 
#define IMX_PIN_USDHC1_CLK              IMX_PIN_NUM_GPIO4_IO23 
#define IMX_PIN_USDHC1_CMD              IMX_PIN_NUM_GPIO4_IO24 
#define IMX_PIN_USDHC1_DATA0            IMX_PIN_NUM_GPIO4_IO25 
#define IMX_PIN_USDHC1_DATA1            IMX_PIN_NUM_GPIO4_IO26 
#define IMX_PIN_USDHC1_DATA2            IMX_PIN_NUM_GPIO4_IO27 
#define IMX_PIN_USDHC1_DATA3            IMX_PIN_NUM_GPIO4_IO28 
#define IMX_PIN_ENET0_RGMII_TXC         IMX_PIN_NUM_GPIO4_IO29 
#define IMX_PIN_ENET0_RGMII_TX_CTL      IMX_PIN_NUM_GPIO4_IO30 
#define IMX_PIN_ENET0_RGMII_TXD0        IMX_PIN_NUM_GPIO4_IO31 

#define IMX_PIN_ENET0_RGMII_TXD1        IMX_PIN_NUM_GPIO5_IO00 
#define IMX_PIN_ENET0_RGMII_TXD2        IMX_PIN_NUM_GPIO5_IO01 
#define IMX_PIN_ENET0_RGMII_TXD3        IMX_PIN_NUM_GPIO5_IO02 
#define IMX_PIN_ENET0_RGMII_RXC         IMX_PIN_NUM_GPIO5_IO03 
#define IMX_PIN_ENET0_RGMII_RX_CTL      IMX_PIN_NUM_GPIO5_IO04 
#define IMX_PIN_ENET0_RGMII_RXD0        IMX_PIN_NUM_GPIO5_IO05 
#define IMX_PIN_ENET0_RGMII_RXD1        IMX_PIN_NUM_GPIO5_IO06 
#define IMX_PIN_ENET0_RGMII_RXD2        IMX_PIN_NUM_GPIO5_IO07 
#define IMX_PIN_ENET0_RGMII_RXD3        IMX_PIN_NUM_GPIO5_IO08 
#define IMX_PIN_ENET0_REFCLK_125M_25M   IMX_PIN_NUM_GPIO5_IO09 
#define IMX_PIN_ENET0_MDIO              IMX_PIN_NUM_GPIO5_IO10 
#define IMX_PIN_ENET0_MDC               IMX_PIN_NUM_GPIO5_IO11

#endif  // _IMX_GPIO_H_