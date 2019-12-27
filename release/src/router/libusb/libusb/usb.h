/*
 * Prototypes, structure definitions and macros.
 *
 * Copyright (c) 2000-2003 Johannes Erdfelt <johannes@erdfelt.com>
 * Copyright (c) 2015      Nathan Hjelm <hjelmn@cs.unm.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * This file (and only this file) may alternatively be licensed under the
 * BSD license. See the LICENSE file shipped with the libusb-compat-0.1 source
 * distribution for details.
 */

#ifndef USB_H
#define USB_H

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <dirent.h>

#ifdef interface
#undef interface
#endif

/* stdint.h is not available on older MSVC */
#if defined(_MSC_VER) && (_MSC_VER < 1600) && (!defined(_STDINT)) && (!defined(_STDINT_H))
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
#else
#include <stdint.h>
#endif

/*
 * USB spec information
 *
 * This is all stuff grabbed from various USB specs and is pretty much
 * not subject to change
 */

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PRINTER		7
#define USB_CLASS_PTP			6
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_DATA			10
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * Descriptor types
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05

#define USB_DT_HID			0x21
#define USB_DT_REPORT			0x22
#define USB_DT_PHYSICAL			0x23
#define USB_DT_HUB			0x29

/*
 * Descriptor sizes per descriptor type
 */
#define USB_DT_DEVICE_SIZE		18
#define USB_DT_CONFIG_SIZE		9
#define USB_DT_INTERFACE_SIZE		9
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE		7

/* All standard descriptors have these 2 fields in common */
struct usb_descriptor_header {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
};

/* String descriptor */
struct usb_string_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wData[1];
};

/* HID descriptor */
struct usb_hid_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdHID;
	uint8_t  bCountryCode;
	uint8_t  bNumDescriptors;
	/* uint8_t  bReportDescriptorType; */
	/* uint16_t wDescriptorLength; */
	/* ... */
};

/* Endpoint descriptor */
#define USB_MAXENDPOINTS	32
struct usb_endpoint_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
	uint8_t  bRefresh;
	uint8_t  bSynchAddress;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

#define USB_ENDPOINT_ADDRESS_MASK	0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_TYPE_MASK		0x03    /* in bmAttributes */
#define USB_ENDPOINT_TYPE_CONTROL	0
#define USB_ENDPOINT_TYPE_ISOCHRONOUS	1
#define USB_ENDPOINT_TYPE_BULK		2
#define USB_ENDPOINT_TYPE_INTERRUPT	3

/* Interface descriptor */
#define USB_MAXINTERFACES	32
struct usb_interface_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  bInterfaceProtocol;
	uint8_t  iInterface;

	struct usb_endpoint_descriptor *endpoint;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

#define USB_MAXALTSETTING	128	/* Hard limit */
struct usb_interface {
	struct usb_interface_descriptor *altsetting;

	int num_altsetting;
};

/* Configuration descriptor information.. */
#define USB_MAXCONFIG		8
struct usb_config_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  MaxPower;

	struct usb_interface *interface;

	unsigned char *extra;	/* Extra descriptors */
	int extralen;
};

/* Device descriptor */
struct usb_device_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
};

struct usb_ctrl_setup {
	uint8_t  bRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
/* 0x02 is reserved */
#define USB_REQ_SET_FEATURE		0x03
/* 0x04 is reserved */
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

/*
 * Various libusb API related stuff
 */

#define USB_ENDPOINT_IN			0x80
#define USB_ENDPOINT_OUT		0x00

/* Error codes */
#define USB_ERROR_BEGIN			500000

/* Data types */
struct usb_device;
struct usb_bus;

/*
 * To maintain compatibility with applications already built with libusb,
 * we must only add entries to the end of this structure. NEVER delete or
 * move members and only change types if you really know what you're doing.
 */
struct usb_device {
  struct usb_device *next, *prev;

  char filename[PATH_MAX + 1];

  struct usb_bus *bus;

  struct usb_device_descriptor descriptor;
  struct usb_config_descriptor *config;

  void *dev;		/* Darwin support */

  uint8_t devnum;

  unsigned char num_children;
  struct usb_device **children;
};

struct usb_bus {
  struct usb_bus *next, *prev;

  char dirname[PATH_MAX + 1];

  struct usb_device *devices;
  uint32_t location;

  struct usb_device *root_dev;
};

struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;

/* Variables */
extern struct usb_bus *usb_busses;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */

/* usb.c */
usb_dev_handle *usb_open(struct usb_device *dev);
int usb_close(usb_dev_handle *dev);
int usb_get_string(usb_dev_handle *dev, int index, int langid, char *buf,
	size_t buflen);
int usb_get_string_simple(usb_dev_handle *dev, int index, char *buf,
	size_t buflen);

/* descriptors.c */
int usb_get_descriptor_by_endpoint(usb_dev_handle *udev, int ep,
	unsigned char type, unsigned char index, void *buf, int size);
int usb_get_descriptor(usb_dev_handle *udev, unsigned char type,
	unsigned char index, void *buf, int size);

/* <arch>.c */
int usb_bulk_write(usb_dev_handle *dev, int ep, const char *bytes, int size,
	int timeout);
int usb_bulk_write_sp(usb_dev_handle *dev, int ep, const char *bytes, int size,
	int timeout, int *actual_length, int max_rw);
int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
	int timeout);
int usb_interrupt_write(usb_dev_handle *dev, int ep, const char *bytes,
	int size, int timeout);
int usb_interrupt_read(usb_dev_handle *dev, int ep, char *bytes, int size,
        int timeout);
int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
	int value, int index, char *bytes, int size, int timeout);
int usb_set_configuration(usb_dev_handle *dev, int configuration);
int usb_claim_interface(usb_dev_handle *dev, int interface);
int usb_release_interface(usb_dev_handle *dev, int interface);
int usb_set_altinterface(usb_dev_handle *dev, int alternate);
int usb_resetep(usb_dev_handle *dev, unsigned int ep);
int usb_clear_halt(usb_dev_handle *dev, unsigned int ep);
int usb_reset(usb_dev_handle *dev);

#define LIBUSB_HAS_GET_DRIVER_NP 1
int usb_get_driver_np(usb_dev_handle *dev, int interface, char *name,
	unsigned int namelen);
#define LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP 1
int usb_detach_kernel_driver_np(usb_dev_handle *dev, int interface);

char *usb_strerror(void);

void usb_init(void);
void usb_set_debug(int level);
int usb_find_busses(void);
int usb_find_devices(void);
struct usb_device *usb_device(usb_dev_handle *dev);
struct usb_bus *usb_get_busses(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */

