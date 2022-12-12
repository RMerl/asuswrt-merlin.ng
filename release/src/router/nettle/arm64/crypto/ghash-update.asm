C arm64/crypto/ghash-update.asm

ifelse(`
   Copyright (C) 2020 Niels Möller and Mamone Tarsha
   Copyright (C) 2021 Michael Weiser
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

.file "ghash-update.asm"
.arch armv8-a+crypto

.text

C common SIMD register usage:
define(`POLY', `v6')
C temporary register that assist the reduction procedure
define(`T', `v7')
C permenant register that hold the 16-byte result of pmull
define(`F', `v16')
C permenant register that hold the 16-byte result of pmull2,
C its value is accumulated on 'F' register immediately
define(`F1', `v17')
C permenant register that hold the 16-byte result of pmull
define(`R', `v18')
C permenant register that hold the 16-byte result of pmull2,
C its value is accumulated on 'F' register immediately
define(`R1', `v19')

C common macros:
C long multiply of six 64-bit polynomials and sum
C R = (in.l × param2.l) + (in.h × param2.h)
C F = (in.l × param3.l) + (in.h × param3.h)
C PMUL(in, param1, param2)
define(`PMUL', m4_assert_numargs(3)`
    pmull          F.1q,$3.1d,$1.1d
    pmull2         F1.1q,$3.2d,$1.2d
    pmull          R.1q,$2.1d,$1.1d
    pmull2         R1.1q,$2.2d,$1.2d
    eor            F.16b,F.16b,F1.16b
    eor            R.16b,R.16b,R1.16b
')
C Reduce 'R' and 'F' values to 128-bit output
C REDUCTION(out)
define(`REDUCTION', m4_assert_numargs(1)`
    pmull          T.1q,F.1d,POLY.1d
    eor            R.16b,R.16b,T.16b
    ext            R.16b,R.16b,R.16b,#8
    eor            $1.16b,F.16b,R.16b
')

C register usage:
define(`CTX', `x0')
define(`X', `x1')
define(`BLOCKS', `x2')
define(`DATA', `x3')

define(`D', `v0')
define(`C0', `v1')
define(`C1', `v2')
define(`C2', `v3')
define(`C3', `v4')
define(`R2', `v20')
define(`F2', `v21')
define(`R3', `v22')
define(`F3', `v23')
define(`H1M', `v24')
define(`H1L', `v25')
define(`H2M', `v26')
define(`H2L', `v27')
define(`H3M', `v28')
define(`H3L', `v29')
define(`H4M', `v30')
define(`H4L', `v31')

C PMUL_SUM(in, param1, param2)
define(`PMUL_SUM', m4_assert_numargs(3)`
    pmull          F2.1q,$3.1d,$1.1d
    pmull2         F3.1q,$3.2d,$1.2d
    pmull          R2.1q,$2.1d,$1.1d
    pmull2         R3.1q,$2.2d,$1.2d
    eor            F2.16b,F2.16b,F3.16b
    eor            R2.16b,R2.16b,R3.16b
    eor            F.16b,F.16b,F2.16b
    eor            R.16b,R.16b,R2.16b
')

	C const uint8_t *_ghash_update (const struct gcm_key *key,
	C				union nettle_block16 *x,
	C				size_t blocks, const uint8_t *data)

PROLOGUE(_nettle_ghash_update)
    mov            x4,#0xC200000000000000
    mov            POLY.d[0],x4

    ld1            {D.2d},[X]
IF_LE(`
    rev64          D.16b,D.16b
')

    ands           x4,BLOCKS,#-4
    b.eq           L1_block

    add            x5,CTX,#64
    ld1            {H1M.2d,H1L.2d,H2M.2d,H2L.2d},[CTX]
    ld1            {H3M.2d,H3L.2d,H4M.2d,H4L.2d},[x5]

L4_blocks_loop:
    ld1            {C0.2d,C1.2d,C2.2d,C3.2d},[DATA],#64
IF_LE(`
    rev64          C0.16b,C0.16b
    rev64          C1.16b,C1.16b
    rev64          C2.16b,C2.16b
    rev64          C3.16b,C3.16b
')

    eor            C0.16b,C0.16b,D.16b

    PMUL(C1,H3M,H3L)
    PMUL_SUM(C2,H2M,H2L)
    PMUL_SUM(C3,H1M,H1L)
    PMUL_SUM(C0,H4M,H4L)

    REDUCTION(D)

    subs           x4,x4,#4
    b.ne           L4_blocks_loop

L1_block:
    ands           BLOCKS,BLOCKS,#3
    b.eq           Lghash_done

    ld1            {H1M.2d,H1L.2d},[CTX]

L1_block_loop:
    ld1            {C0.2d},[DATA],#16
IF_LE(`
    rev64          C0.16b,C0.16b
')

    eor            C0.16b,C0.16b,D.16b

    PMUL(C0,H1M,H1L)

    REDUCTION(D)

    subs           BLOCKS, BLOCKS, #1
    b.ne           L1_block_loop

Lghash_done:
IF_LE(`
    rev64          D.16b,D.16b
')
    st1            {D.2d},[X]
    mov            x0, DATA
    ret
EPILOGUE(_nettle_ghash_update)
