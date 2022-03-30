// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_common.h"
#include "mv_ddr_regs.h"

#define TYPICAL_PBS_VALUE	12

u32 nominal_adll[MAX_INTERFACE_NUM * MAX_BUS_NUM];
enum hws_training_ip_stat train_status[MAX_INTERFACE_NUM];
u8 result_mat[MAX_INTERFACE_NUM][MAX_BUS_NUM][BUS_WIDTH_IN_BITS];
u8 result_mat_rx_dqs[MAX_INTERFACE_NUM][MAX_BUS_NUM][MAX_CS_NUM];
/* 4-EEWA, 3-EWA, 2-SWA, 1-Fail, 0-Pass */
u8 result_all_bit[MAX_BUS_NUM * BUS_WIDTH_IN_BITS * MAX_INTERFACE_NUM];
u8 max_pbs_per_pup[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 min_pbs_per_pup[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 max_adll_per_pup[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 min_adll_per_pup[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u32 pbsdelay_per_pup[NUM_OF_PBS_MODES][MAX_INTERFACE_NUM][MAX_BUS_NUM][MAX_CS_NUM];
u8 adll_shift_lock[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 adll_shift_val[MAX_INTERFACE_NUM][MAX_BUS_NUM];
enum hws_pattern pbs_pattern = PATTERN_VREF;
static u8 pup_state[MAX_INTERFACE_NUM][MAX_BUS_NUM];

/*
 * Name:     ddr3_tip_pbs
 * Desc:     PBS
 * Args:     TBD
 * Notes:
 * Returns:  OK if success, other error code if fail.
 */
int ddr3_tip_pbs(u32 dev_num, enum pbs_dir pbs_mode)
{
	u32 res0[MAX_INTERFACE_NUM];
	int adll_tap = MEGA / mv_ddr_freq_get(medium_freq) / 64;
	int pad_num = 0;
	enum hws_search_dir search_dir =
		(pbs_mode == PBS_RX_MODE) ? HWS_HIGH2LOW : HWS_LOW2HIGH;
	enum hws_dir dir = (pbs_mode == PBS_RX_MODE) ? OPER_READ : OPER_WRITE;
	int iterations = (pbs_mode == PBS_RX_MODE) ? 31 : 63;
	u32 res_valid_mask = (pbs_mode == PBS_RX_MODE) ? 0x1f : 0x3f;
	int init_val = (search_dir == HWS_LOW2HIGH) ? 0 : iterations;
	enum hws_edge_compare search_edge = EDGE_FP;
	u32 pup = 0, bit = 0, if_id = 0, all_lock = 0, cs_num = 0;
	u32 reg_addr = 0;
	u32 validation_val = 0;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM];
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	u8 temp = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* save current cs enable reg val */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		/* save current cs enable reg val */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUAL_DUNIT_CFG_REG, cs_enable_reg_val, MASK_ALL_BITS));

		/* enable single cs */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUAL_DUNIT_CFG_REG, (1 << 3), (1 << 3)));
	}

	reg_addr = (pbs_mode == PBS_RX_MODE) ?
		CRX_PHY_REG(effective_cs) :
		CTX_PHY_REG(effective_cs);
	ddr3_tip_read_adll_value(dev_num, nominal_adll, reg_addr, MASK_ALL_BITS);

	/* stage 1 shift ADLL */
	ddr3_tip_ip_training(dev_num, ACCESS_TYPE_MULTICAST,
			     PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			     PARAM_NOT_CARE, RESULT_PER_BIT,
			     HWS_CONTROL_ELEMENT_ADLL, search_dir, dir,
			     tm->if_act_mask, init_val, iterations,
			     pbs_pattern, search_edge, CS_SINGLE, cs_num,
			     train_status);
	validation_val = (pbs_mode == PBS_RX_MODE) ? 0x1f : 0;
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			min_adll_per_pup[if_id][pup] =
				(pbs_mode == PBS_RX_MODE) ? 0x1f : 0x3f;
			pup_state[if_id][pup] = 0x3;
			adll_shift_lock[if_id][pup] = 1;
			max_adll_per_pup[if_id][pup] = 0x0;
		}
	}

	/* EBA */
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_MULTICAST,
				      PARAM_NOT_CARE,
				      mask_results_dq_reg_map[
					      bit + pup * BUS_WIDTH_IN_BITS],
				      res0, MASK_ALL_BITS));
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
						 ("FP I/F %d, bit:%d, pup:%d res0 0x%x\n",
						  if_id, bit, pup,
						  res0[if_id]));
				if (pup_state[if_id][pup] != 3)
					continue;
				/* if not EBA state than move to next pup */

				if ((res0[if_id] & 0x2000000) == 0) {
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
							 ("-- Fail Training IP\n"));
					/* training machine failed */
					pup_state[if_id][pup] = 1;
					adll_shift_lock[if_id][pup] = 0;
					continue;
				}

				else if ((res0[if_id] & res_valid_mask) ==
					 validation_val) {
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
							 ("-- FAIL EBA %d %d %d %d\n",
							  if_id, bit, pup,
							  res0[if_id]));
					pup_state[if_id][pup] = 4;
					/* this pup move to EEBA */
					adll_shift_lock[if_id][pup] = 0;
					continue;
				} else {
					/*
					 * The search ended in Pass we need
					 * Fail
					 */
					res0[if_id] =
						(pbs_mode == PBS_RX_MODE) ?
						((res0[if_id] &
						  res_valid_mask) + 1) :
						((res0[if_id] &
						  res_valid_mask) - 1);
					max_adll_per_pup[if_id][pup] =
						(max_adll_per_pup[if_id][pup] <
						 res0[if_id]) ?
						(u8)res0[if_id] :
						max_adll_per_pup[if_id][pup];
					min_adll_per_pup[if_id][pup] =
						(res0[if_id] >
						 min_adll_per_pup[if_id][pup]) ?
						min_adll_per_pup[if_id][pup] :
						(u8)
						res0[if_id];
					/*
					 * vs the Rx we are searching for the
					 * smallest value of DQ shift so all
					 * Bus would fail
					 */
					adll_shift_val[if_id][pup] =
						(pbs_mode == PBS_RX_MODE) ?
						max_adll_per_pup[if_id][pup] :
						min_adll_per_pup[if_id][pup];
				}
			}
		}
	}

	/* EEBA */
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

			if (pup_state[if_id][pup] != 4)
				continue;
			/*
			 * if pup state different from EEBA than move to
			 * next pup
			 */
			reg_addr = (pbs_mode == PBS_RX_MODE) ?
				(0x54 + effective_cs * 0x10) :
				(0x14 + effective_cs * 0x10);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, pup, DDR_PHY_DATA,
				      reg_addr, 0x1f));
			reg_addr = (pbs_mode == PBS_RX_MODE) ?
				(0x55 + effective_cs * 0x10) :
				(0x15 + effective_cs * 0x10);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, pup, DDR_PHY_DATA,
				      reg_addr, 0x1f));
			/* initialize the Edge2 Max. */
			adll_shift_val[if_id][pup] = 0;
			min_adll_per_pup[if_id][pup] =
				(pbs_mode == PBS_RX_MODE) ? 0x1f : 0x3f;
			max_adll_per_pup[if_id][pup] = 0x0;

			ddr3_tip_ip_training(dev_num, ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE,
					     ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE, RESULT_PER_BIT,
					     HWS_CONTROL_ELEMENT_ADLL,
					     search_dir, dir,
					     tm->if_act_mask, init_val,
					     iterations, pbs_pattern,
					     search_edge, CS_SINGLE, cs_num,
					     train_status);
			DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
					 ("ADLL shift results:\n"));

			for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE,
					      mask_results_dq_reg_map[
						      bit + pup *
						      BUS_WIDTH_IN_BITS],
					      res0, MASK_ALL_BITS));
				DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
						 ("FP I/F %d, bit:%d, pup:%d res0 0x%x\n",
						  if_id, bit, pup,
						  res0[if_id]));

				if ((res0[if_id] & 0x2000000) == 0) {
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
							 (" -- EEBA Fail\n"));
					bit = BUS_WIDTH_IN_BITS;
					/* exit bit loop */
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
							 ("-- EEBA Fail Training IP\n"));
					/*
					 * training machine failed but pass
					 * before in the EBA so maybe the DQS
					 * shift change env.
					 */
					pup_state[if_id][pup] = 2;
					adll_shift_lock[if_id][pup] = 0;
					reg_addr = (pbs_mode == PBS_RX_MODE) ?
						(0x54 + effective_cs * 0x10) :
						(0x14 + effective_cs * 0x10);
					CHECK_STATUS(ddr3_tip_bus_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      ACCESS_TYPE_UNICAST, pup,
						      DDR_PHY_DATA, reg_addr,
						      0x0));
					reg_addr = (pbs_mode == PBS_RX_MODE) ?
						(0x55 + effective_cs * 0x10) :
						(0x15 + effective_cs * 0x10);
					CHECK_STATUS(ddr3_tip_bus_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      ACCESS_TYPE_UNICAST, pup,
						      DDR_PHY_DATA, reg_addr,
						      0x0));
					continue;
				} else if ((res0[if_id] & res_valid_mask) ==
					   validation_val) {
					/* exit bit loop */
					bit = BUS_WIDTH_IN_BITS;
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
							 ("-- FAIL EEBA\n"));
					/* this pup move to SBA */
					pup_state[if_id][pup] = 2;
					adll_shift_lock[if_id][pup] = 0;
					reg_addr = (pbs_mode == PBS_RX_MODE) ?
						(0x54 + effective_cs * 0x10) :
						(0x14 + effective_cs * 0x10);
					CHECK_STATUS(ddr3_tip_bus_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      ACCESS_TYPE_UNICAST, pup,
						      DDR_PHY_DATA, reg_addr,
						      0x0));
					reg_addr = (pbs_mode == PBS_RX_MODE) ?
						(0x55 + effective_cs * 0x10) :
						(0x15 + effective_cs * 0x10);
					CHECK_STATUS(ddr3_tip_bus_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      ACCESS_TYPE_UNICAST, pup,
						      DDR_PHY_DATA, reg_addr,
						      0x0));
					continue;
				} else {
					adll_shift_lock[if_id][pup] = 1;
					/*
					 * The search ended in Pass we need
					 * Fail
					 */
					res0[if_id] =
						(pbs_mode == PBS_RX_MODE) ?
						((res0[if_id] &
						  res_valid_mask) + 1) :
						((res0[if_id] &
						  res_valid_mask) - 1);
					max_adll_per_pup[if_id][pup] =
						(max_adll_per_pup[if_id][pup] <
						 res0[if_id]) ?
						(u8)res0[if_id] :
						max_adll_per_pup[if_id][pup];
					min_adll_per_pup[if_id][pup] =
						(res0[if_id] >
						 min_adll_per_pup[if_id][pup]) ?
						min_adll_per_pup[if_id][pup] :
						(u8)res0[if_id];
					/*
					 * vs the Rx we are searching for the
					 * smallest value of DQ shift so all Bus
					 * would fail
					 */
					adll_shift_val[if_id][pup] =
						(pbs_mode == PBS_RX_MODE) ?
						max_adll_per_pup[if_id][pup] :
						min_adll_per_pup[if_id][pup];
				}
			}
		}
	}

	/* Print Stage result */
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
					 ("FP I/F %d, ADLL Shift for EBA: pup[%d] Lock status = %d Lock Val = %d,%d\n",
					  if_id, pup,
					  adll_shift_lock[if_id][pup],
					  max_adll_per_pup[if_id][pup],
					  min_adll_per_pup[if_id][pup]));
		}
	}
	DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
			 ("Update ADLL Shift of all pups:\n"));

	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			if (adll_shift_lock[if_id][pup] != 1)
				continue;
			/* if pup not locked continue to next pup */

			reg_addr = (pbs_mode == PBS_RX_MODE) ?
				(0x3 + effective_cs * 4) :
				(0x1 + effective_cs * 4);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, pup, DDR_PHY_DATA,
				      reg_addr, adll_shift_val[if_id][pup]));
			DEBUG_PBS_ENGINE(DEBUG_LEVEL_TRACE,
					 ("FP I/F %d, Pup[%d] = %d\n", if_id,
					  pup, adll_shift_val[if_id][pup]));
		}
	}

	/* PBS EEBA&EBA */
	/* Start the Per Bit Skew search */
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			max_pbs_per_pup[if_id][pup] = 0x0;
			min_pbs_per_pup[if_id][pup] = 0x1f;
			for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
				/* reset result for PBS */
				result_all_bit[bit + pup * BUS_WIDTH_IN_BITS +
					       if_id * MAX_BUS_NUM *
					       BUS_WIDTH_IN_BITS] = 0;
			}
		}
	}

	iterations = 31;
	search_dir = HWS_LOW2HIGH;
	/* !!!!! ran sh (search_dir == HWS_LOW2HIGH)?0:iterations; */
	init_val = 0;

	ddr3_tip_ip_training(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			     ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			     RESULT_PER_BIT, HWS_CONTROL_ELEMENT_DQ_SKEW,
			     search_dir, dir, tm->if_act_mask, init_val,
			     iterations, pbs_pattern, search_edge,
			     CS_SINGLE, cs_num, train_status);

	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			if (adll_shift_lock[if_id][pup] != 1) {
				/* if pup not lock continue to next pup */
				continue;
			}

			for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE,
					      mask_results_dq_reg_map[
						      bit +
						      pup * BUS_WIDTH_IN_BITS],
					      res0, MASK_ALL_BITS));
				DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
						 ("Per Bit Skew search, FP I/F %d, bit:%d, pup:%d res0 0x%x\n",
						  if_id, bit, pup,
						  res0[if_id]));
				if ((res0[if_id] & 0x2000000) == 0) {
					DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
							 ("--EBA PBS Fail - Training IP machine\n"));
					/* exit the bit loop */
					bit = BUS_WIDTH_IN_BITS;
					/*
					 * ADLL is no long in lock need new
					 * search
					 */
					adll_shift_lock[if_id][pup] = 0;
					/* Move to SBA */
					pup_state[if_id][pup] = 2;
					max_pbs_per_pup[if_id][pup] = 0x0;
					min_pbs_per_pup[if_id][pup] = 0x1f;
					continue;
				} else {
					temp = (u8)(res0[if_id] &
						    res_valid_mask);
					max_pbs_per_pup[if_id][pup] =
						(temp >
						 max_pbs_per_pup[if_id][pup]) ?
						temp :
						max_pbs_per_pup[if_id][pup];
					min_pbs_per_pup[if_id][pup] =
						(temp <
						 min_pbs_per_pup[if_id][pup]) ?
						temp :
						min_pbs_per_pup[if_id][pup];
					result_all_bit[bit +
						       pup * BUS_WIDTH_IN_BITS +
						       if_id * MAX_BUS_NUM *
						       BUS_WIDTH_IN_BITS] =
						temp;
				}
			}
		}
	}

	/* Check all Pup lock */
	all_lock = 1;
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			all_lock = all_lock * adll_shift_lock[if_id][pup];
		}
	}

	/* Only if not all Pups Lock */
	if (all_lock == 0) {
		DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
				 ("##########ADLL shift for SBA###########\n"));

		/* ADLL shift for SBA */
		search_dir = (pbs_mode == PBS_RX_MODE) ? HWS_LOW2HIGH :
			HWS_HIGH2LOW;
		init_val = (search_dir == HWS_LOW2HIGH) ? 0 : iterations;
		for (pup = 0; pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				if (adll_shift_lock[if_id][pup] == 1) {
					/*if pup lock continue to next pup */
					continue;
				}
				/*init the var altogth init before */
				adll_shift_lock[if_id][pup] = 0;
				reg_addr = (pbs_mode == PBS_RX_MODE) ?
					(0x54 + effective_cs * 0x10) :
					(0x14 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr, 0));
				reg_addr = (pbs_mode == PBS_RX_MODE) ?
					(0x55 + effective_cs * 0x10) :
					(0x15 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr, 0));
				reg_addr = (pbs_mode == PBS_RX_MODE) ?
					(0x5f + effective_cs * 0x10) :
					(0x1f + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr, 0));
				/* initilaze the Edge2 Max. */
				adll_shift_val[if_id][pup] = 0;
				min_adll_per_pup[if_id][pup] = 0x1f;
				max_adll_per_pup[if_id][pup] = 0x0;

				ddr3_tip_ip_training(dev_num,
						     ACCESS_TYPE_MULTICAST,
						     PARAM_NOT_CARE,
						     ACCESS_TYPE_MULTICAST,
						     PARAM_NOT_CARE,
						     RESULT_PER_BIT,
						     HWS_CONTROL_ELEMENT_ADLL,
						     search_dir, dir,
						     tm->if_act_mask,
						     init_val, iterations,
						     pbs_pattern,
						     search_edge, CS_SINGLE,
						     cs_num, train_status);

				for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_MULTICAST,
						      PARAM_NOT_CARE,
						      mask_results_dq_reg_map
						      [bit +
						       pup *
						       BUS_WIDTH_IN_BITS],
						      res0, MASK_ALL_BITS));
					DEBUG_PBS_ENGINE(
						DEBUG_LEVEL_INFO,
						("FP I/F %d, bit:%d, pup:%d res0 0x%x\n",
						 if_id, bit, pup, res0[if_id]));
					if ((res0[if_id] & 0x2000000) == 0) {
						/* exit the bit loop */
						bit = BUS_WIDTH_IN_BITS;
						/* Fail SBA --> Fail PBS */
						pup_state[if_id][pup] = 1;
						DEBUG_PBS_ENGINE
							(DEBUG_LEVEL_INFO,
							 (" SBA Fail\n"));
						continue;
					} else {
						/*
						 * - increment to get all
						 * 8 bit lock.
						 */
						adll_shift_lock[if_id][pup]++;
						/*
						 * The search ended in Pass
						 * we need Fail
						 */
						res0[if_id] =
							(pbs_mode == PBS_RX_MODE) ?
							((res0[if_id] & res_valid_mask) + 1) :
							((res0[if_id] & res_valid_mask) - 1);
						max_adll_per_pup[if_id][pup] =
							(max_adll_per_pup[if_id]
							 [pup] < res0[if_id]) ?
							(u8)res0[if_id] :
							max_adll_per_pup[if_id][pup];
						min_adll_per_pup[if_id][pup] =
							(res0[if_id] >
							 min_adll_per_pup[if_id]
							 [pup]) ?
							min_adll_per_pup[if_id][pup] :
							(u8)res0[if_id];
						/*
						 * vs the Rx we are searching for
						 * the smallest value of DQ shift
						 * so all Bus would fail
						 */
						adll_shift_val[if_id][pup] =
							(pbs_mode == PBS_RX_MODE) ?
							max_adll_per_pup[if_id][pup] :
							min_adll_per_pup[if_id][pup];
					}
				}
				/* 1 is lock */
				adll_shift_lock[if_id][pup] =
					(adll_shift_lock[if_id][pup] == 8) ?
					1 : 0;
				reg_addr = (pbs_mode == PBS_RX_MODE) ?
					(0x3 + effective_cs * 4) :
					(0x1 + effective_cs * 4);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr,
					      adll_shift_val[if_id][pup]));
				DEBUG_PBS_ENGINE(
					DEBUG_LEVEL_INFO,
					("adll_shift_lock[%x][%x] = %x\n",
					 if_id, pup,
					 adll_shift_lock[if_id][pup]));
			}
		}

		/* End ADLL Shift for SBA */
		/* Start the Per Bit Skew search */
		/* The ADLL shift finished with a Pass */
		search_edge = (pbs_mode == PBS_RX_MODE) ? EDGE_PF : EDGE_FP;
		search_dir = (pbs_mode == PBS_RX_MODE) ?
			HWS_LOW2HIGH : HWS_HIGH2LOW;
		iterations = 0x1f;
		/* - The initial value is different in Rx and Tx mode */
		init_val = (pbs_mode == PBS_RX_MODE) ? 0 : iterations;

		ddr3_tip_ip_training(dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, RESULT_PER_BIT,
				     HWS_CONTROL_ELEMENT_DQ_SKEW,
				     search_dir, dir, tm->if_act_mask,
				     init_val, iterations, pbs_pattern,
				     search_edge, CS_SINGLE, cs_num,
				     train_status);

		for (pup = 0; pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_MULTICAST,
						      PARAM_NOT_CARE,
						      mask_results_dq_reg_map
						      [bit +
						       pup *
						       BUS_WIDTH_IN_BITS],
						      res0, MASK_ALL_BITS));
					if (pup_state[if_id][pup] != 2) {
						/*
						 * if pup is not SBA continue
						 * to next pup
						 */
						bit = BUS_WIDTH_IN_BITS;
						continue;
					}
					DEBUG_PBS_ENGINE(
						DEBUG_LEVEL_INFO,
						("Per Bit Skew search, PF I/F %d, bit:%d, pup:%d res0 0x%x\n",
						 if_id, bit, pup, res0[if_id]));
					if ((res0[if_id] & 0x2000000) == 0) {
						DEBUG_PBS_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("SBA Fail\n"));

						max_pbs_per_pup[if_id][pup] =
							0x1f;
						result_all_bit[
							bit + pup *
							BUS_WIDTH_IN_BITS +
							if_id * MAX_BUS_NUM *
							BUS_WIDTH_IN_BITS] =
							0x1f;
					} else {
						temp = (u8)(res0[if_id] &
							    res_valid_mask);
						max_pbs_per_pup[if_id][pup] =
							(temp >
							 max_pbs_per_pup[if_id]
							 [pup]) ? temp :
							max_pbs_per_pup
							[if_id][pup];
						min_pbs_per_pup[if_id][pup] =
							(temp <
							 min_pbs_per_pup[if_id]
							 [pup]) ? temp :
							min_pbs_per_pup
							[if_id][pup];
						result_all_bit[
							bit + pup *
							BUS_WIDTH_IN_BITS +
							if_id * MAX_BUS_NUM *
							BUS_WIDTH_IN_BITS] =
							temp;
						adll_shift_lock[if_id][pup] = 1;
					}
				}
			}
		}

		/* Check all Pup state */
		all_lock = 1;
		for (pup = 0; pup < octets_per_if_num; pup++) {
			/*
			 * DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
			 * ("pup_state[%d][%d] = %d\n",if_id,pup,pup_state
			 * [if_id][pup]));
			*/
		}
	}

	/* END OF SBA */
	/* Norm */
	for (pup = 0; pup < octets_per_if_num; pup++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
		for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				/* if pup not lock continue to next pup */
				if (adll_shift_lock[if_id][pup] != 1) {
					DEBUG_PBS_ENGINE(
						DEBUG_LEVEL_ERROR,
						("PBS failed for IF #%d\n",
						 if_id));
					training_result[training_stage][if_id]
						= TEST_FAILED;

					result_mat[if_id][pup][bit] = 0;
					max_pbs_per_pup[if_id][pup] = 0;
					min_pbs_per_pup[if_id][pup] = 0;
				} else {
					training_result[
						training_stage][if_id] =
						(training_result[training_stage]
						 [if_id] == TEST_FAILED) ?
						TEST_FAILED : TEST_SUCCESS;
					result_mat[if_id][pup][bit] =
						result_all_bit[
							bit + pup *
							BUS_WIDTH_IN_BITS +
							if_id * MAX_BUS_NUM *
							BUS_WIDTH_IN_BITS] -
						min_pbs_per_pup[if_id][pup];
				}
				DEBUG_PBS_ENGINE(
					DEBUG_LEVEL_INFO,
					("The abs min_pbs[%d][%d] = %d\n",
					 if_id, pup,
					 min_pbs_per_pup[if_id][pup]));
			}
		}
	}

	/* Clean all results */
	ddr3_tip_clean_pbs_result(dev_num, pbs_mode);

	/* DQ PBS register update with the final result */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0; pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);

			DEBUG_PBS_ENGINE(
				DEBUG_LEVEL_INFO,
				("Final Results: if_id %d, pup %d, Pup State: %d\n",
				 if_id, pup, pup_state[if_id][pup]));
			for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
				if (dq_map_table == NULL) {
					DEBUG_PBS_ENGINE(
						DEBUG_LEVEL_ERROR,
						("dq_map_table not initialized\n"));
					return MV_FAIL;
				}
				pad_num = dq_map_table[
					bit + pup * BUS_WIDTH_IN_BITS +
					if_id * BUS_WIDTH_IN_BITS *
					MAX_BUS_NUM];
				DEBUG_PBS_ENGINE(DEBUG_LEVEL_INFO,
						 ("result_mat: %d ",
						  result_mat[if_id][pup]
						  [bit]));
				reg_addr = (pbs_mode == PBS_RX_MODE) ?
					PBS_RX_PHY_REG(effective_cs, 0) :
					PBS_TX_PHY_REG(effective_cs, 0);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr + pad_num,
					      result_mat[if_id][pup][bit]));
			}

			if (max_pbs_per_pup[if_id][pup] == min_pbs_per_pup[if_id][pup]) {
				temp = TYPICAL_PBS_VALUE;
			} else {
				temp = ((max_adll_per_pup[if_id][pup] -
					 min_adll_per_pup[if_id][pup]) *
					adll_tap /
					(max_pbs_per_pup[if_id][pup] -
					 min_pbs_per_pup[if_id][pup]));
			}
			pbsdelay_per_pup[pbs_mode]
			[if_id][pup][effective_cs] = temp;

			/* RX results ready, write RX also */
			if (pbs_mode == PBS_TX_MODE) {
				/* Write TX results */
				reg_addr = (0x14 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr,
					      (max_pbs_per_pup[if_id][pup] -
					       min_pbs_per_pup[if_id][pup]) /
					      2));
				reg_addr = (0x15 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr,
					      (max_pbs_per_pup[if_id][pup] -
					       min_pbs_per_pup[if_id][pup]) /
					      2));

				/* Write previously stored RX results */
				reg_addr = (0x54 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr,
					      result_mat_rx_dqs[if_id][pup]
					      [effective_cs]));
				reg_addr = (0x55 + effective_cs * 0x10);
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr,
					      result_mat_rx_dqs[if_id][pup]
					      [effective_cs]));
			} else {
				/*
				 * RX results may affect RL results correctess,
				 * so just store the results that will written
				 * in TX stage
				 */
				result_mat_rx_dqs[if_id][pup][effective_cs] =
					(max_pbs_per_pup[if_id][pup] -
					 min_pbs_per_pup[if_id][pup]) / 2;
			}
			DEBUG_PBS_ENGINE(
				DEBUG_LEVEL_INFO,
				(", PBS tap=%d [psec] ==> skew observed = %d\n",
				 temp,
				 ((max_pbs_per_pup[if_id][pup] -
				   min_pbs_per_pup[if_id][pup]) *
				 temp)));
		}
	}

	/* Write back to the phy the default values */
	reg_addr = (pbs_mode == PBS_RX_MODE) ?
		CRX_PHY_REG(effective_cs) :
		CTX_PHY_REG(effective_cs);
	ddr3_tip_write_adll_value(dev_num, nominal_adll, reg_addr);

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		reg_addr = (pbs_mode == PBS_RX_MODE) ?
			(0x5a + effective_cs * 0x10) :
			(0x1a + effective_cs * 0x10);
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ACCESS_TYPE_UNICAST, pup, DDR_PHY_DATA, reg_addr,
			      0));

		/* restore cs enable value */
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUAL_DUNIT_CFG_REG, cs_enable_reg_val[if_id],
			      MASK_ALL_BITS));
	}

	/* exit test mode */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_WR_RD_MODE_ENA_REG, 0xffff, MASK_ALL_BITS));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0; pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
			/*
			 * no valid window found
			 * (no lock at EBA ADLL shift at EBS)
			 */
			if (pup_state[if_id][pup] == 1)
				return MV_FAIL;
		}
	}

	return MV_OK;
}

/*
 * Name:     ddr3_tip_pbs_rx.
 * Desc:     PBS TX
 * Args:     TBD
 * Notes:
 * Returns:  OK if success, other error code if fail.
 */
int ddr3_tip_pbs_rx(u32 uidev_num)
{
	return ddr3_tip_pbs(uidev_num, PBS_RX_MODE);
}

/*
 * Name:     ddr3_tip_pbs_tx.
 * Desc:     PBS TX
 * Args:     TBD
 * Notes:
 * Returns:  OK if success, other error code if fail.
 */
int ddr3_tip_pbs_tx(u32 uidev_num)
{
	return ddr3_tip_pbs(uidev_num, PBS_TX_MODE);
}

#ifdef DDR_VIEWER_TOOL
/*
 * Print PBS Result
 */
int ddr3_tip_print_all_pbs_result(u32 dev_num)
{
	u32 curr_cs;
	unsigned int max_cs = mv_ddr_cs_num_get();

	for (curr_cs = 0; curr_cs < max_cs; curr_cs++) {
		ddr3_tip_print_pbs_result(dev_num, curr_cs, PBS_RX_MODE);
		ddr3_tip_print_pbs_result(dev_num, curr_cs, PBS_TX_MODE);
	}

	return MV_OK;
}

/*
 * Print PBS Result
 */
int ddr3_tip_print_pbs_result(u32 dev_num, u32 cs_num, enum pbs_dir pbs_mode)
{
	u32 data_value = 0, bit = 0, if_id = 0, pup = 0;
	u32 reg_addr = (pbs_mode == PBS_RX_MODE) ?
		PBS_RX_PHY_REG(cs_num, 0) :
		PBS_TX_PHY_REG(cs_num , 0);
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	printf("%s,CS%d,PBS,ADLLRATIO,,,",
	       (pbs_mode == PBS_RX_MODE) ? "Rx" : "Tx", cs_num);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0; pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
			printf("%d,",
			       pbsdelay_per_pup[pbs_mode][if_id][pup][cs_num]);
		}
	}
	printf("CS%d, %s ,PBS\n", cs_num,
	       (pbs_mode == PBS_RX_MODE) ? "Rx" : "Tx");

	for (bit = 0; bit < BUS_WIDTH_IN_BITS; bit++) {
		printf("%s, DQ", (pbs_mode == PBS_RX_MODE) ? "Rx" : "Tx");
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			printf("%d ,PBS,,, ", bit);
			for (pup = 0; pup <= octets_per_if_num;
			     pup++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr + bit,
					      &data_value));
				printf("%d , ", data_value);
			}
		}
		printf("\n");
	}
	printf("\n");

	return MV_OK;
}
#endif /* DDR_VIEWER_TOOL */

/*
 * Fixup PBS Result
 */
int ddr3_tip_clean_pbs_result(u32 dev_num, enum pbs_dir pbs_mode)
{
	u32 if_id, pup, bit;
	u32 reg_addr = (pbs_mode == PBS_RX_MODE) ?
		PBS_RX_PHY_REG(effective_cs, 0) :
		PBS_TX_PHY_REG(effective_cs, 0);
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0; pup <= octets_per_if_num; pup++) {
			for (bit = 0; bit <= BUS_WIDTH_IN_BITS + 3; bit++) {
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr + bit, 0));
			}
		}
	}

	return MV_OK;
}
