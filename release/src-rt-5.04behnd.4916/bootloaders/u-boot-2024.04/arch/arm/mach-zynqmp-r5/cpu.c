// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Xilinx, Inc. (Michal Simek)
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/armv7_mpu.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

struct mpu_region_config region_config[] = {
	{ 0x00000000, REGION_0, XN_EN, PRIV_RW_USR_RW,
	  SHARED_WRITE_BUFFERED, REGION_4GB },
	{ 0x00000000, REGION_1, XN_DIS, PRIV_RW_USR_RW,
	  O_I_WB_RD_WR_ALLOC, REGION_1GB },
};

int arch_cpu_init(void)
{
	gd->cpu_clk = CONFIG_CPU_FREQ_HZ;

	setup_mpu_regions(region_config, ARRAY_SIZE(region_config));

	return 0;
}

/*
 * Perform the low-level reset.
 */
void reset_cpu(void)
{
	while (1)
		;
}
