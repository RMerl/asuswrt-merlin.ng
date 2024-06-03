/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>

#define ETH_LEN 6
#define MAC_VID 1

enum mscc_regs_ana_table {
	MSCC_ANA_TABLES_MACHDATA,
	MSCC_ANA_TABLES_MACLDATA,
	MSCC_ANA_TABLES_MACACCESS,
};

int mscc_mac_table_add(void __iomem *regs,
		       const unsigned long *mscc_mac_table_offset,
		       const unsigned char mac[ETH_LEN], int pgid);
