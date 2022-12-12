C arm64/crypto/sha1-compress.asm

ifelse(`
   Copyright (C) 2021 Mamone Tarsha

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

C This implementation uses the SHA-1 instructions of Armv8 crypto
C extension.
C SHA1C: SHA1 hash update (choose)
C SHA1H: SHA1 fixed rotate
C SHA1M: SHA1 hash update (majority)
C SHA1P: SHA1 hash update (parity)
C SHA1SU0: SHA1 schedule update 0
C SHA1SU1: SHA1 schedule update 1

.file "sha1-compress.asm"
.arch armv8-a+crypto

.text

C Register usage:

define(`STATE', `x0')
define(`INPUT', `x1')

define(`CONST0', `v0')
define(`CONST1', `v1')
define(`CONST2', `v2')
define(`CONST3', `v3')
define(`MSG0', `v4')
define(`MSG1', `v5')
define(`MSG2', `v6')
define(`MSG3', `v7')
define(`ABCD', `v16')
define(`ABCD_SAVED', `v17')
define(`E0', `v18')
define(`E0_SAVED', `v19')
define(`E1', `v20')
define(`TMP', `v21')

C void nettle_sha1_compress(uint32_t *state, const uint8_t *input)

PROLOGUE(nettle_sha1_compress)
    C Initialize constants
    mov            w2,#0x7999
    movk           w2,#0x5A82,lsl #16
    dup            CONST0.4s,w2
    mov            w2,#0xEBA1
    movk           w2,#0x6ED9,lsl #16
    dup            CONST1.4s,w2
    mov            w2,#0xBCDC
    movk           w2,#0x8F1B,lsl #16
    dup            CONST2.4s,w2
    mov            w2,#0xC1D6
    movk           w2,#0xCA62,lsl #16
    dup            CONST3.4s,w2

    C Load state
    add            x2,STATE,#16
    movi           E0.4s,#0
    ld1            {ABCD.4s},[STATE]
    ld1            {E0.s}[0],[x2]

    C Save state
    mov            ABCD_SAVED.16b,ABCD.16b
    mov            E0_SAVED.16b,E0.16b

    C Load message
    ld1            {MSG0.16b,MSG1.16b,MSG2.16b,MSG3.16b},[INPUT]

    C Reverse for little endian
    rev32          MSG0.16b,MSG0.16b
    rev32          MSG1.16b,MSG1.16b
    rev32          MSG2.16b,MSG2.16b
    rev32          MSG3.16b,MSG3.16b

    C Rounds 0-3
    add            TMP.4s,MSG0.4s,CONST0.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1c          QFP(ABCD),SFP(E0),TMP.4s
    sha1su0        MSG0.4s,MSG1.4s,MSG2.4s

    C Rounds 4-7
    add            TMP.4s,MSG1.4s,CONST0.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1c          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG0.4s,MSG3.4s
    sha1su0        MSG1.4s,MSG2.4s,MSG3.4s

    C Rounds 8-11
    add            TMP.4s,MSG2.4s,CONST0.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1c          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG1.4s,MSG0.4s
    sha1su0        MSG2.4s,MSG3.4s,MSG0.4s

    C Rounds 12-15
    add            TMP.4s,MSG3.4s,CONST0.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1c          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG2.4s,MSG1.4s
    sha1su0        MSG3.4s,MSG0.4s,MSG1.4s

    C Rounds 16-19
    add            TMP.4s,MSG0.4s,CONST0.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1c          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG3.4s,MSG2.4s
    sha1su0        MSG0.4s,MSG1.4s,MSG2.4s

    C Rounds 20-23
    add            TMP.4s,MSG1.4s,CONST1.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG0.4s,MSG3.4s
    sha1su0        MSG1.4s,MSG2.4s,MSG3.4s

    C Rounds 24-27
    add            TMP.4s,MSG2.4s,CONST1.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG1.4s,MSG0.4s
    sha1su0        MSG2.4s,MSG3.4s,MSG0.4s

    C Rounds 28-31
    add            TMP.4s,MSG3.4s,CONST1.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG2.4s,MSG1.4s
    sha1su0        MSG3.4s,MSG0.4s,MSG1.4s

    C Rounds 32-35
    add            TMP.4s,MSG0.4s,CONST1.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG3.4s,MSG2.4s
    sha1su0        MSG0.4s,MSG1.4s,MSG2.4s

    C Rounds 36-39
    add            TMP.4s,MSG1.4s,CONST1.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG0.4s,MSG3.4s
    sha1su0        MSG1.4s,MSG2.4s,MSG3.4s

    C Rounds 40-43
    add            TMP.4s,MSG2.4s,CONST2.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1m          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG1.4s,MSG0.4s
    sha1su0        MSG2.4s,MSG3.4s,MSG0.4s

    C Rounds 44-47
    add            TMP.4s,MSG3.4s,CONST2.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1m          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG2.4s,MSG1.4s
    sha1su0        MSG3.4s,MSG0.4s,MSG1.4s

    C Rounds 48-51
    add            TMP.4s,MSG0.4s,CONST2.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1m          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG3.4s,MSG2.4s
    sha1su0        MSG0.4s,MSG1.4s,MSG2.4s

    C Rounds 52-55
    add            TMP.4s,MSG1.4s,CONST2.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1m          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG0.4s,MSG3.4s
    sha1su0        MSG1.4s,MSG2.4s,MSG3.4s

    C Rounds 56-59
    add            TMP.4s,MSG2.4s,CONST2.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1m          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG1.4s,MSG0.4s
    sha1su0        MSG2.4s,MSG3.4s,MSG0.4s

    C Rounds 60-63
    add            TMP.4s,MSG3.4s,CONST3.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG2.4s,MSG1.4s
    sha1su0        MSG3.4s,MSG0.4s,MSG1.4s

    C Rounds 64-67
    add            TMP.4s,MSG0.4s,CONST3.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E0),TMP.4s
    sha1su1        MSG3.4s,MSG2.4s
    sha1su0        MSG0.4s,MSG1.4s,MSG2.4s

    C Rounds 68-71
    add            TMP.4s,MSG1.4s,CONST3.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s
    sha1su1        MSG0.4s,MSG3.4s

    C Rounds 72-75
    add            TMP.4s,MSG2.4s,CONST3.4s
    sha1h          SFP(E1),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E0),TMP.4s

    C Rounds 76-79
    add            TMP.4s,MSG3.4s,CONST3.4s
    sha1h          SFP(E0),SFP(ABCD)
    sha1p          QFP(ABCD),SFP(E1),TMP.4s

    C Combine state
    add            E0.4s,E0.4s,E0_SAVED.4s
    add            ABCD.4s,ABCD.4s,ABCD_SAVED.4s

    C Store state
    st1            {ABCD.4s},[STATE]
    st1            {E0.s}[0],[x2]

    ret
EPILOGUE(nettle_sha1_compress)
