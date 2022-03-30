// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This code is based on linux driver for sl811hs chip, source at
 * drivers/usb/host/sl811.c:
 *
 * SL811 Host Controller Interface driver for USB.
 *
 * Copyright (c) 2003/06, Courage Co., Ltd.
 *
 * Based on:
 *	1.uhci.c by Linus Torvalds, Johannes Erdfelt, Randy Dunlap,
 *	  Georg Acher, Deti Fliegl, Thomas Sailer, Roman Weissgaerber,
 *	  Adam Richter, Gregory P. Smith;
 *	2.Original SL811 driver (hc_sl811.o) by Pei Liu <pbl@cypress.com>
 *	3.Rewrited as sl811.o by Yin Aihua <yinah:couragetech.com.cn>
 */

#include <common.h>
#include <mpc8xx.h>
#include <usb.h>
#include "sl811.h"

#include "../../../board/kup/common/kup.h"

#ifdef __PPC__
# define EIEIO		__asm__ volatile ("eieio")
#else
# define EIEIO		/* nothing */
#endif

#define	 SL811_ADR (0x50000000)
#define	 SL811_DAT (0x50000001)

#ifdef SL811_DEBUG
static int debug = 9;
#endif

static int root_hub_devnum = 0;
static struct usb_port_status rh_status = { 0 };/* root hub port status */

static int sl811_rh_submit_urb(struct usb_device *usb_dev, unsigned long pipe,
			       void *data, int buf_len, struct devrequest *cmd);

static void sl811_write (__u8 index, __u8 data)
{
	*(volatile unsigned char *) (SL811_ADR) = index;
	EIEIO;
	*(volatile unsigned char *) (SL811_DAT) = data;
	EIEIO;
}

static __u8 sl811_read (__u8 index)
{
	__u8 data;

	*(volatile unsigned char *) (SL811_ADR) = index;
	EIEIO;
	data = *(volatile unsigned char *) (SL811_DAT);
	EIEIO;
	return (data);
}

/*
 * Read consecutive bytes of data from the SL811H/SL11H buffer
 */
static void inline sl811_read_buf(__u8 offset, __u8 *buf, __u8 size)
{
	*(volatile unsigned char *) (SL811_ADR) = offset;
	EIEIO;
	while (size--) {
		*buf++ = *(volatile unsigned char *) (SL811_DAT);
		EIEIO;
	}
}

/*
 * Write consecutive bytes of data to the SL811H/SL11H buffer
 */
static void inline sl811_write_buf(__u8 offset, __u8 *buf, __u8 size)
{
	*(volatile unsigned char *) (SL811_ADR) = offset;
	EIEIO;
	while (size--) {
		*(volatile unsigned char *) (SL811_DAT) = *buf++;
		EIEIO;
	}
}

int usb_init_kup4x (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int i;
	unsigned char tmp;

	memctl = &immap->im_memctl;
	memctl->memc_or7 = 0xFFFF8726;
	memctl->memc_br7 = 0x50000401;	/* start at 0x50000000 */
	/* BP 14 low = USB ON */
	immap->im_cpm.cp_pbdat &= ~(BP_USB_VCC);
	/* PB 14 nomal port */
	immap->im_cpm.cp_pbpar &= ~(BP_USB_VCC);
	/* output */
	immap->im_cpm.cp_pbdir |= (BP_USB_VCC);

	puts ("USB:   ");

	for (i = 0x10; i < 0xff; i++) {
		sl811_write(i, i);
		tmp = (sl811_read(i));
		if (tmp != i) {
			printf ("SL811 compare error index=0x%02x read=0x%02x\n", i, tmp);
			return (-1);
		}
	}
	printf ("SL811 ready\n");
	return (0);
}

/*
 * This function resets SL811HS controller and detects the speed of
 * the connecting device
 *
 * Return: 0 = no device attached; 1 = USB device attached
 */
static int sl811_hc_reset(void)
{
	int status ;

	sl811_write(SL811_CTRL2, SL811_CTL2_HOST | SL811_12M_HI);
	sl811_write(SL811_CTRL1, SL811_CTRL1_RESET);

	mdelay(20);

	/* Disable hardware SOF generation, clear all irq status. */
	sl811_write(SL811_CTRL1, 0);
	mdelay(2);
	sl811_write(SL811_INTRSTS, 0xff);
	status = sl811_read(SL811_INTRSTS);

	if (status & SL811_INTR_NOTPRESENT) {
		/* Device is not present */
		PDEBUG(0, "Device not present\n");
		rh_status.wPortStatus &= ~(USB_PORT_STAT_CONNECTION | USB_PORT_STAT_ENABLE);
		rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;
		sl811_write(SL811_INTR, SL811_INTR_INSRMV);
		return 0;
	}

	/* Send SOF to address 0, endpoint 0. */
	sl811_write(SL811_LEN_B, 0);
	sl811_write(SL811_PIDEP_B, PIDEP(USB_PID_SOF, 0));
	sl811_write(SL811_DEV_B, 0x00);
	sl811_write(SL811_SOFLOW, SL811_12M_LOW);

	if (status & SL811_INTR_SPEED_FULL) {
		/* full speed device connect directly to root hub */
		PDEBUG (0, "Full speed Device attached\n");

		sl811_write(SL811_CTRL1, SL811_CTRL1_RESET);
		mdelay(20);
		sl811_write(SL811_CTRL2, SL811_CTL2_HOST | SL811_12M_HI);
		sl811_write(SL811_CTRL1, SL811_CTRL1_SOF);

		/* start the SOF or EOP */
		sl811_write(SL811_CTRL_B, SL811_USB_CTRL_ARM);
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus &= ~USB_PORT_STAT_LOW_SPEED;
		mdelay(2);
		sl811_write(SL811_INTRSTS, 0xff);
	} else {
		/* slow speed device connect directly to root-hub */
		PDEBUG(0, "Low speed Device attached\n");

		sl811_write(SL811_CTRL1, SL811_CTRL1_RESET);
		mdelay(20);
		sl811_write(SL811_CTRL2, SL811_CTL2_HOST | SL811_CTL2_DSWAP | SL811_12M_HI);
		sl811_write(SL811_CTRL1, SL811_CTRL1_SPEED_LOW | SL811_CTRL1_SOF);

		/* start the SOF or EOP */
		sl811_write(SL811_CTRL_B, SL811_USB_CTRL_ARM);
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION | USB_PORT_STAT_LOW_SPEED;
		mdelay(2);
		sl811_write(SL811_INTRSTS, 0xff);
	}

	rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;
	sl811_write(SL811_INTR, /*SL811_INTR_INSRMV*/SL811_INTR_DONE_A);

	return 1;
}

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	root_hub_devnum = 0;
	sl811_hc_reset();
	return 0;
}

int usb_lowlevel_stop(int index)
{
	sl811_hc_reset();
	return 0;
}

static int calc_needed_buswidth(int bytes, int need_preamble)
{
	return !need_preamble ? bytes * 8 + 256 : 8 * 8 * bytes + 2048;
}

static int sl811_send_packet(struct usb_device *dev, unsigned long pipe, __u8 *buffer, int len)
{
	__u8 ctrl = SL811_USB_CTRL_ARM | SL811_USB_CTRL_ENABLE;
	__u16 status = 0;
	int err = 0, time_start = get_timer(0);
	int need_preamble = !(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
		(dev->speed == USB_SPEED_LOW);

	if (len > 239)
		return -1;

	if (usb_pipeout(pipe))
		ctrl |= SL811_USB_CTRL_DIR_OUT;
	if (usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe)))
		ctrl |= SL811_USB_CTRL_TOGGLE_1;
	if (need_preamble)
		ctrl |= SL811_USB_CTRL_PREAMBLE;

	sl811_write(SL811_INTRSTS, 0xff);

	while (err < 3) {
		sl811_write(SL811_ADDR_A, 0x10);
		sl811_write(SL811_LEN_A, len);
		if (usb_pipeout(pipe) && len)
			sl811_write_buf(0x10, buffer, len);

		if (!(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
		    sl811_read(SL811_SOFCNTDIV)*64 < calc_needed_buswidth(len, need_preamble))
			ctrl |= SL811_USB_CTRL_SOF;
		else
			ctrl &= ~SL811_USB_CTRL_SOF;

		sl811_write(SL811_CTRL_A, ctrl);
		while (!(sl811_read(SL811_INTRSTS) & SL811_INTR_DONE_A)) {
			if (5*CONFIG_SYS_HZ < get_timer(time_start)) {
				printf("USB transmit timed out\n");
				return -USB_ST_CRC_ERR;
			}
		}

		sl811_write(SL811_INTRSTS, 0xff);
		status = sl811_read(SL811_STS_A);

		if (status & SL811_USB_STS_ACK) {
			int remainder = sl811_read(SL811_CNT_A);
			if (remainder) {
				PDEBUG(0, "usb transfer remainder = %d\n", remainder);
				len -= remainder;
			}
			if (usb_pipein(pipe) && len)
				sl811_read_buf(0x10, buffer, len);
			return len;
		}

		if ((status & SL811_USB_STS_NAK) == SL811_USB_STS_NAK)
			continue;

		PDEBUG(0, "usb transfer error %#x\n", (int)status);
		err++;
	}

	err = 0;

	if (status & SL811_USB_STS_ERROR)
		err |= USB_ST_BUF_ERR;
	if (status & SL811_USB_STS_TIMEOUT)
		err |= USB_ST_CRC_ERR;
	if (status & SL811_USB_STS_STALL)
		err |= USB_ST_STALLED;

	return -err;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
	int dir_out = usb_pipeout(pipe);
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	int done = 0;

	PDEBUG(7, "dev = %ld pipe = %ld buf = %p size = %d dir_out = %d\n",
	       usb_pipedevice(pipe), usb_pipeendpoint(pipe), buffer, len, dir_out);

	dev->status = 0;

	sl811_write(SL811_DEV_A, usb_pipedevice(pipe));
	sl811_write(SL811_PIDEP_A, PIDEP(!dir_out ? USB_PID_IN : USB_PID_OUT, ep));
	while (done < len) {
		int res = sl811_send_packet(dev, pipe, (__u8*)buffer+done,
					    max > len - done ? len - done : max);
		if (res < 0) {
			dev->status = -res;
			return res;
		}

		if (!dir_out && res < max) /* short packet */
			break;

		done += res;
		usb_dotoggle(dev, ep, dir_out);
	}

	dev->act_len = done;

	return 0;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len,struct devrequest *setup)
{
	int done = 0;
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);

	dev->status = 0;

	if (devnum == root_hub_devnum)
		return sl811_rh_submit_urb(dev, pipe, buffer, len, setup);

	PDEBUG(7, "dev = %d pipe = %ld buf = %p size = %d rt = %#x req = %#x bus = %i\n",
	       devnum, ep, buffer, len, (int)setup->requesttype,
	       (int)setup->request, sl811_read(SL811_SOFCNTDIV)*64);

	sl811_write(SL811_DEV_A, devnum);
	sl811_write(SL811_PIDEP_A, PIDEP(USB_PID_SETUP, ep));
	/* setup phase */
	usb_settoggle(dev, ep, 1, 0);
	if (sl811_send_packet(dev, usb_sndctrlpipe(dev, ep),
			      (__u8*)setup, sizeof(*setup)) == sizeof(*setup)) {
		int dir_in = usb_pipein(pipe);
		int max = usb_maxpacket(dev, pipe);

		/* data phase */
		sl811_write(SL811_PIDEP_A,
			    PIDEP(dir_in ? USB_PID_IN : USB_PID_OUT, ep));
		usb_settoggle(dev, ep, usb_pipeout(pipe), 1);
		while (done < len) {
			int res = sl811_send_packet(dev, pipe, (__u8*)buffer+done,
						    max > len - done ? len - done : max);
			if (res < 0) {
				PDEBUG(0, "status data failed!\n");
				dev->status = -res;
				return 0;
			}
			done += res;
			usb_dotoggle(dev, ep, usb_pipeout(pipe));
			if (dir_in && res < max) /* short packet */
				break;
		}

		/* status phase */
		sl811_write(SL811_PIDEP_A,
			    PIDEP(!dir_in ? USB_PID_IN : USB_PID_OUT, ep));
		usb_settoggle(dev, ep, !usb_pipeout(pipe), 1);
		if (sl811_send_packet(dev,
				      !dir_in ? usb_rcvctrlpipe(dev, ep) :
				      usb_sndctrlpipe(dev, ep),
				      0, 0) < 0) {
			PDEBUG(0, "status phase failed!\n");
			dev->status = -1;
		}
	} else {
		PDEBUG(0, "setup phase failed!\n");
		dev->status = -1;
	}

	dev->act_len = done;

	return done;
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval)
{
	PDEBUG(0, "dev = %p pipe = %#lx buf = %p size = %d int = %d\n", dev, pipe,
	       buffer, len, interval);
	return -1;
}

/*
 * SL811 Virtual Root Hub
 */

/* Device descriptor */
static __u8 sl811_rh_dev_des[] =
{
	0x12,	    /*	__u8  bLength; */
	0x01,	    /*	__u8  bDescriptorType; Device */
	0x10,	    /*	__u16 bcdUSB; v1.1 */
	0x01,
	0x09,	    /*	__u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  bDeviceSubClass; */
	0x00,	    /*	__u8  bDeviceProtocol; */
	0x08,	    /*	__u8  bMaxPacketSize0; 8 Bytes */
	0x00,	    /*	__u16 idVendor; */
	0x00,
	0x00,	    /*	__u16 idProduct; */
	0x00,
	0x00,	    /*	__u16 bcdDevice; */
	0x00,
	0x00,	    /*	__u8  iManufacturer; */
	0x02,	    /*	__u8  iProduct; */
	0x01,	    /*	__u8  iSerialNumber; */
	0x01	    /*	__u8  bNumConfigurations; */
};

/* Configuration descriptor */
static __u8 sl811_rh_config_des[] =
{
	0x09,	    /*	__u8  bLength; */
	0x02,	    /*	__u8  bDescriptorType; Configuration */
	0x19,	    /*	__u16 wTotalLength; */
	0x00,
	0x01,	    /*	__u8  bNumInterfaces; */
	0x01,	    /*	__u8  bConfigurationValue; */
	0x00,	    /*	__u8  iConfiguration; */
	0x40,	    /*	__u8  bmAttributes;
		    Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup,
		    4..0: resvd */
	0x00,	    /*	__u8  MaxPower; */

	/* interface */
	0x09,	    /*	__u8  if_bLength; */
	0x04,	    /*	__u8  if_bDescriptorType; Interface */
	0x00,	    /*	__u8  if_bInterfaceNumber; */
	0x00,	    /*	__u8  if_bAlternateSetting; */
	0x01,	    /*	__u8  if_bNumEndpoints; */
	0x09,	    /*	__u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  if_bInterfaceSubClass; */
	0x00,	    /*	__u8  if_bInterfaceProtocol; */
	0x00,	    /*	__u8  if_iInterface; */

	/* endpoint */
	0x07,	    /*	__u8  ep_bLength; */
	0x05,	    /*	__u8  ep_bDescriptorType; Endpoint */
	0x81,	    /*	__u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,	    /*	__u8  ep_bmAttributes; Interrupt */
	0x08,	    /*	__u16 ep_wMaxPacketSize; */
	0x00,
	0xff	    /*	__u8  ep_bInterval; 255 ms */
};

/* root hub class descriptor*/
static __u8 sl811_rh_hub_des[] =
{
	0x09,			/*  __u8  bLength; */
	0x29,			/*  __u8  bDescriptorType; Hub-descriptor */
	0x01,			/*  __u8  bNbrPorts; */
	0x00,			/* __u16  wHubCharacteristics; */
	0x00,
	0x50,			/*  __u8  bPwrOn2pwrGood; 2ms */
	0x00,			/*  __u8  bHubContrCurrent; 0 mA */
	0xfc,			/*  __u8  DeviceRemovable; *** 7 Ports max *** */
	0xff			/*  __u8  PortPwrCtrlMask; *** 7 ports max *** */
};

/*
 * helper routine for returning string descriptors in UTF-16LE
 * input can actually be ISO-8859-1; ASCII is its 7-bit subset
 */
static int ascii2utf (char *s, u8 *utf, int utfmax)
{
	int retval;

	for (retval = 0; *s && utfmax > 1; utfmax -= 2, retval += 2) {
		*utf++ = *s++;
		*utf++ = 0;
	}
	return retval;
}

/*
 * root_hub_string is used by each host controller's root hub code,
 * so that they're identified consistently throughout the system.
 */
static int usb_root_hub_string (int id, int serial, char *type, __u8 *data, int len)
{
	char buf [30];

	/* assert (len > (2 * (sizeof (buf) + 1)));
	   assert (strlen (type) <= 8);*/

	/* language ids */
	if (id == 0) {
		*data++ = 4; *data++ = 3;	/* 4 bytes data */
		*data++ = 0; *data++ = 0;	/* some language id */
		return 4;

	/* serial number */
	} else if (id == 1) {
		sprintf (buf, "%#x", serial);

	/* product description */
	} else if (id == 2) {
		sprintf (buf, "USB %s Root Hub", type);

	/* id 3 == vendor description */

	/* unsupported IDs --> "stall" */
	} else
	    return 0;

	ascii2utf (buf, data + 2, len - 2);
	data [0] = 2 + strlen(buf) * 2;
	data [1] = 3;
	return data [0];
}

/* helper macro */
#define OK(x)	len = (x); break

/*
 * This function handles all USB request to the the virtual root hub
 */
static int sl811_rh_submit_urb(struct usb_device *usb_dev, unsigned long pipe,
			       void *data, int buf_len, struct devrequest *cmd)
{
	__u8 data_buf[16];
	__u8 *bufp = data_buf;
	int len = 0;
	int status = 0;
	__u16 bmRType_bReq;
	__u16 wValue  = le16_to_cpu (cmd->value);
	__u16 wLength = le16_to_cpu (cmd->length);
#ifdef SL811_DEBUG
	__u16 wIndex  = le16_to_cpu (cmd->index);
#endif

	if (usb_pipeint(pipe)) {
		PDEBUG(0, "interrupt transfer unimplemented!\n");
		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);

	PDEBUG(5, "submit rh urb, req = %d(%x) val = %#x index = %#x len=%d\n",
	       bmRType_bReq, bmRType_bReq, wValue, wIndex, wLength);

	/* Request Destination:
		   without flags: Device,
		   USB_RECIP_INTERFACE: interface,
		   USB_RECIP_ENDPOINT: endpoint,
		   USB_TYPE_CLASS means HUB here,
		   USB_RECIP_OTHER | USB_TYPE_CLASS  almost ever means HUB_PORT here
	*/
	switch (bmRType_bReq) {
	case RH_GET_STATUS:
		*(__u16 *)bufp = cpu_to_le16(1);
		OK(2);

	case RH_GET_STATUS | USB_RECIP_INTERFACE:
		*(__u16 *)bufp = cpu_to_le16(0);
		OK(2);

	case RH_GET_STATUS | USB_RECIP_ENDPOINT:
		*(__u16 *)bufp = cpu_to_le16(0);
		OK(2);

	case RH_GET_STATUS | USB_TYPE_CLASS:
		*(__u32 *)bufp = cpu_to_le32(0);
		OK(4);

	case RH_GET_STATUS | USB_RECIP_OTHER | USB_TYPE_CLASS:
		*(__u32 *)bufp = cpu_to_le32(rh_status.wPortChange<<16 | rh_status.wPortStatus);
		OK(4);

	case RH_CLEAR_FEATURE | USB_RECIP_ENDPOINT:
		switch (wValue) {
		case 1:
			OK(0);
		}
		break;

	case RH_CLEAR_FEATURE | USB_TYPE_CLASS:
		switch (wValue) {
		case C_HUB_LOCAL_POWER:
			OK(0);

		case C_HUB_OVER_CURRENT:
			OK(0);
		}
		break;

	case RH_CLEAR_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			rh_status.wPortStatus &= ~USB_PORT_STAT_ENABLE;
			OK(0);

		case USB_PORT_FEAT_SUSPEND:
			rh_status.wPortStatus &= ~USB_PORT_STAT_SUSPEND;
			OK(0);

		case USB_PORT_FEAT_POWER:
			rh_status.wPortStatus &= ~USB_PORT_STAT_POWER;
			OK(0);

		case USB_PORT_FEAT_C_CONNECTION:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_CONNECTION;
			OK(0);

		case USB_PORT_FEAT_C_ENABLE:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_ENABLE;
			OK(0);

		case USB_PORT_FEAT_C_SUSPEND:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_SUSPEND;
			OK(0);

		case USB_PORT_FEAT_C_OVER_CURRENT:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_OVERCURRENT;
			OK(0);

		case USB_PORT_FEAT_C_RESET:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_RESET;
			OK(0);
		}
		break;

	case RH_SET_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			rh_status.wPortStatus |= USB_PORT_STAT_SUSPEND;
			OK(0);

		case USB_PORT_FEAT_RESET:
			rh_status.wPortStatus |= USB_PORT_STAT_RESET;
			rh_status.wPortChange = 0;
			rh_status.wPortChange |= USB_PORT_STAT_C_RESET;
			rh_status.wPortStatus &= ~USB_PORT_STAT_RESET;
			rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
			OK(0);

		case USB_PORT_FEAT_POWER:
			rh_status.wPortStatus |= USB_PORT_STAT_POWER;
			OK(0);

		case USB_PORT_FEAT_ENABLE:
			rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
			OK(0);
		}
		break;

	case RH_SET_ADDRESS:
		root_hub_devnum = wValue;
		OK(0);

	case RH_GET_DESCRIPTOR:
		switch ((wValue & 0xff00) >> 8) {
		case USB_DT_DEVICE:
			len = sizeof(sl811_rh_dev_des);
			bufp = sl811_rh_dev_des;
			OK(len);

		case USB_DT_CONFIG:
			len = sizeof(sl811_rh_config_des);
			bufp = sl811_rh_config_des;
			OK(len);

		case USB_DT_STRING:
			len = usb_root_hub_string(wValue & 0xff, (int)(long)0,	"SL811HS", data, wLength);
			if (len > 0) {
				bufp = data;
				OK(len);
			}

		default:
			status = -32;
		}
		break;

	case RH_GET_DESCRIPTOR | USB_TYPE_CLASS:
		len = sizeof(sl811_rh_hub_des);
		bufp = sl811_rh_hub_des;
		OK(len);

	case RH_GET_CONFIGURATION:
		bufp[0] = 0x01;
		OK(1);

	case RH_SET_CONFIGURATION:
		OK(0);

	default:
		PDEBUG(1, "unsupported root hub command\n");
		status = -32;
	}

	len = min(len, buf_len);
	if (data != bufp)
		memcpy(data, bufp, len);

	PDEBUG(5, "len = %d, status = %d\n", len, status);

	usb_dev->status = status;
	usb_dev->act_len = len;

	return status == 0 ? len : status;
}
