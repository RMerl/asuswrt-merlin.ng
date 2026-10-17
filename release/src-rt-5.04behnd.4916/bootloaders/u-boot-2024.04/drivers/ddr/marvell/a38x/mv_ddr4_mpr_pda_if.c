// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#if defined(CONFIG_DDR4)

/* DDR4 MPR/PDA Interface */
#include "ddr3_init.h"
#include "mv_ddr4_mpr_pda_if.h"
#include "mv_ddr4_training.h"
#include "mv_ddr_training_db.h"
#include "mv_ddr_common.h"
#include "mv_ddr_regs.h"

static u8 dram_to_mc_dq_map[MAX_BUS_NUM][BUS_WIDTH_IN_BITS];
static int dq_map_enable;

static u32 mv_ddr4_tx_odt_get(void)
{
	u16 odt = 0xffff, rtt = 0xffff;

	if (g_odt_config & 0xe0000)
		rtt =  mv_ddr4_rtt_nom_to_odt(g_rtt_nom);
	else if (g_odt_config & 0x10000)
		rtt = mv_ddr4_rtt_wr_to_odt(g_rtt_wr);
	else
		return odt;

	return (odt * rtt) / (odt + rtt);
}

/*
 * mode registers initialization function
 * replaces all MR writes in DDR3 init function
 */
int mv_ddr4_mode_regs_init(u8 dev_num)
{
	int status;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum hws_access_type access_type = ACCESS_TYPE_UNICAST;
	u32 if_id;
	u32 cl, cwl;
	u32 val, mask;
	u32 t_wr, t_ckclk;
	/* design GL params to be set outside */
	u32 dic = 0;
	u32 ron = 30; /* znri */
	u32 rodt = mv_ddr4_tx_odt_get(); /* effective rtt */
	/* vref percentage presented as 100 x percentage value (e.g., 6000 = 100 x 60%) */
	u32 vref = ((ron + rodt / 2) * 10000) / (ron + rodt);
	u32 range = (vref >= 6000) ? 0 : 1; /* if vref is >= 60%, use upper range */
	u32 tap;
	u32 refresh_mode;

	if (range == 0)
		tap = (vref - 6000) / 65;
	else
		tap = (vref - 4500) / 65;

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		cl = tm->interface_params[if_id].cas_l;
		cwl = tm->interface_params[if_id].cas_wl;
		t_ckclk = MEGA / mv_ddr_freq_get(tm->interface_params[if_id].memory_freq);
		t_wr = time_to_nclk(mv_ddr_speed_bin_timing_get(tm->interface_params[if_id].speed_bin_index,
					    SPEED_BIN_TWR), t_ckclk) - 1;

		/* TODO: replace hard-coded values with appropriate defines */
		/* DDR4 MR0 */
		/*
		 * [6:4,2] bits to be taken from S@R frequency and speed bin
		 * rtt_nom to be taken from the algorithm definition
		 * dic to be taken fro the algorithm definition -
		 * set to 0x1 (for driver rzq/5 = 48 ohm) or
		 * set to 0x0 (for driver rzq/7 = 34 ohm)
		 */
		/* set dll reset, 0x1900[8] to 0x1 */
		/* set tm, 0x1900[7] to 0x0 */
		/* set rbt, 0x1900[3] to 0x0 */
		/* set bl, 0x1900[1:0] to 0x0 */
		val = ((cl_mask_table[cl] & 0x1) << 2) |
		      (((cl_mask_table[cl] & 0xe) >> 1)  <<  4) |
		      (twr_mask_table[t_wr + 1] << 9) |
		      (0x1 << 8) | (0x0 << 7) | (0x0 << 3) | 0x0;
		mask = (0x1 << 2) | (0x7 << 4) | (0x7 << 9) |
		       (0x1 << 8) | (0x1 << 7) | (0x1 << 3) | 0x3;
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR0_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR1 */
		/* set rtt nom to 0 if rtt park is activated (not zero) */
		if ((g_rtt_park >> 6) != 0x0)
			g_rtt_nom = 0;
		/* set tdqs, 0x1904[11] to 0x0 */
		/* set al, 0x1904[4:3] to 0x0 */
		/* dic, 0x1904[2:1] */
		/* dll enable */
		val = g_rtt_nom | (0x0 << 11) | (0x0 << 3) | (dic << 1) | 0x1;
		mask = (0x7 << 8) | (0x1 << 11) | (0x3 << 3) | (0x3 << 1) | 0x1;
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR1_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR2 */
		/* set rtt wr, 0x1908[10,9] to 0x0 */
		/* set wr crc, 0x1908[12] to 0x0 */
		/* cwl */
		val = g_rtt_wr | (0x0 << 12) | (cwl_mask_table[cwl] << 3);
		mask = (0x3 << 9) | (0x1 << 12) | (0x7 << 3);
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR2_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR3 */
		/* set fgrm, 0x190C[8:6] to 0x0 */
		/* set gd, 0x190C[3] to 0x0 */
		refresh_mode = (tm->interface_params[if_id].interface_temp == MV_DDR_TEMP_HIGH) ? 1 : 0;

		val = (refresh_mode << 6) | (0x0 << 3);
		mask = (0x7 << 6) | (0x1 << 3);
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR3_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR4 */
		/*
		 * set wp, 0x1910[12] to 0x0
		 * set rp, 0x1910[11] to 0x0
		 * set rp training, 0x1910[10] to 0x0
		 * set sra, 0x1910[9] to 0x0
		 * set cs2cmd, 0x1910[8:6] to 0x0
		 * set mpd, 0x1910[1] to 0x0
		 */
		mask = (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x7 << 6) | (0x1 << 1);
		val =  (0x0 << 12) | (0x1 << 11) | (0x0 << 10) | (0x0 << 9) | (0x0 << 6) | (0x0 << 1);

		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR4_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR5 */
		/*
		 * set rdbi, 0x1914[12] to 0x0 during init sequence (may be enabled with
		 * op cmd mrs - bug in z1, to be fixed in a0)
		 * set wdbi, 0x1914[11] to 0x0
		 * set dm, 0x1914[10] to 0x1
		 * set ca_pl, 0x1914[2:0] to 0x0
		 * set odt input buffer during power down mode, 0x1914[5] to 0x1
		 */
		mask = (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x7 << 6) | (0x1 << 5) | 0x7;
		val = (0x0 << 12) | (0x0 << 11) | (0x1 << 10) | g_rtt_park | (0x1 << 5) | 0x0;
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR5_REG,
					   val, mask);
		if (status != MV_OK)
			return status;

		/* DDR4 MR6 */
		/*
		 * set t_ccd_l, 0x1918[12:10] to 0x0, 0x2, or 0x4 (z1 supports only even
		 * values, to be fixed in a0)
		 * set vdq te, 0x1918[7] to 0x0
		 * set vdq tv, 0x1918[5:0] to vref training value
		 */
		mask = (0x7 << 10) | (0x1 << 7) | (0x1 << 6) | 0x3f;
		val = (0x2 << 10) | (0x0 << 7) | (range << 6) | tap;
		status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR6_REG,
					   val, mask);
		if (status != MV_OK)
			return status;
	}

	return MV_OK;
}

/* enter mpr read mode */
static int mv_ddr4_mpr_read_mode_enable(u8 dev_num, u32 mpr_num, u32 page_num,
				 enum mv_ddr4_mpr_read_format read_format)
{
	/*
	 * enable MPR page 2 mpr mode in DDR4 MR3
	 * read_format: 0 for serial, 1 for parallel, and 2 for staggered
	 * TODO: add support for cs, multicast or unicast, and if id
	 */
	int status;
	u32 val, mask, if_id = 0;

	if (page_num != 0) {
		/* serial is the only read format if the page is other than 0 */
		read_format = MV_DDR4_MPR_READ_SERIAL;
	}

	val = (page_num << 0) | (0x1 << 2) | (read_format << 11);
	mask = (0x3 << 0) | (0x1 << 2) | (0x3 << 11);

	/* cs0 */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DDR4_MR3_REG, val, mask);
	if (status != MV_OK)
		return status;

	/* op cmd: cs0, cs1 are on, cs2, cs3 are off */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, SDRAM_OP_REG,
				   (0x9 | (0xc << 8)) , (0x1f | (0xf << 8)));
	if (status != MV_OK)
		return status;

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_mpr_read_mode_enable: DDR3 poll failed(MPR3)\n"));
	}

	return MV_OK;
}

/* exit mpr read or write mode */
static int mv_ddr4_mpr_mode_disable(u8 dev_num)
{
	 /* TODO: add support for cs, multicast or unicast, and if id */
	int status;
	u32 val, mask, if_id = 0;

	/* exit mpr */
	val =  0x0 << 2;
	mask =  0x1 << 2;
	/* cs0 */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DDR4_MR3_REG, val, mask);
	if (status != MV_OK)
		return status;

	/* op cmd: cs0, cs1 are on, cs2, cs3 are off */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, SDRAM_OP_REG,
				   (0x9 | (0xc << 8)) , (0x1f | (0xf << 8)));
	if (status != MV_OK)
		return status;

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_mpr_mode_disable: DDR3 poll failed(MPR3)\n"));
	}

	return MV_OK;
}

/* translate dq read value per dram dq pin */
static int mv_ddr4_dq_decode(u8 dev_num, u32 *data)
{
	u32 subphy_num, dq_num;
	u32 dq_val = 0, raw_data, idx;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 subphy_max = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);

	/* suppose the third word is stable */
	raw_data = data[2];

	/* skip ecc supbhy; TODO: check to add support for ecc */
	if (subphy_max % 2)
		subphy_max -= 1;

	for (subphy_num = 0; subphy_num < subphy_max; subphy_num++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy_num);
		for (dq_num = 0; dq_num < BUS_WIDTH_IN_BITS; dq_num++) {
			idx = (dram_to_mc_dq_map[subphy_num][dq_num] + (subphy_num * BUS_WIDTH_IN_BITS));
			dq_val |= (((raw_data & (1 << idx)) >> idx) << ((subphy_num * BUS_WIDTH_IN_BITS) + dq_num));
		}
	}

	/* update burst words[0..7] with correct mapping */
	for (idx = 0; idx < EXT_ACCESS_BURST_LENGTH; idx++)
		data[idx] = dq_val;

	return MV_OK;
}

/*
 * read mpr value per requested format and type
 * note: for parallel decoded read, data is presented as stored in mpr on dram side,
 *	for all others, data to be presneted "as is" (i.e. per dq order from high to low
 *	and bus pins connectivity).
 */
int mv_ddr4_mpr_read(u8 dev_num, u32 mpr_num, u32 page_num,
		      enum mv_ddr4_mpr_read_format read_format,
		      enum mv_ddr4_mpr_read_type read_type,
		      u32 *data)
{
	/* TODO: add support for multiple if_id, dev num, and cs */
	u32 word_idx, if_id = 0;
	volatile unsigned long *addr = NULL;

	/* enter mpr read mode */
	mv_ddr4_mpr_read_mode_enable(dev_num, mpr_num, page_num, read_format);

	/* set pattern type*/
	ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DDR4_MPR_WR_REG,
			  mpr_num << 8, 0x3 << 8);

	for (word_idx = 0; word_idx < EXT_ACCESS_BURST_LENGTH; word_idx++) {
		data[word_idx] = *addr;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("mv_ddr4_mpr_read: addr 0x%08lx, data 0x%08x\n",
						     (unsigned long)addr, data[word_idx]));
		addr++;
	}

	/* exit mpr read mode */
	mv_ddr4_mpr_mode_disable(dev_num);

	/* decode mpr read value (only parallel mode supported) */
	if ((read_type == MV_DDR4_MPR_READ_DECODED) && (read_format == MV_DDR4_MPR_READ_PARALLEL)) {
		if (dq_map_enable == 1) {
			mv_ddr4_dq_decode(dev_num, data);
		} else {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_mpr_read: run mv_ddr4_dq_pins_mapping()\n"));
			return MV_FAIL;
		}
	}

	return MV_OK;
}

/* enter mpr write mode */
static int mv_ddr4_mpr_write_mode_enable(u8 dev_num, u32 mpr_location, u32 page_num, u32 data)
{
	/*
	 * enable MPR page 2 mpr mode in DDR4 MR3
	 * TODO: add support for cs, multicast or unicast, and if id
	 */
	int status;
	u32 if_id = 0, val = 0, mask;

	val = (page_num << 0) | (0x1 << 2);
	mask = (0x3 << 0) | (0x1 << 2);
	/* cs0 */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DDR4_MR3_REG, val, mask);
	if (status != MV_OK)
		return status;

	/* cs0 */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, DDR4_MPR_WR_REG,
				   (mpr_location << 8) | data, 0x3ff);
	if (status != MV_OK)
		return status;

	/* op cmd: cs0, cs1 are on, cs2, cs3 are off */
	status = ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST, if_id, SDRAM_OP_REG,
				   (0x13 | 0xc << 8) , (0x1f | (0xf << 8)));
	if (status != MV_OK)
		return status;

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id,  0, 0x1f, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_mpr_write_mode_enable: DDR3 poll failed(MPR3)\n"));
	}

	return MV_OK;
}

/* write mpr value */
int mv_ddr4_mpr_write(u8 dev_num, u32 mpr_location, u32 mpr_num, u32 page_num, u32 data)
{
	/* enter mpr write mode */
	mv_ddr4_mpr_write_mode_enable(dev_num, mpr_location, page_num, data);

	/* TODO: implement this function */

	/* TODO: exit mpr write mode */

	return MV_OK;
}

/*
 * map physical on-board connection of dram dq pins to ddr4 controller pins
 * note: supports only 32b width
 * TODO: add support for 64-bit bus width and ecc subphy
 */
int mv_ddr4_dq_pins_mapping(u8 dev_num)
{
	static int run_once;
	u8 dq_val[MAX_BUS_NUM][BUS_WIDTH_IN_BITS] = { {0} };
	u32 mpr_pattern[MV_DDR4_MPR_READ_PATTERN_NUM][EXT_ACCESS_BURST_LENGTH] = { {0} };
	u32 subphy_num, dq_num, mpr_type;
	u8 subphy_pattern[3];
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 subphy_max = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);

	if (run_once)
		return MV_OK;
	else
		run_once++;

	/* clear dq mapping */
	memset(dram_to_mc_dq_map, 0, sizeof(dram_to_mc_dq_map));

	/* stage 1: read page 0 mpr0..2 raw patterns */
	for (mpr_type = 0; mpr_type < MV_DDR4_MPR_READ_PATTERN_NUM; mpr_type++)
		mv_ddr4_mpr_read(dev_num, mpr_type, 0, MV_DDR4_MPR_READ_PARALLEL,
				 MV_DDR4_MPR_READ_RAW, mpr_pattern[mpr_type]);

	/* stage 2: map every dq for each subphy to 3-bit value, create local database */
	/* skip ecc supbhy; TODO: check to add support for ecc */
	if (subphy_max % 2)
		subphy_max -= 1;

	for (subphy_num = 0; subphy_num < subphy_max; subphy_num++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy_num);
		/* extract pattern for each subphy */
		for (mpr_type = 0; mpr_type < MV_DDR4_MPR_READ_PATTERN_NUM; mpr_type++)
			subphy_pattern[mpr_type] = ((mpr_pattern[mpr_type][2] >> (subphy_num * 8)) & 0xff);

		for (dq_num = 0; dq_num < BUS_WIDTH_IN_BITS; dq_num++)
			for (mpr_type = 0; mpr_type < MV_DDR4_MPR_READ_PATTERN_NUM; mpr_type++)
				dq_val[subphy_num][dq_num] += (((subphy_pattern[mpr_type] >> dq_num) & 1) *
							       (1 << mpr_type));
	}

	/* stage 3: map dram dq to mc dq and update database */
	for (subphy_num = 0; subphy_num < subphy_max; subphy_num++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy_num);
		for (dq_num = 0; dq_num < BUS_WIDTH_IN_BITS; dq_num++)
			dram_to_mc_dq_map[subphy_num][7 - dq_val[subphy_num][dq_num]] = dq_num;
	}

	/* set dq_map_enable */
	dq_map_enable = 1;

	return MV_OK;
}

/* enter to or exit from dram vref training mode */
int mv_ddr4_vref_training_mode_ctrl(u8 dev_num, u8 if_id, enum hws_access_type access_type, int enable)
{
	int status;
	u32 val, mask;

	/* DDR4 MR6 */
	/*
	 * set t_ccd_l, 0x1918[12:10] to 0x0, 0x2, or 0x4 (z1 supports only even
	 * values, to be fixed in a0)
	 * set vdq te, 0x1918[7] to 0x0
	 * set vdq tv, 0x1918[5:0] to vref training value
	 */

	val = (((enable == 1) ? 1 : 0) << 7);
	mask = (0x1 << 7);
	status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR6_REG, val, mask);
	if (status != MV_OK)
		return status;

	/* write DDR4 MR6 cs configuration; only cs0, cs1 supported */
	if (effective_cs == 0)
		val = 0xe;
	else
		val = 0xd;
	val <<= 8;
	/* write DDR4 MR6 command */
	val |= 0x12;
	mask = (0xf << 8) | 0x1f;
	status = ddr3_tip_if_write(dev_num, access_type, if_id, SDRAM_OP_REG, val, mask);
	if (status != MV_OK)
		return status;

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id,  0, 0x1f, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_vref_training_mode_ctrl: Polling command failed\n"));
	}

	return MV_OK;
}

/* set dram vref tap value */
int mv_ddr4_vref_tap_set(u8 dev_num, u8 if_id, enum hws_access_type access_type,
			 u32 taps_num, enum mv_ddr4_vref_tap_state state)
{
	int status;
	u32 range, vdq_tv;

	/* disable and then enable the training with a new range */
	if ((state == MV_DDR4_VREF_TAP_BUSY) && ((taps_num + MV_DDR4_VREF_STEP_SIZE) >= 23) &&
	    (taps_num < 23))
		state = MV_DDR4_VREF_TAP_FLIP;

	if (taps_num < 23) {
		range = 1;
		vdq_tv = taps_num;
	} else {
		range = 0;
		vdq_tv = taps_num - 23;
	}

	if ((state == MV_DDR4_VREF_TAP_FLIP) | (state == MV_DDR4_VREF_TAP_START)) {
		/* 0 to disable */
		status = mv_ddr4_vref_set(dev_num, if_id, access_type, range, vdq_tv, 0);
		if (status != MV_OK)
			return status;
		/* 1 to enable */
		status = (mv_ddr4_vref_set(dev_num, if_id, access_type, range, vdq_tv, 1));
		if (status != MV_OK)
			return status;
	} else if (state == MV_DDR4_VREF_TAP_END) {
		/* 1 to enable */
		status = (mv_ddr4_vref_set(dev_num, if_id, access_type, range, vdq_tv, 1));
		if (status != MV_OK)
			return status;
		/* 0 to disable */
		status = mv_ddr4_vref_set(dev_num, if_id, access_type, range, vdq_tv, 0);
		if (status != MV_OK)
			return status;
	} else {
		/* 1 to enable */
		status = (mv_ddr4_vref_set(dev_num, if_id, access_type, range, vdq_tv, 1));
		if (status != MV_OK)
			return status;
	}

	return MV_OK;
}

/* set dram vref value */
int mv_ddr4_vref_set(u8 dev_num, u8 if_id, enum hws_access_type access_type,
		     u32 range, u32 vdq_tv, u8 vdq_training_ena)
{
	int status;
	u32 read_data;
	u32 val, mask;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("mv_ddr4_vref_set: range %d, vdq_tv %d\n", range, vdq_tv));

	/* DDR4 MR6 */
	/*
	 * set t_ccd_l, 0x1918[12:10] to 0x0, 0x2, or 0x4 (z1 supports only even
	 * values, to be fixed in a0)
	 * set vdq te, 0x1918[7] to 0x0
	 * set vdq tr, 0x1918[6] to 0x0 to disable or 0x1 to enable
	 * set vdq tv, 0x1918[5:0] to vref training value
	 */
	val = (vdq_training_ena << 7) | (range << 6) | vdq_tv;
	mask = (0x0 << 7) | (0x1 << 6) | 0x3f;

	status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR6_REG, val, mask);
	if (status != MV_OK)
		return status;

	ddr3_tip_if_read(dev_num, access_type, if_id, DDR4_MR6_REG, &read_data, 0xffffffff);
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("mv_ddr4_vref_set: MR6 = 0x%x\n", read_data));

	/* write DDR4 MR6 cs configuration; only cs0, cs1 supported */
	if (effective_cs == 0)
		val = 0xe;
	else
		val = 0xd;
	val <<= 8;
	/* write DDR4 MR6 command */
	val |= 0x12;
	mask = (0xf << 8) | 0x1f;
	status = ddr3_tip_if_write(dev_num, access_type, if_id, SDRAM_OP_REG, val, mask);
	if (status != MV_OK)
		return status;

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id,  0, 0x1F, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_vref_set: Polling command failed\n"));
	}

	return MV_OK;
}

/* pda - load pattern to odpg */
int mv_ddr4_pda_pattern_odpg_load(u32 dev_num, enum hws_access_type access_type,
				  u32 if_id, u32 subphy_mask, u32 cs_num)
{
	int status;
	u32 pattern_len_count = 0;
	u32 data_low[KILLER_PATTERN_LENGTH] = {0};
	u32 data_high[KILLER_PATTERN_LENGTH] = {0};
	u32 val, mask, subphy_num;

	/*
	 * set 0x1630[10:5] bits to 0x3 (0x1 for 16-bit bus width)
	 * set 0x1630[14:11] bits to 0x3 (0x1 for 16-bit bus width)
	 */
	val = (cs_num << 26) | (0x1 << 25) | (0x3 << 11) | (0x3 << 5) | 0x1;
	mask = (0x3 << 26) | (0x1 << 25) | (0x3f << 11) | (0x3f << 5) | 0x1;
	status = ddr3_tip_if_write(dev_num, access_type, if_id, ODPG_DATA_CTRL_REG, val, mask);
	if (status != MV_OK)
		return status;

	if (subphy_mask != 0xf) {
		for (subphy_num = 0; subphy_num < 4; subphy_num++)
			if (((subphy_mask >> subphy_num) & 0x1) == 0)
				data_low[0] = (data_low[0] | (0xff << (subphy_num * 8)));
	} else
		data_low[0] = 0;

	for (pattern_len_count = 0; pattern_len_count < 4; pattern_len_count++) {
		data_low[pattern_len_count] = data_low[0];
		data_high[pattern_len_count] = data_low[0];
	}

	for (pattern_len_count = 0; pattern_len_count < 4 ; pattern_len_count++) {
		status = ddr3_tip_if_write(dev_num, access_type, if_id, ODPG_DATA_WR_DATA_LOW_REG,
					   data_low[pattern_len_count], MASK_ALL_BITS);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_if_write(dev_num, access_type, if_id, ODPG_DATA_WR_DATA_HIGH_REG,
					   data_high[pattern_len_count], MASK_ALL_BITS);
		if (status != MV_OK)
			return status;

		status = ddr3_tip_if_write(dev_num, access_type, if_id, ODPG_DATA_WR_ADDR_REG,
					   pattern_len_count, MASK_ALL_BITS);
		if (status != MV_OK)
			return status;
	}

	status = ddr3_tip_if_write(dev_num, access_type, if_id, ODPG_DATA_BUFFER_OFFS_REG,
				   0x0, MASK_ALL_BITS);
	if (status != MV_OK)
		return status;

	return MV_OK;
}

/* enable or disable pda */
int mv_ddr4_pda_ctrl(u8 dev_num, u8 if_id, u8 cs_num, int enable)
{
	/*
	 * if enable is 0, exit
	 * mrs to be directed to all dram devices
	 * a calling function responsible to change odpg to 0x0
	 */

	int status;
	enum hws_access_type access_type = ACCESS_TYPE_UNICAST;
	u32 val, mask;

	/* per dram addressability enable */
	val = ((enable == 1) ? 1 : 0);
	val <<= 4;
	mask = 0x1 << 4;
	status = ddr3_tip_if_write(dev_num, access_type, if_id, DDR4_MR3_REG, val, mask);
	if (status != MV_OK)
		return status;

	/* write DDR4 MR3 cs configuration; only cs0, cs1 supported */
	if (cs_num == 0)
		val = 0xe;
	else
		val = 0xd;
	val <<= 8;
	/* write DDR4 MR3 command */
	val |= 0x9;
	mask = (0xf << 8) | 0x1f;
	status = ddr3_tip_if_write(dev_num, access_type, if_id, SDRAM_OP_REG, val, mask);
	if (status != MV_OK)
		return status;

	if (enable == 0) {
		/* check odpg access is done */
		if (mv_ddr_is_odpg_done(MAX_POLLING_ITERATIONS) != MV_OK)
			return MV_FAIL;
	}

	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f, SDRAM_OP_REG,
				MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr4_pda_ctrl: Polling command failed\n"));

	return MV_OK;
}
#endif /* CONFIG_DDR4 */
