// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx pinctrl driver for ZynqMP
 *
 * Author(s):   Ashok Reddy Soma <ashok.reddy.soma@xilinx.com>
 *              Michal Simek <michal.simek@amd.com>
 *
 * Copyright (C) 2021 Xilinx, Inc. All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <zynqmp_firmware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/compat.h>
#include <dt-bindings/pinctrl/pinctrl-zynqmp.h>

#define PINCTRL_GET_FUNC_GROUPS_RESP_LEN	12
#define PINCTRL_GET_PIN_GROUPS_RESP_LEN		12
#define NUM_GROUPS_PER_RESP			6
#define NA_GROUP				-1
#define RESERVED_GROUP				-2
#define MAX_GROUP_PIN				50
#define MAX_PIN_GROUPS				50
#define MAX_GROUP_NAME_LEN			32
#define MAX_FUNC_NAME_LEN			16

#define DRIVE_STRENGTH_2MA	2
#define DRIVE_STRENGTH_4MA	4
#define DRIVE_STRENGTH_8MA	8
#define DRIVE_STRENGTH_12MA	12

/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */
struct zynqmp_pinctrl_priv {
	u32 npins;
	u32 nfuncs;
	u32 ngroups;
	struct zynqmp_pmux_function *funcs;
	struct zynqmp_pctrl_group *groups;
};

/**
 * struct zynqmp_pinctrl_config - pinconfig parameters
 * @slew:		Slew rate slow or fast
 * @bias:		Bias enabled or disabled
 * @pull_ctrl:		Pull control pull up or pull down
 * @input_type:		CMOS or Schmitt
 * @drive_strength:	Drive strength 2mA/4mA/8mA/12mA
 * @volt_sts:		Voltage status 1.8V or 3.3V
 * @tri_state:		Tristate enabled or disabled
 *
 * This structure holds information about pin control config
 * option that can be set for each pin.
 */
struct zynqmp_pinctrl_config {
	u32 slew;
	u32 bias;
	u32 pull_ctrl;
	u32 input_type;
	u32 drive_strength;
	u32 volt_sts;
	u32 tri_state;
};

/**
 * enum zynqmp_pin_config_param - possible pin configuration parameters
 * @PIN_CFG_IOSTANDARD:	if the pin can select an IO standard,
 *				the argument to this parameter (on a
 *				custom format) tells the driver which
 *				alternative IO standard to use
 * @PIN_CONFIG_SCHMITTCMOS:	this parameter (on a custom format) allows
 *				to select schmitt or cmos input for MIO pins
 */
enum zynqmp_pin_config_param {
	PIN_CFG_IOSTANDARD = PIN_CONFIG_END + 1,
	PIN_CONFIG_SCHMITTCMOS,
};

/**
 * struct zynqmp_pmux_function - a pinmux function
 * @name:	Name of the pinmux function
 * @groups:	List of pingroups for this function
 * @ngroups:	Number of entries in @groups
 *
 * This structure holds information about pin control function
 * and function group names supporting that function.
 */
struct zynqmp_pmux_function {
	char name[MAX_FUNC_NAME_LEN];
	const char * const *groups;
	unsigned int ngroups;
};

/**
 * struct zynqmp_pctrl_group - Pin control group info
 * @name:	Group name
 * @pins:	Group pin numbers
 * @npins:	Number of pins in group
 */
struct zynqmp_pctrl_group {
	const char *name;
	unsigned int pins[MAX_GROUP_PIN];
	unsigned int npins;
};

static char pin_name[PINNAME_SIZE];

/**
 * zynqmp_pm_query_data() - Get query data from firmware
 * @qid:	Value of enum pm_query_id
 * @arg1:	Argument 1
 * @arg2:	Argument 2
 * @out:	Returned output value
 *
 * Return: Returns status, either success or error+reason
 */
static int zynqmp_pm_query_data(enum pm_query_id qid, u32 arg1, u32 arg2, u32 *out)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	ret = xilinx_pm_request(PM_QUERY_DATA, qid, arg1, arg2, 0, ret_payload);
	if (ret)
		return ret;

	*out = ret_payload[1];

	return ret;
}

static int zynqmp_pm_pinctrl_get_config(const u32 pin, const u32 param, u32 *value)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	/* Get config for the pin */
	ret = xilinx_pm_request(PM_PINCTRL_CONFIG_PARAM_GET, pin, param, 0, 0, ret_payload);
	if (ret) {
		printf("%s failed\n", __func__);
		return ret;
	}

	*value = ret_payload[1];

	return ret;
}

static int zynqmp_pm_pinctrl_set_config(const u32 pin, const u32 param, u32 value)
{
	int ret;

	if (param == PM_PINCTRL_CONFIG_TRI_STATE) {
		ret = zynqmp_pm_feature(PM_PINCTRL_CONFIG_PARAM_SET);
		if (ret < PM_PINCTRL_PARAM_SET_VERSION)
			return -EOPNOTSUPP;
	}

	/* Request the pin first */
	ret = xilinx_pm_request(PM_PINCTRL_REQUEST, pin, 0, 0, 0, NULL);
	if (ret) {
		printf("%s: pin request failed\n", __func__);
		return ret;
	}

	/* Set config for the pin */
	ret = xilinx_pm_request(PM_PINCTRL_CONFIG_PARAM_SET, pin, param, value, 0, NULL);
	if (ret) {
		printf("%s failed\n", __func__);
		return ret;
	}

	return ret;
}

static int zynqmp_pinctrl_get_function_groups(u32 fid, u32 index, u16 *groups)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	ret = xilinx_pm_request(PM_QUERY_DATA, PM_QID_PINCTRL_GET_FUNCTION_GROUPS,
				fid, index, 0, ret_payload);
	if (ret) {
		printf("%s failed\n", __func__);
		return ret;
	}

	memcpy(groups, &ret_payload[1], PINCTRL_GET_FUNC_GROUPS_RESP_LEN);

	return ret;
}

static int zynqmp_pinctrl_prepare_func_groups(u32 fid,
					      struct zynqmp_pmux_function *func,
					      struct zynqmp_pctrl_group *groups)
{
	const char **fgroups;
	char name[MAX_GROUP_NAME_LEN];
	u16 resp[NUM_GROUPS_PER_RESP] = {0};
	int ret, index, i;

	fgroups = kcalloc(func->ngroups, sizeof(*fgroups), GFP_KERNEL);
	if (!fgroups)
		return -ENOMEM;

	for (index = 0; index < func->ngroups; index += NUM_GROUPS_PER_RESP) {
		ret = zynqmp_pinctrl_get_function_groups(fid, index, resp);
		if (ret)
			return ret;

		for (i = 0; i < NUM_GROUPS_PER_RESP; i++) {
			if (resp[i] == (u16)NA_GROUP)
				goto done;
			if (resp[i] == (u16)RESERVED_GROUP)
				continue;

			snprintf(name, MAX_GROUP_NAME_LEN, "%s_%d_grp",
				 func->name, index + i);
			fgroups[index + i] = strdup(name);

			snprintf(name, MAX_GROUP_NAME_LEN, "%s_%d_grp",
				 func->name, index + i);
			groups[resp[i]].name = strdup(name);
		}
	}
done:
	func->groups = fgroups;

	return ret;
}

static int zynqmp_pinctrl_get_pin_groups(u32 pin, u32 index, u16 *groups)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	ret = xilinx_pm_request(PM_QUERY_DATA, PM_QID_PINCTRL_GET_PIN_GROUPS,
				pin, index, 0, ret_payload);
	if (ret) {
		printf("%s failed to get pin groups\n", __func__);
		return ret;
	}

	memcpy(groups, &ret_payload[1], PINCTRL_GET_PIN_GROUPS_RESP_LEN);

	return ret;
}

static void zynqmp_pinctrl_group_add_pin(struct zynqmp_pctrl_group *group,
					 unsigned int pin)
{
	group->pins[group->npins++] = pin;
}

static int zynqmp_pinctrl_create_pin_groups(struct zynqmp_pctrl_group *groups,
					    unsigned int pin)
{
	u16 resp[NUM_GROUPS_PER_RESP] = {0};
	int ret, i, index = 0;

	do {
		ret = zynqmp_pinctrl_get_pin_groups(pin, index, resp);
		if (ret)
			return ret;

		for (i = 0; i < NUM_GROUPS_PER_RESP; i++) {
			if (resp[i] == (u16)NA_GROUP)
				goto done;
			if (resp[i] == (u16)RESERVED_GROUP)
				continue;
			zynqmp_pinctrl_group_add_pin(&groups[resp[i]], pin);
		}
		index += NUM_GROUPS_PER_RESP;
	} while (index <= MAX_PIN_GROUPS);

done:
	return ret;
}

static int zynqmp_pinctrl_probe(struct udevice *dev)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);
	int ret, i;
	u32 pin;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	/* Get number of pins first */
	ret = zynqmp_pm_query_data(PM_QID_PINCTRL_GET_NUM_PINS, 0, 0, &priv->npins);
	if (ret) {
		printf("%s failed to get no of pins\n", __func__);
		return ret;
	}

	/* Get number of functions available */
	ret = zynqmp_pm_query_data(PM_QID_PINCTRL_GET_NUM_FUNCTIONS, 0, 0, &priv->nfuncs);
	if (ret) {
		printf("%s failed to get no of functions\n", __func__);
		return ret;
	}

	/* Allocating structures for functions and its groups */
	priv->funcs = kzalloc(sizeof(*priv->funcs) * priv->nfuncs, GFP_KERNEL);
	if (!priv->funcs)
		return -ENOMEM;

	for (i = 0; i < priv->nfuncs; i++) {
		/* Get function name for the function and fill */
		xilinx_pm_request(PM_QUERY_DATA, PM_QID_PINCTRL_GET_FUNCTION_NAME,
				  i, 0, 0, ret_payload);

		memcpy((void *)priv->funcs[i].name, ret_payload, MAX_FUNC_NAME_LEN);

		/* And fill number of groups available for certain function */
		xilinx_pm_request(PM_QUERY_DATA, PM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS,
				  i, 0, 0, ret_payload);

		priv->funcs[i].ngroups = ret_payload[1];
		priv->ngroups += priv->funcs[i].ngroups;
	}

	/* Prepare all groups */
	priv->groups = kzalloc(sizeof(*priv->groups) * priv->ngroups,
			       GFP_KERNEL);
	if (!priv->groups)
		return -ENOMEM;

	for (i = 0; i < priv->nfuncs; i++) {
		ret = zynqmp_pinctrl_prepare_func_groups(i, &priv->funcs[i],
							 priv->groups);
		if (ret) {
			printf("Failed to prepare_func_groups\n");
			return ret;
		}
	}

	for (pin = 0; pin < priv->npins; pin++) {
		ret = zynqmp_pinctrl_create_pin_groups(priv->groups, pin);
		if (ret)
			return ret;
	}

	return 0;
}

static int zynqmp_pinctrl_get_functions_count(struct udevice *dev)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->nfuncs;
}

static const char *zynqmp_pinctrl_get_function_name(struct udevice *dev,
						    unsigned int selector)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->funcs[selector].name;
}

static int zynqmp_pinmux_set(struct udevice *dev, unsigned int selector,
			     unsigned int func_selector)
{
	int ret;

	/* Request the pin first */
	ret = xilinx_pm_request(PM_PINCTRL_REQUEST, selector, 0, 0, 0, NULL);
	if (ret) {
		printf("%s: pin request failed\n", __func__);
		return ret;
	}

	/* Set the pin function */
	ret = xilinx_pm_request(PM_PINCTRL_SET_FUNCTION, selector, func_selector,
				0, 0, NULL);
	if (ret) {
		printf("%s: Failed to set pinmux function\n", __func__);
		return ret;
	}

	return 0;
}

static int zynqmp_pinmux_group_set(struct udevice *dev, unsigned int selector,
				   unsigned int func_selector)
{
	int i;
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);
	const struct zynqmp_pctrl_group *pgrp = &priv->groups[selector];

	for (i = 0; i < pgrp->npins; i++)
		zynqmp_pinmux_set(dev, pgrp->pins[i], func_selector);

	return 0;
}

static int zynqmp_pinconf_set(struct udevice *dev, unsigned int pin,
			      unsigned int param, unsigned int arg)
{
	int ret = 0;
	unsigned int value;

	switch (param) {
	case PIN_CONFIG_SLEW_RATE:
		param = PM_PINCTRL_CONFIG_SLEW_RATE;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		param = PM_PINCTRL_CONFIG_PULL_CTRL;
		arg = PM_PINCTRL_BIAS_PULL_UP;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		param = PM_PINCTRL_CONFIG_PULL_CTRL;
		arg = PM_PINCTRL_BIAS_PULL_DOWN;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		param = PM_PINCTRL_CONFIG_BIAS_STATUS;
		arg = PM_PINCTRL_BIAS_DISABLE;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_SCHMITTCMOS:
		param = PM_PINCTRL_CONFIG_SCHMITT_CMOS;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		param = PM_PINCTRL_CONFIG_SCHMITT_CMOS;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		switch (arg) {
		case DRIVE_STRENGTH_2MA:
			value = PM_PINCTRL_DRIVE_STRENGTH_2MA;
			break;
		case DRIVE_STRENGTH_4MA:
			value = PM_PINCTRL_DRIVE_STRENGTH_4MA;
			break;
		case DRIVE_STRENGTH_8MA:
			value = PM_PINCTRL_DRIVE_STRENGTH_8MA;
			break;
		case DRIVE_STRENGTH_12MA:
			value = PM_PINCTRL_DRIVE_STRENGTH_12MA;
			break;
		default:
			/* Invalid drive strength */
			dev_warn(dev, "Invalid drive strength for pin %d\n", pin);
			return -EINVAL;
		}

		param = PM_PINCTRL_CONFIG_DRIVE_STRENGTH;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, value);
		break;
	case PIN_CFG_IOSTANDARD:
		param = PM_PINCTRL_CONFIG_VOLTAGE_STATUS;
		ret = zynqmp_pm_pinctrl_get_config(pin, param, &value);
		if (arg != value)
			dev_warn(dev, "Invalid IO Standard requested for pin %d\n",
				 pin);
		break;
	case PIN_CONFIG_POWER_SOURCE:
		param = PM_PINCTRL_CONFIG_VOLTAGE_STATUS;
		ret = zynqmp_pm_pinctrl_get_config(pin, param, &value);
		if (arg != value)
			dev_warn(dev, "Invalid IO Standard requested for pin %d\n",
				 pin);
		break;
	case PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
		param = PM_PINCTRL_CONFIG_TRI_STATE;
		arg = PM_PINCTRL_TRI_STATE_ENABLE;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	case PIN_CONFIG_LOW_POWER_MODE:
		/*
		 * This cases are mentioned in dts but configurable
		 * registers are unknown. So falling through to ignore
		 * boot time warnings as of now.
		 */
		ret = 0;
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		param = PM_PINCTRL_CONFIG_TRI_STATE;
		arg = PM_PINCTRL_TRI_STATE_DISABLE;
		ret = zynqmp_pm_pinctrl_set_config(pin, param, arg);
		break;
	default:
		dev_warn(dev, "unsupported configuration parameter '%u'\n",
			 param);
		ret = -ENOTSUPP;
		break;
	}

	return ret;
}

static int zynqmp_pinconf_group_set(struct udevice *dev,
				    unsigned int group_selector,
				    unsigned int param, unsigned int arg)
{
	int i;
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);
	const struct zynqmp_pctrl_group *pgrp = &priv->groups[group_selector];

	for (i = 0; i < pgrp->npins; i++)
		zynqmp_pinconf_set(dev, pgrp->pins[i], param, arg);

	return 0;
}

static int zynqmp_pinctrl_get_pins_count(struct udevice *dev)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->npins;
}

static const char *zynqmp_pinctrl_get_pin_name(struct udevice *dev,
					       unsigned int selector)
{
	snprintf(pin_name, PINNAME_SIZE, "MIO%d", selector);

	return pin_name;
}

static int zynqmp_pinctrl_get_pin_muxing(struct udevice *dev,
					 unsigned int selector,
					 char *buf,
					 int size)
{
	struct zynqmp_pinctrl_config pinmux;

	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_SLEW_RATE,
				     &pinmux.slew);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_BIAS_STATUS,
				     &pinmux.bias);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_PULL_CTRL,
				     &pinmux.pull_ctrl);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_SCHMITT_CMOS,
				     &pinmux.input_type);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_DRIVE_STRENGTH,
				     &pinmux.drive_strength);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_VOLTAGE_STATUS,
				     &pinmux.volt_sts);
	zynqmp_pm_pinctrl_get_config(selector, PM_PINCTRL_CONFIG_TRI_STATE,
				     &pinmux.tri_state);

	switch (pinmux.drive_strength) {
	case PM_PINCTRL_DRIVE_STRENGTH_2MA:
		pinmux.drive_strength = DRIVE_STRENGTH_2MA;
		break;
	case PM_PINCTRL_DRIVE_STRENGTH_4MA:
		pinmux.drive_strength = DRIVE_STRENGTH_4MA;
		break;
	case PM_PINCTRL_DRIVE_STRENGTH_8MA:
		pinmux.drive_strength = DRIVE_STRENGTH_8MA;
		break;
	case PM_PINCTRL_DRIVE_STRENGTH_12MA:
		pinmux.drive_strength = DRIVE_STRENGTH_12MA;
		break;
	default:
		/* Invalid drive strength */
		dev_warn(dev, "Invalid drive strength\n");
		return -EINVAL;
	}

	snprintf(buf, size,
		 "slew:%s\tbias:%s\tpull:%s\tinput:%s\tdrive:%dmA\tvolt:%s\ttri_state:%s",
		 pinmux.slew ? "slow" : "fast",
		 pinmux.bias ? "enabled" : "disabled",
		 pinmux.pull_ctrl ? "up" : "down",
		 pinmux.input_type ? "schmitt" : "cmos",
		 pinmux.drive_strength,
		 pinmux.volt_sts ? "1.8" : "3.3",
		 pinmux.tri_state ? "enabled" : "disabled");

	return 0;
}

static int zynqmp_pinctrl_get_groups_count(struct udevice *dev)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->ngroups;
}

static const char *zynqmp_pinctrl_get_group_name(struct udevice *dev,
						 unsigned int selector)
{
	struct zynqmp_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->groups[selector].name;
}

static const struct pinconf_param zynqmp_conf_params[] = {
	{ "bias-bus-hold", PIN_CONFIG_BIAS_BUS_HOLD, 0 },
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-high-impedance", PIN_CONFIG_BIAS_HIGH_IMPEDANCE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-pin-default", PIN_CONFIG_BIAS_PULL_PIN_DEFAULT, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "drive-open-source", PIN_CONFIG_DRIVE_OPEN_SOURCE, 0 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "drive-strength-microamp", PIN_CONFIG_DRIVE_STRENGTH_UA, 0 },
	{ "input-debounce", PIN_CONFIG_INPUT_DEBOUNCE, 0 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-schmitt", PIN_CONFIG_INPUT_SCHMITT, 0 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "low-power-disable", PIN_CONFIG_LOW_POWER_MODE, 0 },
	{ "low-power-enable", PIN_CONFIG_LOW_POWER_MODE, 1 },
	{ "output-disable", PIN_CONFIG_OUTPUT_ENABLE, 0 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1, },
	{ "output-low", PIN_CONFIG_OUTPUT, 0, },
	{ "power-source", PIN_CONFIG_POWER_SOURCE, 0 },
	{ "sleep-hardware-state", PIN_CONFIG_SLEEP_HARDWARE_STATE, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0 },
	{ "skew-delay", PIN_CONFIG_SKEW_DELAY, 0 },
	/* zynqmp specific */
	{"io-standard", PIN_CFG_IOSTANDARD, IO_STANDARD_LVCMOS18},
	{"schmitt-cmos", PIN_CONFIG_SCHMITTCMOS, PM_PINCTRL_INPUT_TYPE_SCHMITT},
};

static struct pinctrl_ops zynqmp_pinctrl_ops = {
	.get_pins_count = zynqmp_pinctrl_get_pins_count,
	.get_pin_name = zynqmp_pinctrl_get_pin_name,
	.get_pin_muxing = zynqmp_pinctrl_get_pin_muxing,
	.set_state = pinctrl_generic_set_state,
	.get_groups_count = zynqmp_pinctrl_get_groups_count,
	.get_group_name = zynqmp_pinctrl_get_group_name,
	.get_functions_count = zynqmp_pinctrl_get_functions_count,
	.get_function_name = zynqmp_pinctrl_get_function_name,
	.pinmux_group_set = zynqmp_pinmux_group_set,
	.pinmux_set = zynqmp_pinmux_set,
	.pinconf_params = zynqmp_conf_params,
	.pinconf_group_set = zynqmp_pinconf_group_set,
	.pinconf_set = zynqmp_pinconf_set,
	.pinconf_num_params = ARRAY_SIZE(zynqmp_conf_params),
};

static const struct udevice_id zynqmp_pinctrl_ids[] = {
	{ .compatible = "xlnx,zynqmp-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_zynqmp) = {
	.name = "zynqmp-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = zynqmp_pinctrl_ids,
	.priv_auto = sizeof(struct zynqmp_pinctrl_priv),
	.ops = &zynqmp_pinctrl_ops,
	.probe = zynqmp_pinctrl_probe,
};
