/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file time_fmt.c
 *
 * \brief Encode and decode time in various formats.
 *
 * This module is higher-level than the conversion functions in "wallclock",
 * and handles a larger variety of types.  It converts between different time
 * formats, and encodes and decodes them from strings.
 **/

#include "lib/encoding/time_fmt.h"
#include "lib/log/log.h"
#include "lib/log/escape.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"
#include "lib/string/scanf.h"
#include "lib/wallclock/time_to_tm.h"

#include <string.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef _WIN32
/* For struct timeval */
#include <winsock2.h>
#endif

/** As localtime_r, but defined for platforms that don't have it:
 *
 * Convert *<b>timep</b> to a struct tm in local time, and store the value in
 * *<b>result</b>.  Return the result on success, or NULL on failure.
 *
 * Treat malformatted inputs localtime outputs as a BUG.
 */
struct tm *
tor_localtime_r(const time_t *timep, struct tm *result)
{
  char *err = NULL;
  struct tm *r = tor_localtime_r_msg(timep, result, &err);
  if (err) {
    log_warn(LD_BUG, "%s", err);
    tor_free(err);
  }
  return r;
}

/** As gmtime_r, but defined for platforms that don't have it:
 *
 * Convert *<b>timep</b> to a struct tm in UTC, and store the value in
 * *<b>result</b>.  Return the result on success, or NULL on failure.
 *
 * Treat malformatted inputs or gmtime outputs as a BUG.
 */
struct tm *
tor_gmtime_r(const time_t *timep, struct tm *result)
{
  char *err = NULL;
  struct tm *r = tor_gmtime_r_msg(timep, result, &err);
  if (err) {
    log_warn(LD_BUG, "%s", err);
    tor_free(err);
  }
  return r;
}

/** Yield true iff <b>y</b> is a leap-year. */
#define IS_LEAPYEAR(y) (!(y % 4) && ((y % 100) || !(y % 400)))
/** Helper: Return the number of leap-days between Jan 1, y1 and Jan 1, y2. */
static int
n_leapdays(int year1, int year2)
{
  --year1;
  --year2;
  return (year2/4 - year1/4) - (year2/100 - year1/100)
    + (year2/400 - year1/400);
}
/** Number of days per month in non-leap year; used by tor_timegm and
 * parse_rfc1123_time. */
static const int days_per_month[] =
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/** Compute a time_t given a struct tm.  The result is given in UTC, and
 * does not account for leap seconds.  Return 0 on success, -1 on failure.
 */
int
tor_timegm(const struct tm *tm, time_t *time_out)
{
  /* This is a pretty ironclad timegm implementation, snarfed from Python2.2.
   * It's way more brute-force than fiddling with tzset().
   *
   * We use int64_t rather than time_t to avoid overflow on multiplication on
   * platforms with 32-bit time_t. Since year is clipped to INT32_MAX, and
   * since 365 * 24 * 60 * 60 is approximately 31 million, it's not possible
   * for INT32_MAX years to overflow int64_t when converted to seconds. */
  int64_t year, days, hours, minutes, seconds;
  int i, invalid_year, dpm;

  /* Initialize time_out to 0 for now, to avoid bad usage in case this function
     fails and the caller ignores the return value. */
  tor_assert(time_out);
  *time_out = 0;

  /* avoid int overflow on addition */
  if (tm->tm_year < INT32_MAX-1900) {
    year = tm->tm_year + 1900;
  } else {
    /* clamp year */
    year = INT32_MAX;
  }
  invalid_year = (year < 1970 || tm->tm_year >= INT32_MAX-1900);

  if (tm->tm_mon >= 0 && tm->tm_mon <= 11) {
    dpm = days_per_month[tm->tm_mon];
    if (tm->tm_mon == 1 && !invalid_year && IS_LEAPYEAR(tm->tm_year)) {
      dpm = 29;
    }
  } else {
    /* invalid month - default to 0 days per month */
    dpm = 0;
  }

  if (invalid_year ||
      tm->tm_mon < 0 || tm->tm_mon > 11 ||
      tm->tm_mday < 1 || tm->tm_mday > dpm ||
      tm->tm_hour < 0 || tm->tm_hour > 23 ||
      tm->tm_min < 0 || tm->tm_min > 59 ||
      tm->tm_sec < 0 || tm->tm_sec > 60) {
    log_warn(LD_BUG, "Out-of-range argument to tor_timegm");
    return -1;
  }
  days = 365 * (year-1970) + n_leapdays(1970,(int)year);
  for (i = 0; i < tm->tm_mon; ++i)
    days += days_per_month[i];
  if (tm->tm_mon > 1 && IS_LEAPYEAR(year))
    ++days;
  days += tm->tm_mday - 1;
  hours = days*24 + tm->tm_hour;

  minutes = hours*60 + tm->tm_min;
  seconds = minutes*60 + tm->tm_sec;
  /* Check that "seconds" will fit in a time_t. On platforms where time_t is
   * 32-bit, this check will fail for dates in and after 2038.
   *
   * We already know that "seconds" can't be negative because "year" >= 1970 */
#if SIZEOF_TIME_T < 8
  if (seconds < TIME_MIN || seconds > TIME_MAX) {
    log_warn(LD_BUG, "Result does not fit in tor_timegm");
    return -1;
  }
#endif /* SIZEOF_TIME_T < 8 */
  *time_out = (time_t)seconds;
  return 0;
}

/* strftime is locale-specific, so we need to replace those parts */

/** A c-locale array of 3-letter names of weekdays, starting with Sun. */
static const char *WEEKDAY_NAMES[] =
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
/** A c-locale array of 3-letter names of months, starting with Jan. */
static const char *MONTH_NAMES[] =
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/** Set <b>buf</b> to the RFC1123 encoding of the UTC value of <b>t</b>.
 * The buffer must be at least RFC1123_TIME_LEN+1 bytes long.
 *
 * (RFC1123 format is "Fri, 29 Sep 2006 15:54:20 GMT". Note the "GMT"
 * rather than "UTC".)
 */
void
format_rfc1123_time(char *buf, time_t t)
{
  struct tm tm;

  tor_gmtime_r(&t, &tm);

  strftime(buf, RFC1123_TIME_LEN+1, "___, %d ___ %Y %H:%M:%S GMT", &tm);
  tor_assert(tm.tm_wday >= 0);
  tor_assert(tm.tm_wday <= 6);
  memcpy(buf, WEEKDAY_NAMES[tm.tm_wday], 3);
  tor_assert(tm.tm_mon >= 0);
  tor_assert(tm.tm_mon <= 11);
  memcpy(buf+8, MONTH_NAMES[tm.tm_mon], 3);
}

/** Parse the (a subset of) the RFC1123 encoding of some time (in UTC) from
 * <b>buf</b>, and store the result in *<b>t</b>.
 *
 * Note that we only accept the subset generated by format_rfc1123_time above,
 * not the full range of formats suggested by RFC 1123.
 *
 * Return 0 on success, -1 on failure.
*/
int
parse_rfc1123_time(const char *buf, time_t *t)
{
  struct tm tm;
  char month[4];
  char weekday[4];
  int i, m, invalid_year;
  unsigned tm_mday, tm_year, tm_hour, tm_min, tm_sec;
  unsigned dpm;

  if (strlen(buf) != RFC1123_TIME_LEN)
    return -1;
  memset(&tm, 0, sizeof(tm));
  if (tor_sscanf(buf, "%3s, %2u %3s %u %2u:%2u:%2u GMT", weekday,
             &tm_mday, month, &tm_year, &tm_hour,
             &tm_min, &tm_sec) < 7) {
    char *esc = esc_for_log(buf);
    log_warn(LD_GENERAL, "Got invalid RFC1123 time %s", esc);
    tor_free(esc);
    return -1;
  }

  m = -1;
  for (i = 0; i < 12; ++i) {
    if (!strcmp(month, MONTH_NAMES[i])) {
      m = i;
      break;
    }
  }
  if (m<0) {
    char *esc = esc_for_log(buf);
    log_warn(LD_GENERAL, "Got invalid RFC1123 time %s: No such month", esc);
    tor_free(esc);
    return -1;
  }
  tm.tm_mon = m;

  invalid_year = (tm_year >= INT32_MAX || tm_year < 1970);
  tor_assert(m >= 0 && m <= 11);
  dpm = days_per_month[m];
  if (m == 1 && !invalid_year && IS_LEAPYEAR(tm_year)) {
    dpm = 29;
  }

  if (invalid_year || tm_mday < 1 || tm_mday > dpm ||
      tm_hour > 23 || tm_min > 59 || tm_sec > 60) {
    char *esc = esc_for_log(buf);
    log_warn(LD_GENERAL, "Got invalid RFC1123 time %s", esc);
    tor_free(esc);
    return -1;
  }
  tm.tm_mday = (int)tm_mday;
  tm.tm_year = (int)tm_year;
  tm.tm_hour = (int)tm_hour;
  tm.tm_min = (int)tm_min;
  tm.tm_sec = (int)tm_sec;

  if (tm.tm_year < 1970) {
    /* LCOV_EXCL_START
     * XXXX I think this is dead code; we already checked for
     *      invalid_year above. */
    tor_assert_nonfatal_unreached();
    char *esc = esc_for_log(buf);
    log_warn(LD_GENERAL,
             "Got invalid RFC1123 time %s. (Before 1970)", esc);
    tor_free(esc);
    return -1;
    /* LCOV_EXCL_STOP */
  }
  tm.tm_year -= 1900;

  return tor_timegm(&tm, t);
}

/** Set <b>buf</b> to the ISO8601 encoding of the local value of <b>t</b>.
 * The buffer must be at least ISO_TIME_LEN+1 bytes long.
 *
 * (ISO8601 format is 2006-10-29 10:57:20)
 */
void
format_local_iso_time(char *buf, time_t t)
{
  struct tm tm;
  strftime(buf, ISO_TIME_LEN+1, "%Y-%m-%d %H:%M:%S", tor_localtime_r(&t, &tm));
}

/** Set <b>buf</b> to the ISO8601 encoding of the GMT value of <b>t</b>.
 * The buffer must be at least ISO_TIME_LEN+1 bytes long.
 */
void
format_iso_time(char *buf, time_t t)
{
  struct tm tm;
  strftime(buf, ISO_TIME_LEN+1, "%Y-%m-%d %H:%M:%S", tor_gmtime_r(&t, &tm));
}

/** As format_local_iso_time, but use the yyyy-mm-ddThh:mm:ss format to avoid
 * embedding an internal space. */
void
format_local_iso_time_nospace(char *buf, time_t t)
{
  format_local_iso_time(buf, t);
  buf[10] = 'T';
}

/** As format_iso_time, but use the yyyy-mm-ddThh:mm:ss format to avoid
 * embedding an internal space. */
void
format_iso_time_nospace(char *buf, time_t t)
{
  format_iso_time(buf, t);
  buf[10] = 'T';
}

/** As format_iso_time_nospace, but include microseconds in decimal
 * fixed-point format.  Requires that buf be at least ISO_TIME_USEC_LEN+1
 * bytes long. */
void
format_iso_time_nospace_usec(char *buf, const struct timeval *tv)
{
  tor_assert(tv);
  format_iso_time_nospace(buf, (time_t)tv->tv_sec);
  tor_snprintf(buf+ISO_TIME_LEN, 8, ".%06d", (int)tv->tv_usec);
}

/** Given an ISO-formatted UTC time value (after the epoch) in <b>cp</b>,
 * parse it and store its value in *<b>t</b>.  Return 0 on success, -1 on
 * failure.  Ignore extraneous stuff in <b>cp</b> after the end of the time
 * string, unless <b>strict</b> is set. If <b>nospace</b> is set,
 * expect the YYYY-MM-DDTHH:MM:SS format. */
int
parse_iso_time_(const char *cp, time_t *t, int strict, int nospace)
{
  struct tm st_tm;
  unsigned int year=0, month=0, day=0, hour=0, minute=0, second=0;
  int n_fields;
  char extra_char, separator_char;
  n_fields = tor_sscanf(cp, "%u-%2u-%2u%c%2u:%2u:%2u%c",
                        &year, &month, &day,
                        &separator_char,
                        &hour, &minute, &second, &extra_char);
  if (strict ? (n_fields != 7) : (n_fields < 7)) {
    char *esc = esc_for_log(cp);
    log_warn(LD_GENERAL, "ISO time %s was unparseable", esc);
    tor_free(esc);
    return -1;
  }
  if (separator_char != (nospace ? 'T' : ' ')) {
    char *esc = esc_for_log(cp);
    log_warn(LD_GENERAL, "ISO time %s was unparseable", esc);
    tor_free(esc);
    return -1;
  }
  if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
          hour > 23 || minute > 59 || second > 60 || year >= INT32_MAX) {
    char *esc = esc_for_log(cp);
    log_warn(LD_GENERAL, "ISO time %s was nonsensical", esc);
    tor_free(esc);
    return -1;
  }
  st_tm.tm_year = (int)year-1900;
  st_tm.tm_mon = month-1;
  st_tm.tm_mday = day;
  st_tm.tm_hour = hour;
  st_tm.tm_min = minute;
  st_tm.tm_sec = second;
  st_tm.tm_wday = 0; /* Should be ignored. */

  if (st_tm.tm_year < 70) {
    /* LCOV_EXCL_START
     * XXXX I think this is dead code; we already checked for
     *      year < 1970 above. */
    tor_assert_nonfatal_unreached();
    char *esc = esc_for_log(cp);
    log_warn(LD_GENERAL, "Got invalid ISO time %s. (Before 1970)", esc);
    tor_free(esc);
    return -1;
    /* LCOV_EXCL_STOP */
  }
  return tor_timegm(&st_tm, t);
}

/** Given an ISO-formatted UTC time value (after the epoch) in <b>cp</b>,
 * parse it and store its value in *<b>t</b>.  Return 0 on success, -1 on
 * failure. Reject the string if any characters are present after the time.
 */
int
parse_iso_time(const char *cp, time_t *t)
{
  return parse_iso_time_(cp, t, 1, 0);
}

/**
 * As parse_iso_time, but parses a time encoded by format_iso_time_nospace().
 */
int
parse_iso_time_nospace(const char *cp, time_t *t)
{
  return parse_iso_time_(cp, t, 1, 1);
}

/** Given a <b>date</b> in one of the three formats allowed by HTTP (ugh),
 * parse it into <b>tm</b>.  Return 0 on success, negative on failure. */
int
parse_http_time(const char *date, struct tm *tm)
{
  const char *cp;
  char month[4];
  char wkday[4];
  int i;
  unsigned tm_mday, tm_year, tm_hour, tm_min, tm_sec;

  tor_assert(tm);
  memset(tm, 0, sizeof(*tm));

  /* First, try RFC1123 or RFC850 format: skip the weekday.  */
  if ((cp = strchr(date, ','))) {
    ++cp;
    if (*cp != ' ')
      return -1;
    ++cp;
    if (tor_sscanf(cp, "%2u %3s %4u %2u:%2u:%2u GMT",
               &tm_mday, month, &tm_year,
               &tm_hour, &tm_min, &tm_sec) == 6) {
      /* rfc1123-date */
      tm_year -= 1900;
    } else if (tor_sscanf(cp, "%2u-%3s-%2u %2u:%2u:%2u GMT",
                      &tm_mday, month, &tm_year,
                      &tm_hour, &tm_min, &tm_sec) == 6) {
      /* rfc850-date */
    } else {
      return -1;
    }
  } else {
    /* No comma; possibly asctime() format. */
    if (tor_sscanf(date, "%3s %3s %2u %2u:%2u:%2u %4u",
               wkday, month, &tm_mday,
               &tm_hour, &tm_min, &tm_sec, &tm_year) == 7) {
      tm_year -= 1900;
    } else {
      return -1;
    }
  }
  tm->tm_mday = (int)tm_mday;
  tm->tm_year = (int)tm_year;
  tm->tm_hour = (int)tm_hour;
  tm->tm_min = (int)tm_min;
  tm->tm_sec = (int)tm_sec;
  tm->tm_wday = 0; /* Leave this unset. */

  month[3] = '\0';
  /* Okay, now decode the month. */
  /* set tm->tm_mon to dummy value so the check below fails. */
  tm->tm_mon = -1;
  for (i = 0; i < 12; ++i) {
    if (!strcasecmp(MONTH_NAMES[i], month)) {
      tm->tm_mon = i;
    }
  }

  if (tm->tm_year < 0 ||
      tm->tm_mon < 0  || tm->tm_mon > 11 ||
      tm->tm_mday < 1 || tm->tm_mday > 31 ||
      tm->tm_hour < 0 || tm->tm_hour > 23 ||
      tm->tm_min < 0  || tm->tm_min > 59 ||
      tm->tm_sec < 0  || tm->tm_sec > 60)
    return -1; /* Out of range, or bad month. */

  return 0;
}

/** Given an <b>interval</b> in seconds, try to write it to the
 * <b>out_len</b>-byte buffer in <b>out</b> in a human-readable form.
 * Returns a non-negative integer on success, -1 on failure.
 */
int
format_time_interval(char *out, size_t out_len, long interval)
{
  /* We only report seconds if there's no hours. */
  long sec = 0, min = 0, hour = 0, day = 0;

  /* -LONG_MIN is LONG_MAX + 1, which causes signed overflow */
  if (interval < -LONG_MAX)
    interval = LONG_MAX;
  else if (interval < 0)
    interval = -interval;

  if (interval >= 86400) {
    day = interval / 86400;
    interval %= 86400;
  }
  if (interval >= 3600) {
    hour = interval / 3600;
    interval %= 3600;
  }
  if (interval >= 60) {
    min = interval / 60;
    interval %= 60;
  }
  sec = interval;

  if (day) {
    return tor_snprintf(out, out_len, "%ld days, %ld hours, %ld minutes",
                        day, hour, min);
  } else if (hour) {
    return tor_snprintf(out, out_len, "%ld hours, %ld minutes", hour, min);
  } else if (min) {
    return tor_snprintf(out, out_len, "%ld minutes, %ld seconds", min, sec);
  } else {
    return tor_snprintf(out, out_len, "%ld seconds", sec);
  }
}
