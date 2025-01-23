/* cipher-cfb.c  - Generic CFB mode implementation
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
 *               2005, 2007, 2008, 2009, 2011 Free Software Foundation, Inc.
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
#include "cipher.h"
#include "bufhelp.h"
#include "./cipher-internal.h"


gcry_err_code_t
_gcry_cipher_cfb_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  unsigned char *ivp;
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t blocksize_shift = _gcry_blocksize_shift(c);
  size_t blocksize = 1 << blocksize_shift;
  size_t blocksize_x_2 = blocksize + blocksize;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  if ( inbuflen <= c->unused )
    {
      /* Short enough to be encoded by the remaining XOR mask. */
      /* XOR the input with the IV and store input into IV. */
      ivp = c->u_iv.iv + blocksize - c->unused;
      buf_xor_2dst(outbuf, ivp, inbuf, inbuflen);
      c->unused -= inbuflen;
      return 0;
    }

  burn = 0;

  if ( c->unused )
    {
      /* XOR the input with the IV and store input into IV */
      inbuflen -= c->unused;
      ivp = c->u_iv.iv + blocksize - c->unused;
      buf_xor_2dst(outbuf, ivp, inbuf, c->unused);
      outbuf += c->unused;
      inbuf += c->unused;
      c->unused = 0;
    }

  /* Now we can process complete blocks.  We use a loop as long as we
     have at least 2 blocks and use conditions for the rest.  This
     also allows to use a bulk encryption function if available.  */
  if (inbuflen >= blocksize_x_2 && c->bulk.cfb_enc)
    {
      size_t nblocks = inbuflen >> blocksize_shift;
      c->bulk.cfb_enc (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks);
      outbuf += nblocks << blocksize_shift;
      inbuf  += nblocks << blocksize_shift;
      inbuflen -= nblocks << blocksize_shift;
    }
  else
    {
      while ( inbuflen >= blocksize_x_2 )
        {
          /* Encrypt the IV. */
          nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
          burn = nburn > burn ? nburn : burn;
          /* XOR the input with the IV and store input into IV.  */
          cipher_block_xor_2dst(outbuf, c->u_iv.iv, inbuf, blocksize);
          outbuf += blocksize;
          inbuf += blocksize;
          inbuflen -= blocksize;
        }
    }

  if ( inbuflen >= blocksize )
    {
      /* Save the current IV and then encrypt the IV. */
      cipher_block_cpy( c->lastiv, c->u_iv.iv, blocksize );
      nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;
      /* XOR the input with the IV and store input into IV */
      cipher_block_xor_2dst(outbuf, c->u_iv.iv, inbuf, blocksize);
      outbuf += blocksize;
      inbuf += blocksize;
      inbuflen -= blocksize;
    }
  if ( inbuflen )
    {
      /* Save the current IV and then encrypt the IV. */
      cipher_block_cpy( c->lastiv, c->u_iv.iv, blocksize );
      nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;
      c->unused = blocksize;
      /* Apply the XOR. */
      c->unused -= inbuflen;
      buf_xor_2dst(outbuf, c->u_iv.iv, inbuf, inbuflen);
      outbuf += inbuflen;
      inbuf += inbuflen;
      inbuflen = 0;
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}


gcry_err_code_t
_gcry_cipher_cfb_decrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  unsigned char *ivp;
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t blocksize_shift = _gcry_blocksize_shift(c);
  size_t blocksize = 1 << blocksize_shift;
  size_t blocksize_x_2 = blocksize + blocksize;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  if (inbuflen <= c->unused)
    {
      /* Short enough to be encoded by the remaining XOR mask. */
      /* XOR the input with the IV and store input into IV. */
      ivp = c->u_iv.iv + blocksize - c->unused;
      buf_xor_n_copy(outbuf, ivp, inbuf, inbuflen);
      c->unused -= inbuflen;
      return 0;
    }

  burn = 0;

  if (c->unused)
    {
      /* XOR the input with the IV and store input into IV. */
      inbuflen -= c->unused;
      ivp = c->u_iv.iv + blocksize - c->unused;
      buf_xor_n_copy(outbuf, ivp, inbuf, c->unused);
      outbuf += c->unused;
      inbuf += c->unused;
      c->unused = 0;
    }

  /* Now we can process complete blocks.  We use a loop as long as we
     have at least 2 blocks and use conditions for the rest.  This
     also allows to use a bulk encryption function if available.  */
  if (inbuflen >= blocksize_x_2 && c->bulk.cfb_dec)
    {
      size_t nblocks = inbuflen >> blocksize_shift;
      c->bulk.cfb_dec (&c->context.c, c->u_iv.iv, outbuf, inbuf, nblocks);
      outbuf += nblocks << blocksize_shift;
      inbuf  += nblocks << blocksize_shift;
      inbuflen -= nblocks << blocksize_shift;
    }
  else
    {
      while (inbuflen >= blocksize_x_2 )
        {
          /* Encrypt the IV. */
          nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
          burn = nburn > burn ? nburn : burn;
          /* XOR the input with the IV and store input into IV. */
          cipher_block_xor_n_copy(outbuf, c->u_iv.iv, inbuf, blocksize);
          outbuf += blocksize;
          inbuf += blocksize;
          inbuflen -= blocksize;
        }
    }

  if (inbuflen >= blocksize )
    {
      /* Save the current IV and then encrypt the IV. */
      cipher_block_cpy ( c->lastiv, c->u_iv.iv, blocksize);
      nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;
      /* XOR the input with the IV and store input into IV */
      cipher_block_xor_n_copy(outbuf, c->u_iv.iv, inbuf, blocksize);
      outbuf += blocksize;
      inbuf += blocksize;
      inbuflen -= blocksize;
    }

  if (inbuflen)
    {
      /* Save the current IV and then encrypt the IV. */
      cipher_block_cpy ( c->lastiv, c->u_iv.iv, blocksize );
      nburn = enc_fn ( &c->context.c, c->u_iv.iv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;
      c->unused = blocksize;
      /* Apply the XOR. */
      c->unused -= inbuflen;
      buf_xor_n_copy(outbuf, c->u_iv.iv, inbuf, inbuflen);
      outbuf += inbuflen;
      inbuf += inbuflen;
      inbuflen = 0;
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}


gcry_err_code_t
_gcry_cipher_cfb8_encrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t blocksize = c->spec->blocksize;
  unsigned int burn, nburn;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  burn = 0;

  while ( inbuflen > 0)
    {
      int i;

      /* Encrypt the IV. */
      nburn = enc_fn ( &c->context.c, c->lastiv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;

      outbuf[0] = c->lastiv[0] ^ inbuf[0];

      /* Bitshift iv by 8 bit to the left */
      for (i = 0; i < blocksize-1; i++)
        c->u_iv.iv[i] = c->u_iv.iv[i+1];

      /* append cipher text to iv */
      c->u_iv.iv[blocksize-1] = outbuf[0];

      outbuf += 1;
      inbuf += 1;
      inbuflen -= 1;
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}


gcry_err_code_t
_gcry_cipher_cfb8_decrypt (gcry_cipher_hd_t c,
                          unsigned char *outbuf, size_t outbuflen,
                          const unsigned char *inbuf, size_t inbuflen)
{
  gcry_cipher_encrypt_t enc_fn = c->spec->encrypt;
  size_t blocksize = c->spec->blocksize;
  unsigned int burn, nburn;
  unsigned char appendee;

  if (outbuflen < inbuflen)
    return GPG_ERR_BUFFER_TOO_SHORT;

  burn = 0;

  while (inbuflen > 0)
    {
      int i;

      /* Encrypt the IV. */
      nburn = enc_fn ( &c->context.c, c->lastiv, c->u_iv.iv );
      burn = nburn > burn ? nburn : burn;

      /* inbuf might == outbuf, make sure we keep the value
         so we can append it later */
      appendee = inbuf[0];

      outbuf[0] = inbuf[0] ^ c->lastiv[0];

      /* Bitshift iv by 8 bit to the left */
      for (i = 0; i < blocksize-1; i++)
        c->u_iv.iv[i] = c->u_iv.iv[i+1];

      c->u_iv.iv[blocksize-1] = appendee;

      outbuf += 1;
      inbuf += 1;
      inbuflen -= 1;
    }

  if (burn > 0)
    _gcry_burn_stack (burn + 4 * sizeof(void *));

  return 0;
}
