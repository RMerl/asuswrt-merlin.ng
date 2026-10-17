// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

struct onboard_hub {
	struct udevice *vdd;
};

static int usb_onboard_hub_probe(struct udevice *dev)
{
	struct onboard_hub *hub = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vdd-supply", &hub->vdd);
	if (ret) {
		dev_err(dev, "can't get vdd-supply: %d\n", ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(hub->vdd, true);
	if (ret)
		dev_err(dev, "can't enable vdd-supply: %d\n", ret);

	return ret;
}

static int usb_onboard_hub_remove(struct udevice *dev)
{
	struct onboard_hub *hub = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(hub->vdd, false);
	if (ret)
		dev_err(dev, "can't disable vdd-supply: %d\n", ret);

	return ret;
}

static const struct udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{ .compatible = "usb424,2514" }, /* USB2514B USB 2.0 */
	{ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.of_match = usb_onboard_hub_ids,
	.priv_auto = sizeof(struct onboard_hub),
};
