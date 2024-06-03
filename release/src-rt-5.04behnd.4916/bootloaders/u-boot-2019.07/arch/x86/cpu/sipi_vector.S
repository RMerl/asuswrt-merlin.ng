/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Google, Inc
 *
 * Taken from coreboot file of the same name
 */

/*
 * The SIPI vector is responsible for initializing the APs in the sytem. It
 * loads microcode, sets up MSRs, and enables caching before calling into
 * C code
 */

#include <asm/global_data.h>
#include <asm/msr-index.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <asm/sipi.h>

#define CODE_SEG	(X86_GDT_ENTRY_32BIT_CS * X86_GDT_ENTRY_SIZE)
#define DATA_SEG	(X86_GDT_ENTRY_32BIT_DS * X86_GDT_ENTRY_SIZE)

/*
 * First we have the 16-bit section. Every AP process starts here.
 * The simple task is to load U-Boot's Global Descriptor Table (GDT) to allow
 * U-Boot's 32-bit code to become visible, then jump to ap_start.
 *
 * Note that this code is copied to RAM below 1MB in mp_init.c, and runs from
 * there, but the 32-bit code (ap_start and onwards) is part of U-Boot and
 * is therefore relocated to the top of RAM with other U-Boot code. This
 * means that for the 16-bit code we must write relocatable code, but for the
 * rest, we can do what we like.
 */
.text
.code16
.globl ap_start16
ap_start16:
	cli
	xorl	%eax, %eax
	movl	%eax, %cr3		/* Invalidate TLB */

	/* setup the data segment */
	movw	%cs, %ax
	movw	%ax, %ds

	/* Use an address relative to the data segment for the GDT */
	movl	$gdtaddr, %ebx
	subl	$ap_start16, %ebx

	data32 lgdt (%ebx)

	movl	%cr0, %eax
	andl	$(~(X86_CR0_PG | X86_CR0_AM | X86_CR0_WP | X86_CR0_NE | \
		    X86_CR0_TS | X86_CR0_EM | X86_CR0_MP)), %eax
	orl	$(X86_CR0_NW | X86_CR0_CD | X86_CR0_PE), %eax
	movl	%eax, %cr0

	movl	$ap_start_jmp, %eax
	subl	$ap_start16, %eax
	movw	%ax, %bp

	/* Jump to ap_start within U-Boot */
data32 cs	ljmp	*(%bp)

	.align	4
.globl sipi_params_16bit
sipi_params_16bit:
	/* 48-bit far pointer */
ap_start_jmp:
	.long	0		/* offset set to ap_start by U-Boot */
	.word	CODE_SEG	/* segment */

	.word	0		/* padding */
gdtaddr:
	.word	0 /* limit */
	.long	0 /* table */
	.word	0 /* unused */

.globl ap_start16_code_end
ap_start16_code_end:

/*
 * Set up the special 'fs' segment for global_data. Then jump to ap_continue
 * to set up the AP.
 */
.globl ap_start
ap_start:
	.code32
	movw	$DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movw	%ax, %gs

	movw	$(X86_GDT_ENTRY_32BIT_FS * X86_GDT_ENTRY_SIZE), %ax
	movw	%ax, %fs

	/* Load the Interrupt descriptor table */
	mov	idt_ptr, %ebx
	lidt	(%ebx)

	/* Obtain cpu number */
	movl	ap_count, %eax
1:
	movl	%eax, %ecx
	inc	%ecx
	lock cmpxchg %ecx, ap_count
	jnz	1b

	/* Setup stacks for each CPU */
	movl	stack_size, %eax
	mul	%ecx
	movl	stack_top, %edx
	subl	%eax, %edx
	mov	%edx, %esp
	/* Save cpu number */
	mov	%ecx, %esi

	/* Determine if one should check microcode versions */
	mov	microcode_ptr, %edi
	test	%edi, %edi
	jz	microcode_done /* Bypass if no microde exists */

	/* Get the Microcode version */
	mov	$1, %eax
	cpuid
	mov	$MSR_IA32_UCODE_REV, %ecx
	rdmsr
	/* If something already loaded skip loading again */
	test	%edx, %edx
	jnz	microcode_done

	/* Determine if parallel microcode loading is allowed */
	cmp	$0xffffffff, microcode_lock
	je	load_microcode

	/* Protect microcode loading */
lock_microcode:
	lock bts $0, microcode_lock
	jc	lock_microcode

load_microcode:
	/* Load new microcode */
	mov	$MSR_IA32_UCODE_WRITE, %ecx
	xor	%edx, %edx
	mov	%edi, %eax
	/*
	 * The microcode pointer is passed in pointing to the header. Adjust
	 * pointer to reflect the payload (header size is 48 bytes)
	 */
	add	$UCODE_HEADER_LEN, %eax
	pusha
	wrmsr
	popa

	/* Unconditionally unlock microcode loading */
	cmp	$0xffffffff, microcode_lock
	je	microcode_done

	xor	%eax, %eax
	mov	%eax, microcode_lock

microcode_done:
	/*
	 * Load MSRs. Each entry in the table consists of:
	 * 0: index,
	 * 4: value[31:0]
	 * 8: value[63:32]
	 * See struct saved_msr in mp_init.c.
	 */
	mov	msr_table_ptr, %edi
	mov	msr_count, %ebx
	test	%ebx, %ebx
	jz	1f
load_msr:
	mov	(%edi), %ecx
	mov	4(%edi), %eax
	mov	8(%edi), %edx
	wrmsr
	add	$12, %edi
	dec	%ebx
	jnz	load_msr

1:
	/* Enable caching */
	mov	%cr0, %eax
	andl	$(~(X86_CR0_CD | X86_CR0_NW)), %eax
	mov	%eax, %cr0

	/* c_handler(cpu_num) */
	movl	%esi, %eax	/* cpu_num */
	mov	c_handler, %esi
	call	*%esi

	/* This matches struct sipi_param */
	.align	4
.globl	sipi_params
sipi_params:
idt_ptr:
	.long 0
stack_top:
	.long 0
stack_size:
	.long 0
microcode_lock:
	.long 0
microcode_ptr:
	.long 0
msr_table_ptr:
	.long 0
msr_count:
	.long 0
c_handler:
	.long 0
ap_count:
	.long 0
