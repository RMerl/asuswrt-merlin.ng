C powerpc/ecc-curve448-modp.asm

ifelse(`
   Copyright (C) 2021 Martin Schwenke & Amitay Isaacs, IBM Corporation

   Based on x86_64/ecc-curve448-modp.asm

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

	.file "ecc-curve448-modp.asm"

define(`SP', `r1')

define(`RP', `r4')
define(`XP', `r5')

define(`X0', `r3')
define(`X1', `r9')
define(`X2', `r10')
define(`X3', `r11')
define(`X4', `r12')
define(`X5', `r14')
define(`X6', `r15')
define(`X7', `r16')
define(`T0', `r6')
define(`T1', `r7')
define(`T2', `r8')
define(`TT', `r17')

define(`LO', `TT')	C Overlap

	C void ecc_curve448_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_curve448_modp)

	std	r14, -32(SP)
	std	r15, -24(SP)
	std	r16, -16(SP)
	std	r17, -8(SP)

	C First load the values to be shifted by 32.
	ld	T0, 88(XP)	C use for X0, X1
	ld	T1, 96(XP)	C use for X2
	ld	T2, 104(XP)	C use for X3
	ld	X4, 56(XP)
	ld	X5, 64(XP)
	ld	X6, 72(XP)
	ld	X7, 80(XP)

	C Multiply by 2^32
	sldi	X0, T0, 32
	srdi	LO, T0, 32
	sldi	X1, T1, 32
	or	X1, X1, LO
	srdi	LO, T1, 32
	sldi	X2, T2, 32
	or	X2, X2, LO
	srdi	LO, T2, 32
	sldi	X3, X4, 32
	or	X3, X3, LO
	srdi	LO, X4, 32
	sldi	X4, X5, 32
	or	X4, X4, LO
	srdi	LO, X5, 32
	sldi	X5, X6, 32
	or	X5, X5, LO
	srdi	LO, X6, 32
	sldi	X6, X7, 32
	or	X6, X6, LO

	srdi	X7, X7, 32

	C Multiply by 2
	addc	T0, T0, T0
	adde	T1, T1, T1
	adde	T2, T2, T2
	addze	X7, X7

	C Main additions
	ld	TT, 56(XP)
	addc	X0, TT, X0
	ld	TT, 64(XP)
	adde	X1, TT, X1
	ld	TT, 72(XP)
	adde	X2, TT, X2
	ld	TT, 80(XP)
	adde	X3, TT, X3
	adde	X4, T0, X4
	adde	X5, T1, X5
	adde	X6, T2, X6
	addze	X7, X7

	ld	T0, 0(XP)
	addc	X0, T0, X0
	ld	T1, 8(XP)
	adde	X1, T1, X1
	ld	T2, 16(XP)
	adde	X2, T2, X2
	ld	TT, 24(XP)
	adde	X3, TT, X3
	ld	T0, 32(XP)
	adde	X4, T0, X4
	ld	T1, 40(XP)
	adde	X5, T1, X5
	ld	T2, 48(XP)
	adde	X6, T2, X6
	addze	X7, X7

	C X7 wraparound
	sldi	T0, X7, 32
	srdi	T1, X7, 32
	li	T2, 0
	addc	X0, X7, X0
	addze	X1, X1
	addze	X2, X2
	adde	X3, T0, X3
	adde	X4, T1, X4
	addze	X5, X5
	addze	X6, X6
	addze	T2, T2

	C Final carry wraparound. Carry T2 > 0 only if
	C X6 is zero, so carry is absorbed.
	sldi	T0, T2, 32

	addc	X0, T2, X0
	addze	X1, X1
	addze	X2, X2
	adde	X3, T0, X3
	addze	X4, X4
	addze	X5, X5
	addze	X6, X6

	std	X0, 0(RP)
	std	X1, 8(RP)
	std	X2, 16(RP)
	std	X3, 24(RP)
	std	X4, 32(RP)
	std	X5, 40(RP)
	std	X6, 48(RP)

	ld	r14, -32(SP)
	ld	r15, -24(SP)
	ld	r16, -16(SP)
	ld	r17, -8(SP)

	blr
EPILOGUE(_nettle_ecc_curve448_modp)
