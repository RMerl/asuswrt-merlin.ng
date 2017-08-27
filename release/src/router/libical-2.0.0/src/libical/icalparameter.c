/*======================================================================
 FILE: icalderivedparameters.{c,h}
 CREATOR: eric 09 May 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalderivedparameters.{c,h}

 Contributions from:
    Graham Davison <g.m.davison@computer.org>
======================================================================*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalparameter.h"
#include "icalparameterimpl.h"
#include "icalerror.h"
#include "icalmemory.h"

#include <errno.h>
#include <stdlib.h>

/* In icalderivedparameter */
icalparameter *icalparameter_new_from_value_string(icalparameter_kind kind, const char *val);

LIBICAL_ICAL_EXPORT struct icalparameter_impl *icalparameter_new_impl(icalparameter_kind kind)
{
    struct icalparameter_impl *v;

    if ((v = (struct icalparameter_impl *)malloc(sizeof(struct icalparameter_impl))) == 0) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        return 0;
    }

    memset(v, 0, sizeof(struct icalparameter_impl));

    strcpy(v->id, "para");

    v->kind = kind;

    return v;
}

icalparameter *icalparameter_new(icalparameter_kind kind)
{
    struct icalparameter_impl *v = icalparameter_new_impl(kind);

    return (icalparameter *) v;
}

void icalparameter_free(icalparameter *param)
{
/*  Comment out the following as it always triggers, even when parameter is non-zero
    icalerror_check_arg_rv((parameter==0),"parameter");*/

    if (param->parent != 0) {
        return;
    }

    if (param->string != 0) {
        free((void *)param->string);
    }

    if (param->x_name != 0) {
        free((void *)param->x_name);
    }

    memset(param, 0, sizeof(icalparameter));

    param->parent = 0;
    param->id[0] = 'X';
    free(param);
}

icalparameter *icalparameter_new_clone(icalparameter *old)
{
    struct icalparameter_impl *new;

    icalerror_check_arg_rz((old != 0), "param");

    new = icalparameter_new_impl(old->kind);

    if (new == 0) {
        return 0;
    }

    memcpy(new, old, sizeof(struct icalparameter_impl));

    if (old->string != 0) {
        new->string = icalmemory_strdup(old->string);
        if (new->string == 0) {
            icalparameter_free(new);
            return 0;
        }
    }

    if (old->x_name != 0) {
        new->x_name = icalmemory_strdup(old->x_name);
        if (new->x_name == 0) {
            icalparameter_free(new);
            return 0;
        }
    }

    return new;
}

icalparameter *icalparameter_new_from_string(const char *str)
{
    char *eq;
    char *cpy;
    icalparameter_kind kind;
    icalparameter *param;

    icalerror_check_arg_rz(str != 0, "str");

    cpy = icalmemory_strdup(str);

    if (cpy == 0) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        return 0;
    }

    eq = strchr(cpy, '=');

    if (eq == 0) {
        icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        free(cpy);
        return 0;
    }

    *eq = '\0';

    eq++;

    kind = icalparameter_string_to_kind(cpy);

    if (kind == ICAL_NO_PARAMETER) {
        icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        free(cpy);
        return 0;
    }

    param = icalparameter_new_from_value_string(kind, eq);

    if (kind == ICAL_X_PARAMETER) {
        icalparameter_set_xname(param, cpy);
    } else if (kind == ICAL_IANA_PARAMETER) {
        icalparameter_set_iana_name(param, cpy);
    }

    free(cpy);

    return param;
}

char *icalparameter_as_ical_string(icalparameter *param)
{
    char *buf;

    buf = icalparameter_as_ical_string_r(param);
    icalmemory_add_tmp_buffer(buf);
    return buf;
}

/**
 * Return a string representation of the parameter according to RFC5445.
 *
 * param        = param-name "=" param-value
 * param-name   = iana-token / x-token
 * param-value  = paramtext /quoted-string
 * paramtext    = *SAFE-SHARE
 * quoted-string= DQUOTE *QSAFE-CHARE DQUOTE
 * QSAFE-CHAR   = any character except CTLs and DQUOTE
 * SAFE-CHAR    = any character except CTLs, DQUOTE. ";", ":", ","
 */
char *icalparameter_as_ical_string_r(icalparameter *param)
{
    size_t buf_size = 1024;
    char *buf;
    char *buf_ptr;
    const char *kind_string;

    icalerror_check_arg_rz((param != 0), "parameter");

    /* Create new buffer that we can append names, parameters and a
     * value to, and reallocate as needed.
     */

    buf = icalmemory_new_buffer(buf_size);
    buf_ptr = buf;

    if (param->kind == ICAL_X_PARAMETER) {
        icalmemory_append_string(&buf, &buf_ptr, &buf_size, icalparameter_get_xname(param));
    } else if (param->kind == ICAL_IANA_PARAMETER) {
        icalmemory_append_string(&buf, &buf_ptr, &buf_size, icalparameter_get_iana_name(param));
    } else {

        kind_string = icalparameter_kind_to_string(param->kind);

        if (param->kind == ICAL_NO_PARAMETER ||
            param->kind == ICAL_ANY_PARAMETER || kind_string == 0) {
            icalerror_set_errno(ICAL_BADARG_ERROR);
            free(buf);
            return 0;
        }

        /* Put the parameter name into the string */
        icalmemory_append_string(&buf, &buf_ptr, &buf_size, kind_string);
    }

    icalmemory_append_string(&buf, &buf_ptr, &buf_size, "=");

    if (param->string != 0) {
        int qm = 0;

        /* Encapsulate the property in quotes if necessary */
        if (strpbrk(param->string, ";:,") != 0) {
            icalmemory_append_char(&buf, &buf_ptr, &buf_size, '"');
            qm = 1;
        }
        icalmemory_append_string(&buf, &buf_ptr, &buf_size, param->string);
        if (qm == 1) {
            icalmemory_append_char(&buf, &buf_ptr, &buf_size, '"');
        }
    } else if (param->data != 0) {
        const char *str = icalparameter_enum_to_string(param->data);

        icalmemory_append_string(&buf, &buf_ptr, &buf_size, str);
    } else {
        icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        free(buf);
        return 0;
    }

    return buf;
}

int icalparameter_is_valid(icalparameter *parameter);

icalparameter_kind icalparameter_isa(icalparameter *parameter)
{
    if (parameter == 0) {
        return ICAL_NO_PARAMETER;
    }

    return parameter->kind;
}

int icalparameter_isa_parameter(void *parameter)
{
    struct icalparameter_impl *impl = (struct icalparameter_impl *)parameter;

    if (parameter == 0) {
        return 0;
    }

    if (strcmp(impl->id, "para") == 0) {
        return 1;
    } else {
        return 0;
    }
}

void icalparameter_set_xname(icalparameter *param, const char *v)
{
    icalerror_check_arg_rv((param != 0), "param");
    icalerror_check_arg_rv((v != 0), "v");

    if (param->x_name != 0) {
        free((void *)param->x_name);
    }

    param->x_name = icalmemory_strdup(v);

    if (param->x_name == 0) {
        errno = ENOMEM;
    }
}

const char *icalparameter_get_xname(icalparameter *param)
{
    icalerror_check_arg_rz((param != 0), "param");

    return param->x_name;
}

void icalparameter_set_xvalue(icalparameter *param, const char *v)
{
    icalerror_check_arg_rv((param != 0), "param");
    icalerror_check_arg_rv((v != 0), "v");

    if (param->string != 0) {
        free((void *)param->string);
    }

    param->string = icalmemory_strdup(v);

    if (param->string == 0) {
        errno = ENOMEM;
    }
}

const char *icalparameter_get_xvalue(icalparameter *param)
{
    icalerror_check_arg_rz((param != 0), "param");

    return param->string;
}

void icalparameter_set_iana_value(icalparameter *param, const char *v)
{
    icalparameter_set_xvalue(param, v);
}

const char *icalparameter_get_iana_value(icalparameter *param)
{
    return icalparameter_get_xvalue(param);
}

void icalparameter_set_iana_name(icalparameter *param, const char *v)
{
    icalparameter_set_xname(param, v);
}

const char *icalparameter_get_iana_name(icalparameter *param)
{
    return icalparameter_get_xname(param);
}

void icalparameter_set_parent(icalparameter *param, icalproperty *property)
{
    icalerror_check_arg_rv((param != 0), "param");

    param->parent = property;
}

icalproperty *icalparameter_get_parent(icalparameter *param)
{
    icalerror_check_arg_rz((param != 0), "param");

    return param->parent;
}

/* returns 1 if parameters have same name in ICAL, otherwise 0 */
int icalparameter_has_same_name(icalparameter *param1, icalparameter *param2)
{
    icalparameter_kind kind1;
    icalparameter_kind kind2;
    const char *name1;
    const char *name2;

    icalerror_check_arg_rz((param1 != 0), "param1");
    icalerror_check_arg_rz((param2 != 0), "param2");

    kind1 = icalparameter_isa(param1);
    kind2 = icalparameter_isa(param2);

    if (kind1 != kind2)
        return 0;

    if (kind1 == ICAL_X_PARAMETER) {
        name1 = icalparameter_get_xname(param1);
        name2 = icalparameter_get_xname(param2);
        if (strcasecmp(name1, name2) != 0) {
            return 0;
        }
    } else if (kind1 == ICAL_IANA_PARAMETER) {
        name1 = icalparameter_get_iana_name(param1);
        name2 = icalparameter_get_iana_name(param2);
        if (strcasecmp(name1, name2) != 0) {
            return 0;
        }
    }
    return 1;
}

/* Everything below this line is machine generated. Do not edit. */
/* ALTREP */
