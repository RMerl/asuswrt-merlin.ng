/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board-specific sbf ddr/sdram init.
 *
 * (C) Copyright 2017 Angelo Dureghello <angelo@sysam.it>
 */

.global sbf_dram_init
.text

sbf_dram_init:
	move.l	#0xFC04002D, %a1
	move.b	#46, (%a1)		/* DDR */

	/* slew settings */
	move.l	#0xEC094060, %a1
	move.b	#0, (%a1)

	/* use vco instead of cpu*2 clock for ddr clock */
	move.l	#0xEC09001A, %a1
	move.w	#0xE01D, (%a1)

	/* DDR settings */
	move.l	#0xFC0B8180, %a1
	move.l	#0x00000000, (%a1)
	move.l	#0x40000000, (%a1)

	move.l	#0xFC0B81AC, %a1
	move.l	#0x01030203, (%a1)

	move.l	#0xFC0B8000, %a1
	move.l	#0x01010101, (%a1)+	/* 0x00 */
	move.l	#0x00000101, (%a1)+	/* 0x04 */
	move.l	#0x01010100, (%a1)+	/* 0x08 */
	move.l	#0x01010000, (%a1)+	/* 0x0C */
	move.l	#0x00010101, (%a1)+	/* 0x10 */
	move.l	#0xFC0B8018, %a1
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
	move.l	#0xFC0B80A0, %a1
	move.l	#0x00000000, (%a1)+	/* 0xA0 */
	move.l	#0x00C80064, (%a1)+	/* 0xA4 */
	move.l	#0x44520002, (%a1)+	/* 0xA8 */
	move.l	#0x00C80023, (%a1)+	/* 0xAC */
	move.l	#0xFC0B80B4, %a1
	move.l	#0x0000C350, (%a1)	/* 0xB4 */
	move.l	#0xFC0B80E0, %a1
	move.l	#0x04000000, (%a1)+	/* 0xE0 */
	move.l	#0x03000304, (%a1)+	/* 0xE4 */
	move.l	#0x40040000, (%a1)+	/* 0xE8 */
	move.l	#0xC0004004, (%a1)+	/* 0xEC */
	move.l	#0x0642C000, (%a1)+	/* 0xF0 */
	move.l	#0x00000642, (%a1)+	/* 0xF4 */
	move.l	#0xFC0B8024, %a1
	tpf
	move.l	#0x01000100, (%a1)	/* 0x24 */

	move.l	#0x2000, %d1
	bsr	asm_delay

	rts
