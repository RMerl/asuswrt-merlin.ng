/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 *
 * Eddie Cai <eddie.cai.linux@gmail.com>
 */

#ifndef _F_ROCKUSB_H_
#define _F_ROCKUSB_H_
#include <blk.h>

#define ROCKUSB_VERSION		"0.1"

#define ROCKUSB_INTERFACE_CLASS	0xff
#define ROCKUSB_INTERFACE_SUB_CLASS	0x06
#define ROCKUSB_INTERFACE_PROTOCOL	0x05

#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0  0x0200
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1  0x0040
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE      0x0040

#define EP_BUFFER_SIZE			4096
/*
 * EP_BUFFER_SIZE must always be an integral multiple of maxpacket size
 * (64 or 512 or 1024), else we break on certain controllers like DWC3
 * that expect bulk OUT requests to be divisible by maxpacket size.
 */

#define RKUSB_BUF_SIZE		EP_BUFFER_SIZE * 2
#define RKBLOCK_BUF_SIZE		4096

#define RKUSB_STATUS_IDLE			0
#define RKUSB_STATUS_CMD			1
#define RKUSB_STATUS_RXDATA			2
#define RKUSB_STATUS_TXDATA			3
#define RKUSB_STATUS_CSW			4
#define RKUSB_STATUS_RXDATA_PREPARE		5
#define RKUSB_STATUS_TXDATA_PREPARE		6

enum rkusb_command {
K_FW_TEST_UNIT_READY	= 0x00,
K_FW_READ_FLASH_ID = 0x01,
K_FW_SET_DEVICE_ID = 0x02,
K_FW_TEST_BAD_BLOCK = 0x03,
K_FW_READ_10 = 0x04,
K_FW_WRITE_10 = 0x05,
K_FW_ERASE_10 = 0x06,
K_FW_WRITE_SPARE = 0x07,
K_FW_READ_SPARE = 0x08,

K_FW_ERASE_10_FORCE = 0x0b,
K_FW_GET_VERSION = 0x0c,

K_FW_LBA_READ_10 = 0x14,
K_FW_LBA_WRITE_10 = 0x15,
K_FW_ERASE_SYS_DISK = 0x16,
K_FW_SDRAM_READ_10 = 0x17,
K_FW_SDRAM_WRITE_10 = 0x18,
K_FW_SDRAM_EXECUTE = 0x19,
K_FW_READ_FLASH_INFO = 0x1A,
K_FW_GET_CHIP_VER = 0x1B,
K_FW_LOW_FORMAT = 0x1C,
K_FW_SET_RESET_FLAG = 0x1E,
K_FW_SPI_READ_10 = 0x21,
K_FW_SPI_WRITE_10 = 0x22,
K_FW_LBA_ERASE_10 = 0x25,

K_FW_SESSION = 0X30,
K_FW_RESET = 0xff,
};

#define CBW_DIRECTION_OUT		0x00
#define CBW_DIRECTION_IN		0x80

struct cmd_dispatch_info {
	enum rkusb_command cmd;
	/* call back function to handle rockusb command */
	void (*cb)(struct usb_ep *ep, struct usb_request *req);
};

/* Bulk-only data structures */

/* Command Block Wrapper */
struct fsg_bulk_cb_wrap {
	__le32  signature;              /* Contains 'USBC' */
	u32     tag;                    /* Unique per command id */
	__le32  data_transfer_length;   /* Size of the data */
	u8      flags;                  /* Direction in bit 7 */
	u8      lun;                    /* lun (normally 0) */
	u8      length;                 /* Of the CDB, <= MAX_COMMAND_SIZE */
	u8      CDB[16];                /* Command Data Block */
};

#define USB_BULK_CB_WRAP_LEN    31
#define USB_BULK_CB_SIG         0x43425355      /* Spells out USBC */
#define USB_BULK_IN_FLAG        0x80

/* Command status Wrapper */
struct bulk_cs_wrap {
	__le32  signature;              /* Should = 'USBS' */
	u32     tag;                    /* Same as original command */
	__le32  residue;                /* Amount not transferred */
	u8      status;                 /* See below */
};

#define USB_BULK_CS_WRAP_LEN    13
#define USB_BULK_CS_SIG         0x53425355      /* Spells out 'USBS' */
#define USB_STATUS_PASS         0
#define USB_STATUS_FAIL         1
#define USB_STATUS_PHASE_ERROR  2

#define CSW_GOOD                0x00
#define CSW_FAIL                0x01

struct f_rockusb {
	struct usb_function usb_function;
	struct usb_ep *in_ep, *out_ep;
	struct usb_request *in_req, *out_req;
	char *dev_type;
	unsigned int dev_index;
	unsigned int tag;
	unsigned int lba;
	unsigned int dl_size;
	unsigned int dl_bytes;
	unsigned int ul_size;
	unsigned int ul_bytes;
	struct blk_desc *desc;
	int reboot_flag;
	void *buf;
	void *buf_head;
};

/* init rockusb device, tell rockusb which device you want to read/write*/
void rockusb_dev_init(char *dev_type, int dev_index);
#endif /* _F_ROCKUSB_H_ */

