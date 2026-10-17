// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdev for USB
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <usb.h>

static int usb_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_5_SCAN_SLOW;

	return 0;
}

static int usb_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	if (usb_started)
		return 0;

	return usb_init();
}

struct bootdev_ops usb_bootdev_ops = {
};

static const struct udevice_id usb_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-usb" },
	{ }
};

U_BOOT_DRIVER(usb_bootdev) = {
	.name		= "usb_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &usb_bootdev_ops,
	.bind		= usb_bootdev_bind,
	.of_match	= usb_bootdev_ids,
};

BOOTDEV_HUNTER(usb_bootdev_hunter) = {
	.prio		= BOOTDEVP_5_SCAN_SLOW,
	.uclass		= UCLASS_USB,
	.hunt		= usb_bootdev_hunt,
	.drv		= DM_DRIVER_REF(usb_bootdev),
};
