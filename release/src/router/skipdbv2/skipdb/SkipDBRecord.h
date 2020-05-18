/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("SkipDBRecord")    
docDescription("A skip list record.")
*/

#ifndef SkipDBRecord_DEFINED
#define SkipDBRecord_DEFINED 1

#include "List.h"
#include "BStream.h"
#include "UDB.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SkipDBRecord  SkipDBRecord;
typedef struct SkipDBPointer SkipDBPointer;

#define SKIPDBRECORD_UNMARKED  0
#define SKIPDBRECORD_MARKED    1

struct SkipDBPointer
{
    PID_TYPE pid;
    unsigned char matchingPrefixSize;
    SkipDBRecord *record;
};

struct SkipDBRecord
{
    int level;
    SkipDBPointer *pointers;
    
    PID_TYPE previousPid;
    SkipDBRecord *previousRecord;

    UArray *key;
    UArray *value;

    void *sdb;
    PID_TYPE pid;
    void *object; // extra pointer for user to make use of - not stored 
    int ownsKey;
    uint8_t isDirty;

    // cache related 

    SkipDBRecord *youngerRecord; // accessed before self 
    SkipDBRecord *olderRecord;   // accessed after self 
    unsigned char mark;
};

SKIPDB_API int SkipDBRecord_pointersAreEmpty(SkipDBRecord *self);

SKIPDB_API SkipDBRecord *SkipDBRecord_new(void);
SKIPDB_API SkipDBRecord *SkipDBRecord_newWithDB_(void *db);

SKIPDB_API void SkipDBRecord_setOlderRecord_(SkipDBRecord *self, SkipDBRecord *r);
SKIPDB_API SkipDBRecord *SkipDBRecord_olderRecord(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_setYoungerRecord_(SkipDBRecord *self, SkipDBRecord *r);
SKIPDB_API SkipDBRecord *SkipDBRecord_youngerRecord(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_removeFromAgeList(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_removeReferencesToUnmarked(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_dealloc(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_db_(SkipDBRecord *self, void *db);

SKIPDB_API void SkipDBRecord_pid_(SkipDBRecord *self, PID_TYPE pid);
SKIPDB_API PID_TYPE SkipDBRecord_pid(SkipDBRecord *self);
SKIPDB_API PID_TYPE SkipDBRecord_pidAllocIfNeeded(SkipDBRecord *self);

SKIPDB_API int SkipDBRecord_level(SkipDBRecord *self);
SKIPDB_API void SkipDBRecord_level_(SkipDBRecord *self, int level);

SKIPDB_API void SkipDBRecord_mark(SkipDBRecord *self);

SKIPDB_API uint8_t SkipDBRecord_isDirty(SkipDBRecord *self);
SKIPDB_API void SkipDBRecord_markAsDirty(SkipDBRecord *self);
SKIPDB_API void SkipDBRecord_markAsClean(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_atLevel_setPid_(SkipDBRecord *self, int level, PID_TYPE pid);
SKIPDB_API PID_TYPE SkipDBRecord_pidAtLevel_(SkipDBRecord *self, int level);

SKIPDB_API void SkipDBRecord_atLevel_setRecord_(SkipDBRecord *self, int level, SkipDBRecord *r);
SKIPDB_API SkipDBRecord *SkipDBRecord_recordAtLevel_(SkipDBRecord *self, int level);

SKIPDB_API void SkipDBRecord_copyLevel_from_(SkipDBRecord *self, int level, SkipDBRecord *other);

//-------------------------------------------- 

SKIPDB_API void SkipDBRecord_keyDatum_(SkipDBRecord *self, Datum k);
SKIPDB_API Datum SkipDBRecord_keyDatum(SkipDBRecord *self);
SKIPDB_API UArray *SkipDBRecord_key(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_valueDatum_(SkipDBRecord *self, Datum v);
SKIPDB_API Datum SkipDBRecord_valueDatum(SkipDBRecord *self);
SKIPDB_API UArray *SkipDBRecord_value(SkipDBRecord *self);

// serialization ----------------------------- 

SKIPDB_API void SkipDBRecord_toStream_(SkipDBRecord *self, BStream *s);
SKIPDB_API void SkipDBRecord_fromStream_(SkipDBRecord *self, BStream *s);
SKIPDB_API void SkipDBRecord_save(SkipDBRecord *self);

// search ------------------------------------ 

SKIPDB_API SkipDBRecord *SkipDBRecord_find_quick_(SkipDBRecord *self, Datum key, int quick);
SkipDBRecord *SkipDBRecord_findLastRecord(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_willRemove_(SkipDBRecord *self, SkipDBRecord *other);

SKIPDB_API void SkipDBRecord_show(SkipDBRecord *self);

// object ------------------------------------ 

SKIPDB_API void SkipDBRecord_object_(SkipDBRecord *self, void *object);
SKIPDB_API void *SkipDBRecord_object(SkipDBRecord *self);

// next ------------------------------------ 

SKIPDB_API SkipDBRecord *SkipDBRecord_nextRecord(SkipDBRecord *self);

// previous ------------------------------------ 

SKIPDB_API void SkipDBRecord_previousPid_(SkipDBRecord *self, PID_TYPE pid);
SKIPDB_API PID_TYPE SkipDBRecord_previousPid(SkipDBRecord *self);

SKIPDB_API void SkipDBRecord_previousRecord_(SkipDBRecord *self, SkipDBRecord *r);
SKIPDB_API SkipDBRecord *SkipDBRecord_previousRecord(SkipDBRecord *self);

#ifdef __cplusplus
}
#endif
#endif
