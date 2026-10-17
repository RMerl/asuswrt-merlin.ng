// SPDX-License-Identifier: GPL-2.0+
/*
 * SiFive GPIO driver
 *
 * Copyright (C) 2019 SiFive, Inc.
 */

#include <common.h>
#include <dm.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/gpio.h>
#include <linux/bitops.h>

static int sifive_gpio_probe(struct udevice *dev)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[18], *str;

	sprintf(name, "gpio@%4lx_", (uintptr_t)plat->base);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;

	/*
	 * Use the gpio count mentioned in device tree,
	 * if not specified in dt, set NR_GPIOS as default
	 */
	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", NR_GPIOS);

	return 0;
}

static void sifive_update_gpio_reg(void *bptr, u32 offset, bool value)
{
	void __iomem *ptr = (void __iomem *)bptr;

	u32 bit = BIT(offset);
	u32 old = readl(ptr);

	if (value)
		writel(old | bit, ptr);
	else
		writel(old & ~bit, ptr);
}

static int sifive_gpio_direction_input(struct udevice *dev, u32 offset)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	/* Configure gpio direction as input */
	sifive_update_gpio_reg(plat->base + GPIO_INPUT_EN,  offset, true);
	sifive_update_gpio_reg(plat->base + GPIO_OUTPUT_EN, offset, false);

	return 0;
}

static int sifive_gpio_direction_output(struct udevice *dev, u32 offset,
					int value)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	/* Configure gpio direction as output */
	sifive_update_gpio_reg(plat->base + GPIO_OUTPUT_EN, offset, true);
	sifive_update_gpio_reg(plat->base + GPIO_INPUT_EN,  offset, false);

	/* Set the output state of the pin */
	sifive_update_gpio_reg(plat->base + GPIO_OUTPUT_VAL, offset, value);

	return 0;
}

static int sifive_gpio_get_value(struct udevice *dev, u32 offset)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int val;
	int dir;

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	/* Get direction of the pin */
	dir = !(readl(plat->base + GPIO_OUTPUT_EN) & BIT(offset));

	if (dir)
		val = readl(plat->base + GPIO_INPUT_VAL) & BIT(offset);
	else
		val = readl(plat->base + GPIO_OUTPUT_VAL) & BIT(offset);

	return val ? HIGH : LOW;
}

static int sifive_gpio_set_value(struct udevice *dev, u32 offset, int value)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	sifive_update_gpio_reg(plat->base + GPIO_OUTPUT_VAL, offset, value);

	return 0;
}

static int sifive_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);
	u32	outdir, indir, val;
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -1;

	/* Get direction of the pin */
	outdir = readl(plat->base + GPIO_OUTPUT_EN) & BIT(offset);
	indir  = readl(plat->base + GPIO_INPUT_EN) & BIT(offset);

	if (outdir)
		/* Pin at specified offset is configured as output */
		val = GPIOF_OUTPUT;
	else if (indir)
		/* Pin at specified offset is configured as input */
		val = GPIOF_INPUT;
	else
		/*The requested GPIO is not set as input or output */
		val = GPIOF_UNUSED;

	return val;
}

static const struct udevice_id sifive_gpio_match[] = {
	{ .compatible = "sifive,gpio0" },
	{ }
};

static const struct dm_gpio_ops sifive_gpio_ops = {
	.direction_input        = sifive_gpio_direction_input,
	.direction_output       = sifive_gpio_direction_output,
	.get_value              = sifive_gpio_get_value,
	.set_value              = sifive_gpio_set_value,
	.get_function		= sifive_gpio_get_function,
};

static int sifive_gpio_of_to_plat(struct udevice *dev)
{
	struct sifive_gpio_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(gpio_sifive) = {
	.name	= "gpio_sifive",
	.id	= UCLASS_GPIO,
	.of_match = sifive_gpio_match,
	.of_to_plat = of_match_ptr(sifive_gpio_of_to_plat),
	.plat_auto	= sizeof(struct sifive_gpio_plat),
	.ops	= &sifive_gpio_ops,
	.probe	= sifive_gpio_probe,
};
