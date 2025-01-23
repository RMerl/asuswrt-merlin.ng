/* ARM mul_2 -- Multiply a limb vector with a limb and add the result to
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
 * _gcry_mpih_addmul_1( mpi_ptr_t res_ptr,	%r0
 *		     mpi_ptr_t s1_ptr,		%r1
 *		     mpi_size_t s1_size,	%r2
 *		     mpi_limb_t s2_limb)	%r3
 */

.text

.globl _gcry_mpih_addmul_1
.type  _gcry_mpih_addmul_1,%function
_gcry_mpih_addmul_1:
	push	{%r4, %r5, %r6, %r8, %r10, %lr};
	mov	%lr, #0;
	cmn	%r0, #0; /* clear carry flag */

	tst	%r2, #3;
	beq	.Large_loop;
.Loop:
	ldr	%r5, [%r1], #4;
	ldr	%r4, [%r0];
	sub	%r2, #1;
	adcs	%r4, %lr;
	mov	%lr, #0;
	umlal	%r4, %lr, %r5, %r3;
	tst	%r2, #3;
	str	%r4, [%r0], #4;
	bne	.Loop;

	teq	%r2, #0;
	beq	.Lend;

.Large_loop:
	ldr	%r5, [%r1], #4;
	ldm	%r0, {%r4, %r6, %r8, %r10};

	sub	%r2, #4;
	adcs	%r4, %lr;
	mov	%lr, #0;
	umlal	%r4, %lr, %r5, %r3;

	ldr	%r5, [%r1], #4;
	adcs	%r6, %lr;
	mov	%lr, #0;
	umlal	%r6, %lr, %r5, %r3;

	ldr	%r5, [%r1], #4;
	adcs	%r8, %lr;
	mov	%lr, #0;
	umlal	%r8, %lr, %r5, %r3;

	ldr	%r5, [%r1], #4;
	adcs	%r10, %lr;
	mov	%lr, #0;
	umlal	%r10, %lr, %r5, %r3;

	teq	%r2, #0;
	stm	%r0!, {%r4, %r6, %r8, %r10};
	bne	.Large_loop;

.Lend:
	adc	%r0, %lr, #0;
	pop	{%r4, %r5, %r6, %r8, %r10, %pc};
.size _gcry_mpih_addmul_1,.-_gcry_mpih_addmul_1;
