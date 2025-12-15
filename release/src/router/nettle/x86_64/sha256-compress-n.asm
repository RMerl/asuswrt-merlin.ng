C x86_64/sha256-compress-n.asm

ifelse(`
   Copyright (C) 2013, 2022 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
')

	.file "sha256-compress-n.asm"
define(`STATE', `%rdi')
define(`K', `%rsi')
define(`BLOCKS', `%rdx')
define(`INPUT', `%rcx')
define(`STATE_SAVED', `64(%rsp)')

define(`SA', `%eax')
define(`SB', `%ebx')
define(`SC', `%ebp')
define(`SD', `%r8d')
define(`SE', `%r9d')
define(`SF', `%r10d')
define(`SG', `%r11d')
define(`SH', `%r12d')
define(`T0', `%r13d')
define(`T1', `%r14d')
define(`COUNT', `%rdi')	C Overlap STATE
define(`W', `%r15d')

define(`EXPN', `
	movl	OFFSET($1)(%rsp), W
	movl	OFFSET(eval(($1 + 14) % 16))(%rsp), T0
	movl	T0, T1
	shrl	`$'10, T0
	roll	`$'13, T1
	xorl	T1, T0
	roll	`$'2, T1
	xorl	T1, T0
	addl	T0, W
	movl	OFFSET(eval(($1 + 1) % 16))(%rsp), T0
	movl	T0, T1
	shrl	`$'3, T0
	roll	`$'14, T1
	xorl	T1, T0
	roll	`$'11, T1
	xorl	T1, T0
	addl	T0, W
	addl	OFFSET(eval(($1 + 9) % 16))(%rsp), W
	movl	W, OFFSET($1)(%rsp)
')

C ROUND(A,B,C,D,E,F,G,H,K)
C
C H += S1(E) + Choice(E,F,G) + K + W
C D += H
C H += S0(A) + Majority(A,B,C)
C
C Where
C
C S1(E) = E<<<26 ^ E<<<21 ^ E<<<7
C S0(A) = A<<<30 ^ A<<<19 ^ A<<<10
C Choice (E, F, G) = G^(E&(F^G))
C Majority (A,B,C) = (A&B) + (C&(A^B))

define(`ROUND', `
	movl	$5, T0
	movl	$5, T1
	roll	`$'7, T0
	roll	`$'21, T1
	xorl	T0, T1
	roll	`$'19, T0
	xorl	T0, T1
	addl	W, $8
	addl	T1, $8
	movl	$7, T0
	xorl	$6, T0
	andl	$5, T0
	xorl	$7, T0
	addl	OFFSET($9)(K,COUNT,4), $8
	addl	T0, $8
	addl	$8, $4

	movl	$1, T0
	movl	$1, T1
	roll	`$'10, T0
	roll	`$'19, T1
	xorl	T0, T1
	roll	`$'20, T0
	xorl	T0, T1
	addl	T1, $8
	movl	$1, T0
	movl	$1, T1
	andl	$2, T0
	xorl	$2, T1
	addl	T0, $8
	andl	$3, T1
	addl	T1, $8
')

define(`NOEXPN', `
	movl	OFFSET($1)(INPUT, COUNT, 4), W
	bswapl	W
	movl	W, OFFSET($1)(%rsp, COUNT, 4)
')

	C const uint8_t *
	C _nettle_sha256_compress_n(uint32_t *state, const uint32_t *k,
	C                           size_t blocks, const uint8_t *input)

	.text
	ALIGN(16)

PROLOGUE(_nettle_sha256_compress_n)
	W64_ENTRY(4, 0)
	test	BLOCKS, BLOCKS
	jz	.Lend

	sub	$120, %rsp
	mov	STATE, STATE_SAVED	C Save state, to free a register
	mov	%rbx, 72(%rsp)
	mov	%rbp, 80(%rsp)
	mov	%r12, 88(%rsp)
	mov	%r13, 96(%rsp)
	mov	%r14, 104(%rsp)
	mov	%r15, 112(%rsp)

	movl	(STATE),   SA
	movl	4(STATE),  SB
	movl	8(STATE),  SC
	movl	12(STATE), SD
	movl	16(STATE), SE
	movl	20(STATE), SF
	movl	24(STATE), SG
	movl	28(STATE), SH

.Loop_block:
	xorl	XREG(COUNT), XREG(COUNT)
	ALIGN(16)

.Loop1:
	NOEXPN(0) ROUND(SA,SB,SC,SD,SE,SF,SG,SH,0)
	NOEXPN(1) ROUND(SH,SA,SB,SC,SD,SE,SF,SG,1)
	NOEXPN(2) ROUND(SG,SH,SA,SB,SC,SD,SE,SF,2)
	NOEXPN(3) ROUND(SF,SG,SH,SA,SB,SC,SD,SE,3)
	NOEXPN(4) ROUND(SE,SF,SG,SH,SA,SB,SC,SD,4)
	NOEXPN(5) ROUND(SD,SE,SF,SG,SH,SA,SB,SC,5)
	NOEXPN(6) ROUND(SC,SD,SE,SF,SG,SH,SA,SB,6)
	NOEXPN(7) ROUND(SB,SC,SD,SE,SF,SG,SH,SA,7)
	addl	$8, XREG(COUNT)
	cmpl	$16, XREG(COUNT)
	jne	.Loop1

.Loop2:
	EXPN( 0) ROUND(SA,SB,SC,SD,SE,SF,SG,SH,0)
	EXPN( 1) ROUND(SH,SA,SB,SC,SD,SE,SF,SG,1)
	EXPN( 2) ROUND(SG,SH,SA,SB,SC,SD,SE,SF,2)
	EXPN( 3) ROUND(SF,SG,SH,SA,SB,SC,SD,SE,3)
	EXPN( 4) ROUND(SE,SF,SG,SH,SA,SB,SC,SD,4)
	EXPN( 5) ROUND(SD,SE,SF,SG,SH,SA,SB,SC,5)
	EXPN( 6) ROUND(SC,SD,SE,SF,SG,SH,SA,SB,6)
	EXPN( 7) ROUND(SB,SC,SD,SE,SF,SG,SH,SA,7)
	EXPN( 8) ROUND(SA,SB,SC,SD,SE,SF,SG,SH,8)
	EXPN( 9) ROUND(SH,SA,SB,SC,SD,SE,SF,SG,9)
	EXPN(10) ROUND(SG,SH,SA,SB,SC,SD,SE,SF,10)
	EXPN(11) ROUND(SF,SG,SH,SA,SB,SC,SD,SE,11)
	EXPN(12) ROUND(SE,SF,SG,SH,SA,SB,SC,SD,12)
	EXPN(13) ROUND(SD,SE,SF,SG,SH,SA,SB,SC,13)
	EXPN(14) ROUND(SC,SD,SE,SF,SG,SH,SA,SB,14)
	EXPN(15) ROUND(SB,SC,SD,SE,SF,SG,SH,SA,15)
	addl	$16, XREG(COUNT)
	cmpl	$64, XREG(COUNT)
	jne	.Loop2

	mov	STATE_SAVED, STATE

	addl	(STATE), SA
	addl	4(STATE), SB
	addl	8(STATE), SC
	addl	12(STATE), SD
	addl	16(STATE), SE
	addl	20(STATE), SF
	addl	24(STATE), SG
	addl	28(STATE), SH

	movl	SA, (STATE)
	movl	SB, 4(STATE)
	movl	SC, 8(STATE)
	movl	SD, 12(STATE)
	movl	SE, 16(STATE)
	movl	SF, 20(STATE)
	movl	SG, 24(STATE)
	movl	SH, 28(STATE)

	add	$64, INPUT
	dec	BLOCKS
	jnz	.Loop_block

	mov	72(%rsp), %rbx
	mov	80(%rsp), %rbp
	mov	88(%rsp), %r12
	mov	96(%rsp), %r13
	mov	104(%rsp),%r14
	mov	112(%rsp),%r15

	add	$120, %rsp
.Lend:
	mov	INPUT, %rax
	W64_EXIT(4, 0)
	ret
EPILOGUE(_nettle_sha256_compress_n)
