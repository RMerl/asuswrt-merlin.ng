// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com
 * Written by Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#define LOG_CATEGORY UCLASS_USB_GADGET_GENERIC

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/printk.h>
#include <linux/usb/gadget.h>

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
int udc_device_get_by_index(int index, struct udevice **udev)
{
	struct udevice *dev = NULL;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_USB_GADGET_GENERIC, index, &dev);
	if (!ret && dev) {
		*udev = dev;
		return 0;
	}

	ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, index, &dev);
	if (!ret && dev) {
		*udev = dev;
		return 0;
	}

	pr_err("No USB device found\n");
	return -ENODEV;
}

int udc_device_put(struct udevice *udev)
{
#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
	return device_remove(udev, DM_REMOVE_NORMAL);
#else
	return -ENOSYS;
#endif
}
#else
/* Backwards hardware compatibility -- switch to DM_USB_GADGET */
static int legacy_index;
int udc_device_get_by_index(int index, struct udevice **udev)
{
	legacy_index = index;
	return board_usb_init(index, USB_INIT_DEVICE);
}

int udc_device_put(struct udevice *udev)
{
	return board_usb_cleanup(legacy_index, USB_INIT_DEVICE);
}
#endif

#if CONFIG_IS_ENABLED(DM)
UCLASS_DRIVER(usb_gadget_generic) = {
	.id		= UCLASS_USB_GADGET_GENERIC,
	.name		= "usb",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};
#endif
