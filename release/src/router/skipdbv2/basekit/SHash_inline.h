//metadoc SHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc SHash license BSD revised
/*metadoc SHash description
	SHash - Cuckoo Hash
	keys and values are references (they are not copied or freed)
	key pointers are assumed unique
*/

#ifdef SHASH_C
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#include <stdio.h>

#define SHASH_RECORDS_AT_(self, tableIndex, index) (self->records + self->tableSize*tableIndex + index)
#define SHASH_RECORDS_AT_HASH_(self, tableIndex, hash) \
(self->records + self->tableSize*tableIndex + (hash & self->mask))

IOINLINE unsigned int SHash_count(SHash *self)
{
	return self->numKeys;
}

IOINLINE void SHashRecord_swap(SHashRecord* r1, SHashRecord* r2)
{
	SHashRecord t = *r1;
	*r1 = *r2;
	*r2 = t;
}

IOINLINE unsigned int SHash_keysAreEqual_(SHash *self, void *key1, void *key2)
{
	return key2 && self->keysEqual(key1, key2);
}

// simple hash functions, should be enough for pointers

IOINLINE unsigned int SHash_hash(SHash *self, void *key)
{
	intptr_t k = self->hashForKey(key);
	return k ^ (k >> 4);
}

IOINLINE unsigned int SHash_hash_more(SHash *self, unsigned int hash)
{
	return hash ^ (hash >> self->log2tableSize);
}

// -----------------------------------

IOINLINE void SHash_clean(SHash *self)
{
	memset(self->records, 0, sizeof(SHashRecord) * self->tableSize * 2);
	self->numKeys = 0;
}

IOINLINE SHashRecord *SHash_recordAt_(SHash *self, void *key)
{
	unsigned int hash;
	SHashRecord *record;

	// try first location

	hash = SHash_hash(self, key);
	record = SHASH_RECORDS_AT_HASH_(self, 0, hash);
	if (SHash_keysAreEqual_(self, key, record->key)) { return record; }

	// try second location

	hash = SHash_hash_more(self, hash);
	record = SHASH_RECORDS_AT_HASH_(self, 1, hash);
	if (SHash_keysAreEqual_(self, key, record->key)) { return record; }

	return &self->nullRecord;
}

IOINLINE void *SHash_at_(SHash *self, void *key)
{
	return SHash_recordAt_(self, key)->value;
}

IOINLINE int SHashKey_hasKey_(SHash *self, void *key)
{
	return SHash_at_(self, key) != NULL;
}

IOINLINE void SHash_at_put_(SHash *self, void *key, void *value)
{
	SHashRecord thisRecord;
	SHashRecord *record;

	record = SHash_recordAt_(self, key);

	// already a matching key, replace it
	if (record != &self->nullRecord && SHash_keysAreEqual_(self, key, record->key))
	{
		record->value = value;
		return;
	}

	thisRecord.key = key;
	thisRecord.value = value;

	record = SHash_cuckoo_(self, &thisRecord);

	if (!record) // collision
	{
		SHash_growWithRecord(self, &thisRecord);
		//printf("grow due to key collision: SHash_%p numKeys %i \ttableSize %i \tratio %f\n", (void *)self, self->numKeys, self->tableSize, (float) self->numKeys / (float) self->tableSize);
	}
	else
	{
		*record = thisRecord;
		self->numKeys ++;
		if (self->numKeys > SHash_maxKeys(self))
		{
			SHash_grow(self);
			//printf("grow due to full table: SHash_%p numKeys %i \ttableSize %i \tratio %f\n", (void *)self, self->numKeys, self->tableSize, (float) self->numKeys / (float) self->tableSize);
		}
	}

}

IOINLINE void SHash_removeKey_(SHash *self, void *key)
{
	SHashRecord *record = SHash_recordAt_(self, key);
	void *rkey = record->key;

	if (rkey && SHash_keysAreEqual_(self, rkey, key))
	{
		self->numKeys --;
		memset(record, 0, sizeof(SHashRecord));
	}
}

// --- enumeration --------------------------------------------------

#define SHASH_FOREACH(self, pkey, pvalue, code) \
{\
	SHash *_self = (self);\
	unsigned int _i, _j, _size = _self->tableSize;\
	void * pkey;\
	void * pvalue;\
	\
	for (_j = 0; _j < 2; _j ++)\
	for (_i = 0; _i < _size; _i ++)\
	{\
		SHashRecord *_record = SHASH_RECORDS_AT_(_self, _j, _i);\
		if (_record->key)\
		{\
			pkey = _record->key;\
			pvalue = _record->value;\
			code;\
		}\
	}\
}

/*
typedef BASEKIT_API void (SHashDoCallback)(void *);
IOINLINE void SHash_do_(SHash *self, SHashDoCallback *callback)
{ SHASH_FOREACH(self, k, v, (*callback)(v)); }

IOINLINE void SHash_doOnKeys_(SHash *self, SHashDoCallback *callback)
{ SHASH_FOREACH(self, k, v, (*callback)(k)); }

IOINLINE void SHash_doOnKeyAndValue_(SHash *self, SHashDoCallback *callback)
{ SHASH_FOREACH(self, k, v, (*callback)(k); (*callback)(v);); }
*/

#undef IO_IN_C_FILE
#endif
