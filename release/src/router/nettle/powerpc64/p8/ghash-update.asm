C powerpc64/p8/ghash-update.asm

ifelse(`
   Copyright (C) 2020, 2020 Niels Möller and Mamone Tarsha
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

.file "ghash-update.asm"

.text

define(`CTX', `r3')
define(`X', `r4')
define(`BLOCKS', `r5')
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

    C const uint8_t *_ghash_update (const struct gcm_key *ctx,
    C                               union nettle_block16 *x,
    C                               size_t blocks, const uint8_t *data)

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_ghash_update)
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

    srdi.          r7,BLOCKS,2                   C 4-blocks loop count 'BLOCKS / 4'
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
    lxvd2x         VSR(H1M),0,CTX
    lxvd2x         VSR(H1L),r8,CTX
    lxvd2x         VSR(H2M),r9,CTX
    lxvd2x         VSR(H2L),r10,CTX
    li             r7,4*16
    li             r8,5*16
    li             r9,6*16
    li             r10,7*16
    lxvd2x         VSR(H3M),r7,CTX
    lxvd2x         VSR(H3L),r8,CTX
    lxvd2x         VSR(H4M),r9,CTX
    lxvd2x         VSR(H4L),r10,CTX

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

    clrldi         BLOCKS,BLOCKS,62              C 'set the high-order 62 bits to zeros'
L2x:
    C --- process 2 blocks ---

    srdi.          r7,BLOCKS,1                   C 'BLOCKS / 2'
    beq            L1x

    C load table elements
    li             r8,1*16
    li             r9,2*16
    li             r10,3*16
    lxvd2x         VSR(H1M),0,CTX
    lxvd2x         VSR(H1L),r8,CTX
    lxvd2x         VSR(H2M),r9,CTX
    lxvd2x         VSR(H2L),r10,CTX

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
    clrldi         BLOCKS,BLOCKS,63              C 'set the high-order 63 bits to zeros'
L1x:
    C --- process 1 block ---

    srdi.          r7,BLOCKS,0                   C 'LENGTH / 1'
    beq            Ldone

    C load table elements
    li             r8,1*16
    lxvd2x         VSR(H1M),0,CTX
    lxvd2x         VSR(H1L),r8,CTX

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
    clrldi         BLOCKS,BLOCKS,60              C 'set the high-order 60 bits to zeros'

Ldone:
    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          D,D,D,LE_MASK
')
    stxvd2x        VSR(D),0,X                    C store digest 'D'
    mr             r3, DATA

    blr
EPILOGUE(_nettle_ghash_update)

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
