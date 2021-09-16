C powerpc64/p8/gcm-hash.asm

ifelse(`
   Copyright (C) 2020 Niels Möller and Mamone Tarsha
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

C gcm_set_key() assigns H value in the middle element of the table
define(`H_Idx', `128')

C Register usage:

define(`SP', `r1')
define(`TOCP', `r2')

define(`TABLE', `r3')

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

.file "gcm-hash.asm"

.text

    C void gcm_init_key (union gcm_block *table)

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
PROLOGUE(_nettle_gcm_init_key)
    DATA_LOAD_VEC(POLY,.polynomial,r7)           C 0xC2000000000000000000000000000001
IF_LE(`
    li             r8,0
    lvsl           LE_MASK,0,r8                  C 0x000102030405060708090A0B0C0D0E0F
    vspltisb       LE_TEMP,0x07                  C 0x07070707070707070707070707070707
    vxor           LE_MASK,LE_MASK,LE_TEMP       C 0x07060504030201000F0E0D0C0B0A0908
')

    C 'H' is assigned by gcm_set_key() to the middle element of the table
    li             r10,H_Idx*16
    lxvd2x         VSR(H),r10,TABLE              C load 'H'
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
    stxvd2x        VSR(H1M),0,TABLE
    stxvd2x        VSR(H1L),r8,TABLE
    stxvd2x        VSR(H2M),r9,TABLE
    stxvd2x        VSR(H2L),r10,TABLE

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
    stxvd2x        VSR(H1M),r7,TABLE
    stxvd2x        VSR(H1L),r8,TABLE
    stxvd2x        VSR(H2M),r9,TABLE
    stxvd2x        VSR(H2L),r10,TABLE

    blr
EPILOGUE(_nettle_gcm_init_key)

define(`TABLE', `r3')
define(`X', `r4')
define(`LENGTH', `r5')
define(`DATA', `r6')

define(`ZERO', `v16')
define(`POLY', `v17')
define(`POLY_L', `v0')

define(`D', `v1')
define(`C0', `v2')
define(`C1', `v3')
define(`C2', `v4')
define(`C3', `v5')
define(`H1M', `v6')
define(`H1L', `v7')
define(`H2M', `v8')
define(`H2L', `v9')
define(`H3M', `v10')
define(`H3L', `v11')
define(`H4M', `v12')
define(`H4L', `v13')
define(`R', `v14')
define(`F', `v15')
define(`R2', `v16')
define(`F2', `v17')
define(`T', `v18')
define(`R3', `v20')
define(`F3', `v21')
define(`R4', `v22')
define(`F4', `v23')

define(`LE_TEMP', `v18')
define(`LE_MASK', `v19')

    C void gcm_hash (const struct gcm_key *key, union gcm_block *x,
    C                size_t length, const uint8_t *data)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_gcm_hash)
    vxor           ZERO,ZERO,ZERO
    DATA_LOAD_VEC(POLY,.polynomial,r7)
IF_LE(`
    li             r8,0
    lvsl           LE_MASK,0,r8
    vspltisb       LE_TEMP,0x07
    vxor           LE_MASK,LE_MASK,LE_TEMP
')
    xxmrghd        VSR(POLY_L),VSR(ZERO),VSR(POLY)

    lxvd2x         VSR(D),0,X                    C load 'X' pointer
    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          D,D,D,LE_MASK
')

    C --- process 4 blocks '128-bit each' per one loop ---

    srdi.          r7,LENGTH,6                   C 4-blocks loop count 'LENGTH / (4 * 16)'
    beq            L2x

    mtctr          r7                            C assign counter register to loop count

    C store non-volatile vector registers
    addi           r8,SP,-64
    stvx           v20,0,r8
    addi           r8,r8,16
    stvx           v21,0,r8
    addi           r8,r8,16
    stvx           v22,0,r8
    addi           r8,r8,16
    stvx           v23,0,r8

    C load table elements
    li             r8,1*16
    li             r9,2*16
    li             r10,3*16
    lxvd2x         VSR(H1M),0,TABLE
    lxvd2x         VSR(H1L),r8,TABLE
    lxvd2x         VSR(H2M),r9,TABLE
    lxvd2x         VSR(H2L),r10,TABLE
    li             r7,4*16
    li             r8,5*16
    li             r9,6*16
    li             r10,7*16
    lxvd2x         VSR(H3M),r7,TABLE
    lxvd2x         VSR(H3L),r8,TABLE
    lxvd2x         VSR(H4M),r9,TABLE
    lxvd2x         VSR(H4L),r10,TABLE

    li             r8,0x10
    li             r9,0x20
    li             r10,0x30
.align 5
L4x_loop:
    C input loading
    lxvd2x         VSR(C0),0,DATA                C load C0
    lxvd2x         VSR(C1),r8,DATA               C load C1
    lxvd2x         VSR(C2),r9,DATA               C load C2
    lxvd2x         VSR(C3),r10,DATA              C load C3

IF_LE(`
    vperm          C0,C0,C0,LE_MASK
    vperm          C1,C1,C1,LE_MASK
    vperm          C2,C2,C2,LE_MASK
    vperm          C3,C3,C3,LE_MASK
')

    C previous digest combining
    vxor           C0,C0,D

    C polynomial multiplication
    vpmsumd        F2,H3L,C1
    vpmsumd        R2,H3M,C1
    vpmsumd        F3,H2L,C2
    vpmsumd        R3,H2M,C2
    vpmsumd        F4,H1L,C3
    vpmsumd        R4,H1M,C3
    vpmsumd        F,H4L,C0
    vpmsumd        R,H4M,C0

    C deferred recombination of partial products
    vxor           F3,F3,F4
    vxor           R3,R3,R4
    vxor           F,F,F2
    vxor           R,R,R2
    vxor           F,F,F3
    vxor           R,R,R3

    C reduction
    vpmsumd        T,F,POLY_L
    xxswapd        VSR(D),VSR(F)
    vxor           R,R,T
    vxor           D,R,D

    addi           DATA,DATA,0x40
    bdnz           L4x_loop

    C restore non-volatile vector registers
    addi           r8,SP,-64
    lvx            v20,0,r8
    addi           r8,r8,16
    lvx            v21,0,r8
    addi           r8,r8,16
    lvx            v22,0,r8
    addi           r8,r8,16
    lvx            v23,0,r8

    clrldi         LENGTH,LENGTH,58              C 'set the high-order 58 bits to zeros'
L2x:
    C --- process 2 blocks ---

    srdi.          r7,LENGTH,5                   C 'LENGTH / (2 * 16)'
    beq            L1x

    C load table elements
    li             r8,1*16
    li             r9,2*16
    li             r10,3*16
    lxvd2x         VSR(H1M),0,TABLE
    lxvd2x         VSR(H1L),r8,TABLE
    lxvd2x         VSR(H2M),r9,TABLE
    lxvd2x         VSR(H2L),r10,TABLE

    C input loading
    li             r10,0x10
    lxvd2x         VSR(C0),0,DATA                C load C0
    lxvd2x         VSR(C1),r10,DATA              C load C1

IF_LE(`
    vperm          C0,C0,C0,LE_MASK
    vperm          C1,C1,C1,LE_MASK
')

    C previous digest combining
    vxor           C0,C0,D

    C polynomial multiplication
    vpmsumd        F2,H1L,C1
    vpmsumd        R2,H1M,C1
    vpmsumd        F,H2L,C0
    vpmsumd        R,H2M,C0

    C deferred recombination of partial products
    vxor           F,F,F2
    vxor           R,R,R2

    C reduction
    vpmsumd        T,F,POLY_L
    xxswapd        VSR(D),VSR(F)
    vxor           R,R,T
    vxor           D,R,D

    addi           DATA,DATA,0x20
    clrldi         LENGTH,LENGTH,59              C 'set the high-order 59 bits to zeros'
L1x:
    C --- process 1 block ---

    srdi.          r7,LENGTH,4                   C 'LENGTH / (1 * 16)'
    beq            Lmod

    C load table elements
    li             r8,1*16
    lxvd2x         VSR(H1M),0,TABLE
    lxvd2x         VSR(H1L),r8,TABLE

    C input loading
    lxvd2x         VSR(C0),0,DATA                C load C0

IF_LE(`
    vperm          C0,C0,C0,LE_MASK
')

    C previous digest combining
    vxor           C0,C0,D

    C polynomial multiplication
    vpmsumd        F,H1L,C0
    vpmsumd        R,H1M,C0

    C reduction
    vpmsumd        T,F,POLY_L
    xxswapd        VSR(D),VSR(F)
    vxor           R,R,T
    vxor           D,R,D

    addi           DATA,DATA,0x10
    clrldi         LENGTH,LENGTH,60              C 'set the high-order 60 bits to zeros'
Lmod:
    C --- process the modulo bytes, padding the low-order bytes with zeros ---

    cmpldi         LENGTH,0
    beq            Ldone

    C load table elements
    li             r8,1*16
    lxvd2x         VSR(H1M),0,TABLE
    lxvd2x         VSR(H1L),r8,TABLE

    C push every modulo byte to the stack and load them with padding into vector register
    vxor           ZERO,ZERO,ZERO
    addi           r8,SP,-16
    stvx           ZERO,0,r8
Lstb_loop:
    subic.         LENGTH,LENGTH,1
    lbzx           r7,LENGTH,DATA
    stbx           r7,LENGTH,r8
    bne            Lstb_loop
    lxvd2x         VSR(C0),0,r8

IF_LE(`
    vperm          C0,C0,C0,LE_MASK
')

    C previous digest combining
    vxor           C0,C0,D

    C polynomial multiplication
    vpmsumd        F,H1L,C0
    vpmsumd        R,H1M,C0

    C reduction
    vpmsumd        T,F,POLY_L
    xxswapd        VSR(D),VSR(F)
    vxor           R,R,T
    vxor           D,R,D

Ldone:
    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          D,D,D,LE_MASK
')
    stxvd2x        VSR(D),0,X                    C store digest 'D'

    blr
EPILOGUE(_nettle_gcm_hash)

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
