/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

/*
 * SDRAM Controller
 */
#ifndef __FTSDMC020_H
#define __FTSDMC020_H

#define FTSDMC020_OFFSET_TP0		0x00
#define FTSDMC020_OFFSET_TP1		0x04
#define FTSDMC020_OFFSET_CR		0x08
#define FTSDMC020_OFFSET_BANK0_BSR	0x0C
#define FTSDMC020_OFFSET_BANK1_BSR	0x10
#define FTSDMC020_OFFSET_BANK2_BSR	0x14
#define FTSDMC020_OFFSET_BANK3_BSR	0x18
#define FTSDMC020_OFFSET_BANK4_BSR	0x1C
#define FTSDMC020_OFFSET_BANK5_BSR	0x20
#define FTSDMC020_OFFSET_BANK6_BSR	0x24
#define FTSDMC020_OFFSET_BANK7_BSR	0x28
#define FTSDMC020_OFFSET_ACR		0x34

/*
 * Timing Parametet 0 Register
 */
#define FTSDMC020_TP0_TCL(x)	((x) & 0x3)
#define FTSDMC020_TP0_TWR(x)	(((x) & 0x3) << 4)
#define FTSDMC020_TP0_TRF(x)	(((x) & 0xf) << 8)
#define FTSDMC020_TP0_TRCD(x)	(((x) & 0x7) << 12)
#define FTSDMC020_TP0_TRP(x)	(((x) & 0xf) << 16)
#define FTSDMC020_TP0_TRAS(x)	(((x) & 0xf) << 20)

/*
 * Timing Parametet 1 Register
 */
#define FTSDMC020_TP1_REF_INTV(x)	((x) & 0xffff)
#define FTSDMC020_TP1_INI_REFT(x)	(((x) & 0xf) << 16)
#define FTSDMC020_TP1_INI_PREC(x)	(((x) & 0xf) << 20)

/*
 * Configuration Register
 */
#define FTSDMC020_CR_SREF	(1 << 0)
#define FTSDMC020_CR_PWDN	(1 << 1)
#define FTSDMC020_CR_ISMR	(1 << 2)
#define FTSDMC020_CR_IREF	(1 << 3)
#define FTSDMC020_CR_IPREC	(1 << 4)
#define FTSDMC020_CR_REFTYPE	(1 << 5)

/*
 * SDRAM External Bank Base/Size Register
 */
#define FTSDMC020_BANK_ENABLE		(1 << 28)

#define FTSDMC020_BANK_BASE(addr)	(((addr) >> 20) << 16)

#define FTSDMC020_BANK_DDW_X4		(0 << 12)
#define FTSDMC020_BANK_DDW_X8		(1 << 12)
#define FTSDMC020_BANK_DDW_X16		(2 << 12)
#define FTSDMC020_BANK_DDW_X32		(3 << 12)

#define FTSDMC020_BANK_DSZ_16M		(0 << 8)
#define FTSDMC020_BANK_DSZ_64M		(1 << 8)
#define FTSDMC020_BANK_DSZ_128M		(2 << 8)
#define FTSDMC020_BANK_DSZ_256M		(3 << 8)

#define FTSDMC020_BANK_MBW_8		(0 << 4)
#define FTSDMC020_BANK_MBW_16		(1 << 4)
#define FTSDMC020_BANK_MBW_32		(2 << 4)

#define FTSDMC020_BANK_SIZE_1M		0x0
#define FTSDMC020_BANK_SIZE_2M		0x1
#define FTSDMC020_BANK_SIZE_4M		0x2
#define FTSDMC020_BANK_SIZE_8M		0x3
#define FTSDMC020_BANK_SIZE_16M		0x4
#define FTSDMC020_BANK_SIZE_32M		0x5
#define FTSDMC020_BANK_SIZE_64M		0x6
#define FTSDMC020_BANK_SIZE_128M	0x7
#define FTSDMC020_BANK_SIZE_256M	0x8

/*
 * Arbiter Control Register
 */
#define FTSDMC020_ACR_TOC(x)	((x) & 0x1f)
#define FTSDMC020_ACR_TOE	(1 << 8)

#endif	/* __FTSDMC020_H */
