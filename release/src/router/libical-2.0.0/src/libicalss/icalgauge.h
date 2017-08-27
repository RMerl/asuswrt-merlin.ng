/*======================================================================
 FILE: icalgauge.h
 CREATOR: eric 23 December 1999

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

#ifndef ICALGAUGE_H
#define ICALGAUGE_H

#include "libical_icalss_export.h"
#include "icalcomponent.h"

/** @file icalgauge.h
 *  @brief Routines implementing a filter for ical components
 */

typedef struct icalgauge_impl icalgauge;

LIBICAL_ICALSS_EXPORT icalgauge *icalgauge_new_from_sql(char *sql, int expand);

LIBICAL_ICALSS_EXPORT int icalgauge_get_expand(icalgauge *gauge);

LIBICAL_ICALSS_EXPORT void icalgauge_free(icalgauge *gauge);

LIBICAL_ICALSS_EXPORT char *icalgauge_as_sql(icalcomponent *gauge);

LIBICAL_ICALSS_EXPORT void icalgauge_dump(icalgauge *gauge);

/** @brief Return true if comp matches the gauge.
 *
 * The component must be in
 * cannonical form -- a VCALENDAR with one VEVENT, VTODO or VJOURNAL
 * sub component
 */
LIBICAL_ICALSS_EXPORT int icalgauge_compare(icalgauge *g, icalcomponent *comp);

/** Clone the component, but only return the properties
 *  specified in the gauge */
LIBICAL_ICALSS_EXPORT icalcomponent *icalgauge_new_clone(icalgauge *g, icalcomponent *comp);

#endif /* ICALGAUGE_H */
