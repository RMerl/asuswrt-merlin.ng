// SPDX-License-Identifier: GPL-2.0+
/*
 * Bubblegum-96 Boards Support
 *
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <linux/arm-smccc.h>
#include <linux/psci.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/psci.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * dram_init - sets uboots idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

/* This is called after dram_init() so use get_ram_size result */
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

static void show_psci_version(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	printf("PSCI:  v%ld.%ld\n",
	       PSCI_VERSION_MAJOR(res.a0),
		PSCI_VERSION_MINOR(res.a0));
}

int board_init(void)
{
	show_psci_version();

	return 0;
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}
