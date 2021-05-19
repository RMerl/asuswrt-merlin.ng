/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file time_to_tm.c
 * \brief Convert to struct tm, portably.
 **/

#include "orconfig.h"
#include "lib/cc/torint.h"
#include "lib/cc/compat_compiler.h"
#include "lib/wallclock/time_to_tm.h"
#include "lib/string/printf.h"
#include "lib/err/torerr.h"

#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#if !defined(_WIN32)
/** Defined iff we need to add locks when defining fake versions of reentrant
 * versions of time-related functions. */
#define TIME_FNS_NEED_LOCKS
#endif

/** Helper: Deal with confused or out-of-bounds values from localtime_r and
 * friends.  (On some platforms, they can give out-of-bounds values or can
 * return NULL.)  If <b>islocal</b>, this is a localtime result; otherwise
 * it's from gmtime.  The function returns <b>r</b>, when given <b>timep</b>
 * as its input. If we need to store new results, store them in
 * <b>resultbuf</b>. */
static struct tm *
correct_tm(int islocal, const time_t *timep, struct tm *resultbuf,
           struct tm *r, char **err_out)
{
  const char *outcome;

  if (PREDICT_LIKELY(r)) {
    /* We can't strftime dates after 9999 CE, and we want to avoid dates
     * before 1 CE (avoiding the year 0 issue and negative years). */
    if (r->tm_year > 8099) {
      r->tm_year = 8099;
      r->tm_mon = 11;
      r->tm_mday = 31;
      r->tm_yday = 364;
      r->tm_wday = 6;
      r->tm_hour = 23;
      r->tm_min = 59;
      r->tm_sec = 59;
    } else if (r->tm_year < (1-1900)) {
      r->tm_year = (1-1900);
      r->tm_mon = 0;
      r->tm_mday = 1;
      r->tm_yday = 0;
      r->tm_wday = 0;
      r->tm_hour = 0;
      r->tm_min = 0;
      r->tm_sec = 0;
    }
    return r;
  }

  /* If we get here, gmtime or localtime returned NULL. It might have done
   * this because of overrun or underrun, or it might have done it because of
   * some other weird issue. */
  if (timep) {
    if (*timep < 0) {
      r = resultbuf;
      r->tm_year = 70; /* 1970 CE */
      r->tm_mon = 0;
      r->tm_mday = 1;
      r->tm_yday = 0;
      r->tm_wday = 0;
      r->tm_hour = 0;
      r->tm_min = 0 ;
      r->tm_sec = 0;
      outcome = "Rounding up to 1970";
      goto done;
    } else if (*timep >= INT32_MAX) {
      /* Rounding down to INT32_MAX isn't so great, but keep in mind that we
       * only do it if gmtime/localtime tells us NULL. */
      r = resultbuf;
      r->tm_year = 137; /* 2037 CE */
      r->tm_mon = 11;
      r->tm_mday = 31;
      r->tm_yday = 364;
      r->tm_wday = 6;
      r->tm_hour = 23;
      r->tm_min = 59;
      r->tm_sec = 59;
      outcome = "Rounding down to 2037";
      goto done;
    }
  }

  /* If we get here, then gmtime/localtime failed without getting an extreme
   * value for *timep */
  /* LCOV_EXCL_START */
  r = resultbuf;
  memset(resultbuf, 0, sizeof(struct tm));
  outcome="can't recover";
  /* LCOV_EXCL_STOP */
 done:
  if (err_out) {
    tor_asprintf(err_out, "%s(%"PRId64") failed with error %s: %s",
                 islocal?"localtime":"gmtime",
                 timep?((int64_t)*timep):0,
                 strerror(errno),
                 outcome);
  }
  return r;
}

/** @{ */
/** As localtime_r, but defined for platforms that don't have it:
 *
 * Convert *<b>timep</b> to a struct tm in local time, and store the value in
 * *<b>result</b>.  Return the result on success, or NULL on failure.
 */
#ifdef HAVE_LOCALTIME_R
struct tm *
tor_localtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  r = localtime_r(timep, result);
  return correct_tm(1, timep, result, r, err_out);
}
#elif defined(TIME_FNS_NEED_LOCKS)
struct tm *
tor_localtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  static tor_mutex_t *m=NULL;
  if (!m) { m=tor_mutex_new(); }
  raw_assert(result);
  tor_mutex_acquire(m);
  r = localtime(timep);
  if (r)
    memcpy(result, r, sizeof(struct tm));
  tor_mutex_release(m);
  return correct_tm(1, timep, result, r, err_out);
}
#else
struct tm *
tor_localtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  raw_assert(result);
  r = localtime(timep);
  if (r)
    memcpy(result, r, sizeof(struct tm));
  return correct_tm(1, timep, result, r, err_out);
}
#endif /* defined(HAVE_LOCALTIME_R) || ... */
/** @} */

/** @{ */
/** As gmtime_r, but defined for platforms that don't have it:
 *
 * Convert *<b>timep</b> to a struct tm in UTC, and store the value in
 * *<b>result</b>.  Return the result on success, or NULL on failure.
 */
#ifdef HAVE_GMTIME_R
struct tm *
tor_gmtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  r = gmtime_r(timep, result);
  return correct_tm(0, timep, result, r, err_out);
}
#elif defined(TIME_FNS_NEED_LOCKS)
struct tm *
tor_gmtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  static tor_mutex_t *m=NULL;
  if (!m) { m=tor_mutex_new(); }
  raw_assert(result);
  tor_mutex_acquire(m);
  r = gmtime(timep);
  if (r)
    memcpy(result, r, sizeof(struct tm));
  tor_mutex_release(m);
  return correct_tm(0, timep, result, r, err_out);
}
#else
struct tm *
tor_gmtime_r_msg(const time_t *timep, struct tm *result, char **err_out)
{
  struct tm *r;
  raw_assert(result);
  r = gmtime(timep);
  if (r)
    memcpy(result, r, sizeof(struct tm));
  return correct_tm(0, timep, result, r, err_out);
}
#endif /* defined(HAVE_GMTIME_R) || ... */
/**@}*/
