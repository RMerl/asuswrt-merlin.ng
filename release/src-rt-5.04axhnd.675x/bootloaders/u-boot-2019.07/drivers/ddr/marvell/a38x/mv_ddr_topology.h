/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_TOPOLOGY_H
#define _MV_DDR_TOPOLOGY_H

#define MAX_CS_NUM	4

enum mv_ddr_speed_bin {
	SPEED_BIN_DDR_800D,
	SPEED_BIN_DDR_800E,
	SPEED_BIN_DDR_1066E,
	SPEED_BIN_DDR_1066F,
	SPEED_BIN_DDR_1066G,
	SPEED_BIN_DDR_1333F,
	SPEED_BIN_DDR_1333G,
	SPEED_BIN_DDR_1333H,
	SPEED_BIN_DDR_1333J,
	SPEED_BIN_DDR_1600G,
	SPEED_BIN_DDR_1600H,
	SPEED_BIN_DDR_1600J,
	SPEED_BIN_DDR_1600K,
	SPEED_BIN_DDR_1866J,
	SPEED_BIN_DDR_1866K,
	SPEED_BIN_DDR_1866L,
	SPEED_BIN_DDR_1866M,
	SPEED_BIN_DDR_2133K,
	SPEED_BIN_DDR_2133L,
	SPEED_BIN_DDR_2133M,
	SPEED_BIN_DDR_2133N,

	SPEED_BIN_DDR_1333H_EXT,
	SPEED_BIN_DDR_1600K_EXT,
	SPEED_BIN_DDR_1866M_EXT
};

enum mv_ddr_freq {
	MV_DDR_FREQ_LOW_FREQ,
	MV_DDR_FREQ_400,
	MV_DDR_FREQ_533,
	MV_DDR_FREQ_667,
	MV_DDR_FREQ_800,
	MV_DDR_FREQ_933,
	MV_DDR_FREQ_1066,
	MV_DDR_FREQ_311,
	MV_DDR_FREQ_333,
	MV_DDR_FREQ_467,
	MV_DDR_FREQ_850,
	MV_DDR_FREQ_600,
	MV_DDR_FREQ_300,
	MV_DDR_FREQ_900,
	MV_DDR_FREQ_360,
	MV_DDR_FREQ_1000,
	MV_DDR_FREQ_LAST,
	MV_DDR_FREQ_SAR
};

enum mv_ddr_speed_bin_timing {
	SPEED_BIN_TRCD,
	SPEED_BIN_TRP,
	SPEED_BIN_TRAS,
	SPEED_BIN_TRC,
	SPEED_BIN_TRRD1K,
	SPEED_BIN_TRRD2K,
	SPEED_BIN_TPD,
	SPEED_BIN_TFAW1K,
	SPEED_BIN_TFAW2K,
	SPEED_BIN_TWTR,
	SPEED_BIN_TRTP,
	SPEED_BIN_TWR,
	SPEED_BIN_TMOD,
	SPEED_BIN_TXPDLL,
	SPEED_BIN_TXSDLL
};

/* ddr bus masks */
#define BUS_MASK_32BIT			0xf
#define BUS_MASK_32BIT_ECC		0x1f
#define BUS_MASK_16BIT			0x3
#define BUS_MASK_16BIT_ECC		0x13
#define BUS_MASK_16BIT_ECC_PUP3		0xb
#define MV_DDR_64BIT_BUS_MASK		0xff
#define MV_DDR_64BIT_ECC_PUP8_BUS_MASK	0x1ff
#define MV_DDR_32BIT_ECC_PUP8_BUS_MASK	0x10f

#define MV_DDR_CS_BITMASK_1CS		0x1
#define MV_DDR_CS_BITMASK_2CS		0x3

#define MV_DDR_ONE_SPHY_PER_DUNIT	1
#define MV_DDR_TWO_SPHY_PER_DUNIT	2

/* source of ddr configuration data */
enum mv_ddr_cfg_src {
	MV_DDR_CFG_DEFAULT,	/* based on data in mv_ddr_topology_map structure */
	MV_DDR_CFG_SPD,		/* based on data in spd */
	MV_DDR_CFG_USER,	/* based on data from user */
	MV_DDR_CFG_STATIC,	/* based on data from user in register-value format */
	MV_DDR_CFG_LAST
};

enum mv_ddr_temperature {
	MV_DDR_TEMP_LOW,
	MV_DDR_TEMP_NORMAL,
	MV_DDR_TEMP_HIGH
};

enum mv_ddr_timing {
	MV_DDR_TIM_DEFAULT,
	MV_DDR_TIM_1T,
	MV_DDR_TIM_2T
};

enum mv_ddr_timing_data {
	MV_DDR_TCK_AVG_MIN, /* sdram min cycle time (t ck avg min) */
	MV_DDR_TAA_MIN, /* min cas latency time (t aa min) */
	MV_DDR_TRFC1_MIN, /* min refresh recovery delay time (t rfc1 min) */
	MV_DDR_TWR_MIN, /* min write recovery time (t wr min) */
	MV_DDR_TRCD_MIN, /* min ras to cas delay time (t rcd min) */
	MV_DDR_TRP_MIN, /* min row precharge delay time (t rp min) */
	MV_DDR_TRC_MIN, /* min active to active/refresh delay time (t rc min) */
	MV_DDR_TRAS_MIN, /* min active to precharge delay time (t ras min) */
	MV_DDR_TRRD_S_MIN, /* min activate to activate delay time (t rrd_s min), diff bank group */
	MV_DDR_TRRD_L_MIN, /* min activate to activate delay time (t rrd_l min), same bank group */
	MV_DDR_TCCD_L_MIN, /* min cas to cas delay time (t ccd_l min), same bank group */
	MV_DDR_TFAW_MIN, /* min four activate window delay time (t faw min) */
	MV_DDR_TWTR_S_MIN, /* min write to read time (t wtr s min), diff bank group */
	MV_DDR_TWTR_L_MIN, /* min write to read time (t wtr l min), same bank group */
	MV_DDR_TDATA_LAST
};

enum mv_ddr_electrical_data {
	MV_DDR_CK_DLY,
	MV_DDR_PHY_REG3,
	MV_DDR_ZPRI_DATA,
	MV_DDR_ZNRI_DATA,
	MV_DDR_ZPRI_CTRL,
	MV_DDR_ZNRI_CTRL,
	MV_DDR_ZPODT_DATA,
	MV_DDR_ZNODT_DATA,
	MV_DDR_ZPODT_CTRL,
	MV_DDR_ZNODT_CTRL,
	MV_DDR_DIC,
	MV_DDR_ODT_CFG,
	MV_DDR_RTT_NOM,
	MV_DDR_RTT_WR,
	MV_DDR_RTT_PARK,
	MV_DDR_EDATA_LAST
};

/* memory electrical configuration values */
enum mv_ddr_rtt_nom_park_evalue {
	MV_DDR_RTT_NOM_PARK_RZQ_DISABLE,
	MV_DDR_RTT_NOM_PARK_RZQ_DIV4,	/* 60-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV2,	/* 120-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV6,	/* 40-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV1,	/* 240-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV5,	/* 48-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV3,	/* 80-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_DIV7,	/* 34-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_NOM_PARK_RZQ_LAST
};

enum mv_ddr_rtt_wr_evalue {
	MV_DDR_RTT_WR_DYN_ODT_OFF,
	MV_DDR_RTT_WR_RZQ_DIV2,	/* 120-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_WR_RZQ_DIV1,	/* 240-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_WR_HIZ,
	MV_DDR_RTT_WR_RZQ_DIV3,	/* 80-Ohm; RZQ = 240-Ohm */
	MV_DDR_RTT_WR_RZQ_LAST
};

enum mv_ddr_dic_evalue {
	MV_DDR_DIC_RZQ_DIV7,	/* 34-Ohm; RZQ = 240-Ohm */
	MV_DDR_DIC_RZQ_DIV5,	/* 48-Ohm; RZQ = 240-Ohm */
	MV_DDR_DIC_RZQ_LAST
};

/* phy electrical configuration values */
enum mv_ddr_ohm_evalue {
	MV_DDR_OHM_30 = 30,
	MV_DDR_OHM_48 = 48,
	MV_DDR_OHM_60 = 60,
	MV_DDR_OHM_80 = 80,
	MV_DDR_OHM_120 = 120,
	MV_DDR_OHM_240 = 240,
	MV_DDR_OHM_LAST
};

/* mac electrical configuration values */
enum mv_ddr_odt_cfg_evalue {
	MV_DDR_ODT_CFG_NORMAL,
	MV_DDR_ODT_CFG_ALWAYS_ON,
	MV_DDR_ODT_CFG_LAST
};

enum mv_ddr_dev_width { /* sdram device width */
	MV_DDR_DEV_WIDTH_4BIT,
	MV_DDR_DEV_WIDTH_8BIT,
	MV_DDR_DEV_WIDTH_16BIT,
	MV_DDR_DEV_WIDTH_32BIT,
	MV_DDR_DEV_WIDTH_LAST
};

enum mv_ddr_die_capacity { /* total sdram capacity per die, megabits */
	MV_DDR_DIE_CAP_256MBIT,
	MV_DDR_DIE_CAP_512MBIT = 0,
	MV_DDR_DIE_CAP_1GBIT,
	MV_DDR_DIE_CAP_2GBIT,
	MV_DDR_DIE_CAP_4GBIT,
	MV_DDR_DIE_CAP_8GBIT,
	MV_DDR_DIE_CAP_16GBIT,
	MV_DDR_DIE_CAP_32GBIT,
	MV_DDR_DIE_CAP_12GBIT,
	MV_DDR_DIE_CAP_24GBIT,
	MV_DDR_DIE_CAP_LAST
};

enum mv_ddr_pkg_rank { /* number of package ranks per dimm */
	MV_DDR_PKG_RANK_1,
	MV_DDR_PKG_RANK_2,
	MV_DDR_PKG_RANK_3,
	MV_DDR_PKG_RANK_4,
	MV_DDR_PKG_RANK_5,
	MV_DDR_PKG_RANK_6,
	MV_DDR_PKG_RANK_7,
	MV_DDR_PKG_RANK_8,
	MV_DDR_PKG_RANK_LAST
};

enum mv_ddr_pri_bus_width { /* number of primary bus width bits */
	MV_DDR_PRI_BUS_WIDTH_8,
	MV_DDR_PRI_BUS_WIDTH_16,
	MV_DDR_PRI_BUS_WIDTH_32,
	MV_DDR_PRI_BUS_WIDTH_64,
	MV_DDR_PRI_BUS_WIDTH_LAST
};

enum mv_ddr_bus_width_ext { /* number of extension bus width bits */
	MV_DDR_BUS_WIDTH_EXT_0,
	MV_DDR_BUS_WIDTH_EXT_8,
	MV_DDR_BUS_WIDTH_EXT_LAST
};

enum mv_ddr_die_count {
	MV_DDR_DIE_CNT_1,
	MV_DDR_DIE_CNT_2,
	MV_DDR_DIE_CNT_3,
	MV_DDR_DIE_CNT_4,
	MV_DDR_DIE_CNT_5,
	MV_DDR_DIE_CNT_6,
	MV_DDR_DIE_CNT_7,
	MV_DDR_DIE_CNT_8,
	MV_DDR_DIE_CNT_LAST
};

#define IS_ACTIVE(mask, id) \
	((mask) & (1 << (id)))

#define VALIDATE_ACTIVE(mask, id)		\
	{					\
	if (IS_ACTIVE(mask, id) == 0)		\
		continue;			\
	}

#define IS_IF_ACTIVE(if_mask, if_id) \
	((if_mask) & (1 << (if_id)))

#define VALIDATE_IF_ACTIVE(mask, id)		\
	{					\
	if (IS_IF_ACTIVE(mask, id) == 0)	\
		continue;			\
	}

#define IS_BUS_ACTIVE(if_mask , if_id) \
	(((if_mask) >> (if_id)) & 1)

#define VALIDATE_BUS_ACTIVE(mask, id)		\
	{					\
	if (IS_BUS_ACTIVE(mask, id) == 0)	\
		continue;			\
	}

#define DDR3_IS_ECC_PUP3_MODE(if_mask)		\
	(((if_mask) == BUS_MASK_16BIT_ECC_PUP3) ? 1 : 0)

#define DDR3_IS_ECC_PUP4_MODE(if_mask)		\
	(((if_mask) == BUS_MASK_32BIT_ECC ||	\
	  (if_mask) == BUS_MASK_16BIT_ECC) ? 1 : 0)

#define DDR3_IS_16BIT_DRAM_MODE(mask)		\
	(((mask) == BUS_MASK_16BIT ||		\
	  (mask) == BUS_MASK_16BIT_ECC ||	\
	  (mask) == BUS_MASK_16BIT_ECC_PUP3) ? 1 : 0)

#define DDR3_IS_ECC_PUP8_MODE(if_mask)				\
	(((if_mask) == MV_DDR_32BIT_ECC_PUP8_BUS_MASK ||	\
	  (if_mask) == MV_DDR_64BIT_ECC_PUP8_BUS_MASK) ? 1 : 0)

#define MV_DDR_IS_64BIT_DRAM_MODE(mask)					\
	((((mask) & MV_DDR_64BIT_BUS_MASK) == MV_DDR_64BIT_BUS_MASK) ||	\
	 (((mask) & MV_DDR_64BIT_ECC_PUP8_BUS_MASK) == MV_DDR_64BIT_ECC_PUP8_BUS_MASK) ? 1 : 0)

#define MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(mask, sphys)		\
	(((sphys) == 9) &&					\
	(((mask) == BUS_MASK_32BIT) ||				\
	 ((mask) == MV_DDR_32BIT_ECC_PUP8_BUS_MASK)) ? 1 : 0)

#define MV_DDR_IS_HALF_BUS_DRAM_MODE(mask, sphys)		\
	(MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(mask, sphys) ||	\
	 DDR3_IS_16BIT_DRAM_MODE(mask))

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void);
unsigned int mv_ddr_cl_calc(unsigned int taa_min, unsigned int tclk);
unsigned int mv_ddr_cwl_calc(unsigned int tclk);
int mv_ddr_topology_map_update(void);
unsigned short mv_ddr_bus_bit_mask_get(void);
unsigned int mv_ddr_if_bus_width_get(void);
unsigned int mv_ddr_cs_num_get(void);
int mv_ddr_is_ecc_ena(void);
unsigned long long mv_ddr_mem_sz_per_cs_get(void);
unsigned long long mv_ddr_mem_sz_get(void);
unsigned int mv_ddr_rtt_nom_get(void);
unsigned int mv_ddr_rtt_park_get(void);
unsigned int mv_ddr_rtt_wr_get(void);
unsigned int mv_ddr_dic_get(void);

#endif /* _MV_DDR_TOPOLOGY_H */
