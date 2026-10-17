// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2019 Stephan Gerhold */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>

struct nmk_gpio_regs {
	u32 dat;	/* data */
	u32 dats;	/* data set */
	u32 datc;	/* data clear */
	u32 pdis;	/* pull disable */
	u32 dir;	/* direction */
	u32 dirs;	/* direction set */
	u32 dirc;	/* direction clear */
	u32 slpm;	/* sleep mode */
	u32 afsla;	/* alternate function select A */
	u32 afslb;	/* alternate function select B */
	u32 lowemi;	/* low EMI mode */
};

struct nmk_gpio {
	struct nmk_gpio_regs *regs;
};

static int nmk_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct nmk_gpio *priv = dev_get_priv(dev);

	return !!(readl(&priv->regs->dat) & BIT(offset));
}

static int nmk_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	struct nmk_gpio *priv = dev_get_priv(dev);

	if (value)
		writel(BIT(offset), &priv->regs->dats);
	else
		writel(BIT(offset), &priv->regs->datc);

	return 0;
}

static int nmk_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct nmk_gpio *priv = dev_get_priv(dev);

	if (readl(&priv->regs->afsla) & BIT(offset) ||
	    readl(&priv->regs->afslb) & BIT(offset))
		return GPIOF_FUNC;

	if (readl(&priv->regs->dir) & BIT(offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int nmk_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct nmk_gpio *priv = dev_get_priv(dev);

	writel(BIT(offset), &priv->regs->dirc);
	return 0;
}

static int nmk_gpio_direction_output(struct udevice *dev, unsigned offset,
				     int value)
{
	struct nmk_gpio *priv = dev_get_priv(dev);

	writel(BIT(offset), &priv->regs->dirs);
	return nmk_gpio_set_value(dev, offset, value);
}

static const struct dm_gpio_ops nmk_gpio_ops = {
	.get_value		= nmk_gpio_get_value,
	.set_value		= nmk_gpio_set_value,
	.get_function		= nmk_gpio_get_function,
	.direction_input	= nmk_gpio_direction_input,
	.direction_output	= nmk_gpio_direction_output,
};

static int nmk_gpio_probe(struct udevice *dev)
{
	struct nmk_gpio *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char buf[20];
	u32 bank;
	int ret;

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	ret = dev_read_u32(dev, "gpio-bank", &bank);
	if (ret < 0) {
		printf("nmk_gpio(%s): Failed to read gpio-bank\n", dev->name);
		return ret;
	}

	sprintf(buf, "nmk%u-gpio", bank);
	uc_priv->bank_name = strdup(buf);
	if (!uc_priv->bank_name)
		return -ENOMEM;

	uc_priv->gpio_count = sizeof(priv->regs->dat) * BITS_PER_BYTE;

	return 0;
}

static const struct udevice_id nmk_gpio_ids[] = {
	{ .compatible = "st,nomadik-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_nmk) = {
	.name		= "nmk_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= nmk_gpio_ids,
	.probe		= nmk_gpio_probe,
	.ops		= &nmk_gpio_ops,
	.priv_auto	= sizeof(struct nmk_gpio),
};
