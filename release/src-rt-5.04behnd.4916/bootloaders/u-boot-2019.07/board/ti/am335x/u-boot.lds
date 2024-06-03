/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text :
	{
		*(.__image_copy_start)
		*(.vectors)
		CPUDIR/start.o (.text*)
		board/ti/am335x/built-in.o (.text*)
	}

	/* This needs to come before *(.text*) */
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

	.text_rest :
	{
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);

	. = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

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

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rel.dyn : {
		*(.rel*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	.hash : { *(.hash*) }

	.end :
	{
		*(.__end)
	}

	_image_binary_end = .;

	/*
	 * Deprecated: this MMU section is used by pxa at present but
	 * should not be used by new boards/CPUs.
	 */
	. = ALIGN(4096);
	.mmutable : {
		*(.mmutable)
	}

/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}

	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_limit = .;
	}

	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}

	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.gnu.hash : { *(.gnu.hash) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
}
