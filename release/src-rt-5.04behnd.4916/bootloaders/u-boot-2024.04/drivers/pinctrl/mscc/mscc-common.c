// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <alexandre.belloni@free-electrons.com>
 * Author: <gregory.clement@bootlin.com>
 * License: Dual MIT/GPL
 * Copyright (c) 2017 Microsemi Corporation
 */

#include <asm/gpio.h>
#include <asm/system.h>
#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include "mscc-common.h"

static void mscc_writel(unsigned int offset, void *addr)
{
	if (offset < 32)
		writel(BIT(offset), addr);
	else
		writel(BIT(offset % 32), addr + 4);
}

static unsigned int mscc_readl(unsigned int offset, void *addr)
{
	if (offset < 32)
		return readl(addr);
	else
		return readl(addr + 4);
}

static void mscc_setbits(unsigned int offset, void *addr)
{
	if (offset < 32)
		writel(readl(addr) | BIT(offset), addr);
	else
		writel(readl(addr + 4) | BIT(offset % 32), addr + 4);
}

static void mscc_clrbits(unsigned int offset, void *addr)
{
	if (offset < 32)
		writel(readl(addr) & ~BIT(offset), addr);
	else
		writel(readl(addr + 4) & ~BIT(offset % 32), addr + 4);
}

static int mscc_get_functions_count(struct udevice *dev)
{
	struct mscc_pinctrl *info = dev_get_priv(dev);

	return info->num_func;
}

static const char *mscc_get_function_name(struct udevice *dev,
					  unsigned int function)
{
	struct mscc_pinctrl *info = dev_get_priv(dev);

	return info->function_names[function];
}

static int mscc_pin_function_idx(unsigned int pin, unsigned int function,
				 const struct mscc_pin_data *mscc_pins)
{
	struct mscc_pin_caps *p = mscc_pins[pin].drv_data;
	int i;

	for (i = 0; i < MSCC_FUNC_PER_PIN; i++) {
		if (function == p->functions[i])
			return i;
	}

	return -1;
}

static int mscc_pinmux_set_mux(struct udevice *dev,
			       unsigned int pin_selector, unsigned int selector)
{
	struct mscc_pinctrl *info = dev_get_priv(dev);
	struct mscc_pin_caps *pin = info->mscc_pins[pin_selector].drv_data;
	int f, offset, regoff;

	f = mscc_pin_function_idx(pin_selector, selector, info->mscc_pins);
	if (f < 0)
		return -EINVAL;
	/*
	 * f is encoded on two bits.
	 * bit 0 of f goes in BIT(pin) of ALT0, bit 1 of f goes in BIT(pin) of
	 * ALT1
	 * This is racy because both registers can't be updated at the same time
	 * but it doesn't matter much for now.
	 */
	offset = pin->pin;
	regoff = info->mscc_gpios[MSCC_GPIO_ALT0];
	if (offset >= 32) {
		offset = offset % 32;
		regoff = info->mscc_gpios[MSCC_GPIO_ALT1];
	}

	if (f & BIT(0))
		mscc_setbits(offset, info->regs + regoff);
	else
		mscc_clrbits(offset, info->regs + regoff);

	if (f & BIT(1))
		mscc_setbits(offset, info->regs + regoff + 4);
	else
		mscc_clrbits(offset, info->regs + regoff + 4);

	return 0;
}

static int mscc_pctl_get_groups_count(struct udevice *dev)
{
	struct mscc_pinctrl *info = dev_get_priv(dev);

	return info->num_pins;
}

static const char *mscc_pctl_get_group_name(struct udevice *dev,
					    unsigned int group)
{
	struct mscc_pinctrl *info = dev_get_priv(dev);

	return info->mscc_pins[group].name;
}

static int mscc_create_group_func_map(struct udevice *dev,
				      struct mscc_pinctrl *info)
{
	u16 pins[info->num_pins];
	int f, npins, i;

	for (f = 0; f < info->num_func; f++) {
		for (npins = 0, i = 0; i < info->num_pins; i++) {
			if (mscc_pin_function_idx(i, f, info->mscc_pins) >= 0)
				pins[npins++] = i;
		}

		info->func[f].ngroups = npins;
		info->func[f].groups = devm_kzalloc(dev, npins * sizeof(char *),
						    GFP_KERNEL);
		if (!info->func[f].groups)
			return -ENOMEM;

		for (i = 0; i < npins; i++)
			info->func[f].groups[i] = info->mscc_pins[pins[i]].name;
	}

	return 0;
}

static int mscc_pinctrl_register(struct udevice *dev, struct mscc_pinctrl *info)
{
	int ret;

	ret = mscc_create_group_func_map(dev, info);
	if (ret) {
		dev_err(dev, "Unable to create group func map.\n");
		return ret;
	}

	return 0;
}

static int mscc_gpio_get(struct udevice *dev, unsigned int offset)
{
	struct mscc_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int val;

	if (mscc_readl(offset, info->regs + info->mscc_gpios[MSCC_GPIO_OE]) &
	    BIT(offset % 32))
		val = mscc_readl(offset,
				 info->regs + info->mscc_gpios[MSCC_GPIO_OUT]);
	else
		val = mscc_readl(offset,
				 info->regs + info->mscc_gpios[MSCC_GPIO_IN]);

	return !!(val & BIT(offset % 32));
}

static int mscc_gpio_set(struct udevice *dev, unsigned int offset, int value)
{
	struct mscc_pinctrl *info = dev_get_priv(dev->parent);

	if (value)
		mscc_writel(offset,
			    info->regs + info->mscc_gpios[MSCC_GPIO_OUT_SET]);
	else
		mscc_writel(offset,
			    info->regs + info->mscc_gpios[MSCC_GPIO_OUT_CLR]);

	return 0;
}

static int mscc_gpio_get_direction(struct udevice *dev, unsigned int offset)
{
	struct mscc_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int val;

	val = mscc_readl(offset, info->regs + info->mscc_gpios[MSCC_GPIO_OE]);

	return (val & BIT(offset % 32)) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int mscc_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct mscc_pinctrl *info = dev_get_priv(dev->parent);

	mscc_clrbits(offset, info->regs + info->mscc_gpios[MSCC_GPIO_OE]);

	return 0;
}

static int mscc_gpio_direction_output(struct udevice *dev,
				      unsigned int offset, int value)
{
	struct mscc_pinctrl *info = dev_get_priv(dev->parent);

	mscc_setbits(offset, info->regs + info->mscc_gpios[MSCC_GPIO_OE]);

	return mscc_gpio_set(dev, offset, value);
}

const struct dm_gpio_ops mscc_gpio_ops = {
	.set_value = mscc_gpio_set,
	.get_value = mscc_gpio_get,
	.get_function = mscc_gpio_get_direction,
	.direction_input = mscc_gpio_direction_input,
	.direction_output = mscc_gpio_direction_output,
};

const struct pinctrl_ops mscc_pinctrl_ops = {
	.get_pins_count = mscc_pctl_get_groups_count,
	.get_pin_name = mscc_pctl_get_group_name,
	.get_functions_count = mscc_get_functions_count,
	.get_function_name = mscc_get_function_name,
	.pinmux_set = mscc_pinmux_set_mux,
	.set_state = pinctrl_generic_set_state,
};

int mscc_pinctrl_probe(struct udevice *dev, int num_func,
		       const struct mscc_pin_data *mscc_pins, int num_pins,
		       char * const *function_names,
		       const unsigned long *mscc_gpios)
{
	struct mscc_pinctrl *priv = dev_get_priv(dev);
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->func = devm_kzalloc(dev, num_func * sizeof(struct mscc_pmx_func),
				  GFP_KERNEL);
	priv->num_func = num_func;
	priv->mscc_pins = mscc_pins;
	priv->num_pins = num_pins;
	priv->function_names = function_names;
	priv->mscc_gpios = mscc_gpios;
	ret = mscc_pinctrl_register(dev, priv);

	return ret;
}
