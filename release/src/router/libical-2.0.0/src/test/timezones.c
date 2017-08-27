/*
======================================================================

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

#include <stdlib.h>

int main()
{
    icalarray *timezones;
    icaltimezone *zone, *utc_zone;
    char *zone_location;
    size_t i;
    int ret = 0;
    unsigned int total_failed = 0;
    unsigned int total_okay = 0;
    unsigned int percent_failed = 0;
    int verbose = 0;

    int day;
    time_t start_time;
    struct tm start_tm;
    time_t curr_time;
    struct tm curr_tm;
    struct icaltimetype curr_tt;
    int failed = 0;
    int curr_failed;
    int zonedef_printed = 0;
#if !defined(HAVE_SETENV)
    static char new_tz[256];
#endif

    set_zone_directory("../../zoneinfo");
    icaltimezone_set_tzid_prefix("/softwarestudio.org/");

    timezones = icaltimezone_get_builtin_timezones();
    utc_zone = icaltimezone_get_utc_timezone();

    /* for all known time zones... */
    for (i = 0; i < timezones->num_elements; i++) {
        zone = (icaltimezone *)icalarray_element_at(timezones, i);
        zone_location = (char *)icaltimezone_get_location(zone);

        /*
         * select this location for glibc: needs support for TZ=<location>
         * which is not POSIX
         */
#if defined(HAVE_SETENV)
        setenv("TZ", zone_location, 1);
#else
        new_tz[0] = '\0';
        strncat(new_tz, "TZ=", 255);
        strncat(new_tz, zone_location, 255);
        putenv(new_tz);
#endif
        tzset();
        /*
         * determine current local time and date: always use midday in
         * the current zone and first day of first month in the year
         */
        start_time = time(NULL);
        localtime_r(&start_time, &start_tm);
        start_tm.tm_hour = 12;
        start_tm.tm_min = 0;
        start_tm.tm_sec = 0;
        start_tm.tm_mday = 1;
        start_tm.tm_mon = 0;
        start_time = mktime(&start_tm);

        /* check time conversion for the next 365 days */
        for (day = 0, curr_time = start_time; day < 365; day++, curr_time += 24 * 60 * 60) {
            /* determine date/time with glibc */
            localtime_r(&curr_time, &curr_tm);
            /* determine date/time with libical */
            curr_tt = icaltime_from_timet_with_zone(curr_time, 0, utc_zone);
            curr_tt.zone = utc_zone;    /* workaround: icaltime_from_timet_with_zone()
                                           should do this, but doesn't! */
            curr_tt = icaltime_convert_to_zone(curr_tt, zone);

            /* compare... */
            curr_failed =
                curr_tm.tm_year + 1900 != curr_tt.year ||
                curr_tm.tm_mon + 1 != curr_tt.month ||
                curr_tm.tm_mday != curr_tt.day ||
                curr_tm.tm_hour != curr_tt.hour ||
                curr_tm.tm_min != curr_tt.minute ||
                curr_tm.tm_sec != curr_tt.second;

            /* only print first failed day and first day which is okay again */
            if (verbose || curr_failed != failed) {
                struct tm utc_tm;

                gmtime_r(&curr_time, &utc_tm);
                printf(
                    "%s: day %03d: %s: %04d-%02d-%02d %02d:%02d:%02d UTC = "
                    "libc %04d-%02d-%02d %02d:%02d:%02d dst %d",
                    zone_location, day,
                    verbose ? (curr_failed ? "failed" : "okay") : (curr_failed ? "first failed" :
                                                                   "okay again"),
                    utc_tm.tm_year + 1900, utc_tm.tm_mon + 1, utc_tm.tm_mday, utc_tm.tm_hour,
                    utc_tm.tm_min, utc_tm.tm_sec, curr_tm.tm_year + 1900, curr_tm.tm_mon + 1,
                    curr_tm.tm_mday, curr_tm.tm_hour, curr_tm.tm_min, curr_tm.tm_sec,
                    curr_tm.tm_isdst);
                if (curr_failed) {
                    printf(" != libical %04d-%02d-%02d %02d:%02d:%02d dst %d",
                        curr_tt.year,
                        curr_tt.month,
                        curr_tt.day,
                        curr_tt.hour,
                        curr_tt.minute,
                        curr_tt.second,
                        curr_tt.is_daylight);
                    ret = 1;
                }
                printf("\n");
                failed = curr_failed;

                if (!zonedef_printed) {
                    icalcomponent *comp = icaltimezone_get_component(zone);

                    if (comp) {
                        printf("%s\n", icalcomponent_as_ical_string(comp));
                    }
                    zonedef_printed = 1;
                }
            }

            if (curr_failed) {
                total_failed++;
            } else {
                total_okay++;
            }
        }
    }

    if (total_failed || total_okay) {
        percent_failed = total_failed * 100 / (total_failed + total_okay);
        printf(" *** Summary: %lu zones tested, %u days failed, %u okay => %u%% failed ***\n",
               (unsigned long)timezones->num_elements, total_failed, total_okay, percent_failed);

        if (!icaltzutil_get_exact_vtimezones_support()) {
            if (!percent_failed) {
                ret = 0;
                printf(" *** Expect some small error rate with inter-operable vtimezones *** \n");
            }
       }
    }

    return ret;
}
