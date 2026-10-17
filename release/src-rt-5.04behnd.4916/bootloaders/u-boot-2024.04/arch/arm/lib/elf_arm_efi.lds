/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot ARM EFI linker script
 *
 * Modified from elf_arm_efi.lds in gnu-efi
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.text 0x0 : {
		_text = .;
		*(.text.head)
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		*(.srodata)
		*(.rodata*)
		. = ALIGN(512);
	}
	_etext = .;
	_text_size = . - _text;
	.dynamic  : { *(.dynamic) }
	.data : {
		_data = .;
		*(.sdata)
		*(.data)
		*(.data1)
		*(.data.*)
		*(.got.plt)
		*(.got)

		/*
		 * The EFI loader doesn't seem to like a .bss section, so we
		 * stick it all into .data:
		 */
		. = ALIGN(16);
		_bss = .;
		*(.sbss)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(512);
		_bss_end = .;
		_edata = .;
	}
	.rel.dyn : { *(.rel.dyn) }
	.rel.plt : { *(.rel.plt) }
	.rel.got : { *(.rel.got) }
	.rel.data : { *(.rel.data) *(.rel.data*) }
	_data_size = . - _etext;

	/DISCARD/ : {
		*(.rel.reloc)
		*(.eh_frame)
		*(.note.GNU-stack)
		*(.dynsym)
		*(.dynstr)
		*(.note.gnu.build-id)
		*(.comment)
	}
}
