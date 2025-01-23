/* ARM mul_3 -- Multiply a limb vector with a limb and subtract the result
 *              from a second limb vector.
 *
 *      Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Note: This code is heavily based on the GNU MP Library (version 4.2.1).
 */

#include "sysdep.h"
#include "asm-syntax.h"

.syntax unified
.arm

/*******************
 * mpi_limb_t
 * _gcry_mpih_submul_1( mpi_ptr_t res_ptr,	%r0
 *		     mpi_ptr_t s1_ptr,		%r1
 *		     mpi_size_t s1_size,	%r2
 *		     mpi_limb_t s2_limb)	%r3
 */

.text

.globl _gcry_mpih_submul_1
.type  _gcry_mpih_submul_1,%function
_gcry_mpih_submul_1:
	push	{%r4, %r5, %r6, %r8, %r9, %r10, %lr};
	mov	%lr, #0;
	cmp	%r0, #0; /* prepare carry flag for sbc */

	tst	%r2, #3;
	beq	.Large_loop;
.Loop:
	ldr	%r5, [%r1], #4;
	mov	%r4, %lr;
	mov	%lr, #0;
	ldr	%r6, [%r0];
	umlal	%r4, %lr, %r5, %r3;
	sub	%r2, #1;
	sbcs	%r4, %r6, %r4;
	tst	%r2, #3;
	str	%r4, [%r0], #4;
	bne	.Loop;

	teq	%r2, #0;
	beq	.Lend;

.Large_loop:
	ldr	%r5, [%r1], #4;
	mov	%r9, #0;
	ldr	%r4, [%r0, #0];

	umlal	%lr, %r9, %r5, %r3;
	ldr	%r6, [%r0, #4];
	ldr	%r5, [%r1], #4;
	sbcs	%r4, %r4, %lr;

	mov	%lr, #0;
	umlal	%r9, %lr, %r5, %r3;
	ldr	%r8, [%r0, #8];
	ldr	%r5, [%r1], #4;
	sbcs	%r6, %r6, %r9;

	mov	%r9, #0;
	umlal	%lr, %r9, %r5, %r3;
	ldr	%r10, [%r0, #12];
	ldr	%r5, [%r1], #4;
	sbcs	%r8, %r8, %lr;

	mov	%lr, #0;
	umlal	%r9, %lr, %r5, %r3;
	sub	%r2, #4;
	sbcs	%r10, %r10, %r9;

	teq	%r2, #0;
	stm	%r0!, {%r4, %r6, %r8, %r10};
	bne	.Large_loop;

.Lend:
	it	cc
	movcc	%r2, #1;
	add	%r0, %lr, %r2;
	pop	{%r4, %r5, %r6, %r8, %r9, %r10, %pc};
.size _gcry_mpih_submul_1,.-_gcry_mpih_submul_1;
