// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <dm.h>
#include <errno.h>
#include <miiphy.h>

/* macros copied over from mdio_sandbox.c */
#define SANDBOX_PHY_ADDR	5
#define SANDBOX_PHY_REG_CNT	2

struct mdio_mux_sandbox_priv {
	int enabled;
	int sel;
};

static int mdio_mux_sandbox_mark_selection(struct udevice *dev, int sel)
{
	struct udevice *mdio;
	int err;

	/*
	 * find the sandbox parent mdio and write a register on the PHY there
	 * so the mux test can verify selection.
	 */
	err = uclass_get_device_by_name(UCLASS_MDIO, "mdio-test", &mdio);
	if (err)
		return err;
	return dm_mdio_write(mdio, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			     SANDBOX_PHY_REG_CNT - 1, (u16)sel);
}

static int mdio_mux_sandbox_select(struct udevice *dev, int cur, int sel)
{
	struct mdio_mux_sandbox_priv *priv = dev_get_priv(dev);

	if (!priv->enabled)
		return -ENODEV;

	if (cur != priv->sel)
		return -EINVAL;

	priv->sel = sel;
	mdio_mux_sandbox_mark_selection(dev, priv->sel);

	return 0;
}

static int mdio_mux_sandbox_deselect(struct udevice *dev, int sel)
{
	struct mdio_mux_sandbox_priv *priv = dev_get_priv(dev);

	if (!priv->enabled)
		return -ENODEV;

	if (sel != priv->sel)
		return -EINVAL;

	priv->sel = -1;
	mdio_mux_sandbox_mark_selection(dev, priv->sel);

	return 0;
}

static const struct mdio_mux_ops mdio_mux_sandbox_ops = {
	.select = mdio_mux_sandbox_select,
	.deselect = mdio_mux_sandbox_deselect,
};

static int mdio_mux_sandbox_probe(struct udevice *dev)
{
	struct mdio_mux_sandbox_priv *priv = dev_get_priv(dev);

	priv->enabled = 1;
	priv->sel = -1;

	return 0;
}

static const struct udevice_id mdio_mux_sandbox_ids[] = {
	{ .compatible = "sandbox,mdio-mux" },
	{ }
};

U_BOOT_DRIVER(mdio_mux_sandbox) = {
	.name		= "mdio_mux_sandbox",
	.id		= UCLASS_MDIO_MUX,
	.of_match	= mdio_mux_sandbox_ids,
	.probe		= mdio_mux_sandbox_probe,
	.ops		= &mdio_mux_sandbox_ops,
	.priv_auto	= sizeof(struct mdio_mux_sandbox_priv),
};
