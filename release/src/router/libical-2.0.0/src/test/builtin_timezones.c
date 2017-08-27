/*======================================================================
 FILE: builtin_timezones.c
 CREATOR: Milan Crha 26 November 2014

 (C) COPYRIGHT 2014 Milan Crha <mcrha@redhat.com>

 The contents of this file are subject to the Mozilla Public License
 Version 1.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 the License for the specific language governing rights and
 limitations under the License.
======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libical/ical.h"

#include <stdio.h>

int main()
{
    icalarray *builtin_timezones;
    icaltimetype tt;
    int dd, hh, zz, tried = 0;
    long zz2 = -1;

    set_zone_directory("../../zoneinfo");
    icaltimezone_set_tzid_prefix("/softwarestudio.org/");

    tt = icaltime_current_time_with_zone(icaltimezone_get_builtin_timezone("America/New_York"));

    tt.year = 2038;
    (void)icaltime_as_timet_with_zone(tt, icaltimezone_get_builtin_timezone("PST"));
    tried++;

    tt.year = 2050;
    (void)icaltime_as_timet_with_zone(tt, icaltimezone_get_builtin_timezone("PST"));
    tried++;

    tt.year = 1958;
    (void)icaltime_as_timet_with_zone(tt, icaltimezone_get_builtin_timezone("PST"));
    tried++;

    builtin_timezones = icaltimezone_get_builtin_timezones();
    printf("got %lu zones\n", (unsigned long)builtin_timezones->num_elements);
    if (builtin_timezones->num_elements == 0) {
      printf("YIKES. Try running from the build/bin directory\n");
      return(1);
    }

    for (zz = -1; zz < (int)builtin_timezones->num_elements; zz++) {
        icaltimezone *zone;

        if (zz < 0) {
            zone = icaltimezone_get_utc_timezone();
        } else {
            zone = icalarray_element_at(builtin_timezones, (size_t)zz);
        }

        tt = icaltime_current_time_with_zone(zone);

        for (dd = 0; dd < 370; dd += 17) {
            for (hh = 0; hh < 60 * 60 * 24; hh += 567) {
                int zz2cnt;

                icaltime_adjust(&tt, 0, 0, 0, 1);

                for (zz2cnt = 0; zz2cnt < 15; zz2cnt++) {
                    icaltimezone *zone2;

                    if (zz2 < 0) {
                        zone2 = icaltimezone_get_utc_timezone();
                    } else {
                        zone2 = icalarray_element_at(builtin_timezones, (size_t)zz2);
                    }

                    (void)icaltime_as_timet_with_zone(tt, zone2);
                    tried++;

                    zz2++;
                    if (zz2 >= (long)builtin_timezones->num_elements)
                        zz2 = -1;
                }
            }
        }

        printf("\r%lu %% done",
               (zz >= 0 ? zz : 0) * 100 / (unsigned long)builtin_timezones->num_elements);
        fflush(stdout);
    }

    printf("\ntried %d times\n", tried);
    return 0;
}
