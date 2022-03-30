/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Message Digest Hardware Accelerator Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __MDHA_H__
#define __MDHA_H__

/* Message Digest Hardware Accelerator */
typedef struct mdha_ctrl {
	u32 mr;			/* 0x00 MDHA Mode */
	u32 cr;			/* 0x04 Control */
	u32 cmd;		/* 0x08 Command */
	u32 sr;			/* 0x0C Status */
	u32 isr;		/* 0x10 Interrupt Status */
	u32 imr;		/* 0x14 Interrupt Mask */
	u32 dsz;		/* 0x1C Data Size */
	u32 inp;		/* 0x20 Input FIFO */
	u32 res1[3];		/* 0x24 - 0x2F */
	u32 mda0;		/* 0x30 Message Digest AO */
	u32 mdb0;		/* 0x34 Message Digest BO */
	u32 mdc0;		/* 0x38 Message Digest CO */
	u32 mdd0;		/* 0x3C Message Digest DO */
	u32 mde0;		/* 0x40 Message Digest EO */
	u32 mdsz;		/* 0x44 Message Data Size */
	u32 res[10];		/* 0x48 - 0x6F */
	u32 mda1;		/* 0x70 Message Digest A1 */
	u32 mdb1;		/* 0x74 Message Digest B1 */
	u32 mdc1;		/* 0x78 Message Digest C1 */
	u32 mdd1;		/* 0x7C Message Digest D1 */
	u32 mde1;		/* 0x80 Message Digest E1 */
} mdha_t;

#define MDHA_MR_SSL		(0x00000400)
#define MDHA_MR_MACFUL		(0x00000200)
#define MDHA_MR_SWAP		(0x00000100)
#define MDHA_MR_OPAD		(0x00000080)
#define MDHA_MR_IPAD		(0x00000040)
#define MDHA_MR_INIT		(0x00000020)
#define MDHA_MR_MAC(x)		(((x) & 0x03) << 3)
#define MDHA_MR_MAC_MASK	(0xFFFFFFE7)
#define MDHA_MR_MAC_EHMAC	(0x00000010)
#define MDHA_MR_MAC_HMAC	(0x00000008)
#define MDHA_MR_MAC_NONE	(0x00000000)
#define MDHA_MR_PDATA		(0x00000004)
#define MDHA_MR_ALG		(0x00000001)

#define MDHA_CR_DMAL(x)		(((x) & 0x1F) << 16)	/* 532x */
#define MDHA_CR_DMAL_MASK	(0xFFE0FFFF)		/* 532x */
#define MDHA_CR_END		(0x00000004)		/* 532x */
#define MDHA_CR_DMA		(0x00000002)		/* 532x */
#define MDHA_CR_IE		(0x00000001)

#define MDHA_CMD_GO		(0x00000008)
#define MDHA_CMD_CI		(0x00000004)
#define MDHA_CMD_RI		(0x00000001)
#define MDHA_CMD_SWR		(0x00000001)

#define MDHA_SR_IFL(x)		(((x) & 0xFF) << 16)
#define MDHA_SR_IFL_MASK	(0xFF00FFFF)
#define MDHA_SR_APD(x)		(((x) & 0x7) << 13)
#define MDHA_SR_APD_MASK	(0xFFFF1FFF)
#define MDHA_SR_FS(x)		(((x) & 0x7) << 8)
#define MDHA_SR_FS_MASK		(0xFFFFF8FF)
#define MDHA_SR_GNW		(0x00000080)
#define MDHA_SR_HSH		(0x00000040)
#define MDHA_SR_BUSY		(0x00000010)
#define MDHA_SR_RD		(0x00000008)
#define MDHA_SR_ERR		(0x00000004)
#define MDHA_SR_DONE		(0x00000002)
#define MDHA_SR_INT		(0x00000001)

#define MDHA_ISR_DRL		(0x00000400)		/* 532x */
#define MDHA_ISR_GTDS		(0x00000200)
#define MDHA_ISR_ERE		(0x00000100)
#define MDHA_ISR_RMDP		(0x00000080)
#define MDHA_ISR_DSE		(0x00000020)
#define MDHA_ISR_IME		(0x00000010)
#define MDHA_ISR_NEIF		(0x00000004)
#define MDHA_ISR_IFO		(0x00000001)

#endif				/* __MDHA_H__ */
