// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

#ifdef CONFIG_TARGET_ESPRESSO7420
/*
 * Exynos7420 uses CPU0 of Cluster-1 as boot CPU. Due to this, branch_if_master
 * fails to identify as the boot CPU as the master CPU. As temporary workaround,
 * setup the slave CPU boot address as "_main".
 */
extern void _main(void);
void *secondary_boot_addr = (void *)_main;
#endif /* CONFIG_TARGET_ESPRESSO7420 */

void reset_cpu(ulong addr)
{
#ifdef CONFIG_CPU_V7A
	writel(0x1, samsung_get_base_swreset());
#endif
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
