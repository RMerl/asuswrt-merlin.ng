// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr_ml_wrapper.h"

#include "ddr3_training_ip_flow.h"
#include "mv_ddr_topology.h"
#include "mv_ddr_training_db.h"
#include "ddr3_training_ip_db.h"

/* Device attributes structures */
enum mv_ddr_dev_attribute ddr_dev_attributes[MV_ATTR_LAST];
int ddr_dev_attr_init_done = 0;

static inline u32 pattern_table_get_killer_word16(u8 dqs, u8 index);
static inline u32 pattern_table_get_sso_word(u8 sso, u8 index);
static inline u32 pattern_table_get_vref_word(u8 index);
static inline u32 pattern_table_get_vref_word16(u8 index);
static inline u32 pattern_table_get_sso_full_xtalk_word(u8 bit, u8 index);
static inline u32 pattern_table_get_sso_full_xtalk_word16(u8 bit, u8 index);
static inline u32 pattern_table_get_sso_xtalk_free_word(u8 bit, u8 index);
static inline u32 pattern_table_get_sso_xtalk_free_word16(u8 bit, u8 index);
static inline u32 pattern_table_get_isi_word(u8 index);
static inline u32 pattern_table_get_isi_word16(u8 index);

/* List of allowed frequency listed in order of enum mv_ddr_freq */
static unsigned int freq_val[MV_DDR_FREQ_LAST] = {
	0,			/*MV_DDR_FREQ_LOW_FREQ */
	400,			/*MV_DDR_FREQ_400, */
	533,			/*MV_DDR_FREQ_533, */
	666,			/*MV_DDR_FREQ_667, */
	800,			/*MV_DDR_FREQ_800, */
	933,			/*MV_DDR_FREQ_933, */
	1066,			/*MV_DDR_FREQ_1066, */
	311,			/*MV_DDR_FREQ_311, */
	333,			/*MV_DDR_FREQ_333, */
	467,			/*MV_DDR_FREQ_467, */
	850,			/*MV_DDR_FREQ_850, */
	600,			/*MV_DDR_FREQ_600 */
	300,			/*MV_DDR_FREQ_300 */
	900,			/*MV_DDR_FREQ_900 */
	360,			/*MV_DDR_FREQ_360 */
	1000			/*MV_DDR_FREQ_1000 */
};

unsigned int *mv_ddr_freq_tbl_get(void)
{
	return &freq_val[0];
}

u32 mv_ddr_freq_get(enum mv_ddr_freq freq)
{
	return freq_val[freq];
}

/* cas latency values per frequency for each speed bin index */
static struct mv_ddr_cl_val_per_freq cl_table[] = {
	/*
	 * 400M   667M     933M   311M     467M  600M    360
	 * 100M    533M    800M    1066M   333M    850M      900
	 * 1000 (the order is 100, 400, 533 etc.)
	 */
	/* DDR3-800D */
	{ {6, 5, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 0, 5, 0} },
	/* DDR3-800E */
	{ {6, 6, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 6, 0, 6, 0} },
	/* DDR3-1066E */
	{ {6, 5, 6, 0, 0, 0, 0, 5, 5, 6, 0, 0, 5, 0, 5, 0} },
	/* DDR3-1066F */
	{ {6, 6, 7, 0, 0, 0, 0, 6, 6, 7, 0, 0, 6, 0, 6, 0} },
	/* DDR3-1066G */
	{ {6, 6, 8, 0, 0, 0, 0, 6, 6, 8, 0, 0, 6, 0, 6, 0} },
	/* DDR3-1333F* */
	{ {6, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1333G */
	{ {6, 5, 7, 8, 0, 0, 0, 5, 5, 7, 0, 8, 5, 0, 5, 0} },
	/* DDR3-1333H */
	{ {6, 6, 8, 9, 0, 0, 0, 6, 6, 8, 0, 9, 6, 0, 6, 0} },
	/* DDR3-1333J* */
	{ {6, 6, 8, 10, 0, 0, 0, 6, 6, 8, 0, 10, 6, 0, 6,  0}
	 /* DDR3-1600G* */},
	{ {6, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600H */
	{ {6, 5, 6, 8, 9, 0, 0, 5, 5, 6, 0, 8, 5, 0, 5, 0} },
	/* DDR3-1600J */
	{ {6, 5, 7, 9, 10, 0, 0, 5, 5, 7, 0, 9, 5, 0, 5, 0} },
	/* DDR3-1600K */
	{ {6, 6, 8, 10, 11, 0, 0, 6, 6, 8, 0, 10, 6, 0, 6, 0 } },
	/* DDR3-1866J* */
	{ {6, 5, 6, 8, 9, 11, 0, 5, 5, 6, 11, 8, 5, 0, 5, 0} },
	/* DDR3-1866K */
	{ {6, 5, 7, 8, 10, 11, 0, 5, 5, 7, 11, 8, 5, 11, 5, 11} },
	/* DDR3-1866L */
	{ {6, 6, 7, 9, 11, 12, 0, 6, 6, 7, 12, 9, 6, 12, 6, 12} },
	/* DDR3-1866M* */
	{ {6, 6, 8, 10, 11, 13, 0, 6, 6, 8, 13, 10, 6, 13, 6, 13} },
	/* DDR3-2133K* */
	{ {6, 5, 6, 7, 9, 10, 11, 5, 5, 6, 10, 7, 5, 11, 5, 11} },
	/* DDR3-2133L */
	{ {6, 5, 6, 8, 9, 11, 12, 5, 5, 6, 11, 8, 5, 12, 5, 12} },
	/* DDR3-2133M */
	{ {6, 5, 7, 9, 10, 12, 13, 5, 5, 7, 12, 9, 5, 13, 5, 13} },
	/* DDR3-2133N* */
	{ {6, 6, 7, 9, 11, 13, 14, 6, 6, 7, 13, 9, 6, 14,  6, 14} },
	/* DDR3-1333H-ext */
	{ {6, 6, 7, 9, 0, 0, 0, 6, 6, 7, 0, 9, 6, 0, 6, 0} },
	/* DDR3-1600K-ext */
	{ {6, 6, 7, 9, 11, 0, 0, 6, 6, 7, 0, 9, 6, 0, 6, 0} },
	/* DDR3-1866M-ext */
	{ {6, 6, 7, 9, 11, 13, 0, 6, 6, 7, 13, 9, 6, 13, 6, 13} },
};

u32 mv_ddr_cl_val_get(u32 index, u32 freq)
{
	return cl_table[index].cl_val[freq];
}

/* cas write latency values per frequency for each speed bin index */
static struct mv_ddr_cl_val_per_freq cwl_table[] = {
	/*
	 * 400M   667M     933M   311M     467M  600M    360
	 * 100M    533M    800M    1066M   333M    850M      900
	 * (the order is 100, 400, 533 etc.)
	 */
	/* DDR3-800D  */
	{ {5, 5, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 0, 5, 0} },
	/* DDR3-800E  */
	{ {5, 5, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 0, 5, 0} },
	/* DDR3-1066E  */
	{ {5, 5, 6, 0, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1066F  */
	{ {5, 5, 6, 0, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1066G  */
	{ {5, 5, 6, 0, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1333F*  */
	{ {5, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1333G  */
	{ {5, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1333H  */
	{ {5, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1333J*  */
	{ {5, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600G*  */
	{ {5, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600H  */
	{ {5, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600J  */
	{ {5, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600K  */
	{ {5, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1866J*  */
	{ {5, 5, 6, 7, 8, 9, 0, 5, 5, 6, 9, 7, 5, 0, 5, 0} },
	/* DDR3-1866K  */
	{ {5, 5, 6, 7, 8, 9, 0, 5, 5, 6, 9, 7, 5, 0, 5, 0} },
	/* DDR3-1866L  */
	{ {5, 5, 6, 7, 8, 9, 0, 5, 5, 6, 9, 7, 5, 9, 5, 9} },
	/* DDR3-1866M*   */
	{ {5, 5, 6, 7, 8, 9, 0, 5, 5, 6, 9, 7, 5, 9, 5, 9} },
	/* DDR3-2133K*  */
	{ {5, 5, 6, 7, 8, 9, 10, 5, 5, 6, 9, 7, 5, 9, 5, 10} },
	/* DDR3-2133L  */
	{ {5, 5, 6, 7, 8, 9, 10, 5, 5, 6, 9, 7, 5, 9, 5, 10} },
	/* DDR3-2133M  */
	{ {5, 5, 6, 7, 8, 9, 10, 5, 5, 6, 9, 7, 5, 9, 5, 10} },
	/* DDR3-2133N*  */
	{ {5, 5, 6, 7, 8, 9, 10, 5, 5, 6, 9, 7, 5, 9, 5, 10} },
	/* DDR3-1333H-ext  */
	{ {5, 5, 6, 7, 0, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1600K-ext  */
	{ {5, 5, 6, 7, 8, 0, 0, 5, 5, 6, 0, 7, 5, 0, 5, 0} },
	/* DDR3-1866M-ext  */
	{ {5, 5, 6, 7, 8, 9, 0, 5, 5, 6, 9, 7, 5, 9, 5, 9} },
};

u32 mv_ddr_cwl_val_get(u32 index, u32 freq)
{
	return cwl_table[index].cl_val[freq];
}

u8 twr_mask_table[] = {
	10,
	10,
	10,
	10,
	10,
	1,			/* 5 */
	2,			/* 6 */
	3,			/* 7 */
	4,			/* 8 */
	10,
	5,			/* 10 */
	10,
	6,			/* 12 */
	10,
	7,			/* 14 */
	10,
	0			/* 16 */
};

u8 cl_mask_table[] = {
	0,
	0,
	0,
	0,
	0,
	0x2,
	0x4,
	0x6,
	0x8,
	0xa,
	0xc,
	0xe,
	0x1,
	0x3,
	0x5,
	0x5
};

u8 cwl_mask_table[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0x1,
	0x2,
	0x3,
	0x4,
	0x5,
	0x6,
	0x7,
	0x8,
	0x9,
	0x9
};

/* RFC values (in ns) */
static unsigned int rfc_table[] = {
	90,	/* 512M */
	110,	/* 1G */
	160,	/* 2G */
	260,	/* 4G */
	350,	/* 8G */
	0,	/* TODO: placeholder for 16-Mbit dev width */
	0,	/* TODO: placeholder for 32-Mbit dev width */
	0,	/* TODO: placeholder for 12-Mbit dev width */
	0	/* TODO: placeholder for 24-Mbit dev width */
};

u32 mv_ddr_rfc_get(u32 mem)
{
	return rfc_table[mem];
}

u32 speed_bin_table_t_rc[] = {
	50000,
	52500,
	48750,
	50625,
	52500,
	46500,
	48000,
	49500,
	51000,
	45000,
	46250,
	47500,
	48750,
	44700,
	45770,
	46840,
	47910,
	43285,
	44220,
	45155,
	46090
};

u32 speed_bin_table_t_rcd_t_rp[] = {
	12500,
	15000,
	11250,
	13125,
	15000,
	10500,
	12000,
	13500,
	15000,
	10000,
	11250,
	12500,
	13750,
	10700,
	11770,
	12840,
	13910,
	10285,
	11220,
	12155,
	13090,
};

enum {
	PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_AGGRESSOR = 0,
	PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM
};

static u8 pattern_killer_pattern_table_map[KILLER_PATTERN_LENGTH * 2][2] = {
	/*Aggressor / Victim */
	{1, 0},
	{0, 0},
	{1, 0},
	{1, 1},
	{0, 1},
	{0, 1},
	{1, 0},
	{0, 1},
	{1, 0},
	{0, 1},
	{1, 0},
	{1, 0},
	{0, 1},
	{1, 0},
	{0, 1},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{1, 0},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 1},
	{0, 1},
	{1, 1},
	{0, 0},
	{0, 0},
	{1, 1},
	{1, 1},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{1, 1},
	{0, 0},
	{0, 0},
	{1, 1},
	{0, 0},
	{1, 1},
	{0, 1},
	{0, 0},
	{0, 1},
	{0, 1},
	{0, 0},
	{1, 1},
	{1, 1},
	{1, 0},
	{1, 0},
	{1, 1},
	{1, 1},
	{1, 1},
	{1, 1},
	{1, 1},
	{1, 1},
	{1, 1}
};

static u8 pattern_vref_pattern_table_map[] = {
	/* 1 means 0xffffffff, 0 is 0x0 */
	0xb8,
	0x52,
	0x55,
	0x8a,
	0x33,
	0xa6,
	0x6d,
	0xfe
};

static struct mv_ddr_page_element page_tbl[] = {
	/* 8-bit, 16-bit page size */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 512M */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 1G */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 2G */
	{MV_DDR_PAGE_SIZE_1K, MV_DDR_PAGE_SIZE_2K}, /* 4G */
	{MV_DDR_PAGE_SIZE_2K, MV_DDR_PAGE_SIZE_2K}, /* 8G */
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

/* Return speed Bin value for selected index and t* element */
unsigned int mv_ddr_speed_bin_timing_get(enum mv_ddr_speed_bin index, enum mv_ddr_speed_bin_timing element)
{
	u32 result = 0;

	switch (element) {
	case SPEED_BIN_TRCD:
	case SPEED_BIN_TRP:
		result = speed_bin_table_t_rcd_t_rp[index];
		break;
	case SPEED_BIN_TRAS:
		if (index <= SPEED_BIN_DDR_1066G)
			result = 37500;
		else if (index <= SPEED_BIN_DDR_1333J)
			result = 36000;
		else if (index <= SPEED_BIN_DDR_1600K)
			result = 35000;
		else if (index <= SPEED_BIN_DDR_1866M)
			result = 34000;
		else
			result = 33000;
		break;
	case SPEED_BIN_TRC:
		result = speed_bin_table_t_rc[index];
		break;
	case SPEED_BIN_TRRD1K:
		if (index <= SPEED_BIN_DDR_800E)
			result = 10000;
		else if (index <= SPEED_BIN_DDR_1066G)
			result = 7500;
		else if (index <= SPEED_BIN_DDR_1600K)
			result = 6000;
		else
			result = 5000;
		break;
	case SPEED_BIN_TRRD2K:
		if (index <= SPEED_BIN_DDR_1066G)
			result = 10000;
		else if (index <= SPEED_BIN_DDR_1600K)
			result = 7500;
		else
			result = 6000;
		break;
	case SPEED_BIN_TPD:
		if (index < SPEED_BIN_DDR_800E)
			result = 7500;
		else if (index < SPEED_BIN_DDR_1333J)
			result = 5625;
		else
			result = 5000;
		break;
	case SPEED_BIN_TFAW1K:
		if (index <= SPEED_BIN_DDR_800E)
			result = 40000;
		else if (index <= SPEED_BIN_DDR_1066G)
			result = 37500;
		else if (index <= SPEED_BIN_DDR_1600K)
			result = 30000;
		else if (index <= SPEED_BIN_DDR_1866M)
			result = 27000;
		else
			result = 25000;
		break;
	case SPEED_BIN_TFAW2K:
		if (index <= SPEED_BIN_DDR_1066G)
			result = 50000;
		else if (index <= SPEED_BIN_DDR_1333J)
			result = 45000;
		else if (index <= SPEED_BIN_DDR_1600K)
			result = 40000;
		else
			result = 35000;
		break;
	case SPEED_BIN_TWTR:
		result = 7500;
		break;
	case SPEED_BIN_TRTP:
		result = 7500;
		break;
	case SPEED_BIN_TWR:
		result = 15000;
		break;
	case SPEED_BIN_TMOD:
		result = 15000;
		break;
	case SPEED_BIN_TXPDLL:
		result = 24000;
		break;
	case SPEED_BIN_TXSDLL:
		result = 512;
		break;
	default:
		break;
	}

	return result;
}

static inline u32 pattern_table_get_killer_word(u8 dqs, u8 index)
{
	u8 i, byte = 0;
	u8 role;

	for (i = 0; i < 8; i++) {
		role = (i == dqs) ?
			(PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_AGGRESSOR) :
			(PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM);
		byte |= pattern_killer_pattern_table_map[index][role] << i;
	}

	return byte | (byte << 8) | (byte << 16) | (byte << 24);
}

static inline u32 pattern_table_get_killer_word16(u8 dqs, u8 index)
{
	u8 i, byte0 = 0, byte1 = 0;
	u8 role;

	for (i = 0; i < 8; i++) {
		role = (i == dqs) ?
			(PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_AGGRESSOR) :
			(PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM);
		byte0 |= pattern_killer_pattern_table_map[index * 2][role] << i;
		byte1 |= pattern_killer_pattern_table_map[index * 2 + 1][role] << i;
	}

	return byte0 | (byte0 << 8) | (byte1 << 16) | (byte1 << 24);
}

static inline u32 pattern_table_get_sso_word(u8 sso, u8 index)
{
	u8 step = sso + 1;

	if (0 == ((index / step) & 1))
		return 0x0;
	else
		return 0xffffffff;
}

static inline u32 pattern_table_get_sso_full_xtalk_word(u8 bit, u8 index)
{
	u8 byte = (1 << bit);

	if ((index & 1) == 1)
		byte = ~byte;

	return byte | (byte << 8) | (byte << 16) | (byte << 24);

}

static inline u32 pattern_table_get_sso_xtalk_free_word(u8 bit, u8 index)
{
	u8 byte = (1 << bit);

	if ((index & 1) == 1)
		byte = 0;

	return byte | (byte << 8) | (byte << 16) | (byte << 24);
}

static inline u32 pattern_table_get_isi_word(u8 index)
{
	u8 i0 = index % 32;
	u8 i1 = index % 8;
	u32 word;

	if (i0 > 15)
		word = ((i1 == 5) | (i1 == 7)) ? 0xffffffff : 0x0;
	else
		word = (i1 == 6) ? 0xffffffff : 0x0;

	word = ((i0 % 16) > 7) ? ~word : word;

	return word;
}

static inline u32 pattern_table_get_sso_full_xtalk_word16(u8 bit, u8 index)
{
	u8 byte = (1 << bit);

	if ((index & 1) == 1)
		byte = ~byte;

	return byte | (byte << 8) | ((~byte) << 16) | ((~byte) << 24);
}

static inline u32 pattern_table_get_sso_xtalk_free_word16(u8 bit, u8 index)
{
	u8 byte = (1 << bit);

	if ((index & 1) == 0)
		return (byte << 16) | (byte << 24);
	else
		return byte | (byte << 8);
}

static inline u32 pattern_table_get_isi_word16(u8 index)
{
	u8 i0 = index % 16;
	u8 i1 = index % 4;
	u32 word;

	if (i0 > 7)
		word = (i1 > 1) ? 0x0000ffff : 0x0;
	else
		word = (i1 == 3) ? 0xffff0000 : 0x0;

	word = ((i0 % 8) > 3) ? ~word : word;

	return word;
}

static inline u32 pattern_table_get_vref_word(u8 index)
{
	if (0 == ((pattern_vref_pattern_table_map[index / 8] >>
		   (index % 8)) & 1))
		return 0x0;
	else
		return 0xffffffff;
}

static inline u32 pattern_table_get_vref_word16(u8 index)
{
	if (0 == pattern_killer_pattern_table_map
	    [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2] &&
	    0 == pattern_killer_pattern_table_map
	    [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2 + 1])
		return 0x00000000;
	else if (1 == pattern_killer_pattern_table_map
		 [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2] &&
		 0 == pattern_killer_pattern_table_map
		 [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2 + 1])
		return 0xffff0000;
	else if (0 == pattern_killer_pattern_table_map
		 [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2] &&
		 1 == pattern_killer_pattern_table_map
		 [PATTERN_KILLER_PATTERN_TABLE_MAP_ROLE_VICTIM][index * 2 + 1])
		return 0x0000ffff;
	else
		return 0xffffffff;
}

static inline u32 pattern_table_get_static_pbs_word(u8 index)
{
	u16 temp;

	temp = ((0x00ff << (index / 3)) & 0xff00) >> 8;

	return temp | (temp << 8) | (temp << 16) | (temp << 24);
}

u32 pattern_table_get_word(u32 dev_num, enum hws_pattern type, u8 index)
{
	u32 pattern = 0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask) == 0) {
		/* 32/64-bit patterns */
		switch (type) {
		case PATTERN_PBS1:
		case PATTERN_PBS2:
			if (index == 0 || index == 2 || index == 5 ||
			    index == 7)
				pattern = PATTERN_55;
			else
				pattern = PATTERN_AA;
			break;
		case PATTERN_PBS3:
			if (0 == (index & 1))
				pattern = PATTERN_55;
			else
				pattern = PATTERN_AA;
			break;
		case PATTERN_RL:
			if (index < 6)
				pattern = PATTERN_00;
			else
				pattern = PATTERN_80;
			break;
		case PATTERN_STATIC_PBS:
			pattern = pattern_table_get_static_pbs_word(index);
			break;
		case PATTERN_KILLER_DQ0:
		case PATTERN_KILLER_DQ1:
		case PATTERN_KILLER_DQ2:
		case PATTERN_KILLER_DQ3:
		case PATTERN_KILLER_DQ4:
		case PATTERN_KILLER_DQ5:
		case PATTERN_KILLER_DQ6:
		case PATTERN_KILLER_DQ7:
			pattern = pattern_table_get_killer_word(
				(u8)(type - PATTERN_KILLER_DQ0), index);
			break;
		case PATTERN_RL2:
			if (index < 6)
				pattern = PATTERN_00;
			else
				pattern = PATTERN_01;
			break;
		case PATTERN_TEST:
			if (index > 1 && index < 6)
				pattern = PATTERN_00;
			else
				pattern = PATTERN_FF;
			break;
		case PATTERN_FULL_SSO0:
		case PATTERN_FULL_SSO1:
		case PATTERN_FULL_SSO2:
		case PATTERN_FULL_SSO3:
			pattern = pattern_table_get_sso_word(
				(u8)(type - PATTERN_FULL_SSO0), index);
			break;
		case PATTERN_VREF:
			pattern = pattern_table_get_vref_word(index);
			break;
		case PATTERN_SSO_FULL_XTALK_DQ0:
		case PATTERN_SSO_FULL_XTALK_DQ1:
		case PATTERN_SSO_FULL_XTALK_DQ2:
		case PATTERN_SSO_FULL_XTALK_DQ3:
		case PATTERN_SSO_FULL_XTALK_DQ4:
		case PATTERN_SSO_FULL_XTALK_DQ5:
		case PATTERN_SSO_FULL_XTALK_DQ6:
		case PATTERN_SSO_FULL_XTALK_DQ7:
			pattern = pattern_table_get_sso_full_xtalk_word(
				(u8)(type - PATTERN_SSO_FULL_XTALK_DQ0), index);
			break;
		case PATTERN_SSO_XTALK_FREE_DQ0:
		case PATTERN_SSO_XTALK_FREE_DQ1:
		case PATTERN_SSO_XTALK_FREE_DQ2:
		case PATTERN_SSO_XTALK_FREE_DQ3:
		case PATTERN_SSO_XTALK_FREE_DQ4:
		case PATTERN_SSO_XTALK_FREE_DQ5:
		case PATTERN_SSO_XTALK_FREE_DQ6:
		case PATTERN_SSO_XTALK_FREE_DQ7:
			pattern = pattern_table_get_sso_xtalk_free_word(
				(u8)(type - PATTERN_SSO_XTALK_FREE_DQ0), index);
			break;
		case PATTERN_ISI_XTALK_FREE:
			pattern = pattern_table_get_isi_word(index);
			break;
		default:
			printf("error: %s: unsupported pattern type [%d] found\n",
			       __func__, (int)type);
			pattern = 0;
			break;
		}
	} else {
		/* 16bit patterns */
		switch (type) {
		case PATTERN_PBS1:
		case PATTERN_PBS2:
		case PATTERN_PBS3:
			pattern = PATTERN_55AA;
			break;
		case PATTERN_RL:
			if (index < 3)
				pattern = PATTERN_00;
			else
				pattern = PATTERN_80;
			break;
		case PATTERN_STATIC_PBS:
			pattern = PATTERN_00FF;
			break;
		case PATTERN_KILLER_DQ0:
		case PATTERN_KILLER_DQ1:
		case PATTERN_KILLER_DQ2:
		case PATTERN_KILLER_DQ3:
		case PATTERN_KILLER_DQ4:
		case PATTERN_KILLER_DQ5:
		case PATTERN_KILLER_DQ6:
		case PATTERN_KILLER_DQ7:
			pattern = pattern_table_get_killer_word16(
				(u8)(type - PATTERN_KILLER_DQ0), index);
			break;
		case PATTERN_RL2:
			if (index < 3)
				pattern = PATTERN_00;
			else
				pattern = PATTERN_01;
			break;
		case PATTERN_TEST:
			if ((index == 0) || (index == 3))
				pattern = 0x00000000;
			else
				pattern = 0xFFFFFFFF;
			break;
		case PATTERN_FULL_SSO0:
			pattern = 0x0000ffff;
			break;
		case PATTERN_FULL_SSO1:
		case PATTERN_FULL_SSO2:
		case PATTERN_FULL_SSO3:
			pattern = pattern_table_get_sso_word(
				(u8)(type - PATTERN_FULL_SSO1), index);
			break;
		case PATTERN_VREF:
			pattern = pattern_table_get_vref_word16(index);
			break;
		case PATTERN_SSO_FULL_XTALK_DQ0:
		case PATTERN_SSO_FULL_XTALK_DQ1:
		case PATTERN_SSO_FULL_XTALK_DQ2:
		case PATTERN_SSO_FULL_XTALK_DQ3:
		case PATTERN_SSO_FULL_XTALK_DQ4:
		case PATTERN_SSO_FULL_XTALK_DQ5:
		case PATTERN_SSO_FULL_XTALK_DQ6:
		case PATTERN_SSO_FULL_XTALK_DQ7:
			pattern = pattern_table_get_sso_full_xtalk_word16(
				(u8)(type - PATTERN_SSO_FULL_XTALK_DQ0), index);
			break;
		case PATTERN_SSO_XTALK_FREE_DQ0:
		case PATTERN_SSO_XTALK_FREE_DQ1:
		case PATTERN_SSO_XTALK_FREE_DQ2:
		case PATTERN_SSO_XTALK_FREE_DQ3:
		case PATTERN_SSO_XTALK_FREE_DQ4:
		case PATTERN_SSO_XTALK_FREE_DQ5:
		case PATTERN_SSO_XTALK_FREE_DQ6:
		case PATTERN_SSO_XTALK_FREE_DQ7:
			pattern = pattern_table_get_sso_xtalk_free_word16(
				(u8)(type - PATTERN_SSO_XTALK_FREE_DQ0), index);
			break;
		case PATTERN_ISI_XTALK_FREE:
			pattern = pattern_table_get_isi_word16(index);
			break;
		default:
			printf("error: %s: unsupported pattern type [%d] found\n",
			       __func__, (int)type);
			pattern = 0;
			break;
		}
	}

	return pattern;
}

/* Device attribute functions */
void ddr3_tip_dev_attr_init(u32 dev_num)
{
	u32 attr_id;

	for (attr_id = 0; attr_id < MV_ATTR_LAST; attr_id++)
		ddr_dev_attributes[attr_id] = 0xFF;

	ddr_dev_attr_init_done = 1;
}

u32 ddr3_tip_dev_attr_get(u32 dev_num, enum mv_ddr_dev_attribute attr_id)
{
	if (ddr_dev_attr_init_done == 0)
		ddr3_tip_dev_attr_init(dev_num);

	return ddr_dev_attributes[attr_id];
}

void ddr3_tip_dev_attr_set(u32 dev_num, enum mv_ddr_dev_attribute attr_id, u32 value)
{
	if (ddr_dev_attr_init_done == 0)
		ddr3_tip_dev_attr_init(dev_num);

	ddr_dev_attributes[attr_id] = value;
}
