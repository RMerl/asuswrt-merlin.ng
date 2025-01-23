/* serpent-sse2-amd64.S  -  SSE2 implementation of Serpent cipher
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

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_SERPENT)

#include "asm-common-amd64.h"

/* struct serpent_context: */
#define ctx_keys 0

/* register macros */
#define CTX %rdi

/* vector registers */
#define RA0 %xmm0
#define RA1 %xmm1
#define RA2 %xmm2
#define RA3 %xmm3
#define RA4 %xmm4

#define RB0 %xmm5
#define RB1 %xmm6
#define RB2 %xmm7
#define RB3 %xmm8
#define RB4 %xmm9

#define RNOT %xmm10
#define RTMP0 %xmm11
#define RTMP1 %xmm12
#define RTMP2 %xmm13

/**********************************************************************
  helper macros
 **********************************************************************/

/* vector 32-bit rotation to left */
#define vec_rol(reg, nleft, tmp) \
	movdqa reg, tmp; 		\
	pslld $(nleft), tmp;		\
	psrld $(32 - (nleft)), reg;	\
	por tmp, reg;

/* vector 32-bit rotation to right */
#define vec_ror(reg, nright, tmp) \
	vec_rol(reg, 32 - nright, tmp)

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3, t1, t2, t3) \
	movdqa    x0, t2; \
	punpckhdq x1, t2; \
	punpckldq x1, x0; \
	\
	movdqa    x2, t1; \
	punpckldq x3, t1; \
	punpckhdq x3, x2; \
	\
	movdqa     x0, x1; \
	punpckhqdq t1, x1; \
	punpcklqdq t1, x0; \
	\
	movdqa     t2, x3; \
	punpckhqdq x2, x3; \
	punpcklqdq x2, t2; \
	movdqa     t2, x2;

/* fill xmm register with 32-bit value from memory */
#define pbroadcastd(mem32, xreg) \
	movd mem32, xreg; \
	pshufd $0, xreg, xreg;

/* xor with unaligned memory operand */
#define pxor_u(umem128, xreg, t) \
	movdqu umem128, t; \
	pxor t, xreg;

/* 128-bit wide byte swap */
#define pbswap(xreg, t0) \
	/* reorder 32-bit words, [a,b,c,d] => [d,c,b,a] */ \
	pshufd $0x1b, xreg, xreg; \
	/* reorder high&low 16-bit words, [d0,d1,c0,c1] => [d1,d0,c1,c0] */ \
	pshuflw $0xb1, xreg, xreg; \
	pshufhw $0xb1, xreg, xreg; \
	/* reorder bytes in 16-bit words */ \
	movdqa xreg, t0; \
	psrlw $8, t0; \
	psllw $8, xreg; \
	por t0, xreg;

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
#define SBOX0(r0, r1, r2, r3, r4) \
	pxor	r0, r3;		movdqa	r1, r4;		\
	pand	r3, r1;		pxor	r2, r4;		\
	pxor	r0, r1;		por	r3, r0;		\
	pxor	r4, r0;		pxor	r3, r4;		\
	pxor	r2, r3;		por	r1, r2;		\
	pxor	r4, r2;		pxor	RNOT, r4;	\
	por	r1, r4;		pxor	r3, r1;		\
	pxor	r4, r1;		por	r0, r3;		\
	pxor	r3, r1;		pxor	r3, r4;

#define SBOX0_INVERSE(r0, r1, r2, r3, r4) \
	pxor	RNOT, r2;	movdqa	r1, r4;		\
	por	r0, r1;		pxor	RNOT, r4;	\
	pxor	r2, r1;		por	r4, r2;		\
	pxor	r3, r1;		pxor	r4, r0;		\
	pxor	r0, r2;		pand	r3, r0;		\
	pxor	r0, r4;		por	r1, r0;		\
	pxor	r2, r0;		pxor	r4, r3;		\
	pxor	r1, r2;		pxor	r0, r3;		\
	pxor	r1, r3;	\
	pand	r3, r2;	\
	pxor	r2, r4;

#define SBOX1(r0, r1, r2, r3, r4) \
	pxor	RNOT, r0;	pxor	RNOT, r2;	\
	movdqa	r0, r4;		pand	r1, r0;		\
	pxor	r0, r2;		por	r3, r0;		\
	pxor	r2, r3;		pxor	r0, r1;		\
	pxor	r4, r0;		por	r1, r4;		\
	pxor	r3, r1;		por	r0, r2;		\
	pand	r4, r2;		pxor	r1, r0;		\
	pand	r2, r1;	\
	pxor	r0, r1;		pand	r2, r0;		\
	pxor	r4, r0;

#define SBOX1_INVERSE(r0, r1, r2, r3, r4) \
	movdqa	r1, r4;		pxor	r3, r1;		\
	pand	r1, r3;		pxor	r2, r4;		\
	pxor	r0, r3;		por	r1, r0;		\
	pxor	r3, r2;		pxor	r4, r0;		\
	por	r2, r0;		pxor	r3, r1;		\
	pxor	r1, r0;		por	r3, r1;		\
	pxor	r0, r1;		pxor	RNOT, r4;	\
	pxor	r1, r4;		por	r0, r1;		\
	pxor	r0, r1;	\
	por	r4, r1;	\
	pxor	r1, r3;

#define SBOX2(r0, r1, r2, r3, r4) \
	movdqa	r0, r4;		pand	r2, r0;		\
	pxor	r3, r0;		pxor	r1, r2;		\
	pxor	r0, r2;		por	r4, r3;		\
	pxor	r1, r3;		pxor	r2, r4;		\
	movdqa	r3, r1;		por	r4, r3;		\
	pxor	r0, r3;		pand	r1, r0;		\
	pxor	r0, r4;		pxor	r3, r1;		\
	pxor	r4, r1;		pxor	RNOT, r4;

#define SBOX2_INVERSE(r0, r1, r2, r3, r4) \
	pxor	r3, r2;		pxor	r0, r3;		\
	movdqa	r3, r4;		pand	r2, r3;		\
	pxor	r1, r3;		por	r2, r1;		\
	pxor	r4, r1;		pand	r3, r4;		\
	pxor	r3, r2;		pand	r0, r4;		\
	pxor	r2, r4;		pand	r1, r2;		\
	por	r0, r2;		pxor	RNOT, r3;	\
	pxor	r3, r2;		pxor	r3, r0;		\
	pand	r1, r0;		pxor	r4, r3;		\
	pxor	r0, r3;

#define SBOX3(r0, r1, r2, r3, r4) \
	movdqa	r0, r4;		por	r3, r0;		\
	pxor	r1, r3;		pand	r4, r1;		\
	pxor	r2, r4;		pxor	r3, r2;		\
	pand	r0, r3;		por	r1, r4;		\
	pxor	r4, r3;		pxor	r1, r0;		\
	pand	r0, r4;		pxor	r3, r1;		\
	pxor	r2, r4;		por	r0, r1;		\
	pxor	r2, r1;		pxor	r3, r0;		\
	movdqa	r1, r2;		por	r3, r1;		\
	pxor	r0, r1;

#define SBOX3_INVERSE(r0, r1, r2, r3, r4) \
	movdqa	r2, r4;		pxor	r1, r2;		\
	pxor	r2, r0;		pand	r2, r4;		\
	pxor	r0, r4;		pand	r1, r0;		\
	pxor	r3, r1;		por	r4, r3;		\
	pxor	r3, r2;		pxor	r3, r0;		\
	pxor	r4, r1;		pand	r2, r3;		\
	pxor	r1, r3;		pxor	r0, r1;		\
	por	r2, r1;		pxor	r3, r0;		\
	pxor	r4, r1;	\
	pxor	r1, r0;

#define SBOX4(r0, r1, r2, r3, r4) \
	pxor	r3, r1;		pxor	RNOT, r3;	\
	pxor	r3, r2;		pxor	r0, r3;		\
	movdqa	r1, r4;		pand	r3, r1;		\
	pxor	r2, r1;		pxor	r3, r4;		\
	pxor	r4, r0;		pand	r4, r2;		\
	pxor	r0, r2;		pand	r1, r0;		\
	pxor	r0, r3;		por	r1, r4;		\
	pxor	r0, r4;		por	r3, r0;		\
	pxor	r2, r0;		pand	r3, r2;		\
	pxor	RNOT, r0;	pxor	r2, r4;

#define SBOX4_INVERSE(r0, r1, r2, r3, r4) \
	movdqa	r2, r4;		pand	r3, r2;		\
	pxor	r1, r2;		por	r3, r1;		\
	pand	r0, r1;		pxor	r2, r4;		\
	pxor	r1, r4;		pand	r2, r1;		\
	pxor	RNOT, r0;	pxor	r4, r3;		\
	pxor	r3, r1;		pand	r0, r3;		\
	pxor	r2, r3;		pxor	r1, r0;		\
	pand	r0, r2;		pxor	r0, r3;		\
	pxor	r4, r2;	\
	por	r3, r2;		pxor	r0, r3;		\
	pxor	r1, r2;

#define SBOX5(r0, r1, r2, r3, r4) \
	pxor	r1, r0;		pxor	r3, r1;		\
	pxor	RNOT, r3;	movdqa	r1, r4;		\
	pand	r0, r1;		pxor	r3, r2;		\
	pxor	r2, r1;		por	r4, r2;		\
	pxor	r3, r4;		pand	r1, r3;		\
	pxor	r0, r3;		pxor	r1, r4;		\
	pxor	r2, r4;		pxor	r0, r2;		\
	pand	r3, r0;		pxor	RNOT, r2;	\
	pxor	r4, r0;		por	r3, r4;		\
	pxor	r4, r2;

#define SBOX5_INVERSE(r0, r1, r2, r3, r4) \
	pxor	RNOT, r1;	movdqa	r3, r4;		\
	pxor	r1, r2;		por	r0, r3;		\
	pxor	r2, r3;		por	r1, r2;		\
	pand	r0, r2;		pxor	r3, r4;		\
	pxor	r4, r2;		por	r0, r4;		\
	pxor	r1, r4;		pand	r2, r1;		\
	pxor	r3, r1;		pxor	r2, r4;		\
	pand	r4, r3;		pxor	r1, r4;		\
	pxor	r4, r3;		pxor	RNOT, r4;	\
	pxor	r0, r3;

#define SBOX6(r0, r1, r2, r3, r4) \
	pxor	RNOT, r2;	movdqa	r3, r4;		\
	pand	r0, r3;		pxor	r4, r0;		\
	pxor	r2, r3;		por	r4, r2;		\
	pxor	r3, r1;		pxor	r0, r2;		\
	por	r1, r0;		pxor	r1, r2;		\
	pxor	r0, r4;		por	r3, r0;		\
	pxor	r2, r0;		pxor	r3, r4;		\
	pxor	r0, r4;		pxor	RNOT, r3;	\
	pand	r4, r2;	\
	pxor	r3, r2;

#define SBOX6_INVERSE(r0, r1, r2, r3, r4) \
	pxor	r2, r0;		movdqa	r2, r4;		\
	pand	r0, r2;		pxor	r3, r4;		\
	pxor	RNOT, r2;	pxor	r1, r3;		\
	pxor	r3, r2;		por	r0, r4;		\
	pxor	r2, r0;		pxor	r4, r3;		\
	pxor	r1, r4;		pand	r3, r1;		\
	pxor	r0, r1;		pxor	r3, r0;		\
	por	r2, r0;		pxor	r1, r3;		\
	pxor	r0, r4;

#define SBOX7(r0, r1, r2, r3, r4) \
	movdqa	r1, r4;		por	r2, r1;		\
	pxor	r3, r1;		pxor	r2, r4;		\
	pxor	r1, r2;		por	r4, r3;		\
	pand	r0, r3;		pxor	r2, r4;		\
	pxor	r1, r3;		por	r4, r1;		\
	pxor	r0, r1;		por	r4, r0;		\
	pxor	r2, r0;		pxor	r4, r1;		\
	pxor	r1, r2;		pand	r0, r1;		\
	pxor	r4, r1;		pxor	RNOT, r2;	\
	por	r0, r2;	\
	pxor	r2, r4;

#define SBOX7_INVERSE(r0, r1, r2, r3, r4) \
	movdqa	r2, r4;		pxor	r0, r2;		\
	pand	r3, r0;		por	r3, r4;		\
	pxor	RNOT, r2;	pxor	r1, r3;		\
	por	r0, r1;		pxor	r2, r0;		\
	pand	r4, r2;		pand	r4, r3;		\
	pxor	r2, r1;		pxor	r0, r2;		\
	por	r2, r0;		pxor	r1, r4;		\
	pxor	r3, r0;		pxor	r4, r3;		\
	por	r0, r4;		pxor	r2, r3;		\
	pxor	r2, r4;

/* Apply SBOX number WHICH to to the block.  */
#define SBOX(which, r0, r1, r2, r3, r4) \
	SBOX##which (r0, r1, r2, r3, r4)

/* Apply inverse SBOX number WHICH to to the block.  */
#define SBOX_INVERSE(which, r0, r1, r2, r3, r4) \
	SBOX##which##_INVERSE (r0, r1, r2, r3, r4)

/* XOR round key into block state in r0,r1,r2,r3. r4 used as temporary.  */
#define BLOCK_XOR_KEY(r0, r1, r2, r3, r4, round) \
	pbroadcastd ((ctx_keys + (round) * 16 + 0 * 4)(CTX), r4); \
	pxor r4, r0; \
	pbroadcastd ((ctx_keys + (round) * 16 + 1 * 4)(CTX), r4); \
	pxor r4, r1; \
	pbroadcastd ((ctx_keys + (round) * 16 + 2 * 4)(CTX), r4); \
	pxor r4, r2; \
	pbroadcastd ((ctx_keys + (round) * 16 + 3 * 4)(CTX), r4); \
	pxor r4, r3;

/* Apply the linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION(r0, r1, r2, r3, r4) \
	vec_rol(r0, 13, r4);	\
	vec_rol(r2, 3, r4);	\
	pxor r0, r1;		\
	pxor r2, r1;		\
	movdqa r0, r4;		\
	pslld $3, r4;		\
	pxor r2, r3;		\
	pxor r4, r3;		\
	vec_rol(r1, 1, r4);	\
	vec_rol(r3, 7, r4);	\
	pxor r1, r0;		\
	pxor r3, r0;		\
	movdqa r1, r4;		\
	pslld $7, r4;		\
	pxor r3, r2;		\
	pxor r4, r2;		\
	vec_rol(r0, 5, r4);	\
	vec_rol(r2, 22, r4);

/* Apply the inverse linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION_INVERSE(r0, r1, r2, r3, r4) \
	vec_ror(r2, 22, r4);	\
	vec_ror(r0, 5, r4);	\
	movdqa r1, r4;		\
	pslld $7, r4;		\
	pxor r3, r2;		\
	pxor r4, r2;		\
	pxor r1, r0;		\
	pxor r3, r0;		\
	vec_ror(r3, 7, r4);	\
	vec_ror(r1, 1, r4);	\
	movdqa r0, r4;		\
	pslld $3, r4;		\
	pxor r2, r3;		\
	pxor r4, r3;		\
	pxor r0, r1;		\
	pxor r2, r1;		\
	vec_ror(r2, 3, r4);	\
	vec_ror(r0, 13, r4);

/* Apply a Serpent round to eight parallel blocks.  This macro increments
   `round'.  */
#define ROUND(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
			    b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, round);		\
	SBOX (which, a0, a1, a2, a3, a4);			\
		BLOCK_XOR_KEY (b0, b1, b2, b3, b4, round);		\
		SBOX (which, b0, b1, b2, b3, b4);			\
	LINEAR_TRANSFORMATION (na0, na1, na2, na3, na4);	\
		LINEAR_TRANSFORMATION (nb0, nb1, nb2, nb3, nb4);

/* Apply the last Serpent round to eight parallel blocks.  This macro increments
   `round'.  */
#define ROUND_LAST(round, which, a0, a1, a2, a3, a4, na0, na1, na2, na3, na4, \
				 b0, b1, b2, b3, b4, nb0, nb1, nb2, nb3, nb4) \
	BLOCK_XOR_KEY (a0, a1, a2, a3, a4, round);		\
	SBOX (which, a0, a1, a2, a3, a4);			\
		BLOCK_XOR_KEY (b0, b1, b2, b3, b4, round);		\
		SBOX (which, b0, b1, b2, b3, b4);			\
	BLOCK_XOR_KEY (na0, na1, na2, na3, na4, ((round) + 1));		\
		BLOCK_XOR_KEY (nb0, nb1, nb2, nb3, nb4, ((round) + 1));

/* Apply an inverse Serpent round to eight parallel blocks.  This macro
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

/* Apply the first inverse Serpent round to eight parallel blocks.  This macro
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
ELF(.type   __serpent_enc_blk8,@function;)
__serpent_enc_blk8:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel plaintext
	 *						blocks
	 * output:
	 *	RA4, RA1, RA2, RA0, RB4, RB1, RB2, RB0: eight parallel
	 * 						ciphertext blocks
	 */
	CFI_STARTPROC();

	pcmpeqd RNOT, RNOT;

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
ELF(.size __serpent_enc_blk8,.-__serpent_enc_blk8;)

.align 8
ELF(.type   __serpent_dec_blk8,@function;)
__serpent_dec_blk8:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel
	 * 						ciphertext blocks
	 * output:
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel plaintext
	 *						blocks
	 */
	CFI_STARTPROC();

	pcmpeqd RNOT, RNOT;

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
ELF(.size __serpent_dec_blk8,.-__serpent_dec_blk8;)

.align 8
.globl _gcry_serpent_sse2_ctr_enc
ELF(.type   _gcry_serpent_sse2_ctr_enc,@function;)
_gcry_serpent_sse2_ctr_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();

	/* load IV and byteswap */
	movdqu (%rcx), RA0;
	movdqa RA0, RTMP0;
	pbswap(RTMP0, RTMP1); /* be => le */

	pcmpeqd RNOT, RNOT;
	psrldq $8, RNOT; /* low: -1, high: 0 */
	movdqa RNOT, RTMP2;
	paddq RTMP2, RTMP2; /* low: -2, high: 0 */

	/* construct IVs */
	movdqa RTMP0, RTMP1;
	psubq RNOT, RTMP0; /* +1 */
	movdqa RTMP0, RA1;
	psubq RTMP2, RTMP1; /* +2 */
	movdqa RTMP1, RA2;
	psubq RTMP2, RTMP0; /* +3 */
	movdqa RTMP0, RA3;
	psubq RTMP2, RTMP1; /* +4 */
	movdqa RTMP1, RB0;
	psubq RTMP2, RTMP0; /* +5 */
	movdqa RTMP0, RB1;
	psubq RTMP2, RTMP1; /* +6 */
	movdqa RTMP1, RB2;
	psubq RTMP2, RTMP0; /* +7 */
	movdqa RTMP0, RB3;
	psubq RTMP2, RTMP1; /* +8 */

	/* check need for handling 64-bit overflow and carry */
	cmpl $0xffffffff, 8(%rcx);
	jne .Lno_ctr_carry;

	movl 12(%rcx), %eax;
	bswapl %eax;
	cmpl $-8, %eax;
	jb .Lno_ctr_carry;
	pslldq $8, RNOT; /* low: 0, high: -1 */
	je .Lcarry_RTMP0;

	cmpl $-6, %eax;
	jb .Lcarry_RB3;
	je .Lcarry_RB2;

	cmpl $-4, %eax;
	jb .Lcarry_RB1;
	je .Lcarry_RB0;

	cmpl $-2, %eax;
	jb .Lcarry_RA3;
	je .Lcarry_RA2;

	psubq RNOT, RA1;
.Lcarry_RA2:
	psubq RNOT, RA2;
.Lcarry_RA3:
	psubq RNOT, RA3;
.Lcarry_RB0:
	psubq RNOT, RB0;
.Lcarry_RB1:
	psubq RNOT, RB1;
.Lcarry_RB2:
	psubq RNOT, RB2;
.Lcarry_RB3:
	psubq RNOT, RB3;
.Lcarry_RTMP0:
	psubq RNOT, RTMP1;

.Lno_ctr_carry:
	/* le => be */
	pbswap(RA1, RTMP0);
	pbswap(RA2, RTMP0);
	pbswap(RA3, RTMP0);
	pbswap(RB0, RTMP0);
	pbswap(RB1, RTMP0);
	pbswap(RB2, RTMP0);
	pbswap(RB3, RTMP0);
	pbswap(RTMP1, RTMP0);
	/* store new IV */
	movdqu RTMP1, (%rcx);

	call __serpent_enc_blk8;

	pxor_u((0 * 16)(%rdx), RA4, RTMP0);
	pxor_u((1 * 16)(%rdx), RA1, RTMP0);
	pxor_u((2 * 16)(%rdx), RA2, RTMP0);
	pxor_u((3 * 16)(%rdx), RA0, RTMP0);
	pxor_u((4 * 16)(%rdx), RB4, RTMP0);
	pxor_u((5 * 16)(%rdx), RB1, RTMP0);
	pxor_u((6 * 16)(%rdx), RB2, RTMP0);
	pxor_u((7 * 16)(%rdx), RB0, RTMP0);

	movdqu RA4, (0 * 16)(%rsi);
	movdqu RA1, (1 * 16)(%rsi);
	movdqu RA2, (2 * 16)(%rsi);
	movdqu RA0, (3 * 16)(%rsi);
	movdqu RB4, (4 * 16)(%rsi);
	movdqu RB1, (5 * 16)(%rsi);
	movdqu RB2, (6 * 16)(%rsi);
	movdqu RB0, (7 * 16)(%rsi);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_ctr_enc,.-_gcry_serpent_sse2_ctr_enc;)

.align 8
.globl _gcry_serpent_sse2_cbc_dec
ELF(.type   _gcry_serpent_sse2_cbc_dec,@function;)
_gcry_serpent_sse2_cbc_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	movdqu (0 * 16)(%rdx), RA0;
	movdqu (1 * 16)(%rdx), RA1;
	movdqu (2 * 16)(%rdx), RA2;
	movdqu (3 * 16)(%rdx), RA3;
	movdqu (4 * 16)(%rdx), RB0;
	movdqu (5 * 16)(%rdx), RB1;
	movdqu (6 * 16)(%rdx), RB2;
	movdqu (7 * 16)(%rdx), RB3;

	call __serpent_dec_blk8;

	movdqu (7 * 16)(%rdx), RNOT;
	pxor_u((%rcx), RA0, RTMP0);
	pxor_u((0 * 16)(%rdx), RA1, RTMP0);
	pxor_u((1 * 16)(%rdx), RA2, RTMP0);
	pxor_u((2 * 16)(%rdx), RA3, RTMP0);
	pxor_u((3 * 16)(%rdx), RB0, RTMP0);
	pxor_u((4 * 16)(%rdx), RB1, RTMP0);
	pxor_u((5 * 16)(%rdx), RB2, RTMP0);
	pxor_u((6 * 16)(%rdx), RB3, RTMP0);
	movdqu RNOT, (%rcx); /* store new IV */

	movdqu RA0, (0 * 16)(%rsi);
	movdqu RA1, (1 * 16)(%rsi);
	movdqu RA2, (2 * 16)(%rsi);
	movdqu RA3, (3 * 16)(%rsi);
	movdqu RB0, (4 * 16)(%rsi);
	movdqu RB1, (5 * 16)(%rsi);
	movdqu RB2, (6 * 16)(%rsi);
	movdqu RB3, (7 * 16)(%rsi);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_cbc_dec,.-_gcry_serpent_sse2_cbc_dec;)

.align 8
.globl _gcry_serpent_sse2_cfb_dec
ELF(.type   _gcry_serpent_sse2_cfb_dec,@function;)
_gcry_serpent_sse2_cfb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	/* Load input */
	movdqu (%rcx), RA0;
	movdqu 0 * 16(%rdx), RA1;
	movdqu 1 * 16(%rdx), RA2;
	movdqu 2 * 16(%rdx), RA3;
	movdqu 3 * 16(%rdx), RB0;
	movdqu 4 * 16(%rdx), RB1;
	movdqu 5 * 16(%rdx), RB2;
	movdqu 6 * 16(%rdx), RB3;

	/* Update IV */
	movdqu 7 * 16(%rdx), RNOT;
	movdqu RNOT, (%rcx);

	call __serpent_enc_blk8;

	pxor_u((0 * 16)(%rdx), RA4, RTMP0);
	pxor_u((1 * 16)(%rdx), RA1, RTMP0);
	pxor_u((2 * 16)(%rdx), RA2, RTMP0);
	pxor_u((3 * 16)(%rdx), RA0, RTMP0);
	pxor_u((4 * 16)(%rdx), RB4, RTMP0);
	pxor_u((5 * 16)(%rdx), RB1, RTMP0);
	pxor_u((6 * 16)(%rdx), RB2, RTMP0);
	pxor_u((7 * 16)(%rdx), RB0, RTMP0);

	movdqu RA4, (0 * 16)(%rsi);
	movdqu RA1, (1 * 16)(%rsi);
	movdqu RA2, (2 * 16)(%rsi);
	movdqu RA0, (3 * 16)(%rsi);
	movdqu RB4, (4 * 16)(%rsi);
	movdqu RB1, (5 * 16)(%rsi);
	movdqu RB2, (6 * 16)(%rsi);
	movdqu RB0, (7 * 16)(%rsi);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_cfb_dec,.-_gcry_serpent_sse2_cfb_dec;)

.align 8
.globl _gcry_serpent_sse2_ocb_enc
ELF(.type _gcry_serpent_sse2_ocb_enc,@function;)

_gcry_serpent_sse2_ocb_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[8])
	 */
	CFI_STARTPROC();

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

	movdqu (%rcx), RTMP0;
	movdqu (%r8), RTMP1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  movdqu (n * 16)(%rdx), xreg; \
	  movdqu (lreg), RNOT; \
	  pxor RNOT, RTMP0; \
	  pxor xreg, RTMP1; \
	  pxor RTMP0, xreg; \
	  movdqu RTMP0, (n * 16)(%rsi);
	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, RA0);
	OCB_INPUT(1, %r11, RA1);
	OCB_INPUT(2, %r12, RA2);
	OCB_INPUT(3, %r13, RA3);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, RB0);
	OCB_INPUT(5, %r11, RB1);
	OCB_INPUT(6, %r12, RB2);
	OCB_INPUT(7, %r13, RB3);
#undef OCB_INPUT

	movdqu RTMP0, (%rcx);
	movdqu RTMP1, (%r8);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_enc_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	pxor_u((0 * 16)(%rsi), RA4, RTMP0);
	pxor_u((1 * 16)(%rsi), RA1, RTMP0);
	pxor_u((2 * 16)(%rsi), RA2, RTMP0);
	pxor_u((3 * 16)(%rsi), RA0, RTMP0);
	pxor_u((4 * 16)(%rsi), RB4, RTMP0);
	pxor_u((5 * 16)(%rsi), RB1, RTMP0);
	pxor_u((6 * 16)(%rsi), RB2, RTMP0);
	pxor_u((7 * 16)(%rsi), RB0, RTMP0);

	movdqu RA4, (0 * 16)(%rsi);
	movdqu RA1, (1 * 16)(%rsi);
	movdqu RA2, (2 * 16)(%rsi);
	movdqu RA0, (3 * 16)(%rsi);
	movdqu RB4, (4 * 16)(%rsi);
	movdqu RB1, (5 * 16)(%rsi);
	movdqu RB2, (6 * 16)(%rsi);
	movdqu RB0, (7 * 16)(%rsi);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_ocb_enc,.-_gcry_serpent_sse2_ocb_enc;)

.align 8
.globl _gcry_serpent_sse2_ocb_dec
ELF(.type _gcry_serpent_sse2_ocb_dec,@function;)

_gcry_serpent_sse2_ocb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[8])
	 */
	CFI_STARTPROC();

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

	movdqu (%rcx), RTMP0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  movdqu (n * 16)(%rdx), xreg; \
	  movdqu (lreg), RNOT; \
	  pxor RNOT, RTMP0; \
	  pxor RTMP0, xreg; \
	  movdqu RTMP0, (n * 16)(%rsi);
	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, RA0);
	OCB_INPUT(1, %r11, RA1);
	OCB_INPUT(2, %r12, RA2);
	OCB_INPUT(3, %r13, RA3);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, RB0);
	OCB_INPUT(5, %r11, RB1);
	OCB_INPUT(6, %r12, RB2);
	OCB_INPUT(7, %r13, RB3);
#undef OCB_INPUT

	movdqu RTMP0, (%rcx);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_dec_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	movdqu (%r8), RTMP0;

	pxor_u((0 * 16)(%rsi), RA0, RTMP1);
	pxor_u((1 * 16)(%rsi), RA1, RTMP1);
	pxor_u((2 * 16)(%rsi), RA2, RTMP1);
	pxor_u((3 * 16)(%rsi), RA3, RTMP1);
	pxor_u((4 * 16)(%rsi), RB0, RTMP1);
	pxor_u((5 * 16)(%rsi), RB1, RTMP1);
	pxor_u((6 * 16)(%rsi), RB2, RTMP1);
	pxor_u((7 * 16)(%rsi), RB3, RTMP1);

	/* Checksum_i = Checksum_{i-1} xor P_i  */

	movdqu RA0, (0 * 16)(%rsi);
	pxor RA0, RTMP0;
	movdqu RA1, (1 * 16)(%rsi);
	pxor RA1, RTMP0;
	movdqu RA2, (2 * 16)(%rsi);
	pxor RA2, RTMP0;
	movdqu RA3, (3 * 16)(%rsi);
	pxor RA3, RTMP0;
	movdqu RB0, (4 * 16)(%rsi);
	pxor RB0, RTMP0;
	movdqu RB1, (5 * 16)(%rsi);
	pxor RB1, RTMP0;
	movdqu RB2, (6 * 16)(%rsi);
	pxor RB2, RTMP0;
	movdqu RB3, (7 * 16)(%rsi);
	pxor RB3, RTMP0;

	movdqu RTMP0, (%r8);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_ocb_dec,.-_gcry_serpent_sse2_ocb_dec;)

.align 8
.globl _gcry_serpent_sse2_ocb_auth
ELF(.type _gcry_serpent_sse2_ocb_auth,@function;)

_gcry_serpent_sse2_ocb_auth:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: abuf (8 blocks)
	 *	%rdx: offset
	 *	%rcx: checksum
	 *	%r8 : L pointers (void *L[8])
	 */
	CFI_STARTPROC();

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

	movdqu (%rdx), RTMP0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  movdqu (n * 16)(%rsi), xreg; \
	  movdqu (lreg), RNOT; \
	  pxor RNOT, RTMP0; \
	  pxor RTMP0, xreg;
	movq (0 * 8)(%r8), %r10;
	movq (1 * 8)(%r8), %r11;
	movq (2 * 8)(%r8), %r12;
	movq (3 * 8)(%r8), %r13;
	OCB_INPUT(0, %r10, RA0);
	OCB_INPUT(1, %r11, RA1);
	OCB_INPUT(2, %r12, RA2);
	OCB_INPUT(3, %r13, RA3);
	movq (4 * 8)(%r8), %r10;
	movq (5 * 8)(%r8), %r11;
	movq (6 * 8)(%r8), %r12;
	movq (7 * 8)(%r8), %r13;
	OCB_INPUT(4, %r10, RB0);
	OCB_INPUT(5, %r11, RB1);
	OCB_INPUT(6, %r12, RB2);
	OCB_INPUT(7, %r13, RB3);
#undef OCB_INPUT

	movdqu RTMP0, (%rdx);

	movq (0 * 8)(%rsp), %r10;
	movq (1 * 8)(%rsp), %r11;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	call __serpent_enc_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	movdqu (%rcx), RTMP0;
	pxor RB4, RA4;
	pxor RB1, RA1;
	pxor RB2, RA2;
	pxor RB0, RA0;

	pxor RTMP0, RA2;
	pxor RA4, RA1;
	pxor RA2, RA0;

	pxor RA1, RA0;
	movdqu RA0, (%rcx);

	/* clear the used registers */
	pxor RA0, RA0;
	pxor RA1, RA1;
	pxor RA2, RA2;
	pxor RA3, RA3;
	pxor RA4, RA4;
	pxor RB0, RB0;
	pxor RB1, RB1;
	pxor RB2, RB2;
	pxor RB3, RB3;
	pxor RB4, RB4;
	pxor RTMP0, RTMP0;
	pxor RTMP1, RTMP1;
	pxor RTMP2, RTMP2;
	pxor RNOT, RNOT;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_serpent_sse2_ocb_auth,.-_gcry_serpent_sse2_ocb_auth;)

#endif /*defined(USE_SERPENT)*/
#endif /*__x86_64*/
