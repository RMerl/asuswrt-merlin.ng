// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/compat.h>
#include <dm/pinctrl.h>

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

enum pinmux_subnode_type {
	PST_NONE = 0,
	PST_PIN,
	PST_GROUP,
	PST_PINMUX,
};

static const char *alloc_name_with_prefix(const char *name, const char *prefix)
{
	if (prefix) {
		char *name_with_prefix = malloc(strlen(prefix) + strlen(name) + 1);
		if (name_with_prefix)
			sprintf(name_with_prefix, "%s%s", prefix, name);
		return name_with_prefix;
	} else {
		return name;
	}
}

static void free_name_with_prefix(const char *name_with_prefix, const char *prefix)
{
	if (prefix)
		free((char *)name_with_prefix);
}

/**
 * pinctrl_generic_set_state_one() - set state for a certain pin/group
 * Apply all pin multiplexing and pin configurations specified by @config
 * for a given pin or pin group.
 *
 * @dev: pin controller device
 * @config: pseudo device pointing to config node
 * @subnode_type: target of operation (pin, group, or pin specified by a pinmux
 * group)
 * @selector: pin selector or group selector, depending on @subnode_type
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_generic_set_state_one(struct udevice *dev,
					 struct udevice *config,
					 const char *prefix,
					 enum pinmux_subnode_type subnode_type,
					 unsigned selector)
{
	const char *function_propname;
	const char *propname;
	const void *value;
	struct ofprop property;
	int len, func_selector, param, ret;
	u32 arg, default_val;

	assert(subnode_type != PST_NONE);

	function_propname = alloc_name_with_prefix("function", prefix);
	if (!function_propname)
		return -ENOMEM;

	dev_for_each_property(property, config) {
		value = dev_read_prop_by_prop(&property, &propname, &len);
		if (!value) {
			free_name_with_prefix(function_propname, prefix);
			return -EINVAL;
		}

		/* pinmux subnodes already have their muxing set */
		if (subnode_type != PST_PINMUX &&
		    !strcmp(propname, function_propname)) {
			func_selector = pinmux_func_name_to_selector(dev,
								     value);
			if (func_selector < 0) {
				free_name_with_prefix(function_propname, prefix);
				return func_selector;
			}
			ret = pinmux_enable_setting(dev,
						    subnode_type == PST_GROUP,
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

			ret = pinconf_enable_setting(dev,
						     subnode_type == PST_GROUP,
						     selector, param, arg);
		}

		if (ret) {
			free_name_with_prefix(function_propname, prefix);
			return ret;
		}
	}

	free_name_with_prefix(function_propname, prefix);
	return 0;
}

/**
 * pinctrl_generic_get_subnode_type() - determine whether there is a valid
 * pins, groups, or pinmux property in the config node
 *
 * @dev: pin controller device
 * @config: pseudo device pointing to config node
 * @count: number of specifiers contained within the property
 * @return: the type of the subnode, or PST_NONE
 */
static enum pinmux_subnode_type pinctrl_generic_get_subnode_type(struct udevice *dev,
								 struct udevice *config,
								 const char *prefix,
								 int *count)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
	const char *propname;

	propname = alloc_name_with_prefix("pins", prefix);
	if (!propname)
		return -ENOMEM;
	*count = dev_read_string_count(config, propname);
	free_name_with_prefix(propname, prefix);
	if (*count >= 0)
		return PST_PIN;

	propname = alloc_name_with_prefix("groups", prefix);
	if (!propname)
		return -ENOMEM;
	*count = dev_read_string_count(config, propname);
	free_name_with_prefix(propname, prefix);
	if (*count >= 0)
		return PST_GROUP;

	if (ops->pinmux_property_set) {
		propname = alloc_name_with_prefix("pinmux", prefix);
		if (!propname)
			return -ENOMEM;
		*count = dev_read_size(config, propname);
		free_name_with_prefix(propname, prefix);
		if (*count >= 0 && !(*count % sizeof(u32))) {
			*count /= sizeof(u32);
			return PST_PINMUX;
		}
	}

	*count = 0;
	return PST_NONE;
}

/**
 * pinctrl_generic_set_state_subnode() - apply all settings in config node
 *
 * @dev: pin controller device
 * @config: pseudo device pointing to config node
 * @prefix: device tree property prefix (e.g. vendor specific)
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_generic_set_state_subnode(struct udevice *dev,
					     struct udevice *config,
					     const char *prefix)
{
	enum pinmux_subnode_type subnode_type;
	const char *propname;
	const char *name;
	int count, selector, i, ret, scratch;
	const u32 *pinmux_groups = NULL; /* prevent use-uninitialized warning */

	subnode_type = pinctrl_generic_get_subnode_type(dev, config, prefix, &count);

	debug("%s(%s, %s): count=%d\n", __func__, dev->name, config->name,
	      count);

	if (subnode_type == PST_PINMUX) {
		propname = alloc_name_with_prefix("pinmux", prefix);
		if (!propname)
			return -ENOMEM;
		pinmux_groups = dev_read_prop(config, propname, &scratch);
		free_name_with_prefix(propname, prefix);
		if (!pinmux_groups)
			return -EINVAL;
	}

	for (i = 0; i < count; i++) {
		switch (subnode_type) {
		case PST_PIN:
			propname = alloc_name_with_prefix("pins", prefix);
			if (!propname)
				return -ENOMEM;
			ret = dev_read_string_index(config, propname, i, &name);
			free_name_with_prefix(propname, prefix);
			if (ret)
				return ret;
			selector = pinctrl_pin_name_to_selector(dev, name);
			break;
		case PST_GROUP:
			propname = alloc_name_with_prefix("groups", prefix);
			if (!propname)
				return -ENOMEM;
			ret = dev_read_string_index(config, propname, i, &name);
			free_name_with_prefix(propname, prefix);
			if (ret)
				return ret;
			selector = pinctrl_group_name_to_selector(dev, name);
			break;
		case PST_PINMUX: {
			const struct pinctrl_ops *ops = pinctrl_get_ops(dev);
			u32 pinmux_group = fdt32_to_cpu(pinmux_groups[i]);

			/* Checked for in pinctrl_generic_get_subnode_type */
			selector = ops->pinmux_property_set(dev, pinmux_group);
			break;
		}
		case PST_NONE:
		default:
			/* skip this node; may contain config child nodes */
			return 0;
		}

		if (selector < 0)
			return selector;

		ret = pinctrl_generic_set_state_one(dev, config, prefix,
						    subnode_type, selector);
		if (ret)
			return ret;
	}

	return 0;
}

int pinctrl_generic_set_state_prefix(struct udevice *dev, struct udevice *config,
				     const char *prefix)
{
	struct udevice *child;
	int ret;

	ret = pinctrl_generic_set_state_subnode(dev, config, prefix);
	if (ret)
		return ret;

	for (device_find_first_child(config, &child);
	     child;
	     device_find_next_child(&child)) {
		ret = pinctrl_generic_set_state_subnode(dev, child, prefix);
		if (ret)
			return ret;
	}

	return 0;
}

int pinctrl_generic_set_state(struct udevice *dev, struct udevice *config)
{
	return pinctrl_generic_set_state_prefix(dev, config, NULL);
}
