/*======================================================================
 FILE: icalfileset.c
 CREATOR: eric 23 December 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom
======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalfileset.h"
#include "icalfilesetimpl.h"
#include "icalparser.h"
#include "icalvalue.h"

#include <errno.h>
#include <stdlib.h>

#if defined(_WIN32_WCE)
#include <winbase.h>
#endif

/** Default options used when NULL is passed to icalset_new() **/
icalfileset_options icalfileset_options_default = { O_RDWR | O_CREAT, 0644, 0, NULL };

static int icalfileset_lock(icalfileset *set);
static int icalfileset_unlock(icalfileset *set);
static icalerrorenum icalfileset_read_file(icalfileset *set, int mode);
static long icalfileset_filesize(icalfileset *set);

icalerrorenum icalfileset_create_cluster(const char *path);

icalset *icalfileset_new(const char *path)
{
    return icalset_new(ICAL_FILE_SET, path, &icalfileset_options_default);
}

icalset *icalfileset_new_reader(const char *path)
{
    icalfileset_options reader_options = icalfileset_options_default;

    reader_options.flags = O_RDONLY;

    return icalset_new(ICAL_FILE_SET, path, &reader_options);
}

icalset *icalfileset_new_writer(const char *path)
{
    icalfileset_options writer_options = icalfileset_options_default;

    writer_options.flags = O_RDONLY;

    return icalset_new(ICAL_FILE_SET, path, &writer_options);
}

icalset *icalfileset_init(icalset *set, const char *path, void *options_in)
{
    icalfileset_options *options = (options_in) ? options_in : &icalfileset_options_default;
    icalfileset *fset = (icalfileset *) set;
    int flags;
    int mode;
    long cluster_file_size;

    icalerror_clear_errno();
    icalerror_check_arg_rz((path != 0), "path");
    icalerror_check_arg_rz((fset != 0), "fset");

    fset->path = strdup(path);
    fset->options = *options;

    flags = options->flags;
    mode = options->mode;

    cluster_file_size = icalfileset_filesize(fset);

    if (cluster_file_size < 0) {
        icalfileset_free(set);
        return 0;
    }

    fset->fd = open(fset->path, flags, mode);

    if (fset->fd < 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        icalfileset_free(set);
        return 0;
    }

    (void)icalfileset_lock(fset);

    if (cluster_file_size > 0) {
        icalerrorenum error;

        if ((error = icalfileset_read_file(fset, mode)) != ICAL_NO_ERROR) {
            icalfileset_free(set);
            return 0;
        }
    }

    if (options->cluster) {
        fset->cluster = icalcomponent_new_clone(icalcluster_get_component(options->cluster));
        fset->changed = 1;
    }

    if (fset->cluster == 0) {
        fset->cluster = icalcomponent_new(ICAL_XROOT_COMPONENT);
    }

    return set;
}

icalcluster *icalfileset_produce_icalcluster(const char *path)
{
    icalset *fileset;
    icalcluster *ret;

    int errstate = icalerror_get_errors_are_fatal();

    icalerror_set_errors_are_fatal(0);

    fileset = icalfileset_new_reader(path);

    if (fileset == 0 || icalerrno == ICAL_FILE_ERROR) {
        /* file does not exist */
        ret = icalcluster_new(path, NULL);
    } else {
        ret = icalcluster_new(path, ((icalfileset *) fileset)->cluster);
        icalfileset_free(fileset);
    }

    icalerror_set_errors_are_fatal(errstate);
    icalerror_set_errno(ICAL_NO_ERROR);
    return ret;
}

static char *icalfileset_read_from_file(char *s, size_t size, void *d)
{
    char *p = s;
    icalfileset *set = d;

    /* Simulate fgets -- read single characters and stop at '\n' */

    for (p = s; p < s + size - 1; p++) {

        if (read(set->fd, p, 1) != 1 || *p == '\n') {
            p++;
            break;
        }
    }

    *p = '\0';

    if (*s == 0) {
        return 0;
    } else {
        return s;
    }
}

icalerrorenum icalfileset_read_file(icalfileset *set, int mode)
{
    icalparser *parser;

    _unused(mode);

    parser = icalparser_new();

    icalparser_set_gen_data(parser, set);
    set->cluster = icalparser_parse(parser, icalfileset_read_from_file);
    icalparser_free(parser);

    if (set->cluster == 0 || icalerrno != ICAL_NO_ERROR) {
        icalerror_set_errno(ICAL_PARSE_ERROR);
        /*return ICAL_PARSE_ERROR; */
    }

    if (icalcomponent_isa(set->cluster) != ICAL_XROOT_COMPONENT) {
        /* The parser got a single component, so it did not put it in
           an XROOT. */
        icalcomponent *cl = set->cluster;

        set->cluster = icalcomponent_new(ICAL_XROOT_COMPONENT);
        icalcomponent_add_component(set->cluster, cl);
    }

    return ICAL_NO_ERROR;
}

long icalfileset_filesize(icalfileset *fset)
{
    struct stat sbuf;

    if (stat(fset->path, &sbuf) != 0) {

        /* A file by the given name does not exist, or there was
           another error */
        if (errno == ENOENT) {
            /* It was because the file does not exist */
            return 0;
        } else {
            /* It was because of another error */
            icalerror_set_errno(ICAL_FILE_ERROR);
            return -1;
        }
    } else {
        /* A file by the given name exists, but is it a regular file? */

        if (!S_ISREG(sbuf.st_mode)) {
            /* Nope, not a regular file */
            icalerror_set_errno(ICAL_FILE_ERROR);
            return -1;
        } else {
            /* Lets assume that it is a file of the right type */
            return (long)sbuf.st_size;
        }
    }

    /*return -1; not reached */
}

void icalfileset_free(icalset *set)
{
    icalfileset *fset;

    icalerror_check_arg_rv((set != 0), "set");

    fset = (icalfileset *) set;

    if (fset->cluster != 0) {
        (void)icalfileset_commit(set);
        icalcomponent_free(fset->cluster);
        fset->cluster = 0;
    }

    if (fset->gauge != 0) {
        icalgauge_free(fset->gauge);
        fset->gauge = 0;
    }

    if (fset->fd > 0) {
        (void)icalfileset_unlock(fset);
        close(fset->fd);
        fset->fd = -1;
    }

    if (fset->path != 0) {
        free(fset->path);
        fset->path = 0;
    }
}

const char *icalfileset_path(icalset *set)
{
    icalerror_check_arg_rz((set != 0), "set");

    return ((icalfileset *) set)->path;
}

int icalfileset_lock(icalfileset *set)
{
#if !defined(_WIN32)
    struct flock lock;
    int rtrn;

    icalerror_check_arg_rz((set->fd > 0), "set->fd");
    errno = 0;
    lock.l_type = F_WRLCK;      /* F_RDLCK, F_WRLCK, F_UNLCK */
    lock.l_start = 0;   /* byte offset relative to l_whence */
    lock.l_whence = SEEK_SET;   /* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = 0;     /* #bytes (0 means to EOF) */

    rtrn = fcntl(set->fd, F_SETLKW, &lock);

    return rtrn;
#else
    _unused(set);
    return 0;
#endif
}

int icalfileset_unlock(icalfileset *set)
{
#if !defined(_WIN32)
    struct flock lock;

    icalerror_check_arg_rz((set->fd > 0), "set->fd");

    lock.l_type = F_WRLCK;      /* F_RDLCK, F_WRLCK, F_UNLCK */
    lock.l_start = 0;   /* byte offset relative to l_whence */
    lock.l_whence = SEEK_SET;   /* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = 0;     /* #bytes (0 means to EOF) */

    return (fcntl(set->fd, F_UNLCK, &lock));
#else
    _unused(set);
    return 0;
#endif
}

#if !defined(_WIN32)
static char *shell_quote(const char *s)
{
    char *result;
    char *p;

    p = result = malloc(strlen(s) * 5 + 1);
    while (*s) {
        if (*s == '\'') {
            *p++ = '\'';
            *p++ = '"';
            *p++ = *s++;
            *p++ = '"';
            *p++ = '\'';
        } else {
            *p++ = *s++;
        }
    }
    *p = '\0';
    return result;
}

#endif

icalerrorenum icalfileset_commit(icalset *set)
{
    char tmp[MAXPATHLEN];
    char *str;
    icalcomponent *c;
    size_t write_size = 0;
    icalfileset *fset = (icalfileset *) set;

#if defined(_WIN32_WCE)
    wchar_t *wtmp = 0;
    PROCESS_INFORMATION pi;
#endif

    icalerror_check_arg_re((fset != 0), "set", ICAL_BADARG_ERROR);

    icalerror_check_arg_re((fset->fd > 0), "set->fd is invalid", ICAL_INTERNAL_ERROR);

    if (fset->changed == 0) {
        return ICAL_NO_ERROR;
    }

    if (fset->options.safe_saves == 1) {
#if !defined(_WIN32)
        char *quoted_file = shell_quote(fset->path);

        snprintf(tmp, MAXPATHLEN, "cp '%s' '%s.bak'", fset->path, fset->path);
        free(quoted_file);
#else
        snprintf(tmp, MAXPATHLEN, "copy %s %s.bak", fset->path, fset->path);
#endif

#if !defined(_WIN32_WCE)
        if (system(tmp) < 0) {
#else

        wtmp = wce_mbtowc(tmp);

        if (CreateProcess(wtmp, L"", NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi)) {
#endif
            icalerror_set_errno(ICAL_FILE_ERROR);
            return ICAL_FILE_ERROR;
        }
    }
#if defined(_WIN32_WCE)
    free(wtmp);
#endif

    if (lseek(fset->fd, 0, SEEK_SET) < 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        return ICAL_FILE_ERROR;
    }

    for (c = icalcomponent_get_first_component(fset->cluster, ICAL_ANY_COMPONENT);
         c != 0; c = icalcomponent_get_next_component(fset->cluster, ICAL_ANY_COMPONENT)) {
        IO_SSIZE_T sz;

        str = icalcomponent_as_ical_string_r(c);

        sz = write(fset->fd, str, (IO_SIZE_T) strlen(str));
        if (sz != (IO_SSIZE_T) strlen(str)) {
            perror("write");
            icalerror_set_errno(ICAL_FILE_ERROR);
            free(str);
            return ICAL_FILE_ERROR;
        }

        free(str);
        write_size += sz;
    }

    fset->changed = 0;

#if !defined(_WIN32)
    if (ftruncate(fset->fd, (off_t) write_size) < 0) {
        return ICAL_FILE_ERROR;
    }
#else
#if !defined(_WIN32_WCE)
    chsize(fset->fd, tell(fset->fd));
#else
    SetEndOfFile(fset->fd);
#endif
#endif

    return ICAL_NO_ERROR;
}

void icalfileset_mark(icalset *set)
{
    icalerror_check_arg_rv((set != 0), "set");

    ((icalfileset *) set)->changed = 1;
}

icalcomponent *icalfileset_get_component(icalset *set)
{
    icalfileset *fset;

    icalerror_check_arg_rz((set != 0), "set");

    fset = (icalfileset *) set;
    return fset->cluster;
}

/* manipulate the components in the set */

icalerrorenum icalfileset_add_component(icalset *set, icalcomponent *child)
{
    icalfileset *fset;

    icalerror_check_arg_re((set != 0), "set", ICAL_BADARG_ERROR);
    icalerror_check_arg_re((child != 0), "child", ICAL_BADARG_ERROR);

    fset = (icalfileset *) set;
    icalcomponent_add_component(fset->cluster, child);

    icalfileset_mark(set);

    return ICAL_NO_ERROR;
}

icalerrorenum icalfileset_remove_component(icalset *set, icalcomponent *child)
{
    icalfileset *fset;

    icalerror_check_arg_re((set != 0), "set", ICAL_BADARG_ERROR);
    icalerror_check_arg_re((child != 0), "child", ICAL_BADARG_ERROR);

    fset = (icalfileset *) set;
    icalcomponent_remove_component(fset->cluster, child);

    icalfileset_mark(set);

    return ICAL_NO_ERROR;
}

int icalfileset_count_components(icalset *set, icalcomponent_kind kind)
{
    icalfileset *fset;

    if (set == 0) {
        icalerror_set_errno(ICAL_BADARG_ERROR);
        return -1;
    }

    fset = (icalfileset *) set;
    return icalcomponent_count_components(fset->cluster, kind);
}

icalerrorenum icalfileset_select(icalset *set, icalgauge *gauge)
{
    icalfileset *fset;

    icalerror_check_arg_re(gauge != 0, "gauge", ICAL_BADARG_ERROR);

    fset = (icalfileset *) set;
    fset->gauge = gauge;

    return ICAL_NO_ERROR;
}

void icalfileset_clear(icalset *set)
{
    icalfileset *fset;

    icalerror_check_arg_rv(set != 0, "set");

    fset = (icalfileset *) set;
    fset->gauge = 0;
}

icalcomponent *icalfileset_fetch(icalset *set, icalcomponent_kind kind, const char *uid)
{
    icalfileset *fset;
    icalcompiter i;

    _unused(kind);

    icalerror_check_arg_rz(set != 0, "set");
    fset = (icalfileset *) set;

    for (i = icalcomponent_begin_component(fset->cluster, ICAL_ANY_COMPONENT);
         icalcompiter_deref(&i) != 0; icalcompiter_next(&i)) {

        icalcomponent *this = icalcompiter_deref(&i);
        icalcomponent *inner;
        icalproperty *p;
        const char *this_uid;

        for (inner = icalcomponent_get_first_component(this, ICAL_ANY_COMPONENT);
             inner != 0; inner = icalcomponent_get_next_component(this, ICAL_ANY_COMPONENT)) {

            p = icalcomponent_get_first_property(inner, ICAL_UID_PROPERTY);
            if (p) {
                this_uid = icalproperty_get_uid(p);

                if (this_uid == 0) {
                    icalerror_warn("icalfileset_fetch found a component with no UID");
                    continue;
                }

                if (strcmp(uid, this_uid) == 0) {
                    return this;
                }
            }
        }
    }

    return 0;
}

int icalfileset_has_uid(icalset *set, const char *uid)
{
    _unused(set);
    _unused(uid);
    assert(0);  /* HACK, not implemented */
    return 0;
}

/******* support routines for icalfileset_fetch_match *********/

struct icalfileset_id
{
    char *uid;
    char *recurrence_id;
    int sequence;
};

static void icalfileset_id_free(struct icalfileset_id *id)
{
    if (id->recurrence_id != 0) {
        free(id->recurrence_id);
    }

    if (id->uid != 0) {
        free(id->uid);
    }
}

static struct icalfileset_id icalfileset_get_id(icalcomponent *comp)
{
    icalcomponent *inner;
    struct icalfileset_id id;
    icalproperty *p;

    inner = icalcomponent_get_first_real_component(comp);

    p = icalcomponent_get_first_property(inner, ICAL_UID_PROPERTY);

    assert(p != 0);

    id.uid = strdup(icalproperty_get_uid(p));

    p = icalcomponent_get_first_property(inner, ICAL_SEQUENCE_PROPERTY);

    if (p == 0) {
        id.sequence = 0;
    } else {
        id.sequence = icalproperty_get_sequence(p);
    }

    p = icalcomponent_get_first_property(inner, ICAL_RECURRENCEID_PROPERTY);

    if (p == 0) {
        id.recurrence_id = 0;
    } else {
        icalvalue *v;

        v = icalproperty_get_value(p);
        id.recurrence_id = icalvalue_as_ical_string_r(v);

        assert(id.recurrence_id != 0);
    }

    return id;
}

/* Find the component that is related to the given
   component. Currently, it just matches based on UID and
   RECURRENCE-ID */
icalcomponent *icalfileset_fetch_match(icalset *set, icalcomponent *comp)
{
    icalfileset *fset = (icalfileset *) set;
    icalcompiter i;

    struct icalfileset_id comp_id, match_id;

    comp_id = icalfileset_get_id(comp);

    for (i = icalcomponent_begin_component(fset->cluster, ICAL_ANY_COMPONENT);
         icalcompiter_deref(&i) != 0; icalcompiter_next(&i)) {

        icalcomponent *match = icalcompiter_deref(&i);

        match_id = icalfileset_get_id(match);

        if (strcmp(comp_id.uid, match_id.uid) == 0 &&
            (comp_id.recurrence_id == 0 ||
             strcmp(comp_id.recurrence_id, match_id.recurrence_id) == 0)) {

            /* HACK. What to do with SEQUENCE? */

            icalfileset_id_free(&match_id);
            icalfileset_id_free(&comp_id);
            return match;
        }

        icalfileset_id_free(&match_id);
    }

    icalfileset_id_free(&comp_id);
    return 0;
}

icalerrorenum icalfileset_modify(icalset *set, icalcomponent *old, icalcomponent *new)
{
    _unused(set);
    _unused(old);
    _unused(new);
    assert(0);  /* HACK, not implemented */
    return ICAL_NO_ERROR;
}

/* Iterate through components */
icalcomponent *icalfileset_get_current_component(icalset *set)
{
    icalfileset *fset;

    icalerror_check_arg_rz((set != 0), "set");

    fset = (icalfileset *) set;
    return icalcomponent_get_current_component(fset->cluster);
}

icalcomponent *icalfileset_get_first_component(icalset *set)
{
    icalcomponent *c = 0;
    icalfileset *fset;

    icalerror_check_arg_rz((set != 0), "set");
    fset = (icalfileset *) set;

    do {
        if (c == 0) {
            c = icalcomponent_get_first_component(fset->cluster, ICAL_ANY_COMPONENT);
        } else {
            c = icalcomponent_get_next_component(fset->cluster, ICAL_ANY_COMPONENT);
        }

        if (c != 0 && (fset->gauge == 0 || icalgauge_compare(fset->gauge, c) == 1)) {
            return c;
        }

    } while (c != 0);

    return 0;
}

icalcomponent *icalfileset_get_next_component(icalset *set)
{
    icalfileset *fset;
    icalcomponent *c;

    icalerror_check_arg_rz((set != 0), "set");
    fset = (icalfileset *) set;

    do {
        c = icalcomponent_get_next_component(fset->cluster, ICAL_ANY_COMPONENT);

        if (c != 0 && (fset->gauge == 0 || icalgauge_compare(fset->gauge, c) == 1)) {
            return c;
        }

    } while (c != 0);

    return 0;
}

/*
icalsetiter icalfileset_begin_component(icalset* set, icalcomponent_kind kind, icalgauge* gauge)
{
    icalsetiter itr = icalsetiter_null;
    icalcomponent* comp = NULL;
    icalcompiter citr;
    icalfileset *fset = (icalfileset*)set;

    icalerror_check_arg_re((set!=0), "set", icalsetiter_null);

    itr.gauge = gauge;

    citr = icalcomponent_begin_component(fset->cluster, kind);
    comp = icalcompiter_deref(&citr);

    while (comp != 0) {
    comp = icalcompiter_deref(&citr);
    if (gauge == 0 || icalgauge_compare(itr.gauge, comp) == 1) {
        itr.iter = citr;
        return itr;
    }
    comp =  icalcompiter_next(&citr);
    }

    return icalsetiter_null;
}

*/

icalsetiter icalfileset_begin_component(icalset *set, icalcomponent_kind kind, icalgauge *gauge,
                                        const char *tzid)
{
    icalsetiter itr = icalsetiter_null;
    icalcomponent *comp = NULL;
    icalcompiter citr;
    icalfileset *fset;
    struct icaltimetype start, next;
    icalproperty *dtstart, *rrule, *prop, *due;
    struct icalrecurrencetype recur;
    int g = 0;

    _unused(tzid);

    icalerror_check_arg_re((set != 0), "set", icalsetiter_null);

    start = icaltime_from_timet(time(0), 0);
    itr.gauge = gauge;

    fset = (icalfileset *) set;
    citr = icalcomponent_begin_component(fset->cluster, kind);
    comp = icalcompiter_deref(&citr);

    if (gauge == 0) {
        itr.iter = citr;
        return itr;
    }

    while (comp != 0) {

        /* check if it is a recurring component and with guage expand, if so
           we need to add recurrence-id property to the given component */
        rrule = icalcomponent_get_first_property(comp, ICAL_RRULE_PROPERTY);
        g = icalgauge_get_expand(gauge);

        if (rrule != 0 && g == 1) {

            recur = icalproperty_get_rrule(rrule);
            if (icalcomponent_isa(comp) == ICAL_VEVENT_COMPONENT) {
                dtstart = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
                if (dtstart) {
                    start = icalproperty_get_dtstart(dtstart);
                }
            } else if (icalcomponent_isa(comp) == ICAL_VTODO_COMPONENT) {
                due = icalcomponent_get_first_property(comp, ICAL_DUE_PROPERTY);
                if (due) {
                    start = icalproperty_get_due(due);
                }
            }

            if (itr.last_component == NULL) {
                itr.ritr = icalrecur_iterator_new(recur, start);
                next = icalrecur_iterator_next(itr.ritr);
                itr.last_component = comp;
            } else {
                next = icalrecur_iterator_next(itr.ritr);
                if (icaltime_is_null_time(next)) {
                    itr.last_component = NULL;
                    icalrecur_iterator_free(itr.ritr);
                    itr.ritr = NULL;
                    return icalsetiter_null;
                } else {
                    itr.last_component = comp;
                }
            }

            /* add recurrence-id to the component
               if there is a recurrence-id already, remove it, then add the new one */
            prop = icalcomponent_get_first_property(comp, ICAL_RECURRENCEID_PROPERTY);
            if (prop) {
                icalcomponent_remove_property(comp, prop);
            }
            icalcomponent_add_property(comp, icalproperty_new_recurrenceid(next));
        }

        if (gauge == 0 || icalgauge_compare(itr.gauge, comp) == 1) {
            /* matches and returns */
            itr.iter = citr;
            return itr;
        }

        /* if there is no previous component pending, then get the next component */
        if (itr.last_component == NULL) {
            comp = icalcompiter_next(&citr);
        }
    }

    return icalsetiter_null;
}

icalcomponent *icalfileset_form_a_matched_recurrence_component(icalsetiter *itr)
{
    icalcomponent *comp = NULL;
    struct icaltimetype start, next;
    icalproperty *dtstart, *rrule, *prop, *due;
    struct icalrecurrencetype recur;

    start = icaltime_from_timet(time(0), 0);
    comp = itr->last_component;

    if (comp == NULL || itr->gauge == NULL) {
        return NULL;
    }

    rrule = icalcomponent_get_first_property(comp, ICAL_RRULE_PROPERTY);

    recur = icalproperty_get_rrule(rrule);

    if (icalcomponent_isa(comp) == ICAL_VEVENT_COMPONENT) {
        dtstart = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
        if (dtstart) {
            start = icalproperty_get_dtstart(dtstart);
        }
    } else if (icalcomponent_isa(comp) == ICAL_VTODO_COMPONENT) {
        due = icalcomponent_get_first_property(comp, ICAL_DUE_PROPERTY);
        if (due) {
            start = icalproperty_get_due(due);
        }
    }

    if (itr->ritr == NULL) {
        itr->ritr = icalrecur_iterator_new(recur, start);
        next = icalrecur_iterator_next(itr->ritr);
        itr->last_component = comp;
    } else {
        next = icalrecur_iterator_next(itr->ritr);
        if (icaltime_is_null_time(next)) {
            /* no more recurrence, returns */
            itr->last_component = NULL;
            icalrecur_iterator_free(itr->ritr);
            itr->ritr = NULL;
            return NULL;
        } else {
            itr->last_component = comp;
        }
    }

    /* add recurrence-id to the component
     * if there is a recurrence-id already, remove it, then add the new one */
    prop = icalcomponent_get_first_property(comp, ICAL_RECURRENCEID_PROPERTY);
    if (prop) {
        icalcomponent_remove_property(comp, prop);
    }
    icalcomponent_add_property(comp, icalproperty_new_recurrenceid(next));

    if (itr->gauge == 0 || icalgauge_compare(itr->gauge, comp) == 1) {
        /* matches and returns */
        return comp;
    }
    /* not matched */
    return NULL;
}

icalcomponent *icalfilesetiter_to_next(icalset *set, icalsetiter *i)
{
    icalcomponent *c = NULL;
    struct icaltimetype start, next;
    icalproperty *dtstart, *rrule, *prop, *due;
    struct icalrecurrencetype recur;
    int g = 0;

    _unused(set);

    start = icaltime_from_timet(time(0), 0);
    next = icaltime_from_timet(time(0), 0);

    do {
        c = icalcompiter_next(&(i->iter));

        if (c == 0) {
            continue;
        }
        if (i->gauge == 0) {
            return c;
        }

        rrule = icalcomponent_get_first_property(c, ICAL_RRULE_PROPERTY);
        g = icalgauge_get_expand(i->gauge);

        /* a recurring component with expand query */
        if (rrule != 0 && g == 1) {

            recur = icalproperty_get_rrule(rrule);

            if (icalcomponent_isa(c) == ICAL_VEVENT_COMPONENT) {
                dtstart = icalcomponent_get_first_property(c, ICAL_DTSTART_PROPERTY);
                if (dtstart) {
                    start = icalproperty_get_dtstart(dtstart);
                }
            } else if (icalcomponent_isa(c) == ICAL_VTODO_COMPONENT) {
                due = icalcomponent_get_first_property(c, ICAL_DUE_PROPERTY);
                if (due) {
                    start = icalproperty_get_due(due);
                }
            }

            if (i->ritr == NULL) {
                i->ritr = icalrecur_iterator_new(recur, start);
                next = icalrecur_iterator_next(i->ritr);
                i->last_component = c;
            } else {
                next = icalrecur_iterator_next(i->ritr);
                if (icaltime_is_null_time(next)) {
                    /* no more recurrence, returns */
                    i->last_component = NULL;
                    icalrecur_iterator_free(i->ritr);
                    i->ritr = NULL;
                    return NULL;
                } else {
                    i->last_component = c;
                }
            }
        }

        /* add recurrence-id to the component
         * if there is a recurrence-id already, remove it, then add the new one */
        prop = icalcomponent_get_first_property(c, ICAL_RECURRENCEID_PROPERTY);
        if (prop) {
            icalcomponent_remove_property(c, prop);
        }
        icalcomponent_add_property(c, icalproperty_new_recurrenceid(next));

        if (c != 0 && (i->gauge == 0 || icalgauge_compare(i->gauge, c) == 1)) {
            return c;
        }
    } while (c != 0);

    return 0;
}
