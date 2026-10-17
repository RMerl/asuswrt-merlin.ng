// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Arm Ltd
 * Author: Liviu Dudau <liviu.dudau@foss.arm.com>
 *
 */
#define DEBUG
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <errno.h>
#include <misc.h>
#include <linux/bitops.h>

#define CLK_FUNCTION		BIT(20)

struct vexpress_osc_clk_priv {
	u8 osc;
	ulong rate_min;
	ulong rate_max;
};

static ulong vexpress_osc_clk_get_rate(struct clk *clk)
{
	int err;
	u32 data;
	struct udevice *vexpress_cfg = dev_get_parent(clk->dev);
	struct vexpress_osc_clk_priv *priv = dev_get_priv(clk->dev);

	data = CLK_FUNCTION | priv->osc;
	err = misc_read(vexpress_cfg, 0, &data, sizeof(data));
	if (err < 0)
		return err;

	return data;
}

#ifndef CONFIG_SPL_BUILD
static ulong vexpress_osc_clk_set_rate(struct clk *clk, ulong rate)
{
	int err;
	u32 buffer[2];
	struct udevice *vexpress_cfg = dev_get_parent(clk->dev);
	struct vexpress_osc_clk_priv *priv = dev_get_priv(clk->dev);

	if (rate < priv->rate_min || rate > priv->rate_max)
		return -EINVAL;

	/*
	 * we are sending the parent the info about the oscillator
	 * and the value we want to set
	 */
	buffer[0] = CLK_FUNCTION | priv->osc;
	buffer[1] = rate;
	err = misc_write(vexpress_cfg, 0, buffer, 2 * sizeof(u32));
	if (err < 0)
		return err;

	return rate;
}
#endif

static struct clk_ops vexpress_osc_clk_ops = {
	.get_rate = vexpress_osc_clk_get_rate,
#ifndef CONFIG_SPL_BUILD
	.set_rate = vexpress_osc_clk_set_rate,
#endif
};

static int vexpress_osc_clk_probe(struct udevice *dev)
{
	struct vexpress_osc_clk_priv *priv = dev_get_priv(dev);
	u32 values[2];
	int err;

	err = dev_read_u32_array(dev, "freq-range", values, 2);
	if (err)
		return err;
	priv->rate_min = values[0];
	priv->rate_max = values[1];

	err = dev_read_u32_array(dev, "arm,vexpress-sysreg,func", values, 2);
	if (err)
		return err;

	if (values[0] != 1) {
		dev_err(dev, "Invalid VExpress function for clock, must be '1'");
		return -EINVAL;
	}
	priv->osc = values[1];
	debug("clk \"%s%d\", min freq %luHz, max freq %luHz\n", dev->name,
	      priv->osc, priv->rate_min, priv->rate_max);

	return 0;
}

static const struct udevice_id vexpress_osc_clk_ids[] = {
	{ .compatible = "arm,vexpress-osc", },
	{}
};

U_BOOT_DRIVER(vexpress_osc_clk) = {
	.name = "vexpress_osc_clk",
	.id = UCLASS_CLK,
	.of_match = vexpress_osc_clk_ids,
	.ops = &vexpress_osc_clk_ops,
	.priv_auto	= sizeof(struct vexpress_osc_clk_priv),
	.probe = vexpress_osc_clk_probe,
};
