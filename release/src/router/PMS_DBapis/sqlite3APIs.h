#include "sqlite3.h"

#define PMS_SQLITEAPI(x)						\
{									\
	int ret=0;							\
	char *errMsg = NULL;						\
	ret=sqlite_##x;							\
	if(ret != SQLITE_OK)						\
		printf("SQL error: %s, %s\n", #x,sqlite3_errmsg (db));	\
}
#define TABLE_ACC_USER          0 
#define TABLE_ACC_GROUP         1
#define TABLE_DEV_ACCOUNT       2
#define TABLE_DEV_GROUP         3 
#define TABLE_USER2GROUP        4
#define TABLE_GROUP2USER        5
#define TABLE_DEV2GROUP         6
#define TABLE_GROUP2DEV         7
#define TABLE_SERVICES          8

#define USER2GROUP              0
#define GROUP2USER              1

#define ACCUSERCOL              6
#define ACCGROUPCOL             4
#define ACCMATCHCOL             2
#define DEVUSERCOL              3
#define DEVGROUPCOL             4
#define DEVMATCHCOL             2

extern int sqlite_create(sqlite3 *);
extern int sqlite_getTable(sqlite3 * , int , int *, int *, char ***, char *, int );
extern int sqlite_update(sqlite3 *, int, char *[]);
extern int sqlite_delete(sqlite3 *, int , char *);
extern int sqlite_modify(sqlite3 *, int , char *[], char *);

extern int sqlite_open(char *, sqlite3 **, sqlite3 *);
extern void sqlite_freeTable(char **);
extern int sqlite_close(sqlite3 *);

