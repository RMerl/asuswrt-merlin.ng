// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clock/boston-clock.h>
#include <regmap.h>
#include <syscon.h>

struct clk_boston {
	struct regmap *regmap;
};

#define BOSTON_PLAT_MMCMDIV		0x30
# define BOSTON_PLAT_MMCMDIV_CLK0DIV	(0xff << 0)
# define BOSTON_PLAT_MMCMDIV_INPUT	(0xff << 8)
# define BOSTON_PLAT_MMCMDIV_MUL	(0xff << 16)
# define BOSTON_PLAT_MMCMDIV_CLK1DIV	(0xff << 24)

static uint32_t ext_field(uint32_t val, uint32_t mask)
{
	return (val & mask) >> (ffs(mask) - 1);
}

static ulong clk_boston_get_rate(struct clk *clk)
{
	struct clk_boston *state = dev_get_platdata(clk->dev);
	uint32_t in_rate, mul, div;
	uint mmcmdiv;
	int err;

	err = regmap_read(state->regmap, BOSTON_PLAT_MMCMDIV, &mmcmdiv);
	if (err)
		return 0;

	in_rate = ext_field(mmcmdiv, BOSTON_PLAT_MMCMDIV_INPUT);
	mul = ext_field(mmcmdiv, BOSTON_PLAT_MMCMDIV_MUL);

	switch (clk->id) {
	case BOSTON_CLK_SYS:
		div = ext_field(mmcmdiv, BOSTON_PLAT_MMCMDIV_CLK0DIV);
		break;
	case BOSTON_CLK_CPU:
		div = ext_field(mmcmdiv, BOSTON_PLAT_MMCMDIV_CLK1DIV);
		break;
	default:
		return 0;
	}

	return (in_rate * mul * 1000000) / div;
}

const struct clk_ops clk_boston_ops = {
	.get_rate = clk_boston_get_rate,
};

static int clk_boston_ofdata_to_platdata(struct udevice *dev)
{
	struct clk_boston *state = dev_get_platdata(dev);
	struct udevice *syscon;
	int err;

	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "regmap", &syscon);
	if (err) {
		pr_err("unable to find syscon device\n");
		return err;
	}

	state->regmap = syscon_get_regmap(syscon);
	if (!state->regmap) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	return 0;
}

static const struct udevice_id clk_boston_match[] = {
	{
		.compatible = "img,boston-clock",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(clk_boston) = {
	.name = "boston_clock",
	.id = UCLASS_CLK,
	.of_match = clk_boston_match,
	.ofdata_to_platdata = clk_boston_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct clk_boston),
	.ops = &clk_boston_ops,
};
