/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de
 *
 * Copyright 2009 Freescale Semiconductor, Inc.
 */

#include "config.h"

OUTPUT_ARCH(powerpc)
SECTIONS
{
	. = 0xfff00000;
	.text : {
		*(.text*)
	}
	_etext = .;

	.reloc : {
		_GOT2_TABLE_ = .;
		KEEP(*(.got2))
		KEEP(*(.got))
		_FIXUP_TABLE_ = .;
		KEEP(*(.fixup))
	}
	__got2_entries = ((_GLOBAL_OFFSET_TABLE_ - _GOT2_TABLE_) >> 2) - 1;
	__fixup_entries = (. - _FIXUP_TABLE_) >> 2;

	. = ALIGN(8);
	.data : {
		*(.rodata*)
		*(.data*)
		*(.sdata*)
	}
	_edata  =  .;

	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(8);
	__init_begin = .;
	__init_end = .;
	_end = .;
#if defined(CONFIG_FSL_IFC) /* Restrict bootpg at 4K boundry for IFC */
	.bootpg ADDR(.text) + 0x1000 :
	{
		start.o	(.bootpg)
	}
#define RESET_VECTOR_OFFSET 0x1ffc /* IFC has 8K sram */
#elif defined(CONFIG_FSL_ELBC)
#define RESET_VECTOR_OFFSET 0xffc /* LBC has 4k sram */
#else
#error unknown NAND controller
#endif
	.resetvec ADDR(.text) + RESET_VECTOR_OFFSET : {
		KEEP(*(.resetvec))
	} = 0xffff

	__bss_start = .;
	.bss : {
		*(.sbss*)
		*(.bss*)
	}
	__bss_end = .;
}
ASSERT(__init_end <= (0xfff00000 + RESET_VECTOR_OFFSET), "NAND bootstrap too big");
