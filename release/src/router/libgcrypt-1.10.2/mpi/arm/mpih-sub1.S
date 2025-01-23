/* ARM sub_n -- Subtract two limb vectors of the same length > 0 and store
 *              sum in a third limb vector.
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
 *  mpi_limb_t
 *  _gcry_mpih_sub_n( mpi_ptr_t res_ptr,	%r0
 *		   mpi_ptr_t s1_ptr,		%r1
 *		   mpi_ptr_t s2_ptr,		%r2
 *		   mpi_size_t size)		%r3
 */

.text

.globl _gcry_mpih_sub_n
.type  _gcry_mpih_sub_n,%function
_gcry_mpih_sub_n:
	push	{%r4, %r5, %r6, %r7, %r8, %r9, %r10, %lr};
	cmp	%r0, #0; /* prepare carry flag for sub */

	tst	%r3, #3;
	beq	.Large_loop;

.Loop:
	ldr	%r4, [%r1], #4;
	sub	%r3, #1;
	ldr	%lr, [%r2], #4;
	sbcs	%r4, %lr;
	tst	%r3, #3;
	str	%r4, [%r0], #4;
	bne	.Loop;

	teq	%r3, #0;
	beq	.Lend;

.Large_loop:
	ldm	%r1!, {%r4, %r6, %r8, %r10};
	sub	%r3, #4;
	ldm	%r2!, {%r5, %r7, %r9, %lr};
	sbcs	%r4, %r5;
	sbcs	%r6, %r7;
	sbcs	%r8, %r9;
	sbcs	%r10, %lr;
	teq	%r3, #0;
	stm	%r0!, {%r4, %r6, %r8, %r10};
	bne	.Large_loop;

.Lend:
	sbc	%r0, %r3, #0;
	neg	%r0, %r0;
	pop	{%r4, %r5, %r6, %r7, %r8, %r9, %r10, %pc};
.size _gcry_mpih_sub_n,.-_gcry_mpih_sub_n;
