// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm/csr.h>
#include <asm/asm.h>
#include <cache.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <asm/arch-andes/csr.h>

#ifdef CONFIG_V5L2_CACHE
void enable_caches(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CACHE,
					  DM_DRIVER_GET(v5l2_cache),
					  &dev);
	if (ret) {
		log_debug("Cannot enable v5l2 cache\n");
	} else {
		ret = cache_enable(dev);
		if (ret)
			log_debug("v5l2 cache enable failed\n");
	}
}

static void cache_ops(int (*ops)(struct udevice *dev))
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		ops(dev);
}
#endif

void flush_dcache_all(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csr_write(CSR_MCCTLCOMMAND, CCTL_L1D_WBINVAL_ALL);
#endif
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void icache_enable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile("csrsi %0, 0x1" :: "i"(CSR_MCACHE_CTL));
#endif
}

void icache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile("csrci %0, 0x1" :: "i"(CSR_MCACHE_CTL));
#endif
}

void dcache_enable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile("csrsi %0, 0x2" :: "i"(CSR_MCACHE_CTL));
#endif

#ifdef CONFIG_V5L2_CACHE
	cache_ops(cache_enable);
#endif
}

void dcache_disable(void)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile("csrci %0, 0x2" :: "i"(CSR_MCACHE_CTL));
#endif

#ifdef CONFIG_V5L2_CACHE
	cache_ops(cache_disable);
#endif
}

int icache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, %1\n\t"
		"andi %0, t1, 0x01\n\t"
		: "=r" (ret)
		: "i"(CSR_MCACHE_CTL)
		: "memory"
	);
#endif

	return !!ret;
}

int dcache_status(void)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, %1\n\t"
		"andi %0, t1, 0x02\n\t"
		: "=r" (ret)
		: "i" (CSR_MCACHE_CTL)
		: "memory"
	);
#endif

	return !!ret;
}
