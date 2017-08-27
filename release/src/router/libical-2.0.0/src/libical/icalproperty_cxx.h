/**
 * @file    icalproperty_cxx.h
 * @author  fnguyen (12/10/01)
 * @brief   Definition of C++ Wrapper for icalproperty.c
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

#ifndef ICALPROPERTY_CXX_H
#define ICALPROPERTY_CXX_H

#include "libical_ical_export.h"
#include "icptrholder_cxx.h"

extern "C"
{
#include "icalerror.h"
#include "icalproperty.h"
};

#include <string>

namespace LibICal
{

    class ICalParameter;
    class ICalValue;

    class LIBICAL_ICAL_EXPORT ICalProperty
    {
      public:
        ICalProperty();
        ICalProperty(const ICalProperty &) throw(icalerrorenum);
        ICalProperty & operator=(const ICalProperty &) throw(icalerrorenum);
        ~ICalProperty();

        explicit ICalProperty(icalproperty *v);
        explicit ICalProperty(std::string str);
        explicit ICalProperty(icalproperty_kind kind);
        ICalProperty(icalproperty_kind kind, std::string str);

        operator  icalproperty *()
        {
            return imp;
        }
        int operator==(ICalProperty & rhs);

        void detach();

      public:
        std::string as_ical_string();
        icalproperty_kind isa();
        int isa_property(void *property);

        void add_parameter(ICalParameter & parameter);
        void set_parameter(ICalParameter & parameter);
        void set_parameter_from_string(const std::string & name, const std::string & val);
        std::string get_parameter_as_string(const std::string & name);
        void remove_parameter(const icalparameter_kind & kind);
        int count_parameters();

    /** Iterate through the parameters */
        ICalParameter *get_first_parameter(const icalparameter_kind & kind);
        ICalParameter *get_next_parameter(const icalparameter_kind & kind);

    /** Access the value of the property */
        void set_value(const ICalValue & val);
        void set_value_from_string(const std::string & val, const std::string & kind);

        ICalValue *get_value();
        std::string get_value_as_string();

    /** Return the name of the property -- the type name converted
     *  to a string, or the value of get_x_name if the type is X
     *  property
     */
        std::string get_name() const;

      public:
        /* Deal with X properties */
        static void set_x_name(ICalProperty & prop, const std::string & name);
        static std::string get_x_name(ICalProperty & prop);

        static icalvalue_kind value_to_value_kind(const icalparameter_value & val);

        /* Convert kinds to string and get default value type */
        static icalvalue_kind kind_to_value_kind(const icalproperty_kind & kind);
        static icalproperty_kind value_kind_to_kind(const icalvalue_kind & kind);
        static std::string kind_to_string(const icalproperty_kind & kind);
        static icalproperty_kind string_to_kind(const std::string & str);

        static icalproperty_method string_to_method(const std::string & str);
        static std::string method_to_string(const icalproperty_method & method);

        static std::string enum_to_string(const int &e);
        static int string_to_enum(const std::string & str);

        static std::string status_to_string(const icalproperty_status & status);
        static icalproperty_status string_to_status(const std::string & str);

        static int enum_belongs_to_property(const icalproperty_kind & kind, const int &e);

      public:
        /* ACTION */
        void set_action(const enum icalproperty_action &v);
        enum icalproperty_action get_action();

        /* ATTACH */
        void set_attach(icalattach *v);
        icalattach *get_attach() const;

        /* ATTENDEE */
        void set_attendee(const std::string & val);
        std::string get_attendee() const;

        /* CALSCALE */
        void set_calscale(const std::string & val);
        std::string get_calscale() const;

        /* CATEGORIES */
        void set_categories(const std::string & val);
        std::string get_categories() const;

        /* CLASS */
        void set_class(const enum icalproperty_class &val);
        enum icalproperty_class get_class() const;

        /* COMMENT */
        void set_comment(const std::string & val);
        std::string get_comment() const;

        /* COMPLETED */
        void set_completed(const struct icaltimetype &val);
        struct icaltimetype get_completed() const;

        /* CONTACT */
        void set_contact(const std::string & val);
        std::string get_contact() const;

        /* CREATED */
        void set_created(const struct icaltimetype &val);
        struct icaltimetype get_created() const;

        /* DESCRIPTION */
        void set_description(const std::string & val);
        std::string get_description() const;

        /* DTEND */
        void set_dtend(const struct icaltimetype &val);
        struct icaltimetype get_dtend() const;

        /* DTSTAMP */
        void set_dtstamp(const struct icaltimetype &val);
        struct icaltimetype get_dtstamp() const;

        /* DTSTART */
        void set_dtstart(const struct icaltimetype &val);
        struct icaltimetype get_dtstart() const;

        /* DUE */
        void set_due(const struct icaltimetype &val);
        struct icaltimetype get_due() const;

        /* DURATION */
        void set_duration(const struct icaldurationtype &val);
        struct icaldurationtype get_duration() const;

        /* EXDATE */
        void set_exdate(const struct icaltimetype &val);
        struct icaltimetype get_exdate() const;

        /* EXPAND */
        void set_expand(const int &val);
        int get_expand() const;

        /* EXRULE */
        void set_exrule(const struct icalrecurrencetype &val);
        struct icalrecurrencetype get_exrule() const;

        /* FREEBUSY */
        void set_freebusy(const struct icalperiodtype &val);
        struct icalperiodtype get_freebusy() const;

        /* GEO */
        void set_geo(const struct icalgeotype &val);
        struct icalgeotype get_geo() const;

        /* GRANT */
        void set_grant(const std::string & val);
        std::string get_grant() const;

        /* LAST-MODIFIED */
        void set_lastmodified(const struct icaltimetype &val);
        struct icaltimetype get_lastmodified() const;

        /* LOCATION */
        void set_location(const std::string & val);
        std::string get_location() const;

        /* MAXRESULTS */
        void set_maxresults(const int &val);
        int get_maxresults() const;

        /* MAXRESULTSSIZE */
        void set_maxresultsize(const int &val);
        int get_maxresultsize() const;

        /* METHOD */
        void set_method(const enum icalproperty_method &val);
        enum icalproperty_method get_method() const;

        /* OWNER */
        void set_owner(const std::string & val);
        std::string get_owner() const;

        /* ORGANIZER */
        void set_organizer(const std::string & val);
        std::string get_organizer() const;

        /* PERCENT-COMPLETE */
        void set_percentcomplete(const int &val);
        int get_percentcomplete() const;

        /* PRIORITY */
        void set_priority(const int &val);
        int get_priority() const;

        /* PRODID */
        void set_prodid(const std::string & val);
        std::string get_prodid() const;

        /* QUERY */
        void set_query(const std::string & val);
        std::string get_query() const;

        /* QUERYNAME */
        void set_queryname(const std::string & val);
        std::string get_queryname() const;

        /* RDATE */
        void set_rdate(const struct icaldatetimeperiodtype &val);
        struct icaldatetimeperiodtype get_rdate() const;

        /* RECURRENCE-ID */
        void set_recurrenceid(const struct icaltimetype &val);
        struct icaltimetype get_recurrenceid() const;

        /* RELATED-TO */
        void set_relatedto(const std::string & val);
        std::string get_relatedto() const;

        /* RELCALID */
        void set_relcalid(const std::string & val);
        std::string get_relcalid() const;

        /* REPEAT */
        void set_repeat(const int &val);
        int get_repeat() const;

        /* REQUEST-STATUS */
        void set_requeststatus(const std::string & val);
        std::string get_requeststatus() const;

        /* RESOURCES */
        void set_resources(const std::string & val);
        std::string get_resources() const;

        /* RRULE */
        void set_rrule(const struct icalrecurrencetype &val);
        struct icalrecurrencetype get_rrule() const;

        /* SCOPE */
        void set_scope(const std::string & val);
        std::string get_scope() const;

        /* SEQUENCE */
        void set_sequence(const int &val);
        int get_sequence() const;

        /* STATUS */
        void set_status(const enum icalproperty_status &val);
        enum icalproperty_status get_status() const;

        /* SUMMARY */
        void set_summary(const std::string & val);
        std::string get_summary() const;

        /* TARGET */
        void set_target(const std::string & val);
        std::string get_target() const;

        /* TRANSP */
        void set_transp(const enum icalproperty_transp &val);
        enum icalproperty_transp get_transp() const;

        /* TRIGGER */
        void set_trigger(const struct icaltriggertype &val);
        struct icaltriggertype get_trigger() const;

        /* TZID */
        void set_tzid(const std::string & val);
        std::string get_tzid() const;

        /* TZNAME */
        void set_tzname(const std::string & val);
        std::string get_tzname() const;

        /* TZOFFSETFROM */
        void set_tzoffsetfrom(const int &val);
        int get_tzoffsetfrom() const;

        /* TZOFFSETTO */
        void set_tzoffsetto(const int &val);
        int get_tzoffsetto() const;

        /* TZURL */
        void set_tzurl(const std::string & val);
        std::string get_tzurl() const;

        /* UID */
        void set_uid(const std::string & val);
        std::string get_uid() const;

        /* URL */
        void set_url(const std::string & val);
        std::string get_url() const;

        /* VERSION */
        void set_version(const std::string & val);
        std::string get_version() const;

        /* X */
        void set_x(const std::string & val);
        std::string get_x() const;

        /* X-LIC-CLUSTERCOUNT */
        void set_xlicclustercount(const std::string & val);
        std::string get_xlicclustercount() const;

        /* X-LIC-ERROR */
        void set_xlicerror(const std::string & val);
        std::string get_xlicerror() const;

        /* X-LIC-MIMECHARSET */
        void set_xlicmimecharset(const std::string & val);
        std::string get_xlicmimecharset() const;

        /* X-LIC-MIMECID */
        void set_xlicmimecid(const std::string & val);
        std::string get_xlicmimecid() const;

        /* X-LIC-MIMECONTENTTYPE */
        void set_xlicmimecontenttype(const std::string & val);
        std::string get_xlicmimecontenttype() const;

        /* X-LIC-MIMEENCODING */
        void set_xlicmimeencoding(const std::string & val);
        std::string get_xlicmimeencoding() const;

        /* X-LIC-MIMEFILENAME */
        void set_xlicmimefilename(const std::string & val);
        std::string get_xlicmimefilename() const;

        /* X-LIC-MIMEOPTINFO */
        void set_xlicmimeoptinfo(const std::string & val);
        std::string get_xlicmimeoptinfo() const;

      private:
        icalproperty *imp;
                       /**< The actual C based icalproperty */
    };

}       // namespace LibICal

typedef ICPointerHolder < LibICal::ICalProperty > ICalPropertyTmpPtr;

#endif /* ICalProperty_H */
