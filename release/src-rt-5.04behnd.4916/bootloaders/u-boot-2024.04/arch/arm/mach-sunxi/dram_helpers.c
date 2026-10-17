// SPDX-License-Identifier: GPL-2.0+
/*
 * DRAM init helper functions
 *
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 */

#include <config.h>
#include <time.h>
#include <vsprintf.h>
#include <asm/barriers.h>
#include <asm/io.h>
#include <asm/arch/dram.h>

/*
 * Wait up to 1s for value to be set in given part of reg.
 */
void mctl_await_completion(u32 *reg, u32 mask, u32 val)
{
	unsigned long tmo = timer_get_us() + 1000000;

	while ((readl(reg) & mask) != val) {
		if (timer_get_us() > tmo)
			panic("Timeout initialising DRAM\n");
	}
}

/*
 * Test if memory at offset matches memory at a certain base
 *
 * Note: dsb() is not available on ARMv5 in Thumb mode
 */
#ifndef CONFIG_MACH_SUNIV
bool mctl_mem_matches_base(u32 offset, ulong base)
{
	u32 val_base;
	u32 val_offset;
	bool ret;

	/* Save original values */
	val_base = readl(base);
	val_offset = readl(base + offset);

	/* Try to write different values to RAM at two addresses */
	writel(0, base);
	writel(0xaa55aa55, base + offset);
	dsb();
	/* Check if the same value is actually observed when reading back */
	ret = readl(base) == readl(base + offset);

	/* Restore original values */
	writel(val_base, base);
	writel(val_offset, base + offset);
	return ret;
}

/*
 * Test if memory at offset matches memory at begin of DRAM
 */
bool mctl_mem_matches(u32 offset)
{
	return mctl_mem_matches_base(offset, CFG_SYS_SDRAM_BASE);
}
#endif
