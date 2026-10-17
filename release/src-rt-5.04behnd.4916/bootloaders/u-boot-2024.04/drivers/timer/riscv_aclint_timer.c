// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <linux/err.h>

#define CLINT_MTIME_OFFSET		0xbff8
#define ACLINT_MTIME_OFFSET		0

/* mtime register */
#define MTIME_REG(base, offset)		((ulong)(base) + (offset))

static u64 notrace riscv_aclint_timer_get_count(struct udevice *dev)
{
	return readq((void __iomem *)MTIME_REG(dev_get_priv(dev),
					       dev_get_driver_data(dev)));
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
	return readq((void __iomem *)MTIME_REG(RISCV_MMODE_TIMERBASE,
					       RISCV_MMODE_TIMEROFF));
}
#endif

#if CONFIG_IS_ENABLED(RISCV_MMODE) && CONFIG_IS_ENABLED(BOOTSTAGE)
ulong timer_get_boot_us(void)
{
	int ret;
	u64 ticks = 0;
	u32 rate;

	ret = dm_timer_init();
	if (!ret) {
		rate = timer_get_rate(gd->timer);
		timer_get_count(gd->timer, &ticks);
	} else {
		rate = RISCV_MMODE_TIMER_FREQ;
		ticks = readq((void __iomem *)MTIME_REG(RISCV_MMODE_TIMERBASE,
							RISCV_MMODE_TIMEROFF));
	}

	/* Below is converted from time(us) = (tick / rate) * 10000000 */
	return lldiv(ticks * 1000, (rate / 1000));
}
#endif

static const struct timer_ops riscv_aclint_timer_ops = {
	.get_count = riscv_aclint_timer_get_count,
};

static int riscv_aclint_timer_probe(struct udevice *dev)
{
	dev_set_priv(dev, dev_read_addr_ptr(dev));
	if (!dev_get_priv(dev))
		return -EINVAL;

	return timer_timebase_fallback(dev);
}

static const struct udevice_id riscv_aclint_timer_ids[] = {
	{ .compatible = "riscv,clint0", .data = CLINT_MTIME_OFFSET },
	{ .compatible = "sifive,clint0", .data = CLINT_MTIME_OFFSET },
	{ .compatible = "riscv,aclint-mtimer", .data = ACLINT_MTIME_OFFSET },
	{ }
};

U_BOOT_DRIVER(riscv_aclint_timer) = {
	.name		= "riscv_aclint_timer",
	.id		= UCLASS_TIMER,
	.of_match	= riscv_aclint_timer_ids,
	.probe		= riscv_aclint_timer_probe,
	.ops		= &riscv_aclint_timer_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
