// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_regs.h"

#define VREF_INITIAL_STEP		3
#define VREF_SECOND_STEP		1
#define VREF_MAX_INDEX			7
#define MAX_VALUE			(1024 - 1)
#define MIN_VALUE			(-MAX_VALUE)
#define GET_RD_SAMPLE_DELAY(data, cs)	((data >> rd_sample_mask[cs]) & 0x1f)

u32 ca_delay;
int ddr3_tip_centr_skip_min_win_check = 0;
u8 current_vref[MAX_BUS_NUM][MAX_INTERFACE_NUM];
u8 last_vref[MAX_BUS_NUM][MAX_INTERFACE_NUM];
u16 current_valid_window[MAX_BUS_NUM][MAX_INTERFACE_NUM];
u16 last_valid_window[MAX_BUS_NUM][MAX_INTERFACE_NUM];
u8 lim_vref[MAX_BUS_NUM][MAX_INTERFACE_NUM];
u8 interface_state[MAX_INTERFACE_NUM];
u8 vref_window_size[MAX_INTERFACE_NUM][MAX_BUS_NUM];
u8 vref_window_size_th = 12;

static u8 pup_st[MAX_BUS_NUM][MAX_INTERFACE_NUM];

static u32 rd_sample_mask[] = {
	0,
	8,
	16,
	24
};

#define	VREF_STEP_1		0
#define	VREF_STEP_2		1
#define	VREF_CONVERGE		2

/*
 * ODT additional timing
 */
int ddr3_tip_write_additional_odt_setting(u32 dev_num, u32 if_id)
{
	u32 cs_num = 0, max_read_sample = 0, min_read_sample = 0x1f;
	u32 data_read[MAX_INTERFACE_NUM] = { 0 };
	u32 read_sample[MAX_CS_NUM];
	u32 val;
	u32 pup_index;
	int max_phase = MIN_VALUE, current_phase;
	enum hws_access_type access_type = ACCESS_TYPE_UNICAST;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	unsigned int max_cs = mv_ddr_cs_num_get();

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DUNIT_ODT_CTRL_REG,
				       0 << 8, 0x3 << 8));
	CHECK_STATUS(ddr3_tip_if_read(dev_num, access_type, if_id,
				      RD_DATA_SMPL_DLYS_REG,
				      data_read, MASK_ALL_BITS));
	val = data_read[if_id];

	for (cs_num = 0; cs_num < max_cs; cs_num++) {
		read_sample[cs_num] = GET_RD_SAMPLE_DELAY(val, cs_num);

		/* find maximum of read_samples */
		if (read_sample[cs_num] >= max_read_sample) {
			if (read_sample[cs_num] == max_read_sample)
				max_phase = MIN_VALUE;
			else
				max_read_sample = read_sample[cs_num];

			for (pup_index = 0;
			     pup_index < octets_per_if_num;
			     pup_index++) {
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup_index,
					      DDR_PHY_DATA,
					      RL_PHY_REG(cs_num),
					      &val));

				current_phase = ((int)val & 0xe0) >> 6;
				if (current_phase >= max_phase)
					max_phase = current_phase;
			}
		}

		/* find minimum */
		if (read_sample[cs_num] < min_read_sample)
			min_read_sample = read_sample[cs_num];
	}

	min_read_sample = min_read_sample + 2;
	max_read_sample = max_read_sample + 7 + (max_phase + 1) / 2 + 1;
	if (min_read_sample >= 0xf)
		min_read_sample = 0xf;
	if (max_read_sample >= 0x1f)
		max_read_sample = 0x1f;

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DDR_ODT_TIMING_LOW_REG,
				       ((min_read_sample - 1) << 12),
				       0xf << 12));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DDR_ODT_TIMING_LOW_REG,
				       (max_read_sample << 16),
				       0x1f << 16));

	return MV_OK;
}

int get_valid_win_rx(u32 dev_num, u32 if_id, u8 res[4])
{
	u32 reg_pup = RESULT_PHY_REG;
	u32 reg_data;
	u32 cs_num;
	int i;

	cs_num = 0;

	/* TBD */
	reg_pup += cs_num;

	for (i = 0; i < 4; i++) {
		CHECK_STATUS(ddr3_tip_bus_read(dev_num, if_id,
					       ACCESS_TYPE_UNICAST, i,
					       DDR_PHY_DATA, reg_pup,
					       &reg_data));
		res[i] = (reg_data >> RESULT_PHY_RX_OFFS) & 0x1f;
	}

	return 0;
}

/*
 * This algorithm deals with the vertical optimum from Voltage point of view
 * of the sample signal.
 * Voltage sample point can improve the Eye / window size of the bit and the
 * pup.
 * The problem is that it is tune for all DQ the same so there isn't any
 * PBS like code.
 * It is more like centralization.
 * But because we don't have The training SM support we do it a bit more
 * smart search to save time.
 */
int ddr3_tip_vref(u32 dev_num)
{
	/*
	 * The Vref register have non linear order. Need to check what will be
	 * in future projects.
	 */
	u32 vref_map[8] = {
		1, 2, 3, 4, 5, 6, 7, 0
	};
	/* State and parameter definitions */
	u32 initial_step = VREF_INITIAL_STEP;
	/* need to be assign with minus ????? */
	u32 second_step = VREF_SECOND_STEP;
	u32 algo_run_flag = 0, currrent_vref = 0;
	u32 while_count = 0;
	u32 pup = 0, if_id = 0, num_pup = 0, rep = 0;
	u32 val = 0;
	u32 reg_addr = 0xa8;
	u32 copy_start_pattern, copy_end_pattern;
	enum hws_result *flow_result = ddr3_tip_get_result_ptr(training_stage);
	u8 res[4];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	CHECK_STATUS(ddr3_tip_special_rx(dev_num));

	/* save start/end pattern */
	copy_start_pattern = start_pattern;
	copy_end_pattern = end_pattern;

	/* set vref as centralization pattern */
	start_pattern = PATTERN_VREF;
	end_pattern = PATTERN_VREF;

	/* init params */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0;
		     pup < octets_per_if_num; pup++) {
			current_vref[pup][if_id] = 0;
			last_vref[pup][if_id] = 0;
			lim_vref[pup][if_id] = 0;
			current_valid_window[pup][if_id] = 0;
			last_valid_window[pup][if_id] = 0;
			if (vref_window_size[if_id][pup] >
			    vref_window_size_th) {
				pup_st[pup][if_id] = VREF_CONVERGE;
				DEBUG_TRAINING_HW_ALG(
					DEBUG_LEVEL_INFO,
					("VREF config, IF[ %d ]pup[ %d ] - Vref tune not requered (%d)\n",
					 if_id, pup, __LINE__));
			} else {
				pup_st[pup][if_id] = VREF_STEP_1;
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, pup,
					      DDR_PHY_DATA, reg_addr, &val));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      pup, DDR_PHY_DATA, reg_addr,
					      (val & (~0xf)) | vref_map[0]));
				DEBUG_TRAINING_HW_ALG(
					DEBUG_LEVEL_INFO,
					("VREF config, IF[ %d ]pup[ %d ] - Vref = %X (%d)\n",
					 if_id, pup,
					 (val & (~0xf)) | vref_map[0],
					 __LINE__));
			}
		}
		interface_state[if_id] = 0;
	}

	/* TODO: Set number of active interfaces */
	num_pup = octets_per_if_num * MAX_INTERFACE_NUM;

	while ((algo_run_flag <= num_pup) & (while_count < 10)) {
		while_count++;
		for (rep = 1; rep < 4; rep++) {
			ddr3_tip_centr_skip_min_win_check = 1;
			ddr3_tip_centralization_rx(dev_num);
			ddr3_tip_centr_skip_min_win_check = 0;

			/* Read Valid window results only for non converge pups */
			for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				if (interface_state[if_id] != 4) {
					get_valid_win_rx(dev_num, if_id, res);
					for (pup = 0;
					     pup < octets_per_if_num;
					     pup++) {
						VALIDATE_BUS_ACTIVE
							(tm->bus_act_mask, pup);
						if (pup_st[pup]
						    [if_id] ==
						    VREF_CONVERGE)
							continue;

						current_valid_window[pup]
							[if_id] =
							(current_valid_window[pup]
							 [if_id] * (rep - 1) +
							 1000 * res[pup]) / rep;
					}
				}
			}
		}

		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			DEBUG_TRAINING_HW_ALG(
				DEBUG_LEVEL_TRACE,
				("current_valid_window: IF[ %d ] - ", if_id));

			for (pup = 0;
			     pup < octets_per_if_num; pup++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
				DEBUG_TRAINING_HW_ALG(DEBUG_LEVEL_TRACE,
						      ("%d ",
						       current_valid_window
						       [pup][if_id]));
			}
			DEBUG_TRAINING_HW_ALG(DEBUG_LEVEL_TRACE, ("\n"));
		}

		/* Compare results and respond as function of state */
		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
			for (pup = 0;
			     pup < octets_per_if_num; pup++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
				DEBUG_TRAINING_HW_ALG(DEBUG_LEVEL_TRACE,
						      ("I/F[ %d ], pup[ %d ] STATE #%d (%d)\n",
						       if_id, pup,
						       pup_st[pup]
						       [if_id], __LINE__));

				if (pup_st[pup][if_id] == VREF_CONVERGE)
					continue;

				DEBUG_TRAINING_HW_ALG(DEBUG_LEVEL_TRACE,
						      ("I/F[ %d ], pup[ %d ] CHECK progress - Current %d Last %d, limit VREF %d (%d)\n",
						       if_id, pup,
						       current_valid_window[pup]
						       [if_id],
						       last_valid_window[pup]
						       [if_id], lim_vref[pup]
						       [if_id], __LINE__));

				/*
				 * The -1 is for solution resolution +/- 1 tap
				 * of ADLL
				 */
				if (current_valid_window[pup][if_id] + 200 >=
				    (last_valid_window[pup][if_id])) {
					if (pup_st[pup][if_id] == VREF_STEP_1) {
						/*
						 * We stay in the same state and
						 * step just update the window
						 * size (take the max) and Vref
						 */
						if (current_vref[pup]
						    [if_id] == VREF_MAX_INDEX) {
							/*
							 * If we step to the end
							 * and didn't converge
							 * to some particular
							 * better Vref value
							 * define the pup as
							 * converge and step
							 * back to nominal
							 * Vref.
							 */
							pup_st[pup]
								[if_id] =
								VREF_CONVERGE;
							algo_run_flag++;
							interface_state
								[if_id]++;
							DEBUG_TRAINING_HW_ALG
								(DEBUG_LEVEL_TRACE,
								 ("I/F[ %d ], pup[ %d ] VREF_CONVERGE - Vref = %X (%d)\n",
								  if_id, pup,
								  current_vref[pup]
								  [if_id],
								  __LINE__));
						} else {
							/* continue to update the Vref index */
							current_vref[pup]
								[if_id] =
								((current_vref[pup]
								  [if_id] +
								  initial_step) >
								 VREF_MAX_INDEX) ?
								VREF_MAX_INDEX
								: (current_vref[pup]
								   [if_id] +
								   initial_step);
							if (current_vref[pup]
							    [if_id] ==
							    VREF_MAX_INDEX) {
								pup_st[pup]
									[if_id]
									=
									VREF_STEP_2;
							}
							lim_vref[pup]
								[if_id] =
								last_vref[pup]
								[if_id] =
								current_vref[pup]
								[if_id];
						}

						last_valid_window[pup]
							[if_id] =
							GET_MAX(current_valid_window
								[pup][if_id],
								last_valid_window
								[pup]
								[if_id]);

						/* update the Vref for next stage */
						currrent_vref =
							current_vref[pup]
							[if_id];
						CHECK_STATUS
							(ddr3_tip_bus_read
							 (dev_num, if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  &val));
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  (val & (~0xf)) |
							  vref_map[currrent_vref]));
						DEBUG_TRAINING_HW_ALG
							(DEBUG_LEVEL_TRACE,
							 ("VREF config, IF[ %d ]pup[ %d ] - Vref = %X (%d)\n",
							  if_id, pup,
							  (val & (~0xf)) |
							  vref_map[currrent_vref],
							  __LINE__));
					} else if (pup_st[pup][if_id]
						   == VREF_STEP_2) {
						/*
						 * We keep on search back with
						 * the same step size.
						 */
						last_valid_window[pup]
							[if_id] =
							GET_MAX(current_valid_window
								[pup][if_id],
								last_valid_window
								[pup]
								[if_id]);
						last_vref[pup][if_id] =
							current_vref[pup]
							[if_id];

						/* we finish all search space */
						if ((current_vref[pup]
						     [if_id] - second_step) == lim_vref[pup][if_id]) {
							/*
							 * If we step to the end
							 * and didn't converge
							 * to some particular
							 * better Vref value
							 * define the pup as
							 * converge and step
							 * back to nominal
							 * Vref.
							 */
							pup_st[pup]
								[if_id] =
								VREF_CONVERGE;
							algo_run_flag++;

							interface_state
								[if_id]++;

							current_vref[pup]
								[if_id] =
								(current_vref[pup]
								 [if_id] -
								 second_step);

							DEBUG_TRAINING_HW_ALG
								(DEBUG_LEVEL_TRACE,
								 ("I/F[ %d ], pup[ %d ] VREF_CONVERGE - Vref = %X (%d)\n",
								  if_id, pup,
								  current_vref[pup]
								  [if_id],
								  __LINE__));
						} else
							/* we finish all search space */
							if (current_vref[pup]
							    [if_id] ==
							    lim_vref[pup]
							    [if_id]) {
								/*
								 * If we step to the end
								 * and didn't converge
								 * to some particular
								 * better Vref value
								 * define the pup as
								 * converge and step
								 * back to nominal
								 * Vref.
								 */
								pup_st[pup]
									[if_id] =
									VREF_CONVERGE;

								algo_run_flag++;
								interface_state
									[if_id]++;
								DEBUG_TRAINING_HW_ALG
									(DEBUG_LEVEL_TRACE,
									 ("I/F[ %d ], pup[ %d ] VREF_CONVERGE - Vref = %X (%d)\n",
									  if_id, pup,
									  current_vref[pup]
									  [if_id],
									  __LINE__));
							} else {
								current_vref[pup]
									[if_id] =
									current_vref[pup]
									[if_id] -
									second_step;
							}

						/* Update the Vref for next stage */
						currrent_vref =
							current_vref[pup]
							[if_id];
						CHECK_STATUS
							(ddr3_tip_bus_read
							 (dev_num, if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  &val));
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  (val & (~0xf)) |
							  vref_map[currrent_vref]));
						DEBUG_TRAINING_HW_ALG
							(DEBUG_LEVEL_TRACE,
							 ("VREF config, IF[ %d ]pup[ %d ] - Vref = %X (%d)\n",
							  if_id, pup,
							  (val & (~0xf)) |
							  vref_map[currrent_vref],
							  __LINE__));
					}
				} else {
					/* we change state and change step */
					if (pup_st[pup][if_id] == VREF_STEP_1) {
						pup_st[pup][if_id] =
							VREF_STEP_2;
						lim_vref[pup][if_id] =
							current_vref[pup]
							[if_id] - initial_step;
						last_valid_window[pup]
							[if_id] =
							current_valid_window[pup]
							[if_id];
						last_vref[pup][if_id] =
							current_vref[pup]
							[if_id];
						current_vref[pup][if_id] =
							last_vref[pup][if_id] -
							second_step;

						/* Update the Vref for next stage */
						CHECK_STATUS
							(ddr3_tip_bus_read
							 (dev_num, if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  &val));
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  (val & (~0xf)) |
							  vref_map[current_vref[pup]
								   [if_id]]));
						DEBUG_TRAINING_HW_ALG
							(DEBUG_LEVEL_TRACE,
							 ("VREF config, IF[ %d ]pup[ %d ] - Vref = %X (%d)\n",
							  if_id, pup,
							  (val & (~0xf)) |
							  vref_map[current_vref[pup]
								   [if_id]],
							  __LINE__));

					} else if (pup_st[pup][if_id] == VREF_STEP_2) {
						/*
						 * The last search was the max
						 * point set value and exit
						 */
						CHECK_STATUS
							(ddr3_tip_bus_read
							 (dev_num, if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  &val));
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST, pup,
							  DDR_PHY_DATA, reg_addr,
							  (val & (~0xf)) |
							  vref_map[last_vref[pup]
								   [if_id]]));
						DEBUG_TRAINING_HW_ALG
							(DEBUG_LEVEL_TRACE,
							 ("VREF config, IF[ %d ]pup[ %d ] - Vref = %X (%d)\n",
							  if_id, pup,
							  (val & (~0xf)) |
							  vref_map[last_vref[pup]
								   [if_id]],
							  __LINE__));
						pup_st[pup][if_id] =
							VREF_CONVERGE;
						algo_run_flag++;
						interface_state[if_id]++;
						DEBUG_TRAINING_HW_ALG
							(DEBUG_LEVEL_TRACE,
							 ("I/F[ %d ], pup[ %d ] VREF_CONVERGE - Vref = %X (%d)\n",
							  if_id, pup,
							  current_vref[pup]
							  [if_id], __LINE__));
					}
				}
			}
		}
	}

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (pup = 0;
		     pup < octets_per_if_num; pup++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup);
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id,
				      ACCESS_TYPE_UNICAST, pup,
				      DDR_PHY_DATA, reg_addr, &val));
			DEBUG_TRAINING_HW_ALG(
				DEBUG_LEVEL_INFO,
				("FINAL values: I/F[ %d ], pup[ %d ] - Vref = %X (%d)\n",
				 if_id, pup, val, __LINE__));
		}
	}

	flow_result[if_id] = TEST_SUCCESS;

	/* restore start/end pattern */
	start_pattern = copy_start_pattern;
	end_pattern = copy_end_pattern;

	return 0;
}

/*
 * CK/CA Delay
 */
int ddr3_tip_cmd_addr_init_delay(u32 dev_num, u32 adll_tap)
{
	u32 if_id = 0;
	u32 ck_num_adll_tap = 0, ca_num_adll_tap = 0, data = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/*
	 * ck_delay_table is delaying the of the clock signal only.
	 * (to overcome timing issues between_c_k & command/address signals)
	 */
	/*
	 * ca_delay is delaying the of the entire command & Address signals
	 * (include Clock signal to overcome DGL error on the Clock versus
	 * the DQS).
	 */

	/* Calc ADLL Tap */
	if (ck_delay == PARAM_UNDEFINED)
		DEBUG_TRAINING_HW_ALG(
			DEBUG_LEVEL_ERROR,
			("ERROR: ck_delay is not initialized!\n"));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

		/* Calc delay ps in ADLL tap */
		ck_num_adll_tap = ck_delay / adll_tap;
		ca_num_adll_tap = ca_delay / adll_tap;

		data = (ck_num_adll_tap & 0x3f) +
			((ca_num_adll_tap & 0x3f) << 10);

		/*
		 * Set the ADLL number to the CK ADLL for Interfaces for
		 * all Pup
		 */
		DEBUG_TRAINING_HW_ALG(
			DEBUG_LEVEL_TRACE,
			("ck_num_adll_tap %d ca_num_adll_tap %d adll_tap %d\n",
			 ck_num_adll_tap, ca_num_adll_tap, adll_tap));

		CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST,
						if_id, ACCESS_TYPE_MULTICAST,
						PARAM_NOT_CARE, DDR_PHY_CONTROL,
						0x0, data));
	}

	return MV_OK;
}
