/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2005, 2009 Freescale Semiconductor, Inc
 * Copyright (c) 2005 MontaVista Software
 * Copyright (c) 2008 Excito Elektronik i Sk=E5ne AB
 */

#ifndef _EHCI_CI_H
#define _EHCI_CI_H

#include <asm/processor.h>

#define CONTROL_REGISTER_W1C_MASK       0x00020000  /* W1C: PHY_CLK_VALID */

/* Global offsets */
#define FSL_SKIP_PCI		0x100

/* offsets for the non-ehci registers in the FSL SOC USB controller */
#define FSL_SOC_USB_ULPIVP	0x170
#define FSL_SOC_USB_PORTSC1	0x184
#define PORT_PTS_MSK		(3 << 30)
#define PORT_PTS_UTMI		(0 << 30)
#define PORT_PTS_ULPI		(2 << 30)
#define PORT_PTS_SERIAL		(3 << 30)
#define PORT_PTS_PTW		(1 << 28)
#define PORT_PTS_HSIC		(1 << 25)
#define PORT_PFSC		(1 << 24) /* Defined on Page 39-44 of the mpc5151 ERM */
#define PORT_PTS_PHCD		(1 << 23)
#define PORT_PP			(1 << 12)
#define PORT_PR			(1 << 8)

/* USBMODE Register bits */
#define CM_IDLE			(0 << 0)
#define CM_RESERVED		(1 << 0)
#define CM_DEVICE		(2 << 0)
#define CM_HOST			(3 << 0)
#define ES_BE			(1 << 2)	/* Big Endian Select, default is LE */
#define USBMODE_RESERVED_2	(0 << 2)
#define SLOM			(1 << 3)
#define SDIS			(1 << 4)

/* CONTROL Register bits */
#define ULPI_INT_EN		(1 << 0)
#define WU_INT_EN		(1 << 1)
#define USB_EN			(1 << 2)
#define LSF_EN			(1 << 3)
#define KEEP_OTG_ON		(1 << 4)
#define OTG_PORT		(1 << 5)
#define REFSEL_12MHZ		(0 << 6)
#define REFSEL_16MHZ		(1 << 6)
#define REFSEL_48MHZ		(2 << 6)
#define PLL_RESET		(1 << 8)
#define UTMI_PHY_EN		(1 << 9)
#define PHY_CLK_SEL_UTMI	(0 << 10)
#define PHY_CLK_SEL_ULPI	(1 << 10)
#define CLKIN_SEL_USB_CLK	(0 << 11)
#define CLKIN_SEL_USB_CLK2	(1 << 11)
#define CLKIN_SEL_SYS_CLK	(2 << 11)
#define CLKIN_SEL_SYS_CLK2	(3 << 11)
#define RESERVED_18		(0 << 13)
#define RESERVED_17		(0 << 14)
#define RESERVED_16		(0 << 15)
#define WU_INT			(1 << 16)
#define PHY_CLK_VALID		(1 << 17)

#define FSL_SOC_USB_PORTSC2	0x188

/* OTG Status Control Register bits */
#define FSL_SOC_USB_OTGSC	0x1a4
#define CTRL_VBUS_DISCHARGE	(0x1<<0)
#define CTRL_VBUS_CHARGE	(0x1<<1)
#define CTRL_OTG_TERMINATION	(0x1<<3)
#define CTRL_DATA_PULSING	(0x1<<4)
#define CTRL_ID_PULL_EN		(0x1<<5)
#define HA_DATA_PULSE		(0x1<<6)
#define HA_BA			(0x1<<7)
#define STS_USB_ID		(0x1<<8)
#define STS_A_VBUS_VALID	(0x1<<9)
#define STS_A_SESSION_VALID	(0x1<<10)
#define STS_B_SESSION_VALID	(0x1<<11)
#define STS_B_SESSION_END	(0x1<<12)
#define STS_1MS_TOGGLE		(0x1<<13)
#define STS_DATA_PULSING	(0x1<<14)
#define INTSTS_USB_ID		(0x1<<16)
#define INTSTS_A_VBUS_VALID	(0x1<<17)
#define INTSTS_A_SESSION_VALID	(0x1<<18)
#define INTSTS_B_SESSION_VALID	(0x1<<19)
#define INTSTS_B_SESSION_END	(0x1<<20)
#define INTSTS_1MS		(0x1<<21)
#define INTSTS_DATA_PULSING	(0x1<<22)
#define INTR_USB_ID_EN		(0x1<<24)
#define INTR_A_VBUS_VALID_EN	(0x1<<25)
#define INTR_A_SESSION_VALID_EN (0x1<<26)
#define INTR_B_SESSION_VALID_EN (0x1<<27)
#define INTR_B_SESSION_END_EN	(0x1<<28)
#define INTR_1MS_TIMER_EN	(0x1<<29)
#define INTR_DATA_PULSING_EN	(0x1<<30)
#define INTSTS_MASK		(0x00ff0000)

#define  INTERRUPT_ENABLE_BITS_MASK  \
		(INTR_USB_ID_EN		| \
		INTR_1MS_TIMER_EN	| \
		INTR_A_VBUS_VALID_EN	| \
		INTR_A_SESSION_VALID_EN | \
		INTR_B_SESSION_VALID_EN | \
		INTR_B_SESSION_END_EN	| \
		INTR_DATA_PULSING_EN)

#define  INTERRUPT_STATUS_BITS_MASK  \
		(INTSTS_USB_ID		| \
		INTR_1MS_TIMER_EN	| \
		INTSTS_A_VBUS_VALID	| \
		INTSTS_A_SESSION_VALID  | \
		INTSTS_B_SESSION_VALID  | \
		INTSTS_B_SESSION_END	| \
		INTSTS_DATA_PULSING)

#define FSL_SOC_USB_USBMODE	0x1a8

#define USBGENCTRL		0x200		/* NOTE: big endian */
#define GC_WU_INT_CLR		(1 << 5)	/* Wakeup int clear */
#define GC_ULPI_SEL		(1 << 4)	/* ULPI i/f select (usb0 only)*/
#define GC_PPP			(1 << 3)	/* Port Power Polarity */
#define GC_PFP			(1 << 2)	/* Power Fault Polarity */
#define GC_WU_ULPI_EN		(1 << 1)	/* Wakeup on ULPI event */
#define GC_WU_IE		(1 << 1)	/* Wakeup interrupt enable */

#define ISIPHYCTRL		0x204		/* NOTE: big endian */
#define PHYCTRL_PHYE		(1 << 4)	/* On-chip UTMI PHY enable */
#define PHYCTRL_BSENH		(1 << 3)	/* Bit Stuff Enable High */
#define PHYCTRL_BSEN		(1 << 2)	/* Bit Stuff Enable */
#define PHYCTRL_LSFE		(1 << 1)	/* Line State Filter Enable */
#define PHYCTRL_PXE		(1 << 0)	/* PHY oscillator enable */

#define FSL_SOC_USB_SNOOP1	0x400	/* NOTE: big-endian */
#define FSL_SOC_USB_SNOOP2	0x404	/* NOTE: big-endian */
#define FSL_SOC_USB_AGECNTTHRSH	0x408	/* NOTE: big-endian */
#define FSL_SOC_USB_PRICTRL	0x40c	/* NOTE: big-endian */
#define FSL_SOC_USB_SICTRL	0x410	/* NOTE: big-endian */
#define FSL_SOC_USB_CTRL	0x500	/* NOTE: big-endian */
#define SNOOP_SIZE_2GB		0x1e

/* System Clock Control Register */
#define MPC83XX_SCCR_USB_MASK		0x00f00000
#define MPC83XX_SCCR_USB_DRCM_11	0x00300000
#define MPC83XX_SCCR_USB_DRCM_01	0x00100000
#define MPC83XX_SCCR_USB_DRCM_10	0x00200000

/*
 * Increasing TX FIFO threshold value from 2 to 4 decreases
 * data burst rate with which data packets are posted from the TX
 * latency FIFO to compensate for latencies in DDR pipeline during DMA
 */
#define TXFIFOTHRESH		4

/*
 * USB Registers
 */
struct usb_ehci {
	u32	id;		/* 0x000 - Identification register */
	u32	hwgeneral;	/* 0x004 - General hardware parameters */
	u32	hwhost;		/* 0x008 - Host hardware parameters */
	u32	hwdevice;	/* 0x00C - Device hardware parameters  */
	u32	hwtxbuf;	/* 0x010 - TX buffer hardware parameters */
	u32	hwrxbuf;	/* 0x014 - RX buffer hardware parameters */
	u8	res1[0x68];
	u32	gptimer0_ld;	/* 0x080 - General Purpose Timer 0 load value */
	u32	gptimer0_ctrl;	/* 0x084 - General Purpose Timer 0 control */
	u32     gptimer1_ld;	/* 0x088 - General Purpose Timer 1 load value */
	u32     gptimer1_ctrl;	/* 0x08C - General Purpose Timer 1 control */
	u32	sbuscfg;	/* 0x090 - System Bus Interface Control */
	u32	sbusstatus;	/* 0x094 - System Bus Interface Status */
	u32	sbusmode;	/* 0x098 - System Bus Interface Mode */
	u32	genconfig;	/* 0x09C - USB Core Configuration */
	u32	genconfig2;	/* 0x0A0 - USB Core Configuration 2 */
	u8	res2[0x5c];
	u8	caplength;	/* 0x100 - Capability Register Length */
	u8	res3[0x1];
	u16	hciversion;	/* 0x102 - Host Interface Version */
	u32	hcsparams;	/* 0x104 - Host Structural Parameters */
	u32	hccparams;	/* 0x108 - Host Capability Parameters */
	u8	res4[0x14];
	u32	dciversion;	/* 0x120 - Device Interface Version */
	u32	dciparams;	/* 0x124 - Device Controller Params */
	u8	res5[0x18];
	u32	usbcmd;		/* 0x140 - USB Command */
	u32	usbsts;		/* 0x144 - USB Status */
	u32	usbintr;	/* 0x148 - USB Interrupt Enable */
	u32	frindex;	/* 0x14C - USB Frame Index */
	u8	res6[0x4];
	u32	perlistbase;	/* 0x154 - Periodic List Base
					 - USB Device Address */
	u32	ep_list_addr;	/* 0x158 - Next Asynchronous List
					 - End Point Address */
	u8	res7[0x4];
	u32	burstsize;	/* 0x160 - Programmable Burst Size */
#define FSL_EHCI_TXPBURST(X)	((X) << 8)
#define FSL_EHCI_RXPBURST(X)	(X)
	u32	txfilltuning;	/* 0x164 - Host TT Transmit
					   pre-buffer packet tuning */
	u8	res8[0x8];
	u32	ulpi_viewpoint;	/* 0x170 - ULPI Reister Access */
	u8	res9[0xc];
	u32	config_flag;	/* 0x180 - Configured Flag Register */
	u32	portsc;		/* 0x184 - Port status/control */
	u8	res10[0x1C];
	u32	otgsc;		/* 0x1a4 - Oo-The-Go status and control */
	u32	usbmode;	/* 0x1a8 - USB Device Mode */
	u32	epsetupstat;	/* 0x1ac - End Point Setup Status */
	u32	epprime;	/* 0x1b0 - End Point Init Status */
	u32	epflush;	/* 0x1b4 - End Point De-initlialize */
	u32	epstatus;	/* 0x1b8 - End Point Status */
	u32	epcomplete;	/* 0x1bc - End Point Complete */
	u32	epctrl0;	/* 0x1c0 - End Point Control 0 */
	u32	epctrl1;	/* 0x1c4 - End Point Control 1 */
	u32	epctrl2;	/* 0x1c8 - End Point Control 2 */
	u32	epctrl3;	/* 0x1cc - End Point Control 3 */
	u32	epctrl4;	/* 0x1d0 - End Point Control 4 */
	u32	epctrl5;	/* 0x1d4 - End Point Control 5 */
	u8	res11[0x28];
	u32	usbgenctrl;	/* 0x200 - USB General Control */
	u32	isiphyctrl;	/* 0x204 - On-Chip PHY Control */
	u8	res12[0x1F8];
	u32	snoop1;		/* 0x400 - Snoop 1 */
	u32	snoop2;		/* 0x404 - Snoop 2 */
	u32	age_cnt_limit;	/* 0x408 - Age Count Threshold */
	u32	prictrl;	/* 0x40c - Priority Control */
	u32	sictrl;		/* 0x410 - System Interface Control */
	u8	res13[0xEC];
	u32	control;	/* 0x500 - Control */
	u8	res14[0xafc];
};

/*
 * For MXC SOCs
 */

/* values for flags field */
#define MXC_EHCI_INTERFACE_DIFF_UNI	(0 << 0)
#define MXC_EHCI_INTERFACE_DIFF_BI	(1 << 0)
#define MXC_EHCI_INTERFACE_SINGLE_UNI	(2 << 0)
#define MXC_EHCI_INTERFACE_SINGLE_BI	(3 << 0)
#define MXC_EHCI_INTERFACE_MASK		(0xf)

#define MXC_EHCI_POWER_PINS_ENABLED	(1 << 5)
#define MXC_EHCI_PWR_PIN_ACTIVE_HIGH	(1 << 6)
#define MXC_EHCI_OC_PIN_ACTIVE_LOW	(1 << 7)
#define MXC_EHCI_TTL_ENABLED		(1 << 8)

#define MXC_EHCI_INTERNAL_PHY		(1 << 9)
#define MXC_EHCI_IPPUE_DOWN		(1 << 10)
#define MXC_EHCI_IPPUE_UP		(1 << 11)

int usb_phy_mode(int port);
/* Board-specific initialization */
int board_ehci_hcd_init(int port);
int board_ehci_power(int port, int on);
int board_usb_phy_mode(int port);

#endif /* _EHCI_CI_H */
