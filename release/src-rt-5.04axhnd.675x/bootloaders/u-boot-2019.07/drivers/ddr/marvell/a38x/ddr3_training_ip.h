/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_H_
#define _DDR3_TRAINING_IP_H_

#include "ddr_topology_def.h"

#define TIP_ENG_LOCK	0x02000000
#define TIP_TX_DLL_RANGE_MAX	64

#define GET_MIN(arg1, arg2)	((arg1) < (arg2)) ? (arg1) : (arg2)
#define GET_MAX(arg1, arg2)	((arg1) < (arg2)) ? (arg2) : (arg1)

#define INIT_CONTROLLER_MASK_BIT	0x00000001
#define STATIC_LEVELING_MASK_BIT	0x00000002
#define SET_LOW_FREQ_MASK_BIT		0x00000004
#define LOAD_PATTERN_MASK_BIT		0x00000008
#define SET_MEDIUM_FREQ_MASK_BIT	0x00000010
#define WRITE_LEVELING_MASK_BIT		0x00000020
#define LOAD_PATTERN_2_MASK_BIT		0x00000040
#define READ_LEVELING_MASK_BIT		0x00000080
#define SW_READ_LEVELING_MASK_BIT	0x00000100
#define WRITE_LEVELING_SUPP_MASK_BIT	0x00000200
#define PBS_RX_MASK_BIT			0x00000400
#define PBS_TX_MASK_BIT			0x00000800
#define SET_TARGET_FREQ_MASK_BIT	0x00001000
#define ADJUST_DQS_MASK_BIT		0x00002000
#define WRITE_LEVELING_TF_MASK_BIT	0x00004000
#define LOAD_PATTERN_HIGH_MASK_BIT	0x00008000
#define READ_LEVELING_TF_MASK_BIT	0x00010000
#define WRITE_LEVELING_SUPP_TF_MASK_BIT	0x00020000
#define DM_PBS_TX_MASK_BIT		0x00040000
#define RL_DQS_BURST_MASK_BIT		0x00080000
#define CENTRALIZATION_RX_MASK_BIT	0x00100000
#define CENTRALIZATION_TX_MASK_BIT	0x00200000
#define TX_EMPHASIS_MASK_BIT		0x00400000
#define PER_BIT_READ_LEVELING_TF_MASK_BIT	0x00800000
#define VREF_CALIBRATION_MASK_BIT	0x01000000
#define WRITE_LEVELING_LF_MASK_BIT	0x02000000

/* DDR4 Specific Training Mask bits */

enum hws_result {
	TEST_FAILED = 0,
	TEST_SUCCESS = 1,
	NO_TEST_DONE = 2
};

enum hws_training_result {
	RESULT_PER_BIT,
	RESULT_PER_BYTE
};

enum auto_tune_stage {
	INIT_CONTROLLER,
	STATIC_LEVELING,
	SET_LOW_FREQ,
	LOAD_PATTERN,
	SET_MEDIUM_FREQ,
	WRITE_LEVELING,
	LOAD_PATTERN_2,
	READ_LEVELING,
	WRITE_LEVELING_SUPP,
	PBS_RX,
	PBS_TX,
	SET_TARGET_FREQ,
	ADJUST_DQS,
	WRITE_LEVELING_TF,
	READ_LEVELING_TF,
	WRITE_LEVELING_SUPP_TF,
	DM_PBS_TX,
	VREF_CALIBRATION,
	CENTRALIZATION_RX,
	CENTRALIZATION_TX,
	TX_EMPHASIS,
	LOAD_PATTERN_HIGH,
	PER_BIT_READ_LEVELING_TF,
	WRITE_LEVELING_LF,
	MAX_STAGE_LIMIT
};

enum hws_access_type {
	ACCESS_TYPE_UNICAST = 0,
	ACCESS_TYPE_MULTICAST = 1
};

enum hws_algo_type {
	ALGO_TYPE_DYNAMIC,
	ALGO_TYPE_STATIC
};

struct init_cntr_param {
	int is_ctrl64_bit;
	int do_mrs_phy;
	int init_phy;
	int msys_init;
};

struct pattern_info {
	u8 num_of_phases_tx;
	u8 tx_burst_size;
	u8 delay_between_bursts;
	u8 num_of_phases_rx;
	u32 start_addr;
	u8 pattern_len;
};

struct cs_element {
	u8 cs_num;
	u8 num_of_cs;
};

struct hws_tip_freq_config_info {
	u8 is_supported;
	u8 bw_per_freq;
	u8 rate_per_freq;
};

struct hws_cs_config_info {
	u32 cs_reg_value;
	u32 cs_cbe_value;
};

struct dfx_access {
	u8 pipe;
	u8 client;
};

struct hws_xsb_info {
	struct dfx_access *dfx_table;
};

int ddr3_tip_register_dq_table(u32 dev_num, u32 *table);
int hws_ddr3_tip_select_ddr_controller(u32 dev_num, int enable);
int hws_ddr3_tip_init_controller(u32 dev_num,
				 struct init_cntr_param *init_cntr_prm);
int hws_ddr3_tip_load_topology_map(u32 dev_num,
				   struct mv_ddr_topology_map *topology);
int hws_ddr3_tip_run_alg(u32 dev_num, enum hws_algo_type algo_type);
int ddr3_tip_is_pup_lock(u32 *pup_buf, enum hws_training_result read_mode);
u8 ddr3_tip_get_buf_min(u8 *buf_ptr);
u8 ddr3_tip_get_buf_max(u8 *buf_ptr);
#endif /* _DDR3_TRAINING_IP_H_ */
