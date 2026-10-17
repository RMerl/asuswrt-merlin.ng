/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ATA Internal Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __ATA_H__
#define __ATA_H__

/* ATA */
typedef struct atac {
	/* PIO */
	u8 toff;		/* 0x00 */
	u8 ton;			/* 0x01 */
	u8 t1;			/* 0x02 */
	u8 t2w;			/* 0x03 */
	u8 t2r;			/* 0x04 */
	u8 ta;			/* 0x05 */
	u8 trd;			/* 0x06 */
	u8 t4;			/* 0x07 */
	u8 t9;			/* 0x08 */

	/* DMA */
	u8 tm;			/* 0x09 */
	u8 tn;			/* 0x0A */
	u8 td;			/* 0x0B */
	u8 tk;			/* 0x0C */
	u8 tack;		/* 0x0D */
	u8 tenv;		/* 0x0E */
	u8 trp;			/* 0x0F */
	u8 tzah;		/* 0x10 */
	u8 tmli;		/* 0x11 */
	u8 tdvh;		/* 0x12 */
	u8 tdzfs;		/* 0x13 */
	u8 tdvs;		/* 0x14 */
	u8 tcvh;		/* 0x15 */
	u8 tss;			/* 0x16 */
	u8 tcyc;		/* 0x17 */

	/* FIFO */
	u32 fifo32;		/* 0x18 */
	u16 fifo16;		/* 0x1C */
	u8 rsvd0[2];
	u8 ffill;		/* 0x20 */
	u8 rsvd1[3];

	/* ATA */
	u8 cr;			/* 0x24 */
	u8 rsvd2[3];
	u8 isr;			/* 0x28 */
	u8 rsvd3[3];
	u8 ier;			/* 0x2C */
	u8 rsvd4[3];
	u8 icr;			/* 0x30 */
	u8 rsvd5[3];
	u8 falarm;		/* 0x34 */
	u8 rsvd6[106];
} atac_t;

#endif				/* __ATA_H__ */
