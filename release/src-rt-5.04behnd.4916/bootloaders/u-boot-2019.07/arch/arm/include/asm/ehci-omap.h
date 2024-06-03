/* SPDX-License-Identifier: GPL-2.0 */
/*
 * OMAP EHCI port support
 * Based on LINUX KERNEL
 * drivers/usb/host/ehci-omap.c and drivers/mfd/omap-usb-host.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com*
 * Author: Govindraj R <govindraj.raja@ti.com>
 */

#ifndef _OMAP_COMMON_EHCI_H_
#define _OMAP_COMMON_EHCI_H_

enum usbhs_omap_port_mode {
	OMAP_USBHS_PORT_MODE_UNUSED,
	OMAP_EHCI_PORT_MODE_PHY,
	OMAP_EHCI_PORT_MODE_TLL,
	OMAP_EHCI_PORT_MODE_HSIC,
};

#define OMAP_HS_USB_PORTS	3

#define is_ehci_phy_mode(x)	((x) == OMAP_EHCI_PORT_MODE_PHY)
#define is_ehci_tll_mode(x)	((x) == OMAP_EHCI_PORT_MODE_TLL)
#define is_ehci_hsic_mode(x)	((x) == OMAP_EHCI_PORT_MODE_HSIC)

/* Values of UHH_REVISION - Note: these are not given in the TRM */
#define OMAP_USBHS_REV1					0x00000010 /* OMAP3 */
#define OMAP_USBHS_REV2					0x50700100 /* OMAP4 */
#define OMAP_USBHS_REV2_1				0x50700101 /* OMAP5 */

/* UHH Register Set */
#define OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN		(1 << 2)
#define OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN		(1 << 3)
#define OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN		(1 << 4)
#define OMAP_UHH_HOSTCONFIG_INCRX_ALIGN_EN		(1 << 5)

#define OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS		1
#define OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS		(1 << 11)
#define OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS		(1 << 12)
#define OMAP4_UHH_HOSTCONFIG_APP_START_CLK		(1 << 31)

#define OMAP_P1_MODE_CLEAR				(3 << 16)
#define OMAP_P1_MODE_TLL				(1 << 16)
#define OMAP_P1_MODE_HSIC				(3 << 16)
#define OMAP_P2_MODE_CLEAR				(3 << 18)
#define OMAP_P2_MODE_TLL				(1 << 18)
#define OMAP_P2_MODE_HSIC				(3 << 18)
#define OMAP_P3_MODE_CLEAR				(3 << 20)
#define OMAP_P3_MODE_HSIC				(3 << 20)

/* EHCI Register Set */
#define EHCI_INSNREG04_DISABLE_UNSUSPEND		(1 << 5)
#define EHCI_INSNREG05_ULPI_CONTROL_SHIFT		31
#define EHCI_INSNREG05_ULPI_PORTSEL_SHIFT		24
#define EHCI_INSNREG05_ULPI_OPSEL_SHIFT			22
#define EHCI_INSNREG05_ULPI_REGADD_SHIFT		16

#define OMAP_REV1_TLL_CHANNEL_COUNT			3
#define OMAP_REV2_TLL_CHANNEL_COUNT			2

/* TLL Register Set */
#define OMAP_TLL_CHANNEL_CONF(num)			(0x004 * num)
#define OMAP_TLL_CHANNEL_CONF_DRVVBUS			(1 << 16)
#define OMAP_TLL_CHANNEL_CONF_CHRGVBUS			(1 << 15)
#define OMAP_TLL_CHANNEL_CONF_ULPINOBITSTUFF		(1 << 11)
#define OMAP_TLL_CHANNEL_CONF_CHANMODE_TRANSPARENT_UTMI	(2 << 1)
#define OMAP_TLL_CHANNEL_CONF_CHANEN			1

struct omap_usbhs_board_data {
	enum usbhs_omap_port_mode port_mode[OMAP_HS_USB_PORTS];
};

struct omap_usbtll {
	u32 rev;		/* 0x00 */
	u32 hwinfo;		/* 0x04 */
	u8 reserved1[0x8];
	u32 sysc;		/* 0x10 */
	u32 syss;		/* 0x14 */
	u32 irqst;		/* 0x18 */
	u32 irqen;		/* 0x1c */
	u8 reserved2[0x10];
	u32 shared_conf;	/* 0x30 */
	u8 reserved3[0xc];
	u32 channel_conf;	/* 0x40 */
};

struct omap_uhh {
	u32 rev;	/* 0x00 */
	u32 hwinfo;	/* 0x04 */
	u8 reserved1[0x8];
	u32 sysc;	/* 0x10 */
	u32 syss;	/* 0x14 */
	u8 reserved2[0x28];
	u32 hostconfig;	/* 0x40 */
	u32 debugcsr;	/* 0x44 */
};

struct omap_ehci {
	u32 hccapbase;		/* 0x00 */
	u32 hcsparams;		/* 0x04 */
	u32 hccparams;		/* 0x08 */
	u8 reserved1[0x04];
	u32 usbcmd;		/* 0x10 */
	u32 usbsts;		/* 0x14 */
	u32 usbintr;		/* 0x18 */
	u32 frindex;		/* 0x1c */
	u32 ctrldssegment;	/* 0x20 */
	u32 periodiclistbase;	/* 0x24 */
	u32 asysnclistaddr;	/* 0x28 */
	u8 reserved2[0x24];
	u32 configflag;		/* 0x50 */
	u32 portsc_i;		/* 0x54 */
	u8 reserved3[0x38];
	u32 insreg00;		/* 0x90 */
	u32 insreg01;		/* 0x94 */
	u32 insreg02;		/* 0x98 */
	u32 insreg03;		/* 0x9c */
	u32 insreg04;		/* 0xa0 */
	u32 insreg05_utmi_ulpi;	/* 0xa4 */
	u32 insreg06;		/* 0xa8 */
	u32 insreg07;		/* 0xac */
	u32 insreg08;		/* 0xb0 */
};

/*
 * FIXME: forward declaration of this structs needed because omap got the
 * ehci implementation backwards. move out ehci_hcd_x from board files
 */
struct ehci_hccr;
struct ehci_hcor;

int omap_ehci_hcd_init(int index, struct omap_usbhs_board_data *usbhs_pdata,
		       struct ehci_hccr **hccr, struct ehci_hcor **hcor);
int omap_ehci_hcd_stop(void);

#endif /* _OMAP_COMMON_EHCI_H_ */
