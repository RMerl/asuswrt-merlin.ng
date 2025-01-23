/* blowfish-amd64.S  -  AMD64 assembly implementation of Blowfish cipher
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
#if defined(USE_BLOWFISH) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))

#include "asm-common-amd64.h"

.text

/* structure of BLOWFISH_context: */
#define s0	0
#define s1	((s0) + 256 * 4)
#define s2	((s1) + 256 * 4)
#define s3	((s2) + 256 * 4)
#define p	((s3) + 256 * 4)

/* register macros */
#define CTX %rdi
#define RIO %rsi

#define RX0 %rax
#define RX1 %rbx
#define RX2 %rcx
#define RX3 %rdx

#define RX0d %eax
#define RX1d %ebx
#define RX2d %ecx
#define RX3d %edx

#define RX0bl %al
#define RX1bl %bl
#define RX2bl %cl
#define RX3bl %dl

#define RX0bh %ah
#define RX1bh %bh
#define RX2bh %ch
#define RX3bh %dh

#define RT0 %rbp
#define RT1 %rsi
#define RT2 %r8
#define RT3 %r9

#define RT0d %ebp
#define RT1d %esi
#define RT2d %r8d
#define RT3d %r9d

#define RKEY %r10

/***********************************************************************
 * 1-way blowfish
 ***********************************************************************/
#define F() \
	movzbl RX0bh,		RT1d; \
	movzbl RX0bl,		RT3d; \
	rorq $16,		RX0; \
	movzbl RX0bh,		RT0d; \
	movzbl RX0bl,		RT2d; \
	rorq $16,		RX0; \
	movl s0(CTX,RT0,4),	RT0d; \
	addl s1(CTX,RT2,4),	RT0d; \
	xorl s2(CTX,RT1,4),	RT0d; \
	addl s3(CTX,RT3,4),	RT0d; \
	xorq RT0,		RX0;

#define load_roundkey_enc(n) \
	movq p+4*(n)(CTX), 	RX3;

#define add_roundkey_enc() \
	xorq RX3, 		RX0;

#define round_enc(n) \
	add_roundkey_enc(); \
	load_roundkey_enc(n); \
	\
	F(); \
	F();

#define load_roundkey_dec(n) \
	movq p+4*(n-1)(CTX),	RX3; \
	rorq $32,		RX3;

#define add_roundkey_dec() \
	xorq RX3, 		RX0;

#define round_dec(n) \
	add_roundkey_dec(); \
	load_roundkey_dec(n); \
	\
	F(); \
	F();

#define read_block() \
	movq (RIO), 		RX0; \
	rorq $32, 		RX0; \
	bswapq 			RX0;

#define write_block() \
	bswapq 			RX0; \
	movq RX0, 		(RIO);

.align 8
ELF(.type   __blowfish_enc_blk1,@function;)

__blowfish_enc_blk1:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RX0: input plaintext block
	 * output:
	 *	RX0: output plaintext block
	 */
	CFI_STARTPROC();
	movq %rbp, %r11;
	CFI_REGISTER(%rbp, %r11);

	load_roundkey_enc(0);
	round_enc(2);
	round_enc(4);
	round_enc(6);
	round_enc(8);
	round_enc(10);
	round_enc(12);
	round_enc(14);
	round_enc(16);
	add_roundkey_enc();

	movq %r11, %rbp;
	CFI_RESTORE(%rbp)

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __blowfish_enc_blk1,.-__blowfish_enc_blk1;)

.align 8
.globl  _gcry_blowfish_amd64_do_encrypt
ELF(.type   _gcry_blowfish_amd64_do_encrypt,@function;)

_gcry_blowfish_amd64_do_encrypt:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: u32 *ret_xl
	 *	%rdx: u32 *ret_xr
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	movl (%rdx), RX0d;
	shlq $32, RX0;
	movl (%rsi), RT3d;
	movq %rdx, %r10;
	orq RT3, RX0;
	movq %rsi, RX2;

	call __blowfish_enc_blk1;

	movl RX0d, (%r10);
	shrq $32, RX0;
	movl RX0d, (RX2);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_blowfish_amd64_do_encrypt,.-_gcry_blowfish_amd64_do_encrypt;)

.align 8
.globl  _gcry_blowfish_amd64_encrypt_block
ELF(.type   _gcry_blowfish_amd64_encrypt_block,@function;)

_gcry_blowfish_amd64_encrypt_block:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	movq %rsi, %r10;

	movq %rdx, RIO;
	read_block();

	call __blowfish_enc_blk1;

	movq %r10, RIO;
	write_block();

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_blowfish_amd64_encrypt_block,.-_gcry_blowfish_amd64_encrypt_block;)

.align 8
.globl  _gcry_blowfish_amd64_decrypt_block
ELF(.type   _gcry_blowfish_amd64_decrypt_block,@function;)

_gcry_blowfish_amd64_decrypt_block:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_0_4

	movq %rbp, %r11;
	CFI_REGISTER(%rbp, %r11);

	movq %rsi, %r10;
	movq %rdx, RIO;

	read_block();

	load_roundkey_dec(17);
	round_dec(15);
	round_dec(13);
	round_dec(11);
	round_dec(9);
	round_dec(7);
	round_dec(5);
	round_dec(3);
	round_dec(1);
	add_roundkey_dec();

	movq %r10, RIO;
	write_block();

	movq %r11, %rbp;
	CFI_RESTORE(%rbp);

	EXIT_SYSV_FUNC
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_blowfish_amd64_decrypt_block,.-_gcry_blowfish_amd64_decrypt_block;)

/**********************************************************************
  4-way blowfish, four blocks parallel
 **********************************************************************/
#define F4(x) \
	movzbl x ## bh,		RT1d; \
	movzbl x ## bl,		RT3d; \
	rorq $16,		x; \
	movzbl x ## bh,		RT0d; \
	movzbl x ## bl,		RT2d; \
	rorq $16,		x; \
	movl s0(CTX,RT0,4),	RT0d; \
	addl s1(CTX,RT2,4),	RT0d; \
	xorl s2(CTX,RT1,4),	RT0d; \
	addl s3(CTX,RT3,4),	RT0d; \
	xorq RT0,		x;

#define add_preloaded_roundkey4() \
	xorq RKEY,		RX0; \
	xorq RKEY,		RX1; \
	xorq RKEY,		RX2; \
	xorq RKEY,		RX3;

#define preload_roundkey_enc(n) \
	movq p+4*(n)(CTX),	RKEY;

#define add_roundkey_enc4(n) \
	add_preloaded_roundkey4(); \
	preload_roundkey_enc(n + 2);

#define round_enc4(n) \
	add_roundkey_enc4(n); \
	\
	F4(RX0); \
	F4(RX1); \
	F4(RX2); \
	F4(RX3); \
	\
	F4(RX0); \
	F4(RX1); \
	F4(RX2); \
	F4(RX3);

#define preload_roundkey_dec(n) \
	movq p+4*((n)-1)(CTX),	RKEY; \
	rorq $32,		RKEY;

#define add_roundkey_dec4(n) \
	add_preloaded_roundkey4(); \
	preload_roundkey_dec(n - 2);

#define round_dec4(n) \
	add_roundkey_dec4(n); \
	\
	F4(RX0); \
	F4(RX1); \
	F4(RX2); \
	F4(RX3); \
	\
	F4(RX0); \
	F4(RX1); \
	F4(RX2); \
	F4(RX3);

#define inbswap_block4() \
	rorq $32,		RX0; \
	bswapq 			RX0; \
	rorq $32,		RX1; \
	bswapq 			RX1; \
	rorq $32,		RX2; \
	bswapq 			RX2; \
	rorq $32,		RX3; \
	bswapq 			RX3;

#define inctrswap_block4() \
	rorq $32,		RX0; \
	rorq $32,		RX1; \
	rorq $32,		RX2; \
	rorq $32,		RX3;

#define outbswap_block4() \
	bswapq 			RX0; \
	bswapq 			RX1; \
	bswapq 			RX2; \
	bswapq 			RX3;

.align 8
ELF(.type   __blowfish_enc_blk4,@function;)

__blowfish_enc_blk4:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RX0,RX1,RX2,RX3: four input inbswapped plaintext blocks
	 * output:
	 *	RX0,RX1,RX2,RX3: four output ciphertext blocks
	 */
	CFI_STARTPROC();
	preload_roundkey_enc(0);

	round_enc4(0);
	round_enc4(2);
	round_enc4(4);
	round_enc4(6);
	round_enc4(8);
	round_enc4(10);
	round_enc4(12);
	round_enc4(14);
	add_preloaded_roundkey4();

	outbswap_block4();

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __blowfish_enc_blk4,.-__blowfish_enc_blk4;)

.align 8
ELF(.type   __blowfish_dec_blk4,@function;)

__blowfish_dec_blk4:
	/* input:
	 *	%rdi: ctx, CTX
	 *	RX0,RX1,RX2,RX3: four input ciphertext blocks
	 * output:
	 *	RX0,RX1,RX2,RX3: four output plaintext blocks
	 */
	CFI_STARTPROC();
	preload_roundkey_dec(17);

	inbswap_block4();

	round_dec4(17);
	round_dec4(15);
	round_dec4(13);
	round_dec4(11);
	round_dec4(9);
	round_dec4(7);
	round_dec4(5);
	round_dec4(3);
	add_preloaded_roundkey4();

	outbswap_block4();

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __blowfish_dec_blk4,.-__blowfish_dec_blk4;)

.align 8
.globl  _gcry_blowfish_amd64_ctr_enc
ELF(.type   _gcry_blowfish_amd64_ctr_enc,@function;)
_gcry_blowfish_amd64_ctr_enc:
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

	/* %r11-%r13 are not used by __blowfish_enc_blk4 */
	movq %rcx, %r13; /*iv*/
	movq %rdx, %r12; /*src*/
	movq %rsi, %r11; /*dst*/

	/* load IV and byteswap */
	movq (%r13), RT0;
	bswapq RT0;
	movq RT0, RX0;

	/* construct IVs */
	leaq 1(RT0), RX1;
	leaq 2(RT0), RX2;
	leaq 3(RT0), RX3;
	leaq 4(RT0), RT0;
	bswapq RT0;

	inctrswap_block4();

	/* store new IV */
	movq RT0, (%r13);

	call __blowfish_enc_blk4;

	/* XOR key-stream with plaintext */
	xorq 0 * 8(%r12), RX0;
	xorq 1 * 8(%r12), RX1;
	xorq 2 * 8(%r12), RX2;
	xorq 3 * 8(%r12), RX3;
	movq RX0, 0 * 8(%r11);
	movq RX1, 1 * 8(%r11);
	movq RX2, 2 * 8(%r11);
	movq RX3, 3 * 8(%r11);

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
ELF(.size _gcry_blowfish_amd64_ctr_enc,.-_gcry_blowfish_amd64_ctr_enc;)

.align 8
.globl  _gcry_blowfish_amd64_cbc_dec
ELF(.type   _gcry_blowfish_amd64_cbc_dec,@function;)
_gcry_blowfish_amd64_cbc_dec:
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

	/* %r11-%r13 are not used by __blowfish_dec_blk4 */
	movq %rsi, %r11; /*dst*/
	movq %rdx, %r12; /*src*/
	movq %rcx, %r13; /*iv*/

	/* load input */
	movq 0 * 8(%r12), RX0;
	movq 1 * 8(%r12), RX1;
	movq 2 * 8(%r12), RX2;
	movq 3 * 8(%r12), RX3;

	call __blowfish_dec_blk4;

	movq 3 * 8(%r12), RT0;
	xorq      (%r13), RX0;
	xorq 0 * 8(%r12), RX1;
	xorq 1 * 8(%r12), RX2;
	xorq 2 * 8(%r12), RX3;
	movq RT0, (%r13); /* store new IV */

	movq RX0, 0 * 8(%r11);
	movq RX1, 1 * 8(%r11);
	movq RX2, 2 * 8(%r11);
	movq RX3, 3 * 8(%r11);

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
ELF(.size _gcry_blowfish_amd64_cbc_dec,.-_gcry_blowfish_amd64_cbc_dec;)

.align 8
.globl  _gcry_blowfish_amd64_cfb_dec
ELF(.type   _gcry_blowfish_amd64_cfb_dec,@function;)
_gcry_blowfish_amd64_cfb_dec:
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

	/* %r11-%r13 are not used by __blowfish_enc_blk4 */
	movq %rcx, %r13; /*iv*/
	movq %rdx, %r12; /*src*/
	movq %rsi, %r11; /*dst*/

	/* Load input */
	movq (%r13), RX0;
	movq 0 * 8(%r12), RX1;
	movq 1 * 8(%r12), RX2;
	movq 2 * 8(%r12), RX3;

	inbswap_block4();

	/* Update IV */
	movq 3 * 8(%r12), RT0;
	movq RT0, (%r13);

	call __blowfish_enc_blk4;

	xorq 0 * 8(%r12), RX0;
	xorq 1 * 8(%r12), RX1;
	xorq 2 * 8(%r12), RX2;
	xorq 3 * 8(%r12), RX3;
	movq RX0, 0 * 8(%r11);
	movq RX1, 1 * 8(%r11);
	movq RX2, 2 * 8(%r11);
	movq RX3, 3 * 8(%r11);

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
ELF(.size _gcry_blowfish_amd64_cfb_dec,.-_gcry_blowfish_amd64_cfb_dec;)

#endif /*defined(USE_BLOWFISH)*/
#endif /*__x86_64*/
