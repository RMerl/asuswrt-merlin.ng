/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR4_MPR_PDA_IF_H
#define _MV_DDR4_MPR_PDA_IF_H

#include "ddr3_init.h"
#include "mv_ddr_common.h"

#define MV_DDR4_VREF_STEP_SIZE	3
#define MV_DDR4_VREF_MIN_RANGE	1
#define MV_DDR4_VREF_MAX_RANGE	73
#define MV_DDR4_VREF_MAX_COUNT	(((MV_DDR4_VREF_MAX_RANGE - MV_DDR4_VREF_MIN_RANGE) / MV_DDR4_VREF_STEP_SIZE) + 2)

#define MV_DDR4_MPR_READ_PATTERN_NUM	3

enum mv_ddr4_mpr_read_format {
	MV_DDR4_MPR_READ_SERIAL,
	MV_DDR4_MPR_READ_PARALLEL,
	MV_DDR4_MPR_READ_STAGGERED,
	MV_DDR4_MPR_READ_RSVD_TEMP
};

enum mv_ddr4_mpr_read_type {
	MV_DDR4_MPR_READ_RAW,
	MV_DDR4_MPR_READ_DECODED
};

enum mv_ddr4_vref_tap_state {
	MV_DDR4_VREF_TAP_START,
	MV_DDR4_VREF_TAP_BUSY,
	MV_DDR4_VREF_TAP_FLIP,
	MV_DDR4_VREF_TAP_END
};

int mv_ddr4_mode_regs_init(u8 dev_num);
int mv_ddr4_mpr_read(u8 dev_num, u32 mpr_num, u32 page_num,
		     enum mv_ddr4_mpr_read_format read_format,
		     enum mv_ddr4_mpr_read_type read_type,
		     u32 *data);
int mv_ddr4_mpr_write(u8 dev_num, u32 mpr_location, u32 mpr_num,
		      u32 page_num, u32 data);
int mv_ddr4_dq_pins_mapping(u8 dev_num);
int mv_ddr4_vref_training_mode_ctrl(u8 dev_num, u8 if_id,
				 enum hws_access_type access_type,
				 int enable);
int mv_ddr4_vref_tap_set(u8 dev_num, u8 if_id,
			 enum hws_access_type access_type,
			 u32 taps_num,
			 enum mv_ddr4_vref_tap_state state);
int mv_ddr4_vref_set(u8 dev_num, u8 if_id, enum hws_access_type access_type,
		     u32 range, u32 vdq_tv, u8 vdq_training_ena);
int mv_ddr4_pda_pattern_odpg_load(u32 dev_num, enum hws_access_type access_type,
				  u32 if_id, u32 subphy_mask, u32 cs_num);
int mv_ddr4_pda_ctrl(u8 dev_num, u8 if_id, u8 cs_num, int enable);

#endif /* _MV_DDR4_MPR_PDA_IF_H */
