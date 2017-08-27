/*======================================================================
 FILE: icaldirset.h
 CREATOR: eric 28 November 1999

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

#ifndef ICALDIRSET_H
#define ICALDIRSET_H

#include "libical_icalss_export.h"
#include "icalset.h"

/* icaldirset Routines for storing, fetching, and searching for ical
 * objects in a database */

typedef struct icaldirset_impl icaldirset;

LIBICAL_ICALSS_EXPORT icalset *icaldirset_new(const char *path);

LIBICAL_ICALSS_EXPORT icalset *icaldirset_new_reader(const char *path);

LIBICAL_ICALSS_EXPORT icalset *icaldirset_new_writer(const char *path);

LIBICAL_ICALSS_EXPORT icalset *icaldirset_init(icalset *set, const char *dsn, void *options);

LIBICAL_ICALSS_EXPORT void icaldirset_free(icalset *set);

LIBICAL_ICALSS_EXPORT const char *icaldirset_path(icalset *set);

/* Mark the cluster as changed, so it will be written to disk when it
   is freed. Commit writes to disk immediately*/
LIBICAL_ICALSS_EXPORT void icaldirset_mark(icalset *set);

LIBICAL_ICALSS_EXPORT icalerrorenum icaldirset_commit(icalset *set);

LIBICAL_ICALSS_EXPORT icalerrorenum icaldirset_add_component(icalset *store, icalcomponent *comp);
LIBICAL_ICALSS_EXPORT icalerrorenum icaldirset_remove_component(icalset *store,
                                                                icalcomponent *comp);

LIBICAL_ICALSS_EXPORT int icaldirset_count_components(icalset *store, icalcomponent_kind kind);

/* Restrict the component returned by icaldirset_first, _next to those
   that pass the gauge. _clear removes the gauge. */
LIBICAL_ICALSS_EXPORT icalerrorenum icaldirset_select(icalset *store, icalgauge *gauge);

LIBICAL_ICALSS_EXPORT void icaldirset_clear(icalset *store);

/* Get a component by uid */
LIBICAL_ICALSS_EXPORT icalcomponent *icaldirset_fetch(icalset *store,
                                                      icalcomponent_kind kind, const char *uid);

LIBICAL_ICALSS_EXPORT int icaldirset_has_uid(icalset *store, const char *uid);

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirset_fetch_match(icalset *set, icalcomponent *c);

/* Modify components according to the MODIFY method of CAP. Works on
   the currently selected components. */
LIBICAL_ICALSS_EXPORT icalerrorenum icaldirset_modify(icalset *store,
                                                      icalcomponent *oldc, icalcomponent *newc);

/* Iterate through the components. If a gauge has been defined, these
   will skip over components that do not pass the gauge */

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirset_get_current_component(icalset *store);

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirset_get_first_component(icalset *store);

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirset_get_next_component(icalset *store);

/* External iterator for thread safety */
LIBICAL_ICALSS_EXPORT icalsetiter icaldirset_begin_component(icalset *set,
                                                             icalcomponent_kind kind,
                                                             icalgauge *gauge, const char *tzid);

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirsetiter_to_next(icalset *set, icalsetiter *i);

LIBICAL_ICALSS_EXPORT icalcomponent *icaldirsetiter_to_prior(icalset *set, icalsetiter *i);

typedef struct icaldirset_options
{
    int flags;            /**< flags corresponding to the open() system call O_RDWR, etc. */
} icaldirset_options;

#endif /* !ICALDIRSET_H */
