//metadoc PHash copyright Steve Dekorte 2002, Marc Fauconneau 2007
//metadoc PHash license BSD revised
/*metadoc PHash description
	PHash - Cuckoo Hash
	keys and values are references (they are not copied or freed)
	key pointers are assumed unique
*/

#ifndef PHASH_DEFINED
#define PHASH_DEFINED 1

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
} PHashRecord;


typedef struct
{
	PHashRecord *records;		// contains the two tables

	unsigned int log2tableSize;	// log2(tableSize)
	unsigned int tableSize;		// total number of records per table
	unsigned int numKeys;		// number of used records

	unsigned int mask;			// hash bit mask
	PHashRecord nullRecord;		// for lookup misses
	unsigned int balance;		// to balance tables
} PHash;

//#define PHash_mask(self) (self->tableSize-1)
#define PHash_maxLoops(self) (self->tableSize)
#define PHash_maxKeys(self) (self->tableSize)

BASEKIT_API void PHash_print(PHash *self); // to debug

BASEKIT_API PHash *PHash_new(void);
BASEKIT_API void PHash_free(PHash *self);
BASEKIT_API PHash *PHash_clone(PHash *self);
BASEKIT_API void PHash_copy_(PHash *self, PHash *other);

BASEKIT_API PHashRecord* PHash_cuckoo_(PHash *self, PHashRecord* record);
BASEKIT_API void PHash_grow(PHash *self);
BASEKIT_API void PHash_growWithRecord(PHash *self, PHashRecord* record);

BASEKIT_API size_t PHash_memorySize(PHash *self);
BASEKIT_API void PHash_compact(PHash *self);

//BASEKIT_API unsigned int PHash_count(PHash *self);
BASEKIT_API unsigned int PHash_countRecords_size_(unsigned char *records, unsigned int size);

BASEKIT_API void *PHash_firstKeyForValue_(PHash *self, void *v);

// --- perform --------------------------------------------------

BASEKIT_API void PHash_removeValue_(PHash *self, void *value);

#include "PHash_inline.h"

#ifdef __cplusplus
}
#endif
#endif

