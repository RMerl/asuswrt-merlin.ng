/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Interrupt Controller Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __INTCTRL_H__
#define __INTCTRL_H__

#if defined(CONFIG_M5235) || defined(CONFIG_M5271) || \
    defined(CONFIG_M5275) || defined(CONFIG_M5282) || \
    defined(CONFIG_M547x) || defined(CONFIG_M548x)
#	define	CONFIG_SYS_CF_INTC_REG1
#endif

typedef struct int0_ctrl {
	/* Interrupt Controller 0 */
	u32 iprh0;		/* 0x00 Pending High */
	u32 iprl0;		/* 0x04 Pending Low */
	u32 imrh0;		/* 0x08 Mask High */
	u32 imrl0;		/* 0x0C Mask Low */
	u32 frch0;		/* 0x10 Force High */
	u32 frcl0;		/* 0x14 Force Low */
#if defined(CONFIG_SYS_CF_INTC_REG1)
	u8 irlr;		/* 0x18 */
	u8 iacklpr;		/* 0x19 */
	u16 res1[19];		/* 0x1a - 0x3c */
#else
	u16 res1;		/* 0x18 - 0x19 */
	u16 icfg0;		/* 0x1A Configuration */
	u8 simr0;		/* 0x1C Set Interrupt Mask */
	u8 cimr0;		/* 0x1D Clear Interrupt Mask */
	u8 clmask0;		/* 0x1E Current Level Mask */
	u8 slmask;		/* 0x1F Saved Level Mask */
	u32 res2[8];		/* 0x20 - 0x3F */
#endif
	u8 icr0[64];		/* 0x40 - 0x7F Control registers */
	u32 res3[24];		/* 0x80 - 0xDF */
	u8 swiack0;		/* 0xE0 Software Interrupt ack */
	u8 res4[3];		/* 0xE1 - 0xE3 */
	u8 L1iack0;		/* 0xE4 Level n interrupt ack */
	u8 res5[3];		/* 0xE5 - 0xE7 */
	u8 L2iack0;		/* 0xE8 Level n interrupt ack */
	u8 res6[3];		/* 0xE9 - 0xEB */
	u8 L3iack0;		/* 0xEC Level n interrupt ack */
	u8 res7[3];		/* 0xED - 0xEF */
	u8 L4iack0;		/* 0xF0 Level n interrupt ack */
	u8 res8[3];		/* 0xF1 - 0xF3 */
	u8 L5iack0;		/* 0xF4 Level n interrupt ack */
	u8 res9[3];		/* 0xF5 - 0xF7 */
	u8 L6iack0;		/* 0xF8 Level n interrupt ack */
	u8 resa[3];		/* 0xF9 - 0xFB */
	u8 L7iack0;		/* 0xFC Level n interrupt ack */
	u8 resb[3];		/* 0xFD - 0xFF */
} int0_t;

typedef struct int1_ctrl {
	/* Interrupt Controller 1 */
	u32 iprh1;		/* 0x00 Pending High */
	u32 iprl1;		/* 0x04 Pending Low */
	u32 imrh1;		/* 0x08 Mask High */
	u32 imrl1;		/* 0x0C Mask Low */
	u32 frch1;		/* 0x10 Force High */
	u32 frcl1;		/* 0x14 Force Low */
#if defined(CONFIG_SYS_CF_INTC_REG1)
	u8 irlr;		/* 0x18 */
	u8 iacklpr;		/* 0x19 */
	u16 res1[19];		/* 0x1a - 0x3c */
#else
	u16 res1;		/* 0x18 */
	u16 icfg1;		/* 0x1A Configuration */
	u8 simr1;		/* 0x1C Set Interrupt Mask */
	u8 cimr1;		/* 0x1D Clear Interrupt Mask */
	u16 res2;		/* 0x1E - 0x1F */
	u32 res3[8];		/* 0x20 - 0x3F */
#endif
	u8 icr1[64];		/* 0x40 - 0x7F */
	u32 res4[24];		/* 0x80 - 0xDF */
	u8 swiack1;		/* 0xE0 Software Interrupt ack */
	u8 res5[3];		/* 0xE1 - 0xE3 */
	u8 L1iack1;		/* 0xE4 Level n interrupt ack */
	u8 res6[3];		/* 0xE5 - 0xE7 */
	u8 L2iack1;		/* 0xE8 Level n interrupt ack */
	u8 res7[3];		/* 0xE9 - 0xEB */
	u8 L3iack1;		/* 0xEC Level n interrupt ack */
	u8 res8[3];		/* 0xED - 0xEF */
	u8 L4iack1;		/* 0xF0 Level n interrupt ack */
	u8 res9[3];		/* 0xF1 - 0xF3 */
	u8 L5iack1;		/* 0xF4 Level n interrupt ack */
	u8 resa[3];		/* 0xF5 - 0xF7 */
	u8 L6iack1;		/* 0xF8 Level n interrupt ack */
	u8 resb[3];		/* 0xF9 - 0xFB */
	u8 L7iack1;		/* 0xFC Level n interrupt ack */
	u8 resc[3];		/* 0xFD - 0xFF */
} int1_t;

typedef struct intgack_ctrl1 {
	/* Global IACK Registers */
	u8 swiack;		/* 0x00 Global Software Interrupt ack */
	u8 res0[0x3];
	u8 gl1iack;		/* 0x04 */
	u8 resv1[0x3];
	u8 gl2iack;		/* 0x08 */
	u8 res2[0x3];
	u8 gl3iack;		/* 0x0C */
	u8 res3[0x3];
	u8 gl4iack;		/* 0x10 */
	u8 res4[0x3];
	u8 gl5iack;		/* 0x14 */
	u8 res5[0x3];
	u8 gl6iack;		/* 0x18 */
	u8 res6[0x3];
	u8 gl7iack;		/* 0x1C */
	u8 res7[0x3];
} intgack_t;

#define INTC_IPRH_INT63			(0x80000000)
#define INTC_IPRH_INT62			(0x40000000)
#define INTC_IPRH_INT61			(0x20000000)
#define INTC_IPRH_INT60			(0x10000000)
#define INTC_IPRH_INT59			(0x08000000)
#define INTC_IPRH_INT58			(0x04000000)
#define INTC_IPRH_INT57			(0x02000000)
#define INTC_IPRH_INT56			(0x01000000)
#define INTC_IPRH_INT55			(0x00800000)
#define INTC_IPRH_INT54			(0x00400000)
#define INTC_IPRH_INT53			(0x00200000)
#define INTC_IPRH_INT52			(0x00100000)
#define INTC_IPRH_INT51			(0x00080000)
#define INTC_IPRH_INT50			(0x00040000)
#define INTC_IPRH_INT49			(0x00020000)
#define INTC_IPRH_INT48			(0x00010000)
#define INTC_IPRH_INT47			(0x00008000)
#define INTC_IPRH_INT46			(0x00004000)
#define INTC_IPRH_INT45			(0x00002000)
#define INTC_IPRH_INT44			(0x00001000)
#define INTC_IPRH_INT43			(0x00000800)
#define INTC_IPRH_INT42			(0x00000400)
#define INTC_IPRH_INT41			(0x00000200)
#define INTC_IPRH_INT40			(0x00000100)
#define INTC_IPRH_INT39			(0x00000080)
#define INTC_IPRH_INT38			(0x00000040)
#define INTC_IPRH_INT37			(0x00000020)
#define INTC_IPRH_INT36			(0x00000010)
#define INTC_IPRH_INT35			(0x00000008)
#define INTC_IPRH_INT34			(0x00000004)
#define INTC_IPRH_INT33			(0x00000002)
#define INTC_IPRH_INT32			(0x00000001)

#define INTC_IPRL_INT31			(0x80000000)
#define INTC_IPRL_INT30			(0x40000000)
#define INTC_IPRL_INT29			(0x20000000)
#define INTC_IPRL_INT28			(0x10000000)
#define INTC_IPRL_INT27			(0x08000000)
#define INTC_IPRL_INT26			(0x04000000)
#define INTC_IPRL_INT25			(0x02000000)
#define INTC_IPRL_INT24			(0x01000000)
#define INTC_IPRL_INT23			(0x00800000)
#define INTC_IPRL_INT22			(0x00400000)
#define INTC_IPRL_INT21			(0x00200000)
#define INTC_IPRL_INT20			(0x00100000)
#define INTC_IPRL_INT19			(0x00080000)
#define INTC_IPRL_INT18			(0x00040000)
#define INTC_IPRL_INT17			(0x00020000)
#define INTC_IPRL_INT16			(0x00010000)
#define INTC_IPRL_INT15			(0x00008000)
#define INTC_IPRL_INT14			(0x00004000)
#define INTC_IPRL_INT13			(0x00002000)
#define INTC_IPRL_INT12			(0x00001000)
#define INTC_IPRL_INT11			(0x00000800)
#define INTC_IPRL_INT10			(0x00000400)
#define INTC_IPRL_INT9			(0x00000200)
#define INTC_IPRL_INT8			(0x00000100)
#define INTC_IPRL_INT7			(0x00000080)
#define INTC_IPRL_INT6			(0x00000040)
#define INTC_IPRL_INT5			(0x00000020)
#define INTC_IPRL_INT4			(0x00000010)
#define INTC_IPRL_INT3			(0x00000008)
#define INTC_IPRL_INT2			(0x00000004)
#define INTC_IPRL_INT1			(0x00000002)
#define INTC_IPRL_INT0			(0x00000001)

#define INTC_IMRLn_MASKALL		(0x00000001)

#define INTC_IRLR(x)			(((x) & 0x7F) << 1)
#define INTC_IRLR_MASK			(0x01)

#define INTC_IACKLPR_LVL(x)		(((x) & 0x07) << 4)
#define INTC_IACKLPR_LVL_MASK		(0x8F)
#define INTC_IACKLPR_PRI(x)		((x) & 0x0F)
#define INTC_IACKLPR_PRI_MASK		(0xF0)

#if defined(CONFIG_SYS_CF_INTC_REG1)
#define INTC_ICR_IL(x)			(((x) & 0x07) << 3)
#define INTC_ICR_IL_MASK		(0xC7)
#define INTC_ICR_IP(x)			((x) & 0x07)
#define INTC_ICR_IP_MASK		(0xF8)
#else
#define INTC_ICR_IL(x)			((x) & 0x07)
#define INTC_ICR_IL_MASK		(0xF8)
#endif

#define INTC_ICONFIG_ELVLPRI_MASK	(0x01FF)
#define INTC_ICONFIG_ELVLPRI7		(0x8000)
#define INTC_ICONFIG_ELVLPRI6		(0x4000)
#define INTC_ICONFIG_ELVLPRI5		(0x2000)
#define INTC_ICONFIG_ELVLPRI4		(0x1000)
#define INTC_ICONFIG_ELVLPRI3		(0x0800)
#define INTC_ICONFIG_ELVLPRI2		(0x0400)
#define INTC_ICONFIG_ELVLPRI1		(0x0200)
#define INTC_ICONFIG_EMASK		(0x0020)

#define INTC_SIMR_ALL			(0x40)
#define INTC_SIMR(x)			((x) & 0x3F)
#define INTC_SIMR_MASK			(0x80)

#define INTC_CIMR_ALL			(0x40)
#define INTC_CIMR(x)			((x) & 0x3F)
#define INTC_CIMR_MASK			(0x80)

#define INTC_CLMASK(x)			((x) & 0x0F)
#define INTC_CLMASK_MASK		(0xF0)

#define INTC_SLMASK(x)			((x) & 0x0F)
#define INTC_SLMASK_MASK		(0xF0)

#endif				/* __INTCTRL_H__ */
