// SPDX-License-Identifier: GPL-2.0
/*
 * max7320 I2C GPIO EXPANDER DRIVER
 *
 * Copyright (C) 2021 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm-generic/gpio.h>
#include <linux/bitops.h>

struct max7320_chip {
	u32 outreg;
};

static int max7320_direction_output(struct udevice *dev,
				    unsigned int offset, int value)
{
	struct max7320_chip *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	int ret;

	if (value)
		plat->outreg |= BIT(offset);
	else
		plat->outreg &= ~BIT(offset);

	ret = dm_i2c_write(dev,
			   plat->outreg & 0xff,
			   (uint8_t *)&plat->outreg + 1,
			   uc_priv->gpio_count > 8 ? 1 : 0);
	if (ret)
		printf("%s i2c write failed to addr %x\n", __func__,
		       chip->chip_addr);

	return ret;
}

static int max7320_get_value(struct udevice *dev, unsigned int offset)
{
	struct max7320_chip *plat = dev_get_plat(dev);

	return (plat->outreg >> offset) & 0x1;
}

static int max7320_set_value(struct udevice *dev, unsigned int offset,
			     int value)
{
	return max7320_direction_output(dev, offset, value);
}

static int max7320_get_function(struct udevice *dev, unsigned int offset)
{
	return GPIOF_OUTPUT;
}

static int max7320_ofdata_plat(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", 8);
	if (uc_priv->gpio_count > 16) {
		printf("%s: max7320 doesn't support more than 16 gpios!",
		       __func__);
		return -EINVAL;
	}

	uc_priv->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
					 "gpio-bank-name", NULL);
	if (!uc_priv->bank_name)
		uc_priv->bank_name = fdt_get_name(gd->fdt_blob,
						  dev_of_offset(dev), NULL);

	return 0;
}

static int max7320_gpio_probe(struct udevice  *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	debug("%s GPIO controller with %d gpios probed\n",
	      uc_priv->bank_name, uc_priv->gpio_count);

	return 0;
}

static const struct dm_gpio_ops max7320_gpio_ops = {
	.direction_output	= max7320_direction_output,
	.set_value		= max7320_set_value,
	.get_value		= max7320_get_value,
	.get_function		= max7320_get_function,
};

static const struct udevice_id max7320_gpio_ids[] = {
	{ .compatible = "maxim,max7320" },
	{ }
};

U_BOOT_DRIVER(gpio_max7320) = {
	.name		= "gpio_max7320",
	.id		= UCLASS_GPIO,
	.ops		= &max7320_gpio_ops,
	.of_match	= max7320_gpio_ids,
	.of_to_plat	= max7320_ofdata_plat,
	.probe		= max7320_gpio_probe,
	.plat_auto	= sizeof(struct max7320_chip),
};
