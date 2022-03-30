/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * U-Boot riscv64 EFI linker script
 *
 * SPDX-License-Identifier:	BSD-2-Clause
 *
 * Modified from arch/arm/lib/elf_aarch64_efi.lds
 */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
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
		. = ALIGN(16);
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
		. = ALIGN(16);
		_bss_end = .;
		_edata = .;
	}
	.rela.dyn : { *(.rela.dyn) }
	.rela.plt : { *(.rela.plt) }
	.rela.got : { *(.rela.got) }
	.rela.data : { *(.rela.data) *(.rela.data*) }
	_data_size = . - _etext;

	. = ALIGN(4096);
	.dynsym   : { *(.dynsym) }
	. = ALIGN(4096);
	.dynstr   : { *(.dynstr) }
	. = ALIGN(4096);
	.note.gnu.build-id : { *(.note.gnu.build-id) }
	/DISCARD/ : {
		*(.rel.reloc)
		*(.eh_frame)
		*(.note.GNU-stack)
	}
	.comment 0 : { *(.comment) }
}
