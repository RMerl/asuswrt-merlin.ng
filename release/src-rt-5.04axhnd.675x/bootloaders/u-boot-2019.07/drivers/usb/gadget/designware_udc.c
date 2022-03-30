// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on drivers/usb/gadget/omap1510_udc.c
 * TI OMAP1510 USB bus interface driver
 *
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <asm/io.h>

#include <usbdevice.h>
#include "ep0.h"
#include <usb/designware_udc.h>
#include <usb/udc.h>
#include <asm/arch/hardware.h>

#define UDC_INIT_MDELAY		80	/* Device settle delay */

/* Some kind of debugging output... */
#ifndef DEBUG_DWUSBTTY
#define UDCDBG(str)
#define UDCDBGA(fmt, args...)
#else
#define UDCDBG(str) serial_printf(str "\n")
#define UDCDBGA(fmt, args...) serial_printf(fmt "\n", ##args)
#endif

static struct urb *ep0_urb;
static struct usb_device_instance *udc_device;

static struct plug_regs *const plug_regs_p =
    (struct plug_regs * const)CONFIG_SYS_PLUG_BASE;
static struct udc_regs *const udc_regs_p =
    (struct udc_regs * const)CONFIG_SYS_USBD_BASE;
static struct udc_endp_regs *const outep_regs_p =
    &((struct udc_regs * const)CONFIG_SYS_USBD_BASE)->out_regs[0];
static struct udc_endp_regs *const inep_regs_p =
    &((struct udc_regs * const)CONFIG_SYS_USBD_BASE)->in_regs[0];

/*
 * udc_state_transition - Write the next packet to TxFIFO.
 * @initial:	Initial state.
 * @final:	Final state.
 *
 * Helper function to implement device state changes. The device states and
 * the events that transition between them are:
 *
 *				STATE_ATTACHED
 *				||	/\
 *				\/	||
 *	DEVICE_HUB_CONFIGURED			DEVICE_HUB_RESET
 *				||	/\
 *				\/	||
 *				STATE_POWERED
 *				||	/\
 *				\/	||
 *	DEVICE_RESET				DEVICE_POWER_INTERRUPTION
 *				||	/\
 *				\/	||
 *				STATE_DEFAULT
 *				||	/\
 *				\/	||
 *	DEVICE_ADDRESS_ASSIGNED			DEVICE_RESET
 *				||	/\
 *				\/	||
 *				STATE_ADDRESSED
 *				||	/\
 *				\/	||
 *	DEVICE_CONFIGURED			DEVICE_DE_CONFIGURED
 *				||	/\
 *				\/	||
 *				STATE_CONFIGURED
 *
 * udc_state_transition transitions up (in the direction from STATE_ATTACHED
 * to STATE_CONFIGURED) from the specified initial state to the specified final
 * state, passing through each intermediate state on the way. If the initial
 * state is at or above (i.e. nearer to STATE_CONFIGURED) the final state, then
 * no state transitions will take place.
 *
 * udc_state_transition also transitions down (in the direction from
 * STATE_CONFIGURED to STATE_ATTACHED) from the specified initial state to the
 * specified final state, passing through each intermediate state on the way.
 * If the initial state is at or below (i.e. nearer to STATE_ATTACHED) the final
 * state, then no state transitions will take place.
 *
 * This function must only be called with interrupts disabled.
 */
static void udc_state_transition(usb_device_state_t initial,
				 usb_device_state_t final)
{
	if (initial < final) {
		switch (initial) {
		case STATE_ATTACHED:
			usbd_device_event_irq(udc_device,
					      DEVICE_HUB_CONFIGURED, 0);
			if (final == STATE_POWERED)
				break;
		case STATE_POWERED:
			usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
			if (final == STATE_DEFAULT)
				break;
		case STATE_DEFAULT:
			usbd_device_event_irq(udc_device,
					      DEVICE_ADDRESS_ASSIGNED, 0);
			if (final == STATE_ADDRESSED)
				break;
		case STATE_ADDRESSED:
			usbd_device_event_irq(udc_device, DEVICE_CONFIGURED, 0);
		case STATE_CONFIGURED:
			break;
		default:
			break;
		}
	} else if (initial > final) {
		switch (initial) {
		case STATE_CONFIGURED:
			usbd_device_event_irq(udc_device,
					      DEVICE_DE_CONFIGURED, 0);
			if (final == STATE_ADDRESSED)
				break;
		case STATE_ADDRESSED:
			usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
			if (final == STATE_DEFAULT)
				break;
		case STATE_DEFAULT:
			usbd_device_event_irq(udc_device,
					      DEVICE_POWER_INTERRUPTION, 0);
			if (final == STATE_POWERED)
				break;
		case STATE_POWERED:
			usbd_device_event_irq(udc_device, DEVICE_HUB_RESET, 0);
		case STATE_ATTACHED:
			break;
		default:
			break;
		}
	}
}

/* Stall endpoint */
static void udc_stall_ep(u32 ep_num)
{
	writel(readl(&inep_regs_p[ep_num].endp_cntl) | ENDP_CNTL_STALL,
	       &inep_regs_p[ep_num].endp_cntl);

	writel(readl(&outep_regs_p[ep_num].endp_cntl) | ENDP_CNTL_STALL,
	       &outep_regs_p[ep_num].endp_cntl);
}

static void *get_fifo(int ep_num, int in)
{
	u32 *fifo_ptr = (u32 *)CONFIG_SYS_FIFO_BASE;

	switch (ep_num) {
	case UDC_EP3:
		fifo_ptr += readl(&inep_regs_p[1].endp_bsorfn);
		/* break intentionally left out */

	case UDC_EP1:
		fifo_ptr += readl(&inep_regs_p[0].endp_bsorfn);
		/* break intentionally left out */

	case UDC_EP0:
	default:
		if (in) {
			fifo_ptr +=
			    readl(&outep_regs_p[2].endp_maxpacksize) >> 16;
			/* break intentionally left out */
		} else {
			break;
		}

	case UDC_EP2:
		fifo_ptr += readl(&outep_regs_p[0].endp_maxpacksize) >> 16;
		/* break intentionally left out */
	}

	return (void *)fifo_ptr;
}

static int usbgetpckfromfifo(int epNum, u8 *bufp, u32 len)
{
	u8 *fifo_ptr = (u8 *)get_fifo(epNum, 0);
	u32 i, nw, nb;
	u32 *wrdp;
	u8 *bytp;
	u32 tmp[128];

	if (readl(&udc_regs_p->dev_stat) & DEV_STAT_RXFIFO_EMPTY)
		return -1;

	nw = len / sizeof(u32);
	nb = len % sizeof(u32);

	/* use tmp buf if bufp is not word aligned */
	if ((int)bufp & 0x3)
		wrdp = (u32 *)&tmp[0];
	else
		wrdp = (u32 *)bufp;

	for (i = 0; i < nw; i++) {
		writel(readl(fifo_ptr), wrdp);
		wrdp++;
	}

	bytp = (u8 *)wrdp;
	for (i = 0; i < nb; i++) {
		writeb(readb(fifo_ptr), bytp);
		fifo_ptr++;
		bytp++;
	}
	readl(&outep_regs_p[epNum].write_done);

	/* copy back tmp buffer to bufp if bufp is not word aligned */
	if ((int)bufp & 0x3)
		memcpy(bufp, tmp, len);

	return 0;
}

static void usbputpcktofifo(int epNum, u8 *bufp, u32 len)
{
	u32 i, nw, nb;
	u32 *wrdp;
	u8 *bytp;
	u8 *fifo_ptr = get_fifo(epNum, 1);

	nw = len / sizeof(int);
	nb = len % sizeof(int);
	wrdp = (u32 *)bufp;
	for (i = 0; i < nw; i++) {
		writel(*wrdp, fifo_ptr);
		wrdp++;
	}

	bytp = (u8 *)wrdp;
	for (i = 0; i < nb; i++) {
		writeb(*bytp, fifo_ptr);
		fifo_ptr++;
		bytp++;
	}
}

/*
 * dw_write_noniso_tx_fifo - Write the next packet to TxFIFO.
 * @endpoint:		Endpoint pointer.
 *
 * If the endpoint has an active tx_urb, then the next packet of data from the
 * URB is written to the tx FIFO.  The total amount of data in the urb is given
 * by urb->actual_length.  The maximum amount of data that can be sent in any
 * one packet is given by endpoint->tx_packetSize.  The number of data bytes
 * from this URB that have already been transmitted is given by endpoint->sent.
 * endpoint->last is updated by this routine with the number of data bytes
 * transmitted in this packet.
 *
 */
static void dw_write_noniso_tx_fifo(struct usb_endpoint_instance
				       *endpoint)
{
	struct urb *urb = endpoint->tx_urb;
	int align;

	if (urb) {
		u32 last;

		UDCDBGA("urb->buffer %p, buffer_length %d, actual_length %d",
			urb->buffer, urb->buffer_length, urb->actual_length);

		last = min_t(u32, urb->actual_length - endpoint->sent,
			     endpoint->tx_packetSize);

		if (last) {
			u8 *cp = urb->buffer + endpoint->sent;

			/*
			 * This ensures that USBD packet fifo is accessed
			 * - through word aligned pointer or
			 * - through non word aligned pointer but only
			 *   with a max length to make the next packet
			 *   word aligned
			 */

			align = ((ulong)cp % sizeof(int));
			if (align)
				last = min(last, sizeof(int) - align);

			UDCDBGA("endpoint->sent %d, tx_packetSize %d, last %d",
				endpoint->sent, endpoint->tx_packetSize, last);

			usbputpcktofifo(endpoint->endpoint_address &
					USB_ENDPOINT_NUMBER_MASK, cp, last);
		}
		endpoint->last = last;
	}
}

/*
 * Handle SETUP USB interrupt.
 * This function implements TRM Figure 14-14.
 */
static void dw_udc_setup(struct usb_endpoint_instance *endpoint)
{
	u8 *datap = (u8 *)&ep0_urb->device_request;
	int ep_addr = endpoint->endpoint_address;

	UDCDBG("-> Entering device setup");
	usbgetpckfromfifo(ep_addr, datap, 8);

	/* Try to process setup packet */
	if (ep0_recv_setup(ep0_urb)) {
		/* Not a setup packet, stall next EP0 transaction */
		udc_stall_ep(0);
		UDCDBG("can't parse setup packet, still waiting for setup");
		return;
	}

	/* Check direction */
	if ((ep0_urb->device_request.bmRequestType & USB_REQ_DIRECTION_MASK)
	    == USB_REQ_HOST2DEVICE) {
		UDCDBG("control write on EP0");
		if (le16_to_cpu(ep0_urb->device_request.wLength)) {
			/* Stall this request */
			UDCDBG("Stalling unsupported EP0 control write data "
			       "stage.");
			udc_stall_ep(0);
		}
	} else {

		UDCDBG("control read on EP0");
		/*
		 * The ep0_recv_setup function has already placed our response
		 * packet data in ep0_urb->buffer and the packet length in
		 * ep0_urb->actual_length.
		 */
		endpoint->tx_urb = ep0_urb;
		endpoint->sent = 0;
		/*
		 * Write packet data to the FIFO.  dw_write_noniso_tx_fifo
		 * will update endpoint->last with the number of bytes written
		 * to the FIFO.
		 */
		dw_write_noniso_tx_fifo(endpoint);

		writel(0x0, &inep_regs_p[ep_addr].write_done);
	}

	udc_unset_nak(endpoint->endpoint_address);

	UDCDBG("<- Leaving device setup");
}

/*
 * Handle endpoint 0 RX interrupt
 */
static void dw_udc_ep0_rx(struct usb_endpoint_instance *endpoint)
{
	u8 dummy[64];

	UDCDBG("RX on EP0");

	/* Check direction */
	if ((ep0_urb->device_request.bmRequestType
	     & USB_REQ_DIRECTION_MASK) == USB_REQ_HOST2DEVICE) {
		/*
		 * This rx interrupt must be for a control write data
		 * stage packet.
		 *
		 * We don't support control write data stages.
		 * We should never end up here.
		 */

		UDCDBG("Stalling unexpected EP0 control write "
		       "data stage packet");
		udc_stall_ep(0);
	} else {
		/*
		 * This rx interrupt must be for a control read status
		 * stage packet.
		 */
		UDCDBG("ACK on EP0 control read status stage packet");
		u32 len = (readl(&outep_regs_p[0].endp_status) >> 11) & 0xfff;
		usbgetpckfromfifo(0, dummy, len);
	}
}

/*
 * Handle endpoint 0 TX interrupt
 */
static void dw_udc_ep0_tx(struct usb_endpoint_instance *endpoint)
{
	struct usb_device_request *request = &ep0_urb->device_request;
	int ep_addr;

	UDCDBG("TX on EP0");

	/* Check direction */
	if ((request->bmRequestType & USB_REQ_DIRECTION_MASK) ==
	    USB_REQ_HOST2DEVICE) {
		/*
		 * This tx interrupt must be for a control write status
		 * stage packet.
		 */
		UDCDBG("ACK on EP0 control write status stage packet");
	} else {
		/*
		 * This tx interrupt must be for a control read data
		 * stage packet.
		 */
		int wLength = le16_to_cpu(request->wLength);

		/*
		 * Update our count of bytes sent so far in this
		 * transfer.
		 */
		endpoint->sent += endpoint->last;

		/*
		 * We are finished with this transfer if we have sent
		 * all of the bytes in our tx urb (urb->actual_length)
		 * unless we need a zero-length terminating packet.  We
		 * need a zero-length terminating packet if we returned
		 * fewer bytes than were requested (wLength) by the host,
		 * and the number of bytes we returned is an exact
		 * multiple of the packet size endpoint->tx_packetSize.
		 */
		if ((endpoint->sent == ep0_urb->actual_length) &&
		    ((ep0_urb->actual_length == wLength) ||
		     (endpoint->last != endpoint->tx_packetSize))) {
			/* Done with control read data stage. */
			UDCDBG("control read data stage complete");
		} else {
			/*
			 * We still have another packet of data to send
			 * in this control read data stage or else we
			 * need a zero-length terminating packet.
			 */
			UDCDBG("ACK control read data stage packet");
			dw_write_noniso_tx_fifo(endpoint);

			ep_addr = endpoint->endpoint_address;
			writel(0x0, &inep_regs_p[ep_addr].write_done);
		}
	}
}

static struct usb_endpoint_instance *dw_find_ep(int ep)
{
	int i;

	for (i = 0; i < udc_device->bus->max_endpoints; i++) {
		if ((udc_device->bus->endpoint_array[i].endpoint_address &
		     USB_ENDPOINT_NUMBER_MASK) == ep)
			return &udc_device->bus->endpoint_array[i];
	}
	return NULL;
}

/*
 * Handle RX transaction on non-ISO endpoint.
 * The ep argument is a physical endpoint number for a non-ISO IN endpoint
 * in the range 1 to 15.
 */
static void dw_udc_epn_rx(int ep)
{
	int nbytes = 0;
	struct urb *urb;
	struct usb_endpoint_instance *endpoint = dw_find_ep(ep);

	if (endpoint) {
		urb = endpoint->rcv_urb;

		if (urb) {
			u8 *cp = urb->buffer + urb->actual_length;

			nbytes = (readl(&outep_regs_p[ep].endp_status) >> 11) &
			    0xfff;
			usbgetpckfromfifo(ep, cp, nbytes);
			usbd_rcv_complete(endpoint, nbytes, 0);
		}
	}
}

/*
 * Handle TX transaction on non-ISO endpoint.
 * The ep argument is a physical endpoint number for a non-ISO IN endpoint
 * in the range 16 to 30.
 */
static void dw_udc_epn_tx(int ep)
{
	struct usb_endpoint_instance *endpoint = dw_find_ep(ep);

	if (!endpoint)
		return;

	/*
	 * We need to transmit a terminating zero-length packet now if
	 * we have sent all of the data in this URB and the transfer
	 * size was an exact multiple of the packet size.
	 */
	if (endpoint->tx_urb &&
	    (endpoint->last == endpoint->tx_packetSize) &&
	    (endpoint->tx_urb->actual_length - endpoint->sent -
	     endpoint->last == 0)) {
		/* handle zero length packet here */
		writel(0x0, &inep_regs_p[ep].write_done);

	}

	if (endpoint->tx_urb && endpoint->tx_urb->actual_length) {
		/* retire the data that was just sent */
		usbd_tx_complete(endpoint);
		/*
		 * Check to see if we have more data ready to transmit
		 * now.
		 */
		if (endpoint->tx_urb && endpoint->tx_urb->actual_length) {
			/* write data to FIFO */
			dw_write_noniso_tx_fifo(endpoint);
			writel(0x0, &inep_regs_p[ep].write_done);

		} else if (endpoint->tx_urb
			   && (endpoint->tx_urb->actual_length == 0)) {
			/* udc_set_nak(ep); */
		}
	}
}

/*
 * Start of public functions.
 */

/* Called to start packet transmission. */
int udc_endpoint_write(struct usb_endpoint_instance *endpoint)
{
	udc_unset_nak(endpoint->endpoint_address & USB_ENDPOINT_NUMBER_MASK);
	return 0;
}

/* Start to initialize h/w stuff */
int udc_init(void)
{
	int i;
	u32 plug_st;

	udc_device = NULL;

	UDCDBG("starting");

	readl(&plug_regs_p->plug_pending);

	for (i = 0; i < UDC_INIT_MDELAY; i++)
		udelay(1000);

	plug_st = readl(&plug_regs_p->plug_state);
	writel(plug_st | PLUG_STATUS_EN, &plug_regs_p->plug_state);

	writel(~0x0, &udc_regs_p->endp_int);
	writel(~0x0, &udc_regs_p->dev_int_mask);
	writel(~0x0, &udc_regs_p->endp_int_mask);

#ifndef CONFIG_USBD_HS
	writel(DEV_CONF_FS_SPEED | DEV_CONF_REMWAKEUP | DEV_CONF_SELFPOW |
	       DEV_CONF_PHYINT_16, &udc_regs_p->dev_conf);
#else
	writel(DEV_CONF_HS_SPEED | DEV_CONF_REMWAKEUP | DEV_CONF_SELFPOW |
			DEV_CONF_PHYINT_16, &udc_regs_p->dev_conf);
#endif

	writel(DEV_CNTL_SOFTDISCONNECT, &udc_regs_p->dev_cntl);

	/* Clear all interrupts pending */
	writel(DEV_INT_MSK, &udc_regs_p->dev_int);

	return 0;
}

int is_usbd_high_speed(void)
{
	return (readl(&udc_regs_p->dev_stat) & DEV_STAT_ENUM) ? 0 : 1;
}

/*
 * udc_setup_ep - setup endpoint
 * Associate a physical endpoint with endpoint_instance
 */
void udc_setup_ep(struct usb_device_instance *device,
		  u32 ep, struct usb_endpoint_instance *endpoint)
{
	UDCDBGA("setting up endpoint addr %x", endpoint->endpoint_address);
	int ep_addr;
	int ep_num, ep_type;
	int packet_size;
	int buffer_size;
	int attributes;
	char *tt;
	u32 endp_intmask;

	if ((ep != 0) && (udc_device->device_state < STATE_ADDRESSED))
		return;

	tt = env_get("usbtty");
	if (!tt)
		tt = "generic";

	ep_addr = endpoint->endpoint_address;
	ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;

	if ((ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN) {
		/* IN endpoint */
		packet_size = endpoint->tx_packetSize;
		buffer_size = packet_size * 2;
		attributes = endpoint->tx_attributes;
	} else {
		/* OUT endpoint */
		packet_size = endpoint->rcv_packetSize;
		buffer_size = packet_size * 2;
		attributes = endpoint->rcv_attributes;
	}

	switch (attributes & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_CONTROL:
		ep_type = ENDP_EPTYPE_CNTL;
		break;
	case USB_ENDPOINT_XFER_BULK:
	default:
		ep_type = ENDP_EPTYPE_BULK;
		break;
	case USB_ENDPOINT_XFER_INT:
		ep_type = ENDP_EPTYPE_INT;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		ep_type = ENDP_EPTYPE_ISO;
		break;
	}

	struct udc_endp_regs *out_p = &outep_regs_p[ep_num];
	struct udc_endp_regs *in_p = &inep_regs_p[ep_num];

	if (!ep_addr) {
		/* Setup endpoint 0 */
		buffer_size = packet_size;

		writel(readl(&in_p->endp_cntl) | ENDP_CNTL_CNAK,
		       &in_p->endp_cntl);

		writel(readl(&out_p->endp_cntl) | ENDP_CNTL_CNAK,
		       &out_p->endp_cntl);

		writel(ENDP_CNTL_CONTROL | ENDP_CNTL_FLUSH, &in_p->endp_cntl);

		writel(buffer_size / sizeof(int), &in_p->endp_bsorfn);

		writel(packet_size, &in_p->endp_maxpacksize);

		writel(ENDP_CNTL_CONTROL | ENDP_CNTL_RRDY, &out_p->endp_cntl);

		writel(packet_size | ((buffer_size / sizeof(int)) << 16),
		       &out_p->endp_maxpacksize);

	} else if ((ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN) {
		/* Setup the IN endpoint */
		writel(0x0, &in_p->endp_status);
		writel((ep_type << 4) | ENDP_CNTL_RRDY, &in_p->endp_cntl);
		writel(buffer_size / sizeof(int), &in_p->endp_bsorfn);
		writel(packet_size, &in_p->endp_maxpacksize);

		if (!strcmp(tt, "cdc_acm")) {
			if (ep_type == ENDP_EPTYPE_INT) {
				/* Conf no. 1 Interface no. 0 */
				writel((packet_size << 19) |
				       ENDP_EPDIR_IN | (1 << 7) |
				       (0 << 11) | (ep_type << 5) | ep_num,
				       &udc_regs_p->udc_endp_reg[ep_num]);
			} else {
				/* Conf no. 1 Interface no. 1 */
				writel((packet_size << 19) |
				       ENDP_EPDIR_IN | (1 << 7) |
				       (1 << 11) | (ep_type << 5) | ep_num,
				       &udc_regs_p->udc_endp_reg[ep_num]);
			}
		} else {
			/* Conf no. 1 Interface no. 0 */
			writel((packet_size << 19) |
			       ENDP_EPDIR_IN | (1 << 7) |
			       (0 << 11) | (ep_type << 5) | ep_num,
			       &udc_regs_p->udc_endp_reg[ep_num]);
		}

	} else {
		/* Setup the OUT endpoint */
		writel(0x0, &out_p->endp_status);
		writel((ep_type << 4) | ENDP_CNTL_RRDY, &out_p->endp_cntl);
		writel(packet_size | ((buffer_size / sizeof(int)) << 16),
		       &out_p->endp_maxpacksize);

		if (!strcmp(tt, "cdc_acm")) {
			writel((packet_size << 19) |
			       ENDP_EPDIR_OUT | (1 << 7) |
			       (1 << 11) | (ep_type << 5) | ep_num,
			       &udc_regs_p->udc_endp_reg[ep_num]);
		} else {
			writel((packet_size << 19) |
			       ENDP_EPDIR_OUT | (1 << 7) |
			       (0 << 11) | (ep_type << 5) | ep_num,
			       &udc_regs_p->udc_endp_reg[ep_num]);
		}

	}

	endp_intmask = readl(&udc_regs_p->endp_int_mask);
	endp_intmask &= ~((1 << ep_num) | 0x10000 << ep_num);
	writel(endp_intmask, &udc_regs_p->endp_int_mask);
}

/* Turn on the USB connection by enabling the pullup resistor */
void udc_connect(void)
{
	u32 plug_st, dev_cntl;

	dev_cntl = readl(&udc_regs_p->dev_cntl);
	dev_cntl |= DEV_CNTL_SOFTDISCONNECT;
	writel(dev_cntl, &udc_regs_p->dev_cntl);

	udelay(1000);

	dev_cntl = readl(&udc_regs_p->dev_cntl);
	dev_cntl &= ~DEV_CNTL_SOFTDISCONNECT;
	writel(dev_cntl, &udc_regs_p->dev_cntl);

	plug_st = readl(&plug_regs_p->plug_state);
	plug_st &= ~(PLUG_STATUS_PHY_RESET | PLUG_STATUS_PHY_MODE);
	writel(plug_st, &plug_regs_p->plug_state);
}

/* Turn off the USB connection by disabling the pullup resistor */
void udc_disconnect(void)
{
	u32 plug_st;

	writel(DEV_CNTL_SOFTDISCONNECT, &udc_regs_p->dev_cntl);

	plug_st = readl(&plug_regs_p->plug_state);
	plug_st |= (PLUG_STATUS_PHY_RESET | PLUG_STATUS_PHY_MODE);
	writel(plug_st, &plug_regs_p->plug_state);
}

/* Switch on the UDC */
void udc_enable(struct usb_device_instance *device)
{
	UDCDBGA("enable device %p, status %d", device, device->status);

	/* Save the device structure pointer */
	udc_device = device;

	/* Setup ep0 urb */
	if (!ep0_urb) {
		ep0_urb =
		    usbd_alloc_urb(udc_device, udc_device->bus->endpoint_array);
	} else {
		serial_printf("udc_enable: ep0_urb already allocated %p\n",
			      ep0_urb);
	}

	writel(DEV_INT_SOF, &udc_regs_p->dev_int_mask);
}

/**
 * udc_startup - allow udc code to do any additional startup
 */
void udc_startup_events(struct usb_device_instance *device)
{
	/* The DEVICE_INIT event puts the USB device in the state STATE_INIT. */
	usbd_device_event_irq(device, DEVICE_INIT, 0);

	/*
	 * The DEVICE_CREATE event puts the USB device in the state
	 * STATE_ATTACHED.
	 */
	usbd_device_event_irq(device, DEVICE_CREATE, 0);

	/*
	 * Some USB controller driver implementations signal
	 * DEVICE_HUB_CONFIGURED and DEVICE_RESET events here.
	 * DEVICE_HUB_CONFIGURED causes a transition to the state STATE_POWERED,
	 * and DEVICE_RESET causes a transition to the state STATE_DEFAULT.
	 * The DW USB client controller has the capability to detect when the
	 * USB cable is connected to a powered USB bus, so we will defer the
	 * DEVICE_HUB_CONFIGURED and DEVICE_RESET events until later.
	 */

	udc_enable(device);
}

/*
 * Plug detection interrupt handling
 */
static void dw_udc_plug_irq(void)
{
	if (readl(&plug_regs_p->plug_state) & PLUG_STATUS_ATTACHED) {
		/*
		 * USB cable attached
		 * Turn off PHY reset bit (PLUG detect).
		 * Switch PHY opmode to normal operation (PLUG detect).
		 */
		udc_connect();
		writel(DEV_INT_SOF, &udc_regs_p->dev_int_mask);

		UDCDBG("device attached and powered");
		udc_state_transition(udc_device->device_state, STATE_POWERED);
	} else {
		writel(~0x0, &udc_regs_p->dev_int_mask);

		UDCDBG("device detached or unpowered");
		udc_state_transition(udc_device->device_state, STATE_ATTACHED);
	}
}

/*
 * Device interrupt handling
 */
static void dw_udc_dev_irq(void)
{
	if (readl(&udc_regs_p->dev_int) & DEV_INT_USBRESET) {
		writel(~0x0, &udc_regs_p->endp_int_mask);

		writel(readl(&inep_regs_p[0].endp_cntl) | ENDP_CNTL_FLUSH,
		       &inep_regs_p[0].endp_cntl);

		writel(DEV_INT_USBRESET, &udc_regs_p->dev_int);

		/*
		 * This endpoint0 specific register can be programmed only
		 * after the phy clock is initialized
		 */
		writel((EP0_MAX_PACKET_SIZE << 19) | ENDP_EPTYPE_CNTL,
				&udc_regs_p->udc_endp_reg[0]);

		UDCDBG("device reset in progess");
		udc_state_transition(udc_device->device_state, STATE_DEFAULT);
	}

	/* Device Enumeration completed */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_ENUM) {
		writel(DEV_INT_ENUM, &udc_regs_p->dev_int);

		/* Endpoint interrupt enabled for Ctrl IN & Ctrl OUT */
		writel(readl(&udc_regs_p->endp_int_mask) & ~0x10001,
		       &udc_regs_p->endp_int_mask);

		UDCDBG("default -> addressed");
		udc_state_transition(udc_device->device_state, STATE_ADDRESSED);
	}

	/* The USB will be in SUSPEND in 3 ms */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_INACTIVE) {
		writel(DEV_INT_INACTIVE, &udc_regs_p->dev_int);

		UDCDBG("entering inactive state");
		/* usbd_device_event_irq(udc_device, DEVICE_BUS_INACTIVE, 0); */
	}

	/* SetConfiguration command received */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_SETCFG) {
		writel(DEV_INT_SETCFG, &udc_regs_p->dev_int);

		UDCDBG("entering configured state");
		udc_state_transition(udc_device->device_state,
				     STATE_CONFIGURED);
	}

	/* SetInterface command received */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_SETINTF)
		writel(DEV_INT_SETINTF, &udc_regs_p->dev_int);

	/* USB Suspend detected on cable */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_SUSPUSB) {
		writel(DEV_INT_SUSPUSB, &udc_regs_p->dev_int);

		UDCDBG("entering suspended state");
		usbd_device_event_irq(udc_device, DEVICE_BUS_INACTIVE, 0);
	}

	/* USB Start-Of-Frame detected on cable */
	if (readl(&udc_regs_p->dev_int) & DEV_INT_SOF)
		writel(DEV_INT_SOF, &udc_regs_p->dev_int);
}

/*
 * Endpoint interrupt handling
 */
static void dw_udc_endpoint_irq(void)
{
	while (readl(&udc_regs_p->endp_int) & ENDP0_INT_CTRLOUT) {

		writel(ENDP0_INT_CTRLOUT, &udc_regs_p->endp_int);

		if ((readl(&outep_regs_p[0].endp_status) & ENDP_STATUS_OUTMSK)
		    == ENDP_STATUS_OUT_SETUP) {
			dw_udc_setup(udc_device->bus->endpoint_array + 0);
			writel(ENDP_STATUS_OUT_SETUP,
			       &outep_regs_p[0].endp_status);

		} else if ((readl(&outep_regs_p[0].endp_status) &
			    ENDP_STATUS_OUTMSK) == ENDP_STATUS_OUT_DATA) {
			dw_udc_ep0_rx(udc_device->bus->endpoint_array + 0);
			writel(ENDP_STATUS_OUT_DATA,
			       &outep_regs_p[0].endp_status);

		} else if ((readl(&outep_regs_p[0].endp_status) &
			    ENDP_STATUS_OUTMSK) == ENDP_STATUS_OUT_NONE) {
			/* NONE received */
		}

		writel(0x0, &outep_regs_p[0].endp_status);
	}

	if (readl(&udc_regs_p->endp_int) & ENDP0_INT_CTRLIN) {
		dw_udc_ep0_tx(udc_device->bus->endpoint_array + 0);

		writel(ENDP_STATUS_IN, &inep_regs_p[0].endp_status);
		writel(ENDP0_INT_CTRLIN, &udc_regs_p->endp_int);
	}

	if (readl(&udc_regs_p->endp_int) & ENDP_INT_NONISOOUT_MSK) {
		u32 epnum = 0;
		u32 ep_int = readl(&udc_regs_p->endp_int) &
		    ENDP_INT_NONISOOUT_MSK;

		ep_int >>= 16;
		while (0x0 == (ep_int & 0x1)) {
			ep_int >>= 1;
			epnum++;
		}

		writel((1 << 16) << epnum, &udc_regs_p->endp_int);

		if ((readl(&outep_regs_p[epnum].endp_status) &
		     ENDP_STATUS_OUTMSK) == ENDP_STATUS_OUT_DATA) {

			dw_udc_epn_rx(epnum);
			writel(ENDP_STATUS_OUT_DATA,
			       &outep_regs_p[epnum].endp_status);
		} else if ((readl(&outep_regs_p[epnum].endp_status) &
			    ENDP_STATUS_OUTMSK) == ENDP_STATUS_OUT_NONE) {
			writel(0x0, &outep_regs_p[epnum].endp_status);
		}
	}

	if (readl(&udc_regs_p->endp_int) & ENDP_INT_NONISOIN_MSK) {
		u32 epnum = 0;
		u32 ep_int = readl(&udc_regs_p->endp_int) &
		    ENDP_INT_NONISOIN_MSK;

		while (0x0 == (ep_int & 0x1)) {
			ep_int >>= 1;
			epnum++;
		}

		if (readl(&inep_regs_p[epnum].endp_status) & ENDP_STATUS_IN) {
			writel(ENDP_STATUS_IN,
			       &outep_regs_p[epnum].endp_status);
			dw_udc_epn_tx(epnum);

			writel(ENDP_STATUS_IN,
			       &outep_regs_p[epnum].endp_status);
		}

		writel((1 << epnum), &udc_regs_p->endp_int);
	}
}

/*
 * UDC interrupts
 */
void udc_irq(void)
{
	/*
	 * Loop while we have interrupts.
	 * If we don't do this, the input chain
	 * polling delay is likely to miss
	 * host requests.
	 */
	while (readl(&plug_regs_p->plug_pending))
		dw_udc_plug_irq();

	while (readl(&udc_regs_p->dev_int))
		dw_udc_dev_irq();

	if (readl(&udc_regs_p->endp_int))
		dw_udc_endpoint_irq();
}

/* Flow control */
void udc_set_nak(int epid)
{
	writel(readl(&inep_regs_p[epid].endp_cntl) | ENDP_CNTL_SNAK,
	       &inep_regs_p[epid].endp_cntl);

	writel(readl(&outep_regs_p[epid].endp_cntl) | ENDP_CNTL_SNAK,
	       &outep_regs_p[epid].endp_cntl);
}

void udc_unset_nak(int epid)
{
	u32 val;

	val = readl(&inep_regs_p[epid].endp_cntl);
	val &= ~ENDP_CNTL_SNAK;
	val |= ENDP_CNTL_CNAK;
	writel(val, &inep_regs_p[epid].endp_cntl);

	val = readl(&outep_regs_p[epid].endp_cntl);
	val &= ~ENDP_CNTL_SNAK;
	val |= ENDP_CNTL_CNAK;
	writel(val, &outep_regs_p[epid].endp_cntl);
}
