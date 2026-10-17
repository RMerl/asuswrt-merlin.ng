// SPDX-License-Identifier: GPL-2.0
/*
 * GXP timer driver
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 */

#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>

#define USTIMELO	0x18
#define USTIMEHI	0x1C

struct gxp_timer_priv {
	void __iomem *base;
};

static u64 gxp_timer_get_count(struct udevice *dev)
{
	struct gxp_timer_priv *priv = dev_get_priv(dev);
	u64 val;

	val = readl(priv->base + USTIMEHI);
	val = (val << 32) | readl(priv->base + USTIMELO);

	return val;
}

static int gxp_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct gxp_timer_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	uc_priv->clock_rate = 1000000;

	return 0;
}

static const struct timer_ops gxp_timer_ops = {
	.get_count = gxp_timer_get_count,
};

static const struct udevice_id gxp_timer_ids[] = {
	{ .compatible = "hpe,gxp-timer" },
	{}
};

U_BOOT_DRIVER(gxp_timer) = {
	.name = "gxp-timer",
	.id = UCLASS_TIMER,
	.of_match = gxp_timer_ids,
	.priv_auto = sizeof(struct gxp_timer_priv),
	.probe = gxp_timer_probe,
	.ops = &gxp_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
