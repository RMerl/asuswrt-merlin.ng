/* poly1305-s390x.S  -  zSeries implementation of Poly1305
 *
 * Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 */

#include <config.h>
#if defined (__s390x__) && __GNUC__ >= 4 && __ARCH__ >= 9
#if defined(HAVE_GCC_INLINE_ASM_S390X)

#include "asm-poly1305-s390x.h"

.text

.balign 8
.globl _gcry_poly1305_s390x_blocks1
ELF(.type _gcry_poly1305_s390x_blocks1,@function;)

_gcry_poly1305_s390x_blocks1:
	/* input:
	 *	%r2: poly1305-state
	 *	%r3: src
	 *	%r4: len
	 *	%r5: high_pad
	 */
	CFI_STARTPROC();

	stmg %r6, %r14, 6 * 8(%r15);

	lgr POLY_RSTATE, %r2;
	lgr POLY_RSRC, %r3;
	srlg %r0, %r4, 4;

	cgije %r5, 0, .Lpoly_high0;

	POLY1305_LOAD_STATE();

.balign 4
.Lpoly_loop_high1:
	POLY1305_BLOCK_PART1(0 * 16);
	INC_POLY1305_SRC(1 * 16);
.Lpoly_block_part2:
	POLY1305_BLOCK_PART2();
	POLY1305_BLOCK_PART3();
	POLY1305_BLOCK_PART4();
	POLY1305_BLOCK_PART5();
	POLY1305_BLOCK_PART6();
	POLY1305_BLOCK_PART7();
	POLY1305_BLOCK_PART8();

	brctg %r0, .Lpoly_loop_high1;

.balign 4
.Lpoly_done:
	POLY1305_STORE_STATE();

	lmg %r6, %r14, 6 * 8(%r15);
	xgr %r2, %r2;
	br %r14;

.balign 4
.Lpoly_high0:
	lghi %r0, 1;
	POLY1305_LOAD_STATE();
	POLY1305_BLOCK_PART1_HB(0 * 16, 0);
	j .Lpoly_block_part2;

	CFI_ENDPROC();
ELF(.size _gcry_poly1305_s390x_blocks1,
    .-_gcry_poly1305_s390x_blocks1;)

#endif /*HAVE_GCC_INLINE_ASM_S390X*/
#endif /*__s390x__*/
