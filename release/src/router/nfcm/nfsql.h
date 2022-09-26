#ifndef __NFSQL_H__
#define __NFSQL_H__

#include <sqlite3.h>
#include "list.h"

extern sqlite3 *sqlite_open(NFCM_DB_TYPE db_type, sqlite3 *db, char *fname);
extern int sqlite_close(sqlite3 *db);

extern int sqlite_integrity_check(sqlite3 *db, char *db_path);
extern void sqlite_remove_journal(char *db_file);

extern int sqlite_app_insert(sqlite3 *db, struct list_head *iplist);
extern int sqlite_sum_insert(sqlite3 *db, struct list_head *smlist);
extern int sqlite_tcp_insert(sqlite3 *db, struct list_head *tcplist);

extern int sqlite_get_timestamp(sqlite3 *db, time_t *timestamp);
extern int sqlite_db_timestamp_select(sqlite3 *db, time_t *timestamp, char *sql);

#endif /* __NFSQL_H__ */
