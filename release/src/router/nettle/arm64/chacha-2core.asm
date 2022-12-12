C arm64/chacha-2core.asm

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

define(`ROT24', `v0')

define(`T0', `v16')

C State, even elements in X, odd elements in Y
define(`X0', `v17')
define(`X1', `v18')
define(`X2', `v19')
define(`X3', `v20')
define(`Y0', `v21')
define(`Y1', `v22')
define(`Y2', `v23')
define(`Y3', `v24')

C Original input state
define(`S0', `v25')
define(`S1', `v26')
define(`S2', `v27')
define(`S3', `v28')
define(`S3p1', `v29')

define(`TMP0', `v30')
define(`TMP1', `v31')

	C _chacha_2core(uint32_t *dst, const uint32_t *src, unsigned rounds)
PROLOGUE(_nettle_chacha_2core)

	eor		X1.16b, X1.16b, X1.16b
	mov		w3, #1
	mov		X1.s[0], w3

	add		x3, SRC, #48
	ld1		{X3.4s}, [x3]

	add     Y3.4s, X3.4s, X1.4s
	cmhi	Y3.4s, X3.4s, Y3.4s
	ext		Y3.16b, Y3.16b, Y3.16b, #12
	orr		Y3.16b, Y3.16b, X1.16b

.Lshared_entry:
	adr		x3, .Lrot24
	ld1		{ROT24.4s},[x3]

	add     Y3.4s, Y3.4s, X3.4s

C Load state
	ld1		{X0.4s,X1.4s,X2.4s}, [SRC]

	mov		S0.16b, X0.16b
	mov		S1.16b, X1.16b
	mov		S2.16b, X2.16b
	mov		S3.16b, X3.16b
	mov		S3p1.16b, Y3.16b

	trn2	Y0.4s, X0.4s, X0.4s	C  1  1  3  3
	trn1	X0.4s, X0.4s, X0.4s	C  0  0  2  2
	trn2	Y1.4s, X1.4s, X1.4s	C  5  5  7  7
	trn1	X1.4s, X1.4s, X1.4s	C  4  4  6  6
	trn2	Y2.4s, X2.4s, X2.4s	C  9  9 11 11
	trn1	X2.4s, X2.4s, X2.4s	C  8  8 10 10
	trn2	Y3.4s, X3.4s, S3p1.4s	C  13 13 15 15
	trn1	X3.4s, X3.4s, S3p1.4s	C  12 12 14 14

.Loop:
C Register layout (A is first block, B is second block)
C
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C X1:  A4  B4  A6  B6  Y1:  A5  B5  A7  B7
C X2:  A8  B8 A10 B10  Y2:  A9  B9 A11 B11
C X3: A12 B12 A14 B14  Y3: A13 B13 A15 B15
	add		X0.4s, X0.4s, X1.4s
	 add	Y0.4s, Y0.4s, Y1.4s
	eor		X3.16b, X3.16b, X0.16b
	 eor	Y3.16b, Y3.16b, Y0.16b
	rev32	X3.8h, X3.8h
	 rev32	Y3.8h, Y3.8h
	
	add		X2.4s, X2.4s, X3.4s
	 add	Y2.4s, Y2.4s, Y3.4s
	eor		TMP0.16b, X1.16b, X2.16b
	 eor	TMP1.16b, Y1.16b, Y2.16b
	ushr	X1.4s, TMP0.4s, #20
	 ushr	Y1.4s, TMP1.4s, #20
	sli		X1.4s, TMP0.4s, #12
	 sli	Y1.4s, TMP1.4s, #12
	
	add		X0.4s, X0.4s, X1.4s
	 add	Y0.4s, Y0.4s, Y1.4s
	eor		X3.16b, X3.16b, X0.16b
	 eor	Y3.16b, Y3.16b, Y0.16b
	tbl		X3.16b, {X3.16b}, ROT24.16b
	 tbl	Y3.16b, {Y3.16b}, ROT24.16b
	
	add		X2.4s, X2.4s, X3.4s
	 add	Y2.4s, Y2.4s, Y3.4s
	eor		TMP0.16b, X1.16b, X2.16b
	 eor	TMP1.16b, Y1.16b, Y2.16b
	ushr	X1.4s, TMP0.4s, #25
	 ushr	Y1.4s, TMP1.4s, #25
	sli		X1.4s, TMP0.4s, #7
	 sli	Y1.4s, TMP1.4s, #7

	ext		X1.16b, X1.16b, X1.16b, #8
	ext		X2.16b, X2.16b, X2.16b, #8
	ext		Y2.16b, Y2.16b, Y2.16b, #8
	ext		Y3.16b, Y3.16b, Y3.16b, #8

C Register layout:
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C Y1:  A5  B5  A7  B7  X1:  A6  B6  A4  B4 (X1 swapped)
C X2: A10 B10  A8  B8  Y2: A11 A11  A9  B9 (X2, Y2 swapped)
C Y3  A15 B15 A13 B13  X3  A12 B12 A14 B14 (Y3 swapped)

	add		X0.4s, X0.4s, Y1.4s
	 add	Y0.4s, Y0.4s, X1.4s
	eor		Y3.16b, Y3.16b, X0.16b
	 eor	X3.16b, X3.16b, Y0.16b
	rev32	Y3.8h, Y3.8h
	 rev32	X3.8h, X3.8h
	
	add		X2.4s, X2.4s, Y3.4s
	 add	Y2.4s, Y2.4s, X3.4s
	eor		TMP0.16b, Y1.16b, X2.16b
	 eor	TMP1.16b, X1.16b, Y2.16b
	ushr	Y1.4s, TMP0.4s, #20
	 ushr	X1.4s, TMP1.4s, #20
	sli		Y1.4s, TMP0.4s, #12
	 sli	X1.4s, TMP1.4s, #12
	
	add		X0.4s, X0.4s, Y1.4s
	 add	Y0.4s, Y0.4s, X1.4s
	eor		Y3.16b, Y3.16b, X0.16b
	 eor	X3.16b, X3.16b, Y0.16b
	tbl		Y3.16b, {Y3.16b}, ROT24.16b
	 tbl	X3.16b, {X3.16b}, ROT24.16b
	
	add		X2.4s, X2.4s, Y3.4s
	 add	Y2.4s, Y2.4s, X3.4s
	eor		TMP0.16b, Y1.16b, X2.16b
	 eor	TMP1.16b, X1.16b, Y2.16b
	ushr	Y1.4s, TMP0.4s, #25
	 ushr	X1.4s, TMP1.4s, #25
	sli		Y1.4s, TMP0.4s, #7
	 sli	X1.4s, TMP1.4s, #7

	ext		X1.16b, X1.16b, X1.16b, #8
	ext		X2.16b, X2.16b, X2.16b, #8
	ext		Y2.16b, Y2.16b, Y2.16b, #8
	ext		Y3.16b, Y3.16b, Y3.16b, #8

	subs	ROUNDS, ROUNDS, #2
	b.ne	.Loop

	trn1	T0.4s, X0.4s, Y0.4s
	trn2	Y0.4s, X0.4s, Y0.4s

	trn1	X0.4s, X1.4s, Y1.4s
	trn2	Y1.4s, X1.4s, Y1.4s

	trn1	X1.4s, X2.4s, Y2.4s
	trn2	Y2.4s, X2.4s, Y2.4s

	trn1	X2.4s, X3.4s, Y3.4s
	trn2	Y3.4s, X3.4s, Y3.4s

	add		T0.4s, T0.4s, S0.4s
	add		Y0.4s, Y0.4s, S0.4s
	add		X0.4s, X0.4s, S1.4s
	add		Y1.4s, Y1.4s, S1.4s
	add		X1.4s, X1.4s, S2.4s
	add		Y2.4s, Y2.4s, S2.4s
	add		X2.4s, X2.4s, S3.4s
	add		Y3.4s, Y3.4s, S3p1.4s

	st1		{T0.16b,X0.16b,X1.16b,X2.16b}, [DST], #64
	st1		{Y0.16b,Y1.16b,Y2.16b,Y3.16b}, [DST]
	ret
EPILOGUE(_nettle_chacha_2core)

PROLOGUE(_nettle_chacha_2core32)
	eor		Y3.16b, Y3.16b, Y3.16b	C {0,0,...,0}
	mov		w3, #1
	mov		Y3.s[0], w3	C {1,0,...,0}
	add		x3, SRC, #48
	ld1		{X3.4s}, [x3]
	b		.Lshared_entry
EPILOGUE(_nettle_chacha_2core32)

.align	4
.Lrot24: .long	0x02010003,0x06050407,0x0a09080b,0x0e0d0c0f
