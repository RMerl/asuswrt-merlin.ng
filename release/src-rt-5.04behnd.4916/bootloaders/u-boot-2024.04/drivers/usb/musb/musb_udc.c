// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This file is a rewrite of the usb device part of
 * repository git.omapzoom.org/repo/u-boot.git, branch master,
 * file cpu/omap3/fastboot.c
 *
 * This is the unique part of its copyright :
 *
 * -------------------------------------------------------------------------
 *
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * -------------------------------------------------------------------------
 *
 * The details of connecting the device to the uboot usb device subsystem
 * came from the old omap3 repository www.sakoman.net/u-boot-omap3.git,
 * branch omap3-dev-usb, file drivers/usb/usbdcore_musb.c
 *
 * This is the unique part of its copyright :
 *
 * -------------------------------------------------------------------------
 *
 * (C) Copyright 2008 Texas Instruments Incorporated.
 *
 * Based on
 * u-boot OMAP1510 USB drivers (drivers/usbdcore_omap1510.c)
 * twl4030 init based on linux (drivers/i2c/chips/twl4030_usb.c)
 *
 * Author: Diego Dompe (diego.dompe@ridgerun.com)
 *         Atin Malaviya (atin.malaviya@gmail.com)
 *
 * -------------------------------------------------------------------------
 */

#include <common.h>
#include <hang.h>
#include <serial.h>
#include <usbdevice.h>
#include <linux/delay.h>
#include <usb/udc.h>
#include "../gadget/ep0.h"
#include "musb_core.h"
#if defined(CONFIG_USB_OMAP3)
#include "omap3.h"
#elif defined(CONFIG_USB_AM35X)
#include "am35x.h"
#endif

/* Define MUSB_DEBUG for debugging */
/* #define MUSB_DEBUG */
#include "musb_debug.h"

#define MAX_ENDPOINT 15

#define GET_ENDPOINT(dev,ep)						\
(((struct usb_device_instance *)(dev))->bus->endpoint_array + ep)

#define SET_EP0_STATE(s)						\
do {									\
	if ((0 <= (s)) && (SET_ADDRESS >= (s))) {			\
		if ((s) != ep0_state) {					\
			if ((debug_setup) && (debug_level > 1))		\
				serial_printf("INFO : Changing state "  \
					      "from %s to %s in %s at " \
					      "line %d\n",		\
					      ep0_state_strings[ep0_state],\
					      ep0_state_strings[s],	\
					      __PRETTY_FUNCTION__,	\
					      __LINE__);		\
			ep0_state = s;					\
		}							\
	} else {							\
		if (debug_level > 0)					\
			serial_printf("Error at %s %d with setting "	\
				      "state %d is invalid\n",		\
				      __PRETTY_FUNCTION__, __LINE__, s); \
	}								\
} while (0)

/* static implies these initialized to 0 or NULL */
static int debug_setup;
static int debug_level;
static struct musb_epinfo epinfo[MAX_ENDPOINT * 2 + 2];
static enum ep0_state_enum {
	IDLE = 0,
	TX,
	RX,
	SET_ADDRESS
} ep0_state = IDLE;
static char *ep0_state_strings[4] = {
	"IDLE",
	"TX",
	"RX",
	"SET_ADDRESS",
};

static struct urb *ep0_urb;
struct usb_endpoint_instance *ep0_endpoint;
static struct usb_device_instance *udc_device;
static int enabled;

static u16 pending_intrrx;

#ifdef MUSB_DEBUG
static void musb_db_regs(void)
{
	u8 b;
	u16 w;

	b = readb(&musbr->faddr);
	serial_printf("\tfaddr   0x%2.2x\n", b);

	b = readb(&musbr->power);
	musb_print_pwr(b);

	w = readw(&musbr->ep[0].ep0.csr0);
	musb_print_csr0(w);

	b = readb(&musbr->devctl);
	musb_print_devctl(b);

	b = readb(&musbr->ep[0].ep0.configdata);
	musb_print_config(b);

	w = readw(&musbr->frame);
	serial_printf("\tframe   0x%4.4x\n", w);

	b = readb(&musbr->index);
	serial_printf("\tindex   0x%2.2x\n", b);

	w = readw(&musbr->ep[1].epN.rxmaxp);
	musb_print_rxmaxp(w);

	w = readw(&musbr->ep[1].epN.rxcsr);
	musb_print_rxcsr(w);

	w = readw(&musbr->ep[1].epN.txmaxp);
	musb_print_txmaxp(w);

	w = readw(&musbr->ep[1].epN.txcsr);
	musb_print_txcsr(w);
}
#else
#define musb_db_regs()
#endif /* DEBUG_MUSB */

static void musb_peri_softconnect(void)
{
	u8 power, devctl;

	/* Power off MUSB */
	power = readb(&musbr->power);
	power &= ~MUSB_POWER_SOFTCONN;
	writeb(power, &musbr->power);

	/* Read intr to clear */
	readb(&musbr->intrusb);
	readw(&musbr->intrrx);
	readw(&musbr->intrtx);

	udelay(1000 * 1000); /* 1 sec */

	/* Power on MUSB */
	power = readb(&musbr->power);
	power |= MUSB_POWER_SOFTCONN;
	/*
	 * The usb device interface is usb 1.1
	 * Disable 2.0 high speed by clearring the hsenable bit.
	 */
	power &= ~MUSB_POWER_HSENAB;
	writeb(power, &musbr->power);

	/* Check if device is in b-peripheral mode */
	devctl = readb(&musbr->devctl);
	if (!(devctl & MUSB_DEVCTL_BDEVICE) ||
	    (devctl & MUSB_DEVCTL_HM)) {
		serial_printf("ERROR : Unsupport USB mode\n");
		serial_printf("Check that mini-B USB cable is attached "
			      "to the device\n");
	}

	if (debug_setup && (debug_level > 1))
		musb_db_regs();
}

static void musb_peri_reset(void)
{
	if ((debug_setup) && (debug_level > 1))
		serial_printf("INFO : %s reset\n", __PRETTY_FUNCTION__);

	if (ep0_endpoint)
		ep0_endpoint->endpoint_address = 0xff;

	/* Sync sw and hw addresses */
	writeb(udc_device->address, &musbr->faddr);

	SET_EP0_STATE(IDLE);
}

static void musb_peri_resume(void)
{
	/* noop */
}

static void musb_peri_ep0_stall(void)
{
	u16 csr0;

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	csr0 |= MUSB_CSR0_P_SENDSTALL;
	writew(csr0, &musbr->ep[0].ep0.csr0);
	if ((debug_setup) && (debug_level > 1))
		serial_printf("INFO : %s stall\n", __PRETTY_FUNCTION__);
}

static void musb_peri_ep0_ack_req(void)
{
	u16 csr0;

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	csr0 |= MUSB_CSR0_P_SVDRXPKTRDY;
	writew(csr0, &musbr->ep[0].ep0.csr0);
}

static void musb_ep0_tx_ready(void)
{
	u16 csr0;

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	csr0 |= MUSB_CSR0_TXPKTRDY;
	writew(csr0, &musbr->ep[0].ep0.csr0);
}

static void musb_ep0_tx_ready_and_last(void)
{
	u16 csr0;

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	csr0 |= (MUSB_CSR0_TXPKTRDY | MUSB_CSR0_P_DATAEND);
	writew(csr0, &musbr->ep[0].ep0.csr0);
}

static void musb_peri_ep0_last(void)
{
	u16 csr0;

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	csr0 |= MUSB_CSR0_P_DATAEND;
	writew(csr0, &musbr->ep[0].ep0.csr0);
}

static void musb_peri_ep0_set_address(void)
{
	u8 faddr;
	writeb(udc_device->address, &musbr->faddr);

	/* Verify */
	faddr = readb(&musbr->faddr);
	if (udc_device->address == faddr) {
		SET_EP0_STATE(IDLE);
		usbd_device_event_irq(udc_device, DEVICE_ADDRESS_ASSIGNED, 0);
		if ((debug_setup) && (debug_level > 1))
			serial_printf("INFO : %s Address set to %d\n",
				      __PRETTY_FUNCTION__, udc_device->address);
	} else {
		if (debug_level > 0)
			serial_printf("ERROR : %s Address mismatch "
				      "sw %d vs hw %d\n",
				      __PRETTY_FUNCTION__,
				      udc_device->address, faddr);
	}
}

static void musb_peri_rx_ack(unsigned int ep)
{
	u16 peri_rxcsr;

	peri_rxcsr = readw(&musbr->ep[ep].epN.rxcsr);
	peri_rxcsr &= ~MUSB_RXCSR_RXPKTRDY;
	writew(peri_rxcsr, &musbr->ep[ep].epN.rxcsr);
}

static void musb_peri_tx_ready(unsigned int ep)
{
	u16 peri_txcsr;

	peri_txcsr = readw(&musbr->ep[ep].epN.txcsr);
	peri_txcsr |= MUSB_TXCSR_TXPKTRDY;
	writew(peri_txcsr, &musbr->ep[ep].epN.txcsr);
}

static void musb_peri_ep0_zero_data_request(int err)
{
	musb_peri_ep0_ack_req();

	if (err) {
		musb_peri_ep0_stall();
		SET_EP0_STATE(IDLE);
	} else {

		musb_peri_ep0_last();

		/* USBD state */
		switch (ep0_urb->device_request.bRequest) {
		case USB_REQ_SET_ADDRESS:
			if ((debug_setup) && (debug_level > 1))
				serial_printf("INFO : %s received set "
					      "address\n", __PRETTY_FUNCTION__);
			break;

		case USB_REQ_SET_CONFIGURATION:
			if ((debug_setup) && (debug_level > 1))
				serial_printf("INFO : %s Configured\n",
					      __PRETTY_FUNCTION__);
			usbd_device_event_irq(udc_device, DEVICE_CONFIGURED, 0);
			break;
		}

		/* EP0 state */
		if (USB_REQ_SET_ADDRESS == ep0_urb->device_request.bRequest) {
			SET_EP0_STATE(SET_ADDRESS);
		} else {
			SET_EP0_STATE(IDLE);
		}
	}
}

static void musb_peri_ep0_rx_data_request(void)
{
	/*
	 * This is the completion of the data OUT / RX
	 *
	 * Host is sending data to ep0 that is not
	 * part of setup.  This comes from the cdc_recv_setup
	 * op that is device specific.
	 *
	 */
	musb_peri_ep0_ack_req();

	ep0_endpoint->rcv_urb = ep0_urb;
	ep0_urb->actual_length = 0;
	SET_EP0_STATE(RX);
}

static void musb_peri_ep0_tx_data_request(int err)
{
	if (err) {
		musb_peri_ep0_stall();
		SET_EP0_STATE(IDLE);
	} else {
		musb_peri_ep0_ack_req();

		ep0_endpoint->tx_urb = ep0_urb;
		ep0_endpoint->sent = 0;
		SET_EP0_STATE(TX);
	}
}

static void musb_peri_ep0_idle(void)
{
	u16 count0;
	int err;
	u16 csr0;

	/*
	 * Verify addresses
	 * A lot of confusion can be caused if the address
	 * in software, udc layer, does not agree with the
	 * hardware.  Since the setting of the hardware address
	 * must be set after the set address request, the
	 * usb state machine is out of sync for a few frame.
	 * It is a good idea to run this check when changes
	 * are made to the state machine.
	 */
	if ((debug_level > 0) &&
	    (ep0_state != SET_ADDRESS)) {
		u8 faddr;

		faddr = readb(&musbr->faddr);
		if (udc_device->address != faddr) {
			serial_printf("ERROR : %s addresses do not"
				      "match sw %d vs hw %d\n",
				      __PRETTY_FUNCTION__,
				      udc_device->address, faddr);
			udelay(1000 * 1000);
			hang();
		}
	}

	csr0 = readw(&musbr->ep[0].ep0.csr0);

	if (!(MUSB_CSR0_RXPKTRDY & csr0))
		goto end;

	count0 = readw(&musbr->ep[0].ep0.count0);
	if (count0 == 0)
		goto end;

	if (count0 != 8) {
		if ((debug_setup) && (debug_level > 1))
			serial_printf("WARN : %s SETUP incorrect size %d\n",
				      __PRETTY_FUNCTION__, count0);
		musb_peri_ep0_stall();
		goto end;
	}

	read_fifo(0, count0, &ep0_urb->device_request);

	if (debug_level > 2)
		print_usb_device_request(&ep0_urb->device_request);

	if (ep0_urb->device_request.wLength == 0) {
		err = ep0_recv_setup(ep0_urb);

		/* Zero data request */
		musb_peri_ep0_zero_data_request(err);
	} else {
		/* Is data coming or going ? */
		u8 reqType = ep0_urb->device_request.bmRequestType;

		if (USB_REQ_DEVICE2HOST == (reqType & USB_REQ_DIRECTION_MASK)) {
			err = ep0_recv_setup(ep0_urb);
			/* Device to host */
			musb_peri_ep0_tx_data_request(err);
		} else {
			/*
			 * Host to device
			 *
			 * The RX routine will call ep0_recv_setup
			 * when the data packet has arrived.
			 */
			musb_peri_ep0_rx_data_request();
		}
	}

end:
	return;
}

static void musb_peri_ep0_rx(void)
{
	/*
	 * This is the completion of the data OUT / RX
	 *
	 * Host is sending data to ep0 that is not
	 * part of setup.  This comes from the cdc_recv_setup
	 * op that is device specific.
	 *
	 * Pass the data back to driver ep0_recv_setup which
	 * should give the cdc_recv_setup the chance to handle
	 * the rx
	 */
	u16 csr0;
	u16 count0;

	if (debug_level > 3) {
		if (0 != ep0_urb->actual_length) {
			serial_printf("%s finished ? %d of %d\n",
				      __PRETTY_FUNCTION__,
				      ep0_urb->actual_length,
				      ep0_urb->device_request.wLength);
		}
	}

	if (ep0_urb->device_request.wLength == ep0_urb->actual_length) {
		musb_peri_ep0_last();
		SET_EP0_STATE(IDLE);
		ep0_recv_setup(ep0_urb);
		return;
	}

	csr0 = readw(&musbr->ep[0].ep0.csr0);
	if (!(MUSB_CSR0_RXPKTRDY & csr0))
		return;

	count0 = readw(&musbr->ep[0].ep0.count0);

	if (count0) {
		struct usb_endpoint_instance *endpoint;
		u32 length;
		u8 *data;

		endpoint = ep0_endpoint;
		if (endpoint && endpoint->rcv_urb) {
			struct urb *urb = endpoint->rcv_urb;
			unsigned int remaining_space = urb->buffer_length -
				urb->actual_length;

			if (remaining_space) {
				int urb_bad = 0; /* urb is good */

				if (count0 > remaining_space)
					length = remaining_space;
				else
					length = count0;

				data = (u8 *) urb->buffer_data;
				data += urb->actual_length;

				/* The common musb fifo reader */
				read_fifo(0, length, data);

				musb_peri_ep0_ack_req();

				/*
				 * urb's actual_length is updated in
				 * usbd_rcv_complete
				 */
				usbd_rcv_complete(endpoint, length, urb_bad);

			} else {
				if (debug_level > 0)
					serial_printf("ERROR : %s no space in "
						      "rcv buffer\n",
						      __PRETTY_FUNCTION__);
			}
		} else {
			if (debug_level > 0)
				serial_printf("ERROR : %s problem with "
					      "endpoint\n",
					      __PRETTY_FUNCTION__);
		}
	} else {
		if (debug_level > 0)
			serial_printf("ERROR : %s with nothing to do\n",
				      __PRETTY_FUNCTION__);
	}
}

static void musb_peri_ep0_tx(void)
{
	u16 csr0;
	int transfer_size = 0;
	unsigned int p, pm;

	csr0 = readw(&musbr->ep[0].ep0.csr0);

	/* Check for pending tx */
	if (csr0 & MUSB_CSR0_TXPKTRDY)
		goto end;

	/* Check if this is the last packet sent */
	if (ep0_endpoint->sent >= ep0_urb->actual_length) {
		SET_EP0_STATE(IDLE);
		goto end;
	}

	transfer_size = ep0_urb->actual_length - ep0_endpoint->sent;
	/* Is the transfer size negative ? */
	if (transfer_size <= 0) {
		if (debug_level > 0)
			serial_printf("ERROR : %s problem with the"
				      " transfer size %d\n",
				      __PRETTY_FUNCTION__,
				      transfer_size);
		SET_EP0_STATE(IDLE);
		goto end;
	}

	/* Truncate large transfers to the fifo size */
	if (transfer_size > ep0_endpoint->tx_packetSize)
		transfer_size = ep0_endpoint->tx_packetSize;

	write_fifo(0, transfer_size, &ep0_urb->buffer[ep0_endpoint->sent]);
	ep0_endpoint->sent += transfer_size;

	/* Done or more to send ? */
	if (ep0_endpoint->sent >= ep0_urb->actual_length)
		musb_ep0_tx_ready_and_last();
	else
		musb_ep0_tx_ready();

	/* Wait a bit */
	pm = 10;
	for (p = 0; p < pm; p++) {
		csr0 = readw(&musbr->ep[0].ep0.csr0);
		if (!(csr0 & MUSB_CSR0_TXPKTRDY))
			break;

		/* Double the delay. */
		udelay(1 << pm);
	}

	if ((ep0_endpoint->sent >= ep0_urb->actual_length) && (p < pm))
		SET_EP0_STATE(IDLE);

end:
	return;
}

static void musb_peri_ep0(void)
{
	u16 csr0;

	if (SET_ADDRESS == ep0_state)
		return;

	csr0 = readw(&musbr->ep[0].ep0.csr0);

	/* Error conditions */
	if (MUSB_CSR0_P_SENTSTALL & csr0) {
		csr0 &= ~MUSB_CSR0_P_SENTSTALL;
		writew(csr0, &musbr->ep[0].ep0.csr0);
		SET_EP0_STATE(IDLE);
	}
	if (MUSB_CSR0_P_SETUPEND & csr0) {
		csr0 |= MUSB_CSR0_P_SVDSETUPEND;
		writew(csr0, &musbr->ep[0].ep0.csr0);
		SET_EP0_STATE(IDLE);
		if ((debug_setup) && (debug_level > 1))
			serial_printf("WARN: %s SETUPEND\n",
				      __PRETTY_FUNCTION__);
	}

	/* Normal states */
	if (IDLE == ep0_state)
		musb_peri_ep0_idle();

	if (TX == ep0_state)
		musb_peri_ep0_tx();

	if (RX == ep0_state)
		musb_peri_ep0_rx();
}

static void musb_peri_rx_ep(unsigned int ep)
{
	u16 peri_rxcount;
	u16 peri_rxcsr = readw(&musbr->ep[ep].epN.rxcsr);

	if (!(peri_rxcsr & MUSB_RXCSR_RXPKTRDY)) {
		if (debug_level > 0)
			serial_printf("ERROR : %s %d without MUSB_RXCSR_RXPKTRDY set\n",
				      __PRETTY_FUNCTION__, ep);
		return;
	}

	peri_rxcount = readw(&musbr->ep[ep].epN.rxcount);
	if (peri_rxcount) {
		struct usb_endpoint_instance *endpoint;
		u32 length;
		u8 *data;

		endpoint = GET_ENDPOINT(udc_device, ep);
		if (endpoint && endpoint->rcv_urb) {
			struct urb *urb = endpoint->rcv_urb;
			unsigned int remaining_space = urb->buffer_length -
				urb->actual_length;

			if (remaining_space) {
				int urb_bad = 0; /* urb is good */

				if (peri_rxcount > remaining_space)
					length = remaining_space;
				else
					length = peri_rxcount;

				data = (u8 *) urb->buffer_data;
				data += urb->actual_length;

				/* The common musb fifo reader */
				read_fifo(ep, length, data);

				if (length == peri_rxcount)
					musb_peri_rx_ack(ep);
				else
					pending_intrrx |= (1 << ep);

				/*
				 * urb's actual_length is updated in
				 * usbd_rcv_complete
				 */
				usbd_rcv_complete(endpoint, length, urb_bad);

			} else {
				if (debug_level > 0)
					serial_printf("ERROR : %s %d no space "
						      "in rcv buffer\n",
						      __PRETTY_FUNCTION__, ep);

				pending_intrrx |= (1 << ep);
			}
		} else {
			if (debug_level > 0)
				serial_printf("ERROR : %s %d problem with "
					      "endpoint\n",
					      __PRETTY_FUNCTION__, ep);

			pending_intrrx |= (1 << ep);
		}

	} else {
		if (debug_level > 0)
			serial_printf("ERROR : %s %d with nothing to do\n",
				      __PRETTY_FUNCTION__, ep);

		musb_peri_rx_ack(ep);
	}
}

static void musb_peri_rx(u16 intr)
{
	unsigned int ep;

	/* First bit is reserved and does not indicate interrupt for EP0 */

	for (ep = 1; ep < 16; ep++) {
		if ((1 << ep) & intr)
			musb_peri_rx_ep(ep);
	}
}

static void musb_peri_tx(u16 intr)
{
	unsigned int ep;

	/* Check for EP0: first bit indicates interrupt for both RX and TX */
	if (0x01 & intr)
		musb_peri_ep0();

	for (ep = 1; ep < 16; ep++) {
		if ((1 << ep) & intr)
			udc_endpoint_write(GET_ENDPOINT(udc_device, ep));
	}
}

void udc_irq(void)
{
	/* This is a high freq called function */
	if (enabled) {
		u8 intrusb;

		intrusb = readb(&musbr->intrusb);

		/*
		 * See drivers/usb/gadget/mpc8xx_udc.c for
		 * state diagram going from detached through
		 * configuration.
		 */
		if (MUSB_INTR_RESUME & intrusb) {
			usbd_device_event_irq(udc_device,
					      DEVICE_BUS_ACTIVITY, 0);
			musb_peri_resume();
		}

		if (MUSB_INTR_RESET & intrusb) {
			usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
			musb_peri_reset();
		}

		if (MUSB_INTR_DISCONNECT & intrusb) {
			/* cable unplugged from hub/host */
			usbd_device_event_irq(udc_device, DEVICE_RESET, 0);
			musb_peri_reset();
			usbd_device_event_irq(udc_device, DEVICE_HUB_RESET, 0);
		}

		if (MUSB_INTR_SOF & intrusb) {
			usbd_device_event_irq(udc_device,
					      DEVICE_BUS_ACTIVITY, 0);
			musb_peri_resume();
		}

		if (MUSB_INTR_SUSPEND & intrusb) {
			usbd_device_event_irq(udc_device,
					      DEVICE_BUS_INACTIVE, 0);
		}

		if (ep0_state != SET_ADDRESS) {
			u16 intrrx, intrtx;

			intrrx = readw(&musbr->intrrx);
			intrtx = readw(&musbr->intrtx);

			intrrx |= pending_intrrx;
			pending_intrrx = 0;

			if (intrrx)
				musb_peri_rx(intrrx);

			if (intrtx)
				musb_peri_tx(intrtx);
		} else {
			if (readw(&musbr->intrtx) & 0x1) {
				u8 faddr;
				faddr = readb(&musbr->faddr);
				/*
				 * Setting of the address can fail.
				 * Normally it succeeds the second time.
				 */
				if (udc_device->address != faddr)
					musb_peri_ep0_set_address();
			}
		}
	}
}

void udc_set_nak(int ep_num)
{
	/* noop */
}

void udc_unset_nak(int ep_num)
{
	/* noop */
}

int udc_endpoint_write(struct usb_endpoint_instance *endpoint)
{
	int ret = 0;

	/* Transmit only if the hardware is available */
	if (endpoint->tx_urb && endpoint->state == 0) {
		unsigned int ep = endpoint->endpoint_address &
			USB_ENDPOINT_NUMBER_MASK;

		u16 peri_txcsr = readw(&musbr->ep[ep].epN.txcsr);

		/* Error conditions */
		if (peri_txcsr & MUSB_TXCSR_P_UNDERRUN) {
			peri_txcsr &= ~MUSB_TXCSR_P_UNDERRUN;
			writew(peri_txcsr, &musbr->ep[ep].epN.txcsr);
		}

		if (debug_level > 1)
			musb_print_txcsr(peri_txcsr);

		/* Check if a packet is waiting to be sent */
		if (!(peri_txcsr & MUSB_TXCSR_TXPKTRDY)) {
			u32 length;
			u8 *data;
			struct urb *urb = endpoint->tx_urb;
			unsigned int remaining_packet = urb->actual_length -
				endpoint->sent;

			if (endpoint->tx_packetSize < remaining_packet)
				length = endpoint->tx_packetSize;
			else
				length = remaining_packet;

			data = (u8 *) urb->buffer;
			data += endpoint->sent;

			/* common musb fifo function */
			write_fifo(ep, length, data);

			musb_peri_tx_ready(ep);

			endpoint->last = length;
			/* usbd_tx_complete will take care of updating 'sent' */
			usbd_tx_complete(endpoint);
		}
	} else {
		if (debug_level > 0)
			serial_printf("ERROR : %s Problem with urb %p "
				      "or ep state %d\n",
				      __PRETTY_FUNCTION__,
				      endpoint->tx_urb, endpoint->state);
	}

	return ret;
}

void udc_setup_ep(struct usb_device_instance *device, unsigned int id,
		  struct usb_endpoint_instance *endpoint)
{
	if (0 == id) {
		/* EP0 */
		ep0_endpoint = endpoint;
		ep0_endpoint->endpoint_address = 0xff;
		ep0_urb = usbd_alloc_urb(device, endpoint);
	} else if (MAX_ENDPOINT >= id) {
		epinfo[(id * 2) + 0].epsize = endpoint->rcv_packetSize;
		epinfo[(id * 2) + 1].epsize = endpoint->tx_packetSize;
		musb_configure_ep(&epinfo[0], ARRAY_SIZE(epinfo));
	} else {
		if (debug_level > 0)
			serial_printf("ERROR : %s endpoint request %d "
				      "exceeds maximum %d\n",
				      __PRETTY_FUNCTION__, id, MAX_ENDPOINT);
	}
}

void udc_connect(void)
{
	/* noop */
}

void udc_disconnect(void)
{
	/* noop */
}

void udc_enable(struct usb_device_instance *device)
{
	/* Save the device structure pointer */
	udc_device = device;

	enabled = 1;
}

void udc_disable(void)
{
	enabled = 0;
}

void udc_startup_events(struct usb_device_instance *device)
{
	/* The DEVICE_INIT event puts the USB device in the state STATE_INIT. */
	usbd_device_event_irq(device, DEVICE_INIT, 0);

	/*
	 * The DEVICE_CREATE event puts the USB device in the state
	 * STATE_ATTACHED.
	 */
	usbd_device_event_irq(device, DEVICE_CREATE, 0);

	/* Resets the address to 0 */
	usbd_device_event_irq(device, DEVICE_RESET, 0);

	udc_enable(device);
}

int udc_init(void)
{
	int ret;
	int ep_loop;

	ret = musb_platform_init();
	if (ret < 0)
		goto end;

	/* Configure all the endpoint FIFO's and start usb controller */
	musbr = musb_cfg.regs;

	/* Initialize the endpoints */
	for (ep_loop = 0; ep_loop <= MAX_ENDPOINT * 2; ep_loop++) {
		epinfo[ep_loop].epnum = (ep_loop / 2) + 1;
		epinfo[ep_loop].epdir = ep_loop % 2; /* OUT, IN */
		epinfo[ep_loop].epsize = 0;
	}

	musb_peri_softconnect();

	ret = 0;
end:

	return ret;
}
