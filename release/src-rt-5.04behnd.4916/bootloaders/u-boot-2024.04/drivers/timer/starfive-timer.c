// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 StarFive, Inc. All rights reserved.
 *   Author: Kuan Lim Lee <kuanlim.lee@starfivetech.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <time.h>
#include <timer.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <linux/err.h>

#define	STF_TIMER_INT_STATUS	0x00
#define STF_TIMER_CTL		0x04
#define STF_TIMER_LOAD		0x08
#define STF_TIMER_ENABLE	0x10
#define STF_TIMER_RELOAD	0x14
#define STF_TIMER_VALUE		0x18
#define STF_TIMER_INT_CLR	0x20
#define STF_TIMER_INT_MASK	0x24

struct starfive_timer_priv {
	void __iomem *base;
	u32 timer_size;
};

static u64 notrace starfive_get_count(struct udevice *dev)
{
	struct starfive_timer_priv *priv = dev_get_priv(dev);

	/* Read decrement timer value and convert to increment value */
	return priv->timer_size - readl(priv->base + STF_TIMER_VALUE);
}

static const struct timer_ops starfive_ops = {
	.get_count = starfive_get_count,
};

static int starfive_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct starfive_timer_priv *priv = dev_get_priv(dev);
	int timer_channel;
	struct clk clk;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	timer_channel = dev_read_u32_default(dev, "channel", 0);
	priv->base = priv->base + (0x40 * timer_channel);

	/* Get clock rate from channel selectecd*/
	ret = clk_get_by_index(dev, timer_channel, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;
	uc_priv->clock_rate = clk_get_rate(&clk);

	/*
	 * Initiate timer, channel 0
	 * Unmask Interrupt Mask
	 */
	writel(0, priv->base + STF_TIMER_INT_MASK);
	/* Single run mode Setting */
	if (dev_read_bool(dev, "single-run"))
		writel(1, priv->base + STF_TIMER_CTL);
	/* Set Reload value */
	priv->timer_size = dev_read_u32_default(dev, "timer-size", -1U);
	writel(priv->timer_size, priv->base + STF_TIMER_LOAD);
	/* Enable to start timer */
	writel(1, priv->base + STF_TIMER_ENABLE);

	return 0;
}

static const struct udevice_id starfive_ids[] = {
	{ .compatible = "starfive,jh8100-timers" },
	{ }
};

U_BOOT_DRIVER(jh8100_starfive_timer) = {
	.name		= "starfive_timer",
	.id		= UCLASS_TIMER,
	.of_match	= starfive_ids,
	.probe		= starfive_probe,
	.ops		= &starfive_ops,
	.priv_auto	= sizeof(struct starfive_timer_priv),
};
