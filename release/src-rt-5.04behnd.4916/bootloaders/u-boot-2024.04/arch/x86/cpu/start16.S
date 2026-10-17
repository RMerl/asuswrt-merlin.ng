/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  U-Boot - x86 Startup Code
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002,2003
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#include <asm/processor-flags.h>

#define BOOT_SEG	0xffff0000	/* linear segment of boot code */

.section .start16, "ax"
.code16
.globl start16
start16:
	/* Save BIST */
	movl	%eax, %ecx

	xorl	%eax, %eax
	movl	%eax, %cr3	/* Invalidate TLB */

	/* Turn off cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0
	wbinvd

	/* load the temporary Global Descriptor Table */
data32 cs	lidt	idt_ptr
data32 cs	lgdt	gdt_ptr

	/* Now, we enter protected mode */
	movl	%cr0, %eax
	orl	$X86_CR0_PE, %eax
	movl	%eax, %cr0

	/* Flush the prefetch queue */
	jmp	ff
ff:

	/* Finally restore BIST and jump to the 32-bit initialization code */
	movl	%ecx, %eax
data32 cs	ljmp	*code32start

	/* 48-bit far pointer */
code32start:
	.long	_start		/* offset */
	.word	0x10		/* segment */

idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

	/*
	 * The following Global Descriptor Table is just enough to get us into
	 * 'Flat Protected Mode' - It will be discarded as soon as the final
	 * GDT is setup in a safe location in RAM
	 */
gdt_ptr:
	.word	0x1f		/* limit (31 bytes = 4 GDT entries - 1) */
	.long	BOOT_SEG + gdt_rom	/* base */

	/* Some CPUs are picky about GDT alignment... */
	.align	16
.globl gdt_rom
gdt_rom:
	/*
	 * The GDT table ...
	 *
	 *	 Selector	Type
	 *	 0x00		NULL
	 *	 0x08		Unused
	 *	 0x10		32bit code
	 *	 0x18		32bit data/stack
	 */
	/* The NULL Desciptor - Mandatory */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/* Unused Desciptor - (matches Linux) */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Code Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Exec (Code), Readable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x9b		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Data Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Non-Exec (Data), Writable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x93		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */
