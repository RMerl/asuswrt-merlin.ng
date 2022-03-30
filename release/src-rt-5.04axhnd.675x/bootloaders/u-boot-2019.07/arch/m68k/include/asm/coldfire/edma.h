/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * EDMA Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __EDMA_H__
#define __EDMA_H__

/*********************************************************************
* Enhanced DMA (EDMA)
*********************************************************************/

/* eDMA module registers */
typedef struct edma_ctrl {
	u32 cr;			/* 0x00 Control Register */
	u32 es;			/* 0x04 Error Status Register */
	u16 res1[3];		/* 0x08 - 0x0D */
	u16 erq;		/* 0x0E Enable Request Register */
	u16 res2[3];		/* 0x10 - 0x15 */
	u16 eei;		/* 0x16 Enable Error Interrupt Request */
	u8 serq;		/* 0x18 Set Enable Request */
	u8 cerq;		/* 0x19 Clear Enable Request */
	u8 seei;		/* 0x1A Set En Error Interrupt Request */
	u8 ceei;		/* 0x1B Clear En Error Interrupt Request */
	u8 cint;		/* 0x1C Clear Interrupt Enable */
	u8 cerr;		/* 0x1D Clear Error */
	u8 ssrt;		/* 0x1E Set START Bit */
	u8 cdne;		/* 0x1F Clear DONE Status Bit */
	u16 res3[3];		/* 0x20 - 0x25 */
	u16 intr;		/* 0x26 Interrupt Request */
	u16 res4[3];		/* 0x28 - 0x2D */
	u16 err;		/* 0x2E Error Register */
	u32 res5[52];		/* 0x30 - 0xFF */
	u8 dchpri0;		/* 0x100 Channel 0 Priority */
	u8 dchpri1;		/* 0x101 Channel 1 Priority */
	u8 dchpri2;		/* 0x102 Channel 2 Priority */
	u8 dchpri3;		/* 0x103 Channel 3 Priority */
	u8 dchpri4;		/* 0x104 Channel 4 Priority */
	u8 dchpri5;		/* 0x105 Channel 5 Priority */
	u8 dchpri6;		/* 0x106 Channel 6 Priority */
	u8 dchpri7;		/* 0x107 Channel 7 Priority */
	u8 dchpri8;		/* 0x108 Channel 8 Priority */
	u8 dchpri9;		/* 0x109 Channel 9 Priority */
	u8 dchpri10;		/* 0x110 Channel 10 Priority */
	u8 dchpri11;		/* 0x111 Channel 11 Priority */
	u8 dchpri12;		/* 0x112 Channel 12 Priority */
	u8 dchpri13;		/* 0x113 Channel 13 Priority */
	u8 dchpri14;		/* 0x114 Channel 14 Priority */
	u8 dchpri15;		/* 0x115 Channel 15 Priority */
} edma_t;

/* TCD - eDMA*/
typedef struct tcd_ctrl {
	u32 saddr;		/* 0x00 Source Address */
	u16 attr;		/* 0x04 Transfer Attributes */
	u16 soff;		/* 0x06 Signed Source Address Offset */
	u32 nbytes;		/* 0x08 Minor Byte Count */
	u32 slast;		/* 0x0C Last Source Address Adjustment */
	u32 daddr;		/* 0x10 Destination address */
	u16 citer;		/* 0x14 Cur Minor Loop Link, Major Loop Cnt */
	u16 doff;		/* 0x16 Signed Destination Address Offset */
	u32 dlast_sga;		/* 0x18 Last Dest Adr Adj/Scatter Gather Adr */
	u16 biter;		/* 0x1C Minor Loop Lnk, Major Loop Cnt */
	u16 csr;		/* 0x1E Control and Status */
} tcd_st;

typedef struct tcd_multiple {
	tcd_st tcd[16];
} tcd_t;

/* Bit definitions and macros for EPPAR */
#define EPORT_EPPAR_EPPA1(x)		(((x)&0x0003)<<2)
#define EPORT_EPPAR_EPPA2(x)		(((x)&0x0003)<<4)
#define EPORT_EPPAR_EPPA3(x)		(((x)&0x0003)<<6)
#define EPORT_EPPAR_EPPA4(x)		(((x)&0x0003)<<8)
#define EPORT_EPPAR_EPPA5(x)		(((x)&0x0003)<<10)
#define EPORT_EPPAR_EPPA6(x)		(((x)&0x0003)<<12)
#define EPORT_EPPAR_EPPA7(x)		(((x)&0x0003)<<14)
#define EPORT_EPPAR_LEVEL		(0)
#define EPORT_EPPAR_RISING		(1)
#define EPORT_EPPAR_FALLING		(2)
#define EPORT_EPPAR_BOTH		(3)
#define EPORT_EPPAR_EPPA7_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA7_RISING	(0x4000)
#define EPORT_EPPAR_EPPA7_FALLING	(0x8000)
#define EPORT_EPPAR_EPPA7_BOTH		(0xC000)
#define EPORT_EPPAR_EPPA6_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA6_RISING	(0x1000)
#define EPORT_EPPAR_EPPA6_FALLING	(0x2000)
#define EPORT_EPPAR_EPPA6_BOTH		(0x3000)
#define EPORT_EPPAR_EPPA5_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA5_RISING	(0x0400)
#define EPORT_EPPAR_EPPA5_FALLING	(0x0800)
#define EPORT_EPPAR_EPPA5_BOTH		(0x0C00)
#define EPORT_EPPAR_EPPA4_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA4_RISING	(0x0100)
#define EPORT_EPPAR_EPPA4_FALLING	(0x0200)
#define EPORT_EPPAR_EPPA4_BOTH		(0x0300)
#define EPORT_EPPAR_EPPA3_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA3_RISING	(0x0040)
#define EPORT_EPPAR_EPPA3_FALLING	(0x0080)
#define EPORT_EPPAR_EPPA3_BOTH		(0x00C0)
#define EPORT_EPPAR_EPPA2_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA2_RISING	(0x0010)
#define EPORT_EPPAR_EPPA2_FALLING	(0x0020)
#define EPORT_EPPAR_EPPA2_BOTH		(0x0030)
#define EPORT_EPPAR_EPPA1_LEVEL		(0x0000)
#define EPORT_EPPAR_EPPA1_RISING	(0x0004)
#define EPORT_EPPAR_EPPA1_FALLING	(0x0008)
#define EPORT_EPPAR_EPPA1_BOTH		(0x000C)

/* Bit definitions and macros for EPDDR */
#define EPORT_EPDDR_EPDD1		(0x02)
#define EPORT_EPDDR_EPDD2		(0x04)
#define EPORT_EPDDR_EPDD3		(0x08)
#define EPORT_EPDDR_EPDD4		(0x10)
#define EPORT_EPDDR_EPDD5		(0x20)
#define EPORT_EPDDR_EPDD6		(0x40)
#define EPORT_EPDDR_EPDD7		(0x80)

/* Bit definitions and macros for EPIER */
#define EPORT_EPIER_EPIE1		(0x02)
#define EPORT_EPIER_EPIE2		(0x04)
#define EPORT_EPIER_EPIE3		(0x08)
#define EPORT_EPIER_EPIE4		(0x10)
#define EPORT_EPIER_EPIE5		(0x20)
#define EPORT_EPIER_EPIE6		(0x40)
#define EPORT_EPIER_EPIE7		(0x80)

/* Bit definitions and macros for EPDR */
#define EPORT_EPDR_EPD1			(0x02)
#define EPORT_EPDR_EPD2			(0x04)
#define EPORT_EPDR_EPD3			(0x08)
#define EPORT_EPDR_EPD4			(0x10)
#define EPORT_EPDR_EPD5			(0x20)
#define EPORT_EPDR_EPD6			(0x40)
#define EPORT_EPDR_EPD7			(0x80)

/* Bit definitions and macros for EPPDR */
#define EPORT_EPPDR_EPPD1		(0x02)
#define EPORT_EPPDR_EPPD2		(0x04)
#define EPORT_EPPDR_EPPD3		(0x08)
#define EPORT_EPPDR_EPPD4		(0x10)
#define EPORT_EPPDR_EPPD5		(0x20)
#define EPORT_EPPDR_EPPD6		(0x40)
#define EPORT_EPPDR_EPPD7		(0x80)

/* Bit definitions and macros for EPFR */
#define EPORT_EPFR_EPF1			(0x02)
#define EPORT_EPFR_EPF2			(0x04)
#define EPORT_EPFR_EPF3			(0x08)
#define EPORT_EPFR_EPF4			(0x10)
#define EPORT_EPFR_EPF5			(0x20)
#define EPORT_EPFR_EPF6			(0x40)
#define EPORT_EPFR_EPF7			(0x80)

#endif					/* __EDMA_H__ */
