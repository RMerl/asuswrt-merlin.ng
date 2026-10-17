/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_rstc.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Reset Controller (RSTC) - System peripherals regsters.
 * Based on AT91SAM9261 datasheet revision D.
 */

#ifndef AT91_RSTC_H
#define AT91_RSTC_H

/* Reset Controller Status Register */
#define AT91_ASM_RSTC_SR	(ATMEL_BASE_RSTC + 0x04)
#define AT91_ASM_RSTC_MR	(ATMEL_BASE_RSTC + 0x08)

#ifndef __ASSEMBLY__

typedef struct at91_rstc {
	u32	cr;	/* Reset Controller Control Register */
	u32	sr;	/* Reset Controller Status Register */
	u32	mr;	/* Reset Controller Mode Register */
} at91_rstc_t;

#endif /* __ASSEMBLY__ */

#define AT91_RSTC_KEY		0xA5000000

#define AT91_RSTC_CR_PROCRST	0x00000001
#define AT91_RSTC_CR_PERRST	0x00000004
#define AT91_RSTC_CR_EXTRST	0x00000008

#define AT91_RSTC_MR_URSTEN	0x00000001
#define AT91_RSTC_MR_URSTIEN	0x00000010
#define AT91_RSTC_MR_ERSTL(x)	((x & 0xf) << 8)
#define AT91_RSTC_MR_ERSTL_MASK	0x0000FF00

#define AT91_RSTC_SR_NRSTL	0x00010000

#define AT91_RSTC_RSTTYP		(7 << 8)	/* Reset Type */
#define AT91_RSTC_RSTTYP_GENERAL	(0 << 8)
#define AT91_RSTC_RSTTYP_WAKEUP	(1 << 8)
#define AT91_RSTC_RSTTYP_WATCHDOG	(2 << 8)
#define AT91_RSTC_RSTTYP_SOFTWARE	(3 << 8)
#define AT91_RSTC_RSTTYP_USER		(4 << 8)

#endif
