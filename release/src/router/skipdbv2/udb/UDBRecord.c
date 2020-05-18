/*#io
UDBRecord ioDoc(
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("UDBRecord")    
docDescription("")
*/

#include "UDB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UDBRecord *UDBRecord_new(void)
{
    UDBRecord *self = (UDBRecord *)calloc(1, sizeof(UDBRecord));
    return self;
}

UDBRecord *UDBRecord_clone(UDBRecord *self)
{
    UDBRecord *clone = (UDBRecord *)cpalloc(self, sizeof(UDBRecord));
    return clone;
}

void UDBRecord_free(UDBRecord *self)
{
    if (self->data) UArray_free(self->data);
    free(self);
}

UArray *UDBRecord_data(UDBRecord *self)
{
    if (!self->data) self->data = UArray_new();
    return self->data;
}

void UDBRecord_setJFile_(UDBRecord *self, JFile *file)
{ 
    self->file = file;
}

void UDBRecord_setPos_(UDBRecord *self, PID_TYPE pos)
{   
    self->pos = pos;
}

PID_TYPE UDBRecord_pos(UDBRecord *self)
{
    return self->pos;
}

void UDBRecord_setPosToEnd(UDBRecord *self)
{
    self->pos = JFile_setPositionToEnd(self->file);
}

void UDBRecord_pid_(UDBRecord *self, PID_TYPE pid)
{
    self->header.pid = pid;
}

PID_TYPE UDBRecord_pid(UDBRecord *self)
{
    return self->header.pid;
}

void UDBRecord_size_(UDBRecord *self, PID_TYPE size)
{
    self->header.size = size;
}

PID_TYPE UDBRecord_size(UDBRecord *self)
{
    return self->header.size;
}

void UDBRecord_remove(UDBRecord *self)
{
    self->header.tag = RECORD_TAG_EMPTY;
    UDBRecord_saveHeader(self);
}

int UDBRecord_isEmpty(UDBRecord *self)
{
    return (self->header.tag == RECORD_TAG_EMPTY);
}


// save ----------------------------------------- 

void UDBRecord_saveHeader(UDBRecord *self) 
{ 
    if (self->pos) JFile_setPosition_(self->file, self->pos);
    JFile_fwrite(self->file, (unsigned char *)(&(self->header)), sizeof(UDBRecordHeader), 1);
}

void UDBRecord_saveWithDatum_(UDBRecord *self, Datum d)
{
    self->header.size = d.size;
    UDBRecord_saveHeader(self);
    JFile_fwrite(self->file, d.data, d.size, 1);
}

// read ----------------------------------------- 

int UDBRecord_readHeader(UDBRecord *self) 
{ 
    int objectsRead;
    
    JFile_setPosition_(self->file, self->pos);
    objectsRead = JFile_fread(self->file, (unsigned char *)(&(self->header)), sizeof(UDBRecordHeader), 1);
    
    if (objectsRead != 1)
    {
	self->header.pid = 0;
	self->header.size = 0;
	return 0;
    }
    
    return 1;
}

Datum UDBRecord_readDatum(UDBRecord *self)
{
    UArray *ba = UDBRecord_data(self);
    Datum d;
    PID_TYPE size = self->header.size;
    
    JFile_fseek(self->file, self->pos + sizeof(UDBRecordHeader), SEEK_SET);
    
    UArray_setSize_(ba, size);
    JFile_fread(self->file, UArray_bytes(ba), size, 1);
    d.size = size;
    d.data = UArray_bytes(ba);
    return d;
}

// compact ----------------------------------------- 

void UDBRecord_moveToPos_(UDBRecord *self, PID_TYPE newPos)
{
    PID_TYPE totalSize = UDBRecord_totalSize(self);
    unsigned char *data;
    UArray *ba = UDBRecord_data(self);
    UArray_setSize_(ba, totalSize);
    data = UArray_bytes(ba); 
       
    JFile_fseek(self->file, self->pos, SEEK_SET);
    JFile_fread(self->file, data, totalSize, 1);
    
    JFile_fseek(self->file, newPos, SEEK_SET);
    JFile_fwrite(self->file, data, totalSize, 1);
}

PID_TYPE UDBRecord_totalSize(UDBRecord *self)
{
    return sizeof(UDBRecordHeader) + self->header.size;
}

void UDBRecord_show(UDBRecord *self)
{
    printf("Record pos: %" PID_FORMAT "\n", self->pos);
    printf("  pid: %" PID_FORMAT "\n", self->header.pid);
    printf("  size: %" PID_FORMAT "\n", self->header.size);
    printf("\n");
}


