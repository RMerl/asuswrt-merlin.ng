/* rsa-oaep-encrypt.c

   The RSA publickey algorithm. OAEP encryption.

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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "rsa.h"

#include "nettle-internal.h"
#include "oaep.h"
#include "rsa-internal.h"

int
_rsa_oaep_encrypt (const struct rsa_public_key *key,
		   void *random_ctx, nettle_random_func *random,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t length, const uint8_t *message,
		   uint8_t *ciphertext)
{
  mpz_t gibberish;

  mpz_init (gibberish);

  if (_oaep_encode_mgf1 (gibberish, key->size,
			 random_ctx, random,
			 hash_ctx, hash,
			 label_length, label,
			 length, message))
    {
      mpz_powm (gibberish, gibberish, key->e, key->n);
      nettle_mpz_get_str_256 (key->size, ciphertext, gibberish);
      mpz_clear (gibberish);
      return 1;
    }

  mpz_clear (gibberish);
  return 0;
}

int
rsa_oaep_sha256_encrypt (const struct rsa_public_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t length, const uint8_t *message,
			 uint8_t *ciphertext)
{
  struct sha256_ctx ctx;

  sha256_init (&ctx);

  return _rsa_oaep_encrypt (key,
			    random_ctx, random,
			    &ctx, &nettle_sha256,
			    label_length, label,
			    length, message,
			    ciphertext);
}

int
rsa_oaep_sha384_encrypt (const struct rsa_public_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t length, const uint8_t *message,
			 uint8_t *ciphertext)
{
  struct sha384_ctx ctx;

  sha384_init (&ctx);

  return _rsa_oaep_encrypt (key,
			    random_ctx, random,
			    &ctx, &nettle_sha384,
			    label_length, label,
			    length, message,
			    ciphertext);
}

int
rsa_oaep_sha512_encrypt (const struct rsa_public_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t length, const uint8_t *message,
			 uint8_t *ciphertext)
{
  struct sha512_ctx ctx;

  sha512_init (&ctx);

  return _rsa_oaep_encrypt (key,
			    random_ctx, random,
			    &ctx, &nettle_sha512,
			    label_length, label,
			    length, message,
			    ciphertext);
}
