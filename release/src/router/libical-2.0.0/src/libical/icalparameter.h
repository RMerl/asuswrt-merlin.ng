/*======================================================================
  FILE: icalparam.h
  CREATOR: eric 20 March 1999

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

#ifndef ICALPARAMETER_H
#define ICALPARAMETER_H

#include "libical_ical_export.h"
#include "icalderivedparameter.h"

/* Declared in icalderivedparameter.h */
/*typedef struct icalparameter_impl icalparameter;*/

LIBICAL_ICAL_EXPORT icalparameter *icalparameter_new(icalparameter_kind kind);

LIBICAL_ICAL_EXPORT icalparameter *icalparameter_new_clone(icalparameter *p);

/* Create from string of form "PARAMNAME=VALUE" */
LIBICAL_ICAL_EXPORT icalparameter *icalparameter_new_from_string(const char *value);

/* Create from just the value, the part after the "=" */
LIBICAL_ICAL_EXPORT icalparameter *icalparameter_new_from_value_string(icalparameter_kind kind,
                                                                       const char *value);

LIBICAL_ICAL_EXPORT void icalparameter_free(icalparameter *parameter);

LIBICAL_ICAL_EXPORT char *icalparameter_as_ical_string(icalparameter *parameter);

LIBICAL_ICAL_EXPORT char *icalparameter_as_ical_string_r(icalparameter *parameter);

LIBICAL_ICAL_EXPORT int icalparameter_is_valid(icalparameter *parameter);

LIBICAL_ICAL_EXPORT icalparameter_kind icalparameter_isa(icalparameter *parameter);

LIBICAL_ICAL_EXPORT int icalparameter_isa_parameter(void *param);

/* Access the name of an X parameter */
LIBICAL_ICAL_EXPORT void icalparameter_set_xname(icalparameter *param, const char *v);

LIBICAL_ICAL_EXPORT const char *icalparameter_get_xname(icalparameter *param);

LIBICAL_ICAL_EXPORT void icalparameter_set_xvalue(icalparameter *param, const char *v);

LIBICAL_ICAL_EXPORT const char *icalparameter_get_xvalue(icalparameter *param);

/* Access the name of an IANA parameter */
LIBICAL_ICAL_EXPORT void icalparameter_set_iana_name(icalparameter *param, const char *v);

LIBICAL_ICAL_EXPORT const char *icalparameter_get_iana_name(icalparameter *param);

LIBICAL_ICAL_EXPORT void icalparameter_set_iana_value(icalparameter *param, const char *v);

LIBICAL_ICAL_EXPORT const char *icalparameter_get_iana_value(icalparameter *param);

/* returns 1 if parameters have same name in ICAL, otherwise 0 */
LIBICAL_ICAL_EXPORT int icalparameter_has_same_name(icalparameter *param1, icalparameter *param2);

/* Convert enumerations */

LIBICAL_ICAL_EXPORT const char *icalparameter_kind_to_string(icalparameter_kind kind);

LIBICAL_ICAL_EXPORT icalparameter_kind icalparameter_string_to_kind(const char *string);

#endif
