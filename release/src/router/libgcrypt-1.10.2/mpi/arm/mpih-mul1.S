/* ARM mul_1 -- Multiply a limb vector with a limb and store the result in
 *              a second limb vector.
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
 * _gcry_mpih_mul_1( mpi_ptr_t res_ptr,		%r0
 *		  mpi_ptr_t s1_ptr,		%r1
 *		  mpi_size_t s1_size,		%r2
 *		  mpi_limb_t s2_limb)		%r3
 */

.text

.globl _gcry_mpih_mul_1
.type  _gcry_mpih_mul_1,%function
_gcry_mpih_mul_1:
	push	{%r4, %r5, %r6, %r7, %r8, %r9, %r10, %r11, %lr};
	mov	%r4, #0;

	tst	%r2, #3;
	beq	.Large_loop;

.Loop:
	ldr	%r5, [%r1], #4;
	mov	%lr, #0;
	umlal	%r4, %lr, %r5, %r3;
	sub	%r2, #1;
	str	%r4, [%r0], #4;
	tst	%r2, #3;
	mov	%r4, %lr;
	bne	.Loop;

	teq	%r2, #0;
	beq	.Lend;

.Large_loop:
	ldm	%r1!, {%r5, %r6, %r7, %r8};
	mov	%r9, #0;
	mov	%r10, #0;
	umlal	%r4, %r9, %r5, %r3;
	mov	%r11, #0;
	umlal	%r9, %r10, %r6, %r3;
	str	%r4, [%r0], #4;
	mov	%r4, #0;
	umlal	%r10, %r11, %r7, %r3;
	subs	%r2, #4;
	umlal	%r11, %r4, %r8, %r3;
	stm	%r0!, {%r9, %r10, %r11};
	bne	.Large_loop;

.Lend:
	mov	%r0, %r4;
	pop	{%r4, %r5, %r6, %r7, %r8, %r9, %r10, %r11, %pc};
.size _gcry_mpih_mul_1,.-_gcry_mpih_mul_1;
