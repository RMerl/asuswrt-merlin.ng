// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/compat.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * pinctrl_pin_name_to_selector() - return the pin selector for a pin
 *
 * @dev: pin controller device
 * @pin: the pin name to look up
 * @return: pin selector, or negative error code on failure
 */
static int pinctrl_pin_name_to_selector(struct udevice *dev, const char *pin)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
	unsigned npins, selector;

	if (!ops->get_pins_count || !ops->get_pin_name) {
		dev_dbg(dev, "get_pins_count or get_pin_name missing\n");
		return -ENOSYS;
	}

	npins = ops->get_pins_count(dev);

	/* See if this pctldev has this pin */
	for (selector = 0; selector < npins; selector++) {
		const char *pname = ops->get_pin_name(dev, selector);

		if (!strcmp(pin, pname))
			return selector;
	}

	return -ENOSYS;
}

/**
 * pinctrl_group_name_to_selector() - return the group selector for a group
 *
 * @dev: pin controller device
 * @group: the pin group name to look up
 * @return: pin group selector, or negative error code on failure
 */
static int pinctrl_group_name_to_selector(struct udevice *dev,
					  const char *group)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
	unsigned ngroups, selector;

	if (!ops->get_groups_count || !ops->get_group_name) {
		dev_dbg(dev, "get_groups_count or get_group_name missing\n");
		return -ENOSYS;
	}

	ngroups = ops->get_groups_count(dev);

	/* See if this pctldev has this group */
	for (selector = 0; selector < ngroups; selector++) {
		const char *gname = ops->get_group_name(dev, selector);

		if (!strcmp(group, gname))
			return selector;
	}

	return -ENOSYS;
}

#if CONFIG_IS_ENABLED(PINMUX)
/**
 * pinmux_func_name_to_selector() - return the function selector for a function
 *
 * @dev: pin controller device
 * @function: the function name to look up
 * @return: function selector, or negative error code on failure
 */
static int pinmux_func_name_to_selector(struct udevice *dev,
					const char *function)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
	unsigned nfuncs, selector = 0;

	if (!ops->get_functions_count || !ops->get_function_name) {
		dev_dbg(dev,
			"get_functions_count or get_function_name missing\n");
		return -ENOSYS;
	}

	nfuncs = ops->get_functions_count(dev);

	/* See if this pctldev has this function */
	for (selector = 0; selector < nfuncs; selector++) {
		const char *fname = ops->get_function_name(dev, selector);

		if (!strcmp(function, fname))
			return selector;
	}

	return -ENOSYS;
}

/**
 * pinmux_enable_setting() - enable pin-mux setting for a certain pin/group
 *
 * @dev: pin controller device
 * @is_group: target of operation (true: pin group, false: pin)
 * @selector: pin selector or group selector, depending on @is_group
 * @func_selector: function selector
 * @return: 0 on success, or negative error code on failure
 */
static int pinmux_enable_setting(struct udevice *dev, bool is_group,
				 unsigned selector, unsigned func_selector)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (is_group) {
		if (!ops->pinmux_group_set) {
			dev_dbg(dev, "pinmux_group_set op missing\n");
			return -ENOSYS;
		}

		return ops->pinmux_group_set(dev, selector, func_selector);
	} else {
		if (!ops->pinmux_set) {
			dev_dbg(dev, "pinmux_set op missing\n");
			return -ENOSYS;
		}
		return ops->pinmux_set(dev, selector, func_selector);
	}
}
#else
static int pinmux_func_name_to_selector(struct udevice *dev,
					const char *function)
{
	return 0;
}

static int pinmux_enable_setting(struct udevice *dev, bool is_group,
				 unsigned selector, unsigned func_selector)
{
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(PINCONF)
/**
 * pinconf_prop_name_to_param() - return parameter ID for a property name
 *
 * @dev: pin controller device
 * @property: property name in DTS, such as "bias-pull-up", "slew-rate", etc.
 * @default_value: return default value in case no value is specified in DTS
 * @return: return pamater ID, or negative error code on failure
 */
static int pinconf_prop_name_to_param(struct udevice *dev,
				      const char *property, u32 *default_value)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
	const struct pinconf_param *p, *end;

	if (!ops->pinconf_num_params || !ops->pinconf_params) {
		dev_dbg(dev, "pinconf_num_params or pinconf_params missing\n");
		return -ENOSYS;
	}

	p = ops->pinconf_params;
	end = p + ops->pinconf_num_params;

	/* See if this pctldev supports this parameter */
	for (; p < end; p++) {
		if (!strcmp(property, p->property)) {
			*default_value = p->default_value;
			return p->param;
		}
	}

	return -ENOSYS;
}

/**
 * pinconf_enable_setting() - apply pin configuration for a certain pin/group
 *
 * @dev: pin controller device
 * @is_group: target of operation (true: pin group, false: pin)
 * @selector: pin selector or group selector, depending on @is_group
 * @param: configuration paramter
 * @argument: argument taken by some configuration parameters
 * @return: 0 on success, or negative error code on failure
 */
static int pinconf_enable_setting(struct udevice *dev, bool is_group,
				  unsigned selector, unsigned param,
				  u32 argument)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (is_group) {
		if (!ops->pinconf_group_set) {
			dev_dbg(dev, "pinconf_group_set op missing\n");
			return -ENOSYS;
		}

		return ops->pinconf_group_set(dev, selector, param,
					      argument);
	} else {
		if (!ops->pinconf_set) {
			dev_dbg(dev, "pinconf_set op missing\n");
			return -ENOSYS;
		}
		return ops->pinconf_set(dev, selector, param, argument);
	}
}
#else
static int pinconf_prop_name_to_param(struct udevice *dev,
				      const char *property, u32 *default_value)
{
	return -ENOSYS;
}

static int pinconf_enable_setting(struct udevice *dev, bool is_group,
				  unsigned selector, unsigned param,
				  u32 argument)
{
	return 0;
}
#endif

/**
 * pinctrl_generic_set_state_one() - set state for a certain pin/group
 * Apply all pin multiplexing and pin configurations specified by @config
 * for a given pin or pin group.
 *
 * @dev: pin controller device
 * @config: pseudo device pointing to config node
 * @is_group: target of operation (true: pin group, false: pin)
 * @selector: pin selector or group selector, depending on @is_group
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_generic_set_state_one(struct udevice *dev,
					 struct udevice *config,
					 bool is_group, unsigned selector)
{
	const void *fdt = gd->fdt_blob;
	int node_offset = dev_of_offset(config);
	const char *propname;
	const void *value;
	int prop_offset, len, func_selector, param, ret;
	u32 arg, default_val;

	for (prop_offset = fdt_first_property_offset(fdt, node_offset);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(fdt, prop_offset)) {
		value = fdt_getprop_by_offset(fdt, prop_offset,
					      &propname, &len);
		if (!value)
			return -EINVAL;

		if (!strcmp(propname, "function")) {
			func_selector = pinmux_func_name_to_selector(dev,
								     value);
			if (func_selector < 0)
				return func_selector;
			ret = pinmux_enable_setting(dev, is_group,
						    selector,
						    func_selector);
		} else {
			param = pinconf_prop_name_to_param(dev, propname,
							   &default_val);
			if (param < 0)
				continue; /* just skip unknown properties */

			if (len >= sizeof(fdt32_t))
				arg = fdt32_to_cpu(*(fdt32_t *)value);
			else
				arg = default_val;

			ret = pinconf_enable_setting(dev, is_group,
						     selector, param, arg);
		}

		if (ret)
			return ret;
	}

	return 0;
}

/**
 * pinctrl_generic_set_state_subnode() - apply all settings in config node
 *
 * @dev: pin controller device
 * @config: pseudo device pointing to config node
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_generic_set_state_subnode(struct udevice *dev,
					     struct udevice *config)
{
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(config);
	const char *subnode_target_type = "pins";
	bool is_group = false;
	const char *name;
	int strings_count, selector, i, ret;

	strings_count = fdt_stringlist_count(fdt, node, subnode_target_type);
	if (strings_count < 0) {
		subnode_target_type = "groups";
		is_group = true;
		strings_count = fdt_stringlist_count(fdt, node,
						     subnode_target_type);
		if (strings_count < 0) {
			/* skip this node; may contain config child nodes */
			return 0;
		}
	}

	for (i = 0; i < strings_count; i++) {
		name = fdt_stringlist_get(fdt, node, subnode_target_type, i,
					  NULL);
		if (!name)
			return -EINVAL;

		if (is_group)
			selector = pinctrl_group_name_to_selector(dev, name);
		else
			selector = pinctrl_pin_name_to_selector(dev, name);
		if (selector < 0)
			return selector;

		ret = pinctrl_generic_set_state_one(dev, config,
						    is_group, selector);
		if (ret)
			return ret;
	}

	return 0;
}

int pinctrl_generic_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child;
	int ret;

	ret = pinctrl_generic_set_state_subnode(dev, config);
	if (ret)
		return ret;

	for (device_find_first_child(config, &child);
	     child;
	     device_find_next_child(&child)) {
		ret = pinctrl_generic_set_state_subnode(dev, child);
		if (ret)
			return ret;
	}

	return 0;
}
