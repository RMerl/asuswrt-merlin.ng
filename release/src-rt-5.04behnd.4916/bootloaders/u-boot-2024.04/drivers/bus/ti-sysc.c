// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments sysc interconnect target driver
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>

enum ti_sysc_clocks {
	TI_SYSC_FCK,
	TI_SYSC_ICK,
	TI_SYSC_MAX_CLOCKS,
};

static const char *const clock_names[] = {"fck", "ick"};

struct ti_sysc_priv {
	int clocks_count;
	struct clk clocks[TI_SYSC_MAX_CLOCKS];
};

static const struct udevice_id ti_sysc_ids[] = {
	{.compatible = "ti,sysc-omap2"},
	{.compatible = "ti,sysc-omap4"},
	{.compatible = "ti,sysc-omap4-simple"},
	{.compatible = "ti,sysc-omap3430-sr"},
	{.compatible = "ti,sysc-omap3630-sr"},
	{.compatible = "ti,sysc-omap4-sr"},
	{.compatible = "ti,sysc-omap3-sham"},
	{.compatible = "ti,sysc-omap-aes"},
	{.compatible = "ti,sysc-mcasp"},
	{.compatible = "ti,sysc-usb-host-fs"},
	{}
};

static int ti_sysc_get_one_clock(struct udevice *dev, enum ti_sysc_clocks index)
{
	struct ti_sysc_priv *priv = dev_get_priv(dev);
	const char *name;
	int err;

	switch (index) {
	case TI_SYSC_FCK:
		break;
	case TI_SYSC_ICK:
		break;
	default:
		return -EINVAL;
	}

	name = clock_names[index];

	err = clk_get_by_name(dev, name, &priv->clocks[index]);
	if (err) {
		if (err == -ENODATA)
			return 0;

		dev_err(dev, "failed to get %s clock\n", name);
		return err;
	}

	return 0;
}

static int ti_sysc_put_clocks(struct udevice *dev)
{
	struct ti_sysc_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_release_all(priv->clocks, priv->clocks_count);
	if (err)
		dev_err(dev, "failed to release all clocks\n");

	return err;
}

static int ti_sysc_get_clocks(struct udevice *dev)
{
	struct ti_sysc_priv *priv = dev_get_priv(dev);
	int i, err;

	for (i = 0; i < TI_SYSC_MAX_CLOCKS; i++) {
		err = ti_sysc_get_one_clock(dev, i);
		if (!err)
			priv->clocks_count++;
		else if (err != -ENOENT)
			return err;
	}

	return 0;
}

static int ti_sysc_child_post_remove(struct udevice *dev)
{
	struct ti_sysc_priv *priv = dev_get_priv(dev->parent);
	int i, err;

	for (i = 0; i < priv->clocks_count; i++) {
		err = clk_disable(&priv->clocks[i]);
		if (err) {
			dev_err(dev->parent, "failed to disable %s clock\n",
				clock_names[i]);
			return err;
		}
	}

	return 0;
}

static int ti_sysc_child_pre_probe(struct udevice *dev)
{
	struct ti_sysc_priv *priv = dev_get_priv(dev->parent);
	int i, err;

	for (i = 0; i < priv->clocks_count; i++) {
		err = clk_enable(&priv->clocks[i]);
		if (err) {
			dev_err(dev->parent, "failed to enable %s clock\n",
				clock_names[i]);
			return err;
		}
	}

	return 0;
}

static int ti_sysc_remove(struct udevice *dev)
{
	return ti_sysc_put_clocks(dev);
}

static int ti_sysc_probe(struct udevice *dev)
{
	int err;

	err = ti_sysc_get_clocks(dev);
	if (err)
		goto clocks_err;

	return 0;

clocks_err:
	ti_sysc_put_clocks(dev);
	return err;
}

U_BOOT_DRIVER(ti_sysc) = {
	.name = "ti_sysc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = ti_sysc_ids,
	.probe = ti_sysc_probe,
	.remove = ti_sysc_remove,
	.child_pre_probe = ti_sysc_child_pre_probe,
	.child_post_remove = ti_sysc_child_post_remove,
	.priv_auto = sizeof(struct ti_sysc_priv)
};
