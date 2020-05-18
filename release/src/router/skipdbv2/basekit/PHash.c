
//metadoc PHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc PHash license BSD revised

#define PHASH_C
#include "PHash.h"
#undef PHASH_C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void PHash_print(PHash* self)
{
	printf("self->log2tableSize = %d\n", self->log2tableSize);
	printf("self->tableSize = %d\n", self->tableSize);
	printf("self->numKeys = %d\n", self->numKeys);

	printf("self->mask = %d\n", self->mask);
	printf("self->balance = %d\n", self->balance);
	printf("self->maxLoops = %d\n", PHash_maxLoops(self));
	printf("self->maxKeys = %d\n", PHash_maxKeys(self));

	printf("self->nullRecord.key = %d\n", self->nullRecord.key);
	printf("self->nullRecord.value = %d\n", self->nullRecord.value);

	printf("\nmemory usage : %d bytes\n", PHash_memorySize(self));
	printf("\ndensity : %f \n", (self->numKeys*sizeof(PHashRecord))/(double)PHash_memorySize(self));

	{
		unsigned int i,j;
		int count[2] = {0,0};

		for (j = 0; j < 2; j ++)
		{
			for (i = 0; i < self->tableSize; i ++)
			{
				PHashRecord *record = PHASH_RECORDS_AT_(self, j, i);
				if (NULL == record->key)
				{
					if (NULL == record->value)
						printf("_");
					else
						printf("!");
				}
				else
				{
					printf("x");
					count[j]++;
				}
			}
			printf("\n");
		}
		printf("balance : %d / %d [%1.3f]\n", count[0], count[1], (count[0]-count[1])/(double)(count[0]+count[1]) );
	}
}

void PHash_tableInit_(PHash* self, int log2tableSize)
{
	if (log2tableSize > 20)
		printf("ouuups");
	self->log2tableSize = log2tableSize;
	self->tableSize = 1<<self->log2tableSize;
	self->records = (PHashRecord *)io_calloc(1, sizeof(PHashRecord) * self->tableSize * 2);
	memset(self->records, 0x0, sizeof(PHashRecord) * self->tableSize * 2);
	self->mask = self->tableSize-1;
}

PHash *PHash_new(void)
{
	PHash *self = (PHash *)io_calloc(1, sizeof(PHash));
	memset(self, 0x0, sizeof(PHash));
	self->numKeys = 0;
	PHash_tableInit_(self, 1);
	//printf("ok");
	return self;
}

PHash *PHash_clone(PHash *self)
{
	PHash *child = PHash_new();
	PHash_copy_(child, self);
	return child;
}

void PHash_free(PHash *self)
{
	io_free(self->records);
	io_free(self);
}

size_t PHash_memorySize(PHash *self)
{
	return sizeof(PHash) + (self->tableSize * 2 * sizeof(PHashRecord));
}

void PHash_compact(PHash *self)
{
	printf("need to implement PHash_compact\n");
}

void PHash_copy_(PHash *self, PHash *other)
{
	PHashRecord *records = self->records;
	memcpy(self, other, sizeof(PHash));
	self->records = (PHashRecord *)io_realloc(records, sizeof(PHashRecord) * self->tableSize * 2);
	memcpy(self->records, other->records, sizeof(PHashRecord) * self->tableSize * 2);
}

/*	this is where our cuckoo acts :
	it kicks records out of nests until it finds an empty nest or gets tired
	returns the empty nest if found, or NULL if it is too tired
*/
PHashRecord *PHash_cuckoo_(PHash *self, PHashRecord* thisRecord)
{
	unsigned int hash;
	PHashRecord* record;
	record = PHash_recordAt_(self, thisRecord->key);

	if (record != &self->nullRecord && NULL == record->key)
		return record;
	else
	{
		if (PHashKey_isEqual_(thisRecord->key, record->key))
		{
			return record;
		}
		else
		{
			int i;
			// to balance load
			if (self->balance)
			{
				self->balance = 0;

				hash = PHash_hash_more(self, PHash_hash(self, thisRecord->key));
				record = PHASH_RECORDS_AT_HASH_(self, 1, hash);
				if (NULL == record->key)
					return record;
				else
					PHashRecord_swap(record, thisRecord);

				if (PHashKey_isEqual_(thisRecord->key, record->key))
					return record;
			}
			self->balance = 1;

			for (i = 0; i < PHash_maxLoops(self); i++)
			{
				hash = PHash_hash(self, thisRecord->key);
				record = PHASH_RECORDS_AT_HASH_(self, 0, hash);
				if (NULL == record->key)
					return record;
				else
					PHashRecord_swap(record, thisRecord);

				if (PHashKey_isEqual_(thisRecord->key, record->key))
					return record;

				hash = PHash_hash_more(self, PHash_hash(self, thisRecord->key));
				record = PHASH_RECORDS_AT_HASH_(self, 1, hash);
				if (NULL == record->key)
					return record;
				else
					PHashRecord_swap(record, thisRecord);

				if (PHashKey_isEqual_(thisRecord->key, record->key))
					return record;
			}

			// the cuckoo is tired ^^
			return NULL;
		}
	}

	return NULL;
}

void PHash_grow(PHash *self)
{
	unsigned int oldTableSize = self->tableSize;
	PHashRecord* oldRecords = self->records;

	self->records = NULL;
	while (self->records == NULL)
	{
		unsigned int i;

		PHash_tableInit_(self, self->log2tableSize + 1);

		// enumerate old records

		i = 0;
		while (i < oldTableSize*2)
		{
			PHashRecord thisRecord = oldRecords[i];
			i++;

			if (thisRecord.key)
			{
				PHashRecord *record = PHash_cuckoo_(self, &thisRecord);
				if (!record) // collision
				{
					io_free(self->records);
					self->records = NULL;
					break; // grow & rehash
				}
				*record = thisRecord;
			}
		}
	}
	io_free(oldRecords);
}

void PHash_growWithRecord(PHash *self, PHashRecord* thisRecord)
{
	// put the value anywhere, PHash_grow will rehash
	unsigned int i,j;

	for (j = 0; j < 2; j ++)
	for (i = 0; i < self->tableSize; i ++)
	{
		PHashRecord *record = PHASH_RECORDS_AT_(self, j, i);

		if (record != &self->nullRecord && NULL == record->key)
		{
			*record = *thisRecord;
			self->numKeys ++;
			PHash_grow(self);
			return;
		}
	}

	// we can never be there
	return;
}

void PHash_removeValue_(PHash *self, void *value)
{
	PHashRecord *record;
	int index = 0;

	while (index < self->tableSize*2)
	{
		record = self->records + index;
		index ++;

		if (record->key && record->value == value)
		{
			self->numKeys --;
			memset(record, 0, sizeof(PHashRecord));
			return;
		}
	}
}

void *PHash_firstKeyForValue_(PHash *self, void *v)
{
	unsigned int i,j;

	for (j = 0; j < 2; j ++)
	for (i = 0; i < self->tableSize; i ++)
	{
		PHashRecord *record = PHASH_RECORDS_AT_(self, j, i);

		if (record->key && record->value == v)
			return record->key;
	}
	return NULL;
}


