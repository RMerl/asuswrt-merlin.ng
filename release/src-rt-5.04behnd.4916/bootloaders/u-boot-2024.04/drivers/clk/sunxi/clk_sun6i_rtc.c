// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Copyright (C) 2020 Samuel Holland <samuel@sholland.org>
 */

#include <clk-uclass.h>
#include <dm.h>

static int clk_sun6i_rtc_enable(struct clk *clk)
{
	return 0;
}

static const struct clk_ops clk_sun6i_rtc_ops = {
	.enable = clk_sun6i_rtc_enable,
};

static const struct udevice_id sun6i_rtc_ids[] = {
	{ .compatible = "allwinner,sun6i-a31-rtc" },
	{ .compatible = "allwinner,sun8i-a23-rtc" },
	{ .compatible = "allwinner,sun8i-h3-rtc" },
	{ .compatible = "allwinner,sun8i-r40-rtc" },
	{ .compatible = "allwinner,sun8i-v3-rtc" },
	{ .compatible = "allwinner,sun50i-h5-rtc" },
	{ .compatible = "allwinner,sun50i-h6-rtc" },
	{ .compatible = "allwinner,sun50i-h616-rtc" },
	{ .compatible = "allwinner,sun50i-r329-rtc" },
	{ }
};

U_BOOT_DRIVER(clk_sun6i_rtc) = {
	.name		= "clk_sun6i_rtc",
	.id		= UCLASS_CLK,
	.of_match	= sun6i_rtc_ids,
	.ops		= &clk_sun6i_rtc_ops,
};
