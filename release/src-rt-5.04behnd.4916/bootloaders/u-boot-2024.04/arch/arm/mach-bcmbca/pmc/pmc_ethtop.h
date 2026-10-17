// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _PMC_ETHTOP_H
#define _PMC_ETHTOP_H

#if defined(CONFIG_BCMBCA_PMC_ETHTOP) && defined(CONFIG_SMC_BASED)
typedef enum {
	ETHTOP_COMMON,
	ETHTOP_XPORT0,
	ETHTOP_XPORT1,
#if defined(CONFIG_BCM68880) || defined (CONFIG_BCM6837)
	ETHTOP_XPORT2,
#endif
	ETHTOP_LAST, //Must be last
} eth_block_t;

int pmc_ethtop_power_up(eth_block_t block_id);
int pmc_ethtop_power_down(eth_block_t block_id);
#else
#define pmc_ethtop_power_up(x) (0)
#define pmc_ethtop_power_down(x) (0)
#endif

#endif
