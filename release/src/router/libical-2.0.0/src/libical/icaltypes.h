/*======================================================================
 FILE: icaltypes.h
 CREATOR: eric 20 March 1999

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

#ifndef ICALTYPES_H
#define ICALTYPES_H

#include "libical_ical_export.h"
#include "icalduration.h"
#include "icalenums.h"
#include "icalperiod.h"

struct icaldatetimeperiodtype
{
    struct icaltimetype time;
    struct icalperiodtype period;
};

struct icalgeotype
{
    double lat;
    double lon;
};

struct icaltriggertype
{
    struct icaltimetype time;
    struct icaldurationtype duration;
};

LIBICAL_ICAL_EXPORT struct icaltriggertype icaltriggertype_from_int(const int reltime);

LIBICAL_ICAL_EXPORT struct icaltriggertype icaltriggertype_from_string(const char *str);

LIBICAL_ICAL_EXPORT int icaltriggertype_is_null_trigger(struct icaltriggertype tr);

LIBICAL_ICAL_EXPORT int icaltriggertype_is_bad_trigger(struct icaltriggertype tr);

/* struct icalreqstattype. This struct contains two string pointers,
but don't try to free either of them. The "desc" string is a pointer
to a static table inside the library.  Don't try to free it. The
"debug" string is a pointer into the string that the called passed
into to icalreqstattype_from_string. Don't try to free it either, and
don't use it after the original string has been freed.

BTW, you would get that original string from
*icalproperty_get_requeststatus() or icalvalue_get_text(), when
operating on the value of a request_status property. */

struct icalreqstattype
{
    icalrequeststatus code;
    const char *desc;
    const char *debug;
};

LIBICAL_ICAL_EXPORT struct icalreqstattype icalreqstattype_from_string(const char *str);

LIBICAL_ICAL_EXPORT const char *icalreqstattype_as_string(struct icalreqstattype);

LIBICAL_ICAL_EXPORT char *icalreqstattype_as_string_r(struct icalreqstattype);

struct icaltimezonephase
{
    const char *tzname;
    int is_stdandard;   /* 1 = standard tme, 0 = daylight savings time */
    struct icaltimetype dtstart;
    int offsetto;
    int tzoffsetfrom;
    const char *comment;
    struct icaldatetimeperiodtype rdate;
    const char *rrule;
};

struct icaltimezonetype
{
    const char *tzid;
    struct icaltimetype last_mod;
    const char *tzurl;

    /* Array of phases. The end of the array is a phase with tzname == 0 */
    struct icaltimezonephase *phases;
};

LIBICAL_ICAL_EXPORT void icaltimezonetype_free(struct icaltimezonetype tzt);

/* ical_unknown_token_handling :
 *    How should the ICAL library handle components, properties and parameters with
 *    unknown names?
 *    FIXME:  Currently only affects parameters.  Extend to components and properties.
 */
typedef enum ical_unknown_token_handling
{
    ICAL_ASSUME_IANA_TOKEN = 1,
    ICAL_DISCARD_TOKEN = 2,
    ICAL_TREAT_AS_ERROR = 3
} ical_unknown_token_handling;

LIBICAL_ICAL_EXPORT ical_unknown_token_handling ical_get_unknown_token_handling_setting(void);

LIBICAL_ICAL_EXPORT void ical_set_unknown_token_handling_setting(
    ical_unknown_token_handling newSetting);

#endif /* !ICALTYPES_H */
