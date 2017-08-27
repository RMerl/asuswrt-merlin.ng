/*======================================================================
 FILE: icalvcal.c
 CREATOR: eric 25 May 00

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The original code is icalvcal.c

 The icalvcal_convert routine calls icalvcal_traverse_objects to do
 its work.s his routine steps through through all of the properties
 and components of a VObject. For each name of a property or a
 component, icalvcal_traverse_objects looks up the name in
 conversion_table[]. This table indicates whether the name is of a
 component or a property, lists a routine to handle conversion, and
 has extra data for the conversion.

 The conversion routine will create new iCal components or properties
 and add them to the iCal component structure.

 The most common conversion routine is dc_prop. This routine converts
 properties for which the text representation of the vCal component
 is identical the iCal representation.
======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalvcal.h"
#include "icalerror.h"
#include "icalvalue.h"
#include "icalversion.h"        /* for ICAL_PACKAGE */

#include <stddef.h>     /* for ptrdiff_t */

enum datatype
{
    DT_COMPONENT,
    DT_PROPERTY,
    DT_PARAMETER,
    DT_UNSUPPORTED,
    DT_IGNORE
};

/* The indices must match between the strings and the codes. */
static char *weekdays[] = { "SU", "MO", "TU", "WE", "TH", "FR", "SA" };

static int weekday_codes[] = {
    ICAL_SUNDAY_WEEKDAY,
    ICAL_MONDAY_WEEKDAY,
    ICAL_TUESDAY_WEEKDAY,
    ICAL_WEDNESDAY_WEEKDAY,
    ICAL_THURSDAY_WEEKDAY,
    ICAL_FRIDAY_WEEKDAY,
    ICAL_SATURDAY_WEEKDAY
};

struct conversion_table_struct
{
    char *vcalname;
    enum datatype type;
    void *(*conversion_func) (int icaltype, VObject *o, icalcomponent *comp,
                              icalvcal_defaults *defaults);
    int icaltype;
};

static void *dc_prop(int icaltype, VObject *object, icalcomponent *comp,
                     icalvcal_defaults *defaults);

/* Creates an error property with the given message. */
static icalproperty *create_parse_error_property(const char *message,
                                                 const char *property_name,
                                                 const char *property_value)
{
    char temp[4096];
    icalparameter *error_param;
    icalproperty *error_prop;

    snprintf(temp, 1024, "%s: %s:%s", message, property_name, property_value);

    error_param = icalparameter_new_xlicerrortype(ICAL_XLICERRORTYPE_VCALPROPPARSEERROR);
    error_prop = icalproperty_new_xlicerror(temp);
    icalproperty_add_parameter(error_prop, error_param);

    return error_prop;
}

static char *get_string_value(VObject *object, int *free_string)
{
    switch (vObjectValueType(object)) {
    case VCVT_USTRINGZ:
        *free_string = 1;
        return fakeCString(vObjectUStringZValue(object));

    case VCVT_STRINGZ:
        *free_string = 0;
        return (char *)vObjectStringZValue(object);
    }

    *free_string = 0;

    /* We return "" here, to cut down on the risk of crashing. */
    return "";
}

static void convert_floating_time_to_utc(struct icaltimetype *itt)
{
    struct tm tmp_tm, utc_tm;
    time_t t;

    /* We assume the floating time is using the current Unix timezone.
       So we convert to a time_t using mktime(), and then back to a struct tm
       using gmtime, so it is the UTC time. */
    tmp_tm.tm_year = itt->year - 1900;
    tmp_tm.tm_mon = itt->month - 1;
    tmp_tm.tm_mday = itt->day;
    tmp_tm.tm_hour = itt->hour;
    tmp_tm.tm_min = itt->minute;
    tmp_tm.tm_sec = itt->second;
    tmp_tm.tm_isdst = -1;

    /* Convert to a time_t. */
    t = mktime(&tmp_tm);

    /* Now convert back to a struct tm, but with a UTC time. */
    gmtime_r(&t, &utc_tm);

    /* Now put it back into the icaltime. */
    itt->year = utc_tm.tm_year + 1900;
    itt->month = utc_tm.tm_mon + 1;
    itt->day = utc_tm.tm_mday;
    itt->hour = utc_tm.tm_hour;
    itt->minute = utc_tm.tm_min;
    itt->second = utc_tm.tm_sec;

    /* Set the is_utc flag. */
    itt->is_utc = 1;
}

static void icalvcal_traverse_objects(VObject *,
                                      icalcomponent *, icalproperty *, icalvcal_defaults *);

icalcomponent *icalvcal_convert_with_defaults(VObject *object, icalvcal_defaults *defaults)
{
    char *name = (char *)vObjectName(object);
    icalcomponent *container;
    icalcomponent *root;
    icalproperty *prop;

    icalerror_check_arg_rz((object != 0), "Object");

    container = icalcomponent_new(ICAL_XROOT_COMPONENT);

    /* The root object must be a VCALENDAR */
    if (*name == 0 || strcmp(name, VCCalProp) != 0) {
        icalcomponent_free(container);
        return 0;       /* HACK. Should return an error */
    }
#if 0
    /* Just for testing. */
    printf("This is the internal VObject representation:\n");
    printf("===========================================\n");
    printVObject(stdout, object);
    printf("===========================================\n");
#endif

    icalvcal_traverse_objects(object, container, 0, defaults);

    /* HACK. I am using the extra 'container' component because I am
       lazy. I know there is a way to get rid of it, but I did not care
       to find it. */

    root = icalcomponent_get_first_component(container, ICAL_ANY_COMPONENT);

    icalcomponent_remove_component(container, root);
    icalcomponent_free(container);

    /* We add a VERSION and PRODID here, to make it a valid iCalendar object,
       but the application may change them if necessary. */
    prop = icalproperty_new_prodid("-//Softwarestudio.org//" ICAL_PACKAGE
                                   " version " ICAL_VERSION "//EN");
    icalcomponent_add_property(root, prop);

    prop = icalproperty_new_version("2.0");
    icalcomponent_add_property(root, prop);

    return root;
}

icalcomponent *icalvcal_convert(VObject *object)
{
    return icalvcal_convert_with_defaults(object, NULL);
}

/* comp() is useful for most components, but alarm, daylight and
 * timezone are different. In vcal, they are properties, and in ical,
 * they are components. Although because of the way that vcal treats
 * everything as a property, alarm_comp() daylight_comp() and
 * timezone_comp() may not really be necessary, I think it would be
 * easier to use them. */

static void *comp(int icaltype, VObject *o, icalcomponent *comp, icalvcal_defaults *defaults)
{
    icalcomponent_kind kind = (icalcomponent_kind) icaltype;
    icalcomponent *c = icalcomponent_new(kind);

    _unused(o);
    _unused(comp);
    _unused(defaults);

    return (void *)c;
}

/* vCalendar has 4 properties for alarms: AALARM, DALARM, MALARM, PALARM
   (for audio, display, mail, and procedure alarms).

   AALARM has Run Time, Snooze Time, Repeat Count, Audio Content.
        It may also have a TYPE parameter specifying the MIME type, e.g.
      AALARM;TYPE=WAVE;VALUE=URL:19960415T235959; ; ; file:///mmedia/taps.wav
      AALARM;TYPE=WAVE;VALUE=CONTENT-ID:19960903T060000;PT15M;4;<jsmith.part2.=
        960901T083000.xyzMail@host1.com>

   DALARM has Run Time, Snooze Time, Repeat Count, Display String.
      DALARM:19960415T235000;PT5M;2;Your Taxes Are Due !!!

   MALARM has Run Time, Snooze Time, Repeat Count, Email Address, Note.
      MALARM:19960416T000000;PT1H;24;IRS@us.gov;The Check Is In The Mail!

   PALARM has Run Time, Snooze Time, Repeat Count, Procedure Name.
      PALARM;VALUE=URL:19960415T235000;PT5M;2;file:///myapps/shockme.exe

   AALARM and PALARM: Check the VALUE is a URL. We won't support CONTENT-ID.

   iCalendar uses one component, VALARM, for all of these, and uses an ACTION
   property of "AUDIO", "DISPLAY", "EMAIL" or "PROCEDURE".

   The Run Time value can be copied into the iCalendar TRIGGER property,
   except it must be UTC in iCalendar. If it is floating, we'll convert to
   UTC using the current Unix timezone.

   The Snooze Time becomes DURATION, and the Repeat Count becomes REPEAT.

   For AALARM, the Audio Content becomes the ATTACH property, and the TYPE
   becomes a FMTTYPE of this property (PCM -> 'audio/basic' (?),
   WAVE -> 'audio/x-wav', AIFF -> 'audio/x-aiff'), e.g.
     ATTACH;FMTTYPE=audio/basic:ftp://host.com/pub/sounds/bell-01.aud

   For DALARM, Display String becomes the DESCRIPTION property.

   For MALARM, Email Address becomes an ATTENDEE property, e.g.
     ATTENDEE:MAILTO:john_doe@host.com

   For PALARM, the Procedure Name becomes an ATTACH property, like AALARM, e.g.
     ATTACH;FMTTYPE=application/binary:ftp://host.com/novo-procs/felizano.exe
*/

/* This converts the vCalendar alarm properties into iCalendar properties and
   adds them to the component. It returns 1 if the alarm is valid, 0 if not. */
static int get_alarm_properties(icalcomponent *comp, VObject *object,
                                int icaltype, icalvcal_defaults *defaults)
{
    VObjectIterator iterator;
    icalproperty *trigger_prop = NULL, *duration_prop = NULL;
    icalproperty *repeat_prop = NULL, *attach_prop = NULL;
    icalproperty *summary_prop = NULL, *description_prop = NULL;
    icalproperty *action_prop, *attendee_prop = NULL;
    icalparameter *fmttype_param = NULL;
    enum icalproperty_action action = ICAL_ACTION_NONE;
    int value_is_url = 0, is_valid_alarm = 1;

    initPropIterator(&iterator, object);
    while (moreIteration(&iterator)) {
        VObject *eachProp = nextVObject(&iterator);
        const char *name = vObjectName(eachProp);
        char *s;
        int free_string;

        s = get_string_value(eachProp, &free_string);

        /* Common properties. */
        if (!strcmp(name, VCRunTimeProp)) {
            if (*s) {
                struct icaltriggertype t;
                icalparameter *param;

                /* Convert it to an icaltimetype. */
                t.time = icaltime_from_string(s);
                t.duration = icaldurationtype_null_duration();

                /* If it is a floating time, convert it to a UTC time. */
                if (!t.time.is_utc)
                    convert_floating_time_to_utc(&t.time);

                /* Create a TRIGGER property. */
                trigger_prop = icalproperty_new_trigger(t);

                /* vCalendar triggers are always specific DATE-TIME values. */
                param = icalparameter_new_value(ICAL_VALUE_DATETIME);
                icalproperty_add_parameter(trigger_prop, param);

                icalcomponent_add_property(comp, trigger_prop);
            }

        } else if (!strcmp(name, VCSnoozeTimeProp)) {
            struct icaldurationtype d;

            /* Parse the duration string.
               FIXME: vCalendar also permits 'Y' (Year) and 'M' (Month) here,
               which we don't handle at present. Though it is unlikely they
               will be used as a snooze time between repeated alarms! */
            d = icaldurationtype_from_string(s);

            duration_prop = icalproperty_new_duration(d);
            icalcomponent_add_property(comp, duration_prop);

        } else if (!strcmp(name, VCRepeatCountProp)) {
            /* If it starts with a digit convert it into a REPEAT property. */
            if (*s && *s >= '0' && *s <= '9') {
                repeat_prop = icalproperty_new_repeat(atoi(s));
                icalcomponent_add_property(comp, repeat_prop);
            }

        } else if (!strcmp(name, VCValueProp)) {
            /* We just remember if the value is a URL. */
            if (!strcmp(s, "URL")) {
                value_is_url = 1;
            }

            /* Audio properties && Procedure properties. */
        } else if (!strcmp(name, VCAudioContentProp) ||
                   !strcmp(name, VCProcedureNameProp)) {
            if (*s && !attach_prop) {
                icalattach *attach;

                attach = icalattach_new_from_url(s);
                attach_prop = icalproperty_new_attach(attach);
                icalcomponent_add_property(comp, attach_prop);
                icalattach_unref(attach);

                /* We output a "application/binary" FMTTYPE for Procedure
                   alarms. */
                if (!strcmp(name, VCProcedureNameProp) && !fmttype_param)
                    fmttype_param = icalparameter_new_fmttype("application/binary");
            }

        } else if (!strcmp(name, "TYPE")) {
            char *fmttype = NULL;

            if (!strcmp(s, "PCM")) {
                fmttype = "audio/basic";
            } else if (!strcmp(s, "AIFF")) {
                fmttype = "audio/x-aiff";
            } else if (!strcmp(s, "WAVE")) {
                fmttype = "audio/x-wav";
            }

            if (fmttype)
                fmttype_param = icalparameter_new_fmttype(fmttype);

            /* Display properties. */
        } else if (!strcmp(name, VCDisplayStringProp)) {
            if (!description_prop) {
                description_prop = icalproperty_new_description(s);
                icalcomponent_add_property(comp, description_prop);
            }

            /* Mail properties. */
        } else if (!strcmp(name, VCEmailAddressProp)) {
            if (*s && strlen(s) < 1000) {
                char buffer[1024];

                /* We need to add 'MAILTO:' before the email address, to make
                   it valid iCalendar. */
                snprintf(buffer, 1024, "MAILTO:%s", s);
                attendee_prop = icalproperty_new_attendee(buffer);
                icalcomponent_add_property(comp, attendee_prop);
            }

        } else if (!strcmp(name, VCNoteProp)) {
            if (!description_prop) {
                description_prop = icalproperty_new_description(s);
                icalcomponent_add_property(comp, description_prop);
            }

            /* We also copy the Note to the SUMMARY property, since that is
               required in iCalendar. */
            if (!summary_prop) {
                summary_prop = icalproperty_new_summary(s);
                icalcomponent_add_property(comp, summary_prop);
            }
        }

        if (free_string)
            deleteStr(s);
    }

    /* Add the FMTTYPE parameter to the ATTACH property if it exists. */
    if (fmttype_param) {
        if (attach_prop) {
            icalproperty_add_parameter(attach_prop, fmttype_param);
        } else {
            icalparameter_free(fmttype_param);
        }
    }

    /* Now check if the alarm is valid, i.e. it has the required properties
       according to its type. */

    /* All alarms must have a trigger. */
    if (!trigger_prop)
        is_valid_alarm = 0;

    /* If there is a Duration but not a Repeat Count, we just remove the
       Duration so the alarm only occurs once. */
    if (duration_prop && !repeat_prop) {
        icalcomponent_remove_property(comp, duration_prop);
        icalproperty_free(duration_prop);
        duration_prop = NULL;
    }

    /* Similarly if we have a Repeat Count but no Duration, we remove it. */
    if (repeat_prop && !duration_prop) {
        icalcomponent_remove_property(comp, repeat_prop);
        icalproperty_free(repeat_prop);
        repeat_prop = NULL;
    }

    switch (icaltype) {
    case ICAL_XAUDIOALARM_COMPONENT:
        action = ICAL_ACTION_AUDIO;

        /* Audio alarms must have an ATTACH property, which is a URL.
           If they don't have one, we use the default alarm URL. */
        if (!attach_prop || !value_is_url) {
            if (defaults && defaults->alarm_audio_url && defaults->alarm_audio_fmttype) {
                icalattach *attach;

                if (attach_prop) {
                    icalcomponent_remove_property(comp, attach_prop);
                    icalproperty_free(attach_prop);
                }

                attach = icalattach_new_from_url(defaults->alarm_audio_url);
                attach_prop = icalproperty_new_attach(attach);
                icalcomponent_add_property(comp, attach_prop);

                fmttype_param = icalparameter_new_fmttype(defaults->alarm_audio_fmttype);
                icalproperty_add_parameter(attach_prop, fmttype_param);
                icalattach_unref(attach);
            } else {
                is_valid_alarm = 0;
            }
        }
        break;

    case ICAL_XDISPLAYALARM_COMPONENT:
        action = ICAL_ACTION_DISPLAY;

        /* Display alarms must have a DESCRIPTION. */
        if (!description_prop) {
            if (defaults && defaults->alarm_description) {
                description_prop = icalproperty_new_description(defaults->alarm_description);
                icalcomponent_add_property(comp, description_prop);
            } else {
                is_valid_alarm = 0;
            }
        }
        break;

    case ICAL_XEMAILALARM_COMPONENT:
        action = ICAL_ACTION_EMAIL;

        /* Email alarms must have a SUMMARY, a DESCRIPTION, and an ATTENDEE. */
        if (!attendee_prop) {
            is_valid_alarm = 0;
        } else if (!summary_prop || !description_prop) {
            if (!summary_prop && defaults->alarm_description) {
                summary_prop = icalproperty_new_summary(defaults->alarm_description);
                icalcomponent_add_property(comp, summary_prop);
            }

            if (!description_prop && defaults->alarm_description) {
                description_prop = icalproperty_new_description(defaults->alarm_description);
                icalcomponent_add_property(comp, description_prop);
            }

            if (!summary_prop || !description_prop)
                is_valid_alarm = 0;
        }
        break;

    case ICAL_XPROCEDUREALARM_COMPONENT:
        action = ICAL_ACTION_PROCEDURE;

        /* Procedure alarms must have an ATTACH property, which is a URL.
           We don't support inline data. */
        if (!attach_prop) {
            is_valid_alarm = 0;
        } else if (!value_is_url) {
            icalattach *attach;
            const char *url;

            attach = icalproperty_get_attach(attach_prop);
            url = icalattach_get_url(attach);

            /* Check for Gnome Calendar, which will just save a pathname. */
            if (url && url[0] == '/') {
                size_t len;
                char *new_url;
                icalattach *new_attach;

                /* Turn it into a proper file: URL. */
                len = strlen(url) + 12;

                new_url = malloc(len);
                strcpy(new_url, "file://");
                strcat(new_url, url);

                new_attach = icalattach_new_from_url(new_url);
                free(new_url);

                icalproperty_set_attach(attach_prop, new_attach);
                icalattach_unref(attach);

            } else {
                is_valid_alarm = 0;
            }
        }
        break;

    default:
        /* Shouldn't reach here ever. */
        assert(0);
        break;
    }

    action_prop = icalproperty_new_action(action);
    icalcomponent_add_property(comp, action_prop);

    return is_valid_alarm;
}

static void *alarm_comp(int icaltype, VObject *o, icalcomponent *comp,
                        icalvcal_defaults *defaults)
{
/*    icalcomponent_kind kind = (icalcomponent_kind)icaltype; */
    int is_valid_alarm;
    icalcomponent *c = icalcomponent_new(ICAL_VALARM_COMPONENT);

    _unused(comp);

    is_valid_alarm = get_alarm_properties(c, o, icaltype, defaults);

    if (is_valid_alarm) {
        return (void *)c;
    } else {
        icalcomponent_free(c);
        return NULL;
    }
}

/* These #defines indicate conversion routines that are not defined yet. */

#define parameter NULL
#define rsvp_parameter NULL

static void *transp_prop(int icaltype, VObject *object, icalcomponent *comp,
                         icalvcal_defaults *defaults)
{
    icalproperty *prop = NULL;
    char *s;
    int free_string;

    _unused(icaltype);
    _unused(comp);
    _unused(defaults);

    s = get_string_value(object, &free_string);

    /* In vCalendar "0" means opaque, "1" means transparent, and >1 is
       implementation-specific. So we just check for "1" and output
       TRANSPARENT. For anything else, the default OPAQUE will be used. */
    if (!strcmp(s, "1")) {
        prop = icalproperty_new_transp(ICAL_TRANSP_TRANSPARENT);
    }

    if (free_string)
        deleteStr(s);

    return (void *)prop;
}

static void *sequence_prop(int icaltype, VObject *object, icalcomponent *comp,
                           icalvcal_defaults *defaults)
{
    icalproperty *prop = NULL;
    char *s;
    int free_string, sequence;

    _unused(icaltype);
    _unused(comp);
    _unused(defaults);

    s = get_string_value(object, &free_string);

    /* GnomeCalendar outputs '-1' for this. I have no idea why.
       So we just check it is a valid +ve integer, and output 0 if it isn't. */
    sequence = atoi(s);
    if (sequence < 0)
        sequence = 0;

    prop = icalproperty_new_sequence(sequence);

    if (free_string)
        deleteStr(s);

    return (void *)prop;
}

/* This handles properties which have multiple values, which are separated by
   ';' in vCalendar but ',' in iCalendar. So we just switch those. */
static void *multivalued_prop(int icaltype, VObject *object, icalcomponent *comp,
                              icalvcal_defaults *defaults)
{
    icalproperty_kind kind = (icalproperty_kind) icaltype;
    icalproperty *prop = NULL;
    icalvalue *value;
    icalvalue_kind value_kind;
    char *s, *tmp_copy, *p;
    int free_string;

    _unused(comp);
    _unused(defaults);

    s = get_string_value(object, &free_string);

    tmp_copy = strdup(s);

    if (free_string)
        deleteStr(s);

    if (tmp_copy) {
        prop = icalproperty_new(kind);

        value_kind = icalenum_property_kind_to_value_kind(icalproperty_isa(prop));

        for (p = tmp_copy; *p; p++) {
            if (*p == ';')
                *p = ',';
        }

        value = icalvalue_new_from_string(value_kind, tmp_copy);
        icalproperty_set_value(prop, value);

        free(tmp_copy);
    }

    return (void *)prop;
}

static void *status_prop(int icaltype, VObject *object, icalcomponent *comp,
                         icalvcal_defaults *defaults)
{
    icalproperty *prop = NULL;
    char *s;
    int free_string;
    icalcomponent_kind kind;

    _unused(icaltype);
    _unused(defaults);

    kind = icalcomponent_isa(comp);

    s = get_string_value(object, &free_string);

    /* In vCalendar:
       VEVENT can have: "NEEDS ACTION" (default), "SENT", "TENTATIVE",
       "CONFIRMED", "DECLINED", "DELEGATED".
       VTODO can have:  "ACCEPTED", "NEEDS ACTION" (default), "SENT",
       "DECLINED", "COMPLETED", "DELEGATED".
       (Those are the only 2 components - there is no VJOURNAL)

       In iCalendar:
       VEVENT can have: "TENTATIVE", "CONFIRMED", "CANCELLED".
       VTODO can have:  "NEEDS-ACTION", "COMPLETED", "IN-PROCESS", "CANCELLED".

       So for VEVENT if it is "TENTATIVE" or "CONFIRMED" we keep it, otherwise
       we skip it.

       For a VTODO if it is "NEEDS ACTION" we convert to "NEEDS-ACTION", if it
       is "COMPLETED" we keep it, otherwise we skip it.
     */
    if (kind == ICAL_VEVENT_COMPONENT) {
        if (!strcmp(s, "TENTATIVE")) {
            prop = icalproperty_new_status(ICAL_STATUS_TENTATIVE);
        } else if (!strcmp(s, "CONFIRMED")) {
            prop = icalproperty_new_status(ICAL_STATUS_CONFIRMED);
        }
    } else if (kind == ICAL_VTODO_COMPONENT) {
        if (!strcmp(s, "NEEDS ACTION")) {
            prop = icalproperty_new_status(ICAL_STATUS_NEEDSACTION);
        } else if (!strcmp(s, "COMPLETED")) {
            prop = icalproperty_new_status(ICAL_STATUS_COMPLETED);
        }
    }

    if (free_string)
        deleteStr(s);

    return (void *)prop;
}

static void *utc_datetime_prop(int icaltype, VObject *object, icalcomponent *comp,
                               icalvcal_defaults *defaults)
{
    icalproperty_kind kind = (icalproperty_kind) icaltype;
    icalproperty *prop;
    icalvalue *value;
    char *s;
    int free_string;
    struct icaltimetype itt;

    _unused(comp);
    _unused(defaults);

    prop = icalproperty_new(kind);

    s = get_string_value(object, &free_string);

    /* Convert it to an icaltimetype. */
    itt = icaltime_from_string(s);

    /* If it is a floating time, convert it to a UTC time. */
    if (!itt.is_utc)
        convert_floating_time_to_utc(&itt);

    value = icalvalue_new_datetime(itt);
    icalproperty_set_value(prop, value);

    if (free_string)
        deleteStr(s);

    return (void *)prop;
}

/* Parse the interval from the RRULE, returning a pointer to the first char
   after the interval and any whitespace. s points to the start of the
   interval. error_message is set if an error occurs. */
static char *rrule_parse_interval(char *s, struct icalrecurrencetype *recur, char **error_message)
{
    int interval = 0;

    /* It must start with a digit. */
    if (*s < '0' || *s > '9') {
        *error_message = "Invalid Interval";
        return NULL;
    }

    while (*s >= '0' && *s <= '9')
        interval = (interval * 10) + (*s++ - '0');

    /* It must be followed by whitespace. I'm not sure if anything else is
       allowed. */
    if (*s != ' ' && *s != '\t') {
        *error_message = "Invalid Interval";
        return NULL;
    }

    /* Skip any whitespace. */
    while (*s == ' ' || *s == '\t')
        s++;

    recur->interval = interval;
    return s;
}

/* Parse the duration from the RRULE, either a COUNT, e.g. '#5', or an UNTIL
   date, e.g. 20020124T000000. error_message is set if an error occurs.
   If no duration is given, '#2' is assumed. */
static char *rrule_parse_duration(char *s, struct icalrecurrencetype *recur, char **error_message)
{
    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    if (!s || *s == '\0') {
        /* If we are at the end of the string, assume '#2'. */
        recur->count = 2;

    } else if (*s == '#') {
        /* If it starts with a '#' it is the COUNT. Note that in vCalendar
           #0 means forever, and setting recur->count to 0 means the same. */
        int count = 0;

        s++;
        while (*s >= '0' && *s <= '9')
            count = (count * 10) + (*s++ - '0');

        recur->count = count;

    } else if (*s >= '0' && *s <= '9') {
        /* If it starts with a digit it must be the UNTIL date. */
        char *e, buffer[20];
        ptrdiff_t len;

        /* Find the end of the date. */
        e = s;
        while ((*e >= '0' && *e <= '9') || *e == 'T' || *e == 'Z')
            e++;

        /* Check it is a suitable length. */
        len = (ptrdiff_t) (e - s);
        if (len != 8 && len != 15 && len != 16) {
            *error_message = "Invalid End Date";
            return NULL;
        }

        /* Copy the date to our buffer and null-terminate it. */
        strncpy(buffer, s, len);
        buffer[len] = '\0';

        /* Parse it into the until field. */
        recur->until = icaltime_from_string(buffer);

        /* In iCalendar UNTIL must be UTC if it is a DATE-TIME. But we
           don't really know what timezone the vCalendar times are in. So if
           it can be converted to a DATE value, we do that. Otherwise we just
           use the current Unix timezone. Should be OK 99% of the time. */
        if (!recur->until.is_utc) {
            if (recur->until.hour == 0 &&
                recur->until.minute == 0 &&
                recur->until.second == 0) {
                recur->until.is_date = 1;
            } else {
                convert_floating_time_to_utc(&recur->until);
            }
        }

        s = e;

    } else {
        *error_message = "Invalid Duration";
        return NULL;
    }

    /* It must be followed by whitespace or the end of the string.
       I'm not sure if anything else is allowed. */
    if (s && *s != '\0' && *s != ' ' && *s != '\t') {
        *error_message = "Invalid Duration";
        return NULL;
    }

    return s;
}

static char *rrule_parse_weekly_days(char *s,
                                     struct icalrecurrencetype *recur, char **error_message)
{
    int i;

    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    for (i = 0; i < ICAL_BY_DAY_SIZE; i++) {
        char *e = s;
        int found_day, day;

        found_day = -1;
        for (day = 0; day < 7; day++) {
            if (!strncmp(weekdays[day], s, 2)) {
                /* Check the next char is whitespace or the end of string. */
                e = s + 2;
                if (*e == ' ' || *e == '\t' || *e == '\0') {
                    found_day = day;
                    break;
                }
            }
        }

        if (found_day == -1)
            break;

        /* cppcheck-suppress arrayIndexOutOfBounds since 'day' can't be >6 */
        recur->by_day[i] = weekday_codes[day];

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }

    /* Terminate the array, if it isn't full. */
    if (i < ICAL_BY_DAY_SIZE)
        recur->by_day[i] = ICAL_RECURRENCE_ARRAY_MAX;

    return s;
}

static char *rrule_parse_monthly_days(char *s,
                                      struct icalrecurrencetype *recur, char **error_message)
{
    int i;

    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    for (i = 0; i < ICAL_BY_MONTHDAY_SIZE; i++) {
        char *e;
        int month_day;

        if (!strncmp(s, "LD", 2)) {
            month_day = -1;
            e = s + 2;
        } else {
            month_day = strtol(s, &e, 10);

            /* Check we got a valid day. */
            if (month_day < 1 || month_day > 31)
                break;

            /* See if it is followed by a '+' or '-'. */
            if (*e == '+') {
                e++;
            } else if (*e == '-') {
                e++;
                month_day = -month_day;
            }
        }

        /* Check the next char is whitespace or the end of the string. */
        if (*e != ' ' && *e != '\t' && *e != '\0')
            break;

        recur->by_month_day[i] = month_day;

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }

    /* Terminate the array, if it isn't full. */
    if (i < ICAL_BY_MONTHDAY_SIZE)
        recur->by_month_day[i] = ICAL_RECURRENCE_ARRAY_MAX;

    return s;
}

static char *rrule_parse_monthly_positions(char *s,
                                           struct icalrecurrencetype *recur, char **error_message)
{
    int occurrences[ICAL_BY_DAY_SIZE];
    int found_weekdays[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int i, num_positions, elems, month_position, day;
    int num_weekdays, only_weekday = 0;
    char *e;

    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    /* First read the month position into our local occurrences array. */
    for (i = 0; i < ICAL_BY_DAY_SIZE; i++) {
        int month_position;

        /* Check we got a valid position number. */
        month_position = *s - '0';
        if (month_position < 0 || month_position > 5)
            break;

        /* See if it is followed by a '+' or '-'. */
        e = s + 1;
        if (*e == '+') {
            e++;
        } else if (*e == '-') {
            e++;
            month_position = -month_position;
        }

        /* Check the next char is whitespace or the end of the string. */
        if (*e != ' ' && *e != '\t' && *e != '\0')
            break;

        occurrences[i] = month_position;

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }
    num_positions = i;

    /* Now read the weekdays in. */
    for (;;) {
        char *e = s;
        int found_day, day;

        found_day = -1;
        for (day = 0; day < 7; day++) {
            if (!strncmp(weekdays[day], s, 2)) {
                /* Check the next char is whitespace or the end of string. */
                e = s + 2;
                if (*e == ' ' || *e == '\t' || *e == '\0') {
                    found_day = day;
                    break;
                }
            }
        }

        if (found_day == -1)
            break;

        found_weekdays[found_day] = 1;

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }

    /* Now merge them together into the recur->by_day array. If there is a
       single position & weekday we output something like
       'BYDAY=TU;BYSETPOS=2', so Outlook will understand it. */
    num_weekdays = 0;
    for (day = 0; day < 7; day++) {
        if (found_weekdays[day]) {
            num_weekdays++;
            only_weekday = day;
        }
    }
    if (num_positions == 1 && num_weekdays == 1) {
        recur->by_day[0] = weekday_codes[only_weekday];
        recur->by_day[1] = ICAL_RECURRENCE_ARRAY_MAX;

        recur->by_set_pos[0] = occurrences[0];
        recur->by_set_pos[1] = ICAL_RECURRENCE_ARRAY_MAX;
    } else {
        elems = 0;
        for (i = 0; i < num_positions; i++) {
            month_position = occurrences[i];

            for (day = 0; day < 7; day++) {
                if (found_weekdays[day]) {
                    recur->by_day[elems] =
                        (abs(month_position) * 8 +
                         weekday_codes[day]) * ((month_position < 0) ? -1 : 1);
                    elems++;
                    if (elems == ICAL_BY_DAY_SIZE)
                        break;
                }
            }

            if (elems == ICAL_BY_DAY_SIZE)
                break;
        }

        /* Terminate the array, if it isn't full. */
        if (elems < ICAL_BY_DAY_SIZE)
            recur->by_day[elems] = ICAL_RECURRENCE_ARRAY_MAX;
    }

    return s;
}

static char *rrule_parse_yearly_months(char *s,
                                       struct icalrecurrencetype *recur, char **error_message)
{
    int i;

    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    for (i = 0; i < ICAL_BY_MONTH_SIZE; i++) {
        char *e;
        int month;

        month = strtol(s, &e, 10);

        /* Check we got a valid month. */
        if (month < 1 || month > 12)
            break;

        /* Check the next char is whitespace or the end of the string. */
        if (*e != ' ' && *e != '\t' && *e != '\0')
            break;

        recur->by_month[i] = month;

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }

    /* Terminate the array, if it isn't full. */
    if (i < ICAL_BY_MONTH_SIZE)
        recur->by_month[i] = ICAL_RECURRENCE_ARRAY_MAX;

    return s;
}

static char *rrule_parse_yearly_days(char *s,
                                     struct icalrecurrencetype *recur, char **error_message)
{
    int i;

    /* If we've already found an error, just return. */
    if (*error_message)
        return NULL;

    for (i = 0; i < ICAL_BY_YEARDAY_SIZE; i++) {
        char *e;
        int year_day;

        year_day = strtol(s, &e, 10);

        /* Check we got a valid year_day. */
        if (year_day < 1 || year_day > 366)
            break;

        /* Check the next char is whitespace or the end of the string. */
        if (*e != ' ' && *e != '\t' && *e != '\0')
            break;

        recur->by_year_day[i] = year_day;

        s = e;
        /* Skip any whitespace. */
        while (*s == ' ' || *s == '\t')
            s++;
    }

    /* Terminate the array, if it isn't full. */
    if (i < ICAL_BY_YEARDAY_SIZE)
        recur->by_year_day[i] = ICAL_RECURRENCE_ARRAY_MAX;

    return s;
}

/* Converts an RRULE/EXRULE property.
   NOTE: There are a few things that this doesn't handle:
     1) vCalendar RRULE properties can contain an UNTIL date and a COUNT, and
        the first to occur specifies the end of the recurrence. However they
        are mutually exclusive in iCalendar. For now we just use the COUNT.
     2) For MONTHLY By Position recurrences, if no modifiers are given they
        are to be calculated based on the DTSTART, e.g. if DTSTART is on the
        3rd Wednesday of the month then all occurrences are on the 3rd Wed.
        This is awkward to do as we need to access the DTSTART property, which
        may be after the RRULE property. So we don't do this at present.
     3) The Extended Recurrence Rule Grammar - we only support the Basic rules.
        The extended grammar supports rules embedded in other rules, MINUTELY
        recurrences, time modifiers in DAILY rules and maybe other stuff.
*/

static void *rule_prop(int icaltype, VObject *object, icalcomponent *comp,
                       icalvcal_defaults *defaults)
{
    icalproperty *prop = NULL;
    char *s, *p, *parsestat, *error_message;
    const char *property_name;
    int free_string;
    struct icalrecurrencetype recur;

    _unused(icaltype);
    _unused(comp);
    _unused(defaults);

    s = get_string_value(object, &free_string);

    property_name = vObjectName(object);

    icalrecurrencetype_clear(&recur);

    error_message = NULL;
    parsestat = NULL;
    if (*s == 'D') {
        /* The DAILY RRULE only has an interval and duration (COUNT/UNTIL). */
        recur.freq = ICAL_DAILY_RECURRENCE;
        p = rrule_parse_interval(s + 1, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    } else if (*s == 'W') {
        /* The WEEKLY RRULE has weekday modifiers - MO TU WE. */
        recur.freq = ICAL_WEEKLY_RECURRENCE;
        p = rrule_parse_interval(s + 1, &recur, &error_message);
        p = rrule_parse_weekly_days(p, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    } else if (*s == 'M' && *(s + 1) == 'D') {
        /* The MONTHLY By Day RRULE has day number modifiers - 1 1- LD. */
        recur.freq = ICAL_MONTHLY_RECURRENCE;
        p = rrule_parse_interval(s + 2, &recur, &error_message);
        p = rrule_parse_monthly_days(p, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    } else if (*s == 'M' && *(s + 1) == 'P') {
        /* The MONTHLY By Position RRULE has position modifiers - 1 2- and
           weekday modifiers - MO TU. */
        recur.freq = ICAL_MONTHLY_RECURRENCE;
        p = rrule_parse_interval(s + 2, &recur, &error_message);
        p = rrule_parse_monthly_positions(p, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    } else if (*s == 'Y' && *(s + 1) == 'M') {
        /* The YEARLY By Month RRULE has month modifiers - 1 3 12. */
        recur.freq = ICAL_YEARLY_RECURRENCE;
        p = rrule_parse_interval(s + 2, &recur, &error_message);
        p = rrule_parse_yearly_months(p, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    } else if (*s == 'Y' && *(s + 1) == 'D') {
        /* The YEARLY By Day RRULE has day number modifiers - 100 200. */
        recur.freq = ICAL_YEARLY_RECURRENCE;
        p = rrule_parse_interval(s + 2, &recur, &error_message);
        p = rrule_parse_yearly_days(p, &recur, &error_message);
        parsestat = rrule_parse_duration(p, &recur, &error_message);
    }

    if (!parsestat) {
        if (!error_message) {
            error_message = "Invalid RRULE Frequency";
        }
        prop = create_parse_error_property(error_message, property_name, s);
    } else {
        if (!strcmp(property_name, "RRULE")) {
            prop = icalproperty_new_rrule(recur);
        } else {
            prop = icalproperty_new_exrule(recur);
        }
    }

    if (free_string)
        deleteStr(s);

    return (void *)prop;
}

/* directly convertable property. The string representation of vcal is
   the same as ical */

void *dc_prop(int icaltype, VObject *object, icalcomponent *comp, icalvcal_defaults *defaults)
{
    icalproperty_kind kind = (icalproperty_kind) icaltype;
    icalproperty *prop;
    icalvalue *value;
    icalvalue_kind value_kind;
    char *s;

/*/,*t=0; */
    int free_string;

    _unused(comp);
    _unused(defaults);

    prop = icalproperty_new(kind);

    value_kind = icalenum_property_kind_to_value_kind(icalproperty_isa(prop));

    s = get_string_value(object, &free_string);

    value = icalvalue_new_from_string(value_kind, s);

    if (free_string)
        deleteStr(s);

    icalproperty_set_value(prop, value);

    return (void *)prop;
}

/* My extraction program screwed up, so this table does not have all
of the vcal properties in it. I didn't feel like re-doing the entire
table, so you'll have to find the missing properties the hard way --
the code will assert */

static const struct conversion_table_struct conversion_table[] = {
    {VCCalProp, DT_COMPONENT, comp, ICAL_VCALENDAR_COMPONENT},
    {VCTodoProp, DT_COMPONENT, comp, ICAL_VTODO_COMPONENT},
    {VCEventProp, DT_COMPONENT, comp, ICAL_VEVENT_COMPONENT},
    {VCAAlarmProp, DT_COMPONENT, alarm_comp, ICAL_XAUDIOALARM_COMPONENT},
    {VCDAlarmProp, DT_COMPONENT, alarm_comp, ICAL_XDISPLAYALARM_COMPONENT},
    {VCMAlarmProp, DT_COMPONENT, alarm_comp, ICAL_XEMAILALARM_COMPONENT},
    {VCPAlarmProp, DT_COMPONENT, alarm_comp, ICAL_XPROCEDUREALARM_COMPONENT},

/* These can all be converted directly by parsing the string into a libical
   value. */
    {VCClassProp, DT_PROPERTY, dc_prop, ICAL_CLASS_PROPERTY},
    {VCDescriptionProp, DT_PROPERTY, dc_prop, ICAL_DESCRIPTION_PROPERTY},
    {VCAttendeeProp, DT_PROPERTY, dc_prop, ICAL_ATTENDEE_PROPERTY},
    {VCDTendProp, DT_PROPERTY, dc_prop, ICAL_DTEND_PROPERTY},
    {VCDTstartProp, DT_PROPERTY, dc_prop, ICAL_DTSTART_PROPERTY},
    {VCDueProp, DT_PROPERTY, dc_prop, ICAL_DUE_PROPERTY},
    {VCLocationProp, DT_PROPERTY, dc_prop, ICAL_LOCATION_PROPERTY},
    {VCSummaryProp, DT_PROPERTY, dc_prop, ICAL_SUMMARY_PROPERTY},
    {VCUniqueStringProp, DT_PROPERTY, dc_prop, ICAL_UID_PROPERTY},
    {VCURLProp, DT_PROPERTY, dc_prop, ICAL_URL_PROPERTY},
    {VCPriorityProp, DT_PROPERTY, dc_prop, ICAL_PRIORITY_PROPERTY},

/* These can contain multiple values, which are separated in ';' in vCalendar
   but ',' in iCalendar. */
    {VCCategoriesProp, DT_PROPERTY, multivalued_prop, ICAL_CATEGORIES_PROPERTY},
    {VCRDateProp, DT_PROPERTY, multivalued_prop, ICAL_RDATE_PROPERTY},
    {VCExpDateProp, DT_PROPERTY, multivalued_prop, ICAL_EXDATE_PROPERTY},

/* These can be in floating time in vCalendar, but must be in UTC in iCalendar.
 */
    {VCDCreatedProp, DT_PROPERTY, utc_datetime_prop, ICAL_CREATED_PROPERTY},
    {VCLastModifiedProp, DT_PROPERTY, utc_datetime_prop, ICAL_LASTMODIFIED_PROPERTY},
    {VCCompletedProp, DT_PROPERTY, utc_datetime_prop, ICAL_COMPLETED_PROPERTY},

    {VCTranspProp, DT_PROPERTY, transp_prop, ICAL_TRANSP_PROPERTY},
    {VCSequenceProp, DT_PROPERTY, sequence_prop, ICAL_SEQUENCE_PROPERTY},
    {VCStatusProp, DT_PROPERTY, status_prop, ICAL_STATUS_PROPERTY},
    {VCRRuleProp, DT_PROPERTY, rule_prop, ICAL_RRULE_PROPERTY},
    {VCXRuleProp, DT_PROPERTY, rule_prop, ICAL_EXRULE_PROPERTY},

    {VCRSVPProp, DT_UNSUPPORTED, rsvp_parameter, ICAL_RSVP_PARAMETER},
    {VCEncodingProp, DT_UNSUPPORTED, parameter, ICAL_ENCODING_PARAMETER},
    {VCRoleProp, DT_UNSUPPORTED, parameter, ICAL_ROLE_PARAMETER},

/* We don't want the old VERSION or PRODID properties copied across as they
   are now incorrect. New VERSION & PRODID properties are added instead. */
    {VCVersionProp, DT_IGNORE, NULL, 0},
    {VCProdIdProp, DT_IGNORE, NULL, 0},

/* We ignore DAYLIGHT and TZ properties of the toplevel object, since we can't
   really do much with them. */
    {VCDayLightProp, DT_IGNORE, NULL, 0},
    {VCTimeZoneProp, DT_IGNORE, NULL, 0},

/* These are all alarm properties. We handle these when the alarm component
   is created, so we ignore them when doing the automatic conversions.
   "TYPE" is used in AALARM, but doesn't seem to have a name in vobject.h. */
    {"TYPE", DT_IGNORE, NULL, 0},
    {VCRunTimeProp, DT_IGNORE, NULL, 0},
    {VCSnoozeTimeProp, DT_IGNORE, NULL, 0},
    {VCRepeatCountProp, DT_IGNORE, NULL, 0},
    {VCValueProp, DT_IGNORE, NULL, 0},
    {VCProcedureNameProp, DT_IGNORE, NULL, 0},
    {VCDisplayStringProp, DT_IGNORE, NULL, 0},
    {VCEmailAddressProp, DT_IGNORE, NULL, 0},
    {VCNoteProp, DT_IGNORE, NULL, 0},

    {VCQuotedPrintableProp, DT_UNSUPPORTED, NULL, 0},
    {VC7bitProp, DT_UNSUPPORTED, NULL, 0},
    {VC8bitProp, DT_UNSUPPORTED, NULL, 0},
    {VCAdditionalNamesProp, DT_UNSUPPORTED, NULL, 0},
    {VCAdrProp, DT_UNSUPPORTED, NULL, 0},
    {VCAgentProp, DT_UNSUPPORTED, NULL, 0},
    {VCAIFFProp, DT_UNSUPPORTED, NULL, 0},
    {VCAOLProp, DT_UNSUPPORTED, NULL, 0},
    {VCAppleLinkProp, DT_UNSUPPORTED, NULL, 0},
    {VCAttachProp, DT_UNSUPPORTED, NULL, 0},
    {VCATTMailProp, DT_UNSUPPORTED, NULL, 0},
    {VCAudioContentProp, DT_UNSUPPORTED, NULL, 0},
    {VCAVIProp, DT_UNSUPPORTED, NULL, 0},
    {VCBase64Prop, DT_UNSUPPORTED, NULL, 0},
    {VCBBSProp, DT_UNSUPPORTED, NULL, 0},
    {VCBirthDateProp, DT_UNSUPPORTED, NULL, 0},
    {VCBMPProp, DT_UNSUPPORTED, NULL, 0},
    {VCBodyProp, DT_UNSUPPORTED, NULL, 0},
    {VCCaptionProp, DT_UNSUPPORTED, NULL, 0},
    {VCCarProp, DT_UNSUPPORTED, NULL, 0},
    {VCCellularProp, DT_UNSUPPORTED, NULL, 0},
    {VCCGMProp, DT_UNSUPPORTED, NULL, 0},
    {VCCharSetProp, DT_UNSUPPORTED, NULL, 0},
    {VCCIDProp, DT_UNSUPPORTED, NULL, 0},
    {VCCISProp, DT_UNSUPPORTED, NULL, 0},
    {VCCityProp, DT_UNSUPPORTED, NULL, 0},
    {VCCommentProp, DT_UNSUPPORTED, NULL, 0},
    {VCCountryNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCDataSizeProp, DT_UNSUPPORTED, NULL, 0},
    {VCDeliveryLabelProp, DT_UNSUPPORTED, NULL, 0},
    {VCDIBProp, DT_UNSUPPORTED, NULL, 0},
    {VCDomesticProp, DT_UNSUPPORTED, NULL, 0},
    {VCEndProp, DT_UNSUPPORTED, NULL, 0},
    {VCEWorldProp, DT_UNSUPPORTED, NULL, 0},
    {VCExNumProp, DT_UNSUPPORTED, NULL, 0},
    {VCExpectProp, DT_UNSUPPORTED, NULL, 0},
    {VCFamilyNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCFaxProp, DT_UNSUPPORTED, NULL, 0},
    {VCFullNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCGeoProp, DT_UNSUPPORTED, NULL, 0},
    {VCGeoLocationProp, DT_UNSUPPORTED, NULL, 0},
    {VCGIFProp, DT_UNSUPPORTED, NULL, 0},
    {VCGivenNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCGroupingProp, DT_UNSUPPORTED, NULL, 0},
    {VCHomeProp, DT_UNSUPPORTED, NULL, 0},
    {VCIBMMailProp, DT_UNSUPPORTED, NULL, 0},
    {VCInlineProp, DT_UNSUPPORTED, NULL, 0},
    {VCInternationalProp, DT_UNSUPPORTED, NULL, 0},
    {VCInternetProp, DT_UNSUPPORTED, NULL, 0},
    {VCISDNProp, DT_UNSUPPORTED, NULL, 0},
    {VCJPEGProp, DT_UNSUPPORTED, NULL, 0},
    {VCLanguageProp, DT_UNSUPPORTED, NULL, 0},
    {VCLastRevisedProp, DT_UNSUPPORTED, NULL, 0},
    {VCLogoProp, DT_UNSUPPORTED, NULL, 0},
    {VCMailerProp, DT_UNSUPPORTED, NULL, 0},
    {VCMCIMailProp, DT_UNSUPPORTED, NULL, 0},
    {VCMessageProp, DT_UNSUPPORTED, NULL, 0},
    {VCMETProp, DT_UNSUPPORTED, NULL, 0},
    {VCModemProp, DT_UNSUPPORTED, NULL, 0},
    {VCMPEG2Prop, DT_UNSUPPORTED, NULL, 0},
    {VCMPEGProp, DT_UNSUPPORTED, NULL, 0},
    {VCMSNProp, DT_UNSUPPORTED, NULL, 0},
    {VCNamePrefixesProp, DT_UNSUPPORTED, NULL, 0},
    {VCNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCNameSuffixesProp, DT_UNSUPPORTED, NULL, 0},
    {VCOrgNameProp, DT_UNSUPPORTED, NULL, 0},
    {VCOrgProp, DT_UNSUPPORTED, NULL, 0},
    {VCOrgUnit2Prop, DT_UNSUPPORTED, NULL, 0},
    {VCOrgUnit3Prop, DT_UNSUPPORTED, NULL, 0},
    {VCOrgUnit4Prop, DT_UNSUPPORTED, NULL, 0},
    {VCOrgUnitProp, DT_UNSUPPORTED, NULL, 0},
    {VCPagerProp, DT_UNSUPPORTED, NULL, 0},
    {VCParcelProp, DT_UNSUPPORTED, NULL, 0},
    {VCPartProp, DT_UNSUPPORTED, NULL, 0},
    {VCPCMProp, DT_UNSUPPORTED, NULL, 0},
    {VCPDFProp, DT_UNSUPPORTED, NULL, 0},
    {VCPGPProp, DT_UNSUPPORTED, NULL, 0},
    {VCPhotoProp, DT_UNSUPPORTED, NULL, 0},
    {VCPICTProp, DT_UNSUPPORTED, NULL, 0},
    {VCPMBProp, DT_UNSUPPORTED, NULL, 0},
    {VCPostalBoxProp, DT_UNSUPPORTED, NULL, 0},
    {VCPostalCodeProp, DT_UNSUPPORTED, NULL, 0},
    {VCPostalProp, DT_UNSUPPORTED, NULL, 0},
    {VCPowerShareProp, DT_UNSUPPORTED, NULL, 0},
    {VCPreferredProp, DT_UNSUPPORTED, NULL, 0},
    {VCProdigyProp, DT_UNSUPPORTED, NULL, 0},
    {VCPronunciationProp, DT_UNSUPPORTED, NULL, 0},
    {VCPSProp, DT_UNSUPPORTED, NULL, 0},
    {VCPublicKeyProp, DT_UNSUPPORTED, NULL, 0},
    {VCQPProp, DT_UNSUPPORTED, NULL, 0},
    {VCQuickTimeProp, DT_UNSUPPORTED, NULL, 0},
    {VCRegionProp, DT_UNSUPPORTED, NULL, 0},
    {VCResourcesProp, DT_UNSUPPORTED, NULL, 0},
    {VCRNumProp, DT_UNSUPPORTED, NULL, 0},
    {VCStartProp, DT_UNSUPPORTED, NULL, 0},
    {VCStreetAddressProp, DT_UNSUPPORTED, NULL, 0},
    {VCSubTypeProp, DT_UNSUPPORTED, NULL, 0},
    {VCTelephoneProp, DT_UNSUPPORTED, NULL, 0},
    {VCTIFFProp, DT_UNSUPPORTED, NULL, 0},
    {VCTitleProp, DT_UNSUPPORTED, NULL, 0},
    {VCTLXProp, DT_UNSUPPORTED, NULL, 0},
    {VCURLValueProp, DT_UNSUPPORTED, NULL, 0},
    {VCVideoProp, DT_UNSUPPORTED, NULL, 0},
    {VCVoiceProp, DT_UNSUPPORTED, NULL, 0},
    {VCWAVEProp, DT_UNSUPPORTED, NULL, 0},
    {VCWMFProp, DT_UNSUPPORTED, NULL, 0},
    {VCWorkProp, DT_UNSUPPORTED, NULL, 0},
    {VCX400Prop, DT_UNSUPPORTED, NULL, 0},
    {VCX509Prop, DT_UNSUPPORTED, NULL, 0},
    {0, 0, NULL, 0}
};

static void icalvcal_traverse_objects(VObject *object,
                                      icalcomponent *last_comp,
                                      icalproperty *last_prop, icalvcal_defaults *defaults)
{
    VObjectIterator iterator;
    char *name = "[No Name]";
    icalcomponent *subc = 0;
    int i;

    if (vObjectName(object) == 0) {
        printf("ERROR, object has no name");
        assert(0);
        return;
    }

    name = (char *)vObjectName(object);

    /* Lookup this object in the conversion table */
    for (i = 0; conversion_table[i].vcalname != 0; i++) {
        if (strcmp(conversion_table[i].vcalname, name) == 0) {
            break;
        }
    }

    /* Did not find the object. It may be an X-property, or an unknown
       property */
    if (conversion_table[i].vcalname == 0) {

        /* Handle X properties */
        if (strncmp(name, "X-", 2) == 0) {
            icalproperty *prop = (icalproperty *) dc_prop(ICAL_X_PROPERTY, object,
                                                          last_comp, defaults);

            icalproperty_set_x_name(prop, name);
            icalcomponent_add_property(last_comp, prop);
        } else {
            return;
        }

    } else {

        /* The vCal property is in the table, and it is not an X
           property, so try to convert it to an iCal component,
           property or parameter. */

        switch (conversion_table[i].type) {

        case DT_COMPONENT:
        {
            subc =
                (icalcomponent *)(conversion_table[i].conversion_func(
                    conversion_table[i].icaltype, object, last_comp, defaults));

            if (subc) {
                icalcomponent_add_component(last_comp, subc);
            }
            break;
        }

        case DT_PROPERTY:
        {
            if (vObjectValueType(object) && conversion_table[i].conversion_func != NULL) {

              icalproperty *prop =
                  (icalproperty *)(conversion_table[i].conversion_func(
                      conversion_table[i].icaltype, object, last_comp, defaults));

              if (prop)
                  icalcomponent_add_property(last_comp, prop);

              last_prop = prop;
            }
            break;
        }

        case DT_PARAMETER:
        {
            break;
        }

        case DT_UNSUPPORTED:
        {
            /* If the property is listed as DT_UNSUPPORTED, insert a
               X_LIC_ERROR property to note this fact. */

            char temp[1024];
            char *message = "Unsupported vCal property";
            icalparameter *error_param;
            icalproperty *error_prop;

            snprintf(temp, 1024, "%s: %s", message, name);

            error_param = icalparameter_new_xlicerrortype(ICAL_XLICERRORTYPE_UNKNOWNVCALPROPERROR);

            error_prop = icalproperty_new_xlicerror(temp);
            icalproperty_add_parameter(error_prop, error_param);

            icalcomponent_add_property(last_comp, error_prop);

            break;
        }

        case DT_IGNORE:
        {
            /* Do Nothing. */
            break;
        }
        }
    }

    /* Now, step down into the next vCalproperty */

    initPropIterator(&iterator, object);
    while (moreIteration(&iterator)) {
        VObject *eachProp = nextVObject(&iterator);

        /* If 'object' is a component, then the next traversal down
           should use it as the 'last_comp' */

        if (subc != 0) {
            icalvcal_traverse_objects(eachProp, subc, last_prop, defaults);

        } else {
            icalvcal_traverse_objects(eachProp, last_comp, last_prop, defaults);
        }
    }
}

#if 0
switch (vObjectValueType(object)) {
case VCVT_USTRINGZ:
{
    char c;
    char *t, *s;

    s = t = fakeCString(vObjectUStringZValue(object));
    printf(" ustringzstring:%s\n", s);
    deleteStr(s);
    break;
}
case VCVT_STRINGZ:
{
    char c;
    const char *s = vObjectStringZValue(object);

    printf(" stringzstring:%s\n", s);
    break;
}
case VCVT_UINT:
{
    int i = vObjectIntegerValue(object);

    printf(" int:%d\n", i);
    break;
}
case VCVT_ULONG:
{
    long l = vObjectLongValue(object);

    printf(" int:%d\n", l);
    break;
}
case VCVT_VOBJECT:
{
    printf("ERROR, should not get here\n");
    break;
}
case VCVT_RAW:
case 0:
default:
    break;
}

#endif
