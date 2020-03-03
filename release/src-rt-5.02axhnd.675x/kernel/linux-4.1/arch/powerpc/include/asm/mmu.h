#ifndef _ASM_POWERPC_MMU_H_
#define _ASM_POWERPC_MMU_H_
#ifdef __KERNEL__

#include <linux/types.h>

#include <asm/asm-compat.h>
#include <asm/feature-fixups.h>

/*
 * MMU features bit definitions
 */

/*
 * First half is MMU families
 */
#define MMU_FTR_HPTE_TABLE		ASM_CONST(0x00000001)
#define MMU_FTR_TYPE_8xx		ASM_CONST(0x00000002)
#define MMU_FTR_TYPE_40x		ASM_CONST(0x00000004)
#define MMU_FTR_TYPE_44x		ASM_CONST(0x00000008)
#define MMU_FTR_TYPE_FSL_E		ASM_CONST(0x00000010)
#define MMU_FTR_TYPE_47x		ASM_CONST(0x00000020)

/*
 * This is individual features
 */

/* Enable use of high BAT registers */
#define MMU_FTR_USE_HIGH_BATS		ASM_CONST(0x00010000)

/* Enable >32-bit physical addresses on 32-bit processor, only used
 * by CONFIG_6xx currently as BookE supports that from day 1
 */
#define MMU_FTR_BIG_PHYS		ASM_CONST(0x00020000)

/* Enable use of broadcast TLB invalidations. We don't always set it
 * on processors that support it due to other constraints with the
 * use of such invalidations
 */
#define MMU_FTR_USE_TLBIVAX_BCAST	ASM_CONST(0x00040000)

/* Enable use of tlbilx invalidate instructions.
 */
#define MMU_FTR_USE_TLBILX		ASM_CONST(0x00080000)

/* This indicates that the processor cannot handle multiple outstanding
 * broadcast tlbivax or tlbsync. This makes the code use a spinlock
 * around such invalidate forms.
 */
#define MMU_FTR_LOCK_BCAST_INVAL	ASM_CONST(0x00100000)

/* This indicates that the processor doesn't handle way selection
 * properly and needs SW to track and update the LRU state.  This
 * is specific to an errata on e300c2/c3/c4 class parts
 */
#define MMU_FTR_NEED_DTLB_SW_LRU	ASM_CONST(0x00200000)

/* Enable use of TLB reservation.  Processor should support tlbsrx.
 * instruction and MAS0[WQ].
 */
#define MMU_FTR_USE_TLBRSRV		ASM_CONST(0x00800000)

/* Use paired MAS registers (MAS7||MAS3, etc.)
 */
#define MMU_FTR_USE_PAIRED_MAS		ASM_CONST(0x01000000)

/* Doesn't support the B bit (1T segment) in SLBIE
 */
#define MMU_FTR_NO_SLBIE_B		ASM_CONST(0x02000000)

/* Support 16M large pages
 */
#define MMU_FTR_16M_PAGE		ASM_CONST(0x04000000)

/* Supports TLBIEL variant
 */
#define MMU_FTR_TLBIEL			ASM_CONST(0x08000000)

/* Supports tlbies w/o locking
 */
#define MMU_FTR_LOCKLESS_TLBIE		ASM_CONST(0x10000000)

/* Large pages can be marked CI
 */
#define MMU_FTR_CI_LARGE_PAGE		ASM_CONST(0x20000000)

/* 1T segments available
 */
#define MMU_FTR_1T_SEGMENT		ASM_CONST(0x40000000)

/* MMU feature bit sets for various CPUs */
#define MMU_FTRS_DEFAULT_HPTE_ARCH_V2	\
	MMU_FTR_HPTE_TABLE | MMU_FTR_PPCAS_ARCH_V2
#define MMU_FTRS_POWER4		MMU_FTRS_DEFAULT_HPTE_ARCH_V2
#define MMU_FTRS_PPC970		MMU_FTRS_POWER4
#define MMU_FTRS_POWER5		MMU_FTRS_POWER4 | MMU_FTR_LOCKLESS_TLBIE
#define MMU_FTRS_POWER6		MMU_FTRS_POWER4 | MMU_FTR_LOCKLESS_TLBIE
#define MMU_FTRS_POWER7		MMU_FTRS_POWER4 | MMU_FTR_LOCKLESS_TLBIE
#define MMU_FTRS_POWER8		MMU_FTRS_POWER4 | MMU_FTR_LOCKLESS_TLBIE
#define MMU_FTRS_CELL		MMU_FTRS_DEFAULT_HPTE_ARCH_V2 | \
				MMU_FTR_CI_LARGE_PAGE
#define MMU_FTRS_PA6T		MMU_FTRS_DEFAULT_HPTE_ARCH_V2 | \
				MMU_FTR_CI_LARGE_PAGE | MMU_FTR_NO_SLBIE_B
#ifndef __ASSEMBLY__
#include <asm/cputable.h>

#ifdef CONFIG_PPC_FSL_BOOK3E
#include <asm/percpu.h>
DECLARE_PER_CPU(int, next_tlbcam_idx);
#endif

static inline int mmu_has_feature(unsigned long feature)
{
	return (cur_cpu_spec->mmu_features & feature);
}

static inline void mmu_clear_feature(unsigned long feature)
{
	cur_cpu_spec->mmu_features &= ~feature;
}

extern unsigned int __start___mmu_ftr_fixup, __stop___mmu_ftr_fixup;

/* MMU initialization */
extern void early_init_mmu(void);
extern void early_init_mmu_secondary(void);

extern void setup_initial_memory_limit(phys_addr_t first_memblock_base,
				       phys_addr_t first_memblock_size);

#ifdef CONFIG_PPC64
/* This is our real memory area size on ppc64 server, on embedded, we
 * make it match the size our of bolted TLB area
 */
extern u64 ppc64_rma_size;
#endif /* CONFIG_PPC64 */

struct mm_struct;
#ifdef CONFIG_DEBUG_VM
extern void assert_pte_locked(struct mm_struct *mm, unsigned long addr);
#else /* CONFIG_DEBUG_VM */
static inline void assert_pte_locked(struct mm_struct *mm, unsigned long addr)
{
}
#endif /* !CONFIG_DEBUG_VM */

#endif /* !__ASSEMBLY__ */

/* The kernel use the constants below to index in the page sizes array.
 * The use of fixed constants for this purpose is better for performances
 * of the low level hash refill handlers.
 *
 * A non supported page size has a "shift" field set to 0
 *
 * Any new page size being implemented can get a new entry in here. Whether
 * the kernel will use it or not is a different matter though. The actual page
 * size used by hugetlbfs is not defined here and may be made variable
 *
 * Note: This array ended up being a false good idea as it's growing to the
 * point where I wonder if we should replace it with something different,
 * to think about, feedback welcome. --BenH.
 */

/* These are #defines as they have to be used in assembly */
#define MMU_PAGE_4K	0
#define MMU_PAGE_16K	1
#define MMU_PAGE_64K	2
#define MMU_PAGE_64K_AP	3	/* "Admixed pages" (hash64 only) */
#define MMU_PAGE_256K	4
#define MMU_PAGE_1M	5
#define MMU_PAGE_2M	6
#define MMU_PAGE_4M	7
#define MMU_PAGE_8M	8
#define MMU_PAGE_16M	9
#define MMU_PAGE_64M	10
#define MMU_PAGE_256M	11
#define MMU_PAGE_1G	12
#define MMU_PAGE_16G	13
#define MMU_PAGE_64G	14

#define MMU_PAGE_COUNT	15

#if defined(CONFIG_PPC_STD_MMU_64)
/* 64-bit classic hash table MMU */
#  include <asm/mmu-hash64.h>
#elif defined(CONFIG_PPC_STD_MMU_32)
/* 32-bit classic hash table MMU */
#  include <asm/mmu-hash32.h>
#elif defined(CONFIG_40x)
/* 40x-style software loaded TLB */
#  include <asm/mmu-40x.h>
#elif defined(CONFIG_44x)
/* 44x-style software loaded TLB */
#  include <asm/mmu-44x.h>
#elif defined(CONFIG_PPC_BOOK3E_MMU)
/* Freescale Book-E software loaded TLB or Book-3e (ISA 2.06+) MMU */
#  include <asm/mmu-book3e.h>
#elif defined (CONFIG_PPC_8xx)
/* Motorola/Freescale 8xx software loaded TLB */
#  include <asm/mmu-8xx.h>
#endif


#endif /* __KERNEL__ */
#endif /* _ASM_POWERPC_MMU_H_ */
