C s390x/msa_x2/aes256-decrypt.asm

ifelse(`
   Copyright (C) 2020 Mamone Tarsha
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

C KM (CIPHER MESSAGE) is specefied in "z/Architecture Principles of Operation SA22-7832-12" as follows:
C A function specified by the function code in general register 0 is performed.
C General register 1 contains the logical address of the leftmost byte of the parameter block in storage.
C The second operand is ciphered as specified by the function code using a cryptographic
C key in the parameter block, and the result is placed in the first-operand location.

C This implementation uses KM-AES-256 function.
C The parameter block used for the KM-AES-256 function has the following format:
C *----------------------------------------------*
C |         Cryptographic Key (32 bytes)         |
C *----------------------------------------------*

.file "aes256-decrypt.asm"

.text

C void
C aes256_decrypt(const struct aes256_ctx *ctx,
C                size_t length, uint8_t *dst,
C                const uint8_t *src)

PROLOGUE(nettle_aes256_decrypt)
    lghi           %r0,128|20                    C KM function code (KM-AES-256), enable modifier bit to perform decryption operation
    lgr            %r1,%r2                       C parameter block: byte offsets 0-31 Cryptographic Key
    lgr            %r2,%r5
1:  .long   0xb92e0042                           C km %r4,%r2
    brc            1,1b                          C safely branch back in case of partial completion
    br             RA
EPILOGUE(nettle_aes256_decrypt)
