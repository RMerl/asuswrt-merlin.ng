/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_INIT_H
#define _DDR3_INIT_H

#include "ddr_ml_wrapper.h"
#include "mv_ddr_plat.h"

#include "seq_exec.h"
#include "ddr3_logging_def.h"
#include "ddr3_training_hw_algo.h"
#include "ddr3_training_ip.h"
#include "ddr3_training_ip_centralization.h"
#include "ddr3_training_ip_engine.h"
#include "ddr3_training_ip_flow.h"
#include "ddr3_training_ip_pbs.h"
#include "ddr3_training_ip_prv_if.h"
#include "ddr3_training_leveling.h"
#include "xor.h"

/* For checking function return values */
#define CHECK_STATUS(orig_func)		\
	{				\
		int status;		\
		status = orig_func;	\
		if (MV_OK != status)	\
			return status;	\
	}

#define SUB_VERSION	0

enum log_level  {
	MV_LOG_LEVEL_0,
	MV_LOG_LEVEL_1,
	MV_LOG_LEVEL_2,
	MV_LOG_LEVEL_3
};

/* TODO: consider to move to misl phy driver */
#define MISL_PHY_DRV_P_OFFS	0x7
#define MISL_PHY_DRV_N_OFFS	0x0
#define MISL_PHY_ODT_P_OFFS	0x6
#define MISL_PHY_ODT_N_OFFS	0x0

/* Globals */
extern u8 debug_training, debug_calibration, debug_ddr4_centralization,
	debug_tap_tuning, debug_dm_tuning;
extern u8 is_reg_dump;
extern u8 generic_init_controller;
/* list of allowed frequency listed in order of enum mv_ddr_freq */
extern u32 is_pll_old;
extern struct pattern_info pattern_table[];
extern u8 debug_centralization, debug_training_ip, debug_training_bist,
	debug_pbs, debug_training_static, debug_leveling;
extern struct hws_tip_config_func_db config_func_info[];
extern u8 twr_mask_table[];
extern u8 cl_mask_table[];
extern u8 cwl_mask_table[];
extern u32 speed_bin_table_t_rc[];
extern u32 speed_bin_table_t_rcd_t_rp[];

extern u32 vref_init_val;
extern u32 g_zpri_data;
extern u32 g_znri_data;
extern u32 g_zpri_ctrl;
extern u32 g_znri_ctrl;
extern u32 g_zpodt_data;
extern u32 g_znodt_data;
extern u32 g_zpodt_ctrl;
extern u32 g_znodt_ctrl;
extern u32 g_dic;
extern u32 g_odt_config;
extern u32 g_rtt_nom;
extern u32 g_rtt_wr;
extern u32 g_rtt_park;

extern u8 debug_training_access;
extern u32 first_active_if;
extern u32 delay_enable, ck_delay, ca_delay;
extern u32 mask_tune_func;
extern u32 rl_version;
extern int rl_mid_freq_wa;
extern u8 calibration_update_control; /* 2 external only, 1 is internal only */
extern enum mv_ddr_freq medium_freq;

extern enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
extern enum mv_ddr_freq low_freq;
extern enum auto_tune_stage training_stage;
extern u32 is_pll_before_init;
extern u32 is_adll_calib_before_init;
extern u32 is_dfs_in_init;
extern int wl_debug_delay;
extern u32 silicon_delay[MAX_DEVICE_NUM];
extern u32 start_pattern, end_pattern;
extern u32 phy_reg0_val;
extern u32 phy_reg1_val;
extern u32 phy_reg2_val;
extern u32 phy_reg3_val;
extern enum hws_pattern sweep_pattern;
extern enum hws_pattern pbs_pattern;
extern u32 g_znri_data;
extern u32 g_zpri_data;
extern u32 g_znri_ctrl;
extern u32 g_zpri_ctrl;
extern u32 finger_test, p_finger_start, p_finger_end, n_finger_start,
	n_finger_end, p_finger_step, n_finger_step;
extern u32 mode_2t;
extern u32 xsb_validate_type;
extern u32 xsb_validation_base_address;
extern u32 odt_additional;
extern u32 debug_mode;
extern u32 debug_dunit;
extern u32 clamp_tbl[];
extern u32 freq_mask[MAX_DEVICE_NUM][MV_DDR_FREQ_LAST];

extern u32 maxt_poll_tries;
extern u32 is_bist_reset_bit;

extern u8 vref_window_size[MAX_INTERFACE_NUM][MAX_BUS_NUM];
extern u32 effective_cs;
extern int ddr3_tip_centr_skip_min_win_check;
extern u32 *dq_map_table;

extern u8 debug_training_hw_alg;

extern u32 start_xsb_offset;
extern u32 odt_config;

extern u16 mask_results_dq_reg_map[];

extern u32 target_freq;
extern u32 dfs_low_freq;

extern u32 nominal_avs;
extern u32 extension_avs;


/* Prototypes */
int ddr3_init(void);
int ddr3_tip_enable_init_sequence(u32 dev_num);

int ddr3_hws_hw_training(enum hws_algo_type algo_mode);
int mv_ddr_early_init(void);
int mv_ddr_early_init2(void);
int ddr3_silicon_post_init(void);
int ddr3_post_run_alg(void);
void ddr3_new_tip_ecc_scrub(void);

int ddr3_tip_reg_write(u32 dev_num, u32 reg_addr, u32 data);
int ddr3_tip_reg_read(u32 dev_num, u32 reg_addr, u32 *data, u32 reg_mask);
int ddr3_silicon_get_ddr_target_freq(u32 *ddr_freq);

int print_adll(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM]);
int print_ph(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM]);
int read_phase_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		     int reg_addr, u32 mask);
int write_leveling_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			 u32 pup_ph_values[MAX_INTERFACE_NUM * MAX_BUS_NUM], int reg_addr);
int ddr3_tip_restore_dunit_regs(u32 dev_num);
void print_topology(struct mv_ddr_topology_map *tm);

u32 mv_board_id_get(void);

int ddr3_load_topology_map(void);
void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level);
void mv_ddr_user_log_level_set(enum ddr_lib_debug_block block);
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params);
void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps);
void ddr3_fast_path_static_cs_size_config(u32 cs_ena);
u32 mv_board_id_index_get(u32 board_id);
void ddr3_set_log_level(u32 n_log_level);

int hws_ddr3_cs_base_adr_calc(u32 if_id, u32 cs, u32 *cs_base_addr);

int ddr3_tip_print_pbs_result(u32 dev_num, u32 cs_num, enum pbs_dir pbs_mode);
int ddr3_tip_clean_pbs_result(u32 dev_num, enum pbs_dir pbs_mode);
void mv_ddr_mc_config(void);
int mv_ddr_mc_init(void);
void mv_ddr_set_calib_controller(void);
/* TODO: consider to move to misl phy driver */
unsigned int mv_ddr_misl_phy_drv_data_p_get(void);
unsigned int mv_ddr_misl_phy_drv_data_n_get(void);
unsigned int mv_ddr_misl_phy_drv_ctrl_p_get(void);
unsigned int mv_ddr_misl_phy_drv_ctrl_n_get(void);
unsigned int mv_ddr_misl_phy_odt_p_get(void);
unsigned int mv_ddr_misl_phy_odt_n_get(void);

#endif /* _DDR3_INIT_H */
