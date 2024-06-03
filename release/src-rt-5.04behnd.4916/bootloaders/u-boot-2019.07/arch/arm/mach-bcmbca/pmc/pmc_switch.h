// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 
*/

#ifndef PMC_SWITCH_H
#define PMC_SWITCH_H

int pmc_switch_power_up(void);
int pmc_switch_power_down(void);
void pmc_switch_clock_lowpower_mode(int low_power);
int pmc_switch_enable_rgmii_zone_clk(int z1_clk_enable, int z2_clk_enable);

#endif //#ifndef PMC_SWITCH_H
