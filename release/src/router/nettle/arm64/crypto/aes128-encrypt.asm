C arm64/crypto/aes128-encrypt.asm

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

.file "aes128-encrypt.asm"
.arch armv8-a+crypto

.text

C Register usage:

define(`KEYS', `x0')
define(`LENGTH', `x1')
define(`DST', `x2')
define(`SRC', `x3')

define(`S0', `v0')
define(`S1', `v1')
define(`S2', `v2')
define(`S3', `v3')
define(`K0', `v16')
define(`K1', `v17')
define(`K2', `v18')
define(`K3', `v19')
define(`K4', `v20')
define(`K5', `v21')
define(`K6', `v22')
define(`K7', `v23')
define(`K8', `v24')
define(`K9', `v25')
define(`K10', `v26')

C void
C aes128_encrypt(const struct aes128_ctx *ctx,
C                size_t length, uint8_t *dst,
C                const uint8_t *src)

PROLOGUE(nettle_aes128_encrypt)
    ld1            {K0.4s,K1.4s,K2.4s,K3.4s},[KEYS],#64
    ld1            {K4.4s,K5.4s,K6.4s,K7.4s},[KEYS],#64
    ld1            {K8.4s,K9.4s,K10.4s},[KEYS]
    
    ands           x4,LENGTH,#-64
    b.eq           L1B

L4B_loop:
    ld1            {S0.16b,S1.16b,S2.16b,S3.16b},[SRC],#64
    
    AESE_ROUND_4B(S0,S1,S2,S3,K0)
    AESE_ROUND_4B(S0,S1,S2,S3,K1)
    AESE_ROUND_4B(S0,S1,S2,S3,K2)
    AESE_ROUND_4B(S0,S1,S2,S3,K3)
    AESE_ROUND_4B(S0,S1,S2,S3,K4)
    AESE_ROUND_4B(S0,S1,S2,S3,K5)
    AESE_ROUND_4B(S0,S1,S2,S3,K6)
    AESE_ROUND_4B(S0,S1,S2,S3,K7)
    AESE_ROUND_4B(S0,S1,S2,S3,K8)
    AESE_LAST_ROUND_4B(S0,S1,S2,S3,K9,K10)

    st1            {S0.16b,S1.16b,S2.16b,S3.16b},[DST],#64

    subs           x4,x4,#64
    b.ne           L4B_loop

    and            LENGTH,LENGTH,#63

L1B:
    cbz            LENGTH,Ldone

L1B_loop:
    ld1            {S0.16b},[SRC],#16
    
    AESE_ROUND_1B(S0,K0)
    AESE_ROUND_1B(S0,K1)
    AESE_ROUND_1B(S0,K2)
    AESE_ROUND_1B(S0,K3)
    AESE_ROUND_1B(S0,K4)
    AESE_ROUND_1B(S0,K5)
    AESE_ROUND_1B(S0,K6)
    AESE_ROUND_1B(S0,K7)
    AESE_ROUND_1B(S0,K8)
    AESE_LAST_ROUND_1B(S0,K9,K10)

    st1            {S0.16b},[DST],#16

    subs           LENGTH,LENGTH,#16
    b.ne           L1B_loop

Ldone:
    ret
EPILOGUE(nettle_aes128_encrypt)
