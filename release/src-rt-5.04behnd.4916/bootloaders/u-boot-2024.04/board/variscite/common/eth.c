// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Variscite Ltd.
 */
#include <net.h>
#include <miiphy.h>
#include <env.h>
#include "../common/imx9_eeprom.h"

#define CHAR_BIT 8

static u64 mac2int(const u8 hwaddr[])
{
	u8 i;
	u64 ret = 0;
	const u8 *p = hwaddr;

	for (i = 6; i > 0; i--)
		ret |= (u64)*p++ << (CHAR_BIT * (i - 1));

	return ret;
}

static void int2mac(const u64 mac, u8 *hwaddr)
{
	u8 i;
	u8 *p = hwaddr;

	for (i = 6; i > 0; i--)
		*p++ = mac >> (CHAR_BIT * (i - 1));
}

int var_setup_mac(struct var_eeprom *eeprom)
{
	int ret;
	unsigned char enetaddr[6];
	u64 addr;
	unsigned char enet1addr[6];

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)
		return 0;

	ret = var_eeprom_get_mac(eeprom, enetaddr);
	if (ret)
		return ret;

	if (!is_valid_ethaddr(enetaddr))
		return -EINVAL;

	eth_env_set_enetaddr("ethaddr", enetaddr);

	addr = mac2int(enetaddr);
	int2mac(addr + 1, enet1addr);
	eth_env_set_enetaddr("eth1addr", enet1addr);

	return 0;
}
