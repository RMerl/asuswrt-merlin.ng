/* posix-lock.c - GPGRT lock functions for POSIX systems
   Copyright (C) 2005-2009 Free Software Foundation, Inc.
   Copyright (C) 2014 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.

   Parts of the code, in particular use_pthreads_p, are based on code
   from gettext, written by Bruno Haible <bruno@clisp.org>, 2005.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_W32_SYSTEM
# error This module may not be build for Windows.
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if USE_POSIX_THREADS
# include <pthread.h>
#endif

#include "gpgrt-int.h"
#include "lock.h"
#include "posix-lock-obj.h"


#if USE_POSIX_THREADS
# if USE_POSIX_THREADS_FROM_LIBC && HAVE_SYS_SINGLE_THREADED_H
#  include <sys/single_threaded.h>
#  define use_pthread_p()  !__libc_single_threaded
# elif USE_POSIX_THREADS_WEAK
   /* On ELF systems it is easy to use pthreads using weak
      references.  Take care not to test the address of a weak
      referenced function we actually use; some GCC versions have a
      bug were &foo != NULL is always evaluated to true in PIC mode.  */
#  pragma weak pthread_cancel
#  pragma weak pthread_mutex_init
#  pragma weak pthread_mutex_lock
#  pragma weak pthread_mutex_trylock
#  pragma weak pthread_mutex_unlock
#  pragma weak pthread_mutex_destroy
#  if ! PTHREAD_IN_USE_DETECTION_HARD
#   define use_pthread_p() (!!pthread_cancel)
#  endif
# else /*!USE_POSIX_THREADS_WEAK*/
#  if ! PTHREAD_IN_USE_DETECTION_HARD
#   define use_pthread_p() (1)
#  endif
# endif /*!USE_POSIX_THREADS_WEAK*/
# if PTHREAD_IN_USE_DETECTION_HARD
#  if defined __FreeBSD__ || defined __DragonFly__                 /* FreeBSD */

/* Test using pthread_key_create.  */

static int
use_pthread_p (void)
{
  static int tested;
  static int result; /* 1: linked with -lpthread, 0: only with libc */

  if (!tested)
    {
      pthread_key_t key;
      int err = pthread_key_create (&key, NULL);

      if (err == ENOSYS)
        result = 0;
      else
        {
          result = 1;
          if (err == 0)
            pthread_key_delete (key);
        }
      tested = 1;
    }
  return result;
}

#  else                                                     /* Solaris, HP-UX */

/* Test using pthread_create.  */

/* The function to be executed by a dummy thread.  */
static void *
dummy_thread_func (void *arg)
{
  return arg;
}

static int
use_pthread_p (void)
{
  static int tested;
  static int result; /* 1: linked with -lpthread, 0: only with libc */

  if (!tested)
    {
      pthread_t thread;

      if (pthread_create (&thread, NULL, dummy_thread_func, NULL) != 0)
        result = 0; /* Thread creation failed.  */
      else
        {
          /* Thread creation works.  */
          void *retval;
          if (pthread_join (thread, &retval) != 0)
            {
              fputs ("gpgrt fatal: pthread_join in use_pthread_p failed\n",
                     stderr);
              _gpgrt_abort ();
            }
          result = 1;
        }
      tested = 1;
    }
  return result;
}
#  endif                                                     /* Solaris, HP-UX */

# endif /*PTHREAD_IN_USE_DETECTION_HARD*/
#endif /*USE_POSIX_THREADS*/


static _gpgrt_lock_t *
get_lock_object (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = (_gpgrt_lock_t*)lockhd;

  if (lock->vers != LOCK_ABI_VERSION)
    {
      fputs ("gpgrt fatal: lock ABI version mismatch\n", stderr);
      _gpgrt_abort ();
    }
  if (sizeof (gpgrt_lock_t) < sizeof (_gpgrt_lock_t))
    {
      fputs ("gpgrt fatal: sizeof lock obj\n", stderr);
      _gpgrt_abort ();
    }

  return lock;
}


gpg_err_code_t
_gpgrt_lock_init (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = (_gpgrt_lock_t*)lockhd;
  int rc;

  /* If VERS is zero we assume that no static initialization has been
     done, so we setup our ABI version right here.  The caller might
     have called us to test whether lock support is at all available. */
  if (!lock->vers)
    {
      if (sizeof (gpgrt_lock_t) < sizeof (_gpgrt_lock_t))
        {
          fputs ("gpgrt fatal: sizeof lock obj\n", stderr);
          _gpgrt_abort ();
        }
      lock->vers = LOCK_ABI_VERSION;
    }
  else /* Run the usual check.  */
    lock = get_lock_object (lockhd);

#if USE_POSIX_THREADS
  if (use_pthread_p())
    {
      rc = pthread_mutex_init (&lock->u.mtx, NULL);
      if (rc)
        rc = _gpg_err_code_from_errno (rc);
    }
  else
    rc = 0; /* Threads are not used.  */
#else /* Unknown thread system.  */
  rc = lock->vers == LOCK_ABI_NOT_AVAILABLE? 0 : GPG_ERR_NOT_IMPLEMENTED;
#endif /* Unknown thread system.  */

  return rc;
}


gpg_err_code_t
_gpgrt_lock_lock (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = get_lock_object (lockhd);
  int rc;

#if USE_POSIX_THREADS
  if (use_pthread_p())
    {
      _gpgrt_pre_syscall ();
      rc = pthread_mutex_lock (&lock->u.mtx);
      if (rc)
        rc = _gpg_err_code_from_errno (rc);
      _gpgrt_post_syscall ();
    }
  else
    rc = 0; /* Threads are not used.  */
#else /* Unknown thread system.  */
  rc = lock->vers == LOCK_ABI_NOT_AVAILABLE? 0 : GPG_ERR_NOT_IMPLEMENTED;
#endif /* Unknown thread system.  */

  return rc;
}


gpg_err_code_t
_gpgrt_lock_trylock (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = get_lock_object (lockhd);
  int rc;

#if USE_POSIX_THREADS
  if (use_pthread_p())
    {
      rc = pthread_mutex_trylock (&lock->u.mtx);
      if (rc)
        rc = _gpg_err_code_from_errno (rc);
    }
  else
    rc = 0; /* Threads are not used.  */
#else /* Unknown thread system.  */
  rc = lock->vers == LOCK_ABI_NOT_AVAILABLE? 0 : GPG_ERR_NOT_IMPLEMENTED;
#endif /* Unknown thread system.  */

  return rc;
}


gpg_err_code_t
_gpgrt_lock_unlock (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = get_lock_object (lockhd);
  int rc;

#if USE_POSIX_THREADS
  if (use_pthread_p())
    {
      rc = pthread_mutex_unlock (&lock->u.mtx);
      if (rc)
        rc = _gpg_err_code_from_errno (rc);
    }
  else
    rc = 0; /* Threads are not used.  */
#else /* Unknown thread system.  */
  rc = lock->vers == LOCK_ABI_NOT_AVAILABLE? 0 : GPG_ERR_NOT_IMPLEMENTED;
#endif /* Unknown thread system.  */

  return rc;
}


/* Note: Use this function only if no other thread holds or waits for
   this lock.  */
gpg_err_code_t
_gpgrt_lock_destroy (gpgrt_lock_t *lockhd)
{
  _gpgrt_lock_t *lock = get_lock_object (lockhd);
  int rc;

#if USE_POSIX_THREADS
  if (use_pthread_p())
    {
      rc = pthread_mutex_destroy (&lock->u.mtx);
      if (rc)
        rc = _gpg_err_code_from_errno (rc);
      else
        {
          /* Re-init the mutex so that it can be re-used.  */
          gpgrt_lock_t tmp = GPGRT_LOCK_INITIALIZER;
          memcpy (lockhd, &tmp, sizeof tmp);
        }
    }
  else
    rc = 0; /* Threads are not used.  */
#else /* Unknown thread system.  */
  rc = lock->vers == LOCK_ABI_NOT_AVAILABLE? 0 : GPG_ERR_NOT_IMPLEMENTED;
#endif /* Unknown thread system.  */

  return rc;
}
