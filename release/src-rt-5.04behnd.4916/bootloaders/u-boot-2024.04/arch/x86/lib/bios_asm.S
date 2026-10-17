/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot x86_asm.S, cleaned up substantially
 *
 * Copyright (C) 2009-2010 coresystems GmbH
 */

#include <asm/processor.h>
#include <asm/processor-flags.h>
#include "bios.h"

#define SEG(segment)	$segment * X86_GDT_ENTRY_SIZE

/*
 * This is the interrupt handler stub code. It gets copied to the IDT and
 * to some fixed addresses in the F segment. Before the code can used,
 * it gets patched up by the C function copying it: byte 3 (the $0 in
 * movb $0, %al) is overwritten with the interrupt numbers.
 */

	.code16
	.globl __idt_handler
__idt_handler:
	pushal
	movb	$0, %al /* This instruction gets modified */
	ljmp	$0, $__interrupt_handler_16bit
	.globl __idt_handler_size
__idt_handler_size:
	.long  . - __idt_handler

.macro setup_registers
	/* initial register values */
	movl	44(%ebp), %eax
	movl	%eax, __registers +  0 /* eax */
	movl	48(%ebp), %eax
	movl	%eax, __registers +  4 /* ebx */
	movl	52(%ebp), %eax
	movl	%eax, __registers +  8 /* ecx */
	movl	56(%ebp), %eax
	movl	%eax, __registers + 12 /* edx */
	movl	60(%ebp), %eax
	movl	%eax, __registers + 16 /* esi */
	movl	64(%ebp), %eax
	movl	%eax, __registers + 20 /* edi */
.endm

.macro	enter_real_mode
	/* Activate the right segment descriptor real mode. */
	ljmp	SEG(X86_GDT_ENTRY_16BIT_CS), $PTR_TO_REAL_MODE(1f)
1:
.code16
	/*
	 * Load the segment registers with properly configured segment
	 * descriptors. They will retain these configurations (limits,
	 * writability, etc.) once protected mode is turned off.
	 */
	mov	SEG(X86_GDT_ENTRY_16BIT_DS), %ax
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
	ljmp	$0, $PTR_TO_REAL_MODE(1f)
1:
	/*
	 * Set up a stack: Put the stack at the end of page zero. That way
	 * we can easily share it between real and protected, since the
	 * 16-bit ESP at segment 0 will work for any case.
	 */
	mov	$0x0, %ax
	mov	%ax, %ss

	/* Load 16 bit IDT */
	xor	%ax, %ax
	mov	%ax, %ds
	lidt	__realmode_idt

.endm

.macro	prepare_for_irom
	movl	$0x1000, %eax
	movl	%eax, %esp

	/* Initialise registers for option rom lcall */
	movl	__registers +  0, %eax
	movl	__registers +  4, %ebx
	movl	__registers +  8, %ecx
	movl	__registers + 12, %edx
	movl	__registers + 16, %esi
	movl	__registers + 20, %edi

	/* Set all segments to 0x0000, ds to 0x0040 */
	push	%ax
	xor	%ax, %ax
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	SEG(X86_GDT_ENTRY_16BIT_FLAT_DS), %ax
	mov	%ax, %ds
	pop	%ax

.endm

.macro	enter_protected_mode
	/* Go back to protected mode */
	movl	%cr0, %eax
	orl	$X86_CR0_PE, %eax
	movl	%eax, %cr0

	/* Now that we are in protected mode jump to a 32 bit code segment */
	data32	ljmp	SEG(X86_GDT_ENTRY_32BIT_CS), $PTR_TO_REAL_MODE(1f)
1:
	.code32
	mov	SEG(X86_GDT_ENTRY_32BIT_DS), %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %gs
	mov	%ax, %ss
	mov	SEG(X86_GDT_ENTRY_32BIT_FS), %ax
	mov	%ax, %fs

	/* restore proper idt */
	lidt	idt_ptr
.endm

/*
 * In order to be independent of U-Boot's position in RAM we relocate a part
 * of the code to the first megabyte of RAM, so the CPU can use it in
 * real-mode. This code lives at asm_realmode_code.
 */
	.globl asm_realmode_code
asm_realmode_code:

/* Realmode IDT pointer structure. */
__realmode_idt = PTR_TO_REAL_MODE(.)
	.word 1023	/* 16 bit limit */
	.long 0		/* 24 bit base */
	.word 0

/* Preserve old stack */
__stack = PTR_TO_REAL_MODE(.)
	.long 0

/* Register store for realmode_call and realmode_interrupt */
__registers = PTR_TO_REAL_MODE(.)
	.long 0 /*  0 - EAX */
	.long 0 /*  4 - EBX */
	.long 0 /*  8 - ECX */
	.long 0 /* 12 - EDX */
	.long 0 /* 16 - ESI */
	.long 0 /* 20 - EDI */

/* 256 byte buffer, used by int10 */
	.globl asm_realmode_buffer
asm_realmode_buffer:
	.skip 256

	.code32
	.globl asm_realmode_call
asm_realmode_call:
	/* save all registers to the stack */
	pusha
	pushf
	movl	%esp, __stack
	movl	%esp, %ebp

	/*
	 * This function is called with regparm=0 and we have to skip the
	 * 36 bytes from pushf+pusha. Hence start at 40.
	 * Set up our call instruction.
	 */
	movl	40(%ebp), %eax
	mov	%ax, __lcall_instr + 1
	andl	$0xffff0000, %eax
	shrl	$4, %eax
	mov	%ax, __lcall_instr + 3

	wbinvd

	setup_registers
	enter_real_mode
	prepare_for_irom

__lcall_instr = PTR_TO_REAL_MODE(.)
	.byte 0x9a
	.word 0x0000, 0x0000

	enter_protected_mode

	/* restore stack pointer, eflags and register values and exit */
	movl	__stack, %esp
	popf
	popa
	ret

	.globl __realmode_interrupt
__realmode_interrupt:
	/* save all registers to the stack and store the stack pointer */
	pusha
	pushf
	movl	%esp, __stack
	movl	%esp, %ebp

	/*
	 * This function is called with regparm=0 and we have to skip the
	 * 36 bytes from pushf+pusha. Hence start at 40.
	 * Prepare interrupt calling code.
	 */
	movl	40(%ebp), %eax
	movb	%al, __intXX_instr + 1 /* intno */

	setup_registers
	enter_real_mode
	prepare_for_irom

__intXX_instr = PTR_TO_REAL_MODE(.)
	.byte 0xcd, 0x00 /* This becomes intXX */

	enter_protected_mode

	/* restore stack pointer, eflags and register values and exit */
	movl	__stack, %esp
	popf
	popa
	ret

/*
 * This is the 16-bit interrupt entry point called by the IDT stub code.
 *
 * Before this code code is called, %eax is pushed to the stack, and the
 * interrupt number is loaded into %al. On return this function cleans up
 * for its caller.
 */
	.code16
__interrupt_handler_16bit = PTR_TO_REAL_MODE(.)
	push	%ds
	push	%es
	push	%fs
	push	%gs

	/* Save real mode SS */
	movw	%ss, %cs:__realmode_ss

	/* Clear DF to not break ABI assumptions */
	cld

	/*
	 * Clean up the interrupt number. We could do this in the stub, but
	 * it would cost two more bytes per stub entry.
	 */
	andl	$0xff, %eax
	pushl	%eax		/* ... and make it the first parameter */

	enter_protected_mode

	/*
	 * Now we are in protected mode. We need compute the right ESP based
	 * on saved real mode SS otherwise interrupt_handler() won't get
	 * correct parameters from the stack.
	 */
	movzwl	%cs:__realmode_ss, %ecx
	shll	$4, %ecx
	addl	%ecx, %esp

	/* Call the C interrupt handler */
	movl	$interrupt_handler, %eax
	call	*%eax

	/* Restore real mode ESP based on saved SS */
	movzwl	%cs:__realmode_ss, %ecx
	shll	$4, %ecx
	subl	%ecx, %esp

	enter_real_mode

	/* Restore real mode SS */
	movw	%cs:__realmode_ss, %ss

	/*
	 * Restore all registers, including those manipulated by the C
	 * handler
	 */
	popl	%eax
	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

__realmode_ss = PTR_TO_REAL_MODE(.)
	.word	0

	.globl asm_realmode_code_size
asm_realmode_code_size:
	.long  . - asm_realmode_code
