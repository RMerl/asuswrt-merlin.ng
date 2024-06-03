/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fsl_mcdmafec.h -- Multi-channel DMA Fast Ethernet Controller definitions
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef fsl_mcdmafec_h
#define fsl_mcdmafec_h

/* Re-use of the definitions */
#include <asm/fec.h>

typedef struct fecdma {
	u32 rsvd0;		/* 0x000 */
	u32 eir;		/* 0x004 */
	u32 eimr;		/* 0x008 */
	u32 rsvd1[6];		/* 0x00C - 0x023 */
	u32 ecr;		/* 0x024 */
	u32 rsvd2[6];		/* 0x028 - 0x03F */
	u32 mmfr;		/* 0x040 */
	u32 mscr;		/* 0x044 */
	u32 rsvd3[7];		/* 0x048 - 0x063 */
	u32 mibc;		/* 0x064 */
	u32 rsvd4[7];		/* 0x068 - 0x083 */
	u32 rcr;		/* 0x084 */
	u32 rhr;		/* 0x088 */
	u32 rsvd5[14];		/* 0x08C - 0x0C3 */
	u32 tcr;		/* 0x0C4 */
	u32 rsvd6[7];		/* 0x0C8 - 0x0E3 */
	u32 palr;		/* 0x0E4 */
	u32 paur;		/* 0x0E8 */
	u32 opd;		/* 0x0EC */
	u32 rsvd7[10];		/* 0x0F0 - 0x117 */
	u32 iaur;		/* 0x118 */
	u32 ialr;		/* 0x11C */
	u32 gaur;		/* 0x120 */
	u32 galr;		/* 0x124 */
	u32 rsvd8[7];		/* 0x128 - 0x143 */
	u32 tfwr;		/* 0x144 */
	u32 rsvd9[14];		/* 0x148 - 0x17F */
	u32 fmc;		/* 0x180 */
	u32 rfdr;		/* 0x184 */
	u32 rfsr;		/* 0x188 */
	u32 rfcr;		/* 0x18C */
	u32 rlrfp;		/* 0x190 */
	u32 rlwfp;		/* 0x194 */
	u32 rfar;		/* 0x198 */
	u32 rfrp;		/* 0x19C */
	u32 rfwp;		/* 0x1A0 */
	u32 tfdr;		/* 0x1A4 */
	u32 tfsr;		/* 0x1A8 */
	u32 tfcr;		/* 0x1AC */
	u32 tlrfp;		/* 0x1B0 */
	u32 tlwfp;		/* 0x1B4 */
	u32 tfar;		/* 0x1B8 */
	u32 tfrp;		/* 0x1BC */
	u32 tfwp;		/* 0x1C0 */
	u32 frst;		/* 0x1C4 */
	u32 ctcwr;		/* 0x1C8 */
} fecdma_t;

struct fec_info_dma {
	int index;
	u32 iobase;
	u32 pinmux;
	u32 miibase;
	int phy_addr;
	int dup_spd;
	char *phy_name;
	int phyname_init;
	cbd_t *rxbd;		/* Rx BD */
	cbd_t *txbd;		/* Tx BD */
	uint rxIdx;
	uint txIdx;
	char *txbuf;
	int initialized;
	struct fec_info_dma *next;

	u16 rxTask;		/* DMA receive Task Number */
	u16 txTask;		/* DMA Transmit Task Number */
	u16 rxPri;		/* DMA Receive Priority */
	u16 txPri;		/* DMA Transmit Priority */
	u16 rxInit;		/* DMA Receive Initiator */
	u16 txInit;		/* DMA Transmit Initiator */
	u16 usedTbdIdx;		/* next transmit BD to clean */
	u16 cleanTbdNum;	/* the number of available transmit BDs */
};

/* Bit definitions and macros for IEVENT */
#define FEC_EIR_TXERR		(0x00040000)
#define FEC_EIR_RXERR		(0x00020000)
#undef FEC_EIR_CLEAR_ALL
#define FEC_EIR_CLEAR_ALL	(0xFFFE0000)

/* Bit definitions and macros for R_HASH */
#define FEC_RHASH_FCE_DC	(0x80000000)
#define FEC_RHASH_MULTCAST	(0x40000000)
#define FEC_RHASH_HASH(x)	(((x)&0x0000003F)<<24)

/* Bit definitions and macros for FEC_TFWR */
#undef FEC_TFWR_X_WMRK
#undef FEC_TFWR_X_WMRK_64
#undef FEC_TFWR_X_WMRK_128
#undef FEC_TFWR_X_WMRK_192

#define FEC_TFWR_X_WMRK(x)	((x)&0x0F)
#define FEC_TFWR_X_WMRK_64	(0x00)
#define FEC_TFWR_X_WMRK_128	(0x01)
#define FEC_TFWR_X_WMRK_192	(0x02)
#define FEC_TFWR_X_WMRK_256	(0x03)
#define FEC_TFWR_X_WMRK_320	(0x04)
#define FEC_TFWR_X_WMRK_384	(0x05)
#define FEC_TFWR_X_WMRK_448	(0x06)
#define FEC_TFWR_X_WMRK_512	(0x07)
#define FEC_TFWR_X_WMRK_576	(0x08)
#define FEC_TFWR_X_WMRK_640	(0x09)
#define FEC_TFWR_X_WMRK_704	(0x0A)
#define FEC_TFWR_X_WMRK_768	(0x0B)
#define FEC_TFWR_X_WMRK_832	(0x0C)
#define FEC_TFWR_X_WMRK_896	(0x0D)
#define FEC_TFWR_X_WMRK_960	(0x0E)
#define FEC_TFWR_X_WMRK_1024	(0x0F)

/* FIFO definitions */
/* Bit definitions and macros for FSTAT */
#define FIFO_STAT_IP		(0x80000000)
#define FIFO_STAT_FRAME(x)	(((x)&0x0000000F)<<24)
#define FIFO_STAT_FAE		(0x00800000)
#define FIFO_STAT_RXW		(0x00400000)
#define FIFO_STAT_UF		(0x00200000)
#define FIFO_STAT_OF		(0x00100000)
#define FIFO_STAT_FR		(0x00080000)
#define FIFO_STAT_FULL		(0x00040000)
#define FIFO_STAT_ALARM		(0x00020000)
#define FIFO_STAT_EMPTY		(0x00010000)

/* Bit definitions and macros for FCTRL */
#define FIFO_CTRL_WCTL		(0x40000000)
#define FIFO_CTRL_WFR		(0x20000000)
#define FIFO_CTRL_FRAME		(0x08000000)
#define FIFO_CTRL_GR(x)		(((x)&0x00000007)<<24)
#define FIFO_CTRL_IPMASK	(0x00800000)
#define FIFO_CTRL_FAEMASK	(0x00400000)
#define FIFO_CTRL_RXWMASK	(0x00200000)
#define FIFO_CTRL_UFMASK	(0x00100000)
#define FIFO_CTRL_OFMASK	(0x00080000)

#endif				/* fsl_mcdmafec_h */
