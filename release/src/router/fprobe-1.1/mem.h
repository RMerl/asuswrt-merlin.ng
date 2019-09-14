/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: mem.h,v 1.1.1.1.2.3 2004/08/13 20:40:43 sla Exp $
*/

#ifndef _MEM_H_
#define _MEM_H_

#if MEM_BITS != 0 && MEM_BITS != 8 && MEM_BITS != 16
#error illegal value in MEM_BITS
#endif

#if defined _REENTRANT || defined _THREAD_SAFE
#define MEM_THREADSAFE
#endif

#include <my_inttypes.h>

#if MEM_BITS == 0
typedef void* mem_index_t;
#endif
#if MEM_BITS == 8
typedef uint8_t mem_index_t;
#endif
#if MEM_BITS == 16
typedef uint16_t mem_index_t;
#endif

void *mem_alloc();
void mem_free(void *);
int mem_init(unsigned int, unsigned int, unsigned int);

struct Mem {
	struct Mem *next;
	unsigned int free;
	void *first;
	void *last;
	/*
	mem_index_t mem_index_table[bulk_quantity]
	struct UserDef element_table[bulk_quantity]
	*/
};

#endif
