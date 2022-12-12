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

C State, even elements in X, odd elements in Y
define(`X0', `%v1')
define(`X1', `%v2')
define(`X2', `%v3')
define(`X3', `%v29')
define(`Y0', `%v4')
define(`Y1', `%v5')
define(`Y2', `%v6')
define(`Y3', `%v7')

C Original input state
define(`S0', `%v24')
define(`S1', `%v25')
define(`S2', `%v26')
define(`S3', `%v27')
define(`S3p1', `%v28')

define(`T0', `%v0')

define(`BRW', `%v30')
define(`EW', `%v30')
define(`OW', `%v31')

.file "chacha-2core.asm"
.machine "z13"

.text
C _chacha_2core(uint32_t *dst, const uint32_t *src, unsigned rounds)

PROLOGUE(_nettle_chacha_2core)

	vzero	X1
	vleif	X1, 1, 0

	vl		X3, 48(SRC)

	vaccf	Y3, X3, X1	C Counter carry out
	vsldb	Y3, Y3, Y3, 12
	vo		Y3, Y3, X1

.Lshared_entry:
	vaf		Y3, Y3, X3

	vlm		X0, X2, 0(SRC)

	vlr		S0, X0
	vlr		S1, X1
	vlr		S2, X2
	vlr		S3, X3
	vlr		S3p1, Y3

	larl	%r5,.Lword_even
	vlm		EW, OW, 0(%r5)

	vperm	Y0, X0, X0, OW	C  1  1  3  3
	vperm	X0, X0, X0, EW	C  0  0  2  2
	vperm	Y1, X1, X1, OW	C  5  5  7  7
	vperm	X1, X1, X1, EW	C  4  4  6  6
	vperm	Y2, X2, X2, OW	C  9  9 11 11
	vperm	X2, X2, X2, EW	C  8  8 10 10
	vperm	Y3, X3, S3p1, OW	C 13 13 15 15
	vperm	X3, X3, S3p1, EW	C 12 12 14 14

	srlg	ROUNDS, ROUNDS, 1
.Loop:
C Register layout (A is first block, B is second block)
C
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C X1:  A4  B4  A6  B6  Y1:  A5  B5  A7  B7
C X2:  A8  B8 A10 B10  Y2:  A9  B9 A11 B11
C X3: A12 B12 A14 B14  Y3: A13 B13 A15 B15
	vaf		X0, X0, X1
	 vaf	Y0, Y0, Y1
	vx		X3, X3, X0
	 vx		Y3, Y3, Y0
	verllf	X3, X3, 16
	 verllf	Y3, Y3, 16

	vaf		X2, X2, X3
	 vaf	Y2, Y2, Y3
	vx		X1, X1, X2
	 vx		Y1, Y1, Y2
	verllf	X1, X1, 12
	 verllf	Y1, Y1, 12

	vaf		X0, X0, X1
	 vaf	Y0, Y0, Y1
	vx		X3, X3, X0
	 vx		Y3, Y3, Y0
	verllf	X3, X3, 8
	 verllf	Y3, Y3, 8

	vaf		X2, X2, X3
	 vaf	Y2, Y2, Y3
	vx		X1, X1, X2
	 vx		Y1, Y1, Y2
	verllf	X1, X1, 7
	 verllf	Y1, Y1, 7

	vpdi	X1, X1, X1, 0b0100
	vpdi	X2, X2, X2, 0b0100
	vpdi	Y2, Y2, Y2, 0b0100
	vpdi	Y3, Y3, Y3, 0b0100

C Register layout:
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C Y1:  A5  B5  A7  B7  X1:  A6  B6  A4  B4 (X1 swapped)
C X2: A10 B10  A8  B8  Y2: A11 A11  A9  B9 (X2, Y2 swapped)
C Y3  A15 B15 A13 B13  X3  A12 B12 A14 B14 (Y3 swapped)

	vaf		X0, X0, Y1
	 vaf	Y0, Y0, X1
	vx		Y3, Y3, X0
	 vx		X3, X3, Y0
	verllf	Y3, Y3, 16
	 verllf	X3, X3, 16

	vaf		X2, X2, Y3
	 vaf	Y2, Y2, X3
	vx		Y1, Y1, X2
	 vx		X1, X1, Y2
	verllf	Y1, Y1, 12
	 verllf	X1, X1, 12

	vaf		X0, X0, Y1
	 vaf	Y0, Y0, X1
	vx		Y3, Y3, X0
	 vx		X3, X3, Y0
	verllf	Y3, Y3, 8
	 verllf	X3, X3, 8

	vaf		X2, X2, Y3
	 vaf	Y2, Y2, X3
	vx		Y1, Y1, X2
	 vx		X1, X1, Y2
	verllf	Y1, Y1, 7
	 verllf	X1, X1, 7

	vpdi	X1, X1, X1, 0b0100
	vpdi	X2, X2, X2, 0b0100
	vpdi	Y2, Y2, Y2, 0b0100
	vpdi	Y3, Y3, Y3, 0b0100

	brctg	ROUNDS, .Loop

	vperm	T0, X0, Y0, EW
	vperm	Y0, X0, Y0, OW

	vperm	X0, X1, Y1, EW
	vperm	Y1, X1, Y1, OW

	vperm	X1, X2, Y2, EW
	vperm	Y2, X2, Y2, OW

	vperm	X2, X3, Y3, EW
	vperm	Y3, X3, Y3, OW

	vaf		T0, T0, S0
	vaf		Y0, Y0, S0
	vaf		X0, X0, S1
	vaf		Y1, Y1, S1
	vaf		X1, X1, S2
	vaf		Y2, Y2, S2
	vaf		X2, X2, S3
	vaf		Y3, Y3, S3p1

	vl		BRW, 32(%r5)
	vperm	T0, T0, T0, BRW
	vperm	X0, X0, X0, BRW
	vperm	X1, X1, X1, BRW
	vperm	X2, X2, X2, BRW
	vperm	Y0, Y0, Y0, BRW
	vperm	Y1, Y1, Y1, BRW
	vperm	Y2, Y2, Y2, BRW
	vperm	Y3, Y3, Y3, BRW

	vstm	T0, Y3, 0(DST)
	br		RA
EPILOGUE(_nettle_chacha_2core)

PROLOGUE(_nettle_chacha_2core32)
	vzero	Y3
	vleif	Y3, 1, 0
	vl		X3, 48(SRC)
	j		.Lshared_entry
EPILOGUE(_nettle_chacha_2core32)

.align	16
.Lword_even: .long	0x00010203,0x10111213,0x08090A0B,0x18191A1B
.Lword_odd: .long	0x04050607,0x14151617,0x0C0D0E0F,0x1C1D1E1F
.Lword_byte_reverse: .long	0x03020100,0x07060504,0x0B0A0908,0x0F0E0D0C
