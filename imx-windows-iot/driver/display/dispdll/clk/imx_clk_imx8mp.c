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

#include <ntddk.h>
#include <stddef.h>
#include "imx_clk_imx8m_registers.h"
#include "imx_clk_imx8m_pll.h"
#include "imx_clk_imx8mp.h"

/* #define IMX_CLK_DEBUG_DUMP_WHOLE_CLK_TREE */

#define CLK_PRINTK(...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

#ifdef IMX_CLK_DEBUG
#define CLK_DEBUG(...) CLK_PRINTK(__VA_ARGS__)
#else
#define CLK_DEBUG(...)
#endif

#define CHECK_SUCCESS(e) do { if ((e) != 0) { \
                               CLK_PRINTK("Display CLK: CHECK_SUCCESS failed."); \
                               return -1; \
                           } \
                      } while (0);

#define IMX_CLK_GATE_DOMAIN_ALL_MODES 3

/* Clock Root Selects for clock root offsets and muxing (Clock Root Table in RM) */
#define IMX8MP_MEDIA_AXI_CLK_ROOT               20U
#define IMX8MP_MEDIA_APB_CLK_ROOT               21U
#define IMX8MP_MEDIA_DISP2_PIX_CLK_ROOT         38U
#define IMX8MP_MEDIA_MIPI_PHY1_REF_CLK_ROOT     123U
#define IMX8MP_MEDIA_DISP1_PIX_CLK_ROOT         124U
#define IMX8MP_MEDIA_LDB_CLK_ROOT               126U

/* CCGRx device clock gate mapping */
#define IMX8MP_CCM_CCGR_MEDIA                   93U

/* Main object of the clock tree - single instance for all monitors*/
static imx8mp_clk_device_t clktree = {.disp_lvds = imx_interface_undefined, .disp_mipi = imx_interface_undefined };

static unsigned share_display_clockgate = 0; /* Represents shared CCGR93 LPCG clock gate */
static unsigned share_apb_slice = 0; /* Shared enable/disable of IMX8MP_CLK_MEDIA_APB */
static unsigned share_axi_slice = 0; /* Shared enable/disable of IMX8MP_CLK_MEDIA_AXI */
static unsigned share_video_pll1 = 0; /* Shared Video_pll */

/* --------------------------- Clock slice mux initialization data --------------------------------*/

static struct clk_root_mux_data
imx8mp_media_axi_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_SYS_PLL2, IMX8MP_SYS_PLL2},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL3, IMX8MP_SYS_PLL3},
    {IMX_SYS_PLL1_DIV20, IMX8MP_SYS_PLL1_DIV20},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_EXT1, IMX8MP_CLK_EXT1},
    {IMX_SYS_PLL2_DIV2, IMX8MP_SYS_PLL2_DIV2}
};

static struct clk_root_mux_data
imx8mp_media_apb_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_SYS_PLL2_DIV8, IMX8MP_SYS_PLL2_DIV8},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL3, IMX8MP_SYS_PLL3},
    {IMX_SYS_PLL1_DIV20, IMX8MP_SYS_PLL1_DIV20},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_EXT1, IMX8MP_CLK_EXT1},
    {IMX_SYS_PLL1_DIV6, IMX8MP_SYS_PLL1_DIV6}
};

static struct clk_root_mux_data
imx8mp_media_disp1_pix_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_VIDEO_PLL, IMX8MP_VIDEO_PLL1},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_AUDIO_PLL1, IMX8MP_AUDIO_PLL1},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL2, IMX8MP_SYS_PLL2},
    {IMX_SYS_PLL3, IMX8MP_SYS_PLL3},
    {IMX_EXT4, IMX8MP_CLK_EXT4}
};

static struct clk_root_mux_data
imx8mp_media_disp2_pix_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_VIDEO_PLL, IMX8MP_VIDEO_PLL1},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_AUDIO_PLL1, IMX8MP_AUDIO_PLL1},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL2, IMX8MP_SYS_PLL2},
    {IMX_SYS_PLL3, IMX8MP_SYS_PLL3},
    {IMX_EXT4, IMX8MP_CLK_EXT4}
};

static struct clk_root_mux_data
imx8mp_media_mipi_phy1_ref_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_SYS_PLL2_DIV3, IMX8MP_SYS_PLL2_DIV3},
    {IMX_SYS_PLL2_DIV10, IMX8MP_SYS_PLL2_DIV10},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL2, IMX8MP_SYS_PLL2},
    {IMX_EXT2, IMX8MP_CLK_EXT2},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_VIDEO_PLL, IMX8MP_VIDEO_PLL1}
};

static struct clk_root_mux_data
imx8mp_media_ldb_sels_data[IMX_CLK_ROOT_MUX_MAX_PARENTS] = {
    {IMX_24M_REF, IMX8MP_CLK_24M},
    {IMX_SYS_PLL2_DIV3, IMX8MP_SYS_PLL2_DIV3},
    {IMX_SYS_PLL2_DIV10, IMX8MP_SYS_PLL2_DIV10},
    {IMX_SYS_PLL1, IMX8MP_SYS_PLL1},
    {IMX_SYS_PLL2, IMX8MP_SYS_PLL2},
    {IMX_EXT2, IMX8MP_CLK_EXT2},
    {IMX_AUDIO_PLL2, IMX8MP_AUDIO_PLL2},
    {IMX_VIDEO_PLL, IMX8MP_VIDEO_PLL1}
};

static struct clk_root_mux imx8mp_media_axi_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];
static struct clk_root_mux imx8mp_media_apb_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];
static struct clk_root_mux imx8mp_media_disp1_pix_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];
static struct clk_root_mux imx8mp_media_disp2_pix_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];
static struct clk_root_mux imx8mp_media_mipi_phy1_ref_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];
static struct clk_root_mux imx8mp_media_ldb_sels[IMX_CLK_ROOT_MUX_MAX_PARENTS];

/**
 * Initalize clock slice selector structure.
 *
 * @param  dev              device with the clock tree
 * @param  clk_sels_data    structure prefilled with parent ids of the clock slice
 * @param  clk_sels         clock slice object to be pre=filled
 */
static void fill_clk_sels(const imx8mp_clk_device_t *dev,
    const struct clk_root_mux_data *clk_sels_data, struct clk_root_mux *clk_sels)
{
    NT_ASSERT(dev);

    for (int i = 0; i < IMX_CLK_ROOT_MUX_MAX_PARENTS; i++) {
        clk_sels[i].parent_id = clk_sels_data[i].parent_id;
        clk_sels[i].parent_clock = dev->clks[clk_sels_data[i].parent_clock];
    }
}

/**
 * Check integrity of the clock tree.
 *
 * @param  dev              device with the clock tree
 * @param  cnt              number of clock tree nodes (typically IMX8_DC_CLK_END)
 * @return 0 on success, -1 on fail
 */
static int imx_check_clks(const imx8mp_clk_device_t *dev, int cnt)
{
    NT_ASSERT(dev);

    for (int i = 0; i < cnt; i++) {
        if (dev->clks[i] == NULL) {
            CLK_PRINTK("Display CLK: Clock #%d initialization failed", i);
            return -1;
        }
    }
    return 0;
}

/**
 * Get specific clock node according to index.
 *
 * @param  index index of clock node. See "Clock tree items - START" table
 * @return 0 on success, -1 on fail
 */
imx_clk_t *clk_get_item_imx8mp(int index)
{
    if (index < IMX8MP_DC_CLK_END) {
        return clktree.clks[index];
    }
    return NULL;
}

/* --------------------------- High-level clock configuration functions --------------------------------*/

/**
 * Initializes clock tree. Call for each display interface, so shared mode can be addressed accordingly
 *
 * @param  dev device for which initialize the clock tree.
 * @return NULL on fail, poiter to clktree on success.
 */
imx8mp_clk_device_t *clk_init_imx8mp(imx_display_interface new_disp)
{
    const struct imx_pll_rate_table *rate_table;
    imx8mp_clk_device_t *dev = &clktree;
    imx_display_interface last_disp_lvds = dev->disp_lvds;
    imx_display_interface last_disp_mipi = dev->disp_mipi;

    if (dev == NULL) {
        return NULL;
    }

    switch (new_disp) {
    case imx_lvds:
        rate_table = videopll_tab_24m_lvds;
        dev->disp_lvds = new_disp;
        break;
    case imx_mipi_dsi:
        rate_table = videopll_tab_24m_mipi;
        dev->disp_mipi = new_disp;
        break;
    default:
        CLK_PRINTK("Display CLK: unsupported interface %d!", new_disp);
        return NULL;
    }

    if ((last_disp_lvds != imx_interface_undefined) || (last_disp_mipi != imx_interface_undefined)) {
        /* TODO: clock tree has already been initialized, which means it is shared between LVDS and MIPI_DSI.
           initialize rate_table to values that can be used for both interfaces
           and then set dev->clks[IMX8MP_VIDEO_PLL1] with new rate_table */

        /* Don't initialize the rest of the clock tree again */
        return dev;
    }

    dev->ccm_regptr = ioremap(IMX_CCM_BASE, IMX_CCM_SIZE);
    if (dev->ccm_regptr == NULL) {
        CLK_PRINTK("Display CLK: mmap of CCM regs failed!");
        return NULL;
    }
    dev->ccm_analog_regptr = ioremap(IMX_CCM_ANALOG_BASE, IMX_CCM_ANALOG_SIZE);
    if (dev->ccm_analog_regptr == NULL) {
        CLK_PRINTK("Display CLK: mmap of CCM ANALOG regs failed!");
        return NULL;
    }
    dev->mediamix_regptr = ioremap(IMX_MEDIAMIX_BASE, IMX_MEDIAMIX_SIZE);
    if (dev->mediamix_regptr == NULL) {
        CLK_PRINTK("Display CLK: mmap of MEIDIAMIX regs failed!");
        return NULL;
    }

    /* clock sources */
    dev->clks[IMX8MP_CLK_DUMMY] = imx_clk_fixed("dummy", NULL, IMX_CLK_FREQ_0K);
    dev->clks[IMX8MP_CLK_24M] = imx_clk_fixed("osc_24m", NULL, IMX_CLK_FREQ_24M);
    dev->clks[IMX8MP_CLK_32K] = imx_clk_fixed("osc_32k", NULL, IMX_CLK_FREQ_32K);
    dev->clks[IMX8MP_CLK_EXT1] = imx_clk_fixed("clk_ext1", NULL, IMX_CLK_FREQ_0K);
    dev->clks[IMX8MP_CLK_EXT2] = imx_clk_fixed("clk_ext2", NULL, IMX_CLK_FREQ_0K);
    dev->clks[IMX8MP_CLK_EXT3] = imx_clk_fixed("clk_ext3", NULL, IMX_CLK_FREQ_0K);
    dev->clks[IMX8MP_CLK_EXT4] = imx_clk_fixed("clk_ext4", NULL, IMX_CLK_FREQ_0K);
    /* PLLs with fixed frequencies configured in IPL/BSP startup */
    dev->clks[IMX8MP_SYS_PLL1] = imx_clk_fixed("sys_pll1", NULL, IMX_CLK_FREQ_800M);
    dev->clks[IMX8MP_SYS_PLL1_DIV2] = imx_clk_fixed("sys_pll1_div2", NULL, IMX_CLK_FREQ_400M);
    dev->clks[IMX8MP_SYS_PLL1_DIV3] = imx_clk_fixed("sys_pll1_div3", NULL, IMX_CLK_FREQ_266_6M);
    dev->clks[IMX8MP_SYS_PLL1_DIV4] = imx_clk_fixed("sys_pll1_div4", NULL, IMX_CLK_FREQ_200M);
    dev->clks[IMX8MP_SYS_PLL1_DIV5] = imx_clk_fixed("sys_pll1_div5", NULL, IMX_CLK_FREQ_160M);
    dev->clks[IMX8MP_SYS_PLL1_DIV6] = imx_clk_fixed("sys_pll1_div6", NULL, IMX_CLK_FREQ_133_3M);
    dev->clks[IMX8MP_SYS_PLL1_DIV8] = imx_clk_fixed("sys_pll1_div8", NULL, IMX_CLK_FREQ_100M);
    dev->clks[IMX8MP_SYS_PLL1_DIV10] = imx_clk_fixed("sys_pll1_div10", NULL, IMX_CLK_FREQ_80M);
    dev->clks[IMX8MP_SYS_PLL1_DIV20] = imx_clk_fixed("sys_pll1_div20", NULL, IMX_CLK_FREQ_40M);
    dev->clks[IMX8MP_SYS_PLL2] = imx_clk_fixed("sys_pll2", NULL, IMX_CLK_FREQ_1000M);
    dev->clks[IMX8MP_SYS_PLL2_DIV2] = imx_clk_fixed("sys_pll2_div2", NULL, IMX_CLK_FREQ_500M);
    dev->clks[IMX8MP_SYS_PLL2_DIV3] = imx_clk_fixed("sys_pll2_div3", NULL, IMX_CLK_FREQ_333_3M);
    dev->clks[IMX8MP_SYS_PLL2_DIV4] = imx_clk_fixed("sys_pll2_div4", NULL, IMX_CLK_FREQ_250M);
    dev->clks[IMX8MP_SYS_PLL2_DIV5] = imx_clk_fixed("sys_pll2_div5", NULL, IMX_CLK_FREQ_200M);
    dev->clks[IMX8MP_SYS_PLL2_DIV6] = imx_clk_fixed("sys_pll2_div6", NULL, IMX_CLK_FREQ_166_6M);
    dev->clks[IMX8MP_SYS_PLL2_DIV8] = imx_clk_fixed("sys_pll2_div8", NULL, IMX_CLK_FREQ_125M);
    dev->clks[IMX8MP_SYS_PLL2_DIV10] = imx_clk_fixed("sys_pll2_div10", NULL, IMX_CLK_FREQ_100M);
    dev->clks[IMX8MP_SYS_PLL2_DIV20] = imx_clk_fixed("sys_pll2_div20", NULL, IMX_CLK_FREQ_50M);
    dev->clks[IMX8MP_SYS_PLL3] = imx_clk_fixed("sys_pll3", NULL, IMX_CLK_FREQ_750M);
    dev->clks[IMX8MP_AUDIO_PLL1] = imx_clk_fixed("audio_pll1", NULL, IMX_CLK_FREQ_786_4M);
    dev->clks[IMX8MP_AUDIO_PLL2] = imx_clk_fixed("audio_pll2", NULL, IMX_CLK_FREQ_722_5M);

    /* configurable video pll*/
    dev->clks[IMX8MP_VIDEO_PLL1] = imx_clk_pll("video_pll1",
        dev->clks[IMX8MP_CLK_24M],
        dev->ccm_analog_regptr, IMX_CCM_ANALOG_VIDEO_PLL1_GEN_CTRL,
        rate_table);

    /* fill cock sources for CCM slices */
    fill_clk_sels(dev, imx8mp_media_axi_sels_data, imx8mp_media_axi_sels);
    fill_clk_sels(dev, imx8mp_media_apb_sels_data, imx8mp_media_apb_sels);
    fill_clk_sels(dev, imx8mp_media_mipi_phy1_ref_sels_data,
        imx8mp_media_mipi_phy1_ref_sels);
    fill_clk_sels(dev, imx8mp_media_disp1_pix_sels_data,
        imx8mp_media_disp1_pix_sels);
    fill_clk_sels(dev, imx8mp_media_disp2_pix_sels_data,
        imx8mp_media_disp2_pix_sels);
    fill_clk_sels(dev, imx8mp_media_ldb_sels_data,
        imx8mp_media_ldb_sels);

    /* clock slices in CCM */
    dev->clks[IMX8MP_CLK_MEDIA_AXI] = imx_clk_slice("media_axi",
        dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_AXI_CLK_ROOT),
        imx8mp_media_axi_sels, &share_axi_slice);
    dev->clks[IMX8MP_CLK_MEDIA_APB] = imx_clk_slice("media_apb",
        dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_APB_CLK_ROOT),
        imx8mp_media_apb_sels, &share_apb_slice);
    dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX] = imx_clk_slice("media_disp2_pix",
        dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_DISP2_PIX_CLK_ROOT),
        imx8mp_media_disp2_pix_sels, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF] = imx_clk_slice(
        "media_mipi_phy1_ref", dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_MIPI_PHY1_REF_CLK_ROOT),
        imx8mp_media_mipi_phy1_ref_sels, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX] = imx_clk_slice("media_disp1_pix",
        dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_DISP1_PIX_CLK_ROOT),
        imx8mp_media_disp1_pix_sels, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LDB] = imx_clk_slice(
        "media_ldb", dev->clks[IMX8MP_CLK_24M],
        dev->ccm_regptr, IMX_CCM_TARGET_ROOTn(IMX8MP_MEDIA_LDB_CLK_ROOT),
        imx8mp_media_ldb_sels, NULL);

    /* clock gate in LPCG Note: only one LPCG shared among all clocks */
    dev->clks[IMX8MP_CLK_MEDIA_AXI_ROOT] = imx_clk_gate(
        "media_axi_root", dev->clks[IMX8MP_CLK_MEDIA_AXI],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);
    dev->clks[IMX8MP_CLK_MEDIA_APB_ROOT] = imx_clk_gate(
        "media_apb_root", dev->clks[IMX8MP_CLK_MEDIA_APB],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);
    dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX_ROOT] = imx_clk_gate(
        "media_disp2_pix_root", dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);
    dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX_ROOT] = imx_clk_gate(
        "media_disp1_pix_root", dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);
    dev->clks[IMX8MP_CLK_MEDIA_LDB_ROOT] = imx_clk_gate(
        "media_ldb_root", dev->clks[IMX8MP_CLK_MEDIA_LDB],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);
    dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF_ROOT] = imx_clk_gate(
        "media_mipi_phy1_ref_root", dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF],
        dev->ccm_regptr, IMX_CCM_CCGRn(IMX8MP_CCM_CCGR_MEDIA), IMX_CCM_BASE,
        0, IMX_CLK_GATE_DOMAIN_ALL_MODES, 3, &share_display_clockgate);

    /* clock gates in BLK_CTRL */
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_AXI] = imx_clk_gate(
        "lcdif1_axi_clk", dev->clks[IMX8MP_CLK_MEDIA_AXI_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF_AXI_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_APB] = imx_clk_gate(
        "lcdif1_apb_clk", dev->clks[IMX8MP_CLK_MEDIA_APB_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF_APB_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_MIPI_DSI_PCLK] = imx_clk_gate(
        "mipi_dsi_pclk", dev->clks[IMX8MP_CLK_MEDIA_APB_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_MIPI_DSI_PCLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_PIXEL] = imx_clk_gate(
        "lcdif1_pixel_clk", dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF_PIXEL_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_AXI] = imx_clk_gate(
        "lcdif2_axi_clk", dev->clks[IMX8MP_CLK_MEDIA_AXI_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF2_AXI_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_APB] = imx_clk_gate(
        "lcdif2_apb_clk", dev->clks[IMX8MP_CLK_MEDIA_APB_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF2_APB_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_PIXEL] = imx_clk_gate(
        "lcdif2_pixel_clk", dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_LCDIF2_PIXEL_CLK_SHIFT, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_MIPI_DSI_CLKREF] = imx_clk_gate(
        "mipi_dsi_clkref", dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF_ROOT],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_MIPI_DSI_CLKREF_MASK, 1, 1, NULL);
    dev->clks[IMX8MP_CLK_MEDIA_BUS_BLK] = imx_clk_gate(
        "bus_blk", dev->clks[IMX8MP_CLK_24M],
        dev->mediamix_regptr, MEDIAMIX_CLK_EN, IMX_MEDIAMIX_BASE,
        SFT_EN_BUS_BLK_CLK_SHIFT, 1, 1, NULL);

#ifdef IMX_CLK_DEBUG
    {
        uint32_t val;
        /* Connect sellected clocks to a test pin */
        val = IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT2_CKE_MASK |
            IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT2_SEL(IMX_CCM_ANALOG_CLKOUT_SYSTEM_PLL3_CLK) |
            IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT2_DIV(3) |
            IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT1_CKE_MASK |
            IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT1_SEL(IMX_CCM_ANALOG_CLKOUT_VIDEO_PLL1_CLK) |
            IMX_CCM_ANALOG_PLL_MNIT_CTL_CLKOUT1_DIV(3);
        *(uint32_t *)(((uint8_t*)dev->ccm_analog_regptr) + IMX_CCM_ANALOG_ANAMIX_PLL_MNIT_CTL) = val;
    }
#endif

    if (imx_check_clks(dev, IMX8MP_DC_CLK_END) != 0) {
        return NULL;
    }
    /* reset in mediamix */
    if (dev->disp_lvds != imx_interface_undefined) {
        mediamix_lvds_reset(dev->mediamix_regptr, 0);
    }
    if (dev->disp_mipi != imx_interface_undefined) {
        mediamix_dsi_reset(dev->mediamix_regptr, 0);
    }

    imx_clk_enable(dev->clks[IMX8MP_CLK_MEDIA_BUS_BLK], true, false);
#ifdef IMX_CLK_DEBUG
    mediamix_dump_reg(dev->mediamix_regptr);
#endif
    return dev;
}

void clk_dump_clock_tree_imx8mp(const imx8mp_clk_device_t *dev)
{
    if (dev == NULL) {
        return;
    }
#ifdef IMX_CLK_DEBUG_DUMP_WHOLE_CLK_TREE
    imx_clk_dump(dev->clks[IMX8MP_CLK_DUMMY]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_24M]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_32K]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_EXT1]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_EXT2]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_EXT3]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_EXT4]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV2]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV3]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV4]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV5]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV6]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV8]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV10]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL1_DIV20]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV2]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV3]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV4]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV5]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV6]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV8]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV10]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL2_DIV20]);
    imx_clk_dump(dev->clks[IMX8MP_SYS_PLL3]);
    imx_clk_dump(dev->clks[IMX8MP_AUDIO_PLL1]);
    imx_clk_dump(dev->clks[IMX8MP_AUDIO_PLL2]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_AXI]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_APB]);
#endif
    imx_clk_dump(dev->clks[IMX8MP_VIDEO_PLL1]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LDB]);
#ifdef IMX_CLK_DEBUG_DUMP_WHOLE_CLK_TREE
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_AXI_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_APB_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_MIPI_PHY1_REF_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LDB_ROOT]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_AXI]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_APB]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF1_PIXEL]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_AXI]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_APB]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_LCDIF2_PIXEL]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_MIPI_DSI_PCLK]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_MIPI_DSI_CLKREF]);
    imx_clk_dump(dev->clks[IMX8MP_CLK_MEDIA_BUS_BLK]);
#endif
}

/**
 * Stop selected clocks.
 *
 * @param  dev device to stop the clock for
 *
 * @return 0 on success. Returns -1 on failure.
 */

int clk_stop_imx8mp(imx8mp_clk_device_t *dev)
{
    if (dev == NULL) {
        return -1;
    }

    if (dev->disp_mipi != imx_interface_undefined) {
        CHECK_SUCCESS(imx_clk_enable(dev->clks[IMX8MP_CLK_MEDIA_DISP1_PIX], false, false));
    }
    if ((dev->disp_lvds != imx_interface_undefined)) {
        CHECK_SUCCESS(imx_clk_enable(dev->clks[IMX8MP_CLK_MEDIA_DISP2_PIX], false, false));
    }

    return 0;
}

/**
 * De-initialize clock tree.
 *
 * @param  dev device to init clock tree for
 *
 * @return 0 on success. Returns -1 on failure.
 */

int clk_deinit_imx8mp(imx8mp_clk_device_t *dev)
{
    int i;

    if (dev == NULL) {
        return -1;
    }

    for (i = 0; i < IMX8MP_DC_CLK_END; i++) {
        if (dev->clks[i] != NULL) {
            imx_clk_uninit(dev->clks[i]);
        }
    }
    if (dev->ccm_regptr != 0) {
        iounmap(dev->ccm_regptr, IMX_CCM_SIZE);
    }
    if (dev->ccm_analog_regptr != 0) {
        iounmap(dev->ccm_analog_regptr, IMX_CCM_ANALOG_SIZE);
    }
    if (dev->mediamix_regptr != 0) {
        iounmap(dev->mediamix_regptr, IMX_MEDIAMIX_SIZE);
    }

    return 0;
}
