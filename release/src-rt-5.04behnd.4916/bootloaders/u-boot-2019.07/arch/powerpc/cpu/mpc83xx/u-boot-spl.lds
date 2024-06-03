/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

OUTPUT_ARCH(powerpc)
SECTIONS
{
	. = 0xfff00000;
	.text : {
		*(.text*)
		. = ALIGN(16);
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(8);
	.data : {
		*(.data*)
		*(.sdata*)
		_GOT2_TABLE_ = .;
		KEEP(*(.got2))
		KEEP(*(.got))
	}
	__got2_entries = ((_GLOBAL_OFFSET_TABLE_ - _GOT2_TABLE_) >> 2) - 1;

	. = ALIGN(8);
	__bss_start = .;
	.bss (NOLOAD) : {
		*(.*bss)
	}
	__bss_end = .;
}
ENTRY(_start)
ASSERT(__bss_end <= 0xfff01000, "NAND bootstrap too big");
