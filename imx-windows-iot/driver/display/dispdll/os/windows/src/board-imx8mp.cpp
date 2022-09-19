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

#include "precomp.h"

extern "C" {

#include <stdlib.h>

#include "boot\dts\freescale\board.h"
#include "linux\of.h"
#include "linux\platform_device.h"
#include "clk\imx_clk_imx8mp.h"

/* Resources such as registers memory area allocated for devices */
static resource mp_lcdif1_res_list[] = {
    { 0x32E80000, 0x32E8FFFF, "lcdif1", IORESOURCE_MEM },
};

static resource mp_lcdif2_res_list[] = {
    { 0x32E90000, 0x32E9FFFF, "lcdif2", IORESOURCE_MEM },
};

static resource mp_lvds_res_list[] = {
    { 0x32EC0000, 0x32ECFFFF, "media-blk-ctl", IORESOURCE_MEM },
};

static resource mp_mipi_res_list[] = {
    { 0x32E60000, 0x32E6FFFF, "mipi-dsi1", IORESOURCE_MEM },
    { 0x32EC0000, 0x32ECFFFF, "media-blk-ctl", IORESOURCE_MEM },
};

/* Properties such as compatible assigned to devices */
static property mp_lcdif2_properties[] = {
    { "compatible", 1, "fsl,imx8mp-lcdif2" },
    { "" } // marks end of the list
};

static property mp_lcdif1_properties[] = {
    { "compatible", 1, "fsl,imx8mp-lcdif1" },
    { "" } // marks end of the list
};

/* Clock config for devices:
   name: The same name as used in the device driver in "devm_clk_get" call. Mandatory.
   id: index of the clock item in the clock tree. See "Clock tree items - START" table.
       Mandatory.
   parent_sel: Initial value of the parent selector for attached clock slice.
               Set during "devm_clk_get" call. See "Clock tree items - START" table.
               Optional, set to zero if not used.
   rate: initial rate of the clock in Hz set during "devm_clk_get" call.
         Optional, set to zero if not used.
   parent_enable: Non-zero = initially enable all attached parents
                 (doesn't apply for the clock itself, it's state remains the same).
                 Zero if not used.
*/
static struct clk_init_data_desc mp_lcdif1_clock_desc[] = {
    /* name, id, parent_sel, rate, parent_enable */
    { "pix", IMX8MP_CLK_MEDIA_LCDIF1_PIXEL, IMX8MP_VIDEO_PLL1, 0, 1 },
    { "disp-axi", IMX8MP_CLK_MEDIA_LCDIF1_AXI, IMX8MP_SYS_PLL2, IMX_CLK_FREQ_500M, 1 },
    { "disp-apb", IMX8MP_CLK_MEDIA_LCDIF1_APB, IMX8MP_SYS_PLL1, IMX_CLK_FREQ_200M, 1 },
    {""} // marks end of the list
};

static struct clk_init_data_desc mp_lcdif2_clock_desc[] = {
    /* name, id, parent_sel, rate, parent_enable */
    { "pix", IMX8MP_CLK_MEDIA_LCDIF2_PIXEL, IMX8MP_VIDEO_PLL1, 0, 1 },
    { "disp-axi", IMX8MP_CLK_MEDIA_LCDIF2_AXI, IMX8MP_SYS_PLL2, IMX_CLK_FREQ_500M, 1 },
    { "disp-apb", IMX8MP_CLK_MEDIA_LCDIF2_APB, IMX8MP_SYS_PLL1, IMX_CLK_FREQ_200M, 1 },
    {""} // marks end of the list
};

static struct clk_init_data_desc mp_lvds_clock_desc[] = {
    /* name, id, parent_sel, rate, parent_enable */
    { "ldb", IMX8MP_CLK_MEDIA_LDB, IMX8MP_VIDEO_PLL1, 0, 0 },
    { "apb", IMX8MP_CLK_MEDIA_APB, 0, 0,  0 }, /* APB initialized by LCDIF */
    {""} // marks end of the list
};

static struct clk_init_data_desc mp_mipi_clock_desc[] = {
    /* name, id, parent_sel, rate, parent_enable */
    { "pll-ref", IMX8MP_CLK_MEDIA_MIPI_DSI_CLKREF, IMX8MP_CLK_24M, IMX_CLK_FREQ_12M, 1 },
    { "cfg", IMX8MP_CLK_MEDIA_MIPI_DSI_PCLK, 0, 0, 0 }, /* Parent of MIPI_DSI_PCLK is APB, which is initialized by LCDIF */
    {""} // marks end of the list
};

static void lcdif2_irq_init(void)
{
    irq_desc[0].name = "vblank";
    irq_desc[0].irq_data.hwirq = 6;
    irq_desc[0].handler = nullptr;
    irq_desc[0].irq_data.chip.type = IRQCHIP_TYPE_GIC;
}

static void lcdif1_irq_init(void)
{
    irq_desc[0].name = "vblank";
    irq_desc[0].irq_data.hwirq = 5;
    irq_desc[0].handler = nullptr;
    irq_desc[0].irq_data.chip.type = IRQCHIP_TYPE_GIC;
}

static inline void lvds_res_init(platform_device* pdev)
{
    pdev->resource = &mp_lvds_res_list[0];
    pdev->num_resources = ARRAYSIZE(mp_lvds_res_list);
    pdev->dev.of_clk = (struct clk_init_data_desc *)&mp_lvds_clock_desc;
    pdev->dev.get_clock_item = &clk_get_item_imx8mp;
}

static inline void mipi_res_init(platform_device* pdev)
{
    pdev->resource = &mp_mipi_res_list[0];
    pdev->num_resources = ARRAYSIZE(mp_mipi_res_list);
    pdev->dev.of_clk = (struct clk_init_data_desc *)&mp_mipi_clock_desc;
    pdev->dev.get_clock_item = &clk_get_item_imx8mp;
}

static inline void lcdif1_res_init(platform_device* pdev)
{
    pdev->resource = &mp_lcdif1_res_list[0];
    pdev->num_resources = ARRAYSIZE(mp_lcdif1_res_list);
    pdev->dev.of_node.properties = (property *) &mp_lcdif1_properties;
    pdev->dev.of_clk = (struct clk_init_data_desc *)&mp_lcdif1_clock_desc;
    pdev->dev.get_clock_item = &clk_get_item_imx8mp;
}

static inline void lcdif2_res_init(platform_device* pdev)
{
    pdev->resource = &mp_lcdif2_res_list[0];
    pdev->num_resources = ARRAYSIZE(mp_lcdif2_res_list);
    pdev->dev.of_node.properties = (property *) &mp_lcdif2_properties;
    pdev->dev.of_clk = (struct clk_init_data_desc *)&mp_lcdif2_clock_desc;
    pdev->dev.get_clock_item = &clk_get_item_imx8mp;
}

void mp_board_init(platform_device* pdev)
{
    NT_ASSERT(pdev);

    if (!strcmp(pdev->name, "lcdif1_dev"))
    {
        lcdif1_res_init(pdev);
        lcdif1_irq_init();
    }
    else if (!strcmp(pdev->name, "lcdif2_dev"))
    {
        lcdif2_res_init(pdev);
        lcdif2_irq_init();
    }
    else if ((!strcmp(pdev->name, "lvds_dev")) || (!strcmp(pdev->name, "lvds_phy")))
    {
        lvds_res_init(pdev);
    }
    else if (!strcmp(pdev->name, "mipi-dsi_dev"))
    {
        mipi_res_init(pdev);
        //mipi_irq_init(pdev);
    }
}

void mp_board_deinit(platform_device* pdev)
{
    NT_ASSERT(pdev);

}

} // extern "C"
