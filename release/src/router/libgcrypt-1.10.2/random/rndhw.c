/* rndhw.c  - Access to the external random daemon
 * Copyright (C) 2007  Free Software Foundation, Inc.
 * Copyright (C) 2012  Dmitry Kasatkin
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

#include "types.h"
#include "g10lib.h"
#include "rand-internal.h"

#undef USE_PADLOCK
#ifdef ENABLE_PADLOCK_SUPPORT
# ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
#  if (defined (__i386__) && SIZEOF_UNSIGNED_LONG == 4) || defined(__x86_64__)
#   define USE_PADLOCK 1
#  endif
# endif
#endif /*ENABLE_PADLOCK_SUPPORT*/

#undef USE_DRNG
#ifdef ENABLE_DRNG_SUPPORT
# ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
#  if (defined (__i386__) && SIZEOF_UNSIGNED_LONG == 4) || defined(__x86_64__)
#   define USE_DRNG 1
#  endif
# endif
#endif /*ENABLE_RDRAND_SUPPORT*/

typedef void (*add_fn_t)(const void*, size_t, enum random_origins);

/* Keep track on whether the RNG has problems.  */
static volatile int rng_failed;


#ifdef USE_PADLOCK
static size_t
poll_padlock (void (*add)(const void*, size_t, enum random_origins),
              enum random_origins origin, int fast)
{
  volatile char buffer[64+8] __attribute__ ((aligned (8)));
  volatile char *p;
  unsigned int nbytes, status;

  /* Peter Gutmann's cryptlib tests again whether the RNG is enabled
     but we don't do so.  We would have to do this also for our AES
     implementation and that is definitely too time consuming.  There
     would be a race condition anyway.  Thus we assume that the OS
     does not change the Padlock initialization while a user process
     is running.  */
  p = buffer;
  nbytes = 0;
  while (nbytes < 64)
    {
#if defined(__x86_64__) && SIZEOF_VOID_P == 8
      asm volatile
        ("movq %1, %%rdi\n\t"         /* Set buffer.  */
         "xorq %%rdx, %%rdx\n\t"      /* Request up to 8 bytes.  */
         ".byte 0x0f, 0xa7, 0xc0\n\t" /* XSTORE RNG. */
         : "=a" (status)
         : "g" (p)
         : "%rdx", "%rdi", "cc", "memory"
         );
#else
      asm volatile
        ("movl %1, %%edi\n\t"         /* Set buffer.  */
         "xorl %%edx, %%edx\n\t"      /* Request up to 8 bytes.  */
         ".byte 0x0f, 0xa7, 0xc0\n\t" /* XSTORE RNG. */
         : "=a" (status)
         : "g" (p)
         : "%edx", "%edi", "cc", "memory"
         );
#endif
      if ((status & (1<<6))         /* RNG still enabled.  */
          && !(status & (1<<13))    /* von Neumann corrector is enabled.  */
          && !(status & (1<<14))    /* String filter is disabled.  */
          && !(status & 0x1c00)     /* BIAS voltage at default.  */
          && (!(status & 0x1f) || (status & 0x1f) == 8) /* Sanity check.  */
          )
        {
          nbytes += (status & 0x1f);
          if (fast)
            break; /* Don't get into the loop with the fast flag set.  */
          p += (status & 0x1f);
        }
      else
        {
          /* If there was an error we need to break the loop and
             record that there is something wrong with the padlock
             RNG.  */
          rng_failed = 1;
          break;
        }
    }

  if (nbytes)
    {
      (*add) ((void*)buffer, nbytes, origin);
      wipememory (buffer, nbytes);
    }
  return nbytes;
}
#endif /*USE_PADLOCK*/


#ifdef USE_DRNG
# define RDRAND_RETRY_LOOPS	10
# define RDRAND_INT	".byte 0x0f,0xc7,0xf0"
# if defined(__x86_64__) && SIZEOF_UNSIGNED_LONG == 8
#  define RDRAND_LONG	".byte 0x48,0x0f,0xc7,0xf0"
# else
#  define RDRAND_LONG	RDRAND_INT
# endif
static inline int
rdrand_long (volatile unsigned long *v)
{
  int ok;
  asm volatile ("1: " RDRAND_LONG "\n\t"
                "jc 2f\n\t"
                "decl %0\n\t"
                "jnz 1b\n\t"
                "2:"
                : "=r" (ok), "=a" (*v)
                : "0" (RDRAND_RETRY_LOOPS)
                : "cc", "memory");
  return ok;
}


static inline int
rdrand_nlong (volatile unsigned long *v, int count)
{
  while (count--)
    if (!rdrand_long(v++))
      return 0;
  return 1;
}


static size_t
poll_drng (add_fn_t add, enum random_origins origin, int fast)
{
  volatile unsigned long buffer[8] __attribute__ ((aligned (8)));
  unsigned int nbytes = sizeof (buffer);

  (void)fast;

  if (!rdrand_nlong (buffer, DIM(buffer)))
    return 0;
  (*add)((void *)buffer, nbytes, origin);
  return nbytes;
}
#endif /*USE_DRNG*/


int
_gcry_rndhw_failed_p (void)
{
  return rng_failed;
}


/* Try to read random from a hardware RNG if a fast one is
   available.  */
void
_gcry_rndhw_poll_fast (void (*add)(const void*, size_t, enum random_origins),
                       enum random_origins origin)
{
  (void)add;
  (void)origin;

#ifdef USE_DRNG
  if ((_gcry_get_hw_features () & HWF_INTEL_RDRAND))
    poll_drng (add, origin, 1);
#endif
#ifdef USE_PADLOCK
  if ((_gcry_get_hw_features () & HWF_PADLOCK_RNG))
    poll_padlock (add, origin, 1);
#endif
}


/* Read 64 bytes from a hardware RNG and return the number of bytes
   actually read.  However hardware source is let account only
   for up to 50% (or 25% for RDRAND) of the requested bytes.  */
size_t
_gcry_rndhw_poll_slow (void (*add)(const void*, size_t, enum random_origins),
                       enum random_origins origin, size_t req_length)
{
  size_t nbytes = 0;

  (void)add;
  (void)origin;

  req_length /= 2; /* Up to 50%. */

#ifdef USE_DRNG
  if ((_gcry_get_hw_features () & HWF_INTEL_RDRAND))
    {
      req_length /= 2; /* Up to 25%. */
      nbytes += poll_drng (add, origin, 0);
    }
#endif
#ifdef USE_PADLOCK
  if ((_gcry_get_hw_features () & HWF_PADLOCK_RNG))
    nbytes += poll_padlock (add, origin, 0);
#endif

  if (nbytes > req_length)
    nbytes = req_length;

  return nbytes;
}
