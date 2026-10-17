// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 Adaptive Voltage Scaling driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *      Tero Kristo <t-kristo@ti.com>
 *
 */

#ifndef _K3_AVS0_
#define _K3_AVS0_

#define AM6_VDD_WKUP		0
#define AM6_VDD_MCU		1
#define AM6_VDD_CORE		2
#define AM6_VDD_MPU0		3
#define AM6_VDD_MPU1		4

#define J721E_VDD_MPU		2

#define NUM_OPPS		4

#define AM6_OPP_NOM		1
#define AM6_OPP_OD		2
#define AM6_OPP_TURBO		3

int k3_avs_set_opp(struct udevice *dev, int vdd_id, int opp_id);
int k3_avs_notify_freq(int dev_id, int clk_id, u32 freq);

#endif
