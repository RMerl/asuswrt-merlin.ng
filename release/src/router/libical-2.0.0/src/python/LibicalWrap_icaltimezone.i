
/*======================================================================
  FILE: LibicalWrap_icaltimezone.i

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

%rename(icaltimezone) _icaltimezone;

%inline %{
#include "libical/icaltimezone.h"
#include "libical/icaltimezoneimpl.h"
%}
%include "libical/icaltimezone.h"
%include "libical/icaltimezoneimpl.h"


%pythoncode %{

import time, datetime

##### Support datetime.tzinfo API #####
# This is a "good enough" implementation right now.  Make better
# later, if needed.
class icaltzinfo(datetime.tzinfo):
    def __init__(self, icaltimezone):
        self.tz = icaltimezone

    def __cmp__(self, tzinfo):
        return cmp(self.tz, self.tz)

    def utcoffset(self, dt):
        timet = time.mktime(dt.timetuple())
        tt = icaltimetype.from_timet(int(timet),0,None)
        utcoffset = _LibicalWrap.icaltimezone_get_utc_offset(self.tz, tt, None)
        return datetime.timedelta(utcoffset)

    def dst(self, dt):
        # FIXME: Since icaltimezone_get_utc_offset does all the
        #    calc for dst internally and there is not function which
        #    returns what we need here, we'll probably need to partly
        #    reimplement icaltimezone_get_utc_offset
        return datetime.timedelta(0)

    def tzname(self, dt):
        return _LibicalWrap.icaltimezone_get_tzid(self.tz)

#    def fromutc(self, dt): pass

%}


#if 0

/** Sets the prefix to be used for tzid's generated from system tzdata.
    Must be globally unique (such as a domain name owned by the developer
    of the calling application), and begin and end with forward slashes.
    Do not change or de-allocate the string buffer after calling this.
 */
void icaltimezone_set_tzid_prefix(const char *new_prefix);

/**
 * @par Accessing timezones.
 */

/** Free any builtin timezone information **/
void icaltimezone_free_builtin_timezones(void);

/** Returns the array of builtin icaltimezones. */
icalarray* icaltimezone_get_builtin_timezones	(void);

/**
 * @par Converting times between timezones.
 */

void	icaltimezone_convert_time		(struct icaltimetype *tt,
						 icaltimezone *from_zone,
						 icaltimezone *to_zone);


/**
 * @par Getting offsets from UTC.
 */

/** Calculates the UTC offset of a given local time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
int icaltimezone_get_utc_offset	(icaltimezone *zone,
				 struct icaltimetype *tt,
				 int		*is_daylight);

/** Calculates the UTC offset of a given UTC time in the given
   timezone.  It is the number of seconds to add to UTC to get local
   time.  The is_daylight flag is set to 1 if the time is in
   daylight-savings time. */
int	icaltimezone_get_utc_offset_of_utc_time	(icaltimezone *zone,
						 struct icaltimetype *tt,
						 int		*is_daylight);


/*
 * @par Handling the default location the timezone files
 */

/** Set the directory to look for the zonefiles */
void set_zone_directory(char *path);

/** Free memory dedicated to the zonefile directory */
void free_zone_directory(void);
void icaltimezone_release_zone_tab(void);

/*
 * @par Debugging Output.
 */

/** Dumps information about changes in the timezone up to and including
   max_year. */
int	icaltimezone_dump_changes		(icaltimezone *zone,
						 int		 max_year,
						 FILE		*fp);

#endif


// Add some methods to the icaltimetype struct
%extend _icaltimezone {

    /* Might want to change this to somethingmore reasonable,
       like longitude or utc offset. */
    int __cmp__(icaltimezone *zone) {
        return strcmp(icaltimezone_get_tzid($self),
                      icaltimezone_get_tzid(zone));
    }

}

%pythoncode %{

# Remove accessors to private structure members, which is all of them
_swig_remove_private_properties(icaltimezone)

def _icaltimezone_set_component_wrap(self, comp):
    ret = _LibicalWrap.icaltimezone_set_component(self, comp)
    if not ret:
        # Not successful, raise an exception because setting a property
        # has not return value to be checked.
        raise Error.LibicalError("Failed to set component to timezone")

# Set/Overwrite icaltimezone properties
icaltimezone_props = {
    "tzid": (_LibicalWrap.icaltimezone_get_tzid, ),
    "location": (_LibicalWrap.icaltimezone_get_location, ),
    "tznames": (_LibicalWrap.icaltimezone_get_tznames, ),
    "latitude": (_LibicalWrap.icaltimezone_get_latitude, ),
    "longitude": (_LibicalWrap.icaltimezone_get_longitude, ),
    "display_name": (_LibicalWrap.icaltimezone_get_display_name, ),
    "component": (_LibicalWrap.icaltimezone_get_component,
                  _icaltimezone_set_component_wrap, ),
}

_swig_set_properties(icaltimezone, icaltimezone_props)

# UTC = _LibicalWrap.icaltimezone_get_utc_timezone()

def icaltimezone_copy(self):
    tz = _LibicalWrap.icaltimezone_copy(self)
    tz.this.acquire()
    return tz

def icaltimezone_new(self):
    # Hand off the underlying pointer by setting the this attribute
    print "newing icaltimezone"
    obj = _LibicalWrap.icaltimezone_new()
    obj.this.acquire()
    try: self.this.append(obj.this)
    except: self.this = obj.this
    
def icaltimezone_delete(self):
    # do not delete the struct because swig will do this
    if self.this.own():
        _LibicalWrap.icaltimezone_free(self, 0)
    
icaltimezone_methods = {
    'as_tzinfo': icaltzinfo,
    'copy': icaltimezone_copy,
    '__init__': icaltimezone_new, 
    '__del__': icaltimezone_delete, 
}
_swig_add_instance_methods(icaltimezone, icaltimezone_methods)

icaltimezone.get_builtin_timezone = staticmethod(_LibicalWrap.icaltimezone_get_builtin_timezone)
icaltimezone.get_builtin_timezone_from_offset = staticmethod(_LibicalWrap.icaltimezone_get_builtin_timezone_from_offset)
icaltimezone.get_builtin_timezone_from_tzid = staticmethod(_LibicalWrap.icaltimezone_get_builtin_timezone_from_tzid)

#icaltimezone.free_builtin_timezones = staticmethod(_LibicalWrap.icaltimezone_free_builtin_timezones)
#icaltimezone.get_builtin_timezones = staticmethod(_LibicalWrap.icaltimezone_get_builtin_timezones)


%}

