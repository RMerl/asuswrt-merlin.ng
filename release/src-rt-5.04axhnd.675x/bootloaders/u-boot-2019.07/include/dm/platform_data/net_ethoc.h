/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Cadence Design Systems Inc.
 */

#ifndef _ETHOC_H
#define _ETHOC_H

#include <net.h>

#ifdef CONFIG_DM_ETH

struct ethoc_eth_pdata {
	struct eth_pdata eth_pdata;
	phys_addr_t packet_base;
};

#endif

#endif /* _ETHOC_H */
