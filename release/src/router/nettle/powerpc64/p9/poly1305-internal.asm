C powerpc64/p9/poly1305-internal.asm

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
define(`DATA', `r4')
define(`PADBYTE', `r5') C Padding byte register

.text

C _poly1305_set_key(struct poly1305_ctx *ctx, const uint8_t key[16])
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_poly1305_set_key)
	li			r9, 0
	addis		r5, TOCP, .key_mask@got@ha
	ld			r5, .key_mask@got@l(r5)
	ld			r8, 0(r5)
	ori			r7, r8, 3

	C Load R_0 and R_1
IF_LE(`
	ld			r5, 0(r4)
	ld			r6, 8(r4)
')
IF_BE(`
	ldbrx		r5, 0, r4
	addi		r4, r4, 8
	ldbrx		r6, 0, r4
')
	and			r5, r5, r7        C R_0 &= 0x0FFFFFFC0FFFFFFF
	and			r6, r6, r8        C R_1 &= 0x0FFFFFFC0FFFFFFC

	srdi		r10, r6, 2
	sldi		r7, r5, 2
	sldi		r8, r10, 2
	add			r7, r7, r5
	add			r8, r8, r10

	C Store key
	std			r5, P1305_R0 (r3)
	std			r6, P1305_R1 (r3)
	std			r7, P1305_S0 (r3)
	std			r8, P1305_S1 (r3)
	C Reset state
	std			r9, P1305_H0 (r3)
	std			r9, P1305_H1 (r3)
	std			r9, P1305_H2 (r3)

	blr
EPILOGUE(_nettle_poly1305_set_key)

C void _nettle_poly1305_block(struct poly1305_ctx *ctx, const uint8_t *m, unsigned m128)
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_poly1305_block)
	ld			r6, P1305_H0 (CTX)
	ld			r7, P1305_H1 (CTX)
	ld			r8, P1305_H2 (CTX)

	BLOCK_R64(CTX,DATA,PADBYTE,r6,v0)

	li			r10, P1305_H1
	xxswapd		VSR(v0), VSR(v0)
	xxswapd		VSR(v1), VSR(v1)
	stxsd		v0, P1305_H0 (CTX)
	stxvd2x		VSR(v1), r10, CTX

	blr
EPILOGUE(_nettle_poly1305_block)

C _poly1305_digest (struct poly1305_ctx *ctx, uint8_t *s)
define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_poly1305_digest)
	C Load current state
	ld			r5, P1305_H0 (r3)
	ld			r6, P1305_H1 (r3)
	ld			r7, P1305_H2 (r3)

	C Fold high part of H2
	li			r10, 0
	srdi		r9, r7, 2
	sldi		r8, r9, 2
	add			r8, r8, r9
	andi.		r7, r7, 3
	addc		r5, r5, r8
	adde		r6, r6, r10
	adde		r7, r7, r10

	C Add 5 to state, save result if it carries
	li			r8, 5
	li			r9, 0
	li			r10, -4
	addc		r8, r8, r5
	adde		r9, r9, r6
	adde.		r10, r10, r7
	iseleq		r5, r8, r5
	iseleq		r6, r9, r6

	C Load digest
IF_LE(`
	ld			r7, 0(r4)
	ld			r8, 8(r4)
')
IF_BE(`
	li			r10, 8
	ldbrx		r7, 0, r4
	ldbrx		r8, r10, r4
')

	C Add hash to digest
	addc		r5, r5, r7
	adde		r6, r6, r8

	C Store digest
IF_LE(`
	std			r5, 0(r4)
	std			r6, 8(r4)
')
IF_BE(`
	stdbrx		r5, 0, r4
	stdbrx		r6, r10, r4
')
	C Reset hash
	li			r9, 0
	std			r9, P1305_H0 (r3)
	std			r9, P1305_H1 (r3)
	std			r9, P1305_H2 (r3)

	blr
EPILOGUE(_nettle_poly1305_digest)

.rodata
.align 3
.key_mask:
.quad 0x0FFFFFFC0FFFFFFC
