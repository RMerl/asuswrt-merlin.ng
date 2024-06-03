// SPDX-License-Identifier: GPL-2.0+
/*
 * U-Boot Marvell 37xx SoC pinctrl driver
 *
 * Copyright (C) 2017 Stefan Roese <sr@denx.de>
 *
 * This driver is based on the Linux driver version, which is:
 * Copyright (C) 2017 Marvell
 * Gregory CLEMENT <gregory.clement@free-electrons.com>
 *
 * Additionally parts are derived from the Meson U-Boot pinctrl driver,
 * which is:
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 * Based on code from Linux kernel:
 * Copyright (C) 2016 Endless Mobile, Inc.
 * https://spdx.org/licenses
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <errno.h>
#include <fdtdec.h>
#include <regmap.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define OUTPUT_EN	0x0
#define INPUT_VAL	0x10
#define OUTPUT_VAL	0x18
#define OUTPUT_CTL	0x20
#define SELECTION	0x30

#define IRQ_EN		0x0
#define IRQ_POL		0x08
#define IRQ_STATUS	0x10
#define IRQ_WKUP	0x18

#define NB_FUNCS 3
#define GPIO_PER_REG	32

/**
 * struct armada_37xx_pin_group: represents group of pins of a pinmux function.
 * The pins of a pinmux groups are composed of one or two groups of contiguous
 * pins.
 * @name:	Name of the pin group, used to lookup the group.
 * @start_pins:	Index of the first pin of the main range of pins belonging to
 *		the group
 * @npins:	Number of pins included in the first range
 * @reg_mask:	Bit mask matching the group in the selection register
 * @extra_pins:	Index of the first pin of the optional second range of pins
 *		belonging to the group
 * @npins:	Number of pins included in the second optional range
 * @funcs:	A list of pinmux functions that can be selected for this group.
 * @pins:	List of the pins included in the group
 */
struct armada_37xx_pin_group {
	const char	*name;
	unsigned int	start_pin;
	unsigned int	npins;
	u32		reg_mask;
	u32		val[NB_FUNCS];
	unsigned int	extra_pin;
	unsigned int	extra_npins;
	const char	*funcs[NB_FUNCS];
	unsigned int	*pins;
};

struct armada_37xx_pin_data {
	u8				nr_pins;
	char				*name;
	struct armada_37xx_pin_group	*groups;
	int				ngroups;
};

struct armada_37xx_pmx_func {
	const char		*name;
	const char		**groups;
	unsigned int		ngroups;
};

struct armada_37xx_pinctrl {
	void __iomem			*base;
	const struct armada_37xx_pin_data	*data;
	struct udevice			*dev;
	struct pinctrl_dev		*pctl_dev;
	struct armada_37xx_pin_group	*groups;
	unsigned int			ngroups;
	struct armada_37xx_pmx_func	*funcs;
	unsigned int			nfuncs;
};

#define PIN_GRP(_name, _start, _nr, _mask, _func1, _func2)	\
	{					\
		.name = _name,			\
		.start_pin = _start,		\
		.npins = _nr,			\
		.reg_mask = _mask,		\
		.val = {0, _mask},		\
		.funcs = {_func1, _func2}	\
	}

#define PIN_GRP_GPIO(_name, _start, _nr, _mask, _func1)	\
	{					\
		.name = _name,			\
		.start_pin = _start,		\
		.npins = _nr,			\
		.reg_mask = _mask,		\
		.val = {0, _mask},		\
		.funcs = {_func1, "gpio"}	\
	}

#define PIN_GRP_GPIO_2(_name, _start, _nr, _mask, _val1, _val2, _func1)   \
	{					\
		.name = _name,			\
		.start_pin = _start,		\
		.npins = _nr,			\
		.reg_mask = _mask,		\
		.val = {_val1, _val2},		\
		.funcs = {_func1, "gpio"}	\
	}

#define PIN_GRP_GPIO_3(_name, _start, _nr, _mask, _v1, _v2, _v3, _f1, _f2) \
	{					\
		.name = _name,			\
		.start_pin = _start,		\
		.npins = _nr,			\
		.reg_mask = _mask,		\
		.val = {_v1, _v2, _v3},	\
		.funcs = {_f1, _f2, "gpio"}	\
	}

#define PIN_GRP_EXTRA(_name, _start, _nr, _mask, _v1, _v2, _start2, _nr2, \
		      _f1, _f2)				\
	{						\
		.name = _name,				\
		.start_pin = _start,			\
		.npins = _nr,				\
		.reg_mask = _mask,			\
		.val = {_v1, _v2},			\
		.extra_pin = _start2,			\
		.extra_npins = _nr2,			\
		.funcs = {_f1, _f2}			\
	}

static struct armada_37xx_pin_group armada_37xx_nb_groups[] = {
	PIN_GRP_GPIO("jtag", 20, 5, BIT(0), "jtag"),
	PIN_GRP_GPIO("sdio0", 8, 3, BIT(1), "sdio"),
	PIN_GRP_GPIO("emmc_nb", 27, 9, BIT(2), "emmc"),
	PIN_GRP_GPIO("pwm0", 11, 1, BIT(3), "pwm"),
	PIN_GRP_GPIO("pwm1", 12, 1, BIT(4), "pwm"),
	PIN_GRP_GPIO("pwm2", 13, 1, BIT(5), "pwm"),
	PIN_GRP_GPIO("pwm3", 14, 1, BIT(6), "pwm"),
	PIN_GRP_GPIO("pmic1", 7, 1, BIT(7), "pmic"),
	PIN_GRP_GPIO("pmic0", 6, 1, BIT(8), "pmic"),
	PIN_GRP_GPIO("i2c2", 2, 2, BIT(9), "i2c"),
	PIN_GRP_GPIO("i2c1", 0, 2, BIT(10), "i2c"),
	PIN_GRP_GPIO("spi_cs1", 17, 1, BIT(12), "spi"),
	PIN_GRP_GPIO_2("spi_cs2", 18, 1, BIT(13) | BIT(19), 0, BIT(13), "spi"),
	PIN_GRP_GPIO_2("spi_cs3", 19, 1, BIT(14) | BIT(19), 0, BIT(14), "spi"),
	PIN_GRP_GPIO("onewire", 4, 1, BIT(16), "onewire"),
	PIN_GRP_GPIO("uart1", 25, 2, BIT(17), "uart"),
	PIN_GRP_GPIO("spi_quad", 15, 2, BIT(18), "spi"),
	PIN_GRP_EXTRA("uart2", 9, 2, BIT(1) | BIT(13) | BIT(14) | BIT(19),
		      BIT(1) | BIT(13) | BIT(14), BIT(1) | BIT(19),
		      18, 2, "gpio", "uart"),
	PIN_GRP_GPIO("led0_od", 11, 1, BIT(20), "led"),
	PIN_GRP_GPIO("led1_od", 12, 1, BIT(21), "led"),
	PIN_GRP_GPIO("led2_od", 13, 1, BIT(22), "led"),
	PIN_GRP_GPIO("led3_od", 14, 1, BIT(23), "led"),

};

static struct armada_37xx_pin_group armada_37xx_sb_groups[] = {
	PIN_GRP_GPIO("usb32_drvvbus0", 0, 1, BIT(0), "drvbus"),
	PIN_GRP_GPIO("usb2_drvvbus1", 1, 1, BIT(1), "drvbus"),
	PIN_GRP_GPIO("sdio_sb", 24, 6, BIT(2), "sdio"),
	PIN_GRP_GPIO("rgmii", 6, 12, BIT(3), "mii"),
	PIN_GRP_GPIO("smi", 18, 2, BIT(4), "smi"),
	PIN_GRP_GPIO("pcie1", 3, 3, BIT(5) | BIT(9) | BIT(10), "pcie"),
	PIN_GRP_GPIO("ptp", 20, 3, BIT(11) | BIT(12) | BIT(13), "ptp"),
	PIN_GRP("ptp_clk", 21, 1, BIT(6), "ptp", "mii"),
	PIN_GRP("ptp_trig", 22, 1, BIT(7), "ptp", "mii"),
	PIN_GRP_GPIO_3("mii_col", 23, 1, BIT(8) | BIT(14), 0, BIT(8), BIT(14),
		       "mii", "mii_err"),
};

const struct armada_37xx_pin_data armada_37xx_pin_nb = {
	.nr_pins = 36,
	.name = "GPIO1",
	.groups = armada_37xx_nb_groups,
	.ngroups = ARRAY_SIZE(armada_37xx_nb_groups),
};

const struct armada_37xx_pin_data armada_37xx_pin_sb = {
	.nr_pins = 30,
	.name = "GPIO2",
	.groups = armada_37xx_sb_groups,
	.ngroups = ARRAY_SIZE(armada_37xx_sb_groups),
};

static inline void armada_37xx_update_reg(unsigned int *reg,
					  unsigned int *offset)
{
	/* We never have more than 2 registers */
	if (*offset >= GPIO_PER_REG) {
		*offset -= GPIO_PER_REG;
		*reg += sizeof(u32);
	}
}

static int armada_37xx_get_func_reg(struct armada_37xx_pin_group *grp,
				    const char *func)
{
	int f;

	for (f = 0; (f < NB_FUNCS) && grp->funcs[f]; f++)
		if (!strcmp(grp->funcs[f], func))
			return f;

	return -ENOTSUPP;
}

static int armada_37xx_pmx_get_groups_count(struct udevice *dev)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);

	return info->ngroups;
}

static const char *armada_37xx_pmx_dummy_name = "_dummy";

static const char *armada_37xx_pmx_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);

	if (!info->groups[selector].name)
		return armada_37xx_pmx_dummy_name;

	return info->groups[selector].name;
}

static int armada_37xx_pmx_get_funcs_count(struct udevice *dev)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);

	return info->nfuncs;
}

static const char *armada_37xx_pmx_get_func_name(struct udevice *dev,
						 unsigned selector)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);

	return info->funcs[selector].name;
}

static int armada_37xx_pmx_set_by_name(struct udevice *dev,
				       const char *name,
				       struct armada_37xx_pin_group *grp)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);
	unsigned int reg = SELECTION;
	unsigned int mask = grp->reg_mask;
	int func, val;

	dev_dbg(info->dev, "enable function %s group %s\n",
		name, grp->name);

	func = armada_37xx_get_func_reg(grp, name);

	if (func < 0)
		return func;

	val = grp->val[func];

	clrsetbits_le32(info->base + reg, mask, val);

	return 0;
}

static int armada_37xx_pmx_group_set(struct udevice *dev,
				     unsigned group_selector,
				     unsigned func_selector)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);
	struct armada_37xx_pin_group *grp = &info->groups[group_selector];
	const char *name = info->funcs[func_selector].name;

	return armada_37xx_pmx_set_by_name(dev, name, grp);
}

/**
 * armada_37xx_add_function() - Add a new function to the list
 * @funcs: array of function to add the new one
 * @funcsize: size of the remaining space for the function
 * @name: name of the function to add
 *
 * If it is a new function then create it by adding its name else
 * increment the number of group associated to this function.
 */
static int armada_37xx_add_function(struct armada_37xx_pmx_func *funcs,
				    int *funcsize, const char *name)
{
	int i = 0;

	if (*funcsize <= 0)
		return -EOVERFLOW;

	while (funcs->ngroups) {
		/* function already there */
		if (strcmp(funcs->name, name) == 0) {
			funcs->ngroups++;

			return -EEXIST;
		}
		funcs++;
		i++;
	}

	/* append new unique function */
	funcs->name = name;
	funcs->ngroups = 1;
	(*funcsize)--;

	return 0;
}

/**
 * armada_37xx_fill_group() - complete the group array
 * @info: info driver instance
 *
 * Based on the data available from the armada_37xx_pin_group array
 * completes the last member of the struct for each function: the list
 * of the groups associated to this function.
 *
 */
static int armada_37xx_fill_group(struct armada_37xx_pinctrl *info)
{
	int n, num = 0, funcsize = info->data->nr_pins;

	for (n = 0; n < info->ngroups; n++) {
		struct armada_37xx_pin_group *grp = &info->groups[n];
		int i, j, f;

		grp->pins = devm_kzalloc(info->dev,
					 (grp->npins + grp->extra_npins) *
					 sizeof(*grp->pins), GFP_KERNEL);
		if (!grp->pins)
			return -ENOMEM;

		for (i = 0; i < grp->npins; i++)
			grp->pins[i] = grp->start_pin + i;

		for (j = 0; j < grp->extra_npins; j++)
			grp->pins[i+j] = grp->extra_pin + j;

		for (f = 0; (f < NB_FUNCS) && grp->funcs[f]; f++) {
			int ret;
			/* check for unique functions and count groups */
			ret = armada_37xx_add_function(info->funcs, &funcsize,
					    grp->funcs[f]);
			if (ret == -EOVERFLOW)
				dev_err(info->dev,
					"More functions than pins(%d)\n",
					info->data->nr_pins);
			if (ret < 0)
				continue;
			num++;
		}
	}

	info->nfuncs = num;

	return 0;
}

/**
 * armada_37xx_fill_funcs() - complete the funcs array
 * @info: info driver instance
 *
 * Based on the data available from the armada_37xx_pin_group array
 * completes the last two member of the struct for each group:
 * - the list of the pins included in the group
 * - the list of pinmux functions that can be selected for this group
 *
 */
static int armada_37xx_fill_func(struct armada_37xx_pinctrl *info)
{
	struct armada_37xx_pmx_func *funcs = info->funcs;
	int n;

	for (n = 0; n < info->nfuncs; n++) {
		const char *name = funcs[n].name;
		const char **groups;
		int g;

		funcs[n].groups = devm_kzalloc(info->dev, funcs[n].ngroups *
					       sizeof(*(funcs[n].groups)),
					       GFP_KERNEL);
		if (!funcs[n].groups)
			return -ENOMEM;

		groups = funcs[n].groups;

		for (g = 0; g < info->ngroups; g++) {
			struct armada_37xx_pin_group *gp = &info->groups[g];
			int f;

			for (f = 0; (f < NB_FUNCS) && gp->funcs[f]; f++) {
				if (strcmp(gp->funcs[f], name) == 0) {
					*groups = gp->name;
					groups++;
				}
			}
		}
	}
	return 0;
}

static int armada_37xx_gpio_get(struct udevice *dev, unsigned int offset)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int reg = INPUT_VAL;
	unsigned int val, mask;

	armada_37xx_update_reg(&reg, &offset);
	mask = BIT(offset);

	val = readl(info->base + reg);

	return (val & mask) != 0;
}

static int armada_37xx_gpio_set(struct udevice *dev, unsigned int offset,
				int value)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int reg = OUTPUT_VAL;
	unsigned int mask, val;

	armada_37xx_update_reg(&reg, &offset);
	mask = BIT(offset);
	val = value ? mask : 0;

	clrsetbits_le32(info->base + reg, mask, val);

	return 0;
}

static int armada_37xx_gpio_get_direction(struct udevice *dev,
					  unsigned int offset)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int reg = OUTPUT_EN;
	unsigned int val, mask;

	armada_37xx_update_reg(&reg, &offset);
	mask = BIT(offset);
	val = readl(info->base + reg);

	if (val & mask)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int armada_37xx_gpio_direction_input(struct udevice *dev,
					    unsigned int offset)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int reg = OUTPUT_EN;
	unsigned int mask;

	armada_37xx_update_reg(&reg, &offset);
	mask = BIT(offset);

	clrbits_le32(info->base + reg, mask);

	return 0;
}

static int armada_37xx_gpio_direction_output(struct udevice *dev,
					     unsigned int offset, int value)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	unsigned int reg = OUTPUT_EN;
	unsigned int mask;

	armada_37xx_update_reg(&reg, &offset);
	mask = BIT(offset);

	setbits_le32(info->base + reg, mask);

	/* And set the requested value */
	return armada_37xx_gpio_set(dev, offset, value);
}

static int armada_37xx_gpio_probe(struct udevice *dev)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = info->data->name;
	uc_priv->gpio_count = info->data->nr_pins;

	return 0;
}

static const struct dm_gpio_ops armada_37xx_gpio_ops = {
	.set_value = armada_37xx_gpio_set,
	.get_value = armada_37xx_gpio_get,
	.get_function = armada_37xx_gpio_get_direction,
	.direction_input = armada_37xx_gpio_direction_input,
	.direction_output = armada_37xx_gpio_direction_output,
};

static struct driver armada_37xx_gpio_driver = {
	.name	= "armada-37xx-gpio",
	.id	= UCLASS_GPIO,
	.probe	= armada_37xx_gpio_probe,
	.ops	= &armada_37xx_gpio_ops,
};

static int armada_37xx_gpiochip_register(struct udevice *parent,
					 struct armada_37xx_pinctrl *info)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(parent);
	struct uclass_driver *drv;
	struct udevice *dev;
	int ret = -ENODEV;
	int subnode;
	char *name;

	/* Lookup GPIO driver */
	drv = lists_uclass_lookup(UCLASS_GPIO);
	if (!drv) {
		puts("Cannot find GPIO driver\n");
		return -ENOENT;
	}

	fdt_for_each_subnode(subnode, blob, node) {
		if (fdtdec_get_bool(blob, subnode, "gpio-controller")) {
			ret = 0;
			break;
		}
	};
	if (ret)
		return ret;

	name = calloc(1, 32);
	sprintf(name, "armada-37xx-gpio");

	/* Create child device UCLASS_GPIO and bind it */
	device_bind(parent, &armada_37xx_gpio_driver, name, NULL, subnode,
		    &dev);
	dev_set_of_offset(dev, subnode);

	return 0;
}

const struct pinctrl_ops armada_37xx_pinctrl_ops  = {
	.get_groups_count = armada_37xx_pmx_get_groups_count,
	.get_group_name = armada_37xx_pmx_get_group_name,
	.get_functions_count = armada_37xx_pmx_get_funcs_count,
	.get_function_name = armada_37xx_pmx_get_func_name,
	.pinmux_group_set = armada_37xx_pmx_group_set,
	.set_state = pinctrl_generic_set_state,
};

int armada_37xx_pinctrl_probe(struct udevice *dev)
{
	struct armada_37xx_pinctrl *info = dev_get_priv(dev);
	const struct armada_37xx_pin_data *pin_data;
	int ret;

	info->data = (struct armada_37xx_pin_data *)dev_get_driver_data(dev);
	pin_data = info->data;

	info->base = (void __iomem *)devfdt_get_addr(dev);
	if (!info->base) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	info->groups = pin_data->groups;
	info->ngroups = pin_data->ngroups;

	/*
	 * we allocate functions for number of pins and hope there are
	 * fewer unique functions than pins available
	 */
	info->funcs = devm_kzalloc(info->dev, pin_data->nr_pins *
			   sizeof(struct armada_37xx_pmx_func), GFP_KERNEL);
	if (!info->funcs)
		return -ENOMEM;


	ret = armada_37xx_fill_group(info);
	if (ret)
		return ret;

	ret = armada_37xx_fill_func(info);
	if (ret)
		return ret;

	ret = armada_37xx_gpiochip_register(dev, info);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id armada_37xx_pinctrl_of_match[] = {
	{
		.compatible = "marvell,armada3710-sb-pinctrl",
		.data = (ulong)&armada_37xx_pin_sb,
	},
	{
		.compatible = "marvell,armada3710-nb-pinctrl",
		.data = (ulong)&armada_37xx_pin_nb,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(armada_37xx_pinctrl) = {
	.name = "armada-37xx-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(armada_37xx_pinctrl_of_match),
	.probe = armada_37xx_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct armada_37xx_pinctrl),
	.ops = &armada_37xx_pinctrl_ops,
};
