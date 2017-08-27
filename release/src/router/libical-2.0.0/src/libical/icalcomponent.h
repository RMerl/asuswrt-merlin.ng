/*======================================================================
 FILE: icalcomponent.h
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

#ifndef ICALCOMPONENT_H
#define ICALCOMPONENT_H

#include "libical_ical_export.h"
#include "icalenums.h"  /* defines icalcomponent_kind */
#include "icalproperty.h"
#include "pvl.h"

typedef struct icalcomponent_impl icalcomponent;

/* This is exposed so that callers will not have to allocate and
   deallocate iterators. Pretend that you can't see it. */
typedef struct icalcompiter
{
    icalcomponent_kind kind;
    pvl_elem iter;

} icalcompiter;

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new(icalcomponent_kind kind);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_clone(icalcomponent *component);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_from_string(const char *str);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_vanew(icalcomponent_kind kind, ...);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_x(const char *x_name);

LIBICAL_ICAL_EXPORT void icalcomponent_free(icalcomponent *component);

LIBICAL_ICAL_EXPORT char *icalcomponent_as_ical_string(icalcomponent *component);

LIBICAL_ICAL_EXPORT char *icalcomponent_as_ical_string_r(icalcomponent *component);

LIBICAL_ICAL_EXPORT int icalcomponent_is_valid(icalcomponent *component);

LIBICAL_ICAL_EXPORT icalcomponent_kind icalcomponent_isa(const icalcomponent *component);

LIBICAL_ICAL_EXPORT int icalcomponent_isa_component(void *component);

/*
 * Working with properties
 */

LIBICAL_ICAL_EXPORT void icalcomponent_add_property(icalcomponent *component,
                                                    icalproperty *property);

LIBICAL_ICAL_EXPORT void icalcomponent_remove_property(icalcomponent *component,
                                                       icalproperty *property);

LIBICAL_ICAL_EXPORT int icalcomponent_count_properties(icalcomponent *component,
                                                       icalproperty_kind kind);

LIBICAL_ICAL_EXPORT icalcomponent *icalproperty_get_parent(const icalproperty *property);

/* Iterate through the properties */
LIBICAL_ICAL_EXPORT icalproperty *icalcomponent_get_current_property(icalcomponent *component);

LIBICAL_ICAL_EXPORT icalproperty *icalcomponent_get_first_property(icalcomponent *component,
                                                                   icalproperty_kind kind);
LIBICAL_ICAL_EXPORT icalproperty *icalcomponent_get_next_property(icalcomponent *component,
                                                                  icalproperty_kind kind);

/*
 * Working with components
 */

/* Return the first VEVENT, VTODO or VJOURNAL sub-component of cop, or
   comp if it is one of those types */

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_inner(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_add_component(icalcomponent *parent, icalcomponent *child);

LIBICAL_ICAL_EXPORT void icalcomponent_remove_component(icalcomponent *parent,
                                                        icalcomponent *child);

LIBICAL_ICAL_EXPORT int icalcomponent_count_components(icalcomponent *component,
                                                       icalcomponent_kind kind);

/**
   This takes 2 VCALENDAR components and merges the second one into the first,
   resolving any problems with conflicting TZIDs. comp_to_merge will no
   longer exist after calling this function. */
LIBICAL_ICAL_EXPORT void icalcomponent_merge_component(icalcomponent *comp,
                                                       icalcomponent *comp_to_merge);

/* Iteration Routines. There are two forms of iterators, internal and
external. The internal ones came first, and are almost completely
sufficient, but they fail badly when you want to construct a loop that
removes components from the container.*/

/* Iterate through components */
LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_current_component(icalcomponent *component);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_first_component(icalcomponent *component,
                                                                     icalcomponent_kind kind);
LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_next_component(icalcomponent *component,
                                                                    icalcomponent_kind kind);

/* Using external iterators */
LIBICAL_ICAL_EXPORT icalcompiter icalcomponent_begin_component(icalcomponent *component,
                                                               icalcomponent_kind kind);

LIBICAL_ICAL_EXPORT icalcompiter icalcomponent_end_component(icalcomponent *component,
                                                             icalcomponent_kind kind);

LIBICAL_ICAL_EXPORT icalcomponent *icalcompiter_next(icalcompiter * i);

LIBICAL_ICAL_EXPORT icalcomponent *icalcompiter_prior(icalcompiter * i);

LIBICAL_ICAL_EXPORT icalcomponent *icalcompiter_deref(icalcompiter * i);

/* Working with embedded error properties */

/* Check the component against itip rules and insert error properties*/
/* Working with embedded error properties */
LIBICAL_ICAL_EXPORT int icalcomponent_check_restrictions(icalcomponent *comp);

/** Count embedded errors. */
LIBICAL_ICAL_EXPORT int icalcomponent_count_errors(icalcomponent *component);

/** Remove all X-LIC-ERROR properties*/
LIBICAL_ICAL_EXPORT void icalcomponent_strip_errors(icalcomponent *component);

/** Convert some X-LIC-ERROR properties into RETURN-STATUS properties*/
LIBICAL_ICAL_EXPORT void icalcomponent_convert_errors(icalcomponent *component);

/* Internal operations. They are private, and you should not be using them. */
LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_parent(icalcomponent *component);

LIBICAL_ICAL_EXPORT void icalcomponent_set_parent(icalcomponent *component,
                                                  icalcomponent *parent);

/* Kind conversion routines */

LIBICAL_ICAL_EXPORT int icalcomponent_kind_is_valid(const icalcomponent_kind kind);

LIBICAL_ICAL_EXPORT icalcomponent_kind icalcomponent_string_to_kind(const char *string);

LIBICAL_ICAL_EXPORT const char *icalcomponent_kind_to_string(icalcomponent_kind kind);

/************* Derived class methods.  ****************************

If the code was in an OO language, the remaining routines would be
members of classes derived from icalcomponent. Don't call them on the
wrong component subtypes. */

/** For VCOMPONENT: Return a reference to the first VEVENT, VTODO or
   VJOURNAL */
LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_get_first_real_component(icalcomponent *c);

/** For VEVENT, VTODO, VJOURNAL and VTIMEZONE: report the start and end
   times of an event in UTC */
LIBICAL_ICAL_EXPORT struct icaltime_span icalcomponent_get_span(icalcomponent *comp);

/******************** Convenience routines **********************/

LIBICAL_ICAL_EXPORT void icalcomponent_set_dtstart(icalcomponent *comp, struct icaltimetype v);
LIBICAL_ICAL_EXPORT struct icaltimetype icalcomponent_get_dtstart(icalcomponent *comp);

/* For the icalcomponent routines only, dtend and duration are tied
   together. If you call the set routine for one and the other exists,
   the routine will calculate the change to the other. That is, if
   there is a DTEND and you call set_duration, the routine will modify
   DTEND to be the sum of DTSTART and the duration. If you call a get
   routine for one and the other exists, the routine will calculate
   the return value. If you call a set routine and neither exists, the
   routine will create the apcompriate comperty */

LIBICAL_ICAL_EXPORT struct icaltimetype icalcomponent_get_dtend(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_dtend(icalcomponent *comp, struct icaltimetype v);

LIBICAL_ICAL_EXPORT struct icaltimetype icalcomponent_get_due(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_due(icalcomponent *comp, struct icaltimetype v);

LIBICAL_ICAL_EXPORT void icalcomponent_set_duration(icalcomponent *comp,
                                                    struct icaldurationtype v);

LIBICAL_ICAL_EXPORT struct icaldurationtype icalcomponent_get_duration(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_method(icalcomponent *comp, icalproperty_method method);

LIBICAL_ICAL_EXPORT icalproperty_method icalcomponent_get_method(icalcomponent *comp);

LIBICAL_ICAL_EXPORT struct icaltimetype icalcomponent_get_dtstamp(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_dtstamp(icalcomponent *comp, struct icaltimetype v);

LIBICAL_ICAL_EXPORT void icalcomponent_set_summary(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_summary(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_comment(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_comment(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_uid(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_uid(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_relcalid(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_relcalid(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_recurrenceid(icalcomponent *comp,
                                                        struct icaltimetype v);

LIBICAL_ICAL_EXPORT struct icaltimetype icalcomponent_get_recurrenceid(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_description(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_description(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_location(icalcomponent *comp, const char *v);

LIBICAL_ICAL_EXPORT const char *icalcomponent_get_location(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_sequence(icalcomponent *comp, int v);

LIBICAL_ICAL_EXPORT int icalcomponent_get_sequence(icalcomponent *comp);

LIBICAL_ICAL_EXPORT void icalcomponent_set_status(icalcomponent *comp, enum icalproperty_status v);

LIBICAL_ICAL_EXPORT enum icalproperty_status icalcomponent_get_status(icalcomponent *comp);

/** Calls the given function for each TZID parameter found in the
    component, and any subcomponents. */
LIBICAL_ICAL_EXPORT void icalcomponent_foreach_tzid(icalcomponent *comp,
                                                    void (*callback) (icalparameter *param,
                                                                      void *data),
                                                    void *callback_data);

/** Returns the icaltimezone in the component corresponding to the
    TZID, or NULL if it can't be found. */
LIBICAL_ICAL_EXPORT icaltimezone *icalcomponent_get_timezone(icalcomponent *comp,
                                                             const char *tzid);

LIBICAL_ICAL_EXPORT int icalproperty_recurrence_is_excluded(icalcomponent *comp,
                                                            struct icaltimetype *dtstart,
                                                            struct icaltimetype *recurtime);

LIBICAL_ICAL_EXPORT void icalcomponent_foreach_recurrence(icalcomponent *comp,
                                                          struct icaltimetype start,
                                                          struct icaltimetype end,
                                                          void (*callback) (icalcomponent *comp,
                                                                            struct icaltime_span *
                                                                            span, void *data),
                                                          void *callback_data);

/*************** Type Specific routines ***************/

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vcalendar(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vevent(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vtodo(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vjournal(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_valarm(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vfreebusy(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vtimezone(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_xstandard(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_xdaylight(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vagenda(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vquery(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vavailability(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_xavailable(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vpoll(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_vvoter(void);

LIBICAL_ICAL_EXPORT icalcomponent *icalcomponent_new_xvote(void);

#endif /* !ICALCOMPONENT_H */
