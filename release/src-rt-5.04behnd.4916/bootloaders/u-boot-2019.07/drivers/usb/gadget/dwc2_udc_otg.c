// SPDX-License-Identifier: GPL-2.0+
/*
 * drivers/usb/gadget/dwc2_udc_otg.c
 * Designware DWC2 on-chip full/high speed USB OTG 2.0 device controllers
 *
 * Copyright (C) 2008 for Samsung Electronics
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
#undef DEBUG
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <reset.h>

#include <linux/errno.h>
#include <linux/list.h>

#include <linux/usb/ch9.h>
#include <linux/usb/otg.h>
#include <linux/usb/gadget.h>

#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <asm/io.h>

#include <asm/mach-types.h>

#include <power/regulator.h>

#include "dwc2_udc_otg_regs.h"
#include "dwc2_udc_otg_priv.h"

/***********************************************************/

#define OTG_DMA_MODE		1

#define DEBUG_SETUP 0
#define DEBUG_EP0 0
#define DEBUG_ISR 0
#define DEBUG_OUT_EP 0
#define DEBUG_IN_EP 0

#include <usb/dwc2_udc.h>

#define EP0_CON		0
#define EP_MASK		0xF

static char *state_names[] = {
	"WAIT_FOR_SETUP",
	"DATA_STATE_XMIT",
	"DATA_STATE_NEED_ZLP",
	"WAIT_FOR_OUT_STATUS",
	"DATA_STATE_RECV",
	"WAIT_FOR_COMPLETE",
	"WAIT_FOR_OUT_COMPLETE",
	"WAIT_FOR_IN_COMPLETE",
	"WAIT_FOR_NULL_COMPLETE",
};

#define DRIVER_VERSION "15 March 2009"

struct dwc2_udc	*the_controller;

static const char driver_name[] = "dwc2-udc";
static const char ep0name[] = "ep0-control";

/* Max packet size*/
static unsigned int ep0_fifo_size = 64;
static unsigned int ep_fifo_size =  512;
static unsigned int ep_fifo_size2 = 1024;
static int reset_available = 1;

static struct usb_ctrlrequest *usb_ctrl;
static dma_addr_t usb_ctrl_dma_addr;

/*
  Local declarations.
*/
static int dwc2_ep_enable(struct usb_ep *ep,
			 const struct usb_endpoint_descriptor *);
static int dwc2_ep_disable(struct usb_ep *ep);
static struct usb_request *dwc2_alloc_request(struct usb_ep *ep,
					     gfp_t gfp_flags);
static void dwc2_free_request(struct usb_ep *ep, struct usb_request *);

static int dwc2_queue(struct usb_ep *ep, struct usb_request *, gfp_t gfp_flags);
static int dwc2_dequeue(struct usb_ep *ep, struct usb_request *);
static int dwc2_fifo_status(struct usb_ep *ep);
static void dwc2_fifo_flush(struct usb_ep *ep);
static void dwc2_ep0_read(struct dwc2_udc *dev);
static void dwc2_ep0_kick(struct dwc2_udc *dev, struct dwc2_ep *ep);
static void dwc2_handle_ep0(struct dwc2_udc *dev);
static int dwc2_ep0_write(struct dwc2_udc *dev);
static int write_fifo_ep0(struct dwc2_ep *ep, struct dwc2_request *req);
static void done(struct dwc2_ep *ep, struct dwc2_request *req, int status);
static void stop_activity(struct dwc2_udc *dev,
			  struct usb_gadget_driver *driver);
static int udc_enable(struct dwc2_udc *dev);
static void udc_set_address(struct dwc2_udc *dev, unsigned char address);
static void reconfig_usbd(struct dwc2_udc *dev);
static void set_max_pktsize(struct dwc2_udc *dev, enum usb_device_speed speed);
static void nuke(struct dwc2_ep *ep, int status);
static int dwc2_udc_set_halt(struct usb_ep *_ep, int value);
static void dwc2_udc_set_nak(struct dwc2_ep *ep);

void set_udc_gadget_private_data(void *p)
{
	debug_cond(DEBUG_SETUP != 0,
		   "%s: the_controller: 0x%p, p: 0x%p\n", __func__,
		   the_controller, p);
	the_controller->gadget.dev.device_data = p;
}

void *get_udc_gadget_private_data(struct usb_gadget *gadget)
{
	return gadget->dev.device_data;
}

static struct usb_ep_ops dwc2_ep_ops = {
	.enable = dwc2_ep_enable,
	.disable = dwc2_ep_disable,

	.alloc_request = dwc2_alloc_request,
	.free_request = dwc2_free_request,

	.queue = dwc2_queue,
	.dequeue = dwc2_dequeue,

	.set_halt = dwc2_udc_set_halt,
	.fifo_status = dwc2_fifo_status,
	.fifo_flush = dwc2_fifo_flush,
};

#define create_proc_files() do {} while (0)
#define remove_proc_files() do {} while (0)

/***********************************************************/

struct dwc2_usbotg_reg *reg;

bool dfu_usb_get_reset(void)
{
	return !!(readl(&reg->gintsts) & INT_RESET);
}

__weak void otg_phy_init(struct dwc2_udc *dev) {}
__weak void otg_phy_off(struct dwc2_udc *dev) {}

/***********************************************************/

#include "dwc2_udc_otg_xfer_dma.c"

/*
 *	udc_disable - disable USB device controller
 */
static void udc_disable(struct dwc2_udc *dev)
{
	debug_cond(DEBUG_SETUP != 0, "%s: %p\n", __func__, dev);

	udc_set_address(dev, 0);

	dev->ep0state = WAIT_FOR_SETUP;
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->usb_address = 0;

	otg_phy_off(dev);
}

/*
 *	udc_reinit - initialize software state
 */
static void udc_reinit(struct dwc2_udc *dev)
{
	unsigned int i;

	debug_cond(DEBUG_SETUP != 0, "%s: %p\n", __func__, dev);

	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	dev->ep0state = WAIT_FOR_SETUP;

	/* basic endpoint records init */
	for (i = 0; i < DWC2_MAX_ENDPOINTS; i++) {
		struct dwc2_ep *ep = &dev->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);

		ep->desc = 0;
		ep->stopped = 0;
		INIT_LIST_HEAD(&ep->queue);
		ep->pio_irqs = 0;
	}

	/* the rest was statically initialized, and is read-only */
}

#define BYTES2MAXP(x)	(x / 8)
#define MAXP2BYTES(x)	(x * 8)

/* until it's enabled, this UDC should be completely invisible
 * to any USB host.
 */
static int udc_enable(struct dwc2_udc *dev)
{
	debug_cond(DEBUG_SETUP != 0, "%s: %p\n", __func__, dev);

	otg_phy_init(dev);
	reconfig_usbd(dev);

	debug_cond(DEBUG_SETUP != 0,
		   "DWC2 USB 2.0 OTG Controller Core Initialized : 0x%x\n",
		    readl(&reg->gintmsk));

	dev->gadget.speed = USB_SPEED_UNKNOWN;

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB_GADGET)
/*
  Register entry point for the peripheral controller driver.
*/
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct dwc2_udc *dev = the_controller;
	int retval = 0;
	unsigned long flags = 0;

	debug_cond(DEBUG_SETUP != 0, "%s: %s\n", __func__, "no name");

	if (!driver
	    || (driver->speed != USB_SPEED_FULL
		&& driver->speed != USB_SPEED_HIGH)
	    || !driver->bind || !driver->disconnect || !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;

	spin_lock_irqsave(&dev->lock, flags);
	/* first hook up the driver ... */
	dev->driver = driver;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (retval) { /* TODO */
		printf("target device_add failed, error %d\n", retval);
		return retval;
	}

	retval = driver->bind(&dev->gadget);
	if (retval) {
		debug_cond(DEBUG_SETUP != 0,
			   "%s: bind to driver --> error %d\n",
			    dev->gadget.name, retval);
		dev->driver = 0;
		return retval;
	}

	enable_irq(IRQ_OTG);

	debug_cond(DEBUG_SETUP != 0,
		   "Registered gadget driver %s\n", dev->gadget.name);
	udc_enable(dev);

	return 0;
}

/*
 * Unregister entry point for the peripheral controller driver.
 */
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct dwc2_udc *dev = the_controller;
	unsigned long flags = 0;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);
	dev->driver = 0;
	stop_activity(dev, driver);
	spin_unlock_irqrestore(&dev->lock, flags);

	driver->unbind(&dev->gadget);

	disable_irq(IRQ_OTG);

	udc_disable(dev);
	return 0;
}
#else /* !CONFIG_IS_ENABLED(DM_USB_GADGET) */

static int dwc2_gadget_start(struct usb_gadget *g,
			     struct usb_gadget_driver *driver)
{
	struct dwc2_udc *dev = the_controller;

	debug_cond(DEBUG_SETUP != 0, "%s: %s\n", __func__, "no name");

	if (!driver ||
	    (driver->speed != USB_SPEED_FULL &&
	     driver->speed != USB_SPEED_HIGH) ||
	    !driver->bind || !driver->disconnect || !driver->setup)
		return -EINVAL;

	if (!dev)
		return -ENODEV;

	if (dev->driver)
		return -EBUSY;

	/* first hook up the driver ... */
	dev->driver = driver;

	debug_cond(DEBUG_SETUP != 0,
		   "Registered gadget driver %s\n", dev->gadget.name);
	return udc_enable(dev);
}

static int dwc2_gadget_stop(struct usb_gadget *g)
{
	struct dwc2_udc *dev = the_controller;

	if (!dev)
		return -ENODEV;

	if (!dev->driver)
		return -EINVAL;

	dev->driver = 0;
	stop_activity(dev, dev->driver);

	udc_disable(dev);

	return 0;
}

#endif /* !CONFIG_IS_ENABLED(DM_USB_GADGET) */

/*
 *	done - retire a request; caller blocked irqs
 */
static void done(struct dwc2_ep *ep, struct dwc2_request *req, int status)
{
	unsigned int stopped = ep->stopped;

	debug("%s: %s %p, req = %p, stopped = %d\n",
	      __func__, ep->ep.name, ep, &req->req, stopped);

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN) {
		debug("complete %s req %p stat %d len %u/%u\n",
		      ep->ep.name, &req->req, status,
		      req->req.actual, req->req.length);
	}

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

#ifdef DEBUG
	printf("calling complete callback\n");
	{
		int i, len = req->req.length;

		printf("pkt[%d] = ", req->req.length);
		if (len > 64)
			len = 64;
		for (i = 0; i < len; i++) {
			printf("%02x", ((u8 *)req->req.buf)[i]);
			if ((i & 7) == 7)
				printf(" ");
		}
		printf("\n");
	}
#endif
	spin_unlock(&ep->dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->dev->lock);

	debug("callback completed\n");

	ep->stopped = stopped;
}

/*
 *	nuke - dequeue ALL requests
 */
static void nuke(struct dwc2_ep *ep, int status)
{
	struct dwc2_request *req;

	debug("%s: %s %p\n", __func__, ep->ep.name, ep);

	/* called with irqs blocked */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct dwc2_request, queue);
		done(ep, req, status);
	}
}

static void stop_activity(struct dwc2_udc *dev,
			  struct usb_gadget_driver *driver)
{
	int i;

	/* don't disconnect drivers more than once */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;
	dev->gadget.speed = USB_SPEED_UNKNOWN;

	/* prevent new request submissions, kill any outstanding requests  */
	for (i = 0; i < DWC2_MAX_ENDPOINTS; i++) {
		struct dwc2_ep *ep = &dev->ep[i];
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	/* re-init driver-visible data structures */
	udc_reinit(dev);
}

static void reconfig_usbd(struct dwc2_udc *dev)
{
	/* 2. Soft-reset OTG Core and then unreset again. */
	int i;
	unsigned int uTemp = writel(CORE_SOFT_RESET, &reg->grstctl);
	uint32_t dflt_gusbcfg;
	uint32_t rx_fifo_sz, tx_fifo_sz, np_tx_fifo_sz;
	u32 max_hw_ep;
	int pdata_hw_ep;

	debug("Reseting OTG controller\n");

	dflt_gusbcfg =
		0<<15		/* PHY Low Power Clock sel*/
		|1<<14		/* Non-Periodic TxFIFO Rewind Enable*/
		|0x5<<10	/* Turnaround time*/
		|0<<9 | 0<<8	/* [0:HNP disable,1:HNP enable][ 0:SRP disable*/
				/* 1:SRP enable] H1= 1,1*/
		|0<<7		/* Ulpi DDR sel*/
		|0<<6		/* 0: high speed utmi+, 1: full speed serial*/
		|0<<4		/* 0: utmi+, 1:ulpi*/
#ifdef CONFIG_USB_GADGET_DWC2_OTG_PHY_BUS_WIDTH_8
		|0<<3		/* phy i/f  0:8bit, 1:16bit*/
#else
		|1<<3		/* phy i/f  0:8bit, 1:16bit*/
#endif
		|0x7<<0;	/* HS/FS Timeout**/

	if (dev->pdata->usb_gusbcfg)
		dflt_gusbcfg = dev->pdata->usb_gusbcfg;

	writel(dflt_gusbcfg, &reg->gusbcfg);

	/* 3. Put the OTG device core in the disconnected state.*/
	uTemp = readl(&reg->dctl);
	uTemp |= SOFT_DISCONNECT;
	writel(uTemp, &reg->dctl);

	udelay(20);

	/* 4. Make the OTG device core exit from the disconnected state.*/
	uTemp = readl(&reg->dctl);
	uTemp = uTemp & ~SOFT_DISCONNECT;
	writel(uTemp, &reg->dctl);

	/* 5. Configure OTG Core to initial settings of device mode.*/
	/* [][1: full speed(30Mhz) 0:high speed]*/
	writel(EP_MISS_CNT(1) | DEV_SPEED_HIGH_SPEED_20, &reg->dcfg);

	mdelay(1);

	/* 6. Unmask the core interrupts*/
	writel(GINTMSK_INIT, &reg->gintmsk);

	/* 7. Set NAK bit of EP0, EP1, EP2*/
	writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->out_endp[EP0_CON].doepctl);
	writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->in_endp[EP0_CON].diepctl);

	for (i = 1; i < DWC2_MAX_ENDPOINTS; i++) {
		writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->out_endp[i].doepctl);
		writel(DEPCTL_EPDIS|DEPCTL_SNAK, &reg->in_endp[i].diepctl);
	}

	/* 8. Unmask EPO interrupts*/
	writel(((1 << EP0_CON) << DAINT_OUT_BIT)
	       | (1 << EP0_CON), &reg->daintmsk);

	/* 9. Unmask device OUT EP common interrupts*/
	writel(DOEPMSK_INIT, &reg->doepmsk);

	/* 10. Unmask device IN EP common interrupts*/
	writel(DIEPMSK_INIT, &reg->diepmsk);

	rx_fifo_sz = RX_FIFO_SIZE;
	np_tx_fifo_sz = NPTX_FIFO_SIZE;
	tx_fifo_sz = PTX_FIFO_SIZE;

	if (dev->pdata->rx_fifo_sz)
		rx_fifo_sz = dev->pdata->rx_fifo_sz;
	if (dev->pdata->np_tx_fifo_sz)
		np_tx_fifo_sz = dev->pdata->np_tx_fifo_sz;
	if (dev->pdata->tx_fifo_sz)
		tx_fifo_sz = dev->pdata->tx_fifo_sz;

	/* 11. Set Rx FIFO Size (in 32-bit words) */
	writel(rx_fifo_sz, &reg->grxfsiz);

	/* 12. Set Non Periodic Tx FIFO Size */
	writel((np_tx_fifo_sz << 16) | rx_fifo_sz,
	       &reg->gnptxfsiz);

	/* retrieve the number of IN Endpoints (excluding ep0) */
	max_hw_ep = (readl(&reg->ghwcfg4) & GHWCFG4_NUM_IN_EPS_MASK) >>
		    GHWCFG4_NUM_IN_EPS_SHIFT;
	pdata_hw_ep = dev->pdata->tx_fifo_sz_nb;

	/* tx_fifo_sz_nb should equal to number of IN Endpoint */
	if (pdata_hw_ep && max_hw_ep != pdata_hw_ep)
		pr_warn("Got %d hw endpoint but %d tx-fifo-size in array !!\n",
			max_hw_ep, pdata_hw_ep);

	for (i = 0; i < max_hw_ep; i++) {
		if (pdata_hw_ep)
			tx_fifo_sz = dev->pdata->tx_fifo_sz_array[i];

		writel((rx_fifo_sz + np_tx_fifo_sz + (tx_fifo_sz * i)) |
			tx_fifo_sz << 16, &reg->dieptxf[i]);
	}
	/* Flush the RX FIFO */
	writel(RX_FIFO_FLUSH, &reg->grstctl);
	while (readl(&reg->grstctl) & RX_FIFO_FLUSH)
		debug("%s: waiting for DWC2_UDC_OTG_GRSTCTL\n", __func__);

	/* Flush all the Tx FIFO's */
	writel(TX_FIFO_FLUSH_ALL, &reg->grstctl);
	writel(TX_FIFO_FLUSH_ALL | TX_FIFO_FLUSH, &reg->grstctl);
	while (readl(&reg->grstctl) & TX_FIFO_FLUSH)
		debug("%s: waiting for DWC2_UDC_OTG_GRSTCTL\n", __func__);

	/* 13. Clear NAK bit of EP0, EP1, EP2*/
	/* For Slave mode*/
	/* EP0: Control OUT */
	writel(DEPCTL_EPDIS | DEPCTL_CNAK,
	       &reg->out_endp[EP0_CON].doepctl);

	/* 14. Initialize OTG Link Core.*/
	writel(GAHBCFG_INIT, &reg->gahbcfg);
}

static void set_max_pktsize(struct dwc2_udc *dev, enum usb_device_speed speed)
{
	unsigned int ep_ctrl;
	int i;

	if (speed == USB_SPEED_HIGH) {
		ep0_fifo_size = 64;
		ep_fifo_size = 512;
		ep_fifo_size2 = 1024;
		dev->gadget.speed = USB_SPEED_HIGH;
	} else {
		ep0_fifo_size = 64;
		ep_fifo_size = 64;
		ep_fifo_size2 = 64;
		dev->gadget.speed = USB_SPEED_FULL;
	}

	dev->ep[0].ep.maxpacket = ep0_fifo_size;
	for (i = 1; i < DWC2_MAX_ENDPOINTS; i++)
		dev->ep[i].ep.maxpacket = ep_fifo_size;

	/* EP0 - Control IN (64 bytes)*/
	ep_ctrl = readl(&reg->in_endp[EP0_CON].diepctl);
	writel(ep_ctrl|(0<<0), &reg->in_endp[EP0_CON].diepctl);

	/* EP0 - Control OUT (64 bytes)*/
	ep_ctrl = readl(&reg->out_endp[EP0_CON].doepctl);
	writel(ep_ctrl|(0<<0), &reg->out_endp[EP0_CON].doepctl);
}

static int dwc2_ep_enable(struct usb_ep *_ep,
			 const struct usb_endpoint_descriptor *desc)
{
	struct dwc2_ep *ep;
	struct dwc2_udc *dev;
	unsigned long flags = 0;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct dwc2_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
	    || desc->bDescriptorType != USB_DT_ENDPOINT
	    || ep->bEndpointAddress != desc->bEndpointAddress
	    || ep_maxpacket(ep) <
	    le16_to_cpu(get_unaligned(&desc->wMaxPacketSize))) {

		debug("%s: bad ep or descriptor\n", __func__);
		return -EINVAL;
	}

	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
	    && ep->bmAttributes != USB_ENDPOINT_XFER_BULK
	    && desc->bmAttributes != USB_ENDPOINT_XFER_INT) {

		debug("%s: %s type mismatch\n", __func__, _ep->name);
		return -EINVAL;
	}

	/* hardware _could_ do smaller, but driver doesn't */
	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK &&
	     le16_to_cpu(get_unaligned(&desc->wMaxPacketSize)) >
	     ep_maxpacket(ep)) || !get_unaligned(&desc->wMaxPacketSize)) {

		debug("%s: bad %s maxpacket\n", __func__, _ep->name);
		return -ERANGE;
	}

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {

		debug("%s: bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	ep->stopped = 0;
	ep->desc = desc;
	ep->pio_irqs = 0;
	ep->ep.maxpacket = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	/* Reset halt state */
	dwc2_udc_set_nak(ep);
	dwc2_udc_set_halt(_ep, 0);

	spin_lock_irqsave(&ep->dev->lock, flags);
	dwc2_udc_ep_activate(ep);
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	debug("%s: enabled %s, stopped = %d, maxpacket = %d\n",
	      __func__, _ep->name, ep->stopped, ep->ep.maxpacket);
	return 0;
}

/*
 * Disable EP
 */
static int dwc2_ep_disable(struct usb_ep *_ep)
{
	struct dwc2_ep *ep;
	unsigned long flags = 0;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct dwc2_ep, ep);
	if (!_ep || !ep->desc) {
		debug("%s: %s not enabled\n", __func__,
		      _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* Nuke all pending requests */
	nuke(ep, -ESHUTDOWN);

	ep->desc = 0;
	ep->stopped = 1;

	spin_unlock_irqrestore(&ep->dev->lock, flags);

	debug("%s: disabled %s\n", __func__, _ep->name);
	return 0;
}

static struct usb_request *dwc2_alloc_request(struct usb_ep *ep,
					     gfp_t gfp_flags)
{
	struct dwc2_request *req;

	debug("%s: %s %p\n", __func__, ep->name, ep);

	req = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*req));
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	INIT_LIST_HEAD(&req->queue);

	return &req->req;
}

static void dwc2_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct dwc2_request *req;

	debug("%s: %p\n", __func__, ep);

	req = container_of(_req, struct dwc2_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/* dequeue JUST ONE request */
static int dwc2_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct dwc2_ep *ep;
	struct dwc2_request *req;
	unsigned long flags = 0;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct dwc2_ep, ep);
	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;

	spin_lock_irqsave(&ep->dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ep->dev->lock, flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);

	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return 0;
}

/*
 * Return bytes in EP FIFO
 */
static int dwc2_fifo_status(struct usb_ep *_ep)
{
	int count = 0;
	struct dwc2_ep *ep;

	ep = container_of(_ep, struct dwc2_ep, ep);
	if (!_ep) {
		debug("%s: bad ep\n", __func__);
		return -ENODEV;
	}

	debug("%s: %d\n", __func__, ep_index(ep));

	/* LPD can't report unclaimed bytes from IN fifos */
	if (ep_is_in(ep))
		return -EOPNOTSUPP;

	return count;
}

/*
 * Flush EP FIFO
 */
static void dwc2_fifo_flush(struct usb_ep *_ep)
{
	struct dwc2_ep *ep;

	ep = container_of(_ep, struct dwc2_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		debug("%s: bad ep\n", __func__);
		return;
	}

	debug("%s: %d\n", __func__, ep_index(ep));
}

static const struct usb_gadget_ops dwc2_udc_ops = {
	/* current versions must always be self-powered */
#if CONFIG_IS_ENABLED(DM_USB_GADGET)
	.udc_start		= dwc2_gadget_start,
	.udc_stop		= dwc2_gadget_stop,
#endif
};

static struct dwc2_udc memory = {
	.usb_address = 0,
	.gadget = {
		.ops = &dwc2_udc_ops,
		.ep0 = &memory.ep[0].ep,
		.name = driver_name,
	},

	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name = ep0name,
			.ops = &dwc2_ep_ops,
			.maxpacket = EP0_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = 0,
		.bmAttributes = 0,

		.ep_type = ep_control,
	},

	/* first group of endpoints */
	.ep[1] = {
		.ep = {
			.name = "ep1in-bulk",
			.ops = &dwc2_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_out,
		.fifo_num = 1,
	},

	.ep[2] = {
		.ep = {
			.name = "ep2out-bulk",
			.ops = &dwc2_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_OUT | 2,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,

		.ep_type = ep_bulk_in,
		.fifo_num = 2,
	},

	.ep[3] = {
		.ep = {
			.name = "ep3in-int",
			.ops = &dwc2_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,

		.bEndpointAddress = USB_DIR_IN | 3,
		.bmAttributes = USB_ENDPOINT_XFER_INT,

		.ep_type = ep_interrupt,
		.fifo_num = 3,
	},
};

/*
 *	probe - binds to the platform device
 */

int dwc2_udc_probe(struct dwc2_plat_otg_data *pdata)
{
	struct dwc2_udc *dev = &memory;
	int retval = 0;

	debug("%s: %p\n", __func__, pdata);

	dev->pdata = pdata;

	reg = (struct dwc2_usbotg_reg *)pdata->regs_otg;

	dev->gadget.is_dualspeed = 1;	/* Hack only*/
	dev->gadget.is_otg = 0;
	dev->gadget.is_a_peripheral = 0;
	dev->gadget.b_hnp_enable = 0;
	dev->gadget.a_hnp_support = 0;
	dev->gadget.a_alt_hnp_support = 0;

	the_controller = dev;

	usb_ctrl = memalign(CONFIG_SYS_CACHELINE_SIZE,
			    ROUND(sizeof(struct usb_ctrlrequest),
				  CONFIG_SYS_CACHELINE_SIZE));
	if (!usb_ctrl) {
		pr_err("No memory available for UDC!\n");
		return -ENOMEM;
	}

	usb_ctrl_dma_addr = (dma_addr_t) usb_ctrl;

	udc_reinit(dev);

	return retval;
}

int dwc2_udc_handle_interrupt(void)
{
	u32 intr_status = readl(&reg->gintsts);
	u32 gintmsk = readl(&reg->gintmsk);

	if (intr_status & gintmsk)
		return dwc2_udc_irq(1, (void *)the_controller);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB_GADGET)

int usb_gadget_handle_interrupts(int index)
{
	return dwc2_udc_handle_interrupt();
}

#else /* CONFIG_IS_ENABLED(DM_USB_GADGET) */

struct dwc2_priv_data {
	struct clk_bulk		clks;
	struct reset_ctl_bulk	resets;
	struct phy *phys;
	int num_phys;
	struct udevice *usb33d_supply;
};

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	return dwc2_udc_handle_interrupt();
}

int dwc2_phy_setup(struct udevice *dev, struct phy **array, int *num_phys)
{
	int i, ret, count;
	struct phy *usb_phys;

	/* Return if no phy declared */
	if (!dev_read_prop(dev, "phys", NULL))
		return 0;

	count = dev_count_phandle_with_args(dev, "phys", "#phy-cells");
	if (count <= 0)
		return count;

	usb_phys = devm_kcalloc(dev, count, sizeof(struct phy),
				GFP_KERNEL);
	if (!usb_phys)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = generic_phy_get_by_index(dev, i, &usb_phys[i]);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Failed to get USB PHY%d for %s\n",
				i, dev->name);
			return ret;
		}
	}

	for (i = 0; i < count; i++) {
		ret = generic_phy_init(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't init USB PHY%d for %s\n",
				i, dev->name);
			goto phys_init_err;
		}
	}

	for (i = 0; i < count; i++) {
		ret = generic_phy_power_on(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't power USB PHY%d for %s\n",
				i, dev->name);
			goto phys_poweron_err;
		}
	}

	*array = usb_phys;
	*num_phys =  count;

	return 0;

phys_poweron_err:
	for (i = count - 1; i >= 0; i--)
		generic_phy_power_off(&usb_phys[i]);

	for (i = 0; i < count; i++)
		generic_phy_exit(&usb_phys[i]);

	return ret;

phys_init_err:
	for (; i >= 0; i--)
		generic_phy_exit(&usb_phys[i]);

	return ret;
}

void dwc2_phy_shutdown(struct udevice *dev, struct phy *usb_phys, int num_phys)
{
	int i, ret;

	for (i = 0; i < num_phys; i++) {
		if (!generic_phy_valid(&usb_phys[i]))
			continue;

		ret = generic_phy_power_off(&usb_phys[i]);
		ret |= generic_phy_exit(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't shutdown USB PHY%d for %s\n",
				i, dev->name);
		}
	}
}

static int dwc2_udc_otg_ofdata_to_platdata(struct udevice *dev)
{
	struct dwc2_plat_otg_data *platdata = dev_get_platdata(dev);
	int node = dev_of_offset(dev);
	ulong drvdata;
	void (*set_params)(struct dwc2_plat_otg_data *data);
	int ret;

	if (usb_get_dr_mode(node) != USB_DR_MODE_PERIPHERAL &&
	    usb_get_dr_mode(node) != USB_DR_MODE_OTG) {
		dev_dbg(dev, "Invalid mode\n");
		return -ENODEV;
	}

	platdata->regs_otg = dev_read_addr(dev);

	platdata->rx_fifo_sz = dev_read_u32_default(dev, "g-rx-fifo-size", 0);
	platdata->np_tx_fifo_sz = dev_read_u32_default(dev,
						       "g-np-tx-fifo-size", 0);

	platdata->tx_fifo_sz_nb =
		dev_read_size(dev, "g-tx-fifo-size") / sizeof(u32);
	if (platdata->tx_fifo_sz_nb > DWC2_MAX_HW_ENDPOINTS)
		platdata->tx_fifo_sz_nb = DWC2_MAX_HW_ENDPOINTS;
	if (platdata->tx_fifo_sz_nb) {
		ret = dev_read_u32_array(dev, "g-tx-fifo-size",
					 platdata->tx_fifo_sz_array,
					 platdata->tx_fifo_sz_nb);
		if (ret)
			return ret;
	}

	platdata->force_b_session_valid =
		dev_read_bool(dev, "u-boot,force-b-session-valid");

	/* force platdata according compatible */
	drvdata = dev_get_driver_data(dev);
	if (drvdata) {
		set_params = (void *)drvdata;
		set_params(platdata);
	}

	return 0;
}

static void dwc2_set_stm32mp1_hsotg_params(struct dwc2_plat_otg_data *p)
{
	p->activate_stm_id_vb_detection = true;
	p->usb_gusbcfg =
		0 << 15		/* PHY Low Power Clock sel*/
		| 0x9 << 10	/* USB Turnaround time (0x9 for HS phy) */
		| 0 << 9	/* [0:HNP disable,1:HNP enable]*/
		| 0 << 8	/* [0:SRP disable 1:SRP enable]*/
		| 0 << 6	/* 0: high speed utmi+, 1: full speed serial*/
		| 0x7 << 0;	/* FS timeout calibration**/

	if (p->force_b_session_valid)
		p->usb_gusbcfg |= 1 << 30; /* FDMOD: Force device mode */
}

static int dwc2_udc_otg_reset_init(struct udevice *dev,
				   struct reset_ctl_bulk *resets)
{
	int ret;

	ret = reset_get_bulk(dev, resets);
	if (ret == -ENOTSUPP)
		return 0;

	if (ret)
		return ret;

	ret = reset_assert_bulk(resets);

	if (!ret) {
		udelay(2);
		ret = reset_deassert_bulk(resets);
	}
	if (ret) {
		reset_release_bulk(resets);
		return ret;
	}

	return 0;
}

static int dwc2_udc_otg_clk_init(struct udevice *dev,
				 struct clk_bulk *clks)
{
	int ret;

	ret = clk_get_bulk(dev, clks);
	if (ret == -ENOSYS)
		return 0;

	if (ret)
		return ret;

	ret = clk_enable_bulk(clks);
	if (ret) {
		clk_release_bulk(clks);
		return ret;
	}

	return 0;
}

static int dwc2_udc_otg_probe(struct udevice *dev)
{
	struct dwc2_plat_otg_data *platdata = dev_get_platdata(dev);
	struct dwc2_priv_data *priv = dev_get_priv(dev);
	struct dwc2_usbotg_reg *usbotg_reg =
		(struct dwc2_usbotg_reg *)platdata->regs_otg;
	int ret;

	ret = dwc2_udc_otg_clk_init(dev, &priv->clks);
	if (ret)
		return ret;

	ret = dwc2_udc_otg_reset_init(dev, &priv->resets);
	if (ret)
		return ret;

	ret = dwc2_phy_setup(dev, &priv->phys, &priv->num_phys);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(DM_REGULATOR) &&
	    platdata->activate_stm_id_vb_detection &&
	    !platdata->force_b_session_valid) {
		ret = device_get_supply_regulator(dev, "usb33d-supply",
						  &priv->usb33d_supply);
		if (ret) {
			dev_err(dev, "can't get voltage level detector supply\n");
			return ret;
		}
		ret = regulator_set_enable(priv->usb33d_supply, true);
		if (ret) {
			dev_err(dev, "can't enable voltage level detector supply\n");
			return ret;
		}
		/* Enable vbus sensing */
		setbits_le32(&usbotg_reg->ggpio,
			     GGPIO_STM32_OTG_GCCFG_VBDEN |
			     GGPIO_STM32_OTG_GCCFG_IDEN);
	}

	if (platdata->force_b_session_valid)
		/* Override B session bits : value and enable */
		setbits_le32(&usbotg_reg->gotgctl,
			     A_VALOEN | A_VALOVAL | B_VALOEN | B_VALOVAL);

	ret = dwc2_udc_probe(platdata);
	if (ret)
		return ret;

	the_controller->driver = 0;

	ret = usb_add_gadget_udc((struct device *)dev, &the_controller->gadget);

	return ret;
}

static int dwc2_udc_otg_remove(struct udevice *dev)
{
	struct dwc2_priv_data *priv = dev_get_priv(dev);

	usb_del_gadget_udc(&the_controller->gadget);

	reset_release_bulk(&priv->resets);

	clk_release_bulk(&priv->clks);

	dwc2_phy_shutdown(dev, priv->phys, priv->num_phys);

	return dm_scan_fdt_dev(dev);
}

static const struct udevice_id dwc2_udc_otg_ids[] = {
	{ .compatible = "snps,dwc2" },
	{ .compatible = "st,stm32mp1-hsotg",
	  .data = (ulong)dwc2_set_stm32mp1_hsotg_params },
	{},
};

U_BOOT_DRIVER(dwc2_udc_otg) = {
	.name	= "dwc2-udc-otg",
	.id	= UCLASS_USB_GADGET_GENERIC,
	.of_match = dwc2_udc_otg_ids,
	.ofdata_to_platdata = dwc2_udc_otg_ofdata_to_platdata,
	.probe = dwc2_udc_otg_probe,
	.remove = dwc2_udc_otg_remove,
	.platdata_auto_alloc_size = sizeof(struct dwc2_plat_otg_data),
	.priv_auto_alloc_size = sizeof(struct dwc2_priv_data),
};

int dwc2_udc_B_session_valid(struct udevice *dev)
{
	struct dwc2_plat_otg_data *platdata = dev_get_platdata(dev);
	struct dwc2_usbotg_reg *usbotg_reg =
		(struct dwc2_usbotg_reg *)platdata->regs_otg;

	return readl(&usbotg_reg->gotgctl) & B_SESSION_VALID;
}
#endif /* CONFIG_IS_ENABLED(DM_USB_GADGET) */
