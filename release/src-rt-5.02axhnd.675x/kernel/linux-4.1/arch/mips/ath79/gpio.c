/*
 *  Atheros AR71XX/AR724X/AR913X GPIO API support
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros' 2.6.15/2.6.31 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include "common.h"

static void __iomem *ath79_gpio_base;
static unsigned long ath79_gpio_count;
static DEFINE_SPINLOCK(ath79_gpio_lock);

static void __ath79_gpio_set_value(unsigned gpio, int value)
{
	void __iomem *base = ath79_gpio_base;

	if (value)
		__raw_writel(1 << gpio, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << gpio, base + AR71XX_GPIO_REG_CLEAR);
}

static int __ath79_gpio_get_value(unsigned gpio)
{
	return (__raw_readl(ath79_gpio_base + AR71XX_GPIO_REG_IN) >> gpio) & 1;
}

static int ath79_gpio_get_value(struct gpio_chip *chip, unsigned offset)
{
	return __ath79_gpio_get_value(offset);
}

static void ath79_gpio_set_value(struct gpio_chip *chip,
				  unsigned offset, int value)
{
	__ath79_gpio_set_value(offset, value);
}

static int ath79_gpio_direction_input(struct gpio_chip *chip,
				       unsigned offset)
{
	void __iomem *base = ath79_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ath79_gpio_lock, flags);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << offset),
		     base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ath79_gpio_lock, flags);

	return 0;
}

static int ath79_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	void __iomem *base = ath79_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ath79_gpio_lock, flags);

	if (value)
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_CLEAR);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) | (1 << offset),
		     base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ath79_gpio_lock, flags);

	return 0;
}

static int ar934x_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	void __iomem *base = ath79_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ath79_gpio_lock, flags);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) | (1 << offset),
		     base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ath79_gpio_lock, flags);

	return 0;
}

static int ar934x_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
					int value)
{
	void __iomem *base = ath79_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ath79_gpio_lock, flags);

	if (value)
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_CLEAR);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << offset),
		     base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ath79_gpio_lock, flags);

	return 0;
}

static struct gpio_chip ath79_gpio_chip = {
	.label			= "ath79",
	.get			= ath79_gpio_get_value,
	.set			= ath79_gpio_set_value,
	.direction_input	= ath79_gpio_direction_input,
	.direction_output	= ath79_gpio_direction_output,
	.base			= 0,
};

static void __iomem *ath79_gpio_get_function_reg(void)
{
	u32 reg = 0;

	if (soc_is_ar71xx() ||
	    soc_is_ar724x() ||
	    soc_is_ar913x() ||
	    soc_is_ar933x())
		reg = AR71XX_GPIO_REG_FUNC;
	else if (soc_is_ar934x())
		reg = AR934X_GPIO_REG_FUNC;
	else
		BUG();

	return ath79_gpio_base + reg;
}

void ath79_gpio_function_setup(u32 set, u32 clear)
{
	void __iomem *reg = ath79_gpio_get_function_reg();
	unsigned long flags;

	spin_lock_irqsave(&ath79_gpio_lock, flags);

	__raw_writel((__raw_readl(reg) & ~clear) | set, reg);
	/* flush write */
	__raw_readl(reg);

	spin_unlock_irqrestore(&ath79_gpio_lock, flags);
}

void ath79_gpio_function_enable(u32 mask)
{
	ath79_gpio_function_setup(mask, 0);
}

void ath79_gpio_function_disable(u32 mask)
{
	ath79_gpio_function_setup(0, mask);
}

void __init ath79_gpio_init(void)
{
	int err;

	if (soc_is_ar71xx())
		ath79_gpio_count = AR71XX_GPIO_COUNT;
	else if (soc_is_ar7240())
		ath79_gpio_count = AR7240_GPIO_COUNT;
	else if (soc_is_ar7241() || soc_is_ar7242())
		ath79_gpio_count = AR7241_GPIO_COUNT;
	else if (soc_is_ar913x())
		ath79_gpio_count = AR913X_GPIO_COUNT;
	else if (soc_is_ar933x())
		ath79_gpio_count = AR933X_GPIO_COUNT;
	else if (soc_is_ar934x())
		ath79_gpio_count = AR934X_GPIO_COUNT;
	else if (soc_is_qca955x())
		ath79_gpio_count = QCA955X_GPIO_COUNT;
	else
		BUG();

	ath79_gpio_base = ioremap_nocache(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);
	ath79_gpio_chip.ngpio = ath79_gpio_count;
	if (soc_is_ar934x() || soc_is_qca955x()) {
		ath79_gpio_chip.direction_input = ar934x_gpio_direction_input;
		ath79_gpio_chip.direction_output = ar934x_gpio_direction_output;
	}

	err = gpiochip_add(&ath79_gpio_chip);
	if (err)
		panic("cannot add AR71xx GPIO chip, error=%d", err);
}

int gpio_get_value(unsigned gpio)
{
	if (gpio < ath79_gpio_count)
		return __ath79_gpio_get_value(gpio);

	return __gpio_get_value(gpio);
}
EXPORT_SYMBOL(gpio_get_value);

void gpio_set_value(unsigned gpio, int value)
{
	if (gpio < ath79_gpio_count)
		__ath79_gpio_set_value(gpio, value);
	else
		__gpio_set_value(gpio, value);
}
EXPORT_SYMBOL(gpio_set_value);

int gpio_to_irq(unsigned gpio)
{
	/* FIXME */
	return -EINVAL;
}
EXPORT_SYMBOL(gpio_to_irq);

int irq_to_gpio(unsigned irq)
{
	/* FIXME */
	return -EINVAL;
}
EXPORT_SYMBOL(irq_to_gpio);
