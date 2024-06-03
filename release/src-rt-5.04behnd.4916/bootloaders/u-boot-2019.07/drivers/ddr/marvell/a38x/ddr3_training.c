// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_common.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_regs.h"

#define GET_CS_FROM_MASK(mask)	(cs_mask2_num[mask])
#define CS_CBE_VALUE(cs_num)	(cs_cbe_reg[cs_num])

u32 window_mem_addr = 0;
u32 phy_reg0_val = 0;
u32 phy_reg1_val = 8;
u32 phy_reg2_val = 0;
u32 phy_reg3_val = PARAM_UNDEFINED;
enum mv_ddr_freq low_freq = MV_DDR_FREQ_LOW_FREQ;
enum mv_ddr_freq medium_freq;
u32 debug_dunit = 0;
u32 odt_additional = 1;
u32 *dq_map_table = NULL;

/* in case of ddr4 do not run ddr3_tip_write_additional_odt_setting function - mc odt always 'on'
 * in ddr4 case the terminations are rttWR and rttPARK and the odt must be always 'on' 0x1498 = 0xf
 */
u32 odt_config = 1;

u32 nominal_avs;
u32 extension_avs;

u32 is_pll_before_init = 0, is_adll_calib_before_init = 1, is_dfs_in_init = 0;
u32 dfs_low_freq;

u32 g_rtt_nom_cs0, g_rtt_nom_cs1;
u8 calibration_update_control;	/* 2 external only, 1 is internal only */

enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
enum auto_tune_stage training_stage = INIT_CONTROLLER;
u32 finger_test = 0, p_finger_start = 11, p_finger_end = 64,
	n_finger_start = 11, n_finger_end = 64,
	p_finger_step = 3, n_finger_step = 3;
u32 clamp_tbl[] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

/* Initiate to 0xff, this variable is define by user in debug mode */
u32 mode_2t = 0xff;
u32 xsb_validate_type = 0;
u32 xsb_validation_base_address = 0xf000;
u32 first_active_if = 0;
u32 dfs_low_phy1 = 0x1f;
u32 multicast_id = 0;
int use_broadcast = 0;
struct hws_tip_freq_config_info *freq_info_table = NULL;
u8 is_cbe_required = 0;
u32 debug_mode = 0;
u32 delay_enable = 0;
int rl_mid_freq_wa = 0;

u32 effective_cs = 0;

u32 vref_init_val = 0x4;
u32 ck_delay = PARAM_UNDEFINED;

/* Design guidelines parameters */
u32 g_zpri_data = PARAM_UNDEFINED; /* controller data - P drive strength */
u32 g_znri_data = PARAM_UNDEFINED; /* controller data - N drive strength */
u32 g_zpri_ctrl = PARAM_UNDEFINED; /* controller C/A - P drive strength */
u32 g_znri_ctrl = PARAM_UNDEFINED; /* controller C/A - N drive strength */

u32 g_zpodt_data = PARAM_UNDEFINED; /* controller data - P ODT */
u32 g_znodt_data = PARAM_UNDEFINED; /* controller data - N ODT */
u32 g_zpodt_ctrl = PARAM_UNDEFINED; /* controller data - P ODT */
u32 g_znodt_ctrl = PARAM_UNDEFINED; /* controller data - N ODT */

u32 g_odt_config = PARAM_UNDEFINED;
u32 g_rtt_nom = PARAM_UNDEFINED;
u32 g_rtt_wr = PARAM_UNDEFINED;
u32 g_dic = PARAM_UNDEFINED;
u32 g_rtt_park = PARAM_UNDEFINED;

u32 mask_tune_func = (SET_MEDIUM_FREQ_MASK_BIT |
		      WRITE_LEVELING_MASK_BIT |
		      LOAD_PATTERN_2_MASK_BIT |
		      READ_LEVELING_MASK_BIT |
		      SET_TARGET_FREQ_MASK_BIT |
		      WRITE_LEVELING_TF_MASK_BIT |
		      READ_LEVELING_TF_MASK_BIT |
		      CENTRALIZATION_RX_MASK_BIT |
		      CENTRALIZATION_TX_MASK_BIT);

static int ddr3_tip_ddr3_training_main_flow(u32 dev_num);
static int ddr3_tip_write_odt(u32 dev_num, enum hws_access_type access_type,
			      u32 if_id, u32 cl_value, u32 cwl_value);
static int ddr3_tip_ddr3_auto_tune(u32 dev_num);

#ifdef ODT_TEST_SUPPORT
static int odt_test(u32 dev_num, enum hws_algo_type algo_type);
#endif

int adll_calibration(u32 dev_num, enum hws_access_type access_type,
		     u32 if_id, enum mv_ddr_freq frequency);
static int ddr3_tip_set_timing(u32 dev_num, enum hws_access_type access_type,
			       u32 if_id, enum mv_ddr_freq frequency);

static u8 mem_size_config[MV_DDR_DIE_CAP_LAST] = {
	0x2,			/* 512Mbit  */
	0x3,			/* 1Gbit    */
	0x0,			/* 2Gbit    */
	0x4,			/* 4Gbit    */
	0x5,			/* 8Gbit    */
	0x0, /* TODO: placeholder for 16-Mbit die capacity */
	0x0, /* TODO: placeholder for 32-Mbit die capacity */
	0x0, /* TODO: placeholder for 12-Mbit die capacity */
	0x0  /* TODO: placeholder for 24-Mbit die capacity */
};

static u8 cs_mask2_num[] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };

static struct reg_data odpg_default_value[] = {
	{0x1034, 0x38000, MASK_ALL_BITS},
	{0x1038, 0x0, MASK_ALL_BITS},
	{0x10b0, 0x0, MASK_ALL_BITS},
	{0x10b8, 0x0, MASK_ALL_BITS},
	{0x10c0, 0x0, MASK_ALL_BITS},
	{0x10f0, 0x0, MASK_ALL_BITS},
	{0x10f4, 0x0, MASK_ALL_BITS},
	{0x10f8, 0xff, MASK_ALL_BITS},
	{0x10fc, 0xffff, MASK_ALL_BITS},
	{0x1130, 0x0, MASK_ALL_BITS},
	{0x1830, 0x2000000, MASK_ALL_BITS},
	{0x14d0, 0x0, MASK_ALL_BITS},
	{0x14d4, 0x0, MASK_ALL_BITS},
	{0x14d8, 0x0, MASK_ALL_BITS},
	{0x14dc, 0x0, MASK_ALL_BITS},
	{0x1454, 0x0, MASK_ALL_BITS},
	{0x1594, 0x0, MASK_ALL_BITS},
	{0x1598, 0x0, MASK_ALL_BITS},
	{0x159c, 0x0, MASK_ALL_BITS},
	{0x15a0, 0x0, MASK_ALL_BITS},
	{0x15a4, 0x0, MASK_ALL_BITS},
	{0x15a8, 0x0, MASK_ALL_BITS},
	{0x15ac, 0x0, MASK_ALL_BITS},
	{0x1604, 0x0, MASK_ALL_BITS},
	{0x1608, 0x0, MASK_ALL_BITS},
	{0x160c, 0x0, MASK_ALL_BITS},
	{0x1610, 0x0, MASK_ALL_BITS},
	{0x1614, 0x0, MASK_ALL_BITS},
	{0x1618, 0x0, MASK_ALL_BITS},
	{0x1624, 0x0, MASK_ALL_BITS},
	{0x1690, 0x0, MASK_ALL_BITS},
	{0x1694, 0x0, MASK_ALL_BITS},
	{0x1698, 0x0, MASK_ALL_BITS},
	{0x169c, 0x0, MASK_ALL_BITS},
	{0x14b8, 0x6f67, MASK_ALL_BITS},
	{0x1630, 0x0, MASK_ALL_BITS},
	{0x1634, 0x0, MASK_ALL_BITS},
	{0x1638, 0x0, MASK_ALL_BITS},
	{0x163c, 0x0, MASK_ALL_BITS},
	{0x16b0, 0x0, MASK_ALL_BITS},
	{0x16b4, 0x0, MASK_ALL_BITS},
	{0x16b8, 0x0, MASK_ALL_BITS},
	{0x16bc, 0x0, MASK_ALL_BITS},
	{0x16c0, 0x0, MASK_ALL_BITS},
	{0x16c4, 0x0, MASK_ALL_BITS},
	{0x16c8, 0x0, MASK_ALL_BITS},
	{0x16cc, 0x1, MASK_ALL_BITS},
	{0x16f0, 0x1, MASK_ALL_BITS},
	{0x16f4, 0x0, MASK_ALL_BITS},
	{0x16f8, 0x0, MASK_ALL_BITS},
	{0x16fc, 0x0, MASK_ALL_BITS}
};

/* MR cmd and addr definitions */
struct mv_ddr_mr_data mr_data[] = {
	{MRS0_CMD, MR0_REG},
	{MRS1_CMD, MR1_REG},
	{MRS2_CMD, MR2_REG},
	{MRS3_CMD, MR3_REG}
};

/* inverse pads */
static int ddr3_tip_pad_inv(void)
{
	u32 sphy, data;
	u32 sphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 ck_swap_ctrl_sphy;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (sphy = 0; sphy < sphy_max; sphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
		if (tm->interface_params[0].
		    as_bus_params[sphy].is_dqs_swap == 1) {
			data = (INVERT_PAD << INV_PAD4_OFFS |
				INVERT_PAD << INV_PAD5_OFFS);
			/* dqs swap */
			ddr3_tip_bus_read_modify_write(0, ACCESS_TYPE_UNICAST,
						       0, sphy,
						       DDR_PHY_DATA,
						       PHY_CTRL_PHY_REG,
						       data, data);
		}

		if (tm->interface_params[0].as_bus_params[sphy].
		    is_ck_swap == 1 && sphy == 0) {
/* TODO: move this code to per platform one */
#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
			/* clock swap for both cs0 and cs1 */
			data = (INVERT_PAD << INV_PAD2_OFFS |
				INVERT_PAD << INV_PAD6_OFFS |
				INVERT_PAD << INV_PAD4_OFFS |
				INVERT_PAD << INV_PAD5_OFFS);
			ck_swap_ctrl_sphy = CK_SWAP_CTRL_PHY_NUM;
			ddr3_tip_bus_read_modify_write(0, ACCESS_TYPE_UNICAST,
						       0, ck_swap_ctrl_sphy,
						       DDR_PHY_CONTROL,
						       PHY_CTRL_PHY_REG,
						       data, data);
#else /* !CONFIG_ARMADA_38X && !CONFIG_ARMADA_39X && !A70X0 && !A80X0 && !A3900 */
#pragma message "unknown platform to configure ddr clock swap"
#endif
		}
	}

	return MV_OK;
}

static int ddr3_tip_rank_control(u32 dev_num, u32 if_id);

/*
 * Update global training parameters by data from user
 */
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params)
{
	if (params->ck_delay != PARAM_UNDEFINED)
		ck_delay = params->ck_delay;
	if (params->phy_reg3_val != PARAM_UNDEFINED)
		phy_reg3_val = params->phy_reg3_val;
	if (params->g_rtt_nom != PARAM_UNDEFINED)
		g_rtt_nom = params->g_rtt_nom;
	if (params->g_rtt_wr != PARAM_UNDEFINED)
		g_rtt_wr = params->g_rtt_wr;
	if (params->g_dic != PARAM_UNDEFINED)
		g_dic = params->g_dic;
	if (params->g_odt_config != PARAM_UNDEFINED)
		g_odt_config = params->g_odt_config;
	if (params->g_zpri_data != PARAM_UNDEFINED)
		g_zpri_data = params->g_zpri_data;
	if (params->g_znri_data != PARAM_UNDEFINED)
		g_znri_data = params->g_znri_data;
	if (params->g_zpri_ctrl != PARAM_UNDEFINED)
		g_zpri_ctrl = params->g_zpri_ctrl;
	if (params->g_znri_ctrl != PARAM_UNDEFINED)
		g_znri_ctrl = params->g_znri_ctrl;
	if (params->g_zpodt_data != PARAM_UNDEFINED)
		g_zpodt_data = params->g_zpodt_data;
	if (params->g_znodt_data != PARAM_UNDEFINED)
		g_znodt_data = params->g_znodt_data;
	if (params->g_zpodt_ctrl != PARAM_UNDEFINED)
		g_zpodt_ctrl = params->g_zpodt_ctrl;
	if (params->g_znodt_ctrl != PARAM_UNDEFINED)
		g_znodt_ctrl = params->g_znodt_ctrl;
	if (params->g_rtt_park != PARAM_UNDEFINED)
		g_rtt_park = params->g_rtt_park;


	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("DGL parameters: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n",
			   g_zpri_data, g_znri_data, g_zpri_ctrl, g_znri_ctrl, g_zpodt_data, g_znodt_data,
			   g_zpodt_ctrl, g_znodt_ctrl, g_rtt_nom, g_dic, g_odt_config, g_rtt_wr));

	return MV_OK;
}

/*
 * Configure CS
 */
int ddr3_tip_configure_cs(u32 dev_num, u32 if_id, u32 cs_num, u32 enable)
{
	u32 data, addr_hi, data_high;
	u32 mem_index;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (enable == 1) {
		data = (tm->interface_params[if_id].bus_width ==
			MV_DDR_DEV_WIDTH_8BIT) ? 0 : 1;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ADDR_CTRL_REG, (data << (cs_num * 4)),
			      0x3 << (cs_num * 4)));
		mem_index = tm->interface_params[if_id].memory_size;

		addr_hi = mem_size_config[mem_index] & 0x3;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ADDR_CTRL_REG,
			      (addr_hi << (2 + cs_num * 4)),
			      0x3 << (2 + cs_num * 4)));

		data_high = (mem_size_config[mem_index] & 0x4) >> 2;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ADDR_CTRL_REG,
			      data_high << (20 + cs_num), 1 << (20 + cs_num)));

		/* Enable Address Select Mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ADDR_CTRL_REG, 1 << (16 + cs_num),
			      1 << (16 + cs_num)));
	}
	switch (cs_num) {
	case 0:
	case 1:
	case 2:
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUNIT_CTRL_LOW_REG, (enable << (cs_num + 11)),
			      1 << (cs_num + 11)));
		break;
	case 3:
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUNIT_CTRL_LOW_REG, (enable << 15), 1 << 15));
		break;
	}

	return MV_OK;
}

/*
 * Init Controller Flow
 */
int hws_ddr3_tip_init_controller(u32 dev_num, struct init_cntr_param *init_cntr_prm)
{
	u32 if_id;
	u32 cs_num;
	u32 t_ckclk = 0, t_wr = 0, t2t = 0;
	u32 data_value = 0, cs_cnt = 0,
		mem_mask = 0, bus_index = 0;
	enum mv_ddr_speed_bin speed_bin_index = SPEED_BIN_DDR_2133N;
	u32 cs_mask = 0;
	u32 cl_value = 0, cwl_val = 0;
	u32 bus_cnt = 0, adll_tap = 0;
	enum hws_access_type access_type = ACCESS_TYPE_UNICAST;
	u32 data_read[MAX_INTERFACE_NUM];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_timing timing;
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
			  ("Init_controller, do_mrs_phy=%d, is_ctrl64_bit=%d\n",
			   init_cntr_prm->do_mrs_phy,
			   init_cntr_prm->is_ctrl64_bit));

	if (init_cntr_prm->init_phy == 1) {
		CHECK_STATUS(ddr3_tip_configure_phy(dev_num));
	}

	if (generic_init_controller == 1) {
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("active IF %d\n", if_id));
			mem_mask = 0;
			for (bus_index = 0;
			     bus_index < octets_per_if_num;
			     bus_index++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_index);
				mem_mask |=
					tm->interface_params[if_id].
					as_bus_params[bus_index].mirror_enable_bitmask;
			}

			if (mem_mask != 0) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      if_id, DUAL_DUNIT_CFG_REG, 0,
					      0x8));
			}

			speed_bin_index =
				tm->interface_params[if_id].
				speed_bin_index;

			/* t_ckclk is external clock */
			t_ckclk = (MEGA / mv_ddr_freq_get(freq));

			if (MV_DDR_IS_HALF_BUS_DRAM_MODE(tm->bus_act_mask, octets_per_if_num))
				data_value = (0x4000 | 0 | 0x1000000) & ~(1 << 26);
			else
				data_value = (0x4000 | 0x8000 | 0x1000000) & ~(1 << 26);

			/* Interface Bus Width */
			/* SRMode */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      SDRAM_CFG_REG, data_value,
				      0x100c000));

			/* Interleave first command pre-charge enable (TBD) */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      SDRAM_OPEN_PAGES_CTRL_REG, (1 << 10),
				      (1 << 10)));

			/* Reset divider_b assert -> de-assert */
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
						       SDRAM_CFG_REG,
						       0x0 << PUP_RST_DIVIDER_OFFS,
						       PUP_RST_DIVIDER_MASK << PUP_RST_DIVIDER_OFFS));

			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
						       SDRAM_CFG_REG,
						       0x1 << PUP_RST_DIVIDER_OFFS,
						       PUP_RST_DIVIDER_MASK << PUP_RST_DIVIDER_OFFS));

			/* PHY configuration */
			/*
			 * Postamble Length = 1.5cc, Addresscntl to clk skew
			 * \BD, Preamble length normal, parralal ADLL enable
			 */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DRAM_PHY_CFG_REG, 0x28, 0x3e));
			if (init_cntr_prm->is_ctrl64_bit) {
				/* positive edge */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DRAM_PHY_CFG_REG, 0x0,
					      0xff80));
			}

			/* calibration block disable */
			/* Xbar Read buffer select (for Internal access) */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      MAIN_PADS_CAL_MACH_CTRL_REG, 0x1200c,
				      0x7dffe01c));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      MAIN_PADS_CAL_MACH_CTRL_REG,
				      calibration_update_control << 3, 0x3 << 3));

			/* Pad calibration control - enable */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      MAIN_PADS_CAL_MACH_CTRL_REG, 0x1, 0x1));
			if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) < MV_TIP_REV_3) {
				/* DDR3 rank ctrl \96 part of the generic code */
				/* CS1 mirroring enable + w/a for JIRA DUNIT-14581 */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DDR3_RANK_CTRL_REG, 0x27, MASK_ALL_BITS));
			}

			cs_mask = 0;
			data_value = 0x7;
			/*
			 * Address ctrl \96 Part of the Generic code
			 * The next configuration is done:
			 * 1)  Memory Size
			 * 2) Bus_width
			 * 3) CS#
			 * 4) Page Number
			 * Per Dunit get from the Map_topology the parameters:
			 * Bus_width
			 */

			data_value =
				(tm->interface_params[if_id].
				 bus_width == MV_DDR_DEV_WIDTH_8BIT) ? 0 : 1;

			/* create merge cs mask for all cs available in dunit */
			for (bus_cnt = 0;
			     bus_cnt < octets_per_if_num;
			     bus_cnt++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
				cs_mask |=
					tm->interface_params[if_id].
					as_bus_params[bus_cnt].cs_bitmask;
			}
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("Init_controller IF %d cs_mask %d\n",
					   if_id, cs_mask));
			/*
			 * Configure the next upon the Map Topology \96 If the
			 * Dunit is CS0 Configure CS0 if it is multi CS
			 * configure them both:  The Bust_width it\92s the
			 * Memory Bus width \96 x8 or x16
			 */
			for (cs_cnt = 0; cs_cnt < MAX_CS_NUM; cs_cnt++) {
				ddr3_tip_configure_cs(dev_num, if_id, cs_cnt,
						      ((cs_mask & (1 << cs_cnt)) ? 1
						       : 0));
			}

			if (init_cntr_prm->do_mrs_phy) {
				/*
				 * MR0 \96 Part of the Generic code
				 * The next configuration is done:
				 * 1) Burst Length
				 * 2) CAS Latency
				 * get for each dunit what is it Speed_bin &
				 * Target Frequency. From those both parameters
				 * get the appropriate Cas_l from the CL table
				 */
				cl_value =
					tm->interface_params[if_id].
					cas_l;
				cwl_val =
					tm->interface_params[if_id].
					cas_wl;
				DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
						  ("cl_value 0x%x cwl_val 0x%x\n",
						   cl_value, cwl_val));

				t_wr = time_to_nclk(mv_ddr_speed_bin_timing_get
							   (speed_bin_index,
							    SPEED_BIN_TWR), t_ckclk);

				data_value =
					((cl_mask_table[cl_value] & 0x1) << 2) |
					((cl_mask_table[cl_value] & 0xe) << 3);
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR0_REG, data_value,
					      (0x7 << 4) | (1 << 2)));
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR0_REG, twr_mask_table[t_wr] << 9,
					      0x7 << 9));

				/*
				 * MR1: Set RTT and DIC Design GL values
				 * configured by user
				 */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE, MR1_REG,
					      g_dic | g_rtt_nom, 0x266));

				/* MR2 - Part of the Generic code */
				/*
				 * The next configuration is done:
				 * 1)  SRT
				 * 2) CAS Write Latency
				 */
				data_value = (cwl_mask_table[cwl_val] << 3);
				data_value |=
					((tm->interface_params[if_id].
					  interface_temp ==
					  MV_DDR_TEMP_HIGH) ? (1 << 7) : 0);
				data_value |= g_rtt_wr;
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR2_REG, data_value,
					      (0x7 << 3) | (0x1 << 7) | (0x3 <<
									 9)));
			}

			ddr3_tip_write_odt(dev_num, access_type, if_id,
					   cl_value, cwl_val);
			ddr3_tip_set_timing(dev_num, access_type, if_id, freq);

			if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) < MV_TIP_REV_3) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DUNIT_CTRL_HIGH_REG, 0x1000119,
					      0x100017F));
			} else {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DUNIT_CTRL_HIGH_REG, 0x600177 |
					      (init_cntr_prm->is_ctrl64_bit ?
					      CPU_INTERJECTION_ENA_SPLIT_ENA << CPU_INTERJECTION_ENA_OFFS :
					      CPU_INTERJECTION_ENA_SPLIT_DIS << CPU_INTERJECTION_ENA_OFFS),
					      0x1600177 | CPU_INTERJECTION_ENA_MASK <<
					      CPU_INTERJECTION_ENA_OFFS));
			}

			/* reset bit 7 */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CTRL_HIGH_REG,
				      (init_cntr_prm->msys_init << 7), (1 << 7)));

			timing = tm->interface_params[if_id].timing;

			if (mode_2t != 0xff) {
				t2t = mode_2t;
			} else if (timing != MV_DDR_TIM_DEFAULT) {
				t2t = (timing == MV_DDR_TIM_2T) ? 1 : 0;
			} else {
				/* calculate number of CS (per interface) */
				cs_num = mv_ddr_cs_num_get();
				t2t = (cs_num == 1) ? 0 : 1;
			}

			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CTRL_LOW_REG, t2t << 3,
				      0x3 << 3));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_TIMING_REG, 0x28 << 9, 0x3f << 9));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_TIMING_REG, 0xa << 21, 0xff << 21));

			/* move the block to ddr3_tip_set_timing - end */
			/* AUTO_ZQC_TIMING */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      ZQC_CFG_REG, (AUTO_ZQC_TIMING | (2 << 20)),
				      0x3fffff));
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, access_type, if_id,
				      DRAM_PHY_CFG_REG, data_read, 0x30));
			data_value =
				(data_read[if_id] == 0) ? (1 << 11) : 0;
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CTRL_HIGH_REG, data_value,
				      (1 << 11)));

			/* Set Active control for ODT write transactions */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_MULTICAST,
				      PARAM_NOT_CARE, 0x1494, g_odt_config,
				      MASK_ALL_BITS));

			if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) == MV_TIP_REV_3) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      0x14a8, 0x900, 0x900));
				/* wa: controls control sub-phy outputs floating during self-refresh */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      0x16d0, 0, 0x8000));
			}
		}
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_rank_control(dev_num, if_id));

		if (init_cntr_prm->do_mrs_phy)
			ddr3_tip_pad_inv();

		/* Pad calibration control - disable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      MAIN_PADS_CAL_MACH_CTRL_REG, 0x0, 0x1));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      MAIN_PADS_CAL_MACH_CTRL_REG,
			      calibration_update_control << 3, 0x3 << 3));
	}


	if (delay_enable != 0) {
		adll_tap = MEGA / (mv_ddr_freq_get(freq) * 64);
		ddr3_tip_cmd_addr_init_delay(dev_num, adll_tap);
	}

	return MV_OK;
}

/*
 * Rank Control Flow
 */
static int ddr3_tip_rev2_rank_control(u32 dev_num, u32 if_id)
{
	u32 data_value = 0,  bus_cnt = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (bus_cnt = 0; bus_cnt < octets_per_if_num; bus_cnt++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
		data_value |= tm->interface_params[if_id].as_bus_params[bus_cnt].
			      cs_bitmask;

		if (tm->interface_params[if_id].as_bus_params[bus_cnt].
		    mirror_enable_bitmask == 1) {
			/*
			 * Check mirror_enable_bitmask
			 * If it is enabled, CS + 4 bit in a word to be '1'
			 */
			if ((tm->interface_params[if_id].as_bus_params[bus_cnt].
			     cs_bitmask & 0x1) != 0) {
				data_value |= tm->interface_params[if_id].
					      as_bus_params[bus_cnt].
					      mirror_enable_bitmask << 4;
			}

			if ((tm->interface_params[if_id].as_bus_params[bus_cnt].
			     cs_bitmask & 0x2) != 0) {
				data_value |= tm->interface_params[if_id].
					      as_bus_params[bus_cnt].
					      mirror_enable_bitmask << 5;
			}

			if ((tm->interface_params[if_id].as_bus_params[bus_cnt].
			     cs_bitmask & 0x4) != 0) {
				data_value |= tm->interface_params[if_id].
					      as_bus_params[bus_cnt].
					      mirror_enable_bitmask << 6;
			}

			if ((tm->interface_params[if_id].as_bus_params[bus_cnt].
			     cs_bitmask & 0x8) != 0) {
				data_value |= tm->interface_params[if_id].
					      as_bus_params[bus_cnt].
					      mirror_enable_bitmask << 7;
			}
		}
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, DDR3_RANK_CTRL_REG,
		      data_value, 0xff));

	return MV_OK;
}

static int ddr3_tip_rev3_rank_control(u32 dev_num, u32 if_id)
{
	u32 data_value = 0, bus_cnt;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (bus_cnt = 1; bus_cnt < octets_per_if_num; bus_cnt++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
		if ((tm->interface_params[if_id].
		     as_bus_params[0].cs_bitmask !=
		     tm->interface_params[if_id].
		     as_bus_params[bus_cnt].cs_bitmask) ||
		    (tm->interface_params[if_id].
		     as_bus_params[0].mirror_enable_bitmask !=
		     tm->interface_params[if_id].
		     as_bus_params[bus_cnt].mirror_enable_bitmask))
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("WARNING:Wrong configuration for pup #%d CS mask and CS mirroring for all pups should be the same\n",
					   bus_cnt));
	}

	data_value |= tm->interface_params[if_id].
		as_bus_params[0].cs_bitmask;
	data_value |= tm->interface_params[if_id].
		as_bus_params[0].mirror_enable_bitmask << 4;

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, DDR3_RANK_CTRL_REG,
		      data_value, 0xff));

	return MV_OK;
}

static int ddr3_tip_rank_control(u32 dev_num, u32 if_id)
{
	if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) == MV_TIP_REV_2)
		return ddr3_tip_rev2_rank_control(dev_num, if_id);
	else
		return ddr3_tip_rev3_rank_control(dev_num, if_id);
}

/*
 * Algorithm Parameters Validation
 */
int ddr3_tip_validate_algo_var(u32 value, u32 fail_value, char *var_name)
{
	if (value == fail_value) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Error: %s is not initialized (Algo Components Validation)\n",
				   var_name));
		return 0;
	}

	return 1;
}

int ddr3_tip_validate_algo_ptr(void *ptr, void *fail_value, char *ptr_name)
{
	if (ptr == fail_value) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Error: %s is not initialized (Algo Components Validation)\n",
				   ptr_name));
		return 0;
	}

	return 1;
}

int ddr3_tip_validate_algo_components(u8 dev_num)
{
	int status = 1;

	/* Check DGL parameters*/
	status &= ddr3_tip_validate_algo_var(ck_delay, PARAM_UNDEFINED, "ck_delay");
	status &= ddr3_tip_validate_algo_var(phy_reg3_val, PARAM_UNDEFINED, "phy_reg3_val");
	status &= ddr3_tip_validate_algo_var(g_rtt_nom, PARAM_UNDEFINED, "g_rtt_nom");
	status &= ddr3_tip_validate_algo_var(g_dic, PARAM_UNDEFINED, "g_dic");
	status &= ddr3_tip_validate_algo_var(odt_config, PARAM_UNDEFINED, "odt_config");
	status &= ddr3_tip_validate_algo_var(g_zpri_data, PARAM_UNDEFINED, "g_zpri_data");
	status &= ddr3_tip_validate_algo_var(g_znri_data, PARAM_UNDEFINED, "g_znri_data");
	status &= ddr3_tip_validate_algo_var(g_zpri_ctrl, PARAM_UNDEFINED, "g_zpri_ctrl");
	status &= ddr3_tip_validate_algo_var(g_znri_ctrl, PARAM_UNDEFINED, "g_znri_ctrl");
	status &= ddr3_tip_validate_algo_var(g_zpodt_data, PARAM_UNDEFINED, "g_zpodt_data");
	status &= ddr3_tip_validate_algo_var(g_znodt_data, PARAM_UNDEFINED, "g_znodt_data");
	status &= ddr3_tip_validate_algo_var(g_zpodt_ctrl, PARAM_UNDEFINED, "g_zpodt_ctrl");
	status &= ddr3_tip_validate_algo_var(g_znodt_ctrl, PARAM_UNDEFINED, "g_znodt_ctrl");

	/* Check functions pointers */
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].tip_dunit_mux_select_func,
					     NULL, "tip_dunit_mux_select_func");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].mv_ddr_dunit_write,
					     NULL, "mv_ddr_dunit_write");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].mv_ddr_dunit_read,
					     NULL, "mv_ddr_dunit_read");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].mv_ddr_phy_write,
					     NULL, "mv_ddr_phy_write");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].mv_ddr_phy_read,
					     NULL, "mv_ddr_phy_read");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].tip_get_freq_config_info_func,
					     NULL, "tip_get_freq_config_info_func");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].tip_set_freq_divider_func,
					     NULL, "tip_set_freq_divider_func");
	status &= ddr3_tip_validate_algo_ptr(config_func_info[dev_num].tip_get_clock_ratio,
					     NULL, "tip_get_clock_ratio");

	status &= ddr3_tip_validate_algo_ptr(dq_map_table, NULL, "dq_map_table");
	status &= ddr3_tip_validate_algo_var(dfs_low_freq, 0, "dfs_low_freq");

	return (status == 1) ? MV_OK : MV_NOT_INITIALIZED;
}


int ddr3_pre_algo_config(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* Set Bus3 ECC training mode */
	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask)) {
		/* Set Bus3 ECC MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      DRAM_PINS_MUX_REG, 0x100, 0x100));
	}

	/* Set regular ECC training mode (bus4 and bus 3) */
	if ((DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask))) {
		/* Enable ECC Write MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x100, 0x100));
		/* General ECC enable */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      SDRAM_CFG_REG, 0x40000, 0x40000));
		/* Disable Read Data ECC MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x0, 0x2));
	}

	return MV_OK;
}

int ddr3_post_algo_config(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int status;

	status = ddr3_post_run_alg();
	if (MV_OK != status) {
		printf("DDR3 Post Run Alg - FAILED 0x%x\n", status);
		return status;
	}

	/* Un_set ECC training mode */
	if ((DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask))) {
		/* Disable ECC Write MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x0, 0x100));
		/* General ECC and Bus3 ECC MUX remains enabled */
	}

	return MV_OK;
}

/*
 * Run Training Flow
 */
int hws_ddr3_tip_run_alg(u32 dev_num, enum hws_algo_type algo_type)
{
	int status = MV_OK;

	status = ddr3_pre_algo_config();
	if (MV_OK != status) {
		printf("DDR3 Pre Algo Config - FAILED 0x%x\n", status);
		return status;
	}

#ifdef ODT_TEST_SUPPORT
	if (finger_test == 1)
		return odt_test(dev_num, algo_type);
#endif

	if (algo_type == ALGO_TYPE_DYNAMIC) {
		status = ddr3_tip_ddr3_auto_tune(dev_num);
	}

	if (status != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("********   DRAM initialization Failed (res 0x%x)   ********\n",
				   status));
		return status;
	}

	status = ddr3_post_algo_config();
	if (MV_OK != status) {
		printf("DDR3 Post Algo Config - FAILED 0x%x\n", status);
		return status;
	}

	return status;
}

#ifdef ODT_TEST_SUPPORT
/*
 * ODT Test
 */
static int odt_test(u32 dev_num, enum hws_algo_type algo_type)
{
	int ret = MV_OK, ret_tune = MV_OK;
	int pfinger_val = 0, nfinger_val;

	for (pfinger_val = p_finger_start; pfinger_val <= p_finger_end;
	     pfinger_val += p_finger_step) {
		for (nfinger_val = n_finger_start; nfinger_val <= n_finger_end;
		     nfinger_val += n_finger_step) {
			if (finger_test != 0) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
						  ("pfinger_val %d nfinger_val %d\n",
						   pfinger_val, nfinger_val));
				/*
				 * TODO: need to check the correctness
				 * of the following two lines.
				 */
				g_zpodt_data = pfinger_val;
				g_znodt_data = nfinger_val;
			}

			if (algo_type == ALGO_TYPE_DYNAMIC) {
				ret = ddr3_tip_ddr3_auto_tune(dev_num);
			}
		}
	}

	if (ret_tune != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Run_alg: tuning failed %d\n", ret_tune));
		ret = (ret == MV_OK) ? ret_tune : ret;
	}

	return ret;
}
#endif

/*
 * Select Controller
 */
int hws_ddr3_tip_select_ddr_controller(u32 dev_num, int enable)
{
	return config_func_info[dev_num].
		tip_dunit_mux_select_func((u8)dev_num, enable);
}

/*
 * Dunit Register Write
 */
int ddr3_tip_if_write(u32 dev_num, enum hws_access_type interface_access,
		      u32 if_id, u32 reg_addr, u32 data_value, u32 mask)
{
	config_func_info[dev_num].mv_ddr_dunit_write(reg_addr, mask, data_value);

	return MV_OK;
}

/*
 * Dunit Register Read
 */
int ddr3_tip_if_read(u32 dev_num, enum hws_access_type interface_access,
		     u32 if_id, u32 reg_addr, u32 *data, u32 mask)
{
	config_func_info[dev_num].mv_ddr_dunit_read(reg_addr, mask, data);

	return MV_OK;
}

/*
 * Dunit Register Polling
 */
int ddr3_tip_if_polling(u32 dev_num, enum hws_access_type access_type,
			u32 if_id, u32 exp_value, u32 mask, u32 offset,
			u32 poll_tries)
{
	u32 poll_cnt = 0, interface_num = 0, start_if, end_if;
	u32 read_data[MAX_INTERFACE_NUM];
	int ret;
	int is_fail = 0, is_if_fail;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	for (interface_num = start_if; interface_num <= end_if; interface_num++) {
		/* polling bit 3 for n times */
		VALIDATE_IF_ACTIVE(tm->if_act_mask, interface_num);

		is_if_fail = 0;
		for (poll_cnt = 0; poll_cnt < poll_tries; poll_cnt++) {
			ret =
				ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST,
						 interface_num, offset, read_data,
						 mask);
			if (ret != MV_OK)
				return ret;

			if (read_data[interface_num] == exp_value)
				break;
		}

		if (poll_cnt >= poll_tries) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("max poll IF #%d\n", interface_num));
			is_fail = 1;
			is_if_fail = 1;
		}

		training_result[training_stage][interface_num] =
			(is_if_fail == 1) ? TEST_FAILED : TEST_SUCCESS;
	}

	return (is_fail == 0) ? MV_OK : MV_FAIL;
}

/*
 * Bus read access
 */
int ddr3_tip_bus_read(u32 dev_num, u32 if_id,
		      enum hws_access_type phy_access, u32 phy_id,
		      enum hws_ddr_phy phy_type, u32 reg_addr, u32 *data)
{
	return config_func_info[dev_num].
		mv_ddr_phy_read(phy_access, phy_id, phy_type, reg_addr, data);
}

/*
 * Bus write access
 */
int ddr3_tip_bus_write(u32 dev_num, enum hws_access_type interface_access,
		       u32 if_id, enum hws_access_type phy_access,
		       u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr,
		       u32 data_value)
{
	return config_func_info[dev_num].
		mv_ddr_phy_write(phy_access, phy_id, phy_type, reg_addr, data_value, OPERATION_WRITE);
}


/*
 * Phy read-modify-write
 */
int ddr3_tip_bus_read_modify_write(u32 dev_num, enum hws_access_type access_type,
				   u32 interface_id, u32 phy_id,
				   enum hws_ddr_phy phy_type, u32 reg_addr,
				   u32 data_value, u32 reg_mask)
{
	u32 data_val = 0, if_id, start_if, end_if;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = interface_id;
		end_if = interface_id;
	}

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_bus_read
			     (dev_num, if_id, ACCESS_TYPE_UNICAST, phy_id,
			      phy_type, reg_addr, &data_val));
		data_value = (data_val & (~reg_mask)) | (data_value & reg_mask);
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ACCESS_TYPE_UNICAST, phy_id, phy_type, reg_addr,
			      data_value));
	}

	return MV_OK;
}

/*
 * ADLL Calibration
 */
int adll_calibration(u32 dev_num, enum hws_access_type access_type,
		     u32 if_id, enum mv_ddr_freq frequency)
{
	struct hws_tip_freq_config_info freq_config_info;
	u32 bus_cnt = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* Reset Diver_b assert -> de-assert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CFG_REG,
		      0, 0x10000000));
	mdelay(10);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CFG_REG,
		      0x10000000, 0x10000000));

	CHECK_STATUS(config_func_info[dev_num].
		     tip_get_freq_config_info_func((u8)dev_num, frequency,
						   &freq_config_info));

	for (bus_cnt = 0; bus_cnt < octets_per_if_num; bus_cnt++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, access_type, if_id, bus_cnt,
			      DDR_PHY_DATA, ADLL_CFG0_PHY_REG,
			      freq_config_info.bw_per_freq << 8, 0x700));
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, access_type, if_id, bus_cnt,
			      DDR_PHY_DATA, ADLL_CFG2_PHY_REG,
			      freq_config_info.rate_per_freq, 0x7));
	}

	for (bus_cnt = 0; bus_cnt < DDR_IF_CTRL_SUBPHYS_NUM; bus_cnt++) {
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, bus_cnt,
			      DDR_PHY_CONTROL, ADLL_CFG0_PHY_REG,
			      freq_config_info.bw_per_freq << 8, 0x700));
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, bus_cnt,
			      DDR_PHY_CONTROL, ADLL_CFG2_PHY_REG,
			      freq_config_info.rate_per_freq, 0x7));
	}

	/* DUnit to Phy drive post edge, ADLL reset assert de-assert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, DRAM_PHY_CFG_REG,
		      0, (0x80000000 | 0x40000000)));
	mdelay(100 / (mv_ddr_freq_get(frequency)) / mv_ddr_freq_get(MV_DDR_FREQ_LOW_FREQ));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, DRAM_PHY_CFG_REG,
		      (0x80000000 | 0x40000000), (0x80000000 | 0x40000000)));

	/* polling for ADLL Done */
	if (ddr3_tip_if_polling(dev_num, access_type, if_id,
				0x3ff03ff, 0x3ff03ff, PHY_LOCK_STATUS_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Freq_set: DDR3 poll failed(1)"));
	}

	/* pup data_pup reset assert-> deassert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CFG_REG,
		      0, 0x60000000));
	mdelay(10);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CFG_REG,
		      0x60000000, 0x60000000));

	return MV_OK;
}

int ddr3_tip_freq_set(u32 dev_num, enum hws_access_type access_type,
		      u32 if_id, enum mv_ddr_freq frequency)
{
	u32 cl_value = 0, cwl_value = 0, mem_mask = 0, val = 0,
		bus_cnt = 0, t_wr = 0, t_ckclk = 0,
		cnt_id;
	u32 end_if, start_if;
	u32 bus_index = 0;
	int is_dll_off = 0;
	enum mv_ddr_speed_bin speed_bin_index = 0;
	struct hws_tip_freq_config_info freq_config_info;
	enum hws_result *flow_result = training_result[training_stage];
	u32 adll_tap = 0;
	u32 cs_num;
	u32 t2t;
	u32 cs_mask[MAX_INTERFACE_NUM];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int tclk;
	enum mv_ddr_timing timing = tm->interface_params[if_id].timing;
	u32 freq = mv_ddr_freq_get(frequency);

	DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
			  ("dev %d access %d IF %d freq %d\n", dev_num,
			   access_type, if_id, frequency));

	if (frequency == MV_DDR_FREQ_LOW_FREQ)
		is_dll_off = 1;
	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	/* calculate interface cs mask - Oferb 4/11 */
	/* speed bin can be different for each interface */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* cs enable is active low */
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		cs_mask[if_id] = CS_BIT_MASK;
		training_result[training_stage][if_id] = TEST_SUCCESS;
		ddr3_tip_calc_cs_mask(dev_num, if_id, effective_cs,
				      &cs_mask[if_id]);
	}

	/* speed bin can be different for each interface */
	/*
	 * moti b - need to remove the loop for multicas access functions
	 * and loop the unicast access functions
	 */
	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		flow_result[if_id] = TEST_SUCCESS;
		speed_bin_index =
			tm->interface_params[if_id].speed_bin_index;
		if (tm->interface_params[if_id].memory_freq ==
		    frequency) {
			cl_value =
				tm->interface_params[if_id].cas_l;
			cwl_value =
				tm->interface_params[if_id].cas_wl;
		} else if (tm->cfg_src == MV_DDR_CFG_SPD) {
			tclk = 1000000 / freq;
			cl_value = mv_ddr_cl_calc(tm->timing_data[MV_DDR_TAA_MIN], tclk);
			if (cl_value == 0) {
				printf("mv_ddr: unsupported cas latency value found\n");
				return MV_FAIL;
			}
			cwl_value = mv_ddr_cwl_calc(tclk);
			if (cwl_value == 0) {
				printf("mv_ddr: unsupported cas write latency value found\n");
				return MV_FAIL;
			}
		} else {
			cl_value = mv_ddr_cl_val_get(speed_bin_index, frequency);
			cwl_value = mv_ddr_cwl_val_get(speed_bin_index, frequency);
		}

		DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
				  ("Freq_set dev 0x%x access 0x%x if 0x%x freq 0x%x speed %d:\n\t",
				   dev_num, access_type, if_id,
				   frequency, speed_bin_index));

		for (cnt_id = 0; cnt_id < MV_DDR_FREQ_LAST; cnt_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("%d ", mv_ddr_cl_val_get(speed_bin_index, cnt_id)));
		}

		DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE, ("\n"));
		mem_mask = 0;
		for (bus_index = 0; bus_index < octets_per_if_num;
		     bus_index++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_index);
			mem_mask |=
				tm->interface_params[if_id].
				as_bus_params[bus_index].mirror_enable_bitmask;
		}

		if (mem_mask != 0) {
			/* motib redundent in KW28 */
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
						       if_id,
						       DUAL_DUNIT_CFG_REG, 0, 0x8));
		}

		/* dll state after exiting SR */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DFS_REG, 0x1, 0x1));
		} else {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DFS_REG, 0, 0x1));
		}

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DUNIT_MMASK_REG, 0, 0x1));
		/* DFS  - block  transactions */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DFS_REG, 0x2, 0x2));

		/* disable ODT in case of dll off */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1874, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1884, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1894, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x18a4, 0, 0x244));
		}

		/* DFS  - Enter Self-Refresh */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0x4,
			      0x4));
		/* polling on self refresh entry */
		if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST,
					if_id, 0x8, 0x8, DFS_REG,
					MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed on SR entry\n"));
		}

		/* Calculate 2T mode */
		if (mode_2t != 0xff) {
			t2t = mode_2t;
		} else if (timing != MV_DDR_TIM_DEFAULT) {
			t2t = (timing == MV_DDR_TIM_2T) ? 1 : 0;
		} else {
			/* Calculate number of CS per interface */
			cs_num = mv_ddr_cs_num_get();
			t2t = (cs_num == 1) ? 0 : 1;
		}


		if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_INTERLEAVE_WA) == 1) {
			/* Use 1T mode if 1:1 ratio configured */
			if (config_func_info[dev_num].tip_get_clock_ratio(frequency) == 1) {
				/* Low freq*/
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      SDRAM_OPEN_PAGES_CTRL_REG, 0x0, 0x3C0));
				t2t = 0;
			} else {
				/* Middle or target freq */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      SDRAM_OPEN_PAGES_CTRL_REG, 0x3C0, 0x3C0));
			}
		}
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
					       DUNIT_CTRL_LOW_REG, t2t << 3, 0x3 << 3));

		/* PLL configuration */
		config_func_info[dev_num].tip_set_freq_divider_func(dev_num, if_id,
								    frequency);

		/* DFS  - CL/CWL/WR parameters after exiting SR */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (cl_mask_table[cl_value] << 8), 0xf00));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (cwl_mask_table[cwl_value] << 12), 0x7000));

		t_ckclk = (MEGA / freq);
		t_wr = time_to_nclk(mv_ddr_speed_bin_timing_get
					   (speed_bin_index,
					    SPEED_BIN_TWR), t_ckclk);

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (twr_mask_table[t_wr] << 16), 0x70000));

		/* Restore original RTT values if returning from DLL OFF mode */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1874,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1884,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1894,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x18a4,
				      g_dic | g_rtt_nom, 0x266));
		}

		/* Reset divider_b assert -> de-assert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CFG_REG, 0, 0x10000000));
		mdelay(10);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CFG_REG, 0x10000000, 0x10000000));

		/* ADLL configuration function of process and frequency */
		CHECK_STATUS(config_func_info[dev_num].
			     tip_get_freq_config_info_func(dev_num, frequency,
							   &freq_config_info));

		/* TBD check milo5 using device ID ? */
		for (bus_cnt = 0; bus_cnt < octets_per_if_num;
		     bus_cnt++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, bus_cnt, DDR_PHY_DATA,
				      0x92,
				      freq_config_info.
				      bw_per_freq << 8
				      /*freq_mask[dev_num][frequency] << 8 */
				      , 0x700));
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      bus_cnt, DDR_PHY_DATA, 0x94,
				      freq_config_info.rate_per_freq, 0x7));
		}

		/* Dunit to PHY drive post edge, ADLL reset assert -> de-assert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DRAM_PHY_CFG_REG, 0,
			      (0x80000000 | 0x40000000)));
		mdelay(100 / (freq / mv_ddr_freq_get(MV_DDR_FREQ_LOW_FREQ)));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DRAM_PHY_CFG_REG, (0x80000000 | 0x40000000),
			      (0x80000000 | 0x40000000)));

		/* polling for ADLL Done */
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3ff03ff,
		     0x3ff03ff, PHY_LOCK_STATUS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(1)\n"));
		}

		/* pup data_pup reset assert-> deassert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CFG_REG, 0, 0x60000000));
		mdelay(10);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CFG_REG, 0x60000000, 0x60000000));

		/* Set proper timing params before existing Self-Refresh */
		ddr3_tip_set_timing(dev_num, access_type, if_id, frequency);
		if (delay_enable != 0) {
			adll_tap = (is_dll_off == 1) ? 1000 : (MEGA / (freq * 64));
			ddr3_tip_cmd_addr_init_delay(dev_num, adll_tap);
		}

		/* Exit SR */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0,
			      0x4));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x8, DFS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(2)"));
		}

		/* Refresh Command */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_OP_REG, 0x2, 0xf1f));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f,
		     SDRAM_OP_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(3)"));
		}

		/* Release DFS Block */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0,
			      0x2));
		/* Controller to MBUS Retry - normal */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DUNIT_MMASK_REG,
			      0x1, 0x1));

		/* MRO: Burst Length 8, CL , Auto_precharge 0x16cc */
		val =
			((cl_mask_table[cl_value] & 0x1) << 2) |
			((cl_mask_table[cl_value] & 0xe) << 3);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, MR0_REG,
			      val, (0x7 << 4) | (1 << 2)));
		/* MR2:  CWL = 10 , Auto Self-Refresh - disable */
		val = (cwl_mask_table[cwl_value] << 3) | g_rtt_wr;
		/*
		 * nklein 24.10.13 - should not be here - leave value as set in
		 * the init configuration val |= (1 << 9);
		 * val |= ((tm->interface_params[if_id].
		 * interface_temp == MV_DDR_TEMP_HIGH) ? (1 << 7) : 0);
		 */
		/* nklein 24.10.13 - see above comment */
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, MR2_REG,
					       val, (0x7 << 3) | (0x3 << 9)));

		/* ODT TIMING */
		val = ((cl_value - cwl_value + 1) << 4) |
			((cl_value - cwl_value + 6) << 8) |
			((cl_value - 1) << 12) | ((cl_value + 6) << 16);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, DDR_ODT_TIMING_LOW_REG,
					       val, 0xffff0));
		val = 0x91 | ((cwl_value - 1) << 8) | ((cwl_value + 5) << 12);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, DDR_ODT_TIMING_HIGH_REG,
					       val, 0xffff));

		/* in case of ddr4 need to set the receiver to odt always 'on' (odt_config = '0')
		 * in case of ddr3 configure the odt through the timing
		 */
		if (odt_config != 0) {
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id, DUNIT_ODT_CTRL_REG, 0xf, 0xf));
		}
		else {
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id, DUNIT_ODT_CTRL_REG,
						       0x30f, 0x30f));
		}

		/* re-write CL */
		val = ((cl_mask_table[cl_value] & 0x1) << 2) |
			((cl_mask_table[cl_value] & 0xe) << 3);

		CHECK_STATUS(ddr3_tip_write_mrs_cmd(dev_num, cs_mask, MR_CMD0,
			val, (0x7 << 4) | (0x1 << 2)));

		/* re-write CWL */
		val = (cwl_mask_table[cwl_value] << 3) | g_rtt_wr;
		CHECK_STATUS(ddr3_tip_write_mrs_cmd(dev_num, cs_mask, MR_CMD2,
			val, (0x7 << 3) | (0x3 << 9)));

		if (mem_mask != 0) {
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
						       if_id,
						       DUAL_DUNIT_CFG_REG,
						       1 << 3, 0x8));
		}
	}

	return MV_OK;
}

/*
 * Set ODT values
 */
static int ddr3_tip_write_odt(u32 dev_num, enum hws_access_type access_type,
			      u32 if_id, u32 cl_value, u32 cwl_value)
{
	/* ODT TIMING */
	u32 val = (cl_value - cwl_value + 6);

	val = ((cl_value - cwl_value + 1) << 4) | ((val & 0xf) << 8) |
		(((cl_value - 1) & 0xf) << 12) |
		(((cl_value + 6) & 0xf) << 16) | (((val & 0x10) >> 4) << 21);
	val |= (((cl_value - 1) >> 4) << 22) | (((cl_value + 6) >> 4) << 23);

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DDR_ODT_TIMING_LOW_REG, val, 0xffff0));
	val = 0x91 | ((cwl_value - 1) << 8) | ((cwl_value + 5) << 12);
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DDR_ODT_TIMING_HIGH_REG, val, 0xffff));
	if (odt_additional == 1) {
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id,
					       SDRAM_ODT_CTRL_HIGH_REG,
					       0xf, 0xf));
	}

	/* ODT Active */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DUNIT_ODT_CTRL_REG, 0xf, 0xf));

	return MV_OK;
}

/*
 * Set Timing values for training
 */
static int ddr3_tip_set_timing(u32 dev_num, enum hws_access_type access_type,
			       u32 if_id, enum mv_ddr_freq frequency)
{
	u32 t_ckclk = 0, t_ras = 0;
	u32 t_rcd = 0, t_rp = 0, t_wr = 0, t_wtr = 0, t_rrd = 0, t_rtp = 0,
		t_rfc = 0, t_mod = 0, t_r2r = 0x3, t_r2r_high = 0,
		t_r2w_w2r = 0x3, t_r2w_w2r_high = 0x1, t_w2w = 0x3;
	u32 refresh_interval_cnt, t_hclk, t_refi, t_faw, t_pd, t_xpdll;
	u32 val = 0, page_size = 0, mask = 0;
	enum mv_ddr_speed_bin speed_bin_index;
	enum mv_ddr_die_capacity memory_size = MV_DDR_DIE_CAP_2GBIT;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 freq = mv_ddr_freq_get(frequency);

	speed_bin_index = tm->interface_params[if_id].speed_bin_index;
	memory_size = tm->interface_params[if_id].memory_size;
	page_size = mv_ddr_page_size_get(tm->interface_params[if_id].bus_width, memory_size);
	t_ckclk = (MEGA / freq);
	/* HCLK in[ps] */
	t_hclk = MEGA / (freq / config_func_info[dev_num].tip_get_clock_ratio(frequency));

	t_refi = (tm->interface_params[if_id].interface_temp == MV_DDR_TEMP_HIGH) ? TREFI_HIGH : TREFI_LOW;
	t_refi *= 1000;	/* psec */
	refresh_interval_cnt = t_refi / t_hclk;	/* no units */

	if (page_size == 1) {
		t_faw = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TFAW1K);
		t_faw = time_to_nclk(t_faw, t_ckclk);
		t_faw = GET_MAX_VALUE(20, t_faw);
	} else {	/* page size =2, we do not support page size 0.5k */
		t_faw = mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TFAW2K);
		t_faw = time_to_nclk(t_faw, t_ckclk);
		t_faw = GET_MAX_VALUE(28, t_faw);
	}

	t_pd = GET_MAX_VALUE(t_ckclk * 3, mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TPD));
	t_pd = time_to_nclk(t_pd, t_ckclk);

	t_xpdll = GET_MAX_VALUE(t_ckclk * 10, mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TXPDLL));
	t_xpdll = time_to_nclk(t_xpdll, t_ckclk);

	t_rrd =	(page_size == 1) ? mv_ddr_speed_bin_timing_get(speed_bin_index,
						   SPEED_BIN_TRRD1K) :
		mv_ddr_speed_bin_timing_get(speed_bin_index, SPEED_BIN_TRRD2K);
	t_rrd = GET_MAX_VALUE(t_ckclk * 4, t_rrd);
	t_rtp =	GET_MAX_VALUE(t_ckclk * 4, mv_ddr_speed_bin_timing_get(speed_bin_index,
							   SPEED_BIN_TRTP));
	t_mod = GET_MAX_VALUE(t_ckclk * 12, 15000);
	t_wtr = GET_MAX_VALUE(t_ckclk * 4, mv_ddr_speed_bin_timing_get(speed_bin_index,
							   SPEED_BIN_TWTR));
	t_ras = time_to_nclk(mv_ddr_speed_bin_timing_get(speed_bin_index,
						    SPEED_BIN_TRAS),
				    t_ckclk);
	t_rcd = time_to_nclk(mv_ddr_speed_bin_timing_get(speed_bin_index,
						    SPEED_BIN_TRCD),
				    t_ckclk);
	t_rp = time_to_nclk(mv_ddr_speed_bin_timing_get(speed_bin_index,
						   SPEED_BIN_TRP),
				   t_ckclk);
	t_wr = time_to_nclk(mv_ddr_speed_bin_timing_get(speed_bin_index,
						   SPEED_BIN_TWR),
				   t_ckclk);
	t_wtr = time_to_nclk(t_wtr, t_ckclk);
	t_rrd = time_to_nclk(t_rrd, t_ckclk);
	t_rtp = time_to_nclk(t_rtp, t_ckclk);
	t_rfc = time_to_nclk(mv_ddr_rfc_get(memory_size) * 1000, t_ckclk);
	t_mod = time_to_nclk(t_mod, t_ckclk);

	/* SDRAM Timing Low */
	val = (((t_ras - 1) & SDRAM_TIMING_LOW_TRAS_MASK) << SDRAM_TIMING_LOW_TRAS_OFFS) |
	      (((t_rcd - 1) & SDRAM_TIMING_LOW_TRCD_MASK) << SDRAM_TIMING_LOW_TRCD_OFFS) |
	      (((t_rcd - 1) >> SDRAM_TIMING_LOW_TRCD_OFFS & SDRAM_TIMING_HIGH_TRCD_MASK)
	      << SDRAM_TIMING_HIGH_TRCD_OFFS) |
	      (((t_rp - 1) & SDRAM_TIMING_LOW_TRP_MASK) << SDRAM_TIMING_LOW_TRP_OFFS) |
	      (((t_rp - 1) >> SDRAM_TIMING_LOW_TRP_MASK & SDRAM_TIMING_HIGH_TRP_MASK)
	      << SDRAM_TIMING_HIGH_TRP_OFFS) |
	      (((t_wr - 1) & SDRAM_TIMING_LOW_TWR_MASK) << SDRAM_TIMING_LOW_TWR_OFFS) |
	      (((t_wtr - 1) & SDRAM_TIMING_LOW_TWTR_MASK) << SDRAM_TIMING_LOW_TWTR_OFFS) |
	      ((((t_ras - 1) >> 4) & SDRAM_TIMING_LOW_TRAS_HIGH_MASK) << SDRAM_TIMING_LOW_TRAS_HIGH_OFFS) |
	      (((t_rrd - 1) & SDRAM_TIMING_LOW_TRRD_MASK) << SDRAM_TIMING_LOW_TRRD_OFFS) |
	      (((t_rtp - 1) & SDRAM_TIMING_LOW_TRTP_MASK) << SDRAM_TIMING_LOW_TRTP_OFFS);

	mask = (SDRAM_TIMING_LOW_TRAS_MASK << SDRAM_TIMING_LOW_TRAS_OFFS) |
	       (SDRAM_TIMING_LOW_TRCD_MASK << SDRAM_TIMING_LOW_TRCD_OFFS) |
	       (SDRAM_TIMING_HIGH_TRCD_MASK << SDRAM_TIMING_HIGH_TRCD_OFFS) |
	       (SDRAM_TIMING_LOW_TRP_MASK << SDRAM_TIMING_LOW_TRP_OFFS) |
	       (SDRAM_TIMING_HIGH_TRP_MASK << SDRAM_TIMING_HIGH_TRP_OFFS) |
	       (SDRAM_TIMING_LOW_TWR_MASK << SDRAM_TIMING_LOW_TWR_OFFS) |
	       (SDRAM_TIMING_LOW_TWTR_MASK << SDRAM_TIMING_LOW_TWTR_OFFS) |
	       (SDRAM_TIMING_LOW_TRAS_HIGH_MASK << SDRAM_TIMING_LOW_TRAS_HIGH_OFFS) |
	       (SDRAM_TIMING_LOW_TRRD_MASK << SDRAM_TIMING_LOW_TRRD_OFFS) |
	       (SDRAM_TIMING_LOW_TRTP_MASK << SDRAM_TIMING_LOW_TRTP_OFFS);

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_LOW_REG, val, mask));

	/* SDRAM Timing High */
	val = 0;
	mask = 0;

	val = (((t_rfc - 1) & SDRAM_TIMING_HIGH_TRFC_MASK) << SDRAM_TIMING_HIGH_TRFC_OFFS) |
	      ((t_r2r & SDRAM_TIMING_HIGH_TR2R_MASK) << SDRAM_TIMING_HIGH_TR2R_OFFS) |
	      ((t_r2w_w2r & SDRAM_TIMING_HIGH_TR2W_W2R_MASK) << SDRAM_TIMING_HIGH_TR2W_W2R_OFFS) |
	      ((t_w2w & SDRAM_TIMING_HIGH_TW2W_MASK) << SDRAM_TIMING_HIGH_TW2W_OFFS) |
	      ((((t_rfc - 1) >> 7) & SDRAM_TIMING_HIGH_TRFC_HIGH_MASK) << SDRAM_TIMING_HIGH_TRFC_HIGH_OFFS) |
	      ((t_r2r_high & SDRAM_TIMING_HIGH_TR2R_HIGH_MASK) << SDRAM_TIMING_HIGH_TR2R_HIGH_OFFS) |
	      ((t_r2w_w2r_high & SDRAM_TIMING_HIGH_TR2W_W2R_HIGH_MASK) << SDRAM_TIMING_HIGH_TR2W_W2R_HIGH_OFFS) |
	      (((t_mod - 1) & SDRAM_TIMING_HIGH_TMOD_MASK) << SDRAM_TIMING_HIGH_TMOD_OFFS) |
	      ((((t_mod - 1) >> 4) & SDRAM_TIMING_HIGH_TMOD_HIGH_MASK) << SDRAM_TIMING_HIGH_TMOD_HIGH_OFFS);

	mask = (SDRAM_TIMING_HIGH_TRFC_MASK << SDRAM_TIMING_HIGH_TRFC_OFFS) |
	       (SDRAM_TIMING_HIGH_TR2R_MASK << SDRAM_TIMING_HIGH_TR2R_OFFS) |
	       (SDRAM_TIMING_HIGH_TR2W_W2R_MASK << SDRAM_TIMING_HIGH_TR2W_W2R_OFFS) |
	       (SDRAM_TIMING_HIGH_TW2W_MASK << SDRAM_TIMING_HIGH_TW2W_OFFS) |
	       (SDRAM_TIMING_HIGH_TRFC_HIGH_MASK << SDRAM_TIMING_HIGH_TRFC_HIGH_OFFS) |
	       (SDRAM_TIMING_HIGH_TR2R_HIGH_MASK << SDRAM_TIMING_HIGH_TR2R_HIGH_OFFS) |
	       (SDRAM_TIMING_HIGH_TR2W_W2R_HIGH_MASK << SDRAM_TIMING_HIGH_TR2W_W2R_HIGH_OFFS) |
	       (SDRAM_TIMING_HIGH_TMOD_MASK << SDRAM_TIMING_HIGH_TMOD_OFFS) |
	       (SDRAM_TIMING_HIGH_TMOD_HIGH_MASK << SDRAM_TIMING_HIGH_TMOD_HIGH_OFFS);

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG, val, mask));

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_CFG_REG,
				       refresh_interval_cnt << REFRESH_OFFS,
				       REFRESH_MASK << REFRESH_OFFS));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_ADDR_CTRL_REG, (t_faw - 1) << T_FAW_OFFS,
				       T_FAW_MASK << T_FAW_OFFS));

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id, DDR_TIMING_REG,
				       (t_pd - 1) << DDR_TIMING_TPD_OFFS |
				       (t_xpdll - 1) << DDR_TIMING_TXPDLL_OFFS,
				       DDR_TIMING_TPD_MASK << DDR_TIMING_TPD_OFFS |
				       DDR_TIMING_TXPDLL_MASK << DDR_TIMING_TXPDLL_OFFS));


	return MV_OK;
}


/*
 * Write CS Result
 */
int ddr3_tip_write_cs_result(u32 dev_num, u32 offset)
{
	u32 if_id, bus_num, cs_bitmask, data_val, cs_num;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_num = 0; bus_num < octets_per_if_num;
		     bus_num++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_num);
			cs_bitmask =
				tm->interface_params[if_id].
				as_bus_params[bus_num].cs_bitmask;
			if (cs_bitmask != effective_cs) {
				cs_num = GET_CS_FROM_MASK(cs_bitmask);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_num,
						  DDR_PHY_DATA,
						  offset +
						  (effective_cs * 0x4),
						  &data_val);
				ddr3_tip_bus_write(dev_num,
						   ACCESS_TYPE_UNICAST,
						   if_id,
						   ACCESS_TYPE_UNICAST,
						   bus_num, DDR_PHY_DATA,
						   offset +
						   (cs_num * 0x4),
						   data_val);
			}
		}
	}

	return MV_OK;
}

/*
 * Write MRS
 */
int ddr3_tip_write_mrs_cmd(u32 dev_num, u32 *cs_mask_arr, enum mr_number mr_num, u32 data, u32 mask)
{
	u32 if_id;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, mr_data[mr_num].reg_addr, data, mask));
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_OP_REG,
			      (cs_mask_arr[if_id] << 8) | mr_data[mr_num].cmd, 0xf1f));
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0,
					0x1f, SDRAM_OP_REG,
					MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("write_mrs_cmd: Poll cmd fail"));
		}
	}

	return MV_OK;
}

/*
 * Reset XSB Read FIFO
 */
int ddr3_tip_reset_fifo_ptr(u32 dev_num)
{
	u32 if_id = 0;

	/* Configure PHY reset value to 0 in order to "clean" the FIFO */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15c8, 0, 0xff000000));
	/*
	 * Move PHY to RL mode (only in RL mode the PHY overrides FIFO values
	 * during FIFO reset)
	 */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, TRAINING_SW_2_REG,
				       0x1, 0x9));
	/* In order that above configuration will influence the PHY */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15b0,
				       0x80000000, 0x80000000));
	/* Reset read fifo assertion */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x1400, 0, 0x40000000));
	/* Reset read fifo deassertion */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x1400,
				       0x40000000, 0x40000000));
	/* Move PHY back to functional mode */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, TRAINING_SW_2_REG,
				       0x8, 0x9));
	/* Stop training machine */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15b4, 0x10000, 0x10000));

	return MV_OK;
}

/*
 * Reset Phy registers
 */
int ddr3_tip_ddr3_reset_phy_regs(u32 dev_num)
{
	u32 if_id, phy_id, cs;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (phy_id = 0; phy_id < octets_per_if_num;
		     phy_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, phy_id);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, ACCESS_TYPE_UNICAST,
				      phy_id, DDR_PHY_DATA,
				      WL_PHY_REG(effective_cs),
				      phy_reg0_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      RL_PHY_REG(effective_cs),
				      phy_reg2_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      CRX_PHY_REG(effective_cs), phy_reg3_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      CTX_PHY_REG(effective_cs), phy_reg1_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_TX_BCAST_PHY_REG(effective_cs), 0x0));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_RX_BCAST_PHY_REG(effective_cs), 0));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_TX_PHY_REG(effective_cs, DQSP_PAD), 0));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_RX_PHY_REG(effective_cs, DQSP_PAD), 0));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_TX_PHY_REG(effective_cs, DQSN_PAD), 0));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      PBS_RX_PHY_REG(effective_cs, DQSN_PAD), 0));
		}
	}

	/* Set Receiver Calibration value */
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		/* PHY register 0xdb bits[5:0] - configure to 63 */
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      DDR_PHY_DATA, VREF_BCAST_PHY_REG(cs), 63));
	}

	return MV_OK;
}

/*
 * Restore Dunit registers
 */
int ddr3_tip_restore_dunit_regs(u32 dev_num)
{
	u32 index_cnt;

	mv_ddr_set_calib_controller();

	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
				       0x1, 0x1));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
				       calibration_update_control << 3,
				       0x3 << 3));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE,
				       ODPG_WR_RD_MODE_ENA_REG,
				       0xffff, MASK_ALL_BITS));

	for (index_cnt = 0; index_cnt < ARRAY_SIZE(odpg_default_value);
	     index_cnt++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      odpg_default_value[index_cnt].reg_addr,
			      odpg_default_value[index_cnt].reg_data,
			      odpg_default_value[index_cnt].reg_mask));
	}

	return MV_OK;
}

int ddr3_tip_adll_regs_bypass(u32 dev_num, u32 reg_val1, u32 reg_val2)
{
	u32 if_id, phy_id;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (phy_id = 0; phy_id < octets_per_if_num; phy_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, phy_id);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				     ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				     CTX_PHY_REG(effective_cs), reg_val1));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				     ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				     PBS_TX_BCAST_PHY_REG(effective_cs), reg_val2));
		}
	}

	return MV_OK;
}

/*
 * Auto tune main flow
 */
static int ddr3_tip_ddr3_training_main_flow(u32 dev_num)
{
/* TODO: enable this functionality for other platforms */
#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
	struct init_cntr_param init_cntr_prm;
#endif
	int ret = MV_OK;
	int adll_bypass_flag = 0;
	u32 if_id;
	unsigned int max_cs = mv_ddr_cs_num_get();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum mv_ddr_freq freq = tm->interface_params[0].memory_freq;
	unsigned int *freq_tbl = mv_ddr_freq_tbl_get();

#ifdef DDR_VIEWER_TOOL
	if (debug_training == DEBUG_LEVEL_TRACE) {
		CHECK_STATUS(print_device_info((u8)dev_num));
	}
#endif

	ddr3_tip_validate_algo_components(dev_num);

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		CHECK_STATUS(ddr3_tip_ddr3_reset_phy_regs(dev_num));
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	freq_tbl[MV_DDR_FREQ_LOW_FREQ] = dfs_low_freq;

	if (is_pll_before_init != 0) {
		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			config_func_info[dev_num].tip_set_freq_divider_func(
				(u8)dev_num, if_id, freq);
		}
	}

/* TODO: enable this functionality for other platforms */
#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
	if (is_adll_calib_before_init != 0) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("with adll calib before init\n"));
		adll_calibration(dev_num, ACCESS_TYPE_MULTICAST, 0, freq);
	}

	if (is_reg_dump != 0) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("Dump before init controller\n"));
		ddr3_tip_reg_dump(dev_num);
	}

	if (mask_tune_func & INIT_CONTROLLER_MASK_BIT) {
		training_stage = INIT_CONTROLLER;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("INIT_CONTROLLER_MASK_BIT\n"));
		init_cntr_prm.do_mrs_phy = 1;
		init_cntr_prm.is_ctrl64_bit = 0;
		init_cntr_prm.init_phy = 1;
		init_cntr_prm.msys_init = 0;
		ret = hws_ddr3_tip_init_controller(dev_num, &init_cntr_prm);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("hws_ddr3_tip_init_controller failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}
#endif

	ret = adll_calibration(dev_num, ACCESS_TYPE_MULTICAST, 0, freq);
	if (ret != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
			("adll_calibration failure\n"));
		if (debug_mode == 0)
			return MV_FAIL;
	}

	if (mask_tune_func & SET_LOW_FREQ_MASK_BIT) {
		training_stage = SET_LOW_FREQ;

		for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
			ddr3_tip_adll_regs_bypass(dev_num, 0, 0x1f);
			adll_bypass_flag = 1;
		}
		effective_cs = 0;

		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_LOW_FREQ_MASK_BIT %d\n",
				   freq_tbl[low_freq]));
		ret = ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, low_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_LF_MASK_BIT) {
		training_stage = WRITE_LEVELING_LF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			("WRITE_LEVELING_LF_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_write_leveling(dev_num, 1);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				("ddr3_tip_dynamic_write_leveling LF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & LOAD_PATTERN_MASK_BIT) {
			training_stage = LOAD_PATTERN;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("LOAD_PATTERN_MASK_BIT #%d\n",
					   effective_cs));
			ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_load_all_pattern_to_mem failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}

	if (adll_bypass_flag == 1) {
		for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
			ddr3_tip_adll_regs_bypass(dev_num, phy_reg1_val, 0);
			adll_bypass_flag = 0;
		}
	}

	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & SET_MEDIUM_FREQ_MASK_BIT) {
		training_stage = SET_MEDIUM_FREQ;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_MEDIUM_FREQ_MASK_BIT %d\n",
				   freq_tbl[medium_freq]));
		ret =
			ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					  PARAM_NOT_CARE, medium_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_MASK_BIT) {
		training_stage = WRITE_LEVELING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_MASK_BIT\n"));
		if ((rl_mid_freq_wa == 0) || (freq_tbl[medium_freq] == 533)) {
			ret = ddr3_tip_dynamic_write_leveling(dev_num, 0);
		} else {
			/* Use old WL */
			ret = ddr3_tip_legacy_dynamic_write_leveling(dev_num);
		}

		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & LOAD_PATTERN_2_MASK_BIT) {
			training_stage = LOAD_PATTERN_2;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("LOAD_PATTERN_2_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_load_all_pattern_to_mem failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & READ_LEVELING_MASK_BIT) {
		training_stage = READ_LEVELING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("READ_LEVELING_MASK_BIT\n"));
		if ((rl_mid_freq_wa == 0) || (freq_tbl[medium_freq] == 533)) {
			ret = ddr3_tip_dynamic_read_leveling(dev_num, medium_freq);
		} else {
			/* Use old RL */
			ret = ddr3_tip_legacy_dynamic_read_leveling(dev_num);
		}

		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_read_leveling failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_SUPP_MASK_BIT) {
		training_stage = WRITE_LEVELING_SUPP;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_SUPP_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_write_leveling_supp(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling_supp failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & PBS_RX_MASK_BIT) {
			training_stage = PBS_RX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("PBS_RX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_pbs_rx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_pbs_rx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & PBS_TX_MASK_BIT) {
			training_stage = PBS_TX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("PBS_TX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_pbs_tx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_pbs_tx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & SET_TARGET_FREQ_MASK_BIT) {
		training_stage = SET_TARGET_FREQ;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_TARGET_FREQ_MASK_BIT %d\n",
				   freq_tbl[tm->
					    interface_params[first_active_if].
					    memory_freq]));
		ret = ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE,
					tm->interface_params[first_active_if].
					memory_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_TF_MASK_BIT) {
		training_stage = WRITE_LEVELING_TF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_TF_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_write_leveling(dev_num, 0);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling TF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & LOAD_PATTERN_HIGH_MASK_BIT) {
		training_stage = LOAD_PATTERN_HIGH;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("LOAD_PATTERN_HIGH\n"));
		ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_load_all_pattern_to_mem failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & READ_LEVELING_TF_MASK_BIT) {
		training_stage = READ_LEVELING_TF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("READ_LEVELING_TF_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_read_leveling(dev_num, tm->
						     interface_params[first_active_if].
						     memory_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_read_leveling TF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & RL_DQS_BURST_MASK_BIT) {
		training_stage = READ_LEVELING_TF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("RL_DQS_BURST_MASK_BIT\n"));
		ret = mv_ddr_rl_dqs_burst(0, 0, tm->interface_params[0].memory_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("mv_ddr_rl_dqs_burst TF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & DM_PBS_TX_MASK_BIT) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("DM_PBS_TX_MASK_BIT\n"));
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & VREF_CALIBRATION_MASK_BIT) {
			training_stage = VREF_CALIBRATION;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("VREF\n"));
			ret = ddr3_tip_vref(dev_num);
			if (is_reg_dump != 0) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("VREF Dump\n"));
				ddr3_tip_reg_dump(dev_num);
			}
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_vref failure\n"));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & CENTRALIZATION_RX_MASK_BIT) {
			training_stage = CENTRALIZATION_RX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("CENTRALIZATION_RX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_centralization_rx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_centralization_rx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & WRITE_LEVELING_SUPP_TF_MASK_BIT) {
			training_stage = WRITE_LEVELING_SUPP_TF;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("WRITE_LEVELING_SUPP_TF_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_dynamic_write_leveling_supp(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_dynamic_write_leveling_supp TF failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;


	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & CENTRALIZATION_TX_MASK_BIT) {
			training_stage = CENTRALIZATION_TX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("CENTRALIZATION_TX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_centralization_tx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_centralization_tx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("restore registers to default\n"));
	/* restore register values */
	CHECK_STATUS(ddr3_tip_restore_dunit_regs(dev_num));

	if (is_reg_dump != 0)
		ddr3_tip_reg_dump(dev_num);

	return MV_OK;
}

/*
 * DDR3 Dynamic training flow
 */
static int ddr3_tip_ddr3_auto_tune(u32 dev_num)
{
	int status;
	u32 if_id, stage;
	int is_if_fail = 0, is_auto_tune_fail = 0;

	training_stage = INIT_CONTROLLER;

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		for (stage = 0; stage < MAX_STAGE_LIMIT; stage++)
			training_result[stage][if_id] = NO_TEST_DONE;
	}

	status = ddr3_tip_ddr3_training_main_flow(dev_num);

	/* activate XSB test */
	if (xsb_validate_type != 0) {
		run_xsb_test(dev_num, xsb_validation_base_address, 1, 1,
			     0x1024);
	}

	if (is_reg_dump != 0)
		ddr3_tip_reg_dump(dev_num);

	/* print log */
	CHECK_STATUS(ddr3_tip_print_log(dev_num, window_mem_addr));

#ifndef EXCLUDE_DEBUG_PRINTS
	if (status != MV_OK) {
		CHECK_STATUS(ddr3_tip_print_stability_log(dev_num));
	}
#endif /* EXCLUDE_DEBUG_PRINTS */

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		is_if_fail = 0;
		for (stage = 0; stage < MAX_STAGE_LIMIT; stage++) {
			if (training_result[stage][if_id] == TEST_FAILED)
				is_if_fail = 1;
		}
		if (is_if_fail == 1) {
			is_auto_tune_fail = 1;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("Auto Tune failed for IF %d\n",
					   if_id));
		}
	}

	if (((status == MV_FAIL) && (is_auto_tune_fail == 0)) ||
	    ((status == MV_OK) && (is_auto_tune_fail == 1))) {
		/*
		 * If MainFlow result and trainingResult DB not in sync,
		 * issue warning (caused by no update of trainingResult DB
		 * when failed)
		 */
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("Warning: Algorithm return value and Result DB"
				   "are not synced (status 0x%x  result DB %d)\n",
				   status, is_auto_tune_fail));
	}

	if ((status != MV_OK) || (is_auto_tune_fail == 1))
		return MV_FAIL;
	else
		return MV_OK;
}

/*
 * Enable init sequence
 */
int ddr3_tip_enable_init_sequence(u32 dev_num)
{
	int is_fail = 0;
	u32 if_id = 0, mem_mask = 0, bus_index = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* Enable init sequence */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, 0,
				       SDRAM_INIT_CTRL_REG, 0x1, 0x1));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1,
		     SDRAM_INIT_CTRL_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("polling failed IF %d\n",
					   if_id));
			is_fail = 1;
			continue;
		}

		mem_mask = 0;
		for (bus_index = 0; bus_index < octets_per_if_num;
		     bus_index++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_index);
			mem_mask |=
				tm->interface_params[if_id].
				as_bus_params[bus_index].mirror_enable_bitmask;
		}

		if (mem_mask != 0) {
			/* Disable Multi CS */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_MULTICAST,
				      if_id, DUAL_DUNIT_CFG_REG, 1 << 3,
				      1 << 3));
		}
	}

	return (is_fail == 0) ? MV_OK : MV_FAIL;
}

int ddr3_tip_register_dq_table(u32 dev_num, u32 *table)
{
	dq_map_table = table;

	return MV_OK;
}

/*
 * Check if pup search is locked
 */
int ddr3_tip_is_pup_lock(u32 *pup_buf, enum hws_training_result read_mode)
{
	u32 bit_start = 0, bit_end = 0, bit_id;

	if (read_mode == RESULT_PER_BIT) {
		bit_start = 0;
		bit_end = BUS_WIDTH_IN_BITS - 1;
	} else {
		bit_start = 0;
		bit_end = 0;
	}

	for (bit_id = bit_start; bit_id <= bit_end; bit_id++) {
		if (GET_LOCK_RESULT(pup_buf[bit_id]) == 0)
			return 0;
	}

	return 1;
}

/*
 * Get minimum buffer value
 */
u8 ddr3_tip_get_buf_min(u8 *buf_ptr)
{
	u8 min_val = 0xff;
	u8 cnt = 0;

	for (cnt = 0; cnt < BUS_WIDTH_IN_BITS; cnt++) {
		if (buf_ptr[cnt] < min_val)
			min_val = buf_ptr[cnt];
	}

	return min_val;
}

/*
 * Get maximum buffer value
 */
u8 ddr3_tip_get_buf_max(u8 *buf_ptr)
{
	u8 max_val = 0;
	u8 cnt = 0;

	for (cnt = 0; cnt < BUS_WIDTH_IN_BITS; cnt++) {
		if (buf_ptr[cnt] > max_val)
			max_val = buf_ptr[cnt];
	}

	return max_val;
}

/*
 * The following functions return memory parameters:
 * bus and device width, device size
 */

u32 hws_ddr3_get_bus_width(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	return (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask) ==
		1) ? 16 : 32;
}

u32 hws_ddr3_get_device_width(u32 if_id)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	return (tm->interface_params[if_id].bus_width ==
		MV_DDR_DEV_WIDTH_8BIT) ? 8 : 16;
}

u32 hws_ddr3_get_device_size(u32 if_id)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (tm->interface_params[if_id].memory_size >=
	    MV_DDR_DIE_CAP_LAST) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Error: Wrong device size of Cs: %d",
				   tm->interface_params[if_id].memory_size));
		return 0;
	} else {
		return 1 << tm->interface_params[if_id].memory_size;
	}
}

int hws_ddr3_calc_mem_cs_size(u32 if_id, u32 cs, u32 *cs_size)
{
	u32 cs_mem_size, dev_size;

	dev_size = hws_ddr3_get_device_size(if_id);
	if (dev_size != 0) {
		cs_mem_size = ((hws_ddr3_get_bus_width() /
				hws_ddr3_get_device_width(if_id)) * dev_size);

		/* the calculated result in Gbytex16 to avoid float using */

		if (cs_mem_size == 2) {
			*cs_size = _128M;
		} else if (cs_mem_size == 4) {
			*cs_size = _256M;
		} else if (cs_mem_size == 8) {
			*cs_size = _512M;
		} else if (cs_mem_size == 16) {
			*cs_size = _1G;
		} else if (cs_mem_size == 32) {
			*cs_size = _2G;
		} else {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Error: Wrong Memory size of Cs: %d", cs));
			return MV_FAIL;
		}
		return MV_OK;
	} else {
		return MV_FAIL;
	}
}

int hws_ddr3_cs_base_adr_calc(u32 if_id, u32 cs, u32 *cs_base_addr)
{
	u32 cs_mem_size = 0;
#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	u32 physical_mem_size;
	u32 max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE;
#endif

	if (hws_ddr3_calc_mem_cs_size(if_id, cs, &cs_mem_size) != MV_OK)
		return MV_FAIL;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	/*
	 * if number of address pins doesn't allow to use max mem size that
	 * is defined in topology mem size is defined by
	 * DEVICE_MAX_DRAM_ADDRESS_SIZE
	 */
	physical_mem_size = mem_size[tm->interface_params[0].memory_size];

	if (hws_ddr3_get_device_width(cs) == 16) {
		/*
		 * 16bit mem device can be twice more - no need in less
		 * significant pin
		 */
		max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE * 2;
	}

	if (physical_mem_size > max_mem_size) {
		cs_mem_size = max_mem_size *
			(hws_ddr3_get_bus_width() /
			 hws_ddr3_get_device_width(if_id));
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Updated Physical Mem size is from 0x%x to %x\n",
				   physical_mem_size,
				   DEVICE_MAX_DRAM_ADDRESS_SIZE));
	}
#endif

	/* calculate CS base addr */
	*cs_base_addr = ((cs_mem_size) * cs) & 0xffff0000;

	return MV_OK;
}

/* TODO: consider to move to misl phy driver */
enum {
	MISL_PHY_DRV_OHM_30 = 0xf,
	MISL_PHY_DRV_OHM_48 = 0xa,
	MISL_PHY_DRV_OHM_80 = 0x6,
	MISL_PHY_DRV_OHM_120 = 0x4
};

enum {
	MISL_PHY_ODT_OHM_60 = 0x8,
	MISL_PHY_ODT_OHM_80 = 0x6,
	MISL_PHY_ODT_OHM_120 = 0x4,
	MISL_PHY_ODT_OHM_240 = 0x2
};

static unsigned int mv_ddr_misl_phy_drv_calc(unsigned int cfg)
{
	unsigned int val;

	switch (cfg) {
	case MV_DDR_OHM_30:
		val = MISL_PHY_DRV_OHM_30;
		break;
	case MV_DDR_OHM_48:
		val = MISL_PHY_DRV_OHM_48;
		break;
	case MV_DDR_OHM_80:
		val = MISL_PHY_DRV_OHM_80;
		break;
	case MV_DDR_OHM_120:
		val = MISL_PHY_DRV_OHM_120;
		break;
	default:
		val = PARAM_UNDEFINED;
	}

	return val;
}

static unsigned int mv_ddr_misl_phy_odt_calc(unsigned int cfg)
{
	unsigned int val;

	switch (cfg) {
	case MV_DDR_OHM_60:
		val = MISL_PHY_ODT_OHM_60;
		break;
	case MV_DDR_OHM_80:
		val = MISL_PHY_ODT_OHM_80;
		break;
	case MV_DDR_OHM_120:
		val = MISL_PHY_ODT_OHM_120;
		break;
	case MV_DDR_OHM_240:
		val = MISL_PHY_ODT_OHM_240;
		break;
	default:
		val = PARAM_UNDEFINED;
	}

	return val;
}

unsigned int mv_ddr_misl_phy_drv_data_p_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int drv_data_p = mv_ddr_misl_phy_drv_calc(tm->edata.phy_edata.drv_data_p);

	if (drv_data_p == PARAM_UNDEFINED)
		printf("error: %s: unsupported drv_data_p parameter found\n", __func__);

	return drv_data_p;
}

unsigned int mv_ddr_misl_phy_drv_data_n_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int drv_data_n = mv_ddr_misl_phy_drv_calc(tm->edata.phy_edata.drv_data_n);

	if (drv_data_n == PARAM_UNDEFINED)
		printf("error: %s: unsupported drv_data_n parameter found\n", __func__);

	return drv_data_n;
}

unsigned int mv_ddr_misl_phy_drv_ctrl_p_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int drv_ctrl_p = mv_ddr_misl_phy_drv_calc(tm->edata.phy_edata.drv_ctrl_p);

	if (drv_ctrl_p == PARAM_UNDEFINED)
		printf("error: %s: unsupported drv_ctrl_p parameter found\n", __func__);

	return drv_ctrl_p;
}

unsigned int mv_ddr_misl_phy_drv_ctrl_n_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int drv_ctrl_n = mv_ddr_misl_phy_drv_calc(tm->edata.phy_edata.drv_ctrl_n);

	if (drv_ctrl_n == PARAM_UNDEFINED)
		printf("error: %s: unsupported drv_ctrl_n parameter found\n", __func__);

	return drv_ctrl_n;
}

unsigned int mv_ddr_misl_phy_odt_p_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int odt_p = PARAM_UNDEFINED;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		odt_p = mv_ddr_misl_phy_odt_calc(tm->edata.phy_edata.odt_p[cs_num - 1]);

	if (odt_p == PARAM_UNDEFINED)
		printf("error: %s: unsupported odt_p parameter found\n", __func__);

	return odt_p;
}

unsigned int mv_ddr_misl_phy_odt_n_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int odt_n = PARAM_UNDEFINED;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		odt_n = mv_ddr_misl_phy_odt_calc(tm->edata.phy_edata.odt_n[cs_num - 1]);

	if (odt_n == PARAM_UNDEFINED)
		printf("error: %s: unsupported odt_n parameter found\n", __func__);

	return odt_n;
}
