// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010,2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>

/*
 * This is called right before passing control to
 * the Linux kernel point.
 */
int cleanup_before_linux(void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * disable interrupt and turn off caches etc ...
	 */
	disable_interrupts();
	/*
	 * turn off D-cache
	 * dcache_disable() in turn flushes the d-cache
	 * MPU is still enabled & can't be disabled as the u-boot
	 * code might be running in sdram which by default is not
	 * executable area.
	 */
	dcache_disable();
	/* invalidate to make sure no cache line gets dirty between
	 * dcache flushing and disabling dcache */
	invalidate_dcache_all();

	icache_disable();
	invalidate_icache_all();

	return 0;
}

/*
 * Perform the low-level reset.
 */
void reset_cpu(ulong addr)
{
	/*
	 * Perform reset but keep priority group unchanged.
	 */
	writel((V7M_AIRCR_VECTKEY << V7M_AIRCR_VECTKEY_SHIFT)
		| (V7M_SCB->aircr & V7M_AIRCR_PRIGROUP_MSK)
		| V7M_AIRCR_SYSRESET, &V7M_SCB->aircr);
}
