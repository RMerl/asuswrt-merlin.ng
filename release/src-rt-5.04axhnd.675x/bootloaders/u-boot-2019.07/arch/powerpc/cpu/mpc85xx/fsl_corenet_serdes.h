/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 *
 * Author: Roy Zang <tie-fei.zang@freescale.com>
 */

#ifndef __FSL_CORENET_SERDES_H
#define __FSL_CORENET_SERDES_H

enum srds_bank {
	FSL_SRDS_BANK_1  = 0,
	FSL_SRDS_BANK_2  = 1,
	FSL_SRDS_BANK_3  = 2,
};

int is_serdes_prtcl_valid(u32 prtcl);
int serdes_get_lane_idx(int lane);
int serdes_get_bank_by_lane(int lane);
int serdes_lane_enabled(int lane);
enum srds_prtcl serdes_get_prtcl(int cfg, int lane);

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
extern uint16_t srds_lpd_b[SRDS_MAX_BANK];
#endif

#endif /* __FSL_CORENET_SERDES_H */
