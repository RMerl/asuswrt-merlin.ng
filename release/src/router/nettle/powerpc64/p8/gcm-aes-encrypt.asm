C powerpc64/p8/gcm-aes-encrypt.asm

ifelse(`
   Copyright (C) 2023- IBM Inc.
   Copyright (C) 2024 Niels Möller.

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

C Input arguments.
define(`HT', `r3')
define(`SRND', `r4')
define(`SLEN', `r5')
define(`SDST', `r6')
define(`SSRC', `r7')

define(`RK', `r8')	C Round key, also used as temporary in prologue.
C r9-r11 used as constant indices.
define(`LOOP', `r12')

C
C vectors used in aes encrypt output
C

define(`K', `v1')
define(`S0', `v2')
define(`S1', `v3')
define(`S2', `v4')
define(`S3', `v5')
define(`S4', `v6')
define(`S5', `v7')
define(`S6', `v8')
define(`S7', `v9')

C
C ghash assigned registers and vectors
C

define(`ZERO', `v21')	C Overlap R2 (only used in setup phase)
define(`POLY', `v22')	C Overlap F2 (only used in setup phase)
define(`POLY_L', `v0')

define(`D', `v10')
define(`H1M', `v11')
define(`H1L', `v12')
define(`H2M', `v13')
define(`H2L', `v14')
define(`H3M', `v15')
define(`H3L', `v16')
define(`H4M', `v17')
define(`H4L', `v18')
define(`R', `v19')
define(`F', `v20')
define(`R2', `v21')
define(`F2', `v22')

define(`LE_TEMP', `v1')	C Overlap K (only used in setup phase)
define(`LE_MASK', `v23')

define(`CNT1', `v24')
define(`LASTCNT', `v25')

.file "gcm-aes-encrypt.asm"

.text

 C size_t
 C _gcm_aes_encrypt(struct gcm_key *key, size_t rounds,
 C                  size_t len, uint8_t *dst, const uint8_t *src)
 C

define(`FUNC_ALIGN', `5')
PROLOGUE(_nettle_gcm_aes_encrypt)
    srdi. LOOP, SLEN, 7		C loop n 8 blocks
    sldi SLEN, LOOP, 7
    beq end

    li             r9,1*16
    li             r10,2*16
    li             r11,3*16

    C 288 byte "protected zone" is sufficient for storage.
    subi           RK, SP, 64
    stxvd2x VSR(v20), r11, RK
    stxvd2x VSR(v21), r10, RK
    stxvd2x VSR(v22), r9, RK
    stxvd2x VSR(v23), 0, RK
    subi           RK, SP, 96
    stxvd2x VSR(v24), r9, RK
    stxvd2x VSR(v25), 0, RK

    vxor ZERO,ZERO,ZERO
    vspltisb CNT1, 1
    vsldoi CNT1, ZERO, CNT1, 1		C counter 1

    DATA_LOAD_VEC(POLY,.polynomial,RK)

    li             RK,0
    lvsl           LE_MASK,0,RK
IF_LE(`vspltisb    LE_TEMP,0x07')
IF_BE(`vspltisb    LE_TEMP,0x03')
    vxor           LE_MASK,LE_MASK,LE_TEMP

    xxmrghd        VSR(POLY_L),VSR(ZERO),VSR(POLY)

    C load table elements
    lxvd2x         VSR(H1M),0,HT
    lxvd2x         VSR(H1L),r9,HT
    lxvd2x         VSR(H2M),r10,HT
    lxvd2x         VSR(H2L),r11,HT
    addi HT, HT, 64
    lxvd2x         VSR(H3M),0,HT
    lxvd2x         VSR(H3L),r9,HT
    lxvd2x         VSR(H4M),r10,HT
    lxvd2x         VSR(H4L),r11,HT

    addi HT, HT,  4048  C Advance to point to the 'CTR' field in the context

    lxvd2x         VSR(D),r9,HT		C load 'X' pointer
    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          D,D,D,LE_MASK
')

    lxvd2x VSR(S0), 0, HT		C Load 'CTR'
IF_LE(`vperm S0, S0, S0, LE_MASK')

    addi LOOP, LOOP, -1

    lxvd2x VSR(K),r11,HT		C First subkey
    vperm   K,K,K,LE_MASK

.align 5
    C increase ctr value as input to aes_encrypt
    vadduwm S1, S0, CNT1
    vadduwm S2, S1, CNT1
    vadduwm S3, S2, CNT1
    vadduwm S4, S3, CNT1
    vadduwm S5, S4, CNT1
    vadduwm S6, S5, CNT1
    vadduwm S7, S6, CNT1
    vmr LASTCNT, S7			C save last cnt

    OPN_XXY(vxor, K, S0, S1, S2, S3, S4, S5, S6, S7)

    addi SRND, SRND, -1
    mtctr SRND
    addi RK, HT, 64			C Point at second subkey
.align 5
L8x_round_loop1:
    lxvd2x VSR(K),0,RK
    vperm   K,K,K,LE_MASK
    OPN_XXY(vcipher, K, S0, S1, S2, S3, S4, S5, S6, S7)
    addi RK,RK,0x10
    bdnz L8x_round_loop1

    lxvd2x VSR(K),0,RK
    vperm   K,K,K,LE_MASK
    OPN_XXY(vcipherlast, K, S0, S1, S2, S3, S4, S5, S6, S7)

    cmpdi LOOP, 0
    beq do_ghash

.align 5
Loop8x_en:
    xxlor vs1, VSR(S0), VSR(S0)
    xxlor vs2, VSR(S1), VSR(S1)
    xxlor vs3, VSR(S2), VSR(S2)
    xxlor vs4, VSR(S3), VSR(S3)
    xxlor vs5, VSR(S4), VSR(S4)
    xxlor vs6, VSR(S5), VSR(S5)
    xxlor vs7, VSR(S6), VSR(S6)
    xxlor vs8, VSR(S7), VSR(S7)

    lxvd2x VSR(S0),0,SSRC
    lxvd2x VSR(S1),r9,SSRC
    lxvd2x VSR(S2),r10,SSRC
    lxvd2x VSR(S3),r11,SSRC
    addi SSRC, SSRC, 0x40
    lxvd2x VSR(S4),0,SSRC
    lxvd2x VSR(S5),r9,SSRC
    lxvd2x VSR(S6),r10,SSRC
    lxvd2x VSR(S7),r11,SSRC
    addi SSRC, SSRC, 0x40

IF_LE(`OPN_XXXY(vperm, LE_MASK, S0,S1,S2,S3)')

    xxlxor VSR(S0), VSR(S0), vs1
    xxlxor VSR(S1), VSR(S1), vs2
    xxlxor VSR(S2), VSR(S2), vs3
    xxlxor VSR(S3), VSR(S3), vs4

IF_LE(`OPN_XXXY(vperm, LE_MASK, S4,S5,S6,S7)')

    C do two 4x ghash

    C previous digest combining
    vxor D,S0,D

    GF_MUL(F2, R2, H3L, H3M, S1)
    GF_MUL(F, R, H4L, H4M, D)
    vxor           F,F,F2
    vxor           R,R,R2

    GF_MUL(F2, R2, H2L, H2M, S2)
    vxor	   F,F,F2
    vxor	   R,R,R2
    GF_MUL(F2, R2, H1L, H1M, S3)
    vxor	   F,F,F2
    vxor	   D,R,R2

    GHASH_REDUCE(D, F, POLY_L, R2, F2)  C R2, F2 used as temporaries

IF_LE(`OPN_XXXY(vperm, LE_MASK, S0,S1,S2,S3)')

    stxvd2x VSR(S0),0,SDST
    stxvd2x VSR(S1),r9,SDST
    stxvd2x VSR(S2),r10,SDST
    stxvd2x VSR(S3),r11,SDST
    addi SDST, SDST, 0x40

    xxlxor VSR(S4), VSR(S4), vs5
    xxlxor VSR(S5), VSR(S5), vs6
    xxlxor VSR(S6), VSR(S6), vs7
    xxlxor VSR(S7), VSR(S7), vs8

    C previous digest combining
    vxor D,S4,D

    GF_MUL(F2, R2, H3L, H3M, S5)
    GF_MUL(F, R, H4L, H4M, D)
    vxor           F,F,F2
    vxor           R,R,R2

    GF_MUL(F2, R2, H2L, H2M, S6)
    vxor	   F,F,F2
    vxor	   R,R,R2
    GF_MUL(F2, R2, H1L, H1M, S7)
    vxor	   F,F,F2
    vxor	   D,R,R2

    GHASH_REDUCE(D, F, POLY_L, R2, F2)  C R2, F2 used as temporaries

IF_LE(`OPN_XXXY(vperm, LE_MASK, S4,S5,S6,S7)')

    stxvd2x VSR(S4),0,SDST
    stxvd2x VSR(S5),r9,SDST
    stxvd2x VSR(S6),r10,SDST
    stxvd2x VSR(S7),r11,SDST
    addi SDST, SDST, 0x40

    lxvd2x VSR(K),r11,HT		C First subkey
    vperm   K,K,K,LE_MASK

    vadduwm S0, LASTCNT, CNT1
    vadduwm S1, S0, CNT1
    vadduwm S2, S1, CNT1
    vadduwm S3, S2, CNT1
    vadduwm S4, S3, CNT1
    vadduwm S5, S4, CNT1
    vadduwm S6, S5, CNT1
    vadduwm S7, S6, CNT1
    vmr LASTCNT, S7			C save last cnt to v29

    OPN_XXY(vxor, K, S0, S1, S2, S3, S4, S5, S6, S7)

    mtctr SRND
    addi RK, HT, 64			C Point at second subkey
.align 5
L8x_round_loop2:
    lxvd2x VSR(K),0,RK
    vperm   K,K,K,LE_MASK
    OPN_XXY(vcipher, K, S0, S1, S2, S3, S4, S5, S6, S7)
    addi RK,RK,0x10
    bdnz L8x_round_loop2

    lxvd2x VSR(K),0,RK
    vperm   K,K,K,LE_MASK
    OPN_XXY(vcipherlast, K, S0, S1, S2, S3, S4, S5, S6, S7)

    addi LOOP, LOOP, -1

    cmpdi LOOP, 0
    bne Loop8x_en

do_ghash:
    xxlor vs1, VSR(S0), VSR(S0)
    xxlor vs2, VSR(S1), VSR(S1)
    xxlor vs3, VSR(S2), VSR(S2)
    xxlor vs4, VSR(S3), VSR(S3)
    xxlor vs5, VSR(S4), VSR(S4)
    xxlor vs6, VSR(S5), VSR(S5)
    xxlor vs7, VSR(S6), VSR(S6)
    xxlor vs8, VSR(S7), VSR(S7)

    lxvd2x VSR(S0),0,SSRC
    lxvd2x VSR(S1),r9,SSRC
    lxvd2x VSR(S2),r10,SSRC
    lxvd2x VSR(S3),r11,SSRC
    addi SSRC, SSRC, 0x40
    lxvd2x VSR(S4),0,SSRC
    lxvd2x VSR(S5),r9,SSRC
    lxvd2x VSR(S6),r10,SSRC
    lxvd2x VSR(S7),r11,SSRC
    addi SSRC, SSRC, 0x40

IF_LE(`OPN_XXXY(vperm, LE_MASK, S0,S1,S2,S3)')

    xxlxor VSR(S0), VSR(S0), vs1
    xxlxor VSR(S1), VSR(S1), vs2
    xxlxor VSR(S2), VSR(S2), vs3
    xxlxor VSR(S3), VSR(S3), vs4

IF_LE(`OPN_XXXY(vperm, LE_MASK, S4,S5,S6,S7)')

    C previous digest combining
    vxor D,S0,D

    GF_MUL(F2, R2, H3L, H3M, S1)
    GF_MUL(F, R, H4L, H4M, D)
    vxor           F,F,F2
    vxor           R,R,R2

    GF_MUL(F2, R2, H2L, H2M, S2)
    vxor	   F,F,F2
    vxor	   R,R,R2
    GF_MUL(F2, R2, H1L, H1M, S3)
    vxor	   F,F,F2
    vxor	   D,R,R2

    GHASH_REDUCE(D, F, POLY_L, R2, F2)  C R2, F2 used as temporaries

IF_LE(`OPN_XXXY(vperm, LE_MASK, S0,S1,S2,S3)')

    stxvd2x VSR(S0),0,SDST
    stxvd2x VSR(S1),r9,SDST
    stxvd2x VSR(S2),r10,SDST
    stxvd2x VSR(S3),r11,SDST
    addi SDST, SDST, 0x40

    xxlxor VSR(S4), VSR(S4), vs5
    xxlxor VSR(S5), VSR(S5), vs6
    xxlxor VSR(S6), VSR(S6), vs7
    xxlxor VSR(S7), VSR(S7), vs8

    C previous digest combining
    vxor D,S4,D

    GF_MUL(F2, R2, H3L, H3M, S5)
    GF_MUL(F, R, H4L, H4M, D)
    vxor           F,F,F2
    vxor           R,R,R2

    GF_MUL(F2, R2, H2L, H2M, S6)
    vxor	   F,F,F2
    vxor	   R,R,R2
    GF_MUL(F2, R2, H1L, H1M, S7)
    vxor	   F,F,F2
    vxor	   D,R,R2

    GHASH_REDUCE(D, F, POLY_L, R2, F2)  C R2, F2 used as temporaries

IF_LE(`OPN_XXXY(vperm, LE_MASK, S4,S5,S6,S7)')

    stxvd2x VSR(S4),0,SDST
    stxvd2x VSR(S5),r9,SDST
    stxvd2x VSR(S6),r10,SDST
    stxvd2x VSR(S7),r11,SDST

gcm_aes_out:
    vadduwm LASTCNT, LASTCNT, CNT1		C increase ctr

    C byte-reverse of each doubleword permuting on little-endian mode
IF_LE(`
    vperm          D,D,D,LE_MASK
')
    stxvd2x        VSR(D),r9,HT			C store digest 'D'

IF_LE(`
    vperm LASTCNT,LASTCNT,LASTCNT,LE_MASK
')
    stxvd2x VSR(LASTCNT), 0, HT		C store ctr

    subi           RK, SP, 64
    lxvd2x VSR(v20), r11, RK
    lxvd2x VSR(v21), r10, RK
    lxvd2x VSR(v22), r9, RK
    lxvd2x VSR(v23), 0, RK
    subi           RK, SP, 96
    lxvd2x VSR(v24), r9, RK
    lxvd2x VSR(v25), 0, RK

end:
    mr r3, SLEN
    blr
EPILOGUE(_nettle_gcm_aes_encrypt)

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
