// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Rick Chen <rick@andestech.com>
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 *
 * U-Boot syscon driver for Andes's Platform Level Machine Timer (PLMT).
 * The PLMT block holds memory-mapped mtime register
 * associated with timer tick.
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <linux/err.h>

/* mtime register */
#define MTIME_REG(base)			((ulong)(base))

static u64 notrace andes_plmt_get_count(struct udevice *dev)
{
	return readq((void __iomem *)MTIME_REG(dev_get_priv(dev)));
}

#if CONFIG_IS_ENABLED(RISCV_MMODE) && IS_ENABLED(CONFIG_TIMER_EARLY)
/**
 * timer_early_get_rate() - Get the timer rate before driver model
 */
unsigned long notrace timer_early_get_rate(void)
{
	return RISCV_MMODE_TIMER_FREQ;
}

/**
 * timer_early_get_count() - Get the timer count before driver model
 *
 */
u64 notrace timer_early_get_count(void)
{
	return readq((void __iomem *)MTIME_REG(RISCV_MMODE_TIMERBASE));
}
#endif

static const struct timer_ops andes_plmt_ops = {
	.get_count = andes_plmt_get_count,
};

static int andes_plmt_probe(struct udevice *dev)
{
	dev_set_priv(dev, dev_read_addr_ptr(dev));
	if (!dev_get_priv(dev))
		return -EINVAL;

	return timer_timebase_fallback(dev);
}

static const struct udevice_id andes_plmt_ids[] = {
	{ .compatible = "andestech,plmt0" },
	{ }
};

U_BOOT_DRIVER(andes_plmt) = {
	.name		= "andes_plmt",
	.id		= UCLASS_TIMER,
	.of_match	= andes_plmt_ids,
	.ops		= &andes_plmt_ops,
	.probe		= andes_plmt_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
