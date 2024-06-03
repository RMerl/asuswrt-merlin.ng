/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 */

#include <config.h>
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
#ifndef CONFIG_CMDLINE
	/DISCARD/ : { *(.u_boot_list_2_cmd_*) }
#endif

	. = IMAGE_TEXT_BASE;	/* Location of bootcode in flash */
	__text_start = .;
	.text  : { *(.text*); }

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	__data_end = .;
	__init_end = .;

        _image_binary_end = .;

	. = 0x120000;
	.bss (OVERLAY) : {
		__bss_start = .;
		*(.bss*)
		*(COM*)
		. = ALIGN(4);
		__bss_end = .;
	}
	__bss_size = __bss_end - __bss_start;

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

#if defined(CONFIG_SPL_X86_16BIT_INIT) || defined(CONFIG_TPL_X86_16BIT_INIT)
	/*
	 * The following expressions place the 16-bit Real-Mode code and
	 * Reset Vector at the end of the Flash ROM
	 */
	. = START_16 - RESET_SEG_START;
	.start16 : AT (START_16) {
		KEEP(*(.start16));
	}

	. = RESET_VEC_LOC - RESET_SEG_START;
	.resetvec : AT (RESET_VEC_LOC) {
		KEEP(*(.resetvec));
	}
#endif

}
