/*
======================================================================
 FILE: icaldirset.c
 CREATOR: eric 28 November 1999

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

/**
   @file   icaldirset.c

   @brief  icaldirset manages a database of ical components and offers
  interfaces for reading, writing and searching for components.

  icaldirset groups components in to clusters based on their DTSTAMP
  time -- all components that start in the same month are grouped
  together in a single file. All files in a sotre are kept in a single
  directory.

  The primary interfaces are icaldirset__get_first_component and
  icaldirset_get_next_component. These routine iterate through all of
  the components in the store, subject to the current gauge. A gauge
  is an icalcomponent that is tested against other componets for a
  match. If a gauge has been set with icaldirset_select,
  icaldirset_first and icaldirset_next will only return componentes
  that match the gauge.

  The Store generated UIDs for all objects that are stored if they do
  not already have a UID. The UID is the name of the cluster (month &
  year as MMYYYY) plus a unique serial number. The serial number is
  stored as a property of the cluster.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icaldirset.h"
#include "icaldirsetimpl.h"
#include "icalfileset.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H)
#include <sys/utsname.h>
#endif

/** Default options used when NULL is passed to icalset_new() **/
static icaldirset_options icaldirset_options_default = { O_RDWR | O_CREAT };

const char *icaldirset_path(icalset *set)
{
    icaldirset *dset = (icaldirset *) set;

    return dset->dir;
}

void icaldirset_mark(icalset *set)
{
    icaldirset *dset = (icaldirset *) set;

    icalcluster_mark(dset->cluster);
}

icalerrorenum icaldirset_commit(icalset *set)
{
    icaldirset *dset = (icaldirset *) set;
    icalset *fileset;
    icalfileset_options options = icalfileset_options_default;

    options.cluster = dset->cluster;

    fileset = icalset_new(ICAL_FILE_SET, icalcluster_key(dset->cluster), &options);

    (void)fileset->commit(fileset);
    fileset->free(fileset);

    return ICAL_NO_ERROR;
}

static void icaldirset_lock(const char *dir)
{
    _unused(dir);
}

static void icaldirset_unlock(const char *dir)
{
    _unused(dir);
}

/* Load the contents of the store directory into the store's internal directory list*/
static icalerrorenum icaldirset_read_directory(icaldirset *dset)
{
    char *str;

#if defined(HAVE_DIRENT_H)
    struct dirent *de;
    DIR *dp;

    dp = opendir(dset->dir);

    if (dp == 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        return ICAL_FILE_ERROR;
    }

    /* clear contents of directory list */
    while ((str = pvl_pop(dset->directory))) {
        free(str);
    }

    /* load all of the cluster names in the directory list */
    for (de = readdir(dp); de != 0; de = readdir(dp)) {

        /* Remove known directory names  '.' and '..' */
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        pvl_push(dset->directory, (void *)strdup(de->d_name));
    }

    closedir(dp);
#else
    struct _finddata_t c_file;
    intptr_t hFile;

    /* Find first .c file in current directory */
    if ((hFile = _findfirst("*", &c_file)) == -1L) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        return ICAL_FILE_ERROR;
    } else {
        while ((str = pvl_pop(dset->directory))) {
            free(str);
        }

        /* load all of the cluster names in the directory list */
        do {
            /* Remove known directory names  '.' and '..' */
            if (strcmp(c_file.name, ".") == 0 || strcmp(c_file.name, "..") == 0) {
                continue;
            }

            pvl_push(dset->directory, (void *)strdup(c_file.name));
        } while (_findnext(hFile, &c_file) == 0);

        _findclose(hFile);
    }

#endif

    return ICAL_NO_ERROR;
}

icalset *icaldirset_init(icalset *set, const char *dir, void *options_in)
{
    icaldirset *dset;
    icaldirset_options *options = options_in;
    struct stat sbuf;

    icalerror_check_arg_rz((dir != 0), "dir");
    icalerror_check_arg_rz((set != 0), "set");
    dset = (icaldirset *) set;

    if (stat(dir, &sbuf) != 0) {
        icalerror_set_errno(ICAL_FILE_ERROR);
        return 0;
    }

    /* dir is not the name of a direectory */
    if (!S_ISDIR(sbuf.st_mode)) {
        icalerror_set_errno(ICAL_USAGE_ERROR);
        return 0;
    }

    icaldirset_lock(dir);

    dset->dir = (char *)strdup(dir);
    dset->options = *options;
    dset->directory = pvl_newlist();
    dset->directory_iterator = 0;
    dset->gauge = 0;
    dset->first_component = 0;
    dset->cluster = 0;

    return set;
}

icalset *icaldirset_new(const char *dir)
{
    return icalset_new(ICAL_DIR_SET, dir, &icaldirset_options_default);
}

icalset *icaldirset_new_reader(const char *dir)
{
    icaldirset_options reader_options = icaldirset_options_default;

    reader_options.flags = O_RDONLY;

    return icalset_new(ICAL_DIR_SET, dir, &reader_options);
}

icalset *icaldirset_new_writer(const char *dir)
{
    icaldirset_options writer_options = icaldirset_options_default;

    writer_options.flags = O_RDWR | O_CREAT;

    return icalset_new(ICAL_DIR_SET, dir, &writer_options);
}

void icaldirset_free(icalset *s)
{
    icaldirset *dset = (icaldirset *) s;
    char *str;

    icaldirset_unlock(dset->dir);

    if (dset->dir != 0) {
        free(dset->dir);
        dset->dir = 0;
    }

    if (dset->gauge != 0) {
        icalgauge_free(dset->gauge);
        dset->gauge = 0;
    }

    if (dset->cluster != 0) {
        icalcluster_free(dset->cluster);
    }

    while (dset->directory != 0 && (str = pvl_pop(dset->directory)) != 0) {
        free(str);
    }

    if (dset->directory != 0) {
        pvl_free(dset->directory);
        dset->directory = 0;
    }

    dset->directory_iterator = 0;
    dset->first_component = 0;
}

static icalerrorenum icaldirset_next_cluster(icaldirset *dset)
{
    char path[MAXPATHLEN];

    if (dset->directory_iterator == 0) {
        icalerror_set_errno(ICAL_INTERNAL_ERROR);
        return ICAL_INTERNAL_ERROR;
    }
    dset->directory_iterator = pvl_next(dset->directory_iterator);

    if (dset->directory_iterator == 0) {
        /* There are no more clusters */
        if (dset->cluster != 0) {
            icalcluster_free(dset->cluster);
            dset->cluster = 0;
        }
        return ICAL_NO_ERROR;
    }

    snprintf(path, sizeof(path), "%s/%s", dset->dir, (char *)pvl_data(dset->directory_iterator));

    icalcluster_free(dset->cluster);
    dset->cluster = icalfileset_produce_icalcluster(path);

    return icalerrno;
}

static void icaldirset_add_uid(icalcomponent *comp)
{
    char uidstring[MAXPATHLEN] = { 0 };
    icalproperty *uid;

#if defined(HAVE_SYS_UTSNAME_H)
    struct utsname unamebuf;
#endif

    icalerror_check_arg_rv((comp != 0), "comp");

    uid = icalcomponent_get_first_property(comp, ICAL_UID_PROPERTY);

    if (uid == 0) {

#if defined(HAVE_SYS_UTSNAME_H)
        uname(&unamebuf);
        snprintf(uidstring, sizeof(uidstring), "%d-%s", (int)getpid(), unamebuf.nodename);
#else
        /* FIXME: There must be an easy get the system name */
        snprintf(uidstring, sizeof(uidstring), "%d-%s", (int)getpid(), "WINDOWS");
#endif
        uid = icalproperty_new_uid(uidstring);
        icalcomponent_add_property(comp, uid);
    } else {
        strncpy(uidstring, icalproperty_get_uid(uid), MAXPATHLEN - 1);
        uidstring[MAXPATHLEN - 1] = '\0';
    }
}

/**
  This assumes that the top level component is a VCALENDAR, and there
   is an inner component of type VEVENT, VTODO or VJOURNAL. The inner
  component must have a DSTAMP property
*/

icalerrorenum icaldirset_add_component(icalset *set, icalcomponent *comp)
{
    char clustername[MAXPATHLEN] = { 0 };
    icalproperty *dt = 0;
    icalvalue *v;
    struct icaltimetype tm;
    icalerrorenum error = ICAL_NO_ERROR;
    icalcomponent *inner;
    icaldirset *dset;

    icalerror_check_arg_rz((set != 0), "set");
    icalerror_check_arg_rz((comp != 0), "comp");

    dset = (icaldirset *) set;

    icaldirset_add_uid(comp);

    /* Determine which cluster this object belongs in. This is a HACK */

    for (inner = icalcomponent_get_first_component(comp, ICAL_ANY_COMPONENT);
         inner != 0; inner = icalcomponent_get_next_component(comp, ICAL_ANY_COMPONENT)) {

        dt = icalcomponent_get_first_property(inner, ICAL_DTSTAMP_PROPERTY);

        if (dt != 0) {
            break;
        }
    }

    if (dt == 0) {
        for (inner = icalcomponent_get_first_component(comp, ICAL_ANY_COMPONENT);
             inner != 0; inner = icalcomponent_get_next_component(comp, ICAL_ANY_COMPONENT)) {

            dt = icalcomponent_get_first_property(inner, ICAL_DTSTART_PROPERTY);

            if (dt != 0) {
                break;
            }
        }
    }

    if (dt == 0) {
        icalerror_warn("The component does not have a DTSTAMP or DTSTART property, "
                       "so it cannot be added to the store");
        icalerror_set_errno(ICAL_BADARG_ERROR);
        return ICAL_BADARG_ERROR;
    }

    v = icalproperty_get_value(dt);
    tm = icalvalue_get_datetime(v);

    snprintf(clustername, MAXPATHLEN, "%s/%04d%02d", dset->dir, tm.year, tm.month);

    /* Load the cluster and insert the object */
    if (dset->cluster != 0 && strcmp(clustername, icalcluster_key(dset->cluster)) != 0) {
        icalcluster_free(dset->cluster);
        dset->cluster = 0;
    }

    if (dset->cluster == 0) {
        dset->cluster = icalfileset_produce_icalcluster(clustername);

        if (dset->cluster == 0) {
            error = icalerrno;
        }
    }

    if (error != ICAL_NO_ERROR) {
        icalerror_set_errno(error);
        return error;
    }

    /* Add the component to the cluster */
    (void)icalcluster_add_component(dset->cluster, comp);

    /* icalcluster_mark(impl->cluster); */

    return ICAL_NO_ERROR;
}

/**
   Remove a component in the current cluster. HACK. This routine is a
   "friend" of icalfileset, and breaks its encapsulation. It was
   either do it this way, or add several layers of interfaces that had
   no other use.
 */

icalerrorenum icaldirset_remove_component(icalset *set, icalcomponent *comp)
{
    icaldirset *dset;
    icalcomponent *filecomp;
    icalcompiter i;
    int found = 0;

    icalerror_check_arg_re((set != 0), "set", ICAL_BADARG_ERROR);
    icalerror_check_arg_re((comp != 0), "comp", ICAL_BADARG_ERROR);

    dset = (icaldirset *) set;
    icalerror_check_arg_re((dset->cluster != 0), "Cluster pointer", ICAL_USAGE_ERROR);

    filecomp = icalcluster_get_component(dset->cluster);

    for (i = icalcomponent_begin_component(filecomp, ICAL_ANY_COMPONENT);
         icalcompiter_deref(&i) != 0; icalcompiter_next(&i)) {

        icalcomponent *this = icalcompiter_deref(&i);

        if (this == comp) {
            found = 1;
            break;
        }
    }

    if (found != 1) {
        icalerror_warn("icaldirset_remove_component: component is not part of current cluster");
        icalerror_set_errno(ICAL_USAGE_ERROR);
        return ICAL_USAGE_ERROR;
    }

    (void)icalcluster_remove_component(dset->cluster, comp);

    /* icalcluster_mark(impl->cluster); */

    /* If the removal emptied the fileset, get the next fileset */
    if (icalcluster_count_components(dset->cluster, ICAL_ANY_COMPONENT) == 0) {
        icalerrorenum error = icaldirset_next_cluster(dset);

        if (dset->cluster != 0 && error == ICAL_NO_ERROR) {
            (void)icalcluster_get_first_component(dset->cluster);
        } else {
            /* HACK. Not strictly correct for impl->cluster==0 */
            return error;
        }
    } else {
        /* Do nothing */
    }

    return ICAL_NO_ERROR;
}

int icaldirset_count_components(icalset *store, icalcomponent_kind kind)
{
    _unused(store);
    _unused(kind);

    /* NOT IMPLEMENTED */
    assert(0);

    return 0;
}

icalcomponent *icaldirset_fetch_match(icalset *set, icalcomponent *c)
{
    _unused(set);
    _unused(c);

    fprintf(stderr, " icaldirset_fetch_match is not implemented\n");
    assert(0);
    return 0;
}

icalcomponent *icaldirset_fetch(icalset *set, icalcomponent_kind kind, const char *uid)
{
    icaldirset *dset;
    icalgauge *gauge;
    icalgauge *old_gauge;
    icalcomponent *c;
    char sql[256];

    _unused(kind);

    icalerror_check_arg_rz((set != 0), "set");
    icalerror_check_arg_rz((uid != 0), "uid");

    snprintf(sql, 256, "SELECT * FROM VEVENT WHERE UID = \"%s\"", uid);

    gauge = icalgauge_new_from_sql(sql, 0);
    dset = (icaldirset *) set;
    old_gauge = dset->gauge;
    dset->gauge = gauge;

    c = icaldirset_get_first_component(set);

    dset->gauge = old_gauge;

    if (gauge) {
        icalgauge_free(gauge);
    }

    return c;
}

int icaldirset_has_uid(icalset *set, const char *uid)
{
    icalcomponent *c;

    icalerror_check_arg_rz((set != 0), "set");
    icalerror_check_arg_rz((uid != 0), "uid");

    /* HACK. This is a temporary implementation. _has_uid should use a
       database, and _fetch should use _has_uid, not the other way
       around */
    c = icaldirset_fetch(set, 0, uid);

    return c != 0;
}

icalerrorenum icaldirset_select(icalset *set, icalgauge *gauge)
{
    icaldirset *dset;

    icalerror_check_arg_re((set != 0), "set", ICAL_BADARG_ERROR);
    icalerror_check_arg_re((gauge != 0), "gauge", ICAL_BADARG_ERROR);

    dset = (icaldirset *) set;
    dset->gauge = gauge;

    return ICAL_NO_ERROR;
}

icalerrorenum icaldirset_modify(icalset *set, icalcomponent *old, icalcomponent *new)
{
    _unused(set);
    _unused(old);
    _unused(new);

    /* NOT IMPLEMENTED */
    assert(0);
    return ICAL_NO_ERROR;
}

void icaldirset_clear(icalset *set)
{
    _unused(set);

    /* NOT IMPLEMENTED */
    assert(0);
    return;
}

icalcomponent *icaldirset_get_current_component(icalset *set)
{
    icaldirset *dset = (icaldirset *) set;

    if (dset->cluster == 0) {
        (void)icaldirset_get_first_component(set);
    }
    if (dset->cluster == 0) {
        return 0;
    }

    return icalcluster_get_current_component(dset->cluster);
}

icalcomponent *icaldirset_get_first_component(icalset *set)
{
    icaldirset *dset = (icaldirset *) set;

    icalerrorenum error;
    char path[MAXPATHLEN];

    error = icaldirset_read_directory(dset);

    if (error != ICAL_NO_ERROR) {
        icalerror_set_errno(error);
        return 0;
    }

    dset->directory_iterator = pvl_head(dset->directory);

    if (dset->directory_iterator == 0) {
        icalerror_set_errno(error);
        return 0;
    }

    snprintf(path, MAXPATHLEN, "%s/%s", dset->dir, (char *)pvl_data(dset->directory_iterator));

    /* If the next cluster we need is different than the current cluster,
       delete the current one and get a new one */

    if (dset->cluster != 0 && strcmp(path, icalcluster_key(dset->cluster)) != 0) {
        icalcluster_free(dset->cluster);
        dset->cluster = 0;
    }

    if (dset->cluster == 0) {
        dset->cluster = icalfileset_produce_icalcluster(path);

        if (dset->cluster == 0) {
            error = icalerrno;
        }
    }

    if (error != ICAL_NO_ERROR) {
        icalerror_set_errno(error);
        return 0;
    }

    dset->first_component = 1;

    return icaldirset_get_next_component(set);
}

icalcomponent *icaldirset_get_next_component(icalset *set)
{
    icaldirset *dset;
    icalcomponent *c;
    icalerrorenum error;

    icalerror_check_arg_rz((set != 0), "set");
    dset = (icaldirset *) set;

    if (dset->cluster == 0) {
        icalerror_warn("icaldirset_get_next_component called with a NULL cluster "
                       "(Caller must call icaldirset_get_first_component first)");
        icalerror_set_errno(ICAL_USAGE_ERROR);
        return 0;
    }

    /* Set the component iterator for the following for loop */
    if (dset->first_component == 1) {
        (void)icalcluster_get_first_component(dset->cluster);
        dset->first_component = 0;
    } else {
        (void)icalcluster_get_next_component(dset->cluster);
    }

    while (1) {
        /* Iterate through all of the objects in the cluster */
        for (c = icalcluster_get_current_component(dset->cluster);
             c != 0; c = icalcluster_get_next_component(dset->cluster)) {

            /* If there is a gauge defined and the component does not
               pass the gauge, skip the rest of the loop */

            if (dset->gauge != 0 && icalgauge_compare(dset->gauge, c) == 0) {
                continue;
            }

            /* Either there is no gauge, or the component passed the
               gauge, so return it */

            return c;
        }

        /* Fell through the loop, so the component we want is not
           in this cluster. Load a new cluster and try again. */

        error = icaldirset_next_cluster(dset);

        if (dset->cluster == 0 || error != ICAL_NO_ERROR) {
            /* No more clusters */
            return 0;
        } else {
            c = icalcluster_get_first_component(dset->cluster);

            return c;
        }
    }

    return 0;   /* Should never get here */
}

icalsetiter icaldirset_begin_component(icalset *set, icalcomponent_kind kind, icalgauge *gauge,
                                       const char *tzid)
{
    _unused(set);
    _unused(kind);
    _unused(gauge);
    _unused(tzid);
    /*
       icalsetiter itr = icalsetiter_null;
       icaldirset *fset = (icaldirset*) set;

       icalerror_check_arg_re((fset!=0), "set", icalsetiter_null);

       itr.iter.kind = kind;
       itr.gauge = gauge;
     */
    /* TO BE IMPLEMENTED */
    return icalsetiter_null;
}

icalcomponent *icaldirsetiter_to_next(icalset *set, icalsetiter *i)
{
    _unused(set);
    _unused(i);
    /* TO BE IMPLEMENTED */
    return NULL;
}

icalcomponent *icaldirsetiter_to_prior(icalset *set, icalsetiter *i)
{
    _unused(set);
    _unused(i);
    /* TO BE IMPLEMENTED */
    return NULL;
}
