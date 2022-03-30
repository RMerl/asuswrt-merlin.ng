/* SPDX-License-Identifier: GPL-2.0 */
/******************************************************************
 * Copyright 2008 Mentor Graphics Corporation
 * Copyright (C) 2008 by Texas Instruments
 *
 * This file is part of the Inventra Controller Driver for Linux.
 ******************************************************************/

#ifndef __MUSB_HDRC_DEFS_H__
#define __MUSB_HDRC_DEFS_H__

#include <usb_defs.h>
#include <asm/io.h>

#define MUSB_EP0_FIFOSIZE	64	/* This is non-configurable */

/* EP0 */
struct musb_ep0_regs {
	u16	reserved4;
	u16	csr0;
	u16	reserved5;
	u16	reserved6;
	u16	count0;
	u8	host_type0;
	u8	host_naklimit0;
	u8	reserved7;
	u8	reserved8;
	u8	reserved9;
	u8	configdata;
};

/* EP 1-15 */
struct musb_epN_regs {
	u16	txmaxp;
	u16	txcsr;
	u16	rxmaxp;
	u16	rxcsr;
	u16	rxcount;
	u8	txtype;
	u8	txinterval;
	u8	rxtype;
	u8	rxinterval;
	u8	reserved0;
	u8	fifosize;
};

/* Mentor USB core register overlay structure */
#ifndef musb_regs
struct musb_regs {
	/* common registers */
	u8	faddr;
	u8	power;
	u16	intrtx;
	u16	intrrx;
	u16	intrtxe;
	u16	intrrxe;
	u8	intrusb;
	u8	intrusbe;
	u16	frame;
	u8	index;
	u8	testmode;
	/* indexed registers */
	u16	txmaxp;
	u16	txcsr;
	u16	rxmaxp;
	u16	rxcsr;
	u16	rxcount;
	u8	txtype;
	u8	txinterval;
	u8	rxtype;
	u8	rxinterval;
	u8	reserved0;
	u8	fifosize;
	/* fifo */
	u32	fifox[16];
	/* OTG, dynamic FIFO, version & vendor registers */
	u8	devctl;
	u8	reserved1;
	u8	txfifosz;
	u8	rxfifosz;
	u16	txfifoadd;
	u16	rxfifoadd;
	u32	vcontrol;
	u16	hwvers;
	u16	reserved2a[1];
	u8	ulpi_busctl;
	u8	reserved2b[1];
	u16	reserved2[3];
	u8	epinfo;
	u8	raminfo;
	u8	linkinfo;
	u8	vplen;
	u8	hseof1;
	u8	fseof1;
	u8	lseof1;
	u8	reserved3;
	/* target address registers */
	struct musb_tar_regs {
		u8	txfuncaddr;
		u8	reserved0;
		u8	txhubaddr;
		u8	txhubport;
		u8	rxfuncaddr;
		u8	reserved1;
		u8	rxhubaddr;
		u8	rxhubport;
	} tar[16];
	/*
	 * endpoint registers
	 * ep0 elements are valid when array index is 0
	 * otherwise epN is valid
	 */
	union musb_ep_regs {
		struct musb_ep0_regs ep0;
		struct musb_epN_regs epN;
	} ep[16];

} __attribute__((packed));
#endif

/*
 * MUSB Register bits
 */

/* POWER */
#define MUSB_POWER_ISOUPDATE	0x80
#define MUSB_POWER_SOFTCONN	0x40
#define MUSB_POWER_HSENAB	0x20
#define MUSB_POWER_HSMODE	0x10
#define MUSB_POWER_RESET	0x08
#define MUSB_POWER_RESUME	0x04
#define MUSB_POWER_SUSPENDM	0x02
#define MUSB_POWER_ENSUSPEND	0x01
#define MUSB_POWER_HSMODE_SHIFT	4

/* INTRUSB */
#define MUSB_INTR_SUSPEND	0x01
#define MUSB_INTR_RESUME	0x02
#define MUSB_INTR_RESET		0x04
#define MUSB_INTR_BABBLE	0x04
#define MUSB_INTR_SOF		0x08
#define MUSB_INTR_CONNECT	0x10
#define MUSB_INTR_DISCONNECT	0x20
#define MUSB_INTR_SESSREQ	0x40
#define MUSB_INTR_VBUSERROR	0x80	/* For SESSION end */

/* DEVCTL */
#define MUSB_DEVCTL_BDEVICE	0x80
#define MUSB_DEVCTL_FSDEV	0x40
#define MUSB_DEVCTL_LSDEV	0x20
#define MUSB_DEVCTL_VBUS	0x18
#define MUSB_DEVCTL_VBUS_SHIFT	3
#define MUSB_DEVCTL_HM		0x04
#define MUSB_DEVCTL_HR		0x02
#define MUSB_DEVCTL_SESSION	0x01

/* ULPI VBUSCONTROL */
#define ULPI_USE_EXTVBUS	0x01
#define ULPI_USE_EXTVBUSIND	0x02

/* TESTMODE */
#define MUSB_TEST_FORCE_HOST	0x80
#define MUSB_TEST_FIFO_ACCESS	0x40
#define MUSB_TEST_FORCE_FS	0x20
#define MUSB_TEST_FORCE_HS	0x10
#define MUSB_TEST_PACKET	0x08
#define MUSB_TEST_K		0x04
#define MUSB_TEST_J		0x02
#define MUSB_TEST_SE0_NAK	0x01

/* Allocate for double-packet buffering (effectively doubles assigned _SIZE) */
#define MUSB_FIFOSZ_DPB		0x10
/* Allocation size (8, 16, 32, ... 4096) */
#define MUSB_FIFOSZ_SIZE	0x0f

/* CSR0 */
#define MUSB_CSR0_FLUSHFIFO	0x0100
#define MUSB_CSR0_TXPKTRDY	0x0002
#define MUSB_CSR0_RXPKTRDY	0x0001

/* CSR0 in Peripheral mode */
#define MUSB_CSR0_P_SVDSETUPEND	0x0080
#define MUSB_CSR0_P_SVDRXPKTRDY	0x0040
#define MUSB_CSR0_P_SENDSTALL	0x0020
#define MUSB_CSR0_P_SETUPEND	0x0010
#define MUSB_CSR0_P_DATAEND	0x0008
#define MUSB_CSR0_P_SENTSTALL	0x0004

/* CSR0 in Host mode */
#define MUSB_CSR0_H_DIS_PING		0x0800
#define MUSB_CSR0_H_WR_DATATOGGLE	0x0400	/* Set to allow setting: */
#define MUSB_CSR0_H_DATATOGGLE		0x0200	/* Data toggle control */
#define MUSB_CSR0_H_NAKTIMEOUT		0x0080
#define MUSB_CSR0_H_STATUSPKT		0x0040
#define MUSB_CSR0_H_REQPKT		0x0020
#define MUSB_CSR0_H_ERROR		0x0010
#define MUSB_CSR0_H_SETUPPKT		0x0008
#define MUSB_CSR0_H_RXSTALL		0x0004

/* CSR0 bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB_CSR0_P_WZC_BITS	\
	(MUSB_CSR0_P_SENTSTALL)
#define MUSB_CSR0_H_WZC_BITS	\
	(MUSB_CSR0_H_NAKTIMEOUT | MUSB_CSR0_H_RXSTALL \
	| MUSB_CSR0_RXPKTRDY)

/* TxType/RxType */
#define MUSB_TYPE_SPEED		0xc0
#define MUSB_TYPE_SPEED_SHIFT	6
#define MUSB_TYPE_SPEED_HIGH 	1
#define MUSB_TYPE_SPEED_FULL 	2
#define MUSB_TYPE_SPEED_LOW	3
#define MUSB_TYPE_PROTO		0x30	/* Implicitly zero for ep0 */
#define MUSB_TYPE_PROTO_SHIFT	4
#define MUSB_TYPE_REMOTE_END	0xf	/* Implicitly zero for ep0 */
#define MUSB_TYPE_PROTO_BULK 	2
#define MUSB_TYPE_PROTO_INTR 	3

/* CONFIGDATA */
#define MUSB_CONFIGDATA_MPRXE		0x80	/* Auto bulk pkt combining */
#define MUSB_CONFIGDATA_MPTXE		0x40	/* Auto bulk pkt splitting */
#define MUSB_CONFIGDATA_BIGENDIAN	0x20
#define MUSB_CONFIGDATA_HBRXE		0x10	/* HB-ISO for RX */
#define MUSB_CONFIGDATA_HBTXE		0x08	/* HB-ISO for TX */
#define MUSB_CONFIGDATA_DYNFIFO		0x04	/* Dynamic FIFO sizing */
#define MUSB_CONFIGDATA_SOFTCONE	0x02	/* SoftConnect */
#define MUSB_CONFIGDATA_UTMIDW		0x01	/* Data width 0/1 => 8/16bits */

/* TXCSR in Peripheral and Host mode */
#define MUSB_TXCSR_AUTOSET		0x8000
#define MUSB_TXCSR_MODE			0x2000
#define MUSB_TXCSR_DMAENAB		0x1000
#define MUSB_TXCSR_FRCDATATOG		0x0800
#define MUSB_TXCSR_DMAMODE		0x0400
#define MUSB_TXCSR_CLRDATATOG		0x0040
#define MUSB_TXCSR_FLUSHFIFO		0x0008
#define MUSB_TXCSR_FIFONOTEMPTY		0x0002
#define MUSB_TXCSR_TXPKTRDY		0x0001

/* TXCSR in Peripheral mode */
#define MUSB_TXCSR_P_ISO		0x4000
#define MUSB_TXCSR_P_INCOMPTX		0x0080
#define MUSB_TXCSR_P_SENTSTALL		0x0020
#define MUSB_TXCSR_P_SENDSTALL		0x0010
#define MUSB_TXCSR_P_UNDERRUN		0x0004

/* TXCSR in Host mode */
#define MUSB_TXCSR_H_WR_DATATOGGLE	0x0200
#define MUSB_TXCSR_H_DATATOGGLE		0x0100
#define MUSB_TXCSR_H_NAKTIMEOUT		0x0080
#define MUSB_TXCSR_H_RXSTALL		0x0020
#define MUSB_TXCSR_H_ERROR		0x0004
#define MUSB_TXCSR_H_DATATOGGLE_SHIFT	8

/* TXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB_TXCSR_P_WZC_BITS	\
	(MUSB_TXCSR_P_INCOMPTX | MUSB_TXCSR_P_SENTSTALL \
	| MUSB_TXCSR_P_UNDERRUN | MUSB_TXCSR_FIFONOTEMPTY)
#define MUSB_TXCSR_H_WZC_BITS	\
	(MUSB_TXCSR_H_NAKTIMEOUT | MUSB_TXCSR_H_RXSTALL \
	| MUSB_TXCSR_H_ERROR | MUSB_TXCSR_FIFONOTEMPTY)

/* RXCSR in Peripheral and Host mode */
#define MUSB_RXCSR_AUTOCLEAR		0x8000
#define MUSB_RXCSR_DMAENAB		0x2000
#define MUSB_RXCSR_DISNYET		0x1000
#define MUSB_RXCSR_PID_ERR		0x1000
#define MUSB_RXCSR_DMAMODE		0x0800
#define MUSB_RXCSR_INCOMPRX		0x0100
#define MUSB_RXCSR_CLRDATATOG		0x0080
#define MUSB_RXCSR_FLUSHFIFO		0x0010
#define MUSB_RXCSR_DATAERROR		0x0008
#define MUSB_RXCSR_FIFOFULL		0x0002
#define MUSB_RXCSR_RXPKTRDY		0x0001

/* RXCSR in Peripheral mode */
#define MUSB_RXCSR_P_ISO		0x4000
#define MUSB_RXCSR_P_SENTSTALL		0x0040
#define MUSB_RXCSR_P_SENDSTALL		0x0020
#define MUSB_RXCSR_P_OVERRUN		0x0004

/* RXCSR in Host mode */
#define MUSB_RXCSR_H_AUTOREQ		0x4000
#define MUSB_RXCSR_H_WR_DATATOGGLE	0x0400
#define MUSB_RXCSR_H_DATATOGGLE		0x0200
#define MUSB_RXCSR_H_RXSTALL		0x0040
#define MUSB_RXCSR_H_REQPKT		0x0020
#define MUSB_RXCSR_H_ERROR		0x0004
#define MUSB_S_RXCSR_H_DATATOGGLE	9

/* RXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB_RXCSR_P_WZC_BITS	\
	(MUSB_RXCSR_P_SENTSTALL | MUSB_RXCSR_P_OVERRUN \
	| MUSB_RXCSR_RXPKTRDY)
#define MUSB_RXCSR_H_WZC_BITS	\
	(MUSB_RXCSR_H_RXSTALL | MUSB_RXCSR_H_ERROR \
	| MUSB_RXCSR_DATAERROR | MUSB_RXCSR_RXPKTRDY)

/* HUBADDR */
#define MUSB_HUBADDR_MULTI_TT		0x80

/* Endpoint configuration information. Note: The value of endpoint fifo size
 * element should be either 8,16,32,64,128,256,512,1024,2048 or 4096. Other
 * values are not supported
 */
struct musb_epinfo {
	u8	epnum;	/* endpoint number 	*/
	u8	epdir;	/* endpoint direction	*/
	u16	epsize;	/* endpoint FIFO size	*/
};

/*
 * Platform specific MUSB configuration. Any platform using the musb
 * functionality should create one instance of this structure in the
 * platform specific file.
 */
struct musb_config {
	struct	musb_regs	*regs;
	u32			timeout;
	u8			musb_speed;
	u8			extvbus;
};

/* externally defined data */
extern struct musb_config	musb_cfg;
extern struct musb_regs		*musbr;

/* exported functions */
extern void musb_start(void);
extern void musb_configure_ep(const struct musb_epinfo *epinfo, u8 cnt);
extern void write_fifo(u8 ep, u32 length, void *fifo_data);
extern void read_fifo(u8 ep, u32 length, void *fifo_data);

static inline u8 musb_read_ulpi_buscontrol(struct musb_regs *musbr)
{
	return readb(&musbr->ulpi_busctl);
}
static inline void musb_write_ulpi_buscontrol(struct musb_regs *musbr, u8 val)
{
	writeb(val, &musbr->ulpi_busctl);
}

#endif	/* __MUSB_HDRC_DEFS_H__ */
