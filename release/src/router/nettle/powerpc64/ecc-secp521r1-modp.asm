C powerpc64/ecc-secp521r1-modp.asm

ifelse(`
   Copyright (C) 2021 Martin Schwenke & Alastair D´Silva, IBM Corporation

   Based on x86_64/ecc-secp521r1-modp.asm

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

define(`SP', `r1')

define(`RP', `r4')
define(`XP', `r5')

define(`U0', `r6')
define(`U1', `r7')
define(`U2', `r8')
define(`U3', `r9')
define(`U4', `r10')
define(`U5', `r11')
define(`U6', `r12')
define(`U7', `r14')
define(`U8', `r15')
define(`U9', `r16')

define(`T0', `r3')
define(`T1', `r17')


	C void ecc_secp521r1_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_secp521r1_modp)

	std	r14, -32(SP)
	std	r15, -24(SP)
	std	r16, -16(SP)
	std	r17, -8(SP)

	C Read top 17 limbs, shift left 55 bits
	ld	U1, 72(XP)
	sldi	U0, U1, 55
	srdi	U1, U1, 9

	ld	T0, 80(XP)
	srdi	U2, T0, 9
	sldi	T0, T0, 55
	or	U1, T0, U1

	ld	T0, 88(XP)
	srdi	U3, T0, 9
	sldi	T0, T0, 55
	or	U2, T0, U2

	ld	T0, 96(XP)
	srdi	U4, T0, 9
	sldi	T0, T0, 55
	or	U3, T0, U3

	ld	T0, 104(XP)
	srdi	U5, T0, 9
	sldi	T0, T0, 55
	or	U4, T0, U4

	ld	T0, 112(XP)
	srdi	U6, T0, 9
	sldi	T0, T0, 55
	or	U5, T0, U5

	ld	T0, 120(XP)
	srdi	U7, T0, 9
	sldi	T0, T0, 55
	or	U6, T0, U6

	ld	T0, 128(XP)
	srdi	U8, T0, 9
	sldi	T0, T0, 55
	or	U7, T0, U7

	ld	T0, 136(XP)
	srdi	U9, T0, 9
	sldi	T0, T0, 55
	or	U8, T0, U8

	ld	T0, 0(XP)
	ld	T1, 8(XP)
	addc	U0, T0, U0
	adde	U1, T1, U1
	ld	T0, 16(XP)
	ld	T1, 24(XP)
	adde	U2, T0, U2
	adde	U3, T1, U3
	ld	T0, 32(XP)
	ld	T1, 40(XP)
	adde	U4, T0, U4
	adde	U5, T1, U5
	ld	T0, 48(XP)
	ld	T1, 56(XP)
	adde	U6, T0, U6
	adde	U7, T1, U7
	ld	T0, 64(XP)
	adde	U8, T0, U8
	addze	U9, U9

	C Top limbs are <U9, U8>. Keep low 9 bits of 8, and fold the
	C top bits (at most 65 bits).
	srdi	T0, U8, 9
	andi.	U8, U8, 0x1ff
	srdi	T1, U9, 9
	sldi	U9, U9, 55
	or	T0, U9, T0

	addc	U0, T0, U0
	adde	U1, T1, U1
	addze	U2, U2
	addze	U3, U3
	addze	U4, U4
	addze	U5, U5
	addze	U6, U6
	addze	U7, U7
	addze	U8, U8

	std	U0, 0(RP)
	std	U1, 8(RP)
	std	U2, 16(RP)
	std	U3, 24(RP)
	std	U4, 32(RP)
	std	U5, 40(RP)
	std	U6, 48(RP)
	std	U7, 56(RP)
	std	U8, 64(RP)

	ld	r14, -32(SP)
	ld	r15, -24(SP)
	ld	r16, -16(SP)
	ld	r17, -8(SP)

	blr
EPILOGUE(_nettle_ecc_secp521r1_modp)
