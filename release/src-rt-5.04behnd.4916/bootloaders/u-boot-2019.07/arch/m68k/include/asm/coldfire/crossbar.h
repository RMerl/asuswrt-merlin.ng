/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Cross Bar Switch Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __CROSSBAR_H__
#define __CROSSBAR_H__

/*********************************************************************
* Cross-bar switch (XBS)
*********************************************************************/
typedef struct xbs {
	u32 prs1;		/* 0x100 Priority Register Slave 1 */
	u32 res1[3];		/* 0x104 - 0F */
	u32 crs1;		/* 0x110 Control Register Slave 1 */
	u32 res2[187];		/* 0x114 - 0x3FF */

	u32 prs4;		/* 0x400 Priority Register Slave 4 */
	u32 res3[3];		/* 0x404 - 0F */
	u32 crs4;		/* 0x410 Control Register Slave 4 */
	u32 res4[123];		/* 0x414 - 0x5FF */

	u32 prs6;		/* 0x600 Priority Register Slave 6 */
	u32 res5[3];		/* 0x604 - 0F */
	u32 crs6;		/* 0x610 Control Register Slave 6 */
	u32 res6[59];		/* 0x614 - 0x6FF */

	u32 prs7;		/* 0x700 Priority Register Slave 7 */
	u32 res7[3];		/* 0x704 - 0F */
	u32 crs7;		/* 0x710 Control Register Slave 7 */
} xbs_t;

/* Bit definitions and macros for PRS group */
#define XBS_PRS_M0(x)			(((x)&0x00000007))	/* Core */
#define XBS_PRS_M1(x)			(((x)&0x00000007)<<4)	/* eDMA */
#define XBS_PRS_M2(x)			(((x)&0x00000007)<<8)	/* FEC0 */
#define XBS_PRS_M3(x)			(((x)&0x00000007)<<12)	/* FEC1 */
#define XBS_PRS_M5(x)			(((x)&0x00000007)<<20)	/* PCI controller */
#define XBS_PRS_M6(x)			(((x)&0x00000007)<<24)	/* USB OTG */
#define XBS_PRS_M7(x)			(((x)&0x00000007)<<28)	/* Serial Boot */

/* Bit definitions and macros for CRS group */
#define XBS_CRS_PARK(x)			(((x)&0x00000007))	/* Master parking ctrl */
#define XBS_CRS_PCTL(x)			(((x)&0x00000003)<<4)	/* Parking mode ctrl */
#define XBS_CRS_ARB			(0x00000100)	/* Arbitration Mode */
#define XBS_CRS_RO			(0x80000000)	/* Read Only */

#define XBS_CRS_PCTL_PARK_FIELD		(0)
#define XBS_CRS_PCTL_PARK_ON_LAST	(1)
#define XBS_CRS_PCTL_PARK_NONE		(2)
#define XBS_CRS_PCTL_PARK_CORE		(0)
#define XBS_CRS_PCTL_PARK_EDMA		(1)
#define XBS_CRS_PCTL_PARK_FEC0		(2)
#define XBS_CRS_PCTL_PARK_FEC1		(3)
#define XBS_CRS_PCTL_PARK_PCI		(5)
#define XBS_CRS_PCTL_PARK_USB		(6)
#define XBS_CRS_PCTL_PARK_SBF		(7)

#endif				/* __CROSSBAR_H__ */
