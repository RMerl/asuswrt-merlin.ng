/* non-nettle.h

   Things that are used only by the testsuite and benchmark, and
   not included in the library.

   Copyright (C) 2002, 2014 Niels Möller

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
*/

#ifndef NETTLE_NON_NETTLE_H_INCLUDED
#define NETTLE_NON_NETTLE_H_INCLUDED

#include "nettle-meta.h"
#include "ocb.h"
#include "aes.h"

/* Doesn't quite fit with the other algorithms, because of the weak
 * keys. Weak keys are not reported, the functions will simply crash
 * if you try to use a weak key. */

extern const struct nettle_cipher nettle_des;
extern const struct nettle_cipher nettle_des3;

extern const struct nettle_cipher nettle_blowfish128;

extern const struct nettle_cipher nettle_unified_aes128;
extern const struct nettle_cipher nettle_unified_aes192;
extern const struct nettle_cipher nettle_unified_aes256;

/* Stream ciphers treated as aead algorithms with no authentication. */
extern const struct nettle_aead nettle_arcfour128;
extern const struct nettle_aead nettle_chacha;
extern const struct nettle_aead nettle_salsa20;
extern const struct nettle_aead nettle_salsa20r12;

/* All-in-one CBC encrypt functinos treated as AEAD with no
   authentication and no decrypt method. */
extern const struct nettle_aead nettle_cbc_aes128;
extern const struct nettle_aead nettle_cbc_aes192;
extern const struct nettle_aead nettle_cbc_aes256;

extern const struct nettle_aead nettle_openssl_gcm_aes128;
extern const struct nettle_aead nettle_openssl_gcm_aes192;
extern const struct nettle_aead nettle_openssl_gcm_aes256;


/* Glue to openssl, for comparative benchmarking. Code in
 * examples/nettle-openssl.c. */
extern const struct nettle_cipher nettle_openssl_aes128;
extern const struct nettle_cipher nettle_openssl_aes192;
extern const struct nettle_cipher nettle_openssl_aes256;

extern const struct nettle_hash nettle_openssl_md5;
extern const struct nettle_hash nettle_openssl_sha1;

/* OCB-declarations to be moved to a public header file, once it's
   settled which nonce and tag sizes to use. */
#define OCB_NONCE_SIZE 12

struct ocb_aes128_ctx
{
  struct ocb_ctx ocb;
  struct ocb_aes128_encrypt_key key;
  struct aes128_ctx decrypt;
};

extern const struct nettle_aead nettle_ocb_aes128;


#endif /* NETTLE_NON_NETTLE_H_INCLUDED */
