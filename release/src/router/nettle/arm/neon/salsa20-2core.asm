C arm/neon/salsa20-2core.asm

ifelse(`
   Copyright (C) 2020 Niels Möller

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

	.file "salsa20-2core.asm"
	.fpu	neon

define(`DST', `r0')
define(`SRC', `r1')
define(`ROUNDS', `r2')
define(`SRCp32', `r3')

C State, even elements in X, odd elements in Y
define(`X0', `q0')
define(`X1', `q1')
define(`X2', `q2')
define(`X3', `q3')
define(`Y0', `q8')
define(`Y1', `q9')
define(`Y2', `q10')
define(`Y3', `q11')
define(`T0', `q12')
define(`T1', `q13')
define(`T2', `q14')
define(`T3', `q15')

	.text
	.align 4
.Lcount1:
	.int 1,0,0,0

	C _salsa20_2core(uint32_t *dst, const uint32_t *src, unsigned rounds)
PROLOGUE(_nettle_salsa20_2core)
	C loads using vld1.32 to be endianness-neutral wrt consecutive 32-bit words
	add	SRCp32, SRC, #32
	vld1.32	{X0,X1}, [SRC]
	vld1.32	{X2,X3}, [SRCp32]
	adr	r12, .Lcount1

	vmov	Y3, X0
	vld1.32 {Y1}, [r12]
	vmov	Y0, X1
	vadd.i64 Y1, Y1, X2	C Increment counter
	vmov	Y2, X3

	vtrn.32	X0, Y3		C X0:  0  0  2  2  Y3:  1  1  3  3
	vtrn.32	X1, Y0		C X1:  4  4  6  6  Y0:  5  5  7  7
	vtrn.32	X2, Y1		C X2:  8  8 10 10  Y1:  9  9 11 11
	vtrn.32	X3, Y2		C X3: 12 12 14 14  Y2: 13 13 15 15

	C Swap, to get
	C X0:  0 10  Y0:  5 15
	C X1:  4 14  Y1:  9  3
	C X2:  8  2  Y2: 13  7
	C X3: 12  6  Y3:  1 11
	vswp	D1REG(X0), D1REG(X2)
	vswp	D1REG(X1), D1REG(X3)
	vswp	D1REG(Y0), D1REG(Y2)
	vswp	D1REG(Y1), D1REG(Y3)

.Loop:
C Register layout (A is first block, B is second block)
C
C X0: A0  B0  A10 B10  Y0: A5  A5  A15 B15
C X1: A4  B4  A14 B14  Y1: A9  B9  A3  B3
C X2: A8  B8  A2  B2   Y2: A13 B13 A7  B7
C X3: A12 B12 A6  B6   Y3: A1  B1  A11 B11

	vadd.i32	T0, X0, X3
	vshl.i32	T1, T0, #7
	 vadd.i32	T2, Y0, Y3
	vsri.u32	T1, T0, #25
	 vshl.i32	T3, T2, #7
	veor		X1, X1, T1
	 vsri.u32	T3, T2, #25
	vadd.i32	T0, X1, X0
	 veor		Y1, Y1, T3
	vshl.i32	T1, T0, #9
	 vadd.i32	T2, Y1, Y0
	vsri.u32	T1, T0, #23
	 vshl.i32	T3, T2, #9
	veor		X2, X2, T1
	 vsri.u32	T3, T2, #23
	vadd.i32	T0, X2, X1
	 veor		Y2, Y2, T3
	vshl.i32	T1, T0, #13
	 vadd.i32	T2, Y2, Y1
	vsri.u32	T1, T0, #19
	 vshl.i32	T3, T2, #13
	veor		X3, X3, T1
	 vsri.u32	T3, T2, #19
	vadd.i32	T0, X3, X2
	 veor		Y3, Y3, T3
	vshl.i32	T1, T0, #18
	 vadd.i32	T2, Y3, Y2
	  vext.32	Y1, Y1, Y1, #2
	vsri.u32	T1, T0, #14
	 vshl.i32	T3, T2, #18
	  vext.32	Y2, Y2, Y2, #2
	veor		X0, X0, T1
	 vsri.u32	T3, T2, #14
	  vext.32	X3, X3, X3, #2
	 veor		Y0, Y0, T3

C Register layout:
C X0: A0  B0  A10 B10  Y0: A5  A5  A15 B15
C Y1: A3  B3   A9  B9  X1: A4  B4  A14 B14 (Y1 swapped)
C X2: A2  B2   A8  B8  Y2: A7  B7  A13 B13 (X2, Y2 swapped)
C Y3: A1  B1  A11 B11  X3: A6  B6  A12 B12 (X3 swapped)

	vadd.i32	T0, X0, Y1
	  vext.32	X2, X2, X2, #2
	vshl.i32	T1, T0, #7
	 vadd.i32	T2, Y0, X1
	vsri.u32	T1, T0, #25
	 vshl.i32	T3, T2, #7
	veor		Y3, Y3, T1
	 vsri.u32	T3, T2, #25
	vadd.i32	T0, Y3, X0
	 veor		X3, X3, T3
	vshl.i32	T1, T0, #9
	 vadd.i32	T2, X3, Y0
	vsri.u32	T1, T0, #23
	 vshl.i32	T3, T2, #9
	veor		X2, X2, T1
	 vsri.u32	T3, T2, #23
	vadd.i32	T0, X2, Y3
	 veor		Y2, Y2, T3
	vshl.i32	T1, T0, #13
	 vadd.i32	T2, Y2, X3
	vsri.u32	T1, T0, #19
	 vshl.i32	T3, T2, #13
	veor		Y1, Y1, T1
	 vsri.u32	T3, T2, #19
	vadd.i32	T0, Y1, X2
	 veor		X1, X1, T3
	  vext.32	X2, X2, X2, #2
	vshl.i32	T1, T0, #18
	 vadd.i32	T2, X1, Y2
	  vext.32	Y1, Y1, Y1, #2
	vsri.u32	T1, T0, #14
	   subs		ROUNDS, ROUNDS, #2
	 vshl.i32	T3, T2, #18
	  vext.32	X3, X3, X3, #2
	veor		X0, X0, T1
	 vsri.u32	T3, T2, #14
	  vext.32	Y2, Y2, Y2, #2
	 veor		Y0, Y0, T3

	bhi		.Loop

C Inverse swaps and transpositions

	vswp	D1REG(X0), D1REG(X2)
	vswp	D1REG(X1), D1REG(X3)
	vswp	D1REG(Y0), D1REG(Y2)
	vswp	D1REG(Y1), D1REG(Y3)

	vld1.32	{T0,T1}, [SRC]
	vld1.32	{T2,T3}, [SRCp32]

	vtrn.32	X0, Y3
	vtrn.32	X1, Y0
	vtrn.32	X2, Y1
	vtrn.32	X3, Y2

C Add in the original context
	vadd.i32	X0, X0, T0
	vadd.i32	X1, X1, T1

C vst1.8 because caller expects results little-endian
C interleave loads, calculations and stores to save cycles on stores
C use vstm when little-endian for some additional speedup
IF_BE(`	vst1.8	{X0,X1}, [DST]!')

	vadd.i32	X2, X2, T2
	vadd.i32	X3, X3, T3
IF_BE(`	vst1.8	{X2,X3}, [DST]!')
IF_LE(`	vstmia	DST!, {X0,X1,X2,X3}')

	vld1.32 {X0}, [r12]
	vadd.i32	T0, T0, Y3
	vadd.i64	T2, T2, X0
	vadd.i32	T1, T1, Y0
IF_BE(`	vst1.8	{T0,T1}, [DST]!')

	vadd.i32	T2, T2, Y1
	vadd.i32	T3, T3, Y2
IF_BE(`	vst1.8	{T2,T3}, [DST]')
IF_LE(`	vstm	DST, {T0,T1,T2,T3}')
	bx	lr
EPILOGUE(_nettle_salsa20_2core)
