C s390x/vf/chacha-2core.asm

ifelse(`
   Copyright (C) 2020 Niels Möller and Torbjörn Granlund
   Copyright (C) 2022 Mamone Tarsha
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
define(`DST', `%r2')
define(`SRC', `%r3')
define(`ROUNDS', `%r4')

C Working state
define(`X0', `%v0')
define(`X1', `%v1')
define(`X2', `%v2')
define(`X3', `%v3')

C Original input state
define(`S0', `%v4')
define(`S1', `%v5')
define(`S2', `%v6')
define(`S3', `%v7')

define(`BRW', `%v24')

C QROUND(X0, X1, X2, X3)
define(`QROUND', `
	C x0 += x1, x3 ^= x0, x3 lrot 16
	C x2 += x3, x1 ^= x2, x1 lrot 12
	C x0 += x1, x3 ^= x0, x3 lrot 8
	C x2 += x3, x1 ^= x2, x1 lrot 7

	vaf		$1, $1, $2
	vx		$4, $4, $1
	verllf	$4, $4, 16

	vaf		$3, $3, $4
	vx		$2, $2, $3
	verllf	$2, $2, 12

	vaf		$1, $1, $2
	vx		$4, $4, $1
	verllf	$4, $4, 8

	vaf		$3, $3, $4
	vx		$2, $2, $3
	verllf	$2, $2, 7
')

.file "chacha-core-internal.asm"
.machine "z13"

.text
C _chacha_core(uint32_t *dst, const uint32_t *src, unsigned rounds)

PROLOGUE(_nettle_chacha_core)
	vlm		X0, X3, 0(SRC)

	vlr		S0, X0
	vlr		S1, X1
	vlr		S2, X2
	vlr		S3, X3

	srlg	ROUNDS, ROUNDS, 1
.Loop:
QROUND(X0, X1, X2, X3)
	C Rotate rows, to get
	C	 0  1  2  3
	C	 5  6  7  4  <<< 1
	C	10 11  8  9  <<< 2
	C	15 12 13 14  <<< 3

	vsldb	X1, X1, X1, 4
	vsldb	X2, X2, X2, 8
	vsldb	X3, X3, X3, 12

	QROUND(X0, X1, X2, X3)

	C Inverse rotation
	vsldb	X1, X1, X1, 12
	vsldb	X2, X2, X2, 8
	vsldb	X3, X3, X3, 4

	brctg	ROUNDS, .Loop

	vaf		X0, X0, S0
	vaf		X1, X1, S1
	vaf		X2, X2, S2
	vaf		X3, X3, S3

	larl	%r5,.Lword_byte_reverse
	vl		BRW, 0(%r5)
	vperm	X0, X0, X0, BRW
	vperm	X1, X1, X1, BRW
	vperm	X2, X2, X2, BRW
	vperm	X3, X3, X3, BRW

	vstm	X0, X3, 0(DST)
	br		RA
EPILOGUE(_nettle_chacha_core)

.align	16
.Lword_byte_reverse: .long	0x03020100,0x07060504,0x0B0A0908,0x0F0E0D0C
