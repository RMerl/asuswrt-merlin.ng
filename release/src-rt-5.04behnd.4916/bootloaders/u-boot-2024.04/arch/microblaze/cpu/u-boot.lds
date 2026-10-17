/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

OUTPUT_ARCH(microblaze)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(0x4):
	{
		__text_start = .;
		arch/microblaze/cpu/start.o (.text)
		*(.text*)
		__text_end = .;
	}

	.rodata ALIGN(0x4):
	{
		__rodata_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		__rodata_end = .;
	}

	.data ALIGN(0x4):
	{
		__data_start = .;
		*(.data*)
		__data_end = .;
	}

	.got ALIGN(4):
	{
		_got_start = .;
		*(.got*)
		. = ALIGN(4);
		_got_end = .;
	}

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}
	__init_end = . ;

	. = ALIGN(4);
	__rel_dyn_start = .;
	.rela.dyn : {
		*(.rela.dyn)
	}
	__rel_dyn_end = .;

	. = ALIGN(4);
	__dyn_sym_start = .;
	.dynsym : {
		*(.dynsym)
	}
	__dyn_sym_end = .;

	.bss ALIGN(0x4):
	{
		__bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	_end = . ;
}
