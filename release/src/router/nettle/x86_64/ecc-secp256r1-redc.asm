C x86_64/ecc-secp256r1-redc.asm

ifelse(`
   Copyright (C) 2013, 2021 Niels Möller

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

	.file "ecc-secp256r1-redc.asm"

define(`RP', `%rsi')
define(`XP', `%rdx')

define(`U0', `%rdi') C Overlaps unused modulo input
define(`U1', `%rcx')
define(`U2', `%rax')
define(`U3', `%r8')
define(`F0', `%r9')
define(`F1', `%r10')
define(`F2', `%r11')
define(`F3', `%rdx') C Overlap XP, used only in final carry folding

C FOLD(x), sets (x,F2,F1,F0 )  <--  (x << 192) - (x << 160) + (x << 128) + (x << 32)
define(`FOLD', `
	mov	$1, F0
	mov	$1, F1
	mov	$1, F2
	shl	`$'32, F0
	shr	`$'32, F1
	sub	F0, F2
	sbb	F1, $1
')
C FOLDC(x), sets (x,F2,F1,F0)  <--  ((x+c) << 192) - (x << 160) + (x << 128) + (x << 32)
define(`FOLDC', `
	mov	$1, F0
	mov	$1, F1
	mov	$1, F2
	adc	`$'0, $1	C May overflow, but final result will not.
	shl	`$'32, F0
	shr	`$'32, F1
	sub	F0, F2
	sbb	F1, $1
')
PROLOGUE(_nettle_ecc_secp256r1_redc)
	W64_ENTRY(3, 0)

	mov	(XP), U0
	FOLD(U0)
	mov	8(XP), U1
	mov	16(XP), U2
	mov	24(XP), U3
	add	F0, U1
	adc	F1, U2
	adc	F2, U3
	adc	32(XP), U0

	FOLDC(U1)
	add	F0, U2
	adc	F1, U3
	adc	F2, U0
	adc	40(XP), U1

	FOLDC(U2)
	add	F0, U3
	adc	F1, U0
	adc	F2, U1
	adc	48(XP), U2

	FOLDC(U3)
	add	F0, U0
	adc	F1, U1
	adc	F2, U2
	adc	56(XP), U3

	C Sum, including carry, is < 2^{256} + p.
	C If carry, we need to add in 2^{256} mod p = 2^{256} - p
	C     = <0xfffffffe, 0xff..ff, 0xffffffff00000000, 1>
	C and this addition can not overflow.
	sbb	F2, F2
	mov	F2, F0
	mov	F2, F1
	mov	XREG(F2), XREG(F3)
	neg	F0
	shl	$32, F1
	and	$-2, XREG(F3)

	add	F0, U0
	mov	U0, (RP)
	adc	F1, U1
	mov	U1, 8(RP)
	adc	F2, U2
	mov	U2, 16(RP)
	adc	F3, U3

	mov	U3, 24(RP)

	W64_EXIT(3, 0)
	ret
EPILOGUE(_nettle_ecc_secp256r1_redc)
