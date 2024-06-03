// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_regs.h"
#include "mv_ddr_sys_env_lib.h"

#define DDR_INTERFACES_NUM		1
#define DDR_INTERFACE_OCTETS_NUM	5

/*
 * 1. L2 filter should be set at binary header to 0xD000000,
 *    to avoid conflict with internal register IO.
 * 2. U-Boot modifies internal registers base to 0xf100000,
 *    and than should update L2 filter accordingly to 0xf000000 (3.75 GB)
 */
#define L2_FILTER_FOR_MAX_MEMORY_SIZE	0xC0000000 /* temporary limit l2 filter to 3gb (LSP issue) */
#define ADDRESS_FILTERING_END_REGISTER	0x8c04

#define DYNAMIC_CS_SIZE_CONFIG
#define DISABLE_L2_FILTERING_DURING_DDR_TRAINING

/* Termal Sensor Registers */
#define TSEN_CONTROL_LSB_REG		0xE4070
#define TSEN_CONTROL_LSB_TC_TRIM_OFFSET	0
#define TSEN_CONTROL_LSB_TC_TRIM_MASK	(0x7 << TSEN_CONTROL_LSB_TC_TRIM_OFFSET)
#define TSEN_CONTROL_MSB_REG		0xE4074
#define TSEN_CONTROL_MSB_RST_OFFSET	8
#define TSEN_CONTROL_MSB_RST_MASK	(0x1 << TSEN_CONTROL_MSB_RST_OFFSET)
#define TSEN_STATUS_REG			0xe4078
#define TSEN_STATUS_READOUT_VALID_OFFSET	10
#define TSEN_STATUS_READOUT_VALID_MASK	(0x1 <<				\
					 TSEN_STATUS_READOUT_VALID_OFFSET)
#define TSEN_STATUS_TEMP_OUT_OFFSET	0
#define TSEN_STATUS_TEMP_OUT_MASK	(0x3ff << TSEN_STATUS_TEMP_OUT_OFFSET)

static struct dlb_config ddr3_dlb_config_table[] = {
	{DLB_CTRL_REG, 0x2000005c},
	{DLB_BUS_OPT_WT_REG, 0x00880000},
	{DLB_AGING_REG, 0x0f7f007f},
	{DLB_EVICTION_CTRL_REG, 0x0000129f},
	{DLB_EVICTION_TIMERS_REG, 0x00ff0000},
	{DLB_WTS_DIFF_CS_REG, 0x04030802},
	{DLB_WTS_DIFF_BG_REG, 0x00000a02},
	{DLB_WTS_SAME_BG_REG, 0x09000a01},
	{DLB_WTS_CMDS_REG, 0x00020005},
	{DLB_WTS_ATTR_PRIO_REG, 0x00060f10},
	{DLB_QUEUE_MAP_REG, 0x00000543},
	{DLB_SPLIT_REG, 0x00000000},
	{DLB_USER_CMD_REG, 0x00000000},
	{0x0, 0x0}
};

static struct dlb_config *sys_env_dlb_config_ptr_get(void)
{
	return &ddr3_dlb_config_table[0];
}

static u8 a38x_bw_per_freq[MV_DDR_FREQ_LAST] = {
	0x3,			/* MV_DDR_FREQ_100 */
	0x4,			/* MV_DDR_FREQ_400 */
	0x4,			/* MV_DDR_FREQ_533 */
	0x5,			/* MV_DDR_FREQ_667 */
	0x5,			/* MV_DDR_FREQ_800 */
	0x5,			/* MV_DDR_FREQ_933 */
	0x5,			/* MV_DDR_FREQ_1066 */
	0x3,			/* MV_DDR_FREQ_311 */
	0x3,			/* MV_DDR_FREQ_333 */
	0x4,			/* MV_DDR_FREQ_467 */
	0x5,			/* MV_DDR_FREQ_850 */
	0x5,			/* MV_DDR_FREQ_600 */
	0x3,			/* MV_DDR_FREQ_300 */
	0x5,			/* MV_DDR_FREQ_900 */
	0x3,			/* MV_DDR_FREQ_360 */
	0x5			/* MV_DDR_FREQ_1000 */
};

static u8 a38x_rate_per_freq[MV_DDR_FREQ_LAST] = {
	0x1,			/* MV_DDR_FREQ_100 */
	0x2,			/* MV_DDR_FREQ_400 */
	0x2,			/* MV_DDR_FREQ_533 */
	0x2,			/* MV_DDR_FREQ_667 */
	0x2,			/* MV_DDR_FREQ_800 */
	0x3,			/* MV_DDR_FREQ_933 */
	0x3,			/* MV_DDR_FREQ_1066 */
	0x1,			/* MV_DDR_FREQ_311 */
	0x1,			/* MV_DDR_FREQ_333 */
	0x2,			/* MV_DDR_FREQ_467 */
	0x2,			/* MV_DDR_FREQ_850 */
	0x2,			/* MV_DDR_FREQ_600 */
	0x1,			/* MV_DDR_FREQ_300 */
	0x2,			/* MV_DDR_FREQ_900 */
	0x1,			/* MV_DDR_FREQ_360 */
	0x2			/* MV_DDR_FREQ_1000 */
};

static u16 a38x_vco_freq_per_sar_ref_clk_25_mhz[] = {
	666,			/* 0 */
	1332,
	800,
	1600,
	1066,
	2132,
	1200,
	2400,
	1332,
	1332,
	1500,
	1500,
	1600,			/* 12 */
	1600,
	1700,
	1700,
	1866,
	1866,
	1800,			/* 18 */
	2000,
	2000,
	4000,
	2132,
	2132,
	2300,
	2300,
	2400,
	2400,
	2500,
	2500,
	800
};

static u16 a38x_vco_freq_per_sar_ref_clk_40_mhz[] = {
	666,			/* 0 */
	1332,
	800,
	800,			/* 0x3 */
	1066,
	1066,			/* 0x5 */
	1200,
	2400,
	1332,
	1332,
	1500,			/* 10 */
	1600,			/* 0xB */
	1600,
	1600,
	1700,
	1560,			/* 0xF */
	1866,
	1866,
	1800,
	2000,
	2000,			/* 20 */
	4000,
	2132,
	2132,
	2300,
	2300,
	2400,
	2400,
	2500,
	2500,
	1800			/* 30 - 0x1E */
};


static u32 async_mode_at_tf;

static u32 dq_bit_map_2_phy_pin[] = {
	1, 0, 2, 6, 9, 8, 3, 7,	/* 0 */
	8, 9, 1, 7, 2, 6, 3, 0,	/* 1 */
	3, 9, 7, 8, 1, 0, 2, 6,	/* 2 */
	1, 0, 6, 2, 8, 3, 7, 9,	/* 3 */
	0, 1, 2, 9, 7, 8, 3, 6,	/* 4 */
};

void mv_ddr_mem_scrubbing(void)
{
	ddr3_new_tip_ecc_scrub();
}

static int ddr3_tip_a38x_set_divider(u8 dev_num, u32 if_id,
				     enum mv_ddr_freq freq);

/*
 * Read temperature TJ value
 */
static u32 ddr3_ctrl_get_junc_temp(u8 dev_num)
{
	int reg = 0;

	/* Initiates TSEN hardware reset once */
	if ((reg_read(TSEN_CONTROL_MSB_REG) & TSEN_CONTROL_MSB_RST_MASK) == 0) {
		reg_bit_set(TSEN_CONTROL_MSB_REG, TSEN_CONTROL_MSB_RST_MASK);
		/* set Tsen Tc Trim to correct default value (errata #132698) */
		reg = reg_read(TSEN_CONTROL_LSB_REG);
		reg &= ~TSEN_CONTROL_LSB_TC_TRIM_MASK;
		reg |= 0x3 << TSEN_CONTROL_LSB_TC_TRIM_OFFSET;
		reg_write(TSEN_CONTROL_LSB_REG, reg);
	}
	mdelay(10);

	/* Check if the readout field is valid */
	if ((reg_read(TSEN_STATUS_REG) & TSEN_STATUS_READOUT_VALID_MASK) == 0) {
		printf("%s: TSEN not ready\n", __func__);
		return 0;
	}

	reg = reg_read(TSEN_STATUS_REG);
	reg = (reg & TSEN_STATUS_TEMP_OUT_MASK) >> TSEN_STATUS_TEMP_OUT_OFFSET;

	return ((((10000 * reg) / 21445) * 1000) - 272674) / 1000;
}

/*
 * Name:     ddr3_tip_a38x_get_freq_config.
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_tip_a38x_get_freq_config(u8 dev_num, enum mv_ddr_freq freq,
				  struct hws_tip_freq_config_info
				  *freq_config_info)
{
	if (a38x_bw_per_freq[freq] == 0xff)
		return MV_NOT_SUPPORTED;

	if (freq_config_info == NULL)
		return MV_BAD_PARAM;

	freq_config_info->bw_per_freq = a38x_bw_per_freq[freq];
	freq_config_info->rate_per_freq = a38x_rate_per_freq[freq];
	freq_config_info->is_supported = 1;

	return MV_OK;
}

static void dunit_read(u32 addr, u32 mask, u32 *data)
{
	*data = reg_read(addr) & mask;
}

static void dunit_write(u32 addr, u32 mask, u32 data)
{
	u32 reg_val = data;

	if (mask != MASK_ALL_BITS) {
		dunit_read(addr, MASK_ALL_BITS, &reg_val);
		reg_val &= (~mask);
		reg_val |= (data & mask);
	}

	reg_write(addr, reg_val);
}

#define ODPG_ENABLE_REG				0x186d4
#define ODPG_EN_OFFS				0
#define ODPG_EN_MASK				0x1
#define ODPG_EN_ENA				1
#define ODPG_EN_DONE				0
#define ODPG_DIS_OFFS				8
#define ODPG_DIS_MASK				0x1
#define ODPG_DIS_DIS				1
void mv_ddr_odpg_enable(void)
{
	dunit_write(ODPG_ENABLE_REG,
		    ODPG_EN_MASK << ODPG_EN_OFFS,
		    ODPG_EN_ENA << ODPG_EN_OFFS);
}

void mv_ddr_odpg_disable(void)
{
	dunit_write(ODPG_ENABLE_REG,
		    ODPG_DIS_MASK << ODPG_DIS_OFFS,
		    ODPG_DIS_DIS << ODPG_DIS_OFFS);
}

void mv_ddr_odpg_done_clr(void)
{
	return;
}

int mv_ddr_is_odpg_done(u32 count)
{
	u32 i, data;

	for (i = 0; i < count; i++) {
		dunit_read(ODPG_ENABLE_REG, MASK_ALL_BITS, &data);
		if (((data >> ODPG_EN_OFFS) & ODPG_EN_MASK) ==
		     ODPG_EN_DONE)
			break;
	}

	if (i >= count) {
		printf("%s: timeout\n", __func__);
		return MV_FAIL;
	}

	return MV_OK;
}

void mv_ddr_training_enable(void)
{
	dunit_write(GLOB_CTRL_STATUS_REG,
		    TRAINING_TRIGGER_MASK << TRAINING_TRIGGER_OFFS,
		    TRAINING_TRIGGER_ENA << TRAINING_TRIGGER_OFFS);
}

#define DRAM_INIT_CTRL_STATUS_REG	0x18488
#define TRAINING_TRIGGER_OFFS		0
#define TRAINING_TRIGGER_MASK		0x1
#define TRAINING_TRIGGER_ENA		1
#define TRAINING_DONE_OFFS		1
#define TRAINING_DONE_MASK		0x1
#define TRAINING_DONE_DONE		1
#define TRAINING_DONE_NOT_DONE		0
#define TRAINING_RESULT_OFFS		2
#define TRAINING_RESULT_MASK		0x1
#define TRAINING_RESULT_PASS		0
#define TRAINING_RESULT_FAIL		1
int mv_ddr_is_training_done(u32 count, u32 *result)
{
	u32 i, data;

	if (result == NULL) {
		printf("%s: NULL result pointer found\n", __func__);
		return MV_FAIL;
	}

	for (i = 0; i < count; i++) {
		dunit_read(DRAM_INIT_CTRL_STATUS_REG, MASK_ALL_BITS, &data);
		if (((data >> TRAINING_DONE_OFFS) & TRAINING_DONE_MASK) ==
		     TRAINING_DONE_DONE)
			break;
	}

	if (i >= count) {
		printf("%s: timeout\n", __func__);
		return MV_FAIL;
	}

	*result = (data >> TRAINING_RESULT_OFFS) & TRAINING_RESULT_MASK;

	return MV_OK;
}

#define DM_PAD	10
u32 mv_ddr_dm_pad_get(void)
{
	return DM_PAD;
}

/*
 * Name:     ddr3_tip_a38x_select_ddr_controller.
 * Desc:     Enable/Disable access to Marvell's server.
 * Args:     dev_num     - device number
 *           enable        - whether to enable or disable the server
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_tip_a38x_select_ddr_controller(u8 dev_num, int enable)
{
	u32 reg;

	reg = reg_read(DUAL_DUNIT_CFG_REG);

	if (enable)
		reg |= (1 << 6);
	else
		reg &= ~(1 << 6);

	reg_write(DUAL_DUNIT_CFG_REG, reg);

	return MV_OK;
}

static u8 ddr3_tip_clock_mode(u32 frequency)
{
	if ((frequency == MV_DDR_FREQ_LOW_FREQ) || (mv_ddr_freq_get(frequency) <= 400))
		return 1;

	return 2;
}

static int mv_ddr_sar_freq_get(int dev_num, enum mv_ddr_freq *freq)
{
	u32 reg, ref_clk_satr;

	/* Read sample at reset setting */
	reg = (reg_read(REG_DEVICE_SAR1_ADDR) >>
	       RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
		RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;

	ref_clk_satr = reg_read(DEVICE_SAMPLE_AT_RESET2_REG);
	if (((ref_clk_satr >> DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_OFFSET) & 0x1) ==
	    DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_25MHZ) {
		switch (reg) {
		case 0x1:
			DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
					      ("Warning: Unsupported freq mode for 333Mhz configured(%d)\n",
					      reg));
			/* fallthrough */
		case 0x0:
			*freq = MV_DDR_FREQ_333;
			break;
		case 0x3:
			DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
					      ("Warning: Unsupported freq mode for 400Mhz configured(%d)\n",
					      reg));
			/* fallthrough */
		case 0x2:
			*freq = MV_DDR_FREQ_400;
			break;
		case 0xd:
			DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
					      ("Warning: Unsupported freq mode for 533Mhz configured(%d)\n",
					      reg));
			/* fallthrough */
		case 0x4:
			*freq = MV_DDR_FREQ_533;
			break;
		case 0x6:
			*freq = MV_DDR_FREQ_600;
			break;
		case 0x11:
		case 0x14:
			DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
					      ("Warning: Unsupported freq mode for 667Mhz configured(%d)\n",
					      reg));
			/* fallthrough */
		case 0x8:
			*freq = MV_DDR_FREQ_667;
			break;
		case 0x15:
		case 0x1b:
			DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
					      ("Warning: Unsupported freq mode for 800Mhz configured(%d)\n",
					      reg));
			/* fallthrough */
		case 0xc:
			*freq = MV_DDR_FREQ_800;
			break;
		case 0x10:
			*freq = MV_DDR_FREQ_933;
			break;
		case 0x12:
			*freq = MV_DDR_FREQ_900;
			break;
		case 0x13:
			*freq = MV_DDR_FREQ_933;
			break;
		default:
			*freq = 0;
			return MV_NOT_SUPPORTED;
		}
	} else { /* REFCLK 40MHz case */
		switch (reg) {
		case 0x3:
			*freq = MV_DDR_FREQ_400;
			break;
		case 0x5:
			*freq = MV_DDR_FREQ_533;
			break;
		case 0xb:
			*freq = MV_DDR_FREQ_800;
			break;
		case 0x1e:
			*freq = MV_DDR_FREQ_900;
			break;
		default:
			*freq = 0;
			return MV_NOT_SUPPORTED;
		}
	}

	return MV_OK;
}

static int ddr3_tip_a38x_get_medium_freq(int dev_num, enum mv_ddr_freq *freq)
{
	u32 reg, ref_clk_satr;

	/* Read sample at reset setting */
	reg = (reg_read(REG_DEVICE_SAR1_ADDR) >>
	RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
	RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;

	ref_clk_satr = reg_read(DEVICE_SAMPLE_AT_RESET2_REG);
	if (((ref_clk_satr >> DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_OFFSET) & 0x1) ==
	    DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_25MHZ) {
		switch (reg) {
		case 0x0:
		case 0x1:
			/* Medium is same as TF to run PBS in this freq */
			*freq = MV_DDR_FREQ_333;
			break;
		case 0x2:
		case 0x3:
			/* Medium is same as TF to run PBS in this freq */
			*freq = MV_DDR_FREQ_400;
			break;
		case 0x4:
		case 0xd:
			/* Medium is same as TF to run PBS in this freq */
			*freq = MV_DDR_FREQ_533;
			break;
		case 0x8:
		case 0x10:
		case 0x11:
		case 0x14:
			*freq = MV_DDR_FREQ_333;
			break;
		case 0xc:
		case 0x15:
		case 0x1b:
			*freq = MV_DDR_FREQ_400;
			break;
		case 0x6:
			*freq = MV_DDR_FREQ_300;
			break;
		case 0x12:
			*freq = MV_DDR_FREQ_360;
			break;
		case 0x13:
			*freq = MV_DDR_FREQ_400;
			break;
		default:
			*freq = 0;
			return MV_NOT_SUPPORTED;
		}
	} else { /* REFCLK 40MHz case */
		switch (reg) {
		case 0x3:
			/* Medium is same as TF to run PBS in this freq */
			*freq = MV_DDR_FREQ_400;
			break;
		case 0x5:
			/* Medium is same as TF to run PBS in this freq */
			*freq = MV_DDR_FREQ_533;
			break;
		case 0xb:
			*freq = MV_DDR_FREQ_400;
			break;
		case 0x1e:
			*freq = MV_DDR_FREQ_360;
			break;
		default:
			*freq = 0;
			return MV_NOT_SUPPORTED;
		}
	}

	return MV_OK;
}

static int ddr3_tip_a38x_get_device_info(u8 dev_num, struct ddr3_device_info *info_ptr)
{
#if defined(CONFIG_ARMADA_39X)
	info_ptr->device_id = 0x6900;
#else
	info_ptr->device_id = 0x6800;
#endif
	info_ptr->ck_delay = ck_delay;

	return MV_OK;
}

/* check indirect access to phy register file completed */
static int is_prfa_done(void)
{
	u32 reg_val;
	u32 iter = 0;

	do {
		if (iter++ > MAX_POLLING_ITERATIONS) {
			printf("error: %s: polling timeout\n", __func__);
			return MV_FAIL;
		}
		dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
		reg_val >>= PRFA_REQ_OFFS;
		reg_val &= PRFA_REQ_MASK;
	} while (reg_val == PRFA_REQ_ENA); /* request pending */

	return MV_OK;
}

/* write to phy register thru indirect access */
static int prfa_write(enum hws_access_type phy_access, u32 phy,
		      enum hws_ddr_phy phy_type, u32 addr,
		      u32 data, enum hws_operation op_type)
{
	u32 reg_val = ((data & PRFA_DATA_MASK) << PRFA_DATA_OFFS) |
		      ((addr & PRFA_REG_NUM_MASK) << PRFA_REG_NUM_OFFS) |
		      ((phy & PRFA_PUP_NUM_MASK) << PRFA_PUP_NUM_OFFS) |
		      ((phy_type & PRFA_PUP_CTRL_DATA_MASK) << PRFA_PUP_CTRL_DATA_OFFS) |
		      ((phy_access & PRFA_PUP_BCAST_WR_ENA_MASK) << PRFA_PUP_BCAST_WR_ENA_OFFS) |
		      (((addr >> 6) & PRFA_REG_NUM_HI_MASK) << PRFA_REG_NUM_HI_OFFS) |
		      ((op_type & PRFA_TYPE_MASK) << PRFA_TYPE_OFFS);
	dunit_write(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, reg_val);
	reg_val |= (PRFA_REQ_ENA << PRFA_REQ_OFFS);
	dunit_write(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, reg_val);

	/* polling for prfa request completion */
	if (is_prfa_done() != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/* read from phy register thru indirect access */
static int prfa_read(enum hws_access_type phy_access, u32 phy,
		     enum hws_ddr_phy phy_type, u32 addr, u32 *data)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 max_phy = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 i, reg_val;

	if (phy_access == ACCESS_TYPE_MULTICAST) {
		for (i = 0; i < max_phy; i++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
			if (prfa_write(ACCESS_TYPE_UNICAST, i, phy_type, addr, 0, OPERATION_READ) != MV_OK)
				return MV_FAIL;
			dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
			data[i] = (reg_val >> PRFA_DATA_OFFS) & PRFA_DATA_MASK;
		}
	} else {
		if (prfa_write(phy_access, phy, phy_type, addr, 0, OPERATION_READ) != MV_OK)
			return MV_FAIL;
		dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
		*data = (reg_val >> PRFA_DATA_OFFS) & PRFA_DATA_MASK;
	}

	return MV_OK;
}

static int mv_ddr_sw_db_init(u32 dev_num, u32 board_id)
{
	struct hws_tip_config_func_db config_func;

	/* new read leveling version */
	config_func.mv_ddr_dunit_read = dunit_read;
	config_func.mv_ddr_dunit_write = dunit_write;
	config_func.tip_dunit_mux_select_func =
		ddr3_tip_a38x_select_ddr_controller;
	config_func.tip_get_freq_config_info_func =
		ddr3_tip_a38x_get_freq_config;
	config_func.tip_set_freq_divider_func = ddr3_tip_a38x_set_divider;
	config_func.tip_get_device_info_func = ddr3_tip_a38x_get_device_info;
	config_func.tip_get_temperature = ddr3_ctrl_get_junc_temp;
	config_func.tip_get_clock_ratio = ddr3_tip_clock_mode;
	config_func.tip_external_read = ddr3_tip_ext_read;
	config_func.tip_external_write = ddr3_tip_ext_write;
	config_func.mv_ddr_phy_read = prfa_read;
	config_func.mv_ddr_phy_write = prfa_write;

	ddr3_tip_init_config_func(dev_num, &config_func);

	ddr3_tip_register_dq_table(dev_num, dq_bit_map_2_phy_pin);

	/* set device attributes*/
	ddr3_tip_dev_attr_init(dev_num);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_TIP_REV, MV_TIP_REV_4);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_PHY_EDGE, MV_DDR_PHY_EDGE_POSITIVE);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_OCTET_PER_INTERFACE, DDR_INTERFACE_OCTETS_NUM);
#ifdef CONFIG_ARMADA_39X
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_INTERLEAVE_WA, 1);
#else
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_INTERLEAVE_WA, 0);
#endif

	ca_delay = 0;
	delay_enable = 1;
	dfs_low_freq = DFS_LOW_FREQ_VALUE;
	calibration_update_control = 1;

	ddr3_tip_a38x_get_medium_freq(dev_num, &medium_freq);

	return MV_OK;
}

static int mv_ddr_training_mask_set(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq ddr_freq = tm->interface_params[0].memory_freq;

	mask_tune_func = (SET_LOW_FREQ_MASK_BIT |
			  LOAD_PATTERN_MASK_BIT |
			  SET_MEDIUM_FREQ_MASK_BIT | WRITE_LEVELING_MASK_BIT |
			  WRITE_LEVELING_SUPP_MASK_BIT |
			  READ_LEVELING_MASK_BIT |
			  PBS_RX_MASK_BIT |
			  PBS_TX_MASK_BIT |
			  SET_TARGET_FREQ_MASK_BIT |
			  WRITE_LEVELING_TF_MASK_BIT |
			  WRITE_LEVELING_SUPP_TF_MASK_BIT |
			  READ_LEVELING_TF_MASK_BIT |
			  CENTRALIZATION_RX_MASK_BIT |
			  CENTRALIZATION_TX_MASK_BIT);
	rl_mid_freq_wa = 1;

	if ((ddr_freq == MV_DDR_FREQ_333) || (ddr_freq == MV_DDR_FREQ_400)) {
		mask_tune_func = (WRITE_LEVELING_MASK_BIT |
				  LOAD_PATTERN_2_MASK_BIT |
				  WRITE_LEVELING_SUPP_MASK_BIT |
				  READ_LEVELING_MASK_BIT |
				  PBS_RX_MASK_BIT |
				  PBS_TX_MASK_BIT |
				  CENTRALIZATION_RX_MASK_BIT |
				  CENTRALIZATION_TX_MASK_BIT);
		rl_mid_freq_wa = 0; /* WA not needed if 333/400 is TF */
	}

	/* Supplementary not supported for ECC modes */
	if (mv_ddr_is_ecc_ena()) {
		mask_tune_func &= ~WRITE_LEVELING_SUPP_TF_MASK_BIT;
		mask_tune_func &= ~WRITE_LEVELING_SUPP_MASK_BIT;
		mask_tune_func &= ~PBS_TX_MASK_BIT;
		mask_tune_func &= ~PBS_RX_MASK_BIT;
	}

	return MV_OK;
}

/* function: mv_ddr_set_calib_controller
 * this function sets the controller which will control
 * the calibration cycle in the end of the training.
 * 1 - internal controller
 * 2 - external controller
 */
void mv_ddr_set_calib_controller(void)
{
	calibration_update_control = CAL_UPDATE_CTRL_INT;
}

static int ddr3_tip_a38x_set_divider(u8 dev_num, u32 if_id,
				     enum mv_ddr_freq frequency)
{
	u32 divider = 0;
	u32 sar_val, ref_clk_satr;
	u32 async_val;
	u32 freq = mv_ddr_freq_get(frequency);

	if (if_id != 0) {
		DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
				      ("A38x does not support interface 0x%x\n",
				       if_id));
		return MV_BAD_PARAM;
	}

	/* get VCO freq index */
	sar_val = (reg_read(REG_DEVICE_SAR1_ADDR) >>
		   RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
		RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;

	ref_clk_satr = reg_read(DEVICE_SAMPLE_AT_RESET2_REG);
	if (((ref_clk_satr >> DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_OFFSET) & 0x1) ==
	    DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_25MHZ)
		divider = a38x_vco_freq_per_sar_ref_clk_25_mhz[sar_val] / freq;
	else
		divider = a38x_vco_freq_per_sar_ref_clk_40_mhz[sar_val] / freq;

	if ((async_mode_at_tf == 1) && (freq > 400)) {
		/* Set async mode */
		dunit_write(0x20220, 0x1000, 0x1000);
		dunit_write(0xe42f4, 0x200, 0x200);

		/* Wait for async mode setup */
		mdelay(5);

		/* Set KNL values */
		switch (frequency) {
		case MV_DDR_FREQ_467:
			async_val = 0x806f012;
			break;
		case MV_DDR_FREQ_533:
			async_val = 0x807f012;
			break;
		case MV_DDR_FREQ_600:
			async_val = 0x805f00a;
			break;
		case MV_DDR_FREQ_667:
			async_val = 0x809f012;
			break;
		case MV_DDR_FREQ_800:
			async_val = 0x807f00a;
			break;
		case MV_DDR_FREQ_850:
			async_val = 0x80cb012;
			break;
		case MV_DDR_FREQ_900:
			async_val = 0x80d7012;
			break;
		case MV_DDR_FREQ_933:
			async_val = 0x80df012;
			break;
		case MV_DDR_FREQ_1000:
			async_val = 0x80ef012;
			break;
		case MV_DDR_FREQ_1066:
			async_val = 0x80ff012;
			break;
		default:
			/* set MV_DDR_FREQ_667 as default */
			async_val = 0x809f012;
		}
		dunit_write(0xe42f0, 0xffffffff, async_val);
	} else {
		/* Set sync mode */
		dunit_write(0x20220, 0x1000, 0x0);
		dunit_write(0xe42f4, 0x200, 0x0);

		/* cpupll_clkdiv_reset_mask */
		dunit_write(0xe4264, 0xff, 0x1f);

		/* cpupll_clkdiv_reload_smooth */
		dunit_write(0xe4260, (0xff << 8), (0x2 << 8));

		/* cpupll_clkdiv_relax_en */
		dunit_write(0xe4260, (0xff << 24), (0x2 << 24));

		/* write the divider */
		dunit_write(0xe4268, (0x3f << 8), (divider << 8));

		/* set cpupll_clkdiv_reload_ratio */
		dunit_write(0xe4264, (1 << 8), (1 << 8));

		/* undet cpupll_clkdiv_reload_ratio */
		dunit_write(0xe4264, (1 << 8), 0x0);

		/* clear cpupll_clkdiv_reload_force */
		dunit_write(0xe4260, (0xff << 8), 0x0);

		/* clear cpupll_clkdiv_relax_en */
		dunit_write(0xe4260, (0xff << 24), 0x0);

		/* clear cpupll_clkdiv_reset_mask */
		dunit_write(0xe4264, 0xff, 0x0);
	}

	/* Dunit training clock + 1:1/2:1 mode */
	dunit_write(0x18488, (1 << 16), ((ddr3_tip_clock_mode(frequency) & 0x1) << 16));
	dunit_write(0x1524, (1 << 15), ((ddr3_tip_clock_mode(frequency) - 1) << 15));

	return MV_OK;
}

/*
 * external read from memory
 */
int ddr3_tip_ext_read(u32 dev_num, u32 if_id, u32 reg_addr,
		      u32 num_of_bursts, u32 *data)
{
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		data[burst_num] = readl(reg_addr + 4 * burst_num);

	return MV_OK;
}

/*
 * external write to memory
 */
int ddr3_tip_ext_write(u32 dev_num, u32 if_id, u32 reg_addr,
		       u32 num_of_bursts, u32 *data) {
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		writel(data[burst_num], reg_addr + 4 * burst_num);

	return MV_OK;
}

int mv_ddr_early_init(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* FIXME: change this configuration per ddr type
	 * configure a380 and a390 to work with receiver odt timing
	 * the odt_config is defined:
	 * '1' in ddr4
	 * '0' in ddr3
	 * here the parameter is run over in ddr4 and ddr3 to '1' (in ddr4 the default is '1')
	 * to configure the odt to work with timing restrictions
	 */

	mv_ddr_sw_db_init(0, 0);

	if (tm->interface_params[0].memory_freq != MV_DDR_FREQ_SAR)
		async_mode_at_tf = 1;

	return MV_OK;
}

int mv_ddr_early_init2(void)
{
	mv_ddr_training_mask_set();

	return MV_OK;
}

int mv_ddr_pre_training_fixup(void)
{
	return 0;
}

int mv_ddr_post_training_fixup(void)
{
	return 0;
}

int ddr3_post_run_alg(void)
{
	return MV_OK;
}

int ddr3_silicon_post_init(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* Set half bus width */
	if (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask)) {
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      SDRAM_CFG_REG, 0x0, 0x8000));
	}

	return MV_OK;
}

u32 mv_ddr_init_freq_get(void)
{
	enum mv_ddr_freq freq;

	mv_ddr_sar_freq_get(0, &freq);

	return freq;
}

static u32 ddr3_get_bus_width(void)
{
	u32 bus_width;

	bus_width = (reg_read(SDRAM_CFG_REG) & 0x8000) >>
		BUS_IN_USE_OFFS;

	return (bus_width == 0) ? 16 : 32;
}

static u32 ddr3_get_device_width(u32 cs)
{
	u32 device_width;

	device_width = (reg_read(SDRAM_ADDR_CTRL_REG) &
			(CS_STRUCT_MASK << CS_STRUCT_OFFS(cs))) >>
			CS_STRUCT_OFFS(cs);

	return (device_width == 0) ? 8 : 16;
}

static u32 ddr3_get_device_size(u32 cs)
{
	u32 device_size_low, device_size_high, device_size;
	u32 data, cs_low_offset, cs_high_offset;

	cs_low_offset = CS_SIZE_OFFS(cs);
	cs_high_offset = CS_SIZE_HIGH_OFFS(cs);

	data = reg_read(SDRAM_ADDR_CTRL_REG);
	device_size_low = (data >> cs_low_offset) & 0x3;
	device_size_high = (data >> cs_high_offset) & 0x1;

	device_size = device_size_low | (device_size_high << 2);

	switch (device_size) {
	case 0:
		return 2048;
	case 2:
		return 512;
	case 3:
		return 1024;
	case 4:
		return 4096;
	case 5:
		return 8192;
	case 1:
	default:
		DEBUG_INIT_C("Error: Wrong device size of Cs: ", cs, 1);
		/* zeroes mem size in ddr3_calc_mem_cs_size */
		return 0;
	}
}

int ddr3_calc_mem_cs_size(u32 cs, uint64_t *cs_size)
{
	u32 cs_mem_size;

	/* Calculate in MiB */
	cs_mem_size = ((ddr3_get_bus_width() / ddr3_get_device_width(cs)) *
		       ddr3_get_device_size(cs)) / 8;

	/*
	 * Multiple controller bus width, 2x for 64 bit
	 * (SoC controller may be 32 or 64 bit,
	 * so bit 15 in 0x1400, that means if whole bus used or only half,
	 * have a differnt meaning
	 */
	cs_mem_size *= DDR_CONTROLLER_BUS_WIDTH_MULTIPLIER;

	if ((cs_mem_size < 128) || (cs_mem_size > 4096)) {
		DEBUG_INIT_C("Error: Wrong Memory size of Cs: ", cs, 1);
		return MV_BAD_VALUE;
	}

	*cs_size = cs_mem_size << 20; /* write cs size in bytes */

	return MV_OK;
}

static int ddr3_fast_path_dynamic_cs_size_config(u32 cs_ena)
{
	u32 reg, cs;
	uint64_t mem_total_size = 0;
	uint64_t cs_mem_size = 0;
	uint64_t mem_total_size_c, cs_mem_size_c;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	u32 physical_mem_size;
	u32 max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
#endif

	/* Open fast path windows */
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		if (cs_ena & (1 << cs)) {
			/* get CS size */
			if (ddr3_calc_mem_cs_size(cs, &cs_mem_size) != MV_OK)
				return MV_FAIL;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
			/*
			 * if number of address pins doesn't allow to use max
			 * mem size that is defined in topology
			 * mem size is defined by DEVICE_MAX_DRAM_ADDRESS_SIZE
			 */
			physical_mem_size = mem_size
				[tm->interface_params[0].memory_size];

			if (ddr3_get_device_width(cs) == 16) {
				/*
				 * 16bit mem device can be twice more - no need
				 * in less significant pin
				 */
				max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE * 2;
			}

			if (physical_mem_size > max_mem_size) {
				cs_mem_size = max_mem_size *
					(ddr3_get_bus_width() /
					 ddr3_get_device_width(cs));
				printf("Updated Physical Mem size is from 0x%x to %x\n",
				       physical_mem_size,
				       DEVICE_MAX_DRAM_ADDRESS_SIZE);
			}
#endif

			/* set fast path window control for the cs */
			reg = 0xffffe1;
			reg |= (cs << 2);
			reg |= (cs_mem_size - 1) & 0xffff0000;
			/*Open fast path Window */
			reg_write(REG_FASTPATH_WIN_CTRL_ADDR(cs), reg);

			/* Set fast path window base address for the cs */
			reg = ((cs_mem_size) * cs) & 0xffff0000;
			/* Set base address */
			reg_write(REG_FASTPATH_WIN_BASE_ADDR(cs), reg);

			/*
			 * Since memory size may be bigger than 4G the summ may
			 * be more than 32 bit word,
			 * so to estimate the result divide mem_total_size and
			 * cs_mem_size by 0x10000 (it is equal to >> 16)
			 */
			mem_total_size_c = (mem_total_size >> 16) & 0xffffffffffff;
			cs_mem_size_c = (cs_mem_size >> 16) & 0xffffffffffff;
			/* if the sum less than 2 G - calculate the value */
			if (mem_total_size_c + cs_mem_size_c < 0x10000)
				mem_total_size += cs_mem_size;
			else	/* put max possible size */
				mem_total_size = L2_FILTER_FOR_MAX_MEMORY_SIZE;
		}
	}

	/* Set L2 filtering to Max Memory size */
	reg_write(ADDRESS_FILTERING_END_REGISTER, mem_total_size);

	return MV_OK;
}

static int ddr3_restore_and_set_final_windows(u32 *win, const char *ddr_type)
{
	u32 win_ctrl_reg, num_of_win_regs;
	u32 cs_ena = mv_ddr_sys_env_get_cs_ena_from_reg();
	u32 ui;

	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	num_of_win_regs = 16;

	/* Return XBAR windows 4-7 or 16-19 init configuration */
	for (ui = 0; ui < num_of_win_regs; ui++)
		reg_write((win_ctrl_reg + 0x4 * ui), win[ui]);

	printf("%s Training Sequence - Switching XBAR Window to FastPath Window\n",
	       ddr_type);

#if defined DYNAMIC_CS_SIZE_CONFIG
	if (ddr3_fast_path_dynamic_cs_size_config(cs_ena) != MV_OK)
		printf("ddr3_fast_path_dynamic_cs_size_config FAILED\n");
#else
	u32 reg, cs;
	reg = 0x1fffffe1;
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		if (cs_ena & (1 << cs)) {
			reg |= (cs << 2);
			break;
		}
	}
	/* Open fast path Window to - 0.5G */
	reg_write(REG_FASTPATH_WIN_CTRL_ADDR(0), reg);
#endif

	return MV_OK;
}

static int ddr3_save_and_set_training_windows(u32 *win)
{
	u32 cs_ena;
	u32 reg, tmp_count, cs, ui;
	u32 win_ctrl_reg, win_base_reg, win_remap_reg;
	u32 num_of_win_regs, win_jump_index;
	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	win_base_reg = REG_XBAR_WIN_4_BASE_ADDR;
	win_remap_reg = REG_XBAR_WIN_4_REMAP_ADDR;
	win_jump_index = 0x10;
	num_of_win_regs = 16;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

#ifdef DISABLE_L2_FILTERING_DURING_DDR_TRAINING
	/*
	 * Disable L2 filtering during DDR training
	 * (when Cross Bar window is open)
	 */
	reg_write(ADDRESS_FILTERING_END_REGISTER, 0);
#endif

	cs_ena = tm->interface_params[0].as_bus_params[0].cs_bitmask;

	/* Close XBAR Window 19 - Not needed */
	/* {0x000200e8}  -   Open Mbus Window - 2G */
	reg_write(REG_XBAR_WIN_19_CTRL_ADDR, 0);

	/* Save XBAR Windows 4-19 init configurations */
	for (ui = 0; ui < num_of_win_regs; ui++)
		win[ui] = reg_read(win_ctrl_reg + 0x4 * ui);

	/* Open XBAR Windows 4-7 or 16-19 for other CS */
	reg = 0;
	tmp_count = 0;
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		if (cs_ena & (1 << cs)) {
			switch (cs) {
			case 0:
				reg = 0x0e00;
				break;
			case 1:
				reg = 0x0d00;
				break;
			case 2:
				reg = 0x0b00;
				break;
			case 3:
				reg = 0x0700;
				break;
			}
			reg |= (1 << 0);
			reg |= (SDRAM_CS_SIZE & 0xffff0000);

			reg_write(win_ctrl_reg + win_jump_index * tmp_count,
				  reg);
			reg = (((SDRAM_CS_SIZE + 1) * (tmp_count)) &
			       0xffff0000);
			reg_write(win_base_reg + win_jump_index * tmp_count,
				  reg);

			if (win_remap_reg <= REG_XBAR_WIN_7_REMAP_ADDR)
				reg_write(win_remap_reg +
					  win_jump_index * tmp_count, 0);

			tmp_count++;
		}
	}

	return MV_OK;
}

static u32 win[16];

int mv_ddr_pre_training_soc_config(const char *ddr_type)
{
	u32 soc_num;
	u32 reg_val;

	/* Switching CPU to MRVL ID */
	soc_num = (reg_read(REG_SAMPLE_RESET_HIGH_ADDR) & SAR1_CPU_CORE_MASK) >>
		SAR1_CPU_CORE_OFFSET;
	switch (soc_num) {
	case 0x3:
		reg_bit_set(CPU_CONFIGURATION_REG(3), CPU_MRVL_ID_OFFSET);
		reg_bit_set(CPU_CONFIGURATION_REG(2), CPU_MRVL_ID_OFFSET);
		/* fallthrough */
	case 0x1:
		reg_bit_set(CPU_CONFIGURATION_REG(1), CPU_MRVL_ID_OFFSET);
		/* fallthrough */
	case 0x0:
		reg_bit_set(CPU_CONFIGURATION_REG(0), CPU_MRVL_ID_OFFSET);
		/* fallthrough */
	default:
		break;
	}

	/*
	 * Set DRAM Reset Mask in case detected GPIO indication of wakeup from
	 * suspend i.e the DRAM values will not be overwritten / reset when
	 * waking from suspend
	 */
	if (mv_ddr_sys_env_suspend_wakeup_check() ==
	    SUSPEND_WAKEUP_ENABLED_GPIO_DETECTED) {
		reg_bit_set(SDRAM_INIT_CTRL_REG,
			    DRAM_RESET_MASK_MASKED << DRAM_RESET_MASK_OFFS);
	}

	/* Check if DRAM is already initialized  */
	if (reg_read(REG_BOOTROM_ROUTINE_ADDR) &
	    (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS)) {
		printf("%s Training Sequence - 2nd boot - Skip\n", ddr_type);
		return MV_OK;
	}

	/* Fix read ready phases for all SOC in reg 0x15c8 */
	reg_val = reg_read(TRAINING_DBG_3_REG);

	reg_val &= ~(TRN_DBG_RDY_INC_PH_2TO1_MASK << TRN_DBG_RDY_INC_PH_2TO1_OFFS(0));
	reg_val |= (0x4 << TRN_DBG_RDY_INC_PH_2TO1_OFFS(0));	/* phase 0 */

	reg_val &= ~(TRN_DBG_RDY_INC_PH_2TO1_MASK << TRN_DBG_RDY_INC_PH_2TO1_OFFS(1));
	reg_val |= (0x4 << TRN_DBG_RDY_INC_PH_2TO1_OFFS(1));	/* phase 1 */

	reg_val &= ~(TRN_DBG_RDY_INC_PH_2TO1_MASK << TRN_DBG_RDY_INC_PH_2TO1_OFFS(3));
	reg_val |= (0x6 << TRN_DBG_RDY_INC_PH_2TO1_OFFS(3));	/* phase 3 */

	reg_val &= ~(TRN_DBG_RDY_INC_PH_2TO1_MASK << TRN_DBG_RDY_INC_PH_2TO1_OFFS(4));
	reg_val |= (0x6 << TRN_DBG_RDY_INC_PH_2TO1_OFFS(4));	/* phase 4 */

	reg_val &= ~(TRN_DBG_RDY_INC_PH_2TO1_MASK << TRN_DBG_RDY_INC_PH_2TO1_OFFS(5));
	reg_val |= (0x6 << TRN_DBG_RDY_INC_PH_2TO1_OFFS(5));	/* phase 5 */

	reg_write(TRAINING_DBG_3_REG, reg_val);

	/*
	 * Axi_bresp_mode[8] = Compliant,
	 * Axi_addr_decode_cntrl[11] = Internal,
	 * Axi_data_bus_width[0] = 128bit
	 * */
	/* 0x14a8 - AXI Control Register */
	reg_write(AXI_CTRL_REG, 0);

	/*
	 * Stage 2 - Training Values Setup
	 */
	/* Set X-BAR windows for the training sequence */
	ddr3_save_and_set_training_windows(win);

	return MV_OK;
}

static int ddr3_new_tip_dlb_config(void)
{
	u32 reg, i = 0;
	struct dlb_config *config_table_ptr = sys_env_dlb_config_ptr_get();

	/* Write the configuration */
	while (config_table_ptr[i].reg_addr != 0) {
		reg_write(config_table_ptr[i].reg_addr,
			  config_table_ptr[i].reg_data);
		i++;
	}


	/* Enable DLB */
	reg = reg_read(DLB_CTRL_REG);
	reg &= ~(DLB_EN_MASK << DLB_EN_OFFS) &
	       ~(WR_COALESCE_EN_MASK << WR_COALESCE_EN_OFFS) &
	       ~(AXI_PREFETCH_EN_MASK << AXI_PREFETCH_EN_OFFS) &
	       ~(MBUS_PREFETCH_EN_MASK << MBUS_PREFETCH_EN_OFFS) &
	       ~(PREFETCH_NXT_LN_SZ_TRIG_MASK << PREFETCH_NXT_LN_SZ_TRIG_OFFS);

	reg |= (DLB_EN_ENA << DLB_EN_OFFS) |
	       (WR_COALESCE_EN_ENA << WR_COALESCE_EN_OFFS) |
	       (AXI_PREFETCH_EN_ENA << AXI_PREFETCH_EN_OFFS) |
	       (MBUS_PREFETCH_EN_ENA << MBUS_PREFETCH_EN_OFFS) |
	       (PREFETCH_NXT_LN_SZ_TRIG_ENA << PREFETCH_NXT_LN_SZ_TRIG_OFFS);

	reg_write(DLB_CTRL_REG, reg);

	return MV_OK;
}

int mv_ddr_post_training_soc_config(const char *ddr_type)
{
	u32 reg_val;

	/* Restore and set windows */
	ddr3_restore_and_set_final_windows(win, ddr_type);

	/* Update DRAM init indication in bootROM register */
	reg_val = reg_read(REG_BOOTROM_ROUTINE_ADDR);
	reg_write(REG_BOOTROM_ROUTINE_ADDR,
		  reg_val | (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS));

	/* DLB config */
	ddr3_new_tip_dlb_config();

	return MV_OK;
}

void mv_ddr_mc_config(void)
{
	/* Memory controller initializations */
	struct init_cntr_param init_param;
	int status;

	init_param.do_mrs_phy = 1;
	init_param.is_ctrl64_bit = 0;
	init_param.init_phy = 1;
	init_param.msys_init = 1;
	status = hws_ddr3_tip_init_controller(0, &init_param);
	if (status != MV_OK)
		printf("DDR3 init controller - FAILED 0x%x\n", status);

	status = mv_ddr_mc_init();
	if (status != MV_OK)
		printf("DDR3 init_sequence - FAILED 0x%x\n", status);
}
/* function: mv_ddr_mc_init
 * this function enables the dunit after init controller configuration
 */
int mv_ddr_mc_init(void)
{
	CHECK_STATUS(ddr3_tip_enable_init_sequence(0));

	return MV_OK;
}

/* function: ddr3_tip_configure_phy
 * configures phy and electrical parameters
 */
int ddr3_tip_configure_phy(u32 dev_num)
{
	u32 if_id, phy_id;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		PAD_ZRI_CAL_PHY_REG,
		((0x7f & g_zpri_data) << 7 | (0x7f & g_znri_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		PAD_ZRI_CAL_PHY_REG,
		((0x7f & g_zpri_ctrl) << 7 | (0x7f & g_znri_ctrl))));
	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		PAD_ODT_CAL_PHY_REG,
		((0x3f & g_zpodt_data) << 6 | (0x3f & g_znodt_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		PAD_ODT_CAL_PHY_REG,
		((0x3f & g_zpodt_ctrl) << 6 | (0x3f & g_znodt_ctrl))));

	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		PAD_PRE_DISABLE_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		CMOS_CONFIG_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		CMOS_CONFIG_PHY_REG, 0));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* check if the interface is enabled */
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		for (phy_id = 0;
			phy_id < octets_per_if_num;
			phy_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, phy_id);
				/* Vref & clamp */
				CHECK_STATUS(ddr3_tip_bus_read_modify_write
					(dev_num, ACCESS_TYPE_UNICAST,
					if_id, phy_id, DDR_PHY_DATA,
					PAD_CFG_PHY_REG,
					((clamp_tbl[if_id] << 4) | vref_init_val),
					((0x7 << 4) | 0x7)));
				/* clamp not relevant for control */
				CHECK_STATUS(ddr3_tip_bus_read_modify_write
					(dev_num, ACCESS_TYPE_UNICAST,
					if_id, phy_id, DDR_PHY_CONTROL,
					PAD_CFG_PHY_REG, 0x4, 0x7));
		}
	}

	if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_PHY_EDGE) ==
		MV_DDR_PHY_EDGE_POSITIVE)
		CHECK_STATUS(ddr3_tip_bus_write
		(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		DDR_PHY_DATA, 0x90, 0x6002));


	return MV_OK;
}


int mv_ddr_manual_cal_do(void)
{
	return 0;
}
