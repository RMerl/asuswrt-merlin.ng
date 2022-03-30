/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __USB_ETHER_H__
#define __USB_ETHER_H__

#include <net.h>

/* TODO(sjg@chromium.org): Remove @pusb_dev when all boards use CONFIG_DM_ETH */
struct ueth_data {
	/* eth info */
#ifdef CONFIG_DM_ETH
	uint8_t *rxbuf;
	int rxsize;
	int rxlen;			/* Total bytes available in rxbuf */
	int rxptr;			/* Current position in rxbuf */
#else
	struct eth_device eth_dev;	/* used with eth_register */
	/* driver private */
	void *dev_priv;
#endif
	int phy_id;			/* mii phy id */

	/* usb info */
	struct usb_device *pusb_dev;	/* this usb_device */
	unsigned char	ifnum;		/* interface number */
	unsigned char	ep_in;		/* in endpoint */
	unsigned char	ep_out;		/* out ....... */
	unsigned char	ep_int;		/* interrupt . */
	unsigned char	subclass;	/* as in overview */
	unsigned char	protocol;	/* .............. */
	unsigned char	irqinterval;	/* Intervall for IRQ Pipe */
};

#ifdef CONFIG_DM_ETH
/**
 * usb_ether_register() - register a new USB ethernet device
 *
 * This selects the correct USB interface and figures out the endpoints to use.
 *
 * @dev:	USB device
 * @ss:		Place to put USB ethernet data
 * @rxsize:	Maximum size to allocate for the receive buffer
 * @return 0 if OK, -ve on error
 */
int usb_ether_register(struct udevice *dev, struct ueth_data *ueth, int rxsize);

/**
 * usb_ether_deregister() - deregister a USB ethernet device
 *
 * @ueth:	USB Ethernet device
 * @return 0
 */
int usb_ether_deregister(struct ueth_data *ueth);

/**
 * usb_ether_receive() - recieve a packet from the bulk in endpoint
 *
 * The packet is stored in the internal buffer ready for processing.
 *
 * @ueth:	USB Ethernet device
 * @rxsize:	Maximum size to receive
 * @return 0 if a packet was received, -EAGAIN if not, -ENOSPC if @rxsize is
 * larger than the size passed ot usb_ether_register(), other -ve on error
 */
int usb_ether_receive(struct ueth_data *ueth, int rxsize);

/**
 * usb_ether_get_rx_bytes() - obtain bytes from the internal packet buffer
 *
 * This should be called repeatedly to obtain packet data until it returns 0.
 * After each packet is processed, call usb_ether_advance_rxbuf() to move to
 * the next one.
 *
 * @ueth:	USB Ethernet device
 * @ptrp:	Returns a pointer to the start of the next packet if there is
 *		one available
 * @return number of bytes available, or 0 if none
 */
int usb_ether_get_rx_bytes(struct ueth_data *ueth, uint8_t **ptrp);

/**
 * usb_ether_advance_rxbuf() - Advance to the next packet in the internal buffer
 *
 * After processing the data returned by usb_ether_get_rx_bytes(), call this
 * function to move to the next packet. You must specify the number of bytes
 * you have processed in @num_bytes.
 *
 * @ueth:	USB Ethernet device
 * @num_bytes:	Number of bytes to skip, or -1 to skip all bytes
 */
void usb_ether_advance_rxbuf(struct ueth_data *ueth, int num_bytes);
#else
/*
 * Function definitions for each USB ethernet driver go here
 * (declaration is unconditional, compilation is conditional)
 */
void asix_eth_before_probe(void);
int asix_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int asix_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
		      struct eth_device *eth);

void ax88179_eth_before_probe(void);
int ax88179_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int ax88179_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
		      struct eth_device *eth);

void mcs7830_eth_before_probe(void);
int mcs7830_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss);
int mcs7830_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			 struct eth_device *eth);

void smsc95xx_eth_before_probe(void);
int smsc95xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
			struct ueth_data *ss);
int smsc95xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			struct eth_device *eth);

void r8152_eth_before_probe(void);
int r8152_eth_probe(struct usb_device *dev, unsigned int ifnum,
		    struct ueth_data *ss);
int r8152_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
		       struct eth_device *eth);
#endif

#endif /* __USB_ETHER_H__ */
