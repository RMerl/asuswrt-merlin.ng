// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday USB 2.0 OTG Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <net.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <usb/fotg210.h>

#define CFG_NUM_ENDPOINTS		4
#define CFG_EP0_MAX_PACKET_SIZE	64
#define CFG_EPX_MAX_PACKET_SIZE	512

#define CFG_CMD_TIMEOUT (CONFIG_SYS_HZ >> 2) /* 250 ms */

struct fotg210_chip;

struct fotg210_ep {
	struct usb_ep ep;

	uint maxpacket;
	uint id;
	uint stopped;

	struct list_head                      queue;
	struct fotg210_chip                  *chip;
	const struct usb_endpoint_descriptor *desc;
};

struct fotg210_request {
	struct usb_request req;
	struct list_head   queue;
	struct fotg210_ep *ep;
};

struct fotg210_chip {
	struct usb_gadget         gadget;
	struct usb_gadget_driver *driver;
	struct fotg210_regs      *regs;
	uint8_t                   irq;
	uint16_t                  addr;
	int                       pullup;
	enum usb_device_state     state;
	struct fotg210_ep         ep[1 + CFG_NUM_ENDPOINTS];
};

static struct usb_endpoint_descriptor ep0_desc = {
	.bLength = sizeof(struct usb_endpoint_descriptor),
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
};

static inline int fifo_to_ep(struct fotg210_chip *chip, int id, int in)
{
	return (id < 0) ? 0 : ((id & 0x03) + 1);
}

static inline int ep_to_fifo(struct fotg210_chip *chip, int id)
{
	return (id <= 0) ? -1 : ((id - 1) & 0x03);
}

static inline int ep_reset(struct fotg210_chip *chip, uint8_t ep_addr)
{
	int ep = ep_addr & USB_ENDPOINT_NUMBER_MASK;
	struct fotg210_regs *regs = chip->regs;

	if (ep_addr & USB_DIR_IN) {
		/* reset endpoint */
		setbits_le32(&regs->iep[ep - 1], IEP_RESET);
		mdelay(1);
		clrbits_le32(&regs->iep[ep - 1], IEP_RESET);
		/* clear endpoint stall */
		clrbits_le32(&regs->iep[ep - 1], IEP_STALL);
	} else {
		/* reset endpoint */
		setbits_le32(&regs->oep[ep - 1], OEP_RESET);
		mdelay(1);
		clrbits_le32(&regs->oep[ep - 1], OEP_RESET);
		/* clear endpoint stall */
		clrbits_le32(&regs->oep[ep - 1], OEP_STALL);
	}

	return 0;
}

static int fotg210_reset(struct fotg210_chip *chip)
{
	struct fotg210_regs *regs = chip->regs;
	uint32_t i;

	chip->state = USB_STATE_POWERED;

	/* chip enable */
	writel(DEVCTRL_EN, &regs->dev_ctrl);

	/* device address reset */
	chip->addr = 0;
	writel(0, &regs->dev_addr);

	/* set idle counter to 7ms */
	writel(7, &regs->idle);

	/* disable all interrupts */
	writel(IMR_MASK, &regs->imr);
	writel(GIMR_MASK, &regs->gimr);
	writel(GIMR0_MASK, &regs->gimr0);
	writel(GIMR1_MASK, &regs->gimr1);
	writel(GIMR2_MASK, &regs->gimr2);

	/* clear interrupts */
	writel(ISR_MASK, &regs->isr);
	writel(0, &regs->gisr);
	writel(0, &regs->gisr0);
	writel(0, &regs->gisr1);
	writel(0, &regs->gisr2);

	/* chip reset */
	setbits_le32(&regs->dev_ctrl, DEVCTRL_RESET);
	mdelay(10);
	if (readl(&regs->dev_ctrl) & DEVCTRL_RESET) {
		printf("fotg210: chip reset failed\n");
		return -1;
	}

	/* CX FIFO reset */
	setbits_le32(&regs->cxfifo, CXFIFO_CXFIFOCLR);
	mdelay(10);
	if (readl(&regs->cxfifo) & CXFIFO_CXFIFOCLR) {
		printf("fotg210: ep0 fifo reset failed\n");
		return -1;
	}

	/* create static ep-fifo map (EP1 <-> FIFO0, EP2 <-> FIFO1 ...) */
	writel(EPMAP14_DEFAULT, &regs->epmap14);
	writel(EPMAP58_DEFAULT, &regs->epmap58);
	writel(FIFOMAP_DEFAULT, &regs->fifomap);
	writel(0, &regs->fifocfg);
	for (i = 0; i < 8; ++i) {
		writel(CFG_EPX_MAX_PACKET_SIZE, &regs->iep[i]);
		writel(CFG_EPX_MAX_PACKET_SIZE, &regs->oep[i]);
	}

	/* FIFO reset */
	for (i = 0; i < 4; ++i) {
		writel(FIFOCSR_RESET, &regs->fifocsr[i]);
		mdelay(10);
		if (readl(&regs->fifocsr[i]) & FIFOCSR_RESET) {
			printf("fotg210: fifo%d reset failed\n", i);
			return -1;
		}
	}

	/* enable only device interrupt and triggered at level-high */
	writel(IMR_IRQLH | IMR_HOST | IMR_OTG, &regs->imr);
	writel(ISR_MASK, &regs->isr);
	/* disable EP0 IN/OUT interrupt */
	writel(GIMR0_CXOUT | GIMR0_CXIN, &regs->gimr0);
	/* disable EPX IN+SPK+OUT interrupts */
	writel(GIMR1_MASK, &regs->gimr1);
	/* disable wakeup+idle+dma+zlp interrupts */
	writel(GIMR2_WAKEUP | GIMR2_IDLE | GIMR2_DMAERR | GIMR2_DMAFIN
		| GIMR2_ZLPRX | GIMR2_ZLPTX, &regs->gimr2);
	/* enable all group interrupt */
	writel(0, &regs->gimr);

	/* suspend delay = 3 ms */
	writel(3, &regs->idle);

	/* turn-on device interrupts */
	setbits_le32(&regs->dev_ctrl, DEVCTRL_GIRQ_EN);

	return 0;
}

static inline int fotg210_cxwait(struct fotg210_chip *chip, uint32_t mask)
{
	struct fotg210_regs *regs = chip->regs;
	int ret = -1;
	ulong ts;

	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		if ((readl(&regs->cxfifo) & mask) != mask)
			continue;
		ret = 0;
		break;
	}

	if (ret)
		printf("fotg210: cx/ep0 timeout\n");

	return ret;
}

static int fotg210_dma(struct fotg210_ep *ep, struct fotg210_request *req)
{
	struct fotg210_chip *chip = ep->chip;
	struct fotg210_regs *regs = chip->regs;
	uint32_t tmp, ts;
	uint8_t *buf  = req->req.buf + req->req.actual;
	uint32_t len  = req->req.length - req->req.actual;
	int fifo = ep_to_fifo(chip, ep->id);
	int ret = -EBUSY;

	/* 1. init dma buffer */
	if (len > ep->maxpacket)
		len = ep->maxpacket;

	/* 2. wait for dma ready (hardware) */
	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		if (!(readl(&regs->dma_ctrl) & DMACTRL_START)) {
			ret = 0;
			break;
		}
	}
	if (ret) {
		printf("fotg210: dma busy\n");
		req->req.status = ret;
		return ret;
	}

	/* 3. DMA target setup */
	if (ep->desc->bEndpointAddress & USB_DIR_IN)
		flush_dcache_range((ulong)buf, (ulong)buf + len);
	else
		invalidate_dcache_range((ulong)buf, (ulong)buf + len);

	writel(virt_to_phys(buf), &regs->dma_addr);

	if (ep->desc->bEndpointAddress & USB_DIR_IN) {
		if (ep->id == 0) {
			/* Wait until cx/ep0 fifo empty */
			fotg210_cxwait(chip, CXFIFO_CXFIFOE);
			udelay(1);
			writel(DMAFIFO_CX, &regs->dma_fifo);
		} else {
			/* Wait until epx fifo empty */
			fotg210_cxwait(chip, CXFIFO_FIFOE(fifo));
			writel(DMAFIFO_FIFO(fifo), &regs->dma_fifo);
		}
		writel(DMACTRL_LEN(len) | DMACTRL_MEM2FIFO, &regs->dma_ctrl);
	} else {
		uint32_t blen;

		if (ep->id == 0) {
			writel(DMAFIFO_CX, &regs->dma_fifo);
			do {
				blen = CXFIFO_BYTES(readl(&regs->cxfifo));
			} while (blen < len);
		} else {
			writel(DMAFIFO_FIFO(fifo), &regs->dma_fifo);
			blen = FIFOCSR_BYTES(readl(&regs->fifocsr[fifo]));
		}
		len  = (len < blen) ? len : blen;
		writel(DMACTRL_LEN(len) | DMACTRL_FIFO2MEM, &regs->dma_ctrl);
	}

	/* 4. DMA start */
	setbits_le32(&regs->dma_ctrl, DMACTRL_START);

	/* 5. DMA wait */
	ret = -EBUSY;
	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		tmp = readl(&regs->gisr2);
		/* DMA complete */
		if (tmp & GISR2_DMAFIN) {
			ret = 0;
			break;
		}
		/* DMA error */
		if (tmp & GISR2_DMAERR) {
			printf("fotg210: dma error\n");
			break;
		}
		/* resume, suspend, reset */
		if (tmp & (GISR2_RESUME | GISR2_SUSPEND | GISR2_RESET)) {
			printf("fotg210: dma reset by host\n");
			break;
		}
	}

	/* 7. DMA target reset */
	if (ret)
		writel(DMACTRL_ABORT | DMACTRL_CLRFF, &regs->dma_ctrl);

	writel(0, &regs->gisr2);
	writel(0, &regs->dma_fifo);

	req->req.status = ret;
	if (!ret)
		req->req.actual += len;
	else
		printf("fotg210: ep%d dma error(code=%d)\n", ep->id, ret);

	return len;
}

/*
 * result of setup packet
 */
#define CX_IDLE		0
#define CX_FINISH	1
#define CX_STALL	2

static void fotg210_setup(struct fotg210_chip *chip)
{
	int id, ret = CX_IDLE;
	uint32_t tmp[2];
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)tmp;
	struct fotg210_regs *regs = chip->regs;

	/*
	 * If this is the first Cx 8 byte command,
	 * we can now query USB mode (high/full speed; USB 2.0/USB 1.0)
	 */
	if (chip->state == USB_STATE_POWERED) {
		chip->state = USB_STATE_DEFAULT;
		if (readl(&regs->otgcsr) & OTGCSR_DEV_B) {
			/* Mini-B */
			if (readl(&regs->dev_ctrl) & DEVCTRL_HS) {
				puts("fotg210: HS\n");
				chip->gadget.speed = USB_SPEED_HIGH;
				/* SOF mask timer = 1100 ticks */
				writel(SOFMTR_TMR(1100), &regs->sof_mtr);
			} else {
				puts("fotg210: FS\n");
				chip->gadget.speed = USB_SPEED_FULL;
				/* SOF mask timer = 10000 ticks */
				writel(SOFMTR_TMR(10000), &regs->sof_mtr);
			}
		} else {
			printf("fotg210: mini-A?\n");
		}
	}

	/* switch data port to ep0 */
	writel(DMAFIFO_CX, &regs->dma_fifo);
	/* fetch 8 bytes setup packet */
	tmp[0] = readl(&regs->ep0_data);
	tmp[1] = readl(&regs->ep0_data);
	/* release data port */
	writel(0, &regs->dma_fifo);

	if (req->bRequestType & USB_DIR_IN)
		ep0_desc.bEndpointAddress = USB_DIR_IN;
	else
		ep0_desc.bEndpointAddress = USB_DIR_OUT;

	ret = CX_IDLE;

	if ((req->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (req->bRequest) {
		case USB_REQ_SET_CONFIGURATION:
			debug("fotg210: set_cfg(%d)\n", req->wValue & 0x00FF);
			if (!(req->wValue & 0x00FF)) {
				chip->state = USB_STATE_ADDRESS;
				writel(chip->addr, &regs->dev_addr);
			} else {
				chip->state = USB_STATE_CONFIGURED;
				writel(chip->addr | DEVADDR_CONF,
					&regs->dev_addr);
			}
			ret = CX_IDLE;
			break;

		case USB_REQ_SET_ADDRESS:
			debug("fotg210: set_addr(0x%04X)\n", req->wValue);
			chip->state = USB_STATE_ADDRESS;
			chip->addr  = req->wValue & DEVADDR_ADDR_MASK;
			ret = CX_FINISH;
			writel(chip->addr, &regs->dev_addr);
			break;

		case USB_REQ_CLEAR_FEATURE:
			debug("fotg210: clr_feature(%d, %d)\n",
				req->bRequestType & 0x03, req->wValue);
			switch (req->wValue) {
			case 0:    /* [Endpoint] halt */
				ep_reset(chip, req->wIndex);
				ret = CX_FINISH;
				break;
			case 1:    /* [Device] remote wake-up */
			case 2:    /* [Device] test mode */
			default:
				ret = CX_STALL;
				break;
			}
			break;

		case USB_REQ_SET_FEATURE:
			debug("fotg210: set_feature(%d, %d)\n",
				req->wValue, req->wIndex & 0xf);
			switch (req->wValue) {
			case 0:    /* Endpoint Halt */
				id = req->wIndex & 0xf;
				setbits_le32(&regs->iep[id - 1], IEP_STALL);
				setbits_le32(&regs->oep[id - 1], OEP_STALL);
				ret = CX_FINISH;
				break;
			case 1:    /* Remote Wakeup */
			case 2:    /* Test Mode */
			default:
				ret = CX_STALL;
				break;
			}
			break;

		case USB_REQ_GET_STATUS:
			debug("fotg210: get_status\n");
			ret = CX_STALL;
			break;

		case USB_REQ_SET_DESCRIPTOR:
			debug("fotg210: set_descriptor\n");
			ret = CX_STALL;
			break;

		case USB_REQ_SYNCH_FRAME:
			debug("fotg210: sync frame\n");
			ret = CX_STALL;
			break;
		}
	} /* if ((req->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) */

	if (ret == CX_IDLE && chip->driver->setup) {
		if (chip->driver->setup(&chip->gadget, req) < 0)
			ret = CX_STALL;
		else
			ret = CX_FINISH;
	}

	switch (ret) {
	case CX_FINISH:
		setbits_le32(&regs->cxfifo, CXFIFO_CXFIN);
		break;

	case CX_STALL:
		setbits_le32(&regs->cxfifo, CXFIFO_CXSTALL | CXFIFO_CXFIN);
		printf("fotg210: cx_stall!\n");
		break;

	case CX_IDLE:
		debug("fotg210: cx_idle?\n");
	default:
		break;
	}
}

/*
 * fifo - FIFO id
 * zlp  - zero length packet
 */
static void fotg210_recv(struct fotg210_chip *chip, int ep_id)
{
	struct fotg210_regs *regs = chip->regs;
	struct fotg210_ep *ep = chip->ep + ep_id;
	struct fotg210_request *req;
	int len;

	if (ep->stopped || (ep->desc->bEndpointAddress & USB_DIR_IN)) {
		printf("fotg210: ep%d recv, invalid!\n", ep->id);
		return;
	}

	if (list_empty(&ep->queue)) {
		printf("fotg210: ep%d recv, drop!\n", ep->id);
		return;
	}

	req = list_first_entry(&ep->queue, struct fotg210_request, queue);
	len = fotg210_dma(ep, req);
	if (len < ep->ep.maxpacket || req->req.length <= req->req.actual) {
		list_del_init(&req->queue);
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
	}

	if (ep->id > 0 && list_empty(&ep->queue)) {
		setbits_le32(&regs->gimr1,
			GIMR1_FIFO_RX(ep_to_fifo(chip, ep->id)));
	}
}

/*
 * USB Gadget Layer
 */
static int fotg210_ep_enable(
	struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct fotg210_ep *ep = container_of(_ep, struct fotg210_ep, ep);
	struct fotg210_chip *chip = ep->chip;
	struct fotg210_regs *regs = chip->regs;
	int id = ep_to_fifo(chip, ep->id);
	int in = (desc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;

	if (!_ep || !desc
		|| desc->bDescriptorType != USB_DT_ENDPOINT
		|| le16_to_cpu(desc->wMaxPacketSize) == 0) {
		printf("fotg210: bad ep or descriptor\n");
		return -EINVAL;
	}

	ep->desc = desc;
	ep->stopped = 0;

	if (in)
		setbits_le32(&regs->fifomap, FIFOMAP(id, FIFOMAP_IN));

	switch (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_CONTROL:
		return -EINVAL;

	case USB_ENDPOINT_XFER_ISOC:
		setbits_le32(&regs->fifocfg,
			FIFOCFG(id, FIFOCFG_EN | FIFOCFG_ISOC));
		break;

	case USB_ENDPOINT_XFER_BULK:
		setbits_le32(&regs->fifocfg,
			FIFOCFG(id, FIFOCFG_EN | FIFOCFG_BULK));
		break;

	case USB_ENDPOINT_XFER_INT:
		setbits_le32(&regs->fifocfg,
			FIFOCFG(id, FIFOCFG_EN | FIFOCFG_INTR));
		break;
	}

	return 0;
}

static int fotg210_ep_disable(struct usb_ep *_ep)
{
	struct fotg210_ep *ep = container_of(_ep, struct fotg210_ep, ep);
	struct fotg210_chip *chip = ep->chip;
	struct fotg210_regs *regs = chip->regs;
	int id = ep_to_fifo(chip, ep->id);

	ep->desc = NULL;
	ep->stopped = 1;

	clrbits_le32(&regs->fifocfg, FIFOCFG(id, FIFOCFG_CFG_MASK));
	clrbits_le32(&regs->fifomap, FIFOMAP(id, FIFOMAP_DIR_MASK));

	return 0;
}

static struct usb_request *fotg210_ep_alloc_request(
	struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct fotg210_request *req = malloc(sizeof(*req));

	if (req) {
		memset(req, 0, sizeof(*req));
		INIT_LIST_HEAD(&req->queue);
	}
	return &req->req;
}

static void fotg210_ep_free_request(
	struct usb_ep *_ep, struct usb_request *_req)
{
	struct fotg210_request *req;

	req = container_of(_req, struct fotg210_request, req);
	free(req);
}

static int fotg210_ep_queue(
	struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct fotg210_ep *ep = container_of(_ep, struct fotg210_ep, ep);
	struct fotg210_chip *chip = ep->chip;
	struct fotg210_regs *regs = chip->regs;
	struct fotg210_request *req;

	req = container_of(_req, struct fotg210_request, req);
	if (!_req || !_req->complete || !_req->buf
		|| !list_empty(&req->queue)) {
		printf("fotg210: invalid request to ep%d\n", ep->id);
		return -EINVAL;
	}

	if (!chip || chip->state == USB_STATE_SUSPENDED) {
		printf("fotg210: request while chip suspended\n");
		return -EINVAL;
	}

	req->req.actual = 0;
	req->req.status = -EINPROGRESS;

	if (req->req.length == 0) {
		req->req.status = 0;
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
		return 0;
	}

	if (ep->id == 0) {
		do {
			int len = fotg210_dma(ep, req);
			if (len < ep->ep.maxpacket)
				break;
			if (ep->desc->bEndpointAddress & USB_DIR_IN)
				udelay(100);
		} while (req->req.length > req->req.actual);
	} else {
		if (ep->desc->bEndpointAddress & USB_DIR_IN) {
			do {
				int len = fotg210_dma(ep, req);
				if (len < ep->ep.maxpacket)
					break;
			} while (req->req.length > req->req.actual);
		} else {
			list_add_tail(&req->queue, &ep->queue);
			clrbits_le32(&regs->gimr1,
				GIMR1_FIFO_RX(ep_to_fifo(chip, ep->id)));
		}
	}

	if (ep->id == 0 || (ep->desc->bEndpointAddress & USB_DIR_IN)) {
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
	}

	return 0;
}

static int fotg210_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct fotg210_ep *ep = container_of(_ep, struct fotg210_ep, ep);
	struct fotg210_request *req;

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req)
		return -EINVAL;

	/* remove the request */
	list_del_init(&req->queue);

	/* update status & invoke complete callback */
	if (req->req.status == -EINPROGRESS) {
		req->req.status = -ECONNRESET;
		if (req->req.complete)
			req->req.complete(_ep, &req->req);
	}

	return 0;
}

static int fotg210_ep_halt(struct usb_ep *_ep, int halt)
{
	struct fotg210_ep *ep = container_of(_ep, struct fotg210_ep, ep);
	struct fotg210_chip *chip = ep->chip;
	struct fotg210_regs *regs = chip->regs;
	int ret = -1;

	debug("fotg210: ep%d halt=%d\n", ep->id, halt);

	/* Endpoint STALL */
	if (ep->id > 0 && ep->id <= CFG_NUM_ENDPOINTS) {
		if (halt) {
			/* wait until all ep fifo empty */
			fotg210_cxwait(chip, 0xf00);
			/* stall */
			if (ep->desc->bEndpointAddress & USB_DIR_IN) {
				setbits_le32(&regs->iep[ep->id - 1],
					IEP_STALL);
			} else {
				setbits_le32(&regs->oep[ep->id - 1],
					OEP_STALL);
			}
		} else {
			if (ep->desc->bEndpointAddress & USB_DIR_IN) {
				clrbits_le32(&regs->iep[ep->id - 1],
					IEP_STALL);
			} else {
				clrbits_le32(&regs->oep[ep->id - 1],
					OEP_STALL);
			}
		}
		ret = 0;
	}

	return ret;
}

/*
 * activate/deactivate link with host.
 */
static void pullup(struct fotg210_chip *chip, int is_on)
{
	struct fotg210_regs *regs = chip->regs;

	if (is_on) {
		if (!chip->pullup) {
			chip->state = USB_STATE_POWERED;
			chip->pullup = 1;
			/* enable the chip */
			setbits_le32(&regs->dev_ctrl, DEVCTRL_EN);
			/* clear unplug bit (BIT0) */
			clrbits_le32(&regs->phy_tmsr, PHYTMSR_UNPLUG);
		}
	} else {
		chip->state = USB_STATE_NOTATTACHED;
		chip->pullup = 0;
		chip->addr = 0;
		writel(chip->addr, &regs->dev_addr);
		/* set unplug bit (BIT0) */
		setbits_le32(&regs->phy_tmsr, PHYTMSR_UNPLUG);
		/* disable the chip */
		clrbits_le32(&regs->dev_ctrl, DEVCTRL_EN);
	}
}

static int fotg210_pullup(struct usb_gadget *_gadget, int is_on)
{
	struct fotg210_chip *chip;

	chip = container_of(_gadget, struct fotg210_chip, gadget);

	debug("fotg210: pullup=%d\n", is_on);

	pullup(chip, is_on);

	return 0;
}

static int fotg210_get_frame(struct usb_gadget *_gadget)
{
	struct fotg210_chip *chip;
	struct fotg210_regs *regs;

	chip = container_of(_gadget, struct fotg210_chip, gadget);
	regs = chip->regs;

	return SOFFNR_FNR(readl(&regs->sof_fnr));
}

static struct usb_gadget_ops fotg210_gadget_ops = {
	.get_frame = fotg210_get_frame,
	.pullup = fotg210_pullup,
};

static struct usb_ep_ops fotg210_ep_ops = {
	.enable         = fotg210_ep_enable,
	.disable        = fotg210_ep_disable,
	.queue          = fotg210_ep_queue,
	.dequeue        = fotg210_ep_dequeue,
	.set_halt       = fotg210_ep_halt,
	.alloc_request  = fotg210_ep_alloc_request,
	.free_request   = fotg210_ep_free_request,
};

static struct fotg210_chip controller = {
	.regs = (void __iomem *)CONFIG_FOTG210_BASE,
	.gadget = {
		.name = "fotg210_udc",
		.ops = &fotg210_gadget_ops,
		.ep0 = &controller.ep[0].ep,
		.speed = USB_SPEED_UNKNOWN,
		.is_dualspeed = 1,
		.is_otg = 0,
		.is_a_peripheral = 0,
		.b_hnp_enable = 0,
		.a_hnp_support = 0,
		.a_alt_hnp_support = 0,
	},
	.ep[0] = {
		.id = 0,
		.ep = {
			.name  = "ep0",
			.ops   = &fotg210_ep_ops,
		},
		.desc      = &ep0_desc,
		.chip      = &controller,
		.maxpacket = CFG_EP0_MAX_PACKET_SIZE,
	},
	.ep[1] = {
		.id = 1,
		.ep = {
			.name  = "ep1",
			.ops   = &fotg210_ep_ops,
		},
		.chip      = &controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[2] = {
		.id = 2,
		.ep = {
			.name  = "ep2",
			.ops   = &fotg210_ep_ops,
		},
		.chip      = &controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[3] = {
		.id = 3,
		.ep = {
			.name  = "ep3",
			.ops   = &fotg210_ep_ops,
		},
		.chip      = &controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[4] = {
		.id = 4,
		.ep = {
			.name  = "ep4",
			.ops   = &fotg210_ep_ops,
		},
		.chip      = &controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
};

int usb_gadget_handle_interrupts(int index)
{
	struct fotg210_chip *chip = &controller;
	struct fotg210_regs *regs = chip->regs;
	uint32_t id, st, isr, gisr;

	isr  = readl(&regs->isr) & (~readl(&regs->imr));
	gisr = readl(&regs->gisr) & (~readl(&regs->gimr));
	if (!(isr & ISR_DEV) || !gisr)
		return 0;

	writel(ISR_DEV, &regs->isr);

	/* CX interrupts */
	if (gisr & GISR_GRP0) {
		st = readl(&regs->gisr0);
		/*
		 * Write 1 and then 0 works for both W1C & RW.
		 *
		 * HW v1.11.0+: It's a W1C register (write 1 clear)
		 * HW v1.10.0-: It's a R/W register (write 0 clear)
		 */
		writel(st & GISR0_CXABORT, &regs->gisr0);
		writel(0, &regs->gisr0);

		if (st & GISR0_CXERR)
			printf("fotg210: cmd error\n");

		if (st & GISR0_CXABORT)
			printf("fotg210: cmd abort\n");

		if (st & GISR0_CXSETUP)    /* setup */
			fotg210_setup(chip);
		else if (st & GISR0_CXEND) /* command finish */
			setbits_le32(&regs->cxfifo, CXFIFO_CXFIN);
	}

	/* FIFO interrupts */
	if (gisr & GISR_GRP1) {
		st = readl(&regs->gisr1);
		for (id = 0; id < 4; ++id) {
			if (st & GISR1_RX_FIFO(id))
				fotg210_recv(chip, fifo_to_ep(chip, id, 0));
		}
	}

	/* Device Status Interrupts */
	if (gisr & GISR_GRP2) {
		st = readl(&regs->gisr2);
		/*
		 * Write 1 and then 0 works for both W1C & RW.
		 *
		 * HW v1.11.0+: It's a W1C register (write 1 clear)
		 * HW v1.10.0-: It's a R/W register (write 0 clear)
		 */
		writel(st, &regs->gisr2);
		writel(0, &regs->gisr2);

		if (st & GISR2_RESET)
			printf("fotg210: reset by host\n");
		else if (st & GISR2_SUSPEND)
			printf("fotg210: suspend/removed\n");
		else if (st & GISR2_RESUME)
			printf("fotg210: resume\n");

		/* Errors */
		if (st & GISR2_ISOCERR)
			printf("fotg210: iso error\n");
		if (st & GISR2_ISOCABT)
			printf("fotg210: iso abort\n");
		if (st & GISR2_DMAERR)
			printf("fotg210: dma error\n");
	}

	return 0;
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int i, ret = 0;
	struct fotg210_chip *chip = &controller;

	if (!driver    || !driver->bind || !driver->setup) {
		puts("fotg210: bad parameter.\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&chip->gadget.ep_list);
	for (i = 0; i < CFG_NUM_ENDPOINTS + 1; ++i) {
		struct fotg210_ep *ep = chip->ep + i;

		ep->ep.maxpacket = ep->maxpacket;
		INIT_LIST_HEAD(&ep->queue);

		if (ep->id == 0) {
			ep->stopped = 0;
		} else {
			ep->stopped = 1;
			list_add_tail(&ep->ep.ep_list, &chip->gadget.ep_list);
		}
	}

	if (fotg210_reset(chip)) {
		puts("fotg210: reset failed.\n");
		return -EINVAL;
	}

	ret = driver->bind(&chip->gadget);
	if (ret) {
		debug("fotg210: driver->bind() returned %d\n", ret);
		return ret;
	}
	chip->driver = driver;

	return ret;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct fotg210_chip *chip = &controller;

	driver->unbind(&chip->gadget);
	chip->driver = NULL;

	pullup(chip, 0);

	return 0;
}
