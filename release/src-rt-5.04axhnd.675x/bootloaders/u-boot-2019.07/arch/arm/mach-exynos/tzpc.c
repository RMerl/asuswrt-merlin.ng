// SPDX-License-Identifier: GPL-2.0+
/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <asm/arch/tzpc.h>
#include <asm/io.h>

/* Setting TZPC[TrustZone Protection Controller] */
void tzpc_init(void)
{
	struct exynos_tzpc *tzpc;
	unsigned int addr, start = 0, end = 0;

	start = samsung_get_base_tzpc();

	if (cpu_is_exynos5())
		end = start + ((EXYNOS5_NR_TZPC_BANKS - 1) * TZPC_BASE_OFFSET);
	else if (cpu_is_exynos4())
		end = start + ((EXYNOS4_NR_TZPC_BANKS - 1) * TZPC_BASE_OFFSET);

	for (addr = start; addr <= end; addr += TZPC_BASE_OFFSET) {
		tzpc = (struct exynos_tzpc *)addr;

		if (addr == start)
			writel(R0SIZE, &tzpc->r0size);

		writel(DECPROTXSET, &tzpc->decprot0set);
		writel(DECPROTXSET, &tzpc->decprot1set);

		if (cpu_is_exynos5() && (addr == end))
			break;

		writel(DECPROTXSET, &tzpc->decprot2set);
		writel(DECPROTXSET, &tzpc->decprot3set);
	}
}
