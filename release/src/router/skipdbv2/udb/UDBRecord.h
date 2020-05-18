/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDBRecord")    
docDescription("An individual UDB Record.")
*/

#ifndef UDBRecord_DEFINED
#define UDBRecord_DEFINED 1

#include <stdio.h>
#include "JFile.h"
#include "Datum.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_TAG_NORMAL 0
#define RECORD_TAG_EMPTY  1

typedef struct
{
    //unsigned char recordSizeLog2; 
    unsigned char tag;
    PID_TYPE pid;
    PID_TYPE size; // the actual size of the datum 
} UDBRecordHeader;

typedef struct
{
    JFile *file;
    PID_TYPE pos;
    UDBRecordHeader header;
    UArray *data;
} UDBRecord;

UDBRecord *UDBRecord_new(void);
UDBRecord *UDBRecord_clone(UDBRecord *self);
void UDBRecord_free(UDBRecord *self);

void UDBRecord_setJFile_(UDBRecord *self, JFile *file);

void UDBRecord_setPos_(UDBRecord *self, PID_TYPE pos);
void UDBRecord_setPosToEnd(UDBRecord *self);
PID_TYPE UDBRecord_pos(UDBRecord *self);

void UDBRecord_pid_(UDBRecord *self, PID_TYPE pid);
PID_TYPE UDBRecord_pid(UDBRecord *self);

void UDBRecord_size_(UDBRecord *self, PID_TYPE size);
PID_TYPE UDBRecord_size(UDBRecord *self);

void UDBRecord_remove(UDBRecord *self);

void UDBRecord_saveHeader(UDBRecord *self);
void UDBRecord_saveWithDatum_(UDBRecord *self, Datum d);

int UDBRecord_readHeader(UDBRecord *self);
Datum UDBRecord_readDatum(UDBRecord *self);

int UDBRecord_isEmpty(UDBRecord *self);
//int UDBRecord_readNextHeader(UDBRecord *self);
void UDBRecord_moveToPos_(UDBRecord *self, PID_TYPE newPos);

PID_TYPE UDBRecord_totalSize(UDBRecord *self);
void UDBRecord_show(UDBRecord *self);

#ifdef __cplusplus
}
#endif
#endif
