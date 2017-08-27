
/*======================================================================
  FILE: LibicalWrap_icaltime.i

  (C) COPYRIGHT 2010 Glenn Washburn

  The contents of this file are subject to the Mozilla Public License
  Version 1.0 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and
  limitations under the License.

  The original author is Glenn Washburn (crass@berlios.de)

  Contributions from:

  ======================================================================*/  

// Add some methods to the icaltimetype struct
%extend icaltimetype {

    /* ***** Special methods ***** */

    int __cmp__(const icaltimetype b) { return icaltime_compare(*($self), b); }
    
    /* ***** Conversion methods ***** */

    const char* as_ical_string() { return icaltime_as_ical_string(*($self)); }
    time_t as_timet(const icaltimezone *zone=NULL) {
        return icaltime_as_timet_with_zone(*($self), zone);
    }
    
    /* ***** Accessor methods ***** */
    
    const char *get_tzid() { return icaltime_get_tzid(*($self)); }
    int day_of_year() { return icaltime_day_of_year(*($self)); }
    int day_of_week() { return icaltime_day_of_week(*($self)); }

    /** Return the day of the year for the Sunday of the week that the
       given time is within. */
    /* int start_doy_of_week() { return icaltime_start_doy_of_week(*($self)); } */

    /** Return the day of the year for the first day of the week that the
       given time is within. */
    int start_doy_week(int fdow) {
        return icaltime_start_doy_week(*($self), fdow);
    }

    /** Return the week number for the week the given time is within */
    int week_number() { return icaltime_week_number(*($self)); }
    
    
    /* ***** Query methods ***** */
    
    int is_null_time() { return icaltime_is_null_time(*($self)); }

    /** Returns false if the time is clearly invalid, but is not null. This
       is usually the result of creating a new time type buy not clearing
       it, or setting one of the flags to an illegal value. */
    int is_valid_time() { return icaltime_is_valid_time(*($self)); }

    /* is_date and is_utc are both over shadowed by the struct accessors,
       but they do the same thing. */
    int is_date() { return icaltime_is_date(*($self)); }
    int is_utc() { return icaltime_is_utc(*($self)); }
    
    /* ***** Modify, compare and utility methods ***** */
    
    /** Return -1, 0, or 1 to indicate that a<b, a==b or a>b */
    int compare(const icaltimetype b) { return icaltime_compare(*($self), b); }

    /** like icaltime_compare, but only use the date parts. */
    int compare_date_only(const icaltimetype b, icaltimezone *tz=NULL) {
        if (tz == NULL)
            tz = icaltimezone_get_utc_timezone();
        return icaltime_compare_date_only_tz(*($self), b, tz);
    }

    /** Adds or subtracts a number of days, hours, minutes and seconds. */
    void  adjust(const int days, const int hours, const int minutes, const int seconds) {
        return icaltime_adjust($self, days, hours, minutes, seconds);
    }

    /** Normalize the icaltime, so that all fields are within the normal range. */
    icaltimetype normalize() { return icaltime_normalize(*($self)); }

    icaltimetype convert_to_zone(icaltimezone *zone) {
        return icaltime_convert_to_zone(*($self), zone);
    }

    /* ***** Static methods ***** */
    
    static icaltimetype from_timet(const time_t tm,
            const int is_date=0, const icaltimezone *zone=NULL) {
        return icaltime_from_timet_with_zone(tm, is_date, zone);
    }
    
    static icaltimetype null_time(void) { return icaltime_null_time(); }
    static icaltimetype null_date(void) { return icaltime_null_date(); }

    static icaltimetype current_time(const icaltimezone *zone=NULL) {
        return icaltime_current_time_with_zone(zone);
    }

    static icaltimetype today(void) { return icaltime_today(); }

#if 0
    static icaltimetype from_string(const char* str, const icaltimezone *zone=NULL) {
        /* return _with_zone(str, zone); */
        (void)zone;
        return icaltime_from_string(str);
    }
#else
    /* For the time being do not allow specifying a timezone because this
       is unimplemented as of yet. */
    static icaltimetype from_string(const char* str) {
        return icaltime_from_string(str);
    }
#endif
    
    /** Return the number of days in the given month */
    static int days_in_month(const int month, const int year) {
        return icaltime_days_in_month(month, year);
    }

    /** Return whether you've specified a leapyear or not. */
    static int is_leap_year (const int year) {
        return icaltime_is_leap_year(year);
    }

    /** Return the number of days in this year */
    /* static int days_in_year (const int year) { return icaltime_days_in_year(year); } */

}

// This is a hackish way to support adding the __str__ method to
// a class in python.  Its much easier than writing in C (that
// I've figured out).
%pythoncode %{

def __icaltimetype_str__(self):
    return "<icaltimetype (%d, %d, %d, %d, %d, %d, %d, %d)>" % (
        self.year, self.month, self.day, self.hour, self.minute,
        self.second, self.is_date, self.is_daylight)
icaltimetype.__str__ = __icaltimetype_str__

import datetime
def icaltimetype_as_datetime(self):
    "as_datetime() -> returns datetime object"
    return datetime.datetime(self.year, self.month, self.day, self.hour,
        self.minute, self.second, 0, self.timezone)
icaltimetype.as_datetime = icaltimetype_as_datetime

def icaltimetype_from_datetime(dt):
    "from_datetime() -> returns icaltimetype object"
    tt = icaltimetype()
    
    tt.year = dt.year
    tt.month = dt.month
    tt.day = dt.day
    tt.hour = dt.hour
    tt.minute = dt.minute
    tt.second = dt.second
    if dt.tzinfo:
        # TODO: convert to the right timezone, assume for now we are UTC
        tt.zone = 0
        tt.is_utc = True
    tt.is_date = False
    tt.isdaylight = False
    
    return tt
icaltimetype.from_datetime = staticmethod(icaltimetype_from_datetime)

# Remove accessors to private structure members
icaltimetype_delprops = ["is_date", "is_utc", "zone"]

_swig_remove_private_properties(icaltimetype, icaltimetype_delprops)


# Set/Overwrite icaltimetype properties
icaltimetype_props = {
    "zone": (_LibicalWrap.icaltime_get_timezone, _LibicalWrap.icaltime_set_timezone, ),
    "is_null_time": (_LibicalWrap.icaltime_is_null_time, ),
    "is_valid_time": (_LibicalWrap.icaltime_is_valid_time, ),
    # These do essentially the same thing as the default swig generated
    # accessors is_date and is_utc, but by not defining the setter, we
    # make them immutable from python
    "is_date": (_LibicalWrap.icaltime_is_date, ),
    "is_utc": (_LibicalWrap.icaltime_is_utc, ),
}

_swig_set_properties(icaltimetype, icaltimetype_props)

%}

// TODO: Add icaltime_span_* to icaltime_spantype

