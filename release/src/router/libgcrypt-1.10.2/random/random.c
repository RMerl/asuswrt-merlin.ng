/* random.c - Random number switch
 * Copyright (C) 2003, 2006, 2008, 2012  Free Software Foundation, Inc.
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
  This module switches between different implementations of random
  number generators and provides a few help functions.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SYSLOG
# include <syslog.h>
#endif /*HAVE_SYSLOG*/
#include <ctype.h>

#include "g10lib.h"
#include "random.h"
#include "rand-internal.h"
#include "cipher.h"         /* For _gcry_sha1_hash_buffer().  */

/* The name of a file used to globally configure the RNG. */
#define RANDOM_CONF_FILE "/etc/gcrypt/random.conf"


/* If not NULL a progress function called from certain places and the
   opaque value passed along.  Registered by
   _gcry_register_random_progress (). */
static void (*progress_cb) (void *,const char*,int,int, int );
static void *progress_cb_data;

/* Flags indicating the requested RNG types.  */
static struct
{
  int standard;
  int fips;
  int system;
} rng_types;


/* This is the lock we use to protect the buffer used by the nonce
   generation.  */
GPGRT_LOCK_DEFINE (nonce_buffer_lock);



/* ---  Functions  --- */


/* Used to register a progress callback.  This needs to be called
   before any threads are created. */
void
_gcry_register_random_progress (void (*cb)(void *,const char*,int,int,int),
                                void *cb_data )
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}


/* This progress function is currently used by the random modules to
   give hints on how much more entropy is required. */
void
_gcry_random_progress (const char *what, int printchar, int current, int total)
{
  if (progress_cb)
    progress_cb (progress_cb_data, what, printchar, current, total);
}


/* Read a file with configure options.  The file is a simple text file
 * where empty lines and lines with the first non white-space
 * character being '#' are ignored.  Supported configure options are:
 *
 *  disable-jent - Disable the jitter based extra entropy generator.
 *                 This sets the RANDOM_CONF_DISABLE_JENT bit.
 *  only-urandom - Always use /dev/urandom instead of /dev/random.
 *                 This sets the RANDOM_CONF_ONLY_URANDOM bit.
 *
 * The function returns a bit vector with flags read from the file.
 */
unsigned int
_gcry_random_read_conf (void)
{
  const char *fname = RANDOM_CONF_FILE;
  FILE *fp;
  char buffer[256];
  char *p, *pend;
  int lnr = 0;
  unsigned int result = 0;

  fp = fopen (fname, "r");
  if (!fp)
    return result;

  for (;;)
    {
      if (!fgets (buffer, sizeof buffer, fp))
        {
          if (!feof (fp))
            {
#ifdef HAVE_SYSLOG
              syslog (LOG_USER|LOG_WARNING,
                      "Libgcrypt warning: error reading '%s', line %d",
                      fname, lnr);
#endif /*HAVE_SYSLOG*/
            }
          fclose (fp);
          return result;
        }
      lnr++;
      for (p=buffer; my_isascii (*p) && isspace (*p); p++)
        ;
      pend = strchr (p, '\n');
      if (pend)
        *pend = 0;
      pend = p + (*p? (strlen (p)-1):0);
      for ( ;pend > p; pend--)
        if (my_isascii (*pend) && isspace (*pend))
          *pend = 0;
      if (!*p || *p == '#')
        continue;

      if (!strcmp (p, "disable-jent"))
        result |= RANDOM_CONF_DISABLE_JENT;
      else if (!strcmp (p, "only-urandom"))
        result |= RANDOM_CONF_ONLY_URANDOM;
      else
        {
#ifdef HAVE_SYSLOG
          syslog (LOG_USER|LOG_WARNING,
                  "Libgcrypt warning: unknown option in '%s', line %d",
                  fname, lnr);
#endif /*HAVE_SYSLOG*/
        }
    }
}


/* Set the preferred RNG type.  This may be called at any time even
   before gcry_check_version.  Thus we can't assume any thread system
   initialization.  A type of 0 is used to indicate that any Libgcrypt
   initialization has been done.*/
void
_gcry_set_preferred_rng_type (int type)
{
  static int any_init;

  if (!type)
    {
      any_init = 1;
    }
  else if (type == GCRY_RNG_TYPE_STANDARD)
    {
      rng_types.standard = 1;
    }
  else if (any_init)
    {
      /* After any initialization has been done we only allow to
         upgrade to the standard RNG (handled above).  All other
         requests are ignored.  The idea is that the application needs
         to declare a preference for a weaker RNG as soon as possible
         and before any library sets a preference.  We assume that a
         library which uses Libgcrypt calls an init function very
         early.  This way --- even if the library gets initialized
         early by the application --- it is unlikely that it can
         select a lower priority RNG.

         This scheme helps to ensure that existing unmodified
         applications (e.g. gpg2), which don't known about the new RNG
         selection system, will continue to use the standard RNG and
         not be tricked by some library to use a lower priority RNG.
         There are some loopholes here but at least most GnuPG stuff
         should be save because it calls src_c{gcry_control
         (GCRYCTL_SUSPEND_SECMEM_WARN);} quite early and thus inhibits
         switching to a low priority RNG.
       */
    }
  else if (type == GCRY_RNG_TYPE_FIPS)
    {
      rng_types.fips = 1;
    }
  else if (type == GCRY_RNG_TYPE_SYSTEM)
    {
      rng_types.system = 1;
    }
}


/* Initialize this random subsystem.  If FULL is false, this function
   merely calls the basic initialization of the module and does not do
   anything more.  Doing this is not really required but when running
   in a threaded environment we might get a race condition
   otherwise. */
void
_gcry_random_initialize (int full)
{
  if (fips_mode ())
    _gcry_rngdrbg_inititialize (full);
  else if (rng_types.standard)
    _gcry_rngcsprng_initialize (full);
  else if (rng_types.fips)
    _gcry_rngdrbg_inititialize (full);
  else if (rng_types.system)
    _gcry_rngsystem_initialize (full);
  else
    _gcry_rngcsprng_initialize (full);
}


/* If possible close file descriptors used by the RNG. */
void
_gcry_random_close_fds (void)
{
  /* Note that we can't do that directly because each random system
     has its own lock functions which need to be used for accessing
     the entropy gatherer.  */

  if (fips_mode ())
    _gcry_rngdrbg_close_fds ();
  else if (rng_types.standard)
    _gcry_rngcsprng_close_fds ();
  else if (rng_types.fips)
    _gcry_rngdrbg_close_fds ();
  else if (rng_types.system)
    _gcry_rngsystem_close_fds ();
  else
    _gcry_rngcsprng_close_fds ();
}


/* Return the current RNG type.  IGNORE_FIPS_MODE is a flag used to
   skip the test for FIPS.  This is useful, so that we are able to
   return the type of the RNG even before we have setup FIPS mode
   (note that FIPS mode is enabled by default until it is switched off
   by the initialization).  This is mostly useful for the regression
   test.  */
int
_gcry_get_rng_type (int ignore_fips_mode)
{
  if (!ignore_fips_mode && fips_mode ())
    return GCRY_RNG_TYPE_FIPS;
  else if (rng_types.standard)
    return GCRY_RNG_TYPE_STANDARD;
  else if (rng_types.fips)
    return GCRY_RNG_TYPE_FIPS;
  else if (rng_types.system)
    return GCRY_RNG_TYPE_SYSTEM;
  else
    return GCRY_RNG_TYPE_STANDARD;
}


void
_gcry_random_dump_stats (void)
{
  if (fips_mode ())
    _gcry_rngdrbg_dump_stats ();
  else
    _gcry_rngcsprng_dump_stats ();
  _gcry_rndjent_dump_stats ();
}


/* This function should be called during initialization and before
   initialization of this module to place the random pools into secure
   memory.  */
void
_gcry_secure_random_alloc (void)
{
  if (fips_mode ())
    ;  /* Not used; the FIPS RNG is always in secure mode.  */
  else
    _gcry_rngcsprng_secure_alloc ();
}


/* This may be called before full initialization to degrade the
   quality of the RNG for the sake of a faster running test suite.  */
void
_gcry_enable_quick_random_gen (void)
{
  if (fips_mode ())
    ;  /* Not used.  */
  else
    _gcry_rngcsprng_enable_quick_gen ();
}


/* This function returns true if no real RNG is available or the
   quality of the RNG has been degraded for test purposes.  */
int
_gcry_random_is_faked (void)
{
  if (fips_mode ())
    return _gcry_rngdrbg_is_faked ();
  else
    return _gcry_rngcsprng_is_faked ();
}


/* Add BUFLEN bytes from BUF to the internal random pool.  QUALITY
   should be in the range of 0..100 to indicate the goodness of the
   entropy added, or -1 for goodness not known.  */
gcry_err_code_t
_gcry_random_add_bytes (const void *buf, size_t buflen, int quality)
{
  if (fips_mode ())
    return 0; /* No need for this in fips mode.  */
  else if (rng_types.standard)
    return gpg_err_code (_gcry_rngcsprng_add_bytes (buf, buflen, quality));
  else if (rng_types.fips)
    return 0;
  else if (rng_types.system)
    return 0;
  else /* default */
    return gpg_err_code (_gcry_rngcsprng_add_bytes (buf, buflen, quality));
}


/* Helper function.  */
static void
do_randomize (void *buffer, size_t length, enum gcry_random_level level)
{
  if (fips_mode ())
    _gcry_rngdrbg_randomize (buffer, length, level);
  else if (rng_types.standard)
    _gcry_rngcsprng_randomize (buffer, length, level);
  else if (rng_types.fips)
    _gcry_rngdrbg_randomize (buffer, length, level);
  else if (rng_types.system)
    _gcry_rngsystem_randomize (buffer, length, level);
  else /* default */
    _gcry_rngcsprng_randomize (buffer, length, level);
}

/* The public function to return random data of the quality LEVEL.
   Returns a pointer to a newly allocated and randomized buffer of
   LEVEL and NBYTES length.  Caller must free the buffer.  */
void *
_gcry_random_bytes (size_t nbytes, enum gcry_random_level level)
{
  void *buffer;

  buffer = xmalloc (nbytes);
  do_randomize (buffer, nbytes, level);
  return buffer;
}


/* The public function to return random data of the quality LEVEL;
   this version of the function returns the random in a buffer allocated
   in secure memory.  Caller must free the buffer. */
void *
_gcry_random_bytes_secure (size_t nbytes, enum gcry_random_level level)
{
  void *buffer;

  /* Historical note (1.3.0--1.4.1): The buffer was only allocated
     in secure memory if the pool in random-csprng.c was also set to
     use secure memory.  */
  buffer = xmalloc_secure (nbytes);
  do_randomize (buffer, nbytes, level);
  return buffer;
}


/* Public function to fill the buffer with LENGTH bytes of
   cryptographically strong random bytes.  Level GCRY_WEAK_RANDOM is
   not very strong, GCRY_STRONG_RANDOM is strong enough for most
   usage, GCRY_VERY_STRONG_RANDOM is good for key generation stuff but
   may be very slow.  */
void
_gcry_randomize (void *buffer, size_t length, enum gcry_random_level level)
{
  do_randomize (buffer, length, level);
}


/* This function may be used to specify the file to be used as a seed
   file for the PRNG.  This function should be called prior to the
   initialization of the random module.  NAME may not be NULL.  */
void
_gcry_set_random_seed_file (const char *name)
{
  if (fips_mode ())
    ; /* No need for this in fips mode.  */
  else if (rng_types.standard)
    _gcry_rngcsprng_set_seed_file (name);
  else if (rng_types.fips)
    ;
  else if (rng_types.system)
    ;
  else /* default */
    _gcry_rngcsprng_set_seed_file (name);
}


/* If a seed file has been setup, this function may be used to write
   back the random numbers entropy pool.  */
void
_gcry_update_random_seed_file (void)
{
  if (fips_mode ())
    ; /* No need for this in fips mode.  */
  else if (rng_types.standard)
    _gcry_rngcsprng_update_seed_file ();
  else if (rng_types.fips)
    ;
  else if (rng_types.system)
    ;
  else /* default */
    _gcry_rngcsprng_update_seed_file ();
}



/* The fast random pool function as called at some places in
   libgcrypt.  This is merely a wrapper to make sure that this module
   is initialized and to lock the pool.  Note, that this function is a
   NOP unless a random function has been used or _gcry_initialize (1)
   has been used.  We use this hack so that the internal use of this
   function in cipher_open and md_open won't start filling up the
   random pool, even if no random will be required by the process. */
void
_gcry_fast_random_poll (void)
{
  if (fips_mode ())
    ; /* No need for this in fips mode.  */
  else if (rng_types.standard)
    _gcry_rngcsprng_fast_poll ();
  else if (rng_types.fips)
    ;
  else if (rng_types.system)
    ;
  else /* default */
    _gcry_rngcsprng_fast_poll ();
}



/* Create an unpredicable nonce of LENGTH bytes in BUFFER. */
void
_gcry_create_nonce (void *buffer, size_t length)
{
  static unsigned char nonce_buffer[20+8];
  static int nonce_buffer_initialized = 0;
  static volatile pid_t my_pid; /* The volatile is there to make sure the
                                   compiler does not optimize the code away
                                   in case the getpid function is badly
                                   attributed. */
  volatile pid_t apid;
  unsigned char *p;
  size_t n;
  int err;

  /* First check whether we shall use the FIPS nonce generator.  This
     is only done in FIPS mode, in all other modes, we use our own
     nonce generator which is seeded by the RNG actual in use.  */
  if (fips_mode ())
    {
      _gcry_rngdrbg_randomize (buffer, length, GCRY_WEAK_RANDOM);
      return;
    }

  /* This is the nonce generator, which formerly lived in
     random-csprng.c.  It is now used by all RNG types except when in
     FIPS mode (not that this means it is also used if the FIPS RNG
     has been selected but we are not in fips mode).  */

  /* Make sure we are initialized. */
  _gcry_random_initialize (1);

  /* Acquire the nonce buffer lock. */
  err = gpgrt_lock_lock (&nonce_buffer_lock);
  if (err)
    log_fatal ("failed to acquire the nonce buffer lock: %s\n",
               gpg_strerror (err));

  apid = getpid ();
  /* The first time initialize our buffer. */
  if (!nonce_buffer_initialized)
    {
      time_t atime = time (NULL);
      pid_t xpid = apid;

      my_pid = apid;

      if ((sizeof apid + sizeof atime) > sizeof nonce_buffer)
        BUG ();

      /* Initialize the first 20 bytes with a reasonable value so that
         a failure of gcry_randomize won't affect us too much.  Don't
         care about the uninitialized remaining bytes. */
      p = nonce_buffer;
      memcpy (p, &xpid, sizeof xpid);
      p += sizeof xpid;
      memcpy (p, &atime, sizeof atime);

      /* Initialize the never changing private part of 64 bits. */
      _gcry_randomize (nonce_buffer+20, 8, GCRY_WEAK_RANDOM);

      nonce_buffer_initialized = 1;
    }
  else if ( my_pid != apid )
    {
      /* We forked. Need to reseed the buffer - doing this for the
         private part should be sufficient. */
      do_randomize (nonce_buffer+20, 8, GCRY_WEAK_RANDOM);
      /* Update the pid so that we won't run into here again and
         again. */
      my_pid = apid;
    }

  /* Create the nonce by hashing the entire buffer, returning the hash
     and updating the first 20 bytes of the buffer with this hash. */
  for (p = buffer; length > 0; length -= n, p += n)
    {
      _gcry_sha1_hash_buffer (nonce_buffer,
                              nonce_buffer, sizeof nonce_buffer);
      n = length > 20? 20 : length;
      memcpy (p, nonce_buffer, n);
    }

  /* Release the nonce buffer lock. */
  err = gpgrt_lock_unlock (&nonce_buffer_lock);
  if (err)
    log_fatal ("failed to release the nonce buffer lock: %s\n",
               gpg_strerror (err));
}


/* Run the self-tests for the RNG.  This is currently only implemented
   for the FIPS generator.  */
gpg_error_t
_gcry_random_selftest (selftest_report_func_t report)
{
  if (fips_mode ())
    return _gcry_rngdrbg_selftest (report);
  else
    return 0; /* No selftests yet.  */
}
