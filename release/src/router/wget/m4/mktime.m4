# serial 39
dnl Copyright (C) 2002-2003, 2005-2007, 2009-2024 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Jim Meyering.

AC_DEFUN([gl_TIME_T_IS_SIGNED],
[
  AC_CACHE_CHECK([whether time_t is signed],
    [gl_cv_time_t_is_signed],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([[#include <time.h>
                          char time_t_signed[(time_t) -1 < 0 ? 1 : -1];]])],
       [gl_cv_time_t_is_signed=yes],
       [gl_cv_time_t_is_signed=no])])
  if test $gl_cv_time_t_is_signed = yes; then
    AC_DEFINE([TIME_T_IS_SIGNED], [1], [Define to 1 if time_t is signed.])
  fi
])

dnl Test whether mktime works. Set gl_cv_func_working_mktime.
AC_DEFUN([gl_FUNC_MKTIME_WORKS],
[
  AC_REQUIRE([gl_TIME_T_IS_SIGNED])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  dnl We don't use AC_FUNC_MKTIME any more, because it is no longer maintained
  dnl in Autoconf and because it invokes AC_LIBOBJ.
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CHECK_DECLS_ONCE([alarm])
  AC_REQUIRE([gl_MULTIARCH])
  AC_CACHE_CHECK([for working mktime], [gl_cv_func_working_mktime],
    [if test $APPLE_UNIVERSAL_BUILD = 1; then
       # A universal build on Apple Mac OS X platforms.
       # The test result would be 'yes' in 32-bit mode and 'no' in 64-bit mode.
       # But we need a configuration result that is valid in both modes.
       gl_cv_func_working_mktime="guessing no"
     else
       AC_RUN_IFELSE(
         [AC_LANG_SOURCE(
[[/* Test program from Paul Eggert and Tony Leneis.  */
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_DECL_ALARM
# include <signal.h>
#endif

]GL_MDA_DEFINES[

#ifndef TIME_T_IS_SIGNED
# define TIME_T_IS_SIGNED 0
#endif

static time_t time_t_max;
static time_t time_t_min;

/* Values we'll use to set the TZ environment variable.  */
static char *tz_strings[] = {
  (char *) 0, "TZ=GMT0", "TZ=JST-9",
  "TZ=EST+3EDT+2,M10.1.0/00:00:00,M2.3.0/00:00:00"
};
#define N_STRINGS (sizeof (tz_strings) / sizeof (tz_strings[0]))

/* Return 0 if mktime fails to convert a date in the spring-forward gap.
   Based on a problem report from Andreas Jaeger.  */
static int
spring_forward_gap ()
{
  /* glibc (up to about 1998-10-07) failed this test. */
  struct tm tm;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  if (putenv ("TZ=PST8PDT,M4.1.0,M10.5.0") != 0)
    return -1;

  tm.tm_year = 98;
  tm.tm_mon = 3;
  tm.tm_mday = 5;
  tm.tm_hour = 2;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  return mktime (&tm) != (time_t) -1;
}

static int
mktime_test1 (time_t now)
{
  struct tm *lt;
  return ! (lt = localtime (&now)) || mktime (lt) == now;
}

static int
mktime_test (time_t now)
{
  return (mktime_test1 (now)
          && mktime_test1 ((time_t) (time_t_max - now))
          && mktime_test1 ((time_t) (time_t_min + now)));
}

static int
irix_6_4_bug ()
{
  /* Based on code from Ariel Faigon.  */
  struct tm tm;
  tm.tm_year = 96;
  tm.tm_mon = 3;
  tm.tm_mday = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  mktime (&tm);
  return tm.tm_mon == 2 && tm.tm_mday == 31;
}

static int
bigtime_test (int j)
{
  struct tm tm;
  time_t now;
  tm.tm_year = tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_min = tm.tm_sec = j;
  now = mktime (&tm);
  if (now != (time_t) -1)
    {
      struct tm *lt = localtime (&now);
      if (! (lt
             && lt->tm_year == tm.tm_year
             && lt->tm_mon == tm.tm_mon
             && lt->tm_mday == tm.tm_mday
             && lt->tm_hour == tm.tm_hour
             && lt->tm_min == tm.tm_min
             && lt->tm_sec == tm.tm_sec
             && lt->tm_yday == tm.tm_yday
             && lt->tm_wday == tm.tm_wday
             && ((lt->tm_isdst < 0 ? -1 : 0 < lt->tm_isdst)
                  == (tm.tm_isdst < 0 ? -1 : 0 < tm.tm_isdst))))
        return 0;
    }
  return 1;
}

static int
year_2050_test ()
{
  /* The correct answer for 2050-02-01 00:00:00 in Pacific time,
     ignoring leap seconds.  */
  unsigned long int answer = 2527315200UL;

  struct tm tm;
  time_t t;
  tm.tm_year = 2050 - 1900;
  tm.tm_mon = 2 - 1;
  tm.tm_mday = 1;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  tm.tm_isdst = -1;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  if (putenv ("TZ=PST8PDT,M4.1.0,M10.5.0") != 0)
    return -1;

  t = mktime (&tm);

  /* Check that the result is either a failure, or close enough
     to the correct answer that we can assume the discrepancy is
     due to leap seconds.  */
  return (t == (time_t) -1
          || (0 < t && answer - 120 <= t && t <= answer + 120));
}

static int
indiana_test ()
{
  if (putenv ("TZ=America/Indiana/Indianapolis") != 0)
    return -1;
  struct tm tm;
  tm.tm_year = 1986 - 1900; tm.tm_mon = 4 - 1; tm.tm_mday = 28;
  tm.tm_hour = 16; tm.tm_min = 24; tm.tm_sec = 50; tm.tm_isdst = 0;
  time_t std = mktime (&tm);
  if (! (std == 515107490 || std == 515107503))
    return 1;

  /* This platform supports TZDB, either without or with leap seconds.
     Return true if GNU Bug#48085 is absent.  */
  tm.tm_isdst = 1;
  time_t dst = mktime (&tm);
  return std - dst == 60 * 60;
}

int
main ()
{
  int result = 0;
  time_t t, delta;
  int i, j;
  int time_t_signed_magnitude = (time_t) ~ (time_t) 0 < (time_t) -1;

#if HAVE_DECL_ALARM
  /* This test makes some buggy mktime implementations loop.
     Give up after 60 seconds; a mktime slower than that
     isn't worth using anyway.  */
  signal (SIGALRM, SIG_DFL);
  alarm (60);
#endif

  time_t_max = (! TIME_T_IS_SIGNED
                ? (time_t) -1
                : ((((time_t) 1 << (sizeof (time_t) * CHAR_BIT - 2)) - 1)
                   * 2 + 1));
  time_t_min = (! TIME_T_IS_SIGNED
                ? (time_t) 0
                : time_t_signed_magnitude
                ? ~ (time_t) 0
                : ~ time_t_max);

  delta = time_t_max / 997; /* a suitable prime number */
  for (i = 0; i < N_STRINGS; i++)
    {
      if (tz_strings[i])
        putenv (tz_strings[i]);

      for (t = 0; t <= time_t_max - delta && (result & 1) == 0; t += delta)
        if (! mktime_test (t))
          result |= 1;
      if ((result & 2) == 0
          && ! (mktime_test ((time_t) 1)
                && mktime_test ((time_t) (60 * 60))
                && mktime_test ((time_t) (60 * 60 * 24))))
        result |= 2;

      for (j = 1; (result & 4) == 0; j <<= 1)
        {
          if (! bigtime_test (j))
            result |= 4;
          if (INT_MAX / 2 < j)
            break;
        }
      if ((result & 8) == 0 && ! bigtime_test (INT_MAX))
        result |= 8;
    }
  if (! irix_6_4_bug ())
    result |= 16;
  if (! spring_forward_gap ())
    result |= 32;
  if (! year_2050_test () || ! indiana_test ())
    result |= 64;
  return result;
}]])],
         [gl_cv_func_working_mktime=yes],
         [gl_cv_func_working_mktime=no],
         [case "$host_os" in
                               # Guess no on native Windows.
            mingw* | windows*) gl_cv_func_working_mktime="guessing no" ;;
            *)                 gl_cv_func_working_mktime="$gl_cross_guess_normal" ;;
          esac
         ])
     fi
    ])
])

dnl Main macro of module 'mktime'.
AC_DEFUN([gl_FUNC_MKTIME],
[
  AC_REQUIRE([gl_TIME_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_FUNC_MKTIME_WORKS])

  if test "$gl_cv_func_working_mktime" != yes; then
    REPLACE_MKTIME=1
    AC_DEFINE([NEED_MKTIME_WORKING], [1],
      [Define if the compilation of mktime.c should define 'mktime'
       with the algorithmic workarounds.])
  fi
  case "$host_os" in
    mingw* | windows*)
      REPLACE_MKTIME=1
      AC_DEFINE([NEED_MKTIME_WINDOWS], [1],
        [Define if the compilation of mktime.c should define 'mktime'
         with the native Windows TZ workaround.])
      ;;
  esac
])

dnl Main macro of module 'mktime-internal'.
AC_DEFUN([gl_FUNC_MKTIME_INTERNAL], [
  AC_REQUIRE([gl_FUNC_MKTIME_WORKS])

  WANT_MKTIME_INTERNAL=0
  dnl BeOS has __mktime_internal in libc, but other platforms don't.
  AC_CHECK_FUNC([__mktime_internal],
    [AC_DEFINE([mktime_internal], [__mktime_internal],
       [Define to the real name of the mktime_internal function.])
    ],
    [dnl mktime works but it doesn't export __mktime_internal,
     dnl so we need to substitute our own mktime implementation.
     WANT_MKTIME_INTERNAL=1
     AC_DEFINE([NEED_MKTIME_INTERNAL], [1],
       [Define if the compilation of mktime.c should define 'mktime_internal'.])
    ])
])

# Prerequisites of lib/mktime.c.
AC_DEFUN([gl_PREREQ_MKTIME], [:])
