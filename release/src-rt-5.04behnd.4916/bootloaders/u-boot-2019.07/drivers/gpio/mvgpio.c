// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include "mvgpio.h"
#include <asm/gpio.h>

#ifndef MV_MAX_GPIO
#define MV_MAX_GPIO	128
#endif

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO requested %d\n", __func__, gpio);
		return -1;
	}
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gcdr);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gsdr);
	gpio_set_value(gpio, value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct gpio_reg *gpio_reg_bank;
	u32 gpio_val;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	gpio_val = readl(&gpio_reg_bank->gplr);

	return GPIO_VAL(gpio, gpio_val);
}

int gpio_set_value(unsigned gpio, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	if (value)
		writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gpsr);
	else
		writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gpcr);

	return 0;
}
