// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on arch/powerpc/include/asm/mpc85xx_gpio.h, which is
 *
 * Copyright 2010 eXMeritus, A Boeing Company
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>

struct ccsr_gpio {
	u32	gpdir;
	u32	gpodr;
	u32	gpdat;
	u32	gpier;
	u32	gpimr;
	u32	gpicr;
};

struct mpc8xxx_gpio_data {
	/* The bank's register base in memory */
	struct ccsr_gpio __iomem *base;
	/* The address of the registers; used to identify the bank */
	ulong addr;
	/* The GPIO count of the bank */
	uint gpio_count;
	/* The GPDAT register cannot be used to determine the value of output
	 * pins on MPC8572/MPC8536, so we shadow it and use the shadowed value
	 * for output pins
	 */
	u32 dat_shadow;
	ulong type;
};

enum {
	MPC8XXX_GPIO_TYPE,
	MPC5121_GPIO_TYPE,
};

inline u32 gpio_mask(uint gpio)
{
	return (1U << (31 - (gpio)));
}

static inline u32 mpc8xxx_gpio_get_val(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpdat) & mask;
}

static inline u32 mpc8xxx_gpio_get_dir(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpdir) & mask;
}

static inline void mpc8xxx_gpio_set_in(struct ccsr_gpio *base, u32 gpios)
{
	clrbits_be32(&base->gpdat, gpios);
	/* GPDIR register 0 -> input */
	clrbits_be32(&base->gpdir, gpios);
}

static inline void mpc8xxx_gpio_set_low(struct ccsr_gpio *base, u32 gpios)
{
	clrbits_be32(&base->gpdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be32(&base->gpdir, gpios);
}

static inline void mpc8xxx_gpio_set_high(struct ccsr_gpio *base, u32 gpios)
{
	setbits_be32(&base->gpdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be32(&base->gpdir, gpios);
}

static inline int mpc8xxx_gpio_open_drain_val(struct ccsr_gpio *base, u32 mask)
{
	return in_be32(&base->gpodr) & mask;
}

static inline void mpc8xxx_gpio_open_drain_on(struct ccsr_gpio *base, u32
					      gpios)
{
	/* GPODR register 1 -> open drain on */
	setbits_be32(&base->gpodr, gpios);
}

static inline void mpc8xxx_gpio_open_drain_off(struct ccsr_gpio *base,
					       u32 gpios)
{
	/* GPODR register 0 -> open drain off (actively driven) */
	clrbits_be32(&base->gpodr, gpios);
}

static int mpc8xxx_gpio_direction_input(struct udevice *dev, uint gpio)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	mpc8xxx_gpio_set_in(data->base, gpio_mask(gpio));
	return 0;
}

static int mpc8xxx_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (value) {
		data->dat_shadow |= gpio_mask(gpio);
		mpc8xxx_gpio_set_high(data->base, gpio_mask(gpio));
	} else {
		data->dat_shadow &= ~gpio_mask(gpio);
		mpc8xxx_gpio_set_low(data->base, gpio_mask(gpio));
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

	if (!!mpc8xxx_gpio_get_dir(data->base, gpio_mask(gpio))) {
		/* Output -> use shadowed value */
		return !!(data->dat_shadow & gpio_mask(gpio));
	}

	/* Input -> read value from GPDAT register */
	return !!mpc8xxx_gpio_get_val(data->base, gpio_mask(gpio));
}

static int mpc8xxx_gpio_get_open_drain(struct udevice *dev, uint gpio)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	return !!mpc8xxx_gpio_open_drain_val(data->base, gpio_mask(gpio));
}

static int mpc8xxx_gpio_set_open_drain(struct udevice *dev, uint gpio,
				       int value)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);

	if (value)
		mpc8xxx_gpio_open_drain_on(data->base, gpio_mask(gpio));
	else
		mpc8xxx_gpio_open_drain_off(data->base, gpio_mask(gpio));

	return 0;
}

static int mpc8xxx_gpio_get_function(struct udevice *dev, uint gpio)
{
	struct mpc8xxx_gpio_data *data = dev_get_priv(dev);
	int dir;

	dir = !!mpc8xxx_gpio_get_dir(data->base, gpio_mask(gpio));
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int mpc8xxx_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct mpc8xxx_gpio_plat *plat = dev_get_platdata(dev);
	fdt_addr_t addr;
	u32 reg[2];

	dev_read_u32_array(dev, "reg", reg, 2);
	addr = dev_translate_address(dev, reg);

	plat->addr = addr;
	plat->size = reg[1];
	plat->ngpios = dev_read_u32_default(dev, "ngpios", 32);

	return 0;
}
#endif

static int mpc8xxx_gpio_platdata_to_priv(struct udevice *dev)
{
	struct mpc8xxx_gpio_data *priv = dev_get_priv(dev);
	struct mpc8xxx_gpio_plat *plat = dev_get_platdata(dev);
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

	mpc8xxx_gpio_platdata_to_priv(dev);

	snprintf(name, sizeof(name), "MPC@%lx_", data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = data->gpio_count;

	return 0;
}

static const struct dm_gpio_ops gpio_mpc8xxx_ops = {
	.direction_input	= mpc8xxx_gpio_direction_input,
	.direction_output	= mpc8xxx_gpio_direction_output,
	.get_value		= mpc8xxx_gpio_get_value,
	.set_value		= mpc8xxx_gpio_set_value,
	.get_open_drain		= mpc8xxx_gpio_get_open_drain,
	.set_open_drain		= mpc8xxx_gpio_set_open_drain,
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
	.ofdata_to_platdata = mpc8xxx_gpio_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mpc8xxx_gpio_plat),
	.of_match = mpc8xxx_gpio_ids,
#endif
	.probe	= mpc8xxx_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct mpc8xxx_gpio_data),
};
