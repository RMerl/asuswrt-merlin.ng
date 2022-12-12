C s390x/msa_x4/ghash-update.asm

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

C KIMD (COMPUTE INTERMEDIATE MESSAGE DIGEST) is specefied in
C "z/Architecture Principles of Operation SA22-7832-12" as follows:
C A function specified by the function code in general register 0 is performed.
C General register 1 contains the logical address of the leftmost byte of the parameter block in storage.
C the second operand is processed as specified by the function code using an initial chaining value in
C the parameter block, and the result replaces the chaining value.

C This implementation uses KIMD-GHASH function.
C The parameter block used for the KIMD-GHASH function has the following format:
C *----------------------------------------------*
C |       Initial Chaining Value (16 bytes)      |
C |----------------------------------------------|
C |            Hash Subkey (16 bytes)            |
C *----------------------------------------------*

C Size of parameter block
define(`PB_SIZE', `32')

.file "ghash-update.asm"

.text

    C const uint8_t *_ghash_update (const struct gcm_key *ctx,
    C                               union nettle_block16 *x,
    C                               size_t blocks, const uint8_t *data)

PROLOGUE(_nettle_ghash_update)
    C --- allocate a stack space for parameter block in addition to 16-byte buffer to handle leftover bytes ---
    ALLOC_STACK(%r1,PB_SIZE)                     C parameter block (must be general register 1)
    mvc            0(16,%r1),0(%r3)              C copy x Initial Chaining Value field
    mvc            16(16,%r1),0(%r2)             C copy H to Hash Subkey field
    lghi           %r0,65                        C GHASH function code (must be general register 0)
    lgr            %r2,%r5                       C location of leftmost byte of data (must not be odd-numbered general register nor be general register 0)
    lgr            %r5,%r3
    C number of bytes (must be general register of data + 1). length must be a multiple of the data block size (16).
    sllg           %r3,%r4,4                     C LEN = 16*BLOCKS
1:  .long   0xb93e0002                           C kimd %r0,%r2
    brc            1,1b                          C safely branch back in case of partial completion
    mvc            0(16,%r5),0(%r1)              C store x
    xc             0(PB_SIZE,%r1),0(%r1)         C wipe parameter block content from stack
    FREE_STACK(PB_SIZE)
    br             RA
EPILOGUE(_nettle_ghash_update)
