C arm64/crypto/ghash-set-key.asm

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

.file "ghash-set-key.asm"
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

    C void _ghash_set_key (struct gcm_key *ctx, const union nettle_block16 *key)

C This function populates the gcm table as the following layout
C *******************************************************************************
C | H1M = (H1 div x⁶⁴)||((H1 mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷)) div x⁶⁴              |
C | H1L = (H1 mod x⁶⁴)||(((H1 mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)) mod x⁶⁴) + (H1 div x⁶⁴) |
C |                                                                             |
C | H2M = (H2 div x⁶⁴)||((H2 mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷)) div x⁶⁴              |
C | H2L = (H2 mod x⁶⁴)||(((H2 mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)) mod x⁶⁴) + (H2 div x⁶⁴) |
C |                                                                             |
C | H3M = (H3 div x⁶⁴)||((H3 mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷)) div x⁶⁴              |
C | H3L = (H3 mod x⁶⁴)||(((H3 mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)) mod x⁶⁴) + (H3 div x⁶⁴) |
C |                                                                             |
C | H4M = (H3 div x⁶⁴)||((H4 mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷)) div x⁶⁴              |
C | H4L = (H3 mod x⁶⁴)||(((H4 mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)) mod x⁶⁴) + (H4 div x⁶⁴) |
C *******************************************************************************

C Register usage:
define(`CTX', `x0')
define(`KEY', `x1')

define(`EMSB', `v0')
define(`B', `v1')
define(`H', `v2')
define(`H2', `v3')
define(`H3', `v4')
define(`H4', `v5')
define(`Hp', `v20')
define(`Hl', `v21')
define(`Hm', `v22')
define(`H1M', `v23')
define(`H1L', `v24')
define(`H2M', `v25')
define(`H2L', `v26')
define(`H3M', `v27')
define(`H3L', `v28')
define(`H4M', `v29')
define(`H4L', `v30')

C PMUL_PARAM(in, param1, param2)
define(`PMUL_PARAM', m4_assert_numargs(3)`
    pmull2         Hp.1q,$1.2d,POLY.2d
    eor            Hm.16b,$1.16b,Hp.16b
    ext            $2.16b,Hm.16b,$1.16b,#8
    ext            $3.16b,$1.16b,Hm.16b,#8
    ext            $2.16b,$2.16b,$2.16b,#8
')

PROLOGUE(_nettle_ghash_set_key)
    ld1            {H.2d},[KEY]

    C we treat data as big-endian doublewords for processing. Since there is no
    C endianness-neutral MSB-first load operation we need to restore our desired
    C byte order on little-endian systems. The same holds true for DATA below
    C but not our own internal precalculated CTX (see below).
IF_LE(`
    rev64          H.16b,H.16b
')
    C --- calculate H = H × x mod R(X); R(X) = (x¹²⁸+x¹²⁷+x¹²⁶+x¹²¹+1) ---

    dup            EMSB.16b,H.b[7]
    mov            x1,#0xC200000000000000
    mov            x2,#1
    mov            POLY.d[0],x1
    mov            POLY.d[1],x2
    sshr           EMSB.16b,EMSB.16b,#7
    and            EMSB.16b,EMSB.16b,POLY.16b
    ushr           B.2d,H.2d,#63
    and            B.16b,B.16b,POLY.16b
    ext            B.16b,B.16b,B.16b,#8
    shl            H.2d,H.2d,#1
    orr            H.16b,H.16b,B.16b
    eor            H.16b,H.16b,EMSB.16b

    dup            POLY.2d,POLY.d[0]

    C --- calculate H^2 = H × H ---

    PMUL_PARAM(H,H1M,H1L)

    PMUL(H,H1M,H1L)

    REDUCTION(H2)

    PMUL_PARAM(H2,H2M,H2L)

    C we store to the table as doubleword-vectors in current memory endianness
    C because it's our own strictly internal data structure and what ghash_update
    C can most naturally use
    st1            {H1M.2d,H1L.2d,H2M.2d,H2L.2d},[CTX],#64

    C --- calculate H^3 = H^1 × H^2 ---

    PMUL(H2,H1M,H1L)

    REDUCTION(H3)

    PMUL_PARAM(H3,H3M,H3L)

    C --- calculate H^4 = H^2 × H^2 ---

    PMUL(H2,H2M,H2L)

    REDUCTION(H4)

    PMUL_PARAM(H4,H4M,H4L)

    st1            {H3M.2d,H3L.2d,H4M.2d,H4L.2d},[CTX]

    ret
EPILOGUE(_nettle_ghash_set_key)
