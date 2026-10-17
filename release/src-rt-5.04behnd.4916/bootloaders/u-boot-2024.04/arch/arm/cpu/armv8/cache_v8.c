// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2016
 * Alexander Graf <agraf@suse.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)

/*
 *  With 4k page granule, a virtual address is split into 4 lookup parts
 *  spanning 9 bits each:
 *
 *    _______________________________________________
 *   |       |       |       |       |       |       |
 *   |   0   |  Lv0  |  Lv1  |  Lv2  |  Lv3  |  off  |
 *   |_______|_______|_______|_______|_______|_______|
 *     63-48   47-39   38-30   29-21   20-12   11-00
 *
 *             mask        page size
 *
 *    Lv0: FF8000000000       --
 *    Lv1:   7FC0000000       1G
 *    Lv2:     3FE00000       2M
 *    Lv3:       1FF000       4K
 *    off:          FFF
 */

static int get_effective_el(void)
{
	int el = current_el();

	if (el == 2) {
		u64 hcr_el2;

		/*
		 * If we are using the EL2&0 translation regime, the TCR_EL2
		 * looks like the EL1 version, even though we are in EL2.
		 */
		__asm__ ("mrs %0, HCR_EL2\n" : "=r" (hcr_el2));
		if (hcr_el2 & BIT(HCR_EL2_E2H_BIT))
			return 1;
	}

	return el;
}

u64 get_tcr(u64 *pips, u64 *pva_bits)
{
	int el = get_effective_el();
	u64 max_addr = 0;
	u64 ips, va_bits;
	u64 tcr;
	int i;

	/* Find the largest address we need to support */
	for (i = 0; mem_map[i].size || mem_map[i].attrs; i++)
		max_addr = max(max_addr, mem_map[i].virt + mem_map[i].size);

	/* Calculate the maximum physical (and thus virtual) address */
	if (max_addr > (1ULL << 44)) {
		ips = 5;
		va_bits = 48;
	} else  if (max_addr > (1ULL << 42)) {
		ips = 4;
		va_bits = 44;
	} else  if (max_addr > (1ULL << 40)) {
		ips = 3;
		va_bits = 42;
	} else  if (max_addr > (1ULL << 36)) {
		ips = 2;
		va_bits = 40;
	} else  if (max_addr > (1ULL << 32)) {
		ips = 1;
		va_bits = 36;
	} else {
		ips = 0;
		va_bits = 32;
	}

	if (el == 1) {
		tcr = TCR_EL1_RSVD | (ips << 32) | TCR_EPD1_DISABLE;
	} else if (el == 2) {
		tcr = TCR_EL2_RSVD | (ips << 16);
	} else {
		tcr = TCR_EL3_RSVD | (ips << 16);
	}

	/* PTWs cacheable, inner/outer WBWA and inner shareable */
	tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
	tcr |= TCR_T0SZ(va_bits);

	if (pips)
		*pips = ips;
	if (pva_bits)
		*pva_bits = va_bits;

	return tcr;
}

#define MAX_PTE_ENTRIES 512

static int pte_type(u64 *pte)
{
	return *pte & PTE_TYPE_MASK;
}

/* Returns the LSB number for a PTE on level <level> */
static int level2shift(int level)
{
	/* Page is 12 bits wide, every level translates 9 bits */
	return (12 + 9 * (3 - level));
}

static u64 *find_pte(u64 addr, int level)
{
	int start_level = 0;
	u64 *pte;
	u64 idx;
	u64 va_bits;
	int i;

	debug("addr=%llx level=%d\n", addr, level);

	get_tcr(NULL, &va_bits);
	if (va_bits < 39)
		start_level = 1;

	if (level < start_level)
		return NULL;

	/* Walk through all page table levels to find our PTE */
	pte = (u64*)gd->arch.tlb_addr;
	for (i = start_level; i < 4; i++) {
		idx = (addr >> level2shift(i)) & 0x1FF;
		pte += idx;
		debug("idx=%llx PTE %p at level %d: %llx\n", idx, pte, i, *pte);

		/* Found it */
		if (i == level)
			return pte;
		/* PTE is no table (either invalid or block), can't traverse */
		if (pte_type(pte) != PTE_TYPE_TABLE)
			return NULL;
		/* Off to the next level */
		pte = (u64*)(*pte & 0x0000fffffffff000ULL);
	}

	/* Should never reach here */
	return NULL;
}

#ifdef CONFIG_CMO_BY_VA_ONLY
static void __cmo_on_leaves(void (*cmo_fn)(unsigned long, unsigned long),
			    u64 pte, int level, u64 base)
{
	u64 *ptep;
	int i;

	ptep = (u64 *)(pte & GENMASK_ULL(47, PAGE_SHIFT));
	for (i = 0; i < PAGE_SIZE / sizeof(u64); i++) {
		u64 end, va = base + i * BIT(level2shift(level));
		u64 type, attrs;

		pte = ptep[i];
		type = pte & PTE_TYPE_MASK;
		attrs = pte & PMD_ATTRINDX_MASK;
		debug("PTE %llx at level %d VA %llx\n", pte, level, va);

		/* Not valid? next! */
		if (!(type & PTE_TYPE_VALID))
			continue;

		/* Not a leaf? Recurse on the next level */
		if (!(type == PTE_TYPE_BLOCK ||
		      (level == 3 && type == PTE_TYPE_PAGE))) {
			__cmo_on_leaves(cmo_fn, pte, level + 1, va);
			continue;
		}

		/*
		 * From this point, this must be a leaf.
		 *
		 * Start excluding non memory mappings
		 */
		if (attrs != PTE_BLOCK_MEMTYPE(MT_NORMAL) &&
		    attrs != PTE_BLOCK_MEMTYPE(MT_NORMAL_NC))
			continue;

		end = va + BIT(level2shift(level)) - 1;

		/* No intersection with RAM? */
		if (end < gd->ram_base ||
		    va >= (gd->ram_base + gd->ram_size))
			continue;

		/*
		 * OK, we have a partial RAM mapping. However, this
		 * can cover *more* than the RAM. Yes, u-boot is
		 * *that* braindead. Compute the intersection we care
		 * about, and not a byte more.
		 */
		va = max(va, (u64)gd->ram_base);
		end = min(end, gd->ram_base + gd->ram_size);

		debug("Flush PTE %llx at level %d: %llx-%llx\n",
		      pte, level, va, end);
		cmo_fn(va, end);
	}
}

static void apply_cmo_to_mappings(void (*cmo_fn)(unsigned long, unsigned long))
{
	u64 va_bits;
	int sl = 0;

	if (!gd->arch.tlb_addr)
		return;

	get_tcr(NULL, &va_bits);
	if (va_bits < 39)
		sl = 1;

	__cmo_on_leaves(cmo_fn, gd->arch.tlb_addr, sl, 0);
}
#else
static inline void apply_cmo_to_mappings(void *dummy) {}
#endif

/* Returns and creates a new full table (512 entries) */
static u64 *create_table(void)
{
	u64 *new_table = (u64*)gd->arch.tlb_fillptr;
	u64 pt_len = MAX_PTE_ENTRIES * sizeof(u64);

	/* Allocate MAX_PTE_ENTRIES pte entries */
	gd->arch.tlb_fillptr += pt_len;

	if (gd->arch.tlb_fillptr - gd->arch.tlb_addr > gd->arch.tlb_size)
		panic("Insufficient RAM for page table: 0x%lx > 0x%lx. "
		      "Please increase the size in get_page_table_size()",
			gd->arch.tlb_fillptr - gd->arch.tlb_addr,
			gd->arch.tlb_size);

#ifndef CONFIG_BCMBCA_IKOS_SKIP_PGTBL_CLEAN
	/* Mark all entries as invalid */
	memset(new_table, 0, pt_len);
#endif

	return new_table;
}

static void set_pte_table(u64 *pte, u64 *table)
{
	/* Point *pte to the new table */
	debug("Setting %p to addr=%p\n", pte, table);
	*pte = PTE_TYPE_TABLE | (ulong)table;
}

/* Splits a block PTE into table with subpages spanning the old block */
static void split_block(u64 *pte, int level)
{
	u64 old_pte = *pte;
	u64 *new_table;
	u64 i = 0;
	/* level describes the parent level, we need the child ones */
	int levelshift = level2shift(level + 1);

	if (pte_type(pte) != PTE_TYPE_BLOCK)
		panic("PTE %p (%llx) is not a block. Some driver code wants to "
		      "modify dcache settings for an range not covered in "
		      "mem_map.", pte, old_pte);

	new_table = create_table();
	debug("Splitting pte %p (%llx) into %p\n", pte, old_pte, new_table);

	for (i = 0; i < MAX_PTE_ENTRIES; i++) {
		new_table[i] = old_pte | (i << levelshift);

		/* Level 3 block PTEs have the table type */
		if ((level + 1) == 3)
			new_table[i] |= PTE_TYPE_TABLE;

		debug("Setting new_table[%lld] = %llx\n", i, new_table[i]);
	}

	/* Set the new table into effect */
	set_pte_table(pte, new_table);
}

static void map_range(u64 virt, u64 phys, u64 size, int level,
		      u64 *table, u64 attrs)
{
	u64 map_size = BIT_ULL(level2shift(level));
	int i, idx;

	idx = (virt >> level2shift(level)) & (MAX_PTE_ENTRIES - 1);
	for (i = idx; size; i++) {
		u64 next_size, *next_table;

		if (level >= 1 &&
		    size >= map_size && !(virt & (map_size - 1))) {
			if (level == 3)
				table[i] = phys | attrs | PTE_TYPE_PAGE;
			else
				table[i] = phys | attrs;

			virt += map_size;
			phys += map_size;
			size -= map_size;

			continue;
		}

		/* Going one level down */
		if (pte_type(&table[i]) == PTE_TYPE_FAULT)
			set_pte_table(&table[i], create_table());

		next_table = (u64 *)(table[i] & GENMASK_ULL(47, PAGE_SHIFT));
		next_size = min(map_size - (virt & (map_size - 1)), size);

		map_range(virt, phys, next_size, level + 1, next_table, attrs);

		virt += next_size;
		phys += next_size;
		size -= next_size;
	}
}

static void add_map(struct mm_region *map)
{
	u64 attrs = map->attrs | PTE_TYPE_BLOCK | PTE_BLOCK_AF;
	u64 va_bits;
	int level = 0;

	get_tcr(NULL, &va_bits);
	if (va_bits < 39)
		level = 1;

	map_range(map->virt, map->phys, map->size, level,
		  (u64 *)gd->arch.tlb_addr, attrs);
}

static void count_range(u64 virt, u64 size, int level, int *cntp)
{
	u64 map_size = BIT_ULL(level2shift(level));
	int i, idx;

	idx = (virt >> level2shift(level)) & (MAX_PTE_ENTRIES - 1);
	for (i = idx; size; i++) {
		u64 next_size;

		if (level >= 1 &&
		    size >= map_size && !(virt & (map_size - 1))) {
			virt += map_size;
			size -= map_size;

			continue;
		}

		/* Going one level down */
		(*cntp)++;
		next_size = min(map_size - (virt & (map_size - 1)), size);

		count_range(virt, next_size, level + 1, cntp);

		virt += next_size;
		size -= next_size;
	}
}

static int count_ranges(void)
{
	int i, count = 0, level = 0;
	u64 va_bits;

	get_tcr(NULL, &va_bits);
	if (va_bits < 39)
		level = 1;

	for (i = 0; mem_map[i].size || mem_map[i].attrs; i++)
		count_range(mem_map[i].virt, mem_map[i].size, level, &count);

	return count;
}

/* Returns the estimated required size of all page tables */
__weak u64 get_page_table_size(void)
{
	u64 one_pt = MAX_PTE_ENTRIES * sizeof(u64);
	u64 size;

	/* Account for all page tables we would need to cover our memory map */
	size = one_pt * count_ranges();

	/*
	 * We need to duplicate our page table once to have an emergency pt to
	 * resort to when splitting page tables later on
	 */
	size *= 2;

	/*
	 * We may need to split page tables later on if dcache settings change,
	 * so reserve up to 4 (random pick) page tables for that.
	 */
	size += one_pt * 4;

	return size;
}

void setup_pgtables(void)
{
	int i;

	if (!gd->arch.tlb_fillptr || !gd->arch.tlb_addr)
		panic("Page table pointer not setup.");

	/*
	 * Allocate the first level we're on with invalidate entries.
	 * If the starting level is 0 (va_bits >= 39), then this is our
	 * Lv0 page table, otherwise it's the entry Lv1 page table.
	 */
	create_table();

	/* Now add all MMU table entries one after another to the table */
	for (i = 0; mem_map[i].size || mem_map[i].attrs; i++)
		add_map(&mem_map[i]);
}

static void setup_all_pgtables(void)
{
	u64 tlb_addr = gd->arch.tlb_addr;
	u64 tlb_size = gd->arch.tlb_size;

	/* Reset the fill ptr */
	gd->arch.tlb_fillptr = tlb_addr;

	/* Create normal system page tables */
	setup_pgtables();

	/* Create emergency page tables */
	gd->arch.tlb_size -= (uintptr_t)gd->arch.tlb_fillptr -
			     (uintptr_t)gd->arch.tlb_addr;
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	setup_pgtables();
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
	gd->arch.tlb_addr = tlb_addr;
	gd->arch.tlb_size = tlb_size;
}

/* to activate the MMU we need to set up virtual memory */
__weak void mmu_setup(void)
{
	int el;

	/* Set up page tables only once */
	if (!gd->arch.tlb_fillptr)
		setup_all_pgtables();

	el = current_el();
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr, get_tcr(NULL, NULL),
			  MEMORY_ATTRIBUTES);

	/* enable the mmu */
	set_sctlr(get_sctlr() | CR_M);
}

/*
 * Performs a invalidation of the entire data cache at all levels
 */
void invalidate_dcache_all(void)
{
#ifndef CONFIG_CMO_BY_VA_ONLY
	__asm_invalidate_dcache_all();
	__asm_invalidate_l3_dcache();
#else
	apply_cmo_to_mappings(invalidate_dcache_range);
#endif
}

/*
 * Performs a clean & invalidation of the entire data cache at all levels.
 * This function needs to be inline to avoid using stack.
 * __asm_flush_l3_dcache return status of timeout
 */
inline void flush_dcache_all(void)
{
#ifndef CONFIG_CMO_BY_VA_ONLY
	int ret;

	__asm_flush_dcache_all();
	ret = __asm_flush_l3_dcache();
	if (ret)
		debug("flushing dcache returns 0x%x\n", ret);
	else
		debug("flushing dcache successfully.\n");
#else
	apply_cmo_to_mappings(flush_dcache_range);
#endif
}

#ifndef CONFIG_SYS_DISABLE_DCACHE_OPS
/*
 * Invalidates range in all levels of D-cache/unified cache
 */
void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	__asm_invalidate_dcache_range(start, stop);
}

/*
 * Flush range(clean & invalidate) from all levels of D-cache/unified cache
 */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
	__asm_flush_dcache_range(start, stop);
}
#else
void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}
#endif /* CONFIG_SYS_DISABLE_DCACHE_OPS */

void dcache_enable(void)
{
	/* The data cache is not active unless the mmu is enabled */
	if (!(get_sctlr() & CR_M)) {
		invalidate_dcache_all();
		__asm_invalidate_tlb_all();
		mmu_setup();
	}

	/* Set up page tables only once (it is done also by mmu_setup()) */
	if (!gd->arch.tlb_fillptr)
		setup_all_pgtables();

	set_sctlr(get_sctlr() | CR_C);
}

void dcache_disable(void)
{
	uint32_t sctlr;

	sctlr = get_sctlr();

	/* if cache isn't enabled no need to disable */
	if (!(sctlr & CR_C))
		return;

	if (IS_ENABLED(CONFIG_CMO_BY_VA_ONLY)) {
		/*
		 * When invalidating by VA, do it *before* turning the MMU
		 * off, so that at least our stack is coherent.
		 */
		flush_dcache_all();
	}

	set_sctlr(sctlr & ~(CR_C|CR_M));

	if (!IS_ENABLED(CONFIG_CMO_BY_VA_ONLY))
		flush_dcache_all();

	__asm_invalidate_tlb_all();
}

void dcache_sanitize_disable(uintptr_t sanitize_base_addr, unsigned long sanitize_size)
{
	/* Disable write streaming so cache controller ALWAYS allocates and fill cache lines */
#if defined(CORTEX_A55)
	uint64_t cpuectlr = get_cpuectlr();
	set_cpuectlr(cpuectlr|CPUECTLR_EL1_L3WSCTL|CPUECTLR_EL1_L2WSCTL|CPUECTLR_EL1_L1WSCTL);
#elif defined(CORTEX_A53)
	uint64_t cpuactlr = get_cpuactlr();
	set_cpuactlr(cpuactlr|CPUACTLR_EL1_RADIS|CPUACTLR_EL1_L1RADIS);
#endif

	/* Sanitize memory */
	memset((void*)sanitize_base_addr, 0, sanitize_size);
	
	/* flush and disable dcache */
	dcache_disable();

	/* restore write streaming settings */
#if defined(CORTEX_A55)
	set_cpuectlr(cpuectlr);
#elif defined(CORTEX_A53)
	set_cpuactlr(cpuactlr);
#endif
}

int dcache_status(void)
{
	return (get_sctlr() & CR_C) != 0;
}

u64 *__weak arch_get_page_table(void) {
	puts("No page table offset defined\n");

	return NULL;
}

static bool is_aligned(u64 addr, u64 size, u64 align)
{
	return !(addr & (align - 1)) && !(size & (align - 1));
}

/* Use flag to indicate if attrs has more than d-cache attributes */
static u64 set_one_region(u64 start, u64 size, u64 attrs, bool flag, int level)
{
	int levelshift = level2shift(level);
	u64 levelsize = 1ULL << levelshift;
	u64 *pte = find_pte(start, level);

	/* Can we can just modify the current level block PTE? */
	if (is_aligned(start, size, levelsize)) {
		if (flag) {
			*pte &= ~PMD_ATTRMASK;
			*pte |= attrs & PMD_ATTRMASK;
		} else {
			*pte &= ~PMD_ATTRINDX_MASK;
			*pte |= attrs & PMD_ATTRINDX_MASK;
		}
		debug("Set attrs=%llx pte=%p level=%d\n", attrs, pte, level);

		return levelsize;
	}

	/* Unaligned or doesn't fit, maybe split block into table */
	debug("addr=%llx level=%d pte=%p (%llx)\n", start, level, pte, *pte);

	/* Maybe we need to split the block into a table */
	if (pte_type(pte) == PTE_TYPE_BLOCK)
		split_block(pte, level);

	/* And then double-check it became a table or already is one */
	if (pte_type(pte) != PTE_TYPE_TABLE)
		panic("PTE %p (%llx) for addr=%llx should be a table",
		      pte, *pte, start);

	/* Roll on to the next page table level */
	return 0;
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
	u64 attrs = PMD_ATTRINDX(option >> 2);
	u64 real_start = start;
	u64 real_size = size;

	debug("start=%lx size=%lx\n", (ulong)start, (ulong)size);

	if (!gd->arch.tlb_emerg)
		panic("Emergency page table not setup.");

	/*
	 * We can not modify page tables that we're currently running on,
	 * so we first need to switch to the "emergency" page tables where
	 * we can safely modify our primary page tables and then switch back
	 */
	__asm_switch_ttbr(gd->arch.tlb_emerg);

	/*
	 * Loop through the address range until we find a page granule that fits
	 * our alignment constraints, then set it to the new cache attributes
	 */
	while (size > 0) {
		int level;
		u64 r;

		for (level = 1; level < 4; level++) {
			/* Set d-cache attributes only */
			r = set_one_region(start, size, attrs, false, level);
			if (r) {
				/* PTE successfully replaced */
				size -= r;
				start += r;
				break;
			}
		}

	}

	/* We're done modifying page tables, switch back to our primary ones */
	__asm_switch_ttbr(gd->arch.tlb_addr);

	/*
	 * Make sure there's nothing stale in dcache for a region that might
	 * have caches off now
	 */
	flush_dcache_range(real_start, real_start + real_size);
}

/*
 * Modify MMU table for a region with updated PXN/UXN/Memory type/valid bits.
 * The procecess is break-before-make. The target region will be marked as
 * invalid during the process of changing.
 */
void mmu_change_region_attr(phys_addr_t addr, size_t siz, u64 attrs)
{
	int level;
	u64 r, size, start;

	start = addr;
	size = siz;
	/*
	 * Loop through the address range until we find a page granule that fits
	 * our alignment constraints, then set it to "invalid".
	 */
	while (size > 0) {
		for (level = 1; level < 4; level++) {
			/* Set PTE to fault */
			r = set_one_region(start, size, PTE_TYPE_FAULT, true,
					   level);
			if (r) {
				/* PTE successfully invalidated */
				size -= r;
				start += r;
				break;
			}
		}
	}

	flush_dcache_range(gd->arch.tlb_addr,
			   gd->arch.tlb_addr + gd->arch.tlb_size);
	__asm_invalidate_tlb_all();

	/*
	 * Loop through the address range until we find a page granule that fits
	 * our alignment constraints, then set it to the new cache attributes
	 */
	start = addr;
	size = siz;
	while (size > 0) {
		for (level = 1; level < 4; level++) {
			/* Set PTE to new attributes */
			r = set_one_region(start, size, attrs, true, level);
			if (r) {
				/* PTE successfully updated */
				size -= r;
				start += r;
				break;
			}
		}
	}
	flush_dcache_range(gd->arch.tlb_addr,
			   gd->arch.tlb_addr + gd->arch.tlb_size);
	__asm_invalidate_tlb_all();
}

#else	/* !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */

/*
 * For SPL builds, we may want to not have dcache enabled. Any real U-Boot
 * running however really wants to have dcache and the MMU active. Check that
 * everything is sane and give the developer a hint if it isn't.
 */
#ifndef CONFIG_SPL_BUILD
#error Please describe your MMU layout in CONFIG_SYS_MEM_MAP and enable dcache.
#endif

void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

void dcache_sanitize_disable(uintptr_t sanitize_base_addr, unsigned long sanitize_size)
{
}

int dcache_status(void)
{
	return 0;
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
}

#endif	/* !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */

#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)

void icache_enable(void)
{
	invalidate_icache_all();
	set_sctlr(get_sctlr() | CR_I);
}

void icache_disable(void)
{
	set_sctlr(get_sctlr() & ~CR_I);
}

int icache_status(void)
{
	return (get_sctlr() & CR_I) != 0;
}

int mmu_status(void)
{
	return (get_sctlr() & CR_M) != 0;
}

void invalidate_icache_all(void)
{
	__asm_invalidate_icache_all();
	__asm_invalidate_l3_icache();
}

#else	/* !CONFIG_IS_ENABLED(SYS_ICACHE_OFF) */

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

int mmu_status(void)
{
	return 0;
}

void invalidate_icache_all(void)
{
}

#endif	/* !CONFIG_IS_ENABLED(SYS_ICACHE_OFF) */

/*
 * Enable dCache & iCache, whether cache is actually enabled
 * depend on CONFIG_SYS_DCACHE_OFF and CONFIG_SYS_ICACHE_OFF
 */
void __weak enable_caches(void)
{
	icache_enable();
	dcache_enable();
}
