/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * crt0-efi-riscv.S - PE/COFF header for RISC-V EFI applications
 *
 * Copright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
 * Copright (C) 2018 Alexander Graf <agraf@suse.de>
 *
 * This file is inspired by arch/arm/lib/crt0_aarch64_efi.S
 */

#include <asm-generic/pe.h>

#if __riscv_xlen == 64
#define SIZE_LONG	8
#define SAVE_LONG(reg, idx)	sd	reg, (idx*SIZE_LONG)(sp)
#define LOAD_LONG(reg, idx)	ld	reg, (idx*SIZE_LONG)(sp)
#define PE_MACHINE	0x5064
#else
#define SIZE_LONG	4
#define SAVE_LONG(reg, idx)	sw	reg, (idx*SIZE_LONG)(sp)
#define LOAD_LONG(reg, idx)	lw	reg, (idx*SIZE_LONG)(sp)
#define PE_MACHINE	0x5032
#endif


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
	.short	PE_MACHINE			/* RISC-V 64/32-bit */
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
	addi		sp, sp, -(SIZE_LONG * 3)
	SAVE_LONG(a0, 0)
	SAVE_LONG(a1, 1)
	SAVE_LONG(ra, 2)

	lla		a0, ImageBase
	lla		a1, _DYNAMIC
	call		_relocate
	bne		a0, zero, 0f

	LOAD_LONG(a1, 1)
	LOAD_LONG(a0, 0)
	call		efi_main

	LOAD_LONG(ra, 2)

0:	addi		sp, sp, (SIZE_LONG * 3)
	ret
