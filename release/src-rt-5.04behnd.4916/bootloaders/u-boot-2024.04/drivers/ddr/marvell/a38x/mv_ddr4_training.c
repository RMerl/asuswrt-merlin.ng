// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#if defined(CONFIG_DDR4)

/* DDR4 training service API and data structures */

#include "ddr3_init.h"
#include "mv_ddr4_training.h"
#include "mv_ddr4_mpr_pda_if.h"
#include "mv_ddr4_training_leveling.h"
#include "mv_ddr4_training_calibration.h"
#include "mv_ddr_regs.h"

/* 1 for wa and sstl and pod to get the same vref value */
u8 vref_calibration_wa = 1;

static int a39x_z1_config(u32 dev_num);

/* vref values for vcommon */
static u16 vref_val[] = {
	746,
	654,
	671,
	686,
	701,
	713,
	725,
	736
};

static u32 mv_ddr4_config_phy_vref_tap;

/* configure DDR4 SDRAM */
int mv_ddr4_sdram_config(u32 dev_num)
{
	/* TODO: zq params to be frequency dependent */
	u32 zq_init = 1023;
	u32 zq_oper = 511;
	u32 zq_cs = 127;
	u32 if_id;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int status;

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		/* dtype: 0x3 for DDR4, 0x1 for DDR3 */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, SDRAM_CFG_REG,
					   (0x1 << 14) | (0x1 << 20), (0x1 << 14) | (0x1 << 20));
		if (status != MV_OK)
			return status;

		/* cpm */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DRAM_PINS_MUX_REG,
					   0x2, 0x3);
		if (status != MV_OK)
			return status;

		/*
		 * set t_dllk to 1024 to the maximum of minimum for high speed bin
		 * TODO: may change for future speed bins
		 */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DRAM_DLL_TIMING_REG,
					   0x400, 0xfff);
		if (status != MV_OK)
			return status;

		/* set zq_init */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DRAM_ZQ_INIT_TIMIMG_REG,
					   zq_init, 0xfff);
		if (status != MV_OK)
			return status;

		/* set zq_oper */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DRAM_ZQ_TIMING_REG,
					   zq_oper, 0x7ff);
		if (status != MV_OK)
			return status;

		/* set zq_cs */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DRAM_ZQ_TIMING_REG,
					   zq_cs << 16, 0x3ff0000);
		if (status != MV_OK)
			return status;

		/*
		 * set registered dimm to unbuffered dimm
		 * TODO: support registered dimm
		 */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, SDRAM_CFG_REG,
					   0x0, 0x1 << 17);
		if (status != MV_OK)
			return status;
	}

	a39x_z1_config(dev_num);

	return MV_OK;
}

u16 mv_ddr4_rtt_nom_to_odt(u16 rtt_nom)
{
	u8 odt;

	if (rtt_nom == 0)
		odt = 0xff;
	else if (rtt_nom == (1 << 8))
		odt = 60; /* 240 / 4 */
	else if (rtt_nom == (2 << 8))
		odt = 120; /* 240 / 2 */
	else if (rtt_nom == (3 << 8))
		odt = 40; /* 240 / 6 */
	else if (rtt_nom == (4 << 8))
		odt = 240; /* 240 / 1 */
	else if (rtt_nom == (5 << 8))
		odt = 48; /* 240 / 5 */
	else if (rtt_nom == (6 << 8))
		odt = 80; /* 240 / 3 */
	else if (rtt_nom == (7 << 8))
		odt = 34; /* 240 / 7 */
	else
		odt = 1;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("mv_ddr4_rtt_nom_to_odt: rtt_nom = %d, odt = %d\n", rtt_nom, odt));

	return odt;
}

u16 mv_ddr4_rtt_wr_to_odt(u16 rtt_wr)
{
	u8 odt;

	if (rtt_wr == 0)
		odt = 0xff;
	else if (rtt_wr == (1 << 9))
		odt = 120; /* 240 / 2 */
	else if (rtt_wr == (2 << 9))
		odt = 240; /* 240 / 1 */
	else
		odt = 1;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("mv_ddr4_rtt_wr_to_odt rtt_wr = %d, odt = %d\n", rtt_wr, odt));

	return odt;
}

static u32 mv_ddr4_rx_odt_get(void)
{
	u16 odt = odt_intercept[(int)g_zpodt_data / 8] - (g_zpodt_data * odt_slope[(int)g_zpodt_data / 8]) / 100;
	u16 rtt;

	if (g_odt_config & 0xf) {
		rtt = mv_ddr4_rtt_nom_to_odt(g_rtt_nom);
		odt = (odt * rtt) / (odt + rtt);
	}

	return odt;
}

static u8 mv_ddr4_vcommon_to_vref(u16 vcommon)
{
	u8 vref_tap;

	if ((vcommon > 600) && (vcommon <= 662)) {
		vref_tap = 1;
	} else if ((vcommon > 662) && (vcommon <= 679)) {
		vref_tap = 2;
	} else if ((vcommon > 679) && (vcommon <= 693)) {
		vref_tap = 3;
	} else if ((vcommon > 693) && (vcommon <= 707)) {
		vref_tap = 4;
	} else if ((vcommon > 707) && (vcommon <= 719)) {
		vref_tap = 5;
	} else if ((vcommon > 719) && (vcommon <= 725)) {
		vref_tap = 6;
	} else if ((vcommon > 725) && (vcommon <= 731)) {
		vref_tap = 7;
	} else if ((vcommon > 731) && (vcommon <= 800)) {
		vref_tap = 0;
	} else if (vcommon > 800) {
		vref_tap = 0;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_vcommon_to_vref: warning: vcommon value too high: %d\n", vcommon));
	} else if (vcommon < 600) {
		vref_tap = 1;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_vcommon_to_vref: warning: vcommon value too low: %d\n", vcommon));
	} else {
		vref_tap = 1;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_vcommon_to_vref: warning: vcommon out of range: %d\n", vcommon));
	}

	return vref_tap;
}

/* configure phy */
int mv_ddr4_phy_config(u32 dev_num)
{
	u8 cs, i, pod_val;
	u32 upper_pcal, left_pcal, upper_ncal;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	/* design GL params to be set outside */
	u32 ron = 34; /* dic - rzq / 6 or rzq / 7 */
	u32 rodt = mv_ddr4_rx_odt_get(); /* effective odt per DGL */
	u32 vcommon = (1000 * (ron + rodt / 2)) / (ron + rodt);
	u32 vref_idx;
	u8 rc_tap;
	u8 subphy_max = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	int status;

	mv_ddr4_config_phy_vref_tap = mv_ddr4_vcommon_to_vref(vcommon);

	/* change calculation for 1GHz frequency */
	if (tm->interface_params[0].memory_freq == MV_DDR_FREQ_1000)
		mv_ddr4_config_phy_vref_tap += 2;

	vref_idx = (mv_ddr4_config_phy_vref_tap < 8) ? mv_ddr4_config_phy_vref_tap : 0;
	rc_tap = (430 * (vref_val[vref_idx] - vcommon)) / 1000 + 33;
	/* 0x1 for pod mode */
	pod_val = (vref_calibration_wa == 1) ? 0x0 : 0x1;
	upper_pcal = pod_val;
	left_pcal = pod_val;
	upper_ncal = 0;

	status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				    PARAM_NOT_CARE, DDR_PHY_DATA, TEST_ADLL_PHY_REG, pod_val);
	if (status != MV_OK)
		return status;

	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, GP_RSVD0_REG,
				   (upper_pcal << 12) | (left_pcal << 6) | (upper_ncal << 5), 0x1060);
	if (status != MV_OK)
		return status;

	/*
	 * phy register 0xbf, bit 0 - configure to pod mode (0x1)
	 * phy register 0xa8, bits [6:4] - configure to clamp (0x0)
	 * subphys (broadcast) register 0xa8, bits [2:0] - configure to int ref m (0x4),
	 * TODO: need to write it to control subphys too
	 * vref tap - configure to SSTL calibration only (4)
	 * enhanced vref value - set to no clamp (0)
	 */
	for (i = 0; i < subphy_max; i++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
		ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST, 0, i, DDR_PHY_DATA, PAD_CFG_PHY_REG,
					       (0 << 4) | 4, ((0x7 << 4) | 0x7));
	}

	for (i = 0; i < 3; i++)
		ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST, 0, i, DDR_PHY_CONTROL, PAD_CFG_PHY_REG,
					       (0 << 4) | 4 , ((0x7 << 4) | 0x7));

	/* phy register 0xa4, bits [13:7] - configure to 0x7c zpri /znri */
	status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				    PARAM_NOT_CARE, DDR_PHY_DATA, PAD_ZRI_CAL_PHY_REG,
				    ((0x7f & g_zpri_data) << 7) | (0x7f & g_znri_data));
	if (status != MV_OK)
		return status;

	/*
	 * phy register 0xa6, bits [5:0] - configure to znodt (0x0)
	 * phy register 0xa6 bits [11:6] - configure to zpodt (60Ohm, 0x1d)
	 */
	status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				    PARAM_NOT_CARE, DDR_PHY_DATA, PAD_ODT_CAL_PHY_REG, g_zpodt_data << 6);
	if (status != MV_OK)
		return status;

	/* update for all active cs */
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		/*
		 * writes to present cs only
		 * phy register 0xdb, bits [5:0] - configure to rcvr cal for 50% duty cycle,
		 * broadcast to all bits cs0 (0x26)
		 */
		status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					    PARAM_NOT_CARE, DDR_PHY_DATA, VREF_BCAST_PHY_REG(cs), rc_tap);
		if (status != MV_OK)
			return status;
	}

	return MV_OK;
}

/*
 * configure sstl for manual calibration and pod for automatic one
 * assumes subphy configured to pod ealier
 */
int mv_ddr4_calibration_adjust(u32 dev_num, u8 vref_en, u8 pod_only)
{
	u8 i, if_id = 0;
	u32 read_data[MAX_INTERFACE_NUM];
	u32 ncal = 0, pcal = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int status = MV_OK;
	u8 subphy_max = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	u8  vref_tap = mv_ddr4_config_phy_vref_tap;
	u32 vref_idx = (vref_tap < 8) ? vref_tap : 0;

	if (vref_calibration_wa == 0)
		return mv_ddr4_calibration_validate(dev_num);

	if (vref_en == 1) {
		/* enhanced vref value set to no clamp (0) */
		for (i = 0; i < subphy_max; i++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
			ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST, 0, i, DDR_PHY_DATA,
						       PAD_CFG_PHY_REG, (0 << 4) | vref_idx, ((0x7 << 4) | 0x7));
		}

		for (i = 0; i < 3; i++)
			ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST, 0, i, DDR_PHY_CONTROL,
						       PAD_CFG_PHY_REG, (0 << 4) | vref_idx, ((0x7 << 4) | 0x7));
	}

	/* pad calibration control - enable */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
				   (calibration_update_control << 3) | 0x1, (0x3 << 3) | 0x1);
	if (status != MV_OK)
		return status;

	/* calibration update external */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id,
				   MAIN_PADS_CAL_MACH_CTRL_REG, 0x2 << 3, 0x3 << 3);
	if (status != MV_OK)
		return status;

	/* poll init calibration done */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x80000000, 0x80000000,
				MAIN_PADS_CAL_MACH_CTRL_REG, MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: calibration polling failed (0)\n"));

	/* poll calibration propogated to io */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3ffffff, 0x3ffffff, 0x1674,
				MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: calibration polling failed (1)\n"));

	mdelay(10); /* TODO: check it */

	/* disable dynamic */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, MAIN_PADS_CAL_MACH_CTRL_REG, 0, 0x1);
	if (status != MV_OK)
		return status;

	/* poll initial calibration done*/
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x80000000, 0x80000000,
				MAIN_PADS_CAL_MACH_CTRL_REG, MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: calibration polling failed (2)\n"));

	/* poll calibration propogated to io */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3ffffff, 0x3ffffff, 0x1674,
				MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: calibration polling failed (3)\n"));

	mdelay(10); /* TODO: check why polling insufficient */

	/* read calibration value and set it manually */
	status = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1dc8, read_data, MASK_ALL_BITS);
	if (status != MV_OK)
		return status;

	ncal = (read_data[if_id] & (0x3f << 10)) >> 10;
	pcal = (read_data[if_id] & (0x3f << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("mv_ddr4_calibration_adjust: sstl pcal = 0x%x, ncal = 0x%x\n",
			   pcal, ncal));
	if ((ncal >= 56) || (ncal <= 6) || (pcal >= 59) || (pcal <= 7)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: error: sstl pcal = 0x%x, ncal = 0x%x out of range\n",
				   pcal, ncal));
		status = MV_FAIL;
	}

	if (pod_only == 0) {
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1dc8, 0x1 << 3, 0x1 << 3);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1dc8,
					   (ncal << 22) | (pcal << 16), (0x3f << 22) | (0x3f << 16));
		if (status != MV_OK)
			return status;

		/* configure to pod mode (0x1) */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
					   GP_RSVD0_REG,
					   (0x1 << 12) | (0x1 << 6) | (0x1 << 5), 0x1060);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					    PARAM_NOT_CARE, DDR_PHY_DATA, TEST_ADLL_PHY_REG, 0x1);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					    PARAM_NOT_CARE, DDR_PHY_CONTROL, TEST_ADLL_PHY_REG, 0x1);
		if (status != MV_OK)
			return status;

		/* pad calibration control - enable */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
					   0x1, 0x1);
		if (status != MV_OK)
			return status;

		/* poll initial calibration done*/
		if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x80000000, 0x80000000,
					MAIN_PADS_CAL_MACH_CTRL_REG, MAX_POLLING_ITERATIONS) != MV_OK)
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("mv_ddr4_calibration_adjust: calibration polling failed (4)\n"));
	}

	/* calibration update internal */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, MAIN_PADS_CAL_MACH_CTRL_REG,
				   calibration_update_control << 3, 0x3 << 3);
	if (status != MV_OK)
		return status;

	/* vertical */
	status = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x14c8, read_data, MASK_ALL_BITS);
	if (status != MV_OK)
		return status;
	ncal = (read_data[if_id] & (0x3f << 10)) >> 10;
	pcal = (read_data[if_id] & (0x3f << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("mv_ddr4_calibration_adjust: pod-v pcal = 0x%x, ncal = 0x%x\n",
			   pcal, ncal));
	if ((ncal >= 56) || (ncal <= 6) || (pcal >= 59) || (pcal <= 7)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: error: pod-v pcal = 0x%x, ncal = 0x%x out of range\n",
				   pcal, ncal));
		status = MV_FAIL;
	}

	/* horizontal */
	status = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x17c8, read_data, MASK_ALL_BITS);
	if (status != MV_OK)
		return status;
	ncal = (read_data[if_id] & (0x3f << 10)) >> 10;
	pcal = (read_data[if_id] & (0x3F << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("mv_ddr4_calibration_adjust: pod-h pcal = 0x%x, ncal = 0x%x\n",
			   pcal, ncal));
	if ((ncal >= 56) || (ncal <= 6) || (pcal >= 59) || (pcal <= 7)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("mv_ddr4_calibration_adjust: error: pod-h pcal = 0x%x, ncal = 0x%x out of range\n",
				   pcal, ncal));
		status = MV_FAIL;
	}
	/* pad calibration control - disable */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
				   (calibration_update_control << 3) | 0x0, (0x3 << 3) | 0x1);
	if (status != MV_OK)
		return status;

    return status;
}

static int a39x_z1_config(u32 dev_num)
{
	u32 if_id;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int status;

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		/*
		 * xbar split bypass - dlb is off,
		 * when enabled, set to 0x1
		 */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1424, 0x0 << 3, 0x1 << 3);
		if (status != MV_OK)
			return status;

		/* auto power save option */
		status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1474, 0x0, 0xffffffff);
		if (status != MV_OK)
			return status;
	}

	return MV_OK;
}

int mv_ddr4_training_main_flow(u32 dev_num)
{
	int status = MV_OK;
	u16 pbs_tap_factor[MAX_INTERFACE_NUM][MAX_BUS_NUM][BUS_WIDTH_IN_BITS] = {0};

	if (mask_tune_func & RECEIVER_CALIBRATION_MASK_BIT) {
		training_stage = RECEIVER_CALIBRATION;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("RECEIVER_CALIBRATION_MASK_BIT #%d\n", effective_cs));
		status = mv_ddr4_receiver_calibration(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (status != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_receiver_calibrate failure\n"));
			if (debug_mode == 0)
				return status;
		}
	}

	if (mask_tune_func & WL_PHASE_CORRECTION_MASK_BIT) {
		training_stage = WL_PHASE_CORRECTION;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("WL_PHASE_CORRECTION_MASK_BIT #%d\n", effective_cs));
		status = mv_ddr4_dynamic_wl_supp(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (status != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_dynamic_wl_supp failure\n"));
			if (debug_mode == 0)
				return status;
		}
	}

	if (mask_tune_func & DQ_VREF_CALIBRATION_MASK_BIT) {
		training_stage = DQ_VREF_CALIBRATION;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("DQ_VREF_CALIBRATION_MASK_BIT #%d\n", effective_cs));
		status = mv_ddr4_dq_vref_calibration(dev_num, pbs_tap_factor);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (status != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_dq_vref_calibrate failure\n"));
			if (debug_mode == 0)
				return status;
		}
	}

	if (mask_tune_func & DM_TUNING_MASK_BIT) {
		training_stage = DM_TUNING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("DM_TUNING_MASK_BIT #%d\n", effective_cs));
		status = mv_ddr4_dm_tuning(effective_cs, pbs_tap_factor);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (status != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_dm_tuning failure\n"));
			if (debug_mode == 0)
				return status;
		}
	}

	if (mask_tune_func & DQ_MAPPING_MASK_BIT) {
		training_stage = DQ_MAPPING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("DQ_MAPPING_MASK_BIT\n"));
		status = mv_ddr4_dq_pins_mapping(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (status != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_dq_pins_mapping failure\n"));
			if (debug_mode == 0)
				return status;
		}
	}

	return status;
}
#endif /* CONFIG_DDR4 */
