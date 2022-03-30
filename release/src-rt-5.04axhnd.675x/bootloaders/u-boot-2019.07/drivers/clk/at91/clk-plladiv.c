// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Microhip / Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

static int at91_plladiv_clk_enable(struct clk *clk)
{
	return 0;
}

static ulong at91_plladiv_clk_get_rate(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &source);
	if (ret)
		return -EINVAL;

	clk_rate = clk_get_rate(&source);
	if (readl(&pmc->mckr) & AT91_PMC_MCKR_PLLADIV_2)
		clk_rate /= 2;

	return clk_rate;
}

static ulong at91_plladiv_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	ulong parent_rate;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &source);
	if (ret)
		return -EINVAL;

	parent_rate = clk_get_rate(&source);
	if ((parent_rate != rate) && ((parent_rate) / 2 != rate))
		return -EINVAL;

	if (parent_rate != rate) {
		writel((readl(&pmc->mckr) | AT91_PMC_MCKR_PLLADIV_2),
		       &pmc->mckr);
	}

	return 0;
}

static struct clk_ops at91_plladiv_clk_ops = {
	.enable = at91_plladiv_clk_enable,
	.get_rate = at91_plladiv_clk_get_rate,
	.set_rate = at91_plladiv_clk_set_rate,
};

static int at91_plladiv_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id at91_plladiv_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-plldiv" },
	{}
};

U_BOOT_DRIVER(at91_plladiv_clk) = {
	.name = "at91-plladiv-clk",
	.id = UCLASS_CLK,
	.of_match = at91_plladiv_clk_match,
	.probe = at91_plladiv_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &at91_plladiv_clk_ops,
};
