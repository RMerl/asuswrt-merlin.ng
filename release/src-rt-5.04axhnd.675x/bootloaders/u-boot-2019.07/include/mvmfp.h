/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef __MVMFP_H
#define __MVMFP_H

/*
 * Header file for MultiFunctionPin (MFP) Configururation framework
 *
 * Processors Supported:
 * 1. Marvell ARMADA100 Processors
 *
 * processor to be supported should be added here
 */

/*
 * MFP configuration is represented by a 32-bit unsigned integer
 */
#ifdef CONFIG_MVMFP_V2
#define MFP(_off, _pull, _drv, _slp, _edge, _sleep, _afn) ( \
	/* bits 31..16 - MFP Register Offset */	(((_off) & 0xffff) << 16) | \
	/* bits 15..13 - Run Mode Pull State */	(((_pull) & 0x7) << 13) | \
	/* bit  12..11 - Driver Strength */	(((_drv) & 0x3) << 11) | \
	/* bits 10     - pad driver */		(((_slp) & 0x1) << 10) | \
	/* bit  09..07 - sleep mode */		(((_sleep) & 0xe) << 6) | \
	/* bits 06..04 - Edge Detection */	(((_edge) & 0x7) << 4) | \
	/* bits 03     - sleep mode */		(((_sleep) & 0x1) << 3) | \
	/* bits 02..00 - Alt-fun select */	((_afn) & 0x7))
#else
#define MFP(_off, _pull, _drv, _slp, _edge, _sleep, _afn) ( \
	/* bits 31..16 - MFP Register Offset */	(((_off) & 0xffff) << 16) | \
	/* bits 15..13 - Run Mode Pull State */	(((_pull) & 0x7) << 13) | \
	/* bit  12     - Unused */ \
	/* bits 11..10 - Driver Strength */	(((_drv) & 0x3) << 10) | \
	/* bit  09..07 - sleep mode */		(((_sleep) & 0xe) << 6) | \
	/* bits 06..04 - Edge Detection */	(((_edge) & 0x7) << 4) | \
	/* bits 03     - sleep mode */		(((_sleep) & 0x1) << 3) | \
	/* bits 02..00 - Alt-fun select */	((_afn) & 0x7))
#endif

/*
 * to facilitate the definition, the following macros are provided
 *
 * 				    offset, pull,pF, drv,dF, edge,eF ,afn,aF
 */
#define MFP_OFFSET_MASK		MFP(0xffff,    0,    0,   0,   0,   0,   0)
#define MFP_REG(x)		MFP(x,         0,    0,   0,   0,   0,   0)
#define MFP_REG_GET_OFFSET(x)	((x & MFP_OFFSET_MASK) >> 16)

#define MFP_AF0			MFP(0x0000,    0,    0,   0,   0,   0,   0)
#define MFP_AF1			MFP(0x0000,    0,    0,   0,   0,   0,   1)
#define MFP_AF2			MFP(0x0000,    0,    0,   0,   0,   0,   2)
#define MFP_AF3			MFP(0x0000,    0,    0,   0,   0,   0,   3)
#define MFP_AF4			MFP(0x0000,    0,    0,   0,   0,   0,   4)
#define MFP_AF5			MFP(0x0000,    0,    0,   0,   0,   0,   5)
#define MFP_AF6			MFP(0x0000,    0,    0,   0,   0,   0,   6)
#define MFP_AF7			MFP(0x0000,    0,    0,   0,   0,   0,   7)
#define MFP_AF_MASK		MFP(0x0000,    0,    0,   0,   0,   0,   7)

#define MFP_SLEEP_CTRL2		MFP(0x0000,    0,    0,   0,   0,   1,   0)
#define MFP_SLEEP_DIR		MFP(0x0000,    0,    0,   0,   0,   2,   0)
#define MFP_SLEEP_DATA		MFP(0x0000,    0,    0,   0,   0,   4,   0)
#define MFP_SLEEP_CTRL		MFP(0x0000,    0,    0,   0,   0,   8,   0)
#define MFP_SLEEP_MASK		MFP(0x0000,    0,    0,   0,   0, 0xf,   0)

#define MFP_LPM_EDGE_NONE	MFP(0x0000,    0,    0,   0,   4,   0,   0)
#define MFP_LPM_EDGE_RISE	MFP(0x0000,    0,    0,   0,   1,   0,   0)
#define MFP_LPM_EDGE_FALL	MFP(0x0000,    0,    0,   0,   2,   0,   0)
#define MFP_LPM_EDGE_BOTH	MFP(0x0000,    0,    0,   0,   3,   0,   0)
#define MFP_LPM_EDGE_MASK	MFP(0x0000,    0,    0,   0,   7,   0,   0)

#define MFP_SLP_DI		MFP(0x0000,    0,    0,   1,   0,   0,   0)

#define MFP_DRIVE_VERY_SLOW	MFP(0x0000,    0,    0,   0,   0,   0,   0)
#define MFP_DRIVE_SLOW		MFP(0x0000,    0,    1,   0,   0,   0,   0)
#define MFP_DRIVE_MEDIUM	MFP(0x0000,    0,    2,   0,   0,   0,   0)
#define MFP_DRIVE_FAST		MFP(0x0000,    0,    3,   0,   0,   0,   0)
#define MFP_DRIVE_MASK		MFP(0x0000,    0,    3,   0,   0,   0,   0)

#define MFP_PULL_NONE		MFP(0x0000,    0,    0,   0,   0,   0,   0)
#define MFP_PULL_LOW		MFP(0x0000,    5,    0,   0,   0,   0,   0)
#define MFP_PULL_HIGH		MFP(0x0000,    6,    0,   0,   0,   0,   0)
#define MFP_PULL_BOTH		MFP(0x0000,    7,    0,   0,   0,   0,   0)
#define MFP_PULL_FLOAT		MFP(0x0000,    4,    0,   0,   0,   0,   0)
#define MFP_PULL_MASK		MFP(0x0000,    7,    0,   0,   0,   0,   0)

#define MFP_VALUE_MASK		(MFP_PULL_MASK | MFP_DRIVE_MASK | MFP_SLP_DI \
				| MFP_LPM_EDGE_MASK | MFP_SLEEP_MASK \
				| MFP_AF_MASK)
#define MFP_EOC			0xffffffff	/* indicates end-of-conf */

/* Functions */
void mfp_config(u32 *mfp_cfgs);

#endif /* __MVMFP_H */
