/* gen-posix-lock-obj.c - Build tool to construct the lock object.
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
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_W32_SYSTEM
# error This module may not be build for Windows.
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#ifdef USE_POSIX_THREADS
# include <pthread.h>
#endif

#include "posix-lock-obj.h"

#define PGM "gen-posix-lock-obj"

/* Check that configure did its job.  */
#ifdef USE_POSIX_THREADS
#if SIZEOF_PTHREAD_MUTEX_T < 4
# error sizeof pthread_mutex_t is not known.
#endif
#endif

/* Special requirements for certain platforms.  */
# define USE_LONG_DOUBLE_FOR_ALIGNMENT 0
#if defined(__sun) && !defined (__LP64__) && !defined(_LP64)
/* Solaris on 32-bit architecture.  */
# define USE_DOUBLE_FOR_ALIGNMENT 1
#else
# define USE_DOUBLE_FOR_ALIGNMENT 0
#endif
#if defined(__hppa__)
# define USE_16BYTE_ALIGNMENT 1
#else
# define USE_16BYTE_ALIGNMENT 0
#endif

#if USE_16BYTE_ALIGNMENT && !HAVE_GCC_ATTRIBUTE_ALIGNED
# error compiler is not able to enforce a 16 byte alignment
#endif

#ifdef USE_POSIX_THREADS
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

int
main (void)
{
#ifdef USE_POSIX_THREADS
  unsigned char *p;
  int i;
#endif
  struct {
    long vers;
#ifdef USE_POSIX_THREADS
    pthread_mutex_t mtx;
#endif
  } dummyobj;


#ifdef USE_POSIX_THREADS
  if (sizeof mtx != SIZEOF_PTHREAD_MUTEX_T)
    {
      fprintf (stderr, PGM ": pthread_mutex_t mismatch\n");
      exit (1);
    }
#endif /*USE_POSIX_THREADS*/

  if (sizeof (dummyobj) != sizeof (_gpgrt_lock_t))
    {
      fprintf (stderr, PGM ": internal and external lock object mismatch\n");
      exit (1);
    }

  printf ("## lock-obj-pub.%s.h%s\n"
          "## File created by " PGM " - DO NOT EDIT\n"
          "## To be included by mkheader into gpg-error.h\n"
          "\n",
          HOST_TRIPLET_STRING,
#ifdef USE_POSIX_THREADS
          ""
#else
          " - NO LOCK SUPPORT"
#endif
          );

#ifdef USE_POSIX_THREADS

  /* To force a probably suitable alignment of the structure we use a
     union and include a long and a pointer to a long.  */
  printf ("typedef struct\n"
          "{\n"
          "  long _vers;\n"
          "  union {\n"
          "    volatile char _priv[%d];\n"
          "%s"
          "    long _x_align;\n"
          "    long *_xp_align;\n"
          "  } u;\n"
          "} gpgrt_lock_t;\n"
          "\n"
          "#define GPGRT_LOCK_INITIALIZER {%d,{{",
          SIZEOF_PTHREAD_MUTEX_T,
# if USE_16BYTE_ALIGNMENT
          "    int _x16_align __attribute__ ((aligned (16)));\n",
# elif USE_DOUBLE_FOR_ALIGNMENT
          "    double _xd_align;\n",
# elif USE_LONG_DOUBLE_FOR_ALIGNMENT
          "    long double _xld_align;\n",
# else
          "",
# endif
          LOCK_ABI_VERSION);
  p = (unsigned char *)&mtx;
  for (i=0; i < sizeof mtx; i++)
    {
      if (i && !(i % 8))
        printf (" \\\n%*s", 36, "");
      printf ("%u", p[i]);
      if (i < sizeof mtx - 1)
        putchar (',');
    }
  fputs ("}}}\n", stdout);

#else /*!USE_POSIX_THREADS*/

  printf ("/* Dummy object - no locking available.  */\n"
          "typedef struct\n"
          "{\n"
          "  long _vers;\n"
          "} gpgrt_lock_t;\n"
          "\n"
          "#define GPGRT_LOCK_INITIALIZER {%d}\n",
          LOCK_ABI_VERSION);

#endif /*!USE_POSIX_THREADS*/

  fputs ("##\n"
         "## Loc" "al Variables:\n"
         "## mode: c\n"
         "## buffer-read-only: t\n"
         "## End:\n"
         "##\n", stdout);

  if (ferror (stdout))
    {
      fprintf (stderr, PGM ": error writing to stdout: %s\n", strerror (errno));
      return 1;
    }

  return 0;
}
