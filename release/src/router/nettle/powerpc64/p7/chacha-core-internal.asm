C powerpc64/p7/chacha-core-internal.asm

ifelse(`
   Copyright (C) 2020 Niels Möller and Torbjörn Granlund
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

C Argments
define(`DST', `r3')
define(`SRC', `r4')
define(`ROUNDS', `r5')

C Working state
define(`X0', `v0')
define(`X1', `v1')
define(`X2', `v2')
define(`X3', `v3')

define(`ROT16', `v4')
define(`ROT12', `v5')
define(`ROT8',  `v6')
define(`ROT7',  `v7')

C Original input state
define(`S0', `v8')
define(`S1', `v9')
define(`S2', `v10')
define(`S3', `v11')

C Big-endian working state
define(`LE_MASK', `v12')
define(`LE_TEMP', `v13')

C QROUND(X0, X1, X2, X3)
define(`QROUND', `
	C x0 += x1, x3 ^= x0, x3 lrot 16
	C x2 += x3, x1 ^= x2, x1 lrot 12
	C x0 += x1, x3 ^= x0, x3 lrot 8
	C x2 += x3, x1 ^= x2, x1 lrot 7

	vadduwm $1, $1, $2
	vxor	$4, $4, $1
	vrlw	$4, $4, ROT16

	vadduwm $3, $3, $4
	vxor	$2, $2, $3
	vrlw	$2, $2, ROT12

	vadduwm $1, $1, $2
	vxor	$4, $4, $1
	vrlw	$4, $4, ROT8

	vadduwm $3, $3, $4
	vxor	$2, $2, $3
	vrlw	$2, $2, ROT7
')

C LE_SWAP32(X0, X1, X2, X3)
define(`LE_SWAP32', `IF_BE(`
	vperm	X0, X0, X0, LE_MASK
	vperm	X1, X1, X1, LE_MASK
	vperm	X2, X2, X2, LE_MASK
	vperm	X3, X3, X3, LE_MASK
')')

	.text
	C _chacha_core(uint32_t *dst, const uint32_t *src, unsigned rounds)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_chacha_core)

	li	r6, 0x10	C set up some...
	li	r7, 0x20	C ...useful...
	li	r8, 0x30	C ...offsets

	vspltisw ROT16, -16	C -16 instead of 16 actually works!
	vspltisw ROT12, 12
	vspltisw ROT8, 8
	vspltisw ROT7, 7
IF_BE(`
	li	 r9, 0
	lvsl	 LE_MASK, r9, r9		C 00 01 02 03 ... 0c 0d 0e 0f
	vspltisb LE_TEMP, 0x03			C 03 03 03 03 ... 03 03 03 03
	vxor	 LE_MASK, LE_MASK, LE_TEMP	C 03 02 01 00 ... 0f 0e 0d 0c
')

	lxvw4x	VSR(X0), 0, SRC
	lxvw4x	VSR(X1), r6, SRC
	lxvw4x	VSR(X2), r7, SRC
	lxvw4x	VSR(X3), r8, SRC

	vor	S0, X0, X0
	vor	S1, X1, X1
	vor	S2, X2, X2
	vor	S3, X3, X3

	srdi	ROUNDS, ROUNDS, 1
	mtctr	ROUNDS

.Loop:
	QROUND(X0, X1, X2, X3)
	C Rotate rows, to get
	C	 0  1  2  3
	C	 5  6  7  4  <<< 1
	C	10 11  8  9  <<< 2
	C	15 12 13 14  <<< 3

	vsldoi	X1, X1, X1, 4
	vsldoi	X2, X2, X2, 8
	vsldoi	X3, X3, X3, 12

	QROUND(X0, X1, X2, X3)

	C Inverse rotation
	vsldoi	X1, X1, X1, 12
	vsldoi	X2, X2, X2, 8
	vsldoi	X3, X3, X3, 4

	bdnz	.Loop

	vadduwm	X0, X0, S0
	vadduwm	X1, X1, S1
	vadduwm	X2, X2, S2
	vadduwm	X3, X3, S3

	LE_SWAP32(X0, X1, X2, X3)

	stxvw4x	VSR(X0), 0, DST
	stxvw4x	VSR(X1), r6, DST
	stxvw4x	VSR(X2), r7, DST
	stxvw4x	VSR(X3), r8, DST

	blr
EPILOGUE(_nettle_chacha_core)
