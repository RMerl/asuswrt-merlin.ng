// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/io.h>

/* control register */
#define ALTERA_TIMER_CONT	BIT(1)	/* Continuous mode */
#define ALTERA_TIMER_START	BIT(2)	/* Start timer */
#define ALTERA_TIMER_STOP	BIT(3)	/* Stop timer */

struct altera_timer_regs {
	u32	status;		/* Timer status reg */
	u32	control;	/* Timer control reg */
	u32	periodl;	/* Timeout period low */
	u32	periodh;	/* Timeout period high */
	u32	snapl;		/* Snapshot low */
	u32	snaph;		/* Snapshot high */
};

struct altera_timer_platdata {
	struct altera_timer_regs *regs;
};

static int altera_timer_get_count(struct udevice *dev, u64 *count)
{
	struct altera_timer_platdata *plat = dev->platdata;
	struct altera_timer_regs *const regs = plat->regs;
	u32 val;

	/* Trigger update */
	writel(0x0, &regs->snapl);

	/* Read timer value */
	val = readl(&regs->snapl) & 0xffff;
	val |= (readl(&regs->snaph) & 0xffff) << 16;
	*count = timer_conv_64(~val);

	return 0;
}

static int altera_timer_probe(struct udevice *dev)
{
	struct altera_timer_platdata *plat = dev->platdata;
	struct altera_timer_regs *const regs = plat->regs;

	writel(0, &regs->status);
	writel(0, &regs->control);
	writel(ALTERA_TIMER_STOP, &regs->control);

	writel(0xffff, &regs->periodl);
	writel(0xffff, &regs->periodh);
	writel(ALTERA_TIMER_CONT | ALTERA_TIMER_START, &regs->control);

	return 0;
}

static int altera_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct altera_timer_platdata *plat = dev_get_platdata(dev);

	plat->regs = map_physmem(devfdt_get_addr(dev),
				 sizeof(struct altera_timer_regs),
				 MAP_NOCACHE);

	return 0;
}

static const struct timer_ops altera_timer_ops = {
	.get_count = altera_timer_get_count,
};

static const struct udevice_id altera_timer_ids[] = {
	{ .compatible = "altr,timer-1.0" },
	{}
};

U_BOOT_DRIVER(altera_timer) = {
	.name	= "altera_timer",
	.id	= UCLASS_TIMER,
	.of_match = altera_timer_ids,
	.ofdata_to_platdata = altera_timer_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct altera_timer_platdata),
	.probe = altera_timer_probe,
	.ops	= &altera_timer_ops,
};
