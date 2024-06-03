// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015  Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <linux/err.h>
#include <linux/list.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/util.h>
#include <dm/of_access.h>

DECLARE_GLOBAL_DATA_PTR;

int pinctrl_decode_pin_config(const void *blob, int node)
{
	int flags = 0;

	if (fdtdec_get_bool(blob, node, "bias-pull-up"))
		flags |= 1 << PIN_CONFIG_BIAS_PULL_UP;
	else if (fdtdec_get_bool(blob, node, "bias-pull-down"))
		flags |= 1 << PIN_CONFIG_BIAS_PULL_DOWN;

	return flags;
}

#if CONFIG_IS_ENABLED(PINCTRL_FULL)
/**
 * pinctrl_config_one() - apply pinctrl settings for a single node
 *
 * @config: pin configuration node
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_config_one(struct udevice *config)
{
	struct udevice *pctldev;
	const struct pinctrl_ops *ops;

	pctldev = config;
	for (;;) {
		pctldev = dev_get_parent(pctldev);
		if (!pctldev) {
			dev_err(config, "could not find pctldev\n");
			return -EINVAL;
		}
		if (pctldev->uclass->uc_drv->id == UCLASS_PINCTRL)
			break;
	}

	ops = pinctrl_get_ops(pctldev);
	return ops->set_state(pctldev, config);
}

/**
 * pinctrl_select_state_full() - full implementation of pinctrl_select_state
 *
 * @dev: peripheral device
 * @statename: state name, like "default"
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_select_state_full(struct udevice *dev, const char *statename)
{
	char propname[32]; /* long enough */
	const fdt32_t *list;
	uint32_t phandle;
	struct udevice *config;
	int state, size, i, ret;

	state = dev_read_stringlist_search(dev, "pinctrl-names", statename);
	if (state < 0) {
		char *end;
		/*
		 * If statename is not found in "pinctrl-names",
		 * assume statename is just the integer state ID.
		 */
		state = simple_strtoul(statename, &end, 10);
		if (*end)
			return -EINVAL;
	}

	snprintf(propname, sizeof(propname), "pinctrl-%d", state);
	list = dev_read_prop(dev, propname, &size);
	if (!list)
		return -EINVAL;

	size /= sizeof(*list);
	for (i = 0; i < size; i++) {
		phandle = fdt32_to_cpu(*list++);
		ret = uclass_get_device_by_phandle_id(UCLASS_PINCONFIG, phandle,
						      &config);
		if (ret)
			return ret;

		ret = pinctrl_config_one(config);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * pinconfig_post_bind() - post binding for PINCONFIG uclass
 * Recursively bind its children as pinconfig devices.
 *
 * @dev: pinconfig device
 * @return: 0 on success, or negative error code on failure
 */
static int pinconfig_post_bind(struct udevice *dev)
{
	bool pre_reloc_only = !(gd->flags & GD_FLG_RELOC);
	const char *name;
	ofnode node;
	int ret;

	if (!dev_of_valid(dev))
		return 0;

	dev_for_each_subnode(node, dev) {
		if (pre_reloc_only &&
		    !ofnode_pre_reloc(node))
			continue;
		/*
		 * If this node has "compatible" property, this is not
		 * a pin configuration node, but a normal device. skip.
		 */
		ofnode_get_property(node, "compatible", &ret);
		if (ret >= 0)
			continue;
		/* If this node has "gpio-controller" property, skip */
		if (ofnode_read_bool(node, "gpio-controller"))
			continue;

		if (ret != -FDT_ERR_NOTFOUND)
			return ret;

		name = ofnode_get_name(node);
		if (!name)
			return -EINVAL;
		ret = device_bind_driver_to_node(dev, "pinconfig", name,
						 node, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(pinconfig) = {
	.id = UCLASS_PINCONFIG,
	.post_bind = pinconfig_post_bind,
	.name = "pinconfig",
};

U_BOOT_DRIVER(pinconfig_generic) = {
	.name = "pinconfig",
	.id = UCLASS_PINCONFIG,
};

#else
static int pinctrl_select_state_full(struct udevice *dev, const char *statename)
{
	return -ENODEV;
}

static int pinconfig_post_bind(struct udevice *dev)
{
	return 0;
}
#endif

static int
pinctrl_gpio_get_pinctrl_and_offset(struct udevice *dev, unsigned offset,
				    struct udevice **pctldev,
				    unsigned int *pin_selector)
{
	struct ofnode_phandle_args args;
	unsigned gpio_offset, pfc_base, pfc_pins;
	int ret;

	ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3,
					 0, &args);
	if (ret) {
		dev_dbg(dev, "%s: dev_read_phandle_with_args: err=%d\n",
			__func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PINCTRL,
					  args.node, pctldev);
	if (ret) {
		dev_dbg(dev,
			"%s: uclass_get_device_by_of_offset failed: err=%d\n",
			__func__, ret);
		return ret;
	}

	gpio_offset = args.args[0];
	pfc_base = args.args[1];
	pfc_pins = args.args[2];

	if (offset < gpio_offset || offset > gpio_offset + pfc_pins) {
		dev_dbg(dev,
			"%s: GPIO can not be mapped to pincontrol pin\n",
			__func__);
		return -EINVAL;
	}

	offset -= gpio_offset;
	offset += pfc_base;
	*pin_selector = offset;

	return 0;
}

/**
 * pinctrl_gpio_request() - request a single pin to be used as GPIO
 *
 * @dev: GPIO peripheral device
 * @offset: the GPIO pin offset from the GPIO controller
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_request(struct udevice *dev, unsigned offset)
{
	const struct pinctrl_ops *ops;
	struct udevice *pctldev;
	unsigned int pin_selector;
	int ret;

	ret = pinctrl_gpio_get_pinctrl_and_offset(dev, offset,
						  &pctldev, &pin_selector);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops || !ops->gpio_request_enable)
		return -ENOTSUPP;

	return ops->gpio_request_enable(pctldev, pin_selector);
}

/**
 * pinctrl_gpio_free() - free a single pin used as GPIO
 *
 * @dev: GPIO peripheral device
 * @offset: the GPIO pin offset from the GPIO controller
 * @return: 0 on success, or negative error code on failure
 */
int pinctrl_gpio_free(struct udevice *dev, unsigned offset)
{
	const struct pinctrl_ops *ops;
	struct udevice *pctldev;
	unsigned int pin_selector;
	int ret;

	ret = pinctrl_gpio_get_pinctrl_and_offset(dev, offset,
						  &pctldev, &pin_selector);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops || !ops->gpio_disable_free)
		return -ENOTSUPP;

	return ops->gpio_disable_free(pctldev, pin_selector);
}

/**
 * pinctrl_select_state_simple() - simple implementation of pinctrl_select_state
 *
 * @dev: peripheral device
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_select_state_simple(struct udevice *dev)
{
	struct udevice *pctldev;
	struct pinctrl_ops *ops;
	int ret;

	/*
	 * For most system, there is only one pincontroller device. But in
	 * case of multiple pincontroller devices, probe the one with sequence
	 * number 0 (defined by alias) to avoid race condition.
	 */
	ret = uclass_get_device_by_seq(UCLASS_PINCTRL, 0, &pctldev);
	if (ret)
		/* if not found, get the first one */
		ret = uclass_get_device(UCLASS_PINCTRL, 0, &pctldev);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops->set_state_simple) {
		dev_dbg(dev, "set_state_simple op missing\n");
		return -ENOSYS;
	}

	return ops->set_state_simple(pctldev, dev);
}

int pinctrl_select_state(struct udevice *dev, const char *statename)
{
	/*
	 * Some device which is logical like mmc.blk, do not have
	 * a valid ofnode.
	 */
	if (!ofnode_valid(dev->node))
		return 0;
	/*
	 * Try full-implemented pinctrl first.
	 * If it fails or is not implemented, try simple one.
	 */
	if (pinctrl_select_state_full(dev, statename))
		return pinctrl_select_state_simple(dev);

	return 0;
}

int pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->request)
		return -ENOSYS;

	return ops->request(dev, func, flags);
}

int pinctrl_request_noflags(struct udevice *dev, int func)
{
	return pinctrl_request(dev, func, 0);
}

int pinctrl_get_periph_id(struct udevice *dev, struct udevice *periph)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_periph_id)
		return -ENOSYS;

	return ops->get_periph_id(dev, periph);
}

int pinctrl_get_gpio_mux(struct udevice *dev, int banknum, int index)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_gpio_mux)
		return -ENOSYS;

	return ops->get_gpio_mux(dev, banknum, index);
}

int pinctrl_get_pins_count(struct udevice *dev)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_pins_count)
		return -ENOSYS;

	return ops->get_pins_count(dev);
}

int pinctrl_get_pin_name(struct udevice *dev, int selector, char *buf,
			 int size)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_pin_name)
		return -ENOSYS;

	snprintf(buf, size, ops->get_pin_name(dev, selector));

	return 0;
}

int pinctrl_get_pin_muxing(struct udevice *dev, int selector, char *buf,
			   int size)
{
	struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops->get_pin_muxing)
		return -ENOSYS;

	return ops->get_pin_muxing(dev, selector, buf, size);
}

/**
 * pinconfig_post_bind() - post binding for PINCTRL uclass
 * Recursively bind child nodes as pinconfig devices in case of full pinctrl.
 *
 * @dev: pinctrl device
 * @return: 0 on success, or negative error code on failure
 */
static int pinctrl_post_bind(struct udevice *dev)
{
	const struct pinctrl_ops *ops = pinctrl_get_ops(dev);

	if (!ops) {
		dev_dbg(dev, "ops is not set.  Do not bind.\n");
		return -EINVAL;
	}

	/*
	 * If set_state callback is set, we assume this pinctrl driver is the
	 * full implementation.  In this case, its child nodes should be bound
	 * so that peripheral devices can easily search in parent devices
	 * during later DT-parsing.
	 */
	if (ops->set_state)
		return pinconfig_post_bind(dev);

	return 0;
}

UCLASS_DRIVER(pinctrl) = {
	.id = UCLASS_PINCTRL,
	.post_bind = pinctrl_post_bind,
	.flags = DM_UC_FLAG_SEQ_ALIAS,
	.name = "pinctrl",
};
