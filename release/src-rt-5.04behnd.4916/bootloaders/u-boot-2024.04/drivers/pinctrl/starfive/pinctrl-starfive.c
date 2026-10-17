// SPDX-License-Identifier: GPL-2.0
/*
 * Pinctrl / GPIO driver for StarFive JH7100 SoC
 *
 * Copyright (C) 2022 Shanghai StarFive Technology Co., Ltd.
 *   Author: Lee Kuan Lim <kuanlim.lee@starfivetech.com>
 *   Author: Jianlong Huang <jianlong.huang@starfivetech.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <asm-generic/gpio.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm/device_compat.h>
#include <dt-bindings/pinctrl/pinctrl-starfive-jh7110.h>

#include "pinctrl-starfive.h"

/* pad control bits */
#define STARFIVE_PADCFG_POS	BIT(7)
#define STARFIVE_PADCFG_SMT	BIT(6)
#define STARFIVE_PADCFG_SLEW	BIT(5)
#define STARFIVE_PADCFG_PD	BIT(4)
#define STARFIVE_PADCFG_PU	BIT(3)
#define STARFIVE_PADCFG_BIAS	(STARFIVE_PADCFG_PD | STARFIVE_PADCFG_PU)
#define STARFIVE_PADCFG_DS_MASK	GENMASK(2, 1)
#define STARFIVE_PADCFG_DS_2MA	(0U << 1)
#define STARFIVE_PADCFG_DS_4MA	BIT(1)
#define STARFIVE_PADCFG_DS_8MA	(2U << 1)
#define STARFIVE_PADCFG_DS_12MA	(3U << 1)
#define STARFIVE_PADCFG_IE	BIT(0)
#define GPIO_NUM_PER_WORD	32

/*
 * The packed pinmux values from the device tree look like this:
 *
 *  | 31 - 24 | 23 - 16 | 15 - 10 |  9 - 8   | 7 - 0 |
 *  |   din   |  dout   |  doen   | function |  pin  |
 */
static unsigned int starfive_pinmux_din(u32 v)
{
	return (v & GENMASK(31, 24)) >> 24;
}

static u32 starfive_pinmux_dout(u32 v)
{
	return (v & GENMASK(23, 16)) >> 16;
}

static u32 starfive_pinmux_doen(u32 v)
{
	return (v & GENMASK(15, 10)) >> 10;
}

static u32 starfive_pinmux_function(u32 v)
{
	return (v & GENMASK(9, 8)) >> 8;
}

static unsigned int starfive_pinmux_pin(u32 v)
{
	return v & GENMASK(7, 0);
}

void starfive_set_gpiomux(struct udevice *dev, unsigned int pin,
			  unsigned int din, u32 dout, u32 doen)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);
	const struct starfive_pinctrl_soc_info *info = priv->info;

	unsigned int offset = 4 * (pin / 4);
	unsigned int shift  = 8 * (pin % 4);
	u32 dout_mask = info->dout_mask << shift;
	u32 done_mask = info->doen_mask << shift;
	u32 ival, imask;
	void __iomem *reg_dout;
	void __iomem *reg_doen;
	void __iomem *reg_din;

	reg_dout = priv->base + info->dout_reg_base + offset;
	reg_doen = priv->base + info->doen_reg_base + offset;
	dout <<= shift;
	doen <<= shift;
	if (din != GPI_NONE) {
		unsigned int ioffset = 4 * (din / 4);
		unsigned int ishift  = 8 * (din % 4);

		reg_din = priv->base + info->gpi_reg_base + ioffset;
		ival = (pin + 2) << ishift;
		imask = info->gpi_mask << ishift;
	} else {
		reg_din = NULL;
	}

	dout |= readl(reg_dout) & ~dout_mask;
	writel(dout, reg_dout);
	doen |= readl(reg_doen) & ~done_mask;
	writel(doen, reg_doen);
	if (reg_din) {
		ival |= readl(reg_din) & ~imask;
		writel(ival, reg_din);
	}
}

static const struct pinconf_param starfive_pinconf_params[] = {
	{ "bias-disable",	PIN_CONFIG_BIAS_DISABLE,	0 },
	{ "bias-pull-up",	PIN_CONFIG_BIAS_PULL_UP,	1 },
	{ "bias-pull-down",	PIN_CONFIG_BIAS_PULL_DOWN,	1 },
	{ "drive-strength",	PIN_CONFIG_DRIVE_STRENGTH,	0 },
	{ "input-schmitt-enable",  PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "input-enable",	PIN_CONFIG_INPUT_ENABLE,	1 },
	{ "input-disable",	PIN_CONFIG_INPUT_ENABLE,	0 },
	{ "slew-rate",		PIN_CONFIG_SLEW_RATE,		0 },
};

static const u8 starfive_drive_strength_mA[4] = { 2, 4, 8, 12 };

static u32 starfive_padcfg_ds_from_mA(u32 v)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (v <= starfive_drive_strength_mA[i])
			break;
	}
	return i << 1;
}

static void starfive_padcfg_rmw(struct udevice *dev,
				unsigned int pin, u32 mask, u32 value)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);
	struct starfive_pinctrl_soc_info *info = priv->info;
	void __iomem *reg;
	int padcfg_base;

	if (!info->get_padcfg_base)
		return;

	padcfg_base = info->get_padcfg_base(dev, pin);
	if (padcfg_base < 0)
		return;

	reg = priv->base + padcfg_base + 4 * pin;
	value &= mask;

	value |= readl(reg) & ~mask;
	writel(value, reg);
}

static int starfive_pinconf_set(struct udevice *dev, unsigned int pin,
				unsigned int param, unsigned int arg)
{
	u16 mask = 0;
	u16 value = 0;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		mask |= STARFIVE_PADCFG_BIAS;
		value &= ~STARFIVE_PADCFG_BIAS;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (arg == 0)
			return -EINVAL;
		mask |= STARFIVE_PADCFG_BIAS;
		value = (value & ~STARFIVE_PADCFG_BIAS) | STARFIVE_PADCFG_PD;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		if (arg == 0)
			return -EINVAL;
		mask |= STARFIVE_PADCFG_BIAS;
		value = (value & ~STARFIVE_PADCFG_BIAS) | STARFIVE_PADCFG_PU;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		mask |= STARFIVE_PADCFG_DS_MASK;
		value = (value & ~STARFIVE_PADCFG_DS_MASK) |
			starfive_padcfg_ds_from_mA(arg);
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		mask |= STARFIVE_PADCFG_IE;
		if (arg)
			value |= STARFIVE_PADCFG_IE;
		else
			value &= ~STARFIVE_PADCFG_IE;
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		mask |= STARFIVE_PADCFG_SMT;
		if (arg)
			value |= STARFIVE_PADCFG_SMT;
		else
			value &= ~STARFIVE_PADCFG_SMT;
		break;
	case PIN_CONFIG_SLEW_RATE:
		mask |= STARFIVE_PADCFG_SLEW;
		if (arg)
			value |= STARFIVE_PADCFG_SLEW;
		else
			value &= ~STARFIVE_PADCFG_SLEW;
		break;
	default:
		return -EINVAL;
	}

	starfive_padcfg_rmw(dev, pin, mask, value);

	return 0;
}

static int starfive_property_set(struct udevice *dev, u32 pinmux_group)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	if (info->set_one_pinmux)
		info->set_one_pinmux(dev,
			starfive_pinmux_pin(pinmux_group),
			starfive_pinmux_din(pinmux_group),
			starfive_pinmux_dout(pinmux_group),
			starfive_pinmux_doen(pinmux_group),
			starfive_pinmux_function(pinmux_group));

	return starfive_pinmux_pin(pinmux_group);
}

const struct pinctrl_ops starfive_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.pinconf_num_params	= ARRAY_SIZE(starfive_pinconf_params),
	.pinconf_params		= starfive_pinconf_params,
	.pinconf_set		= starfive_pinconf_set,
	.pinmux_property_set = starfive_property_set,
};

static int starfive_gpio_get_direction(struct udevice *dev, unsigned int off)
{
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	unsigned int offset = 4 * (off / 4);
	unsigned int shift  = 8 * (off % 4);
	u32 doen = readl(priv->base + info->doen_reg_base + offset);

	doen = (doen >> shift) & info->doen_mask;

	return doen == GPOEN_ENABLE ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int starfive_gpio_direction_input(struct udevice *dev, unsigned int off)
{
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	/* enable input and schmitt trigger */
	starfive_padcfg_rmw(pdev, off,
			    STARFIVE_PADCFG_IE | STARFIVE_PADCFG_SMT,
			    STARFIVE_PADCFG_IE | STARFIVE_PADCFG_SMT);

	if (info->set_one_pinmux)
		info->set_one_pinmux(pdev, off,
				GPI_NONE, GPOUT_LOW, GPOEN_DISABLE, 0);

	return 0;
}

static int starfive_gpio_direction_output(struct udevice *dev,
					  unsigned int off, int val)
{
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	if (info->set_one_pinmux)
		info->set_one_pinmux(pdev, off,
				GPI_NONE, val ? GPOUT_HIGH : GPOUT_LOW,
				GPOEN_ENABLE, 0);

	/* disable input, schmitt trigger and bias */
	starfive_padcfg_rmw(pdev, off,
			    STARFIVE_PADCFG_IE | STARFIVE_PADCFG_SMT
			    | STARFIVE_PADCFG_BIAS,
			    0);

	return 0;
}

static int starfive_gpio_get_value(struct udevice *dev, unsigned int off)
{
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	void __iomem *reg = priv->base + info->gpioin_reg_base
			+ 4 * (off / GPIO_NUM_PER_WORD);

	return !!(readl(reg) & BIT(off % GPIO_NUM_PER_WORD));
}

static int starfive_gpio_set_value(struct udevice *dev,
				   unsigned int off, int val)
{
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	unsigned int offset = 4 * (off / 4);
	unsigned int shift  = 8 * (off % 4);
	void __iomem *reg_dout = priv->base + info->dout_reg_base + offset;
	u32 dout = (val ? GPOUT_HIGH : GPOUT_LOW) << shift;
	u32 mask = info->dout_mask << shift;

	dout |= readl(reg_dout) & ~mask;
	writel(dout, reg_dout);

	return 0;
}

static int starfive_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *pdev = dev->parent;
	struct starfive_pinctrl_priv *priv = dev_get_priv(pdev);
	struct starfive_pinctrl_soc_info *info = priv->info;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = info->gpio_bank_name;
	uc_priv->gpio_count = info->ngpios;

	if (!info->gpio_init_hw)
		return -ENXIO;

	info->gpio_init_hw(pdev);

	return 0;
}

static const struct dm_gpio_ops starfive_gpio_ops = {
	.get_function = starfive_gpio_get_direction,
	.direction_input = starfive_gpio_direction_input,
	.direction_output = starfive_gpio_direction_output,
	.get_value = starfive_gpio_get_value,
	.set_value = starfive_gpio_set_value,
};

static struct driver starfive_gpio_driver = {
	.name = "starfive_gpio",
	.id = UCLASS_GPIO,
	.probe = starfive_gpio_probe,
	.ops = &starfive_gpio_ops,
};

static int starfive_gpiochip_register(struct udevice *parent)
{
	struct uclass_driver *drv;
	struct udevice *dev;
	int ret;
	ofnode node;

	drv = lists_uclass_lookup(UCLASS_GPIO);
	if (!drv)
		return -ENOENT;

	node = dev_ofnode(parent);
	ret = device_bind_with_driver_data(parent, &starfive_gpio_driver,
					   "starfive_gpio", 0, node, &dev);

	return (ret == 0) ? 0 : ret;
}

int starfive_pinctrl_probe(struct udevice *dev,
			   const struct starfive_pinctrl_soc_info *info)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);
	int ret;

	/* Bind pinctrl_info from .data to priv */
	priv->info =
		(struct starfive_pinctrl_soc_info *)dev_get_driver_data(dev);

	if (!priv->info)
		return -EINVAL;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	/* gpiochip register */
	ret = starfive_gpiochip_register(dev);

	return (ret == 0) ? 0 : ret;
}
