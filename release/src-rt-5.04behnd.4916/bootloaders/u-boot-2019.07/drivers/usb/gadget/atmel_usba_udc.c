// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for the Atmel USBA high speed USB device controller
 * [Original from Linux kernel: drivers/usb/gadget/atmel_usba_udc.c]
 *
 * Copyright (C) 2005-2013 Atmel Corporation
 *			   Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/hardware.h>
#include <linux/list.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/atmel_usba_udc.h>
#include <malloc.h>

#include "atmel_usba_udc.h"

static int vbus_is_present(struct usba_udc *udc)
{
	/* No Vbus detection: Assume always present */
	return 1;
}

static void next_fifo_transaction(struct usba_ep *ep, struct usba_request *req)
{
	unsigned int transaction_len;

	transaction_len = req->req.length - req->req.actual;
	req->last_transaction = 1;
	if (transaction_len > ep->ep.maxpacket) {
		transaction_len = ep->ep.maxpacket;
		req->last_transaction = 0;
	} else if (transaction_len == ep->ep.maxpacket && req->req.zero) {
			req->last_transaction = 0;
	}

	DBG(DBG_QUEUE, "%s: submit_transaction, req %p (length %d)%s\n",
	    ep->ep.name, req, transaction_len,
	    req->last_transaction ? ", done" : "");

	memcpy(ep->fifo, req->req.buf + req->req.actual, transaction_len);
	usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
	req->req.actual += transaction_len;
}

static void submit_request(struct usba_ep *ep, struct usba_request *req)
{
	DBG(DBG_QUEUE, "%s: submit_request: req %p (length %d), dma: %d\n",
	    ep->ep.name, req, req->req.length, req->using_dma);

	req->req.actual = 0;
	req->submitted = 1;

	next_fifo_transaction(ep, req);
	if (req->last_transaction) {
		usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY);
		usba_ep_writel(ep, CTL_ENB, USBA_TX_COMPLETE);
	} else {
		usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
		usba_ep_writel(ep, CTL_ENB, USBA_TX_PK_RDY);
	}
}

static void submit_next_request(struct usba_ep *ep)
{
	struct usba_request *req;

	if (list_empty(&ep->queue)) {
		usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY | USBA_RX_BK_RDY);
		return;
	}

	req = list_entry(ep->queue.next, struct usba_request, queue);
	if (!req->submitted)
		submit_request(ep, req);
}

static void send_status(struct usba_udc *udc, struct usba_ep *ep)
{
	ep->state = STATUS_STAGE_IN;
	usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
	usba_ep_writel(ep, CTL_ENB, USBA_TX_COMPLETE);
}

static void receive_data(struct usba_ep *ep)
{
	struct usba_udc *udc = ep->udc;
	struct usba_request *req;
	unsigned long status;
	unsigned int bytecount, nr_busy;
	int is_complete = 0;

	status = usba_ep_readl(ep, STA);
	nr_busy = USBA_BFEXT(BUSY_BANKS, status);

	DBG(DBG_QUEUE, "receive data: nr_busy=%u\n", nr_busy);

	while (nr_busy > 0) {
		if (list_empty(&ep->queue)) {
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
			break;
		}
		req = list_entry(ep->queue.next,
				 struct usba_request, queue);

		bytecount = USBA_BFEXT(BYTE_COUNT, status);

		if (status & USBA_SHORT_PACKET)
			is_complete = 1;
		if (req->req.actual + bytecount >= req->req.length) {
			is_complete = 1;
			bytecount = req->req.length - req->req.actual;
		}

		memcpy(req->req.buf + req->req.actual, ep->fifo, bytecount);
		req->req.actual += bytecount;

		usba_ep_writel(ep, CLR_STA, USBA_RX_BK_RDY);

		if (is_complete) {
			DBG(DBG_QUEUE, "%s: request done\n", ep->ep.name);
			req->req.status = 0;
			list_del_init(&req->queue);
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
			spin_lock(&udc->lock);
			req->req.complete(&ep->ep, &req->req);
			spin_unlock(&udc->lock);
		}

		status = usba_ep_readl(ep, STA);
		nr_busy = USBA_BFEXT(BUSY_BANKS, status);

		if (is_complete && ep_is_control(ep)) {
			send_status(udc, ep);
			break;
		}
	}
}

static void
request_complete(struct usba_ep *ep, struct usba_request *req, int status)
{
	if (req->req.status == -EINPROGRESS)
		req->req.status = status;

	DBG(DBG_GADGET | DBG_REQ, "%s: req %p complete: status %d, actual %u\n",
	    ep->ep.name, req, req->req.status, req->req.actual);

	req->req.complete(&ep->ep, &req->req);
}

static void
request_complete_list(struct usba_ep *ep, struct list_head *list, int status)
{
	struct usba_request *req, *tmp_req;

	list_for_each_entry_safe(req, tmp_req, list, queue) {
		list_del_init(&req->queue);
		request_complete(ep, req, status);
	}
}

static int
usba_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_udc *udc = ep->udc;
	unsigned long flags = 0, ept_cfg, maxpacket;
	unsigned int nr_trans;

	DBG(DBG_GADGET, "%s: ep_enable: desc=%p\n", ep->ep.name, desc);

	maxpacket = usb_endpoint_maxp(desc) & 0x7ff;

	if (((desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)
	      != ep->index) ||
	      ep->index == 0 ||
	      desc->bDescriptorType != USB_DT_ENDPOINT ||
	      maxpacket == 0 ||
	      maxpacket > ep->fifo_size) {
		DBG(DBG_ERR, "ep_enable: Invalid argument");
		return -EINVAL;
	}

	ep->is_isoc = 0;
	ep->is_in = 0;

	if (maxpacket <= 8)
		ept_cfg = USBA_BF(EPT_SIZE, USBA_EPT_SIZE_8);
	else
		/* LSB is bit 1, not 0 */
		ept_cfg = USBA_BF(EPT_SIZE, fls(maxpacket - 1) - 3);

	DBG(DBG_HW, "%s: EPT_SIZE = %lu (maxpacket = %lu)\n",
	    ep->ep.name, ept_cfg, maxpacket);

	if (usb_endpoint_dir_in(desc)) {
		ep->is_in = 1;
		ept_cfg |= USBA_EPT_DIR_IN;
	}

	switch (usb_endpoint_type(desc)) {
	case USB_ENDPOINT_XFER_CONTROL:
		ept_cfg |= USBA_BF(EPT_TYPE, USBA_EPT_TYPE_CONTROL);
		ept_cfg |= USBA_BF(BK_NUMBER, USBA_BK_NUMBER_ONE);
		break;
	case USB_ENDPOINT_XFER_ISOC:
		if (!ep->can_isoc) {
			DBG(DBG_ERR, "ep_enable: %s is not isoc capable\n",
			    ep->ep.name);
			return -EINVAL;
		}

		/*
		 * Bits 11:12 specify number of _additional_
		 * transactions per microframe.
		 */
		nr_trans = ((usb_endpoint_maxp(desc) >> 11) & 3) + 1;
		if (nr_trans > 3)
			return -EINVAL;

		ep->is_isoc = 1;
		ept_cfg |= USBA_BF(EPT_TYPE, USBA_EPT_TYPE_ISO);

		/*
		 * Do triple-buffering on high-bandwidth iso endpoints.
		 */
		if (nr_trans > 1 && ep->nr_banks == 3)
			ept_cfg |= USBA_BF(BK_NUMBER, USBA_BK_NUMBER_TRIPLE);
		else
			ept_cfg |= USBA_BF(BK_NUMBER, USBA_BK_NUMBER_DOUBLE);
		ept_cfg |= USBA_BF(NB_TRANS, nr_trans);
		break;
	case USB_ENDPOINT_XFER_BULK:
		ept_cfg |= USBA_BF(EPT_TYPE, USBA_EPT_TYPE_BULK);
		ept_cfg |= USBA_BF(BK_NUMBER, USBA_BK_NUMBER_ONE);
		break;
	case USB_ENDPOINT_XFER_INT:
		ept_cfg |= USBA_BF(EPT_TYPE, USBA_EPT_TYPE_INT);
		ept_cfg |= USBA_BF(BK_NUMBER, USBA_BK_NUMBER_ONE);
		break;
	}

	spin_lock_irqsave(&ep->udc->lock, flags);

	ep->desc = desc;
	ep->ep.maxpacket = maxpacket;

	usba_ep_writel(ep, CFG, ept_cfg);
	usba_ep_writel(ep, CTL_ENB, USBA_EPT_ENABLE);

	usba_writel(udc, INT_ENB,
		    (usba_readl(udc, INT_ENB)
		     | USBA_BF(EPT_INT, 1 << ep->index)));

	spin_unlock_irqrestore(&udc->lock, flags);

	DBG(DBG_HW, "EPT_CFG%d after init: %#08lx\n", ep->index,
	    (unsigned long)usba_ep_readl(ep, CFG));
	DBG(DBG_HW, "INT_ENB after init: %#08lx\n",
	    (unsigned long)usba_readl(udc, INT_ENB));

	return 0;
}

static int usba_ep_disable(struct usb_ep *_ep)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_udc *udc = ep->udc;
	LIST_HEAD(req_list);
	unsigned long flags = 0;

	DBG(DBG_GADGET, "ep_disable: %s\n", ep->ep.name);

	spin_lock_irqsave(&udc->lock, flags);

	if (!ep->desc) {
		spin_unlock_irqrestore(&udc->lock, flags);
		/* REVISIT because this driver disables endpoints in
		 * reset_all_endpoints() before calling disconnect(),
		 * most gadget drivers would trigger this non-error ...
		 */
		if (udc->gadget.speed != USB_SPEED_UNKNOWN)
			DBG(DBG_ERR, "ep_disable: %s not enabled\n",
			    ep->ep.name);
		return -EINVAL;
	}
	ep->desc = NULL;

	list_splice_init(&ep->queue, &req_list);
	usba_ep_writel(ep, CFG, 0);
	usba_ep_writel(ep, CTL_DIS, USBA_EPT_ENABLE);
	usba_writel(udc, INT_ENB,
		    usba_readl(udc, INT_ENB) &
		    ~USBA_BF(EPT_INT, 1 << ep->index));

	request_complete_list(ep, &req_list, -ESHUTDOWN);

	spin_unlock_irqrestore(&udc->lock, flags);

	return 0;
}

static struct usb_request *
usba_ep_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct usba_request *req;

	DBG(DBG_GADGET, "ep_alloc_request: %p, 0x%x\n", _ep, gfp_flags);

	req = calloc(1, sizeof(struct usba_request));
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void
usba_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct usba_request *req = to_usba_req(_req);

	DBG(DBG_GADGET, "ep_free_request: %p, %p\n", _ep, _req);

	free(req);
}

static int
usba_ep_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct usba_request *req = to_usba_req(_req);
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_udc *udc = ep->udc;
	unsigned long flags = 0;
	int ret;

	DBG(DBG_GADGET | DBG_QUEUE | DBG_REQ, "%s: queue req %p, len %u\n",
	    ep->ep.name, req, _req->length);

	if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN ||
	    !ep->desc)
		return -ESHUTDOWN;

	req->submitted = 0;
	req->using_dma = 0;
	req->last_transaction = 0;

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* May have received a reset since last time we checked */
	ret = -ESHUTDOWN;
	spin_lock_irqsave(&udc->lock, flags);
	if (ep->desc) {
		list_add_tail(&req->queue, &ep->queue);

		if ((!ep_is_control(ep) && ep->is_in) ||
		    (ep_is_control(ep) && (ep->state == DATA_STAGE_IN ||
		    ep->state == STATUS_STAGE_IN)))
			usba_ep_writel(ep, CTL_ENB, USBA_TX_PK_RDY);
		else
			usba_ep_writel(ep, CTL_ENB, USBA_RX_BK_RDY);

		ret = 0;
	}
	spin_unlock_irqrestore(&udc->lock, flags);

	return ret;
}

static int usba_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_request *req = to_usba_req(_req);

	DBG(DBG_GADGET | DBG_QUEUE, "ep_dequeue: %s, req %p\n",
	    ep->ep.name, req);

	/*
	 * Errors should stop the queue from advancing until the
	 * completion function returns.
	 */
	list_del_init(&req->queue);

	request_complete(ep, req, -ECONNRESET);

	/* Process the next request if any */
	submit_next_request(ep);

	return 0;
}

static int usba_ep_set_halt(struct usb_ep *_ep, int value)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	unsigned long flags = 0;
	int ret = 0;

	DBG(DBG_GADGET, "endpoint %s: %s HALT\n", ep->ep.name,
	    value ? "set" : "clear");

	if (!ep->desc) {
		DBG(DBG_ERR, "Attempted to halt uninitialized ep %s\n",
		    ep->ep.name);
		return -ENODEV;
	}

	if (ep->is_isoc) {
		DBG(DBG_ERR, "Attempted to halt isochronous ep %s\n",
		    ep->ep.name);
		return -ENOTTY;
	}

	spin_lock_irqsave(&udc->lock, flags);

	/*
	 * We can't halt IN endpoints while there are still data to be
	 * transferred
	 */
	if (!list_empty(&ep->queue) ||
	    ((value && ep->is_in && (usba_ep_readl(ep, STA) &
	    USBA_BF(BUSY_BANKS, -1L))))) {
		ret = -EAGAIN;
	} else {
		if (value)
			usba_ep_writel(ep, SET_STA, USBA_FORCE_STALL);
		else
			usba_ep_writel(ep, CLR_STA,
				       USBA_FORCE_STALL | USBA_TOGGLE_CLR);
		usba_ep_readl(ep, STA);
	}

	spin_unlock_irqrestore(&udc->lock, flags);

	return ret;
}

static int usba_ep_fifo_status(struct usb_ep *_ep)
{
	struct usba_ep *ep = to_usba_ep(_ep);

	return USBA_BFEXT(BYTE_COUNT, usba_ep_readl(ep, STA));
}

static void usba_ep_fifo_flush(struct usb_ep *_ep)
{
	struct usba_ep *ep = to_usba_ep(_ep);
	struct usba_udc *udc = ep->udc;

	usba_writel(udc, EPT_RST, 1 << ep->index);
}

static const struct usb_ep_ops usba_ep_ops = {
	.enable		= usba_ep_enable,
	.disable	= usba_ep_disable,
	.alloc_request	= usba_ep_alloc_request,
	.free_request	= usba_ep_free_request,
	.queue		= usba_ep_queue,
	.dequeue	= usba_ep_dequeue,
	.set_halt	= usba_ep_set_halt,
	.fifo_status	= usba_ep_fifo_status,
	.fifo_flush	= usba_ep_fifo_flush,
};

static int usba_udc_get_frame(struct usb_gadget *gadget)
{
	struct usba_udc *udc = to_usba_udc(gadget);

	return USBA_BFEXT(FRAME_NUMBER, usba_readl(udc, FNUM));
}

static int usba_udc_wakeup(struct usb_gadget *gadget)
{
	struct usba_udc *udc = to_usba_udc(gadget);
	unsigned long flags = 0;
	u32 ctrl;
	int ret = -EINVAL;

	spin_lock_irqsave(&udc->lock, flags);
	if (udc->devstatus & (1 << USB_DEVICE_REMOTE_WAKEUP)) {
		ctrl = usba_readl(udc, CTRL);
		usba_writel(udc, CTRL, ctrl | USBA_REMOTE_WAKE_UP);
		ret = 0;
	}
	spin_unlock_irqrestore(&udc->lock, flags);

	return ret;
}

static int
usba_udc_set_selfpowered(struct usb_gadget *gadget, int is_selfpowered)
{
	struct usba_udc *udc = to_usba_udc(gadget);
	unsigned long flags = 0;

	spin_lock_irqsave(&udc->lock, flags);
	if (is_selfpowered)
		udc->devstatus |= 1 << USB_DEVICE_SELF_POWERED;
	else
		udc->devstatus &= ~(1 << USB_DEVICE_SELF_POWERED);
	spin_unlock_irqrestore(&udc->lock, flags);

	return 0;
}

static const struct usb_gadget_ops usba_udc_ops = {
	.get_frame		= usba_udc_get_frame,
	.wakeup			= usba_udc_wakeup,
	.set_selfpowered	= usba_udc_set_selfpowered,
};

static struct usb_endpoint_descriptor usba_ep0_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0,
	.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
	.wMaxPacketSize = cpu_to_le16(64),
	/* FIXME: I have no idea what to put here */
	.bInterval = 1,
};

/*
 * Called with interrupts disabled and udc->lock held.
 */
static void reset_all_endpoints(struct usba_udc *udc)
{
	struct usba_ep *ep;
	struct usba_request *req, *tmp_req;

	usba_writel(udc, EPT_RST, ~0UL);

	ep = to_usba_ep(udc->gadget.ep0);
	list_for_each_entry_safe(req, tmp_req, &ep->queue, queue) {
		list_del_init(&req->queue);
		request_complete(ep, req, -ECONNRESET);
	}

	/* NOTE:  normally, the next call to the gadget driver is in
	 * charge of disabling endpoints... usually disconnect().
	 * The exception would be entering a high speed test mode.
	 *
	 * FIXME remove this code ... and retest thoroughly.
	 */
	list_for_each_entry(ep, &udc->gadget.ep_list, ep.ep_list) {
		if (ep->desc) {
			spin_unlock(&udc->lock);
			usba_ep_disable(&ep->ep);
			spin_lock(&udc->lock);
		}
	}
}

static struct usba_ep *get_ep_by_addr(struct usba_udc *udc, u16 wIndex)
{
	struct usba_ep *ep;

	if ((wIndex & USB_ENDPOINT_NUMBER_MASK) == 0)
		return to_usba_ep(udc->gadget.ep0);

	list_for_each_entry(ep, &udc->gadget.ep_list, ep.ep_list) {
		u8 bEndpointAddress;

		if (!ep->desc)
			continue;
		bEndpointAddress = ep->desc->bEndpointAddress;
		if ((wIndex ^ bEndpointAddress) & USB_DIR_IN)
			continue;
		if ((bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)
				== (wIndex & USB_ENDPOINT_NUMBER_MASK))
			return ep;
	}

	return NULL;
}

/* Called with interrupts disabled and udc->lock held */
static inline void set_protocol_stall(struct usba_udc *udc, struct usba_ep *ep)
{
	usba_ep_writel(ep, SET_STA, USBA_FORCE_STALL);
	ep->state = WAIT_FOR_SETUP;
}

static inline int is_stalled(struct usba_udc *udc, struct usba_ep *ep)
{
	if (usba_ep_readl(ep, STA) & USBA_FORCE_STALL)
		return 1;
	return 0;
}

static inline void set_address(struct usba_udc *udc, unsigned int addr)
{
	u32 regval;

	DBG(DBG_BUS, "setting address %u...\n", addr);
	regval = usba_readl(udc, CTRL);
	regval = USBA_BFINS(DEV_ADDR, addr, regval);
	usba_writel(udc, CTRL, regval);
}

static int do_test_mode(struct usba_udc *udc)
{
	static const char test_packet_buffer[] = {
		/* JKJKJKJK * 9 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* JJKKJJKK * 8 */
		0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
		/* JJKKJJKK * 8 */
		0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
		/* JJJJJJJKKKKKKK * 8 */
		0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		/* JJJJJJJK * 8 */
		0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD,
		/* {JKKKKKKK * 10}, JK */
		0xFC, 0x7E, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0x7E
	};
	struct usba_ep *ep;
	int test_mode;

	test_mode = udc->test_mode;

	/* Start from a clean slate */
	reset_all_endpoints(udc);

	switch (test_mode) {
	case 0x0100:
		/* Test_J */
		usba_writel(udc, TST, USBA_TST_J_MODE);
		DBG(DBG_ALL, "Entering Test_J mode...\n");
		break;
	case 0x0200:
		/* Test_K */
		usba_writel(udc, TST, USBA_TST_K_MODE);
		DBG(DBG_ALL, "Entering Test_K mode...\n");
		break;
	case 0x0300:
		/*
		 * Test_SE0_NAK: Force high-speed mode and set up ep0
		 * for Bulk IN transfers
		 */
		ep = &udc->usba_ep[0];
		usba_writel(udc, TST,
			    USBA_BF(SPEED_CFG, USBA_SPEED_CFG_FORCE_HIGH));
		usba_ep_writel(ep, CFG,
			       USBA_BF(EPT_SIZE, USBA_EPT_SIZE_64)
			       | USBA_EPT_DIR_IN
			       | USBA_BF(EPT_TYPE, USBA_EPT_TYPE_BULK)
			       | USBA_BF(BK_NUMBER, 1));
		if (!(usba_ep_readl(ep, CFG) & USBA_EPT_MAPPED)) {
			set_protocol_stall(udc, ep);
			DBG(DBG_ALL, "Test_SE0_NAK: ep0 not mapped\n");
		} else {
			usba_ep_writel(ep, CTL_ENB, USBA_EPT_ENABLE);
			DBG(DBG_ALL, "Entering Test_SE0_NAK mode...\n");
		}
		break;
	case 0x0400:
		/* Test_Packet */
		ep = &udc->usba_ep[0];
		usba_ep_writel(ep, CFG,
			       USBA_BF(EPT_SIZE, USBA_EPT_SIZE_64)
			       | USBA_EPT_DIR_IN
			       | USBA_BF(EPT_TYPE, USBA_EPT_TYPE_BULK)
			       | USBA_BF(BK_NUMBER, 1));
		if (!(usba_ep_readl(ep, CFG) & USBA_EPT_MAPPED)) {
			set_protocol_stall(udc, ep);
			DBG(DBG_ALL, "Test_Packet: ep0 not mapped\n");
		} else {
			usba_ep_writel(ep, CTL_ENB, USBA_EPT_ENABLE);
			usba_writel(udc, TST, USBA_TST_PKT_MODE);
			memcpy(ep->fifo, test_packet_buffer,
			       sizeof(test_packet_buffer));
			usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
			DBG(DBG_ALL, "Entering Test_Packet mode...\n");
		}
		break;
	default:
		DBG(DBG_ERR, "Invalid test mode: 0x%04x\n", test_mode);
		return -EINVAL;
	}

	return 0;
}

/* Avoid overly long expressions */
static inline bool feature_is_dev_remote_wakeup(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_DEVICE_REMOTE_WAKEUP))
		return true;
	return false;
}

static inline bool feature_is_dev_test_mode(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_DEVICE_TEST_MODE))
		return true;
	return false;
}

static inline bool feature_is_ep_halt(struct usb_ctrlrequest *crq)
{
	if (crq->wValue == cpu_to_le16(USB_ENDPOINT_HALT))
		return true;
	return false;
}

static int handle_ep0_setup(struct usba_udc *udc, struct usba_ep *ep,
		struct usb_ctrlrequest *crq)
{
	int retval = 0;

	switch (crq->bRequest) {
	case USB_REQ_GET_STATUS: {
		u16 status;

		if (crq->bRequestType == (USB_DIR_IN | USB_RECIP_DEVICE)) {
			status = cpu_to_le16(udc->devstatus);
		} else if (crq->bRequestType
				== (USB_DIR_IN | USB_RECIP_INTERFACE)) {
			status = cpu_to_le16(0);
		} else if (crq->bRequestType
				== (USB_DIR_IN | USB_RECIP_ENDPOINT)) {
			struct usba_ep *target;

			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			status = 0;
			if (is_stalled(udc, target))
				status |= cpu_to_le16(1);
		} else {
			goto delegate;
		}

		/* Write directly to the FIFO. No queueing is done. */
		if (crq->wLength != cpu_to_le16(sizeof(status)))
			goto stall;
		ep->state = DATA_STAGE_IN;
		__raw_writew(status, ep->fifo);
		usba_ep_writel(ep, SET_STA, USBA_TX_PK_RDY);
		break;
	}

	case USB_REQ_CLEAR_FEATURE: {
		if (crq->bRequestType == USB_RECIP_DEVICE) {
			if (feature_is_dev_remote_wakeup(crq))
				udc->devstatus
					&= ~(1 << USB_DEVICE_REMOTE_WAKEUP);
			else
				/* Can't CLEAR_FEATURE TEST_MODE */
				goto stall;
		} else if (crq->bRequestType == USB_RECIP_ENDPOINT) {
			struct usba_ep *target;

			if (crq->wLength != cpu_to_le16(0) ||
			    !feature_is_ep_halt(crq))
				goto stall;
			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			usba_ep_writel(target, CLR_STA, USBA_FORCE_STALL);
			if (target->index != 0)
				usba_ep_writel(target, CLR_STA,
					       USBA_TOGGLE_CLR);
		} else {
			goto delegate;
		}

		send_status(udc, ep);
		break;
	}

	case USB_REQ_SET_FEATURE: {
		if (crq->bRequestType == USB_RECIP_DEVICE) {
			if (feature_is_dev_test_mode(crq)) {
				send_status(udc, ep);
				ep->state = STATUS_STAGE_TEST;
				udc->test_mode = le16_to_cpu(crq->wIndex);
				return 0;
			} else if (feature_is_dev_remote_wakeup(crq)) {
				udc->devstatus |= 1 << USB_DEVICE_REMOTE_WAKEUP;
			} else {
				goto stall;
			}
		} else if (crq->bRequestType == USB_RECIP_ENDPOINT) {
			struct usba_ep *target;

			if (crq->wLength != cpu_to_le16(0) ||
			    !feature_is_ep_halt(crq))
				goto stall;

			target = get_ep_by_addr(udc, le16_to_cpu(crq->wIndex));
			if (!target)
				goto stall;

			usba_ep_writel(target, SET_STA, USBA_FORCE_STALL);
		} else {
			goto delegate;
		}

		send_status(udc, ep);
		break;
	}

	case USB_REQ_SET_ADDRESS:
		if (crq->bRequestType != (USB_DIR_OUT | USB_RECIP_DEVICE))
			goto delegate;

		set_address(udc, le16_to_cpu(crq->wValue));
		send_status(udc, ep);
		ep->state = STATUS_STAGE_ADDR;
		break;

	default:
delegate:
		spin_unlock(&udc->lock);
		retval = udc->driver->setup(&udc->gadget, crq);
		spin_lock(&udc->lock);
	}

	return retval;

stall:
	DBG(DBG_ALL, "%s: Invalid setup request: %02x.%02x v%04x i%04x l%d\n",
	    ep->ep.name, crq->bRequestType, crq->bRequest,
	    le16_to_cpu(crq->wValue), le16_to_cpu(crq->wIndex),
	    le16_to_cpu(crq->wLength));
	set_protocol_stall(udc, ep);

	return -1;
}

static void usba_control_irq(struct usba_udc *udc, struct usba_ep *ep)
{
	struct usba_request *req;
	u32 epstatus;
	u32 epctrl;

restart:
	epstatus = usba_ep_readl(ep, STA);
	epctrl = usba_ep_readl(ep, CTL);

	DBG(DBG_INT, "%s [%d]: s/%08x c/%08x\n",
	    ep->ep.name, ep->state, epstatus, epctrl);

	req = NULL;
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next,
				 struct usba_request, queue);

	if ((epctrl & USBA_TX_PK_RDY) && !(epstatus & USBA_TX_PK_RDY)) {
		if (req->submitted)
			next_fifo_transaction(ep, req);
		else
			submit_request(ep, req);

		if (req->last_transaction) {
			usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY);
			usba_ep_writel(ep, CTL_ENB, USBA_TX_COMPLETE);
		}
		goto restart;
	}
	if ((epstatus & epctrl) & USBA_TX_COMPLETE) {
		usba_ep_writel(ep, CLR_STA, USBA_TX_COMPLETE);

		switch (ep->state) {
		case DATA_STAGE_IN:
			usba_ep_writel(ep, CTL_ENB, USBA_RX_BK_RDY);
			usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
			ep->state = STATUS_STAGE_OUT;
			break;
		case STATUS_STAGE_ADDR:
			/* Activate our new address */
			usba_writel(udc, CTRL, (usba_readl(udc, CTRL)
						| USBA_FADDR_EN));
			usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
			ep->state = WAIT_FOR_SETUP;
			break;
		case STATUS_STAGE_IN:
			if (req) {
				list_del_init(&req->queue);
				request_complete(ep, req, 0);
				submit_next_request(ep);
			}
			usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
			ep->state = WAIT_FOR_SETUP;
			break;
		case STATUS_STAGE_TEST:
			usba_ep_writel(ep, CTL_DIS, USBA_TX_COMPLETE);
			ep->state = WAIT_FOR_SETUP;
			if (do_test_mode(udc))
				set_protocol_stall(udc, ep);
			break;
		default:
			DBG(DBG_ALL, "%s: TXCOMP: Invalid endpoint state %d\n",
			    ep->ep.name, ep->state);
			set_protocol_stall(udc, ep);
			break;
		}

		goto restart;
	}
	if ((epstatus & epctrl) & USBA_RX_BK_RDY) {
		switch (ep->state) {
		case STATUS_STAGE_OUT:
			usba_ep_writel(ep, CLR_STA, USBA_RX_BK_RDY);
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);

			if (req) {
				list_del_init(&req->queue);
				request_complete(ep, req, 0);
			}
			ep->state = WAIT_FOR_SETUP;
			break;

		case DATA_STAGE_OUT:
			receive_data(ep);
			break;

		default:
			usba_ep_writel(ep, CLR_STA, USBA_RX_BK_RDY);
			usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
			DBG(DBG_ALL, "%s: RXRDY: Invalid endpoint state %d\n",
			    ep->ep.name, ep->state);
			set_protocol_stall(udc, ep);
			break;
		}

		goto restart;
	}
	if (epstatus & USBA_RX_SETUP) {
		union {
			struct usb_ctrlrequest crq;
			unsigned long data[2];
		} crq;
		unsigned int pkt_len;
		int ret;

		if (ep->state != WAIT_FOR_SETUP) {
			/*
			 * Didn't expect a SETUP packet at this
			 * point. Clean up any pending requests (which
			 * may be successful).
			 */
			int status = -EPROTO;

			/*
			 * RXRDY and TXCOMP are dropped when SETUP
			 * packets arrive.  Just pretend we received
			 * the status packet.
			 */
			if (ep->state == STATUS_STAGE_OUT ||
			    ep->state == STATUS_STAGE_IN) {
				usba_ep_writel(ep, CTL_DIS, USBA_RX_BK_RDY);
				status = 0;
			}

			if (req) {
				list_del_init(&req->queue);
				request_complete(ep, req, status);
			}
		}

		pkt_len = USBA_BFEXT(BYTE_COUNT, usba_ep_readl(ep, STA));
		DBG(DBG_HW, "Packet length: %u\n", pkt_len);
		if (pkt_len != sizeof(crq)) {
			DBG(DBG_ALL, "udc: Invalid length %u (expected %zu)\n",
			    pkt_len, sizeof(crq));
			set_protocol_stall(udc, ep);
			return;
		}

		DBG(DBG_FIFO, "Copying ctrl request from 0x%p:\n", ep->fifo);
		memcpy(crq.data, ep->fifo, sizeof(crq));

		/* Free up one bank in the FIFO so that we can
		 * generate or receive a reply right away. */
		usba_ep_writel(ep, CLR_STA, USBA_RX_SETUP);

		if (crq.crq.bRequestType & USB_DIR_IN) {
			/*
			 * The USB 2.0 spec states that "if wLength is
			 * zero, there is no data transfer phase."
			 * However, testusb #14 seems to actually
			 * expect a data phase even if wLength = 0...
			 */
			ep->state = DATA_STAGE_IN;
		} else {
			if (crq.crq.wLength != cpu_to_le16(0))
				ep->state = DATA_STAGE_OUT;
			else
				ep->state = STATUS_STAGE_IN;
		}

		ret = -1;
		if (ep->index == 0) {
			ret = handle_ep0_setup(udc, ep, &crq.crq);
		} else {
			spin_unlock(&udc->lock);
			ret = udc->driver->setup(&udc->gadget, &crq.crq);
			spin_lock(&udc->lock);
		}

		DBG(DBG_BUS, "req %02x.%02x, length %d, state %d, ret %d\n",
		    crq.crq.bRequestType, crq.crq.bRequest,
		    le16_to_cpu(crq.crq.wLength), ep->state, ret);

		if (ret < 0) {
			/* Let the host know that we failed */
			set_protocol_stall(udc, ep);
		}
	}
}

static void usba_ep_irq(struct usba_udc *udc, struct usba_ep *ep)
{
	struct usba_request *req;
	u32 epstatus;
	u32 epctrl;

	epstatus = usba_ep_readl(ep, STA);
	epctrl = usba_ep_readl(ep, CTL);

	DBG(DBG_INT, "%s: interrupt, status: 0x%08x\n", ep->ep.name, epstatus);

	while ((epctrl & USBA_TX_PK_RDY) && !(epstatus & USBA_TX_PK_RDY)) {
		DBG(DBG_BUS, "%s: TX PK ready\n", ep->ep.name);

		if (list_empty(&ep->queue)) {
			DBG(DBG_INT, "ep_irq: queue empty\n");
			usba_ep_writel(ep, CTL_DIS, USBA_TX_PK_RDY);
			return;
		}

		req = list_entry(ep->queue.next, struct usba_request, queue);

		if (req->submitted)
			next_fifo_transaction(ep, req);
		else
			submit_request(ep, req);

		if (req->last_transaction) {
			list_del_init(&req->queue);
			submit_next_request(ep);
			request_complete(ep, req, 0);
		}

		epstatus = usba_ep_readl(ep, STA);
		epctrl = usba_ep_readl(ep, CTL);
	}

	if ((epstatus & epctrl) & USBA_RX_BK_RDY) {
		DBG(DBG_BUS, "%s: RX data ready\n", ep->ep.name);
		receive_data(ep);
	}
}

static int usba_udc_irq(struct usba_udc *udc)
{
	u32 status, ep_status;

	spin_lock(&udc->lock);

	status = usba_readl(udc, INT_STA);
	DBG(DBG_INT, "irq, status=%#08x\n", status);

	if (status & USBA_DET_SUSPEND) {
		usba_writel(udc, INT_CLR, USBA_DET_SUSPEND);
		DBG(DBG_BUS, "Suspend detected\n");
		if (udc->gadget.speed != USB_SPEED_UNKNOWN &&
		    udc->driver && udc->driver->suspend) {
			spin_unlock(&udc->lock);
			udc->driver->suspend(&udc->gadget);
			spin_lock(&udc->lock);
		}
	}

	if (status & USBA_WAKE_UP) {
		usba_writel(udc, INT_CLR, USBA_WAKE_UP);
		DBG(DBG_BUS, "Wake Up CPU detected\n");
	}

	if (status & USBA_END_OF_RESUME) {
		usba_writel(udc, INT_CLR, USBA_END_OF_RESUME);
		DBG(DBG_BUS, "Resume detected\n");
		if (udc->gadget.speed != USB_SPEED_UNKNOWN &&
		    udc->driver && udc->driver->resume) {
			spin_unlock(&udc->lock);
			udc->driver->resume(&udc->gadget);
			spin_lock(&udc->lock);
		}
	}

	ep_status = USBA_BFEXT(EPT_INT, status);
	if (ep_status) {
		int i;

		for (i = 0; i < USBA_NR_ENDPOINTS; i++)
			if (ep_status & (1 << i)) {
				if (ep_is_control(&udc->usba_ep[i]))
					usba_control_irq(udc, &udc->usba_ep[i]);
				else
					usba_ep_irq(udc, &udc->usba_ep[i]);
			}
	}

	if (status & USBA_END_OF_RESET) {
		struct usba_ep *ep0;

		usba_writel(udc, INT_CLR, USBA_END_OF_RESET);
		reset_all_endpoints(udc);

		if (udc->gadget.speed != USB_SPEED_UNKNOWN &&
		    udc->driver->disconnect) {
			udc->gadget.speed = USB_SPEED_UNKNOWN;
			spin_unlock(&udc->lock);
			udc->driver->disconnect(&udc->gadget);
			spin_lock(&udc->lock);
		}

		if (status & USBA_HIGH_SPEED)
			udc->gadget.speed = USB_SPEED_HIGH;
		else
			udc->gadget.speed = USB_SPEED_FULL;

		ep0 = &udc->usba_ep[0];
		ep0->desc = &usba_ep0_desc;
		ep0->state = WAIT_FOR_SETUP;
		usba_ep_writel(ep0, CFG,
			       (USBA_BF(EPT_SIZE, EP0_EPT_SIZE)
				| USBA_BF(EPT_TYPE, USBA_EPT_TYPE_CONTROL)
				| USBA_BF(BK_NUMBER, USBA_BK_NUMBER_ONE)));
		usba_ep_writel(ep0, CTL_ENB,
			       USBA_EPT_ENABLE | USBA_RX_SETUP);
		usba_writel(udc, INT_ENB,
			    (usba_readl(udc, INT_ENB)
			     | USBA_BF(EPT_INT, 1)
			     | USBA_DET_SUSPEND
			     | USBA_END_OF_RESUME));

		/*
		 * Unclear why we hit this irregularly, e.g. in usbtest,
		 * but it's clearly harmless...
		 */
		if (!(usba_ep_readl(ep0, CFG) & USBA_EPT_MAPPED))
			DBG(DBG_ALL, "ODD: EP0 configuration is invalid!\n");
	}

	spin_unlock(&udc->lock);

	return 0;
}

static int atmel_usba_start(struct usba_udc *udc)
{
	udc->devstatus = 1 << USB_DEVICE_SELF_POWERED;

	udc->vbus_prev = 0;

	/* If Vbus is present, enable the controller and wait for reset */
	if (vbus_is_present(udc) && udc->vbus_prev == 0) {
		usba_writel(udc, CTRL, USBA_ENABLE_MASK);
		usba_writel(udc, INT_ENB, USBA_END_OF_RESET);
	}

	return 0;
}

static int atmel_usba_stop(struct usba_udc *udc)
{
	udc->gadget.speed = USB_SPEED_UNKNOWN;
	reset_all_endpoints(udc);

	/* This will also disable the DP pullup */
	usba_writel(udc, CTRL, USBA_DISABLE_MASK);

	return 0;
}

static struct usba_udc controller = {
	.regs = (unsigned *)ATMEL_BASE_UDPHS,
	.fifo = (unsigned *)ATMEL_BASE_UDPHS_FIFO,
	.gadget = {
		.ops		= &usba_udc_ops,
		.ep_list	= LIST_HEAD_INIT(controller.gadget.ep_list),
		.speed		= USB_SPEED_HIGH,
		.is_dualspeed	= 1,
		.name		= "atmel_usba_udc",
	},
};

int usb_gadget_handle_interrupts(int index)
{
	struct usba_udc *udc = &controller;

	return usba_udc_irq(udc);
}


int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct usba_udc *udc = &controller;
	int ret;

	if (!driver || !driver->bind || !driver->setup) {
		printf("bad paramter\n");
		return -EINVAL;
	}

	if (udc->driver) {
		printf("UDC already has a gadget driver\n");
		return -EBUSY;
	}

	atmel_usba_start(udc);

	udc->driver = driver;

	ret = driver->bind(&udc->gadget);
	if (ret) {
		pr_err("driver->bind() returned %d\n", ret);
		udc->driver = NULL;
	}

	return ret;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct usba_udc *udc = &controller;

	if (!driver || !driver->unbind || !driver->disconnect) {
		pr_err("bad paramter\n");
		return -EINVAL;
	}

	driver->disconnect(&udc->gadget);
	driver->unbind(&udc->gadget);
	udc->driver = NULL;

	atmel_usba_stop(udc);

	return 0;
}

static struct usba_ep *usba_udc_pdata(struct usba_platform_data *pdata,
				      struct usba_udc *udc)
{
	struct usba_ep *eps;
	int i;

	eps = malloc(sizeof(struct usba_ep) * pdata->num_ep);
	if (!eps) {
		pr_err("failed to alloc eps\n");
		return NULL;
	}

	udc->gadget.ep0 = &eps[0].ep;

	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&eps[0].ep.ep_list);

	for (i = 0; i < pdata->num_ep; i++) {
		struct usba_ep *ep = &eps[i];

		ep->ep_regs = udc->regs + USBA_EPT_BASE(i);
		ep->dma_regs = udc->regs + USBA_DMA_BASE(i);
		ep->fifo = udc->fifo + USBA_FIFO_BASE(i);
		ep->ep.ops = &usba_ep_ops;
		ep->ep.name = pdata->ep[i].name;
		ep->ep.maxpacket = pdata->ep[i].fifo_size;
		ep->fifo_size = ep->ep.maxpacket;
		ep->udc = udc;
		INIT_LIST_HEAD(&ep->queue);
		ep->nr_banks = pdata->ep[i].nr_banks;
		ep->index = pdata->ep[i].index;
		ep->can_dma = pdata->ep[i].can_dma;
		ep->can_isoc = pdata->ep[i].can_isoc;
		if (i)
			list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);
	};

	return eps;
}

int usba_udc_probe(struct usba_platform_data *pdata)
{
	struct usba_udc *udc;

	udc = &controller;

	udc->usba_ep = usba_udc_pdata(pdata, udc);

	return 0;
}
