/*======================================================================
 FILE: icalattach.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalattachimpl.h"
#include "icalerror.h"

#include <errno.h>
#include <stdlib.h>

icalattach *icalattach_new_from_url(const char *url)
{
    icalattach *attach;
    char *url_copy;

    icalerror_check_arg_rz((url != NULL), "url");

    if ((attach = malloc(sizeof(icalattach))) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if ((url_copy = strdup(url)) == NULL) {
        free(attach);
        errno = ENOMEM;
        return NULL;
    }

    attach->refcount = 1;
    attach->is_url = 1;
    attach->u.url.url = url_copy;

    return attach;
}

icalattach *icalattach_new_from_data(const char *data, icalattach_free_fn_t free_fn,
                                     void *free_fn_data)
{
    icalattach *attach;
    char *data_copy;

    icalerror_check_arg_rz((data != NULL), "data");

    if ((attach = malloc(sizeof(icalattach))) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if ((data_copy = strdup(data)) == NULL) {
        free(attach);
        errno = ENOMEM;
        return NULL;
    }

    attach->refcount = 1;
    attach->is_url = 0;
    attach->u.data.data = data_copy;
    attach->u.data.free_fn = free_fn;
    attach->u.data.free_fn_data = free_fn_data;

    return attach;
}

void icalattach_ref(icalattach *attach)
{
    icalerror_check_arg_rv((attach != NULL), "attach");
    icalerror_check_arg_rv((attach->refcount > 0), "attach->refcount > 0");

    attach->refcount++;
}

void icalattach_unref(icalattach *attach)
{
    icalerror_check_arg_rv((attach != NULL), "attach");
    icalerror_check_arg_rv((attach->refcount > 0), "attach->refcount > 0");

    attach->refcount--;

    if (attach->refcount != 0)
        return;

    if (attach->is_url) {
        free(attach->u.url.url);
    } else {
        free(attach->u.data.data);
/* unused for now
        if (attach->u.data.free_fn)
           (* attach->u.data.free_fn) (attach->u.data.data, attach->u.data.free_fn_data);
*/
    }

    free(attach);
}

int icalattach_get_is_url(icalattach *attach)
{
    icalerror_check_arg_rz((attach != NULL), "attach");

    return attach->is_url ? 1 : 0;
}

const char *icalattach_get_url(icalattach *attach)
{
    icalerror_check_arg_rz((attach != NULL), "attach");
    icalerror_check_arg_rz((attach->is_url), "attach->is_url");

    return attach->u.url.url;
}

unsigned char *icalattach_get_data(icalattach *attach)
{
    icalerror_check_arg_rz((attach != NULL), "attach");
    icalerror_check_arg_rz((!attach->is_url), "!attach->is_url");

    return (unsigned char *)attach->u.data.data;
}
