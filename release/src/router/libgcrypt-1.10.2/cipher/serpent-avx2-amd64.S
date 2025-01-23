/* serpent-avx2-amd64.S  -  AVX2 implementation of Serpent cipher
 *
 * Copyright (C) 2013-2015 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#ifdef __x86_64
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_SERPENT) && \
    defined(ENABLE_AVX2_SUPPORT)

#include "asm-common-amd64.h"

/* struct serpent_context: */
#define ctx_keys 0

/* register macros */
#define CTX %rdi

/* vector registers */
#define RA0 %ymm0
#define RA1 %ymm1
#define RA2 %ymm2
#define RA3 %ymm3
#define RA4 %ymm4

#define RB0 %ymm5
#define RB1 %ymm6
#define RB2 %ymm7
#define RB3 %ymm8
#define RB4 %ymm9

#define RNOT %ymm10
#define RTMP0 %ymm11
#define RTMP1 %ymm12
#define RTMP2 %ymm13
#define RTMP3 %ymm14
#define RTMP4 %ymm15

#define RNOTx %xmm10
#define RTMP0x %xmm11
#define RTMP1x %xmm12
#define RTMP2x %xmm13
#define RTMP3x %xmm14
#define RTMP4x %xmm15

/**********************************************************************
  helper macros
 **********************************************************************/

/* vector 32-bit rotation to left */
#define vec_rol(reg, nleft, tmp) \
	vpslld $(nleft), reg, tmp;		\
	vpsrld $(32 - (nleft)), reg, reg;	\
	vpor tmp, reg, reg;

/* vector 32-bit rotation to right */
#define vec_ror(reg, nright, tmp) \
	vec_rol(reg, 32 - nright, tmp)

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3, t1, t2, t3) \
	vpunpckhdq x1, x0, t2; \
	vpunpckldq x1, x0, x0; \
	\
	vpunpckldq x3, x2, t1; \
	vpunpckhdq x3, x2, x2; \
	\
	vpunpckhqdq t1, x0, x1; \
	vpunpcklqdq t1, x0, x0; \
	\
	vpunpckhqdq x2, t2, x3; \
	vpunpcklqdq x2, t2, x2;

/**********************************************************************
  16-way serpent
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
#define SBOX0(r0, r1, r2, r3, r4) \
	vpxor	r0, r3, r3;		vmovdqa	r1, r4;			\
	vpand	r3, r1, r1;		vpxor	r2, r4, r4;		\
	vpxor	r0, r1, r1;		vpor	r3, r0, r0;		\
	vpxor	r4, r0, r0;		vpxor	r3, r4, r4;		\
	vpxor	r2, r3, r3;		vpor	r1, r2, r2;		\
	vpxor	r4, r2, r2;		vpxor	RNOT, r4, r4;		\
	vpor	r1, r4, r4;		vpxor	r3, r1, r1;		\
	vpxor	r4, r1, r1;		vpor	r0, r3, r3;		\
	vpxor	r3, r1, r1;		vpxor	r3, r4, r4;

#define SBOX0_INVERSE(r0, r1, r2, r3, r4) \
	vpxor	RNOT, r2, r2;		vmovdqa	r1, r4;			\
	vpor	r0, r1, r1;		vpxor	RNOT, r4, r4;		\
	vpxor	r2, r1, r1;		vpor	r4, r2, r2;		\
	vpxor	r3, r1, r1;		vpxor	r4, r0, r0;		\
	vpxor	r0, r2, r2;		vpand	r3, r0, r0;		\
	vpxor	r0, r4, r4;		vpor	r1, r0, r0;		\
	vpxor	r2, r0, r0;		vpxor	r4, r3, r3;		\
	vpxor	r1, r2, r2;		vpxor	r0, r3, r3;		\
	vpxor	r1, r3, r3;	\
	vpand	r3, r2, r2;	\
	vpxor	r2, r4, r4;

#define SBOX1(r0, r1, r2, r3, r4) \
	vpxor	RNOT, r0, r0;		vpxor	RNOT, r2, r2;		\
	vmovdqa	r0, r4;			vpand	r1, r0, r0;		\
	vpxor	r0, r2, r2;		vpor	r3, r0, r0;		\
	vpxor	r2, r3, r3;		vpxor	r0, r1, r1;		\
	vpxor	r4, r0, r0;		vpor	r1, r4, r4;		\
	vpxor	r3, r1, r1;		vpor	r0, r2, r2;		\
	vpand	r4, r2, r2;		vpxor	r1, r0, r0;		\
	vpand	r2, r1, r1;	\
	vpxor	r0, r1, r1;		vpand	r2, r0, r0;		\
	vpxor	r4, r0, r0;

#define SBOX1_INVERSE(r0, r1, r2, r3, r4) \
	vmovdqa	r1, r4;			vpxor	r3, r1, r1;		\
	vpand	r1, r3, r3;		vpxor	r2, r4, r4;		\
	vpxor	r0, r3, r3;		vpor	r1, r0, r0;		\
	vpxor	r3, r2, r2;		vpxor	r4, r0, r0;		\
	vpor	r2, r0, r0;		vpxor	r3, r1, r1;		\
	vpxor	r1, r0, r0;		vpor	r3, r1, r1;		\
	vpxor	r0, r1, r1;		vpxor	RNOT, r4, r4;		\
	vpxor	r1, r4, r4;		vpor	r0, r1, r1;		\
	vpxor	r0, r1, r1;	\
	vpor	r4, r1, r1;	\
	vpxor	r1, r3, r3;

#define SBOX2(r0, r1, r2, r3, r4) \
	vmovdqa	r0, r4;			vpand	r2, r0, r0;		\
	vpxor	r3, r0, r0;		vpxor	r1, r2, r2;		\
	vpxor	r0, r2, r2;		vpor	r4, r3, r3;		\
	vpxor	r1, r3, r3;		vpxor	r2, r4, r4;		\
	vmovdqa	r3, r1;			vpor	r4, r3, r3;		\
	vpxor	r0, r3, r3;		vpand	r1, r0, r0;		\
	vpxor	r0, r4, r4;		vpxor	r3, r1, r1;		\
	vpxor	r4, r1, r1;		vpxor	RNOT, r4, r4;

#define SBOX2_INVERSE(r0, r1, r2, r3, r4) \
	vpxor	r3, r2, r2;		vpxor	r0, r3, r3;		\
	vmovdqa	r3, r4;			vpand	r2, r3, r3;		\
	vpxor	r1, r3, r3;		vpor	r2, r1, r1;		\
	vpxor	r4, r1, r1;		vpand	r3, r4, r4;		\
	vpxor	r3, r2, r2;		vpand	r0, r4, r4;		\
	vpxor	r2, r4, r4;		vpand	r1, r2, r2;		\
	vpor	r0, r2, r2;		vpxor	RNOT, r3, r3;		\
	vpxor	r3, r2, r2;		vpxor	r3, r0, r0;		\
	vpand	r1, r0, r0;		vpxor	r4, r3, r3;		\
	vpxor	r0, r3, r3;

#define SBOX3(r0, r1, r2, r3, r4) \
	vmovdqa	r0, r4;			vpor	r3, r0, r0;		\
	vpxor	r1, r3, r3;		vpand	r4, r1, r1;		\
	vpxor	r2, r4, r4;		vpxor	r3, r2, r2;		\
	vpand	r0, r3, r3;		vpor	r1, r4, r4;		\
	vpxor	r4, r3, r3;		vpxor	r1, r0, r0;		\
	vpand	r0, r4, r4;		vpxor	r3, r1, r1;		\
	vpxor	r2, r4, r4;		vpor	r0, r1, r1;		\
	vpxor	r2, r1, r1;		vpxor	r3, r0, r0;		\
	vmovdqa	r1, r2;			vpor	r3, r1, r1;		\
	vpxor	r0, r1, r1;

#define SBOX3_INVERSE(r0, r1, r2, r3, r4) \
	vmovdqa	r2, r4;			vpxor	r1, r2, r2;		\
	vpxor	r2, r0, r0;		vpand	r2, r4, r4;		\
	vpxor	r0, r4, r4;		vpand	r1, r0, r0;		\
	vpxor	r3, r1, r1;		vpor	r4, r3, r3;		\
	vpxor	r3, r2, r2;		vpxor	r3, r0, r0;		\
	vpxor	r4, r1, r1;		vpand	r2, r3, r3;		\
	vpxor	r1, r3, r3;		vpxor	r0, r1, r1;		\
	vpor	r2, r1, r1;		vpxor	r3, r0, r0;		\
	vpxor	r4, r1, r1;	\
	vpxor	r1, r0, r0;

#define SBOX4(r0, r1, r2, r3, r4) \
	vpxor	r3, r1, r1;		vpxor	RNOT, r3, r3;		\
	vpxor	r3, r2, r2;		vpxor	r0, r3, r3;		\
	vmovdqa	r1, r4;			vpand	r3, r1, r1;		\
	vpxor	r2, r1, r1;		vpxor	r3, r4, r4;		\
	vpxor	r4, r0, r0;		vpand	r4, r2, r2;		\
	vpxor	r0, r2, r2;		vpand	r1, r0, r0;		\
	vpxor	r0, r3, r3;		vpor	r1, r4, r4;		\
	vpxor	r0, r4, r4;		vpor	r3, r0, r0;		\
	vpxor	r2, r0, r0;		vpand	r3, r2, r2;		\
	vpxor	RNOT, r0, r0;		vpxor	r2, r4, r4;

#define SBOX4_INVERSE(r0, r1, r2, r3, r4) \
	vmovdqa	r2, r4;			vpand	r3, r2, r2;		\
	vpxor	r1, r2, r2;		vpor	r3, r1, r1;		\
	vpand	r0, r1, r1;		vpxor	r2, r4, r4;		\
	vpxor	r1, r4, r4;		vpand	r2, r1, r1;		\
	vpxor	RNOT, r0, r0;		vpxor	r4, r3, r3;		\
	vpxor	r3, r1, r1;		vpand	r0, r3, r3;		\
	vpxor	r2, r3, r3;		vpxor	r1, r0, r0;		\
	vpand	r0, r2, r2;		vpxor	r0, r3, r3;		\
	vpxor	r4, r2, r2;	\
	vpor	r3, r2, r2;		vpxor	r0, r3, r3;		\
	vpxor	r1, r2, r2;

#define SBOX5(r0, r1, r2, r3, r4) \
	vpxor	r1, r0, r0;		vpxor	r3, r1, r1;		\
	vpxor	RNOT, r3, r3;		vmovdqa	r1, r4;			\
	vpand	r0, r1, r1;		vpxor	r3, r2, r2;		\
	vpxor	r2, r1, r1;		vpor	r4, r2, r2;		\
	vpxor	r3, r4, r4;		vpand	r1, r3, r3;		\
	vpxor	r0, r3, r3;		vpxor	r1, r4, r4;		\
	vpxor	r2, r4, r4;		vpxor	r0, r2, r2;		\
	vpand	r3, r0, r0;		vpxor	RNOT, r2, r2;		\
	vpxor	r4, r0, r0;		vpor	r3, r4, r4;		\
	vpxor	r4, r2, r2;

#define SBOX5_INVERSE(r0, r1, r2, r3, r4) \
	vpxor	RNOT, r1, r1;		vmovdqa	r3, r4;			\
	vpxor	r1, r2, r2;		vpor	r0, r3, r3;		\
	vpxor	r2, r3, r3;		vpor	r1, r2, r2;		\
	vpand	r0, r2, r2;		vpxor	r3, r4, r4;		\
	vpxor	r4, r2, r2;		vpor	r0, r4, r4;		\
	vpxor	r1, r4, r4;		vpand	r2, r1, r1;		\
	vpxor	r3, r1, r1;		vpxor	r2, r4, r4;		\
	vpand	r4, r3, r3;		vpxor	r1, r4, r4;		\
	vpxor	r4, r3, r3;		vpxor	RNOT, r4, r4;		\
	vpxor	r0, r3, r3;

#define SBOX6(r0, r1, r2, r3, r4) \
	vpxor	RNOT, r2, r2;		vmovdqa	r3, r4;			\
	vpand	r0, r3, r3;		vpxor	r4, r0, r0;		\
	vpxor	r2, r3, r3;		vpor	r4, r2, r2;		\
	vpxor	r3, r1, r1;		vpxor	r0, r2, r2;		\
	vpor	r1, r0, r0;		vpxor	r1, r2, r2;		\
	vpxor	r0, r4, r4;		vpor	r3, r0, r0;		\
	vpxor	r2, r0, r0;		vpxor	r3, r4, r4;		\
	vpxor	r0, r4, r4;		vpxor	RNOT, r3, r3;		\
	vpand	r4, r2, r2;	\
	vpxor	r3, r2, r2;

#define SBOX6_INVERSE(r0, r1, r2, r3, r4) \
	vpxor	r2, r0, r0;		vmovdqa	r2, r4;			\
	vpand	r0, r2, r2;		vpxor	r3, r4, r4;		\
	vpxor	RNOT, r2, r2;		vpxor	r1, r3, r3;		\
	vpxor	r3, r2, r2;		vpor	r0, r4, r4;		\
	vpxor	r2, r0, r0;		vpxor	r4, r3, r3;		\
	vpxor	r1, r4, r4;		vpand	r3, r1, r1;		\
	vpxor	r0, r1, r1;		vpxor	r3, r0, r0;		\
	vpor	r2, r0, r0;		vpxor	r1, r3, r3;		\
	vpxor	r0, r4, r4;

#define SBOX7(r0, r1, r2, r3, r4) \
	vmovdqa	r1, r4;			vpor	r2, r1, r1;		\
	vpxor	r3, r1, r1;		vpxor	r2, r4, r4;		\
	vpxor	r1, r2, r2;		vpor	r4, r3, r3;		\
	vpand	r0, r3, r3;		vpxor	r2, r4, r4;		\
	vpxor	r1, r3, r3;		vpor	r4, r1, r1;		\
	vpxor	r0, r1, r1;		vpor	r4, r0, r0;		\
	vpxor	r2, r0, r0;		vpxor	r4, r1, r1;		\
	vpxor	r1, r2, r2;		vpand	r0, r1, r1;		\
	vpxor	r4, r1, r1;		vpxor	RNOT, r2, r2;		\
	vpor	r0, r2, r2;	\
	vpxor	r2, r4, r4;

#define SBOX7_INVERSE(r0, r1, r2, r3, r4) \
	vmovdqa	r2, r4;			vpxor	r0, r2, r2;		\
	vpand	r3, r0, r0;		vpor	r3, r4, r4;		\
	vpxor	RNOT, r2, r2;		vpxor	r1, r3, r3;		\
	vpor	r0, r1, r1;		vpxor	r2, r0, r0;		\
	vpand	r4, r2, r2;		vpand	r4, r3, r3;		\
	vpxor	r2, r1, r1;		vpxor	r0, r2, r2;		\
	vpor	r2, r0, r0;		vpxor	r1, r4, r4;		\
	vpxor	r3, r0, r0;		vpxor	r4, r3, r3;		\
	vpor	r0, r4, r4;		vpxor	r2, r3, r3;		\
	vpxor	r2, r4, r4;

/* Apply SBOX number WHICH to to the block.  */
#define SBOX(which, r0, r1, r2, r3, r4) \
	SBOX##which (r0, r1, r2, r3, r4)

/* Apply inverse SBOX number WHICH to to the block.  */
#define SBOX_INVERSE(which, r0, r1, r2, r3, r4) \
	SBOX##which##_INVERSE (r0, r1, r2, r3, r4)

/* XOR round key into block state in r0,r1,r2,r3. r4 used as temporary.  */
#define BLOCK_XOR_KEY(r0, r1, r2, r3, r4, round) \
	vpbroadcastd (ctx_keys + (round) * 16 + 0 * 4)(CTX), r4; \
	vpxor r4, r0, r0; \
	vpbroadcastd (ctx_keys + (round) * 16 + 1 * 4)(CTX), r4; \
	vpxor r4, r1, r1; \
	vpbroadcastd (ctx_keys + (round) * 16 + 2 * 4)(CTX), r4; \
	vpxor r4, r2, r2; \
	vpbroadcastd (ctx_keys + (round) * 16 + 3 * 4)(CTX), r4; \
	vpxor r4, r3, r3;

/* Apply the linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION(r0, r1, r2, r3, r4) \
	vec_rol(r0, 13, r4);	\
	vec_rol(r2, 3, r4);	\
	vpxor r0, r1, r1;	\
	vpxor r2, r1, r1;	\
	vpslld $3, r0, r4;	\
	vpxor r2, r3, r3;	\
	vpxor r4, r3, r3;	\
	vec_rol(r1, 1, r4);	\
	vec_rol(r3, 7, r4);	\
	vpxor r1, r0, r0;	\
	vpxor r3, r0, r0;	\
	vpslld $7, r1, r4;	\
	vpxor r3, r2, r2;	\
	vpxor r4, r2, r2;	\
	vec_rol(r0, 5, r4);	\
	vec_rol(r2, 22, r4);

/* Apply the inverse linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION_INVERSE(r0, r1, r2, r3, r4) \
	vec_ror(r2, 22, r4);	\
	vec_ror(r0, 5, r4);	\
	vpslld $7, r1, r4;	\
	vpxor r3, r2, r2;	\
	vpxor r4, r2, r2;	\
	vpxor r1, r0, r0;	\
	vpxor r3, r0, r0;	\
	vec_ror(r3, 7, r4);	\
	vec_ror(r1, 1, r4);	\
	vpslld $3, r0, r4;	\
	vpxor r2, r3, r3;	\
	vpxor r4, r3, r3;	\
	vpxor r0, r1, r1;	\
	vpxor r2, r1, r1;	\
	vec_ror(r2, 3, r4);	\
	vec_ror(r0, 13, r4);

/* Apply a Serpent round to sixteen parallel blocks.  This macro increments
   `round'.  */
#define ROUND(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
			    b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, round);		\
	SBOX (which, a0, a1, a2, a3, a4);			\
		BLOCK_XOR_KEY (b0, b1, b2, b3, b4, round);		\
		SBOX (which, b0, b1, b2, b3, b4);			\
	LINEAR_TRANSFORMATION (na0, na1, na2, na3, na4);	\
		LINEAR_TRANSFORMATION (nb0, nb1, nb2, nb3, nb4);

/* Apply the last Serpent round to sixteen parallel blocks.  This macro
   increments `round'.  */
#define ROUND_LAST(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
				 b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, round);		\
	SBOX (which, a0, a1, a2, a3, a4);			\
		BLOCK_XOR_KEY (b0, b1, b2, b3, b4, round);		\
		SBOX (which, b0, b1, b2, b3, b4);			\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, ((round) + 1));		\
		BLOCK_XOR_KEY (nb0, nb1, nb2, nb3, nb4, ((round) + 1));

/* Apply an inverse Serpent round to sixteen parallel blocks.  This macro
   increments `round'.  */
#define ROUND_INVERSE(round, which, a0, a1, a2, a3, a4, \
				    na0, na1, na2, na3, na4, \
				    b0, b1, b2, b3, b4, \
				    nb0, nb1, nb2, nb3, nb4) \
	LINEAR_TRANSFORMATION_INVERSE (a0, a1, a2, a3, a4);	\
		LINEAR_TRANSFORMATION_INVERSE (b0, b1, b2, b3, b4);	\
	SBOX_INVERSE (which, a0, a1, a2, a3, a4);		\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, round);		\
		SBOX_INVERSE (which, b0, b1, b2, b3, b4);		\
		BLOCK_XOR_KEY (nb0, nb1, nb2, nb3, nb4, round);

/* Apply the first inverse Serpent round to sixteen parallel blocks.  This macro
   increments `round'.  */
#define ROUND_FIRST_INVERSE(round, which, a0, a1, a2, a3, a4, \
					  na0, na1, na2, na3, na4, \
					  b0, b1, b2, b3, b4, \
					  nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, ((round) + 1));	\
		BLOCK_XOR_KEY (b0, b1, b2, b3, b4, ((round) + 1));	\
	SBOX_INVERSE (which, a0, a1, a2, a3, a4); 	\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, round);	\
		SBOX_INVERSE (which, b0, b1, b2, b3, b4); 	\
		BLOCK_XOR_KEY (nb0, nb1, nb2, nb3, nb4, round);

.text

.align 8
ELF(.type   __serpent_enc_blk16,@function;)
__serpent_enc_blk16:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: sixteen parallel
	 *						plaintext blocks
	 * output:
	 *	RA4, RA1, RA2, RA0, RB4, RB1, RB2, RB0: sixteen parallel
	 * 						ciphertext blocks
	 */
	CFI_STARTPROC();

	vpcmpeqd RNOT, RNOT, RNOT;

	transpose_4x4(RA0, RA1, RA2, RA3, RA4, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RB4, RTMP0, RTMP1);

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

	transpose_4x4(RA4, RA1, RA2, RA0, RA3, RTMP0, RTMP1);
	transpose_4x4(RB4, RB1, RB2, RB0, RB3, RTMP0, RTMP1);

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __serpent_enc_blk16,.-__serpent_enc_blk16;)

.align 8
ELF(.type   __serpent_dec_blk16,@function;)
__serpent_dec_blk16:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: sixteen parallel
	 * 						ciphertext blocks
	 * output:
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: sixteen parallel
	 *						plaintext blocks
	 */
	CFI_STARTPROC();

	vpcmpeqd RNOT, RNOT, RNOT;

	transpose_4x4(RA0, RA1, RA2, RA3, RA4, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RB4, RTMP0, RTMP1);

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

	transpose_4x4(RA0, RA1, RA2, RA3, RA4, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RB4, RTMP0, RTMP1);

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __serpent_dec_blk16,.-__serpent_dec_blk16;)

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

.align 8
.globl _gcry_serpent_avx2_ctr_enc
ELF(.type   _gcry_serpent_avx2_ctr_enc,@function;)
_gcry_serpent_avx2_ctr_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();

	movq 8(%rcx), %rax;
	bswapq %rax;

	vzeroupper;

	vbroadcasti128 .Lbswap128_mask rRIP, RTMP3;
	vpcmpeqd RNOT, RNOT, RNOT;
	vpsrldq $8, RNOT, RNOT;   /* ab: -1:0 ; cd: -1:0 */
	vpaddq RNOT, RNOT, RTMP2; /* ab: -2:0 ; cd: -2:0 */

	/* load IV and byteswap */
	vmovdqu (%rcx), RTMP4x;
	vpshufb RTMP3x, RTMP4x, RTMP4x;
	vmovdqa RTMP4x, RTMP0x;
	inc_le128(RTMP4x, RNOTx, RTMP1x);
	vinserti128 $1, RTMP4x, RTMP0, RTMP0;
	vpshufb RTMP3, RTMP0, RA0; /* +1 ; +0 */

	/* check need for handling 64-bit overflow and carry */
	cmpq $(0xffffffffffffffff - 16), %rax;
	ja .Lhandle_ctr_carry;

	/* construct IVs */
	vpsubq RTMP2, RTMP0, RTMP0; /* +3 ; +2 */
	vpshufb RTMP3, RTMP0, RA1;
	vpsubq RTMP2, RTMP0, RTMP0; /* +5 ; +4 */
	vpshufb RTMP3, RTMP0, RA2;
	vpsubq RTMP2, RTMP0, RTMP0; /* +7 ; +6 */
	vpshufb RTMP3, RTMP0, RA3;
	vpsubq RTMP2, RTMP0, RTMP0; /* +9 ; +8 */
	vpshufb RTMP3, RTMP0, RB0;
	vpsubq RTMP2, RTMP0, RTMP0; /* +11 ; +10 */
	vpshufb RTMP3, RTMP0, RB1;
	vpsubq RTMP2, RTMP0, RTMP0; /* +13 ; +12 */
	vpshufb RTMP3, RTMP0, RB2;
	vpsubq RTMP2, RTMP0, RTMP0; /* +15 ; +14 */
	vpshufb RTMP3, RTMP0, RB3;
	vpsubq RTMP2, RTMP0, RTMP0; /* +16 */
	vpshufb RTMP3x, RTMP0x, RTMP0x;

	jmp .Lctr_carry_done;

.Lhandle_ctr_carry:
	/* construct IVs */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RA1; /* +3 ; +2 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RA2; /* +5 ; +4 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RA3; /* +7 ; +6 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RB0; /* +9 ; +8 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RB1; /* +11 ; +10 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RB2; /* +13 ; +12 */
	inc_le128(RTMP0, RNOT, RTMP1);
	inc_le128(RTMP0, RNOT, RTMP1);
	vpshufb RTMP3, RTMP0, RB3; /* +15 ; +14 */
	inc_le128(RTMP0, RNOT, RTMP1);
	vextracti128 $1, RTMP0, RTMP0x;
	vpshufb RTMP3x, RTMP0x, RTMP0x; /* +16 */

.align 4
.Lctr_carry_done:
	/* store new IV */
	vmovdqu RTMP0x, (%rcx);

	call __serpent_enc_blk16;

	vpxor (0 * 32)(%rdx), RA4, RA4;
	vpxor (1 * 32)(%rdx), RA1, RA1;
	vpxor (2 * 32)(%rdx), RA2, RA2;
	vpxor (3 * 32)(%rdx), RA0, RA0;
	vpxor (4 * 32)(%rdx), RB4, RB4;
	vpxor (5 * 32)(%rdx), RB1, RB1;
	vpxor (6 * 32)(%rdx), RB2, RB2;
	vpxor (7 * 32)(%rdx), RB0, RB0;

	vmovdqu RA4, (0 * 32)(%rsi);
	vmovdqu RA1, (1 * 32)(%rsi);
	vmovdqu RA2, (2 * 32)(%rsi);
	vmovdqu RA0, (3 * 32)(%rsi);
	vmovdqu RB4, (4 * 32)(%rsi);
	vmovdqu RB1, (5 * 32)(%rsi);
	vmovdqu RB2, (6 * 32)(%rsi);
	vmovdqu RB0, (7 * 32)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_ctr_enc,.-_gcry_serpent_avx2_ctr_enc;)

.align 8
.globl _gcry_serpent_avx2_cbc_dec
ELF(.type   _gcry_serpent_avx2_cbc_dec,@function;)
_gcry_serpent_avx2_cbc_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	vzeroupper;

	vmovdqu (0 * 32)(%rdx), RA0;
	vmovdqu (1 * 32)(%rdx), RA1;
	vmovdqu (2 * 32)(%rdx), RA2;
	vmovdqu (3 * 32)(%rdx), RA3;
	vmovdqu (4 * 32)(%rdx), RB0;
	vmovdqu (5 * 32)(%rdx), RB1;
	vmovdqu (6 * 32)(%rdx), RB2;
	vmovdqu (7 * 32)(%rdx), RB3;

	call __serpent_dec_blk16;

	vmovdqu (%rcx), RNOTx;
	vinserti128 $1, (%rdx), RNOT, RNOT;
	vpxor RNOT, RA0, RA0;
	vpxor (0 * 32 + 16)(%rdx), RA1, RA1;
	vpxor (1 * 32 + 16)(%rdx), RA2, RA2;
	vpxor (2 * 32 + 16)(%rdx), RA3, RA3;
	vpxor (3 * 32 + 16)(%rdx), RB0, RB0;
	vpxor (4 * 32 + 16)(%rdx), RB1, RB1;
	vpxor (5 * 32 + 16)(%rdx), RB2, RB2;
	vpxor (6 * 32 + 16)(%rdx), RB3, RB3;
	vmovdqu (7 * 32 + 16)(%rdx), RNOTx;
	vmovdqu RNOTx, (%rcx); /* store new IV */

	vmovdqu RA0, (0 * 32)(%rsi);
	vmovdqu RA1, (1 * 32)(%rsi);
	vmovdqu RA2, (2 * 32)(%rsi);
	vmovdqu RA3, (3 * 32)(%rsi);
	vmovdqu RB0, (4 * 32)(%rsi);
	vmovdqu RB1, (5 * 32)(%rsi);
	vmovdqu RB2, (6 * 32)(%rsi);
	vmovdqu RB3, (7 * 32)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_cbc_dec,.-_gcry_serpent_avx2_cbc_dec;)

.align 8
.globl _gcry_serpent_avx2_cfb_dec
ELF(.type   _gcry_serpent_avx2_cfb_dec,@function;)
_gcry_serpent_avx2_cfb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	vzeroupper;

	/* Load input */
	vmovdqu (%rcx), RNOTx;
	vinserti128 $1, (%rdx), RNOT, RA0;
	vmovdqu (0 * 32 + 16)(%rdx), RA1;
	vmovdqu (1 * 32 + 16)(%rdx), RA2;
	vmovdqu (2 * 32 + 16)(%rdx), RA3;
	vmovdqu (3 * 32 + 16)(%rdx), RB0;
	vmovdqu (4 * 32 + 16)(%rdx), RB1;
	vmovdqu (5 * 32 + 16)(%rdx), RB2;
	vmovdqu (6 * 32 + 16)(%rdx), RB3;

	/* Update IV */
	vmovdqu (7 * 32 + 16)(%rdx), RNOTx;
	vmovdqu RNOTx, (%rcx);

	call __serpent_enc_blk16;

	vpxor (0 * 32)(%rdx), RA4, RA4;
	vpxor (1 * 32)(%rdx), RA1, RA1;
	vpxor (2 * 32)(%rdx), RA2, RA2;
	vpxor (3 * 32)(%rdx), RA0, RA0;
	vpxor (4 * 32)(%rdx), RB4, RB4;
	vpxor (5 * 32)(%rdx), RB1, RB1;
	vpxor (6 * 32)(%rdx), RB2, RB2;
	vpxor (7 * 32)(%rdx), RB0, RB0;

	vmovdqu RA4, (0 * 32)(%rsi);
	vmovdqu RA1, (1 * 32)(%rsi);
	vmovdqu RA2, (2 * 32)(%rsi);
	vmovdqu RA0, (3 * 32)(%rsi);
	vmovdqu RB4, (4 * 32)(%rsi);
	vmovdqu RB1, (5 * 32)(%rsi);
	vmovdqu RB2, (6 * 32)(%rsi);
	vmovdqu RB0, (7 * 32)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_cfb_dec,.-_gcry_serpent_avx2_cfb_dec;)

.align 8
.globl _gcry_serpent_avx2_ocb_enc
ELF(.type _gcry_serpent_avx2_ocb_enc,@function;)

_gcry_serpent_avx2_ocb_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	vzeroupper;

	subq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(4 * 8);

	movq %r10, (0 * 8)(%rsp);
	movq %r11, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	CFI_REL_OFFSET(%r10, 0 * 8);
	CFI_REL_OFFSET(%r11, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);

	vmovdqu (%rcx), RTMP0x;
	vmovdqu (%r8), RTMP1x;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rdx), yreg; \
	  vpxor (l0reg), RTMP0x, RNOTx; \
	  vpxor (l1reg), RNOTx, RTMP0x; \
	  vinserti128 $1, RTMP0x, RNOT, RNOT; \
	  vpxor yreg, RTMP1, RTMP1; \
	  vpxor yreg, RNOT, yreg; \
	  vmovdqu RNOT, (n * 32)(%rsi);

	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %r11, RA0);
	OCB_INPUT(1, %r12, %r13, RA1);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(2, %r10, %r11, RA2);
	OCB_INPUT(3, %r12, %r13, RA3);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %r11, RB0);
	OCB_INPUT(5, %r12, %r13, RB1);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(6, %r10, %r11, RB2);
	OCB_INPUT(7, %r12, %r13, RB3);
#undef OCB_INPUT

	vextracti128 $1, RTMP1, RNOTx;
	vmovdqu RTMP0x, (%rcx);
	vpxor RNOTx, RTMP1x, RTMP1x;
	vmovdqu RTMP1x, (%r8);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_enc_blk16;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vpxor (0 * 32)(%rsi), RA4, RA4;
	vpxor (1 * 32)(%rsi), RA1, RA1;
	vpxor (2 * 32)(%rsi), RA2, RA2;
	vpxor (3 * 32)(%rsi), RA0, RA0;
	vpxor (4 * 32)(%rsi), RB4, RB4;
	vpxor (5 * 32)(%rsi), RB1, RB1;
	vpxor (6 * 32)(%rsi), RB2, RB2;
	vpxor (7 * 32)(%rsi), RB0, RB0;

	vmovdqu RA4, (0 * 32)(%rsi);
	vmovdqu RA1, (1 * 32)(%rsi);
	vmovdqu RA2, (2 * 32)(%rsi);
	vmovdqu RA0, (3 * 32)(%rsi);
	vmovdqu RB4, (4 * 32)(%rsi);
	vmovdqu RB1, (5 * 32)(%rsi);
	vmovdqu RB2, (6 * 32)(%rsi);
	vmovdqu RB0, (7 * 32)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_ocb_enc,.-_gcry_serpent_avx2_ocb_enc;)

.align 8
.globl _gcry_serpent_avx2_ocb_dec
ELF(.type _gcry_serpent_avx2_ocb_dec,@function;)

_gcry_serpent_avx2_ocb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	vzeroupper;

	subq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(4 * 8);

	movq %r10, (0 * 8)(%rsp);
	movq %r11, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	CFI_REL_OFFSET(%r10, 0 * 8);
	CFI_REL_OFFSET(%r11, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);

	vmovdqu (%rcx), RTMP0x;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rdx), yreg; \
	  vpxor (l0reg), RTMP0x, RNOTx; \
	  vpxor (l1reg), RNOTx, RTMP0x; \
	  vinserti128 $1, RTMP0x, RNOT, RNOT; \
	  vpxor yreg, RNOT, yreg; \
	  vmovdqu RNOT, (n * 32)(%rsi);

	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %r11, RA0);
	OCB_INPUT(1, %r12, %r13, RA1);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(2, %r10, %r11, RA2);
	OCB_INPUT(3, %r12, %r13, RA3);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %r11, RB0);
	OCB_INPUT(5, %r12, %r13, RB1);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(6, %r10, %r11, RB2);
	OCB_INPUT(7, %r12, %r13, RB3);
#undef OCB_INPUT

	vmovdqu RTMP0x, (%rcx);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_dec_blk16;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vmovdqu (%r8), RTMP1x;

	vpxor (0 * 32)(%rsi), RA0, RA0;
	vpxor (1 * 32)(%rsi), RA1, RA1;
	vpxor (2 * 32)(%rsi), RA2, RA2;
	vpxor (3 * 32)(%rsi), RA3, RA3;
	vpxor (4 * 32)(%rsi), RB0, RB0;
	vpxor (5 * 32)(%rsi), RB1, RB1;
	vpxor (6 * 32)(%rsi), RB2, RB2;
	vpxor (7 * 32)(%rsi), RB3, RB3;

	/* Checksum_i = Checksum_{i-1} xor P_i  */

	vmovdqu RA0, (0 * 32)(%rsi);
	vpxor RA0, RTMP1, RTMP1;
	vmovdqu RA1, (1 * 32)(%rsi);
	vpxor RA1, RTMP1, RTMP1;
	vmovdqu RA2, (2 * 32)(%rsi);
	vpxor RA2, RTMP1, RTMP1;
	vmovdqu RA3, (3 * 32)(%rsi);
	vpxor RA3, RTMP1, RTMP1;
	vmovdqu RB0, (4 * 32)(%rsi);
	vpxor RB0, RTMP1, RTMP1;
	vmovdqu RB1, (5 * 32)(%rsi);
	vpxor RB1, RTMP1, RTMP1;
	vmovdqu RB2, (6 * 32)(%rsi);
	vpxor RB2, RTMP1, RTMP1;
	vmovdqu RB3, (7 * 32)(%rsi);
	vpxor RB3, RTMP1, RTMP1;

	vextracti128 $1, RTMP1, RNOTx;
	vpxor RNOTx, RTMP1x, RTMP1x;
	vmovdqu RTMP1x, (%r8);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_ocb_dec,.-_gcry_serpent_avx2_ocb_dec;)

.align 8
.globl _gcry_serpent_avx2_ocb_auth
ELF(.type _gcry_serpent_avx2_ocb_auth,@function;)

_gcry_serpent_avx2_ocb_auth:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: abuf (16 blocks)
	 *	%rdx: offset
	 *	%rcx: checksum
	 *	%r8 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	vzeroupper;

	subq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(4 * 8);

	movq %r10, (0 * 8)(%rsp);
	movq %r11, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	CFI_REL_OFFSET(%r10, 0 * 8);
	CFI_REL_OFFSET(%r11, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);

	vmovdqu (%rdx), RTMP0x;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rsi), yreg; \
	  vpxor (l0reg), RTMP0x, RNOTx; \
	  vpxor (l1reg), RNOTx, RTMP0x; \
	  vinserti128 $1, RTMP0x, RNOT, RNOT; \
	  vpxor yreg, RNOT, yreg;

	movq (0 * 8)(%r8), %r10;
	movq (1 * 8)(%r8), %r11;
	movq (2 * 8)(%r8), %r12;
	movq (3 * 8)(%r8), %r13;
	OCB_INPUT(0, %r10, %r11, RA0);
	OCB_INPUT(1, %r12, %r13, RA1);
	movq (4 * 8)(%r8), %r10;
	movq (5 * 8)(%r8), %r11;
	movq (6 * 8)(%r8), %r12;
	movq (7 * 8)(%r8), %r13;
	OCB_INPUT(2, %r10, %r11, RA2);
	OCB_INPUT(3, %r12, %r13, RA3);
	movq (8 * 8)(%r8), %r10;
	movq (9 * 8)(%r8), %r11;
	movq (10 * 8)(%r8), %r12;
	movq (11 * 8)(%r8), %r13;
	OCB_INPUT(4, %r10, %r11, RB0);
	OCB_INPUT(5, %r12, %r13, RB1);
	movq (12 * 8)(%r8), %r10;
	movq (13 * 8)(%r8), %r11;
	movq (14 * 8)(%r8), %r12;
	movq (15 * 8)(%r8), %r13;
	OCB_INPUT(6, %r10, %r11, RB2);
	OCB_INPUT(7, %r12, %r13, RB3);
#undef OCB_INPUT

	vmovdqu RTMP0x, (%rdx);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_enc_blk16;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vpxor RA4, RB4, RA4;
	vpxor RA1, RB1, RA1;
	vpxor RA2, RB2, RA2;
	vpxor RA0, RB0, RA0;

	vpxor RA4, RA1, RA1;
	vpxor RA2, RA0, RA0;

	vpxor RA1, RA0, RTMP1;

	vextracti128 $1, RTMP1, RNOTx;
	vpxor (%rcx), RTMP1x, RTMP1x;
	vpxor RNOTx, RTMP1x, RTMP1x;
	vmovdqu RTMP1x, (%rcx);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_avx2_ocb_auth,.-_gcry_serpent_avx2_ocb_auth;)

.align 16

/* For CTR-mode IV byteswap */
.Lbswap128_mask:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#endif /*defined(USE_SERPENT) && defined(ENABLE_AVX2_SUPPORT)*/
#endif /*__x86_64*/
