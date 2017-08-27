/*======================================================================
 FILE: icaltimezoneimpl.h
 CREATOR: glenn 07 March 2010

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALTIMEZONEIMPL_H
#define ICALTIMEZONEIMPL_H

struct _icaltimezone
{
    char *tzid;
    /**< The unique ID of this timezone,
       e.g. "/citadel.org/Olson_20010601_1/Africa/Banjul".
       This should only be used to identify a VTIMEZONE. It is not
       meant to be displayed to the user in any form. */

    char *location;
    /**< The location for the timezone, e.g. "Africa/Accra" for the
       Olson database. We look for this in the "LOCATION" or
       "X-LIC-LOCATION" properties of the VTIMEZONE component. It
       isn't a standard property yet. This will be NULL if no location
       is found in the VTIMEZONE. */

    char *tznames;
    /**< This will be set to a combination of the TZNAME properties
       from the last STANDARD and DAYLIGHT components in the
       VTIMEZONE, e.g. "EST/EDT".  If they both use the same TZNAME,
       or only one type of component is found, then only one TZNAME
       will appear, e.g. "AZOT". If no TZNAME is found this will be
       NULL. */

    double latitude;
    double longitude;
    /**< The coordinates of the city, in degrees. */

    icalcomponent *component;
    /**< The toplevel VTIMEZONE component loaded from the .ics file for this
         timezone. If we need to regenerate the changes data we need this. */

    icaltimezone *builtin_timezone;
    /**< If this is not NULL it points to the builtin icaltimezone
       that the above TZID refers to. This icaltimezone should be used
       instead when accessing the timezone changes data, so that the
       expanded timezone changes data is shared between calendar
       components. */

    int end_year;
    /**< This is the last year for which we have expanded the data to.
       If we need to calculate a date past this we need to expand the
       timezone component data from scratch. */

    icalarray *changes;
    /**< A dynamically-allocated array of time zone changes, sorted by the
       time of the change in local time. So we can do fast binary-searches
       to convert from local time to UTC. */
};

#endif /*ICALTIMEZONE_IMPL */
