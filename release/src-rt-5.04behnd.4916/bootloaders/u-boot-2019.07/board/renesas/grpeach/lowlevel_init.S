/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) 2017 Chris Brandt
 */
#include <config.h>
#include <version.h>
#include <asm/macro.h>

/* Watchdog Registers */
#define RZA1_WDT_BASE	0xFCFE0000
#define WTCSR	(RZA1_WDT_BASE + 0x00) /* Watchdog Timer Control Register */
#define WTCNT	(RZA1_WDT_BASE + 0x02) /* Watchdog Timer Counter Register */
#define WRCSR	(RZA1_WDT_BASE + 0x04) /* Watchdog Reset Control Register */

/* Standby controller registers (chapter 55) */
#define RZA1_STBCR_BASE	0xFCFE0020
#define STBCR1	(RZA1_STBCR_BASE + 0x00)
#define STBCR2	(RZA1_STBCR_BASE + 0x04)
#define STBCR3	(RZA1_STBCR_BASE + 0x400)
#define STBCR4	(RZA1_STBCR_BASE + 0x404)
#define STBCR5	(RZA1_STBCR_BASE + 0x408)
#define STBCR6	(RZA1_STBCR_BASE + 0x40c)
#define STBCR7	(RZA1_STBCR_BASE + 0x410)
#define STBCR8	(RZA1_STBCR_BASE + 0x414)
#define STBCR9	(RZA1_STBCR_BASE + 0x418)
#define STBCR10	(RZA1_STBCR_BASE + 0x41c)
#define STBCR11	(RZA1_STBCR_BASE + 0x420)
#define STBCR12	(RZA1_STBCR_BASE + 0x424)
#define STBCR13	(RZA1_STBCR_BASE + 0x450)

/* Clock Registers */
#define RZA1_FRQCR_BASE	0xFCFE0010
#define FRQCR	(RZA1_FRQCR_BASE + 0x00)
#define FRQCR2	(RZA1_FRQCR_BASE + 0x04)

#define SYSCR1	0xFCFE0400 /* System control register 1 */
#define SYSCR2	0xFCFE0404 /* System control register 2 */
#define SYSCR3	0xFCFE0408 /* System control register 3 */

/* Disable WDT */
#define WTCSR_D		0xA518
#define WTCNT_D		0x5A00

/* Enable all peripheral clocks */
#define STBCR3_D	0x00000000
#define STBCR4_D	0x00000000
#define STBCR5_D	0x00000000
#define STBCR6_D	0x00000000
#define STBCR7_D	0x00000024
#define STBCR8_D	0x00000005
#define STBCR9_D	0x00000000
#define STBCR10_D	0x00000000
#define STBCR11_D	0x000000c0
#define STBCR12_D	0x000000f0

/*
 * Set all system clocks to full speed.
 * On reset, the CPU will be running at 1/2 speed.
 * In the Hardware Manual, see Table 6.3 Settable Frequency Ranges
 */
#define FRQCR_D		0x0035
#define FRQCR2_D	0x0001

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	/* PL310 init */
	write32 0x3fffff80, 0x00000001

	/* Disable WDT */
	write16	WTCSR, WTCSR_D
	write16	WTCNT, WTCNT_D

	/* Set clocks */
	write16	FRQCR, FRQCR_D
	write16	FRQCR2, FRQCR2_D

	/* Enable all peripherals(Standby Control) */
	write8 STBCR3, STBCR3_D
	write8 STBCR4, STBCR4_D
	write8 STBCR5, STBCR5_D
	write8 STBCR6, STBCR6_D
	write8 STBCR7, STBCR7_D
	write8 STBCR8, STBCR8_D
	write8 STBCR9, STBCR9_D
	write8 STBCR10, STBCR10_D
	write8 STBCR11, STBCR11_D
	write8 STBCR12, STBCR12_D

	/* For serial booting, enable read ahead caching to speed things up */
#define DRCR_0  0x3FEFA00C
	write32 DRCR_0, 0x00010100	/* Read Burst ON, Length=2 */

	/* Enable all internal RAM */
	write8 SYSCR1, 0xFF
	write8 SYSCR2, 0xFF
	write8 SYSCR3, 0xFF

	nop
	/* back to arch calling code */
	mov	pc, lr

	.align 4
