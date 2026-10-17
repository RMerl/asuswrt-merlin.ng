// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Pierrick Hascoet, Abilis Systems
 */

#include <cpu_func.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>

void reset_cpu(void)
{
#define CRM_SWRESET	0xff101044
	writel(0x1, (void *)CRM_SWRESET);
}

/*
 * Ethernet configuration
 */
#define ETH0_BASE_ADDRESS		0xFE100000
int board_eth_init(struct bd_info *bis)
{
	if (designware_initialize(ETH0_BASE_ADDRESS, 0) >= 0)
		return 1;

	return 0;
}
