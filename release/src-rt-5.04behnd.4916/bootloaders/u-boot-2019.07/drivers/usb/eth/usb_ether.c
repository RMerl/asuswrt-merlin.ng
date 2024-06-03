// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <usb.h>
#include <dm/device-internal.h>

#include "usb_ether.h"

#ifdef CONFIG_DM_ETH

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

#else

typedef void (*usb_eth_before_probe)(void);
typedef int (*usb_eth_probe)(struct usb_device *dev, unsigned int ifnum,
			struct ueth_data *ss);
typedef int (*usb_eth_get_info)(struct usb_device *dev, struct ueth_data *ss,
			struct eth_device *dev_desc);

struct usb_eth_prob_dev {
	usb_eth_before_probe	before_probe; /* optional */
	usb_eth_probe			probe;
	usb_eth_get_info		get_info;
};

/* driver functions go here, each bracketed by #ifdef CONFIG_USB_ETHER_xxx */
static const struct usb_eth_prob_dev prob_dev[] = {
#ifdef CONFIG_USB_ETHER_ASIX
	{
		.before_probe = asix_eth_before_probe,
		.probe = asix_eth_probe,
		.get_info = asix_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_ASIX88179
	{
		.before_probe = ax88179_eth_before_probe,
		.probe = ax88179_eth_probe,
		.get_info = ax88179_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_MCS7830
	{
		.before_probe = mcs7830_eth_before_probe,
		.probe = mcs7830_eth_probe,
		.get_info = mcs7830_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_SMSC95XX
	{
		.before_probe = smsc95xx_eth_before_probe,
		.probe = smsc95xx_eth_probe,
		.get_info = smsc95xx_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_RTL8152
	{
		.before_probe = r8152_eth_before_probe,
		.probe = r8152_eth_probe,
		.get_info = r8152_eth_get_info,
	},
#endif
	{ },		/* END */
};

static int usb_max_eth_dev; /* number of highest available usb eth device */
static struct ueth_data usb_eth[USB_MAX_ETH_DEV];

/*******************************************************************************
 * tell if current ethernet device is a usb dongle
 */
int is_eth_dev_on_usb_host(void)
{
	int i;
	struct eth_device *dev = eth_get_dev();

	if (dev) {
		for (i = 0; i < usb_max_eth_dev; i++)
			if (&usb_eth[i].eth_dev == dev)
				return 1;
	}
	return 0;
}

/*
 * Given a USB device, ask each driver if it can support it, and attach it
 * to the first driver that says 'yes'
 */
static void probe_valid_drivers(struct usb_device *dev)
{
	struct eth_device *eth;
	int j;

	for (j = 0; prob_dev[j].probe && prob_dev[j].get_info; j++) {
		if (!prob_dev[j].probe(dev, 0, &usb_eth[usb_max_eth_dev]))
			continue;
		/*
		 * ok, it is a supported eth device. Get info and fill it in
		 */
		eth = &usb_eth[usb_max_eth_dev].eth_dev;
		if (prob_dev[j].get_info(dev,
			&usb_eth[usb_max_eth_dev],
			eth)) {
			/* found proper driver */
			/* register with networking stack */
			usb_max_eth_dev++;

			/*
			 * usb_max_eth_dev must be incremented prior to this
			 * call since eth_current_changed (internally called)
			 * relies on it
			 */
			eth_register(eth);
			if (eth_write_hwaddr(eth, "usbeth",
					usb_max_eth_dev - 1))
				puts("Warning: failed to set MAC address\n");
			break;
			}
		}
	}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
int usb_host_eth_scan(int mode)
{
	int i, old_async;

	if (mode == 1)
		printf("       scanning usb for ethernet devices... ");

	old_async = usb_disable_asynch(1); /* asynch transfer not allowed */

	/* unregister a previously detected device */
	for (i = 0; i < usb_max_eth_dev; i++)
		eth_unregister(&usb_eth[i].eth_dev);

	memset(usb_eth, 0, sizeof(usb_eth));

	for (i = 0; prob_dev[i].probe; i++) {
		if (prob_dev[i].before_probe)
			prob_dev[i].before_probe();
	}

	usb_max_eth_dev = 0;
#if CONFIG_IS_ENABLED(DM_USB)
	/*
	 * TODO: We should add U_BOOT_USB_DEVICE() declarations to each USB
	 * Ethernet driver and then most of this file can be removed.
	 */
	struct udevice *bus;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(bus, uc) {
		for (i = 0; i < USB_MAX_DEVICE; i++) {
			struct usb_device *dev;

			dev = usb_get_dev_index(bus, i); /* get device */
			debug("i=%d, %s\n", i, dev ? dev->dev->name : "(done)");
			if (!dev)
				break; /* no more devices available */

			/*
			 * find valid usb_ether driver for this device,
			 * if any
			 */
			probe_valid_drivers(dev);

			/* check limit */
			if (usb_max_eth_dev == USB_MAX_ETH_DEV)
				break;
		} /* for */
	}
#else
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		struct usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		if (!dev)
			break; /* no more devices available */

		/* find valid usb_ether driver for this device, if any */
		probe_valid_drivers(dev);

		/* check limit */
		if (usb_max_eth_dev == USB_MAX_ETH_DEV)
			break;
	} /* for */
#endif
	if (usb_max_eth_dev == USB_MAX_ETH_DEV) {
		printf("max USB Ethernet Device reached: %d stopping\n",
		       usb_max_eth_dev);
	}
	usb_disable_asynch(old_async); /* restore asynch value */
	printf("%d Ethernet Device(s) found\n", usb_max_eth_dev);
	if (usb_max_eth_dev > 0)
		return 0;
	return -1;
}
#endif
