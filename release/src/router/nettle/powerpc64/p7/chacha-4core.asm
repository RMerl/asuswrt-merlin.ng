C powerpc64/chacha-4core.asm

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

define(`SP', `r1')
define(`TOCP', `r2')

C Argments
define(`DST', `r3')
define(`SRC', `r4')
define(`ROUNDS', `r5')

C Working state in v0,...,v15

define(`ROT16', v16)
define(`ROT12', v17)
define(`ROT8',	v18)
define(`ROT7',	v19)

C During the loop, used to save the original values for last 4 words
C of each block. Also used as temporaries for transpose.
define(`T0', `v20')
define(`T1', `v21')
define(`T2', `v22')
define(`T3', `v23')

C A workaround for expanding multiple digits of argument references to QR macro which is incompatible with POSIX
C See https://www.gnu.org/software/m4/manual/html_node/Arguments.html
define(`P1',
`ifelse($1, 0, v0, $1, 1, v4, $1, 2, v8, $1, 3, v12, $1, 4, v1, $1, 5, v5, $1, 6, v9, $1, 7, v13, $1, 8, v2, $1, 9, v6, $1, 10, v10, $1, 11, v14, $1, 12, v3, $1, 13, v7, $1, 14, v11, $1, 15, v15)')
define(`P2',
`ifelse($1, 0, v0, $1, 1, v5, $1, 2, v10, $1, 3, v15, $1, 4, v1, $1, 5, v6, $1, 6, v11, $1, 7, v12, $1, 8, v2, $1, 9, v7, $1, 10, v8, $1, 11, v13, $1, 12, v3, $1, 13, v4, $1, 14, v9, $1, 15, v14)')

C Main loop for round
define(`QR',`
	vadduwm $1(0), $1(0), $1(1)
	vadduwm $1(4), $1(4), $1(5)
	vadduwm $1(8), $1(8), $1(9)
	vadduwm $1(12), $1(12), $1(13)
	vxor	$1(3), $1(3), $1(0)
	vxor	$1(7), $1(7), $1(4)
	vxor	$1(11), $1(11), $1(8)
	vxor	$1(15), $1(15), $1(12)
	vrlw	$1(3), $1(3), ROT16
	vrlw	$1(7), $1(7), ROT16
	vrlw	$1(11), $1(11), ROT16
	vrlw	$1(15), $1(15), ROT16

	vadduwm $1(2), $1(2), $1(3)
	vadduwm $1(6), $1(6), $1(7)
	vadduwm $1(10), $1(10), $1(11)
	vadduwm $1(14), $1(14), $1(15)
	vxor	$1(1), $1(1), $1(2)
	vxor	$1(5), $1(5), $1(6)
	vxor	$1(9), $1(9), $1(10)
	vxor	$1(13), $1(13), $1(14)
	vrlw	$1(1), $1(1), ROT12
	vrlw	$1(5), $1(5), ROT12
	vrlw	$1(9), $1(9), ROT12
	vrlw	$1(13), $1(13), ROT12

	vadduwm $1(0), $1(0), $1(1)
	vadduwm $1(4), $1(4), $1(5)
	vadduwm $1(8), $1(8), $1(9)
	vadduwm $1(12), $1(12), $1(13)
	vxor	$1(3), $1(3), $1(0)
	vxor	$1(7), $1(7), $1(4)
	vxor	$1(11), $1(11), $1(8)
	vxor	$1(15), $1(15), $1(12)
	vrlw	$1(3), $1(3), ROT8
	vrlw	$1(7), $1(7), ROT8
	vrlw	$1(11), $1(11), ROT8
	vrlw	$1(15), $1(15), ROT8

	vadduwm $1(2), $1(2), $1(3)
	vadduwm $1(6), $1(6), $1(7)
	vadduwm $1(10), $1(10), $1(11)
	vadduwm $1(14), $1(14), $1(15)
	vxor	$1(1), $1(1), $1(2)
	vxor	$1(5), $1(5), $1(6)
	vxor	$1(9), $1(9), $1(10)
	vxor	$1(13), $1(13), $1(14)
	vrlw	$1(1), $1(1), ROT7
	vrlw	$1(5), $1(5), ROT7
	vrlw	$1(9), $1(9), ROT7
	vrlw	$1(13), $1(13), ROT7
')

define(`TRANSPOSE',`
	vmrghw	T0, $1, $3	C A0 A2 B0 B2
	vmrghw	T1, $2, $4	C A1 A3 B1 B3
	vmrglw	T2, $1, $3	C C0 C2 D0 D2
	vmrglw	T3, $2, $4	C C1 C3 D1 D3

	vmrghw	$1, T0, T1	C A0 A1 A2 A3
	vmrglw	$2, T0, T1	C B0 B1 B2 B3
	vmrghw	$3, T2, T3	C C0 C2 C1 C3
	vmrglw	$4, T2, T3	C D0 D1 D2 D3
')

	C _chacha_4core(uint32_t *dst, const uint32_t *src, unsigned rounds)
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_chacha_4core)

	vspltisw T2, 1		C Apply counter carries

.Lshared_entry:

	li	r6, 0x10	C set up some...
	li	r7, 0x20	C ...useful...
	li	r8, 0x30	C ...offsets

	C Save callee-save registers. Use the "protected zone", max
	C 228 bytes, below the stack pointer, accessed via r10.
	addi	r10, SP, -0x40
	stvx	v20, 0, r10
	stvx	v21, r6, r10
	stvx	v22, r7, r10
	stvx	v23, r8, r10

	vspltisw ROT16, -16	C -16 instead of 16 actually works!
	vspltisw ROT12, 12
	vspltisw ROT8, 8
	vspltisw ROT7, 7

C Load state and splat
	lxvw4x	VSR(v0),  0, SRC	C "expa ..."
	lxvw4x	VSR(v4),  r6, SRC	C key
	lxvw4x	VSR(v8),  r7, SRC	C key
	lxvw4x	VSR(v12), r8, SRC	C cnt and nonce

	vspltw	v1, v0, 1
	vspltw	v2, v0, 2
	vspltw	v3, v0, 3
	vspltw	v0, v0, 0
	vspltw	v5, v4, 1
	vspltw	v6, v4, 2
	vspltw	v7, v4, 3
	vspltw	v4, v4, 0
	vspltw	v9,  v8, 1
	vspltw	v10, v8, 2
	vspltw	v11, v8, 3
	vspltw	v8,  v8, 0
	vspltw	v13, v12, 1
	vspltw	v14, v12, 2
	vspltw	v15, v12, 3
	vspltw	v12, v12, 0

	addis	r9, r2, .Lcnts@got@ha
	ld	r9, .Lcnts@got@l(r9)
	lxvw4x	VSR(T0), 0, r9	C increments
	vaddcuw	T1, v12, T0	C compute carry-out
	vadduwm	v12, v12, T0	C low adds
	vand	T1, T1, T2	C discard carries for 32-bit counter variant
	vadduwm	v13, v13, T1	C apply carries

	C Save all 4x4 of the last words.
	vor	T0, v12, v12
	vor	T1, v13, v13
	vor	T2, v14, v14
	vor	T3, v15, v15

	srdi	ROUNDS, ROUNDS, 1
	mtctr	ROUNDS
.Loop:
	QR(`P1')
	QR(`P2')
	bdnz	.Loop

	C Add in saved original words, including counters, before
	C transpose.
	vadduwm	v12, v12, T0
	vadduwm	v13, v13, T1
	vadduwm v14, v14, T2
	vadduwm	v15, v15, T3

	TRANSPOSE(v0, v1,v2, v3)
	TRANSPOSE(v4, v5, v6, v7)
	TRANSPOSE(v8, v9, v10, v11)
	TRANSPOSE(v12, v13, v14, v15)

	lxvw4x	VSR(T0),  0, SRC
	lxvw4x	VSR(T1), r6, SRC
	lxvw4x	VSR(T2), r7, SRC

	vadduwm	v0, v0, T0
	vadduwm	v1, v1, T0
	vadduwm	v2, v2, T0
	vadduwm	v3, v3, T0

	vadduwm	v4, v4, T1
	vadduwm	v5, v5, T1
	vadduwm	v6, v6, T1
	vadduwm	v7, v7, T1

	vadduwm	v8, v8, T2
	vadduwm	v9, v9, T2
	vadduwm	v10, v10, T2
	vadduwm	v11, v11, T2

IF_BE(`
	C Output always stored in little-endian byte order.
	C Can reuse T0 and T1 to construct permutation mask.
	li	 r9, 0
	lvsl	 T0, r9, r9	C 00 01 02 03 ... 0c 0d 0e 0f
	vspltisb T1, 0x03	C 03 03 03 03 ... 03 03 03 03
	vxor	 T1, T1, T0	C 03 02 01 00 ... 0f 0e 0d 0c

	forloop(i, 0, 15, `
	vperm   m4_unquote(v`'i), m4_unquote(v`'i), m4_unquote(v`'i), T1
	')
')

	stxvw4x	VSR(v0), 0, DST
	stxvw4x	VSR(v4), r6, DST
	stxvw4x	VSR(v8), r7, DST
	stxvw4x	VSR(v12), r8, DST

	addi	DST, DST, 64

	stxvw4x	VSR(v1), 0, DST
	stxvw4x	VSR(v5), r6, DST
	stxvw4x	VSR(v9), r7, DST
	stxvw4x	VSR(v13), r8, DST

	addi	DST, DST, 64

	stxvw4x	VSR(v2), 0, DST
	stxvw4x	VSR(v6), r6, DST
	stxvw4x	VSR(v10), r7, DST
	stxvw4x	VSR(v14), r8, DST

	addi	DST, DST, 64

	stxvw4x	VSR(v3), 0, DST
	stxvw4x	VSR(v7), r6, DST
	stxvw4x	VSR(v11), r7, DST
	stxvw4x	VSR(v15), r8, DST

	C Restore callee-save registers
	lvx	v20, 0, r10
	lvx	v21, r6, r10
	lvx	v22, r7, r10
	lvx	v23, r8, r10

	blr
EPILOGUE(_nettle_chacha_4core)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_chacha_4core32)
	vspltisw T2, 0		C Ignore counter carries
	b	.Lshared_entry
EPILOGUE(_nettle_chacha_4core32)

	.section .rodata
	ALIGN(16)
.Lcnts: .long	0,1,2,3		C increments
