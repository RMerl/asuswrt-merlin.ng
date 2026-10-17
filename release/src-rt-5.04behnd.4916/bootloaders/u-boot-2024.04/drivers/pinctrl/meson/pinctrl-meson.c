// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <fdt_support.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/gpio.h>

#include "pinctrl-meson.h"

DECLARE_GLOBAL_DATA_PTR;

static const char *meson_pinctrl_dummy_name = "_dummy";

static char pin_name[PINNAME_SIZE];

int meson_pinctrl_get_groups_count(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->num_groups;
}

const char *meson_pinctrl_get_group_name(struct udevice *dev,
					 unsigned int selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	if (!priv->data->groups[selector].name)
		return meson_pinctrl_dummy_name;

	return priv->data->groups[selector].name;
}

int meson_pinctrl_get_pins_count(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->num_pins;
}

const char *meson_pinctrl_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	if (selector > priv->data->num_pins ||
	    selector > priv->data->funcs[0].num_groups)
		snprintf(pin_name, PINNAME_SIZE, "Error");
	else
		snprintf(pin_name, PINNAME_SIZE, "%s",
			 priv->data->funcs[0].groups[selector]);

	return pin_name;
}

int meson_pinmux_get_functions_count(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->num_funcs;
}

const char *meson_pinmux_get_function_name(struct udevice *dev,
					   unsigned int selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->funcs[selector].name;
}

static int meson_gpio_calc_reg_and_bit(struct udevice *dev, unsigned int offset,
				       enum meson_reg_type reg_type,
				       unsigned int *reg, unsigned int *bit)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	struct meson_bank *bank = NULL;
	struct meson_reg_desc *desc;
	unsigned int pin;
	int i;

	pin = priv->data->pin_base + offset;

	for (i = 0; i < priv->data->num_banks; i++) {
		if (pin >= priv->data->banks[i].first &&
		    pin <= priv->data->banks[i].last) {
			bank = &priv->data->banks[i];
			break;
		}
	}

	if (!bank)
		return -EINVAL;

	desc = &bank->regs[reg_type];
	*reg = desc->reg * 4;
	*bit = desc->bit + pin - bank->first;

	return 0;
}

int meson_gpio_get(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_IN, &reg,
					  &bit);
	if (ret)
		return ret;

	return !!(readl(priv->reg_gpio + reg) & BIT(bit));
}

int meson_gpio_set(struct udevice *dev, unsigned int offset, int value)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_OUT, &reg,
					  &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), value ? BIT(bit) : 0);

	return 0;
}

int meson_gpio_get_direction(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit, val;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_DIR, &reg,
					  &bit);
	if (ret)
		return ret;

	val = readl(priv->reg_gpio + reg);

	return (val & BIT(bit)) ? GPIOF_INPUT : GPIOF_OUTPUT;
}

int meson_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_DIR, &reg,
					  &bit);
	if (ret)
		return ret;

	setbits_le32(priv->reg_gpio + reg, BIT(bit));

	return 0;
}

int meson_gpio_direction_output(struct udevice *dev,
				unsigned int offset, int value)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_DIR, &reg,
					  &bit);
	if (ret)
		return ret;

	clrbits_le32(priv->reg_gpio + reg, BIT(bit));

	ret = meson_gpio_calc_reg_and_bit(dev->parent, offset, REG_OUT, &reg,
					  &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), value ? BIT(bit) : 0);

	return 0;
}

static int meson_pinconf_bias_set(struct udevice *dev, unsigned int pin,
				  unsigned int param)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	unsigned int offset = pin - priv->data->pin_base;
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_PULLEN, &reg, &bit);
	if (ret)
		return ret;

	if (param == PIN_CONFIG_BIAS_DISABLE) {
		clrsetbits_le32(priv->reg_pullen + reg, BIT(bit), 0);
		return 0;
	}

	/* othewise, enable the bias and select level */
	clrsetbits_le32(priv->reg_pullen + reg, BIT(bit), BIT(bit));
	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_PULL, &reg, &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_pull + reg, BIT(bit),
			(param == PIN_CONFIG_BIAS_PULL_UP ? BIT(bit) : 0));

	return 0;
}

static int meson_pinconf_drive_strength_set(struct udevice *dev,
					    unsigned int pin,
					    unsigned int drive_strength_ua)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	unsigned int offset = pin - priv->data->pin_base;
	unsigned int reg, bit;
	unsigned int ds_val;
	int ret;

	if (!priv->reg_ds) {
		dev_err(dev, "drive-strength-microamp not supported\n");
		return -ENOTSUPP;
	}

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_DS, &reg, &bit);
	if (ret)
		return ret;

	bit = bit << 1;

	if (drive_strength_ua <= 500) {
		ds_val = MESON_PINCONF_DRV_500UA;
	} else if (drive_strength_ua <= 2500) {
		ds_val = MESON_PINCONF_DRV_2500UA;
	} else if (drive_strength_ua <= 3000) {
		ds_val = MESON_PINCONF_DRV_3000UA;
	} else if (drive_strength_ua <= 4000) {
		ds_val = MESON_PINCONF_DRV_4000UA;
	} else {
		dev_warn(dev,
			 "pin %u: invalid drive-strength-microamp : %d , default to 4mA\n",
			 pin, drive_strength_ua);
		ds_val = MESON_PINCONF_DRV_4000UA;
	}

	clrsetbits_le32(priv->reg_ds + reg, 0x3 << bit, ds_val << bit);

	return 0;
}

int meson_pinconf_set(struct udevice *dev, unsigned int pin,
		      unsigned int param, unsigned int arg)
{
	int ret;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
		ret = meson_pinconf_bias_set(dev, pin, param);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH_UA:
		ret = meson_pinconf_drive_strength_set(dev, pin, arg);
		break;
	default:
		dev_err(dev, "unsupported configuration parameter %u\n", param);
		return -EINVAL;
	}

	return ret;
}

int meson_pinconf_group_set(struct udevice *dev,
			    unsigned int group_selector,
			    unsigned int param, unsigned int arg)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	struct meson_pmx_group *grp = &priv->data->groups[group_selector];
	int i, ret;

	for (i = 0; i < grp->num_pins; i++) {
		ret = meson_pinconf_set(dev, grp->pins[i], param, arg);
		if (ret)
			return ret;
	}

	return 0;
}

int meson_gpio_probe(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = priv->data->name;
	uc_priv->gpio_count = priv->data->num_pins;

	return 0;
}

static fdt_addr_t parse_address(int offset, const char *name, int na, int ns)
{
	int index, len = 0;
	const fdt32_t *reg;

	index = fdt_stringlist_search(gd->fdt_blob, offset, "reg-names", name);
	if (index < 0)
		return FDT_ADDR_T_NONE;

	reg = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (!reg || (len <= (index * sizeof(fdt32_t) * (na + ns))))
		return FDT_ADDR_T_NONE;

	reg += index * (na + ns);

	return fdt_translate_address((void *)gd->fdt_blob, offset, reg);
}

int meson_pinctrl_probe(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	struct uclass_driver *drv;
	struct udevice *gpio_dev;
	fdt_addr_t addr;
	int node, gpio = -1, len;
	int na, ns;
	char *name;

	/* FIXME: Should use livetree */
	na = fdt_address_cells(gd->fdt_blob, dev_of_offset(dev->parent));
	if (na < 1) {
		debug("bad #address-cells\n");
		return -EINVAL;
	}

	ns = fdt_size_cells(gd->fdt_blob, dev_of_offset(dev->parent));
	if (ns < 1) {
		debug("bad #size-cells\n");
		return -EINVAL;
	}

	fdt_for_each_subnode(node, gd->fdt_blob, dev_of_offset(dev)) {
		if (fdt_getprop(gd->fdt_blob, node, "gpio-controller", &len)) {
			gpio = node;
			break;
		}
	}

	if (!gpio) {
		debug("gpio node not found\n");
		return -EINVAL;
	}

	addr = parse_address(gpio, "mux", na, ns);
	if (addr == FDT_ADDR_T_NONE) {
		debug("mux address not found\n");
		return -EINVAL;
	}
	priv->reg_mux = (void __iomem *)addr;

	addr = parse_address(gpio, "gpio", na, ns);
	if (addr == FDT_ADDR_T_NONE) {
		debug("gpio address not found\n");
		return -EINVAL;
	}
	priv->reg_gpio = (void __iomem *)addr;

	addr = parse_address(gpio, "pull", na, ns);
	/* Use gpio region if pull one is not present */
	if (addr == FDT_ADDR_T_NONE)
		priv->reg_pull = priv->reg_gpio;
	else
		priv->reg_pull = (void __iomem *)addr;

	addr = parse_address(gpio, "pull-enable", na, ns);
	/* Use pull region if pull-enable one is not present */
	if (addr == FDT_ADDR_T_NONE)
		priv->reg_pullen = priv->reg_pull;
	else
		priv->reg_pullen = (void __iomem *)addr;

	addr = parse_address(gpio, "ds", na, ns);
	/* Drive strength region is optional */
	if (addr == FDT_ADDR_T_NONE)
		priv->reg_ds = NULL;
	else
		priv->reg_ds = (void __iomem *)addr;

	priv->data = (struct meson_pinctrl_data *)dev_get_driver_data(dev);

	/* Lookup GPIO driver */
	drv = lists_uclass_lookup(UCLASS_GPIO);
	if (!drv) {
		puts("Cannot find GPIO driver\n");
		return -ENOENT;
	}

	name = calloc(1, 32);
	sprintf(name, "meson-gpio");

	/* Create child device UCLASS_GPIO and bind it */
	device_bind(dev, priv->data->gpio_driver, name, NULL,
		    offset_to_ofnode(gpio), &gpio_dev);

	return 0;
}
