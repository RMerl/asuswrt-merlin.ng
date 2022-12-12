C s390x/msa_x2/sha512-compress.asm

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

C This implementation uses KIMD-SHA-512 function.
C The parameter block used for the KIMD-SHA-512 function has the following format:
C *----------------------------------------------*
C |                 H0 (8 bytes)                 |
C |----------------------------------------------|
C |                 H1 (8 bytes)                 |
C |----------------------------------------------|
C |                 H2 (8 bytes)                 |
C |----------------------------------------------|
C |                 H3 (8 bytes)                 |
C |----------------------------------------------|
C |                 H4 (8 bytes)                 |
C |----------------------------------------------|
C |                 H5 (8 bytes)                 |
C |----------------------------------------------|
C |                 H6 (8 bytes)                 |
C |----------------------------------------------|
C |                 H7 (8 bytes)                 |
C *----------------------------------------------*

.file "sha512-compress.asm"

.text

C SHA function code
define(`SHA512_FUNCTION_CODE', `3')
C Size of block
define(`SHA512_BLOCK_SIZE', `128')

C void 
C _nettle_sha512_compress(uint64_t *state, const uint8_t *input,
C                         const uint64_t *k)

PROLOGUE(_nettle_sha512_compress)
    lghi           %r0,SHA512_FUNCTION_CODE      C SHA-512 Function Code
    lgr            %r1,%r2
    lgr            %r4,%r3
    lghi           %r5,SHA512_BLOCK_SIZE
1:  .long   0xb93e0004                           C kimd %r0,%r4. perform KIMD-SHA operation on data
    brc            1,1b
    br             RA
EPILOGUE(_nettle_sha512_compress)
