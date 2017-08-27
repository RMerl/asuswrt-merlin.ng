/**
 * @file     icalbdbset_cxx.h
 * @author   dml 12/12/01
 * @brief    Definition of C++ Wrapper for icalbdbset.c
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

#ifndef ICALBDBSET_CXX_H
#define ICALBDBSET_CXX_H

#include "libical_icalss_export.h"

#include <string>

namespace LibICal
{

    class VComponent;

    class LIBICAL_ICALSS_EXPORT ICalBDBSet
    {
      public:

        ICalBDBSet();
        ICalBDBSet(const ICalBDBSet &);
        ICalBDBSet(const std::string &path, int flags);
        ICalBDBSet operator=(const ICalBDBSet &);
        ~ICalBDBSet();

      public:

        void free();
        std::string path();

        icalerrorenum add_component(VComponent *child);
        icalerrorenum remove_component(VComponent *child);
        int count_components(icalcomponent_kind kind);

        // Restrict the component returned by icalbdbset_first, _next to those
        // that pass the gauge. _clear removes the gauge
        icalerrorenum select(icalgauge *gauge);
        void clear();

        // Get and search for a component by uid
        VComponent *fetch(std::string &uid);
        VComponent *fetch_match(icalcomponent *c);
        int has_uid(std::string &uid);

        // Iterate through components. If a guage has been defined, these
        // will skip over components that do not pass the gauge
        VComponent *get_current_component();
        VComponent *get_first_component();
        VComponent *get_next_component();

        VComponent *get_component();

    };

}       // namespace LibICal

#endif
