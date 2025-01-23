/* twofish-amd64.S  -  AMD64 assembly implementation of Twofish cipher
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
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_TWOFISH)

#include "asm-common-amd64.h"

.text

/* structure of TWOFISH_context: */
#define s0 0
#define s1 ((s0) + 4 * 256)
#define s2 ((s1) + 4 * 256)
#define s3 ((s2) + 4 * 256)
#define w  ((s3) + 4 * 256)
#define k  ((w) + 4 * 8)

/* register macros */
#define CTX	%rdi

#define RA	%rax
#define RB	%rbx
#define RC	%rcx
#define RD	%rdx

#define RAd	%eax
#define RBd	%ebx
#define RCd	%ecx
#define RDd	%edx

#define RAbl	%al
#define RBbl	%bl
#define RCbl	%cl
#define RDbl	%dl

#define RAbh	%ah
#define RBbh	%bh
#define RCbh	%ch
#define RDbh	%dh

#define RX	%r8
#define RY	%r9

#define RXd	%r8d
#define RYd	%r9d

#define RT0	%rsi
#define RT1	%rbp
#define RT2	%r10
#define RT3	%r11

#define RT0d	%esi
#define RT1d	%ebp
#define RT2d	%r10d
#define RT3d	%r11d

/***********************************************************************
 * AMD64 assembly implementation of the Twofish cipher
 ***********************************************************************/
#define enc_g1_2(a, b, x, y) \
	movzbl b ## bl, RT3d; \
	movzbl b ## bh, RT1d; \
	movzbl a ## bl, RT2d; \
	movzbl a ## bh, RT0d; \
	rorl $16, b ## d; \
	rorl $16, a ## d; \
	movl s1(CTX, RT3, 4), RYd; \
	movzbl b ## bl, RT3d; \
	movl s0(CTX, RT2, 4), RXd; \
	movzbl a ## bl, RT2d; \
	xorl s2(CTX, RT1, 4), RYd; \
	movzbl b ## bh, RT1d; \
	xorl s1(CTX, RT0, 4), RXd; \
	movzbl a ## bh, RT0d; \
	rorl $16, b ## d; \
	rorl $16, a ## d; \
	xorl s3(CTX, RT3, 4), RYd; \
	xorl s2(CTX, RT2, 4), RXd; \
	xorl s0(CTX, RT1, 4), RYd; \
	xorl s3(CTX, RT0, 4), RXd;

#define dec_g1_2(a, b, x, y) \
	movzbl a ## bl, RT2d; \
	movzbl a ## bh, RT0d; \
	movzbl b ## bl, RT3d; \
	movzbl b ## bh, RT1d; \
	rorl $16, a ## d; \
	rorl $16, b ## d; \
	movl s0(CTX, RT2, 4), RXd; \
	movzbl a ## bl, RT2d; \
	movl s1(CTX, RT3, 4), RYd; \
	movzbl b ## bl, RT3d; \
	xorl s1(CTX, RT0, 4), RXd; \
	movzbl a ## bh, RT0d; \
	xorl s2(CTX, RT1, 4), RYd; \
	movzbl b ## bh, RT1d; \
	rorl $16, a ## d; \
	rorl $16, b ## d; \
	xorl s2(CTX, RT2, 4), RXd; \
	xorl s3(CTX, RT3, 4), RYd; \
	xorl s3(CTX, RT0, 4), RXd; \
	xorl s0(CTX, RT1, 4), RYd;

#define encrypt_round(ra, rb, rc, rd, n) \
	enc_g1_2(##ra, ##rb, RX, RY); \
	\
	leal (RXd, RYd, 2), RT0d; \
	addl RYd, RXd; \
	addl (k + 8 * (n) + 4)(CTX), RT0d; \
	roll $1, rd ## d; \
	addl (k + 8 * (n))(CTX), RXd; \
	xorl RT0d, rd ## d; \
	xorl RXd, rc ## d; \
	rorl $1, rc ## d;

#define decrypt_round(ra, rb, rc, rd, n) \
	dec_g1_2(##ra, ##rb, RX, RY); \
	\
	leal (RXd, RYd, 2), RT0d; \
	addl RYd, RXd; \
	addl (k + 8 * (n) + 4)(CTX), RT0d; \
	roll $1, rc ## d; \
	addl (k + 8 * (n))(CTX), RXd; \
	xorl RXd, rc ## d; \
	xorl RT0d, rd ## d; \
	rorl $1, rd ## d;

#define encrypt_cycle(a, b, c, d, nc) \
	encrypt_round(##a, ##b, ##c, ##d, (nc) * 2); \
	encrypt_round(##c, ##d, ##a, ##b, (nc) * 2 + 1);

#define decrypt_cycle(a, b, c, d, nc) \
	decrypt_round(##c, ##d, ##a, ##b, (nc) * 2 + 1); \
	decrypt_round(##a, ##b, ##c, ##d, (nc) * 2);

#define inpack(in, n, x, m) \
	movl (4 * (n))(in), x; \
	xorl (w + 4 * (m))(CTX), x;

#define outunpack(out, n, x, m) \
	xorl (w + 4 * (m))(CTX), x; \
	movl x, (4 * (n))(out);

.align 8
.globl _gcry_twofish_amd64_encrypt_block
ELF(.type   _gcry_twofish_amd64_encrypt_block,@function;)

_gcry_twofish_amd64_encrypt_block:
	/* input:
	 *	%rdi: context, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	subq $(3 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(3 * 8);
	movq %rsi, (0 * 8)(%rsp);
	movq %rbp, (1 * 8)(%rsp);
	movq %rbx, (2 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 1 * 8);
	CFI_REL_OFFSET(%rbx, 2 * 8);

	movq %rdx, RX;
	inpack(RX, 0, RAd, 0);
	inpack(RX, 1, RBd, 1);
	inpack(RX, 2, RCd, 2);
	inpack(RX, 3, RDd, 3);

	encrypt_cycle(RA, RB, RC, RD, 0);
	encrypt_cycle(RA, RB, RC, RD, 1);
	encrypt_cycle(RA, RB, RC, RD, 2);
	encrypt_cycle(RA, RB, RC, RD, 3);
	encrypt_cycle(RA, RB, RC, RD, 4);
	encrypt_cycle(RA, RB, RC, RD, 5);
	encrypt_cycle(RA, RB, RC, RD, 6);
	encrypt_cycle(RA, RB, RC, RD, 7);

	movq (0 * 8)(%rsp), RX; /*dst*/
	outunpack(RX, 0, RCd, 4);
	outunpack(RX, 1, RDd, 5);
	outunpack(RX, 2, RAd, 6);
	outunpack(RX, 3, RBd, 7);

	movq (2 * 8)(%rsp), %rbx;
	movq (1 * 8)(%rsp), %rbp;
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%rbp);
	addq $(3 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-3 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_encrypt_block,.-_gcry_twofish_amd64_encrypt_block;)

.align 8
.globl _gcry_twofish_amd64_decrypt_block
ELF(.type   _gcry_twofish_amd64_decrypt_block,@function;)

_gcry_twofish_amd64_decrypt_block:
	/* input:
	 *	%rdi: context, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	subq $(3 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(3 * 8);
	movq %rsi, (0 * 8)(%rsp);
	movq %rbp, (1 * 8)(%rsp);
	movq %rbx, (2 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 1 * 8);
	CFI_REL_OFFSET(%rbx, 2 * 8);

	movq %rdx, RX;
	inpack(RX, 0, RCd, 4);
	inpack(RX, 1, RDd, 5);
	inpack(RX, 2, RAd, 6);
	inpack(RX, 3, RBd, 7);

	decrypt_cycle(RA, RB, RC, RD, 7);
	decrypt_cycle(RA, RB, RC, RD, 6);
	decrypt_cycle(RA, RB, RC, RD, 5);
	decrypt_cycle(RA, RB, RC, RD, 4);
	decrypt_cycle(RA, RB, RC, RD, 3);
	decrypt_cycle(RA, RB, RC, RD, 2);
	decrypt_cycle(RA, RB, RC, RD, 1);
	decrypt_cycle(RA, RB, RC, RD, 0);

	movq (0 * 8)(%rsp), RX; /*dst*/
	outunpack(RX, 0, RAd, 0);
	outunpack(RX, 1, RBd, 1);
	outunpack(RX, 2, RCd, 2);
	outunpack(RX, 3, RDd, 3);

	movq (2 * 8)(%rsp), %rbx;
	movq (1 * 8)(%rsp), %rbp;
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%rbp);
	addq $(3 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-3 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_encrypt_block,.-_gcry_twofish_amd64_encrypt_block;)

#undef CTX

#undef RA
#undef RB
#undef RC
#undef RD

#undef RAd
#undef RBd
#undef RCd
#undef RDd

#undef RAbl
#undef RBbl
#undef RCbl
#undef RDbl

#undef RAbh
#undef RBbh
#undef RCbh
#undef RDbh

#undef RX
#undef RY

#undef RXd
#undef RYd

#undef RT0
#undef RT1
#undef RT2
#undef RT3

#undef RT0d
#undef RT1d
#undef RT2d
#undef RT3d

/***********************************************************************
 * AMD64 assembly implementation of the Twofish cipher, 3-way parallel
 ***********************************************************************/
#define CTX %rdi
#define RIO %rdx

#define RAB0 %rax
#define RAB1 %rbx
#define RAB2 %rcx

#define RAB0d %eax
#define RAB1d %ebx
#define RAB2d %ecx

#define RAB0bh %ah
#define RAB1bh %bh
#define RAB2bh %ch

#define RAB0bl %al
#define RAB1bl %bl
#define RAB2bl %cl

#define RCD0 %r8
#define RCD1 %r9
#define RCD2 %r10

#define RCD0d %r8d
#define RCD1d %r9d
#define RCD2d %r10d

#define RX0 %rbp
#define RX1 %r11
#define RX2 %r12

#define RX0d %ebp
#define RX1d %r11d
#define RX2d %r12d

#define RY0 %r13
#define RY1 %r14
#define RY2 %r15

#define RY0d %r13d
#define RY1d %r14d
#define RY2d %r15d

#define RT0 %rdx
#define RT1 %rsi

#define RT0d %edx
#define RT1d %esi

#define do16bit_ror(rot, op1, op2, T0, T1, tmp1, tmp2, ab, dst) \
	movzbl ab ## bl,		tmp2 ## d; \
	movzbl ab ## bh,		tmp1 ## d; \
	rorq $(rot),			ab; \
	op1##l T0(CTX, tmp2, 4),	dst ## d; \
	op2##l T1(CTX, tmp1, 4),	dst ## d;

/*
 * Combined G1 & G2 function. Reordered with help of rotates to have moves
 * at beginning.
 */
#define g1g2_3(ab, cd, Tx0, Tx1, Tx2, Tx3, Ty0, Ty1, Ty2, Ty3, x, y) \
	/* G1,1 && G2,1 */ \
	do16bit_ror(32, mov, xor, Tx0, Tx1, RT0, x ## 0, ab ## 0, x ## 0); \
	do16bit_ror(48, mov, xor, Ty1, Ty2, RT0, y ## 0, ab ## 0, y ## 0); \
	\
	do16bit_ror(32, mov, xor, Tx0, Tx1, RT0, x ## 1, ab ## 1, x ## 1); \
	do16bit_ror(48, mov, xor, Ty1, Ty2, RT0, y ## 1, ab ## 1, y ## 1); \
	\
	do16bit_ror(32, mov, xor, Tx0, Tx1, RT0, x ## 2, ab ## 2, x ## 2); \
	do16bit_ror(48, mov, xor, Ty1, Ty2, RT0, y ## 2, ab ## 2, y ## 2); \
	\
	/* G1,2 && G2,2 */ \
	do16bit_ror(32, xor, xor, Tx2, Tx3, RT0, RT1, ab ## 0, x ## 0); \
	do16bit_ror(16, xor, xor, Ty3, Ty0, RT0, RT1, ab ## 0, y ## 0); \
	movq ab ## 0, RT0; \
	movq cd ## 0, ab ## 0; \
	movq RT0, cd ## 0; \
	\
	do16bit_ror(32, xor, xor, Tx2, Tx3, RT0, RT1, ab ## 1, x ## 1); \
	do16bit_ror(16, xor, xor, Ty3, Ty0, RT0, RT1, ab ## 1, y ## 1); \
	movq ab ## 1, RT0; \
	movq cd ## 1, ab ## 1; \
	movq RT0, cd ## 1; \
	\
	do16bit_ror(32, xor, xor, Tx2, Tx3, RT0, RT1, ab ## 2, x ## 2); \
	do16bit_ror(16, xor, xor, Ty3, Ty0, RT0, RT1, ab ## 2, y ## 2); \
	movq ab ## 2, RT0; \
	movq cd ## 2, ab ## 2; \
	movq RT0, cd ## 2;

#define enc_round_end(ab, x, y, n) \
	addl y ## d,			x ## d; \
	addl x ## d,			y ## d; \
	addl k+4*(2*(n))(CTX),		x ## d; \
	xorl ab ## d,			x ## d; \
	addl k+4*(2*(n)+1)(CTX),	y ## d; \
	shrq $32,			ab; \
	roll $1,			ab ## d; \
	xorl y ## d,			ab ## d; \
	shlq $32,			ab; \
	rorl $1,			x ## d; \
	orq x,				ab;

#define dec_round_end(ba, x, y, n) \
	addl y ## d,			x ## d; \
	addl x ## d,			y ## d; \
	addl k+4*(2*(n))(CTX),		x ## d; \
	addl k+4*(2*(n)+1)(CTX),	y ## d; \
	xorl ba ## d,			y ## d; \
	shrq $32,			ba; \
	roll $1,			ba ## d; \
	xorl x ## d,			ba ## d; \
	shlq $32,			ba; \
	rorl $1,			y ## d; \
	orq y,				ba;

#define encrypt_round3(ab, cd, n) \
	g1g2_3(ab, cd, s0, s1, s2, s3, s0, s1, s2, s3, RX, RY); \
	\
	enc_round_end(ab ## 0, RX0, RY0, n); \
	enc_round_end(ab ## 1, RX1, RY1, n); \
	enc_round_end(ab ## 2, RX2, RY2, n);

#define decrypt_round3(ba, dc, n) \
	g1g2_3(ba, dc, s1, s2, s3, s0, s3, s0, s1, s2, RY, RX); \
	\
	dec_round_end(ba ## 0, RX0, RY0, n); \
	dec_round_end(ba ## 1, RX1, RY1, n); \
	dec_round_end(ba ## 2, RX2, RY2, n);

#define encrypt_cycle3(ab, cd, n) \
	encrypt_round3(ab, cd, n*2); \
	encrypt_round3(ab, cd, (n*2)+1);

#define decrypt_cycle3(ba, dc, n) \
	decrypt_round3(ba, dc, (n*2)+1); \
	decrypt_round3(ba, dc, (n*2));

#define inpack3(xy, m) \
	xorq w+4*m(CTX),		xy ## 0; \
	xorq w+4*m(CTX),		xy ## 1; \
	xorq w+4*m(CTX),		xy ## 2;

#define outunpack3(xy, m) \
	xorq w+4*m(CTX),		xy ## 0; \
	xorq w+4*m(CTX),		xy ## 1; \
	xorq w+4*m(CTX),		xy ## 2;

#define inpack_enc3() \
	inpack3(RAB, 0); \
	inpack3(RCD, 2);

#define outunpack_enc3() \
	outunpack3(RAB, 6); \
	outunpack3(RCD, 4);

#define inpack_dec3() \
	inpack3(RAB, 4); \
	rorq $32,			RAB0; \
	rorq $32,			RAB1; \
	rorq $32,			RAB2; \
	inpack3(RCD, 6); \
	rorq $32,			RCD0; \
	rorq $32,			RCD1; \
	rorq $32,			RCD2;

#define outunpack_dec3() \
	rorq $32,			RCD0; \
	rorq $32,			RCD1; \
	rorq $32,			RCD2; \
	outunpack3(RCD, 0); \
	rorq $32,			RAB0; \
	rorq $32,			RAB1; \
	rorq $32,			RAB2; \
	outunpack3(RAB, 2);

.align 8
ELF(.type __twofish_enc_blk3,@function;)

__twofish_enc_blk3:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RAB0,RCD0,RAB1,RCD1,RAB2,RCD2: three plaintext blocks
	 * output:
	 *	RCD0,RAB0,RCD1,RAB1,RCD2,RAB2: three ciphertext blocks
	 */
	CFI_STARTPROC();

	inpack_enc3();

	encrypt_cycle3(RAB, RCD, 0);
	encrypt_cycle3(RAB, RCD, 1);
	encrypt_cycle3(RAB, RCD, 2);
	encrypt_cycle3(RAB, RCD, 3);
	encrypt_cycle3(RAB, RCD, 4);
	encrypt_cycle3(RAB, RCD, 5);
	encrypt_cycle3(RAB, RCD, 6);
	encrypt_cycle3(RAB, RCD, 7);

	outunpack_enc3();

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __twofish_enc_blk3,.-__twofish_enc_blk3;)

.align 8
ELF(.type  __twofish_dec_blk3,@function;)

__twofish_dec_blk3:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RAB0,RCD0,RAB1,RCD1,RAB2,RCD2: three ciphertext blocks
	 * output:
	 *	RCD0,RAB0,RCD1,RAB1,RCD2,RAB2: three plaintext blocks
	 */
	CFI_STARTPROC();

	inpack_dec3();

	decrypt_cycle3(RAB, RCD, 7);
	decrypt_cycle3(RAB, RCD, 6);
	decrypt_cycle3(RAB, RCD, 5);
	decrypt_cycle3(RAB, RCD, 4);
	decrypt_cycle3(RAB, RCD, 3);
	decrypt_cycle3(RAB, RCD, 2);
	decrypt_cycle3(RAB, RCD, 1);
	decrypt_cycle3(RAB, RCD, 0);

	outunpack_dec3();

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __twofish_dec_blk3,.-__twofish_dec_blk3;)

.align 8
.globl _gcry_twofish_amd64_ctr_enc
ELF(.type   _gcry_twofish_amd64_ctr_enc,@function;)
_gcry_twofish_amd64_ctr_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (3 blocks)
	 *	%rdx: src (3 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	subq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(8 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rsi, (6 * 8)(%rsp);
	movq %rdx, (7 * 8)(%rsp);
	movq %rcx, RX0;

	/* load IV and byteswap */
	movq 8(RX0), RT0;
	movq 0(RX0), RT1;
	movq RT0, RCD0;
	movq RT1, RAB0;
	bswapq RT0;
	bswapq RT1;

	/* construct IVs */
	movq RT0, RCD1;
	movq RT1, RAB1;
	movq RT0, RCD2;
	movq RT1, RAB2;
	addq $1, RCD1;
	adcq $0, RAB1;
	bswapq RCD1;
	bswapq RAB1;
	addq $2, RCD2;
	adcq $0, RAB2;
	bswapq RCD2;
	bswapq RAB2;
	addq $3, RT0;
	adcq $0, RT1;
	bswapq RT0;
	bswapq RT1;

	/* store new IV */
	movq RT0, 8(RX0);
	movq RT1, 0(RX0);

	call __twofish_enc_blk3;

	movq (7 * 8)(%rsp), RX0; /*src*/
	movq (6 * 8)(%rsp), RX1; /*dst*/

	/* XOR key-stream with plaintext */
	xorq (0 * 8)(RX0), RCD0;
	xorq (1 * 8)(RX0), RAB0;
	xorq (2 * 8)(RX0), RCD1;
	xorq (3 * 8)(RX0), RAB1;
	xorq (4 * 8)(RX0), RCD2;
	xorq (5 * 8)(RX0), RAB2;
	movq RCD0, (0 * 8)(RX1);
	movq RAB0, (1 * 8)(RX1);
	movq RCD1, (2 * 8)(RX1);
	movq RAB1, (3 * 8)(RX1);
	movq RCD2, (4 * 8)(RX1);
	movq RAB2, (5 * 8)(RX1);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-8 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_ctr_enc,.-_gcry_twofish_amd64_ctr_enc;)

.align 8
.globl _gcry_twofish_amd64_cbc_dec
ELF(.type   _gcry_twofish_amd64_cbc_dec,@function;)
_gcry_twofish_amd64_cbc_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (3 blocks)
	 *	%rdx: src (3 blocks)
	 *	%rcx: iv (128bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	subq $(9 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(9 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rsi, (6 * 8)(%rsp);
	movq %rdx, (7 * 8)(%rsp);
	movq %rcx, (8 * 8)(%rsp);
	movq %rdx, RX0;

	/* load input */
	movq (0 * 8)(RX0), RAB0;
	movq (1 * 8)(RX0), RCD0;
	movq (2 * 8)(RX0), RAB1;
	movq (3 * 8)(RX0), RCD1;
	movq (4 * 8)(RX0), RAB2;
	movq (5 * 8)(RX0), RCD2;

	call __twofish_dec_blk3;

	movq (8 * 8)(%rsp), RT0; /*iv*/
	movq (7 * 8)(%rsp), RX0; /*src*/
	movq (6 * 8)(%rsp), RX1; /*dst*/

	movq (4 * 8)(RX0), RY0;
	movq (5 * 8)(RX0), RY1;
	xorq (0 * 8)(RT0), RCD0;
	xorq (1 * 8)(RT0), RAB0;
	xorq (0 * 8)(RX0), RCD1;
	xorq (1 * 8)(RX0), RAB1;
	xorq (2 * 8)(RX0), RCD2;
	xorq (3 * 8)(RX0), RAB2;
	movq RY0, (0 * 8)(RT0);
	movq RY1, (1 * 8)(RT0);

	movq RCD0, (0 * 8)(RX1);
	movq RAB0, (1 * 8)(RX1);
	movq RCD1, (2 * 8)(RX1);
	movq RAB1, (3 * 8)(RX1);
	movq RCD2, (4 * 8)(RX1);
	movq RAB2, (5 * 8)(RX1);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(9 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-9 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_cbc_dec,.-_gcry_twofish_amd64_cbc_dec;)

.align 8
.globl _gcry_twofish_amd64_cfb_dec
ELF(.type   _gcry_twofish_amd64_cfb_dec,@function;)
_gcry_twofish_amd64_cfb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (3 blocks)
	 *	%rdx: src (3 blocks)
	 *	%rcx: iv (128bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	subq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(8 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rsi, (6 * 8)(%rsp);
	movq %rdx, (7 * 8)(%rsp);
	movq %rdx, RX0;
	movq %rcx, RX1;

	/* load input */
	movq (0 * 8)(RX1), RAB0;
	movq (1 * 8)(RX1), RCD0;
	movq (0 * 8)(RX0), RAB1;
	movq (1 * 8)(RX0), RCD1;
	movq (2 * 8)(RX0), RAB2;
	movq (3 * 8)(RX0), RCD2;

	/* Update IV */
	movq (4 * 8)(RX0), RY0;
	movq (5 * 8)(RX0), RY1;
	movq RY0, (0 * 8)(RX1);
	movq RY1, (1 * 8)(RX1);

	call __twofish_enc_blk3;

	movq (7 * 8)(%rsp), RX0; /*src*/
	movq (6 * 8)(%rsp), RX1; /*dst*/

	xorq (0 * 8)(RX0), RCD0;
	xorq (1 * 8)(RX0), RAB0;
	xorq (2 * 8)(RX0), RCD1;
	xorq (3 * 8)(RX0), RAB1;
	xorq (4 * 8)(RX0), RCD2;
	xorq (5 * 8)(RX0), RAB2;
	movq RCD0, (0 * 8)(RX1);
	movq RAB0, (1 * 8)(RX1);
	movq RCD1, (2 * 8)(RX1);
	movq RAB1, (3 * 8)(RX1);
	movq RCD2, (4 * 8)(RX1);
	movq RAB2, (5 * 8)(RX1);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-8 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_cfb_dec,.-_gcry_twofish_amd64_cfb_dec;)

.align 8
.globl _gcry_twofish_amd64_ocb_enc
ELF(.type   _gcry_twofish_amd64_ocb_enc,@function;)
_gcry_twofish_amd64_ocb_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (3 blocks)
	 *	%rdx: src (3 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[3])
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_6

	subq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(8 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rsi, (6 * 8)(%rsp);
	movq %rdx, RX0;
	movq %rcx, RX1;
	movq %r8, RX2;
	movq %r9, RY0;
	movq %rsi, RY1;

	/* Load offset */
	movq (0 * 8)(RX1), RT0;
	movq (1 * 8)(RX1), RT1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq (RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (0 * 8)(RX0), RAB0;
	movq (1 * 8)(RX0), RCD0;
	/* Store Offset_i */
	movq RT0, (0 * 8)(RY1);
	movq RT1, (1 * 8)(RY1);
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	xor RAB0, (0 * 8)(RX2);
	xor RCD0, (1 * 8)(RX2);
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB0;
	xorq RT1, RCD0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 8(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (2 * 8)(RX0), RAB1;
	movq (3 * 8)(RX0), RCD1;
	/* Store Offset_i */
	movq RT0, (2 * 8)(RY1);
	movq RT1, (3 * 8)(RY1);
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	xor RAB1, (0 * 8)(RX2);
	xor RCD1, (1 * 8)(RX2);
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB1;
	xorq RT1, RCD1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 16(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (4 * 8)(RX0), RAB2;
	movq (5 * 8)(RX0), RCD2;
	/* Store Offset_i */
	movq RT0, (4 * 8)(RY1);
	movq RT1, (5 * 8)(RY1);
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	xor RAB2, (0 * 8)(RX2);
	xor RCD2, (1 * 8)(RX2);
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB2;
	xorq RT1, RCD2;

	/* Store offset */
	movq RT0, (0 * 8)(RX1);
	movq RT1, (1 * 8)(RX1);

	/* CX_i = ENCIPHER(K, PX_i)  */
	call __twofish_enc_blk3;

	movq (6 * 8)(%rsp), RX1; /*dst*/

	/* C_i = CX_i xor Offset_i  */
	xorq RCD0, (0 * 8)(RX1);
	xorq RAB0, (1 * 8)(RX1);
	xorq RCD1, (2 * 8)(RX1);
	xorq RAB1, (3 * 8)(RX1);
	xorq RCD2, (4 * 8)(RX1);
	xorq RAB2, (5 * 8)(RX1);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-8 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_ocb_enc,.-_gcry_twofish_amd64_ocb_enc;)

.align 8
.globl _gcry_twofish_amd64_ocb_dec
ELF(.type   _gcry_twofish_amd64_ocb_dec,@function;)
_gcry_twofish_amd64_ocb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (3 blocks)
	 *	%rdx: src (3 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[3])
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_6

	subq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(8 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rsi, (6 * 8)(%rsp);
	movq %r8,  (7 * 8)(%rsp);
	movq %rdx, RX0;
	movq %rcx, RX1;
	movq %r9, RY0;
	movq %rsi, RY1;

	/* Load offset */
	movq (0 * 8)(RX1), RT0;
	movq (1 * 8)(RX1), RT1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq (RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (0 * 8)(RX0), RAB0;
	movq (1 * 8)(RX0), RCD0;
	/* Store Offset_i */
	movq RT0, (0 * 8)(RY1);
	movq RT1, (1 * 8)(RY1);
	/* CX_i = C_i xor Offset_i */
	xorq RT0, RAB0;
	xorq RT1, RCD0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 8(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (2 * 8)(RX0), RAB1;
	movq (3 * 8)(RX0), RCD1;
	/* Store Offset_i */
	movq RT0, (2 * 8)(RY1);
	movq RT1, (3 * 8)(RY1);
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB1;
	xorq RT1, RCD1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 16(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (4 * 8)(RX0), RAB2;
	movq (5 * 8)(RX0), RCD2;
	/* Store Offset_i */
	movq RT0, (4 * 8)(RY1);
	movq RT1, (5 * 8)(RY1);
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB2;
	xorq RT1, RCD2;

	/* Store offset */
	movq RT0, (0 * 8)(RX1);
	movq RT1, (1 * 8)(RX1);

	/* PX_i = DECIPHER(K, CX_i)  */
	call __twofish_dec_blk3;

	movq (7 * 8)(%rsp), RX2; /*checksum*/
	movq (6 * 8)(%rsp), RX1; /*dst*/

	/* Load checksum */
	movq (0 * 8)(RX2), RT0;
	movq (1 * 8)(RX2), RT1;

	/* P_i = PX_i xor Offset_i  */
	xorq RCD0, (0 * 8)(RX1);
	xorq RAB0, (1 * 8)(RX1);
	xorq RCD1, (2 * 8)(RX1);
	xorq RAB1, (3 * 8)(RX1);
	xorq RCD2, (4 * 8)(RX1);
	xorq RAB2, (5 * 8)(RX1);

	/* Checksum_i = Checksum_{i-1} xor P_i  */
	xorq (0 * 8)(RX1), RT0;
	xorq (1 * 8)(RX1), RT1;
	xorq (2 * 8)(RX1), RT0;
	xorq (3 * 8)(RX1), RT1;
	xorq (4 * 8)(RX1), RT0;
	xorq (5 * 8)(RX1), RT1;

	/* Store checksum */
	movq RT0, (0 * 8)(RX2);
	movq RT1, (1 * 8)(RX2);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-8 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_ocb_dec,.-_gcry_twofish_amd64_ocb_dec;)

.align 8
.globl _gcry_twofish_amd64_ocb_auth
ELF(.type   _gcry_twofish_amd64_ocb_auth,@function;)
_gcry_twofish_amd64_ocb_auth:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: abuf (3 blocks)
	 *	%rdx: offset
	 *	%rcx: checksum
	 *	%r8 : L pointers (void *L[3])
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_5

	subq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(8 * 8);
	movq %rbp, (0 * 8)(%rsp);
	movq %rbx, (1 * 8)(%rsp);
	movq %r12, (2 * 8)(%rsp);
	movq %r13, (3 * 8)(%rsp);
	movq %r14, (4 * 8)(%rsp);
	movq %r15, (5 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 0 * 8);
	CFI_REL_OFFSET(%rbx, 1 * 8);
	CFI_REL_OFFSET(%r12, 2 * 8);
	CFI_REL_OFFSET(%r13, 3 * 8);
	CFI_REL_OFFSET(%r14, 4 * 8);
	CFI_REL_OFFSET(%r15, 5 * 8);

	movq %rcx, (6 * 8)(%rsp);
	movq %rsi, RX0;
	movq %rdx, RX1;
	movq %r8, RY0;

	/* Load offset */
	movq (0 * 8)(RX1), RT0;
	movq (1 * 8)(RX1), RT1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq (RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (0 * 8)(RX0), RAB0;
	movq (1 * 8)(RX0), RCD0;
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB0;
	xorq RT1, RCD0;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 8(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (2 * 8)(RX0), RAB1;
	movq (3 * 8)(RX0), RCD1;
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB1;
	xorq RT1, RCD1;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	movq 16(RY0), RY2;
	xorq (0 * 8)(RY2), RT0;
	xorq (1 * 8)(RY2), RT1;
	movq (4 * 8)(RX0), RAB2;
	movq (5 * 8)(RX0), RCD2;
	/* PX_i = P_i xor Offset_i */
	xorq RT0, RAB2;
	xorq RT1, RCD2;

	/* Store offset */
	movq RT0, (0 * 8)(RX1);
	movq RT1, (1 * 8)(RX1);

	/* C_i = ENCIPHER(K, PX_i)  */
	call __twofish_enc_blk3;

	movq (6 * 8)(%rsp), RX1; /*checksum*/

	/* Checksum_i = C_i xor Checksum_i  */
	xorq RCD0, RCD1;
	xorq RAB0, RAB1;
	xorq RCD1, RCD2;
	xorq RAB1, RAB2;
	xorq RCD2, (0 * 8)(RX1);
	xorq RAB2, (1 * 8)(RX1);

	movq (0 * 8)(%rsp), %rbp;
	movq (1 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %r13;
	movq (4 * 8)(%rsp), %r14;
	movq (5 * 8)(%rsp), %r15;
	CFI_RESTORE(%rbp);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);
	CFI_RESTORE(%r14);
	CFI_RESTORE(%r15);
	addq $(8 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-8 * 8);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_amd64_ocb_auth,.-_gcry_twofish_amd64_ocb_auth;)

#endif /*USE_TWOFISH*/
#endif /*__x86_64*/
