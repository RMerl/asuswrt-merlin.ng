// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <usb.h>
#include <asm/cache.h>
#include <dm/device-internal.h>

#include "usb_ether.h"

#define USB_BULK_RECV_TIMEOUT 500

int usb_ether_register(struct udevice *dev, struct ueth_data *ueth, int rxsize)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct usb_interface_descriptor *iface_desc;
	bool ep_in_found = false, ep_out_found = false;
	struct usb_interface *iface;
	const int ifnum = 0; /* Always use interface 0 */
	int ret, i;

	iface = &udev->config.if_desc[ifnum];
	iface_desc = &udev->config.if_desc[ifnum].desc;

	/* Initialize the ueth_data structure with some useful info */
	ueth->ifnum = ifnum;
	ueth->subclass = iface_desc->bInterfaceSubClass;
	ueth->protocol = iface_desc->bInterfaceProtocol;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and int.
	 * We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		int ep_addr = iface->ep_desc[i].bEndpointAddress;

		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			if (ep_addr & USB_DIR_IN && !ep_in_found) {
				ueth->ep_in = ep_addr &
					USB_ENDPOINT_NUMBER_MASK;
				ep_in_found = true;
			} else if (!(ep_addr & USB_DIR_IN) && !ep_out_found) {
				ueth->ep_out = ep_addr &
					USB_ENDPOINT_NUMBER_MASK;
				ep_out_found = true;
			}
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ueth->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ueth->irqinterval = iface->ep_desc[i].bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n", ueth->ep_in, ueth->ep_out,
	      ueth->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (!ueth->ep_in || !ueth->ep_out || !ueth->ep_int) {
		debug("%s: %s: Cannot find endpoints\n", __func__, dev->name);
		return -ENXIO;
	}

	ueth->rxsize = rxsize;
	ueth->rxbuf = memalign(ARCH_DMA_MINALIGN, rxsize);
	if (!ueth->rxbuf)
		return -ENOMEM;

	ret = usb_set_interface(udev, iface_desc->bInterfaceNumber, ifnum);
	if (ret) {
		debug("%s: %s: Cannot set interface: %d\n", __func__, dev->name,
		      ret);
		return ret;
	}
	ueth->pusb_dev = udev;

	return 0;
}

int usb_ether_deregister(struct ueth_data *ueth)
{
	return 0;
}

int usb_ether_receive(struct ueth_data *ueth, int rxsize)
{
	int actual_len;
	int ret;

	if (rxsize > ueth->rxsize)
		return -EINVAL;
	ret = usb_bulk_msg(ueth->pusb_dev,
			   usb_rcvbulkpipe(ueth->pusb_dev, ueth->ep_in),
			   ueth->rxbuf, rxsize, &actual_len,
			   USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", rxsize, actual_len, ret);
	if (ret) {
		printf("Rx: failed to receive: %d\n", ret);
		return ret;
	}
	if (actual_len > rxsize) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -ENOSPC;
	}
	ueth->rxlen = actual_len;
	ueth->rxptr = 0;

	return actual_len ? 0 : -EAGAIN;
}

void usb_ether_advance_rxbuf(struct ueth_data *ueth, int num_bytes)
{
	ueth->rxptr += num_bytes;
	if (num_bytes < 0 || ueth->rxptr >= ueth->rxlen)
		ueth->rxlen = 0;
}

int usb_ether_get_rx_bytes(struct ueth_data *ueth, uint8_t **ptrp)
{
	if (!ueth->rxlen)
		return 0;

	*ptrp = &ueth->rxbuf[ueth->rxptr];

	return ueth->rxlen - ueth->rxptr;
}
