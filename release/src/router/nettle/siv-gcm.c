/* siv-gcm.c

   AES-GCM-SIV, RFC8452

   Copyright (C) 2022 Red Hat, Inc.

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

#include "siv-gcm.h"
#include "ghash-internal.h"
#include "block-internal.h"
#include "nettle-internal.h"
#include "macros.h"
#include "memops.h"
#include "ctr-internal.h"
#include <string.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

static void
siv_gcm_derive_keys (const void *ctx,
		     nettle_cipher_func *f,
		     size_t key_size,
		     size_t nlength, const uint8_t *nonce,
		     union nettle_block16 *auth_key,
		     uint8_t *encryption_key)
{
  union nettle_block16 block;
  union nettle_block16 out;
  size_t i;

  block16_zero (&block);
  memcpy (block.b + 4, nonce, MIN(nlength, SIV_GCM_NONCE_SIZE));

  f (ctx, SIV_GCM_BLOCK_SIZE, out.b, block.b);
  auth_key->u64[0] = out.u64[0];

  block.b[0] = 1;
  f (ctx, SIV_GCM_BLOCK_SIZE, out.b, block.b);
  auth_key->u64[1] = out.u64[0];

  assert (key_size % 8 == 0 && key_size / 8 + 2 <= UINT8_MAX);

  for (i = 0; i < key_size; i += 8)
    {
      block.b[0]++;
      f (ctx, SIV_GCM_BLOCK_SIZE, out.b, block.b);
      memcpy (encryption_key + i, out.b, 8);
    }
}

static nettle_fill16_func siv_gcm_fill;

static void
siv_gcm_fill(uint8_t *ctr, size_t blocks, union nettle_block16 *buffer)
{
  uint32_t c;

  c = LE_READ_UINT32(ctr);

  for (; blocks-- > 0; buffer++, c++)
    {
      memcpy(buffer->b + 4, ctr + 4, SIV_GCM_BLOCK_SIZE - 4);
      LE_WRITE_UINT32(buffer->b, c);
    }

  LE_WRITE_UINT32(ctr, c);
}

static void
siv_ghash_pad_update (struct gcm_key *ctx,
		      union nettle_block16 *state,
		      size_t length, const uint8_t *data)
{
  size_t blocks;

  blocks = length / SIV_GCM_BLOCK_SIZE;
  if (blocks > 0)
    {
      data = _siv_ghash_update (ctx, state, blocks, data);
      length &= 0xf;
    }
  if (length > 0)
    {
      uint8_t block[SIV_GCM_BLOCK_SIZE];

      memset (block + length, 0, SIV_GCM_BLOCK_SIZE - length);
      memcpy (block, data, length);
      _siv_ghash_update (ctx, state, 1, block);
    }
}

static void
siv_gcm_authenticate (const void *ctx,
		      const struct nettle_cipher *nc,
		      const union nettle_block16 *authentication_key,
		      const uint8_t *nonce,
		      size_t alength, const uint8_t *adata,
		      size_t mlength, const uint8_t *mdata,
		      uint8_t *tag)
{
  union nettle_block16 state;
  struct gcm_key siv_ghash_key;
  union nettle_block16 block;

  _siv_ghash_set_key (&siv_ghash_key, authentication_key);

  block16_zero (&state);
  siv_ghash_pad_update (&siv_ghash_key, &state, alength, adata);
  siv_ghash_pad_update (&siv_ghash_key, &state, mlength, mdata);

  block.u64[0] = bswap64_if_be (alength * 8);
  block.u64[1] = bswap64_if_be (mlength * 8);

  _siv_ghash_update (&siv_ghash_key, &state, 1, block.b);
  block16_bswap (&state, &state);

  memxor (state.b, nonce, SIV_GCM_NONCE_SIZE);
  state.b[15] &= 0x7f;
  nc->encrypt (ctx, SIV_GCM_BLOCK_SIZE, tag, state.b);
}

void
siv_gcm_encrypt_message (const struct nettle_cipher *nc,
			 const void *ctx,
			 void *ctr_ctx,
			 size_t nlength, const uint8_t *nonce,
			 size_t alength, const uint8_t *adata,
			 size_t clength, uint8_t *dst, const uint8_t *src)
{
  union nettle_block16 authentication_key;
  TMP_DECL(encryption_key, uint8_t, NETTLE_MAX_CIPHER_KEY_SIZE);
  uint8_t ctr[SIV_GCM_DIGEST_SIZE];
  uint8_t *tag = dst + clength - SIV_GCM_BLOCK_SIZE;

  assert (clength >= SIV_GCM_DIGEST_SIZE);
  assert (nlength == SIV_GCM_NONCE_SIZE);

  TMP_ALLOC(encryption_key, nc->key_size);
  siv_gcm_derive_keys (ctx, nc->encrypt, nc->key_size, nlength, nonce,
		       &authentication_key, encryption_key);

  /* Calculate authentication tag.  */
  nc->set_encrypt_key (ctr_ctx, encryption_key);

  siv_gcm_authenticate (ctr_ctx, nc,
			&authentication_key,
			nonce, alength, adata,
			clength - SIV_GCM_BLOCK_SIZE, src,
			tag);

  /* Encrypt the plaintext.  */

  /* The initial counter block is the tag with the most significant
     bit of the last byte set to one.  */
  memcpy (ctr, tag, SIV_GCM_DIGEST_SIZE);
  ctr[15] |= 0x80;
  _nettle_ctr_crypt16 (ctr_ctx, nc->encrypt, siv_gcm_fill, ctr,
		       clength - SIV_GCM_BLOCK_SIZE, dst, src);
}

int
siv_gcm_decrypt_message (const struct nettle_cipher *nc,
			 const void *ctx,
			 void *ctr_ctx,
			 size_t nlength, const uint8_t *nonce,
			 size_t alength, const uint8_t *adata,
			 size_t mlength, uint8_t *dst, const uint8_t *src)
{
  union nettle_block16 authentication_key;
  TMP_DECL(encryption_key, uint8_t, NETTLE_MAX_CIPHER_KEY_SIZE);
  union nettle_block16 state;
  uint8_t tag[SIV_GCM_DIGEST_SIZE];

  assert (nlength == SIV_GCM_NONCE_SIZE);

  TMP_ALLOC(encryption_key, nc->key_size);
  siv_gcm_derive_keys (ctx, nc->encrypt, nc->key_size, nlength, nonce,
		       &authentication_key, encryption_key);

  memcpy (state.b, src + mlength, SIV_GCM_DIGEST_SIZE);
  /* The initial counter block is the tag with the most significant
     bit of the last byte set to one.  */
  state.b[15] |= 0x80;

  /* Decrypt the ciphertext.  */
  nc->set_encrypt_key (ctr_ctx, encryption_key);

  _nettle_ctr_crypt16 (ctr_ctx, nc->encrypt, siv_gcm_fill, state.b,
		       mlength, dst, src);

  /* Calculate authentication tag.  */
  siv_gcm_authenticate (ctr_ctx, nc,
			&authentication_key,
			nonce, alength, adata,
			mlength, dst,
			tag);

  return memeql_sec (tag, src + mlength, SIV_GCM_DIGEST_SIZE);
}
