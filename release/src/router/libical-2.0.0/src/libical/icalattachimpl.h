/*======================================================================
 FILE: icalattachimpl.h
 CREATOR: acampi 28 May 02

 (C) COPYRIGHT 2000, Andrea Campi <a.campi@inet.it>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALATTACHIMPL_H
#define ICALATTACHIMPL_H

#include "icalattach.h"

/* Private structure for ATTACH values */
struct icalattach_impl
{
    /* Reference count */
    int refcount;

    union
    {
        /* URL attachment data */
        struct
        {
            char *url;
        } url;

        /* Inline data */
        struct
        {
            char *data;
            icalattach_free_fn_t free_fn;
            void *free_fn_data;
        } data;
    } u;

    /* TRUE if URL, FALSE if inline data */
    unsigned int is_url:1;
};

#endif
