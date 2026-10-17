// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 CR GROUP France
 * Christophe Leroy <christophe.leroy@csgroup.eu>
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>
#include <asm/immap_83xx.h>
#include <asm/io.h>
#include <dm/of_access.h>

#define QE_DIR_NONE	0
#define QE_DIR_OUT	1
#define QE_DIR_IN	2
#define QE_DIR_IN_OUT	3

struct qe_gpio_data {
	/* The bank's register base in memory */
	struct gpio_n __iomem *base;
	/* The address of the registers; used to identify the bank */
	phys_addr_t addr;
};

static inline u32 gpio_mask(uint gpio)
{
	return 1U << (31 - (gpio));
}

static inline u32 gpio_mask2(uint gpio)
{
	return 1U << (30 - ((gpio & 15) << 1));
}

static int qe_gpio_direction_input(struct udevice *dev, uint gpio)
{
	struct qe_gpio_data *data = dev_get_priv(dev);
	struct gpio_n __iomem *base = data->base;
	u32 mask2 = gpio_mask2(gpio);

	if (gpio < 16)
		clrsetbits_be32(&base->dir1, mask2 * QE_DIR_OUT, mask2 * QE_DIR_IN);
	else
		clrsetbits_be32(&base->dir2, mask2 * QE_DIR_OUT, mask2 * QE_DIR_IN);

	return 0;
}

static int qe_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	struct qe_gpio_data *data = dev_get_priv(dev);
	struct gpio_n __iomem *base = data->base;
	u32 mask = gpio_mask(gpio);
	u32 mask2 = gpio_mask2(gpio);

	if (gpio < 16)
		clrsetbits_be32(&base->dir1, mask2 * QE_DIR_IN, mask2 * QE_DIR_OUT);
	else
		clrsetbits_be32(&base->dir2, mask2 * QE_DIR_IN, mask2 * QE_DIR_OUT);

	if (value)
		setbits_be32(&base->pdat, mask);
	else
		clrbits_be32(&base->pdat, mask);

	return 0;
}

static int qe_gpio_get_value(struct udevice *dev, uint gpio)
{
	struct qe_gpio_data *data = dev_get_priv(dev);
	struct gpio_n __iomem *base = data->base;
	u32 mask = gpio_mask(gpio);

	return !!(in_be32(&base->pdat) & mask);
}

static int qe_gpio_get_function(struct udevice *dev, uint gpio)
{
	struct qe_gpio_data *data = dev_get_priv(dev);
	struct gpio_n __iomem *base = data->base;
	u32 mask2 = gpio_mask2(gpio);
	int dir;

	if (gpio < 16)
		dir = in_be32(&base->dir1);
	else
		dir = in_be32(&base->dir2);

	if ((dir & (mask2 * QE_DIR_IN_OUT)) == (mask2 & QE_DIR_IN))
		return GPIOF_INPUT;
	else if ((dir & (mask2 * QE_DIR_IN_OUT)) == (mask2 & QE_DIR_OUT))
		return GPIOF_OUTPUT;
	else
		return GPIOF_UNKNOWN;
}

static int qe_gpio_of_to_plat(struct udevice *dev)
{
	struct qe_gpio_plat *plat = dev_get_plat(dev);

	plat->addr = dev_read_addr_size_index(dev, 0, (fdt_size_t *)&plat->size);

	return 0;
}

static int qe_gpio_plat_to_priv(struct udevice *dev)
{
	struct qe_gpio_data *priv = dev_get_priv(dev);
	struct qe_gpio_plat *plat = dev_get_plat(dev);
	unsigned long size = plat->size;

	if (size == 0)
		size = sizeof(struct gpio_n);

	priv->addr = plat->addr;
	priv->base = (void __iomem *)plat->addr;

	if (!priv->base)
		return -ENOMEM;

	return 0;
}

static int qe_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct qe_gpio_data *data = dev_get_priv(dev);
	char name[32], *str;

	qe_gpio_plat_to_priv(dev);

	snprintf(name, sizeof(name), "QE@%.8llx",
		 (unsigned long long)data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = 32;

	return 0;
}

static const struct dm_gpio_ops gpio_qe_ops = {
	.direction_input	= qe_gpio_direction_input,
	.direction_output	= qe_gpio_set_value,
	.get_value		= qe_gpio_get_value,
	.set_value		= qe_gpio_set_value,
	.get_function		= qe_gpio_get_function,
};

static const struct udevice_id qe_gpio_ids[] = {
	{ .compatible = "fsl,mpc8323-qe-pario-bank"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_qe) = {
	.name	= "gpio_qe",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_qe_ops,
	.of_to_plat = qe_gpio_of_to_plat,
	.plat_auto	= sizeof(struct qe_gpio_plat),
	.of_match = qe_gpio_ids,
	.probe	= qe_gpio_probe,
	.priv_auto	= sizeof(struct qe_gpio_data),
};
