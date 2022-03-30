// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_regs.h"

static u32 bist_offset = 32;
enum hws_pattern sweep_pattern = PATTERN_KILLER_DQ0;

static int ddr3_tip_bist_operation(u32 dev_num,
				   enum hws_access_type access_type,
				   u32 if_id,
				   enum hws_bist_operation oper_type);

/*
 * BIST activate
 */
int ddr3_tip_bist_activate(u32 dev_num, enum hws_pattern pattern,
			   enum hws_access_type access_type, u32 if_num,
			   enum hws_dir dir,
			   enum hws_stress_jump addr_stress_jump,
			   enum hws_pattern_duration duration,
			   enum hws_bist_operation oper_type,
			   u32 offset, u32 cs_num, u32 pattern_addr_length)
{
	u32 tx_burst_size;
	u32 delay_between_burst;
	u32 rd_mode;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();

	/* odpg bist write enable */
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG,
			  (ODPG_WRBUF_WR_CTRL_ENA << ODPG_WRBUF_WR_CTRL_OFFS),
			  (ODPG_WRBUF_WR_CTRL_MASK << ODPG_WRBUF_WR_CTRL_OFFS));

	/* odpg bist read enable/disable */
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG,
			  (dir == OPER_READ) ? (ODPG_WRBUF_RD_CTRL_ENA << ODPG_WRBUF_RD_CTRL_OFFS) :
					       (ODPG_WRBUF_RD_CTRL_DIS << ODPG_WRBUF_RD_CTRL_OFFS),
			  (ODPG_WRBUF_RD_CTRL_MASK << ODPG_WRBUF_RD_CTRL_OFFS));

	ddr3_tip_load_pattern_to_odpg(0, access_type, 0, pattern, offset);

	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_BUFFER_SIZE_REG, pattern_addr_length, MASK_ALL_BITS);
	tx_burst_size = (dir == OPER_WRITE) ?
		pattern_table[pattern].tx_burst_size : 0;
	delay_between_burst = (dir == OPER_WRITE) ? 2 : 0;
	rd_mode = (dir == OPER_WRITE) ? 1 : 0;
	ddr3_tip_configure_odpg(0, access_type, 0, dir,
		      pattern_table[pattern].num_of_phases_tx, tx_burst_size,
		      pattern_table[pattern].num_of_phases_rx,
		      delay_between_burst,
		      rd_mode, cs_num, addr_stress_jump, duration);
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_BUFFER_OFFS_REG, offset, MASK_ALL_BITS);

	if (oper_type == BIST_STOP) {
		ddr3_tip_bist_operation(0, access_type, 0, BIST_STOP);
	} else {
		ddr3_tip_bist_operation(0, access_type, 0, BIST_START);
		if (mv_ddr_is_odpg_done(MAX_POLLING_ITERATIONS) != MV_OK)
			return MV_FAIL;
		ddr3_tip_bist_operation(0, access_type, 0, BIST_STOP);
	}
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG, 0, MASK_ALL_BITS);

	return MV_OK;
}

/*
 * BIST read result
 */
int ddr3_tip_bist_read_result(u32 dev_num, u32 if_id,
			      struct bist_result *pst_bist_result)
{
	int ret;
	u32 read_data[MAX_INTERFACE_NUM];
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (IS_IF_ACTIVE(tm->if_act_mask, if_id) == 0)
		return MV_NOT_SUPPORTED;
	DEBUG_TRAINING_BIST_ENGINE(DEBUG_LEVEL_TRACE,
				   ("ddr3_tip_bist_read_result if_id %d\n",
				    if_id));
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_DATA_RX_WORD_ERR_DATA_HIGH_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_fail_high = read_data[if_id];
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_DATA_RX_WORD_ERR_DATA_LOW_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_fail_low = read_data[if_id];

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_DATA_RX_WORD_ERR_ADDR_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_last_fail_addr = read_data[if_id];
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_DATA_RX_WORD_ERR_CNTR_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_error_cnt = read_data[if_id];

	return MV_OK;
}

/*
 * BIST flow - Activate & read result
 */
int hws_ddr3_run_bist(u32 dev_num, enum hws_pattern pattern, u32 *result,
		      u32 cs_num)
{
	int ret;
	u32 i = 0;
	u32 win_base;
	struct bist_result st_bist_result;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, i);
		hws_ddr3_cs_base_adr_calc(i, cs_num, &win_base);
		ret = ddr3_tip_bist_activate(dev_num, pattern,
					     ACCESS_TYPE_UNICAST,
					     i, OPER_WRITE, STRESS_NONE,
					     DURATION_SINGLE, BIST_START,
					     bist_offset + win_base,
					     cs_num, 15);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_activate failed (0x%x)\n", ret);
			return ret;
		}

		ret = ddr3_tip_bist_activate(dev_num, pattern,
					     ACCESS_TYPE_UNICAST,
					     i, OPER_READ, STRESS_NONE,
					     DURATION_SINGLE, BIST_START,
					     bist_offset + win_base,
					     cs_num, 15);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_activate failed (0x%x)\n", ret);
			return ret;
		}

		ret = ddr3_tip_bist_read_result(dev_num, i, &st_bist_result);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_read_result failed\n");
			return ret;
		}
		result[i] = st_bist_result.bist_error_cnt;
	}

	return MV_OK;
}

/*
 * Set BIST Operation
 */

static int ddr3_tip_bist_operation(u32 dev_num,
				   enum hws_access_type access_type,
				   u32 if_id, enum hws_bist_operation oper_type)
{
	if (oper_type == BIST_STOP)
		mv_ddr_odpg_disable();
	else
		mv_ddr_odpg_enable();

	return MV_OK;
}

/*
 * Print BIST result
 */
void ddr3_tip_print_bist_res(void)
{
	u32 dev_num = 0;
	u32 i;
	struct bist_result st_bist_result[MAX_INTERFACE_NUM];
	int res;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, i);

		res = ddr3_tip_bist_read_result(dev_num, i, &st_bist_result[i]);
		if (res != MV_OK) {
			DEBUG_TRAINING_BIST_ENGINE(
				DEBUG_LEVEL_ERROR,
				("ddr3_tip_bist_read_result failed\n"));
			return;
		}
	}

	DEBUG_TRAINING_BIST_ENGINE(
		DEBUG_LEVEL_INFO,
		("interface | error_cnt | fail_low | fail_high | fail_addr\n"));

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, i);

		DEBUG_TRAINING_BIST_ENGINE(
			DEBUG_LEVEL_INFO,
			("%d |  0x%08x  |  0x%08x  |  0x%08x  | 0x%08x\n",
			 i, st_bist_result[i].bist_error_cnt,
			 st_bist_result[i].bist_fail_low,
			 st_bist_result[i].bist_fail_high,
			 st_bist_result[i].bist_last_fail_addr));
	}
}

enum {
	PASS,
	FAIL
};
#define TIP_ITERATION_NUM	31
static int mv_ddr_tip_bist(enum hws_dir dir, u32 val, enum hws_pattern pattern, u32 cs, u32 *result)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum hws_training_ip_stat training_result;
	u16 *reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u32 max_subphy = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 subphy, read_data;

	ddr3_tip_ip_training(0, ACCESS_TYPE_MULTICAST, 0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			     RESULT_PER_BYTE, HWS_CONTROL_ELEMENT_ADLL, HWS_LOW2HIGH, dir, tm->if_act_mask, val,
			     TIP_ITERATION_NUM, pattern, EDGE_FP, CS_SINGLE, cs, &training_result);

	for (subphy = 0; subphy < max_subphy; subphy++) {
		ddr3_tip_if_read(0, ACCESS_TYPE_UNICAST, 0, reg_map[subphy], &read_data, MASK_ALL_BITS);
		if (((read_data >> BLOCK_STATUS_OFFS) & BLOCK_STATUS_MASK) == BLOCK_STATUS_NOT_LOCKED)
			*result |= (FAIL << subphy);
	}

	return MV_OK;
}

struct interval {
	u8 *vector;
	u8 lendpnt;		/* interval's left endpoint */
	u8 rendpnt;		/* interval's right endpoint */
	u8 size;		/* interval's size */
	u8 lmarker;		/* left marker */
	u8 rmarker;		/* right marker */
	u8 pass_lendpnt;	/* left endpoint of internal pass interval */
	u8 pass_rendpnt;	/* right endpoint of internal pass interval */
};

static int interval_init(u8 *vector, u8 lendpnt, u8 rendpnt,
			 u8 lmarker, u8 rmarker, struct interval *intrvl)
{
	if (intrvl == NULL) {
		printf("%s: NULL intrvl pointer found\n", __func__);
		return MV_FAIL;
	}

	if (vector == NULL) {
		printf("%s: NULL vector pointer found\n", __func__);
		return MV_FAIL;
	}
	intrvl->vector = vector;

	if (lendpnt >= rendpnt) {
		printf("%s: incorrect lendpnt and/or rendpnt parameters found\n", __func__);
		return MV_FAIL;
	}
	intrvl->lendpnt = lendpnt;
	intrvl->rendpnt = rendpnt;
	intrvl->size = rendpnt - lendpnt + 1;

	if ((lmarker < lendpnt) || (lmarker > rendpnt)) {
		printf("%s: incorrect lmarker parameter found\n", __func__);
		return MV_FAIL;
	}
	intrvl->lmarker = lmarker;

	if ((rmarker < lmarker) || (rmarker > (intrvl->rendpnt + intrvl->size))) {
		printf("%s: incorrect rmarker parameter found\n", __func__);
		return MV_FAIL;
	}
	intrvl->rmarker = rmarker;

	return MV_OK;
}
static int interval_set(u8 pass_lendpnt, u8 pass_rendpnt, struct interval *intrvl)
{
	if (intrvl == NULL) {
		printf("%s: NULL intrvl pointer found\n", __func__);
		return MV_FAIL;
	}

	intrvl->pass_lendpnt = pass_lendpnt;
	intrvl->pass_rendpnt = pass_rendpnt;

	return MV_OK;
}

static int interval_proc(struct interval *intrvl)
{
	int curr;
	int pass_lendpnt, pass_rendpnt;
	int lmt;
	int fcnt = 0, pcnt = 0;

	if (intrvl == NULL) {
		printf("%s: NULL intrvl pointer found\n", __func__);
		return MV_FAIL;
	}

	/* count fails and passes */
	curr = intrvl->lendpnt;
	while (curr <= intrvl->rendpnt) {
		if (intrvl->vector[curr] == PASS)
			pcnt++;
		else
			fcnt++;
		curr++;
	}

	/* check for all fail */
	if (fcnt == intrvl->size) {
		printf("%s: no pass found\n", __func__);
		return MV_FAIL;
	}

	/* check for all pass */
	if (pcnt == intrvl->size) {
		if (interval_set(intrvl->lendpnt, intrvl->rendpnt, intrvl) != MV_OK)
			return MV_FAIL;
		return MV_OK;
	}

	/* proceed with rmarker */
	curr = intrvl->rmarker;
	if (intrvl->vector[curr % intrvl->size] == PASS) { /* pass at rmarker */
		/* search for fail on right */
		if (intrvl->rmarker > intrvl->rendpnt)
			lmt = intrvl->rendpnt + intrvl->size;
		else
			lmt = intrvl->rmarker + intrvl->size - 1;
		while ((curr <= lmt) &&
		       (intrvl->vector[curr % intrvl->size] == PASS))
			curr++;
		if (curr > lmt) { /* fail not found */
			printf("%s: rmarker: fail following pass not found\n", __func__);
			return MV_FAIL;
		}
		/* fail found */
		pass_rendpnt = curr - 1;
	} else { /* fail at rmarker */
		/* search for pass on left */
		if (intrvl->rmarker > intrvl->rendpnt)
			lmt = intrvl->rmarker - intrvl->size + 1;
		else
			lmt = intrvl->lendpnt;
		while ((curr >= lmt) &&
		       (intrvl->vector[curr % intrvl->size] == FAIL))
			curr--;
		if (curr < lmt) { /* pass not found */
			printf("%s: rmarker: pass preceding fail not found\n", __func__);
			return MV_FAIL;
		}
		/* pass found */
		pass_rendpnt = curr;
	}

	/* search for fail on left */
	curr = pass_rendpnt;
	if (pass_rendpnt > intrvl->rendpnt)
		lmt =  pass_rendpnt - intrvl->size + 1;
	else
		lmt = intrvl->lendpnt;
	while ((curr >= lmt) &&
	       (intrvl->vector[curr % intrvl->size] == PASS))
		curr--;
	if (curr < lmt) { /* fail not found */
		printf("%s: rmarker: fail preceding pass not found\n", __func__);
		return MV_FAIL;
	}
	/* fail found */
	pass_lendpnt = curr + 1;
	if (interval_set(pass_lendpnt, pass_rendpnt, intrvl) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

#define ADLL_TAPS_PER_PERIOD	64
int mv_ddr_dm_to_dq_diff_get(u8 vw_sphy_hi_lmt, u8 vw_sphy_lo_lmt, u8 *vw_vector,
			     int *vw_sphy_hi_diff, int *vw_sphy_lo_diff)
{
	struct interval intrvl;

	/* init interval structure */
	if (interval_init(vw_vector, 0, ADLL_TAPS_PER_PERIOD - 1,
			  vw_sphy_lo_lmt, vw_sphy_hi_lmt, &intrvl) != MV_OK)
		return MV_FAIL;

	/* find pass sub-interval */
	if (interval_proc(&intrvl) != MV_OK)
		return MV_FAIL;

	/* check for all pass */
	if ((intrvl.pass_rendpnt == intrvl.rendpnt) &&
	    (intrvl.pass_lendpnt == intrvl.lendpnt)) {
		printf("%s: no fail found\n", __func__);
		return MV_FAIL;
	}

	*vw_sphy_hi_diff = intrvl.pass_rendpnt - vw_sphy_hi_lmt;
	*vw_sphy_lo_diff = vw_sphy_lo_lmt - intrvl.pass_lendpnt;

	return MV_OK;
}

static int mv_ddr_bist_tx(enum hws_access_type access_type)
{
	mv_ddr_odpg_done_clr();

	ddr3_tip_bist_operation(0, access_type, 0, BIST_START);

	if (mv_ddr_is_odpg_done(MAX_POLLING_ITERATIONS) != MV_OK)
		return MV_FAIL;

	ddr3_tip_bist_operation(0, access_type, 0, BIST_STOP);

	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG, 0, MASK_ALL_BITS);

	return MV_OK;
}

/* prepare odpg for bist operation */
#define WR_OP_ODPG_DATA_CMD_BURST_DLY	2
static int mv_ddr_odpg_bist_prepare(enum hws_pattern pattern, enum hws_access_type access_type,
			     enum hws_dir dir, enum hws_stress_jump stress_jump_addr,
			     enum hws_pattern_duration duration, u32 offset, u32 cs,
			     u32 pattern_addr_len, enum dm_direction dm_dir)
{
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u32 tx_burst_size;
	u32 burst_delay;
	u32 rd_mode;

	/* odpg bist write enable */
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG,
			  (ODPG_WRBUF_WR_CTRL_ENA << ODPG_WRBUF_WR_CTRL_OFFS),
			  (ODPG_WRBUF_WR_CTRL_MASK << ODPG_WRBUF_WR_CTRL_OFFS));

	/* odpg bist read enable/disable */
	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_CTRL_REG,
			  (dir == OPER_READ) ? (ODPG_WRBUF_RD_CTRL_ENA << ODPG_WRBUF_RD_CTRL_OFFS) :
					       (ODPG_WRBUF_RD_CTRL_DIS << ODPG_WRBUF_RD_CTRL_OFFS),
			  (ODPG_WRBUF_RD_CTRL_MASK << ODPG_WRBUF_RD_CTRL_OFFS));

	if (pattern == PATTERN_00 || pattern == PATTERN_FF)
		ddr3_tip_load_pattern_to_odpg(0, access_type, 0, pattern, offset);
	else
		mv_ddr_load_dm_pattern_to_odpg(access_type, pattern, dm_dir);

	ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_BUFFER_SIZE_REG, pattern_addr_len, MASK_ALL_BITS);
	if (dir == OPER_WRITE) {
		tx_burst_size = pattern_table[pattern].tx_burst_size;
		burst_delay = WR_OP_ODPG_DATA_CMD_BURST_DLY;
		rd_mode = ODPG_MODE_TX;
	} else {
		tx_burst_size = 0;
		burst_delay = 0;
		rd_mode = ODPG_MODE_RX;
	}
	ddr3_tip_configure_odpg(0, access_type, 0, dir, pattern_table[pattern].num_of_phases_tx,
				tx_burst_size, pattern_table[pattern].num_of_phases_rx, burst_delay,
				rd_mode, cs, stress_jump_addr, duration);

	return MV_OK;
}

#define BYTES_PER_BURST_64BIT	0x20
#define BYTES_PER_BURST_32BIT	0x10
int mv_ddr_dm_vw_get(enum hws_pattern pattern, u32 cs, u8 *vw_vector)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u32 adll_tap;
	u32 wr_ctrl_adll[MAX_BUS_NUM] = {0};
	u32 rd_ctrl_adll[MAX_BUS_NUM] = {0};
	u32 subphy;
	u32 subphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 odpg_addr = 0x0;
	u32 result;
	u32 idx;
	/* burst length in bytes */
	u32 burst_len = (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask) ?
			BYTES_PER_BURST_64BIT : BYTES_PER_BURST_32BIT);

	/* save dqs values to restore after algorithm's run */
	ddr3_tip_read_adll_value(0, wr_ctrl_adll, CTX_PHY_REG(cs), MASK_ALL_BITS);
	ddr3_tip_read_adll_value(0, rd_ctrl_adll, CRX_PHY_REG(cs), MASK_ALL_BITS);

	/* fill memory with base pattern */
	ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_CTRL_REG, 0, MASK_ALL_BITS);
	mv_ddr_odpg_bist_prepare(pattern, ACCESS_TYPE_UNICAST, OPER_WRITE, STRESS_NONE, DURATION_SINGLE,
				 bist_offset, cs, pattern_table[pattern].num_of_phases_tx,
				 (pattern == PATTERN_00) ? DM_DIR_DIRECT : DM_DIR_INVERSE);

	for (adll_tap = 0; adll_tap < ADLL_TAPS_PER_PERIOD; adll_tap++) {
		/* change target odpg address */
		odpg_addr = adll_tap * burst_len;
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_BUFFER_OFFS_REG,
				  odpg_addr, MASK_ALL_BITS);

		ddr3_tip_configure_odpg(0, ACCESS_TYPE_UNICAST, 0, OPER_WRITE,
					pattern_table[pattern].num_of_phases_tx,
					pattern_table[pattern].tx_burst_size,
					pattern_table[pattern].num_of_phases_rx,
					WR_OP_ODPG_DATA_CMD_BURST_DLY,
					ODPG_MODE_TX, cs, STRESS_NONE, DURATION_SINGLE);

		/* odpg bist write enable */
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_CTRL_REG,
				  (ODPG_WRBUF_WR_CTRL_ENA << ODPG_WRBUF_WR_CTRL_OFFS),
				  (ODPG_WRBUF_WR_CTRL_MASK << ODPG_WRBUF_WR_CTRL_OFFS));

		/* odpg bist read disable */
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_CTRL_REG,
				  (ODPG_WRBUF_RD_CTRL_DIS << ODPG_WRBUF_RD_CTRL_OFFS),
				  (ODPG_WRBUF_RD_CTRL_MASK << ODPG_WRBUF_RD_CTRL_OFFS));

		/* trigger odpg */
		mv_ddr_bist_tx(ACCESS_TYPE_MULTICAST);
	}

	/* fill memory with vref pattern to increment addr using odpg bist */
	mv_ddr_odpg_bist_prepare(PATTERN_VREF, ACCESS_TYPE_UNICAST, OPER_WRITE, STRESS_NONE, DURATION_SINGLE,
				 bist_offset, cs, pattern_table[pattern].num_of_phases_tx,
				 (pattern == PATTERN_00) ? DM_DIR_DIRECT : DM_DIR_INVERSE);

	for (adll_tap = 0; adll_tap < ADLL_TAPS_PER_PERIOD; adll_tap++) {
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_MULTICAST, 0,
				   DDR_PHY_DATA, CTX_PHY_REG(cs), adll_tap);
		/* change target odpg address */
		odpg_addr = adll_tap * burst_len;
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_BUFFER_OFFS_REG,
				  odpg_addr, MASK_ALL_BITS);
		ddr3_tip_configure_odpg(0, ACCESS_TYPE_UNICAST, 0, OPER_WRITE,
					pattern_table[pattern].num_of_phases_tx,
					pattern_table[pattern].tx_burst_size,
					pattern_table[pattern].num_of_phases_rx,
					WR_OP_ODPG_DATA_CMD_BURST_DLY,
					ODPG_MODE_TX, cs, STRESS_NONE, DURATION_SINGLE);

		/* odpg bist write enable */
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_CTRL_REG,
				  (ODPG_WRBUF_WR_CTRL_ENA << ODPG_WRBUF_WR_CTRL_OFFS),
				  (ODPG_WRBUF_WR_CTRL_MASK << ODPG_WRBUF_WR_CTRL_OFFS));

		/* odpg bist read disable */
		ddr3_tip_if_write(0, ACCESS_TYPE_UNICAST, 0, ODPG_DATA_CTRL_REG,
				  (ODPG_WRBUF_RD_CTRL_DIS << ODPG_WRBUF_RD_CTRL_OFFS),
				  (ODPG_WRBUF_RD_CTRL_MASK << ODPG_WRBUF_RD_CTRL_OFFS));

		/* trigger odpg */
		mv_ddr_bist_tx(ACCESS_TYPE_MULTICAST);
	}

	/* restore subphy's tx adll_tap to its position */
	for (subphy = 0; subphy < subphy_max; subphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST,
				   subphy, DDR_PHY_DATA, CTX_PHY_REG(cs),
				   wr_ctrl_adll[subphy]);
	}

	/* read and validate bist (comparing with the base pattern) */
	for (adll_tap = 0; adll_tap < ADLL_TAPS_PER_PERIOD; adll_tap++) {
		result = 0;
		odpg_addr = adll_tap * burst_len;
		/* change addr to fit write */
		mv_ddr_pattern_start_addr_set(pattern_table, pattern, odpg_addr);
		mv_ddr_tip_bist(OPER_READ, 0, pattern, 0, &result);
		for (subphy = 0; subphy < subphy_max; subphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy);
			idx = ADLL_TAPS_PER_PERIOD * subphy + adll_tap;
			vw_vector[idx] |= ((result >> subphy) & 0x1);
		}
	}

	/* restore subphy's rx adll_tap to its position */
	for (subphy = 0; subphy < subphy_max; subphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy);
		ddr3_tip_bus_write(0, ACCESS_TYPE_UNICAST, 0, ACCESS_TYPE_UNICAST,
				   subphy, DDR_PHY_DATA, CRX_PHY_REG(cs),
				   rd_ctrl_adll[subphy]);
	}

	return MV_OK;
}
