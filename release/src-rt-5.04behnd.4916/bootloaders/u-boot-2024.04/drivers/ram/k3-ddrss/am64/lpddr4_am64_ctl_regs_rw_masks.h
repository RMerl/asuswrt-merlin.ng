/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_RW_MASKS_H_
#define LPDDR4_RW_MASKS_H_

#include <stdint.h>

extern u32 g_lpddr4_ddr_controller_rw_mask[423];
extern u32 g_lpddr4_pi_rw_mask[345];
extern u32 g_lpddr4_data_slice_0_rw_mask[126];
extern u32 g_lpddr4_data_slice_1_rw_mask[126];
extern u32 g_lpddr4_address_slice_0_rw_mask[43];
extern u32 g_lpddr4_address_slice_1_rw_mask[43];
extern u32 g_lpddr4_address_slice_2_rw_mask[43];
extern u32 g_lpddr4_phy_core_rw_mask[126];

#endif /* LPDDR4_RW_MASKS_H_ */
