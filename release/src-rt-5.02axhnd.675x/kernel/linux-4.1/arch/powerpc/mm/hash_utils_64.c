/*
 * PowerPC64 port by Mike Corrigan and Dave Engebretsen
 *   {mikejc|engebret}@us.ibm.com
 *
 *    Copyright (c) 2000 Mike Corrigan <mikejc@us.ibm.com>
 *
 * SMP scalability work:
 *    Copyright (C) 2001 Anton Blanchard <anton@au.ibm.com>, IBM
 * 
 *    Module name: htab.c
 *
 *    Description:
 *      PowerPC Hashed Page Table functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#undef DEBUG
#undef DEBUG_LOW

#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/sysctl.h>
#include <linux/export.h>
#include <linux/ctype.h>
#include <linux/cache.h>
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/memblock.h>
#include <linux/context_tracking.h>

#include <asm/processor.h>
#include <asm/pgtable.h>
#include <asm/mmu.h>
#include <asm/mmu_context.h>
#include <asm/page.h>
#include <asm/types.h>
#include <asm/uaccess.h>
#include <asm/machdep.h>
#include <asm/prom.h>
#include <asm/tlbflush.h>
#include <asm/io.h>
#include <asm/eeh.h>
#include <asm/tlb.h>
#include <asm/cacheflush.h>
#include <asm/cputable.h>
#include <asm/sections.h>
#include <asm/copro.h>
#include <asm/udbg.h>
#include <asm/code-patching.h>
#include <asm/fadump.h>
#include <asm/firmware.h>
#include <asm/tm.h>

#ifdef DEBUG
#define DBG(fmt...) udbg_printf(fmt)
#else
#define DBG(fmt...)
#endif

#ifdef DEBUG_LOW
#define DBG_LOW(fmt...) udbg_printf(fmt)
#else
#define DBG_LOW(fmt...)
#endif

#define KB (1024)
#define MB (1024*KB)
#define GB (1024L*MB)

/*
 * Note:  pte   --> Linux PTE
 *        HPTE  --> PowerPC Hashed Page Table Entry
 *
 * Execution context:
 *   htab_initialize is called with the MMU off (of course), but
 *   the kernel has been copied down to zero so it can directly
 *   reference global data.  At this point it is very difficult
 *   to print debug info.
 *
 */

#ifdef CONFIG_U3_DART
extern unsigned long dart_tablebase;
#endif /* CONFIG_U3_DART */

static unsigned long _SDR1;
struct mmu_psize_def mmu_psize_defs[MMU_PAGE_COUNT];
EXPORT_SYMBOL_GPL(mmu_psize_defs);

struct hash_pte *htab_address;
unsigned long htab_size_bytes;
unsigned long htab_hash_mask;
EXPORT_SYMBOL_GPL(htab_hash_mask);
int mmu_linear_psize = MMU_PAGE_4K;
EXPORT_SYMBOL_GPL(mmu_linear_psize);
int mmu_virtual_psize = MMU_PAGE_4K;
int mmu_vmalloc_psize = MMU_PAGE_4K;
#ifdef CONFIG_SPARSEMEM_VMEMMAP
int mmu_vmemmap_psize = MMU_PAGE_4K;
#endif
int mmu_io_psize = MMU_PAGE_4K;
int mmu_kernel_ssize = MMU_SEGSIZE_256M;
EXPORT_SYMBOL_GPL(mmu_kernel_ssize);
int mmu_highuser_ssize = MMU_SEGSIZE_256M;
u16 mmu_slb_size = 64;
EXPORT_SYMBOL_GPL(mmu_slb_size);
#ifdef CONFIG_PPC_64K_PAGES
int mmu_ci_restrictions;
#endif
#ifdef CONFIG_DEBUG_PAGEALLOC
static u8 *linear_map_hash_slots;
static unsigned long linear_map_hash_count;
static DEFINE_SPINLOCK(linear_map_hash_lock);
#endif /* CONFIG_DEBUG_PAGEALLOC */

/* There are definitions of page sizes arrays to be used when none
 * is provided by the firmware.
 */

/* Pre-POWER4 CPUs (4k pages only)
 */
static struct mmu_psize_def mmu_psize_defaults_old[] = {
	[MMU_PAGE_4K] = {
		.shift	= 12,
		.sllp	= 0,
		.penc   = {[MMU_PAGE_4K] = 0, [1 ... MMU_PAGE_COUNT - 1] = -1},
		.avpnm	= 0,
		.tlbiel = 0,
	},
};

/* POWER4, GPUL, POWER5
 *
 * Support for 16Mb large pages
 */
static struct mmu_psize_def mmu_psize_defaults_gp[] = {
	[MMU_PAGE_4K] = {
		.shift	= 12,
		.sllp	= 0,
		.penc   = {[MMU_PAGE_4K] = 0, [1 ... MMU_PAGE_COUNT - 1] = -1},
		.avpnm	= 0,
		.tlbiel = 1,
	},
	[MMU_PAGE_16M] = {
		.shift	= 24,
		.sllp	= SLB_VSID_L,
		.penc   = {[0 ... MMU_PAGE_16M - 1] = -1, [MMU_PAGE_16M] = 0,
			    [MMU_PAGE_16M + 1 ... MMU_PAGE_COUNT - 1] = -1 },
		.avpnm	= 0x1UL,
		.tlbiel = 0,
	},
};

static unsigned long htab_convert_pte_flags(unsigned long pteflags)
{
	unsigned long rflags = pteflags & 0x1fa;

	/* _PAGE_EXEC -> NOEXEC */
	if ((pteflags & _PAGE_EXEC) == 0)
		rflags |= HPTE_R_N;

	/* PP bits. PAGE_USER is already PP bit 0x2, so we only
	 * need to add in 0x1 if it's a read-only user page
	 */
	if ((pteflags & _PAGE_USER) && !((pteflags & _PAGE_RW) &&
					 (pteflags & _PAGE_DIRTY)))
		rflags |= 1;
	/*
	 * Always add "C" bit for perf. Memory coherence is always enabled
	 */
	return rflags | HPTE_R_C | HPTE_R_M;
}

int htab_bolt_mapping(unsigned long vstart, unsigned long vend,
		      unsigned long pstart, unsigned long prot,
		      int psize, int ssize)
{
	unsigned long vaddr, paddr;
	unsigned int step, shift;
	int ret = 0;

	shift = mmu_psize_defs[psize].shift;
	step = 1 << shift;

	prot = htab_convert_pte_flags(prot);

	DBG("htab_bolt_mapping(%lx..%lx -> %lx (%lx,%d,%d)\n",
	    vstart, vend, pstart, prot, psize, ssize);

	for (vaddr = vstart, paddr = pstart; vaddr < vend;
	     vaddr += step, paddr += step) {
		unsigned long hash, hpteg;
		unsigned long vsid = get_kernel_vsid(vaddr, ssize);
		unsigned long vpn  = hpt_vpn(vaddr, vsid, ssize);
		unsigned long tprot = prot;

		/*
		 * If we hit a bad address return error.
		 */
		if (!vsid)
			return -1;
		/* Make kernel text executable */
		if (overlaps_kernel_text(vaddr, vaddr + step))
			tprot &= ~HPTE_R_N;

		/* Make kvm guest trampolines executable */
		if (overlaps_kvm_tmp(vaddr, vaddr + step))
			tprot &= ~HPTE_R_N;

		/*
		 * If relocatable, check if it overlaps interrupt vectors that
		 * are copied down to real 0. For relocatable kernel
		 * (e.g. kdump case) we copy interrupt vectors down to real
		 * address 0. Mark that region as executable. This is
		 * because on p8 system with relocation on exception feature
		 * enabled, exceptions are raised with MMU (IR=DR=1) ON. Hence
		 * in order to execute the interrupt handlers in virtual
		 * mode the vector region need to be marked as executable.
		 */
		if ((PHYSICAL_START > MEMORY_START) &&
			overlaps_interrupt_vector_text(vaddr, vaddr + step))
				tprot &= ~HPTE_R_N;

		hash = hpt_hash(vpn, shift, ssize);
		hpteg = ((hash & htab_hash_mask) * HPTES_PER_GROUP);

		BUG_ON(!ppc_md.hpte_insert);
		ret = ppc_md.hpte_insert(hpteg, vpn, paddr, tprot,
					 HPTE_V_BOLTED, psize, psize, ssize);

		if (ret < 0)
			break;
#ifdef CONFIG_DEBUG_PAGEALLOC
		if ((paddr >> PAGE_SHIFT) < linear_map_hash_count)
			linear_map_hash_slots[paddr >> PAGE_SHIFT] = ret | 0x80;
#endif /* CONFIG_DEBUG_PAGEALLOC */
	}
	return ret < 0 ? ret : 0;
}

#ifdef CONFIG_MEMORY_HOTPLUG
int htab_remove_mapping(unsigned long vstart, unsigned long vend,
		      int psize, int ssize)
{
	unsigned long vaddr;
	unsigned int step, shift;

	shift = mmu_psize_defs[psize].shift;
	step = 1 << shift;

	if (!ppc_md.hpte_removebolted) {
		printk(KERN_WARNING "Platform doesn't implement "
				"hpte_removebolted\n");
		return -EINVAL;
	}

	for (vaddr = vstart; vaddr < vend; vaddr += step)
		ppc_md.hpte_removebolted(vaddr, psize, ssize);

	return 0;
}
#endif /* CONFIG_MEMORY_HOTPLUG */

static int __init htab_dt_scan_seg_sizes(unsigned long node,
					 const char *uname, int depth,
					 void *data)
{
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	const __be32 *prop;
	int size = 0;

	/* We are scanning "cpu" nodes only */
	if (type == NULL || strcmp(type, "cpu") != 0)
		return 0;

	prop = of_get_flat_dt_prop(node, "ibm,processor-segment-sizes", &size);
	if (prop == NULL)
		return 0;
	for (; size >= 4; size -= 4, ++prop) {
		if (be32_to_cpu(prop[0]) == 40) {
			DBG("1T segment support detected\n");
			cur_cpu_spec->mmu_features |= MMU_FTR_1T_SEGMENT;
			return 1;
		}
	}
	cur_cpu_spec->mmu_features &= ~MMU_FTR_NO_SLBIE_B;
	return 0;
}

static void __init htab_init_seg_sizes(void)
{
	of_scan_flat_dt(htab_dt_scan_seg_sizes, NULL);
}

static int __init get_idx_from_shift(unsigned int shift)
{
	int idx = -1;

	switch (shift) {
	case 0xc:
		idx = MMU_PAGE_4K;
		break;
	case 0x10:
		idx = MMU_PAGE_64K;
		break;
	case 0x14:
		idx = MMU_PAGE_1M;
		break;
	case 0x18:
		idx = MMU_PAGE_16M;
		break;
	case 0x22:
		idx = MMU_PAGE_16G;
		break;
	}
	return idx;
}

static int __init htab_dt_scan_page_sizes(unsigned long node,
					  const char *uname, int depth,
					  void *data)
{
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	const __be32 *prop;
	int size = 0;

	/* We are scanning "cpu" nodes only */
	if (type == NULL || strcmp(type, "cpu") != 0)
		return 0;

	prop = of_get_flat_dt_prop(node, "ibm,segment-page-sizes", &size);
	if (!prop)
		return 0;

	pr_info("Page sizes from device-tree:\n");
	size /= 4;
	cur_cpu_spec->mmu_features &= ~(MMU_FTR_16M_PAGE);
	while(size > 0) {
		unsigned int base_shift = be32_to_cpu(prop[0]);
		unsigned int slbenc = be32_to_cpu(prop[1]);
		unsigned int lpnum = be32_to_cpu(prop[2]);
		struct mmu_psize_def *def;
		int idx, base_idx;

		size -= 3; prop += 3;
		base_idx = get_idx_from_shift(base_shift);
		if (base_idx < 0) {
			/* skip the pte encoding also */
			prop += lpnum * 2; size -= lpnum * 2;
			continue;
		}
		def = &mmu_psize_defs[base_idx];
		if (base_idx == MMU_PAGE_16M)
			cur_cpu_spec->mmu_features |= MMU_FTR_16M_PAGE;

		def->shift = base_shift;
		if (base_shift <= 23)
			def->avpnm = 0;
		else
			def->avpnm = (1 << (base_shift - 23)) - 1;
		def->sllp = slbenc;
		/*
		 * We don't know for sure what's up with tlbiel, so
		 * for now we only set it for 4K and 64K pages
		 */
		if (base_idx == MMU_PAGE_4K || base_idx == MMU_PAGE_64K)
			def->tlbiel = 1;
		else
			def->tlbiel = 0;

		while (size > 0 && lpnum) {
			unsigned int shift = be32_to_cpu(prop[0]);
			int penc  = be32_to_cpu(prop[1]);

			prop += 2; size -= 2;
			lpnum--;

			idx = get_idx_from_shift(shift);
			if (idx < 0)
				continue;

			if (penc == -1)
				pr_err("Invalid penc for base_shift=%d "
				       "shift=%d\n", base_shift, shift);

			def->penc[idx] = penc;
			pr_info("base_shift=%d: shift=%d, sllp=0x%04lx,"
				" avpnm=0x%08lx, tlbiel=%d, penc=%d\n",
				base_shift, shift, def->sllp,
				def->avpnm, def->tlbiel, def->penc[idx]);
		}
	}

	return 1;
}

#ifdef CONFIG_HUGETLB_PAGE
/* Scan for 16G memory blocks that have been set aside for huge pages
 * and reserve those blocks for 16G huge pages.
 */
static int __init htab_dt_scan_hugepage_blocks(unsigned long node,
					const char *uname, int depth,
					void *data) {
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	const __be64 *addr_prop;
	const __be32 *page_count_prop;
	unsigned int expected_pages;
	long unsigned int phys_addr;
	long unsigned int block_size;

	/* We are scanning "memory" nodes only */
	if (type == NULL || strcmp(type, "memory") != 0)
		return 0;

	/* This property is the log base 2 of the number of virtual pages that
	 * will represent this memory block. */
	page_count_prop = of_get_flat_dt_prop(node, "ibm,expected#pages", NULL);
	if (page_count_prop == NULL)
		return 0;
	expected_pages = (1 << be32_to_cpu(page_count_prop[0]));
	addr_prop = of_get_flat_dt_prop(node, "reg", NULL);
	if (addr_prop == NULL)
		return 0;
	phys_addr = be64_to_cpu(addr_prop[0]);
	block_size = be64_to_cpu(addr_prop[1]);
	if (block_size != (16 * GB))
		return 0;
	printk(KERN_INFO "Huge page(16GB) memory: "
			"addr = 0x%lX size = 0x%lX pages = %d\n",
			phys_addr, block_size, expected_pages);
	if (phys_addr + (16 * GB) <= memblock_end_of_DRAM()) {
		memblock_reserve(phys_addr, block_size * expected_pages);
		add_gpage(phys_addr, block_size, expected_pages);
	}
	return 0;
}
#endif /* CONFIG_HUGETLB_PAGE */

static void mmu_psize_set_default_penc(void)
{
	int bpsize, apsize;
	for (bpsize = 0; bpsize < MMU_PAGE_COUNT; bpsize++)
		for (apsize = 0; apsize < MMU_PAGE_COUNT; apsize++)
			mmu_psize_defs[bpsize].penc[apsize] = -1;
}

#ifdef CONFIG_PPC_64K_PAGES

static bool might_have_hea(void)
{
	/*
	 * The HEA ethernet adapter requires awareness of the
	 * GX bus. Without that awareness we can easily assume
	 * we will never see an HEA ethernet device.
	 */
#ifdef CONFIG_IBMEBUS
	return !cpu_has_feature(CPU_FTR_ARCH_207S);
#else
	return false;
#endif
}

#endif /* #ifdef CONFIG_PPC_64K_PAGES */

static void __init htab_init_page_sizes(void)
{
	int rc;

	/* se the invalid penc to -1 */
	mmu_psize_set_default_penc();

	/* Default to 4K pages only */
	memcpy(mmu_psize_defs, mmu_psize_defaults_old,
	       sizeof(mmu_psize_defaults_old));

	/*
	 * Try to find the available page sizes in the device-tree
	 */
	rc = of_scan_flat_dt(htab_dt_scan_page_sizes, NULL);
	if (rc != 0)  /* Found */
		goto found;

	/*
	 * Not in the device-tree, let's fallback on known size
	 * list for 16M capable GP & GR
	 */
	if (mmu_has_feature(MMU_FTR_16M_PAGE))
		memcpy(mmu_psize_defs, mmu_psize_defaults_gp,
		       sizeof(mmu_psize_defaults_gp));
 found:
#ifndef CONFIG_DEBUG_PAGEALLOC
	/*
	 * Pick a size for the linear mapping. Currently, we only support
	 * 16M, 1M and 4K which is the default
	 */
	if (mmu_psize_defs[MMU_PAGE_16M].shift)
		mmu_linear_psize = MMU_PAGE_16M;
	else if (mmu_psize_defs[MMU_PAGE_1M].shift)
		mmu_linear_psize = MMU_PAGE_1M;
#endif /* CONFIG_DEBUG_PAGEALLOC */

#ifdef CONFIG_PPC_64K_PAGES
	/*
	 * Pick a size for the ordinary pages. Default is 4K, we support
	 * 64K for user mappings and vmalloc if supported by the processor.
	 * We only use 64k for ioremap if the processor
	 * (and firmware) support cache-inhibited large pages.
	 * If not, we use 4k and set mmu_ci_restrictions so that
	 * hash_page knows to switch processes that use cache-inhibited
	 * mappings to 4k pages.
	 */
	if (mmu_psize_defs[MMU_PAGE_64K].shift) {
		mmu_virtual_psize = MMU_PAGE_64K;
		mmu_vmalloc_psize = MMU_PAGE_64K;
		if (mmu_linear_psize == MMU_PAGE_4K)
			mmu_linear_psize = MMU_PAGE_64K;
		if (mmu_has_feature(MMU_FTR_CI_LARGE_PAGE)) {
			/*
			 * When running on pSeries using 64k pages for ioremap
			 * would stop us accessing the HEA ethernet. So if we
			 * have the chance of ever seeing one, stay at 4k.
			 */
			if (!might_have_hea() || !machine_is(pseries))
				mmu_io_psize = MMU_PAGE_64K;
		} else
			mmu_ci_restrictions = 1;
	}
#endif /* CONFIG_PPC_64K_PAGES */

#ifdef CONFIG_SPARSEMEM_VMEMMAP
	/* We try to use 16M pages for vmemmap if that is supported
	 * and we have at least 1G of RAM at boot
	 */
	if (mmu_psize_defs[MMU_PAGE_16M].shift &&
	    memblock_phys_mem_size() >= 0x40000000)
		mmu_vmemmap_psize = MMU_PAGE_16M;
	else if (mmu_psize_defs[MMU_PAGE_64K].shift)
		mmu_vmemmap_psize = MMU_PAGE_64K;
	else
		mmu_vmemmap_psize = MMU_PAGE_4K;
#endif /* CONFIG_SPARSEMEM_VMEMMAP */

	printk(KERN_DEBUG "Page orders: linear mapping = %d, "
	       "virtual = %d, io = %d"
#ifdef CONFIG_SPARSEMEM_VMEMMAP
	       ", vmemmap = %d"
#endif
	       "\n",
	       mmu_psize_defs[mmu_linear_psize].shift,
	       mmu_psize_defs[mmu_virtual_psize].shift,
	       mmu_psize_defs[mmu_io_psize].shift
#ifdef CONFIG_SPARSEMEM_VMEMMAP
	       ,mmu_psize_defs[mmu_vmemmap_psize].shift
#endif
	       );

#ifdef CONFIG_HUGETLB_PAGE
	/* Reserve 16G huge page memory sections for huge pages */
	of_scan_flat_dt(htab_dt_scan_hugepage_blocks, NULL);
#endif /* CONFIG_HUGETLB_PAGE */
}

static int __init htab_dt_scan_pftsize(unsigned long node,
				       const char *uname, int depth,
				       void *data)
{
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	const __be32 *prop;

	/* We are scanning "cpu" nodes only */
	if (type == NULL || strcmp(type, "cpu") != 0)
		return 0;

	prop = of_get_flat_dt_prop(node, "ibm,pft-size", NULL);
	if (prop != NULL) {
		/* pft_size[0] is the NUMA CEC cookie */
		ppc64_pft_size = be32_to_cpu(prop[1]);
		return 1;
	}
	return 0;
}

static unsigned long __init htab_get_table_size(void)
{
	unsigned long mem_size, rnd_mem_size, pteg_count, psize;

	/* If hash size isn't already provided by the platform, we try to
	 * retrieve it from the device-tree. If it's not there neither, we
	 * calculate it now based on the total RAM size
	 */
	if (ppc64_pft_size == 0)
		of_scan_flat_dt(htab_dt_scan_pftsize, NULL);
	if (ppc64_pft_size)
		return 1UL << ppc64_pft_size;

	/* round mem_size up to next power of 2 */
	mem_size = memblock_phys_mem_size();
	rnd_mem_size = 1UL << __ilog2(mem_size);
	if (rnd_mem_size < mem_size)
		rnd_mem_size <<= 1;

	/* # pages / 2 */
	psize = mmu_psize_defs[mmu_virtual_psize].shift;
	pteg_count = max(rnd_mem_size >> (psize + 1), 1UL << 11);

	return pteg_count << 7;
}

#ifdef CONFIG_MEMORY_HOTPLUG
int create_section_mapping(unsigned long start, unsigned long end)
{
	return htab_bolt_mapping(start, end, __pa(start),
				 pgprot_val(PAGE_KERNEL), mmu_linear_psize,
				 mmu_kernel_ssize);
}

int remove_section_mapping(unsigned long start, unsigned long end)
{
	return htab_remove_mapping(start, end, mmu_linear_psize,
			mmu_kernel_ssize);
}
#endif /* CONFIG_MEMORY_HOTPLUG */

extern u32 htab_call_hpte_insert1[];
extern u32 htab_call_hpte_insert2[];
extern u32 htab_call_hpte_remove[];
extern u32 htab_call_hpte_updatepp[];
extern u32 ht64_call_hpte_insert1[];
extern u32 ht64_call_hpte_insert2[];
extern u32 ht64_call_hpte_remove[];
extern u32 ht64_call_hpte_updatepp[];

static void __init htab_finish_init(void)
{
#ifdef CONFIG_PPC_HAS_HASH_64K
	patch_branch(ht64_call_hpte_insert1,
		ppc_function_entry(ppc_md.hpte_insert),
		BRANCH_SET_LINK);
	patch_branch(ht64_call_hpte_insert2,
		ppc_function_entry(ppc_md.hpte_insert),
		BRANCH_SET_LINK);
	patch_branch(ht64_call_hpte_remove,
		ppc_function_entry(ppc_md.hpte_remove),
		BRANCH_SET_LINK);
	patch_branch(ht64_call_hpte_updatepp,
		ppc_function_entry(ppc_md.hpte_updatepp),
		BRANCH_SET_LINK);
#endif /* CONFIG_PPC_HAS_HASH_64K */

	patch_branch(htab_call_hpte_insert1,
		ppc_function_entry(ppc_md.hpte_insert),
		BRANCH_SET_LINK);
	patch_branch(htab_call_hpte_insert2,
		ppc_function_entry(ppc_md.hpte_insert),
		BRANCH_SET_LINK);
	patch_branch(htab_call_hpte_remove,
		ppc_function_entry(ppc_md.hpte_remove),
		BRANCH_SET_LINK);
	patch_branch(htab_call_hpte_updatepp,
		ppc_function_entry(ppc_md.hpte_updatepp),
		BRANCH_SET_LINK);
}

static void __init htab_initialize(void)
{
	unsigned long table;
	unsigned long pteg_count;
	unsigned long prot;
	unsigned long base = 0, size = 0, limit;
	struct memblock_region *reg;

	DBG(" -> htab_initialize()\n");

	/* Initialize segment sizes */
	htab_init_seg_sizes();

	/* Initialize page sizes */
	htab_init_page_sizes();

	if (mmu_has_feature(MMU_FTR_1T_SEGMENT)) {
		mmu_kernel_ssize = MMU_SEGSIZE_1T;
		mmu_highuser_ssize = MMU_SEGSIZE_1T;
		printk(KERN_INFO "Using 1TB segments\n");
	}

	/*
	 * Calculate the required size of the htab.  We want the number of
	 * PTEGs to equal one half the number of real pages.
	 */ 
	htab_size_bytes = htab_get_table_size();
	pteg_count = htab_size_bytes >> 7;

	htab_hash_mask = pteg_count - 1;

	if (firmware_has_feature(FW_FEATURE_LPAR)) {
		/* Using a hypervisor which owns the htab */
		htab_address = NULL;
		_SDR1 = 0; 
#ifdef CONFIG_FA_DUMP
		/*
		 * If firmware assisted dump is active firmware preserves
		 * the contents of htab along with entire partition memory.
		 * Clear the htab if firmware assisted dump is active so
		 * that we dont end up using old mappings.
		 */
		if (is_fadump_active() && ppc_md.hpte_clear_all)
			ppc_md.hpte_clear_all();
#endif
	} else {
		/* Find storage for the HPT.  Must be contiguous in
		 * the absolute address space. On cell we want it to be
		 * in the first 2 Gig so we can use it for IOMMU hacks.
		 */
		if (machine_is(cell))
			limit = 0x80000000;
		else
			limit = MEMBLOCK_ALLOC_ANYWHERE;

		table = memblock_alloc_base(htab_size_bytes, htab_size_bytes, limit);

		DBG("Hash table allocated at %lx, size: %lx\n", table,
		    htab_size_bytes);

		htab_address = __va(table);

		/* htab absolute addr + encoded htabsize */
		_SDR1 = table + __ilog2(pteg_count) - 11;

		/* Initialize the HPT with no entries */
		memset((void *)table, 0, htab_size_bytes);

		/* Set SDR1 */
		mtspr(SPRN_SDR1, _SDR1);
	}

	prot = pgprot_val(PAGE_KERNEL);

#ifdef CONFIG_DEBUG_PAGEALLOC
	linear_map_hash_count = memblock_end_of_DRAM() >> PAGE_SHIFT;
	linear_map_hash_slots = __va(memblock_alloc_base(linear_map_hash_count,
						    1, ppc64_rma_size));
	memset(linear_map_hash_slots, 0, linear_map_hash_count);
#endif /* CONFIG_DEBUG_PAGEALLOC */

	/* On U3 based machines, we need to reserve the DART area and
	 * _NOT_ map it to avoid cache paradoxes as it's remapped non
	 * cacheable later on
	 */

	/* create bolted the linear mapping in the hash table */
	for_each_memblock(memory, reg) {
		base = (unsigned long)__va(reg->base);
		size = reg->size;

		DBG("creating mapping for region: %lx..%lx (prot: %lx)\n",
		    base, size, prot);

#ifdef CONFIG_U3_DART
		/* Do not map the DART space. Fortunately, it will be aligned
		 * in such a way that it will not cross two memblock regions and
		 * will fit within a single 16Mb page.
		 * The DART space is assumed to be a full 16Mb region even if
		 * we only use 2Mb of that space. We will use more of it later
		 * for AGP GART. We have to use a full 16Mb large page.
		 */
		DBG("DART base: %lx\n", dart_tablebase);

		if (dart_tablebase != 0 && dart_tablebase >= base
		    && dart_tablebase < (base + size)) {
			unsigned long dart_table_end = dart_tablebase + 16 * MB;
			if (base != dart_tablebase)
				BUG_ON(htab_bolt_mapping(base, dart_tablebase,
							__pa(base), prot,
							mmu_linear_psize,
							mmu_kernel_ssize));
			if ((base + size) > dart_table_end)
				BUG_ON(htab_bolt_mapping(dart_tablebase+16*MB,
							base + size,
							__pa(dart_table_end),
							 prot,
							 mmu_linear_psize,
							 mmu_kernel_ssize));
			continue;
		}
#endif /* CONFIG_U3_DART */
		BUG_ON(htab_bolt_mapping(base, base + size, __pa(base),
				prot, mmu_linear_psize, mmu_kernel_ssize));
	}
	memblock_set_current_limit(MEMBLOCK_ALLOC_ANYWHERE);

	/*
	 * If we have a memory_limit and we've allocated TCEs then we need to
	 * explicitly map the TCE area at the top of RAM. We also cope with the
	 * case that the TCEs start below memory_limit.
	 * tce_alloc_start/end are 16MB aligned so the mapping should work
	 * for either 4K or 16MB pages.
	 */
	if (tce_alloc_start) {
		tce_alloc_start = (unsigned long)__va(tce_alloc_start);
		tce_alloc_end = (unsigned long)__va(tce_alloc_end);

		if (base + size >= tce_alloc_start)
			tce_alloc_start = base + size + 1;

		BUG_ON(htab_bolt_mapping(tce_alloc_start, tce_alloc_end,
					 __pa(tce_alloc_start), prot,
					 mmu_linear_psize, mmu_kernel_ssize));
	}

	htab_finish_init();

	DBG(" <- htab_initialize()\n");
}
#undef KB
#undef MB

void __init early_init_mmu(void)
{
	/* Initialize the MMU Hash table and create the linear mapping
	 * of memory. Has to be done before SLB initialization as this is
	 * currently where the page size encoding is obtained.
	 */
	htab_initialize();

	/* Initialize SLB management */
	slb_initialize();
}

#ifdef CONFIG_SMP
void early_init_mmu_secondary(void)
{
	/* Initialize hash table for that CPU */
	if (!firmware_has_feature(FW_FEATURE_LPAR))
		mtspr(SPRN_SDR1, _SDR1);

	/* Initialize SLB */
	slb_initialize();
}
#endif /* CONFIG_SMP */

/*
 * Called by asm hashtable.S for doing lazy icache flush
 */
unsigned int hash_page_do_lazy_icache(unsigned int pp, pte_t pte, int trap)
{
	struct page *page;

	if (!pfn_valid(pte_pfn(pte)))
		return pp;

	page = pte_page(pte);

	/* page is dirty */
	if (!test_bit(PG_arch_1, &page->flags) && !PageReserved(page)) {
		if (trap == 0x400) {
			flush_dcache_icache_page(page);
			set_bit(PG_arch_1, &page->flags);
		} else
			pp |= HPTE_R_N;
	}
	return pp;
}

#ifdef CONFIG_PPC_MM_SLICES
static unsigned int get_paca_psize(unsigned long addr)
{
	u64 lpsizes;
	unsigned char *hpsizes;
	unsigned long index, mask_index;

	if (addr < SLICE_LOW_TOP) {
		lpsizes = get_paca()->context.low_slices_psize;
		index = GET_LOW_SLICE_INDEX(addr);
		return (lpsizes >> (index * 4)) & 0xF;
	}
	hpsizes = get_paca()->context.high_slices_psize;
	index = GET_HIGH_SLICE_INDEX(addr);
	mask_index = index & 0x1;
	return (hpsizes[index >> 1] >> (mask_index * 4)) & 0xF;
}

#else
unsigned int get_paca_psize(unsigned long addr)
{
	return get_paca()->context.user_psize;
}
#endif

/*
 * Demote a segment to using 4k pages.
 * For now this makes the whole process use 4k pages.
 */
#ifdef CONFIG_PPC_64K_PAGES
void demote_segment_4k(struct mm_struct *mm, unsigned long addr)
{
	if (get_slice_psize(mm, addr) == MMU_PAGE_4K)
		return;
	slice_set_range_psize(mm, addr, 1, MMU_PAGE_4K);
	copro_flush_all_slbs(mm);
	if ((get_paca_psize(addr) != MMU_PAGE_4K) && (current->mm == mm)) {
		get_paca()->context = mm->context;
		slb_flush_and_rebolt();
	}
}
#endif /* CONFIG_PPC_64K_PAGES */

#ifdef CONFIG_PPC_SUBPAGE_PROT
/*
 * This looks up a 2-bit protection code for a 4k subpage of a 64k page.
 * Userspace sets the subpage permissions using the subpage_prot system call.
 *
 * Result is 0: full permissions, _PAGE_RW: read-only,
 * _PAGE_USER or _PAGE_USER|_PAGE_RW: no access.
 */
static int subpage_protection(struct mm_struct *mm, unsigned long ea)
{
	struct subpage_prot_table *spt = &mm->context.spt;
	u32 spp = 0;
	u32 **sbpm, *sbpp;

	if (ea >= spt->maxaddr)
		return 0;
	if (ea < 0x100000000UL) {
		/* addresses below 4GB use spt->low_prot */
		sbpm = spt->low_prot;
	} else {
		sbpm = spt->protptrs[ea >> SBP_L3_SHIFT];
		if (!sbpm)
			return 0;
	}
	sbpp = sbpm[(ea >> SBP_L2_SHIFT) & (SBP_L2_COUNT - 1)];
	if (!sbpp)
		return 0;
	spp = sbpp[(ea >> PAGE_SHIFT) & (SBP_L1_COUNT - 1)];

	/* extract 2-bit bitfield for this 4k subpage */
	spp >>= 30 - 2 * ((ea >> 12) & 0xf);

	/* turn 0,1,2,3 into combination of _PAGE_USER and _PAGE_RW */
	spp = ((spp & 2) ? _PAGE_USER : 0) | ((spp & 1) ? _PAGE_RW : 0);
	return spp;
}

#else /* CONFIG_PPC_SUBPAGE_PROT */
static inline int subpage_protection(struct mm_struct *mm, unsigned long ea)
{
	return 0;
}
#endif

void hash_failure_debug(unsigned long ea, unsigned long access,
			unsigned long vsid, unsigned long trap,
			int ssize, int psize, int lpsize, unsigned long pte)
{
	if (!printk_ratelimit())
		return;
	pr_info("mm: Hashing failure ! EA=0x%lx access=0x%lx current=%s\n",
		ea, access, current->comm);
	pr_info("    trap=0x%lx vsid=0x%lx ssize=%d base psize=%d psize %d pte=0x%lx\n",
		trap, vsid, ssize, psize, lpsize, pte);
}

static void check_paca_psize(unsigned long ea, struct mm_struct *mm,
			     int psize, bool user_region)
{
	if (user_region) {
		if (psize != get_paca_psize(ea)) {
			get_paca()->context = mm->context;
			slb_flush_and_rebolt();
		}
	} else if (get_paca()->vmalloc_sllp !=
		   mmu_psize_defs[mmu_vmalloc_psize].sllp) {
		get_paca()->vmalloc_sllp =
			mmu_psize_defs[mmu_vmalloc_psize].sllp;
		slb_vmalloc_update();
	}
}

/* Result code is:
 *  0 - handled
 *  1 - normal page fault
 * -1 - critical hash insertion error
 * -2 - access not permitted by subpage protection mechanism
 */
int hash_page_mm(struct mm_struct *mm, unsigned long ea,
		 unsigned long access, unsigned long trap,
		 unsigned long flags)
{
	enum ctx_state prev_state = exception_enter();
	pgd_t *pgdir;
	unsigned long vsid;
	pte_t *ptep;
	unsigned hugeshift;
	const struct cpumask *tmp;
	int rc, user_region = 0;
	int psize, ssize;

	DBG_LOW("hash_page(ea=%016lx, access=%lx, trap=%lx\n",
		ea, access, trap);

	/* Get region & vsid */
 	switch (REGION_ID(ea)) {
	case USER_REGION_ID:
		user_region = 1;
		if (! mm) {
			DBG_LOW(" user region with no mm !\n");
			rc = 1;
			goto bail;
		}
		psize = get_slice_psize(mm, ea);
		ssize = user_segment_size(ea);
		vsid = get_vsid(mm->context.id, ea, ssize);
		break;
	case VMALLOC_REGION_ID:
		vsid = get_kernel_vsid(ea, mmu_kernel_ssize);
		if (ea < VMALLOC_END)
			psize = mmu_vmalloc_psize;
		else
			psize = mmu_io_psize;
		ssize = mmu_kernel_ssize;
		break;
	default:
		/* Not a valid range
		 * Send the problem up to do_page_fault 
		 */
		rc = 1;
		goto bail;
	}
	DBG_LOW(" mm=%p, mm->pgdir=%p, vsid=%016lx\n", mm, mm->pgd, vsid);

	/* Bad address. */
	if (!vsid) {
		DBG_LOW("Bad address!\n");
		rc = 1;
		goto bail;
	}
	/* Get pgdir */
	pgdir = mm->pgd;
	if (pgdir == NULL) {
		rc = 1;
		goto bail;
	}

	/* Check CPU locality */
	tmp = cpumask_of(smp_processor_id());
	if (user_region && cpumask_equal(mm_cpumask(mm), tmp))
		flags |= HPTE_LOCAL_UPDATE;

#ifndef CONFIG_PPC_64K_PAGES
	/* If we use 4K pages and our psize is not 4K, then we might
	 * be hitting a special driver mapping, and need to align the
	 * address before we fetch the PTE.
	 *
	 * It could also be a hugepage mapping, in which case this is
	 * not necessary, but it's not harmful, either.
	 */
	if (psize != MMU_PAGE_4K)
		ea &= ~((1ul << mmu_psize_defs[psize].shift) - 1);
#endif /* CONFIG_PPC_64K_PAGES */

	/* Get PTE and page size from page tables */
	ptep = __find_linux_pte_or_hugepte(pgdir, ea, &hugeshift);
	if (ptep == NULL || !pte_present(*ptep)) {
		DBG_LOW(" no PTE !\n");
		rc = 1;
		goto bail;
	}

	/* Add _PAGE_PRESENT to the required access perm */
	access |= _PAGE_PRESENT;

	/* Pre-check access permissions (will be re-checked atomically
	 * in __hash_page_XX but this pre-check is a fast path
	 */
	if (access & ~pte_val(*ptep)) {
		DBG_LOW(" no access !\n");
		rc = 1;
		goto bail;
	}

	if (hugeshift) {
		if (pmd_trans_huge(*(pmd_t *)ptep))
			rc = __hash_page_thp(ea, access, vsid, (pmd_t *)ptep,
					     trap, flags, ssize, psize);
#ifdef CONFIG_HUGETLB_PAGE
		else
			rc = __hash_page_huge(ea, access, vsid, ptep, trap,
					      flags, ssize, hugeshift, psize);
#else
		else {
			/*
			 * if we have hugeshift, and is not transhuge with
			 * hugetlb disabled, something is really wrong.
			 */
			rc = 1;
			WARN_ON(1);
		}
#endif
		if (current->mm == mm)
			check_paca_psize(ea, mm, psize, user_region);

		goto bail;
	}

#ifndef CONFIG_PPC_64K_PAGES
	DBG_LOW(" i-pte: %016lx\n", pte_val(*ptep));
#else
	DBG_LOW(" i-pte: %016lx %016lx\n", pte_val(*ptep),
		pte_val(*(ptep + PTRS_PER_PTE)));
#endif
	/* Do actual hashing */
#ifdef CONFIG_PPC_64K_PAGES
	/* If _PAGE_4K_PFN is set, make sure this is a 4k segment */
	if ((pte_val(*ptep) & _PAGE_4K_PFN) && psize == MMU_PAGE_64K) {
		demote_segment_4k(mm, ea);
		psize = MMU_PAGE_4K;
	}

	/* If this PTE is non-cacheable and we have restrictions on
	 * using non cacheable large pages, then we switch to 4k
	 */
	if (mmu_ci_restrictions && psize == MMU_PAGE_64K &&
	    (pte_val(*ptep) & _PAGE_NO_CACHE)) {
		if (user_region) {
			demote_segment_4k(mm, ea);
			psize = MMU_PAGE_4K;
		} else if (ea < VMALLOC_END) {
			/*
			 * some driver did a non-cacheable mapping
			 * in vmalloc space, so switch vmalloc
			 * to 4k pages
			 */
			printk(KERN_ALERT "Reducing vmalloc segment "
			       "to 4kB pages because of "
			       "non-cacheable mapping\n");
			psize = mmu_vmalloc_psize = MMU_PAGE_4K;
			copro_flush_all_slbs(mm);
		}
	}

	if (current->mm == mm)
		check_paca_psize(ea, mm, psize, user_region);
#endif /* CONFIG_PPC_64K_PAGES */

#ifdef CONFIG_PPC_HAS_HASH_64K
	if (psize == MMU_PAGE_64K)
		rc = __hash_page_64K(ea, access, vsid, ptep, trap,
				     flags, ssize);
	else
#endif /* CONFIG_PPC_HAS_HASH_64K */
	{
		int spp = subpage_protection(mm, ea);
		if (access & spp)
			rc = -2;
		else
			rc = __hash_page_4K(ea, access, vsid, ptep, trap,
					    flags, ssize, spp);
	}

	/* Dump some info in case of hash insertion failure, they should
	 * never happen so it is really useful to know if/when they do
	 */
	if (rc == -1)
		hash_failure_debug(ea, access, vsid, trap, ssize, psize,
				   psize, pte_val(*ptep));
#ifndef CONFIG_PPC_64K_PAGES
	DBG_LOW(" o-pte: %016lx\n", pte_val(*ptep));
#else
	DBG_LOW(" o-pte: %016lx %016lx\n", pte_val(*ptep),
		pte_val(*(ptep + PTRS_PER_PTE)));
#endif
	DBG_LOW(" -> rc=%d\n", rc);

bail:
	exception_exit(prev_state);
	return rc;
}
EXPORT_SYMBOL_GPL(hash_page_mm);

int hash_page(unsigned long ea, unsigned long access, unsigned long trap,
	      unsigned long dsisr)
{
	unsigned long flags = 0;
	struct mm_struct *mm = current->mm;

	if (REGION_ID(ea) == VMALLOC_REGION_ID)
		mm = &init_mm;

	if (dsisr & DSISR_NOHPTE)
		flags |= HPTE_NOHPTE_UPDATE;

	return hash_page_mm(mm, ea, access, trap, flags);
}
EXPORT_SYMBOL_GPL(hash_page);

void hash_preload(struct mm_struct *mm, unsigned long ea,
		  unsigned long access, unsigned long trap)
{
	int hugepage_shift;
	unsigned long vsid;
	pgd_t *pgdir;
	pte_t *ptep;
	unsigned long flags;
	int rc, ssize, update_flags = 0;

	BUG_ON(REGION_ID(ea) != USER_REGION_ID);

#ifdef CONFIG_PPC_MM_SLICES
	/* We only prefault standard pages for now */
	if (unlikely(get_slice_psize(mm, ea) != mm->context.user_psize))
		return;
#endif

	DBG_LOW("hash_preload(mm=%p, mm->pgdir=%p, ea=%016lx, access=%lx,"
		" trap=%lx\n", mm, mm->pgd, ea, access, trap);

	/* Get Linux PTE if available */
	pgdir = mm->pgd;
	if (pgdir == NULL)
		return;

	/* Get VSID */
	ssize = user_segment_size(ea);
	vsid = get_vsid(mm->context.id, ea, ssize);
	if (!vsid)
		return;
	/*
	 * Hash doesn't like irqs. Walking linux page table with irq disabled
	 * saves us from holding multiple locks.
	 */
	local_irq_save(flags);

	/*
	 * THP pages use update_mmu_cache_pmd. We don't do
	 * hash preload there. Hence can ignore THP here
	 */
	ptep = find_linux_pte_or_hugepte(pgdir, ea, &hugepage_shift);
	if (!ptep)
		goto out_exit;

	WARN_ON(hugepage_shift);
#ifdef CONFIG_PPC_64K_PAGES
	/* If either _PAGE_4K_PFN or _PAGE_NO_CACHE is set (and we are on
	 * a 64K kernel), then we don't preload, hash_page() will take
	 * care of it once we actually try to access the page.
	 * That way we don't have to duplicate all of the logic for segment
	 * page size demotion here
	 */
	if (pte_val(*ptep) & (_PAGE_4K_PFN | _PAGE_NO_CACHE))
		goto out_exit;
#endif /* CONFIG_PPC_64K_PAGES */

	/* Is that local to this CPU ? */
	if (cpumask_equal(mm_cpumask(mm), cpumask_of(smp_processor_id())))
		update_flags |= HPTE_LOCAL_UPDATE;

	/* Hash it in */
#ifdef CONFIG_PPC_HAS_HASH_64K
	if (mm->context.user_psize == MMU_PAGE_64K)
		rc = __hash_page_64K(ea, access, vsid, ptep, trap,
				     update_flags, ssize);
	else
#endif /* CONFIG_PPC_HAS_HASH_64K */
		rc = __hash_page_4K(ea, access, vsid, ptep, trap, update_flags,
				    ssize, subpage_protection(mm, ea));

	/* Dump some info in case of hash insertion failure, they should
	 * never happen so it is really useful to know if/when they do
	 */
	if (rc == -1)
		hash_failure_debug(ea, access, vsid, trap, ssize,
				   mm->context.user_psize,
				   mm->context.user_psize,
				   pte_val(*ptep));
out_exit:
	local_irq_restore(flags);
}

/* WARNING: This is called from hash_low_64.S, if you change this prototype,
 *          do not forget to update the assembly call site !
 */
void flush_hash_page(unsigned long vpn, real_pte_t pte, int psize, int ssize,
		     unsigned long flags)
{
	unsigned long hash, index, shift, hidx, slot;
	int local = flags & HPTE_LOCAL_UPDATE;

	DBG_LOW("flush_hash_page(vpn=%016lx)\n", vpn);
	pte_iterate_hashed_subpages(pte, psize, vpn, index, shift) {
		hash = hpt_hash(vpn, shift, ssize);
		hidx = __rpte_to_hidx(pte, index);
		if (hidx & _PTEIDX_SECONDARY)
			hash = ~hash;
		slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
		slot += hidx & _PTEIDX_GROUP_IX;
		DBG_LOW(" sub %ld: hash=%lx, hidx=%lx\n", index, slot, hidx);
		/*
		 * We use same base page size and actual psize, because we don't
		 * use these functions for hugepage
		 */
		ppc_md.hpte_invalidate(slot, vpn, psize, psize, ssize, local);
	} pte_iterate_hashed_end();

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/* Transactions are not aborted by tlbiel, only tlbie.
	 * Without, syncing a page back to a block device w/ PIO could pick up
	 * transactional data (bad!) so we force an abort here.  Before the
	 * sync the page will be made read-only, which will flush_hash_page.
	 * BIG ISSUE here: if the kernel uses a page from userspace without
	 * unmapping it first, it may see the speculated version.
	 */
	if (local && cpu_has_feature(CPU_FTR_TM) &&
	    current->thread.regs &&
	    MSR_TM_ACTIVE(current->thread.regs->msr)) {
		tm_enable();
		tm_abort(TM_CAUSE_TLBI);
	}
#endif
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
void flush_hash_hugepage(unsigned long vsid, unsigned long addr,
			 pmd_t *pmdp, unsigned int psize, int ssize,
			 unsigned long flags)
{
	int i, max_hpte_count, valid;
	unsigned long s_addr;
	unsigned char *hpte_slot_array;
	unsigned long hidx, shift, vpn, hash, slot;
	int local = flags & HPTE_LOCAL_UPDATE;

	s_addr = addr & HPAGE_PMD_MASK;
	hpte_slot_array = get_hpte_slot_array(pmdp);
	/*
	 * IF we try to do a HUGE PTE update after a withdraw is done.
	 * we will find the below NULL. This happens when we do
	 * split_huge_page_pmd
	 */
	if (!hpte_slot_array)
		return;

	if (ppc_md.hugepage_invalidate) {
		ppc_md.hugepage_invalidate(vsid, s_addr, hpte_slot_array,
					   psize, ssize, local);
		goto tm_abort;
	}
	/*
	 * No bluk hpte removal support, invalidate each entry
	 */
	shift = mmu_psize_defs[psize].shift;
	max_hpte_count = HPAGE_PMD_SIZE >> shift;
	for (i = 0; i < max_hpte_count; i++) {
		/*
		 * 8 bits per each hpte entries
		 * 000| [ secondary group (one bit) | hidx (3 bits) | valid bit]
		 */
		valid = hpte_valid(hpte_slot_array, i);
		if (!valid)
			continue;
		hidx =  hpte_hash_index(hpte_slot_array, i);

		/* get the vpn */
		addr = s_addr + (i * (1ul << shift));
		vpn = hpt_vpn(addr, vsid, ssize);
		hash = hpt_hash(vpn, shift, ssize);
		if (hidx & _PTEIDX_SECONDARY)
			hash = ~hash;

		slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
		slot += hidx & _PTEIDX_GROUP_IX;
		ppc_md.hpte_invalidate(slot, vpn, psize,
				       MMU_PAGE_16M, ssize, local);
	}
tm_abort:
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/* Transactions are not aborted by tlbiel, only tlbie.
	 * Without, syncing a page back to a block device w/ PIO could pick up
	 * transactional data (bad!) so we force an abort here.  Before the
	 * sync the page will be made read-only, which will flush_hash_page.
	 * BIG ISSUE here: if the kernel uses a page from userspace without
	 * unmapping it first, it may see the speculated version.
	 */
	if (local && cpu_has_feature(CPU_FTR_TM) &&
	    current->thread.regs &&
	    MSR_TM_ACTIVE(current->thread.regs->msr)) {
		tm_enable();
		tm_abort(TM_CAUSE_TLBI);
	}
#endif
	return;
}
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

void flush_hash_range(unsigned long number, int local)
{
	if (ppc_md.flush_hash_range)
		ppc_md.flush_hash_range(number, local);
	else {
		int i;
		struct ppc64_tlb_batch *batch =
			this_cpu_ptr(&ppc64_tlb_batch);

		for (i = 0; i < number; i++)
			flush_hash_page(batch->vpn[i], batch->pte[i],
					batch->psize, batch->ssize, local);
	}
}

/*
 * low_hash_fault is called when we the low level hash code failed
 * to instert a PTE due to an hypervisor error
 */
void low_hash_fault(struct pt_regs *regs, unsigned long address, int rc)
{
	enum ctx_state prev_state = exception_enter();

	if (user_mode(regs)) {
#ifdef CONFIG_PPC_SUBPAGE_PROT
		if (rc == -2)
			_exception(SIGSEGV, regs, SEGV_ACCERR, address);
		else
#endif
			_exception(SIGBUS, regs, BUS_ADRERR, address);
	} else
		bad_page_fault(regs, address, SIGBUS);

	exception_exit(prev_state);
}

long hpte_insert_repeating(unsigned long hash, unsigned long vpn,
			   unsigned long pa, unsigned long rflags,
			   unsigned long vflags, int psize, int ssize)
{
	unsigned long hpte_group;
	long slot;

repeat:
	hpte_group = ((hash & htab_hash_mask) *
		       HPTES_PER_GROUP) & ~0x7UL;

	/* Insert into the hash table, primary slot */
	slot = ppc_md.hpte_insert(hpte_group, vpn, pa, rflags, vflags,
				  psize, psize, ssize);

	/* Primary is full, try the secondary */
	if (unlikely(slot == -1)) {
		hpte_group = ((~hash & htab_hash_mask) *
			      HPTES_PER_GROUP) & ~0x7UL;
		slot = ppc_md.hpte_insert(hpte_group, vpn, pa, rflags,
					  vflags | HPTE_V_SECONDARY,
					  psize, psize, ssize);
		if (slot == -1) {
			if (mftb() & 0x1)
				hpte_group = ((hash & htab_hash_mask) *
					      HPTES_PER_GROUP)&~0x7UL;

			ppc_md.hpte_remove(hpte_group);
			goto repeat;
		}
	}

	return slot;
}

#ifdef CONFIG_DEBUG_PAGEALLOC
static void kernel_map_linear_page(unsigned long vaddr, unsigned long lmi)
{
	unsigned long hash;
	unsigned long vsid = get_kernel_vsid(vaddr, mmu_kernel_ssize);
	unsigned long vpn = hpt_vpn(vaddr, vsid, mmu_kernel_ssize);
	unsigned long mode = htab_convert_pte_flags(PAGE_KERNEL);
	long ret;

	hash = hpt_hash(vpn, PAGE_SHIFT, mmu_kernel_ssize);

	/* Don't create HPTE entries for bad address */
	if (!vsid)
		return;

	ret = hpte_insert_repeating(hash, vpn, __pa(vaddr), mode,
				    HPTE_V_BOLTED,
				    mmu_linear_psize, mmu_kernel_ssize);

	BUG_ON (ret < 0);
	spin_lock(&linear_map_hash_lock);
	BUG_ON(linear_map_hash_slots[lmi] & 0x80);
	linear_map_hash_slots[lmi] = ret | 0x80;
	spin_unlock(&linear_map_hash_lock);
}

static void kernel_unmap_linear_page(unsigned long vaddr, unsigned long lmi)
{
	unsigned long hash, hidx, slot;
	unsigned long vsid = get_kernel_vsid(vaddr, mmu_kernel_ssize);
	unsigned long vpn = hpt_vpn(vaddr, vsid, mmu_kernel_ssize);

	hash = hpt_hash(vpn, PAGE_SHIFT, mmu_kernel_ssize);
	spin_lock(&linear_map_hash_lock);
	BUG_ON(!(linear_map_hash_slots[lmi] & 0x80));
	hidx = linear_map_hash_slots[lmi] & 0x7f;
	linear_map_hash_slots[lmi] = 0;
	spin_unlock(&linear_map_hash_lock);
	if (hidx & _PTEIDX_SECONDARY)
		hash = ~hash;
	slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
	slot += hidx & _PTEIDX_GROUP_IX;
	ppc_md.hpte_invalidate(slot, vpn, mmu_linear_psize, mmu_linear_psize,
			       mmu_kernel_ssize, 0);
}

void __kernel_map_pages(struct page *page, int numpages, int enable)
{
	unsigned long flags, vaddr, lmi;
	int i;

	local_irq_save(flags);
	for (i = 0; i < numpages; i++, page++) {
		vaddr = (unsigned long)page_address(page);
		lmi = __pa(vaddr) >> PAGE_SHIFT;
		if (lmi >= linear_map_hash_count)
			continue;
		if (enable)
			kernel_map_linear_page(vaddr, lmi);
		else
			kernel_unmap_linear_page(vaddr, lmi);
	}
	local_irq_restore(flags);
}
#endif /* CONFIG_DEBUG_PAGEALLOC */

void setup_initial_memory_limit(phys_addr_t first_memblock_base,
				phys_addr_t first_memblock_size)
{
	/* We don't currently support the first MEMBLOCK not mapping 0
	 * physical on those processors
	 */
	BUG_ON(first_memblock_base != 0);

	/* On LPAR systems, the first entry is our RMA region,
	 * non-LPAR 64-bit hash MMU systems don't have a limitation
	 * on real mode access, but using the first entry works well
	 * enough. We also clamp it to 1G to avoid some funky things
	 * such as RTAS bugs etc...
	 */
	ppc64_rma_size = min_t(u64, first_memblock_size, 0x40000000);

	/* Finally limit subsequent allocations */
	memblock_set_current_limit(ppc64_rma_size);
}
