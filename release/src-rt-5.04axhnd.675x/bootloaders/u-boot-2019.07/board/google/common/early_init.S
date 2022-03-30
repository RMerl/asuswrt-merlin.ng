/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014 Google, Inc
 */

.globl early_board_init
early_board_init:
	/* Enable post codes to EC */
#ifdef CONFIG_EARLY_POST_CROS_EC
	mov    $0x1b, %ecx
	rdmsr
	and    $0x100, %eax
	test   %eax, %eax
	je     1f

	mov    $0x8000f8f0, %eax
	mov    $0xcf8, %dx
	out    %eax, (%dx)
	mov    $0xfed1c001, %eax
	mov    $0xcfc, %dx
	out    %eax, (%dx)
	mov    $0xfed1f410, %esp
	mov    (%esp), %eax
	and    $0xfffffffb, %eax
	mov    %eax, (%esp)
1:
#endif
	jmp	early_board_init_ret
