// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long zynqmp_get_system_timer_freq(void)
{
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_QEMU:
		return 50000000;
	}

	return 100000000;
}

#ifdef CONFIG_CLOCKS
/**
 * set_cpu_clk_info() - Initialize clock framework
 * Always returns zero.
 *
 * This function is called from common code after relocation and sets up the
 * clock framework. The framework must not be used before this function had been
 * called.
 */
int set_cpu_clk_info(void)
{
	gd->cpu_clk = get_tbclk();

	gd->bd->bi_arm_freq = gd->cpu_clk / 1000000;

	gd->bd->bi_dsp_freq = 0;

	return 0;
}
#endif
