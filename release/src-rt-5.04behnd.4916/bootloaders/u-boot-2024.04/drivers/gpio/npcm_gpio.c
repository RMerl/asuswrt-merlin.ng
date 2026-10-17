// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <linux/io.h>

#define NPCM_GPIOS_PER_BANK    32

/* Register offsets */
#define GPIO_DIN		0x4	/* RO - Data In */
#define GPIO_DOUT		0xC	/* RW - Data Out */
#define GPIO_OE			0x10	/* RW - Output Enable */
#define GPIO_IEM		0x58	/* RW - Input Enable Mask */
#define GPIO_OES		0x70	/* WO - Output Enable Register Set */
#define GPIO_OEC		0x74	/* WO - Output Enable Register Clear */

struct npcm_gpio_priv {
	void __iomem *base;
};

static int npcm_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	writel(BIT(offset), priv->base + GPIO_OEC);
	setbits_le32(priv->base + GPIO_IEM, BIT(offset));

	return 0;
}

static int npcm_gpio_direction_output(struct udevice *dev, unsigned int offset,
				      int value)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	if (value)
		setbits_le32(priv->base + GPIO_DOUT, BIT(offset));
	else
		clrbits_le32(priv->base + GPIO_DOUT, BIT(offset));

	clrbits_le32(priv->base + GPIO_IEM, BIT(offset));
	writel(BIT(offset), priv->base + GPIO_OES);

	return 0;
}

static int npcm_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + GPIO_IEM) & BIT(offset))
		return !!(readl(priv->base + GPIO_DIN) & BIT(offset));

	if (readl(priv->base + GPIO_OE) & BIT(offset))
		return !!(readl(priv->base + GPIO_DOUT) & BIT(offset));

	return -EINVAL;
}

static int npcm_gpio_set_value(struct udevice *dev, unsigned int offset,
			       int value)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	if (value)
		setbits_le32(priv->base + GPIO_DOUT, BIT(offset));
	else
		clrbits_le32(priv->base + GPIO_DOUT, BIT(offset));

	return 0;
}

static int npcm_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + GPIO_IEM) & BIT(offset))
		return GPIOF_INPUT;

	if (readl(priv->base + GPIO_OE) & BIT(offset))
		return GPIOF_OUTPUT;

	return GPIOF_FUNC;
}

static const struct dm_gpio_ops npcm_gpio_ops = {
	.direction_input	= npcm_gpio_direction_input,
	.direction_output	= npcm_gpio_direction_output,
	.get_value		= npcm_gpio_get_value,
	.set_value		= npcm_gpio_set_value,
	.get_function		= npcm_gpio_get_function,
};

static int npcm_gpio_probe(struct udevice *dev)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	uc_priv->gpio_count = NPCM_GPIOS_PER_BANK;
	uc_priv->bank_name = dev->name;

	return 0;
}

static const struct udevice_id npcm_gpio_match[] = {
	{ .compatible = "nuvoton,npcm845-gpio" },
	{ .compatible = "nuvoton,npcm750-gpio" },
	{ }
};

U_BOOT_DRIVER(npcm_gpio) = {
	.name	= "npcm_gpio",
	.id	= UCLASS_GPIO,
	.of_match = npcm_gpio_match,
	.probe	= npcm_gpio_probe,
	.priv_auto = sizeof(struct npcm_gpio_priv),
	.ops	= &npcm_gpio_ops,
};
