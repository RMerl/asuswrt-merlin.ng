// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_FSL_DSP_CCSRBAR_PHYS, LAW_SIZE_1M,
		LAW_TRGT_IF_DSP_CCSR),
	SET_LAW(CONFIG_SYS_FSL_DSP_M2_RAM_ADDR, LAW_SIZE_16M,
		LAW_TRGT_IF_OCN_DSP),
};

int num_law_entries = ARRAY_SIZE(law_table);
