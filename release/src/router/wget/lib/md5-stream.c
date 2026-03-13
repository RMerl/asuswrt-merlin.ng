/* Functions to compute MD5 message digest of files or memory blocks.
   according to the definition of MD5 in RFC 1321 from April 1992.
   Copyright (C) 1995-1997, 1999-2001, 2005-2006, 2008-2024 Free Software
   Foundation, Inc.
   This file is part of the GNU C Library.

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

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#include <config.h>

/* Specification.  */
#if HAVE_OPENSSL_MD5
# define GL_OPENSSL_INLINE _GL_EXTERN_INLINE
#endif
#include "md5.h"

#include <stdlib.h>

#if USE_UNLOCKED_IO
# include "unlocked-io.h"
#endif

#include "af_alg.h"

#ifdef _LIBC
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define WORDS_BIGENDIAN 1
# endif
/* We need to keep the namespace clean so define the MD5 function
   protected using leading __ .  */
# define md5_init_ctx __md5_init_ctx
# define md5_process_block __md5_process_block
# define md5_process_bytes __md5_process_bytes
# define md5_finish_ctx __md5_finish_ctx
# define md5_stream __md5_stream
#endif

#define BLOCKSIZE 32768
#if BLOCKSIZE % 64 != 0
# error "invalid BLOCKSIZE"
#endif

/* Compute MD5 message digest for bytes read from STREAM.  The
   resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
int
md5_stream (FILE *stream, void *resblock)
{
  switch (afalg_stream (stream, "md5", resblock, MD5_DIGEST_SIZE))
    {
    case 0: return 0;
    case -EIO: return 1;
    }

  char *buffer = malloc (BLOCKSIZE + 72);
  if (!buffer)
    return 1;

  struct md5_ctx ctx;
  md5_init_ctx (&ctx);
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
      md5_process_block (buffer, BLOCKSIZE, &ctx);
    }

process_partial_block:

  /* Process any remaining bytes.  */
  if (sum > 0)
    md5_process_bytes (buffer, sum, &ctx);

  /* Construct result in desired memory.  */
  md5_finish_ctx (&ctx, resblock);
  free (buffer);
  return 0;
}
