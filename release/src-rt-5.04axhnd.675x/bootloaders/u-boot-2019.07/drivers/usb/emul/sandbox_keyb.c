// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <os.h>
#include <scsi.h>
#include <usb.h>

/*
 * This driver emulates a USB keyboard using the USB HID specification (boot
 * protocol)
 */

enum {
	SANDBOX_KEYB_EP_IN		= 1,	/* endpoints */
};

enum cmd_phase {
	PHASE_START,
	PHASE_DATA,
	PHASE_STATUS,
};

enum {
	STRINGID_MANUFACTURER = 1,
	STRINGID_PRODUCT,
	STRINGID_SERIAL,

	STRINGID_COUNT,
};

/**
 * struct sandbox_keyb_priv - private state for this driver
 *
 */
struct sandbox_keyb_priv {
	struct membuff in;
};

struct sandbox_keyb_plat {
	struct usb_string keyb_strings[STRINGID_COUNT];
};

static struct usb_device_descriptor keyb_device_desc = {
	.bLength =		sizeof(keyb_device_desc),
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0100),

	.bDeviceClass =		0,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,

	.idVendor =		__constant_cpu_to_le16(0x1234),
	.idProduct =		__constant_cpu_to_le16(0x5679),
	.iManufacturer =	STRINGID_MANUFACTURER,
	.iProduct =		STRINGID_PRODUCT,
	.iSerialNumber =	STRINGID_SERIAL,
	.bNumConfigurations =	1,
};

static struct usb_config_descriptor keyb_config0 = {
	.bLength		= sizeof(keyb_config0),
	.bDescriptorType	= USB_DT_CONFIG,

	/* wTotalLength is set up by usb-emul-uclass */
	.bNumInterfaces		= 2,
	.bConfigurationValue	= 0,
	.iConfiguration		= 0,
	.bmAttributes		= 1 << 7 | 1 << 5,
	.bMaxPower		= 50,
};

static struct usb_interface_descriptor keyb_interface0 = {
	.bLength		= sizeof(keyb_interface0),
	.bDescriptorType	= USB_DT_INTERFACE,

	.bInterfaceNumber	= 0,
	.bAlternateSetting	= 0,
	.bNumEndpoints		= 1,
	.bInterfaceClass	= USB_CLASS_HID,
	.bInterfaceSubClass	= USB_SUB_HID_BOOT,
	.bInterfaceProtocol	= USB_PROT_HID_KEYBOARD,
	.iInterface		= 0,
};

static struct usb_class_hid_descriptor keyb_report0 = {
	.bLength		= sizeof(keyb_report0),
	.bDescriptorType	= USB_DT_HID,
	.bcdCDC			= 0x101,
	.bCountryCode		= 0,
	.bNumDescriptors	= 1,
	.bDescriptorType0	= USB_DT_HID_REPORT,
	.wDescriptorLength0	= 0x3f,
};

static struct usb_endpoint_descriptor keyb_endpoint0_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,

	.bEndpointAddress	= SANDBOX_KEYB_EP_IN | USB_ENDPOINT_DIR_MASK,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK |
					USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize		= __constant_cpu_to_le16(8),
	.bInterval		= 0xa,
};

static struct usb_interface_descriptor keyb_interface1 = {
	.bLength		= sizeof(keyb_interface1),
	.bDescriptorType	= USB_DT_INTERFACE,

	.bInterfaceNumber	= 1,
	.bAlternateSetting	= 0,
	.bNumEndpoints		= 1,
	.bInterfaceClass	= USB_CLASS_HID,
	.bInterfaceSubClass	= USB_SUB_HID_BOOT,
	.bInterfaceProtocol	= USB_PROT_HID_MOUSE,
	.iInterface		= 0,
};

static struct usb_class_hid_descriptor keyb_report1 = {
	.bLength		= sizeof(struct usb_class_hid_descriptor),
	.bDescriptorType	= USB_DT_HID,
	.bcdCDC			= 0x101,
	.bCountryCode		= 0,
	.bNumDescriptors	= 1,
	.bDescriptorType0	= USB_DT_HID_REPORT,
	.wDescriptorLength0	= 0x32,
};

static struct usb_endpoint_descriptor keyb_endpoint1_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,

	.bEndpointAddress	= SANDBOX_KEYB_EP_IN | USB_ENDPOINT_DIR_MASK,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK |
					USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize		= __constant_cpu_to_le16(8),
	.bInterval		= 0xa,
};

static void *keyb_desc_list[] = {
	&keyb_device_desc,
	&keyb_config0,
	&keyb_interface0,
	&keyb_report0,
	&keyb_endpoint0_in,
	&keyb_interface1,
	&keyb_report1,
	&keyb_endpoint1_in,
	NULL,
};

int sandbox_usb_keyb_add_string(struct udevice *dev, const char *str)
{
	struct sandbox_keyb_priv *priv = dev_get_priv(dev);
	int len, ret;

	len = strlen(str);
	ret = membuff_put(&priv->in, str, len);
	if (ret != len)
		return -ENOSPC;

	return 0;
}

static int sandbox_keyb_control(struct udevice *dev, struct usb_device *udev,
				unsigned long pipe, void *buff, int len,
				struct devrequest *setup)
{
	debug("pipe=%lx\n", pipe);

	return -EIO;
}

static int sandbox_keyb_interrupt(struct udevice *dev, struct usb_device *udev,
		unsigned long pipe, void *buffer, int length, int interval)
{
	struct sandbox_keyb_priv *priv = dev_get_priv(dev);
	uint8_t *data = buffer;
	int ch;

	memset(data, '\0', length);
	ch = membuff_getbyte(&priv->in);
	if (ch != -1)
		data[2] = 4 + ch - 'a';

	return 0;
}

static int sandbox_keyb_bind(struct udevice *dev)
{
	struct sandbox_keyb_plat *plat = dev_get_platdata(dev);
	struct usb_string *fs;

	fs = plat->keyb_strings;
	fs[0].id = STRINGID_MANUFACTURER;
	fs[0].s = "sandbox";
	fs[1].id = STRINGID_PRODUCT;
	fs[1].s = "keyboard";
	fs[2].id = STRINGID_SERIAL;
	fs[2].s = dev->name;

	return usb_emul_setup_device(dev, plat->keyb_strings, keyb_desc_list);
}

static int sandbox_keyb_probe(struct udevice *dev)
{
	struct sandbox_keyb_priv *priv = dev_get_priv(dev);

	return membuff_new(&priv->in, 256);
}

static const struct dm_usb_ops sandbox_usb_keyb_ops = {
	.control	= sandbox_keyb_control,
	.interrupt	= sandbox_keyb_interrupt,
};

static const struct udevice_id sandbox_usb_keyb_ids[] = {
	{ .compatible = "sandbox,usb-keyb" },
	{ }
};

U_BOOT_DRIVER(usb_sandbox_keyb) = {
	.name	= "usb_sandbox_keyb",
	.id	= UCLASS_USB_EMUL,
	.of_match = sandbox_usb_keyb_ids,
	.bind	= sandbox_keyb_bind,
	.probe	= sandbox_keyb_probe,
	.ops	= &sandbox_usb_keyb_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_keyb_priv),
	.platdata_auto_alloc_size = sizeof(struct sandbox_keyb_plat),
};
