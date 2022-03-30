/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SERDES_H
#define __FSL_SERDES_H

#include <config.h>

enum srds_prtcl {
	/*
	 * Nobody will check whether the device 'NONE' has been configured,
	 * So use it to indicate if the serdes_prtcl_map has been initialized.
	 */
	NONE = 0,
	PCIE1,
	PCIE2,
	SATA1,
	SGMII_TSEC1,
	SGMII_TSEC2,
};

enum srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1,
};

int is_serdes_configured(enum srds_prtcl device);
void fsl_serdes_init(void);
const char *serdes_clock_to_string(u32 clock);

int serdes_get_first_lane(u32 sd, enum srds_prtcl device);
enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane);

#endif /* __FSL_SERDES_H */
