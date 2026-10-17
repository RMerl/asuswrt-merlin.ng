// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2022 Google LLC
 */
#include <net.h>
#include <errno.h>
#include "mercury_aa1.h"

int misc_init_r(void)
{
	u8 mac[ARP_HLEN];
	int res;

	if (env_get("ethaddr"))
		return 0;

	res = mercury_aa1_read_mac(mac);
	if (res) {
		printf("couldn't read mac address: %s\n", errno_str(res));
		return 0;
	}

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("ethaddr", mac);

	return 0;
}
