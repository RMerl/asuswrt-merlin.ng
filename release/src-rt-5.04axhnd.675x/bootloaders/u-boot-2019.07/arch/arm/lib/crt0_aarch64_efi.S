/* SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause */
/*
 * crt0-efi-aarch64.S - PE/COFF header for aarch64 EFI applications
 *
 * Copright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
 *
 *
 * This file is taken and modified from the gnu-efi project.
 */

#include <asm-generic/pe.h>

	.section	.text.head

	/*
	 * Magic "MZ" signature for PE/COFF
	 */
	.globl	ImageBase
ImageBase:
	.ascii	"MZ"
	.skip	58				/* 'MZ' + pad + offset == 64 */
	.long	pe_header - ImageBase		/* Offset to the PE header */
pe_header:
	.ascii	"PE"
	.short	0
coff_header:
	.short	0xaa64				/* AArch64 */
	.short	2				/* nr_sections */
	.long	0				/* TimeDateStamp */
	.long	0				/* PointerToSymbolTable */
	.long	0				/* NumberOfSymbols */
	.short	section_table - optional_header	/* SizeOfOptionalHeader */
	/* Characteristics */
	.short	(IMAGE_FILE_EXECUTABLE_IMAGE | \
		 IMAGE_FILE_LINE_NUMS_STRIPPED | \
		 IMAGE_FILE_LOCAL_SYMS_STRIPPED | \
		 IMAGE_FILE_DEBUG_STRIPPED)
optional_header:
	.short	0x20b				/* PE32+ format */
	.byte	0x02				/* MajorLinkerVersion */
	.byte	0x14				/* MinorLinkerVersion */
	.long	_edata - _start			/* SizeOfCode */
	.long	0				/* SizeOfInitializedData */
	.long	0				/* SizeOfUninitializedData */
	.long	_start - ImageBase		/* AddressOfEntryPoint */
	.long	_start - ImageBase		/* BaseOfCode */

extra_header_fields:
	.quad	0				/* ImageBase */
	.long	0x20				/* SectionAlignment */
	.long	0x8				/* FileAlignment */
	.short	0				/* MajorOperatingSystemVersion */
	.short	0				/* MinorOperatingSystemVersion */
	.short	0				/* MajorImageVersion */
	.short	0				/* MinorImageVersion */
	.short	0				/* MajorSubsystemVersion */
	.short	0				/* MinorSubsystemVersion */
	.long	0				/* Win32VersionValue */

	.long	_edata - ImageBase		/* SizeOfImage */

	/*
	 * Everything before the kernel image is considered part of the header
	 */
	.long	_start - ImageBase		/* SizeOfHeaders */
	.long	0				/* CheckSum */
	.short	IMAGE_SUBSYSTEM_EFI_APPLICATION /* Subsystem */
	.short	0				/* DllCharacteristics */
	.quad	0				/* SizeOfStackReserve */
	.quad	0				/* SizeOfStackCommit */
	.quad	0				/* SizeOfHeapReserve */
	.quad	0				/* SizeOfHeapCommit */
	.long	0				/* LoaderFlags */
	.long	0x6				/* NumberOfRvaAndSizes */

	.quad	0				/* ExportTable */
	.quad	0				/* ImportTable */
	.quad	0				/* ResourceTable */
	.quad	0				/* ExceptionTable */
	.quad	0				/* CertificationTable */
	.quad	0				/* BaseRelocationTable */

	/* Section table */
section_table:

	/*
	 * The EFI application loader requires a relocation section
	 * because EFI applications must be relocatable.  This is a
	 * dummy section as far as we are concerned.
	 */
	.ascii	".reloc"
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	0
	.long	0
	.long	0			/* SizeOfRawData */
	.long	0			/* PointerToRawData */
	.long	0			/* PointerToRelocations */
	.long	0			/* PointerToLineNumbers */
	.short	0			/* NumberOfRelocations */
	.short	0			/* NumberOfLineNumbers */
	.long	0x42100040		/* Characteristics (section flags) */


	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	_edata - _start		/* VirtualSize */
	.long	_start - ImageBase	/* VirtualAddress */
	.long	_edata - _start		/* SizeOfRawData */
	.long	_start - ImageBase	/* PointerToRawData */

	.long	0		/* PointerToRelocations (0 for executables) */
	.long	0		/* PointerToLineNumbers (0 for executables) */
	.short	0		/* NumberOfRelocations  (0 for executables) */
	.short	0		/* NumberOfLineNumbers  (0 for executables) */
	.long	0xe0500020	/* Characteristics (section flags) */

_start:
	stp		x29, x30, [sp, #-32]!
	mov		x29, sp

	stp		x0, x1, [sp, #16]
	adr		x0, ImageBase
	adrp		x1, _DYNAMIC
	add		x1, x1, #:lo12:_DYNAMIC
	bl		_relocate
	cbnz		x0, 0f

	ldp		x0, x1, [sp, #16]
	bl		efi_main

0:	ldp		x29, x30, [sp], #32
	ret
