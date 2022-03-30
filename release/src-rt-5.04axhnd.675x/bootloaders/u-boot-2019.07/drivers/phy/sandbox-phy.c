// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>

struct sandbox_phy_priv {
	bool initialized;
	bool on;
	bool broken;
};

static int sandbox_phy_power_on(struct phy *phy)
{
	struct sandbox_phy_priv *priv = dev_get_priv(phy->dev);

	if (!priv->initialized)
		return -EIO;

	if (priv->broken)
		return -EIO;

	priv->on = true;

	return 0;
}

static int sandbox_phy_power_off(struct phy *phy)
{
	struct sandbox_phy_priv *priv = dev_get_priv(phy->dev);

	if (!priv->initialized)
		return -EIO;

	if (priv->broken)
		return -EIO;

	/*
	 * for validation purpose, let's says that power off
	 * works only for PHY 0
	 */
	if (phy->id)
		return -EIO;

	priv->on = false;

	return 0;
}

static int sandbox_phy_init(struct phy *phy)
{
	struct sandbox_phy_priv *priv = dev_get_priv(phy->dev);

	priv->initialized = true;
	priv->on = true;

	return 0;
}

static int sandbox_phy_exit(struct phy *phy)
{
	struct sandbox_phy_priv *priv = dev_get_priv(phy->dev);

	priv->initialized = false;
	priv->on = false;

	return 0;
}

static int sandbox_phy_probe(struct udevice *dev)
{
	struct sandbox_phy_priv *priv = dev_get_priv(dev);

	priv->initialized = false;
	priv->on = false;
	priv->broken = dev_read_bool(dev, "broken");

	return 0;
}

static struct phy_ops sandbox_phy_ops = {
	.power_on = sandbox_phy_power_on,
	.power_off = sandbox_phy_power_off,
	.init = sandbox_phy_init,
	.exit = sandbox_phy_exit,
};

static const struct udevice_id sandbox_phy_ids[] = {
	{ .compatible = "sandbox,phy" },
	{ }
};

U_BOOT_DRIVER(phy_sandbox) = {
	.name		= "phy_sandbox",
	.id		= UCLASS_PHY,
	.of_match	= sandbox_phy_ids,
	.ops		= &sandbox_phy_ops,
	.probe		= sandbox_phy_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_phy_priv),
};
