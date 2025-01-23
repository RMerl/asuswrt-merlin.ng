/* hash-common.c - Common code for hash algorithms
 * Copyright (C) 2008 Free Software Foundation, Inc.
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
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include "g10lib.h"
#include "bufhelp.h"
#include "hash-common.h"


/* Run a selftest for hash algorithm ALGO.  If the resulting digest
   matches EXPECT/EXPECTLEN and everything else is fine as well,
   return NULL.  If an error occurs, return a static text string
   describing the error.

   DATAMODE controls what will be hashed according to this table:

     0 - Hash the supplied DATA of DATALEN.
     1 - Hash one million times a 'a'.  DATA and DATALEN are ignored.

*/
const char *
_gcry_hash_selftest_check_one (int algo,
                               int datamode, const void *data, size_t datalen,
                               const void *expect, size_t expectlen)
{
  const char *result = NULL;
  gcry_error_t err = 0;
  gcry_md_hd_t hd;
  unsigned char *digest;
  char aaa[1000];
  int xof = 0;

  if (_gcry_md_get_algo_dlen (algo) == 0)
    xof = 1;
  else if (_gcry_md_get_algo_dlen (algo) != expectlen)
    return "digest size does not match expected size";

  err = _gcry_md_open (&hd, algo, 0);
  if (err)
    return "gcry_md_open failed";

  switch (datamode)
    {
    case 0:
      _gcry_md_write (hd, data, datalen);
      break;

    case 1: /* Hash one million times an "a". */
      {
        int i;

        /* Write in odd size chunks so that we test the buffering.  */
        memset (aaa, 'a', 1000);
        for (i = 0; i < 1000; i++)
          _gcry_md_write (hd, aaa, 1000);
      }
      break;

    default:
      result = "invalid DATAMODE";
    }

  if (!result)
    {
      if (!xof)
	{
	  digest = _gcry_md_read (hd, algo);

	  if ( memcmp (digest, expect, expectlen) )
	    result = "digest mismatch";
	}
      else
	{
	  gcry_assert(expectlen <= sizeof(aaa));

	  err = _gcry_md_extract (hd, algo, aaa, expectlen);
	  if (err)
	    result = "error extracting output from XOF";
	  else if ( memcmp (aaa, expect, expectlen) )
	    result = "digest mismatch";
	}
    }

  _gcry_md_close (hd);

  return result;
}


/* Common function to write a chunk of data to the transform function
   of a hash algorithm.  Note that the use of the term "block" does
   not imply a fixed size block.  Note that we explicitly allow to use
   this function after the context has been finalized; the result does
   not have any meaning but writing after finalize is sometimes
   helpful to mitigate timing attacks. */
void
_gcry_md_block_write (void *context, const void *inbuf_arg, size_t inlen)
{
  const unsigned char *inbuf = inbuf_arg;
  gcry_md_block_ctx_t *hd = context;
  unsigned int stack_burn = 0;
  unsigned int nburn;
  const unsigned int blocksize_shift = hd->blocksize_shift;
  const unsigned int blocksize = 1 << blocksize_shift;
  size_t inblocks;
  size_t copylen;

  if (sizeof(hd->buf) < blocksize)
    BUG();

  if (!hd->bwrite)
    return;

  if (hd->count > blocksize)
    {
      /* This happens only when gcry_md_write is called after final.
       * Writing after final is used for mitigating timing attacks. */
      hd->count = 0;
    }

  while (hd->count)
    {
      if (hd->count == blocksize)  /* Flush the buffer. */
	{
	  nburn = hd->bwrite (hd, hd->buf, 1);
	  stack_burn = nburn > stack_burn ? nburn : stack_burn;
	  hd->count = 0;
	  if (!++hd->nblocks)
	    hd->nblocks_high++;
	}
      else
	{
	  copylen = inlen;
	  if (copylen > blocksize - hd->count)
	    copylen = blocksize - hd->count;

	  if (copylen == 0)
	    break;

	  buf_cpy (&hd->buf[hd->count], inbuf, copylen);
	  hd->count += copylen;
	  inbuf += copylen;
	  inlen -= copylen;
	}
    }

  if (inlen == 0)
    return;

  if (inlen >= blocksize)
    {
      inblocks = inlen >> blocksize_shift;
      nburn = hd->bwrite (hd, inbuf, inblocks);
      stack_burn = nburn > stack_burn ? nburn : stack_burn;
      hd->count = 0;
      hd->nblocks_high += (hd->nblocks + inblocks < inblocks);
      hd->nblocks += inblocks;
      inlen -= inblocks << blocksize_shift;
      inbuf += inblocks << blocksize_shift;
    }

  if (inlen)
    {
      buf_cpy (hd->buf, inbuf, inlen);
      hd->count = inlen;
    }

  if (stack_burn > 0)
    _gcry_burn_stack (stack_burn);
}
