/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDBRecords")    
docDescription("An object for storing and fetching UDB records from the records file.")
*/

#ifndef UDBRecords_DEFINED
#define UDBRecords_DEFINED 1

#include "List.h"
#include "UDBRecord.h"
#include "UDBIndex.h"
#include "JFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    JFile *file;
    UDBRecord *record;
    UDBRecord *firstEmptyRecord;
    PID_TYPE firstEmptyPos;
} UDBRecords;

UDBRecords *UDBRecords_new(void);
void UDBRecords_free(UDBRecords *self);

JFile *UDBRecords_jfile(UDBIndex *self);

void UDBRecords_setPath_(UDBRecords *self, const char *s);
void UDBRecords_setLogPath_(UDBRecords *self, const char *s);
char *UDBRecords_path(UDBRecords *self);

void UDBRecords_delete(UDBRecords *self);
void UDBRecords_open(UDBRecords *self);
void UDBRecords_close(UDBRecords *self);

void UDBRecords_begin(UDBRecords *self);
void UDBRecords_preCommit(UDBRecords *self);
void UDBRecords_commit(UDBRecords *self);
int UDBRecords_isUncommitted(UDBRecords *self);

// ops -------------------------------------------------- 

UDBRecord *UDBRecords_firstRecord(UDBRecords *self);
UDBRecord *UDBRecords_newRecord(UDBRecords *self);
UDBRecord *UDBRecords_nextRecord(UDBRecords *self);
UDBRecord *UDBRecords_recordAfter_(UDBRecords *self, UDBRecord *record);
UDBRecord *UDBRecords_recordAtPos_(UDBRecords *self, PID_TYPE pos);
void UDBRecords_removeRecord_(UDBRecords *self, UDBRecord *record);
void UDBRecords_moveRecord_toPos_(UDBRecords *self, UDBRecord *record, PID_TYPE pos);

void UDBRecords_truncate_(UDBRecords *self, off_t size);

// first empty record ------------------------------------ 

UDBRecord *UDBRecords_findFirstEmptyRecord(UDBRecords *self);
//UDBRecord *UDBRecords_nextEmptyRecordAfter_(UDBRecords *self, UDBRecord *record);
PID_TYPE UDBRecords_firstEmptyRecordPos(UDBRecords *self);
void UDBRecords_firstEmptyRecordPos_(UDBRecords *self, PID_TYPE firstEmptyPos);
UDBRecord *UDBRecords_firstEmptyRecord(UDBRecords *self);

#ifdef __cplusplus
}
#endif
#endif
