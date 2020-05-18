/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDBIndex")    
docDescription("An on-disk array of indexes into the data file.")
*/

#ifndef UDBIndex_DEFINED
#define UDBIndex_DEFINED 1

#include "List.h"
#include "UDBRecord.h"
#include "JFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    //unsigned char sizeLog2; // size = 2^sizeLog2 
    PID_TYPE pos;
} UDBIndexEntry;

typedef struct
{
    JFile *file;
    List *holes;
    PID_TYPE maxPid;
} UDBIndex;

UDBIndex *UDBIndex_new(void);
void UDBIndex_free(UDBIndex *self);

JFile *UDBIndex_jfile(UDBIndex *self);

void UDBIndex_setPath_(UDBIndex *self, const char *s);
void UDBIndex_setLogPath_(UDBIndex *self, const char *s);
char *UDBIndex_path(UDBIndex *self);

void UDBIndex_delete(UDBIndex *self);
void UDBIndex_open(UDBIndex *self);
void UDBIndex_finishOpening(UDBIndex *self);
void UDBIndex_close(UDBIndex *self);

void UDBIndex_begin(UDBIndex *self);
void UDBIndex_preCommit(UDBIndex *self);
void UDBIndex_commit(UDBIndex *self);

// holes ------------------------------------------- 

void UDBIndex_findHoles(UDBIndex *self);

// pid ops ------------------------------------------- 

PID_TYPE UDBIndex_nextPid(UDBIndex *self);
PID_TYPE UDBIndex_allocPid(UDBIndex *self);
PID_TYPE UDBIndex_posForPid_(UDBIndex *self, PID_TYPE pid);
void UDBIndex_setPos_forPid_(UDBIndex *self, PID_TYPE pos, PID_TYPE pid);
long UDBIndex_pidCount(UDBIndex *self);
void UDBIndex_show(UDBIndex *self);

#ifdef __cplusplus
}
#endif
#endif
