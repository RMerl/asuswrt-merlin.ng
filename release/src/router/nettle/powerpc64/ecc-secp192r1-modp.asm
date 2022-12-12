C powerpc64/ecc-secp192r1-modp.asm

ifelse(`
   Copyright (C) 2021 Amitay Isaacs, IBM Corporation

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

	.file "ecc-secp192r1-modp.asm"

define(`RP', `r4')
define(`XP', `r5')

define(`T0', `r6')
define(`T1', `r7')
define(`T2', `r8')
define(`T3', `r9')
define(`C1', `r10')
define(`C2', `r11')

	C void ecc_secp192r1_modp (const struct ecc_modulo *m, mp_limb_t *rp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_secp192r1_modp)
	ld	T0, 0(XP)
	ld	T1, 8(XP)
	ld	T2, 16(XP)

	li	C1, 0
	li	C2, 0

	ld	T3, 24(XP)
	addc	T0, T3, T0
	adde	T1, T3, T1
	addze	T2, T2
	addze	C1, C1

	ld	T3, 32(XP)
	addc	T1, T3, T1
	adde	T2, T3, T2
	addze	C1, C1

	ld	T3, 40(XP)
	addc	T0, T3, T0
	adde	T1, T3, T1
	adde	T2, T3, T2
	addze	C1, C1

	addc	T0, C1, T0
	adde	T1, C1, T1
	addze	T2, T2
	addze	C2, C2

	addc	T0, C2, T0
	adde	T1, C2, T1
	addze	T2, T2

	std	T0, 0(RP)
	std	T1, 8(RP)
	std	T2, 16(RP)

	blr
EPILOGUE(_nettle_ecc_secp192r1_modp)
