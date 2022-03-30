// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

#define UNIPHIER_PINCTRL_PINMUX_BASE	0x1000
#define UNIPHIER_PINCTRL_LOAD_PINMUX	0x1700
#define UNIPHIER_PINCTRL_DRVCTRL_BASE	0x1800
#define UNIPHIER_PINCTRL_DRV2CTRL_BASE	0x1900
#define UNIPHIER_PINCTRL_DRV3CTRL_BASE	0x1980
#define UNIPHIER_PINCTRL_PUPDCTRL_BASE	0x1a00
#define UNIPHIER_PINCTRL_IECTRL		0x1d00

static const char *uniphier_pinctrl_dummy_name = "_dummy";

static int uniphier_pinctrl_get_pins_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_pin *pins = priv->socdata->pins;
	int pins_count = priv->socdata->pins_count;

	/*
	 * We do not list all pins in the pin table to save memory footprint.
	 * Report the max pin number + 1 to fake the framework.
	 */
	return pins[pins_count - 1].number + 1;
}

static const char *uniphier_pinctrl_get_pin_name(struct udevice *dev,
						 unsigned int selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_pin *pins = priv->socdata->pins;
	int pins_count = priv->socdata->pins_count;
	int i;

	for (i = 0; i < pins_count; i++)
		if (pins[i].number == selector)
			return pins[i].name;

	return uniphier_pinctrl_dummy_name;
}

static int uniphier_pinctrl_get_groups_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->groups_count;
}

static const char *uniphier_pinctrl_get_group_name(struct udevice *dev,
						   unsigned selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->socdata->groups[selector].name)
		return uniphier_pinctrl_dummy_name;

	return priv->socdata->groups[selector].name;
}

static int uniphier_pinmux_get_functions_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->functions_count;
}

static const char *uniphier_pinmux_get_function_name(struct udevice *dev,
						     unsigned selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->socdata->functions[selector])
		return uniphier_pinctrl_dummy_name;

	return priv->socdata->functions[selector];
}

static int uniphier_pinconf_input_enable_perpin(struct udevice *dev,
						unsigned int pin, int enable)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned reg;
	u32 mask, tmp;

	reg = UNIPHIER_PINCTRL_IECTRL + pin / 32 * 4;
	mask = BIT(pin % 32);

	tmp = readl(priv->base + reg);
	if (enable)
		tmp |= mask;
	else
		tmp &= ~mask;
	writel(tmp, priv->base + reg);

	return 0;
}

static int uniphier_pinconf_input_enable_legacy(struct udevice *dev,
						unsigned int pin, int enable)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	/*
	 * Multiple pins share one input enable, per-pin disabling is
	 * impossible.
	 */
	if (!enable)
		return -EINVAL;

	/* Set all bits instead of having a bunch of pin data */
	writel(U32_MAX, priv->base + UNIPHIER_PINCTRL_IECTRL);

	return 0;
}

static int uniphier_pinconf_input_enable(struct udevice *dev,
					 unsigned int pin, int enable)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	if (priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL)
		return uniphier_pinconf_input_enable_perpin(dev, pin, enable);
	else
		return uniphier_pinconf_input_enable_legacy(dev, pin, enable);
}

#if CONFIG_IS_ENABLED(PINCONF)

static const struct pinconf_param uniphier_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "bias-pull-pin-default", PIN_CONFIG_BIAS_PULL_PIN_DEFAULT, 1 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
};

static const struct uniphier_pinctrl_pin *
uniphier_pinctrl_pin_get(struct uniphier_pinctrl_priv *priv, unsigned int pin)
{
	const struct uniphier_pinctrl_pin *pins = priv->socdata->pins;
	int pins_count = priv->socdata->pins_count;
	int i;

	for (i = 0; i < pins_count; i++)
		if (pins[i].number == pin)
			return &pins[i];

	return NULL;
}

static int uniphier_pinconf_bias_set(struct udevice *dev, unsigned int pin,
				     unsigned int param, unsigned int arg)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int enable = 1;
	unsigned int reg;
	u32 mask, tmp;

	if (!(priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE))
		return -ENOTSUPP;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		enable = 0;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (arg == 0)	/* total bias is not supported */
			return -EINVAL;
		break;
	case PIN_CONFIG_BIAS_PULL_PIN_DEFAULT:
		if (arg == 0)	/* configuration ignored */
			return 0;
	default:
		BUG();
	}

	reg = UNIPHIER_PINCTRL_PUPDCTRL_BASE + pin / 32 * 4;
	mask = BIT(pin % 32);

	tmp = readl(priv->base + reg);
	if (enable)
		tmp |= mask;
	else
		tmp &= ~mask;
	writel(tmp, priv->base + reg);

	return 0;
}

static const unsigned int uniphier_pinconf_drv_strengths_1bit[] = {
	4, 8,
};

static const unsigned int uniphier_pinconf_drv_strengths_2bit[] = {
	8, 12, 16, 20,
};

static const unsigned int uniphier_pinconf_drv_strengths_3bit[] = {
	4, 5, 7, 9, 11, 12, 14, 16,
};

static int uniphier_pinconf_drive_set(struct udevice *dev, unsigned int pin,
				      unsigned int strength)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_pin *desc;
	const unsigned int *strengths;
	unsigned int base, stride, width, drvctrl, reg, shift;
	u32 val, mask, tmp;

	desc = uniphier_pinctrl_pin_get(priv, pin);
	if (WARN_ON(!desc))
		return -EINVAL;

	switch (uniphier_pin_get_drv_type(desc->data)) {
	case UNIPHIER_PIN_DRV_1BIT:
		strengths = uniphier_pinconf_drv_strengths_1bit;
		base = UNIPHIER_PINCTRL_DRVCTRL_BASE;
		stride = 1;
		width = 1;
		break;
	case UNIPHIER_PIN_DRV_2BIT:
		strengths = uniphier_pinconf_drv_strengths_2bit;
		base = UNIPHIER_PINCTRL_DRV2CTRL_BASE;
		stride = 2;
		width = 2;
		break;
	case UNIPHIER_PIN_DRV_3BIT:
		strengths = uniphier_pinconf_drv_strengths_3bit;
		base = UNIPHIER_PINCTRL_DRV3CTRL_BASE;
		stride = 4;
		width = 3;
		break;
	default:
		/* drive strength control is not supported for this pin */
		return -EINVAL;
	}

	drvctrl = uniphier_pin_get_drvctrl(desc->data);
	drvctrl *= stride;

	reg = base + drvctrl / 32 * 4;
	shift = drvctrl % 32;
	mask = (1U << width) - 1;

	for (val = 0; val <= mask; val++) {
		if (strengths[val] > strength)
			break;
	}

	if (val == 0) {
		dev_err(dev, "unsupported drive strength %u mA for pin %s\n",
			strength, desc->name);
		return -EINVAL;
	}

	if (!mask)
		return 0;

	val--;

	tmp = readl(priv->base + reg);
	tmp &= ~(mask << shift);
	tmp |= (mask & val) << shift;
	writel(tmp, priv->base + reg);

	return 0;
}

static int uniphier_pinconf_set(struct udevice *dev, unsigned int pin,
				unsigned int param, unsigned int arg)
{
	int ret;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_PIN_DEFAULT:
		ret = uniphier_pinconf_bias_set(dev, pin, param, arg);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		ret = uniphier_pinconf_drive_set(dev, pin, arg);
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		ret = uniphier_pinconf_input_enable(dev, pin, arg);
		break;
	default:
		dev_err(dev, "unsupported configuration parameter %u\n", param);
		return -EINVAL;
	}

	return ret;
}

static int uniphier_pinconf_group_set(struct udevice *dev,
				      unsigned int group_selector,
				      unsigned int param, unsigned int arg)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_group *grp =
					&priv->socdata->groups[group_selector];
	int i, ret;

	for (i = 0; i < grp->num_pins; i++) {
		ret = uniphier_pinconf_set(dev, grp->pins[i], param, arg);
		if (ret)
			return ret;
	}

	return 0;
}

#endif /* CONFIG_IS_ENABLED(PINCONF) */

static void uniphier_pinmux_set_one(struct udevice *dev, unsigned pin,
				    int muxval)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned reg, reg_end, shift, mask;
	unsigned mux_bits = 8;
	unsigned reg_stride = 4;
	bool load_pinctrl = false;
	u32 tmp;

	/* some pins need input-enabling */
	uniphier_pinconf_input_enable(dev, pin, 1);

	if (muxval < 0)
		return;		/* dedicated pin; nothing to do for pin-mux */

	if (priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_MUX_4BIT)
		mux_bits = 4;

	if (priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE) {
		/*
		 *  Mode       offset        bit
		 *  Normal     4 * n     shift+3:shift
		 *  Debug      4 * n     shift+7:shift+4
		 */
		mux_bits /= 2;
		reg_stride = 8;
		load_pinctrl = true;
	}

	reg = UNIPHIER_PINCTRL_PINMUX_BASE + pin * mux_bits / 32 * reg_stride;
	reg_end = reg + reg_stride;
	shift = pin * mux_bits % 32;
	mask = (1U << mux_bits) - 1;

	/*
	 * If reg_stride is greater than 4, the MSB of each pinsel shall be
	 * stored in the offset+4.
	 */
	for (; reg < reg_end; reg += 4) {
		tmp = readl(priv->base + reg);
		tmp &= ~(mask << shift);
		tmp |= (mask & muxval) << shift;
		writel(tmp, priv->base + reg);

		muxval >>= mux_bits;
	}

	if (load_pinctrl)
		writel(1, priv->base + UNIPHIER_PINCTRL_LOAD_PINMUX);
}

static int uniphier_pinmux_group_set(struct udevice *dev,
				     unsigned group_selector,
				     unsigned func_selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_group *grp =
					&priv->socdata->groups[group_selector];
	int i;

	for (i = 0; i < grp->num_pins; i++)
		uniphier_pinmux_set_one(dev, grp->pins[i], grp->muxvals[i]);

	return 0;
}

const struct pinctrl_ops uniphier_pinctrl_ops = {
	.get_pins_count = uniphier_pinctrl_get_pins_count,
	.get_pin_name = uniphier_pinctrl_get_pin_name,
	.get_groups_count = uniphier_pinctrl_get_groups_count,
	.get_group_name = uniphier_pinctrl_get_group_name,
	.get_functions_count = uniphier_pinmux_get_functions_count,
	.get_function_name = uniphier_pinmux_get_function_name,
	.pinmux_group_set = uniphier_pinmux_group_set,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params = ARRAY_SIZE(uniphier_pinconf_params),
	.pinconf_params = uniphier_pinconf_params,
	.pinconf_set = uniphier_pinconf_set,
	.pinconf_group_set = uniphier_pinconf_group_set,
#endif
	.set_state = pinctrl_generic_set_state,
};

int uniphier_pinctrl_probe(struct udevice *dev,
			   struct uniphier_pinctrl_socdata *socdata)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_4K);
	if (!priv->base)
		return -ENOMEM;

	priv->socdata = socdata;

	return 0;
}
