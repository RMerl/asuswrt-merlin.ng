// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Pierrick Hascoet, Abilis Systems
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>

void reset_cpu(ulong addr)
{
#define CRM_SWRESET	0xff101044
	writel(0x1, (void *)CRM_SWRESET);
}

int board_eth_init(bd_t *bis)
{
	if (designware_initialize(ETH0_BASE_ADDRESS, 0) >= 0)
		return 1;

	return 0;
}
