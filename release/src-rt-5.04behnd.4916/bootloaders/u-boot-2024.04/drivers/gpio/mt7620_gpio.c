// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * GPIO controller driver for MediaTek MT7620 SoC
 */

#include <dm.h>
#include <errno.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/gpio.h>

enum mt7620_regs {
	GPIO_REG_DATA,
	GPIO_REG_DIR,
	GPIO_REG_SET,
	GPIO_REG_CLR,

	__GPIO_REG_MAX
};

struct mt7620_gpio_priv {
	void __iomem *base;
	u32 regs[__GPIO_REG_MAX];
	u32 count;
};

static int mt7620_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);

	return !!(readl(priv->base + priv->regs[GPIO_REG_DATA]) & BIT(offset));
}

static int mt7620_gpio_set_value(struct udevice *dev, unsigned int offset,
				 int value)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);
	u32 reg;

	reg = value ? priv->regs[GPIO_REG_SET] : priv->regs[GPIO_REG_CLR];

	writel(BIT(offset), priv->base + reg);

	return 0;
}

static int mt7620_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);

	clrbits_32(priv->base + priv->regs[GPIO_REG_DIR], BIT(offset));

	return 0;
}

static int mt7620_gpio_direction_output(struct udevice *dev,
					unsigned int offset, int value)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);

	/* Set value first */
	mt7620_gpio_set_value(dev, offset, value);

	setbits_32(priv->base + priv->regs[GPIO_REG_DIR], BIT(offset));

	return 0;
}

static int mt7620_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);

	return (readl(priv->base + priv->regs[GPIO_REG_DIR]) & BIT(offset)) ?
	       GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops mt7620_gpio_ops = {
	.direction_input	= mt7620_gpio_direction_input,
	.direction_output	= mt7620_gpio_direction_output,
	.get_value		= mt7620_gpio_get_value,
	.set_value		= mt7620_gpio_set_value,
	.get_function		= mt7620_gpio_get_function,
};

static int mt7620_gpio_probe(struct udevice *dev)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const char *name;

	name = dev_read_string(dev, "mediatek,bank-name");
	if (!name)
		name = dev->name;

	uc_priv->gpio_count = priv->count;
	uc_priv->bank_name = name;

	return 0;
}

static int mt7620_gpio_of_to_plat(struct udevice *dev)
{
	struct mt7620_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base) {
		dev_err(dev, "mt7620_gpio: unable to map registers\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "mediatek,gpio-num", &priv->count);
	if (ret) {
		dev_err(dev, "mt7620_gpio: failed to get GPIO count\n");
		return -EINVAL;
	}

	ret = dev_read_u32_array(dev, "mediatek,register-map", priv->regs,
				 __GPIO_REG_MAX);
	if (ret) {
		dev_err(dev, "mt7620_gpio: unable to get register map\n");
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id mt7620_gpio_ids[] = {
	{ .compatible = "mediatek,mt7620-gpio" },
	{ }
};

U_BOOT_DRIVER(mt7620_gpio) = {
	.name	= "mt7620_gpio",
	.id	= UCLASS_GPIO,
	.ops	= &mt7620_gpio_ops,
	.of_match = mt7620_gpio_ids,
	.probe	= mt7620_gpio_probe,
	.of_to_plat = mt7620_gpio_of_to_plat,
	.priv_auto = sizeof(struct mt7620_gpio_priv),
};
