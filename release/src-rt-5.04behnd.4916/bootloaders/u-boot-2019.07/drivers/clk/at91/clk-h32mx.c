// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/util.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

#define H32MX_MAX_FREQ	90000000

static ulong sama5d4_h32mx_clk_get_rate(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	ulong rate = gd->arch.mck_rate_hz;

	if (readl(&pmc->mckr) & AT91_PMC_MCKR_H32MXDIV)
		rate /= 2;

	if (rate > H32MX_MAX_FREQ)
		dev_dbg(clk->dev, "H32MX clock is too fast\n");

	return rate;
}

static struct clk_ops sama5d4_h32mx_clk_ops = {
	.get_rate = sama5d4_h32mx_clk_get_rate,
};

static int sama5d4_h32mx_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id sama5d4_h32mx_clk_match[] = {
	{ .compatible = "atmel,sama5d4-clk-h32mx" },
	{}
};

U_BOOT_DRIVER(sama5d4_h32mx_clk) = {
	.name = "sama5d4-h32mx-clk",
	.id = UCLASS_CLK,
	.of_match = sama5d4_h32mx_clk_match,
	.probe = sama5d4_h32mx_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &sama5d4_h32mx_clk_ops,
};
