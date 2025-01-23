/* serpent-armv7-neon.S  -  ARM/NEON assembly implementation of Serpent cipher
 *
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) && \
    defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_NEON)

.text

.syntax unified
.fpu neon
.arm

/* ARM registers */
#define RROUND r0

/* NEON vector registers */
#define RA0 q0
#define RA1 q1
#define RA2 q2
#define RA3 q3
#define RA4 q4
#define RB0 q5
#define RB1 q6
#define RB2 q7
#define RB3 q8
#define RB4 q9

#define RT0 q10
#define RT1 q11
#define RT2 q12
#define RT3 q13

#define RA0d0 d0
#define RA0d1 d1
#define RA1d0 d2
#define RA1d1 d3
#define RA2d0 d4
#define RA2d1 d5
#define RA3d0 d6
#define RA3d1 d7
#define RA4d0 d8
#define RA4d1 d9
#define RB0d0 d10
#define RB0d1 d11
#define RB1d0 d12
#define RB1d1 d13
#define RB2d0 d14
#define RB2d1 d15
#define RB3d0 d16
#define RB3d1 d17
#define RB4d0 d18
#define RB4d1 d19
#define RT0d0 d20
#define RT0d1 d21
#define RT1d0 d22
#define RT1d1 d23
#define RT2d0 d24
#define RT2d1 d25

/**********************************************************************
  helper macros
 **********************************************************************/

#define transpose_4x4(_q0, _q1, _q2, _q3) \
	vtrn.32 _q0, _q1;	\
	vtrn.32 _q2, _q3;	\
	vswp _q0##d1, _q2##d0;	\
	vswp _q1##d1, _q3##d0;

/**********************************************************************
  8-way serpent
 **********************************************************************/

/*
 * These are the S-Boxes of Serpent from following research paper.
 *
 *  D. A. Osvik, “Speeding up Serpent,” in Third AES Candidate Conference,
 *   (New York, New York, USA), p. 317–329, National Institute of Standards and
 *   Technology, 2000.
 *
 * Paper is also available at: http://www.ii.uib.no/~osvik/pub/aes3.pdf
 *
 */
#define SBOX0(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	veor	a3, a3, a0;	veor	b3, b3, b0;	vmov	a4, a1;		vmov	b4, b1;		\
	vand	a1, a1, a3;	vand	b1, b1, b3;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	veor	a1, a1, a0;	veor	b1, b1, b0;	vorr	a0, a0, a3;	vorr	b0, b0, b3;	\
	veor	a0, a0, a4;	veor	b0, b0, b4;	veor	a4, a4, a3;	veor	b4, b4, b3;	\
	veor	a3, a3, a2;	veor	b3, b3, b2;	vorr	a2, a2, a1;	vorr	b2, b2, b1;	\
	veor	a2, a2, a4;	veor	b2, b2, b4;	vmvn	a4, a4;		vmvn	b4, b4;		\
	vorr	a4, a4, a1;	vorr	b4, b4, b1;	veor	a1, a1, a3;	veor	b1, b1, b3;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;	vorr	a3, a3, a0;	vorr	b3, b3, b0;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	veor	a4, a3;		veor	b4, b3;

#define SBOX0_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmvn	a2, a2;		vmvn	b2, b2;		vmov	a4, a1;		vmov	b4, b1;		\
	vorr	a1, a1, a0;	vorr	b1, b1, b0;	vmvn	a4, a4;		vmvn	b4, b4;		\
	veor	a1, a1, a2;	veor	b1, b1, b2;	vorr	a2, a2, a4;	vorr	b2, b2, b4;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	veor	a0, a0, a4;	veor	b0, b0, b4;	\
	veor	a2, a2, a0;	veor	b2, b2, b0;	vand	a0, a0, a3;	vand	b0, b0, b3;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	vorr	a0, a0, a1;	vorr	b0, b0, b1;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	veor	a3, a3, a4;	veor	b3, b3, b4;	\
	veor	a2, a2, a1;	veor	b2, b2, b1;	veor	a3, a3, a0;	veor	b3, b3, b0;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;\
	vand	a2, a2, a3;	vand	b2, b2, b3;\
	veor	a4, a2;	veor	b4, b2;

#define SBOX1(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmvn	a0, a0;		vmvn	b0, b0;		vmvn	a2, a2;		vmvn	b2, b2;		\
	vmov	a4, a0;		vmov	b4, b0;		vand	a0, a0, a1;	vand	b0, b0, b1;	\
	veor	a2, a2, a0;	veor	b2, b2, b0;	vorr	a0, a0, a3;	vorr	b0, b0, b3;	\
	veor	a3, a3, a2;	veor	b3, b3, b2;	veor	a1, a1, a0;	veor	b1, b1, b0;	\
	veor	a0, a0, a4;	veor	b0, b0, b4;	vorr	a4, a4, a1;	vorr	b4, b4, b1;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	vorr	a2, a2, a0;	vorr	b2, b2, b0;	\
	vand	a2, a2, a4;	vand	b2, b2, b4;	veor	a0, a0, a1;	veor	b0, b0, b1;	\
	vand	a1, a1, a2;	vand	b1, b1, b2;\
	veor	a1, a1, a0;	veor	b1, b1, b0;	vand	a0, a0, a2;	vand	b0, b0, b2;	\
	veor	a0, a4;		veor	b0, b4;

#define SBOX1_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a1;		vmov	b4, b1;		veor	a1, a1, a3;	veor	b1, b1, b3;	\
	vand	a3, a3, a1;	vand	b3, b3, b1;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	veor	a3, a3, a0;	veor	b3, b3, b0;	vorr	a0, a0, a1;	vorr	b0, b0, b1;	\
	veor	a2, a2, a3;	veor	b2, b2, b3;	veor	a0, a0, a4;	veor	b0, b0, b4;	\
	vorr	a0, a0, a2;	vorr	b0, b0, b2;	veor	a1, a1, a3;	veor	b1, b1, b3;	\
	veor	a0, a0, a1;	veor	b0, b0, b1;	vorr	a1, a1, a3;	vorr	b1, b1, b3;	\
	veor	a1, a1, a0;	veor	b1, b1, b0;	vmvn	a4, a4;		vmvn	b4, b4;		\
	veor	a4, a4, a1;	veor	b4, b4, b1;	vorr	a1, a1, a0;	vorr	b1, b1, b0;	\
	veor	a1, a1, a0;	veor	b1, b1, b0;\
	vorr	a1, a1, a4;	vorr	b1, b1, b4;\
	veor	a3, a1;		veor	b3, b1;

#define SBOX2(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a0;		vmov	b4, b0;		vand	a0, a0, a2;	vand	b0, b0, b2;	\
	veor	a0, a0, a3;	veor	b0, b0, b3;	veor	a2, a2, a1;	veor	b2, b2, b1;	\
	veor	a2, a2, a0;	veor	b2, b2, b0;	vorr	a3, a3, a4;	vorr	b3, b3, b4;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	vmov	a1, a3;		vmov	b1, b3;		vorr	a3, a3, a4;	vorr	b3, b3, b4;	\
	veor	a3, a3, a0;	veor	b3, b3, b0;	vand	a0, a0, a1;	vand	b0, b0, b1;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	veor	a1, a1, a3;	veor	b1, b1, b3;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;	vmvn	a4, a4;		vmvn	b4, b4;

#define SBOX2_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	veor	a2, a2, a3;	veor	b2, b2, b3;	veor	a3, a3, a0;	veor	b3, b3, b0;	\
	vmov	a4, a3;		vmov	b4, b3;		vand	a3, a3, a2;	vand	b3, b3, b2;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;	vorr	a1, a1, a2;	vorr	b1, b1, b2;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;	vand	a4, a4, a3;	vand	b4, b4, b3;	\
	veor	a2, a2, a3;	veor	b2, b2, b3;	vand	a4, a4, a0;	vand	b4, b4, b0;	\
	veor	a4, a4, a2;	veor	b4, b4, b2;	vand	a2, a2, a1;	vand	b2, b2, b1;	\
	vorr	a2, a2, a0;	vorr	b2, b2, b0;	vmvn	a3, a3;		vmvn	b3, b3;		\
	veor	a2, a2, a3;	veor	b2, b2, b3;	veor	a0, a0, a3;	veor	b0, b0, b3;	\
	vand	a0, a0, a1;	vand	b0, b0, b1;	veor	a3, a3, a4;	veor	b3, b3, b4;	\
	veor	a3, a0;		veor	b3, b0;

#define SBOX3(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a0;		vmov	b4, b0;		vorr	a0, a0, a3;	vorr	b0, b0, b3;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;	vand	a1, a1, a4;	vand	b1, b1, b4;	\
	veor	a4, a4, a2;	veor	b4, b4, b2;	veor	a2, a2, a3;	veor	b2, b2, b3;	\
	vand	a3, a3, a0;	vand	b3, b3, b0;	vorr	a4, a4, a1;	vorr	b4, b4, b1;	\
	veor	a3, a3, a4;	veor	b3, b3, b4;	veor	a0, a0, a1;	veor	b0, b0, b1;	\
	vand	a4, a4, a0;	vand	b4, b4, b0;	veor	a1, a1, a3;	veor	b1, b1, b3;	\
	veor	a4, a4, a2;	veor	b4, b4, b2;	vorr	a1, a1, a0;	vorr	b1, b1, b0;	\
	veor	a1, a1, a2;	veor	b1, b1, b2;	veor	a0, a0, a3;	veor	b0, b0, b3;	\
	vmov	a2, a1;		vmov	b2, b1;		vorr	a1, a1, a3;	vorr	b1, b1, b3;	\
	veor	a1, a0;		veor	b1, b0;

#define SBOX3_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a2;		vmov	b4, b2;		veor	a2, a2, a1;	veor	b2, b2, b1;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	vand	a4, a4, a2;	vand	b4, b4, b2;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	vand	a0, a0, a1;	vand	b0, b0, b1;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	vorr	a3, a3, a4;	vorr	b3, b3, b4;	\
	veor	a2, a2, a3;	veor	b2, b2, b3;	veor	a0, a0, a3;	veor	b0, b0, b3;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;	vand	a3, a3, a2;	vand	b3, b3, b2;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;	veor	a1, a1, a0;	veor	b1, b1, b0;	\
	vorr	a1, a1, a2;	vorr	b1, b1, b2;	veor	a0, a0, a3;	veor	b0, b0, b3;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;\
	veor	a0, a1;		veor	b0, b1;

#define SBOX4(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	veor	a1, a1, a3;	veor	b1, b1, b3;	vmvn	a3, a3;		vmvn	b3, b3;		\
	veor	a2, a2, a3;	veor	b2, b2, b3;	veor	a3, a3, a0;	veor	b3, b3, b0;	\
	vmov	a4, a1;		vmov	b4, b1;		vand	a1, a1, a3;	vand	b1, b1, b3;	\
	veor	a1, a1, a2;	veor	b1, b1, b2;	veor	a4, a4, a3;	veor	b4, b4, b3;	\
	veor	a0, a0, a4;	veor	b0, b0, b4;	vand	a2, a2, a4;	vand	b2, b2, b4;	\
	veor	a2, a2, a0;	veor	b2, b2, b0;	vand	a0, a0, a1;	vand	b0, b0, b1;	\
	veor	a3, a3, a0;	veor	b3, b3, b0;	vorr	a4, a4, a1;	vorr	b4, b4, b1;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	vorr	a0, a0, a3;	vorr	b0, b0, b3;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	vand	a2, a2, a3;	vand	b2, b2, b3;	\
	vmvn	a0, a0;		vmvn	b0, b0;		veor	a4, a2;		veor	b4, b2;

#define SBOX4_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a2;		vmov	b4, b2;		vand	a2, a2, a3;	vand	b2, b2, b3;	\
	veor	a2, a2, a1;	veor	b2, b2, b1;	vorr	a1, a1, a3;	vorr	b1, b1, b3;	\
	vand	a1, a1, a0;	vand	b1, b1, b0;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	veor	a4, a4, a1;	veor	b4, b4, b1;	vand	a1, a1, a2;	vand	b1, b1, b2;	\
	vmvn	a0, a0;		vmvn	b0, b0;		veor	a3, a3, a4;	veor	b3, b3, b4;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	vand	a3, a3, a0;	vand	b3, b3, b0;	\
	veor	a3, a3, a2;	veor	b3, b3, b2;	veor	a0, a0, a1;	veor	b0, b0, b1;	\
	vand	a2, a2, a0;	vand	b2, b2, b0;	veor	a3, a3, a0;	veor	b3, b3, b0;	\
	veor	a2, a2, a4;	veor	b2, b2, b4;\
	vorr	a2, a2, a3;	vorr	b2, b2, b3;	veor	a3, a3, a0;	veor	b3, b3, b0;	\
	veor	a2, a1;		veor	b2, b1;

#define SBOX5(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	veor	a0, a0, a1;	veor	b0, b0, b1;	veor	a1, a1, a3;	veor	b1, b1, b3;	\
	vmvn	a3, a3;		vmvn	b3, b3;		vmov	a4, a1;		vmov	b4, b1;		\
	vand	a1, a1, a0;	vand	b1, b1, b0;	veor	a2, a2, a3;	veor	b2, b2, b3;	\
	veor	a1, a1, a2;	veor	b1, b1, b2;	vorr	a2, a2, a4;	vorr	b2, b2, b4;	\
	veor	a4, a4, a3;	veor	b4, b4, b3;	vand	a3, a3, a1;	vand	b3, b3, b1;	\
	veor	a3, a3, a0;	veor	b3, b3, b0;	veor	a4, a4, a1;	veor	b4, b4, b1;	\
	veor	a4, a4, a2;	veor	b4, b4, b2;	veor	a2, a2, a0;	veor	b2, b2, b0;	\
	vand	a0, a0, a3;	vand	b0, b0, b3;	vmvn	a2, a2;		vmvn	b2, b2;		\
	veor	a0, a0, a4;	veor	b0, b0, b4;	vorr	a4, a4, a3;	vorr	b4, b4, b3;	\
	veor	a2, a4;		veor	b2, b4;

#define SBOX5_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmvn	a1, a1;		vmvn	b1, b1;		vmov	a4, a3;		vmov	b4, b3;		\
	veor	a2, a2, a1;	veor	b2, b2, b1;	vorr	a3, a3, a0;	vorr	b3, b3, b0;	\
	veor	a3, a3, a2;	veor	b3, b3, b2;	vorr	a2, a2, a1;	vorr	b2, b2, b1;	\
	vand	a2, a2, a0;	vand	b2, b2, b0;	veor	a4, a4, a3;	veor	b4, b4, b3;	\
	veor	a2, a2, a4;	veor	b2, b2, b4;	vorr	a4, a4, a0;	vorr	b4, b4, b0;	\
	veor	a4, a4, a1;	veor	b4, b4, b1;	vand	a1, a1, a2;	vand	b1, b1, b2;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	vand	a3, a3, a4;	vand	b3, b3, b4;	veor	a4, a4, a1;	veor	b4, b4, b1;	\
	veor	a3, a3, a4;	veor	b3, b3, b4;	vmvn	a4, a4;		vmvn	b4, b4;		\
	veor	a3, a0;		veor	b3, b0;

#define SBOX6(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmvn	a2, a2;		vmvn	b2, b2;		vmov	a4, a3;		vmov	b4, b3;		\
	vand	a3, a3, a0;	vand	b3, b3, b0;	veor	a0, a0, a4;	veor	b0, b0, b4;	\
	veor	a3, a3, a2;	veor	b3, b3, b2;	vorr	a2, a2, a4;	vorr	b2, b2, b4;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	veor	a2, a2, a0;	veor	b2, b2, b0;	\
	vorr	a0, a0, a1;	vorr	b0, b0, b1;	veor	a2, a2, a1;	veor	b2, b2, b1;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	vorr	a0, a0, a3;	vorr	b0, b0, b3;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	veor	a4, a4, a3;	veor	b4, b4, b3;	\
	veor	a4, a4, a0;	veor	b4, b4, b0;	vmvn	a3, a3;		vmvn	b3, b3;		\
	vand	a2, a2, a4;	vand	b2, b2, b4;\
	veor	a2, a3;		veor	b2, b3;

#define SBOX6_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	veor	a0, a0, a2;	veor	b0, b0, b2;	vmov	a4, a2;		vmov	b4, b2;		\
	vand	a2, a2, a0;	vand	b2, b2, b0;	veor	a4, a4, a3;	veor	b4, b4, b3;	\
	vmvn	a2, a2;		vmvn	b2, b2;		veor	a3, a3, a1;	veor	b3, b3, b1;	\
	veor	a2, a2, a3;	veor	b2, b2, b3;	vorr	a4, a4, a0;	vorr	b4, b4, b0;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	veor	a3, a3, a4;	veor	b3, b3, b4;	\
	veor	a4, a4, a1;	veor	b4, b4, b1;	vand	a1, a1, a3;	vand	b1, b1, b3;	\
	veor	a1, a1, a0;	veor	b1, b1, b0;	veor	a0, a0, a3;	veor	b0, b0, b3;	\
	vorr	a0, a0, a2;	vorr	b0, b0, b2;	veor	a3, a3, a1;	veor	b3, b3, b1;	\
	veor	a4, a0;		veor	b4, b0;

#define SBOX7(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a1;		vmov	b4, b1;		vorr	a1, a1, a2;	vorr	b1, b1, b2;	\
	veor	a1, a1, a3;	veor	b1, b1, b3;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	veor	a2, a2, a1;	veor	b2, b2, b1;	vorr	a3, a3, a4;	vorr	b3, b3, b4;	\
	vand	a3, a3, a0;	vand	b3, b3, b0;	veor	a4, a4, a2;	veor	b4, b4, b2;	\
	veor	a3, a3, a1;	veor	b3, b3, b1;	vorr	a1, a1, a4;	vorr	b1, b1, b4;	\
	veor	a1, a1, a0;	veor	b1, b1, b0;	vorr	a0, a0, a4;	vorr	b0, b0, b4;	\
	veor	a0, a0, a2;	veor	b0, b0, b2;	veor	a1, a1, a4;	veor	b1, b1, b4;	\
	veor	a2, a2, a1;	veor	b2, b2, b1;	vand	a1, a1, a0;	vand	b1, b1, b0;	\
	veor	a1, a1, a4;	veor	b1, b1, b4;	vmvn	a2, a2;		vmvn	b2, b2;		\
	vorr	a2, a2, a0;	vorr	b2, b2, b0;\
	veor	a4, a2;		veor	b4, b2;

#define SBOX7_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vmov	a4, a2;		vmov	b4, b2;		veor	a2, a2, a0;	veor	b2, b2, b0;	\
	vand	a0, a0, a3;	vand	b0, b0, b3;	vorr	a4, a4, a3;	vorr	b4, b4, b3;	\
	vmvn	a2, a2;		vmvn	b2, b2;		veor	a3, a3, a1;	veor	b3, b3, b1;	\
	vorr	a1, a1, a0;	vorr	b1, b1, b0;	veor	a0, a0, a2;	veor	b0, b0, b2;	\
	vand	a2, a2, a4;	vand	b2, b2, b4;	vand	a3, a3, a4;	vand	b3, b3, b4;	\
	veor	a1, a1, a2;	veor	b1, b1, b2;	veor	a2, a2, a0;	veor	b2, b2, b0;	\
	vorr	a0, a0, a2;	vorr	b0, b0, b2;	veor	a4, a4, a1;	veor	b4, b4, b1;	\
	veor	a0, a0, a3;	veor	b0, b0, b3;	veor	a3, a3, a4;	veor	b3, b3, b4;	\
	vorr	a4, a4, a0;	vorr	b4, b4, b0;	veor	a3, a3, a2;	veor	b3, b3, b2;	\
	veor	a4, a2;		veor	b4, b2;

/* Apply SBOX number WHICH to to the block.  */
#define SBOX(which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	SBOX##which (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4)

/* Apply inverse SBOX number WHICH to to the block.  */
#define SBOX_INVERSE(which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	SBOX##which##_INVERSE (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4)

/* XOR round key into block state in a0,a1,a2,a3. a4 used as temporary.  */
#define BLOCK_XOR_KEY(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vdup.32 RT3, RT0d0[0]; \
	vdup.32 RT1, RT0d0[1]; \
	vdup.32 RT2, RT0d1[0]; \
	vdup.32 RT0, RT0d1[1]; \
	veor a0, a0, RT3;	veor b0, b0, RT3; \
	veor a1, a1, RT1;	veor b1, b1, RT1; \
	veor a2, a2, RT2;	veor b2, b2, RT2; \
	veor a3, a3, RT0;	veor b3, b3, RT0;

#define BLOCK_LOAD_KEY_ENC() \
	vld1.8 {RT0d0, RT0d1}, [RROUND]!;

#define BLOCK_LOAD_KEY_DEC() \
	vld1.8 {RT0d0, RT0d1}, [RROUND]; \
	sub RROUND, RROUND, #16

/* Apply the linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vshl.u32	a4, a0, #13;		vshl.u32	b4, b0, #13;		\
	vshr.u32	a0, a0, #(32-13);	vshr.u32	b0, b0, #(32-13);	\
	veor		a0, a0, a4;		veor		b0, b0, b4;		\
	vshl.u32	a4, a2, #3;		vshl.u32	b4, b2, #3;		\
	vshr.u32	a2, a2, #(32-3);	vshr.u32	b2, b2, #(32-3);	\
	veor		a2, a2, a4;		veor		b2, b2, b4;		\
	veor		a1, a0, a1;		veor		b1, b0, b1;		\
	veor		a1, a2, a1;		veor		b1, b2, b1;		\
	vshl.u32	a4, a0, #3;		vshl.u32	b4, b0, #3;		\
	veor		a3, a2, a3;		veor		b3, b2, b3;		\
	veor		a3, a4, a3;		veor		b3, b4, b3;		\
	vshl.u32	a4, a1, #1;		vshl.u32	b4, b1, #1;		\
	vshr.u32	a1, a1, #(32-1);	vshr.u32	b1, b1, #(32-1);	\
	veor		a1, a1, a4;		veor		b1, b1, b4;		\
	vshl.u32	a4, a3, #7;		vshl.u32	b4, b3, #7;		\
	vshr.u32	a3, a3, #(32-7);	vshr.u32	b3, b3, #(32-7);	\
	veor		a3, a3, a4;		veor		b3, b3, b4;		\
	veor		a0, a1, a0;		veor		b0, b1, b0;		\
	veor		a0, a3, a0;		veor		b0, b3, b0;		\
	vshl.u32	a4, a1, #7;		vshl.u32	b4, b1, #7;		\
	veor		a2, a3, a2;		veor		b2, b3, b2;		\
	veor		a2, a4, a2;		veor		b2, b4, b2;		\
	vshl.u32	a4, a0, #5;		vshl.u32	b4, b0, #5;		\
	vshr.u32	a0, a0, #(32-5);	vshr.u32	b0, b0, #(32-5);	\
	veor		a0, a0, a4;		veor		b0, b0, b4;		\
	vshl.u32	a4, a2, #22;		vshl.u32	b4, b2, #22;		\
	vshr.u32	a2, a2, #(32-22);	vshr.u32	b2, b2, #(32-22);	\
	veor		a2, a2, a4;		veor		b2, b2, b4;

/* Apply the inverse linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION_INVERSE(a0, a1, a2, a3, a4, b0, b1, b2, b3, b4) \
	vshr.u32	a4, a2, #22;		vshr.u32	b4, b2, #22;		\
	vshl.u32	a2, a2, #(32-22);	vshl.u32	b2, b2, #(32-22);	\
	veor		a2, a2, a4;		veor		b2, b2, b4;		\
	vshr.u32	a4, a0, #5;		vshr.u32	b4, b0, #5;		\
	vshl.u32	a0, a0, #(32-5);	vshl.u32	b0, b0, #(32-5);	\
	veor		a0, a0, a4;		veor		b0, b0, b4;		\
	vshl.u32	a4, a1, #7;		vshl.u32	b4, b1, #7;		\
	veor		a2, a3, a2;		veor		b2, b3, b2;		\
	veor		a2, a4, a2;		veor		b2, b4, b2;		\
	veor		a0, a1, a0;		veor		b0, b1, b0;		\
	veor		a0, a3, a0;		veor		b0, b3, b0;		\
	vshr.u32	a4, a3, #7;		vshr.u32	b4, b3, #7;		\
	vshl.u32	a3, a3, #(32-7);	vshl.u32	b3, b3, #(32-7);	\
	veor		a3, a3, a4;		veor		b3, b3, b4;		\
	vshr.u32	a4, a1, #1;		vshr.u32	b4, b1, #1;		\
	vshl.u32	a1, a1, #(32-1);	vshl.u32	b1, b1, #(32-1);	\
	veor		a1, a1, a4;		veor		b1, b1, b4;		\
	vshl.u32	a4, a0, #3;		vshl.u32	b4, b0, #3;		\
	veor		a3, a2, a3;		veor		b3, b2, b3;		\
	veor		a3, a4, a3;		veor		b3, b4, b3;		\
	veor		a1, a0, a1;		veor		b1, b0, b1;		\
	veor		a1, a2, a1;		veor		b1, b2, b1;		\
	vshr.u32	a4, a2, #3;		vshr.u32	b4, b2, #3;		\
	vshl.u32	a2, a2, #(32-3);	vshl.u32	b2, b2, #(32-3);	\
	veor		a2, a2, a4;		veor		b2, b2, b4;		\
	vshr.u32	a4, a0, #13;		vshr.u32	b4, b0, #13;		\
	vshl.u32	a0, a0, #(32-13);	vshl.u32	b0, b0, #(32-13);	\
	veor		a0, a0, a4;		veor		b0, b0, b4;

/* Apply a Serpent round to eight parallel blocks.  This macro increments
   `round'.  */
#define ROUND(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
			    b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);		\
	BLOCK_LOAD_KEY_ENC ();						\
	SBOX (which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);		\
	LINEAR_TRANSFORMATION (na0, na1, na2, na3, na4, nb0, nb1, nb2, nb3, nb4);

/* Apply the last Serpent round to eight parallel blocks.  This macro increments
   `round'.  */
#define ROUND_LAST(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
				 b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);		\
	BLOCK_LOAD_KEY_ENC ();						\
	SBOX (which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);		\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, nb0, nb1, nb2, nb3, nb4);

/* Apply an inverse Serpent round to eight parallel blocks.  This macro
   increments `round'.  */
#define ROUND_INVERSE(round, which, a0, a1, a2, a3, a4, \
				    na0, na1, na2, na3, na4, \
				    b0, b1, b2, b3, b4, \
				    nb0, nb1, nb2, nb3, nb4) \
	LINEAR_TRANSFORMATION_INVERSE (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);	\
	SBOX_INVERSE (which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);		\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, nb0, nb1, nb2, nb3, nb4);	\
	BLOCK_LOAD_KEY_DEC ();

/* Apply the first inverse Serpent round to eight parallel blocks.  This macro
   increments `round'.  */
#define ROUND_FIRST_INVERSE(round, which, a0, a1, a2, a3, a4, \
					  na0, na1, na2, na3, na4, \
					  b0, b1, b2, b3, b4, \
					  nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, b0, b1, b2, b3, b4);			\
	BLOCK_LOAD_KEY_DEC ();							\
	SBOX_INVERSE (which, a0, a1, a2, a3, a4, b0, b1, b2, b3, b4); 		\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, nb0, nb1, nb2, nb3, nb4);	\
	BLOCK_LOAD_KEY_DEC ();

.align 3
.type __serpent_enc_blk8,%function;
__serpent_enc_blk8:
	/* input:
	 *	r0: round key pointer
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel plaintext
	 *						blocks
	 * output:
	 *	RA4, RA1, RA2, RA0, RB4, RB1, RB2, RB0: eight parallel
	 * 						ciphertext blocks
	 */

	transpose_4x4(RA0, RA1, RA2, RA3);
	BLOCK_LOAD_KEY_ENC ();
	transpose_4x4(RB0, RB1, RB2, RB3);

	ROUND (0, 0, RA0, RA1, RA2, RA3, RA4, RA1, RA4, RA2, RA0, RA3,
		     RB0, RB1, RB2, RB3, RB4, RB1, RB4, RB2, RB0, RB3);
	ROUND (1, 1, RA1, RA4, RA2, RA0, RA3, RA2, RA1, RA0, RA4, RA3,
		     RB1, RB4, RB2, RB0, RB3, RB2, RB1, RB0, RB4, RB3);
	ROUND (2, 2, RA2, RA1, RA0, RA4, RA3, RA0, RA4, RA1, RA3, RA2,
		     RB2, RB1, RB0, RB4, RB3, RB0, RB4, RB1, RB3, RB2);
	ROUND (3, 3, RA0, RA4, RA1, RA3, RA2, RA4, RA1, RA3, RA2, RA0,
		     RB0, RB4, RB1, RB3, RB2, RB4, RB1, RB3, RB2, RB0);
	ROUND (4, 4, RA4, RA1, RA3, RA2, RA0, RA1, RA0, RA4, RA2, RA3,
		     RB4, RB1, RB3, RB2, RB0, RB1, RB0, RB4, RB2, RB3);
	ROUND (5, 5, RA1, RA0, RA4, RA2, RA3, RA0, RA2, RA1, RA4, RA3,
		     RB1, RB0, RB4, RB2, RB3, RB0, RB2, RB1, RB4, RB3);
	ROUND (6, 6, RA0, RA2, RA1, RA4, RA3, RA0, RA2, RA3, RA1, RA4,
		     RB0, RB2, RB1, RB4, RB3, RB0, RB2, RB3, RB1, RB4);
	ROUND (7, 7, RA0, RA2, RA3, RA1, RA4, RA4, RA1, RA2, RA0, RA3,
		     RB0, RB2, RB3, RB1, RB4, RB4, RB1, RB2, RB0, RB3);
	ROUND (8, 0, RA4, RA1, RA2, RA0, RA3, RA1, RA3, RA2, RA4, RA0,
		     RB4, RB1, RB2, RB0, RB3, RB1, RB3, RB2, RB4, RB0);
	ROUND (9, 1, RA1, RA3, RA2, RA4, RA0, RA2, RA1, RA4, RA3, RA0,
		     RB1, RB3, RB2, RB4, RB0, RB2, RB1, RB4, RB3, RB0);
	ROUND (10, 2, RA2, RA1, RA4, RA3, RA0, RA4, RA3, RA1, RA0, RA2,
		      RB2, RB1, RB4, RB3, RB0, RB4, RB3, RB1, RB0, RB2);
	ROUND (11, 3, RA4, RA3, RA1, RA0, RA2, RA3, RA1, RA0, RA2, RA4,
		      RB4, RB3, RB1, RB0, RB2, RB3, RB1, RB0, RB2, RB4);
	ROUND (12, 4, RA3, RA1, RA0, RA2, RA4, RA1, RA4, RA3, RA2, RA0,
		      RB3, RB1, RB0, RB2, RB4, RB1, RB4, RB3, RB2, RB0);
	ROUND (13, 5, RA1, RA4, RA3, RA2, RA0, RA4, RA2, RA1, RA3, RA0,
		      RB1, RB4, RB3, RB2, RB0, RB4, RB2, RB1, RB3, RB0);
	ROUND (14, 6, RA4, RA2, RA1, RA3, RA0, RA4, RA2, RA0, RA1, RA3,
		      RB4, RB2, RB1, RB3, RB0, RB4, RB2, RB0, RB1, RB3);
	ROUND (15, 7, RA4, RA2, RA0, RA1, RA3, RA3, RA1, RA2, RA4, RA0,
		      RB4, RB2, RB0, RB1, RB3, RB3, RB1, RB2, RB4, RB0);
	ROUND (16, 0, RA3, RA1, RA2, RA4, RA0, RA1, RA0, RA2, RA3, RA4,
		      RB3, RB1, RB2, RB4, RB0, RB1, RB0, RB2, RB3, RB4);
	ROUND (17, 1, RA1, RA0, RA2, RA3, RA4, RA2, RA1, RA3, RA0, RA4,
		      RB1, RB0, RB2, RB3, RB4, RB2, RB1, RB3, RB0, RB4);
	ROUND (18, 2, RA2, RA1, RA3, RA0, RA4, RA3, RA0, RA1, RA4, RA2,
		      RB2, RB1, RB3, RB0, RB4, RB3, RB0, RB1, RB4, RB2);
	ROUND (19, 3, RA3, RA0, RA1, RA4, RA2, RA0, RA1, RA4, RA2, RA3,
		      RB3, RB0, RB1, RB4, RB2, RB0, RB1, RB4, RB2, RB3);
	ROUND (20, 4, RA0, RA1, RA4, RA2, RA3, RA1, RA3, RA0, RA2, RA4,
		      RB0, RB1, RB4, RB2, RB3, RB1, RB3, RB0, RB2, RB4);
	ROUND (21, 5, RA1, RA3, RA0, RA2, RA4, RA3, RA2, RA1, RA0, RA4,
		      RB1, RB3, RB0, RB2, RB4, RB3, RB2, RB1, RB0, RB4);
	ROUND (22, 6, RA3, RA2, RA1, RA0, RA4, RA3, RA2, RA4, RA1, RA0,
		      RB3, RB2, RB1, RB0, RB4, RB3, RB2, RB4, RB1, RB0);
	ROUND (23, 7, RA3, RA2, RA4, RA1, RA0, RA0, RA1, RA2, RA3, RA4,
		      RB3, RB2, RB4, RB1, RB0, RB0, RB1, RB2, RB3, RB4);
	ROUND (24, 0, RA0, RA1, RA2, RA3, RA4, RA1, RA4, RA2, RA0, RA3,
		      RB0, RB1, RB2, RB3, RB4, RB1, RB4, RB2, RB0, RB3);
	ROUND (25, 1, RA1, RA4, RA2, RA0, RA3, RA2, RA1, RA0, RA4, RA3,
		      RB1, RB4, RB2, RB0, RB3, RB2, RB1, RB0, RB4, RB3);
	ROUND (26, 2, RA2, RA1, RA0, RA4, RA3, RA0, RA4, RA1, RA3, RA2,
		      RB2, RB1, RB0, RB4, RB3, RB0, RB4, RB1, RB3, RB2);
	ROUND (27, 3, RA0, RA4, RA1, RA3, RA2, RA4, RA1, RA3, RA2, RA0,
		      RB0, RB4, RB1, RB3, RB2, RB4, RB1, RB3, RB2, RB0);
	ROUND (28, 4, RA4, RA1, RA3, RA2, RA0, RA1, RA0, RA4, RA2, RA3,
		      RB4, RB1, RB3, RB2, RB0, RB1, RB0, RB4, RB2, RB3);
	ROUND (29, 5, RA1, RA0, RA4, RA2, RA3, RA0, RA2, RA1, RA4, RA3,
		      RB1, RB0, RB4, RB2, RB3, RB0, RB2, RB1, RB4, RB3);
	ROUND (30, 6, RA0, RA2, RA1, RA4, RA3, RA0, RA2, RA3, RA1, RA4,
		      RB0, RB2, RB1, RB4, RB3, RB0, RB2, RB3, RB1, RB4);
	ROUND_LAST (31, 7, RA0, RA2, RA3, RA1, RA4, RA4, RA1, RA2, RA0, RA3,
		           RB0, RB2, RB3, RB1, RB4, RB4, RB1, RB2, RB0, RB3);

	transpose_4x4(RA4, RA1, RA2, RA0);
	transpose_4x4(RB4, RB1, RB2, RB0);

	bx lr;
.size __serpent_enc_blk8,.-__serpent_enc_blk8;

.align 3
.type   __serpent_dec_blk8,%function;
__serpent_dec_blk8:
	/* input:
	 *	r0: round key pointer
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel
	 * 						ciphertext blocks
	 * output:
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel plaintext
	 *						blocks
	 */

	add RROUND, RROUND, #(32*16);

	transpose_4x4(RA0, RA1, RA2, RA3);
	BLOCK_LOAD_KEY_DEC ();
	transpose_4x4(RB0, RB1, RB2, RB3);

	ROUND_FIRST_INVERSE (31, 7, RA0, RA1, RA2, RA3, RA4,
				    RA3, RA0, RA1, RA4, RA2,
				    RB0, RB1, RB2, RB3, RB4,
				    RB3, RB0, RB1, RB4, RB2);
	ROUND_INVERSE (30, 6, RA3, RA0, RA1, RA4, RA2, RA0, RA1, RA2, RA4, RA3,
		              RB3, RB0, RB1, RB4, RB2, RB0, RB1, RB2, RB4, RB3);
	ROUND_INVERSE (29, 5, RA0, RA1, RA2, RA4, RA3, RA1, RA3, RA4, RA2, RA0,
		              RB0, RB1, RB2, RB4, RB3, RB1, RB3, RB4, RB2, RB0);
	ROUND_INVERSE (28, 4, RA1, RA3, RA4, RA2, RA0, RA1, RA2, RA4, RA0, RA3,
		              RB1, RB3, RB4, RB2, RB0, RB1, RB2, RB4, RB0, RB3);
	ROUND_INVERSE (27, 3, RA1, RA2, RA4, RA0, RA3, RA4, RA2, RA0, RA1, RA3,
		              RB1, RB2, RB4, RB0, RB3, RB4, RB2, RB0, RB1, RB3);
	ROUND_INVERSE (26, 2, RA4, RA2, RA0, RA1, RA3, RA2, RA3, RA0, RA1, RA4,
		              RB4, RB2, RB0, RB1, RB3, RB2, RB3, RB0, RB1, RB4);
	ROUND_INVERSE (25, 1, RA2, RA3, RA0, RA1, RA4, RA4, RA2, RA1, RA0, RA3,
		              RB2, RB3, RB0, RB1, RB4, RB4, RB2, RB1, RB0, RB3);
	ROUND_INVERSE (24, 0, RA4, RA2, RA1, RA0, RA3, RA4, RA3, RA2, RA0, RA1,
		              RB4, RB2, RB1, RB0, RB3, RB4, RB3, RB2, RB0, RB1);
	ROUND_INVERSE (23, 7, RA4, RA3, RA2, RA0, RA1, RA0, RA4, RA3, RA1, RA2,
		              RB4, RB3, RB2, RB0, RB1, RB0, RB4, RB3, RB1, RB2);
	ROUND_INVERSE (22, 6, RA0, RA4, RA3, RA1, RA2, RA4, RA3, RA2, RA1, RA0,
		              RB0, RB4, RB3, RB1, RB2, RB4, RB3, RB2, RB1, RB0);
	ROUND_INVERSE (21, 5, RA4, RA3, RA2, RA1, RA0, RA3, RA0, RA1, RA2, RA4,
		              RB4, RB3, RB2, RB1, RB0, RB3, RB0, RB1, RB2, RB4);
	ROUND_INVERSE (20, 4, RA3, RA0, RA1, RA2, RA4, RA3, RA2, RA1, RA4, RA0,
		              RB3, RB0, RB1, RB2, RB4, RB3, RB2, RB1, RB4, RB0);
	ROUND_INVERSE (19, 3, RA3, RA2, RA1, RA4, RA0, RA1, RA2, RA4, RA3, RA0,
		              RB3, RB2, RB1, RB4, RB0, RB1, RB2, RB4, RB3, RB0);
	ROUND_INVERSE (18, 2, RA1, RA2, RA4, RA3, RA0, RA2, RA0, RA4, RA3, RA1,
		              RB1, RB2, RB4, RB3, RB0, RB2, RB0, RB4, RB3, RB1);
	ROUND_INVERSE (17, 1, RA2, RA0, RA4, RA3, RA1, RA1, RA2, RA3, RA4, RA0,
		              RB2, RB0, RB4, RB3, RB1, RB1, RB2, RB3, RB4, RB0);
	ROUND_INVERSE (16, 0, RA1, RA2, RA3, RA4, RA0, RA1, RA0, RA2, RA4, RA3,
		              RB1, RB2, RB3, RB4, RB0, RB1, RB0, RB2, RB4, RB3);
	ROUND_INVERSE (15, 7, RA1, RA0, RA2, RA4, RA3, RA4, RA1, RA0, RA3, RA2,
		              RB1, RB0, RB2, RB4, RB3, RB4, RB1, RB0, RB3, RB2);
	ROUND_INVERSE (14, 6, RA4, RA1, RA0, RA3, RA2, RA1, RA0, RA2, RA3, RA4,
		              RB4, RB1, RB0, RB3, RB2, RB1, RB0, RB2, RB3, RB4);
	ROUND_INVERSE (13, 5, RA1, RA0, RA2, RA3, RA4, RA0, RA4, RA3, RA2, RA1,
		              RB1, RB0, RB2, RB3, RB4, RB0, RB4, RB3, RB2, RB1);
	ROUND_INVERSE (12, 4, RA0, RA4, RA3, RA2, RA1, RA0, RA2, RA3, RA1, RA4,
		              RB0, RB4, RB3, RB2, RB1, RB0, RB2, RB3, RB1, RB4);
	ROUND_INVERSE (11, 3, RA0, RA2, RA3, RA1, RA4, RA3, RA2, RA1, RA0, RA4,
		              RB0, RB2, RB3, RB1, RB4, RB3, RB2, RB1, RB0, RB4);
	ROUND_INVERSE (10, 2, RA3, RA2, RA1, RA0, RA4, RA2, RA4, RA1, RA0, RA3,
		              RB3, RB2, RB1, RB0, RB4, RB2, RB4, RB1, RB0, RB3);
	ROUND_INVERSE (9, 1, RA2, RA4, RA1, RA0, RA3, RA3, RA2, RA0, RA1, RA4,
		             RB2, RB4, RB1, RB0, RB3, RB3, RB2, RB0, RB1, RB4);
	ROUND_INVERSE (8, 0, RA3, RA2, RA0, RA1, RA4, RA3, RA4, RA2, RA1, RA0,
		             RB3, RB2, RB0, RB1, RB4, RB3, RB4, RB2, RB1, RB0);
	ROUND_INVERSE (7, 7, RA3, RA4, RA2, RA1, RA0, RA1, RA3, RA4, RA0, RA2,
		             RB3, RB4, RB2, RB1, RB0, RB1, RB3, RB4, RB0, RB2);
	ROUND_INVERSE (6, 6, RA1, RA3, RA4, RA0, RA2, RA3, RA4, RA2, RA0, RA1,
		             RB1, RB3, RB4, RB0, RB2, RB3, RB4, RB2, RB0, RB1);
	ROUND_INVERSE (5, 5, RA3, RA4, RA2, RA0, RA1, RA4, RA1, RA0, RA2, RA3,
		             RB3, RB4, RB2, RB0, RB1, RB4, RB1, RB0, RB2, RB3);
	ROUND_INVERSE (4, 4, RA4, RA1, RA0, RA2, RA3, RA4, RA2, RA0, RA3, RA1,
		             RB4, RB1, RB0, RB2, RB3, RB4, RB2, RB0, RB3, RB1);
	ROUND_INVERSE (3, 3, RA4, RA2, RA0, RA3, RA1, RA0, RA2, RA3, RA4, RA1,
		             RB4, RB2, RB0, RB3, RB1, RB0, RB2, RB3, RB4, RB1);
	ROUND_INVERSE (2, 2, RA0, RA2, RA3, RA4, RA1, RA2, RA1, RA3, RA4, RA0,
		             RB0, RB2, RB3, RB4, RB1, RB2, RB1, RB3, RB4, RB0);
	ROUND_INVERSE (1, 1, RA2, RA1, RA3, RA4, RA0, RA0, RA2, RA4, RA3, RA1,
		             RB2, RB1, RB3, RB4, RB0, RB0, RB2, RB4, RB3, RB1);
	ROUND_INVERSE (0, 0, RA0, RA2, RA4, RA3, RA1, RA0, RA1, RA2, RA3, RA4,
		             RB0, RB2, RB4, RB3, RB1, RB0, RB1, RB2, RB3, RB4);

	transpose_4x4(RA0, RA1, RA2, RA3);
	transpose_4x4(RB0, RB1, RB2, RB3);

	bx lr;
.size __serpent_dec_blk8,.-__serpent_dec_blk8;

.align 3
.globl _gcry_serpent_neon_ctr_enc
.type _gcry_serpent_neon_ctr_enc,%function;
_gcry_serpent_neon_ctr_enc:
	/* input:
	 *	r0: ctx, CTX
	 *	r1: dst (8 blocks)
	 *	r2: src (8 blocks)
	 *	r3: iv
	 */

	vmov.u8 RT1d0, #0xff; /* u64: -1 */
	push {r4,lr};
	vadd.u64 RT2d0, RT1d0, RT1d0; /* u64: -2 */
	vpush {RA4-RB2};

	/* load IV and byteswap */
	vld1.8 {RA0}, [r3];
	vrev64.u8 RT0, RA0; /* be => le */
	ldr r4, [r3, #8];

	/* construct IVs */
	vsub.u64 RA2d1, RT0d1, RT2d0; /* +2 */
	vsub.u64 RA1d1, RT0d1, RT1d0; /* +1 */
	cmp r4, #-1;

	vsub.u64 RB0d1, RA2d1, RT2d0; /* +4 */
	vsub.u64 RA3d1, RA2d1, RT1d0; /* +3 */
	ldr r4, [r3, #12];

	vsub.u64 RB2d1, RB0d1, RT2d0; /* +6 */
	vsub.u64 RB1d1, RB0d1, RT1d0; /* +5 */

	vsub.u64 RT2d1, RB2d1, RT2d0; /* +8 */
	vsub.u64 RB3d1, RB2d1, RT1d0; /* +7 */

	vmov RA1d0, RT0d0;
	vmov RA2d0, RT0d0;
	vmov RA3d0, RT0d0;
	vmov RB0d0, RT0d0;
	rev r4, r4;
	vmov RB1d0, RT0d0;
	vmov RB2d0, RT0d0;
	vmov RB3d0, RT0d0;
	vmov RT2d0, RT0d0;

	/* check need for handling 64-bit overflow and carry */
	beq .Ldo_ctr_carry;

.Lctr_carry_done:
	/* le => be */
	vrev64.u8 RA1, RA1;
	vrev64.u8 RA2, RA2;
	vrev64.u8 RA3, RA3;
	vrev64.u8 RB0, RB0;
	vrev64.u8 RT2, RT2;
	vrev64.u8 RB1, RB1;
	vrev64.u8 RB2, RB2;
	vrev64.u8 RB3, RB3;
	/* store new IV */
	vst1.8 {RT2}, [r3];

	bl __serpent_enc_blk8;

	vld1.8 {RT0, RT1}, [r2]!;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RA4, RA4, RT0;
	veor RA1, RA1, RT1;
	vld1.8 {RT0, RT1}, [r2]!;
	veor RA2, RA2, RT2;
	veor RA0, RA0, RT3;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RB4, RB4, RT0;
	veor RT0, RT0;
	veor RB1, RB1, RT1;
	veor RT1, RT1;
	veor RB2, RB2, RT2;
	veor RT2, RT2;
	veor RB0, RB0, RT3;
	veor RT3, RT3;

	vst1.8 {RA4}, [r1]!;
	vst1.8 {RA1}, [r1]!;
	veor RA1, RA1;
	vst1.8 {RA2}, [r1]!;
	veor RA2, RA2;
	vst1.8 {RA0}, [r1]!;
	veor RA0, RA0;
	vst1.8 {RB4}, [r1]!;
	veor RB4, RB4;
	vst1.8 {RB1}, [r1]!;
	vst1.8 {RB2}, [r1]!;
	vst1.8 {RB0}, [r1]!;

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RA3, RA3;
	veor RB3, RB3;

	pop {r4,pc};

.Ldo_ctr_carry:
	cmp r4, #-8;
	blo .Lctr_carry_done;
	beq .Lcarry_RT2;

	cmp r4, #-6;
	blo .Lcarry_RB3;
	beq .Lcarry_RB2;

	cmp r4, #-4;
	blo .Lcarry_RB1;
	beq .Lcarry_RB0;

	cmp r4, #-2;
	blo .Lcarry_RA3;
	beq .Lcarry_RA2;

	vsub.u64 RA1d0, RT1d0;
.Lcarry_RA2:
	vsub.u64 RA2d0, RT1d0;
.Lcarry_RA3:
	vsub.u64 RA3d0, RT1d0;
.Lcarry_RB0:
	vsub.u64 RB0d0, RT1d0;
.Lcarry_RB1:
	vsub.u64 RB1d0, RT1d0;
.Lcarry_RB2:
	vsub.u64 RB2d0, RT1d0;
.Lcarry_RB3:
	vsub.u64 RB3d0, RT1d0;
.Lcarry_RT2:
	vsub.u64 RT2d0, RT1d0;

	b .Lctr_carry_done;
.size _gcry_serpent_neon_ctr_enc,.-_gcry_serpent_neon_ctr_enc;

.align 3
.globl _gcry_serpent_neon_cfb_dec
.type _gcry_serpent_neon_cfb_dec,%function;
_gcry_serpent_neon_cfb_dec:
	/* input:
	 *	r0: ctx, CTX
	 *	r1: dst (8 blocks)
	 *	r2: src (8 blocks)
	 *	r3: iv
	 */

	push {lr};
	vpush {RA4-RB2};

	/* Load input */
	vld1.8 {RA0}, [r3];
	vld1.8 {RA1, RA2}, [r2]!;
	vld1.8 {RA3}, [r2]!;
	vld1.8 {RB0}, [r2]!;
	vld1.8 {RB1, RB2}, [r2]!;
	vld1.8 {RB3}, [r2]!;

	/* Update IV */
	vld1.8 {RT0}, [r2]!;
	vst1.8 {RT0}, [r3];
	mov r3, lr;
	sub r2, r2, #(8*16);

	bl __serpent_enc_blk8;

	vld1.8 {RT0, RT1}, [r2]!;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RA4, RA4, RT0;
	veor RA1, RA1, RT1;
	vld1.8 {RT0, RT1}, [r2]!;
	veor RA2, RA2, RT2;
	veor RA0, RA0, RT3;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RB4, RB4, RT0;
	veor RT0, RT0;
	veor RB1, RB1, RT1;
	veor RT1, RT1;
	veor RB2, RB2, RT2;
	veor RT2, RT2;
	veor RB0, RB0, RT3;
	veor RT3, RT3;

	vst1.8 {RA4}, [r1]!;
	vst1.8 {RA1}, [r1]!;
	veor RA1, RA1;
	vst1.8 {RA2}, [r1]!;
	veor RA2, RA2;
	vst1.8 {RA0}, [r1]!;
	veor RA0, RA0;
	vst1.8 {RB4}, [r1]!;
	veor RB4, RB4;
	vst1.8 {RB1}, [r1]!;
	vst1.8 {RB2}, [r1]!;
	vst1.8 {RB0}, [r1]!;

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RA3, RA3;
	veor RB3, RB3;

	pop {pc};
.size _gcry_serpent_neon_cfb_dec,.-_gcry_serpent_neon_cfb_dec;

.align 3
.globl _gcry_serpent_neon_cbc_dec
.type _gcry_serpent_neon_cbc_dec,%function;
_gcry_serpent_neon_cbc_dec:
	/* input:
	 *	r0: ctx, CTX
	 *	r1: dst (8 blocks)
	 *	r2: src (8 blocks)
	 *	r3: iv
	 */

	push {lr};
	vpush {RA4-RB2};

	vld1.8 {RA0, RA1}, [r2]!;
	vld1.8 {RA2, RA3}, [r2]!;
	vld1.8 {RB0, RB1}, [r2]!;
	vld1.8 {RB2, RB3}, [r2]!;
	sub r2, r2, #(8*16);

	bl __serpent_dec_blk8;

	vld1.8 {RB4}, [r3];
	vld1.8 {RT0, RT1}, [r2]!;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RA0, RA0, RB4;
	veor RA1, RA1, RT0;
	veor RA2, RA2, RT1;
	vld1.8 {RT0, RT1}, [r2]!;
	veor RA3, RA3, RT2;
	veor RB0, RB0, RT3;
	vld1.8 {RT2, RT3}, [r2]!;
	veor RB1, RB1, RT0;
	veor RT0, RT0;
	veor RB2, RB2, RT1;
	veor RT1, RT1;
	veor RB3, RB3, RT2;
	veor RT2, RT2;
	vst1.8 {RT3}, [r3]; /* store new IV */
	veor RT3, RT3;

	vst1.8 {RA0, RA1}, [r1]!;
	veor RA0, RA0;
	veor RA1, RA1;
	vst1.8 {RA2, RA3}, [r1]!;
	veor RA2, RA2;
	vst1.8 {RB0, RB1}, [r1]!;
	veor RA3, RA3;
	vst1.8 {RB2, RB3}, [r1]!;
	veor RB3, RB3;

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RB4, RB4;

	pop {pc};
.size _gcry_serpent_neon_cbc_dec,.-_gcry_serpent_neon_cbc_dec;

.align 3
.globl _gcry_serpent_neon_ocb_enc
.type _gcry_serpent_neon_ocb_enc,%function;
_gcry_serpent_neon_ocb_enc:
	/* input:
	 *	r0  : ctx, CTX
	 *	r1  : dst (8 blocks)
	 *	r2  : src (8 blocks)
	 *	r3  : offset
	 *	sp+0: checksum
	 *	sp+4: L pointers (void *L[8])
	 */

	push {r4-r11, ip, lr};
	add ip, sp, #(10*4);

	vpush {RA4-RB2};

	ldm ip, {r4, lr};

	vld1.8 {RT0}, [r3];
	vld1.8 {RT1}, [r4];

	/* Load L pointers */
	ldm lr!, {r5, r6, r7, r8};
	ldm lr, {r9, r10, r11, ip};

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

	vld1.8 {RA0, RA1}, [r2]!;
	vld1.8 {RA2, RA3}, [r2]!;
	vld1.8 {RB0, RB1}, [r2]!;
	vld1.8 {RB2, RB3}, [r2];

#define OCB_INPUT(lreg, vreg) \
	  vld1.8 {RT3}, [lreg]; \
	  veor RT0, RT3; \
	  veor RT1, vreg; \
	  veor vreg, RT0; \
	  vst1.8 {RT0}, [r1]!;

	OCB_INPUT(r5, RA0);
	OCB_INPUT(r6, RA1);
	OCB_INPUT(r7, RA2);
	OCB_INPUT(r8, RA3);
	OCB_INPUT(r9, RB0);
	OCB_INPUT(r10, RB1);
	OCB_INPUT(r11, RB2);
	OCB_INPUT(ip, RB3);
#undef OCB_INPUT

	sub r1, r1, #(8*16);
	vst1.8 {RT0}, [r3];
	vst1.8 {RT1}, [r4];
	mov r2, r1;

	bl __serpent_enc_blk8;

	vld1.8 {RT0, RT1}, [r1]!;
	veor RT0, RA4, RT0;
	veor RT1, RA1, RT1;
	vld1.8 {RT2, RT3}, [r1]!;
	vst1.8 {RT0, RT1}, [r2]!;
	veor RT2, RA2, RT2;
	veor RT3, RA0, RT3;
	vld1.8 {RT0, RT1}, [r1]!;
	vst1.8 {RT2, RT3}, [r2]!;
	veor RT0, RB4, RT0;
	veor RT1, RB1, RT1;
	vld1.8 {RT2, RT3}, [r1]!;
	vst1.8 {RT0, RT1}, [r2]!;
	veor RT2, RB2, RT2;
	veor RT3, RB0, RT3;
	vst1.8 {RT2, RT3}, [r2]!;

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RA3, RA3;
	veor RB3, RB3;

	pop {r4-r11, ip, pc};
.size _gcry_serpent_neon_ocb_enc,.-_gcry_serpent_neon_ocb_enc;

.align 3
.globl _gcry_serpent_neon_ocb_dec
.type _gcry_serpent_neon_ocb_dec,%function;
_gcry_serpent_neon_ocb_dec:
	/* input:
	 *	r0  : ctx, CTX
	 *	r1  : dst (8 blocks)
	 *	r2  : src (8 blocks)
	 *	r3  : offset
	 *	sp+0: checksum
	 *	sp+4: L pointers (void *L[8])
	 */

	push {r4-r11, ip, lr};
	add ip, sp, #(10*4);

	vpush {RA4-RB2};

	ldm ip, {r4, lr};

	vld1.8 {RT0}, [r3];

	/* Load L pointers */
	ldm lr!, {r5, r6, r7, r8};
	ldm lr, {r9, r10, r11, ip};

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */

	vld1.8 {RA0, RA1}, [r2]!;
	vld1.8 {RA2, RA3}, [r2]!;
	vld1.8 {RB0, RB1}, [r2]!;
	vld1.8 {RB2, RB3}, [r2];

#define OCB_INPUT(lreg, vreg) \
	  vld1.8 {RT3}, [lreg]; \
	  veor RT0, RT3; \
	  veor vreg, RT0; \
	  vst1.8 {RT0}, [r1]!;

	OCB_INPUT(r5, RA0);
	OCB_INPUT(r6, RA1);
	OCB_INPUT(r7, RA2);
	OCB_INPUT(r8, RA3);
	OCB_INPUT(r9, RB0);
	OCB_INPUT(r10, RB1);
	OCB_INPUT(r11, RB2);
	OCB_INPUT(ip, RB3);
#undef OCB_INPUT

	sub r1, r1, #(8*16);
	vst1.8 {RT0}, [r3];
	mov r2, r1;

	bl __serpent_dec_blk8;

	/* Checksum_i = Checksum_{i-1} xor P_i  */
	vld1.8 {RA4}, [r4];

	vld1.8 {RT0, RT1}, [r1]!;
	veor RA0, RA0, RT0;
	veor RA1, RA1, RT1;
	vld1.8 {RT2, RT3}, [r1]!;
	veor RA4, RA4, RA0;
	vst1.8 {RA0, RA1}, [r2]!;
	veor RA4, RA4, RA1;
	veor RA2, RA2, RT2;
	veor RA3, RA3, RT3;
	vld1.8 {RT0, RT1}, [r1]!;
	veor RA4, RA4, RA2;
	vst1.8 {RA2, RA3}, [r2]!;
	veor RA4, RA4, RA3;
	veor RB0, RB0, RT0;
	veor RB1, RB1, RT1;
	vld1.8 {RT2, RT3}, [r1]!;
	veor RA4, RA4, RB0;
	vst1.8 {RB0, RB1}, [r2]!;
	veor RA4, RA4, RB1;
	veor RB2, RB2, RT2;
	veor RB3, RB3, RT3;
	veor RA4, RA4, RB2;
	vst1.8 {RB2, RB3}, [r2]!;

	veor RA4, RA4, RB3;
	vst1.8 {RA4}, [r4];

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RB4, RB4;

	pop {r4-r11, ip, pc};
.size _gcry_serpent_neon_ocb_dec,.-_gcry_serpent_neon_ocb_dec;

.align 3
.globl _gcry_serpent_neon_ocb_auth
.type _gcry_serpent_neon_ocb_auth,%function;
_gcry_serpent_neon_ocb_auth:
	/* input:
	 *	r0  : ctx, CTX
	 *	r1  : abuf (8 blocks)
	 *	r2  : offset
	 *	r3  : checksum
	 *	sp+0: L pointers (void *L[8])
	 */

	push {r5-r11, ip, lr};
	ldr lr, [sp, #(9*4)];

	vpush {RA4-RB2};

	vld1.8 {RT0}, [r2];

	/* Load L pointers */
	ldm lr!, {r5, r6, r7, r8};
	ldm lr, {r9, r10, r11, ip};

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */

	vld1.8 {RA0, RA1}, [r1]!;
	vld1.8 {RA2, RA3}, [r1]!;
	vld1.8 {RB0, RB1}, [r1]!;
	vld1.8 {RB2, RB3}, [r1];

#define OCB_INPUT(lreg, vreg) \
	  vld1.8 {RT3}, [lreg]; \
	  veor RT0, RT3; \
	  veor vreg, RT0;

	OCB_INPUT(r5, RA0);
	OCB_INPUT(r6, RA1);
	OCB_INPUT(r7, RA2);
	OCB_INPUT(r8, RA3);
	OCB_INPUT(r9, RB0);
	OCB_INPUT(r10, RB1);
	OCB_INPUT(r11, RB2);
	OCB_INPUT(ip, RB3);
#undef OCB_INPUT

	vst1.8 {RT0}, [r2];

	bl __serpent_enc_blk8;

	/* Checksum_i = Checksum_{i-1} xor P_i  */
	vld1.8 {RT0}, [r3];

	veor RA4, RB4;
	veor RA1, RB1;
	veor RA2, RB2;
	veor RA0, RB0;

	veor RA2, RT0;
	veor RA1, RA4;
	veor RA0, RA2;

	veor RA0, RA1;

	vst1.8 {RA0}, [r3];

	vpop {RA4-RB2};

	/* clear the used registers */
	veor RA3, RA3;
	veor RB3, RB3;

	pop {r5-r11, ip, pc};
.size _gcry_serpent_neon_ocb_auth,.-_gcry_serpent_neon_ocb_auth;

#endif
