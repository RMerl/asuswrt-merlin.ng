C x86_64/ghash-set-key.asm

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

C Common registers

define(`CTX', `%rdi')
define(`KEY', `%rsi')
define(`P', `%xmm0')
define(`BSWAP', `%xmm1')
define(`H', `%xmm2')
define(`D', `%xmm3')
define(`T', `%xmm4')
define(`MASK', `%xmm5')

    C void _ghash_set_key (struct gcm_key *ctx, const union nettle_block16 *key)

PROLOGUE(_nettle_ghash_set_key)
	W64_ENTRY(2, 6)
	movdqa	.Lpolynomial(%rip), P
	movdqa	.Lbswap(%rip), BSWAP
	movups	(KEY), H
	pshufb	BSWAP, H
	C Multiply by x mod P, which is a left shift.
	movdqa	H, T
	psllq	$1, T
	psrlq	$63, H		C 127 --> 64, 63 --> 0
	pshufd	$0xaa, H, MASK	C 64 --> (96, 64, 32, 0)
	pslldq	$8, H		C 0 --> 64
	por	T, H
	pxor	T, T
	psubd	MASK, T		C All-ones if bit 127 was set
	pand	P, T
	pxor	T, H
	movups	H, (CTX)

	C Set D = x^{-64} H = {H0, H1} + P1 H0
	pshufd	$0x4e, H, D	C Swap H0, H1
	pclmullqhqdq P, H
	pxor	H, D
	movups	D, 16(CTX)
	W64_EXIT(2, 6)
	ret
EPILOGUE(_nettle_ghash_set_key)

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
