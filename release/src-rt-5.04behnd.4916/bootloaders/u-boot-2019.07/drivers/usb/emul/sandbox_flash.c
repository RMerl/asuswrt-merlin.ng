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
 * This driver emulates a flash stick using the UFI command specification and
 * the BBB (bulk/bulk/bulk) protocol. It supports only a single logical unit
 * number (LUN 0).
 */

enum {
	SANDBOX_FLASH_EP_OUT		= 1,	/* endpoints */
	SANDBOX_FLASH_EP_IN		= 2,
	SANDBOX_FLASH_BLOCK_LEN		= 512,
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
 * struct sandbox_flash_priv - private state for this driver
 *
 * @error:	true if there is an error condition
 * @alloc_len:	Allocation length from the last incoming command
 * @transfer_len: Transfer length from CBW header
 * @read_len:	Number of blocks of data left in the current read command
 * @tag:	Tag value from last command
 * @fd:		File descriptor of backing file
 * @file_size:	Size of file in bytes
 * @status_buff:	Data buffer for outgoing status
 * @buff_used:	Number of bytes ready to transfer back to host
 * @buff:	Data buffer for outgoing data
 */
struct sandbox_flash_priv {
	bool error;
	int alloc_len;
	int transfer_len;
	int read_len;
	enum cmd_phase phase;
	u32 tag;
	int fd;
	loff_t file_size;
	struct umass_bbb_csw status;
	int buff_used;
	u8 buff[512];
};

struct sandbox_flash_plat {
	const char *pathname;
	struct usb_string flash_strings[STRINGID_COUNT];
};

struct scsi_inquiry_resp {
	u8 type;
	u8 flags;
	u8 version;
	u8 data_format;
	u8 additional_len;
	u8 spare[3];
	char vendor[8];
	char product[16];
	char revision[4];
};

struct scsi_read_capacity_resp {
	u32 last_block_addr;
	u32 block_len;
};

struct __packed scsi_read10_req {
	u8 cmd;
	u8 lun_flags;
	u32 lba;
	u8 spare;
	u16 transfer_len;
	u8 spare2[3];
};

static struct usb_device_descriptor flash_device_desc = {
	.bLength =		sizeof(flash_device_desc),
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),

	.bDeviceClass =		0,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,

	.idVendor =		__constant_cpu_to_le16(0x1234),
	.idProduct =		__constant_cpu_to_le16(0x5678),
	.iManufacturer =	STRINGID_MANUFACTURER,
	.iProduct =		STRINGID_PRODUCT,
	.iSerialNumber =	STRINGID_SERIAL,
	.bNumConfigurations =	1,
};

static struct usb_config_descriptor flash_config0 = {
	.bLength		= sizeof(flash_config0),
	.bDescriptorType	= USB_DT_CONFIG,

	/* wTotalLength is set up by usb-emul-uclass */
	.bNumInterfaces		= 1,
	.bConfigurationValue	= 0,
	.iConfiguration		= 0,
	.bmAttributes		= 1 << 7,
	.bMaxPower		= 50,
};

static struct usb_interface_descriptor flash_interface0 = {
	.bLength		= sizeof(flash_interface0),
	.bDescriptorType	= USB_DT_INTERFACE,

	.bInterfaceNumber	= 0,
	.bAlternateSetting	= 0,
	.bNumEndpoints		= 2,
	.bInterfaceClass	= USB_CLASS_MASS_STORAGE,
	.bInterfaceSubClass	= US_SC_UFI,
	.bInterfaceProtocol	= US_PR_BULK,
	.iInterface		= 0,
};

static struct usb_endpoint_descriptor flash_endpoint0_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,

	.bEndpointAddress	= SANDBOX_FLASH_EP_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(1024),
	.bInterval		= 0,
};

static struct usb_endpoint_descriptor flash_endpoint1_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,

	.bEndpointAddress	= SANDBOX_FLASH_EP_IN | USB_ENDPOINT_DIR_MASK,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(1024),
	.bInterval		= 0,
};

static void *flash_desc_list[] = {
	&flash_device_desc,
	&flash_config0,
	&flash_interface0,
	&flash_endpoint0_out,
	&flash_endpoint1_in,
	NULL,
};

static int sandbox_flash_control(struct udevice *dev, struct usb_device *udev,
				 unsigned long pipe, void *buff, int len,
				 struct devrequest *setup)
{
	struct sandbox_flash_priv *priv = dev_get_priv(dev);

	if (pipe == usb_rcvctrlpipe(udev, 0)) {
		switch (setup->request) {
		case US_BBB_RESET:
			priv->error = false;
			return 0;
		case US_BBB_GET_MAX_LUN:
			*(char *)buff = '\0';
			return 1;
		default:
			debug("request=%x\n", setup->request);
			break;
		}
	}
	debug("pipe=%lx\n", pipe);

	return -EIO;
}

static void setup_fail_response(struct sandbox_flash_priv *priv)
{
	struct umass_bbb_csw *csw = &priv->status;

	csw->dCSWSignature = CSWSIGNATURE;
	csw->dCSWTag = priv->tag;
	csw->dCSWDataResidue = 0;
	csw->bCSWStatus = CSWSTATUS_FAILED;
	priv->buff_used = 0;
}

/**
 * setup_response() - set up a response to send back to the host
 *
 * @priv:	Sandbox flash private data
 * @resp:	Response to send, or NULL if none
 * @size:	Size of response
 */
static void setup_response(struct sandbox_flash_priv *priv, void *resp,
			   int size)
{
	struct umass_bbb_csw *csw = &priv->status;

	csw->dCSWSignature = CSWSIGNATURE;
	csw->dCSWTag = priv->tag;
	csw->dCSWDataResidue = 0;
	csw->bCSWStatus = CSWSTATUS_GOOD;

	assert(!resp || resp == priv->buff);
	priv->buff_used = size;
}

static void handle_read(struct sandbox_flash_priv *priv, ulong lba,
			ulong transfer_len)
{
	debug("%s: lba=%lx, transfer_len=%lx\n", __func__, lba, transfer_len);
	if (priv->fd != -1) {
		os_lseek(priv->fd, lba * SANDBOX_FLASH_BLOCK_LEN, OS_SEEK_SET);
		priv->read_len = transfer_len;
		setup_response(priv, priv->buff,
			       transfer_len * SANDBOX_FLASH_BLOCK_LEN);
	} else {
		setup_fail_response(priv);
	}
}

static int handle_ufi_command(struct sandbox_flash_plat *plat,
			      struct sandbox_flash_priv *priv, const void *buff,
			      int len)
{
	const struct scsi_cmd *req = buff;

	switch (*req->cmd) {
	case SCSI_INQUIRY: {
		struct scsi_inquiry_resp *resp = (void *)priv->buff;

		priv->alloc_len = req->cmd[4];
		memset(resp, '\0', sizeof(*resp));
		resp->data_format = 1;
		resp->additional_len = 0x1f;
		strncpy(resp->vendor,
			plat->flash_strings[STRINGID_MANUFACTURER -  1].s,
			sizeof(resp->vendor));
		strncpy(resp->product,
			plat->flash_strings[STRINGID_PRODUCT - 1].s,
			sizeof(resp->product));
		strncpy(resp->revision, "1.0", sizeof(resp->revision));
		setup_response(priv, resp, sizeof(*resp));
		break;
	}
	case SCSI_TST_U_RDY:
		setup_response(priv, NULL, 0);
		break;
	case SCSI_RD_CAPAC: {
		struct scsi_read_capacity_resp *resp = (void *)priv->buff;
		uint blocks;

		if (priv->file_size)
			blocks = priv->file_size / SANDBOX_FLASH_BLOCK_LEN - 1;
		else
			blocks = 0;
		resp->last_block_addr = cpu_to_be32(blocks);
		resp->block_len = cpu_to_be32(SANDBOX_FLASH_BLOCK_LEN);
		setup_response(priv, resp, sizeof(*resp));
		break;
	}
	case SCSI_READ10: {
		struct scsi_read10_req *req = (void *)buff;

		handle_read(priv, be32_to_cpu(req->lba),
			    be16_to_cpu(req->transfer_len));
		break;
	}
	default:
		debug("Command not supported: %x\n", req->cmd[0]);
		return -EPROTONOSUPPORT;
	}

	priv->phase = priv->transfer_len ? PHASE_DATA : PHASE_STATUS;
	return 0;
}

static int sandbox_flash_bulk(struct udevice *dev, struct usb_device *udev,
			      unsigned long pipe, void *buff, int len)
{
	struct sandbox_flash_plat *plat = dev_get_platdata(dev);
	struct sandbox_flash_priv *priv = dev_get_priv(dev);
	int ep = usb_pipeendpoint(pipe);
	struct umass_bbb_cbw *cbw = buff;

	debug("%s: dev=%s, pipe=%lx, ep=%x, len=%x, phase=%d\n", __func__,
	      dev->name, pipe, ep, len, priv->phase);
	switch (ep) {
	case SANDBOX_FLASH_EP_OUT:
		switch (priv->phase) {
		case PHASE_START:
			priv->alloc_len = 0;
			priv->read_len = 0;
			if (priv->error || len != UMASS_BBB_CBW_SIZE ||
			    cbw->dCBWSignature != CBWSIGNATURE)
				goto err;
			if ((cbw->bCBWFlags & CBWFLAGS_SBZ) ||
			    cbw->bCBWLUN != 0)
				goto err;
			if (cbw->bCDBLength < 1 || cbw->bCDBLength >= 0x10)
				goto err;
			priv->transfer_len = cbw->dCBWDataTransferLength;
			priv->tag = cbw->dCBWTag;
			return handle_ufi_command(plat, priv, cbw->CBWCDB,
						  cbw->bCDBLength);
		case PHASE_DATA:
			debug("data out\n");
			break;
		default:
			break;
		}
	case SANDBOX_FLASH_EP_IN:
		switch (priv->phase) {
		case PHASE_DATA:
			debug("data in, len=%x, alloc_len=%x, priv->read_len=%x\n",
			      len, priv->alloc_len, priv->read_len);
			if (priv->read_len) {
				ulong bytes_read;

				bytes_read = os_read(priv->fd, buff, len);
				if (bytes_read != len)
					return -EIO;
				priv->read_len -= len / SANDBOX_FLASH_BLOCK_LEN;
				if (!priv->read_len)
					priv->phase = PHASE_STATUS;
			} else {
				if (priv->alloc_len && len > priv->alloc_len)
					len = priv->alloc_len;
				memcpy(buff, priv->buff, len);
				priv->phase = PHASE_STATUS;
			}
			return len;
		case PHASE_STATUS:
			debug("status in, len=%x\n", len);
			if (len > sizeof(priv->status))
				len = sizeof(priv->status);
			memcpy(buff, &priv->status, len);
			priv->phase = PHASE_START;
			return len;
		default:
			break;
		}
	}
err:
	priv->error = true;
	debug("%s: Detected transfer error\n", __func__);
	return 0;
}

static int sandbox_flash_ofdata_to_platdata(struct udevice *dev)
{
	struct sandbox_flash_plat *plat = dev_get_platdata(dev);

	plat->pathname = dev_read_string(dev, "sandbox,filepath");

	return 0;
}

static int sandbox_flash_bind(struct udevice *dev)
{
	struct sandbox_flash_plat *plat = dev_get_platdata(dev);
	struct usb_string *fs;

	fs = plat->flash_strings;
	fs[0].id = STRINGID_MANUFACTURER;
	fs[0].s = "sandbox";
	fs[1].id = STRINGID_PRODUCT;
	fs[1].s = "flash";
	fs[2].id = STRINGID_SERIAL;
	fs[2].s = dev->name;

	return usb_emul_setup_device(dev, plat->flash_strings, flash_desc_list);
}

static int sandbox_flash_probe(struct udevice *dev)
{
	struct sandbox_flash_plat *plat = dev_get_platdata(dev);
	struct sandbox_flash_priv *priv = dev_get_priv(dev);

	priv->fd = os_open(plat->pathname, OS_O_RDONLY);
	if (priv->fd != -1)
		return os_get_filesize(plat->pathname, &priv->file_size);

	return 0;
}

static const struct dm_usb_ops sandbox_usb_flash_ops = {
	.control	= sandbox_flash_control,
	.bulk		= sandbox_flash_bulk,
};

static const struct udevice_id sandbox_usb_flash_ids[] = {
	{ .compatible = "sandbox,usb-flash" },
	{ }
};

U_BOOT_DRIVER(usb_sandbox_flash) = {
	.name	= "usb_sandbox_flash",
	.id	= UCLASS_USB_EMUL,
	.of_match = sandbox_usb_flash_ids,
	.bind	= sandbox_flash_bind,
	.probe	= sandbox_flash_probe,
	.ofdata_to_platdata = sandbox_flash_ofdata_to_platdata,
	.ops	= &sandbox_usb_flash_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_flash_priv),
	.platdata_auto_alloc_size = sizeof(struct sandbox_flash_plat),
};
