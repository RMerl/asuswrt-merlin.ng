// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2018, Anup Patel <anup@brainfault.org>
 * Copyright (C) 2012 Regents of the University of California
 *
 * RISC-V architecturally-defined generic timer driver
 *
 * This driver provides generic timer support for S-mode U-Boot.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <timer.h>
#include <asm/csr.h>

static u64 notrace riscv_timer_get_count(struct udevice *dev)
{
	__maybe_unused u32 hi, lo;

	if (IS_ENABLED(CONFIG_64BIT))
		return csr_read(CSR_TIME);

	do {
		hi = csr_read(CSR_TIMEH);
		lo = csr_read(CSR_TIME);
	} while (hi != csr_read(CSR_TIMEH));

	return ((u64)hi << 32) | lo;
}

#if CONFIG_IS_ENABLED(RISCV_SMODE) && IS_ENABLED(CONFIG_TIMER_EARLY)
/**
 * timer_early_get_rate() - Get the timer rate before driver model
 */
unsigned long notrace timer_early_get_rate(void)
{
	return RISCV_SMODE_TIMER_FREQ;
}

/**
 * timer_early_get_count() - Get the timer count before driver model
 *
 */
u64 notrace timer_early_get_count(void)
{
	return riscv_timer_get_count(NULL);
}
#endif

#if CONFIG_IS_ENABLED(RISCV_SMODE) && CONFIG_IS_ENABLED(BOOTSTAGE)
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
		rate = RISCV_SMODE_TIMER_FREQ;
		ticks = riscv_timer_get_count(NULL);
	}

	/* Below is converted from time(us) = (tick / rate) * 10000000 */
	return lldiv(ticks * 1000, (rate / 1000));
}
#endif

static int riscv_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	u32 rate;

	/*  When this function was called from the CPU driver, clock
	 *  frequency is passed as driver data.
	 */
	rate = dev->driver_data;

	/* When called from an FDT match, the rate needs to be looked up. */
	if (!rate && gd->fdt_blob) {
		rate = fdt_getprop_u32_default(gd->fdt_blob,
					       "/cpus", "timebase-frequency", 0);
	}

	uc_priv->clock_rate = rate;

	/* With rate==0, timer uclass post_probe might later fail with -EINVAL.
	 * Give a hint at the cause for debugging.
	 */
	if (!rate)
		log_err("riscv_timer_probe with invalid clock rate 0!\n");

	return 0;
}

static const struct timer_ops riscv_timer_ops = {
	.get_count = riscv_timer_get_count,
};

static const struct udevice_id riscv_timer_ids[] = {
	{ .compatible = "riscv,timer", },
	{ }
};

U_BOOT_DRIVER(riscv_timer) = {
	.name = "riscv_timer",
	.id = UCLASS_TIMER,
	.of_match = of_match_ptr(riscv_timer_ids),
	.probe = riscv_timer_probe,
	.ops = &riscv_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
