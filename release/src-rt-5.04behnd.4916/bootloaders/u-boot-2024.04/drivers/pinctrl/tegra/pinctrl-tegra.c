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

static void tegra_pinctrl_set_drive(struct udevice *config, int drvcnt)
{
	struct pmux_drvgrp_config *drive_group;
	int i, ret, pad_id;
	const char **pads;

	drive_group = kmalloc_array(drvcnt, sizeof(*drive_group), GFP_KERNEL);
	if (!drive_group) {
		log_debug("%s: cannot allocate drive group array\n", __func__);
		return;
	}

	drive_group[0].slwf = dev_read_u32_default(config, "nvidia,slew-rate-falling", 0);
	drive_group[0].slwr = dev_read_u32_default(config, "nvidia,slew-rate-rising", 0);
	drive_group[0].drvup = dev_read_u32_default(config, "nvidia,pull-up-strength", 0);
	drive_group[0].drvdn = dev_read_u32_default(config, "nvidia,pull-down-strength", 0);
#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
	drive_group[0].lpmd = dev_read_u32_default(config, "nvidia,low-power-mode", 0);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
	drive_group[0].schmt = dev_read_u32_default(config, "nvidia,schmitt", 0);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_HSM
	drive_group[0].hsm = dev_read_u32_default(config, "nvidia,high-speed-mode", 0);
#endif

	for (i = 1; i < drvcnt; i++)
		memcpy(&drive_group[i], &drive_group[0], sizeof(drive_group[0]));

	ret = dev_read_string_list(config, "nvidia,pins", &pads);
	if (ret < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		goto exit;
	}

	for (i = 0; i < drvcnt; i++) {
		for (pad_id = 0; pad_id < PMUX_DRVGRP_COUNT; pad_id++)
			if (tegra_pinctrl_to_drvgrp[pad_id])
				if (!strcmp(pads[i], tegra_pinctrl_to_drvgrp[pad_id])) {
					drive_group[i].drvgrp = pad_id;
					break;
				}

		debug("%s drvmap: %d, %d, %d, %d, %d\n", pads[i],
		      drive_group[i].drvgrp, drive_group[i].slwf,
		      drive_group[i].slwr, drive_group[i].drvup,
		      drive_group[i].drvdn);
	}

	pinmux_config_drvgrp_table(drive_group, drvcnt);

	free(pads);
exit:
	kfree(drive_group);
}

static void tegra_pinctrl_set_pin(struct udevice *config, int pincnt)
{
	struct pmux_pingrp_config *pinmux_group;
	int i, ret, pin_id;
	const char *function;
	const char **pins;

	pinmux_group = kmalloc_array(pincnt, sizeof(*pinmux_group), GFP_KERNEL);
	if (!pinmux_group) {
		log_debug("%s: cannot allocate pinmux group array\n", __func__);
		return;
	}

	/* decode function id and fill the first copy of pmux_pingrp_config */
	function = dev_read_string(config, "nvidia,function");
	if (function)
		for (i = 0; i < PMUX_FUNC_COUNT; i++)
			if (tegra_pinctrl_to_func[i])
				if (!strcmp(function, tegra_pinctrl_to_func[i]))
					break;

	pinmux_group[0].func = i;

	pinmux_group[0].pull = dev_read_u32_default(config, "nvidia,pull", 0);
	pinmux_group[0].tristate = dev_read_u32_default(config, "nvidia,tristate", 0);
#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
	pinmux_group[0].io = dev_read_u32_default(config, "nvidia,enable-input", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_LOCK
	pinmux_group[0].lock = dev_read_u32_default(config, "nvidia,lock", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_OD
	pinmux_group[0].od = dev_read_u32_default(config, "nvidia,open-drain", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
	pinmux_group[0].ioreset = dev_read_u32_default(config, "nvidia,io-reset", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
	pinmux_group[0].rcv_sel = dev_read_u32_default(config, "nvidia,rcv-sel", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
	pinmux_group[0].e_io_hv = dev_read_u32_default(config, "nvidia,io-hv", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
	pinmux_group[0].schmt = dev_read_u32_default(config, "nvidia,schmitt", 0);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_HSM
	pinmux_group[0].hsm = dev_read_u32_default(config, "nvidia,high-speed-mode", 0);
#endif

	for (i = 1; i < pincnt; i++)
		memcpy(&pinmux_group[i], &pinmux_group[0], sizeof(pinmux_group[0]));

	ret = dev_read_string_list(config, "nvidia,pins", &pins);
	if (ret < 0) {
		log_debug("%s: could not parse property nvidia,pins\n", __func__);
		goto exit;
	}

	for (i = 0; i < pincnt; i++) {
		for (pin_id = 0; pin_id < PMUX_PINGRP_COUNT; pin_id++)
			if (tegra_pinctrl_to_pingrp[pin_id])
				if (!strcmp(pins[i], tegra_pinctrl_to_pingrp[pin_id])) {
					pinmux_group[i].pingrp = pin_id;
					break;
				}

		debug("%s pinmap: %d, %d, %d, %d\n", pins[i],
		      pinmux_group[i].pingrp, pinmux_group[i].func,
		      pinmux_group[i].pull, pinmux_group[i].tristate);
	}

	pinmux_config_pingrp_table(pinmux_group, pincnt);

	free(pins);
exit:
	kfree(pinmux_group);
}

static int tegra_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child;
	int ret;
	const char *name;

	device_foreach_child(child, config) {
		/* Pinmux node can contain pins and drives */
		ret = dev_read_string_index(child, "nvidia,pins", 0,
					    &name);
		if (ret < 0) {
			log_debug("%s: could not parse property nvidia,pins\n", __func__);
			return ret;
		}

		ret = dev_read_string_count(child, "nvidia,pins");
		if (ret < 0) {
			log_debug("%s: could not count nvidia,pins\n", __func__);
			return ret;
		}

		if (!strncmp(name, "drive_", 6))
			/* Drive node is detected */
			tegra_pinctrl_set_drive(child, ret);
		else
			/* Pin node is detected */
			tegra_pinctrl_set_pin(child, ret);
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
	{ .compatible = "nvidia,tegra30-pinmux" },
	{ .compatible = "nvidia,tegra114-pinmux" },
	{ },
};

U_BOOT_DRIVER(tegra_pinctrl) = {
	.name		= "tegra_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= tegra_pinctrl_ids,
	.bind		= tegra_pinctrl_bind,
	.ops		= &tegra_pinctrl_ops,
};
