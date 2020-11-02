/*
 * Broadcom PCI V.90 CODEC interface.
 *
 * Refer to "BCM4211 Controller Programmers Guide".
 * Copyright(c) 1999 Broadcom Corporation
 *
 * $Id: bcmcodec.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_BCMCODEC_H
#define	_BCMCODEC_H

#include <typedefs.h>

#define	VENDOR_BROADCOM		0x14e4

#define	BCM4211CODEC_DEVICE_ID	0x4212
#define	BCM4231CODEC_DEVICE_ID	0x4232

#define	BCM4411CODEC_DEVICE_ID	0x4411		/* bcm44xx family pci v.90 */
#define	BCM4431CODEC_DEVICE_ID	0x4431		/* bcm44xx family cardbus v.90 */

#define	BCMCODEC_MAX_FIFO	63		/* max xmtfifoavail/rcvfifodepth */
#define	BCMCODEC_MAX_DMA	4096		/* 12 bits of dma addressing */

/* cpp contortions to concatenate w/arg prescan */
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)

/*
 * PCI/MSI host interface registers.
 */
typedef volatile struct {
	/* device control */
	uint32	devcontrol;
	uint32	PAD[7];

	/* interrupt status and mask */
	uint32	intstatus;
	uint32	intmask;
	uint32	gptimer;
	uint32	PAD[3];

	/* indirect codec register access */
	uint32	codecctlcommand;
	uint32	codecctlstatus;
	uint32	PAD[28];

	/* general purpose io */
	uint32	gpiooutput;
	uint32	gpioouten;
	uint32	gpioinput;
	uint32	PAD;

	/* pio mode */
	uint16	xmtfifoavail;
	uint16	xmtfifodata;
	uint16	rcvfifodepth;
	uint16	rcvfifodata;
	uint16	xmtfifowrmk;
	uint16	rcvfifowrmk;
	uint32	PAD;

	/* cardbus */
	uint32	funcevnt;
	uint32	funcevntmask;
	uint32	funcstate;
	uint32	funcforce;
	uint32	PAD[40];

	/* msi mode: shadow pci config registers */
	uint16	pcicfg[64];

	/* transmit dma engine */
	uint32	xmtcontrol;
	uint32	xmtaddr;
	uint32	xmtptr;
	uint32	xmtstatus;

	/* receive dma engine */
	uint32	rcvcontrol;
	uint32	rcvaddr;
	uint32	rcvptr;
	uint32	rcvstatus;
} bcmcodecregs_t;

/* device control */
#define	DC_RS		((uint32)1 << 0)	/* soft reset */
#define	DC_XE		((uint32)1 << 5)	/* transmit enable */
#define	DC_RE		((uint32)1 << 6)	/* receive enable */
#define	DC_WE		((uint32)1 << 7)	/* wakeup (on ring) enable */
#define	DC_BS		((uint32)1 << 11)	/* byte swap */
#define	DC_CR		((uint32)1 << 12)	/* codec reset */
#define	DC_CM		((uint32)1 << 13)	/* control mode - ctl not mixed w/data */

/* intstatus and intmask */
#define	I_RD		((uint32)1 << 6)	/* ring detect */
#define	I_TO		((uint32)1 << 7)	/* timeout */
#define	I_PC		((uint32)1 << 10)	/* pci descriptor error */
#define	I_PD		((uint32)1 << 11)	/* pci data error */
#define	I_DE		((uint32)1 << 12)	/* descriptor protocol error */
#define	I_RU		((uint32)1 << 13)	/* receive descriptor underflow */
#define	I_RO		((uint32)1 << 14)	/* receive fifo overflow */
#define	I_XU		((uint32)1 << 15)	/* transmit fifo underflow */
#define	I_RI		((uint32)1 << 16)	/* receive buffer dma complete */
#define	I_RF		((uint32)1 << 17)	/* receive fifo watermark */
#define	I_XI		((uint32)1 << 24)	/* transmit buffer dma complete */
#define	I_XF		((uint32)1 << 25)	/* transmit fifo watermark */

/* codec control command */
#define	CC_DA_MASK	0xff			/* write data */
#define	CC_AD_MASK	0x1f00			/* address */
#define	CC_AD_SHIFT	8
#define	CC_RD		((uint32)1 << 13)	/* 1=read, 0=write */

/* codec control status */
#define	CS_DA_MASK	0xff			/* read data */
#define	CS_BU		((uint32)1 << 15)	/* busy */

/* cardbus funcevnt, funcevntmask, funcstate, and funcforce */
#define	F_RD		((uint32)1 << 1)	/* card function ready */
#define	F_GW		((uint32)1 << 4)	/* general wakeup */
#define	F_WU		((uint32)1 << 14)	/* wakeup mask */
#define	F_IR		((uint32)1 << 15)	/* interrupt request pending */

/* transmit dma channel control */
#define	XC_XE		((uint32)1 << 0)	/* transmit enable */
#define	XC_LE		((uint32)1 << 2)	/* dma loopback enable */

/* transmit descriptor table pointer */
#define	XP_LD_MASK	0xfff			/* last valid descriptor */

/* transmit channel status */
#define	XS_CD_MASK	0x0fff			/* current descriptor pointer */
#define	XS_XS_MASK	0xf000			/* transmit state */
#define	XS_XS_SHIFT	12
#define	XS_XS_DISABLED	0x0000			/* disabled */
#define	XS_XS_ACTIVE	0x1000			/* active */
#define	XS_XS_IDLE	0x2000			/* idle wait */
#define	XS_XS_STOPPED	0x3000			/* stopped */
#define	XS_XS_SUSP	0x4000			/* suspended */
#define	XS_XE_MASK	0xf0000			/* transmit errors */
#define	XS_XE_SHIFT	16
#define	XS_XE_NOERR	0x00000			/* no error */
#define	XS_XE_DPE	0x10000			/* descriptor protocol error */
#define	XS_XE_DFU	0x20000			/* data fifo underrun */
#define	XS_XE_PCIBR	0x30000			/* pci bus error on buffer read */
#define	XS_XE_PCIDA	0x40000			/* pci bus error on descriptor access */

/* receive dma channel control */
#define	RC_RE		((uint32)1 << 0)	/* receive enable */

/* receive descriptor table pointer */
#define	RP_LD_MASK	0xfff			/* last valid descriptor */

/* receive channel status */
#define	RS_CD_MASK	0x0fff			/* current descriptor pointer */
#define	RS_RS_MASK	0xf000			/* receive state */
#define	RS_RS_SHIFT	12
#define	RS_RS_DISABLED	0x0000			/* disabled */
#define	RS_RS_ACTIVE	0x1000			/* active */
#define	RS_RS_IDLE	0x2000			/* idle wait */
#define	RS_RS_STOPPED	0x3000			/* reserved */
#define	RS_RE_MASK	0xf0000			/* receive errors */
#define	RS_RE_SHIFT	16
#define	RS_RE_NOERR	0x00000			/* no error */
#define	RS_RE_DPE	0x10000			/* descriptor protocol error */
#define	RS_RE_DFO	0x20000			/* data fifo overflow */
#define	RS_RE_PCIBW	0x30000			/* pci bus error on buffer write */
#define	RS_RE_PCIDA	0x40000			/* pci bus error on descriptor access */

/*
 * Transmit/Receive Ring Descriptor
 */
typedef volatile struct {
	uint32	ctrl;		/* misc control bits & bufcount */
	uint32	addr;		/* data buffer address */
} bcmcodecdd_t;

#define	CTRL_BC_MASK	0x1fff			/* buffer byte count */
#define	CTRL_EOT	((uint32)1 << 28)	/* end of descriptor table */
#define	CTRL_IOC	((uint32)1 << 29)	/* interrupt on completion */
#define	CTRL_EOF	((uint32)1 << 30)	/* end of frame */
#define	CTRL_SOF	((uint32)1 << 31)	/* start of frame */

/*
 * Si3034 codec chip registers.
 */
typedef volatile struct {
	uint8	ctl1;
	uint8	ctl2;
	uint8	resv1;
	uint8	resv2;
	uint8	daactl1;
	uint8	daactl2;
	uint8	plldivn1;
	uint8	plldivm1;
	uint8	pll2divmult;
	uint8	pllctl;
	uint8	chiparev;
	uint8	linestatus;
	uint8	chipbrev;
	uint8	daisyctl;
	uint8	txrxgainctl;
	uint8	inationalctl1;
	uint8	inationalctl2;
	uint8	inationalctl3;
	uint8	inationalctl4;
} si3034regs_t;

#endif	/* _BCMCODEC_H */
