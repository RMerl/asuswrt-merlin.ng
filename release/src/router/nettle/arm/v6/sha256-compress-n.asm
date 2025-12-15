C arm/v6/sha256-compress-n.asm

ifelse(`
   Copyright (C) 2013, 2022 Niels Möller

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

	.file "sha256-compress-n.asm"
	.arch armv6

define(`STATE', `r0')
define(`K', `r1')
define(`BLOCKS', `r2')
define(`INPUT', `r3')
define(`SA', `r2')	C Overlap BLOCKS
define(`SB', `r4')
define(`SC', `r5')
define(`SD', `r6')
define(`SE', `r7')
define(`SF', `r8')
define(`SG', `r10')
define(`SH', `r11')
define(`T0', `r12')
define(`T1', `r3')	C Overlap INPUT
define(`COUNT', `r0')	C Overlap STATE
define(`W', `r14')

C Used for data load. Must not clobber STATE (r0), K (r1) or INPUT (r3)
define(`I0', `r2')
define(`I1', `r4')
define(`I2', `r5')
define(`I3', `r6')
define(`I4', `r7')
define(`DST', `r8')
define(`SHIFT', `r10')
define(`ILEFT', `r11')

define(`EXPN', `
	ldr	W, [sp, #+eval(4*$1)]
	ldr	T0, [sp, #+eval(4*(($1 + 14) % 16))]
	ror	T1, T0, #17
	eor	T1, T1, T0, ror #19
	eor	T1, T1, T0, lsr #10
	add	W, W, T1
	ldr	T0, [sp, #+eval(4*(($1 + 9) % 16))]
	add	W, W, T0
	ldr	T0, [sp, #+eval(4*(($1 + 1) % 16))]
	ror	T1, T0, #7
	eor	T1, T1, T0, ror #18
	eor	T1, T1, T0, lsr #3
	add	W, W, T1
	str	W, [sp, #+eval(4*$1)]
')

C ROUND(A,B,C,D,E,F,G,H)
C
C H += S1(E) + Choice(E,F,G) + K + W
C D += H
C H += S0(A) + Majority(A,B,C)
C
C Where
C
C S1(E) = E<<<26 ^ E<<<21 ^ E<<<7
C S0(A) = A<<<30 ^ A<<<19 ^ A<<<10
C Choice (E, F, G) = G^(E&(F^G))
C Majority (A,B,C) = (A&B) + (C&(A^B))

define(`ROUND', `
	ror	T0, $5, #6
	eor	T0, T0, $5, ror #11
	eor	T0, T0, $5, ror #25
	add	$8, $8, T0
	eor	T0, $6, $7
	and	T0, T0, $5
	eor	T0, T0, $7
	add	$8,$8, T0
	ldr	T0, [K], #+4
	add	$8, $8, W
	add	$8, $8, T0
	add	$4, $4, $8
	ror	T0, $1, #2
	eor	T0, T0, $1, ror #13
	eor	T0, T0, $1, ror #22
	add	$8, $8, T0
	and	T0, $1, $2
	add	$8, $8, T0
	eor	T0, $1, $2
	and	T0, T0, $3
	add	$8, $8, T0
')

define(`NOEXPN', `
	ldr	W, [sp, + $1]
	add	$1, $1, #4
')
	.text
	.align 2

define(`SHIFT_OFFSET', 64)
define(`INPUT_OFFSET', 68)
define(`I0_OFFSET', 72)
define(`STATE_OFFSET', 76)
define(`K_OFFSET', 80)
define(`BLOCKS_OFFSET', 84)

	C const uint8_t *
	C _nettle_sha256_compress_n(uint32_t *state, const uint32_t *k,
	C                           size_t blocks, const uint8_t *input)

PROLOGUE(_nettle_sha256_compress_n)
	cmp	BLOCKS, #0
	bne	.Lwork

	mov	r0, INPUT
	bx lr

.Lwork:
	C Also save STATE (r0), K (r1) and BLOCKS (r2)
	push	{r0,r1,r2,r4,r5,r6,r7,r8,r10,r11,r12,r14}
	sub	sp, sp, #STATE_OFFSET

	C Load data up front, since we don't have enough registers
	C to load and shift on-the-fly
	ands	SHIFT, INPUT, #3
	and	INPUT, INPUT, $-4
	ldr	I0, [INPUT]
	addne	INPUT, INPUT, #4
	lsl	SHIFT, SHIFT, #3
	mov	T0, #0
	movne	T0, #-1
IF_LE(`	lsl	I1, T0, SHIFT')
IF_BE(`	lsr	I1, T0, SHIFT')
	uadd8	T0, T0, I1		C Sets APSR.GE bits
	C on BE rotate right by 32-SHIFT bits
	C because there is no rotate left
IF_BE(`	rsb	SHIFT, SHIFT, #32')

	str	SHIFT, [sp, #SHIFT_OFFSET]

.Loop_block:
	mov	DST, sp
	mov	ILEFT, #4
.Lcopy:
	ldm	INPUT!, {I1,I2,I3,I4}
	sel	I0, I0, I1
	ror	I0, I0, SHIFT
IF_LE(`	rev	I0, I0')
	sel	I1, I1, I2
	ror	I1, I1, SHIFT
IF_LE(`	rev	I1, I1')
	sel	I2, I2, I3
	ror	I2, I2, SHIFT
IF_LE(`	rev	I2, I2')
	sel	I3, I3, I4
	ror	I3, I3, SHIFT
IF_LE(`	rev	I3, I3')
	subs	ILEFT, ILEFT, #1
	stm	DST!, {I0,I1,I2,I3}
	mov	I0, I4	
	bne	.Lcopy

	str	INPUT, [sp, #INPUT_OFFSET]
	str	I0, [sp, #I0_OFFSET]

	C Process block, with input at sp, expanded on the fly

	ldm	STATE, {SA,SB,SC,SD,SE,SF,SG,SH}

	mov	COUNT,#0

.Loop1:
	NOEXPN(COUNT) ROUND(SA,SB,SC,SD,SE,SF,SG,SH)
	NOEXPN(COUNT) ROUND(SH,SA,SB,SC,SD,SE,SF,SG)
	NOEXPN(COUNT) ROUND(SG,SH,SA,SB,SC,SD,SE,SF)
	NOEXPN(COUNT) ROUND(SF,SG,SH,SA,SB,SC,SD,SE)
	NOEXPN(COUNT) ROUND(SE,SF,SG,SH,SA,SB,SC,SD)
	NOEXPN(COUNT) ROUND(SD,SE,SF,SG,SH,SA,SB,SC)
	NOEXPN(COUNT) ROUND(SC,SD,SE,SF,SG,SH,SA,SB)
	NOEXPN(COUNT) ROUND(SB,SC,SD,SE,SF,SG,SH,SA)
	cmp	COUNT,#64
	bne	.Loop1

	mov	COUNT, #3
.Loop2:
	
	EXPN( 0) ROUND(SA,SB,SC,SD,SE,SF,SG,SH)
	EXPN( 1) ROUND(SH,SA,SB,SC,SD,SE,SF,SG)
	EXPN( 2) ROUND(SG,SH,SA,SB,SC,SD,SE,SF)
	EXPN( 3) ROUND(SF,SG,SH,SA,SB,SC,SD,SE)
	EXPN( 4) ROUND(SE,SF,SG,SH,SA,SB,SC,SD)
	EXPN( 5) ROUND(SD,SE,SF,SG,SH,SA,SB,SC)
	EXPN( 6) ROUND(SC,SD,SE,SF,SG,SH,SA,SB)
	EXPN( 7) ROUND(SB,SC,SD,SE,SF,SG,SH,SA)
	EXPN( 8) ROUND(SA,SB,SC,SD,SE,SF,SG,SH)
	EXPN( 9) ROUND(SH,SA,SB,SC,SD,SE,SF,SG)
	EXPN(10) ROUND(SG,SH,SA,SB,SC,SD,SE,SF)
	EXPN(11) ROUND(SF,SG,SH,SA,SB,SC,SD,SE)
	EXPN(12) ROUND(SE,SF,SG,SH,SA,SB,SC,SD)
	EXPN(13) ROUND(SD,SE,SF,SG,SH,SA,SB,SC)
	EXPN(14) ROUND(SC,SD,SE,SF,SG,SH,SA,SB)
	subs	COUNT, COUNT, #1
	EXPN(15) ROUND(SB,SC,SD,SE,SF,SG,SH,SA)
	bne	.Loop2

	ldr	STATE, [sp, #STATE_OFFSET]
	C No longer needed registers
	ldm	STATE, {K, T1, T0, W}
	add	SA, SA, K
	add	SB, SB, T1
	add	SC, SC, T0
	add	SD, SD, W
	stm	STATE!, {SA,SB,SC,SD}
	ldm	STATE, {K, T1, T0, W}
	add	SE, SE, K
	add	SF, SF, T1
	add	SG, SG, T0
	add	SH, SH, W
	stm	STATE, {SE,SF,SG,SH}
	sub	STATE, STATE, #16

	ldr	BLOCKS, [sp, #BLOCKS_OFFSET]
	subs	BLOCKS, BLOCKS, #1
	str	BLOCKS, [sp, #BLOCKS_OFFSET]

	ldr	SHIFT, [sp, #SHIFT_OFFSET]
	ldr	K, [sp, #K_OFFSET]
	ldr	INPUT, [sp, #INPUT_OFFSET]
	ldr	I0, [sp, #I0_OFFSET]

	bne	.Loop_block

	C Restore input pointer adjustment
IF_BE(`	rsbs	SHIFT, SHIFT, #32')
IF_LE(` cmp	SHIFT, #0')
	subne	INPUT, INPUT, #4
	orr	r0, INPUT, SHIFT, lsr #3

	C Discard saved STATE, K and BLOCKS.
	add	sp, sp, #STATE_OFFSET + 12
	pop	{r4,r5,r6,r7,r8,r10,r11,r12,pc}
EPILOGUE(_nettle_sha256_compress_n)
