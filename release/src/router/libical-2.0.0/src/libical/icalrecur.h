/*======================================================================
 FILE: icalrecur.h
 CREATOR: eric 20 March 2000

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
========================================================================*/

/**
@file icalrecur.h
@brief Routines for dealing with recurring time

How to use:

1) Get a rule and a start time from a component

@code
        icalproperty rrule;
        struct icalrecurrencetype recur;
        struct icaltimetype dtstart;

        rrule = icalcomponent_get_first_property(comp,ICAL_RRULE_PROPERTY);
        recur = icalproperty_get_rrule(rrule);
        start = icalproperty_get_dtstart(dtstart);
@endcode

Or, just make them up:

@code
        recur = icalrecurrencetype_from_string("FREQ=YEARLY;BYDAY=SU,WE");
        dtstart = icaltime_from_string("19970101T123000")
@endcode

2) Create an iterator

@code
        icalrecur_iterator* ritr;
        ritr = icalrecur_iterator_new(recur,start);
@endcode

3) Iterator over the occurrences

@code
        struct icaltimetype next;
        while (next = icalrecur_iterator_next(ritr)
               && !icaltime_is_null_time(next){
                Do something with next
        }
@endcode

Note that that the time returned by icalrecur_iterator_next is in
whatever timezone that dtstart is in.

*/

#ifndef ICALRECUR_H
#define ICALRECUR_H

#include "libical_ical_export.h"
#include "icalarray.h"
#include "icaltime.h"

/*
 * Recurrence enumerations
 */

typedef enum icalrecurrencetype_frequency
{
    /* These enums are used to index an array, so don't change the
       order or the integers */

    ICAL_SECONDLY_RECURRENCE = 0,
    ICAL_MINUTELY_RECURRENCE = 1,
    ICAL_HOURLY_RECURRENCE = 2,
    ICAL_DAILY_RECURRENCE = 3,
    ICAL_WEEKLY_RECURRENCE = 4,
    ICAL_MONTHLY_RECURRENCE = 5,
    ICAL_YEARLY_RECURRENCE = 6,
    ICAL_NO_RECURRENCE = 7
} icalrecurrencetype_frequency;

typedef enum icalrecurrencetype_weekday
{
    ICAL_NO_WEEKDAY,
    ICAL_SUNDAY_WEEKDAY,
    ICAL_MONDAY_WEEKDAY,
    ICAL_TUESDAY_WEEKDAY,
    ICAL_WEDNESDAY_WEEKDAY,
    ICAL_THURSDAY_WEEKDAY,
    ICAL_FRIDAY_WEEKDAY,
    ICAL_SATURDAY_WEEKDAY
} icalrecurrencetype_weekday;

typedef enum icalrecurrencetype_skip
{
    ICAL_SKIP_BACKWARD = 0,
    ICAL_SKIP_FORWARD,
    ICAL_SKIP_OMIT,
    ICAL_SKIP_UNDEFINED
} icalrecurrencetype_skip;

enum icalrecurrence_array_max_values
{
    ICAL_RECURRENCE_ARRAY_MAX = 0x7f7f,
    ICAL_RECURRENCE_ARRAY_MAX_BYTE = 0x7f
};

/*
 * Recurrence enumerations conversion routines.
 */

LIBICAL_ICAL_EXPORT icalrecurrencetype_frequency icalrecur_string_to_freq(const char *str);
LIBICAL_ICAL_EXPORT const char *icalrecur_freq_to_string(icalrecurrencetype_frequency kind);

LIBICAL_ICAL_EXPORT icalrecurrencetype_skip icalrecur_string_to_skip(const char *str);
LIBICAL_ICAL_EXPORT const char *icalrecur_skip_to_string(icalrecurrencetype_skip kind);

LIBICAL_ICAL_EXPORT const char *icalrecur_weekday_to_string(icalrecurrencetype_weekday kind);
LIBICAL_ICAL_EXPORT icalrecurrencetype_weekday icalrecur_string_to_weekday(const char *str);

/**
 * Recurrence type routines
 */

/* See RFC 5545 Section 3.3.10, RECUR Value, and RFC 7529
 * for an explanation of the values and fields in struct icalrecurrencetype.
 *
 * The maximums below are based on lunisolar leap years (13 months)
 */
#define ICAL_BY_SECOND_SIZE     62      /* 0 to 60 */
#define ICAL_BY_MINUTE_SIZE     61      /* 0 to 59 */
#define ICAL_BY_HOUR_SIZE       25      /* 0 to 23 */
#define ICAL_BY_MONTH_SIZE      14      /* 1 to 13 */
#define ICAL_BY_MONTHDAY_SIZE   32      /* 1 to 31 */
#define ICAL_BY_WEEKNO_SIZE     56      /* 1 to 55 */
#define ICAL_BY_YEARDAY_SIZE    386     /* 1 to 385 */
#define ICAL_BY_SETPOS_SIZE     ICAL_BY_YEARDAY_SIZE          /* 1 to N */
#define ICAL_BY_DAY_SIZE        7*(ICAL_BY_WEEKNO_SIZE-1)+1   /* 1 to N */

/** Main struct for holding digested recurrence rules */
struct icalrecurrencetype
{
    icalrecurrencetype_frequency freq;

    /* until and count are mutually exclusive. */
    struct icaltimetype until;
    int count;

    short interval;

    icalrecurrencetype_weekday week_start;

    /* The BY* parameters can each take a list of values. Here I
     * assume that the list of values will not be larger than the
     * range of the value -- that is, the client will not name a
     * value more than once.

     * Each of the lists is terminated with the value
     * ICAL_RECURRENCE_ARRAY_MAX unless the list is full.
     */

    short by_second[ICAL_BY_SECOND_SIZE];
    short by_minute[ICAL_BY_MINUTE_SIZE];
    short by_hour[ICAL_BY_HOUR_SIZE];
    short by_day[ICAL_BY_DAY_SIZE];     /* Encoded value, see below */
    short by_month_day[ICAL_BY_MONTHDAY_SIZE];
    short by_year_day[ICAL_BY_YEARDAY_SIZE];
    short by_week_no[ICAL_BY_WEEKNO_SIZE];
    short by_month[ICAL_BY_MONTH_SIZE];
    short by_set_pos[ICAL_BY_SETPOS_SIZE];

    /* For RSCALE extension (RFC 7529) */
    char *rscale;
    icalrecurrencetype_skip skip;
};

LIBICAL_ICAL_EXPORT int icalrecurrencetype_rscale_is_supported(void);

LIBICAL_ICAL_EXPORT icalarray *icalrecurrencetype_rscale_supported_calendars(void);

LIBICAL_ICAL_EXPORT void icalrecurrencetype_clear(struct icalrecurrencetype *r);

/**
 * Array Encoding
 *
 * The 'day' element of the by_day array is encoded to allow
 * representation of both the day of the week ( Monday, Tueday), but also
 * the Nth day of the week ( First tuesday of the month, last thursday of
 * the year) These routines decode the day values
 */

/** 1 == Monday, etc. */
LIBICAL_ICAL_EXPORT enum icalrecurrencetype_weekday icalrecurrencetype_day_day_of_week(short day);

/** 0 == any of day of week. 1 == first, 2 = second, -2 == second to last, etc */
LIBICAL_ICAL_EXPORT int icalrecurrencetype_day_position(short day);

/**
 * The 'month' element of the by_month array is encoded to allow
 * representation of the "L" leap suffix (RFC 7529).
 * These routines decode the month values.
 */

LIBICAL_ICAL_EXPORT int icalrecurrencetype_month_is_leap(short month);

LIBICAL_ICAL_EXPORT int icalrecurrencetype_month_month(short month);

/** Recurrance rule parser */

/** Convert between strings and recurrencetype structures. */
LIBICAL_ICAL_EXPORT struct icalrecurrencetype icalrecurrencetype_from_string(const char *str);

LIBICAL_ICAL_EXPORT char *icalrecurrencetype_as_string(struct icalrecurrencetype *recur);

LIBICAL_ICAL_EXPORT char *icalrecurrencetype_as_string_r(struct icalrecurrencetype *recur);

/** Recurrence iteration routines */

typedef struct icalrecur_iterator_impl icalrecur_iterator;

/** Create a new recurrence rule iterator */
LIBICAL_ICAL_EXPORT icalrecur_iterator *icalrecur_iterator_new(struct icalrecurrencetype rule,
                                                               struct icaltimetype dtstart);

/** Get the next occurrence from an iterator */
LIBICAL_ICAL_EXPORT struct icaltimetype icalrecur_iterator_next(icalrecur_iterator *);

/** Free the iterator */
LIBICAL_ICAL_EXPORT void icalrecur_iterator_free(icalrecur_iterator *);

/**
 * Fills array up with at most 'count' time_t values, each
 *  representing an occurrence time in seconds past the POSIX epoch
 */
LIBICAL_ICAL_EXPORT int icalrecur_expand_recurrence(char *rule, time_t start,
                                                    int count, time_t * array);

#endif
