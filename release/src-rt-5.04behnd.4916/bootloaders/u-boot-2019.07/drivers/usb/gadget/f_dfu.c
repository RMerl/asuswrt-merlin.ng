// SPDX-License-Identifier: GPL-2.0+
/*
 * f_dfu.c -- Device Firmware Update USB function
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *          Lukasz Majewski <l.majewski@samsung.com>
 *
 * Based on OpenMoko u-boot: drivers/usb/usbdfu.c
 * (C) 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on existing SAM7DFU code from OpenPCD:
 * (C) Copyright 2006 by Harald Welte <hwelte at hmw-consulting.de>
 */

#include <errno.h>
#include <common.h>
#include <malloc.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>

#include <dfu.h>
#include <g_dnl.h>
#include "f_dfu.h"

struct f_dfu {
	struct usb_function		usb_function;

	struct usb_descriptor_header	**function;
	struct usb_string		*strings;

	/* when configured, we have one config */
	u8				config;
	u8				altsetting;
	enum dfu_state			dfu_state;
	unsigned int			dfu_status;

	/* Send/received block number is handy for data integrity check */
	int                             blk_seq_num;
	unsigned int                    poll_timeout;
};

struct dfu_entity *dfu_defer_flush;

typedef int (*dfu_state_fn) (struct f_dfu *,
			     const struct usb_ctrlrequest *,
			     struct usb_gadget *,
			     struct usb_request *);

static inline struct f_dfu *func_to_dfu(struct usb_function *f)
{
	return container_of(f, struct f_dfu, usb_function);
}

static const struct dfu_function_descriptor dfu_func = {
	.bLength =		sizeof dfu_func,
	.bDescriptorType =	DFU_DT_FUNC,
	.bmAttributes =		DFU_BIT_WILL_DETACH |
				DFU_BIT_MANIFESTATION_TOLERANT |
				DFU_BIT_CAN_UPLOAD |
				DFU_BIT_CAN_DNLOAD,
	.wDetachTimeOut =	0,
	.wTransferSize =	DFU_USB_BUFSIZ,
	.bcdDFUVersion =	__constant_cpu_to_le16(0x0110),
};

static struct usb_interface_descriptor dfu_intf_runtime = {
	.bLength =		sizeof dfu_intf_runtime,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_APP_SPEC,
	.bInterfaceSubClass =	1,
	.bInterfaceProtocol =	1,
	/* .iInterface = DYNAMIC */
};

static struct usb_descriptor_header *dfu_runtime_descs[] = {
	(struct usb_descriptor_header *) &dfu_intf_runtime,
	NULL,
};

static const char dfu_name[] = "Device Firmware Upgrade";

/*
 * static strings, in UTF-8
 *
 * dfu_generic configuration
 */
static struct usb_string strings_dfu_generic[] = {
	[0].s = dfu_name,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dfu_generic = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dfu_generic,
};

static struct usb_gadget_strings *dfu_generic_strings[] = {
	&stringtab_dfu_generic,
	NULL,
};

/*
 * usb_function specific
 */
static struct usb_gadget_strings stringtab_dfu = {
	.language	= 0x0409,	/* en-us */
	/*
	 * .strings
	 *
	 * assigned during initialization,
	 * depends on number of flash entities
	 *
	 */
};

static struct usb_gadget_strings *dfu_strings[] = {
	&stringtab_dfu,
	NULL,
};

static void dfu_set_poll_timeout(struct dfu_status *dstat, unsigned int ms)
{
	/*
	 * The bwPollTimeout DFU_GETSTATUS request payload provides information
	 * about minimum time, in milliseconds, that the host should wait before
	 * sending a subsequent DFU_GETSTATUS request
	 *
	 * This permits the device to vary the delay depending on its need to
	 * erase or program the memory
	 *
	 */

	unsigned char *p = (unsigned char *)&ms;

	if (!ms || (ms & ~DFU_POLL_TIMEOUT_MASK)) {
		dstat->bwPollTimeout[0] = 0;
		dstat->bwPollTimeout[1] = 0;
		dstat->bwPollTimeout[2] = 0;

		return;
	}

	dstat->bwPollTimeout[0] = *p++;
	dstat->bwPollTimeout[1] = *p++;
	dstat->bwPollTimeout[2] = *p;
}

/*-------------------------------------------------------------------------*/

static void dnload_request_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_dfu *f_dfu = req->context;
	int ret;

	ret = dfu_write(dfu_get_entity(f_dfu->altsetting), req->buf,
			req->actual, f_dfu->blk_seq_num);
	if (ret) {
		f_dfu->dfu_status = DFU_STATUS_errUNKNOWN;
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
	}
}

static void dnload_request_flush(struct usb_ep *ep, struct usb_request *req)
{
	struct f_dfu *f_dfu = req->context;
	dfu_set_defer_flush(dfu_get_entity(f_dfu->altsetting));
}

static inline int dfu_get_manifest_timeout(struct dfu_entity *dfu)
{
	return dfu->poll_timeout ? dfu->poll_timeout(dfu) :
		DFU_MANIFEST_POLL_TIMEOUT;
}

static int handle_getstatus(struct usb_request *req)
{
	struct dfu_status *dstat = (struct dfu_status *)req->buf;
	struct f_dfu *f_dfu = req->context;
	struct dfu_entity *dfu = dfu_get_entity(f_dfu->altsetting);

	dfu_set_poll_timeout(dstat, 0);

	switch (f_dfu->dfu_state) {
	case DFU_STATE_dfuDNLOAD_SYNC:
	case DFU_STATE_dfuDNBUSY:
		f_dfu->dfu_state = DFU_STATE_dfuDNLOAD_IDLE;
		break;
	case DFU_STATE_dfuMANIFEST_SYNC:
		f_dfu->dfu_state = DFU_STATE_dfuMANIFEST;
		break;
	case DFU_STATE_dfuMANIFEST:
		dfu_set_poll_timeout(dstat, dfu_get_manifest_timeout(dfu));
		break;
	default:
		break;
	}

	if (f_dfu->poll_timeout)
		if (!(f_dfu->blk_seq_num %
		      (dfu_get_buf_size() / DFU_USB_BUFSIZ)))
			dfu_set_poll_timeout(dstat, f_dfu->poll_timeout);

	/* send status response */
	dstat->bStatus = f_dfu->dfu_status;
	dstat->bState = f_dfu->dfu_state;
	dstat->iString = 0;

	return sizeof(struct dfu_status);
}

static int handle_getstate(struct usb_request *req)
{
	struct f_dfu *f_dfu = req->context;

	((u8 *)req->buf)[0] = f_dfu->dfu_state;
	return sizeof(u8);
}

static inline void to_dfu_mode(struct f_dfu *f_dfu)
{
	f_dfu->usb_function.strings = dfu_strings;
	f_dfu->usb_function.hs_descriptors = f_dfu->function;
	f_dfu->usb_function.descriptors = f_dfu->function;
	f_dfu->dfu_state = DFU_STATE_dfuIDLE;
}

static inline void to_runtime_mode(struct f_dfu *f_dfu)
{
	f_dfu->usb_function.strings = NULL;
	f_dfu->usb_function.hs_descriptors = dfu_runtime_descs;
	f_dfu->usb_function.descriptors = dfu_runtime_descs;
}

static int handle_upload(struct usb_request *req, u16 len)
{
	struct f_dfu *f_dfu = req->context;

	return dfu_read(dfu_get_entity(f_dfu->altsetting), req->buf,
			req->length, f_dfu->blk_seq_num);
}

static int handle_dnload(struct usb_gadget *gadget, u16 len)
{
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct usb_request *req = cdev->req;
	struct f_dfu *f_dfu = req->context;

	if (len == 0)
		f_dfu->dfu_state = DFU_STATE_dfuMANIFEST_SYNC;

	req->complete = dnload_request_complete;

	return len;
}

/*-------------------------------------------------------------------------*/
/* DFU state machine  */
static int state_app_idle(struct f_dfu *f_dfu,
			  const struct usb_ctrlrequest *ctrl,
			  struct usb_gadget *gadget,
			  struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	case USB_REQ_DFU_DETACH:
		f_dfu->dfu_state = DFU_STATE_appDETACH;
		to_dfu_mode(f_dfu);
		value = RET_ZLP;
		break;
	default:
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_app_detach(struct f_dfu *f_dfu,
			    const struct usb_ctrlrequest *ctrl,
			    struct usb_gadget *gadget,
			    struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_appIDLE;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_idle(struct f_dfu *f_dfu,
			  const struct usb_ctrlrequest *ctrl,
			  struct usb_gadget *gadget,
			  struct usb_request *req)
{
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 len = le16_to_cpu(ctrl->wLength);
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_DNLOAD:
		if (len == 0) {
			f_dfu->dfu_state = DFU_STATE_dfuERROR;
			value = RET_STALL;
			break;
		}
		f_dfu->dfu_state = DFU_STATE_dfuDNLOAD_SYNC;
		f_dfu->blk_seq_num = w_value;
		value = handle_dnload(gadget, len);
		break;
	case USB_REQ_DFU_UPLOAD:
		f_dfu->dfu_state = DFU_STATE_dfuUPLOAD_IDLE;
		f_dfu->blk_seq_num = 0;
		value = handle_upload(req, len);
		break;
	case USB_REQ_DFU_ABORT:
		/* no zlp? */
		value = RET_ZLP;
		break;
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	case USB_REQ_DFU_DETACH:
		/*
		 * Proprietary extension: 'detach' from idle mode and
		 * get back to runtime mode in case of USB Reset.  As
		 * much as I dislike this, we just can't use every USB
		 * bus reset to switch back to runtime mode, since at
		 * least the Linux USB stack likes to send a number of
		 * resets in a row :(
		 */
		f_dfu->dfu_state =
			DFU_STATE_dfuMANIFEST_WAIT_RST;
		to_runtime_mode(f_dfu);
		f_dfu->dfu_state = DFU_STATE_appIDLE;

		g_dnl_trigger_detach();
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_dnload_sync(struct f_dfu *f_dfu,
				 const struct usb_ctrlrequest *ctrl,
				 struct usb_gadget *gadget,
				 struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_dnbusy(struct f_dfu *f_dfu,
			    const struct usb_ctrlrequest *ctrl,
			    struct usb_gadget *gadget,
			    struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_dnload_idle(struct f_dfu *f_dfu,
				 const struct usb_ctrlrequest *ctrl,
				 struct usb_gadget *gadget,
				 struct usb_request *req)
{
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 len = le16_to_cpu(ctrl->wLength);
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_DNLOAD:
		f_dfu->dfu_state = DFU_STATE_dfuDNLOAD_SYNC;
		f_dfu->blk_seq_num = w_value;
		value = handle_dnload(gadget, len);
		break;
	case USB_REQ_DFU_ABORT:
		f_dfu->dfu_state = DFU_STATE_dfuIDLE;
		value = RET_ZLP;
		break;
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_manifest_sync(struct f_dfu *f_dfu,
				   const struct usb_ctrlrequest *ctrl,
				   struct usb_gadget *gadget,
				   struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		/* We're MainfestationTolerant */
		f_dfu->dfu_state = DFU_STATE_dfuMANIFEST;
		value = handle_getstatus(req);
		f_dfu->blk_seq_num = 0;
		req->complete = dnload_request_flush;
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_manifest(struct f_dfu *f_dfu,
			      const struct usb_ctrlrequest *ctrl,
			      struct usb_gadget *gadget,
			      struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		/* We're MainfestationTolerant */
		f_dfu->dfu_state = DFU_STATE_dfuIDLE;
		value = handle_getstatus(req);
		f_dfu->blk_seq_num = 0;
		puts("DOWNLOAD ... OK\nCtrl+C to exit ...\n");
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}
	return value;
}

static int state_dfu_upload_idle(struct f_dfu *f_dfu,
				 const struct usb_ctrlrequest *ctrl,
				 struct usb_gadget *gadget,
				 struct usb_request *req)
{
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 len = le16_to_cpu(ctrl->wLength);
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_UPLOAD:
		/* state transition if less data then requested */
		f_dfu->blk_seq_num = w_value;
		value = handle_upload(req, len);
		if (value >= 0 && value < len)
			f_dfu->dfu_state = DFU_STATE_dfuIDLE;
		break;
	case USB_REQ_DFU_ABORT:
		f_dfu->dfu_state = DFU_STATE_dfuIDLE;
		/* no zlp? */
		value = RET_ZLP;
		break;
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static int state_dfu_error(struct f_dfu *f_dfu,
				 const struct usb_ctrlrequest *ctrl,
				 struct usb_gadget *gadget,
				 struct usb_request *req)
{
	int value = 0;

	switch (ctrl->bRequest) {
	case USB_REQ_DFU_GETSTATUS:
		value = handle_getstatus(req);
		break;
	case USB_REQ_DFU_GETSTATE:
		value = handle_getstate(req);
		break;
	case USB_REQ_DFU_CLRSTATUS:
		f_dfu->dfu_state = DFU_STATE_dfuIDLE;
		f_dfu->dfu_status = DFU_STATUS_OK;
		/* no zlp? */
		value = RET_ZLP;
		break;
	default:
		f_dfu->dfu_state = DFU_STATE_dfuERROR;
		value = RET_STALL;
		break;
	}

	return value;
}

static dfu_state_fn dfu_state[] = {
	state_app_idle,          /* DFU_STATE_appIDLE */
	state_app_detach,        /* DFU_STATE_appDETACH */
	state_dfu_idle,          /* DFU_STATE_dfuIDLE */
	state_dfu_dnload_sync,   /* DFU_STATE_dfuDNLOAD_SYNC */
	state_dfu_dnbusy,        /* DFU_STATE_dfuDNBUSY */
	state_dfu_dnload_idle,   /* DFU_STATE_dfuDNLOAD_IDLE */
	state_dfu_manifest_sync, /* DFU_STATE_dfuMANIFEST_SYNC */
	state_dfu_manifest,	 /* DFU_STATE_dfuMANIFEST */
	NULL,                    /* DFU_STATE_dfuMANIFEST_WAIT_RST */
	state_dfu_upload_idle,   /* DFU_STATE_dfuUPLOAD_IDLE */
	state_dfu_error          /* DFU_STATE_dfuERROR */
};

static int
dfu_handle(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct usb_gadget *gadget = f->config->cdev->gadget;
	struct usb_request *req = f->config->cdev->req;
	struct f_dfu *f_dfu = f->config->cdev->req->context;
	u16 len = le16_to_cpu(ctrl->wLength);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	int value = 0;
	u8 req_type = ctrl->bRequestType & USB_TYPE_MASK;

	debug("w_value: 0x%x len: 0x%x\n", w_value, len);
	debug("req_type: 0x%x ctrl->bRequest: 0x%x f_dfu->dfu_state: 0x%x\n",
	       req_type, ctrl->bRequest, f_dfu->dfu_state);

	if (req_type == USB_TYPE_STANDARD) {
		if (ctrl->bRequest == USB_REQ_GET_DESCRIPTOR &&
		    (w_value >> 8) == DFU_DT_FUNC) {
			value = min(len, (u16) sizeof(dfu_func));
			memcpy(req->buf, &dfu_func, value);
		}
	} else /* DFU specific request */
		value = dfu_state[f_dfu->dfu_state] (f_dfu, ctrl, gadget, req);

	if (value >= 0) {
		req->length = value;
		req->zero = value < len;
		value = usb_ep_queue(gadget->ep0, req, 0);
		if (value < 0) {
			debug("ep_queue --> %d\n", value);
			req->status = 0;
		}
	}

	return value;
}

/*-------------------------------------------------------------------------*/

static int
dfu_prepare_strings(struct f_dfu *f_dfu, int n)
{
	struct dfu_entity *de = NULL;
	int i = 0;

	f_dfu->strings = calloc(sizeof(struct usb_string), n + 1);
	if (!f_dfu->strings)
		return -ENOMEM;

	for (i = 0; i < n; ++i) {
		de = dfu_get_entity(i);
		f_dfu->strings[i].s = de->name;
	}

	f_dfu->strings[i].id = 0;
	f_dfu->strings[i].s = NULL;

	return 0;
}

static int dfu_prepare_function(struct f_dfu *f_dfu, int n)
{
	struct usb_interface_descriptor *d;
	int i = 0;

	f_dfu->function = calloc(sizeof(struct usb_descriptor_header *), n + 2);
	if (!f_dfu->function)
		goto enomem;

	for (i = 0; i < n; ++i) {
		d = calloc(sizeof(*d), 1);
		if (!d)
			goto enomem;

		d->bLength =		sizeof(*d);
		d->bDescriptorType =	USB_DT_INTERFACE;
		d->bAlternateSetting =	i;
		d->bNumEndpoints =	0;
		d->bInterfaceClass =	USB_CLASS_APP_SPEC;
		d->bInterfaceSubClass =	1;
		d->bInterfaceProtocol =	2;

		f_dfu->function[i] = (struct usb_descriptor_header *)d;
	}

	/* add DFU Functional Descriptor */
	f_dfu->function[i] = calloc(sizeof(dfu_func), 1);
	if (!f_dfu->function[i])
		goto enomem;
	memcpy(f_dfu->function[i], &dfu_func, sizeof(dfu_func));

	i++;
	f_dfu->function[i] = NULL;

	return 0;

enomem:
	while (i) {
		free(f_dfu->function[--i]);
		f_dfu->function[i] = NULL;
	}
	free(f_dfu->function);

	return -ENOMEM;
}

static int dfu_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_dfu *f_dfu = func_to_dfu(f);
	const char *s;
	int alt_num = dfu_get_alt_number();
	int rv, id, i;

	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	dfu_intf_runtime.bInterfaceNumber = id;

	f_dfu->dfu_state = DFU_STATE_appIDLE;
	f_dfu->dfu_status = DFU_STATUS_OK;

	rv = dfu_prepare_function(f_dfu, alt_num);
	if (rv)
		goto error;

	rv = dfu_prepare_strings(f_dfu, alt_num);
	if (rv)
		goto error;
	for (i = 0; i < alt_num; i++) {
		id = usb_string_id(cdev);
		if (id < 0)
			return id;
		f_dfu->strings[i].id = id;
		((struct usb_interface_descriptor *)f_dfu->function[i])
			->iInterface = id;
	}

	to_dfu_mode(f_dfu);

	stringtab_dfu.strings = f_dfu->strings;

	cdev->req->context = f_dfu;

	s = env_get("serial#");
	if (s)
		g_dnl_set_serialnumber((char *)s);

error:
	return rv;
}

static void dfu_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_dfu *f_dfu = func_to_dfu(f);
	int alt_num = dfu_get_alt_number();
	int i;

	if (f_dfu->strings) {
		i = alt_num;
		while (i)
			f_dfu->strings[--i].s = NULL;

		free(f_dfu->strings);
	}

	if (f_dfu->function) {
		i = alt_num;
		while (i) {
			free(f_dfu->function[--i]);
			f_dfu->function[i] = NULL;
		}
		free(f_dfu->function);
	}

	free(f_dfu);
}

static int dfu_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_dfu *f_dfu = func_to_dfu(f);

	debug("%s: intf:%d alt:%d\n", __func__, intf, alt);

	f_dfu->altsetting = alt;
	f_dfu->dfu_state = DFU_STATE_dfuIDLE;
	f_dfu->dfu_status = DFU_STATUS_OK;

	return 0;
}

static int __dfu_get_alt(struct usb_function *f, unsigned intf)
{
	struct f_dfu *f_dfu = func_to_dfu(f);

	return f_dfu->altsetting;
}

/* TODO: is this really what we need here? */
static void dfu_disable(struct usb_function *f)
{
	struct f_dfu *f_dfu = func_to_dfu(f);
	if (f_dfu->config == 0)
		return;

	debug("%s: reset config\n", __func__);

	f_dfu->config = 0;
}

static int dfu_bind_config(struct usb_configuration *c)
{
	struct f_dfu *f_dfu;
	int status;

	f_dfu = calloc(sizeof(*f_dfu), 1);
	if (!f_dfu)
		return -ENOMEM;
	f_dfu->usb_function.name = "dfu";
	f_dfu->usb_function.hs_descriptors = dfu_runtime_descs;
	f_dfu->usb_function.descriptors = dfu_runtime_descs;
	f_dfu->usb_function.bind = dfu_bind;
	f_dfu->usb_function.unbind = dfu_unbind;
	f_dfu->usb_function.set_alt = dfu_set_alt;
	f_dfu->usb_function.get_alt = __dfu_get_alt;
	f_dfu->usb_function.disable = dfu_disable;
	f_dfu->usb_function.strings = dfu_generic_strings;
	f_dfu->usb_function.setup = dfu_handle;
	f_dfu->poll_timeout = DFU_DEFAULT_POLL_TIMEOUT;

	status = usb_add_function(c, &f_dfu->usb_function);
	if (status)
		free(f_dfu);

	return status;
}

int dfu_add(struct usb_configuration *c)
{
	int id;

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;
	strings_dfu_generic[0].id = id;
	dfu_intf_runtime.iInterface = id;

	debug("%s: cdev: 0x%p gadget:0x%p gadget->ep0: 0x%p\n", __func__,
	       c->cdev, c->cdev->gadget, c->cdev->gadget->ep0);

	return dfu_bind_config(c);
}

DECLARE_GADGET_BIND_CALLBACK(usb_dnl_dfu, dfu_add);
