/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright(C) 2020
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imxrt1020-clock.h>

#include "clk.h"

static struct clk_ops imxrt1020_clk_ops = {
	.set_rate = ccf_clk_set_rate,
	.get_rate = ccf_clk_get_rate,
	.enable = ccf_clk_enable,
	.disable = ccf_clk_disable,
};

static const char * const pll2_bypass_sels[] = {"pll2_sys", "osc", };
static const char * const pll3_bypass_sels[] = {"pll3_usb_otg", "osc", };

static const char *const pre_periph_sels[] = { "pll2_sys", "pll2_pfd3_297m", "pll3_pfd3_454_74m", "arm_podf", };
static const char *const periph_sels[] = { "pre_periph_sel", "todo", };
static const char *const usdhc_sels[] = { "pll2_pfd2_396m", "pll2_pfd0_352m", };
static const char *const lpuart_sels[] = { "pll3_80m", "osc", };
static const char *const semc_alt_sels[] = { "pll2_pfd2_396m", "pll3_pfd1_664_62m", };
static const char *const semc_sels[] = { "periph_sel", "semc_alt_sel", };

static int imxrt1020_clk_probe(struct udevice *dev)
{
	void *base;

	/* Anatop clocks */
	base = (void *)ofnode_get_addr(ofnode_by_compatible(ofnode_null(), "fsl,imxrt-anatop"));

	clk_dm(IMXRT1020_CLK_PLL2_SYS,
	       imx_clk_pllv3(IMX_PLLV3_GENERIC, "pll2_sys", "osc",
			     base + 0x30, 0x1));
	clk_dm(IMXRT1020_CLK_PLL3_USB_OTG,
	       imx_clk_pllv3(IMX_PLLV3_USB, "pll3_usb_otg", "osc",
			     base + 0x10, 0x1));

	/* PLL bypass out */
	clk_dm(IMXRT1020_CLK_PLL2_BYPASS,
	       imx_clk_mux_flags("pll2_bypass", base + 0x30, 16, 1,
				 pll2_bypass_sels,
				 ARRAY_SIZE(pll2_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMXRT1020_CLK_PLL3_BYPASS,
	       imx_clk_mux_flags("pll3_bypass", base + 0x10, 16, 1,
				 pll3_bypass_sels,
				 ARRAY_SIZE(pll3_bypass_sels),
				 CLK_SET_RATE_PARENT));

	clk_dm(IMXRT1020_CLK_PLL3_80M,
	       imx_clk_fixed_factor("pll3_80m",  "pll3_usb_otg",   1, 6));

	clk_dm(IMXRT1020_CLK_PLL2_PFD0_352M,
	       imx_clk_pfd("pll2_pfd0_352m", "pll2_sys", base + 0x100, 0));
	clk_dm(IMXRT1020_CLK_PLL2_PFD1_594M,
	       imx_clk_pfd("pll2_pfd1_594m", "pll2_sys", base + 0x100, 1));
	clk_dm(IMXRT1020_CLK_PLL2_PFD2_396M,
	       imx_clk_pfd("pll2_pfd2_396m", "pll2_sys", base + 0x100, 2));
	clk_dm(IMXRT1020_CLK_PLL2_PFD3_297M,
	       imx_clk_pfd("pll2_pfd3_297m", "pll2_sys", base + 0x100, 3));
	clk_dm(IMXRT1020_CLK_PLL3_PFD1_664_62M,
	       imx_clk_pfd("pll3_pfd1_664_62m", "pll3_usb_otg", base + 0xf0, 1));
	clk_dm(IMXRT1020_CLK_PLL3_PFD3_454_74M,
	       imx_clk_pfd("pll3_pfd3_454_74m", "pll3_usb_otg", base + 0xf0, 3));

	/* CCM clocks */
	base = dev_read_addr_ptr(dev);
	if (base == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	clk_dm(IMXRT1020_CLK_PRE_PERIPH_SEL,
	       imx_clk_mux("pre_periph_sel", base + 0x18, 18, 2,
			   pre_periph_sels, ARRAY_SIZE(pre_periph_sels)));
	clk_dm(IMXRT1020_CLK_PERIPH_SEL,
	       imx_clk_mux("periph_sel", base + 0x14, 25, 1,
			   periph_sels, ARRAY_SIZE(periph_sels)));
	clk_dm(IMXRT1020_CLK_USDHC1_SEL,
	       imx_clk_mux("usdhc1_sel", base + 0x1c, 16, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMXRT1020_CLK_USDHC2_SEL,
	       imx_clk_mux("usdhc2_sel", base + 0x1c, 17, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMXRT1020_CLK_LPUART_SEL,
	       imx_clk_mux("lpuart_sel", base + 0x24, 6, 1,
			   lpuart_sels, ARRAY_SIZE(lpuart_sels)));
	clk_dm(IMXRT1020_CLK_SEMC_ALT_SEL,
	       imx_clk_mux("semc_alt_sel", base + 0x14, 7, 1,
			   semc_alt_sels, ARRAY_SIZE(semc_alt_sels)));
	clk_dm(IMXRT1020_CLK_SEMC_SEL,
	       imx_clk_mux("semc_sel", base + 0x14, 6, 1,
			   semc_sels, ARRAY_SIZE(semc_sels)));

	clk_dm(IMXRT1020_CLK_AHB_PODF,
	       imx_clk_divider("ahb_podf", "periph_sel",
			       base + 0x14, 10, 3));
	clk_dm(IMXRT1020_CLK_USDHC1_PODF,
	       imx_clk_divider("usdhc1_podf", "usdhc1_sel",
			       base + 0x24, 11, 3));
	clk_dm(IMXRT1020_CLK_USDHC2_PODF,
	       imx_clk_divider("usdhc2_podf", "usdhc2_sel",
			       base + 0x24, 16, 3));
	clk_dm(IMXRT1020_CLK_LPUART_PODF,
	       imx_clk_divider("lpuart_podf", "lpuart_sel",
			       base + 0x24, 0, 6));
	clk_dm(IMXRT1020_CLK_SEMC_PODF,
	       imx_clk_divider("semc_podf", "semc_sel",
			       base + 0x14, 16, 3));

	clk_dm(IMXRT1020_CLK_USDHC1,
	       imx_clk_gate2("usdhc1", "usdhc1_podf", base + 0x80, 2));
	clk_dm(IMXRT1020_CLK_USDHC2,
	       imx_clk_gate2("usdhc2", "usdhc2_podf", base + 0x80, 4));
	clk_dm(IMXRT1020_CLK_LPUART1,
	       imx_clk_gate2("lpuart1", "lpuart_podf", base + 0x7c, 24));
	clk_dm(IMXRT1020_CLK_SEMC,
	       imx_clk_gate2("semc", "semc_podf", base + 0x74, 4));

#ifdef CONFIG_SPL_BUILD
	struct clk *clk, *clk1;

	clk_get_by_id(IMXRT1020_CLK_SEMC_SEL, &clk1);
	clk_get_by_id(IMXRT1020_CLK_SEMC_ALT_SEL, &clk);
	clk_set_parent(clk1, clk);

	/* Configure PLL3_USB_OTG to 480MHz */
	clk_get_by_id(IMXRT1020_CLK_PLL3_USB_OTG, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 480000000UL);

	clk_get_by_id(IMXRT1020_CLK_PLL3_BYPASS, &clk1);
	clk_set_parent(clk1, clk);

	clk_get_by_id(IMXRT1020_CLK_PLL2_PFD3_297M, &clk);
	clk_set_rate(clk, 297000000UL);

	clk_get_by_id(IMXRT1020_CLK_PLL2_SYS, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 528000000UL);

	clk_get_by_id(IMXRT1020_CLK_PLL2_BYPASS, &clk1);
	clk_set_parent(clk1, clk);

#endif

	return 0;
}

static const struct udevice_id imxrt1020_clk_ids[] = {
	{ .compatible = "fsl,imxrt1020-ccm" },
	{ },
};

U_BOOT_DRIVER(imxrt1020_clk) = {
	.name = "clk_imxrt1020",
	.id = UCLASS_CLK,
	.of_match = imxrt1020_clk_ids,
	.ops = &imxrt1020_clk_ops,
	.probe = imxrt1020_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
