/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Queue Serial Peripheral Interface Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __QSPI_H__
#define __QSPI_H__

/* QSPI module registers */
typedef struct qspi_ctrl {
	u16 mr;			/* 0x00 Mode */
	u16 res1;
	u16 dlyr;		/* 0x04 Delay */
	u16 res2;
	u16 wr;			/* 0x08 Wrap */
	u16 res3;
	u16 ir;			/* 0x0C Interrupt */
	u16 res4;
	u16 ar;			/* 0x10 Address */
	u16 res5;
	u16 dr;			/* 0x14 Data */
	u16 res6;
} qspi_t;

/* MR */
#define QSPI_QMR_MSTR			(0x8000)
#define QSPI_QMR_DOHIE			(0x4000)
#define QSPI_QMR_BITS(x)		(((x)&0x000F)<<10)
#define QSPI_QMR_BITS_MASK		(0xC3FF)
#define QSPI_QMR_BITS_8			(0x2000)
#define QSPI_QMR_BITS_9			(0x2400)
#define QSPI_QMR_BITS_10		(0x2800)
#define QSPI_QMR_BITS_11		(0x2C00)
#define QSPI_QMR_BITS_12		(0x3000)
#define QSPI_QMR_BITS_13		(0x3400)
#define QSPI_QMR_BITS_14		(0x3800)
#define QSPI_QMR_BITS_15		(0x3C00)
#define QSPI_QMR_BITS_16		(0x0000)
#define QSPI_QMR_CPOL			(0x0200)
#define QSPI_QMR_CPHA			(0x0100)
#define QSPI_QMR_BAUD(x)		((x)&0x00FF)
#define QSPI_QMR_BAUD_MASK		(0xFF00)

/* DLYR */
#define QSPI_QDLYR_SPE			(0x8000)
#define QSPI_QDLYR_QCD(x)		(((x)&0x007F)<<8)
#define QSPI_QDLYR_QCD_MASK		(0x80FF)
#define QSPI_QDLYR_DTL(x)		((x)&0x00FF)
#define QSPI_QDLYR_DTL_MASK		(0xFF00)

/* WR */
#define QSPI_QWR_HALT			(0x8000)
#define QSPI_QWR_WREN			(0x4000)
#define QSPI_QWR_WRTO			(0x2000)
#define QSPI_QWR_CSIV			(0x1000)
#define QSPI_QWR_ENDQP(x)		(((x)&0x000F)<<8)
#define QSPI_QWR_ENDQP_MASK		(0xF0FF)
#define QSPI_QWR_CPTQP(x)		(((x)&0x000F)<<4)
#define QSPI_QWR_CPTQP_MASK		(0xFF0F)
#define QSPI_QWR_NEWQP(x)		((x)&0x000F)
#define QSPI_QWR_NEWQP_MASK		(0xFFF0)

/* IR */
#define QSPI_QIR_WCEFB			(0x8000)
#define QSPI_QIR_ABRTB			(0x4000)
#define QSPI_QIR_ABRTL			(0x1000)
#define QSPI_QIR_WCEFE			(0x0800)
#define QSPI_QIR_ABRTE			(0x0400)
#define QSPI_QIR_SPIFE			(0x0100)
#define QSPI_QIR_WCEF			(0x0008)
#define QSPI_QIR_ABRT			(0x0004)
#define QSPI_QIR_SPIF			(0x0001)

/* AR */
#define QSPI_QAR_ADDR(x)		((x)&0x003F)
#define QSPI_QAR_ADDR_MASK		(0xFFC0)
#define QSPI_QAR_TRANS			(0x0000)
#define QSPI_QAR_RECV			(0x0010)
#define QSPI_QAR_CMD			(0x0020)

/* DR with RAM command word definitions */
#define QSPI_QDR_CONT			(0x8000)
#define QSPI_QDR_BITSE			(0x4000)
#define QSPI_QDR_DT			(0x2000)
#define QSPI_QDR_DSCK			(0x1000)
#define QSPI_QDR_QSPI_CS3		(0x0800)
#define QSPI_QDR_QSPI_CS2		(0x0400)
#define QSPI_QDR_QSPI_CS1		(0x0200)
#define QSPI_QDR_QSPI_CS0		(0x0100)

#endif				/* __QSPI_H__ */
