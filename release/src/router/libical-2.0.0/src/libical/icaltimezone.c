/*======================================================================
 FILE: icaltimezone.c
 CREATOR: Damon Chaplin 15 March 2001

 (C) COPYRIGHT 2001, Damon Chaplin <damon@ximian.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/
//krazy:excludeall=cpp

/** @file icaltimezone.c
 *  @brief implementation of timezone handling routines
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icaltimezone.h"
#include "icaltimezoneimpl.h"
#include "icalarray.h"
#include "icalerror.h"
#include "icalparser.h"
#include "icaltz-util.h"

#include <ctype.h>
#include <stddef.h>     /* for ptrdiff_t */
#include <stdlib.h>

#if defined(HAVE_PTHREAD)
#include <pthread.h>
#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
// It seems the same thread can attempt to lock builtin_mutex multiple times
// (at least when using builtin tzdata), so make it builtin_mutex recursive:
static pthread_mutex_t builtin_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
static pthread_mutex_t builtin_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

#if defined(_WIN32)
#if !defined(_WIN32_WCE)
#include <mbstring.h>
#endif
#include <windows.h>
#endif

/** This is the toplevel directory where the timezone data is installed in. */
#define ZONEINFO_DIRECTORY      PACKAGE_DATA_DIR "/zoneinfo"

/** The prefix we use to uniquely identify TZIDs.
    It must begin and end with forward slashes.
 */
static const char *ical_tzid_prefix = "/freeassociation.sourceforge.net/";

/** This is the filename of the file containing the city names and
    coordinates of all the builtin timezones. */
#define ZONES_TAB_FILENAME      "zones.tab"

/** This is the number of years of extra coverage we do when expanding
    the timezone changes. */
#define ICALTIMEZONE_EXTRA_COVERAGE     5

#if (SIZEOF_TIME_T > 4)
/** Arbitrarily go up to 1000th anniversary of Gregorian calendar, since
    64-bit time_t values get us up to the tm_year limit of 2+ billion years. */
#define ICALTIMEZONE_MAX_YEAR           2582
#else
/** This is the maximum year we will expand to, since 32-bit time_t values
    only go up to the start of 2038. */
#define ICALTIMEZONE_MAX_YEAR           2037
#endif

typedef struct _icaltimezonechange icaltimezonechange;

struct _icaltimezonechange
{
    int utc_offset;
    /**< The offset to add to UTC to get local time, in seconds. */

    int prev_utc_offset;
    /**< The offset to add to UTC, before this change, in seconds. */

    int year;                   /**< Actual year, e.g. 2001. */
    int month;                  /**< 1 (Jan) to 12 (Dec). */
    int day;
    int hour;
    int minute;
    int second;
    /**< The time that the change came into effect, in UTC.
       Note that the prev_utc_offset applies to this local time,
       since we haven't changed to the new offset yet. */

    int is_daylight;
    /**< Whether this is STANDARD or DAYLIGHT time. */
};

/** An array of icaltimezones for the builtin timezones. */
static icalarray *builtin_timezones = NULL;

/** This is the special UTC timezone, which isn't in builtin_timezones. */
static icaltimezone utc_timezone = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *zone_files_directory = NULL;

#if defined(USE_BUILTIN_TZDATA)
static int use_builtin_tzdata = 1;
#else
static int use_builtin_tzdata = 0;
#endif

static void icaltimezone_reset(icaltimezone *zone);
static void icaltimezone_expand_changes(icaltimezone *zone, int end_year);
static int icaltimezone_compare_change_fn(const void *elem1, const void *elem2);

static size_t icaltimezone_find_nearby_change(icaltimezone *zone, icaltimezonechange *change);

static void icaltimezone_adjust_change(icaltimezonechange *tt,
                                       int days, int hours, int minutes, int seconds);

static void icaltimezone_init(icaltimezone *zone);

/** Gets the TZID, LOCATION/X-LIC-LOCATION, and TZNAME properties from the
   VTIMEZONE component and places them in the icaltimezone. It returns 1 on
   success, or 0 if the TZID can't be found. */
static int icaltimezone_get_vtimezone_properties(icaltimezone *zone, icalcomponent *component);

static void icaltimezone_load_builtin_timezone(icaltimezone *zone);

static void icaltimezone_ensure_coverage(icaltimezone *zone, int end_year);

static void icaltimezone_init_builtin_timezones(void);

static void icaltimezone_parse_zone_tab(void);

static char *icaltimezone_load_get_line_fn(char *s, size_t size, void *data);

static void format_utc_offset(int utc_offset, char *buffer, size_t buffer_size);
static const char *get_zone_directory(void);

const char *icaltimezone_tzid_prefix(void)
{
    return ical_tzid_prefix;
}

/** Creates a new icaltimezone. */
icaltimezone *icaltimezone_new(void)
{
    icaltimezone *zone;

    zone = (icaltimezone *) malloc(sizeof(icaltimezone));
    if (!zone) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        return NULL;
    }

    icaltimezone_init(zone);

    return zone;
}

icaltimezone *icaltimezone_copy(icaltimezone *originalzone)
{
    icaltimezone *zone;

    zone = (icaltimezone *) malloc(sizeof(icaltimezone));
    if (!zone) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        return NULL;
    }

    memcpy(zone, originalzone, sizeof(icaltimezone));
    if (zone->tzid != NULL) {
        zone->tzid = strdup(zone->tzid);
    }
    if (zone->location != NULL) {
        zone->location = strdup(zone->location);
    }
    if (zone->tznames != NULL) {
        zone->tznames = strdup(zone->tznames);
    }
    if (zone->changes != NULL) {
        zone->changes = icalarray_copy(zone->changes);
    }

    /* Let the caller set the component because then they will
       know to be careful not to free this reference twice. */
    zone->component = NULL;

    return zone;
}

/** Frees all memory used for the icaltimezone. */
void icaltimezone_free(icaltimezone *zone, int free_struct)
{
    icaltimezone_reset(zone);
    if (free_struct)
        free(zone);
}

/** Resets the icaltimezone to the initial state, freeing most of the fields. */
static void icaltimezone_reset(icaltimezone *zone)
{
    if (zone->tzid)
        free(zone->tzid);

    if (zone->location)
        free(zone->location);

    if (zone->tznames)
        free(zone->tznames);

    if (zone->component)
        icalcomponent_free(zone->component);

    if (zone->changes)
        icalarray_free(zone->changes);

    icaltimezone_init(zone);
}

/** Initializes an icaltimezone. */
static void icaltimezone_init(icaltimezone *zone)
{
    zone->tzid = NULL;
    zone->location = NULL;
    zone->tznames = NULL;
    zone->latitude = 0.0;
    zone->longitude = 0.0;
    zone->component = NULL;
    zone->builtin_timezone = NULL;
    zone->end_year = 0;
    zone->changes = NULL;
}

/** Gets the TZID, LOCATION/X-LIC-LOCATION and TZNAME properties of
   the VTIMEZONE component and stores them in the icaltimezone.  It
   returns 1 on success, or 0 if the TZID can't be found.  Note that
   it expects the zone to be initialized or reset - it doesn't free
   any old values. */
static int icaltimezone_get_vtimezone_properties(icaltimezone *zone, icalcomponent *component)
{
    icalproperty *prop;
    const char *tzid;

    prop = icalcomponent_get_first_property(component, ICAL_TZID_PROPERTY);
    if (!prop)
        return 0;

    /* A VTIMEZONE MUST have a TZID, or a lot of our code won't work. */
    tzid = icalproperty_get_tzid(prop);
    if (!tzid) {
        return 0;
    }

    if (zone->tzid) {
        free(zone->tzid);
    }
    zone->tzid = strdup(tzid);

    if (zone->component) {
        icalcomponent_free(zone->component);
    }
    zone->component = component;

    if (zone->location) {
        free(zone->location);
    }
    zone->location = icaltimezone_get_location_from_vtimezone(component);

    if (zone->tznames) {
        free(zone->tznames);
    }
    zone->tznames = icaltimezone_get_tznames_from_vtimezone(component);

    return 1;
}

/** Gets the LOCATION or X-LIC-LOCATION property from a VTIMEZONE. */
char *icaltimezone_get_location_from_vtimezone(icalcomponent *component)
{
    icalproperty *prop;
    const char *location;
    const char *name;

    prop = icalcomponent_get_first_property(component, ICAL_LOCATION_PROPERTY);
    if (prop) {
        location = icalproperty_get_location(prop);
        if (location)
            return strdup(location);
    }

    prop = icalcomponent_get_first_property(component, ICAL_X_PROPERTY);
    while (prop) {
        name = icalproperty_get_x_name(prop);
        if (name && !strcasecmp(name, "X-LIC-LOCATION")) {
            location = icalproperty_get_x(prop);
            if (location)
                return strdup(location);
        }
        prop = icalcomponent_get_next_property(component, ICAL_X_PROPERTY);
    }

    return NULL;
}

/** Gets the TZNAMEs used for the last STANDARD & DAYLIGHT components
   in a VTIMEZONE. If both STANDARD and DAYLIGHT components use the
   same TZNAME, it returns that. If they use different TZNAMEs, it
   formats them like "EST/EDT". The returned string should be freed by
   the caller. */
char *icaltimezone_get_tznames_from_vtimezone(icalcomponent *component)
{
    icalcomponent *comp;
    icalcomponent_kind type;
    icalproperty *prop;
    struct icaltimetype dtstart;
    struct icaldatetimeperiodtype rdate;
    const char *current_tzname;
    const char *standard_tzname = NULL, *daylight_tzname = NULL;
    struct icaltimetype standard_max_date, daylight_max_date;
    struct icaltimetype current_max_date;

    standard_max_date = icaltime_null_time();
    daylight_max_date = icaltime_null_time();

    /* Step through the STANDARD & DAYLIGHT subcomponents. */
    comp = icalcomponent_get_first_component(component, ICAL_ANY_COMPONENT);
    while (comp) {
        type = icalcomponent_isa(comp);
        if (type == ICAL_XSTANDARD_COMPONENT || type == ICAL_XDAYLIGHT_COMPONENT) {
            current_max_date = icaltime_null_time();
            current_tzname = NULL;

            /* Step through the properties. We want to find the TZNAME, and
               the largest DTSTART or RDATE. */
            prop = icalcomponent_get_first_property(comp, ICAL_ANY_PROPERTY);
            while (prop) {
                switch (icalproperty_isa(prop)) {
                case ICAL_TZNAME_PROPERTY:
                    current_tzname = icalproperty_get_tzname(prop);
                    break;

                case ICAL_DTSTART_PROPERTY:
                    dtstart = icalproperty_get_dtstart(prop);
                    if (icaltime_compare(dtstart, current_max_date) > 0)
                        current_max_date = dtstart;

                    break;

                case ICAL_RDATE_PROPERTY:
                    rdate = icalproperty_get_rdate(prop);
                    if (icaltime_compare(rdate.time, current_max_date) > 0)
                        current_max_date = rdate.time;

                    break;

                default:
                    break;
                }

                prop = icalcomponent_get_next_property(comp, ICAL_ANY_PROPERTY);
            }

            if (current_tzname) {
                if (type == ICAL_XSTANDARD_COMPONENT) {
                    if (!standard_tzname ||
                        icaltime_compare(current_max_date, standard_max_date) > 0) {
                        standard_max_date = current_max_date;
                        standard_tzname = current_tzname;
                    }
                } else {
                    if (!daylight_tzname ||
                        icaltime_compare(current_max_date, daylight_max_date) > 0) {
                        daylight_max_date = current_max_date;
                        daylight_tzname = current_tzname;
                    }
                }
            }
        }

        comp = icalcomponent_get_next_component(component, ICAL_ANY_COMPONENT);
    }

    /* Outlook (2000) places "Standard Time" and "Daylight Time" in the TZNAME
       strings, which is totally useless. So we return NULL in that case. */
    if (standard_tzname && !strcmp(standard_tzname, "Standard Time"))
        return NULL;

    /* If both standard and daylight TZNAMEs were found, if they are the same
       we return just one, else we format them like "EST/EDT". */
    if (standard_tzname && daylight_tzname) {
        size_t standard_len, daylight_len;
        char *tznames;

        if (!strcmp(standard_tzname, daylight_tzname))
            return strdup(standard_tzname);

        standard_len = strlen(standard_tzname);
        daylight_len = strlen(daylight_tzname);
        tznames = malloc(standard_len + daylight_len + 2);
        strcpy(tznames, standard_tzname);
        tznames[standard_len] = '/';
        strcpy(tznames + standard_len + 1, daylight_tzname);
        return tznames;
    } else {
        const char *tznames;

        /* If either of the TZNAMEs was found just return that, else NULL. */
        tznames = standard_tzname ? standard_tzname : daylight_tzname;
        return tznames ? strdup(tznames) : NULL;
    }
}

static void icaltimezone_ensure_coverage(icaltimezone *zone, int end_year)
{
    /* When we expand timezone changes we always expand at least up to this
       year, plus ICALTIMEZONE_EXTRA_COVERAGE. */
    static int icaltimezone_minimum_expansion_year = -1;

    int changes_end_year;

    icaltimezone_load_builtin_timezone(zone);

    if (icaltimezone_minimum_expansion_year == -1) {
        struct icaltimetype today = icaltime_today();

        icaltimezone_minimum_expansion_year = today.year;
    }

    changes_end_year = end_year;
    if (changes_end_year < icaltimezone_minimum_expansion_year)
        changes_end_year = icaltimezone_minimum_expansion_year;

    changes_end_year += ICALTIMEZONE_EXTRA_COVERAGE;

    if (changes_end_year > ICALTIMEZONE_MAX_YEAR)
        changes_end_year = ICALTIMEZONE_MAX_YEAR;

    if (!zone->changes || zone->end_year < end_year)
        icaltimezone_expand_changes(zone, changes_end_year);
}

static void icaltimezone_expand_changes(icaltimezone *zone, int end_year)
{
    icalarray *changes;
    icalcomponent *comp;

#if 0
    printf("\nExpanding changes for: %s to year: %i\n", zone->tzid, end_year);
#endif

    changes = icalarray_new(sizeof(icaltimezonechange), 32);
    if (!changes)
        return;

    /* Scan the STANDARD and DAYLIGHT subcomponents. */
    comp = icalcomponent_get_first_component(zone->component, ICAL_ANY_COMPONENT);
    while (comp) {
        icaltimezone_expand_vtimezone(comp, end_year, changes);
        comp = icalcomponent_get_next_component(zone->component, ICAL_ANY_COMPONENT);
    }

    /* Sort the changes. We may have duplicates but I don't think it will
       matter. */
    icalarray_sort(changes, icaltimezone_compare_change_fn);

    if (zone->changes)
        icalarray_free(zone->changes);

    zone->changes = changes;
    zone->end_year = end_year;
}

void icaltimezone_expand_vtimezone(icalcomponent *comp, int end_year, icalarray *changes)
{
    icaltimezonechange change;
    icalproperty *prop;
    struct icaltimetype dtstart, occ;
    struct icalrecurrencetype rrule;
    icalrecur_iterator *rrule_iterator;
    struct icaldatetimeperiodtype rdate;
    int found_dtstart = 0, found_tzoffsetto = 0, found_tzoffsetfrom = 0;
    int has_rdate = 0, has_rrule = 0;

    /* First we check if it is a STANDARD or DAYLIGHT component, and
       just return if it isn't. */
    if (icalcomponent_isa(comp) == ICAL_XSTANDARD_COMPONENT) {
        change.is_daylight = 0;
    } else if (icalcomponent_isa(comp) == ICAL_XDAYLIGHT_COMPONENT) {
        change.is_daylight = 1;
    } else {
        return;
    }

    /* Step through each of the properties to find the DTSTART,
       TZOFFSETFROM and TZOFFSETTO. We can't expand recurrences here
       since we need these properties before we can do that. */
    prop = icalcomponent_get_first_property(comp, ICAL_ANY_PROPERTY);
    while (prop) {
        switch (icalproperty_isa(prop)) {
        case ICAL_DTSTART_PROPERTY:
            dtstart = icalproperty_get_dtstart(prop);
            found_dtstart = 1;
            break;
        case ICAL_TZOFFSETTO_PROPERTY:
            change.utc_offset = icalproperty_get_tzoffsetto(prop);
            /*printf ("Found TZOFFSETTO: %i\n", change.utc_offset); */
            found_tzoffsetto = 1;
            break;
        case ICAL_TZOFFSETFROM_PROPERTY:
            change.prev_utc_offset = icalproperty_get_tzoffsetfrom(prop);
            /*printf ("Found TZOFFSETFROM: %i\n", change.prev_utc_offset); */
            found_tzoffsetfrom = 1;
            break;
        case ICAL_RDATE_PROPERTY:
            has_rdate = 1;
            break;
        case ICAL_RRULE_PROPERTY:
            has_rrule = 1;
            break;
        default:
            /* Just ignore any other properties. */
            break;
        }

        prop = icalcomponent_get_next_property(comp, ICAL_ANY_PROPERTY);
    }

    /* Microsoft Outlook for Mac (and possibly other versions) will create
       timezones without a tzoffsetfrom property if it's a timezone that
       doesn't change for DST. */
    if (found_tzoffsetto && !found_tzoffsetfrom) {
        change.prev_utc_offset = change.utc_offset;
        found_tzoffsetfrom = 1;
    }

    /* If we didn't find a DTSTART, TZOFFSETTO and TZOFFSETFROM we have to
       ignore the component. FIXME: Add an error property? */
    if (!found_dtstart || !found_tzoffsetto || !found_tzoffsetfrom)
        return;

#if 0
    printf("\n Expanding component DTSTART (Y/M/D): %i/%i/%i %i:%02i:%02i\n",
           dtstart.year, dtstart.month, dtstart.day, dtstart.hour, dtstart.minute, dtstart.second);
#endif

    /* If the STANDARD/DAYLIGHT component has no recurrence rule, we add
       a single change for the DTSTART. */
    if (!has_rrule) {
        change.year = dtstart.year;
        change.month = dtstart.month;
        change.day = dtstart.day;
        change.hour = dtstart.hour;
        change.minute = dtstart.minute;
        change.second = dtstart.second;

        /* Convert to UTC. */
        icaltimezone_adjust_change(&change, 0, 0, 0, -change.prev_utc_offset);

#if 0
        printf("  Appending single DTSTART (Y/M/D): %i/%02i/%02i %i:%02i:%02i\n",
               change.year, change.month, change.day, change.hour, change.minute, change.second);
#endif

        /* Add the change to the array. */
        icalarray_append(changes, &change);
    }

    /* The component has recurrence data, so we expand that now. */
    prop = icalcomponent_get_first_property(comp, ICAL_ANY_PROPERTY);
    while (prop && (has_rdate || has_rrule)) {
#if 0
        printf("Expanding property...\n");
#endif
        switch (icalproperty_isa(prop)) {
        case ICAL_RDATE_PROPERTY:
            rdate = icalproperty_get_rdate(prop);
            change.year = rdate.time.year;
            change.month = rdate.time.month;
            change.day = rdate.time.day;
            /* RDATEs with a DATE value inherit the time from
               the DTSTART. */
            if (icaltime_is_date(rdate.time)) {
                change.hour = dtstart.hour;
                change.minute = dtstart.minute;
                change.second = dtstart.second;
            } else {
                change.hour = rdate.time.hour;
                change.minute = rdate.time.minute;
                change.second = rdate.time.second;

                /* The spec was a bit vague about whether RDATEs were in local
                   time or UTC so we support both to be safe. So if it is in
                   UTC we have to add the UTC offset to get a local time. */
                if (!icaltime_is_utc(rdate.time))
                    icaltimezone_adjust_change(&change, 0, 0, 0, -change.prev_utc_offset);
            }

#if 0
            printf("  Appending RDATE element (Y/M/D): %i/%02i/%02i %i:%02i:%02i\n",
                   change.year, change.month, change.day,
                   change.hour, change.minute, change.second);
#endif

            icalarray_append(changes, &change);
            break;
        case ICAL_RRULE_PROPERTY:
            rrule = icalproperty_get_rrule(prop);

            /* If the rrule UNTIL value is set and is in UTC, we convert it to
               a local time, since the recurrence code has no way to convert
               it itself. */
            if (!icaltime_is_null_time(rrule.until) && rrule.until.is_utc) {
#if 0
                printf("  Found RRULE UNTIL in UTC.\n");
#endif

                /* To convert from UTC to a local time, we use the TZOFFSETFROM
                   since that is the offset from UTC that will be in effect
                   when each of the RRULE occurrences happens. */
                icaltime_adjust(&rrule.until, 0, 0, 0, change.prev_utc_offset);
                rrule.until.is_utc = 0;
            }

            /* Add the dtstart to changes, otherwise some oddly-defined VTIMEZONE
               components can cause the first year to get skipped. */
            change.year = dtstart.year;
            change.month = dtstart.month;
            change.day = dtstart.day;
            change.hour = dtstart.hour;
            change.minute = dtstart.minute;
            change.second = dtstart.second;

#if 0
            printf("  Appending RRULE element (Y/M/D): %i/%02i/%02i %i:%02i:%02i\n",
                   change.year, change.month, change.day,
                   change.hour, change.minute, change.second);
#endif

            icaltimezone_adjust_change(&change, 0, 0, 0, -change.prev_utc_offset);

            icalarray_append(changes, &change);

            rrule_iterator = icalrecur_iterator_new(rrule, dtstart);
            for (; rrule_iterator;) {
                occ = icalrecur_iterator_next(rrule_iterator);
                /* Skip dtstart since we just added it */
                if (icaltime_compare(dtstart, occ) == 0) {
                    continue;
                }
                if (occ.year > end_year || icaltime_is_null_time(occ)) {
                    break;
                }
                change.year = occ.year;
                change.month = occ.month;
                change.day = occ.day;
                change.hour = occ.hour;
                change.minute = occ.minute;
                change.second = occ.second;

#if 0
                printf("  Appending RRULE element (Y/M/D): %i/%02i/%02i %i:%02i:%02i\n",
                       change.year, change.month, change.day,
                       change.hour, change.minute, change.second);
#endif

                icaltimezone_adjust_change(&change, 0, 0, 0, -change.prev_utc_offset);

                icalarray_append(changes, &change);
            }

            icalrecur_iterator_free(rrule_iterator);
            break;
        default:
            break;
        }

        prop = icalcomponent_get_next_property(comp, ICAL_ANY_PROPERTY);
    }
}

/** A function to compare 2 icaltimezonechange elements, used for qsort(). */
static int icaltimezone_compare_change_fn(const void *elem1, const void *elem2)
{
    const icaltimezonechange *change1, *change2;
    int retval;

    change1 = (const icaltimezonechange *)elem1;
    change2 = (const icaltimezonechange *)elem2;

    if (change1->year < change2->year) {
        retval = -1;
    } else if (change1->year > change2->year) {
        retval = 1;
    } else if (change1->month < change2->month) {
        retval = -1;
    } else if (change1->month > change2->month) {
        retval = 1;
    } else if (change1->day < change2->day) {
        retval = -1;
    } else if (change1->day > change2->day) {
        retval = 1;
    } else if (change1->hour < change2->hour) {
        retval = -1;
    } else if (change1->hour > change2->hour) {
        retval = 1;
    } else if (change1->minute < change2->minute) {
        retval = -1;
    } else if (change1->minute > change2->minute) {
        retval = 1;
    } else if (change1->second < change2->second) {
        retval = -1;
    } else if (change1->second > change2->second) {
        retval = 1;
    } else {
        retval = 0;
    }

    return retval;
}

void icaltimezone_convert_time(struct icaltimetype *tt,
                               icaltimezone *from_zone, icaltimezone *to_zone)
{
    int utc_offset, is_daylight;

    /* If the time is a DATE value or both timezones are the same, or we are
       converting a floating time, we don't need to do anything. */
    if (icaltime_is_date(*tt) || from_zone == to_zone || from_zone == NULL)
        return;

    /* Convert the time to UTC by getting the UTC offset and subtracting it. */
    utc_offset = icaltimezone_get_utc_offset(from_zone, tt, NULL);
    icaltime_adjust(tt, 0, 0, 0, -utc_offset);

    /* Now we convert the time to the new timezone by getting the UTC offset
       of our UTC time and adding it. */
    utc_offset = icaltimezone_get_utc_offset_of_utc_time(to_zone, tt, &is_daylight);
    tt->is_daylight = is_daylight;
    icaltime_adjust(tt, 0, 0, 0, utc_offset);
}

/** @deprecated This API wasn't updated when we changed icaltimetype to contain its own
    timezone. Also, this takes a pointer instead of the struct. */
/* Calculates the UTC offset of a given local time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
int icaltimezone_get_utc_offset(icaltimezone *zone, struct icaltimetype *tt, int *is_daylight)
{
    icaltimezonechange *zone_change, *prev_zone_change, tt_change, tmp_change;
    size_t change_num, change_num_to_use;
    int found_change;
    int step, utc_offset_change, cmp;
    int want_daylight;

    if (tt == NULL)
        return 0;

    if (is_daylight)
        *is_daylight = 0;

    /* For local times and UTC return 0. */
    if (zone == NULL || zone == &utc_timezone)
        return 0;

    /* Use the builtin icaltimezone if possible. */
    if (zone->builtin_timezone)
        zone = zone->builtin_timezone;

    /* Make sure the changes array is expanded up to the given time. */
    icaltimezone_ensure_coverage(zone, tt->year);

    if (!zone->changes || zone->changes->num_elements == 0)
        return 0;

    /* Copy the time parts of the icaltimetype to an icaltimezonechange so we
       can use our comparison function on it. */
    tt_change.year = tt->year;
    tt_change.month = tt->month;
    tt_change.day = tt->day;
    tt_change.hour = tt->hour;
    tt_change.minute = tt->minute;
    tt_change.second = tt->second;

    /* This should find a change close to the time, either the change before
       it or the change after it. */
    change_num = icaltimezone_find_nearby_change(zone, &tt_change);

    /* Now move backwards or forwards to find the timezone change that applies
       to tt. It should only have to do 1 or 2 steps. */
    zone_change = icalarray_element_at(zone->changes, change_num);
    step = 1;
    found_change = 0;
    change_num_to_use = -1;
    for (;;) {
        /* Copy the change, so we can adjust it. */
        tmp_change = *zone_change;

        /* If the clock is going backward, check if it is in the region of time
           that is used twice. If it is, use the change with the daylight
           setting which matches tt, or use standard if we don't know. */
        if (tmp_change.utc_offset < tmp_change.prev_utc_offset) {
            /* If the time change is at 2:00AM local time and the clock is
               going back to 1:00AM we adjust the change to 1:00AM. We may
               have the wrong change but we'll figure that out later. */
            icaltimezone_adjust_change(&tmp_change, 0, 0, 0, tmp_change.utc_offset);
        } else {
            icaltimezone_adjust_change(&tmp_change, 0, 0, 0, tmp_change.prev_utc_offset);
        }

        cmp = icaltimezone_compare_change_fn(&tt_change, &tmp_change);

        /* If the given time is on or after this change, then this change may
           apply, but we continue as a later change may be the right one.
           If the given time is before this change, then if we have already
           found a change which applies we can use that, else we need to step
           backwards. */
        if (cmp >= 0) {
            found_change = 1;
            change_num_to_use = change_num;
        } else {
            step = -1;
        }

        /* If we are stepping backwards through the changes and we have found
           a change that applies, then we know this is the change to use so
           we exit the loop. */
        if (step == -1 && found_change == 1)
            break;

        /* If we go past the start of the changes array, then we have no data
           for this time so we return the prev UTC offset. */
        if (change_num == 0 && step < 0) {
            if (is_daylight) {
                *is_daylight = ! tmp_change.is_daylight;
            }
            return tmp_change.prev_utc_offset;
        }

        change_num += step;

        if (change_num >= zone->changes->num_elements)
            break;

        zone_change = icalarray_element_at(zone->changes, change_num);
    }

    /* If we didn't find a change to use, then we have a bug! */
    icalerror_assert(found_change == 1, "No applicable timezone change found");

    /* Now we just need to check if the time is in the overlapped region of
       time when clocks go back. */
    zone_change = icalarray_element_at(zone->changes, change_num_to_use);

    utc_offset_change = zone_change->utc_offset - zone_change->prev_utc_offset;
    if (utc_offset_change < 0 && change_num_to_use > 0) {
        tmp_change = *zone_change;
        icaltimezone_adjust_change(&tmp_change, 0, 0, 0, tmp_change.prev_utc_offset);

        if (icaltimezone_compare_change_fn(&tt_change, &tmp_change) < 0) {
            /* The time is in the overlapped region, so we may need to use
               either the current zone_change or the previous one. If the
               time has the is_daylight field set we use the matching change,
               else we use the change with standard time. */
            prev_zone_change = icalarray_element_at(zone->changes, change_num_to_use - 1);

            /* I was going to add an is_daylight flag to struct icaltimetype,
               but iCalendar doesn't let us distinguish between standard and
               daylight time anyway, so there's no point. So we just use the
               standard time instead. */
            want_daylight = (tt->is_daylight == 1) ? 1 : 0;

#if 0
            if (zone_change->is_daylight == prev_zone_change->is_daylight) {
                printf(" **** Same is_daylight setting\n");
            }
#endif

            if (zone_change->is_daylight != want_daylight &&
                prev_zone_change->is_daylight == want_daylight) {
                zone_change = prev_zone_change;
            }
        }
    }

    /* Now we know exactly which timezone change applies to the time, so
       we can return the UTC offset and whether it is a daylight time. */
    if (is_daylight) {
        *is_daylight = zone_change->is_daylight;
    }
    return zone_change->utc_offset;
}

/** @deprecated This API wasn't updated when we changed icaltimetype to contain its own
    timezone. Also, this takes a pointer instead of the struct. */
/** Calculates the UTC offset of a given UTC time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
int icaltimezone_get_utc_offset_of_utc_time(icaltimezone *zone,
                                            struct icaltimetype *tt, int *is_daylight)
{
    icaltimezonechange *zone_change, tt_change, tmp_change;
    size_t change_num, change_num_to_use;
    int found_change = 1;
    int step;

    if (is_daylight)
        *is_daylight = 0;

    /* For local times and UTC return 0. */
    if (zone == NULL || zone == &utc_timezone)
        return 0;

    /* Use the builtin icaltimezone if possible. */
    if (zone->builtin_timezone)
        zone = zone->builtin_timezone;

    /* Make sure the changes array is expanded up to the given time. */
    icaltimezone_ensure_coverage(zone, tt->year);

    if (!zone->changes || zone->changes->num_elements == 0)
        return 0;

    /* Copy the time parts of the icaltimetype to an icaltimezonechange so we
       can use our comparison function on it. */
    tt_change.year = tt->year;
    tt_change.month = tt->month;
    tt_change.day = tt->day;
    tt_change.hour = tt->hour;
    tt_change.minute = tt->minute;
    tt_change.second = tt->second;

    /* This should find a change close to the time, either the change before
       it or the change after it. */
    change_num = icaltimezone_find_nearby_change(zone, &tt_change);

    /* Now move backwards or forwards to find the timezone change that applies
       to tt. It should only have to do 1 or 2 steps. */
    zone_change = icalarray_element_at(zone->changes, change_num);
    step = 1;
    found_change = 0;
    change_num_to_use = -1;
    for (;;) {
        /* Copy the change and adjust it to UTC. */
        tmp_change = *zone_change;

        /* If the given time is on or after this change, then this change may
           apply, but we continue as a later change may be the right one.
           If the given time is before this change, then if we have already
           found a change which applies we can use that, else we need to step
           backwards. */
        if (icaltimezone_compare_change_fn(&tt_change, &tmp_change) >= 0) {
            found_change = 1;
            change_num_to_use = change_num;
        } else {
            step = -1;
        }

        /* If we are stepping backwards through the changes and we have found
           a change that applies, then we know this is the change to use so
           we exit the loop. */
        if (step == -1 && found_change == 1)
            break;

        /* If we go past the start of the changes array, then we have no data
           for this time so we return the prev UTC offset. */
        if (change_num == 0 && step < 0) {
            if (is_daylight) {
                *is_daylight = ! tmp_change.is_daylight;
            }
            return tmp_change.prev_utc_offset;
        }

        change_num += step;

        if (change_num >= zone->changes->num_elements)
            break;

        zone_change = icalarray_element_at(zone->changes, change_num);
    }

    /* If we didn't find a change to use, then we have a bug! */
    icalerror_assert(found_change == 1, "No applicable timezone change found");

    /* Now we know exactly which timezone change applies to the time, so
       we can return the UTC offset and whether it is a daylight time. */
    zone_change = icalarray_element_at(zone->changes, change_num_to_use);
    if (is_daylight)
        *is_daylight = zone_change->is_daylight;

    return zone_change->utc_offset;
}

/** Returns the index of a timezone change which is close to the time
   given in change. */
static size_t icaltimezone_find_nearby_change(icaltimezone *zone, icaltimezonechange * change)
{
    icaltimezonechange *zone_change;
    size_t lower, middle, upper;
    int cmp;

    /* Do a simple binary search. */
    lower = middle = 0;
    upper = zone->changes->num_elements;

    while (lower < upper) {
        middle = (lower + upper) / 2;
        zone_change = icalarray_element_at(zone->changes, middle);
        cmp = icaltimezone_compare_change_fn(change, zone_change);
        if (cmp == 0) {
            break;
        } else if (cmp < 0) {
            upper = middle;
        } else {
            lower = middle + 1;
        }
    }

    return middle;
}

/** Adds (or subtracts) a time from a icaltimezonechange.  NOTE: This
   function is exactly the same as icaltime_adjust() except for the
   type of the first parameter. */
static void icaltimezone_adjust_change(icaltimezonechange *tt,
                                       int days, int hours, int minutes, int seconds)
{
    int second, minute, hour, day;
    int minutes_overflow, hours_overflow, days_overflow;
    int days_in_month;

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

const char *icaltimezone_get_tzid(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return NULL. */
    if (!zone)
        return NULL;

    icaltimezone_load_builtin_timezone(zone);

    return zone->tzid;
}

const char *icaltimezone_get_location(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return NULL. */
    if (!zone)
        return NULL;

    /* Note that for builtin timezones this comes from zones.tab so we don't
       need to check the timezone is loaded here. */
    return zone->location;
}

const char *icaltimezone_get_tznames(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return NULL. */
    if (!zone)
        return NULL;

    icaltimezone_load_builtin_timezone(zone);

    return zone->tznames;
}

/** Returns the latitude of a builtin timezone. */
double icaltimezone_get_latitude(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return 0. */
    if (!zone)
        return 0.0;

    /* Note that for builtin timezones this comes from zones.tab so we don't
       need to check the timezone is loaded here. */
    return zone->latitude;
}

/** Returns the longitude of a builtin timezone. */
double icaltimezone_get_longitude(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return 0. */
    if (!zone)
        return 0.0;

    /* Note that for builtin timezones this comes from zones.tab so we don't
       need to check the timezone is loaded here. */
    return zone->longitude;
}

/** Returns the VTIMEZONE component of a timezone. */
icalcomponent *icaltimezone_get_component(icaltimezone *zone)
{
    /* If this is a floating time, without a timezone, return NULL. */
    if (!zone)
        return NULL;

    icaltimezone_load_builtin_timezone(zone);

    return zone->component;
}

/** Sets the VTIMEZONE component of an icaltimezone, initializing the
   tzid, location & tzname fields. It returns 1 on success or 0 on
   failure, i.e.  no TZID was found. */
int icaltimezone_set_component(icaltimezone *zone, icalcomponent *comp)
{
    icaltimezone_reset(zone);
    return icaltimezone_get_vtimezone_properties(zone, comp);
}

/* Returns the timezone name to display to the user. We prefer to use the
   Olson city name, but fall back on the TZNAME, or finally the TZID. We don't
   want to use "" as it may be wrongly interpreted as a floating time.
   Do not free the returned string. */
const char *icaltimezone_get_display_name(icaltimezone *zone)
{
    const char *display_name;

    display_name = icaltimezone_get_location(zone);
    if (!display_name) {
        display_name = icaltimezone_get_tznames(zone);
    }
    if (!display_name) {
        display_name = icaltimezone_get_tzid(zone);
        /* Outlook will strip out X-LIC-LOCATION property and so all
           we get back in the iTIP replies is the TZID. So we see if
           this is one of our TZIDs and if so we jump to the city name
           at the end of it. */
        if (display_name && !strncmp(display_name, ical_tzid_prefix, strlen(ical_tzid_prefix))) {
#if 0
            /* XXX  The code below makes assumptions about the prefix
               which don't even jive with our default declared up top */
            /* Get the location, which is after the 3rd '/' char. */
            const char *p;
            int num_slashes = 0;

            for (p = display_name; *p; p++) {
                if (*p == '/') {
                    num_slashes++;
                    if (num_slashes == 3)
                        return p + 1;
                }
            }
#else
            /* Skip past our prefix */
            display_name += strlen(ical_tzid_prefix);
#endif
        }
    }

    return display_name;
}

icalarray *icaltimezone_array_new(void)
{
    return icalarray_new(sizeof(icaltimezone), 16);
}

void icaltimezone_array_append_from_vtimezone(icalarray *timezones, icalcomponent *child)
{
    icaltimezone zone;

    icaltimezone_init(&zone);
    if (icaltimezone_get_vtimezone_properties(&zone, child))
        icalarray_append(timezones, &zone);
}

void icaltimezone_array_free(icalarray *timezones)
{
    icaltimezone *zone;
    size_t i;

    if (timezones) {
        for (i = 0; i < timezones->num_elements; i++) {
            zone = icalarray_element_at(timezones, i);
            icaltimezone_free(zone, 0);
        }

        icalarray_free(timezones);
    }
}

/*
 * BUILTIN TIMEZONE HANDLING
 */

/** Returns an icalarray of icaltimezone structs, one for each builtin
   timezone.  This will load and parse the zones.tab file to get the
   timezone names and their coordinates. It will not load the
   VTIMEZONE data for any timezones. */
icalarray *icaltimezone_get_builtin_timezones(void)
{
    if (!builtin_timezones)
        icaltimezone_init_builtin_timezones();

    return builtin_timezones;
}

/** Release builtin timezone memory */
void icaltimezone_free_builtin_timezones(void)
{
    icaltimezone_array_free(builtin_timezones);
    builtin_timezones = 0;
}

/** Returns a single builtin timezone, given its Olson city name. */
icaltimezone *icaltimezone_get_builtin_timezone(const char *location)
{
    icalcomponent *comp;
    icaltimezone *zone;
    size_t lower;
    const char *zone_location;

    if (!location || !location[0])
        return NULL;

    if (!builtin_timezones)
        icaltimezone_init_builtin_timezones();

    if (strcmp(location, "UTC") == 0 || strcmp(location, "GMT") == 0)
        return &utc_timezone;

#if 0
    /* Do a simple binary search. */
    lower = middle = 0;
    upper = builtin_timezones->num_elements;

    while (lower < upper) {
        middle = (lower + upper) / 2;
        zone = icalarray_element_at(builtin_timezones, middle);
        zone_location = icaltimezone_get_location(zone);
        cmp = strcmp(location, zone_location);
        if (cmp == 0) {
            return zone;
        } else if (cmp < 0) {
            upper = middle;
        } else {
            lower = middle + 1;
        }
    }
#endif

    /* The zones from the system are not stored in alphabetical order,
       so we just do a sequential search */
    for (lower = 0; lower < builtin_timezones->num_elements; lower++) {
        zone = icalarray_element_at(builtin_timezones, lower);
        zone_location = icaltimezone_get_location(zone);
        if (strcmp(location, zone_location) == 0)
            return zone;
    }

    /* Check whether file exists, but is not mentioned in zone.tab.
       It means it's a deprecated timezone, but still available. */
    comp = icaltzutil_fetch_timezone(location);
    if (comp) {
        icaltimezone tz;

        icaltimezone_init(&tz);
        if (icaltimezone_set_component(&tz, comp)) {
            icalarray_append(builtin_timezones, &tz);
            return icalarray_element_at(builtin_timezones, builtin_timezones->num_elements - 1);
        } else {
            icalcomponent_free(comp);
        }
    }

    return NULL;
}

static struct icaltimetype tm_to_icaltimetype(struct tm *tm)
{
    struct icaltimetype itt;

    memset(&itt, 0, sizeof(struct icaltimetype));

    itt.second = tm->tm_sec;
    itt.minute = tm->tm_min;
    itt.hour = tm->tm_hour;

    itt.day = tm->tm_mday;
    itt.month = tm->tm_mon + 1;
    itt.year = tm->tm_year + 1900;

    itt.is_utc = 0;
    itt.is_date = 0;

    return itt;
}

static int get_offset(icaltimezone *zone)
{
    struct tm local;
    struct icaltimetype tt;
    int offset;
    const time_t now = time(NULL);

    gmtime_r(&now, &local);
    tt = tm_to_icaltimetype(&local);
    offset = icaltimezone_get_utc_offset(zone, &tt, NULL);

    return offset;
}

/** Returns a single builtin timezone, given its offset from UTC */
icaltimezone *icaltimezone_get_builtin_timezone_from_offset(int offset, const char *tzname)
{
    icaltimezone *zone = NULL;
    size_t i, count;

    if (!builtin_timezones)
        icaltimezone_init_builtin_timezones();

    if (offset == 0)
        return &utc_timezone;

    if (!tzname)
        return NULL;

    count = builtin_timezones->num_elements;

    for (i = 0; i < count; i++) {
        int z_offset;

        zone = icalarray_element_at(builtin_timezones, i);
        icaltimezone_load_builtin_timezone(zone);

        z_offset = get_offset(zone);

        if (z_offset == offset && zone->tznames && !strcmp(tzname, zone->tznames))
            return zone;
    }

    return NULL;
}

/** Returns a single builtin timezone, given its TZID. */
icaltimezone *icaltimezone_get_builtin_timezone_from_tzid(const char *tzid)
{
#if 0
    int num_slashes = 0;
#endif
    const char *p, *zone_tzid;
    icaltimezone *zone;

    if (!tzid || !tzid[0])
        return NULL;

    if (strcmp(tzid, "UTC") == 0 || strcmp(tzid, "GMT") == 0) {
        return icaltimezone_get_builtin_timezone(tzid);
    }

    /* Check that the TZID starts with our unique prefix. */
    if (strncmp(tzid, ical_tzid_prefix, strlen(ical_tzid_prefix)))
        return NULL;

#if 0
    /* XXX  The code below makes assumptions about the prefix
       which don't even jive with our default declared up top */
    /* Get the location, which is after the 3rd '/' character. */
    for (p = tzid; *p; p++) {
        if (*p == '/') {
            num_slashes++;
            if (num_slashes == 3)
                break;
        }
    }

    if (num_slashes != 3)
        return NULL;

    p++;
#else
    /* Skip past our prefix */
    p = tzid + strlen(ical_tzid_prefix);
#endif

    /* Now we can use the function to get the builtin timezone from the
       location string. */
    zone = icaltimezone_get_builtin_timezone(p);
    if (!zone)
        return NULL;

    /* Check that the builtin TZID matches exactly. We don't want to return
       a different version of the VTIMEZONE. */
    zone_tzid = icaltimezone_get_tzid(zone);
    if (!strcmp(zone_tzid, tzid)) {
        return zone;
    } else {
        return NULL;
    }
}

/** Returns the special UTC timezone. */
icaltimezone *icaltimezone_get_utc_timezone(void)
{
    if (!builtin_timezones)
        icaltimezone_init_builtin_timezones();

    return &utc_timezone;
}

/** This initializes the builtin timezone data, i.e. the
   builtin_timezones array and the special UTC timezone. It should be
   called before any code that uses the timezone functions. */
static void icaltimezone_init_builtin_timezones(void)
{
    /* Initialize the special UTC timezone. */
    utc_timezone.tzid = (char *)"UTC";

#if defined(HAVE_PTHREAD)
    pthread_mutex_lock(&builtin_mutex);
#endif
    if (!builtin_timezones) {
        icaltimezone_parse_zone_tab();
    }
#if defined(HAVE_PTHREAD)
    pthread_mutex_unlock(&builtin_mutex);
#endif
}

static int parse_coord(char *coord, int len, int *degrees, int *minutes, int *seconds)
{
    if (len == 5) {
        sscanf(coord + 1, "%2d%2d", degrees, minutes);
    } else if (len == 6) {
        sscanf(coord + 1, "%3d%2d", degrees, minutes);
    } else if (len == 7) {
        sscanf(coord + 1, "%2d%2d%2d", degrees, minutes, seconds);
    } else if (len == 8) {
        sscanf(coord + 1, "%3d%2d%2d", degrees, minutes, seconds);
    } else {
        fprintf(stderr, "Invalid coordinate: %s\n", coord);
        return 1;
    }

    if (coord[0] == '-')
        *degrees = -*degrees;

    return 0;
}

static int fetch_lat_long_from_string(const char *str,
                                      int *latitude_degrees, int *latitude_minutes,
                                      int *latitude_seconds,
                                      int *longitude_degrees, int *longitude_minutes,
                                      int *longitude_seconds,
                                      char *location)
{
    size_t len;
    char *sptr, *lat, *lon, *loc, *temp;

    /* We need to parse the latitude/longitude co-ordinates and location fields  */
    sptr = (char *)str;
    while (*sptr != '\t') {
        sptr++;
    }
    temp = ++sptr;
    while (*sptr != '\t') {
        sptr++;
    }
    len = (ptrdiff_t) (sptr - temp);
    lat = (char *)malloc(len + 1);
    lat = strncpy(lat, temp, len);
    lat[len] = '\0';
    while (*sptr != '\t') {
        sptr++;
    }
    loc = ++sptr;
    while (!isspace((int)(*sptr))) {
        sptr++;
    }
    len = (ptrdiff_t) (sptr - loc);
    location = strncpy(location, loc, len);
    location[len] = '\0';

#if defined(sun) && defined(__SVR4)
    /* Handle EET, MET and WET in zone_sun.tab. */
    if (!strcmp(location, "Europe/")) {
        while (*sptr != '\t') {
            sptr++;
        }
        loc = ++sptr;
        while (!isspace(*sptr)) {
            sptr++;
        }
        len = sptr - loc;
        location = strncpy(location, loc, len);
        location[len] = '\0';
    }
#endif

    lon = lat + 1;
    while (*lon != '\0' && *lon != '+' && *lon != '-') {
        lon++;
    }

    if (parse_coord(lat, (int)(lon - lat),
                    latitude_degrees,
                    latitude_minutes,
                    latitude_seconds) == 1 ||
        parse_coord(lon, (int)strlen(lon),
                    longitude_degrees, longitude_minutes, longitude_seconds) == 1) {
        free(lat);
        return 1;
    }

    free(lat);

    return 0;
}

/** This parses the zones.tab file containing the names and locations
   of the builtin timezones. It creates the builtin_timezones array
   which is an icalarray of icaltimezone structs. It only fills in the
   location, latitude and longtude fields; the rest are left
   blank. The VTIMEZONE component is loaded later if it is needed. The
   timezones in the zones.tab file are sorted by their name, which is
   useful for binary searches. */
static void icaltimezone_parse_zone_tab(void)
{
    const char *zonedir, *zonetab;
    char *filename;
    FILE *fp;
    char buf[1024];     /* Used to store each line of zones.tab as it is read. */
    char location[1024];        /* Stores the city name when parsing buf. */
    size_t filename_len;
    int latitude_degrees = 0, latitude_minutes = 0, latitude_seconds = 0;
    int longitude_degrees = 0, longitude_minutes = 0, longitude_seconds = 0;
    icaltimezone zone;

    icalerror_assert(builtin_timezones == NULL, "Parsing zones.tab file multiple times");

    builtin_timezones = icalarray_new(sizeof(icaltimezone), 1024);

    if (!use_builtin_tzdata) {
        zonedir = icaltzutil_get_zone_directory();
        zonetab = ZONES_TAB_SYSTEM_FILENAME;
    } else {
        zonedir = get_zone_directory();
        zonetab = ZONES_TAB_FILENAME;
    }

    filename_len = 0;
    if (zonedir) {
        filename_len = strlen(zonedir);
    }

    icalerror_assert(filename_len > 0, "Unable to locate a zoneinfo dir");
    if (filename_len == 0) {
        icalerror_set_errno(ICAL_INTERNAL_ERROR);\
        return;
    }

    filename_len += strlen(zonetab);
    filename_len += 2; /* for dir separator and final '\0' */

    filename = (char *)malloc(filename_len);
    if (!filename) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        return;
    }
    snprintf(filename, filename_len, "%s/%s", zonedir, zonetab);

    fp = fopen(filename, "r");
    free(filename);
    icalerror_assert(fp, "Cannot open the zonetab file for reading");
    if (!fp) {
        icalerror_set_errno(ICAL_INTERNAL_ERROR);
        return;
    }

    while (fgets(buf, (int)sizeof(buf), fp)) {
        if (*buf == '#')
            continue;

        if (use_builtin_tzdata) {
            /* The format of each line is: "[ latitude longitude ] location". */
            if (buf[0] != '+' && buf[0] != '-') {
                latitude_degrees = longitude_degrees = 360;
                latitude_minutes = longitude_minutes = 0;
                latitude_seconds = longitude_seconds = 0;
                if (sscanf(buf, "%1000s", location) != 1) {     /*limit location to 1000chars */
                    /*increase as needed */
                    /*see location and buf declarations */
                    fprintf(stderr, "Invalid timezone description line: %s\n", buf);
                    continue;
                }
            } else if (sscanf(buf, "%4d%2d%2d %4d%2d%2d %1000s", /*limit location to 1000chars */
                              /*increase as needed */
                              /*see location and buf declarations */
                              &latitude_degrees, &latitude_minutes,
                              &latitude_seconds,
                              &longitude_degrees, &longitude_minutes,
                              &longitude_seconds, location) != 7) {
                fprintf(stderr, "Invalid timezone description line: %s\n", buf);
                continue;
            }
        } else {
            if (fetch_lat_long_from_string(buf, &latitude_degrees, &latitude_minutes,
                                           &latitude_seconds,
                                           &longitude_degrees, &longitude_minutes,
                                           &longitude_seconds, location)) {
                fprintf(stderr, "Invalid timezone description line: %s\n", buf);
                continue;
            }
        }

        icaltimezone_init(&zone);
        zone.location = strdup(location);

        if (latitude_degrees >= 0) {
            zone.latitude =
                (double)latitude_degrees +
                (double)latitude_minutes / 60 +
                (double)latitude_seconds / 3600;
        } else {
            zone.latitude =
                (double)latitude_degrees -
                (double)latitude_minutes / 60 -
                (double)latitude_seconds / 3600;
        }

        if (longitude_degrees >= 0) {
            zone.longitude =
                (double)longitude_degrees +
                (double)longitude_minutes / 60 +
                (double)longitude_seconds / 3600;
        } else {
            zone.longitude =
                (double)longitude_degrees -
                (double)longitude_minutes / 60 -
                (double)longitude_seconds / 3600;
        }

        icalarray_append(builtin_timezones, &zone);

#if 0
        printf("Found zone: %s %f %f\n", location, zone.latitude, zone.longitude);
#endif
    }

    fclose(fp);
}

void icaltimezone_release_zone_tab(void)
{
    size_t i;
    icalarray *mybuiltin_timezones = builtin_timezones;

    if (builtin_timezones == NULL)
        return;

    builtin_timezones = NULL;
    for (i = 0; i < mybuiltin_timezones->num_elements; i++) {
        free(((icaltimezone *) icalarray_element_at(mybuiltin_timezones, i))->location);
    }
    icalarray_free(mybuiltin_timezones);
}

/** Loads the builtin VTIMEZONE data for the given timezone. */
static void icaltimezone_load_builtin_timezone(icaltimezone *zone)
{
    icalcomponent *comp = 0, *subcomp;

    /* If the location isn't set, it isn't a builtin timezone. */
    if (!zone->location || !zone->location[0])
        return;

    /* Prevent blocking on mutex lock caused by recursive calls */
    if (zone->component)
        return;

#if defined(HAVE_PTHREAD)
    pthread_mutex_lock(&builtin_mutex);
#endif

    if (use_builtin_tzdata) {
        char *filename;
        size_t filename_len;
        FILE *fp;
        icalparser *parser;

        filename_len = strlen(get_zone_directory()) + strlen(zone->location) + 6;

        filename = (char *)malloc(filename_len);
        if (!filename) {
            icalerror_set_errno(ICAL_NEWFAILED_ERROR);
            goto out;
        }

        snprintf(filename, filename_len, "%s/%s.ics", get_zone_directory(), zone->location);

        fp = fopen(filename, "r");
        free(filename);
        if (!fp) {
            icalerror_set_errno(ICAL_FILE_ERROR);
            goto out;
        }

        /* ##### B.# Sun, 11 Nov 2001 04:04:29 +1100
           this is where the MALFORMEDDATA error is being set, after the call to 'icalparser_parse'
           fprintf(stderr, "** WARNING ** %s: %d %s\n",
                   __FILE__, __LINE__, icalerror_strerror(icalerrno));
         */

        parser = icalparser_new();
        icalparser_set_gen_data(parser, fp);
        comp = icalparser_parse(parser, icaltimezone_load_get_line_fn);
        icalparser_free(parser);
        fclose(fp);

        /* Find the VTIMEZONE component inside the VCALENDAR. There should be 1. */
        subcomp = icalcomponent_get_first_component(comp, ICAL_VTIMEZONE_COMPONENT);
    } else {
        subcomp = icaltzutil_fetch_timezone(zone->location);
    }

    if (!subcomp) {
        icalerror_set_errno(ICAL_PARSE_ERROR);
        goto out;
    }

    icaltimezone_get_vtimezone_properties(zone, subcomp);

    if (use_builtin_tzdata) {
        icalcomponent_remove_component(comp, subcomp);
        icalcomponent_free(comp);
    }

  out:
#if defined(HAVE_PTHREAD)
    pthread_mutex_unlock(&builtin_mutex);
#endif
    return;
}

/** Callback used from icalparser_parse() */
static char *icaltimezone_load_get_line_fn(char *s, size_t size, void *data)
{
    return fgets(s, (int)size, (FILE *) data);
}

/*
 * DEBUGGING
 */

/**
 * This outputs a list of timezone changes for the given timezone to the
 * given file, up to the maximum year given. We compare this output with the
 * output from 'vzic --dump-changes' to make sure that we are consistent.
 * (vzic is the Olson timezone database to VTIMEZONE converter.)
 *
 * The output format is:
 *
 *      Zone-Name [tab] Date [tab] Time [tab] UTC-Offset
 *
 * The Date and Time fields specify the time change in UTC.
 *
 * The UTC Offset is for local (wall-clock) time. It is the amount of time
 * to add to UTC to get local time.
 */
int icaltimezone_dump_changes(icaltimezone *zone, int max_year, FILE *fp)
{
    static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    icaltimezonechange *zone_change;
    size_t change_num;
    char buffer[8];

    /* Make sure the changes array is expanded up to the given time. */
    icaltimezone_ensure_coverage(zone, max_year);

#if 0
    printf("Num changes: %i\n", zone->changes->num_elements);
#endif

    for (change_num = 0; change_num < zone->changes->num_elements; change_num++) {
        zone_change = icalarray_element_at(zone->changes, change_num);

        if (zone_change->year > max_year)
            break;

        fprintf(fp, "%s\t%2i %s %04i\t%2i:%02i:%02i",
                zone->location,
                zone_change->day, months[zone_change->month - 1],
                zone_change->year, zone_change->hour, zone_change->minute, zone_change->second);

        /* Wall Clock Time offset from UTC. */
        format_utc_offset(zone_change->utc_offset, buffer, sizeof(buffer));
        fprintf(fp, "\t%s", buffer);

        fprintf(fp, "\n");
    }
    return 1;
}

/** This formats a UTC offset as "+HHMM" or "+HHMMSS".
   buffer should have space for 8 characters. */
static void format_utc_offset(int utc_offset, char *buffer, size_t buffer_size)
{
    const char *sign = "+";
    int hours, minutes, seconds;

    if (utc_offset < 0) {
        utc_offset = -utc_offset;
        sign = "-";
    }

    hours = utc_offset / 3600;
    minutes = (utc_offset % 3600) / 60;
    seconds = utc_offset % 60;

    /* Sanity check. Standard timezone offsets shouldn't be much more than 12
       hours, and daylight saving shouldn't change it by more than a few hours.
       (The maximum offset is 15 hours 56 minutes at present.) */
    if (hours < 0 || hours >= 24 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60) {
        fprintf(stderr, "Warning: Strange timezone offset: H:%i M:%i S:%i\n",
                hours, minutes, seconds);
    }

    if (seconds == 0) {
        snprintf(buffer, buffer_size, "%s%02i%02i", sign, hours, minutes);
    } else {
        snprintf(buffer, buffer_size, "%s%02i%02i%02i", sign, hours, minutes, seconds);
    }
}

static const char *get_zone_directory(void)
{
#if !defined(_WIN32)
    return zone_files_directory == NULL ? ZONEINFO_DIRECTORY : zone_files_directory;
#else
    wchar_t wbuffer[1000];

#if !defined(_WIN32_WCE)
    char buffer[1000], zoneinfodir[1000], dirname[1000];
#else
    wchar_t zoneinfodir[1000], dirname[1000];
#endif
    int used_default;
    static char *cache = NULL;

#if !defined(_WIN32_WCE)
    unsigned char *dirslash, *zislash, *zislashp1;
#else
    wchar_t *dirslash, *zislash;
#endif
    struct stat st;

    if (zone_files_directory)
        return zone_files_directory;

    if (cache)
        return cache;

    /* Get the filename of the application */
    if (!GetModuleFileNameW(NULL, wbuffer, sizeof(wbuffer) / sizeof(wbuffer[0])))
        return ZONEINFO_DIRECTORY;

/*wince supports only unicode*/
#if !defined(_WIN32_WCE)
    /* Convert to system codepage */
    if (!WideCharToMultiByte(CP_ACP, 0, wbuffer, -1, buffer, sizeof(buffer),
                             NULL, &used_default) || used_default) {
        /* Failed, try 8.3 format */
        if (!GetShortPathNameW(wbuffer, wbuffer,
                               sizeof(wbuffer) / sizeof(wbuffer[0])) ||
            !WideCharToMultiByte(CP_ACP, 0, wbuffer, -1, buffer, sizeof(buffer),
                                 NULL, &used_default) || used_default) {
            return ZONEINFO_DIRECTORY;
        }
    }
#endif
    /* Look for the zoneinfo directory somewhere in the path where
     * the app is installed. If the path to the app is
     *
     *      C:\opt\evo-2.6\bin\evolution-2.6.exe
     *
     * and the compile-time ZONEINFO_DIRECTORY is
     *
     *      C:/devel/target/evo/share/evolution-data-server-1.6/zoneinfo,
     *
     * we check the pathnames:
     *
     *      C:\opt\evo-2.6/devel/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt\evo-2.6/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt\evo-2.6/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt\evo-2.6/share/evolution-data-server-1.6/zoneinfo         <===
     *      C:\opt\evo-2.6/evolution-data-server-1.6/zoneinfo
     *      C:\opt\evo-2.6/zoneinfo
     *      C:\opt/devel/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt/share/evolution-data-server-1.6/zoneinfo
     *      C:\opt/evolution-data-server-1.6/zoneinfo
     *      C:\opt/zoneinfo
     *      C:/devel/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:/target/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:/evo/share/evolution-data-server-1.6/zoneinfo
     *      C:/share/evolution-data-server-1.6/zoneinfo
     *      C:/evolution-data-server-1.6/zoneinfo
     *      C:/zoneinfo
     *
     * In Evolution's case, we would get a match already at the
     * fourth pathname check.
     */

    /* Strip away basename of app .exe first */
#if !defined(_WIN32_WCE)
    dirslash = _mbsrchr((unsigned char *)buffer, '\\');
#else
    dirslash = wcsrchr(wbuffer, L'\\');
#endif
    if (dirslash) {
#if !defined(_WIN32_WCE)
        *dirslash = '\0';
#else
        *dirslash = L'\0';
#endif
    }

#if defined(_WIN32_WCE)
    while ((dirslash = wcsrchr(wbuffer, '\\'))) {
        /* Strip one more directory from app .exe location */
        *dirslash = L'\0';

        MultiByteToWideChar(CP_ACP, 0, ZONEINFO_DIRECTORY, -1, zoneinfodir, 1000);

        while ((zislash = wcschr(zoneinfodir, L'/'))) {
            *zislash = L'.';
            wcscpy(dirname, wbuffer);
            wcscat(dirname, "/");
            wcscat(dirname, zislash + 1);
            if (stat(dirname, &st) == 0 && S_ISDIR(st.st_mode)) {
                cache = wce_wctomb(dirname);
                return cache;
            }
        }
    }
#else
    while ((dirslash = _mbsrchr((unsigned char *)buffer, '\\'))) {
        /* Strip one more directory from app .exe location */
        *dirslash = '\0';

        strcpy(zoneinfodir, ZONEINFO_DIRECTORY);
        while ((zislash = _mbschr((unsigned char *)zoneinfodir, '/'))) {
            *zislash = '.';
            strcpy(dirname, buffer);
            strcat(dirname, "/");
            zislashp1 = zislash + 1;
            strcat(dirname, (char *)zislashp1);
            if (stat(dirname, &st) == 0 && S_ISDIR(st.st_mode)) {
                cache = strdup(dirname);
                return cache;
            }
        }
    }
#endif
    return ZONEINFO_DIRECTORY;
#endif
}

void set_zone_directory(char *path)
{
    if (zone_files_directory)
        free_zone_directory();

    zone_files_directory = malloc(strlen(path) + 1);

    if (zone_files_directory != NULL)
        strcpy(zone_files_directory, path);
}

void free_zone_directory(void)
{
    if (zone_files_directory != NULL) {
        free(zone_files_directory);
        zone_files_directory = NULL;
    }
}

void icaltimezone_set_tzid_prefix(const char *new_prefix)
{
    if (new_prefix) {
        ical_tzid_prefix = new_prefix;
    }
}

void icaltimezone_set_builtin_tzdata(int set)
{
    use_builtin_tzdata = set;
}

int icaltimezone_get_builtin_tzdata(void)
{
    return use_builtin_tzdata;
}
