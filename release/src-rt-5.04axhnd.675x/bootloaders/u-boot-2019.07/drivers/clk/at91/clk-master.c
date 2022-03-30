// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong at91_master_clk_get_rate(struct clk *clk)
{
	return gd->arch.mck_rate_hz;
}

static struct clk_ops at91_master_clk_ops = {
	.get_rate = at91_master_clk_get_rate,
};

static const struct udevice_id at91_master_clk_match[] = {
	{ .compatible = "atmel,at91rm9200-clk-master" },
	{ .compatible = "atmel,at91sam9x5-clk-master" },
	{}
};

U_BOOT_DRIVER(at91_master_clk) = {
	.name = "at91-master-clk",
	.id = UCLASS_CLK,
	.of_match = at91_master_clk_match,
	.ops = &at91_master_clk_ops,
};
