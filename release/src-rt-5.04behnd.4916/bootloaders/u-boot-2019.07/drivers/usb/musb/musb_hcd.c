// SPDX-License-Identifier: GPL-2.0+
/*
 * Mentor USB OTG Core host controller driver.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#include <common.h>
#include <usb.h>
#include "musb_hcd.h"

/* MSC control transfers */
#define USB_MSC_BBB_RESET 	0xFF
#define USB_MSC_BBB_GET_MAX_LUN	0xFE

/* Endpoint configuration information */
static const struct musb_epinfo epinfo[3] = {
	{MUSB_BULK_EP, 1, 512}, /* EP1 - Bluk Out - 512 Bytes */
	{MUSB_BULK_EP, 0, 512}, /* EP1 - Bluk In  - 512 Bytes */
	{MUSB_INTR_EP, 0, 64}   /* EP2 - Interrupt IN - 64 Bytes */
};

/* --- Virtual Root Hub ---------------------------------------------------- */
#ifdef MUSB_NO_MULTIPOINT
static int rh_devnum;
static u32 port_status;

#include <usbroothubdes.h>

#endif

/*
 * This function writes the data toggle value.
 */
static void write_toggle(struct usb_device *dev, u8 ep, u8 dir_out)
{
	u16 toggle = usb_gettoggle(dev, ep, dir_out);
	u16 csr;

	if (dir_out) {
		csr = readw(&musbr->txcsr);
		if (!toggle) {
			if (csr & MUSB_TXCSR_MODE)
				csr = MUSB_TXCSR_CLRDATATOG;
			else
				csr = 0;
			writew(csr, &musbr->txcsr);
		} else {
			csr |= MUSB_TXCSR_H_WR_DATATOGGLE;
			writew(csr, &musbr->txcsr);
			csr |= (toggle << MUSB_TXCSR_H_DATATOGGLE_SHIFT);
			writew(csr, &musbr->txcsr);
		}
	} else {
		if (!toggle) {
			csr = readw(&musbr->txcsr);
			if (csr & MUSB_TXCSR_MODE)
				csr = MUSB_RXCSR_CLRDATATOG;
			else
				csr = 0;
			writew(csr, &musbr->rxcsr);
		} else {
			csr = readw(&musbr->rxcsr);
			csr |= MUSB_RXCSR_H_WR_DATATOGGLE;
			writew(csr, &musbr->rxcsr);
			csr |= (toggle << MUSB_S_RXCSR_H_DATATOGGLE);
			writew(csr, &musbr->rxcsr);
		}
	}
}

/*
 * This function checks if RxStall has occurred on the endpoint. If a RxStall
 * has occurred, the RxStall is cleared and 1 is returned. If RxStall has
 * not occurred, 0 is returned.
 */
static u8 check_stall(u8 ep, u8 dir_out)
{
	u16 csr;

	/* For endpoint 0 */
	if (!ep) {
		csr = readw(&musbr->txcsr);
		if (csr & MUSB_CSR0_H_RXSTALL) {
			csr &= ~MUSB_CSR0_H_RXSTALL;
			writew(csr, &musbr->txcsr);
			return 1;
		}
	} else { /* For non-ep0 */
		if (dir_out) { /* is it tx ep */
			csr = readw(&musbr->txcsr);
			if (csr & MUSB_TXCSR_H_RXSTALL) {
				csr &= ~MUSB_TXCSR_H_RXSTALL;
				writew(csr, &musbr->txcsr);
				return 1;
			}
		} else { /* is it rx ep */
			csr = readw(&musbr->rxcsr);
			if (csr & MUSB_RXCSR_H_RXSTALL) {
				csr &= ~MUSB_RXCSR_H_RXSTALL;
				writew(csr, &musbr->rxcsr);
				return 1;
			}
		}
	}
	return 0;
}

/*
 * waits until ep0 is ready. Returns 0 if ep is ready, -1 for timeout
 * error and -2 for stall.
 */
static int wait_until_ep0_ready(struct usb_device *dev, u32 bit_mask)
{
	u16 csr;
	int result = 1;
	int timeout = CONFIG_USB_MUSB_TIMEOUT;

	while (result > 0) {
		csr = readw(&musbr->txcsr);
		if (csr & MUSB_CSR0_H_ERROR) {
			csr &= ~MUSB_CSR0_H_ERROR;
			writew(csr, &musbr->txcsr);
			dev->status = USB_ST_CRC_ERR;
			result = -1;
			break;
		}

		switch (bit_mask) {
		case MUSB_CSR0_TXPKTRDY:
			if (!(csr & MUSB_CSR0_TXPKTRDY)) {
				if (check_stall(MUSB_CONTROL_EP, 0)) {
					dev->status = USB_ST_STALLED;
					result = -2;
				} else
					result = 0;
			}
			break;

		case MUSB_CSR0_RXPKTRDY:
			if (check_stall(MUSB_CONTROL_EP, 0)) {
				dev->status = USB_ST_STALLED;
				result = -2;
			} else
				if (csr & MUSB_CSR0_RXPKTRDY)
					result = 0;
			break;

		case MUSB_CSR0_H_REQPKT:
			if (!(csr & MUSB_CSR0_H_REQPKT)) {
				if (check_stall(MUSB_CONTROL_EP, 0)) {
					dev->status = USB_ST_STALLED;
					result = -2;
				} else
					result = 0;
			}
			break;
		}

		/* Check the timeout */
		if (--timeout)
			udelay(1);
		else {
			dev->status = USB_ST_CRC_ERR;
			result = -1;
			break;
		}
	}

	return result;
}

/*
 * waits until tx ep is ready. Returns 1 when ep is ready and 0 on error.
 */
static int wait_until_txep_ready(struct usb_device *dev, u8 ep)
{
	u16 csr;
	int timeout = CONFIG_USB_MUSB_TIMEOUT;

	do {
		if (check_stall(ep, 1)) {
			dev->status = USB_ST_STALLED;
			return 0;
		}

		csr = readw(&musbr->txcsr);
		if (csr & MUSB_TXCSR_H_ERROR) {
			dev->status = USB_ST_CRC_ERR;
			return 0;
		}

		/* Check the timeout */
		if (--timeout)
			udelay(1);
		else {
			dev->status = USB_ST_CRC_ERR;
			return -1;
		}

	} while (csr & MUSB_TXCSR_TXPKTRDY);
	return 1;
}

/*
 * waits until rx ep is ready. Returns 1 when ep is ready and 0 on error.
 */
static int wait_until_rxep_ready(struct usb_device *dev, u8 ep)
{
	u16 csr;
	int timeout = CONFIG_USB_MUSB_TIMEOUT;

	do {
		if (check_stall(ep, 0)) {
			dev->status = USB_ST_STALLED;
			return 0;
		}

		csr = readw(&musbr->rxcsr);
		if (csr & MUSB_RXCSR_H_ERROR) {
			dev->status = USB_ST_CRC_ERR;
			return 0;
		}

		/* Check the timeout */
		if (--timeout)
			udelay(1);
		else {
			dev->status = USB_ST_CRC_ERR;
			return -1;
		}

	} while (!(csr & MUSB_RXCSR_RXPKTRDY));
	return 1;
}

/*
 * This function performs the setup phase of the control transfer
 */
static int ctrlreq_setup_phase(struct usb_device *dev, struct devrequest *setup)
{
	int result;
	u16 csr;

	/* write the control request to ep0 fifo */
	write_fifo(MUSB_CONTROL_EP, sizeof(struct devrequest), (void *)setup);

	/* enable transfer of setup packet */
	csr = readw(&musbr->txcsr);
	csr |= (MUSB_CSR0_TXPKTRDY|MUSB_CSR0_H_SETUPPKT);
	writew(csr, &musbr->txcsr);

	/* wait until the setup packet is transmitted */
	result = wait_until_ep0_ready(dev, MUSB_CSR0_TXPKTRDY);
	dev->act_len = 0;
	return result;
}

/*
 * This function handles the control transfer in data phase
 */
static int ctrlreq_in_data_phase(struct usb_device *dev, u32 len, void *buffer)
{
	u16 csr;
	u32 rxlen = 0;
	u32 nextlen = 0;
	u8  maxpktsize = (1 << dev->maxpacketsize) * 8;
	u8  *rxbuff = (u8 *)buffer;
	u8  rxedlength;
	int result;

	while (rxlen < len) {
		/* Determine the next read length */
		nextlen = ((len-rxlen) > maxpktsize) ? maxpktsize : (len-rxlen);

		/* Set the ReqPkt bit */
		csr = readw(&musbr->txcsr);
		writew(csr | MUSB_CSR0_H_REQPKT, &musbr->txcsr);
		result = wait_until_ep0_ready(dev, MUSB_CSR0_RXPKTRDY);
		if (result < 0)
			return result;

		/* Actual number of bytes received by usb */
		rxedlength = readb(&musbr->rxcount);

		/* Read the data from the RxFIFO */
		read_fifo(MUSB_CONTROL_EP, rxedlength, &rxbuff[rxlen]);

		/* Clear the RxPktRdy Bit */
		csr = readw(&musbr->txcsr);
		csr &= ~MUSB_CSR0_RXPKTRDY;
		writew(csr, &musbr->txcsr);

		/* short packet? */
		if (rxedlength != nextlen) {
			dev->act_len += rxedlength;
			break;
		}
		rxlen += nextlen;
		dev->act_len = rxlen;
	}
	return 0;
}

/*
 * This function handles the control transfer out data phase
 */
static int ctrlreq_out_data_phase(struct usb_device *dev, u32 len, void *buffer)
{
	u16 csr;
	u32 txlen = 0;
	u32 nextlen = 0;
	u8  maxpktsize = (1 << dev->maxpacketsize) * 8;
	u8  *txbuff = (u8 *)buffer;
	int result = 0;

	while (txlen < len) {
		/* Determine the next write length */
		nextlen = ((len-txlen) > maxpktsize) ? maxpktsize : (len-txlen);

		/* Load the data to send in FIFO */
		write_fifo(MUSB_CONTROL_EP, txlen, &txbuff[txlen]);

		/* Set TXPKTRDY bit */
		csr = readw(&musbr->txcsr);
			
		csr |= MUSB_CSR0_TXPKTRDY;
		csr |= MUSB_CSR0_H_DIS_PING;
		writew(csr, &musbr->txcsr);
		result = wait_until_ep0_ready(dev, MUSB_CSR0_TXPKTRDY);
		if (result < 0)
			break;

		txlen += nextlen;
		dev->act_len = txlen;
	}
	return result;
}

/*
 * This function handles the control transfer out status phase
 */
static int ctrlreq_out_status_phase(struct usb_device *dev)
{
	u16 csr;
	int result;

	/* Set the StatusPkt bit */
	csr = readw(&musbr->txcsr);
	csr |= (MUSB_CSR0_TXPKTRDY | MUSB_CSR0_H_STATUSPKT);
	csr |= MUSB_CSR0_H_DIS_PING;
	writew(csr, &musbr->txcsr);

	/* Wait until TXPKTRDY bit is cleared */
	result = wait_until_ep0_ready(dev, MUSB_CSR0_TXPKTRDY);
	return result;
}

/*
 * This function handles the control transfer in status phase
 */
static int ctrlreq_in_status_phase(struct usb_device *dev)
{
	u16 csr;
	int result;

	/* Set the StatusPkt bit and ReqPkt bit */
	csr = MUSB_CSR0_H_REQPKT | MUSB_CSR0_H_STATUSPKT;
	csr |= MUSB_CSR0_H_DIS_PING;
	writew(csr, &musbr->txcsr);
	result = wait_until_ep0_ready(dev, MUSB_CSR0_H_REQPKT);

	/* clear StatusPkt bit and RxPktRdy bit */
	csr = readw(&musbr->txcsr);
	csr &= ~(MUSB_CSR0_RXPKTRDY | MUSB_CSR0_H_STATUSPKT);
	writew(csr, &musbr->txcsr);
	return result;
}

/*
 * determines the speed of the device (High/Full/Slow)
 */
static u8 get_dev_speed(struct usb_device *dev)
{
	return (dev->speed == USB_SPEED_HIGH) ? MUSB_TYPE_SPEED_HIGH :
		((dev->speed == USB_SPEED_LOW) ? MUSB_TYPE_SPEED_LOW :
						MUSB_TYPE_SPEED_FULL);
}

/*
 * configure the hub address and the port address.
 */
static void config_hub_port(struct usb_device *dev, u8 ep)
{
	u8 chid;
	u8 hub;

	/* Find out the nearest parent which is high speed */
	while (dev->parent->parent != NULL)
		if (get_dev_speed(dev->parent) !=  MUSB_TYPE_SPEED_HIGH)
			dev = dev->parent;
		else
			break;

	/* determine the port address at that hub */
	hub = dev->parent->devnum;
	for (chid = 0; chid < USB_MAXCHILDREN; chid++)
		if (dev->parent->children[chid] == dev)
			break;

#ifndef MUSB_NO_MULTIPOINT
	/* configure the hub address and the port address */
	writeb(hub, &musbr->tar[ep].txhubaddr);
	writeb((chid + 1), &musbr->tar[ep].txhubport);
	writeb(hub, &musbr->tar[ep].rxhubaddr);
	writeb((chid + 1), &musbr->tar[ep].rxhubport);
#endif
}

#ifdef MUSB_NO_MULTIPOINT

static void musb_port_reset(int do_reset)
{
	u8 power = readb(&musbr->power);

	if (do_reset) {
		power &= 0xf0;
		writeb(power | MUSB_POWER_RESET, &musbr->power);
		port_status |= USB_PORT_STAT_RESET;
		port_status &= ~USB_PORT_STAT_ENABLE;
		udelay(30000);
	} else {
		writeb(power & ~MUSB_POWER_RESET, &musbr->power);

		power = readb(&musbr->power);
		if (power & MUSB_POWER_HSMODE)
			port_status |= USB_PORT_STAT_HIGH_SPEED;

		port_status &= ~(USB_PORT_STAT_RESET | (USB_PORT_STAT_C_CONNECTION << 16));
		port_status |= USB_PORT_STAT_ENABLE
			| (USB_PORT_STAT_C_RESET << 16)
			| (USB_PORT_STAT_C_ENABLE << 16);
	}
}

/*
 * root hub control
 */
static int musb_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
			      void *buffer, int transfer_len,
			      struct devrequest *cmd)
{
	int leni = transfer_len;
	int len = 0;
	int stat = 0;
	u32 datab[4];
	const u8 *data_buf = (u8 *) datab;
	u16 bmRType_bReq;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
	u16 int_usb;

	if ((pipe & PIPE_INTERRUPT) == PIPE_INTERRUPT) {
		debug("Root-Hub submit IRQ: NOT implemented\n");
		return 0;
	}

	bmRType_bReq = cmd->requesttype | (cmd->request << 8);
	wValue = swap_16(cmd->value);
	wIndex = swap_16(cmd->index);
	wLength = swap_16(cmd->length);

	debug("--- HUB ----------------------------------------\n");
	debug("submit rh urb, req=%x val=%#x index=%#x len=%d\n",
	    bmRType_bReq, wValue, wIndex, wLength);
	debug("------------------------------------------------\n");

	switch (bmRType_bReq) {
	case RH_GET_STATUS:
		debug("RH_GET_STATUS\n");

		*(__u16 *) data_buf = swap_16(1);
		len = 2;
		break;

	case RH_GET_STATUS | RH_INTERFACE:
		debug("RH_GET_STATUS | RH_INTERFACE\n");

		*(__u16 *) data_buf = swap_16(0);
		len = 2;
		break;

	case RH_GET_STATUS | RH_ENDPOINT:
		debug("RH_GET_STATUS | RH_ENDPOINT\n");

		*(__u16 *) data_buf = swap_16(0);
		len = 2;
		break;

	case RH_GET_STATUS | RH_CLASS:
		debug("RH_GET_STATUS | RH_CLASS\n");

		*(__u32 *) data_buf = swap_32(0);
		len = 4;
		break;

	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		debug("RH_GET_STATUS | RH_OTHER | RH_CLASS\n");

		int_usb = readw(&musbr->intrusb);
		if (int_usb & MUSB_INTR_CONNECT) {
			port_status |= USB_PORT_STAT_CONNECTION
				| (USB_PORT_STAT_C_CONNECTION << 16);
			port_status |= USB_PORT_STAT_HIGH_SPEED
				| USB_PORT_STAT_ENABLE;
		}

		if (port_status & USB_PORT_STAT_RESET)
			musb_port_reset(0);

		*(__u32 *) data_buf = swap_32(port_status);
		len = 4;
		break;

	case RH_CLEAR_FEATURE | RH_ENDPOINT:
		debug("RH_CLEAR_FEATURE | RH_ENDPOINT\n");

		switch (wValue) {
		case RH_ENDPOINT_STALL:
			debug("C_HUB_ENDPOINT_STALL\n");
			len = 0;
			break;
		}
		port_status &= ~(1 << wValue);
		break;

	case RH_CLEAR_FEATURE | RH_CLASS:
		debug("RH_CLEAR_FEATURE | RH_CLASS\n");

		switch (wValue) {
		case RH_C_HUB_LOCAL_POWER:
			debug("C_HUB_LOCAL_POWER\n");
			len = 0;
			break;

		case RH_C_HUB_OVER_CURRENT:
			debug("C_HUB_OVER_CURRENT\n");
			len = 0;
			break;
		}
		port_status &= ~(1 << wValue);
		break;

	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		debug("RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS\n");

		switch (wValue) {
		case RH_PORT_ENABLE:
			len = 0;
			break;

		case RH_PORT_SUSPEND:
			len = 0;
			break;

		case RH_PORT_POWER:
			len = 0;
			break;

		case RH_C_PORT_CONNECTION:
			len = 0;
			break;

		case RH_C_PORT_ENABLE:
			len = 0;
			break;

		case RH_C_PORT_SUSPEND:
			len = 0;
			break;

		case RH_C_PORT_OVER_CURRENT:
			len = 0;
			break;

		case RH_C_PORT_RESET:
			len = 0;
			break;

		default:
			debug("invalid wValue\n");
			stat = USB_ST_STALLED;
		}

		port_status &= ~(1 << wValue);
		break;

	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		debug("RH_SET_FEATURE | RH_OTHER | RH_CLASS\n");

		switch (wValue) {
		case RH_PORT_SUSPEND:
			len = 0;
			break;

		case RH_PORT_RESET:
			musb_port_reset(1);
			len = 0;
			break;

		case RH_PORT_POWER:
			len = 0;
			break;

		case RH_PORT_ENABLE:
			len = 0;
			break;

		default:
			debug("invalid wValue\n");
			stat = USB_ST_STALLED;
		}

		port_status |= 1 << wValue;
		break;

	case RH_SET_ADDRESS:
		debug("RH_SET_ADDRESS\n");

		rh_devnum = wValue;
		len = 0;
		break;

	case RH_GET_DESCRIPTOR:
		debug("RH_GET_DESCRIPTOR: %x, %d\n", wValue, wLength);

		switch (wValue) {
		case (USB_DT_DEVICE << 8):	/* device descriptor */
			len = min_t(unsigned int,
				    leni, min_t(unsigned int,
						sizeof(root_hub_dev_des),
						wLength));
			data_buf = root_hub_dev_des;
			break;

		case (USB_DT_CONFIG << 8):	/* configuration descriptor */
			len = min_t(unsigned int,
				    leni, min_t(unsigned int,
						sizeof(root_hub_config_des),
						wLength));
			data_buf = root_hub_config_des;
			break;

		case ((USB_DT_STRING << 8) | 0x00):	/* string 0 descriptors */
			len = min_t(unsigned int,
				    leni, min_t(unsigned int,
						sizeof(root_hub_str_index0),
						wLength));
			data_buf = root_hub_str_index0;
			break;

		case ((USB_DT_STRING << 8) | 0x01):	/* string 1 descriptors */
			len = min_t(unsigned int,
				    leni, min_t(unsigned int,
						sizeof(root_hub_str_index1),
						wLength));
			data_buf = root_hub_str_index1;
			break;

		default:
			debug("invalid wValue\n");
			stat = USB_ST_STALLED;
		}

		break;

	case RH_GET_DESCRIPTOR | RH_CLASS: {
		u8 *_data_buf = (u8 *) datab;
		debug("RH_GET_DESCRIPTOR | RH_CLASS\n");

		_data_buf[0] = 0x09;	/* min length; */
		_data_buf[1] = 0x29;
		_data_buf[2] = 0x1;	/* 1 port */
		_data_buf[3] = 0x01;	/* per-port power switching */
		_data_buf[3] |= 0x10;	/* no overcurrent reporting */

		/* Corresponds to data_buf[4-7] */
		_data_buf[4] = 0;
		_data_buf[5] = 5;
		_data_buf[6] = 0;
		_data_buf[7] = 0x02;
		_data_buf[8] = 0xff;

		len = min_t(unsigned int, leni,
			    min_t(unsigned int, data_buf[0], wLength));
		break;
	}

	case RH_GET_CONFIGURATION:
		debug("RH_GET_CONFIGURATION\n");

		*(__u8 *) data_buf = 0x01;
		len = 1;
		break;

	case RH_SET_CONFIGURATION:
		debug("RH_SET_CONFIGURATION\n");

		len = 0;
		break;

	default:
		debug("*** *** *** unsupported root hub command *** *** ***\n");
		stat = USB_ST_STALLED;
	}

	len = min_t(int, len, leni);
	if (buffer != data_buf)
		memcpy(buffer, data_buf, len);

	dev->act_len = len;
	dev->status = stat;
	debug("dev act_len %d, status %lu\n", dev->act_len, dev->status);

	return stat;
}

static void musb_rh_init(void)
{
	rh_devnum = 0;
	port_status = 0;
}

#else

static void musb_rh_init(void) {}

#endif

/*
 * do a control transfer
 */
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
			int len, struct devrequest *setup)
{
	int devnum = usb_pipedevice(pipe);
	u8  devspeed;

#ifdef MUSB_NO_MULTIPOINT
	/* Control message is for the HUB? */
	if (devnum == rh_devnum) {
		int stat = musb_submit_rh_msg(dev, pipe, buffer, len, setup);
		if (stat)
			return stat;
	}
#endif

	/* select control endpoint */
	writeb(MUSB_CONTROL_EP, &musbr->index);
	readw(&musbr->txcsr);

#ifndef MUSB_NO_MULTIPOINT
	/* target addr and (for multipoint) hub addr/port */
	writeb(devnum, &musbr->tar[MUSB_CONTROL_EP].txfuncaddr);
	writeb(devnum, &musbr->tar[MUSB_CONTROL_EP].rxfuncaddr);
#endif

	/* configure the hub address and the port number as required */
	devspeed = get_dev_speed(dev);
	if ((musb_ishighspeed()) && (dev->parent != NULL) &&
		(devspeed != MUSB_TYPE_SPEED_HIGH)) {
		config_hub_port(dev, MUSB_CONTROL_EP);
		writeb(devspeed << 6, &musbr->txtype);
	} else {
		writeb(musb_cfg.musb_speed << 6, &musbr->txtype);
#ifndef MUSB_NO_MULTIPOINT
		writeb(0, &musbr->tar[MUSB_CONTROL_EP].txhubaddr);
		writeb(0, &musbr->tar[MUSB_CONTROL_EP].txhubport);
		writeb(0, &musbr->tar[MUSB_CONTROL_EP].rxhubaddr);
		writeb(0, &musbr->tar[MUSB_CONTROL_EP].rxhubport);
#endif
	}

	/* Control transfer setup phase */
	if (ctrlreq_setup_phase(dev, setup) < 0)
		return 0;

	switch (setup->request) {
	case USB_REQ_GET_DESCRIPTOR:
	case USB_REQ_GET_CONFIGURATION:
	case USB_REQ_GET_INTERFACE:
	case USB_REQ_GET_STATUS:
	case USB_MSC_BBB_GET_MAX_LUN:
		/* control transfer in-data-phase */
		if (ctrlreq_in_data_phase(dev, len, buffer) < 0)
			return 0;
		/* control transfer out-status-phase */
		if (ctrlreq_out_status_phase(dev) < 0)
			return 0;
		break;

	case USB_REQ_SET_ADDRESS:
	case USB_REQ_SET_CONFIGURATION:
	case USB_REQ_SET_FEATURE:
	case USB_REQ_SET_INTERFACE:
	case USB_REQ_CLEAR_FEATURE:
	case USB_MSC_BBB_RESET:
		/* control transfer in status phase */
		if (ctrlreq_in_status_phase(dev) < 0)
			return 0;
		break;

	case USB_REQ_SET_DESCRIPTOR:
		/* control transfer out data phase */
		if (ctrlreq_out_data_phase(dev, len, buffer) < 0)
			return 0;
		/* control transfer in status phase */
		if (ctrlreq_in_status_phase(dev) < 0)
			return 0;
		break;

	default:
		/* unhandled control transfer */
		return -1;
	}

	dev->status = 0;
	dev->act_len = len;

#ifdef MUSB_NO_MULTIPOINT
	/* Set device address to USB_FADDR register */
	if (setup->request == USB_REQ_SET_ADDRESS)
		writeb(dev->devnum, &musbr->faddr);
#endif

	return len;
}

/*
 * do a bulk transfer
 */
int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
					void *buffer, int len)
{
	int dir_out = usb_pipeout(pipe);
	int ep = usb_pipeendpoint(pipe);
#ifndef MUSB_NO_MULTIPOINT
	int devnum = usb_pipedevice(pipe);
#endif
	u8  type;
	u16 csr;
	u32 txlen = 0;
	u32 nextlen = 0;
	u8  devspeed;

	/* select bulk endpoint */
	writeb(MUSB_BULK_EP, &musbr->index);

#ifndef MUSB_NO_MULTIPOINT
	/* write the address of the device */
	if (dir_out)
		writeb(devnum, &musbr->tar[MUSB_BULK_EP].txfuncaddr);
	else
		writeb(devnum, &musbr->tar[MUSB_BULK_EP].rxfuncaddr);
#endif

	/* configure the hub address and the port number as required */
	devspeed = get_dev_speed(dev);
	if ((musb_ishighspeed()) && (dev->parent != NULL) &&
		(devspeed != MUSB_TYPE_SPEED_HIGH)) {
		/*
		 * MUSB is in high speed and the destination device is full
		 * speed device. So configure the hub address and port
		 * address registers.
		 */
		config_hub_port(dev, MUSB_BULK_EP);
	} else {
#ifndef MUSB_NO_MULTIPOINT
		if (dir_out) {
			writeb(0, &musbr->tar[MUSB_BULK_EP].txhubaddr);
			writeb(0, &musbr->tar[MUSB_BULK_EP].txhubport);
		} else {
			writeb(0, &musbr->tar[MUSB_BULK_EP].rxhubaddr);
			writeb(0, &musbr->tar[MUSB_BULK_EP].rxhubport);
		}
#endif
		devspeed = musb_cfg.musb_speed;
	}

	/* Write the saved toggle bit value */
	write_toggle(dev, ep, dir_out);

	if (dir_out) { /* bulk-out transfer */
		/* Program the TxType register */
		type = (devspeed << MUSB_TYPE_SPEED_SHIFT) |
			   (MUSB_TYPE_PROTO_BULK << MUSB_TYPE_PROTO_SHIFT) |
			   (ep & MUSB_TYPE_REMOTE_END);
		writeb(type, &musbr->txtype);

		/* Write maximum packet size to the TxMaxp register */
		writew(dev->epmaxpacketout[ep], &musbr->txmaxp);
		while (txlen < len) {
			nextlen = ((len-txlen) < dev->epmaxpacketout[ep]) ?
					(len-txlen) : dev->epmaxpacketout[ep];

			/* Write the data to the FIFO */
			write_fifo(MUSB_BULK_EP, nextlen,
					(void *)(((u8 *)buffer) + txlen));

			/* Set the TxPktRdy bit */
			csr = readw(&musbr->txcsr);
			writew(csr | MUSB_TXCSR_TXPKTRDY, &musbr->txcsr);

			/* Wait until the TxPktRdy bit is cleared */
			if (wait_until_txep_ready(dev, MUSB_BULK_EP) != 1) {
				readw(&musbr->txcsr);
				usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_TXCSR_H_DATATOGGLE_SHIFT) & 1);
				dev->act_len = txlen;
				return 0;
			}
			txlen += nextlen;
		}

		/* Keep a copy of the data toggle bit */
		csr = readw(&musbr->txcsr);
		usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_TXCSR_H_DATATOGGLE_SHIFT) & 1);
	} else { /* bulk-in transfer */
		/* Write the saved toggle bit value */
		write_toggle(dev, ep, dir_out);

		/* Program the RxType register */
		type = (devspeed << MUSB_TYPE_SPEED_SHIFT) |
			   (MUSB_TYPE_PROTO_BULK << MUSB_TYPE_PROTO_SHIFT) |
			   (ep & MUSB_TYPE_REMOTE_END);
		writeb(type, &musbr->rxtype);

		/* Write the maximum packet size to the RxMaxp register */
		writew(dev->epmaxpacketin[ep], &musbr->rxmaxp);
		while (txlen < len) {
			nextlen = ((len-txlen) < dev->epmaxpacketin[ep]) ?
					(len-txlen) : dev->epmaxpacketin[ep];

			/* Set the ReqPkt bit */
			csr = readw(&musbr->rxcsr);
			writew(csr | MUSB_RXCSR_H_REQPKT, &musbr->rxcsr);

			/* Wait until the RxPktRdy bit is set */
			if (wait_until_rxep_ready(dev, MUSB_BULK_EP) != 1) {
				csr = readw(&musbr->rxcsr);
				usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_S_RXCSR_H_DATATOGGLE) & 1);
				csr &= ~MUSB_RXCSR_RXPKTRDY;
				writew(csr, &musbr->rxcsr);
				dev->act_len = txlen;
				return 0;
			}

			/* Read the data from the FIFO */
			read_fifo(MUSB_BULK_EP, nextlen,
					(void *)(((u8 *)buffer) + txlen));

			/* Clear the RxPktRdy bit */
			csr =  readw(&musbr->rxcsr);
			csr &= ~MUSB_RXCSR_RXPKTRDY;
			writew(csr, &musbr->rxcsr);
			txlen += nextlen;
		}

		/* Keep a copy of the data toggle bit */
		csr = readw(&musbr->rxcsr);
		usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_S_RXCSR_H_DATATOGGLE) & 1);
	}

	/* bulk transfer is complete */
	dev->status = 0;
	dev->act_len = len;
	return 0;
}

/*
 * This function initializes the usb controller module.
 */
int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	u8  power;
	u32 timeout;

	musb_rh_init();

	if (musb_platform_init() == -1)
		return -1;

	/* Configure all the endpoint FIFO's and start usb controller */
	musbr = musb_cfg.regs;
	musb_configure_ep(&epinfo[0], ARRAY_SIZE(epinfo));
	musb_start();

	/*
	 * Wait until musb is enabled in host mode with a timeout. There
	 * should be a usb device connected.
	 */
	timeout = musb_cfg.timeout;
	while (--timeout)
		if (readb(&musbr->devctl) & MUSB_DEVCTL_HM)
			break;

	/* if musb core is not in host mode, then return */
	if (!timeout)
		return -1;

	/* start usb bus reset */
	power = readb(&musbr->power);
	writeb(power | MUSB_POWER_RESET, &musbr->power);

	/* After initiating a usb reset, wait for about 20ms to 30ms */
	udelay(30000);

	/* stop usb bus reset */
	power = readb(&musbr->power);
	power &= ~MUSB_POWER_RESET;
	writeb(power, &musbr->power);

	/* Determine if the connected device is a high/full/low speed device */
	musb_cfg.musb_speed = (readb(&musbr->power) & MUSB_POWER_HSMODE) ?
			MUSB_TYPE_SPEED_HIGH :
			((readb(&musbr->devctl) & MUSB_DEVCTL_FSDEV) ?
			MUSB_TYPE_SPEED_FULL : MUSB_TYPE_SPEED_LOW);
	return 0;
}

/*
 * This function stops the operation of the davinci usb module.
 */
int usb_lowlevel_stop(int index)
{
	/* Reset the USB module */
	musb_platform_deinit();
	writeb(0, &musbr->devctl);
	return 0;
}

/*
 * This function supports usb interrupt transfers. Currently, usb interrupt
 * transfers are not supported.
 */
int submit_int_msg(struct usb_device *dev, unsigned long pipe,
				void *buffer, int len, int interval)
{
	int dir_out = usb_pipeout(pipe);
	int ep = usb_pipeendpoint(pipe);
#ifndef MUSB_NO_MULTIPOINT
	int devnum = usb_pipedevice(pipe);
#endif
	u8  type;
	u16 csr;
	u32 txlen = 0;
	u32 nextlen = 0;
	u8  devspeed;

	/* select interrupt endpoint */
	writeb(MUSB_INTR_EP, &musbr->index);

#ifndef MUSB_NO_MULTIPOINT
	/* write the address of the device */
	if (dir_out)
		writeb(devnum, &musbr->tar[MUSB_INTR_EP].txfuncaddr);
	else
		writeb(devnum, &musbr->tar[MUSB_INTR_EP].rxfuncaddr);
#endif

	/* configure the hub address and the port number as required */
	devspeed = get_dev_speed(dev);
	if ((musb_ishighspeed()) && (dev->parent != NULL) &&
		(devspeed != MUSB_TYPE_SPEED_HIGH)) {
		/*
		 * MUSB is in high speed and the destination device is full
		 * speed device. So configure the hub address and port
		 * address registers.
		 */
		config_hub_port(dev, MUSB_INTR_EP);
	} else {
#ifndef MUSB_NO_MULTIPOINT
		if (dir_out) {
			writeb(0, &musbr->tar[MUSB_INTR_EP].txhubaddr);
			writeb(0, &musbr->tar[MUSB_INTR_EP].txhubport);
		} else {
			writeb(0, &musbr->tar[MUSB_INTR_EP].rxhubaddr);
			writeb(0, &musbr->tar[MUSB_INTR_EP].rxhubport);
		}
#endif
		devspeed = musb_cfg.musb_speed;
	}

	/* Write the saved toggle bit value */
	write_toggle(dev, ep, dir_out);

	if (!dir_out) { /* intrrupt-in transfer */
		/* Write the saved toggle bit value */
		write_toggle(dev, ep, dir_out);
		writeb(interval, &musbr->rxinterval);

		/* Program the RxType register */
		type = (devspeed << MUSB_TYPE_SPEED_SHIFT) |
			   (MUSB_TYPE_PROTO_INTR << MUSB_TYPE_PROTO_SHIFT) |
			   (ep & MUSB_TYPE_REMOTE_END);
		writeb(type, &musbr->rxtype);

		/* Write the maximum packet size to the RxMaxp register */
		writew(dev->epmaxpacketin[ep], &musbr->rxmaxp);

		while (txlen < len) {
			nextlen = ((len-txlen) < dev->epmaxpacketin[ep]) ?
					(len-txlen) : dev->epmaxpacketin[ep];

			/* Set the ReqPkt bit */
			csr = readw(&musbr->rxcsr);
			writew(csr | MUSB_RXCSR_H_REQPKT, &musbr->rxcsr);

			/* Wait until the RxPktRdy bit is set */
			if (wait_until_rxep_ready(dev, MUSB_INTR_EP) != 1) {
				csr = readw(&musbr->rxcsr);
				usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_S_RXCSR_H_DATATOGGLE) & 1);
				csr &= ~MUSB_RXCSR_RXPKTRDY;
				writew(csr, &musbr->rxcsr);
				dev->act_len = txlen;
				return 0;
			}

			/* Read the data from the FIFO */
			read_fifo(MUSB_INTR_EP, nextlen,
					(void *)(((u8 *)buffer) + txlen));

			/* Clear the RxPktRdy bit */
			csr =  readw(&musbr->rxcsr);
			csr &= ~MUSB_RXCSR_RXPKTRDY;
			writew(csr, &musbr->rxcsr);
			txlen += nextlen;
		}

		/* Keep a copy of the data toggle bit */
		csr = readw(&musbr->rxcsr);
		usb_settoggle(dev, ep, dir_out,
				(csr >> MUSB_S_RXCSR_H_DATATOGGLE) & 1);
	}

	/* interrupt transfer is complete */
	dev->irq_status = 0;
	dev->irq_act_len = len;
	dev->irq_handle(dev);
	dev->status = 0;
	dev->act_len = len;
	return 0;
}
