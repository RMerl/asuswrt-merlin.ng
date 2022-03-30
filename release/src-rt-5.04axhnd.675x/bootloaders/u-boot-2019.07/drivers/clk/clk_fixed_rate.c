// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>

struct clk_fixed_rate {
	unsigned long fixed_rate;
};

#define to_clk_fixed_rate(dev)	((struct clk_fixed_rate *)dev_get_platdata(dev))

static ulong clk_fixed_rate_get_rate(struct clk *clk)
{
	if (clk->id != 0)
		return -EINVAL;

	return to_clk_fixed_rate(clk->dev)->fixed_rate;
}

const struct clk_ops clk_fixed_rate_ops = {
	.get_rate = clk_fixed_rate_get_rate,
};

static int clk_fixed_rate_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	to_clk_fixed_rate(dev)->fixed_rate =
		dev_read_u32_default(dev, "clock-frequency", 0);
#endif

	return 0;
}

static const struct udevice_id clk_fixed_rate_match[] = {
	{
		.compatible = "fixed-clock",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(clk_fixed_rate) = {
	.name = "fixed_rate_clock",
	.id = UCLASS_CLK,
	.of_match = clk_fixed_rate_match,
	.ofdata_to_platdata = clk_fixed_rate_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct clk_fixed_rate),
	.ops = &clk_fixed_rate_ops,
};
