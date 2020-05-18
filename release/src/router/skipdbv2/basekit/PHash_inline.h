//metadoc PHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc PHash license BSD revised
/*metadoc PHash description
	PHash - Cuckoo Hash
	keys and values are references (they are not copied or freed)
	key pointers are assumed unique
*/

#ifdef PHASH_C
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#include <stdio.h>

#define PHASH_RECORDS_AT_(self, tableIndex, index) (self->records + self->tableSize*tableIndex + index)
#define PHASH_RECORDS_AT_HASH_(self, tableIndex, hash) \
(self->records + self->tableSize*tableIndex + (hash & self->mask))

IOINLINE unsigned int PHash_count(PHash *self)
{
	return self->numKeys;
}

IOINLINE void PHashRecord_swap(PHashRecord* r1, PHashRecord* r2)
{
	PHashRecord t = *r1;
	*r1 = *r2;
	*r2 = t;
}

#define PHashKey_value(k) k

/*
IOINLINE void *PHashKey_value(void *key)
{
	return key;
}
*/

IOINLINE unsigned int PHashKey_isEqual_(void *key1, void *key2)
{
//	return key2 && (PHashKey_value(key1) == PHashKey_value(key2));
	return key1 == key2;
}

// simple hash functions, should be enough for pointers
IOINLINE unsigned int PHash_hash(PHash *self, void *key)
{
	intptr_t k = (intptr_t)PHashKey_value(key);
	return k^(k>>4);
}

IOINLINE unsigned int PHash_hash_more(PHash *self, unsigned int hash)
{
	return hash ^ (hash >> self->log2tableSize);
}

// -----------------------------------

IOINLINE void PHash_clean(PHash *self)
{
	memset(self->records, 0, sizeof(PHashRecord) * self->tableSize * 2);
	self->numKeys = 0;
}

IOINLINE PHashRecord *PHash_recordAt_(PHash *self, void *key)
{
	unsigned int hash;
	PHashRecord *record;

	hash = PHash_hash(self, key);
	record = PHASH_RECORDS_AT_HASH_(self, 0, hash);

	// we try the second location
	hash = PHash_hash_more(self, hash);
	record = (key == record->key) ? record : PHASH_RECORDS_AT_HASH_(self, 1, hash);

	return (key == record->key) ? record : &self->nullRecord;
}

IOINLINE void *PHash_at_(PHash *self, void *key)
{
	return PHash_recordAt_(self, key)->value;
}

IOINLINE unsigned char PHash_at_update_(PHash *self, void *key, void *value)
{
	PHashRecord *record = PHash_recordAt_(self, key);

	if (record->key)
	{
		// already a matching key, replace it
		if (PHashKey_isEqual_(key, record->key))
		{
			if (record->value == value)
			{
				return 0; // return 0 if no change
			}

			record->value = value;
			return 1;
		}
	}

	return 0;
}

IOINLINE void PHash_at_put_(PHash *self, void *key, void *value)
{
	PHashRecord thisRecord;
	PHashRecord *record;

	record = PHash_recordAt_(self, key);

	// already a matching key, replace it
	if (record != &self->nullRecord && PHashKey_isEqual_(key, record->key))
	{
		record->value = value;
		return;
	}

	thisRecord.key = key;
	thisRecord.value = value;

	record = PHash_cuckoo_(self, &thisRecord);

	if (!record) // collision
	{
		PHash_growWithRecord(self, &thisRecord);
	}
	else
	{
		*record = thisRecord;
		self->numKeys ++;
		if (self->numKeys > PHash_maxKeys(self))
			PHash_grow(self);
	}
}

IOINLINE void PHash_removeKey_(PHash *self, void *key)
{
	PHashRecord *record = PHash_recordAt_(self, key);
	void *rkey = record->key;

	if (rkey && PHashKey_value(rkey) == PHashKey_value(key))
	{
		self->numKeys --;
		memset(record, 0, sizeof(PHashRecord));
	}
}

// --- enumeration --------------------------------------------------

#define PHASH_FOREACH(self, pkey, pvalue, code) \
{\
	PHash *_self = (self);\
	unsigned int _i, _j, _size = _self->tableSize;\
	void * pkey;\
	void * pvalue;\
	\
	for (_j = 0; _j < 2; _j ++)\
	for (_i = 0; _i < _size; _i ++)\
	{\
		PHashRecord *_record = PHASH_RECORDS_AT_(_self, _j, _i);\
		if (_record->key)\
		{\
			pkey = _record->key;\
			pvalue = _record->value;\
			code;\
		}\
	}\
}

/*
typedef BASEKIT_API void (PHashDoCallback)(void *);

IOINLINE void PHash_do_(PHash *self, PHashDoCallback *callback)
{ PHASH_FOREACH(self, k, v, (*callback)(v)); }

IOINLINE void *PHash_detect_(PHash *self, PHashDetectCallback *callback)
{ PHASH_FOREACH(self, k, v, if ((*callback)(v)) return k; ); return NULL; }

IOINLINE void PHash_doOnKeys_(PHash *self, PHashDoCallback *callback)
{ PHASH_FOREACH(self, k, v, (*callback)(k)); }

IOINLINE void PHash_doOnKeyAndValue_(PHash *self, PHashDoCallback *callback)
{ PHASH_FOREACH(self, k, v, (*callback)(k); (*callback)(v);); }
*/

#undef IO_IN_C_FILE
#endif
