/* pss.c

   PKCS#1 RSA-PSS padding (RFC-3447).

   Copyright (C) 2017 Daiki Ueno

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
# include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "pss.h"
#include "pss-mgf1.h"

#include "bignum.h"
#include "gmp-glue.h"

#include "memxor.h"
#include "nettle-internal.h"

/* Masks to clear the leftmost N bits.  */
static const uint8_t pss_masks[8] = {
  0xFF, 0x7F, 0x3F, 0x1F, 0xF, 0x7, 0x3, 0x1
};

static const uint8_t pss_pad[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* Format the PKCS#1 PSS padding for given salt and digest, using
 * pss_mgf1() as the mask generation function.
 *
 * The encoded messsage is stored in M, and the consistency can be
 * checked with pss_verify_mgf1(), which takes the encoded message,
 * the length of salt, and the digest.  */
int
pss_encode_mgf1(mpz_t m, size_t bits,
		const struct nettle_hash *hash,
		size_t salt_length, const uint8_t *salt,
		const uint8_t *digest)
{
  TMP_GMP_DECL(em, uint8_t);
  TMP_DECL(state, uint8_t, NETTLE_MAX_HASH_CONTEXT_SIZE);
  size_t key_size = (bits + 7) / 8;
  size_t j;

  TMP_GMP_ALLOC(em, key_size);
  TMP_ALLOC(state, hash->context_size);

  if (key_size < hash->digest_size + salt_length + 2)
    {
      TMP_GMP_FREE(em);
      return 0;
    }

  /* Compute M'.  */
  hash->init(state);
  hash->update(state, sizeof(pss_pad), pss_pad);
  hash->update(state, hash->digest_size, digest);
  hash->update(state, salt_length, salt);

  /* Store H in EM, right after maskedDB.  */
  hash->digest(state, hash->digest_size, em + key_size - hash->digest_size - 1);

  /* Compute dbMask.  */
  hash->init(state);
  hash->update(state, hash->digest_size, em + key_size - hash->digest_size - 1);

  pss_mgf1(state, hash, key_size - hash->digest_size - 1, em);

  /* Compute maskedDB and store it in front of H in EM.  */
  j = key_size - salt_length - hash->digest_size - 2;

  em[j++] ^= 1;
  memxor(em + j, salt, salt_length);
  j += salt_length;

  /* Store the trailer field following H.  */
  j += hash->digest_size;
  em[j] = 0xbc;

  /* Clear the leftmost 8 * emLen - emBits of the leftmost octet in EM.  */
  *em &= pss_masks[(8 * key_size - bits)];

  nettle_mpz_set_str_256_u(m, key_size, em);
  TMP_GMP_FREE(em);
  return 1;
}

/* Check the consistency of given PKCS#1 PSS encoded message, created
 * with pss_encode_mgf1().
 *
 * Returns 1 if the encoded message is consistent, 0 if it is
 * inconsistent.  */
int
pss_verify_mgf1(const mpz_t m, size_t bits,
		const struct nettle_hash *hash,
		size_t salt_length,
		const uint8_t *digest)
{
  TMP_GMP_DECL(em, uint8_t);
  TMP_DECL(h2, uint8_t, NETTLE_MAX_HASH_DIGEST_SIZE);
  TMP_DECL(state, uint8_t, NETTLE_MAX_HASH_CONTEXT_SIZE);
  uint8_t *h, *db, *salt;
  size_t key_size = (bits + 7) / 8;
  size_t j;
  int ret = 0;

  /* Allocate twice the key size to store the intermediate data DB
   * following the EM value.  */
  TMP_GMP_ALLOC(em, key_size * 2);

  TMP_ALLOC(h2, hash->digest_size);
  TMP_ALLOC(state, hash->context_size);

  if (key_size < hash->digest_size + salt_length + 2)
    goto cleanup;

  if (mpz_sizeinbase(m, 2) > bits)
    goto cleanup;

  nettle_mpz_get_str_256(key_size, em, m);

  /* Check the trailer field.  */
  if (em[key_size - 1] != 0xbc)
    goto cleanup;

  /* Extract H.  */
  h = em + (key_size - hash->digest_size - 1);

  /* The leftmost 8 * emLen - emBits bits of the leftmost octet of EM
   * must all equal to zero. Always true here, thanks to the above
   * check on the bit size of m. */
  assert((*em & ~pss_masks[(8 * key_size - bits)]) == 0);

  /* Compute dbMask.  */
  hash->init(state);
  hash->update(state, hash->digest_size, h);

  db = em + key_size;
  pss_mgf1(state, hash, key_size - hash->digest_size - 1, db);

  /* Compute DB.  */
  memxor(db, em, key_size - hash->digest_size - 1);

  *db &= pss_masks[(8 * key_size - bits)];
  for (j = 0; j < key_size - salt_length - hash->digest_size - 2; j++)
    if (db[j] != 0)
      goto cleanup;

  /* Check the octet right after PS is 0x1.  */
  if (db[j] != 0x1)
    goto cleanup;
  salt = db + j + 1;

  /* Compute H'.  */
  hash->init(state);
  hash->update(state, sizeof(pss_pad), pss_pad);
  hash->update(state, hash->digest_size, digest);
  hash->update(state, salt_length, salt);
  hash->digest(state, hash->digest_size, h2);

  /* Check if H' = H.  */
  if (memcmp(h2, h, hash->digest_size) != 0)
    goto cleanup;

  ret = 1;
 cleanup:
  TMP_GMP_FREE(em);
  return ret;
}
