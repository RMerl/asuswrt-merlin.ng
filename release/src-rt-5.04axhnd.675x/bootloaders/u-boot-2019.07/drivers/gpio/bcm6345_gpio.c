// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/gpio.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright (C) 2008-2011 Florian Fainelli <florian@openwrt.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

struct bcm6345_gpio_priv {
	void __iomem *reg_dirout;
	void __iomem *reg_data;
};

static int bcm6345_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	return !!(readl(priv->reg_data) & BIT(offset));
}

static int bcm6345_gpio_set_value(struct udevice *dev, unsigned offset,
				  int value)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	if (value)
		setbits_32(priv->reg_data, BIT(offset));
	else
		clrbits_32(priv->reg_data, BIT(offset));

	return 0;
}

static int bcm6345_gpio_set_direction(void __iomem *dirout, unsigned offset,
				      bool input)
{
	if (input)
		clrbits_32(dirout, BIT(offset));
	else
		setbits_32(dirout, BIT(offset));

	return 0;
}

static int bcm6345_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	return bcm6345_gpio_set_direction(priv->reg_dirout, offset, 1);
}

static int bcm6345_gpio_direction_output(struct udevice *dev, unsigned offset,
					 int value)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	bcm6345_gpio_set_value(dev, offset, value);

	return bcm6345_gpio_set_direction(priv->reg_dirout, offset, 0);
}

static int bcm6345_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	if (readl(priv->reg_dirout) & BIT(offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops bcm6345_gpio_ops = {
	.direction_input = bcm6345_gpio_direction_input,
	.direction_output = bcm6345_gpio_direction_output,
	.get_value = bcm6345_gpio_get_value,
	.set_value = bcm6345_gpio_set_value,
	.get_function = bcm6345_gpio_get_function,
};

static int bcm6345_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	priv->reg_dirout = dev_remap_addr_index(dev, 0);
	if (!priv->reg_dirout)
		return -EINVAL;

	priv->reg_data = dev_remap_addr_index(dev, 1);
	if (!priv->reg_data)
		return -EINVAL;

	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", 32);
	uc_priv->bank_name = dev->name;

	return 0;
}

static const struct udevice_id bcm6345_gpio_ids[] = {
	{ .compatible = "brcm,bcm6345-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6345_gpio) = {
	.name = "bcm6345-gpio",
	.id = UCLASS_GPIO,
	.of_match = bcm6345_gpio_ids,
	.ops = &bcm6345_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6345_gpio_priv),
	.probe = bcm6345_gpio_probe,
};
