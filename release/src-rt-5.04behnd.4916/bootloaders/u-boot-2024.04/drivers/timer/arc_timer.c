// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/arcregs.h>
#include <asm/global_data.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define NH_MODE (1 << 1)

/*
 * ARC timer control registers are mapped to auxiliary address space.
 * There are special ARC asm command to access that addresses.
 * Therefore we use built-in functions to read from and write to timer
 * control register.
 */

/* Driver private data. Contains timer id. Could be either 0 or 1. */
struct arc_timer_priv {
		uint timer_id;
};

static u64 arc_timer_get_count(struct udevice *dev)
{
	u32 val = 0;
	struct arc_timer_priv *priv = dev_get_priv(dev);

	switch (priv->timer_id) {
	case 0:
		val = read_aux_reg(ARC_AUX_TIMER0_CNT);
		break;
	case 1:
		val = read_aux_reg(ARC_AUX_TIMER1_CNT);
		break;
	}
	return timer_conv_64(val);
}

static int arc_timer_probe(struct udevice *dev)
{
	int id;
	struct arc_timer_priv *priv = dev_get_priv(dev);

	/* Get registers offset and size */
	id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "reg", -1);
	if (id < 0)
		return -EINVAL;

	if (id > 1)
		return -ENXIO;

	priv->timer_id = (uint)id;

	/*
	 * In ARC core there're special registers (Auxiliary or AUX) in its
	 * separate memory space that are used for accessing some hardware
	 * features of the core. They are not mapped in normal memory space
	 * and also always have the same location regardless core configuration.
	 * Thus to simplify understanding of the programming model we chose to
	 * access AUX regs of Timer0 and Timer1 separately instead of using
	 * offsets from some base address.
	 */

	switch (priv->timer_id) {
	case 0:
		/* Disable timer if CPU is halted */
		write_aux_reg(ARC_AUX_TIMER0_CTRL, NH_MODE);
		/* Set max value for counter/timer */
		write_aux_reg(ARC_AUX_TIMER0_LIMIT, 0xffffffff);
		/* Set initial count value and restart counter/timer */
		write_aux_reg(ARC_AUX_TIMER0_CNT, 0);
		break;
	case 1:
		/* Disable timer if CPU is halted */
		write_aux_reg(ARC_AUX_TIMER1_CTRL, NH_MODE);
		/* Set max value for counter/timer */
		write_aux_reg(ARC_AUX_TIMER1_LIMIT, 0xffffffff);
		/* Set initial count value and restart counter/timer */
		write_aux_reg(ARC_AUX_TIMER1_CNT, 0);
		break;
	}

	return 0;
}


static const struct timer_ops arc_timer_ops = {
	.get_count = arc_timer_get_count,
};

static const struct udevice_id arc_timer_ids[] = {
	{ .compatible = "snps,arc-timer" },
	{}
};

U_BOOT_DRIVER(arc_timer) = {
	.name	= "arc_timer",
	.id	= UCLASS_TIMER,
	.of_match = arc_timer_ids,
	.probe = arc_timer_probe,
	.ops	= &arc_timer_ops,
	.priv_auto	= sizeof(struct arc_timer_priv),
};
