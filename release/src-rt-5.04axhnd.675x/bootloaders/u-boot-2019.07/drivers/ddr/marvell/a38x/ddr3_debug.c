// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_regs.h"

u8 is_reg_dump = 0;
u8 debug_pbs = DEBUG_LEVEL_ERROR;

/*
 * API to change flags outside of the lib
 */
#if defined(SILENT_LIB)
void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level)
{
	/* do nothing */
}
#else /* SILENT_LIB */
/* Debug flags for other Training modules */
u8 debug_training_static = DEBUG_LEVEL_ERROR;
u8 debug_training = DEBUG_LEVEL_ERROR;
u8 debug_leveling = DEBUG_LEVEL_ERROR;
u8 debug_centralization = DEBUG_LEVEL_ERROR;
u8 debug_training_ip = DEBUG_LEVEL_ERROR;
u8 debug_training_bist = DEBUG_LEVEL_ERROR;
u8 debug_training_hw_alg = DEBUG_LEVEL_ERROR;
u8 debug_training_access = DEBUG_LEVEL_ERROR;
u8 debug_training_device = DEBUG_LEVEL_ERROR;


void mv_ddr_user_log_level_set(enum ddr_lib_debug_block block)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	ddr3_hws_set_log_level(block, tm->debug_level);
};

void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level)
{
	switch (block) {
	case DEBUG_BLOCK_STATIC:
		debug_training_static = level;
		break;
	case DEBUG_BLOCK_TRAINING_MAIN:
		debug_training = level;
		break;
	case DEBUG_BLOCK_LEVELING:
		debug_leveling = level;
		break;
	case DEBUG_BLOCK_CENTRALIZATION:
		debug_centralization = level;
		break;
	case DEBUG_BLOCK_PBS:
		debug_pbs = level;
		break;
	case DEBUG_BLOCK_ALG:
		debug_training_hw_alg = level;
		break;
	case DEBUG_BLOCK_DEVICE:
		debug_training_device = level;
		break;
	case DEBUG_BLOCK_ACCESS:
		debug_training_access = level;
		break;
	case DEBUG_STAGES_REG_DUMP:
		if (level == DEBUG_LEVEL_TRACE)
			is_reg_dump = 1;
		else
			is_reg_dump = 0;
		break;
	case DEBUG_BLOCK_ALL:
	default:
		debug_training_static = level;
		debug_training = level;
		debug_leveling = level;
		debug_centralization = level;
		debug_pbs = level;
		debug_training_hw_alg = level;
		debug_training_access = level;
		debug_training_device = level;
	}
}
#endif /* SILENT_LIB */

#if defined(DDR_VIEWER_TOOL)
static char *convert_freq(enum mv_ddr_freq freq);
#if defined(EXCLUDE_SWITCH_DEBUG)
u32 ctrl_sweepres[ADLL_LENGTH][MAX_INTERFACE_NUM][MAX_BUS_NUM];
u32 ctrl_adll[MAX_CS_NUM * MAX_INTERFACE_NUM * MAX_BUS_NUM];
u32 ctrl_adll1[MAX_CS_NUM * MAX_INTERFACE_NUM * MAX_BUS_NUM];
u32 ctrl_level_phase[MAX_CS_NUM * MAX_INTERFACE_NUM * MAX_BUS_NUM];
#endif /* EXCLUDE_SWITCH_DEBUG */
#endif /* DDR_VIEWER_TOOL */

struct hws_tip_config_func_db config_func_info[MAX_DEVICE_NUM];
u8 is_default_centralization = 0;
u8 is_tune_result = 0;
u8 is_validate_window_per_if = 0;
u8 is_validate_window_per_pup = 0;
u8 sweep_cnt = 1;
u32 is_bist_reset_bit = 1;
u8 is_run_leveling_sweep_tests;

static struct hws_xsb_info xsb_info[MAX_DEVICE_NUM];

/*
 * Dump Dunit & Phy registers
 */
int ddr3_tip_reg_dump(u32 dev_num)
{
	u32 if_id, reg_addr, data_value, bus_id;
	u32 read_data[MAX_INTERFACE_NUM];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	printf("-- dunit registers --\n");
	for (reg_addr = 0x1400; reg_addr < 0x19f0; reg_addr += 4) {
		printf("0x%x ", reg_addr);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, reg_addr, read_data,
				      MASK_ALL_BITS));
			printf("0x%x ", read_data[if_id]);
		}
		printf("\n");
	}

	printf("-- Phy registers --\n");
	for (reg_addr = 0; reg_addr <= 0xff; reg_addr++) {
		printf("0x%x ", reg_addr);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			for (bus_id = 0;
			     bus_id < octets_per_if_num;
			     bus_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_id,
					      DDR_PHY_DATA, reg_addr,
					      &data_value));
				printf("0x%x ", data_value);
			}
			for (bus_id = 0;
			     bus_id < octets_per_if_num;
			     bus_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_id,
					      DDR_PHY_CONTROL, reg_addr,
					      &data_value));
				printf("0x%x ", data_value);
			}
		}
		printf("\n");
	}

	return MV_OK;
}

/*
 * Register access func registration
 */
int ddr3_tip_init_config_func(u32 dev_num,
			      struct hws_tip_config_func_db *config_func)
{
	if (config_func == NULL)
		return MV_BAD_PARAM;

	memcpy(&config_func_info[dev_num], config_func,
	       sizeof(struct hws_tip_config_func_db));

	return MV_OK;
}

/*
 * Get training result info pointer
 */
enum hws_result *ddr3_tip_get_result_ptr(u32 stage)
{
	return training_result[stage];
}

/*
 * Device info read
 */
int ddr3_tip_get_device_info(u32 dev_num, struct ddr3_device_info *info_ptr)
{
	if (config_func_info[dev_num].tip_get_device_info_func != NULL) {
		return config_func_info[dev_num].
			tip_get_device_info_func((u8) dev_num, info_ptr);
	}

	return MV_FAIL;
}

#if defined(DDR_VIEWER_TOOL)
/*
 * Convert freq to character string
 */
static char *convert_freq(enum mv_ddr_freq freq)
{
	switch (freq) {
	case MV_DDR_FREQ_LOW_FREQ:
		return "MV_DDR_FREQ_LOW_FREQ";

	case MV_DDR_FREQ_400:
		return "400";

	case MV_DDR_FREQ_533:
		return "533";

	case MV_DDR_FREQ_667:
		return "667";

	case MV_DDR_FREQ_800:
		return "800";

	case MV_DDR_FREQ_933:
		return "933";

	case MV_DDR_FREQ_1066:
		return "1066";

	case MV_DDR_FREQ_311:
		return "311";

	case MV_DDR_FREQ_333:
		return "333";

	case MV_DDR_FREQ_467:
		return "467";

	case MV_DDR_FREQ_850:
		return "850";

	case MV_DDR_FREQ_900:
		return "900";

	case MV_DDR_FREQ_360:
		return "MV_DDR_FREQ_360";

	case MV_DDR_FREQ_1000:
		return "MV_DDR_FREQ_1000";

	default:
		return "Unknown Frequency";
	}
}

/*
 * Convert device ID to character string
 */
static char *convert_dev_id(u32 dev_id)
{
	switch (dev_id) {
	case 0x6800:
		return "A38xx";
	case 0x6900:
		return "A39XX";
	case 0xf400:
		return "AC3";
	case 0xfc00:
		return "BC2";

	default:
		return "Unknown Device";
	}
}

/*
 * Convert device ID to character string
 */
static char *convert_mem_size(u32 dev_id)
{
	switch (dev_id) {
	case 0:
		return "512 MB";
	case 1:
		return "1 GB";
	case 2:
		return "2 GB";
	case 3:
		return "4 GB";
	case 4:
		return "8 GB";

	default:
		return "wrong mem size";
	}
}

int print_device_info(u8 dev_num)
{
	struct ddr3_device_info info_ptr;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	CHECK_STATUS(ddr3_tip_get_device_info(dev_num, &info_ptr));
	printf("=== DDR setup START===\n");
	printf("\tDevice ID: %s\n", convert_dev_id(info_ptr.device_id));
	printf("\tDDR3  CK delay: %d\n", info_ptr.ck_delay);
	print_topology(tm);
	printf("=== DDR setup END===\n");

	return MV_OK;
}

void hws_ddr3_tip_sweep_test(int enable)
{
	if (enable) {
		is_validate_window_per_if = 1;
		is_validate_window_per_pup = 1;
		debug_training = DEBUG_LEVEL_TRACE;
	} else {
		is_validate_window_per_if = 0;
		is_validate_window_per_pup = 0;
	}
}
#endif /* DDR_VIEWER_TOOL */

char *ddr3_tip_convert_tune_result(enum hws_result tune_result)
{
	switch (tune_result) {
	case TEST_FAILED:
		return "FAILED";
	case TEST_SUCCESS:
		return "PASS";
	case NO_TEST_DONE:
		return "NOT COMPLETED";
	default:
		return "Un-KNOWN";
	}
}

/*
 * Print log info
 */
int ddr3_tip_print_log(u32 dev_num, u32 mem_addr)
{
	u32 if_id = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

#if defined(DDR_VIEWER_TOOL)
	if ((is_validate_window_per_if != 0) ||
	    (is_validate_window_per_pup != 0)) {
		u32 is_pup_log = 0;
		enum mv_ddr_freq freq;

		freq = tm->interface_params[first_active_if].memory_freq;

		is_pup_log = (is_validate_window_per_pup != 0) ? 1 : 0;
		printf("===VALIDATE WINDOW LOG START===\n");
		printf("DDR Frequency: %s   ======\n", convert_freq(freq));
		/* print sweep windows */
		ddr3_tip_run_sweep_test(dev_num, sweep_cnt, 1, is_pup_log);
		ddr3_tip_run_sweep_test(dev_num, sweep_cnt, 0, is_pup_log);
#if defined(EXCLUDE_SWITCH_DEBUG)
		if (is_run_leveling_sweep_tests == 1) {
			ddr3_tip_run_leveling_sweep_test(dev_num, sweep_cnt, 0, is_pup_log);
			ddr3_tip_run_leveling_sweep_test(dev_num, sweep_cnt, 1, is_pup_log);
		}
#endif /* EXCLUDE_SWITCH_DEBUG */
		ddr3_tip_print_all_pbs_result(dev_num);
		ddr3_tip_print_wl_supp_result(dev_num);
		printf("===VALIDATE WINDOW LOG END ===\n");
		CHECK_STATUS(ddr3_tip_restore_dunit_regs(dev_num));
		ddr3_tip_reg_dump(dev_num);
	}
#endif /* DDR_VIEWER_TOOL */

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("IF %d Status:\n", if_id));

		if (mask_tune_func & INIT_CONTROLLER_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tInit Controller: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[INIT_CONTROLLER]
					    [if_id])));
		}
		if (mask_tune_func & SET_LOW_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLow freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_LOW_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & LOAD_PATTERN_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLoad Pattern: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[LOAD_PATTERN]
					    [if_id])));
		}
		if (mask_tune_func & SET_MEDIUM_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tMedium freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_MEDIUM_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING]
					    [if_id])));
		}
		if (mask_tune_func & LOAD_PATTERN_2_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLoad Pattern: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[LOAD_PATTERN_2]
					    [if_id])));
		}
		if (mask_tune_func & READ_LEVELING_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tRL: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[READ_LEVELING]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_SUPP_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL Supp: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING_SUPP]
					    [if_id])));
		}
		if (mask_tune_func & PBS_RX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tPBS RX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[PBS_RX]
					    [if_id])));
		}
		if (mask_tune_func & PBS_TX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tPBS TX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[PBS_TX]
					    [if_id])));
		}
		if (mask_tune_func & SET_TARGET_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tTarget freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_TARGET_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL TF: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING_TF]
					    [if_id])));
		}
		if (mask_tune_func & READ_LEVELING_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tRL TF: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[READ_LEVELING_TF]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_SUPP_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL TF Supp: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result
					    [WRITE_LEVELING_SUPP_TF]
					    [if_id])));
		}
		if (mask_tune_func & CENTRALIZATION_RX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tCentr RX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[CENTRALIZATION_RX]
					    [if_id])));
		}
		if (mask_tune_func & VREF_CALIBRATION_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tVREF_CALIBRATION: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[VREF_CALIBRATION]
					    [if_id])));
		}
		if (mask_tune_func & CENTRALIZATION_TX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tCentr TX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[CENTRALIZATION_TX]
					    [if_id])));
		}
	}

	return MV_OK;
}

#if !defined(EXCLUDE_DEBUG_PRINTS)
/*
 * Print stability log info
 */
int ddr3_tip_print_stability_log(u32 dev_num)
{
	u8 if_id = 0, csindex = 0, bus_id = 0, idx = 0;
	u32 reg_data;
	u32 read_data[MAX_INTERFACE_NUM];
	unsigned int max_cs = mv_ddr_cs_num_get();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* Title print */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		printf("Title: I/F# , Tj, Calibration_n0, Calibration_p0, Calibration_n1, Calibration_p1, Calibration_n2, Calibration_p2,");
		for (csindex = 0; csindex < max_cs; csindex++) {
			printf("CS%d , ", csindex);
			printf("\n");
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			printf("VWTx, VWRx, WL_tot, WL_ADLL, WL_PH, RL_Tot, RL_ADLL, RL_PH, RL_Smp, Cen_tx, Cen_rx, Vref, DQVref,");
			printf("\t\t");
			for (idx = 0; idx < 11; idx++)
				printf("PBSTx-Pad%d,", idx);
			printf("\t\t");
			for (idx = 0; idx < 11; idx++)
				printf("PBSRx-Pad%d,", idx);
		}
	}
	printf("\n");

	/* Data print */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		printf("Data: %d,%d,", if_id,
		       (config_func_info[dev_num].tip_get_temperature != NULL)
		       ? (config_func_info[dev_num].
			  tip_get_temperature(dev_num)) : (0));

		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x14c8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0) >> 4),
		       ((read_data[if_id] & 0xfc00) >> 10));
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x17c8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0) >> 4),
		       ((read_data[if_id] & 0xfc00) >> 10));
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1dc8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0000) >> 16),
		       ((read_data[if_id] & 0xfc00000) >> 22));

		for (csindex = 0; csindex < max_cs; csindex++) {
			printf("CS%d , ", csindex);
			for (bus_id = 0; bus_id < MAX_BUS_NUM; bus_id++) {
				printf("\n");
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST,
						  bus_id, DDR_PHY_DATA,
						  RESULT_PHY_REG +
						  csindex, &reg_data);
				printf("%d,%d,", (reg_data & 0x1f),
				       ((reg_data & 0x3e0) >> 5));
				/* WL */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST,
						  bus_id, DDR_PHY_DATA,
						  WL_PHY_REG(csindex),
						  &reg_data);
				printf("%d,%d,%d,",
				       (reg_data & 0x1f) +
				       ((reg_data & 0x1c0) >> 6) * 32,
				       (reg_data & 0x1f),
				       (reg_data & 0x1c0) >> 6);
				/* RL */
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id,
					      RD_DATA_SMPL_DLYS_REG,
					      read_data, MASK_ALL_BITS));
				read_data[if_id] =
					(read_data[if_id] &
					 (0x1f << (8 * csindex))) >>
					(8 * csindex);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  RL_PHY_REG(csindex),
						  &reg_data);
				printf("%d,%d,%d,%d,",
				       (reg_data & 0x1f) +
				       ((reg_data & 0x1c0) >> 6) * 32 +
				       read_data[if_id] * 64,
				       (reg_data & 0x1f),
				       ((reg_data & 0x1c0) >> 6),
				       read_data[if_id]);
				/* Centralization */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  CTX_PHY_REG(csindex),
						  &reg_data);
				printf("%d,", (reg_data & 0x3f));
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  CRX_PHY_REG(csindex),
						   &reg_data);
				printf("%d,", (reg_data & 0x1f));
				/* Vref */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  PAD_CFG_PHY_REG,
						  &reg_data);
				printf("%d,", (reg_data & 0x7));
				/* DQVref */
				/* Need to add the Read Function from device */
				printf("%d,", 0);
				printf("\t\t");
				for (idx = 0; idx < 11; idx++) {
					ddr3_tip_bus_read(dev_num, if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_id, DDR_PHY_DATA,
							  0x10 +
							  16 * csindex +
							  idx, &reg_data);
					printf("%d,", (reg_data & 0x3f));
				}
				printf("\t\t");
				for (idx = 0; idx < 11; idx++) {
					ddr3_tip_bus_read(dev_num, if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_id, DDR_PHY_DATA,
							  0x50 +
							  16 * csindex +
							  idx, &reg_data);
					printf("%d,", (reg_data & 0x3f));
				}
			}
		}
	}
	printf("\n");

	return MV_OK;
}
#endif /* EXCLUDE_DEBUG_PRINTS */

/*
 * Register XSB information
 */
int ddr3_tip_register_xsb_info(u32 dev_num, struct hws_xsb_info *xsb_info_table)
{
	memcpy(&xsb_info[dev_num], xsb_info_table, sizeof(struct hws_xsb_info));
	return MV_OK;
}

/*
 * Read ADLL Value
 */
int ddr3_tip_read_adll_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			     u32 reg_addr, u32 mask)
{
	u32 data_value;
	u32 if_id = 0, bus_id = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/*
	 * multi CS support - reg_addr is calucalated in calling function
	 * with CS offset
	 */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < octets_per_if_num;
		     bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			CHECK_STATUS(ddr3_tip_bus_read(dev_num, if_id,
						       ACCESS_TYPE_UNICAST,
						       bus_id,
						       DDR_PHY_DATA, reg_addr,
						       &data_value));
			pup_values[if_id *
				   octets_per_if_num + bus_id] =
				data_value & mask;
		}
	}

	return 0;
}

/*
 * Write ADLL Value
 */
int ddr3_tip_write_adll_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			      u32 reg_addr)
{
	u32 if_id = 0, bus_id = 0;
	u32 data;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/*
	 * multi CS support - reg_addr is calucalated in calling function
	 * with CS offset
	 */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < octets_per_if_num;
		     bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			data = pup_values[if_id *
					  octets_per_if_num +
					  bus_id];
			CHECK_STATUS(ddr3_tip_bus_write(dev_num,
							ACCESS_TYPE_UNICAST,
							if_id,
							ACCESS_TYPE_UNICAST,
							bus_id, DDR_PHY_DATA,
							reg_addr, data));
		}
	}

	return 0;
}

/**
 * Read Phase Value
 */
int read_phase_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		     int reg_addr, u32 mask)
{
	u32  data_value;
	u32 if_id = 0, bus_id = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* multi CS support - reg_addr is calucalated in calling function with CS offset */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < octets_per_if_num; bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			CHECK_STATUS(ddr3_tip_bus_read(dev_num, if_id,
						       ACCESS_TYPE_UNICAST,
						       bus_id,
						       DDR_PHY_DATA, reg_addr,
						       &data_value));
			pup_values[if_id * octets_per_if_num + bus_id] = data_value & mask;
		}
	}

	return 0;
}

/**
 * Write Leveling Value
 */
int write_leveling_value(u32 dev_num, u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			 u32 pup_ph_values[MAX_INTERFACE_NUM * MAX_BUS_NUM], int reg_addr)
{
	u32 if_id = 0, bus_id = 0;
	u32 data;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* multi CS support - reg_addr is calucalated in calling function with CS offset */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0 ; bus_id < octets_per_if_num ; bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			data = pup_values[if_id * octets_per_if_num + bus_id] +
			       pup_ph_values[if_id * octets_per_if_num + bus_id];
			CHECK_STATUS(ddr3_tip_bus_write(dev_num,
							ACCESS_TYPE_UNICAST,
							if_id,
							ACCESS_TYPE_UNICAST,
							bus_id,
							DDR_PHY_DATA,
							reg_addr,
							data));
		}
	}

	return 0;
}

#if !defined(EXCLUDE_SWITCH_DEBUG)
struct hws_tip_config_func_db config_func_info[MAX_DEVICE_NUM];
u32 start_xsb_offset = 0;
u8 is_rl_old = 0;
u8 is_freq_old = 0;
u8 is_dfs_disabled = 0;
u32 default_centrlization_value = 0x12;
u32 activate_select_before_run_alg = 1, activate_deselect_after_run_alg = 1,
	rl_test = 0, reset_read_fifo = 0;
int debug_acc = 0;
u32 ctrl_sweepres[ADLL_LENGTH][MAX_INTERFACE_NUM][MAX_BUS_NUM];
u32 ctrl_adll[MAX_CS_NUM * MAX_INTERFACE_NUM * MAX_BUS_NUM];

u32 xsb_test_table[][8] = {
	{0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555,
	 0x66666666, 0x77777777},
	{0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd,
	 0xeeeeeeee, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,
	 0xffffffff, 0xffffffff},
	{0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000,
	 0x00000000, 0x00000000},
	{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
	 0xffffffff, 0xffffffff}
};

int ddr3_tip_print_adll(void)
{
	u32 bus_cnt = 0, if_id, data_p1, data_p2, ui_data3, dev_num = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_cnt = 0; bus_cnt < octets_per_if_num;
		     bus_cnt++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id,
				      ACCESS_TYPE_UNICAST, bus_cnt,
				      DDR_PHY_DATA, 0x1, &data_p1));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_cnt, DDR_PHY_DATA, 0x2, &data_p2));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_cnt, DDR_PHY_DATA, 0x3, &ui_data3));
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  (" IF %d bus_cnt %d  phy_reg_1_data 0x%x phy_reg_2_data 0x%x phy_reg_3_data 0x%x\n",
					   if_id, bus_cnt, data_p1, data_p2,
					   ui_data3));
			}
	}

	return MV_OK;
}

#endif /* EXCLUDE_SWITCH_DEBUG */

#if defined(DDR_VIEWER_TOOL)
/*
 * Print ADLL
 */
int print_adll(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM])
{
	u32 i, j;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (j = 0; j < octets_per_if_num; j++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, j);
		for (i = 0; i < MAX_INTERFACE_NUM; i++)
			printf("%d ,", adll[i * octets_per_if_num + j]);
	}
	printf("\n");

	return MV_OK;
}

int print_ph(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM])
{
	u32 i, j;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (j = 0; j < octets_per_if_num; j++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, j);
		for (i = 0; i < MAX_INTERFACE_NUM; i++)
			printf("%d ,", adll[i * octets_per_if_num + j] >> 6);
	}
	printf("\n");

	return MV_OK;
}
#endif /* DDR_VIEWER_TOOL */

#if !defined(EXCLUDE_SWITCH_DEBUG)
/* byte_index - only byte 0, 1, 2, or 3, oxff - test all bytes */
static u32 ddr3_tip_compare(u32 if_id, u32 *p_src, u32 *p_dst,
			    u32 byte_index)
{
	u32 burst_cnt = 0, addr_offset, i_id;
	int b_is_fail = 0;

	addr_offset =
		(byte_index ==
		 0xff) ? (u32) 0xffffffff : (u32) (0xff << (byte_index * 8));
	for (burst_cnt = 0; burst_cnt < EXT_ACCESS_BURST_LENGTH; burst_cnt++) {
		if ((p_src[burst_cnt] & addr_offset) !=
		    (p_dst[if_id] & addr_offset))
			b_is_fail = 1;
	}

	if (b_is_fail == 1) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("IF %d exp: ", if_id));
		for (i_id = 0; i_id <= MAX_INTERFACE_NUM - 1; i_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("0x%8x ", p_src[i_id]));
		}
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("\n_i_f %d rcv: ", if_id));
		for (i_id = 0; i_id <= MAX_INTERFACE_NUM - 1; i_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("(0x%8x ", p_dst[i_id]));
		}
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("\n "));
	}

	return b_is_fail;
}
#endif /* EXCLUDE_SWITCH_DEBUG */

#if defined(DDR_VIEWER_TOOL)
/*
 * Sweep validation
 */
int ddr3_tip_run_sweep_test(int dev_num, u32 repeat_num, u32 direction,
			    u32 mode)
{
	u32 pup = 0, start_pup = 0, end_pup = 0;
	u32 adll = 0, rep = 0, pattern_idx = 0;
	u32 res[MAX_INTERFACE_NUM] = { 0 };
	int if_id = 0;
	u32 adll_value = 0;
	u32 reg;
	enum hws_access_type pup_access;
	u32 cs;
	unsigned int max_cs = mv_ddr_cs_num_get();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	repeat_num = 2;

	if (mode == 1) {
		/* per pup */
		start_pup = 0;
		end_pup = octets_per_if_num - 1;
		pup_access = ACCESS_TYPE_UNICAST;
	} else {
		start_pup = 0;
		end_pup = 0;
		pup_access = ACCESS_TYPE_MULTICAST;
	}

	for (cs = 0; cs < max_cs; cs++) {
		reg = (direction == 0) ? CTX_PHY_REG(cs) : CRX_PHY_REG(cs);
		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			for (if_id = 0;
			     if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE
					(tm->if_act_mask,
					 if_id);
				for (pup = start_pup; pup <= end_pup; pup++) {
					ctrl_sweepres[adll][if_id][pup] =
						0;
				}
			}
		}

		for (adll = 0; adll < (MAX_INTERFACE_NUM * MAX_BUS_NUM); adll++)
			ctrl_adll[adll] = 0;
			/* Save DQS value(after algorithm run) */
			ddr3_tip_read_adll_value(dev_num, ctrl_adll,
						 reg, MASK_ALL_BITS);

		/*
		 * Sweep ADLL  from 0:31 on all I/F on all Pup and perform
		 * BIST on each stage.
		 */
		for (pup = start_pup; pup <= end_pup; pup++) {
			for (adll = 0; adll < ADLL_LENGTH; adll++) {
				for (rep = 0; rep < repeat_num; rep++) {
					for (pattern_idx = PATTERN_KILLER_DQ0;
					     pattern_idx < PATTERN_LAST;
					     pattern_idx++) {
						adll_value =
							(direction == 0) ? (adll * 2) : adll;
						CHECK_STATUS(ddr3_tip_bus_write
							     (dev_num, ACCESS_TYPE_MULTICAST, 0,
							      pup_access, pup, DDR_PHY_DATA,
							      reg, adll_value));
						hws_ddr3_run_bist(dev_num, sweep_pattern, res,
								  cs);
						/* ddr3_tip_reset_fifo_ptr(dev_num); */
						for (if_id = 0;
						     if_id < MAX_INTERFACE_NUM;
						     if_id++) {
							VALIDATE_IF_ACTIVE
								(tm->if_act_mask,
								 if_id);
							ctrl_sweepres[adll][if_id][pup]
								+= res[if_id];
							if (mode == 1) {
								CHECK_STATUS
									(ddr3_tip_bus_write
									 (dev_num,
									  ACCESS_TYPE_UNICAST,
									  if_id,
									  ACCESS_TYPE_UNICAST,
									  pup,
									  DDR_PHY_DATA,
									  reg,
									  ctrl_adll[if_id *
										    cs *
										    octets_per_if_num
										    + pup]));
							}
						}
					}
				}
			}
		}
		printf("Final, CS %d,%s, Sweep, Result, Adll,", cs,
		       ((direction == 0) ? "TX" : "RX"));
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			if (mode == 1) {
				for (pup = start_pup; pup <= end_pup; pup++) {
					VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
					printf("I/F%d-PHY%d , ", if_id, pup);
				}
			} else {
				printf("I/F%d , ", if_id);
			}
		}
		printf("\n");

		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			adll_value = (direction == 0) ? (adll * 2) : adll;
			printf("Final,%s, Sweep, Result, %d ,",
			       ((direction == 0) ? "TX" : "RX"), adll_value);

			for (if_id = 0;
			     if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				for (pup = start_pup; pup <= end_pup; pup++) {
					printf("%8d , ",
					       ctrl_sweepres[adll][if_id]
					       [pup]);
				}
			}
			printf("\n");
		}

		/*
		 * Write back to the phy the Rx DQS value, we store in
		 * the beginning.
		 */
		ddr3_tip_write_adll_value(dev_num, ctrl_adll, reg);
		/* print adll results */
		ddr3_tip_read_adll_value(dev_num, ctrl_adll, reg, MASK_ALL_BITS);
		printf("%s, DQS, ADLL,,,", (direction == 0) ? "Tx" : "Rx");
		print_adll(dev_num, ctrl_adll);
	}
	ddr3_tip_reset_fifo_ptr(dev_num);

	return 0;
}

#if defined(EXCLUDE_SWITCH_DEBUG)
int ddr3_tip_run_leveling_sweep_test(int dev_num, u32 repeat_num,
				     u32 direction, u32 mode)
{
	u32 pup = 0, start_pup = 0, end_pup = 0, start_adll = 0;
	u32 adll = 0, rep = 0, pattern_idx = 0;
	u32 read_data[MAX_INTERFACE_NUM];
	u32 res[MAX_INTERFACE_NUM] = { 0 };
	int if_id = 0, gap = 0;
	u32 adll_value = 0;
	u32 reg;
	enum hws_access_type pup_access;
	u32 cs;
	unsigned int max_cs = mv_ddr_cs_num_get();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (mode == 1) { /* per pup */
		start_pup = 0;
		end_pup = octets_per_if_num - 1;
		pup_access = ACCESS_TYPE_UNICAST;
	} else {
		start_pup = 0;
		end_pup = 0;
		pup_access = ACCESS_TYPE_MULTICAST;
	}

	for (cs = 0; cs < max_cs; cs++) {
		reg = (direction == 0) ? WL_PHY_REG(cs) : RL_PHY_REG(cs);
		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				for (pup = start_pup; pup <= end_pup; pup++)
					ctrl_sweepres[adll][if_id][pup] = 0;
			}
		}

		for (adll = 0; adll < MAX_INTERFACE_NUM * MAX_BUS_NUM; adll++) {
			ctrl_adll[adll] = 0;
			ctrl_level_phase[adll] = 0;
			ctrl_adll1[adll] = 0;
		}

		/* save leveling value after running algorithm */
		ddr3_tip_read_adll_value(dev_num, ctrl_adll, reg, 0x1f);
		read_phase_value(dev_num, ctrl_level_phase, reg, 0x7 << 6);

		if (direction == 0)
			ddr3_tip_read_adll_value(dev_num, ctrl_adll1,
						 CTX_PHY_REG(cs), MASK_ALL_BITS);

		/* Sweep ADLL from 0 to 31 on all interfaces, all pups,
		 * and perform BIST on each stage
		 */
		for (pup = start_pup; pup <= end_pup; pup++) {
			for (adll = 0; adll < ADLL_LENGTH; adll++) {
				for (rep = 0; rep < repeat_num; rep++) {
					adll_value = (direction == 0) ? (adll * 2) : (adll * 3);
					for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
						start_adll = ctrl_adll[if_id * cs * octets_per_if_num + pup] +
							     (ctrl_level_phase[if_id * cs *
									     octets_per_if_num +
									     pup] >> 6) * 32;

						if (direction == 0)
							start_adll = (start_adll > 32) ? (start_adll - 32) : 0;
						else
							start_adll = (start_adll > 48) ? (start_adll - 48) : 0;

						adll_value += start_adll;

						gap = ctrl_adll1[if_id * cs * octets_per_if_num + pup] -
						      ctrl_adll[if_id * cs * octets_per_if_num + pup];
						gap = (((adll_value % 32) + gap) % 64);

						adll_value = ((adll_value % 32) +
							       (((adll_value - (adll_value % 32)) / 32) << 6));

						CHECK_STATUS(ddr3_tip_bus_write(dev_num,
										ACCESS_TYPE_UNICAST,
										if_id,
										pup_access,
										pup,
										DDR_PHY_DATA,
										reg,
										adll_value));
						if (direction == 0)
							CHECK_STATUS(ddr3_tip_bus_write(dev_num,
											ACCESS_TYPE_UNICAST,
											if_id,
											pup_access,
											pup,
											DDR_PHY_DATA,
											CTX_PHY_REG(cs),
											gap));
					}

					for (pattern_idx = PATTERN_KILLER_DQ0;
					     pattern_idx < PATTERN_LAST;
					     pattern_idx++) {
						hws_ddr3_run_bist(dev_num, sweep_pattern, res, cs);
						ddr3_tip_reset_fifo_ptr(dev_num);
						for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
							VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
							if (pup != 4) { /* TODO: remove literal */
								ctrl_sweepres[adll][if_id][pup] += res[if_id];
							} else {
								CHECK_STATUS(ddr3_tip_if_read(dev_num,
											      ACCESS_TYPE_UNICAST,
											      if_id,
											      0x1458,
											      read_data,
											      MASK_ALL_BITS));
								ctrl_sweepres[adll][if_id][pup] += read_data[if_id];
								CHECK_STATUS(ddr3_tip_if_write(dev_num,
											       ACCESS_TYPE_UNICAST,
											       if_id,
											       0x1458,
											       0x0,
											       0xFFFFFFFF));
								CHECK_STATUS(ddr3_tip_if_write(dev_num,
											       ACCESS_TYPE_UNICAST,
											       if_id,
											       0x145C,
											       0x0,
											       0xFFFFFFFF));
							}
						}
					}
				}
			}

			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
				start_adll = ctrl_adll[if_id * cs * octets_per_if_num + pup] +
					     ctrl_level_phase[if_id * cs * octets_per_if_num + pup];
				CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST, if_id, pup_access, pup,
								DDR_PHY_DATA, reg, start_adll));
				if (direction == 0)
					CHECK_STATUS(ddr3_tip_bus_write(dev_num,
									ACCESS_TYPE_UNICAST,
									if_id,
									pup_access,
									pup,
									DDR_PHY_DATA,
									CTX_PHY_REG(cs),
									ctrl_adll1[if_id *
										   cs *
										   octets_per_if_num +
										   pup]));
			}
		}

		printf("Final,CS %d,%s,Leveling,Result,Adll,", cs, ((direction == 0) ? "TX" : "RX"));

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			if (mode == 1) {
				for (pup = start_pup; pup <= end_pup; pup++) {
					VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
					printf("I/F%d-PHY%d , ", if_id, pup);
				}
			} else {
				printf("I/F%d , ", if_id);
			}
		}
		printf("\n");

		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			adll_value = (direction == 0) ? ((adll * 2) - 32) : ((adll * 3) - 48);
			printf("Final,%s,LevelingSweep,Result, %d ,", ((direction == 0) ? "TX" : "RX"), adll_value);

			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				for (pup = start_pup; pup <= end_pup; pup++)
					printf("%8d , ", ctrl_sweepres[adll][if_id][pup]);
			}
			printf("\n");
		}

		/* write back to the phy the Rx DQS value, we store in the beginning */
		write_leveling_value(dev_num, ctrl_adll, ctrl_level_phase, reg);
		if (direction == 0)
			ddr3_tip_write_adll_value(dev_num, ctrl_adll1, CTX_PHY_REG(cs));

		/* print adll results */
		ddr3_tip_read_adll_value(dev_num, ctrl_adll, reg, MASK_ALL_BITS);
		printf("%s,DQS,Leveling,,,", (direction == 0) ? "Tx" : "Rx");
		print_adll(dev_num, ctrl_adll);
		print_ph(dev_num, ctrl_level_phase);
	}
	ddr3_tip_reset_fifo_ptr(dev_num);

	return 0;
}
#endif /* EXCLUDE_SWITCH_DEBUG */

void print_topology(struct mv_ddr_topology_map *topology_db)
{
	u32 ui, uj;
	u32 dev_num = 0;

	printf("\tinterface_mask: 0x%x\n", topology_db->if_act_mask);
	printf("\tNumber of buses: 0x%x\n",
	       ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE));
	printf("\tbus_act_mask: 0x%x\n", topology_db->bus_act_mask);

	for (ui = 0; ui < MAX_INTERFACE_NUM; ui++) {
		VALIDATE_IF_ACTIVE(topology_db->if_act_mask, ui);
		printf("\n\tInterface ID: %d\n", ui);
		printf("\t\tDDR Frequency: %s\n",
		       convert_freq(topology_db->
				    interface_params[ui].memory_freq));
		printf("\t\tSpeed_bin: %d\n",
		       topology_db->interface_params[ui].speed_bin_index);
		printf("\t\tBus_width: %d\n",
		       (4 << topology_db->interface_params[ui].bus_width));
		printf("\t\tMem_size: %s\n",
		       convert_mem_size(topology_db->
					interface_params[ui].memory_size));
		printf("\t\tCAS-WL: %d\n",
		       topology_db->interface_params[ui].cas_wl);
		printf("\t\tCAS-L: %d\n",
		       topology_db->interface_params[ui].cas_l);
		printf("\t\tTemperature: %d\n",
		       topology_db->interface_params[ui].interface_temp);
		printf("\n");
		for (uj = 0; uj < 4; uj++) {
			printf("\t\tBus %d parameters- CS Mask: 0x%x\t", uj,
			       topology_db->interface_params[ui].
			       as_bus_params[uj].cs_bitmask);
			printf("Mirror: 0x%x\t",
			       topology_db->interface_params[ui].
			       as_bus_params[uj].mirror_enable_bitmask);
			printf("DQS Swap is %s \t",
			       (topology_db->
				interface_params[ui].as_bus_params[uj].
				is_dqs_swap == 1) ? "enabled" : "disabled");
			printf("Ck Swap:%s\t",
			       (topology_db->
				interface_params[ui].as_bus_params[uj].
				is_ck_swap == 1) ? "enabled" : "disabled");
			printf("\n");
		}
	}
}
#endif /* DDR_VIEWER_TOOL */

#if !defined(EXCLUDE_SWITCH_DEBUG)
/*
 * Execute XSB Test transaction (rd/wr/both)
 */
int run_xsb_test(u32 dev_num, u32 mem_addr, u32 write_type,
		 u32 read_type, u32 burst_length)
{
	u32 seq = 0, if_id = 0, addr, cnt;
	int ret = MV_OK, ret_tmp;
	u32 data_read[MAX_INTERFACE_NUM];
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		addr = mem_addr;
		for (cnt = 0; cnt <= burst_length; cnt++) {
			seq = (seq + 1) % 8;
			if (write_type != 0) {
				CHECK_STATUS(ddr3_tip_ext_write
					     (dev_num, if_id, addr, 1,
					      xsb_test_table[seq]));
			}
			if (read_type != 0) {
				CHECK_STATUS(ddr3_tip_ext_read
					     (dev_num, if_id, addr, 1,
					      data_read));
			}
			if ((read_type != 0) && (write_type != 0)) {
				ret_tmp =
					ddr3_tip_compare(if_id,
							 xsb_test_table[seq],
							 data_read,
							 0xff);
				addr += (EXT_ACCESS_BURST_LENGTH * 4);
				ret = (ret != MV_OK) ? ret : ret_tmp;
			}
		}
	}

	return ret;
}

#else /*EXCLUDE_SWITCH_DEBUG */
u32 start_xsb_offset = 0;

int run_xsb_test(u32 dev_num, u32 mem_addr, u32 write_type,
		 u32 read_type, u32 burst_length)
{
	return MV_OK;
}

#endif /* EXCLUDE_SWITCH_DEBUG */
