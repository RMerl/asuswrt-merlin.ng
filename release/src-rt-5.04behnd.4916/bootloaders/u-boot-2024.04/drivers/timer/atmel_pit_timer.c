// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Microchip Corporation
 *		      Wenyou.Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define AT91_PIT_VALUE		0xfffff
#define AT91_PIT_PITEN		BIT(24)		/* Timer Enabled */

struct atmel_pit_regs {
	u32	mode;
	u32	status;
	u32	value;
	u32	value_image;
};

struct atmel_pit_plat {
	struct atmel_pit_regs *regs;
};

static u64 atmel_pit_get_count(struct udevice *dev)
{
	struct atmel_pit_plat *plat = dev_get_plat(dev);
	struct atmel_pit_regs *const regs = plat->regs;
	u32 val = readl(&regs->value_image);

	return timer_conv_64(val);
}

static int atmel_pit_probe(struct udevice *dev)
{
	struct atmel_pit_plat *plat = dev_get_plat(dev);
	struct atmel_pit_regs *const regs = plat->regs;
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;

	uc_priv->clock_rate = clk_rate / 16;

	writel(AT91_PIT_VALUE | AT91_PIT_PITEN, &regs->mode);

	return 0;
}

static int atmel_pit_of_to_plat(struct udevice *dev)
{
	struct atmel_pit_plat *plat = dev_get_plat(dev);

	plat->regs = dev_read_addr_ptr(dev);

	return 0;
}

static const struct timer_ops atmel_pit_ops = {
	.get_count = atmel_pit_get_count,
};

static const struct udevice_id atmel_pit_ids[] = {
	{ .compatible = "atmel,at91sam9260-pit" },
	{ }
};

U_BOOT_DRIVER(atmel_pit) = {
	.name	= "atmel_pit",
	.id	= UCLASS_TIMER,
	.of_match = atmel_pit_ids,
	.of_to_plat = atmel_pit_of_to_plat,
	.plat_auto	= sizeof(struct atmel_pit_plat),
	.probe	= atmel_pit_probe,
	.ops	= &atmel_pit_ops,
};
