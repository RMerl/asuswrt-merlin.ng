// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE_PHYS, LAW_SIZE_64M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_SSD_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_LBC)
};

int num_law_entries = ARRAY_SIZE(law_table);
