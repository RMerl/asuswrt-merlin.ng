// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/bitops.h>

#define MVEBU_GPIOS_PER_BANK	32

struct mvebu_gpio_regs {
	u32 data_out;
	u32 io_conf;
	u32 blink_en;
	u32 in_pol;
	u32 data_in;
};

struct mvebu_gpio_priv {
	struct mvebu_gpio_regs *regs;
	char name[sizeof("mvebuX_")];
};

static int mvebu_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);
	struct mvebu_gpio_regs *regs = priv->regs;

	setbits_le32(&regs->io_conf, BIT(gpio));

	return 0;
}

static int mvebu_gpio_direction_output(struct udevice *dev, unsigned gpio,
				       int value)
{
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);
	struct mvebu_gpio_regs *regs = priv->regs;

	if (value)
		setbits_le32(&regs->data_out, BIT(gpio));
	else
		clrbits_le32(&regs->data_out, BIT(gpio));
	clrbits_le32(&regs->io_conf, BIT(gpio));

	return 0;
}

static int mvebu_gpio_get_function(struct udevice *dev, unsigned gpio)
{
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);
	struct mvebu_gpio_regs *regs = priv->regs;
	u32 val;

	val = readl(&regs->io_conf) & BIT(gpio);
	if (val)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static int mvebu_gpio_set_value(struct udevice *dev, unsigned gpio,
				int value)
{
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);
	struct mvebu_gpio_regs *regs = priv->regs;

	if (value)
		setbits_le32(&regs->data_out, BIT(gpio));
	else
		clrbits_le32(&regs->data_out, BIT(gpio));

	return 0;
}

static int mvebu_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);
	struct mvebu_gpio_regs *regs = priv->regs;

	return !!(readl(&regs->data_in) & BIT(gpio));
}

static int mvebu_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mvebu_gpio_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", MVEBU_GPIOS_PER_BANK);
	sprintf(priv->name, "mvebu%d_", dev_seq(dev));
	uc_priv->bank_name = priv->name;

	return 0;
}

static const struct dm_gpio_ops mvebu_gpio_ops = {
#if CONFIG_IS_ENABLED(PINCTRL_ARMADA_38X)
	.request		= pinctrl_gpio_request,
	.rfree			= pinctrl_gpio_free,
#endif
	.direction_input	= mvebu_gpio_direction_input,
	.direction_output	= mvebu_gpio_direction_output,
	.get_function		= mvebu_gpio_get_function,
	.get_value		= mvebu_gpio_get_value,
	.set_value		= mvebu_gpio_set_value,
};

static const struct udevice_id mvebu_gpio_ids[] = {
	{ .compatible = "marvell,orion-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_mvebu) = {
	.name			= "gpio_mvebu",
	.id			= UCLASS_GPIO,
	.of_match		= mvebu_gpio_ids,
	.ops			= &mvebu_gpio_ops,
	.probe			= mvebu_gpio_probe,
	.priv_auto	= sizeof(struct mvebu_gpio_priv),
};
