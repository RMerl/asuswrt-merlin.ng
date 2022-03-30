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

static int main_osc_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;

	if (readl(&pmc->sr) & AT91_PMC_MOSCSELS)
		return 0;

	return -EINVAL;
}

static ulong main_osc_clk_get_rate(struct clk *clk)
{
	return gd->arch.main_clk_rate_hz;
}

static struct clk_ops main_osc_clk_ops = {
	.enable = main_osc_clk_enable,
	.get_rate = main_osc_clk_get_rate,
};

static int main_osc_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id main_osc_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-main" },
	{}
};

U_BOOT_DRIVER(at91sam9x5_main_osc_clk) = {
	.name = "at91sam9x5-main-osc-clk",
	.id = UCLASS_CLK,
	.of_match = main_osc_clk_match,
	.probe = main_osc_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &main_osc_clk_ops,
};
