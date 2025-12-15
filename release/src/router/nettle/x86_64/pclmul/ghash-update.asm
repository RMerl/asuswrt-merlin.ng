C x86_64/ghash-update.asm

ifelse(`
   Copyright (C) 2022 Niels Möller
   Copyright (C) 2023 Mamone Tarsha

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

C Common registers

define(`CTX', `%rdi')
define(`X', `%rsi')
define(`BLOCKS', `%rdx')
define(`DATA', `%rcx')

define(`P', `%xmm0')
define(`BSWAP', `%xmm1')
define(`H', `%xmm2')
define(`D', `%xmm3')
define(`H2', `%xmm4')
define(`D2', `%xmm5')
define(`T', `%xmm6')
define(`R', `%xmm7')
define(`M', `%xmm8')
define(`F', `%xmm9')
define(`T2', `%xmm10')
define(`R2', `%xmm11')
define(`M2', `%xmm12')
define(`F2', `%xmm13')

C Use pclmulqdq, doing one 64x64 --> 127 bit carry-less multiplication,
C with source operands being selected from the halves of two 128-bit registers.
C Variants:
C  pclmullqlqdq low half of both src and destination
C  pclmulhqlqdq low half of src register, high half of dst register
C  pclmullqhqdq high half of src register, low half of dst register
C  pclmulhqhqdq high half of both src and destination

C To do a single block, M0, M1, we need to compute
C
C R = M0 D1 + M1 H1
C F = M0 D0 + M1 H0
C
C Corresponding to x^{-127} M H = R + x^{-64} F
C
C Split F as F = F1 + x^64 F0, then the final reduction is
C
C R + x^{-64} F = R + P1 F0 + x^{64} F0 + F1
C
C In all, 5 pclmulqdq. If we we have enough registers to interleave two blocks,
C final reduction is needed only once, so 9 pclmulqdq for two blocks, etc.
C
C We need one register each for D and H, one for P1, one each for accumulating F
C and R. That uses 5 out of the 16 available xmm registers. If we interleave
C blocks, we need additionan D ang H registers (for powers of the key) and the
C additional message word, but we could perhaps interlave as many as 4, with two
C registers left for temporaries.

	C const uint8_t *_ghash_update (const struct gcm_key *ctx,
	C				union nettle_block16 *x,
	C				size_t blocks, const uint8_t *data)

PROLOGUE(_nettle_ghash_update)
	W64_ENTRY(4, 14)
	movdqa		.Lpolynomial(%rip), P
	movdqa		.Lbswap(%rip), BSWAP
	movups		(CTX), H
	movups		16(CTX), D
	movups		32(CTX), H2
	movups		48(CTX), D2
	movups		(X), R
	pshufb		BSWAP, R

	mov		BLOCKS, %rax
	shr		$1, %rax
	jz		.L1_block

.Loop:
	movups		(DATA), M
	pshufb		BSWAP, M
	pxor		M, R
	movdqa		R, M
	movdqa		R, F
	movdqa		R, T
	pclmullqlqdq	D2, F 	C {D^2}0 * M1_0
	pclmullqhqdq	D2, R	C {D^2}1 * M1_0
	pclmulhqlqdq	H2, T	C {H^2}0 * M1_1
	pclmulhqhqdq	H2, M	C {H^2}1 * M1_1
	

	movups		16(DATA), M2
	pshufb		BSWAP, M2
	movdqa		M2, R2
	movdqa		M2, F2
	movdqa		M2, T2
	pclmullqlqdq	D, F2 	C D0 * M2_0
	pclmullqhqdq	D, R2	C D1 * M2_0
	pclmulhqlqdq	H, T2	C H0 * M2_1
	pclmulhqhqdq	H, M2	C H1 * M2_1

	pxor		T, F
	pxor		M, R
	pxor		T2, F2
	pxor		M2, R2

	pxor		F2, F
	pxor		R2, R

	pshufd		$0x4e, F, T		C Swap halves of F
	pxor		T, R
	pclmullqhqdq	P, F
	pxor		F, R

	add		$32, DATA
	dec		%rax
	jnz		.Loop

.L1_block:
	test		$1, BLOCKS
	jz		.Ldone

	movups		(DATA), M
	pshufb		BSWAP, M
	pxor		M, R
	movdqa		R, M
	movdqa		R, F
	movdqa		R, T
	pclmullqlqdq	D, F 	C D0 * M0
	pclmullqhqdq	D, R	C D1 * M0
	pclmulhqlqdq	H, T	C H0 * M1
	pclmulhqhqdq	H, M	C H1 * M1
	pxor		T, F
	pxor		M, R

	pshufd		$0x4e, F, T		C Swap halves of F
	pxor		T, R
	pclmullqhqdq	P, F
	pxor		F, R

	add		$16, DATA

.Ldone:
	pshufb		BSWAP, R
	movups		R, (X)
	mov		DATA, %rax
	W64_EXIT(4, 14)
	ret
EPILOGUE(_nettle_ghash_update)

	RODATA
	C The GCM polynomial is x^{128} + x^7 + x^2 + x + 1,
	C but in bit-reversed representation, that is
	C P = x^{128}+ x^{127} + x^{126} + x^{121} + 1
	C We will mainly use the middle part,
	C P1 = (P + a + x^{128}) / x^64 = x^{563} + x^{62} + x^{57}
	ALIGN(16)
.Lpolynomial:
	.byte 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xC2
.Lbswap:
	.byte 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
