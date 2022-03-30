// SPDX-License-Identifier: GPL-2.0+
/*
 * Andestech ATFTMR010 timer driver
 *
 * (C) Copyright 2016
 * Rick Chen, NDS32 Software Engineering, rick@andestech.com
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <linux/io.h>

/*
 * Timer Control Register
 */
#define T3_UPDOWN	(1 << 11)
#define T2_UPDOWN	(1 << 10)
#define T1_UPDOWN	(1 << 9)
#define T3_OFENABLE	(1 << 8)
#define T3_CLOCK	(1 << 7)
#define T3_ENABLE	(1 << 6)
#define T2_OFENABLE	(1 << 5)
#define T2_CLOCK	(1 << 4)
#define T2_ENABLE	(1 << 3)
#define T1_OFENABLE	(1 << 2)
#define T1_CLOCK	(1 << 1)
#define T1_ENABLE	(1 << 0)

/*
 * Timer Interrupt State & Mask Registers
 */
#define T3_OVERFLOW	(1 << 8)
#define T3_MATCH2	(1 << 7)
#define T3_MATCH1	(1 << 6)
#define T2_OVERFLOW	(1 << 5)
#define T2_MATCH2	(1 << 4)
#define T2_MATCH1	(1 << 3)
#define T1_OVERFLOW	(1 << 2)
#define T1_MATCH2	(1 << 1)
#define T1_MATCH1	(1 << 0)

struct atftmr_timer_regs {
	u32	t1_counter;		/* 0x00 */
	u32	t1_load;		/* 0x04 */
	u32	t1_match1;		/* 0x08 */
	u32	t1_match2;		/* 0x0c */
	u32	t2_counter;		/* 0x10 */
	u32	t2_load;		/* 0x14 */
	u32	t2_match1;		/* 0x18 */
	u32	t2_match2;		/* 0x1c */
	u32	t3_counter;		/* 0x20 */
	u32	t3_load;		/* 0x24 */
	u32	t3_match1;		/* 0x28 */
	u32	t3_match2;		/* 0x2c */
	u32	cr;			/* 0x30 */
	u32	int_state;		/* 0x34 */
	u32	int_mask;		/* 0x38 */
};

struct atftmr_timer_platdata {
	struct atftmr_timer_regs *regs;
};

static int atftmr_timer_get_count(struct udevice *dev, u64 *count)
{
	struct atftmr_timer_platdata *plat = dev->platdata;
	struct atftmr_timer_regs *const regs = plat->regs;
	u32 val;
	val = readl(&regs->t3_counter);
	*count = timer_conv_64(val);
	return 0;
}

static int atftmr_timer_probe(struct udevice *dev)
{
	struct atftmr_timer_platdata *plat = dev->platdata;
	struct atftmr_timer_regs *const regs = plat->regs;
	u32 cr;
	writel(0, &regs->t3_load);
	writel(0, &regs->t3_counter);
	writel(TIMER_LOAD_VAL, &regs->t3_match1);
	writel(TIMER_LOAD_VAL, &regs->t3_match2);
	/* disable interrupts */
	writel(T3_MATCH1|T3_MATCH2|T3_OVERFLOW , &regs->int_mask);
	cr = readl(&regs->cr);
	cr |= (T3_ENABLE|T3_UPDOWN);
	writel(cr, &regs->cr);
	return 0;
}

static int atftme_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct atftmr_timer_platdata *plat = dev_get_platdata(dev);
	plat->regs = map_physmem(devfdt_get_addr(dev),
				 sizeof(struct atftmr_timer_regs),
				 MAP_NOCACHE);
	return 0;
}

static const struct timer_ops ag101p_timer_ops = {
	.get_count = atftmr_timer_get_count,
};

static const struct udevice_id ag101p_timer_ids[] = {
	{ .compatible = "andestech,attmr010" },
	{}
};

U_BOOT_DRIVER(altera_timer) = {
	.name	= "ag101p_timer",
	.id	= UCLASS_TIMER,
	.of_match = ag101p_timer_ids,
	.ofdata_to_platdata = atftme_timer_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct atftmr_timer_platdata),
	.probe = atftmr_timer_probe,
	.ops	= &ag101p_timer_ops,
};
