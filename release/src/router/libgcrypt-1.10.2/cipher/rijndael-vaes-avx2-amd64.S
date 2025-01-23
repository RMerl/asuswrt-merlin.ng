/* VAES/AVX2 AMD64 accelerated AES for Libgcrypt
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if defined(__x86_64__)
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT) && \
    defined(HAVE_GCC_INLINE_ASM_VAES_VPCLMUL)

#include "asm-common-amd64.h"

.text

/**********************************************************************
  helper macros
 **********************************************************************/
#define no(...) /*_*/
#define yes(...) __VA_ARGS__

#define AES_OP8(op, key, b0, b1, b2, b3, b4, b5, b6, b7) \
	op key, b0, b0; \
	op key, b1, b1; \
	op key, b2, b2; \
	op key, b3, b3; \
	op key, b4, b4; \
	op key, b5, b5; \
	op key, b6, b6; \
	op key, b7, b7;

#define VAESENC8(key, b0, b1, b2, b3, b4, b5, b6, b7) \
	AES_OP8(vaesenc, key, b0, b1, b2, b3, b4, b5, b6, b7)

#define VAESDEC8(key, b0, b1, b2, b3, b4, b5, b6, b7) \
	AES_OP8(vaesdec, key, b0, b1, b2, b3, b4, b5, b6, b7)

#define XOR8(key, b0, b1, b2, b3, b4, b5, b6, b7) \
	AES_OP8(vpxor, key, b0, b1, b2, b3, b4, b5, b6, b7)

#define AES_OP4(op, key, b0, b1, b2, b3) \
	op key, b0, b0; \
	op key, b1, b1; \
	op key, b2, b2; \
	op key, b3, b3;

#define VAESENC4(key, b0, b1, b2, b3) \
	AES_OP4(vaesenc, key, b0, b1, b2, b3)

#define VAESDEC4(key, b0, b1, b2, b3) \
	AES_OP4(vaesdec, key, b0, b1, b2, b3)

#define XOR4(key, b0, b1, b2, b3) \
	AES_OP4(vpxor, key, b0, b1, b2, b3)

#define AES_OP2(op, key, b0, b1) \
	op key, b0, b0; \
	op key, b1, b1;

#define VAESENC2(key, b0, b1) \
	AES_OP2(vaesenc, key, b0, b1)

#define VAESDEC2(key, b0, b1) \
	AES_OP2(vaesdec, key, b0, b1)

#define XOR2(key, b0, b1) \
	AES_OP2(vpxor, key, b0, b1)

/**********************************************************************
  CBC-mode decryption
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_cbc_dec_amd64,@function)
.globl _gcry_vaes_avx2_cbc_dec_amd64
_gcry_vaes_avx2_cbc_dec_amd64:
	/* input:
	 *	%rdi: round keys
	 *	%rsi: iv
	 *	%rdx: dst
	 *	%rcx: src
	 *	%r8:  nblocks
	 *	%r9:  nrounds
	 */
	CFI_STARTPROC();

	/* Load IV. */
	vmovdqu (%rsi), %xmm15;

	/* Process 16 blocks per loop. */
.align 8
.Lcbc_dec_blk16:
	cmpq $16, %r8;
	jb .Lcbc_dec_blk8;

	leaq -16(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm8;
	vmovdqu (0 * 16)(%rcx), %ymm0;
	vmovdqu (2 * 16)(%rcx), %ymm1;
	vmovdqu (4 * 16)(%rcx), %ymm2;
	vmovdqu (6 * 16)(%rcx), %ymm3;
	vmovdqu (8 * 16)(%rcx), %ymm4;
	vmovdqu (10 * 16)(%rcx), %ymm5;
	vmovdqu (12 * 16)(%rcx), %ymm6;
	vmovdqu (14 * 16)(%rcx), %ymm7;
	vpxor %ymm8, %ymm0, %ymm0;
	vpxor %ymm8, %ymm1, %ymm1;
	vpxor %ymm8, %ymm2, %ymm2;
	vpxor %ymm8, %ymm3, %ymm3;
	vpxor %ymm8, %ymm4, %ymm4;
	vpxor %ymm8, %ymm5, %ymm5;
	vpxor %ymm8, %ymm6, %ymm6;
	vpxor %ymm8, %ymm7, %ymm7;
	vbroadcasti128 (1 * 16)(%rdi), %ymm8;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm9;
	vmovdqu (1 * 16)(%rcx), %ymm10;
	vmovdqu (3 * 16)(%rcx), %ymm11;
	vmovdqu (5 * 16)(%rcx), %ymm12;
	vmovdqu (7 * 16)(%rcx), %ymm13;
	vmovdqu (9 * 16)(%rcx), %ymm14;
	vmovdqu (15 * 16)(%rcx), %xmm15;
	leaq (16 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (2 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (3 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (4 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (5 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (6 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (7 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (8 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (9 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (10 * 16)(%rdi), %ymm8;
	cmpl $12, %r9d;
	jb .Lcbc_dec_blk16_last;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (11 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (12 * 16)(%rdi), %ymm8;
	jz .Lcbc_dec_blk16_last;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (13 * 16)(%rdi), %ymm8;
	VAESDEC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (14 * 16)(%rdi), %ymm8;

	/* Last round and output handling. */
  .Lcbc_dec_blk16_last:
	vpxor %ymm8, %ymm9, %ymm9;
	vpxor %ymm8, %ymm10, %ymm10;
	vpxor %ymm8, %ymm11, %ymm11;
	vpxor %ymm8, %ymm12, %ymm12;
	vpxor %ymm8, %ymm13, %ymm13;
	vpxor %ymm8, %ymm14, %ymm14;
	vaesdeclast %ymm9, %ymm0, %ymm0;
	vaesdeclast %ymm10, %ymm1, %ymm1;
	vpxor (-5 * 16)(%rcx), %ymm8, %ymm9;
	vpxor (-3 * 16)(%rcx), %ymm8, %ymm10;
	vaesdeclast %ymm11, %ymm2, %ymm2;
	vaesdeclast %ymm12, %ymm3, %ymm3;
	vaesdeclast %ymm13, %ymm4, %ymm4;
	vaesdeclast %ymm14, %ymm5, %ymm5;
	vaesdeclast %ymm9, %ymm6, %ymm6;
	vaesdeclast %ymm10, %ymm7, %ymm7;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	vmovdqu %ymm4, (8 * 16)(%rdx);
	vmovdqu %ymm5, (10 * 16)(%rdx);
	vmovdqu %ymm6, (12 * 16)(%rdx);
	vmovdqu %ymm7, (14 * 16)(%rdx);
	leaq (16 * 16)(%rdx), %rdx;

	jmp .Lcbc_dec_blk16;

	/* Handle trailing eight blocks. */
.align 8
.Lcbc_dec_blk8:
	cmpq $8, %r8;
	jb .Lcbc_dec_blk4;

	leaq -8(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vmovdqu (0 * 16)(%rcx), %ymm0;
	vmovdqu (2 * 16)(%rcx), %ymm1;
	vmovdqu (4 * 16)(%rcx), %ymm2;
	vmovdqu (6 * 16)(%rcx), %ymm3;
	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm4, %ymm1, %ymm1;
	vpxor %ymm4, %ymm2, %ymm2;
	vpxor %ymm4, %ymm3, %ymm3;
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm10;
	vmovdqu (1 * 16)(%rcx), %ymm11;
	vmovdqu (3 * 16)(%rcx), %ymm12;
	vmovdqu (5 * 16)(%rcx), %ymm13;
	vmovdqu (7 * 16)(%rcx), %xmm15;
	leaq (8 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lcbc_dec_blk8_last;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lcbc_dec_blk8_last;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lcbc_dec_blk8_last:
	vpxor %ymm4, %ymm10, %ymm10;
	vpxor %ymm4, %ymm11, %ymm11;
	vpxor %ymm4, %ymm12, %ymm12;
	vpxor %ymm4, %ymm13, %ymm13;
	vaesdeclast %ymm10, %ymm0, %ymm0;
	vaesdeclast %ymm11, %ymm1, %ymm1;
	vaesdeclast %ymm12, %ymm2, %ymm2;
	vaesdeclast %ymm13, %ymm3, %ymm3;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	leaq (8 * 16)(%rdx), %rdx;

	/* Handle trailing four blocks. */
.align 8
.Lcbc_dec_blk4:
	cmpq $4, %r8;
	jb .Lcbc_dec_blk1;

	leaq -4(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vmovdqu (0 * 16)(%rcx), %ymm0;
	vmovdqu (2 * 16)(%rcx), %ymm1;
	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm4, %ymm1, %ymm1;
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm10;
	vmovdqu (1 * 16)(%rcx), %ymm11;
	vmovdqu (3 * 16)(%rcx), %xmm15;
	leaq (4 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lcbc_dec_blk4_last;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lcbc_dec_blk4_last;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESDEC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lcbc_dec_blk4_last:
	vpxor %ymm4, %ymm10, %ymm10;
	vpxor %ymm4, %ymm11, %ymm11;
	vaesdeclast %ymm10, %ymm0, %ymm0;
	vaesdeclast %ymm11, %ymm1, %ymm1;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	leaq (4 * 16)(%rdx), %rdx;

	/* Process trailing one to three blocks, one per loop. */
.align 8
.Lcbc_dec_blk1:
	cmpq $1, %r8;
	jb .Ldone_cbc_dec;

	leaq -1(%r8), %r8;

	/* Load input. */
	vmovdqu (%rcx), %xmm2;
	leaq 16(%rcx), %rcx;

	/* Xor first key. */
	vpxor (0 * 16)(%rdi), %xmm2, %xmm0;

	/* AES rounds. */
	vaesdec (1 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (2 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (3 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (4 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (5 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (6 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (7 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (8 * 16)(%rdi), %xmm0, %xmm0;
	vaesdec (9 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (10 * 16)(%rdi), %xmm1;
	cmpl $12, %r9d;
	jb .Lcbc_dec_blk1_last;
	vaesdec %xmm1, %xmm0, %xmm0;
	vaesdec (11 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (12 * 16)(%rdi), %xmm1;
	jz .Lcbc_dec_blk1_last;
	vaesdec %xmm1, %xmm0, %xmm0;
	vaesdec (13 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (14 * 16)(%rdi), %xmm1;

	/* Last round and output handling. */
  .Lcbc_dec_blk1_last:
	vpxor %xmm1, %xmm15, %xmm15;
	vaesdeclast %xmm15, %xmm0, %xmm0;
	vmovdqa %xmm2, %xmm15;
	vmovdqu %xmm0, (%rdx);
	leaq 16(%rdx), %rdx;

	jmp .Lcbc_dec_blk1;

.align 8
.Ldone_cbc_dec:
	/* Store IV. */
	vmovdqu %xmm15, (%rsi);

	vzeroall;
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_cbc_dec_amd64,.-_gcry_vaes_avx2_cbc_dec_amd64)

/**********************************************************************
  CFB-mode decryption
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_cfb_dec_amd64,@function)
.globl _gcry_vaes_avx2_cfb_dec_amd64
_gcry_vaes_avx2_cfb_dec_amd64:
	/* input:
	 *	%rdi: round keys
	 *	%rsi: iv
	 *	%rdx: dst
	 *	%rcx: src
	 *	%r8:  nblocks
	 *	%r9:  nrounds
	 */
	CFI_STARTPROC();

	/* Load IV. */
	vmovdqu (%rsi), %xmm15;

	/* Process 16 blocks per loop. */
.align 8
.Lcfb_dec_blk16:
	cmpq $16, %r8;
	jb .Lcfb_dec_blk8;

	leaq -16(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm8;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm0;
	vmovdqu (1 * 16)(%rcx), %ymm1;
	vmovdqu (3 * 16)(%rcx), %ymm2;
	vmovdqu (5 * 16)(%rcx), %ymm3;
	vmovdqu (7 * 16)(%rcx), %ymm4;
	vmovdqu (9 * 16)(%rcx), %ymm5;
	vmovdqu (11 * 16)(%rcx), %ymm6;
	vmovdqu (13 * 16)(%rcx), %ymm7;
	vmovdqu (15 * 16)(%rcx), %xmm15;
	vpxor %ymm8, %ymm0, %ymm0;
	vpxor %ymm8, %ymm1, %ymm1;
	vpxor %ymm8, %ymm2, %ymm2;
	vpxor %ymm8, %ymm3, %ymm3;
	vpxor %ymm8, %ymm4, %ymm4;
	vpxor %ymm8, %ymm5, %ymm5;
	vpxor %ymm8, %ymm6, %ymm6;
	vpxor %ymm8, %ymm7, %ymm7;
	vbroadcasti128 (1 * 16)(%rdi), %ymm8;
	vmovdqu (0 * 16)(%rcx), %ymm9;
	vmovdqu (2 * 16)(%rcx), %ymm10;
	vmovdqu (4 * 16)(%rcx), %ymm11;
	vmovdqu (6 * 16)(%rcx), %ymm12;
	vmovdqu (8 * 16)(%rcx), %ymm13;
	vmovdqu (10 * 16)(%rcx), %ymm14;

	leaq (16 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (2 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (3 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (4 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (5 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (6 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (7 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (8 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (9 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (10 * 16)(%rdi), %ymm8;
	cmpl $12, %r9d;
	jb .Lcfb_dec_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (11 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (12 * 16)(%rdi), %ymm8;
	jz .Lcfb_dec_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (13 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (14 * 16)(%rdi), %ymm8;

	/* Last round and output handling. */
  .Lcfb_dec_blk16_last:
	vpxor %ymm8, %ymm9, %ymm9;
	vpxor %ymm8, %ymm10, %ymm10;
	vpxor %ymm8, %ymm11, %ymm11;
	vpxor %ymm8, %ymm12, %ymm12;
	vpxor %ymm8, %ymm13, %ymm13;
	vpxor %ymm8, %ymm14, %ymm14;
	vaesenclast %ymm9, %ymm0, %ymm0;
	vaesenclast %ymm10, %ymm1, %ymm1;
	vpxor (-4 * 16)(%rcx), %ymm8, %ymm9;
	vpxor (-2 * 16)(%rcx), %ymm8, %ymm10;
	vaesenclast %ymm11, %ymm2, %ymm2;
	vaesenclast %ymm12, %ymm3, %ymm3;
	vaesenclast %ymm13, %ymm4, %ymm4;
	vaesenclast %ymm14, %ymm5, %ymm5;
	vaesenclast %ymm9, %ymm6, %ymm6;
	vaesenclast %ymm10, %ymm7, %ymm7;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	vmovdqu %ymm4, (8 * 16)(%rdx);
	vmovdqu %ymm5, (10 * 16)(%rdx);
	vmovdqu %ymm6, (12 * 16)(%rdx);
	vmovdqu %ymm7, (14 * 16)(%rdx);
	leaq (16 * 16)(%rdx), %rdx;

	jmp .Lcfb_dec_blk16;

	/* Handle trailing eight blocks. */
.align 8
.Lcfb_dec_blk8:
	cmpq $8, %r8;
	jb .Lcfb_dec_blk4;

	leaq -8(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm0;
	vmovdqu (1 * 16)(%rcx), %ymm1;
	vmovdqu (3 * 16)(%rcx), %ymm2;
	vmovdqu (5 * 16)(%rcx), %ymm3;
	vmovdqu (7 * 16)(%rcx), %xmm15;
	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm4, %ymm1, %ymm1;
	vpxor %ymm4, %ymm2, %ymm2;
	vpxor %ymm4, %ymm3, %ymm3;
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	vmovdqu (0 * 16)(%rcx), %ymm10;
	vmovdqu (2 * 16)(%rcx), %ymm11;
	vmovdqu (4 * 16)(%rcx), %ymm12;
	vmovdqu (6 * 16)(%rcx), %ymm13;

	leaq (8 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lcfb_dec_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lcfb_dec_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lcfb_dec_blk8_last:
	vpxor %ymm4, %ymm10, %ymm10;
	vpxor %ymm4, %ymm11, %ymm11;
	vpxor %ymm4, %ymm12, %ymm12;
	vpxor %ymm4, %ymm13, %ymm13;
	vaesenclast %ymm10, %ymm0, %ymm0;
	vaesenclast %ymm11, %ymm1, %ymm1;
	vaesenclast %ymm12, %ymm2, %ymm2;
	vaesenclast %ymm13, %ymm3, %ymm3;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	leaq (8 * 16)(%rdx), %rdx;

	/* Handle trailing four blocks. */
.align 8
.Lcfb_dec_blk4:
	cmpq $4, %r8;
	jb .Lcfb_dec_blk1;

	leaq -4(%r8), %r8;

	/* Load input and xor first key. Update IV. */
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vinserti128 $1, (0 * 16)(%rcx), %ymm15, %ymm0;
	vmovdqu (1 * 16)(%rcx), %ymm1;
	vmovdqu (3 * 16)(%rcx), %xmm15;
	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm4, %ymm1, %ymm1;
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	vmovdqu (0 * 16)(%rcx), %ymm10;
	vmovdqu (2 * 16)(%rcx), %ymm11;

	leaq (4 * 16)(%rcx), %rcx;

	/* AES rounds */
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lcfb_dec_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lcfb_dec_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lcfb_dec_blk4_last:
	vpxor %ymm4, %ymm10, %ymm10;
	vpxor %ymm4, %ymm11, %ymm11;
	vaesenclast %ymm10, %ymm0, %ymm0;
	vaesenclast %ymm11, %ymm1, %ymm1;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	leaq (4 * 16)(%rdx), %rdx;

	/* Process trailing one to three blocks, one per loop. */
.align 8
.Lcfb_dec_blk1:
	cmpq $1, %r8;
	jb .Ldone_cfb_dec;

	leaq -1(%r8), %r8;

	/* Xor first key. */
	vpxor (0 * 16)(%rdi), %xmm15, %xmm0;

	/* Load input as next IV. */
	vmovdqu (%rcx), %xmm15;
	leaq 16(%rcx), %rcx;

	/* AES rounds. */
	vaesenc (1 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (2 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (3 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (4 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (5 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (6 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (7 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (8 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (9 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (10 * 16)(%rdi), %xmm1;
	cmpl $12, %r9d;
	jb .Lcfb_dec_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (11 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (12 * 16)(%rdi), %xmm1;
	jz .Lcfb_dec_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (13 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (14 * 16)(%rdi), %xmm1;

	/* Last round and output handling. */
  .Lcfb_dec_blk1_last:
	vpxor %xmm15, %xmm1, %xmm1;
	vaesenclast %xmm1, %xmm0, %xmm0;
	vmovdqu %xmm0, (%rdx);
	leaq 16(%rdx), %rdx;

	jmp .Lcfb_dec_blk1;

.align 8
.Ldone_cfb_dec:
	/* Store IV. */
	vmovdqu %xmm15, (%rsi);

	vzeroall;
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_cfb_dec_amd64,.-_gcry_vaes_avx2_cfb_dec_amd64)

/**********************************************************************
  CTR-mode encryption
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_ctr_enc_amd64,@function)
.globl _gcry_vaes_avx2_ctr_enc_amd64
_gcry_vaes_avx2_ctr_enc_amd64:
	/* input:
	 *	%rdi: round keys
	 *	%rsi: counter
	 *	%rdx: dst
	 *	%rcx: src
	 *	%r8:  nblocks
	 *	%r9:  nrounds
	 */
	CFI_STARTPROC();

	movq 8(%rsi), %r10;
	movq 0(%rsi), %r11;
	bswapq %r10;
	bswapq %r11;

	vpcmpeqd %ymm15, %ymm15, %ymm15;
	vpsrldq $8, %ymm15, %ymm15;     // 0:-1
	vpaddq %ymm15, %ymm15, %ymm14;  // 0:-2
	vbroadcasti128 .Lbswap128_mask rRIP, %ymm13;

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

#define add2_le128(x, minus_one, minus_two, tmp1, tmp2) \
	vpcmpeqq minus_one, x, tmp1; \
	vpcmpeqq minus_two, x, tmp2; \
	vpor tmp1, tmp2, tmp2; \
	vpsubq minus_two, x, x; \
	vpslldq $8, tmp2, tmp2; \
	vpsubq tmp2, x, x;

	/* Process 16 blocks per loop. */
.align 8
.Lctr_enc_blk16:
	cmpq $16, %r8;
	jb .Lctr_enc_blk8;

	leaq -16(%r8), %r8;

	vbroadcasti128 (%rsi), %ymm7;
	vbroadcasti128 (0 * 16)(%rdi), %ymm8;

	/* detect if carry handling is needed */
	addb $16, 15(%rsi);
	jc .Lctr_enc_blk16_handle_carry;

	/* Increment counters. */
	vpaddb .Lbige_addb_0 rRIP, %ymm7, %ymm0;
	vpaddb .Lbige_addb_2 rRIP, %ymm7, %ymm1;
	vpaddb .Lbige_addb_4 rRIP, %ymm7, %ymm2;
	vpaddb .Lbige_addb_6 rRIP, %ymm7, %ymm3;
	vpaddb .Lbige_addb_8 rRIP, %ymm7, %ymm4;
	vpaddb .Lbige_addb_10 rRIP, %ymm7, %ymm5;
	vpaddb .Lbige_addb_12 rRIP, %ymm7, %ymm6;
	vpaddb .Lbige_addb_14 rRIP, %ymm7, %ymm7;
	leaq 16(%r10), %r10;

  .Lctr_enc_blk16_rounds:
	/* AES rounds */
	XOR8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (1 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (2 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (3 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (4 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (5 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (6 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (7 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (8 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (9 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (10 * 16)(%rdi), %ymm8;
	cmpl $12, %r9d;
	jb .Lctr_enc_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (11 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (12 * 16)(%rdi), %ymm8;
	jz .Lctr_enc_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (13 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (14 * 16)(%rdi), %ymm8;

	/* Last round and output handling. */
  .Lctr_enc_blk16_last:
	vpxor (0 * 16)(%rcx), %ymm8, %ymm9; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm8, %ymm10;
	vpxor (4 * 16)(%rcx), %ymm8, %ymm11;
	vpxor (6 * 16)(%rcx), %ymm8, %ymm12;
	vaesenclast %ymm9, %ymm0, %ymm0;
	vaesenclast %ymm10, %ymm1, %ymm1;
	vaesenclast %ymm11, %ymm2, %ymm2;
	vaesenclast %ymm12, %ymm3, %ymm3;
	vpxor (8 * 16)(%rcx), %ymm8, %ymm9;
	vpxor (10 * 16)(%rcx), %ymm8, %ymm10;
	vpxor (12 * 16)(%rcx), %ymm8, %ymm11;
	vpxor (14 * 16)(%rcx), %ymm8, %ymm8;
	leaq (16 * 16)(%rcx), %rcx;
	vaesenclast %ymm9, %ymm4, %ymm4;
	vaesenclast %ymm10, %ymm5, %ymm5;
	vaesenclast %ymm11, %ymm6, %ymm6;
	vaesenclast %ymm8, %ymm7, %ymm7;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	vmovdqu %ymm4, (8 * 16)(%rdx);
	vmovdqu %ymm5, (10 * 16)(%rdx);
	vmovdqu %ymm6, (12 * 16)(%rdx);
	vmovdqu %ymm7, (14 * 16)(%rdx);
	leaq (16 * 16)(%rdx), %rdx;

	jmp .Lctr_enc_blk16;

  .align 8
  .Lctr_enc_blk16_handle_carry:
	/* Increment counters (handle carry). */
	vpshufb %xmm13, %xmm7, %xmm1; /* be => le */
	vmovdqa %xmm1, %xmm0;
	inc_le128(%xmm1, %xmm15, %xmm5);
	vinserti128 $1, %xmm1, %ymm0, %ymm7; /* ctr: +1:+0 */
	vpshufb %ymm13, %ymm7, %ymm0;
	addq $16, %r10;
	adcq $0, %r11;
	bswapq %r10;
	bswapq %r11;
	movq %r10, 8(%rsi);
	movq %r11, 0(%rsi);
	bswapq %r10;
	bswapq %r11;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +3:+2 */
	vpshufb %ymm13, %ymm7, %ymm1;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +5:+4 */
	vpshufb %ymm13, %ymm7, %ymm2;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +7:+6 */
	vpshufb %ymm13, %ymm7, %ymm3;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +9:+8 */
	vpshufb %ymm13, %ymm7, %ymm4;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +11:+10 */
	vpshufb %ymm13, %ymm7, %ymm5;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +13:+12 */
	vpshufb %ymm13, %ymm7, %ymm6;
	add2_le128(%ymm7, %ymm15, %ymm14, %ymm9, %ymm10); /* ctr: +15:+14 */
	vpshufb %ymm13, %ymm7, %ymm7;

	jmp .Lctr_enc_blk16_rounds;

	/* Handle trailing eight blocks. */
.align 8
.Lctr_enc_blk8:
	cmpq $8, %r8;
	jb .Lctr_enc_blk4;

	leaq -8(%r8), %r8;

	vbroadcasti128 (%rsi), %ymm3;
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;

	/* detect if carry handling is needed */
	addb $8, 15(%rsi);
	jc .Lctr_enc_blk8_handle_carry;

	/* Increment counters. */
	vpaddb .Lbige_addb_0 rRIP, %ymm3, %ymm0;
	vpaddb .Lbige_addb_2 rRIP, %ymm3, %ymm1;
	vpaddb .Lbige_addb_4 rRIP, %ymm3, %ymm2;
	vpaddb .Lbige_addb_6 rRIP, %ymm3, %ymm3;
	leaq 8(%r10), %r10;

  .Lctr_enc_blk8_rounds:
	/* AES rounds */
	XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lctr_enc_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lctr_enc_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lctr_enc_blk8_last:
	vpxor (0 * 16)(%rcx), %ymm4, %ymm5; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm4, %ymm6;
	vpxor (4 * 16)(%rcx), %ymm4, %ymm7;
	vpxor (6 * 16)(%rcx), %ymm4, %ymm4;
	leaq (8 * 16)(%rcx), %rcx;
	vaesenclast %ymm5, %ymm0, %ymm0;
	vaesenclast %ymm6, %ymm1, %ymm1;
	vaesenclast %ymm7, %ymm2, %ymm2;
	vaesenclast %ymm4, %ymm3, %ymm3;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	leaq (8 * 16)(%rdx), %rdx;

	jmp .Lctr_enc_blk4;

  .align 8
  .Lctr_enc_blk8_handle_carry:
	/* Increment counters (handle carry). */
	vpshufb %xmm13, %xmm3, %xmm1; /* be => le */
	vmovdqa %xmm1, %xmm0;
	inc_le128(%xmm1, %xmm15, %xmm5);
	vinserti128 $1, %xmm1, %ymm0, %ymm3; /* ctr: +1:+0 */
	vpshufb %ymm13, %ymm3, %ymm0;
	addq $8, %r10;
	adcq $0, %r11;
	bswapq %r10;
	bswapq %r11;
	movq %r10, 8(%rsi);
	movq %r11, 0(%rsi);
	bswapq %r10;
	bswapq %r11;
	add2_le128(%ymm3, %ymm15, %ymm14, %ymm5, %ymm6); /* ctr: +3:+2 */
	vpshufb %ymm13, %ymm3, %ymm1;
	add2_le128(%ymm3, %ymm15, %ymm14, %ymm5, %ymm6); /* ctr: +5:+4 */
	vpshufb %ymm13, %ymm3, %ymm2;
	add2_le128(%ymm3, %ymm15, %ymm14, %ymm5, %ymm6); /* ctr: +7:+6 */
	vpshufb %ymm13, %ymm3, %ymm3;

	jmp .Lctr_enc_blk8_rounds;

	/* Handle trailing four blocks. */
.align 8
.Lctr_enc_blk4:
	cmpq $4, %r8;
	jb .Lctr_enc_blk1;

	leaq -4(%r8), %r8;

	vbroadcasti128 (%rsi), %ymm3;
	vbroadcasti128 (0 * 16)(%rdi), %ymm4;

	/* detect if carry handling is needed */
	addb $4, 15(%rsi);
	jc .Lctr_enc_blk4_handle_carry;

	/* Increment counters. */
	vpaddb .Lbige_addb_0 rRIP, %ymm3, %ymm0;
	vpaddb .Lbige_addb_2 rRIP, %ymm3, %ymm1;
	leaq 4(%r10), %r10;

  .Lctr_enc_blk4_rounds:
	/* AES rounds */
	XOR2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lctr_enc_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lctr_enc_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lctr_enc_blk4_last:
	vpxor (0 * 16)(%rcx), %ymm4, %ymm5; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm4, %ymm6;
	leaq (4 * 16)(%rcx), %rcx;
	vaesenclast %ymm5, %ymm0, %ymm0;
	vaesenclast %ymm6, %ymm1, %ymm1;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	leaq (4 * 16)(%rdx), %rdx;

	jmp .Lctr_enc_blk1;

  .align 8
  .Lctr_enc_blk4_handle_carry:
	/* Increment counters (handle carry). */
	vpshufb %xmm13, %xmm3, %xmm1; /* be => le */
	vmovdqa %xmm1, %xmm0;
	inc_le128(%xmm1, %xmm15, %xmm5);
	vinserti128 $1, %xmm1, %ymm0, %ymm3; /* ctr: +1:+0 */
	vpshufb %ymm13, %ymm3, %ymm0;
	addq $4, %r10;
	adcq $0, %r11;
	bswapq %r10;
	bswapq %r11;
	movq %r10, 8(%rsi);
	movq %r11, 0(%rsi);
	bswapq %r10;
	bswapq %r11;
	add2_le128(%ymm3, %ymm15, %ymm14, %ymm5, %ymm6); /* ctr: +3:+2 */
	vpshufb %ymm13, %ymm3, %ymm1;

	jmp .Lctr_enc_blk4_rounds;

	/* Process trailing one to three blocks, one per loop. */
.align 8
.Lctr_enc_blk1:
	cmpq $1, %r8;
	jb .Ldone_ctr_enc;

	leaq -1(%r8), %r8;

	/* Load and increament counter. */
	vmovdqu (%rsi), %xmm0;
	addq $1, %r10;
	adcq $0, %r11;
	bswapq %r10;
	bswapq %r11;
	movq %r10, 8(%rsi);
	movq %r11, 0(%rsi);
	bswapq %r10;
	bswapq %r11;

	/* AES rounds. */
	vpxor (0 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (1 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (2 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (3 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (4 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (5 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (6 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (7 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (8 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (9 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (10 * 16)(%rdi), %xmm1;
	cmpl $12, %r9d;
	jb .Lctr_enc_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (11 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (12 * 16)(%rdi), %xmm1;
	jz .Lctr_enc_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (13 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (14 * 16)(%rdi), %xmm1;

	/* Last round and output handling. */
  .Lctr_enc_blk1_last:
	vpxor (%rcx), %xmm1, %xmm1; /* Xor src to last round key. */
	leaq 16(%rcx), %rcx;
	vaesenclast %xmm1, %xmm0, %xmm0; /* Last round and xor with xmm1. */
	vmovdqu %xmm0, (%rdx);
	leaq 16(%rdx), %rdx;

	jmp .Lctr_enc_blk1;

.align 8
.Ldone_ctr_enc:
	vzeroall;
	xorl %r10d, %r10d;
	xorl %r11d, %r11d;
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_ctr_enc_amd64,.-_gcry_vaes_avx2_ctr_enc_amd64)

/**********************************************************************
  Little-endian 32-bit CTR-mode encryption (GCM-SIV)
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_ctr32le_enc_amd64,@function)
.globl _gcry_vaes_avx2_ctr32le_enc_amd64
_gcry_vaes_avx2_ctr32le_enc_amd64:
	/* input:
	 *	%rdi: round keys
	 *	%rsi: counter
	 *	%rdx: dst
	 *	%rcx: src
	 *	%r8:  nblocks
	 *	%r9:  nrounds
	 */
	CFI_STARTPROC();

	vbroadcasti128 (%rsi), %ymm15; // CTR

	/* Process 16 blocks per loop. */
.align 8
.Lctr32le_enc_blk16:
	cmpq $16, %r8;
	jb .Lctr32le_enc_blk8;

	leaq -16(%r8), %r8;

	vbroadcasti128 (0 * 16)(%rdi), %ymm8;

	/* Increment counters. */
	vpaddd .Lle_addd_0 rRIP, %ymm15, %ymm0;
	vpaddd .Lle_addd_2 rRIP, %ymm15, %ymm1;
	vpaddd .Lle_addd_4 rRIP, %ymm15, %ymm2;
	vpaddd .Lle_addd_6 rRIP, %ymm15, %ymm3;
	vpaddd .Lle_addd_8 rRIP, %ymm15, %ymm4;
	vpaddd .Lle_addd_10 rRIP, %ymm15, %ymm5;
	vpaddd .Lle_addd_12 rRIP, %ymm15, %ymm6;
	vpaddd .Lle_addd_14 rRIP, %ymm15, %ymm7;

	vpaddd .Lle_addd_16_2 rRIP, %ymm15, %ymm15;

	/* AES rounds */
	XOR8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (1 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (2 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (3 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (4 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (5 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (6 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (7 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (8 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (9 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (10 * 16)(%rdi), %ymm8;
	cmpl $12, %r9d;
	jb .Lctr32le_enc_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (11 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (12 * 16)(%rdi), %ymm8;
	jz .Lctr32le_enc_blk16_last;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (13 * 16)(%rdi), %ymm8;
	VAESENC8(%ymm8, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
	vbroadcasti128 (14 * 16)(%rdi), %ymm8;

	/* Last round and output handling. */
  .Lctr32le_enc_blk16_last:
	vpxor (0 * 16)(%rcx), %ymm8, %ymm9; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm8, %ymm10;
	vpxor (4 * 16)(%rcx), %ymm8, %ymm11;
	vpxor (6 * 16)(%rcx), %ymm8, %ymm12;
	vaesenclast %ymm9, %ymm0, %ymm0;
	vaesenclast %ymm10, %ymm1, %ymm1;
	vaesenclast %ymm11, %ymm2, %ymm2;
	vaesenclast %ymm12, %ymm3, %ymm3;
	vpxor (8 * 16)(%rcx), %ymm8, %ymm9;
	vpxor (10 * 16)(%rcx), %ymm8, %ymm10;
	vpxor (12 * 16)(%rcx), %ymm8, %ymm11;
	vpxor (14 * 16)(%rcx), %ymm8, %ymm8;
	leaq (16 * 16)(%rcx), %rcx;
	vaesenclast %ymm9, %ymm4, %ymm4;
	vaesenclast %ymm10, %ymm5, %ymm5;
	vaesenclast %ymm11, %ymm6, %ymm6;
	vaesenclast %ymm8, %ymm7, %ymm7;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	vmovdqu %ymm4, (8 * 16)(%rdx);
	vmovdqu %ymm5, (10 * 16)(%rdx);
	vmovdqu %ymm6, (12 * 16)(%rdx);
	vmovdqu %ymm7, (14 * 16)(%rdx);
	leaq (16 * 16)(%rdx), %rdx;

	jmp .Lctr32le_enc_blk16;

	/* Handle trailing eight blocks. */
.align 8
.Lctr32le_enc_blk8:
	cmpq $8, %r8;
	jb .Lctr32le_enc_blk4;

	leaq -8(%r8), %r8;

	vbroadcasti128 (0 * 16)(%rdi), %ymm4;

	/* Increment counters. */
	vpaddd .Lle_addd_0 rRIP, %ymm15, %ymm0;
	vpaddd .Lle_addd_2 rRIP, %ymm15, %ymm1;
	vpaddd .Lle_addd_4 rRIP, %ymm15, %ymm2;
	vpaddd .Lle_addd_6 rRIP, %ymm15, %ymm3;

	vpaddd .Lle_addd_8_2 rRIP, %ymm15, %ymm15;

	/* AES rounds */
	XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lctr32le_enc_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lctr32le_enc_blk8_last;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lctr32le_enc_blk8_last:
	vpxor (0 * 16)(%rcx), %ymm4, %ymm5; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm4, %ymm6;
	vpxor (4 * 16)(%rcx), %ymm4, %ymm7;
	vpxor (6 * 16)(%rcx), %ymm4, %ymm4;
	leaq (8 * 16)(%rcx), %rcx;
	vaesenclast %ymm5, %ymm0, %ymm0;
	vaesenclast %ymm6, %ymm1, %ymm1;
	vaesenclast %ymm7, %ymm2, %ymm2;
	vaesenclast %ymm4, %ymm3, %ymm3;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	vmovdqu %ymm2, (4 * 16)(%rdx);
	vmovdqu %ymm3, (6 * 16)(%rdx);
	leaq (8 * 16)(%rdx), %rdx;

	/* Handle trailing four blocks. */
.align 8
.Lctr32le_enc_blk4:
	cmpq $4, %r8;
	jb .Lctr32le_enc_blk1;

	leaq -4(%r8), %r8;

	vbroadcasti128 (0 * 16)(%rdi), %ymm4;

	/* Increment counters. */
	vpaddd .Lle_addd_0 rRIP, %ymm15, %ymm0;
	vpaddd .Lle_addd_2 rRIP, %ymm15, %ymm1;

	vpaddd .Lle_addd_4_2 rRIP, %ymm15, %ymm15;

	/* AES rounds */
	XOR2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (1 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (2 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (3 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (4 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (5 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (6 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (7 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (8 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (9 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (10 * 16)(%rdi), %ymm4;
	cmpl $12, %r9d;
	jb .Lctr32le_enc_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (11 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (12 * 16)(%rdi), %ymm4;
	jz .Lctr32le_enc_blk4_last;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (13 * 16)(%rdi), %ymm4;
	VAESENC2(%ymm4, %ymm0, %ymm1);
	vbroadcasti128 (14 * 16)(%rdi), %ymm4;

	/* Last round and output handling. */
  .Lctr32le_enc_blk4_last:
	vpxor (0 * 16)(%rcx), %ymm4, %ymm5; /* Xor src to last round key. */
	vpxor (2 * 16)(%rcx), %ymm4, %ymm6;
	leaq (4 * 16)(%rcx), %rcx;
	vaesenclast %ymm5, %ymm0, %ymm0;
	vaesenclast %ymm6, %ymm1, %ymm1;
	vmovdqu %ymm0, (0 * 16)(%rdx);
	vmovdqu %ymm1, (2 * 16)(%rdx);
	leaq (4 * 16)(%rdx), %rdx;

	/* Process trailing one to three blocks, one per loop. */
.align 8
.Lctr32le_enc_blk1:
	cmpq $1, %r8;
	jb .Ldone_ctr32le_enc;

	leaq -1(%r8), %r8;

	/* Load and increament counter. */
	vmovdqu %xmm15, %xmm0;
	vpaddd .Lle_addd_1 rRIP, %xmm15, %xmm15;

	/* AES rounds. */
	vpxor (0 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (1 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (2 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (3 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (4 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (5 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (6 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (7 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (8 * 16)(%rdi), %xmm0, %xmm0;
	vaesenc (9 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (10 * 16)(%rdi), %xmm1;
	cmpl $12, %r9d;
	jb .Lctr32le_enc_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (11 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (12 * 16)(%rdi), %xmm1;
	jz .Lctr32le_enc_blk1_last;
	vaesenc %xmm1, %xmm0, %xmm0;
	vaesenc (13 * 16)(%rdi), %xmm0, %xmm0;
	vmovdqa (14 * 16)(%rdi), %xmm1;

	/* Last round and output handling. */
  .Lctr32le_enc_blk1_last:
	vpxor (%rcx), %xmm1, %xmm1; /* Xor src to last round key. */
	leaq 16(%rcx), %rcx;
	vaesenclast %xmm1, %xmm0, %xmm0; /* Last round and xor with xmm1. */
	vmovdqu %xmm0, (%rdx);
	leaq 16(%rdx), %rdx;

	jmp .Lctr32le_enc_blk1;

.align 8
.Ldone_ctr32le_enc:
	vmovdqu %xmm15, (%rsi);
	vzeroall;
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_ctr32le_enc_amd64,.-_gcry_vaes_avx2_ctr32le_enc_amd64)

/**********************************************************************
  OCB-mode encryption/decryption
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_ocb_checksum,@function)
_gcry_vaes_avx2_ocb_checksum:
	/* input:
	 *	%rax:     offset pointer
	 *	%r10:     plaintext pointer
	 *	%r11:     nblocks
	 */
	CFI_STARTPROC();

	vpxor %xmm0, %xmm0, %xmm0;
	cmpq $4, %r11;
	jb .Locb_checksum_blk1;
	vpxor %xmm1, %xmm1, %xmm1;
	vpxor %xmm2, %xmm2, %xmm2;
	vpxor %xmm3, %xmm3, %xmm3;
	cmpq $16, %r11;
	jb .Locb_checksum_blk4;
	vpxor %xmm4, %xmm4, %xmm4;
	vpxor %xmm5, %xmm5, %xmm5;
	vpxor %xmm6, %xmm6, %xmm6;
	vpxor %xmm7, %xmm7, %xmm7;
	cmpq $32, %r11;
	jb .Locb_checksum_blk16;
	vpxor %xmm8, %xmm8, %xmm8;
	vpxor %xmm9, %xmm9, %xmm9;
	vpxor %xmm10, %xmm10, %xmm10;
	vpxor %xmm11, %xmm11, %xmm11;
	vpxor %xmm12, %xmm12, %xmm12;
	vpxor %xmm13, %xmm13, %xmm13;
	vpxor %xmm14, %xmm14, %xmm14;
	vpxor %xmm15, %xmm15, %xmm15;

.align 8
.Locb_checksum_blk32:
	cmpq $32, %r11;
	jb .Locb_checksum_blk32_done;

	leaq -32(%r11), %r11;

	vpxor (0 * 16)(%r10), %ymm0, %ymm0;
	vpxor (2 * 16)(%r10), %ymm1, %ymm1;
	vpxor (4 * 16)(%r10), %ymm2, %ymm2;
	vpxor (6 * 16)(%r10), %ymm3, %ymm3;
	vpxor (8 * 16)(%r10), %ymm4, %ymm4;
	vpxor (10 * 16)(%r10), %ymm5, %ymm5;
	vpxor (12 * 16)(%r10), %ymm6, %ymm6;
	vpxor (14 * 16)(%r10), %ymm7, %ymm7;
	vpxor (16 * 16)(%r10), %ymm8, %ymm8;
	vpxor (18 * 16)(%r10), %ymm9, %ymm9;
	vpxor (20 * 16)(%r10), %ymm10, %ymm10;
	vpxor (22 * 16)(%r10), %ymm11, %ymm11;
	vpxor (24 * 16)(%r10), %ymm12, %ymm12;
	vpxor (26 * 16)(%r10), %ymm13, %ymm13;
	vpxor (28 * 16)(%r10), %ymm14, %ymm14;
	vpxor (30 * 16)(%r10), %ymm15, %ymm15;
	leaq (32 * 16)(%r10), %r10;

	jmp .Locb_checksum_blk32;

.align 8
.Locb_checksum_blk32_done:
	vpxor %ymm8, %ymm0, %ymm0;
	vpxor %ymm9, %ymm1, %ymm1;
	vpxor %ymm10, %ymm2, %ymm2;
	vpxor %ymm11, %ymm3, %ymm3;
	vpxor %ymm12, %ymm4, %ymm4;
	vpxor %ymm13, %ymm5, %ymm5;
	vpxor %ymm14, %ymm6, %ymm6;
	vpxor %ymm15, %ymm7, %ymm7;

.align 8
.Locb_checksum_blk16:
	cmpq $16, %r11;
	jb .Locb_checksum_blk16_done;

	leaq -16(%r11), %r11;

	vpxor (0 * 16)(%r10), %ymm0, %ymm0;
	vpxor (2 * 16)(%r10), %ymm1, %ymm1;
	vpxor (4 * 16)(%r10), %ymm2, %ymm2;
	vpxor (6 * 16)(%r10), %ymm3, %ymm3;
	vpxor (8 * 16)(%r10), %ymm4, %ymm4;
	vpxor (10 * 16)(%r10), %ymm5, %ymm5;
	vpxor (12 * 16)(%r10), %ymm6, %ymm6;
	vpxor (14 * 16)(%r10), %ymm7, %ymm7;
	leaq (16 * 16)(%r10), %r10;

	jmp .Locb_checksum_blk16;

.align 8
.Locb_checksum_blk16_done:
	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm5, %ymm1, %ymm1;
	vpxor %ymm6, %ymm2, %ymm2;
	vpxor %ymm7, %ymm3, %ymm3;
	vextracti128 $1, %ymm0, %xmm4;
	vextracti128 $1, %ymm1, %xmm5;
	vextracti128 $1, %ymm2, %xmm6;
	vextracti128 $1, %ymm3, %xmm7;
	vpxor %xmm4, %xmm0, %xmm0;
	vpxor %xmm5, %xmm1, %xmm1;
	vpxor %xmm6, %xmm2, %xmm2;
	vpxor %xmm7, %xmm3, %xmm3;

.align 8
.Locb_checksum_blk4:
	cmpq $4, %r11;
	jb .Locb_checksum_blk4_done;

	leaq -4(%r11), %r11;

	vpxor (0 * 16)(%r10), %xmm0, %xmm0;
	vpxor (1 * 16)(%r10), %xmm1, %xmm1;
	vpxor (2 * 16)(%r10), %xmm2, %xmm2;
	vpxor (3 * 16)(%r10), %xmm3, %xmm3;
	leaq (4 * 16)(%r10), %r10;

	jmp .Locb_checksum_blk4;

.align 8
.Locb_checksum_blk4_done:
	vpxor %xmm1, %xmm0, %xmm0;
	vpxor %xmm3, %xmm2, %xmm2;
	vpxor %xmm2, %xmm0, %xmm0;

.align 8
.Locb_checksum_blk1:
	cmpq $1, %r11;
	jb .Locb_checksum_done;

	leaq -1(%r11), %r11;

	vpxor (%r10), %xmm0, %xmm0;
	leaq 16(%r10), %r10;

	jmp .Locb_checksum_blk1;

.align 8
.Locb_checksum_done:
	vpxor (%rax), %xmm0, %xmm0;
	vmovdqu %xmm0, (%rax);
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_ocb_checksum,.-_gcry_vaes_avx2_ocb_checksum)

ELF(.type _gcry_vaes_avx2_ocb_crypt_amd64,@function)
.globl _gcry_vaes_avx2_ocb_crypt_amd64
_gcry_vaes_avx2_ocb_crypt_amd64:
	/* input:
	 *	%rdi:     round keys
	 *	%esi:     nblk
	 *	%rdx:     dst
	 *	%rcx:     src
	 *	%r8:      nblocks
	 *	%r9:      nrounds
	 *	16(%rbp): offset
	 *	24(%rbp): checksum
	 *	32(%rbp): L-array
	 *	40(%rbp): encrypt (%r15d)
	 */
	CFI_STARTPROC();

#define STACK_REGS_POS (16 * 16 + 4 * 16)
#define STACK_ALLOC (STACK_REGS_POS + 6 * 8)

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $STACK_ALLOC, %rsp;
	andq $~63, %rsp;

	movq %r12, (STACK_REGS_POS + 0 * 8)(%rsp);
	CFI_REG_ON_STACK(r12, STACK_REGS_POS + 0 * 8);
	movq %r13, (STACK_REGS_POS + 1 * 8)(%rsp);
	CFI_REG_ON_STACK(r13, STACK_REGS_POS + 1 * 8);
	movq %r14, (STACK_REGS_POS + 2 * 8)(%rsp);
	CFI_REG_ON_STACK(r14, STACK_REGS_POS + 2 * 8);
	movq %r15, (STACK_REGS_POS + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r15, STACK_REGS_POS + 3 * 8);

	movl 40(%rbp), %r15d; /* encrypt-flag. */
	movq 16(%rbp), %r14; /* offset ptr. */

	/* Handle encryption checksumming. */
	testl %r15d, %r15d;
	jz .Locb_dec_checksum_prepare;
	movq 24(%rbp), %rax; /* checksum ptr. */
	movq %rcx, %r10;
	movq %r8, %r11;
	call _gcry_vaes_avx2_ocb_checksum;
	jmp .Locb_enc_checksum_done;
.Locb_dec_checksum_prepare:
	/* Store plaintext address and number of blocks for decryption
	 * checksumming. */
	movq %rdx, (STACK_REGS_POS + 4 * 8)(%rsp);
	movq %r8, (STACK_REGS_POS + 5 * 8)(%rsp);
.Locb_enc_checksum_done:

	vmovdqu (%r14), %xmm15; /* Load offset. */
	movq 32(%rbp), %r14; /* L-array ptr. */
	vmovdqa (0 * 16)(%rdi), %xmm0; /* first key */
	movl $(10 * 16), %eax;
	cmpl $12, %r9d;
	jb .Llast_key_ptr;
	movl $(12 * 16), %eax;
	je .Llast_key_ptr;
	movl $(14 * 16), %eax;
  .align 8
  .Llast_key_ptr:
	vpxor (%rdi, %rax), %xmm0, %xmm0; /* first key ^ last key */
	vpxor (0 * 16)(%rdi), %xmm15, %xmm15; /* offset ^ first key */
	vmovdqa %xmm0, (14 * 16)(%rsp);
	vmovdqa %xmm0, (15 * 16)(%rsp);

.align 8
.Lhandle_unaligned_ocb:
	/* Get number of blocks to align nblk to 16 (and L-array optimization). */
	movl %esi, %r10d;
	negl %r10d;
	andl $15, %r10d;
	cmpq %r8, %r10;
	cmovaq %r8, %r10;
	cmpq $1, %r10;
	jb .Lunaligned_ocb_done;

	/* Number of blocks after alignment. */
	movq %r8, %r11;
	subq %r10, %r11;

	/* If number after alignment is less than 16, skip aligned handling
	 * completely. */
	cmp $16, %r11;
	cmovbq %r8, %r10;

	/* Unaligned: Process eight blocks per loop. */
.align 8
.Locb_unaligned_blk8:
	cmpq $8, %r10;
	jb .Locb_unaligned_blk4;

	leaq -8(%r8), %r8;
	leaq -8(%r10), %r10;

	leal 1(%esi), %r11d;
	leal 2(%esi), %r12d;
	leal 3(%esi), %r13d;
	leal 4(%esi), %eax;
	tzcntl %r11d, %r11d;
	tzcntl %r12d, %r12d;
	tzcntl %r13d, %r13d;
	tzcntl %eax, %eax;
	shll $4, %r11d;
	shll $4, %r12d;
	shll $4, %r13d;
	shll $4, %eax;
	vpxor (%r14, %r11), %xmm15, %xmm5;
	vpxor (%r14, %r12), %xmm5, %xmm6;
	vpxor (%r14, %r13), %xmm6, %xmm7;
	vpxor (%r14, %rax), %xmm7, %xmm8;

	leal 5(%esi), %r11d;
	leal 6(%esi), %r12d;
	leal 7(%esi), %r13d;
	leal 8(%esi), %esi;
	tzcntl %r11d, %r11d;
	tzcntl %r12d, %r12d;
	tzcntl %r13d, %r13d;
	tzcntl %esi, %eax;
	shll $4, %r11d;
	shll $4, %r12d;
	shll $4, %r13d;
	shll $4, %eax;
	vpxor (%r14, %r11), %xmm8, %xmm9;
	vpxor (%r14, %r12), %xmm9, %xmm10;
	vpxor (%r14, %r13), %xmm10, %xmm11;
	vpxor (%r14, %rax), %xmm11, %xmm15;

	vinserti128 $1, %xmm6, %ymm5, %ymm5;
	vinserti128 $1, %xmm8, %ymm7, %ymm6;
	vinserti128 $1, %xmm10, %ymm9, %ymm7;
	vinserti128 $1, %xmm15, %ymm11, %ymm8;

	vpxor (0 * 16)(%rcx), %ymm5, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm6, %ymm1;
	vpxor (4 * 16)(%rcx), %ymm7, %ymm2;
	vpxor (6 * 16)(%rcx), %ymm8, %ymm3;
	leaq (8 * 16)(%rcx), %rcx;

	vmovdqa (14 * 16)(%rsp), %ymm9;

	testl %r15d, %r15d;
	jz .Locb_unaligned_blk8_dec;
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk8_enc_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		jz .Locb_unaligned_blk8_enc_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);

		/* Last round and output handling. */
	.Locb_unaligned_blk8_enc_last:
		vpxor %ymm5, %ymm9, %ymm5; /* Xor src to last round key. */
		vpxor %ymm6, %ymm9, %ymm6;
		vpxor %ymm7, %ymm9, %ymm7;
		vpxor %ymm8, %ymm9, %ymm4;
		vaesenclast %ymm5, %ymm0, %ymm0;
		vaesenclast %ymm6, %ymm1, %ymm1;
		vaesenclast %ymm7, %ymm2, %ymm2;
		vaesenclast %ymm4, %ymm3, %ymm3;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		leaq (8 * 16)(%rdx), %rdx;

		jmp .Locb_unaligned_blk8;

	.align 8
	.Locb_unaligned_blk8_dec:
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk8_dec_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		jz .Locb_unaligned_blk8_dec_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);

		/* Last round and output handling. */
	.Locb_unaligned_blk8_dec_last:
		vpxor %ymm5, %ymm9, %ymm5; /* Xor src to last round key. */
		vpxor %ymm6, %ymm9, %ymm6;
		vpxor %ymm7, %ymm9, %ymm7;
		vpxor %ymm8, %ymm9, %ymm4;
		vaesdeclast %ymm5, %ymm0, %ymm0;
		vaesdeclast %ymm6, %ymm1, %ymm1;
		vaesdeclast %ymm7, %ymm2, %ymm2;
		vaesdeclast %ymm4, %ymm3, %ymm3;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		leaq (8 * 16)(%rdx), %rdx;

		jmp .Locb_unaligned_blk8;

	/* Unaligned: Process four blocks. */
.align 8
.Locb_unaligned_blk4:
	cmpq $4, %r10;
	jb .Locb_unaligned_blk1;

	leaq -4(%r8), %r8;
	leaq -4(%r10), %r10;

	leal 1(%esi), %r11d;
	leal 2(%esi), %r12d;
	leal 3(%esi), %r13d;
	leal 4(%esi), %esi;
	tzcntl %r11d, %r11d;
	tzcntl %r12d, %r12d;
	tzcntl %r13d, %r13d;
	tzcntl %esi, %eax;
	shll $4, %r11d;
	shll $4, %r12d;
	shll $4, %r13d;
	shll $4, %eax;

	vpxor (%r14, %r11), %xmm15, %xmm5;
	vpxor (%r14, %r12), %xmm5, %xmm6;
	vinserti128 $1, %xmm6, %ymm5, %ymm5;
	vpxor (%r14, %r13), %xmm6, %xmm7;
	vpxor (%r14, %rax), %xmm7, %xmm15;
	vinserti128 $1, %xmm15, %ymm7, %ymm6;

	vpxor (0 * 16)(%rcx), %ymm5, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm6, %ymm1;
	leaq (4 * 16)(%rcx), %rcx;

	testl %r15d, %r15d;
	jz .Locb_unaligned_blk4_dec;
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk4_enc_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		jz .Locb_unaligned_blk4_enc_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);

	      /* Last round and output handling. */
	.Locb_unaligned_blk4_enc_last:
		vmovdqa (14 * 16)(%rsp), %ymm8;
		vpxor %ymm5, %ymm8, %ymm5; /* Xor src to last round key. */
		vpxor %ymm6, %ymm8, %ymm6;
		vaesenclast %ymm5, %ymm0, %ymm0;
		vaesenclast %ymm6, %ymm1, %ymm1;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		leaq (4 * 16)(%rdx), %rdx;

		jmp .Locb_unaligned_blk1;

	.align 8
	.Locb_unaligned_blk4_dec:
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk4_dec_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		jz .Locb_unaligned_blk4_dec_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);

	      /* Last round and output handling. */
	.Locb_unaligned_blk4_dec_last:
		vmovdqa (14 * 16)(%rsp), %ymm8;
		vpxor %ymm5, %ymm8, %ymm5; /* Xor src to last round key. */
		vpxor %ymm6, %ymm8, %ymm6;
		vaesdeclast %ymm5, %ymm0, %ymm0;
		vaesdeclast %ymm6, %ymm1, %ymm1;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		leaq (4 * 16)(%rdx), %rdx;

	/* Unaligned: Process one block per loop. */
.align 8
.Locb_unaligned_blk1:
	cmpq $1, %r10;
	jb .Lunaligned_ocb_done;

	leaq -1(%r8), %r8;
	leaq -1(%r10), %r10;

	leal 1(%esi), %esi;
	tzcntl %esi, %r11d;
	shll $4, %r11d;
	vpxor (%r14, %r11), %xmm15, %xmm15;
	vpxor (%rcx), %xmm15, %xmm0;
	leaq 16(%rcx), %rcx;

	testl %r15d, %r15d;
	jz .Locb_unaligned_blk1_dec;
		/* AES rounds. */
		vaesenc (1 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (2 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (3 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (4 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (5 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (6 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (7 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (8 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (9 * 16)(%rdi), %xmm0, %xmm0;
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk1_enc_last;
		vaesenc (10 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (11 * 16)(%rdi), %xmm0, %xmm0;
		jz .Locb_unaligned_blk1_enc_last;
		vaesenc (12 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (13 * 16)(%rdi), %xmm0, %xmm0;

		/* Last round and output handling. */
	.Locb_unaligned_blk1_enc_last:
		vpxor (14 * 16)(%rsp), %xmm15, %xmm1;
		vaesenclast %xmm1, %xmm0, %xmm0;
		vmovdqu %xmm0, (%rdx);
		leaq 16(%rdx), %rdx;

		jmp .Locb_unaligned_blk1;

	.align 8
	.Locb_unaligned_blk1_dec:
		/* AES rounds. */
		vaesdec (1 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (2 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (3 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (4 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (5 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (6 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (7 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (8 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (9 * 16)(%rdi), %xmm0, %xmm0;
		cmpl $12, %r9d;
		jb .Locb_unaligned_blk1_dec_last;
		vaesdec (10 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (11 * 16)(%rdi), %xmm0, %xmm0;
		jz .Locb_unaligned_blk1_dec_last;
		vaesdec (12 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (13 * 16)(%rdi), %xmm0, %xmm0;

		/* Last round and output handling. */
	.Locb_unaligned_blk1_dec_last:
		vpxor (14 * 16)(%rsp), %xmm15, %xmm1;
		vaesdeclast %xmm1, %xmm0, %xmm0;
		vmovdqu %xmm0, (%rdx);
		leaq 16(%rdx), %rdx;

		jmp .Locb_unaligned_blk1;

.align 8
.Lunaligned_ocb_done:
	cmpq $1, %r8;
	jb .Ldone_ocb;

	/* Short buffers do not benefit from L-array optimization. */
	movq %r8, %r10;
	cmpq $16, %r8;
	jb .Locb_unaligned_blk8;

	vinserti128 $1, %xmm15, %ymm15, %ymm15;

	/* Prepare L-array optimization.
	 * Since nblk is aligned to 16, offsets will have following
	 * construction:
	 *  - block1 = ntz{0} = offset ^ L[0]
	 *  - block2 = ntz{1} = offset ^ L[0] ^ L[1]
	 *  - block3 = ntz{0} = offset ^ L[1]
	 *  - block4 = ntz{2} = offset ^ L[1] ^ L[2]
	 *  - block5 = ntz{0} = offset ^ L[0] ^ L[1] ^ L[2]
	 *  - block6 = ntz{1} = offset ^ L[0] ^ L[2]
	 *  - block7 = ntz{0} = offset ^ L[2]
	 *  - block8 = ntz{3} = offset ^ L[2] ^ L[3]
	 *  - block9 = ntz{0} = offset ^ L[0] ^ L[2] ^ L[3]
	 *  - block10 = ntz{1} = offset ^ L[0] ^ L[1] ^ L[2] ^ L[3]
	 *  - block11 = ntz{0} = offset ^ L[1] ^ L[2] ^ L[3]
	 *  - block12 = ntz{2} = offset ^ L[1] ^ L[3]
	 *  - block13 = ntz{0} = offset ^ L[0] ^ L[1] ^ L[3]
	 *  - block14 = ntz{1} = offset ^ L[0] ^ L[3]
	 *  - block15 = ntz{0} = offset ^ L[3]
	 *  - block16 = ntz{x} = offset ^ L[3] ^ L[ntz{x}]
	 */
	vmovdqu (0 * 16)(%r14), %xmm0;
	vmovdqu (1 * 16)(%r14), %xmm1;
	vmovdqu (2 * 16)(%r14), %xmm2;
	vmovdqu (3 * 16)(%r14), %xmm3;
	vpxor %xmm0, %xmm1, %xmm4; /* L[0] ^ L[1] */
	vpxor %xmm0, %xmm2, %xmm5; /* L[0] ^ L[2] */
	vpxor %xmm0, %xmm3, %xmm6; /* L[0] ^ L[3] */
	vpxor %xmm1, %xmm2, %xmm7; /* L[1] ^ L[2] */
	vpxor %xmm1, %xmm3, %xmm8; /* L[1] ^ L[3] */
	vpxor %xmm2, %xmm3, %xmm9; /* L[2] ^ L[3] */
	vpxor %xmm4, %xmm2, %xmm10; /* L[0] ^ L[1] ^ L[2] */
	vpxor %xmm5, %xmm3, %xmm11; /* L[0] ^ L[2] ^ L[3] */
	vpxor %xmm7, %xmm3, %xmm12; /* L[1] ^ L[2] ^ L[3] */
	vpxor %xmm0, %xmm8, %xmm13; /* L[0] ^ L[1] ^ L[3] */
	vpxor %xmm4, %xmm9, %xmm14; /* L[0] ^ L[1] ^ L[2] ^ L[3] */
	vinserti128 $1, %xmm4, %ymm0, %ymm0;
	vinserti128 $1, %xmm7, %ymm1, %ymm1;
	vinserti128 $1, %xmm5, %ymm10, %ymm10;
	vinserti128 $1, %xmm9, %ymm2, %ymm2;
	vinserti128 $1, %xmm14, %ymm11, %ymm11;
	vinserti128 $1, %xmm8, %ymm12, %ymm12;
	vinserti128 $1, %xmm6, %ymm13, %ymm13;
	vmovdqa %ymm0,  (0 * 16)(%rsp);
	vmovdqa %ymm1,  (2 * 16)(%rsp);
	vmovdqa %ymm10, (4 * 16)(%rsp);
	vmovdqa %ymm2,  (6 * 16)(%rsp);
	vmovdqa %ymm11, (8 * 16)(%rsp);
	vmovdqa %ymm12, (10 * 16)(%rsp);
	vmovdqa %ymm13, (12 * 16)(%rsp);

	/* Aligned: Process 16 blocks per loop. */
.align 8
.Locb_aligned_blk16:
	cmpq $16, %r8;
	jb .Locb_aligned_blk8;

	leaq -16(%r8), %r8;

	leal 16(%esi), %esi;
	tzcntl %esi, %eax;
	shll $4, %eax;

	vpxor (0 * 16)(%rsp), %ymm15, %ymm8;
	vpxor (2 * 16)(%rsp), %ymm15, %ymm9;
	vpxor (4 * 16)(%rsp), %ymm15, %ymm10;
	vpxor (6 * 16)(%rsp), %ymm15, %ymm11;
	vpxor (8 * 16)(%rsp), %ymm15, %ymm12;

	vpxor (3 * 16)(%r14), %xmm15, %xmm13; /* offset ^ first key ^ L[3] */
	vpxor (%r14, %rax), %xmm13, %xmm14; /* offset ^ first key ^ L[3] ^ L[ntz{nblk+16}] */
	vinserti128 $1, %xmm14, %ymm13, %ymm14;

	vpxor (10 * 16)(%rsp), %ymm15, %ymm13;
	vpxor (14 * 16)(%rcx), %ymm14, %ymm7;

	vpxor (0 * 16)(%rcx), %ymm8, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm9, %ymm1;
	vpxor (4 * 16)(%rcx), %ymm10, %ymm2;
	vpxor (6 * 16)(%rcx), %ymm11, %ymm3;
	vpxor (8 * 16)(%rcx), %ymm12, %ymm4;
	vpxor (10 * 16)(%rcx), %ymm13, %ymm5;
	vmovdqa %ymm13, (16 * 16)(%rsp);
	vpxor (12 * 16)(%rsp), %ymm15, %ymm13;
	vpxor (12 * 16)(%rcx), %ymm13, %ymm6;
	vmovdqa %ymm13, (18 * 16)(%rsp);

	leaq (16 * 16)(%rcx), %rcx;

	vperm2i128 $0x11, %ymm14, %ymm14, %ymm15;

	testl %r15d, %r15d;
	jz .Locb_aligned_blk16_dec;
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (2 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (3 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (4 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (5 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (6 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (7 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (8 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (9 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		cmpl $12, %r9d;
		jb .Locb_aligned_blk16_enc_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (11 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		jz .Locb_aligned_blk16_enc_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (13 * 16)(%rdi), %ymm13;
		VAESENC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);

		/* Last round and output handling. */
	.Locb_aligned_blk16_enc_last:
		vmovdqa (14 * 16)(%rsp), %ymm13;
		vpxor %ymm8, %ymm13, %ymm8;
		vpxor %ymm9, %ymm13, %ymm9;
		vpxor %ymm10, %ymm13, %ymm10;
		vpxor %ymm11, %ymm13, %ymm11;
		vaesenclast %ymm8, %ymm0, %ymm0;
		vaesenclast %ymm9, %ymm1, %ymm1;
		vaesenclast %ymm10, %ymm2, %ymm2;
		vaesenclast %ymm11, %ymm3, %ymm3;
		vpxor %ymm12, %ymm13, %ymm12;
		vpxor (16 * 16)(%rsp), %ymm13, %ymm8;
		vpxor (18 * 16)(%rsp), %ymm13, %ymm9;
		vpxor %ymm14, %ymm13, %ymm13;
		vaesenclast %ymm12, %ymm4, %ymm4;
		vaesenclast %ymm8, %ymm5, %ymm5;
		vaesenclast %ymm9, %ymm6, %ymm6;
		vaesenclast %ymm13, %ymm7, %ymm7;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		vmovdqu %ymm4, (8 * 16)(%rdx);
		vmovdqu %ymm5, (10 * 16)(%rdx);
		vmovdqu %ymm6, (12 * 16)(%rdx);
		vmovdqu %ymm7, (14 * 16)(%rdx);
		leaq (16 * 16)(%rdx), %rdx;

		jmp .Locb_aligned_blk16;

	.align 8
	.Locb_aligned_blk16_dec:
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (2 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (3 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (4 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (5 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (6 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (7 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (8 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (9 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		cmpl $12, %r9d;
		jb .Locb_aligned_blk16_dec_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (11 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		jz .Locb_aligned_blk16_dec_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);
		vbroadcasti128 (13 * 16)(%rdi), %ymm13;
		VAESDEC8(%ymm13, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7);

		/* Last round and output handling. */
	.Locb_aligned_blk16_dec_last:
		vmovdqa (14 * 16)(%rsp), %ymm13;
		vpxor %ymm8, %ymm13, %ymm8;
		vpxor %ymm9, %ymm13, %ymm9;
		vpxor %ymm10, %ymm13, %ymm10;
		vpxor %ymm11, %ymm13, %ymm11;
		vaesdeclast %ymm8, %ymm0, %ymm0;
		vaesdeclast %ymm9, %ymm1, %ymm1;
		vaesdeclast %ymm10, %ymm2, %ymm2;
		vaesdeclast %ymm11, %ymm3, %ymm3;
		vpxor %ymm12, %ymm13, %ymm12;
		vpxor (16 * 16)(%rsp), %ymm13, %ymm8;
		vpxor (18 * 16)(%rsp), %ymm13, %ymm9;
		vpxor %ymm14, %ymm13, %ymm13;
		vaesdeclast %ymm12, %ymm4, %ymm4;
		vaesdeclast %ymm8, %ymm5, %ymm5;
		vaesdeclast %ymm9, %ymm6, %ymm6;
		vaesdeclast %ymm13, %ymm7, %ymm7;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		vmovdqu %ymm4, (8 * 16)(%rdx);
		vmovdqu %ymm5, (10 * 16)(%rdx);
		vmovdqu %ymm6, (12 * 16)(%rdx);
		vmovdqu %ymm7, (14 * 16)(%rdx);
		leaq (16 * 16)(%rdx), %rdx;

		jmp .Locb_aligned_blk16;

	/* Aligned: Process trailing eight blocks. */
.align 8
.Locb_aligned_blk8:
	cmpq $8, %r8;
	jb .Locb_aligned_done;

	leaq -8(%r8), %r8;

	leal 8(%esi), %esi;
	tzcntl %esi, %eax;
	shll $4, %eax;

	vpxor (0 * 16)(%rsp), %ymm15, %ymm5;
	vpxor (2 * 16)(%rsp), %ymm15, %ymm6;
	vpxor (4 * 16)(%rsp), %ymm15, %ymm7;

	vpxor (2 * 16)(%r14), %xmm15, %xmm13; /* offset ^ first key ^ L[2] */
	vpxor (%r14, %rax), %xmm13, %xmm14; /* offset ^ first key ^ L[2] ^ L[ntz{nblk+8}] */
	vinserti128 $1, %xmm14, %ymm13, %ymm14;

	vpxor (0 * 16)(%rcx), %ymm5, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm6, %ymm1;
	vpxor (4 * 16)(%rcx), %ymm7, %ymm2;
	vpxor (6 * 16)(%rcx), %ymm14, %ymm3;
	leaq (8 * 16)(%rcx), %rcx;

	vperm2i128 $0x11, %ymm14, %ymm14, %ymm15;

	vmovdqa (14 * 16)(%rsp), %ymm8;

	testl %r15d, %r15d;
	jz .Locb_aligned_blk8_dec;
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		cmpl $12, %r9d;
		jb .Locb_aligned_blk8_enc_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		jz .Locb_aligned_blk8_enc_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);

		/* Last round and output handling. */
	.Locb_aligned_blk8_enc_last:
		vpxor %ymm5, %ymm8, %ymm5;
		vpxor %ymm6, %ymm8, %ymm6;
		vpxor %ymm7, %ymm8, %ymm7;
		vpxor %ymm14, %ymm8, %ymm4;
		vaesenclast %ymm5, %ymm0, %ymm0;
		vaesenclast %ymm6, %ymm1, %ymm1;
		vaesenclast %ymm7, %ymm2, %ymm2;
		vaesenclast %ymm4, %ymm3, %ymm3;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		leaq (8 * 16)(%rdx), %rdx;

		jmp .Locb_aligned_done;

	.align 8
	.Locb_aligned_blk8_dec:
		/* AES rounds */
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		cmpl $12, %r9d;
		jb .Locb_aligned_blk8_dec_last;
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		jz .Locb_aligned_blk8_dec_last;
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
		vbroadcasti128 (14 * 16)(%rdi), %ymm4;

		/* Last round and output handling. */
	.Locb_aligned_blk8_dec_last:
		vpxor %ymm5, %ymm8, %ymm5;
		vpxor %ymm6, %ymm8, %ymm6;
		vpxor %ymm7, %ymm8, %ymm7;
		vpxor %ymm14, %ymm8, %ymm4;
		vaesdeclast %ymm5, %ymm0, %ymm0;
		vaesdeclast %ymm6, %ymm1, %ymm1;
		vaesdeclast %ymm7, %ymm2, %ymm2;
		vaesdeclast %ymm4, %ymm3, %ymm3;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		vmovdqu %ymm2, (4 * 16)(%rdx);
		vmovdqu %ymm3, (6 * 16)(%rdx);
		leaq (8 * 16)(%rdx), %rdx;

.align 8
.Locb_aligned_done:
	/* Burn stack. */
	vpxor %ymm0, %ymm0, %ymm0;
	vmovdqa %ymm0, (0 * 16)(%rsp);
	vmovdqa %ymm0, (2 * 16)(%rsp);
	vmovdqa %ymm0, (4 * 16)(%rsp);
	vmovdqa %ymm0, (6 * 16)(%rsp);
	vmovdqa %ymm0, (8 * 16)(%rsp);
	vmovdqa %ymm0, (10 * 16)(%rsp);
	vmovdqa %ymm0, (12 * 16)(%rsp);
	vmovdqa %ymm0, (16 * 16)(%rsp);
	vmovdqa %ymm0, (18 * 16)(%rsp);

	/* Handle tailing 1…7 blocks in nblk-unaligned loop. */
	movq %r8, %r10;
	cmpq $1, %r8;
	jnb .Locb_unaligned_blk8;

.align 8
.Ldone_ocb:
	movq 16(%rbp), %r14; /* offset ptr. */
	vpxor (0 * 16)(%rdi), %xmm15, %xmm15; /* offset ^ first key ^ first key */
	vmovdqu %xmm15, (%r14); /* Store offset. */

	/* Handle decryption checksumming. */

	testl %r15d, %r15d;
	jnz .Locb_dec_checksum_done;
	movq 24(%rbp), %rax; /* checksum ptr. */
	movq (STACK_REGS_POS + 4 * 8)(%rsp), %r10;
	movq (STACK_REGS_POS + 5 * 8)(%rsp), %r11;
	call _gcry_vaes_avx2_ocb_checksum;
.Locb_dec_checksum_done:

	/* Burn stack. */
	vpxor %ymm0, %ymm0, %ymm0;
	vmovdqa %ymm0, (14 * 16)(%rsp);

	vzeroall;

	movq (STACK_REGS_POS + 0 * 8)(%rsp), %r12;
	CFI_RESTORE(%r12);
	movq (STACK_REGS_POS + 1 * 8)(%rsp), %r13;
	CFI_RESTORE(%r13);
	movq (STACK_REGS_POS + 2 * 8)(%rsp), %r14;
	CFI_RESTORE(%r14);
	movq (STACK_REGS_POS + 3 * 8)(%rsp), %r15;
	CFI_RESTORE(%r15);

	leave;
	CFI_LEAVE();
	ret_spec_stop

#undef STACK_REGS_POS
#undef STACK_ALLOC

	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_ocb_crypt_amd64,.-_gcry_vaes_avx2_ocb_crypt_amd64)

/**********************************************************************
  CTR-mode encryption
 **********************************************************************/
ELF(.type _gcry_vaes_avx2_xts_crypt_amd64,@function)
.globl _gcry_vaes_avx2_xts_crypt_amd64
_gcry_vaes_avx2_xts_crypt_amd64:
	/* input:
	 *	%rdi: round keys
	 *	%rsi: tweak
	 *	%rdx: dst
	 *	%rcx: src
	 *	%r8:  nblocks
	 *	%r9:  nrounds
	 *	8(%rsp): encrypt
	 */
	CFI_STARTPROC();

	movl 8(%rsp), %eax;

#define tweak_clmul(shift, out, tweak, hi_tweak, tmp1, tmp2) \
	vpsrld $(32-(shift)), hi_tweak, tmp2; \
	vpsllq $(shift), tweak, out; \
	vpclmulqdq $0, .Lxts_gfmul_clmul rRIP, tmp2, tmp1; \
	vpunpckhqdq tmp2, tmp1, tmp1; \
	vpxor tmp1, out, out;

	/* Prepare tweak. */
	vmovdqu (%rsi), %xmm15;
	vpshufb .Lxts_high_bit_shuf rRIP, %xmm15, %xmm13;
	tweak_clmul(1, %xmm11, %xmm15, %xmm13, %xmm0, %xmm1);
	vinserti128 $1, %xmm11, %ymm15, %ymm15; /* tweak:tweak1 */
	vpshufb .Lxts_high_bit_shuf rRIP, %ymm15, %ymm13;

	cmpq $8, %r8;
	jb .Lxts_crypt_blk4;

	/* Process eight blocks per loop. */
	leaq -8(%r8), %r8;

	vmovdqa %ymm15, %ymm5;
	tweak_clmul(2, %ymm6, %ymm15, %ymm13, %ymm0, %ymm1);
	tweak_clmul(4, %ymm7, %ymm15, %ymm13, %ymm0, %ymm1);
	tweak_clmul(6, %ymm8, %ymm15, %ymm13, %ymm0, %ymm1);
	tweak_clmul(8, %ymm15, %ymm15, %ymm13, %ymm0, %ymm1);
	vpshufb .Lxts_high_bit_shuf rRIP, %ymm15, %ymm13;

	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vpxor (0 * 16)(%rcx), %ymm5, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm6, %ymm1;
	vpxor (4 * 16)(%rcx), %ymm7, %ymm2;
	vpxor (6 * 16)(%rcx), %ymm8, %ymm3;

	leaq (8 * 16)(%rcx), %rcx;

.align 8
.Lxts_crypt_blk8_loop:
	cmpq $8, %r8;
	jb .Lxts_crypt_blk8_tail;
		leaq -8(%r8), %r8;

		testl %eax, %eax;
		jz .Lxts_dec_blk8;
			/* AES rounds */
			XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (1 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (2 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (3 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (4 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			  vmovdqa %ymm15, %ymm9;
			  tweak_clmul(2, %ymm10, %ymm15, %ymm13, %ymm12, %ymm14);
			  tweak_clmul(4, %ymm11, %ymm15, %ymm13, %ymm12, %ymm14);
			vbroadcasti128 (5 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (6 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (7 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (8 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (9 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (10 * 16)(%rdi), %ymm4;
			cmpl $12, %r9d;
			jb .Lxts_enc_blk8_last;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (11 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (12 * 16)(%rdi), %ymm4;
			jz .Lxts_enc_blk8_last;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (13 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (14 * 16)(%rdi), %ymm4;

			/* Last round and output handling. */
		.Lxts_enc_blk8_last:
			vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
			vpxor %ymm4, %ymm6, %ymm6;
			vpxor %ymm4, %ymm7, %ymm7;
			vpxor %ymm4, %ymm8, %ymm4;
			  tweak_clmul(6, %ymm8, %ymm15, %ymm13, %ymm12, %ymm14);
			  tweak_clmul(8, %ymm15, %ymm15, %ymm13, %ymm12, %ymm14);
			vpshufb .Lxts_high_bit_shuf rRIP, %ymm15, %ymm13;
			vaesenclast %ymm5, %ymm0, %ymm0;
			vaesenclast %ymm6, %ymm1, %ymm1;
			vaesenclast %ymm7, %ymm2, %ymm2;
			vaesenclast %ymm4, %ymm3, %ymm3;

			vmovdqu %ymm0, (0 * 16)(%rdx);
			vmovdqu %ymm1, (2 * 16)(%rdx);
			vmovdqu %ymm2, (4 * 16)(%rdx);
			vmovdqu %ymm3, (6 * 16)(%rdx);
			leaq (8 * 16)(%rdx), %rdx;

			vbroadcasti128 (0 * 16)(%rdi), %ymm4;
			vpxor (0 * 16)(%rcx), %ymm9, %ymm0;
			vpxor (2 * 16)(%rcx), %ymm10, %ymm1;
			vpxor (4 * 16)(%rcx), %ymm11, %ymm2;
			vpxor (6 * 16)(%rcx), %ymm8, %ymm3;

			  vmovdqa %ymm9, %ymm5;
			  vmovdqa %ymm10, %ymm6;
			  vmovdqa %ymm11, %ymm7;

			leaq (8 * 16)(%rcx), %rcx;

			jmp .Lxts_crypt_blk8_loop;

		.align 8
		.Lxts_dec_blk8:
			/* AES rounds */
			XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (1 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (2 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (3 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (4 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			  vmovdqa %ymm15, %ymm9;
			  tweak_clmul(2, %ymm10, %ymm15, %ymm13, %ymm12, %ymm14);
			  tweak_clmul(4, %ymm11, %ymm15, %ymm13, %ymm12, %ymm14);
			vbroadcasti128 (5 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (6 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (7 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (8 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (9 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (10 * 16)(%rdi), %ymm4;
			cmpl $12, %r9d;
			jb .Lxts_dec_blk8_last;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (11 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (12 * 16)(%rdi), %ymm4;
			jz .Lxts_dec_blk8_last;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (13 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (14 * 16)(%rdi), %ymm4;

			/* Last round and output handling. */
		.Lxts_dec_blk8_last:
			vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
			vpxor %ymm4, %ymm6, %ymm6;
			vpxor %ymm4, %ymm7, %ymm7;
			vpxor %ymm4, %ymm8, %ymm4;
			  tweak_clmul(6, %ymm8, %ymm15, %ymm13, %ymm12, %ymm14);
			  tweak_clmul(8, %ymm15, %ymm15, %ymm13, %ymm12, %ymm14);
			vpshufb .Lxts_high_bit_shuf rRIP, %ymm15, %ymm13;
			vaesdeclast %ymm5, %ymm0, %ymm0;
			vaesdeclast %ymm6, %ymm1, %ymm1;
			vaesdeclast %ymm7, %ymm2, %ymm2;
			vaesdeclast %ymm4, %ymm3, %ymm3;

			vmovdqu %ymm0, (0 * 16)(%rdx);
			vmovdqu %ymm1, (2 * 16)(%rdx);
			vmovdqu %ymm2, (4 * 16)(%rdx);
			vmovdqu %ymm3, (6 * 16)(%rdx);
			leaq (8 * 16)(%rdx), %rdx;

			vbroadcasti128 (0 * 16)(%rdi), %ymm4;
			vpxor (0 * 16)(%rcx), %ymm9, %ymm0;
			vpxor (2 * 16)(%rcx), %ymm10, %ymm1;
			vpxor (4 * 16)(%rcx), %ymm11, %ymm2;
			vpxor (6 * 16)(%rcx), %ymm8, %ymm3;

			  vmovdqa %ymm9, %ymm5;
			  vmovdqa %ymm10, %ymm6;
			  vmovdqa %ymm11, %ymm7;

			leaq (8 * 16)(%rcx), %rcx;

			jmp .Lxts_crypt_blk8_loop;

	.align 8
	.Lxts_crypt_blk8_tail:
		testl %eax, %eax;
		jz .Lxts_dec_tail_blk8;
			/* AES rounds */
			XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (1 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (2 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (3 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (4 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (5 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (6 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (7 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (8 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (9 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (10 * 16)(%rdi), %ymm4;
			cmpl $12, %r9d;
			jb .Lxts_enc_blk8_tail_last;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (11 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (12 * 16)(%rdi), %ymm4;
			jz .Lxts_enc_blk8_tail_last;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (13 * 16)(%rdi), %ymm4;
			VAESENC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (14 * 16)(%rdi), %ymm4;

			/* Last round and output handling. */
		.Lxts_enc_blk8_tail_last:
			vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
			vpxor %ymm4, %ymm6, %ymm6;
			vpxor %ymm4, %ymm7, %ymm7;
			vpxor %ymm4, %ymm8, %ymm4;
			vaesenclast %ymm5, %ymm0, %ymm0;
			vaesenclast %ymm6, %ymm1, %ymm1;
			vaesenclast %ymm7, %ymm2, %ymm2;
			vaesenclast %ymm4, %ymm3, %ymm3;
			vmovdqu %ymm0, (0 * 16)(%rdx);
			vmovdqu %ymm1, (2 * 16)(%rdx);
			vmovdqu %ymm2, (4 * 16)(%rdx);
			vmovdqu %ymm3, (6 * 16)(%rdx);
			leaq (8 * 16)(%rdx), %rdx;

			jmp .Lxts_crypt_blk4;

		.align 8
		.Lxts_dec_tail_blk8:
			/* AES rounds */
			XOR4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (1 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (2 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (3 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (4 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (5 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (6 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (7 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (8 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (9 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (10 * 16)(%rdi), %ymm4;
			cmpl $12, %r9d;
			jb .Lxts_dec_blk8_tail_last;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (11 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (12 * 16)(%rdi), %ymm4;
			jz .Lxts_dec_blk8_tail_last;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (13 * 16)(%rdi), %ymm4;
			VAESDEC4(%ymm4, %ymm0, %ymm1, %ymm2, %ymm3);
			vbroadcasti128 (14 * 16)(%rdi), %ymm4;

			/* Last round and output handling. */
		.Lxts_dec_blk8_tail_last:
			vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
			vpxor %ymm4, %ymm6, %ymm6;
			vpxor %ymm4, %ymm7, %ymm7;
			vpxor %ymm4, %ymm8, %ymm4;
			vaesdeclast %ymm5, %ymm0, %ymm0;
			vaesdeclast %ymm6, %ymm1, %ymm1;
			vaesdeclast %ymm7, %ymm2, %ymm2;
			vaesdeclast %ymm4, %ymm3, %ymm3;
			vmovdqu %ymm0, (0 * 16)(%rdx);
			vmovdqu %ymm1, (2 * 16)(%rdx);
			vmovdqu %ymm2, (4 * 16)(%rdx);
			vmovdqu %ymm3, (6 * 16)(%rdx);
			leaq (8 * 16)(%rdx), %rdx;

	/* Handle trailing four blocks. */
.align 8
.Lxts_crypt_blk4:
	/* Try exit early as typically input length is large power of 2. */
	cmpq $0, %r8;
	jb .Ldone_xts_crypt;
	cmpq $4, %r8;
	jb .Lxts_crypt_blk1;

	leaq -4(%r8), %r8;

	vmovdqa %ymm15, %ymm5;
	tweak_clmul(2, %ymm6, %ymm15, %ymm13, %ymm0, %ymm1);
	tweak_clmul(4, %ymm15, %ymm15, %ymm13, %ymm0, %ymm1);
	vpshufb .Lxts_high_bit_shuf rRIP, %ymm15, %ymm13;

	vbroadcasti128 (0 * 16)(%rdi), %ymm4;
	vpxor (0 * 16)(%rcx), %ymm5, %ymm0;
	vpxor (2 * 16)(%rcx), %ymm6, %ymm1;

	leaq (4 * 16)(%rcx), %rcx;

	testl %eax, %eax;
	jz .Lxts_dec_blk4;
		/* AES rounds */
		XOR2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		cmpl $12, %r9d;
		jb .Lxts_enc_blk4_last;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		jz .Lxts_enc_blk4_last;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESENC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (14 * 16)(%rdi), %ymm4;

		/* Last round and output handling. */
	.Lxts_enc_blk4_last:
		vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
		vpxor %ymm4, %ymm6, %ymm6;
		vaesenclast %ymm5, %ymm0, %ymm0;
		vaesenclast %ymm6, %ymm1, %ymm1;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		leaq (4 * 16)(%rdx), %rdx;

		jmp .Lxts_crypt_blk1;

	.align 8
	.Lxts_dec_blk4:
		/* AES rounds */
		XOR2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (1 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (2 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (3 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (4 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (5 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (6 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (7 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (8 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (9 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (10 * 16)(%rdi), %ymm4;
		cmpl $12, %r9d;
		jb .Lxts_dec_blk4_last;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (11 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (12 * 16)(%rdi), %ymm4;
		jz .Lxts_dec_blk4_last;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (13 * 16)(%rdi), %ymm4;
		VAESDEC2(%ymm4, %ymm0, %ymm1);
		vbroadcasti128 (14 * 16)(%rdi), %ymm4;

		/* Last round and output handling. */
	.Lxts_dec_blk4_last:
		vpxor %ymm4, %ymm5, %ymm5; /* Xor tweak to last round key. */
		vpxor %ymm4, %ymm6, %ymm6;
		vaesdeclast %ymm5, %ymm0, %ymm0;
		vaesdeclast %ymm6, %ymm1, %ymm1;
		vmovdqu %ymm0, (0 * 16)(%rdx);
		vmovdqu %ymm1, (2 * 16)(%rdx);
		leaq (4 * 16)(%rdx), %rdx;

	/* Process trailing one to three blocks, one per loop. */
.align 8
.Lxts_crypt_blk1:
	cmpq $1, %r8;
	jb .Ldone_xts_crypt;

	leaq -1(%r8), %r8;

	vpxor (%rcx), %xmm15, %xmm0;
	vmovdqa %xmm15, %xmm5;
	tweak_clmul(1, %xmm15, %xmm15, %xmm13, %xmm2, %xmm3);
	vpshufb .Lxts_high_bit_shuf rRIP, %xmm15, %xmm13;

	leaq 16(%rcx), %rcx;

	testl %eax, %eax;
	jz .Lxts_dec_blk1;
		/* AES rounds. */
		vpxor (0 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (1 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (2 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (3 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (4 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (5 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (6 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (7 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (8 * 16)(%rdi), %xmm0, %xmm0;
		vaesenc (9 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (10 * 16)(%rdi), %xmm1;
		cmpl $12, %r9d;
		jb .Lxts_enc_blk1_last;
		vaesenc %xmm1, %xmm0, %xmm0;
		vaesenc (11 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (12 * 16)(%rdi), %xmm1;
		jz .Lxts_enc_blk1_last;
		vaesenc %xmm1, %xmm0, %xmm0;
		vaesenc (13 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (14 * 16)(%rdi), %xmm1;

		/* Last round and output handling. */
	.Lxts_enc_blk1_last:
		vpxor %xmm1, %xmm5, %xmm5; /* Xor tweak to last round key. */
		vaesenclast %xmm5, %xmm0, %xmm0;
		vmovdqu %xmm0, (%rdx);
		leaq 16(%rdx), %rdx;

		jmp .Lxts_crypt_blk1;

	.align 8
	.Lxts_dec_blk1:
		/* AES rounds. */
		vpxor (0 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (1 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (2 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (3 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (4 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (5 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (6 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (7 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (8 * 16)(%rdi), %xmm0, %xmm0;
		vaesdec (9 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (10 * 16)(%rdi), %xmm1;
		cmpl $12, %r9d;
		jb .Lxts_dec_blk1_last;
		vaesdec %xmm1, %xmm0, %xmm0;
		vaesdec (11 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (12 * 16)(%rdi), %xmm1;
		jz .Lxts_dec_blk1_last;
		vaesdec %xmm1, %xmm0, %xmm0;
		vaesdec (13 * 16)(%rdi), %xmm0, %xmm0;
		vmovdqa (14 * 16)(%rdi), %xmm1;

		/* Last round and output handling. */
	.Lxts_dec_blk1_last:
		vpxor %xmm1, %xmm5, %xmm5; /* Xor tweak to last round key. */
		vaesdeclast %xmm5, %xmm0, %xmm0;
		vmovdqu %xmm0, (%rdx);
		leaq 16(%rdx), %rdx;

		jmp .Lxts_crypt_blk1;

.align 8
.Ldone_xts_crypt:
	/* Store IV. */
	vmovdqu %xmm15, (%rsi);

	vzeroall;

	xorl %eax, %eax
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_vaes_avx2_xts_crypt_amd64,.-_gcry_vaes_avx2_xts_crypt_amd64)

/**********************************************************************
  constants
 **********************************************************************/
ELF(.type _gcry_vaes_consts,@object)
_gcry_vaes_consts:
.align 32
.Lbige_addb_0:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lbige_addb_1:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
.Lbige_addb_2:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2
.Lbige_addb_3:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
.Lbige_addb_4:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4
.Lbige_addb_5:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5
.Lbige_addb_6:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6
.Lbige_addb_7:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
.Lbige_addb_8:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8
.Lbige_addb_9:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9
.Lbige_addb_10:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10
.Lbige_addb_11:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11
.Lbige_addb_12:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12
.Lbige_addb_13:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13
.Lbige_addb_14:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14
.Lbige_addb_15:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15

.Lle_addd_0:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_1:
	.byte 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_2:
	.byte 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_3:
	.byte 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_4:
	.byte 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_5:
	.byte 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_6:
	.byte 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_7:
	.byte 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_8:
	.byte 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_9:
	.byte 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_10:
	.byte 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_11:
	.byte 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_12:
	.byte 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_13:
	.byte 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_14:
	.byte 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_15:
	.byte 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.Lle_addd_4_2:
	.byte 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.byte 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_8_2:
	.byte 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.byte 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.Lle_addd_16_2:
	.byte 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.byte 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.Lxts_gfmul_clmul:
	.long 0x00, 0x87, 0x00, 0x00
	.long 0x00, 0x87, 0x00, 0x00
.Lxts_high_bit_shuf:
	.byte -1, -1, -1, -1, 12, 13, 14, 15
	.byte 4, 5, 6, 7, -1, -1, -1, -1
	.byte -1, -1, -1, -1, 12, 13, 14, 15
	.byte 4, 5, 6, 7, -1, -1, -1, -1
.Lbswap128_mask:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
ELF(.size _gcry_vaes_consts,.-_gcry_vaes_consts)

#endif /* HAVE_GCC_INLINE_ASM_VAES */
#endif /* __x86_64__ */
