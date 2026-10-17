/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Keystone2: DDR3 configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __DDR3_CFG_H
#define __DDR3_CFG_H

#include <asm/arch/ddr3.h>

extern struct ddr3_phy_config ddr3phy_1600_2g;
extern struct ddr3_emif_config ddr3_1600_2g;

int ddr3_get_dimm_params_from_spd(struct ddr3_spd_cb *spd_cb);

#endif /* __DDR3_CFG_H */
