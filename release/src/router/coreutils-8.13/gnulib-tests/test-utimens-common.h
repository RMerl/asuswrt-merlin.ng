/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Test of file timestamp modification functions.
   Copyright (C) 2009-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* This file defines some prerequisites useful to utime-related tests.  */

#ifndef GL_TEST_UTIMENS_COMMON
# define GL_TEST_UTIMENS_COMMON

# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <unistd.h>

/* Gnulib modules.  */
# include "stat-time.h"
# include "timespec.h"
# include "utimecmp.h"

/* Gnulib test header.  */
# include "nap.h"

enum {
  BILLION = 1000 * 1000 * 1000,

  Y2K = 946684800, /* Jan 1, 2000, in seconds since epoch.  */

  /* Bogus positive and negative tv_nsec values closest to valid
     range, but without colliding with UTIME_NOW or UTIME_OMIT.  */
  UTIME_BOGUS_POS = BILLION + ((UTIME_NOW == BILLION || UTIME_OMIT == BILLION)
                               ? (1 + (UTIME_NOW == BILLION + 1)
                                  + (UTIME_OMIT == BILLION + 1))
                               : 0),
  UTIME_BOGUS_NEG = -1 - ((UTIME_NOW == -1 || UTIME_OMIT == -1)
                          ? (1 + (UTIME_NOW == -2) + (UTIME_OMIT == -2))
                          : 0)
};

# if (defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__
/* Skip ctime tests on native Windows, since it is either a copy of
   mtime or birth time (depending on the file system), rather than a
   properly tracked change time.  */
#  define check_ctime 0
# else
#  define check_ctime 1
# endif

#endif /* GL_TEST_UTIMENS_COMMON */
