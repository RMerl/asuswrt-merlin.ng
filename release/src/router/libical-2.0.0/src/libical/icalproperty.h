/*======================================================================
 FILE: icalproperty.h
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

#ifndef ICALPROPERTY_H
#define ICALPROPERTY_H

#include "libical_ical_export.h"
#include "icalderivedproperty.h"        /* To get icalproperty_kind enumerations */

#include <stdarg.h>     /* for va_... */

LIBICAL_ICAL_EXPORT icalproperty *icalproperty_new(icalproperty_kind kind);

LIBICAL_ICAL_EXPORT icalproperty *icalproperty_new_impl(icalproperty_kind kind);

LIBICAL_ICAL_EXPORT icalproperty *icalproperty_new_clone(icalproperty *prop);

LIBICAL_ICAL_EXPORT icalproperty *icalproperty_new_from_string(const char *str);

LIBICAL_ICAL_EXPORT const char *icalproperty_as_ical_string(icalproperty *prop);

LIBICAL_ICAL_EXPORT char *icalproperty_as_ical_string_r(icalproperty *prop);

LIBICAL_ICAL_EXPORT void icalproperty_free(icalproperty *prop);

LIBICAL_ICAL_EXPORT icalproperty_kind icalproperty_isa(icalproperty *property);

LIBICAL_ICAL_EXPORT int icalproperty_isa_property(void *property);

LIBICAL_ICAL_EXPORT void icalproperty_add_parameters(struct icalproperty_impl *prop, va_list args);

LIBICAL_ICAL_EXPORT void icalproperty_add_parameter(icalproperty *prop, icalparameter *parameter);

LIBICAL_ICAL_EXPORT void icalproperty_set_parameter(icalproperty *prop, icalparameter *parameter);

LIBICAL_ICAL_EXPORT void icalproperty_set_parameter_from_string(icalproperty *prop,
                                                                const char *name,
                                                                const char *value);
LIBICAL_ICAL_EXPORT const char *icalproperty_get_parameter_as_string(icalproperty *prop,
                                                                     const char *name);

LIBICAL_ICAL_EXPORT char *icalproperty_get_parameter_as_string_r(icalproperty *prop,
                                                                 const char *name);

LIBICAL_ICAL_EXPORT void icalproperty_remove_parameter(icalproperty *prop,
                                                       icalparameter_kind kind);

LIBICAL_ICAL_EXPORT void icalproperty_remove_parameter_by_kind(icalproperty *prop,
                                                               icalparameter_kind kind);

LIBICAL_ICAL_EXPORT void icalproperty_remove_parameter_by_name(icalproperty *prop,
                                                               const char *name);

LIBICAL_ICAL_EXPORT void icalproperty_remove_parameter_by_ref(icalproperty *prop,
                                                              icalparameter *param);

LIBICAL_ICAL_EXPORT int icalproperty_count_parameters(const icalproperty *prop);

/* Iterate through the parameters */
LIBICAL_ICAL_EXPORT icalparameter *icalproperty_get_first_parameter(icalproperty *prop,
                                                                    icalparameter_kind kind);
LIBICAL_ICAL_EXPORT icalparameter *icalproperty_get_next_parameter(icalproperty *prop,
                                                                   icalparameter_kind kind);
/* Access the value of the property */
LIBICAL_ICAL_EXPORT void icalproperty_set_value(icalproperty *prop, icalvalue *value);
LIBICAL_ICAL_EXPORT void icalproperty_set_value_from_string(icalproperty *prop, const char *value,
                                                            const char *kind);

LIBICAL_ICAL_EXPORT icalvalue *icalproperty_get_value(const icalproperty *prop);
LIBICAL_ICAL_EXPORT const char *icalproperty_get_value_as_string(const icalproperty *prop);
LIBICAL_ICAL_EXPORT char *icalproperty_get_value_as_string_r(const icalproperty *prop);

LIBICAL_ICAL_EXPORT void icalvalue_set_parent(icalvalue *value, icalproperty *property);

/* Deal with X properties */

LIBICAL_ICAL_EXPORT void icalproperty_set_x_name(icalproperty *prop, const char *name);
LIBICAL_ICAL_EXPORT const char *icalproperty_get_x_name(icalproperty *prop);

/** Return the name of the property -- the type name converted to a
 *  string, or the value of _get_x_name if the type is and X
 *  property
 */
LIBICAL_ICAL_EXPORT const char *icalproperty_get_property_name(const icalproperty *prop);
LIBICAL_ICAL_EXPORT char *icalproperty_get_property_name_r(const icalproperty *prop);

LIBICAL_ICAL_EXPORT icalvalue_kind icalparameter_value_to_value_kind(icalparameter_value value);

/* Convert kinds to string and get default value type */
LIBICAL_ICAL_EXPORT icalvalue_kind icalproperty_kind_to_value_kind(icalproperty_kind kind);
LIBICAL_ICAL_EXPORT icalproperty_kind icalproperty_value_kind_to_kind(icalvalue_kind kind);
LIBICAL_ICAL_EXPORT const char *icalproperty_kind_to_string(icalproperty_kind kind);
LIBICAL_ICAL_EXPORT icalproperty_kind icalproperty_string_to_kind(const char *string);

/** Check validity of a specific icalproperty_kind **/
LIBICAL_ICAL_EXPORT int icalproperty_kind_is_valid(const icalproperty_kind kind);

LIBICAL_ICAL_EXPORT icalproperty_method icalproperty_string_to_method(const char *str);
LIBICAL_ICAL_EXPORT const char *icalproperty_method_to_string(icalproperty_method method);

LIBICAL_ICAL_EXPORT const char *icalproperty_enum_to_string(int e);
LIBICAL_ICAL_EXPORT char *icalproperty_enum_to_string_r(int e);
LIBICAL_ICAL_EXPORT int icalproperty_string_to_enum(const char *str);
LIBICAL_ICAL_EXPORT int icalproperty_kind_and_string_to_enum(const int kind, const char *str);

LIBICAL_ICAL_EXPORT const char *icalproperty_status_to_string(icalproperty_status);
LIBICAL_ICAL_EXPORT icalproperty_status icalproperty_string_to_status(const char *string);

LIBICAL_ICAL_EXPORT int icalproperty_enum_belongs_to_property(icalproperty_kind kind, int e);

#endif /*ICALPROPERTY_H */
