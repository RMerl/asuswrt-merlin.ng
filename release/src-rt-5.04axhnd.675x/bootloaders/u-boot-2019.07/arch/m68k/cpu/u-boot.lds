/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2015
 * Angelo Dureghello, Sysam Firmware, angelo@sysam.it
 */

#include <config.h>

OUTPUT_ARCH(m68k)

#ifndef LDS_BOARD_TEXT
#define LDS_BOARD_TEXT
#endif

SECTIONS
{
	.text :
	{
		CPUDIR/start.o (.text*)
		LDS_BOARD_TEXT

		*(.text*)
	}
	_etext = .;
	PROVIDE (etext = .);
	.rodata :
	{
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	/* Read-write section, merged into data segment: */
	. = (. + 0x00FF) & 0xFFFFFF00;
	_erotext = .;
	PROVIDE (erotext = .);

	.reloc :
	{
		__got_start = .;
		KEEP(*(.got))
		__got_end = .;
		_GOT2_TABLE_ = .;
		KEEP(*(.got2))
		_FIXUP_TABLE_ = .;
		KEEP(*(.fixup))
	}
	__got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >>2;
	__fixup_entries = (. - _FIXUP_TABLE_)>>2;

	.data :
	{
		*(.data*)
		*(.sdata*)
	}
	_edata = .;
	PROVIDE (edata = .);

	. = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = .;
	__start___ex_table = .;
	__ex_table : { *(__ex_table) }
	__stop___ex_table = .;

	. = ALIGN(4);
	__init_begin = .;
	.text.init : { *(.text.init) }
	.data.init : { *(.data.init) }
	. = ALIGN(4);
	__init_end = .;

	_end = .;

	__bss_start = .;
	.bss (NOLOAD)       :
	{
		_sbss = .;
		*(.bss*)
		*(.sbss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	}
	__bss_end = . ;
	PROVIDE (end = .);
}
