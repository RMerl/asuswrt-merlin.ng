/*
 * arch/arm/include/asm/outercache.h
 *
 * Copyright (C) 2010 ARM Ltd.
 * Written by Catalin Marinas <catalin.marinas@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ASM_OUTERCACHE_H
#define __ASM_OUTERCACHE_H

#include <linux/types.h>

struct l2x0_regs;

struct outer_cache_fns {
	void (*inv_range)(unsigned long, unsigned long);
	void (*clean_range)(unsigned long, unsigned long);
	void (*flush_range)(unsigned long, unsigned long);
	void (*flush_all)(void);
	void (*disable)(void);
#ifdef CONFIG_OUTER_CACHE_SYNC
	void (*sync)(void);
#endif
	void (*resume)(void);

	/* This is an ARM L2C thing */
	void (*write_sec)(unsigned long, unsigned);
	void (*configure)(const struct l2x0_regs *);
#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
	void (*spin_lock_irqsave)(void);
	void (*spin_unlock_irqrestore)(void);
	void (*sync_no_lock)(void);
	void (*flush_line_no_lock)(unsigned long);
	void (*inv_line_no_lock)(unsigned long);
#endif
};

extern struct outer_cache_fns outer_cache;

#ifdef CONFIG_OUTER_CACHE
/**
 * outer_inv_range - invalidate range of outer cache lines
 * @start: starting physical address, inclusive
 * @end: end physical address, exclusive
 */
static inline void outer_inv_range(phys_addr_t start, phys_addr_t end)
{
	if (outer_cache.inv_range)
		outer_cache.inv_range(start, end);
}

/**
 * outer_clean_range - clean dirty outer cache lines
 * @start: starting physical address, inclusive
 * @end: end physical address, exclusive
 */
static inline void outer_clean_range(phys_addr_t start, phys_addr_t end)
{
	if (outer_cache.clean_range)
		outer_cache.clean_range(start, end);
}

/**
 * outer_flush_range - clean and invalidate outer cache lines
 * @start: starting physical address, inclusive
 * @end: end physical address, exclusive
 */
static inline void outer_flush_range(phys_addr_t start, phys_addr_t end)
{
	if (outer_cache.flush_range)
		outer_cache.flush_range(start, end);
}

/**
 * outer_flush_all - clean and invalidate all cache lines in the outer cache
 *
 * Note: depending on implementation, this may not be atomic - it must
 * only be called with interrupts disabled and no other active outer
 * cache masters.
 *
 * It is intended that this function is only used by implementations
 * needing to override the outer_cache.disable() method due to security.
 * (Some implementations perform this as a clean followed by an invalidate.)
 */
static inline void outer_flush_all(void)
{
	if (outer_cache.flush_all)
		outer_cache.flush_all();
}

/**
 * outer_disable - clean, invalidate and disable the outer cache
 *
 * Disable the outer cache, ensuring that any data contained in the outer
 * cache is pushed out to lower levels of system memory.  The note and
 * conditions above concerning outer_flush_all() applies here.
 */
extern void outer_disable(void);

/**
 * outer_resume - restore the cache configuration and re-enable outer cache
 *
 * Restore any configuration that the cache had when previously enabled,
 * and re-enable the outer cache.
 */
static inline void outer_resume(void)
{
	if (outer_cache.resume)
		outer_cache.resume();
}

#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
static inline void outer_spin_lock_irqsave(void)
{
	outer_cache.spin_lock_irqsave();
}

static inline void outer_spin_unlock_irqrestore(void)
{
	outer_cache.spin_unlock_irqrestore();
}

static inline void outer_sync_no_lock(void)
{
	outer_cache.sync_no_lock();
}

static inline void outer_flush_line_no_lock(phys_addr_t addr)
{
	outer_cache.flush_line_no_lock(addr);
}

static inline void outer_inv_line_no_lock(phys_addr_t addr)
{
	outer_cache.inv_line_no_lock(addr);
}
#endif
#else

static inline void outer_inv_range(phys_addr_t start, phys_addr_t end)
{ }
static inline void outer_clean_range(phys_addr_t start, phys_addr_t end)
{ }
static inline void outer_flush_range(phys_addr_t start, phys_addr_t end)
{ }
static inline void outer_flush_all(void) { }
static inline void outer_disable(void) { }
static inline void outer_resume(void) { }

#endif

#if defined(CONFIG_BCM_KF_SPECTRE_PATCH) && defined(CONFIG_BCM_SPECTRE_PATCH_ENABLE)
#else
#ifdef CONFIG_OUTER_CACHE_SYNC
/**
 * outer_sync - perform a sync point for outer cache
 *
 * Ensure that all outer cache operations are complete and any store
 * buffers are drained.
 */
static inline void outer_sync(void)
{
	if (outer_cache.sync)
		outer_cache.sync();
}
#else
static inline void outer_sync(void)
{ }
#endif
#endif /* CONFIG_BCM_KF_SPECTRE_PATCH && CONFIG_BCM_SPECTRE_PATCH_ENABLE */

#endif	/* __ASM_OUTERCACHE_H */
