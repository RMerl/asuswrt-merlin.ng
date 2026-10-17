/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <config.h>

OUTPUT_FORMAT("elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)

SECTIONS
{
	. = CONFIG_SYS_MONITOR_BASE;
	.text :
	{
	  arch/nios2/cpu/start.o (.text)
	  *(.text)
	  *(.text.*)
	  *(.gnu.linkonce.t*)
	  *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	  *(.gnu.linkonce.r*)
	}
	. = ALIGN (4);
	_etext = .;
	PROVIDE (etext = .);

	/* CMD TABLE - sandwich this in between text and data so
	 * the initialization code relocates the command table as
	 * well -- admittedly, this is just pure laziness ;-)
	 */

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	/* INIT DATA sections - "Small" data (see the gcc -G option)
	 * is always gp-relative. Here we make all init data sections
	 * adjacent to simplify the startup code -- and provide
	 * the global pointer for gp-relative access.
	 */
	_data = .;
	.data :
	{
	  *(.data)
	  *(.data.*)
	  *(.gnu.linkonce.d*)
	}

	/*
	 * gp - Since we don't use gp for small data with option "-G0",
	 * we will use gp as global data pointer. The _gp location is
	 * not needed.
	 */

	.sdata :
	{
	  *(.sdata)
	  *(.sdata.*)
	  *(.gnu.linkonce.s.*)
	}
	. = ALIGN(4);

	_edata = .;
	PROVIDE (edata = .);

	/*
	 * _end - This is end of u-boot.bin image.
	 * dtb will be appended here to make u-boot-dtb.bin
	 */
	_end = .;

	/* UNINIT DATA - Small uninitialized data is first so it's
	 * adjacent to sdata and can be referenced via gp. The normal
	 * bss follows. We keep it adjacent to simplify init code.
	 */
	__bss_start = .;
	.sbss (NOLOAD) :
	{
	  *(.sbss)
	  *(.sbss.*)
	  *(.gnu.linkonce.sb.*)
	  *(.scommon)
	}
	. = ALIGN(4);
	.bss (NOLOAD) :
	{
	  *(.bss)
	  *(.bss.*)
	  *(.dynbss)
	  *(COMMON)
	  *(.scommon)
	}
	. = ALIGN(4);
	__bss_end = .;
	PROVIDE (end = .);

	/* DEBUG -- symbol table, string table, etc. etc.
	 */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	.debug		0 : { *(.debug) }
	.line		0 : { *(.line) }
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	.debug_info	0 : { *(.debug_info) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line	0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str	0 : { *(.debug_str) }
	.debug_loc	0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames	 0 : { *(.debug_varnames) }
}
