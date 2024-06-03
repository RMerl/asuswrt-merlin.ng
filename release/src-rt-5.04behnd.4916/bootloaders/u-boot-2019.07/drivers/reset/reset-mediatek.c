// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *	   Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>

struct mediatek_reset_priv {
	struct regmap *regmap;
	u32 regofs;
	u32 nr_resets;
};

static int mediatek_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int mediatek_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int mediatek_reset_assert(struct reset_ctl *reset_ctl)
{
	struct mediatek_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;

	if (id >= priv->nr_resets)
		return -EINVAL;

	return regmap_update_bits(priv->regmap,
		priv->regofs + ((id / 32) << 2), BIT(id % 32), BIT(id % 32));
}

static int mediatek_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct mediatek_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;

	if (id >= priv->nr_resets)
		return -EINVAL;

	return regmap_update_bits(priv->regmap,
		priv->regofs + ((id / 32) << 2), BIT(id % 32), 0);
}

struct reset_ops mediatek_reset_ops = {
	.request = mediatek_reset_request,
	.free = mediatek_reset_free,
	.rst_assert = mediatek_reset_assert,
	.rst_deassert = mediatek_reset_deassert,
};

static int mediatek_reset_probe(struct udevice *dev)
{
	struct mediatek_reset_priv *priv = dev_get_priv(dev);

	if (!priv->regofs && !priv->nr_resets)
		return -EINVAL;

	priv->regmap = syscon_node_to_regmap(dev_ofnode(dev));
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	return 0;
}

int mediatek_reset_bind(struct udevice *pdev, u32 regofs, u32 num_regs)
{
	struct udevice *rst_dev;
	struct mediatek_reset_priv *priv;
	int ret;

	ret = device_bind_driver_to_node(pdev, "mediatek_reset", "reset",
					 dev_ofnode(pdev), &rst_dev);
	if (ret)
		return ret;

	priv = malloc(sizeof(struct mediatek_reset_priv));
	priv->regofs = regofs;
	priv->nr_resets = num_regs * 32;
	rst_dev->priv = priv;

	return 0;
}

U_BOOT_DRIVER(mediatek_reset) = {
	.name = "mediatek_reset",
	.id = UCLASS_RESET,
	.probe = mediatek_reset_probe,
	.ops = &mediatek_reset_ops,
	.priv_auto_alloc_size = sizeof(struct mediatek_reset_priv),
};
