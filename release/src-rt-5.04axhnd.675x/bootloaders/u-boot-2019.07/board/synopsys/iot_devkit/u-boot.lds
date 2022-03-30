/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

MEMORY {
	ROM : ORIGIN = ROM_BASE, LENGTH = ROM_SIZE
	RAM : ORIGIN = RAM_DATA_BASE, LENGTH = RAM_DATA_SIZE
}

OUTPUT_FORMAT("elf32-littlearc", "elf32-littlearc", "elf32-littlearc")
OUTPUT_ARCH(arc)
ENTRY(_start)
SECTIONS
{
	. = CONFIG_SYS_MONITOR_BASE;
	__image_copy_start = .;
	.ivt :
	{
		__ivt_start = .;
		KEEP(*(.ivt));
		__ivt_end = .;
	} > ROM

	. = ALIGN(1024);
	.text :	{
		__text_start = .;
		arch/arc/lib/start.o (.text*)
		*(.text*)
		__text_end = .;
	} > ROM

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	} > ROM

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));

		/* Mark RAM's LMA */
		. = ALIGN(4);
		__rom_end = .;
	} > ROM

	.data : {
		/* Mark RAM's VMA */
		. = ALIGN(4);

		/*
		 * Everything between __ram_start and __ram_start will be
		 * copied from ROM to RAM in board_early_init_f().
		 */
		__ram_start = .;

		*(.data*)

		__ram_end = .;
	} > RAM AT > ROM

	.bss : {
		. = ALIGN(1024);
		__bss_start = .;
		*(.bss*)
		__bss_end = .;
	} > RAM

	/* Keep relocation-related symbols to make linker happy */
	__rel_dyn_start = .;
	__rel_dyn_end = .;
	__image_copy_end = .;
	__init_end = .;
}
