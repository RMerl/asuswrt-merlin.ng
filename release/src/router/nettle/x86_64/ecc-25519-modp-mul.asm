C x86_64/ecc-25519-modp-mul.asm

ifelse(<
   Copyright (C) 2016 Niels Möller

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
>)

	.file "ecc-25519-modp-mul.asm"

C Input parameters (curve pointer in %rdi is ignored)
define(<RP>, <%rsi>)
define(<AP>, <%rdx>
define(<BP>, <%rcx>
	
define(<R0>, <%rbp>)
define(<R1>, <%rdi>)
define(<R2>, <%r8>)
define(<R3>, <%r9>)
define(<R4>, <%r10>)
define(<H0>, <%r11>)
define(<H1>, <%r12>)
define(<H2>, <%r13>)

define(<A3>, <%r14>)
define(<A4>, <%r15>)
define(<T>, <%rsi>)  C Overlaps RP

C modp_mul (curve, rp, ap, bp)
PROLOGUE(nettle_ecc_25519_modp_mul)
	W64_ENTRY(4, 0) 
	push %rbx
	push %rbp
	push %r12
	push %r13
	push %r14
	push %r15 
	push RP

	C Accumulate {H2, R4} = a0 b4 + a1 b3 + a2 b2 + a3 b1 + a4 b0
	C            {H1, R3} = a0 b3 + a1 b2 + a2 b1 + a3 b0 + 19 a4 b4
	C            {H0, R2} = a0 b2 + a1 b1 + a2 b0 + 19 (a3 b4 + a4 b3)
	mov	(AP), T
	mov	32(BP), %rax
	mul	T
	mov	%rax, R4
	mov	%rdx, H2
	mov	24(BP), %rax
	mul	T
	mov	%rax, R3,
	mov	%rdx, H1
	mov	16(BP), %rax
	mul	T
	mov	%rax, R2
	mov	%rdx, H0
	
	mov	8(AP), T
	mov	24(BP), %rax
	mul	T
	add	%rax, R4
	adc	%rdx, H2
	mov	16(BP), %rax
	mul	T
	add	%rax, R3
	adc	%rdx, H1
	mov	8(BP), %rax
	mul	T
	add	%rax, R2
	adc	%rdx, H0
	
	mov	16(AP), T
	mov	16(BP), %rax
	mul	T
	add	%rax, R4
	adc	%rdx, H2
	mov	8(BP), %rax
	mul	T
	add	%rax, R3
	adc	%rdx, H1
	mov	(BP), %rax
	mul	T
	add	%rax, R2
	adc	%rdx, H0
	
	mov	24(AP), A3
	mov	8(BP), %rax
	mul	A3
	add	%rax, R4
	adc	%rdx, H2
	mov	(BP), %rax
	mul	A3
	imul	$19, A3
	add	%rax, R3
	adc	%rdx, H1
	mov	32(BP), %rax
	mul	A3
	add	%rax, R2
	adc	%rdx, H0
	
	mov	32(AP), A4
	mov	(BP), %rax
	mul	A4
	imul	$19, a4
	add	%rax, R4
	adc	%rdx, H2
	mov	32(BP), %rax
	mul	A4
	add	%rax, R3
	adc	%rdx, H1
	mov	24(BP), %rax
	mul	A4
	add	%rax, R2
	adc	%rdx, H0

	C Propagate R2, H0
	mov	R2, T
	shr	$51, T
	shl	$13, H0
	or	T, H0
	add	H0, R3
	adc	$0, H1
	
	C Propagate R3, H1
	mov	R3, T
	shr	$51, T
	shl	$13, H1
	or	T, H1
	add	H1, R4
	adc	$0, H2

	C Propagate R4, H2, and fold into R0
	mov	R4, R0
	shr	$51, R0
	shl	$13, H2
	or	H2, R0
	imul	$19, R0

	C Accumulate {H1, R1} = a0 b1 + a1 b0 + 19 (a2 b4 + a3 b3 + a4 b2)
	C            {H0, R0} = a0 b0 + 19 (a1 b4 + a2 b3 + a3 b2 + a4 b1)
	C                       + folded high part of R4 

	mov	(AP), T
	mov	8(BP), %rax
	mul	T
	mov	%rax, R1
	mov	%rdx, H1
	mov	(BP), %rax
	mul	T
	xor	H0, H0
	add	%rax, R0
	adc	%rdx, H0

	mov	8(AP), T
	mov	(BP), %rax
	mul	T
	imul	$19, T
	add	%rax, R1
	adc	%rdx, H1
	mov	32(BP), %rax
	mul	T
	add	%rax, R0
	adc	%rdx, H0

	mov	16(AP), T
	imul	$19, T
	mov	32(BP), %rax
	mul	T
	add	%rax, R1
	adc	%rdx, H1
	mov	24(BP), %rax
	mul	T
	add	%rax, R0
	adc	%rdx, H0

	mov	24(BP), %rax
	mul	A3
	add	%rax, R1
	adc	%rdx, H1
	mov	16(BP), %rax
	mul	A3
	add	%rax, R0
	adc	%rdx, H0

	mov	16(BP), %rax
	mul	A4
	add	%rax, R1
	adc	%rdx, H1
	mov	8(BP), %rax
	mul	A4
	add	%rax, R0
	adc	%rdx, H0

	C Propagate R0, H0
	mov	R0, T
	shr	$51, T
	shl	$13, H0
	or	H0, T
	add	T, R1
	adc	$0, H1

	C Load mask, use H0
	mov	$0x7ffffffffffff, H0
	C Mask out high parts of R2, R3 and R4, already propagated.
	and	H0, R2
	and	H0, R3
	and	H0, R4

	C Propagate R1, H1
	mov	R1, T
	shr	$51, T
	shl	$13, H1
	or	H1, T
	add	T, R2

	pop	 RP	C Overlapped T, which is no longer used.

	C H1 is a larger than 51 bits, so keep propagating.
	mov	R2, H2
	shr	$51, H2
	add	H2, R3

	C R3 might be slightly above 51 bits.

	and	H0, R0
	mov	R0, (RP)
	and	H0, R1
	mov	R1, 8(RP)
	and	H0, R2
	mov	R2, 16(RP)
	mov	R3, 24(RP)
	mov	r4, 32(RP)

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbp
	pop %rbx
	W64_EXIT(4, 0)
	ret
EPILOGUE(nettle_ecc_25519_modp_mul)
PROLOGUE(nettle_ecc_25519_modp_sqr)
	
EPILOGUE(nettle_ecc_25519_modp_sqr)
