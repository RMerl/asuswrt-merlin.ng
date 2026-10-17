// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for the NXP ISP1760 chip
 *
 * Copyright 2021 Linaro, Rui Miguel Silva <rui.silva@linaro.org>
 *
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/usb/otg.h>
#include <linux/usb/usb_urb_compat.h>
#include <log.h>
#include <usb.h>

#include "isp1760-core.h"
#include "isp1760-hcd.h"
#include "isp1760-regs.h"
#include "isp1760-uboot.h"

static int isp1760_msg_submit_control(struct udevice *dev,
				      struct usb_device *udev,
				      unsigned long pipe, void *buffer,
				      int length, struct devrequest *setup)
{
	struct isp1760_host_data *host = dev_get_priv(dev);

	return usb_urb_submit_control(&host->hcd, &host->urb, &host->hep, udev,
				      pipe, buffer, length, setup, 0,
				      host->host_speed);
}

static int isp1760_msg_submit_bulk(struct udevice *dev, struct usb_device *udev,
				   unsigned long pipe, void *buffer, int length)
{
	struct isp1760_host_data *host = dev_get_priv(dev);

	return usb_urb_submit_bulk(&host->hcd, &host->urb, &host->hep, udev,
				   pipe, buffer, length);
}

static int isp1760_msg_submit_irq(struct udevice *dev, struct usb_device *udev,
				  unsigned long pipe, void *buffer, int length,
				  int interval, bool nonblock)
{
	struct isp1760_host_data *host = dev_get_priv(dev);

	return usb_urb_submit_irq(&host->hcd, &host->urb, &host->hep, udev,
				  pipe, buffer, length, interval);
}

static int isp1760_get_max_xfer_size(struct udevice *dev, size_t *size)
{
	struct isp1760_host_data *host = dev_get_priv(dev);
	struct isp1760_hcd *priv = host->hcd.hcd_priv;
	const struct isp1760_memory_layout *mem = priv->memory_layout;

	*size = mem->blocks_size[ISP176x_BLOCK_NUM - 1];

	return 0;
}

struct dm_usb_ops isp1760_usb_ops = {
	.control		= isp1760_msg_submit_control,
	.bulk			= isp1760_msg_submit_bulk,
	.interrupt		= isp1760_msg_submit_irq,
	.get_max_xfer_size	= isp1760_get_max_xfer_size,
};
