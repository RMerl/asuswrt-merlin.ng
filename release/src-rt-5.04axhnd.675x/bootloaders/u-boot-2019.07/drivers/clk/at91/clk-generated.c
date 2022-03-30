// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

#define GENERATED_SOURCE_MAX	6
#define GENERATED_MAX_DIV	255

/**
 * generated_clk_bind() - for the generated clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int generated_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "generic-clk");
}

static const struct udevice_id generated_clk_match[] = {
	{ .compatible = "atmel,sama5d2-clk-generated" },
	{}
};

U_BOOT_DRIVER(generated_clk) = {
	.name = "generated-clk",
	.id = UCLASS_MISC,
	.of_match = generated_clk_match,
	.bind = generated_clk_bind,
};

/*-------------------------------------------------------------*/

struct generic_clk_priv {
	u32 num_parents;
};

static ulong generic_clk_get_rate(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk parent;
	ulong clk_rate;
	u32 tmp, gckdiv;
	u8 clock_source, parent_index;
	int ret;

	writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
	tmp = readl(&pmc->pcr);
	clock_source = (tmp >> AT91_PMC_PCR_GCKCSS_OFFSET) &
		    AT91_PMC_PCR_GCKCSS_MASK;
	gckdiv = (tmp >> AT91_PMC_PCR_GCKDIV_OFFSET) & AT91_PMC_PCR_GCKDIV_MASK;

	parent_index = clock_source - 1;
	ret = clk_get_by_index(dev_get_parent(clk->dev), parent_index, &parent);
	if (ret)
		return 0;

	clk_rate = clk_get_rate(&parent) / (gckdiv + 1);

	clk_free(&parent);

	return clk_rate;
}

static ulong generic_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct generic_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk parent, best_parent;
	ulong tmp_rate, best_rate = rate, parent_rate;
	int tmp_diff, best_diff = -1;
	u32 div, best_div = 0;
	u8 best_parent_index, best_clock_source = 0;
	u8 i;
	u32 tmp;
	int ret;

	for (i = 0; i < priv->num_parents; i++) {
		ret = clk_get_by_index(dev_get_parent(clk->dev), i, &parent);
		if (ret)
			return ret;

		parent_rate = clk_get_rate(&parent);
		if (IS_ERR_VALUE(parent_rate))
			return parent_rate;

		for (div = 1; div < GENERATED_MAX_DIV + 2; div++) {
			tmp_rate = DIV_ROUND_CLOSEST(parent_rate, div);
			tmp_diff = abs(rate - tmp_rate);

			if (best_diff < 0 || best_diff > tmp_diff) {
				best_rate = tmp_rate;
				best_diff = tmp_diff;

				best_div = div - 1;
				best_parent = parent;
				best_parent_index = i;
				best_clock_source = best_parent_index + 1;
			}

			if (!best_diff || tmp_rate < rate)
				break;
		}

		if (!best_diff)
			break;
	}

	debug("GCK: best parent: %s, best_rate = %ld, best_div = %d\n",
	      best_parent.dev->name, best_rate, best_div);

	ret = clk_enable(&best_parent);
	if (ret)
		return ret;

	writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
	tmp = readl(&pmc->pcr);
	tmp &= ~(AT91_PMC_PCR_GCKDIV | AT91_PMC_PCR_GCKCSS);
	tmp |= AT91_PMC_PCR_GCKCSS_(best_clock_source) |
	       AT91_PMC_PCR_CMD_WRITE |
	       AT91_PMC_PCR_GCKDIV_(best_div) |
	       AT91_PMC_PCR_GCKEN;
	writel(tmp, &pmc->pcr);

	while (!(readl(&pmc->sr) & AT91_PMC_GCKRDY))
		;

	return 0;
}

static struct clk_ops generic_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.get_rate = generic_clk_get_rate,
	.set_rate = generic_clk_set_rate,
};

static int generic_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct generic_clk_priv *priv = dev_get_priv(dev);
	u32 cells[GENERATED_SOURCE_MAX];
	u32 num_parents;

	num_parents = fdtdec_get_int_array_count(gd->fdt_blob,
			dev_of_offset(dev_get_parent(dev)), "clocks", cells,
			GENERATED_SOURCE_MAX);

	if (!num_parents)
		return -1;

	priv->num_parents = num_parents;

	return 0;
}

U_BOOT_DRIVER(generic_clk) = {
	.name = "generic-clk",
	.id = UCLASS_CLK,
	.probe = at91_clk_probe,
	.ofdata_to_platdata = generic_clk_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct generic_clk_priv),
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &generic_clk_ops,
};
