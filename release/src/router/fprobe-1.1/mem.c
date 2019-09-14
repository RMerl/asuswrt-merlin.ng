/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: mem.c,v 1.4.2.3 2005/01/29 19:28:04 sla Exp $
*/

#include <common.h>
#include <stdlib.h>
#include <string.h>

#include <my_log.h>
#include <errno.h>
#include <mem.h>

#ifdef MEM_THREADSAFE
#include <pthread.h>
#endif

#ifdef MEM_THREADSAFE
static pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static struct Mem *root;
unsigned int total_elements;
unsigned int free_elements;
static unsigned int element_size;
static unsigned int bulk_quantity;
static unsigned int limit_memory;
unsigned int total_memory;
static unsigned int mem_index_table_size;
static unsigned int element_table_size;
static unsigned int malloc_size;

void *mem_alloc()
{
	unsigned int i;
	struct Mem *mptr;
	mem_index_t *iptr;
#if MEM_BITS == 0
	void *eptr;
#endif

#ifdef MEM_THREADSAFE
	pthread_mutex_lock(&mem_mutex);
#endif
	if (!free_elements) {
		if (limit_memory && (total_memory + malloc_size) > limit_memory) {
#if ((DEBUG) & DEBUG_M)
			my_log(LOG_DEBUG, "M: limit exhausted");
#endif
			mptr = 0;
			errno = ENOMEM;
			goto done;
		}
		if (!(mptr = calloc(malloc_size, 1))) goto done;
		iptr = (void *) mptr + sizeof(struct Mem);
#if MEM_BITS == 0
		eptr = (void *) iptr + mem_index_table_size;
#endif
		for (i = 0; i < bulk_quantity; i++) {
#if MEM_BITS == 0
			*iptr++ = eptr;
			eptr += element_size;
#else
			*iptr++ = i;
#endif
		}
		mptr->free = bulk_quantity - 1;
		free_elements += mptr->free;
		total_elements += bulk_quantity;
		total_memory += malloc_size;
		mptr->first = iptr;
		mptr->last = (void *) iptr + element_table_size - element_size;
		mptr->next = root;
		root = mptr;
#if ((DEBUG) & DEBUG_M)
		my_log(LOG_DEBUG, "M: alloc bulk: base:%x first:%x last:%x",
			mptr, mptr->first, mptr->last);
		my_log(LOG_DEBUG, "M: mem: total: %d (%dKB), free: %d",
			total_elements, total_memory >> 10, free_elements);
#endif
		mptr = mptr->last;
		goto done;
	}

	mptr = root;
	while (mptr->free == 0) mptr = mptr->next;
	mptr->free--;
	free_elements--;
	iptr = (void *) mptr + sizeof(struct Mem);
#if MEM_BITS == 0
	mptr = iptr[mptr->free];
#else
	mptr = mptr->first + iptr[mptr->free] * element_size;
#endif

done:
#if ((DEBUG) & DEBUG_M)
	my_log(LOG_DEBUG, "M: alloc: %x", mptr);
#endif
#ifdef MEM_THREADSAFE
	pthread_mutex_unlock(&mem_mutex);
#endif
	return mptr;
}

void mem_free(void *eptr)
{
	mem_index_t *iptr;
	struct Mem *mptr, **pptr;

#ifdef MEM_THREADSAFE
	pthread_mutex_lock(&mem_mutex);
#endif
#if ((DEBUG) & DEBUG_M)
	my_log(LOG_DEBUG, "M: free: %x", eptr);
#endif
	mptr = root;
	pptr = &root;
	while (mptr->first > eptr || mptr->last < eptr) {
		pptr = &mptr->next;
		mptr = mptr->next;
	}
	iptr = (void *) mptr + sizeof(struct Mem);
#if MEM_BITS == 0
	iptr[mptr->free] = eptr;
#else
	iptr[mptr->free] = (eptr - mptr->first) / element_size;
#endif
	mptr->free++;
	free_elements++;
	if (mptr->free == bulk_quantity) {
#if ((DEBUG) & DEBUG_M)
		my_log(LOG_DEBUG, "M: free bulk: base:%x first:%x last:%x",
			mptr, mptr->first, mptr->last);
#endif
		*pptr = mptr->next;
		free(mptr);
		total_elements -= bulk_quantity;
		free_elements -= bulk_quantity;
		total_memory -= malloc_size;
#if ((DEBUG) & DEBUG_M)
		my_log(LOG_DEBUG, "M: mem: total: %d (%dKB), free: %d",
			total_elements, total_memory >> 10, free_elements);
#endif
	}
#ifdef MEM_THREADSAFE
	pthread_mutex_unlock(&mem_mutex);
#endif
}

int mem_init(unsigned int element, unsigned int bulk, unsigned int limit)
{
	bulk_quantity = (unsigned) (mem_index_t) bulk; /* for safety: movzbl, movzwl */
	mem_index_table_size = sizeof(mem_index_t) * bulk_quantity;
	element_size = element;
	element_table_size = element_size * bulk_quantity;
	malloc_size = sizeof(struct Mem) + mem_index_table_size + element_table_size;
	limit_memory = limit;
#if ((DEBUG) & DEBUG_M)
	my_log(LOG_DEBUG, "M: init: element size:%d quantity:%d bulk size:%d limit:%d",
		element_size, bulk_quantity, malloc_size, limit_memory);
#endif
	return 0;
}
