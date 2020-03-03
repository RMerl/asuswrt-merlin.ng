/*
 * IMX pinmux core definitions
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Copyright (C) 2012 Linaro Ltd.
 *
 * Author: Dong Aisheng <dong.aisheng@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __DRIVERS_PINCTRL_IMX1_H
#define __DRIVERS_PINCTRL_IMX1_H

struct platform_device;

/**
 * struct imx1_pin - describes an IMX1/21/27 pin.
 * @pin_id: ID of the described pin.
 * @mux_id: ID of the mux setup.
 * @config: Configuration of the pin (currently only pullup-enable).
 */
struct imx1_pin {
	unsigned int pin_id;
	unsigned int mux_id;
	unsigned long config;
};

/**
 * struct imx1_pin_group - describes an IMX pin group
 * @name: the name of this specific pin group
 * @pins: an array of imx1_pin structs used in this group
 * @npins: the number of pins in this group array, i.e. the number of
 *	elements in .pins so we can iterate over that array
 */
struct imx1_pin_group {
	const char *name;
	unsigned int *pin_ids;
	struct imx1_pin *pins;
	unsigned npins;
};

/**
 * struct imx1_pmx_func - describes IMX pinmux functions
 * @name: the name of this specific function
 * @groups: corresponding pin groups
 * @num_groups: the number of groups
 */
struct imx1_pmx_func {
	const char *name;
	const char **groups;
	unsigned num_groups;
};

struct imx1_pinctrl_soc_info {
	struct device *dev;
	const struct pinctrl_pin_desc *pins;
	unsigned int npins;
	struct imx1_pin_group *groups;
	unsigned int ngroups;
	struct imx1_pmx_func *functions;
	unsigned int nfunctions;
};

#define IMX_PINCTRL_PIN(pin) PINCTRL_PIN(pin, #pin)

int imx1_pinctrl_core_probe(struct platform_device *pdev,
			struct imx1_pinctrl_soc_info *info);
int imx1_pinctrl_core_remove(struct platform_device *pdev);
#endif /* __DRIVERS_PINCTRL_IMX1_H */
