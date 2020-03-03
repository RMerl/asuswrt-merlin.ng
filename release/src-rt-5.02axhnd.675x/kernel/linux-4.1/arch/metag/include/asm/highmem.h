#ifndef _ASM_HIGHMEM_H
#define _ASM_HIGHMEM_H

#include <asm/cacheflush.h>
#include <asm/kmap_types.h>
#include <asm/fixmap.h>

/*
 * Right now we initialize only a single pte table. It can be extended
 * easily, subsequent pte tables have to be allocated in one physical
 * chunk of RAM.
 */
/*
 * Ordering is (from lower to higher memory addresses):
 *
 * high_memory
 *			Persistent kmap area
 * PKMAP_BASE
 *			fixed_addresses
 * FIXADDR_START
 * FIXADDR_TOP
 *			Vmalloc area
 * VMALLOC_START
 * VMALLOC_END
 */
#define PKMAP_BASE		(FIXADDR_START - PMD_SIZE)
#define LAST_PKMAP		PTRS_PER_PTE
#define LAST_PKMAP_MASK		(LAST_PKMAP - 1)
#define PKMAP_NR(virt)		(((virt) - PKMAP_BASE) >> PAGE_SHIFT)
#define PKMAP_ADDR(nr)		(PKMAP_BASE + ((nr) << PAGE_SHIFT))

#define kmap_prot		PAGE_KERNEL

static inline void flush_cache_kmaps(void)
{
	flush_cache_all();
}

/* declarations for highmem.c */
extern unsigned long highstart_pfn, highend_pfn;

extern pte_t *pkmap_page_table;

extern void *kmap_high(struct page *page);
extern void kunmap_high(struct page *page);

extern void kmap_init(void);

/*
 * The following functions are already defined by <linux/highmem.h>
 * when CONFIG_HIGHMEM is not set.
 */
#ifdef CONFIG_HIGHMEM
extern void *kmap(struct page *page);
extern void kunmap(struct page *page);
extern void *kmap_atomic(struct page *page);
extern void __kunmap_atomic(void *kvaddr);
extern void *kmap_atomic_pfn(unsigned long pfn);
extern struct page *kmap_atomic_to_page(void *ptr);
#endif

#endif
