/*======================================================================
 FILE: icaltime.c
 CREATOR: eric 02 June 2000

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 The timegm code is Copyright (c) 2001-2006, NLnet Labs. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom

 The timegm code in this file is copyrighted by NLNET LABS 2001-2006,
 according to the following conditions:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.

 * Neither the name of the NLNET LABS nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icaltime.h"
#include "astime.h"
#include "icalerror.h"
#include "icalmemory.h"
#include "icaltimezone.h"

#include <ctype.h>
#include <stdlib.h>

/* The first array is for non-leap years, the second for leap years*/
static const int days_in_year_passed_month[2][13] = {
    /* jan feb mar  apr  may  jun  jul  aug  sep  oct  nov  dec */
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static int icaltime_leap_days(int y1, int y2)
{
    --y1;
    --y2;
    return (y2 / 4 - y1 / 4) - (y2 / 100 - y1 / 100) + (y2 / 400 - y1 / 400);
}

/*
 * Code adapted from Python 2.4.1 sources (Lib/calendar.py).
 */
static time_t icaltime_timegm(const struct tm *tm)
{
    int year;
    time_t days;
    time_t hours;
    time_t minutes;
    time_t seconds;

    year = 1900 + tm->tm_year;
    days = (time_t)(365 * (year - 1970) + icaltime_leap_days(1970, year));
    days += days_in_year_passed_month[0][tm->tm_mon];

    if (tm->tm_mon > 1 && icaltime_is_leap_year(year))
        ++days;

    days += tm->tm_mday - 1;
    hours = days * 24 + tm->tm_hour;
    minutes = hours * 60 + tm->tm_min;
    seconds = minutes * 60 + tm->tm_sec;

    return seconds;
}

/*
 *  Function to convert a struct tm time specification
 *  to an ANSI time_t using the specified time zone.
 *  This is different from the standard mktime() function
 *  in that we don't want the automatic adjustments for
 *  local daylight savings time applied to the result.
 *  This function expects well-formed input.
 */
static time_t make_time(struct tm *tm, int tzm)
{
    time_t tim;

    static int days[] = { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364 };

    /* check that month specification within range */

    if (tm->tm_mon < 0 || tm->tm_mon > 11)
        return ((time_t) - 1);

#if (SIZEOF_TIME_T == 4)
    /* check that year specification within range */

    if (tm->tm_year < 70 || tm->tm_year > 138)
        return ((time_t) - 1);

    /* check for upper bound of Jan 17, 2038 (to avoid possibility of
       32-bit arithmetic overflow) */

    if (tm->tm_year == 138) {
        if (tm->tm_mon > 0) {
            return ((time_t) - 1);
        } else if (tm->tm_mday > 17) {
            return ((time_t) - 1);
        }
    }
#endif /* SIZEOF_TIME_T */

    /*
     *  calculate elapsed days since start of the epoch (midnight Jan
     *  1st, 1970 UTC) 17 = number of leap years between 1900 and 1970
     *  (number of leap days to subtract)
     */

    tim = (time_t) ((tm->tm_year - 70) * 365 + ((tm->tm_year - 1) / 4) - 17);

    /* add number of days elapsed in the current year */

    tim += days[tm->tm_mon];

    /* check and adjust for leap years (the leap year check only valid
       during the 32-bit era */

    if ((tm->tm_year & 3) == 0 && tm->tm_mon > 1)
        tim += 1;

    /* elapsed days to current date */

    tim += tm->tm_mday;

    /* calculate elapsed hours since start of the epoch */

    tim = tim * 24 + tm->tm_hour;

    /* calculate elapsed minutes since start of the epoch */

    tim = tim * 60 + tm->tm_min;

    /* adjust per time zone specification */

    tim -= tzm;

    /* calculate elapsed seconds since start of the epoch */

    tim = tim * 60 + tm->tm_sec;

    /* return number of seconds since start of the epoch */

    return (tim);
}

/**     @brief Constructor (deprecated).
 *
 * Convert seconds past UNIX epoch to a timetype.
 *
 * @deprecated This constructor is deprecated and shouldn't be used in
 *   new software.  Use icaltime_from_timet_with_zone(time_t, int,
 *   icaltimezone *) instead.  In the meantime, calls to this method
 *   return a floating time, which can always be converted to a local
 *   time with an appropriate call to icaltime_convert_to_zone().
 */

struct icaltimetype icaltime_from_timet(const time_t tm, const int is_date)
{
#if NO_WARN_DEPRECATED == 0
    icalerror_warn(
        "icaltime_from_timet() is DEPRECATED, use icaltime_from_timet_with_zone() instead");
#endif

    return icaltime_from_timet_with_zone(tm, is_date, 0);
}

/**     @brief Constructor.
 *
 *      @param tm The time
 *      @param is_date Boolean: 1 means we should treat tm as a DATE
 *      @param zone The timezone tm is in, NULL means to treat tm as a
 *              floating time
 *
 *      Return a new icaltime instance, initialized to the given time
 *      expressed as seconds past UNIX epoch, optionally using the given
 *      timezone.
 *
 *      If the caller specifies the is_date param as TRUE, the returned
 *      object is of DATE type, otherwise the input is meant to be of
 *      DATE-TIME type.
 *      If the zone is not specified (NULL zone param) the time is taken
 *      to be floating, that is, valid in any timezone. Note that, in
 *      addition to the uses specified in [RFC5545], this can be used
 *      when doing simple math on couples of times.
 *      If the zone is specified (UTC or otherwise), it's stored in the
 *      object and it's used as the native timezone for this object.
 *      This means that the caller can convert this time to a different
 *      target timezone with no need to store the source timezone.
 *
 */
struct icaltimetype icaltime_from_timet_with_zone(const time_t tm, const int is_date,
                                                  const icaltimezone *zone)
{
    struct icaltimetype tt;
    struct tm t;
    icaltimezone *utc_zone;

    utc_zone = icaltimezone_get_utc_timezone();

    /* Convert the time_t to a struct tm in UTC time. We can trust gmtime for this. */
    gmtime_r(&tm, &t);

    tt.year = t.tm_year + 1900;
    tt.month = t.tm_mon + 1;
    tt.day = t.tm_mday;
    tt.hour = t.tm_hour;
    tt.minute = t.tm_min;
    tt.second = t.tm_sec;
    tt.is_date = 0;
    tt.is_utc = (zone == utc_zone) ? 1 : 0;
    tt.is_daylight = 0;
    tt.zone = NULL;

    /* Use our timezone functions to convert to the required timezone. */
    icaltimezone_convert_time(&tt, utc_zone, (icaltimezone *) zone);

    tt.is_date = is_date;

    /* If it is a DATE value, make sure hour, minute & second are 0. */
    if (is_date) {
        tt.hour = 0;
        tt.minute = 0;
        tt.second = 0;
    }

    return tt;
}

/**     @brief Convenience constructor.
 *
 * Returns the current time in the given timezone, as an icaltimetype.
 */
struct icaltimetype icaltime_current_time_with_zone(const icaltimezone *zone)
{
    return icaltime_from_timet_with_zone(time(NULL), 0, zone);
}

/**     @brief Convenience constructor.
 *
 * Returns the current day as an icaltimetype, with is_date set.
 */
struct icaltimetype icaltime_today(void)
{
    return icaltime_from_timet_with_zone(time(NULL), 1, NULL);
}

/**     @brief  Return the time as seconds past the UNIX epoch
 *
 *      While this function is not currently deprecated, it probably won't do
 *      what you expect, unless you know what you're doing. In particular, you
 *      should only pass an icaltime in UTC, since no conversion is done. Even
 *      in that case, it's probably better to just use
 *      icaltime_as_timet_with_zone().
 */
time_t icaltime_as_timet(const struct icaltimetype tt)
{
    struct tm stm;
    time_t t;

    /* If the time is the special null time, return 0. */
    if (icaltime_is_null_time(tt)) {
        return 0;
    }

    /* Copy the icaltimetype to a struct tm. */
    memset(&stm, 0, sizeof(struct tm));

    if (icaltime_is_date(tt)) {
        stm.tm_sec = stm.tm_min = stm.tm_hour = 0;
    } else {
        stm.tm_sec = tt.second;
        stm.tm_min = tt.minute;
        stm.tm_hour = tt.hour;
    }

    stm.tm_mday = tt.day;
    stm.tm_mon = tt.month - 1;
    stm.tm_year = tt.year - 1900;
    stm.tm_isdst = -1;

    t = make_time(&stm, 0);

    return t;
}

/**     Return the time as seconds past the UNIX epoch, using the
 *      given timezone.
 *
 *      This convenience method combines a call to icaltime_convert_to_zone()
 *      with a call to icaltime_as_timet().
 *      If the input timezone is null, no conversion is done; that is, the
 *      time is simply returned as time_t in its native timezone.
 */
time_t icaltime_as_timet_with_zone(const struct icaltimetype tt, const icaltimezone *zone)
{
    icaltimezone *utc_zone;
    struct tm stm;
    time_t t;
    struct icaltimetype local_tt;

    utc_zone = icaltimezone_get_utc_timezone();

    /* If the time is the special null time, return 0. */
    if (icaltime_is_null_time(tt)) {
        return 0;
    }

    local_tt = tt;

    /* Clear the is_date flag, so we can convert the time. */
    local_tt.is_date = 0;

    /* Use our timezone functions to convert to UTC. */
    icaltimezone_convert_time(&local_tt, (icaltimezone *) zone, utc_zone);

    /* Copy the icaltimetype to a struct tm. */
    memset(&stm, 0, sizeof(struct tm));

    stm.tm_sec = local_tt.second;
    stm.tm_min = local_tt.minute;
    stm.tm_hour = local_tt.hour;
    stm.tm_mday = local_tt.day;
    stm.tm_mon = local_tt.month - 1;
    stm.tm_year = local_tt.year - 1900;
    stm.tm_isdst = -1;

    t = icaltime_timegm(&stm);

    return t;
}

/**
 * Return a string represention of the time, in RFC5545 format. The
 * string is owned by libical.
 */
const char *icaltime_as_ical_string(const struct icaltimetype tt)
{
    char *buf;

    buf = icaltime_as_ical_string_r(tt);
    icalmemory_add_tmp_buffer(buf);
    return buf;
}

/**
 * Return a string represention of the time, in RFC5545 format. The
 * string is owned by the caller.
 */
char *icaltime_as_ical_string_r(const struct icaltimetype tt)
{
    size_t size = 17;
    char *buf = icalmemory_new_buffer(size);

    if (tt.is_date) {
        snprintf(buf, size, "%04d%02d%02d", tt.year, tt.month, tt.day);
    } else {
        const char *fmt;

        if (tt.is_utc) {
            fmt = "%04d%02d%02dT%02d%02d%02dZ";
        } else {
            fmt = "%04d%02d%02dT%02d%02d%02d";
        }
        snprintf(buf, size, fmt, tt.year, tt.month, tt.day, tt.hour, tt.minute, tt.second);
    }

    return buf;
}

/**
 *      Reset all of the time components to be in their normal ranges. For
 *      instance, given a time with minutes=70, the minutes will be reduces
 *      to 10, and the hour incremented. This allows the caller to do
 *      arithmetic on times without worrying about overflow or
 *      underflow.
 *
 *      Implementation note: we call icaltime_adjust() with no adjustment.
 */
struct icaltimetype icaltime_normalize(const struct icaltimetype tt)
{
    struct icaltimetype ret = tt;

    icaltime_adjust(&ret, 0, 0, 0, 0);
    return ret;
}

/**     @brief Contructor.
 *
 * Create a time from an ISO format string.
 *
 * @todo If the given string specifies a DATE-TIME not in UTC, there
 *       is no way to know if this is a floating time or really refers to a
 *       timezone. We should probably add a new constructor:
 *       icaltime_from_string_with_zone()
 */
struct icaltimetype icaltime_from_string(const char *str)
{
    struct icaltimetype tt = icaltime_null_time();
    size_t size;

    icalerror_check_arg_re(str != 0, "str", icaltime_null_time());

    size = strlen(str);

    if ((size == 15) || (size == 19)) { /* floating time with/without separators */
        tt.is_utc = 0;
        tt.is_date = 0;
    } else if ((size == 16) || (size == 20)) {  /* UTC time, ends in 'Z' */
        if ((str[15] != 'Z') && (str[19] != 'Z'))
            goto FAIL;

        tt.is_utc = 1;
        tt.zone = icaltimezone_get_utc_timezone();
        tt.is_date = 0;
    } else if ((size == 8) || (size == 10)) {   /* A DATE */
        tt.is_utc = 0;
        tt.is_date = 1;
    } else {    /* error */
        goto FAIL;
    }

    if (tt.is_date == 1) {
        if (size == 10) {
            char dsep1, dsep2;

            if (sscanf(str, "%04d%c%02d%c%02d",
                       &tt.year, &dsep1, &tt.month, &dsep2, &tt.day) < 5) {
                goto FAIL;
            }

            if ((dsep1 != '-') || (dsep2 != '-')) {
                goto FAIL;
            }
        } else if (sscanf(str, "%04d%02d%02d", &tt.year, &tt.month, &tt.day) < 3) {
            goto FAIL;
        }
    } else {
        if (size > 16) {
            char dsep1, dsep2, tsep, tsep1, tsep2;

            if (sscanf(str, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d",
                       &tt.year, &dsep1, &tt.month, &dsep2, &tt.day, &tsep,
                       &tt.hour, &tsep1, &tt.minute, &tsep2, &tt.second) < 11) {
                goto FAIL;
            }

            if ((tsep != 'T') ||
                (dsep1 != '-') || (dsep2 != '-') ||
                (tsep1 != ':') || (tsep2 != ':')) {
                goto FAIL;
            }

        } else {
            char tsep;

            if (sscanf(str, "%04d%02d%02d%c%02d%02d%02d",
                       &tt.year, &tt.month, &tt.day, &tsep,
                       &tt.hour, &tt.minute, &tt.second) < 7) {
                goto FAIL;
            }

            if (tsep != 'T') {
                goto FAIL;
            }
        }
    }

    return tt;

  FAIL:
    icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
    return icaltime_null_time();
}

/* Returns whether the specified year is a leap year. Year is the normal year,
   e.g. 2001. */
int icaltime_is_leap_year(const int year)
{
    if (year <= 1752) {
        return (year % 4 == 0);
    } else {
        return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
    }
}

int icaltime_days_in_year(const int year)
{
    if (icaltime_is_leap_year(year)) {
        return 366;
    } else {
        return 365;
    }
}

static int _days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int icaltime_days_in_month(const int month, const int year)
{
    int days = _days_in_month[month];

/* The old code aborting if it was passed a parameter like BYMONTH=0
 * Unfortunately it's not practical right now to pass an error all
 * the way up the stack, so instead of aborting we're going to apply
 * the GIGO principle and simply return '30 days' if we get an
 * invalid month.  Modern applications cannot tolerate crashing.
 *  assert(month > 0);
 *  assert(month <= 12);
 */
    if ((month < 1) || (month > 12)) {
        return 30;
    }

    if (month == 2) {
        days += icaltime_is_leap_year(year);
    }

    return days;
}

/* 1-> Sunday, 7->Saturday */
int icaltime_day_of_week(const struct icaltimetype t)
{
    UTinstant jt;

    memset(&jt, 0, sizeof(UTinstant));

    jt.year = t.year;
    jt.month = t.month;
    jt.day = t.day;
    jt.i_hour = 0;
    jt.i_minute = 0;
    jt.i_second = 0;

    (void)juldat(&jt);

    return jt.weekday + 1;
}

/** Day of the year that the first day of the week (Sunday) is on.
 */
int icaltime_start_doy_week(const struct icaltimetype t, int fdow)
{
    UTinstant jt;
    int delta;

    memset(&jt, 0, sizeof(UTinstant));

    jt.year = t.year;
    jt.month = t.month;
    jt.day = t.day;
    jt.i_hour = 0;
    jt.i_minute = 0;
    jt.i_second = 0;

    (void)juldat(&jt);
    (void)caldat(&jt);

    delta = jt.weekday - (fdow - 1);
    if (delta < 0) {
        delta += 7;
    }
    return jt.day_of_year - delta;
}

/** Day of the year that the first day of the week (Sunday) is on.
 *
 *  @deprecated Doesn't take into account different week start days.
 */
int icaltime_start_doy_of_week(const struct icaltimetype t)
{
#if NO_WARN_DEPRECATED == 0
    icalerror_warn(
        "icaltime_start_doy_of_week() is DEPRECATED, use icaltime_start_doy_week() instead");
#endif

    return icaltime_start_doy_week(t, 1);
}

/**
 * @todo Doesn't take into account the start day of the
 * week. strftime assumes that weeks start on Monday.
 */
int icaltime_week_number(const struct icaltimetype ictt)
{
    UTinstant jt;

    memset(&jt, 0, sizeof(UTinstant));

    jt.year = ictt.year;
    jt.month = ictt.month;
    jt.day = ictt.day;
    jt.i_hour = 0;
    jt.i_minute = 0;
    jt.i_second = 0;

    (void)juldat(&jt);
    (void)caldat(&jt);

    return (jt.day_of_year - jt.weekday) / 7;
}

/**
 *      Returns the day of the year, counting from 1 (Jan 1st).
 */
int icaltime_day_of_year(const struct icaltimetype t)
{
    int is_leap = icaltime_is_leap_year(t.year);

    return days_in_year_passed_month[is_leap][t.month - 1] + t.day;
}

/**     @brief Contructor.
 *
 *      Create a new time, given a day of year and a year.
 */
/* Jan 1 is day #1, not 0 */
struct icaltimetype icaltime_from_day_of_year(const int _doy, const int _year)
{
    struct icaltimetype tt = icaltime_null_date();
    int is_leap;
    int month;
    int doy = _doy;
    int year = _year;

    is_leap = icaltime_is_leap_year(year);

    /* Zero and neg numbers represent days  of the previous year */
    if (doy < 1) {
        year--;
        is_leap = icaltime_is_leap_year(year);
        doy += days_in_year_passed_month[is_leap][12];
    } else if (doy > days_in_year_passed_month[is_leap][12]) {
        /* Move on to the next year */
        is_leap = icaltime_is_leap_year(year);
        doy -= days_in_year_passed_month[is_leap][12];
        year++;
    }

    tt.year = year;

    for (month = 11; month >= 0; month--) {
        if (doy > days_in_year_passed_month[is_leap][month]) {
            tt.month = month + 1;
            tt.day = doy - days_in_year_passed_month[is_leap][month];
            break;
        }
    }

    return tt;
}

/**     @brief Constructor.
 *
 *      Return a null time, which indicates no time has been set.
 *      This time represents the beginning of the epoch.
 */
struct icaltimetype icaltime_null_time(void)
{
    struct icaltimetype t;

    memset(&t, 0, sizeof(struct icaltimetype));

    return t;
}

/**     @brief Constructor.
 *
 *      Return a null date, which indicates no time has been set.
 */
struct icaltimetype icaltime_null_date(void)
{
    struct icaltimetype t;

    memset(&t, 0, sizeof(struct icaltimetype));

    t.is_date = 1;

    /*
     * Init to -1 to match what icalyacc.y used to do.
     * Does anything depend on this?
     */
    t.hour = -1;
    t.minute = -1;
    t.second = -1;

    return t;
}

/**
 *      Returns false if the time is clearly invalid, but is not null. This
 *      is usually the result of creating a new time type buy not clearing
 *      it, or setting one of the flags to an illegal value.
 */
int icaltime_is_valid_time(const struct icaltimetype t)
{
    if (t.is_utc > 1 || t.is_utc < 0 ||
        t.year < 0 || t.year > 3000 || t.is_date > 1 || t.is_date < 0) {
        return 0;
    } else {
        return 1;
    }
}

/**     @brief Returns true if time is a DATE
 */
int icaltime_is_date(const struct icaltimetype t)
{
    return t.is_date;
}

/**     @brief Returns true if time is relative to UTC zone
 *
 *      @todo  We should only check the zone
 */
int icaltime_is_utc(const struct icaltimetype t)
{
    return t.is_utc;
}

/**
 *      Return true if the time is null.
 */
int icaltime_is_null_time(const struct icaltimetype t)
{
    if (t.second + t.minute + t.hour + t.day + t.month + t.year == 0) {
        return 1;
    }

    return 0;
}

/**
 *      Return -1, 0, or 1 to indicate that a<b, a==b, or a>b.
 *      This calls icaltime_compare function after converting them to the utc
 *      timezone.
 */

int icaltime_compare(const struct icaltimetype a_in, const struct icaltimetype b_in)
{
    struct icaltimetype a, b;

    /* We only need to perform time zone conversion if times aren't in the same time zone */
    if (a_in.zone != b_in.zone || a_in.is_utc != b_in.is_utc) {
        a = icaltime_convert_to_zone(a_in, icaltimezone_get_utc_timezone());
        b = icaltime_convert_to_zone(b_in, icaltimezone_get_utc_timezone());
    } else {
        a = a_in;
        b = b_in;
    }

    if (a.year > b.year) {
        return 1;
    } else if (a.year < b.year) {
        return -1;
    } else if (a.month > b.month) {
        return 1;
    } else if (a.month < b.month) {
        return -1;
    } else if (a.day > b.day) {
        return 1;
    } else if (a.day < b.day) {
        return -1;
    }

    /* if both are dates, we are done */
    if (a.is_date && b.is_date) {
        return 0;

    /* else, if only one is a date (and we already know the date part is equal),
       then the other is greater */
    } else if (b.is_date) {
        return 1;
    } else if (a.is_date) {
        return -1;
    } else if (a.hour > b.hour) {
        return 1;
    } else if (a.hour < b.hour) {
        return -1;
    } else if (a.minute > b.minute) {
        return 1;
    } else if (a.minute < b.minute) {
        return -1;
    } else if (a.second > b.second) {
        return 1;
    } else if (a.second < b.second) {
        return -1;
    }

    return 0;
}

/**
 *      like icaltime_compare, but only use the date parts.
 */

int icaltime_compare_date_only(const struct icaltimetype a_in,
                               const struct icaltimetype b_in)
{
    struct icaltimetype a, b;
    icaltimezone *tz = icaltimezone_get_utc_timezone();

    a = icaltime_convert_to_zone(a_in, tz);
    b = icaltime_convert_to_zone(b_in, tz);

    if (a.year > b.year) {
        return 1;
    } else if (a.year < b.year) {
        return -1;
    }

    if (a.month > b.month) {
        return 1;
    } else if (a.month < b.month) {
        return -1;
    }

    if (a.day > b.day) {
        return 1;
    } else if (a.day < b.day) {
        return -1;
    }

    return 0;
}

/**
 *      like icaltime_compare, but only use the date parts; accepts timezone.
 */

int icaltime_compare_date_only_tz(const struct icaltimetype a_in,
                                  const struct icaltimetype b_in,
                                  icaltimezone *tz)
{
    struct icaltimetype a, b;

    a = icaltime_convert_to_zone(a_in, tz);
    b = icaltime_convert_to_zone(b_in, tz);

    if (a.year > b.year) {
        return 1;
    } else if (a.year < b.year) {
        return -1;
    }

    if (a.month > b.month) {
        return 1;
    } else if (a.month < b.month) {
        return -1;
    }

    if (a.day > b.day) {
        return 1;
    } else if (a.day < b.day) {
        return -1;
    }

    return 0;
}

/* These are defined in icalduration.c:
struct icaltimetype  icaltime_add(struct icaltimetype t,
                                  struct icaldurationtype  d)
struct icaldurationtype  icaltime_subtract(struct icaltimetype t1,
                                           struct icaltimetype t2)
*/

/**     @brief Internal, shouldn't be part of the public API
 *
 *      Adds (or subtracts) a time from a icaltimetype.
 *      NOTE: This function is exactly the same as icaltimezone_adjust_change()
 *      except for the type of the first parameter.
 */
void icaltime_adjust(struct icaltimetype *tt,
                     const int days, const int hours,
                     const int minutes, const int seconds)
{
    int second, minute, hour, day;
    int minutes_overflow, hours_overflow, days_overflow = 0, years_overflow;
    int days_in_month;

    /* If we are passed a date make sure to ignore hour minute and second */
    if (tt->is_date)
        goto IS_DATE;

    /* Add on the seconds. */
    second = tt->second + seconds;
    tt->second = second % 60;
    minutes_overflow = second / 60;
    if (tt->second < 0) {
        tt->second += 60;
        minutes_overflow--;
    }

    /* Add on the minutes. */
    minute = tt->minute + minutes + minutes_overflow;
    tt->minute = minute % 60;
    hours_overflow = minute / 60;
    if (tt->minute < 0) {
        tt->minute += 60;
        hours_overflow--;
    }

    /* Add on the hours. */
    hour = tt->hour + hours + hours_overflow;
    tt->hour = hour % 24;
    days_overflow = hour / 24;
    if (tt->hour < 0) {
        tt->hour += 24;
        days_overflow--;
    }

  IS_DATE:
    /* Normalize the month. We do this before handling the day since we may
       need to know what month it is to get the number of days in it.
       Note that months are 1 to 12, so we have to be a bit careful. */
    if (tt->month >= 13) {
        years_overflow = (tt->month - 1) / 12;
        tt->year += years_overflow;
        tt->month -= years_overflow * 12;
    } else if (tt->month <= 0) {
        /* 0 to -11 is -1 year out, -12 to -23 is -2 years. */
        years_overflow = (tt->month / 12) - 1;
        tt->year += years_overflow;
        tt->month -= years_overflow * 12;
    }

    /* Add on the days. */
    day = tt->day + days + days_overflow;
    if (day > 0) {
        for (;;) {
            days_in_month = icaltime_days_in_month(tt->month, tt->year);
            if (day <= days_in_month)
                break;

            tt->month++;
            if (tt->month >= 13) {
                tt->year++;
                tt->month = 1;
            }

            day -= days_in_month;
        }
    } else {
        while (day <= 0) {
            if (tt->month == 1) {
                tt->year--;
                tt->month = 12;
            } else {
                tt->month--;
            }

            day += icaltime_days_in_month(tt->month, tt->year);
        }
    }
    tt->day = day;
}

/**     @brief Convert time to a given timezone
 *
 *      Convert a time from its native timezone to a given timezone.
 *
 *      If tt is a date, the returned time is an exact
 *      copy of the input. If it's a floating time, the returned object
 *      represents the same time translated to the given timezone.
 *      Otherwise the time will be converted to the new
 *      time zone, and its native timezone set to the right timezone.
 */
struct icaltimetype icaltime_convert_to_zone(const struct icaltimetype tt, icaltimezone *zone)
{
    struct icaltimetype ret = tt;

    /* If it's a date do nothing */
    if (tt.is_date) {
        return ret;
    }

    if (tt.zone == zone) {
        return ret;
    }

    /* If it's a floating time we don't want to adjust the time */
    if (tt.zone != NULL || tt.is_utc) {
        icaltimezone *from_zone = (icaltimezone *) tt.zone;

        if (!from_zone) {
            from_zone = icaltimezone_get_utc_timezone();
        }

        icaltimezone_convert_time(&ret, from_zone, zone);
    }

    ret.zone = zone;
    if (zone == icaltimezone_get_utc_timezone()) {
        ret.is_utc = 1;
    } else {
        ret.is_utc = 0;
    }

    return ret;
}

const icaltimezone *icaltime_get_timezone(const struct icaltimetype t)
{
    return t.zone;
}

const char *icaltime_get_tzid(const struct icaltimetype t)
{
    if (t.zone != NULL) {
        return icaltimezone_get_tzid((icaltimezone *) t.zone);
    } else {
        return NULL;
    }
}

/**     @brief Set the timezone
 *
 *      Force the icaltime to be interpreted relative to another timezone.
 *      If you need to do timezone conversion, applying offset adjustments,
 *      then you should use icaltime_convert_to_zone instead.
 */
struct icaltimetype icaltime_set_timezone(struct icaltimetype *t, const icaltimezone *zone)
{
    /* If it's a date do nothing */
    if (t->is_date) {
        return *t;
    }

    if (t->zone == zone) {
        return *t;
    }

    t->zone = zone;
    if (zone == icaltimezone_get_utc_timezone()) {
        t->is_utc = 1;
    } else {
        t->is_utc = 0;
    }

    return *t;
}

/**
 *  @brief builds an icaltimespan given a start time, end time and busy value.
 *
 *  @param dtstart   The beginning time of the span, can be a date-time
 *                   or just a date.
 *  @param dtend     The end time of the span.
 *  @param is_busy   A boolean value, 0/1.
 *  @return          A span using the supplied values.
 *
 *  returned span contains times specified in UTC.
 */

icaltime_span icaltime_span_new(struct icaltimetype dtstart, struct icaltimetype dtend, int is_busy)
{
    icaltime_span span;

    span.is_busy = is_busy;

    span.start = icaltime_as_timet_with_zone(dtstart,
                                             dtstart.zone ? dtstart.
                                             zone : icaltimezone_get_utc_timezone());

    if (icaltime_is_null_time(dtend)) {
        if (!icaltime_is_date(dtstart)) {
            /* If dtstart is a DATE-TIME and there is no DTEND nor DURATION
               it takes no time */
            span.end = span.start;
            return span;
        } else {
            dtend = dtstart;
        }
    }

    span.end = icaltime_as_timet_with_zone(dtend,
                                           dtend.zone ? dtend.
                                           zone : icaltimezone_get_utc_timezone());

    if (icaltime_is_date(dtstart)) {
        /* no time specified, go until the end of the day.. */
        span.end += 60 * 60 * 24 - 1;
    }
    return span;
}

/** @brief Returns true if the two spans overlap
 *
 *  @param s1         1st span to test
 *  @param s2         2nd span to test
 *  @return           boolean value
 *
 *  The result is calculated by testing if the start time of s1 is contained
 *  by the s2 span, or if the end time of s1 is contained by the s2 span.
 *
 *  Also returns true if the spans are equal.
 *
 *  Note, this will return false if the spans are adjacent.
 */

int icaltime_span_overlaps(icaltime_span *s1, icaltime_span *s2)
{
    /* s1->start in s2 */
    if (s1->start > s2->start && s1->start < s2->end)
        return 1;

    /* s1->end in s2 */
    if (s1->end > s2->start && s1->end < s2->end)
        return 1;

    /* s2->start in s1 */
    if (s2->start > s1->start && s2->start < s1->end)
        return 1;

    /* s2->end in s1 */
    if (s2->end > s1->start && s2->end < s1->end)
        return 1;

    if (s1->start == s2->start && s1->end == s2->end)
        return 1;

    return 0;
}

/** @brief Returns true if the span is totally within the containing
 *  span
 *
 *  @param s          The span to test for.
 *  @param container  The span to test against.
 *  @return           boolean value.
 *
 */

int icaltime_span_contains(icaltime_span *s, icaltime_span *container)
{
    if ((s->start >= container->start && s->start < container->end) &&
        (s->end <= container->end && s->end > container->start)) {
        return 1;
    }

    return 0;
}
