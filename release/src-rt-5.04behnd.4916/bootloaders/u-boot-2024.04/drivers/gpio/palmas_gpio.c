// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on mainline Linux palmas GPIO driver
 * Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <power/palmas.h>

#define NUM_GPIOS	8

static int palmas_gpio_set_value(struct udevice *dev, unsigned int offset,
				 int value)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	u32 reg;
	int ret;

	reg = (value) ? PALMAS_GPIO_SET_DATA_OUT : PALMAS_GPIO_CLEAR_DATA_OUT;

	ret = dm_i2c_reg_write(priv->chip2, reg, BIT(offset));
	if (ret < 0)
		log_debug("%s: Reg 0x%02x write failed, %d\n", __func__, reg, ret);

	return ret;
}

static int palmas_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	u32 reg;
	int ret;

	ret = dm_i2c_reg_read(priv->chip2, PALMAS_GPIO_DATA_DIR);
	if (ret < 0) {
		log_debug("%s: GPIO_DATA_DIR read failed, %d\n", __func__, ret);
		return ret;
	}

	if (ret & BIT(offset))
		reg = PALMAS_GPIO_DATA_OUT;
	else
		reg = PALMAS_GPIO_DATA_IN;

	ret = dm_i2c_reg_read(priv->chip2, reg);
	if (ret < 0) {
		log_debug("%s: Reg 0x%02x read failed, %d\n", __func__, reg, ret);
		return ret;
	}

	return !!(ret & BIT(offset));
}

static int palmas_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	int ret;

	ret = dm_i2c_reg_clrset(priv->chip2, PALMAS_GPIO_DATA_DIR,
				BIT(offset), 0);
	if (ret < 0)
		log_debug("%s: GPIO_DATA_DIR val update failed: %d\n", __func__, ret);

	return ret;
}

static int palmas_gpio_direction_output(struct udevice *dev, unsigned int offset,
					int value)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	int ret;

	/* Set the initial value */
	palmas_gpio_set_value(dev, offset, value);

	ret = dm_i2c_reg_clrset(priv->chip2, PALMAS_GPIO_DATA_DIR,
				BIT(offset), BIT(offset));
	if (ret < 0)
		log_debug("%s: GPIO_DATA_DIR val update failed: %d\n", __func__, ret);

	return ret;
}

static int palmas_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	int ret;

	ret = dm_i2c_reg_read(priv->chip2, PALMAS_GPIO_DATA_DIR);
	if (ret < 0) {
		log_debug("%s: GPIO_DATA_DIR read failed, %d\n", __func__, ret);
		return ret;
	}

	if (ret & BIT(offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops palmas_gpio_ops = {
	.direction_input	= palmas_gpio_direction_input,
	.direction_output	= palmas_gpio_direction_output,
	.get_value		= palmas_gpio_get_value,
	.set_value		= palmas_gpio_set_value,
	.get_function		= palmas_gpio_get_function,
};

static int palmas_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = NUM_GPIOS;
	uc_priv->bank_name = "GPIO";

	return 0;
}

static const struct udevice_id palmas_ids[] = {
	{ .compatible = "ti,palmas-gpio" },
	{ }
};

U_BOOT_DRIVER(palmas_gpio) = {
	.name	= PALMAS_GPIO_DRIVER,
	.id	= UCLASS_GPIO,
	.of_match = palmas_ids,
	.probe	= palmas_gpio_probe,
	.ops	= &palmas_gpio_ops,
};
