#ifndef _ark_db_h_
#define _ark_db_h_

#include "sqlite3.h"
#include "json.h"
#include "time.h"

#define DAY_SEC     86400
#define QUERY_LEN   960

// copy from httpd.h
#define websWrite(wp, fmt, args...) ({ int TMPVAR = fprintf(wp, fmt, ## args); fflush(wp); TMPVAR; })

extern int ark_sql_exec(sqlite3 *db, const char *sql);
#endif /* _ark_db_h_ */
