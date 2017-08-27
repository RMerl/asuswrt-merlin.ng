/*======================================================================
 FILE: icalvcal.h
 CREATOR: eric 25 May 00

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

#ifndef ICALVCAL_H
#define ICALVCAL_H

#include "libical_vcal_export.h"
#include "vobject.h"
#include "icalcomponent.h"

/* These are used as default values if the values are missing in the vCalendar
   file. Gnome Calendar, for example, does not save the URL of the audio alarm,
   so we have to add a value here to make a valid iCalendar object. */
typedef struct _icalvcal_defaults icalvcal_defaults;
struct _icalvcal_defaults
{
    char *alarm_audio_url;
    char *alarm_audio_fmttype;
    char *alarm_description;
};

/* Convert a vObject into an icalcomponent */

LIBICAL_VCAL_EXPORT icalcomponent *icalvcal_convert(VObject *object);

LIBICAL_VCAL_EXPORT icalcomponent *icalvcal_convert_with_defaults(VObject *object,
                                                                  icalvcal_defaults * defaults);

#endif /* !ICALVCAL_H */
