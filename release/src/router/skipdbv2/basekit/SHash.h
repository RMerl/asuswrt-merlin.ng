
//metadoc SHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc SHash license BSD revised
/*metadoc SHash description
	SHash - Cuckoo Hash
	keys and values are references (they are not copied or freed)
	key pointers are assumed unique
*/

#ifndef SHASH_DEFINED
#define SHASH_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	void *key;
	void *value;
} SHashRecord;

typedef int (SHashKeysEqualCallback)(void *, void *);
typedef intptr_t (SHashHashforKeyCallback)(void *);

typedef struct
{
	SHashRecord *records;		// contains the two tables

	unsigned int log2tableSize;	// log2(tableSize)
	unsigned int tableSize;		// total number of records per table
	unsigned int numKeys;		// number of used records

	unsigned int mask;			// hash bit mask
	SHashRecord nullRecord;		// for lookup misses
	unsigned int balance;		// to balance tables
	SHashKeysEqualCallback *keysEqual;
	SHashHashforKeyCallback *hashForKey;

} SHash;

//#define SHash_mask(self) (self->tableSize-1)
#define SHash_maxLoops(self) (self->tableSize < 20 ? self->tableSize : 20)
#define SHash_maxKeys(self)  (self->tableSize)
#define SHash_setKeysEqualCallback(self, v)  (self->keysEqual = v)
#define SHash_setHashForKeyCallback(self, v)  (self->hashForKey = v)

BASEKIT_API void SHash_print(SHash *self); // to debug

BASEKIT_API SHash *SHash_new(void);
BASEKIT_API void SHash_free(SHash *self);
BASEKIT_API SHash *SHash_clone(SHash *self);
BASEKIT_API void SHash_copy_(SHash *self, SHash *other);

BASEKIT_API SHashRecord* SHash_cuckoo_(SHash *self, SHashRecord* record);
BASEKIT_API void SHash_grow(SHash *self);
BASEKIT_API void SHash_growWithRecord(SHash *self, SHashRecord* record);

BASEKIT_API size_t SHash_memorySize(SHash *self);
BASEKIT_API void SHash_compact(SHash *self);

//BASEKIT_API unsigned int SHash_count(SHash *self);
BASEKIT_API unsigned int SHash_countRecords_size_(unsigned char *records, unsigned int size);

BASEKIT_API void *SHash_firstKeyForValue_(SHash *self, void *v);

BASEKIT_API void SHash_removeValue_(SHash *self, void *value);

#include "SHash_inline.h"

#ifdef __cplusplus
}
#endif
#endif

