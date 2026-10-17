// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/armv7.h>
#include <asm/cache.h>
#include <asm/pl310.h>
#include <asm/io.h>
#include <asm/mach-imx/sys_proto.h>

void enable_ca7_smp(void)
{
	u32 val;

	/* Read MIDR */
	asm volatile ("mrc p15, 0, %0, c0, c0, 0\n\t" : "=r"(val));
	val = (val >> 4);
	val &= 0xf;

	/* Only set the SMP for Cortex A7 */
	if (val == 0x7) {
		/* Read auxiliary control register */
		asm volatile ("mrc p15, 0, %0, c1, c0, 1\n\t" : "=r"(val));

		if (val & (1 << 6))
			return;

		/* Enable SMP */
		val |= (1 << 6);

		/* Write auxiliary control register */
		asm volatile ("mcr p15, 0, %0, c1, c0, 1\n\t" : : "r"(val));

		DSB;
		ISB;
	}
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	enum dcache_option option = DCACHE_WRITETHROUGH;
#else
	enum dcache_option option = DCACHE_WRITEBACK;
#endif
	/* Avoid random hang when download by usb */
	invalidate_dcache_all();

	/* Set ACTLR.SMP bit for Cortex-A7 */
	enable_ca7_smp();

	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();

	/* Enable caching on OCRAM and ROM */
	mmu_set_region_dcache_behaviour(ROMCP_ARB_BASE_ADDR,
					ROMCP_ARB_END_ADDR,
					option);
	mmu_set_region_dcache_behaviour(IRAM_BASE_ADDR,
					IRAM_SIZE,
					option);
}
#else
void enable_caches(void)
{
	/*
	 * Set ACTLR.SMP bit for Cortex-A7, even if the caches are
	 * disabled by u-boot
	 */
	enable_ca7_smp();

	puts("WARNING: Caches not enabled\n");
}
#endif

#ifndef CONFIG_SYS_L2CACHE_OFF
#ifdef CONFIG_SYS_L2_PL310
#define IOMUXC_GPR11_L2CACHE_AS_OCRAM 0x00000002
void v7_outer_cache_enable(void)
{
	struct pl310_regs *const pl310 = (struct pl310_regs *)L2_PL310_BASE;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	unsigned int val, cache_id;


	/*
	 * Must disable the L2 before changing the latency parameters
	 * and auxiliary control register.
	 */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	/*
	 * Set bit 22 in the auxiliary control register. If this bit
	 * is cleared, PL310 treats Normal Shared Non-cacheable
	 * accesses as Cacheable no-allocate.
	 */
	setbits_le32(&pl310->pl310_aux_ctrl, L310_SHARED_ATT_OVERRIDE_ENABLE);

	if (is_mx6sl() || is_mx6sll()) {
		val = readl(&iomux->gpr[11]);
		if (val & IOMUXC_GPR11_L2CACHE_AS_OCRAM) {
			/* L2 cache configured as OCRAM, reset it */
			val &= ~IOMUXC_GPR11_L2CACHE_AS_OCRAM;
			writel(val, &iomux->gpr[11]);
		}
	}

	writel(0x132, &pl310->pl310_tag_latency_ctrl);
	writel(0x132, &pl310->pl310_data_latency_ctrl);

	val = readl(&pl310->pl310_prefetch_ctrl);

	/* Turn on the L2 I/D prefetch, double linefill */
	/* Set prefetch offset with any value except 23 as per errata 765569 */
	val |= 0x7000000f;

	/*
	 * The L2 cache controller(PL310) version on the i.MX6D/Q is r3p1-50rel0
	 * The L2 cache controller(PL310) version on the i.MX6DL/SOLO/SL/SX/DQP
	 * is r3p2.
	 * But according to ARM PL310 errata: 752271
	 * ID: 752271: Double linefill feature can cause data corruption
	 * Fault Status: Present in: r3p0, r3p1, r3p1-50rel0. Fixed in r3p2
	 * Workaround: The only workaround to this erratum is to disable the
	 * double linefill feature. This is the default behavior.
	 */
	cache_id = readl(&pl310->pl310_cache_id);
	if (((cache_id & L2X0_CACHE_ID_PART_MASK) == L2X0_CACHE_ID_PART_L310)
	    && ((cache_id & L2X0_CACHE_ID_RTL_MASK) < L2X0_CACHE_ID_RTL_R3P2))
		val &= ~(1 << 30);
	writel(val, &pl310->pl310_prefetch_ctrl);

	val = readl(&pl310->pl310_power_ctrl);
	val |= L2X0_DYNAMIC_CLK_GATING_EN;
	val |= L2X0_STNDBY_MODE_EN;
	writel(val, &pl310->pl310_power_ctrl);

	setbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void v7_outer_cache_disable(void)
{
	struct pl310_regs *const pl310 = (struct pl310_regs *)L2_PL310_BASE;

	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}
#endif /* !CONFIG_SYS_L2_PL310 */
#endif /* !CONFIG_SYS_L2CACHE_OFF */
