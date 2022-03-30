/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Symmetric Key Hardware Accelerator Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __SKHA_H__
#define __SKHA_H__

typedef struct skha_ctrl {
	u32 mr;			/* 0x00 Mode */
	u32 cr;			/* 0x04 Control */
	u32 cmr;		/* 0x08 Command */
	u32 sr;			/* 0x0C Status */
	u32 esr;		/* 0x10 Error Status */
	u32 emr;		/* 0x14 Error Status Mask Register) */
	u32 ksr;		/* 0x18 Key Size */
	u32 dsr;		/* 0x1C Data Size */
	u32 in;			/* 0x20 Input FIFO */
	u32 out;		/* 0x24 Output FIFO */
	u32 res1[2];		/* 0x28 - 0x2F */
	u32 kdr1;		/* 0x30 Key Data 1  */
	u32 kdr2;		/* 0x34 Key Data 2 */
	u32 kdr3;		/* 0x38 Key Data 3 */
	u32 kdr4;		/* 0x3C Key Data 4 */
	u32 kdr5;		/* 0x40 Key Data 5 */
	u32 kdr6;		/* 0x44 Key Data 6 */
	u32 res2[10];		/* 0x48 - 0x6F */
	u32 c1;			/* 0x70 Context 1 */
	u32 c2;			/* 0x74 Context 2 */
	u32 c3;			/* 0x78 Context 3 */
	u32 c4;			/* 0x7C Context 4 */
	u32 c5;			/* 0x80 Context 5 */
	u32 c6;			/* 0x84 Context 6 */
	u32 c7;			/* 0x88 Context 7 */
	u32 c8;			/* 0x8C Context 8 */
	u32 c9;			/* 0x90 Context 9 */
	u32 c10;		/* 0x94 Context 10 */
	u32 c11;		/* 0x98 Context 11 */
	u32 c12;		/* 0x9C Context 12 - 5235, 5271, 5272 */
} skha_t;

#ifdef CONFIG_MCF532x
#define	SKHA_MODE_CTRM(x)	(((x) & 0x0F) << 9)
#define	SKHA_MODE_CTRM_MASK	(0xFFFFE1FF)
#define	SKHA_MODE_DKP		(0x00000100)
#else
#define	SKHA_MODE_CTRM(x)	(((x) & 0x0F) << 8)
#define	SKHA_MODE_CTRM_MASK	(0xFFFFF0FF)
#define	SKHA_MODE_DKP		(0x00000080)
#endif
#define	SKHA_MODE_CM(x)		(((x) & 0x03) << 3)
#define	SKHA_MODE_CM_MASK	(0xFFFFFFE7)
#define	SKHA_MODE_DIR		(0x00000004)
#define	SKHA_MODE_ALG(x)	((x) & 0x03)
#define	SKHA_MODE_ALG_MASK	(0xFFFFFFFC)

#define SHKA_CR_ODMAL(x)	(((x) & 0x3F) << 24)
#define SHKA_CR_ODMAL_MASK	(0xC0FFFFFF)
#define SHKA_CR_IDMAL(x)	(((x) & 0x3F) << 16)
#define SHKA_CR_IDMAL_MASK	(0xFFC0FFFF)
#define SHKA_CR_END		(0x00000008)
#define SHKA_CR_ODMA		(0x00000004)
#define SHKA_CR_IDMA		(0x00000002)
#define	SKHA_CR_IE		(0x00000001)

#define	SKHA_CMR_GO		(0x00000008)
#define	SKHA_CMR_CI		(0x00000004)
#define	SKHA_CMR_RI		(0x00000002)
#define	SKHA_CMR_SWR		(0x00000001)

#define SKHA_SR_OFL(x)		(((x) & 0xFF) << 24)
#define SKHA_SR_OFL_MASK	(0x00FFFFFF)
#define SKHA_SR_IFL(x)		(((x) & 0xFF) << 16)
#define SKHA_SR_IFL_MASK	(0xFF00FFFF)
#define SKHA_SR_AESES(x)	(((x) & 0x1F) << 11)
#define SKHA_SR_AESES_MASK	(0xFFFF07FF)
#define SKHA_SR_DESES(x)	(((x) & 0x7) << 8)
#define SKHA_SR_DESES_MASK	(0xFFFFF8FF)
#define SKHA_SR_BUSY		(0x00000010)
#define SKHA_SR_RD		(0x00000008)
#define SKHA_SR_ERR		(0x00000004)
#define SKHA_SR_DONE		(0x00000002)
#define SKHA_SR_INT		(0x00000001)

#define SHKA_ESE_DRL		(0x00000800)
#define	SKHA_ESR_KRE		(0x00000400)
#define	SKHA_ESR_KPE		(0x00000200)
#define	SKHA_ESR_ERE		(0x00000100)
#define	SKHA_ESR_RMDP		(0x00000080)
#define	SKHA_ESR_KSE		(0x00000040)
#define	SKHA_ESR_DSE		(0x00000020)
#define	SKHA_ESR_IME		(0x00000010)
#define	SKHA_ESR_NEOF		(0x00000008)
#define	SKHA_ESR_NEIF		(0x00000004)
#define	SKHA_ESR_OFU		(0x00000002)
#define	SKHA_ESR_IFO		(0x00000001)

#define	SKHA_KSR_SZ(x)		((x) & 0x3F)
#define	SKHA_KSR_SZ_MASK	(0xFFFFFFC0)

#endif				/* __SKHA_H__ */
