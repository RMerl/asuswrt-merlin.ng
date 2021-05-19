/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file tvdiff.c
 * \brief Compute the difference between timevals, in various units.
 **/

#include "lib/time/tvdiff.h"

#include "lib/cc/compat_compiler.h"
#include "lib/defs/time.h"
#include "lib/log/log.h"

#ifdef _WIN32
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/** Return the difference between start->tv_sec and end->tv_sec.
 * Returns INT64_MAX on overflow and underflow.
 */
static int64_t
tv_secdiff_impl(const struct timeval *start, const struct timeval *end)
{
  const int64_t s = (int64_t)start->tv_sec;
  const int64_t e = (int64_t)end->tv_sec;

  /* This may not be the most efficient way of implementing this check,
   * but it's easy to see that it's correct and doesn't overflow */

  if (s > 0 && e < INT64_MIN + s) {
    /* s is positive: equivalent to e - s < INT64_MIN, but without any
     * overflow */
    return INT64_MAX;
  } else if (s < 0 && e > INT64_MAX + s) {
    /* s is negative: equivalent to e - s > INT64_MAX, but without any
     * overflow */
    return INT64_MAX;
  }

  return e - s;
}

/** Return the number of microseconds elapsed between *start and *end.
 * Returns LONG_MAX on overflow and underflow.
 */
long
tv_udiff(const struct timeval *start, const struct timeval *end)
{
  /* Sanity check tv_usec */
  if (start->tv_usec > TOR_USEC_PER_SEC || start->tv_usec < 0) {
    log_warn(LD_GENERAL, "comparing times on microsecond detail with bad "
             "start tv_usec: %"PRId64 " microseconds",
             (int64_t)start->tv_usec);
    return LONG_MAX;
  }

  if (end->tv_usec > TOR_USEC_PER_SEC || end->tv_usec < 0) {
    log_warn(LD_GENERAL, "comparing times on microsecond detail with bad "
             "end tv_usec: %"PRId64 " microseconds",
             (int64_t)end->tv_usec);
    return LONG_MAX;
  }

  /* Some BSDs have struct timeval.tv_sec 64-bit, but time_t (and long) 32-bit
   */
  int64_t udiff;
  const int64_t secdiff = tv_secdiff_impl(start, end);

  /* end->tv_usec - start->tv_usec can be up to 1 second either way */
  if (secdiff > (int64_t)(LONG_MAX/1000000 - 1) ||
      secdiff < (int64_t)(LONG_MIN/1000000 + 1)) {
    log_warn(LD_GENERAL, "comparing times on microsecond detail too far "
             "apart: %"PRId64 " seconds", (secdiff));
    return LONG_MAX;
  }

  /* we'll never get an overflow here, because we check that both usecs are
   * between 0 and TV_USEC_PER_SEC. */
  udiff = secdiff*1000000 + ((int64_t)end->tv_usec - (int64_t)start->tv_usec);

  /* Some compilers are smart enough to work out this is a no-op on L64 */
#if SIZEOF_LONG < 8
  if (udiff > (int64_t)LONG_MAX || udiff < (int64_t)LONG_MIN) {
    return LONG_MAX;
  }
#endif

  return (long)udiff;
}

/** Return the number of milliseconds elapsed between *start and *end.
 * If the tv_usec difference is 500, rounds away from zero.
 * Returns LONG_MAX on overflow and underflow.
 */
long
tv_mdiff(const struct timeval *start, const struct timeval *end)
{
  /* Sanity check tv_usec */
  if (start->tv_usec > TOR_USEC_PER_SEC || start->tv_usec < 0) {
    log_warn(LD_GENERAL, "comparing times on millisecond detail with bad "
             "start tv_usec: %"PRId64 " microseconds",
             (int64_t)start->tv_usec);
    return LONG_MAX;
  }

  if (end->tv_usec > TOR_USEC_PER_SEC || end->tv_usec < 0) {
    log_warn(LD_GENERAL, "comparing times on millisecond detail with bad "
             "end tv_usec: %"PRId64 " microseconds",
             (int64_t)end->tv_usec);
    return LONG_MAX;
  }

  /* Some BSDs have struct timeval.tv_sec 64-bit, but time_t (and long) 32-bit
   */
  int64_t mdiff;
  const int64_t secdiff = tv_secdiff_impl(start, end);

  /* end->tv_usec - start->tv_usec can be up to 1 second either way, but the
   * mdiff calculation may add another temporary second for rounding.
   * Whether this actually causes overflow depends on the compiler's constant
   * folding and order of operations. */
  if (secdiff > (int64_t)(LONG_MAX/1000 - 2) ||
      secdiff < (int64_t)(LONG_MIN/1000 + 1)) {
    log_warn(LD_GENERAL, "comparing times on millisecond detail too far "
             "apart: %"PRId64 " seconds", (int64_t)secdiff);
    return LONG_MAX;
  }

  /* Subtract and round */
  mdiff = secdiff*1000 +
      /* We add a million usec here to ensure that the result is positive,
       * so that the round-towards-zero behavior of the division will give
       * the right result for rounding to the nearest msec. Later we subtract
       * 1000 in order to get the correct result.
       * We'll never get an overflow here, because we check that both usecs are
       * between 0 and TV_USEC_PER_SEC. */
      ((int64_t)end->tv_usec - (int64_t)start->tv_usec + 500 + 1000000) / 1000
      - 1000;

  /* Some compilers are smart enough to work out this is a no-op on L64 */
#if SIZEOF_LONG < 8
  if (mdiff > (int64_t)LONG_MAX || mdiff < (int64_t)LONG_MIN) {
    return LONG_MAX;
  }
#endif

  return (long)mdiff;
}

/**
 * Converts timeval to milliseconds.
 */
int64_t
tv_to_msec(const struct timeval *tv)
{
  int64_t conv = ((int64_t)tv->tv_sec)*1000L;
  /* Round ghetto-style */
  conv += ((int64_t)tv->tv_usec+500)/1000L;
  return conv;
}

/**
 * Return duration in seconds between time_t values
 * <b>t1</b> and <b>t2</b> iff <b>t1</b> is numerically
 * less or equal than <b>t2</b>. Otherwise, return TIME_MAX.
 *
 * This provides a safe way to compute difference between
 * two UNIX timestamps (<b>t2</b> can be assumed by calling
 * code to be later than <b>t1</b>) or two durations measured
 * in seconds (<b>t2</b> can be assumed to be longer than
 * <b>t1</b>). Calling code is expected to check for TIME_MAX
 * return value and interpret that as error condition.
 */
time_t
time_diff(const time_t t1, const time_t t2)
{
  if (t1 <= t2)
    return t2 - t1;

  return TIME_MAX;
}

