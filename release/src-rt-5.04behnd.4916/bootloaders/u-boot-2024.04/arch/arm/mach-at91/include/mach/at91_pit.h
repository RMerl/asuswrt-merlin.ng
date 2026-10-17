/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_pit.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Periodic Interval Timer (PIT) - System peripherals regsters.
 * Based on AT91SAM9261 datasheet revision D.
 */

#ifndef AT91_PIT_H
#define AT91_PIT_H

typedef struct at91_pit {
	u32	mr;	/* 0x00 Mode Register */
	u32	sr;	/* 0x04 Status Register */
	u32	pivr;	/* 0x08 Periodic Interval Value Register */
	u32	piir;	/* 0x0C Periodic Interval Image Register */
} at91_pit_t;

#define		AT91_PIT_MR_IEN		0x02000000
#define		AT91_PIT_MR_EN		0x01000000
#define		AT91_PIT_MR_PIV_MASK(x)	(x & 0x000fffff)
#define		AT91_PIT_MR_PIV(x)	(x & AT91_PIT_MR_PIV_MASK)

#endif
