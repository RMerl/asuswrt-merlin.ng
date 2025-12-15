C arm64/crypto/sha256-compress-n.asm

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

C This implementation uses the SHA-256 instructions of Armv8 crypto
C extension.
C SHA256H: SHA256 hash update (part 1)
C SHA256H2: SHA256 hash update (part 2)
C SHA256SU0: SHA256 schedule update 0
C SHA256SU1: SHA256 schedule update 1

.file "sha256-compress-n.asm"
.arch armv8-a+crypto

.text

C Register usage:

define(`STATE', `x0')
define(`K', `x1')
define(`BLOCKS', `x2')
define(`INPUT', `x3')

define(`MSG0', `v0')
define(`MSG1', `v1')
define(`MSG2', `v2')
define(`MSG3', `v3')
define(`STATE0', `v4')
define(`STATE1', `v5')
define(`CONST', `v6')
define(`TMP', `v7')
define(`STATE0_SAVED', `v16')
define(`STATE1_SAVED', `v17')

C const uint8_t *
C _nettle_sha256_compress_n(uint32_t *state, const uint32_t *k,
C                           size_t blocks, const uint8_t *input)

PROLOGUE(_nettle_sha256_compress_n)
    cbz            BLOCKS, .Lend

    C Load state
    ld1            {STATE0.4s,STATE1.4s},[STATE]

.Loop:
    C Save state
    mov            STATE0_SAVED.16b,STATE0.16b
    mov            STATE1_SAVED.16b,STATE1.16b

    C Load message
    ld1            {MSG0.16b,MSG1.16b,MSG2.16b,MSG3.16b},[INPUT],#64
    
    C Reverse for little endian
    rev32          MSG0.16b,MSG0.16b
    rev32          MSG1.16b,MSG1.16b
    rev32          MSG2.16b,MSG2.16b
    rev32          MSG3.16b,MSG3.16b

    C Rounds 0-3
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG0.4s,CONST.4s
    sha256su0      MSG0.4s,MSG1.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(STATE0_SAVED),CONST.4s
    sha256su1      MSG0.4s,MSG2.4s,MSG3.4s

    C Rounds 4-7
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG1.4s,CONST.4s
    sha256su0      MSG1.4s,MSG2.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG1.4s,MSG3.4s,MSG0.4s

    C Rounds 8-11
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG2.4s,CONST.4s
    sha256su0      MSG2.4s,MSG3.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG2.4s,MSG0.4s,MSG1.4s

    C Rounds 12-15
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG3.4s,CONST.4s
    sha256su0      MSG3.4s,MSG0.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG3.4s,MSG1.4s,MSG2.4s

    C Rounds 16-19
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG0.4s,CONST.4s
    sha256su0      MSG0.4s,MSG1.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG0.4s,MSG2.4s,MSG3.4s

    C Rounds 20-23
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG1.4s,CONST.4s
    sha256su0      MSG1.4s,MSG2.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG1.4s,MSG3.4s,MSG0.4s

    C Rounds 24-27
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG2.4s,CONST.4s
    sha256su0      MSG2.4s,MSG3.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG2.4s,MSG0.4s,MSG1.4s

    C Rounds 28-31
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG3.4s,CONST.4s
    sha256su0      MSG3.4s,MSG0.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG3.4s,MSG1.4s,MSG2.4s

    C Rounds 32-35
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG0.4s,CONST.4s
    sha256su0      MSG0.4s,MSG1.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG0.4s,MSG2.4s,MSG3.4s

    C Rounds 36-39
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG1.4s,CONST.4s
    sha256su0      MSG1.4s,MSG2.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG1.4s,MSG3.4s,MSG0.4s

    C Rounds 40-43
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG2.4s,CONST.4s
    sha256su0      MSG2.4s,MSG3.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG2.4s,MSG0.4s,MSG1.4s

    C Rounds 44-47
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG3.4s,CONST.4s
    sha256su0      MSG3.4s,MSG0.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s
    sha256su1      MSG3.4s,MSG1.4s,MSG2.4s

    C Rounds 48-51
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG0.4s,CONST.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s

    C Rounds 52-55
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG1.4s,CONST.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s

    C Rounds 56-59
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K],#16
    add            CONST.4s,MSG2.4s,CONST.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s

    C Rounds 60-63
    mov            TMP.16b,STATE0.16b
    ld1            {CONST.4s},[K]
    add            CONST.4s,MSG3.4s,CONST.4s
    sha256h        QFP(STATE0),QFP(STATE1),CONST.4s
    sha256h2       QFP(STATE1),QFP(TMP),CONST.4s

    C Combine state
    add            STATE0.4s,STATE0.4s,STATE0_SAVED.4s
    add            STATE1.4s,STATE1.4s,STATE1_SAVED.4s
    subs           BLOCKS, BLOCKS, #1
    sub            K, K, #240
    b.ne           .Loop

    C Store state
    st1            {STATE0.4s,STATE1.4s},[STATE]
.Lend:
    mov            x0, INPUT
    ret
EPILOGUE(_nettle_sha256_compress_n)
