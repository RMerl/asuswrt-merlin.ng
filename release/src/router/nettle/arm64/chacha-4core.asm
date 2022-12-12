C arm64/chacha-4core.asm

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

C During the loop, used to save the original values for last 4 words
C of each block. Also used as temporaries for transpose.
define(`T0', `v0')
define(`T1', `v1')
define(`T2', `v2')
define(`T3', `v3')

define(`TMP0', `v4')
define(`TMP1', `v5')
define(`TMP2', `v6')
define(`TMP3', `v7')

define(`ROT24', `v8')

C A workaround for expanding multiple digits of argument references to QR macro which is incompatible with POSIX
C See https://www.gnu.org/software/m4/manual/html_node/Arguments.html
define(`P1',
`ifelse($1, 0, v16, $1, 1, v17, $1, 2, v18, $1, 3, v19, $1, 4, v20, $1, 5, v21, $1, 6, v22, $1, 7, v23, $1, 8, v24, $1, 9, v25, $1, 10, v26, $1, 11, v27, $1, 12, v28, $1, 13, v29, $1, 14, v30, $1, 15, v31)')
define(`P2',
`ifelse($1, 0, v16, $1, 1, v21, $1, 2, v26, $1, 3, v31, $1, 4, v20, $1, 5, v25, $1, 6, v30, $1, 7, v19, $1, 8, v24, $1, 9, v29, $1, 10, v18, $1, 11, v23, $1, 12, v28, $1, 13, v17, $1, 14, v22, $1, 15, v27)')

C Main loop for round
define(`QR',`
	add		$1(0).4s, $1(0).4s, $1(1).4s
	add		$1(4).4s, $1(4).4s, $1(5).4s
	add		$1(8).4s, $1(8).4s, $1(9).4s
	add		$1(12).4s, $1(12).4s, $1(13).4s
	eor		$1(3).16b, $1(3).16b, $1(0).16b
	eor		$1(7).16b, $1(7).16b, $1(4).16b
	eor		$1(11).16b, $1(11).16b, $1(8).16b
	eor		$1(15).16b, $1(15).16b, $1(12).16b
	rev32	$1(3).8h, $1(3).8h
	rev32	$1(7).8h, $1(7).8h
	rev32	$1(11).8h, $1(11).8h
	rev32	$1(15).8h, $1(15).8h

	add		$1(2).4s, $1(2).4s, $1(3).4s
	add		$1(6).4s, $1(6).4s, $1(7).4s
	add		$1(10).4s, $1(10).4s, $1(11).4s
	add		$1(14).4s, $1(14).4s, $1(15).4s
	eor		TMP0.16b, $1(1).16b, $1(2).16b
	eor		TMP1.16b, $1(5).16b, $1(6).16b
	eor		TMP2.16b, $1(9).16b, $1(10).16b
	eor		TMP3.16b, $1(13).16b, $1(14).16b
	ushr	$1(1).4s, TMP0.4s, #20
	ushr	$1(5).4s, TMP1.4s, #20
	ushr	$1(9).4s, TMP2.4s, #20
	ushr	$1(13).4s, TMP3.4s, #20
	sli		$1(1).4s, TMP0.4s, #12
	sli		$1(5).4s, TMP1.4s, #12
	sli		$1(9).4s, TMP2.4s, #12
	sli		$1(13).4s, TMP3.4s, #12

	add		$1(0).4s, $1(0).4s, $1(1).4s
	add		$1(4).4s, $1(4).4s, $1(5).4s
	add		$1(8).4s, $1(8).4s, $1(9).4s
	add		$1(12).4s, $1(12).4s, $1(13).4s
	eor		$1(3).16b, $1(3).16b, $1(0).16b
	eor		$1(7).16b, $1(7).16b, $1(4).16b
	eor		$1(11).16b, $1(11).16b, $1(8).16b
	eor		$1(15).16b, $1(15).16b, $1(12).16b
	tbl		$1(3).16b, {$1(3).16b}, ROT24.16b
	tbl		$1(7).16b, {$1(7).16b}, ROT24.16b
	tbl		$1(11).16b, {$1(11).16b}, ROT24.16b
	tbl		$1(15).16b, {$1(15).16b}, ROT24.16b

	add		$1(2).4s, $1(2).4s, $1(3).4s
	add		$1(6).4s, $1(6).4s, $1(7).4s
	add		$1(10).4s, $1(10).4s, $1(11).4s
	add		$1(14).4s, $1(14).4s, $1(15).4s
	eor		TMP0.16b, $1(1).16b, $1(2).16b
	eor		TMP1.16b, $1(5).16b, $1(6).16b
	eor		TMP2.16b, $1(9).16b, $1(10).16b
	eor		TMP3.16b, $1(13).16b, $1(14).16b
	ushr	$1(1).4s, TMP0.4s, #25
	ushr	$1(5).4s, TMP1.4s, #25
	ushr	$1(9).4s, TMP2.4s, #25
	ushr	$1(13).4s, TMP3.4s, #25
	sli		$1(1).4s, TMP0.4s, #7
	sli		$1(5).4s, TMP1.4s, #7
	sli		$1(9).4s, TMP2.4s, #7
	sli		$1(13).4s, TMP3.4s, #7
')

define(`TRANSPOSE',`
	zip1	T0.4s, $1.4s, $3.4s		C A0 A2 B0 B2
	zip1	T1.4s, $2.4s, $4.4s		C A1 A3 B1 B3
	zip2	T2.4s, $1.4s, $3.4s		C C0 C2 D0 D2
	zip2	T3.4s, $2.4s, $4.4s		C C1 C3 D1 D3

	zip1	$1.4s, T0.4s, T1.4s		C A0 A1 A2 A3
	zip2	$2.4s, T0.4s, T1.4s		C B0 B1 B2 B3
	zip1	$3.4s, T2.4s, T3.4s		C C0 C2 C1 C3
	zip2	$4.4s, T2.4s, T3.4s		C D0 D1 D2 D3
')

	C _chacha_4core(uint32_t *dst, const uint32_t *src, unsigned rounds)
PROLOGUE(_nettle_chacha_4core)

	mov		w3, #1
	dup		TMP2.4s, w3	C Apply counter carries

.Lshared_entry:

	C Save callee-save registers
	fmov	x3, d8

	adr		x4, .Lcnts
	ld1		{TMP3.4s,ROT24.4s},[x4]

C Load state and splat
	ld1		{v16.4s,v17.4s,v18.4s,v19.4s}, [SRC]

	dup		v20.4s, v16.s[1]
	dup		v24.4s, v16.s[2]
	dup		v28.4s, v16.s[3]
	dup		v16.4s, v16.s[0]
	dup		v21.4s, v17.s[1]
	dup		v25.4s, v17.s[2]
	dup		v29.4s, v17.s[3]
	dup		v17.4s, v17.s[0]
	dup		v22.4s, v18.s[1]
	dup		v26.4s, v18.s[2]
	dup		v30.4s, v18.s[3]
	dup		v18.4s, v18.s[0]
	dup		v23.4s, v19.s[1]
	dup		v27.4s, v19.s[2]
	dup		v31.4s, v19.s[3]
	dup		v19.4s, v19.s[0]

	add		v19.4s, v19.4s, TMP3.4s	C low adds
	cmhi	TMP1.4s, TMP3.4s, v19.4s	C compute carry-out
	and		TMP1.16b, TMP1.16b, TMP2.16b	C discard carries for 32-bit counter variant
	add		v23.4s, v23.4s, TMP1.4s	C apply carries

	C Save all 4x4 of the last words.
	mov		T0.16b, v19.16b
	mov		T1.16b, v23.16b
	mov		T2.16b, v27.16b
	mov		T3.16b, v31.16b

.Loop:
	QR(`P1')
	QR(`P2')
	subs	ROUNDS, ROUNDS, #2
	b.ne	.Loop

	C Add in saved original words, including counters, before
	C transpose.
	add		v19.4s, v19.4s, T0.4s
	add		v23.4s, v23.4s, T1.4s
	add		v27.4s, v27.4s, T2.4s
	add		v31.4s, v31.4s, T3.4s

	TRANSPOSE(v16, v20,v24, v28)
	TRANSPOSE(v17, v21, v25, v29)
	TRANSPOSE(v18, v22, v26, v30)
	TRANSPOSE(v19, v23, v27, v31)

	ld1		{T0.4s,T1.4s,T2.4s}, [SRC]

	add		v16.4s, v16.4s, T0.4s
	add		v20.4s, v20.4s, T0.4s
	add		v24.4s, v24.4s, T0.4s
	add		v28.4s, v28.4s, T0.4s

	add		v17.4s, v17.4s, T1.4s
	add		v21.4s, v21.4s, T1.4s
	add		v25.4s, v25.4s, T1.4s
	add		v29.4s, v29.4s, T1.4s

	add		v18.4s, v18.4s, T2.4s
	add		v22.4s, v22.4s, T2.4s
	add		v26.4s, v26.4s, T2.4s
	add		v30.4s, v30.4s, T2.4s

	st1		{v16.16b,v17.16b,v18.16b,v19.16b}, [DST], #64
	st1		{v20.16b,v21.16b,v22.16b,v23.16b}, [DST], #64
	st1		{v24.16b,v25.16b,v26.16b,v27.16b}, [DST], #64
	st1		{v28.16b,v29.16b,v30.16b,v31.16b}, [DST]

	C Restore callee-save registers
	fmov	d8, x3
	ret
EPILOGUE(_nettle_chacha_4core)

PROLOGUE(_nettle_chacha_4core32)
	eor		TMP2.16b, TMP2.16b, TMP2.16b	C Ignore counter carries
	b		.Lshared_entry
EPILOGUE(_nettle_chacha_4core32)

.align	4
.Lcnts: .long	0,1,2,3		C increments
.Lrot24: .long	0x02010003,0x06050407,0x0a09080b,0x0e0d0c0f
