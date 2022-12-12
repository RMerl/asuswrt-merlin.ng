C powerpc64/ecc-secp256r1-redc.asm

ifelse(`
   Copyright (C) 2021 Amitay Isaacs & Martin Schwenke, IBM Corporation

   Based on x86_64/ecc-secp256r1-redc.asm

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

C Register usage:

define(`RP', `r4')
define(`XP', `r5')

define(`F0', `r3')
define(`F1', `r6')
define(`F2', `r7')
define(`T', `r8')

define(`U0', `r9')
define(`U1', `r10')
define(`U2', `r11')
define(`U3', `r12')

	.file "ecc-secp256r1-redc.asm"

C FOLD(x), sets (x,F2,F1,F0)  <-- [(x << 192) - (x << 160) + (x << 128) + (x <<32)]
define(`FOLD', `
	sldi	F0, $1, 32
	srdi	F1, $1, 32
	subfc	F2, F0, $1
	subfe	$1, F1, $1
')

C FOLDC(x), sets (x,F2,F1,F0)  <-- [((x+c) << 192) - (x << 160) + (x << 128) + (x <<32)]
define(`FOLDC', `
	sldi	F0, $1, 32
	srdi	F1, $1, 32
	addze	T, $1
	subfc	F2, F0, $1
	subfe	$1, F1, T
')

	C void ecc_secp256r1_redc (const struct ecc_modulo *p, mp_limb_t *rp, mp_limb_t *xp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_secp256r1_redc)

	ld	U0, 0(XP)
	ld	U1, 8(XP)
	ld	U2, 16(XP)
	ld	U3, 24(XP)

	FOLD(U0)
	ld	T, 32(XP)
	addc	U1, F0, U1
	adde	U2, F1, U2
	adde	U3, F2, U3
	adde	U0, U0, T

	FOLDC(U1)
	ld	T, 40(XP)
	addc	U2, F0, U2
	adde	U3, F1, U3
	adde	U0, F2, U0
	adde	U1, U1, T

	FOLDC(U2)
	ld	T, 48(XP)
	addc	U3, F0, U3
	adde	U0, F1, U0
	adde	U1, F2, U1
	adde	U2, U2, T

	FOLDC(U3)
	ld	T, 56(XP)
	addc	U0, F0, U0
	adde	U1, F1, U1
	adde	U2, F2, U2
	adde	U3, U3, T

	C If carry, we need to add in
	C 2^256 - p = <0xfffffffe, 0xff..ff, 0xffffffff00000000, 1>
	li	F0, 0
	addze	F0, F0
	neg	F2, F0
	sldi	F1, F2, 32
	srdi	T, F2, 32
	li	XP, -2
	and	T, T, XP

	addc	U0, F0, U0
	adde	U1, F1, U1
	adde	U2, F2, U2
	adde	U3, T, U3

	std	U0, 0(RP)
	std	U1, 8(RP)
	std	U2, 16(RP)
	std	U3, 24(RP)

	blr
EPILOGUE(_nettle_ecc_secp256r1_redc)
