/*======================================================================
 FILE: icalattach.h
 CREATOR: acampi 28 May 02

 (C) COPYRIGHT 2002, Andrea Campi <a.campi@inet.it>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALATTACH_H
#define ICALATTACH_H

#include "libical_ical_export.h"

typedef struct icalattach_impl icalattach;

typedef void (*icalattach_free_fn_t) (unsigned char *data, void *user_data);

LIBICAL_ICAL_EXPORT icalattach *icalattach_new_from_url(const char *url);

LIBICAL_ICAL_EXPORT icalattach *icalattach_new_from_data(const char *data,
                                                         icalattach_free_fn_t free_fn,
                                                         void *free_fn_data);

LIBICAL_ICAL_EXPORT void icalattach_ref(icalattach *attach);

LIBICAL_ICAL_EXPORT void icalattach_unref(icalattach *attach);

LIBICAL_ICAL_EXPORT int icalattach_get_is_url(icalattach *attach);

LIBICAL_ICAL_EXPORT const char *icalattach_get_url(icalattach *attach);

LIBICAL_ICAL_EXPORT unsigned char *icalattach_get_data(icalattach *attach);

LIBICAL_ICAL_EXPORT struct icalattachtype *icalattachtype_new(void);

LIBICAL_ICAL_EXPORT void icalattachtype_add_reference(struct icalattachtype *v);

LIBICAL_ICAL_EXPORT void icalattachtype_free(struct icalattachtype *v);

LIBICAL_ICAL_EXPORT void icalattachtype_set_url(struct icalattachtype *v, char *url);

LIBICAL_ICAL_EXPORT char *icalattachtype_get_url(struct icalattachtype *v);

LIBICAL_ICAL_EXPORT void icalattachtype_set_base64(struct icalattachtype *v,
                                                   char *base64, int owns);

LIBICAL_ICAL_EXPORT char *icalattachtype_get_base64(struct icalattachtype *v);

LIBICAL_ICAL_EXPORT void icalattachtype_set_binary(struct icalattachtype *v,
                                                   char *binary, int owns);

LIBICAL_ICAL_EXPORT void *icalattachtype_get_binary(struct icalattachtype *v);

#endif /* !ICALATTACH_H */
