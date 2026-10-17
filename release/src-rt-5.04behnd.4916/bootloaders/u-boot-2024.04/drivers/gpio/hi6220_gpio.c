// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/bitops.h>

static int hi6220_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	u8 data;

	data = readb(bank->base + HI6220_GPIO_DIR);
	data &= ~(1 << gpio);
	writeb(data, bank->base + HI6220_GPIO_DIR);

	return 0;
}

static int hi6220_gpio_set_value(struct udevice *dev, unsigned gpio,
				  int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	writeb(!!value << gpio, bank->base + (BIT(gpio + 2)));
	return 0;
}

static int hi6220_gpio_direction_output(struct udevice *dev, unsigned gpio,
					int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	u8 data;

	data = readb(bank->base + HI6220_GPIO_DIR);
	data |= 1 << gpio;
	writeb(data, bank->base + HI6220_GPIO_DIR);

	hi6220_gpio_set_value(dev, gpio, value);

	return 0;
}

static int hi6220_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	return !!readb(bank->base + (BIT(gpio + 2)));
}

static const struct dm_gpio_ops gpio_hi6220_ops = {
	.direction_input	= hi6220_gpio_direction_input,
	.direction_output	= hi6220_gpio_direction_output,
	.get_value		= hi6220_gpio_get_value,
	.set_value		= hi6220_gpio_set_value,
};

static int hi6220_gpio_probe(struct udevice *dev)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	struct hikey_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[18], *str;

	sprintf(name, "GPIO%d_", plat->bank_index);

	str = strdup(name);
	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = HI6220_GPIO_PER_BANK;

	bank->base = (u8 *)plat->base;

	return 0;
}

U_BOOT_DRIVER(gpio_hi6220) = {
	.name	= "gpio_hi6220",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_hi6220_ops,
	.probe	= hi6220_gpio_probe,
	.priv_auto	= sizeof(struct gpio_bank),
};
