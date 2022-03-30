// SPDX-License-Identifier: GPL-2.0+
/*
 * f_thor.c -- USB TIZEN THOR Downloader gadget function
 *
 * Copyright (C) 2013 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * Based on code from:
 * git://review.tizen.org/kernel/u-boot
 *
 * Developed by:
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Sanghee Kim <sh0130.kim@samsung.com>
 */

#include <errno.h>
#include <common.h>
#include <console.h>
#include <malloc.h>
#include <memalign.h>
#include <version.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>
#include <g_dnl.h>
#include <dfu.h>

#include "f_thor.h"

static void thor_tx_data(unsigned char *data, int len);
static void thor_set_dma(void *addr, int len);
static int thor_rx_data(void);

static struct f_thor *thor_func;
static inline struct f_thor *func_to_thor(struct usb_function *f)
{
	return container_of(f, struct f_thor, usb_function);
}

DEFINE_CACHE_ALIGN_BUFFER(unsigned char, thor_tx_data_buf,
			  sizeof(struct rsp_box));
DEFINE_CACHE_ALIGN_BUFFER(unsigned char, thor_rx_data_buf,
			  sizeof(struct rqt_box));

/* ********************************************************** */
/*         THOR protocol - transmission handling	      */
/* ********************************************************** */
DEFINE_CACHE_ALIGN_BUFFER(char, f_name, F_NAME_BUF_SIZE + 1);
static unsigned long long int thor_file_size;
static int alt_setting_num;

static void send_rsp(const struct rsp_box *rsp)
{
	memcpy(thor_tx_data_buf, rsp, sizeof(struct rsp_box));
	thor_tx_data(thor_tx_data_buf, sizeof(struct rsp_box));

	debug("-RSP: %d, %d\n", rsp->rsp, rsp->rsp_data);
}

static void send_data_rsp(s32 ack, s32 count)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct data_rsp_box, rsp,
				 sizeof(struct data_rsp_box));

	rsp->ack = ack;
	rsp->count = count;

	memcpy(thor_tx_data_buf, rsp, sizeof(struct data_rsp_box));
	thor_tx_data(thor_tx_data_buf, sizeof(struct data_rsp_box));

	debug("-DATA RSP: %d, %d\n", ack, count);
}

static int process_rqt_info(const struct rqt_box *rqt)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct rsp_box, rsp, sizeof(struct rsp_box));
	memset(rsp, 0, sizeof(struct rsp_box));

	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_INFO_VER_PROTOCOL:
		rsp->int_data[0] = VER_PROTOCOL_MAJOR;
		rsp->int_data[1] = VER_PROTOCOL_MINOR;
		break;
	case RQT_INIT_VER_HW:
		snprintf(rsp->str_data[0], sizeof(rsp->str_data[0]),
			 "%x", checkboard());
		break;
	case RQT_INIT_VER_BOOT:
		sprintf(rsp->str_data[0], "%s", U_BOOT_VERSION);
		break;
	case RQT_INIT_VER_KERNEL:
		sprintf(rsp->str_data[0], "%s", "k unknown");
		break;
	case RQT_INIT_VER_PLATFORM:
		sprintf(rsp->str_data[0], "%s", "p unknown");
		break;
	case RQT_INIT_VER_CSC:
		sprintf(rsp->str_data[0], "%s", "c unknown");
		break;
	default:
		return -EINVAL;
	}

	send_rsp(rsp);
	return true;
}

static int process_rqt_cmd(const struct rqt_box *rqt)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct rsp_box, rsp, sizeof(struct rsp_box));
	memset(rsp, 0, sizeof(struct rsp_box));

	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_CMD_REBOOT:
		debug("TARGET RESET\n");
		send_rsp(rsp);
		g_dnl_unregister();
		dfu_free_entities();
#ifdef CONFIG_THOR_RESET_OFF
		return RESET_DONE;
#endif
		run_command("reset", 0);
		break;
	case RQT_CMD_POWEROFF:
	case RQT_CMD_EFSCLEAR:
		send_rsp(rsp);
	default:
		printf("Command not supported -> cmd: %d\n", rqt->rqt_data);
		return -EINVAL;
	}

	return true;
}

static long long int download_head(unsigned long long total,
				   unsigned int packet_size,
				   long long int *left,
				   int *cnt)
{
	long long int rcv_cnt = 0, left_to_rcv, ret_rcv;
	struct dfu_entity *dfu_entity = dfu_get_entity(alt_setting_num);
	void *transfer_buffer = dfu_get_buf(dfu_entity);
	void *buf = transfer_buffer;
	int usb_pkt_cnt = 0, ret;

	/*
	 * Files smaller than THOR_STORE_UNIT_SIZE (now 32 MiB) are stored on
	 * the medium.
	 * The packet response is sent on the purpose after successful data
	 * chunk write. There is a room for improvement when asynchronous write
	 * is performed.
	 */
	while (total - rcv_cnt >= packet_size) {
		thor_set_dma(buf, packet_size);
		buf += packet_size;
		ret_rcv = thor_rx_data();
		if (ret_rcv < 0)
			return ret_rcv;
		rcv_cnt += ret_rcv;
		debug("%d: RCV data count: %llu cnt: %d\n", usb_pkt_cnt,
		      rcv_cnt, *cnt);

		if ((rcv_cnt % THOR_STORE_UNIT_SIZE) == 0) {
			ret = dfu_write(dfu_get_entity(alt_setting_num),
					transfer_buffer, THOR_STORE_UNIT_SIZE,
					(*cnt)++);
			if (ret) {
				pr_err("DFU write failed [%d] cnt: %d",
				      ret, *cnt);
				return ret;
			}
			buf = transfer_buffer;
		}
		send_data_rsp(0, ++usb_pkt_cnt);
	}

	/* Calculate the amount of data to arrive from PC (in bytes) */
	left_to_rcv = total - rcv_cnt;

	/*
	 * Calculate number of data already received. but not yet stored
	 * on the medium (they are smaller than THOR_STORE_UNIT_SIZE)
	 */
	*left = left_to_rcv + buf - transfer_buffer;
	debug("%s: left: %llu left_to_rcv: %llu buf: 0x%p\n", __func__,
	      *left, left_to_rcv, buf);

	if (left_to_rcv) {
		thor_set_dma(buf, packet_size);
		ret_rcv = thor_rx_data();
		if (ret_rcv < 0)
			return ret_rcv;
		rcv_cnt += ret_rcv;
		send_data_rsp(0, ++usb_pkt_cnt);
	}

	debug("%s: %llu total: %llu cnt: %d\n", __func__, rcv_cnt, total, *cnt);

	return rcv_cnt;
}

static int download_tail(long long int left, int cnt)
{
	struct dfu_entity *dfu_entity;
	void *transfer_buffer;
	int ret;

	debug("%s: left: %llu cnt: %d\n", __func__, left, cnt);

	dfu_entity = dfu_get_entity(alt_setting_num);
	if (!dfu_entity) {
		pr_err("Alt setting: %d entity not found!\n", alt_setting_num);
		return -ENOENT;
	}

	transfer_buffer = dfu_get_buf(dfu_entity);
	if (!transfer_buffer) {
		pr_err("Transfer buffer not allocated!");
		return -ENXIO;
	}

	if (left) {
		ret = dfu_write(dfu_entity, transfer_buffer, left, cnt++);
		if (ret) {
			pr_err("DFU write failed [%d]: left: %llu", ret, left);
			return ret;
		}
	}

	/*
	 * To store last "packet" or write file from buffer to filesystem
	 * DFU storage backend requires dfu_flush
	 *
	 * This also frees memory malloc'ed by dfu_get_buf(), so no explicit
	 * need fo call dfu_free_buf() is needed.
	 */
	ret = dfu_flush(dfu_entity, transfer_buffer, 0, cnt);
	if (ret)
		pr_err("DFU flush failed!");

	return ret;
}

static long long int process_rqt_download(const struct rqt_box *rqt)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct rsp_box, rsp, sizeof(struct rsp_box));
	static long long int left, ret_head;
	int file_type, ret = 0;
	static int cnt;

	memset(rsp, 0, sizeof(struct rsp_box));
	rsp->rsp = rqt->rqt;
	rsp->rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_DL_INIT:
		thor_file_size = (unsigned long long int)rqt->int_data[0] +
				 (((unsigned long long int)rqt->int_data[1])
				  << 32);
		debug("INIT: total %llu bytes\n", thor_file_size);
		break;
	case RQT_DL_FILE_INFO:
		file_type = rqt->int_data[0];
		if (file_type == FILE_TYPE_PIT) {
			puts("PIT table file - not supported\n");
			rsp->ack = -ENOTSUPP;
			ret = rsp->ack;
			break;
		}

		thor_file_size = (unsigned long long int)rqt->int_data[1] +
				 (((unsigned long long int)rqt->int_data[2])
				  << 32);
		memcpy(f_name, rqt->str_data[0], F_NAME_BUF_SIZE);
		f_name[F_NAME_BUF_SIZE] = '\0';

		debug("INFO: name(%s, %d), size(%llu), type(%d)\n",
		      f_name, 0, thor_file_size, file_type);

		rsp->int_data[0] = THOR_PACKET_SIZE;

		alt_setting_num = dfu_get_alt(f_name);
		if (alt_setting_num < 0) {
			pr_err("Alt setting [%d] to write not found!",
			      alt_setting_num);
			rsp->ack = -ENODEV;
			ret = rsp->ack;
		}
		break;
	case RQT_DL_FILE_START:
		send_rsp(rsp);
		ret_head = download_head(thor_file_size, THOR_PACKET_SIZE,
					 &left, &cnt);
		if (ret_head < 0) {
			left = 0;
			cnt = 0;
		}
		return ret_head;
	case RQT_DL_FILE_END:
		debug("DL FILE_END\n");
		rsp->ack = download_tail(left, cnt);
		ret = rsp->ack;
		left = 0;
		cnt = 0;
		break;
	case RQT_DL_EXIT:
		debug("DL EXIT\n");
		break;
	default:
		pr_err("Operation not supported: %d", rqt->rqt_data);
		ret = -ENOTSUPP;
	}

	send_rsp(rsp);
	return ret;
}

static int process_data(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct rqt_box, rqt, sizeof(struct rqt_box));
	int ret = -EINVAL;

	memcpy(rqt, thor_rx_data_buf, sizeof(struct rqt_box));

	debug("+RQT: %d, %d\n", rqt->rqt, rqt->rqt_data);

	switch (rqt->rqt) {
	case RQT_INFO:
		ret = process_rqt_info(rqt);
		break;
	case RQT_CMD:
		ret = process_rqt_cmd(rqt);
		break;
	case RQT_DL:
		ret = (int) process_rqt_download(rqt);
		break;
	case RQT_UL:
		puts("RQT: UPLOAD not supported!\n");
		break;
	default:
		pr_err("unknown request (%d)", rqt->rqt);
	}

	return ret;
}

/* ********************************************************** */
/*         THOR USB Function				      */
/* ********************************************************** */

static inline struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
	struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

static struct usb_interface_descriptor thor_downloader_intf_data = {
	.bLength =		sizeof(thor_downloader_intf_data),
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
};

static struct usb_endpoint_descriptor fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =	USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =	USB_ENDPOINT_XFER_BULK,
};

/* CDC configuration */
static struct usb_interface_descriptor thor_downloader_intf_int = {
	.bLength =		sizeof(thor_downloader_intf_int),
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	 /* 0x02 Abstract Line Control Model */
	.bInterfaceSubClass =   USB_CDC_SUBCLASS_ACM,
	/* 0x01 Common AT commands */
	.bInterfaceProtocol =   USB_CDC_ACM_PROTO_AT_V25TER,
};

static struct usb_cdc_header_desc thor_downloader_cdc_header = {
	.bLength         =    sizeof(thor_downloader_cdc_header),
	.bDescriptorType =    0x24, /* CS_INTERFACE */
	.bDescriptorSubType = 0x00,
	.bcdCDC =             0x0110,
};

static struct usb_cdc_call_mgmt_descriptor thor_downloader_cdc_call = {
	.bLength         =    sizeof(thor_downloader_cdc_call),
	.bDescriptorType =    0x24, /* CS_INTERFACE */
	.bDescriptorSubType = 0x01,
	.bmCapabilities =     0x00,
	.bDataInterface =     0x01,
};

static struct usb_cdc_acm_descriptor thor_downloader_cdc_abstract = {
	.bLength         =    sizeof(thor_downloader_cdc_abstract),
	.bDescriptorType =    0x24, /* CS_INTERFACE */
	.bDescriptorSubType = 0x02,
	.bmCapabilities =     0x00,
};

static struct usb_cdc_union_desc thor_downloader_cdc_union = {
	.bLength         =     sizeof(thor_downloader_cdc_union),
	.bDescriptorType =     0x24, /* CS_INTERFACE */
	.bDescriptorSubType =  USB_CDC_UNION_TYPE,
};

static struct usb_endpoint_descriptor fs_int_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = 3 | USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(16),

	.bInterval = 0x9,
};

static struct usb_interface_assoc_descriptor
thor_iad_descriptor = {
	.bLength =		sizeof(thor_iad_descriptor),
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,

	.bFirstInterface =	0,
	.bInterfaceCount =	2,	/* control + data */
	.bFunctionClass =	USB_CLASS_COMM,
	.bFunctionSubClass =	USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol =	USB_CDC_PROTO_NONE,
};

static struct usb_endpoint_descriptor hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_int_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(16),

	.bInterval = 0x9,
};

/*
 * This attribute vendor descriptor is necessary for correct operation with
 * Windows version of THOR download program
 *
 * It prevents windows driver from sending zero lenght packet (ZLP) after
 * each THOR_PACKET_SIZE. This assures consistent behaviour with libusb
 */
static struct usb_cdc_attribute_vendor_descriptor thor_downloader_cdc_av = {
	.bLength =              sizeof(thor_downloader_cdc_av),
	.bDescriptorType =      0x24,
	.bDescriptorSubType =   0x80,
	.DAUType =              0x0002,
	.DAULength =            0x0001,
	.DAUValue =             0x00,
};

static const struct usb_descriptor_header *hs_thor_downloader_function[] = {
	(struct usb_descriptor_header *)&thor_iad_descriptor,

	(struct usb_descriptor_header *)&thor_downloader_intf_int,
	(struct usb_descriptor_header *)&thor_downloader_cdc_header,
	(struct usb_descriptor_header *)&thor_downloader_cdc_call,
	(struct usb_descriptor_header *)&thor_downloader_cdc_abstract,
	(struct usb_descriptor_header *)&thor_downloader_cdc_union,
	(struct usb_descriptor_header *)&hs_int_desc,

	(struct usb_descriptor_header *)&thor_downloader_intf_data,
	(struct usb_descriptor_header *)&thor_downloader_cdc_av,
	(struct usb_descriptor_header *)&hs_in_desc,
	(struct usb_descriptor_header *)&hs_out_desc,
	NULL,
};

/*-------------------------------------------------------------------------*/
static struct usb_request *alloc_ep_req(struct usb_ep *ep, unsigned length)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, 0);
	if (!req)
		return req;

	req->length = length;
	req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, length);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		req = NULL;
	}

	return req;
}

static int thor_rx_data(void)
{
	struct thor_dev *dev = thor_func->dev;
	int data_to_rx, tmp, status;

	data_to_rx = dev->out_req->length;
	tmp = data_to_rx;
	do {
		dev->out_req->length = data_to_rx;
		debug("dev->out_req->length:%d dev->rxdata:%d\n",
		      dev->out_req->length, dev->rxdata);

		status = usb_ep_queue(dev->out_ep, dev->out_req, 0);
		if (status) {
			pr_err("kill %s:  resubmit %d bytes --> %d",
			      dev->out_ep->name, dev->out_req->length, status);
			usb_ep_set_halt(dev->out_ep);
			return -EAGAIN;
		}

		while (!dev->rxdata) {
			usb_gadget_handle_interrupts(0);
			if (ctrlc())
				return -1;
		}
		dev->rxdata = 0;
		data_to_rx -= dev->out_req->actual;
	} while (data_to_rx);

	return tmp;
}

static void thor_tx_data(unsigned char *data, int len)
{
	struct thor_dev *dev = thor_func->dev;
	unsigned char *ptr = dev->in_req->buf;
	int status;

	memset(ptr, 0, len);
	memcpy(ptr, data, len);

	dev->in_req->length = len;

	debug("%s: dev->in_req->length:%d to_cpy:%zd\n", __func__,
	      dev->in_req->length, sizeof(data));

	status = usb_ep_queue(dev->in_ep, dev->in_req, 0);
	if (status) {
		pr_err("kill %s:  resubmit %d bytes --> %d",
		      dev->in_ep->name, dev->in_req->length, status);
		usb_ep_set_halt(dev->in_ep);
	}

	/* Wait until tx interrupt received */
	while (!dev->txdata)
		usb_gadget_handle_interrupts(0);

	dev->txdata = 0;
}

static void thor_rx_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct thor_dev *dev = thor_func->dev;
	int status = req->status;

	debug("%s: ep_ptr:%p, req_ptr:%p\n", __func__, ep, req);
	switch (status) {
	case 0:
		if (ep == dev->out_ep)
			dev->rxdata = 1;
		else
			dev->txdata = 1;

		break;

	/* this endpoint is normally active while we're configured */
	case -ECONNABORTED:		/* hardware forced ep reset */
	case -ECONNRESET:		/* request dequeued */
	case -ESHUTDOWN:		/* disconnect from host */
	case -EREMOTEIO:                /* short read */
	case -EOVERFLOW:
		pr_err("ERROR:%d", status);
		break;
	}

	debug("%s complete --> %d, %d/%d\n", ep->name,
	      status, req->actual, req->length);
}

static void thor_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		debug("setup complete --> %d, %d/%d\n",
		      req->status, req->actual, req->length);
}

static int
thor_func_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct thor_dev *dev = thor_func->dev;
	struct usb_request *req = dev->req;
	struct usb_gadget *gadget = dev->gadget;
	int value = 0;

	u16 len = le16_to_cpu(ctrl->wLength);

	debug("Req_Type: 0x%x Req: 0x%x wValue: 0x%x wIndex: 0x%x wLen: 0x%x\n",
	      ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex,
	      ctrl->wLength);

	switch (ctrl->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		value = 0;
		break;
	case USB_CDC_REQ_SET_LINE_CODING:
		value = len;
		/* Line Coding set done = configuration done */
		thor_func->dev->configuration_done = 1;
		break;

	default:
		pr_err("thor_setup: unknown request: %d", ctrl->bRequest);
	}

	if (value >= 0) {
		req->length = value;
		req->zero = value < len;
		value = usb_ep_queue(gadget->ep0, req, 0);
		if (value < 0) {
			debug("%s: ep_queue: %d\n", __func__, value);
			req->status = 0;
		}
	}

	return value;
}

/* Specific to the THOR protocol */
static void thor_set_dma(void *addr, int len)
{
	struct thor_dev *dev = thor_func->dev;

	debug("in_req:%p, out_req:%p\n", dev->in_req, dev->out_req);
	debug("addr:%p, len:%d\n", addr, len);

	dev->out_req->buf = addr;
	dev->out_req->length = len;
}

int thor_init(void)
{
	struct thor_dev *dev = thor_func->dev;

	/* Wait for a device enumeration and configuration settings */
	debug("THOR enumeration/configuration setting....\n");
	while (!dev->configuration_done)
		usb_gadget_handle_interrupts(0);

	thor_set_dma(thor_rx_data_buf, strlen("THOR"));
	/* detect the download request from Host PC */
	if (thor_rx_data() < 0) {
		printf("%s: Data not received!\n", __func__);
		return -1;
	}

	if (!strncmp((char *)thor_rx_data_buf, "THOR", strlen("THOR"))) {
		puts("Download request from the Host PC\n");
		udelay(30 * 1000); /* 30 ms */

		strcpy((char *)thor_tx_data_buf, "ROHT");
		thor_tx_data(thor_tx_data_buf, strlen("ROHT"));
	} else {
		puts("Wrong reply information\n");
		return -1;
	}

	return 0;
}

int thor_handle(void)
{
	int ret;

	/* receive the data from Host PC */
	while (1) {
		thor_set_dma(thor_rx_data_buf, sizeof(struct rqt_box));
		ret = thor_rx_data();

		if (ret > 0) {
			ret = process_data();
#ifdef CONFIG_THOR_RESET_OFF
			if (ret == RESET_DONE)
				break;
#endif
			if (ret < 0)
				return ret;
		} else {
			printf("%s: No data received!\n", __func__);
			break;
		}
	}

	return 0;
}

static void free_ep_req(struct usb_ep *ep, struct usb_request *req)
{
	if (req->buf)
		free(req->buf);
	usb_ep_free_request(ep, req);
}

static int thor_func_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_gadget *gadget = c->cdev->gadget;
	struct f_thor *f_thor = func_to_thor(f);
	struct thor_dev *dev;
	struct usb_ep *ep;
	int status;

	thor_func = f_thor;
	dev = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	memset(dev, 0, sizeof(*dev));
	dev->gadget = gadget;
	f_thor->dev = dev;

	debug("%s: usb_configuration: 0x%p usb_function: 0x%p\n",
	      __func__, c, f);
	debug("f_thor: 0x%p thor: 0x%p\n", f_thor, dev);

	/* EP0  */
	/* preallocate control response and buffer */
	dev->req = usb_ep_alloc_request(gadget->ep0, 0);
	if (!dev->req) {
		status = -ENOMEM;
		goto fail;
	}
	dev->req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE,
				 THOR_PACKET_SIZE);
	if (!dev->req->buf) {
		status = -ENOMEM;
		goto fail;
	}

	dev->req->complete = thor_setup_complete;

	/* DYNAMIC interface numbers assignments */
	status = usb_interface_id(c, f);

	if (status < 0)
		goto fail;

	thor_downloader_intf_int.bInterfaceNumber = status;
	thor_downloader_cdc_union.bMasterInterface0 = status;

	status = usb_interface_id(c, f);

	if (status < 0)
		goto fail;

	thor_downloader_intf_data.bInterfaceNumber = status;
	thor_downloader_cdc_union.bSlaveInterface0 = status;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(gadget, &fs_in_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	if (gadget_is_dualspeed(gadget)) {
		hs_in_desc.bEndpointAddress =
				fs_in_desc.bEndpointAddress;
	}

	dev->in_ep = ep; /* Store IN EP for enabling @ setup */
	ep->driver_data = dev;

	ep = usb_ep_autoconfig(gadget, &fs_out_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	if (gadget_is_dualspeed(gadget))
		hs_out_desc.bEndpointAddress =
				fs_out_desc.bEndpointAddress;

	dev->out_ep = ep; /* Store OUT EP for enabling @ setup */
	ep->driver_data = dev;

	ep = usb_ep_autoconfig(gadget, &fs_int_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	dev->int_ep = ep;
	ep->driver_data = dev;

	if (gadget_is_dualspeed(gadget)) {
		hs_int_desc.bEndpointAddress =
				fs_int_desc.bEndpointAddress;

		f->hs_descriptors = (struct usb_descriptor_header **)
			&hs_thor_downloader_function;

		if (!f->hs_descriptors)
			goto fail;
	}

	debug("%s: out_ep:%p out_req:%p\n", __func__,
	      dev->out_ep, dev->out_req);

	return 0;

 fail:
	if (dev->req)
		free_ep_req(gadget->ep0, dev->req);
	free(dev);
	return status;
}

static void thor_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_thor *f_thor = func_to_thor(f);
	struct thor_dev *dev = f_thor->dev;

	free_ep_req(dev->gadget->ep0, dev->req);
	free(dev);
	memset(thor_func, 0, sizeof(*thor_func));
	thor_func = NULL;
}

static void thor_func_disable(struct usb_function *f)
{
	struct f_thor *f_thor = func_to_thor(f);
	struct thor_dev *dev = f_thor->dev;

	debug("%s:\n", __func__);

	/* Avoid freeing memory when ep is still claimed */
	if (dev->in_ep->driver_data) {
		usb_ep_disable(dev->in_ep);
		free_ep_req(dev->in_ep, dev->in_req);
		dev->in_ep->driver_data = NULL;
	}

	if (dev->out_ep->driver_data) {
		usb_ep_disable(dev->out_ep);
		usb_ep_free_request(dev->out_ep, dev->out_req);
		dev->out_ep->driver_data = NULL;
	}

	if (dev->int_ep->driver_data) {
		usb_ep_disable(dev->int_ep);
		dev->int_ep->driver_data = NULL;
	}
}

static int thor_eps_setup(struct usb_function *f)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_gadget *gadget = cdev->gadget;
	struct thor_dev *dev = thor_func->dev;
	struct usb_endpoint_descriptor *d;
	struct usb_request *req;
	struct usb_ep *ep;
	int result;

	ep = dev->in_ep;
	d = ep_desc(gadget, &hs_in_desc, &fs_in_desc);
	debug("(d)bEndpointAddress: 0x%x\n", d->bEndpointAddress);

	result = usb_ep_enable(ep, d);
	if (result)
		goto err;

	ep->driver_data = cdev; /* claim */
	req = alloc_ep_req(ep, THOR_PACKET_SIZE);
	if (!req) {
		result = -EIO;
		goto err_disable_in_ep;
	}

	memset(req->buf, 0, req->length);
	req->complete = thor_rx_tx_complete;
	dev->in_req = req;
	ep = dev->out_ep;
	d = ep_desc(gadget, &hs_out_desc, &fs_out_desc);
	debug("(d)bEndpointAddress: 0x%x\n", d->bEndpointAddress);

	result = usb_ep_enable(ep, d);
	if (result)
		goto err_free_in_req;

	ep->driver_data = cdev; /* claim */
	req = usb_ep_alloc_request(ep, 0);
	if (!req) {
		result = -EIO;
		goto err_disable_out_ep;
	}

	req->complete = thor_rx_tx_complete;
	dev->out_req = req;
	/* ACM control EP */
	ep = dev->int_ep;
	ep->driver_data = cdev;	/* claim */

	return 0;

 err_disable_out_ep:
	usb_ep_disable(dev->out_ep);

 err_free_in_req:
	free_ep_req(dev->in_ep, dev->in_req);
	dev->in_req = NULL;

 err_disable_in_ep:
	usb_ep_disable(dev->in_ep);

 err:
	return result;
}

static int thor_func_set_alt(struct usb_function *f,
			     unsigned intf, unsigned alt)
{
	struct thor_dev *dev = thor_func->dev;
	int result;

	debug("%s: func: %s intf: %d alt: %d\n",
	      __func__, f->name, intf, alt);

	switch (intf) {
	case 0:
		debug("ACM INTR interface\n");
		break;
	case 1:
		debug("Communication Data interface\n");
		result = thor_eps_setup(f);
		if (result)
			pr_err("%s: EPs setup failed!", __func__);
		dev->configuration_done = 1;
		break;
	}

	return 0;
}

static int thor_func_init(struct usb_configuration *c)
{
	struct f_thor *f_thor;
	int status;

	debug("%s: cdev: 0x%p\n", __func__, c->cdev);

	f_thor = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*f_thor));
	if (!f_thor)
		return -ENOMEM;

	memset(f_thor, 0, sizeof(*f_thor));

	f_thor->usb_function.name = "f_thor";
	f_thor->usb_function.bind = thor_func_bind;
	f_thor->usb_function.unbind = thor_unbind;
	f_thor->usb_function.setup = thor_func_setup;
	f_thor->usb_function.set_alt = thor_func_set_alt;
	f_thor->usb_function.disable = thor_func_disable;

	status = usb_add_function(c, &f_thor->usb_function);
	if (status)
		free(f_thor);

	return status;
}

int thor_add(struct usb_configuration *c)
{
	debug("%s:\n", __func__);
	return thor_func_init(c);
}

DECLARE_GADGET_BIND_CALLBACK(usb_dnl_thor, thor_add);
