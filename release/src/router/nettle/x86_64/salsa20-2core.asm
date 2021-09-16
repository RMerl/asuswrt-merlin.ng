C x86_64/salsa20-2core.asm

ifelse(`
   Copyright (C) 2012, 2020 Niels Möller

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

define(`DST', `%rdi')
define(`SRC', `%rsi')
define(`COUNT', `%rdx')

C State, even elements in X, odd elements in Y
define(`X0', `%xmm0')
define(`X1', `%xmm1')
define(`X2', `%xmm2')
define(`X3', `%xmm3')
define(`Y0', `%xmm4')
define(`Y1', `%xmm5')
define(`Y2', `%xmm6')
define(`Y3', `%xmm7')

define(`T0', `%xmm8')
define(`T1', `%xmm9')
define(`T2', `%xmm10')
define(`T3', `%xmm11')

define(`M0011', `%xmm12')

include_src(`x86_64/salsa20.m4')

	.text
	ALIGN(16)
	C _salsa20_2core(uint32_t *dst, const uint32_t *src, unsigned rounds)
PROLOGUE(_nettle_salsa20_2core)
	W64_ENTRY(3, 13)

	movups	(SRC), T0	C [0, 1, 2, 3]
	movups	16(SRC), T1	C [4, 5, 6, 7]
	movups	32(SRC), T2	C [8, 9, 10, 11]
	movups	48(SRC), T3	C [12, 13, 14, 15]

	pshufd	$0xa0, T0, X0	C X0: [0,0,2,2]
	pshufd	$0xf5, T0, Y3	C Y3: [1,1,3,3]
	pshufd	$0xa0, T1, X1	C X1: [4,4,6,6]
	pshufd	$0xf5, T1, Y0	C Y0: [5,5,7,7]
	pshufd	$0xa0, T2, X2	C X2: [8,8,10,10]
	pshufd	$0xf5, T2, Y1	C Y1: [9,9,11,11]
	pshufd	$0xa0, T3, X3	C [12,12,14,14]
	pshufd	$0xf5, T3, Y2	C [13,13,15,15]

	C Complicated counter increment. Could be done with
	C mov $1, %eax; movd %eax, TMP;  paddq T2, TMP
	C earlier, but then it gets more complicated to construct X2 and Y1.

	mov	$1, %eax
	movd	%eax, T0	C [1,0,0,0]
	pshufd	$0x51, T0, T0	C [0,1,0,0]
	pxor	T1, T1
	paddd	T0, X2
	pcmpeqd	X2, T1
	pand	T0, T1
	paddd	T1, Y1

	C Load mask registers
	mov	$-1, %eax
	movd	%eax, M0011
	pshufd	$0x09, M0011, M0011	C 01 01 00 00

	C Swap, to get
	C X0:  0 10  Y0:  5 15
	C X1:  4 14  Y1:  9  3
	C X2:  8  2  Y2: 13  7
	C X3: 12  6  Y3:  1 11
	SWAP(X0, X2, M0011)
	SWAP(X1, X3, M0011)
	SWAP(Y0, Y2, M0011)
	SWAP(Y1, Y3, M0011)

	shrl	$1, XREG(COUNT)

	ALIGN(16)

.Loop:
C Register layout (A is first block, B is second block)
C
C X0: A0  B0  A10 B10  Y0: A5  A5  A15 B15
C X1: A4  B4  A14 B14  Y1: A9  B9  A3  B3
C X2: A8  B8  A2  B2   Y2: A13 B13 A7  B7
C X3: A12 B12 A6  B6   Y3: A1  B1  A11 B11

	movaps	X0, T0
	paddd	X3, T0
	movaps	T0, T1
	 movaps	Y0, T2
	pslld	$7, T0
	 paddd	Y3, T2
	psrld	$25, T1
	 movaps	T2, T3
	pxor	T0, X1
	 pslld	$7, T2
	pxor	T1, X1
	 psrld	$25, T3

	movaps	X0, T0
	 pxor	T2, Y1
	paddd	X1, T0
	 pxor	T3, Y1
	movaps	T0, T1
	 movaps	Y0, T2
	pslld	$9, T0
	 paddd	Y1, T2
	psrld	$23, T1
	 movaps	T2, T3
	pxor	T0, X2
	 pslld	$9, T2
	pxor	T1, X2
	 psrld	$23, T3

	movaps	X1, T0
	 pxor	T2, Y2
	paddd	X2, T0
	 pxor	T3, Y2
	movaps	T0, T1
	 movaps	Y1, T2
	pslld	$13, T0
	 paddd	Y2, T2
	psrld	$19, T1
	 movaps	T2, T3
	pxor	T0, X3
	 pslld	$13, T2
	pxor	T1, X3
	 psrld	$19, T3

	movaps	X2, T0
	 pxor	T2, Y3
	paddd	X3, T0
	 pxor	T3, Y3
	movaps	T0, T1
	 movaps	Y2, T2
	pslld	$18, T0
	 paddd	Y3, T2
	psrld	$14, T1
	 movaps	T2, T3
	pxor	T0, X0
	 pslld	$18, T2
	pxor	T1, X0
	 psrld	$14, T3
	 pxor	T2, Y0
	 pxor	T3, Y0

C Register layout:
C X0: A0  B0  A10 B10  Y0: A5  A5  A15 B15
C Y1: A3  B3   A9  B9  X1: A4  B4  A14 B14 (Y1 swapped)
C X2: A2  B2   A8  B8  Y2: A7  B7  A13 B13 (X2, Y2 swapped)
C Y3: A1  B1  A11 B11  X3: A6  B6  A12 B12 (X3 swapped)

	pshufd	$0x4e, Y1, Y1	C 10 11 00 01
	pshufd	$0x4e, X2, X2
	pshufd	$0x4e, Y2, Y2
	pshufd	$0x4e, X3, X3

	movaps	X0, T0
	paddd	Y1, T0
	movaps	T0, T1
	 movaps	Y0, T2
	pslld	$7, T0
	 paddd	X1, T2
	psrld	$25, T1
	 movaps	T2, T3
	pxor	T0, Y3
	 pslld	$7, T2
	pxor	T1, Y3
	 psrld	$25, T3

	movaps	Y3, T0
	 pxor	T2, X3
	paddd	X0, T0
	 pxor	T3, X3
	movaps	T0, T1
	 movaps	X3, T2
	pslld	$9, T0
	 paddd	Y0, T2
	psrld	$23, T1
	 movaps	T2, T3
	pxor	T0, X2
	 pslld	$9, T2
	pxor	T1, X2
	 psrld	$23, T3

	movaps	X2, T0
	 pxor	T2, Y2
	paddd	Y3, T0
	 pxor	T3, Y2
	movaps	T0, T1
	 movaps	Y2, T2
	pslld	$13, T0
	 paddd	X3, T2
	psrld	$19, T1
	 movaps	T2, T3
	pxor	T0, Y1
	 pslld	$13, T2
	pxor	T1, Y1
	 psrld	$19, T3

	movaps	Y1, T0
	 pxor	T2, X1
	paddd	X2, T0
	 pxor	T3, X1
	movaps	T0, T1
	 movaps	X1, T2
	pslld	$18, T0
	 paddd	Y2, T2
	psrld	$14, T1
	 movaps	T2, T3
	pxor	T0, X0
	 pslld	$18, T2
	pxor	T1, X0
	 psrld	$14, T3
	 pxor	T2, Y0
	 pxor	T3, Y0

	pshufd	$0x4e, Y1, Y1	C 10 11 00 01
	pshufd	$0x4e, X2, X2
	pshufd	$0x4e, Y2, Y2
	pshufd	$0x4e, X3, X3

	decl	XREG(COUNT)
	jnz	.Loop

	SWAP(X0, X2, M0011)
	SWAP(X1, X3, M0011)
	SWAP(Y0, Y2, M0011)
	SWAP(Y1, Y3, M0011)

	movaps	X0, T0
	punpckldq	Y3, X0	C [A0, A1, B0, B1]
	punpckhdq	Y3, T0	C [A2, A3, B2, B3]
	movaps	X0, Y3
	punpcklqdq	T0, X0	C [A0, A1, A2, A3]
	punpckhqdq	T0, Y3	C [B0, B1, B2, B3]

	movups	(SRC), T0
	paddd	T0, X0
	paddd	T0, Y3

	movaps	X1, T1
	punpckldq	Y0, X1	C [A4, A5, B4, B5]
	punpckhdq	Y0, T1	C [A6, A7, B6, B7]
	movaps	X1, Y0
	punpcklqdq	T1, X1	C [A4, A5, A6, A7]
	punpckhqdq	T1, Y0	C [B4, B5, B6, B7]

	movups	16(SRC), T1
	paddd	T1, X1
	paddd	T1, Y0

	movaps	X2, T2
	punpckldq	Y1, X2	C [A8, A9, B8, B9]
	punpckhdq	Y1, T2	C [A10, A11, B10, B11]
	movaps	X2, Y1
	punpcklqdq	T2, X2	C [A8, A9, A10, A11]
	punpckhqdq	T2, Y1	C [B8, B9, B10, B11]

	movups	32(SRC), T2
	paddd	T2, X2
	mov	$1, %eax
	movd	%eax, M0011
	paddq	M0011, T2
	paddd	T2, Y1

	movaps	X3, T3
	punpckldq	Y2, X3	C [A12, A13, B12, B13]
	punpckhdq	Y2, T3	C [A14, A15, B14, B15]
	movaps	X3, Y2
	punpcklqdq	T3, X3	C [A12, A13, A14, A15]
	punpckhqdq	T3, Y2	C [B12, B13, B14, B15]

	movups	48(SRC), T3
	paddd	T3, X3
	paddd	T3, Y2

	movups	X0,(DST)
	movups	X1,16(DST)
	movups	X2,32(DST)
	movups	X3,48(DST)
	movups	Y3,64(DST)
	movups	Y0,80(DST)
	movups	Y1,96(DST)
	movups	Y2,112(DST)

	W64_EXIT(3, 13)
	ret
EPILOGUE(_nettle_salsa20_2core)
