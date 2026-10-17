// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022
 * Author(s): Jesse Taube <Mr.Bossman075@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imxrt1170-clock.h>

#include "clk.h"

static ulong imxrt1170_clk_get_rate(struct clk *clk)
{
	struct clk *c;
	int ret;

	debug("%s(#%lu)\n", __func__, clk->id);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_get_rate(c);
}

static ulong imxrt1170_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk *c;
	int ret;

	debug("%s(#%lu), rate: %lu\n", __func__, clk->id, rate);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_set_rate(c, rate);
}

static int __imxrt1170_clk_enable(struct clk *clk, bool enable)
{
	struct clk *c;
	int ret;

	debug("%s(#%lu) en: %d\n", __func__, clk->id, enable);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	if (enable)
		ret = clk_enable(c);
	else
		ret = clk_disable(c);

	return ret;
}

static int imxrt1170_clk_disable(struct clk *clk)
{
	return __imxrt1170_clk_enable(clk, 0);
}

static int imxrt1170_clk_enable(struct clk *clk)
{
	return __imxrt1170_clk_enable(clk, 1);
}

static int imxrt1170_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *c, *cp;
	int ret;

	debug("%s(#%lu), parent: %lu\n", __func__, clk->id, parent->id);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	ret = clk_get_by_id(parent->id, &cp);
	if (ret)
		return ret;

	return clk_set_parent(c, cp);
}

static struct clk_ops imxrt1170_clk_ops = {
	.set_rate = imxrt1170_clk_set_rate,
	.get_rate = imxrt1170_clk_get_rate,
	.enable = imxrt1170_clk_enable,
	.disable = imxrt1170_clk_disable,
	.set_parent = imxrt1170_clk_set_parent,
};

static const char * const lpuart1_sels[] = {"rcosc48M_div2", "osc", "rcosc400M", "rcosc16M",
"pll3_div2", "pll1_div5", "pll2_sys", "pll2_pfd3"};
static const char * const gpt1_sels[] = {"rcosc48M_div2", "osc", "rcosc400M", "rcosc16M",
"pll3_div2", "pll1_div5", "pll3_pfd2", "pll3_pfd3"};
static const char * const usdhc1_sels[] = {"rcosc48M_div2", "osc", "rcosc400M", "rcosc16M",
"pll2_pfd2", "pll2_pfd0", "pll1_div5", "pll_arm"};
static const char * const semc_sels[] = {"rcosc48M_div2", "osc", "rcosc400M", "rcosc16M",
"pll1_div5", "pll2_sys", "pll2_pfd2", "pll3_pfd0"};

static int imxrt1170_clk_probe(struct udevice *dev)
{
	void *base;

	/* Anatop clocks */
	base = (void *)ofnode_get_addr(ofnode_by_compatible(ofnode_null(), "fsl,imxrt-anatop"));



	clk_dm(IMXRT1170_CLK_RCOSC_48M,
	       imx_clk_fixed_factor("rcosc48M", "rcosc16M", 3, 1));
	clk_dm(IMXRT1170_CLK_RCOSC_400M,
	       imx_clk_fixed_factor("rcosc400M",  "rcosc16M", 25, 1));
	clk_dm(IMXRT1170_CLK_RCOSC_48M_DIV2,
	       imx_clk_fixed_factor("rcosc48M_div2",  "rcosc48M", 1, 2));


	clk_dm(IMXRT1170_CLK_PLL_ARM,
	       imx_clk_pllv3(IMX_PLLV3_SYS, "pll_arm", "osc",
			     base + 0x200, 0xff));
	clk_dm(IMXRT1170_CLK_PLL3,
	       imx_clk_pllv3(IMX_PLLV3_GENERICV2, "pll3_sys", "osc",
			     base + 0x210, 1));
	clk_dm(IMXRT1170_CLK_PLL2,
	       imx_clk_pllv3(IMX_PLLV3_GENERICV2, "pll2_sys", "osc",
			     base + 0x240, 1));

	clk_dm(IMXRT1170_CLK_PLL3_PFD0,
	       imx_clk_pfd("pll3_pfd0", "pll3_sys", base + 0x230, 0));
	clk_dm(IMXRT1170_CLK_PLL3_PFD1,
	       imx_clk_pfd("pll3_pfd1", "pll3_sys", base + 0x230, 1));
	clk_dm(IMXRT1170_CLK_PLL3_PFD2,
	       imx_clk_pfd("pll3_pfd2", "pll3_sys", base + 0x230, 2));
	clk_dm(IMXRT1170_CLK_PLL3_PFD3,
	       imx_clk_pfd("pll3_pfd3", "pll3_sys", base + 0x230, 3));

	clk_dm(IMXRT1170_CLK_PLL2_PFD0,
	       imx_clk_pfd("pll2_pfd0", "pll2_sys", base + 0x270, 0));
	clk_dm(IMXRT1170_CLK_PLL2_PFD1,
	       imx_clk_pfd("pll2_pfd1", "pll2_sys", base + 0x270, 1));
	clk_dm(IMXRT1170_CLK_PLL2_PFD2,
	       imx_clk_pfd("pll2_pfd2", "pll2_sys", base + 0x270, 2));
	clk_dm(IMXRT1170_CLK_PLL2_PFD3,
	       imx_clk_pfd("pll2_pfd3", "pll2_sys", base + 0x270, 3));

	clk_dm(IMXRT1170_CLK_PLL3_DIV2,
	       imx_clk_fixed_factor("pll3_div2", "pll3_sys", 1, 2));

	/* CCM clocks */
	base = dev_read_addr_ptr(dev);
	if (base == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	clk_dm(IMXRT1170_CLK_LPUART1_SEL,
	       imx_clk_mux("lpuart1_sel", base + (25 * 0x80), 8, 3,
			   lpuart1_sels, ARRAY_SIZE(lpuart1_sels)));
	clk_dm(IMXRT1170_CLK_LPUART1,
	       imx_clk_divider("lpuart1", "lpuart1_sel",
			       base + (25 * 0x80), 0, 8));

	clk_dm(IMXRT1170_CLK_USDHC1_SEL,
	       imx_clk_mux("usdhc1_sel", base + (58 * 0x80), 8, 3,
			   usdhc1_sels, ARRAY_SIZE(usdhc1_sels)));
	clk_dm(IMXRT1170_CLK_USDHC1,
	       imx_clk_divider("usdhc1", "usdhc1_sel",
			       base + (58 * 0x80), 0, 8));

	clk_dm(IMXRT1170_CLK_GPT1_SEL,
	       imx_clk_mux("gpt1_sel", base + (14 * 0x80), 8, 3,
			   gpt1_sels, ARRAY_SIZE(gpt1_sels)));
	clk_dm(IMXRT1170_CLK_GPT1,
	       imx_clk_divider("gpt1", "gpt1_sel",
			       base + (14 * 0x80), 0, 8));

	clk_dm(IMXRT1170_CLK_SEMC_SEL,
	       imx_clk_mux("semc_sel", base + (4 * 0x80), 8, 3,
			   semc_sels, ARRAY_SIZE(semc_sels)));
	clk_dm(IMXRT1170_CLK_SEMC,
	       imx_clk_divider("semc", "semc_sel",
			       base + (4 * 0x80), 0, 8));
	struct clk *clk, *clk1;

	clk_get_by_id(IMXRT1170_CLK_PLL2_PFD2, &clk);

	clk_get_by_id(IMXRT1170_CLK_SEMC_SEL, &clk1);
	clk_enable(clk1);
	clk_set_parent(clk1, clk);

	clk_get_by_id(IMXRT1170_CLK_SEMC, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 132000000UL);

	clk_get_by_id(IMXRT1170_CLK_GPT1, &clk);
	clk_enable(clk);
	clk_set_rate(clk, 32000000UL);

	return 0;
}

static const struct udevice_id imxrt1170_clk_ids[] = {
	{ .compatible = "fsl,imxrt1170-ccm" },
	{ },
};

U_BOOT_DRIVER(imxrt1170_clk) = {
	.name = "clk_imxrt1170",
	.id = UCLASS_CLK,
	.of_match = imxrt1170_clk_ids,
	.ops = &imxrt1170_clk_ops,
	.probe = imxrt1170_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
