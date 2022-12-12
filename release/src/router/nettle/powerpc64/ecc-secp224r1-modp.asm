C powerpc64/ecc-secp224r1-modp.asm

ifelse(`
   Copyright (C) 2021 Amitay Isaacs, IBM Corporation

   Based on x86_64/ecc-secp224r1-modp.asm

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

	.file "ecc-secp224r1-modp.asm"

define(`SP', `r1')

define(`RP', `r4')
define(`XP', `r5')

define(`T0', `r6')
define(`T1', `r7')
define(`H0', `r8')
define(`H1', `r9')
define(`H2', `r10')
define(`F0', `r11')
define(`F1', `r12')
define(`F2', `r14')
define(`T2', `r3')

	C void ecc_secp224r1_modp (const struct ecc_modulo *m, mp_limb_t *rp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_secp224r1_modp)
	std	r14, -8(SP)

	ld	H0, 48(XP)
	ld	H1, 56(XP)
	C set (F2, F1, F0) <-- (H1, H0) << 32
	sldi	F0, H0, 32
	srdi	F1, H0, 32
	sldi	T0, H1, 32
	srdi	F2, H1, 32
	or	F1, T0, F1

	li	H2, 0
	ld	T0, 16(XP)
	ld	T1, 24(XP)
	subfc	T0, F0, T0
	subfe	T1, F1, T1
	subfe	H0, F2, H0
	addme	H1, H1

	ld	T2, 32(XP)
	addc	H0, T2, H0
	ld	T2, 40(XP)
	adde	H1, T2, H1
	addze	H2, H2

	C Set (F2, F1, F0) <-- (H2, H1, H0) << 32
	sldi	F0, H0, 32
	srdi	F1, H0, 32
	addc	H0, T0, H0
	sldi	T0, H1, 32
	srdi	F2, H1, 32
	adde	H1, T1, H1
	sldi	T1, H2, 32
	addze	H2, H2
	or	F1, T0, F1
	or	F2, T1, F2

	ld	T0, 0(XP)
	ld	T1, 8(XP)
	subfc	T0, F0, T0
	subfe	T1, F1, T1
	subfe	H0, F2, H0
	addme	H1, H1
	addme	H2, H2

	srdi	F0, H1, 32
	sldi	F1, H2, 32
	or	F0, F1, F0
	clrrdi	F1, H1, 32
	mr	F2, H2
	clrldi	H1, H1, 32

	subfc	T0, F0, T0
	addme	F1, F1
	addme	F2, F2
	addc	T1, F1, T1
	adde	H0, F2, H0
	addze	H1, H1

	std	T0, 0(RP)
	std	T1, 8(RP)
	std	H0, 16(RP)
	std	H1, 24(RP)

	ld	r14, -8(SP)

	blr
EPILOGUE(_nettle_ecc_secp224r1_modp)
