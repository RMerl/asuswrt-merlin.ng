// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#if defined(CONFIG_DDR4)

/* DDR4 Training Database */

#include "ddr_ml_wrapper.h"

#include "mv_ddr_topology.h"
#include "mv_ddr_training_db.h"
#include "ddr_topology_def.h"

/* list of allowed frequencies listed in order of enum mv_ddr_freq */
static unsigned int freq_val[MV_DDR_FREQ_LAST] = {
	130,	/* MV_DDR_FREQ_LOW_FREQ */
	650,	/* MV_DDR_FREQ_650 */
	666,	/* MV_DDR_FREQ_667 */
	800,	/* MV_DDR_FREQ_800 */
	933,	/* MV_DDR_FREQ_933 */
	1066,	/* MV_DDR_FREQ_1066 */
	900,	/* MV_DDR_FREQ_900 */
	1000,	/* MV_DDR_FREQ_1000 */
	1050,	/* MV_DDR_FREQ_1050 */
	1200,	/* MV_DDR_FREQ_1200 */
	1333,	/* MV_DDR_FREQ_1333 */
	1466,	/* MV_DDR_FREQ_1466 */
	1600	/* MV_DDR_FREQ_1600 */
};

unsigned int *mv_ddr_freq_tbl_get(void)
{
	return &freq_val[0];
}

u32 mv_ddr_freq_get(enum mv_ddr_freq freq)
{
	return freq_val[freq];
}

/* non-dbi mode - table for cl values per frequency for each speed bin index */
static struct mv_ddr_cl_val_per_freq cl_table[] = {
/*   130   650   667   800   933   1067   900   1000   1050   1200   1333   1466   1600 FREQ(MHz)*/
/*   7.69  1.53  1.5   1.25  1.07  0.937  1.11	1	   0.95   0.83	 0.75	0.68   0.625 TCK(ns)*/
	{{10,  10,	 10,   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_1600J */
	{{10,  11,	 11,   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600K */
	{{10,  12,	 12,   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_1600L */
	{{10,  12,	 12,   12,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_1866L */
	{{10,  12,	 12,   13,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_1866M */
	{{10,  12,	 12,   14,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_1866N */
	{{10,  10,	 10,   12,	 14,   14,	  14,	14,	   14,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2133N */
	{{10,  9,	 9,    12,	 14,   15,	  14,	15,	   15,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2133P */
	{{10,  10,	 10,   12,	 14,   16,	  14,	16,	   16,	  0,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2133R */
	{{10,  10,	 10,   12,	 14,   16,	  14,	16,	   16,	  18,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2400P */
	{{10,  9,	 9,    11,	 13,   15,	  13,	15,	   15,	  18,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2400R */
	{{10,  9,	 9,    11,	 13,   15,	  13,	15,	   15,	  17,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2400T */
	{{10,  10,	 10,   12,	 14,   16,	  14,	16,	   16,	  18,	 0,	    0,	   0} },/* SPEED_BIN_DDR_2400U */
	{{10,  10,   10,   11,   13,   15,    13,   15,    15,    16,    17,    0,     0} },/* SPEED_BIN_DDR_2666T */
	{{10,  9,    10,   11,   13,   15,    13,   15,    15,    17,    18,    0,     0} },/* SPEED_BIN_DDR_2666U */
	{{10,  9,    10,   12,   14,   16,    14,   16,    16,    18,    19,    0,     0} },/* SPEED_BIN_DDR_2666V */
	{{10,  10,   10,   12,   14,   16,    14,   16,    16,    18,    20,    0,     0} },/* SPEED_BIN_DDR_2666W */
	{{10,  10,   9,    11,   13,   15,    13,   15,    15,    16,    18,    19,    0} },/* SPEED_BIN_DDR_2933V */
	{{10,  9,    10,   11,   13,   15,    13,   15,    15,    17,    19,    20,    0} },/* SPEED_BIN_DDR_2933W */
	{{10,  9,    10,   12,   14,   16,    14,   16,    16,    18,    20,    21,    0} },/* SPEED_BIN_DDR_2933Y */
	{{10,  10,   10,   12,   14,   16,    14,   16,    16,    18,    20,    22,    0} },/* SPEED_BIN_DDR_2933AA*/
	{{10,  10,   9,    11,   13,   15,    13,   15,    15,    16,    18,    20,    20} },/* SPEED_BIN_DDR_3200W */
	{{10,  9,    0,    11,   13,   15,    13,   15,    15,    17,    19,    22,    22} },/* SPEED_BIN_DDR_3200AA*/
	{{10,  9,    10,   12,   14,   16,    14,   16,    16,    18,    20,    24,    24} } /* SPEED_BIN_DDR_3200AC*/

};

u32 mv_ddr_cl_val_get(u32 index, u32 freq)
{
	return cl_table[index].cl_val[freq];
}

/* dbi mode - table for cl values per frequency for each speed bin index */
struct mv_ddr_cl_val_per_freq cas_latency_table_dbi[] = {
/*	 130   650   667   800   933   1067   900   1000   1050   1200   1333   1466   1600 FREQ(MHz)*/
/*	 7.69  1.53  1.5   1.25  1.07  0.937  1.11  1      0.95   0.83   0.75   0.68   0.625 TCK(ns)*/
	{{0,   12,	 12,   0,	 0,	   0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600J */
	{{0,   13,	 13,   0,	 0,	   0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600K */
	{{0,   14,	 14,   0,	 0,	   0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600L */
	{{0,   14,	 14,   14,	 0,	   0,	  14,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866L */
	{{0,   14,	 14,   15,	 0,	   0,	  15,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866M */
	{{0,   14,	 14,   16,	 0,	   0,	  16,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866N */
	{{0,   12,	 12,   14,	 16,	  17,	  14,	17,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133N */
	{{0,   11,	 11,   14,	 16,	  18,	  14,	18,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133P */
	{{0,   12,	 12,   14,	 16,	  19,	  14,	19,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133R */
	{{0,   12,	 12,   14,	 16,	  19,	  14,	19,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400P */
	{{0,   11,	 11,   13,	 15,	  18,	  13,	18,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400R */
	{{0,   11,	 11,   13,	 15,	  18,	  13,	18,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400T */
	{{0,   12,	 12,   14,	 16,	  19,	  14,	19,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400U */
	{{10,  10,   11,   13,   15,   18,    13,   18,    18,    19,    20,    0,     0} },/* SPEED_BIN_DDR_2666T */
	{{10,  9,    11,   13,   15,   18,    13,   18,    18,    20,    21,    0,     0} },/* SPEED_BIN_DDR_2666U */
	{{10,  9,    12,   14,   16,   19,    14,   19,    19,    21,    22,    0,     0} },/* SPEED_BIN_DDR_2666V */
	{{10,  10,   12,   14,   16,   19,    14,   19,    19,    21,    23,    0,     0} },/* SPEED_BIN_DDR_2666W */
	{{10,  10,   11,   13,   15,   18,    15,   18,    18,    19,    21,    23,    0} },/* SPEED_BIN_DDR_2933V */
	{{10,  9,    12,   13,   15,   18,    15,   18,    18,    20,    22,    24,    0} },/* SPEED_BIN_DDR_2933W */
	{{10,  9,    12,   14,   16,   19,    16,   19,    19,    21,    23,    26,    0} },/* SPEED_BIN_DDR_2933Y */
	{{10,  10,   12,   14,   16,   19,    16,   19,    19,    21,    23,    26,    0} },/* SPEED_BIN_DDR_2933AA*/
	{{10,  10,   11,   13,   15,   18,    15,   18,    18,    19,    21,    24,    24} },/* SPEED_BIN_DDR_3200W */
	{{10,  9,    0,    13,   15,   18,    15,   18,    18,    20,    22,    26,    26} },/* SPEED_BIN_DDR_3200AA*/
	{{10,  9,    12,   14,   16,   19,    16,   19,    19,    21,    23,    28,    28} } /* SPEED_BIN_DDR_3200AC*/
};

/* table for cwl values per speed bin index */
static struct mv_ddr_cl_val_per_freq cwl_table[] = {
/*	 130   650   667   800   933   1067   900   1000   1050   1200   1333  1466   1600 FREQ(MHz)*/
/*	7.69   1.53  1.5   1.25  1.07  0.937  1.11  1      0.95   0.83   0.75  0.68   0.625 TCK(ns)*/
	{{9,   9,	 9,	   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600J */
	{{9,   9,	 9,	   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600K */
	{{9,   9,	 9,	   0,	 0,    0,	  0,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1600L */
	{{9,   9,	 9,	   10,	 0,    0,	  10,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866L */
	{{9,   9,	 9,	   10,	 0,    0,	  10,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866M */
	{{9,   9,	 9,	   10,	 0,    0,	  10,	0,	   0,	  0,	 0,	    0,     0} },/* SPEED_BIN_DDR_1866N */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  11,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133N */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  11,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133P */
	{{9,   9,	 9,	   10,	 10,   11,	  10,	11,	   10,	  11,	 0,	    0,     0} },/* SPEED_BIN_DDR_2133R */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  12,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400P */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  12,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400R */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  12,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400T */
	{{9,   9,	 9,	   9,	 10,   11,	  10,	11,	   10,	  12,	 0,	    0,     0} },/* SPEED_BIN_DDR_2400U */
	{{10,  10,   9,    9,    10,   11,    10,   11,    11,    12,    14,    0,     0} },/* SPEED_BIN_DDR_2666T */
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    0,     0} },/* SPEED_BIN_DDR_2666U */
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    0,     0} },/* SPEED_BIN_DDR_2666V */
	{{10,  10,   9,    9,    10,   11,    10,   11,    11,    12,    14,    0,     0} },/* SPEED_BIN_DDR_2666W */
	{{10,  10,   9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    0} },/* SPEED_BIN_DDR_2933V */
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    0} },/* SPEED_BIN_DDR_2933W */
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    0} },/* SPEED_BIN_DDR_2933Y */
	{{10,  10,   9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    0} },/* SPEED_BIN_DDR_2933AA*/
	{{10,  10,   9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    16} },/* SPEED_BIN_DDR_3200W */
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    16} },/* SPEED_BIN_DDR_3200AA*/
	{{10,  9,    9,    9,    10,   11,    10,   11,    11,    12,    14,    16,    16} } /* SPEED_BIN_DDR_3200AC*/
};

u32 mv_ddr_cwl_val_get(u32 index, u32 freq)
{
	return cwl_table[index].cl_val[freq];
}

/*
 * rfc values, ns
 * note: values per JEDEC speed bin 1866; TODO: check it
 */
static unsigned int rfc_table[] = {
	0,	/* placholder */
	0,	/* placholder */
	160,	/* 2G */
	260,	/* 4G */
	350,	/* 8G */
	0,	/* TODO: placeholder for 16-Mbit die capacity */
	0,	/* TODO: placeholder for 32-Mbit die capacity*/
	0,	/* TODO: placeholder for 12-Mbit die capacity */
	0	/* TODO: placeholder for 24-Mbit die capacity */
};

u32 mv_ddr_rfc_get(u32 mem)
{
	return rfc_table[mem];
}

u16 rtt_table[] = {
	0xffff,
	60,
	120,
	40,
	240,
	48,
	80,
	34
};

u8 twr_mask_table[] = {
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0xa,
	0x0,	/* 10 */
	0xa,
	0x1,	/* 12 */
	0xa,
	0x2,	/* 14 */
	0xa,
	0x3,	/* 16 */
	0xa,
	0x4,	/* 18 */
	0xa,
	0x5,	/* 20 */
	0xa,
	0xa,	/* 22 */
	0xa,
	0x6	/* 24 */
};

u8 cl_mask_table[] = {
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x1,	/* 10 */
	0x2,
	0x3,	/* 12 */
	0x4,
	0x5,	/* 14 */
	0x6,
	0x7,	/* 16 */
	0xd,
	0x8,	/* 18 */
	0x0,
	0x9,	/* 20 */
	0x0,
	0xa,	/* 22 */
	0x0,
	0xb	/* 24 */
};

u8 cwl_mask_table[] = {
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x1,	/* 10 */
	0x2,
	0x3,	/* 12 */
	0x0,
	0x4,	/* 14 */
	0x0,
	0x5,	/* 16 */
	0x0,
	0x6	/* 18 */
};

u32 speed_bin_table_t_rcd_t_rp[] = {
	12500,
	13750,
	15000,
	12850,
	13920,
	15000,
	13130,
	14060,
	15000,
	12500,
	13320,
	14160,
	15000,
	12750,
	13500,
	14250,
	15000,
	12960,
	13640,
	14320,
	15000,
	12500,
	13750,
	15000
};

u32 speed_bin_table_t_rc[] = {
	47500,
	48750,
	50000,
	46850,
	47920,
	49000,
	46130,
	47060,
	48000,
	44500,
	45320,
	46160,
	47000,
	44750,
	45500,
	46250,
	47000,
	44960,
	45640,
	46320,
	47000,
	44500,
	45750,
	47000
};

static struct mv_ddr_page_element page_tbl[] = {
	/* 8-bit, 16-bit page size */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 512M */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 1G */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 2G */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 4G */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 8G */
	{0, 0}, /* TODO: placeholder for 16-Mbit die capacity */
	{0, 0}, /* TODO: placeholder for 32-Mbit die capacity */
	{0, 0}, /* TODO: placeholder for 12-Mbit die capacity */
	{0, 0}  /* TODO: placeholder for 24-Mbit die capacity */
};

u32 mv_ddr_page_size_get(enum mv_ddr_dev_width bus_width, enum mv_ddr_die_capacity mem_size)
{
	if (bus_width == MV_DDR_DEV_WIDTH_8BIT)
		return page_tbl[mem_size].page_size_8bit;
	else
		return page_tbl[mem_size].page_size_16bit;
}

/* DLL locking time, tDLLK */
#define MV_DDR_TDLLK_DDR4_1600	597
#define MV_DDR_TDLLK_DDR4_1866	597
#define MV_DDR_TDLLK_DDR4_2133	768
#define MV_DDR_TDLLK_DDR4_2400	768
#define MV_DDR_TDLLK_DDR4_2666	854
#define MV_DDR_TDLLK_DDR4_2933	940
#define MV_DDR_TDLLK_DDR4_3200	1024
static int mv_ddr_tdllk_get(unsigned int freq, unsigned int *tdllk)
{
	if (freq >= 1600)
		*tdllk = MV_DDR_TDLLK_DDR4_3200;
	else if (freq >= 1466)
		*tdllk = MV_DDR_TDLLK_DDR4_2933;
	else if (freq >= 1333)
		*tdllk = MV_DDR_TDLLK_DDR4_2666;
	else if (freq >= 1200)
		*tdllk = MV_DDR_TDLLK_DDR4_2400;
	else if (freq >= 1066)
		*tdllk = MV_DDR_TDLLK_DDR4_2133;
	else if (freq >= 933)
		*tdllk = MV_DDR_TDLLK_DDR4_1866;
	else if (freq >= 800)
		*tdllk = MV_DDR_TDLLK_DDR4_1600;
	else {
		printf("error: %s: unsupported data rate found\n", __func__);
		return -1;
	}

	return 0;
}

/* return speed bin value for selected index and element */
unsigned int mv_ddr_speed_bin_timing_get(enum mv_ddr_speed_bin index, enum mv_ddr_speed_bin_timing element)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int freq;
	u32 result = 0;

	/* get frequency in MHz */
	freq = mv_ddr_freq_get(tm->interface_params[0].memory_freq);

	switch (element) {
	case SPEED_BIN_TRCD:
	case SPEED_BIN_TRP:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRCD_MIN];
		else
			result = speed_bin_table_t_rcd_t_rp[index];
		break;
	case SPEED_BIN_TRAS:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRAS_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 35000;
			else if (index <= SPEED_BIN_DDR_1866N)
				result = 34000;
			else if (index <= SPEED_BIN_DDR_2133R)
				result = 33000;
			else
				result = 32000;
		}
		break;
	case SPEED_BIN_TRC:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRC_MIN];
		else
			result = speed_bin_table_t_rc[index];
		break;
	case SPEED_BIN_TRRD0_5K:
	case SPEED_BIN_TRRD1K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRRD_S_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 5000;
			else if (index <= SPEED_BIN_DDR_1866N)
				result = 4200;
			else if (index <= SPEED_BIN_DDR_2133R)
				result = 3700;
			else if (index <= SPEED_BIN_DDR_2400U)
				result = 3500;
			else if (index <= SPEED_BIN_DDR_2666W)
				result = 3000;
			else if (index <= SPEED_BIN_DDR_2933AA)
				result = 2700;
			else
				result = 2500;
		}
	        break;
	case SPEED_BIN_TRRD2K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRRD_S_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 6000;
			else
				result = 5300;
		}

		break;
	case SPEED_BIN_TRRDL0_5K:
	case SPEED_BIN_TRRDL1K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRRD_L_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 6000;
			else if (index <= SPEED_BIN_DDR_2133R)
				result = 5300;
			else
				result = 4900;
		}
		break;
	case SPEED_BIN_TRRDL2K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TRRD_L_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 7500;
			else
				result = 6400;
		}
	        break;
	case SPEED_BIN_TPD:
		result = 5000;
		break;
	case SPEED_BIN_TFAW0_5K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TFAW_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 20000;
			else if (index <= SPEED_BIN_DDR_1866N)
				result = 17000;
			else if (index <= SPEED_BIN_DDR_2133R)
				result = 15000;
			else if (index <= SPEED_BIN_DDR_2400U)
				result = 13000;
			else if (index <= SPEED_BIN_DDR_2666W)
				result = 12000;
			else if (index <= SPEED_BIN_DDR_2933AA)
				result = 10875;
			else
				result = 10000;
		}
	        break;
	case SPEED_BIN_TFAW1K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TFAW_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 25000;
			else if (index <= SPEED_BIN_DDR_1866N)
				result = 23000;
			else
				result = 21000;
		}
	        break;
	case SPEED_BIN_TFAW2K:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TFAW_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 35000;
			else
				result = 30000;
		}
		break;
	case SPEED_BIN_TWTR:
		result = 2500;
		/* FIXME: wa: set twtr_s to a default value, if it's unset on spd */
		if (tm->cfg_src == MV_DDR_CFG_SPD && tm->timing_data[MV_DDR_TWTR_S_MIN])
			result = tm->timing_data[MV_DDR_TWTR_S_MIN];
		break;
	case SPEED_BIN_TWTRL:
	case SPEED_BIN_TRTP:
		result = 7500;
		/* FIXME: wa: set twtr_l to a default value, if it's unset on spd */
		if (tm->cfg_src == MV_DDR_CFG_SPD && tm->timing_data[MV_DDR_TWTR_L_MIN])
			result = tm->timing_data[MV_DDR_TWTR_L_MIN];
		break;
	case SPEED_BIN_TWR:
	case SPEED_BIN_TMOD:
		result = 15000;
		/* FIXME: wa: set twr to a default value, if it's unset on spd */
		if (tm->cfg_src == MV_DDR_CFG_SPD && tm->timing_data[MV_DDR_TWR_MIN])
			result = tm->timing_data[MV_DDR_TWR_MIN];
		break;
	case SPEED_BIN_TXPDLL:
		result = 24000;
		break;
	case SPEED_BIN_TXSDLL:
		if (mv_ddr_tdllk_get(freq, &result))
			result = 0;
		break;
	case SPEED_BIN_TCCDL:
		if (tm->cfg_src == MV_DDR_CFG_SPD)
			result = tm->timing_data[MV_DDR_TCCD_L_MIN];
		else {
			if (index <= SPEED_BIN_DDR_1600L)
				result = 6250;
			else if (index <= SPEED_BIN_DDR_2133R)
				result = 5355;
			else
				result = 5000;
		}
		break;
	default:
		printf("error: %s: invalid element [%d] found\n", __func__, (int)element);
		break;
	}

	return result;
}
#endif /* CONFIG_DDR4 */
