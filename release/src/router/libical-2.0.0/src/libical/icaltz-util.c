/*
 * Authors :
 *  Chenthill Palanisamy <pchenthill@novell.com>
 *
 * Copyright 2007, Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icaltz-util.h"
#include "icalerror.h"
#include "icaltimezone.h"

#include <stdlib.h>

#if defined(sun) && defined(__SVR4)
#include <sys/types.h>
#include <sys/byteorder.h>
#else
#if defined(HAVE_BYTESWAP_H)
#include <byteswap.h>
#endif
#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#else
#if defined(HAVE_SYS_ENDIAN_H)
#include <sys/endian.h>
#if defined(bswap32)
#define bswap_32 bswap32
#else
#define bswap_32 swap32
#endif
#endif
#endif
#endif

#if defined(__OpenBSD__) && !defined(bswap_32)
#define bswap_32 swap32
#endif

#if defined(_MSC_VER)
#if !defined(HAVE_BYTESWAP_H) && !defined(HAVE_SYS_ENDIAN_H) && !defined(HAVE_ENDIAN_H)
#define bswap_16(x) (((x) << 8) & 0xff00) | (((x) >> 8) & 0xff)

#define bswap_32(x) \
(((x) << 24) & 0xff000000) | \
(((x) << 8) & 0xff0000)    | \
(((x) >> 8) & 0xff00)      | \
(((x) >> 24) & 0xff)

#define bswap_64(x) \
((((x) & 0xff00000000000000ull) >> 56) | \
(((x) & 0x00ff000000000000ull) >> 40)  | \
(((x) & 0x0000ff0000000000ull) >> 24)  | \
(((x) & 0x000000ff00000000ull) >> 8)   | \
(((x) & 0x00000000ff000000ull) << 8)   | \
(((x) & 0x0000000000ff0000ull) << 24)  | \
(((x) & 0x000000000000ff00ull) << 40)  | \
(((x) & 0x00000000000000ffull) << 56))
#endif
#include <io.h>
#endif

#if defined(__APPLE__) || defined(__MINGW32__)
#define bswap_16(x) (((x) << 8) & 0xff00) | (((x) >> 8) & 0xff)
#define bswap_32 __builtin_bswap32
#define bswap_64 __builtin_bswap64
#endif

typedef struct
{
    char ttisgmtcnt[4];
    char ttisstdcnt[4];
    char leapcnt[4];
    char timecnt[4];
    char typecnt[4];
    char charcnt[4];
} tzinfo;

static char *zdir = NULL;

static char *search_paths[] = {
    "/usr/share/zoneinfo",
    "/usr/lib/zoneinfo",
    "/etc/zoneinfo",
    "/usr/share/lib/zoneinfo"
};

#define EFREAD(buf,size,num,fs) \
if (fread(buf, size, num, fs) < num  && ferror(fs)) {  \
    icalerror_set_errno(ICAL_FILE_ERROR);              \
    goto error;                                        \
}

typedef struct
{
    long int gmtoff;
    unsigned char isdst;
    unsigned int abbr;
    unsigned char isstd;
    unsigned char isgmt;
    char *zname;

} ttinfo;

typedef struct
{
    time_t transition;
    long int change;
} leap;

static int decode(const void *ptr)
{
#if defined(sun) && defined(__SVR4)
    if (sizeof(int) == 4) {
#if defined(_BIG_ENDIAN)
        return *(const int *)ptr;
#else
        return BSWAP_32(*(const int *)ptr);
#endif
#else
    if ((BYTE_ORDER == BIG_ENDIAN) && sizeof(int) == 4) {
        return *(const int *)ptr;
    } else if (BYTE_ORDER == LITTLE_ENDIAN && sizeof(int) == 4) {
        return (int)bswap_32(*(const unsigned int *)ptr);
#endif
    } else {
        const unsigned char *p = ptr;
        int result = *p & (1 << (CHAR_BIT - 1)) ? ~0 : 0;

        result = (result << 8) | *p++;
        result = (result << 8) | *p++;
        result = (result << 8) | *p++;
        result = (result << 8) | *p++;

        return result;
    }
}

static char *zname_from_stridx(char *str, long idx)
{
    long i;
    size_t size;
    char *ret;

    i = idx;
    while (str[i] != '\0') {
        i++;
    }

    size = (size_t)(i - idx);
    str += idx;
    ret = (char *)malloc(size + 1);
    ret = strncpy(ret, str, size);
    ret[size] = '\0';

    return ret;
}

static void set_zonedir(void)
{
    char file_path[MAXPATHLEN];
    const char *fname = ZONES_TAB_SYSTEM_FILENAME;
    size_t i, num_search_paths;

    num_search_paths = sizeof(search_paths) / sizeof(search_paths[0]);
    for (i = 0; i < num_search_paths; i++) {
        snprintf(file_path, MAXPATHLEN, "%s/%s", search_paths[i], fname);
        if (!access(file_path, F_OK | R_OK)) {
            zdir = search_paths[i];
            break;
        }
    }
}

const char *icaltzutil_get_zone_directory(void)
{
    if (!zdir)
        set_zonedir();

    return zdir;
}

static void find_transidx(time_t *transitions, ttinfo *types,
                          int *trans_idx, long int num_trans,
                          int *stdidx, int *dstidx)
{
    time_t now, year_start;
    int i, found = 0;
    struct icaltimetype itime;

    now = time(NULL);
    itime = icaltime_from_timet(now, 0);
    itime.month = itime.day = 1;
    itime.hour = itime.minute = itime.second = 0;
    year_start = icaltime_as_timet(itime);

    /* Set this by default */
    *stdidx = (num_trans - 1);

    for (i = (num_trans - 1); i >= 0; --i) {
        if (year_start < transitions[i]) {
          int idx;
          found = 1;
          idx = trans_idx[i];
          (types[idx].isdst) ? (*dstidx = i) : (*stdidx = i);
        }
    }

    /* If the transition found is the last among the list, prepare to use the last two transtions.
     * Using this will most likely throw the DTSTART of the resulting component off by 1 or 2 days
     * but it would set right by the adjustment made.
     * NOTE: We need to use the last two transitions only because there is no data for the future
     * transitions.
     */
    if (found && (*dstidx == -1)) {
        *dstidx = ((*stdidx) - 1);
    }

    return;
}

static int calculate_pos(icaltimetype icaltime)
{
   static int r_pos[] = {1, 2, 3, -2, -1};
   int pos;

   pos = (icaltime.day - 1) / 7;

   /* Check if pos 3 is the last occurrence of the week day in the month */
   if (pos == 3 && ((icaltime.day + 7) > icaltime_days_in_month(icaltime.month, icaltime.year))) {
       pos = 4;
   }

   return r_pos[pos];
}

#if defined(USE_INTEROPERABLE_VTIMEZONES)
static int _s_use_exact_timezones = 0;
#else
static int _s_use_exact_timezones = 1;
#endif

void icaltzutil_set_exact_vtimezones_support(int on)
{
    _s_use_exact_timezones = (on != 0);
}

int icaltzutil_get_exact_vtimezones_support(void)
{
    return _s_use_exact_timezones;
}

static void adjust_dtstart_day_to_rrule(icalcomponent *comp, struct icalrecurrencetype rule)
{
    time_t now, year_start;
    struct icaltimetype start, comp_start, iter_start, itime;
    icalrecur_iterator *iter;

    now = time(NULL);
    itime = icaltime_from_timet(now, 0);
    itime.month = itime.day = 1;
    itime.hour = itime.minute = itime.second = 0;
    year_start = icaltime_as_timet(itime);

    comp_start = icalcomponent_get_dtstart(comp);
    start = icaltime_from_timet(year_start, 0);

    iter = icalrecur_iterator_new(rule, start);
    iter_start = icalrecur_iterator_next(iter);
    icalrecur_iterator_free(iter);

    if (iter_start.day != comp_start.day) {
        comp_start.day = iter_start.day;
        icalcomponent_set_dtstart(comp, comp_start);
    }
}

icalcomponent *icaltzutil_fetch_timezone(const char *location)
{
    tzinfo type_cnts;
    size_t i, num_trans, num_chars, num_leaps, num_isstd, num_isgmt;
    size_t num_types = 0;
    size_t size;
    time_t trans;
    int dstidx = -1, stdidx = -1, pos, sign, zidx, zp_idx;
    icalcomponent *std_comp = NULL;

    const char *zonedir;
    FILE *f = NULL;
    char *full_path = NULL;
    time_t *transitions = NULL;
    char *r_trans = NULL, *temp;
    int *trans_idx = NULL;
    ttinfo *types = NULL;
    char *znames = NULL;
    leap *leaps = NULL;
    char *tzid = NULL;

    time_t start, end;
    int idx, prev_idx;
    icalcomponent *tz_comp = NULL, *comp = NULL, *dst_comp;
    icalproperty *icalprop;
    icaltimetype dtstart, icaltime;
    struct icalrecurrencetype ical_recur;

    if (icaltimezone_get_builtin_tzdata()) {
        goto error;
    }

    zonedir = icaltzutil_get_zone_directory();
    if (!zonedir) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        goto error;
    }

    size = strlen(zonedir) + strlen(location) + 2;
    full_path = (char *)malloc(size);
    if (full_path == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    snprintf(full_path, size, "%s/%s", zonedir, location);
    if ((f = fopen(full_path, "rb")) == 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        goto error;
    }

    if (fseek(f, 20, SEEK_SET) != 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        goto error;
    }

    EFREAD(&type_cnts, 24, 1, f);

    num_isgmt = (size_t)decode(type_cnts.ttisgmtcnt);
    num_leaps = (size_t)decode(type_cnts.leapcnt);
    num_chars = (size_t)decode(type_cnts.charcnt);
    num_trans = (size_t)decode(type_cnts.timecnt);
    num_isstd = (size_t)decode(type_cnts.ttisstdcnt);
    num_types = (size_t)decode(type_cnts.typecnt);

    transitions = calloc(num_trans, sizeof(time_t));
    if (transitions == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    r_trans = calloc(num_trans, 4);
    if (r_trans == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }

    EFREAD(r_trans, 4, num_trans, f);
    temp = r_trans;
    if (num_trans) {
        trans_idx = calloc(num_trans, sizeof(int));
        if (trans_idx == NULL) {
            icalerror_set_errno(ICAL_NEWFAILED_ERROR);
            goto error;
        }
        for (i = 0; i < num_trans; i++) {
            trans_idx[i] = fgetc(f);
            transitions[i] = (time_t) decode(r_trans);
            r_trans += 4;
        }
    }
    r_trans = temp;

    types = calloc(num_types, sizeof(ttinfo));
    if (types == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    for (i = 0; i < num_types; i++) {
        unsigned char a[4];
        int c;

        EFREAD(a, 4, 1, f);
        c = fgetc(f);
        types[i].isdst = (unsigned char)c;
        if ((c = fgetc(f)) < 0) {
            break;
        }
        types[i].abbr = (unsigned int)c;
        types[i].gmtoff = decode(a);
    }

    znames = (char *)malloc(num_chars);
    if (znames == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    EFREAD(znames, num_chars, 1, f);

    /* We got all the information which we need */

    leaps = calloc(num_leaps, sizeof(leap));
    if (leaps == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    for (i = 0; i < num_leaps; i++) {
        char c[4];

        EFREAD(c, 4, 1, f);
        leaps[i].transition = (time_t)decode(c);

        EFREAD(c, 4, 1, f);
        leaps[i].change = decode(c);
    }

    for (i = 0; i < num_isstd; ++i) {
        int c = getc(f);
        types[i].isstd = c != 0;
    }

    while (i < num_types) {
        types[i++].isstd = 0;
    }

    for (i = 0; i < num_isgmt; ++i) {
        int c = getc(f);

        types[i].isgmt = c != 0;
    }

    while (i < num_types) {
        types[i++].isgmt = 0;
    }

    /* Read all the contents now */

    for (i = 0; i < num_types; i++) {
        types[i].zname = zname_from_stridx(znames, (long)types[i].abbr);
    }

    if (!_s_use_exact_timezones) {
        if (num_trans != 0) {
            find_transidx(transitions, types, trans_idx, (long int)num_trans, &stdidx, &dstidx);
        } else {
            stdidx = 0;
        }
    }

    tz_comp = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);

    /* Add tzid property */
    size = strlen(icaltimezone_tzid_prefix()) + strlen(location) + 1;
    tzid = (char *)malloc(size);
    if (tzid == NULL) {
        icalerror_set_errno(ICAL_NEWFAILED_ERROR);
        goto error;
    }
    snprintf(tzid, size, "%s%s", icaltimezone_tzid_prefix(), location);
    icalprop = icalproperty_new_tzid(tzid);
    icalcomponent_add_property(tz_comp, icalprop);

    icalprop = icalproperty_new_x(location);
    icalproperty_set_x_name(icalprop, "X-LIC-LOCATION");
    icalcomponent_add_property(tz_comp, icalprop);

    if (!_s_use_exact_timezones) {
        if (stdidx != -1) {
            if (num_trans != 0) {
                zidx = trans_idx[stdidx];
            } else {
                zidx = 0;
            }

            std_comp = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);
            icalprop = icalproperty_new_tzname(types[zidx].zname);
            icalcomponent_add_property(std_comp, icalprop);

            if (dstidx != -1) {
                zp_idx = trans_idx[stdidx-1];
            } else {
                zp_idx = zidx;
            }
            /* DTSTART localtime uses TZOFFSETFROM UTC offset */
            if (num_trans != 0) {
                trans = transitions[stdidx] + types[zp_idx].gmtoff;
            } else {
                trans = (time_t)types[zp_idx].gmtoff;
            }
            icaltime = icaltime_from_timet(trans, 0);
            dtstart = icaltime;
            dtstart.year = 1970;
            dtstart.minute = dtstart.second = 0;
            icalprop = icalproperty_new_dtstart(dtstart);
            icalcomponent_add_property(std_comp, icalprop);

            /* If DST changes are present use RRULE */
            if (dstidx != -1) {
                icalrecurrencetype_clear(&ical_recur);
                ical_recur.freq = ICAL_YEARLY_RECURRENCE;
                ical_recur.by_month[0] = icaltime.month;
                pos = calculate_pos(icaltime);
                pos < 0 ? (sign = -1): (sign = 1);
                ical_recur.by_day[0] = sign * ((abs(pos) * 8) + icaltime_day_of_week(icaltime));
                icalprop = icalproperty_new_rrule(ical_recur);
                icalcomponent_add_property(std_comp, icalprop);

                adjust_dtstart_day_to_rrule(std_comp, ical_recur);
            }
            icalprop = icalproperty_new_tzoffsetfrom(types[zp_idx].gmtoff);
            icalcomponent_add_property(std_comp, icalprop);
            icalprop = icalproperty_new_tzoffsetto(types[zidx].gmtoff);
            icalcomponent_add_property(std_comp, icalprop);
            icalcomponent_add_component(tz_comp, std_comp);
        } else {
            icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        }

        if (dstidx != -1) {
            zidx = trans_idx[dstidx];
            zp_idx = trans_idx[dstidx-1];
            dst_comp = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);
            icalprop = icalproperty_new_tzname(types[zidx].zname);
            icalcomponent_add_property(dst_comp, icalprop);

            /* DTSTART localtime uses TZOFFSETFROM UTC offset */
            if (num_trans != 0) {
                trans = transitions[dstidx] + types[zp_idx].gmtoff;
            } else {
                trans = (time_t)types[zp_idx].gmtoff;
            }

            icaltime = icaltime_from_timet(trans, 0);
            dtstart = icaltime;
            dtstart.year = 1970;
            dtstart.minute = dtstart.second = 0;
            icalprop = icalproperty_new_dtstart(dtstart);
            icalcomponent_add_property(dst_comp, icalprop);

            icalrecurrencetype_clear(&ical_recur);
            ical_recur.freq = ICAL_YEARLY_RECURRENCE;
            ical_recur.by_month[0] = icaltime.month;
            pos = calculate_pos(icaltime);
            pos < 0 ? (sign = -1): (sign = 1);
            ical_recur.by_day[0] = sign * ((abs(pos) * 8) + icaltime_day_of_week(icaltime));
            icalprop = icalproperty_new_rrule(ical_recur);
            icalcomponent_add_property(dst_comp, icalprop);

            adjust_dtstart_day_to_rrule(dst_comp, ical_recur);

            icalprop = icalproperty_new_tzoffsetfrom(types[zp_idx].gmtoff);
            icalcomponent_add_property(dst_comp, icalprop);

            icalprop = icalproperty_new_tzoffsetto(types[zidx].gmtoff);
            icalcomponent_add_property(dst_comp, icalprop);

            icalcomponent_add_component(tz_comp, dst_comp);
        }
    } else { /*exact vtimezones*/
        prev_idx = 0;
        if (num_trans == 0) {
            prev_idx = idx = 0;
        } else {
            idx = trans_idx[0];
        }
        start = 0;
        for (i = 1; i < num_trans; i++, start = end) {
            prev_idx = idx;
            idx = trans_idx[i];
            end = transitions[i] + types[prev_idx].gmtoff;
            /* don't bother starting until the epoch */
            if (0 > end)
                continue;

            if (types[prev_idx].isdst) {
                comp = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);
            } else {
                comp = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);
            }
            icalprop = icalproperty_new_tzname(types[prev_idx].zname);
            icalcomponent_add_property(comp, icalprop);
            dtstart = icaltime_from_timet(start, 0);
            icalprop = icalproperty_new_dtstart(dtstart);
            icalcomponent_add_property(comp, icalprop);
            icalprop = icalproperty_new_tzoffsetfrom(types[idx].gmtoff);
            icalcomponent_add_property(comp, icalprop);
            icalprop = icalproperty_new_tzoffsetto(types[prev_idx].gmtoff);
            icalcomponent_add_property(comp, icalprop);
            icalcomponent_add_component(tz_comp, comp);
        }
        /* finally, add a last zone with no end date */
        if (types[idx].isdst) {
            comp = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);
        } else {
            comp = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);
        }
        icalprop = icalproperty_new_tzname(types[idx].zname);
        icalcomponent_add_property(comp, icalprop);
        dtstart = icaltime_from_timet(start, 0);
        icalprop = icalproperty_new_dtstart(dtstart);
        icalcomponent_add_property(comp, icalprop);
        icalprop = icalproperty_new_tzoffsetfrom(types[prev_idx].gmtoff);
        icalcomponent_add_property(comp, icalprop);
        icalprop = icalproperty_new_tzoffsetto(types[idx].gmtoff);
        icalcomponent_add_property(comp, icalprop);
        icalcomponent_add_component(tz_comp, comp);
    }

  error:
    if (f)
        fclose(f);

    if (full_path)
        free(full_path);

    if (transitions)
        free(transitions);

    if (r_trans)
        free(r_trans);

    if (trans_idx)
        free(trans_idx);

    if (types) {
        for (i = 0; i < num_types; i++) {
            if (types[i].zname) {
                free(types[i].zname);
            }
        }
        free(types);
    }

    if (znames)
        free(znames);

    if (leaps)
        free(leaps);

    if (tzid)
        free(tzid);

    return tz_comp;
}
