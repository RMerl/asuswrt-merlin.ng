// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 *
 * ARM Cortext A9 global timer driver
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <timer.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/arch-armv7/globaltimer.h>

struct arm_global_timer_priv {
	struct globaltimer *global_timer;
};

static u64 arm_global_timer_get_count(struct udevice *dev)
{
	struct arm_global_timer_priv *priv = dev_get_priv(dev);
	struct globaltimer *global_timer = priv->global_timer;
	u32 low, high;
	u64 timer;
	u32 old = readl(&global_timer->cnt_h);

	while (1) {
		low = readl(&global_timer->cnt_l);
		high = readl(&global_timer->cnt_h);
		if (old == high)
			break;
		else
			old = high;
	}
	timer = high;
	return (u64)((timer << 32) | low);
}

static int arm_global_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct arm_global_timer_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int err;
	ulong ret;

	/* get arm global timer base address */
	priv->global_timer = (struct globaltimer *)dev_read_addr_ptr(dev);
	if (!priv->global_timer)
		return -ENOENT;

	err = clk_get_by_index(dev, 0, &clk);
	if (!err) {
		ret = clk_get_rate(&clk);
		if (IS_ERR_VALUE(ret))
			return ret;
		uc_priv->clock_rate = ret;
	} else {
		uc_priv->clock_rate = CFG_SYS_HZ_CLOCK;
	}

	/* init timer */
	writel(0x01, &priv->global_timer->ctl);

	return 0;
}

static const struct timer_ops arm_global_timer_ops = {
	.get_count = arm_global_timer_get_count,
};

static const struct udevice_id arm_global_timer_ids[] = {
	{ .compatible = "arm,cortex-a9-global-timer" },
	{}
};

U_BOOT_DRIVER(arm_global_timer) = {
	.name = "arm_global_timer",
	.id = UCLASS_TIMER,
	.of_match = arm_global_timer_ids,
	.priv_auto	= sizeof(struct arm_global_timer_priv),
	.probe = arm_global_timer_probe,
	.ops = &arm_global_timer_ops,
};
