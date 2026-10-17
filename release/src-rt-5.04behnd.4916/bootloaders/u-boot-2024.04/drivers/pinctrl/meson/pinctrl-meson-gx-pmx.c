// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#include <log.h>
#include <asm/gpio.h>
#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include "pinctrl-meson-gx.h"

static void meson_gx_pinmux_disable_other_groups(struct meson_pinctrl *priv,
						 unsigned int pin,
						 int sel_group)
{
	struct meson_pmx_group *group;
	struct meson_gx_pmx_data *pmx_data;
	void __iomem *addr;
	int i, j;

	for (i = 0; i < priv->data->num_groups; i++) {
		group = &priv->data->groups[i];
		pmx_data = (struct meson_gx_pmx_data *)group->data;
		if (pmx_data->is_gpio || i == sel_group)
			continue;

		for (j = 0; j < group->num_pins; j++) {
			if (group->pins[j] == pin) {
				/* We have found a group using the pin */
				debug("pinmux: disabling %s\n", group->name);
				addr = priv->reg_mux + pmx_data->reg * 4;
				writel(readl(addr) & ~BIT(pmx_data->bit), addr);
			}
		}
	}
}

static int meson_gx_pinmux_group_set(struct udevice *dev,
				     unsigned int group_selector,
				     unsigned int func_selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	const struct meson_pmx_group *group;
	const struct meson_pmx_func *func;
	struct meson_gx_pmx_data *pmx_data;
	void __iomem *addr;
	int i;

	group = &priv->data->groups[group_selector];
	pmx_data = (struct meson_gx_pmx_data *)group->data;
	func = &priv->data->funcs[func_selector];

	debug("pinmux: set group %s func %s\n", group->name, func->name);

	/*
	 * Disable groups using the same pins.
	 * The selected group is not disabled to avoid glitches.
	 */
	for (i = 0; i < group->num_pins; i++) {
		meson_gx_pinmux_disable_other_groups(priv,
						     group->pins[i],
						     group_selector);
	}

	/* Function 0 (GPIO) doesn't need any additional setting */
	if (func_selector) {
		addr = priv->reg_mux + pmx_data->reg * 4;
		writel(readl(addr) | BIT(pmx_data->bit), addr);
	}

	return 0;
}

static int meson_gx_pinmux_get(struct udevice *dev,
				      unsigned int selector,
				      char *buf, int size)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	struct meson_pmx_group *group;
	struct meson_gx_pmx_data *pmx_data;
	void __iomem *addr;
	int i, j, pos = 0;
	unsigned int pin;
	u32 reg;

	pin = selector + priv->data->pin_base;

	for (i = 0; i < priv->data->num_groups; i++) {
		group = &priv->data->groups[i];
		pmx_data = (struct meson_gx_pmx_data *)group->data;
		if (pmx_data->is_gpio)
			continue;

		for (j = 0; j < group->num_pins; j++) {
			if (group->pins[j] == pin) {
				/* We have found a group using the pin */
				addr = priv->reg_mux + pmx_data->reg * 4;
				reg = readl(addr) & BIT(pmx_data->bit);
				if (reg) {
					pos += snprintf(buf + pos, size - pos,
							"%s ", group->name) - 1;
					return 0;
				}
			}
		}
	}

	/* Fallback, must be used as GPIO */
	snprintf(buf, size, "%s or Unknown",
		 priv->data->groups[selector].name);

	return 0;
}

const struct pinconf_param meson_gx_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
};

const struct pinctrl_ops meson_gx_pinctrl_ops = {
	.get_groups_count = meson_pinctrl_get_groups_count,
	.get_group_name = meson_pinctrl_get_group_name,
	.get_functions_count = meson_pinmux_get_functions_count,
	.get_function_name = meson_pinmux_get_function_name,
	.pinmux_group_set = meson_gx_pinmux_group_set,
	.set_state = pinctrl_generic_set_state,
	.pinconf_params = meson_gx_pinconf_params,
	.pinconf_num_params = ARRAY_SIZE(meson_gx_pinconf_params),
	.pinconf_set = meson_pinconf_set,
	.pinconf_group_set = meson_pinconf_group_set,
	.get_pin_name = meson_pinctrl_get_pin_name,
	.get_pins_count = meson_pinctrl_get_pins_count,
	.get_pin_muxing	= meson_gx_pinmux_get,
};

static const struct dm_gpio_ops meson_gx_gpio_ops = {
	.set_value = meson_gpio_set,
	.get_value = meson_gpio_get,
	.get_function = meson_gpio_get_direction,
	.direction_input = meson_gpio_direction_input,
	.direction_output = meson_gpio_direction_output,
};

U_BOOT_DRIVER(meson_gx_gpio_driver) = {
	.name	= "meson-gx-gpio",
	.id	= UCLASS_GPIO,
	.probe	= meson_gpio_probe,
	.ops	= &meson_gx_gpio_ops,
};
