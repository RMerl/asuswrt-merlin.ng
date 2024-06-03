/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * Based on arch/arm/cpu/armv7/omap-common/u-boot-spl.lds
 */

MEMORY { .sram : ORIGIN = IMAGE_TEXT_BASE, \
		LENGTH = CONFIG_SPL_MAX_FOOTPRINT }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	.text :
	{
		__start = .;
		*(.vectors)
		arch/arm/cpu/armv7/start.o (.text*)
		*(.text*)
	} >.sram
	. = ALIGN(4);

	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram
	. = ALIGN(4);

	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram
	. = ALIGN(4);

	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} >.sram
	. = ALIGN(4);

	.machine_param : { *(.machine_param) } >.sram
	. = ALIGN(4);

	__image_copy_end = .;

	.end :
	{
		*(.__end)
	} >.sram

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} >.sram
}
