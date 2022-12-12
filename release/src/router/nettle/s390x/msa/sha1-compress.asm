C s390x/msa/sha1-compress.asm

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

C KIMD (COMPUTE INTERMEDIATE MESSAGE DIGEST) is specefied in
C "z/Architecture Principles of Operation SA22-7832-12" as follows:
C A function specified by the function code in general register 0 is performed.
C General register 1 contains the logical address of the leftmost byte of the parameter block in storage.
C the second operand is processed as specified by the function code using an initial chaining value in
C the parameter block, and the result replaces the chaining value.

C This implementation uses KIMD-SHA-1 function.
C The parameter block used for the KIMD-SHA-1 function has the following format:
C *----------------------------------------------*
C |                 H0 (4 bytes)                 |
C |----------------------------------------------|
C |                 H1 (4 bytes)                 |
C |----------------------------------------------|
C |                 H2 (4 bytes)                 |
C |----------------------------------------------|
C |                 H3 (4 bytes)                 |
C |----------------------------------------------|
C |                 H4 (4 bytes)                 |
C *----------------------------------------------*

.file "sha1-compress.asm"

.text

C SHA function code
define(`SHA1_FUNCTION_CODE', `1')
C Size of block
define(`SHA1_BLOCK_SIZE', `64')

C void nettle_sha1_compress(uint32_t *state, const uint8_t *input)

PROLOGUE(nettle_sha1_compress)
    lghi           %r0,SHA1_FUNCTION_CODE        C FUNCTION_CODE
    lgr            %r1,%r2
    lgr            %r4,%r3
    lghi           %r5,SHA1_BLOCK_SIZE
1:  .long   0xb93e0004                           C kimd %r0,%r4. perform KIMD-SHA operation on data
    brc            1,1b
    br             RA
EPILOGUE(nettle_sha1_compress)
