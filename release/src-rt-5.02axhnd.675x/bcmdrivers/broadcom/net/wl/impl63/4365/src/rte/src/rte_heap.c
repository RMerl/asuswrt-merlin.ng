/** @file hnd_heap.c
 *
 * HND heap management.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_heap.c 672151 2016-11-24 11:30:07Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>

#include <rte_mem.h>
#include "rte_mem_priv.h"
#include <rte_heap.h>
#include "rte_heap_priv.h"
#include <rte_cons.h>

#include <osl.h>
#include <osl_ext.h>
#include <bcm_buzzz.h>

/* debug */
#ifdef BCMDBG
#define HND_MSG(x) printf x
#else
#define HND_MSG(x)
#endif // endif

#include <event_log.h>

/*
 * ======RTE====== Memory allocation:
 *	hnd_free(w): Free previously allocated memory at w
 *	hnd_malloc_align(size, abits): Allocate memory at a 2^abits boundary
 *	hnd_memavail(): Return a (slightly optimistic) estimate of free memory
 *	hnd_hwm(): Return a high watermark of allocated memory
 *	hnd_print_heapuse(): Dump heap usage stats.
 *	hnd_print_memwaste(): Malloc memory to simulate low memory environment
 *	hnd_print_malloc(): (BCMDBG_MEM) Dump free & inuse lists
 *	hnd_arena_add(base, size): Add a block of memory to the arena
 */
#define	MIN_MEM_SIZE	8	/* Min. memory size is 8 bytes */
#define	MIN_ALIGN	4	/* Alignment at 4 bytes */
#ifdef HNDLBUFCOMPACT
#define	MAX_ALIGN	LB_RAMSIZE
#else
#define	MAX_ALIGN	16384	/* Max alignment at 16k */
#endif // endif
#define	ALIGN(ad, al)	(((ad) + ((al) - 1)) & ~((al) - 1))
#define	ALIGN_DOWN(ad, al)	((ad) & ~((al) - 1))

#ifdef BCMDBG_MEM

typedef struct _mem_dbg {
	uint32		magic;
	uchar		*malloc_function;
	uchar		*free_function;
	const char	*file;
	int		line;
	uint32		size;
	struct _mem_dbg	*next;
} mem_t;

static mem_t	free_mem_dbg;		/* Free list head */

/* Note: the dummy_mem structure and pointer is purely to satisfy the rom symbol size check.
 * free_mem is not used.
 */
typedef struct _dummy_mem {
	uint32		size;
	struct _dummy_mem	*next;
} dummy_mem_t;

static dummy_mem_t	free_mem;	/* Free list head */
dummy_mem_t *dummy_mem_p = &free_mem;

#else /* ! BCMDBG_MEM */

typedef struct _mem {
	uint32		size;
	struct _mem	*next;
} mem_t;

static mem_t	free_mem;		/* Free list head */

static mem_t* hnd_freemem_get(void);
#endif /* BCMDBG_MEM */

static uint	arena_size;		/* Total heap size */
static uint	inuse_size;		/* Current in use */
static uint	inuse_overhead;		/* tally of allocated mem_t blocks */
static uint	inuse_hwm;		/* High watermark of memory - reclaimed memory */
static uint	mf_count;		/* Malloc failure count */

/* mutex macros for thread safe */
#ifdef HND_HEAP_THREAD_SAFE
#define HND_HEAP_MUTEX_DECL(mutex)		OSL_EXT_MUTEX_DECL(mutex)
#define HND_HEAP_MUTEX_CREATE(name, mutex)	osl_ext_mutex_create(name, mutex)
#define HND_HEAP_MUTEX_DELETE(mutex)		osl_ext_mutex_delete(mutex)
#define HND_HEAP_MUTEX_ACQUIRE(mutex, msec)	osl_ext_mutex_acquire(mutex, msec)
#define HND_HEAP_MUTEX_RELEASE(mutex)	osl_ext_mutex_release(mutex)
#else
#define HND_HEAP_MUTEX_DECL(mutex)
#define HND_HEAP_MUTEX_CREATE(name, mutex)	OSL_EXT_SUCCESS
#define HND_HEAP_MUTEX_DELETE(mutex)		OSL_EXT_SUCCESS
#define HND_HEAP_MUTEX_ACQUIRE(mutex, msec)	OSL_EXT_SUCCESS
#define HND_HEAP_MUTEX_RELEASE(mutex)	OSL_EXT_SUCCESS
#endif	/* HND_HEAP_THREAD_SAFE */

HND_HEAP_MUTEX_DECL(heap_mutex);

#ifdef EVENT_LOG_COMPILE
#define EVENT_LOG_IF_READY(_tag, _format, _count, _size, _address, _ra, _success) \
	do {                                \
		if (event_log_is_ready()) {             \
			EVENT_LOG(_tag, _format, _count, _size, _address, _ra, _success); \
		}                           \
	}                               \
	while (0)
static uint malloc_event_count = 0;
#define INC_MALLOC_EVENT_COUNT (malloc_event_count++)
#else /* EVENT_LOG_COMPILE */
#define EVENT_LOG_IF_READY(_tag, _format, _count, _size, _address, _ra, _success)
#define INC_MALLOC_EVENT_COUNT /* prevents ROM abandons on eg 4365c0 */
#endif /* EVENT_LOG_COMPILE */

#ifdef BCMDBG_MEM
#define	MEM_MAGIC	0x4d4e4743	/* Magic # for mem overwrite check: 'MNGC' */

static mem_t	inuse_mem;		/* In-use list head */

static void hnd_print_malloc(void *arg, int argc, char *argv[]);

#else /* BCMDBG_MEM */

static mem_t*
BCMRAMFN(hnd_freemem_get)(void)
{
	return (&free_mem);
}
#endif /* BCMDBG_MEM */

bool
BCMATTACHFN(hnd_arena_init)(uintptr base, uintptr lim)
{
	mem_t *first;
#ifndef BCMDBG_MEM
	mem_t *free_memptr;
#endif // endif

	ASSERT(base);
	ASSERT(lim > base);

	/* create mutex for critical section locking */
	if (HND_HEAP_MUTEX_CREATE("heap_mutex", &heap_mutex) != OSL_EXT_SUCCESS) {
		return FALSE;
	}

	/* Align */
	first = (mem_t *)ALIGN(base, MIN_ALIGN);

	arena_size = lim - (uint32)first;
	inuse_size = 0;
	inuse_overhead = 0;
	inuse_hwm = 0;

	mf_count = 0;
#ifdef BCMDBG_MEM
	free_mem_dbg.magic = inuse_mem.magic = first->magic = MEM_MAGIC;
	inuse_mem.next = NULL;
#endif /* BCMDBG_MEM */
	first->size = arena_size - sizeof(mem_t);
	first->next = NULL;
#ifdef BCMDBG_MEM
	free_mem_dbg.next = first;
#else
	free_memptr = hnd_freemem_get();
	free_memptr->next = first;
#endif /* BCMDBG_MEM */
	return TRUE;
}

uint
hnd_arena_add(uint32 base, uint size)
{
	uint32 addr;
	mem_t *this;

	addr = ALIGN(base, MIN_ALIGN);
	if ((addr - base) > size) {
		/* Ignore this miniscule thing,
		 * otherwise size below will go negative!
		 */
		return 0;
	}
	size -= (addr - base);
	size = ALIGN_DOWN(size, MIN_ALIGN);

	if (size < (sizeof(mem_t) + MIN_MEM_SIZE)) {
		return 0;
	}

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;

	this = (mem_t *)addr;
	arena_size += size;
	size -= sizeof(mem_t);
	addr += sizeof(mem_t);
	this->size = size;

	/* This chunk was not in use before, make believe it was */
	inuse_size += size;
	inuse_overhead += sizeof(mem_t);

#ifdef BCMDBG_MEM
	this->magic = MEM_MAGIC;
	this->file = NULL;
	this->line = 0;
	this->next = inuse_mem.next;
	inuse_mem.next = this;
	printf("%s: Adding %p: 0x%x(%d) @ 0x%x\n", __FUNCTION__, this, size, size, addr);
#else
	this->next = NULL;
#endif /* BCMDBG_MEM */

	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return 0;

	hnd_free((void *)addr);
	return (size);
}

void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hnd_malloc_align(uint size, uint alignbits, const char *file, int line)
#else
hnd_malloc_align(uint size, uint alignbits)
#endif /* BCMDBG_MEM */
{
	mem_t	*curr, *last, *this = NULL, *prev = NULL;
	uint	align, rem, waste;
	uintptr	addr = 0, top = 0;
	uint	inuse_total;
	void	*ptr = NULL;

#ifdef BCMDBG_MEM
	const char *basename;
#endif /* BCMDBG_MEM */

	ASSERT(size);

	size = ALIGN(size, MIN_ALIGN);

	BUZZZ_LVL5(HND_MALLOC, 2, (uint32)__builtin_return_address(0), size);

	align = 1 << alignbits;
	if (align <= MIN_ALIGN)
		align = MIN_ALIGN;
	else if (align > MAX_ALIGN)
		align = MAX_ALIGN;

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return  NULL;

	/* Search for available memory */
#ifdef BCMDBG_MEM
	last = &free_mem_dbg;
#else
	last = hnd_freemem_get();
#endif /* BCMDBG_MEM */
	waste = arena_size;

	/* Algorithm: best fit */
	while ((curr = last->next) != NULL) {
		if (curr->size >= size) {
			/* Calculate alignment */
			uintptr lowest = (uintptr)curr + sizeof(mem_t);
			uintptr end = lowest + curr->size;
			uintptr highest = end - size;
			uintptr a = ALIGN_DOWN(highest, align);

			/* Find closest sized buffer to avoid fragmentation BUT aligned address
			 * must be greater or equal to the lowest address available in the free
			 * block AND lowest address is aligned with "align" bytes OR
			 * space preceeding a returned block's header is either big
			 * enough to support another free block
			 */
			if (a >= lowest &&
			    (ISALIGNED(lowest, align) || (a-lowest) >= sizeof(mem_t)) &&
			    (curr->size - size) < waste)
			{

				waste = curr->size - size;
				this = curr;
				prev = last;
				top = end;
				addr = a;

				if (waste == 0)
					break;
			}
		}
		last = curr;
	}

	if (this == NULL) {
		mf_count++; /* Increment malloc failure count */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
		printf("No memory to satisfy request for %d bytes, inuse %d, file %s, line %d\n",
		       size, (inuse_size + inuse_overhead), file ? file : "unknown", line);
#ifdef BCMDBG_MEM
		hnd_print_malloc(NULL, 0, NULL);
#endif // endif
#else
		HND_MSG(("No memory to satisfy request %d bytes, inuse %d\n", size,
		         (inuse_size + inuse_overhead)));
#endif /* BCMDBG_MEM */
		goto done;
	}

#ifdef BCMDBG_MEM
	ASSERT(this->magic == MEM_MAGIC);
#endif // endif

#ifdef GLOBAL_STACK
	hnd_stack_check();
#endif // endif

	/* best fit has been found as below
	 *  - split above or below if tht's big enough
	 *  - otherwise adjust size to absorb those tiny gap
	 *
	 *      ----------------  <-- this
	 *          mem_t
	 *      ----------------
	 *
	 *       waste(not used)
	 *
	 *      ----------------  <-- addr
	 *      alignment offset
	 *      ----------------
	 *	    size
	 *
	 *      ----------------  <-- top
	 */

	/* Anything above? */
	rem = (top - addr) - size;
	if (rem < sizeof(mem_t)) {
		/* take it all */
		size += rem;
	} else {
		/* Split off the top */
		mem_t *new = (mem_t *)(addr + size);

		this->size -= rem;
		new->size = rem - sizeof(mem_t);
#ifdef BCMDBG_MEM
		new->magic = MEM_MAGIC;
#endif /* BCMDBG_MEM */
		new->next = this->next;
		this->next = new;
	}

	/* Anything below? */
	rem = this->size - size;
	if (rem < sizeof(mem_t)) {
		/* take it all */
		prev->next = this->next;
	} else {
		/* Split this */
		mem_t *new = (mem_t *)((uint32)this + rem);

		new->size = size;
		this->size = rem - sizeof(mem_t);
#ifdef BCMDBG_MEM
		new->magic = MEM_MAGIC;
#endif /* BCMDBG_MEM */

		this = new;
	}

#ifdef BCMDBG_MEM
	this->next = inuse_mem.next;
	inuse_mem.next = this;
	this->line = line;
	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;
	if (!basename)
		basename = file;
	this->file = basename;
	this->malloc_function = __builtin_return_address(0);
#else
	this->next = NULL;
#endif /* BCMDBG_MEM */
	inuse_size += this->size;
	inuse_overhead += sizeof(mem_t);

	/* find the instance where the free memory was the least to calculate
	 * inuse memory hwm
	 */
	inuse_total = inuse_size + inuse_overhead;
	if (inuse_total > inuse_hwm)
		inuse_hwm = inuse_total;

#ifdef BCMDBG_MEM
	HND_MSG(("malloc: 0x%x\n", (uint32) ((void *)((uintptr)this + sizeof(mem_t)))));
#endif // endif

	ptr = (void *)((uint32)this + sizeof(mem_t));
done:
	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS) {
		if (ptr)
			hnd_free(ptr);
		return NULL;
	}
	return ptr;
}

#ifdef HNDLBUFCOMPACT
/* HNDLBUFCOMPACT is implemented based on an assumption that
 * lbuf head and end addresses falls into the same 2M bytes address boundary.
 *
 * However, ATCM and BTCM are spanning over 2M address boundary in many dongle chips.
 * So, if lbuf is allocated at the end of ATCM, it could cross 2M boundary over to BTCM.
 *
 * In order to avoid this kind of situation, we make a hole of 4 bytes memory at 2M address.
 * This function allocates 4 bytes memory at all possible 2M aligned addresses.
 *
 * This function must be called whenever new memory region is added to arena.
 * (i.e., after hnd_arena_init() or hnd_arena_add())
*/
void
hnd_lbuf_fixup_2M_tcm(void)
{
	/* Reserving 4 bytes memory at all 2M boundary to create a hole */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	while (hnd_malloc_align(4, 21, __FILE__, __LINE__) != NULL)
#else
	while (hnd_malloc_align(4, 21) != NULL)
#endif // endif
		;	/* empty */

	mf_count--;	/* decrement malloc failure count to avoid confusion,
			   because this was an intended/expected failure by the logic
			*/
}
#endif /* HNDLBUFCOMPACT */

static int
hnd_malloc_size(void *where)
{
	uint32 w = (uint32)where;
	mem_t *this;

#ifdef BCMDBG_MEM
	mem_t *prev;

	/* Get it off of the inuse list */
	prev = &inuse_mem;
	while ((this = prev->next) != NULL) {
		if (((uint32)this + sizeof(mem_t)) == w)
			break;
		prev = this;
	}

	if (this == NULL) {
		HND_MSG(("%s: 0x%x is not in the inuse list\n", __FUNCTION__, w));
		ASSERT(this);
		return -1;
	}

	if (this->magic != MEM_MAGIC) {
		HND_MSG(("\n%s: Corrupt magic (0x%x) in 0x%x; size %d; file %s, line %d\n\n",
		       __FUNCTION__, this->magic, w, this->size, this->file, this->line));
		ASSERT(this->magic == MEM_MAGIC);
		return -1;
	}

#else
	this = (mem_t *)(w - sizeof(mem_t));
#endif /* BCMDBG_MEM */

	return this->size;
}

void *
hnd_realloc(void *ptr, uint size)
{
	int osz = hnd_malloc_size(ptr);

	if (osz < 0)
		return NULL;

	void *new = hnd_malloc(size);
	if (new == NULL)
		return NULL;
	memcpy(new, ptr, MIN(size, osz));
	hnd_free(ptr);
	return new;
}

int
hnd_free(void *where)
{
	uint32 w = (uint32)where;
	mem_t *prev, *next, *this;
	int err = 0;

	BUZZZ_LVL5(HND_FREE, 1, (uint32)__builtin_return_address(0));

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return -1;

#ifdef BCMDBG_MEM
	/* Get it off of the inuse list */
	prev = &inuse_mem;
	while ((this = prev->next) != NULL) {
		if (((uint32)this + sizeof(mem_t)) == w)
			break;
		prev = this;
	}

	if (this == NULL) {
		HND_MSG(("%s: 0x%x is not in the inuse list\n", __FUNCTION__, w));
		ASSERT(this);
		EVENT_LOG_IF_READY(EVENT_LOG_TAG_MEM_FREE,
		          "0x%06x FREE(FAIL) - sz: 0x%08x, ad: 0x%08x, ra: 0x%08x, succ: %d\n",
		          malloc_event_count, 0, (uint32) where,
		          (uint32)__builtin_return_address(0), 0);
		INC_MALLOC_EVENT_COUNT;
		err = -1;
		goto done;
	}

	if (this->magic != MEM_MAGIC) {
		HND_MSG(("\n%s: Corrupt magic (0x%x) in 0x%x; size %d; file %s, line %d\n\n",
		       __FUNCTION__, this->magic, w, this->size, this->file, this->line));
		ASSERT(this->magic == MEM_MAGIC);
		EVENT_LOG_IF_READY(EVENT_LOG_TAG_MEM_FREE,
		        "0x%06x FREE(FAIL) - sz: 0x%08x, ad: 0x%08x, ra: 0x%08x, succ: %d\n",
		        malloc_event_count, 0, (uint32) where,
		        (uint32)__builtin_return_address(0), 0);
		INC_MALLOC_EVENT_COUNT;
		err = -1;
		goto done;
	}

	this->free_function = __builtin_return_address(0);
	prev->next = this->next;
#else
	this = (mem_t *)(w - sizeof(mem_t));
#endif /* BCMDBG_MEM */

	inuse_size -= this->size;
	inuse_overhead -= sizeof(mem_t);

	/* Find the right place in the free list for it */
#ifdef BCMDBG_MEM
	prev = &free_mem_dbg;
#else
	prev = hnd_freemem_get();
#endif /* BCMDBG_MEM */
	while ((next = prev->next) != NULL) {
		if (next >= this)
			break;
		prev = next;
	}

	/* Coalesce with next if appropriate */
	if ((w + this->size) == (uint32)next) {
		this->size += next->size + sizeof(mem_t);
		this->next = next->next;
#ifdef BCMDBG_MEM
		next->magic = 0;
#endif /* BCMDBG_MEM */
	} else
		this->next = next;

	/* Coalesce with prev if appropriate */
	if (((uint32)prev + sizeof(mem_t) + prev->size) == (uint32)this) {
		prev->size += this->size + sizeof(mem_t);
		prev->next = this->next;
#ifdef BCMDBG_MEM
		this->magic = 0;
#endif /* BCMDBG_MEM */
	} else
		prev->next = this;

	EVENT_LOG_IF_READY(EVENT_LOG_TAG_MEM_FREE,
	          "0x%06x FREE(SUCC) - sz: 0x%08x, ad: 0x%08x, ra: 0x%08x, succ: %d\n",
	          malloc_event_count, inuse_size, (uint32) where,
	          (uint32)__builtin_return_address(0), 1);
	INC_MALLOC_EVENT_COUNT;

	err = 0;
#ifdef BCMDBG_MEM
done:
#endif /* BCMDBG_MEM */
	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return -1;
	return err;
}

void *
hnd_calloc(uint num, uint size)
{
	void *ptr;

	ptr = hnd_malloc(size*num);
	if (ptr)
		bzero(ptr, size*num);

	return (ptr);
}

uint
hnd_memavail(void)
{
	uint mem_avail;

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 0;
	mem_avail = arena_size - inuse_size - inuse_overhead;
	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return 0;
	return (mem_avail);
}

void
hnd_meminuse(uint *inuse, uint *inuse_oh)
{
	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;
	if (inuse != NULL)
		*inuse = inuse_size;
	if (inuse_oh != NULL)
		*inuse_oh = inuse_overhead;
	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return;
}

uint
hnd_hwm(void)
{
	return (inuse_hwm);
}

#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)

/*
* Function to get heap memory usage
* for both dhd cons hu and wl memuse commands
*/

int
hnd_get_heapuse(memuse_info_t *mu)
{
	hnd_image_info_t info;
	size_t rodata_len;

	if (mu == NULL)
		return -1;

	hnd_image_info(&info);

	mu->ver = 0;

	mu->text_len = (info._text_end - info._text_start);
	rodata_len = (info._rodata_end - info._rodata_start);
	mu->data_len = (info._data_end - info._data_start);
	mu->bss_len = (info._bss_end - info._bss_start);

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return -1;

	mu->arena_size = arena_size;
	mu->inuse_size = inuse_size;
	mu->inuse_hwm = inuse_hwm;
	mu->inuse_overhead = inuse_overhead;

	mu->text_len += rodata_len;
	mu->tot = mu->text_len + mu->data_len + mu->bss_len;

	mu->tot += mu->inuse_hwm;

	mu->inuse_total = mu->inuse_size + mu->inuse_overhead;
	mu->arena_free = mu->arena_size - mu->inuse_total;

	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return -1;
	return 0;
}

/*
* Function to print heap memory usage for dhd cons hu command
*/

static void
hnd_print_heapuse(void *arg, int argc, char *argv[])
{
	memuse_info_t mu;
	int ret;

	if ((ret = hnd_get_heapuse(&mu)) != 0)
		return;

	printf("Memory usage:\n");
	printf("\tText: %d(%dK), Data: %d(%dK), Bss: %d(%dK)\n",
	       mu.text_len, KB(mu.text_len),
	       mu.data_len, KB(mu.data_len),
	       mu.bss_len, KB(mu.bss_len));
	printf("\tArena total: %d(%dK), Free: %d(%dK), In use: %d(%dK), HWM: %d(%dK)\n",
	       mu.arena_size, KB(mu.arena_size),
	       mu.arena_free, KB(mu.arena_free),
	       mu.inuse_size, KB(mu.inuse_size),
	       mu.inuse_hwm, KB(mu.inuse_hwm));
	printf("\tIn use + overhead: %d(%dK), Max memory in use: %d(%dK)\n",
	       mu.inuse_total, KB(mu.inuse_total),
	       mu.tot, KB(mu.tot));
	printf("\tMalloc failure count: %d\n", mf_count);

}
#endif /* RTE_CONS || BCM_OL_DEV */
#endif /* BCM_BOOTLOADER */

#ifdef BCMDBG_MEM
int
hnd_memcheck(char *file, int line)
{
	mem_t *this = NULL;
	int err = 0;

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return 1;

	this = inuse_mem.next;
	while (this) {
		if (this->magic != MEM_MAGIC) {
			printf("CORRUPTION: %s %d\n", file, line);
			printf("\n%s: Corrupt magic (0x%x); size %d; file %s, line %d\n\n",
			       __FUNCTION__, this->magic, this->size, this->file, this->line);
			err = 1;
			goto out;
		}
		this = this->next;
	}
out:
	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return 1;
	return err;
}

static void
hnd_print_malloc(void *arg, int argc, char *argv[])
{
	uint32 inuse = 0, free = 0, total;
	mem_t *this;

	printf("Heap inuse list:\n");
	printf("Addr\t\tSize\tfile:line\t\tmalloc fn\n");

	if (HND_HEAP_MUTEX_ACQUIRE(&heap_mutex, OSL_EXT_TIME_FOREVER) != OSL_EXT_SUCCESS)
		return;

	this = inuse_mem.next;

	while (this) {
		printf("%p\t%5d\t%s:%d\t%p\t%p\n",
		       &this[1], this->size,
		       this->file, this->line, this->malloc_function, this->free_function);
		inuse += this->size + sizeof(mem_t);
		this = this->next;
	}
	printf("Heap free list:\n");
	this = free_mem_dbg.next;
	while (this) {
		printf("%p: 0x%x(%d) @ %p\n",
		       this, this->size, this->size, &this[1]);
		free += this->size + sizeof(mem_t);
		this = this->next;
	}
	total = inuse + free;
	printf("Heap Dyn inuse: 0x%x(%d), inuse: 0x%x(%d)\nDyn free: 0x%x(%d), free: 0x%x(%d)\n",
	       inuse, inuse, inuse_size, inuse_size, free, free,
	       (arena_size - inuse_size), (arena_size - inuse_size));
	if (total != arena_size)
		printf("Total (%d) does NOT agree with original %d!\n",
		       total, arena_size);

	if (HND_HEAP_MUTEX_RELEASE(&heap_mutex) != OSL_EXT_SUCCESS)
		return;
}
#endif /* BCMDBG_MEM */

/* Must be called after hnd_cons_init() which depends on heap to be available */
void
BCMATTACHFN(hnd_heap_cli_init)(void)
{
#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
	hnd_cons_add_cmd("hu", hnd_print_heapuse, 0);
#endif // endif
#if defined(RTE_CONS) && defined(BCMDBG_MEM)
	hnd_cons_add_cmd("ar", hnd_print_malloc, 0);
#endif // endif
#endif /* BCM_BOOTLOADER */
}
