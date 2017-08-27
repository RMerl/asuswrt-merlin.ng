/**
 * @file    icalparameter_cxx.h
 * @author  fnguyen (12/10/01)
 * @brief   Definition of C++ Wrapper for icalparameter.c
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

#ifndef ICALPARAMETER_CXX_H
#define ICALPARAMETER_CXX_H

#include "libical_ical_export.h"
#include "icptrholder_cxx.h"

extern "C"
{
#include "icalerror.h"
#include "icalparameter.h"
};

#include <string>

namespace LibICal
{

    class LIBICAL_ICAL_EXPORT ICalParameter
    {
      public:
        ICalParameter() throw(icalerrorenum);
        ICalParameter(const ICalParameter &) throw(icalerrorenum);
        ICalParameter & operator=(const ICalParameter &) throw(icalerrorenum);
        ~ICalParameter();

        explicit ICalParameter(icalparameter *v) throw(icalerrorenum);

        // Create from string of form "PARAMNAME=VALUE"
        explicit ICalParameter(const std::string & str) throw(icalerrorenum);

        // Create from just the value, the part after the "="
        explicit ICalParameter(const icalparameter_kind & kind) throw(icalerrorenum);
        ICalParameter(const icalparameter_kind & kind,
                      const std::string & str) throw(icalerrorenum);

        operator  icalparameter *()
        {
            return imp;
        }

        void detach();

      public:
        std::string as_ical_string() throw(icalerrorenum);
        bool is_valid();
        icalparameter_kind isa();
        int isa_parameter(void *param);

      public:
        /* Access the name of an X parameter */
        static void set_xname(ICalParameter & param, const std::string & v);
        static std::string get_xname(ICalParameter & param);
        static void set_xvalue(ICalParameter & param, const std::string & v);
        static std::string get_xvalue(ICalParameter & param);

        /* Convert enumerations */
        static std::string kind_to_string(const icalparameter_kind & kind);
        static icalparameter_kind string_to_kind(const std::string & str);

      public:
        /* DELEGATED-FROM */
        std::string get_delegatedfrom() const;
        void set_delegatedfrom(const std::string & v);

        /* RELATED */
        icalparameter_related get_related() const;
        void set_related(const icalparameter_related & v);

        /* SENT-BY */
        std::string get_sentby() const;
        void set_sentby(const std::string & v);

        /* LANGUAGE */
        std::string get_language() const;
        void set_language(const std::string & v);

        /* RELTYPE */
        icalparameter_reltype get_reltype() const;
        void set_reltype(const icalparameter_reltype & v);

        /* ENCODING */
        icalparameter_encoding get_encoding() const;
        void set_encoding(const icalparameter_encoding & v);

        /* ALTREP */
        std::string get_altrep() const;
        void set_altrep(const std::string & v);

        /* FMTTYPE */
        std::string get_fmttype() const;
        void set_fmttype(const std::string & v);

        /* FBTYPE */
        icalparameter_fbtype get_fbtype() const;
        void set_fbtype(const icalparameter_fbtype & v);

        /* RSVP */
        icalparameter_rsvp get_rsvp() const;
        void set_rsvp(const icalparameter_rsvp & v);

        /* RANGE */
        icalparameter_range get_range() const;
        void set_range(const icalparameter_range & v);

        /* DELEGATED-TO */
        std::string get_delegatedto() const;
        void set_delegatedto(const std::string & v);

        /* CN */
        std::string get_cn() const;
        void set_cn(const std::string & v);

        /* ROLE */
        icalparameter_role get_role() const;
        void set_role(const icalparameter_role & v);

        /* X-LIC-COMPARETYPE */
        icalparameter_xliccomparetype get_xliccomparetype() const;
        void set_xliccomparetype(const icalparameter_xliccomparetype & v);

        /* PARTSTAT */
        icalparameter_partstat get_partstat() const;
        void set_partstat(const icalparameter_partstat & v);

        /* X-LIC-ERRORTYPE */
        icalparameter_xlicerrortype get_xlicerrortype() const;
        void set_xlicerrortype(const icalparameter_xlicerrortype & v);

        /* MEMBER */
        std::string get_member() const;
        void set_member(const std::string & v);

        /* X */
        std::string get_x() const;
        void set_x(const std::string & v);

        /* CUTYPE */
        icalparameter_cutype get_cutype() const;
        void set_cutype(const icalparameter_cutype & v);

        /* TZID */
        std::string get_tzid() const;
        void set_tzid(const std::string & v);

        /* VALUE */
        icalparameter_value get_value() const;
        void set_value(const icalparameter_value & v);

        /* DIR */
        std::string get_dir() const;
        void set_dir(const std::string & v);

      private:
        icalparameter *imp;
    };

}       // namespace LibICal

#endif
