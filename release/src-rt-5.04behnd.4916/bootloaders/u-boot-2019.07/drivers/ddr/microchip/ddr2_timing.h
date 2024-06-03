/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#ifndef __MICROCHIP_DDR2_TIMING_H
#define __MICROCHIP_DDR2_TIMING_H

/* MPLL freq is 400MHz */
#define T_CK		2500    /* 2500 psec */
#define T_CK_CTRL	(T_CK * 2)

/* Burst length in cycles */
#define BL		2
/* default CAS latency for all speed grades */
#define RL		5
/* default write latency for all speed grades = CL-1 */
#define WL		4

/* From Micron MT47H64M16HR-3 data sheet */
#define T_RFC_MIN	127500	/* psec */
#define T_WR		15000	/* psec */
#define T_RP		12500	/* psec */
#define T_RCD		12500	/* psec */
#define T_RRD		7500	/* psec */
/* T_RRD_TCK is minimum of 2 clk periods, regardless of freq */
#define T_RRD_TCK	2
#define T_WTR		7500	/* psec */
/* T_WTR_TCK is minimum of 2 clk periods, regardless of freq */
#define T_WTR_TCK	2
#define T_RTP		7500	/* psec */
#define T_RTP_TCK	(BL / 2)
#define T_XP_TCK	2	/* clocks */
#define T_CKE_TCK	3	/* clocks */
#define T_XSNR		(T_RFC_MIN + 10000) /* psec */
#define T_DLLK		200     /* clocks */
#define T_RAS_MIN	45000   /* psec */
#define T_RC		57500   /* psec */
#define T_FAW		35000   /* psec */
#define T_MRD_TCK	2       /* clocks */
#define T_RFI		7800000 /* psec */

/* DDR Addressing */
#define COL_BITS	10
#define BA_BITS		3
#define ROW_BITS	13
#define CS_BITS		1

/* DDR Addressing scheme: {CS, ROW, BA, COL} */
#define COL_HI_RSHFT	0
#define COL_HI_MASK	0
#define COL_LO_MASK	((1 << COL_BITS) - 1)

#define BA_RSHFT	COL_BITS
#define BA_MASK		((1 << BA_BITS) - 1)

#define ROW_ADDR_RSHIFT	(BA_RSHFT + BA_BITS)
#define ROW_ADDR_MASK	((1 << ROW_BITS) - 1)

#define CS_ADDR_RSHIFT	0
#define CS_ADDR_MASK	0

#endif	/* __MICROCHIP_DDR2_TIMING_H */
