/*
 *
 * Copyright (C) 2019-2020, Broadband Forum
 * Copyright (C) 2019-2020  CommScope, Inc
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
 * \file device_time.c
 *
 * Implements the Device.Time data model object
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "iso8601.h"

#ifndef REMOVE_DEVICE_TIME
//------------------------------------------------------------------------------
// Data model path strings
static char *timezone_path = "Device.Time.LocalTimeZone";

// Environment variable used for timezone
static char *tz_env_var = "TZ";
//------------------------------------------------------------------------------
// Ccached values of timezone parameter
#define MAX_TIMEZONE_LEN 257
static char database_tz[MAX_TIMEZONE_LEN];      // Cached value of timezone in the USP DB
static char *process_tz = NULL;                 // Cached value of timezone environment variable. NOTE: A value of NULL indicates that the environment variable was not set

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int Validate_LocalTimeZone(dm_req_t *req, char *value);
int NotifyChange_LocalTimeZone(dm_req_t *req, char *value);
int GetCurrentLocalTime(dm_req_t *req, char *buf, int len);
bool mifd_tzvalidate(char *p);
char *tz_skip_name(char *p);
char *tz_skip_word(char *p, bool allow_numeric);
char *tz_skip_hours_minutes_seconds(char *p);
char *tz_skip_number(char *p, int min_digits, int max_digits, int min_value, int max_value, char *name);
char *tz_skip_dst_definition(char *p);
char *tz_skip_month_week_day(char *p);
char *tz_skip_offset(char *p);

/*********************************************************************//**
**
** DEVICE_TIME_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DEVICE_TIME_Init(void)
{
    int err = USP_ERR_OK;

    // Register parameters implemented by this component
    err = USP_ERR_OK;
    err |= USP_REGISTER_VendorParam_ReadOnly("Device.Time.CurrentLocalTime", GetCurrentLocalTime, DM_DATETIME);
    err |= USP_REGISTER_DBParam_ReadWrite(timezone_path, "", Validate_LocalTimeZone, NotifyChange_LocalTimeZone, DM_STRING);

    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_TIME_Start
**
** If a timezone environment variable is set, then ensure the timezone in the USP DB matches (if none has been setup)
**
** \param   None
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DEVICE_TIME_Start(void)
{
    int err;

    // Cache the timezone environment variable
    process_tz = getenv(tz_env_var);

    // Exit if unable to cache the current timezone in the USP DB
    err = DATA_MODEL_GetParameterValue(timezone_path, database_tz, sizeof(database_tz), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_LocalTimeZone
**
** Function called to validate Device.Time.LocalTimeZone
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_LocalTimeZone(dm_req_t *req, char *value)
{
    int is_valid;

    // Exit if timezone failed to validate
    is_valid = mifd_tzvalidate(value);
    if (is_valid == false)
    {
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_LocalTimeZone
**
** Function called after Device.Time.LocalTimeZone is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_LocalTimeZone(dm_req_t *req, char *value)
{
    USP_STRNCPY(database_tz, value, sizeof(database_tz));
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCurrentLocalTime
**
** Returns the current local time in ISO8601 format
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCurrentLocalTime(dm_req_t *req, char *buf, int len)
{
    time_t t;
    struct tm tm;
    int result;

    // Get current time offset from unix epoch
    t = time(NULL);

    // Temporarily change the timezone environment variable to the database value
    // NOTE: This is necessary because otherwise busy box syslog entries will show inconsistent times between processes
    result = setenv(tz_env_var, database_tz, 1);
    if (result != 0)
    {
        USP_ERR_SetMessage("%s: setenv(TZ=%s) failed: %s", __FUNCTION__, database_tz, strerror(errno));
        return USP_ERR_INTERNAL_ERROR;
    }

    tzset();   /* NOTE: tzset() does not return an error code, so we have to assume it was successful. (Also cannot use errno. It is always set with type 2 format timezone strings, even though no error occurred) */

    // Create split representation of time, applying the timezone definition
    localtime_r(&t, &tm);

    // Quickly switch back to the original timezone for this process
    if (process_tz != NULL)
    {
        result = setenv(tz_env_var, process_tz, 1);
        if (result != 0)
        {
            USP_LOG_Warning("%s: setenv(TZ=%s) failed: %s", __FUNCTION__, process_tz, strerror(errno));
            // NOTE: Deliberately ignoring error, since it will only affect syslog output in the worst case
        }
    }
    else
    {
        result = unsetenv(tz_env_var);
        if (result != 0)
        {
            USP_LOG_Warning("%s: unsetenv(TZ) failed: %s", __FUNCTION__, strerror(errno));
            // NOTE: Deliberately ignoring error, since it will only affect syslog output in the worst case
        }
    }

    tzset();

    // Finally create the current time string
    iso8601_strftime(buf, len, &tm);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  mifd_tzvalidate
**
**  Validates the specified POSIX format timezone string
**  See the man page for tzset. The format of a timezone string is:-
**     name offset1[dst[offset2][,start[/time],end[/time]]]
**
** \param   p - pointer to POSIX format timezone string to validate
**
** \return  true if the string was a valid timezone, false otherwise
**
**************************************************************************/
bool mifd_tzvalidate(char *p)
{
    // Exit if the timezone is a blank string. We allow this, to mean 'use the system default'
    if (*p == '\0')
    {
        return true;
    }

    // Exit if the timezone string incorrectly includes spaces. This is invalid according to the tzset man page
    if (strchr(p, ' ') != NULL)
    {
        USP_ERR_SetMessage("%s: timezone definition '%s' should not include spaces", __FUNCTION__, p);
        return false;
    }


    // Exit if the name of the timezone ("name") is incorrectly specified
    p = tz_skip_name(p);
    if (p == NULL)
    {
        return false;
    }

    // Exit if the timezone offset ("offset1") is incorrectly specified (failure)
    // or we have reached the end of the string (success)
    p = tz_skip_offset(p);
    if (p == NULL)
    {
        return false;
    }

    if (*p == '\0')
    {
        return true;
    }

    // Exit if the daylight saving timezone name ("dst") is incorrectly specified (failure)
    // or we have reached the end of the string (success)
    // NOTE: "dst" is optional, if it is missing, then next character will be ',' or '\0'
    if ((*p != ',') && (*p != '\0'))
    {
        p = tz_skip_name(p);
        if (p == NULL)
        {
            return false;
        }

        if (*p == '\0')
        {
            return true;
        }

        // Exit if the daylight saving timezone offset ("offset2") is incorrectly specified (failure)
        // or we have reached the end of the string (success)
        // NOTE: "offset2" is optional, if it is missing, then next character will be ','
        if (*p != ',')
        {
            p = tz_skip_offset(p);
            if (p == NULL)
            {
                return false;
            }

            if (*p == '\0')
            {
                return true;
            }
        }
    }

    // Exit if the daylight saving time start date/time are incorrectly specified (failure)
    p = tz_skip_dst_definition(p);
    if (p == NULL)
    {
        return false;
    }

    // Exit if the daylight saving time end date/time are incorrectly specified (failure)
    // or we have reached the end of the string (success)
    p = tz_skip_dst_definition(p);
    if (p == NULL)
    {
        return false;
    }

    if (*p == '\0')
    {
        return true;
    }

    // If there is still any string left after successfully parsing the rest of the string, then flag this as a failure
    return false;
}

/*********************************************************************//**
**
**  tz_skip_name
**
**  Skips a time zone name, which is either:-
**    Format A: A string containing alphabetic characters
**    Format B: A string enclosed by <> and containing alphabetic characters, digits and '+' or '-'
**
** \param   p - pointer to POSIX format timezone string to validate, starting at a timezone name
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_name(char *p)
{
    #define ALLOW_ALPHABETIC_ONLY false
    #define ALLOW_ALPHABETIC_AND_NUMERIC true

    if (*p == '<')
    {
        // Deal with format B
        p++;    // Skip leading '<'
        p = tz_skip_word(p, ALLOW_ALPHABETIC_AND_NUMERIC);
        if (p != NULL)
        {
            // Exit if name is not terminated with trailing '>' character
            if (*p != '>')
            {
                USP_ERR_SetMessage("%s: Missing '>' in timezone definition", __FUNCTION__);
                return NULL;
            }
            p++;    // Skip trailing '>'
        }
    }
    else
    {
        // Deal with format A
        p = tz_skip_word(p, ALLOW_ALPHABETIC_ONLY);
    }

    return p;
}

/*********************************************************************//**
**
**  tz_skip_word
**
**  Skips a word which may or may not contain numeric characters
**
** \param   p - pointer to POSIX format timezone string to validate, starting at a timezone name
** \param   allow_numeric - Set to false if the word can contain only alphabetic characters.
**                          Set to true if the word can additionally contain numberic characters.
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_word(char *p, bool allow_numeric)
{
    int count = 0;
    char *saved_p;

    // Skip valid characters
    #define is_alphabetic(c) ( (((c) >= 'a') && ((c) <= 'z')) || (((c) >= 'A') && ((c) <= 'Z')) )
    #define is_numeric(c)    ( ((c) >= '0') && ((c) <= '9') )
    #define is_numeric_ext(c)    ( (is_numeric(c)) || ((c) == '+')  || ((c) == '-')  )
    saved_p = p;
    while ( is_alphabetic(*p) || (allow_numeric && is_numeric_ext(*p)) )
    {
        count++;
        p++;
    }

    // Exit if string is not long enough to be valid
    if (count < 3)
    {
        USP_ERR_SetMessage("%s: Expecting word of 3 or more characters at '%s'", __FUNCTION__, saved_p);
        return NULL;
    }

    // If the code gets here, then the string was parsed successfully
    return p;
}

/*********************************************************************//**
**
**  tz_skip_hours_minutes_seconds
**
**  Skips a time specified as hours:minutes:seconds in the form
**              [+|-]hh[:mm[:ss]]
**
** \param   p - pointer to POSIX format timezone string to validate, starting at a time
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_hours_minutes_seconds(char *p)
{
    // Exit if hour is not correct
    p = tz_skip_number(p, 1,2, 0,24, "Hour");
    if (p == NULL)
    {
        return NULL;
    }

    if (*p == ':')
    {
        // Exit if minutes are not correct (if present)
        p++;        // Skip leading ':'
        p = tz_skip_number(p, 1,2, 0,59, "Minutes");
        if (p == NULL)
        {
            return NULL;
        }

        if (*p == ':')
        {
            // Exit if seconds are not correct (if present)
            p++;        // Skip leading ':'
            p = tz_skip_number(p, 1,2, 0,59, "Seconds");
            if (p == NULL)
            {
                return NULL;
            }
        }
    }

    return p;
}

/*********************************************************************//**
**
**  tz_skip_number
**
**  Skip a number made up of a number of digits, and validating it against a range
**
** \param   p - pointer to POSIX format timezone string to validate, starting at a number
** \param   min_digits - Minimum number of digits comprising the number
** \param   max_digits - Maximum number of digits comprising the number
** \param   min_value - Minimum value of the number
** \param   max_value - Maximum value of the number
** \param   name - name of the field being parsed (used in error message)
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_number(char *p, int min_digits, int max_digits, int min_value, int max_value, char *name)
{
    int number = 0;
    int digit_count = 0;
    char *saved_p;

    saved_p = p;
    while (digit_count < max_digits)
    {
        // Exit loop if we've encountered a non digit
        if (!is_numeric(*p))
        {
            break;
        }

        // Otherwise concatenate the digit to the number, and move to next digit
        number = 10*number + (int)(*p -'0');
        digit_count++;
        p++;
    }

    // Exit if we didn't convert enough digits for the number
    if (digit_count < min_digits)
    {
        USP_ERR_SetMessage("%s: Expecting %d digits in %s field at '%s'", __FUNCTION__, min_digits, name, saved_p);
        return NULL;
    }

    // Exit if number is not in range
    if ((number < min_value) || (number > max_value))
    {
        USP_ERR_SetMessage("%s: %s field out of range (%d:%d) at '%s'", __FUNCTION__, name, min_value, max_value, saved_p);
        return NULL;
    }

    return p;
}

/*********************************************************************//**
**
**  tz_skip_dst_definition
**
**  Skip a daylight saving time (DST) definition. This has the format
**      ,start[/time]
**
** \param   p - pointer to POSIX format timezone string to validate, starting at the DST start time definition
** \param   min_digits - Minimum number of digits comprising the number
** \param   max_digits - Maximum number of digits comprising the number
** \param   min_value - Minimum value of the number
** \param   max_value - Maximum value of the number
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_dst_definition(char *p)
{
    // Exit if the next character is not ','
    if (*p != ',')
    {
        USP_ERR_SetMessage("%s: Expecting ',' at '%s'", __FUNCTION__, p);
        return NULL;
    }
    p++;        // Skip leading ','

    // Start date has 3 possible formats
    switch(*p)
    {
        case 'J':
            // Julian day excluding leap days
            p++;  // Skip leading 'J'
            p = tz_skip_number(p, 1,3, 1,365, "Julian day (excluding leap seconds)");
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // Julian day including leap days
            p = tz_skip_number(p, 1,3, 0,365, "Julian day (including leap seconds)");
            break;

        case 'M':
            // m.w.d  Month.Week.Day format
            p++;  // Skip leading 'M'
            p = tz_skip_month_week_day(p);
            break;


        default:
            USP_ERR_SetMessage("%s: Illegal DST change format (expected 'J', 'M' or number)", __FUNCTION__);
            return NULL;
    }

    // Exit if failed
    if (p == NULL)
    {
        return NULL;
    }

    // Exit if a time is not specified
    if (*p != '/')
    {
        return p;
    }
    p++; // Skip leading '/'

    // Exit if hours:minutes:seconds does not follow
    p = tz_skip_hours_minutes_seconds(p);

    return p;
}

/*********************************************************************//**
**
**  tz_skip_month_week_day
**
**  Skip a month.week.day definition (used to define the day that daylight saving time starts/stops).
**  This has the format
**                   m.w.d
**
** \param   p - pointer to POSIX format timezone string to validate, starting at the month
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_month_week_day(char *p)
{
    char *saved_p;

    // Exit if month is out of range
    saved_p = p;
    p = tz_skip_number(p, 1,2, 1,12, "Month");
    if (p == NULL)
    {
        return NULL;
    }

    // Exit if month does not terminate in a '.'
    if (*p != '.')
    {
        USP_ERR_SetMessage("%s: Month should terminate in a '.' at '%s'", __FUNCTION__, saved_p);
        return NULL;
    }
    p++;  // Skip '.'

    // Exit if week is out of range
    saved_p = p;
    p = tz_skip_number(p, 1,1, 1,5, "Week");
    if ((p == NULL) || (*p != '.'))
    {
        return NULL;
    }

    // Exit if week does not terminate in a '.'
    if (*p != '.')
    {
        USP_ERR_SetMessage("%s: Week should terminate in a '.' at '%s'", __FUNCTION__, saved_p);
        return NULL;
    }
    p++;  // Skip '.'

    // Exit if day is out of range
    p = tz_skip_number(p, 1,1, 0,6, "Day");
    if (p == NULL)
    {
        return NULL;
    }

    return p;
}

/*********************************************************************//**
**
**  tz_skip_offset
**
**  Skips a time offset
**  This has the format
**          [+/-]HH[:MM[:SS]]
**
** \param   p - pointer to POSIX format timezone string to validate, starting at the time offset
**
** \return  pointer to next section of the timezone, or NULL if an error occurred in parsing
**
**************************************************************************/
char *tz_skip_offset(char *p)
{
    // Skip leading '+' or '-' characters
    if ((*p == '+') || (*p == '-'))
    {
        p++;
    }

    p = tz_skip_hours_minutes_seconds(p);
    return p;
}

//---------------------------------------------------------------------------------------
// Test code
#if 0
//typedef struct
//t{
//t    char *timezone;
//t    bool result;  // true=pass, false=fail
//t} test_t;
//t
//ttest_t tests[] = {
//t    { "EST+5EDT,M4.1.0/2,M10.5.0/2", true },
//t    { "EST+5 EDT,M4.1.0/2,M10.5.0/2", false },
//t    { "EST", false },
//t    { "EST+", false },
//t    { "EST-", false },
//t    { "EST+9", true },
//t    { "EST-0", true },
//t    { "ET-0", false },
//t    { "ESTime+0", true },
//t    { "EST+00", true },
//t    { "EST+05", true },
//t    { "EST+24", true },
//t    { "EST+25", false },
//t    { "EST+25:", false },
//t    { "EST+05:20", true },
//t    { "EST+05:00", true },
//t    { "EST+05:59", true },
//t    { "EST+05:60", false },
//t    { "EST+05:20:47", true },
//t    { "EST+05:20:00", true },
//t    { "EST+05:20:59", true },
//t    { "EST+05:20:60", false },
//t    { "EST+05:20:59:", false },
//t
//t    { "EST+5EDT", true },
//t    { "EST+5EDT+5", true },
//t    { "EST+5EDT-0", true },
//t    { "EST+5EDT-0:20:59", true },
//t
//t    { "EST+5EDT-0,", false },
//t    { "EST+5EDT-0,J", false },
//t    { "EST+5EDT-0,J1", false },
//t    { "EST+5EDT-0,J1,", false },
//t    { "EST+5EDT-0,J1,J", false },
//t    { "EST+5EDT-0,J1,J1", true },
//t    { "EST+5EDT-0,J0,J1", false },
//t    { "EST+5EDT-0,J365,J1", true },
//t    { "EST+5EDT-0,J366,J1", false },
//t    { "EST+5EDT-0,0,0", true },
//t    { "EST+5EDT-0,0,365", true },
//t    { "EST+5EDT-0,0,366", false },
//t    { "EST+5EDT-0,0", false },
//t    { "EST+5EDT-0,0,", false },
//t    { "EST+5EDT-0,0,J365", true },
//t
//t    { "EST+5EDT-0,M,J1", false },
//t    { "EST+5EDT-0,M1,J1", false },
//t    { "EST+5EDT-0,M1.1,J1", false },
//t    { "EST+5EDT-0,M1.1.0,J1", true },
//t    { "EST+5EDT-0,M12.1.0,J1", true },
//t    { "EST+5EDT-0,M13.1.0,J1", false },
//t    { "EST+5EDT-0,M1.5.0,J1", true },
//t    { "EST+5EDT-0,M1.6.0,J1", false },
//t    { "EST+5EDT-0,M1.1.6,J1", true },
//t    { "EST+5EDT-0,M1.1.7,J1", false },
//t    { "EST+5EDT-0,M1.1.6,M12.1.6", true },
//t    { "EST+5EDT-0,M01.1.6,M12.1.6", true },
//t
//t    { "EST+5EDT-0,M01.1.6/,M12.1.6", false },
//t    { "EST+5EDT-0,M01.1.6,M12.1.6/", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6", true },
//t    { "EST+5EDT-0,M01.1.6/00:00:00,M12.1.6", true },
//t    { "EST+5EDT-0,M01.1.6/0:0:0", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M", false },
//t    { "EST+5EDT-0,M01.1.6/24:59:59,M12.1.6", true },
//t    { "EST+5EDT-0,M01.1.6/25:59:59,M12.1.6", false },
//t    { "EST+5EDT-0,M01.1.6/24:60:59,M12.1.6", false },
//t    { "EST+5EDT-0,M01.1.6/24:59:60,M12.1.6", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0", true },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0:", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0:0", true },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0:0:", false },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0:0:0", true },
//t    { "EST+5EDT-0,M01.1.6/0:0:0,M12.1.6/0:0:0:", false },
//t
//t    { "EST+5,M01.1.6/0:0:0,M12.1.6", true },
//t    { "EST+5,6/0:0:0,2/0:0:0", true },
//t
//t    // Misc examples
//t    { "NZST-12:00:00NZDT-13:00:00,M10.1.0,M3.3.0", true },
//t    { "EST5EDT", true },
//t    { "GMT0", true },
//t    { "JST-9", true },
//t    { "MET-1MEST", true },
//t    { "MST7MDT", true },
//t    { "PST8PDT", true },
//t
//t};
//t
//t
//t//---------------------------------------------------------------------------------------
//tint main(int argc, char *argv[])
//t{
//t    bool result;
//t    int i;
//t    int failure_count = 0;
//t
//t    for (i=0; i<NUM_ELEM(tests); i++)
//t    {
//t        printf("%s", tests[i].timezone);
//t        result = mifd_tzvalidate(tests[i].timezone);
//t        printf("    result=%d", result);
//t        if (result == tests[i].result)
//t        {
//t            printf("   OK");
//t        }
//t        else
//t        {
//t            printf("   ERROR expected result=%d", tests[i].result);
//t            failure_count++;
//t        }
//t        printf("\n");
//t    }
//t
//t    if (failure_count > 0)
//t    {
//t        printf("\nERROR: failure count=%d\n", failure_count);
//t    }
//t    else
//t    {
//t        printf("\nOK. All tests passed\n");
//t    }
//t
//t    return 0;
//t}
#endif

#endif // REMOVE_DEVICE_TIME























