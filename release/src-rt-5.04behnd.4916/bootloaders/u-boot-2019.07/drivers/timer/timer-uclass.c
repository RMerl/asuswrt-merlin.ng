// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <clk.h>
#include <errno.h>
#include <timer.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Implement a timer uclass to work with lib/time.c. The timer is usually
 * a 32/64 bits free-running up counter. The get_rate() method is used to get
 * the input clock frequency of the timer. The get_count() method is used
 * to get the current 64 bits count value. If the hardware is counting down,
 * the value should be inversed inside the method. There may be no real
 * tick, and no timer interrupt.
 */

int notrace timer_get_count(struct udevice *dev, u64 *count)
{
	const struct timer_ops *ops = device_get_ops(dev);

	if (!ops->get_count)
		return -ENOSYS;

	return ops->get_count(dev, count);
}

unsigned long notrace timer_get_rate(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev->uclass_priv;

	return uc_priv->clock_rate;
}

static int timer_pre_probe(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk timer_clk;
	int err;
	ulong ret;

	err = clk_get_by_index(dev, 0, &timer_clk);
	if (!err) {
		ret = clk_get_rate(&timer_clk);
		if (IS_ERR_VALUE(ret))
			return ret;
		uc_priv->clock_rate = ret;
	} else {
		uc_priv->clock_rate =
			dev_read_u32_default(dev, "clock-frequency", 0);
	}
#endif

	return 0;
}

static int timer_post_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!uc_priv->clock_rate)
		return -EINVAL;

	return 0;
}

u64 timer_conv_64(u32 count)
{
	/* increment tbh if tbl has rolled over */
	if (count < gd->timebase_l)
		gd->timebase_h++;
	gd->timebase_l = count;
	return ((u64)gd->timebase_h << 32) | gd->timebase_l;
}

int notrace dm_timer_init(void)
{
	struct udevice *dev = NULL;
	__maybe_unused ofnode node;
	int ret;

	if (gd->timer)
		return 0;

	/*
	 * Directly access gd->dm_root to suppress error messages, if the
	 * virtual root driver does not yet exist.
	 */
	if (gd->dm_root == NULL)
		return -EAGAIN;

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	/* Check for a chosen timer to be used for tick */
	node = ofnode_get_chosen_node("tick-timer");

	if (ofnode_valid(node) &&
	    uclass_get_device_by_ofnode(UCLASS_TIMER, node, &dev)) {
		/*
		 * If the timer is not marked to be bound before
		 * relocation, bind it anyway.
		 */
		if (!lists_bind_fdt(dm_root(), node, &dev, false)) {
			ret = device_probe(dev);
			if (ret)
				return ret;
		}
	}
#endif

	if (!dev) {
		/* Fall back to the first available timer */
		ret = uclass_first_device_err(UCLASS_TIMER, &dev);
		if (ret)
			return ret;
	}

	if (dev) {
		gd->timer = dev;
		return 0;
	}

	return -ENODEV;
}

UCLASS_DRIVER(timer) = {
	.id		= UCLASS_TIMER,
	.name		= "timer",
	.pre_probe	= timer_pre_probe,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_probe	= timer_post_probe,
	.per_device_auto_alloc_size = sizeof(struct timer_dev_priv),
};
