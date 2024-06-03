/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2008-2019  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file iso8601.h
 *
 * ISO8601 Time Format string utilities
 *
 */

#define _XOPEN_SOURCE  /* for strptime */
#define _DEFAULT_SOURCE // Must be defined before _BSD_SOURCE to avoid a warning on GCC on Ubuntu Desktop 16.10
#define _BSD_SOURCE    /* for timegm */

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common_defs.h"
#include "iso8601.h"
#include "usp_api.h"

/*********************************************************************//**
**
** iso8601_cur_time
**
** Gets the current UTC time in a string format
**
** \param   buf - pointer to buffer in which to return the string
** \param   len - length of buffer. Must be at least MAX_ISO8601_LEN bytes long.
**
** \return  pointer to string (i.e. supplied buffer)
**
**************************************************************************/
char *iso8601_cur_time(char *buf, int len)
{
    time_t t;

    time(&t);
    return iso8601_from_unix_time(t, buf, len);
}

/*********************************************************************//**
**
** iso8601_from_unix_time
**
** Given a time_t converts it to an ISO8601 string
**
** \param   unix_time - time in seconds since the epoch (UTC)
** \param   buf - pointer to buffer in which to return the string
** \param   len - length of buffer. Must be at least MAX_ISO8601_LEN bytes long.
**
** \return  pointer to string (i.e. supplied buffer)
**
**************************************************************************/
char *iso8601_from_unix_time(time_t unix_time, char *buf, int len)
{
   	struct tm tm;

    // Exit if the time provided is the special case of the unknown time
    if (unix_time == UNKNOWN_TIME)
    {
        USP_STRNCPY(buf, UNKNOWN_TIME_STR, len);
        return buf;
    }

    memset(&tm, 0, sizeof(tm));
    gmtime_r(&unix_time, &tm);
    iso8601_strftime(buf, len, &tm);
    return buf;
}


/**
 * Represent time_t in iso8601 format w/ optional timezone info
 *   YYYY-MM-DDThh:mm:ssTZD
 * where,
 *   YYYY = four-digit year
 *   MM   = two-digit month (01=January, etc.)
 *   DD   = two-digit day of month (01 through 31)
 *   hh   = two digits of hour (00 through 23) (am/pm NOT allowed)
 *   mm   = two digits of minute (00 through 59)
 *   ss   = two digits of second (00 through 59)
 *   TZD  = time zone designator (Z or +hh:mm or -hh:mm)
 *
 * See the w3c date-time format profiles on
 *   Complete date:hours:minutes:seconds
 *
 * @param tm - input - Pointer to time in seconds since 1900
 * @param buf - input/output - Pointer to buffer to print into
 * @return - number of characters placed into buf, not including the
 *           terminating null character. if any error occurs,
 *           0 is returned and the content is interminate.
 */
size_t
iso8601_strftime(char *buf, size_t buflen, const struct tm *tm)
{
	size_t sz;

	if (!tm || !buf || (buflen == 0)) {
		return 0;
	}
	if (tm->tm_gmtoff == 0) {
		sz = strftime(buf, buflen, "%FT%TZ", tm);
	} else {
		sz = strftime(buf, buflen, "%FT%T", tm);
		if (sz < buflen) {
			long absoff;

			if (tm->tm_gmtoff < 0) {
				absoff = -tm->tm_gmtoff;
			} else {
				absoff = tm->tm_gmtoff;
			}
			sz += USP_SNPRINTF(&buf[sz], buflen - sz,
					 "%c%02ld:%02ld",
					 (tm->tm_gmtoff < 0) ? '-' : '+',
					 absoff / 3600, (absoff % 3600) / 60);
		}
	}
	return sz;
}

/**
 * Format a (struct timeval) to an ISO-8601 string with microsecond precision.
 *
 * Result will be in UTC.
 *
 * Example: 2008-04-09T15:01:05.123456Z
 *
 * @return Length of resulting string in characters, or -1 if formatting failed.
 */
size_t
iso8601_us_strftime(char *buf, size_t bufsiz, const struct timeval *tv)
{
        if (!buf || !tv || bufsiz < 8) {
            return -1;
        }

        size_t sz;
        sz = strftime(buf, bufsiz-8, "%FT%T.", gmtime(&tv->tv_sec));
        sz += sprintf(buf+strlen(buf), "%06ldZ", (long int)tv->tv_usec);
        return sz;
}

/*********************************************************************//**
**
**  iso8601_to_unix_time
**
**  Converts an ISO8601 string time into a UTC-based unix time
**
** \param   date - pointer to ISO8601 string to convert
**
** \return  Number of seconds since the UTC unix epoch, or INVALID_TIME if the conversion failed
**
**************************************************************************/
time_t
iso8601_to_unix_time(const char *date)
{
    char *p;
    struct tm tm;
    time_t converted_time;

    // Exit if the date is the UNKNOWN_TIME. This is a special case which we accept which is outside of 32 bit time_t
    if (strcmp(date, UNKNOWN_TIME_STR)==0)
    {
        return UNKNOWN_TIME;
    }

    memset(&tm, 0, sizeof(tm));     // Needed as strptime only fills-in the fields it parses out

    // Exit if unable to parse ISO8601 string into tm structure
    p = strptime(date, "%Y-%m-%dT%H:%M:%S", &tm);
    if (p == NULL)
    {
        return INVALID_TIME;
    }


    // In order to work with Linux kernels that only support a 32 bit time_t,
    // we limit any reference time to between 1970-2037 (32 bit Unix time ends on Jan 19th 2038)
    if ((tm.tm_year < 70) || (tm.tm_year > 137))
    {
        return INVALID_TIME;
    }

    // Convert tm structure into UTC unix time
    converted_time = timegm(&tm);

    return converted_time;
}

/**
 * Represent system uptime(or delta time) in following format:
 * hh:mm:ss up x day(s), r:ii
 * where,
 * hh = hours (digits, 00 – 24)
 * mm = minutes (digits, 00 – 60)
 * ss = seconds (digits, 00 – 60)
 * x = num of days the system has been running
 * r = num of hours the system has been running on the day
 * ii = num of minutes the system has been runningon the day in the hour
 *
 * @param uptime - input - uptime in seconds
 * @param buf - input/output - Pointer to buffer to print into
 * @param buflen - input - size of buf
 * @return - number of characters placed into buf, not including the
 *           terminating null character. if any error occurs,
 *           0 is returned and the content is interminate.
 */
size_t
uptime_strftime(char *buf, size_t buflen, unsigned uptime)
{
	int years;
	int months;
	int days;
	int hours;
	int mins;
	int secs;
	int total_days;

	size_t sz;

	if (!buf || (buflen == 0)) {
		return 0;
	}

	total_days = uptime / (24 * 3600);
	years = total_days / (12 * 30);  /* 12mon * 30days/mon */
	days = total_days % (12 * 30);
	months = days / 30;
	days = days % 30;
	hours = uptime / 3600 - total_days * 24;
	mins = uptime / 60 - total_days * 24 * 60 - hours * 60;
	secs = uptime - total_days * 24 * 3600 - hours * 3600 - mins * 60;

	sz = USP_SNPRINTF(buf, buflen, "P%04d-%02d-%02dT%02d:%02d:%02d",
			years, months, days, hours, mins, secs);


	return sz;
}

