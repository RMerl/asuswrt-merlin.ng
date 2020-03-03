/*
 * TLB Management (flush/create/diagnostics) for ARC700
 *
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * vineetg: Aug 2011
 *  -Reintroduce duplicate PD fixup - some customer chips still have the issue
 *
 * vineetg: May 2011
 *  -No need to flush_cache_page( ) for each call to update_mmu_cache()
 *   some of the LMBench tests improved amazingly
 *      = page-fault thrice as fast (75 usec to 28 usec)
 *      = mmap twice as fast (9.6 msec to 4.6 msec),
 *      = fork (5.3 msec to 3.7 msec)
 *
 * vineetg: April 2011 :
 *  -MMU v3: PD{0,1} bits layout changed: They don't overlap anymore,
 *      helps avoid a shift when preparing PD0 from PTE
 *
 * vineetg: April 2011 : Preparing for MMU V3
 *  -MMU v2/v3 BCRs decoded differently
 *  -Remove TLB_SIZE hardcoding as it's variable now: 256 or 512
 *  -tlb_entry_erase( ) can be void
 *  -local_flush_tlb_range( ):
 *      = need not "ceil" @end
 *      = walks MMU only if range spans < 32 entries, as opposed to 256
 *
 * Vineetg: Sept 10th 2008
 *  -Changes related to MMU v2 (Rel 4.8)
 *
 * Vineetg: Aug 29th 2008
 *  -In TLB Flush operations (Metal Fix MMU) there is a explict command to
 *    flush Micro-TLBS. If TLB Index Reg is invalid prior to TLBIVUTLB cmd,
 *    it fails. Thus need to load it with ANY valid value before invoking
 *    TLBIVUTLB cmd
 *
 * Vineetg: Aug 21th 2008:
 *  -Reduced the duration of IRQ lockouts in TLB Flush routines
 *  -Multiple copies of TLB erase code seperated into a "single" function
 *  -In TLB Flush routines, interrupt disabling moved UP to retrieve ASID
 *       in interrupt-safe region.
 *
 * Vineetg: April 23rd Bug #93131
 *    Problem: tlb_flush_kernel_range() doesnt do anything if the range to
 *              flush is more than the size of TLB itself.
 *
 * Rahul Trivedi : Codito Technologies 2004
 */

#include <linux/module.h>
#include <linux/bug.h>
#include <asm/arcregs.h>
#include <asm/setup.h>
#include <asm/mmu_context.h>
#include <asm/mmu.h>

/*			Need for ARC MMU v2
 *
 * ARC700 MMU-v1 had a Joint-TLB for Code and Data and is 2 way set-assoc.
 * For a memcpy operation with 3 players (src/dst/code) such that all 3 pages
 * map into same set, there would be contention for the 2 ways causing severe
 * Thrashing.
 *
 * Although J-TLB is 2 way set assoc, ARC700 caches J-TLB into uTLBS which has
 * much higher associativity. u-D-TLB is 8 ways, u-I-TLB is 4 ways.
 * Given this, the thrasing problem should never happen because once the 3
 * J-TLB entries are created (even though 3rd will knock out one of the prev
 * two), the u-D-TLB and u-I-TLB will have what is required to accomplish memcpy
 *
 * Yet we still see the Thrashing because a J-TLB Write cause flush of u-TLBs.
 * This is a simple design for keeping them in sync. So what do we do?
 * The solution which James came up was pretty neat. It utilised the assoc
 * of uTLBs by not invalidating always but only when absolutely necessary.
 *
 * - Existing TLB commands work as before
 * - New command (TLBWriteNI) for TLB write without clearing uTLBs
 * - New command (TLBIVUTLB) to invalidate uTLBs.
 *
 * The uTLBs need only be invalidated when pages are being removed from the
 * OS page table. If a 'victim' TLB entry is being overwritten in the main TLB
 * as a result of a miss, the removed entry is still allowed to exist in the
 * uTLBs as it is still valid and present in the OS page table. This allows the
 * full associativity of the uTLBs to hide the limited associativity of the main
 * TLB.
 *
 * During a miss handler, the new "TLBWriteNI" command is used to load
 * entries without clearing the uTLBs.
 *
 * When the OS page table is updated, TLB entries that may be associated with a
 * removed page are removed (flushed) from the TLB using TLBWrite. In this
 * circumstance, the uTLBs must also be cleared. This is done by using the
 * existing TLBWrite command. An explicit IVUTLB is also required for those
 * corner cases when TLBWrite was not executed at all because the corresp
 * J-TLB entry got evicted/replaced.
 */


/* A copy of the ASID from the PID reg is kept in asid_cache */
DEFINE_PER_CPU(unsigned int, asid_cache) = MM_CTXT_FIRST_CYCLE;

/*
 * Utility Routine to erase a J-TLB entry
 * Caller needs to setup Index Reg (manually or via getIndex)
 */
static inline void __tlb_entry_erase(void)
{
	write_aux_reg(ARC_REG_TLBPD1, 0);
	write_aux_reg(ARC_REG_TLBPD0, 0);
	write_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);
}

static inline unsigned int tlb_entry_lkup(unsigned long vaddr_n_asid)
{
	unsigned int idx;

	write_aux_reg(ARC_REG_TLBPD0, vaddr_n_asid);

	write_aux_reg(ARC_REG_TLBCOMMAND, TLBProbe);
	idx = read_aux_reg(ARC_REG_TLBINDEX);

	return idx;
}

static void tlb_entry_erase(unsigned int vaddr_n_asid)
{
	unsigned int idx;

	/* Locate the TLB entry for this vaddr + ASID */
	idx = tlb_entry_lkup(vaddr_n_asid);

	/* No error means entry found, zero it out */
	if (likely(!(idx & TLB_LKUP_ERR))) {
		__tlb_entry_erase();
	} else {
		/* Duplicate entry error */
		WARN(idx == TLB_DUP_ERR, "Probe returned Dup PD for %x\n",
					   vaddr_n_asid);
	}
}

/****************************************************************************
 * ARC700 MMU caches recently used J-TLB entries (RAM) as uTLBs (FLOPs)
 *
 * New IVUTLB cmd in MMU v2 explictly invalidates the uTLB
 *
 * utlb_invalidate ( )
 *  -For v2 MMU calls Flush uTLB Cmd
 *  -For v1 MMU does nothing (except for Metal Fix v1 MMU)
 *      This is because in v1 TLBWrite itself invalidate uTLBs
 ***************************************************************************/

static void utlb_invalidate(void)
{
#if (CONFIG_ARC_MMU_VER >= 2)

#if (CONFIG_ARC_MMU_VER == 2)
	/* MMU v2 introduced the uTLB Flush command.
	 * There was however an obscure hardware bug, where uTLB flush would
	 * fail when a prior probe for J-TLB (both totally unrelated) would
	 * return lkup err - because the entry didnt exist in MMU.
	 * The Workround was to set Index reg with some valid value, prior to
	 * flush. This was fixed in MMU v3 hence not needed any more
	 */
	unsigned int idx;

	/* make sure INDEX Reg is valid */
	idx = read_aux_reg(ARC_REG_TLBINDEX);

	/* If not write some dummy val */
	if (unlikely(idx & TLB_LKUP_ERR))
		write_aux_reg(ARC_REG_TLBINDEX, 0xa);
#endif

	write_aux_reg(ARC_REG_TLBCOMMAND, TLBIVUTLB);
#endif

}

static void tlb_entry_insert(unsigned int pd0, unsigned int pd1)
{
	unsigned int idx;

	/*
	 * First verify if entry for this vaddr+ASID already exists
	 * This also sets up PD0 (vaddr, ASID..) for final commit
	 */
	idx = tlb_entry_lkup(pd0);

	/*
	 * If Not already present get a free slot from MMU.
	 * Otherwise, Probe would have located the entry and set INDEX Reg
	 * with existing location. This will cause Write CMD to over-write
	 * existing entry with new PD0 and PD1
	 */
	if (likely(idx & TLB_LKUP_ERR))
		write_aux_reg(ARC_REG_TLBCOMMAND, TLBGetIndex);

	/* setup the other half of TLB entry (pfn, rwx..) */
	write_aux_reg(ARC_REG_TLBPD1, pd1);

	/*
	 * Commit the Entry to MMU
	 * It doesnt sound safe to use the TLBWriteNI cmd here
	 * which doesn't flush uTLBs. I'd rather be safe than sorry.
	 */
	write_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);
}

/*
 * Un-conditionally (without lookup) erase the entire MMU contents
 */

noinline void local_flush_tlb_all(void)
{
	unsigned long flags;
	unsigned int entry;
	struct cpuinfo_arc_mmu *mmu = &cpuinfo_arc700[smp_processor_id()].mmu;

	local_irq_save(flags);

	/* Load PD0 and PD1 with template for a Blank Entry */
	write_aux_reg(ARC_REG_TLBPD1, 0);
	write_aux_reg(ARC_REG_TLBPD0, 0);

	for (entry = 0; entry < mmu->num_tlb; entry++) {
		/* write this entry to the TLB */
		write_aux_reg(ARC_REG_TLBINDEX, entry);
		write_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);
	}

	utlb_invalidate();

	local_irq_restore(flags);
}

/*
 * Flush the entrie MM for userland. The fastest way is to move to Next ASID
 */
noinline void local_flush_tlb_mm(struct mm_struct *mm)
{
	/*
	 * Small optimisation courtesy IA64
	 * flush_mm called during fork,exit,munmap etc, multiple times as well.
	 * Only for fork( ) do we need to move parent to a new MMU ctxt,
	 * all other cases are NOPs, hence this check.
	 */
	if (atomic_read(&mm->mm_users) == 0)
		return;

	/*
	 * - Move to a new ASID, but only if the mm is still wired in
	 *   (Android Binder ended up calling this for vma->mm != tsk->mm,
	 *    causing h/w - s/w ASID to get out of sync)
	 * - Also get_new_mmu_context() new implementation allocates a new
	 *   ASID only if it is not allocated already - so unallocate first
	 */
	destroy_context(mm);
	if (current->mm == mm)
		get_new_mmu_context(mm);
}

/*
 * Flush a Range of TLB entries for userland.
 * @start is inclusive, while @end is exclusive
 * Difference between this and Kernel Range Flush is
 *  -Here the fastest way (if range is too large) is to move to next ASID
 *      without doing any explicit Shootdown
 *  -In case of kernel Flush, entry has to be shot down explictly
 */
void local_flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
			   unsigned long end)
{
	const unsigned int cpu = smp_processor_id();
	unsigned long flags;

	/* If range @start to @end is more than 32 TLB entries deep,
	 * its better to move to a new ASID rather than searching for
	 * individual entries and then shooting them down
	 *
	 * The calc above is rough, doesn't account for unaligned parts,
	 * since this is heuristics based anyways
	 */
	if (unlikely((end - start) >= PAGE_SIZE * 32)) {
		local_flush_tlb_mm(vma->vm_mm);
		return;
	}

	/*
	 * @start moved to page start: this alone suffices for checking
	 * loop end condition below, w/o need for aligning @end to end
	 * e.g. 2000 to 4001 will anyhow loop twice
	 */
	start &= PAGE_MASK;

	local_irq_save(flags);

	if (asid_mm(vma->vm_mm, cpu) != MM_CTXT_NO_ASID) {
		while (start < end) {
			tlb_entry_erase(start | hw_pid(vma->vm_mm, cpu));
			start += PAGE_SIZE;
		}
	}

	utlb_invalidate();

	local_irq_restore(flags);
}

/* Flush the kernel TLB entries - vmalloc/modules (Global from MMU perspective)
 *  @start, @end interpreted as kvaddr
 * Interestingly, shared TLB entries can also be flushed using just
 * @start,@end alone (interpreted as user vaddr), although technically SASID
 * is also needed. However our smart TLbProbe lookup takes care of that.
 */
void local_flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	unsigned long flags;

	/* exactly same as above, except for TLB entry not taking ASID */

	if (unlikely((end - start) >= PAGE_SIZE * 32)) {
		local_flush_tlb_all();
		return;
	}

	start &= PAGE_MASK;

	local_irq_save(flags);
	while (start < end) {
		tlb_entry_erase(start);
		start += PAGE_SIZE;
	}

	utlb_invalidate();

	local_irq_restore(flags);
}

/*
 * Delete TLB entry in MMU for a given page (??? address)
 * NOTE One TLB entry contains translation for single PAGE
 */

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
{
	const unsigned int cpu = smp_processor_id();
	unsigned long flags;

	/* Note that it is critical that interrupts are DISABLED between
	 * checking the ASID and using it flush the TLB entry
	 */
	local_irq_save(flags);

	if (asid_mm(vma->vm_mm, cpu) != MM_CTXT_NO_ASID) {
		tlb_entry_erase((page & PAGE_MASK) | hw_pid(vma->vm_mm, cpu));
		utlb_invalidate();
	}

	local_irq_restore(flags);
}

#ifdef CONFIG_SMP

struct tlb_args {
	struct vm_area_struct *ta_vma;
	unsigned long ta_start;
	unsigned long ta_end;
};

static inline void ipi_flush_tlb_page(void *arg)
{
	struct tlb_args *ta = arg;

	local_flush_tlb_page(ta->ta_vma, ta->ta_start);
}

static inline void ipi_flush_tlb_range(void *arg)
{
	struct tlb_args *ta = arg;

	local_flush_tlb_range(ta->ta_vma, ta->ta_start, ta->ta_end);
}

static inline void ipi_flush_tlb_kernel_range(void *arg)
{
	struct tlb_args *ta = (struct tlb_args *)arg;

	local_flush_tlb_kernel_range(ta->ta_start, ta->ta_end);
}

void flush_tlb_all(void)
{
	on_each_cpu((smp_call_func_t)local_flush_tlb_all, NULL, 1);
}

void flush_tlb_mm(struct mm_struct *mm)
{
	on_each_cpu_mask(mm_cpumask(mm), (smp_call_func_t)local_flush_tlb_mm,
			 mm, 1);
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long uaddr)
{
	struct tlb_args ta = {
		.ta_vma = vma,
		.ta_start = uaddr
	};

	on_each_cpu_mask(mm_cpumask(vma->vm_mm), ipi_flush_tlb_page, &ta, 1);
}

void flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
		     unsigned long end)
{
	struct tlb_args ta = {
		.ta_vma = vma,
		.ta_start = start,
		.ta_end = end
	};

	on_each_cpu_mask(mm_cpumask(vma->vm_mm), ipi_flush_tlb_range, &ta, 1);
}

void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	struct tlb_args ta = {
		.ta_start = start,
		.ta_end = end
	};

	on_each_cpu(ipi_flush_tlb_kernel_range, &ta, 1);
}
#endif

/*
 * Routine to create a TLB entry
 */
void create_tlb(struct vm_area_struct *vma, unsigned long address, pte_t *ptep)
{
	unsigned long flags;
	unsigned int asid_or_sasid, rwx;
	unsigned long pd0, pd1;

	/*
	 * create_tlb() assumes that current->mm == vma->mm, since
	 * -it ASID for TLB entry is fetched from MMU ASID reg (valid for curr)
	 * -completes the lazy write to SASID reg (again valid for curr tsk)
	 *
	 * Removing the assumption involves
	 * -Using vma->mm->context{ASID,SASID}, as opposed to MMU reg.
	 * -Fix the TLB paranoid debug code to not trigger false negatives.
	 * -More importantly it makes this handler inconsistent with fast-path
	 *  TLB Refill handler which always deals with "current"
	 *
	 * Lets see the use cases when current->mm != vma->mm and we land here
	 *  1. execve->copy_strings()->__get_user_pages->handle_mm_fault
	 *     Here VM wants to pre-install a TLB entry for user stack while
	 *     current->mm still points to pre-execve mm (hence the condition).
	 *     However the stack vaddr is soon relocated (randomization) and
	 *     move_page_tables() tries to undo that TLB entry.
	 *     Thus not creating TLB entry is not any worse.
	 *
	 *  2. ptrace(POKETEXT) causes a CoW - debugger(current) inserting a
	 *     breakpoint in debugged task. Not creating a TLB now is not
	 *     performance critical.
	 *
	 * Both the cases above are not good enough for code churn.
	 */
	if (current->active_mm != vma->vm_mm)
		return;

	local_irq_save(flags);

	tlb_paranoid_check(asid_mm(vma->vm_mm, smp_processor_id()), address);

	address &= PAGE_MASK;

	/* update this PTE credentials */
	pte_val(*ptep) |= (_PAGE_PRESENT | _PAGE_ACCESSED);

	/* Create HW TLB(PD0,PD1) from PTE  */

	/* ASID for this task */
	asid_or_sasid = read_aux_reg(ARC_REG_PID) & 0xff;

	pd0 = address | asid_or_sasid | (pte_val(*ptep) & PTE_BITS_IN_PD0);

	/*
	 * ARC MMU provides fully orthogonal access bits for K/U mode,
	 * however Linux only saves 1 set to save PTE real-estate
	 * Here we convert 3 PTE bits into 6 MMU bits:
	 * -Kernel only entries have Kr Kw Kx 0 0 0
	 * -User entries have mirrored K and U bits
	 */
	rwx = pte_val(*ptep) & PTE_BITS_RWX;

	if (pte_val(*ptep) & _PAGE_GLOBAL)
		rwx <<= 3;		/* r w x => Kr Kw Kx 0 0 0 */
	else
		rwx |= (rwx << 3);	/* r w x => Kr Kw Kx Ur Uw Ux */

	pd1 = rwx | (pte_val(*ptep) & PTE_BITS_NON_RWX_IN_PD1);

	tlb_entry_insert(pd0, pd1);

	local_irq_restore(flags);
}

/*
 * Called at the end of pagefault, for a userspace mapped page
 *  -pre-install the corresponding TLB entry into MMU
 *  -Finalize the delayed D-cache flush of kernel mapping of page due to
 *  	flush_dcache_page(), copy_user_page()
 *
 * Note that flush (when done) involves both WBACK - so physical page is
 * in sync as well as INV - so any non-congruent aliases don't remain
 */
void update_mmu_cache(struct vm_area_struct *vma, unsigned long vaddr_unaligned,
		      pte_t *ptep)
{
	unsigned long vaddr = vaddr_unaligned & PAGE_MASK;
	unsigned long paddr = pte_val(*ptep) & PAGE_MASK;
	struct page *page = pfn_to_page(pte_pfn(*ptep));

	create_tlb(vma, vaddr, ptep);

	if (page == ZERO_PAGE(0)) {
		return;
	}

	/*
	 * Exec page : Independent of aliasing/page-color considerations,
	 *	       since icache doesn't snoop dcache on ARC, any dirty
	 *	       K-mapping of a code page needs to be wback+inv so that
	 *	       icache fetch by userspace sees code correctly.
	 * !EXEC page: If K-mapping is NOT congruent to U-mapping, flush it
	 *	       so userspace sees the right data.
	 *  (Avoids the flush for Non-exec + congruent mapping case)
	 */
	if ((vma->vm_flags & VM_EXEC) ||
	     addr_not_cache_congruent(paddr, vaddr)) {

		int dirty = !test_and_set_bit(PG_dc_clean, &page->flags);
		if (dirty) {
			/* wback + inv dcache lines */
			__flush_dcache_page(paddr, paddr);

			/* invalidate any existing icache lines */
			if (vma->vm_flags & VM_EXEC)
				__inv_icache_page(paddr, vaddr);
		}
	}
}

/* Read the Cache Build Confuration Registers, Decode them and save into
 * the cpuinfo structure for later use.
 * No Validation is done here, simply read/convert the BCRs
 */
void read_decode_mmu_bcr(void)
{
	struct cpuinfo_arc_mmu *mmu = &cpuinfo_arc700[smp_processor_id()].mmu;
	unsigned int tmp;
	struct bcr_mmu_1_2 {
#ifdef CONFIG_CPU_BIG_ENDIAN
		unsigned int ver:8, ways:4, sets:4, u_itlb:8, u_dtlb:8;
#else
		unsigned int u_dtlb:8, u_itlb:8, sets:4, ways:4, ver:8;
#endif
	} *mmu2;

	struct bcr_mmu_3 {
#ifdef CONFIG_CPU_BIG_ENDIAN
	unsigned int ver:8, ways:4, sets:4, osm:1, reserv:3, pg_sz:4,
		     u_itlb:4, u_dtlb:4;
#else
	unsigned int u_dtlb:4, u_itlb:4, pg_sz:4, reserv:3, osm:1, sets:4,
		     ways:4, ver:8;
#endif
	} *mmu3;

	tmp = read_aux_reg(ARC_REG_MMU_BCR);
	mmu->ver = (tmp >> 24);

	if (mmu->ver <= 2) {
		mmu2 = (struct bcr_mmu_1_2 *)&tmp;
		mmu->pg_sz = PAGE_SIZE;
		mmu->sets = 1 << mmu2->sets;
		mmu->ways = 1 << mmu2->ways;
		mmu->u_dtlb = mmu2->u_dtlb;
		mmu->u_itlb = mmu2->u_itlb;
	} else {
		mmu3 = (struct bcr_mmu_3 *)&tmp;
		mmu->pg_sz = 512 << mmu3->pg_sz;
		mmu->sets = 1 << mmu3->sets;
		mmu->ways = 1 << mmu3->ways;
		mmu->u_dtlb = mmu3->u_dtlb;
		mmu->u_itlb = mmu3->u_itlb;
	}

	mmu->num_tlb = mmu->sets * mmu->ways;
}

char *arc_mmu_mumbojumbo(int cpu_id, char *buf, int len)
{
	int n = 0;
	struct cpuinfo_arc_mmu *p_mmu = &cpuinfo_arc700[cpu_id].mmu;

	n += scnprintf(buf + n, len - n,
		      "MMU [v%x]\t: %dk PAGE, JTLB %d (%dx%d), uDTLB %d, uITLB %d %s\n",
		       p_mmu->ver, TO_KB(p_mmu->pg_sz),
		       p_mmu->num_tlb, p_mmu->sets, p_mmu->ways,
		       p_mmu->u_dtlb, p_mmu->u_itlb,
		       IS_ENABLED(CONFIG_ARC_MMU_SASID) ? ",SASID" : "");

	return buf;
}

void arc_mmu_init(void)
{
	char str[256];
	struct cpuinfo_arc_mmu *mmu = &cpuinfo_arc700[smp_processor_id()].mmu;

	printk(arc_mmu_mumbojumbo(0, str, sizeof(str)));

	/* For efficiency sake, kernel is compile time built for a MMU ver
	 * This must match the hardware it is running on.
	 * Linux built for MMU V2, if run on MMU V1 will break down because V1
	 *  hardware doesn't understand cmds such as WriteNI, or IVUTLB
	 * On the other hand, Linux built for V1 if run on MMU V2 will do
	 *   un-needed workarounds to prevent memcpy thrashing.
	 * Similarly MMU V3 has new features which won't work on older MMU
	 */
	if (mmu->ver != CONFIG_ARC_MMU_VER) {
		panic("MMU ver %d doesn't match kernel built for %d...\n",
		      mmu->ver, CONFIG_ARC_MMU_VER);
	}

	if (mmu->pg_sz != PAGE_SIZE)
		panic("MMU pg size != PAGE_SIZE (%luk)\n", TO_KB(PAGE_SIZE));

	/* Enable the MMU */
	write_aux_reg(ARC_REG_PID, MMU_ENABLE);

	/* In smp we use this reg for interrupt 1 scratch */
#ifndef CONFIG_SMP
	/* swapper_pg_dir is the pgd for the kernel, used by vmalloc */
	write_aux_reg(ARC_REG_SCRATCH_DATA0, swapper_pg_dir);
#endif
}

/*
 * TLB Programmer's Model uses Linear Indexes: 0 to {255, 511} for 128 x {2,4}
 * The mapping is Column-first.
 *		---------------------	-----------
 *		|way0|way1|way2|way3|	|way0|way1|
 *		---------------------	-----------
 * [set0]	|  0 |  1 |  2 |  3 |	|  0 |  1 |
 * [set1]	|  4 |  5 |  6 |  7 |	|  2 |  3 |
 *		~		    ~	~	  ~
 * [set127]	| 508| 509| 510| 511|	| 254| 255|
 *		---------------------	-----------
 * For normal operations we don't(must not) care how above works since
 * MMU cmd getIndex(vaddr) abstracts that out.
 * However for walking WAYS of a SET, we need to know this
 */
#define SET_WAY_TO_IDX(mmu, set, way)  ((set) * mmu->ways + (way))

/* Handling of Duplicate PD (TLB entry) in MMU.
 * -Could be due to buggy customer tapeouts or obscure kernel bugs
 * -MMU complaints not at the time of duplicate PD installation, but at the
 *      time of lookup matching multiple ways.
 * -Ideally these should never happen - but if they do - workaround by deleting
 *      the duplicate one.
 * -Knob to be verbose abt it.(TODO: hook them up to debugfs)
 */
volatile int dup_pd_verbose = 1;/* Be slient abt it or complain (default) */

void do_tlb_overlap_fault(unsigned long cause, unsigned long address,
			  struct pt_regs *regs)
{
	int set, way, n;
	unsigned long flags, is_valid;
	struct cpuinfo_arc_mmu *mmu = &cpuinfo_arc700[smp_processor_id()].mmu;
	unsigned int pd0[mmu->ways], pd1[mmu->ways];

	local_irq_save(flags);

	/* loop thru all sets of TLB */
	for (set = 0; set < mmu->sets; set++) {

		/* read out all the ways of current set */
		for (way = 0, is_valid = 0; way < mmu->ways; way++) {
			write_aux_reg(ARC_REG_TLBINDEX,
					  SET_WAY_TO_IDX(mmu, set, way));
			write_aux_reg(ARC_REG_TLBCOMMAND, TLBRead);
			pd0[way] = read_aux_reg(ARC_REG_TLBPD0);
			pd1[way] = read_aux_reg(ARC_REG_TLBPD1);
			is_valid |= pd0[way] & _PAGE_PRESENT;
		}

		/* If all the WAYS in SET are empty, skip to next SET */
		if (!is_valid)
			continue;

		/* Scan the set for duplicate ways: needs a nested loop */
		for (way = 0; way < mmu->ways - 1; way++) {
			if (!pd0[way])
				continue;

			for (n = way + 1; n < mmu->ways; n++) {
				if ((pd0[way] & PAGE_MASK) ==
				    (pd0[n] & PAGE_MASK)) {

					if (dup_pd_verbose) {
						pr_info("Duplicate PD's @"
							"[%d:%d]/[%d:%d]\n",
						     set, way, set, n);
						pr_info("TLBPD0[%u]: %08x\n",
						     way, pd0[way]);
					}

					/*
					 * clear entry @way and not @n. This is
					 * critical to our optimised loop
					 */
					pd0[way] = pd1[way] = 0;
					write_aux_reg(ARC_REG_TLBINDEX,
						SET_WAY_TO_IDX(mmu, set, way));
					__tlb_entry_erase();
				}
			}
		}
	}

	local_irq_restore(flags);
}

/***********************************************************************
 * Diagnostic Routines
 *  -Called from Low Level TLB Hanlders if things don;t look good
 **********************************************************************/

#ifdef CONFIG_ARC_DBG_TLB_PARANOIA

/*
 * Low Level ASM TLB handler calls this if it finds that HW and SW ASIDS
 * don't match
 */
void print_asid_mismatch(int mm_asid, int mmu_asid, int is_fast_path)
{
	pr_emerg("ASID Mismatch in %s Path Handler: sw-pid=0x%x hw-pid=0x%x\n",
	       is_fast_path ? "Fast" : "Slow", mm_asid, mmu_asid);

	__asm__ __volatile__("flag 1");
}

void tlb_paranoid_check(unsigned int mm_asid, unsigned long addr)
{
	unsigned int mmu_asid;

	mmu_asid = read_aux_reg(ARC_REG_PID) & 0xff;

	/*
	 * At the time of a TLB miss/installation
	 *   - HW version needs to match SW version
	 *   - SW needs to have a valid ASID
	 */
	if (addr < 0x70000000 &&
	    ((mm_asid == MM_CTXT_NO_ASID) ||
	      (mmu_asid != (mm_asid & MM_CTXT_ASID_MASK))))
		print_asid_mismatch(mm_asid, mmu_asid, 0);
}
#endif
