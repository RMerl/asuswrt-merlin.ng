// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * Based on earlier arch/arm/cpu/armv7/sunxi/gpio.c:
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/gpio.h>

#define SUNXI_GPIOS_PER_BANK	SUNXI_GPIO_A_NR

struct sunxi_gpio_platdata {
	struct sunxi_gpio *regs;
	const char *bank_name;	/* Name of bank, e.g. "B" */
	int gpio_count;
};

#ifndef CONFIG_DM_GPIO
static int sunxi_gpio_output(u32 pin, u32 val)
{
	u32 dat;
	u32 bank = GPIO_BANK(pin);
	u32 num = GPIO_NUM(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	dat = readl(&pio->dat);
	if (val)
		dat |= 0x1 << num;
	else
		dat &= ~(0x1 << num);

	writel(dat, &pio->dat);

	return 0;
}

static int sunxi_gpio_input(u32 pin)
{
	u32 dat;
	u32 bank = GPIO_BANK(pin);
	u32 num = GPIO_NUM(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	dat = readl(&pio->dat);
	dat >>= num;

	return dat & 0x1;
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	sunxi_gpio_set_cfgpin(gpio, SUNXI_GPIO_INPUT);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	sunxi_gpio_set_cfgpin(gpio, SUNXI_GPIO_OUTPUT);

	return sunxi_gpio_output(gpio, value);
}

int gpio_get_value(unsigned gpio)
{
	return sunxi_gpio_input(gpio);
}

int gpio_set_value(unsigned gpio, int value)
{
	return sunxi_gpio_output(gpio, value);
}

int sunxi_name_to_gpio(const char *name)
{
	int group = 0;
	int groupsize = 9 * 32;
	long pin;
	char *eptr;

	if (*name == 'P' || *name == 'p')
		name++;
	if (*name >= 'A') {
		group = *name - (*name > 'a' ? 'a' : 'A');
		groupsize = 32;
		name++;
	}

	pin = simple_strtol(name, &eptr, 10);
	if (!*name || *eptr)
		return -1;
	if (pin < 0 || pin > groupsize || group >= 9)
		return -1;
	return group * 32 + pin;
}
#endif

int sunxi_name_to_gpio_bank(const char *name)
{
	int group = 0;

	if (*name == 'P' || *name == 'p')
		name++;
	if (*name >= 'A') {
		group = *name - (*name > 'a' ? 'a' : 'A');
		return group;
	}

	return -1;
}

#ifdef CONFIG_DM_GPIO
/* TODO(sjg@chromium.org): Remove this function and use device tree */
int sunxi_name_to_gpio(const char *name)
{
	unsigned int gpio;
	int ret;
#if !defined CONFIG_SPL_BUILD && defined CONFIG_AXP_GPIO
	char lookup[8];

	if (strcasecmp(name, "AXP0-VBUS-DETECT") == 0) {
		sprintf(lookup, SUNXI_GPIO_AXP0_PREFIX "%d",
			SUNXI_GPIO_AXP0_VBUS_DETECT);
		name = lookup;
	} else if (strcasecmp(name, "AXP0-VBUS-ENABLE") == 0) {
		sprintf(lookup, SUNXI_GPIO_AXP0_PREFIX "%d",
			SUNXI_GPIO_AXP0_VBUS_ENABLE);
		name = lookup;
	}
#endif
	ret = gpio_lookup_name(name, NULL, NULL, &gpio);

	return ret ? ret : gpio;
}

static int sunxi_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);

	sunxi_gpio_set_cfgbank(plat->regs, offset, SUNXI_GPIO_INPUT);

	return 0;
}

static int sunxi_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);
	u32 num = GPIO_NUM(offset);

	sunxi_gpio_set_cfgbank(plat->regs, offset, SUNXI_GPIO_OUTPUT);
	clrsetbits_le32(&plat->regs->dat, 1 << num, value ? (1 << num) : 0);

	return 0;
}

static int sunxi_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);
	u32 num = GPIO_NUM(offset);
	unsigned dat;

	dat = readl(&plat->regs->dat);
	dat >>= num;

	return dat & 0x1;
}

static int sunxi_gpio_set_value(struct udevice *dev, unsigned offset,
				int value)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);
	u32 num = GPIO_NUM(offset);

	clrsetbits_le32(&plat->regs->dat, 1 << num, value ? (1 << num) : 0);
	return 0;
}

static int sunxi_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);
	int func;

	func = sunxi_gpio_get_cfgbank(plat->regs, offset);
	if (func == SUNXI_GPIO_OUTPUT)
		return GPIOF_OUTPUT;
	else if (func == SUNXI_GPIO_INPUT)
		return GPIOF_INPUT;
	else
		return GPIOF_FUNC;
}

static int sunxi_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	int ret;

	ret = device_get_child(dev, args->args[0], &desc->dev);
	if (ret)
		return ret;
	desc->offset = args->args[1];
	desc->flags = args->args[2] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops gpio_sunxi_ops = {
	.direction_input	= sunxi_gpio_direction_input,
	.direction_output	= sunxi_gpio_direction_output,
	.get_value		= sunxi_gpio_get_value,
	.set_value		= sunxi_gpio_set_value,
	.get_function		= sunxi_gpio_get_function,
	.xlate			= sunxi_gpio_xlate,
};

/**
 * Returns the name of a GPIO bank
 *
 * GPIO banks are named A, B, C, ...
 *
 * @bank:	Bank number (0, 1..n-1)
 * @return allocated string containing the name
 */
static char *gpio_bank_name(int bank)
{
	char *name;

	name = malloc(3);
	if (name) {
		name[0] = 'P';
		name[1] = 'A' + bank;
		name[2] = '\0';
	}

	return name;
}

static int gpio_sunxi_probe(struct udevice *dev)
{
	struct sunxi_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Tell the uclass how many GPIOs we have */
	if (plat) {
		uc_priv->gpio_count = plat->gpio_count;
		uc_priv->bank_name = plat->bank_name;
	}

	return 0;
}

struct sunxi_gpio_soc_data {
	int start;
	int no_banks;
};

/**
 * We have a top-level GPIO device with no actual GPIOs. It has a child
 * device for each Sunxi bank.
 */
static int gpio_sunxi_bind(struct udevice *parent)
{
	struct sunxi_gpio_soc_data *soc_data =
		(struct sunxi_gpio_soc_data *)dev_get_driver_data(parent);
	struct sunxi_gpio_platdata *plat = parent->platdata;
	struct sunxi_gpio_reg *ctlr;
	int bank, ret;

	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;

	ctlr = (struct sunxi_gpio_reg *)devfdt_get_addr(parent);
	for (bank = 0; bank < soc_data->no_banks; bank++) {
		struct sunxi_gpio_platdata *plat;
		struct udevice *dev;

		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;
		plat->regs = &ctlr->gpio_bank[bank];
		plat->bank_name = gpio_bank_name(soc_data->start + bank);
		plat->gpio_count = SUNXI_GPIOS_PER_BANK;

		ret = device_bind(parent, parent->driver,
					plat->bank_name, plat, -1, &dev);
		if (ret)
			return ret;
		dev_set_of_offset(dev, dev_of_offset(parent));
	}

	return 0;
}

static const struct sunxi_gpio_soc_data soc_data_a_all = {
	.start = 0,
	.no_banks = SUNXI_GPIO_BANKS,
};

static const struct sunxi_gpio_soc_data soc_data_l_1 = {
	.start = 'L' - 'A',
	.no_banks = 1,
};

static const struct sunxi_gpio_soc_data soc_data_l_2 = {
	.start = 'L' - 'A',
	.no_banks = 2,
};

static const struct sunxi_gpio_soc_data soc_data_l_3 = {
	.start = 'L' - 'A',
	.no_banks = 3,
};

#define ID(_compat_, _soc_data_) \
	{ .compatible = _compat_, .data = (ulong)&soc_data_##_soc_data_ }

static const struct udevice_id sunxi_gpio_ids[] = {
	ID("allwinner,sun4i-a10-pinctrl",	a_all),
	ID("allwinner,sun5i-a10s-pinctrl",	a_all),
	ID("allwinner,sun5i-a13-pinctrl",	a_all),
	ID("allwinner,sun50i-h5-pinctrl",	a_all),
	ID("allwinner,sun6i-a31-pinctrl",	a_all),
	ID("allwinner,sun6i-a31s-pinctrl",	a_all),
	ID("allwinner,sun7i-a20-pinctrl",	a_all),
	ID("allwinner,sun8i-a23-pinctrl",	a_all),
	ID("allwinner,sun8i-a33-pinctrl",	a_all),
	ID("allwinner,sun8i-a83t-pinctrl",	a_all),
	ID("allwinner,sun8i-h3-pinctrl",	a_all),
	ID("allwinner,sun8i-r40-pinctrl",	a_all),
	ID("allwinner,sun8i-v3s-pinctrl",	a_all),
	ID("allwinner,sun9i-a80-pinctrl",	a_all),
	ID("allwinner,sun50i-a64-pinctrl",	a_all),
	ID("allwinner,sun6i-a31-r-pinctrl",	l_2),
	ID("allwinner,sun8i-a23-r-pinctrl",	l_1),
	ID("allwinner,sun8i-a83t-r-pinctrl",	l_1),
	ID("allwinner,sun8i-h3-r-pinctrl",	l_1),
	ID("allwinner,sun9i-a80-r-pinctrl",	l_3),
	ID("allwinner,sun50i-a64-r-pinctrl",	l_1),
	{ }
};

U_BOOT_DRIVER(gpio_sunxi) = {
	.name	= "gpio_sunxi",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_sunxi_ops,
	.of_match = sunxi_gpio_ids,
	.bind	= gpio_sunxi_bind,
	.probe	= gpio_sunxi_probe,
};
#endif
