/* SPDX-License-Identifier: MIT */
/*
 * memset - fill memory with a constant byte
 *
 * Copyright (c) 2012-2021, Arm Limited.
 */

/* Assumptions:
 *
 * ARMv8-a, AArch64, Advanced SIMD, unaligned accesses.
 *
 */

#include <asm/macro.h>
#include "asmdefs.h"

#define dstin	x0
#define val	x1
#define valw	w1
#define count	x2
#define dst	x3
#define dstend	x4
#define zva_val	x5

ENTRY (memset)
	PTR_ARG (0)
	SIZE_ARG (2)

	/*
	 * The optimized memset uses the dc opcode, which causes problems
	 * when the cache is disabled. Let's check if the cache is disabled
	 * and use a very simple memset implementation in this case. Otherwise
	 * jump to the optimized version.
	 */
	switch_el x6, 3f, 2f, 1f
3:	mrs	x6, sctlr_el3
	b	0f
2:	mrs	x6, sctlr_el2
	b	0f
1:	mrs	x6, sctlr_el1
0:
	tst	x6, #CR_C
	bne	9f

	/*
	 * A very "simple" memset implementation without the use of the
	 * dc opcode. Can be run with caches disabled.
	 */
	mov	x3, #0x0
	cmp	count, x3	/* check for zero length */
	beq	8f
4:	strb	valw, [dstin, x3]
	add	x3, x3, #0x1
	cmp	count, x3
	bne	4b
8:	ret
9:

	/* Here the optimized memset version starts */
	dup	v0.16B, valw
	add	dstend, dstin, count

	cmp	count, 96
	b.hi	L(set_long)
	cmp	count, 16
	b.hs	L(set_medium)
	mov	val, v0.D[0]

	/* Set 0..15 bytes.  */
	tbz	count, 3, 1f
	str	val, [dstin]
	str	val, [dstend, -8]
	ret
	.p2align 4
1:	tbz	count, 2, 2f
	str	valw, [dstin]
	str	valw, [dstend, -4]
	ret
2:	cbz	count, 3f
	strb	valw, [dstin]
	tbz	count, 1, 3f
	strh	valw, [dstend, -2]
3:	ret

	/* Set 17..96 bytes.  */
L(set_medium):
	str	q0, [dstin]
	tbnz	count, 6, L(set96)
	str	q0, [dstend, -16]
	tbz	count, 5, 1f
	str	q0, [dstin, 16]
	str	q0, [dstend, -32]
1:	ret

	.p2align 4
	/* Set 64..96 bytes.  Write 64 bytes from the start and
	   32 bytes from the end.  */
L(set96):
	str	q0, [dstin, 16]
	stp	q0, q0, [dstin, 32]
	stp	q0, q0, [dstend, -32]
	ret

	.p2align 4
L(set_long):
	and	valw, valw, 255
	bic	dst, dstin, 15
	str	q0, [dstin]
	cmp	count, 160
	ccmp	valw, 0, 0, hs
	b.ne	L(no_zva)

#ifndef SKIP_ZVA_CHECK
	mrs	zva_val, dczid_el0
	and	zva_val, zva_val, 31
	cmp	zva_val, 4		/* ZVA size is 64 bytes.  */
	b.ne	L(no_zva)
#endif
	str	q0, [dst, 16]
	stp	q0, q0, [dst, 32]
	bic	dst, dst, 63
	sub	count, dstend, dst	/* Count is now 64 too large.  */
	sub	count, count, 128	/* Adjust count and bias for loop.  */

	.p2align 4
L(zva_loop):
	add	dst, dst, 64
	dc	zva, dst
	subs	count, count, 64
	b.hi	L(zva_loop)
	stp	q0, q0, [dstend, -64]
	stp	q0, q0, [dstend, -32]
	ret

L(no_zva):
	sub	count, dstend, dst	/* Count is 16 too large.  */
	sub	dst, dst, 16		/* Dst is biased by -32.  */
	sub	count, count, 64 + 16	/* Adjust count and bias for loop.  */
L(no_zva_loop):
	stp	q0, q0, [dst, 32]
	stp	q0, q0, [dst, 64]!
	subs	count, count, 64
	b.hi	L(no_zva_loop)
	stp	q0, q0, [dstend, -64]
	stp	q0, q0, [dstend, -32]
	ret

END (memset)
