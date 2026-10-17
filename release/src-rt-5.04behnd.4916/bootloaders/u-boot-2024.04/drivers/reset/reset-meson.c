// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Reset Controller driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <regmap.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define BITS_PER_REG	32

struct meson_reset_drvdata {
	unsigned int reg_count;
	unsigned int level_offset;
};

struct meson_reset_priv {
	struct regmap *regmap;
	struct meson_reset_drvdata *drvdata;
};

static int meson_reset_request(struct reset_ctl *reset_ctl)
{
	struct meson_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct meson_reset_drvdata *data = priv->drvdata;

	if (reset_ctl->id > (data->reg_count * BITS_PER_REG))
		return -EINVAL;

	return 0;
}

static int meson_reset_level(struct reset_ctl *reset_ctl, bool assert)
{
	struct meson_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct meson_reset_drvdata *data = priv->drvdata;
	uint bank = reset_ctl->id / BITS_PER_REG;
	uint offset = reset_ctl->id % BITS_PER_REG;
	uint reg_offset = data->level_offset + (bank << 2);
	uint val;

	regmap_read(priv->regmap, reg_offset, &val);
	if (assert)
		val &= ~BIT(offset);
	else
		val |= BIT(offset);
	regmap_write(priv->regmap, reg_offset, val);

	return 0;
}

static int meson_reset_assert(struct reset_ctl *reset_ctl)
{
	return meson_reset_level(reset_ctl, true);
}

static int meson_reset_deassert(struct reset_ctl *reset_ctl)
{
	return meson_reset_level(reset_ctl, false);
}

struct reset_ops meson_reset_ops = {
	.request = meson_reset_request,
	.rst_assert = meson_reset_assert,
	.rst_deassert = meson_reset_deassert,
};

static const struct meson_reset_drvdata meson_gxbb_data = {
	.reg_count = 8,
	.level_offset = 0x7c,
};

static const struct meson_reset_drvdata meson_a1_data = {
	.reg_count = 3,
	.level_offset = 0x40,
};

static const struct udevice_id meson_reset_ids[] = {
	{
		.compatible = "amlogic,meson-gxbb-reset",
		.data = (ulong)&meson_gxbb_data,
	},
	{
		.compatible = "amlogic,meson-axg-reset",
		.data = (ulong)&meson_gxbb_data,
	},
	{
		.compatible = "amlogic,meson-a1-reset",
		.data = (ulong)&meson_a1_data,
	},
	{ }
};

static int meson_reset_probe(struct udevice *dev)
{
	struct meson_reset_priv *priv = dev_get_priv(dev);
	priv->drvdata = (struct meson_reset_drvdata *)dev_get_driver_data(dev);

	return regmap_init_mem(dev_ofnode(dev), &priv->regmap);
}

U_BOOT_DRIVER(meson_reset) = {
	.name = "meson_reset",
	.id = UCLASS_RESET,
	.of_match = meson_reset_ids,
	.probe = meson_reset_probe,
	.ops = &meson_reset_ops,
	.priv_auto	= sizeof(struct meson_reset_priv),
};
