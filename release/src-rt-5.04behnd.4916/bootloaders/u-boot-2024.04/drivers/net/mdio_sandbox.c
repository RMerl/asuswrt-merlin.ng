// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <dm.h>
#include <errno.h>
#include <miiphy.h>

#define SANDBOX_PHY_ADDR	5
#define SANDBOX_PHY_REG_CNT	2

struct mdio_sandbox_priv {
	int enabled;
	u16 reg[SANDBOX_PHY_REG_CNT];
};

static int mdio_sandbox_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct mdio_sandbox_priv *priv = dev_get_priv(dev);

	if (!priv->enabled)
		return -ENODEV;

	if (addr != SANDBOX_PHY_ADDR)
		return -ENODEV;
	if (devad != MDIO_DEVAD_NONE)
		return -ENODEV;
	if (reg < 0 || reg >= SANDBOX_PHY_REG_CNT)
		return -ENODEV;

	return priv->reg[reg];
}

static int mdio_sandbox_write(struct udevice *dev, int addr, int devad, int reg,
			      u16 val)
{
	struct mdio_sandbox_priv *priv = dev_get_priv(dev);

	if (!priv->enabled)
		return -ENODEV;

	if (addr != SANDBOX_PHY_ADDR)
		return -ENODEV;
	if (devad != MDIO_DEVAD_NONE)
		return -ENODEV;
	if (reg < 0 || reg >= SANDBOX_PHY_REG_CNT)
		return -ENODEV;

	priv->reg[reg] = val;

	return 0;
}

static int mdio_sandbox_reset(struct udevice *dev)
{
	struct mdio_sandbox_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < SANDBOX_PHY_REG_CNT; i++)
		priv->reg[i] = 0;

	return 0;
}

static const struct mdio_ops mdio_sandbox_ops = {
	.read = mdio_sandbox_read,
	.write = mdio_sandbox_write,
	.reset = mdio_sandbox_reset,
};

static int mdio_sandbox_probe(struct udevice *dev)
{
	struct mdio_sandbox_priv *priv = dev_get_priv(dev);

	priv->enabled = 1;

	return 0;
}

static const struct udevice_id mdio_sandbox_ids[] = {
	{ .compatible = "sandbox,mdio" },
	{ }
};

U_BOOT_DRIVER(mdio_sandbox) = {
	.name		= "mdio_sandbox",
	.id		= UCLASS_MDIO,
	.of_match	= mdio_sandbox_ids,
	.probe		= mdio_sandbox_probe,
	.ops		= &mdio_sandbox_ops,
	.priv_auto	= sizeof(struct mdio_sandbox_priv),
};
