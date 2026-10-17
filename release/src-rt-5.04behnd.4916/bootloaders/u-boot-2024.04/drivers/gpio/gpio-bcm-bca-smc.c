// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Broadcom
 */
/*
 *
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

struct bcmbca_gpio_priv {
	void __iomem *reg_base;
	int	ngpios;
};

/**
 * enum gio_reg_index - GIO block GPIO register index.
 * @GIO_REG_ODEN: Open drain enable register.
 * @GIO_REG_DATA: Data register.
 * @GIO_REG_IODIR: Input/output direction register.
 * @GIO_REG_EC: Edge insensitive register.
 * @GIO_REG_MASK: Interrupt mask register.
 * @GIO_REG_LEVEL: Interrupt level type register.
 * @GIO_REG_STAT: Interrupt status register.
 * @GIO_REG_COUNT: Total GIO GPIO register count.
 */
enum gio_reg_index {
	GIO_REG_ODEN = 0,
	GIO_REG_DATA,
	GIO_REG_IODIR,
	GIO_REG_EC,
	GIO_REG_EI,
	GIO_REG_MASK,
	GIO_REG_LEVEL,
	GIO_REG_STAT,
	GIO_REG_COUNT
};

#define GIO_BANK_SIZE	(GIO_REG_COUNT * sizeof(u32))
#define GIO_BANK_OFF(bank, off)	(((bank) * GIO_BANK_SIZE) + (off * sizeof(u32)))
#define GIO_ODEN(bank)	GIO_BANK_OFF(bank, GIO_REG_ODEN)
#define GIO_DATA(bank)	GIO_BANK_OFF(bank, GIO_REG_DATA)
#define GIO_IODIR(bank)	GIO_BANK_OFF(bank, GIO_REG_IODIR)
#define GIO_EC(bank)	GIO_BANK_OFF(bank, GIO_REG_EC)
#define GIO_EI(bank)	GIO_BANK_OFF(bank, GIO_REG_EI)
#define GIO_MASK(bank)	GIO_BANK_OFF(bank, GIO_REG_MASK)
#define GIO_LEVEL(bank)	GIO_BANK_OFF(bank, GIO_REG_LEVEL)
#define GIO_STAT(bank)	GIO_BANK_OFF(bank, GIO_REG_STAT)

#define MAX_GPIO_PER_BANK	32
#define GPIO_BANK(gpio)	((gpio) >> 5)

static int bcmbca_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned int bank_id = gpio / MAX_GPIO_PER_BANK;
	unsigned int offset = gpio - (MAX_GPIO_PER_BANK * bank_id);

	if (gpio >= priv->ngpios)
		return -EINVAL;

	val = readl(priv->reg_base + GIO_IODIR(bank_id));
	val |= (1 << offset);
	writel(val, priv->reg_base + GIO_IODIR(bank_id));

	return 0;
}

static int bcmbca_gpio_direction_output(struct udevice *dev, unsigned int gpio, int value)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned int bank_id = gpio / MAX_GPIO_PER_BANK;
	unsigned int offset = gpio - (MAX_GPIO_PER_BANK * bank_id);

	if (gpio >= priv->ngpios)
		return -EINVAL;

	val = readl(priv->reg_base + GIO_IODIR(bank_id));
	val &= ~(1 << offset);
	writel(val, priv->reg_base + GIO_IODIR(bank_id));

	val = readl(priv->reg_base + GIO_DATA(bank_id));
	if (value)
		val |= (1 << offset);
	else
		val &= ~(1 << offset);

	writel(val, priv->reg_base + GIO_DATA(bank_id));

	return 0;
}

static int bcmbca_gpio_get_direction(struct udevice *dev, unsigned int gpio)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned int bank_id = gpio / MAX_GPIO_PER_BANK;
	unsigned int offset = gpio - (MAX_GPIO_PER_BANK * bank_id);

	if (gpio >= priv->ngpios)
		return -EINVAL;

	val = readl(priv->reg_base + GIO_IODIR(bank_id));
	val &= (1 << offset);
	val >>= offset;

	return val ? GPIOF_INPUT : GPIOF_OUTPUT;
}

static int bcmbca_gpio_get(struct udevice *dev, unsigned int gpio)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned int bank_id = gpio / MAX_GPIO_PER_BANK;
	unsigned int offset = gpio - (MAX_GPIO_PER_BANK * bank_id);

	if (gpio >= priv->ngpios)
		return -EINVAL;

	val = readl(priv->reg_base + GIO_DATA(bank_id));
	val &= (1 << offset);
	val >>= offset;

	return val;
}

static int bcmbca_gpio_set(struct udevice *dev, unsigned int gpio, int value)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);
	unsigned int val = 0;
	unsigned int bank_id = gpio / MAX_GPIO_PER_BANK;
	unsigned int offset = gpio - (MAX_GPIO_PER_BANK * bank_id);

	if (gpio >= priv->ngpios)
		return -EINVAL;

	val = readl(priv->reg_base + GIO_DATA(bank_id));
	if (value)
		val |= (1 << offset);
	else
		val &= ~(1 << offset);

	writel(val, priv->reg_base + GIO_DATA(bank_id));

	return 0;
}

static int bcmbca_gpio_request(struct udevice *dev, unsigned int offset, const char *label)
{
	struct bcmbca_gpio_priv *priv = dev_get_priv(dev);

	if (offset >= priv->ngpios)
		return -EINVAL;

	return pinctrl_gpio_request(dev, offset, label);
}

static const struct dm_gpio_ops bcmbca_gpio_ops = {
	.request = bcmbca_gpio_request,
	.get_function = bcmbca_gpio_get_direction,
	.direction_input = bcmbca_gpio_direction_input,
	.direction_output = bcmbca_gpio_direction_output,
	.get_value = bcmbca_gpio_get,
	.set_value = bcmbca_gpio_set,
};

static const struct udevice_id bcmbca_gpio_of_match[] = {
	{ .compatible = "brcm,bca-gpio-smc" },
	{}
};

static int bcmbca_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *chip = dev_get_uclass_priv(dev);
	struct bcmbca_gpio_priv *bca_gpio = dev_get_priv(dev);
	int bank_width;
	int i;

	bca_gpio->ngpios = 0;

	bca_gpio->reg_base = dev_remap_addr_index(dev, 0);
	if (!bca_gpio->reg_base)
		return -EINVAL;

	for (i = 0; ; i++) {

		if (dev_read_u32_index(dev, "brcm,gpio-bank-widths", i, &bank_width))
			break;

		if (!bank_width)
			return -EINVAL;

		bca_gpio->ngpios += bank_width;
	}
	chip->gpio_base = 0;
	chip->gpio_count = bca_gpio->ngpios;
	chip->bank_name = dev->name;

	return 0;
}

U_BOOT_DRIVER(bcmbca_gpio) = {
	.name = "bcm-bca-gpio-smc",
	.id = UCLASS_GPIO,
	.of_match = bcmbca_gpio_of_match,
	.ops = &bcmbca_gpio_ops,
	.priv_auto = sizeof(struct bcmbca_gpio_priv),
	.probe = bcmbca_gpio_probe,
};

