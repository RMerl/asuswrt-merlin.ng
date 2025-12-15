/* oaep.c

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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "oaep.h"

#include "gmp-glue.h"
#include "memops.h"
#include "memxor.h"
#include "nettle-internal.h"
#include "pss-mgf1.h"
#include <stdlib.h>
#include <string.h>

/* Inputs are always cast to uint32_t values. But all values used in this
 * function should never exceed the maximum value of a uint32_t anyway.
 * these macros returns 1 on success, 0 on failure */
#define NOT_EQUAL(a, b) \
    ((0U - ((uint32_t)(a) ^ (uint32_t)(b))) >> 31)
#define EQUAL(a, b) (IS_ZERO_SMALL ((a) ^ (b)))
#define GREATER_OR_EQUAL(a, b) \
    (1U - (((uint32_t)(a) - (uint32_t)(b)) >> 31))

/* This is a copy of _pkcs1_sec_decrypt_variable with a slight
 * modification for the padding format.
 */
static int
_oaep_sec_decrypt_variable(size_t *length, uint8_t *message,
			   size_t padded_message_length,
			   const volatile uint8_t *padded_message,
			   volatile size_t offset)
{
  volatile int not_found = 1;
  volatile int ok = 1;
  size_t buflen, msglen;
  size_t shift, i;

  /* length is discovered in a side-channel silent way.
   * not_found goes to 0 when the terminator is found. */
  for (i = offset; i < padded_message_length; i++)
    {
      not_found &= NOT_EQUAL(padded_message[i], 1);
      offset += not_found;
    }
  /* check if we ran out of buffer */
  ok &= NOT_EQUAL(not_found, 1);

  /* skip terminator */
  offset++;

  /* offset can be up to padded_message_length, due to the loop above,
   * therefore msglen can't underflow */
  msglen = padded_message_length - offset;

  /* we always fill the whole buffer but only up to
   * padded_message_length length */
  buflen = *length;
  if (buflen > padded_message_length) { /* input independent branch */
    buflen = padded_message_length;
  }

  /* if the message length is larger than the buffer we must fail */
  ok &= GREATER_OR_EQUAL(buflen, msglen);

  /* fill destination buffer fully regardless of outcome. Copies the message
   * in a memory access independent way. The destination message buffer will
   * be clobbered past the message length. */
  shift = padded_message_length - buflen;
  cnd_memcpy(ok, message, padded_message + shift, buflen);
  offset -= shift;
  /* In this loop, the bits of the 'offset' variable are used as shifting
   * conditions, starting from the least significant bit. The end result is
   * that the buffer is shifted left exactly 'offset' bytes. */
  for (shift = 1; shift < buflen; shift <<= 1, offset >>= 1)
    {
      /* 'ok' is both a least significant bit mask and a condition */
      cnd_memcpy(offset & ok, message, message + shift, buflen - shift);
    }

  /* update length only if we succeeded, otherwise leave unchanged */
  *length = (msglen & (-(size_t) ok)) + (*length & ((size_t) ok - 1));

  return ok;
}

int
_oaep_decode_mgf1 (const uint8_t *em,
		   size_t key_size,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t *length, uint8_t *message)
{
  const uint8_t *db;
  size_t db_length;
  const uint8_t *seed;
  TMP_GMP_DECL(db_mask, uint8_t);
  uint8_t seed_mask[NETTLE_MAX_HASH_DIGEST_SIZE];
  uint8_t lhash[NETTLE_MAX_HASH_DIGEST_SIZE];
  int ok = 1;

  assert (key_size >= 2 * hash->digest_size - 2);

  /* EM = 0x00 || maskedSeed || maskedDB */
  ok &= EQUAL(*em, 0);
  seed = em + 1;
  db = seed + hash->digest_size;
  db_length = key_size - hash->digest_size - 1;

  TMP_GMP_ALLOC(db_mask, db_length);

  /* seedMask = MGF(maskedDB, hLen) */
  hash->init (hash_ctx);
  hash->update (hash_ctx, db_length, db);
  pss_mgf1 (hash_ctx, hash, hash->digest_size, seed_mask);

  /* seed = maskedSeed \xor seedMask */
  memxor (seed_mask, seed, hash->digest_size);

  /* dbMask = MGF(seed, seed - hLen - 1) */
  hash->init (hash_ctx);
  hash->update (hash_ctx, hash->digest_size, seed_mask);
  pss_mgf1 (hash_ctx, hash, db_length, db_mask);

  /* DB = maskedDB \xor dbMask */
  memxor (db_mask, db, db_length);

  hash->init (hash_ctx);
  hash->update (hash_ctx, label_length, label);
  hash->digest (hash_ctx, hash->digest_size, lhash);

  ok &= memeql_sec (db_mask, lhash, hash->digest_size);

  ok &= _oaep_sec_decrypt_variable (length, message,
				    db_length, db_mask,
				    hash->digest_size);

  TMP_GMP_FREE (db_mask);

  return ok;
}

int
_oaep_encode_mgf1 (mpz_t m, size_t key_size,
		   void *random_ctx, nettle_random_func *random,
		   void *hash_ctx, const struct nettle_hash *hash,
		   size_t label_length, const uint8_t *label,
		   size_t message_length, const uint8_t *message)
{
  TMP_GMP_DECL(em, uint8_t);
  TMP_GMP_DECL(db_mask, uint8_t);
  uint8_t *db;
  size_t db_length;
  uint8_t *seed;
  uint8_t seed_mask[NETTLE_MAX_HASH_DIGEST_SIZE];

  if (message_length > key_size
      || message_length + 2 + 2 * hash->digest_size > key_size)
    return 0;

  TMP_GMP_ALLOC(em, key_size);
  TMP_GMP_ALLOC(db_mask, key_size);

  /* EM = 0x00 || maskedSeed || maskedDB */
  *em = 0;
  seed = em + 1;
  db = seed + hash->digest_size;
  db_length = key_size - hash->digest_size - 1;

  /* DB = Hash(L) || PS || 0x01 || M */
  memset (db, 0, db_length);
  hash->init (hash_ctx);
  hash->update (hash_ctx, label_length, label);
  hash->digest (hash_ctx, hash->digest_size, db);
  memcpy (&db[db_length - message_length], message, message_length);
  db[db_length - message_length - 1] = 0x01;

  /* Generate seed */
  random (random_ctx, hash->digest_size, seed);

  /* dbMask = MGF(seed, k - hLen - 1) */
  hash->init (hash_ctx);
  hash->update (hash_ctx, hash->digest_size, seed);
  pss_mgf1 (hash_ctx, hash, db_length, db_mask);

  /* maskedDB = DB \xor dbMask */
  memxor (db, db_mask, db_length);

  /* seedMask = MGF(maskedDB, hLen) */
  hash->init (hash_ctx);
  hash->update (hash_ctx, db_length, db);
  pss_mgf1 (hash_ctx, hash, hash->digest_size, seed_mask);

  /* maskedSeed = seed \xor seedMask */
  memxor (seed, seed_mask, hash->digest_size);

  nettle_mpz_set_str_256_u (m, key_size, em);

  TMP_GMP_FREE (em);
  TMP_GMP_FREE (db_mask);

  return 1;
}
