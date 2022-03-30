// SPDX-License-Identifier: GPL-2.0+
/*
 * g_dnl.c -- USB Downloader Gadget
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 */

#include <common.h>
#include <malloc.h>

#include <mmc.h>
#include <part.h>
#include <usb.h>

#include <g_dnl.h>
#include <usb_mass_storage.h>
#include <dfu.h>
#include <thor.h>

#include <env_callback.h>

#include "gadget_chips.h"
#include "composite.c"

/*
 * One needs to define the following:
 * CONFIG_USB_GADGET_VENDOR_NUM
 * CONFIG_USB_GADGET_PRODUCT_NUM
 * CONFIG_USB_GADGET_MANUFACTURER
 * at e.g. ./configs/<board>_defconfig
 */

#define STRING_MANUFACTURER 25
#define STRING_PRODUCT 2
/* Index of String Descriptor describing this configuration */
#define STRING_USBDOWN 2
/* Index of String serial */
#define STRING_SERIAL  3
#define MAX_STRING_SERIAL	256
/* Number of supported configurations */
#define CONFIGURATION_NUMBER 1

#define DRIVER_VERSION		"usb_dnl 2.0"

static const char product[] = "USB download gadget";
static char g_dnl_serial[MAX_STRING_SERIAL];
static const char manufacturer[] = CONFIG_USB_GADGET_MANUFACTURER;

void g_dnl_set_serialnumber(char *s)
{
	memset(g_dnl_serial, 0, MAX_STRING_SERIAL);
	strncpy(g_dnl_serial, s, MAX_STRING_SERIAL - 1);
}

static struct usb_device_descriptor device_desc = {
	.bLength = sizeof device_desc,
	.bDescriptorType = USB_DT_DEVICE,

	.bcdUSB = __constant_cpu_to_le16(0x0200),
	.bDeviceClass = USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass = 0, /*0x02:CDC-modem , 0x00:CDC-serial*/

	.idVendor = __constant_cpu_to_le16(CONFIG_USB_GADGET_VENDOR_NUM),
	.idProduct = __constant_cpu_to_le16(CONFIG_USB_GADGET_PRODUCT_NUM),
	/* .iProduct = DYNAMIC */
	/* .iSerialNumber = DYNAMIC */
	.bNumConfigurations = 1,
};

/*
 * static strings, in UTF-8
 * IDs for those strings are assigned dynamically at g_dnl_bind()
 */
static struct usb_string g_dnl_string_defs[] = {
	{.s = manufacturer},
	{.s = product},
	{.s = g_dnl_serial},
	{ }		/* end of list */
};

static struct usb_gadget_strings g_dnl_string_tab = {
	.language = 0x0409, /* en-us */
	.strings = g_dnl_string_defs,
};

static struct usb_gadget_strings *g_dnl_composite_strings[] = {
	&g_dnl_string_tab,
	NULL,
};

static int g_dnl_unbind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;

	debug("%s: calling usb_gadget_disconnect for "
			"controller '%s'\n", __func__, gadget->name);
	usb_gadget_disconnect(gadget);

	return 0;
}

static inline struct g_dnl_bind_callback *g_dnl_bind_callback_first(void)
{
	return ll_entry_start(struct g_dnl_bind_callback,
				g_dnl_bind_callbacks);
}

static inline struct g_dnl_bind_callback *g_dnl_bind_callback_end(void)
{
	return ll_entry_end(struct g_dnl_bind_callback,
				g_dnl_bind_callbacks);
}

static int g_dnl_do_config(struct usb_configuration *c)
{
	const char *s = c->cdev->driver->name;
	struct g_dnl_bind_callback *callback = g_dnl_bind_callback_first();

	debug("%s: configuration: 0x%p composite dev: 0x%p\n",
	      __func__, c, c->cdev);

	for (; callback != g_dnl_bind_callback_end(); callback++)
		if (!strcmp(s, callback->usb_function_name))
			return callback->fptr(c);
	return -ENODEV;
}

static int g_dnl_config_register(struct usb_composite_dev *cdev)
{
	struct usb_configuration *config;
	const char *name = "usb_dnload";

	config = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*config));
	if (!config)
		return -ENOMEM;

	memset(config, 0, sizeof(*config));

	config->label = name;
	config->bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
	config->bConfigurationValue = CONFIGURATION_NUMBER;
	config->iConfiguration = STRING_USBDOWN;
	config->bind = g_dnl_do_config;

	return usb_add_config(cdev, config);
}

__weak
int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

__weak
int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}

__weak
int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	return 0;
}

__weak int g_dnl_get_board_bcd_device_number(int gcnum)
{
	return gcnum;
}

__weak int g_dnl_board_usb_cable_connected(void)
{
	return -EOPNOTSUPP;
}

static bool g_dnl_detach_request;

bool g_dnl_detach(void)
{
	return g_dnl_detach_request;
}

void g_dnl_trigger_detach(void)
{
	g_dnl_detach_request = true;
}

void g_dnl_clear_detach(void)
{
	g_dnl_detach_request = false;
}

static int g_dnl_get_bcd_device_number(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	int gcnum;

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum > 0)
		gcnum += 0x200;

	return g_dnl_get_board_bcd_device_number(gcnum);
}

/**
 * Update internal serial number variable when the "serial#" env var changes.
 *
 * Handle all cases, even when flags == H_PROGRAMMATIC or op == env_op_delete.
 */
static int on_serialno(const char *name, const char *value, enum env_op op,
		int flags)
{
	g_dnl_set_serialnumber((char *)value);
	return 0;
}
U_BOOT_ENV_CALLBACK(serialno, on_serialno);

static int g_dnl_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	int id, ret;
	int gcnum;

	debug("%s: gadget: 0x%p cdev: 0x%p\n", __func__, gadget, cdev);

	id = usb_string_id(cdev);

	if (id < 0)
		return id;
	g_dnl_string_defs[0].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;

	g_dnl_string_defs[1].id = id;
	device_desc.iProduct = id;

	g_dnl_bind_fixup(&device_desc, cdev->driver->name);

	if (strlen(g_dnl_serial)) {
		id = usb_string_id(cdev);
		if (id < 0)
			return id;

		g_dnl_string_defs[2].id = id;
		device_desc.iSerialNumber = id;
	}

	ret = g_dnl_config_register(cdev);
	if (ret)
		goto error;

	gcnum = g_dnl_get_bcd_device_number(cdev);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(gcnum);
	else {
		debug("%s: controller '%s' not recognized\n",
			__func__, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	debug("%s: calling usb_gadget_connect for "
			"controller '%s'\n", __func__, gadget->name);
	usb_gadget_connect(gadget);

	return 0;

 error:
	g_dnl_unbind(cdev);
	return -ENOMEM;
}

static struct usb_composite_driver g_dnl_driver = {
	.name = NULL,
	.dev = &device_desc,
	.strings = g_dnl_composite_strings,

	.bind = g_dnl_bind,
	.unbind = g_dnl_unbind,
};

/*
 * NOTICE:
 * Registering via USB function name won't be necessary after rewriting
 * g_dnl to support multiple USB functions.
 */
int g_dnl_register(const char *name)
{
	int ret;

	debug("%s: g_dnl_driver.name = %s\n", __func__, name);
	g_dnl_driver.name = name;

	ret = usb_composite_register(&g_dnl_driver);
	if (ret) {
		printf("%s: failed!, error: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}

void g_dnl_unregister(void)
{
	usb_composite_unregister(&g_dnl_driver);
}
