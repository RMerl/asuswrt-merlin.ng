// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/clock_manager.h>

/* Board-specific header. */
#include <qts/iocsr_config.h>

int iocsr_get_config_table(const unsigned int chain_id,
			   const unsigned long **table,
			   unsigned int *table_len)
{
	switch (chain_id) {
	case 0:
		*table = iocsr_scan_chain0_table;
		*table_len = CFG_HPS_IOCSR_SCANCHAIN0_LENGTH;
		break;
	case 1:
		*table = iocsr_scan_chain1_table;
		*table_len = CFG_HPS_IOCSR_SCANCHAIN1_LENGTH;
		break;
	case 2:
		*table = iocsr_scan_chain2_table;
		*table_len = CFG_HPS_IOCSR_SCANCHAIN2_LENGTH;
		break;
	case 3:
		*table = iocsr_scan_chain3_table;
		*table_len = CFG_HPS_IOCSR_SCANCHAIN3_LENGTH;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
