C x86_64/poly1305-blocks.asm

ifelse(`
   Copyright (C) 2022 Niels Möller

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

	.file "poly1305-blocks.asm"

define(`CTX', `%rdi') C First argument to all functions
define(`BLOCKS', `%rsi')
define(`MP_PARAM', `%rdx')	C Moved to MP, to not collide with mul instruction.

define(`MP', `%r8')		C May clobber, both with unix and windows conventions.
define(`T0', `%rbx')
define(`T1', `%rcx')
define(`H0', `%rbp')
define(`H1', `%r9')
define(`H2', `%r10')
define(`F0', `%r11')
define(`F1', `%r12')

C const uint8_t *
C _nettle_poly1305_blocks (struct poly1305_ctx *ctx, size_t blocks, const uint8_t *m)

PROLOGUE(_nettle_poly1305_blocks)
	W64_ENTRY(3, 0)
	mov	MP_PARAM, MP
	test	BLOCKS, BLOCKS
	jz	.Lend

	push 	%rbx
	push 	%rbp
	push	%r12
	mov	P1305_H0 (CTX), H0
	mov	P1305_H1 (CTX), H1
	mov	P1305_H2 (CTX), H2
	ALIGN(16)
.Loop:
	mov	(MP), T0
	mov	8(MP), T1
	add	$16, MP

	add	H0, T0
	adc	H1, T1
	adc	$1, H2

	mov	P1305_R1 (CTX), %rax
	mul	T0			C R1*T0
	mov	%rax, F0
	mov	%rdx, F1

	mov	T0, %rax		C Last use of T0 input
	mov	P1305_R0 (CTX), T0
	mul	T0			C R0*T0
	mov	%rax, H0
	mov	%rdx, H1

	mov	T1, %rax
	mul	T0			C R0*T1
	add	%rax, F0
	adc	%rdx, F1

	mov	P1305_S1 (CTX), T0
	mov	T1, %rax		C Last use of T1 input
	mul	T0			C S1*T1
	add	%rax, H0
	adc	%rdx, H1

	mov	H2, %rax
	mul	T0			C S1*H2
	add	%rax, F0
	adc	%rdx, F1

	mov	H2, T0
	and	$3, H2

	shr	$2, T0
	mov	P1305_S0 (CTX), %rax
	mul	T0			C S0*(H2 >> 2)
	add	%rax, H0
	adc	%rdx, H1

	imul	P1305_R0 (CTX), H2	C R0*(H2 & 3)
	add 	F0, H1
	adc	F1, H2

	dec	BLOCKS
	jnz	.Loop

	mov	H0, P1305_H0 (CTX)
	mov	H1, P1305_H1 (CTX)
	mov	H2, P1305_H2 (CTX)

	pop	%r12
	pop	%rbp
	pop 	%rbx

.Lend:
	mov	MP, %rax
	W64_EXIT(3, 0)
	ret
EPILOGUE(_nettle_poly1305_blocks)
