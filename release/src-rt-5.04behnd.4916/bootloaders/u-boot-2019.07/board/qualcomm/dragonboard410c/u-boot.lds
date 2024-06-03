/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Override linker script for fastboot-readable images
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Based on arch/arm/cpu/armv8/u-boot.lds (Just add header)
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_arm64_header)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(8);
	.text :
	{
		*(.__image_copy_start)
		board/qualcomm/dragonboard410c/head.o (.text*)
		CPUDIR/start.o (.text*)
	}

	/* This needs to come before *(.text*) */
	.efi_runtime : {
                __efi_runtime_start = .;
		*(.text.efi_runtime*)
		*(.rodata.efi_runtime*)
		*(.data.efi_runtime*)
                __efi_runtime_stop = .;
	}

	.text_rest :
	{
		*(.text*)
	}

	. = ALIGN(8);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(8);
	.data : {
		*(.data*)
	}

	. = ALIGN(8);

	. = .;

	. = ALIGN(8);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(8);

	.efi_runtime : {
                __efi_runtime_start = .;
		*(efi_runtime_text)
		*(efi_runtime_data)
                __efi_runtime_stop = .;
	}

	.efi_runtime_rel : {
                __efi_runtime_rel_start = .;
		*(.rel*.efi_runtime)
		*(.rel*.efi_runtime.*)
                __efi_runtime_rel_stop = .;
	}

	. = ALIGN(8);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	. = ALIGN(8);

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rela.dyn : {
		*(.rela*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	_end = .;

	. = ALIGN(8);

	.bss_start : {
		KEEP(*(.__bss_start));
	}

	.bss : {
		*(.bss*)
		 . = ALIGN(8);
	}

	.bss_end : {
		KEEP(*(.__bss_end));
	}

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
