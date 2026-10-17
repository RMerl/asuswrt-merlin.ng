// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "mv_ddr_spd.h"

#define MV_DDR_SPD_DATA_MTB		125	/* medium timebase, ps */
#define MV_DDR_SPD_DATA_FTB		1	/* fine timebase, ps */
#define MV_DDR_SPD_MSB_OFFS		8	/* most significant byte offset, bits */

#define MV_DDR_SPD_SUPPORTED_CLS_NUM	30

static unsigned int mv_ddr_spd_supported_cls[MV_DDR_SPD_SUPPORTED_CLS_NUM];

int mv_ddr_spd_supported_cls_calc(union mv_ddr_spd_data *spd_data)
{
	unsigned int byte, bit, start_cl;

	start_cl = (spd_data->all_bytes[23] & 0x8) ? 23 : 7;

	for (byte = 20; byte < 23; byte++) {
		for (bit = 0; bit < 8; bit++) {
			if (spd_data->all_bytes[byte] & (1 << bit))
				mv_ddr_spd_supported_cls[(byte - 20) * 8 + bit] = start_cl + (byte - 20) * 8 + bit;
			else
				mv_ddr_spd_supported_cls[(byte - 20) * 8 + bit] = 0;
		}
	}

	for (byte = 23, bit = 0; bit < 6; bit++) {
		if (spd_data->all_bytes[byte] & (1 << bit))
			mv_ddr_spd_supported_cls[(byte - 20) * 8 + bit] = start_cl + (byte - 20) * 8 + bit;
		else
			mv_ddr_spd_supported_cls[(byte - 20) * 8 + bit] = 0;
	}

	return 0;
}

unsigned int mv_ddr_spd_supported_cl_get(unsigned int cl)
{
	unsigned int supported_cl;
	int i = 0;

	while (i < MV_DDR_SPD_SUPPORTED_CLS_NUM &&
		mv_ddr_spd_supported_cls[i] < cl)
		i++;

	if (i < MV_DDR_SPD_SUPPORTED_CLS_NUM)
		supported_cl = mv_ddr_spd_supported_cls[i];
	else
		supported_cl = 0;

	return supported_cl;
}

int mv_ddr_spd_timing_calc(union mv_ddr_spd_data *spd_data, unsigned int timing_data[])
{
	int calc_val;

	/* t ck avg min, ps */
	calc_val = spd_data->byte_fields.byte_18 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_125 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TCK_AVG_MIN] = calc_val;

	/* t aa min, ps */
	calc_val = spd_data->byte_fields.byte_24 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_123 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TAA_MIN] = calc_val;

	/* t rfc1 min, ps */
	timing_data[MV_DDR_TRFC1_MIN] = (spd_data->byte_fields.byte_30 +
		(spd_data->byte_fields.byte_31 << MV_DDR_SPD_MSB_OFFS)) * MV_DDR_SPD_DATA_MTB;

	/* t wr min, ps */
	timing_data[MV_DDR_TWR_MIN] = (spd_data->byte_fields.byte_42 +
		(spd_data->byte_fields.byte_41.bit_fields.t_wr_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB;

	/* t rcd min, ps */
	calc_val = spd_data->byte_fields.byte_25 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_122 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TRCD_MIN] = calc_val;

	/* t rp min, ps */
	calc_val = spd_data->byte_fields.byte_26 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_121 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TRP_MIN] = calc_val;

	/* t rc min, ps */
	calc_val = (spd_data->byte_fields.byte_29 +
		(spd_data->byte_fields.byte_27.bit_fields.t_rc_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_120 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TRC_MIN] = calc_val;

	/* t ras min, ps */
	timing_data[MV_DDR_TRAS_MIN] = (spd_data->byte_fields.byte_28 +
		(spd_data->byte_fields.byte_27.bit_fields.t_ras_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB;

	/* t rrd s min, ps */
	calc_val = spd_data->byte_fields.byte_38 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_119 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TRRD_S_MIN] = calc_val;

	/* t rrd l min, ps */
	calc_val = spd_data->byte_fields.byte_39 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_118 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TRRD_L_MIN] = calc_val;

	/* t ccd l min, ps */
	calc_val = spd_data->byte_fields.byte_40 * MV_DDR_SPD_DATA_MTB +
		(signed char)spd_data->byte_fields.byte_117 * MV_DDR_SPD_DATA_FTB;
	if (calc_val < 0)
		return 1;
	timing_data[MV_DDR_TCCD_L_MIN] = calc_val;

	/* t faw min, ps */
	timing_data[MV_DDR_TFAW_MIN] = (spd_data->byte_fields.byte_37 +
		(spd_data->byte_fields.byte_36.bit_fields.t_faw_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB;

	/* t wtr s min, ps */
	timing_data[MV_DDR_TWTR_S_MIN] = (spd_data->byte_fields.byte_44 +
		(spd_data->byte_fields.byte_43.bit_fields.t_wtr_s_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB;

	/* t wtr l min, ps */
	timing_data[MV_DDR_TWTR_L_MIN] = (spd_data->byte_fields.byte_45 +
		(spd_data->byte_fields.byte_43.bit_fields.t_wtr_l_min_msn << MV_DDR_SPD_MSB_OFFS)) *
		MV_DDR_SPD_DATA_MTB;

	return 0;
}

enum mv_ddr_dev_width mv_ddr_spd_dev_width_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char dev_width = spd_data->byte_fields.byte_12.bit_fields.device_width;
	enum mv_ddr_dev_width ret_val;

	switch (dev_width) {
	case 0x00:
		ret_val = MV_DDR_DEV_WIDTH_4BIT;
		break;
	case 0x01:
		ret_val = MV_DDR_DEV_WIDTH_8BIT;
		break;
	case 0x02:
		ret_val = MV_DDR_DEV_WIDTH_16BIT;
		break;
	case 0x03:
		ret_val = MV_DDR_DEV_WIDTH_32BIT;
		break;
	default:
		ret_val = MV_DDR_DEV_WIDTH_LAST;
	}

	return ret_val;
}

enum mv_ddr_die_capacity mv_ddr_spd_die_capacity_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char die_cap = spd_data->byte_fields.byte_4.bit_fields.die_capacity;
	enum mv_ddr_die_capacity ret_val;

	switch (die_cap) {
	case 0x00:
		ret_val = MV_DDR_DIE_CAP_256MBIT;
		break;
	case 0x01:
		ret_val = MV_DDR_DIE_CAP_512MBIT;
		break;
	case 0x02:
		ret_val = MV_DDR_DIE_CAP_1GBIT;
		break;
	case 0x03:
		ret_val = MV_DDR_DIE_CAP_2GBIT;
		break;
	case 0x04:
		ret_val = MV_DDR_DIE_CAP_4GBIT;
		break;
	case 0x05:
		ret_val = MV_DDR_DIE_CAP_8GBIT;
		break;
	case 0x06:
		ret_val = MV_DDR_DIE_CAP_16GBIT;
		break;
	case 0x07:
		ret_val = MV_DDR_DIE_CAP_32GBIT;
		break;
	case 0x08:
		ret_val = MV_DDR_DIE_CAP_12GBIT;
		break;
	case 0x09:
		ret_val = MV_DDR_DIE_CAP_24GBIT;
		break;
	default:
		ret_val = MV_DDR_DIE_CAP_LAST;
	}

	return ret_val;
}

unsigned char mv_ddr_spd_mem_mirror_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char mem_mirror = spd_data->byte_fields.byte_131.bit_fields.rank_1_mapping;

	return mem_mirror;
}

enum mv_ddr_pkg_rank mv_ddr_spd_pri_bus_width_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char pri_bus_width = spd_data->byte_fields.byte_13.bit_fields.primary_bus_width;
	enum mv_ddr_pri_bus_width ret_val;

	switch (pri_bus_width) {
	case 0x00:
		ret_val = MV_DDR_PRI_BUS_WIDTH_8;
		break;
	case 0x01:
		ret_val = MV_DDR_PRI_BUS_WIDTH_16;
		break;
	case 0x02:
		ret_val = MV_DDR_PRI_BUS_WIDTH_32;
		break;
	case 0x03:
		ret_val = MV_DDR_PRI_BUS_WIDTH_64;
		break;
	default:
		ret_val = MV_DDR_PRI_BUS_WIDTH_LAST;
	}

	return ret_val;
}

enum mv_ddr_pkg_rank mv_ddr_spd_bus_width_ext_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char bus_width_ext = spd_data->byte_fields.byte_13.bit_fields.bus_width_ext;
	enum mv_ddr_bus_width_ext ret_val;

	switch (bus_width_ext) {
	case 0x00:
		ret_val = MV_DDR_BUS_WIDTH_EXT_0;
		break;
	case 0x01:
		ret_val = MV_DDR_BUS_WIDTH_EXT_8;
		break;
	default:
		ret_val = MV_DDR_BUS_WIDTH_EXT_LAST;
	}

	return ret_val;
}

static enum mv_ddr_pkg_rank mv_ddr_spd_pkg_rank_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char pkg_rank = spd_data->byte_fields.byte_12.bit_fields.dimm_pkg_ranks_num;
	enum mv_ddr_pkg_rank ret_val;

	switch (pkg_rank) {
	case 0x00:
		ret_val = MV_DDR_PKG_RANK_1;
		break;
	case 0x01:
		ret_val = MV_DDR_PKG_RANK_2;
		break;
	case 0x02:
		ret_val = MV_DDR_PKG_RANK_3;
		break;
	case 0x03:
		ret_val = MV_DDR_PKG_RANK_4;
		break;
	case 0x04:
		ret_val = MV_DDR_PKG_RANK_5;
		break;
	case 0x05:
		ret_val = MV_DDR_PKG_RANK_6;
		break;
	case 0x06:
		ret_val = MV_DDR_PKG_RANK_7;
		break;
	case 0x07:
		ret_val = MV_DDR_PKG_RANK_8;
		break;
	default:
		ret_val = MV_DDR_PKG_RANK_LAST;
	}

	return ret_val;
}

static enum mv_ddr_die_count mv_ddr_spd_die_count_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char die_count = spd_data->byte_fields.byte_6.bit_fields.die_count;
	enum mv_ddr_die_count ret_val;

	switch (die_count) {
	case 0x00:
		ret_val = MV_DDR_DIE_CNT_1;
		break;
	case 0x01:
		ret_val = MV_DDR_DIE_CNT_2;
		break;
	case 0x02:
		ret_val = MV_DDR_DIE_CNT_3;
		break;
	case 0x03:
		ret_val = MV_DDR_DIE_CNT_4;
		break;
	case 0x04:
		ret_val = MV_DDR_DIE_CNT_5;
		break;
	case 0x05:
		ret_val = MV_DDR_DIE_CNT_6;
		break;
	case 0x06:
		ret_val = MV_DDR_DIE_CNT_7;
		break;
	case 0x07:
		ret_val = MV_DDR_DIE_CNT_8;
		break;
	default:
		ret_val = MV_DDR_DIE_CNT_LAST;
	}

	return ret_val;
}

unsigned char mv_ddr_spd_cs_bit_mask_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char cs_bit_mask = 0x0;
	enum mv_ddr_pkg_rank pkg_rank = mv_ddr_spd_pkg_rank_get(spd_data);
	enum mv_ddr_die_count die_cnt = mv_ddr_spd_die_count_get(spd_data);

	if (pkg_rank == MV_DDR_PKG_RANK_1 && die_cnt == MV_DDR_DIE_CNT_1)
		cs_bit_mask = 0x1;
	else if (pkg_rank == MV_DDR_PKG_RANK_1 && die_cnt == MV_DDR_DIE_CNT_2)
		cs_bit_mask = 0x3;
	else if (pkg_rank == MV_DDR_PKG_RANK_2 && die_cnt == MV_DDR_DIE_CNT_1)
		cs_bit_mask = 0x3;
	else if (pkg_rank == MV_DDR_PKG_RANK_2 && die_cnt == MV_DDR_DIE_CNT_2)
		cs_bit_mask = 0xf;

	return cs_bit_mask;
}

unsigned char mv_ddr_spd_dev_type_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char dev_type = spd_data->byte_fields.byte_2;

	return dev_type;
}

unsigned char mv_ddr_spd_module_type_get(union mv_ddr_spd_data *spd_data)
{
	unsigned char module_type = spd_data->byte_fields.byte_3.bit_fields.module_type;

	return module_type;
}
