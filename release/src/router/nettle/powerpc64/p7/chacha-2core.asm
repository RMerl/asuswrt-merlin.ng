C powerpc64/p7/chacha-2core.asm

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

C State, even elements in X, odd elements in Y
define(`X0', `v0')
define(`X1', `v1')
define(`X2', `v2')
define(`X3', `v3')
define(`Y0', `v4')
define(`Y1', `v5')
define(`Y2', `v6')
define(`Y3', `v7')

define(`ROT16', `v8')
define(`ROT12', `v9')
define(`ROT8',  `v10')
define(`ROT7',  `v11')

C Original input state
define(`S0', `v12')
define(`S1', `v13')
define(`S2', `v14')
define(`S3', `v15')
define(`S3p1', `v16')

define(`T0', `v17')

	.text
	C _chacha_2core(uint32_t *dst, const uint32_t *src, unsigned rounds)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_chacha_2core)

	li	r8, 0x30	C offset for x3
	vspltisw X1, 1		C {1,1,...,1}
	vspltisw X0, 0		C {0,0,...,0}
	vsldoi	X1, X1, X0, 12	C {1,0,...,0}

	lxvw4x	VSR(X3), r8, SRC

	vaddcuw	Y3, X3, X1	C Counter carry out
	vsldoi	Y3, Y3, Y3, 12
	vor	Y3, Y3, X1

.Lshared_entry:
	vadduwm	Y3, Y3, X3

	li	r6, 0x10	C set up some...
	li	r7, 0x20	C ...useful...
	lxvw4x	VSR(X0), 0, SRC
	lxvw4x	VSR(X1), r6, SRC
	lxvw4x	VSR(X2), r7, SRC

	vor	S0, X0, X0
	vor	S1, X1, X1
	vor	S2, X2, X2
	vor	S3, X3, X3
	vor	S3p1, Y3, Y3

	vmrgow	Y0, X0, X0	C  1  1  3  3
	vmrgew	X0, X0, X0	C  0  0  2  2
	vmrgow	Y1, X1, X1	C  5  5  7  7
	vmrgew	X1, X1, X1	C  4  4  6  6
	vmrgow	Y2, X2, X2	C  9  9 11 11
	vmrgew	X2, X2, X2	C  8  8 10 10
	vmrgow	Y3, X3, S3p1	C 13 13 15 15
	vmrgew	X3, X3, S3p1	C 12 12 14 14

	vspltisw ROT16, -16	C -16 instead of 16 actually works!
	vspltisw ROT12, 12
	vspltisw ROT8, 8
	vspltisw ROT7, 7

	srdi	ROUNDS, ROUNDS, 1
	mtctr	ROUNDS
.Loop:
C Register layout (A is first block, B is second block)
C
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C X1:  A4  B4  A6  B6  Y1:  A5  B5  A7  B7
C X2:  A8  B8 A10 B10  Y2:  A9  B9 A11 B11
C X3: A12 B12 A14 B14  Y3: A13 B13 A15 B15
	vadduwm X0, X0, X1
	 vadduwm Y0, Y0, Y1
	vxor	X3, X3, X0
	 vxor	Y3, Y3, Y0
	vrlw	X3, X3, ROT16
	 vrlw	Y3, Y3, ROT16

	vadduwm X2, X2, X3
	 vadduwm Y2, Y2, Y3
	vxor	X1, X1, X2
	 vxor	Y1, Y1, Y2
	vrlw	X1, X1, ROT12
	 vrlw	Y1, Y1, ROT12

	vadduwm X0, X0, X1
	 vadduwm Y0, Y0, Y1
	vxor	X3, X3, X0
	 vxor	Y3, Y3, Y0
	vrlw	X3, X3, ROT8
	 vrlw	Y3, Y3, ROT8

	vadduwm X2, X2, X3
	 vadduwm Y2, Y2, Y3
	vxor	X1, X1, X2
	 vxor	Y1, Y1, Y2
	vrlw	X1, X1, ROT7
	 vrlw	Y1, Y1, ROT7

	vsldoi	X1, X1, X1, 8
	vsldoi	X2, X2, X2, 8
	vsldoi	Y2, Y2, Y2, 8
	vsldoi	Y3, Y3, Y3, 8

C Register layout:
C X0:  A0  B0  A2  B2  Y0:  A1  B1  A3  B3
C Y1:  A5  B5  A7  B7  X1:  A6  B6  A4  B4 (X1 swapped)
C X2: A10 B10  A8  B8  Y2: A11 A11  A9  B9 (X2, Y2 swapped)
C Y3  A15 B15 A13 B13  X3  A12 B12 A14 B14 (Y3 swapped)

	vadduwm X0, X0, Y1
	 vadduwm Y0, Y0, X1
	vxor	Y3, Y3, X0
	 vxor	X3, X3, Y0
	vrlw	Y3, Y3, ROT16
	 vrlw	X3, X3, ROT16

	vadduwm X2, X2, Y3
	 vadduwm Y2, Y2, X3
	vxor	Y1, Y1, X2
	 vxor	X1, X1, Y2
	vrlw	Y1, Y1, ROT12
	 vrlw	X1, X1, ROT12

	vadduwm X0, X0, Y1
	 vadduwm Y0, Y0, X1
	vxor	Y3, Y3, X0
	 vxor	X3, X3, Y0
	vrlw	Y3, Y3, ROT8
	 vrlw	X3, X3, ROT8

	vadduwm X2, X2, Y3
	 vadduwm Y2, Y2, X3
	vxor	Y1, Y1, X2
	 vxor	X1, X1, Y2
	vrlw	Y1, Y1, ROT7
	 vrlw	X1, X1, ROT7

	vsldoi	X1, X1, X1, 8
	vsldoi	X2, X2, X2, 8
	vsldoi	Y2, Y2, Y2, 8
	vsldoi	Y3, Y3, Y3, 8

	bdnz	.Loop

	vmrgew	T0, X0, Y0
	vmrgow	Y0, X0, Y0

	vmrgew	X0, X1, Y1
	vmrgow	Y1, X1, Y1

	vmrgew	X1, X2, Y2
	vmrgow	Y2, X2, Y2

	vmrgew	X2, X3, Y3
	vmrgow	Y3, X3, Y3

	vadduwm T0, T0, S0
	vadduwm Y0, Y0, S0
	vadduwm X0, X0, S1
	vadduwm Y1, Y1, S1
	vadduwm X1, X1, S2
	vadduwm Y2, Y2, S2
	vadduwm X2, X2, S3
	vadduwm Y3, Y3, S3p1

IF_BE(`
	C Output always stored in little-endian byte order.
	C Can reuse S0 and S1 to construct permutation mask.
	li	 r9, 0
	lvsl	 S0, r9, r9	C 00 01 02 03 ... 0c 0d 0e 0f
	vspltisb S1, 0x03	C 03 03 03 03 ... 03 03 03 03
	vxor	 S1, S1, S0	C 03 02 01 00 ... 0f 0e 0d 0c

	vperm	T0, T0, T0, S1
	vperm	X0, X0, X0, S1
	vperm	X1, X1, X1, S1
	vperm	X2, X2, X2, S1
	vperm	Y0, Y0, Y0, S1
	vperm	Y1, Y1, Y1, S1
	vperm	Y2, Y2, Y2, S1
	vperm	Y3, Y3, Y3, S1
')
	stxvw4x	VSR(T0), 0, DST
	stxvw4x	VSR(X0), r6, DST
	stxvw4x	VSR(X1), r7, DST
	stxvw4x	VSR(X2), r8, DST

	addi	DST, DST, 64

	stxvw4x	VSR(Y0), 0, DST
	stxvw4x	VSR(Y1), r6, DST
	stxvw4x	VSR(Y2), r7, DST
	stxvw4x	VSR(Y3), r8, DST
	blr
EPILOGUE(_nettle_chacha_2core)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_chacha_2core32)
	li	r8, 0x30	C offset for x3
	vspltisw Y3, 1		C {1,1,...,1}
	vspltisw X0, 0		C {0,0,...,0}
	vsldoi	Y3, Y3, X0, 12	C {1,0,...,0}
	lxvw4x	VSR(X3), r8, SRC
	b	.Lshared_entry
EPILOGUE(_nettle_chacha_2core32)

divert(-1)
define core2state
p/x $vs32.v4_int32
p/x $vs33.v4_int32
p/x $vs34.v4_int32
p/x $vs35.v4_int32
p/x $vs36.v4_int32
p/x $vs37.v4_int32
p/x $vs38.v4_int32
p/x $vs39.v4_int32
end
