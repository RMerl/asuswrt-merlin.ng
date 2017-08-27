#ifndef __HASH_H__
#define __HASH_H__

#include <sys/types.h>
#include  <hlist.h>

typedef void (*hashDestroyFunc)(void *data);

typedef struct __hash__t_ {
	unsigned int           tableSize;
	HLIST_T                *hashed;
	hashDestroyFunc        destroy;
} *HASH_T;

HASH_T hInit(size_t size, hashDestroyFunc destroy);

void *hGetItem(HASH_T table, const char *key);
void hInsert(HASH_T table, const char *key, const void *data);
void hDestroy(HASH_T table);

#endif /* __HASH_H__ */
