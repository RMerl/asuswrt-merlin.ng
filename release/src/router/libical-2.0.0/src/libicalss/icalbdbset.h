/*======================================================================
 FILE: icalbdbset.h

 (C) COPYRIGHT 2001, Critical Path

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALBDBSET_H
#define ICALBDBSET_H

#include "libical_icalss_export.h"
#include "icalset.h"

#include <db.h>

typedef struct icalbdbset_impl icalbdbset;

enum icalbdbset_subdb_type
{ ICALBDB_CALENDARS, ICALBDB_EVENTS, ICALBDB_TODOS, ICALBDB_REMINDERS };
typedef enum icalbdbset_subdb_type icalbdbset_subdb_type;

/** sets up the db environment, should be done in parent thread.. */
LIBICAL_ICALSS_EXPORT int icalbdbset_init_dbenv(char *db_env_dir,
                                                void (*logDbFunc) (const DB_ENV *,
                                                                   const char *, const char *));

LIBICAL_ICALSS_EXPORT icalset *icalbdbset_init(icalset *set, const char *dsn, void *options);

LIBICAL_ICALSS_EXPORT int icalbdbset_cleanup(void);

LIBICAL_ICALSS_EXPORT void icalbdbset_checkpoint(void);

LIBICAL_ICALSS_EXPORT void icalbdbset_rmdbLog(void);

/** Creates a component handle.  flags allows caller to
   specify if database is internally a BTREE or HASH */
LIBICAL_ICALSS_EXPORT icalset *icalbdbset_new(const char *database_filename,
                                              icalbdbset_subdb_type subdb_type,
                                              int dbtype, u_int32_t flag);

LIBICAL_ICALSS_EXPORT DB *icalbdbset_bdb_open_secondary(DB *dbp,
                                                        const char *subdb,
                                                        const char *sindex,
                                                        int (*callback) (DB *db,
                                                                         const DBT *dbt1,
                                                                         const DBT *dbt2,
                                                                         DBT *dbt3), int type);

LIBICAL_ICALSS_EXPORT char *icalbdbset_parse_data(DBT *dbt, char *(*pfunc) (const DBT *dbt));

LIBICAL_ICALSS_EXPORT void icalbdbset_free(icalset *set);

/* cursor operations */
LIBICAL_ICALSS_EXPORT int icalbdbset_acquire_cursor(DB *dbp, DB_TXN *tid, DBC ** rdbcp);

LIBICAL_ICALSS_EXPORT int icalbdbset_cget(DBC *dbcp, DBT *key, DBT *data,
                                          u_int32_t access_method);

LIBICAL_ICALSS_EXPORT int icalbdbset_cput(DBC *dbcp, DBT *key, DBT *data,
                                          u_int32_t access_method);

LIBICAL_ICALSS_EXPORT int icalbdbset_get_first(DBC *dbcp, DBT *key, DBT *data);

LIBICAL_ICALSS_EXPORT int icalbdbset_get_next(DBC *dbcp, DBT *key, DBT *data);

LIBICAL_ICALSS_EXPORT int icalbdbset_get_last(DBC *dbcp, DBT *key, DBT *data);

LIBICAL_ICALSS_EXPORT int icalbdbset_get_key(DBC *dbcp, DBT *key, DBT *data);

LIBICAL_ICALSS_EXPORT int icalbdbset_delete(DB *dbp, DBT *key);

LIBICAL_ICALSS_EXPORT int icalbdbset_put(DB *dbp, DBT *key, DBT *data, u_int32_t access_method);

LIBICAL_ICALSS_EXPORT int icalbdbset_get(DB *dbp, DB_TXN *tid, DBT *key, DBT *data,
                                         u_int32_t flags);

LIBICAL_ICALSS_EXPORT const char *icalbdbset_path(icalset *set);

LIBICAL_ICALSS_EXPORT const char *icalbdbset_subdb(icalset *set);

/* Mark the set as changed, so it will be written to disk when it
   is freed. Commit writes to disk immediately. */
LIBICAL_ICALSS_EXPORT void icalbdbset_mark(icalset *set);

LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_commit(icalset *set);

LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_add_component(icalset *set, icalcomponent *child);

LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_remove_component(icalset *set,
                                                                icalcomponent *child);

LIBICAL_ICALSS_EXPORT int icalbdbset_count_components(icalset *set, icalcomponent_kind kind);

/* Restrict the component returned by icalbdbset_first, _next to those
   that pass the gauge. _clear removes the gauge */
LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_select(icalset *store, icalgauge *gauge);

LIBICAL_ICALSS_EXPORT void icalbdbset_clear(icalset *store);

/* Get and search for a component by uid */
LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_fetch(icalset *set,
                                                      icalcomponent_kind kind, const char *uid);

LIBICAL_ICALSS_EXPORT int icalbdbset_has_uid(icalset *set, const char *uid);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_fetch_match(icalset *set, icalcomponent *c);

LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_modify(icalset *set, icalcomponent *old,
                                                      icalcomponent *newc);

/* cluster management functions */
LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_set_cluster(icalset *set, icalcomponent *cluster);

LIBICAL_ICALSS_EXPORT icalerrorenum icalbdbset_free_cluster(icalset *set);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_get_cluster(icalset *set);

/* Iterate through components. If a gauge has been defined, these
   will skip over components that do not pass the gauge */

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_get_current_component(icalset *set);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_get_first_component(icalset *set);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_get_next_component(icalset *set);

/* External iterator for thread safety */
LIBICAL_ICALSS_EXPORT icalsetiter icalbdbset_begin_component(icalset *set,
                                                             icalcomponent_kind kind,
                                                             icalgauge *gauge, const char *tzid);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_form_a_matched_recurrence_component(icalsetiter *
                                                                                    itr);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbsetiter_to_next(icalset *set, icalsetiter *i);

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbsetiter_to_prior(icalset *set, icalsetiter *i);

/* Return a reference to the internal component. You probably should
   not be using this. */

LIBICAL_ICALSS_EXPORT icalcomponent *icalbdbset_get_component(icalset *set);

LIBICAL_ICALSS_EXPORT DB_ENV *icalbdbset_get_env(void);

LIBICAL_ICALSS_EXPORT int icalbdbset_begin_transaction(DB_TXN *parent_id, DB_TXN ** txnid);

LIBICAL_ICALSS_EXPORT int icalbdbset_commit_transaction(DB_TXN *txnid);

LIBICAL_ICALSS_EXPORT DB *icalbdbset_bdb_open(const char *path,
                                              const char *subdb,
                                              int type, int mode, u_int32_t flag);

typedef struct icalbdbset_options
{
    icalbdbset_subdb_type subdb;     /**< the subdatabase to open */
    int dbtype;                      /**< db_open type: DB_HASH | DB_BTREE */
    int mode;                        /**< file mode */
    u_int32_t flag;                  /**< DB->set_flags(): DB_DUP | DB_DUPSORT */
    char *(*pfunc) (const DBT *dbt);
                                    /**< parsing function */
    int (*callback) (DB *db,
                            /**< callback for secondary db open */
                     const DBT *dbt1, const DBT *dbt2, DBT *dbt3);
} icalbdbset_options;

#endif /* !ICALBDBSET_H */
