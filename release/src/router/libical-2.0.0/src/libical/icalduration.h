/*======================================================================
 FILE: icalduration.h
 CREATOR: eric 26 Jan 2001

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom
======================================================================*/

#ifndef ICALDURATION_H
#define ICALDURATION_H

#include "libical_ical_export.h"
#include "icaltime.h"

struct icaldurationtype
{
    int is_neg;
    unsigned int days;
    unsigned int weeks;
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
};

LIBICAL_ICAL_EXPORT struct icaldurationtype icaldurationtype_from_int(int t);
LIBICAL_ICAL_EXPORT struct icaldurationtype icaldurationtype_from_string(const char *);
LIBICAL_ICAL_EXPORT int icaldurationtype_as_int(struct icaldurationtype duration);
LIBICAL_ICAL_EXPORT char *icaldurationtype_as_ical_string(struct icaldurationtype d);
LIBICAL_ICAL_EXPORT char *icaldurationtype_as_ical_string_r(struct icaldurationtype d);
LIBICAL_ICAL_EXPORT struct icaldurationtype icaldurationtype_null_duration(void);
LIBICAL_ICAL_EXPORT struct icaldurationtype icaldurationtype_bad_duration(void);
LIBICAL_ICAL_EXPORT int icaldurationtype_is_null_duration(struct icaldurationtype d);
LIBICAL_ICAL_EXPORT int icaldurationtype_is_bad_duration(struct icaldurationtype d);

LIBICAL_ICAL_EXPORT struct icaltimetype icaltime_add(struct icaltimetype t,
                                                     struct icaldurationtype d);

LIBICAL_ICAL_EXPORT struct icaldurationtype icaltime_subtract(struct icaltimetype t1,
                                                              struct icaltimetype t2);

#endif /* !ICALDURATION_H */
