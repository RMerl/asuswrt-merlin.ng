// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on arch/powerpc/include/asm/mpc85xx_gpio.h, which is
 *
 * Copyright 2010 eXMeritus, A Boeing Company
 * Copyright 2020-2021 NXP
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/of_access.h>

struct mpc8xxx_gpio_data {
	/* The bank's register base in memory */
	struct ccsr_gpio __iomem *base;
	/* The address of the registers; used to identify the bank */
	phys_addr_t addr;
	/* The GPIO count of the bank */
	uint gpio_count;
	/* The GPDAT register cannot be used to determine the value of output
	 * pins on MPC8572/MPC8536, so we shadow it and use the shadowed value
	 * for output pins
	 */
	u32 dat_shadow;
	ulong type;
	bool  little_endian;
};

enum {
	MPC8XXX_GPIO_TYPE,
	MPC5121_GPIO_TYPE,
};

inline u32 gpio_mask(uint gpio)
{
	return (1U << (31 - (gpio)));
}

static inline u32 mpc8xxx_gpio_get_val(struct udevice *dev, u32 mask)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (data->little_endian)
		return in_le32(&data->base->gpdat) & mask;
	else
		return in_be32(&data->base->gpdat) & mask;
}

static inline u32 mpc8xxx_gpio_get_dir(struct udevice *dev, u32 mask)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (data->little_endian)
		return in_le32(&data->base->gpdir) & mask;
	else
		return in_be32(&data->base->gpdir) & mask;
}

static inline int mpc8xxx_gpio_open_drain_val(struct udevice *dev, u32 mask)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (data->little_endian)
		return in_le32(&data->base->gpodr) & mask;
	else
		return in_be32(&data->base->gpodr) & mask;
}

static inline void mpc8xxx_gpio_open_drain_on(struct udevice *dev, u32
					      gpios)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	/* GPODR register 1 -> open drain on */
	if (data->little_endian)
		setbits_le32(&data->base->gpodr, gpios);
	else
		setbits_be32(&data->base->gpodr, gpios);
}

static inline void mpc8xxx_gpio_open_drain_off(struct udevice *dev,
					       u32 gpios)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	/* GPODR register 0 -> open drain off (actively driven) */
	if (data->little_endian)
		clrbits_le32(&data->base->gpodr, gpios);
	else
		clrbits_be32(&data->base->gpodr, gpios);
}

static int mpc8xxx_gpio_direction_input(struct udevice *dev, uint gpio)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	u32 mask = gpio_mask(gpio);

	/* GPDIR register 0 -> input */
	if (data->little_endian)
		clrbits_le32(&data->base->gpdir, mask);
	else
		clrbits_be32(&data->base->gpdir, mask);

	return 0;
}

static int mpc8xxx_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	struct ccsr_gpio *base = data->base;
	u32 mask = gpio_mask(gpio);
	u32 gpdir;

	if (value) {
		data->dat_shadow |= mask;
	} else {
		data->dat_shadow &= ~mask;
	}

	if (data->little_endian)
		gpdir = in_le32(&base->gpdir);
	else
		gpdir = in_be32(&base->gpdir);

	gpdir |= gpio_mask(gpio);

	if (data->little_endian) {
		out_le32(&base->gpdat, gpdir & data->dat_shadow);
		out_le32(&base->gpdir, gpdir);
	} else {
		out_be32(&base->gpdat, gpdir & data->dat_shadow);
		out_be32(&base->gpdir, gpdir);
	}

	return 0;
}

static int mpc8xxx_gpio_direction_output(struct udevice *dev, uint gpio,
					 int value)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	/* GPIO 28..31 are input only on MPC5121 */
	if (data->type == MPC5121_GPIO_TYPE && gpio >= 28)
		return -EINVAL;

	return mpc8xxx_gpio_set_value(dev, gpio, value);
}

static int mpc8xxx_gpio_get_value(struct udevice *dev, uint gpio)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (!!mpc8xxx_gpio_get_dir(dev, gpio_mask(gpio))) {
		/* Output -> use shadowed value */
		return !!(data->dat_shadow & gpio_mask(gpio));
	}

	/* Input -> read value from GPDAT register */
	return !!mpc8xxx_gpio_get_val(dev, gpio_mask(gpio));
}

static int mpc8xxx_gpio_get_function(struct udevice *dev, uint gpio)
{
	int dir;

	dir = !!mpc8xxx_gpio_get_dir(dev, gpio_mask(gpio));
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int mpc8xxx_gpio_of_to_plat(struct udevice *dev)
{
	struct mpc8xxx_gpio_plat *plat = dev_get_plat(dev);
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (dev_read_bool(dev, "little-endian"))
		data->little_endian = true;

	plat->addr = dev_read_addr_size_index(dev, 0, (fdt_size_t *)&plat->size);
	plat->ngpios = dev_read_u32_default(dev, "ngpios", 32);

	return 0;
}
#endif

static int mpc8xxx_gpio_plat_to_priv(struct udevice *dev)
{
	struct mpc8xxx_gpio_data *priv = dev_get_priv(dev);
	struct mpc8xxx_gpio_plat *plat = dev_get_plat(dev);
	unsigned long size = plat->size;
	ulong driver_data = dev_get_driver_data(dev);

	if (size == 0)
		size = 0x100;

	priv->addr = plat->addr;
	priv->base = map_sysmem(plat->addr, size);

	if (!priv->base)
		return -ENOMEM;

	priv->gpio_count = plat->ngpios;
	priv->dat_shadow = 0;

	priv->type = driver_data;

	return 0;
}

static int mpc8xxx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	char name[32], *str;

	mpc8xxx_gpio_plat_to_priv(dev);

	snprintf(name, sizeof(name), "MPC@%.8llx",
		 (unsigned long long)data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	if (device_is_compatible(dev, "fsl,qoriq-gpio")) {
		if (data->little_endian)
			out_le32(&data->base->gpibe, 0xffffffff);
		else
			out_be32(&data->base->gpibe, 0xffffffff);
	}

	uc_priv->bank_name = str;
	uc_priv->gpio_count = data->gpio_count;

	return 0;
}

static const struct dm_gpio_ops gpio_mpc8xxx_ops = {
	.direction_input	= mpc8xxx_gpio_direction_input,
	.direction_output	= mpc8xxx_gpio_direction_output,
	.get_value		= mpc8xxx_gpio_get_value,
	.set_value		= mpc8xxx_gpio_set_value,
	.get_function		= mpc8xxx_gpio_get_function,
};

static const struct udevice_id mpc8xxx_gpio_ids[] = {
	{ .compatible = "fsl,pq3-gpio", .data = MPC8XXX_GPIO_TYPE },
	{ .compatible = "fsl,mpc8308-gpio", .data = MPC8XXX_GPIO_TYPE },
	{ .compatible = "fsl,mpc8349-gpio", .data = MPC8XXX_GPIO_TYPE },
	{ .compatible = "fsl,mpc8572-gpio", .data = MPC8XXX_GPIO_TYPE},
	{ .compatible = "fsl,mpc8610-gpio", .data = MPC8XXX_GPIO_TYPE},
	{ .compatible = "fsl,mpc5121-gpio", .data = MPC5121_GPIO_TYPE, },
	{ .compatible = "fsl,qoriq-gpio", .data = MPC8XXX_GPIO_TYPE },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_mpc8xxx) = {
	.name	= "gpio_mpc8xxx",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mpc8xxx_ops,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_to_plat = mpc8xxx_gpio_of_to_plat,
	.plat_auto	= sizeof(struct mpc8xxx_gpio_plat),
	.of_match = mpc8xxx_gpio_ids,
#endif
	.probe	= mpc8xxx_gpio_probe,
	.priv_auto	= sizeof(struct mpc8xxx_gpio_data),
};
