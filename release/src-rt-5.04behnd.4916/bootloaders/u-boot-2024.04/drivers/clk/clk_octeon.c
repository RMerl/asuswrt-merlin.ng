// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dt-bindings/clock/octeon-clock.h>

DECLARE_GLOBAL_DATA_PTR;

struct octeon_clk_priv {
	u64 core_clk;
	u64 io_clk;
};

static int octeon_clk_enable(struct clk *clk)
{
	/* Nothing to do on Octeon */
	return 0;
}

static ulong octeon_clk_get_rate(struct clk *clk)
{
	struct octeon_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case OCTEON_CLK_CORE:
		return priv->core_clk;

	case OCTEON_CLK_IO:
		return priv->io_clk;

	default:
		return 0;
	}

	return 0;
}

static struct clk_ops octeon_clk_ops = {
	.enable = octeon_clk_enable,
	.get_rate = octeon_clk_get_rate,
};

static const struct udevice_id octeon_clk_ids[] = {
	{ .compatible = "mrvl,octeon-clk" },
	{ /* sentinel */ }
};

static int octeon_clk_probe(struct udevice *dev)
{
	struct octeon_clk_priv *priv = dev_get_priv(dev);

	/*
	 * The clock values are already read into GD, lets just store them
	 * in priv data
	 */
	priv->core_clk = gd->cpu_clk;
	priv->io_clk = gd->bus_clk;

	return 0;
}

U_BOOT_DRIVER(clk_octeon) = {
	.name = "clk_octeon",
	.id = UCLASS_CLK,
	.of_match = octeon_clk_ids,
	.ops = &octeon_clk_ops,
	.probe = octeon_clk_probe,
	.priv_auto	= sizeof(struct octeon_clk_priv),
};
