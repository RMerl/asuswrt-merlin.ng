/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale DSPI Module Defines
 *
 * Copyright (C) 2004-2007, 2015 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 * Chao Fu (B44548@freesacle.com)
 * Haikun Wang (B53464@freescale.com)
 */

#ifndef _FSL_DSPI_H_
#define _FSL_DSPI_H_

/* DMA Serial Peripheral Interface (DSPI) */
struct dspi {
	u32 mcr;	/* 0x00 */
	u32 resv0;	/* 0x04 */
	u32 tcr;	/* 0x08 */
	u32 ctar[8];	/* 0x0C - 0x28 */
	u32 sr;		/* 0x2C */
	u32 irsr;	/* 0x30 */
	u32 tfr;	/* 0x34 - PUSHR */
	u32 rfr;	/* 0x38 - POPR */
	u32 tfdr[16];	/* 0x3C */
	u32 rfdr[16];	/* 0x7C */
};

/* Module configuration */
#define DSPI_MCR_MSTR			0x80000000
#define DSPI_MCR_CSCK			0x40000000
#define DSPI_MCR_DCONF(x)		(((x) & 0x03) << 28)
#define DSPI_MCR_FRZ			0x08000000
#define DSPI_MCR_MTFE			0x04000000
#define DSPI_MCR_PCSSE			0x02000000
#define DSPI_MCR_ROOE			0x01000000
#define DSPI_MCR_PCSIS(x)		(1 << (16 + (x)))
#define DSPI_MCR_PCSIS_MASK		(0xff << 16)
#define DSPI_MCR_CSIS7			0x00800000
#define DSPI_MCR_CSIS6			0x00400000
#define DSPI_MCR_CSIS5			0x00200000
#define DSPI_MCR_CSIS4			0x00100000
#define DSPI_MCR_CSIS3			0x00080000
#define DSPI_MCR_CSIS2			0x00040000
#define DSPI_MCR_CSIS1			0x00020000
#define DSPI_MCR_CSIS0			0x00010000
#define DSPI_MCR_DOZE			0x00008000
#define DSPI_MCR_MDIS			0x00004000
#define DSPI_MCR_DTXF			0x00002000
#define DSPI_MCR_DRXF			0x00001000
#define DSPI_MCR_CTXF			0x00000800
#define DSPI_MCR_CRXF			0x00000400
#define DSPI_MCR_SMPL_PT(x)		(((x) & 0x03) << 8)
#define DSPI_MCR_FCPCS			0x00000001
#define DSPI_MCR_PES			0x00000001
#define DSPI_MCR_HALT			0x00000001

/* Transfer count */
#define DSPI_TCR_SPI_TCNT(x)		(((x) & 0x0000FFFF) << 16)

/* Clock and transfer attributes */
#define DSPI_CTAR(x)			(0x0c + (x * 4))
#define DSPI_CTAR_DBR			0x80000000
#define DSPI_CTAR_TRSZ(x)		(((x) & 0x0F) << 27)
#define DSPI_CTAR_CPOL			0x04000000
#define DSPI_CTAR_CPHA			0x02000000
#define DSPI_CTAR_LSBFE			0x01000000
#define DSPI_CTAR_PCSSCK(x)		(((x) & 0x03) << 22)
#define DSPI_CTAR_PCSSCK_7CLK		0x00A00000
#define DSPI_CTAR_PCSSCK_5CLK		0x00800000
#define DSPI_CTAR_PCSSCK_3CLK		0x00400000
#define DSPI_CTAR_PCSSCK_1CLK		0x00000000
#define DSPI_CTAR_PASC(x)		(((x) & 0x03) << 20)
#define DSPI_CTAR_PASC_7CLK		0x00300000
#define DSPI_CTAR_PASC_5CLK		0x00200000
#define DSPI_CTAR_PASC_3CLK		0x00100000
#define DSPI_CTAR_PASC_1CLK		0x00000000
#define DSPI_CTAR_PDT(x)		(((x) & 0x03) << 18)
#define DSPI_CTAR_PDT_7CLK		0x000A0000
#define DSPI_CTAR_PDT_5CLK		0x00080000
#define DSPI_CTAR_PDT_3CLK		0x00040000
#define DSPI_CTAR_PDT_1CLK		0x00000000
#define DSPI_CTAR_PBR(x)		(((x) & 0x03) << 16)
#define DSPI_CTAR_PBR_7CLK		0x00030000
#define DSPI_CTAR_PBR_5CLK		0x00020000
#define DSPI_CTAR_PBR_3CLK		0x00010000
#define DSPI_CTAR_PBR_1CLK		0x00000000
#define DSPI_CTAR_CSSCK(x)		(((x) & 0x0F) << 12)
#define DSPI_CTAR_ASC(x)		(((x) & 0x0F) << 8)
#define DSPI_CTAR_DT(x)			(((x) & 0x0F) << 4)
#define DSPI_CTAR_BR(x)			((x) & 0x0F)
#define DSPI_CTAR_SCALE_BITS		0xf

/* Status */
#define DSPI_SR_TCF			0x80000000
#define DSPI_SR_TXRXS			0x40000000
#define DSPI_SR_EOQF			0x10000000
#define DSPI_SR_TFUF			0x08000000
#define DSPI_SR_TFFF			0x02000000
#define DSPI_SR_RFOF			0x00080000
#define DSPI_SR_RFDF			0x00020000
#define DSPI_SR_TXCTR(x)		(((x) & 0x0000F000) >> 12)
#define DSPI_SR_TXPTR(x)		(((x) & 0x00000F00) >> 8)
#define DSPI_SR_RXCTR(x)		(((x) & 0x000000F0) >> 4)
#define DSPI_SR_RXPTR(x)		((x) & 0x0000000F)

/* DMA/interrupt request selct and enable */
#define DSPI_IRSR_TCFE			0x80000000
#define DSPI_IRSR_EOQFE			0x10000000
#define DSPI_IRSR_TFUFE			0x08000000
#define DSPI_IRSR_TFFFE			0x02000000
#define DSPI_IRSR_TFFFS			0x01000000
#define DSPI_IRSR_RFOFE			0x00080000
#define DSPI_IRSR_RFDFE			0x00020000
#define DSPI_IRSR_RFDFS			0x00010000

/* Transfer control - 32-bit access */
#define DSPI_TFR_PCS(x)			(((1 << x) & 0x0000003f) << 16)
#define DSPI_TFR_CONT			0x80000000
#define DSPI_TFR_CTAS(x)		(((x) & 0x07) << 28)
#define DSPI_TFR_EOQ			0x08000000
#define DSPI_TFR_CTCNT			0x04000000
#define DSPI_TFR_CS7			0x00800000
#define DSPI_TFR_CS6			0x00400000
#define DSPI_TFR_CS5			0x00200000
#define DSPI_TFR_CS4			0x00100000
#define DSPI_TFR_CS3			0x00080000
#define DSPI_TFR_CS2			0x00040000
#define DSPI_TFR_CS1			0x00020000
#define DSPI_TFR_CS0			0x00010000

/* Transfer Fifo */
#define DSPI_TFR_TXDATA(x)		((x) & 0x0000FFFF)

/* Bit definitions and macros for DRFR */
#define DSPI_RFR_RXDATA(x)		((x) & 0x0000FFFF)

/* Bit definitions and macros for DTFDR group */
#define DSPI_TFDR_TXDATA(x)		((x) & 0x0000FFFF)
#define DSPI_TFDR_TXCMD(x)		(((x) & 0x0000FFFF) << 16)

/* Bit definitions and macros for DRFDR group */
#define DSPI_RFDR_RXDATA(x)		((x) & 0x0000FFFF)

#endif				/* _FSL_DSPI_H_ */
