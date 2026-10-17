// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm IPQ40xx pinctrl
 *
 * Copyright (c) 2019 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");
static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"gpio", 0},
	{"blsp_uart0_0", 1}, /* Only for GPIO:16,17 */
	{"blsp_uart0_1", 2}, /* Only for GPIO:60,61 */
	{"blsp_uart1", 1},
	{"blsp_spi0_0", 1}, /* Only for GPIO:12,13,14,15 */
	{"blsp_spi0_1", 2}, /* Only for GPIO:54,55,56,57 */
	{"blsp_spi1", 2},
	{"mdio_0", 1}, /* Only for GPIO6 */
	{"mdio_1", 2}, /* Only for GPIO53 */
	{"mdc_0", 1}, /* Only for GPIO7 */
	{"mdc_1", 2}, /* Only for GPIO52 */
};
static const char *ipq4019_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *ipq4019_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "GPIO_%u", selector);
	return pin_name;
}

static unsigned int ipq4019_get_function_mux(unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static const struct msm_pinctrl_data ipq4019_data = {
	.pin_data = { .pin_count = 100, },
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = ipq4019_get_function_name,
	.get_function_mux = ipq4019_get_function_mux,
	.get_pin_name = ipq4019_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,ipq4019-pinctrl", .data = (ulong)&ipq4019_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_ipq4019) = {
	.name		= "pinctrl_ipq4019",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
