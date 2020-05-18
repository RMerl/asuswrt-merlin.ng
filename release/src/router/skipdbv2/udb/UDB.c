/*#io
UDB ioDoc(
		docCopyright("Steve Dekorte", 2004)
		docLicense("BSD revised")
		docObject("UDB")    
		docDescription("")
		*/

#include "UDB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

UDB *UDB_new(void)
{
	UDB *self = (UDB *)calloc(1, sizeof(UDB));
	
	self->index       = UDBIndex_new();
	self->records     = UDBRecords_new();
	
	UDB_setPath_(self, "default");
	return self;
}

void UDB_free(UDB *self)
{
	UDB_close(self);
	free(self->path);    
	free(self->committedPath);    
	
	UDBIndex_free(self->index);   
	UDBRecords_free(self->records);   
	free(self);
}

void UDB_delete(UDB *self)
{
	UDB_close(self);
	UDBIndex_delete(self->index);
	UDBRecords_delete(self->records);
	remove(self->committedPath);
}

void UDB_setPath_(UDB *self, const char *path)
{
	self->path = strcpy((char *)realloc(self->path, strlen(path)+1), path);
	self->committedPath = strcpy((char *)realloc(self->committedPath, strlen(path)+20), path);
	strcat(self->committedPath, ".committed");
	
	UDBIndex_setPath_(self->index, path);
	UDBRecords_setPath_(self->records, path);
}

/*
void UDB_setSecondaryPath_(UDB *self, const char *secondaryPath)
{
	UDBIndex_setPath_(self->index, self->path);
	UDBIndex_setLogPath_(self->index, secondaryPath);
	
	UDBRecords_setPath_(self->records, secondaryPath);
	UDBRecords_setLogPath_(self->records, self->path);
}
*/


char *UDB_path(UDB *self) 
{ 
	return self->path; 
}

void UDB_open(UDB *self)
{
	self->committedFile = fopen(self->committedPath, "r+");

	if (!self->committedFile) 
	{
		self->committedFile = fopen(self->committedPath, "w");
		fclose(self->committedFile);
		self->committedFile = fopen(self->committedPath, "r+");
	}
	
	
	if (UDB_readCommitted(self) == 0)
	{
		JFile_clipLog(self->index->file);
		JFile_clipLog(self->records->file);
	}
	
	UDBIndex_open(self->index);
	UDBRecords_open(self->records);
	self->isOpen = 1;
}

int UDB_isOpen(UDB *self)
{
	return self->isOpen;
}

void UDB_close(UDB *self)
{
        if(self->isOpen) {
                UDBIndex_close(self->index);
                UDBRecords_close(self->records);
                if (self->committedFile) fclose(self->committedFile);
                self->isOpen = 0;
        }
}

// transactions --------------------------------------------------- 

int UDB_isInTransaction(UDB *self)
{
	if (!self->withinTransaction)
	{
		printf("UDB error - mutation operations must be within a transaction\n");
		exit(1);
		return 0;
	}
	return 1;
}

void UDB_beginTransaction(UDB *self)
{
	UDBIndex_begin(self->index);
	UDBRecords_begin(self->records);
	self->withinTransaction = 1;
}

int UDB_readCommitted(UDB *self)
{
	fseek(self->committedFile, 0, SEEK_SET);
	return fgetc(self->committedFile);
}

void UDB_setCommittedFlag_(UDB *self, int flag)
{
	fseek(self->committedFile, 0, SEEK_SET);
	fputc(flag, self->committedFile);
	#ifdef F_FULLFSYNC
		fcntl(fileno(self->committedFile), F_FULLFSYNC, NULL);
	#else
		#warning Linux doesn't support syncing to physical media
		fsync(fileno(self->committedFile));
	#endif
}

void UDB_commitTransaction(UDB *self)
{
	UDBIndex_preCommit(self->index);
	UDBRecords_preCommit(self->records);
	
	UDB_setCommittedFlag_(self, 1);
	
	UDBIndex_commit(self->index);
	UDBRecords_commit(self->records);
	
	UDB_setCommittedFlag_(self, 0);
	
	self->withinTransaction = 0;
}

// ops --------------------------------------------------- 

PID_TYPE UDB_nextPid(UDB *self)
{
	return UDBIndex_nextPid(self->index);
}

PID_TYPE UDB_allocPid(UDB *self)
{
	return UDBIndex_allocPid(self->index);
}

void UDB_append_withPid_(UDB *self, Datum d, PID_TYPE pid) 
{
	UDBRecord *record = UDBRecords_newRecord(self->records);
	UDBRecord_pid_(record, pid);
	UDBRecord_saveWithDatum_(record, d);
	UDBIndex_setPos_forPid_(self->index, UDBRecord_pos(record), pid);
}

PID_TYPE UDB_put_(UDB *self, Datum d) // returns pid 
{
	if (UDB_isInTransaction(self))
	{
		PID_TYPE pid = UDBIndex_allocPid(self->index);
		UDB_append_withPid_(self, d, pid);
		return pid;
	}
	
	return 0;
}

void UDB_at_put_(UDB *self, PID_TYPE pid, Datum d)
{
	if (UDB_isInTransaction(self))
	{
		UDB_append_withPid_(self, d, pid);
	}
}

UDBRecord *UDB_recordAtPid_(UDB *self, PID_TYPE pid)
{
	PID_TYPE pos = UDBIndex_posForPid_(self->index, pid);
	if (!pos) return NULL;
	return UDBRecords_recordAtPos_(self->records, pos);
}

Datum UDB_at_(UDB *self, PID_TYPE pid)
{
	Datum d;
	UDBRecord *record = UDB_recordAtPid_(self, pid);
	
	if (!record)
	{
		d.size = 0;
		d.data = NULL;
		//printf("missing record with pid %i\n", pid);
		return d;
	}
	
	return UDBRecord_readDatum(record);
}

void UDB_removeAt_(UDB *self, PID_TYPE pid)
{
	if (UDB_isInTransaction(self))
	{
		UDBRecord *record = UDB_recordAtPid_(self, pid);
		
		if (!record)
		{
                        //TODO jannson
			//printf("UDB error: missing record with pid %" PID_FORMAT " for remove\n", pid);
			record = UDB_recordAtPid_(self, pid);
			return;
		}
		
		UDBRecords_removeRecord_(self->records, record);
		UDBIndex_setPos_forPid_(self->index, 0, pid);
	}
}

// compact --------------------------------------------------- 

UDBRecord *UDB_firstEmptyRecord(UDB *self)
{
	UDBRecord *record = UDBRecords_firstRecord(self->records);
	
	while (record)
	{	
		if (UDBRecord_isEmpty(record)) return record;
		record = UDBRecords_nextRecord(self->records);
	}
	return NULL;
}

int UDB_compact(UDB *self)
{
	int count = 0;
	int compactions;
	
	do 
	{
		compactions = UDB_compactStepFor_(self, 0.1);
		count += compactions;
	} while (compactions);
	
	return count;
}

//#define DEBUG 1

int UDB_compactStep(UDB *self)
{
	return UDB_compactStepFor_(self, 0.0);
}

int UDB_compactStepFor_(UDB *self, double maxSeconds)
{   
	UDBRecord *firstEmptyRecord = UDBRecords_firstEmptyRecord(self->records);
	
	if (!firstEmptyRecord)
	{
		return 0;
	}
	
	if (!UDBRecord_isEmpty(firstEmptyRecord))
	{
		printf("firstEmptyRecord not empty!\n");
		firstEmptyRecord = UDBRecords_firstEmptyRecord(self->records);
		UDBRecord_isEmpty(firstEmptyRecord);
		exit(1);
	}
	
#ifdef DEBUG
	printf("empty record pid: %i pos: %i size: %i isEmpty: %i\n", 
		  UDBRecord_pid(firstEmptyRecord), 
		  UDBRecord_pos(firstEmptyRecord), 
		  UDBRecord_totalSize(firstEmptyRecord), 
		  UDBRecord_isEmpty(firstEmptyRecord));
#endif
	    
	if (firstEmptyRecord)
	{
		UDBRecord *record = UDBRecords_recordAfter_(self->records, firstEmptyRecord);
		
		if (!record)
		{
			// reached end of file 
			PID_TYPE size = UDBRecord_pos(firstEmptyRecord);
			UDBRecords_truncate_(self->records, size);
			return 0;
		}
		
#ifdef DEBUG
		printf("next  record pid: %i pos: %i size: %i isEmpty: %i\n\n", 
			  UDBRecord_pid(record), 
			  UDBRecord_pos(record), 
			  UDBRecord_totalSize(record), 
			  UDBRecord_isEmpty(record));
#endif
		
		UDB_beginTransaction(self);
		
		if (UDBRecord_isEmpty(record))
		{
			// coalese this empty record into the first empty record 
			PID_TYPE newSize = UDBRecord_totalSize(firstEmptyRecord) 
			- sizeof(UDBRecordHeader) + UDBRecord_totalSize(record);
			UDBRecord_size_(firstEmptyRecord, newSize);
			UDBRecord_saveHeader(firstEmptyRecord);
#ifdef DEBUG
			printf("merge\n");
#endif
			UDBIndex_setPos_forPid_(self->index, 0, UDBRecord_pid(record));
		}
		else
		{
			// swap places with the first empty record 
			PID_TYPE oldEmptyPos = UDBRecord_pos(firstEmptyRecord);
			PID_TYPE newEmptyPos = oldEmptyPos + UDBRecord_totalSize(record);
			UDBRecords_firstEmptyRecordPos_(self->records, newEmptyPos);
			UDBRecord_moveToPos_(record, UDBRecord_pos(firstEmptyRecord));
			UDBRecord_setPos_(firstEmptyRecord, newEmptyPos);
			UDBRecord_saveHeader(firstEmptyRecord);
#ifdef DEBUG
			printf("move\n");
#endif
			UDBIndex_setPos_forPid_(self->index, newEmptyPos, UDBRecord_pid(firstEmptyRecord));
			UDBIndex_setPos_forPid_(self->index, oldEmptyPos, UDBRecord_pid(record));
		}
		
		UDB_commitTransaction(self);
		
		return 1;
	}
	return 0;
}

void UDB_show(UDB *self)
{
	UDBRecord *record = UDBRecords_firstRecord(self->records);
	printf("UDB Records:\n");
	
	while (record)
	{	
		UDBRecord_show(record);
		record = UDBRecords_nextRecord(self->records);
	}
}

void UDB_showIndex(UDB *self)
{
	UDBIndex_show(self->index);
}

