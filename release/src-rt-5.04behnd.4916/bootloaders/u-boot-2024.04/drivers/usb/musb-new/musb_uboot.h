/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MUSB OTG driver u-boot specific functions
 *
 * Copyright © 2015 Hans de Goede <hdegoede@redhat.com>
 */
#ifndef __MUSB_UBOOT_H__
#define __MUSB_UBOOT_H__

#include <usb.h>
#include <linux/usb/usb_urb_compat.h>
#include "linux-compat.h"
#include "musb_core.h"

struct musb_host_data {
	struct musb *host;
	struct usb_hcd hcd;
	enum usb_device_speed host_speed;
	struct usb_host_endpoint hep;
	struct urb urb;
};

extern struct dm_usb_ops musb_usb_ops;

int musb_lowlevel_init(struct musb_host_data *host);

#endif
