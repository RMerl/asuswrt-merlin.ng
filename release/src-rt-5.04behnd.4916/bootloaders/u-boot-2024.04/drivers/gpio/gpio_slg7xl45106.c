// SPDX-License-Identifier: GPL-2.0
/*
 * slg7xl45106_i2c_gpo driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm.h>
#include <i2c.h>
#include <dt-bindings/gpio/gpio.h>
#include <asm/arch/hardware.h>

#define SLG7XL45106_REG		0xdb

static int slg7xl45106_i2c_gpo_direction_input(struct udevice *dev,
					       unsigned int offset)
{
	return 0;
}

static int slg7xl45106_i2c_gpo_xlate(struct udevice *dev,
				     struct gpio_desc *desc,
				     struct ofnode_phandle_args *args)
{
	desc->offset = (unsigned int)args->args[0];
	desc->flags = (args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0);

	return 0;
}

static int slg7xl45106_i2c_gpo_set_value(struct udevice *dev,
					 unsigned int offset, int value)
{
	int ret;
	u8 val;

	ret = dm_i2c_read(dev, SLG7XL45106_REG, &val, 1);
	if (ret)
		return ret;

	if (value)
		val |= BIT(offset);
	else
		val &= ~BIT(offset);

	return dm_i2c_write(dev, SLG7XL45106_REG, &val, 1);
}

static int slg7xl45106_i2c_gpo_direction_output(struct udevice *dev,
						unsigned int offset, int value)
{
	return slg7xl45106_i2c_gpo_set_value(dev, offset, value);
}

static int slg7xl45106_i2c_gpo_get_value(struct udevice *dev,
					 unsigned int offset)
{
	int ret;
	u8 val;

	ret = dm_i2c_read(dev, SLG7XL45106_REG, &val, 1);
	if (ret)
		return ret;

	return !!(val & BIT(offset));
}

static int slg7xl45106_i2c_gpo_get_function(struct udevice *dev,
					    unsigned int offset)
{
	return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops slg7xl45106_i2c_gpo_ops = {
	.direction_input = slg7xl45106_i2c_gpo_direction_input,
	.direction_output = slg7xl45106_i2c_gpo_direction_output,
	.get_value = slg7xl45106_i2c_gpo_get_value,
	.set_value = slg7xl45106_i2c_gpo_set_value,
	.get_function = slg7xl45106_i2c_gpo_get_function,
	.xlate = slg7xl45106_i2c_gpo_xlate,
};

static int slg7xl45106_i2c_gpo_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *label_ptr;

	label_ptr = dev_read_prop(dev, "label", NULL);
	if (label_ptr) {
		uc_priv->bank_name = strdup(label_ptr);
		if (!uc_priv->bank_name)
			return -ENOMEM;
	} else {
		uc_priv->bank_name = dev->name;
	}

	uc_priv->gpio_count = 8;

	return 0;
}

static const struct udevice_id slg7xl45106_i2c_gpo_ids[] = {
	{ .compatible = "dlg,slg7xl45106",},
	{ }
};

U_BOOT_DRIVER(slg7xl45106_i2c_gpo) = {
	.name = "slg7xl45106_i2c_gpo",
	.id = UCLASS_GPIO,
	.ops = &slg7xl45106_i2c_gpo_ops,
	.of_match = slg7xl45106_i2c_gpo_ids,
	.probe = slg7xl45106_i2c_gpo_probe,
};
