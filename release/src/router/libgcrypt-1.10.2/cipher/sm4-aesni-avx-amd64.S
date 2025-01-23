/* sm4-avx-aesni-amd64.S  -  AES-NI/AVX implementation of SM4 cipher
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

/* Based on SM4 AES-NI work by Markku-Juhani O. Saarinen at:
 *  https://github.com/mjosaarinen/sm4ni
 */

#include <config.h>

#ifdef __x86_64
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)

#include "asm-common-amd64.h"

/* vector registers */
#define RX0          %xmm0
#define RX1          %xmm1
#define MASK_4BIT    %xmm2
#define RTMP0        %xmm3
#define RTMP1        %xmm4
#define RTMP2        %xmm5
#define RTMP3        %xmm6
#define RTMP4        %xmm7

#define RA0          %xmm8
#define RA1          %xmm9
#define RA2          %xmm10
#define RA3          %xmm11

#define RB0          %xmm12
#define RB1          %xmm13
#define RB2          %xmm14
#define RB3          %xmm15

#define RNOT         %xmm0
#define RBSWAP       %xmm1

/**********************************************************************
  helper macros
 **********************************************************************/

/* Transpose four 32-bit words between 128-bit vectors. */
#define transpose_4x4(x0, x1, x2, x3, t1, t2) \
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

/* post-SubByte transform. */
#define transform_pre(x, lo_t, hi_t, mask4bit, tmp0) \
	vpand x, mask4bit, tmp0; \
	vpandn x, mask4bit, x; \
	vpsrld $4, x, x; \
	\
	vpshufb tmp0, lo_t, tmp0; \
	vpshufb x, hi_t, x; \
	vpxor tmp0, x, x;

/* post-SubByte transform. Note: x has been XOR'ed with mask4bit by
 * 'vaeslastenc' instruction. */
#define transform_post(x, lo_t, hi_t, mask4bit, tmp0) \
	vpandn mask4bit, x, tmp0; \
	vpsrld $4, x, x; \
	vpand x, mask4bit, x; \
	\
	vpshufb tmp0, lo_t, tmp0; \
	vpshufb x, hi_t, x; \
	vpxor tmp0, x, x;

/**********************************************************************
  4-way && 8-way SM4 with AES-NI and AVX
 **********************************************************************/

.text
.align 16

/*
 * Following four affine transform look-up tables are from work by
 * Markku-Juhani O. Saarinen, at https://github.com/mjosaarinen/sm4ni
 *
 * These allow exposing SM4 S-Box from AES SubByte.
 */

/* pre-SubByte affine transform, from SM4 field to AES field. */
.Lpre_tf_lo_s:
	.quad 0x9197E2E474720701, 0xC7C1B4B222245157
.Lpre_tf_hi_s:
	.quad 0xE240AB09EB49A200, 0xF052B91BF95BB012

/* post-SubByte affine transform, from AES field to SM4 field. */
.Lpost_tf_lo_s:
	.quad 0x5B67F2CEA19D0834, 0xEDD14478172BBE82
.Lpost_tf_hi_s:
	.quad 0xAE7201DD73AFDC00, 0x11CDBE62CC1063BF

/* For isolating SubBytes from AESENCLAST, inverse shift row */
.Linv_shift_row:
	.byte 0x00, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x0e, 0x0b
	.byte 0x08, 0x05, 0x02, 0x0f, 0x0c, 0x09, 0x06, 0x03

/* Inverse shift row + Rotate left by 8 bits on 32-bit words with vpshufb */
.Linv_shift_row_rol_8:
	.byte 0x07, 0x00, 0x0d, 0x0a, 0x0b, 0x04, 0x01, 0x0e
	.byte 0x0f, 0x08, 0x05, 0x02, 0x03, 0x0c, 0x09, 0x06

/* Inverse shift row + Rotate left by 16 bits on 32-bit words with vpshufb */
.Linv_shift_row_rol_16:
	.byte 0x0a, 0x07, 0x00, 0x0d, 0x0e, 0x0b, 0x04, 0x01
	.byte 0x02, 0x0f, 0x08, 0x05, 0x06, 0x03, 0x0c, 0x09

/* Inverse shift row + Rotate left by 24 bits on 32-bit words with vpshufb */
.Linv_shift_row_rol_24:
	.byte 0x0d, 0x0a, 0x07, 0x00, 0x01, 0x0e, 0x0b, 0x04
	.byte 0x05, 0x02, 0x0f, 0x08, 0x09, 0x06, 0x03, 0x0c

/* For CTR-mode IV byteswap */
.Lbswap128_mask:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

/* For input word byte-swap */
.Lbswap32_mask:
	.byte 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12

.align 4
/* 4-bit mask */
.L0f0f0f0f:
	.long 0x0f0f0f0f

.align 8
.globl _gcry_sm4_aesni_avx_expand_key
ELF(.type   _gcry_sm4_aesni_avx_expand_key,@function;)
_gcry_sm4_aesni_avx_expand_key:
	/* input:
	 *	%rdi: 128-bit key
	 *	%rsi: rkey_enc
	 *	%rdx: rkey_dec
	 *	%rcx: fk array
	 *	%r8: ck array
	 */
	CFI_STARTPROC();

	vmovd 0*4(%rdi), RA0;
	vmovd 1*4(%rdi), RA1;
	vmovd 2*4(%rdi), RA2;
	vmovd 3*4(%rdi), RA3;

	vmovdqa .Lbswap32_mask rRIP, RTMP2;
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;

	vmovd 0*4(%rcx), RB0;
	vmovd 1*4(%rcx), RB1;
	vmovd 2*4(%rcx), RB2;
	vmovd 3*4(%rcx), RB3;
	vpxor RB0, RA0, RA0;
	vpxor RB1, RA1, RA1;
	vpxor RB2, RA2, RA2;
	vpxor RB3, RA3, RA3;

	vbroadcastss .L0f0f0f0f rRIP, MASK_4BIT;
	vmovdqa .Lpre_tf_lo_s rRIP, RTMP4;
	vmovdqa .Lpre_tf_hi_s rRIP, RB0;
	vmovdqa .Lpost_tf_lo_s rRIP, RB1;
	vmovdqa .Lpost_tf_hi_s rRIP, RB2;
	vmovdqa .Linv_shift_row rRIP, RB3;

#define ROUND(round, s0, s1, s2, s3) \
	vbroadcastss (4*(round))(%r8), RX0; \
	vpxor s1, RX0, RX0; \
	vpxor s2, RX0, RX0; \
	vpxor s3, RX0, RX0; /* s1 ^ s2 ^ s3 ^ rk */ \
	\
	/* sbox, non-linear part */ \
	transform_pre(RX0, RTMP4, RB0, MASK_4BIT, RTMP0); \
	vaesenclast MASK_4BIT, RX0, RX0; \
	transform_post(RX0, RB1, RB2, MASK_4BIT, RTMP0); \
	\
	/* linear part */ \
	vpshufb RB3, RX0, RX0; \
	vpxor RX0, s0, s0; /* s0 ^ x */ \
	vpslld $13, RX0, RTMP0; \
	vpsrld $19, RX0, RTMP1; \
	vpslld $23, RX0, RTMP2; \
	vpsrld $9, RX0, RTMP3; \
	vpxor RTMP0, RTMP1, RTMP1;  \
	vpxor RTMP2, RTMP3, RTMP3;  \
	vpxor RTMP1, s0, s0; /* s0 ^ x ^ rol(x,13) */ \
	vpxor RTMP3, s0, s0; /* s0 ^ x ^ rol(x,13) ^ rol(x,23) */

	leaq (32*4)(%r8), %rax;
	leaq (32*4)(%rdx), %rdx;
.align 16
.Lroundloop_expand_key:
	leaq (-4*4)(%rdx), %rdx;
	ROUND(0, RA0, RA1, RA2, RA3);
	ROUND(1, RA1, RA2, RA3, RA0);
	ROUND(2, RA2, RA3, RA0, RA1);
	ROUND(3, RA3, RA0, RA1, RA2);
	leaq (4*4)(%r8), %r8;
	vmovd RA0, (0*4)(%rsi);
	vmovd RA1, (1*4)(%rsi);
	vmovd RA2, (2*4)(%rsi);
	vmovd RA3, (3*4)(%rsi);
	vmovd RA0, (3*4)(%rdx);
	vmovd RA1, (2*4)(%rdx);
	vmovd RA2, (1*4)(%rdx);
	vmovd RA3, (0*4)(%rdx);
	leaq (4*4)(%rsi), %rsi;
	cmpq %rax, %r8;
	jne .Lroundloop_expand_key;

#undef ROUND

	vzeroall;
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_expand_key,.-_gcry_sm4_aesni_avx_expand_key;)

.align 8
ELF(.type   sm4_aesni_avx_crypt_blk1_4,@function;)
sm4_aesni_avx_crypt_blk1_4:
	/* input:
	 *	%rdi: round key array, CTX
	 *	%rsi: dst (1..4 blocks)
	 *	%rdx: src (1..4 blocks)
	 *	%rcx: num blocks (1..4)
	 */
	CFI_STARTPROC();

	vmovdqu 0*16(%rdx), RA0;
	vmovdqa RA0, RA1;
	vmovdqa RA0, RA2;
	vmovdqa RA0, RA3;
	cmpq $2, %rcx;
	jb .Lblk4_load_input_done;
	vmovdqu 1*16(%rdx), RA1;
	je .Lblk4_load_input_done;
	vmovdqu 2*16(%rdx), RA2;
	cmpq $3, %rcx;
	je .Lblk4_load_input_done;
	vmovdqu 3*16(%rdx), RA3;

.Lblk4_load_input_done:

	vmovdqa .Lbswap32_mask rRIP, RTMP2;
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;

	vbroadcastss .L0f0f0f0f rRIP, MASK_4BIT;
	vmovdqa .Lpre_tf_lo_s rRIP, RTMP4;
	vmovdqa .Lpre_tf_hi_s rRIP, RB0;
	vmovdqa .Lpost_tf_lo_s rRIP, RB1;
	vmovdqa .Lpost_tf_hi_s rRIP, RB2;
	vmovdqa .Linv_shift_row rRIP, RB3;
	vmovdqa .Linv_shift_row_rol_8 rRIP, RTMP2;
	vmovdqa .Linv_shift_row_rol_16 rRIP, RTMP3;
	transpose_4x4(RA0, RA1, RA2, RA3, RTMP0, RTMP1);

#define ROUND(round, s0, s1, s2, s3) \
	vbroadcastss (4*(round))(%rdi), RX0; \
	vpxor s1, RX0, RX0; \
	vpxor s2, RX0, RX0; \
	vpxor s3, RX0, RX0; /* s1 ^ s2 ^ s3 ^ rk */ \
	\
	/* sbox, non-linear part */ \
	transform_pre(RX0, RTMP4, RB0, MASK_4BIT, RTMP0); \
	vaesenclast MASK_4BIT, RX0, RX0; \
	transform_post(RX0, RB1, RB2, MASK_4BIT, RTMP0); \
	\
	/* linear part */ \
	vpshufb RB3, RX0, RTMP0; \
	vpxor RTMP0, s0, s0; /* s0 ^ x */ \
	vpshufb RTMP2, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) */ \
	vpshufb RTMP3, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) ^ rol(x,16) */ \
	vpshufb .Linv_shift_row_rol_24 rRIP, RX0, RTMP1; \
	vpxor RTMP1, s0, s0; /* s0 ^ x ^ rol(x,24) */ \
	vpslld $2, RTMP0, RTMP1; \
	vpsrld $30, RTMP0, RTMP0; \
	vpxor RTMP0, s0, s0;  \
	vpxor RTMP1, s0, s0; /* s0 ^ x ^ rol(x,2) ^ rol(x,10) ^ rol(x,18) ^ rol(x,24) */

	leaq (32*4)(%rdi), %rax;
.align 16
.Lroundloop_blk4:
	ROUND(0, RA0, RA1, RA2, RA3);
	ROUND(1, RA1, RA2, RA3, RA0);
	ROUND(2, RA2, RA3, RA0, RA1);
	ROUND(3, RA3, RA0, RA1, RA2);
	leaq (4*4)(%rdi), %rdi;
	cmpq %rax, %rdi;
	jne .Lroundloop_blk4;

#undef ROUND

	vmovdqa .Lbswap128_mask rRIP, RTMP2;

	transpose_4x4(RA0, RA1, RA2, RA3, RTMP0, RTMP1);
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;

	vmovdqu RA0, 0*16(%rsi);
	cmpq $2, %rcx;
	jb .Lblk4_store_output_done;
	vmovdqu RA1, 1*16(%rsi);
	je .Lblk4_store_output_done;
	vmovdqu RA2, 2*16(%rsi);
	cmpq $3, %rcx;
	je .Lblk4_store_output_done;
	vmovdqu RA3, 3*16(%rsi);

.Lblk4_store_output_done:
	vzeroall;
	xorl %eax, %eax;
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size sm4_aesni_avx_crypt_blk1_4,.-sm4_aesni_avx_crypt_blk1_4;)

.align 8
ELF(.type __sm4_crypt_blk8,@function;)
__sm4_crypt_blk8:
	/* input:
	 *	%rdi: round key array, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel
	 * 						ciphertext blocks
	 * output:
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: eight parallel plaintext
	 *						blocks
	 */
	CFI_STARTPROC();

	vmovdqa .Lbswap32_mask rRIP, RTMP2;
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;
	vpshufb RTMP2, RB0, RB0;
	vpshufb RTMP2, RB1, RB1;
	vpshufb RTMP2, RB2, RB2;
	vpshufb RTMP2, RB3, RB3;

	vbroadcastss .L0f0f0f0f rRIP, MASK_4BIT;
	transpose_4x4(RA0, RA1, RA2, RA3, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RTMP0, RTMP1);

#define ROUND(round, s0, s1, s2, s3, r0, r1, r2, r3) \
	vbroadcastss (4*(round))(%rdi), RX0; \
	vmovdqa .Lpre_tf_lo_s rRIP, RTMP4; \
	vmovdqa .Lpre_tf_hi_s rRIP, RTMP1; \
	vmovdqa RX0, RX1; \
	vpxor s1, RX0, RX0; \
	vpxor s2, RX0, RX0; \
	vpxor s3, RX0, RX0; /* s1 ^ s2 ^ s3 ^ rk */ \
	    vmovdqa .Lpost_tf_lo_s rRIP, RTMP2; \
	    vmovdqa .Lpost_tf_hi_s rRIP, RTMP3; \
	    vpxor r1, RX1, RX1; \
	    vpxor r2, RX1, RX1; \
	    vpxor r3, RX1, RX1; /* r1 ^ r2 ^ r3 ^ rk */ \
	\
	/* sbox, non-linear part */ \
	transform_pre(RX0, RTMP4, RTMP1, MASK_4BIT, RTMP0); \
	    transform_pre(RX1, RTMP4, RTMP1, MASK_4BIT, RTMP0); \
	    vmovdqa .Linv_shift_row rRIP, RTMP4; \
	vaesenclast MASK_4BIT, RX0, RX0; \
	    vaesenclast MASK_4BIT, RX1, RX1; \
	transform_post(RX0, RTMP2, RTMP3, MASK_4BIT, RTMP0); \
	    transform_post(RX1, RTMP2, RTMP3, MASK_4BIT, RTMP0); \
	\
	/* linear part */ \
	vpshufb RTMP4, RX0, RTMP0; \
	vpxor RTMP0, s0, s0; /* s0 ^ x */ \
	    vpshufb RTMP4, RX1, RTMP2; \
	    vmovdqa .Linv_shift_row_rol_8 rRIP, RTMP4; \
	    vpxor RTMP2, r0, r0; /* r0 ^ x */ \
	vpshufb RTMP4, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) */ \
	    vpshufb RTMP4, RX1, RTMP3; \
	    vmovdqa .Linv_shift_row_rol_16 rRIP, RTMP4; \
	    vpxor RTMP3, RTMP2, RTMP2; /* x ^ rol(x,8) */ \
	vpshufb RTMP4, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) ^ rol(x,16) */ \
	    vpshufb RTMP4, RX1, RTMP3; \
	    vmovdqa .Linv_shift_row_rol_24 rRIP, RTMP4; \
	    vpxor RTMP3, RTMP2, RTMP2; /* x ^ rol(x,8) ^ rol(x,16) */ \
	vpshufb RTMP4, RX0, RTMP1; \
	vpxor RTMP1, s0, s0; /* s0 ^ x ^ rol(x,24) */ \
	vpslld $2, RTMP0, RTMP1; \
	vpsrld $30, RTMP0, RTMP0; \
	vpxor RTMP0, s0, s0;  \
	vpxor RTMP1, s0, s0; /* s0 ^ x ^ rol(x,2) ^ rol(x,10) ^ rol(x,18) ^ rol(x,24) */ \
	    vpshufb RTMP4, RX1, RTMP3; \
	    vpxor RTMP3, r0, r0; /* r0 ^ x ^ rol(x,24) */ \
	    vpslld $2, RTMP2, RTMP3; \
	    vpsrld $30, RTMP2, RTMP2; \
	    vpxor RTMP2, r0, r0;  \
	    vpxor RTMP3, r0, r0; /* r0 ^ x ^ rol(x,2) ^ rol(x,10) ^ rol(x,18) ^ rol(x,24) */

	leaq (32*4)(%rdi), %rax;
.align 16
.Lroundloop_blk8:
	ROUND(0, RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3);
	ROUND(1, RA1, RA2, RA3, RA0, RB1, RB2, RB3, RB0);
	ROUND(2, RA2, RA3, RA0, RA1, RB2, RB3, RB0, RB1);
	ROUND(3, RA3, RA0, RA1, RA2, RB3, RB0, RB1, RB2);
	leaq (4*4)(%rdi), %rdi;
	cmpq %rax, %rdi;
	jne .Lroundloop_blk8;

#undef ROUND

	vmovdqa .Lbswap128_mask rRIP, RTMP2;

	transpose_4x4(RA0, RA1, RA2, RA3, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RTMP0, RTMP1);
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;
	vpshufb RTMP2, RB0, RB0;
	vpshufb RTMP2, RB1, RB1;
	vpshufb RTMP2, RB2, RB2;
	vpshufb RTMP2, RB3, RB3;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __sm4_crypt_blk8,.-__sm4_crypt_blk8;)

.align 8
.globl _gcry_sm4_aesni_avx_crypt_blk1_8
ELF(.type   _gcry_sm4_aesni_avx_crypt_blk1_8,@function;)
_gcry_sm4_aesni_avx_crypt_blk1_8:
	/* input:
	 *	%rdi: round key array, CTX
	 *	%rsi: dst (1..8 blocks)
	 *	%rdx: src (1..8 blocks)
	 *	%rcx: num blocks (1..8)
	 */
	CFI_STARTPROC();

	cmpq $5, %rcx;
	jb sm4_aesni_avx_crypt_blk1_4;
	vmovdqu (0 * 16)(%rdx), RA0;
	vmovdqu (1 * 16)(%rdx), RA1;
	vmovdqu (2 * 16)(%rdx), RA2;
	vmovdqu (3 * 16)(%rdx), RA3;
	vmovdqu (4 * 16)(%rdx), RB0;
	vmovdqa RB0, RB1;
	vmovdqa RB0, RB2;
	vmovdqa RB0, RB3;
	je .Lblk8_load_input_done;
	vmovdqu (5 * 16)(%rdx), RB1;
	cmpq $7, %rcx;
	jb .Lblk8_load_input_done;
	vmovdqu (6 * 16)(%rdx), RB2;
	je .Lblk8_load_input_done;
	vmovdqu (7 * 16)(%rdx), RB3;

.Lblk8_load_input_done:
	call __sm4_crypt_blk8;

	cmpq $6, %rcx;
	vmovdqu RA0, (0 * 16)(%rsi);
	vmovdqu RA1, (1 * 16)(%rsi);
	vmovdqu RA2, (2 * 16)(%rsi);
	vmovdqu RA3, (3 * 16)(%rsi);
	vmovdqu RB0, (4 * 16)(%rsi);
	jb .Lblk8_store_output_done;
	vmovdqu RB1, (5 * 16)(%rsi);
	je .Lblk8_store_output_done;
	vmovdqu RB2, (6 * 16)(%rsi);
	cmpq $7, %rcx;
	je .Lblk8_store_output_done;
	vmovdqu RB3, (7 * 16)(%rsi);

.Lblk8_store_output_done:
	vzeroall;
	xorl %eax, %eax;
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_crypt_blk1_8,.-_gcry_sm4_aesni_avx_crypt_blk1_8;)

.align 8
.globl _gcry_sm4_aesni_avx_ctr_enc
ELF(.type   _gcry_sm4_aesni_avx_ctr_enc,@function;)
_gcry_sm4_aesni_avx_ctr_enc:
	/* input:
	 *	%rdi: round key array, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();

	/* load IV and byteswap */
	vmovdqu (%rcx), RA0;

	vmovdqa .Lbswap128_mask rRIP, RBSWAP;
	vpshufb RBSWAP, RA0, RTMP0; /* be => le */

	vpcmpeqd RNOT, RNOT, RNOT;
	vpsrldq $8, RNOT, RNOT; /* low: -1, high: 0 */

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

	/* construct IVs */
	inc_le128(RTMP0, RNOT, RTMP2); /* +1 */
	vpshufb RBSWAP, RTMP0, RA1;
	inc_le128(RTMP0, RNOT, RTMP2); /* +2 */
	vpshufb RBSWAP, RTMP0, RA2;
	inc_le128(RTMP0, RNOT, RTMP2); /* +3 */
	vpshufb RBSWAP, RTMP0, RA3;
	inc_le128(RTMP0, RNOT, RTMP2); /* +4 */
	vpshufb RBSWAP, RTMP0, RB0;
	inc_le128(RTMP0, RNOT, RTMP2); /* +5 */
	vpshufb RBSWAP, RTMP0, RB1;
	inc_le128(RTMP0, RNOT, RTMP2); /* +6 */
	vpshufb RBSWAP, RTMP0, RB2;
	inc_le128(RTMP0, RNOT, RTMP2); /* +7 */
	vpshufb RBSWAP, RTMP0, RB3;
	inc_le128(RTMP0, RNOT, RTMP2); /* +8 */
	vpshufb RBSWAP, RTMP0, RTMP1;

	/* store new IV */
	vmovdqu RTMP1, (%rcx);

	call __sm4_crypt_blk8;

	vpxor (0 * 16)(%rdx), RA0, RA0;
	vpxor (1 * 16)(%rdx), RA1, RA1;
	vpxor (2 * 16)(%rdx), RA2, RA2;
	vpxor (3 * 16)(%rdx), RA3, RA3;
	vpxor (4 * 16)(%rdx), RB0, RB0;
	vpxor (5 * 16)(%rdx), RB1, RB1;
	vpxor (6 * 16)(%rdx), RB2, RB2;
	vpxor (7 * 16)(%rdx), RB3, RB3;

	vmovdqu RA0, (0 * 16)(%rsi);
	vmovdqu RA1, (1 * 16)(%rsi);
	vmovdqu RA2, (2 * 16)(%rsi);
	vmovdqu RA3, (3 * 16)(%rsi);
	vmovdqu RB0, (4 * 16)(%rsi);
	vmovdqu RB1, (5 * 16)(%rsi);
	vmovdqu RB2, (6 * 16)(%rsi);
	vmovdqu RB3, (7 * 16)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_ctr_enc,.-_gcry_sm4_aesni_avx_ctr_enc;)

.align 8
.globl _gcry_sm4_aesni_avx_cbc_dec
ELF(.type   _gcry_sm4_aesni_avx_cbc_dec,@function;)
_gcry_sm4_aesni_avx_cbc_dec:
	/* input:
	 *	%rdi: round key array, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	vmovdqu (0 * 16)(%rdx), RA0;
	vmovdqu (1 * 16)(%rdx), RA1;
	vmovdqu (2 * 16)(%rdx), RA2;
	vmovdqu (3 * 16)(%rdx), RA3;
	vmovdqu (4 * 16)(%rdx), RB0;
	vmovdqu (5 * 16)(%rdx), RB1;
	vmovdqu (6 * 16)(%rdx), RB2;
	vmovdqu (7 * 16)(%rdx), RB3;

	call __sm4_crypt_blk8;

	vmovdqu (7 * 16)(%rdx), RNOT;
	vpxor (%rcx), RA0, RA0;
	vpxor (0 * 16)(%rdx), RA1, RA1;
	vpxor (1 * 16)(%rdx), RA2, RA2;
	vpxor (2 * 16)(%rdx), RA3, RA3;
	vpxor (3 * 16)(%rdx), RB0, RB0;
	vpxor (4 * 16)(%rdx), RB1, RB1;
	vpxor (5 * 16)(%rdx), RB2, RB2;
	vpxor (6 * 16)(%rdx), RB3, RB3;
	vmovdqu RNOT, (%rcx); /* store new IV */

	vmovdqu RA0, (0 * 16)(%rsi);
	vmovdqu RA1, (1 * 16)(%rsi);
	vmovdqu RA2, (2 * 16)(%rsi);
	vmovdqu RA3, (3 * 16)(%rsi);
	vmovdqu RB0, (4 * 16)(%rsi);
	vmovdqu RB1, (5 * 16)(%rsi);
	vmovdqu RB2, (6 * 16)(%rsi);
	vmovdqu RB3, (7 * 16)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_cbc_dec,.-_gcry_sm4_aesni_avx_cbc_dec;)

.align 8
.globl _gcry_sm4_aesni_avx_cfb_dec
ELF(.type   _gcry_sm4_aesni_avx_cfb_dec,@function;)
_gcry_sm4_aesni_avx_cfb_dec:
	/* input:
	 *	%rdi: round key array, CTX
	 *	%rsi: dst (8 blocks)
	 *	%rdx: src (8 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	/* Load input */
	vmovdqu (%rcx), RA0;
	vmovdqu 0 * 16(%rdx), RA1;
	vmovdqu 1 * 16(%rdx), RA2;
	vmovdqu 2 * 16(%rdx), RA3;
	vmovdqu 3 * 16(%rdx), RB0;
	vmovdqu 4 * 16(%rdx), RB1;
	vmovdqu 5 * 16(%rdx), RB2;
	vmovdqu 6 * 16(%rdx), RB3;

	/* Update IV */
	vmovdqu 7 * 16(%rdx), RNOT;
	vmovdqu RNOT, (%rcx);

	call __sm4_crypt_blk8;

	vpxor (0 * 16)(%rdx), RA0, RA0;
	vpxor (1 * 16)(%rdx), RA1, RA1;
	vpxor (2 * 16)(%rdx), RA2, RA2;
	vpxor (3 * 16)(%rdx), RA3, RA3;
	vpxor (4 * 16)(%rdx), RB0, RB0;
	vpxor (5 * 16)(%rdx), RB1, RB1;
	vpxor (6 * 16)(%rdx), RB2, RB2;
	vpxor (7 * 16)(%rdx), RB3, RB3;

	vmovdqu RA0, (0 * 16)(%rsi);
	vmovdqu RA1, (1 * 16)(%rsi);
	vmovdqu RA2, (2 * 16)(%rsi);
	vmovdqu RA3, (3 * 16)(%rsi);
	vmovdqu RB0, (4 * 16)(%rsi);
	vmovdqu RB1, (5 * 16)(%rsi);
	vmovdqu RB2, (6 * 16)(%rsi);
	vmovdqu RB3, (7 * 16)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_cfb_dec,.-_gcry_sm4_aesni_avx_cfb_dec;)

.align 8
.globl _gcry_sm4_aesni_avx_ocb_enc
ELF(.type _gcry_sm4_aesni_avx_ocb_enc,@function;)

_gcry_sm4_aesni_avx_ocb_enc:
	/* input:
	 *	%rdi: round key array, CTX
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

	vmovdqu (%rcx), RTMP0;
	vmovdqu (%r8), RTMP1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  vmovdqu (n * 16)(%rdx), xreg; \
	  vpxor (lreg), RTMP0, RTMP0; \
	  vpxor xreg, RTMP1, RTMP1; \
	  vpxor RTMP0, xreg, xreg; \
	  vmovdqu RTMP0, (n * 16)(%rsi);
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

	vmovdqu RTMP0, (%rcx);
	vmovdqu RTMP1, (%r8);

	movq (0 * 8)(%rsp), %r10;
	CFI_RESTORE(%r10);
	movq (1 * 8)(%rsp), %r11;
	CFI_RESTORE(%r11);
	movq (2 * 8)(%rsp), %r12;
	CFI_RESTORE(%r12);
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r13);

	call __sm4_crypt_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vpxor (0 * 16)(%rsi), RA0, RA0;
	vpxor (1 * 16)(%rsi), RA1, RA1;
	vpxor (2 * 16)(%rsi), RA2, RA2;
	vpxor (3 * 16)(%rsi), RA3, RA3;
	vpxor (4 * 16)(%rsi), RB0, RB0;
	vpxor (5 * 16)(%rsi), RB1, RB1;
	vpxor (6 * 16)(%rsi), RB2, RB2;
	vpxor (7 * 16)(%rsi), RB3, RB3;

	vmovdqu RA0, (0 * 16)(%rsi);
	vmovdqu RA1, (1 * 16)(%rsi);
	vmovdqu RA2, (2 * 16)(%rsi);
	vmovdqu RA3, (3 * 16)(%rsi);
	vmovdqu RB0, (4 * 16)(%rsi);
	vmovdqu RB1, (5 * 16)(%rsi);
	vmovdqu RB2, (6 * 16)(%rsi);
	vmovdqu RB3, (7 * 16)(%rsi);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_ocb_enc,.-_gcry_sm4_aesni_avx_ocb_enc;)

.align 8
.globl _gcry_sm4_aesni_avx_ocb_dec
ELF(.type _gcry_sm4_aesni_avx_ocb_dec,@function;)

_gcry_sm4_aesni_avx_ocb_dec:
	/* input:
	 *	%rdi: round key array, CTX
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
	  vmovdqu (n * 16)(%rdx), xreg; \
	  vpxor (lreg), RTMP0, RTMP0; \
	  vpxor RTMP0, xreg, xreg; \
	  vmovdqu RTMP0, (n * 16)(%rsi);
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

	vmovdqu RTMP0, (%rcx);

	movq (0 * 8)(%rsp), %r10;
	CFI_RESTORE(%r10);
	movq (1 * 8)(%rsp), %r11;
	CFI_RESTORE(%r11);
	movq (2 * 8)(%rsp), %r12;
	CFI_RESTORE(%r12);
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r13);

	call __sm4_crypt_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vmovdqu (%r8), RTMP0;

	vpxor (0 * 16)(%rsi), RA0, RA0;
	vpxor (1 * 16)(%rsi), RA1, RA1;
	vpxor (2 * 16)(%rsi), RA2, RA2;
	vpxor (3 * 16)(%rsi), RA3, RA3;
	vpxor (4 * 16)(%rsi), RB0, RB0;
	vpxor (5 * 16)(%rsi), RB1, RB1;
	vpxor (6 * 16)(%rsi), RB2, RB2;
	vpxor (7 * 16)(%rsi), RB3, RB3;

	/* Checksum_i = Checksum_{i-1} xor P_i  */

	vmovdqu RA0, (0 * 16)(%rsi);
	vpxor RA0, RTMP0, RTMP0;
	vmovdqu RA1, (1 * 16)(%rsi);
	vpxor RA1, RTMP0, RTMP0;
	vmovdqu RA2, (2 * 16)(%rsi);
	vpxor RA2, RTMP0, RTMP0;
	vmovdqu RA3, (3 * 16)(%rsi);
	vpxor RA3, RTMP0, RTMP0;
	vmovdqu RB0, (4 * 16)(%rsi);
	vpxor RB0, RTMP0, RTMP0;
	vmovdqu RB1, (5 * 16)(%rsi);
	vpxor RB1, RTMP0, RTMP0;
	vmovdqu RB2, (6 * 16)(%rsi);
	vpxor RB2, RTMP0, RTMP0;
	vmovdqu RB3, (7 * 16)(%rsi);
	vpxor RB3, RTMP0, RTMP0;

	vmovdqu RTMP0, (%r8);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_ocb_dec,.-_gcry_sm4_aesni_avx_ocb_dec;)

.align 8
.globl _gcry_sm4_aesni_avx_ocb_auth
ELF(.type _gcry_sm4_aesni_avx_ocb_auth,@function;)

_gcry_sm4_aesni_avx_ocb_auth:
	/* input:
	 *	%rdi: round key array, CTX
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

	vmovdqu (%rdx), RTMP0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  vmovdqu (n * 16)(%rsi), xreg; \
	  vpxor (lreg), RTMP0, RTMP0; \
	  vpxor RTMP0, xreg, xreg;
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

	vmovdqu RTMP0, (%rdx);

	movq (0 * 8)(%rsp), %r10;
	CFI_RESTORE(%r10);
	movq (1 * 8)(%rsp), %r11;
	CFI_RESTORE(%r11);
	movq (2 * 8)(%rsp), %r12;
	CFI_RESTORE(%r12);
	movq (3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r13);

	call __sm4_crypt_blk8;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vmovdqu (%rcx), RTMP0;
	vpxor RB0, RA0, RA0;
	vpxor RB1, RA1, RA1;
	vpxor RB2, RA2, RA2;
	vpxor RB3, RA3, RA3;

	vpxor RTMP0, RA3, RA3;
	vpxor RA2, RA0, RA0;
	vpxor RA3, RA1, RA1;

	vpxor RA1, RA0, RA0;
	vmovdqu RA0, (%rcx);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx_ocb_auth,.-_gcry_sm4_aesni_avx_ocb_auth;)

#endif /*defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)*/
#endif /*__x86_64*/
