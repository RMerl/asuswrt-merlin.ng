// SPDX-License-Identifier: GPL-2.0+
/*
 * TI DPLL x2 clock support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Loosely based on Linux kernel drivers/clk/ti/dpll.c
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/clk-provider.h>

struct clk_ti_am3_dpll_x2_priv {
	struct clk parent;
};

static ulong clk_ti_am3_dpll_x2_get_rate(struct clk *clk)
{
	struct clk_ti_am3_dpll_x2_priv *priv = dev_get_priv(clk->dev);
	unsigned long rate;

	rate = clk_get_rate(&priv->parent);
	if (IS_ERR_VALUE(rate))
		return rate;

	rate *= 2;
	dev_dbg(clk->dev, "rate=%ld\n", rate);
	return rate;
}

const struct clk_ops clk_ti_am3_dpll_x2_ops = {
	.get_rate = clk_ti_am3_dpll_x2_get_rate,
};

static int clk_ti_am3_dpll_x2_remove(struct udevice *dev)
{
	struct clk_ti_am3_dpll_x2_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_release_all(&priv->parent, 1);
	if (err) {
		dev_err(dev, "failed to release parent clock\n");
		return err;
	}

	return 0;
}

static int clk_ti_am3_dpll_x2_probe(struct udevice *dev)
{
	struct clk_ti_am3_dpll_x2_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_get_by_index(dev, 0, &priv->parent);
	if (err) {
		dev_err(dev, "%s: failed to get parent clock\n", __func__);
		return err;
	}

	return 0;
}

static const struct udevice_id clk_ti_am3_dpll_x2_of_match[] = {
	{.compatible = "ti,am3-dpll-x2-clock"},
	{}
};

U_BOOT_DRIVER(clk_ti_am3_dpll_x2) = {
	.name = "ti_am3_dpll_x2_clock",
	.id = UCLASS_CLK,
	.of_match = clk_ti_am3_dpll_x2_of_match,
	.probe = clk_ti_am3_dpll_x2_probe,
	.remove = clk_ti_am3_dpll_x2_remove,
	.priv_auto = sizeof(struct clk_ti_am3_dpll_x2_priv),
	.ops = &clk_ti_am3_dpll_x2_ops,
};
