// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO driver for the sl28cpld
 *
 * Copyright (c) 2021 Michael Walle <michael@walle.cc>
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <sl28cpld.h>

/* GPIO flavor */
#define SL28CPLD_GPIO_DIR	0x00
#define SL28CPLD_GPIO_OUT	0x01
#define SL28CPLD_GPIO_IN	0x02

/* input-only flavor */
#define SL28CPLD_GPI_IN		0x00

/* output-only flavor */
#define SL28CPLD_GPO_OUT	0x00

enum {
	SL28CPLD_GPIO,
	SL28CPLD_GPI,
	SL28CPLD_GPO,
};

static int sl28cpld_gpio_get_value(struct udevice *dev, unsigned int gpio)
{
	ulong type = dev_get_driver_data(dev);
	int val, reg;

	switch (type) {
	case SL28CPLD_GPIO:
		reg = SL28CPLD_GPIO_IN;
		break;
	case SL28CPLD_GPI:
		reg = SL28CPLD_GPI_IN;
		break;
	case SL28CPLD_GPO:
		/* we are output only, thus just return the output value */
		reg = SL28CPLD_GPO_OUT;
		break;
	default:
		return -EINVAL;
	}

	val = sl28cpld_read(dev, reg);

	return val < 0 ? val : !!(val & BIT(gpio));
}

static int sl28cpld_gpio_set_value(struct udevice *dev, unsigned int gpio,
				   int value)
{
	ulong type = dev_get_driver_data(dev);
	uint reg;

	switch (type) {
	case SL28CPLD_GPIO:
		reg = SL28CPLD_GPIO_OUT;
		break;
	case SL28CPLD_GPO:
		reg = SL28CPLD_GPO_OUT;
		break;
	case SL28CPLD_GPI:
	default:
		return -EINVAL;
	}

	if (value)
		return sl28cpld_update(dev, reg, 0, BIT(gpio));
	else
		return sl28cpld_update(dev, reg, BIT(gpio), 0);
}

static int sl28cpld_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	ulong type = dev_get_driver_data(dev);

	switch (type) {
	case SL28CPLD_GPI:
		return 0;
	case SL28CPLD_GPIO:
		return sl28cpld_update(dev, SL28CPLD_GPIO_DIR, BIT(gpio), 0);
	case SL28CPLD_GPO:
	default:
		return -EINVAL;
	}
}

static int sl28cpld_gpio_direction_output(struct udevice *dev,
					  unsigned int gpio, int value)
{
	ulong type = dev_get_driver_data(dev);
	int ret;

	/* set_value() will report an error if we are input-only */
	ret = sl28cpld_gpio_set_value(dev, gpio, value);
	if (ret)
		return ret;

	if (type == SL28CPLD_GPIO)
		return sl28cpld_update(dev, SL28CPLD_GPIO_DIR, 0, BIT(gpio));

	return 0;
}

static int sl28cpld_gpio_get_function(struct udevice *dev, unsigned int gpio)
{
	ulong type = dev_get_driver_data(dev);
	int val;

	switch (type) {
	case SL28CPLD_GPIO:
		val = sl28cpld_read(dev, SL28CPLD_GPIO_DIR);
		if (val < 0)
			return val;
		if (val & BIT(gpio))
			return GPIOF_OUTPUT;
		else
			return GPIOF_INPUT;
	case SL28CPLD_GPI:
		return GPIOF_INPUT;
	case SL28CPLD_GPO:
		return GPIOF_OUTPUT;
	default:
		return -EINVAL;
	}
}

static const struct dm_gpio_ops sl28cpld_gpio_ops = {
	.direction_input = sl28cpld_gpio_direction_input,
	.direction_output = sl28cpld_gpio_direction_output,
	.get_value = sl28cpld_gpio_get_value,
	.set_value = sl28cpld_gpio_set_value,
	.get_function = sl28cpld_gpio_get_function,
};

static int sl28cpld_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 8;
	uc_priv->bank_name = dev_read_name(dev);

	return 0;
}

static const struct udevice_id sl28cpld_gpio_ids[] = {
	{ .compatible = "kontron,sl28cpld-gpio", .data = SL28CPLD_GPIO},
	{ .compatible = "kontron,sl28cpld-gpo", .data = SL28CPLD_GPO},
	{ .compatible = "kontron,sl28cpld-gpi", .data = SL28CPLD_GPI},
	{ }
};

U_BOOT_DRIVER(sl28cpld_gpio) = {
	.name	= "sl28cpld_gpio",
	.id	= UCLASS_GPIO,
	.of_match = sl28cpld_gpio_ids,
	.probe	= sl28cpld_gpio_probe,
	.ops	= &sl28cpld_gpio_ops,
};
