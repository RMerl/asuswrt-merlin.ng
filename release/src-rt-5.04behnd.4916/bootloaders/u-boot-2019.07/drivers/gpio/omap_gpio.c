// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This work is derived from the linux 2.6.27 kernel source
 * To fetch, use the kernel repository
 * git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
 * Use the v2.6.27 tag.
 *
 * Below is the original's header including its copyright
 *
 *  linux/arch/arm/plat-omap/gpio.c
 *
 * Support functions for OMAP GPIO
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 * Written by Juha Yrjölä <juha.yrjola@nokia.com>
 */
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

#define OMAP_GPIO_DIR_OUT	0
#define OMAP_GPIO_DIR_IN	1

#ifdef CONFIG_DM_GPIO

#define GPIO_PER_BANK			32

struct gpio_bank {
	/* TODO(sjg@chromium.org): Can we use a struct here? */
	void *base;	/* address of registers in physical memory */
};

#endif

static inline int get_gpio_index(int gpio)
{
	return gpio & 0x1f;
}

int gpio_is_valid(int gpio)
{
	return (gpio >= 0) && (gpio < OMAP_MAX_GPIO);
}

static void _set_gpio_direction(const struct gpio_bank *bank, int gpio,
				int is_input)
{
	void *reg = bank->base;
	u32 l;

	reg += OMAP_GPIO_OE;

	l = __raw_readl(reg);
	if (is_input)
		l |= 1 << gpio;
	else
		l &= ~(1 << gpio);
	__raw_writel(l, reg);
}

/**
 * Get the direction of the GPIO by reading the GPIO_OE register
 * corresponding to the specified bank.
 */
static int _get_gpio_direction(const struct gpio_bank *bank, int gpio)
{
	void *reg = bank->base;
	u32 v;

	reg += OMAP_GPIO_OE;

	v = __raw_readl(reg);

	if (v & (1 << gpio))
		return OMAP_GPIO_DIR_IN;
	else
		return OMAP_GPIO_DIR_OUT;
}

static void _set_gpio_dataout(const struct gpio_bank *bank, int gpio,
				int enable)
{
	void *reg = bank->base;
	u32 l = 0;

	if (enable)
		reg += OMAP_GPIO_SETDATAOUT;
	else
		reg += OMAP_GPIO_CLEARDATAOUT;

	l = 1 << gpio;
	__raw_writel(l, reg);
}

static int _get_gpio_value(const struct gpio_bank *bank, int gpio)
{
	void *reg = bank->base;
	int input;

	input = _get_gpio_direction(bank, gpio);
	switch (input) {
	case OMAP_GPIO_DIR_IN:
		reg += OMAP_GPIO_DATAIN;
		break;
	case OMAP_GPIO_DIR_OUT:
		reg += OMAP_GPIO_DATAOUT;
		break;
	default:
		return -1;
	}

	return (__raw_readl(reg) & (1 << gpio)) != 0;
}

#ifndef CONFIG_DM_GPIO

static inline const struct gpio_bank *get_gpio_bank(int gpio)
{
	return &omap_gpio_bank[gpio >> 5];
}

static int check_gpio(int gpio)
{
	if (!gpio_is_valid(gpio)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

/**
 * Set value of the specified gpio
 */
int gpio_set_value(unsigned gpio, int value)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;
	bank = get_gpio_bank(gpio);
	_set_gpio_dataout(bank, get_gpio_index(gpio), value);

	return 0;
}

/**
 * Get value of the specified gpio
 */
int gpio_get_value(unsigned gpio)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;
	bank = get_gpio_bank(gpio);

	return _get_gpio_value(bank, get_gpio_index(gpio));
}

/**
 * Set gpio direction as input
 */
int gpio_direction_input(unsigned gpio)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;

	bank = get_gpio_bank(gpio);
	_set_gpio_direction(bank, get_gpio_index(gpio), 1);

	return 0;
}

/**
 * Set gpio direction as output
 */
int gpio_direction_output(unsigned gpio, int value)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;

	bank = get_gpio_bank(gpio);
	_set_gpio_dataout(bank, get_gpio_index(gpio), value);
	_set_gpio_direction(bank, get_gpio_index(gpio), 0);

	return 0;
}

/**
 * Request a gpio before using it.
 *
 * NOTE: Argument 'label' is unused.
 */
int gpio_request(unsigned gpio, const char *label)
{
	if (check_gpio(gpio) < 0)
		return -1;

	return 0;
}

/**
 * Reset and free the gpio after using it.
 */
int gpio_free(unsigned gpio)
{
	return 0;
}

#else /* new driver model interface CONFIG_DM_GPIO */

/* set GPIO pin 'gpio' as an input */
static int omap_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	/* Configure GPIO direction as input. */
	_set_gpio_direction(bank, offset, 1);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int omap_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	_set_gpio_dataout(bank, offset, value);
	_set_gpio_direction(bank, offset, 0);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int omap_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	return _get_gpio_value(bank, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int omap_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	_set_gpio_dataout(bank, offset, value);

	return 0;
}

static int omap_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	/* GPIOF_FUNC is not implemented yet */
	if (_get_gpio_direction(bank, offset) == OMAP_GPIO_DIR_OUT)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_omap_ops = {
	.direction_input	= omap_gpio_direction_input,
	.direction_output	= omap_gpio_direction_output,
	.get_value		= omap_gpio_get_value,
	.set_value		= omap_gpio_set_value,
	.get_function		= omap_gpio_get_function,
};

static int omap_gpio_probe(struct udevice *dev)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	struct omap_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[18], *str;

	sprintf(name, "gpio@%4x_", (unsigned int)plat->base);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = GPIO_PER_BANK;
	bank->base = (void *)plat->base;
	return 0;
}

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static int omap_gpio_bind(struct udevice *dev)
{
	struct omap_gpio_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t base_addr;

	if (plat)
		return 0;

	base_addr = devfdt_get_addr(dev);
	if (base_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	/*
	* TODO:
	* When every board is converted to driver model and DT is
	* supported, this can be done by auto-alloc feature, but
	* not using calloc to alloc memory for platdata.
	*
	* For example am33xx_gpio uses platform data rather than device tree.
	*
	* NOTE: DO NOT COPY this code if you are using device tree.
	*/
	plat = calloc(1, sizeof(*plat));
	if (!plat)
		return -ENOMEM;

	plat->base = base_addr;
	plat->port_name = fdt_get_name(gd->fdt_blob, dev_of_offset(dev), NULL);
	dev->platdata = plat;

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct udevice_id omap_gpio_ids[] = {
	{ .compatible = "ti,omap3-gpio" },
	{ .compatible = "ti,omap4-gpio" },
	{ .compatible = "ti,am4372-gpio" },
	{ }
};

static int omap_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct omap_gpio_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	return 0;
}
#endif

U_BOOT_DRIVER(gpio_omap) = {
	.name	= "gpio_omap",
	.id	= UCLASS_GPIO,
#if CONFIG_IS_ENABLED(OF_CONTROL)
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = omap_gpio_ids,
	.ofdata_to_platdata = of_match_ptr(omap_gpio_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct omap_gpio_platdata),
#endif
#else
	.bind   = omap_gpio_bind,
#endif
	.ops	= &gpio_omap_ops,
	.probe	= omap_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct gpio_bank),
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

#endif /* CONFIG_DM_GPIO */
