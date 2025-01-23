/* sm4-avx2-amd64.S  -  AVX2 implementation of SM4 cipher
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
    defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT)

#include "asm-common-amd64.h"

/* vector registers */
#define RX0          %ymm0
#define RX1          %ymm1
#define MASK_4BIT    %ymm2
#define RTMP0        %ymm3
#define RTMP1        %ymm4
#define RTMP2        %ymm5
#define RTMP3        %ymm6
#define RTMP4        %ymm7

#define RA0          %ymm8
#define RA1          %ymm9
#define RA2          %ymm10
#define RA3          %ymm11

#define RB0          %ymm12
#define RB1          %ymm13
#define RB2          %ymm14
#define RB3          %ymm15

#define RNOT         %ymm0
#define RBSWAP       %ymm1

#define RX0x         %xmm0
#define RX1x         %xmm1
#define MASK_4BITx   %xmm2

#define RNOTx        %xmm0
#define RBSWAPx      %xmm1

#define RTMP0x       %xmm3
#define RTMP1x       %xmm4
#define RTMP2x       %xmm5
#define RTMP3x       %xmm6
#define RTMP4x       %xmm7

/**********************************************************************
  helper macros
 **********************************************************************/

/* Transpose four 32-bit words between 128-bit vector lanes. */
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
  16-way SM4 with AES-NI and AVX
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
ELF(.type   __sm4_crypt_blk16,@function;)
__sm4_crypt_blk16:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: sixteen parallel
	 *						plaintext blocks
	 * output:
	 *	RA0, RA1, RA2, RA3, RB0, RB1, RB2, RB3: sixteen parallel
	 * 						ciphertext blocks
	 */
	CFI_STARTPROC();

	vbroadcasti128 .Lbswap32_mask rRIP, RTMP2;
	vpshufb RTMP2, RA0, RA0;
	vpshufb RTMP2, RA1, RA1;
	vpshufb RTMP2, RA2, RA2;
	vpshufb RTMP2, RA3, RA3;
	vpshufb RTMP2, RB0, RB0;
	vpshufb RTMP2, RB1, RB1;
	vpshufb RTMP2, RB2, RB2;
	vpshufb RTMP2, RB3, RB3;

	vpbroadcastd .L0f0f0f0f rRIP, MASK_4BIT;
	transpose_4x4(RA0, RA1, RA2, RA3, RTMP0, RTMP1);
	transpose_4x4(RB0, RB1, RB2, RB3, RTMP0, RTMP1);

#define ROUND(round, s0, s1, s2, s3, r0, r1, r2, r3) \
	vpbroadcastd (4*(round))(%rdi), RX0; \
	vbroadcasti128 .Lpre_tf_lo_s rRIP, RTMP4; \
	vbroadcasti128 .Lpre_tf_hi_s rRIP, RTMP1; \
	vmovdqa RX0, RX1; \
	vpxor s1, RX0, RX0; \
	vpxor s2, RX0, RX0; \
	vpxor s3, RX0, RX0; /* s1 ^ s2 ^ s3 ^ rk */ \
	    vbroadcasti128 .Lpost_tf_lo_s rRIP, RTMP2; \
	    vbroadcasti128 .Lpost_tf_hi_s rRIP, RTMP3; \
	    vpxor r1, RX1, RX1; \
	    vpxor r2, RX1, RX1; \
	    vpxor r3, RX1, RX1; /* r1 ^ r2 ^ r3 ^ rk */ \
	\
	/* sbox, non-linear part */ \
	transform_pre(RX0, RTMP4, RTMP1, MASK_4BIT, RTMP0); \
	    transform_pre(RX1, RTMP4, RTMP1, MASK_4BIT, RTMP0); \
	vextracti128 $1, RX0, RTMP4x; \
	    vextracti128 $1, RX1, RTMP0x; \
	vaesenclast MASK_4BITx, RX0x, RX0x; \
	vaesenclast MASK_4BITx, RTMP4x, RTMP4x; \
	    vaesenclast MASK_4BITx, RX1x, RX1x; \
	    vaesenclast MASK_4BITx, RTMP0x, RTMP0x; \
	vinserti128 $1, RTMP4x, RX0, RX0; \
	vbroadcasti128 .Linv_shift_row rRIP, RTMP4; \
	    vinserti128 $1, RTMP0x, RX1, RX1; \
	transform_post(RX0, RTMP2, RTMP3, MASK_4BIT, RTMP0); \
	    transform_post(RX1, RTMP2, RTMP3, MASK_4BIT, RTMP0); \
	\
	/* linear part */ \
	vpshufb RTMP4, RX0, RTMP0; \
	vpxor RTMP0, s0, s0; /* s0 ^ x */ \
	    vpshufb RTMP4, RX1, RTMP2; \
	    vbroadcasti128 .Linv_shift_row_rol_8 rRIP, RTMP4; \
	    vpxor RTMP2, r0, r0; /* r0 ^ x */ \
	vpshufb RTMP4, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) */ \
	    vpshufb RTMP4, RX1, RTMP3; \
	    vbroadcasti128 .Linv_shift_row_rol_16 rRIP, RTMP4; \
	    vpxor RTMP3, RTMP2, RTMP2; /* x ^ rol(x,8) */ \
	vpshufb RTMP4, RX0, RTMP1; \
	vpxor RTMP1, RTMP0, RTMP0; /* x ^ rol(x,8) ^ rol(x,16) */ \
	    vpshufb RTMP4, RX1, RTMP3; \
	    vbroadcasti128 .Linv_shift_row_rol_24 rRIP, RTMP4; \
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

	vbroadcasti128 .Lbswap128_mask rRIP, RTMP2;

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
ELF(.size __sm4_crypt_blk16,.-__sm4_crypt_blk16;)

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

.align 8
.globl _gcry_sm4_aesni_avx2_ctr_enc
ELF(.type   _gcry_sm4_aesni_avx2_ctr_enc,@function;)
_gcry_sm4_aesni_avx2_ctr_enc:
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

	call __sm4_crypt_blk16;

	vpxor (0 * 32)(%rdx), RA0, RA0;
	vpxor (1 * 32)(%rdx), RA1, RA1;
	vpxor (2 * 32)(%rdx), RA2, RA2;
	vpxor (3 * 32)(%rdx), RA3, RA3;
	vpxor (4 * 32)(%rdx), RB0, RB0;
	vpxor (5 * 32)(%rdx), RB1, RB1;
	vpxor (6 * 32)(%rdx), RB2, RB2;
	vpxor (7 * 32)(%rdx), RB3, RB3;

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
ELF(.size _gcry_sm4_aesni_avx2_ctr_enc,.-_gcry_sm4_aesni_avx2_ctr_enc;)

.align 8
.globl _gcry_sm4_aesni_avx2_cbc_dec
ELF(.type   _gcry_sm4_aesni_avx2_cbc_dec,@function;)
_gcry_sm4_aesni_avx2_cbc_dec:
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

	call __sm4_crypt_blk16;

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
ELF(.size _gcry_sm4_aesni_avx2_cbc_dec,.-_gcry_sm4_aesni_avx2_cbc_dec;)

.align 8
.globl _gcry_sm4_aesni_avx2_cfb_dec
ELF(.type   _gcry_sm4_aesni_avx2_cfb_dec,@function;)
_gcry_sm4_aesni_avx2_cfb_dec:
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

	call __sm4_crypt_blk16;

	vpxor (0 * 32)(%rdx), RA0, RA0;
	vpxor (1 * 32)(%rdx), RA1, RA1;
	vpxor (2 * 32)(%rdx), RA2, RA2;
	vpxor (3 * 32)(%rdx), RA3, RA3;
	vpxor (4 * 32)(%rdx), RB0, RB0;
	vpxor (5 * 32)(%rdx), RB1, RB1;
	vpxor (6 * 32)(%rdx), RB2, RB2;
	vpxor (7 * 32)(%rdx), RB3, RB3;

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
ELF(.size _gcry_sm4_aesni_avx2_cfb_dec,.-_gcry_sm4_aesni_avx2_cfb_dec;)

.align 8
.globl _gcry_sm4_aesni_avx2_ocb_enc
ELF(.type _gcry_sm4_aesni_avx2_ocb_enc,@function;)

_gcry_sm4_aesni_avx2_ocb_enc:
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

	call __sm4_crypt_blk16;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vpxor (0 * 32)(%rsi), RA0, RA0;
	vpxor (1 * 32)(%rsi), RA1, RA1;
	vpxor (2 * 32)(%rsi), RA2, RA2;
	vpxor (3 * 32)(%rsi), RA3, RA3;
	vpxor (4 * 32)(%rsi), RB0, RB0;
	vpxor (5 * 32)(%rsi), RB1, RB1;
	vpxor (6 * 32)(%rsi), RB2, RB2;
	vpxor (7 * 32)(%rsi), RB3, RB3;

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
ELF(.size _gcry_sm4_aesni_avx2_ocb_enc,.-_gcry_sm4_aesni_avx2_ocb_enc;)

.align 8
.globl _gcry_sm4_aesni_avx2_ocb_dec
ELF(.type _gcry_sm4_aesni_avx2_ocb_dec,@function;)

_gcry_sm4_aesni_avx2_ocb_dec:
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

	call __sm4_crypt_blk16;

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
ELF(.size _gcry_sm4_aesni_avx2_ocb_dec,.-_gcry_sm4_aesni_avx2_ocb_dec;)

.align 8
.globl _gcry_sm4_aesni_avx2_ocb_auth
ELF(.type _gcry_sm4_aesni_avx2_ocb_auth,@function;)

_gcry_sm4_aesni_avx2_ocb_auth:
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

	call __sm4_crypt_blk16;

	addq $(4 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-4 * 8);

	vpxor RA0, RB0, RA0;
	vpxor RA1, RB1, RA1;
	vpxor RA2, RB2, RA2;
	vpxor RA3, RB3, RA3;

	vpxor RA1, RA0, RA0;
	vpxor RA3, RA2, RA2;

	vpxor RA2, RA0, RTMP1;

	vextracti128 $1, RTMP1, RNOTx;
	vpxor (%rcx), RTMP1x, RTMP1x;
	vpxor RNOTx, RTMP1x, RTMP1x;
	vmovdqu RTMP1x, (%rcx);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_sm4_aesni_avx2_ocb_auth,.-_gcry_sm4_aesni_avx2_ocb_auth;)

#endif /*defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)*/
#endif /*__x86_64*/
