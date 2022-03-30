// SPDX-License-Identifier: GPL-2.0+
/*
 * DRAM init helper functions
 *
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 */

#include <common.h>
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
 * Test if memory at offset offset matches memory at begin of DRAM
 */
bool mctl_mem_matches(u32 offset)
{
	/* Try to write different values to RAM at two addresses */
	writel(0, CONFIG_SYS_SDRAM_BASE);
	writel(0xaa55aa55, (ulong)CONFIG_SYS_SDRAM_BASE + offset);
	dsb();
	/* Check if the same value is actually observed when reading back */
	return readl(CONFIG_SYS_SDRAM_BASE) ==
	       readl((ulong)CONFIG_SYS_SDRAM_BASE + offset);
}
