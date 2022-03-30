// SPDX-License-Identifier: GPL-2.0+
/*
 * Cortex-R Memory Protection Unit specific code
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <command.h>
#include <asm/armv7.h>
#include <asm/system.h>
#include <asm/barriers.h>
#include <linux/compiler.h>

#include <asm/armv7_mpu.h>

/* MPU Type register definitions */
#define MPUIR_S_SHIFT		0
#define MPUIR_S_MASK		BIT(MPUIR_S_SHIFT)
#define MPUIR_DREGION_SHIFT	8
#define MPUIR_DREGION_MASK	(0xff << 8)

/**
 * Note:
 * The Memory Protection Unit(MPU) allows to partition memory into regions
 * and set individual protection attributes for each region. In absence
 * of MPU a default map[1] will take effect. make sure to run this code
 * from a region which has execution permissions by default.
 * [1] http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0460d/I1002400.html
 */

void disable_mpu(void)
{
	u32 reg;

	reg = get_cr();
	reg &= ~CR_M;
	dsb();
	set_cr(reg);
	isb();
}

void enable_mpu(void)
{
	u32 reg;

	reg = get_cr();
	reg |= CR_M;
	dsb();
	set_cr(reg);
	isb();
}

int mpu_enabled(void)
{
	return get_cr() & CR_M;
}

void mpu_config(struct mpu_region_config *rgn)
{
	u32 attr, val;

	attr = get_attr_encoding(rgn->mr_attr);

	/* MPU Region Number Register */
	asm volatile ("mcr p15, 0, %0, c6, c2, 0" : : "r" (rgn->region_no));

	/* MPU Region Base Address Register */
	asm volatile ("mcr p15, 0, %0, c6, c1, 0" : : "r" (rgn->start_addr));

	/* MPU Region Size and Enable Register */
	if (rgn->reg_size)
		val = (rgn->reg_size << REGION_SIZE_SHIFT) | ENABLE_REGION;
	else
		val = DISABLE_REGION;
	asm volatile ("mcr p15, 0, %0, c6, c1, 2" : : "r" (val));

	/* MPU Region Access Control Register */
	val = rgn->xn << XN_SHIFT | rgn->ap << AP_SHIFT | attr;
	asm volatile ("mcr p15, 0, %0, c6, c1, 4" : : "r" (val));
}

void setup_mpu_regions(struct mpu_region_config *rgns, u32 num_rgns)
{
	u32 num, i;

	asm volatile ("mrc p15, 0, %0, c0, c0, 4" : "=r" (num));
	num = (num & MPUIR_DREGION_MASK) >> MPUIR_DREGION_SHIFT;
	/* Regions to be configured cannot be greater than available regions */
	if (num < num_rgns)
		num_rgns = num;
	/**
	 * Assuming dcache might not be enabled at this point, disabling
	 * and invalidating only icache.
	 */
	icache_disable();
	invalidate_icache_all();

	disable_mpu();

	for (i = 0; i < num_rgns; i++)
		mpu_config(&rgns[i]);

	enable_mpu();

	icache_enable();
}

void enable_caches(void)
{
	/*
	 * setup_mpu_regions() might have enabled Icache. So add a check
	 * before enabling Icache
	 */
	if (!icache_status())
		icache_enable();
	dcache_enable();
}
