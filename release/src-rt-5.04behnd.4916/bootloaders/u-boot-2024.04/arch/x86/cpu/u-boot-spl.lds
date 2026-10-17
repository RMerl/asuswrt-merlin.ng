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
	. = IMAGE_TEXT_BASE;	/* Location of bootcode in flash */
	__text_start = .;
	.text  : {
		__image_copy_start = .;
		*(.text*);
	}

	. = ALIGN(4);

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);

	.priv_data : {
		__priv_data_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.priv_data*)))
		__priv_data_end = .;
	}

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	__data_end = .;
	__init_end = .;
	. = ALIGN(4);
	.binman_sym_table : {
		__binman_sym_start = .;
		KEEP(*(SORT(.binman_sym*)));
		__binman_sym_end = .;

		/*
		 * Force 32-byte alignment so that it lines up with the start of
		 * bss, which may have up to 32-byte alignment. This ensures
		 * that the end of the .bin file matches up with
		 * _image_binary_end or __bss_end - see board_fdt_blob_setup().
		 * The alignment of BSS depends on what is in it, so can range
		 * from 4 to 32 bytes.
		 */
		. = ALIGN(32);
	}

        _image_binary_end = .;

#if CONFIG_IS_ENABLED(SEPARATE_BSS)
	. = 0x120000;
#endif
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
	/DISCARD/ : { *(.note.gnu.property) }

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
