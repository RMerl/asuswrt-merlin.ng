// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_regs.h"

#define VALIDATE_WIN_LENGTH(e1, e2, maxsize)		\
	(((e2) + 1 > (e1) + (u8)MIN_WINDOW_SIZE) &&	\
	 ((e2) + 1 < (e1) + (u8)maxsize))
#define IS_WINDOW_OUT_BOUNDARY(e1, e2, maxsize)			\
	(((e1) == 0 && (e2) != 0) ||				\
	 ((e1) != (maxsize - 1) && (e2) == (maxsize - 1)))
#define CENTRAL_TX		0
#define CENTRAL_RX		1
#define NUM_OF_CENTRAL_TYPES	2

u32 start_pattern = PATTERN_KILLER_DQ0, end_pattern = PATTERN_KILLER_DQ7;

u32 start_if = 0, end_if = (MAX_INTERFACE_NUM - 1);
u8 bus_end_window[NUM_OF_CENTRAL_TYPES][MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 bus_start_window[NUM_OF_CENTRAL_TYPES][MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 centralization_state[MAX_INTERFACE_NUM][MAX_BUS_NUM];
static u8 ddr3_tip_special_rx_run_once_flag;

static int ddr3_tip_centralization(u32 dev_num, u32 mode);

/*
 * Centralization RX Flow
 */
int ddr3_tip_centralization_rx(u32 dev_num)
{
	CHECK_STATUS(ddr3_tip_special_rx(dev_num));
	CHECK_STATUS(ddr3_tip_centralization(dev_num, CENTRAL_RX));

	return MV_OK;
}

/*
 * Centralization TX Flow
 */
int ddr3_tip_centralization_tx(u32 dev_num)
{
	CHECK_STATUS(ddr3_tip_centralization(dev_num, CENTRAL_TX));

	return MV_OK;
}

/*
 * Centralization Flow
 */
static int ddr3_tip_centralization(u32 dev_num, u32 mode)
{
	enum hws_training_ip_stat training_result[MAX_INTERFACE_NUM];
	u32 if_id, pattern_id, bit_id;
	u8 bus_id;
	u8 cur_start_win[BUS_WIDTH_IN_BITS];
	u8 centralization_result[MAX_INTERFACE_NUM][BUS_WIDTH_IN_BITS];
	u8 cur_end_win[BUS_WIDTH_IN_BITS];
	u8 current_window[BUS_WIDTH_IN_BITS];
	u8 opt_window, waste_window, start_window_skew, end_window_skew;
	u8 final_pup_window[MAX_INTERFACE_NUM][BUS_WIDTH_IN_BITS];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum hws_training_result result_type = RESULT_PER_BIT;
	enum hws_dir direction;
	u32 *result[HWS_SEARCH_DIR_LIMIT];
	u32 reg_phy_off, reg;
	u8 max_win_size;
	int lock_success = 1;
	u8 cur_end_win_min, cur_start_win_max;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM];
	int is_if_fail = 0;
	enum hws_result *flow_result = ddr3_tip_get_result_ptr(training_stage);
	u32 pup_win_length = 0;
	enum hws_search_dir search_dir_id;
	u8 cons_tap = (mode == CENTRAL_TX) ? (64) : (0);

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

	if (mode == CENTRAL_TX) {
		max_win_size = MAX_WINDOW_SIZE_TX;
		reg_phy_off = CTX_PHY_REG(effective_cs);
		direction = OPER_WRITE;
	} else {
		max_win_size = MAX_WINDOW_SIZE_RX;
		reg_phy_off = CRX_PHY_REG(effective_cs);
		direction = OPER_READ;
	}

	/* DB initialization */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0;
		     bus_id < octets_per_if_num; bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			centralization_state[if_id][bus_id] = 0;
			bus_end_window[mode][if_id][bus_id] =
				(max_win_size - 1) + cons_tap;
			bus_start_window[mode][if_id][bus_id] = 0;
			centralization_result[if_id][bus_id] = 0;
		}
	}

	/* start flow */
	for (pattern_id = start_pattern; pattern_id <= end_pattern;
	     pattern_id++) {
		ddr3_tip_ip_training_wrapper(dev_num, ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE,
					     ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE, result_type,
					     HWS_CONTROL_ELEMENT_ADLL,
					     PARAM_NOT_CARE, direction,
					     tm->
					     if_act_mask, 0x0,
					     max_win_size - 1,
					     max_win_size - 1,
					     pattern_id, EDGE_FPF, CS_SINGLE,
					     PARAM_NOT_CARE, training_result);

		for (if_id = start_if; if_id <= end_if; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			for (bus_id = 0;
			     bus_id <= octets_per_if_num - 1;
			     bus_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);

				for (search_dir_id = HWS_LOW2HIGH;
				     search_dir_id <= HWS_HIGH2LOW;
				     search_dir_id++) {
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  ALL_BITS_PER_PUP,
						  search_dir_id,
						  direction, result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE,
						  &result[search_dir_id],
						  1, 0, 0));
					DEBUG_CENTRALIZATION_ENGINE
						(DEBUG_LEVEL_INFO,
						 ("%s pat %d IF %d pup %d Regs: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
						  ((mode ==
						    CENTRAL_TX) ? "TX" : "RX"),
						  pattern_id, if_id, bus_id,
						  result[search_dir_id][0],
						  result[search_dir_id][1],
						  result[search_dir_id][2],
						  result[search_dir_id][3],
						  result[search_dir_id][4],
						  result[search_dir_id][5],
						  result[search_dir_id][6],
						  result[search_dir_id][7]));
				}

				for (bit_id = 0; bit_id < BUS_WIDTH_IN_BITS;
				     bit_id++) {
					/* check if this code is valid for 2 edge, probably not :( */
					cur_start_win[bit_id] =
						GET_TAP_RESULT(result
							       [HWS_LOW2HIGH]
							       [bit_id],
							       EDGE_1);
					cur_end_win[bit_id] =
						GET_TAP_RESULT(result
							       [HWS_HIGH2LOW]
							       [bit_id],
							       EDGE_1);
					/* window length */
					current_window[bit_id] =
						cur_end_win[bit_id] -
						cur_start_win[bit_id] + 1;
					DEBUG_CENTRALIZATION_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("cs %x patern %d IF %d pup %d cur_start_win %d cur_end_win %d current_window %d\n",
						  effective_cs, pattern_id,
						  if_id, bus_id,
						  cur_start_win[bit_id],
						  cur_end_win[bit_id],
						  current_window[bit_id]));
				}

				if ((ddr3_tip_is_pup_lock
				     (result[HWS_LOW2HIGH], result_type)) &&
				    (ddr3_tip_is_pup_lock
				     (result[HWS_HIGH2LOW], result_type))) {
					/* read result success */
					DEBUG_CENTRALIZATION_ENGINE
						(DEBUG_LEVEL_INFO,
						 ("Pup locked, pat %d IF %d pup %d\n",
						  pattern_id, if_id, bus_id));
				} else {
					/* read result failure */
					DEBUG_CENTRALIZATION_ENGINE
						(DEBUG_LEVEL_INFO,
						 ("fail Lock, pat %d IF %d pup %d\n",
						  pattern_id, if_id, bus_id));
					if (centralization_state[if_id][bus_id]
					    == 1) {
						/* continue with next pup */
						DEBUG_CENTRALIZATION_ENGINE
							(DEBUG_LEVEL_TRACE,
							 ("continue to next pup %d %d\n",
							  if_id, bus_id));
						continue;
					}

					for (bit_id = 0;
					     bit_id < BUS_WIDTH_IN_BITS;
					     bit_id++) {
						/*
						 * the next check is relevant
						 * only when using search
						 * machine 2 edges
						 */
						if (cur_start_win[bit_id] > 0 &&
						    cur_end_win[bit_id] == 0) {
							cur_end_win
								[bit_id] =
								max_win_size - 1;
							DEBUG_CENTRALIZATION_ENGINE
								(DEBUG_LEVEL_TRACE,
								 ("fail, IF %d pup %d bit %d fail #1\n",
								  if_id, bus_id,
								  bit_id));
							/* the next bit */
							continue;
						} else {
							centralization_state
								[if_id][bus_id] = 1;
							DEBUG_CENTRALIZATION_ENGINE
								(DEBUG_LEVEL_TRACE,
								 ("fail, IF %d pup %d bit %d fail #2\n",
								  if_id, bus_id,
								  bit_id));
						}
					}

					if (centralization_state[if_id][bus_id]
					    == 1) {
						/* going to next pup */
						continue;
					}
				}	/*bit */

				opt_window =
					ddr3_tip_get_buf_min(current_window);
				/* final pup window length */
				final_pup_window[if_id][bus_id] =
					ddr3_tip_get_buf_min(cur_end_win) -
					ddr3_tip_get_buf_max(cur_start_win) +
					1;
				waste_window =
					opt_window -
					final_pup_window[if_id][bus_id];
				start_window_skew =
					ddr3_tip_get_buf_max(cur_start_win) -
					ddr3_tip_get_buf_min(
						cur_start_win);
				end_window_skew =
					ddr3_tip_get_buf_max(
						cur_end_win) -
					ddr3_tip_get_buf_min(
						cur_end_win);
				/* min/max updated with pattern change */
				cur_end_win_min =
					ddr3_tip_get_buf_min(
						cur_end_win);
				cur_start_win_max =
					ddr3_tip_get_buf_max(
						cur_start_win);
				bus_end_window[mode][if_id][bus_id] =
					GET_MIN(bus_end_window[mode][if_id]
						[bus_id],
						cur_end_win_min);
				bus_start_window[mode][if_id][bus_id] =
					GET_MAX(bus_start_window[mode][if_id]
						[bus_id],
						cur_start_win_max);
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_INFO,
					("pat %d IF %d pup %d opt_win %d final_win %d waste_win %d st_win_skew %d end_win_skew %d cur_st_win_max %d cur_end_win_min %d bus_st_win %d bus_end_win %d\n",
					 pattern_id, if_id, bus_id, opt_window,
					 final_pup_window[if_id][bus_id],
					 waste_window, start_window_skew,
					 end_window_skew,
					 cur_start_win_max,
					 cur_end_win_min,
					 bus_start_window[mode][if_id][bus_id],
					 bus_end_window[mode][if_id][bus_id]));

				/* check if window is valid */
				if (ddr3_tip_centr_skip_min_win_check == 0) {
					if ((VALIDATE_WIN_LENGTH
					     (bus_start_window[mode][if_id]
					      [bus_id],
					      bus_end_window[mode][if_id]
					      [bus_id],
					      max_win_size) == 1) ||
					    (IS_WINDOW_OUT_BOUNDARY
					     (bus_start_window[mode][if_id]
					      [bus_id],
					      bus_end_window[mode][if_id]
					      [bus_id],
					      max_win_size) == 1)) {
						DEBUG_CENTRALIZATION_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("win valid, pat %d IF %d pup %d\n",
							  pattern_id, if_id,
							  bus_id));
						/* window is valid */
					} else {
						DEBUG_CENTRALIZATION_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("fail win, pat %d IF %d pup %d bus_st_win %d bus_end_win %d\n",
							  pattern_id, if_id, bus_id,
							  bus_start_window[mode]
							  [if_id][bus_id],
							  bus_end_window[mode]
							  [if_id][bus_id]));
						centralization_state[if_id]
							[bus_id] = 1;
						if (debug_mode == 0) {
							flow_result[if_id] = TEST_FAILED;
							return MV_FAIL;
						}
					}
				}	/* ddr3_tip_centr_skip_min_win_check */
			}	/* pup */
		}		/* interface */
	}			/* pattern */

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		is_if_fail = 0;
		flow_result[if_id] = TEST_SUCCESS;

		for (bus_id = 0;
		     bus_id <= (octets_per_if_num - 1); bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);

			/* continue only if lock */
			if (centralization_state[if_id][bus_id] != 1) {
				if (ddr3_tip_centr_skip_min_win_check == 0)	{
					if ((bus_end_window
					     [mode][if_id][bus_id] ==
					     (max_win_size - 1)) &&
					    ((bus_end_window
					      [mode][if_id][bus_id] -
					      bus_start_window[mode][if_id]
					      [bus_id]) < MIN_WINDOW_SIZE) &&
					    ((bus_end_window[mode][if_id]
					      [bus_id] - bus_start_window
					      [mode][if_id][bus_id]) > 2)) {
						/* prevent false lock */
						/* TBD change to enum */
						centralization_state
							[if_id][bus_id] = 2;
					}

					if ((bus_end_window[mode][if_id][bus_id]
					     == 0) &&
					    ((bus_end_window[mode][if_id]
					      [bus_id] -
					      bus_start_window[mode][if_id]
					      [bus_id]) < MIN_WINDOW_SIZE) &&
					    ((bus_end_window[mode][if_id]
					      [bus_id] -
					      bus_start_window[mode][if_id]
					      [bus_id]) > 2))
						/*prevent false lock */
						centralization_state[if_id]
							[bus_id] = 3;
				}

				if ((bus_end_window[mode][if_id][bus_id] >
				     (max_win_size - 1)) && direction ==
				    OPER_WRITE) {
					DEBUG_CENTRALIZATION_ENGINE
						(DEBUG_LEVEL_INFO,
						 ("Tx special pattern\n"));
					cons_tap = 64;
				}
			}

			/* check states */
			if (centralization_state[if_id][bus_id] == 3) {
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_INFO,
					("SSW - TBD IF %d pup %d\n",
					 if_id, bus_id));
				lock_success = 1;
			} else if (centralization_state[if_id][bus_id] == 2) {
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_INFO,
					("SEW - TBD IF %d pup %d\n",
					 if_id, bus_id));
				lock_success = 1;
			} else if (centralization_state[if_id][bus_id] == 0) {
				lock_success = 1;
			} else {
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_ERROR,
					("fail, IF %d pup %d\n",
					 if_id, bus_id));
				lock_success = 0;
			}

			if (lock_success == 1) {
				centralization_result[if_id][bus_id] =
					(bus_end_window[mode][if_id][bus_id] +
					 bus_start_window[mode][if_id][bus_id])
					/ 2 - cons_tap;
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_TRACE,
					(" bus_id %d Res= %d\n", bus_id,
					 centralization_result[if_id][bus_id]));
				/* copy results to registers  */
				pup_win_length =
					bus_end_window[mode][if_id][bus_id] -
					bus_start_window[mode][if_id][bus_id] +
					1;

				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  RESULT_PHY_REG +
						  effective_cs, &reg);
				reg = (reg & (~0x1f <<
					      ((mode == CENTRAL_TX) ?
					       (RESULT_PHY_TX_OFFS) :
					       (RESULT_PHY_RX_OFFS))))
					| pup_win_length <<
					((mode == CENTRAL_TX) ?
					 (RESULT_PHY_TX_OFFS) :
					 (RESULT_PHY_RX_OFFS));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_id, DDR_PHY_DATA,
					      RESULT_PHY_REG +
					      effective_cs, reg));

				/* offset per CS is calculated earlier */
				CHECK_STATUS(
					ddr3_tip_bus_write(dev_num,
							   ACCESS_TYPE_UNICAST,
							   if_id,
							   ACCESS_TYPE_UNICAST,
							   bus_id,
							   DDR_PHY_DATA,
							   reg_phy_off,
							   centralization_result
							   [if_id]
							   [bus_id]));
			} else {
				is_if_fail = 1;
			}
		}

		if (is_if_fail == 1)
			flow_result[if_id] = TEST_FAILED;
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* restore cs enable value */
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST,
					       if_id, DUAL_DUNIT_CFG_REG,
					       cs_enable_reg_val[if_id],
					       MASK_ALL_BITS));
	}

	return is_if_fail;
}

/*
 * Centralization Flow
 */
int ddr3_tip_special_rx(u32 dev_num)
{
	enum hws_training_ip_stat training_result[MAX_INTERFACE_NUM];
	u32 if_id, pup_id, pattern_id, bit_id;
	u8 cur_start_win[BUS_WIDTH_IN_BITS];
	u8 cur_end_win[BUS_WIDTH_IN_BITS];
	enum hws_training_result result_type = RESULT_PER_BIT;
	enum hws_dir direction;
	enum hws_search_dir search_dir_id;
	u32 *result[HWS_SEARCH_DIR_LIMIT];
	u32 max_win_size;
	u8 cur_end_win_min, cur_start_win_max;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM];
	u32 temp = 0;
	int pad_num = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if ((ddr3_tip_special_rx_run_once_flag & (1 << effective_cs)) == (1 << effective_cs))
		return MV_OK;

	ddr3_tip_special_rx_run_once_flag |= (1 << effective_cs);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		/* save current cs enable reg val */
		CHECK_STATUS(ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST,
					      if_id, DUAL_DUNIT_CFG_REG,
					      cs_enable_reg_val,
					      MASK_ALL_BITS));
		/* enable single cs */
		CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST,
					       if_id, DUAL_DUNIT_CFG_REG,
					       (1 << 3), (1 << 3)));
	}

	max_win_size = MAX_WINDOW_SIZE_RX;
	direction = OPER_READ;
	pattern_id = PATTERN_FULL_SSO1;

	/* start flow */
	ddr3_tip_ip_training_wrapper(dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, result_type,
				     HWS_CONTROL_ELEMENT_ADLL,
				     PARAM_NOT_CARE, direction,
				     tm->if_act_mask, 0x0,
				     max_win_size - 1, max_win_size - 1,
				     pattern_id, EDGE_FPF, CS_SINGLE,
				     PARAM_NOT_CARE, training_result);

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup_id = 0;
		     pup_id <= octets_per_if_num; pup_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup_id);

			for (search_dir_id = HWS_LOW2HIGH;
			     search_dir_id <= HWS_HIGH2LOW;
			     search_dir_id++) {
				CHECK_STATUS(ddr3_tip_read_training_result
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup_id,
					      ALL_BITS_PER_PUP, search_dir_id,
					      direction, result_type,
					      TRAINING_LOAD_OPERATION_UNLOAD,
					      CS_SINGLE, &result[search_dir_id],
					      1, 0, 0));
				DEBUG_CENTRALIZATION_ENGINE(DEBUG_LEVEL_INFO,
							    ("Special: pat %d IF %d pup %d Regs: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
							     pattern_id, if_id,
							     pup_id,
							     result
							     [search_dir_id][0],
							     result
							     [search_dir_id][1],
							     result
							     [search_dir_id][2],
							     result
							     [search_dir_id][3],
							     result
							     [search_dir_id][4],
							     result
							     [search_dir_id][5],
							     result
							     [search_dir_id][6],
							     result
							     [search_dir_id]
							     [7]));
			}

			for (bit_id = 0; bit_id < BUS_WIDTH_IN_BITS; bit_id++) {
				/*
				 * check if this code is valid for 2 edge,
				 * probably not :(
				 */
				cur_start_win[bit_id] =
					GET_TAP_RESULT(result[HWS_LOW2HIGH]
						       [bit_id], EDGE_1);
				cur_end_win[bit_id] =
					GET_TAP_RESULT(result[HWS_HIGH2LOW]
						       [bit_id], EDGE_1);
			}
			if (!((ddr3_tip_is_pup_lock
			       (result[HWS_LOW2HIGH], result_type)) &&
			      (ddr3_tip_is_pup_lock
			       (result[HWS_HIGH2LOW], result_type)))) {
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_ERROR,
					("Special: Pup lock fail, pat %d IF %d pup %d\n",
					 pattern_id, if_id, pup_id));
				return MV_FAIL;
			}

			cur_end_win_min =
				ddr3_tip_get_buf_min(cur_end_win);
			cur_start_win_max =
				ddr3_tip_get_buf_max(cur_start_win);

			if (cur_start_win_max <= 1) {	/* Align left */
				for (bit_id = 0; bit_id < BUS_WIDTH_IN_BITS;
				     bit_id++) {
					pad_num =
						dq_map_table[bit_id +
							     pup_id *
							     BUS_WIDTH_IN_BITS +
							     if_id *
							     BUS_WIDTH_IN_BITS *
							     MAX_BUS_NUM];
					CHECK_STATUS(ddr3_tip_bus_read
						     (dev_num, if_id,
						      ACCESS_TYPE_UNICAST,
						      pup_id, DDR_PHY_DATA,
						      PBS_RX_PHY_REG(effective_cs, pad_num),
						      &temp));
					temp = (temp + 0xa > 31) ?
						(31) : (temp + 0xa);
					CHECK_STATUS(ddr3_tip_bus_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      ACCESS_TYPE_UNICAST,
						      pup_id, DDR_PHY_DATA,
						      PBS_RX_PHY_REG(effective_cs, pad_num),
						      temp));
				}
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_INFO,
					("Special: PBS:: I/F# %d , Bus# %d fix align to the Left\n",
					 if_id, pup_id));
			}

			if (cur_end_win_min > 30) { /* Align right */
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup_id,
					      DDR_PHY_DATA,
					      PBS_RX_PHY_REG(effective_cs, 4),
					      &temp));
				temp += 0xa;
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      pup_id, DDR_PHY_DATA,
					      PBS_RX_PHY_REG(effective_cs, 4),
					      temp));
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup_id,
					      DDR_PHY_DATA,
					      PBS_RX_PHY_REG(effective_cs, 5),
					      &temp));
				temp += 0xa;
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      pup_id, DDR_PHY_DATA,
					      PBS_RX_PHY_REG(effective_cs, 5),
					      temp));
				DEBUG_CENTRALIZATION_ENGINE(
					DEBUG_LEVEL_INFO,
					("Special: PBS:: I/F# %d , Bus# %d fix align to the right\n",
					 if_id, pup_id));
			}

			vref_window_size[if_id][pup_id] =
				cur_end_win_min -
				cur_start_win_max + 1;
			DEBUG_CENTRALIZATION_ENGINE(
				DEBUG_LEVEL_INFO,
				("Special: Winsize I/F# %d , Bus# %d is %d\n",
				 if_id, pup_id, vref_window_size
				 [if_id][pup_id]));
		}		/* pup */
	}			/* end of interface */

	return MV_OK;
}

/*
 * Print Centralization Result
 */
int ddr3_tip_print_centralization_result(u32 dev_num)
{
	u32 if_id = 0, bus_id = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	dev_num = dev_num;

	printf("Centralization Results\n");
	printf("I/F0 Result[0 - success 1-fail 2 - state_2 3 - state_3] ...\n");
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < octets_per_if_num;
		     bus_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_id);
			printf("%d ,\n", centralization_state[if_id][bus_id]);
		}
	}

	return MV_OK;
}
