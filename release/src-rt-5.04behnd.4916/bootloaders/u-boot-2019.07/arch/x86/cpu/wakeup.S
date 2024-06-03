/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 *
 * From coreboot src/arch/x86/wakeup.S
 */

#include <asm/acpi_s3.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>

#define RELOCATED(x)	((x) - __wakeup + WAKEUP_BASE)

#define CODE_SEG	(X86_GDT_ENTRY_16BIT_CS * X86_GDT_ENTRY_SIZE)
#define DATA_SEG	(X86_GDT_ENTRY_16BIT_DS * X86_GDT_ENTRY_SIZE)

	.code32
	.globl __wakeup
__wakeup:
	/* First prepare the jmp to the resume vector */
	mov	0x4(%esp), %eax	/* vector */
	/* last 4 bits of linear addr are taken as offset */
	andw	$0x0f, %ax
	movw	%ax, (__wakeup_offset)
	mov	0x4(%esp), %eax
	/* the rest is taken as segment */
	shr	$4, %eax
	movw	%ax, (__wakeup_segment)

	/* Activate the right segment descriptor real mode */
	ljmp	$CODE_SEG, $RELOCATED(1f)
1:
	/* 16 bit code from here on... */
	.code16

	/*
	 * Load the segment registers w/ properly configured segment
	 * descriptors. They will retain these configurations (limits,
	 * writability, etc.) once protected mode is turned off.
	 */
	mov	$DATA_SEG, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	/* Turn off protection */
	movl	%cr0, %eax
	andl	$~X86_CR0_PE, %eax
	movl	%eax, %cr0

	/* Now really going into real mode */
	ljmp	$0, $RELOCATED(1f)
1:
	movw	$0x0, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	%ax, %fs
	movw	%ax, %gs

	/*
	 * This is a FAR JMP to the OS waking vector.
	 * The C code changes the address to be correct.
	 */
	.byte 0xea

__wakeup_offset = RELOCATED(.)
	.word 0x0000

__wakeup_segment = RELOCATED(.)
	.word 0x0000

	.globl __wakeup_size
__wakeup_size:
	.long . - __wakeup
