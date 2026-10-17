// SPDX-License-Identifier: GPL-2.0+
/*
 * ARM PrimeCell Dual-Timer Module (SP804) driver
 * Copyright (C) 2022 Arm Ltd.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/ofnode.h>
#include <mapmem.h>
#include <dt-structs.h>
#include <timer.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define SP804_TIMERX_LOAD		0x00
#define SP804_TIMERX_VALUE		0x04
#define SP804_TIMERX_CONTROL		0x08

#define SP804_CTRL_TIMER_ENABLE		(1U << 7)
#define SP804_CTRL_TIMER_PERIODIC	(1U << 6)
#define SP804_CTRL_INT_ENABLE		(1U << 5)
#define SP804_CTRL_TIMER_PRESCALE_SHIFT	2
#define SP804_CTRL_TIMER_PRESCALE_MASK	(3U << SP804_CTRL_TIMER_PRESCALE_SHIFT)
#define SP804_CTRL_TIMER_32BIT		(1U << 1)
#define SP804_CTRL_ONESHOT		(1U << 0)


struct sp804_timer_plat {
	uintptr_t base;
};

static u64 sp804_timer_get_count(struct udevice *dev)
{
	struct sp804_timer_plat *plat = dev_get_plat(dev);
	uint32_t cntr = readl(plat->base + SP804_TIMERX_VALUE);

	/* timers are down-counting */
	return ~0u - cntr;
}

static int sp804_clk_of_to_plat(struct udevice *dev)
{
	struct sp804_timer_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr(dev);
	if (!plat->base)
		return -ENOENT;

	return 0;
}

static int sp804_timer_probe(struct udevice *dev)
{
	struct sp804_timer_plat *plat = dev_get_plat(dev);
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk base_clk;
	unsigned int divider = 1;
	uint32_t ctlr;
	int ret;

	ctlr = readl(plat->base + SP804_TIMERX_CONTROL);
	ctlr &= SP804_CTRL_TIMER_PRESCALE_MASK;
	switch (ctlr >> SP804_CTRL_TIMER_PRESCALE_SHIFT) {
	case 0x0: divider = 1; break;
	case 0x1: divider = 16; break;
	case 0x2: divider = 256; break;
	case 0x3: printf("illegal!\n"); break;
	}

	ret = clk_get_by_index(dev, 0, &base_clk);
	if (ret) {
		printf("could not find SP804 timer base clock in DT\n");
		return ret;
	}

	uc_priv->clock_rate = clk_get_rate(&base_clk) / divider;

	/* keep divider, free-running, wrapping, no IRQs, 32-bit mode */
	ctlr |= SP804_CTRL_TIMER_32BIT | SP804_CTRL_TIMER_ENABLE;
	writel(ctlr, plat->base + SP804_TIMERX_CONTROL);

	return 0;
}

static const struct timer_ops sp804_timer_ops = {
	.get_count = sp804_timer_get_count,
};

static const struct udevice_id sp804_timer_ids[] = {
	{ .compatible = "arm,sp804" },
	{}
};

U_BOOT_DRIVER(arm_sp804_timer) = {
	.name		= "arm_sp804_timer",
	.id		= UCLASS_TIMER,
	.of_match 	= sp804_timer_ids,
	.probe		= sp804_timer_probe,
	.ops		= &sp804_timer_ops,
	.plat_auto	= sizeof(struct sp804_timer_plat),
	.of_to_plat 	= sp804_clk_of_to_plat,
};
