/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 */

MEMORY { .sram : ORIGIN = IMAGE_TEXT_BASE,\
		LENGTH = CONFIG_SPL_MAX_FOOTPRINT }

MEMORY { .sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
                LENGTH = CONFIG_SPL_BSS_MAX_SIZE }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text      :
	{
	__start = .;
	  *(.vectors)
	  arch/arm/cpu/arm926ejs/start.o	(.text*)
	  *(.text*)
	} >.sram

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram

	. = ALIGN(4);
	.u_boot_list : { KEEP(*(SORT(.u_boot_list*))); } >.sram

	. = ALIGN(4);
	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	} >.sram

	__image_copy_end = .;

	.end :
	{
		*(.__end)
	}

	_image_binary_end = .;

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} >.sdram
}
