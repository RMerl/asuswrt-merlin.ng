// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <linux/compiler.h>
#include <asm/armv7_mpu.h>

#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_ARM_MMU
__weak void arm_init_before_mmu(void)
{
}

__weak void arm_init_domains(void)
{
}

#ifdef CONFIG_ARMV7_LPAE
void set_section_attr(int section, u64 virt, u64 attr)
#else
void set_section_attr(int section, u32 virt, u32 attr)
#endif
{
#ifdef CONFIG_ARMV7_LPAE
	u64 *page_table = (u64 *)gd->arch.tlb_addr;
	u64 value = virt;
#else
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
	u32 value = virt;
#endif
	/* Add page attribute bits */
	value |= attr;

	/* Set PTE */
	page_table[section] = value;
}

void set_section_dcache(int section, enum dcache_option option)
{
#ifdef CONFIG_ARMV7_LPAE
	u64 *page_table = (u64 *)gd->arch.tlb_addr;
	/* Need to set the access flag to not fault */
	u64 value = TTB_SECT_AP | TTB_SECT_AF;
#else
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
	u32 value = TTB_SECT_AP;
#endif

	/* Add the page offset */
	value |= ((u32)section << MMU_SECTION_SHIFT);

	/* Add caching bits */
	value |= option;

	/* Set PTE */
	page_table[section] = value;
}

__weak void mmu_page_table_flush(unsigned long start, unsigned long stop)
{
	debug("%s: Warning: not implemented\n", __func__);
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
#ifdef CONFIG_ARMV7_LPAE
	u64 *page_table = (u64 *)gd->arch.tlb_addr;
#else
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
#endif
	unsigned long startpt, stoppt;
	unsigned long upto, end;

	end = ALIGN(start + size, MMU_SECTION_SIZE) >> MMU_SECTION_SHIFT;
	start = start >> MMU_SECTION_SHIFT;
#ifdef CONFIG_ARMV7_LPAE
	debug("%s: start=%pa, size=%zu, option=%llx\n", __func__, &start, size,
	      option);
#else
	debug("%s: start=%pa, size=%zu, option=0x%x\n", __func__, &start, size,
	      option);
#endif
	for (upto = start; upto < end; upto++)
		set_section_dcache(upto, option);

	/*
	 * Make sure range is cache line aligned
	 * Only CPU maintains page tables, hence it is safe to always
	 * flush complete cache lines...
	 */

	startpt = (unsigned long)&page_table[start];
	startpt &= ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	stoppt = (unsigned long)&page_table[end];
	stoppt = ALIGN(stoppt, CONFIG_SYS_CACHELINE_SIZE);
	mmu_page_table_flush(startpt, stoppt);
}

__weak void dram_bank_mmu_setup(int bank)
{
	bd_t *bd = gd->bd;
	int	i;

	debug("%s: bank: %d\n", __func__, bank);
	for (i = bd->bi_dram[bank].start >> MMU_SECTION_SHIFT;
	     i < (bd->bi_dram[bank].start >> MMU_SECTION_SHIFT) +
		 (bd->bi_dram[bank].size >> MMU_SECTION_SHIFT);
	     i++) {
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
		set_section_dcache(i, DCACHE_WRITETHROUGH);
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
		set_section_dcache(i, DCACHE_WRITEALLOC);
#else
		set_section_dcache(i, DCACHE_WRITEBACK);
#endif
	}
}

/* to activate the MMU we need to set up virtual memory: use 1M areas */
static inline void mmu_setup(void)
{
	int i;
	u32 reg;

	arm_init_before_mmu();
	/* Set up an identity-mapping for all 4GB, rw for everyone */
	for (i = 0; i < ((4096ULL * 1024 * 1024) >> MMU_SECTION_SHIFT); i++)
		set_section_dcache(i, DCACHE_OFF);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		dram_bank_mmu_setup(i);
	}

#if defined(CONFIG_ARMV7_LPAE) && __LINUX_ARM_ARCH__ != 4
	/* Set up 4 PTE entries pointing to our 4 1GB page tables */
	for (i = 0; i < 4; i++) {
		u64 *page_table = (u64 *)(gd->arch.tlb_addr + (4096 * 4));
		u64 tpt = gd->arch.tlb_addr + (4096 * i);
		page_table[i] = tpt | TTB_PAGETABLE;
	}

	reg = TTBCR_EAE;
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	reg |= TTBCR_ORGN0_WT | TTBCR_IRGN0_WT;
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
	reg |= TTBCR_ORGN0_WBWA | TTBCR_IRGN0_WBWA;
#else
	reg |= TTBCR_ORGN0_WBNWA | TTBCR_IRGN0_WBNWA;
#endif

	if (is_hyp()) {
		/* Set HTCR to enable LPAE */
		asm volatile("mcr p15, 4, %0, c2, c0, 2"
			: : "r" (reg) : "memory");
		/* Set HTTBR0 */
		asm volatile("mcrr p15, 4, %0, %1, c2"
			:
			: "r"(gd->arch.tlb_addr + (4096 * 4)), "r"(0)
			: "memory");
		/* Set HMAIR */
		asm volatile("mcr p15, 4, %0, c10, c2, 0"
			: : "r" (MEMORY_ATTRIBUTES) : "memory");
	} else {
		/* Set TTBCR to enable LPAE */
		asm volatile("mcr p15, 0, %0, c2, c0, 2"
			: : "r" (reg) : "memory");
		/* Set 64-bit TTBR0 */
		asm volatile("mcrr p15, 0, %0, %1, c2"
			:
			: "r"(gd->arch.tlb_addr + (4096 * 4)), "r"(0)
			: "memory");
		/* Set MAIR */
		asm volatile("mcr p15, 0, %0, c10, c2, 0"
			: : "r" (MEMORY_ATTRIBUTES) : "memory");
	}
#elif defined(CONFIG_CPU_V7A)
	if (is_hyp()) {
		/* Set HTCR to disable LPAE */
		asm volatile("mcr p15, 4, %0, c2, c0, 2"
			: : "r" (0) : "memory");
	} else {
		/* Set TTBCR to disable LPAE */
		asm volatile("mcr p15, 0, %0, c2, c0, 2"
			: : "r" (0) : "memory");
	}
	/* Set TTBR0 */
	reg = gd->arch.tlb_addr & TTBR0_BASE_ADDR_MASK;
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	reg |= TTBR0_RGN_WT | TTBR0_IRGN_WT;
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
	reg |= TTBR0_RGN_WBWA | TTBR0_IRGN_WBWA;
#else
	reg |= TTBR0_RGN_WB | TTBR0_IRGN_WB;
#endif
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (reg) : "memory");
#else
	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		     : : "r" (gd->arch.tlb_addr) : "memory");
#endif
	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (~0));

	arm_init_domains();

	/* and enable the mmu */
	reg = get_cr();	/* get control reg. */
	set_cr(reg | CR_M);
}

static int mmu_enabled(void)
{
	return get_cr() & CR_M;
}
#endif /* CONFIG_SYS_ARM_MMU */

/* cache_bit must be either CR_I or CR_C */
static void cache_enable(uint32_t cache_bit)
{
	uint32_t reg;

	/* The data cache is not active unless the mmu/mpu is enabled too */
#ifdef CONFIG_SYS_ARM_MMU
	if ((cache_bit == CR_C) && !mmu_enabled())
		mmu_setup();
#elif defined(CONFIG_SYS_ARM_MPU)
	if ((cache_bit == CR_C) && !mpu_enabled()) {
		printf("Consider enabling MPU before enabling caches\n");
		return;
	}
#endif
	reg = get_cr();	/* get control reg. */
	set_cr(reg | cache_bit);
}

/* cache_bit must be either CR_I or CR_C */
static void cache_disable(uint32_t cache_bit)
{
	uint32_t reg;

	reg = get_cr();

	if (cache_bit == CR_C) {
		/* if cache isn;t enabled no need to disable */
		if ((reg & CR_C) != CR_C)
			return;
		/* if disabling data cache, disable mmu too */
		cache_bit |= CR_M;
	}
	reg = get_cr();

	if (cache_bit == (CR_C | CR_M))
		flush_dcache_all();
	set_cr(reg & ~cache_bit);
}
#endif

#if CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
void icache_enable (void)
{
	return;
}

void icache_disable (void)
{
	return;
}

int icache_status (void)
{
	return 0;					/* always off */
}
#else
void icache_enable(void)
{
	cache_enable(CR_I);
}

void icache_disable(void)
{
	cache_disable(CR_I);
}

int icache_status(void)
{
	return (get_cr() & CR_I) != 0;
}
#endif

#if CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void dcache_enable (void)
{
	return;
}

void dcache_disable (void)
{
	return;
}

int dcache_status (void)
{
	return 0;					/* always off */
}
#else
void dcache_enable(void)
{
	cache_enable(CR_C);
}

void dcache_disable(void)
{
	cache_disable(CR_C);
}

int dcache_status(void)
{
	return (get_cr() & CR_C) != 0;
}
#endif
