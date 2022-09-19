/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef IMX_MEDIAMIX_H
#define IMX_MEDIAMIX_H

#include <linux/io.h>

#define IMX_MEDIAMIX_BASE  0x32EC0000
#define IMX_MEDIAMIX_SIZE  0x10000

 /* Media Mix Software Reset Register */
#define MEDIAMIX_SFT_RSTN                    0x00

#define SFT_EN_MIPI_DSI_PCLK_RESETN_MASK     (1UL << 0)
#define SFT_EN_MIPI_DSI_CLKREF_RESETN_MASK   (1UL << 1)
#define SFT_EN_LCDIF_PIXEL_CLK_RESETN_MASK   (1UL << 4)
#define SFT_EN_LCDIF_APB_CLK_RESETN_MASK     (1UL << 5)
#define SFT_EN_BUS_BLK_CLK_RESETN_MASK       (1UL << 8)
#define SFT_EN_LCDIF2_PIXEL_CLK_RESETN_MASK  (1UL << 11)
#define SFT_EN_LCDIF2_APB_CLK_RESETN_MASK    (1UL << 12)
#define SFT_EN_LCDIF_AXI_CLK_RESETN_MASK     (1UL << 23)
#define SFT_EN_LCDIF2_AXI_CLK_RESETN_MASK    (1UL << 24)



/* Media Mix Clock Enable Register */
#define MEDIAMIX_CLK_EN                      0x04

#define SFT_EN_MIPI_DSI_PCLK_MASK            (1UL << 0)
#define SFT_EN_MIPI_DSI_PCLK_SHIFT           (0)
#define SFT_EN_MIPI_DSI_CLKREF_MASK          (1UL << 1)
#define SFT_EN_MIPI_DSI_CLKREF_SHIFT         (1)
#define SFT_EN_LCDIF_PIXEL_CLK_MASK          (1UL << 4)
#define SFT_EN_LCDIF_PIXEL_CLK_SHIFT         (4)
#define SFT_EN_LCDIF_APB_CLK_MASK            (1UL << 5)
#define SFT_EN_LCDIF_APB_CLK_SHIFT           (5)
#define SFT_EN_BUS_BLK_CLK_MASK              (1UL << 8)
#define SFT_EN_BUS_BLK_CLK_SHIFT             (8)
#define SFT_EN_LCDIF2_PIXEL_CLK_MASK         (1UL << 11)
#define SFT_EN_LCDIF2_PIXEL_CLK_SHIFT        (11)
#define SFT_EN_LCDIF2_APB_CLK_MASK           (1UL << 12)
#define SFT_EN_LCDIF2_APB_CLK_SHIFT          (12)
#define SFT_EN_LCDIF_AXI_CLK_MASK            (1UL << 23)
#define SFT_EN_LCDIF_AXI_CLK_SHIFT           (23)
#define SFT_EN_LCDIF2_AXI_CLK_MASK           (1UL << 24)
#define SFT_EN_LCDIF2_AXI_CLK_SHIFT          (24)



/* MIPI PHY Control Register */
#define GPR_MIPI_RESET_DIV                   0x08

#define GPR_MIPI_S_RESETN_MASK               (1UL << 16)
#define GPR_MIPI_M_RESETN_MASK               (1UL << 17)

void mediamix_dsi_reset(volatile uint8_t __iomem *regptr, bool enable);
void mediamix_lvds_reset(volatile uint8_t __iomem *regptr, bool enable);
void mediamix_dump_reg(volatile uint8_t __iomem *regptr);

#endif
