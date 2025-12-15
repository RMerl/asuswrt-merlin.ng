C x86_64/sha_ni/sha256-compress-n.asm

ifelse(`
   Copyright (C) 2018, 2022 Niels Möller

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
define(`STATE', `%rdi')
define(`K', `%rsi')
define(`BLOCKS', `%rdx')
define(`INPUT', `%rcx')

define(`MSGK',`%xmm0')	C Implicit operand of sha256rnds2
define(`MSG0',`%xmm1')
define(`MSG1',`%xmm2')
define(`MSG2',`%xmm3')
define(`MSG3',`%xmm4')
define(`ABEF',`%xmm5')
define(`CDGH',`%xmm6')
define(`ABEF_ORIG',`%xmm7')
define(`CDGH_ORIG', `%xmm8')
define(`SWAP_MASK',`%xmm9')
define(`TMP', `%xmm10')

C QROUND(M0, M1, M2, M3, R)
define(`QROUND', `
	movdqa	eval($5*4)(K), MSGK
	paddd	$1, MSGK
	sha256rnds2 ABEF, CDGH
	pshufd	`$'0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF
	movdqa	$1, TMP
	palignr	`$'4, $4, TMP
	paddd	TMP, $2
	sha256msg2 $1, $2
	sha256msg1 $1, $4
	')

C FIXME: Do something more clever, taking the pshufd into account.
C TRANSPOSE(ABCD, EFGH, scratch) --> untouched, ABEF, CDGH
define(`TRANSPOSE', `
	movdqa	$2, $3
	punpckhqdq $1, $2
	punpcklqdq $1, $3
')

	C const uint8_t *
	C _nettle_sha256_compress_n(uint32_t *state, const uint32_t *k,
	C                           size_t blocks, const uint8_t *input)

	.text
	ALIGN(16)
.Lswap_mask:
	.byte 3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12
PROLOGUE(_nettle_sha256_compress_n)
	W64_ENTRY(4, 11)
	test	BLOCKS, BLOCKS
	jz	.Lend

	movups	(STATE), TMP
	movups	16(STATE), ABEF

	pshufd	$0x1b, TMP, TMP
	pshufd	$0x1b, ABEF, ABEF

	TRANSPOSE(TMP, ABEF, CDGH)

	movdqa	.Lswap_mask(%rip), SWAP_MASK

.Loop:
	movups	(INPUT), MSG0
	pshufb	SWAP_MASK, MSG0

	movdqa	ABEF, ABEF_ORIG
	movdqa	CDGH, CDGH_ORIG

	movdqa	(K), MSGK
	paddd	MSG0, MSGK
	sha256rnds2 ABEF, CDGH		C Round 0-1
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 2-3

	movups	16(INPUT), MSG1
	pshufb	SWAP_MASK, MSG1

	movdqa	16(K), MSGK
	paddd	MSG1, MSGK
	sha256rnds2 ABEF, CDGH		C Round 4-5
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 6-7
	sha256msg1 MSG1, MSG0

	movups	32(INPUT), MSG2
	pshufb	SWAP_MASK, MSG2

	movdqa	32(K), MSGK
	paddd	MSG2, MSGK
	sha256rnds2 ABEF, CDGH		C Round 8-9
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 10-11
	sha256msg1 MSG2, MSG1

	movups	48(INPUT), MSG3
	pshufb	SWAP_MASK, MSG3

	QROUND(MSG3, MSG0, MSG1, MSG2, 12)	C Round 12-15
	QROUND(MSG0, MSG1, MSG2, MSG3, 16)
	QROUND(MSG1, MSG2, MSG3, MSG0, 20)
	QROUND(MSG2, MSG3, MSG0, MSG1, 24)
	QROUND(MSG3, MSG0, MSG1, MSG2, 28)
	QROUND(MSG0, MSG1, MSG2, MSG3, 32)
	QROUND(MSG1, MSG2, MSG3, MSG0, 36)
	QROUND(MSG2, MSG3, MSG0, MSG1, 40)
	QROUND(MSG3, MSG0, MSG1, MSG2, 44)
	QROUND(MSG0, MSG1, MSG2, MSG3, 48)

	movdqa	208(K), MSGK
	paddd	MSG1, MSGK
	sha256rnds2 ABEF, CDGH		C Round 52-53
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 54-55
	movdqa	MSG1, TMP
	palignr	$4, MSG0, TMP
	paddd	TMP, MSG2
	sha256msg2 MSG1, MSG2

	movdqa	224(K), MSGK
	paddd	MSG2, MSGK
	sha256rnds2 ABEF, CDGH		C Round 56-57
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 58-59
	movdqa	MSG2, TMP
	palignr	$4, MSG1, TMP
	paddd	TMP, MSG3
	sha256msg2 MSG2, MSG3

	movdqa	240(K), MSGK
	paddd	MSG3, MSGK
	sha256rnds2 ABEF, CDGH		C Round 60-61
	pshufd	$0xe, MSGK, MSGK
	sha256rnds2 CDGH, ABEF		C Round 62-63

	paddd ABEF_ORIG, ABEF
	paddd CDGH_ORIG, CDGH

	add	$64, INPUT
	dec	BLOCKS
	jnz	.Loop

	TRANSPOSE(ABEF, CDGH, TMP)

	pshufd	$0x1b, CDGH, CDGH
	pshufd	$0x1b, TMP, TMP
	movups	CDGH, 0(STATE)
	movups	TMP, 16(STATE)

.Lend:
	mov	INPUT, %rax
	W64_EXIT(4, 11)
	ret
EPILOGUE(_nettle_sha256_compress_n)
