/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * USB virtual root hub descriptors
 *
 * (C) Copyright 2014
 * Stephen Warren swarren@wwwdotorg.org
 *
 * Based on ohci-hcd.c
 */

#ifndef __USBROOTHUBDES_H__
#define __USBROOTHUBDES_H__

/* Device descriptor */
static __u8 root_hub_dev_des[] = {
	0x12,		/* __u8  bLength; */
	0x01,		/* __u8  bDescriptorType; Device */
	0x10,		/* __u16 bcdUSB; v1.1 */
	0x01,
	0x09,		/* __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,		/* __u8  bDeviceSubClass; */
	0x00,		/* __u8  bDeviceProtocol; */
	0x08,		/* __u8  bMaxPacketSize0; 8 Bytes */
	0x00,		/* __u16 idVendor; */
	0x00,
	0x00,		/* __u16 idProduct; */
	0x00,
	0x00,		/* __u16 bcdDevice; */
	0x00,
	0x00,		/* __u8  iManufacturer; */
	0x01,		/* __u8  iProduct; */
	0x00,		/* __u8  iSerialNumber; */
	0x01,		/* __u8  bNumConfigurations; */
};

/* Configuration descriptor */
static __u8 root_hub_config_des[] = {
	0x09,		/* __u8  bLength; */
	0x02,		/* __u8  bDescriptorType; Configuration */
	0x19,		/* __u16 wTotalLength; */
	0x00,
	0x01,		/* __u8  bNumInterfaces; */
	0x01,		/* __u8  bConfigurationValue; */
	0x00,		/* __u8  iConfiguration; */
	0x40,		/* __u8  bmAttributes;
			 *       Bit 7: Bus-powered
			 *       6: Self-powered,
			 *       5 Remote-wakwup,
			 *       4..0: resvd
			 */
	0x00,		/* __u8  MaxPower; */
	/* interface */
	0x09,		/* __u8  if_bLength; */
	0x04,		/* __u8  if_bDescriptorType; Interface */
	0x00,		/* __u8  if_bInterfaceNumber; */
	0x00,		/* __u8  if_bAlternateSetting; */
	0x01,		/* __u8  if_bNumEndpoints; */
	0x09,		/* __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,		/* __u8  if_bInterfaceSubClass; */
	0x00,		/* __u8  if_bInterfaceProtocol; */
	0x00,		/* __u8  if_iInterface; */
	/* endpoint */
	0x07,		/* __u8  ep_bLength; */
	0x05,		/* __u8  ep_bDescriptorType; Endpoint */
	0x81,		/* __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,		/* __u8  ep_bmAttributes; Interrupt */
	0x02,		/* __u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
	0x00,
	0xff,		/* __u8  ep_bInterval; 255 ms */
};

#ifdef WANT_USB_ROOT_HUB_HUB_DES
static unsigned char root_hub_hub_des[] = {
	0x09,		/* __u8  bLength; */
	0x29,		/* __u8  bDescriptorType; Hub-descriptor */
	0x02,		/* __u8  bNbrPorts; */
	0x00,		/* __u16 wHubCharacteristics; */
	0x00,
	0x01,		/* __u8  bPwrOn2pwrGood; 2ms */
	0x00,		/* __u8  bHubContrCurrent; 0 mA */
	0x00,		/* __u8  DeviceRemovable; *** 7 Ports max *** */
	0xff,		/* __u8  PortPwrCtrlMask; *** 7 ports max *** */
};
#endif

static unsigned char root_hub_str_index0[] = {
	0x04,		/* __u8  bLength; */
	0x03,		/* __u8  bDescriptorType; String-descriptor */
	0x09,		/* __u8  lang ID */
	0x04,		/* __u8  lang ID */
};

static unsigned char root_hub_str_index1[] = {
	32,		/* __u8  bLength; */
	0x03,		/* __u8  bDescriptorType; String-descriptor */
	'U',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'-',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'B',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'o',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'o',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	't',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	' ',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'R',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'o',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'o',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	't',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	' ',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'H',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'u',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
	'b',		/* __u8  Unicode */
	0,		/* __u8  Unicode */
};

#endif
