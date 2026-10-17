// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_LED

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

int led_bind_generic(struct udevice *parent, const char *driver_name)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		ret = device_bind_driver_to_node(parent, driver_name,
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
	}

	return 0;
}

int led_get_by_label(const char *label, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

		/* Ignore the top-level LED node */
		if (uc_plat->label && !strcmp(label, uc_plat->label))
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->set_state)
		return -ENOSYS;

	return ops->set_state(dev, state);
}

enum led_state_t led_get_state(struct udevice *dev)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->get_state)
		return -ENOSYS;

	return ops->get_state(dev);
}

#ifdef CONFIG_LED_BLINK
int led_set_period(struct udevice *dev, int period_ms)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->set_period)
		return -ENOSYS;

	return ops->set_period(dev, period_ms);
}
#endif

static int led_post_bind(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	const char *default_state;

	if (!uc_plat->label)
		uc_plat->label = dev_read_string(dev, "label");

	if (!uc_plat->label && !dev_read_string(dev, "compatible"))
		uc_plat->label = ofnode_get_name(dev_ofnode(dev));

	uc_plat->default_state = LEDST_COUNT;

	default_state = dev_read_string(dev, "default-state");
	if (!default_state)
		return 0;

	if (!strncmp(default_state, "on", 2))
		uc_plat->default_state = LEDST_ON;
	else if (!strncmp(default_state, "off", 3))
		uc_plat->default_state = LEDST_OFF;
	else
		return 0;

	/*
	 * In case the LED has default-state DT property, trigger
	 * probe() to configure its default state during startup.
	 */
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static int led_post_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	if (uc_plat->default_state == LEDST_ON ||
	    uc_plat->default_state == LEDST_OFF)
		led_set_state(dev, uc_plat->default_state);

	return 0;
}

UCLASS_DRIVER(led) = {
	.id		= UCLASS_LED,
	.name		= "led",
	.per_device_plat_auto	= sizeof(struct led_uc_plat),
	.post_bind	= led_post_bind,
	.post_probe	= led_post_probe,
};
