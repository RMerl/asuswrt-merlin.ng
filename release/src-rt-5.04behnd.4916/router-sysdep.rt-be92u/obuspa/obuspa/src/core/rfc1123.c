/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * @file rfc1123.c
 *
 * RFC1123 Time Format string utilities
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common_defs.h"
#include "rfc1123.h"

// RFC1123 dates are specified independant of locale, hence we need to use the arrays here, rather than the
// locale-respecting ones built into strftime()
const char *rfc1123_day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *rfc1123_month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


/*********************************************************************//**
**
** RFC1123_GetCurTime
**
** Gets the current UTC time in an RFC1123 string format
**
** \param   buf - pointer to buffer in which to return the string
** \param   len - length of buffer. Must be at least 23 bytes long.
**
** \return  pointer to string (i.e. supplied buffer)
**
**************************************************************************/
char *RFC1123_GetCurTime(char *buf, int len)
{
    time_t t;

    time(&t);
    return RFC1123_FromUnixTime(t, buf, len);
}

/*********************************************************************//**
**
** RFC1123_FromUnixTime
**
** Given a time_t converts it to an RFC1123 date format string
**
** \param   unix_time - time in seconds since the epoch (UTC)
** \param   buf - pointer to buffer in which to return the string
** \param   len - length of buffer. Must be at least 30 bytes long.
**
** \return  pointer to string (i.e. supplied buffer)
**
**************************************************************************/
char *RFC1123_FromUnixTime(time_t unix_time, char *buf, int len)
{
   	struct tm tm;

    gmtime_r(&unix_time, &tm);

    USP_SNPRINTF(buf, len, "%s, %d %s %d %02d:%02d:%02d GMT",
                           rfc1123_day[tm.tm_wday],
                           tm.tm_mday,
                           rfc1123_month[tm.tm_mon],
                           1900 + tm.tm_year,
                           tm.tm_hour,
                           tm.tm_min,
                           tm.tm_sec);
    return buf;
}


