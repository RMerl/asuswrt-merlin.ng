// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

/*
 * Driver for SPEAr600 GPIO controller
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

static int gpio_direction(unsigned gpio,
			  enum gpio_direction direction)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;
	u32 val;

	val = readl(&regs->gpiodir);

	if (direction == GPIO_DIRECTION_OUT)
		val |= 1 << gpio;
	else
		val &= ~(1 << gpio);

	writel(val, &regs->gpiodir);

	return 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;

	if (value)
		writel(1 << gpio, &regs->gpiodata[DATA_REG_ADDR(gpio)]);
	else
		writel(0, &regs->gpiodata[DATA_REG_ADDR(gpio)]);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;
	u32 val;

	val = readl(&regs->gpiodata[DATA_REG_ADDR(gpio)]);

	return !!val;
}

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= SPEAR_GPIO_COUNT)
		return -EINVAL;

	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

void gpio_toggle_value(unsigned gpio)
{
	gpio_set_value(gpio, !gpio_get_value(gpio));
}

int gpio_direction_input(unsigned gpio)
{
	return gpio_direction(gpio, GPIO_DIRECTION_IN);
}

int gpio_direction_output(unsigned gpio, int value)
{
	int ret = gpio_direction(gpio, GPIO_DIRECTION_OUT);

	if (ret < 0)
		return ret;

	gpio_set_value(gpio, value);
	return 0;
}
