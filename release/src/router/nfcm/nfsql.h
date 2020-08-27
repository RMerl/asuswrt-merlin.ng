#ifndef __NFSQL_H__
#define __NFSQL_H__

#include <sqlite3.h>
#include "list.h"

extern sqlite3 *sqlite_open(bool is_app, sqlite3 *db, char *fname);
extern int sqlite_close(sqlite3 *db);
extern int sqlite_app_insert(sqlite3 *db, struct list_head *iplist);
extern int sqlite_sum_insert(sqlite3 *db, struct list_head *smlist);
extern int sqlite_select(sqlite3 *db, char *sql);

#endif /* __NFSQL_H__ */
