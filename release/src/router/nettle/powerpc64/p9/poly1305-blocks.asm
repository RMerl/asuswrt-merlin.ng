C powerpc64/p9/poly1305-blocks.asm

ifelse(`
   Copyright (C) 2013, 2022 Niels Möller
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

include_src(`powerpc64/p9/poly1305.m4')

C Register usage:

define(`SP', `r1')
define(`TOCP', `r2')

C Argments
define(`CTX', `r3')
define(`BLOCKS', `r4')
define(`DATA', `r5')

define(`PADBYTE', `r6') C Padding byte register

define(`DEFINES_BLOCK_R44', `
	define(`R0', `v0')
	define(`R1', `v1')
	define(`R2', `v2')
	define(`S1', `v3')
	define(`S2', `v4')
	define(`H0', `v5')
	define(`H1', `v6')
	define(`H2', `v7')

	define(`R3', `v8')
	define(`R4', `v9')
	define(`R5', `v10')
	define(`S4', `v11')
	define(`S5', `v12')

	define(`T0', `v13')
	define(`T1', `v14')
	define(`T2', `v15')
	define(`T3', `v16')
	define(`T4', `v17')
	define(`T5', `v18')
	define(`TMP', `v19')
	define(`TMP2', `v20')

	define(`ZERO', `v21')
	define(`MASK44', `v22')
	define(`MASK42L', `v23')
	define(`MASK44L', `v24')
	define(`T4PAD', `v25')
	define(`D40', `v26')
	define(`D20', `v27')
	define(`D24', `v28')
	define(`D44', `v29')
	define(`D2', `v30')
	define(`D4', `v31')
	')

C Compute S_1 = 20 * R_1 and S_2 = 20 * R_2
C COMPUTE_S(S1, S2, R1, R2)
define(`COMPUTE_S', `
	vsld		$1, $3, D2
	vsld		$2, $4, D2
	vaddudm		$1, $1, $3
	vaddudm		$2, $2, $4
	vsld		$1, $1, D2
	vsld		$2, $2, D2
	')

C Convert two-part radix 2^64 to three-part radix 2^44 of four blocks
C R64_TO_R44_4B(VR0, VR1, VR2, VR3, VR4, VR5)
define(`R64_TO_R44_4B', `
	vsrd		$3, $2, D24
	vsrd		$6, $5, D24
	vsrd		TMP, $1, D44
	vsrd		TMP2, $4, D44
	vsld		$2, $2, D20
	vsld		$5, $5, D20
	vor			$2, $2, TMP
	vor			$5, $5, TMP2
	vand		$1, $1, MASK44
	vand		$4, $4, MASK44
	vand		$2, $2, MASK44
	vand		$5, $5, MASK44
	')

C T_0 = R_0 H_0 + S_2 H_1 + S_1 H_2
C T_1 = R_1 H_0 + R_0 H_1 + S_2 H_2
C T_2 = R_2 H_0 + R_1 H_1 + R_0 H_2
C MUL(T0, T1, T2, H0, H1, H2)
define(`MUL', `
	vmsumudm	$1, $4, R0, ZERO
	vmsumudm	$2, $4, R1, ZERO
	vmsumudm	$3, $4, R2, ZERO

	vmsumudm	$1, $5, S2, $1
	vmsumudm	$2, $5, R0, $2
	vmsumudm	$3, $5, R1, $3

	vmsumudm	$1, $6, S1, $1
	vmsumudm	$2, $6, S2, $2
	vmsumudm	$3, $6, R0, $3
	')

C Apply aforenamed equations on four-blocks
C Each two successive blocks are interleaved horizontally
C MUL_4B(T0, T1, T2, H0, H1, H2, H3, H4, H5)
define(`MUL_4B', `
	vmsumudm	$1, $7, R0, ZERO
	vmsumudm	$2, $7, R1, ZERO
	vmsumudm	$3, $7, R2, ZERO

	vmsumudm	$1, $8, S2, $1
	vmsumudm	$2, $8, R0, $2
	vmsumudm	$3, $8, R1, $3

	vmsumudm	$1, $9, S1, $1
	vmsumudm	$2, $9, S2, $2
	vmsumudm	$3, $9, R0, $3

	vmsumudm	$1, $4, R3, $1
	vmsumudm	$2, $4, R4, $2
	vmsumudm	$3, $4, R5, $3

	vmsumudm	$1, $5, S5, $1
	vmsumudm	$2, $5, R3, $2
	vmsumudm	$3, $5, R4, $3

	vmsumudm	$1, $6, S4, $1
	vmsumudm	$2, $6, S5, $2
	vmsumudm	$3, $6, R3, $3
	')

C Reduction phase of two interleaved chains
C RED(H0, H1, H2, T0, T1, T2)
define(`RED', `
	vand		$1, $4, MASK44L
	vsro		$4, $4, D40
	vsrd		$4, $4, D4
	vadduqm		$5, $5, $4
	vand		$2, $5, MASK44L
	vsro		$5, $5, D40
	vsrd		$5, $5, D4
	vadduqm		$6, $6, $5
	vand		$3, $6, MASK42L
	vsro		$6, $6, D40
	vsrd		$6, $6, D2
	vadduqm		$1, $1, $6
	vsld		$6, $6, D2
	vadduqm		$1, $1, $6
	vsrd		TMP, $1, D44
	vand		$1, $1, MASK44L
	vadduqm		$2, $2, TMP
	')

.text

C void _nettle_poly1305_blocks(struct poly1305_ctx *ctx,
C 				size_t length, const uint8_t *data)
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_poly1305_blocks)
	C Save non-volatile vector registers
	std		r31,-8(SP)
	stxv	VSR(v31),-32(SP)
	stxv	VSR(v30),-48(SP)
	stxv	VSR(v29),-64(SP)
	stxv	VSR(v28),-80(SP)
	stxv	VSR(v27),-96(SP)
	stxv	VSR(v26),-112(SP)
	stxv	VSR(v25),-128(SP)
	stxv	VSR(v24),-144(SP)
	stxv	VSR(v23),-160(SP)
	stxv	VSR(v22),-176(SP)
	stxv	VSR(v21),-192(SP)
	stxv	VSR(v20),-208(SP)

	C Initialize padding byte register
	li		PADBYTE, 1

C Process data blocks of number of multiple 4
	DEFINES_BLOCK_R44()
	cmpldi	BLOCKS, POLY1305_BLOCK_THRESHOLD
	blt		Ldata_r64
	srdi	r9, BLOCKS, 2
	andi.	BLOCKS, BLOCKS, 3
	mtctr	r9

	C Initialize constants

	vxor 		ZERO, ZERO, ZERO
	vspltisb	D2, 2
	vspltisb	D4, 4
	addis		r9, TOCP, .mask44@got@ha
	ld			r9, .mask44@got@l(r9)
	lxvd2x		VSR(MASK44), 0, r9
	addi		r9, r9, 16
	lxvd2x		VSR(MASK42L), 0, r9
	addi		r9, r9, 16
	lxvd2x		VSR(D40), 0, r9
	addi		r9, r9, 16
	lxvd2x		VSR(D20), 0, r9
	addi		r9, r9, 16
	lxvd2x		VSR(D24), 0, r9
	addi		r9, r9, 16
	lxvd2x		VSR(D44), 0, r9
	xxmrghd		VSR(MASK44L), VSR(ZERO), VSR(MASK44)

	sldi		r10, PADBYTE, 40
	mtvsrdd		VSR(T4PAD), r10, r10

	C Load key of radix 2^44
	lxsd		R0, 0(CTX)
	lxsd		R1, 8(CTX)
	vsrd		R2, R1, D24
	vsrd		TMP, R0, D44
	vsld		R1, R1, D20
	vor			R1, R1, TMP
	vand		R0, R0, MASK44
	vand		R1, R1, MASK44
	xxmrghd		VSR(R0), VSR(R0), VSR(ZERO)
	xxmrghd		VSR(R1), VSR(R1), VSR(ZERO)
	xxmrghd		VSR(R2), VSR(R2), VSR(ZERO)

	COMPUTE_S(S1, S2, R1, R2)

	C Calculate R^2 = R R

	MUL(T0, T1, T2, R0, R1, R2)
	RED(H0, H1, H2, T0, T1, T2)
	xxpermdi	VSR(R0), VSR(R0), VSR(H0), 0b01
	xxpermdi	VSR(R1), VSR(R1), VSR(H1), 0b01
	xxpermdi	VSR(R2), VSR(R2), VSR(H2), 0b01

	COMPUTE_S(S1, S2, R1, R2)

	C Calculate R^3 = R^2 R

	xxmrghd		VSR(R3), VSR(ZERO), VSR(R0)
	xxmrghd		VSR(R4), VSR(ZERO), VSR(R1)
	xxmrghd		VSR(R5), VSR(ZERO), VSR(R2)

	MUL(T0, T1, T2, R3, R4, R5)
	RED(H0, H1, H2, T0, T1, T2)

	C Calculate R^4 = R^2 R^2

	xxmrgld		VSR(R3), VSR(ZERO), VSR(R0)
	xxmrgld		VSR(R4), VSR(ZERO), VSR(R1)
	xxmrgld		VSR(R5), VSR(ZERO), VSR(R2)

	MUL(T0, T1, T2, R3, R4, R5)
	RED(R3, R4, R5, T0, T1, T2)
	xxmrgld		VSR(R3), VSR(H0), VSR(R3)
	xxmrgld		VSR(R4), VSR(H1), VSR(R4)
	xxmrgld		VSR(R5), VSR(H2), VSR(R5)

	COMPUTE_S(S4, S5, R4, R5)

	C Load state
	ld			r7, 32(CTX)
	ld			r8, 40(CTX)
	ld			r31, 48(CTX)

	C Fold high part of H2
	srdi		r9, r31, 2
	sldi		r10, r9, 2
	add			r10, r10, r9
	andi.		r31, r31, 3
	li			r9, 0
	addc		r7, r7, r10
	adde		r8, r8, r9
	adde		r31, r31, r9

	mtvsrdd		VSR(H0), 0, r7
	mtvsrdd		VSR(H1), 0, r8
	mtvsrdd		VSR(H2), 0, r31

	C Convert state of radix 2^64 to 2^44
	vsrd		TMP, H1, D24
	vsld		H2, H2, D40
	vor			H2, H2, TMP
	vsrd		TMP2, H0, D44
	vsld		H1, H1, D20
	vor			H1, H1, TMP2
	vand		H0, H0, MASK44
	vand		H1, H1, MASK44

	li			r8, 0x10
	li			r9, 0x20
	li			r10, 0x30
L4B_loop:
	C Load four blocks
	lxvd2x		VSR(T3), 0, DATA
	lxvd2x		VSR(T4), r8, DATA
	lxvd2x		VSR(T5), r9, DATA
	lxvd2x		VSR(TMP), r10, DATA
IF_BE(`
	xxbrd		VSR(T3), VSR(T3)
	xxbrd		VSR(T4), VSR(T4)
	xxbrd		VSR(T5), VSR(T5)
	xxbrd		VSR(TMP), VSR(TMP)
')
	C Permute blocks in little-endian and line each two successive
	C blocks horizontally
	xxmrghd		VSR(T0), VSR(T4), VSR(T3)
	xxmrgld		VSR(T1), VSR(T4), VSR(T3)
	xxmrghd		VSR(T3), VSR(TMP), VSR(T5)
	xxmrgld		VSR(T4), VSR(TMP), VSR(T5)
	R64_TO_R44_4B(T0, T1, T2, T3, T4, T5)
	vor			T2, T2, T4PAD
	vor			T5, T5, T4PAD

	C Combine first block with previous state
	vaddudm		H0, H0, T0
	vaddudm		H1, H1, T1
	vaddudm		H2, H2, T2

	MUL_4B(T0, T1, T2, H0, H1, H2, T3, T4, T5)
	RED(H0, H1, H2, T0, T1, T2)

	addi		DATA, DATA, 64
	bdnz		L4B_loop

	C Moving carry
	vsrd		TMP, H1, D44
	vaddudm		H2, H2, TMP
	vsrd		TMP2, H2, D40
	vsrd		TMP2, TMP2, D2
	vsld		TMP, TMP2, D2
	vand		H1, H1, MASK44
	vaddudm		TMP2, TMP2, TMP
	vaddudm		H0, H0, TMP2
	vsrd		TMP, H0, D44
	vaddudm		H1, H1, TMP
	vand		H2, H2, MASK42L
	vand		H0, H0, MASK44

	C Convert state of radix 2^44 to 2^64
	vsld		TMP, H1, D44
	vor			H0, H0, TMP
	vsrd		H1, H1, D20
	vsld		TMP2, H2, D24
	vor			H1, H1, TMP2
	vsrd		H2, H2, D40

	xxswapd		VSR(H0), VSR(H0)
	xxswapd		VSR(H1), VSR(H1)
	xxswapd		VSR(H2), VSR(H2)

	C Store state
	stxsd		H0, 32(CTX)
	stxsd		H1, 40(CTX)
	stxsd		H2, 48(CTX)

Ldata_r64:
	cmpldi	BLOCKS, 0
	beq		Ldone
	mtctr	BLOCKS
	mr			r4, PADBYTE
	ld			r6, P1305_H0 (CTX)
	ld			r7, P1305_H1 (CTX)
	ld			r8, P1305_H2 (CTX)
L1B_loop:
	BLOCK_R64(CTX,DATA,r4,r6,v0)
	mfvsrld		r6, VSR(v0)
	mfvsrld		r7, VSR(v1)
	mfvsrd		r8, VSR(v1)
	addi	DATA, DATA, 16
	bdnz	L1B_loop
	std		r6, P1305_H0 (CTX)
	std		r7, P1305_H1 (CTX)
	std		r8, P1305_H2 (CTX)

Ldone:
	C Restore non-volatile vector registers
	ld		r31, -8(SP)
	lxv		VSR(v31),-32(SP)
	lxv		VSR(v30),-48(SP)
	lxv		VSR(v29),-64(SP)
	lxv		VSR(v28),-80(SP)
	lxv		VSR(v27),-96(SP)
	lxv		VSR(v26),-112(SP)
	lxv		VSR(v25),-128(SP)
	lxv		VSR(v24),-144(SP)
	lxv		VSR(v23),-160(SP)
	lxv		VSR(v22),-176(SP)
	lxv		VSR(v21),-192(SP)
	lxv		VSR(v20),-208(SP)

	mr		r3, DATA

	blr
EPILOGUE(_nettle_poly1305_blocks)

.rodata
.align 4
.mask44:
.quad 0x00000FFFFFFFFFFF,0x00000FFFFFFFFFFF
.mask42l:
.quad 0x0000000000000000,0x000003FFFFFFFFFF
.d40:
.quad 0x0000000000000028,0x0000000000000028
.d20:
.quad 0x0000000000000014,0x0000000000000014
.d24:
.quad 0x0000000000000018,0x0000000000000018
.d44:
.quad 0x000000000000002C,0x000000000000002C
