// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Microchip Corporation
 *
 * Author: Clément Léger <clement.leger@bootlin.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define TCB_CHAN(chan)		((chan) * 0x40)

#define TCB_CCR(chan)		(0x0 + TCB_CHAN(chan))
#define  TCB_CCR_CLKEN		(1 << 0)

#define TCB_CMR(chan)		(0x4 + TCB_CHAN(chan))
#define  TCB_CMR_WAVE		(1 << 15)
#define  TCB_CMR_TIMER_CLOCK2	1
#define  TCB_CMR_XC1		6
#define  TCB_CMR_ACPA_SET	(1 << 16)
#define  TCB_CMR_ACPC_CLEAR	(2 << 18)

#define TCB_CV(chan)		(0x10 + TCB_CHAN(chan))

#define TCB_RA(chan)		(0x14 + TCB_CHAN(chan))
#define TCB_RC(chan)		(0x1c + TCB_CHAN(chan))

#define TCB_IDR(chan)		(0x28 + TCB_CHAN(chan))

#define TCB_BCR			0xc0
#define  TCB_BCR_SYNC		(1 << 0)

#define TCB_BMR			0xc4
#define  TCB_BMR_TC1XC1S_TIOA0	(2 << 2)

#define TCB_WPMR		0xe4
#define  TCB_WPMR_WAKEY		0x54494d

#define TCB_CLK_DIVISOR		8
struct atmel_tcb_plat {
	void __iomem *base;
};

static u64 atmel_tcb_get_count(struct udevice *dev)
{
	struct atmel_tcb_plat *plat = dev_get_plat(dev);
	u64 cv0 = 0;
	u64 cv1 = 0;

	do {
		cv1 = readl(plat->base + TCB_CV(1));
		cv0 = readl(plat->base + TCB_CV(0));
	} while (readl(plat->base + TCB_CV(1)) != cv1);

	cv0 |= cv1 << 32;

	return cv0;
}

static void atmel_tcb_configure(void __iomem *base)
{
	/* Disable write protection */
	writel(TCB_WPMR_WAKEY, base + TCB_WPMR);

	/* Disable all irqs for both channel 0 & 1 */
	writel(0xff, base + TCB_IDR(0));
	writel(0xff, base + TCB_IDR(1));

	/*
	 * In order to avoid wrapping, use a 64 bit counter by chaining
	 * two channels.
	 * Channel 0 is configured to generate a clock on TIOA0 which is cleared
	 * when reaching 0x80000000 and set when reaching 0.
	 */
	writel(TCB_CMR_TIMER_CLOCK2 | TCB_CMR_WAVE | TCB_CMR_ACPA_SET
		   | TCB_CMR_ACPC_CLEAR, base + TCB_CMR(0));
	writel(0x80000000, base + TCB_RC(0));
	writel(0x1, base + TCB_RA(0));
	writel(TCB_CCR_CLKEN, base + TCB_CCR(0));

	/* Channel 1 is configured to use TIOA0 as input */
	writel(TCB_CMR_XC1 | TCB_CMR_WAVE, base + TCB_CMR(1));
	writel(TCB_CCR_CLKEN, base + TCB_CCR(1));

	/* Set XC1 input to be TIOA0 (ie output of Channel 0) */
	writel(TCB_BMR_TC1XC1S_TIOA0, base + TCB_BMR);

	/* Sync & start all timers */
	writel(TCB_BCR_SYNC, base + TCB_BCR);
}

static int atmel_tcb_probe(struct udevice *dev)
{
	struct atmel_tcb_plat *plat = dev_get_plat(dev);
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	if (!device_is_compatible(dev->parent, "atmel,sama5d2-tcb"))
		return -EINVAL;

	/* Currently, we only support channel 0 and 1 to be chained */
	if (dev_read_addr_index(dev, 0) != 0 &&
	    dev_read_addr_index(dev, 1) != 1) {
		printf("Error: only chained timers 0 and 1 are supported\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev->parent, "t0_clk", &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate) {
		clk_disable(&clk);
		return -EINVAL;
	}

	uc_priv->clock_rate = clk_rate / TCB_CLK_DIVISOR;

	atmel_tcb_configure(plat->base);

	return 0;
}

static int atmel_tcb_of_to_plat(struct udevice *dev)
{
	struct atmel_tcb_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev->parent);

	return 0;
}

static const struct timer_ops atmel_tcb_ops = {
	.get_count = atmel_tcb_get_count,
};

static const struct udevice_id atmel_tcb_ids[] = {
	{ .compatible = "atmel,tcb-timer" },
	{ }
};

U_BOOT_DRIVER(atmel_tcb) = {
	.name = "atmel_tcb",
	.id = UCLASS_TIMER,
	.of_match = atmel_tcb_ids,
	.of_to_plat = atmel_tcb_of_to_plat,
	.plat_auto = sizeof(struct atmel_tcb_plat),
	.probe = atmel_tcb_probe,
	.ops = &atmel_tcb_ops,
};
