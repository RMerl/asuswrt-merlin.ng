/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDB")
docDescription("A sorted key/value pair database implemented with skip lists on top of UDB.")
*/

#ifndef SkipDB_DEFINED
#define SkipDB_DEFINED 1

#if !defined(__MINGW32__) && defined(WIN32)
#if defined(BUILDING_SKIPDB_DLL) || defined(BUILDING_IOVMALL_DLL)
#define SKIPDB_API __declspec(dllexport)
#else
#define SKIPDB_API __declspec(dllimport)
#endif
#else
#define SKIPDB_API
#endif


#include "SkipDBRecord.h"
#include "UDB.h"
#include "PHash.h"
#include "RandomGen.h"

#ifdef __cplusplus
extern "C" {
#endif

// if prob. dist = 0.5, then max level 32 is enough for 2^32 records

//#define SKIPDB_MAX_LEVEL 32
#define SKIPDB_MAX_LEVEL 16
#define SKIPDB_PROBABILITY_DISTRIBUTION 0.5

typedef void (SkipDBObjectMarkFunc)(void *);
typedef void (SkipDBFreeObjectFunc)(void *);

typedef struct
{
	UDB *udb;

    void *dbm;
    PID_TYPE headerPid;
    SkipDBRecord *header;
    SkipDBRecord *youngestRecord; // most recently accessed

    SkipDBRecord *update[SKIPDB_MAX_LEVEL];
    float p;

    BStream *stream;
    SkipDBObjectMarkFunc *objectMarkFunc;
    SkipDBFreeObjectFunc *objectFreeFunc;
    List *cursors;

    List *dirtyRecords;
    List *pidsToRemove;

    size_t cachedRecordCount;
    size_t cacheHighWaterMark;
    size_t cacheLowWaterMark;
    unsigned char mark; // current record mark identifier
    PHash *pidToRecord;
    RandomGen *randomGen;
} SkipDB;

#include "SkipDBCursor.h"

SKIPDB_API SkipDB *SkipDB_new(void);
SKIPDB_API void SkipDB_setPath_(SkipDB *self, char *path);
SKIPDB_API int SkipDB_open(SkipDB *self);
SKIPDB_API void SkipDB_close(SkipDB *self);
SKIPDB_API void SkipDB_free(SkipDB *self);

SKIPDB_API void SkipDB_retain(SkipDB *self);
SKIPDB_API void SkipDB_release(SkipDB *self);

SKIPDB_API BStream *SkipDB_tmpStream(SkipDB *self);

SKIPDB_API void SkipDB_headerPid_(SkipDB *self, PID_TYPE pid);
SKIPDB_API PID_TYPE SkipDB_headerPid(SkipDB *self);
SKIPDB_API SkipDBRecord *SkipDB_headerRecord(SkipDB *self);

SKIPDB_API UDB *SkipDB_udb(SkipDB *self);
SKIPDB_API int SkipDB_isOpen(SkipDB *self);
SKIPDB_API void SkipDB_delete(SkipDB *self);

// notifications

SKIPDB_API void SkipDB_noteNewRecord_(SkipDB *self, SkipDBRecord *r);
SKIPDB_API void SkipDB_noteAccessedRecord_(SkipDB *self, SkipDBRecord *r);
SKIPDB_API void SkipDB_noteDirtyRecord_(SkipDB *self, SkipDBRecord *r);
SKIPDB_API void SkipDB_noteAssignedPidToRecord_(SkipDB *self, SkipDBRecord *r);
SKIPDB_API void SkipDB_noteWillFreeRecord_(SkipDB *self, SkipDBRecord *r);

// cache

SKIPDB_API void SkipDB_setCacheHighWaterMark_(SkipDB *self, size_t recordCount);
SKIPDB_API size_t SkipDB_cacheHighWaterMark(SkipDB *self);

SKIPDB_API void SkipDB_setCacheLowWaterMark_(SkipDB *self, size_t recordCount);
SKIPDB_API size_t SkipDB_cacheLowWaterMark(SkipDB *self);

SKIPDB_API void SkipDB_clearCache(SkipDB *self);
SKIPDB_API void SkipDB_freeAllCachedRecords(SkipDB *self);
SKIPDB_API int SkipDB_headerIsEmpty(SkipDB *self);

// transactions

SKIPDB_API void SkipDB_beginTransaction(SkipDB *self);
SKIPDB_API void SkipDB_commitTransaction(SkipDB *self);

SKIPDB_API void SkipDB_sync(SkipDB *self);
SKIPDB_API void SkipDB_removeDirtyRecordsFromSavedRecords(SkipDB *self);
SKIPDB_API void SkipDB_saveDirtyRecords(SkipDB *self);
void SkipDB_deleteRecordsToRemove(SkipDB *self);

// record api

SKIPDB_API SkipDBRecord *SkipDB_recordAt_(SkipDB *self, Datum k);
SKIPDB_API SkipDBRecord *SkipDB_recordAt_put_(SkipDB *self, Datum k, Datum v);
SKIPDB_API int SkipDB_replace_put_(SkipDB *self, Datum k, Datum v);

// bdb style api

SKIPDB_API int SkipDB_at_put_(SkipDB *self, Datum k, Datum v);
SKIPDB_API Datum SkipDB_at_(SkipDB *self, Datum k);
SKIPDB_API void SkipDB_removeAt_(SkipDB *self, Datum k);

// compact

SKIPDB_API int SkipDB_compact(SkipDB *self);

// debugging

SKIPDB_API void SkipDB_showUpdate(SkipDB *self);
SKIPDB_API void SkipDB_show(SkipDB *self);

// private

SKIPDB_API void SkipDB_updateAt_put_(SkipDB *self, int level, SkipDBRecord *r);
SKIPDB_API SkipDBRecord *SkipDB_recordAtPid_(SkipDB *self, PID_TYPE pid);

// objects

SKIPDB_API void SkipDB_objectMarkFunc_(SkipDB *self, SkipDBObjectMarkFunc *func);
SKIPDB_API void SkipDB_freeObjectCallback_(SkipDB *, SkipDBFreeObjectFunc *func);

// cursor

SKIPDB_API int SkipDB_count(SkipDB *self);

SKIPDB_API SkipDBRecord *SkipDB_firstRecord(SkipDB *self);
SKIPDB_API SkipDBRecord *SkipDB_lastRecord(SkipDB *self);
SKIPDB_API SkipDBRecord *SkipDB_goto_(SkipDB *self, Datum key);

SKIPDB_API SkipDBCursor *SkipDB_createCursor(SkipDB *self);
SKIPDB_API void SkipDB_removeCursor_(SkipDB *self, SkipDBCursor *cursor);

// moving from in-memory to on-disk

SKIPDB_API void SkipDB_mergeInto_(SkipDB *self, SkipDB *other);

SKIPDB_API int SkipDB_exists(SkipDB *self, Datum key);
typedef void (*skipdb_list_callback)(SkipDB* self, SkipDBRecord* rc, void* ctx);
SKIPDB_API void SkipDB_list_prefix(SkipDB* self, Datum k, void* ctx, skipdb_list_callback callback);

#ifdef __cplusplus
}
#endif
#endif
