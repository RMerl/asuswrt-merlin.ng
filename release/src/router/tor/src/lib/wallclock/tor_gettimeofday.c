/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file tor_gettimeofday.c
 * \brief Implementat gettimeofday() for windows, and other platforms without
 *   it.
 **/

#include "orconfig.h"
#include "lib/err/torerr.h"
#include "lib/wallclock/tor_gettimeofday.h"
#include "lib/cc/torint.h"

#include <stddef.h>
#include <stdlib.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef HAVE_GETTIMEOFDAY
#ifdef HAVE_FTIME
#include <sys/timeb.h>
#endif
#endif

/** Set *timeval to the current time of day.  On error, log and terminate.
 * (Same as gettimeofday(timeval,NULL), but never returns -1.)
 */
MOCK_IMPL(void,
tor_gettimeofday, (struct timeval *timeval))
{
#ifdef _WIN32
  /* Epoch bias copied from perl: number of units between windows epoch and
   * Unix epoch. */
#define EPOCH_BIAS UINT64_C(116444736000000000)
#define UNITS_PER_SEC UINT64_C(10000000)
#define USEC_PER_SEC UINT64_C(1000000)
#define UNITS_PER_USEC UINT64_C(10)
  union {
    uint64_t ft_64;
    FILETIME ft_ft;
  } ft;
  /* number of 100-nsec units since Jan 1, 1601 */
  GetSystemTimeAsFileTime(&ft.ft_ft);
  if (ft.ft_64 < EPOCH_BIAS) {
    /* LCOV_EXCL_START */
    raw_assert_unreached_msg("System time is before 1970; failing.");
    /* LCOV_EXCL_STOP */
  }
  ft.ft_64 -= EPOCH_BIAS;
  timeval->tv_sec = (unsigned) (ft.ft_64 / UNITS_PER_SEC);
  timeval->tv_usec = (unsigned) ((ft.ft_64 / UNITS_PER_USEC) % USEC_PER_SEC);
#elif defined(HAVE_GETTIMEOFDAY)
  if (gettimeofday(timeval, NULL)) {
    /* LCOV_EXCL_START */
    /* If gettimeofday dies, we have either given a bad timezone (we didn't),
       or segfaulted.*/
    raw_assert_unreached_msg("gettimeofday failed");
    /* LCOV_EXCL_STOP */
  }
#elif defined(HAVE_FTIME)
  struct timeb tb;
  ftime(&tb);
  timeval->tv_sec = tb.time;
  timeval->tv_usec = tb.millitm * 1000;
#else
#error "No way to get time."
#endif /* defined(_WIN32) || ... */
  return;
}
