/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDB")    
docDescription("An unordered value database. (sort of like malloc for disk space)
    It compacts the data like a single space copying garbage collector.")
*/

#ifndef UDB_DEFINED
#define UDB_DEFINED 1

#if !defined(__MINGW32__) && defined(WIN32)
#if defined(BUILDING_SKIPDB_DLL) || defined(BUILDING_IOVMALL_DLL)
#define UDB_API __declspec(dllexport)
#else
#define UDB_API __declspec(dllimport)
#endif
#else
#define UDB_API
#endif


#include "List.h"
#include "UDBRecord.h"
#include "UDBIndex.h"
#include "UDBRecords.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char *path;
    UDBIndex *index;
    UDBRecords *records;
    int withinTransaction;
    int isOpen;
	FILE *committedFile;
	char *committedPath;
} UDB;

UDB_API UDB *UDB_new(void);
UDB_API void UDB_free(UDB *self);

UDB_API void UDB_setPath_(UDB *self, const char *s);
UDB_API void UDB_setLogPath_(UDB *self, const char *s);
UDB_API char *UDB_path(UDB *self);

UDB_API void UDB_delete(UDB *self);
UDB_API void UDB_open(UDB *self);
UDB_API int UDB_isOpen(UDB *self);
UDB_API void UDB_close(UDB *self);

// transactions --------------------------------------------------- 

UDB_API void UDB_beginTransaction(UDB *self);
UDB_API void UDB_commitTransaction(UDB *self);

// ops -------------------------------------------------- 

UDB_API PID_TYPE UDB_nextPid(UDB *self);
UDB_API PID_TYPE UDB_allocPid(UDB *self);

UDB_API PID_TYPE UDB_put_(UDB *self, Datum d);
UDB_API void UDB_at_put_(UDB *self, PID_TYPE pid, Datum d);
UDB_API Datum UDB_at_(UDB *self, PID_TYPE pid);
UDB_API void UDB_removeAt_(UDB *self, PID_TYPE id);

UDB_API int UDB_compact(UDB *self);
UDB_API int UDB_compactStep(UDB *self);
UDB_API int UDB_compactStepFor_(UDB *self, double maxSeconds);

UDB_API void UDB_show(UDB *self);
UDB_API void UDB_showIndex(UDB *self);

#ifdef __cplusplus
}
#endif
#endif
