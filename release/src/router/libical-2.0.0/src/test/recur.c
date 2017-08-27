/*======================================================================
 FILE: recur.c
 CREATOR: ebusboom 8jun00

 (C) COPYRIGHT 1999 Eric Busboom <eric@softwarestudio.org>
      http://www.softwarestudio.org

 DESCRIPTION:

 Test program for expanding recurrences. Run as:

     ./recur ../../test-data/recur.txt

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
#include "libicalss/icalss.h"

#include <stdlib.h>

#if defined(HAVE_SIGNAL) && defined(HAVE_ALARM)
static void sig_alrm(int i)
{
    _unused(i);
    fprintf(stderr, "Could not get lock on file\n");
    exit(1);
}

#endif

static void recur_callback(icalcomponent *comp, struct icaltime_span *span, void *data)
{
    _unused(comp);
    _unused(data);
    printf("cb: %s", ctime(&span->start));
    printf("    %s\n", ctime(&span->end));
}

int main(int argc, char *argv[])
{
    icalset *cin;
    struct icaltimetype next;
    icalcomponent *itr;
    icalproperty *desc, *dtstart, *rrule;
    struct icalrecurrencetype recur;
    icalrecur_iterator *ritr;
    time_t tt;
    char *file;

    icalerror_set_error_state(ICAL_PARSE_ERROR, ICAL_ERROR_NONFATAL);

#if defined(HAVE_SIGNAL) && defined(HAVE_ALARM)
    (void)signal(SIGALRM, sig_alrm);
#endif

    if (argc <= 1) {
        file = "../../test-data/recur.txt";
    } else if (argc == 2) {
        file = argv[1];
    } else {
        fprintf(stderr, "usage: recur [input file]\n");
        exit(1);
    }

#if defined(HAVE_SIGNAL) && defined(HAVE_ALARM)
    alarm(300); /* to get file lock */
#endif
    cin = icalfileset_new(file);
#if defined(HAVE_SIGNAL) && defined(HAVE_ALARM)
    alarm(0);
#endif

    if (cin == 0) {
        fprintf(stderr, "recur: can't open file %s\n", file);
        exit(1);
    }

    for (itr = icalfileset_get_first_component(cin);
         itr != 0; itr = icalfileset_get_next_component(cin)) {

        struct icaltimetype start;
        struct icaltimetype end = icaltime_today();

        desc = icalcomponent_get_first_property(itr, ICAL_DESCRIPTION_PROPERTY);
        dtstart = icalcomponent_get_first_property(itr, ICAL_DTSTART_PROPERTY);
        rrule = icalcomponent_get_first_property(itr, ICAL_RRULE_PROPERTY);

        if (desc == 0 || dtstart == 0 || rrule == 0) {
            printf("\n******** Error in input component ********\n");
            printf("The following component is malformed:\n %s\n",
                   icalcomponent_as_ical_string(itr));
            continue;
        }

        printf("\n\n#### %s\n", icalproperty_get_description(desc));
        printf("#### %s\n", icalvalue_as_ical_string(icalproperty_get_value(rrule)));
        recur = icalproperty_get_rrule(rrule);
        start = icalproperty_get_dtstart(dtstart);

        ritr = icalrecur_iterator_new(recur, start);

        tt = icaltime_as_timet(start);

        printf("#### %s\n", ctime(&tt));

        icalrecur_iterator_free(ritr);

        ritr = icalrecur_iterator_new(recur, start);
        for (next = icalrecur_iterator_next(ritr);
             !icaltime_is_null_time(next);
             next = icalrecur_iterator_next(ritr)) {
            tt = icaltime_as_timet(next);
            printf("  %s", ctime(&tt));
        }
        icalrecur_iterator_free(ritr);

        icalcomponent_foreach_recurrence(itr, start, end, recur_callback, NULL);
    }

    icalset_free(cin);

    icaltimezone_free_builtin_timezones();

    icalmemory_free_ring();

    free_zone_directory();

    return 0;
}
