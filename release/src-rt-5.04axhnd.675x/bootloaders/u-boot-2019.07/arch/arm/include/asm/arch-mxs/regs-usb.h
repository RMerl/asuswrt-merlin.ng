/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 USB OTG Register Definitions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef __REGS_USB_H__
#define __REGS_USB_H__

struct mxs_usb_regs {
	uint32_t		hw_usbctrl_id;			/* 0x000 */
	uint32_t		hw_usbctrl_hwgeneral;		/* 0x004 */
	uint32_t		hw_usbctrl_hwhost;		/* 0x008 */
	uint32_t		hw_usbctrl_hwdevice;		/* 0x00c */
	uint32_t		hw_usbctrl_hwtxbuf;		/* 0x010 */
	uint32_t		hw_usbctrl_hwrxbuf;		/* 0x014 */

	uint32_t		reserved1[26];

	uint32_t		hw_usbctrl_gptimer0ld;		/* 0x080 */
	uint32_t		hw_usbctrl_gptimer0ctrl;	/* 0x084 */
	uint32_t		hw_usbctrl_gptimer1ld;		/* 0x088 */
	uint32_t		hw_usbctrl_gptimer1ctrl;	/* 0x08c */
	uint32_t		hw_usbctrl_sbuscfg;		/* 0x090 */

	uint32_t		reserved2[27];

	uint32_t		hw_usbctrl_caplength;		/* 0x100 */
	uint32_t		hw_usbctrl_hcsparams;		/* 0x104 */
	uint32_t		hw_usbctrl_hccparams;		/* 0x108 */

	uint32_t		reserved3[5];

	uint32_t		hw_usbctrl_dciversion;		/* 0x120 */
	uint32_t		hw_usbctrl_dccparams;		/* 0x124 */

	uint32_t		reserved4[6];

	uint32_t		hw_usbctrl_usbcmd;		/* 0x140 */
	uint32_t		hw_usbctrl_usbsts;		/* 0x144 */
	uint32_t		hw_usbctrl_usbintr;		/* 0x148 */
	uint32_t		hw_usbctrl_frindex;		/* 0x14c */

	uint32_t		reserved5;

	union {
		uint32_t	hw_usbctrl_periodiclistbase;	/* 0x154 */
		uint32_t	hw_usbctrl_deviceaddr;		/* 0x154 */
	};
	union {
		uint32_t	hw_usbctrl_asynclistaddr;	/* 0x158 */
		uint32_t	hw_usbctrl_endpointlistaddr;	/* 0x158 */
	};

	uint32_t		hw_usbctrl_ttctrl;		/* 0x15c */
	uint32_t		hw_usbctrl_burstsize;		/* 0x160 */
	uint32_t		hw_usbctrl_txfilltuning;	/* 0x164 */

	uint32_t		reserved6;

	uint32_t		hw_usbctrl_ic_usb;		/* 0x16c */
	uint32_t		hw_usbctrl_ulpi;		/* 0x170 */

	uint32_t		reserved7;

	uint32_t		hw_usbctrl_endptnak;		/* 0x178 */
	uint32_t		hw_usbctrl_endptnaken;		/* 0x17c */

	uint32_t		reserved8;

	uint32_t		hw_usbctrl_portsc1;		/* 0x184 */

	uint32_t		reserved9[7];

	uint32_t		hw_usbctrl_otgsc;		/* 0x1a4 */
	uint32_t		hw_usbctrl_usbmode;		/* 0x1a8 */
	uint32_t		hw_usbctrl_endptsetupstat;	/* 0x1ac */
	uint32_t		hw_usbctrl_endptprime;		/* 0x1b0 */
	uint32_t		hw_usbctrl_endptflush;		/* 0x1b4 */
	uint32_t		hw_usbctrl_endptstat;		/* 0x1b8 */
	uint32_t		hw_usbctrl_endptcomplete;	/* 0x1bc */
	uint32_t		hw_usbctrl_endptctrl0;		/* 0x1c0 */
	uint32_t		hw_usbctrl_endptctrl1;		/* 0x1c4 */
	uint32_t		hw_usbctrl_endptctrl2;		/* 0x1c8 */
	uint32_t		hw_usbctrl_endptctrl3;		/* 0x1cc */
	uint32_t		hw_usbctrl_endptctrl4;		/* 0x1d0 */
	uint32_t		hw_usbctrl_endptctrl5;		/* 0x1d4 */
	uint32_t		hw_usbctrl_endptctrl6;		/* 0x1d8 */
	uint32_t		hw_usbctrl_endptctrl7;		/* 0x1dc */
};

#define	CLKCTRL_PLL0CTRL0_LFR_SEL_MASK		(0x3 << 28)

#define	HW_USBCTRL_ID_CIVERSION_OFFSET		29
#define	HW_USBCTRL_ID_CIVERSION_MASK		(0x7 << 29)
#define	HW_USBCTRL_ID_VERSION_OFFSET		25
#define	HW_USBCTRL_ID_VERSION_MASK		(0xf << 25)
#define	HW_USBCTRL_ID_REVISION_OFFSET		21
#define	HW_USBCTRL_ID_REVISION_MASK		(0xf << 21)
#define	HW_USBCTRL_ID_TAG_OFFSET		16
#define	HW_USBCTRL_ID_TAG_MASK			(0x1f << 16)
#define	HW_USBCTRL_ID_NID_OFFSET		8
#define	HW_USBCTRL_ID_NID_MASK			(0x3f << 8)
#define	HW_USBCTRL_ID_ID_OFFSET			0
#define	HW_USBCTRL_ID_ID_MASK			(0x3f << 0)

#define	HW_USBCTRL_HWGENERAL_SM_OFFSET		9
#define	HW_USBCTRL_HWGENERAL_SM_MASK		(0x3 << 9)
#define	HW_USBCTRL_HWGENERAL_PHYM_OFFSET	6
#define	HW_USBCTRL_HWGENERAL_PHYM_MASK		(0x7 << 6)
#define	HW_USBCTRL_HWGENERAL_PHYW_OFFSET	4
#define	HW_USBCTRL_HWGENERAL_PHYW_MASK		(0x3 << 4)
#define	HW_USBCTRL_HWGENERAL_BWT		(1 << 3)
#define	HW_USBCTRL_HWGENERAL_CLKC_OFFSET	1
#define	HW_USBCTRL_HWGENERAL_CLKC_MASK		(0x3 << 1)
#define	HW_USBCTRL_HWGENERAL_RT			(1 << 0)

#define	HW_USBCTRL_HWHOST_TTPER_OFFSET		24
#define	HW_USBCTRL_HWHOST_TTPER_MASK		(0xff << 24)
#define	HW_USBCTRL_HWHOST_TTASY_OFFSET		16
#define	HW_USBCTRL_HWHOST_TTASY_MASK		(0xff << 19)
#define	HW_USBCTRL_HWHOST_NPORT_OFFSET		1
#define	HW_USBCTRL_HWHOST_NPORT_MASK		(0x7 << 1)
#define	HW_USBCTRL_HWHOST_HC			(1 << 0)

#define	HW_USBCTRL_HWDEVICE_DEVEP_OFFSET	1
#define	HW_USBCTRL_HWDEVICE_DEVEP_MASK		(0x1f << 1)
#define	HW_USBCTRL_HWDEVICE_DC			(1 << 0)

#define	HW_USBCTRL_HWTXBUF_TXLCR		(1 << 31)
#define	HW_USBCTRL_HWTXBUF_TXCHANADD_OFFSET	16
#define	HW_USBCTRL_HWTXBUF_TXCHANADD_MASK	(0xff << 16)
#define	HW_USBCTRL_HWTXBUF_TXADD_OFFSET		8
#define	HW_USBCTRL_HWTXBUF_TXADD_MASK		(0xff << 8)
#define	HW_USBCTRL_HWTXBUF_TXBURST_OFFSET	0
#define	HW_USBCTRL_HWTXBUF_TXBURST_MASK		0xff

#define	HW_USBCTRL_HWRXBUF_RXADD_OFFSET		8
#define	HW_USBCTRL_HWRXBUF_RXADD_MASK		(0xff << 8)
#define	HW_USBCTRL_HWRXBUF_RXBURST_OFFSET	0
#define	HW_USBCTRL_HWRXBUF_RXBURST_MASK		0xff

#define	HW_USBCTRL_GPTIMERLD_GPTLD_OFFSET	0
#define	HW_USBCTRL_GPTIMERLD_GPTLD_MASK		0xffffff

#define	HW_USBCTRL_GPTIMERCTRL_GPTRUN		(1 << 31)
#define	HW_USBCTRL_GPTIMERCTRL_GPTRST		(1 << 30)
#define	HW_USBCTRL_GPTIMERCTRL_GPTMODE		(1 << 24)
#define	HW_USBCTRL_GPTIMERCTRL_GPTCNT_OFFSET	0
#define	HW_USBCTRL_GPTIMERCTRL_GPTCNT_MASK	0xffffff

#define	HW_USBCTRL_SBUSCFG_AHBBURST_OFFSET	0
#define	HW_USBCTRL_SBUSCFG_AHBBURST_MASK	0x7
#define	HW_USBCTRL_SBUSCFG_AHBBURST_U_INCR	0x0
#define	HW_USBCTRL_SBUSCFG_AHBBURST_S_INCR4	0x1
#define	HW_USBCTRL_SBUSCFG_AHBBURST_S_INCR8	0x2
#define	HW_USBCTRL_SBUSCFG_AHBBURST_S_INCR16	0x3
#define	HW_USBCTRL_SBUSCFG_AHBBURST_U_INCR4	0x5
#define	HW_USBCTRL_SBUSCFG_AHBBURST_U_INCR8	0x6
#define	HW_USBCTRL_SBUSCFG_AHBBURST_U_INCR16	0x7

#endif	/* __REGS_USB_H__ */
