/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_PLAT_H
#define _MV_DDR_PLAT_H

#define MAX_DEVICE_NUM			1
#define MAX_INTERFACE_NUM		1
#define MAX_BUS_NUM			5
#define DDR_IF_CTRL_SUBPHYS_NUM		3

#define DFS_LOW_FREQ_VALUE		120
#define SDRAM_CS_SIZE			0xfffffff	/* FIXME: implement a function for cs size for each platform */

#define INTER_REGS_BASE			SOC_REGS_PHY_BASE
#define AP_INT_REG_START_ADDR		0xd0000000
#define AP_INT_REG_END_ADDR		0xd0100000

/* Controler bus divider 1 for 32 bit, 2 for 64 bit */
#define DDR_CONTROLLER_BUS_WIDTH_MULTIPLIER	1

/* Tune internal training params values */
#define TUNE_TRAINING_PARAMS_CK_DELAY		160
#define TUNE_TRAINING_PARAMS_PHYREG3VAL		0xA
#define TUNE_TRAINING_PARAMS_PRI_DATA		123
#define TUNE_TRAINING_PARAMS_NRI_DATA		123
#define TUNE_TRAINING_PARAMS_PRI_CTRL		74
#define TUNE_TRAINING_PARAMS_NRI_CTRL		74
#define TUNE_TRAINING_PARAMS_P_ODT_DATA		45
#define TUNE_TRAINING_PARAMS_N_ODT_DATA		45
#define TUNE_TRAINING_PARAMS_P_ODT_CTRL		45
#define TUNE_TRAINING_PARAMS_N_ODT_CTRL		45
#define TUNE_TRAINING_PARAMS_DIC		0x2
#define TUNE_TRAINING_PARAMS_ODT_CONFIG_2CS	0x120012
#define TUNE_TRAINING_PARAMS_ODT_CONFIG_1CS	0x10000
#define TUNE_TRAINING_PARAMS_RTT_NOM		0x44

#define TUNE_TRAINING_PARAMS_RTT_WR_1CS		0x0   /*off*/
#define TUNE_TRAINING_PARAMS_RTT_WR_2CS		0x0   /*off*/

#define MARVELL_BOARD				MARVELL_BOARD_ID_BASE


#define REG_DEVICE_SAR1_ADDR			0xe4204
#define RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET	17
#define RST2_CPU_DDR_CLOCK_SELECT_IN_MASK	0x1f
#define DEVICE_SAMPLE_AT_RESET2_REG		0x18604

#define DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_OFFSET	0
#define DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_25MHZ	0
#define DEVICE_SAMPLE_AT_RESET2_REG_REFCLK_40MHZ	1

/* DRAM Windows */
#define REG_XBAR_WIN_5_CTRL_ADDR		0x20050
#define REG_XBAR_WIN_5_BASE_ADDR		0x20054

/* DRAM Windows */
#define REG_XBAR_WIN_4_CTRL_ADDR                0x20040
#define REG_XBAR_WIN_4_BASE_ADDR                0x20044
#define REG_XBAR_WIN_4_REMAP_ADDR               0x20048
#define REG_XBAR_WIN_7_REMAP_ADDR               0x20078
#define REG_XBAR_WIN_16_CTRL_ADDR               0x200d0
#define REG_XBAR_WIN_16_BASE_ADDR               0x200d4
#define REG_XBAR_WIN_16_REMAP_ADDR              0x200dc
#define REG_XBAR_WIN_19_CTRL_ADDR               0x200e8

#define REG_FASTPATH_WIN_BASE_ADDR(win)         (0x20180 + (0x8 * win))
#define REG_FASTPATH_WIN_CTRL_ADDR(win)         (0x20184 + (0x8 * win))

#define CPU_CONFIGURATION_REG(id)	(0x21800 + (id * 0x100))
#define CPU_MRVL_ID_OFFSET		0x10
#define SAR1_CPU_CORE_MASK		0x00000018
#define SAR1_CPU_CORE_OFFSET		3

/* SatR defined too change topology busWidth and ECC configuration */
#define DDR_SATR_CONFIG_MASK_WIDTH		0x8
#define DDR_SATR_CONFIG_MASK_ECC		0x10
#define DDR_SATR_CONFIG_MASK_ECC_PUP		0x20

#define	REG_SAMPLE_RESET_HIGH_ADDR		0x18600

#define MV_BOARD_REFCLK_25MHZ			25000000
#define MV_BOARD_REFCLK				MV_BOARD_REFCLK_25MHZ

#define MAX_DQ_NUM				40

/* dram line buffer registers */
#define DLB_CTRL_REG			0x1700
#define DLB_EN_OFFS			0
#define DLB_EN_MASK			0x1
#define DLB_EN_ENA			1
#define DLB_EN_DIS			0
#define WR_COALESCE_EN_OFFS		2
#define WR_COALESCE_EN_MASK		0x1
#define WR_COALESCE_EN_ENA		1
#define WR_COALESCE_EN_DIS		0
#define AXI_PREFETCH_EN_OFFS		3
#define AXI_PREFETCH_EN_MASK		0x1
#define AXI_PREFETCH_EN_ENA		1
#define AXI_PREFETCH_EN_DIS		0
#define MBUS_PREFETCH_EN_OFFS		4
#define MBUS_PREFETCH_EN_MASK		0x1
#define MBUS_PREFETCH_EN_ENA		1
#define MBUS_PREFETCH_EN_DIS		0
#define PREFETCH_NXT_LN_SZ_TRIG_OFFS	6
#define PREFETCH_NXT_LN_SZ_TRIG_MASK	0x1
#define PREFETCH_NXT_LN_SZ_TRIG_ENA	1
#define PREFETCH_NXT_LN_SZ_TRIG_DIS	0

#define DLB_BUS_OPT_WT_REG		0x1704
#define DLB_AGING_REG			0x1708
#define DLB_EVICTION_CTRL_REG		0x170c
#define DLB_EVICTION_TIMERS_REG		0x1710
#define DLB_USER_CMD_REG		0x1714
#define DLB_WTS_DIFF_CS_REG		0x1770
#define DLB_WTS_DIFF_BG_REG		0x1774
#define DLB_WTS_SAME_BG_REG		0x1778
#define DLB_WTS_CMDS_REG		0x177c
#define DLB_WTS_ATTR_PRIO_REG		0x1780
#define DLB_QUEUE_MAP_REG		0x1784
#define DLB_SPLIT_REG			0x1788

/* ck swap control subphy number */
#define CK_SWAP_CTRL_PHY_NUM	2

/* Subphy result control per byte registers */
#define RESULT_CONTROL_BYTE_PUP_0_REG		0x1830
#define RESULT_CONTROL_BYTE_PUP_1_REG		0x1834
#define RESULT_CONTROL_BYTE_PUP_2_REG		0x1838
#define RESULT_CONTROL_BYTE_PUP_3_REG		0x183c
#define RESULT_CONTROL_BYTE_PUP_4_REG		0x18b0

/* Subphy result control per bit registers */
#define RESULT_CONTROL_PUP_0_BIT_0_REG		0x18b4
#define RESULT_CONTROL_PUP_0_BIT_1_REG		0x18b8
#define RESULT_CONTROL_PUP_0_BIT_2_REG		0x18bc
#define RESULT_CONTROL_PUP_0_BIT_3_REG		0x18c0
#define RESULT_CONTROL_PUP_0_BIT_4_REG		0x18c4
#define RESULT_CONTROL_PUP_0_BIT_5_REG		0x18c8
#define RESULT_CONTROL_PUP_0_BIT_6_REG		0x18cc
#define RESULT_CONTROL_PUP_0_BIT_7_REG		0x18f0

#define RESULT_CONTROL_PUP_1_BIT_0_REG		0x18f4
#define RESULT_CONTROL_PUP_1_BIT_1_REG		0x18f8
#define RESULT_CONTROL_PUP_1_BIT_2_REG		0x18fc
#define RESULT_CONTROL_PUP_1_BIT_3_REG		0x1930
#define RESULT_CONTROL_PUP_1_BIT_4_REG		0x1934
#define RESULT_CONTROL_PUP_1_BIT_5_REG		0x1938
#define RESULT_CONTROL_PUP_1_BIT_6_REG		0x193c
#define RESULT_CONTROL_PUP_1_BIT_7_REG		0x19b0

#define RESULT_CONTROL_PUP_2_BIT_0_REG		0x19b4
#define RESULT_CONTROL_PUP_2_BIT_1_REG		0x19b8
#define RESULT_CONTROL_PUP_2_BIT_2_REG		0x19bc
#define RESULT_CONTROL_PUP_2_BIT_3_REG		0x19c0
#define RESULT_CONTROL_PUP_2_BIT_4_REG		0x19c4
#define RESULT_CONTROL_PUP_2_BIT_5_REG		0x19c8
#define RESULT_CONTROL_PUP_2_BIT_6_REG		0x19cc
#define RESULT_CONTROL_PUP_2_BIT_7_REG		0x19f0

#define RESULT_CONTROL_PUP_3_BIT_0_REG		0x19f4
#define RESULT_CONTROL_PUP_3_BIT_1_REG		0x19f8
#define RESULT_CONTROL_PUP_3_BIT_2_REG		0x19fc
#define RESULT_CONTROL_PUP_3_BIT_3_REG		0x1a30
#define RESULT_CONTROL_PUP_3_BIT_4_REG		0x1a34
#define RESULT_CONTROL_PUP_3_BIT_5_REG		0x1a38
#define RESULT_CONTROL_PUP_3_BIT_6_REG		0x1a3c
#define RESULT_CONTROL_PUP_3_BIT_7_REG		0x1ab0

#define RESULT_CONTROL_PUP_4_BIT_0_REG		0x1ab4
#define RESULT_CONTROL_PUP_4_BIT_1_REG		0x1ab8
#define RESULT_CONTROL_PUP_4_BIT_2_REG		0x1abc
#define RESULT_CONTROL_PUP_4_BIT_3_REG		0x1ac0
#define RESULT_CONTROL_PUP_4_BIT_4_REG		0x1ac4
#define RESULT_CONTROL_PUP_4_BIT_5_REG		0x1ac8
#define RESULT_CONTROL_PUP_4_BIT_6_REG		0x1acc
#define RESULT_CONTROL_PUP_4_BIT_7_REG		0x1af0

/* CPU */
#define REG_BOOTROM_ROUTINE_ADDR		0x182d0
#define REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS	12

/* Matrix enables DRAM modes (bus width/ECC) per boardId */
#define TOPOLOGY_UPDATE_32BIT			0
#define TOPOLOGY_UPDATE_32BIT_ECC		1
#define TOPOLOGY_UPDATE_16BIT			2
#define TOPOLOGY_UPDATE_16BIT_ECC		3
#define TOPOLOGY_UPDATE_16BIT_ECC_PUP3		4
#define TOPOLOGY_UPDATE { \
		/* 32Bit, 32bit ECC, 16bit, 16bit ECC PUP4, 16bit ECC PUP3 */ \
		{1, 1, 1, 1, 1},	/* RD_NAS_68XX_ID */ \
		{1, 1, 1, 1, 1},	/* DB_68XX_ID	  */ \
		{1, 0, 1, 0, 1},	/* RD_AP_68XX_ID  */ \
		{1, 0, 1, 0, 1},	/* DB_AP_68XX_ID  */ \
		{1, 0, 1, 0, 1},	/* DB_GP_68XX_ID  */ \
		{0, 0, 1, 1, 0},	/* DB_BP_6821_ID  */ \
		{1, 1, 1, 1, 1}		/* DB_AMC_6820_ID */ \
	};

enum {
	CPU_1066MHZ_DDR_400MHZ,
	CPU_RESERVED_DDR_RESERVED0,
	CPU_667MHZ_DDR_667MHZ,
	CPU_800MHZ_DDR_800MHZ,
	CPU_RESERVED_DDR_RESERVED1,
	CPU_RESERVED_DDR_RESERVED2,
	CPU_RESERVED_DDR_RESERVED3,
	LAST_FREQ
};

/* struct used for DLB configuration array */
struct dlb_config {
	u32 reg_addr;
	u32 reg_data;
};

#define ACTIVE_INTERFACE_MASK			0x1

extern u32 dmin_phy_reg_table[][2];
extern u16 odt_slope[];
extern u16 odt_intercept[];

int mv_ddr_pre_training_soc_config(const char *ddr_type);
int mv_ddr_post_training_soc_config(const char *ddr_type);
void mv_ddr_mem_scrubbing(void);
u32 mv_ddr_init_freq_get(void);
void mv_ddr_odpg_enable(void);
void mv_ddr_odpg_disable(void);
void mv_ddr_odpg_done_clr(void);
int mv_ddr_is_odpg_done(u32 count);
void mv_ddr_training_enable(void);
int mv_ddr_is_training_done(u32 count, u32 *result);
u32 mv_ddr_dm_pad_get(void);
int mv_ddr_pre_training_fixup(void);
int mv_ddr_post_training_fixup(void);
int mv_ddr_manual_cal_do(void);
int ddr3_calc_mem_cs_size(u32 cs, uint64_t *cs_size);

#endif /* _MV_DDR_PLAT_H */
