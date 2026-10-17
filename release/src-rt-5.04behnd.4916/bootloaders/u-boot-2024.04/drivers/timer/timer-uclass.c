// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#define LOG_CATEGORY UCLASS_TIMER

#include <common.h>
#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/lists.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <errno.h>
#include <init.h>
#include <timer.h>
#include <linux/err.h>
#include <relocate.h>

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
	struct timer_ops *ops = timer_get_ops(dev);

	if (!ops->get_count)
		return -ENOSYS;

	*count = ops->get_count(dev);
	return 0;
}

unsigned long notrace timer_get_rate(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	return uc_priv->clock_rate;
}

static int timer_pre_probe(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
		struct clk timer_clk;
		int err;
		ulong ret;

		/*
		 * It is possible that a timer device has a null ofnode
		 */
		if (!dev_has_ofnode(dev))
			return 0;

		err = clk_get_by_index(dev, 0, &timer_clk);
		if (!err) {
			ret = clk_get_rate(&timer_clk);
			if (!IS_ERR_VALUE(ret)) {
				uc_priv->clock_rate = ret;
				return 0;
			}
		}

		uc_priv->clock_rate = dev_read_u32_default(dev, "clock-frequency", 0);
	}

	return 0;
}

static int timer_post_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!uc_priv->clock_rate)
		return -EINVAL;

	return 0;
}

#if CONFIG_IS_ENABLED(CPU)
int timer_timebase_fallback(struct udevice *dev)
{
	struct udevice *cpu;
	struct cpu_plat *cpu_plat;
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Did we get our clock rate from the device tree? */
	if (uc_priv->clock_rate)
		return 0;

	/* Fall back to timebase-frequency */
	dev_dbg(dev, "missing clocks or clock-frequency property; falling back on timebase-frequency\n");
	cpu = cpu_get_current_dev();
	if (!cpu)
		return -ENODEV;

	cpu_plat = dev_get_parent_plat(cpu);
	if (!cpu_plat)
		return -ENODEV;

	uc_priv->clock_rate = cpu_plat->timebase_freq;
	return 0;
}
#endif

u64 timer_conv_64(u32 count)
{
	/* increment tbh if tbl has rolled over */
	if (count < gd->timebase_l)
		gd->timebase_h++;
	gd->timebase_l = count;
	return ((u64)gd->timebase_h << 32) | gd->timebase_l;
}

int dm_timer_init(void)
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

	if (CONFIG_IS_ENABLED(OF_REAL)) {
		/* Check for a chosen timer to be used for tick */
		node = ofnode_get_chosen_node("tick-timer");

		if (ofnode_valid(node) &&
		    uclass_get_device_by_ofnode(UCLASS_TIMER, node, &dev)) {
			/*
			 * If the timer is not marked to be bound before
			 * relocation, bind it anyway.
			 */
			if (!lists_bind_fdt(dm_root(), node, &dev, NULL, false)) {
				ret = device_probe(dev);
				if (ret)
					return ret;
			}
		}
	}

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
	.per_device_auto	= sizeof(struct timer_dev_priv),
};
