C powerpc64/ecc-secp384r1-modp.asm

ifelse(`
   Copyright (C) 2021 Martin Schwenke, Amitay Isaacs & Alastair D´Silva, IBM Corporation

   Based on x86_64/ecc-secp256r1-redc.asm

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

	.file "ecc-secp384r1-modp.asm"

C Register usage:

define(`SP', `r1')

define(`RP', `r4')
define(`XP', `r5')

define(`D5', `r6')
define(`T0', `r7')
define(`T1', `r8')
define(`T2', `r9')
define(`T3', `r10')
define(`T4', `r11')
define(`T5', `r12')
define(`H0', `r14')
define(`H1', `r15')
define(`H2', `r16')
define(`H3', `r17')
define(`H4', `r18')
define(`H5', `r19')
define(`C2', `r3')
define(`C0', H5)	C Overlap
define(`TMP', XP)	C Overlap


	C void ecc_secp384r1_modp (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp)
	.text
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ecc_secp384r1_modp)

	std	r14, -48(SP)
	std	r15, -40(SP)
	std	r16, -32(SP)
	std	r17, -24(SP)
	std	r18, -16(SP)
	std	r19, -8(SP)

	C First get top 2 limbs, which need folding twice.
	C B^10 = B^6 + B^4 + 2^32 (B-1)B^4.
	C We handle the terms as follow:
	C
	C B^6: Folded immediatly.
	C
	C B^4: Delayed, added in in the next folding.
	C
	C 2^32(B-1) B^4: Low half limb delayed until the next
	C folding. Top 1.5 limbs subtracted and shifter now, resulting
	C in 2.5 limbs. The low limb saved in D5, high 1.5 limbs added
	C in.

	ld	H4, 80(XP)
	ld	H5, 88(XP)
	C Shift right 32 bits, into H1, H0
	srdi	H1, H5, 32
	sldi	D5, H5, 32
	srdi	H0, H4, 32
	or	H0, H0, D5

	C	H1 H0
	C       -  H1 H0
	C       --------
	C       H1 H0 D5
	subfic	D5, H0, 0
	subfe	H0, H1, H0
	addme	H1, H1

	li	C2, 0
	addc	H0, H4, H0
	adde	H1, H5, H1
	addze	C2, C2

	C Add in to high part
	ld	T1, 48(XP)
	ld	T2, 56(XP)
	addc	H0, T1, H0
	adde	H1, T2, H1
	addze	C2, C2		C Do C2 later

	C +1 term
	ld	T0, 0(XP)
	ld	T1, 8(XP)
	ld	T2, 16(XP)
	ld	T3, 24(XP)
	ld	T4, 32(XP)
	ld	T5, 40(XP)
	ld	H2, 64(XP)
	ld	H3, 72(XP)
	addc	T0, H0, T0
	adde	T1, H1, T1
	adde	T2, H2, T2
	adde	T3, H3, T3
	adde	T4, H4, T4
	adde	T5, H5, T5
	li	C0, 0
	addze	C0, C0

	C +B^2 term
	addc	T2, H0, T2
	adde	T3, H1, T3
	adde	T4, H2, T4
	adde	T5, H3, T5
	addze	C0, C0

	C Shift left, including low half of H4
	sldi	H4, H4, 32
	srdi	TMP, H3, 32
	or	H4, TMP, H4

	sldi	H3, H3, 32
	srdi	TMP, H2, 32
	or	H3, TMP, H3

	sldi	H2, H2, 32
	srdi	TMP, H1, 32
	or	H2, TMP, H2

	sldi	H1, H1, 32
	srdi	TMP, H0, 32
	or	H1, TMP, H1

	sldi	H0, H0, 32

	C   H4 H3 H2 H1 H0  0
	C  -   H4 H3 H2 H1 H0
	C  ---------------
	C   H4 H3 H2 H1 H0 TMP

	subfic	TMP, H0, 0
	subfe	H0, H1, H0
	subfe	H1, H2, H1
	subfe	H2, H3, H2
	subfe	H3, H4, H3
	addme	H4, H4

	addc	T0, TMP, T0
	adde	T1, H0, T1
	adde	T2, H1, T2
	adde	T3, H2, T3
	adde	T4, H3, T4
	adde	T5, H4, T5
	addze	C0, C0

	C Remains to add in C2 and C0
	C Set H1, H0 = (2^96 - 2^32 + 1) C0
	sldi	H1, C0, 32
	subfc	H0, H1, C0
	addme	H1, H1

	C Set H3, H2 = (2^96 - 2^32 + 1) C2
	sldi	H3, C2, 32
	subfc	H2, H3, C2
	addme	H3, H3
	addc	H2, C0, H2

	li	C0, 0
	addc	T0, H0, T0
	adde	T1, H1, T1
	adde	T2, H2, T2
	adde	T3, H3, T3
	adde	T4, C2, T4
	adde	T5, D5, T5		C Value delayed from initial folding
	addze	C0, C0

	C Final unlikely carry
	sldi	H1, C0, 32
	subfc	H0, H1, C0
	addme	H1, H1

	addc	T0, H0, T0
	adde	T1, H1, T1
	adde	T2, C0, T2
	addze	T3, T3
	addze	T4, T4
	addze	T5, T5

	std	T0, 0(RP)
	std	T1, 8(RP)
	std	T2, 16(RP)
	std	T3, 24(RP)
	std	T4, 32(RP)
	std	T5, 40(RP)

	ld	r14, -48(SP)
	ld	r15, -40(SP)
	ld	r16, -32(SP)
	ld	r17, -24(SP)
	ld	r18, -16(SP)
	ld	r19, -8(SP)

	blr
EPILOGUE(_nettle_ecc_secp384r1_modp)
