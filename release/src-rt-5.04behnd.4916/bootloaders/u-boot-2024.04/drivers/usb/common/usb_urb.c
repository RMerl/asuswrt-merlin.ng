// SPDX-License-Identifier: GPL-2.0
/*
 * Common code for usb urb handling, based on the musb-new code
 *
 * Copyright 2021 Linaro, Rui Miguel Silva <rui.silva@linaro.org>
 *
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/usb/usb_urb_compat.h>

#include <time.h>
#include <usb.h>

#if CONFIG_IS_ENABLED(DM_USB)
struct usb_device *usb_dev_get_parent(struct usb_device *udev)
{
	struct udevice *parent = udev->dev->parent;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet.
	 *
	 * If dev is an usb-bus, then we are called from usb_scan_device() for
	 * an usb-device plugged directly into the root port, return NULL.
	 */
	if (device_get_uclass_id(udev->dev) == UCLASS_USB)
		return NULL;

	/*
	 * If these 2 are not the same we are being called from
	 * usb_scan_device() and udev itself is the parent.
	 */
	if (dev_get_parent_priv(udev->dev) != udev)
		return udev;

	/* We are being called normally, use the parent pointer */
	if (device_get_uclass_id(parent) == UCLASS_USB_HUB)
		return dev_get_parent_priv(parent);

	return NULL;
}
#else
struct usb_device *usb_dev_get_parent(struct usb_device *udev)
{
	return udev->parent;
}
#endif

static void usb_urb_complete(struct urb *urb)
{
	urb->dev->status &= ~USB_ST_NOT_PROC;
	urb->dev->act_len = urb->actual_length;

	if (urb->status == -EINPROGRESS)
		urb->status = 0;
}

void usb_urb_fill(struct urb *urb, struct usb_host_endpoint *hep,
		  struct usb_device *dev, int endpoint_type,
		  unsigned long pipe, void *buffer, int len,
		  struct devrequest *setup, int interval)
{
	int epnum = usb_pipeendpoint(pipe);
	int is_in = usb_pipein(pipe);
	u16 maxpacketsize = is_in ? dev->epmaxpacketin[epnum] :
					dev->epmaxpacketout[epnum];

	memset(urb, 0, sizeof(struct urb));
	memset(hep, 0, sizeof(struct usb_host_endpoint));
	INIT_LIST_HEAD(&hep->urb_list);
	INIT_LIST_HEAD(&urb->urb_list);
	urb->ep = hep;
	urb->complete = usb_urb_complete;
	urb->status = -EINPROGRESS;
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = buffer;
	urb->transfer_dma = (unsigned long)buffer;
	urb->transfer_buffer_length = len;
	urb->setup_packet = (unsigned char *)setup;

	urb->ep->desc.wMaxPacketSize = __cpu_to_le16(maxpacketsize);
	urb->ep->desc.bmAttributes = endpoint_type;
	urb->ep->desc.bEndpointAddress = ((is_in ? USB_DIR_IN : USB_DIR_OUT) |
					  epnum);
	urb->ep->desc.bInterval = interval;
}

int usb_urb_submit(struct usb_hcd *hcd, struct urb *urb)
{
	const struct usb_urb_ops *ops = hcd->urb_ops;
	unsigned long timeout;
	int ret;

	if (!ops)
		return -EINVAL;

	ret = ops->urb_enqueue(hcd, urb, 0);
	if (ret < 0) {
		printf("Failed to enqueue URB to controller\n");
		return ret;
	}

	timeout = get_timer(0) + USB_TIMEOUT_MS(urb->pipe);
	do {
		if (ctrlc())
			return -EIO;
		ops->isr(0, hcd);
	} while (urb->status == -EINPROGRESS && get_timer(0) < timeout);

	if (urb->status == -EINPROGRESS)
		ops->urb_dequeue(hcd, urb, -ETIME);

	return urb->status;
}

int usb_urb_submit_control(struct usb_hcd *hcd, struct urb *urb,
			   struct usb_host_endpoint *hep,
			   struct usb_device *dev, unsigned long pipe,
			   void *buffer, int len, struct devrequest *setup,
			   int interval, enum usb_device_speed speed)
{
	const struct usb_urb_ops *ops = hcd->urb_ops;

	usb_urb_fill(urb, hep, dev, USB_ENDPOINT_XFER_CONTROL, pipe, buffer,
		     len, setup, 0);

	/* Fix speed for non hub-attached devices */
	if (!usb_dev_get_parent(dev)) {
		dev->speed = speed;
		if (ops->hub_control)
			return ops->hub_control(hcd, dev, pipe, buffer, len,
						setup);
	}

	return usb_urb_submit(hcd, urb);
}

int usb_urb_submit_bulk(struct usb_hcd *hcd, struct urb *urb,
			struct usb_host_endpoint *hep, struct usb_device *dev,
			unsigned long pipe, void *buffer, int len)
{
	usb_urb_fill(urb, hep, dev, USB_ENDPOINT_XFER_BULK, pipe, buffer, len,
		     NULL, 0);

	return usb_urb_submit(hcd, urb);
}

int usb_urb_submit_irq(struct usb_hcd *hcd, struct urb *urb,
		       struct usb_host_endpoint *hep, struct usb_device *dev,
		       unsigned long pipe, void *buffer, int len, int interval)
{
	usb_urb_fill(urb, hep, dev, USB_ENDPOINT_XFER_INT, pipe, buffer, len,
		     NULL, interval);

	return usb_urb_submit(hcd, urb);
}
