/*#io
SkipDBCursor ioDoc(
			    docCopyright("Steve Dekorte", 2004)
			    docLicense("BSD revised")
			    docObject("SkipDBCursor")
			    docDescription("A cursor for a skipdb.")
			    */

#include "SkipDBCursor.h"

SkipDBCursor *SkipDBCursor_new(void)
{
	SkipDBCursor *self = (SkipDBCursor *)calloc(1, sizeof(SkipDBCursor));
	SkipDBCursor_retain(self);
	return self;
}

SkipDBCursor *SkipDBCursor_newWithDB_(SkipDB *sdb)
{
	SkipDBCursor *self = SkipDBCursor_new();
	self->sdb = sdb;
	return self;
}

void SkipDBCursor_retain(SkipDBCursor *self)
{
	self->refCount ++;
}

void SkipDBCursor_dealloc(SkipDBCursor *self)
{
	if (self->sdb) SkipDB_removeCursor_(self->sdb, self);
	free(self);
}

void SkipDBCursor_release(SkipDBCursor *self)
{
	self->refCount --;

	if (self->refCount == 0)
	{
		SkipDBCursor_dealloc(self);
	}
}

void SkipDBCursor_mark(SkipDBCursor *self)
{
	if (self->record)
	{
		SkipDBRecord_mark(self->record);
	}
}

SkipDBRecord *SkipDBCursor_goto_(SkipDBCursor *self, Datum key)
{
	return (self->record = SkipDB_goto_((SkipDB *)(self->sdb), key));
}

SkipDBRecord *SkipDBCursor_first(SkipDBCursor *self)
{
	return (self->record = SkipDB_firstRecord(self->sdb));
}

SkipDBRecord *SkipDBCursor_last(SkipDBCursor *self)
{
	return (self->record = SkipDB_lastRecord(self->sdb));
}

SkipDBRecord *SkipDBCursor_current(SkipDBCursor *self)
{
	return self->record;
}

SkipDBRecord *SkipDBCursor_previous(SkipDBCursor *self)
{
	if (self->record)
	{
		self->record = SkipDBRecord_previousRecord(self->record);

		if (self->record == SkipDB_headerRecord(self->sdb))
		{
			self->record = NULL;
		}
	}
	return self->record;
}

SkipDBRecord *SkipDBCursor_next(SkipDBCursor *self)
{
	if (!self->record)
	{
		return NULL;
	}

	return (self->record = SkipDBRecord_nextRecord(self->record));
}

