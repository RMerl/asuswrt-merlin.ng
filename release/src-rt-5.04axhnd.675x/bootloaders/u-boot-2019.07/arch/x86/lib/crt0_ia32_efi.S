/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * crt0-efi-ia32.S - x86 EFI startup code.
 *
 * Copyright (C) 1999 Hewlett-Packard Co.
 * Contributed by David Mosberger <davidm@hpl.hp.com>.
 * All rights reserved.
 */

	.text
	.align 4

	.globl _start
_start:
	pushl %ebp
	movl %esp,%ebp

	pushl 12(%ebp)			# copy "image" argument
	pushl  8(%ebp)			# copy "systab" argument

	call 0f
0:	popl %eax
	movl %eax,%ebx

	addl $image_base-0b,%eax	# %eax = ldbase
	addl $_DYNAMIC-0b,%ebx		# %ebx = _DYNAMIC

	pushl %ebx			# pass _DYNAMIC as second argument
	pushl %eax			# pass ldbase as first argument
	call _relocate
	popl %ebx
	popl %ebx
	testl %eax,%eax
	jne .exit
	call efi_main		# call app with "image" and "systab" argument

.exit:	leave
	ret

	/*
	 * hand-craft a dummy .reloc section so EFI knows it's a relocatable
	 * executable:
	 */
	.data
dummy:	.long	0

#define IMAGE_REL_ABSOLUTE	0
	.section .reloc
	.long	dummy					/* Page RVA */
	.long	10					/* Block Size (2*4+2) */
	.word	(IMAGE_REL_ABSOLUTE << 12) +  0		/* reloc for dummy */
