/*======================================================================
 FILE: icaltimezone.h
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
/**
 * @file icaltimezone.h
 * @brief timezone handling routines
 */

#ifndef ICALTIMEZONE_H
#define ICALTIMEZONE_H

#include "libical_ical_export.h"
#include "icalcomponent.h"

#include <stdio.h>

#if !defined(ICALTIMEZONE_DEFINED)
#define ICALTIMEZONE_DEFINED
/** @brief An opaque struct representing a timezone.
 * We declare this here to avoid a circular dependancy.
 */
typedef struct _icaltimezone icaltimezone;
#endif

/**
 * @par Creating/Destroying individual icaltimezones.
 */

/** Creates a new icaltimezone. */
LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_new(void);

LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_copy(icaltimezone *originalzone);

/** Frees all memory used for the icaltimezone. Set free_struct to free the
   icaltimezone struct as well. */
LIBICAL_ICAL_EXPORT void icaltimezone_free(icaltimezone *zone, int free_struct);

/** Sets the prefix to be used for tzid's generated from system tzdata.
    Must be globally unique (such as a domain name owned by the developer
    of the calling application), and begin and end with forward slashes.
    Do not change or de-allocate the string buffer after calling this.
 */
LIBICAL_ICAL_EXPORT void icaltimezone_set_tzid_prefix(const char *new_prefix);

/**
 * @par Accessing timezones.
 */

/** Free any builtin timezone information **/
LIBICAL_ICAL_EXPORT void icaltimezone_free_builtin_timezones(void);

/** Returns the array of builtin icaltimezones. */
LIBICAL_ICAL_EXPORT icalarray *icaltimezone_get_builtin_timezones(void);

/** Returns a single builtin timezone, given its Olson city name. */
LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_get_builtin_timezone(const char *location);

/** Returns a single builtin timezone, given its offset. */
LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_get_builtin_timezone_from_offset(int offset,
                                                                                const char *tzname);

/** Returns a single builtin timezone, given its TZID. */
LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_get_builtin_timezone_from_tzid(const char *tzid);

/** Returns the UTC timezone. */
LIBICAL_ICAL_EXPORT icaltimezone *icaltimezone_get_utc_timezone(void);

/** Returns the TZID of a timezone. */
LIBICAL_ICAL_EXPORT const char *icaltimezone_get_tzid(icaltimezone *zone);

/** Returns the city name of a timezone. */
LIBICAL_ICAL_EXPORT const char *icaltimezone_get_location(icaltimezone *zone);

/** Returns the TZNAME properties used in the latest STANDARD and DAYLIGHT
   components. If they are the same it will return just one, e.g. "LMT".
   If they are different it will format them like "EST/EDT". Note that this
   may also return NULL. */
LIBICAL_ICAL_EXPORT const char *icaltimezone_get_tznames(icaltimezone *zone);

/** Returns the latitude of a builtin timezone. */
LIBICAL_ICAL_EXPORT double icaltimezone_get_latitude(icaltimezone *zone);

/** Returns the longitude of a builtin timezone. */
LIBICAL_ICAL_EXPORT double icaltimezone_get_longitude(icaltimezone *zone);

/** Returns the VTIMEZONE component of a timezone. */
LIBICAL_ICAL_EXPORT icalcomponent *icaltimezone_get_component(icaltimezone *zone);

/** Sets the VTIMEZONE component of an icaltimezone, initializing the tzid,
   location & tzname fields. It returns 1 on success or 0 on failure, i.e.
   no TZID was found. */
LIBICAL_ICAL_EXPORT int icaltimezone_set_component(icaltimezone *zone, icalcomponent *comp);

LIBICAL_ICAL_EXPORT const char *icaltimezone_get_display_name(icaltimezone *zone);

/**
 * @par Converting times between timezones.
 */

LIBICAL_ICAL_EXPORT void icaltimezone_convert_time(struct icaltimetype *tt,
                                                   icaltimezone *from_zone,
                                                   icaltimezone *to_zone);

/**
 * @par Getting offsets from UTC.
 */

/** Calculates the UTC offset of a given local time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
LIBICAL_ICAL_EXPORT int icaltimezone_get_utc_offset(icaltimezone *zone,
                                                    struct icaltimetype *tt, int *is_daylight);

/** Calculates the UTC offset of a given UTC time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
LIBICAL_ICAL_EXPORT int icaltimezone_get_utc_offset_of_utc_time(icaltimezone *zone,
                                                                struct icaltimetype *tt,
                                                                int *is_daylight);

/*
 * Handling arrays of timezones. Mainly for internal use.
 */
LIBICAL_ICAL_EXPORT icalarray *icaltimezone_array_new(void);

LIBICAL_ICAL_EXPORT void icaltimezone_array_append_from_vtimezone(icalarray *timezones,
                                                                  icalcomponent *child);

LIBICAL_ICAL_EXPORT void icaltimezone_array_free(icalarray *timezones);

/*
 * By request (issue #112) make vtimezone functions public
 */
LIBICAL_ICAL_EXPORT void icaltimezone_expand_vtimezone(icalcomponent *comp,
                                                       int end_year, icalarray *changes);

LIBICAL_ICAL_EXPORT char *icaltimezone_get_location_from_vtimezone(icalcomponent *component);

LIBICAL_ICAL_EXPORT char *icaltimezone_get_tznames_from_vtimezone(icalcomponent *component);

/*
 * @par Handling the default location the timezone files
 */

/** Set the directory to look for the zonefiles */
LIBICAL_ICAL_EXPORT void set_zone_directory(char *path);

/** Free memory dedicated to the zonefile directory */
LIBICAL_ICAL_EXPORT void free_zone_directory(void);

LIBICAL_ICAL_EXPORT void icaltimezone_release_zone_tab(void);

/*
 * @par Handling whether to use builtin timezone files
 */
LIBICAL_ICAL_EXPORT void icaltimezone_set_builtin_tzdata(int set);

LIBICAL_ICAL_EXPORT int icaltimezone_get_builtin_tzdata(void);

/*
 * @par Debugging Output.
 */

/** Dumps information about changes in the timezone up to and including
   max_year. */
LIBICAL_ICAL_EXPORT int icaltimezone_dump_changes(icaltimezone *zone, int max_year, FILE *fp);

/* For the library only -- do not make visible */
extern const char *icaltimezone_tzid_prefix(void);

#endif /* ICALTIMEZONE_H */
