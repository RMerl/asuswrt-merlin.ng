// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/cache.h>

volatile int *cf_icache_status = (int *)ICACHE_STATUS;
volatile int *cf_dcache_status = (int *)DCACHE_STATUS;

void flush_cache(ulong start_addr, ulong size)
{
	/* Must be implemented for all M68k processors with copy-back data cache */
}

int icache_status(void)
{
	return *cf_icache_status;
}

int dcache_status(void)
{
	return *cf_dcache_status;
}

void icache_enable(void)
{
	icache_invalid();

	*cf_icache_status = 1;

#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr2"::"r"(CONFIG_SYS_CACHE_ACR2));
	__asm__ __volatile__("movec %0, %%acr3"::"r"(CONFIG_SYS_CACHE_ACR3));
#if defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr6"::"r"(CONFIG_SYS_CACHE_ACR6));
	__asm__ __volatile__("movec %0, %%acr7"::"r"(CONFIG_SYS_CACHE_ACR7));
#endif
#else
	__asm__ __volatile__("movec %0, %%acr0"::"r"(CONFIG_SYS_CACHE_ACR0));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(CONFIG_SYS_CACHE_ACR1));
#endif

	__asm__ __volatile__("movec %0, %%cacr"::"r"(CONFIG_SYS_CACHE_ICACR));
}

void icache_disable(void)
{
	u32 temp = 0;

	*cf_icache_status = 0;
	icache_invalid();

#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr2"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr3"::"r"(temp));
#if defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr6"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr7"::"r"(temp));
#endif
#else
	__asm__ __volatile__("movec %0, %%acr0"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(temp));
#endif
}

void icache_invalid(void)
{
	u32 temp;

	temp = CONFIG_SYS_ICACHE_INV;
	if (*cf_icache_status)
		temp |= CONFIG_SYS_CACHE_ICACR;

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));
}

/*
 * data cache only for ColdFire V4 such as MCF547x_8x, MCF5445x
 * the dcache will be dummy in ColdFire V2 and V3
 */
void dcache_enable(void)
{
	dcache_invalid();
	*cf_dcache_status = 1;

#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr0"::"r"(CONFIG_SYS_CACHE_ACR0));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(CONFIG_SYS_CACHE_ACR1));
#if defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr4"::"r"(CONFIG_SYS_CACHE_ACR4));
	__asm__ __volatile__("movec %0, %%acr5"::"r"(CONFIG_SYS_CACHE_ACR5));
#endif
#endif

	__asm__ __volatile__("movec %0, %%cacr"::"r"(CONFIG_SYS_CACHE_DCACR));
}

void dcache_disable(void)
{
	u32 temp = 0;

	*cf_dcache_status = 0;
	dcache_invalid();

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));

#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr0"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr1"::"r"(temp));
#if defined(CONFIG_CF_V4E)
	__asm__ __volatile__("movec %0, %%acr4"::"r"(temp));
	__asm__ __volatile__("movec %0, %%acr5"::"r"(temp));
#endif
#endif
}

void dcache_invalid(void)
{
#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
	u32 temp;

	temp = CONFIG_SYS_DCACHE_INV;
	if (*cf_dcache_status)
		temp |= CONFIG_SYS_CACHE_DCACR;
	if (*cf_icache_status)
		temp |= CONFIG_SYS_CACHE_ICACR;

	__asm__ __volatile__("movec %0, %%cacr"::"r"(temp));
#endif
}

__weak void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	/* An empty stub, real implementation should be in platform code */
}
__weak void flush_dcache_range(unsigned long start, unsigned long stop)
{
	/* An empty stub, real implementation should be in platform code */
}
