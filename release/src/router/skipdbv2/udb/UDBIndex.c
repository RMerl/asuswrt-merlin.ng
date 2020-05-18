/*#io
UDBIndex ioDoc(
			docCopyright("Steve Dekorte", 2004)
			docLicense("BSD revised")
			docObject("UDBIndex")    
			docDescription("")
			*/

#include "UDBIndex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UDBIndex *UDBIndex_new(void)
{
	UDBIndex *self = (UDBIndex *)calloc(1, sizeof(UDBIndex));
	self->file = JFile_new();
	UDBIndex_setPath_(self, "default");
	self->holes = List_new();
	return self;
}

void UDBIndex_free(UDBIndex *self)
{
	UDBIndex_close(self);
	JFile_free(self->file);
	List_free(self->holes);
	free(self);
}

JFile *UDBIndex_jfile(UDBIndex *self)
{
	return self->file;
}

void UDBIndex_delete(UDBIndex *self)
{
	JFile_close(self->file);
	JFile_delete(self->file);
}

void UDBIndex_setPath_(UDBIndex *self, const char *path)
{
	JFile_setPath_withExtension_(self->file, path, "udbIndex");
}

void UDBIndex_setLogPath_(UDBIndex *self, const char *s)
{
	JFile_setLogPath_(self->file, s);
}

char *UDBIndex_path(UDBIndex *self) 
{ 
	return JFile_path(self->file); 
}

void UDBIndex_open(UDBIndex *self)
{
	JFile_open(self->file);
	// need to also call UDBIndex_finishOpening
	UDBIndex_finishOpening(self);
	self->maxPid = 0;
}

void UDBIndex_finishOpening(UDBIndex *self)
{
	JFile_begin(self->file);
	UDBIndex_setPos_forPid_(self, 0, 0);
	UDBIndex_preCommit(self);
	UDBIndex_findHoles(self);
}

void UDBIndex_close(UDBIndex *self)
{
	JFile_close(self->file);
}

void UDBIndex_begin(UDBIndex *self)
{
	JFile_begin(self->file);
}

void UDBIndex_preCommit(UDBIndex *self)
{
	JFile_commitToLog(self->file);
}

void UDBIndex_commit(UDBIndex *self)
{
	JFile_commitToFile(self->file);
}

// holes ------------------------------------------- 

void UDBIndex_findHoles(UDBIndex *self)
{
	PID_TYPE pid = UDBIndex_pidCount(self);
	
	List_removeAll(self->holes);
	
	while (pid --)
	{
		if (!UDBIndex_posForPid_(self, pid)) 
		{
			List_push_(self->holes, (void *)pid);
		}
	}
}

// pid ops ------------------------------------------- 

PID_TYPE UDBIndex_nextPid(UDBIndex *self)
{
	PID_TYPE hole = (PID_TYPE)List_top(self->holes);
	
	if (hole) 
	{
		return hole;
	}
	
	return JFile_setPositionToEnd(self->file) / sizeof(UDBIndexEntry);
}

PID_TYPE UDBIndex_allocPid(UDBIndex *self)
{
	PID_TYPE hole = (PID_TYPE)List_pop(self->holes);
	
	if (hole) 
	{
		return hole;
	}
	else
	{
		PID_TYPE max = JFile_setPositionToEnd(self->file) / sizeof(UDBIndexEntry);
		
		if (max > self->maxPid)
		{
			self->maxPid = max;
		}
		else
		{
			self->maxPid ++;
		}
		
		return self->maxPid;
	}
}

PID_TYPE UDBIndex_posForPid_(UDBIndex *self, PID_TYPE pid)
{
	UDBIndexEntry entry;
	
	JFile_setPosition_(self->file, pid * sizeof(UDBIndexEntry));
	
	//#ifdef JFILE_SUPPORTS_MMAP
	//JFile_mmapfread(self->file, (unsigned char *)(&pos), sizeof(UDBIndexEntry), 1);
	//#else
	JFile_fread(self->file, (unsigned char *)(&entry), sizeof(UDBIndexEntry), 1);    
	//#endif
	
	return entry.pos;
}

void UDBIndex_setPos_forPid_(UDBIndex *self, PID_TYPE pos, PID_TYPE pid)
{
	UDBIndexEntry entry;
	entry.pos = pos;
	//entry.size = 0;
	
	JFile_setPosition_(self->file, pid * sizeof(UDBIndexEntry));
	JFile_fwrite(self->file, (unsigned char *)(&entry), sizeof(UDBIndexEntry), 1);
	
	if (pos == 0)
	{
		List_push_(self->holes, (void *)pid);
	}
}

long UDBIndex_pidCount(UDBIndex *self)
{
	return JFile_setPositionToEnd(self->file) / sizeof(UDBIndexEntry);
}

void UDBIndex_show(UDBIndex *self)
{
	PID_TYPE pid, maxPid = UDBIndex_pidCount(self);
	
	printf("UDBIndex: (pid/pos)\n\n");
	
	for (pid = 0; pid < maxPid; pid ++)
	{
		PID_TYPE pos = UDBIndex_posForPid_(self, pid);
		printf("  %i -> %i\n", (int)pid, (int)pos);
	}
	
	printf("\n");
}

