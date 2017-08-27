/**
 * @file    icalproperty_cxx.cpp
 * @author  fnguyen (12/10/01)
 * @brief   Implementation of C++ Wrapper for icalproperty.c
 *
 * (C) COPYRIGHT 2001, Critical Path

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
 */

#include "icalproperty_cxx.h"
#include "icalparameter_cxx.h"
#include "icalvalue_cxx.h"
using namespace LibICal;

ICalProperty::ICalProperty() : imp(icalproperty_new(ICAL_ANY_PROPERTY))
{
}

ICalProperty::ICalProperty(const ICalProperty &v) throw(icalerrorenum)
  : imp(icalproperty_new_clone(v.imp))
{
    if (imp == NULL) {
        throw icalerrno;
    }
}

ICalProperty &ICalProperty::operator=(const ICalProperty &v) throw(icalerrorenum)
{
    if (this == &v) {
        return *this;
    }

    if (imp != NULL) {
        icalproperty_free(imp);
        imp = icalproperty_new_clone(v.imp);
        if (imp == NULL) {
            throw icalerrno;
        }
    }

    return *this;
}

void ICalProperty::detach()
{
    imp = NULL;
}

ICalProperty::~ICalProperty()
{
    if (imp != NULL) {
        icalproperty_free(imp);
    }
}

ICalProperty::ICalProperty(icalproperty *v) : imp(v)
{
}

ICalProperty::ICalProperty(std::string str)
  : imp(icalproperty_new_from_string(str.c_str()))
{
}

ICalProperty::ICalProperty(icalproperty_kind kind)
  : imp(icalproperty_new(kind))
{
}

std::string ICalProperty::as_ical_string()
{
    return (std::string)icalproperty_as_ical_string(imp);
}

icalproperty_kind ICalProperty::isa()
{
    return icalproperty_isa(imp);
}

int ICalProperty::isa_property(void *property)
{
    return icalproperty_isa_property(property);
}

int ICalProperty::operator==(ICalProperty &rhs)
{
    icalparameter_xliccomparetype result;
    ICalValue  *thisPropValue = this->get_value();
    ICalValue  *rhsPropValue  = rhs.get_value();
    result = icalvalue_compare((icalvalue *)*thisPropValue, (icalvalue *)*rhsPropValue);
    delete thisPropValue;
    return (result ==  ICAL_XLICCOMPARETYPE_EQUAL);
}

void ICalProperty::add_parameter(ICalParameter &parameter)
{
    icalproperty_add_parameter(imp, parameter);
}

void ICalProperty::set_parameter(ICalParameter &parameter)
{
    icalproperty_set_parameter(imp, parameter);
}

void ICalProperty::set_parameter_from_string(const std::string &name, const std::string &val)
{
    icalproperty_set_parameter_from_string(imp, name.c_str(), val.c_str());
}

std::string ICalProperty::get_parameter_as_string(const std::string &name)
{
    return (std::string)icalproperty_get_parameter_as_string(imp, name.c_str());
}

void ICalProperty::remove_parameter(const icalparameter_kind &kind)
{
    icalproperty_remove_parameter(imp, kind);
}

int ICalProperty::count_parameters()
{
    return icalproperty_count_parameters(imp);
}

/** Iterate through the parameters */
ICalParameter *ICalProperty::get_first_parameter(const icalparameter_kind &kind)
{
    icalparameter *p = icalproperty_get_first_parameter(imp, kind);
    return (p != NULL) ? new ICalParameter(p) : NULL;
}

ICalParameter *ICalProperty::get_next_parameter(const icalparameter_kind &kind)
{
    icalparameter *p = icalproperty_get_next_parameter(imp, kind);
    return (p != NULL) ? new ICalParameter(p) : NULL;
}

/** Access the value of the property */
void ICalProperty::set_value(const ICalValue &val)
{
    icalproperty_set_value(imp, (ICalValue &)val);
}

void ICalProperty::set_value_from_string(const std::string &val, const std::string &kind)
{
    icalproperty_set_value_from_string(imp, val.c_str(), kind.c_str());
}

ICalValue *ICalProperty::get_value()
{
    return new ICalValue(icalproperty_get_value(imp));
}

std::string ICalProperty::get_value_as_string()
{
    return (std::string)icalproperty_get_value_as_string(imp);
}

/** Return the name of the property -- the type name converted to a
 *  string, or the value of get_x_name if the type is X property
 */
std::string ICalProperty::get_name() const
{
    return (std::string)icalproperty_get_property_name(imp);
}

/* Deal with X properties */
void ICalProperty::set_x_name(ICalProperty &prop, const std::string &name)
{
    icalproperty_set_x_name(prop, name.c_str());
}

std::string ICalProperty::get_x_name(ICalProperty &prop)
{
    return (std::string)icalproperty_get_x_name(prop);
}

icalvalue_kind ICalProperty::value_to_value_kind(const icalparameter_value &val)
{
    return icalparameter_value_to_value_kind(val);
}

/* Convert kinds to string and get default value type */
icalvalue_kind ICalProperty::kind_to_value_kind(const icalproperty_kind &kind)
{
    return icalproperty_kind_to_value_kind(kind);
}

icalproperty_kind ICalProperty::value_kind_to_kind(const icalvalue_kind &kind)
{
    return icalproperty_value_kind_to_kind(kind);
}

std::string ICalProperty::kind_to_string(const icalproperty_kind &kind)
{
    return (std::string)icalproperty_kind_to_string(kind);
}

icalproperty_kind ICalProperty::string_to_kind(const std::string &str)
{
    return icalproperty_string_to_kind(str.c_str());
}

std::string ICalProperty::method_to_string(const icalproperty_method &method)
{
    return (std::string)icalproperty_method_to_string(method);
}

icalproperty_method ICalProperty::string_to_method(const std::string &str)
{
    return icalproperty_string_to_method(str.c_str());
}

std::string ICalProperty::enum_to_string(const int &e)
{
    return (std::string)icalproperty_enum_to_string(e);
}

int ICalProperty::string_to_enum(const std::string &str)
{
    return icalproperty_string_to_enum(str.c_str());
}

std::string ICalProperty::status_to_string(const icalproperty_status &s)
{
    return (std::string)icalproperty_status_to_string(s);
}

icalproperty_status ICalProperty::string_to_status(const std::string &str)
{
    return icalproperty_string_to_status(str.c_str());
}

int ICalProperty::enum_belongs_to_property(const icalproperty_kind &kind, const int &e)
{
    return icalproperty_enum_belongs_to_property(kind, e);
}

/* ACTION */
void ICalProperty::set_action(const enum icalproperty_action &val)
{
    icalproperty_set_action(imp, val);
}

enum icalproperty_action ICalProperty::get_action()
{
    return icalproperty_get_action(imp);
}

/* ATTACH */
void ICalProperty::set_attach(icalattach *val)
{
    icalproperty_set_attach(imp, val);
}

icalattach *ICalProperty::get_attach() const
{
    return icalproperty_get_attach(imp);
}

/* ATTENDEE */
void ICalProperty::set_attendee(const std::string &val)
{
    icalproperty_set_attendee(imp, val.c_str());
}

std::string ICalProperty::get_attendee() const
{
    return (std::string)icalproperty_get_attendee(imp);
}

/* CALSCALE */
void ICalProperty::set_calscale(const std::string &val)
{
    icalproperty_set_calscale(imp, val.c_str());
}

std::string ICalProperty::get_calscale() const
{
    return (std::string)icalproperty_get_calscale(imp);
}

/* CATEGORIES */
void ICalProperty::set_categories(const std::string &val)
{
    icalproperty_set_categories(imp, val.c_str());
}

std::string ICalProperty::get_categories() const
{
    return (std::string)icalproperty_get_categories(imp);
}

/* CLASS */
void ICalProperty::set_class(const enum icalproperty_class &val)
{
    icalproperty_set_class(imp, val);
}

enum icalproperty_class ICalProperty::get_class() const
{
    return (enum icalproperty_class)icalproperty_get_class(imp);
}

/* COMMENT */
void ICalProperty::set_comment(const std::string &val)
{
    icalproperty_set_comment(imp, val.c_str());
}

std::string ICalProperty::get_comment() const
{
    return (std::string)icalproperty_get_comment(imp);
}

/* COMPLETED */
void ICalProperty::set_completed(const struct icaltimetype &val)
{
    icalproperty_set_completed(imp, val);
}

struct icaltimetype ICalProperty::get_completed() const
{
    return icalproperty_get_completed(imp);
}

/* CONTACT */
void ICalProperty::set_contact(const std::string &val)
{
    icalproperty_set_contact(imp, val.c_str());
}

std::string ICalProperty::get_contact() const
{
    return (std::string)icalproperty_get_contact(imp);
}

/* CREATED */
void ICalProperty::set_created(const struct icaltimetype &val)
{
    icalproperty_set_created(imp, val);
}

struct icaltimetype ICalProperty::get_created() const
{
    return icalproperty_get_created(imp);
}

/* DESCRIPTION */
void ICalProperty::set_description(const std::string &val)
{
    icalproperty_set_description(imp, val.c_str());
}

std::string ICalProperty::get_description() const
{
    return (std::string)icalproperty_get_description(imp);
}

/* DTEND */
void ICalProperty::set_dtend(const struct icaltimetype &val)
{
    icalproperty_set_dtend(imp, val);
}

struct icaltimetype ICalProperty::get_dtend() const
{
    return icalproperty_get_dtend(imp);
}

/* DTSTAMP */
void ICalProperty::set_dtstamp(const struct icaltimetype &val)
{
    icalproperty_set_dtstamp(imp, val);
}

struct icaltimetype ICalProperty::get_dtstamp() const
{
    return icalproperty_get_dtstamp(imp);
}

/* DTSTART */
void ICalProperty::set_dtstart(const struct icaltimetype &val)
{
    icalproperty_set_dtstart(imp, val);
}

struct icaltimetype ICalProperty::get_dtstart() const
{
    return icalproperty_get_dtstart(imp);
}

/* DUE */
void ICalProperty::set_due(const struct icaltimetype &val)
{
    icalproperty_set_due(imp, val);
}

struct icaltimetype ICalProperty::get_due() const
{
    return icalproperty_get_due(imp);
}

/* DURATION */
void ICalProperty::set_duration(const struct icaldurationtype &val)
{
    icalproperty_set_duration(imp, val);
}

struct icaldurationtype ICalProperty::get_duration() const
{
    return icalproperty_get_duration(imp);
}

/* EXDATE */
void ICalProperty::set_exdate(const struct icaltimetype &val)
{
    icalproperty_set_exdate(imp, val);
}

struct icaltimetype ICalProperty::get_exdate() const
{
    return icalproperty_get_exdate(imp);
}

/* EXPAND */
void ICalProperty::set_expand(const int &val)
{
    icalproperty_set_expand(imp, val);
}

int ICalProperty::get_expand() const
{
    return icalproperty_get_expand(imp);
}

/* EXRULE */
void ICalProperty::set_exrule(const struct icalrecurrencetype &val)
{
    icalproperty_set_exrule(imp, val);
}

struct icalrecurrencetype ICalProperty::get_exrule() const
{
    return icalproperty_get_exrule(imp);
}

/* FREEBUSY */
void ICalProperty::set_freebusy(const struct icalperiodtype &val)
{
    icalproperty_set_freebusy(imp, val);
}

struct icalperiodtype ICalProperty::get_freebusy() const
{
    return icalproperty_get_freebusy(imp);
}

/* GEO */
void ICalProperty::set_geo(const struct icalgeotype &val)
{
    icalproperty_set_geo(imp, val);
}

struct icalgeotype ICalProperty::get_geo() const
{
    return icalproperty_get_geo(imp);
}

/* LAST-MODIFIED */
void ICalProperty::set_lastmodified(const struct icaltimetype &val)
{
    icalproperty_set_lastmodified(imp, val);
}

struct icaltimetype ICalProperty::get_lastmodified() const
{
    return icalproperty_get_lastmodified(imp);
}

/* LOCATION */
void ICalProperty::set_location(const std::string &val)
{
    icalproperty_set_location(imp, val.c_str());
}

std::string ICalProperty::get_location() const
{
    return (std::string)icalproperty_get_location(imp);
}

/* MAXRESULTS */
void ICalProperty::set_maxresults(const int &val)
{
    icalproperty_set_maxresults(imp, val);
}

int ICalProperty::get_maxresults() const
{
    return icalproperty_get_maxresults(imp);
}

/* MAXRESULTSSIZE */
void ICalProperty::set_maxresultsize(const int &val)
{
    icalproperty_set_maxresultssize(imp, val);
}

int ICalProperty::get_maxresultsize() const
{
    return icalproperty_get_maxresultssize(imp);
}

/* METHOD */
void ICalProperty::set_method(const enum icalproperty_method &val)
{
    icalproperty_set_method(imp, val);
}

enum icalproperty_method ICalProperty::get_method() const
{
    return icalproperty_get_method(imp);
}

/* ORGANIZER */
void ICalProperty::set_organizer(const std::string &val)
{
    icalproperty_set_organizer(imp, val.c_str());
}

std::string ICalProperty::get_organizer() const
{
    return (std::string)icalproperty_get_organizer(imp);
}

/* OWNER */
void ICalProperty::set_owner(const std::string &val)
{
    icalproperty_set_owner(imp, val.c_str());
}

std::string ICalProperty::get_owner() const
{
    return (std::string)icalproperty_get_owner(imp);
}

/* PERCENT-COMPLETE */
void ICalProperty::set_percentcomplete(const int &val)
{
    icalproperty_set_percentcomplete(imp, val);
}

int ICalProperty::get_percentcomplete() const
{
    return icalproperty_get_percentcomplete(imp);
}

/* PRIORITY */
void ICalProperty::set_priority(const int &val)
{
    icalproperty_set_priority(imp, val);
}

int ICalProperty::get_priority() const
{
    return icalproperty_get_priority(imp);
}

/* PRODID */
void ICalProperty::set_prodid(const std::string &val)
{
    icalproperty_set_prodid(imp, val.c_str());
}

std::string ICalProperty::get_prodid() const
{
    return (std::string)icalproperty_get_prodid(imp);
}

/* QUERY */
void ICalProperty::set_query(const std::string &val)
{
    icalproperty_set_query(imp, val.c_str());
}

std::string ICalProperty::get_query() const
{
    return (std::string)icalproperty_get_query(imp);
}

/* QUERYNAME */
void ICalProperty::set_queryname(const std::string &val)
{
    icalproperty_set_queryname(imp, val.c_str());
}

std::string ICalProperty::get_queryname() const
{
    return (std::string)icalproperty_get_queryname(imp);
}

/* RDATE */
void ICalProperty::set_rdate(const struct icaldatetimeperiodtype &val)
{
    icalproperty_set_rdate(imp, val);
}

struct icaldatetimeperiodtype ICalProperty::get_rdate() const
{
    return icalproperty_get_rdate(imp);
}

/* RECURRENCE-ID */
void ICalProperty::set_recurrenceid(const struct icaltimetype &val)
{
    icalproperty_set_recurrenceid(imp, val);
}

struct icaltimetype ICalProperty::get_recurrenceid() const
{
    return icalproperty_get_recurrenceid(imp);
}

/* RELATED-TO */
void ICalProperty::set_relatedto(const std::string &val)
{
    icalproperty_set_relatedto(imp, val.c_str());
}

std::string ICalProperty::get_relatedto() const
{
    return (std::string)icalproperty_get_relatedto(imp);
}

/* RELCALID */
void ICalProperty::set_relcalid(const std::string &val)
{
    icalproperty_set_relcalid(imp, val.c_str());
}

std::string ICalProperty::get_relcalid() const
{
    return (std::string)icalproperty_get_relcalid(imp);
}

/* REPEAT */
void ICalProperty::set_repeat(const int &val)
{
    icalproperty_set_repeat(imp, val);
}

int ICalProperty::get_repeat() const
{
    return icalproperty_get_repeat(imp);
}

/* REQUEST-STATUS */
void ICalProperty::set_requeststatus(const std::string &val)
{
    icalreqstattype v;

    v = icalreqstattype_from_string(val.c_str());

    icalproperty_set_requeststatus(imp, v);
}

std::string ICalProperty::get_requeststatus() const
{
    icalreqstattype v;
    v = icalproperty_get_requeststatus(imp);
    return (std::string)(icalreqstattype_as_string(v));
}

/* RESOURCES */
void ICalProperty::set_resources(const std::string &val)
{
    icalproperty_set_resources(imp, val.c_str());
}

std::string ICalProperty::get_resources() const
{
    return (std::string)icalproperty_get_resources(imp);
}

/* RRULE */
void ICalProperty::set_rrule(const struct icalrecurrencetype &val)
{
    icalproperty_set_rrule(imp, val);
}

struct icalrecurrencetype ICalProperty::get_rrule() const
{
    return icalproperty_get_rrule(imp);
}

/* SCOPE */
void ICalProperty::set_scope(const std::string &val)
{
    icalproperty_set_scope(imp, val.c_str());
}

std::string ICalProperty::get_scope() const
{
    return (std::string)icalproperty_get_scope(imp);
}

/* SEQUENCE */
void ICalProperty::set_sequence(const int &val)
{
    icalproperty_set_sequence(imp, val);
}

int ICalProperty::get_sequence() const
{
    return icalproperty_get_sequence(imp);
}

/* STATUS */
void ICalProperty::set_status(const enum icalproperty_status &val)
{
    icalproperty_set_status(imp, val);
}

enum icalproperty_status ICalProperty::get_status() const
{
    return icalproperty_get_status(imp);
}

/* SUMMARY */
void ICalProperty::set_summary(const std::string &val)
{
    icalproperty_set_summary(imp, val.c_str());
}

std::string ICalProperty::get_summary() const
{
    return (std::string)icalproperty_get_summary(imp);
}

/* TARGET */
void ICalProperty::set_target(const std::string &val)
{
    icalproperty_set_target(imp, val.c_str());
}

std::string ICalProperty::get_target() const
{
    return (std::string)icalproperty_get_target(imp);
}

/* TRANSP */
void ICalProperty::set_transp(const enum icalproperty_transp &val)
{
    icalproperty_set_transp(imp, val);
}

enum icalproperty_transp ICalProperty::get_transp() const
{
    return icalproperty_get_transp(imp);
}

/* TRIGGER */
void ICalProperty::set_trigger(const struct icaltriggertype &val)
{
    icalproperty_set_trigger(imp, val);
}

struct icaltriggertype ICalProperty::get_trigger() const
{
    return icalproperty_get_trigger(imp);
}

/* TZID */
void ICalProperty::set_tzid(const std::string &val)
{
    icalproperty_set_tzid(imp, val.c_str());
}

std::string ICalProperty::get_tzid() const
{
    return (std::string)icalproperty_get_tzid(imp);
}

/* TZNAME */
void ICalProperty::set_tzname(const std::string &val)
{
    icalproperty_set_tzname(imp, val.c_str());
}

std::string ICalProperty::get_tzname() const
{
    return (std::string)icalproperty_get_tzname(imp);
}

/* TZOFFSETFROM */
void ICalProperty::set_tzoffsetfrom(const int &val)
{
    icalproperty_set_tzoffsetfrom(imp, val);
}

int ICalProperty::get_tzoffsetfrom() const
{
    return icalproperty_get_tzoffsetfrom(imp);
}

/* TZOFFSETTO */
void ICalProperty::set_tzoffsetto(const int &val)
{
    icalproperty_set_tzoffsetto(imp, val);
}

int ICalProperty::get_tzoffsetto() const
{
    return icalproperty_get_tzoffsetto(imp);
}

/* TZURL */
void ICalProperty::set_tzurl(const std::string &val)
{
    icalproperty_set_tzurl(imp, val.c_str());
}

std::string ICalProperty::get_tzurl() const
{
    return (std::string)icalproperty_get_tzurl(imp);
}

/* UID */
void ICalProperty::set_uid(const std::string &val)
{
    icalproperty_set_uid(imp, val.c_str());
}

std::string ICalProperty::get_uid() const
{
    return (std::string)icalproperty_get_uid(imp);
}

/* URL */
void ICalProperty::set_url(const std::string &val)
{
    icalproperty_set_url(imp, val.c_str());
}

std::string ICalProperty::get_url() const
{
    return (std::string)icalproperty_get_url(imp);
}

/* VERSION */
void ICalProperty::set_version(const std::string &val)
{
    icalproperty_set_version(imp, val.c_str());
}

std::string ICalProperty::get_version() const
{
    return (std::string)icalproperty_get_version(imp);
}

/* X */
void ICalProperty::set_x(const std::string &val)
{
    icalproperty_set_x(imp, val.c_str());
}

std::string ICalProperty::get_x() const
{
    return (std::string)icalproperty_get_x(imp);
}

/* X-LIC-CLUSTERCOUNT */
void ICalProperty::set_xlicclustercount(const std::string &val)
{
    icalproperty_set_xlicclustercount(imp, val.c_str());
}

std::string ICalProperty::get_xlicclustercount() const
{
    return (std::string)icalproperty_get_xlicclustercount(imp);
}

/* X-LIC-ERROR */
void ICalProperty::set_xlicerror(const std::string &val)
{
    icalproperty_set_xlicerror(imp, val.c_str());
}

std::string ICalProperty::get_xlicerror() const
{
    return (std::string)icalproperty_get_xlicerror(imp);
}

/* X-LIC-MIMECHARSET */
void ICalProperty::set_xlicmimecharset(const std::string &val)
{
    icalproperty_set_xlicmimecharset(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimecharset() const
{
    return (std::string)icalproperty_get_xlicmimecharset(imp);
}

/* X-LIC-MIMECID */
void ICalProperty::set_xlicmimecid(const std::string &val)
{
    icalproperty_set_xlicmimecid(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimecid() const
{
    return (std::string)icalproperty_get_xlicmimecid(imp);
}

/* X-LIC-MIMECONTENTTYPE */
void ICalProperty::set_xlicmimecontenttype(const std::string &val)
{
    icalproperty_set_xlicmimecontenttype(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimecontenttype() const
{
    return (std::string)icalproperty_get_xlicmimecontenttype(imp);
}

/* X-LIC-MIMEENCODING */
void ICalProperty::set_xlicmimeencoding(const std::string &val)
{
    icalproperty_set_xlicmimeencoding(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimeencoding() const
{
    return (std::string)icalproperty_get_xlicmimeencoding(imp);
}

/* X-LIC-MIMEFILENAME */
void ICalProperty::set_xlicmimefilename(const std::string &val)
{
    icalproperty_set_xlicmimefilename(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimefilename() const
{
    return (std::string)icalproperty_get_xlicmimefilename(imp);
}

/* X-LIC-MIMEOPTINFO */
void ICalProperty::set_xlicmimeoptinfo(const std::string &val)
{
    icalproperty_set_xlicmimeoptinfo(imp, val.c_str());
}

std::string ICalProperty::get_xlicmimeoptinfo() const
{
    return (std::string)icalproperty_get_xlicmimeoptinfo(imp);
}
