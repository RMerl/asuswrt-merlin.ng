// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <altera.h>
#include <common.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <mach/clock_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * FPGA programming support for SoC FPGA Stratix 10
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Intel_FPGA_SDM_Mailbox,
		/* Interface type */
		secure_device_manager_mailbox,
		/* No limitation as additional data will be ignored */
		-1,
		/* No device function table */
		NULL,
		/* Base interface address specified in driver */
		NULL,
		/* No cookie implementation */
		0
	},
};


/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	puts("CPU:   Intel FPGA SoCFPGA Platform (ARMv8 64bit Cortex-A53)\n");

	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	char qspi_string[13];

	sprintf(qspi_string, "<0x%08x>", cm_get_qspi_controller_clk_hz());
	env_set("qspi_clock", qspi_string);

	return 0;
}
#endif

int arch_early_init_r(void)
{
	socfpga_fpga_add(&altera_fpga[0]);

	return 0;
}

/* Return 1 if FPGA is ready otherwise return 0 */
int is_fpga_config_ready(void)
{
	return (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_FPGA_CONFIG) &
		SYSMGR_FPGACONFIG_READY_MASK) == SYSMGR_FPGACONFIG_READY_MASK;
}

void do_bridge_reset(int enable, unsigned int mask)
{
	/* Check FPGA status before bridge enable */
	if (!is_fpga_config_ready()) {
		puts("FPGA not ready. Bridge reset aborted!\n");
		return;
	}

	socfpga_bridges_reset(enable);
}
