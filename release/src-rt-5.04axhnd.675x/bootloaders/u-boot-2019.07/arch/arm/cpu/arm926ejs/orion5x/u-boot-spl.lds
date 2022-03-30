/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on:
 *
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Based on omap-common/u-boot-spl.lds:
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *	Aneesh V <aneesh@ti.com>
 */
MEMORY { .nor : ORIGIN = IMAGE_TEXT_BASE,\
		LENGTH = IMAGE_MAX_SIZE }
MEMORY { .bss : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
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
		CPUDIR/start.o	(.text)
		*(.text*)
	} > .nor

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.nor

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.nor

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} > .nor

	. = ALIGN(4);
	__image_copy_end = .;
	_end = .;

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} > .bss
}
