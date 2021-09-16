C x86_64/ecc-secp521r1-modp.asm

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

	.file "ecc-secp521r1-modp.asm"

GMP_NUMB_BITS(64)

define(`RP', `%rsi')
define(`XP', `%rdx')

define(`U0', `%rax')
define(`U1', `%rbx')
define(`U2', `%rcx')
define(`U3', `%rbp')
define(`U4', `%rdi')
define(`U5', `%r8')
define(`U6', `%r9')
define(`U7', `%r10')
define(`U8', `%r11')
define(`U9', `%r12')
define(`T0', `%r13')
define(`T1', `%r14')

PROLOGUE(_nettle_ecc_secp521r1_modp)
	W64_ENTRY(3, 0)
	push	%rbx
	push	%rbp
	push	%r12
	push	%r13
	push	%r14

	C Read top 17 limbs, shift left 55 bits
	mov	72(XP), U1
	mov	U1, U0
	shl	$55, U0
	shr	$9, U1

	mov	80(XP), U2
	mov	U2, T0
	shr	$9, U2
	shl	$55, T0
	or	T0, U1

	mov	88(XP), U3
	mov	U3, T0
	shr	$9, U3
	shl	$55, T0
	or	T0, U2

	mov	96(XP), U4
	mov	U4, T0
	shr	$9, U4
	shl	$55, T0
	or	T0, U3

	mov	104(XP), U5
	mov	U5, T0
	shr	$9, U5
	shl	$55, T0
	or	T0, U4

	mov	112(XP), U6
	mov	U6, T0
	shr	$9, U6
	shl	$55, T0
	or	T0, U5

	mov	120(XP), U7
	mov	U7, T0
	shr	$9, U7
	shl	$55, T0
	or	T0, U6

	mov	128(XP), U8
	mov	U8, T0
	shr	$9, U8
	shl	$55, T0
	or	T0, U7

	mov	136(XP), U9
	mov	U9, T0
	shr	$9, U9
	shl	$55, T0
	or	T0, U8

	add	  (XP), U0
	adc	 8(XP), U1
	adc	16(XP), U2
	adc	24(XP), U3
	adc	32(XP), U4
	adc	40(XP), U5
	adc	48(XP), U6
	adc	56(XP), U7
	adc	64(XP), U8
	adc	$0, U9

	C Top limbs are <U9, U8>. Keep low 9 bits of 8, and fold the
	C top bits (at most 65 bits).
	mov	U8, T0
	shr	$9, T0
	and	$0x1ff, U8
	mov	U9, T1
	shl	$55, U9
	shr	$9, T1
	or	U9, T0

	add	T0, U0
	mov	U0, (RP)
	adc	T1, U1
	mov	U1, 8(RP)
	adc	$0, U2
	mov	U2, 16(RP)
	adc	$0, U3
	mov	U3, 24(RP)
	adc	$0, U4
	mov	U4, 32(RP)
	adc	$0, U5
	mov	U5, 40(RP)
	adc	$0, U6
	mov	U6, 48(RP)
	adc	$0, U7
	mov	U7, 56(RP)
	adc	$0, U8
	mov	U8, 64(RP)

	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbp
	pop	%rbx
	W64_EXIT(3, 0)
	ret
EPILOGUE(_nettle_ecc_secp521r1_modp)
