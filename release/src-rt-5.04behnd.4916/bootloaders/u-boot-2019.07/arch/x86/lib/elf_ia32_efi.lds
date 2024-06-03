/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot EFI linker script
 *
 * Modified from usr/lib32/elf_ia32_efi.lds in gnu-efi
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
	image_base = .;
	.hash : { *(.hash) }	/* this MUST come first, EFI expects it */
	. = ALIGN(4096);
	.text :
	{
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
	}
	. = ALIGN(4096);
	.sdata :
	{
		*(.got.plt)
		*(.got)
		*(.srodata)
		*(.sdata)
		*(.sbss)
		*(.scommon)
	}
	. = ALIGN(4096);
	.data :
	{
		*(.rodata*)
		*(.data)
		*(.data1)
		*(.data.*)
		*(.sdata)
		*(.got.plt)
		*(.got)
		/*
		 * the EFI loader doesn't seem to like a .bss section, so we
		 * stick it all into .data:
		 */
		*(.sbss)
		*(.scommon)
		*(.dynbss)
		*(.bss*)
		*(COMMON)

		/* U-Boot lists and device tree */
		. = ALIGN(8);
		*(SORT(.u_boot_list*));
		. = ALIGN(8);
		*(.dtb*);
	}

	. = ALIGN(4096);
	.dynamic  : { *(.dynamic) }
	. = ALIGN(4096);
	.rel :
	{
		*(.rel.data)
		*(.rel.data.*)
		*(.rel.got)
		*(.rel.stab)
		*(.data.rel.ro.local)
		*(.data.rel.local)
		*(.data.rel.ro)
		*(.data.rel*)
		*(.rel.u_boot_list*)
	}
	. = ALIGN(4096);
		.reloc :	/* This is the PECOFF .reloc section! */
	{
	*(.reloc)
	}
	. = ALIGN(4096);
	.dynsym   : { *(.dynsym) }
	. = ALIGN(4096);
	.dynstr   : { *(.dynstr) }
	. = ALIGN(4096);
	/DISCARD/ :
	{
		*(.rel.reloc)
		*(.eh_frame)
		*(.note.GNU-stack)
	}
	.comment 0 : { *(.comment) }
}
