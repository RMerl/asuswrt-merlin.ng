/* ARM64 mul_1 -- Multiply a limb vector with a limb and store the result in
 *                a second limb vector.
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
 */

#include "sysdep.h"
#include "asm-syntax.h"
#include "asm-common-aarch64.h"

/*******************
 * mpi_limb_t
 * _gcry_mpih_mul_1( mpi_ptr_t res_ptr,		x0
 *		  mpi_ptr_t s1_ptr,		x1
 *		  mpi_size_t s1_size,		w2
 *		  mpi_limb_t s2_limb)		x3
 */

.text

.globl C_SYMBOL_NAME(_gcry_mpih_mul_1)
ELF(.type  C_SYMBOL_NAME(_gcry_mpih_mul_1),%function)
C_SYMBOL_NAME(_gcry_mpih_mul_1):
	CFI_STARTPROC()
	and	w5, w2, #3;
	mov	x4, xzr;

	cbz	w5, .Large_loop;

.Loop:
	ldr	x5, [x1], #8;
	sub	w2, w2, #1;
	mul	x9, x5, x3;
	umulh	x10, x5, x3;
	and	w5, w2, #3;
	adds	x4, x4, x9;
	str	x4, [x0], #8;
	adc	x4, x10, xzr;

	cbz	w2, .Lend;
	cbnz	w5, .Loop;

.Large_loop:
	ldp	x5, x6, [x1];
	sub	w2, w2, #4;

	mul	x9, x5, x3;
	ldp	x7, x8, [x1, #16];
	umulh	x10, x5, x3;
	add	x1, x1, #32;

	adds	x4, x4, x9;
	str	x4, [x0], #8;
	mul	x11, x6, x3;
	adc	x4, x10, xzr;

	umulh	x12, x6, x3;

	adds	x4, x4, x11;
	str	x4, [x0], #8;
	mul	x13, x7, x3;
	adc	x4, x12, xzr;

	umulh	x14, x7, x3;

	adds	x4, x4, x13;
	str	x4, [x0], #8;
	mul	x15, x8, x3;
	adc	x4, x14, xzr;

	umulh	x16, x8, x3;

	adds	x4, x4, x15;
	str	x4, [x0], #8;
	adc	x4, x16, xzr;

	cbnz	w2, .Large_loop;

.Lend:
	mov	x0, x4;
	ret_spec_stop;
	CFI_ENDPROC()
ELF(.size C_SYMBOL_NAME(_gcry_mpih_mul_1),.-C_SYMBOL_NAME(_gcry_mpih_mul_1);)
