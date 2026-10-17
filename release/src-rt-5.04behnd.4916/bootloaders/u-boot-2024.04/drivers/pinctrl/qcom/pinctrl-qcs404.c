// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm QCS404 pinctrl
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");
static const char * const msm_pinctrl_pins[] = {
	"SDC1_RCLK",
	"SDC1_CLK",
	"SDC1_CMD",
	"SDC1_DATA",
	"SDC2_CLK",
	"SDC2_CMD",
	"SDC2_DATA",
};

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"blsp_uart2", 1},
	{"rgmii_int", 1},
	{"rgmii_ck", 1},
	{"rgmii_tx", 1},
	{"rgmii_ctl", 1},
	{"rgmii_rx", 1},
	{"rgmii_mdio", 1},
	{"rgmii_mdc", 1},
	{"blsp_i2c0", 3},
	{"blsp_i2c1", 2},
	{"blsp_i2c_sda_a2", 3},
	{"blsp_i2c_scl_a2", 3},
	{"blsp_i2c3", 2},
	{"blsp_i2c4", 1},
};

static const char *qcs404_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *qcs404_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector < 120) {
		snprintf(pin_name, MAX_PIN_NAME_LEN, "GPIO_%u", selector);
		return pin_name;
	} else {
		return msm_pinctrl_pins[selector - 120];
	}
}

static unsigned int qcs404_get_function_mux(unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data qcs404_data = {
	.pin_data = { .pin_count = 126, },
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = qcs404_get_function_name,
	.get_function_mux = qcs404_get_function_mux,
	.get_pin_name = qcs404_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,qcs404-pinctrl", .data = (ulong)&qcs404_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_qcs404) = {
	.name		= "pinctrl_qcs404",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
