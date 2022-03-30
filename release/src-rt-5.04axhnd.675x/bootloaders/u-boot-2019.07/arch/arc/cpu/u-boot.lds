/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <config.h>

OUTPUT_FORMAT("elf32-littlearc", "elf32-bigarc", "elf32-littlearc")
OUTPUT_ARCH(arc)
ENTRY(_start)
SECTIONS
{
	. = CONFIG_SYS_TEXT_BASE;
	__image_copy_start = .;
	. = ALIGN(1024);
	__ivt_start = .;
	.ivt :
	{
		KEEP(*(.ivt))
	}
	__ivt_end = .;

	. = ALIGN(1024);
	__text_start = .;
	.text :	{
		arch/arc/lib/start.o (.text*)
		*(.text*)
	}
	__text_end = .;

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	__rel_dyn_start = .;
	.rela.dyn : {
		*(.rela.dyn)
	}
	__rel_dyn_end = .;

	. = ALIGN(4);
	__bss_start = .;
	.bss : {
		*(.bss*)
	}
	__bss_end = .;

	. = ALIGN(4);
	__image_copy_end = .;
	__init_end = .;
}
