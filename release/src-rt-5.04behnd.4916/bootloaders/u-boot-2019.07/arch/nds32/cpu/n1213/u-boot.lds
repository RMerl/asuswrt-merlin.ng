/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

OUTPUT_FORMAT("elf32-nds32", "elf32-nds32", "elf32-nds32")
OUTPUT_ARCH(nds32)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text :
	{
		arch/nds32/cpu/n1213/start.o	(.text)
		*(.text)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);

	.got : {
		__got_start = .;
		*(.got.plt) *(.got)
		__got_end = .;
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	/DISCARD/ : { *(.rela.plt*) }
	.rela.dyn : {
		__rel_dyn_start = .;
		*(.rela*)
		__rel_dyn_end = .;
 }
	_end = .;

	.bss : {
		__bss_start = .;
		*(.bss)
		. = ALIGN(4);
		__bss_end = .;
	}

}
