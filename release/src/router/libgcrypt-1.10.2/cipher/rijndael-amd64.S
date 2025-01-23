/* rinjdael-amd64.S  -  AMD64 assembly implementation of AES cipher
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
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && defined(USE_AES)

#include "asm-common-amd64.h"

.text

/* table macros */
#define E0	(0)
#define Es0	(1)
#define Esize	4
#define Essize	4

#define D0	(0)
#define Ds0	(4 * 256)
#define Dsize	4
#define Dssize	1

/* register macros */
#define CTX	%rdi
#define RTAB	%r12

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

#define RNA	%r8
#define RNB	%r9
#define RNC	%r10
#define RND	%r11

#define RNAd	%r8d
#define RNBd	%r9d
#define RNCd	%r10d
#define RNDd	%r11d

#define RT0	%rbp
#define RT1	%rsi

#define RT0d	%ebp
#define RT1d	%esi

/* helper macros */
#define do16bit(op, source, tablemul, table1, dest1, table2, dest2, t0, t1) \
	movzbl source ## bl,			t0 ## d; \
	movzbl source ## bh,			t1 ## d; \
	op ## l table1(RTAB,t0,tablemul),	dest1 ## d; \
	op ## l table2(RTAB,t1,tablemul),	dest2 ## d;

#define do16bit_shr(shf, op, source, tablemul, table1, dest1, table2, dest2, t0, t1) \
	movzbl source ## bl,			t0 ## d; \
	movzbl source ## bh,			t1 ## d; \
	shrl $(shf),				source ## d; \
	op ## l table1(RTAB,t0,tablemul),	dest1 ## d; \
	op ## l table2(RTAB,t1,tablemul),	dest2 ## d;

#define last_do16bit(op, source, tablemul, table1, dest1, table2, dest2, t0, t1) \
	movzbl source ## bl,			t0 ## d; \
	movzbl source ## bh,			t1 ## d; \
	movzbl table1(RTAB,t0,tablemul),	t0 ## d; \
	movzbl table2(RTAB,t1,tablemul),	t1 ## d; \
	op ## l t0 ## d,			dest1 ## d; \
	op ## l t1 ## d,			dest2 ## d;

#define last_do16bit_shr(shf, op, source, tablemul, table1, dest1, table2, dest2, t0, t1) \
	movzbl source ## bl,			t0 ## d; \
	movzbl source ## bh,			t1 ## d; \
	shrl $(shf),				source ## d; \
	movzbl table1(RTAB,t0,tablemul),	t0 ## d; \
	movzbl table2(RTAB,t1,tablemul),	t1 ## d; \
	op ## l t0 ## d,			dest1 ## d; \
	op ## l t1 ## d,			dest2 ## d;

/***********************************************************************
 * AMD64 assembly implementation of the AES cipher
 ***********************************************************************/
#define addroundkey(round, ra, rb, rc, rd) \
	xorl (((round) * 16) + 0 * 4)(CTX), ra ## d; \
	xorl (((round) * 16) + 1 * 4)(CTX), rb ## d; \
	xorl (((round) * 16) + 2 * 4)(CTX), rc ## d; \
	xorl (((round) * 16) + 3 * 4)(CTX), rd ## d;

#define do_encround(next_r) \
	do16bit_shr(16, mov, RA, Esize, E0, RNA, E0, RND, RT0, RT1); \
	do16bit(        mov, RA, Esize, E0, RNC, E0, RNB, RT0, RT1); \
	movl (((next_r) * 16) + 0 * 4)(CTX), RAd; \
	roll $8, RNDd; \
	xorl RNAd, RAd; \
	roll $8, RNCd; \
	roll $8, RNBd; \
	roll $8, RAd; \
	\
	do16bit_shr(16, xor, RD, Esize, E0, RND, E0, RNC, RT0, RT1); \
	do16bit(        xor, RD, Esize, E0, RNB, E0, RA,  RT0, RT1); \
	movl (((next_r) * 16) + 3 * 4)(CTX), RDd; \
	roll $8, RNCd; \
	xorl RNDd, RDd; \
	roll $8, RNBd; \
	roll $8, RAd; \
	roll $8, RDd; \
	\
	do16bit_shr(16, xor, RC, Esize, E0, RNC, E0, RNB, RT0, RT1); \
	do16bit(        xor, RC, Esize, E0, RA,  E0, RD,  RT0, RT1); \
	movl (((next_r) * 16) + 2 * 4)(CTX), RCd; \
	roll $8, RNBd; \
	xorl RNCd, RCd; \
	roll $8, RAd; \
	roll $8, RDd; \
	roll $8, RCd; \
	\
	do16bit_shr(16, xor, RB, Esize, E0, RNB, E0, RA,  RT0, RT1); \
	do16bit(        xor, RB, Esize, E0, RD,  E0, RC,  RT0, RT1); \
	movl (((next_r) * 16) + 1 * 4)(CTX), RBd; \
	roll $8, RAd; \
	xorl RNBd, RBd; \
	roll $16, RDd; \
	roll $24, RCd;

#define do_lastencround(next_r) \
	do16bit_shr(16, movzb, RA, Essize, Es0, RNA, Es0, RND, RT0, RT1); \
	do16bit(        movzb, RA, Essize, Es0, RNC, Es0, RNB, RT0, RT1); \
	movl (((next_r) * 16) + 0 * 4)(CTX), RAd; \
	roll $8, RNDd; \
	xorl RNAd, RAd; \
	roll $8, RNCd; \
	roll $8, RNBd; \
	roll $8, RAd; \
	\
	last_do16bit_shr(16, xor, RD, Essize, Es0, RND, Es0, RNC, RT0, RT1); \
	last_do16bit(        xor, RD, Essize, Es0, RNB, Es0, RA,  RT0, RT1); \
	movl (((next_r) * 16) + 3 * 4)(CTX), RDd; \
	roll $8, RNCd; \
	xorl RNDd, RDd; \
	roll $8, RNBd; \
	roll $8, RAd; \
	roll $8, RDd; \
	\
	last_do16bit_shr(16, xor, RC, Essize, Es0, RNC, Es0, RNB, RT0, RT1); \
	last_do16bit(        xor, RC, Essize, Es0, RA,  Es0, RD,  RT0, RT1); \
	movl (((next_r) * 16) + 2 * 4)(CTX), RCd; \
	roll $8, RNBd; \
	xorl RNCd, RCd; \
	roll $8, RAd; \
	roll $8, RDd; \
	roll $8, RCd; \
	\
	last_do16bit_shr(16, xor, RB, Essize, Es0, RNB, Es0, RA,  RT0, RT1); \
	last_do16bit(        xor, RB, Essize, Es0, RD,  Es0, RC,  RT0, RT1); \
	movl (((next_r) * 16) + 1 * 4)(CTX), RBd; \
	roll $8, RAd; \
	xorl RNBd, RBd; \
	roll $16, RDd; \
	roll $24, RCd;

#define firstencround(round) \
	addroundkey(round, RA, RB, RC, RD); \
	do_encround((round) + 1);

#define encround(round) \
	do_encround((round) + 1);

#define lastencround(round) \
	do_lastencround((round) + 1);

.align 8
.globl _gcry_aes_amd64_encrypt_block
ELF(.type   _gcry_aes_amd64_encrypt_block,@function;)

_gcry_aes_amd64_encrypt_block:
	/* input:
	 *	%rdi: keysched, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 *	%ecx: number of rounds.. 10, 12 or 14
	 *	%r8:  encryption tables
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_5

	subq $(5 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(5 * 8);
	movq %rsi, (0 * 8)(%rsp);
	movl %ecx, (1 * 8)(%rsp);
	movq %rbp, (2 * 8)(%rsp);
	movq %rbx, (3 * 8)(%rsp);
	movq %r12, (4 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 2 * 8);
	CFI_REL_OFFSET(%rbx, 3 * 8);
	CFI_REL_OFFSET(%r12, 4 * 8);

	leaq (%r8), RTAB;

	/* read input block */
	movl 0 * 4(%rdx), RAd;
	movl 1 * 4(%rdx), RBd;
	movl 2 * 4(%rdx), RCd;
	movl 3 * 4(%rdx), RDd;

	firstencround(0);
	encround(1);
	encround(2);
	encround(3);
	encround(4);
	encround(5);
	encround(6);
	encround(7);
	encround(8);
	cmpl $12, (1 * 8)(%rsp);
	jnb .Lenc_not_128;
	lastencround(9);

.align 4
.Lenc_done:
	/* write output block */
	movq (0 * 8)(%rsp), %rsi;
	movl RAd, 0 * 4(%rsi);
	movl RBd, 1 * 4(%rsi);
	movl RCd, 2 * 4(%rsi);
	movl RDd, 3 * 4(%rsi);

	CFI_REMEMBER_STATE();

	movq (4 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %rbp;
	CFI_RESTORE(%r12);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%rbp);
	addq $(5 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-5 * 8);

	movl $(6 * 8), %eax;

	EXIT_SYSV_FUNC
	ret_spec_stop;

	CFI_RESTORE_STATE();
.align 4
.Lenc_not_128:
	je .Lenc_192

	encround(9);
	encround(10);
	encround(11);
	encround(12);
	lastencround(13);

	jmp .Lenc_done;

.align 4
.Lenc_192:
	encround(9);
	encround(10);
	lastencround(11);

	jmp .Lenc_done;
	CFI_ENDPROC();
ELF(.size _gcry_aes_amd64_encrypt_block,.-_gcry_aes_amd64_encrypt_block;)

#define do_decround(next_r) \
	do16bit_shr(16, mov, RA, Dsize, D0, RNA, D0, RNB, RT0, RT1); \
	do16bit(        mov, RA, Dsize, D0, RNC, D0, RND, RT0, RT1); \
	movl (((next_r) * 16) + 0 * 4)(CTX), RAd; \
	roll $8, RNBd; \
	xorl RNAd, RAd; \
	roll $8, RNCd; \
	roll $8, RNDd; \
	roll $8, RAd; \
	\
	do16bit_shr(16, xor, RB, Dsize, D0, RNB, D0, RNC, RT0, RT1); \
	do16bit(        xor, RB, Dsize, D0, RND, D0, RA,  RT0, RT1); \
	movl (((next_r) * 16) + 1 * 4)(CTX), RBd; \
	roll $8, RNCd; \
	xorl RNBd, RBd; \
	roll $8, RNDd; \
	roll $8, RAd; \
	roll $8, RBd; \
	\
	do16bit_shr(16, xor, RC, Dsize, D0, RNC, D0, RND, RT0, RT1); \
	do16bit(        xor, RC, Dsize, D0, RA,  D0, RB,  RT0, RT1); \
	movl (((next_r) * 16) + 2 * 4)(CTX), RCd; \
	roll $8, RNDd; \
	xorl RNCd, RCd; \
	roll $8, RAd; \
	roll $8, RBd; \
	roll $8, RCd; \
	\
	do16bit_shr(16, xor, RD, Dsize, D0, RND, D0, RA,  RT0, RT1); \
	do16bit(        xor, RD, Dsize, D0, RB,  D0, RC,  RT0, RT1); \
	movl (((next_r) * 16) + 3 * 4)(CTX), RDd; \
	roll $8, RAd; \
	xorl RNDd, RDd; \
	roll $16, RBd; \
	roll $24, RCd;

#define do_lastdecround(next_r) \
	do16bit_shr(16, movzb, RA, Dssize, Ds0, RNA, Ds0, RNB, RT0, RT1); \
	do16bit(        movzb, RA, Dssize, Ds0, RNC, Ds0, RND, RT0, RT1); \
	movl (((next_r) * 16) + 0 * 4)(CTX), RAd; \
	roll $8, RNBd; \
	xorl RNAd, RAd; \
	roll $8, RNCd; \
	roll $8, RNDd; \
	roll $8, RAd; \
	\
	last_do16bit_shr(16, xor, RB, Dssize, Ds0, RNB, Ds0, RNC, RT0, RT1); \
	last_do16bit(        xor, RB, Dssize, Ds0, RND, Ds0, RA,  RT0, RT1); \
	movl (((next_r) * 16) + 1 * 4)(CTX), RBd; \
	roll $8, RNCd; \
	xorl RNBd, RBd; \
	roll $8, RNDd; \
	roll $8, RAd; \
	roll $8, RBd; \
	\
	last_do16bit_shr(16, xor, RC, Dssize, Ds0, RNC, Ds0, RND, RT0, RT1); \
	last_do16bit(        xor, RC, Dssize, Ds0, RA,  Ds0, RB,  RT0, RT1); \
	movl (((next_r) * 16) + 2 * 4)(CTX), RCd; \
	roll $8, RNDd; \
	xorl RNCd, RCd; \
	roll $8, RAd; \
	roll $8, RBd; \
	roll $8, RCd; \
	\
	last_do16bit_shr(16, xor, RD, Dssize, Ds0, RND, Ds0, RA,  RT0, RT1); \
	last_do16bit(        xor, RD, Dssize, Ds0, RB,  Ds0, RC,  RT0, RT1); \
	movl (((next_r) * 16) + 3 * 4)(CTX), RDd; \
	roll $8, RAd; \
	xorl RNDd, RDd; \
	roll $16, RBd; \
	roll $24, RCd;

#define firstdecround(round) \
	addroundkey((round + 1), RA, RB, RC, RD); \
	do_decround(round);

#define decround(round) \
	do_decround(round);

#define lastdecround(round) \
	do_lastdecround(round);

.align 8
.globl _gcry_aes_amd64_decrypt_block
ELF(.type   _gcry_aes_amd64_decrypt_block,@function;)

_gcry_aes_amd64_decrypt_block:
	/* input:
	 *	%rdi: keysched, CTX
	 *	%rsi: dst
	 *	%rdx: src
	 *	%ecx: number of rounds.. 10, 12 or 14
	 *	%r8:  decryption tables
	 */
	CFI_STARTPROC();
	ENTER_SYSV_FUNC_PARAMS_5

	subq $(5 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(5 * 8);
	movq %rsi, (0 * 8)(%rsp);
	movl %ecx, (1 * 8)(%rsp);
	movq %rbp, (2 * 8)(%rsp);
	movq %rbx, (3 * 8)(%rsp);
	movq %r12, (4 * 8)(%rsp);
	CFI_REL_OFFSET(%rbp, 2 * 8);
	CFI_REL_OFFSET(%rbx, 3 * 8);
	CFI_REL_OFFSET(%r12, 4 * 8);

	leaq (%r8), RTAB;

	/* read input block */
	movl 0 * 4(%rdx), RAd;
	movl 1 * 4(%rdx), RBd;
	movl 2 * 4(%rdx), RCd;
	movl 3 * 4(%rdx), RDd;

	cmpl $12, (1 * 8)(%rsp);
	jnb .Ldec_256;

	firstdecround(9);
.align 4
.Ldec_tail:
	decround(8);
	decround(7);
	decround(6);
	decround(5);
	decround(4);
	decround(3);
	decround(2);
	decround(1);
	lastdecround(0);

	/* write output block */
	movq (0 * 8)(%rsp), %rsi;
	movl RAd, 0 * 4(%rsi);
	movl RBd, 1 * 4(%rsi);
	movl RCd, 2 * 4(%rsi);
	movl RDd, 3 * 4(%rsi);

	CFI_REMEMBER_STATE();

	movq (4 * 8)(%rsp), %r12;
	movq (3 * 8)(%rsp), %rbx;
	movq (2 * 8)(%rsp), %rbp;
	CFI_RESTORE(%r12);
	CFI_RESTORE(%rbx);
	CFI_RESTORE(%rbp);
	addq $(5 * 8), %rsp;
	CFI_ADJUST_CFA_OFFSET(-5 * 8);

	movl $(6 * 8), %eax;

	EXIT_SYSV_FUNC
	ret_spec_stop;

	CFI_RESTORE_STATE();
.align 4
.Ldec_256:
	je .Ldec_192;

	firstdecround(13);
	decround(12);
	decround(11);
	decround(10);
	decround(9);

	jmp .Ldec_tail;

.align 4
.Ldec_192:
	firstdecround(11);
	decround(10);
	decround(9);

	jmp .Ldec_tail;
	CFI_ENDPROC();
ELF(.size _gcry_aes_amd64_decrypt_block,.-_gcry_aes_amd64_decrypt_block;)

#endif /*USE_AES*/
#endif /*__x86_64*/
