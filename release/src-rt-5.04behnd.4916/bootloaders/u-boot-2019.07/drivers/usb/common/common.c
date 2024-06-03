// SPDX-License-Identifier: GPL-2.0+
/*
 * Provides code common for host and device side USB.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <linux/usb/otg.h>
#include <linux/usb/ch9.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const usb_dr_modes[] = {
	[USB_DR_MODE_UNKNOWN]		= "",
	[USB_DR_MODE_HOST]		= "host",
	[USB_DR_MODE_PERIPHERAL]	= "peripheral",
	[USB_DR_MODE_OTG]		= "otg",
};

enum usb_dr_mode usb_get_dr_mode(int node)
{
	const void *fdt = gd->fdt_blob;
	const char *dr_mode;
	int i;

	dr_mode = fdt_getprop(fdt, node, "dr_mode", NULL);
	if (!dr_mode) {
		pr_err("usb dr_mode not found\n");
		return USB_DR_MODE_UNKNOWN;
	}

	for (i = 0; i < ARRAY_SIZE(usb_dr_modes); i++)
		if (!strcmp(dr_mode, usb_dr_modes[i]))
			return i;

	return USB_DR_MODE_UNKNOWN;
}

static const char *const speed_names[] = {
	[USB_SPEED_UNKNOWN] = "UNKNOWN",
	[USB_SPEED_LOW] = "low-speed",
	[USB_SPEED_FULL] = "full-speed",
	[USB_SPEED_HIGH] = "high-speed",
	[USB_SPEED_WIRELESS] = "wireless",
	[USB_SPEED_SUPER] = "super-speed",
};

enum usb_device_speed usb_get_maximum_speed(int node)
{
	const void *fdt = gd->fdt_blob;
	const char *max_speed;
	int i;

	max_speed = fdt_getprop(fdt, node, "maximum-speed", NULL);
	if (!max_speed) {
		pr_err("usb maximum-speed not found\n");
		return USB_SPEED_UNKNOWN;
	}

	for (i = 0; i < ARRAY_SIZE(speed_names); i++)
		if (!strcmp(max_speed, speed_names[i]))
			return i;

	return USB_SPEED_UNKNOWN;
}
