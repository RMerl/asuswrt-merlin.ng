
//metadoc SHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc SHash license BSD revised

#define SHASH_C
#include "SHash.h"
#undef SHASH_C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void SHash_print(SHash* self)
{
	printf("self->log2tableSize = %d\n", self->log2tableSize);
	printf("self->tableSize = %d\n", self->tableSize);
	printf("self->numKeys = %d\n", self->numKeys);

	printf("self->mask = %d\n", self->mask);
	printf("self->balance = %d\n", self->balance);
	printf("self->maxLoops = %d\n", SHash_maxLoops(self));
	printf("self->maxKeys = %d\n", SHash_maxKeys(self));

	printf("self->nullRecord.key = %d\n", self->nullRecord.key);
	printf("self->nullRecord.value = %d\n", self->nullRecord.value);

	printf("\nmemory usage : %d bytes\n", SHash_memorySize(self));
	printf("\ndensity : %f \n", (self->numKeys*sizeof(SHashRecord))/(double)SHash_memorySize(self));

	{
		unsigned int i,j;
		int count[2] = {0,0};

		for (j = 0; j < 2; j ++)
		{
			for (i = 0; i < self->tableSize; i ++)
			{
				SHashRecord *record = SHASH_RECORDS_AT_(self, j, i);
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

void SHash_tableInit_(SHash* self, int log2tableSize)
{
	if (log2tableSize > 20)
		printf("ouuups");
	self->log2tableSize = log2tableSize;
	self->tableSize = 1<<self->log2tableSize;
	self->records = (SHashRecord *)io_calloc(1, sizeof(SHashRecord) * self->tableSize * 2);
	self->mask = self->tableSize-1;
}

SHash *SHash_new(void)
{
	SHash *self = (SHash *)io_calloc(1, sizeof(SHash));
	self->numKeys = 0;
	SHash_tableInit_(self, 1);
	//printf("ok");
	return self;
}

SHash *SHash_clone(SHash *self)
{
	SHash *child = SHash_new();
	SHash_copy_(child, self);
	return child;
}

void SHash_free(SHash *self)
{
	io_free(self->records);
	io_free(self);
}

size_t SHash_memorySize(SHash *self)
{
	return sizeof(SHash) + (self->tableSize * 2 * sizeof(SHashRecord));
}

void SHash_compact(SHash *self)
{
	printf("need to implement SHash_compact\n");
}

void SHash_copy_(SHash *self, SHash *other)
{
	SHashRecord *records = self->records;
	memcpy(self, other, sizeof(SHash));
	self->records = (SHashRecord *)io_realloc(records, sizeof(SHashRecord) * self->tableSize * 2);
	memcpy(self->records, other->records, sizeof(SHashRecord) * self->tableSize * 2);
}

/*	this is where our cuckoo acts :
	it kicks records out of nests until it finds an empty nest or gets tired
	returns the empty nest if found, or NULL if it is too tired
*/
SHashRecord *SHash_cuckoo_(SHash *self, SHashRecord* thisRecord)
{
	unsigned int hash;
	SHashRecord* record;
	record = SHash_recordAt_(self, thisRecord->key);

	if (record != &self->nullRecord && NULL == record->key)
		return record;
	else
	{
		if (SHash_keysAreEqual_(self, thisRecord->key, record->key))
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

				hash = SHash_hash_more(self, SHash_hash(self, thisRecord->key));
				record = SHASH_RECORDS_AT_HASH_(self, 1, hash);
				if (NULL == record->key)
					return record;
				else
					SHashRecord_swap(record, thisRecord);

				if (SHash_keysAreEqual_(self, thisRecord->key, record->key))
					return record;
			}
			self->balance = 1;

			for (i = 0; i < SHash_maxLoops(self); i++)
			{
				hash = SHash_hash(self, thisRecord->key);
				record = SHASH_RECORDS_AT_HASH_(self, 0, hash);
				if (NULL == record->key)
					return record;
				else
					SHashRecord_swap(record, thisRecord);

				if (SHash_keysAreEqual_(self, thisRecord->key, record->key))
					return record;

				hash = SHash_hash_more(self, SHash_hash(self, thisRecord->key));
				record = SHASH_RECORDS_AT_HASH_(self, 1, hash);
				if (NULL == record->key)
					return record;
				else
					SHashRecord_swap(record, thisRecord);

				if (SHash_keysAreEqual_(self, thisRecord->key, record->key))
					return record;
			}

			// the cuckoo is tired ^^
			return NULL;
		}
	}

	return NULL;
}

void SHash_grow(SHash *self)
{
	unsigned int oldTableSize = self->tableSize;
	SHashRecord* oldRecords = self->records;

	self->records = NULL;
	while (self->records == NULL)
	{
		unsigned int i;

		SHash_tableInit_(self, self->log2tableSize + 1);

		// enumerate old records

		i = 0;
		while (i < oldTableSize*2)
		{
			SHashRecord thisRecord = oldRecords[i];
			i++;

			if (thisRecord.key)
			{
				SHashRecord *record = SHash_cuckoo_(self, &thisRecord);
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

void SHash_growWithRecord(SHash *self, SHashRecord* thisRecord)
{
	// put the value anywhere, SHash_grow will rehash
	unsigned int i,j;

	for (j = 0; j < 2; j ++)
	for (i = 0; i < self->tableSize; i ++)
	{
		SHashRecord *record = SHASH_RECORDS_AT_(self, j, i);

		if (record != &self->nullRecord && NULL == record->key)
		{
			*record = *thisRecord;
			self->numKeys ++;
			SHash_grow(self);
			return;
		}
	}

	// we can never be there
	return;
}

void SHash_removeValue_(SHash *self, void *value)
{
	SHashRecord *record;
	int index = 0;

	while (index < self->tableSize*2)
	{
		record = self->records + index;
		index ++;

		if (record->key && record->value == value)
		{
			self->numKeys --;
			memset(record, 0, sizeof(SHashRecord));
			return;
		}
	}
}

void *SHash_firstKeyForValue_(SHash *self, void *v)
{
	unsigned int i,j;

	for (j = 0; j < 2; j ++)
	for (i = 0; i < self->tableSize; i ++)
	{
		SHashRecord *record = SHASH_RECORDS_AT_(self, j, i);

		if (record->key && record->value == v)
			return record->key;
	}
	return NULL;
}


