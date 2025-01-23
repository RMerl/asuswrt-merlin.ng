/* sha1-avx2-bmi2-amd64.S - Intel AVX2/BMI2 accelerated SHA-1 transform function
 * Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * Based on sha1.c:
 *  Copyright (C) 1998, 2001, 2002, 2003, 2008 Free Software Foundation, Inc.
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

/*
 * Intel SSSE3 accelerated SHA-1 implementation based on white paper:
 *  "Improving the Performance of the Secure Hash Algorithm (SHA-1)"
 *  http://software.intel.com/en-us/articles/improving-the-performance-of-the-secure-hash-algorithm-1
 */

#ifdef __x86_64__
#include <config.h>

#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
     defined(HAVE_GCC_INLINE_ASM_BMI2) && defined(HAVE_GCC_INLINE_ASM_AVX) && \
     defined(HAVE_GCC_INLINE_ASM_AVX2) && defined(USE_SHA1)

#include "asm-common-amd64.h"


/* Context structure */

#define state_h0 0
#define state_h1 4
#define state_h2 8
#define state_h3 12
#define state_h4 16


/* Constants */

#define WK_STACK_WORDS (80 * 2)

.text
.align 16
.Lbswap_shufb_ctl:
	.long 0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f

.LK1:	.long 0x5A827999
.LK2:	.long 0x6ED9EBA1
.LK3:	.long 0x8F1BBCDC
.LK4:	.long 0xCA62C1D6


/* Register macros */

#define RSTATE %r8
#define RDATA %r9
#define ROLDSTACK %r10
#define RNBLKS %r11

#define a %eax
#define b %ebx
#define c %ecx
#define d %edx
#define e %edi
#define ne %r12d

#define RT0 %esi
#define RT1 %ebp

#define Wtmp0 %ymm0
#define Wtmp1 %ymm1
#define Wtmp0x %xmm0
#define Wtmp1x %xmm1

#define W0 %ymm2
#define W1 %ymm3
#define W2 %ymm4
#define W3 %ymm5
#define W4 %ymm6
#define W5 %ymm7
#define W6 %ymm8
#define W7 %ymm9

#define BSWAP_REG %ymm10

#define K1 %ymm11
#define K2 %ymm12
#define K3 %ymm13
#define K4 %ymm14


/* Round function macros. */

#define WK(i,block) ((block) * 16 + ((i) / 4) * 32 + ((i) % 4) * 4)(%rsp)
#define PRE_WK(i) ((i) * 4 * 2)(%rsp)

#define R_F1(a,b,c,d,e,i,block) \
	movl c, RT0; \
	andn d, b, RT1; \
	addl WK(i,block), e; \
	andl b, RT0; \
	leal (a,ne), a; \
	rorxl $2, b, b; \
	addl RT1, e; \
	rorxl $27, a, ne; \
	addl RT0, e;

#define R_F2(a,b,c,d,e,i,block) \
	addl WK(i,block), e; \
	movl c, RT0; \
	xorl b, RT0; \
	leal (a,ne), a; \
	rorxl $2, b, b; \
	xorl d, RT0; \
	addl RT0, e; \
	rorxl $27, a, ne;

#define R_F3(a,b,c,d,e,i,block) \
	movl c, RT0; \
	addl WK(i,block), e; \
	movl b, RT1; \
	xorl b, RT0; \
	leal (a,ne), a; \
	rorxl $2, b, b; \
	andl c, RT1; \
	addl RT1, e; \
	andl d, RT0; \
	rorxl $27, a, ne; \
	addl RT0, e;

#define R_F4(a,b,c,d,e,i,block) R_F2(a,b,c,d,e,i,block)

#define R(a,b,c,d,e,f,i,block) \
	R_##f(a,b,c,d,e,i,block)


/* Input expansion macros. */

#define W_PRECALC_00_15_0(i, W, tmp0) \
	vmovdqu (4*(i))(RDATA), tmp0##x; \
	vinserti128 $1, (4*(i) + 64)(RDATA), tmp0, tmp0;

#define W_PRECALC_00_15_1(i, W, tmp0) \
	vpshufb BSWAP_REG, tmp0, W;

#define W_PRECALC_00_15_2(i, W, tmp0, K) \
	vpaddd K, W, tmp0;

#define W_PRECALC_00_15_3(i, W, tmp0) \
	vmovdqa tmp0, PRE_WK((i)&~3);

#define W_PRECALC_16_31_0(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	vpalignr $8, W_m16, W_m12, W; \
	vpsrldq $4, W_m04, tmp0; \
	vpxor W_m08, W, W;

#define W_PRECALC_16_31_1(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	vpxor W_m16, tmp0, tmp0; \
	vpxor tmp0, W, W; \
	vpslld $1, W, tmp0; \
	vpslldq $12, W, tmp1; \
	vpsrld $31, W, W;

#define W_PRECALC_16_31_2(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	vpor W, tmp0, tmp0; \
	vpsrld $30, tmp1, W; \
	vpslld $2, tmp1, tmp1;

#define W_PRECALC_16_31_3(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1, K) \
	vpxor W, tmp0, tmp0; \
	vpxor tmp1, tmp0, W; \
	vpaddd K, W, tmp0; \
	vmovdqa tmp0, PRE_WK((i)&~3);

#define W_PRECALC_32_79_0(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	vpxor W_m28, W, W; \
	vpalignr $8, W_m08, W_m04, tmp0;

#define W_PRECALC_32_79_1(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	vpxor W_m16, W, W; \
	vpxor tmp0, W, W;

#define W_PRECALC_32_79_2(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	vpsrld $30, W, tmp0; \
	vpslld $2, W, W;

#define W_PRECALC_32_79_3(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0, K) \
	vpor W, tmp0, W; \
	vpaddd K, W, tmp0; \
	vmovdqa tmp0, PRE_WK((i)&~3);


/*
 * Transform 2*nblks*64 bytes (2*nblks*16 32-bit words) at DATA.
 *
 * unsigned int
 * _gcry_sha1_transform_amd64_avx2_bmi2 (void *ctx, const unsigned char *data,
 *                                       size_t nblks)
 */
.globl _gcry_sha1_transform_amd64_avx2_bmi2
ELF(.type _gcry_sha1_transform_amd64_avx2_bmi2,@function)
.align 16
_gcry_sha1_transform_amd64_avx2_bmi2:
  /* input:
   *	%rdi: ctx, CTX
   *	%rsi: data (64*nblks bytes)
   *	%rdx: nblks (multiple of 2, larger than 0)
   */
  CFI_STARTPROC();

  vzeroupper;

  movq %rdx, RNBLKS;
  movq %rdi, RSTATE;
  movq %rsi, RDATA;
  pushq %rbx;
  CFI_PUSH(%rbx);
  pushq %rbp;
  CFI_PUSH(%rbp);
  pushq %r12;
  CFI_PUSH(%r12);

  movq %rsp, ROLDSTACK;
  CFI_DEF_CFA_REGISTER(ROLDSTACK);

  subq $(WK_STACK_WORDS*4), %rsp;
  andq $(~63), %rsp;

  /* Get the values of the chaining variables. */
  movl state_h0(RSTATE), a;
  movl state_h1(RSTATE), b;
  movl state_h2(RSTATE), c;
  movl state_h3(RSTATE), d;
  movl state_h4(RSTATE), e;
  xorl ne, ne;

  vbroadcasti128 .Lbswap_shufb_ctl rRIP, BSWAP_REG;
  vpbroadcastd .LK1 rRIP, K1;
  vpbroadcastd .LK2 rRIP, K2;
  vpbroadcastd .LK3 rRIP, K3;
  vpbroadcastd .LK4 rRIP, K4;

  /* Precalc 0-31 for block 1 & 2. */
  W_PRECALC_00_15_0(0, W0, Wtmp0);
  W_PRECALC_00_15_1(1, W0, Wtmp0);
  W_PRECALC_00_15_2(2, W0, Wtmp0, K1);
  W_PRECALC_00_15_3(3, W0, Wtmp0);
  W_PRECALC_00_15_0(4, W7, Wtmp0);
  W_PRECALC_00_15_1(5, W7, Wtmp0);
  W_PRECALC_00_15_2(6, W7, Wtmp0, K1);
  W_PRECALC_00_15_3(7, W7, Wtmp0);
  W_PRECALC_00_15_0(8, W6, Wtmp0);
  W_PRECALC_00_15_1(9, W6, Wtmp0);
  W_PRECALC_00_15_2(10, W6, Wtmp0, K1);
  W_PRECALC_00_15_3(11, W6, Wtmp0);
  W_PRECALC_00_15_0(12, W5, Wtmp0);
  W_PRECALC_00_15_1(13, W5, Wtmp0);
  W_PRECALC_00_15_2(14, W5, Wtmp0, K1);
  W_PRECALC_00_15_3(15, W5, Wtmp0);
  W_PRECALC_16_31_0(16, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  W_PRECALC_16_31_1(17, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  W_PRECALC_16_31_2(18, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  W_PRECALC_16_31_3(19, W4, W5, W6, W7, W0, Wtmp0, Wtmp1, K1);
  W_PRECALC_16_31_0(20, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  W_PRECALC_16_31_1(21, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  W_PRECALC_16_31_2(22, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  W_PRECALC_16_31_3(23, W3, W4, W5, W6, W7, Wtmp0, Wtmp1, K2);
  W_PRECALC_16_31_0(24, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  W_PRECALC_16_31_1(25, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  W_PRECALC_16_31_2(26, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  W_PRECALC_16_31_3(27, W2, W3, W4, W5, W6, Wtmp0, Wtmp1, K2);
  W_PRECALC_16_31_0(28, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  W_PRECALC_16_31_1(29, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  W_PRECALC_16_31_2(30, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  W_PRECALC_16_31_3(31, W1, W2, W3, W4, W5, Wtmp0, Wtmp1, K2);

.align 8
.Loop:
  addq $(2 * 64), RDATA;

  /* Transform 0-15 for block 1 + Precalc 32-47 for block 1 & 2. */
  R( a, b, c, d, e, F1,  0, 0 ); W_PRECALC_32_79_0(32, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( e, a, b, c, d, F1,  1, 0 ); W_PRECALC_32_79_1(33, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( d, e, a, b, c, F1,  2, 0 ); W_PRECALC_32_79_2(34, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( c, d, e, a, b, F1,  3, 0 ); W_PRECALC_32_79_3(35, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0, K2);
  R( b, c, d, e, a, F1,  4, 0 ); W_PRECALC_32_79_0(36, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( a, b, c, d, e, F1,  5, 0 ); W_PRECALC_32_79_1(37, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( e, a, b, c, d, F1,  6, 0 ); W_PRECALC_32_79_2(38, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( d, e, a, b, c, F1,  7, 0 ); W_PRECALC_32_79_3(39, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0, K2);
  R( c, d, e, a, b, F1,  8, 0 ); W_PRECALC_32_79_0(40, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( b, c, d, e, a, F1,  9, 0 ); W_PRECALC_32_79_1(41, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( a, b, c, d, e, F1, 10, 0 ); W_PRECALC_32_79_2(42, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( e, a, b, c, d, F1, 11, 0 ); W_PRECALC_32_79_3(43, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0, K3);
  R( d, e, a, b, c, F1, 12, 0 ); W_PRECALC_32_79_0(44, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( c, d, e, a, b, F1, 13, 0 ); W_PRECALC_32_79_1(45, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( b, c, d, e, a, F1, 14, 0 ); W_PRECALC_32_79_2(46, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( a, b, c, d, e, F1, 15, 0 ); W_PRECALC_32_79_3(47, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0, K3);

  /* Transform 16-47 for block 1 + Precalc 48-79 for block 1 & 2. */
  R( e, a, b, c, d, F1, 16, 0 ); W_PRECALC_32_79_0(48, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( d, e, a, b, c, F1, 17, 0 ); W_PRECALC_32_79_1(49, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( c, d, e, a, b, F1, 18, 0 ); W_PRECALC_32_79_2(50, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( b, c, d, e, a, F1, 19, 0 ); W_PRECALC_32_79_3(51, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0, K3);
  R( a, b, c, d, e, F2, 20, 0 ); W_PRECALC_32_79_0(52, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( e, a, b, c, d, F2, 21, 0 ); W_PRECALC_32_79_1(53, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( d, e, a, b, c, F2, 22, 0 ); W_PRECALC_32_79_2(54, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( c, d, e, a, b, F2, 23, 0 ); W_PRECALC_32_79_3(55, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0, K3);
  R( b, c, d, e, a, F2, 24, 0 ); W_PRECALC_32_79_0(56, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( a, b, c, d, e, F2, 25, 0 ); W_PRECALC_32_79_1(57, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( e, a, b, c, d, F2, 26, 0 ); W_PRECALC_32_79_2(58, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( d, e, a, b, c, F2, 27, 0 ); W_PRECALC_32_79_3(59, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0, K3);
  R( c, d, e, a, b, F2, 28, 0 ); W_PRECALC_32_79_0(60, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( b, c, d, e, a, F2, 29, 0 ); W_PRECALC_32_79_1(61, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( a, b, c, d, e, F2, 30, 0 ); W_PRECALC_32_79_2(62, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( e, a, b, c, d, F2, 31, 0 ); W_PRECALC_32_79_3(63, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0, K4);
  R( d, e, a, b, c, F2, 32, 0 ); W_PRECALC_32_79_0(64, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( c, d, e, a, b, F2, 33, 0 ); W_PRECALC_32_79_1(65, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( b, c, d, e, a, F2, 34, 0 ); W_PRECALC_32_79_2(66, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( a, b, c, d, e, F2, 35, 0 ); W_PRECALC_32_79_3(67, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0, K4);
  R( e, a, b, c, d, F2, 36, 0 ); W_PRECALC_32_79_0(68, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( d, e, a, b, c, F2, 37, 0 ); W_PRECALC_32_79_1(69, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( c, d, e, a, b, F2, 38, 0 ); W_PRECALC_32_79_2(70, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( b, c, d, e, a, F2, 39, 0 ); W_PRECALC_32_79_3(71, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0, K4);
  R( a, b, c, d, e, F3, 40, 0 ); W_PRECALC_32_79_0(72, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( e, a, b, c, d, F3, 41, 0 ); W_PRECALC_32_79_1(73, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( d, e, a, b, c, F3, 42, 0 ); W_PRECALC_32_79_2(74, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( c, d, e, a, b, F3, 43, 0 ); W_PRECALC_32_79_3(75, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0, K4);
  R( b, c, d, e, a, F3, 44, 0 ); W_PRECALC_32_79_0(76, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( a, b, c, d, e, F3, 45, 0 ); W_PRECALC_32_79_1(77, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( e, a, b, c, d, F3, 46, 0 ); W_PRECALC_32_79_2(78, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( d, e, a, b, c, F3, 47, 0 ); W_PRECALC_32_79_3(79, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0, K4);

  /* Transform 48-79 for block 1. */
  R( c, d, e, a, b, F3, 48, 0 );
  R( b, c, d, e, a, F3, 49, 0 );
  R( a, b, c, d, e, F3, 50, 0 );
  R( e, a, b, c, d, F3, 51, 0 );
  R( d, e, a, b, c, F3, 52, 0 );
  R( c, d, e, a, b, F3, 53, 0 );
  R( b, c, d, e, a, F3, 54, 0 );
  R( a, b, c, d, e, F3, 55, 0 );
  R( e, a, b, c, d, F3, 56, 0 );
  R( d, e, a, b, c, F3, 57, 0 );
  R( c, d, e, a, b, F3, 58, 0 );
  R( b, c, d, e, a, F3, 59, 0 );
  R( a, b, c, d, e, F4, 60, 0 );
  R( e, a, b, c, d, F4, 61, 0 );
  R( d, e, a, b, c, F4, 62, 0 );
  R( c, d, e, a, b, F4, 63, 0 );
  R( b, c, d, e, a, F4, 64, 0 );
  R( a, b, c, d, e, F4, 65, 0 );
  R( e, a, b, c, d, F4, 66, 0 );
  R( d, e, a, b, c, F4, 67, 0 );
  R( c, d, e, a, b, F4, 68, 0 );
  R( b, c, d, e, a, F4, 69, 0 );
  R( a, b, c, d, e, F4, 70, 0 );
  R( e, a, b, c, d, F4, 71, 0 );
  R( d, e, a, b, c, F4, 72, 0 );
  R( c, d, e, a, b, F4, 73, 0 );
  R( b, c, d, e, a, F4, 74, 0 );
  R( a, b, c, d, e, F4, 75, 0 );
  R( e, a, b, c, d, F4, 76, 0 );
  R( d, e, a, b, c, F4, 77, 0 );
  R( c, d, e, a, b, F4, 78, 0 );
  addl state_h0(RSTATE), a;
  R( b, c, d, e, a, F4, 79, 0 );
  addl ne, a;
  xorl ne, ne;

  /* Update the chaining variables. */
  addl state_h3(RSTATE), d;
  addl state_h2(RSTATE), c;
  addl state_h1(RSTATE), b;
  addl state_h4(RSTATE), e;

  movl d, state_h3(RSTATE);
  movl c, state_h2(RSTATE);
  movl b, state_h1(RSTATE);
  movl a, state_h0(RSTATE);
  movl e, state_h4(RSTATE);

  /* Transform 0-47 for block 2. */
  R( a, b, c, d, e, F1,  0, 1 );
  R( e, a, b, c, d, F1,  1, 1 );
  R( d, e, a, b, c, F1,  2, 1 );
  R( c, d, e, a, b, F1,  3, 1 );
  R( b, c, d, e, a, F1,  4, 1 );
  R( a, b, c, d, e, F1,  5, 1 );
  R( e, a, b, c, d, F1,  6, 1 );
  R( d, e, a, b, c, F1,  7, 1 );
  R( c, d, e, a, b, F1,  8, 1 );
  R( b, c, d, e, a, F1,  9, 1 );
  R( a, b, c, d, e, F1, 10, 1 );
  R( e, a, b, c, d, F1, 11, 1 );
  R( d, e, a, b, c, F1, 12, 1 );
  R( c, d, e, a, b, F1, 13, 1 );
  R( b, c, d, e, a, F1, 14, 1 );
  R( a, b, c, d, e, F1, 15, 1 );
  R( e, a, b, c, d, F1, 16, 1 );
  R( d, e, a, b, c, F1, 17, 1 );
  R( c, d, e, a, b, F1, 18, 1 );
  R( b, c, d, e, a, F1, 19, 1 );
  R( a, b, c, d, e, F2, 20, 1 );
  R( e, a, b, c, d, F2, 21, 1 );
  R( d, e, a, b, c, F2, 22, 1 );
  R( c, d, e, a, b, F2, 23, 1 );
  R( b, c, d, e, a, F2, 24, 1 );
  R( a, b, c, d, e, F2, 25, 1 );
  R( e, a, b, c, d, F2, 26, 1 );
  R( d, e, a, b, c, F2, 27, 1 );
  R( c, d, e, a, b, F2, 28, 1 );
  R( b, c, d, e, a, F2, 29, 1 );
  R( a, b, c, d, e, F2, 30, 1 );
  R( e, a, b, c, d, F2, 31, 1 );
  R( d, e, a, b, c, F2, 32, 1 );
  R( c, d, e, a, b, F2, 33, 1 );
  R( b, c, d, e, a, F2, 34, 1 );
  R( a, b, c, d, e, F2, 35, 1 );
  R( e, a, b, c, d, F2, 36, 1 );
  R( d, e, a, b, c, F2, 37, 1 );
  R( c, d, e, a, b, F2, 38, 1 );
  R( b, c, d, e, a, F2, 39, 1 );
  R( a, b, c, d, e, F3, 40, 1 );
  R( e, a, b, c, d, F3, 41, 1 );
  R( d, e, a, b, c, F3, 42, 1 );
  R( c, d, e, a, b, F3, 43, 1 );
  R( b, c, d, e, a, F3, 44, 1 );
  R( a, b, c, d, e, F3, 45, 1 );
  R( e, a, b, c, d, F3, 46, 1 );
  R( d, e, a, b, c, F3, 47, 1 );

  addq $-2, RNBLKS;
  jz .Lend;

  /* Transform 48-79 for block 2 + Precalc 0-31 for next two blocks. */
  R( c, d, e, a, b, F3, 48, 1 ); W_PRECALC_00_15_0(0, W0, Wtmp0);
  R( b, c, d, e, a, F3, 49, 1 ); W_PRECALC_00_15_1(1, W0, Wtmp0);
  R( a, b, c, d, e, F3, 50, 1 ); W_PRECALC_00_15_2(2, W0, Wtmp0, K1);
  R( e, a, b, c, d, F3, 51, 1 ); W_PRECALC_00_15_3(3, W0, Wtmp0);
  R( d, e, a, b, c, F3, 52, 1 ); W_PRECALC_00_15_0(4, W7, Wtmp0);
  R( c, d, e, a, b, F3, 53, 1 ); W_PRECALC_00_15_1(5, W7, Wtmp0);
  R( b, c, d, e, a, F3, 54, 1 ); W_PRECALC_00_15_2(6, W7, Wtmp0, K1);
  R( a, b, c, d, e, F3, 55, 1 ); W_PRECALC_00_15_3(7, W7, Wtmp0);
  R( e, a, b, c, d, F3, 56, 1 ); W_PRECALC_00_15_0(8, W6, Wtmp0);
  R( d, e, a, b, c, F3, 57, 1 ); W_PRECALC_00_15_1(9, W6, Wtmp0);
  R( c, d, e, a, b, F3, 58, 1 ); W_PRECALC_00_15_2(10, W6, Wtmp0, K1);
  R( b, c, d, e, a, F3, 59, 1 ); W_PRECALC_00_15_3(11, W6, Wtmp0);
  R( a, b, c, d, e, F4, 60, 1 ); W_PRECALC_00_15_0(12, W5, Wtmp0);
  R( e, a, b, c, d, F4, 61, 1 ); W_PRECALC_00_15_1(13, W5, Wtmp0);
  R( d, e, a, b, c, F4, 62, 1 ); W_PRECALC_00_15_2(14, W5, Wtmp0, K1);
  R( c, d, e, a, b, F4, 63, 1 ); W_PRECALC_00_15_3(15, W5, Wtmp0);
  R( b, c, d, e, a, F4, 64, 1 ); W_PRECALC_16_31_0(16, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F4, 65, 1 ); W_PRECALC_16_31_1(17, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( e, a, b, c, d, F4, 66, 1 ); W_PRECALC_16_31_2(18, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( d, e, a, b, c, F4, 67, 1 ); W_PRECALC_16_31_3(19, W4, W5, W6, W7, W0, Wtmp0, Wtmp1, K1);
  R( c, d, e, a, b, F4, 68, 1 ); W_PRECALC_16_31_0(20, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( b, c, d, e, a, F4, 69, 1 ); W_PRECALC_16_31_1(21, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F4, 70, 1 ); W_PRECALC_16_31_2(22, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( e, a, b, c, d, F4, 71, 1 ); W_PRECALC_16_31_3(23, W3, W4, W5, W6, W7, Wtmp0, Wtmp1, K2);
  R( d, e, a, b, c, F4, 72, 1 ); W_PRECALC_16_31_0(24, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( c, d, e, a, b, F4, 73, 1 ); W_PRECALC_16_31_1(25, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( b, c, d, e, a, F4, 74, 1 ); W_PRECALC_16_31_2(26, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F4, 75, 1 ); W_PRECALC_16_31_3(27, W2, W3, W4, W5, W6, Wtmp0, Wtmp1, K2);
  R( e, a, b, c, d, F4, 76, 1 ); W_PRECALC_16_31_0(28, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  R( d, e, a, b, c, F4, 77, 1 ); W_PRECALC_16_31_1(29, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  R( c, d, e, a, b, F4, 78, 1 ); W_PRECALC_16_31_2(30, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  addl state_h0(RSTATE), a;      W_PRECALC_16_31_3(31, W1, W2, W3, W4, W5, Wtmp0, Wtmp1, K2);
  R( b, c, d, e, a, F4, 79, 1 );
  addl ne, a;
  xorl ne, ne;

  /* Update the chaining variables. */
  addl state_h3(RSTATE), d;
  addl state_h2(RSTATE), c;
  addl state_h1(RSTATE), b;
  addl state_h4(RSTATE), e;

  movl d, state_h3(RSTATE);
  movl c, state_h2(RSTATE);
  movl b, state_h1(RSTATE);
  movl a, state_h0(RSTATE);
  movl e, state_h4(RSTATE);

  jmp .Loop;

.align 16
.Lend:
  vzeroall;

  /* Transform 48-79 for block 2 + burn stack */
  R( c, d, e, a, b, F3, 48, 1 );
  R( b, c, d, e, a, F3, 49, 1 );
  R( a, b, c, d, e, F3, 50, 1 );
  R( e, a, b, c, d, F3, 51, 1 );
  R( d, e, a, b, c, F3, 52, 1 );
  R( c, d, e, a, b, F3, 53, 1 );
  R( b, c, d, e, a, F3, 54, 1 );
  R( a, b, c, d, e, F3, 55, 1 );
  R( e, a, b, c, d, F3, 56, 1 );
  R( d, e, a, b, c, F3, 57, 1 );
  R( c, d, e, a, b, F3, 58, 1 );
  R( b, c, d, e, a, F3, 59, 1 );
  R( a, b, c, d, e, F4, 60, 1 ); vmovdqa %ymm0, (0*32)(%rsp);
  R( e, a, b, c, d, F4, 61, 1 ); vmovdqa %ymm0, (1*32)(%rsp);
  R( d, e, a, b, c, F4, 62, 1 ); vmovdqa %ymm0, (2*32)(%rsp);
  R( c, d, e, a, b, F4, 63, 1 ); vmovdqa %ymm0, (3*32)(%rsp);
  R( b, c, d, e, a, F4, 64, 1 ); vmovdqa %ymm0, (4*32)(%rsp);
  R( a, b, c, d, e, F4, 65, 1 ); vmovdqa %ymm0, (5*32)(%rsp);
  R( e, a, b, c, d, F4, 66, 1 ); vmovdqa %ymm0, (6*32)(%rsp);
  R( d, e, a, b, c, F4, 67, 1 ); vmovdqa %ymm0, (7*32)(%rsp);
  R( c, d, e, a, b, F4, 68, 1 ); vmovdqa %ymm0, (8*32)(%rsp);
  R( b, c, d, e, a, F4, 69, 1 ); vmovdqa %ymm0, (9*32)(%rsp);
  R( a, b, c, d, e, F4, 70, 1 ); vmovdqa %ymm0, (10*32)(%rsp);
  R( e, a, b, c, d, F4, 71, 1 ); vmovdqa %ymm0, (11*32)(%rsp);
  R( d, e, a, b, c, F4, 72, 1 ); vmovdqa %ymm0, (12*32)(%rsp);
  R( c, d, e, a, b, F4, 73, 1 ); vmovdqa %ymm0, (13*32)(%rsp);
  R( b, c, d, e, a, F4, 74, 1 ); vmovdqa %ymm0, (14*32)(%rsp);
  R( a, b, c, d, e, F4, 75, 1 ); vmovdqa %ymm0, (15*32)(%rsp);
  R( e, a, b, c, d, F4, 76, 1 ); vmovdqa %ymm0, (16*32)(%rsp);
  R( d, e, a, b, c, F4, 77, 1 ); vmovdqa %ymm0, (17*32)(%rsp);
  R( c, d, e, a, b, F4, 78, 1 ); vmovdqa %ymm0, (18*32)(%rsp);
  addl state_h0(RSTATE), a;
  R( b, c, d, e, a, F4, 79, 1 );
  addl ne, a;
  xorl ne, ne;

  /* WK_STACK_WORDS*4/32-1 = 19 */
  vmovdqa %ymm0, (19*32)(%rsp);

  /* Update the chaining variables. */
  addl state_h3(RSTATE), d;
  addl state_h2(RSTATE), c;
  addl state_h1(RSTATE), b;
  addl state_h4(RSTATE), e;

  movl d, state_h3(RSTATE);
  movl c, state_h2(RSTATE);
  movl b, state_h1(RSTATE);
  movl a, state_h0(RSTATE);
  movl e, state_h4(RSTATE);

  movq ROLDSTACK, %rsp;
  CFI_REGISTER(ROLDSTACK, %rsp);
  CFI_DEF_CFA_REGISTER(%rsp);

  popq %r12;
  CFI_POP(%r12);
  popq %rbp;
  CFI_POP(%rbp);
  popq %rbx;
  CFI_POP(%rbx);

  /* stack already burned */
  xorl %eax, %eax;

  ret_spec_stop;
  CFI_ENDPROC();
ELF(.size _gcry_sha1_transform_amd64_avx2_bmi2,
    .-_gcry_sha1_transform_amd64_avx2_bmi2;)

#endif
#endif
