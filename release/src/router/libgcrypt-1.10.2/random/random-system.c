/* random-system.c - wrapper around the system's RNG
 * Copyright (C) 2012  Free Software Foundation, Inc.
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

/*
   This RNG is merely wrapper around the system's native RNG.  For
   example on Unix systems it directly uses /dev/{u,}random.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#include "g10lib.h"
#include "random.h"
#include "rand-internal.h"

/* This is the lock we use to serialize access to this RNG.  The extra
   integer variable is only used to check the locking state; that is,
   it is not meant to be thread-safe but merely as a failsafe feature
   to assert proper locking.  */
GPGRT_LOCK_DEFINE (system_rng_lock);
static int system_rng_is_locked;


/* --- Local prototypes ---  */




/* --- Functions  --- */

/* Basic initialization is required to initialize mutexes and
   do a few checks on the implementation.  */
static void
basic_initialization (void)
{
  static int initialized;

  if (initialized)
    return;
  initialized = 1;

  system_rng_is_locked = 0;

  /* Make sure that we are still using the values we traditionally
     used for the random levels.  */
  gcry_assert (GCRY_WEAK_RANDOM == 0
               && GCRY_STRONG_RANDOM == 1
               && GCRY_VERY_STRONG_RANDOM == 2);

}


/* Acquire the system_rng_lock.  */
static void
lock_rng (void)
{
  gpg_err_code_t rc;

  rc = gpgrt_lock_lock (&system_rng_lock);
  if (rc)
    log_fatal ("failed to acquire the System RNG lock: %s\n",
               gpg_strerror (rc));
  system_rng_is_locked = 1;
}


/* Release the system_rng_lock.  */
static void
unlock_rng (void)
{
  gpg_err_code_t rc;

  system_rng_is_locked = 0;
  rc = gpgrt_lock_unlock (&system_rng_lock);
  if (rc)
    log_fatal ("failed to release the System RNG lock: %s\n",
               gpg_strerror (rc));
}


/* Helper variables for read_cb().

   The _gcry_rnd*_gather_random interface does not allow to provide a
   data pointer.  Thus we need to use a global variable for
   communication.  However, the then required locking is anyway a good
   idea because it does not make sense to have several readers of (say
   /dev/random).  It is easier to serve them one after the other.  */
static unsigned char *read_cb_buffer;   /* The buffer.  */
static size_t         read_cb_size;     /* Size of the buffer.  */
static size_t         read_cb_len;      /* Used length.  */


/* Callback for _gcry_rnd*_gather_random.  */
static void
read_cb (const void *buffer, size_t length, enum random_origins origin)
{
  const unsigned char *p = buffer;

  (void)origin;

  gcry_assert (system_rng_is_locked);
  gcry_assert (read_cb_buffer);

  /* Note that we need to protect against gatherers returning more
     than the requested bytes (e.g. rndw32).  */
  while (length-- && read_cb_len < read_cb_size)
    {
      read_cb_buffer[read_cb_len++] = *p++;
    }
}


/* Fill BUFFER with LENGTH bytes of random at quality LEVEL.  The
   function either succeeds or terminates the process in case of a
   fatal error. */
static void
get_random (void *buffer, size_t length, int level)
{
  int rc;

  gcry_assert (buffer);

  read_cb_buffer = buffer;
  read_cb_size   = length;
  read_cb_len    = 0;

#if USE_RNDGETENTROPY
  rc = _gcry_rndgetentropy_gather_random (read_cb, 0, length, level);
#elif USE_RNDOLDLINUX
  rc = _gcry_rndoldlinux_gather_random (read_cb, 0, length, level);
#elif USE_RNDUNIX
  rc = _gcry_rndunix_gather_random (read_cb, 0, length, level);
#elif USE_RNDW32
  do
    {
      rc = _gcry_rndw32_gather_random (read_cb, 0, length, level);
    }
  while (rc >= 0 && read_cb_len < read_cb_size);
#else
  rc = -1;
#endif

  if (rc < 0 || read_cb_len != read_cb_size)
    {
      log_fatal ("error reading random from system RNG (rc=%d)\n", rc);
    }
}



/* --- Public Functions --- */

/* Initialize this random subsystem.  If FULL is false, this function
   merely calls the basic initialization of the module and does not do
   anything more.  Doing this is not really required but when running
   in a threaded environment we might get a race condition
   otherwise. */
void
_gcry_rngsystem_initialize (int full)
{
  basic_initialization ();
  if (!full)
    return;
  /* Nothing more to initialize.  */
  return;
}


/* Try to close the FDs of the random gather module.  This is
   currently only implemented for rndgetentropy/rndoldlinux. */
void
_gcry_rngsystem_close_fds (void)
{
  lock_rng ();
#if USE_RNDGETENTROPY
  _gcry_rndgetentropy_gather_random (NULL, 0, 0, 0);
#endif
#if USE_RNDOLDLINUX
  _gcry_rndoldlinux_gather_random (NULL, 0, 0, 0);
#endif
  unlock_rng ();
}


/* Print some statistics about the RNG.  */
void
_gcry_rngsystem_dump_stats (void)
{
  /* Not yet implemented.  */
}


/* This function returns true if no real RNG is available or the
   quality of the RNG has been degraded for test purposes.  */
int
_gcry_rngsystem_is_faked (void)
{
  return 0;  /* Faked random is not supported.  */
}


/* Add BUFLEN bytes from BUF to the internal random pool.  QUALITY
   should be in the range of 0..100 to indicate the goodness of the
   entropy added, or -1 for goodness not known. */
gcry_error_t
_gcry_rngsystem_add_bytes (const void *buf, size_t buflen, int quality)
{
  (void)buf;
  (void)buflen;
  (void)quality;
  return 0;  /* Not implemented. */
}


/* Public function to fill the buffer with LENGTH bytes of
   cryptographically strong random bytes.  Level GCRY_WEAK_RANDOM is
   here mapped to GCRY_STRONG_RANDOM, GCRY_STRONG_RANDOM is strong
   enough for most usage, GCRY_VERY_STRONG_RANDOM is good for key
   generation stuff but may be very slow.  */
void
_gcry_rngsystem_randomize (void *buffer, size_t length,
                           enum gcry_random_level level)
{
  _gcry_rngsystem_initialize (1);  /* Auto-initialize if needed.  */

  if (level != GCRY_VERY_STRONG_RANDOM)
    level = GCRY_STRONG_RANDOM;

  lock_rng ();
  get_random (buffer, length, level);
  unlock_rng ();
}
