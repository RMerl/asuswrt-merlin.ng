/* cmac.c - CMAC, Cipher-based MAC.
 * Copyright (C) 2013,2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "cipher.h"
#include "cipher-internal.h"
#include "bufhelp.h"


#define set_burn(burn, nburn) do { \
  unsigned int __nburn = (nburn); \
  (burn) = (burn) > __nburn ? (burn) : __nburn; } while (0)


gcry_err_code_t
_gcry_cmac_write (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx,
		  const byte * inbuf, size_t inlen)
{
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t blocksize_shift = _gcry_blocksize_shift(c);
  size_t blocksize = 1 << blocksize_shift;
  byte outbuf[MAX_BLOCKSIZE];
  unsigned int burn = 0;
  unsigned int nblocks;
  size_t n;

  if (ctx->tag)
    return GPG_ERR_INV_STATE;

  if (!inbuf)
    return GPG_ERR_INV_ARG;

  if (inlen == 0)
    return 0;

  /* Last block is needed for cmac_final.  */
  if (ctx->mac_unused + inlen <= blocksize)
    {
      buf_cpy (&ctx->macbuf[ctx->mac_unused], inbuf, inlen);
      ctx->mac_unused += inlen;
      inbuf += inlen;
      inlen -= inlen;

      return 0;
    }

  if (ctx->mac_unused)
    {
      n = inlen;
      if (n > blocksize - ctx->mac_unused)
	n = blocksize - ctx->mac_unused;

      buf_cpy (&ctx->macbuf[ctx->mac_unused], inbuf, n);
      ctx->mac_unused += n;
      inbuf += n;
      inlen -= n;

      cipher_block_xor (ctx->u_iv.iv, ctx->u_iv.iv, ctx->macbuf, blocksize);
      set_burn (burn, enc_fn (&c->context.c, ctx->u_iv.iv, ctx->u_iv.iv));

      ctx->mac_unused = 0;
    }

  if (c->bulk.cbc_enc && inlen > blocksize)
    {
      nblocks = inlen >> blocksize_shift;
      nblocks -= ((nblocks << blocksize_shift) == inlen);

      c->bulk.cbc_enc (&c->context.c, ctx->u_iv.iv, outbuf, inbuf, nblocks, 1);
      inbuf += nblocks << blocksize_shift;
      inlen -= nblocks << blocksize_shift;

      wipememory (outbuf, sizeof (outbuf));
    }
  else
    while (inlen > blocksize)
      {
        cipher_block_xor (ctx->u_iv.iv, ctx->u_iv.iv, inbuf, blocksize);
        set_burn (burn, enc_fn (&c->context.c, ctx->u_iv.iv, ctx->u_iv.iv));
        inlen -= blocksize;
        inbuf += blocksize;
      }

  /* Make sure that last block is passed to cmac_final.  */
  if (inlen == 0)
    BUG ();

  n = inlen;
  if (n > blocksize - ctx->mac_unused)
    n = blocksize - ctx->mac_unused;

  buf_cpy (&ctx->macbuf[ctx->mac_unused], inbuf, n);
  ctx->mac_unused += n;
  inbuf += n;
  inlen -= n;

  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));

  return 0;
}


gcry_err_code_t
_gcry_cmac_generate_subkeys (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx)
{
  const unsigned int blocksize = c->spec->blocksize;
  byte rb, carry, t, bi;
  unsigned int burn;
  int i, j;
  union
  {
    size_t _aligned;
    byte buf[MAX_BLOCKSIZE];
  } u;

  /* Tell compiler that we require a cipher with a 64bit or 128 bit block
   * length, to allow better optimization of this function.  */
  if (blocksize > 16 || blocksize < 8 || blocksize & (8 - 1))
    return GPG_ERR_INV_CIPHER_MODE;

  if (MAX_BLOCKSIZE < blocksize)
    BUG ();

  /* encrypt zero block */
  memset (u.buf, 0, blocksize);
  burn = c->spec->encrypt (&c->context.c, u.buf, u.buf);

  /* Currently supported blocksizes are 16 and 8. */
  rb = blocksize == 16 ? 0x87 : 0x1B /* blocksize == 8 */ ;

  for (j = 0; j < 2; j++)
    {
      /* Generate subkeys K1 and K2 */
      carry = 0;
      for (i = blocksize - 1; i >= 0; i--)
        {
          bi = u.buf[i];
          t = carry | (bi << 1);
          carry = bi >> 7;
          u.buf[i] = t & 0xff;
          ctx->subkeys[j][i] = u.buf[i];
        }
      u.buf[blocksize - 1] ^= carry ? rb : 0;
      ctx->subkeys[j][blocksize - 1] = u.buf[blocksize - 1];
    }

  wipememory (&u, sizeof (u));
  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));

  return 0;
}


gcry_err_code_t
_gcry_cmac_final (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx)
{
  const unsigned int blocksize = c->spec->blocksize;
  unsigned int count = ctx->mac_unused;
  unsigned int burn;
  byte *subkey;

  /* Tell compiler that we require a cipher with a 64bit or 128 bit block
   * length, to allow better optimization of this function.  */
  if (blocksize > 16 || blocksize < 8 || blocksize & (8 - 1))
    return GPG_ERR_INV_CIPHER_MODE;

  if (count == blocksize)
    subkey = ctx->subkeys[0];        /* K1 */
  else
    {
      subkey = ctx->subkeys[1];      /* K2 */
      ctx->macbuf[count++] = 0x80;
      while (count < blocksize)
        ctx->macbuf[count++] = 0;
    }

  cipher_block_xor (ctx->macbuf, ctx->macbuf, subkey, blocksize);

  cipher_block_xor (ctx->u_iv.iv, ctx->u_iv.iv, ctx->macbuf, blocksize);
  burn = c->spec->encrypt (&c->context.c, ctx->u_iv.iv, ctx->u_iv.iv);
  if (burn)
    _gcry_burn_stack (burn + 4 * sizeof (void *));

  ctx->mac_unused = 0;

  return 0;
}


static gcry_err_code_t
cmac_tag (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx,
	  unsigned char *tag, size_t taglen, int check)
{
  gcry_err_code_t ret;

  if (!tag || taglen == 0 || taglen > c->spec->blocksize)
    return GPG_ERR_INV_ARG;

  if (!ctx->tag)
    {
      ret = _gcry_cmac_final (c, ctx);
      if (ret != 0)
	return ret;

      ctx->tag = 1;
    }

  if (!check)
    {
      memcpy (tag, ctx->u_iv.iv, taglen);
      return GPG_ERR_NO_ERROR;
    }
  else
    {
      return buf_eq_const (tag, ctx->u_iv.iv, taglen) ?
        GPG_ERR_NO_ERROR : GPG_ERR_CHECKSUM;
    }
}


void
_gcry_cmac_reset (gcry_cmac_context_t *ctx)
{
  char tmp_buf[sizeof(ctx->subkeys)];

  /* Only keep subkeys when reseting context. */

  buf_cpy (tmp_buf, ctx->subkeys, sizeof(ctx->subkeys));
  memset (ctx, 0, sizeof(*ctx));
  buf_cpy (ctx->subkeys, tmp_buf, sizeof(ctx->subkeys));
  wipememory (tmp_buf, sizeof(tmp_buf));
}


gcry_err_code_t
_gcry_cipher_cmac_authenticate (gcry_cipher_hd_t c,
                                const unsigned char *abuf, size_t abuflen)
{
  if (abuflen > 0 && !abuf)
    return GPG_ERR_INV_ARG;
  /* To support new blocksize, update cmac_generate_subkeys() then add new
     blocksize here. */
  if (c->spec->blocksize != 16 && c->spec->blocksize != 8)
    return GPG_ERR_INV_CIPHER_MODE;

  return _gcry_cmac_write (c, &c->u_mode.cmac, abuf, abuflen);
}


gcry_err_code_t
_gcry_cipher_cmac_get_tag (gcry_cipher_hd_t c,
                           unsigned char *outtag, size_t taglen)
{
  return cmac_tag (c, &c->u_mode.cmac, outtag, taglen, 0);
}


gcry_err_code_t
_gcry_cipher_cmac_check_tag (gcry_cipher_hd_t c,
                             const unsigned char *intag, size_t taglen)
{
  return cmac_tag (c, &c->u_mode.cmac, (unsigned char *) intag, taglen, 1);
}

gcry_err_code_t
_gcry_cipher_cmac_set_subkeys (gcry_cipher_hd_t c)
{
  return _gcry_cmac_generate_subkeys (c, &c->u_mode.cmac);
}
