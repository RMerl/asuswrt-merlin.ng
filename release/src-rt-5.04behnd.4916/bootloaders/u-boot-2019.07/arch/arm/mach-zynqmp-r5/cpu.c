// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Xilinx, Inc. (Michal Simek)
 */

#include <common.h>
#include <asm/armv7_mpu.h>

DECLARE_GLOBAL_DATA_PTR;

struct mpu_region_config region_config[] = {
	{ 0x00000000, REGION_0, XN_DIS, PRIV_RW_USR_RW,
	  O_I_WB_RD_WR_ALLOC, REGION_1GB },
	{ 0x20000000, REGION_1, XN_EN, PRIV_RO_USR_RO,
	  O_I_WB_RD_WR_ALLOC, REGION_512MB },
	{ 0x40000000, REGION_2, XN_EN, PRIV_RO_USR_RO,
	  O_I_WB_RD_WR_ALLOC, REGION_1GB },
};

int arch_cpu_init(void)
{
	gd->cpu_clk = CONFIG_CPU_FREQ_HZ;

	setup_mpu_regions(region_config, sizeof(region_config) /
			  sizeof(struct mpu_region_config));

	return 0;
}

/*
 * Perform the low-level reset.
 */
void reset_cpu(ulong addr)
{
	while (1)
		;
}
