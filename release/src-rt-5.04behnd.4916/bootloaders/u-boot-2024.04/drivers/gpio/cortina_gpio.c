// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Cortina-Access
 *
 * GPIO Driver for Cortina Access CAxxxx Line of SoCs
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/compiler.h>

/* GPIO Register Map */
#define CORTINA_GPIO_CFG	0x00
#define CORTINA_GPIO_OUT	0x04
#define CORTINA_GPIO_IN		0x08
#define CORTINA_GPIO_LVL	0x0C
#define CORTINA_GPIO_EDGE	0x10
#define CORTINA_GPIO_BOTHEDGE	0x14
#define CORTINA_GPIO_IE		0x18
#define CORTINA_GPIO_INT	0x1C
#define CORTINA_GPIO_STAT	0x20

struct cortina_gpio_bank {
	void __iomem *base;
};

#ifdef CONFIG_DM_GPIO
static int ca_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	setbits_32(priv->base, BIT(offset));
	return 0;
}

static int
ca_gpio_direction_output(struct udevice *dev, unsigned int offset, int value)
{
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	clrbits_32(priv->base, BIT(offset));
	return 0;
}

static int ca_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	return readl(priv->base + CORTINA_GPIO_IN) & BIT(offset);
}

static int ca_gpio_set_value(struct udevice *dev, unsigned int offset,
			     int value)
{
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	setbits_32(priv->base + CORTINA_GPIO_OUT, BIT(offset));
	return 0;
}

static int ca_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	if (readl(priv->base) & BIT(offset))
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops gpio_cortina_ops = {
	.direction_input = ca_gpio_direction_input,
	.direction_output = ca_gpio_direction_output,
	.get_value = ca_gpio_get_value,
	.set_value = ca_gpio_set_value,
	.get_function = ca_gpio_get_function,
};

static int ca_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct cortina_gpio_bank *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -EINVAL;

	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", 32);
	uc_priv->bank_name = dev->name;

	debug("Done Cortina GPIO init\n");
	return 0;
}

static const struct udevice_id ca_gpio_ids[] = {
	{.compatible = "cortina,ca-gpio"},
	{}
};

U_BOOT_DRIVER(cortina_gpio) = {
	.name = "cortina-gpio",
	.id = UCLASS_GPIO,
	.ops = &gpio_cortina_ops,
	.probe = ca_gpio_probe,
	.priv_auto	= sizeof(struct cortina_gpio_bank),
	.of_match = ca_gpio_ids,
};
#endif /* CONFIG_DM_GPIO */
