/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_LS1_SERDES_H
#define __FSL_LS1_SERDES_H

int is_serdes_prtcl_valid(int serdes, u32 prtcl);
int serdes_lane_enabled(int lane);
#endif /* __FSL_LS1_SERDES_H */
