C powerpc64/ecc-25519-modp.asm

ifelse(`
   Copyright (C) 2021 Martin Schwenke & Alastair D´Silva, IBM Corporation

   Based on x86_64/ecc-25519-modp.asm

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

	.file "ecc-25519-modp.asm"

define(`RP', `r4')
define(`XP', `r5')

define(`U0', `r6')
define(`U1', `r7')
define(`U2', `r8')
define(`U3', `r9')
define(`T0', `r10')
define(`T1', `r11')
define(`M', `r12')

define(`UN', r3)	C Overlaps unused modulo input

	C void ecc_curve25519_modp (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_curve25519_modp)

	C First fold the limbs affecting bit 255
	ld	UN, 56(XP)
	li	M, 38
	mulhdu	T1, M, UN
	mulld	UN, M, UN
	ld	U3, 24(XP)
	li	T0, 0
	addc	U3, UN, U3
	adde	T0, T1, T0

	ld	UN, 40(XP)
	mulhdu	U2, M, UN
	mulld	UN, M, UN

	addc	U3, U3, U3
	adde	T0, T0, T0
	srdi	U3, U3, 1	C Undo shift, clear high bit

	C Fold the high limb again, together with RP[5]
	li	T1, 19
	mulld	T0, T1, T0
	ld	U0, 0(XP)
	ld	U1, 8(XP)
	ld	T1, 16(XP)
	addc	U0, T0, U0
	adde	U1, UN, U1
	ld	T0, 32(XP)
	adde	U2, U2, T1
	addze	U3, U3

	mulhdu	T1, M, T0
	mulld	T0, M, T0
	addc	U0, T0, U0
	adde	U1, T1, U1
	std	U0, 0(RP)
	std	U1, 8(RP)

	ld	T0, 48(XP)
	mulhdu	T1, M, T0
	mulld	UN, M, T0
	adde	U2, UN, U2
	adde	U3, T1, U3
	std	U2, 16(RP)
	std	U3, 24(RP)

	blr
EPILOGUE(_nettle_ecc_curve25519_modp)
