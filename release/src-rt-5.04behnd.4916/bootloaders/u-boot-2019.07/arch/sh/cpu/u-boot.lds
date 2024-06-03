/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Vladimir Zapolskiy <vz@mleia.com>
 * Copyright (C) 2008-2009 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 * Copyright (C) 2008 Mark Jonas <mark.jonas@de.bosch.com>
 * Copyright (C) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include "config.h"

#ifdef CONFIG_SYS_BIG_ENDIAN
OUTPUT_FORMAT("elf32-shbig-linux", "elf32-shbig-linux", "elf32-sh-linux")
#else
OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
#endif

OUTPUT_ARCH(sh)

MEMORY
{
	ram	: ORIGIN = CONFIG_SYS_SDRAM_BASE, LENGTH = CONFIG_SYS_SDRAM_SIZE
}

ENTRY(_start)

SECTIONS
{
	reloc_dst = .;

	PROVIDE (_ftext = .);
	PROVIDE (_fcode = .);
	PROVIDE (_start = .);

	.text :
	{
		KEEP(*/start.o		(.text))
		KEEP(CONFIG_BOARDDIR/lowlevel_init.o	(.text .spiboot1.text))
		KEEP(*(.spiboot2.text))
		. = ALIGN(8192);
#ifdef CONFIG_ENV_IS_IN_FLASH
		env/embedded.o  (.doesnotexist)
		. = ALIGN(8192);
#endif
		*(.text)
		. = ALIGN(4);
	} >ram =0xFF
	PROVIDE (_ecode = .);
	.rodata :
	{
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		. = ALIGN(4);
	} >ram
	PROVIDE (_etext = .);


	PROVIDE (_fdata = .);
	.data :
	{
		*(.data)
		. = ALIGN(4);
	} >ram
	PROVIDE (_edata = .);

	PROVIDE (_fgot = .);
	.got :
	{
		*(.got.plt) *(.got)
		. = ALIGN(4);
	} >ram
	PROVIDE (_egot = .);

	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} >ram

	PROVIDE (__init_end = .);
	PROVIDE (reloc_dst_end = .);

	PROVIDE (bss_start = .);
	PROVIDE (__bss_start = .);
	.bss :
	{
		*(.bss)
		. = ALIGN(4);
	} >ram
	PROVIDE (bss_end = .);
	PROVIDE (__bss_end = .);
}
