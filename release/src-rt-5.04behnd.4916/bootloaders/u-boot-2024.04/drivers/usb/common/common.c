// SPDX-License-Identifier: GPL-2.0+
/*
 * Provides code common for host and device side USB.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <linux/printk.h>
#include <linux/usb/otg.h>
#include <linux/usb/ch9.h>
#include <linux/usb/phy.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const usb_dr_modes[] = {
	[USB_DR_MODE_UNKNOWN]		= "",
	[USB_DR_MODE_HOST]		= "host",
	[USB_DR_MODE_PERIPHERAL]	= "peripheral",
	[USB_DR_MODE_OTG]		= "otg",
};

enum usb_dr_mode usb_get_dr_mode(ofnode node)
{
	const char *dr_mode;
	int i;

	dr_mode = ofnode_read_string(node, "dr_mode");
	if (!dr_mode) {
		pr_debug("usb dr_mode not found\n");
		return USB_DR_MODE_UNKNOWN;
	}

	for (i = 0; i < ARRAY_SIZE(usb_dr_modes); i++)
		if (!strcmp(dr_mode, usb_dr_modes[i]))
			return i;

	return USB_DR_MODE_UNKNOWN;
}

enum usb_dr_mode usb_get_role_switch_default_mode(ofnode node)
{
	const char *dr_mode;
	int i;

	dr_mode = ofnode_read_string(node, "role-switch-default-mode");
	if (!dr_mode)
		return USB_DR_MODE_UNKNOWN;

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
	[USB_SPEED_SUPER_PLUS] = "super-speed-plus",
};

const char *usb_speed_string(enum usb_device_speed speed)
{
	if (speed < 0 || speed >= ARRAY_SIZE(speed_names))
		speed = USB_SPEED_UNKNOWN;
	return speed_names[speed];
}

enum usb_device_speed usb_get_maximum_speed(ofnode node)
{
	const char *max_speed;
	int i;

	max_speed = ofnode_read_string(node, "maximum-speed");
	if (!max_speed) {
		pr_debug("usb maximum-speed not found\n");
		return USB_SPEED_UNKNOWN;
	}

	for (i = 0; i < ARRAY_SIZE(speed_names); i++)
		if (!strcmp(max_speed, speed_names[i]))
			return i;

	return USB_SPEED_UNKNOWN;
}

#if CONFIG_IS_ENABLED(DM_USB)
static const char *const usbphy_modes[] = {
	[USBPHY_INTERFACE_MODE_UNKNOWN]	= "",
	[USBPHY_INTERFACE_MODE_UTMI]	= "utmi",
	[USBPHY_INTERFACE_MODE_UTMIW]	= "utmi_wide",
	[USBPHY_INTERFACE_MODE_ULPI]	= "ulpi",
	[USBPHY_INTERFACE_MODE_SERIAL]	= "serial",
	[USBPHY_INTERFACE_MODE_HSIC]	= "hsic",
};

enum usb_phy_interface usb_get_phy_mode(ofnode node)
{
	const char *phy_type;
	int i;

	phy_type = ofnode_get_property(node, "phy_type", NULL);
	if (!phy_type)
		return USBPHY_INTERFACE_MODE_UNKNOWN;

	for (i = 0; i < ARRAY_SIZE(usbphy_modes); i++)
		if (!strcmp(phy_type, usbphy_modes[i]))
			return i;

	return USBPHY_INTERFACE_MODE_UNKNOWN;
}
#endif
