/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <dt-bindings/clock/imx6sx-clock.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/types.h>

#include "clk.h"
#include "common.h"

#define CCDR    0x4
#define BM_CCM_CCDR_MMDC_CH0_MASK       (0x2 << 16)

static const char *step_sels[]		= { "osc", "pll2_pfd2_396m", };
static const char *pll1_sw_sels[]	= { "pll1_sys", "step", };
static const char *periph_pre_sels[]	= { "pll2_bus", "pll2_pfd2_396m", "pll2_pfd0_352m", "pll2_198m", };
static const char *periph2_pre_sels[]	= { "pll2_bus", "pll2_pfd2_396m", "pll2_pfd0_352m", "pll4_audio_div", };
static const char *periph_clk2_sels[]	= { "pll3_usb_otg", "osc", "osc", };
static const char *periph2_clk2_sels[]	= { "pll3_usb_otg", "osc", };
static const char *periph_sels[]	= { "periph_pre", "periph_clk2", };
static const char *periph2_sels[]	= { "periph2_pre", "periph2_clk2", };
static const char *ocram_sels[]		= { "periph", "pll2_pfd2_396m", "periph", "pll3_pfd1_540m", };
static const char *audio_sels[]		= { "pll4_audio_div", "pll3_pfd2_508m", "pll5_video_div", "pll3_usb_otg", };
static const char *gpu_axi_sels[]	= { "pll2_pfd2_396m", "pll3_pfd0_720m", "pll3_pfd1_540m", "pll2_bus", };
static const char *gpu_core_sels[]	= { "pll3_pfd1_540m", "pll3_pfd0_720m", "pll2_bus", "pll2_pfd2_396m", };
static const char *ldb_di0_div_sels[]	= { "ldb_di0_div_3_5", "ldb_di0_div_7", };
static const char *ldb_di1_div_sels[]	= { "ldb_di1_div_3_5", "ldb_di1_div_7", };
static const char *ldb_di0_sels[]	= { "pll5_video_div", "pll2_pfd0_352m", "pll2_pfd2_396m", "pll2_pfd3_594m", "pll2_pfd1_594m", "pll3_pfd3_454m", };
static const char *ldb_di1_sels[]	= { "pll3_usb_otg", "pll2_pfd0_352m", "pll2_pfd2_396m", "pll2_bus", "pll3_pfd3_454m", "pll3_pfd2_508m", };
static const char *pcie_axi_sels[]	= { "axi", "ahb", };
static const char *ssi_sels[]		= { "pll3_pfd2_508m", "pll5_video_div", "pll4_audio_div", };
static const char *qspi1_sels[]		= { "pll3_usb_otg", "pll2_pfd0_352m", "pll2_pfd2_396m", "pll2_bus", "pll3_pfd3_454m", "pll3_pfd2_508m", };
static const char *perclk_sels[]	= { "ipg", "osc", };
static const char *usdhc_sels[]		= { "pll2_pfd2_396m", "pll2_pfd0_352m", };
static const char *vid_sels[]		= { "pll3_pfd1_540m", "pll3_usb_otg", "pll3_pfd3_454m", "pll4_audio_div", "pll5_video_div", };
static const char *can_sels[]		= { "pll3_60m", "osc", "pll3_80m", "dummy", };
static const char *uart_sels[]		= { "pll3_80m", "osc", };
static const char *qspi2_sels[]		= { "pll2_pfd0_352m", "pll2_bus", "pll3_usb_otg", "pll2_pfd2_396m", "pll3_pfd3_454m", "dummy", "dummy", "dummy", };
static const char *enet_pre_sels[]	= { "pll2_bus", "pll3_usb_otg", "pll5_video_div", "pll2_pfd0_352m", "pll2_pfd2_396m", "pll3_pfd2_508m", };
static const char *enet_sels[]		= { "enet_podf", "ipp_di0", "ipp_di1", "ldb_di0", "ldb_di1", };
static const char *m4_pre_sels[]	= { "pll2_bus", "pll3_usb_otg", "osc", "pll2_pfd0_352m", "pll2_pfd2_396m", "pll3_pfd3_454m", };
static const char *m4_sels[]		= { "m4_pre_sel", "ipp_di0", "ipp_di1", "ldb_di0", "ldb_di1", };
static const char *eim_slow_sels[]	= { "ocram", "pll3_usb_otg", "pll2_pfd2_396m", "pll2_pfd0_352m", };
static const char *ecspi_sels[]		= { "pll3_60m", "osc", };
static const char *lcdif1_pre_sels[]	= { "pll2_bus", "pll3_pfd3_454m", "pll5_video_div", "pll2_pfd0_352m", "pll2_pfd1_594m", "pll3_pfd1_540m", };
static const char *lcdif1_sels[]	= { "lcdif1_podf", "ipp_di0", "ipp_di1", "ldb_di0", "ldb_di1", };
static const char *lcdif2_pre_sels[]	= { "pll2_bus", "pll3_pfd3_454m", "pll5_video_div", "pll2_pfd0_352m", "pll2_pfd3_594m", "pll3_pfd1_540m", };
static const char *lcdif2_sels[]	= { "lcdif2_podf", "ipp_di0", "ipp_di1", "ldb_di0", "ldb_di1", };
static const char *display_sels[]	= { "pll2_bus", "pll2_pfd2_396m", "pll3_usb_otg", "pll3_pfd1_540m", };
static const char *csi_sels[]		= { "osc", "pll2_pfd2_396m", "pll3_120m", "pll3_pfd1_540m", };
static const char *cko1_sels[]		= {
	"pll3_usb_otg", "pll2_bus", "pll1_sys", "pll5_video_div",
	"dummy", "ocram", "dummy", "pxp_axi", "epdc_axi", "lcdif_pix",
	"epdc_pix", "ahb", "ipg", "perclk", "ckil", "pll4_audio_div",
};
static const char *cko2_sels[]		= {
	"dummy", "mmdc_p0_fast", "usdhc4", "usdhc1", "dummy", "wrck",
	"ecspi_root", "dummy", "usdhc3", "pcie", "arm", "csi_core",
	"lcdif_axi", "dummy", "osc", "dummy", "gpu2d_ovg_core",
	"usdhc2", "ssi1", "ssi2", "ssi3", "gpu2d_core", "dummy",
	"dummy", "dummy", "dummy", "esai_extal", "eim_slow", "uart_serial",
	"spdif", "asrc", "dummy",
};
static const char *cko_sels[] = { "cko1", "cko2", };
static const char *lvds_sels[]	= {
	"arm", "pll1_sys", "dummy", "dummy", "dummy", "dummy", "dummy", "pll5_video_div",
	"dummy", "dummy", "pcie_ref_125m", "dummy", "usbphy1", "usbphy2",
};
static const char *pll_bypass_src_sels[] = { "osc", "lvds1_in", };
static const char *pll1_bypass_sels[] = { "pll1", "pll1_bypass_src", };
static const char *pll2_bypass_sels[] = { "pll2", "pll2_bypass_src", };
static const char *pll3_bypass_sels[] = { "pll3", "pll3_bypass_src", };
static const char *pll4_bypass_sels[] = { "pll4", "pll4_bypass_src", };
static const char *pll5_bypass_sels[] = { "pll5", "pll5_bypass_src", };
static const char *pll6_bypass_sels[] = { "pll6", "pll6_bypass_src", };
static const char *pll7_bypass_sels[] = { "pll7", "pll7_bypass_src", };

static struct clk *clks[IMX6SX_CLK_CLK_END];
static struct clk_onecell_data clk_data;

static int const clks_init_on[] __initconst = {
	IMX6SX_CLK_AIPS_TZ1, IMX6SX_CLK_AIPS_TZ2, IMX6SX_CLK_AIPS_TZ3,
	IMX6SX_CLK_IPMUX1, IMX6SX_CLK_IPMUX2, IMX6SX_CLK_IPMUX3,
	IMX6SX_CLK_WAKEUP, IMX6SX_CLK_MMDC_P0_FAST, IMX6SX_CLK_MMDC_P0_IPG,
	IMX6SX_CLK_ROM, IMX6SX_CLK_ARM, IMX6SX_CLK_IPG, IMX6SX_CLK_OCRAM,
	IMX6SX_CLK_PER2_MAIN, IMX6SX_CLK_PERCLK, IMX6SX_CLK_M4,
	IMX6SX_CLK_QSPI1, IMX6SX_CLK_QSPI2, IMX6SX_CLK_UART_IPG,
	IMX6SX_CLK_UART_SERIAL, IMX6SX_CLK_I2C3, IMX6SX_CLK_ECSPI5,
	IMX6SX_CLK_CAN1_IPG, IMX6SX_CLK_CAN1_SERIAL, IMX6SX_CLK_CAN2_IPG,
	IMX6SX_CLK_CAN2_SERIAL, IMX6SX_CLK_CANFD, IMX6SX_CLK_EPIT1,
	IMX6SX_CLK_EPIT2,
};

static struct clk_div_table clk_enet_ref_table[] = {
	{ .val = 0, .div = 20, },
	{ .val = 1, .div = 10, },
	{ .val = 2, .div = 5, },
	{ .val = 3, .div = 4, },
	{ }
};

static struct clk_div_table post_div_table[] = {
	{ .val = 2, .div = 1, },
	{ .val = 1, .div = 2, },
	{ .val = 0, .div = 4, },
	{ }
};

static struct clk_div_table video_div_table[] = {
	{ .val = 0, .div = 1, },
	{ .val = 1, .div = 2, },
	{ .val = 2, .div = 1, },
	{ .val = 3, .div = 4, },
	{ }
};

static u32 share_count_asrc;
static u32 share_count_audio;
static u32 share_count_esai;
static u32 share_count_ssi1;
static u32 share_count_ssi2;
static u32 share_count_ssi3;

static void __init imx6sx_clocks_init(struct device_node *ccm_node)
{
	struct device_node *np;
	void __iomem *base;
	int i;

	clks[IMX6SX_CLK_DUMMY] = imx_clk_fixed("dummy", 0);

	clks[IMX6SX_CLK_CKIL] = of_clk_get_by_name(ccm_node, "ckil");
	clks[IMX6SX_CLK_OSC] = of_clk_get_by_name(ccm_node, "osc");

	/* ipp_di clock is external input */
	clks[IMX6SX_CLK_IPP_DI0] = of_clk_get_by_name(ccm_node, "ipp_di0");
	clks[IMX6SX_CLK_IPP_DI1] = of_clk_get_by_name(ccm_node, "ipp_di1");

	/* Clock source from external clock via CLK1 PAD */
	clks[IMX6SX_CLK_ANACLK1] = imx_obtain_fixed_clock("anaclk1", 0);

	np = of_find_compatible_node(NULL, NULL, "fsl,imx6sx-anatop");
	base = of_iomap(np, 0);
	WARN_ON(!base);

	clks[IMX6SX_PLL1_BYPASS_SRC] = imx_clk_mux("pll1_bypass_src", base + 0x00, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL2_BYPASS_SRC] = imx_clk_mux("pll2_bypass_src", base + 0x30, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL3_BYPASS_SRC] = imx_clk_mux("pll3_bypass_src", base + 0x10, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL4_BYPASS_SRC] = imx_clk_mux("pll4_bypass_src", base + 0x70, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL5_BYPASS_SRC] = imx_clk_mux("pll5_bypass_src", base + 0xa0, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL6_BYPASS_SRC] = imx_clk_mux("pll6_bypass_src", base + 0xe0, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));
	clks[IMX6SX_PLL7_BYPASS_SRC] = imx_clk_mux("pll7_bypass_src", base + 0x20, 14, 1, pll_bypass_src_sels, ARRAY_SIZE(pll_bypass_src_sels));

	/*                                    type               name    parent_name        base         div_mask */
	clks[IMX6SX_CLK_PLL1] = imx_clk_pllv3(IMX_PLLV3_SYS,     "pll1", "pll1_bypass_src", base + 0x00, 0x7f);
	clks[IMX6SX_CLK_PLL2] = imx_clk_pllv3(IMX_PLLV3_GENERIC, "pll2", "pll2_bypass_src", base + 0x30, 0x1);
	clks[IMX6SX_CLK_PLL3] = imx_clk_pllv3(IMX_PLLV3_USB,     "pll3", "pll3_bypass_src", base + 0x10, 0x3);
	clks[IMX6SX_CLK_PLL4] = imx_clk_pllv3(IMX_PLLV3_AV,      "pll4", "pll4_bypass_src", base + 0x70, 0x7f);
	clks[IMX6SX_CLK_PLL5] = imx_clk_pllv3(IMX_PLLV3_AV,      "pll5", "pll5_bypass_src", base + 0xa0, 0x7f);
	clks[IMX6SX_CLK_PLL6] = imx_clk_pllv3(IMX_PLLV3_ENET,    "pll6", "pll6_bypass_src", base + 0xe0, 0x3);
	clks[IMX6SX_CLK_PLL7] = imx_clk_pllv3(IMX_PLLV3_USB,     "pll7", "pll7_bypass_src", base + 0x20, 0x3);

	clks[IMX6SX_PLL1_BYPASS] = imx_clk_mux_flags("pll1_bypass", base + 0x00, 16, 1, pll1_bypass_sels, ARRAY_SIZE(pll1_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL2_BYPASS] = imx_clk_mux_flags("pll2_bypass", base + 0x30, 16, 1, pll2_bypass_sels, ARRAY_SIZE(pll2_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL3_BYPASS] = imx_clk_mux_flags("pll3_bypass", base + 0x10, 16, 1, pll3_bypass_sels, ARRAY_SIZE(pll3_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL4_BYPASS] = imx_clk_mux_flags("pll4_bypass", base + 0x70, 16, 1, pll4_bypass_sels, ARRAY_SIZE(pll4_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL5_BYPASS] = imx_clk_mux_flags("pll5_bypass", base + 0xa0, 16, 1, pll5_bypass_sels, ARRAY_SIZE(pll5_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL6_BYPASS] = imx_clk_mux_flags("pll6_bypass", base + 0xe0, 16, 1, pll6_bypass_sels, ARRAY_SIZE(pll6_bypass_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_PLL7_BYPASS] = imx_clk_mux_flags("pll7_bypass", base + 0x20, 16, 1, pll7_bypass_sels, ARRAY_SIZE(pll7_bypass_sels), CLK_SET_RATE_PARENT);

	/* Do not bypass PLLs initially */
	clk_set_parent(clks[IMX6SX_PLL1_BYPASS], clks[IMX6SX_CLK_PLL1]);
	clk_set_parent(clks[IMX6SX_PLL2_BYPASS], clks[IMX6SX_CLK_PLL2]);
	clk_set_parent(clks[IMX6SX_PLL3_BYPASS], clks[IMX6SX_CLK_PLL3]);
	clk_set_parent(clks[IMX6SX_PLL4_BYPASS], clks[IMX6SX_CLK_PLL4]);
	clk_set_parent(clks[IMX6SX_PLL5_BYPASS], clks[IMX6SX_CLK_PLL5]);
	clk_set_parent(clks[IMX6SX_PLL6_BYPASS], clks[IMX6SX_CLK_PLL6]);
	clk_set_parent(clks[IMX6SX_PLL7_BYPASS], clks[IMX6SX_CLK_PLL7]);

	clks[IMX6SX_CLK_PLL1_SYS]      = imx_clk_gate("pll1_sys",      "pll1_bypass", base + 0x00, 13);
	clks[IMX6SX_CLK_PLL2_BUS]      = imx_clk_gate("pll2_bus",      "pll2_bypass", base + 0x30, 13);
	clks[IMX6SX_CLK_PLL3_USB_OTG]  = imx_clk_gate("pll3_usb_otg",  "pll3_bypass", base + 0x10, 13);
	clks[IMX6SX_CLK_PLL4_AUDIO]    = imx_clk_gate("pll4_audio",    "pll4_bypass", base + 0x70, 13);
	clks[IMX6SX_CLK_PLL5_VIDEO]    = imx_clk_gate("pll5_video",    "pll5_bypass", base + 0xa0, 13);
	clks[IMX6SX_CLK_PLL6_ENET]     = imx_clk_gate("pll6_enet",     "pll6_bypass", base + 0xe0, 13);
	clks[IMX6SX_CLK_PLL7_USB_HOST] = imx_clk_gate("pll7_usb_host", "pll7_bypass", base + 0x20, 13);

	/*
	 * Bit 20 is the reserved and read-only bit, we do this only for:
	 * - Do nothing for usbphy clk_enable/disable
	 * - Keep refcount when do usbphy clk_enable/disable, in that case,
	 * the clk framework may need to enable/disable usbphy's parent
	 */
	clks[IMX6SX_CLK_USBPHY1] = imx_clk_gate("usbphy1", "pll3_usb_otg",  base + 0x10, 20);
	clks[IMX6SX_CLK_USBPHY2] = imx_clk_gate("usbphy2", "pll7_usb_host", base + 0x20, 20);

	/*
	 * usbphy*_gate needs to be on after system boots up, and software
	 * never needs to control it anymore.
	 */
	clks[IMX6SX_CLK_USBPHY1_GATE] = imx_clk_gate("usbphy1_gate", "dummy", base + 0x10, 6);
	clks[IMX6SX_CLK_USBPHY2_GATE] = imx_clk_gate("usbphy2_gate", "dummy", base + 0x20, 6);

	/* FIXME 100Mhz is used for pcie ref for all imx6 pcie, excepted imx6q */
	clks[IMX6SX_CLK_PCIE_REF] = imx_clk_fixed_factor("pcie_ref", "pll6_enet", 1, 5);
	clks[IMX6SX_CLK_PCIE_REF_125M] = imx_clk_gate("pcie_ref_125m", "pcie_ref", base + 0xe0, 19);

	clks[IMX6SX_CLK_LVDS1_OUT] = imx_clk_gate_exclusive("lvds1_out", "lvds1_sel", base + 0x160, 10, BIT(12));
	clks[IMX6SX_CLK_LVDS1_IN]  = imx_clk_gate_exclusive("lvds1_in",  "anaclk1",   base + 0x160, 12, BIT(10));

	clks[IMX6SX_CLK_ENET_REF] = clk_register_divider_table(NULL, "enet_ref", "pll6_enet", 0,
			base + 0xe0, 0, 2, 0, clk_enet_ref_table,
			&imx_ccm_lock);
	clks[IMX6SX_CLK_ENET2_REF] = clk_register_divider_table(NULL, "enet2_ref", "pll6_enet", 0,
			base + 0xe0, 2, 2, 0, clk_enet_ref_table,
			&imx_ccm_lock);
	clks[IMX6SX_CLK_ENET2_REF_125M] = imx_clk_gate("enet2_ref_125m", "enet2_ref", base + 0xe0, 20);

	clks[IMX6SX_CLK_ENET_PTP_REF] = imx_clk_fixed_factor("enet_ptp_ref", "pll6_enet", 1, 20);
	clks[IMX6SX_CLK_ENET_PTP] = imx_clk_gate("enet_ptp_25m", "enet_ptp_ref", base + 0xe0, 21);

	/*                                       name              parent_name     reg           idx */
	clks[IMX6SX_CLK_PLL2_PFD0] = imx_clk_pfd("pll2_pfd0_352m", "pll2_bus",     base + 0x100, 0);
	clks[IMX6SX_CLK_PLL2_PFD1] = imx_clk_pfd("pll2_pfd1_594m", "pll2_bus",     base + 0x100, 1);
	clks[IMX6SX_CLK_PLL2_PFD2] = imx_clk_pfd("pll2_pfd2_396m", "pll2_bus",     base + 0x100, 2);
	clks[IMX6SX_CLK_PLL2_PFD3] = imx_clk_pfd("pll2_pfd3_594m", "pll2_bus",     base + 0x100, 3);
	clks[IMX6SX_CLK_PLL3_PFD0] = imx_clk_pfd("pll3_pfd0_720m", "pll3_usb_otg", base + 0xf0,  0);
	clks[IMX6SX_CLK_PLL3_PFD1] = imx_clk_pfd("pll3_pfd1_540m", "pll3_usb_otg", base + 0xf0,  1);
	clks[IMX6SX_CLK_PLL3_PFD2] = imx_clk_pfd("pll3_pfd2_508m", "pll3_usb_otg", base + 0xf0,  2);
	clks[IMX6SX_CLK_PLL3_PFD3] = imx_clk_pfd("pll3_pfd3_454m", "pll3_usb_otg", base + 0xf0,  3);

	/*                                                name         parent_name       mult div */
	clks[IMX6SX_CLK_PLL2_198M] = imx_clk_fixed_factor("pll2_198m", "pll2_pfd2_396m", 1,   2);
	clks[IMX6SX_CLK_PLL3_120M] = imx_clk_fixed_factor("pll3_120m", "pll3_usb_otg",   1,   4);
	clks[IMX6SX_CLK_PLL3_80M]  = imx_clk_fixed_factor("pll3_80m",  "pll3_usb_otg",   1,   6);
	clks[IMX6SX_CLK_PLL3_60M]  = imx_clk_fixed_factor("pll3_60m",  "pll3_usb_otg",   1,   8);
	clks[IMX6SX_CLK_TWD]       = imx_clk_fixed_factor("twd",       "arm",            1,   2);
	clks[IMX6SX_CLK_GPT_3M]    = imx_clk_fixed_factor("gpt_3m",    "osc",            1,   8);

	clks[IMX6SX_CLK_PLL4_POST_DIV]  = clk_register_divider_table(NULL, "pll4_post_div", "pll4_audio",
				CLK_SET_RATE_PARENT, base + 0x70, 19, 2, 0, post_div_table, &imx_ccm_lock);
	clks[IMX6SX_CLK_PLL4_AUDIO_DIV] = clk_register_divider(NULL, "pll4_audio_div", "pll4_post_div",
				CLK_SET_RATE_PARENT, base + 0x170, 15, 1, 0, &imx_ccm_lock);
	clks[IMX6SX_CLK_PLL5_POST_DIV]  = clk_register_divider_table(NULL, "pll5_post_div", "pll5_video",
				CLK_SET_RATE_PARENT, base + 0xa0, 19, 2, 0, post_div_table, &imx_ccm_lock);
	clks[IMX6SX_CLK_PLL5_VIDEO_DIV] = clk_register_divider_table(NULL, "pll5_video_div", "pll5_post_div",
				CLK_SET_RATE_PARENT, base + 0x170, 30, 2, 0, video_div_table, &imx_ccm_lock);

	/*                                                name                reg           shift   width   parent_names       num_parents */
	clks[IMX6SX_CLK_LVDS1_SEL]          = imx_clk_mux("lvds1_sel",        base + 0x160, 0,      5,      lvds_sels,         ARRAY_SIZE(lvds_sels));

	np = ccm_node;
	base = of_iomap(np, 0);
	WARN_ON(!base);

	imx6q_pm_set_ccm_base(base);

	/*                                                name                reg           shift   width   parent_names       num_parents */
	clks[IMX6SX_CLK_STEP]               = imx_clk_mux("step",             base + 0xc,   8,      1,      step_sels,         ARRAY_SIZE(step_sels));
	clks[IMX6SX_CLK_PLL1_SW]            = imx_clk_mux("pll1_sw",          base + 0xc,   2,      1,      pll1_sw_sels,      ARRAY_SIZE(pll1_sw_sels));
	clks[IMX6SX_CLK_OCRAM_SEL]          = imx_clk_mux("ocram_sel",        base + 0x14,  6,      2,      ocram_sels,        ARRAY_SIZE(ocram_sels));
	clks[IMX6SX_CLK_PERIPH_PRE]         = imx_clk_mux("periph_pre",       base + 0x18,  18,     2,      periph_pre_sels,   ARRAY_SIZE(periph_pre_sels));
	clks[IMX6SX_CLK_PERIPH2_PRE]        = imx_clk_mux("periph2_pre",      base + 0x18,  21,     2,      periph2_pre_sels,   ARRAY_SIZE(periph2_pre_sels));
	clks[IMX6SX_CLK_PERIPH_CLK2_SEL]    = imx_clk_mux("periph_clk2_sel",  base + 0x18,  12,     2,      periph_clk2_sels,  ARRAY_SIZE(periph_clk2_sels));
	clks[IMX6SX_CLK_PERIPH2_CLK2_SEL]   = imx_clk_mux("periph2_clk2_sel", base + 0x18,  20,     1,      periph2_clk2_sels, ARRAY_SIZE(periph2_clk2_sels));
	clks[IMX6SX_CLK_PCIE_AXI_SEL]       = imx_clk_mux("pcie_axi_sel",     base + 0x18,  10,     1,      pcie_axi_sels,     ARRAY_SIZE(pcie_axi_sels));
	clks[IMX6SX_CLK_GPU_AXI_SEL]        = imx_clk_mux("gpu_axi_sel",      base + 0x18,  8,      2,      gpu_axi_sels,      ARRAY_SIZE(gpu_axi_sels));
	clks[IMX6SX_CLK_GPU_CORE_SEL]       = imx_clk_mux("gpu_core_sel",     base + 0x18,  4,      2,      gpu_core_sels,     ARRAY_SIZE(gpu_core_sels));
	clks[IMX6SX_CLK_EIM_SLOW_SEL]       = imx_clk_mux("eim_slow_sel",     base + 0x1c,  29,     2,      eim_slow_sels,     ARRAY_SIZE(eim_slow_sels));
	clks[IMX6SX_CLK_USDHC1_SEL]         = imx_clk_mux("usdhc1_sel",       base + 0x1c,  16,     1,      usdhc_sels,        ARRAY_SIZE(usdhc_sels));
	clks[IMX6SX_CLK_USDHC2_SEL]         = imx_clk_mux("usdhc2_sel",       base + 0x1c,  17,     1,      usdhc_sels,        ARRAY_SIZE(usdhc_sels));
	clks[IMX6SX_CLK_USDHC3_SEL]         = imx_clk_mux("usdhc3_sel",       base + 0x1c,  18,     1,      usdhc_sels,        ARRAY_SIZE(usdhc_sels));
	clks[IMX6SX_CLK_USDHC4_SEL]         = imx_clk_mux("usdhc4_sel",       base + 0x1c,  19,     1,      usdhc_sels,        ARRAY_SIZE(usdhc_sels));
	clks[IMX6SX_CLK_SSI3_SEL]           = imx_clk_mux("ssi3_sel",         base + 0x1c,  14,     2,      ssi_sels,          ARRAY_SIZE(ssi_sels));
	clks[IMX6SX_CLK_SSI2_SEL]           = imx_clk_mux("ssi2_sel",         base + 0x1c,  12,     2,      ssi_sels,          ARRAY_SIZE(ssi_sels));
	clks[IMX6SX_CLK_SSI1_SEL]           = imx_clk_mux("ssi1_sel",         base + 0x1c,  10,     2,      ssi_sels,          ARRAY_SIZE(ssi_sels));
	clks[IMX6SX_CLK_QSPI1_SEL]          = imx_clk_mux_flags("qspi1_sel", base + 0x1c,  7, 3, qspi1_sels, ARRAY_SIZE(qspi1_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_PERCLK_SEL]         = imx_clk_mux("perclk_sel",       base + 0x1c,  6,      1,      perclk_sels,       ARRAY_SIZE(perclk_sels));
	clks[IMX6SX_CLK_VID_SEL]            = imx_clk_mux("vid_sel",          base + 0x20,  21,     3,      vid_sels,          ARRAY_SIZE(vid_sels));
	clks[IMX6SX_CLK_ESAI_SEL]           = imx_clk_mux("esai_sel",         base + 0x20,  19,     2,      audio_sels,        ARRAY_SIZE(audio_sels));
	clks[IMX6SX_CLK_CAN_SEL]            = imx_clk_mux("can_sel",          base + 0x20,  8,      2,      can_sels,          ARRAY_SIZE(can_sels));
	clks[IMX6SX_CLK_UART_SEL]           = imx_clk_mux("uart_sel",         base + 0x24,  6,      1,      uart_sels,         ARRAY_SIZE(uart_sels));
	clks[IMX6SX_CLK_QSPI2_SEL]          = imx_clk_mux_flags("qspi2_sel", base + 0x2c, 15, 3, qspi2_sels, ARRAY_SIZE(qspi2_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_SPDIF_SEL]          = imx_clk_mux("spdif_sel",        base + 0x30,  20,     2,      audio_sels,        ARRAY_SIZE(audio_sels));
	clks[IMX6SX_CLK_AUDIO_SEL]          = imx_clk_mux("audio_sel",        base + 0x30,  7,      2,      audio_sels,        ARRAY_SIZE(audio_sels));
	clks[IMX6SX_CLK_ENET_PRE_SEL]       = imx_clk_mux("enet_pre_sel",     base + 0x34,  15,     3,      enet_pre_sels,     ARRAY_SIZE(enet_pre_sels));
	clks[IMX6SX_CLK_ENET_SEL]           = imx_clk_mux("enet_sel",         base + 0x34,  9,      3,      enet_sels,         ARRAY_SIZE(enet_sels));
	clks[IMX6SX_CLK_M4_PRE_SEL]         = imx_clk_mux("m4_pre_sel",       base + 0x34,  6,      3,      m4_pre_sels,       ARRAY_SIZE(m4_pre_sels));
	clks[IMX6SX_CLK_M4_SEL]             = imx_clk_mux("m4_sel",           base + 0x34,  0,      3,      m4_sels,           ARRAY_SIZE(m4_sels));
	clks[IMX6SX_CLK_ECSPI_SEL]          = imx_clk_mux("ecspi_sel",        base + 0x38,  18,     1,      ecspi_sels,        ARRAY_SIZE(ecspi_sels));
	clks[IMX6SX_CLK_LCDIF2_PRE_SEL]     = imx_clk_mux("lcdif2_pre_sel",   base + 0x38,  6,      3,      lcdif2_pre_sels,   ARRAY_SIZE(lcdif2_pre_sels));
	clks[IMX6SX_CLK_LCDIF2_SEL]         = imx_clk_mux("lcdif2_sel",       base + 0x38,  0,      3,      lcdif2_sels,       ARRAY_SIZE(lcdif2_sels));
	clks[IMX6SX_CLK_DISPLAY_SEL]        = imx_clk_mux("display_sel",      base + 0x3c,  14,     2,      display_sels,      ARRAY_SIZE(display_sels));
	clks[IMX6SX_CLK_CSI_SEL]            = imx_clk_mux("csi_sel",          base + 0x3c,  9,      2,      csi_sels,          ARRAY_SIZE(csi_sels));
	clks[IMX6SX_CLK_CKO1_SEL]           = imx_clk_mux("cko1_sel",         base + 0x60,  0,      4,      cko1_sels,         ARRAY_SIZE(cko1_sels));
	clks[IMX6SX_CLK_CKO2_SEL]           = imx_clk_mux("cko2_sel",         base + 0x60,  16,     5,      cko2_sels,         ARRAY_SIZE(cko2_sels));
	clks[IMX6SX_CLK_CKO]                = imx_clk_mux("cko",              base + 0x60,  8,      1,      cko_sels,          ARRAY_SIZE(cko_sels));

	clks[IMX6SX_CLK_LDB_DI1_DIV_SEL]    = imx_clk_mux_flags("ldb_di1_div_sel", base + 0x20, 11, 1, ldb_di1_div_sels, ARRAY_SIZE(ldb_di1_div_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_LDB_DI0_DIV_SEL]    = imx_clk_mux_flags("ldb_di0_div_sel", base + 0x20, 10, 1, ldb_di0_div_sels, ARRAY_SIZE(ldb_di0_div_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_LDB_DI1_SEL]        = imx_clk_mux_flags("ldb_di1_sel",     base + 0x2c, 12, 3, ldb_di1_sels,      ARRAY_SIZE(ldb_di1_sels),    CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_LDB_DI0_SEL]        = imx_clk_mux_flags("ldb_di0_sel",     base + 0x2c, 9,  3, ldb_di0_sels,      ARRAY_SIZE(ldb_di0_sels),    CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_LCDIF1_PRE_SEL]     = imx_clk_mux_flags("lcdif1_pre_sel",  base + 0x38, 15, 3, lcdif1_pre_sels,   ARRAY_SIZE(lcdif1_pre_sels), CLK_SET_RATE_PARENT);
	clks[IMX6SX_CLK_LCDIF1_SEL]         = imx_clk_mux_flags("lcdif1_sel",      base + 0x38, 9,  3, lcdif1_sels,       ARRAY_SIZE(lcdif1_sels),     CLK_SET_RATE_PARENT);

	/*                                                    name              parent_name          reg          shift width */
	clks[IMX6SX_CLK_PERIPH_CLK2]        = imx_clk_divider("periph_clk2",    "periph_clk2_sel",   base + 0x14, 27,   3);
	clks[IMX6SX_CLK_PERIPH2_CLK2]       = imx_clk_divider("periph2_clk2",   "periph2_clk2_sel",  base + 0x14, 0,    3);
	clks[IMX6SX_CLK_IPG]                = imx_clk_divider("ipg",            "ahb",               base + 0x14, 8,    2);
	clks[IMX6SX_CLK_GPU_CORE_PODF]      = imx_clk_divider("gpu_core_podf",  "gpu_core_sel",      base + 0x18, 29,   3);
	clks[IMX6SX_CLK_GPU_AXI_PODF]       = imx_clk_divider("gpu_axi_podf",   "gpu_axi_sel",       base + 0x18, 26,   3);
	clks[IMX6SX_CLK_LCDIF1_PODF]        = imx_clk_divider("lcdif1_podf",    "lcdif1_pred",       base + 0x18, 23,   3);
	clks[IMX6SX_CLK_QSPI1_PODF]         = imx_clk_divider("qspi1_podf",     "qspi1_sel",         base + 0x1c, 26,   3);
	clks[IMX6SX_CLK_EIM_SLOW_PODF]      = imx_clk_divider("eim_slow_podf",  "eim_slow_sel",      base + 0x1c, 23,   3);
	clks[IMX6SX_CLK_LCDIF2_PODF]        = imx_clk_divider("lcdif2_podf",    "lcdif2_pred",       base + 0x1c, 20,   3);
	clks[IMX6SX_CLK_PERCLK]             = imx_clk_divider("perclk",         "perclk_sel",        base + 0x1c, 0,    6);
	clks[IMX6SX_CLK_VID_PODF]           = imx_clk_divider("vid_podf",       "vid_sel",           base + 0x20, 24,   2);
	clks[IMX6SX_CLK_CAN_PODF]           = imx_clk_divider("can_podf",       "can_sel",           base + 0x20, 2,    6);
	clks[IMX6SX_CLK_USDHC4_PODF]        = imx_clk_divider("usdhc4_podf",    "usdhc4_sel",        base + 0x24, 22,   3);
	clks[IMX6SX_CLK_USDHC3_PODF]        = imx_clk_divider("usdhc3_podf",    "usdhc3_sel",        base + 0x24, 19,   3);
	clks[IMX6SX_CLK_USDHC2_PODF]        = imx_clk_divider("usdhc2_podf",    "usdhc2_sel",        base + 0x24, 16,   3);
	clks[IMX6SX_CLK_USDHC1_PODF]        = imx_clk_divider("usdhc1_podf",    "usdhc1_sel",        base + 0x24, 11,   3);
	clks[IMX6SX_CLK_UART_PODF]          = imx_clk_divider("uart_podf",      "uart_sel",          base + 0x24, 0,    6);
	clks[IMX6SX_CLK_ESAI_PRED]          = imx_clk_divider("esai_pred",      "esai_sel",          base + 0x28, 9,    3);
	clks[IMX6SX_CLK_ESAI_PODF]          = imx_clk_divider("esai_podf",      "esai_pred",         base + 0x28, 25,   3);
	clks[IMX6SX_CLK_SSI3_PRED]          = imx_clk_divider("ssi3_pred",      "ssi3_sel",          base + 0x28, 22,   3);
	clks[IMX6SX_CLK_SSI3_PODF]          = imx_clk_divider("ssi3_podf",      "ssi3_pred",         base + 0x28, 16,   6);
	clks[IMX6SX_CLK_SSI1_PRED]          = imx_clk_divider("ssi1_pred",      "ssi1_sel",          base + 0x28, 6,    3);
	clks[IMX6SX_CLK_SSI1_PODF]          = imx_clk_divider("ssi1_podf",      "ssi1_pred",         base + 0x28, 0,    6);
	clks[IMX6SX_CLK_QSPI2_PRED]         = imx_clk_divider("qspi2_pred",     "qspi2_sel",         base + 0x2c, 18,   3);
	clks[IMX6SX_CLK_QSPI2_PODF]         = imx_clk_divider("qspi2_podf",     "qspi2_pred",        base + 0x2c, 21,   6);
	clks[IMX6SX_CLK_SSI2_PRED]          = imx_clk_divider("ssi2_pred",      "ssi2_sel",          base + 0x2c, 6,    3);
	clks[IMX6SX_CLK_SSI2_PODF]          = imx_clk_divider("ssi2_podf",      "ssi2_pred",         base + 0x2c, 0,    6);
	clks[IMX6SX_CLK_SPDIF_PRED]         = imx_clk_divider("spdif_pred",     "spdif_sel",         base + 0x30, 25,   3);
	clks[IMX6SX_CLK_SPDIF_PODF]         = imx_clk_divider("spdif_podf",     "spdif_pred",        base + 0x30, 22,   3);
	clks[IMX6SX_CLK_AUDIO_PRED]         = imx_clk_divider("audio_pred",     "audio_sel",         base + 0x30, 12,   3);
	clks[IMX6SX_CLK_AUDIO_PODF]         = imx_clk_divider("audio_podf",     "audio_pred",        base + 0x30, 9,    3);
	clks[IMX6SX_CLK_ENET_PODF]          = imx_clk_divider("enet_podf",      "enet_pre_sel",      base + 0x34, 12,   3);
	clks[IMX6SX_CLK_M4_PODF]            = imx_clk_divider("m4_podf",        "m4_sel",            base + 0x34, 3,    3);
	clks[IMX6SX_CLK_ECSPI_PODF]         = imx_clk_divider("ecspi_podf",     "ecspi_sel",         base + 0x38, 19,   6);
	clks[IMX6SX_CLK_LCDIF1_PRED]        = imx_clk_divider("lcdif1_pred",    "lcdif1_pre_sel",    base + 0x38, 12,   3);
	clks[IMX6SX_CLK_LCDIF2_PRED]        = imx_clk_divider("lcdif2_pred",    "lcdif2_pre_sel",    base + 0x38, 3,    3);
	clks[IMX6SX_CLK_DISPLAY_PODF]       = imx_clk_divider("display_podf",   "display_sel",       base + 0x3c, 16,   3);
	clks[IMX6SX_CLK_CSI_PODF]           = imx_clk_divider("csi_podf",       "csi_sel",           base + 0x3c, 11,   3);
	clks[IMX6SX_CLK_CKO1_PODF]          = imx_clk_divider("cko1_podf",      "cko1_sel",          base + 0x60, 4,    3);
	clks[IMX6SX_CLK_CKO2_PODF]          = imx_clk_divider("cko2_podf",      "cko2_sel",          base + 0x60, 21,   3);

	clks[IMX6SX_CLK_LDB_DI0_DIV_3_5]    = imx_clk_fixed_factor("ldb_di0_div_3_5", "ldb_di0_sel", 2, 7);
	clks[IMX6SX_CLK_LDB_DI0_DIV_7]      = imx_clk_fixed_factor("ldb_di0_div_7",   "ldb_di0_sel", 1, 7);
	clks[IMX6SX_CLK_LDB_DI1_DIV_3_5]    = imx_clk_fixed_factor("ldb_di1_div_3_5", "ldb_di1_sel", 2, 7);
	clks[IMX6SX_CLK_LDB_DI1_DIV_7]      = imx_clk_fixed_factor("ldb_di1_div_7",   "ldb_di1_sel", 1, 7);

	/*                                               name        reg          shift width busy: reg,   shift parent_names       num_parents */
	clks[IMX6SX_CLK_PERIPH]       = imx_clk_busy_mux("periph",   base + 0x14, 25,   1,    base + 0x48, 5,    periph_sels,       ARRAY_SIZE(periph_sels));
	clks[IMX6SX_CLK_PERIPH2]      = imx_clk_busy_mux("periph2",  base + 0x14, 26,   1,    base + 0x48, 3,    periph2_sels,      ARRAY_SIZE(periph2_sels));
	/*                                                   name             parent_name    reg          shift width busy: reg,   shift */
	clks[IMX6SX_CLK_OCRAM_PODF]   = imx_clk_busy_divider("ocram_podf",    "ocram_sel",   base + 0x14, 16,   3,    base + 0x48, 0);
	clks[IMX6SX_CLK_AHB]          = imx_clk_busy_divider("ahb",           "periph",      base + 0x14, 10,   3,    base + 0x48, 1);
	clks[IMX6SX_CLK_MMDC_PODF]    = imx_clk_busy_divider("mmdc_podf",     "periph2",     base + 0x14, 3,    3,    base + 0x48, 2);
	clks[IMX6SX_CLK_ARM]          = imx_clk_busy_divider("arm",           "pll1_sw",     base + 0x10, 0,    3,    base + 0x48, 16);

	/*                                            name             parent_name          reg         shift */
	/* CCGR0 */
	clks[IMX6SX_CLK_AIPS_TZ1]     = imx_clk_gate2("aips_tz1",      "ahb",               base + 0x68, 0);
	clks[IMX6SX_CLK_AIPS_TZ2]     = imx_clk_gate2("aips_tz2",      "ahb",               base + 0x68, 2);
	clks[IMX6SX_CLK_APBH_DMA]     = imx_clk_gate2("apbh_dma",      "usdhc3",            base + 0x68, 4);
	clks[IMX6SX_CLK_ASRC_MEM]     = imx_clk_gate2_shared("asrc_mem", "ahb",             base + 0x68, 6, &share_count_asrc);
	clks[IMX6SX_CLK_ASRC_IPG]     = imx_clk_gate2_shared("asrc_ipg", "ahb",             base + 0x68, 6, &share_count_asrc);
	clks[IMX6SX_CLK_CAAM_MEM]     = imx_clk_gate2("caam_mem",      "ahb",               base + 0x68, 8);
	clks[IMX6SX_CLK_CAAM_ACLK]    = imx_clk_gate2("caam_aclk",     "ahb",               base + 0x68, 10);
	clks[IMX6SX_CLK_CAAM_IPG]     = imx_clk_gate2("caam_ipg",      "ipg",               base + 0x68, 12);
	clks[IMX6SX_CLK_CAN1_IPG]     = imx_clk_gate2("can1_ipg",      "ipg",               base + 0x68, 14);
	clks[IMX6SX_CLK_CAN1_SERIAL]  = imx_clk_gate2("can1_serial",   "can_podf",          base + 0x68, 16);
	clks[IMX6SX_CLK_CAN2_IPG]     = imx_clk_gate2("can2_ipg",      "ipg",               base + 0x68, 18);
	clks[IMX6SX_CLK_CAN2_SERIAL]  = imx_clk_gate2("can2_serial",   "can_podf",          base + 0x68, 20);
	clks[IMX6SX_CLK_DCIC1]        = imx_clk_gate2("dcic1",         "display_podf",      base + 0x68, 24);
	clks[IMX6SX_CLK_DCIC2]        = imx_clk_gate2("dcic2",         "display_podf",      base + 0x68, 26);
	clks[IMX6SX_CLK_AIPS_TZ3]     = imx_clk_gate2("aips_tz3",      "ahb",               base + 0x68, 30);

	/* CCGR1 */
	clks[IMX6SX_CLK_ECSPI1]       = imx_clk_gate2("ecspi1",        "ecspi_podf",        base + 0x6c, 0);
	clks[IMX6SX_CLK_ECSPI2]       = imx_clk_gate2("ecspi2",        "ecspi_podf",        base + 0x6c, 2);
	clks[IMX6SX_CLK_ECSPI3]       = imx_clk_gate2("ecspi3",        "ecspi_podf",        base + 0x6c, 4);
	clks[IMX6SX_CLK_ECSPI4]       = imx_clk_gate2("ecspi4",        "ecspi_podf",        base + 0x6c, 6);
	clks[IMX6SX_CLK_ECSPI5]       = imx_clk_gate2("ecspi5",        "ecspi_podf",        base + 0x6c, 8);
	clks[IMX6SX_CLK_EPIT1]        = imx_clk_gate2("epit1",         "perclk",            base + 0x6c, 12);
	clks[IMX6SX_CLK_EPIT2]        = imx_clk_gate2("epit2",         "perclk",            base + 0x6c, 14);
	clks[IMX6SX_CLK_ESAI_EXTAL]   = imx_clk_gate2_shared("esai_extal", "esai_podf",     base + 0x6c, 16, &share_count_esai);
	clks[IMX6SX_CLK_ESAI_IPG]     = imx_clk_gate2_shared("esai_ipg",   "ahb",           base + 0x6c, 16, &share_count_esai);
	clks[IMX6SX_CLK_ESAI_MEM]     = imx_clk_gate2_shared("esai_mem",   "ahb",           base + 0x6c, 16, &share_count_esai);
	clks[IMX6SX_CLK_WAKEUP]       = imx_clk_gate2("wakeup",        "ipg",               base + 0x6c, 18);
	clks[IMX6SX_CLK_GPT_BUS]      = imx_clk_gate2("gpt_bus",       "perclk",            base + 0x6c, 20);
	clks[IMX6SX_CLK_GPT_SERIAL]   = imx_clk_gate2("gpt_serial",    "perclk",            base + 0x6c, 22);
	clks[IMX6SX_CLK_GPU]          = imx_clk_gate2("gpu",           "gpu_core_podf",     base + 0x6c, 26);
	clks[IMX6SX_CLK_CANFD]        = imx_clk_gate2("canfd",         "can_podf",          base + 0x6c, 30);

	/* CCGR2 */
	clks[IMX6SX_CLK_CSI]          = imx_clk_gate2("csi",           "csi_podf",          base + 0x70, 2);
	clks[IMX6SX_CLK_I2C1]         = imx_clk_gate2("i2c1",          "perclk",            base + 0x70, 6);
	clks[IMX6SX_CLK_I2C2]         = imx_clk_gate2("i2c2",          "perclk",            base + 0x70, 8);
	clks[IMX6SX_CLK_I2C3]         = imx_clk_gate2("i2c3",          "perclk",            base + 0x70, 10);
	clks[IMX6SX_CLK_OCOTP]        = imx_clk_gate2("ocotp",         "ipg",               base + 0x70, 12);
	clks[IMX6SX_CLK_IOMUXC]       = imx_clk_gate2("iomuxc",        "lcdif1_podf",       base + 0x70, 14);
	clks[IMX6SX_CLK_IPMUX1]       = imx_clk_gate2("ipmux1",        "ahb",               base + 0x70, 16);
	clks[IMX6SX_CLK_IPMUX2]       = imx_clk_gate2("ipmux2",        "ahb",               base + 0x70, 18);
	clks[IMX6SX_CLK_IPMUX3]       = imx_clk_gate2("ipmux3",        "ahb",               base + 0x70, 20);
	clks[IMX6SX_CLK_TZASC1]       = imx_clk_gate2("tzasc1",        "mmdc_podf",         base + 0x70, 22);
	clks[IMX6SX_CLK_LCDIF_APB]    = imx_clk_gate2("lcdif_apb",     "display_podf",      base + 0x70, 28);
	clks[IMX6SX_CLK_PXP_AXI]      = imx_clk_gate2("pxp_axi",       "display_podf",      base + 0x70, 30);

	/* CCGR3 */
	clks[IMX6SX_CLK_M4]           = imx_clk_gate2("m4",            "m4_podf",           base + 0x74, 2);
	clks[IMX6SX_CLK_ENET]         = imx_clk_gate2("enet",          "ipg",               base + 0x74, 4);
	clks[IMX6SX_CLK_ENET_AHB]     = imx_clk_gate2("enet_ahb",      "enet_sel",          base + 0x74, 4);
	clks[IMX6SX_CLK_DISPLAY_AXI]  = imx_clk_gate2("display_axi",   "display_podf",      base + 0x74, 6);
	clks[IMX6SX_CLK_LCDIF2_PIX]   = imx_clk_gate2("lcdif2_pix",    "lcdif2_sel",        base + 0x74, 8);
	clks[IMX6SX_CLK_LCDIF1_PIX]   = imx_clk_gate2("lcdif1_pix",    "lcdif1_sel",        base + 0x74, 10);
	clks[IMX6SX_CLK_LDB_DI0]      = imx_clk_gate2("ldb_di0",       "ldb_di0_div_sel",   base + 0x74, 12);
	clks[IMX6SX_CLK_QSPI1]        = imx_clk_gate2("qspi1",         "qspi1_podf",        base + 0x74, 14);
	clks[IMX6SX_CLK_MLB]          = imx_clk_gate2("mlb",           "ahb",               base + 0x74, 18);
	clks[IMX6SX_CLK_MMDC_P0_FAST] = imx_clk_gate2("mmdc_p0_fast",  "mmdc_podf",         base + 0x74, 20);
	clks[IMX6SX_CLK_MMDC_P0_IPG]  = imx_clk_gate2("mmdc_p0_ipg",   "ipg",               base + 0x74, 24);
	clks[IMX6SX_CLK_OCRAM]        = imx_clk_gate2("ocram",         "ocram_podf",        base + 0x74, 28);

	/* CCGR4 */
	clks[IMX6SX_CLK_PCIE_AXI]     = imx_clk_gate2("pcie_axi",      "display_podf",      base + 0x78, 0);
	clks[IMX6SX_CLK_QSPI2]        = imx_clk_gate2("qspi2",         "qspi2_podf",        base + 0x78, 10);
	clks[IMX6SX_CLK_PER1_BCH]     = imx_clk_gate2("per1_bch",      "usdhc3",            base + 0x78, 12);
	clks[IMX6SX_CLK_PER2_MAIN]    = imx_clk_gate2("per2_main",     "ahb",               base + 0x78, 14);
	clks[IMX6SX_CLK_PWM1]         = imx_clk_gate2("pwm1",          "perclk",            base + 0x78, 16);
	clks[IMX6SX_CLK_PWM2]         = imx_clk_gate2("pwm2",          "perclk",            base + 0x78, 18);
	clks[IMX6SX_CLK_PWM3]         = imx_clk_gate2("pwm3",          "perclk",            base + 0x78, 20);
	clks[IMX6SX_CLK_PWM4]         = imx_clk_gate2("pwm4",          "perclk",            base + 0x78, 22);
	clks[IMX6SX_CLK_GPMI_BCH_APB] = imx_clk_gate2("gpmi_bch_apb",  "usdhc3",            base + 0x78, 24);
	clks[IMX6SX_CLK_GPMI_BCH]     = imx_clk_gate2("gpmi_bch",      "usdhc4",            base + 0x78, 26);
	clks[IMX6SX_CLK_GPMI_IO]      = imx_clk_gate2("gpmi_io",       "qspi2_podf",        base + 0x78, 28);
	clks[IMX6SX_CLK_GPMI_APB]     = imx_clk_gate2("gpmi_apb",      "usdhc3",            base + 0x78, 30);

	/* CCGR5 */
	clks[IMX6SX_CLK_ROM]          = imx_clk_gate2("rom",           "ahb",               base + 0x7c, 0);
	clks[IMX6SX_CLK_SDMA]         = imx_clk_gate2("sdma",          "ahb",               base + 0x7c, 6);
	clks[IMX6SX_CLK_SPBA]         = imx_clk_gate2("spba",          "ipg",               base + 0x7c, 12);
	clks[IMX6SX_CLK_AUDIO]        = imx_clk_gate2_shared("audio",  "audio_podf",        base + 0x7c, 14, &share_count_audio);
	clks[IMX6SX_CLK_SPDIF]        = imx_clk_gate2_shared("spdif",  "spdif_podf",        base + 0x7c, 14, &share_count_audio);
	clks[IMX6SX_CLK_SSI1_IPG]     = imx_clk_gate2_shared("ssi1_ipg",      "ipg",        base + 0x7c, 18, &share_count_ssi1);
	clks[IMX6SX_CLK_SSI2_IPG]     = imx_clk_gate2_shared("ssi2_ipg",      "ipg",        base + 0x7c, 20, &share_count_ssi2);
	clks[IMX6SX_CLK_SSI3_IPG]     = imx_clk_gate2_shared("ssi3_ipg",      "ipg",        base + 0x7c, 22, &share_count_ssi3);
	clks[IMX6SX_CLK_SSI1]         = imx_clk_gate2_shared("ssi1",          "ssi1_podf",  base + 0x7c, 18, &share_count_ssi1);
	clks[IMX6SX_CLK_SSI2]         = imx_clk_gate2_shared("ssi2",          "ssi2_podf",  base + 0x7c, 20, &share_count_ssi2);
	clks[IMX6SX_CLK_SSI3]         = imx_clk_gate2_shared("ssi3",          "ssi3_podf",  base + 0x7c, 22, &share_count_ssi3);
	clks[IMX6SX_CLK_UART_IPG]     = imx_clk_gate2("uart_ipg",      "ipg",               base + 0x7c, 24);
	clks[IMX6SX_CLK_UART_SERIAL]  = imx_clk_gate2("uart_serial",   "uart_podf",         base + 0x7c, 26);
	clks[IMX6SX_CLK_SAI1_IPG]     = imx_clk_gate2("sai1_ipg",      "ipg",               base + 0x7c, 28);
	clks[IMX6SX_CLK_SAI2_IPG]     = imx_clk_gate2("sai2_ipg",      "ipg",               base + 0x7c, 30);
	clks[IMX6SX_CLK_SAI1]         = imx_clk_gate2("sai1",          "ssi1_podf",         base + 0x7c, 28);
	clks[IMX6SX_CLK_SAI2]         = imx_clk_gate2("sai2",          "ssi2_podf",         base + 0x7c, 30);

	/* CCGR6 */
	clks[IMX6SX_CLK_USBOH3]       = imx_clk_gate2("usboh3",        "ipg",               base + 0x80, 0);
	clks[IMX6SX_CLK_USDHC1]       = imx_clk_gate2("usdhc1",        "usdhc1_podf",       base + 0x80, 2);
	clks[IMX6SX_CLK_USDHC2]       = imx_clk_gate2("usdhc2",        "usdhc2_podf",       base + 0x80, 4);
	clks[IMX6SX_CLK_USDHC3]       = imx_clk_gate2("usdhc3",        "usdhc3_podf",       base + 0x80, 6);
	clks[IMX6SX_CLK_USDHC4]       = imx_clk_gate2("usdhc4",        "usdhc4_podf",       base + 0x80, 8);
	clks[IMX6SX_CLK_EIM_SLOW]     = imx_clk_gate2("eim_slow",      "eim_slow_podf",     base + 0x80, 10);
	clks[IMX6SX_CLK_PWM8]         = imx_clk_gate2("pwm8",          "perclk",            base + 0x80, 16);
	clks[IMX6SX_CLK_VADC]         = imx_clk_gate2("vadc",          "vid_podf",          base + 0x80, 20);
	clks[IMX6SX_CLK_GIS]          = imx_clk_gate2("gis",           "display_podf",      base + 0x80, 22);
	clks[IMX6SX_CLK_I2C4]         = imx_clk_gate2("i2c4",          "perclk",            base + 0x80, 24);
	clks[IMX6SX_CLK_PWM5]         = imx_clk_gate2("pwm5",          "perclk",            base + 0x80, 26);
	clks[IMX6SX_CLK_PWM6]         = imx_clk_gate2("pwm6",          "perclk",            base + 0x80, 28);
	clks[IMX6SX_CLK_PWM7]         = imx_clk_gate2("pwm7",          "perclk",            base + 0x80, 30);

	clks[IMX6SX_CLK_CKO1]         = imx_clk_gate("cko1",           "cko1_podf",         base + 0x60, 7);
	clks[IMX6SX_CLK_CKO2]         = imx_clk_gate("cko2",           "cko2_podf",         base + 0x60, 24);

	/* mask handshake of mmdc */
	writel_relaxed(BM_CCM_CCDR_MMDC_CH0_MASK, base + CCDR);

	imx_check_clocks(clks, ARRAY_SIZE(clks));

	clk_data.clks = clks;
	clk_data.clk_num = ARRAY_SIZE(clks);
	of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);

	for (i = 0; i < ARRAY_SIZE(clks_init_on); i++)
		clk_prepare_enable(clks[clks_init_on[i]]);

	if (IS_ENABLED(CONFIG_USB_MXS_PHY)) {
		clk_prepare_enable(clks[IMX6SX_CLK_USBPHY1_GATE]);
		clk_prepare_enable(clks[IMX6SX_CLK_USBPHY2_GATE]);
	}

	/* Set the default 132MHz for EIM module */
	clk_set_parent(clks[IMX6SX_CLK_EIM_SLOW_SEL], clks[IMX6SX_CLK_PLL2_PFD2]);
	clk_set_rate(clks[IMX6SX_CLK_EIM_SLOW], 132000000);

	/* set parent clock for LCDIF1 pixel clock */
	clk_set_parent(clks[IMX6SX_CLK_LCDIF1_PRE_SEL], clks[IMX6SX_CLK_PLL5_VIDEO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_LCDIF1_SEL], clks[IMX6SX_CLK_LCDIF1_PODF]);

	/* Set the parent clks of PCIe lvds1 and pcie_axi to be pcie ref, axi */
	if (clk_set_parent(clks[IMX6SX_CLK_LVDS1_SEL], clks[IMX6SX_CLK_PCIE_REF_125M]))
		pr_err("Failed to set pcie bus parent clk.\n");
	if (clk_set_parent(clks[IMX6SX_CLK_PCIE_AXI_SEL], clks[IMX6SX_CLK_AXI]))
		pr_err("Failed to set pcie parent clk.\n");

	/*
	 * Init enet system AHB clock, set to 200Mhz
	 * pll2_pfd2_396m-> ENET_PODF-> ENET_AHB
	 */
	clk_set_parent(clks[IMX6SX_CLK_ENET_PRE_SEL], clks[IMX6SX_CLK_PLL2_PFD2]);
	clk_set_parent(clks[IMX6SX_CLK_ENET_SEL], clks[IMX6SX_CLK_ENET_PODF]);
	clk_set_rate(clks[IMX6SX_CLK_ENET_PODF], 200000000);
	clk_set_rate(clks[IMX6SX_CLK_ENET_REF], 125000000);
	clk_set_rate(clks[IMX6SX_CLK_ENET2_REF], 125000000);

	/* Audio clocks */
	clk_set_rate(clks[IMX6SX_CLK_PLL4_AUDIO_DIV], 393216000);

	clk_set_parent(clks[IMX6SX_CLK_SPDIF_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_SPDIF_PODF], 98304000);

	clk_set_parent(clks[IMX6SX_CLK_AUDIO_SEL], clks[IMX6SX_CLK_PLL3_USB_OTG]);
	clk_set_rate(clks[IMX6SX_CLK_AUDIO_PODF], 24000000);

	clk_set_parent(clks[IMX6SX_CLK_SSI1_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_SSI2_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_SSI3_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_SSI1_PODF], 24576000);
	clk_set_rate(clks[IMX6SX_CLK_SSI2_PODF], 24576000);
	clk_set_rate(clks[IMX6SX_CLK_SSI3_PODF], 24576000);

	clk_set_parent(clks[IMX6SX_CLK_ESAI_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_ESAI_PODF], 24576000);

	/* Set parent clock for vadc */
	clk_set_parent(clks[IMX6SX_CLK_VID_SEL], clks[IMX6SX_CLK_PLL3_USB_OTG]);

	/* default parent of can_sel clock is invalid, manually set it here */
	clk_set_parent(clks[IMX6SX_CLK_CAN_SEL], clks[IMX6SX_CLK_PLL3_60M]);

	/* Update gpu clock from default 528M to 720M */
	clk_set_parent(clks[IMX6SX_CLK_GPU_CORE_SEL], clks[IMX6SX_CLK_PLL3_PFD0]);
	clk_set_parent(clks[IMX6SX_CLK_GPU_AXI_SEL], clks[IMX6SX_CLK_PLL3_PFD0]);

	clk_set_parent(clks[IMX6SX_CLK_QSPI1_SEL], clks[IMX6SX_CLK_PLL2_BUS]);
	clk_set_parent(clks[IMX6SX_CLK_QSPI2_SEL], clks[IMX6SX_CLK_PLL2_BUS]);

	/* Set initial power mode */
	imx6q_set_lpm(WAIT_CLOCKED);
}
CLK_OF_DECLARE(imx6sx, "fsl,imx6sx-ccm", imx6sx_clocks_init);
