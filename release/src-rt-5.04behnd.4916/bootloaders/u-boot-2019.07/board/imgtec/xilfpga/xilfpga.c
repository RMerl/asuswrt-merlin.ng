// SPDX-License-Identifier: GPL-2.0+
/*
 * Imagination Technologies MIPSfpga platform code
 *
 * Copyright (C) 2016, Imagination Technologies Ltd.
 *
 * Zubair Lutfullah Kakakhel <Zubair.Kakakhel@imgtec.com>
 *
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

/* initialize the DDR Controller and PHY */
int dram_init(void)
{
	/* MIG IP block is smart and doesn't need SW
	 * to do any init */
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;	/* in bytes */

	return 0;
}
