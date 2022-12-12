C s390x/vf/chacha-4core.asm

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

define(`BRW', `%v24')

C During the loop, used to save the original values for last 4 words
C of each block. Also used as temporaries for transpose.
define(`T0', `%v25')
define(`T1', `%v26')
define(`T2', `%v27')
define(`T3', `%v28')

C A workaround for expanding multiple digits of argument references to QR macro which is incompatible with POSIX
C See https://www.gnu.org/software/m4/manual/html_node/Arguments.html
define(`P1',
`ifelse($1, 0, %v0, $1, 1, %v1, $1, 2, %v2, $1, 3, %v3, $1, 4, %v4, $1, 5, %v5, $1, 6, %v6, $1, 7, %v7, $1, 8, %v8, $1, 9, %v9, $1, 10, %v10, $1, 11, %v11, $1, 12, %v12, $1, 13, %v13, $1, 14, %v14, $1, 15, %v15)')
define(`P2',
`ifelse($1, 0, %v0, $1, 1, %v5, $1, 2, %v10, $1, 3, %v15, $1, 4, %v4, $1, 5, %v9, $1, 6, %v14, $1, 7, %v3, $1, 8, %v8, $1, 9, %v13, $1, 10, %v2, $1, 11, %v7, $1, 12, %v12, $1, 13, %v1, $1, 14, %v6, $1, 15, %v11)')

C Main loop for round
define(`QR',`
	vaf		$1(0), $1(0), $1(1)
	vaf		$1(4), $1(4), $1(5)
	vaf		$1(8), $1(8), $1(9)
	vaf		$1(12), $1(12), $1(13)
	vx		$1(3), $1(3), $1(0)
	vx		$1(7), $1(7), $1(4)
	vx		$1(11), $1(11), $1(8)
	vx		$1(15), $1(15), $1(12)
	verllf	$1(3), $1(3), 16
	verllf	$1(7), $1(7), 16
	verllf	$1(11), $1(11), 16
	verllf	$1(15), $1(15), 16

	vaf		$1(2), $1(2), $1(3)
	vaf		$1(6), $1(6), $1(7)
	vaf		$1(10), $1(10), $1(11)
	vaf		$1(14), $1(14), $1(15)
	vx		$1(1), $1(1), $1(2)
	vx		$1(5), $1(5), $1(6)
	vx		$1(9), $1(9), $1(10)
	vx		$1(13), $1(13), $1(14)
	verllf	$1(1), $1(1), 12
	verllf	$1(5), $1(5), 12
	verllf	$1(9), $1(9), 12
	verllf	$1(13), $1(13), 12

	vaf		$1(0), $1(0), $1(1)
	vaf		$1(4), $1(4), $1(5)
	vaf		$1(8), $1(8), $1(9)
	vaf		$1(12), $1(12), $1(13)
	vx		$1(3), $1(3), $1(0)
	vx		$1(7), $1(7), $1(4)
	vx		$1(11), $1(11), $1(8)
	vx		$1(15), $1(15), $1(12)
	verllf	$1(3), $1(3), 8
	verllf	$1(7), $1(7), 8
	verllf	$1(11), $1(11), 8
	verllf	$1(15), $1(15), 8

	vaf		$1(2), $1(2), $1(3)
	vaf		$1(6), $1(6), $1(7)
	vaf		$1(10), $1(10), $1(11)
	vaf		$1(14), $1(14), $1(15)
	vx		$1(1), $1(1), $1(2)
	vx		$1(5), $1(5), $1(6)
	vx		$1(9), $1(9), $1(10)
	vx		$1(13), $1(13), $1(14)
	verllf	$1(1), $1(1), 7
	verllf	$1(5), $1(5), 7
	verllf	$1(9), $1(9), 7
	verllf	$1(13), $1(13), 7
')

define(`TRANSPOSE',`
	vmrhf	T0, $1, $3		C A0 A2 B0 B2
	vmrhf	T1, $2, $4		C A1 A3 B1 B3
	vmrlf	T2, $1, $3		C C0 C2 D0 D2
	vmrlf	T3, $2, $4		C C1 C3 D1 D3

	vmrhf	$1, T0, T1		C A0 A1 A2 A3
	vmrlf	$2, T0, T1		C B0 B1 B2 B3
	vmrhf	$3, T2, T3		C C0 C2 C1 C3
	vmrlf	$4, T2, T3		C D0 D1 D2 D3
')

.file "chacha-4core.asm"
.machine "z13"

.text
C _chacha_4core(uint32_t *dst, const uint32_t *src, unsigned rounds)

PROLOGUE(_nettle_chacha_4core)

	vrepif	T2, 1		C Apply counter carries

.Lshared_entry:

	C Save callee-save registers
    ALLOC_STACK(%r1,64)		C Allocate 64-byte space on stack
    C Save non-volatile floating point registers
    std		%f8,0(%r1)
    std		%f9,8(%r1)
	std		%f10,16(%r1)
    std		%f11,24(%r1)
	std		%f12,32(%r1)
    std		%f13,40(%r1)
	std		%f14,48(%r1)
    std		%f15,56(%r1)

	larl	%r5,.Lword_byte_reverse
	vlm		BRW, T0, 0(%r5)

C Load state and splat
	vlm		%v0, %v3, 0(SRC)

	vrepf	%v4, %v0, 1
	vrepf	%v8, %v0, 2
	vrepf	%v12, %v0, 3
	vrepf	%v0, %v0, 0
	vrepf	%v5, %v1, 1
	vrepf	%v9, %v1, 2
	vrepf	%v13, %v1, 3
	vrepf	%v1, %v1, 0
	vrepf	%v6, %v2, 1
	vrepf	%v10, %v2, 2
	vrepf	%v14, %v2, 3
	vrepf	%v2, %v2, 0
	vrepf	%v7, %v3, 1
	vrepf	%v11, %v3, 2
	vrepf	%v15, %v3, 3
	vrepf	%v3, %v3, 0

	vaccf	T1, %v3, T0		C low adds
	vaf		%v3, %v3, T0	C compute carry-out
	vn		T1, T1, T2		C discard carries for 32-bit counter variant
	vaf		%v7, %v7, T1	C apply carries

	C Save all 4x4 of the last words.
	vlr		T0, %v3
	vlr		T1, %v7
	vlr		T2, %v11
	vlr		T3, %v15

	srlg	ROUNDS, ROUNDS, 1

.Loop:
	QR(`P1')
	QR(`P2')
	brctg	ROUNDS, .Loop

	C Add in saved original words, including counters, before
	C transpose.
	vaf		%v3, %v3, T0
	vaf		%v7, %v7, T1
	vaf		%v11, %v11, T2
	vaf		%v15, %v15, T3

	TRANSPOSE(%v0, %v4, %v8, %v12)
	TRANSPOSE(%v1, %v5, %v9, %v13)
	TRANSPOSE(%v2, %v6, %v10, %v14)
	TRANSPOSE(%v3, %v7, %v11, %v15)

	vlm		T0, T2, 0(SRC)

	vaf		%v0, %v0, T0
	vaf		%v4, %v4, T0
	vaf		%v8, %v8, T0
	vaf		%v12, %v12, T0

	vperm	%v0, %v0, %v0, BRW
	vperm	%v4, %v4, %v4, BRW
	vperm	%v8, %v8, %v8, BRW
	vperm	%v12, %v12, %v12, BRW

	vaf		%v1, %v1, T1
	vaf		%v5, %v5, T1
	vaf		%v9, %v9, T1
	vaf		%v13, %v13, T1

	vperm	%v1, %v1, %v1, BRW
	vperm	%v5, %v5, %v5, BRW
	vperm	%v9, %v9, %v9, BRW
	vperm	%v13, %v13, %v13, BRW

	vaf		%v2, %v2, T2
	vaf		%v6, %v6, T2
	vaf		%v10, %v10, T2
	vaf		%v14, %v14, T2

	vperm	%v2, %v2, %v2, BRW
	vperm	%v6, %v6, %v6, BRW
	vperm	%v10, %v10, %v10, BRW
	vperm	%v14, %v14, %v14, BRW

	vperm	%v3, %v3, %v3, BRW
	vperm	%v7, %v7, %v7, BRW
	vperm	%v11, %v11, %v11, BRW
	vperm	%v15, %v15, %v15, BRW

	vstm	%v0, %v15, 0(DST)

	C Restore callee-save registers
	ld		%f8,0(%r1)
    ld		%f9,8(%r1)
	ld		%f10,16(%r1)
    ld		%f11,24(%r1)
	ld		%f12,32(%r1)
    ld		%f13,40(%r1)
	ld		%f14,48(%r1)
    ld		%f15,56(%r1)
    FREE_STACK(64)		C Deallocate stack space
	br		RA
EPILOGUE(_nettle_chacha_4core)

PROLOGUE(_nettle_chacha_4core32)

	vzero	T2			C Ignore counter carries
	j		.Lshared_entry
EPILOGUE(_nettle_chacha_4core32)

.align	16
.Lword_byte_reverse: .long	0x03020100,0x07060504,0x0B0A0908,0x0F0E0D0C
.Lcnts: .long	0,1,2,3	C increments
