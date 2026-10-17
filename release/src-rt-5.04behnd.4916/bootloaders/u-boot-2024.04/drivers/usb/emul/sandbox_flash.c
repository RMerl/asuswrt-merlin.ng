// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	UCLASS_USB

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <os.h>
#include <scsi.h>
#include <scsi_emul.h>
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
	SANDBOX_FLASH_BUF_SIZE		= 512,
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
 * @eminfo:	emulator state
 * @error:	true if there is an error condition
 * @tag:	Tag value from last command
 * @fd:		File descriptor of backing file
 * @file_size:	Size of file in bytes
 * @status_buff:	Data buffer for outgoing status
 */
struct sandbox_flash_priv {
	struct scsi_emul_info eminfo;
	bool error;
	u32 tag;
	int fd;
	struct umass_bbb_csw status;
};

struct sandbox_flash_plat {
	const char *pathname;
	struct usb_string flash_strings[STRINGID_COUNT];
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
}

/**
 * setup_response() - set up a response to send back to the host
 *
 * @priv:	Sandbox flash private data
 * @resp:	Response to send, or NULL if none
 * @size:	Size of response
 */
static void setup_response(struct sandbox_flash_priv *priv)
{
	struct umass_bbb_csw *csw = &priv->status;

	csw->dCSWSignature = CSWSIGNATURE;
	csw->dCSWTag = priv->tag;
	csw->dCSWDataResidue = 0;
	csw->bCSWStatus = CSWSTATUS_GOOD;
}

static int handle_ufi_command(struct sandbox_flash_priv *priv, const void *buff,
			      int len)
{
	struct scsi_emul_info *info = &priv->eminfo;
	const struct scsi_cmd *req = buff;
	int ret;
	off_t offset;

	ret = sb_scsi_emul_command(info, req, len);
	if (!ret) {
		setup_response(priv);
	} else if ((ret == SCSI_EMUL_DO_READ || ret == SCSI_EMUL_DO_WRITE) &&
		   priv->fd != -1) {
		offset = os_lseek(priv->fd, info->seek_block * info->block_size,
				  OS_SEEK_SET);
		if (offset == (off_t)-1)
			setup_fail_response(priv);
		else
			setup_response(priv);
	} else {
		setup_fail_response(priv);
	}

	return 0;
}

static int sandbox_flash_bulk(struct udevice *dev, struct usb_device *udev,
			      unsigned long pipe, void *buff, int len)
{
	struct sandbox_flash_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;
	int ep = usb_pipeendpoint(pipe);
	struct umass_bbb_cbw *cbw = buff;

	debug("%s: dev=%s, pipe=%lx, ep=%x, len=%x, phase=%d\n", __func__,
	      dev->name, pipe, ep, len, info->phase);
	switch (ep) {
	case SANDBOX_FLASH_EP_OUT:
		switch (info->phase) {
		case SCSIPH_START:
			info->alloc_len = 0;
			info->read_len = 0;
			info->write_len = 0;
			if (priv->error || len != UMASS_BBB_CBW_SIZE ||
			    cbw->dCBWSignature != CBWSIGNATURE)
				goto err;
			if ((cbw->bCBWFlags & CBWFLAGS_SBZ) ||
			    cbw->bCBWLUN != 0)
				goto err;
			if (cbw->bCDBLength < 1 || cbw->bCDBLength >= 0x10)
				goto err;
			info->transfer_len = cbw->dCBWDataTransferLength;
			priv->tag = cbw->dCBWTag;
			return handle_ufi_command(priv, cbw->CBWCDB,
						  cbw->bCDBLength);
		case SCSIPH_DATA:
			log_debug("data out, len=%x, info->write_len=%x\n", len,
				  info->write_len);
			info->transfer_len = cbw->dCBWDataTransferLength;
			priv->tag = cbw->dCBWTag;
			if (!info->write_len)
				return 0;
			if (priv->fd != -1) {
				ulong bytes_written;

				bytes_written = os_write(priv->fd, buff, len);
				log_debug("bytes_written=%lx", bytes_written);
				if (bytes_written != len)
					return -EIO;
				info->write_len -= len / info->block_size;
				if (!info->write_len)
					info->phase = SCSIPH_STATUS;
			} else {
				if (info->alloc_len && len > info->alloc_len)
					len = info->alloc_len;
				if (len > SANDBOX_FLASH_BUF_SIZE)
					len = SANDBOX_FLASH_BUF_SIZE;
				memcpy(info->buff, buff, len);
				info->phase = SCSIPH_STATUS;
			}
			return len;
		default:
			break;
		}
		break;
	case SANDBOX_FLASH_EP_IN:
		switch (info->phase) {
		case SCSIPH_DATA:
			debug("data in, len=%x, alloc_len=%x, info->read_len=%x\n",
			      len, info->alloc_len, info->read_len);
			if (info->read_len) {
				ulong bytes_read;

				if (priv->fd == -1)
					return -EIO;

				bytes_read = os_read(priv->fd, buff, len);
				if (bytes_read != len)
					return -EIO;
				info->read_len -= len / info->block_size;
				if (!info->read_len)
					info->phase = SCSIPH_STATUS;
			} else {
				if (info->alloc_len && len > info->alloc_len)
					len = info->alloc_len;
				if (len > SANDBOX_FLASH_BUF_SIZE)
					len = SANDBOX_FLASH_BUF_SIZE;
				memcpy(buff, info->buff, len);
				info->phase = SCSIPH_STATUS;
			}
			return len;
		case SCSIPH_STATUS:
			debug("status in, len=%x\n", len);
			if (len > sizeof(priv->status))
				len = sizeof(priv->status);
			memcpy(buff, &priv->status, len);
			info->phase = SCSIPH_START;
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

static int sandbox_flash_of_to_plat(struct udevice *dev)
{
	struct sandbox_flash_plat *plat = dev_get_plat(dev);

	plat->pathname = dev_read_string(dev, "sandbox,filepath");

	return 0;
}

static int sandbox_flash_bind(struct udevice *dev)
{
	struct sandbox_flash_plat *plat = dev_get_plat(dev);
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
	struct sandbox_flash_plat *plat = dev_get_plat(dev);
	struct sandbox_flash_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;
	int ret;

	priv->fd = os_open(plat->pathname, OS_O_RDWR);
	if (priv->fd != -1) {
		ret = os_get_filesize(plat->pathname, &info->file_size);
		if (ret)
			return log_msg_ret("sz", ret);
	}
	info->buff = malloc(SANDBOX_FLASH_BUF_SIZE);
	if (!info->buff)
		return log_ret(-ENOMEM);
	info->vendor = plat->flash_strings[STRINGID_MANUFACTURER -  1].s;
	info->product = plat->flash_strings[STRINGID_PRODUCT - 1].s;
	info->block_size = SANDBOX_FLASH_BLOCK_LEN;

	return 0;
}

static int sandbox_flash_remove(struct udevice *dev)
{
	struct sandbox_flash_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;

	free(info->buff);

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
	.remove	= sandbox_flash_remove,
	.of_to_plat = sandbox_flash_of_to_plat,
	.ops	= &sandbox_usb_flash_ops,
	.priv_auto	= sizeof(struct sandbox_flash_priv),
	.plat_auto	= sizeof(struct sandbox_flash_plat),
};
