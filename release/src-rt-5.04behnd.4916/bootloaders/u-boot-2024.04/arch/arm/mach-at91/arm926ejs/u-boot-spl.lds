/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

MEMORY { .sram : ORIGIN = IMAGE_TEXT_BASE, \
		LENGTH = IMAGE_MAX_SIZE }
MEMORY { .sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
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
	__u_boot_list : { KEEP(*(SORT(__u_boot_list*))) } > .sram

	. = ALIGN(4);
	__image_copy_end = .;

	.end :
	{
		*(.__end)
	} >.sram

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

#if defined(IMAGE_MAX_SIZE)
ASSERT(__image_copy_end - __start <= (IMAGE_MAX_SIZE), \
	"SPL image too big");
#endif

#if defined(CONFIG_SPL_BSS_MAX_SIZE)
ASSERT(__bss_end - __bss_start <= (CONFIG_SPL_BSS_MAX_SIZE), \
	"SPL image BSS too big");
#endif
