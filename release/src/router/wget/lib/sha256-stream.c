/* sha256.c - Functions to compute SHA256 and SHA224 message digest of files or
   memory blocks according to the NIST specification FIPS-180-2.

   Copyright (C) 2005-2006, 2008-2024 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by David Madore, considerably copypasting from
   Scott G. Miller's sha1.c
*/

#include <config.h>

/* Specification.  */
#if HAVE_OPENSSL_SHA256
# define GL_OPENSSL_INLINE _GL_EXTERN_INLINE
#endif
#include "sha256.h"

#include <stdlib.h>

#if USE_UNLOCKED_IO
# include "unlocked-io.h"
#endif

#include "af_alg.h"

#define BLOCKSIZE 32768
#if BLOCKSIZE % 64 != 0
# error "invalid BLOCKSIZE"
#endif

/* Compute message digest for bytes read from STREAM using algorithm ALG.
   Write the message digest into RESBLOCK, which contains HASHLEN bytes.
   The initial and finishing operations are INIT_CTX and FINISH_CTX.
   Return zero if and only if successful.  */
static int
shaxxx_stream (FILE *stream, char const *alg, void *resblock,
               ssize_t hashlen, void (*init_ctx) (struct sha256_ctx *),
               void *(*finish_ctx) (struct sha256_ctx *, void *))
{
  switch (afalg_stream (stream, alg, resblock, hashlen))
    {
    case 0: return 0;
    case -EIO: return 1;
    }

  char *buffer = malloc (BLOCKSIZE + 72);
  if (!buffer)
    return 1;

  struct sha256_ctx ctx;
  init_ctx (&ctx);
  size_t sum;

  /* Iterate over full file contents.  */
  while (1)
    {
      /* We read the file in blocks of BLOCKSIZE bytes.  One call of the
         computation function processes the whole buffer so that with the
         next round of the loop another block can be read.  */
      size_t n;
      sum = 0;

      /* Read block.  Take care for partial reads.  */
      while (1)
        {
          /* Either process a partial fread() from this loop,
             or the fread() in afalg_stream may have gotten EOF.
             We need to avoid a subsequent fread() as EOF may
             not be sticky.  For details of such systems, see:
             https://sourceware.org/bugzilla/show_bug.cgi?id=1190  */
          if (feof (stream))
            goto process_partial_block;

          n = fread (buffer + sum, 1, BLOCKSIZE - sum, stream);

          sum += n;

          if (sum == BLOCKSIZE)
            break;

          if (n == 0)
            {
              /* Check for the error flag IFF N == 0, so that we don't
                 exit the loop after a partial read due to e.g., EAGAIN
                 or EWOULDBLOCK.  */
              if (ferror (stream))
                {
                  free (buffer);
                  return 1;
                }
              goto process_partial_block;
            }
        }

      /* Process buffer with BLOCKSIZE bytes.  Note that
                        BLOCKSIZE % 64 == 0
       */
      sha256_process_block (buffer, BLOCKSIZE, &ctx);
    }

 process_partial_block:;

  /* Process any remaining bytes.  */
  if (sum > 0)
    sha256_process_bytes (buffer, sum, &ctx);

  /* Construct result in desired memory.  */
  finish_ctx (&ctx, resblock);
  free (buffer);
  return 0;
}

int
sha256_stream (FILE *stream, void *resblock)
{
  return shaxxx_stream (stream, "sha256", resblock, SHA256_DIGEST_SIZE,
                        sha256_init_ctx, sha256_finish_ctx);
}

int
sha224_stream (FILE *stream, void *resblock)
{
  return shaxxx_stream (stream, "sha224", resblock, SHA224_DIGEST_SIZE,
                        sha224_init_ctx, sha224_finish_ctx);
}
