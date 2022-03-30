/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 - 2014 Xilinx, Inc
 *
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <asm-offsets.h>

OUTPUT_ARCH(microblaze)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(0x4):
	{
		__text_start = .;
		arch/microblaze/cpu/start.o (.text)
		*(.text)
		*(.text.*)
		__text_end = .;
	}

	.rodata ALIGN(0x4):
	{
		__rodata_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		__rodata_end = .;
	}

	.data ALIGN(0x4):
	{
		__data_start = .;
		*(.data)
		*(.data.*)
		__data_end = .;
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}
	__init_end = . ;

	.bss ALIGN(0x4):
	{
		__bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	__end = . ;
}

#if defined(CONFIG_SPL_MAX_FOOTPRINT)
ASSERT(__end - _start <= (CONFIG_SPL_MAX_FOOTPRINT), \
        "SPL image plus BSS too big");
#endif
