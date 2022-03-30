/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 */

#include <config.h>
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
#ifndef CONFIG_CMDLINE
	/DISCARD/ : { *(.u_boot_list_2_cmd_*) }
#endif

	. = CONFIG_SYS_TEXT_BASE;	/* Location of bootcode in flash */
	__text_start = .;

	.text.start : { *(.text.start); }

	.__efi_runtime_start : {
		*(.__efi_runtime_start)
	}

	.efi_runtime : {
		*(.text.efi_runtime*)
		*(.rodata.efi_runtime*)
		*(.data.efi_runtime*)
	}

	.__efi_runtime_stop : {
		*(.__efi_runtime_stop)
	}

	.text  : { *(.text*); }

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		KEEP(*(.rodata.efi.init));
	}

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.hash : { *(.hash*) }

	. = ALIGN(4);
	.got : { *(.got*) }

	.efi_runtime_rel_start :
	{
		*(.__efi_runtime_rel_start)
	}

	.efi_runtime_rel : {
		*(.rel*.efi_runtime)
		*(.rel*.efi_runtime.*)
	}

	.efi_runtime_rel_stop :
	{
		*(.__efi_runtime_rel_stop)
	}

	. = ALIGN(4);
	__data_end = .;
	__init_end = .;

	. = ALIGN(4);
	.dynsym : { *(.dynsym*) }

	. = ALIGN(4);
	__rel_dyn_start = .;
	.rela.dyn : {
		*(.rela*)
	}
	__rel_dyn_end = .;
	. = ALIGN(4);

	.dynamic : { *(.dynamic) }

	. = ALIGN(4);
	_end = .;

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.bss*)
		*(COM*)
		. = ALIGN(4);
		__bss_end = .;
	}

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
