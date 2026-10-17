/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2022
 * Chia-Wei Wang <chiawei_wang@aspeedtech.com>
 */

MEMORY { .nor : ORIGIN = CONFIG_SPL_TEXT_BASE,
		LENGTH = CONFIG_SPL_SIZE_LIMIT }
MEMORY { .bss : ORIGIN = CONFIG_SPL_BSS_START_ADDR,
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text :
	{
		__image_copy_start = .;
		*(.vectors)
		CPUDIR/start.o (.text*)
		*(.text*)
		*(.glue*)
	} > .nor

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) } > .nor

	. = ALIGN(4);
	.data : {
		*(.data*)
	} > .nor

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	} > .nor

	. = ALIGN(4);
	.binman_sym_table : {
		__binman_sym_start = .;
		KEEP(*(SORT(.binman_sym*)));
		__binman_sym_end = .;
	} > .nor

	. = ALIGN(4);

	__image_copy_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	} > .nor

	.end :
	{
		*(.__end)
	} > .nor

	_image_binary_end = .;

	.bss : {
		__bss_start = .;
		*(.bss*)
		 . = ALIGN(4);
		__bss_end = .;
	} > .bss

	__bss_size = __bss_end - __bss_start;
}

#if defined(IMAGE_MAX_SIZE)
ASSERT(__image_copy_end - __image_copy_start <= (IMAGE_MAX_SIZE), \
	"SPL image too big");
#endif

#if defined(CONFIG_SPL_BSS_MAX_SIZE)
ASSERT(__bss_end - __bss_start <= (CONFIG_SPL_BSS_MAX_SIZE), \
	"SPL image BSS too big");
#endif

#if defined(CONFIG_SPL_MAX_FOOTPRINT)
ASSERT(__bss_end - _start <= (CONFIG_SPL_MAX_FOOTPRINT), \
	"SPL image plus BSS too big");
#endif
