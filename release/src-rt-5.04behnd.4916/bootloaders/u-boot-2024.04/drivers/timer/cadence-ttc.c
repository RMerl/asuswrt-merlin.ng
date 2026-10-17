// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Xilinx, Inc. (Michal Simek)
 */

#include <common.h>
#include <bootstage.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <timer.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/err.h>

#define CNT_CNTRL_RESET		BIT(4)

struct cadence_ttc_regs {
	u32 clk_cntrl1; /* 0x0 - Clock Control 1 */
	u32 clk_cntrl2; /* 0x4 - Clock Control 2 */
	u32 clk_cntrl3; /* 0x8 - Clock Control 3 */
	u32 counter_cntrl1; /* 0xC - Counter Control 1 */
	u32 counter_cntrl2; /* 0x10 - Counter Control 2 */
	u32 counter_cntrl3; /* 0x14 - Counter Control 3 */
	u32 counter_val1; /* 0x18 - Counter Control 1 */
	u32 counter_val2; /* 0x1C - Counter Control 2 */
	u32 counter_val3; /* 0x20 - Counter Control 3 */
	u32 reserved[15];
	u32 interrupt_enable1; /* 0x60 - Interrupt Enable 1 */
	u32 interrupt_enable2; /* 0x64 - Interrupt Enable 2 */
	u32 interrupt_enable3; /* 0x68 - Interrupt Enable 3 */
};

struct cadence_ttc_priv {
	struct cadence_ttc_regs *regs;
};

#if CONFIG_IS_ENABLED(BOOTSTAGE)
ulong timer_get_boot_us(void)
{
	u64 ticks = 0;
	u32 rate = 1;
	u64 us;
	int ret;

	ret = dm_timer_init();
	if (!ret) {
		/* The timer is available */
		rate = timer_get_rate(gd->timer);
		timer_get_count(gd->timer, &ticks);
	} else {
		return 0;
	}

	us = (ticks * 1000) / rate;
	return us;
}
#endif

static u64 cadence_ttc_get_count(struct udevice *dev)
{
	struct cadence_ttc_priv *priv = dev_get_priv(dev);

	return readl(&priv->regs->counter_val1);
}

static int cadence_ttc_probe(struct udevice *dev)
{
	struct cadence_ttc_priv *priv = dev_get_priv(dev);

	/* Disable interrupts for sure */
	writel(0, &priv->regs->interrupt_enable1);
	writel(0, &priv->regs->interrupt_enable2);
	writel(0, &priv->regs->interrupt_enable3);

	/* Make sure that clocks are configured properly without prescaller */
	writel(0, &priv->regs->clk_cntrl1);
	writel(0, &priv->regs->clk_cntrl2);
	writel(0, &priv->regs->clk_cntrl3);

	/* Reset and enable this counter */
	writel(CNT_CNTRL_RESET, &priv->regs->counter_cntrl1);

	return 0;
}

static int cadence_ttc_of_to_plat(struct udevice *dev)
{
	struct cadence_ttc_priv *priv = dev_get_priv(dev);

	priv->regs = map_physmem(dev_read_addr(dev),
				 sizeof(struct cadence_ttc_regs), MAP_NOCACHE);
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	return 0;
}

static int cadence_ttc_bind(struct udevice *dev)
{
	const char *cells;

	cells = dev_read_prop(dev, "#pwm-cells", NULL);
	if (cells)
		return -ENODEV;

	return 0;
}

static const struct timer_ops cadence_ttc_ops = {
	.get_count = cadence_ttc_get_count,
};

static const struct udevice_id cadence_ttc_ids[] = {
	{ .compatible = "cdns,ttc" },
	{}
};

U_BOOT_DRIVER(cadence_ttc) = {
	.name = "cadence_ttc",
	.id = UCLASS_TIMER,
	.of_match = cadence_ttc_ids,
	.of_to_plat = cadence_ttc_of_to_plat,
	.priv_auto	= sizeof(struct cadence_ttc_priv),
	.probe = cadence_ttc_probe,
	.ops = &cadence_ttc_ops,
	.bind = cadence_ttc_bind,
};
