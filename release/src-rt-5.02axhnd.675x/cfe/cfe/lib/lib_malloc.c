/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Local memory manager			File: cfe_malloc.c
    *  
    *  This routine is used to manage memory allocated within the 
    *  firmware.  You give it a chunk of memory to manage, and then
    *  these routines manage suballocations from there.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */

#ifdef TESTPROG
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "lib_malloc.h"
#include "lib_math.h"
#include "bcm_memory.h"


/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#define MEMNODE_SEAL 0xFAAFA123		/* just some random constant */
#define MINBLKSIZE 64

/*  *********************************************************************
    *  Types
    ********************************************************************* */


#define memnode_data(t,m) (t) (((memnode_t *) (m))+1)

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

mempool_t kmempool;			/* default pool */

/*  *********************************************************************
    *  kmeminit(pool,buffer,length)
    *  
    *  Initialize the memory manager, given a pointer to an area
    *  of memory and a size.  This routine simply initializes the
    *  root node to be a single block of empty space.
    *  
    *  Input parameters: 
    *      pool - pool pointer
    *  	   buffer - beginning of buffer area, must be pointer-aligned
    *  	   length - length of buffer area
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */


void kmeminit(mempool_t *pool,unsigned char *buffer,int length)
{
    pool->root = (memnode_t *) buffer;
    pool->root->seal = MEMNODE_SEAL;
    pool->root->length = length - sizeof(memnode_t);
    pool->root->data = memnode_data(unsigned char *,pool->root);
    pool->root->status = memnode_free;
    pool->root->next = NULL;

    pool->base = buffer;
    pool->length = length;
}


/*  *********************************************************************
    *  kmempoolbase(pool)
    *  
    *  Returns the base address of the specified memory pool
    *  
    *  Input parameters: 
    *  	   pool - pool pointer
    *  	   
    *  Return value:
    *  	   pointer to beginning of pool's memory
    ********************************************************************* */
void *kmempoolbase(mempool_t *pool)
{
    return pool->base;
}

/*  *********************************************************************
    *  kmempoolsize(pool)
    *  
    *  Returns the total size of the specified memory pool
    *  
    *  Input parameters: 
    *  	   pool - pool pointer
    *  	   
    *  Return value:
    *  	   size of pool in bytes
    ********************************************************************* */

int kmempoolsize(mempool_t *pool)
{
    return pool->length;
}

/*  *********************************************************************
    *  kmemcompact(pool)
    *  
    *  Compact the memory blocks, coalescing consectutive free blocks
    *  on the list.
    *  
    *  Input parameters: 
    *  	   pool - pool descriptor
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void kmemcompact(mempool_t *pool)
{
    memnode_t *m;
    int compacted;

    do {
	compacted = 0;

	for (m = pool->root; m; m = m->next) {

	    /* Check seal to be sure that we're doing ok */

	    if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
		printf("Memory list corrupted!\n");
#endif
		return;
		}

	    /* 
	     * If we're not on the last block and both this
	     * block and the next one are free, combine them
	     */

	    if (m->next && 
		(m->status == memnode_free) &&
		(m->next->status == memnode_free)) {
		m->length += sizeof(memnode_t) + m->next->length;
		m->next->seal = 0;
		m->next = m->next->next;
		compacted++;
		}

	    /* Keep going till we make a pass without doing anything. */
	    }
	} while (compacted > 0);
}


/*  *********************************************************************
    *  kfree(ptr)
    *  
    *  Return some memory to the pool.
    *  
    *  Input parameters: 
    *  	   ptr - pointer to something allocated via kmalloc()
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void kfree(mempool_t *pool,void *ptr)
{
    memnode_t **backptr;
    memnode_t *m;

    if (((unsigned char *) ptr < pool->base) ||
	((unsigned char *) ptr >= (pool->base+pool->length))) {
#ifdef TESTPROG
	printf("Pointer %08X does not belong to pool %08X\n",ptr,pool);
#endif
	return;
	}

    backptr = (memnode_t **) (((unsigned char *) ptr) - sizeof(memnode_t *));
    m = *backptr;

    if (m->seal != MEMNODE_SEAL) {
#ifdef TESTPROG
	printf("Invalid node freed: %08X\n",m);
#endif
	return;
	}

    m->status = memnode_free;

    kmemcompact(pool);
}

/*  *********************************************************************
    *  lib_outofmemory()
    *  
    *  Called when we run out of memory.
    *  XXX replace with something real someday
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void lib_outofmemory(void);
void lib_outofmemory(void)
{
    xprintf("PANIC: out of memory!\n");
}

/*  *********************************************************************
    *  kmalloc(pool,size,align)
    *  
    *  Allocate some memory from the pool.  
    *  
    *  Input parameters: 
    *      pool - pool structure
    *  	   size - size of item to allocate
    *  	   align - alignment (must be zero or a power of 2)
    *  	   
    *  Return value:
    *  	   pointer to data, or NULL if no memory left
    ********************************************************************* */

void *kmalloc(mempool_t *pool,unsigned int size,unsigned int align)
{
    memnode_t *m;
    memnode_t *newm;
    memnode_t **backptr;
    uintptr_t daddr = 0;
    uintptr_t realsize = 0;
    uintptr_t extra;
    uintptr_t blkend;
    uintptr_t ptralign;

    /*
     * Everything should be aligned by at least the
     * size of an int64
     */

    ptralign = (uintptr_t) align;
    if (ptralign < sizeof(void *)) 
        ptralign = sizeof(uint64_t);

    /*	
     * Everything should be at least a multiple of the 
     * size of a pointer.
     */

    if (size == 0) 
        size = sizeof(void *);

    if (size & (sizeof(void *)-1)) {
        size += sizeof(void *);
        size &= ~(sizeof(void *)-1);
    }

    /*
     * Find a memnode at least big enough to hold the storage we
     * want.
     */

    for (m = pool->root; m; m = m->next) {

	if (m->status == memnode_alloc) continue;

	/*
	 * If we wanted a particular alignment, we will
	 * need to adjust the size.
	 */

	daddr = memnode_data(uintptr_t,m);
	extra = 0;
	if (daddr & (ptralign-1)) {
	    extra = (ptralign - (daddr & (ptralign-1)));
	}
	realsize = size + extra;

	if (m->length < realsize) continue;

	break;
     }

    /*
     * If m is null, there's no memory left.
     */

    if (m == NULL) {
	lib_outofmemory();
	return NULL;
	}

    /*
     * Otherwise, use this block.  Calculate the address of the data
     * to preserve the alignment.
     */

    if (daddr & (ptralign-1)) {
	daddr += ptralign;
	daddr &= ~(ptralign-1);
	}

    /* Mark this node as allocated. */

    m->data   = (unsigned char *) daddr;
    m->status = memnode_alloc;

    /* 
     * Okay, this is ugly.  Store a pointer to the original
     * memnode just before what we've allocated.  It's guaranteed
     * to be aligned at least well enough for this pointer.
     * If for some reason the memnode was already exactly
     * aligned, backing up will put us inside the memnode
     * structure itself... that's why the memnodeptr field
     * is there, as a placeholder for this eventuality.
     */

    backptr   = (memnode_t **) (m->data - sizeof(memnode_t *));
    *backptr  = m;

    /*
     * See if we need to split it.
     * Don't bother to split if the resulting size will be 
     * less than MINBLKSIZE bytes
     */

    if (m->length - realsize < MINBLKSIZE) {
	return m->data;
	}

    /*
     * Split this block.  Align the address on a pointer-size
     * boundary.
     */

    daddr += size;
    if (daddr & (uintptr_t)(sizeof(void *)-1)) {
	daddr += (uintptr_t)sizeof(void *);
	daddr &= ~(uintptr_t)(sizeof(void *)-1);
	}

    blkend = memnode_data(uintptr_t,m) + (uintptr_t)(m->length);

    newm = (memnode_t *) daddr;

    newm->next   = m->next;
    m->length    = (unsigned int) (daddr - memnode_data(uintptr_t,m));
    m->next      = newm;
    m->status    = memnode_alloc;
    newm->seal   = MEMNODE_SEAL;
    newm->data    = memnode_data(unsigned char *,newm);
    newm->length = (unsigned int) (blkend - memnode_data(uintptr_t,newm));
    newm->status = memnode_free;

    return m->data;    
}


/*  *********************************************************************
    *  krealloc(pool,ptr,newsize)
    *  
    *  Rallocate memory from the existing pointer.  
    *  
    *  Input parameters: 
    *      pool - pool structure
    *      ptr - pointer to previously allocated memory 
    *             if null kmalloc is called instead then, the content of the 
    *             old pointer is copied to a newly allocate memory. Old memory is freed.  
    *  	   size - size of item to reallocate 
    *  	   align - alignment (must be zero or a power of 2)
    *  	   
    *  Return value:
    *  	   pointer to data, or NULL if no memory left, invalid pointer
    ********************************************************************* */

void *krealloc(mempool_t *pool, void* ptr, unsigned int size, unsigned int align)
{
    memnode_t *m;
    memnode_t *newn;
    memnode_t **backptr;
    unsigned int new_size = size;
    if (!ptr) {
	goto alloc_new;
    }

    if (((unsigned char *) ptr < pool->base) ||
	((unsigned char *) ptr >= (pool->base+pool->length))) {
#ifdef TESTPROG
	printf("Pointer %08X does not belong to pool %08X\n",ptr,pool);
#endif
	goto err;
    }

    backptr = (memnode_t **) (((unsigned char *) ptr) - sizeof(memnode_t *));
    m = *backptr;
    
    if (m == NULL) {
	lib_outofmemory();
	goto err;
    }
    /* A valid ptr for reallocation always has a reference to the next mem node 
    */
    if (m->status != memnode_alloc || !m->next) {
        /*should've been allocated at least once and not freed */
        goto err;
    }

    if (size == 0) 
        size = sizeof(void *);
    /* calculate the difference */
    if (size == m->length || size < m->length) {
        /*no downsizing - returning the same memory*/ 
        return m->data; 
    }

    /*
     * Everything should be aligned by at least the
     * size of an int64
     */
    size = ALIGN(sizeof(void*), size - m->length); 
    if (m->next->status == memnode_free && 
        m->next->length >=  size + MAX(sizeof(memnode_t), MINBLKSIZE)) {
       /*If next block is contigious and free - update current block data and new block*/
        newn = (memnode_t*)ALIGN(sizeof(void*),(uintptr_t)m->next + size);
        newn->length = (memnode_data(uintptr_t,m->next) + m->next->length) - memnode_data(uintptr_t, newn);
        newn->next   = m->next->next;
        newn->seal   = MEMNODE_SEAL;
        newn->data   = memnode_data(unsigned char *,newn);
        newn->status = memnode_free;
        m->next = newn;
        m->length += size;
        return m->data;
    } 
alloc_new:
   { 
     void* newptr = kmalloc(pool, new_size, align);
     if (newptr) {
         if (ptr) {
             memcpy(newptr, ptr, m->length);
             kfree(pool, ptr);
         }
     } 
     return newptr;
   }

err:
    return NULL;
}

unsigned long kmempool_maxalloc(mempool_t *pool)
{
    unsigned long mem_largest = 0;
    memnode_t *m = pool->root;
    do {
	if (!m->status && m->length > mem_largest) {
            mem_largest = m->length;
        }
        m = m->next;
    } while(m); 
    return mem_largest;
}

int kmemstats(mempool_t *pool,memstats_t *stats)
{
    memnode_t *m;
    memnode_t **backptr;
    uintptr_t daddr;

    stats->mem_totalbytes = pool->length;
    stats->mem_allocbytes = 0;
    stats->mem_freebytes = 0;
    stats->mem_allocnodes = 0;
    stats->mem_freenodes = 0;
    stats->mem_largest = 0;

    for (m = pool->root; m; m = m->next) {
	if (m->status) {
	    stats->mem_allocnodes++;
	    stats->mem_allocbytes += m->length;
	    }
	else {
	    stats->mem_freenodes++;
	    stats->mem_freebytes += m->length;
	    if (m->length > stats->mem_largest) {
		stats->mem_largest = m->length;
		}
	    }

	daddr = memnode_data(uintptr_t,m);
	if (m->seal != MEMNODE_SEAL) {
	    return -1;
	    }
	if (m->next && ((daddr + m->length) != (uintptr_t) m->next)) {
	    return -1;
	    }
	if (m->next && (m->next < m)) {
	    return -1;
	    }
	if (m->data < (unsigned char *) m) {
	    return -1;
	    }
	if (m->status == memnode_alloc) {
	    backptr = (memnode_t **) (m->data - sizeof(void *));
	    if (*backptr != m) {
		return -1;
		}
	    }
	}

    return 0;
}

#ifdef KMALLOC_DEBUG 
/* Simple debug tracers */
void kfree_dbg(mempool_t *pool,void *ptr, const char* lst, int line ,const char* fnc_name)
{
     printf("%s:%d %s|-->free 0x%xl\n",lst, line, fnc_name, (unsigned long)ptr);
     return kfree(pool, ptr);  
}

void *kmalloc_dbg(mempool_t *pool,unsigned int size,unsigned int align, const char* lst, int line ,const char* fnc_name)
{   
     void* ptr =  kmalloc(pool, size, align);
     printf("%s:%d %s|-->alloc 0x%xl size %u\n", lst, line,fnc_name, (unsigned long)ptr, size);
     return ptr;
}

void *krealloc_dbg(mempool_t *pool, void* ptr, unsigned int size,unsigned int align, const char* lst, int line ,const char* fnc_name)
{
     void* ptr2 = krealloc(pool, ptr, size, align);
     printf("%s:%d %s|-->realloc 0x%xl size %u new_ptr 0x%xl\n", lst, line,fnc_name, (unsigned long)ptr, size, (unsigned long)ptr2);
     return ptr2; 
}
#endif

/*  *********************************************************************
    *  kmemchk()
    *  
    *  Check the consistency of the memory pool.
    *  
    *  Input parameters: 
    *      pool - pool pointer
    *  	   
    *  Return value:
    *  	   0 - pool is consistent
    *  	   -1 - pool is corrupt
    ********************************************************************* */

#ifdef TESTPROG
int kmemchk(mempool_t *pool,int verbose)
{
    memnode_t *m;
    memnode_t **backptr;
    unsigned int daddr;

    for (m = pool->root; m; m = m->next) {
	if (verbose) {
	    printf("%08X: Next=%08X  Len=%5u  %s  Data=%08X ",
	       m,m->next,m->length,
	       m->status ? "alloc" : "free ",
	       m->data);
	    }
	daddr = memnode_data(uintptr_t,m);
	if (m->seal != MEMNODE_SEAL) {
	    if (verbose) printf("BadSeal ");
	    else return -1;
	    }
	if (m->next && (daddr + m->length != (unsigned int) m->next)) {
	    if (verbose) printf("BadLength ");
	    else return -1;
	    }
	if (m->next && (m->next < m)) {
	    if (verbose) printf("BadOrder ");
	    else return -1;
	    }
	if (m->data < (unsigned char *) m) {
	    if (verbose) printf("BadData ");
	    else return -1;
	    }
	if (m->status == memnode_alloc) {
	    backptr = (memnode_t **) (m->data - sizeof(void *));
	    if (*backptr != m) {
		if (verbose) printf("BadBackPtr ");
		else return -1;
		}
	    }
	if (verbose) printf("\n");
	}

    return 0;
}


#define MEMSIZE 1024*1024

unsigned char *ptrs[4096];
unsigned int sizes[4096];

/*  *********************************************************************
    *  main(argc,argv)
    *  
    *  Test program for the memory allocator
    *  
    *  Input parameters: 
    *  	   argc,argv
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */


void main(int argc,char *argv[])
{
    unsigned char *mem;
    int items = 0;
    int idx;
    int size;
    int totalsize = 0;
    int nfree,freecnt;
    mempool_t *pool = &kmempool;

    mem = malloc(MEMSIZE);
    kmeminit(pool,mem,MEMSIZE);

    items = 0;

    for (;;) {

	for (;;) {
	    if (items == 4096) break;
	    size = rand() % 1024;
	    ptrs[items] = kmalloc(pool,size,1<<(rand() & 7));
	    if (!ptrs[items]) break;
	    sizes[items] = size;
	    items++;
	    totalsize += size;
	    }

	printf("%d items allocated, %d total bytes\n",items,totalsize);

	if (kmemchk(pool,0) < 0) {
	    kmemchk(pool,1);
	    exit(1);
	    }

	/* Scramble the pointers */
	idx = items - 1;

	while (idx) {
	    if (rand() & 2) {
		mem = ptrs[0];
		ptrs[0] = ptrs[idx];
		ptrs[idx] = mem;

		nfree = sizes[0];
		sizes[0] = sizes[idx];
		sizes[idx] = nfree;
		}
	    idx--;
	    }
	
	/* now free a random number of elements */

	nfree = rand() % items;
	freecnt = 0;

	for (idx = nfree; idx < items; idx++) {
	    kfree(pool,ptrs[idx]);
	    totalsize -= sizes[idx];
	    freecnt++;
	    ptrs[idx] = NULL;
	    sizes[idx] = 0;
	    if (kmemchk(pool,0) < 0) {
		kmemchk(pool,1);
		exit(1);
		}
	    }

	items -= freecnt;

	printf(".");

	}

    kmemchk(pool,1);

    exit(0);
}

#endif	 /* TESTPROG */
