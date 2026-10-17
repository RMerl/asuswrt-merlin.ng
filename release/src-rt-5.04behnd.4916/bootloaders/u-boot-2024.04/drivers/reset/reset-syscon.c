// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Sean Anderson
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <reset.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/err.h>

struct syscon_reset_priv {
	struct regmap *regmap;
	uint offset;
	uint mask;
	bool assert_high;
};

static int syscon_reset_request(struct reset_ctl *rst)
{
	struct syscon_reset_priv *priv = dev_get_priv(rst->dev);

	if (BIT(rst->id) & priv->mask)
		return 0;
	else
		return -EINVAL;
}

static int syscon_reset_assert(struct reset_ctl *rst)
{
	struct syscon_reset_priv *priv = dev_get_priv(rst->dev);

	return regmap_update_bits(priv->regmap, priv->offset, BIT(rst->id),
				  priv->assert_high ? BIT(rst->id) : 0);
}

static int syscon_reset_deassert(struct reset_ctl *rst)
{
	struct syscon_reset_priv *priv = dev_get_priv(rst->dev);

	return regmap_update_bits(priv->regmap, priv->offset, BIT(rst->id),
				  priv->assert_high ? 0 : BIT(rst->id));
}

static const struct reset_ops syscon_reset_ops = {
	.request = syscon_reset_request,
	.rst_assert = syscon_reset_assert,
	.rst_deassert = syscon_reset_deassert,
};

int syscon_reset_probe(struct udevice *dev)
{
	struct syscon_reset_priv *priv = dev_get_priv(dev);

	priv->regmap = syscon_regmap_lookup_by_phandle(dev, "regmap");
	if (IS_ERR(priv->regmap))
		return -ENODEV;

	priv->offset = dev_read_u32_default(dev, "offset", 0);
	priv->mask = dev_read_u32_default(dev, "mask", 0);
	priv->assert_high = dev_read_u32_default(dev, "assert-high", true);

	return 0;
}

static const struct udevice_id syscon_reset_ids[] = {
	{ .compatible = "syscon-reset" },
	{ },
};

U_BOOT_DRIVER(syscon_reset) = {
	.name = "syscon_reset",
	.id = UCLASS_RESET,
	.of_match = syscon_reset_ids,
	.probe = syscon_reset_probe,
	.priv_auto	= sizeof(struct syscon_reset_priv),
	.ops = &syscon_reset_ops,
};
