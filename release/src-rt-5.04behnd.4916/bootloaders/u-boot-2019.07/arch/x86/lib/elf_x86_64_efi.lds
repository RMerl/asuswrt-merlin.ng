/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot EFI linker script
 *
 * Modified from usr/lib32/elf_x86_64_efi.lds in gnu-efi
 */

OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
	image_base = .;
	.hash : { *(.hash) }	/* this MUST come first, EFI expects it */
	. = ALIGN(4096);
	.eh_frame : {
		*(.eh_frame)
	}

	. = ALIGN(4096);

	.text : {
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
	}

	. = ALIGN(4096);

	.reloc : {
		*(.reloc)
	}

	. = ALIGN(4096);

	.data : {
		*(.rodata*)
		*(.got.plt)
		*(.got)
		*(.data*)
		*(.sdata)
		/* the EFI loader doesn't seem to like a .bss section, so we stick
		 * it all into .data: */
		*(.sbss)
		*(.scommon)
		*(.dynbss)
		*(.bss*)
		*(COMMON)
		*(.rel.local)

		/* U-Boot lists and device tree */
		. = ALIGN(8);
		*(SORT(.u_boot_list*));
		. = ALIGN(8);
		*(.dtb*);
	}

	. = ALIGN(4096);
	.dynamic : { *(.dynamic) }
	. = ALIGN(4096);

	.rela : {
		*(.rela.data*)
		*(.rela.got)
		*(.rela.stab)
	}

	. = ALIGN(4096);
	.dynsym : { *(.dynsym) }
	. = ALIGN(4096);
	.dynstr : { *(.dynstr) }
	. = ALIGN(4096);
	.ignored.reloc : {
		*(.rela.reloc)
		*(.eh_frame)
		*(.note.GNU-stack)
	}

	.comment 0 : { *(.comment) }
}
