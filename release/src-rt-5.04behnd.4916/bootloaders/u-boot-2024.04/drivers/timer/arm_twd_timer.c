// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2022 Weidmüller Interface GmbH & Co. KG
 * Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
 *
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2011-2017 Xilinx, Inc. All rights reserved.
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <timer.h>
#include <linux/bitops.h>

#include <asm/io.h>

#define SCUTIMER_CONTROL_PRESCALER_MASK		0x0000FF00 /* Prescaler */
#define SCUTIMER_CONTROL_AUTO_RELOAD_MASK	0x00000002 /* Auto-reload */
#define SCUTIMER_CONTROL_ENABLE_MASK		0x00000001 /* Timer enable */

#define TIMER_LOAD_VAL 0xFFFFFFFF

struct arm_twd_timer_regs {
	u32 load; /* Timer Load Register */
	u32 counter; /* Timer Counter Register */
	u32 control; /* Timer Control Register */
};

struct arm_twd_timer_priv {
	struct arm_twd_timer_regs *base;
};

static u64 arm_twd_timer_get_count(struct udevice *dev)
{
	struct arm_twd_timer_priv *priv = dev_get_priv(dev);
	struct arm_twd_timer_regs *regs = priv->base;
	u32 count = TIMER_LOAD_VAL - readl(&regs->counter);

	return timer_conv_64(count);
}

static int arm_twd_timer_probe(struct udevice *dev)
{
	struct arm_twd_timer_priv *priv = dev_get_priv(dev);
	struct arm_twd_timer_regs *regs;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct arm_twd_timer_regs *)addr;

	regs = priv->base;

	/* Load the timer counter register */
	writel(0xFFFFFFFF, &regs->load);

	/*
	 * Start the A9Timer device
	 * Enable Auto reload mode, Clear prescaler control bits
	 * Set prescaler value, Enable the decrementer
	 */
	clrsetbits_le32(&regs->control, SCUTIMER_CONTROL_PRESCALER_MASK,
			SCUTIMER_CONTROL_AUTO_RELOAD_MASK |
			SCUTIMER_CONTROL_ENABLE_MASK);

	return 0;
}

static const struct timer_ops arm_twd_timer_ops = {
	.get_count = arm_twd_timer_get_count,
};

static const struct udevice_id arm_twd_timer_ids[] = {
	{ .compatible = "arm,cortex-a9-twd-timer" },
	{}
};

U_BOOT_DRIVER(arm_twd_timer) = {
	.name = "arm_twd_timer",
	.id = UCLASS_TIMER,
	.of_match = arm_twd_timer_ids,
	.priv_auto	= sizeof(struct arm_twd_timer_priv),
	.probe = arm_twd_timer_probe,
	.ops = &arm_twd_timer_ops,
};
