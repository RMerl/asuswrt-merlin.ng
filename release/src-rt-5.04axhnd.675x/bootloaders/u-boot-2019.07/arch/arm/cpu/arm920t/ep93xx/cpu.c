// SPDX-License-Identifier: GPL-2.0+
/*
 * Cirrus Logic EP93xx CPU-specific support.
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 */

#include <common.h>
#include <asm/arch/ep93xx.h>
#include <asm/io.h>

/* We reset the CPU by generating a 1-->0 transition on DeviceCfg bit 31. */
extern void reset_cpu(ulong addr)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	uint32_t value;

	/* Unlock DeviceCfg and set SWRST */
	writel(0xAA, &syscon->sysswlock);
	value = readl(&syscon->devicecfg);
	value |= SYSCON_DEVICECFG_SWRST;
	writel(value, &syscon->devicecfg);

	/* Unlock DeviceCfg and clear SWRST */
	writel(0xAA, &syscon->sysswlock);
	value = readl(&syscon->devicecfg);
	value &= ~SYSCON_DEVICECFG_SWRST;
	writel(value, &syscon->devicecfg);

	/* Dying... */
	while (1)
		; /* noop */
}
