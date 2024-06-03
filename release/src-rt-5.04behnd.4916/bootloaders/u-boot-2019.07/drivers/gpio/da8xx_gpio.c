// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO driver for TI DaVinci DA8xx SOCs.
 *
 * (C) Copyright 2011 Guralp Systems Ltd.
 * Laurence Withers <lwithers@guralp.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>

#include "da8xx_gpio.h"

#ifndef CONFIG_DM_GPIO
#include <asm/arch/hardware.h>
#include <asm/arch/davinci_misc.h>

static struct gpio_registry {
	int is_registered;
	char name[GPIO_NAME_SIZE];
} gpio_registry[MAX_NUM_GPIOS];

#if defined(CONFIG_SOC_DA8XX)
#define pinmux(x)       (&davinci_syscfg_regs->pinmux[x])

#if defined(CONFIG_SOC_DA8XX) && !defined(CONFIG_SOC_DA850)
static const struct pinmux_config gpio_pinmux[] = {
	{ pinmux(13), 8, 6 },	/* GP0[0] */
	{ pinmux(13), 8, 7 },
	{ pinmux(14), 8, 0 },
	{ pinmux(14), 8, 1 },
	{ pinmux(14), 8, 2 },
	{ pinmux(14), 8, 3 },
	{ pinmux(14), 8, 4 },
	{ pinmux(14), 8, 5 },
	{ pinmux(14), 8, 6 },
	{ pinmux(14), 8, 7 },
	{ pinmux(15), 8, 0 },
	{ pinmux(15), 8, 1 },
	{ pinmux(15), 8, 2 },
	{ pinmux(15), 8, 3 },
	{ pinmux(15), 8, 4 },
	{ pinmux(15), 8, 5 },
	{ pinmux(15), 8, 6 },	/* GP1[0] */
	{ pinmux(15), 8, 7 },
	{ pinmux(16), 8, 0 },
	{ pinmux(16), 8, 1 },
	{ pinmux(16), 8, 2 },
	{ pinmux(16), 8, 3 },
	{ pinmux(16), 8, 4 },
	{ pinmux(16), 8, 5 },
	{ pinmux(16), 8, 6 },
	{ pinmux(16), 8, 7 },
	{ pinmux(17), 8, 0 },
	{ pinmux(17), 8, 1 },
	{ pinmux(17), 8, 2 },
	{ pinmux(17), 8, 3 },
	{ pinmux(17), 8, 4 },
	{ pinmux(17), 8, 5 },
	{ pinmux(17), 8, 6 },	/* GP2[0] */
	{ pinmux(17), 8, 7 },
	{ pinmux(18), 8, 0 },
	{ pinmux(18), 8, 1 },
	{ pinmux(18), 8, 2 },
	{ pinmux(18), 8, 3 },
	{ pinmux(18), 8, 4 },
	{ pinmux(18), 8, 5 },
	{ pinmux(18), 8, 6 },
	{ pinmux(18), 8, 7 },
	{ pinmux(19), 8, 0 },
	{ pinmux(9), 8, 2 },
	{ pinmux(9), 8, 3 },
	{ pinmux(9), 8, 4 },
	{ pinmux(9), 8, 5 },
	{ pinmux(9), 8, 6 },
	{ pinmux(10), 8, 1 },	/* GP3[0] */
	{ pinmux(10), 8, 2 },
	{ pinmux(10), 8, 3 },
	{ pinmux(10), 8, 4 },
	{ pinmux(10), 8, 5 },
	{ pinmux(10), 8, 6 },
	{ pinmux(10), 8, 7 },
	{ pinmux(11), 8, 0 },
	{ pinmux(11), 8, 1 },
	{ pinmux(11), 8, 2 },
	{ pinmux(11), 8, 3 },
	{ pinmux(11), 8, 4 },
	{ pinmux(9), 8, 7 },
	{ pinmux(2), 8, 6 },
	{ pinmux(11), 8, 5 },
	{ pinmux(11), 8, 6 },
	{ pinmux(12), 8, 4 },	/* GP4[0] */
	{ pinmux(12), 8, 5 },
	{ pinmux(12), 8, 6 },
	{ pinmux(12), 8, 7 },
	{ pinmux(13), 8, 0 },
	{ pinmux(13), 8, 1 },
	{ pinmux(13), 8, 2 },
	{ pinmux(13), 8, 3 },
	{ pinmux(13), 8, 4 },
	{ pinmux(13), 8, 5 },
	{ pinmux(11), 8, 7 },
	{ pinmux(12), 8, 0 },
	{ pinmux(12), 8, 1 },
	{ pinmux(12), 8, 2 },
	{ pinmux(12), 8, 3 },
	{ pinmux(9), 8, 1 },
	{ pinmux(7), 8, 3 },	/* GP5[0] */
	{ pinmux(7), 8, 4 },
	{ pinmux(7), 8, 5 },
	{ pinmux(7), 8, 6 },
	{ pinmux(7), 8, 7 },
	{ pinmux(8), 8, 0 },
	{ pinmux(8), 8, 1 },
	{ pinmux(8), 8, 2 },
	{ pinmux(8), 8, 3 },
	{ pinmux(8), 8, 4 },
	{ pinmux(8), 8, 5 },
	{ pinmux(8), 8, 6 },
	{ pinmux(8), 8, 7 },
	{ pinmux(9), 8, 0 },
	{ pinmux(7), 8, 1 },
	{ pinmux(7), 8, 2 },
	{ pinmux(5), 8, 1 },	/* GP6[0] */
	{ pinmux(5), 8, 2 },
	{ pinmux(5), 8, 3 },
	{ pinmux(5), 8, 4 },
	{ pinmux(5), 8, 5 },
	{ pinmux(5), 8, 6 },
	{ pinmux(5), 8, 7 },
	{ pinmux(6), 8, 0 },
	{ pinmux(6), 8, 1 },
	{ pinmux(6), 8, 2 },
	{ pinmux(6), 8, 3 },
	{ pinmux(6), 8, 4 },
	{ pinmux(6), 8, 5 },
	{ pinmux(6), 8, 6 },
	{ pinmux(6), 8, 7 },
	{ pinmux(7), 8, 0 },
	{ pinmux(1), 8, 0 },	/* GP7[0] */
	{ pinmux(1), 8, 1 },
	{ pinmux(1), 8, 2 },
	{ pinmux(1), 8, 3 },
	{ pinmux(1), 8, 4 },
	{ pinmux(1), 8, 5 },
	{ pinmux(1), 8, 6 },
	{ pinmux(1), 8, 7 },
	{ pinmux(2), 8, 0 },
	{ pinmux(2), 8, 1 },
	{ pinmux(2), 8, 2 },
	{ pinmux(2), 8, 3 },
	{ pinmux(2), 8, 4 },
	{ pinmux(2), 8, 5 },
	{ pinmux(0), 1, 0 },
	{ pinmux(0), 1, 1 },
};
#else /* CONFIG_SOC_DA8XX && CONFIG_SOC_DA850 */
static const struct pinmux_config gpio_pinmux[] = {
	{ pinmux(1), 8, 7 },	/* GP0[0] */
	{ pinmux(1), 8, 6 },
	{ pinmux(1), 8, 5 },
	{ pinmux(1), 8, 4 },
	{ pinmux(1), 8, 3 },
	{ pinmux(1), 8, 2 },
	{ pinmux(1), 8, 1 },
	{ pinmux(1), 8, 0 },
	{ pinmux(0), 8, 7 },
	{ pinmux(0), 8, 6 },
	{ pinmux(0), 8, 5 },
	{ pinmux(0), 8, 4 },
	{ pinmux(0), 8, 3 },
	{ pinmux(0), 8, 2 },
	{ pinmux(0), 8, 1 },
	{ pinmux(0), 8, 0 },
	{ pinmux(4), 8, 7 },	/* GP1[0] */
	{ pinmux(4), 8, 6 },
	{ pinmux(4), 8, 5 },
	{ pinmux(4), 8, 4 },
	{ pinmux(4), 8, 3 },
	{ pinmux(4), 8, 2 },
	{ pinmux(4), 4, 1 },
	{ pinmux(4), 4, 0 },
	{ pinmux(3), 4, 0 },
	{ pinmux(2), 4, 6 },
	{ pinmux(2), 4, 5 },
	{ pinmux(2), 4, 4 },
	{ pinmux(2), 4, 3 },
	{ pinmux(2), 4, 2 },
	{ pinmux(2), 4, 1 },
	{ pinmux(2), 8, 0 },
	{ pinmux(6), 8, 7 },	/* GP2[0] */
	{ pinmux(6), 8, 6 },
	{ pinmux(6), 8, 5 },
	{ pinmux(6), 8, 4 },
	{ pinmux(6), 8, 3 },
	{ pinmux(6), 8, 2 },
	{ pinmux(6), 8, 1 },
	{ pinmux(6), 8, 0 },
	{ pinmux(5), 8, 7 },
	{ pinmux(5), 8, 6 },
	{ pinmux(5), 8, 5 },
	{ pinmux(5), 8, 4 },
	{ pinmux(5), 8, 3 },
	{ pinmux(5), 8, 2 },
	{ pinmux(5), 8, 1 },
	{ pinmux(5), 8, 0 },
	{ pinmux(8), 8, 7 },	/* GP3[0] */
	{ pinmux(8), 8, 6 },
	{ pinmux(8), 8, 5 },
	{ pinmux(8), 8, 4 },
	{ pinmux(8), 8, 3 },
	{ pinmux(8), 8, 2 },
	{ pinmux(8), 8, 1 },
	{ pinmux(8), 8, 0 },
	{ pinmux(7), 8, 7 },
	{ pinmux(7), 8, 6 },
	{ pinmux(7), 8, 5 },
	{ pinmux(7), 8, 4 },
	{ pinmux(7), 8, 3 },
	{ pinmux(7), 8, 2 },
	{ pinmux(7), 8, 1 },
	{ pinmux(7), 8, 0 },
	{ pinmux(10), 8, 7 },	/* GP4[0] */
	{ pinmux(10), 8, 6 },
	{ pinmux(10), 8, 5 },
	{ pinmux(10), 8, 4 },
	{ pinmux(10), 8, 3 },
	{ pinmux(10), 8, 2 },
	{ pinmux(10), 8, 1 },
	{ pinmux(10), 8, 0 },
	{ pinmux(9), 8, 7 },
	{ pinmux(9), 8, 6 },
	{ pinmux(9), 8, 5 },
	{ pinmux(9), 8, 4 },
	{ pinmux(9), 8, 3 },
	{ pinmux(9), 8, 2 },
	{ pinmux(9), 8, 1 },
	{ pinmux(9), 8, 0 },
	{ pinmux(12), 8, 7 },	/* GP5[0] */
	{ pinmux(12), 8, 6 },
	{ pinmux(12), 8, 5 },
	{ pinmux(12), 8, 4 },
	{ pinmux(12), 8, 3 },
	{ pinmux(12), 8, 2 },
	{ pinmux(12), 8, 1 },
	{ pinmux(12), 8, 0 },
	{ pinmux(11), 8, 7 },
	{ pinmux(11), 8, 6 },
	{ pinmux(11), 8, 5 },
	{ pinmux(11), 8, 4 },
	{ pinmux(11), 8, 3 },
	{ pinmux(11), 8, 2 },
	{ pinmux(11), 8, 1 },
	{ pinmux(11), 8, 0 },
	{ pinmux(19), 8, 6 },	/* GP6[0] */
	{ pinmux(19), 8, 5 },
	{ pinmux(19), 8, 4 },
	{ pinmux(19), 8, 3 },
	{ pinmux(19), 8, 2 },
	{ pinmux(16), 8, 1 },
	{ pinmux(14), 8, 1 },
	{ pinmux(14), 8, 0 },
	{ pinmux(13), 8, 7 },
	{ pinmux(13), 8, 6 },
	{ pinmux(13), 8, 5 },
	{ pinmux(13), 8, 4 },
	{ pinmux(13), 8, 3 },
	{ pinmux(13), 8, 2 },
	{ pinmux(13), 8, 1 },
	{ pinmux(13), 8, 0 },
	{ pinmux(18), 8, 1 },	/* GP7[0] */
	{ pinmux(18), 8, 0 },
	{ pinmux(17), 8, 7 },
	{ pinmux(17), 8, 6 },
	{ pinmux(17), 8, 5 },
	{ pinmux(17), 8, 4 },
	{ pinmux(17), 8, 3 },
	{ pinmux(17), 8, 2 },
	{ pinmux(17), 8, 1 },
	{ pinmux(17), 8, 0 },
	{ pinmux(16), 8, 7 },
	{ pinmux(16), 8, 6 },
	{ pinmux(16), 8, 5 },
	{ pinmux(16), 8, 4 },
	{ pinmux(16), 8, 3 },
	{ pinmux(16), 8, 2 },
	{ pinmux(19), 8, 0 },	/* GP8[0] */
	{ pinmux(3), 4, 7 },
	{ pinmux(3), 4, 6 },
	{ pinmux(3), 4, 5 },
	{ pinmux(3), 4, 4 },
	{ pinmux(3), 4, 3 },
	{ pinmux(3), 4, 2 },
	{ pinmux(2), 4, 7 },
	{ pinmux(19), 8, 1 },
	{ pinmux(19), 8, 0 },
	{ pinmux(18), 8, 7 },
	{ pinmux(18), 8, 6 },
	{ pinmux(18), 8, 5 },
	{ pinmux(18), 8, 4 },
	{ pinmux(18), 8, 3 },
	{ pinmux(18), 8, 2 },
};
#endif /* CONFIG_SOC_DA8XX && !CONFIG_SOC_DA850 */
#else /* !CONFIG_SOC_DA8XX */
#define davinci_configure_pin_mux(a, b)
#endif /* CONFIG_SOC_DA8XX */

int gpio_request(unsigned int gpio, const char *label)
{
	if (gpio >= MAX_NUM_GPIOS)
		return -1;

	if (gpio_registry[gpio].is_registered)
		return -1;

	gpio_registry[gpio].is_registered = 1;
	strncpy(gpio_registry[gpio].name, label, GPIO_NAME_SIZE);
	gpio_registry[gpio].name[GPIO_NAME_SIZE - 1] = 0;

	davinci_configure_pin_mux(&gpio_pinmux[gpio], 1);

	return 0;
}

int gpio_free(unsigned int gpio)
{
	if (gpio >= MAX_NUM_GPIOS)
		return -1;

	if (!gpio_registry[gpio].is_registered)
		return -1;

	gpio_registry[gpio].is_registered = 0;
	gpio_registry[gpio].name[0] = '\0';
	/* Do not configure as input or change pin mux here */
	return 0;
}
#endif

static int _gpio_direction_output(struct davinci_gpio *bank, unsigned int gpio, int value)
{
	clrbits_le32(&bank->dir, 1U << GPIO_BIT(gpio));
	gpio_set_value(gpio, value);
	return 0;
}

static int _gpio_direction_input(struct davinci_gpio *bank, unsigned int gpio)
{
	setbits_le32(&bank->dir, 1U << GPIO_BIT(gpio));
	return 0;
}

static int _gpio_get_value(struct davinci_gpio *bank, unsigned int gpio)
{
	unsigned int ip;
	ip = in_le32(&bank->in_data) & (1U << GPIO_BIT(gpio));
	return ip ? 1 : 0;
}

static int _gpio_set_value(struct davinci_gpio *bank, unsigned int gpio, int value)
{
	if (value)
		bank->set_data = 1U << GPIO_BIT(gpio);
	else
		bank->clr_data = 1U << GPIO_BIT(gpio);

	return 0;
}

static int _gpio_get_dir(struct davinci_gpio *bank, unsigned int gpio)
{
	return in_le32(&bank->dir) & (1U << GPIO_BIT(gpio));
}

#ifndef CONFIG_DM_GPIO

void gpio_info(void)
{
	unsigned int gpio, dir, val;
	struct davinci_gpio *bank;

	for (gpio = 0; gpio < MAX_NUM_GPIOS; ++gpio) {
		bank = GPIO_BANK(gpio);
		dir = _gpio_get_dir(bank, gpio);
		val = gpio_get_value(gpio);

		printf("% 4d: %s: %d [%c] %s\n",
			gpio, dir ? " in" : "out", val,
			gpio_registry[gpio].is_registered ? 'x' : ' ',
			gpio_registry[gpio].name);
	}
}

int gpio_direction_input(unsigned int gpio)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	return _gpio_direction_input(bank, gpio);
}

int gpio_direction_output(unsigned int gpio, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	return _gpio_direction_output(bank, gpio, value);
}

int gpio_get_value(unsigned int gpio)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	return _gpio_get_value(bank, gpio);
}

int gpio_set_value(unsigned int gpio, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	return _gpio_set_value(bank, gpio, value);
}

#else /* CONFIG_DM_GPIO */

static struct davinci_gpio *davinci_get_gpio_bank(struct udevice *dev, unsigned int offset)
{
	struct davinci_gpio_bank *bank = dev_get_priv(dev);
	unsigned int addr;

	/*
	 * The device tree is not broken into banks but the infrastructure is
	 * expecting it this way, so we'll first include the 0x10 offset, then
	 * calculate the bank manually based on the offset.
	 * Casting 'addr' as Unsigned long is needed to make the math work.
	 */
	addr = ((unsigned long)(struct davinci_gpio *)bank->base) +
			0x10 + (0x28 * (offset >> 5));
	return (struct davinci_gpio *)addr;
}

static int davinci_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct davinci_gpio *base = davinci_get_gpio_bank(dev, offset);

	/*
	 * Fetch the address based on GPIO, but only pass the masked low 32-bits
	 */
	_gpio_direction_input(base, (offset & 0x1f));
	return 0;
}

static int davinci_gpio_direction_output(struct udevice *dev, unsigned int offset,
					 int value)
{
	struct davinci_gpio *base = davinci_get_gpio_bank(dev, offset);

	_gpio_direction_output(base, (offset & 0x1f), value);
	return 0;
}

static int davinci_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct davinci_gpio *base = davinci_get_gpio_bank(dev, offset);

	return _gpio_get_value(base, (offset & 0x1f));
}

static int davinci_gpio_set_value(struct udevice *dev, unsigned int offset,
				  int value)
{
	struct davinci_gpio *base = davinci_get_gpio_bank(dev, offset);

	_gpio_set_value(base, (offset & 0x1f), value);

	return 0;
}

static int davinci_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	unsigned int dir;
	struct davinci_gpio *base = davinci_get_gpio_bank(dev, offset);

	dir = _gpio_get_dir(base, offset);

	if (dir)
		return GPIOF_INPUT;

	return GPIOF_OUTPUT;
}

static int davinci_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			      struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];

	if (args->args[1] & GPIO_ACTIVE_LOW)
		desc->flags = GPIOD_ACTIVE_LOW;
	else
		desc->flags = 0;
	return 0;
}

static const struct dm_gpio_ops gpio_davinci_ops = {
	.direction_input	= davinci_gpio_direction_input,
	.direction_output	= davinci_gpio_direction_output,
	.get_value		= davinci_gpio_get_value,
	.set_value		= davinci_gpio_set_value,
	.get_function		= davinci_gpio_get_function,
	.xlate			= davinci_gpio_xlate,
};

static int davinci_gpio_probe(struct udevice *dev)
{
	struct davinci_gpio_bank *bank = dev_get_priv(dev);
	struct davinci_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);

	uc_priv->bank_name = plat->port_name;
	uc_priv->gpio_count = fdtdec_get_int(fdt, node, "ti,ngpio", -1);
	bank->base = (struct davinci_gpio *)plat->base;
	return 0;
}

static const struct udevice_id davinci_gpio_ids[] = {
	{ .compatible = "ti,dm6441-gpio" },
	{ .compatible = "ti,k2g-gpio" },
	{ }
};

static int davinci_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct davinci_gpio_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	return 0;
}

U_BOOT_DRIVER(gpio_davinci) = {
	.name	= "gpio_davinci",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_davinci_ops,
	.ofdata_to_platdata = of_match_ptr(davinci_gpio_ofdata_to_platdata),
	.of_match = davinci_gpio_ids,
	.bind   = dm_scan_fdt_dev,
	.platdata_auto_alloc_size = sizeof(struct davinci_gpio_platdata),
	.probe	= davinci_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct davinci_gpio_bank),
};

#endif
