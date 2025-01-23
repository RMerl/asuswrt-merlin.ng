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
 */

#include "sysdep.h"
#include "asm-syntax.h"
#include "asm-common-aarch64.h"

/*******************
 * mpi_limb_t
 * _gcry_mpih_submul_1( mpi_ptr_t res_ptr,	x0
 *		     mpi_ptr_t s1_ptr,		x1
 *		     mpi_size_t s1_size,	w2
 *		     mpi_limb_t s2_limb)	x3
 */

.text

.globl C_SYMBOL_NAME(_gcry_mpih_submul_1)
ELF(.type  C_SYMBOL_NAME(_gcry_mpih_submul_1),%function)
C_SYMBOL_NAME(_gcry_mpih_submul_1):
	CFI_STARTPROC()
	and	w5, w2, #3;
	mov	x7, xzr;
	cbz	w5, .Large_loop;

	subs	xzr, xzr, xzr;

.Loop:
	ldr	x4, [x1], #8;
	cinc	x7, x7, cc;
	ldr	x5, [x0];
	sub	w2, w2, #1;

	mul	x6, x4, x3;
	subs	x5, x5, x7;
	umulh	x4, x4, x3;
	and	w10, w2, #3;

	cset	x7, cc;
	subs	x5, x5, x6;
	add	x7, x7, x4;
	str	x5, [x0], #8;

	cbz	w2, .Loop_end;
	cbnz	w10, .Loop;

	cinc	x7, x7, cc;

.Large_loop:
	ldp	x4, x8, [x1], #16;
	sub	w2, w2, #4;
	ldp	x5, x9, [x0];

	mul	x6, x4, x3;
	subs	x5, x5, x7;
	umulh	x4, x4, x3;

	cset	x7, cc;
	subs	x5, x5, x6;
	mul	x6, x8, x3;
	add	x7, x7, x4;
	str	x5, [x0], #8;
	cinc	x7, x7, cc;

	umulh	x8, x8, x3;

	subs	x9, x9, x7;
	cset	x7, cc;
	subs	x9, x9, x6;
	ldp	x4, x10, [x1], #16;
	str	x9, [x0], #8;
	add	x7, x7, x8;
	ldp	x5, x9, [x0];
	cinc	x7, x7, cc;

	mul	x6, x4, x3;
	subs	x5, x5, x7;
	umulh	x4, x4, x3;

	cset	x7, cc;
	subs	x5, x5, x6;
	mul	x6, x10, x3;
	add	x7, x7, x4;
	str	x5, [x0], #8;
	cinc	x7, x7, cc;

	umulh	x10, x10, x3;

	subs	x9, x9, x7;
	cset	x7, cc;
	subs	x9, x9, x6;
	add	x7, x7, x10;
	str	x9, [x0], #8;
	cinc	x7, x7, cc;

	cbnz	w2, .Large_loop;

	mov	x0, x7;
	ret_spec_stop;

.Loop_end:
	cinc	x0, x7, cc;
	ret_spec_stop;
	CFI_ENDPROC()
ELF(.size C_SYMBOL_NAME(_gcry_mpih_submul_1),.-C_SYMBOL_NAME(_gcry_mpih_submul_1);)
