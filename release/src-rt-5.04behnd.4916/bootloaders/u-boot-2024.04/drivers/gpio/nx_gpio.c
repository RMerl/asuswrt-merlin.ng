// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * DeokJin, Lee <truevirtue@nexell.co.kr>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

struct nx_gpio_regs {
	u32	data;		/* Data register */
	u32	outputenb;	/* Output Enable register */
	u32	detmode[2];	/* Detect Mode Register */
	u32	intenb;		/* Interrupt Enable Register */
	u32	det;		/* Event Detect Register */
	u32	pad;		/* Pad Status Register */
};

struct nx_alive_gpio_regs {
	u32	pwrgate;	/* Power Gating Register */
	u32	reserved0[28];	/* Reserved0 */
	u32	outputenb_reset;/* Alive GPIO Output Enable Reset Register */
	u32	outputenb;	/* Alive GPIO Output Enable Register */
	u32	outputenb_read; /* Alive GPIO Output Read Register */
	u32	reserved1[3];	/* Reserved1 */
	u32	pad_reset;	/* Alive GPIO Output Reset Register */
	u32	data;		/* Alive GPIO Output Register */
	u32	pad_read;	/* Alive GPIO Pad Read Register */
	u32	reserved2[33];	/* Reserved2 */
	u32	pad;		/* Alive GPIO Input Value Register */
};

struct nx_gpio_plat {
	void *regs;
	int gpio_count;
	const char *bank_name;
};

static int nx_alive_gpio_is_check(struct udevice *dev)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	const char *bank_name = plat->bank_name;

	if (!strcmp(bank_name, "gpio_alv"))
		return 1;

	return 0;
}

static int nx_alive_gpio_direction_input(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_alive_gpio_regs *const regs = plat->regs;

	setbits_le32(&regs->outputenb_reset, 1 << pin);

	return 0;
}

static int nx_alive_gpio_direction_output(struct udevice *dev, unsigned int pin,
					  int val)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_alive_gpio_regs *const regs = plat->regs;

	if (val)
		setbits_le32(&regs->data, 1 << pin);
	else
		setbits_le32(&regs->pad_reset, 1 << pin);

	setbits_le32(&regs->outputenb, 1 << pin);

	return 0;
}

static int nx_alive_gpio_get_value(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_alive_gpio_regs *const regs = plat->regs;
	unsigned int mask = 1UL << pin;
	unsigned int value;

	value = (readl(&regs->pad_read) & mask) >> pin;

	return value;
}

static int nx_alive_gpio_set_value(struct udevice *dev, unsigned int pin,
				   int val)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_alive_gpio_regs *const regs = plat->regs;

	if (val)
		setbits_le32(&regs->data, 1 << pin);
	else
		clrbits_le32(&regs->pad_reset, 1 << pin);

	return 0;
}

static int nx_alive_gpio_get_function(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_alive_gpio_regs *const regs = plat->regs;
	unsigned int mask = (1UL << pin);
	unsigned int output;

	output = readl(&regs->outputenb_read) & mask;

	if (output)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int nx_gpio_direction_input(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_gpio_regs *const regs = plat->regs;

	if (nx_alive_gpio_is_check(dev))
		return nx_alive_gpio_direction_input(dev, pin);

	clrbits_le32(&regs->outputenb, 1 << pin);

	return 0;
}

static int nx_gpio_direction_output(struct udevice *dev, unsigned int pin,
				    int val)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_gpio_regs *const regs = plat->regs;

	if (nx_alive_gpio_is_check(dev))
		return nx_alive_gpio_direction_output(dev, pin, val);

	if (val)
		setbits_le32(&regs->data, 1 << pin);
	else
		clrbits_le32(&regs->data, 1 << pin);

	setbits_le32(&regs->outputenb, 1 << pin);

	return 0;
}

static int nx_gpio_get_value(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_gpio_regs *const regs = plat->regs;
	unsigned int mask = 1UL << pin;
	unsigned int value;

	if (nx_alive_gpio_is_check(dev))
		return nx_alive_gpio_get_value(dev, pin);

	value = (readl(&regs->pad) & mask) >> pin;

	return value;
}

static int nx_gpio_set_value(struct udevice *dev, unsigned int pin, int val)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_gpio_regs *const regs = plat->regs;

	if (nx_alive_gpio_is_check(dev))
		return nx_alive_gpio_set_value(dev, pin, val);

	if (val)
		setbits_le32(&regs->data, 1 << pin);
	else
		clrbits_le32(&regs->data, 1 << pin);

	return 0;
}

static int nx_gpio_get_function(struct udevice *dev, unsigned int pin)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);
	struct nx_gpio_regs *const regs = plat->regs;
	unsigned int mask = (1UL << pin);
	unsigned int output;

	if (nx_alive_gpio_is_check(dev))
		return nx_alive_gpio_get_function(dev, pin);

	output = readl(&regs->outputenb) & mask;

	if (output)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int nx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct nx_gpio_plat *plat = dev_get_plat(dev);

	uc_priv->gpio_count = plat->gpio_count;
	uc_priv->bank_name = plat->bank_name;

	return 0;
}

static int nx_gpio_of_to_plat(struct udevice *dev)
{
	struct nx_gpio_plat *plat = dev_get_plat(dev);

	plat->regs = map_physmem(devfdt_get_addr(dev),
				 sizeof(struct nx_gpio_regs),
				 MAP_NOCACHE);
	plat->gpio_count = dev_read_s32_default(dev, "nexell,gpio-bank-width",
						32);
	plat->bank_name = dev_read_string(dev, "gpio-bank-name");

	return 0;
}

static const struct dm_gpio_ops nx_gpio_ops = {
	.direction_input	= nx_gpio_direction_input,
	.direction_output	= nx_gpio_direction_output,
	.get_value		= nx_gpio_get_value,
	.set_value		= nx_gpio_set_value,
	.get_function		= nx_gpio_get_function,
};

static const struct udevice_id nx_gpio_ids[] = {
	{ .compatible = "nexell,nexell-gpio" },
	{ }
};

U_BOOT_DRIVER(nx_gpio) = {
	.name		= "nx_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= nx_gpio_ids,
	.ops		= &nx_gpio_ops,
	.of_to_plat = nx_gpio_of_to_plat,
	.plat_auto	= sizeof(struct nx_gpio_plat),
	.probe		= nx_gpio_probe,
};
