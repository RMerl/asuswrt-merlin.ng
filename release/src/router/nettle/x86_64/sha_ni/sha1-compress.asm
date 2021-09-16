C x86_64/sha_ni/sha1-compress.asm

ifelse(`
   Copyright (C) 2018 Niels Möller

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

C Register usage.

C Arguments
define(`STATE',`%rdi')dnl
define(`INPUT',`%rsi')dnl

define(`MSG0',`%xmm0')
define(`MSG1',`%xmm1')
define(`MSG2',`%xmm2')
define(`MSG3',`%xmm3')
define(`ABCD',`%xmm4')
define(`E0',`%xmm5')
define(`E1',`%xmm6')
define(`ABCD_ORIG', `%xmm7')
define(`E_ORIG', `%xmm8')
define(`SWAP_MASK',`%xmm9')

C QROUND(M0, M1, M2, M3, E0, E1, TYPE)
define(`QROUND', `
	sha1nexte $1, $5
	movdqa	ABCD, $6
	sha1msg2 $1, $2
	sha1rnds4 `$'$7, $5, ABCD
	sha1msg1 $1, $4
	pxor	$1, $3
')

	.file "sha1-compress.asm"

	C nettle_sha1_compress(uint32_t *state, uint8_t *input)

	.text
	ALIGN(16)
.Lswap_mask:
	.byte 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
PROLOGUE(nettle_sha1_compress)
	C save all registers that need to be saved
	W64_ENTRY(2, 10)
	movups	(STATE), ABCD
	movd	16(STATE), E0
	movups	(INPUT), MSG0
	movdqa	.Lswap_mask(%rip), SWAP_MASK
	pshufd	$0x1b, ABCD, ABCD
	pshufd	$0x1b, E0, E0
	movdqa	ABCD, ABCD_ORIG
	movdqa	E0, E_ORIG
	pshufb	SWAP_MASK, MSG0

	paddd	MSG0, E0
	movdqa	ABCD, E1
	sha1rnds4 $0, E0, ABCD	C Rounds 0-3

	movups	16(INPUT), MSG1
	pshufb	SWAP_MASK, MSG1

	sha1nexte MSG1, E1
	movdqa	ABCD, E0
	sha1rnds4 $0, E1, ABCD	C Rounds 4-7
	sha1msg1 MSG1, MSG0

	movups	32(INPUT), MSG2
	pshufb	SWAP_MASK, MSG2

	sha1nexte MSG2, E0
	movdqa	ABCD, E1
	sha1rnds4 $0, E0, ABCD	C Rounds 8-11
	sha1msg1 MSG2, MSG1
	pxor	MSG2, MSG0

	movups	48(INPUT), MSG3
	pshufb	SWAP_MASK, MSG3

	QROUND(MSG3, MSG0, MSG1, MSG2, E1, E0, 0)	C Rounds 12-15
	QROUND(MSG0, MSG1, MSG2, MSG3, E0, E1, 0)	C Rounds 16-19

	QROUND(MSG1, MSG2, MSG3, MSG0, E1, E0, 1)	C Rounds 20-23
	QROUND(MSG2, MSG3, MSG0, MSG1, E0, E1, 1)	C Rounds 24-27
	QROUND(MSG3, MSG0, MSG1, MSG2, E1, E0, 1)	C Rounds 28-31
	QROUND(MSG0, MSG1, MSG2, MSG3, E0, E1, 1)	C Rounds 32-35
	QROUND(MSG1, MSG2, MSG3, MSG0, E1, E0, 1)	C Rounds 36-39

	QROUND(MSG2, MSG3, MSG0, MSG1, E0, E1, 2)	C Rounds 40-43
	QROUND(MSG3, MSG0, MSG1, MSG2, E1, E0, 2)	C Rounds 44-47
	QROUND(MSG0, MSG1, MSG2, MSG3, E0, E1, 2)	C Rounds 48-51
	QROUND(MSG1, MSG2, MSG3, MSG0, E1, E0, 2)	C Rounds 52-55
	QROUND(MSG2, MSG3, MSG0, MSG1, E0, E1, 2)	C Rounds 56-59

	QROUND(MSG3, MSG0, MSG1, MSG2, E1, E0, 3)	C Rounds 60-63
	QROUND(MSG0, MSG1, MSG2, MSG3, E0, E1, 3)	C Rounds 64-67

	sha1nexte MSG1, E1
	movdqa	ABCD, E0
	sha1msg2 MSG1, MSG2
	sha1rnds4 $3, E1, ABCD	C Rounds 68-71
	pxor	MSG1, MSG3

	sha1nexte MSG2, E0
	movdqa	ABCD, E1
	sha1msg2 MSG2, MSG3
	sha1rnds4 $3, E0, ABCD	C Rounds 72-75

	sha1nexte MSG3, E1
	movdqa	ABCD, E0
	sha1rnds4 $3, E1, ABCD	C Rounds 76-79

	sha1nexte E_ORIG, E0
	paddd	ABCD_ORIG, ABCD

	pshufd	$0x1b, ABCD, ABCD
	movups	ABCD, (STATE)
	pshufd	$0x1b, E0, E0
	movd	E0, 16(STATE)

	W64_EXIT(2, 10)
	ret
EPILOGUE(nettle_sha1_compress)
