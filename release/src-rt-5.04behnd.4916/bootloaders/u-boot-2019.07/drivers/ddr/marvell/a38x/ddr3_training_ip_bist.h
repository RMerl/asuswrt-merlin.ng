/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_BIST_H_
#define _DDR3_TRAINING_IP_BIST_H_

#include "ddr3_training_ip.h"

enum hws_bist_operation {
	BIST_STOP = 0,
	BIST_START = 1
};

enum  hws_stress_jump {
	STRESS_NONE = 0,
	STRESS_ENABLE = 1
};

enum hws_pattern_duration {
	DURATION_SINGLE = 0,
	DURATION_STOP_AT_FAIL = 1,
	DURATION_ADDRESS = 2,
	DURATION_CONT = 4
};

struct bist_result {
	u32 bist_error_cnt;
	u32 bist_fail_low;
	u32 bist_fail_high;
	u32 bist_last_fail_addr;
};

int ddr3_tip_bist_read_result(u32 dev_num, u32 if_id,
			      struct bist_result *pst_bist_result);
int ddr3_tip_bist_activate(u32 dev_num, enum hws_pattern pattern,
			   enum hws_access_type access_type,
			   u32 if_num, enum hws_dir direction,
			   enum hws_stress_jump addr_stress_jump,
			   enum hws_pattern_duration duration,
			   enum hws_bist_operation oper_type,
			   u32 offset, u32 cs_num, u32 pattern_addr_length);
int hws_ddr3_run_bist(u32 dev_num, enum hws_pattern pattern, u32 *result,
		      u32 cs_num);
int ddr3_tip_run_sweep_test(int dev_num, u32 repeat_num, u32 direction,
			    u32 mode);
int ddr3_tip_run_leveling_sweep_test(int dev_num, u32 repeat_num,
				     u32 direction, u32 mode);
int ddr3_tip_print_regs(u32 dev_num);
int ddr3_tip_reg_dump(u32 dev_num);
int run_xsb_test(u32 dev_num, u32 mem_addr, u32 write_type, u32 read_type,
		 u32 burst_length);
int mv_ddr_dm_to_dq_diff_get(u8 adll_byte_high, u8 adll_byte_low, u8 *vw_vector,
			     int *delta_h_adll, int *delta_l_adll);
int mv_ddr_dm_vw_get(enum hws_pattern pattern, u32 cs, u8 *vw_vector);
#endif /* _DDR3_TRAINING_IP_BIST_H_ */
