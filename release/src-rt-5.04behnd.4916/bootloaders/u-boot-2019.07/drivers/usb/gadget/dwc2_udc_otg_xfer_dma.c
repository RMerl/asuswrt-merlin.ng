// SPDX-License-Identifier: GPL-2.0+
/*
 * drivers/usb/gadget/dwc2_udc_otg_xfer_dma.c
 * Designware DWC2 on-chip full/high speed USB OTG 2.0 device controllers
 *
 * Copyright (C) 2009 for Samsung Electronics
 *
 * BSP Support for Samsung's UDC driver
 * available at:
 * git://git.kernel.org/pub/scm/linux/kernel/git/kki_ap/linux-2.6-samsung.git
 *
 * State machine bugfixes:
 * Marek Szyprowski <m.szyprowski@samsung.com>
 *
 * Ported to u-boot:
 * Marek Szyprowski <m.szyprowski@samsung.com>
 * Lukasz Majewski <l.majewski@samsumg.com>
 */

static u8 clear_feature_num;
int clear_feature_flag;

/* Bulk-Only Mass Storage Reset (class-specific request) */
#define GET_MAX_LUN_REQUEST	0xFE
#define BOT_RESET_REQUEST	0xFF

static inline void dwc2_udc_ep0_zlp(struct dwc2_udc *dev)
{
	u32 ep_ctrl;

	writel(usb_ctrl_dma_addr, &reg->in_endp[EP0_CON].diepdma);
	writel(DIEPT_SIZ_PKT_CNT(1), &reg->in_endp[EP0_CON].dieptsiz);

	ep_ctrl = readl(&reg->in_endp[EP0_CON].diepctl);
	writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK,
	       &reg->in_endp[EP0_CON].diepctl);

	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DIEPCTL0 = 0x%x\n",
		__func__, readl(&reg->in_endp[EP0_CON].diepctl));
	dev->ep0state = WAIT_FOR_IN_COMPLETE;
}

static void dwc2_udc_pre_setup(void)
{
	u32 ep_ctrl;

	debug_cond(DEBUG_IN_EP,
		   "%s : Prepare Setup packets.\n", __func__);

	writel(DOEPT_SIZ_PKT_CNT(1) | sizeof(struct usb_ctrlrequest),
	       &reg->out_endp[EP0_CON].doeptsiz);
	writel(usb_ctrl_dma_addr, &reg->out_endp[EP0_CON].doepdma);

	ep_ctrl = readl(&reg->out_endp[EP0_CON].doepctl);
	writel(ep_ctrl|DEPCTL_EPENA, &reg->out_endp[EP0_CON].doepctl);

	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DIEPCTL0 = 0x%x\n",
		__func__, readl(&reg->in_endp[EP0_CON].diepctl));
	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DOEPCTL0 = 0x%x\n",
		__func__, readl(&reg->out_endp[EP0_CON].doepctl));

}

static inline void dwc2_ep0_complete_out(void)
{
	u32 ep_ctrl;

	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DIEPCTL0 = 0x%x\n",
		__func__, readl(&reg->in_endp[EP0_CON].diepctl));
	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DOEPCTL0 = 0x%x\n",
		__func__, readl(&reg->out_endp[EP0_CON].doepctl));

	debug_cond(DEBUG_IN_EP,
		"%s : Prepare Complete Out packet.\n", __func__);

	writel(DOEPT_SIZ_PKT_CNT(1) | sizeof(struct usb_ctrlrequest),
	       &reg->out_endp[EP0_CON].doeptsiz);
	writel(usb_ctrl_dma_addr, &reg->out_endp[EP0_CON].doepdma);

	ep_ctrl = readl(&reg->out_endp[EP0_CON].doepctl);
	writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK,
	       &reg->out_endp[EP0_CON].doepctl);

	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DIEPCTL0 = 0x%x\n",
		__func__, readl(&reg->in_endp[EP0_CON].diepctl));
	debug_cond(DEBUG_EP0 != 0, "%s:EP0 ZLP DOEPCTL0 = 0x%x\n",
		__func__, readl(&reg->out_endp[EP0_CON].doepctl));

}


static int setdma_rx(struct dwc2_ep *ep, struct dwc2_request *req)
{
	u32 *buf, ctrl;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep);

	buf = req->req.buf + req->req.actual;
	length = min_t(u32, req->req.length - req->req.actual,
		       ep_num ? DMA_BUFFER_SIZE : ep->ep.maxpacket);

	ep->len = length;
	ep->dma_buf = buf;

	if (ep_num == EP0_CON || length == 0)
		pktcnt = 1;
	else
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;

	ctrl =  readl(&reg->out_endp[ep_num].doepctl);

	invalidate_dcache_range((unsigned long) ep->dma_buf,
				(unsigned long) ep->dma_buf +
				ROUND(ep->len, CONFIG_SYS_CACHELINE_SIZE));

	writel((unsigned long) ep->dma_buf, &reg->out_endp[ep_num].doepdma);
	writel(DOEPT_SIZ_PKT_CNT(pktcnt) | DOEPT_SIZ_XFER_SIZE(length),
	       &reg->out_endp[ep_num].doeptsiz);
	writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, &reg->out_endp[ep_num].doepctl);

	debug_cond(DEBUG_OUT_EP != 0,
		   "%s: EP%d RX DMA start : DOEPDMA = 0x%x,"
		   "DOEPTSIZ = 0x%x, DOEPCTL = 0x%x\n"
		   "\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
		   __func__, ep_num,
		   readl(&reg->out_endp[ep_num].doepdma),
		   readl(&reg->out_endp[ep_num].doeptsiz),
		   readl(&reg->out_endp[ep_num].doepctl),
		   buf, pktcnt, length);
	return 0;

}

static int setdma_tx(struct dwc2_ep *ep, struct dwc2_request *req)
{
	u32 *buf, ctrl = 0;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep);

	buf = req->req.buf + req->req.actual;
	length = req->req.length - req->req.actual;

	if (ep_num == EP0_CON)
		length = min(length, (u32)ep_maxpacket(ep));

	ep->len = length;
	ep->dma_buf = buf;

	flush_dcache_range((unsigned long) ep->dma_buf,
			   (unsigned long) ep->dma_buf +
			   ROUND(ep->len, CONFIG_SYS_CACHELINE_SIZE));

	if (length == 0)
		pktcnt = 1;
	else
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;

	/* Flush the endpoint's Tx FIFO */
	writel(TX_FIFO_NUMBER(ep->fifo_num), &reg->grstctl);
	writel(TX_FIFO_NUMBER(ep->fifo_num) | TX_FIFO_FLUSH, &reg->grstctl);
	while (readl(&reg->grstctl) & TX_FIFO_FLUSH)
		;

	writel((unsigned long) ep->dma_buf, &reg->in_endp[ep_num].diepdma);
	writel(DIEPT_SIZ_PKT_CNT(pktcnt) | DIEPT_SIZ_XFER_SIZE(length),
	       &reg->in_endp[ep_num].dieptsiz);

	ctrl = readl(&reg->in_endp[ep_num].diepctl);

	/* Write the FIFO number to be used for this endpoint */
	ctrl &= DIEPCTL_TX_FIFO_NUM_MASK;
	ctrl |= DIEPCTL_TX_FIFO_NUM(ep->fifo_num);

	/* Clear reserved (Next EP) bits */
	ctrl = (ctrl&~(EP_MASK<<DEPCTL_NEXT_EP_BIT));

	writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, &reg->in_endp[ep_num].diepctl);

	debug_cond(DEBUG_IN_EP,
		"%s:EP%d TX DMA start : DIEPDMA0 = 0x%x,"
		"DIEPTSIZ0 = 0x%x, DIEPCTL0 = 0x%x\n"
		"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
		__func__, ep_num,
		readl(&reg->in_endp[ep_num].diepdma),
		readl(&reg->in_endp[ep_num].dieptsiz),
		readl(&reg->in_endp[ep_num].diepctl),
		buf, pktcnt, length);

	return length;
}

static void complete_rx(struct dwc2_udc *dev, u8 ep_num)
{
	struct dwc2_ep *ep = &dev->ep[ep_num];
	struct dwc2_request *req = NULL;
	u32 ep_tsr = 0, xfer_size = 0, is_short = 0;

	if (list_empty(&ep->queue)) {
		debug_cond(DEBUG_OUT_EP != 0,
			   "%s: RX DMA done : NULL REQ on OUT EP-%d\n",
			   __func__, ep_num);
		return;

	}

	req = list_entry(ep->queue.next, struct dwc2_request, queue);
	ep_tsr = readl(&reg->out_endp[ep_num].doeptsiz);

	if (ep_num == EP0_CON)
		xfer_size = (ep_tsr & DOEPT_SIZ_XFER_SIZE_MAX_EP0);
	else
		xfer_size = (ep_tsr & DOEPT_SIZ_XFER_SIZE_MAX_EP);

	xfer_size = ep->len - xfer_size;

	/*
	 * NOTE:
	 *
	 * Please be careful with proper buffer allocation for USB request,
	 * which needs to be aligned to CONFIG_SYS_CACHELINE_SIZE, not only
	 * with starting address, but also its size shall be a cache line
	 * multiplication.
	 *
	 * This will prevent from corruption of data allocated immediatelly
	 * before or after the buffer.
	 *
	 * For armv7, the cache_v7.c provides proper code to emit "ERROR"
	 * message to warn users.
	 */
	invalidate_dcache_range((unsigned long) ep->dma_buf,
				(unsigned long) ep->dma_buf +
				ROUND(xfer_size, CONFIG_SYS_CACHELINE_SIZE));

	req->req.actual += min(xfer_size, req->req.length - req->req.actual);
	is_short = !!(xfer_size % ep->ep.maxpacket);

	debug_cond(DEBUG_OUT_EP != 0,
		   "%s: RX DMA done : ep = %d, rx bytes = %d/%d, "
		   "is_short = %d, DOEPTSIZ = 0x%x, remained bytes = %d\n",
		   __func__, ep_num, req->req.actual, req->req.length,
		   is_short, ep_tsr, req->req.length - req->req.actual);

	if (is_short || req->req.actual == req->req.length) {
		if (ep_num == EP0_CON && dev->ep0state == DATA_STATE_RECV) {
			debug_cond(DEBUG_OUT_EP != 0, "	=> Send ZLP\n");
			dwc2_udc_ep0_zlp(dev);
			/* packet will be completed in complete_tx() */
			dev->ep0state = WAIT_FOR_IN_COMPLETE;
		} else {
			done(ep, req, 0);

			if (!list_empty(&ep->queue)) {
				req = list_entry(ep->queue.next,
					struct dwc2_request, queue);
				debug_cond(DEBUG_OUT_EP != 0,
					   "%s: Next Rx request start...\n",
					   __func__);
				setdma_rx(ep, req);
			}
		}
	} else
		setdma_rx(ep, req);
}

static void complete_tx(struct dwc2_udc *dev, u8 ep_num)
{
	struct dwc2_ep *ep = &dev->ep[ep_num];
	struct dwc2_request *req;
	u32 ep_tsr = 0, xfer_size = 0, is_short = 0;
	u32 last;

	if (dev->ep0state == WAIT_FOR_NULL_COMPLETE) {
		dev->ep0state = WAIT_FOR_OUT_COMPLETE;
		dwc2_ep0_complete_out();
		return;
	}

	if (list_empty(&ep->queue)) {
		debug_cond(DEBUG_IN_EP,
			"%s: TX DMA done : NULL REQ on IN EP-%d\n",
			__func__, ep_num);
		return;

	}

	req = list_entry(ep->queue.next, struct dwc2_request, queue);

	ep_tsr = readl(&reg->in_endp[ep_num].dieptsiz);

	xfer_size = ep->len;
	is_short = (xfer_size < ep->ep.maxpacket);
	req->req.actual += min(xfer_size, req->req.length - req->req.actual);

	debug_cond(DEBUG_IN_EP,
		"%s: TX DMA done : ep = %d, tx bytes = %d/%d, "
		"is_short = %d, DIEPTSIZ = 0x%x, remained bytes = %d\n",
		__func__, ep_num, req->req.actual, req->req.length,
		is_short, ep_tsr, req->req.length - req->req.actual);

	if (ep_num == 0) {
		if (dev->ep0state == DATA_STATE_XMIT) {
			debug_cond(DEBUG_IN_EP,
				"%s: ep_num = %d, ep0stat =="
				"DATA_STATE_XMIT\n",
				__func__, ep_num);
			last = write_fifo_ep0(ep, req);
			if (last)
				dev->ep0state = WAIT_FOR_COMPLETE;
		} else if (dev->ep0state == WAIT_FOR_IN_COMPLETE) {
			debug_cond(DEBUG_IN_EP,
				"%s: ep_num = %d, completing request\n",
				__func__, ep_num);
			done(ep, req, 0);
			dev->ep0state = WAIT_FOR_SETUP;
		} else if (dev->ep0state == WAIT_FOR_COMPLETE) {
			debug_cond(DEBUG_IN_EP,
				"%s: ep_num = %d, completing request\n",
				__func__, ep_num);
			done(ep, req, 0);
			dev->ep0state = WAIT_FOR_OUT_COMPLETE;
			dwc2_ep0_complete_out();
		} else {
			debug_cond(DEBUG_IN_EP,
				"%s: ep_num = %d, invalid ep state\n",
				__func__, ep_num);
		}
		return;
	}

	if (req->req.actual == req->req.length)
		done(ep, req, 0);

	if (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct dwc2_request, queue);
		debug_cond(DEBUG_IN_EP,
			"%s: Next Tx request start...\n", __func__);
		setdma_tx(ep, req);
	}
}

static inline void dwc2_udc_check_tx_queue(struct dwc2_udc *dev, u8 ep_num)
{
	struct dwc2_ep *ep = &dev->ep[ep_num];
	struct dwc2_request *req;

	debug_cond(DEBUG_IN_EP,
		"%s: Check queue, ep_num = %d\n", __func__, ep_num);

	if (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct dwc2_request, queue);
		debug_cond(DEBUG_IN_EP,
			"%s: Next Tx request(0x%p) start...\n",
			__func__, req);

		if (ep_is_in(ep))
			setdma_tx(ep, req);
		else
			setdma_rx(ep, req);
	} else {
		debug_cond(DEBUG_IN_EP,
			"%s: NULL REQ on IN EP-%d\n", __func__, ep_num);

		return;
	}

}

static void process_ep_in_intr(struct dwc2_udc *dev)
{
	u32 ep_intr, ep_intr_status;
	u8 ep_num = 0;

	ep_intr = readl(&reg->daint);
	debug_cond(DEBUG_IN_EP,
		"*** %s: EP In interrupt : DAINT = 0x%x\n", __func__, ep_intr);

	ep_intr &= DAINT_MASK;

	while (ep_intr) {
		if (ep_intr & DAINT_IN_EP_INT(1)) {
			ep_intr_status = readl(&reg->in_endp[ep_num].diepint);
			debug_cond(DEBUG_IN_EP,
				   "\tEP%d-IN : DIEPINT = 0x%x\n",
				   ep_num, ep_intr_status);

			/* Interrupt Clear */
			writel(ep_intr_status, &reg->in_endp[ep_num].diepint);

			if (ep_intr_status & TRANSFER_DONE) {
				complete_tx(dev, ep_num);

				if (ep_num == 0) {
					if (dev->ep0state ==
					    WAIT_FOR_IN_COMPLETE)
						dev->ep0state = WAIT_FOR_SETUP;

					if (dev->ep0state == WAIT_FOR_SETUP)
						dwc2_udc_pre_setup();

					/* continue transfer after
					   set_clear_halt for DMA mode */
					if (clear_feature_flag == 1) {
						dwc2_udc_check_tx_queue(dev,
							clear_feature_num);
						clear_feature_flag = 0;
					}
				}
			}
		}
		ep_num++;
		ep_intr >>= 1;
	}
}

static void process_ep_out_intr(struct dwc2_udc *dev)
{
	u32 ep_intr, ep_intr_status;
	u8 ep_num = 0;

	ep_intr = readl(&reg->daint);
	debug_cond(DEBUG_OUT_EP != 0,
		   "*** %s: EP OUT interrupt : DAINT = 0x%x\n",
		   __func__, ep_intr);

	ep_intr = (ep_intr >> DAINT_OUT_BIT) & DAINT_MASK;

	while (ep_intr) {
		if (ep_intr & 0x1) {
			ep_intr_status = readl(&reg->out_endp[ep_num].doepint);
			debug_cond(DEBUG_OUT_EP != 0,
				   "\tEP%d-OUT : DOEPINT = 0x%x\n",
				   ep_num, ep_intr_status);

			/* Interrupt Clear */
			writel(ep_intr_status, &reg->out_endp[ep_num].doepint);

			if (ep_num == 0) {
				if (ep_intr_status & TRANSFER_DONE) {
					if (dev->ep0state !=
					    WAIT_FOR_OUT_COMPLETE)
						complete_rx(dev, ep_num);
					else {
						dev->ep0state = WAIT_FOR_SETUP;
						dwc2_udc_pre_setup();
					}
				}

				if (ep_intr_status &
				    CTRL_OUT_EP_SETUP_PHASE_DONE) {
					debug_cond(DEBUG_OUT_EP != 0,
						   "SETUP packet arrived\n");
					dwc2_handle_ep0(dev);
				}
			} else {
				if (ep_intr_status & TRANSFER_DONE)
					complete_rx(dev, ep_num);
			}
		}
		ep_num++;
		ep_intr >>= 1;
	}
}

/*
 *	usb client interrupt handler.
 */
static int dwc2_udc_irq(int irq, void *_dev)
{
	struct dwc2_udc *dev = _dev;
	u32 intr_status, gotgint;
	u32 usb_status, gintmsk;
	unsigned long flags = 0;

	spin_lock_irqsave(&dev->lock, flags);

	intr_status = readl(&reg->gintsts);
	gintmsk = readl(&reg->gintmsk);

	debug_cond(DEBUG_ISR,
		  "\n*** %s : GINTSTS=0x%x(on state %s), GINTMSK : 0x%x,"
		  "DAINT : 0x%x, DAINTMSK : 0x%x\n",
		  __func__, intr_status, state_names[dev->ep0state], gintmsk,
		  readl(&reg->daint), readl(&reg->daintmsk));

	if (!intr_status) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return IRQ_HANDLED;
	}

	if (intr_status & INT_ENUMDONE) {
		debug_cond(DEBUG_ISR, "\tSpeed Detection interrupt\n");

		writel(INT_ENUMDONE, &reg->gintsts);
		usb_status = (readl(&reg->dsts) & 0x6);

		if (usb_status & (USB_FULL_30_60MHZ | USB_FULL_48MHZ)) {
			debug_cond(DEBUG_ISR,
				   "\t\tFull Speed Detection\n");
			set_max_pktsize(dev, USB_SPEED_FULL);

		} else {
			debug_cond(DEBUG_ISR,
				"\t\tHigh Speed Detection : 0x%x\n",
				usb_status);
			set_max_pktsize(dev, USB_SPEED_HIGH);
		}
	}

	if (intr_status & INT_EARLY_SUSPEND) {
		debug_cond(DEBUG_ISR, "\tEarly suspend interrupt\n");
		writel(INT_EARLY_SUSPEND, &reg->gintsts);
	}

	if (intr_status & INT_SUSPEND) {
		usb_status = readl(&reg->dsts);
		debug_cond(DEBUG_ISR,
			"\tSuspend interrupt :(DSTS):0x%x\n", usb_status);
		writel(INT_SUSPEND, &reg->gintsts);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver) {
			if (dev->driver->suspend)
				dev->driver->suspend(&dev->gadget);
		}
	}

	if (intr_status & INT_OTG) {
		gotgint = readl(&reg->gotgint);
		debug_cond(DEBUG_ISR,
			   "\tOTG interrupt: (GOTGINT):0x%x\n", gotgint);

		if (gotgint & GOTGINT_SES_END_DET) {
			debug_cond(DEBUG_ISR, "\t\tSession End Detected\n");
			/* Let gadget detect disconnected state */
			if (dev->driver->disconnect) {
				spin_unlock_irqrestore(&dev->lock, flags);
				dev->driver->disconnect(&dev->gadget);
				spin_lock_irqsave(&dev->lock, flags);
			}
		}
		writel(gotgint, &reg->gotgint);
	}

	if (intr_status & INT_RESUME) {
		debug_cond(DEBUG_ISR, "\tResume interrupt\n");
		writel(INT_RESUME, &reg->gintsts);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver
		    && dev->driver->resume) {

			dev->driver->resume(&dev->gadget);
		}
	}

	if (intr_status & INT_RESET) {
		usb_status = readl(&reg->gotgctl);
		debug_cond(DEBUG_ISR,
			"\tReset interrupt - (GOTGCTL):0x%x\n", usb_status);
		writel(INT_RESET, &reg->gintsts);

		if ((usb_status & 0xc0000) == (0x3 << 18)) {
			if (reset_available) {
				debug_cond(DEBUG_ISR,
					"\t\tOTG core got reset (%d)!!\n",
					reset_available);
				reconfig_usbd(dev);
				dev->ep0state = WAIT_FOR_SETUP;
				reset_available = 0;
				dwc2_udc_pre_setup();
			} else
				reset_available = 1;

		} else {
			reset_available = 1;
			debug_cond(DEBUG_ISR,
				   "\t\tRESET handling skipped\n");
		}
	}

	if (intr_status & INT_IN_EP)
		process_ep_in_intr(dev);

	if (intr_status & INT_OUT_EP)
		process_ep_out_intr(dev);

	spin_unlock_irqrestore(&dev->lock, flags);

	return IRQ_HANDLED;
}

/** Queue one request
 *  Kickstart transfer if needed
 */
static int dwc2_queue(struct usb_ep *_ep, struct usb_request *_req,
			 gfp_t gfp_flags)
{
	struct dwc2_request *req;
	struct dwc2_ep *ep;
	struct dwc2_udc *dev;
	unsigned long flags = 0;
	u32 ep_num, gintsts;

	req = container_of(_req, struct dwc2_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf
		     || !list_empty(&req->queue))) {

		debug("%s: bad params\n", __func__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct dwc2_ep, ep);

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {

		debug("%s: bad ep: %s, %d, %p\n", __func__,
		      ep->ep.name, !ep->desc, _ep);
		return -EINVAL;
	}

	ep_num = ep_index(ep);
	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)) {

		debug("%s: bogus device state %p\n", __func__, dev->driver);
		return -ESHUTDOWN;
	}

	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	debug("\n*** %s: %s-%s req = %p, len = %d, buf = %p"
		"Q empty = %d, stopped = %d\n",
		__func__, _ep->name, ep_is_in(ep) ? "in" : "out",
		_req, _req->length, _req->buf,
		list_empty(&ep->queue), ep->stopped);

#ifdef DEBUG
	{
		int i, len = _req->length;

		printf("pkt = ");
		if (len > 64)
			len = 64;
		for (i = 0; i < len; i++) {
			printf("%02x", ((u8 *)_req->buf)[i]);
			if ((i & 7) == 7)
				printf(" ");
		}
		printf("\n");
	}
#endif

	if (list_empty(&ep->queue) && !ep->stopped) {

		if (ep_num == 0) {
			/* EP0 */
			list_add_tail(&req->queue, &ep->queue);
			dwc2_ep0_kick(dev, ep);
			req = 0;

		} else if (ep_is_in(ep)) {
			gintsts = readl(&reg->gintsts);
			debug_cond(DEBUG_IN_EP,
				   "%s: ep_is_in, DWC2_UDC_OTG_GINTSTS=0x%x\n",
				   __func__, gintsts);

			setdma_tx(ep, req);
		} else {
			gintsts = readl(&reg->gintsts);
			debug_cond(DEBUG_OUT_EP != 0,
				   "%s:ep_is_out, DWC2_UDC_OTG_GINTSTS=0x%x\n",
				   __func__, gintsts);

			setdma_rx(ep, req);
		}
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != 0))
		list_add_tail(&req->queue, &ep->queue);

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/****************************************************************/
/* End Point 0 related functions                                */
/****************************************************************/

/* return:  0 = still running, 1 = completed, negative = errno */
static int write_fifo_ep0(struct dwc2_ep *ep, struct dwc2_request *req)
{
	u32 max;
	unsigned count;
	int is_last;

	max = ep_maxpacket(ep);

	debug_cond(DEBUG_EP0 != 0, "%s: max = %d\n", __func__, max);

	count = setdma_tx(ep, req);

	/* last packet is usually short (or a zlp) */
	if (likely(count != max))
		is_last = 1;
	else {
		if (likely(req->req.length != req->req.actual + count)
		    || req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}

	debug_cond(DEBUG_EP0 != 0,
		   "%s: wrote %s %d bytes%s %d left %p\n", __func__,
		   ep->ep.name, count,
		   is_last ? "/L" : "",
		   req->req.length - req->req.actual - count, req);

	/* requests complete when all IN data is in the FIFO */
	if (is_last) {
		ep->dev->ep0state = WAIT_FOR_SETUP;
		return 1;
	}

	return 0;
}

static int dwc2_fifo_read(struct dwc2_ep *ep, u32 *cp, int max)
{
	invalidate_dcache_range((unsigned long)cp, (unsigned long)cp +
				ROUND(max, CONFIG_SYS_CACHELINE_SIZE));

	debug_cond(DEBUG_EP0 != 0,
		   "%s: bytes=%d, ep_index=%d 0x%p\n", __func__,
		   max, ep_index(ep), cp);

	return max;
}

/**
 * udc_set_address - set the USB address for this device
 * @address:
 *
 * Called from control endpoint function
 * after it decodes a set address setup packet.
 */
static void udc_set_address(struct dwc2_udc *dev, unsigned char address)
{
	u32 ctrl = readl(&reg->dcfg);
	writel(DEVICE_ADDRESS(address) | ctrl, &reg->dcfg);

	dwc2_udc_ep0_zlp(dev);

	debug_cond(DEBUG_EP0 != 0,
		   "%s: USB OTG 2.0 Device address=%d, DCFG=0x%x\n",
		   __func__, address, readl(&reg->dcfg));

	dev->usb_address = address;
}

static inline void dwc2_udc_ep0_set_stall(struct dwc2_ep *ep)
{
	struct dwc2_udc *dev;
	u32		ep_ctrl = 0;

	dev = ep->dev;
	ep_ctrl = readl(&reg->in_endp[EP0_CON].diepctl);

	/* set the disable and stall bits */
	if (ep_ctrl & DEPCTL_EPENA)
		ep_ctrl |= DEPCTL_EPDIS;

	ep_ctrl |= DEPCTL_STALL;

	writel(ep_ctrl, &reg->in_endp[EP0_CON].diepctl);

	debug_cond(DEBUG_EP0 != 0,
		   "%s: set ep%d stall, DIEPCTL0 = 0x%p\n",
		   __func__, ep_index(ep), &reg->in_endp[EP0_CON].diepctl);
	/*
	 * The application can only set this bit, and the core clears it,
	 * when a SETUP token is received for this endpoint
	 */
	dev->ep0state = WAIT_FOR_SETUP;

	dwc2_udc_pre_setup();
}

static void dwc2_ep0_read(struct dwc2_udc *dev)
{
	struct dwc2_request *req;
	struct dwc2_ep *ep = &dev->ep[0];

	if (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct dwc2_request, queue);

	} else {
		debug("%s: ---> BUG\n", __func__);
		BUG();
		return;
	}

	debug_cond(DEBUG_EP0 != 0,
		   "%s: req = %p, req.length = 0x%x, req.actual = 0x%x\n",
		   __func__, req, req->req.length, req->req.actual);

	if (req->req.length == 0) {
		/* zlp for Set_configuration, Set_interface,
		 * or Bulk-Only mass storge reset */

		ep->len = 0;
		dwc2_udc_ep0_zlp(dev);

		debug_cond(DEBUG_EP0 != 0,
			   "%s: req.length = 0, bRequest = %d\n",
			   __func__, usb_ctrl->bRequest);
		return;
	}

	setdma_rx(ep, req);
}

/*
 * DATA_STATE_XMIT
 */
static int dwc2_ep0_write(struct dwc2_udc *dev)
{
	struct dwc2_request *req;
	struct dwc2_ep *ep = &dev->ep[0];
	int ret, need_zlp = 0;

	if (list_empty(&ep->queue))
		req = 0;
	else
		req = list_entry(ep->queue.next, struct dwc2_request, queue);

	if (!req) {
		debug_cond(DEBUG_EP0 != 0, "%s: NULL REQ\n", __func__);
		return 0;
	}

	debug_cond(DEBUG_EP0 != 0,
		   "%s: req = %p, req.length = 0x%x, req.actual = 0x%x\n",
		   __func__, req, req->req.length, req->req.actual);

	if (req->req.length - req->req.actual == ep0_fifo_size) {
		/* Next write will end with the packet size, */
		/* so we need Zero-length-packet */
		need_zlp = 1;
	}

	ret = write_fifo_ep0(ep, req);

	if ((ret == 1) && !need_zlp) {
		/* Last packet */
		dev->ep0state = WAIT_FOR_COMPLETE;
		debug_cond(DEBUG_EP0 != 0,
			   "%s: finished, waiting for status\n", __func__);

	} else {
		dev->ep0state = DATA_STATE_XMIT;
		debug_cond(DEBUG_EP0 != 0,
			   "%s: not finished\n", __func__);
	}

	return 1;
}

static int dwc2_udc_get_status(struct dwc2_udc *dev,
		struct usb_ctrlrequest *crq)
{
	u8 ep_num = crq->wIndex & 0x7F;
	u16 g_status = 0;
	u32 ep_ctrl;

	debug_cond(DEBUG_SETUP != 0,
		   "%s: *** USB_REQ_GET_STATUS\n", __func__);
	printf("crq->brequest:0x%x\n", crq->bRequestType & USB_RECIP_MASK);
	switch (crq->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_INTERFACE:
		g_status = 0;
		debug_cond(DEBUG_SETUP != 0,
			   "\tGET_STATUS:USB_RECIP_INTERFACE, g_stauts = %d\n",
			   g_status);
		break;

	case USB_RECIP_DEVICE:
		g_status = 0x1; /* Self powered */
		debug_cond(DEBUG_SETUP != 0,
			   "\tGET_STATUS: USB_RECIP_DEVICE, g_stauts = %d\n",
			   g_status);
		break;

	case USB_RECIP_ENDPOINT:
		if (crq->wLength > 2) {
			debug_cond(DEBUG_SETUP != 0,
				   "\tGET_STATUS:Not support EP or wLength\n");
			return 1;
		}

		g_status = dev->ep[ep_num].stopped;
		debug_cond(DEBUG_SETUP != 0,
			   "\tGET_STATUS: USB_RECIP_ENDPOINT, g_stauts = %d\n",
			   g_status);

		break;

	default:
		return 1;
	}

	memcpy(usb_ctrl, &g_status, sizeof(g_status));

	flush_dcache_range((unsigned long) usb_ctrl,
			   (unsigned long) usb_ctrl +
			   ROUND(sizeof(g_status), CONFIG_SYS_CACHELINE_SIZE));

	writel(usb_ctrl_dma_addr, &reg->in_endp[EP0_CON].diepdma);
	writel(DIEPT_SIZ_PKT_CNT(1) | DIEPT_SIZ_XFER_SIZE(2),
	       &reg->in_endp[EP0_CON].dieptsiz);

	ep_ctrl = readl(&reg->in_endp[EP0_CON].diepctl);
	writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK,
	       &reg->in_endp[EP0_CON].diepctl);
	dev->ep0state = WAIT_FOR_NULL_COMPLETE;

	return 0;
}

static void dwc2_udc_set_nak(struct dwc2_ep *ep)
{
	u8		ep_num;
	u32		ep_ctrl = 0;

	ep_num = ep_index(ep);
	debug("%s: ep_num = %d, ep_type = %d\n", __func__, ep_num, ep->ep_type);

	if (ep_is_in(ep)) {
		ep_ctrl = readl(&reg->in_endp[ep_num].diepctl);
		ep_ctrl |= DEPCTL_SNAK;
		writel(ep_ctrl, &reg->in_endp[ep_num].diepctl);
		debug("%s: set NAK, DIEPCTL%d = 0x%x\n",
			__func__, ep_num, readl(&reg->in_endp[ep_num].diepctl));
	} else {
		ep_ctrl = readl(&reg->out_endp[ep_num].doepctl);
		ep_ctrl |= DEPCTL_SNAK;
		writel(ep_ctrl, &reg->out_endp[ep_num].doepctl);
		debug("%s: set NAK, DOEPCTL%d = 0x%x\n",
		      __func__, ep_num, readl(&reg->out_endp[ep_num].doepctl));
	}

	return;
}


static void dwc2_udc_ep_set_stall(struct dwc2_ep *ep)
{
	u8		ep_num;
	u32		ep_ctrl = 0;

	ep_num = ep_index(ep);
	debug("%s: ep_num = %d, ep_type = %d\n", __func__, ep_num, ep->ep_type);

	if (ep_is_in(ep)) {
		ep_ctrl = readl(&reg->in_endp[ep_num].diepctl);

		/* set the disable and stall bits */
		if (ep_ctrl & DEPCTL_EPENA)
			ep_ctrl |= DEPCTL_EPDIS;

		ep_ctrl |= DEPCTL_STALL;

		writel(ep_ctrl, &reg->in_endp[ep_num].diepctl);
		debug("%s: set stall, DIEPCTL%d = 0x%x\n",
		      __func__, ep_num, readl(&reg->in_endp[ep_num].diepctl));

	} else {
		ep_ctrl = readl(&reg->out_endp[ep_num].doepctl);

		/* set the stall bit */
		ep_ctrl |= DEPCTL_STALL;

		writel(ep_ctrl, &reg->out_endp[ep_num].doepctl);
		debug("%s: set stall, DOEPCTL%d = 0x%x\n",
		      __func__, ep_num, readl(&reg->out_endp[ep_num].doepctl));
	}

	return;
}

static void dwc2_udc_ep_clear_stall(struct dwc2_ep *ep)
{
	u8		ep_num;
	u32		ep_ctrl = 0;

	ep_num = ep_index(ep);
	debug("%s: ep_num = %d, ep_type = %d\n", __func__, ep_num, ep->ep_type);

	if (ep_is_in(ep)) {
		ep_ctrl = readl(&reg->in_endp[ep_num].diepctl);

		/* clear stall bit */
		ep_ctrl &= ~DEPCTL_STALL;

		/*
		 * USB Spec 9.4.5: For endpoints using data toggle, regardless
		 * of whether an endpoint has the Halt feature set, a
		 * ClearFeature(ENDPOINT_HALT) request always results in the
		 * data toggle being reinitialized to DATA0.
		 */
		if (ep->bmAttributes == USB_ENDPOINT_XFER_INT
		    || ep->bmAttributes == USB_ENDPOINT_XFER_BULK) {
			ep_ctrl |= DEPCTL_SETD0PID; /* DATA0 */
		}

		writel(ep_ctrl, &reg->in_endp[ep_num].diepctl);
		debug("%s: cleared stall, DIEPCTL%d = 0x%x\n",
			__func__, ep_num, readl(&reg->in_endp[ep_num].diepctl));

	} else {
		ep_ctrl = readl(&reg->out_endp[ep_num].doepctl);

		/* clear stall bit */
		ep_ctrl &= ~DEPCTL_STALL;

		if (ep->bmAttributes == USB_ENDPOINT_XFER_INT
		    || ep->bmAttributes == USB_ENDPOINT_XFER_BULK) {
			ep_ctrl |= DEPCTL_SETD0PID; /* DATA0 */
		}

		writel(ep_ctrl, &reg->out_endp[ep_num].doepctl);
		debug("%s: cleared stall, DOEPCTL%d = 0x%x\n",
		      __func__, ep_num, readl(&reg->out_endp[ep_num].doepctl));
	}

	return;
}

static int dwc2_udc_set_halt(struct usb_ep *_ep, int value)
{
	struct dwc2_ep	*ep;
	struct dwc2_udc	*dev;
	unsigned long	flags = 0;
	u8		ep_num;

	ep = container_of(_ep, struct dwc2_ep, ep);
	ep_num = ep_index(ep);

	if (unlikely(!_ep || !ep->desc || ep_num == EP0_CON ||
		     ep->desc->bmAttributes == USB_ENDPOINT_XFER_ISOC)) {
		debug("%s: %s bad ep or descriptor\n", __func__, ep->ep.name);
		return -EINVAL;
	}

	/* Attempt to halt IN ep will fail if any transfer requests
	 * are still queue */
	if (value && ep_is_in(ep) && !list_empty(&ep->queue)) {
		debug("%s: %s queue not empty, req = %p\n",
			__func__, ep->ep.name,
			list_entry(ep->queue.next, struct dwc2_request, queue));

		return -EAGAIN;
	}

	dev = ep->dev;
	debug("%s: ep_num = %d, value = %d\n", __func__, ep_num, value);

	spin_lock_irqsave(&dev->lock, flags);

	if (value == 0) {
		ep->stopped = 0;
		dwc2_udc_ep_clear_stall(ep);
	} else {
		if (ep_num == 0)
			dev->ep0state = WAIT_FOR_SETUP;

		ep->stopped = 1;
		dwc2_udc_ep_set_stall(ep);
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

static void dwc2_udc_ep_activate(struct dwc2_ep *ep)
{
	u8 ep_num;
	u32 ep_ctrl = 0, daintmsk = 0;

	ep_num = ep_index(ep);

	/* Read DEPCTLn register */
	if (ep_is_in(ep)) {
		ep_ctrl = readl(&reg->in_endp[ep_num].diepctl);
		daintmsk = 1 << ep_num;
	} else {
		ep_ctrl = readl(&reg->out_endp[ep_num].doepctl);
		daintmsk = (1 << ep_num) << DAINT_OUT_BIT;
	}

	debug("%s: EPCTRL%d = 0x%x, ep_is_in = %d\n",
		__func__, ep_num, ep_ctrl, ep_is_in(ep));

	/* If the EP is already active don't change the EP Control
	 * register. */
	if (!(ep_ctrl & DEPCTL_USBACTEP)) {
		ep_ctrl = (ep_ctrl & ~DEPCTL_TYPE_MASK) |
			(ep->bmAttributes << DEPCTL_TYPE_BIT);
		ep_ctrl = (ep_ctrl & ~DEPCTL_MPS_MASK) |
			(ep->ep.maxpacket << DEPCTL_MPS_BIT);
		ep_ctrl |= (DEPCTL_SETD0PID | DEPCTL_USBACTEP | DEPCTL_SNAK);

		if (ep_is_in(ep)) {
			writel(ep_ctrl, &reg->in_endp[ep_num].diepctl);
			debug("%s: USB Ative EP%d, DIEPCTRL%d = 0x%x\n",
			      __func__, ep_num, ep_num,
			      readl(&reg->in_endp[ep_num].diepctl));
		} else {
			writel(ep_ctrl, &reg->out_endp[ep_num].doepctl);
			debug("%s: USB Ative EP%d, DOEPCTRL%d = 0x%x\n",
			      __func__, ep_num, ep_num,
			      readl(&reg->out_endp[ep_num].doepctl));
		}
	}

	/* Unmask EP Interrtupt */
	writel(readl(&reg->daintmsk)|daintmsk, &reg->daintmsk);
	debug("%s: DAINTMSK = 0x%x\n", __func__, readl(&reg->daintmsk));

}

static int dwc2_udc_clear_feature(struct usb_ep *_ep)
{
	struct dwc2_udc	*dev;
	struct dwc2_ep	*ep;
	u8		ep_num;

	ep = container_of(_ep, struct dwc2_ep, ep);
	ep_num = ep_index(ep);

	dev = ep->dev;
	debug_cond(DEBUG_SETUP != 0,
		   "%s: ep_num = %d, is_in = %d, clear_feature_flag = %d\n",
		   __func__, ep_num, ep_is_in(ep), clear_feature_flag);

	if (usb_ctrl->wLength != 0) {
		debug_cond(DEBUG_SETUP != 0,
			   "\tCLEAR_FEATURE: wLength is not zero.....\n");
		return 1;
	}

	switch (usb_ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		switch (usb_ctrl->wValue) {
		case USB_DEVICE_REMOTE_WAKEUP:
			debug_cond(DEBUG_SETUP != 0,
				   "\tOFF:USB_DEVICE_REMOTE_WAKEUP\n");
			break;

		case USB_DEVICE_TEST_MODE:
			debug_cond(DEBUG_SETUP != 0,
				   "\tCLEAR_FEATURE: USB_DEVICE_TEST_MODE\n");
			/** @todo Add CLEAR_FEATURE for TEST modes. */
			break;
		}

		dwc2_udc_ep0_zlp(dev);
		break;

	case USB_RECIP_ENDPOINT:
		debug_cond(DEBUG_SETUP != 0,
			   "\tCLEAR_FEATURE:USB_RECIP_ENDPOINT, wValue = %d\n",
			   usb_ctrl->wValue);

		if (usb_ctrl->wValue == USB_ENDPOINT_HALT) {
			if (ep_num == 0) {
				dwc2_udc_ep0_set_stall(ep);
				return 0;
			}

			dwc2_udc_ep0_zlp(dev);

			dwc2_udc_ep_clear_stall(ep);
			dwc2_udc_ep_activate(ep);
			ep->stopped = 0;

			clear_feature_num = ep_num;
			clear_feature_flag = 1;
		}
		break;
	}

	return 0;
}

static int dwc2_udc_set_feature(struct usb_ep *_ep)
{
	struct dwc2_udc	*dev;
	struct dwc2_ep	*ep;
	u8		ep_num;

	ep = container_of(_ep, struct dwc2_ep, ep);
	ep_num = ep_index(ep);
	dev = ep->dev;

	debug_cond(DEBUG_SETUP != 0,
		   "%s: *** USB_REQ_SET_FEATURE , ep_num = %d\n",
		    __func__, ep_num);

	if (usb_ctrl->wLength != 0) {
		debug_cond(DEBUG_SETUP != 0,
			   "\tSET_FEATURE: wLength is not zero.....\n");
		return 1;
	}

	switch (usb_ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		switch (usb_ctrl->wValue) {
		case USB_DEVICE_REMOTE_WAKEUP:
			debug_cond(DEBUG_SETUP != 0,
				   "\tSET_FEATURE:USB_DEVICE_REMOTE_WAKEUP\n");
			break;
		case USB_DEVICE_B_HNP_ENABLE:
			debug_cond(DEBUG_SETUP != 0,
				   "\tSET_FEATURE: USB_DEVICE_B_HNP_ENABLE\n");
			break;

		case USB_DEVICE_A_HNP_SUPPORT:
			/* RH port supports HNP */
			debug_cond(DEBUG_SETUP != 0,
				   "\tSET_FEATURE:USB_DEVICE_A_HNP_SUPPORT\n");
			break;

		case USB_DEVICE_A_ALT_HNP_SUPPORT:
			/* other RH port does */
			debug_cond(DEBUG_SETUP != 0,
				   "\tSET: USB_DEVICE_A_ALT_HNP_SUPPORT\n");
			break;
		}

		dwc2_udc_ep0_zlp(dev);
		return 0;

	case USB_RECIP_INTERFACE:
		debug_cond(DEBUG_SETUP != 0,
			   "\tSET_FEATURE: USB_RECIP_INTERFACE\n");
		break;

	case USB_RECIP_ENDPOINT:
		debug_cond(DEBUG_SETUP != 0,
			   "\tSET_FEATURE: USB_RECIP_ENDPOINT\n");
		if (usb_ctrl->wValue == USB_ENDPOINT_HALT) {
			if (ep_num == 0) {
				dwc2_udc_ep0_set_stall(ep);
				return 0;
			}
			ep->stopped = 1;
			dwc2_udc_ep_set_stall(ep);
		}

		dwc2_udc_ep0_zlp(dev);
		return 0;
	}

	return 1;
}

/*
 * WAIT_FOR_SETUP (OUT_PKT_RDY)
 */
static void dwc2_ep0_setup(struct dwc2_udc *dev)
{
	struct dwc2_ep *ep = &dev->ep[0];
	int i;
	u8 ep_num;

	/* Nuke all previous transfers */
	nuke(ep, -EPROTO);

	/* read control req from fifo (8 bytes) */
	dwc2_fifo_read(ep, (u32 *)usb_ctrl, 8);

	debug_cond(DEBUG_SETUP != 0,
		   "%s: bRequestType = 0x%x(%s), bRequest = 0x%x"
		   "\twLength = 0x%x, wValue = 0x%x, wIndex= 0x%x\n",
		   __func__, usb_ctrl->bRequestType,
		   (usb_ctrl->bRequestType & USB_DIR_IN) ? "IN" : "OUT",
		   usb_ctrl->bRequest,
		   usb_ctrl->wLength, usb_ctrl->wValue, usb_ctrl->wIndex);

#ifdef DEBUG
	{
		int i, len = sizeof(*usb_ctrl);
		char *p = (char *)usb_ctrl;

		printf("pkt = ");
		for (i = 0; i < len; i++) {
			printf("%02x", ((u8 *)p)[i]);
			if ((i & 7) == 7)
				printf(" ");
		}
		printf("\n");
	}
#endif

	if (usb_ctrl->bRequest == GET_MAX_LUN_REQUEST &&
	    usb_ctrl->wLength != 1) {
		debug_cond(DEBUG_SETUP != 0,
			   "\t%s:GET_MAX_LUN_REQUEST:invalid",
			   __func__);
		debug_cond(DEBUG_SETUP != 0,
			   "wLength = %d, setup returned\n",
			   usb_ctrl->wLength);

		dwc2_udc_ep0_set_stall(ep);
		dev->ep0state = WAIT_FOR_SETUP;

		return;
	} else if (usb_ctrl->bRequest == BOT_RESET_REQUEST &&
		 usb_ctrl->wLength != 0) {
		/* Bulk-Only *mass storge reset of class-specific request */
		debug_cond(DEBUG_SETUP != 0,
			   "%s:BOT Rest:invalid wLength =%d, setup returned\n",
			   __func__, usb_ctrl->wLength);

		dwc2_udc_ep0_set_stall(ep);
		dev->ep0state = WAIT_FOR_SETUP;

		return;
	}

	/* Set direction of EP0 */
	if (likely(usb_ctrl->bRequestType & USB_DIR_IN)) {
		ep->bEndpointAddress |= USB_DIR_IN;
	} else {
		ep->bEndpointAddress &= ~USB_DIR_IN;
	}
	/* cope with automagic for some standard requests. */
	dev->req_std = (usb_ctrl->bRequestType & USB_TYPE_MASK)
		== USB_TYPE_STANDARD;

	dev->req_pending = 1;

	/* Handle some SETUP packets ourselves */
	if (dev->req_std) {
		switch (usb_ctrl->bRequest) {
		case USB_REQ_SET_ADDRESS:
		debug_cond(DEBUG_SETUP != 0,
			   "%s: *** USB_REQ_SET_ADDRESS (%d)\n",
			   __func__, usb_ctrl->wValue);
			if (usb_ctrl->bRequestType
				!= (USB_TYPE_STANDARD | USB_RECIP_DEVICE))
				break;

			udc_set_address(dev, usb_ctrl->wValue);
			return;

		case USB_REQ_SET_CONFIGURATION:
			debug_cond(DEBUG_SETUP != 0,
				   "=====================================\n");
			debug_cond(DEBUG_SETUP != 0,
				   "%s: USB_REQ_SET_CONFIGURATION (%d)\n",
				   __func__, usb_ctrl->wValue);

			if (usb_ctrl->bRequestType == USB_RECIP_DEVICE)
				reset_available = 1;

			break;

		case USB_REQ_GET_DESCRIPTOR:
			debug_cond(DEBUG_SETUP != 0,
				   "%s: *** USB_REQ_GET_DESCRIPTOR\n",
				   __func__);
			break;

		case USB_REQ_SET_INTERFACE:
			debug_cond(DEBUG_SETUP != 0,
				   "%s: *** USB_REQ_SET_INTERFACE (%d)\n",
				   __func__, usb_ctrl->wValue);

			if (usb_ctrl->bRequestType == USB_RECIP_INTERFACE)
				reset_available = 1;

			break;

		case USB_REQ_GET_CONFIGURATION:
			debug_cond(DEBUG_SETUP != 0,
				   "%s: *** USB_REQ_GET_CONFIGURATION\n",
				   __func__);
			break;

		case USB_REQ_GET_STATUS:
			if (!dwc2_udc_get_status(dev, usb_ctrl))
				return;

			break;

		case USB_REQ_CLEAR_FEATURE:
			ep_num = usb_ctrl->wIndex & 0x7f;

			if (!dwc2_udc_clear_feature(&dev->ep[ep_num].ep))
				return;

			break;

		case USB_REQ_SET_FEATURE:
			ep_num = usb_ctrl->wIndex & 0x7f;

			if (!dwc2_udc_set_feature(&dev->ep[ep_num].ep))
				return;

			break;

		default:
			debug_cond(DEBUG_SETUP != 0,
				   "%s: *** Default of usb_ctrl->bRequest=0x%x"
				   "happened.\n", __func__, usb_ctrl->bRequest);
			break;
		}
	}


	if (likely(dev->driver)) {
		/* device-2-host (IN) or no data setup command,
		 * process immediately */
		debug_cond(DEBUG_SETUP != 0,
			   "%s:usb_ctrlreq will be passed to fsg_setup()\n",
			    __func__);

		spin_unlock(&dev->lock);
		i = dev->driver->setup(&dev->gadget, usb_ctrl);
		spin_lock(&dev->lock);

		if (i < 0) {
			/* setup processing failed, force stall */
			dwc2_udc_ep0_set_stall(ep);
			dev->ep0state = WAIT_FOR_SETUP;

			debug_cond(DEBUG_SETUP != 0,
				   "\tdev->driver->setup failed (%d),"
				    " bRequest = %d\n",
				i, usb_ctrl->bRequest);


		} else if (dev->req_pending) {
			dev->req_pending = 0;
			debug_cond(DEBUG_SETUP != 0,
				   "\tdev->req_pending...\n");
		}

		debug_cond(DEBUG_SETUP != 0,
			   "\tep0state = %s\n", state_names[dev->ep0state]);

	}
}

/*
 * handle ep0 interrupt
 */
static void dwc2_handle_ep0(struct dwc2_udc *dev)
{
	if (dev->ep0state == WAIT_FOR_SETUP) {
		debug_cond(DEBUG_OUT_EP != 0,
			   "%s: WAIT_FOR_SETUP\n", __func__);
		dwc2_ep0_setup(dev);

	} else {
		debug_cond(DEBUG_OUT_EP != 0,
			   "%s: strange state!!(state = %s)\n",
			__func__, state_names[dev->ep0state]);
	}
}

static void dwc2_ep0_kick(struct dwc2_udc *dev, struct dwc2_ep *ep)
{
	debug_cond(DEBUG_EP0 != 0,
		   "%s: ep_is_in = %d\n", __func__, ep_is_in(ep));
	if (ep_is_in(ep)) {
		dev->ep0state = DATA_STATE_XMIT;
		dwc2_ep0_write(dev);

	} else {
		dev->ep0state = DATA_STATE_RECV;
		dwc2_ep0_read(dev);
	}
}
