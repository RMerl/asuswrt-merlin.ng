// SPDX-License-Identifier: GPL-2.0+
/*
 * PXA27x USB device driver for u-boot.
 *
 * Copyright (C) 2007 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2007 Eurotech S.p.A.  <info@eurotech.it>
 * Copyright (C) 2008 Vivek Kutal      <vivek.kutal@azingo.com>
 */


#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <usbdevice.h>
#include <usb/pxa27x_udc.h>
#include <usb/udc.h>

#include "ep0.h"

/* number of endpoints on this UDC */
#define UDC_MAX_ENDPOINTS	24

static struct urb *ep0_urb;
static struct usb_device_instance *udc_device;
static int ep0state = EP0_IDLE;

#ifdef USBDDBG
static void udc_dump_buffer(char *name, u8 *buf, int len)
{
	usbdbg("%s - buf %p, len %d", name, buf, len);
	print_buffer(0, buf, 1, len, 0);
}
#else
#define udc_dump_buffer(name, buf, len)		/* void */
#endif

static inline void udc_ack_int_UDCCR(int mask)
{
	writel(readl(USIR1) | mask, USIR1);
}

/*
 * If the endpoint has an active tx_urb, then the next packet of data from the
 * URB is written to the tx FIFO.
 * The total amount of data in the urb is given by urb->actual_length.
 * The maximum amount of data that can be sent in any one packet is given by
 * endpoint->tx_packetSize.
 * The number of data bytes from this URB that have already been transmitted
 * is given by endpoint->sent.
 * endpoint->last is updated by this routine with the number of data bytes
 * transmitted in this packet.
 */
static int udc_write_urb(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb = endpoint->tx_urb;
	int ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;
	u32 *data32 = (u32 *) urb->buffer;
	u8  *data8 = (u8 *) urb->buffer;
	unsigned int i, n, w, b, is_short;
	int timeout = 2000;	/* 2ms */

	if (!urb || !urb->actual_length)
		return -1;

	n = min_t(unsigned int, urb->actual_length - endpoint->sent,
		  endpoint->tx_packetSize);
	if (n <= 0)
		return -1;

	usbdbg("write urb on ep %d", ep_num);
#if defined(USBDDBG) && defined(USBDPARANOIA)
	usbdbg("urb: buf %p, buf_len %d, actual_len %d",
		urb->buffer, urb->buffer_length, urb->actual_length);
	usbdbg("endpoint: sent %d, tx_packetSize %d, last %d",
		endpoint->sent, endpoint->tx_packetSize, endpoint->last);
#endif

	is_short = n != endpoint->tx_packetSize;
	w = n / 4;
	b = n % 4;
	usbdbg("n %d%s w %d b %d", n, is_short ? "-s" : "", w, b);
	udc_dump_buffer("urb write", data8 + endpoint->sent, n);

	/* Prepare for data send */
	if (ep_num)
		writel(UDCCSR_PC ,UDCCSN(ep_num));

	for (i = 0; i < w; i++)
		  writel(data32[endpoint->sent / 4 + i], UDCDN(ep_num));

	for (i = 0; i < b; i++)
		  writeb(data8[endpoint->sent + w * 4 + i], UDCDN(ep_num));

	/* Set "Packet Complete" if less data then tx_packetSize */
	if (is_short)
		writel(ep_num ? UDCCSR_SP : UDCCSR0_IPR, UDCCSN(ep_num));

	/* Wait for data sent */
	if (ep_num) {
		while (!(readl(UDCCSN(ep_num)) & UDCCSR_PC)) {
			if (timeout-- == 0)
				return -1;
			else
				udelay(1);
		}
	}

	endpoint->last = n;

	if (ep_num) {
		usbd_tx_complete(endpoint);
	} else {
		endpoint->sent += n;
		endpoint->last -= n;
	}

	if (endpoint->sent >= urb->actual_length) {
		urb->actual_length = 0;
		endpoint->sent = 0;
		endpoint->last = 0;
	}

	if ((endpoint->sent >= urb->actual_length) && (!ep_num)) {
		usbdbg("ep0 IN stage done");
		if (is_short)
			ep0state = EP0_IDLE;
		else
			ep0state = EP0_XFER_COMPLETE;
	}

	return 0;
}

static int udc_read_urb(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb = endpoint->rcv_urb;
	int ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;
	u32 *data32 = (u32 *) urb->buffer;
	unsigned int i, n;

	usbdbg("read urb on ep %d", ep_num);
#if defined(USBDDBG) && defined(USBDPARANOIA)
	usbdbg("urb: buf %p, buf_len %d, actual_len %d",
		urb->buffer, urb->buffer_length, urb->actual_length);
	usbdbg("endpoint: rcv_packetSize %d",
		endpoint->rcv_packetSize);
#endif

	if (readl(UDCCSN(ep_num)) & UDCCSR_BNE)
		n = readl(UDCBCN(ep_num)) & 0x3ff;
	else /* zlp */
		n = 0;

	usbdbg("n %d%s", n, n != endpoint->rcv_packetSize ? "-s" : "");
	for (i = 0; i < n; i += 4)
		data32[urb->actual_length / 4 + i / 4] = readl(UDCDN(ep_num));

	udc_dump_buffer("urb read", (u8 *) data32, urb->actual_length + n);
	usbd_rcv_complete(endpoint, n, 0);

	return 0;
}

static int udc_read_urb_ep0(void)
{
	u32 *data32 = (u32 *) ep0_urb->buffer;
	u8 *data8 = (u8 *) ep0_urb->buffer;
	unsigned int i, n, w, b;

	usbdbg("read urb on ep 0");
#if defined(USBDDBG) && defined(USBDPARANOIA)
	usbdbg("urb: buf %p, buf_len %d, actual_len %d",
		ep0_urb->buffer, ep0_urb->buffer_length, ep0_urb->actual_length);
#endif

	n = readl(UDCBCR0);
	w = n / 4;
	b = n % 4;

	for (i = 0; i < w; i++) {
		data32[ep0_urb->actual_length / 4 + i] = readl(UDCDN(0));
		/* ep0_urb->actual_length += 4; */
	}

	for (i = 0; i < b; i++) {
		data8[ep0_urb->actual_length + w * 4 + i] = readb(UDCDN(0));
		/* ep0_urb->actual_length++; */
	}

	ep0_urb->actual_length += n;

	udc_dump_buffer("urb read", (u8 *) data32, ep0_urb->actual_length);

	writel(UDCCSR0_OPC | UDCCSR0_IPR, UDCCSR0);
	if (ep0_urb->actual_length == ep0_urb->device_request.wLength)
		return 1;

	return 0;
}

static void udc_handle_ep0(struct usb_endpoint_instance *endpoint)
{
	u32 udccsr0 = readl(UDCCSR0);
	u32 *data = (u32 *) &ep0_urb->device_request;
	int i;

	usbdbg("udccsr0 %x", udccsr0);

	/* Clear stall status */
	if (udccsr0 & UDCCSR0_SST) {
		usberr("clear stall status");
		writel(UDCCSR0_SST, UDCCSR0);
		ep0state = EP0_IDLE;
	}

	/* previous request unfinished?  non-error iff back-to-back ... */
	if ((udccsr0 & UDCCSR0_SA) != 0 && ep0state != EP0_IDLE)
		ep0state = EP0_IDLE;

	switch (ep0state) {

	case EP0_IDLE:
		udccsr0 = readl(UDCCSR0);
		/* Start control request? */
		if ((udccsr0 & (UDCCSR0_OPC | UDCCSR0_SA | UDCCSR0_RNE))
			== (UDCCSR0_OPC | UDCCSR0_SA | UDCCSR0_RNE)) {

			/* Read SETUP packet.
			 * SETUP packet size is 8 bytes (aka 2 words)
			 */
			usbdbg("try reading SETUP packet");
			for (i = 0; i < 2; i++) {
				if ((readl(UDCCSR0) & UDCCSR0_RNE) == 0) {
					usberr("setup packet too short:%d", i);
					goto stall;
				}
				data[i] = readl(UDCDR0);
			}

			writel(readl(UDCCSR0) | UDCCSR0_OPC | UDCCSR0_SA, UDCCSR0);
			if ((readl(UDCCSR0) & UDCCSR0_RNE) != 0) {
				usberr("setup packet too long");
				goto stall;
			}

			udc_dump_buffer("ep0 setup read", (u8 *) data, 8);

			if (ep0_urb->device_request.wLength == 0) {
				usbdbg("Zero Data control Packet\n");
				if (ep0_recv_setup(ep0_urb)) {
					usberr("Invalid Setup Packet\n");
					udc_dump_buffer("ep0 setup read",
								(u8 *)data, 8);
					goto stall;
				}
				writel(UDCCSR0_IPR, UDCCSR0);
				ep0state = EP0_IDLE;
			} else {
				/* Check direction */
				if ((ep0_urb->device_request.bmRequestType &
						USB_REQ_DIRECTION_MASK)
						== USB_REQ_HOST2DEVICE) {
					ep0state = EP0_OUT_DATA;
					ep0_urb->buffer =
						(u8 *)ep0_urb->buffer_data;
					ep0_urb->buffer_length =
						sizeof(ep0_urb->buffer_data);
					ep0_urb->actual_length = 0;
					writel(UDCCSR0_IPR, UDCCSR0);
				} else {
					/* The ep0_recv_setup function has
					 * already placed our response packet
					 * data in ep0_urb->buffer and the
					 * packet length in
					 * ep0_urb->actual_length.
					 */
					if (ep0_recv_setup(ep0_urb)) {
stall:
						usberr("Invalid setup packet");
						udc_dump_buffer("ep0 setup read"
							, (u8 *) data, 8);
						ep0state = EP0_IDLE;

						writel(UDCCSR0_SA |
						UDCCSR0_OPC | UDCCSR0_FST |
						UDCCS0_FTF, UDCCSR0);

						return;
					}

					endpoint->tx_urb = ep0_urb;
					endpoint->sent = 0;
					usbdbg("EP0_IN_DATA");
					ep0state = EP0_IN_DATA;
					if (udc_write_urb(endpoint) < 0)
						goto stall;

				}
			}
			return;
		} else if ((udccsr0 & (UDCCSR0_OPC | UDCCSR0_SA))
			== (UDCCSR0_OPC|UDCCSR0_SA)) {
			usberr("Setup Active but no data. Stalling ....\n");
			goto stall;
		} else {
			usbdbg("random early IRQs");
			/* Some random early IRQs:
			 * - we acked FST
			 * - IPR cleared
			 * - OPC got set, without SA (likely status stage)
			 */
			writel(udccsr0 & (UDCCSR0_SA | UDCCSR0_OPC), UDCCSR0);
		}
		break;

	case EP0_OUT_DATA:

		if ((udccsr0 & UDCCSR0_OPC) && !(udccsr0 & UDCCSR0_SA)) {
			if (udc_read_urb_ep0()) {
read_complete:
				ep0state = EP0_IDLE;
				if (ep0_recv_setup(ep0_urb)) {
					/* Not a setup packet, stall next
					 * EP0 transaction
					 */
					udc_dump_buffer("ep0 setup read",
							(u8 *) data, 8);
					usberr("can't parse setup packet\n");
					goto stall;
				}
			}
		} else if (!(udccsr0 & UDCCSR0_OPC) &&
				!(udccsr0 & UDCCSR0_IPR)) {
			if (ep0_urb->device_request.wLength ==
				ep0_urb->actual_length)
				goto read_complete;

			usberr("Premature Status\n");
			ep0state = EP0_IDLE;
		}
		break;

	case EP0_IN_DATA:
		/* GET_DESCRIPTOR etc */
		if (udccsr0 & UDCCSR0_OPC) {
			writel(UDCCSR0_OPC | UDCCSR0_FTF, UDCCSR0);
			usberr("ep0in premature status");
			ep0state = EP0_IDLE;
		} else {
			/* irq was IPR clearing */
			if (udc_write_urb(endpoint) < 0) {
				usberr("ep0_write_error\n");
				goto stall;
			}
		}
		break;

	case EP0_XFER_COMPLETE:
		writel(UDCCSR0_IPR, UDCCSR0);
		ep0state = EP0_IDLE;
		break;

	default:
		usbdbg("Default\n");
	}
	writel(USIR0_IR0, USIR0);
}

static void udc_handle_ep(struct usb_endpoint_instance *endpoint)
{
	int ep_addr = endpoint->endpoint_address;
	int ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;
	int ep_isout = (ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT;

	u32 flags = readl(UDCCSN(ep_num)) & (UDCCSR_SST | UDCCSR_TRN);
	if (flags)
		writel(flags, UDCCSN(ep_num));

	if (ep_isout)
		udc_read_urb(endpoint);
	else
		udc_write_urb(endpoint);

	writel(UDCCSR_PC, UDCCSN(ep_num));
}

static void udc_state_changed(void)
{

	writel(readl(UDCCR) | UDCCR_SMAC, UDCCR);

	usbdbg("New UDC settings are: conf %d - inter %d - alter %d",
	       (readl(UDCCR) & UDCCR_ACN) >> UDCCR_ACN_S,
	       (readl(UDCCR) & UDCCR_AIN) >> UDCCR_AIN_S,
	       (readl(UDCCR) & UDCCR_AAISN) >> UDCCR_AAISN_S);

	usbd_device_event_irq(udc_device, DEVICE_CONFIGURED, 0);
	writel(UDCISR1_IRCC, UDCISR1);
}

void udc_irq(void)
{
	int handled;
	struct usb_endpoint_instance *endpoint;
	int ep_num, i;
	u32 udcisr0;

	do {
		handled = 0;
		/* Suspend Interrupt Request */
		if (readl(USIR1) & UDCCR_SUSIR) {
			usbdbg("Suspend\n");
			udc_ack_int_UDCCR(UDCCR_SUSIR);
			handled = 1;
			ep0state = EP0_IDLE;
		}

		/* Resume Interrupt Request */
		if (readl(USIR1) & UDCCR_RESIR) {
			udc_ack_int_UDCCR(UDCCR_RESIR);
			handled = 1;
			usbdbg("USB resume\n");
		}

		if (readl(USIR1) & (1<<31)) {
			handled = 1;
			udc_state_changed();
		}

		/* Reset Interrupt Request */
		if (readl(USIR1) & UDCCR_RSTIR) {
			udc_ack_int_UDCCR(UDCCR_RSTIR);
			handled = 1;
			usbdbg("Reset\n");
			usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
		} else {
			if (readl(USIR0))
				usbdbg("UISR0: %x \n", readl(USIR0));

			if (readl(USIR0) & 0x2)
				writel(0x2, USIR0);

			/* Control traffic */
			if (readl(USIR0)  & USIR0_IR0) {
				handled = 1;
				writel(USIR0_IR0, USIR0);
				udc_handle_ep0(udc_device->bus->endpoint_array);
			}

			endpoint = udc_device->bus->endpoint_array;
			for (i = 0; i < udc_device->bus->max_endpoints; i++) {
				ep_num = (endpoint[i].endpoint_address) &
						USB_ENDPOINT_NUMBER_MASK;
				if (!ep_num)
					continue;
				udcisr0 = readl(UDCISR0);
				if (udcisr0 &
					UDCISR_INT(ep_num, UDC_INT_PACKETCMP)) {
					writel(UDCISR_INT(ep_num, UDC_INT_PACKETCMP),
					       UDCISR0);
					udc_handle_ep(&endpoint[i]);
				}
			}
		}

	} while (handled);
}

/* The UDCCR reg contains mask and interrupt status bits,
 * so using '|=' isn't safe as it may ack an interrupt.
 */
#define UDCCR_OEN		(1 << 31)   /* On-the-Go Enable */
#define UDCCR_MASK_BITS     	(UDCCR_OEN | UDCCR_UDE)

static inline void udc_set_mask_UDCCR(int mask)
{
    writel((readl(UDCCR) & UDCCR_MASK_BITS) | (mask & UDCCR_MASK_BITS), UDCCR);
}

static inline void udc_clear_mask_UDCCR(int mask)
{
    writel((readl(UDCCR) & UDCCR_MASK_BITS) & ~(mask & UDCCR_MASK_BITS), UDCCR);
}

static void pio_irq_enable(int ep_num)
{
	if (ep_num < 16)
		writel(readl(UDCICR0) | 3 << (ep_num * 2), UDCICR0);
	else {
		ep_num -= 16;
		writel(readl(UDCICR1) | 3 << (ep_num * 2), UDCICR1);
	}
}

/*
 * udc_set_nak
 *
 * Allow upper layers to signal lower layers should not accept more RX data
 */
void udc_set_nak(int ep_num)
{
	/* TODO */
}

/*
 * udc_unset_nak
 *
 * Suspend sending of NAK tokens for DATA OUT tokens on a given endpoint.
 * Switch off NAKing on this endpoint to accept more data output from host.
 */
void udc_unset_nak(int ep_num)
{
	/* TODO */
}

int udc_endpoint_write(struct usb_endpoint_instance *endpoint)
{
	return udc_write_urb(endpoint);
}

/* Associate a physical endpoint with endpoint instance */
void udc_setup_ep(struct usb_device_instance *device, unsigned int id,
				struct usb_endpoint_instance *endpoint)
{
	int ep_num, ep_addr, ep_isout, ep_type, ep_size;
	int config, interface, alternate;
	u32 tmp;

	usbdbg("setting up endpoint id %d", id);

	if (!endpoint) {
		usberr("endpoint void!");
		return;
	}

	ep_num = endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK;
	if (ep_num >= UDC_MAX_ENDPOINTS) {
		usberr("unable to setup ep %d!", ep_num);
		return;
	}

	pio_irq_enable(ep_num);
	if (ep_num == 0) {
		/* Done for ep0 */
		return;
	}

	config = 1;
	interface = 0;
	alternate = 0;

	usbdbg("config %d - interface %d - alternate %d",
		config, interface, alternate);

	ep_addr = endpoint->endpoint_address;
	ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;
	ep_isout = (ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT;
	ep_type = ep_isout ? endpoint->rcv_attributes : endpoint->tx_attributes;
	ep_size = ep_isout ? endpoint->rcv_packetSize : endpoint->tx_packetSize;

	usbdbg("addr %x, num %d, dir %s, type %s, packet size %d",
		ep_addr, ep_num,
		ep_isout ? "out" : "in",
		ep_type == USB_ENDPOINT_XFER_ISOC ? "isoc" :
		ep_type == USB_ENDPOINT_XFER_BULK ? "bulk" :
		ep_type == USB_ENDPOINT_XFER_INT ? "int" : "???",
		ep_size
		);

	/* Configure UDCCRx */
	tmp = 0;
	tmp |= (config << UDCCONR_CN_S) & UDCCONR_CN;
	tmp |= (interface << UDCCONR_IN_S) & UDCCONR_IN;
	tmp |= (alternate << UDCCONR_AISN_S) & UDCCONR_AISN;
	tmp |= (ep_num << UDCCONR_EN_S) & UDCCONR_EN;
	tmp |= (ep_type << UDCCONR_ET_S) & UDCCONR_ET;
	tmp |= ep_isout ? 0 : UDCCONR_ED;
	tmp |= (ep_size << UDCCONR_MPS_S) & UDCCONR_MPS;
	tmp |= UDCCONR_EE;

	writel(tmp, UDCCN(ep_num));

	usbdbg("UDCCR%c = %x", 'A' + ep_num-1, readl(UDCCN(ep_num)));
	usbdbg("UDCCSR%c = %x", 'A' + ep_num-1, readl(UDCCSN(ep_num)));
}

/* Connect the USB device to the bus */
void udc_connect(void)
{
	usbdbg("UDC connect");

#ifdef CONFIG_USB_DEV_PULLUP_GPIO
	/* Turn on the USB connection by enabling the pullup resistor */
	writel(readl(GPDR(CONFIG_USB_DEV_PULLUP_GPIO))
		     | GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO),
	       GPDR(CONFIG_USB_DEV_PULLUP_GPIO));
	writel(GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO), GPSR(CONFIG_USB_DEV_PULLUP_GPIO));
#else
	/* Host port 2 transceiver D+ pull up enable */
	writel(readl(UP2OCR) | UP2OCR_DPPUE, UP2OCR);
#endif
}

/* Disconnect the USB device to the bus */
void udc_disconnect(void)
{
	usbdbg("UDC disconnect");

#ifdef CONFIG_USB_DEV_PULLUP_GPIO
	/* Turn off the USB connection by disabling the pullup resistor */
	writel(GPIO_bit(CONFIG_USB_DEV_PULLUP_GPIO), GPCR(CONFIG_USB_DEV_PULLUP_GPIO));
#else
	/* Host port 2 transceiver D+ pull up disable */
	writel(readl(UP2OCR) & ~UP2OCR_DPPUE, UP2OCR);
#endif
}

/* Switch on the UDC */
void udc_enable(struct usb_device_instance *device)
{

	ep0state = EP0_IDLE;

	/* enable endpoint 0, A, B's Packet Complete Interrupt. */
	writel(0xffffffff, UDCICR0);
	writel(0xa8000000, UDCICR1);

	/* clear the interrupt status/control registers */
	writel(0xffffffff, UDCISR0);
	writel(0xffffffff, UDCISR1);

	/* set UDC-enable */
	udc_set_mask_UDCCR(UDCCR_UDE);

	udc_device = device;
	if (!ep0_urb)
		ep0_urb = usbd_alloc_urb(udc_device,
				udc_device->bus->endpoint_array);
	else
		usbinfo("ep0_urb %p already allocated", ep0_urb);

	usbdbg("UDC Enabled\n");
}

/* Need to check this again */
void udc_disable(void)
{
	usbdbg("disable UDC");

	udc_clear_mask_UDCCR(UDCCR_UDE);

	/* Disable clock for USB device */
	writel(readl(CKEN) & ~CKEN11_USB, CKEN);

	/* Free ep0 URB */
	if (ep0_urb) {
		usbd_dealloc_urb(ep0_urb);
		ep0_urb = NULL;
	}

	/* Reset device pointer */
	udc_device = NULL;
}

/* Allow udc code to do any additional startup */
void udc_startup_events(struct usb_device_instance *device)
{
	/* The DEVICE_INIT event puts the USB device in the state STATE_INIT */
	usbd_device_event_irq(device, DEVICE_INIT, 0);

	/* The DEVICE_CREATE event puts the USB device in the state
	 * STATE_ATTACHED */
	usbd_device_event_irq(device, DEVICE_CREATE, 0);

	/* Some USB controller driver implementations signal
	 * DEVICE_HUB_CONFIGURED and DEVICE_RESET events here.
	 * DEVICE_HUB_CONFIGURED causes a transition to the state
	 * STATE_POWERED, and DEVICE_RESET causes a transition to
	 * the state STATE_DEFAULT.
	 */
	udc_enable(device);
}

/* Initialize h/w stuff */
int udc_init(void)
{
	udc_device = NULL;
	usbdbg("PXA27x usbd start");

	/* Enable clock for USB device */
	writel(readl(CKEN) | CKEN11_USB, CKEN);

	/* Disable the UDC */
	udc_clear_mask_UDCCR(UDCCR_UDE);

	/* Disable IRQs: we don't use them */
	writel(0, UDCICR0);
	writel(0, UDCICR1);

	return 0;
}
