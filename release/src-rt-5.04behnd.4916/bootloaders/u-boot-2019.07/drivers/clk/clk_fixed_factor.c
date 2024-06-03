// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Author: Anup Patel <anup.patel@wdc.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>

struct clk_fixed_factor {
	struct clk parent;
	unsigned int div;
	unsigned int mult;
};

#define to_clk_fixed_factor(dev)	\
	((struct clk_fixed_factor *)dev_get_platdata(dev))

static ulong clk_fixed_factor_get_rate(struct clk *clk)
{
	uint64_t rate;
	struct clk_fixed_factor *ff = to_clk_fixed_factor(clk->dev);

	if (clk->id != 0)
		return -EINVAL;

	rate = clk_get_rate(&ff->parent);
	if (IS_ERR_VALUE(rate))
		return rate;

	do_div(rate, ff->div);

	return rate * ff->mult;
}

const struct clk_ops clk_fixed_factor_ops = {
	.get_rate = clk_fixed_factor_get_rate,
};

static int clk_fixed_factor_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	int err;
	struct clk_fixed_factor *ff = to_clk_fixed_factor(dev);

	err = clk_get_by_index(dev, 0, &ff->parent);
	if (err)
		return err;

	ff->div = dev_read_u32_default(dev, "clock-div", 1);
	ff->mult = dev_read_u32_default(dev, "clock-mult", 1);
#endif

	return 0;
}

static const struct udevice_id clk_fixed_factor_match[] = {
	{
		.compatible = "fixed-factor-clock",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(clk_fixed_factor) = {
	.name = "fixed_factor_clock",
	.id = UCLASS_CLK,
	.of_match = clk_fixed_factor_match,
	.ofdata_to_platdata = clk_fixed_factor_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct clk_fixed_factor),
	.ops = &clk_fixed_factor_ops,
};
