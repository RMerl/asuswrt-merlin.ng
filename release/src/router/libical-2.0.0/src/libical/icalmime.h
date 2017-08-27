/*======================================================================
 FILE: icalmime.h
 CREATOR: eric 26 July 2000

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

#ifndef ICALMIME_H
#define ICALMIME_H

#include "libical_ical_export.h"
#include "icalcomponent.h"

LIBICAL_ICAL_EXPORT icalcomponent *icalmime_parse(char *(*line_gen_func) (char *s,
                                                                          size_t size,
                                                                          void *d), void *data);

/* The inverse of icalmime_parse, not implemented yet. Use sspm.h directly.  */
LIBICAL_ICAL_EXPORT char *icalmime_as_mime_string(char *component);

#endif /* !ICALMIME_H */
