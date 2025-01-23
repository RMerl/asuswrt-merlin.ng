/* t-lock-single-posix.c - Check the lock functions with single thread
 * Copyright (C) 2021 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#define PGM "t-lock-single"

#include "t-common.h"

GPGRT_LOCK_DEFINE (the_lock);

static int locking_used;

#ifdef USE_POSIX_THREADS_FROM_LIBC
#include <pthread.h>

static void *
thread_func (void *arg)
{
  (void)arg;
  return NULL;
}
#endif

static void
run_test (void)
{
  gpg_err_code_t rc;

  rc = gpgrt_lock_lock (&the_lock);
  if (rc)
    fail ("gpgrt_lock_lock failed at %d: %s", __LINE__, gpg_strerror (rc));

  rc = gpgrt_lock_unlock (&the_lock);
  if (rc)
    fail ("gpgrt_lock_unlock failed at %d: %s", __LINE__,gpg_strerror (rc));

  if (locking_used)
    /*It was incremented, even with single thread.  */
    fail ("Single thread situation is not correctly handled\n");

#ifdef USE_POSIX_THREADS_FROM_LIBC
  /*
   * With !USE_POSIX_THREADS_FROM_LIBC, it is determined at link time.
   *
   * With USE_POSIX_THREADS_FROM_LIBC, it is determined at runtime,
   * and the condition is changed when an application actually uses
   * thread.
   */
  {
    pthread_t rthread;
    pthread_create (&rthread, NULL, thread_func, NULL);

    rc = gpgrt_lock_lock (&the_lock);
    if (rc)
      fail ("gpgrt_lock_lock failed at %d: %s", __LINE__, gpg_strerror (rc));

    rc = gpgrt_lock_unlock (&the_lock);
    if (rc)
      fail ("gpgrt_lock_unlock failed at %d: %s", __LINE__,gpg_strerror (rc));

    pthread_join (rthread, NULL);

    if (!locking_used)
      /*It was *NOT* incremented, even with multiple threads.  */
      fail ("Multiple threads situation is not correctly handled\n");
  }
#endif
}


static void
syscall_clam_pre (void)
{
  if (debug)
    show ("syscall pre called\n");
  locking_used++;
}

static void
syscall_clam_post (void)
{
  if (debug)
    show ("syscall post called\n");
}

int
main (int argc, char **argv)
{
  int last_argc = -1;
  int rc;

  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--help"))
        {
          puts (
"usage: ./t-lock-single [options]\n"
"\n"
"Options:\n"
"  --verbose      Show what is going on\n"
"  --debug        Flyswatter\n"
);
          exit (0);
        }
      if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
    }

  if (!gpg_error_check_version (GPG_ERROR_VERSION))
    {
      die ("gpg_error_check_version returned an error");
      errorcount++;
    }

  gpgrt_set_syscall_clamp (syscall_clam_pre, syscall_clam_post);

  rc = gpgrt_lock_init (&the_lock);
  if (rc)
    fail ("gpgrt_lock_init failed at %d: %s", __LINE__, gpg_strerror (rc));

  run_test ();

  gpgrt_lock_destroy (&the_lock);

  return errorcount ? 1 : 0;
}
