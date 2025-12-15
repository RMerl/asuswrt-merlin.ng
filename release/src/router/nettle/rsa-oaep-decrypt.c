/* rsa-oaep-decrypt.c

   The RSA publickey algorithm. OAEP decryption.

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

#include "gmp-glue.h"
#include "nettle-internal.h"
#include "oaep.h"
#include "rsa-internal.h"

int
_rsa_oaep_decrypt (const struct rsa_public_key *pub,
		   const struct rsa_private_key *key,
		   void *random_ctx, nettle_random_func *random,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t *length, uint8_t *message,
		   const uint8_t *ciphertext)
{
  TMP_GMP_DECL (m, mp_limb_t);
  TMP_GMP_DECL (em, uint8_t);
  int res;

  TMP_GMP_ALLOC (m, mpz_size (pub->n));
  TMP_GMP_ALLOC (em, key->size);

  mpn_set_base256 (m, mpz_size (pub->n), ciphertext, pub->size);

  /* Check that input is in range. */
  if (mpn_cmp (m, mpz_limbs_read (pub->n), mpz_size (pub->n)) >= 0)
    {
      TMP_GMP_FREE (em);
      TMP_GMP_FREE (m);
      return 0;
    }

  res = _rsa_sec_compute_root_tr (pub, key, random_ctx, random, m, m);

  mpn_get_base256 (em, key->size, m, mpz_size (pub->n));

  res &= _oaep_decode_mgf1 (em, key->size, hash_ctx, hash, label_length, label,
			    length, message);

  TMP_GMP_FREE (em);
  TMP_GMP_FREE (m);
  return res;
}

int
rsa_oaep_sha256_decrypt (const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t *length, uint8_t *message,
			 const uint8_t *ciphertext)
{
  struct sha256_ctx ctx;

  sha256_init (&ctx);

  return _rsa_oaep_decrypt (pub, key, random_ctx, random,
			    &ctx, &nettle_sha256, label_length, label,
			    length, message, ciphertext);
}

int
rsa_oaep_sha384_decrypt (const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t *length, uint8_t *message,
			 const uint8_t *ciphertext)
{
  struct sha384_ctx ctx;

  sha384_init (&ctx);

  return _rsa_oaep_decrypt (pub, key, random_ctx, random,
			    &ctx, &nettle_sha384, label_length, label,
			    length, message, ciphertext);
}

int
rsa_oaep_sha512_decrypt (const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t *length, uint8_t *message,
			 const uint8_t *ciphertext)
{
  struct sha512_ctx ctx;

  sha512_init (&ctx);

  return _rsa_oaep_decrypt (pub, key, random_ctx, random,
			    &ctx, &nettle_sha512, label_length, label,
			    length, message, ciphertext);
}
