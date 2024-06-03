/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2017 NVIDIA Corporation <www.nvidia.com>
 *
 * Derived from Linux kernel v4.14 files:
 *
 * arch/arm64/include/asm/assembler.h:
 * Based on arch/arm/include/asm/assembler.h, arch/arm/mm/proc-macros.S
 * Copyright (C) 1996-2000 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * arch/arm64/kernel/head.S:
 * Based on arch/arm/kernel/head.S
 * Copyright (C) 1994-2002 Russell King
 * Copyright (C) 2003-2012 ARM Ltd.
 * Authors:	Catalin Marinas <catalin.marinas@arm.com>
 *		Will Deacon <will.deacon@arm.com>
 *
 * arch/arm64/kernel/image.h:
 * Copyright (C) 2014 ARM Ltd.
 */

	/*
	 * Emit a 64-bit absolute little endian symbol reference in a way that
	 * ensures that it will be resolved at build time, even when building a
	 * PIE binary. This requires cooperation from the linker script, which
	 * must emit the lo32/hi32 halves individually.
	 */
	.macro	le64sym, sym
	.long	\sym\()_lo32
	.long	\sym\()_hi32
	.endm

.globl _start
_start:
	/*
	 * DO NOT MODIFY. Image header expected by Linux boot-loaders.
	 */
	b	reset				/* branch to kernel start, magic */
	.long	0				/* reserved */
	le64sym	_kernel_offset_le		/* Image load offset from start of RAM, little-endian */
	le64sym	_kernel_size_le			/* Effective size of kernel image, little-endian */
	le64sym	_kernel_flags_le		/* Informative flags, little-endian */
	.quad	0				/* reserved */
	.quad	0				/* reserved */
	.quad	0				/* reserved */
	.ascii	"ARM\x64"			/* Magic number */
	.long	0				/* reserved */
