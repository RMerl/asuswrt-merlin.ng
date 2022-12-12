C s390x/msa_x2/aes256-set-encrypt-key.asm

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

.file "aes256-set-encrypt-key.asm"

.text

C void
C aes256_set_encrypt_key(struct aes256_ctx *ctx, const uint8_t *key)

PROLOGUE(nettle_aes256_set_encrypt_key)
    C AES cipher functions only need the raw cryptographic key so just copy it to AES context
    mvc            0(32,%r2),0(%r3)              C copy Cryptographic Key (32 bytes)
    br             RA
EPILOGUE(nettle_aes256_set_encrypt_key)
