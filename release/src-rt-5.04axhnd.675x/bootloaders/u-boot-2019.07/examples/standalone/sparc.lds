/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */


OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)
ENTRY(_start)

SECTIONS
{
	.text :
	{
	  *(.text)
	}
	__text_end = .;

	. = ALIGN(4);
	.rodata :
	{
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}
	__rodata_end = .;

	. = ALIGN(4);
	.data :
	{
		*(.data)
	}

	. = ALIGN(4);
	__data_end = .;

	__bss_start = .;
	. = ALIGN(4);
	.bss :
	{
		*(.bss)
	}
	. = ALIGN(4);
	__bss_end = .;
	_end = .;
}
