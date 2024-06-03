/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Vladimir Zapolskiy <vz@mleia.com>
 * Copyright (C) 2007, 2010 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <asm-offsets.h>
#include <config.h>

	.text
	.align	2

	.global	_start
_start:
	mov.l	._lowlevel_init, r0
100:	bsrf	r0
	nop

	bsr	1f
	nop
1:	sts	pr, r5
	mov.l	._reloc_dst, r4
	add	#(_start-1b), r5
	mov.l	._reloc_dst_end, r6

2:	mov.l	@r5+, r1
	mov.l	r1, @r4
	add	#4, r4
	cmp/hs	r6, r4
	bf	2b

	mov.l	._bss_start, r4
	mov.l	._bss_end, r5
	mov	#0, r1

3:	mov.l	r1, @r4			/* bss clear */
	add	#4, r4
	cmp/hs	r5, r4
	bf	3b

	mov.l	._gd_init, r13		/* global data */
	mov.l	._stack_init, r15	/* stack */

	mov.l	._sh_generic_init, r0
	jsr	@r0
	mov     #0, r4

loop:
	bra	loop

	.align	2

._lowlevel_init:	.long	(lowlevel_init - (100b + 4))
._reloc_dst:		.long	_start
._reloc_dst_end:	.long	reloc_dst_end
._bss_start:		.long	bss_start
._bss_end:		.long	bss_end
._gd_init:		.long	(_start - GENERATED_GBL_DATA_SIZE)
._stack_init:		.long	(_start - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_MALLOC_LEN - 16)
._sh_generic_init:	.long	board_init_f
