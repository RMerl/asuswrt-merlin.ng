/* sha1-ssse3-amd64.S - Intel SSSE3 accelerated SHA-1 transform function
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
    defined(HAVE_GCC_INLINE_ASM_SSSE3) && defined(USE_SHA1)

#include "asm-common-amd64.h"


/* Context structure */

#define state_h0 0
#define state_h1 4
#define state_h2 8
#define state_h3 12
#define state_h4 16


/* Constants */

.text
#define K1  0x5A827999
#define K2  0x6ED9EBA1
#define K3  0x8F1BBCDC
#define K4  0xCA62C1D6
.align 16
.LK_XMM:
.LK1:	.long K1, K1, K1, K1
.LK2:	.long K2, K2, K2, K2
.LK3:	.long K3, K3, K3, K3
.LK4:	.long K4, K4, K4, K4

.Lbswap_shufb_ctl:
	.long 0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f


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

#define RT0 %esi
#define RT1 %ebp

#define Wtmp0 %xmm0
#define Wtmp1 %xmm1

#define W0 %xmm2
#define W1 %xmm3
#define W2 %xmm4
#define W3 %xmm5
#define W4 %xmm6
#define W5 %xmm7
#define W6 %xmm8
#define W7 %xmm9

#define BSWAP_REG %xmm10


/* Round function macros. */

#define WK(i) (((i) & 15) * 4)(%rsp)

#define R_F1(a,b,c,d,e,i) \
	movl c, RT0; \
	addl WK(i), e; \
	xorl d, RT0; \
	movl a, RT1; \
	andl b, RT0; \
	roll $30, b; \
	xorl d, RT0; \
	leal (RT0,e), e; \
	roll $5, RT1; \
	addl RT1, e;

#define R_F2(a,b,c,d,e,i) \
	movl c, RT0; \
	addl WK(i), e; \
	xorl b, RT0; \
	roll $30, b; \
	xorl d, RT0; \
	movl a, RT1; \
	leal (RT0,e), e; \
	roll $5, RT1; \
	addl RT1, e;

#define R_F3(a,b,c,d,e,i) \
	movl c, RT0; \
	movl b, RT1; \
	xorl b, RT0; \
	andl c, RT1; \
	andl d, RT0; \
	addl RT1, e; \
	addl WK(i), e; \
	roll $30, b; \
	movl a, RT1; \
	leal (RT0,e), e; \
	roll $5, RT1; \
	addl RT1, e;

#define R_F4(a,b,c,d,e,i) R_F2(a,b,c,d,e,i)

#define R(a,b,c,d,e,f,i) \
	R_##f(a,b,c,d,e,i)


/* Input expansion macros. */

#define W_PRECALC_00_15_0(i, W, tmp0) \
	movdqu (4*(i))(RDATA), tmp0;

#define W_PRECALC_00_15_1(i, W, tmp0) \
	pshufb BSWAP_REG, tmp0; \
	movdqa tmp0, W;

#define W_PRECALC_00_15_2(i, W, tmp0) \
	paddd (.LK_XMM + ((i)/20)*16) rRIP, tmp0;

#define W_PRECALC_00_15_3(i, W, tmp0) \
	movdqa tmp0, WK(i&~3);

#define W_PRECALC_16_31_0(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	movdqa W_m12, W; \
	palignr $8, W_m16, W; \
	movdqa W_m04, tmp0; \
	psrldq $4, tmp0; \
	pxor W_m08, W;

#define W_PRECALC_16_31_1(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	pxor W_m16, tmp0; \
	pxor tmp0, W; \
	movdqa W, tmp1; \
	movdqa W, tmp0; \
	pslldq $12, tmp1;

#define W_PRECALC_16_31_2(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	psrld $31, W; \
	pslld $1, tmp0; \
	por W, tmp0; \
	movdqa tmp1, W; \
	psrld $30, tmp1; \
	pslld $2, W;

#define W_PRECALC_16_31_3(i, W, W_m04, W_m08, W_m12, W_m16, tmp0, tmp1) \
	pxor W, tmp0; \
	pxor tmp1, tmp0; \
	movdqa tmp0, W; \
	paddd (.LK_XMM + ((i)/20)*16) rRIP, tmp0; \
	movdqa tmp0, WK((i)&~3);

#define W_PRECALC_32_79_0(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	movdqa W_m04, tmp0; \
	pxor W_m28, W; \
	palignr $8, W_m08, tmp0;

#define W_PRECALC_32_79_1(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	pxor W_m16, W; \
	pxor tmp0, W; \
	movdqa W, tmp0;

#define W_PRECALC_32_79_2(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	psrld $30, W; \
	pslld $2, tmp0; \
	por W, tmp0;

#define W_PRECALC_32_79_3(i, W, W_m04, W_m08, W_m12, W_m16, W_m20, W_m24, W_m28, tmp0) \
	movdqa tmp0, W; \
	paddd (.LK_XMM + ((i)/20)*16) rRIP, tmp0; \
	movdqa tmp0, WK((i)&~3);

#define CLEAR_REG(reg) pxor reg, reg;


/*
 * Transform nblks*64 bytes (nblks*16 32-bit words) at DATA.
 *
 * unsigned int
 * _gcry_sha1_transform_amd64_ssse3 (void *ctx, const unsigned char *data,
 *                                   size_t nblks)
 */
.globl _gcry_sha1_transform_amd64_ssse3
ELF(.type _gcry_sha1_transform_amd64_ssse3,@function)
.align 16
_gcry_sha1_transform_amd64_ssse3:
  /* input:
   *	%rdi: ctx, CTX
   *	%rsi: data (64*nblks bytes)
   *	%rdx: nblks
   */
  CFI_STARTPROC();

  xorl %eax, %eax;
  cmpq $0, %rdx;
  jz .Lret;

  movq %rdx, RNBLKS;
  movq %rdi, RSTATE;
  movq %rsi, RDATA;
  pushq %rbx;
  CFI_PUSH(%rbx);
  pushq %rbp;
  CFI_PUSH(%rbp);

  movq %rsp, ROLDSTACK;
  CFI_DEF_CFA_REGISTER(ROLDSTACK);

  subq $(16*4), %rsp;
  andq $(~31), %rsp;

  /* Get the values of the chaining variables. */
  movl state_h0(RSTATE), a;
  movl state_h1(RSTATE), b;
  movl state_h2(RSTATE), c;
  movl state_h3(RSTATE), d;
  movl state_h4(RSTATE), e;

  movdqa .Lbswap_shufb_ctl rRIP, BSWAP_REG;

  /* Precalc 0-15. */
  W_PRECALC_00_15_0(0, W0, Wtmp0);
  W_PRECALC_00_15_1(1, W0, Wtmp0);
  W_PRECALC_00_15_2(2, W0, Wtmp0);
  W_PRECALC_00_15_3(3, W0, Wtmp0);
  W_PRECALC_00_15_0(4, W7, Wtmp0);
  W_PRECALC_00_15_1(5, W7, Wtmp0);
  W_PRECALC_00_15_2(6, W7, Wtmp0);
  W_PRECALC_00_15_3(7, W7, Wtmp0);
  W_PRECALC_00_15_0(8, W6, Wtmp0);
  W_PRECALC_00_15_1(9, W6, Wtmp0);
  W_PRECALC_00_15_2(10, W6, Wtmp0);
  W_PRECALC_00_15_3(11, W6, Wtmp0);
  W_PRECALC_00_15_0(12, W5, Wtmp0);
  W_PRECALC_00_15_1(13, W5, Wtmp0);
  W_PRECALC_00_15_2(14, W5, Wtmp0);
  W_PRECALC_00_15_3(15, W5, Wtmp0);

.align 8
.Loop:
  addq $64, RDATA;

  /* Transform 0-15 + Precalc 16-31. */
  R( a, b, c, d, e, F1,  0 ); W_PRECALC_16_31_0(16, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( e, a, b, c, d, F1,  1 ); W_PRECALC_16_31_1(17, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( d, e, a, b, c, F1,  2 ); W_PRECALC_16_31_2(18, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( c, d, e, a, b, F1,  3 ); W_PRECALC_16_31_3(19, W4, W5, W6, W7, W0, Wtmp0, Wtmp1);
  R( b, c, d, e, a, F1,  4 ); W_PRECALC_16_31_0(20, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F1,  5 ); W_PRECALC_16_31_1(21, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( e, a, b, c, d, F1,  6 ); W_PRECALC_16_31_2(22, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( d, e, a, b, c, F1,  7 ); W_PRECALC_16_31_3(23, W3, W4, W5, W6, W7, Wtmp0, Wtmp1);
  R( c, d, e, a, b, F1,  8 ); W_PRECALC_16_31_0(24, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( b, c, d, e, a, F1,  9 ); W_PRECALC_16_31_1(25, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F1, 10 ); W_PRECALC_16_31_2(26, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( e, a, b, c, d, F1, 11 ); W_PRECALC_16_31_3(27, W2, W3, W4, W5, W6, Wtmp0, Wtmp1);
  R( d, e, a, b, c, F1, 12 ); W_PRECALC_16_31_0(28, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  R( c, d, e, a, b, F1, 13 ); W_PRECALC_16_31_1(29, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  R( b, c, d, e, a, F1, 14 ); W_PRECALC_16_31_2(30, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);
  R( a, b, c, d, e, F1, 15 ); W_PRECALC_16_31_3(31, W1, W2, W3, W4, W5, Wtmp0, Wtmp1);

  /* Transform 16-63 + Precalc 32-79. */
  R( e, a, b, c, d, F1, 16 ); W_PRECALC_32_79_0(32, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( d, e, a, b, c, F1, 17 ); W_PRECALC_32_79_1(33, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( c, d, e, a, b, F1, 18 ); W_PRECALC_32_79_2(34, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( b, c, d, e, a, F1, 19 ); W_PRECALC_32_79_3(35, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( a, b, c, d, e, F2, 20 ); W_PRECALC_32_79_0(36, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( e, a, b, c, d, F2, 21 ); W_PRECALC_32_79_1(37, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( d, e, a, b, c, F2, 22 ); W_PRECALC_32_79_2(38, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( c, d, e, a, b, F2, 23 ); W_PRECALC_32_79_3(39, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( b, c, d, e, a, F2, 24 ); W_PRECALC_32_79_0(40, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( a, b, c, d, e, F2, 25 ); W_PRECALC_32_79_1(41, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( e, a, b, c, d, F2, 26 ); W_PRECALC_32_79_2(42, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( d, e, a, b, c, F2, 27 ); W_PRECALC_32_79_3(43, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( c, d, e, a, b, F2, 28 ); W_PRECALC_32_79_0(44, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( b, c, d, e, a, F2, 29 ); W_PRECALC_32_79_1(45, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( a, b, c, d, e, F2, 30 ); W_PRECALC_32_79_2(46, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( e, a, b, c, d, F2, 31 ); W_PRECALC_32_79_3(47, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( d, e, a, b, c, F2, 32 ); W_PRECALC_32_79_0(48, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( c, d, e, a, b, F2, 33 ); W_PRECALC_32_79_1(49, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( b, c, d, e, a, F2, 34 ); W_PRECALC_32_79_2(50, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( a, b, c, d, e, F2, 35 ); W_PRECALC_32_79_3(51, W4, W5, W6, W7, W0, W1, W2, W3, Wtmp0);
  R( e, a, b, c, d, F2, 36 ); W_PRECALC_32_79_0(52, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( d, e, a, b, c, F2, 37 ); W_PRECALC_32_79_1(53, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( c, d, e, a, b, F2, 38 ); W_PRECALC_32_79_2(54, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( b, c, d, e, a, F2, 39 ); W_PRECALC_32_79_3(55, W3, W4, W5, W6, W7, W0, W1, W2, Wtmp0);
  R( a, b, c, d, e, F3, 40 ); W_PRECALC_32_79_0(56, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( e, a, b, c, d, F3, 41 ); W_PRECALC_32_79_1(57, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( d, e, a, b, c, F3, 42 ); W_PRECALC_32_79_2(58, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( c, d, e, a, b, F3, 43 ); W_PRECALC_32_79_3(59, W2, W3, W4, W5, W6, W7, W0, W1, Wtmp0);
  R( b, c, d, e, a, F3, 44 ); W_PRECALC_32_79_0(60, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( a, b, c, d, e, F3, 45 ); W_PRECALC_32_79_1(61, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( e, a, b, c, d, F3, 46 ); W_PRECALC_32_79_2(62, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( d, e, a, b, c, F3, 47 ); W_PRECALC_32_79_3(63, W1, W2, W3, W4, W5, W6, W7, W0, Wtmp0);
  R( c, d, e, a, b, F3, 48 ); W_PRECALC_32_79_0(64, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( b, c, d, e, a, F3, 49 ); W_PRECALC_32_79_1(65, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( a, b, c, d, e, F3, 50 ); W_PRECALC_32_79_2(66, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( e, a, b, c, d, F3, 51 ); W_PRECALC_32_79_3(67, W0, W1, W2, W3, W4, W5, W6, W7, Wtmp0);
  R( d, e, a, b, c, F3, 52 ); W_PRECALC_32_79_0(68, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( c, d, e, a, b, F3, 53 ); W_PRECALC_32_79_1(69, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( b, c, d, e, a, F3, 54 ); W_PRECALC_32_79_2(70, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( a, b, c, d, e, F3, 55 ); W_PRECALC_32_79_3(71, W7, W0, W1, W2, W3, W4, W5, W6, Wtmp0);
  R( e, a, b, c, d, F3, 56 ); W_PRECALC_32_79_0(72, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( d, e, a, b, c, F3, 57 ); W_PRECALC_32_79_1(73, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( c, d, e, a, b, F3, 58 ); W_PRECALC_32_79_2(74, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( b, c, d, e, a, F3, 59 ); W_PRECALC_32_79_3(75, W6, W7, W0, W1, W2, W3, W4, W5, Wtmp0);
  R( a, b, c, d, e, F4, 60 ); W_PRECALC_32_79_0(76, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( e, a, b, c, d, F4, 61 ); W_PRECALC_32_79_1(77, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( d, e, a, b, c, F4, 62 ); W_PRECALC_32_79_2(78, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);
  R( c, d, e, a, b, F4, 63 ); W_PRECALC_32_79_3(79, W5, W6, W7, W0, W1, W2, W3, W4, Wtmp0);

  decq RNBLKS;
  jz .Lend;

  /* Transform 64-79 + Precalc 0-15 of next block. */
  R( b, c, d, e, a, F4, 64 ); W_PRECALC_00_15_0(0, W0, Wtmp0);
  R( a, b, c, d, e, F4, 65 ); W_PRECALC_00_15_1(1, W0, Wtmp0);
  R( e, a, b, c, d, F4, 66 ); W_PRECALC_00_15_2(2, W0, Wtmp0);
  R( d, e, a, b, c, F4, 67 ); W_PRECALC_00_15_3(3, W0, Wtmp0);
  R( c, d, e, a, b, F4, 68 ); W_PRECALC_00_15_0(4, W7, Wtmp0);
  R( b, c, d, e, a, F4, 69 ); W_PRECALC_00_15_1(5, W7, Wtmp0);
  R( a, b, c, d, e, F4, 70 ); W_PRECALC_00_15_2(6, W7, Wtmp0);
  R( e, a, b, c, d, F4, 71 ); W_PRECALC_00_15_3(7, W7, Wtmp0);
  R( d, e, a, b, c, F4, 72 ); W_PRECALC_00_15_0(8, W6, Wtmp0);
  R( c, d, e, a, b, F4, 73 ); W_PRECALC_00_15_1(9, W6, Wtmp0);
  R( b, c, d, e, a, F4, 74 ); W_PRECALC_00_15_2(10, W6, Wtmp0);
  R( a, b, c, d, e, F4, 75 ); W_PRECALC_00_15_3(11, W6, Wtmp0);
  R( e, a, b, c, d, F4, 76 ); W_PRECALC_00_15_0(12, W5, Wtmp0);
  R( d, e, a, b, c, F4, 77 ); W_PRECALC_00_15_1(13, W5, Wtmp0);
  R( c, d, e, a, b, F4, 78 );
  addl state_h0(RSTATE), a;   W_PRECALC_00_15_2(14, W5, Wtmp0);
  R( b, c, d, e, a, F4, 79 ); W_PRECALC_00_15_3(15, W5, Wtmp0);

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
  /* Transform 64-79 + Clear XMM registers + Burn stack. */
  R( b, c, d, e, a, F4, 64 ); CLEAR_REG(BSWAP_REG);
  R( a, b, c, d, e, F4, 65 ); CLEAR_REG(Wtmp0);
  R( e, a, b, c, d, F4, 66 ); CLEAR_REG(Wtmp1);
  R( d, e, a, b, c, F4, 67 ); CLEAR_REG(W0);
  R( c, d, e, a, b, F4, 68 ); CLEAR_REG(W1);
  R( b, c, d, e, a, F4, 69 ); CLEAR_REG(W2);
  R( a, b, c, d, e, F4, 70 ); CLEAR_REG(W3);
  R( e, a, b, c, d, F4, 71 ); CLEAR_REG(W4);
  R( d, e, a, b, c, F4, 72 ); CLEAR_REG(W5);
  R( c, d, e, a, b, F4, 73 ); CLEAR_REG(W6);
  R( b, c, d, e, a, F4, 74 ); CLEAR_REG(W7);
  R( a, b, c, d, e, F4, 75 );
  R( e, a, b, c, d, F4, 76 ); movdqa Wtmp0, (0*16)(%rsp);
  R( d, e, a, b, c, F4, 77 ); movdqa Wtmp0, (1*16)(%rsp);
  R( c, d, e, a, b, F4, 78 ); movdqa Wtmp0, (2*16)(%rsp);
  addl state_h0(RSTATE), a;
  R( b, c, d, e, a, F4, 79 );

  /* 16*4/16-1 = 3 */
  movdqa Wtmp0, (3*16)(%rsp);

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

  popq %rbp;
  CFI_POP(%rbp);
  popq %rbx;
  CFI_POP(%rbx);

  /* stack already burned */
  xorl %eax, %eax;

.Lret:
  ret_spec_stop;
  CFI_ENDPROC();
ELF(.size _gcry_sha1_transform_amd64_ssse3,
    .-_gcry_sha1_transform_amd64_ssse3;)

#endif
#endif
