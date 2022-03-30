// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>

static int at91_slow_clk_enable(struct clk *clk)
{
	return 0;
}

static ulong at91_slow_clk_get_rate(struct clk *clk)
{
	return CONFIG_SYS_AT91_SLOW_CLOCK;
}

static struct clk_ops at91_slow_clk_ops = {
	.enable = at91_slow_clk_enable,
	.get_rate = at91_slow_clk_get_rate,
};

static const struct udevice_id at91_slow_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-slow" },
	{}
};

U_BOOT_DRIVER(at91_slow_clk) = {
	.name = "at91-slow-clk",
	.id = UCLASS_CLK,
	.of_match = at91_slow_clk_match,
	.ops = &at91_slow_clk_ops,
};
