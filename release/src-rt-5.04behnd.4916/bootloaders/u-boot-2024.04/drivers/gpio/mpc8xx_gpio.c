// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 CS Group
 *	Charles Frey <charles.frey@c-s.fr>
 *
 * based on driver/gpio/mpc8xxx_gpio.c, which is
 * Copyright 2016 Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on arch/powerpc/include/asm/mpc85xx_gpio.h, which is
 * Copyright 2010 eXMeritus, A Boeing Company
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>
#include <malloc.h>

enum {
	MPC8XX_CPM1_PORTA,
	MPC8XX_CPM1_PORTB,
	MPC8XX_CPM1_PORTC,
	MPC8XX_CPM1_PORTD,
	MPC8XX_CPM1_PORTE,
};

/*
 * The MPC885 CPU CPM has 5 I/O ports, and each ports has different
 * register length : 16 bits for ports A,C,D and 32 bits for ports
 * B and E.
 *
 * This structure allows us to select the accessors according to the
 * port we are configuring.
 */
struct mpc8xx_gpio_data {
	/* The bank's register base in memory */
	void __iomem *base;
	/* The address of the registers; used to identify the bank */
	ulong addr;
	/* The GPIO count of the bank */
	uint gpio_count;
	/* Type needed to use the correct accessors */
	int type;
};

/* Structure for ports A, C, D */
struct iop_16 {
	u16 pdir;
	u16 ppar;
	u16 podr;
	u16 pdat;
};

/* Port B */
struct iop_32_b {
	u32 pdir;
	u32 ppar;
	u32 podr;
	u32 pdat;
};

/* Port E */
struct iop_32_e {
	u32 pdir;
	u32 ppar;
	u32 psor;
	u32 podr;
	u32 pdat;
};

union iop_32 {
	struct iop_32_b b;
	struct iop_32_e e;
};

inline u32 gpio_mask(uint gpio, int type)
{
	if (type == MPC8XX_CPM1_PORTB || type == MPC8XX_CPM1_PORTE)
		return 1U << (31 - (gpio));
	else
		return 1U << (15 - (gpio));
}

static inline u16 gpio16_get_val(void __iomem *base, u16 mask, int type)
{
	struct iop_16 *regs = base;

	return in_be16(&regs->pdat) & mask;
}

static inline u16 gpio16_get_dir(void __iomem *base, u16 mask, int type)
{
	struct iop_16 *regs = base;

	return in_be16(&regs->pdir) & mask;
}

static inline void gpio16_set_in(void __iomem *base, u16 gpios, int type)
{
	struct iop_16 *regs = base;

	clrbits_be16(&regs->pdat, gpios);
	/* GPDIR register 0 -> input */
	clrbits_be16(&regs->pdir, gpios);
}

static inline void gpio16_set_lo(void __iomem *base, u16 gpios, int type)
{
	struct iop_16 *regs = base;

	clrbits_be16(&regs->pdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be16(&regs->pdir, gpios);
}

static inline void gpio16_set_hi(void __iomem *base, u16 gpios, int type)
{
	struct iop_16 *regs = base;

	setbits_be16(&regs->pdat, gpios);
	/* GPDIR register 1 -> output */
	setbits_be16(&regs->pdir, gpios);
}

/* PORT B AND E */
static inline u32 gpio32_get_val(void __iomem *base, u32 mask, int type)
{
	union iop_32 __iomem *regs = base;

	if (type == MPC8XX_CPM1_PORTB)
		return in_be32(&regs->b.pdat) & mask;
	else
		return in_be32(&regs->e.pdat) & mask;
}

static inline u32 gpio32_get_dir(void __iomem *base, u32 mask, int type)
{
	union iop_32 __iomem *regs = base;

	if (type == MPC8XX_CPM1_PORTB)
		return in_be32(&regs->b.pdir) & mask;
	else
		return in_be32(&regs->e.pdir) & mask;
}

static inline void gpio32_set_in(void __iomem *base, u32 gpios, int type)
{
	union iop_32 __iomem *regs = base;

	if (type == MPC8XX_CPM1_PORTB) {
		clrbits_be32(&regs->b.pdat, gpios);
		/* GPDIR register 0 -> input */
		clrbits_be32(&regs->b.pdir, gpios);
	} else { /* Port E */
		clrbits_be32(&regs->e.pdat, gpios);
		/* GPDIR register 0 -> input */
		clrbits_be32(&regs->e.pdir, gpios);
	}
}

static inline void gpio32_set_lo(void __iomem *base, u32 gpios, int type)
{
	union iop_32 __iomem *regs = base;

	if (type == MPC8XX_CPM1_PORTB) {
		clrbits_be32(&regs->b.pdat, gpios);
		/* GPDIR register 1 -> output */
		setbits_be32(&regs->b.pdir, gpios);
	} else {
		clrbits_be32(&regs->e.pdat, gpios);
		/* GPDIR register 1 -> output */
		setbits_be32(&regs->e.pdir, gpios);
	}
}

static inline void gpio32_set_hi(void __iomem *base, u32 gpios, int type)
{
	union iop_32 __iomem *regs = base;

	if (type == MPC8XX_CPM1_PORTB) {
		setbits_be32(&regs->b.pdat, gpios);
		/* GPDIR register 1 -> output */
		setbits_be32(&regs->b.pdir, gpios);
	} else {
		setbits_be32(&regs->e.pdat, gpios);
		/* GPDIR register 1 -> output */
		setbits_be32(&regs->e.pdir, gpios);
	}
}

static int mpc8xx_gpio_direction_input(struct udevice *dev, uint gpio)
{
	struct mpc8xx_gpio_data *data = dev_get_priv(dev);
	int type = data->type;

	if (type == MPC8XX_CPM1_PORTB || type == MPC8XX_CPM1_PORTE)
		gpio32_set_in(data->base, gpio_mask(gpio, type), type);
	else
		gpio16_set_in(data->base, gpio_mask(gpio, type), type);

	return 0;
}

static int mpc8xx_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	struct mpc8xx_gpio_data *data = dev_get_priv(dev);
	int type = data->type;

	if (type == MPC8XX_CPM1_PORTB || type == MPC8XX_CPM1_PORTE) {
		if (value)
			gpio32_set_hi(data->base, gpio_mask(gpio, type), type);
		else
			gpio32_set_lo(data->base, gpio_mask(gpio, type), type);
	} else {
		if (value)
			gpio16_set_hi(data->base, gpio_mask(gpio, type), type);
		else
			gpio16_set_lo(data->base, gpio_mask(gpio, type), type);
	}

	return 0;
}

static int mpc8xx_gpio_direction_output(struct udevice *dev, uint gpio,
					int value)
{
	return mpc8xx_gpio_set_value(dev, gpio, value);
}

static int mpc8xx_gpio_get_value(struct udevice *dev, uint gpio)
{
	struct mpc8xx_gpio_data *data = dev_get_priv(dev);
	int type = data->type;

	/* Input -> read value from GPDAT register */
	if (type == MPC8XX_CPM1_PORTB || type == MPC8XX_CPM1_PORTE)
		return gpio32_get_val(data->base, gpio_mask(gpio, type), type);
	else
		return gpio16_get_val(data->base, gpio_mask(gpio, type), type);
}

static int mpc8xx_gpio_get_function(struct udevice *dev, uint gpio)
{
	struct mpc8xx_gpio_data *data = dev_get_priv(dev);
	int type = data->type;
	int dir;

	if (type == MPC8XX_CPM1_PORTB || type == MPC8XX_CPM1_PORTE)
		dir = gpio32_get_dir(data->base, gpio_mask(gpio, type), type);
	else
		dir = gpio16_get_dir(data->base, gpio_mask(gpio, type), type);
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int mpc8xx_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct mpc8xx_gpio_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;
	u32 reg[2];

	dev_read_u32_array(dev, "reg", reg, 2);
	addr = dev_translate_address(dev, reg);

	plat->addr = addr;
	plat->size = reg[1];
	plat->ngpios = dev_read_u32_default(dev, "ngpios", 32);

	return 0;
}

static int mpc8xx_gpio_platdata_to_priv(struct udevice *dev)
{
	struct mpc8xx_gpio_data *priv = dev_get_priv(dev);
	struct mpc8xx_gpio_plat *plat = dev_get_plat(dev);
	unsigned long size = plat->size;
	int type;

	if (size == 0)
		size = 0x100;

	priv->addr = plat->addr;
	priv->base = map_sysmem(plat->addr, size);

	if (!priv->base)
		return -ENOMEM;

	priv->gpio_count = plat->ngpios;

	type = dev_get_driver_data(dev);

	if ((type == MPC8XX_CPM1_PORTA || type == MPC8XX_CPM1_PORTC ||
	     type == MPC8XX_CPM1_PORTD) && plat->ngpios == 32)
		priv->gpio_count = 16;

	priv->type = type;

	return 0;
}

static int mpc8xx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mpc8xx_gpio_data *data = dev_get_priv(dev);
	char name[32], *str;

	mpc8xx_gpio_platdata_to_priv(dev);

	snprintf(name, sizeof(name), "MPC@%lx_", data->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = data->gpio_count;

	return 0;
}

static const struct dm_gpio_ops gpio_mpc8xx_ops = {
	.direction_input	= mpc8xx_gpio_direction_input,
	.direction_output	= mpc8xx_gpio_direction_output,
	.get_value		= mpc8xx_gpio_get_value,
	.set_value		= mpc8xx_gpio_set_value,
	.get_function		= mpc8xx_gpio_get_function,
};

static const struct udevice_id mpc8xx_gpio_ids[] = {
	{ .compatible = "fsl,cpm1-pario-bank-a", .data = MPC8XX_CPM1_PORTA },
	{ .compatible = "fsl,cpm1-pario-bank-b", .data = MPC8XX_CPM1_PORTB },
	{ .compatible = "fsl,cpm1-pario-bank-c", .data = MPC8XX_CPM1_PORTC },
	{ .compatible = "fsl,cpm1-pario-bank-d", .data = MPC8XX_CPM1_PORTD },
	{ .compatible = "fsl,cpm1-pario-bank-e", .data = MPC8XX_CPM1_PORTE },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_mpc8xx) = {
	.name	= "gpio_mpc8xx",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mpc8xx_ops,
	.of_to_plat = mpc8xx_gpio_ofdata_to_platdata,
	.plat_auto = sizeof(struct mpc8xx_gpio_plat),
	.of_match = mpc8xx_gpio_ids,
	.probe	= mpc8xx_gpio_probe,
	.priv_auto = sizeof(struct mpc8xx_gpio_data),
};
