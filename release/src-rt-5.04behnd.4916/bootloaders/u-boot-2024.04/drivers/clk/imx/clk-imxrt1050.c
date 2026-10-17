// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright(C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imxrt1050-clock.h>

#include "clk.h"

static const char * const pll_ref_sels[] = {"osc", "dummy", };
static const char * const pll1_bypass_sels[] = {"pll1_arm", "pll1_arm_ref_sel", };
static const char * const pll2_bypass_sels[] = {"pll2_sys", "pll2_sys_ref_sel", };
static const char * const pll3_bypass_sels[] = {"pll3_usb_otg", "pll3_usb_otg_ref_sel", };
static const char * const pll5_bypass_sels[] = {"pll5_video", "pll5_video_ref_sel", };

static const char *const pre_periph_sels[] = { "pll2_sys", "pll2_pfd2_396m", "pll2_pfd0_352m", "arm_podf", };
static const char *const periph_sels[] = { "pre_periph_sel", "todo", };
static const char *const usdhc_sels[] = { "pll2_pfd2_396m", "pll2_pfd0_352m", };
static const char *const lpuart_sels[] = { "pll3_80m", "osc", };
static const char *const semc_alt_sels[] = { "pll2_pfd2_396m", "pll3_pfd1_664_62m", };
static const char *const semc_sels[] = { "periph_sel", "semc_alt_sel", };
static const char *const lcdif_sels[] = { "pll2_sys", "pll3_pfd3_454_74m", "pll5_video", "pll2_pfd0_352m", "pll2_pfd1_594m", "pll3_pfd1_664_62m"};

static int imxrt1050_clk_probe(struct udevice *dev)
{
	void *base;

	/* Anatop clocks */
	base = (void *)ofnode_get_addr(ofnode_by_compatible(ofnode_null(), "fsl,imxrt-anatop"));

	clk_dm(IMXRT1050_CLK_PLL1_REF_SEL,
	       imx_clk_mux("pll1_arm_ref_sel", base + 0x0, 14, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMXRT1050_CLK_PLL2_REF_SEL,
	       imx_clk_mux("pll2_sys_ref_sel", base + 0x30, 14, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMXRT1050_CLK_PLL3_REF_SEL,
	       imx_clk_mux("pll3_usb_otg_ref_sel", base + 0x10, 14, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMXRT1050_CLK_PLL5_REF_SEL,
	       imx_clk_mux("pll5_video_ref_sel", base + 0xa0, 14, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));

	clk_dm(IMXRT1050_CLK_PLL1_ARM,
	       imx_clk_pllv3(IMX_PLLV3_SYS, "pll1_arm", "pll1_arm_ref_sel",
			     base + 0x0, 0x7f));
	clk_dm(IMXRT1050_CLK_PLL2_SYS,
	       imx_clk_pllv3(IMX_PLLV3_GENERIC, "pll2_sys", "pll2_sys_ref_sel",
			     base + 0x30, 0x1));
	clk_dm(IMXRT1050_CLK_PLL3_USB_OTG,
	       imx_clk_pllv3(IMX_PLLV3_USB, "pll3_usb_otg",
			     "pll3_usb_otg_ref_sel",
			     base + 0x10, 0x1));
	clk_dm(IMXRT1050_CLK_PLL5_VIDEO,
	       imx_clk_pllv3(IMX_PLLV3_AV, "pll5_video", "pll5_video_ref_sel",
			     base + 0xa0, 0x7f));

	/* PLL bypass out */
	clk_dm(IMXRT1050_CLK_PLL1_BYPASS,
	       imx_clk_mux_flags("pll1_bypass", base + 0x0, 16, 1,
				 pll1_bypass_sels,
				 ARRAY_SIZE(pll1_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMXRT1050_CLK_PLL2_BYPASS,
	       imx_clk_mux_flags("pll2_bypass", base + 0x30, 16, 1,
				 pll2_bypass_sels,
				 ARRAY_SIZE(pll2_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMXRT1050_CLK_PLL3_BYPASS,
	       imx_clk_mux_flags("pll3_bypass", base + 0x10, 16, 1,
				 pll3_bypass_sels,
				 ARRAY_SIZE(pll3_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMXRT1050_CLK_PLL5_BYPASS,
	       imx_clk_mux_flags("pll5_bypass", base + 0xa0, 16, 1,
				 pll5_bypass_sels,
				 ARRAY_SIZE(pll5_bypass_sels),
				 CLK_SET_RATE_PARENT));

	clk_dm(IMXRT1050_CLK_VIDEO_POST_DIV_SEL,
	       imx_clk_divider("video_post_div_sel", "pll5_video",
			       base + 0xa0, 19, 2));
	clk_dm(IMXRT1050_CLK_VIDEO_DIV,
	       imx_clk_divider("video_div", "video_post_div_sel",
			       base + 0x170, 30, 2));

	clk_dm(IMXRT1050_CLK_PLL3_80M,
	       imx_clk_fixed_factor("pll3_80m",  "pll3_usb_otg",   1, 6));

	clk_dm(IMXRT1050_CLK_PLL2_PFD0_352M,
	       imx_clk_pfd("pll2_pfd0_352m", "pll2_sys", base + 0x100, 0));
	clk_dm(IMXRT1050_CLK_PLL2_PFD1_594M,
	       imx_clk_pfd("pll2_pfd1_594m", "pll2_sys", base + 0x100, 1));
	clk_dm(IMXRT1050_CLK_PLL2_PFD2_396M,
	       imx_clk_pfd("pll2_pfd2_396m", "pll2_sys", base + 0x100, 2));
	clk_dm(IMXRT1050_CLK_PLL3_PFD1_664_62M,
	       imx_clk_pfd("pll3_pfd1_664_62m", "pll3_usb_otg", base + 0xf0,
			   1));
	clk_dm(IMXRT1050_CLK_PLL3_PFD3_454_74M,
	       imx_clk_pfd("pll3_pfd3_454_74m", "pll3_usb_otg", base + 0xf0,
			   3));

	/* CCM clocks */
	base = dev_read_addr_ptr(dev);
	if (base == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	clk_dm(IMXRT1050_CLK_ARM_PODF,
	       imx_clk_divider("arm_podf", "pll1_arm",
			       base + 0x10, 0, 3));

	clk_dm(IMXRT1050_CLK_PRE_PERIPH_SEL,
	       imx_clk_mux("pre_periph_sel", base + 0x18, 18, 2,
			   pre_periph_sels, ARRAY_SIZE(pre_periph_sels)));
	clk_dm(IMXRT1050_CLK_PERIPH_SEL,
	       imx_clk_mux("periph_sel", base + 0x14, 25, 1,
			   periph_sels, ARRAY_SIZE(periph_sels)));
	clk_dm(IMXRT1050_CLK_USDHC1_SEL,
	       imx_clk_mux("usdhc1_sel", base + 0x1c, 16, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMXRT1050_CLK_USDHC2_SEL,
	       imx_clk_mux("usdhc2_sel", base + 0x1c, 17, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMXRT1050_CLK_LPUART_SEL,
	       imx_clk_mux("lpuart_sel", base + 0x24, 6, 1,
			   lpuart_sels, ARRAY_SIZE(lpuart_sels)));
	clk_dm(IMXRT1050_CLK_SEMC_ALT_SEL,
	       imx_clk_mux("semc_alt_sel", base + 0x14, 7, 1,
			   semc_alt_sels, ARRAY_SIZE(semc_alt_sels)));
	clk_dm(IMXRT1050_CLK_SEMC_SEL,
	       imx_clk_mux("semc_sel", base + 0x14, 6, 1,
			   semc_sels, ARRAY_SIZE(semc_sels)));
	clk_dm(IMXRT1050_CLK_LCDIF_SEL,
	       imx_clk_mux("lcdif_sel", base + 0x38, 15, 3,
			   lcdif_sels, ARRAY_SIZE(lcdif_sels)));

	clk_dm(IMXRT1050_CLK_AHB_PODF,
	       imx_clk_divider("ahb_podf", "periph_sel",
			       base + 0x14, 10, 3));
	clk_dm(IMXRT1050_CLK_USDHC1_PODF,
	       imx_clk_divider("usdhc1_podf", "usdhc1_sel",
			       base + 0x24, 11, 3));
	clk_dm(IMXRT1050_CLK_USDHC2_PODF,
	       imx_clk_divider("usdhc2_podf", "usdhc2_sel",
			       base + 0x24, 16, 3));
	clk_dm(IMXRT1050_CLK_LPUART_PODF,
	       imx_clk_divider("lpuart_podf", "lpuart_sel",
			       base + 0x24, 0, 6));
	clk_dm(IMXRT1050_CLK_SEMC_PODF,
	       imx_clk_divider("semc_podf", "semc_sel",
			       base + 0x14, 16, 3));
	clk_dm(IMXRT1050_CLK_LCDIF_PRED,
	       imx_clk_divider("lcdif_pred", "lcdif_sel",
			       base + 0x38, 12, 3));
	clk_dm(IMXRT1050_CLK_LCDIF_PODF,
	       imx_clk_divider("lcdif_podf", "lcdif_pred",
			       base + 0x18, 23, 3));

	clk_dm(IMXRT1050_CLK_USDHC1,
	       imx_clk_gate2("usdhc1", "usdhc1_podf", base + 0x80, 2));
	clk_dm(IMXRT1050_CLK_USDHC2,
	       imx_clk_gate2("usdhc2", "usdhc2_podf", base + 0x80, 4));
	clk_dm(IMXRT1050_CLK_LPUART1,
	       imx_clk_gate2("lpuart1", "lpuart_podf", base + 0x7c, 24));
	clk_dm(IMXRT1050_CLK_SEMC,
	       imx_clk_gate2("semc", "semc_podf", base + 0x74, 4));
	clk_dm(IMXRT1050_CLK_LCDIF_APB,
	       imx_clk_gate2("lcdif", "lcdif_podf", base + 0x70, 28));
	clk_dm(IMXRT1050_CLK_LCDIF_PIX,
	       imx_clk_gate2("lcdif_pix", "lcdif", base + 0x74, 10));
	clk_dm(IMXRT1050_CLK_USBOH3,
	       imx_clk_gate2("usboh3", "pll3_usb_otg", base + 0x80, 0));

	struct clk *clk, *clk1;

#ifdef CONFIG_SPL_BUILD
	/* bypass pll1 before setting its rate */
	clk_get_by_id(IMXRT1050_CLK_PLL1_REF_SEL, &clk);
	clk_get_by_id(IMXRT1050_CLK_PLL1_BYPASS, &clk1);
	clk_set_parent(clk1, clk);

	clk_get_by_id(IMXRT1050_CLK_PLL1_ARM, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 1056000000UL);

	clk_get_by_id(IMXRT1050_CLK_PLL1_BYPASS, &clk1);
	clk_set_parent(clk1, clk);

	clk_get_by_id(IMXRT1050_CLK_SEMC_SEL, &clk1);
	clk_get_by_id(IMXRT1050_CLK_SEMC_ALT_SEL, &clk);
	clk_set_parent(clk1, clk);

	clk_get_by_id(IMXRT1050_CLK_PLL2_SYS, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 528000000UL);

	clk_get_by_id(IMXRT1050_CLK_PLL2_BYPASS, &clk1);
	clk_set_parent(clk1, clk);

	/* Configure PLL3_USB_OTG to 480MHz */
	clk_get_by_id(IMXRT1050_CLK_PLL3_USB_OTG, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 480000000UL);

	clk_get_by_id(IMXRT1050_CLK_PLL3_BYPASS, &clk1);
	clk_set_parent(clk1, clk);
#else
	/* Set PLL5 for LCDIF to its default 650Mhz */
	clk_get_by_id(IMXRT1050_CLK_PLL5_VIDEO, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 650000000UL);

	clk_get_by_id(IMXRT1050_CLK_PLL5_BYPASS, &clk1);
	clk_set_parent(clk1, clk);
#endif

	return 0;
}

static const struct udevice_id imxrt1050_clk_ids[] = {
	{ .compatible = "fsl,imxrt1050-ccm" },
	{ },
};

U_BOOT_DRIVER(imxrt1050_clk) = {
	.name = "clk_imxrt1050",
	.id = UCLASS_CLK,
	.of_match = imxrt1050_clk_ids,
	.ops = &ccf_clk_ops,
	.probe = imxrt1050_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
