/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Wolfgang Denk Engineering, <wd@denx.de>
 */

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text : {
		__text_start = .;
		*(.text*)
		__text_end = .;
	}

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.sdata : {
		*(.sdata*)
	}

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	. = ALIGN(4);
	__image_copy_end = .;
	__init_end = .;

	.data.reloc : {
		__rel_start = .;
		/*
		 * Space for relocation table
		 * This needs to be filled so that the
		 * mips-reloc tool can overwrite the content.
		 * An invalid value is left at the start of the
		 * section to abort relocation if the table
		 * has not been filled in.
		 */
		LONG(0xFFFFFFFF);
		FILL(0);
		. += CONFIG_MIPS_RELOCATION_TABLE_SIZE - 4;
	}

	. = ALIGN(4);
	_end = .;

	.bss __rel_start (OVERLAY) : {
		__bss_start = .;
		*(.sbss.*)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}

	/* These mark the ABI of U-Boot for debuggers. */
	.mdebug.abi32 : {
		KEEP(*(.mdebug.abi32))
	}
	.mdebug.abi64 : {
		KEEP(*(.mdebug.abi64))
	}

	/* This is the MIPS specific mdebug section. */
	.mdebug : { *(.mdebug) }

	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }

	/*
	 * DWARF debug sections.
	 * Symbols in the DWARF debugging sections are relative to
	 * the beginning of the section so we begin them at 0.
	 */
	/* DWARF 1 */
	.debug 0 : { *(.debug) }
	.line 0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo 0 : { *(.debug_srcinfo) }
	.debug_sfnames 0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges 0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info 0 : {
		*(.debug_info
		.gnu.linkonce.wi.*)
	}
	.debug_abbrev 0 : { *(.debug_abbrev) }
	.debug_line 0 : { *(.debug_line) }
	.debug_frame 0 : { *(.debug_frame) }
	.debug_str 0 : { *(.debug_str) }
	.debug_loc 0 : { *(.debug_loc) }
	.debug_macinfo 0 : { *(.debug_macinfo) }
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	/* DWARF 3 */
	.debug_ranges 0 : { *(.debug_ranges) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames 0 : { *(.debug_varnames) }
	/* GNU DWARF 2 extensions */
	.debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) }
	.debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) }
	/* DWARF 4 */
	.debug_types 0 : { *(.debug_types) }
	/* DWARF 5 */
	.debug_macro 0 : { *(.debug_macro) }
	.debug_addr 0 : { *(.debug_addr) }

	/DISCARD/ : {
		/* ABI crap starts here */
		*(.MIPS.abiflags)
		*(.MIPS.options)
		*(.options)
		*(.pdr)
		*(.reginfo)
		*(.eh_frame)
	}
}
