/*
 * printer.c -- Printer gadget driver
 *
 * Copyright (C) 2003-2005 David Brownell
 * Copyright (C) 2006 Craig W. Nadler
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/g_printer.h>

#include "gadget_chips.h"

USB_GADGET_COMPOSITE_OPTIONS();

#define DRIVER_DESC		"Printer Gadget"
#define DRIVER_VERSION		"2015 FEB 17"

static const char shortname [] = "printer";
static const char driver_desc [] = DRIVER_DESC;

#include "u_printer.h"

/*-------------------------------------------------------------------------*/

/* DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
 * Instead:  allocate your own, using normal USB-IF procedures.
 */

/* Thanks to NetChip Technologies for donating this product ID.
 */
#define PRINTER_VENDOR_NUM	0x0525		/* NetChip */
#define PRINTER_PRODUCT_NUM	0xa4a8		/* Linux-USB Printer Gadget */

/* Some systems will want different product identifiers published in the
 * device descriptor, either numbers or strings or both.  These string
 * parameters are in UTF-8 (superset of ASCII's 7 bit characters).
 */

module_param_named(iSerialNum, coverwrite.serial_number, charp, S_IRUGO);
MODULE_PARM_DESC(iSerialNum, "1");

static char *iPNPstring;
module_param(iPNPstring, charp, S_IRUGO);
MODULE_PARM_DESC(iPNPstring, "MFG:linux;MDL:g_printer;CLS:PRINTER;SN:1;");

/* Number of requests to allocate per endpoint, not used for ep0. */
static unsigned qlen = 10;
module_param(qlen, uint, S_IRUGO|S_IWUSR);

#define QLEN	qlen

static struct usb_function_instance *fi_printer;
static struct usb_function *f_printer;

/*-------------------------------------------------------------------------*/

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.
 */

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB =		cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	.idVendor =		cpu_to_le16(PRINTER_VENDOR_NUM),
	.idProduct =		cpu_to_le16(PRINTER_PRODUCT_NUM),
	.bNumConfigurations =	1
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =              sizeof otg_descriptor,
	.bDescriptorType =      USB_DT_OTG,
	.bmAttributes =         USB_OTG_SRP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};

/*-------------------------------------------------------------------------*/

/* descriptors that are built on-demand */

static char				product_desc [40] = DRIVER_DESC;
static char				serial_num [40] = "1";
static char				pnp_string[PNP_STRING_LEN] =
	"XXMFG:linux;MDL:g_printer;CLS:PRINTER;SN:1;";

/* static strings, in UTF-8 */
static struct usb_string		strings [] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "",
	[USB_GADGET_PRODUCT_IDX].s = product_desc,
	[USB_GADGET_SERIAL_IDX].s =	serial_num,
	{  }		/* end of list */
};

static struct usb_gadget_strings	stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_configuration printer_cfg_driver = {
	.label			= "printer",
	.bConfigurationValue	= 1,
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
};

static int printer_do_config(struct usb_configuration *c)
{
	struct usb_gadget	*gadget = c->cdev->gadget;
	int			status = 0;

	usb_ep_autoconfig_reset(gadget);

	usb_gadget_set_selfpowered(gadget);

	if (gadget_is_otg(gadget)) {
		otg_descriptor.bmAttributes |= USB_OTG_HNP;
		printer_cfg_driver.descriptors = otg_desc;
		printer_cfg_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	f_printer = usb_get_function(fi_printer);
	if (IS_ERR(f_printer))
		return PTR_ERR(f_printer);

	status = usb_add_function(c, f_printer);
	if (status < 0)
		usb_put_function(f_printer);

	return status;
}

static int printer_bind(struct usb_composite_dev *cdev)
{
	struct f_printer_opts *opts;
	int ret, len;

	fi_printer = usb_get_function_instance("printer");
	if (IS_ERR(fi_printer))
		return PTR_ERR(fi_printer);

	if (iPNPstring)
		strlcpy(&pnp_string[2], iPNPstring, PNP_STRING_LEN - 2);

	len = strlen(pnp_string);
	pnp_string[0] = (len >> 8) & 0xFF;
	pnp_string[1] = len & 0xFF;

	opts = container_of(fi_printer, struct f_printer_opts, func_inst);
	opts->minor = 0;
	memcpy(opts->pnp_string, pnp_string, PNP_STRING_LEN);
	opts->q_len = QLEN;

	ret = usb_string_ids_tab(cdev, strings);
	if (ret < 0) {
		usb_put_function_instance(fi_printer);
		return ret;
	}
	device_desc.iManufacturer = strings[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings[USB_GADGET_PRODUCT_IDX].id;
	device_desc.iSerialNumber = strings[USB_GADGET_SERIAL_IDX].id;

	ret = usb_add_config(cdev, &printer_cfg_driver, printer_do_config);
	if (ret) {
		usb_put_function_instance(fi_printer);
		return ret;
	}
	usb_composite_overwrite_options(cdev, &coverwrite);
	return ret;
}

static int printer_unbind(struct usb_composite_dev *cdev)
{
	usb_put_function(f_printer);
	usb_put_function_instance(fi_printer);

	return 0;
}

static struct usb_composite_driver printer_driver = {
	.name           = shortname,
	.dev            = &device_desc,
	.strings        = dev_strings,
	.max_speed      = USB_SPEED_SUPER,
	.bind		= printer_bind,
	.unbind		= printer_unbind,
};

module_usb_composite_driver(printer_driver);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Craig Nadler");
MODULE_LICENSE("GPL");
