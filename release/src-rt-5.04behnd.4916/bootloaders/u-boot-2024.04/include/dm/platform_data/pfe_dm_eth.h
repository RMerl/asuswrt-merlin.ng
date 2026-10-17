/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __PFE_DM_ETH_H__
#define __PFE_DM_ETH_H__
#include <net.h>

struct pfe_ddr_address {
	void *ddr_pfe_baseaddr;
	unsigned long ddr_pfe_phys_baseaddr;
};

struct pfe_eth_pdata {
	struct eth_pdata pfe_eth_pdata_mac;
	struct pfe_ddr_address pfe_ddr_addr;
};
#endif /* __PFE_DM_ETH_H__ */
