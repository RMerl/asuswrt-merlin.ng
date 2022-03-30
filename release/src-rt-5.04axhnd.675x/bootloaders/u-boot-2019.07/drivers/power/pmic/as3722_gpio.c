// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 NVIDIA Corporation
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <power/as3722.h>
#include <power/pmic.h>

#define NUM_GPIOS	8

int as3722_gpio_configure(struct udevice *pmic, unsigned int gpio,
			  unsigned long flags)
{
	u8 value = 0;
	int err;

	if (flags & AS3722_GPIO_OUTPUT_VDDH)
		value |= AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH;

	if (flags & AS3722_GPIO_INVERT)
		value |= AS3722_GPIO_CONTROL_INVERT;

	err = pmic_reg_write(pmic, AS3722_GPIO_CONTROL(gpio), value);
	if (err) {
		pr_err("failed to configure GPIO#%u: %d\n", gpio, err);
		return err;
	}

	return 0;
}

static int as3722_gpio_set_value(struct udevice *dev, unsigned int gpio,
				 int level)
{
	struct udevice *pmic = dev_get_parent(dev);
	const char *l;
	u8 value;
	int err;

	if (gpio >= NUM_GPIOS)
		return -EINVAL;

	err = pmic_reg_read(pmic, AS3722_GPIO_SIGNAL_OUT);
	if (err < 0) {
		pr_err("failed to read GPIO signal out register: %d\n", err);
		return err;
	}
	value = err;

	if (level == 0) {
		value &= ~(1 << gpio);
		l = "low";
	} else {
		value |= 1 << gpio;
		l = "high";
	}

	err = pmic_reg_write(pmic, AS3722_GPIO_SIGNAL_OUT, value);
	if (err) {
		pr_err("failed to set GPIO#%u %s: %d\n", gpio, l, err);
		return err;
	}

	return 0;
}

int as3722_gpio_direction_output(struct udevice *dev, unsigned int gpio,
				 int value)
{
	struct udevice *pmic = dev_get_parent(dev);
	int err;

	if (gpio > 7)
		return -EINVAL;

	if (value == 0)
		value = AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDL;
	else
		value = AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH;

	err = pmic_reg_write(pmic, AS3722_GPIO_CONTROL(gpio), value);
	if (err) {
		pr_err("failed to configure GPIO#%u as output: %d\n", gpio,
		       err);
		return err;
	}

	err = as3722_gpio_set_value(pmic, gpio, value);
	if (err < 0) {
		pr_err("failed to set GPIO#%u high: %d\n", gpio, err);
		return err;
	}

	return 0;
}

static int as3722_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = NUM_GPIOS;
	uc_priv->bank_name = "as3722_";

	return 0;
}

static const struct dm_gpio_ops gpio_as3722_ops = {
	.direction_output	= as3722_gpio_direction_output,
	.set_value		= as3722_gpio_set_value,
};

U_BOOT_DRIVER(gpio_as3722) = {
	.name	= "gpio_as3722",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_as3722_ops,
	.probe	= as3722_gpio_probe,
};
