// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sifive, Inc.
 * Author: Sagar Kadam <sagar.kadam@sifive.com>
 */

#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/bitops.h>

#define PRCI_RESETREG_OFFSET 0x28

struct sifive_reset_priv {
	void *base;
	/* number of reset signals */
	int nr_reset;
};

static int sifive_rst_trigger(struct reset_ctl *rst, bool level)
{
	struct sifive_reset_priv *priv = dev_get_priv(rst->dev);
	int id = rst->id;
	int regval = readl(priv->base + PRCI_RESETREG_OFFSET);

	/* Derive bitposition from rst id */
	if (level)
		/* Reset deassert */
		regval |= BIT(id);
	else
		/* Reset assert */
		regval &= ~BIT(id);

	writel(regval, priv->base + PRCI_RESETREG_OFFSET);

	return 0;
}

static int sifive_reset_assert(struct reset_ctl *rst)
{
	return sifive_rst_trigger(rst, false);
}

static int sifive_reset_deassert(struct reset_ctl *rst)
{
	return sifive_rst_trigger(rst, true);
}

static int sifive_reset_request(struct reset_ctl *rst)
{
	struct sifive_reset_priv *priv = dev_get_priv(rst->dev);

	debug("%s(rst=%p) (dev=%p, id=%lu) (nr_reset=%d)\n", __func__,
	      rst, rst->dev, rst->id, priv->nr_reset);

	if (rst->id > priv->nr_reset)
		return -EINVAL;

	return 0;
}

static int sifive_reset_probe(struct udevice *dev)
{
	struct sifive_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -ENOMEM;

	return 0;
}

int sifive_reset_bind(struct udevice *dev, ulong count)
{
	struct udevice *rst_dev;
	struct sifive_reset_priv *priv;
	int ret;

	ret = device_bind_driver_to_node(dev, "sifive-reset", "reset",
					 dev_ofnode(dev), &rst_dev);
	if (ret) {
		dev_err(dev, "failed to bind sifive_reset driver (ret=%d)\n", ret);
		return ret;
	}
	priv = malloc(sizeof(struct sifive_reset_priv));
	priv->nr_reset = count;
	dev_set_priv(rst_dev, priv);

	return 0;
}

const struct reset_ops sifive_reset_ops = {
	.request = sifive_reset_request,
	.rst_assert = sifive_reset_assert,
	.rst_deassert = sifive_reset_deassert,
};

U_BOOT_DRIVER(sifive_reset) = {
	.name		= "sifive-reset",
	.id		= UCLASS_RESET,
	.ops		= &sifive_reset_ops,
	.probe		= sifive_reset_probe,
	.priv_auto	= sizeof(struct sifive_reset_priv),
};
