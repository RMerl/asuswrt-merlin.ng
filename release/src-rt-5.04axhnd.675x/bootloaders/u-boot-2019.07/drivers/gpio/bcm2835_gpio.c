// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Vikram Narayananan
 * <vikram186@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <fdtdec.h>

struct bcm2835_gpios {
	struct bcm2835_gpio_regs *reg;
	struct udevice *pinctrl;
};

static int bcm2835_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	unsigned val;

	val = readl(&gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_INPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

static int bcm2835_gpio_direction_output(struct udevice *dev, unsigned int gpio,
					 int value)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	unsigned val;

	gpio_set_value(gpio, value);

	val = readl(&gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_OUTPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

static int bcm2835_get_value(const struct bcm2835_gpios *gpios, unsigned gpio)
{
	unsigned val;

	val = readl(&gpios->reg->gplev[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return (val >> BCM2835_GPIO_COMMON_SHIFT(gpio)) & 0x1;
}

static int bcm2835_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	const struct bcm2835_gpios *gpios = dev_get_priv(dev);

	return bcm2835_get_value(gpios, gpio);
}

static int bcm2835_gpio_set_value(struct udevice *dev, unsigned gpio,
				  int value)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	u32 *output_reg = value ? gpios->reg->gpset : gpios->reg->gpclr;

	writel(1 << BCM2835_GPIO_COMMON_SHIFT(gpio),
				&output_reg[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return 0;
}

static int bcm2835_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct bcm2835_gpios *priv = dev_get_priv(dev);
	int funcid;

	funcid = pinctrl_get_gpio_mux(priv->pinctrl, 0, offset);

	switch (funcid) {
	case BCM2835_GPIO_OUTPUT:
		return GPIOF_OUTPUT;
	case BCM2835_GPIO_INPUT:
		return GPIOF_INPUT;
	default:
		return GPIOF_FUNC;
	}
}

static const struct dm_gpio_ops gpio_bcm2835_ops = {
	.direction_input	= bcm2835_gpio_direction_input,
	.direction_output	= bcm2835_gpio_direction_output,
	.get_value		= bcm2835_gpio_get_value,
	.set_value		= bcm2835_gpio_set_value,
	.get_function		= bcm2835_gpio_get_function,
};

static int bcm2835_gpio_probe(struct udevice *dev)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	struct bcm2835_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = "GPIO";
	uc_priv->gpio_count = BCM2835_GPIO_COUNT;
	gpios->reg = (struct bcm2835_gpio_regs *)plat->base;

	/* We know we're spawned by the pinctrl driver */
	gpios->pinctrl = dev->parent;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int bcm2835_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct bcm2835_gpio_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	return 0;
}
#endif

U_BOOT_DRIVER(gpio_bcm2835) = {
	.name	= "gpio_bcm2835",
	.id	= UCLASS_GPIO,
	.ofdata_to_platdata = of_match_ptr(bcm2835_gpio_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct bcm2835_gpio_platdata),
	.ops	= &gpio_bcm2835_ops,
	.probe	= bcm2835_gpio_probe,
	.flags	= DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct bcm2835_gpios),
};
