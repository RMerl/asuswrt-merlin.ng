/* oaep.h

   PKCS#1 RSA-OAEP (RFC-8017).

   Copyright (C) 2021-2024 Nicolas Mora
   Copyright (C) 2024 Daiki Ueno

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

#ifndef NETTLE_OAEP_H_INCLUDED
#define NETTLE_OAEP_H_INCLUDED

#include "nettle-meta.h"
#include "bignum.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Namespace mangling */
#define _oaep_encode_mgf1 _nettle_oaep_encode_mgf1
#define _oaep_decode_mgf1 _nettle_oaep_decode_mgf1

int
_oaep_decode_mgf1 (const uint8_t *em, size_t key_size,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t *length, uint8_t *message);

int
_oaep_encode_mgf1 (mpz_t m, size_t key_size,
		   void *random_ctx, nettle_random_func *random,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t message_length, const uint8_t *message);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_OAEP_H_INCLUDED */
