/*#io
SkipDB ioDoc(
		   docCopyright("Steve Dekorte", 2004)
		   docLicense("BSD revised")
		   docObject("SkipDB")
		   docDescription("BerkeleyDB style database implemented with skip lists instead of a b-tree.")
		   */

#include "SkipDB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// lookups

void SkipDB_clearUpdate(SkipDB *self);

// ops

void SkipDB_readRootRecord(SkipDB *self);

// ----------------------------------------------------------

SkipDB *SkipDB_new(void)
{
	SkipDB *self = (SkipDB *)calloc(1, sizeof(SkipDB));
	self->udb = UDB_new();

	self->p = SKIPDB_PROBABILITY_DISTRIBUTION;
	self->stream = BStream_new();

	self->cursors = List_new();

	self->pidsToRemove = List_new();
	self->dirtyRecords = List_new();

	self->cacheHighWaterMark = 10000;
	self->cacheLowWaterMark  = 500;

	self->pidToRecord = PHash_new();
	self->randomGen = RandomGen_new();

	/*
    self->header = SkipDBRecord_newWithDB_(self);
    SkipDBRecord_keyDatum_(self->header, Datum_Empty());
    SkipDBRecord_valueDatum_(self->header, Datum_Empty());
	self->headerPid = 0;
	*/

        self->headerPid = 1;

	return self;
}

void SkipDB_dealloc(SkipDB *self)
{
	SkipDB_clearUpdate(self);

	BStream_free(self->stream);
	SkipDB_freeAllCachedRecords(self);

	// cursors are allocated and must be freed individually.
	List_do_(self->cursors, (ListDoCallback *)SkipDBCursor_release);
	List_free(self->cursors);

        UDB_free(self->udb);
        self->udb = NULL;

	List_free(self->pidsToRemove);
	List_free(self->dirtyRecords);
	PHash_free(self->pidToRecord);
	RandomGen_free(self->randomGen);
	free(self);
}

void SkipDB_free(SkipDB *self)
{
	SkipDB_dealloc(self);
}

void SkipDB_setPath_(SkipDB *self, char *path)
{
	UDB_setPath_(self->udb, path);
}

SKIPDB_API int SkipDB_open(SkipDB *self)
{
    /*
	Datum key;
	Datum value;
    */

	UDB_open(self->udb);

	SkipDB_readRootRecord(self);

#if 0
        if(0 == self->headerPid) {
	    SkipDB_beginTransaction(self);
            key = Datum_FromCString_("__inner__");
            value = Datum_FromCString_("init");
	    SkipDB_at_put_(self, key, value);
	    SkipDB_commitTransaction(self);
	    SkipDBRecord_markAsDirty(self->header);
        }
#endif

        return self->headerPid;
}

SKIPDB_API void SkipDB_close(SkipDB *self)
{
	UDB_close(self->udb);
}

void SkipDB_headerPid_(SkipDB *self, PID_TYPE pid)
{
	self->headerPid = pid;
}

PID_TYPE SkipDB_headerPid(SkipDB *self)
{
	return self->headerPid;
}

SkipDBRecord *SkipDB_headerRecord(SkipDB *self)
{
	return self->header;
}

UDB *SkipDB_udb(SkipDB *self)
{
	return self->udb;
}

BStream *SkipDB_tmpStream(SkipDB *self)
{
	return self->stream;
}

void SkipDB_delete(SkipDB *self)
{
	SkipDB_close(self);
	UDB_delete(self->udb);
/*
	int count = 0;
	SkipDBRecord *r = self->header;

	while (r)
	{
		PID_TYPE pid = SkipDBRecord_pid(r);
		SkipDBRecord *next = SkipDBRecord_nextRecord(self->header);

		if (pid)
		{
			UDB_removeAt_(SkipDB_udb(self), pid);
		}

		r = next;
		count ++;
	}

	return count;
	*/
}

// notifications ---------------------------------

void SkipDB_noteNewRecord_(SkipDB *self, SkipDBRecord *r)
{
	self->cachedRecordCount ++;
	SkipDB_noteAccessedRecord_(self, r);
}

void SkipDB_noteAccessedRecord_(SkipDB *self, SkipDBRecord *r)
{
	SkipDBRecord *y = self->youngestRecord;

	//SkipDBRecord_showAgeList(r);

	if (y != r)
	{
		if (y)
		{
			SkipDBRecord_removeFromAgeList(r);
			SkipDBRecord_setOlderRecord_(r, y);
			SkipDBRecord_setYoungerRecord_(y, r);
		}

		self->youngestRecord = r;
	}

	//SkipDBRecord_showAgeList(r);
}

void SkipDB_noteDirtyRecord_(SkipDB *self, SkipDBRecord *r)
{
	if (SkipDB_isOpen(self))
	{
		List_append_(self->dirtyRecords, r);
	}
}

void SkipDB_noteAssignedPidToRecord_(SkipDB *self, SkipDBRecord *r)
{
	PHash_at_put_(self->pidToRecord, (void *)SkipDBRecord_pid(r), r);
}

void SkipDB_noteWillFreeRecord_(SkipDB *self, SkipDBRecord *r)
{
	SkipDBFreeObjectFunc *f = self->objectFreeFunc;
	void *object = SkipDBRecord_object(r);

	if (f && object)
	{
		(*f)(object);
	}

	self->cachedRecordCount --;

	if (r == self->youngestRecord)
	{
		self->youngestRecord = SkipDBRecord_olderRecord(r);
		//printf("will free youngestRecord\n");
	}

	SkipDBRecord_removeFromAgeList(r);
	PHash_removeKey_(self->pidToRecord, (void *)SkipDBRecord_pid(r));
}

void SkipDB_freeAllCachedRecords(SkipDB *self)
{
	//SkipDBRecord *r = self->youngestRecord;
	//self->headerPid = SkipDBRecord_pid(self->header);

	//SkipDBRecord_showAgeList(self->youngestRecord);

	while (self->youngestRecord)
	{
		SkipDBRecord_dealloc(self->youngestRecord);
	}

	self->header = NULL;
}

// cache -------------------------------------

void SkipDB_setCacheHighWaterMark_(SkipDB *self, size_t recordCount)
{
	self->cacheHighWaterMark = recordCount;

	if (recordCount < self->cacheLowWaterMark)
	{
		self->cacheLowWaterMark = recordCount / 2;
	}
}

size_t SkipDB_cacheHighWaterMark(SkipDB *self)
{
	return self->cacheHighWaterMark;
}

void SkipDB_setCacheLowWaterMark_(SkipDB *self, size_t recordCount)
{
	self->cacheLowWaterMark = recordCount;

	if (recordCount > self->cacheHighWaterMark)
	{
		self->cacheHighWaterMark = recordCount * 2;
	}
}

size_t SkipDB_cacheLowWaterMark(SkipDB *self)
{
	return self->cacheLowWaterMark;
}

int SkipDB_headerIsEmpty(SkipDB *self)
{
	return SkipDBRecord_pointersAreEmpty(self->header);
}

void SkipDB_freeExcessCachedRecords(SkipDB *self)
{
	// this is only called after a sync, so there are no dirty records
	//return;

	if (self->cachedRecordCount > self->cacheHighWaterMark)
	{
		SkipDBRecord *r;
		SkipDBRecord *lastMarkedRecord = NULL;
		size_t lowMark = self->cacheLowWaterMark;

		//printf("SkipDB_freeExcessCachedRecords() start %i\n", (int)self->cachedRecordCount);

		// make sure we keep the header

		SkipDB_noteAccessedRecord_(self, self->header);
		r = self->youngestRecord;

		// mark the cursor records

		List_do_(self->cursors, (ListDoCallback *)SkipDBCursor_mark);

		// mark the recent records

		while (lowMark --)
		{
			r->mark = 1;
			r = r->olderRecord;
		}

		// unmark the rest of the records

		while (r)
		{
			r->mark = 0;
			r = r->olderRecord;
		}

		// remove the references to older records

		lowMark = self->cacheLowWaterMark;
		r = self->youngestRecord;

		while (r->mark)
		{
			lowMark --;
			SkipDBRecord_removeReferencesToUnmarked(r);
			lastMarkedRecord = r; // remember this so we can clip the list
			r = r->olderRecord;
		}

		// dealloc the remaining records

		while (r)
		{
			SkipDBRecord *next = r->olderRecord;

			// to avoid SkipDBRecord_removeFromAgeList() issues
			r->olderRecord   = NULL;
			r->youngerRecord = NULL;

			if (r->mark)
			{
				printf("error - attempt to dealloc marked record\n");
				exit(-1);
			}

			SkipDBRecord_dealloc(r); // what about SkipDBRecord_removeFromAgeList()?
			r = next;
		}

		lastMarkedRecord->olderRecord = NULL; // clip the list

		//printf("SkipDB_freeExcessCachedRecords() done %i\n", (int)self->cachedRecordCount);
	}
}

int SkipDB_isOpen(SkipDB *self)
{
	return SkipDB_udb(self) && UDB_isOpen(SkipDB_udb(self));
}

void SkipDB_clearCache(SkipDB *self)
{
	printf("\nclearCache\n\n");

	if (self->header)
	{
		SkipDB_freeAllCachedRecords(self);
		SkipDB_readRootRecord(self);
	}
}

// transactions ---------------------------------------------------

void SkipDB_sync(SkipDB *self)
{
	if (SkipDB_isOpen(self))
	{
		if (!self->headerPid)
		{
			self->headerPid = SkipDBRecord_pidAllocIfNeeded(self->header);
		}

		//SkipDB_removeDirtyRecordsFromSavedRecords(self);
		SkipDB_saveDirtyRecords(self);
		SkipDB_deleteRecordsToRemove(self);
	}

	List_removeAll(self->dirtyRecords);
	List_removeAll(self->pidsToRemove);
	SkipDB_freeExcessCachedRecords(self);
}

SKIPDB_API void SkipDB_beginTransaction(SkipDB *self)
{
	UDB_beginTransaction(self->udb);
}

SKIPDB_API void SkipDB_commitTransaction(SkipDB *self)
{
	SkipDB_sync(self);
	UDB_commitTransaction(self->udb);
}

void SkipDB_saveDirtyRecords(SkipDB *self)
{
	List_do_(self->dirtyRecords, (ListDoCallback *)SkipDBRecord_save);
}

void SkipDB_deleteRecordsToRemove(SkipDB *self)
{
	UDB *udb = SkipDB_udb(self);
	LIST_FOREACH(self->pidsToRemove, i, pid, UDB_removeAt_(udb, (PID_TYPE)pid));
}

// ops --------------------------------------------------

void SkipDB_readRootRecord(SkipDB *self)
{
	SkipDBRecord *r = SkipDB_recordAtPid_(self, self->headerPid);

	if (!r)
	{
		r = SkipDBRecord_newWithDB_(self);
		SkipDBRecord_keyDatum_(r, Datum_Empty());
		SkipDBRecord_markAsDirty(r);
		self->headerPid = SkipDBRecord_pid(r);
	}

	self->header = r;
}

int SkipDB_level(SkipDB *self)
{
	return SkipDBRecord_level(self->header);
}

void SkipDB_level_(SkipDB *self, int level)
{
	SkipDBRecord_level_(self->header, level);
	SkipDBRecord_markAsDirty(self->header);
}

int SkipDB_pickRandomRecordLevel(SkipDB *self)
{
	float r = (float)RandomGen_randomDouble(self->randomGen);
	int level = 1;

	while (r < self->p && level < SKIPDB_MAX_LEVEL)
	{
		level ++;
		r = (float)RandomGen_randomDouble(self->randomGen);
	}

	if (level > SkipDB_level(self))
	{
		level = SkipDB_level(self) + 1;
	}

	return level;
}

SkipDBRecord *SkipDB_recordAtPid_(SkipDB *self, PID_TYPE pid)
{
	if (pid)
	{
		SkipDBRecord *r = PHash_at_(self->pidToRecord, (void *)pid);

		if (r)
		{
			return r;
		}
		else
		{
			Datum d = UDB_at_(SkipDB_udb(self), pid);

			if (d.size)
			{
				SkipDBRecord *r = SkipDBRecord_newWithDB_(self);
				BStream_setData_length_(self->stream, d.data, d.size); // need to optimize this out
				SkipDBRecord_fromStream_(r, self->stream);
				SkipDBRecord_pid_(r, pid);
				return r;
			}
		}

		if (pid != 1)
		{
			UDB *udb = SkipDB_udb(self);
			printf("MISSING SKIP RECORD WITH PID: %" PID_FORMAT "\n", pid);
			UDB_at_(udb, pid);
		}
	}
	return NULL;
}

// lookups -----------------------------

void SkipDB_updateAt_put_(SkipDB *self, int level, SkipDBRecord *r)
{
	self->update[level] = r;
	//if (r) SkipDB_noteAccessedRecord_(self, r);
}

void SkipDB_clearUpdate(SkipDB *self)
{
	int i;

	for (i = 0; i < SKIPDB_MAX_LEVEL; i ++)
	{
		SkipDB_updateAt_put_(self, i, NULL);
	}
}

// Record API -------------------------------------

SkipDBRecord *SkipDB_recordAt_(SkipDB *self, Datum k)
{
	SkipDB_clearUpdate(self);
	return SkipDBRecord_find_quick_(self->header, k, 0);
}

int SkipDB_replace_put_(SkipDB *self, Datum k, Datum v)
{
	SkipDBRecord *r = SkipDB_recordAt_(self, k);
        if(r) {
		// update record
		SkipDBRecord_valueDatum_(r, v);
		SkipDBRecord_markAsDirty(r);
                return 1;
        }

        return 0;
}

static int SkipDB_recordAt_put_inner(SkipDB *self, Datum k, Datum v, SkipDBRecord** pp)
{
	SkipDBRecord *r = SkipDB_recordAt_(self, k);
        int rlt;
	//SkipDB_showUpdate(self);

	if (r)
	{
		// update record
		SkipDBRecord_valueDatum_(r, v);
		SkipDBRecord_markAsDirty(r);
                rlt = 1;
	}
	else
	{
		// create new record
		r = SkipDBRecord_newWithDB_(self);
		//printf("%p = SkipDBRecord_newWithDB_(self);\n", (void *)r);

		SkipDBRecord_keyDatum_(r, k);
		SkipDBRecord_valueDatum_(r, v);

		// pick a level for it, and update header level if needed
		{
			int level = SkipDB_pickRandomRecordLevel(self);

			//printf("picked level = %i\n", level);
			SkipDBRecord_level_(r, level);

			if (level > SkipDB_level(self))
			{
				int i;

				for (i = SkipDB_level(self); i < level; i ++)
				{
					SkipDB_updateAt_put_(self, i, self->header);
				}

				SkipDB_level_(self, level);
			}
		}

		SkipDBRecord_markAsDirty(r); // need to do this to get a pid
		//SkipDBRecord_show(r);

		// set the records in the update vector to point to the inserted record
		{
			int level = SkipDBRecord_level(r);

			while (level --)
			{
				SkipDBRecord *updateRecord = self->update[level];

				if (updateRecord)
				{
					SkipDBRecord_copyLevel_from_(r, level, updateRecord);
					/*
					 printf("after: %s insert: %s\n",
						   UArray_asCString(updateRecord->key),
						   UArray_asCString(r->key));
					*/

					// update previous pointer
					// (r is to the right of the update records)
					if (level == 0)
					{
						SkipDBRecord *nextRecord = SkipDBRecord_nextRecord(updateRecord);
						SkipDBRecord_previousRecord_(r, updateRecord);

						if(nextRecord)
						{
							SkipDBRecord_previousRecord_(nextRecord, r);
							SkipDBRecord_markAsDirty(nextRecord);
						}
					}

					SkipDBRecord_atLevel_setRecord_(updateRecord, level, r);
					SkipDBRecord_markAsDirty(updateRecord);
				}
			}
		}

		SkipDBRecord_markAsDirty(r);
                rlt = 0;
	}

        if(NULL != pp) {
            *pp = r;
        }

        return rlt;
}

SkipDBRecord *SkipDB_recordAt_put_(SkipDB *self, Datum k, Datum v) {
    SkipDBRecord *p = NULL;
    SkipDB_recordAt_put_inner(self, k, v, &p);
    return p;
}

// Datum API -------------------------------------

Datum SkipDB_at_(SkipDB *self, Datum k)
{
	SkipDBRecord *r;

	SkipDB_clearUpdate(self);

	r = SkipDBRecord_find_quick_(self->header, k, 1);

	//SkipDBRecord *r = SkipDB_recordAt_(self, k);

	if (r)
	{
		return Datum_FromUArray_(SkipDBRecord_value(r));
	}

	return Datum_Empty();
}

int SkipDB_at_put_(SkipDB *self, Datum k, Datum v)
{
	//SkipDB_recordAt_put_(self, k, v);
        return SkipDB_recordAt_put_inner(self, k, v, NULL);
}

void SkipDB_removeAt_(SkipDB *self, Datum k)
{
	SkipDBRecord *r = SkipDB_recordAt_(self, k);
	SkipDBRecord *lastUr = NULL;

	if (r)
	{
		int i;

		for (i = 0; i < SKIPDB_MAX_LEVEL; i ++)
		{
			SkipDBRecord *ur = self->update[i];

			if (ur == r)
			{
				break;
			}

			if (ur && ur != lastUr)
			{
				SkipDBRecord_willRemove_(ur, r);
			}

			// update previous pointer

			if (i == 0)
			{
				SkipDBRecord *nextRecord = SkipDBRecord_nextRecord(r);
				SkipDBRecord *previousRecord = SkipDBRecord_previousRecord(r);

				if (nextRecord)
				{
					SkipDBRecord_previousRecord_(nextRecord, previousRecord);
				}
			}
			lastUr = ur;
		}

		if (SkipDB_isOpen(self))
		{
			PID_TYPE pid = SkipDBRecord_pid(r);

			if (pid)
			{
				List_append_(self->pidsToRemove, (void *)pid);
				List_remove_(self->dirtyRecords, r);
			}
			//UDB_removeAt_(SkipDB_udb(self), SkipDBRecord_pid(r));
		}

		// nothing should be pointing to this record, so we can dealloc it

		//SkipDBRecord_object_(r, NULL);
		SkipDBRecord_dealloc(r);

#ifdef DEBUG
		if (SkipDB_recordAt_(self, k))
		{
			printf("SkipDB_removeAt_ error - not removed\n");
			exit(-1);
		}
#endif
	}
}

void SkipDB_showUpdate(SkipDB *self)
{
	int i = self->header->level;

	printf("SkipDB update vector:\n");

	while (i --)
	{
		printf("  %i : %s\n", i, UArray_asCString(SkipDBRecord_key(self->update[i])));
	}

	printf("\n");
}

void SkipDB_show(SkipDB *self)
{
	SkipDBRecord *r = self->header;
	printf("\nSkipDB:\n");
	while (r)
	{
		SkipDBRecord_show(r);
		r = SkipDBRecord_recordAtLevel_(r, 0);
	}
	printf("\n\n");
}

// objects --------------------------------------------------

void SkipDB_objectMarkFunc_(SkipDB *self, SkipDBObjectMarkFunc *func)
{
	self->objectMarkFunc = func;
}

void SkipDB_freeObjectCallback_(SkipDB *self, SkipDBFreeObjectFunc *func)
{
	self->objectFreeFunc = func;
}

// cursor ---------------------------------

int SkipDB_count(SkipDB *self)
{
	int count = 0;
	SkipDBRecord *r = SkipDB_firstRecord(self);

	if (!r)
	{
		return 0;
	}

	count = 1;

	while ((r = SkipDBRecord_nextRecord(r)))
	{
		count ++;
	}
	return count;
}

SkipDBRecord *SkipDB_firstRecord(SkipDB *self)
{
	if (SkipDBRecord_level(self->header) == 0) return NULL;
	return SkipDBRecord_nextRecord(self->header);
}

SkipDBRecord *SkipDB_lastRecord(SkipDB *self)
{
	return SkipDBRecord_findLastRecord(self->header);
}

SkipDBRecord *SkipDB_goto_(SkipDB *self, Datum key)
{
	SkipDBRecord *r = SkipDB_recordAt_(self, key);

	if (!r)
	{
		r = self->update[0];
	}

	return r;
}

SkipDBCursor *SkipDB_createCursor(SkipDB *self)
{
	SkipDBCursor *cursor = SkipDBCursor_newWithDB_(self);
	List_append_(self->cursors, cursor);
	return cursor;
}

void SkipDB_removeCursor_(SkipDB *self, SkipDBCursor *cursor)
{
	SkipDBCursor_release(cursor);
	List_remove_(self->cursors, cursor);
	//SkipDBCursor_free(cursor);
}

// moving from in-memory to on-disk ---------------------------------

void SkipDB_mergeInto_(SkipDB *self, SkipDB *other)
{
	SkipDBRecord *r = self->header;

	while ((r = SkipDBRecord_nextRecord(r)))
	{
		Datum k = SkipDBRecord_keyDatum(r);
		Datum v = SkipDBRecord_valueDatum(r);
		SkipDB_at_put_(other, k, v);
	}
}

int SkipDB_exists(SkipDB *self, Datum key) {
    Datum dvalue = SkipDB_at_(self, key);
    if(NULL == dvalue.data) {
        return 0;
    } else {
        return 1;
    }
}

void SkipDB_list_prefix(SkipDB* self, Datum k, void* ctx, skipdb_list_callback callback)
{
    Datum t;
    SkipDBCursor* cursor = SkipDB_createCursor(self);
    SkipDBRecord* rc = SkipDBCursor_goto_(cursor, k);
    if(NULL == rc) {
        goto list_finish;
    }

    t = SkipDBRecord_keyDatum(rc);
    //printf("k.size=%d t.size=%d k=%s t=%s\n", k.size, t.size, k.data, t.data);
    if((k.size <= t.size) && (0 == strncmp((char*)t.data, (char*)k.data, k.size-1))) {
        (*callback)(self, rc, ctx);
    }

    rc = SkipDBCursor_next(cursor);
    while(NULL != rc) {
        t = SkipDBRecord_keyDatum(rc);
        if((k.size <= t.size) && (0 == strncmp((char*)t.data, (char*)k.data, k.size-1))) {
            (*callback)(self, rc, ctx);
            rc = SkipDBCursor_next(cursor);
        } else {
            break;
        }
    }

list_finish:
    if(NULL != cursor) {
        SkipDBCursor_release(cursor);
    }
}

/* TODO howto make better */
SkipDBRecord* SkipDB_list_next(SkipDB* self, Datum k, SkipDBCursor* cursor) {
    Datum t;
    SkipDBRecord* rc = SkipDBCursor_next(cursor);
    if(NULL != rc) {
        t = SkipDBRecord_keyDatum(rc);
        if(!((k.size <= t.size) && (0 == strncmp((char*)t.data, (char*)k.data, k.size-1)))) {
            rc = NULL;
        }
    }
    return rc;
}

SkipDBRecord* SkipDB_list_first(SkipDB* self, Datum k, SkipDBCursor** pcur) {
    Datum t;
    SkipDBCursor* cursor = SkipDB_createCursor(self);
    SkipDBRecord* rc = SkipDBCursor_goto_(cursor, k);

    *pcur = cursor;
    if(NULL == rc) {
        return NULL;
    }

    t = SkipDBRecord_keyDatum(rc);
    //printf("k.size=%d t.size=%d k=%s t=%s\n", k.size, t.size, k.data, t.data);
    if((k.size <= t.size) && (0 == strncmp((char*)t.data, (char*)k.data, k.size-1))) {
        return rc;
    } else {
        return SkipDB_list_next(self, k, cursor);
    }
}

int SkipDB_maxPos(SkipDB* self) {
    return self->udb->records->file->maxPos;
}
