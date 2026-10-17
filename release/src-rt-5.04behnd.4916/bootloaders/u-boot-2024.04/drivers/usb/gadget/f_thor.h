/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * f_thor.h - USB TIZEN THOR - internal gadget definitions
 *
 * Copyright (C) 2013 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 */

#ifndef _USB_THOR_H_
#define _USB_THOR_H_

#include <linux/compiler.h>
#include <linux/sizes.h>

/* THOR Composite Gadget */
#define STRING_MANUFACTURER_IDX	0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* ********************************************************** */
/*                   THOR protocol definitions		      */
/* ********************************************************** */

/*
 * Attribute Vendor descriptor - necessary to prevent ZLP transmission
 * from Windows XP HOST PC
 */
struct usb_cdc_attribute_vendor_descriptor {
	__u8 bLength;
	__u8 bDescriptorType;
	__u8 bDescriptorSubType;
	__u16 DAUType;
	__u16 DAULength;
	__u8 DAUValue;
} __packed;

#define VER_PROTOCOL_MAJOR	5
#define VER_PROTOCOL_MINOR	0

enum rqt {
	RQT_INFO = 200,
	RQT_CMD,
	RQT_DL,
	RQT_UL,
};

enum rqt_data {
	/* RQT_INFO */
	RQT_INFO_VER_PROTOCOL = 1,
	RQT_INIT_VER_HW,
	RQT_INIT_VER_BOOT,
	RQT_INIT_VER_KERNEL,
	RQT_INIT_VER_PLATFORM,
	RQT_INIT_VER_CSC,

	/* RQT_CMD */
	RQT_CMD_REBOOT = 1,
	RQT_CMD_POWEROFF,
	RQT_CMD_EFSCLEAR,

	/* RQT_DL */
	RQT_DL_INIT = 1,
	RQT_DL_FILE_INFO,
	RQT_DL_FILE_START,
	RQT_DL_FILE_END,
	RQT_DL_EXIT,

	/* RQT_UL */
	RQT_UL_INIT = 1,
	RQT_UL_START,
	RQT_UL_END,
	RQT_UL_EXIT,
};

struct rqt_box {		/* total: 256B */
	s32 rqt;		/* request id */
	s32 rqt_data;		/* request data id */
	s32 int_data[14];	/* int data */
	char str_data[5][32];	/* string data */
	char md5[32];		/* md5 checksum */
} __packed;

struct rsp_box {		/* total: 128B */
	s32 rsp;		/* response id (= request id) */
	s32 rsp_data;		/* response data id */
	s32 ack;		/* ack */
	s32 int_data[5];	/* int data */
	char str_data[3][32];	/* string data */
} __packed;

struct data_rsp_box {		/* total: 8B */
	s32 ack;		/* response id (= request id) */
	s32 count;		/* response data id */
} __packed;

enum {
	FILE_TYPE_NORMAL,
	FILE_TYPE_PIT,
};

struct thor_dev {
	struct usb_gadget *gadget;
	struct usb_request *req; /* EP0 -> control responses */

	/* IN/OUT EP's and correspoinding requests */
	struct usb_ep *in_ep, *out_ep, *int_ep;
	struct usb_request *in_req, *out_req;

	/* Control flow variables */
	unsigned char configuration_done;
	unsigned char rxdata;
	unsigned char txdata;
};

struct f_thor {
	struct usb_function usb_function;
	struct thor_dev *dev;
};

#define F_NAME_BUF_SIZE 32
#define THOR_PACKET_SIZE SZ_1M      /* 1 MiB */
#define THOR_STORE_UNIT_SIZE SZ_32M /* 32 MiB */
#ifdef CONFIG_THOR_RESET_OFF
#define RESET_DONE 0xFFFFFFFF
#endif
#endif /* _USB_THOR_H_ */
