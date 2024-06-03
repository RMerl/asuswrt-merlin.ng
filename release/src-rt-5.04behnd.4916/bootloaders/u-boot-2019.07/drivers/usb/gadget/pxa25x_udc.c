// SPDX-License-Identifier: GPL-2.0+
/*
 * Intel PXA25x and IXP4xx on-chip full speed USB device controllers
 *
 * Copyright (C) 2002 Intrinsyc, Inc. (Frank Becker)
 * Copyright (C) 2003 Robert Schwebel, Pengutronix
 * Copyright (C) 2003 Benedikt Spranger, Pengutronix
 * Copyright (C) 2003 David Brownell
 * Copyright (C) 2003 Joshua Wise
 * Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
 *
 * MODULE_AUTHOR("Frank Becker, Robert Schwebel, David Brownell");
 */

#define CONFIG_USB_PXA25X_SMALL
#define DRIVER_NAME "pxa25x_udc_linux"
#define ARCH_HAS_PREFETCH

#include <common.h>
#include <errno.h>
#include <asm/byteorder.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/unaligned.h>
#include <linux/compat.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/pxa.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <asm/arch/pxa-regs.h>

#include "pxa25x_udc.h"

/*
 * This driver handles the USB Device Controller (UDC) in Intel's PXA 25x
 * series processors.  The UDC for the IXP 4xx series is very similar.
 * There are fifteen endpoints, in addition to ep0.
 *
 * Such controller drivers work with a gadget driver.  The gadget driver
 * returns descriptors, implements configuration and data protocols used
 * by the host to interact with this device, and allocates endpoints to
 * the different protocol interfaces.  The controller driver virtualizes
 * usb hardware so that the gadget drivers will be more portable.
 *
 * This UDC hardware wants to implement a bit too much USB protocol, so
 * it constrains the sorts of USB configuration change events that work.
 * The errata for these chips are misleading; some "fixed" bugs from
 * pxa250 a0/a1 b0/b1/b2 sure act like they're still there.
 *
 * Note that the UDC hardware supports DMA (except on IXP) but that's
 * not used here.  IN-DMA (to host) is simple enough, when the data is
 * suitably aligned (16 bytes) ... the network stack doesn't do that,
 * other software can.  OUT-DMA is buggy in most chip versions, as well
 * as poorly designed (data toggle not automatic).  So this driver won't
 * bother using DMA.  (Mostly-working IN-DMA support was available in
 * kernels before 2.6.23, but was never enabled or well tested.)
 */

#define DRIVER_VERSION	"18-August-2012"
#define DRIVER_DESC	"PXA 25x USB Device Controller driver"

static const char driver_name[] = "pxa25x_udc";
static const char ep0name[] = "ep0";

/* Watchdog */
static inline void start_watchdog(struct pxa25x_udc *udc)
{
	debug("Started watchdog\n");
	udc->watchdog.base = get_timer(0);
	udc->watchdog.running = 1;
}

static inline void stop_watchdog(struct pxa25x_udc *udc)
{
	udc->watchdog.running = 0;
	debug("Stopped watchdog\n");
}

static inline void test_watchdog(struct pxa25x_udc *udc)
{
	if (!udc->watchdog.running)
		return;

	debug("watchdog %ld %ld\n", get_timer(udc->watchdog.base),
		udc->watchdog.period);

	if (get_timer(udc->watchdog.base) >= udc->watchdog.period) {
		stop_watchdog(udc);
		udc->watchdog.function(udc);
	}
}

static void udc_watchdog(struct pxa25x_udc *dev)
{
	uint32_t udccs0 = readl(&dev->regs->udccs[0]);

	debug("Fired up udc_watchdog\n");

	local_irq_disable();
	if (dev->ep0state == EP0_STALL
			&& (udccs0 & UDCCS0_FST) == 0
			&& (udccs0 & UDCCS0_SST) == 0) {
		writel(UDCCS0_FST|UDCCS0_FTF, &dev->regs->udccs[0]);
		debug("ep0 re-stall\n");
		start_watchdog(dev);
	}
	local_irq_enable();
}

#ifdef DEBUG

static const char * const state_name[] = {
	"EP0_IDLE",
	"EP0_IN_DATA_PHASE", "EP0_OUT_DATA_PHASE",
	"EP0_END_XFER", "EP0_STALL"
};

static void
dump_udccr(const char *label)
{
	u32 udccr = readl(&UDC_REGS->udccr);
	debug("%s %02X =%s%s%s%s%s%s%s%s\n",
		label, udccr,
		(udccr & UDCCR_REM) ? " rem" : "",
		(udccr & UDCCR_RSTIR) ? " rstir" : "",
		(udccr & UDCCR_SRM) ? " srm" : "",
		(udccr & UDCCR_SUSIR) ? " susir" : "",
		(udccr & UDCCR_RESIR) ? " resir" : "",
		(udccr & UDCCR_RSM) ? " rsm" : "",
		(udccr & UDCCR_UDA) ? " uda" : "",
		(udccr & UDCCR_UDE) ? " ude" : "");
}

static void
dump_udccs0(const char *label)
{
	u32 udccs0 = readl(&UDC_REGS->udccs[0]);

	debug("%s %s %02X =%s%s%s%s%s%s%s%s\n",
		label, state_name[the_controller->ep0state], udccs0,
		(udccs0 & UDCCS0_SA) ? " sa" : "",
		(udccs0 & UDCCS0_RNE) ? " rne" : "",
		(udccs0 & UDCCS0_FST) ? " fst" : "",
		(udccs0 & UDCCS0_SST) ? " sst" : "",
		(udccs0 & UDCCS0_DRWF) ? " dwrf" : "",
		(udccs0 & UDCCS0_FTF) ? " ftf" : "",
		(udccs0 & UDCCS0_IPR) ? " ipr" : "",
		(udccs0 & UDCCS0_OPR) ? " opr" : "");
}

static void
dump_state(struct pxa25x_udc *dev)
{
	u32 tmp;
	unsigned i;

	debug("%s, uicr %02X.%02X, usir %02X.%02x, ufnr %02X.%02X\n",
		state_name[dev->ep0state],
		readl(&UDC_REGS->uicr1), readl(&UDC_REGS->uicr0),
		readl(&UDC_REGS->usir1), readl(&UDC_REGS->usir0),
		readl(&UDC_REGS->ufnrh), readl(&UDC_REGS->ufnrl));
	dump_udccr("udccr");
	if (dev->has_cfr) {
		tmp = readl(&UDC_REGS->udccfr);
		debug("udccfr %02X =%s%s\n", tmp,
			(tmp & UDCCFR_AREN) ? " aren" : "",
			(tmp & UDCCFR_ACM) ? " acm" : "");
	}

	if (!dev->driver) {
		debug("no gadget driver bound\n");
		return;
	} else
		debug("ep0 driver '%s'\n", "ether");

	dump_udccs0("udccs0");
	debug("ep0 IN %lu/%lu, OUT %lu/%lu\n",
		dev->stats.write.bytes, dev->stats.write.ops,
		dev->stats.read.bytes, dev->stats.read.ops);

	for (i = 1; i < PXA_UDC_NUM_ENDPOINTS; i++) {
		if (dev->ep[i].desc == NULL)
			continue;
		debug("udccs%d = %02x\n", i, *dev->ep->reg_udccs);
	}
}

#else /* DEBUG */

static inline void dump_udccr(const char *label) { }
static inline void dump_udccs0(const char *label) { }
static inline void dump_state(struct pxa25x_udc *dev) { }

#endif /* DEBUG */

/*
 * ---------------------------------------------------------------------------
 *	endpoint related parts of the api to the usb controller hardware,
 *	used by gadget driver; and the inner talker-to-hardware core.
 * ---------------------------------------------------------------------------
 */

static void pxa25x_ep_fifo_flush(struct usb_ep *ep);
static void nuke(struct pxa25x_ep *, int status);

/* one GPIO should control a D+ pullup, so host sees this device (or not) */
static void pullup_off(void)
{
	struct pxa2xx_udc_mach_info *mach = the_controller->mach;

	if (mach->udc_command)
		mach->udc_command(PXA2XX_UDC_CMD_DISCONNECT);
}

static void pullup_on(void)
{
	struct pxa2xx_udc_mach_info *mach = the_controller->mach;

	if (mach->udc_command)
		mach->udc_command(PXA2XX_UDC_CMD_CONNECT);
}

static void pio_irq_enable(int bEndpointAddress)
{
	bEndpointAddress &= 0xf;
	if (bEndpointAddress < 8) {
		clrbits_le32(&the_controller->regs->uicr0,
			1 << bEndpointAddress);
	} else {
		bEndpointAddress -= 8;
		clrbits_le32(&the_controller->regs->uicr1,
			1 << bEndpointAddress);
	}
}

static void pio_irq_disable(int bEndpointAddress)
{
	bEndpointAddress &= 0xf;
	if (bEndpointAddress < 8) {
		setbits_le32(&the_controller->regs->uicr0,
			1 << bEndpointAddress);
	} else {
		bEndpointAddress -= 8;
		setbits_le32(&the_controller->regs->uicr1,
			1 << bEndpointAddress);
	}
}

static inline void udc_set_mask_UDCCR(int mask)
{
	/*
	 * The UDCCR reg contains mask and interrupt status bits,
	 * so using '|=' isn't safe as it may ack an interrupt.
	 */
	const uint32_t mask_bits = UDCCR_REM | UDCCR_SRM | UDCCR_UDE;

	mask &= mask_bits;
	clrsetbits_le32(&the_controller->regs->udccr, ~mask_bits, mask);
}

static inline void udc_clear_mask_UDCCR(int mask)
{
	const uint32_t mask_bits = UDCCR_REM | UDCCR_SRM | UDCCR_UDE;

	mask = ~mask & mask_bits;
	clrbits_le32(&the_controller->regs->udccr, ~mask);
}

static inline void udc_ack_int_UDCCR(int mask)
{
	const uint32_t mask_bits = UDCCR_REM | UDCCR_SRM | UDCCR_UDE;

	mask &= ~mask_bits;
	clrsetbits_le32(&the_controller->regs->udccr, ~mask_bits, mask);
}

/*
 * endpoint enable/disable
 *
 * we need to verify the descriptors used to enable endpoints.  since pxa25x
 * endpoint configurations are fixed, and are pretty much always enabled,
 * there's not a lot to manage here.
 *
 * because pxa25x can't selectively initialize bulk (or interrupt) endpoints,
 * (resetting endpoint halt and toggle), SET_INTERFACE is unusable except
 * for a single interface (with only the default altsetting) and for gadget
 * drivers that don't halt endpoints (not reset by set_interface).  that also
 * means that if you use ISO, you must violate the USB spec rule that all
 * iso endpoints must be in non-default altsettings.
 */
static int pxa25x_ep_enable(struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct pxa25x_ep *ep;
	struct pxa25x_udc *dev;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
			|| desc->bDescriptorType != USB_DT_ENDPOINT
			|| ep->bEndpointAddress != desc->bEndpointAddress
			|| ep->fifo_size <
			   le16_to_cpu(get_unaligned(&desc->wMaxPacketSize))) {
		printf("%s, bad ep or descriptor\n", __func__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
			&& ep->bmAttributes != USB_ENDPOINT_XFER_BULK
			&& desc->bmAttributes != USB_ENDPOINT_XFER_INT) {
		printf("%s, %s type mismatch\n", __func__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
			&& le16_to_cpu(get_unaligned(&desc->wMaxPacketSize))
						!= BULK_FIFO_SIZE)
			|| !get_unaligned(&desc->wMaxPacketSize)) {
		printf("%s, bad %s maxpacket\n", __func__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
		printf("%s, bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	ep->desc = desc;
	ep->stopped = 0;
	ep->pio_irqs = 0;
	ep->ep.maxpacket = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	/* flush fifo (mostly for OUT buffers) */
	pxa25x_ep_fifo_flush(_ep);

	/* ... reset halt state too, if we could ... */

	debug("enabled %s\n", _ep->name);
	return 0;
}

static int pxa25x_ep_disable(struct usb_ep *_ep)
{
	struct pxa25x_ep *ep;
	unsigned long flags;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (!_ep || !ep->desc) {
		printf("%s, %s not enabled\n", __func__,
			_ep ? ep->ep.name : NULL);
		return -EINVAL;
	}
	local_irq_save(flags);

	nuke(ep, -ESHUTDOWN);

	/* flush fifo (mostly for IN buffers) */
	pxa25x_ep_fifo_flush(_ep);

	ep->desc = NULL;
	ep->stopped = 1;

	local_irq_restore(flags);
	debug("%s disabled\n", _ep->name);
	return 0;
}

/*-------------------------------------------------------------------------*/

/*
 * for the pxa25x, these can just wrap kmalloc/kfree.  gadget drivers
 * must still pass correctly initialized endpoints, since other controller
 * drivers may care about how it's currently set up (dma issues etc).
 */

/*
 *	pxa25x_ep_alloc_request - allocate a request data structure
 */
static struct usb_request *
pxa25x_ep_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct pxa25x_request *req;

	req = kzalloc(sizeof(*req), gfp_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);
	return &req->req;
}


/*
 *	pxa25x_ep_free_request - deallocate a request data structure
 */
static void
pxa25x_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct pxa25x_request	*req;

	req = container_of(_req, struct pxa25x_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/*-------------------------------------------------------------------------*/

/*
 *	done - retire a request; caller blocked irqs
 */
static void done(struct pxa25x_ep *ep, struct pxa25x_request *req, int status)
{
	unsigned stopped = ep->stopped;

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN)
		debug("complete %s req %p stat %d len %u/%u\n",
			ep->ep.name, &req->req, status,
			req->req.actual, req->req.length);

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;
	req->req.complete(&ep->ep, &req->req);
	ep->stopped = stopped;
}


static inline void ep0_idle(struct pxa25x_udc *dev)
{
	dev->ep0state = EP0_IDLE;
}

static int
write_packet(u32 *uddr, struct pxa25x_request *req, unsigned max)
{
	u8 *buf;
	unsigned length, count;

	debug("%s(): uddr %p\n", __func__, uddr);

	buf = req->req.buf + req->req.actual;
	prefetch(buf);

	/* how big will this packet be? */
	length = min(req->req.length - req->req.actual, max);
	req->req.actual += length;

	count = length;
	while (likely(count--))
		writeb(*buf++, uddr);

	return length;
}

/*
 * write to an IN endpoint fifo, as many packets as possible.
 * irqs will use this to write the rest later.
 * caller guarantees at least one packet buffer is ready (or a zlp).
 */
static int
write_fifo(struct pxa25x_ep *ep, struct pxa25x_request *req)
{
	unsigned max;

	max = le16_to_cpu(get_unaligned(&ep->desc->wMaxPacketSize));
	do {
		unsigned count;
		int is_last, is_short;

		count = write_packet(ep->reg_uddr, req, max);

		/* last packet is usually short (or a zlp) */
		if (unlikely(count != max))
			is_last = is_short = 1;
		else {
			if (likely(req->req.length != req->req.actual)
					|| req->req.zero)
				is_last = 0;
			else
				is_last = 1;
			/* interrupt/iso maxpacket may not fill the fifo */
			is_short = unlikely(max < ep->fifo_size);
		}

		debug_cond(NOISY, "wrote %s %d bytes%s%s %d left %p\n",
			ep->ep.name, count,
			is_last ? "/L" : "", is_short ? "/S" : "",
			req->req.length - req->req.actual, req);

		/*
		 * let loose that packet. maybe try writing another one,
		 * double buffering might work.  TSP, TPC, and TFS
		 * bit values are the same for all normal IN endpoints.
		 */
		writel(UDCCS_BI_TPC, ep->reg_udccs);
		if (is_short)
			writel(UDCCS_BI_TSP, ep->reg_udccs);

		/* requests complete when all IN data is in the FIFO */
		if (is_last) {
			done(ep, req, 0);
			if (list_empty(&ep->queue))
				pio_irq_disable(ep->bEndpointAddress);
			return 1;
		}

		/*
		 * TODO experiment: how robust can fifo mode tweaking be?
		 * double buffering is off in the default fifo mode, which
		 * prevents TFS from being set here.
		 */

	} while (readl(ep->reg_udccs) & UDCCS_BI_TFS);
	return 0;
}

/*
 * caller asserts req->pending (ep0 irq status nyet cleared); starts
 * ep0 data stage.  these chips want very simple state transitions.
 */
static inline
void ep0start(struct pxa25x_udc *dev, u32 flags, const char *tag)
{
	writel(flags|UDCCS0_SA|UDCCS0_OPR, &dev->regs->udccs[0]);
	writel(USIR0_IR0, &dev->regs->usir0);
	dev->req_pending = 0;
	debug_cond(NOISY, "%s() %s, udccs0: %02x/%02x usir: %X.%X\n",
		__func__, tag, readl(&dev->regs->udccs[0]), flags,
		readl(&dev->regs->usir1), readl(&dev->regs->usir0));
}

static int
write_ep0_fifo(struct pxa25x_ep *ep, struct pxa25x_request *req)
{
	unsigned count;
	int is_short;

	count = write_packet(&ep->dev->regs->uddr0, req, EP0_FIFO_SIZE);
	ep->dev->stats.write.bytes += count;

	/* last packet "must be" short (or a zlp) */
	is_short = (count != EP0_FIFO_SIZE);

	debug_cond(NOISY, "ep0in %d bytes %d left %p\n", count,
		req->req.length - req->req.actual, req);

	if (unlikely(is_short)) {
		if (ep->dev->req_pending)
			ep0start(ep->dev, UDCCS0_IPR, "short IN");
		else
			writel(UDCCS0_IPR, &ep->dev->regs->udccs[0]);

		count = req->req.length;
		done(ep, req, 0);
		ep0_idle(ep->dev);

		/*
		 * This seems to get rid of lost status irqs in some cases:
		 * host responds quickly, or next request involves config
		 * change automagic, or should have been hidden, or ...
		 *
		 * FIXME get rid of all udelays possible...
		 */
		if (count >= EP0_FIFO_SIZE) {
			count = 100;
			do {
				if ((readl(&ep->dev->regs->udccs[0]) &
				     UDCCS0_OPR) != 0) {
					/* clear OPR, generate ack */
					writel(UDCCS0_OPR,
						&ep->dev->regs->udccs[0]);
					break;
				}
				count--;
				udelay(1);
			} while (count);
		}
	} else if (ep->dev->req_pending)
		ep0start(ep->dev, 0, "IN");

	return is_short;
}


/*
 * read_fifo -  unload packet(s) from the fifo we use for usb OUT
 * transfers and put them into the request.  caller should have made
 * sure there's at least one packet ready.
 *
 * returns true if the request completed because of short packet or the
 * request buffer having filled (and maybe overran till end-of-packet).
 */
static int
read_fifo(struct pxa25x_ep *ep, struct pxa25x_request *req)
{
	u32 udccs;
	u8 *buf;
	unsigned bufferspace, count, is_short;

	for (;;) {
		/*
		 * make sure there's a packet in the FIFO.
		 * UDCCS_{BO,IO}_RPC are all the same bit value.
		 * UDCCS_{BO,IO}_RNE are all the same bit value.
		 */
		udccs = readl(ep->reg_udccs);
		if (unlikely((udccs & UDCCS_BO_RPC) == 0))
			break;
		buf = req->req.buf + req->req.actual;
		prefetchw(buf);
		bufferspace = req->req.length - req->req.actual;

		/* read all bytes from this packet */
		if (likely(udccs & UDCCS_BO_RNE)) {
			count = 1 + (0x0ff & readl(ep->reg_ubcr));
			req->req.actual += min(count, bufferspace);
		} else /* zlp */
			count = 0;
		is_short = (count < ep->ep.maxpacket);
		debug_cond(NOISY, "read %s %02x, %d bytes%s req %p %d/%d\n",
			ep->ep.name, udccs, count,
			is_short ? "/S" : "",
			req, req->req.actual, req->req.length);
		while (likely(count-- != 0)) {
			u8 byte = readb(ep->reg_uddr);

			if (unlikely(bufferspace == 0)) {
				/*
				 * this happens when the driver's buffer
				 * is smaller than what the host sent.
				 * discard the extra data.
				 */
				if (req->req.status != -EOVERFLOW)
					printf("%s overflow %d\n",
						ep->ep.name, count);
				req->req.status = -EOVERFLOW;
			} else {
				*buf++ = byte;
				bufferspace--;
			}
		}
		writel(UDCCS_BO_RPC, ep->reg_udccs);
		/* RPC/RSP/RNE could now reflect the other packet buffer */

		/* iso is one request per packet */
		if (ep->bmAttributes == USB_ENDPOINT_XFER_ISOC) {
			if (udccs & UDCCS_IO_ROF)
				req->req.status = -EHOSTUNREACH;
			/* more like "is_done" */
			is_short = 1;
		}

		/* completion */
		if (is_short || req->req.actual == req->req.length) {
			done(ep, req, 0);
			if (list_empty(&ep->queue))
				pio_irq_disable(ep->bEndpointAddress);
			return 1;
		}

		/* finished that packet.  the next one may be waiting... */
	}
	return 0;
}

/*
 * special ep0 version of the above.  no UBCR0 or double buffering; status
 * handshaking is magic.  most device protocols don't need control-OUT.
 * CDC vendor commands (and RNDIS), mass storage CB/CBI, and some other
 * protocols do use them.
 */
static int
read_ep0_fifo(struct pxa25x_ep *ep, struct pxa25x_request *req)
{
	u8 *buf, byte;
	unsigned bufferspace;

	buf = req->req.buf + req->req.actual;
	bufferspace = req->req.length - req->req.actual;

	while (readl(&ep->dev->regs->udccs[0]) & UDCCS0_RNE) {
		byte = (u8)readb(&ep->dev->regs->uddr0);

		if (unlikely(bufferspace == 0)) {
			/*
			 * this happens when the driver's buffer
			 * is smaller than what the host sent.
			 * discard the extra data.
			 */
			if (req->req.status != -EOVERFLOW)
				printf("%s overflow\n", ep->ep.name);
			req->req.status = -EOVERFLOW;
		} else {
			*buf++ = byte;
			req->req.actual++;
			bufferspace--;
		}
	}

	writel(UDCCS0_OPR | UDCCS0_IPR, &ep->dev->regs->udccs[0]);

	/* completion */
	if (req->req.actual >= req->req.length)
		return 1;

	/* finished that packet.  the next one may be waiting... */
	return 0;
}

/*-------------------------------------------------------------------------*/

static int
pxa25x_ep_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct pxa25x_request *req;
	struct pxa25x_ep *ep;
	struct pxa25x_udc *dev;
	unsigned long flags;

	req = container_of(_req, struct pxa25x_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf
			|| !list_empty(&req->queue))) {
		printf("%s, bad params\n", __func__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		printf("%s, bad ep\n", __func__);
		return -EINVAL;
	}

	dev = ep->dev;
	if (unlikely(!dev->driver
			|| dev->gadget.speed == USB_SPEED_UNKNOWN)) {
		printf("%s, bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	/*
	 * iso is always one packet per request, that's the only way
	 * we can report per-packet status.  that also helps with dma.
	 */
	if (unlikely(ep->bmAttributes == USB_ENDPOINT_XFER_ISOC
			&& req->req.length >
			le16_to_cpu(get_unaligned(&ep->desc->wMaxPacketSize))))
		return -EMSGSIZE;

	debug_cond(NOISY, "%s queue req %p, len %d buf %p\n",
		_ep->name, _req, _req->length, _req->buf);

	local_irq_save(flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	if (list_empty(&ep->queue) && !ep->stopped) {
		if (ep->desc == NULL/* ep0 */) {
			unsigned length = _req->length;

			switch (dev->ep0state) {
			case EP0_IN_DATA_PHASE:
				dev->stats.write.ops++;
				if (write_ep0_fifo(ep, req))
					req = NULL;
				break;

			case EP0_OUT_DATA_PHASE:
				dev->stats.read.ops++;
				/* messy ... */
				if (dev->req_config) {
					debug("ep0 config ack%s\n",
						dev->has_cfr ?  "" : " raced");
					if (dev->has_cfr)
						writel(UDCCFR_AREN|UDCCFR_ACM
							|UDCCFR_MB1,
							&ep->dev->regs->udccfr);
					done(ep, req, 0);
					dev->ep0state = EP0_END_XFER;
					local_irq_restore(flags);
					return 0;
				}
				if (dev->req_pending)
					ep0start(dev, UDCCS0_IPR, "OUT");
				if (length == 0 ||
						((readl(
						&ep->dev->regs->udccs[0])
						& UDCCS0_RNE) != 0
						&& read_ep0_fifo(ep, req))) {
					ep0_idle(dev);
					done(ep, req, 0);
					req = NULL;
				}
				break;

			default:
				printf("ep0 i/o, odd state %d\n",
					dev->ep0state);
				local_irq_restore(flags);
				return -EL2HLT;
			}
		/* can the FIFO can satisfy the request immediately? */
		} else if ((ep->bEndpointAddress & USB_DIR_IN) != 0) {
			if ((readl(ep->reg_udccs) & UDCCS_BI_TFS) != 0
					&& write_fifo(ep, req))
				req = NULL;
		} else if ((readl(ep->reg_udccs) & UDCCS_BO_RFS) != 0
				&& read_fifo(ep, req)) {
			req = NULL;
		}

		if (likely(req && ep->desc))
			pio_irq_enable(ep->bEndpointAddress);
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != NULL))
		list_add_tail(&req->queue, &ep->queue);
	local_irq_restore(flags);

	return 0;
}


/*
 *	nuke - dequeue ALL requests
 */
static void nuke(struct pxa25x_ep *ep, int status)
{
	struct pxa25x_request *req;

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next,
				struct pxa25x_request,
				queue);
		done(ep, req, status);
	}
	if (ep->desc)
		pio_irq_disable(ep->bEndpointAddress);
}


/* dequeue JUST ONE request */
static int pxa25x_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct pxa25x_ep *ep;
	struct pxa25x_request *req;
	unsigned long flags;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;

	local_irq_save(flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		local_irq_restore(flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);

	local_irq_restore(flags);
	return 0;
}

/*-------------------------------------------------------------------------*/

static int pxa25x_ep_set_halt(struct usb_ep *_ep, int value)
{
	struct pxa25x_ep *ep;
	unsigned long flags;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (unlikely(!_ep
			|| (!ep->desc && ep->ep.name != ep0name))
			|| ep->bmAttributes == USB_ENDPOINT_XFER_ISOC) {
		printf("%s, bad ep\n", __func__);
		return -EINVAL;
	}
	if (value == 0) {
		/*
		 * this path (reset toggle+halt) is needed to implement
		 * SET_INTERFACE on normal hardware.  but it can't be
		 * done from software on the PXA UDC, and the hardware
		 * forgets to do it as part of SET_INTERFACE automagic.
		 */
		printf("only host can clear %s halt\n", _ep->name);
		return -EROFS;
	}

	local_irq_save(flags);

	if ((ep->bEndpointAddress & USB_DIR_IN) != 0
			&& ((readl(ep->reg_udccs) & UDCCS_BI_TFS) == 0
			   || !list_empty(&ep->queue))) {
		local_irq_restore(flags);
		return -EAGAIN;
	}

	/* FST bit is the same for control, bulk in, bulk out, interrupt in */
	writel(UDCCS_BI_FST|UDCCS_BI_FTF, ep->reg_udccs);

	/* ep0 needs special care */
	if (!ep->desc) {
		start_watchdog(ep->dev);
		ep->dev->req_pending = 0;
		ep->dev->ep0state = EP0_STALL;

	/* and bulk/intr endpoints like dropping stalls too */
	} else {
		unsigned i;
		for (i = 0; i < 1000; i += 20) {
			if (readl(ep->reg_udccs) & UDCCS_BI_SST)
				break;
			udelay(20);
		}
	}
	local_irq_restore(flags);

	debug("%s halt\n", _ep->name);
	return 0;
}

static int pxa25x_ep_fifo_status(struct usb_ep *_ep)
{
	struct pxa25x_ep        *ep;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (!_ep) {
		printf("%s, bad ep\n", __func__);
		return -ENODEV;
	}
	/* pxa can't report unclaimed bytes from IN fifos */
	if ((ep->bEndpointAddress & USB_DIR_IN) != 0)
		return -EOPNOTSUPP;
	if (ep->dev->gadget.speed == USB_SPEED_UNKNOWN
			|| (readl(ep->reg_udccs) & UDCCS_BO_RFS) == 0)
		return 0;
	else
		return (readl(ep->reg_ubcr) & 0xfff) + 1;
}

static void pxa25x_ep_fifo_flush(struct usb_ep *_ep)
{
	struct pxa25x_ep        *ep;

	ep = container_of(_ep, struct pxa25x_ep, ep);
	if (!_ep || ep->ep.name == ep0name || !list_empty(&ep->queue)) {
		printf("%s, bad ep\n", __func__);
		return;
	}

	/* toggle and halt bits stay unchanged */

	/* for OUT, just read and discard the FIFO contents. */
	if ((ep->bEndpointAddress & USB_DIR_IN) == 0) {
		while (((readl(ep->reg_udccs)) & UDCCS_BO_RNE) != 0)
			(void)readb(ep->reg_uddr);
		return;
	}

	/* most IN status is the same, but ISO can't stall */
	writel(UDCCS_BI_TPC|UDCCS_BI_FTF|UDCCS_BI_TUR
		| (ep->bmAttributes == USB_ENDPOINT_XFER_ISOC
			? 0 : UDCCS_BI_SST), ep->reg_udccs);
}


static struct usb_ep_ops pxa25x_ep_ops = {
	.enable		= pxa25x_ep_enable,
	.disable	= pxa25x_ep_disable,

	.alloc_request	= pxa25x_ep_alloc_request,
	.free_request	= pxa25x_ep_free_request,

	.queue		= pxa25x_ep_queue,
	.dequeue	= pxa25x_ep_dequeue,

	.set_halt	= pxa25x_ep_set_halt,
	.fifo_status	= pxa25x_ep_fifo_status,
	.fifo_flush	= pxa25x_ep_fifo_flush,
};


/* ---------------------------------------------------------------------------
 *	device-scoped parts of the api to the usb controller hardware
 * ---------------------------------------------------------------------------
 */

static int pxa25x_udc_get_frame(struct usb_gadget *_gadget)
{
	return ((readl(&the_controller->regs->ufnrh) & 0x07) << 8) |
		(readl(&the_controller->regs->ufnrl) & 0xff);
}

static int pxa25x_udc_wakeup(struct usb_gadget *_gadget)
{
	/* host may not have enabled remote wakeup */
	if ((readl(&the_controller->regs->udccs[0]) & UDCCS0_DRWF) == 0)
		return -EHOSTUNREACH;
	udc_set_mask_UDCCR(UDCCR_RSM);
	return 0;
}

static void stop_activity(struct pxa25x_udc *, struct usb_gadget_driver *);
static void udc_enable(struct pxa25x_udc *);
static void udc_disable(struct pxa25x_udc *);

/*
 * We disable the UDC -- and its 48 MHz clock -- whenever it's not
 * in active use.
 */
static int pullup(struct pxa25x_udc *udc)
{
	if (udc->pullup)
		pullup_on();
	else
		pullup_off();


	int is_active = udc->pullup;
	if (is_active) {
		if (!udc->active) {
			udc->active = 1;
			udc_enable(udc);
		}
	} else {
		if (udc->active) {
			if (udc->gadget.speed != USB_SPEED_UNKNOWN)
				stop_activity(udc, udc->driver);
			udc_disable(udc);
			udc->active = 0;
		}

	}
	return 0;
}

/* VBUS reporting logically comes from a transceiver */
static int pxa25x_udc_vbus_session(struct usb_gadget *_gadget, int is_active)
{
	struct pxa25x_udc *udc;

	udc = container_of(_gadget, struct pxa25x_udc, gadget);
	printf("vbus %s\n", is_active ? "supplied" : "inactive");
	pullup(udc);
	return 0;
}

/* drivers may have software control over D+ pullup */
static int pxa25x_udc_pullup(struct usb_gadget *_gadget, int is_active)
{
	struct pxa25x_udc	*udc;

	udc = container_of(_gadget, struct pxa25x_udc, gadget);

	/* not all boards support pullup control */
	if (!udc->mach->udc_command)
		return -EOPNOTSUPP;

	udc->pullup = (is_active != 0);
	pullup(udc);
	return 0;
}

/*
 * boards may consume current from VBUS, up to 100-500mA based on config.
 * the 500uA suspend ceiling means that exclusively vbus-powered PXA designs
 * violate USB specs.
 */
static int pxa25x_udc_vbus_draw(struct usb_gadget *_gadget, unsigned mA)
{
	return -EOPNOTSUPP;
}

static const struct usb_gadget_ops pxa25x_udc_ops = {
	.get_frame	= pxa25x_udc_get_frame,
	.wakeup		= pxa25x_udc_wakeup,
	.vbus_session	= pxa25x_udc_vbus_session,
	.pullup		= pxa25x_udc_pullup,
	.vbus_draw	= pxa25x_udc_vbus_draw,
};

/*-------------------------------------------------------------------------*/

/*
 *	udc_disable - disable USB device controller
 */
static void udc_disable(struct pxa25x_udc *dev)
{
	/* block all irqs */
	udc_set_mask_UDCCR(UDCCR_SRM|UDCCR_REM);
	writel(0xff, &dev->regs->uicr0);
	writel(0xff, &dev->regs->uicr1);
	writel(UFNRH_SIM, &dev->regs->ufnrh);

	/* if hardware supports it, disconnect from usb */
	pullup_off();

	udc_clear_mask_UDCCR(UDCCR_UDE);

	ep0_idle(dev);
	dev->gadget.speed = USB_SPEED_UNKNOWN;
}

/*
 *	udc_reinit - initialize software state
 */
static void udc_reinit(struct pxa25x_udc *dev)
{
	u32 i;

	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	dev->ep0state = EP0_IDLE;

	/* basic endpoint records init */
	for (i = 0; i < PXA_UDC_NUM_ENDPOINTS; i++) {
		struct pxa25x_ep *ep = &dev->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);

		ep->desc = NULL;
		ep->stopped = 0;
		INIT_LIST_HEAD(&ep->queue);
		ep->pio_irqs = 0;
	}

	/* the rest was statically initialized, and is read-only */
}

/*
 * until it's enabled, this UDC should be completely invisible
 * to any USB host.
 */
static void udc_enable(struct pxa25x_udc *dev)
{
	debug("udc: enabling udc\n");

	udc_clear_mask_UDCCR(UDCCR_UDE);

	/*
	 * Try to clear these bits before we enable the udc.
	 * Do not touch reset ack bit, we would take care of it in
	 * interrupt handle routine
	 */
	udc_ack_int_UDCCR(UDCCR_SUSIR|UDCCR_RESIR);

	ep0_idle(dev);
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->stats.irqs = 0;

	/*
	 * sequence taken from chapter 12.5.10, PXA250 AppProcDevManual:
	 * - enable UDC
	 * - if RESET is already in progress, ack interrupt
	 * - unmask reset interrupt
	 */
	udc_set_mask_UDCCR(UDCCR_UDE);
	if (!(readl(&dev->regs->udccr) & UDCCR_UDA))
		udc_ack_int_UDCCR(UDCCR_RSTIR);

	if (dev->has_cfr /* UDC_RES2 is defined */) {
		/*
		 * pxa255 (a0+) can avoid a set_config race that could
		 * prevent gadget drivers from configuring correctly
		 */
		writel(UDCCFR_ACM | UDCCFR_MB1, &dev->regs->udccfr);
	}

	/* enable suspend/resume and reset irqs */
	udc_clear_mask_UDCCR(UDCCR_SRM | UDCCR_REM);

	/* enable ep0 irqs */
	clrbits_le32(&dev->regs->uicr0, UICR0_IM0);

	/* if hardware supports it, pullup D+ and wait for reset */
	pullup_on();
}

static inline void clear_ep_state(struct pxa25x_udc *dev)
{
	unsigned i;

	/*
	 * hardware SET_{CONFIGURATION,INTERFACE} automagic resets endpoint
	 * fifos, and pending transactions mustn't be continued in any case.
	 */
	for (i = 1; i < PXA_UDC_NUM_ENDPOINTS; i++)
		nuke(&dev->ep[i], -ECONNABORTED);
}

static void handle_ep0(struct pxa25x_udc *dev)
{
	u32 udccs0 = readl(&dev->regs->udccs[0]);
	struct pxa25x_ep *ep = &dev->ep[0];
	struct pxa25x_request *req;
	union {
		struct usb_ctrlrequest	r;
		u8			raw[8];
		u32			word[2];
	} u;

	if (list_empty(&ep->queue))
		req = NULL;
	else
		req = list_entry(ep->queue.next, struct pxa25x_request, queue);

	/* clear stall status */
	if (udccs0 & UDCCS0_SST) {
		nuke(ep, -EPIPE);
		writel(UDCCS0_SST, &dev->regs->udccs[0]);
		stop_watchdog(dev);
		ep0_idle(dev);
	}

	/* previous request unfinished?  non-error iff back-to-back ... */
	if ((udccs0 & UDCCS0_SA) != 0 && dev->ep0state != EP0_IDLE) {
		nuke(ep, 0);
		stop_watchdog(dev);
		ep0_idle(dev);
	}

	switch (dev->ep0state) {
	case EP0_IDLE:
		/* late-breaking status? */
		udccs0 = readl(&dev->regs->udccs[0]);

		/* start control request? */
		if (likely((udccs0 & (UDCCS0_OPR|UDCCS0_SA|UDCCS0_RNE))
				== (UDCCS0_OPR|UDCCS0_SA|UDCCS0_RNE))) {
			int i;

			nuke(ep, -EPROTO);

			/* read SETUP packet */
			for (i = 0; i < 8; i++) {
				if (unlikely(!(readl(&dev->regs->udccs[0]) &
						UDCCS0_RNE))) {
bad_setup:
					debug("SETUP %d!\n", i);
					goto stall;
				}
				u.raw[i] = (u8)readb(&dev->regs->uddr0);
			}
			if (unlikely((readl(&dev->regs->udccs[0]) &
					UDCCS0_RNE) != 0))
				goto bad_setup;

got_setup:
			debug("SETUP %02x.%02x v%04x i%04x l%04x\n",
				u.r.bRequestType, u.r.bRequest,
				le16_to_cpu(u.r.wValue),
				le16_to_cpu(u.r.wIndex),
				le16_to_cpu(u.r.wLength));

			/* cope with automagic for some standard requests. */
			dev->req_std = (u.r.bRequestType & USB_TYPE_MASK)
						== USB_TYPE_STANDARD;
			dev->req_config = 0;
			dev->req_pending = 1;
			switch (u.r.bRequest) {
			/* hardware restricts gadget drivers here! */
			case USB_REQ_SET_CONFIGURATION:
				debug("GOT SET_CONFIGURATION\n");
				if (u.r.bRequestType == USB_RECIP_DEVICE) {
					/*
					 * reflect hardware's automagic
					 * up to the gadget driver.
					 */
config_change:
					dev->req_config = 1;
					clear_ep_state(dev);
					/*
					 * if !has_cfr, there's no synch
					 * else use AREN (later) not SA|OPR
					 * USIR0_IR0 acts edge sensitive
					 */
				}
				break;
			/* ... and here, even more ... */
			case USB_REQ_SET_INTERFACE:
				if (u.r.bRequestType == USB_RECIP_INTERFACE) {
					/*
					 * udc hardware is broken by design:
					 *  - altsetting may only be zero;
					 *  - hw resets all interfaces' eps;
					 *  - ep reset doesn't include halt(?).
					 */
					printf("broken set_interface (%d/%d)\n",
						le16_to_cpu(u.r.wIndex),
						le16_to_cpu(u.r.wValue));
					goto config_change;
				}
				break;
			/* hardware was supposed to hide this */
			case USB_REQ_SET_ADDRESS:
				debug("GOT SET ADDRESS\n");
				if (u.r.bRequestType == USB_RECIP_DEVICE) {
					ep0start(dev, 0, "address");
					return;
				}
				break;
			}

			if (u.r.bRequestType & USB_DIR_IN)
				dev->ep0state = EP0_IN_DATA_PHASE;
			else
				dev->ep0state = EP0_OUT_DATA_PHASE;

			i = dev->driver->setup(&dev->gadget, &u.r);
			if (i < 0) {
				/* hardware automagic preventing STALL... */
				if (dev->req_config) {
					/*
					 * hardware sometimes neglects to tell
					 * tell us about config change events,
					 * so later ones may fail...
					 */
					printf("config change %02x fail %d?\n",
						u.r.bRequest, i);
					return;
					/*
					 * TODO experiment:  if has_cfr,
					 * hardware didn't ACK; maybe we
					 * could actually STALL!
					 */
				}
				if (0) {
stall:
					/* uninitialized when goto stall */
					i = 0;
				}
				debug("protocol STALL, "
					"%02x err %d\n",
					readl(&dev->regs->udccs[0]), i);

				/*
				 * the watchdog timer helps deal with cases
				 * where udc seems to clear FST wrongly, and
				 * then NAKs instead of STALLing.
				 */
				ep0start(dev, UDCCS0_FST|UDCCS0_FTF, "stall");
				start_watchdog(dev);
				dev->ep0state = EP0_STALL;

			/* deferred i/o == no response yet */
			} else if (dev->req_pending) {
				if (likely(dev->ep0state == EP0_IN_DATA_PHASE
						|| dev->req_std || u.r.wLength))
					ep0start(dev, 0, "defer");
				else
					ep0start(dev, UDCCS0_IPR, "defer/IPR");
			}

			/* expect at least one data or status stage irq */
			return;

		} else if (likely((udccs0 & (UDCCS0_OPR|UDCCS0_SA))
				== (UDCCS0_OPR|UDCCS0_SA))) {
			unsigned i;

			/*
			 * pxa210/250 erratum 131 for B0/B1 says RNE lies.
			 * still observed on a pxa255 a0.
			 */
			debug("e131\n");
			nuke(ep, -EPROTO);

			/* read SETUP data, but don't trust it too much */
			for (i = 0; i < 8; i++)
				u.raw[i] = (u8)readb(&dev->regs->uddr0);
			if ((u.r.bRequestType & USB_RECIP_MASK)
					> USB_RECIP_OTHER)
				goto stall;
			if (u.word[0] == 0 && u.word[1] == 0)
				goto stall;
			goto got_setup;
		} else {
			/*
			 * some random early IRQ:
			 * - we acked FST
			 * - IPR cleared
			 * - OPR got set, without SA (likely status stage)
			 */
			debug("random IRQ %X %X\n", udccs0,
				readl(&dev->regs->udccs[0]));
			writel(udccs0 & (UDCCS0_SA|UDCCS0_OPR),
				&dev->regs->udccs[0]);
		}
		break;
	case EP0_IN_DATA_PHASE:			/* GET_DESCRIPTOR etc */
		if (udccs0 & UDCCS0_OPR) {
			debug("ep0in premature status\n");
			if (req)
				done(ep, req, 0);
			ep0_idle(dev);
		} else /* irq was IPR clearing */ {
			if (req) {
				debug("next ep0 in packet\n");
				/* this IN packet might finish the request */
				(void) write_ep0_fifo(ep, req);
			} /* else IN token before response was written */
		}
		break;
	case EP0_OUT_DATA_PHASE:		/* SET_DESCRIPTOR etc */
		if (udccs0 & UDCCS0_OPR) {
			if (req) {
				/* this OUT packet might finish the request */
				if (read_ep0_fifo(ep, req))
					done(ep, req, 0);
				/* else more OUT packets expected */
			} /* else OUT token before read was issued */
		} else /* irq was IPR clearing */ {
			debug("ep0out premature status\n");
			if (req)
				done(ep, req, 0);
			ep0_idle(dev);
		}
		break;
	case EP0_END_XFER:
		if (req)
			done(ep, req, 0);
		/*
		 * ack control-IN status (maybe in-zlp was skipped)
		 * also appears after some config change events.
		 */
		if (udccs0 & UDCCS0_OPR)
			writel(UDCCS0_OPR, &dev->regs->udccs[0]);
		ep0_idle(dev);
		break;
	case EP0_STALL:
		writel(UDCCS0_FST, &dev->regs->udccs[0]);
		break;
	}

	writel(USIR0_IR0, &dev->regs->usir0);
}

static void handle_ep(struct pxa25x_ep *ep)
{
	struct pxa25x_request	*req;
	int			is_in = ep->bEndpointAddress & USB_DIR_IN;
	int			completed;
	u32			udccs, tmp;

	do {
		completed = 0;
		if (likely(!list_empty(&ep->queue)))
			req = list_entry(ep->queue.next,
					struct pxa25x_request, queue);
		else
			req = NULL;

		/* TODO check FST handling */

		udccs = readl(ep->reg_udccs);
		if (unlikely(is_in)) {	/* irq from TPC, SST, or (ISO) TUR */
			tmp = UDCCS_BI_TUR;
			if (likely(ep->bmAttributes == USB_ENDPOINT_XFER_BULK))
				tmp |= UDCCS_BI_SST;
			tmp &= udccs;
			if (likely(tmp))
				writel(tmp, ep->reg_udccs);
			if (req && likely((udccs & UDCCS_BI_TFS) != 0))
				completed = write_fifo(ep, req);

		} else {	/* irq from RPC (or for ISO, ROF) */
			if (likely(ep->bmAttributes == USB_ENDPOINT_XFER_BULK))
				tmp = UDCCS_BO_SST | UDCCS_BO_DME;
			else
				tmp = UDCCS_IO_ROF | UDCCS_IO_DME;
			tmp &= udccs;
			if (likely(tmp))
				writel(tmp, ep->reg_udccs);

			/* fifos can hold packets, ready for reading... */
			if (likely(req))
				completed = read_fifo(ep, req);
			else
				pio_irq_disable(ep->bEndpointAddress);
		}
		ep->pio_irqs++;
	} while (completed);
}

/*
 *	pxa25x_udc_irq - interrupt handler
 *
 * avoid delays in ep0 processing. the control handshaking isn't always
 * under software control (pxa250c0 and the pxa255 are better), and delays
 * could cause usb protocol errors.
 */
static struct pxa25x_udc memory;
static int
pxa25x_udc_irq(void)
{
	struct pxa25x_udc *dev = &memory;
	int handled;

	test_watchdog(dev);

	dev->stats.irqs++;
	do {
		u32 udccr = readl(&dev->regs->udccr);

		handled = 0;

		/* SUSpend Interrupt Request */
		if (unlikely(udccr & UDCCR_SUSIR)) {
			udc_ack_int_UDCCR(UDCCR_SUSIR);
			handled = 1;
			debug("USB suspend\n");

			if (dev->gadget.speed != USB_SPEED_UNKNOWN
					&& dev->driver
					&& dev->driver->suspend)
				dev->driver->suspend(&dev->gadget);
			ep0_idle(dev);
		}

		/* RESume Interrupt Request */
		if (unlikely(udccr & UDCCR_RESIR)) {
			udc_ack_int_UDCCR(UDCCR_RESIR);
			handled = 1;
			debug("USB resume\n");

			if (dev->gadget.speed != USB_SPEED_UNKNOWN
					&& dev->driver
					&& dev->driver->resume)
				dev->driver->resume(&dev->gadget);
		}

		/* ReSeT Interrupt Request - USB reset */
		if (unlikely(udccr & UDCCR_RSTIR)) {
			udc_ack_int_UDCCR(UDCCR_RSTIR);
			handled = 1;

			if ((readl(&dev->regs->udccr) & UDCCR_UDA) == 0) {
				debug("USB reset start\n");

				/*
				 * reset driver and endpoints,
				 * in case that's not yet done
				 */
				stop_activity(dev, dev->driver);

			} else {
				debug("USB reset end\n");
				dev->gadget.speed = USB_SPEED_FULL;
				memset(&dev->stats, 0, sizeof dev->stats);
				/* driver and endpoints are still reset */
			}

		} else {
			u32 uicr0 = readl(&dev->regs->uicr0);
			u32 uicr1 = readl(&dev->regs->uicr1);
			u32 usir0 = readl(&dev->regs->usir0);
			u32 usir1 = readl(&dev->regs->usir1);

			usir0 = usir0 & ~uicr0;
			usir1 = usir1 & ~uicr1;
			int i;

			if (unlikely(!usir0 && !usir1))
				continue;

			debug_cond(NOISY, "irq %02x.%02x\n", usir1, usir0);

			/* control traffic */
			if (usir0 & USIR0_IR0) {
				dev->ep[0].pio_irqs++;
				handle_ep0(dev);
				handled = 1;
			}

			/* endpoint data transfers */
			for (i = 0; i < 8; i++) {
				u32	tmp = 1 << i;

				if (i && (usir0 & tmp)) {
					handle_ep(&dev->ep[i]);
					setbits_le32(&dev->regs->usir0, tmp);
					handled = 1;
				}
#ifndef	CONFIG_USB_PXA25X_SMALL
				if (usir1 & tmp) {
					handle_ep(&dev->ep[i+8]);
					setbits_le32(&dev->regs->usir1, tmp);
					handled = 1;
				}
#endif
			}
		}

		/* we could also ask for 1 msec SOF (SIR) interrupts */

	} while (handled);
	return IRQ_HANDLED;
}

/*-------------------------------------------------------------------------*/

/*
 * this uses load-time allocation and initialization (instead of
 * doing it at run-time) to save code, eliminate fault paths, and
 * be more obviously correct.
 */
static struct pxa25x_udc memory = {
	.regs = UDC_REGS,

	.gadget = {
		.ops		= &pxa25x_udc_ops,
		.ep0		= &memory.ep[0].ep,
		.name		= driver_name,
	},

	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name		= ep0name,
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= EP0_FIFO_SIZE,
		},
		.dev		= &memory,
		.reg_udccs	= &UDC_REGS->udccs[0],
		.reg_uddr	= &UDC_REGS->uddr0,
	},

	/* first group of endpoints */
	.ep[1] = {
		.ep = {
			.name		= "ep1in-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[1],
		.reg_uddr	= &UDC_REGS->uddr1,
	},
	.ep[2] = {
		.ep = {
			.name		= "ep2out-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = 2,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[2],
		.reg_ubcr	= &UDC_REGS->ubcr2,
		.reg_uddr	= &UDC_REGS->uddr2,
	},
#ifndef CONFIG_USB_PXA25X_SMALL
	.ep[3] = {
		.ep = {
			.name		= "ep3in-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 3,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[3],
		.reg_uddr	= &UDC_REGS->uddr3,
	},
	.ep[4] = {
		.ep = {
			.name		= "ep4out-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = 4,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[4],
		.reg_ubcr	= &UDC_REGS->ubcr4,
		.reg_uddr	= &UDC_REGS->uddr4,
	},
	.ep[5] = {
		.ep = {
			.name		= "ep5in-int",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= INT_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= INT_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 5,
		.bmAttributes	= USB_ENDPOINT_XFER_INT,
		.reg_udccs	= &UDC_REGS->udccs[5],
		.reg_uddr	= &UDC_REGS->uddr5,
	},

	/* second group of endpoints */
	.ep[6] = {
		.ep = {
			.name		= "ep6in-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 6,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[6],
		.reg_uddr	= &UDC_REGS->uddr6,
	},
	.ep[7] = {
		.ep = {
			.name		= "ep7out-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = 7,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[7],
		.reg_ubcr	= &UDC_REGS->ubcr7,
		.reg_uddr	= &UDC_REGS->uddr7,
	},
	.ep[8] = {
		.ep = {
			.name		= "ep8in-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 8,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[8],
		.reg_uddr	= &UDC_REGS->uddr8,
	},
	.ep[9] = {
		.ep = {
			.name		= "ep9out-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = 9,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[9],
		.reg_ubcr	= &UDC_REGS->ubcr9,
		.reg_uddr	= &UDC_REGS->uddr9,
	},
	.ep[10] = {
		.ep = {
			.name		= "ep10in-int",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= INT_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= INT_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 10,
		.bmAttributes	= USB_ENDPOINT_XFER_INT,
		.reg_udccs	= &UDC_REGS->udccs[10],
		.reg_uddr	= &UDC_REGS->uddr10,
	},

	/* third group of endpoints */
	.ep[11] = {
		.ep = {
			.name		= "ep11in-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 11,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[11],
		.reg_uddr	= &UDC_REGS->uddr11,
	},
	.ep[12] = {
		.ep = {
			.name		= "ep12out-bulk",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= BULK_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= BULK_FIFO_SIZE,
		.bEndpointAddress = 12,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.reg_udccs	= &UDC_REGS->udccs[12],
		.reg_ubcr	= &UDC_REGS->ubcr12,
		.reg_uddr	= &UDC_REGS->uddr12,
	},
	.ep[13] = {
		.ep = {
			.name		= "ep13in-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 13,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[13],
		.reg_uddr	= &UDC_REGS->uddr13,
	},
	.ep[14] = {
		.ep = {
			.name		= "ep14out-iso",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= ISO_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= ISO_FIFO_SIZE,
		.bEndpointAddress = 14,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
		.reg_udccs	= &UDC_REGS->udccs[14],
		.reg_ubcr	= &UDC_REGS->ubcr14,
		.reg_uddr	= &UDC_REGS->uddr14,
	},
	.ep[15] = {
		.ep = {
			.name		= "ep15in-int",
			.ops		= &pxa25x_ep_ops,
			.maxpacket	= INT_FIFO_SIZE,
		},
		.dev		= &memory,
		.fifo_size	= INT_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 15,
		.bmAttributes	= USB_ENDPOINT_XFER_INT,
		.reg_udccs	= &UDC_REGS->udccs[15],
		.reg_uddr	= &UDC_REGS->uddr15,
	},
#endif /* !CONFIG_USB_PXA25X_SMALL */
};

static void udc_command(int cmd)
{
	switch (cmd) {
	case PXA2XX_UDC_CMD_CONNECT:
		setbits_le32(GPDR(CONFIG_USB_DEV_PULLUP_GPIO),
			GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO));

		/* enable pullup */
		writel(GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO),
			GPCR(CONFIG_USB_DEV_PULLUP_GPIO));

		debug("Connected to USB\n");
		break;

	case PXA2XX_UDC_CMD_DISCONNECT:
		/* disable pullup resistor */
		writel(GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO),
			GPSR(CONFIG_USB_DEV_PULLUP_GPIO));

		/* setup pin as input, line will float */
		clrbits_le32(GPDR(CONFIG_USB_DEV_PULLUP_GPIO),
			GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO));

		debug("Disconnected from USB\n");
		break;
	}
}

static struct pxa2xx_udc_mach_info mach_info = {
	.udc_command = udc_command,
};

/*
 * when a driver is successfully registered, it will receive
 * control requests including set_configuration(), which enables
 * non-control requests.  then usb traffic follows until a
 * disconnect is reported.  then a host may connect again, or
 * the driver might get unbound.
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct pxa25x_udc *dev = &memory;
	int retval;
	uint32_t chiprev;

	if (!driver
			|| driver->speed < USB_SPEED_FULL
			|| !driver->disconnect
			|| !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;

	/* Enable clock for usb controller */
	setbits_le32(CKEN, CKEN11_USB);

	/* first hook up the driver ... */
	dev->driver = driver;
	dev->pullup = 1;

	/* trigger chiprev-specific logic */
	switch ((chiprev = pxa_get_cpu_revision())) {
	case PXA255_A0:
		dev->has_cfr = 1;
		break;
	case PXA250_A0:
	case PXA250_A1:
		/* A0/A1 "not released"; ep 13, 15 unusable */
		/* fall through */
	case PXA250_B2: case PXA210_B2:
	case PXA250_B1: case PXA210_B1:
	case PXA250_B0: case PXA210_B0:
		/* OUT-DMA is broken ... */
		/* fall through */
	case PXA250_C0: case PXA210_C0:
		break;
	default:
		printf("%s: unrecognized processor: %08x\n",
			DRIVER_NAME, chiprev);
		return -ENODEV;
	}

	the_controller = dev;

	/* prepare watchdog timer */
	dev->watchdog.running = 0;
	dev->watchdog.period = 5000 * CONFIG_SYS_HZ / 1000000; /* 5 ms */
	dev->watchdog.function = udc_watchdog;

	dev->mach = &mach_info;

	udc_disable(dev);
	udc_reinit(dev);

	dev->gadget.name = "pxa2xx_udc";
	retval = driver->bind(&dev->gadget);
	if (retval) {
		printf("bind to driver %s --> error %d\n",
				DRIVER_NAME, retval);
		dev->driver = NULL;
		return retval;
	}

	/*
	 * ... then enable host detection and ep0; and we're ready
	 * for set_configuration as well as eventual disconnect.
	 */
	printf("registered gadget driver '%s'\n", DRIVER_NAME);

	pullup(dev);
	dump_state(dev);
	return 0;
}

static void
stop_activity(struct pxa25x_udc *dev, struct usb_gadget_driver *driver)
{
	int i;

	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = NULL;
	dev->gadget.speed = USB_SPEED_UNKNOWN;

	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < PXA_UDC_NUM_ENDPOINTS; i++) {
		struct pxa25x_ep *ep = &dev->ep[i];

		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}
	stop_watchdog(dev);

	/* report disconnect; the driver is already quiesced */
	if (driver)
		driver->disconnect(&dev->gadget);

	/* re-init driver-visible data structures */
	udc_reinit(dev);
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct pxa25x_udc	*dev = the_controller;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver || !driver->unbind)
		return -EINVAL;

	local_irq_disable();
	dev->pullup = 0;
	pullup(dev);
	stop_activity(dev, driver);
	local_irq_enable();

	driver->unbind(&dev->gadget);
	dev->driver = NULL;

	printf("unregistered gadget driver '%s'\n", DRIVER_NAME);
	dump_state(dev);

	the_controller = NULL;

	clrbits_le32(CKEN, CKEN11_USB);

	return 0;
}

extern void udc_disconnect(void)
{
	setbits_le32(CKEN, CKEN11_USB);
	udc_clear_mask_UDCCR(UDCCR_UDE);
	udc_command(PXA2XX_UDC_CMD_DISCONNECT);
	clrbits_le32(CKEN, CKEN11_USB);
}

/*-------------------------------------------------------------------------*/

extern int
usb_gadget_handle_interrupts(int index)
{
	return pxa25x_udc_irq();
}
