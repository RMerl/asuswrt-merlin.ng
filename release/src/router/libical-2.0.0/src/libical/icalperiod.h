/*======================================================================
 FILE: icalperiod.h
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

#ifndef ICALPERIOD_H
#define ICALPERIOD_H

#include "libical_ical_export.h"
#include "icalduration.h"
#include "icaltime.h"

struct icalperiodtype
{
    struct icaltimetype start;
    struct icaltimetype end;
    struct icaldurationtype duration;
};

LIBICAL_ICAL_EXPORT struct icalperiodtype icalperiodtype_from_string(const char *str);

LIBICAL_ICAL_EXPORT const char *icalperiodtype_as_ical_string(struct icalperiodtype p);

LIBICAL_ICAL_EXPORT char *icalperiodtype_as_ical_string_r(struct icalperiodtype p);

LIBICAL_ICAL_EXPORT struct icalperiodtype icalperiodtype_null_period(void);

LIBICAL_ICAL_EXPORT int icalperiodtype_is_null_period(struct icalperiodtype p);

LIBICAL_ICAL_EXPORT int icalperiodtype_is_valid_period(struct icalperiodtype p);

#endif /* !ICALTIME_H */
