// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <linux/list.h>
#include <linux/bitfield.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <malloc.h>
#include <spi.h>
#include <dm.h>
#include <g_dnl.h>

#define MAX3420_MAX_EPS		4
#define EP_MAX_PACKET		64  /* Same for all Endpoints */
#define EPNAME_SIZE		16  /* Buffer size for endpoint name */

#define MAX3420_SPI_DIR_RD	0	/* read register from MAX3420 */
#define MAX3420_SPI_DIR_WR	1	/* write register to MAX3420 */

/* SPI commands: */
#define MAX3420_SPI_ACK_MASK BIT(0)
#define MAX3420_SPI_DIR_MASK BIT(1)
#define MAX3420_SPI_REG_MASK GENMASK(7, 3)

#define MAX3420_REG_EP0FIFO	0
#define MAX3420_REG_EP1FIFO	1
#define MAX3420_REG_EP2FIFO	2
#define MAX3420_REG_EP3FIFO	3
#define MAX3420_REG_SUDFIFO	4
#define MAX3420_REG_EP0BC	5
#define MAX3420_REG_EP1BC	6
#define MAX3420_REG_EP2BC	7
#define MAX3420_REG_EP3BC	8

#define MAX3420_REG_EPSTALLS	9
	#define bACKSTAT	BIT(6)
	#define bSTLSTAT	BIT(5)
	#define bSTLEP3IN	BIT(4)
	#define bSTLEP2IN	BIT(3)
	#define bSTLEP1OUT	BIT(2)
	#define bSTLEP0OUT	BIT(1)
	#define bSTLEP0IN	BIT(0)

#define MAX3420_REG_CLRTOGS	10
	#define bEP3DISAB	BIT(7)
	#define bEP2DISAB	BIT(6)
	#define bEP1DISAB	BIT(5)
	#define bCTGEP3IN	BIT(4)
	#define bCTGEP2IN	BIT(3)
	#define bCTGEP1OUT	BIT(2)

#define MAX3420_REG_EPIRQ	11
#define MAX3420_REG_EPIEN	12
	#define bSUDAVIRQ	BIT(5)
	#define bIN3BAVIRQ	BIT(4)
	#define bIN2BAVIRQ	BIT(3)
	#define bOUT1DAVIRQ	BIT(2)
	#define bOUT0DAVIRQ	BIT(1)
	#define bIN0BAVIRQ	BIT(0)

#define MAX3420_REG_USBIRQ	13
#define MAX3420_REG_USBIEN	14
	#define bOSCOKIRQ	BIT(0)
	#define bRWUDNIRQ	BIT(1)
	#define bBUSACTIRQ	BIT(2)
	#define bURESIRQ	BIT(3)
	#define bSUSPIRQ	BIT(4)
	#define bNOVBUSIRQ	BIT(5)
	#define bVBUSIRQ	BIT(6)
	#define bURESDNIRQ	BIT(7)

#define MAX3420_REG_USBCTL	15
	#define bHOSCSTEN	BIT(7)
	#define bVBGATE		BIT(6)
	#define bCHIPRES	BIT(5)
	#define bPWRDOWN	BIT(4)
	#define bCONNECT	BIT(3)
	#define bSIGRWU		BIT(2)

#define MAX3420_REG_CPUCTL	16
	#define bIE		BIT(0)

#define MAX3420_REG_PINCTL	17
	#define bEP3INAK	BIT(7)
	#define bEP2INAK	BIT(6)
	#define bEP0INAK	BIT(5)
	#define bFDUPSPI	BIT(4)
	#define bINTLEVEL	BIT(3)
	#define bPOSINT		BIT(2)
	#define bGPXB		BIT(1)
	#define bGPXA		BIT(0)

#define MAX3420_REG_REVISION	18

#define MAX3420_REG_FNADDR	19
	#define FNADDR_MASK	0x7f

#define MAX3420_REG_IOPINS	20
#define MAX3420_REG_IOPINS2	21
#define MAX3420_REG_GPINIRQ	22
#define MAX3420_REG_GPINIEN	23
#define MAX3420_REG_GPINPOL	24
#define MAX3420_REG_HIRQ	25
#define MAX3420_REG_HIEN	26
#define MAX3420_REG_MODE	27
#define MAX3420_REG_PERADDR	28
#define MAX3420_REG_HCTL	29
#define MAX3420_REG_HXFR	30
#define MAX3420_REG_HRSL	31

struct max3420_req {
	struct usb_request usb_req;
	struct list_head queue;
	struct max3420_ep *ep;
};

struct max3420_ep {
	struct max3420_udc *udc;
	struct list_head queue;
	char name[EPNAME_SIZE];
	unsigned int maxpacket;
	struct usb_ep ep_usb;
	int halted;
	int id;
};

struct max3420_udc {
	struct max3420_ep ep[MAX3420_MAX_EPS];
	struct usb_gadget_driver *driver;
	bool softconnect;
	struct usb_ctrlrequest setup;
	struct max3420_req ep0req;
	struct usb_gadget gadget;
	struct spi_slave *slave;
	struct udevice *dev;
	u8 ep0buf[64];
	int remote_wkp;
	bool suspended;
};

#define to_max3420_req(r)	container_of((r), struct max3420_req, usb_req)
#define to_max3420_ep(e)	container_of((e), struct max3420_ep, ep_usb)
#define to_udc(g)		container_of((g), struct max3420_udc, gadget)

static void spi_ack_ctrl(struct max3420_udc *udc)
{
	struct spi_slave *slave = udc->slave;
	u8 txdata[1];

	txdata[0] = FIELD_PREP(MAX3420_SPI_ACK_MASK, 1);
	spi_xfer(slave, sizeof(txdata), txdata, NULL, SPI_XFER_ONCE);
}

static u8 spi_rd8_ack(struct max3420_udc *udc, u8 reg, int ackstat)
{
	struct spi_slave *slave = udc->slave;
	u8 txdata[2], rxdata[2];

	txdata[0] = FIELD_PREP(MAX3420_SPI_REG_MASK, reg) |
			FIELD_PREP(MAX3420_SPI_DIR_MASK, MAX3420_SPI_DIR_RD) |
			FIELD_PREP(MAX3420_SPI_ACK_MASK, ackstat ? 1 : 0);

	rxdata[0] = 0;
	rxdata[1] = 0;
	spi_xfer(slave, sizeof(txdata), txdata, rxdata, SPI_XFER_ONCE);

	return rxdata[1];
}

static u8 spi_rd8(struct max3420_udc *udc, u8 reg)
{
	return spi_rd8_ack(udc, reg, 0);
}

static void spi_wr8_ack(struct max3420_udc *udc, u8 reg, u8 val, int ackstat)
{
	struct spi_slave *slave = udc->slave;
	u8 txdata[2];

	txdata[0] = FIELD_PREP(MAX3420_SPI_REG_MASK, reg) |
			FIELD_PREP(MAX3420_SPI_DIR_MASK, MAX3420_SPI_DIR_WR) |
			FIELD_PREP(MAX3420_SPI_ACK_MASK, ackstat ? 1 : 0);
	txdata[1] = val;

	spi_xfer(slave, sizeof(txdata), txdata, NULL, SPI_XFER_ONCE);
}

static void spi_wr8(struct max3420_udc *udc, u8 reg, u8 val)
{
	spi_wr8_ack(udc, reg, val, 0);
}

static void spi_rd_buf(struct max3420_udc *udc, u8 reg, void *buf, u8 len)
{
	struct spi_slave *slave = udc->slave;
	u8 txdata[1];

	txdata[0] = FIELD_PREP(MAX3420_SPI_REG_MASK, reg) |
			FIELD_PREP(MAX3420_SPI_DIR_MASK, MAX3420_SPI_DIR_RD);

	spi_xfer(slave, sizeof(txdata), txdata, NULL, SPI_XFER_BEGIN);
	spi_xfer(slave, len * 8, NULL, buf, SPI_XFER_END);
}

static void spi_wr_buf(struct max3420_udc *udc, u8 reg, void *buf, u8 len)
{
	struct spi_slave *slave = udc->slave;
	u8 txdata[1];

	txdata[0] = FIELD_PREP(MAX3420_SPI_REG_MASK, reg) |
			FIELD_PREP(MAX3420_SPI_DIR_MASK, MAX3420_SPI_DIR_WR);

	spi_xfer(slave, sizeof(txdata), txdata, NULL, SPI_XFER_BEGIN);
	spi_xfer(slave, len * 8, buf, NULL, SPI_XFER_END);
}

/* 0 if not-connected */
int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}

static void spi_max3420_enable(struct max3420_ep *ep, int enable)
{
	struct max3420_udc *udc = ep->udc;
	u8 epdis, epien;

	if (ep->id == 0)
		return;

	epien = spi_rd8(udc, MAX3420_REG_EPIEN);
	epdis = spi_rd8(udc, MAX3420_REG_CLRTOGS);

	if (enable) {
		epdis &= ~BIT(ep->id + 4);
		epien |= BIT(ep->id + 1);
	} else {
		epdis |= BIT(ep->id + 4);
		epien &= ~BIT(ep->id + 1);
	}

	spi_wr8(udc, MAX3420_REG_CLRTOGS, epdis);
	spi_wr8(udc, MAX3420_REG_EPIEN, epien);
}

static int
max3420_ep_enable(struct usb_ep *_ep,
		  const struct usb_endpoint_descriptor *desc)
{
	struct max3420_ep *ep = to_max3420_ep(_ep);

	_ep->desc = desc;
	_ep->maxpacket = usb_endpoint_maxp(desc) & 0x7ff;

	spi_max3420_enable(ep, 1);

	return 0;
}

static void max3420_req_done(struct max3420_req *req, int status)
{
	struct max3420_ep *ep = req->ep;

	if (req->usb_req.status == -EINPROGRESS)
		req->usb_req.status = status;
	else
		status = req->usb_req.status;

	if (status && status != -ESHUTDOWN)
		dev_err(ep->udc->dev, "%s done %p, status %d\n",
			ep->ep_usb.name, req, status);

	if (req->usb_req.complete)
		req->usb_req.complete(&ep->ep_usb, &req->usb_req);
}

static void max3420_ep_nuke(struct max3420_ep *ep, int status)
{
	struct max3420_req *req, *r;

	list_for_each_entry_safe(req, r, &ep->queue, queue) {
		list_del_init(&req->queue);
		max3420_req_done(req, status);
	}
}

static int max3420_ep_disable(struct usb_ep *_ep)
{
	struct max3420_ep *ep = to_max3420_ep(_ep);

	_ep->desc = NULL;
	max3420_ep_nuke(ep, -ESHUTDOWN);
	spi_max3420_enable(ep, 0);

	return 0;
}

static struct usb_request *
max3420_ep_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct max3420_ep *ep = to_max3420_ep(_ep);
	struct max3420_req *req = kzalloc(sizeof(*req), gfp_flags);

	if (!req)
		return NULL;

	req->ep = ep;
	INIT_LIST_HEAD(&req->queue);

	return &req->usb_req;
}

static void
max3420_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	kfree(to_max3420_req(_req));
}

static int
max3420_ep_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct max3420_req *req = to_max3420_req(_req);
	struct max3420_ep *ep = to_max3420_ep(_ep);

	_req->status = -EINPROGRESS;
	_req->actual = 0;
	list_add_tail(&req->queue, &ep->queue);

	return 0;
}

static int max3420_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct max3420_req *req = to_max3420_req(_req);

	list_del_init(&req->queue);
	max3420_req_done(req, -ECONNRESET);

	return 0;
}

static int max3420_ep_set_halt(struct usb_ep *_ep, int halt)
{
	struct max3420_ep *ep = to_max3420_ep(_ep);
	struct max3420_udc *udc = ep->udc;
	u8 epstalls;

	if (ep->id == 0) /* can't stall EP0 */
		return 0;

	epstalls = spi_rd8(udc, MAX3420_REG_EPSTALLS);
	if (halt) {
		ep->halted = 1;
		epstalls |= BIT(ep->id + 1);
	} else {
		u8 clrtogs;

		ep->halted = 0;
		epstalls &= ~BIT(ep->id + 1);
		clrtogs = spi_rd8(udc, MAX3420_REG_CLRTOGS);
		clrtogs |= BIT(ep->id + 1);
		spi_wr8(udc, MAX3420_REG_CLRTOGS, clrtogs);
	}
	spi_wr8(udc, MAX3420_REG_EPSTALLS, epstalls | bACKSTAT);

	return 0;
}

static const struct usb_ep_ops max3420_ep_ops = {
	.enable		= max3420_ep_enable,
	.disable	= max3420_ep_disable,
	.alloc_request	= max3420_ep_alloc_request,
	.free_request	= max3420_ep_free_request,
	.queue		= max3420_ep_queue,
	.dequeue	= max3420_ep_dequeue,
	.set_halt	= max3420_ep_set_halt,
};

static void __max3420_stop(struct max3420_udc *udc)
{
	u8 val;

	/* Disable IRQ to CPU */
	spi_wr8(udc, MAX3420_REG_CPUCTL, 0);

	val = spi_rd8(udc, MAX3420_REG_USBCTL);
	val |= bPWRDOWN;
	val |= bHOSCSTEN;
	spi_wr8(udc, MAX3420_REG_USBCTL, val);
}

static void __max3420_start(struct max3420_udc *udc)
{
	u8 val;

	/* configure SPI */
	spi_wr8(udc, MAX3420_REG_PINCTL, bFDUPSPI);

	/* Chip Reset */
	spi_wr8(udc, MAX3420_REG_USBCTL, bCHIPRES);
	mdelay(5);
	spi_wr8(udc, MAX3420_REG_USBCTL, 0);

	/* Poll for OSC to stabilize */
	while (1) {
		val = spi_rd8(udc, MAX3420_REG_USBIRQ);
		if (val & bOSCOKIRQ)
			break;
		cond_resched();
	}

	/* Enable PULL-UP only when Vbus detected */
	val = spi_rd8(udc, MAX3420_REG_USBCTL);
	val |= bVBGATE | bCONNECT;
	spi_wr8(udc, MAX3420_REG_USBCTL, val);

	val = bURESDNIRQ | bURESIRQ;
	spi_wr8(udc, MAX3420_REG_USBIEN, val);

	/* Enable only EP0 interrupts */
	val = bIN0BAVIRQ | bOUT0DAVIRQ | bSUDAVIRQ;
	spi_wr8(udc, MAX3420_REG_EPIEN, val);

	/* Enable IRQ to CPU */
	spi_wr8(udc, MAX3420_REG_CPUCTL, bIE);
}

static int max3420_udc_start(struct usb_gadget *gadget,
			     struct usb_gadget_driver *driver)
{
	struct max3420_udc *udc = to_udc(gadget);

	udc->driver = driver;
	udc->remote_wkp = 0;
	udc->softconnect = true;

	__max3420_start(udc);

	return 0;
}

static int max3420_udc_stop(struct usb_gadget *gadget)
{
	struct max3420_udc *udc = to_udc(gadget);

	udc->driver = NULL;
	udc->softconnect = false;

	__max3420_stop(udc);

	return 0;
}

static int max3420_wakeup(struct usb_gadget *gadget)
{
	struct max3420_udc *udc = to_udc(gadget);
	u8 usbctl;

	/* Only if wakeup allowed by host */
	if (!udc->remote_wkp || !udc->suspended)
		return 0;

	/* Set Remote-Wakeup Signal*/
	usbctl = spi_rd8(udc, MAX3420_REG_USBCTL);
	usbctl |= bSIGRWU;
	spi_wr8(udc, MAX3420_REG_USBCTL, usbctl);

	mdelay(5);

	/* Clear Remote-WkUp Signal*/
	usbctl = spi_rd8(udc, MAX3420_REG_USBCTL);
	usbctl &= ~bSIGRWU;
	spi_wr8(udc, MAX3420_REG_USBCTL, usbctl);

	udc->suspended = false;

	return 0;
}

static const struct usb_gadget_ops max3420_udc_ops = {
	.udc_start	= max3420_udc_start,
	.udc_stop	= max3420_udc_stop,
	.wakeup		= max3420_wakeup,
};

static struct usb_endpoint_descriptor ep0_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
	.wMaxPacketSize = cpu_to_le16(EP_MAX_PACKET),
};

static void max3420_getstatus(struct max3420_udc *udc)
{
	struct max3420_ep *ep;
	u16 status = 0;

	switch (udc->setup.bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		/* Get device status */
		status = 0 << USB_DEVICE_SELF_POWERED;
		status |= (udc->remote_wkp << USB_DEVICE_REMOTE_WAKEUP);
		break;
	case USB_RECIP_INTERFACE:
		if (udc->driver->setup(&udc->gadget, &udc->setup) < 0)
			goto stall;
		break;
	case USB_RECIP_ENDPOINT:
		ep = &udc->ep[udc->setup.wIndex & USB_ENDPOINT_NUMBER_MASK];
		if (ep->halted)
			status = 1 << USB_ENDPOINT_HALT;
		break;
	default:
		goto stall;
	}

	status = cpu_to_le16(status);
	spi_wr_buf(udc, MAX3420_REG_EP0FIFO, &status, 2);
	spi_wr8_ack(udc, MAX3420_REG_EP0BC, 2, 1);
	return;
stall:
	dev_err(udc->dev, "Can't respond to getstatus request\n");
	spi_wr8(udc, MAX3420_REG_EPSTALLS, bSTLEP0IN | bSTLEP0OUT | bSTLSTAT);
}

static void max3420_set_clear_feature(struct max3420_udc *udc)
{
	int set = udc->setup.bRequest == USB_REQ_SET_FEATURE;
	struct max3420_ep *ep;
	int id;

	switch (udc->setup.bRequestType) {
	case USB_RECIP_DEVICE:
		if (udc->setup.wValue != USB_DEVICE_REMOTE_WAKEUP)
			break;

		if (udc->setup.bRequest == USB_REQ_SET_FEATURE)
			udc->remote_wkp = 1;
		else
			udc->remote_wkp = 0;

		return spi_ack_ctrl(udc);

	case USB_RECIP_ENDPOINT:
		if (udc->setup.wValue != USB_ENDPOINT_HALT)
			break;

		id = udc->setup.wIndex & USB_ENDPOINT_NUMBER_MASK;
		ep = &udc->ep[id];

		max3420_ep_set_halt(&ep->ep_usb, set);
		return;
	default:
		break;
	}

	dev_err(udc->dev, "Can't respond to SET/CLEAR FEATURE\n");
	spi_wr8(udc, MAX3420_REG_EPSTALLS, bSTLEP0IN | bSTLEP0OUT | bSTLSTAT);
}

static void max3420_handle_setup(struct max3420_udc *udc)
{
	struct usb_ctrlrequest setup;
	u8 addr;

	spi_rd_buf(udc, MAX3420_REG_SUDFIFO, (void *)&setup, 8);

	udc->setup = setup;
	udc->setup.wValue = cpu_to_le16(setup.wValue);
	udc->setup.wIndex = cpu_to_le16(setup.wIndex);
	udc->setup.wLength = cpu_to_le16(setup.wLength);

	switch (udc->setup.bRequest) {
	case USB_REQ_GET_STATUS:
		/* Data+Status phase form udc */
		if ((udc->setup.bRequestType &
				(USB_DIR_IN | USB_TYPE_MASK)) !=
				(USB_DIR_IN | USB_TYPE_STANDARD)) {
			break;
		}
		return max3420_getstatus(udc);
	case USB_REQ_SET_ADDRESS:
		/* Status phase from udc */
		if (udc->setup.bRequestType != (USB_DIR_OUT |
				USB_TYPE_STANDARD | USB_RECIP_DEVICE))
			break;
		addr = spi_rd8_ack(udc, MAX3420_REG_FNADDR, 1);
		dev_dbg(udc->dev, "Assigned Address=%d/%d\n",
			udc->setup.wValue, addr);
		return;
	case USB_REQ_CLEAR_FEATURE:
	case USB_REQ_SET_FEATURE:
		/* Requests with no data phase, status phase from udc */
		if ((udc->setup.bRequestType & USB_TYPE_MASK)
				!= USB_TYPE_STANDARD)
			break;
		return max3420_set_clear_feature(udc);
	default:
		break;
	}

	if (udc->driver->setup(&udc->gadget, &setup) < 0) {
		/* Stall EP0 */
		spi_wr8(udc, MAX3420_REG_EPSTALLS,
			bSTLEP0IN | bSTLEP0OUT | bSTLSTAT);
	}
}

static int do_data(struct max3420_udc *udc, int ep_id, int in)
{
	struct max3420_ep *ep = &udc->ep[ep_id];
	struct max3420_req *req;
	int done, length, psz;
	void *buf;

	if (list_empty(&ep->queue))
		return 0;

	req = list_first_entry(&ep->queue, struct max3420_req, queue);
	buf = req->usb_req.buf + req->usb_req.actual;

	psz = ep->ep_usb.maxpacket;
	length = req->usb_req.length - req->usb_req.actual;
	length = min(length, psz);

	if (length == 0) {
		done = 1;
		goto xfer_done;
	}

	done = 0;
	if (in) {
		spi_wr_buf(udc, MAX3420_REG_EP0FIFO + ep_id, buf, length);
		spi_wr8(udc, MAX3420_REG_EP0BC + ep_id, length);
		if (length < psz)
			done = 1;
	} else {
		psz = spi_rd8(udc, MAX3420_REG_EP0BC + ep_id);
		length = min(length, psz);
		spi_rd_buf(udc, MAX3420_REG_EP0FIFO + ep_id, buf, length);
		if (length < ep->ep_usb.maxpacket)
			done = 1;
	}

	req->usb_req.actual += length;

	if (req->usb_req.actual == req->usb_req.length)
		done = 1;

xfer_done:
	if (done) {
		list_del_init(&req->queue);

		if (ep_id == 0)
			spi_ack_ctrl(udc);

		max3420_req_done(req, 0);
	}

	return 1;
}

static int max3420_handle_irqs(struct max3420_udc *udc)
{
	u8 epien, epirq, usbirq, usbien, reg[4];
	int ret = 0;

	spi_rd_buf(udc, MAX3420_REG_EPIRQ, reg, 4);
	epirq = reg[0];
	epien = reg[1];
	usbirq = reg[2];
	usbien = reg[3];

	usbirq &= usbien;
	epirq &= epien;

	if (epirq & bSUDAVIRQ) {
		spi_wr8(udc, MAX3420_REG_EPIRQ, bSUDAVIRQ);
		max3420_handle_setup(udc);
		return 1;
	}

	if (usbirq & bVBUSIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bVBUSIRQ);
		dev_dbg(udc->dev, "Cable plugged in\n");
		g_dnl_clear_detach();
		return 1;
	}

	if (usbirq & bNOVBUSIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bNOVBUSIRQ);
		dev_dbg(udc->dev, "Cable pulled out\n");
		g_dnl_trigger_detach();
		return 1;
	}

	if (usbirq & bURESIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bURESIRQ);
		return 1;
	}

	if (usbirq & bURESDNIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bURESDNIRQ);
		spi_wr8(udc, MAX3420_REG_USBIEN, bURESDNIRQ | bURESIRQ);
		spi_wr8(udc, MAX3420_REG_EPIEN, bSUDAVIRQ
			| bIN0BAVIRQ | bOUT0DAVIRQ);
		return 1;
	}

	if (usbirq & bSUSPIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bSUSPIRQ);
		dev_dbg(udc->dev, "USB Suspend - Enter\n");
		udc->suspended = true;
		return 1;
	}

	if (usbirq & bBUSACTIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bBUSACTIRQ);
		dev_dbg(udc->dev, "USB Suspend - Exit\n");
		udc->suspended = false;
		return 1;
	}

	if (usbirq & bRWUDNIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bRWUDNIRQ);
		dev_dbg(udc->dev, "Asked Host to wakeup\n");
		return 1;
	}

	if (usbirq & bOSCOKIRQ) {
		spi_wr8(udc, MAX3420_REG_USBIRQ, bOSCOKIRQ);
		dev_dbg(udc->dev, "Osc stabilized, start work\n");
		return 1;
	}

	if (epirq & bOUT0DAVIRQ && do_data(udc, 0, 0)) {
		spi_wr8_ack(udc, MAX3420_REG_EPIRQ, bOUT0DAVIRQ, 1);
		ret = 1;
	}

	if (epirq & bIN0BAVIRQ && do_data(udc, 0, 1))
		ret = 1;

	if (epirq & bOUT1DAVIRQ && do_data(udc, 1, 0)) {
		spi_wr8_ack(udc, MAX3420_REG_EPIRQ, bOUT1DAVIRQ, 1);
		ret = 1;
	}

	if (epirq & bIN2BAVIRQ && do_data(udc, 2, 1))
		ret = 1;

	if (epirq & bIN3BAVIRQ && do_data(udc, 3, 1))
		ret = 1;

	return ret;
}

static int max3420_irq(struct max3420_udc *udc)
{
	do_data(udc, 0, 1); /* get done with the EP0 ZLP */

	return max3420_handle_irqs(udc);
}

static void max3420_setup_eps(struct max3420_udc *udc)
{
	int i;

	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->ep[0].ep_usb.ep_list);

	for (i = 0; i < MAX3420_MAX_EPS; i++) {
		struct max3420_ep *ep = &udc->ep[i];

		INIT_LIST_HEAD(&ep->queue);

		ep->id = i;
		ep->udc = udc;
		ep->ep_usb.ops = &max3420_ep_ops;
		ep->ep_usb.name = ep->name;
		ep->ep_usb.maxpacket = EP_MAX_PACKET;

		if (i == 0) {
			ep->ep_usb.desc = &ep0_desc;
			snprintf(ep->name, EPNAME_SIZE, "ep0");
			continue;
		}

		list_add_tail(&ep->ep_usb.ep_list, &udc->gadget.ep_list);

		if (i == 1)
			snprintf(ep->name, EPNAME_SIZE, "ep1out-bulk");
		else
			snprintf(ep->name, EPNAME_SIZE, "ep%din-bulk", i);
	};
}

static void max3420_setup_spi(struct max3420_udc *udc)
{
	u8 reg[8];

	spi_claim_bus(udc->slave);
	spi_rd_buf(udc, MAX3420_REG_EPIRQ, reg, 8);
	/* configure SPI */
	spi_wr8(udc, MAX3420_REG_PINCTL, bFDUPSPI);
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct max3420_udc *udc = dev_get_priv(dev);

	return max3420_irq(udc);
}

static int max3420_udc_probe(struct udevice *dev)
{
	struct max3420_udc *udc = dev_get_priv(dev);
	struct dm_spi_slave_plat *slave_pdata;
	struct udevice *bus = dev->parent;
	int busnum = dev_seq(bus);
	unsigned int cs;
	uint speed, mode;
	struct udevice *spid;

	slave_pdata = dev_get_parent_plat(dev);
	cs = slave_pdata->cs;
	speed = slave_pdata->max_hz;
	mode = slave_pdata->mode;
	_spi_get_bus_and_cs(busnum, cs, speed, mode, false, "spi_generic_drv",
			    NULL, &spid, &udc->slave);

	udc->dev = dev;
	udc->gadget.ep0 = &udc->ep[0].ep_usb;
	udc->gadget.max_speed = USB_SPEED_FULL;
	udc->gadget.speed = USB_SPEED_FULL;
	udc->gadget.is_dualspeed = 0;
	udc->gadget.ops = &max3420_udc_ops;
	udc->gadget.name = "max3420-udc";

	max3420_setup_eps(udc);
	max3420_setup_spi(udc);

	usb_add_gadget_udc((struct device *)dev, &udc->gadget);

	return 0;
}

static int max3420_udc_remove(struct udevice *dev)
{
	struct max3420_udc *udc = dev_get_priv(dev);

	usb_del_gadget_udc(&udc->gadget);

	spi_release_bus(udc->slave);

	return 0;
}

static const struct udevice_id max3420_ids[] = {
	{ .compatible = "maxim,max3421-udc" },
	{ }
};

U_BOOT_DRIVER(max3420_generic_udc) = {
	.name = "max3420-udc",
	.id = UCLASS_USB_GADGET_GENERIC,
	.of_match = max3420_ids,
	.probe = max3420_udc_probe,
	.remove = max3420_udc_remove,
	.priv_auto	= sizeof(struct max3420_udc),
};
