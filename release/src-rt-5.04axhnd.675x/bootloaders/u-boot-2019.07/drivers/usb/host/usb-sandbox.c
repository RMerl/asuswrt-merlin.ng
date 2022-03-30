// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <dm/root.h>

struct sandbox_usb_ctrl {
	int rootdev;
};

static void usbmon_trace(struct udevice *bus, ulong pipe,
			 struct devrequest *setup, struct udevice *emul)
{
	static const char types[] = "ZICB";
	int type;

	type = (pipe & USB_PIPE_TYPE_MASK) >> USB_PIPE_TYPE_SHIFT;
	debug("0 0 S %c%c:%d:%03ld:%ld", types[type],
	      pipe & USB_DIR_IN ? 'i' : 'o',
	      bus->seq,
	      (pipe & USB_PIPE_DEV_MASK) >> USB_PIPE_DEV_SHIFT,
	      (pipe & USB_PIPE_EP_MASK) >> USB_PIPE_EP_SHIFT);
	if (setup) {
		debug(" s %02x %02x %04x %04x %04x", setup->requesttype,
		      setup->request, setup->value, setup->index,
		      setup->length);
	}
	debug(" %s", emul ? emul->name : "(no emul found)");

	debug("\n");
}

static int sandbox_submit_control(struct udevice *bus,
				      struct usb_device *udev,
				      unsigned long pipe,
				      void *buffer, int length,
				      struct devrequest *setup)
{
	struct sandbox_usb_ctrl *ctrl = dev_get_priv(bus);
	struct udevice *emul;
	int ret;

	/* Just use child of dev as emulator? */
	debug("%s: bus=%s\n", __func__, bus->name);
	ret = usb_emul_find(bus, pipe, udev->portnr, &emul);
	usbmon_trace(bus, pipe, setup, emul);
	if (ret)
		return ret;

	if (usb_pipedevice(pipe) == ctrl->rootdev) {
		if (setup->request == USB_REQ_SET_ADDRESS) {
			debug("%s: Set root hub's USB address\n", __func__);
			ctrl->rootdev = le16_to_cpu(setup->value);
		}
	}

	ret = usb_emul_control(emul, udev, pipe, buffer, length, setup);
	if (ret < 0) {
		debug("ret=%d\n", ret);
		udev->status = ret;
		udev->act_len = 0;
	} else {
		udev->status = 0;
		udev->act_len = ret;
	}

	return ret;
}

static int sandbox_submit_bulk(struct udevice *bus, struct usb_device *udev,
			       unsigned long pipe, void *buffer, int length)
{
	struct udevice *emul;
	int ret;

	/* Just use child of dev as emulator? */
	debug("%s: bus=%s\n", __func__, bus->name);
	ret = usb_emul_find(bus, pipe, udev->portnr, &emul);
	usbmon_trace(bus, pipe, NULL, emul);
	if (ret)
		return ret;
	ret = usb_emul_bulk(emul, udev, pipe, buffer, length);
	if (ret < 0) {
		debug("ret=%d\n", ret);
		udev->status = ret;
		udev->act_len = 0;
	} else {
		udev->status = 0;
		udev->act_len = ret;
	}

	return ret;
}

static int sandbox_submit_int(struct udevice *bus, struct usb_device *udev,
			      unsigned long pipe, void *buffer, int length,
			      int interval)
{
	struct udevice *emul;
	int ret;

	/* Just use child of dev as emulator? */
	debug("%s: bus=%s\n", __func__, bus->name);
	ret = usb_emul_find(bus, pipe, udev->portnr, &emul);
	usbmon_trace(bus, pipe, NULL, emul);
	if (ret)
		return ret;
	ret = usb_emul_int(emul, udev, pipe, buffer, length, interval);

	return ret;
}

static int sandbox_alloc_device(struct udevice *dev, struct usb_device *udev)
{
	struct sandbox_usb_ctrl *ctrl = dev_get_priv(dev);

	/*
	 * Root hub will be the first device to be initailized.
	 * If this device is a root hub, initialize its device speed
	 * to high speed as we are a USB 2.0 controller.
	 */
	if (ctrl->rootdev == 0)
		udev->speed = USB_SPEED_HIGH;

	return 0;
}

static int sandbox_usb_probe(struct udevice *dev)
{
	return 0;
}

static const struct dm_usb_ops sandbox_usb_ops = {
	.control	= sandbox_submit_control,
	.bulk		= sandbox_submit_bulk,
	.interrupt	= sandbox_submit_int,
	.alloc_device	= sandbox_alloc_device,
};

static const struct udevice_id sandbox_usb_ids[] = {
	{ .compatible = "sandbox,usb" },
	{ }
};

U_BOOT_DRIVER(usb_sandbox) = {
	.name	= "usb_sandbox",
	.id	= UCLASS_USB,
	.of_match = sandbox_usb_ids,
	.probe = sandbox_usb_probe,
	.ops	= &sandbox_usb_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_usb_ctrl),
};
