/* Query the name of the current global locale.
   Copyright (C) 2019-2024 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

/* Specification.  */
#include "setlocale_null.h"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#if !(SETLOCALE_NULL_ALL_MTSAFE && SETLOCALE_NULL_ONE_MTSAFE)

# if AVOID_ANY_THREADS

/* The option '--disable-threads' explicitly requests no locking.  */

# elif defined _WIN32 && !defined __CYGWIN__

#  define WIN32_LEAN_AND_MEAN  /* avoid including junk */
#  include <windows.h>

# elif HAVE_PTHREAD_API

#  include <pthread.h>
#  if HAVE_THREADS_H && HAVE_WEAK_SYMBOLS
#   include <threads.h>
#   pragma weak thrd_exit
#   define c11_threads_in_use() (thrd_exit != NULL)
#  else
#   define c11_threads_in_use() 0
#  endif

# elif HAVE_THREADS_H

#  include <threads.h>

# endif

#endif

#if !(SETLOCALE_NULL_ALL_MTSAFE && SETLOCALE_NULL_ONE_MTSAFE) /* musl libc, macOS, FreeBSD, NetBSD, OpenBSD, AIX, Haiku, Cygwin < 3.4.6 */

/* Use a lock, so that no two threads can invoke setlocale_null_r_unlocked
   at the same time.  */

/* Prohibit renaming this symbol.  */
# undef gl_get_setlocale_null_lock

# if AVOID_ANY_THREADS

/* The option '--disable-threads' explicitly requests no locking.  */
#  define setlocale_null_r_with_lock setlocale_null_r_unlocked

# elif defined _WIN32 && !defined __CYGWIN__

extern __declspec(dllimport) CRITICAL_SECTION *gl_get_setlocale_null_lock (void);

static int
setlocale_null_r_with_lock (int category, char *buf, size_t bufsize)
{
  CRITICAL_SECTION *lock = gl_get_setlocale_null_lock ();
  int ret;

  EnterCriticalSection (lock);
  ret = setlocale_null_r_unlocked (category, buf, bufsize);
  LeaveCriticalSection (lock);

  return ret;
}

# elif HAVE_PTHREAD_API /* musl libc, macOS, FreeBSD, NetBSD, OpenBSD, AIX, Haiku, Cygwin < 3.4.6 */

extern
#  if defined _WIN32 || defined __CYGWIN__
  __declspec(dllimport)
#  endif
  pthread_mutex_t *gl_get_setlocale_null_lock (void);

#  if HAVE_WEAK_SYMBOLS /* musl libc, FreeBSD, NetBSD, OpenBSD, Haiku */

    /* Avoid the need to link with '-lpthread'.  */
#   pragma weak pthread_mutex_lock
#   pragma weak pthread_mutex_unlock

    /* Determine whether libpthread is in use.  */
#   pragma weak pthread_mutexattr_gettype
    /* See the comments in lock.h.  */
#   define pthread_in_use() \
      (pthread_mutexattr_gettype != NULL || c11_threads_in_use ())

#  else
#   define pthread_in_use() 1
#  endif

static int
setlocale_null_r_with_lock (int category, char *buf, size_t bufsize)
{
  if (pthread_in_use())
    {
      pthread_mutex_t *lock = gl_get_setlocale_null_lock ();
      int ret;

      if (pthread_mutex_lock (lock))
        abort ();
      ret = setlocale_null_r_unlocked (category, buf, bufsize);
      if (pthread_mutex_unlock (lock))
        abort ();

      return ret;
    }
  else
    return setlocale_null_r_unlocked (category, buf, bufsize);
}

# elif HAVE_THREADS_H

extern mtx_t *gl_get_setlocale_null_lock (void);

static int
setlocale_null_r_with_lock (int category, char *buf, size_t bufsize)
{
  mtx_t *lock = gl_get_setlocale_null_lock ();
  int ret;

  if (mtx_lock (lock) != thrd_success)
    abort ();
  ret = setlocale_null_r_unlocked (category, buf, bufsize);
  if (mtx_unlock (lock) != thrd_success)
    abort ();

  return ret;
}

# endif

#endif

int
setlocale_null_r (int category, char *buf, size_t bufsize)
{
#if SETLOCALE_NULL_ALL_MTSAFE
# if SETLOCALE_NULL_ONE_MTSAFE

  return setlocale_null_r_unlocked (category, buf, bufsize);

# else

  if (category == LC_ALL)
    return setlocale_null_r_unlocked (category, buf, bufsize);
  else
    return setlocale_null_r_with_lock (category, buf, bufsize);

# endif
#else
# if SETLOCALE_NULL_ONE_MTSAFE

  if (category == LC_ALL)
    return setlocale_null_r_with_lock (category, buf, bufsize);
  else
    return setlocale_null_r_unlocked (category, buf, bufsize);

# else

  return setlocale_null_r_with_lock (category, buf, bufsize);

# endif
#endif
}

const char *
setlocale_null (int category)
{
#if SETLOCALE_NULL_ALL_MTSAFE && SETLOCALE_NULL_ONE_MTSAFE
  return setlocale_null_unlocked (category);
#else

  /* This call must be multithread-safe.  To achieve this without using
     thread-local storage:
       1. We use a specific static buffer for each possible CATEGORY
          argument.  So that different threads can call setlocale_mtsafe
          with different CATEGORY arguments, without interfering.
       2. We use a simple strcpy or memcpy to fill this static buffer.
          Filling it through, for example, strcpy + strcat would not be
          guaranteed to leave the buffer's contents intact if another thread
          is currently accessing it.  If necessary, the contents is first
          assembled in a stack-allocated buffer.  */
  if (category == LC_ALL)
    {
# if SETLOCALE_NULL_ALL_MTSAFE
      return setlocale_null_unlocked (LC_ALL);
# else
      char buf[SETLOCALE_NULL_ALL_MAX];
      static char resultbuf[SETLOCALE_NULL_ALL_MAX];

      if (setlocale_null_r (LC_ALL, buf, sizeof (buf)))
        return "C";
      strcpy (resultbuf, buf);
      return resultbuf;
# endif
    }
  else
    {
# if SETLOCALE_NULL_ONE_MTSAFE
      return setlocale_null_unlocked (category);
# else
      enum
        {
          LC_CTYPE_INDEX,
          LC_NUMERIC_INDEX,
          LC_TIME_INDEX,
          LC_COLLATE_INDEX,
          LC_MONETARY_INDEX,
          LC_MESSAGES_INDEX,
#  ifdef LC_PAPER
          LC_PAPER_INDEX,
#  endif
#  ifdef LC_NAME
          LC_NAME_INDEX,
#  endif
#  ifdef LC_ADDRESS
          LC_ADDRESS_INDEX,
#  endif
#  ifdef LC_TELEPHONE
          LC_TELEPHONE_INDEX,
#  endif
#  ifdef LC_MEASUREMENT
          LC_MEASUREMENT_INDEX,
#  endif
#  ifdef LC_IDENTIFICATION
          LC_IDENTIFICATION_INDEX,
#  endif
          LC_INDICES_COUNT
        }
        i;
      char buf[SETLOCALE_NULL_MAX];
      static char resultbuf[LC_INDICES_COUNT][SETLOCALE_NULL_MAX];
      int err;

      err = setlocale_null_r (category, buf, sizeof (buf));
      if (err == EINVAL)
        return NULL;
      if (err)
        return "C";

      switch (category)
        {
        case LC_CTYPE:          i = LC_CTYPE_INDEX;          break;
        case LC_NUMERIC:        i = LC_NUMERIC_INDEX;        break;
        case LC_TIME:           i = LC_TIME_INDEX;           break;
        case LC_COLLATE:        i = LC_COLLATE_INDEX;        break;
        case LC_MONETARY:       i = LC_MONETARY_INDEX;       break;
        case LC_MESSAGES:       i = LC_MESSAGES_INDEX;       break;
#  ifdef LC_PAPER
        case LC_PAPER:          i = LC_PAPER_INDEX;          break;
#  endif
#  ifdef LC_NAME
        case LC_NAME:           i = LC_NAME_INDEX;           break;
#  endif
#  ifdef LC_ADDRESS
        case LC_ADDRESS:        i = LC_ADDRESS_INDEX;        break;
#  endif
#  ifdef LC_TELEPHONE
        case LC_TELEPHONE:      i = LC_TELEPHONE_INDEX;      break;
#  endif
#  ifdef LC_MEASUREMENT
        case LC_MEASUREMENT:    i = LC_MEASUREMENT_INDEX;    break;
#  endif
#  ifdef LC_IDENTIFICATION
        case LC_IDENTIFICATION: i = LC_IDENTIFICATION_INDEX; break;
#  endif
        default:
          /* If you get here, a #ifdef LC_xxx is missing.  */
          abort ();
        }

      strcpy (resultbuf[i], buf);
      return resultbuf[i];
# endif
    }
#endif
}
