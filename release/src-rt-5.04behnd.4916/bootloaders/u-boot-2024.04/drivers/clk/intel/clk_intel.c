// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <clk-uclass.h>
#include <dt-bindings/clock/intel-clock.h>

static ulong intel_clk_get_rate(struct clk *clk)
{
	switch (clk->id) {
	case CLK_I2C:
		/* Hard-coded to 133MHz on current platforms */
		return 133333333;
	default:
		return -ENODEV;
	}
}

static struct clk_ops intel_clk_ops = {
	.get_rate	= intel_clk_get_rate,
};

static const struct udevice_id intel_clk_ids[] = {
	{ .compatible = "intel,apl-clk" },
	{ }
};

U_BOOT_DRIVER(intel_apl_clk) = {
	.name		= "intel_apl_clk",
	.id		= UCLASS_CLK,
	.of_match	= intel_clk_ids,
	.ops		= &intel_clk_ops,
};
