/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_PRV_IF_H
#define _DDR3_TRAINING_IP_PRV_IF_H

#include "ddr3_training_ip.h"
#include "ddr3_training_ip_flow.h"
#include "ddr3_training_ip_bist.h"

enum hws_static_config_type {
	WRITE_LEVELING_STATIC,
	READ_LEVELING_STATIC
};

struct ddr3_device_info {
	u32 device_id;
	u32 ck_delay;
};

typedef int (*HWS_TIP_DUNIT_MUX_SELECT_FUNC_PTR)(u8 dev_num, int enable);
typedef int (*HWS_TIP_DUNIT_REG_READ_FUNC_PTR)(
	u8 dev_num, enum hws_access_type interface_access, u32 if_id,
	u32 offset, u32 *data, u32 mask);
typedef int (*HWS_TIP_DUNIT_REG_WRITE_FUNC_PTR)(
	u8 dev_num, enum hws_access_type interface_access, u32 if_id,
	u32 offset, u32 data, u32 mask);
typedef int (*HWS_TIP_GET_FREQ_CONFIG_INFO)(
	u8 dev_num, enum mv_ddr_freq freq,
	struct hws_tip_freq_config_info *freq_config_info);
typedef int (*HWS_TIP_GET_DEVICE_INFO)(
	u8 dev_num, struct ddr3_device_info *info_ptr);
typedef int (*HWS_GET_CS_CONFIG_FUNC_PTR)(
	u8 dev_num, u32 cs_mask, struct hws_cs_config_info *cs_info);
typedef int (*HWS_SET_FREQ_DIVIDER_FUNC_PTR)(
	u8 dev_num, u32 if_id, enum mv_ddr_freq freq);
typedef int (*HWS_GET_INIT_FREQ)(u8 dev_num, enum mv_ddr_freq *freq);
typedef int (*HWS_TRAINING_IP_IF_WRITE_FUNC_PTR)(
	u32 dev_num, enum hws_access_type access_type, u32 dunit_id,
	u32 reg_addr, u32 data, u32 mask);
typedef int (*HWS_TRAINING_IP_IF_READ_FUNC_PTR)(
	u32 dev_num, enum hws_access_type access_type, u32 dunit_id,
	u32 reg_addr, u32 *data, u32 mask);
typedef int (*HWS_TRAINING_IP_BUS_WRITE_FUNC_PTR)(
	u32 dev_num, enum hws_access_type dunit_access_type, u32 if_id,
	enum hws_access_type phy_access_type, u32 phy_id,
	enum hws_ddr_phy phy_type, u32 reg_addr, u32 data);
typedef int (*HWS_TRAINING_IP_BUS_READ_FUNC_PTR)(
	u32 dev_num, u32 if_id, enum hws_access_type phy_access_type,
	u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr, u32 *data);
typedef int (*HWS_TRAINING_IP_ALGO_RUN_FUNC_PTR)(
	u32 dev_num, enum hws_algo_type algo_type);
typedef int (*HWS_TRAINING_IP_SET_FREQ_FUNC_PTR)(
	u32 dev_num, enum hws_access_type access_type, u32 if_id,
	enum mv_ddr_freq frequency);
typedef int (*HWS_TRAINING_IP_INIT_CONTROLLER_FUNC_PTR)(
	u32 dev_num, struct init_cntr_param *init_cntr_prm);
typedef int (*HWS_TRAINING_IP_PBS_RX_FUNC_PTR)(u32 dev_num);
typedef int (*HWS_TRAINING_IP_PBS_TX_FUNC_PTR)(u32 dev_num);
typedef int (*HWS_TRAINING_IP_SELECT_CONTROLLER_FUNC_PTR)(
	u32 dev_num, int enable);
typedef int (*HWS_TRAINING_IP_TOPOLOGY_MAP_LOAD_FUNC_PTR)(
	u32 dev_num, struct mv_ddr_topology_map *tm);
typedef int (*HWS_TRAINING_IP_STATIC_CONFIG_FUNC_PTR)(
	u32 dev_num, enum mv_ddr_freq frequency,
	enum hws_static_config_type static_config_type, u32 if_id);
typedef int (*HWS_TRAINING_IP_EXTERNAL_READ_PTR)(
	u32 dev_num, u32 if_id, u32 ddr_addr, u32 num_bursts, u32 *data);
typedef int (*HWS_TRAINING_IP_EXTERNAL_WRITE_PTR)(
	u32 dev_num, u32 if_id, u32 ddr_addr, u32 num_bursts, u32 *data);
typedef int (*HWS_TRAINING_IP_BIST_ACTIVATE)(
	u32 dev_num, enum hws_pattern pattern, enum hws_access_type access_type,
	u32 if_num, enum hws_dir direction,
	enum hws_stress_jump addr_stress_jump,
	enum hws_pattern_duration duration,
	enum hws_bist_operation oper_type, u32 offset, u32 cs_num,
	u32 pattern_addr_length);
typedef int (*HWS_TRAINING_IP_BIST_READ_RESULT)(
	u32 dev_num, u32 if_id, struct bist_result *pst_bist_result);
typedef int (*HWS_TRAINING_IP_LOAD_TOPOLOGY)(u32 dev_num, u32 config_num);
typedef int (*HWS_TRAINING_IP_READ_LEVELING)(u32 dev_num, u32 config_num);
typedef int (*HWS_TRAINING_IP_WRITE_LEVELING)(u32 dev_num, u32 config_num);
typedef u32 (*HWS_TRAINING_IP_GET_TEMP)(u8 dev_num);
typedef u8 (*HWS_TRAINING_IP_GET_RATIO)(u32 freq);

struct hws_tip_config_func_db {
	HWS_TIP_DUNIT_MUX_SELECT_FUNC_PTR tip_dunit_mux_select_func;
	void (*mv_ddr_dunit_read)(u32 addr, u32 mask, u32 *data);
	void (*mv_ddr_dunit_write)(u32 addr, u32 mask, u32 data);
	HWS_TIP_GET_FREQ_CONFIG_INFO tip_get_freq_config_info_func;
	HWS_TIP_GET_DEVICE_INFO tip_get_device_info_func;
	HWS_SET_FREQ_DIVIDER_FUNC_PTR tip_set_freq_divider_func;
	HWS_GET_CS_CONFIG_FUNC_PTR tip_get_cs_config_info;
	HWS_TRAINING_IP_GET_TEMP tip_get_temperature;
	HWS_TRAINING_IP_GET_RATIO tip_get_clock_ratio;
	HWS_TRAINING_IP_EXTERNAL_READ_PTR tip_external_read;
	HWS_TRAINING_IP_EXTERNAL_WRITE_PTR tip_external_write;
	int (*mv_ddr_phy_read)(enum hws_access_type phy_access,
			       u32 phy, enum hws_ddr_phy phy_type,
			       u32 reg_addr, u32 *data);
	int (*mv_ddr_phy_write)(enum hws_access_type phy_access,
				u32 phy, enum hws_ddr_phy phy_type,
				u32 reg_addr, u32 data,
				enum hws_operation op_type);
};

int ddr3_tip_init_config_func(u32 dev_num,
			      struct hws_tip_config_func_db *config_func);
int ddr3_tip_register_xsb_info(u32 dev_num,
			       struct hws_xsb_info *xsb_info_table);
enum hws_result *ddr3_tip_get_result_ptr(u32 stage);
int ddr3_set_freq_config_info(struct hws_tip_freq_config_info *table);
int print_device_info(u8 dev_num);

#endif /* _DDR3_TRAINING_IP_PRV_IF_H */
