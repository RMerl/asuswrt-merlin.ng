/*======================================================================
 FILE: icallangbind.c
 CREATOR: eric 15 dec 2000

 (C) COPYRIGHT 1999 Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

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

#include "icallangbind.h"
#include "icalerror.h"
#include "icalmemory.h"
#include "icalvalue.h"

#include <stdlib.h>

int *icallangbind_new_array(int size)
{
    int *p = (int *)malloc(size * sizeof(int));

    return p;   /* Caller handles failures */
}

void icallangbind_free_array(int *array)
{
    free(array);
}

int icallangbind_access_array(int *array, int index)
{
    return array[index];
}

/** Iterators to fetch parameters given property */

icalparameter *icallangbind_get_first_parameter(icalproperty *prop)
{
    icalparameter_kind kind = ICAL_ANY_PARAMETER;

    return icalproperty_get_first_parameter(prop, kind);
}

icalparameter *icallangbind_get_next_parameter(icalproperty *prop)
{
    icalparameter_kind kind = ICAL_ANY_PARAMETER;

    return icalproperty_get_next_parameter(prop, kind);
}

/** Like icalcomponent_get_first_component(), but takes a string for the
   kind and can iterate over X properties as if each X name was a
   separate kind */

icalproperty *icallangbind_get_first_property(icalcomponent *c, const char *prop)
{
    icalproperty_kind kind = icalproperty_string_to_kind(prop);
    icalproperty *p;

    if (kind == ICAL_NO_PROPERTY) {
        return 0;
    }

    if (kind == ICAL_X_PROPERTY) {
        for (p = icalcomponent_get_first_property(c, kind);
             p != 0; p = icalcomponent_get_next_property(c, kind)) {

            if (strcmp(icalproperty_get_x_name(p), prop) == 0) {
                return p;
            }
        }
    } else {
        p = icalcomponent_get_first_property(c, kind);

        return p;
    }

    return 0;
}

icalproperty *icallangbind_get_next_property(icalcomponent *c, const char *prop)
{
    icalproperty_kind kind = icalenum_string_to_property_kind(prop);
    icalproperty *p;

    if (kind == ICAL_NO_PROPERTY) {
        return 0;
    }

    if (kind == ICAL_X_PROPERTY) {
        for (p = icalcomponent_get_next_property(c, kind);
             p != 0; p = icalcomponent_get_next_property(c, kind)) {

            if (strcmp(icalproperty_get_x_name(p), prop) == 0) {
                return p;
            }
        }
    } else {
        p = icalcomponent_get_next_property(c, kind);

        return p;
    }

    return 0;
}

icalcomponent *icallangbind_get_first_component(icalcomponent *c, const char *comp)
{
    icalcomponent_kind kind = icalenum_string_to_component_kind(comp);

    if (kind == ICAL_NO_COMPONENT) {
        return 0;
    }
    return icalcomponent_get_first_component(c, kind);
}

icalcomponent *icallangbind_get_next_component(icalcomponent *c, const char *comp)
{
    icalcomponent_kind kind = icalenum_string_to_component_kind(comp);

    if (kind == ICAL_NO_COMPONENT) {
        return 0;
    }
    return icalcomponent_get_next_component(c, kind);
}

#define APPENDS(x) icalmemory_append_string(&buf, &buf_ptr, &buf_size, x);

#define APPENDC(x) icalmemory_append_char(&buf, &buf_ptr, &buf_size, x);

char *icallangbind_property_eval_string_r(icalproperty *prop, char *sep)
{
    char tmp[25];
    size_t buf_size = 1024;
    char *buf;
    char *buf_ptr;
    icalparameter *param;

    icalvalue *value;

    if (prop == 0) {
        return 0;
    }

    buf = icalmemory_new_buffer(buf_size);
    buf_ptr = buf;

    APPENDS("{ ");

    value = icalproperty_get_value(prop);

    APPENDS(" 'name' ");
    APPENDS(sep);
    APPENDC('\'');
    APPENDS(icalproperty_kind_to_string(icalproperty_isa(prop)));
    APPENDC('\'');

    if (value) {
        APPENDS(", 'value_type' ");
        APPENDS(sep);
        APPENDC('\'');
        APPENDS(icalvalue_kind_to_string(icalvalue_isa(value)));
        APPENDC('\'');
    }

    APPENDS(", 'pid' ");
    APPENDS(sep);
    APPENDC('\'');
    snprintf(tmp, 25, "%p", prop);
    APPENDS(tmp);
    APPENDC('\'');

    if (value) {
        switch (icalvalue_isa(value)) {

        case ICAL_ATTACH_VALUE:
        case ICAL_BINARY_VALUE:
        case ICAL_NO_VALUE:{
                icalerror_set_errno(ICAL_INTERNAL_ERROR);
                break;
            }

        default:
            {
                char *str = icalvalue_as_ical_string_r(value);
                char *copy = (char *)malloc(strlen(str) + 1);

                const char *i;
                char *j;

                if (copy == 0) {
                    icalerror_set_errno(ICAL_NEWFAILED_ERROR);
                    break;
                }
                /* Remove any newlines */

                for (j = copy, i = str; *i != 0; j++, i++) {
                    if (*i == '\n') {
                        i++;
                    }
                    *j = *i;
                }

                *j = 0;

                APPENDS(", 'value'");
                APPENDS(sep);
                APPENDC('\'');
                APPENDS(copy);
                APPENDC('\'');

                free(copy);
                free(str);
                break;
            }
        }
    }

    /* Add Parameters */

    for (param = icalproperty_get_first_parameter(prop, ICAL_ANY_PARAMETER);
         param != 0; param = icalproperty_get_next_parameter(prop, ICAL_ANY_PARAMETER)) {

        char *copy = icalparameter_as_ical_string_r(param);
        char *v;

        if (copy == 0) {
            icalerror_set_errno(ICAL_NEWFAILED_ERROR);
            continue;
        }

        v = strchr(copy, '=');

        if (v == 0) {
            free(copy);
            continue;
        }

        *v = 0;

        v++;

        APPENDS(", ");
        APPENDC('\'');
        APPENDS(copy);
        APPENDC('\'');
        APPENDS(sep);
        APPENDC('\'');
        APPENDS(v);
        APPENDC('\'');
        free(copy);
    }

    APPENDC('}');

    return buf;
}

const char *icallangbind_property_eval_string(icalproperty *prop, char *sep)
{
    char *buf;

    buf = icallangbind_property_eval_string_r(prop, sep);
    icalmemory_add_tmp_buffer(buf);
    return buf;
}

int icallangbind_string_to_open_flag(const char *str)
{
    if (strcmp(str, "r") == 0) {
        return O_RDONLY;
    } else if (strcmp(str, "r+") == 0) {
        return O_RDWR;
    } else if (strcmp(str, "w") == 0) {
        return O_WRONLY;
    } else if (strcmp(str, "w+") == 0) {
        return O_RDWR | O_CREAT;
    } else if (strcmp(str, "a") == 0) {
        return O_WRONLY | O_APPEND;
    } else {
        return -1;
    }
}

char *icallangbind_quote_as_ical_r(const char *str)
{
    size_t buf_size = 2 * strlen(str);

    /* assume every char could be quoted */
    char *buf = icalmemory_new_buffer(buf_size);

    (void)icalvalue_encode_ical_string(str, buf, (int)buf_size);

    return buf;
}

const char *icallangbind_quote_as_ical(const char *str)
{
    char *buf;

    buf = icallangbind_quote_as_ical_r(str);
    icalmemory_add_tmp_buffer(buf);
    return (buf);
}
