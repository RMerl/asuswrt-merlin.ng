

#include "SkipDB.h"
#include "SkipDBRecord.h"

static void SkipDBPointer_setRecord_(SkipDBPointer *self, SkipDBRecord *r)
{
	self->record = r;
}


static PID_TYPE SkipDBPointer_pid(SkipDBPointer *self)
{
	return self->pid;
}

//-----------------------------------------------------

SkipDBRecord *SkipDBRecord_new(void)
{
	//SkipDBRecord *self = (SkipDBRecord *)calloc(1, sizeof(SkipDBRecord));
	SkipDBRecord *self = (SkipDBRecord *)malloc(sizeof(SkipDBRecord));
	memset(self, 0, sizeof(SkipDBRecord));
	self->key = UArray_new();
	self->ownsKey = 1;
	self->value = UArray_new();
	return self;
}

SkipDBRecord *SkipDBRecord_newWithDB_(void *db)
{
	SkipDB *sdb = (SkipDB *)db;
	SkipDBRecord *self = SkipDBRecord_new();
	SkipDBRecord_db_(self, sdb);
	SkipDB_noteNewRecord_(sdb, self);
	return self;
}

// older/younger ------------------------

void SkipDBRecord_setOlderRecord_(SkipDBRecord *self, SkipDBRecord *r)
{
	if (r == self)
	{
		printf("error: r == self\n");
	}

	self->olderRecord = r;
}

SkipDBRecord *SkipDBRecord_olderRecord(SkipDBRecord *self)
{
	return self->olderRecord;
}

void SkipDBRecord_setYoungerRecord_(SkipDBRecord *self, SkipDBRecord *r)
{
	if (r == self)
	{
		printf("error: r == self\n");
	}

	self->youngerRecord = r;
}

SkipDBRecord *SkipDBRecord_youngerRecord(SkipDBRecord *self)
{
	return self->youngerRecord;
}

void SkipDBRecord_removeFromAgeList(SkipDBRecord *self)
{
	SkipDBRecord *or = self->olderRecord;
	SkipDBRecord *yr = self->youngerRecord;

	if (or) SkipDBRecord_setYoungerRecord_(or, yr);
	if (yr) SkipDBRecord_setOlderRecord_(yr, or);
}

void SkipDBRecord_showAgeList(SkipDBRecord *self)
{
	SkipDBRecord *r = self;

	printf("age list:\n");

	while (r)
	{
		printf("  record %p '%s'\n",
			  (void *)r, (char *)UArray_asCString(r->key));
		r = r->olderRecord;
	}
}

//-----------------------------------------

int SkipDBRecord_pointersAreEmpty(SkipDBRecord *self)
{
	SkipDBPointer *pointers = self->pointers;

	if (pointers)
	{
		int i, max = self->level;

		for (i = 0; i < max; i ++)
		{
			SkipDBRecord *r = pointers[i].record;
			PID_TYPE pid = pointers[i].pid;

			if (r || pid) return 0;
		}
	}

	return 1;
}


void SkipDBRecord_removeReferencesToUnmarked(SkipDBRecord *self)
{
	SkipDBPointer *pointers = self->pointers;

	if (pointers)
	{
		int i, max = self->level;

		for (i = 0; i < max; i ++)
		{
			SkipDBRecord *r = pointers[i].record;

			if (r && r->mark == 0)
			{
				pointers[i].pid = SkipDBRecord_pid(r);
				pointers[i].record = NULL;
			}
		}
	}

	if (self->previousRecord && self->previousRecord->mark == 0)
	{
		self->previousPid = SkipDBRecord_pid(self->previousRecord);
		self->previousRecord = NULL;
	}
}

//#define SKIPDB_DEBUG 1

#ifdef SKIPDB_DEBUG
static List *deallocedRecords = NULL;
#endif

void SkipDBRecord_dealloc(SkipDBRecord *self)
{
#ifdef SKIPDB_DEBUG
	if (!deallocedRecords) deallocedRecords = List_new();
	printf("SkipDBRecord_dealloc(%p)\n", (void *)self);
	List_append_(deallocedRecords, self);
#endif

	SkipDB_noteWillFreeRecord_((SkipDB *)(self->sdb), self);

	if (self->pointers)
	{

// Commented out since we should not touch records linked
//  from here, because they may have already been freed.
//  (e.g. see SkipDB_freeAllCachedRecords or SkipDB_freeExcessCachedRecords).
//		SkipDBPointer *next = self->pointers + 0;
//
//		if (next && next->record)
//		{
//			next->record->previousRecord = NULL;
//		}

		free(self->pointers);
	}

	if (self->ownsKey) // is this correct?
	{
		UArray_free(self->key);
	}

	UArray_free(self->value);


#ifdef SKIPDB_DEBUG
	return;
#endif
	free(self);
}

void SkipDBRecord_db_(SkipDBRecord *self, void *sdb)
{
	self->sdb = sdb;
}

void SkipDBRecord_pid_(SkipDBRecord *self, PID_TYPE pid)
{
	self->pid = pid;
	SkipDB_noteAssignedPidToRecord_(self->sdb, self);
}

PID_TYPE SkipDBRecord_pid(SkipDBRecord *self)
{
	return self->pid;
}

PID_TYPE SkipDBRecord_pidAllocIfNeeded(SkipDBRecord *self)
{
	if (!self->pid)
	{
		PID_TYPE pid = UDB_allocPid(SkipDB_udb(self->sdb));
		SkipDBRecord_pid_(self, pid);
		//printf("record '%s' alloced pid %i\n", (char *)UArray_asCString(self->key), (int)self->pid);
	}

	return self->pid;
}

void SkipDBRecord_mark(SkipDBRecord *self)
{
	self->mark = 1;
}

void SkipDBRecord_markAsDirty(SkipDBRecord *self)
{
	if (!self->isDirty)
	{
		self->isDirty = 1;
		SkipDB_noteDirtyRecord_(self->sdb, self);
	}
}

void SkipDBRecord_markAsClean(SkipDBRecord *self)
{
	self->isDirty = 0;
}

uint8_t SkipDBRecord_isDirty(SkipDBRecord *self)
{
	return self->isDirty;
}

// key ------------------------------------

void SkipDBRecord_keyDatum_(SkipDBRecord *self, Datum k)
{
	UArray_setData_type_size_copy_(self->key, k.data, CTYPE_uint8_t, k.size, 1);
}

Datum SkipDBRecord_keyDatum(SkipDBRecord *self)
{
	return Datum_FromUArray_(self->key);
}

UArray *SkipDBRecord_key(SkipDBRecord *self)
{
	return self->key;
}

// value ------------------------------------

void SkipDBRecord_valueDatum_(SkipDBRecord *self, Datum v)
{
	UArray_setData_type_size_copy_(self->value, v.data, CTYPE_uint8_t, v.size, 1);
}

Datum SkipDBRecord_valueDatum(SkipDBRecord *self)
{
	return Datum_FromUArray_(self->value);
}

UArray *SkipDBRecord_value(SkipDBRecord *self)
{
	return self->value;
}

// pointers ------------------------------------

int SkipDBRecord_level(SkipDBRecord *self)
{
	return self->level;
}

void SkipDBRecord_level_(SkipDBRecord *self, int level)
{
	int oldLevel = self->level;
	self->level = level;

	self->pointers = (SkipDBPointer *)realloc(self->pointers, level * sizeof(SkipDBPointer));

	if (level > oldLevel)
	{
		memset(self->pointers + oldLevel, 0, (level - oldLevel) * sizeof(SkipDBPointer));
	}
}

static void SkipDBRecord_checkLevel_(SkipDBRecord *self, int level)
{
     if (level > self->level - 1)
     {
		printf("SkipDBRecord level out of range\n");
		exit(1);
     }
}

static SkipDBPointer *SkipDBRecord_pointerAtLevel_(SkipDBRecord *self, int level)
{
	SkipDBRecord_checkLevel_(self, level);
	return self->pointers + level;
}

void SkipDBRecord_atLevel_setPid_setSuffix_(SkipDBRecord *self, int level, PID_TYPE pid, uint8_t sMatch)
{
	SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
	p->pid = pid;
	SkipDBPointer_setRecord_(p, NULL);
	p->matchingPrefixSize = sMatch;
}

void SkipDBRecord_atLevel_setPid_(SkipDBRecord *self, int level, PID_TYPE pid)
{
	SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
	p->pid = pid;
	SkipDBPointer_setRecord_(p, NULL);
}

PID_TYPE SkipDBRecord_allocedPidAtLevel_(SkipDBRecord *self, int level)
{
	SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
	PID_TYPE pid = p->pid;

	if (!pid && p->record)
	{
		pid = SkipDBRecord_pidAllocIfNeeded(p->record);
	}

	return pid;
}

PID_TYPE SkipDBRecord_pidAtLevel_(SkipDBRecord *self, int level)
{
	SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
	PID_TYPE pid = p->pid;

	if (!pid && p->record)
	{
		pid = SkipDBRecord_pid(self);
	}

	return pid;
}

unsigned char SkipDBRecord_matchingPrefixSizeWith_(SkipDBRecord *self, SkipDBRecord *r)
{
	Datum k1 = Datum_FromUArray_(self->key);
	Datum k2 = Datum_FromUArray_(r->key);
	return Datum_matchingPrefixSizeWith_(&k1, &k2);
	//return UArray_matchingPrefixSizeWith_(self->key, r->key);
}

void SkipDBRecord_atLevel_setRecord_(SkipDBRecord *self, int level, SkipDBRecord *r)
{
	SkipDBRecord_checkLevel_(self, level);
	SkipDBRecord_atLevel_setPid_(self, level, SkipDBRecord_pid(r));

	{
		SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
		SkipDBPointer_setRecord_(p, r);
		p->matchingPrefixSize = SkipDBRecord_matchingPrefixSizeWith_(self, r);
	}
}

SkipDBRecord *SkipDBRecord_cachedRecordAtLevel_(SkipDBRecord *self, int level)
{
	SkipDBRecord_checkLevel_(self, level);

	return self->pointers[level].record;
}

//直接下降到0层级，获取下个元素
SkipDBRecord *SkipDBRecord_recordAtLevel_(SkipDBRecord *self, int level)
{
	SkipDBRecord_checkLevel_(self, level);

	{
		SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
		SkipDBRecord *r = p->record;

		if (!r && p->pid)
		{
			r = SkipDB_recordAtPid_((SkipDB *)(self->sdb), p->pid);
			SkipDBPointer_setRecord_(p, r);
		}

		return r;
	}
}

// serialization ------------------------------------

void SkipDBRecord_toStream_(SkipDBRecord *self, BStream *s)
{
	BStream_writeTaggedUArray_(s, self->key);
	BStream_writeTaggedUArray_(s, self->value);

	//printf("writing record '%s'\n", (char *)UArray_asCString(self->key));

	{
		int level, max = SkipDBRecord_level(self);
		BStream_writeTaggedInt32_(s, max);

		for (level = 0; level < max; level ++)
		{
			SkipDBPointer *p = SkipDBRecord_pointerAtLevel_(self, level);
			PID_TYPE pid = SkipDBRecord_allocedPidAtLevel_(self, level);
			BStream_writeTaggedInt32_(s, pid);

			//if (p->record) printf("'%s' %i\n", (char *)UArray_asCString(p->record->key), (int)pid);

			BStream_writeTaggedUint8_(s, p->matchingPrefixSize);
		}
	}

	//printf("\n");

	//BStream_show(s);
}

void SkipDBRecord_fromStream_(SkipDBRecord *self, BStream *s)
{
	BStream_readTaggedUArray_(s, self->key);
	BStream_readTaggedUArray_(s, self->value);

	{
		int level, max = BStream_readTaggedInt32(s);

		SkipDBRecord_level_(self, max);

		for (level = 0; level < max; level ++)
		{
			PID_TYPE pid = BStream_readTaggedInt32(s);
			uint8_t sMatch = BStream_readTaggedUint8(s);
			SkipDBRecord_atLevel_setPid_setSuffix_(self, level, pid, sMatch);
		}
	}

	//BStream_show(s);
}

void SkipDBRecord_save(SkipDBRecord *self)
{
	if (self->isDirty)
	{
		SkipDB *sdb = self->sdb;
		BStream *s = SkipDB_tmpStream(sdb);
		BStream_empty(s);
		SkipDBRecord_toStream_(self, s);

		{
			Datum d = Datum_FromUArray_(BStream_byteArray(s));
			PID_TYPE pid = SkipDBRecord_pid(self);

			if (pid)
			{
				UDB_at_put_(SkipDB_udb(sdb), pid, d);
			}
			else
			{
				pid = UDB_put_(SkipDB_udb(sdb), d);
				SkipDBRecord_pid_(self, pid);
			}
		}

		SkipDBRecord_markAsClean(self);
	}
}

// search ------------------------------------

static int SkipDBRecord_compareKey_(SkipDBRecord *self, Datum key)
{
	Datum d = Datum_FromUArray_(self->key);
	return Datum_compare_(&d, &key);
}

//Added by jannson
//#define USE_SUFFIX_MATCHING 1

SkipDBRecord *SkipDBRecord_find_quick_(SkipDBRecord *self, Datum key, int quick)
{
	SkipDB *db = (SkipDB *)(self->sdb);
	int level = self->level;

#ifdef USE_SUFFIX_MATCHING
	//Datum kDatum = UArray_asDatum(self->key);
        Datum kDatum = Datum_FromUArray_(self->key);
	unsigned char skMatchSize = Datum_matchingPrefixSizeWith_(&kDatum, &key);
#endif

	//printf("record: %s find: %s in ", UArray_asCString(self->key), key.data);
	//SkipDBRecord_show(self);
        //printf("skMatchSize %i\n", skMatchSize);
        //printf("-------------\n\n");

	while (level --)
	{
		SkipDBRecord *r = SkipDBRecord_recordAtLevel_(self, level);
#ifdef USE_SUFFIX_MATCHING
		unsigned char pkMatchSize = self->pointers[level].matchingPrefixSize;
                printf("pkMatchSize=%d, skMatchSize=%d\n", pkMatchSize, skMatchSize);
#endif

		SkipDB_updateAt_put_(db, level, self);

		//if (r && !(pkMatchSize > skMatchSize))
		if (r)
		{
			int c;

#ifdef USE_SUFFIX_MATCHING
			if (pkMatchSize > skMatchSize)
			{
				//printf("search key: '%s' \n", key.data);
				//printf("self key:   '%s' %i\n", UArray_asCString(self->key), skMatchSize);
				//printf("p key:      '%s' %i\n", UArray_asCString(r->key), pkMatchSize);
				//printf("skip compare\n");
				return SkipDBRecord_find_quick_(r, key, quick);
			}
#endif

			c = SkipDBRecord_compareKey_(r, key);

			//printf("key: %s at level: %i\n", UArray_asCString(r->key), level);

			/*
			 if (UArray_sizeInBytes(r->key) == 0)
			 {
				 printf("ERROR: key length = 0\n");
				 exit(1);
			 }
			 */

			if (c == 0) // record is a match
			{
				if (level == 0 || quick)
				{
					return r;
				}
			}
			else if (c < 0) // record key is smaller
			{
                //往前面节点找，遇到大的，就往下个level找
				return SkipDBRecord_find_quick_(r, key, quick);
			}
			// otherwise record key is bigger so we keep searching
		}
	}

	return NULL;
}

SkipDBRecord *SkipDBRecord_findLastRecord(SkipDBRecord *self)
{
	int level = self->level;

	while (level --)
	{
		SkipDBRecord *r = SkipDBRecord_recordAtLevel_(self, level);

		if (r)
		{
			return SkipDBRecord_findLastRecord(r);
		}

	}

	return self;
}

void SkipDBRecord_copyLevel_from_(SkipDBRecord *self, int level, SkipDBRecord *other)
{
	PID_TYPE newPid = SkipDBRecord_pidAtLevel_(other, level);
	SkipDBRecord *cachedRecord = SkipDBRecord_cachedRecordAtLevel_(other, level);

	if (cachedRecord)
	{
		SkipDBRecord_atLevel_setRecord_(self, level, cachedRecord);
	}
	else
	{
		SkipDBRecord_atLevel_setPid_(self, level, newPid);
	}
}

void SkipDBRecord_willRemove_(SkipDBRecord *self, SkipDBRecord *other)
{
	PID_TYPE otherPid = SkipDBRecord_pid(other);
	int level = self->level;

	while (level --)
	{
		if ((otherPid && SkipDBRecord_pidAtLevel_(self, level) == otherPid) ||
		    (SkipDBRecord_recordAtLevel_(self, level) == other))
		{
			SkipDBRecord_copyLevel_from_(self, level, other);
			SkipDBRecord_markAsDirty(self);
		}
	}

	/*
	 if (SkipDBRecord_hasRecord_(self, other))
	 {
		 printf("error - has record after remove\n");
		 exit(-1);
	 }
	 */
}

int SkipDBRecord_hasRecord_(SkipDBRecord *self, SkipDBRecord *other)
{
	int i;

	for (i = 0; i < self->level; i ++)
	{
		if (self->pointers[i].record == other)
		{
			return 1;
		}
	}
	return 0;
}

void SkipDBRecord_show(SkipDBRecord *self)
{
	int i;
	//printf("SRec %i '%s' '%s' (", self->pid,
	//	UArray_asCString(self->key), UArray_asCString(self->value));
	//printf("SRec %i '%s' (", self->pid, UArray_asCString(self->key));
	printf("  record %i '%s'\t:\t", (int)self->pid, UArray_asCString(self->key));

	for (i = 0; i < self->level; i ++)
	{
		SkipDBRecord *r = self->pointers[i].record;

		if (r)
		{
			printf("'%s'", UArray_asCString(r->key));
		}
		else
		{
			printf("%i", (int)(self->pointers[i].pid));
		}

		printf("\t");

		//if (i != self->level - 1) printf(", ");
	}

	printf("\n");
}

// object ------------------------------------

void SkipDBRecord_object_(SkipDBRecord *self, void *object)
{
	self->object = object;
}

void *SkipDBRecord_object(SkipDBRecord *self)
{
	return self->object;
}

// next ------------------------------------

SkipDBRecord *SkipDBRecord_nextRecord(SkipDBRecord *self)
{
	return SkipDBRecord_recordAtLevel_(self, 0);
}

// previous ------------------------------------

void SkipDBRecord_previousPid_(SkipDBRecord *self, PID_TYPE pid)
{
	self->previousPid = pid;
	self->previousRecord = NULL;
}

PID_TYPE SkipDBRecord_previousPid(SkipDBRecord *self)
{
	return self->previousPid;
}

void SkipDBRecord_previousRecord_(SkipDBRecord *self, SkipDBRecord *r)
{
	if (r != self->previousRecord)
	{
		if (r == self)
		{
			printf("SkipDB error: attempt to set previousRecord to self\n");
		}

		self->previousRecord = r;

		if (r)
		{
			self->previousPid = SkipDBRecord_pid(r);
		}

		SkipDBRecord_markAsDirty(self);
	}
}

#ifdef SKIPDB_DEBUG
void textValid(void *record)
{
	if (!deallocedRecords) deallocedRecords = List_new();
	if (List_contains_(deallocedRecords, record))
	{
		printf("attempt to access freed record %p\n", (void *)record);
		exit(-1);
	}
}
#endif

SkipDBRecord *SkipDBRecord_previousRecord(SkipDBRecord *self)
{
	if (!self->previousRecord && self->previousPid)
	{
		self->previousRecord = SkipDB_recordAtPid_((SkipDB *)(self->sdb), self->previousPid);
	}

#ifdef SKIPDB_DEBUG
	textValid(self);
	textValid(self->previousRecord);
#endif

	return self->previousRecord;
}

