/* rndgetentropy.c  -  raw random number for OSes by getentropy function.
 * Copyright (C) 1998, 2001, 2002, 2003, 2007,
 *               2009  Free Software Foundation, Inc.
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
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_SYS_RANDOM_H
#include <sys/random.h>
#endif

#include "types.h"
#include "g10lib.h"
#include "rand-internal.h"

/* The function returns 0 on success or true on failure (in which case
 * the caller will signal a fatal error).  */
int
_gcry_rndgetentropy_gather_random (void (*add)(const void*, size_t,
                                               enum random_origins),
                                   enum random_origins origin,
                                   size_t length, int level)
{
  byte buffer[256];

  if (!add)
    {
      /* Special mode to release resouces.  */
      _gcry_rndjent_fini ();
      return 0;
    }

  /* When using a blocking random generator try to get some entropy
   * from the jitter based RNG.  In this case we take up to 50% of the
   * remaining requested bytes.  */
  if (level >= GCRY_VERY_STRONG_RANDOM)
    {
      size_t n;

      n = _gcry_rndjent_poll (add, origin, length/2);
      if (n > length/2)
        n = length/2;
      if (length > 1)
        length -= n;
    }

  /* Enter the loop.  */
  while (length)
    {
      int ret;
      size_t nbytes;

      /* For a modern operating system, we use the new getentropy
       * function.  That call guarantees that the kernel's RNG has
       * been properly seeded before returning any data.  This is
       * different from /dev/urandom which may, due to its
       * non-blocking semantics, return data even if the kernel has
       * not been properly seeded.  And it differs from /dev/random by
       * never blocking once the kernel is seeded.  */
      do
        {
          _gcry_pre_syscall ();
          if (fips_mode ())
            {
              /* DRBG chaining defined in SP 800-90A (rev 1) specify
               * the upstream (kernel) DRBG needs to be reseeded for
               * initialization of downstream (libgcrypt) DRBG. For this
               * in RHEL, we repurposed the GRND_RANDOM flag of getrandom API.
               * The libgcrypt DRBG is initialized with 48B of entropy, but
               * the kernel can provide only 32B at a time after reseeding
               * so we need to limit our requests to 32B here.
               * This is clarified in IG 7.19 / IG D.K. for FIPS 140-2 / 3
               * and might not be applicable on other FIPS modules not running
               * RHEL kernel.
               */
              nbytes = length < 32 ? length : 32;
              ret = getrandom (buffer, nbytes, GRND_RANDOM);
            }
          else
            {
              nbytes = length < sizeof (buffer) ? length : sizeof (buffer);
              ret = getentropy (buffer, nbytes);
            }
          _gcry_post_syscall ();
        }
      while (ret == -1 && errno == EINTR);

      if (ret == -1 && errno == ENOSYS)
        log_fatal ("getentropy is not supported: %s\n", strerror (errno));
      else
        { /* getentropy is supported.  Some sanity checks.  */
          if (ret == -1)
            log_fatal ("unexpected error from getentropy: %s\n",
                       strerror (errno));

          (*add) (buffer, nbytes, origin);
          length -= nbytes;
        }
    }
  wipememory (buffer, sizeof buffer);

  return 0; /* success */
}
