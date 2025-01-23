/* rndjent.c  - Driver for the jitterentropy module.
 * Copyright (C) 2017 g10 Code GmbH
 * Copyright (C) 2017 Bundesamt für Sicherheit in der Informationstechnik
 * Copyright (C) 2013 Stephan Mueller <smueller@chronox.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#include <unistd.h>
#include <errno.h>
#ifndef EOPNOTSUPP
# define EOPNOTSUPP ENOSYS
#endif

#ifdef HAVE_W32_SYSTEM
# if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0501
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x0501	/* for GetNativeSystemInfo */
# endif
# include <windows.h>
#endif

#include "types.h"
#include "g10lib.h"
#include "../cipher/bithelp.h"
#include "rand-internal.h"

/*
 * Decide whether we can support jent at compile time.
 */
#undef USE_JENT
#define JENT_USES_RDTSC 1
#define JENT_USES_GETTIME 2
#define JENT_USES_READ_REAL_TIME 3
#ifdef ENABLE_JENT_SUPPORT
# if (defined (__i386__) || defined(__x86_64__)) && defined(HAVE_CPU_ARCH_X86)
#   define USE_JENT JENT_USES_RDTSC
# elif defined (HAVE_CLOCK_GETTIME)
#  if _AIX
#   define USE_JENT JENT_USES_READ_REAL_TIME
#  else
#   define USE_JENT JENT_USES_GETTIME
#  endif
# endif
#endif /*ENABLE_JENT_SUPPORT*/


#ifdef USE_JENT

#undef CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT
/* Uncomment the next line to build with statistics.  */
/* #define CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT 1 */


/* Note that we source include the actual jitter entropy code.
 * Platform dependent code is indirectly included from our own
 * jitterentropy-user-base.h file.   */

/* Tell jitterentropy* that all functions shall be static.  */
#define JENT_PRIVATE_COMPILE 1

#include "jitterentropy-base.c"
#ifdef JENT_CONF_ENABLE_INTERNAL_TIMER
#include <pthread.h>
#endif /* JENT_CONF_ENABLE_INTERNAL_TIMER */
#include "jitterentropy-gcd.c"
#include "jitterentropy-health.c"
#include "jitterentropy-noise.c"
#include "jitterentropy-sha3.c"
#include "jitterentropy-timer.c"

/* This is the lock we use to serialize access to this RNG.  The extra
 * integer variable is only used to check the locking state; that is,
 * it is not meant to be thread-safe but merely as a failsafe feature
 * to assert proper locking.  */
GPGRT_LOCK_DEFINE (jent_rng_lock);
static int jent_rng_is_locked;

/* This flag tracks whether the RNG has been initialized - either
 * with error or with success.  Protected by JENT_RNG_LOCK. */
static int jent_rng_is_initialized;

/* Our collector.  The RNG is in a working state if its value is not
 * NULL.  Protected by JENT_RNG_LOCK.  */
struct rand_data *jent_rng_collector;

/* The number of times the core entropy function has been called and
 * the number of random bytes retrieved.  */
static unsigned long jent_rng_totalcalls;
static unsigned long jent_rng_totalbytes;



/* JENT statistic helper code.  */
#ifdef CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT

static void
jent_init_statistic (struct rand_data *rand_data)
{
  /* int i; */
  /* struct entropy_stat *stat = &rand_data->entropy_stat; */

  /* for (i = 0; i < 64; i++) */
  /*   { */
  /*     stat->bitslot[i] = 0; */
  /*     stat->bitvar[i] = 0; */
  /*   } */

  /* jent_get_nstime (&stat->collection_begin); */
}

static void
jent_bit_count (struct rand_data *rand_data, u64 prev_data)
{
  /* int i; */

  /* if (!rand_data->entropy_stat.enable_bit_test) */
  /*   return; */

  /* for (i = 0; i < 64; i++) */
  /*   { */
  /*     /\* collect the count of set bits per bit position in the */
  /*      * current ->data field *\/ */
  /*     rand_data->entropy_stat.bitslot[i] += (rand_data->data & 1<<i) ? 1:0; */

  /*     /\* collect the count of bit changes between the current */
  /*      * and the previous random data value per bit position *\/ */
  /*     if ((rand_data->data & 1<<i) != (prev_data & 1<<i)) */
  /*       rand_data->entropy_stat.bitvar[i] += 1; */
  /*   } */
}


static void
jent_statistic_copy_stat (struct entropy_stat *src, struct entropy_stat *dst)
{
  /* /\* not copying bitslot and bitvar as they are not needed for */
  /*  * statistic printout *\/ */
  /* dst->collection_begin = src->collection_begin; */
  /* dst->collection_end	= src->collection_end; */
  /* dst->old_delta	= src->old_delta; */
  /* dst->setbits		= src->setbits; */
  /* dst->varbits		= src->varbits; */
  /* dst->obsbits		= src->obsbits; */
  /* dst->collection_loop_cnt= src->collection_loop_cnt; */
}


/*
 * Assessment of statistical behavior of the generated output and returning
 * the information to the caller by filling the target value.
 *
 * Details about the bit statistics are given in chapter 4 of the doc.
 * Chapter 5 documents the timer analysis and the resulting entropy.
 */
static void
jent_calc_statistic (struct rand_data *rand_data,
                     struct entropy_stat *target, unsigned int loop_cnt)
{
  /* int i; */
  /* struct entropy_stat *stat = &rand_data->entropy_stat; */

  /* jent_get_nstime(&stat->collection_end); */

  /* stat->collection_loop_cnt = loop_cnt; */

  /* stat->setbits = 0; */
  /* stat->varbits = 0; */
  /* stat->obsbits = 0; */

  /* for (i = 0; i < DATA_SIZE_BITS; i++) */
  /*   { */
  /*     stat->setbits += stat->bitslot[i]; */
  /*     stat->varbits += stat->bitvar[i]; */

  /*     /\* This is the sum of set bits in the current observation */
  /*      * of the random data. *\/ */
  /*     stat->obsbits += (rand_data->data & 1<<i) ? 1:0; */
  /*   } */

  /* jent_statistic_copy_stat(stat, target); */

  /* stat->old_delta = (stat->collection_end - stat->collection_begin); */
}

#endif /*CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT*/


/* Acquire the jent_rng_lock.  */
static void
lock_rng (void)
{
  gpg_err_code_t rc;

  rc = gpgrt_lock_lock (&jent_rng_lock);
  if (rc)
    log_fatal ("failed to acquire the Jent RNG lock: %s\n",
               gpg_strerror (rc));
  jent_rng_is_locked = 1;
}


/* Release the jent_rng_lock.  */
static void
unlock_rng (void)
{
  gpg_err_code_t rc;

  jent_rng_is_locked = 0;
  rc = gpgrt_lock_unlock (&jent_rng_lock);
  if (rc)
    log_fatal ("failed to release the Jent RNG lock: %s\n",
               gpg_strerror (rc));
}


/* Return true if the JENT RNG code can be run.  It may not yet been
 * initialized, though.  */
static int
is_rng_available (void)
{
#if USE_JENT == JENT_USES_RDTSC
  return !!(_gcry_get_hw_features () & HWF_INTEL_RDTSC);
#elif USE_JENT == JENT_USES_GETTIME
  return 2;
#elif USE_JENT == JENT_USES_READ_REAL_TIME
  return 3;
#else  /* Ooops  */
  return 0;
#endif
}

#endif /* USE_JENT */


/*
 * The API used by the high level code.
 */

/* Read up to LENGTH bytes from a jitter RNG and return the number of
 * bytes actually read.  */
size_t
_gcry_rndjent_poll (void (*add)(const void*, size_t, enum random_origins),
                    enum random_origins origin, size_t length)
{
  size_t nbytes = 0;

#ifdef USE_JENT
  if ( is_rng_available () )
    {
      lock_rng ();

      if (!jent_rng_is_initialized)
        {
          /* Auto-initialize.  */
          jent_rng_is_initialized = 1;
          jent_entropy_collector_free (jent_rng_collector);
          jent_rng_collector = NULL;
          if ( !(_gcry_random_read_conf () & RANDOM_CONF_DISABLE_JENT))
            {
              if (!jent_entropy_init ())
                jent_rng_collector = jent_entropy_collector_alloc (1, 0);
            }
        }

      if (jent_rng_collector && add)
        {
          /* We have a working JENT and it has not been disabled.  */
          char buffer[32];

          while (length)
            {
              int rc;
              size_t n = length < sizeof(buffer)? length : sizeof (buffer);

              jent_rng_totalcalls++;
              rc = jent_read_entropy_safe (&jent_rng_collector, buffer, n);
              if (rc < 0)
                break;
              /* We need to hash the output to conform to the BSI
               * NTG.1 specs.  */
              _gcry_md_hash_buffer (GCRY_MD_SHA256, buffer, buffer, rc);
              n = rc < 32? rc : 32;
              (*add) (buffer, n, origin);
              length -= n;
              nbytes += n;
              jent_rng_totalbytes += n;
            }
          wipememory (buffer, sizeof buffer);
        }

      unlock_rng ();
    }

#else

  (void)add;
  (void)origin;

#endif

  return nbytes;
}


/* Return the version number of the JENT RNG.  If the RNG is not
 * initialized or usable 0 is returned.  If R_ACTIVE is not NULL the
 * jitter RNG will be initialized and true is stored at R_ACTIVE if
 * the initialization succeeded.  */
unsigned int
_gcry_rndjent_get_version (int *r_active)
{
  if (r_active)
    *r_active = 0;
#ifdef USE_JENT
  if ( is_rng_available () )
    {
      if (r_active)
        {
          /* Make sure the RNG is initialized.  */
          _gcry_rndjent_poll (NULL, 0, 0);

          lock_rng ();
          /* To ease debugging we store 2 for a clock_gettime based
           * implementation and 1 for a rdtsc based code.  */
          *r_active = jent_rng_collector? is_rng_available () : 0;
          unlock_rng ();
        }
      return jent_version ();
    }
  else
    return 0;
#else
  return 0;
#endif
}


/* Log statistical informantion about the use of this module.  */
void
_gcry_rndjent_dump_stats (void)
{
  /* In theory we would need to lock the stats here.  However this
   * function is usually called during cleanup and then we _might_ run
   * into problems.  */

#ifdef USE_JENT
  if ( is_rng_available () )
    {
      log_info ("rndjent stat: collector=%p calls=%lu bytes=%lu\n",
                jent_rng_collector, jent_rng_totalcalls, jent_rng_totalbytes);

    }
#endif /*USE_JENT*/
}


void
_gcry_rndjent_fini (void)
{
#ifdef USE_JENT
  lock_rng ();

  if (jent_rng_is_initialized)
    {
      jent_entropy_collector_free (jent_rng_collector);
      jent_rng_collector = NULL;
    }

  unlock_rng ();
#endif
}
