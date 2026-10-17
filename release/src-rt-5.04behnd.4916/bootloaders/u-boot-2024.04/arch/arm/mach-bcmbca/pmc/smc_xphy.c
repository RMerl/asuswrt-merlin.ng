// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

#include <linux/types.h>
#include <string.h>
#include <stdio.h>
#include "smc_xphy.h"
#include "power_rpc_svc.h"

static char *xphy_domain_name[] = {
	"xp0_1g", "xp0_2p5",
	"xp1_1g", "xp1_2p5",
	"xp2_1g", "xp2_2p5",
	"xp3_1g", "xp3_2p5",
	"xphy1g", "xphy10g"
};

int pmc_xphy_power_up(xphy_port_e xphy_num, xphy_speed_e xphy_speed)
{
	int ret = 0;

	if (xphy_num > XPHY_PORT4 || xphy_num < XPHY_PORT0)
		return -1;

	if (xphy_speed > XPHY_HIGH_SPEED || xphy_speed < XPHY_LOW_SPEED)
		return -1;

	ret = bcm_rpc_pwr_set_domain_state(xphy_domain_name[xphy_num * 2 + XPHY_HIGH_SPEED],
			PWR_DOMAIN_NAME_MAX_LEN,
			PWR_DOM_STATE_ON,
			PWR_DOM_RESET_DEASSERT);

	return ret;
}

int pmc_xphy_power_down(xphy_port_e xphy_num, xphy_speed_e xphy_speed)
{
	int ret = 0;

	if (xphy_num > XPHY_PORT4 || xphy_num < XPHY_PORT0)
		return -1;

	if (xphy_speed > XPHY_HIGH_SPEED || xphy_speed < XPHY_LOW_SPEED)
		return -1;

	ret = bcm_rpc_pwr_set_domain_state(xphy_domain_name[xphy_num * 2 + XPHY_HIGH_SPEED],
		PWR_DOMAIN_NAME_MAX_LEN,
		PWR_DOM_STATE_OFF,
		PWR_DOM_RESET_ASSERT);
	if (ret)
		return -1;

	ret = bcm_rpc_pwr_set_domain_state(xphy_domain_name[xphy_num * 2],
			PWR_DOMAIN_NAME_MAX_LEN,
			PWR_DOM_STATE_OFF,
			PWR_DOM_RESET_ASSERT);

	return ret;
}
