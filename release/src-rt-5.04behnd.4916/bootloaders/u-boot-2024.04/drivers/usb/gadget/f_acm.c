// SPDX-License-Identifier: GPL-2.0+
/*
 * USB CDC serial (ACM) function driver
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 * Copyright (C) 2009 by Samsung Electronics
 * Copyright (c) 2021, Linaro Ltd <loic.poulain@linaro.org>
 */

#include <circbuf.h>
#include <common.h>
#include <console.h>
#include <errno.h>
#include <g_dnl.h>
#include <malloc.h>
#include <memalign.h>
#include <stdio_dev.h>
#include <version.h>
#include <watchdog.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>

#define REQ_SIZE_MAX	512

struct f_acm {
	int ctrl_id;
	int data_id;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;
	struct usb_ep *ep_notify;

	struct usb_request *req_in;
	struct usb_request *req_out;

	bool connected;
	bool tx_on;

	circbuf_t rx_buf;
	circbuf_t tx_buf;

	struct usb_function usb_function;

	struct usb_cdc_line_coding line_coding;
	u16 handshake_bits;
#define ACM_CTRL_RTS	BIT(1)	/* unused with full duplex */
#define ACM_CTRL_DTR	BIT(0)	/* host is ready for data r/w */

	struct udevice *udc;
};

static struct f_acm *default_acm_function;

static inline struct f_acm *func_to_acm(struct usb_function *f)
{
	return container_of(f, struct f_acm, usb_function);
}

static inline struct f_acm *stdio_to_acm(struct stdio_dev *s)
{
	/* stdio dev is cloned on registration, do not use container_of */
	return s->priv;
}

static struct usb_interface_assoc_descriptor
acm_iad_descriptor = {
	.bLength =              sizeof(acm_iad_descriptor),
	.bDescriptorType =      USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface =      0,
	.bInterfaceCount =      2,	// control + data
	.bFunctionClass =       USB_CLASS_COMM,
	.bFunctionSubClass =    USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol =    USB_CDC_ACM_PROTO_AT_V25TER,
};

static struct usb_interface_descriptor acm_control_intf_desc = {
	.bLength =              USB_DT_INTERFACE_SIZE,
	.bDescriptorType =      USB_DT_INTERFACE,
	.bNumEndpoints =        1,
	.bInterfaceClass =      USB_CLASS_COMM,
	.bInterfaceSubClass =   USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol =   USB_CDC_ACM_PROTO_AT_V25TER,
};

static struct usb_interface_descriptor acm_data_intf_desc = {
	.bLength =              sizeof(acm_data_intf_desc),
	.bDescriptorType =      USB_DT_INTERFACE,
	.bNumEndpoints =        2,
	.bInterfaceClass =      USB_CLASS_CDC_DATA,
};

static struct usb_cdc_header_desc acm_header_desc = {
	.bLength =		sizeof(acm_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,
	.bcdCDC =		__constant_cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor acm_call_mgmt_desc = {
	.bLength =              sizeof(acm_call_mgmt_desc),
	.bDescriptorType =      USB_DT_CS_INTERFACE,
	.bDescriptorSubType =   USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities =       0,
	.bDataInterface =       0x01,
};

static struct usb_cdc_acm_descriptor acm_desc = {
	.bLength =		sizeof(acm_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,
	.bmCapabilities =	USB_CDC_CAP_LINE,
};

static struct usb_cdc_union_desc acm_union_desc = {
	.bLength =		sizeof(acm_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	.bMasterInterface0 =	0x00,
	.bSlaveInterface0 =	0x01,
};

static struct usb_endpoint_descriptor acm_fs_notify_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     3 | USB_DIR_IN,
	.bmAttributes =         USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =       __constant_cpu_to_le16(64),
	.bInterval =            32,
};

static struct usb_endpoint_descriptor acm_fs_in_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     USB_DIR_IN,
	.bmAttributes =         USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor acm_fs_out_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     USB_DIR_OUT,
	.bmAttributes =         USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *acm_fs_function[] = {
	(struct usb_descriptor_header *)&acm_iad_descriptor,
	(struct usb_descriptor_header *)&acm_control_intf_desc,
	(struct usb_descriptor_header *)&acm_header_desc,
	(struct usb_descriptor_header *)&acm_call_mgmt_desc,
	(struct usb_descriptor_header *)&acm_desc,
	(struct usb_descriptor_header *)&acm_union_desc,
	(struct usb_descriptor_header *)&acm_fs_notify_desc,
	(struct usb_descriptor_header *)&acm_data_intf_desc,
	(struct usb_descriptor_header *)&acm_fs_in_desc,
	(struct usb_descriptor_header *)&acm_fs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor acm_hs_notify_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bmAttributes =         USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =       __constant_cpu_to_le16(64),
	.bInterval =            11,
};

static struct usb_endpoint_descriptor acm_hs_in_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor acm_hs_out_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= __constant_cpu_to_le16(512),
};

static struct usb_descriptor_header *acm_hs_function[] = {
	(struct usb_descriptor_header *)&acm_iad_descriptor,
	(struct usb_descriptor_header *)&acm_control_intf_desc,
	(struct usb_descriptor_header *)&acm_header_desc,
	(struct usb_descriptor_header *)&acm_call_mgmt_desc,
	(struct usb_descriptor_header *)&acm_desc,
	(struct usb_descriptor_header *)&acm_union_desc,
	(struct usb_descriptor_header *)&acm_hs_notify_desc,
	(struct usb_descriptor_header *)&acm_data_intf_desc,
	(struct usb_descriptor_header *)&acm_hs_in_desc,
	(struct usb_descriptor_header *)&acm_hs_out_desc,
	NULL,
};

static inline struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
	struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

static int acm_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_gadget *gadget = c->cdev->gadget;
	struct f_acm *f_acm = func_to_acm(f);
	struct usb_ep *ep;
	int id;

	id = usb_interface_id(c, f);
	if (id < 0)
		return id;

	acm_iad_descriptor.bFirstInterface = id;
	acm_control_intf_desc.bInterfaceNumber = id;
	acm_union_desc.bMasterInterface0 = id;

	f_acm->ctrl_id = id;

	id = usb_interface_id(c, f);
	if (id < 0)
		return id;

	acm_data_intf_desc.bInterfaceNumber = id;
	acm_union_desc.bSlaveInterface0 = id;
	acm_call_mgmt_desc.bDataInterface = id;

	f_acm->data_id = id;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(gadget, &acm_fs_in_desc);
	if (!ep)
		return -ENODEV;

	f_acm->ep_in = ep;

	ep = usb_ep_autoconfig(gadget, &acm_fs_out_desc);
	if (!ep)
		return -ENODEV;

	f_acm->ep_out = ep;

	ep = usb_ep_autoconfig(gadget, &acm_fs_notify_desc);
	if (!ep)
		return -ENODEV;

	f_acm->ep_notify = ep;

	if (gadget_is_dualspeed(gadget)) {
		/* Assume endpoint addresses are the same for both speeds */
		acm_hs_in_desc.bEndpointAddress = acm_fs_in_desc.bEndpointAddress;
		acm_hs_out_desc.bEndpointAddress = acm_fs_out_desc.bEndpointAddress;
		acm_hs_notify_desc.bEndpointAddress = acm_fs_notify_desc.bEndpointAddress;
	}

	return 0;
}

static void acm_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_acm *f_acm = func_to_acm(f);

	if (default_acm_function == f_acm)
		default_acm_function = NULL;

	buf_free(&f_acm->rx_buf);
	buf_free(&f_acm->tx_buf);

	free(f_acm);
}

static void acm_notify_complete(struct usb_ep *ep, struct usb_request *req)
{
	/* nothing to do */
}

static void acm_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_acm *f_acm = req->context;

	f_acm->tx_on = true;
}

static void acm_rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_acm *f_acm = req->context;

	buf_push(&f_acm->rx_buf, req->buf, req->actual);

	/* Queue RX req again */
	req->actual = 0;
	usb_ep_queue(ep, req, 0);
}

static struct usb_request *acm_start_ep(struct usb_ep *ep, void *complete_cb,
					void *context)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, 0);
	if (!req)
		return NULL;

	req->length = REQ_SIZE_MAX;
	req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, REQ_SIZE_MAX);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	memset(req->buf, 0, req->length);
	req->complete = complete_cb;
	req->context = context;

	return req;
}

static int acm_start_data(struct f_acm *f_acm, struct usb_gadget *gadget)
{
	const struct usb_endpoint_descriptor *d;
	int ret;

	/* EP IN */
	d = ep_desc(gadget, &acm_hs_in_desc, &acm_fs_in_desc);
	ret = usb_ep_enable(f_acm->ep_in, d);
	if (ret)
		return ret;

	f_acm->req_in = acm_start_ep(f_acm->ep_in, acm_tx_complete, f_acm);

	/* EP OUT */
	d = ep_desc(gadget, &acm_hs_out_desc, &acm_fs_out_desc);
	ret = usb_ep_enable(f_acm->ep_out, d);
	if (ret)
		return ret;

	f_acm->req_out = acm_start_ep(f_acm->ep_out, acm_rx_complete, f_acm);

	/* Start OUT transfer (EP OUT) */
	ret = usb_ep_queue(f_acm->ep_out, f_acm->req_out, 0);
	if (ret)
		return ret;

	return 0;
}

static int acm_start_ctrl(struct f_acm *f_acm, struct usb_gadget *gadget)
{
	const struct usb_endpoint_descriptor *d;

	usb_ep_disable(f_acm->ep_notify);

	d = ep_desc(gadget, &acm_hs_notify_desc, &acm_fs_notify_desc);
	usb_ep_enable(f_acm->ep_notify, d);

	acm_start_ep(f_acm->ep_notify, acm_notify_complete, f_acm);

	return 0;
}

static int acm_set_alt(struct usb_function *f, unsigned int intf, unsigned int alt)
{
	struct usb_gadget *gadget = f->config->cdev->gadget;
	struct f_acm *f_acm = func_to_acm(f);

	if (intf == f_acm->ctrl_id) {
		return acm_start_ctrl(f_acm, gadget);
	} else if (intf == f_acm->data_id) {
		acm_start_data(f_acm, gadget);
		f_acm->connected = true;
		f_acm->tx_on = true;
		return 0;
	}

	return -EINVAL;
}

static int acm_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct usb_gadget *gadget =  f->config->cdev->gadget;
	struct usb_request *req = f->config->cdev->req;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);
	struct f_acm *f_acm = func_to_acm(f);
	int value = -1;

	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_SET_LINE_CODING:
		/* SET_LINE_CODING */

		if (w_length != sizeof(f_acm->line_coding) || w_index != f_acm->ctrl_id)
			goto invalid;

		value = w_length;

		memcpy(&f_acm->line_coding, req->buf, value);

		break;
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_GET_LINE_CODING:
		/* GET_LINE_CODING */

		if (w_length != sizeof(f_acm->line_coding) || w_index != f_acm->ctrl_id)
			goto invalid;

		value = w_length;

		memcpy(req->buf, &f_acm->line_coding, value);

		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		/* SET_CONTROL_LINE_STATE */

		if (w_index != f_acm->ctrl_id)
			goto invalid;

		value = 0;

		f_acm->handshake_bits = w_value;

		break;
	default:
invalid:
		printf("invalid control req%02x.%02x v%04x i%04x l%d\n",
		       ctrl->bRequestType, ctrl->bRequest, w_value, w_index,
		       w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		req->zero = 0;
		req->length = value;
		usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
	}

	return 0;
}

static void acm_disable(struct usb_function *f)
{
	struct f_acm *f_acm = func_to_acm(f);

	usb_ep_disable(f_acm->ep_out);
	usb_ep_disable(f_acm->ep_in);
	usb_ep_disable(f_acm->ep_notify);

	if (f_acm->req_out) {
		free(f_acm->req_out->buf);
		usb_ep_free_request(f_acm->ep_out, f_acm->req_out);
		f_acm->req_out = NULL;
	}

	if (f_acm->req_in) {
		free(f_acm->req_in->buf);
		usb_ep_free_request(f_acm->ep_in, f_acm->req_in);
		f_acm->req_in = NULL;
	}
}

/* static strings, in UTF-8 */
static struct usb_string acm_string_defs[] = {
	[0].s = "CDC Abstract Control Model (ACM)",
	[1].s = "CDC ACM Data",
	[2].s = "CDC Serial",
	{  } /* end of list */
};

static struct usb_gadget_strings acm_string_table = {
	.language = 0x0409, /* en-us */
	.strings = acm_string_defs,
};

static struct usb_gadget_strings *acm_strings[] = {
	&acm_string_table,
	NULL,
};

static void __acm_tx(struct f_acm *f_acm)
{
	int len, ret;

	do {
		dm_usb_gadget_handle_interrupts(f_acm->udc);

		if (!(f_acm->handshake_bits & ACM_CTRL_DTR))
			break;

		if (!f_acm->tx_on)
			continue;

		len = buf_pop(&f_acm->tx_buf, f_acm->req_in->buf, REQ_SIZE_MAX);
		if (!len)
			break;

		f_acm->req_in->length = len;

		ret = usb_ep_queue(f_acm->ep_in, f_acm->req_in, 0);
		if (ret)
			break;

		f_acm->tx_on = false;

		/* Do not reset the watchdog, if TX is stuck there is probably
		 * a real issue.
		 */
	} while (1);
}

static bool acm_connected(struct stdio_dev *dev)
{
	struct f_acm *f_acm = stdio_to_acm(dev);

	/* give a chance to process udc irq */
	dm_usb_gadget_handle_interrupts(f_acm->udc);

	return f_acm->connected;
}

static int acm_add(struct usb_configuration *c)
{
	struct f_acm *f_acm;
	int status;

	f_acm = calloc(1, sizeof(*f_acm));
	if (!f_acm)
		return -ENOMEM;

	f_acm->usb_function.name = "f_acm";
	f_acm->usb_function.bind = acm_bind;
	f_acm->usb_function.unbind = acm_unbind;
	f_acm->usb_function.set_alt = acm_set_alt;
	f_acm->usb_function.disable = acm_disable;
	f_acm->usb_function.strings = acm_strings;
	f_acm->usb_function.descriptors = acm_fs_function;
	f_acm->usb_function.hs_descriptors = acm_hs_function;
	f_acm->usb_function.setup = acm_setup;

	status = udc_device_get_by_index(0, &f_acm->udc);
	if (status)
		return status;

	status = usb_add_function(c, &f_acm->usb_function);
	if (status) {
		free(f_acm);
		return status;
	}

	buf_init(&f_acm->rx_buf, 2048);
	buf_init(&f_acm->tx_buf, 2048);

	if (!default_acm_function)
		default_acm_function = f_acm;

	return status;
}

DECLARE_GADGET_BIND_CALLBACK(usb_serial_acm, acm_add);

/* STDIO */
static int acm_stdio_tstc(struct stdio_dev *dev)
{
	struct f_acm *f_acm = stdio_to_acm(dev);

	dm_usb_gadget_handle_interrupts(f_acm->udc);

	return (f_acm->rx_buf.size > 0);
}

static int acm_stdio_getc(struct stdio_dev *dev)
{
	struct f_acm *f_acm = stdio_to_acm(dev);
	char c;

	/* Wait for a character to arrive. */
	while (!acm_stdio_tstc(dev))
		schedule();

	buf_pop(&f_acm->rx_buf, &c, 1);

	return c;
}

static void acm_stdio_putc(struct stdio_dev *dev, const char c)
{
	struct f_acm *f_acm = stdio_to_acm(dev);

	if (c == '\n')
		buf_push(&f_acm->tx_buf, "\r", 1);

	buf_push(&f_acm->tx_buf, &c, 1);

	if (!f_acm->connected)
		return;

	__acm_tx(f_acm);
}

static void acm_stdio_puts(struct stdio_dev *dev, const char *str)
{
	struct f_acm *f_acm = stdio_to_acm(dev);

	while (*str) {
		if (*str == '\n')
			buf_push(&f_acm->tx_buf, "\r", 1);

		buf_push(&f_acm->tx_buf, str++, 1);
	}

	if (!f_acm->connected)
		return;

	__acm_tx(f_acm);
}

static int acm_stdio_start(struct stdio_dev *dev)
{
	int ret;

	if (dev->priv) { /* function already exist */
		return 0;
	}

	ret = g_dnl_register("usb_serial_acm");
	if (ret)
		return ret;

	if (default_acm_function)
		dev->priv = default_acm_function;
	else
		return -ENODEV;

	while (!acm_connected(dev)) {
		if (ctrlc())
			return -ECANCELED;

		schedule();
	}

	return 0;
}

static int acm_stdio_stop(struct stdio_dev *dev)
{
	g_dnl_unregister();
	g_dnl_clear_detach();

	return 0;
}

int drv_usbacm_init(void)
{
	struct stdio_dev stdio;

	strcpy(stdio.name, "usbacm");
	stdio.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT;
	stdio.tstc = acm_stdio_tstc;
	stdio.getc = acm_stdio_getc;
	stdio.putc = acm_stdio_putc;
	stdio.puts = acm_stdio_puts;
	stdio.start = acm_stdio_start;
	stdio.stop = acm_stdio_stop;
	stdio.priv = NULL;
	stdio.ext = 0;

	return stdio_register(&stdio);
}
