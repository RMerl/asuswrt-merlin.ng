// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

#ifndef _SMC_XPHY_H
#define _SMC_XPHY_H

typedef enum {
	XPHY_LOW_SPEED,
	XPHY_HIGH_SPEED
} xphy_speed_e;

typedef enum {
	XPHY_PORT0,
	XPHY_PORT1,
	XPHY_PORT2,
	XPHY_PORT3,
	XPHY_PORT4,
	XPHY_MAX_NUM
} xphy_port_e;

int pmc_xphy_power_up(xphy_port_e xphy_num, xphy_speed_e  xphy_speed);
int pmc_xphy_power_down(xphy_port_e xphy_num, xphy_speed_e  xphy_speed);

#endif
