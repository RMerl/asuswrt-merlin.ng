// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm APQ8016 pinctrl
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include "pinctrl-snapdragon.h"
#include <common.h>

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN];
static const char * const msm_pinctrl_pins[] = {
	"SDC1_CLK",
	"SDC1_CMD",
	"SDC1_DATA",
	"SDC2_CLK",
	"SDC2_CMD",
	"SDC2_DATA",
	"QDSD_CLK",
	"QDSD_CMD",
	"QDSD_DATA0",
	"QDSD_DATA1",
	"QDSD_DATA2",
	"QDSD_DATA3",
};

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"blsp1_uart", 2},
};

static const char *apq8016_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *apq8016_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	if (selector < 122) {
		snprintf(pin_name, MAX_PIN_NAME_LEN, "GPIO_%u", selector);
		return pin_name;
	} else {
		return msm_pinctrl_pins[selector - 122];
	}
}

static unsigned int apq8016_get_function_mux(unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

struct msm_pinctrl_data apq8016_data = {
	.pin_count = 133,
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = apq8016_get_function_name,
	.get_function_mux = apq8016_get_function_mux,
	.get_pin_name = apq8016_get_pin_name,
};

