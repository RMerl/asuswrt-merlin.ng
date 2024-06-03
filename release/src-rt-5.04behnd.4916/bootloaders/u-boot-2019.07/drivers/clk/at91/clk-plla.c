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

static int plla_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;

	if (readl(&pmc->sr) & AT91_PMC_LOCKA)
		return 0;

	return -EINVAL;
}

static ulong plla_clk_get_rate(struct clk *clk)
{
	return gd->arch.plla_rate_hz;
}

static struct clk_ops plla_clk_ops = {
	.enable = plla_clk_enable,
	.get_rate = plla_clk_get_rate,
};

static int plla_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id plla_clk_match[] = {
	{ .compatible = "atmel,sama5d3-clk-pll" },
	{}
};

U_BOOT_DRIVER(at91_plla_clk) = {
	.name = "at91-plla-clk",
	.id = UCLASS_CLK,
	.of_match = plla_clk_match,
	.probe = plla_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &plla_clk_ops,
};
