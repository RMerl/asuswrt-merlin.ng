/* cast5-amd64.S  -  AMD64 assembly implementation of CAST5 cipher
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

#ifdef __x86_64
#include <config.h>
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_CAST5)

#include "asm-common-amd64.h"

.text

.extern _gcry_cast5_s1to4;

#define s1 0
#define s2 (s1 + (4 * 256))
#define s3 (s2 + (4 * 256))
#define s4 (s3 + (4 * 256))

/* structure of CAST5_context: */
#define Km 0
#define Kr (Km + (16 * 4))

/* register macros */
#define CTX %rdi
#define RIO %rsi
#define RTAB %r8

#define RLR0 %r9
#define RLR1 %r10
#define RLR2 %r11
#define RLR3 %r12

#define RLR0d %r9d
#define RLR1d %r10d
#define RLR2d %r11d
#define RLR3d %r12d

#define RX0 %rax
#define RX1 %rbx
#define RX2 %rdx

#define RX0d %eax
#define RX1d %ebx
#define RX2d %edx

#define RX0bl %al
#define RX1bl %bl
#define RX2bl %dl

#define RX0bh %ah
#define RX1bh %bh
#define RX2bh %dh

#define RKR %rcx
#define RKRd %ecx
#define RKRbl %cl

#define RT0 %rbp
#define RT1 %rsi

#define RT0d %ebp
#define RT1d %esi

#define RKM0d %r13d
#define RKM1d %r14d

/***********************************************************************
 * 1-way cast5
 ***********************************************************************/
#define dummy(x)

#define shr_kr(none) \
	shrq $8,			RKR;

#define F(km, load_next_kr, op0, op1, op2, op3) \
	op0 ## l RLR0d,			km ## d; \
	roll RKRbl,			km ## d; \
	rorq $32,			RLR0; \
	movzbl km ## bh,		RT0d; \
	movzbl km ## bl,		RT1d; \
	roll $16,			km ## d; \
	movl s1(RTAB,RT0,4),		RT0d; \
	op1 ## l s2(RTAB,RT1,4),	RT0d; \
	load_next_kr(kr_next); \
	movzbl km ## bh,		RT1d; \
	movzbl km ## bl,		km ## d; \
	op2 ## l s3(RTAB,RT1,4),	RT0d; \
	op3 ## l s4(RTAB,km,4),		RT0d; \
	xorq RT0,			RLR0;

#define F1(km, load_next_kr) \
	F(##km, load_next_kr, add, xor, sub, add)
#define F2(km, load_next_kr) \
	F(##km, load_next_kr, xor, sub, add, xor)
#define F3(km, load_next_kr) \
	F(##km, load_next_kr, sub, add, xor, sub)

#define get_round_km(n, km) \
	movl Km+4*(n)(CTX), 		km;

#define get_round_kr_enc(n) \
	movq $0x1010101010101010,	RKR; \
	\
	/* merge rorl rk and rorl $16 */ \
	xorq Kr+(n)(CTX),		RKR;

#define get_round_kr_dec(n) \
	movq $0x1010101010101010,	RKR; \
	\
	/* merge rorl rk and rorl $16 */ \
	xorq Kr+(n - 7)(CTX),		RKR; \
	bswapq				RKR;

#define round_enc(n, FA, FB, fn1, fn2) \
	get_round_km(n + 1, RX2d); \
	FA(RX0, fn1); \
	get_round_km(n + 2, RX0d); \
	FB(RX2, fn2);

#define round_enc_last(n, FXA, FXB) \
	get_round_km(n + 1, RX2d); \
	\
	FXA(RX0, shr_kr); \
	FXB(RX2, dummy);

#define round_enc_1(n, FA, FB) \
	round_enc(n, FA, FB, shr_kr, shr_kr)

#define round_enc_2(n, FA, FB) \
	round_enc(n, FA, FB, shr_kr, dummy)

#define round_dec(n, FA, FB, fn1, fn2) \
	get_round_km(n - 1, RX2d); \
	FA(RX0, fn1); \
	get_round_km(n - 2, RX0d); \
	FB(RX2, fn2);

#define round_dec_last(n, FXA, FXB) \
	get_round_km(n - 1, RX2d); \
	FXA(RX0, shr_kr); \
	FXB(RX2, dummy);

#define round_dec_1(n, FA, FB) \
	round_dec(n, FA, FB, shr_kr, shr_kr)

#define round_dec_2(n, FA, FB) \
	round_dec(n, FA, FB, shr_kr, dummy)

#define read_block() \
	movq (RIO), 		RLR0; \
	bswapq 			RLR0;

#define write_block() \
	bswapq 			RLR0; \
	rorq $32,		RLR0; \
	movq RLR0, 		(RIO);

.align 8
.globl _gcry_cast5_amd64_encrypt_block
ELF(.type   _gcry_cast5_amd64_encrypt_block,@function;)

_gcry_cast5_amd64_encrypt_block:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	pushq %rbp;
	CFI_PUSH(%rbp);
	pushq %rbx;
	CFI_PUSH(%rbx);

	movq %rsi, %r10;

	GET_EXTERN_POINTER(_gcry_cast5_s1to4, RTAB);

	movq %rdx, RIO;
	read_block();

	get_round_km(0, RX0d);
	get_round_kr_enc(0);
	round_enc_1(0, F1, F2);
	round_enc_1(2, F3, F1);
	round_enc_1(4, F2, F3);
	round_enc_2(6, F1, F2);
	get_round_kr_enc(8);
	round_enc_1(8, F3, F1);
	round_enc_1(10, F2, F3);
	round_enc_1(12, F1, F2);
	round_enc_last(14, F3, F1);

	movq %r10, RIO;
	write_block();

	popq %rbx;
	CFI_POP(%rbx);
	popq %rbp;
	CFI_POP(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_cast5_amd64_encrypt_block,.-_gcry_cast5_amd64_encrypt_block;)

.align 8
.globl _gcry_cast5_amd64_decrypt_block
ELF(.type   _gcry_cast5_amd64_decrypt_block,@function;)

_gcry_cast5_amd64_decrypt_block:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	pushq %rbp;
	CFI_PUSH(%rbp);
	pushq %rbx;
	CFI_PUSH(%rbx);

	movq %rsi, %r10;

	GET_EXTERN_POINTER(_gcry_cast5_s1to4, RTAB);

	movq %rdx, RIO;
	read_block();

	get_round_km(15, RX0d);
	get_round_kr_dec(15);
	round_dec_1(15, F1, F3);
	round_dec_1(13, F2, F1);
	round_dec_1(11, F3, F2);
	round_dec_2(9, F1, F3);
	get_round_kr_dec(7);
	round_dec_1(7, F2, F1);
	round_dec_1(5, F3, F2);
	round_dec_1(3, F1, F3);
	round_dec_last(1, F2, F1);

	movq %r10, RIO;
	write_block();

	popq %rbx;
	CFI_POP(%rbx);
	popq %rbp;
	CFI_POP(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_cast5_amd64_decrypt_block,.-_gcry_cast5_amd64_decrypt_block;)

/**********************************************************************
  4-way cast5, four blocks parallel
 **********************************************************************/
#define F_tail(rlr, rx, op1, op2, op3) \
	movzbl rx ## bh,		RT0d; \
	movzbl rx ## bl,		RT1d; \
	roll $16,			rx ## d; \
	movl s1(RTAB,RT0,4),		RT0d; \
	op1 ## l s2(RTAB,RT1,4),	RT0d; \
	movzbl rx ## bh,		RT1d; \
	movzbl rx ## bl,		rx ## d; \
	op2 ## l s3(RTAB,RT1,4),	RT0d; \
	op3 ## l s4(RTAB,rx,4),		RT0d; \
	xorq RT0,			rlr;

#define F4(km, load_next_kr, op0, op1, op2, op3) \
	movl km,			RX0d; \
	op0 ## l RLR0d,			RX0d; \
	roll RKRbl,			RX0d; \
	rorq $32,			RLR0; \
	\
	movl km,			RX1d; \
	op0 ## l RLR1d,			RX1d; \
	roll RKRbl,			RX1d; \
	rorq $32,			RLR1; \
	\
	movl km,			RX2d; \
	op0 ## l RLR2d,			RX2d; \
	roll RKRbl,			RX2d; \
	rorq $32,			RLR2; \
	\
	F_tail(RLR0, RX0, op1, op2, op3); \
	F_tail(RLR1, RX1, op1, op2, op3); \
	F_tail(RLR2, RX2, op1, op2, op3); \
	\
	movl km,			RX0d; \
	op0 ## l RLR3d,			RX0d; \
	roll RKRbl,			RX0d; \
	load_next_kr();			\
	rorq $32,			RLR3; \
	\
	F_tail(RLR3, RX0, op1, op2, op3);

#define F4_1(km, load_next_kr) \
	F4(km, load_next_kr, add, xor, sub, add)
#define F4_2(km, load_next_kr) \
	F4(km, load_next_kr, xor, sub, add, xor)
#define F4_3(km, load_next_kr) \
	F4(km, load_next_kr, sub, add, xor, sub)

#define round_enc4(n, FA, FB, fn1, fn2) \
	get_round_km(n + 1, RKM1d); \
	FA(RKM0d, fn1); \
	get_round_km(n + 2, RKM0d); \
	FB(RKM1d, fn2);

#define round_enc_last4(n, FXA, FXB) \
	get_round_km(n + 1, RKM1d); \
	FXA(RKM0d, shr_kr); \
	FXB(RKM1d, dummy);

#define round_enc4_1(n, FA, FB) \
	round_enc4(n, FA, FB, shr_kr, shr_kr);

#define round_enc4_2(n, FA, FB) \
	round_enc4(n, FA, FB, shr_kr, dummy);

#define round_dec4(n, FA, FB, fn1, fn2) \
	get_round_km(n - 1, RKM1d); \
	FA(RKM0d, fn1); \
	get_round_km(n - 2, RKM0d); \
	FB(RKM1d, fn2);

#define round_dec_last4(n, FXA, FXB) \
	get_round_km(n - 1, RKM1d); \
	FXA(RKM0d, shr_kr); \
	FXB(RKM1d, dummy);

#define round_dec4_1(n, FA, FB) \
	round_dec4(n, FA, FB, shr_kr, shr_kr);

#define round_dec4_2(n, FA, FB) \
	round_dec4(n, FA, FB, shr_kr, dummy);

#define inbswap_block4(a, b, c, d) \
	bswapq 			a; \
	bswapq 			b; \
	bswapq 			c; \
	bswapq 			d;

#define outbswap_block4(a, b, c, d) \
	bswapq 			a; \
	bswapq 			b; \
	bswapq 			c; \
	bswapq 			d; \
	rorq $32,		a; \
	rorq $32,		b; \
	rorq $32,		c; \
	rorq $32,		d;

.align 8
ELF(.type   __cast5_enc_blk4,@function;)

__cast5_enc_blk4:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RLR0,RLR1,RLR2,RLR3: four input plaintext blocks
	 * output:
	 *	RLR0,RLR1,RLR2,RLR3: four output ciphertext blocks
	 */
	CFI_STARTPROC();
	GET_EXTERN_POINTER(_gcry_cast5_s1to4, RTAB);

	get_round_km(0, RKM0d);
	get_round_kr_enc(0);
	round_enc4_1(0, F4_1, F4_2);
	round_enc4_1(2, F4_3, F4_1);
	round_enc4_1(4, F4_2, F4_3);
	round_enc4_2(6, F4_1, F4_2);
	get_round_kr_enc(8);
	round_enc4_1(8, F4_3, F4_1);
	round_enc4_1(10, F4_2, F4_3);
	round_enc4_1(12, F4_1, F4_2);
	round_enc_last4(14, F4_3, F4_1);

	outbswap_block4(RLR0, RLR1, RLR2, RLR3);
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __cast5_enc_blk4,.-__cast5_enc_blk4;)

.align 8
ELF(.type   __cast5_dec_blk4,@function;)

__cast5_dec_blk4:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RLR0,RLR1,RLR2,RLR3: four input ciphertext blocks
	 * output:
	 *	RLR0,RLR1,RLR2,RLR3: four output plaintext blocks
	 */
	CFI_STARTPROC();
	GET_EXTERN_POINTER(_gcry_cast5_s1to4, RTAB);

	inbswap_block4(RLR0, RLR1, RLR2, RLR3);

	get_round_km(15, RKM0d);
	get_round_kr_dec(15);
	round_dec4_1(15, F4_1, F4_3);
	round_dec4_1(13, F4_2, F4_1);
	round_dec4_1(11, F4_3, F4_2);
	round_dec4_2(9, F4_1, F4_3);
	get_round_kr_dec(7);
	round_dec4_1(7, F4_2, F4_1);
	round_dec4_1(5, F4_3, F4_2);
	round_dec4_1(3, F4_1, F4_3);
	round_dec_last4(1, F4_2, F4_1);

	outbswap_block4(RLR0, RLR1, RLR2, RLR3);
	CFI_ENDPROC();
	ret_spec_stop;
ELF(.size __cast5_dec_blk4,.-__cast5_dec_blk4;)

.align 8
.globl _gcry_cast5_amd64_ctr_enc
ELF(.type   _gcry_cast5_amd64_ctr_enc,@function;)
_gcry_cast5_amd64_ctr_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (4 blocks)
	 *	%rdx: src (4 blocks)
	 *	%rcx: iv (big endian, 64bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	pushq %rbp;
	CFI_PUSH(%rbp);
	pushq %rbx;
	CFI_PUSH(%rbx);
	pushq %r12;
	CFI_PUSH(%r12);
	pushq %r13;
	CFI_PUSH(%r13);
	pushq %r14;
	CFI_PUSH(%r14);

	pushq %rsi;
	CFI_PUSH(%rsi);
	pushq %rdx;
	CFI_PUSH(%rdx);

	/* load IV and byteswap */
	movq (%rcx), RX0;
	bswapq RX0;
	movq RX0, RLR0;

	/* construct IVs */
	leaq 1(RX0), RLR1;
	leaq 2(RX0), RLR2;
	leaq 3(RX0), RLR3;
	leaq 4(RX0), RX0;
	bswapq RX0;

	/* store new IV */
	movq RX0, (%rcx);

	call __cast5_enc_blk4;

	popq %r14; /*src*/
	CFI_POP_TMP_REG();
	popq %r13; /*dst*/
	CFI_POP_TMP_REG();

	/* XOR key-stream with plaintext */
	xorq 0 * 8(%r14), RLR0;
	xorq 1 * 8(%r14), RLR1;
	xorq 2 * 8(%r14), RLR2;
	xorq 3 * 8(%r14), RLR3;
	movq RLR0, 0 * 8(%r13);
	movq RLR1, 1 * 8(%r13);
	movq RLR2, 2 * 8(%r13);
	movq RLR3, 3 * 8(%r13);

	popq %r14;
	CFI_POP(%r14);
	popq %r13;
	CFI_POP(%r13);
	popq %r12;
	CFI_POP(%r12);
	popq %rbx;
	CFI_POP(%rbx);
	popq %rbp;
	CFI_POP(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop
	CFI_ENDPROC();
ELF(.size _gcry_cast5_amd64_ctr_enc,.-_gcry_cast5_amd64_ctr_enc;)

.align 8
.globl _gcry_cast5_amd64_cbc_dec
ELF(.type   _gcry_cast5_amd64_cbc_dec,@function;)
_gcry_cast5_amd64_cbc_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (4 blocks)
	 *	%rdx: src (4 blocks)
	 *	%rcx: iv (64bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	pushq %rbp;
	CFI_PUSH(%rbp);
	pushq %rbx;
	CFI_PUSH(%rbx);
	pushq %r12;
	CFI_PUSH(%r12);
	pushq %r13;
	CFI_PUSH(%r13);
	pushq %r14;
	CFI_PUSH(%r14);

	pushq %rcx;
	CFI_PUSH(%rcx);
	pushq %rsi;
	CFI_PUSH(%rsi);
	pushq %rdx;
	CFI_PUSH(%rdx);

	/* load input */
	movq 0 * 8(%rdx), RLR0;
	movq 1 * 8(%rdx), RLR1;
	movq 2 * 8(%rdx), RLR2;
	movq 3 * 8(%rdx), RLR3;

	call __cast5_dec_blk4;

	popq RX0; /*src*/
	CFI_POP_TMP_REG();
	popq RX1; /*dst*/
	CFI_POP_TMP_REG();
	popq RX2; /*iv*/
	CFI_POP_TMP_REG();

	movq 3 * 8(RX0), %r14;
	xorq      (RX2), RLR0;
	xorq 0 * 8(RX0), RLR1;
	xorq 1 * 8(RX0), RLR2;
	xorq 2 * 8(RX0), RLR3;
	movq %r14, (RX2); /* store new IV */

	movq RLR0, 0 * 8(RX1);
	movq RLR1, 1 * 8(RX1);
	movq RLR2, 2 * 8(RX1);
	movq RLR3, 3 * 8(RX1);

	popq %r14;
	CFI_POP(%r14);
	popq %r13;
	CFI_POP(%r13);
	popq %r12;
	CFI_POP(%r12);
	popq %rbx;
	CFI_POP(%rbx);
	popq %rbp;
	CFI_POP(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_cast5_amd64_cbc_dec,.-_gcry_cast5_amd64_cbc_dec;)

.align 8
.globl _gcry_cast5_amd64_cfb_dec
ELF(.type   _gcry_cast5_amd64_cfb_dec,@function;)
_gcry_cast5_amd64_cfb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (4 blocks)
	 *	%rdx: src (4 blocks)
	 *	%rcx: iv (64bit)
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	pushq %rbp;
	CFI_PUSH(%rbp);
	pushq %rbx;
	CFI_PUSH(%rbx);
	pushq %r12;
	CFI_PUSH(%r12);
	pushq %r13;
	CFI_PUSH(%r13);
	pushq %r14;
	CFI_PUSH(%r14);

	pushq %rsi;
	CFI_PUSH(%rsi);
	pushq %rdx;
	CFI_PUSH(%rdx);

	/* Load input */
	movq (%rcx), RLR0;
	movq 0 * 8(%rdx), RLR1;
	movq 1 * 8(%rdx), RLR2;
	movq 2 * 8(%rdx), RLR3;

	inbswap_block4(RLR0, RLR1, RLR2, RLR3);

	/* Update IV */
	movq 3 * 8(%rdx), %rdx;
	movq %rdx, (%rcx);

	call __cast5_enc_blk4;

	popq %rdx; /*src*/
	CFI_POP_TMP_REG();
	popq %rcx; /*dst*/
	CFI_POP_TMP_REG();

	xorq 0 * 8(%rdx), RLR0;
	xorq 1 * 8(%rdx), RLR1;
	xorq 2 * 8(%rdx), RLR2;
	xorq 3 * 8(%rdx), RLR3;
	movq RLR0, 0 * 8(%rcx);
	movq RLR1, 1 * 8(%rcx);
	movq RLR2, 2 * 8(%rcx);
	movq RLR3, 3 * 8(%rcx);

	popq %r14;
	CFI_POP(%r14);
	popq %r13;
	CFI_POP(%r13);
	popq %r12;
	CFI_POP(%r12);
	popq %rbx;
	CFI_POP(%rbx);
	popq %rbp;
	CFI_POP(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_cast5_amd64_cfb_dec,.-_gcry_cast5_amd64_cfb_dec;)

#endif /*defined(USE_CAST5)*/
#endif /*__x86_64*/
