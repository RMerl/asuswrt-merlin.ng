C powerpc64/p8/ghash-set-key.asm

ifelse(`
   Copyright (C) 2020, 2022 Niels Möller and Mamone Tarsha
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

define(`SP', `r1')
define(`TOCP', `r2')

define(`CTX', `r3')
define(`KEY', `r4')

define(`ZERO', `v0')
define(`B1', `v1')
define(`EMSB', `v16')
define(`POLY', `v17')
define(`POLY_L', `v1')

define(`H', `v2')
define(`H2', `v3')
define(`H3', `v4')
define(`H4', `v5')
define(`H1M', `v6')
define(`H1L', `v7')
define(`H2M', `v8')
define(`H2L', `v9')
define(`Hl', `v10')
define(`Hm', `v11')
define(`Hp', `v12')
define(`Hl2', `v13')
define(`Hm2', `v14')
define(`Hp2', `v15')
define(`R', `v13')
define(`F', `v14')
define(`T', `v15')
define(`R2', `v16')
define(`F2', `v17')
define(`T2', `v18')

define(`LE_TEMP', `v18')
define(`LE_MASK', `v19')

.file "ghash-set-key.asm"

.text

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

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ghash_set_key)
    DATA_LOAD_VEC(POLY,.polynomial,r7)           C 0xC2000000000000000000000000000001
IF_LE(`
    li             r8,0
    lvsl           LE_MASK,0,r8                  C 0x000102030405060708090A0B0C0D0E0F
    vspltisb       LE_TEMP,0x07                  C 0x07070707070707070707070707070707
    vxor           LE_MASK,LE_MASK,LE_TEMP       C 0x07060504030201000F0E0D0C0B0A0908
')

    C 'H' is assigned by gcm_set_key() to the middle element of the table
    lxvd2x         VSR(H),0,KEY                  C load 'H'
    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          H,H,H,LE_MASK
')

    C --- calculate H = H << 1 mod P(X), P(X) = (x¹²⁸+x¹²⁷+x¹²⁶+x¹²¹+1) ---

    vupkhsb        EMSB,H                        C extend most significant bit to first byte
    vspltisb       B1,1                          C 0x01010101010101010101010101010101
    vspltb         EMSB,EMSB,0                   C first byte quadword-extend
    vsl            H,H,B1                        C H = H << 1
    vand           EMSB,EMSB,POLY                C EMSB &= 0xC2000000000000000000000000000001
    vxor           ZERO,ZERO,ZERO                C 0x00000000000000000000000000000000
    vxor           H,H,EMSB                      C H ^= EMSB

    C --- calculate H^2 = H*H ---

    xxmrghd        VSR(POLY_L),VSR(ZERO),VSR(POLY) C 0x0000000000000000C200000000000000

    C --- Hp = (H mod x⁶⁴) / x⁶⁴ mod P(X) ---
    C --- Hp = (H mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷) mod P(X), deg(Hp) ≤ 127 ---
    C --- Hp = (H mod x⁶⁴) × (x⁶⁴+x⁶³+x⁶²+x⁵⁷) ---
    vpmsumd        Hp,H,POLY_L                   C Hp = (H mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)
    xxswapd        VSR(Hm),VSR(H)
    xxmrgld        VSR(Hl),VSR(H),VSR(ZERO)      C Hl = (H mod x⁶⁴) × x⁶⁴
    vxor           Hm,Hm,Hp                      C Hm = Hm + Hp
    vxor           Hl,Hl,Hp                      C Hl = Hl + Hp
    xxmrgld        VSR(H1L),VSR(H),VSR(Hm)       C H1L = (H mod x⁶⁴)||(Hl mod x⁶⁴)
    xxmrghd        VSR(H1M),VSR(H),VSR(Hl)       C H1M = (H div x⁶⁴)||(Hl div x⁶⁴)

    vpmsumd        F,H1L,H                       C F = (H1Lh × Hh) + (H1Ll × Hl)
    vpmsumd        R,H1M,H                       C R = (H1Mh × Hh) + (H1Ml × Hl)

    C --- rduction ---
    vpmsumd        T,F,POLY_L                    C T = (F mod x⁶⁴) × (x⁶³+x⁶²+x⁵⁷)
    xxswapd        VSR(H2),VSR(F)
    vxor           R,R,T                         C R = R + T
    vxor           H2,R,H2

    xxmrgld        VSR(Hl),VSR(H2),VSR(ZERO)
    xxswapd        VSR(Hm),VSR(H2)
    vpmsumd        Hp,H2,POLY_L
    vxor           Hl,Hl,Hp
    vxor           Hm,Hm,Hp
    xxmrghd        VSR(H2M),VSR(H2),VSR(Hl)
    xxmrgld        VSR(H2L),VSR(H2),VSR(Hm)

    C store H1M, H1L, H2M, H2L
    li             r8,1*16
    li             r9,2*16
    li             r10,3*16
    stxvd2x        VSR(H1M),0,CTX
    stxvd2x        VSR(H1L),r8,CTX
    stxvd2x        VSR(H2M),r9,CTX
    stxvd2x        VSR(H2L),r10,CTX

    C --- calculate H^3 = H^1*H^2, H^4 = H^2*H^2 ---

    vpmsumd        F,H1L,H2
    vpmsumd        F2,H2L,H2
    vpmsumd        R,H1M,H2
    vpmsumd        R2,H2M,H2

    vpmsumd        T,F,POLY_L
    vpmsumd        T2,F2,POLY_L
    xxswapd        VSR(H3),VSR(F)
    xxswapd        VSR(H4),VSR(F2)
    vxor           R,R,T
    vxor           R2,R2,T2
    vxor           H3,R,H3
    vxor           H4,R2,H4

    xxmrgld        VSR(Hl),VSR(H3),VSR(ZERO)
    xxmrgld        VSR(Hl2),VSR(H4),VSR(ZERO)
    xxswapd        VSR(Hm),VSR(H3)
    xxswapd        VSR(Hm2),VSR(H4)
    vpmsumd        Hp,H3,POLY_L
    vpmsumd        Hp2,H4,POLY_L
    vxor           Hl,Hl,Hp
    vxor           Hl2,Hl2,Hp2
    vxor           Hm,Hm,Hp
    vxor           Hm2,Hm2,Hp2
    xxmrghd        VSR(H1M),VSR(H3),VSR(Hl)
    xxmrghd        VSR(H2M),VSR(H4),VSR(Hl2)
    xxmrgld        VSR(H1L),VSR(H3),VSR(Hm)
    xxmrgld        VSR(H2L),VSR(H4),VSR(Hm2)

    C store H3M, H3L, H4M, H4L
    li             r7,4*16
    li             r8,5*16
    li             r9,6*16
    li             r10,7*16
    stxvd2x        VSR(H1M),r7,CTX
    stxvd2x        VSR(H1L),r8,CTX
    stxvd2x        VSR(H2M),r9,CTX
    stxvd2x        VSR(H2L),r10,CTX

    blr
EPILOGUE(_nettle_ghash_set_key)

.data
    C 0xC2000000000000000000000000000001
.polynomial:
.align 4
IF_BE(`
.byte 0xC2
.rept 14
.byte 0x00
.endr
.byte 0x01
',`
.byte 0x01
.rept 14
.byte 0x00
.endr
.byte 0xC2
')
