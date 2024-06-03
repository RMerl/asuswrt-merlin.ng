/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bus Matrix header file for the SAMA5 family
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __SAMA5_MATRIX_H
#define __SAMA5_MATRIX_H

struct atmel_matrix {
	u32 mcfg[16];	/* 0x00 ~ 0x3c: Master Configuration Register */
	u32 scfg[16];	/* 0x40 ~ 0x7c: Slave Configuration Register */
	u32 pras[16][2];/* 0x80 ~ 0xfc: Priority Register A/B */
	u32 res1[20];	/* 0x100 ~ 0x14c */
	u32 meier;	/* 0x150: Master Error Interrupt Enable Register */
	u32 meidr;	/* 0x154: Master Error Interrupt Disable Register */
	u32 meimr;	/* 0x158: Master Error Interrupt Mask Register */
	u32 mesr;	/* 0x15c: Master Error Status Register */
	u32 mear[16];	/* 0x160 ~ 0x19c: Master Error Address Register */
	u32 res2[17];	/* 0x1A0 ~ 0x1E0 */
	u32 wpmr;	/* 0x1E4: Write Protection Mode Register */
	u32 wpsr;	/* 0x1E8: Write Protection Status Register */
	u32 res3[5];	/* 0x1EC ~ 0x1FC */
	u32 ssr[16];	/* 0x200 ~ 0x23c: Security Slave Register */
	u32 sassr[16];	/* 0x240 ~ 0x27c: Security Areas Split Slave Register */
	u32 srtsr[16];	/* 0x280 ~ 0x2bc: Security Region Top Slave */
	u32 spselr[3];	/* 0x2c0 ~ 0x2c8: Security Peripheral Select Register */
};

/* Bit field in WPMR */
#define ATMEL_MATRIX_WPMR_WPKEY	0x4D415400
#define ATMEL_MATRIX_WPMR_WPEN	0x00000001

#endif
