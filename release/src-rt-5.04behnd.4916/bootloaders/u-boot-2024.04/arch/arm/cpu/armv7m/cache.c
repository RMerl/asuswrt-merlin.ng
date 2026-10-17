// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <log.h>
#include <asm/armv7m.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* Cache maintenance operation registers */

#define V7M_CACHE_REG_ICIALLU		((u32 *)(V7M_CACHE_MAINT_BASE + 0x00))
#define INVAL_ICACHE_POU		0
#define V7M_CACHE_REG_ICIMVALU		((u32 *)(V7M_CACHE_MAINT_BASE + 0x08))
#define V7M_CACHE_REG_DCIMVAC		((u32 *)(V7M_CACHE_MAINT_BASE + 0x0C))
#define V7M_CACHE_REG_DCISW		((u32 *)(V7M_CACHE_MAINT_BASE + 0x10))
#define V7M_CACHE_REG_DCCMVAU		((u32 *)(V7M_CACHE_MAINT_BASE + 0x14))
#define V7M_CACHE_REG_DCCMVAC		((u32 *)(V7M_CACHE_MAINT_BASE + 0x18))
#define V7M_CACHE_REG_DCCSW		((u32 *)(V7M_CACHE_MAINT_BASE + 0x1C))
#define V7M_CACHE_REG_DCCIMVAC		((u32 *)(V7M_CACHE_MAINT_BASE + 0x20))
#define V7M_CACHE_REG_DCCISW		((u32 *)(V7M_CACHE_MAINT_BASE + 0x24))
#define WAYS_SHIFT			30
#define SETS_SHIFT			5

/* armv7m processor feature registers */

#define V7M_PROC_REG_CLIDR		((u32 *)(V7M_PROC_FTR_BASE + 0x00))
#define V7M_PROC_REG_CTR		((u32 *)(V7M_PROC_FTR_BASE + 0x04))
#define V7M_PROC_REG_CCSIDR		((u32 *)(V7M_PROC_FTR_BASE + 0x08))
#define MASK_NUM_WAYS			GENMASK(12, 3)
#define MASK_NUM_SETS			GENMASK(27, 13)
#define CLINE_SIZE_MASK			GENMASK(2, 0)
#define NUM_WAYS_SHIFT			3
#define NUM_SETS_SHIFT			13
#define V7M_PROC_REG_CSSELR		((u32 *)(V7M_PROC_FTR_BASE + 0x0C))
#define SEL_I_OR_D			BIT(0)

enum cache_type {
	DCACHE,
	ICACHE,
};

/* PoU : Point of Unification, Poc: Point of Coherency */
enum cache_action {
	INVALIDATE_POU,		/* i-cache invalidate by address */
	INVALIDATE_POC,		/* d-cache invalidate by address */
	INVALIDATE_SET_WAY,	/* d-cache invalidate by sets/ways */
	FLUSH_POU,		/* d-cache clean by address to the PoU */
	FLUSH_POC,		/* d-cache clean by address to the PoC */
	FLUSH_SET_WAY,		/* d-cache clean by sets/ways */
	FLUSH_INVAL_POC,	/* d-cache clean & invalidate by addr to PoC */
	FLUSH_INVAL_SET_WAY,	/* d-cache clean & invalidate by set/ways */
};

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
struct dcache_config {
	u32 ways;
	u32 sets;
};

static void get_cache_ways_sets(struct dcache_config *cache)
{
	u32 cache_size_id = readl(V7M_PROC_REG_CCSIDR);

	cache->ways = (cache_size_id & MASK_NUM_WAYS) >> NUM_WAYS_SHIFT;
	cache->sets = (cache_size_id & MASK_NUM_SETS) >> NUM_SETS_SHIFT;
}

/*
 * Return the io register to perform required cache action like clean or clean
 * & invalidate by sets/ways.
 */
static u32 *get_action_reg_set_ways(enum cache_action action)
{
	switch (action) {
	case INVALIDATE_SET_WAY:
		return V7M_CACHE_REG_DCISW;
	case FLUSH_SET_WAY:
		return V7M_CACHE_REG_DCCSW;
	case FLUSH_INVAL_SET_WAY:
		return V7M_CACHE_REG_DCCISW;
	default:
		break;
	};

	return NULL;
}

/*
 * Return the io register to perform required cache action like clean or clean
 * & invalidate by adddress or range.
 */
static u32 *get_action_reg_range(enum cache_action action)
{
	switch (action) {
	case INVALIDATE_POU:
		return V7M_CACHE_REG_ICIMVALU;
	case INVALIDATE_POC:
		return V7M_CACHE_REG_DCIMVAC;
	case FLUSH_POU:
		return V7M_CACHE_REG_DCCMVAU;
	case FLUSH_POC:
		return V7M_CACHE_REG_DCCMVAC;
	case FLUSH_INVAL_POC:
		return V7M_CACHE_REG_DCCIMVAC;
	default:
		break;
	}

	return NULL;
}

static u32 get_cline_size(enum cache_type type)
{
	u32 size;

	if (type == DCACHE)
		clrbits_le32(V7M_PROC_REG_CSSELR, BIT(SEL_I_OR_D));
	else if (type == ICACHE)
		setbits_le32(V7M_PROC_REG_CSSELR, BIT(SEL_I_OR_D));
	/* Make sure cache selection is effective for next memory access */
	dsb();

	size = readl(V7M_PROC_REG_CCSIDR) & CLINE_SIZE_MASK;
	/* Size enocoded as 2 less than log(no_of_words_in_cache_line) base 2 */
	size = 1 << (size + 2);
	debug("cache line size is %d\n", size);

	return size;
}

/* Perform the action like invalidate/clean on a range of cache addresses */
static int action_cache_range(enum cache_action action, u32 start_addr,
			      int64_t size)
{
	u32 cline_size;
	u32 *action_reg;
	enum cache_type type;

	action_reg = get_action_reg_range(action);
	if (!action_reg)
		return -EINVAL;
	if (action == INVALIDATE_POU)
		type = ICACHE;
	else
		type = DCACHE;

	/* Cache line size is minium size for the cache action */
	cline_size = get_cline_size(type);
	/* Align start address to cache line boundary */
	start_addr &= ~(cline_size - 1);
	debug("total size for cache action = %llx\n", size);
	do {
		writel(start_addr, action_reg);
		size -= cline_size;
		start_addr += cline_size;
	} while (size > cline_size);

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
	debug("cache action on range done\n");

	return 0;
}

/* Perform the action like invalidate/clean on all cached addresses */
static int action_dcache_all(enum cache_action action)
{
	struct dcache_config cache;
	u32 *action_reg;
	int i, j;

	action_reg = get_action_reg_set_ways(action);
	if (!action_reg)
		return -EINVAL;

	clrbits_le32(V7M_PROC_REG_CSSELR, BIT(SEL_I_OR_D));
	/* Make sure cache selection is effective for next memory access */
	dsb();

	get_cache_ways_sets(&cache);	/* Get number of ways & sets */
	debug("cache: ways= %d, sets= %d\n", cache.ways + 1, cache.sets + 1);
	for (i = cache.sets; i >= 0; i--) {
		for (j = cache.ways; j >= 0; j--) {
			writel((j << WAYS_SHIFT) | (i << SETS_SHIFT),
			       action_reg);
		}
	}

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */

	return 0;
}

void dcache_enable(void)
{
	if (dcache_status())	/* return if cache already enabled */
		return;

	if (action_dcache_all(INVALIDATE_SET_WAY)) {
		printf("ERR: D-cache not enabled\n");
		return;
	}

	setbits_le32(&V7M_SCB->ccr, BIT(V7M_CCR_DCACHE));

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
}

void dcache_disable(void)
{
	if (!dcache_status())
		return;

	/* if dcache is enabled-> dcache disable & then flush */
	if (action_dcache_all(FLUSH_SET_WAY)) {
		printf("ERR: D-cache not flushed\n");
		return;
	}

	clrbits_le32(&V7M_SCB->ccr, BIT(V7M_CCR_DCACHE));

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
}

int dcache_status(void)
{
	return (readl(&V7M_SCB->ccr) & BIT(V7M_CCR_DCACHE)) != 0;
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	if (action_cache_range(INVALIDATE_POC, start, stop - start)) {
		printf("ERR: D-cache not invalidated\n");
		return;
	}
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	if (action_cache_range(FLUSH_POC, start, stop - start)) {
		printf("ERR: D-cache not flushed\n");
		return;
	}
}
void flush_dcache_all(void)
{
	if (action_dcache_all(FLUSH_SET_WAY)) {
		printf("ERR: D-cache not flushed\n");
		return;
	}
}

void invalidate_dcache_all(void)
{
	if (action_dcache_all(INVALIDATE_SET_WAY)) {
		printf("ERR: D-cache not invalidated\n");
		return;
	}
}
#else
void dcache_enable(void)
{
	return;
}

void dcache_disable(void)
{
	return;
}

int dcache_status(void)
{
	return 0;
}

void flush_dcache_all(void)
{
}

void invalidate_dcache_all(void)
{
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
}

#endif

#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)

void invalidate_icache_all(void)
{
	writel(INVAL_ICACHE_POU, V7M_CACHE_REG_ICIALLU);

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
}

void icache_enable(void)
{
	if (icache_status())
		return;

	invalidate_icache_all();
	setbits_le32(&V7M_SCB->ccr, BIT(V7M_CCR_ICACHE));

	/* Make sure cache action is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
}

int icache_status(void)
{
	return (readl(&V7M_SCB->ccr) & BIT(V7M_CCR_ICACHE)) != 0;
}

void icache_disable(void)
{
	if (!icache_status())
		return;

	isb();	/* flush pipeline */
	clrbits_le32(&V7M_SCB->ccr, BIT(V7M_CCR_ICACHE));
	isb();	/* subsequent instructions fetch see cache disable effect */
}
#else
void invalidate_icache_all(void)
{
	return;
}

void icache_enable(void)
{
	return;
}

void icache_disable(void)
{
	return;
}

int icache_status(void)
{
	return 0;
}
#endif

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}
