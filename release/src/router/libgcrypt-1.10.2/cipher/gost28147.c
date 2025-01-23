/* gost28147.c - GOST 28147-89 implementation for Libgcrypt
 * Copyright (C) 2012 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
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

/* GOST 28147-89 defines several modes of encryption:
 * - ECB which should be used only for key transfer
 * - CFB mode
 * - OFB-like mode with additional transformation on keystream
 *   RFC 5830 names this 'counter encryption' mode
 *   Original GOST text uses the term 'gammirovanie'
 * - MAC mode ('imitovstavka')
 *
 * This implementation handles ECB and CFB modes via usual libgcrypt handling.
 * OFB-like modes are unsupported.
 */

#include <config.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "mac-internal.h"
#include "bufhelp.h"
#include "cipher-internal.h"

#include "gost.h"
#include "gost-sb.h"

static void
gost_do_set_sbox (GOST28147_context *ctx, unsigned int index)
{
  ctx->sbox = gost_oid_map[index].sbox;
  ctx->mesh_limit = gost_oid_map[index].keymeshing ? 1024 : 0;
}

static gcry_err_code_t
gost_setkey (void *c, const byte *key, unsigned keylen,
             cipher_bulk_ops_t *bulk_ops)
{
  int i;
  GOST28147_context *ctx = c;

  (void)bulk_ops;

  if (keylen != 256 / 8)
    return GPG_ERR_INV_KEYLEN;

  if (!ctx->sbox)
    gost_do_set_sbox (ctx, 0);

  for (i = 0; i < 8; i++)
    {
      ctx->key[i] = buf_get_le32(&key[4*i]);
    }

  ctx->mesh_counter = 0;

  return GPG_ERR_NO_ERROR;
}

static inline u32
gost_val (u32 subkey, u32 cm1, const u32 *sbox)
{
  cm1 += subkey;
  cm1 = sbox[0*256 + ((cm1 >>  0) & 0xff)] |
        sbox[1*256 + ((cm1 >>  8) & 0xff)] |
        sbox[2*256 + ((cm1 >> 16) & 0xff)] |
        sbox[3*256 + ((cm1 >> 24) & 0xff)];
  return cm1;
}

static unsigned int
_gost_encrypt_data (const u32 *sbox, const u32 *key, u32 *o1, u32 *o2, u32 n1, u32 n2)
{
  n2 ^= gost_val (key[0], n1, sbox); n1 ^= gost_val (key[1], n2, sbox);
  n2 ^= gost_val (key[2], n1, sbox); n1 ^= gost_val (key[3], n2, sbox);
  n2 ^= gost_val (key[4], n1, sbox); n1 ^= gost_val (key[5], n2, sbox);
  n2 ^= gost_val (key[6], n1, sbox); n1 ^= gost_val (key[7], n2, sbox);

  n2 ^= gost_val (key[0], n1, sbox); n1 ^= gost_val (key[1], n2, sbox);
  n2 ^= gost_val (key[2], n1, sbox); n1 ^= gost_val (key[3], n2, sbox);
  n2 ^= gost_val (key[4], n1, sbox); n1 ^= gost_val (key[5], n2, sbox);
  n2 ^= gost_val (key[6], n1, sbox); n1 ^= gost_val (key[7], n2, sbox);

  n2 ^= gost_val (key[0], n1, sbox); n1 ^= gost_val (key[1], n2, sbox);
  n2 ^= gost_val (key[2], n1, sbox); n1 ^= gost_val (key[3], n2, sbox);
  n2 ^= gost_val (key[4], n1, sbox); n1 ^= gost_val (key[5], n2, sbox);
  n2 ^= gost_val (key[6], n1, sbox); n1 ^= gost_val (key[7], n2, sbox);

  n2 ^= gost_val (key[7], n1, sbox); n1 ^= gost_val (key[6], n2, sbox);
  n2 ^= gost_val (key[5], n1, sbox); n1 ^= gost_val (key[4], n2, sbox);
  n2 ^= gost_val (key[3], n1, sbox); n1 ^= gost_val (key[2], n2, sbox);
  n2 ^= gost_val (key[1], n1, sbox); n1 ^= gost_val (key[0], n2, sbox);

  *o1 = n2;
  *o2 = n1;

  return /* burn_stack */ 4*sizeof(void*) /* func call */ +
                          3*sizeof(void*) /* stack */ +
                          4*sizeof(void*) /* gost_val call */;
}

static unsigned int
gost_encrypt_block (void *c, byte *outbuf, const byte *inbuf)
{
  GOST28147_context *ctx = c;
  u32 n1, n2;
  unsigned int burn;

  n1 = buf_get_le32 (inbuf);
  n2 = buf_get_le32 (inbuf+4);

  burn = _gost_encrypt_data(ctx->sbox, ctx->key, &n1, &n2, n1, n2);

  buf_put_le32 (outbuf+0, n1);
  buf_put_le32 (outbuf+4, n2);

  return /* burn_stack */ burn + 6*sizeof(void*) /* func call */;
}

unsigned int _gcry_gost_enc_data (const u32 *key,
    u32 *o1, u32 *o2, u32 n1, u32 n2, int cryptopro)
{
  const u32 *sbox;
  if (cryptopro)
    sbox = sbox_CryptoPro_3411;
  else
    sbox = sbox_test_3411;
  return _gost_encrypt_data (sbox, key, o1, o2, n1, n2) + 7 * sizeof(void *);
}

static unsigned int
gost_decrypt_block (void *c, byte *outbuf, const byte *inbuf)
{
  GOST28147_context *ctx = c;
  u32 n1, n2;
  const u32 *sbox = ctx->sbox;

  n1 = buf_get_le32 (inbuf);
  n2 = buf_get_le32 (inbuf+4);

  n2 ^= gost_val (ctx->key[0], n1, sbox); n1 ^= gost_val (ctx->key[1], n2, sbox);
  n2 ^= gost_val (ctx->key[2], n1, sbox); n1 ^= gost_val (ctx->key[3], n2, sbox);
  n2 ^= gost_val (ctx->key[4], n1, sbox); n1 ^= gost_val (ctx->key[5], n2, sbox);
  n2 ^= gost_val (ctx->key[6], n1, sbox); n1 ^= gost_val (ctx->key[7], n2, sbox);

  n2 ^= gost_val (ctx->key[7], n1, sbox); n1 ^= gost_val (ctx->key[6], n2, sbox);
  n2 ^= gost_val (ctx->key[5], n1, sbox); n1 ^= gost_val (ctx->key[4], n2, sbox);
  n2 ^= gost_val (ctx->key[3], n1, sbox); n1 ^= gost_val (ctx->key[2], n2, sbox);
  n2 ^= gost_val (ctx->key[1], n1, sbox); n1 ^= gost_val (ctx->key[0], n2, sbox);

  n2 ^= gost_val (ctx->key[7], n1, sbox); n1 ^= gost_val (ctx->key[6], n2, sbox);
  n2 ^= gost_val (ctx->key[5], n1, sbox); n1 ^= gost_val (ctx->key[4], n2, sbox);
  n2 ^= gost_val (ctx->key[3], n1, sbox); n1 ^= gost_val (ctx->key[2], n2, sbox);
  n2 ^= gost_val (ctx->key[1], n1, sbox); n1 ^= gost_val (ctx->key[0], n2, sbox);

  n2 ^= gost_val (ctx->key[7], n1, sbox); n1 ^= gost_val (ctx->key[6], n2, sbox);
  n2 ^= gost_val (ctx->key[5], n1, sbox); n1 ^= gost_val (ctx->key[4], n2, sbox);
  n2 ^= gost_val (ctx->key[3], n1, sbox); n1 ^= gost_val (ctx->key[2], n2, sbox);
  n2 ^= gost_val (ctx->key[1], n1, sbox); n1 ^= gost_val (ctx->key[0], n2, sbox);

  buf_put_le32 (outbuf+0, n2);
  buf_put_le32 (outbuf+4, n1);

  return /* burn_stack */ 4*sizeof(void*) /* func call */ +
                          3*sizeof(void*) /* stack */ +
                          4*sizeof(void*) /* gost_val call */;
}

static gpg_err_code_t
gost_set_sbox (GOST28147_context *ctx, const char *oid)
{
  int i;

  for (i = 0; gost_oid_map[i].oid; i++)
    {
      if (!strcmp(gost_oid_map[i].oid, oid))
        {
          gost_do_set_sbox (ctx, i);
          return 0;
        }
    }
  return GPG_ERR_VALUE_NOT_FOUND;
}

static gpg_err_code_t
gost_set_extra_info (void *c, int what, const void *buffer, size_t buflen)
{
  GOST28147_context *ctx = c;
  gpg_err_code_t ec = 0;

  (void)buffer;
  (void)buflen;

  switch (what)
    {
    case GCRYCTL_SET_SBOX:
      ec = gost_set_sbox (ctx, buffer);
      break;

    default:
      ec = GPG_ERR_INV_OP;
      break;
    }
  return ec;
}

static const byte CryptoProKeyMeshingKey[] = {
    0x69, 0x00, 0x72, 0x22, 0x64, 0xC9, 0x04, 0x23,
    0x8D, 0x3A, 0xDB, 0x96, 0x46, 0xE9, 0x2A, 0xC4,
    0x18, 0xFE, 0xAC, 0x94, 0x00, 0xED, 0x07, 0x12,
    0xC0, 0x86, 0xDC, 0xC2, 0xEF, 0x4C, 0xA9, 0x2B
};

/* Implements key meshing algorithm by modifing ctx and returning new IV.
   Thanks to Dmitry Belyavskiy. */
static void
cryptopro_key_meshing (GOST28147_context *ctx)
{
    unsigned char newkey[32];
    unsigned int i;

    /* "Decrypt" the static keymeshing key */
    for (i = 0; i < 4; i++)
      {
	gost_decrypt_block (ctx, newkey + i*8, CryptoProKeyMeshingKey + i*8);
      }

    /* Set new key */
    for (i = 0; i < 8; i++)
      {
	ctx->key[i] = buf_get_le32(&newkey[4*i]);
      }

    ctx->mesh_counter = 0;
}

static unsigned int
gost_encrypt_block_mesh (void *c, byte *outbuf, const byte *inbuf)
{
  GOST28147_context *ctx = c;
  u32 n1, n2;
  unsigned int burn;

  n1 = buf_get_le32 (inbuf);
  n2 = buf_get_le32 (inbuf+4);

  if (ctx->mesh_limit && (ctx->mesh_counter == ctx->mesh_limit))
    {
      cryptopro_key_meshing (ctx);
      /* Yes, encrypt twice: once for KeyMeshing procedure per RFC 4357,
       * once for block encryption */
      _gost_encrypt_data(ctx->sbox, ctx->key, &n1, &n2, n1, n2);
    }

  burn = _gost_encrypt_data(ctx->sbox, ctx->key, &n1, &n2, n1, n2);

  ctx->mesh_counter += 8;

  buf_put_le32 (outbuf+0, n1);
  buf_put_le32 (outbuf+4, n2);

  return /* burn_stack */ burn + 6*sizeof(void*) /* func call */;
}

static const gcry_cipher_oid_spec_t oids_gost28147_mesh[] =
  {
    { "1.2.643.2.2.21", GCRY_CIPHER_MODE_CFB },
    /* { "1.2.643.2.2.31.0", GCRY_CIPHER_MODE_CNTGOST }, */
    { "1.2.643.2.2.31.1", GCRY_CIPHER_MODE_CFB },
    { "1.2.643.2.2.31.2", GCRY_CIPHER_MODE_CFB },
    { "1.2.643.2.2.31.3", GCRY_CIPHER_MODE_CFB },
    { "1.2.643.2.2.31.4", GCRY_CIPHER_MODE_CFB },
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_gost28147 =
  {
    GCRY_CIPHER_GOST28147, {0, 0},
    "GOST28147", NULL, NULL, 8, 256,
    sizeof (GOST28147_context),
    gost_setkey,
    gost_encrypt_block,
    gost_decrypt_block,
    NULL, NULL, NULL, gost_set_extra_info,
  };

/* Meshing is used only for CFB, so no need to have separate
 * gost_decrypt_block_mesh.
 * Moreover key meshing is specified as encrypting the block (IV). Decrypting
 * it afterwards would be meaningless. */
gcry_cipher_spec_t _gcry_cipher_spec_gost28147_mesh =
  {
    GCRY_CIPHER_GOST28147_MESH, {0, 0},
    "GOST28147_MESH", NULL, oids_gost28147_mesh, 8, 256,
    sizeof (GOST28147_context),
    gost_setkey,
    gost_encrypt_block_mesh,
    gost_decrypt_block,
    NULL, NULL, NULL, gost_set_extra_info,
  };

static gcry_err_code_t
gost_imit_open (gcry_mac_hd_t h)
{
  memset(&h->u.imit, 0, sizeof(h->u.imit));
  return 0;
}

static void
gost_imit_close (gcry_mac_hd_t h)
{
  (void) h;
}

static gcry_err_code_t
gost_imit_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  int i;

  if (keylen != 256 / 8)
    return GPG_ERR_INV_KEYLEN;

  if (!h->u.imit.ctx.sbox)
    h->u.imit.ctx.sbox = sbox_CryptoPro_A;

  for (i = 0; i < 8; i++)
    {
      h->u.imit.ctx.key[i] = buf_get_le32(&key[4*i]);
    }

  return 0;
}

static gcry_err_code_t
gost_imit_setiv (gcry_mac_hd_t h,
		 const unsigned char *iv,
		 size_t ivlen)
{
  if (ivlen != 8)
    return GPG_ERR_INV_LENGTH;

  h->u.imit.n1 = buf_get_le32 (iv + 0);
  h->u.imit.n2 = buf_get_le32 (iv + 4);

  return 0;
}

static gcry_err_code_t
gost_imit_reset (gcry_mac_hd_t h)
{
  h->u.imit.n1 = h->u.imit.n2 = 0;
  h->u.imit.unused = 0;
  return 0;
}

static unsigned int
_gost_imit_block (const u32 *sbox, const u32 *key, u32 *o1, u32 *o2, u32 n1, u32 n2)
{
  n1 ^= *o1;
  n2 ^= *o2;

  n2 ^= gost_val (key[0], n1, sbox); n1 ^= gost_val (key[1], n2, sbox);
  n2 ^= gost_val (key[2], n1, sbox); n1 ^= gost_val (key[3], n2, sbox);
  n2 ^= gost_val (key[4], n1, sbox); n1 ^= gost_val (key[5], n2, sbox);
  n2 ^= gost_val (key[6], n1, sbox); n1 ^= gost_val (key[7], n2, sbox);

  n2 ^= gost_val (key[0], n1, sbox); n1 ^= gost_val (key[1], n2, sbox);
  n2 ^= gost_val (key[2], n1, sbox); n1 ^= gost_val (key[3], n2, sbox);
  n2 ^= gost_val (key[4], n1, sbox); n1 ^= gost_val (key[5], n2, sbox);
  n2 ^= gost_val (key[6], n1, sbox); n1 ^= gost_val (key[7], n2, sbox);

  *o1 = n1;
  *o2 = n2;

  return /* burn_stack */ 4*sizeof(void*) /* func call */ +
                          3*sizeof(void*) /* stack */ +
                          4*sizeof(void*) /* gost_val call */;
}

static inline unsigned int
gost_imit_block (GOST28147_context *ctx, u32 *n1, u32 *n2, const unsigned char *buf)
{
  if (ctx->mesh_limit && (ctx->mesh_counter == ctx->mesh_limit))
    cryptopro_key_meshing (ctx);

  return _gost_imit_block (ctx->sbox, ctx->key,
			   n1, n2,
			   buf_get_le32 (buf+0),
			   buf_get_le32 (buf+4));
}

static gcry_err_code_t
gost_imit_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  const int blocksize = 8;
  unsigned int burn = 0;
  if (!buflen || !buf)
    return GPG_ERR_NO_ERROR;

  if (h->u.imit.unused)
    {
      for (; buflen && h->u.imit.unused < blocksize; buflen --)
        h->u.imit.lastiv[h->u.imit.unused++] = *buf++;

      if (h->u.imit.unused < blocksize)
        return GPG_ERR_NO_ERROR;

      h->u.imit.count ++;
      burn = gost_imit_block (&h->u.imit.ctx,
			      &h->u.imit.n1, &h->u.imit.n2,
			      h->u.imit.lastiv);

      h->u.imit.unused = 0;
    }

  while (buflen >= blocksize)
    {
      h->u.imit.count ++;
      burn = gost_imit_block (&h->u.imit.ctx,
			      &h->u.imit.n1, &h->u.imit.n2,
			      buf);
      buf += blocksize;
      buflen -= blocksize;
    }

  for (; buflen; buflen--)
    h->u.imit.lastiv[h->u.imit.unused++] = *buf++;

  _gcry_burn_stack (burn);

  return GPG_ERR_NO_ERROR;
}

static void
gost_imit_finish (gcry_mac_hd_t h)
{
  static const unsigned char zero[8] = {0};

  /* Fill till full block */
  if (h->u.imit.unused)
    gost_imit_write(h, zero, 8 - h->u.imit.unused);

  if (h->u.imit.count == 1)
    gost_imit_write(h, zero, 8);
}

static gcry_err_code_t
gost_imit_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t * outlen)
{
  unsigned int dlen = 8;
  unsigned char digest[8];

  gost_imit_finish (h);

  buf_put_le32 (digest+0, h->u.imit.n1);
  buf_put_le32 (digest+4, h->u.imit.n2);

  if (*outlen <= dlen)
    buf_cpy (outbuf, digest, *outlen);
  else
    {
      buf_cpy (outbuf, digest, dlen);
      *outlen = dlen;
    }
  return 0;
}

static gcry_err_code_t
gost_imit_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  unsigned char tbuf[8];

  gost_imit_finish (h);

  buf_put_le32 (tbuf+0, h->u.imit.n1);
  buf_put_le32 (tbuf+4, h->u.imit.n2);

  return buf_eq_const(tbuf, buf, buflen) ?
             GPG_ERR_NO_ERROR : GPG_ERR_CHECKSUM;
}

static unsigned int
gost_imit_get_maclen (int algo)
{
  (void) algo;
  return 4; /* or 8 */
}


static unsigned int
gost_imit_get_keylen (int algo)
{
  (void) algo;
  return 256 / 8;
}

static gpg_err_code_t
gost_imit_set_extra_info (gcry_mac_hd_t hd, int what, const void *buffer, size_t buflen)
{
  gpg_err_code_t ec = 0;

  (void)buffer;
  (void)buflen;

  switch (what)
    {
    case GCRYCTL_SET_SBOX:
      ec = gost_set_sbox (&hd->u.imit.ctx, buffer);
      break;

    default:
      ec = GPG_ERR_INV_OP;
      break;
    }
  return ec;
}


static gcry_mac_spec_ops_t gost_imit_ops = {
  gost_imit_open,
  gost_imit_close,
  gost_imit_setkey,
  gost_imit_setiv,
  gost_imit_reset,
  gost_imit_write,
  gost_imit_read,
  gost_imit_verify,
  gost_imit_get_maclen,
  gost_imit_get_keylen,
  gost_imit_set_extra_info,
  NULL
};

const gcry_mac_spec_t _gcry_mac_type_spec_gost28147_imit =
  {
    GCRY_MAC_GOST28147_IMIT, {0, 0}, "GOST28147_IMIT",
    &gost_imit_ops
  };
