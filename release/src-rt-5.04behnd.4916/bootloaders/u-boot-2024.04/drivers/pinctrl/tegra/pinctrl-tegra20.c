// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2023
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <stdlib.h>

#include <asm/arch/pinmux.h>

static void tegra_pinctrl_set_pin(struct udevice *config)
{
	int i, count, pin_id, ret;
	int pull, tristate;
	const char **pins;

	ret = dev_read_u32(config, "nvidia,pull", &pull);
	if (ret)
		pull = ret;

	ret = dev_read_u32(config, "nvidia,tristate", &tristate);
	if (ret)
		tristate = ret;

	count = dev_read_string_list(config, "nvidia,pins", &pins);
	if (count < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		for (pin_id = 0; pin_id < PMUX_PINGRP_COUNT; pin_id++)
			if (tegra_pinctrl_to_pingrp[pin_id])
				if (!strcmp(pins[i], tegra_pinctrl_to_pingrp[pin_id]))
					break;

		if (pull >= 0)
			pinmux_set_pullupdown(pin_id, pull);

		if (tristate >= 0) {
			if (!tristate)
				pinmux_tristate_disable(pin_id);
			else
				pinmux_tristate_enable(pin_id);
		}
	}

	free(pins);
}

static void tegra_pinctrl_set_func(struct udevice *config)
{
	int i, count, func_id, pin_id;
	const char *function;
	const char **pins;

	function = dev_read_string(config, "nvidia,function");
	if (function)
		for (i = 0; i < PMUX_FUNC_COUNT; i++)
			if (tegra_pinctrl_to_func[i])
				if (!strcmp(function, tegra_pinctrl_to_func[i]))
					break;

	func_id = i;

	count = dev_read_string_list(config, "nvidia,pins", &pins);
	if (count < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		for (pin_id = 0; pin_id < PMUX_PINGRP_COUNT; pin_id++)
			if (tegra_pinctrl_to_pingrp[pin_id])
				if (!strcmp(pins[i], tegra_pinctrl_to_pingrp[pin_id]))
					break;

		debug("%s(%d) muxed to %s(%d)\n", pins[i], pin_id, function, func_id);

		pinmux_set_func(pin_id, func_id);
	}

	free(pins);
}

static int tegra_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child;

	device_foreach_child(child, config) {
		/*
		 * Tegra20 pinmux is set differently then any other
		 * Tegra SOC. Nodes are arranged by function muxing,
		 * then actual pins setup (with node name prefix
		 * conf_*) and then drive setup.
		 */
		if (!strncmp(child->name, "conf_", 5))
			tegra_pinctrl_set_pin(child);
		else if (!strncmp(child->name, "drive_", 6))
			debug("%s: drive configuration is not supported\n", __func__);
		else
			tegra_pinctrl_set_func(child);
	}

	return 0;
}

static int tegra_pinctrl_get_pins_count(struct udevice *dev)
{
	return PMUX_PINGRP_COUNT;
}

static const char *tegra_pinctrl_get_pin_name(struct udevice *dev,
					      unsigned int selector)
{
	return tegra_pinctrl_to_pingrp[selector];
}

static int tegra_pinctrl_get_groups_count(struct udevice *dev)
{
	return PMUX_DRVGRP_COUNT;
}

static const char *tegra_pinctrl_get_group_name(struct udevice *dev,
						unsigned int selector)
{
	return tegra_pinctrl_to_drvgrp[selector];
}

static int tegra_pinctrl_get_functions_count(struct udevice *dev)
{
	return PMUX_FUNC_COUNT;
}

static const char *tegra_pinctrl_get_function_name(struct udevice *dev,
						   unsigned int selector)
{
	return tegra_pinctrl_to_func[selector];
}

const struct pinctrl_ops tegra_pinctrl_ops = {
	.get_pins_count = tegra_pinctrl_get_pins_count,
	.get_pin_name = tegra_pinctrl_get_pin_name,
	.get_groups_count = tegra_pinctrl_get_groups_count,
	.get_group_name = tegra_pinctrl_get_group_name,
	.get_functions_count = tegra_pinctrl_get_functions_count,
	.get_function_name = tegra_pinctrl_get_function_name,
	.set_state = tegra_pinctrl_set_state,
};

static int tegra_pinctrl_bind(struct udevice *dev)
{
	/*
	 * Make sure that the pinctrl driver gets probed after binding
	 * to provide initial configuration and assure that further
	 * probed devices are working correctly.
	 */
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static const struct udevice_id tegra_pinctrl_ids[] = {
	{ .compatible = "nvidia,tegra20-pinmux" },
	{ },
};

U_BOOT_DRIVER(tegra_pinctrl) = {
	.name		= "tegra_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= tegra_pinctrl_ids,
	.bind		= tegra_pinctrl_bind,
	.ops		= &tegra_pinctrl_ops,
};
