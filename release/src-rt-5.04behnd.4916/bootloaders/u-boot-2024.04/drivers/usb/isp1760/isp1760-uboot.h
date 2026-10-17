/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Driver for the NXP ISP1760 chip
 *
 * Copyright 2021 Linaro, Rui Miguel Silva <rui.silva@linaro.org>
 *
 */

#ifndef __ISP1760_UBOOT_H__
#define __ISP1760_UBOOT_H__

#include <linux/usb/usb_urb_compat.h>
#include <usb.h>

#include "isp1760-core.h"

struct isp1760_host_data {
	struct isp1760_hcd *priv;
	struct usb_hcd hcd;
	enum usb_device_speed host_speed;
	struct usb_host_endpoint hep;
	struct urb urb;
};

extern struct dm_usb_ops isp1760_usb_ops;

#endif
