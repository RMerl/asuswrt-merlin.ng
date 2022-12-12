C arm64/chacha-core-internal.asm

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
define(`DST', `x0')
define(`SRC', `x1')
define(`ROUNDS', `x2')

C Working state
define(`X0', `v0')
define(`X1', `v1')
define(`X2', `v2')
define(`X3', `v3')

C Original input state
define(`S0', `v4')
define(`S1', `v5')
define(`S2', `v6')
define(`S3', `v7')

define(`ROT24', `v16')

define(`TMP', `v17')

C QROUND(X0, X1, X2, X3)
define(`QROUND', `
	C x0 += x1, x3 ^= x0, x3 lrot 16
	C x2 += x3, x1 ^= x2, x1 lrot 12
	C x0 += x1, x3 ^= x0, x3 lrot 8
	C x2 += x3, x1 ^= x2, x1 lrot 7

	add		$1.4s, $1.4s, $2.4s
	eor		$4.16b, $4.16b, $1.16b
	rev32	$4.8h, $4.8h

	add		$3.4s, $3.4s, $4.4s
	eor		TMP.16b, $2.16b, $3.16b
	ushr	$2.4s, TMP.4s, #20
	sli		$2.4s, TMP.4s, #12

	add		$1.4s, $1.4s, $2.4s
	eor		$4.16b, $4.16b, $1.16b
	tbl		$4.16b, {$4.16b}, ROT24.16b

	add		$3.4s, $3.4s, $4.4s
	eor		TMP.16b, $2.16b, $3.16b
	ushr	$2.4s, TMP.4s, #25
	sli		$2.4s, TMP.4s, #7
')

	.text
	C _chacha_core(uint32_t *dst, const uint32_t *src, unsigned rounds)
PROLOGUE(_nettle_chacha_core)
	adr		x3, .Lrot24
	ld1		{ROT24.4s},[x3]

	ld1		{X0.4s,X1.4s,X2.4s,X3.4s}, [SRC]

	mov		S0.16b, X0.16b
	mov		S1.16b, X1.16b
	mov		S2.16b, X2.16b
	mov		S3.16b, X3.16b

.Loop:
	QROUND(X0, X1, X2, X3)
	C Rotate rows, to get
	C	 0  1  2  3
	C	 5  6  7  4  <<< 1
	C	10 11  8  9  <<< 2
	C	15 12 13 14  <<< 3

	ext		X1.16b, X1.16b, X1.16b, #4
	ext		X2.16b, X2.16b, X2.16b, #8
	ext		X3.16b, X3.16b, X3.16b, #12

	QROUND(X0, X1, X2, X3)

	ext		X1.16b, X1.16b, X1.16b, #12
	ext		X2.16b, X2.16b, X2.16b, #8
	ext		X3.16b, X3.16b, X3.16b, #4

	subs	ROUNDS, ROUNDS, #2
	b.ne	.Loop

	add		X0.4s, X0.4s, S0.4s
	add		X1.4s, X1.4s, S1.4s
	add		X2.4s, X2.4s, S2.4s
	add		X3.4s, X3.4s, S3.4s

	st1		{X0.16b,X1.16b,X2.16b,X3.16b}, [DST]
	ret
EPILOGUE(_nettle_chacha_core)

.align	4
.Lrot24: .long	0x02010003,0x06050407,0x0a09080b,0x0e0d0c0f
