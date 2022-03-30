// SPDX-License-Identifier: GPL-2.0
/*
 * Amlogic Meson Reset Controller driver
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <regmap.h>

#define REG_COUNT	8
#define BITS_PER_REG	32
#define LEVEL_OFFSET	0x7c

struct meson_reset_priv {
	struct regmap *regmap;
};

static int meson_reset_request(struct reset_ctl *reset_ctl)
{
	if (reset_ctl->id > (REG_COUNT * BITS_PER_REG))
		return -EINVAL;

	return 0;
}

static int meson_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int meson_reset_level(struct reset_ctl *reset_ctl, bool assert)
{
	struct meson_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	uint bank = reset_ctl->id / BITS_PER_REG;
	uint offset = reset_ctl->id % BITS_PER_REG;
	uint reg_offset = LEVEL_OFFSET + (bank << 2);
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
	.free = meson_reset_free,
	.rst_assert = meson_reset_assert,
	.rst_deassert = meson_reset_deassert,
};

static const struct udevice_id meson_reset_ids[] = {                          
	{ .compatible = "amlogic,meson-gxbb-reset" },                                  
	{ .compatible = "amlogic,meson-axg-reset" },
	{ }                                                                     
};  

static int meson_reset_probe(struct udevice *dev)
{
	struct meson_reset_priv *priv = dev_get_priv(dev);
	
	return regmap_init_mem(dev_ofnode(dev), &priv->regmap);
}

U_BOOT_DRIVER(meson_reset) = {
	.name = "meson_reset",
	.id = UCLASS_RESET,
	.of_match = meson_reset_ids,
	.probe = meson_reset_probe,
	.ops = &meson_reset_ops,
	.priv_auto_alloc_size = sizeof(struct meson_reset_priv),
};
