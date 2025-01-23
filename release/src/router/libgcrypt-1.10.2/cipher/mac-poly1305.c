/* mac-poly1305.c  -  Poly1305 based MACs
 * Copyright (C) 2014 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mac-internal.h"
#include "poly1305-internal.h"


struct poly1305mac_context_s {
  poly1305_context_t ctx;
  gcry_cipher_hd_t hd;
  struct {
    unsigned int key_set:1;
    unsigned int nonce_set:1;
    unsigned int tag:1;
  } marks;
  byte tag[POLY1305_TAGLEN];
  byte key[POLY1305_KEYLEN];
};


static gcry_err_code_t
poly1305mac_open (gcry_mac_hd_t h)
{
  struct poly1305mac_context_s *mac_ctx;
  int secure = (h->magic == CTX_MAC_MAGIC_SECURE);
  unsigned int flags = (secure ? GCRY_CIPHER_SECURE : 0);
  gcry_err_code_t err;
  int cipher_algo;

  if (secure)
    mac_ctx = xtrycalloc_secure (1, sizeof(*mac_ctx));
  else
    mac_ctx = xtrycalloc (1, sizeof(*mac_ctx));

  if (!mac_ctx)
    return gpg_err_code_from_syserror ();

  h->u.poly1305mac.ctx = mac_ctx;

  switch (h->spec->algo)
    {
    default:
      /* already checked. */
    case GCRY_MAC_POLY1305:
      /* plain Poly1305. */
      cipher_algo = -1;
      return 0;
    case GCRY_MAC_POLY1305_AES:
      cipher_algo = GCRY_CIPHER_AES;
      break;
    case GCRY_MAC_POLY1305_CAMELLIA:
      cipher_algo = GCRY_CIPHER_CAMELLIA128;
      break;
    case GCRY_MAC_POLY1305_TWOFISH:
      cipher_algo = GCRY_CIPHER_TWOFISH;
      break;
    case GCRY_MAC_POLY1305_SERPENT:
      cipher_algo = GCRY_CIPHER_SERPENT128;
      break;
    case GCRY_MAC_POLY1305_SEED:
      cipher_algo = GCRY_CIPHER_SEED;
      break;
    }

  err = _gcry_cipher_open_internal (&mac_ctx->hd, cipher_algo,
				    GCRY_CIPHER_MODE_ECB, flags);
  if (err)
    goto err_free;

  return 0;

err_free:
  xfree(h->u.poly1305mac.ctx);
  return err;
}


static void
poly1305mac_close (gcry_mac_hd_t h)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;

  if (h->spec->algo != GCRY_MAC_POLY1305)
    _gcry_cipher_close (mac_ctx->hd);

  xfree(mac_ctx);
}


static gcry_err_code_t
poly1305mac_prepare_key (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;
  size_t block_keylen = keylen - 16;

  /* Need at least 16 + 1 byte key. */
  if (keylen <= 16)
    return GPG_ERR_INV_KEYLEN;

  /* For Poly1305-AES, first part of key is passed to Poly1305 as is. */
  memcpy (mac_ctx->key, key + block_keylen, 16);

  /* Remaining part is used as key for the block cipher. */
  return _gcry_cipher_setkey (mac_ctx->hd, key, block_keylen);
}


static gcry_err_code_t
poly1305mac_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;
  gcry_err_code_t err;

  memset(&mac_ctx->ctx, 0, sizeof(mac_ctx->ctx));
  memset(&mac_ctx->tag, 0, sizeof(mac_ctx->tag));
  memset(&mac_ctx->key, 0, sizeof(mac_ctx->key));

  mac_ctx->marks.key_set = 0;
  mac_ctx->marks.nonce_set = 0;
  mac_ctx->marks.tag = 0;

  if (h->spec->algo != GCRY_MAC_POLY1305)
    {
      err = poly1305mac_prepare_key (h, key, keylen);
      if (err)
        return err;

      /* Poly1305-AES/etc also need nonce. */
      mac_ctx->marks.key_set = 1;
      mac_ctx->marks.nonce_set = 0;
    }
  else
    {
      /* For plain Poly1305, key is the nonce and setup is complete now. */

      if (keylen != POLY1305_KEYLEN)
        return GPG_ERR_INV_KEYLEN;

      memcpy (mac_ctx->key, key, keylen);

      err = _gcry_poly1305_init (&mac_ctx->ctx, mac_ctx->key, POLY1305_KEYLEN);
      if (err)
        {
          memset(&mac_ctx->key, 0, sizeof(mac_ctx->key));
          return err;
        }

      mac_ctx->marks.key_set = 1;
      mac_ctx->marks.nonce_set = 1;
    }

  return 0;
}


static gcry_err_code_t
poly1305mac_setiv (gcry_mac_hd_t h, const unsigned char *iv, size_t ivlen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;
  gcry_err_code_t err;

  if (h->spec->algo == GCRY_MAC_POLY1305)
    return GPG_ERR_INV_ARG;

  if (ivlen != 16)
    return GPG_ERR_INV_ARG;

  if (!mac_ctx->marks.key_set)
    return 0;

  memset(&mac_ctx->ctx, 0, sizeof(mac_ctx->ctx));
  memset(&mac_ctx->tag, 0, sizeof(mac_ctx->tag));
  mac_ctx->marks.nonce_set = 0;
  mac_ctx->marks.tag = 0;

  /* Prepare second part of the poly1305 key. */

  err = _gcry_cipher_encrypt (mac_ctx->hd, mac_ctx->key + 16, 16, iv, 16);
  if (err)
    return err;

  err = _gcry_poly1305_init (&mac_ctx->ctx, mac_ctx->key, POLY1305_KEYLEN);
  if (err)
    return err;

  mac_ctx->marks.nonce_set = 1;
  return 0;
}


static gcry_err_code_t
poly1305mac_reset (gcry_mac_hd_t h)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;

  if (!mac_ctx->marks.key_set || !mac_ctx->marks.nonce_set)
    return GPG_ERR_INV_STATE;

  memset(&mac_ctx->ctx, 0, sizeof(mac_ctx->ctx));
  memset(&mac_ctx->tag, 0, sizeof(mac_ctx->tag));

  mac_ctx->marks.key_set = 1;
  mac_ctx->marks.nonce_set = 1;
  mac_ctx->marks.tag = 0;

  return _gcry_poly1305_init (&mac_ctx->ctx, mac_ctx->key, POLY1305_KEYLEN);
}


static gcry_err_code_t
poly1305mac_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;

  if (!mac_ctx->marks.key_set || !mac_ctx->marks.nonce_set ||
      mac_ctx->marks.tag)
    return GPG_ERR_INV_STATE;

  _gcry_poly1305_update (&mac_ctx->ctx, buf, buflen);
  return 0;
}


static gcry_err_code_t
poly1305mac_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t *outlen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;

  if (!mac_ctx->marks.key_set || !mac_ctx->marks.nonce_set)
    return GPG_ERR_INV_STATE;

  if (!mac_ctx->marks.tag)
    {
      _gcry_poly1305_finish(&mac_ctx->ctx, mac_ctx->tag);

      memset(&mac_ctx->ctx, 0, sizeof(mac_ctx->ctx));
      mac_ctx->marks.tag = 1;
    }

  if (*outlen == 0)
    return 0;

  if (*outlen <= POLY1305_TAGLEN)
    buf_cpy (outbuf, mac_ctx->tag, *outlen);
  else
    {
      buf_cpy (outbuf, mac_ctx->tag, POLY1305_TAGLEN);
      *outlen = POLY1305_TAGLEN;
    }

  return 0;
}


static gcry_err_code_t
poly1305mac_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  struct poly1305mac_context_s *mac_ctx = h->u.poly1305mac.ctx;
  gcry_err_code_t err;
  size_t outlen = 0;

  /* Check and finalize tag. */
  err = poly1305mac_read(h, NULL, &outlen);
  if (err)
    return err;

  if (buflen > POLY1305_TAGLEN)
    return GPG_ERR_INV_LENGTH;

  return buf_eq_const (buf, mac_ctx->tag, buflen) ? 0 : GPG_ERR_CHECKSUM;
}


static unsigned int
poly1305mac_get_maclen (int algo)
{
  (void)algo;

  return POLY1305_TAGLEN;
}


static unsigned int
poly1305mac_get_keylen (int algo)
{
  (void)algo;

  return POLY1305_KEYLEN;
}


static gcry_mac_spec_ops_t poly1305mac_ops = {
  poly1305mac_open,
  poly1305mac_close,
  poly1305mac_setkey,
  poly1305mac_setiv,
  poly1305mac_reset,
  poly1305mac_write,
  poly1305mac_read,
  poly1305mac_verify,
  poly1305mac_get_maclen,
  poly1305mac_get_keylen,
  NULL,
  NULL,
};


const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac = {
  GCRY_MAC_POLY1305, {0, 0}, "POLY1305",
  &poly1305mac_ops
};
#if USE_AES
const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_aes = {
  GCRY_MAC_POLY1305_AES, {0, 0}, "POLY1305_AES",
  &poly1305mac_ops
};
#endif
#if USE_CAMELLIA
const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_camellia = {
  GCRY_MAC_POLY1305_CAMELLIA, {0, 0}, "POLY1305_CAMELLIA",
  &poly1305mac_ops
};
#endif
#if USE_TWOFISH
const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_twofish = {
  GCRY_MAC_POLY1305_TWOFISH, {0, 0}, "POLY1305_TWOFISH",
  &poly1305mac_ops
};
#endif
#if USE_SERPENT
const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_serpent = {
  GCRY_MAC_POLY1305_SERPENT, {0, 0}, "POLY1305_SERPENT",
  &poly1305mac_ops
};
#endif
#if USE_SEED
const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_seed = {
  GCRY_MAC_POLY1305_SEED, {0, 0}, "POLY1305_SEED",
  &poly1305mac_ops
};
#endif
