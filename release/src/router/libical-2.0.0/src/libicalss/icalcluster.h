/*======================================================================
 FILE: icalcluster.h
 CREATOR: acampi 13 March 2002

 Copyright (C) 2002 Andrea Campi <a.campi@inet.it>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALCLUSTER_H
#define ICALCLUSTER_H

#include "libical_icalss_export.h"
#include "icalcomponent.h"
#include "icalerror.h"

typedef struct icalcluster_impl icalcluster;

LIBICAL_ICALSS_EXPORT icalcluster *icalcluster_new(const char *key, icalcomponent *data);

LIBICAL_ICALSS_EXPORT icalcluster *icalcluster_new_clone(const icalcluster *cluster);

LIBICAL_ICALSS_EXPORT void icalcluster_free(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT const char *icalcluster_key(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT int icalcluster_is_changed(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT void icalcluster_mark(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT void icalcluster_commit(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT icalcomponent *icalcluster_get_component(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT int icalcluster_count_components(icalcluster *cluster,
                                                       icalcomponent_kind kind);

LIBICAL_ICALSS_EXPORT icalerrorenum icalcluster_add_component(icalcluster *cluster,
                                                              icalcomponent *child);

LIBICAL_ICALSS_EXPORT icalerrorenum icalcluster_remove_component(icalcluster *cluster,
                                                                 icalcomponent *child);

LIBICAL_ICALSS_EXPORT icalcomponent *icalcluster_get_current_component(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT icalcomponent *icalcluster_get_first_component(icalcluster *cluster);

LIBICAL_ICALSS_EXPORT icalcomponent *icalcluster_get_next_component(icalcluster *cluster);

#endif /* !ICALCLUSTER_H */
