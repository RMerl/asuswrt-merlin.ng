/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDBCursor")    
docDescription("A cursor for a skipdb.")
*/


#ifndef SkipDBCursor_DEFINED
#define SkipDBCursor_DEFINED 1

typedef struct SkipDBCursor SkipDBCursor;

#include "SkipDB.h"
#include <stdio.h>
#include <sys/types.h> 

#ifdef __cplusplus
extern "C" {
#endif

struct SkipDBCursor
{
    int refCount;
    SkipDB *sdb;
    SkipDBRecord *record;
};

SKIPDB_API SkipDBCursor *SkipDBCursor_new(void);
SKIPDB_API SkipDBCursor *SkipDBCursor_newWithDB_(SkipDB *sdb);
SKIPDB_API void SkipDBCursor_sdb_(SkipDB *sdb);
SKIPDB_API void SkipDBCursor_retain(SkipDBCursor *self);
SKIPDB_API void SkipDBCursor_release(SkipDBCursor *self);
SKIPDB_API void SkipDBCursor_mark(SkipDBCursor *self);

SKIPDB_API SkipDBRecord *SkipDBCursor_goto_(SkipDBCursor *self, Datum key);

SKIPDB_API SkipDBRecord *SkipDBCursor_first(SkipDBCursor *self);
SKIPDB_API SkipDBRecord *SkipDBCursor_last(SkipDBCursor *self);

SKIPDB_API SkipDBRecord *SkipDBCursor_previous(SkipDBCursor *self);
SKIPDB_API SkipDBRecord *SkipDBCursor_current(SkipDBCursor *self);
SKIPDB_API SkipDBRecord *SkipDBCursor_next(SkipDBCursor *self);

#ifdef __cplusplus
}
#endif
#endif
