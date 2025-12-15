C x86_64/poly1305-internal.asm

ifelse(`
   Copyright (C) 2013 Niels Möller

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

	.file "poly1305-internal.asm"

C Registers mainly used by poly1305_block
define(`CTX', `%rdi') C First argument to all functions

define(`KEY', `%rsi')
define(`MASK',` %r8')
	C _poly1305_set_key(struct poly1305_ctx *ctx, const uint8_t key[16])
	.text
	ALIGN(16)
PROLOGUE(_nettle_poly1305_set_key)
	W64_ENTRY(2,0)
	mov	$0x0ffffffc0fffffff, MASK
	mov	(KEY), %rax
	and	MASK, %rax
	and	$-4, MASK
	mov	%rax, P1305_R0 (CTX)
	imul	$5, %rax
	mov	%rax, P1305_S0 (CTX)	C 5*R0
	mov	8(KEY), %rax
	and	MASK, %rax
	mov	%rax, P1305_R1 (CTX)
	shr	$2, %rax
	imul	$5, %rax
	mov	%rax, P1305_S1 (CTX)	C 5*(R1>>2)
	xor	XREG(%rax), XREG(%rax)
	mov	%rax, P1305_H0 (CTX)
	mov	%rax, P1305_H1 (CTX)
	mov	%rax, P1305_H2 (CTX)
	
	W64_EXIT(2,0)
	ret

undefine(`KEY')
undefine(`MASK')

EPILOGUE(_nettle_poly1305_set_key)

define(`T0', `%rcx')
define(`T1', `%rsi')	C Overlaps message input pointer.
define(`T2', `%r8')
define(`H0', `%r9')
define(`H1', `%r10')
define(`F0', `%r11')
define(`F1', `%r12')

C First accumulate the independent products
C
C {H1,H0} = R0 T0 + S1 T1 + S0 (T2 >> 2)
C {F1,F0} = R1 T0 + R0 T1 + S1 T2
C T = R0 * (T2 & 3)
C
C Then add together as
C
C     +--+--+--+
C     |T |H1|H0|
C     +--+--+--+
C   + |F1|F0|
C   --+--+--+--+
C     |H2|H1|H0|
C     +--+--+--+

	C _poly1305_block (struct poly1305_ctx *ctx, const uint8_t m[16], unsigned hi)
	
PROLOGUE(_nettle_poly1305_block)
	W64_ENTRY(3, 0)
	push	%r12
	mov	(%rsi), T0
	mov	8(%rsi), T1
	mov	XREG(%rdx), XREG(T2)	C Also zero extends

	add	P1305_H0 (CTX), T0
	adc	P1305_H1 (CTX), T1
	adc	P1305_H2 (CTX), T2

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

	mov	T2, %rax
	mul	T0			C S1*T2
	add	%rax, F0
	adc	%rdx, F1

	mov	$3, XREG(T1)
	and	T2, T1

	shr	$2, T2
	mov	P1305_S0 (CTX), %rax
	mul	T2			C S0*(T2 >> 2)
	add	%rax, H0
	adc	%rdx, H1

	imul	P1305_R0 (CTX), T1	C R0*(T2 & 3)
	add	F0, H1
	adc	T1, F1

	mov	H0, P1305_H0 (CTX)
	mov	H1, P1305_H1 (CTX)
	mov	F1, P1305_H2 (CTX)
	pop	%r12
	W64_EXIT(3, 0)
	ret
EPILOGUE(_nettle_poly1305_block)
undefine(`T0')
undefine(`T1')
undefine(`T2')
undefine(`H0')
undefine(`H1')
undefine(`F0')
undefine(`F1')

	C _poly1305_digest (struct poly1305_ctx *ctx, uint8_t *s)
define(`S', `%rsi')

define(`T0', `%rcx')
define(`T1', `%r8')
define(`H0', `%r9')
define(`H1', `%r10')
define(`F0', `%r11')
define(`F1', `%rrd')	C Overlaps CTX

PROLOGUE(_nettle_poly1305_digest)
	W64_ENTRY(2, 0)

	mov	P1305_H0 (CTX), H0
	mov	P1305_H1 (CTX), H1
	mov	P1305_H2 (CTX), F0

	xor	XREG(%rax), XREG(%rax)
	mov	%rax, P1305_H0 (CTX)
	mov	%rax, P1305_H1 (CTX)
	mov	%rax, P1305_H2 (CTX)

	mov	$3, XREG(%rax)
	and 	XREG(F0), XREG(%rax)
	shr	$2, F0
	imul	$5, F0
	add	F0, H0
	adc	$0, H1
	adc	$0, XREG(%rax)

	C Add 5, use result if >= 2^130
	mov	$5, T0
	xor	T1, T1
	add	H0, T0
	adc	H1, T1
	adc	$-4, XREG(%rax)		C Carry if %rax + c >= 4
	cmovc	T0, H0
	cmovc	T1, H1

	add	H0, (S)
	adc	H1, 8(S)

	W64_EXIT(2, 0)
	ret
EPILOGUE(_nettle_poly1305_digest)

