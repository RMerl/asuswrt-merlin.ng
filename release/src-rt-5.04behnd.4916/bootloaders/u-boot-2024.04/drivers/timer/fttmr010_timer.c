// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * 23/08/2022 Port to DM
 */
#include <common.h>
#include <dm.h>
#include <log.h>
#include <timer.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <faraday/fttmr010.h>
#include <asm/global_data.h>

#define TIMER_LOAD_VAL	0xffffffff

struct fttmr010_timer_priv {
	struct fttmr010 __iomem *regs;
};

static u64 fttmr010_timer_get_count(struct udevice *dev)
{
	struct fttmr010_timer_priv *priv = dev_get_priv(dev);
	struct fttmr010 *tmr = priv->regs;
	u32 now = TIMER_LOAD_VAL - readl(&tmr->timer3_counter);

	/* increment tbu if tbl has rolled over */
	if (now < gd->arch.tbl)
		gd->arch.tbu++;
	gd->arch.tbl = now;

	return ((u64)gd->arch.tbu << 32) | gd->arch.tbl;
}

static int fttmr010_timer_probe(struct udevice *dev)
{
	struct fttmr010_timer_priv *priv = dev_get_priv(dev);
	struct fttmr010 *tmr;
	unsigned int cr;

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;
	tmr = priv->regs;

	debug("Faraday FTTMR010 timer revision 0x%08X\n", readl(&tmr->revision));

	/* disable timers */
	writel(0, &tmr->cr);

	/* setup timer */
	writel(TIMER_LOAD_VAL, &tmr->timer3_load);
	writel(TIMER_LOAD_VAL, &tmr->timer3_counter);
	writel(0, &tmr->timer3_match1);
	writel(0, &tmr->timer3_match2);

	/* we don't want timer to issue interrupts */
	writel(FTTMR010_TM3_MATCH1 |
	       FTTMR010_TM3_MATCH2 |
	       FTTMR010_TM3_OVERFLOW,
	       &tmr->interrupt_mask);

	cr = readl(&tmr->cr);
	cr |= FTTMR010_TM3_CLOCK;	/* use external clock */
	cr |= FTTMR010_TM3_ENABLE;
	writel(cr, &tmr->cr);

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}

static const struct timer_ops fttmr010_timer_ops = {
	.get_count = fttmr010_timer_get_count,
};

static const struct udevice_id fttmr010_timer_ids[] = {
	{ .compatible = "faraday,fttmr010-timer" },
	{}
};

U_BOOT_DRIVER(fttmr010_timer) = {
	.name = "fttmr010_timer",
	.id = UCLASS_TIMER,
	.of_match = fttmr010_timer_ids,
	.priv_auto = sizeof(struct fttmr010_timer_priv),
	.probe = fttmr010_timer_probe,
	.ops = &fttmr010_timer_ops,
};
