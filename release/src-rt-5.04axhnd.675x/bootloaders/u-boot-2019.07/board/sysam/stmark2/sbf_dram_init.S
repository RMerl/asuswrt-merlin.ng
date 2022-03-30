/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board-specific early ddr/sdram init.
 *
 * (C) Copyright 2017 Angelo Dureghello <angelo@sysam.it>
 */

.equ	PPMCR0, 	0xfc04002d
.equ	MSCR_SDRAMC,	0xec094060
.equ	MISCCR2,	0xec09001a
.equ	DDR_RCR,	0xfc0b8180
.equ	DDR_PADCR,	0xfc0b81ac
.equ	DDR_CR00,	0xfc0b8000
.equ	DDR_CR06,	0xfc0b8018
.equ	DDR_CR09,	0xfc0b8024
.equ	DDR_CR40,	0xfc0b80a0
.equ	DDR_CR45,	0xfc0b80b4
.equ	DDR_CR56,	0xfc0b80e0

.global sbf_dram_init
.text

sbf_dram_init:
	/* CD46 = DDR on */
	move.l	#PPMCR0, %a1
	move.b	#46, (%a1)

	/* stmark 2, max drive strength */
	move.l	#MSCR_SDRAMC, %a1
	move.b	#1, (%a1)

	/*
	 * use cpu clock, seems more realiable
	 *
	 * DDR2 clock is serviced from DDR controller as input clock / 2
	 * so, if clock comes from
	 *   vco, i.e. 480(vco) / 2, so ddr clock is 240 Mhz (measured)
	 *   cpu, i.e. 250(cpu) / 2, so ddr clock is 125 Mhz (measured)
	 *
	 *     .
	 *    / \    DDR2 can't be clocked lower than 125Mhz
	 *   / ! \   DDR2 init must pass further i/dcache enable test
	 *  /_____\
	 *  WARNING
	 */

	/* cpu / 2 = 125 Mhz for 480 Mhz pll */
	move.l	#MISCCR2, %a1
	move.w	#0xa01d, (%a1)

	/* DDR force sw reset settings */
	move.l	#DDR_RCR, %a1
	move.l	#0x00000000, (%a1)
	move.l	#0x40000000, (%a1)

	/*
	 * PAD_ODT_CS: for us seems both 1(75 ohm) and 2(150ohm) are good,
	 * 500/700 mV are ok
	 */
	move.l	#DDR_PADCR, %a1
	move.l	#0x01030203, (%a1)	/* as freescale tower */

	move.l	#DDR_CR00, %a1
	move.l	#0x01010101, (%a1)+	/* 0x00 */
	move.l	#0x00000101, (%a1)+	/* 0x04 */
	move.l	#0x01010100, (%a1)+	/* 0x08 */
	move.l	#0x01010000, (%a1)+	/* 0x0C */
	move.l	#0x00010101, (%a1)+	/* 0x10 */
	move.l	#DDR_CR06, %a1
	move.l	#0x00010100, (%a1)+	/* 0x18 */
	move.l	#0x00000001, (%a1)+	/* 0x1C */
	move.l	#0x01000001, (%a1)+	/* 0x20 */
	move.l	#0x00000100, (%a1)+	/* 0x24 */
	move.l	#0x00010001, (%a1)+	/* 0x28 */
	move.l	#0x00000200, (%a1)+	/* 0x2C */
	move.l	#0x01000002, (%a1)+	/* 0x30 */
	move.l	#0x00000000, (%a1)+	/* 0x34 */
	move.l	#0x00000100, (%a1)+	/* 0x38 */
	move.l	#0x02000100, (%a1)+	/* 0x3C */
	move.l	#0x02000407, (%a1)+	/* 0x40 */
	move.l	#0x02030007, (%a1)+	/* 0x44 */
	move.l	#0x02000100, (%a1)+	/* 0x48 */
	move.l	#0x0A030203, (%a1)+	/* 0x4C */
	move.l	#0x00020708, (%a1)+	/* 0x50 */
	move.l	#0x00050008, (%a1)+	/* 0x54 */
	move.l	#0x04030002, (%a1)+	/* 0x58 */
	move.l	#0x00000004, (%a1)+	/* 0x5C */
	move.l	#0x020A0000, (%a1)+	/* 0x60 */
	move.l	#0x0C00000E, (%a1)+	/* 0x64 */
	move.l	#0x00002004, (%a1)+	/* 0x68 */
	move.l	#0x00000000, (%a1)+	/* 0x6C */
	move.l	#0x00100010, (%a1)+	/* 0x70 */
	move.l	#0x00100010, (%a1)+	/* 0x74 */
	move.l	#0x00000000, (%a1)+	/* 0x78 */
	move.l	#0x07990000, (%a1)+	/* 0x7C */
	move.l	#DDR_CR40, %a1
	move.l	#0x00000000, (%a1)+	/* 0xA0 */
	move.l	#0x00C80064, (%a1)+	/* 0xA4 */
	move.l	#0x44520002, (%a1)+	/* 0xA8 */
	move.l	#0x00C80023, (%a1)+	/* 0xAC */
	move.l	#DDR_CR45, %a1
	move.l	#0x0000C350, (%a1)	/* 0xB4 */
	move.l	#DDR_CR56, %a1
	move.l	#0x04000000, (%a1)+	/* 0xE0 */
	move.l	#0x03000304, (%a1)+	/* 0xE4 */
	move.l	#0x40040000, (%a1)+	/* 0xE8 */
	move.l	#0xC0004004, (%a1)+	/* 0xEC */
	move.l	#0x0642C000, (%a1)+	/* 0xF0 */
	move.l	#0x00000642, (%a1)+	/* 0xF4 */
	move.l	#DDR_CR09, %a1
	tpf
	move.l	#0x01000100, (%a1)	/* 0x24 */

	move.l	#0x2000, %d1
	bsr	asm_delay


	rts
