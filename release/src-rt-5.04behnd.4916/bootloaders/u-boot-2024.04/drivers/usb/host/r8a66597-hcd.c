// SPDX-License-Identifier: GPL-2.0
/*
 * R8A66597 HCD (Host Controller Driver) for u-boot
 *
 * Copyright (C) 2008  Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <log.h>
#include <usb.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/usb/usb_urb_compat.h>
#include <power/regulator.h>

#include "r8a66597.h"

#ifdef R8A66597_DEBUG
#define R8A66597_DPRINT		printf
#else
#define R8A66597_DPRINT(...)
#endif

static void get_hub_data(struct usb_device *dev, u16 *hub_devnum, u16 *hubport)
{
	struct usb_device *parent = usb_dev_get_parent(dev);

	*hub_devnum = 0;
	*hubport = 0;

	/* check a device connected to root_hub */
	if ((parent && parent->devnum == 1) ||
	    dev->devnum == 1)
		return;

	*hub_devnum = (u8)parent->devnum;
	*hubport = parent->portnr - 1;
}

static void set_devadd(struct r8a66597 *r8a66597, u8 r8a66597_address,
		       struct usb_device *dev, int port)
{
	u16 val, usbspd, upphub, hubport;
	unsigned long devadd_reg = get_devadd_addr(r8a66597_address);

	get_hub_data(dev, &upphub, &hubport);
	usbspd = r8a66597->speed;
	val = (upphub << 11) | (hubport << 8) | (usbspd << 6) | (port & 0x0001);
	r8a66597_write(r8a66597, val, devadd_reg);
}

static int r8a66597_clock_enable(struct r8a66597 *r8a66597)
{
	u16 tmp;
	int i = 0;

	do {
		r8a66597_write(r8a66597, USBE, SYSCFG0);
		tmp = r8a66597_read(r8a66597, SYSCFG0);
		if (i++ > 1000) {
			printf("register access fail.\n");
			return -1;
		}
	} while ((tmp & USBE) != USBE);
	r8a66597_bclr(r8a66597, USBE, SYSCFG0);
	/*
	 * RZ/A Only:
	 * Bits XTAL(UCKSEL) and UPLLE in SYSCFG0 for USB0 controls both USB0
	 * and USB1, so we must always set the USB0 register
	 */
#if (CONFIG_R8A66597_XTAL == 1)
	r8a66597_bset(r8a66597, XTAL, SYSCFG0);
#endif
	mdelay(1);
	r8a66597_bset(r8a66597, UPLLE, SYSCFG0);
	mdelay(1);
	r8a66597_bset(r8a66597, SUSPM, SUSPMODE0);

	return 0;
}

static void r8a66597_clock_disable(struct r8a66597 *r8a66597)
{
	r8a66597_bclr(r8a66597, SUSPM, SUSPMODE0);

	r8a66597_bclr(r8a66597, UPLLE, SYSCFG0);
	mdelay(1);
	r8a66597_bclr(r8a66597, USBE, SYSCFG0);
	mdelay(1);
}

static void r8a66597_enable_port(struct r8a66597 *r8a66597, int port)
{
	u16 val;

	val = port ? DRPD : DCFM | DRPD;
	r8a66597_bset(r8a66597, val, get_syscfg_reg(port));
	r8a66597_bset(r8a66597, HSE, get_syscfg_reg(port));
}

static void r8a66597_disable_port(struct r8a66597 *r8a66597, int port)
{
	u16 val, tmp;

	r8a66597_write(r8a66597, 0, get_intenb_reg(port));
	r8a66597_write(r8a66597, 0, get_intsts_reg(port));

	r8a66597_port_power(r8a66597, port, 0);

	do {
		tmp = r8a66597_read(r8a66597, SOFCFG) & EDGESTS;
		udelay(640);
	} while (tmp == EDGESTS);

	val = port ? DRPD : DCFM | DRPD;
	r8a66597_bclr(r8a66597, val, get_syscfg_reg(port));
	r8a66597_bclr(r8a66597, HSE, get_syscfg_reg(port));
}

static int enable_controller(struct r8a66597 *r8a66597)
{
	int ret, port;

	ret = r8a66597_clock_enable(r8a66597);
	if (ret < 0)
		return ret;

	r8a66597_bset(r8a66597, USBE, SYSCFG0);

	r8a66597_bset(r8a66597, INTL, SOFCFG);
	r8a66597_write(r8a66597, 0, INTENB0);
	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++)
		r8a66597_write(r8a66597, 0, get_intenb_reg(port));

	r8a66597_bclr(r8a66597, BIGEND, CFIFOSEL);
	r8a66597_bclr(r8a66597, BIGEND, D0FIFOSEL);
	r8a66597_bclr(r8a66597, BIGEND, D1FIFOSEL);
	r8a66597_bset(r8a66597, TRNENSEL, SOFCFG);

	for (port = 0; port < R8A66597_MAX_ROOT_HUB; port++)
		r8a66597_enable_port(r8a66597, port);

	return 0;
}

static void disable_controller(struct r8a66597 *r8a66597)
{
	int i;

	if (!(r8a66597_read(r8a66597, SYSCFG0) & USBE))
		return;

	r8a66597_write(r8a66597, 0, INTENB0);
	r8a66597_write(r8a66597, 0, INTSTS0);

	r8a66597_write(r8a66597, 0, D0FIFOSEL);
	r8a66597_write(r8a66597, 0, D1FIFOSEL);
	r8a66597_write(r8a66597, 0, DCPCFG);
	r8a66597_write(r8a66597, 0x40, DCPMAXP);
	r8a66597_write(r8a66597, 0, DCPCTR);

	for (i = 0; i <= 10; i++)
		r8a66597_write(r8a66597, 0, get_devadd_addr(i));
	for (i = 1; i <= 5; i++) {
		r8a66597_write(r8a66597, 0, get_pipetre_addr(i));
		r8a66597_write(r8a66597, 0, get_pipetrn_addr(i));
	}
	for (i = 1; i < R8A66597_MAX_NUM_PIPE; i++) {
		r8a66597_write(r8a66597, 0, get_pipectr_addr(i));
		r8a66597_write(r8a66597, i, PIPESEL);
		r8a66597_write(r8a66597, 0, PIPECFG);
		r8a66597_write(r8a66597, 0, PIPEBUF);
		r8a66597_write(r8a66597, 0, PIPEMAXP);
		r8a66597_write(r8a66597, 0, PIPEPERI);
	}

	for (i = 0; i < R8A66597_MAX_ROOT_HUB; i++)
		r8a66597_disable_port(r8a66597, i);

	r8a66597_clock_disable(r8a66597);
}

static void r8a66597_reg_wait(struct r8a66597 *r8a66597, unsigned long reg,
			      u16 mask, u16 loop)
{
	u16 tmp;
	int i = 0;

	do {
		tmp = r8a66597_read(r8a66597, reg);
		if (i++ > 1000000) {
			printf("register%lx, loop %x is timeout\n", reg, loop);
			break;
		}
	} while ((tmp & mask) != loop);
}

static void pipe_buffer_setting(struct r8a66597 *r8a66597,
				struct usb_device *dev, unsigned long pipe)
{
	u16 val = 0;
	u16 pipenum, bufnum, maxpacket;

	if (usb_pipein(pipe)) {
		pipenum = BULK_IN_PIPENUM;
		bufnum = BULK_IN_BUFNUM;
		maxpacket = dev->epmaxpacketin[usb_pipeendpoint(pipe)];
	} else {
		pipenum = BULK_OUT_PIPENUM;
		bufnum = BULK_OUT_BUFNUM;
		maxpacket = dev->epmaxpacketout[usb_pipeendpoint(pipe)];
	}

	if (r8a66597->pipe_config & (1 << pipenum))
		return;
	r8a66597->pipe_config |= (1 << pipenum);

	r8a66597_bset(r8a66597, ACLRM, get_pipectr_addr(pipenum));
	r8a66597_bclr(r8a66597, ACLRM, get_pipectr_addr(pipenum));
	r8a66597_write(r8a66597, pipenum, PIPESEL);

	/* FIXME: This driver support bulk transfer only. */
	if (!usb_pipein(pipe))
		val |= R8A66597_DIR;
	else
		val |= R8A66597_SHTNAK;
	val |= R8A66597_BULK | R8A66597_DBLB | usb_pipeendpoint(pipe);
	r8a66597_write(r8a66597, val, PIPECFG);

	r8a66597_write(r8a66597, (8 << 10) | bufnum, PIPEBUF);
	r8a66597_write(r8a66597, make_devsel(usb_pipedevice(pipe)) |
				 maxpacket, PIPEMAXP);
	r8a66597_write(r8a66597, 0, PIPEPERI);
	r8a66597_write(r8a66597, SQCLR, get_pipectr_addr(pipenum));
}

static int send_setup_packet(struct r8a66597 *r8a66597, struct usb_device *dev,
			     struct devrequest *setup)
{
	int i;
	unsigned short *p = (unsigned short *)setup;
	unsigned long setup_addr = USBREQ;
	u16 intsts1;
	int timeout = 3000;
	u16 dcpctr;
	u16 devsel = setup->request == USB_REQ_SET_ADDRESS ? 0 : dev->devnum;

	r8a66597_write(r8a66597, make_devsel(devsel) |
				 (8 << dev->maxpacketsize), DCPMAXP);
	r8a66597_write(r8a66597, ~(SIGN | SACK), INTSTS1);

	dcpctr = r8a66597_read(r8a66597, DCPCTR);
	if ((dcpctr & PID) == PID_BUF) {
		if (readw_poll_timeout(r8a66597->reg + DCPCTR, dcpctr,
				       dcpctr & BSTS, 1000) < 0) {
			printf("DCPCTR BSTS timeout!\n");
			return -ETIMEDOUT;
		}
	}

	for (i = 0; i < 4; i++) {
		r8a66597_write(r8a66597, le16_to_cpu(p[i]), setup_addr);
		setup_addr += 2;
	}
	r8a66597_write(r8a66597, ~0x0001, BRDYSTS);
	r8a66597_write(r8a66597, SUREQ, DCPCTR);

	while (1) {
		intsts1 = r8a66597_read(r8a66597, INTSTS1);
		if (intsts1 & SACK)
			break;
		if (intsts1 & SIGN) {
			printf("setup packet send error\n");
			return -1;
		}
		if (timeout-- < 0) {
			printf("setup packet timeout\n");
			return -1;
		}
		udelay(500);
	}

	return 0;
}

static int send_bulk_packet(struct r8a66597 *r8a66597, struct usb_device *dev,
			    unsigned long pipe, void *buffer, int transfer_len)
{
	u16 tmp, bufsize;
	u16 *buf;
	size_t size;

	R8A66597_DPRINT("%s\n", __func__);

	r8a66597_mdfy(r8a66597, MBW | BULK_OUT_PIPENUM,
		      MBW | CURPIPE, CFIFOSEL);
	r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, BULK_OUT_PIPENUM);
	tmp = r8a66597_read(r8a66597, CFIFOCTR);
	if ((tmp & FRDY) == 0) {
		printf("%s FRDY is not set (%x)\n", __func__, tmp);
		return -1;
	}

	/* prepare parameters */
	bufsize = dev->epmaxpacketout[usb_pipeendpoint(pipe)];
	buf = (u16 *)(buffer + dev->act_len);
	size = min((int)bufsize, transfer_len - dev->act_len);

	/* write fifo */
	r8a66597_write(r8a66597, ~(1 << BULK_OUT_PIPENUM), BEMPSTS);
	if (buffer) {
		r8a66597_write_fifo(r8a66597, CFIFO, buf, size);
		r8a66597_write(r8a66597, BVAL, CFIFOCTR);
	}

	/* update parameters */
	dev->act_len += size;

	r8a66597_mdfy(r8a66597, PID_BUF, PID,
		      get_pipectr_addr(BULK_OUT_PIPENUM));

	while (!(r8a66597_read(r8a66597, BEMPSTS) & (1 << BULK_OUT_PIPENUM)))
		if (ctrlc())
			return -1;
	r8a66597_write(r8a66597, ~(1 << BULK_OUT_PIPENUM), BEMPSTS);

	if (dev->act_len >= transfer_len)
		r8a66597_mdfy(r8a66597, PID_NAK, PID,
			      get_pipectr_addr(BULK_OUT_PIPENUM));

	return 0;
}

static int receive_bulk_packet(struct r8a66597 *r8a66597,
			       struct usb_device *dev,
			       unsigned long pipe,
			       void *buffer, int transfer_len)
{
	u16 tmp;
	u16 *buf;
	const u16 pipenum = BULK_IN_PIPENUM;
	int rcv_len;
	int maxpacket = dev->epmaxpacketin[usb_pipeendpoint(pipe)];

	R8A66597_DPRINT("%s\n", __func__);

	/* prepare */
	if (dev->act_len == 0) {
		r8a66597_mdfy(r8a66597, PID_NAK, PID,
			      get_pipectr_addr(pipenum));
		r8a66597_write(r8a66597, ~(1 << pipenum), BRDYSTS);

		r8a66597_write(r8a66597, TRCLR, get_pipetre_addr(pipenum));
		r8a66597_write(r8a66597,
			       (transfer_len + maxpacket - 1) / maxpacket,
				get_pipetrn_addr(pipenum));
		r8a66597_bset(r8a66597, TRENB, get_pipetre_addr(pipenum));

		r8a66597_mdfy(r8a66597, PID_BUF, PID,
			      get_pipectr_addr(pipenum));
	}

	r8a66597_mdfy(r8a66597, MBW | pipenum, MBW | CURPIPE, CFIFOSEL);
	r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, pipenum);

	while (!(r8a66597_read(r8a66597, BRDYSTS) & (1 << pipenum)))
		if (ctrlc())
			return -1;
	r8a66597_write(r8a66597, ~(1 << pipenum), BRDYSTS);

	tmp = r8a66597_read(r8a66597, CFIFOCTR);
	if ((tmp & FRDY) == 0) {
		printf("%s FRDY is not set. (%x)\n", __func__, tmp);
		return -1;
	}

	buf = (u16 *)(buffer + dev->act_len);
	rcv_len = tmp & DTLN;
	dev->act_len += rcv_len;

	if (buffer) {
		if (rcv_len == 0)
			r8a66597_write(r8a66597, BCLR, CFIFOCTR);
		else
			r8a66597_read_fifo(r8a66597, CFIFO, buf, rcv_len);
	}

	return 0;
}

static int receive_control_packet(struct r8a66597 *r8a66597,
				  struct usb_device *dev,
				  void *buffer, int transfer_len)
{
	u16 tmp;
	int rcv_len;

	/* FIXME: limit transfer size : 64byte or less */

	r8a66597_bclr(r8a66597, R8A66597_DIR, DCPCFG);
	r8a66597_mdfy(r8a66597, 0, ISEL | CURPIPE, CFIFOSEL);
	r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
	r8a66597_bset(r8a66597, SQSET, DCPCTR);
	r8a66597_write(r8a66597, BCLR, CFIFOCTR);
	r8a66597_mdfy(r8a66597, PID_BUF, PID, DCPCTR);

	while (!(r8a66597_read(r8a66597, BRDYSTS) & 0x0001))
		if (ctrlc())
			return -1;
	r8a66597_write(r8a66597, ~0x0001, BRDYSTS);

	r8a66597_mdfy(r8a66597, MBW, MBW | CURPIPE, CFIFOSEL);
	r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);

	tmp = r8a66597_read(r8a66597, CFIFOCTR);
	if ((tmp & FRDY) == 0) {
		printf("%s FRDY is not set. (%x)\n", __func__, tmp);
		return -1;
	}

	rcv_len = tmp & DTLN;
	dev->act_len += rcv_len;

	r8a66597_mdfy(r8a66597, PID_NAK, PID, DCPCTR);

	if (buffer) {
		if (rcv_len == 0)
			r8a66597_write(r8a66597, BCLR, DCPCTR);
		else
			r8a66597_read_fifo(r8a66597, CFIFO, buffer, rcv_len);
	}

	return 0;
}

static int send_status_packet(struct r8a66597 *r8a66597,
			      unsigned long pipe)
{
	r8a66597_bset(r8a66597, SQSET, DCPCTR);
	r8a66597_mdfy(r8a66597, PID_NAK, PID, DCPCTR);

	if (usb_pipein(pipe)) {
		r8a66597_bset(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, ISEL, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		r8a66597_write(r8a66597, ~BEMP0, BEMPSTS);
		r8a66597_write(r8a66597, BCLR | BVAL, CFIFOCTR);
	} else {
		r8a66597_bclr(r8a66597, R8A66597_DIR, DCPCFG);
		r8a66597_mdfy(r8a66597, 0, ISEL | CURPIPE, CFIFOSEL);
		r8a66597_reg_wait(r8a66597, CFIFOSEL, CURPIPE, 0);
		r8a66597_write(r8a66597, BCLR, CFIFOCTR);
	}
	r8a66597_mdfy(r8a66597, PID_BUF, PID, DCPCTR);

	while (!(r8a66597_read(r8a66597, BEMPSTS) & 0x0001))
		if (ctrlc())
			return -1;

	return 0;
}

static void r8a66597_check_syssts(struct r8a66597 *r8a66597, int port)
{
	int count = R8A66597_MAX_SAMPLING;
	unsigned short syssts, old_syssts;

	R8A66597_DPRINT("%s\n", __func__);

	old_syssts = r8a66597_read(r8a66597, get_syssts_reg(port) & LNST);
	while (count > 0) {
		mdelay(R8A66597_RH_POLL_TIME);

		syssts = r8a66597_read(r8a66597, get_syssts_reg(port) & LNST);
		if (syssts == old_syssts) {
			count--;
		} else {
			count = R8A66597_MAX_SAMPLING;
			old_syssts = syssts;
		}
	}
}

static void r8a66597_bus_reset(struct r8a66597 *r8a66597, int port)
{
	mdelay(10);
	r8a66597_mdfy(r8a66597, USBRST, USBRST | UACT, get_dvstctr_reg(port));
	mdelay(50);
	r8a66597_mdfy(r8a66597, UACT, USBRST | UACT, get_dvstctr_reg(port));
	mdelay(50);
}

static int check_usb_device_connecting(struct r8a66597 *r8a66597)
{
	int timeout = 10000;	/* 100usec * 10000 = 1sec */
	int i;

	for (i = 0; i < 5; i++) {
		/* check a usb cable connect */
		while (!(r8a66597_read(r8a66597, INTSTS1) & ATTCH)) {
			if (timeout-- < 0) {
				printf("%s timeout.\n", __func__);
				return -1;
			}
			udelay(100);
		}

		/* check a data line */
		r8a66597_check_syssts(r8a66597, 0);

		r8a66597_bus_reset(r8a66597, 0);
		r8a66597->speed = get_rh_usb_speed(r8a66597, 0);

		if (!(r8a66597_read(r8a66597, INTSTS1) & DTCH)) {
			r8a66597->port_change = USB_PORT_STAT_C_CONNECTION;
			r8a66597->port_status = USB_PORT_STAT_CONNECTION |
						USB_PORT_STAT_ENABLE;
			return 0;	/* success */
		}

		R8A66597_DPRINT("USB device has detached. retry = %d\n", i);
		r8a66597_write(r8a66597, ~DTCH, INTSTS1);
	}

	return -1;	/* fail */
}

/* Virtual Root Hub */

#include <usbroothubdes.h>

static int r8a66597_submit_rh_msg(struct udevice *udev, struct usb_device *dev,
				  unsigned long pipe, void *buffer,
				  int transfer_len, struct devrequest *cmd)
{
	struct r8a66597 *r8a66597 = dev_get_priv(udev);
	int leni = transfer_len;
	int len = 0;
	int stat = 0;
	__u16 bmRType_bReq;
	__u16 wValue;
	__u16 wLength;
	unsigned char data[32];

	R8A66597_DPRINT("%s\n", __func__);

	if (usb_pipeint(pipe)) {
		printf("Root-Hub submit IRQ: NOT implemented");
		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	wValue	      = cpu_to_le16 (cmd->value);
	wLength	      = cpu_to_le16 (cmd->length);

	switch (bmRType_bReq) {
	case RH_GET_STATUS:
		*(__u16 *)buffer = cpu_to_le16(1);
		len = 2;
		break;
	case RH_GET_STATUS | RH_INTERFACE:
		*(__u16 *)buffer = cpu_to_le16(0);
		len = 2;
		break;
	case RH_GET_STATUS | RH_ENDPOINT:
		*(__u16 *)buffer = cpu_to_le16(0);
		len = 2;
		break;
	case RH_GET_STATUS | RH_CLASS:
		*(__u32 *)buffer = cpu_to_le32(0);
		len = 4;
		break;
	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		*(__u32 *)buffer = cpu_to_le32(r8a66597->port_status |
						(r8a66597->port_change << 16));
		len = 4;
		break;
	case RH_CLEAR_FEATURE | RH_ENDPOINT:
	case RH_CLEAR_FEATURE | RH_CLASS:
		break;

	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		switch (wValue) {
		case RH_C_PORT_CONNECTION:
			r8a66597->port_change &= ~USB_PORT_STAT_C_CONNECTION;
			break;
		}
		break;

	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		switch (wValue) {
		case (RH_PORT_SUSPEND):
			break;
		case (RH_PORT_RESET):
			r8a66597_bus_reset(r8a66597, 0);
			break;
		case (RH_PORT_POWER):
			break;
		case (RH_PORT_ENABLE):
			break;
		}
		break;
	case RH_SET_ADDRESS:
		r8a66597->rh_devnum = wValue;
		break;
	case RH_GET_DESCRIPTOR:
		switch ((wValue & 0xff00) >> 8) {
		case (0x01): /* device descriptor */
			len = min_t(unsigned int,
				    leni,
				  min_t(unsigned int,
					sizeof(root_hub_dev_des),
				      wLength));
			memcpy(buffer, root_hub_dev_des, len);
			break;
		case (0x02): /* configuration descriptor */
			len = min_t(unsigned int,
				    leni,
				  min_t(unsigned int,
					sizeof(root_hub_config_des),
				      wLength));
			memcpy(buffer, root_hub_config_des, len);
			break;
		case (0x03): /* string descriptors */
			if (wValue == 0x0300) {
				len = min_t(unsigned int,
					    leni,
					  min_t(unsigned int,
						sizeof(root_hub_str_index0),
					      wLength));
				memcpy(buffer, root_hub_str_index0, len);
			}
			if (wValue == 0x0301) {
				len = min_t(unsigned int,
					    leni,
					  min_t(unsigned int,
						sizeof(root_hub_str_index1),
					      wLength));
				memcpy(buffer, root_hub_str_index1, len);
			}
			break;
		default:
			stat = USB_ST_STALLED;
		}
		break;

	case RH_GET_DESCRIPTOR | RH_CLASS:
	{
		__u32 temp = 0x00000001;

		data[0] = 9;		/* min length; */
		data[1] = 0x29;
		data[2] = temp & RH_A_NDP;
		data[3] = 0;
		if (temp & RH_A_PSM)
			data[3] |= 0x1;
		if (temp & RH_A_NOCP)
			data[3] |= 0x10;
		else if (temp & RH_A_OCPM)
			data[3] |= 0x8;

		/* corresponds to data[4-7] */
		data[5] = (temp & RH_A_POTPGT) >> 24;
		data[7] = temp & RH_B_DR;
		if (data[2] < 7) {
			data[8] = 0xff;
		} else {
			data[0] += 2;
			data[8] = (temp & RH_B_DR) >> 8;
			data[9] = 0xff;
			data[10] = 0xff;
		}

		len = min_t(unsigned int, leni,
			    min_t(unsigned int, data[0], wLength));
		memcpy(buffer, data, len);
		break;
	}

	case RH_GET_CONFIGURATION:
		*(__u8 *)buffer = 0x01;
		len = 1;
		break;
	case RH_SET_CONFIGURATION:
		break;
	default:
		R8A66597_DPRINT("unsupported root hub command");
		stat = USB_ST_STALLED;
	}

	mdelay(1);

	len = min_t(int, len, leni);

	dev->act_len = len;
	dev->status = stat;

	return stat;
}

static int r8a66597_submit_control_msg(struct udevice *udev,
				       struct usb_device *dev,
				       unsigned long pipe, void *buffer,
				       int length, struct devrequest *setup)
{
	struct r8a66597 *r8a66597 = dev_get_priv(udev);
	u16 r8a66597_address = setup->request == USB_REQ_SET_ADDRESS ?
					0 : dev->devnum;

	debug("%s: dev='%s', udev=%p, udev->dev='%s', portnr=%d\n", __func__,
	      udev->name, dev, dev->dev->name, dev->portnr);

	R8A66597_DPRINT("%s\n", __func__);
	if (usb_pipedevice(pipe) == r8a66597->rh_devnum)
		return r8a66597_submit_rh_msg(udev, dev, pipe, buffer,
					      length, setup);

	R8A66597_DPRINT("%s: setup\n", __func__);
	set_devadd(r8a66597, r8a66597_address, dev, 0);

	if (send_setup_packet(r8a66597, dev, setup) < 0) {
		printf("setup packet send error\n");
		return -1;
	}

	dev->act_len = 0;
	if (usb_pipein(pipe))
		if (receive_control_packet(r8a66597, dev, buffer,
					   length) < 0)
			return -1;

	if (send_status_packet(r8a66597, pipe) < 0)
		return -1;

	dev->status = 0;

	return 0;
}

static int r8a66597_submit_bulk_msg(struct udevice *udev,
				    struct usb_device *dev, unsigned long pipe,
				    void *buffer, int length)
{
	struct r8a66597 *r8a66597 = dev_get_priv(udev);
	int ret = 0;

	debug("%s: dev='%s', udev=%p\n", __func__, udev->name, dev);

	R8A66597_DPRINT("%s\n", __func__);
	R8A66597_DPRINT("pipe = %08x, buffer = %p, len = %d, devnum = %d\n",
			pipe, buffer, length, dev->devnum);

	set_devadd(r8a66597, dev->devnum, dev, 0);

	pipe_buffer_setting(r8a66597, dev, pipe);

	dev->act_len = 0;
	while (dev->act_len < length && ret == 0) {
		if (ctrlc())
			return -1;

		if (usb_pipein(pipe))
			ret = receive_bulk_packet(r8a66597, dev, pipe, buffer,
						  length);
		else
			ret = send_bulk_packet(r8a66597, dev, pipe, buffer,
					       length);
	}

	if (ret == 0)
		dev->status = 0;

	return ret;
}

static int r8a66597_usb_of_to_plat(struct udevice *dev)
{
	struct r8a66597 *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->reg = addr;

	return 0;
}

static int r8a66597_usb_probe(struct udevice *dev)
{
	struct r8a66597 *priv = dev_get_priv(dev);
	struct usb_bus_priv *bus_priv = dev_get_uclass_priv(dev);
	int ret;

	bus_priv->desc_before_addr = true;

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = device_get_supply_regulator(dev, "vbus-supply",
						  &priv->vbus_supply);
		if (ret) {
			dev_err(dev,
				"can't get VBUS supply\n");
			return ret;
		}

		ret = regulator_set_enable(priv->vbus_supply, true);
		if (ret) {
			dev_err(dev,
				"can't enable VBUS supply\n");
			return ret;
		}
	}

	disable_controller(priv);
	mdelay(100);

	enable_controller(priv);
	r8a66597_port_power(priv, 0, 1);

	/* check usb device */
	check_usb_device_connecting(priv);

	mdelay(50);

	return 0;
}

static int r8a66597_usb_remove(struct udevice *dev)
{
	struct r8a66597 *priv = dev_get_priv(dev);
	int ret;

	disable_controller(priv);

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = regulator_set_enable(priv->vbus_supply, false);
		if (ret) {
			dev_err(dev,
				"can't disable VBUS supply\n");
			return ret;
		}
	}

	return 0;
}

struct dm_usb_ops r8a66597_usb_ops = {
	.control = r8a66597_submit_control_msg,
	.bulk = r8a66597_submit_bulk_msg,
};

static const struct udevice_id r8a66597_usb_ids[] = {
	{ .compatible = "renesas,rza1-usbhs" },
	{ }
};

U_BOOT_DRIVER(usb_r8a66597) = {
	.name	= "r8a66597_usb",
	.id	= UCLASS_USB,
	.of_match = r8a66597_usb_ids,
	.of_to_plat = r8a66597_usb_of_to_plat,
	.probe	= r8a66597_usb_probe,
	.remove = r8a66597_usb_remove,
	.ops	= &r8a66597_usb_ops,
	.priv_auto	= sizeof(struct r8a66597),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
