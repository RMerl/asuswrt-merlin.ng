/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2008-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 */

#ifndef FSL_DDR_MEMCTL_H
#define FSL_DDR_MEMCTL_H

/*
 * Pick a basic DDR Technology.
 */
#include <ddr_spd.h>
#include <fsl_ddrc_version.h>

#define SDRAM_TYPE_DDR1		2
#define SDRAM_TYPE_DDR2		3
#define SDRAM_TYPE_LPDDR1	6
#define SDRAM_TYPE_DDR3		7
#define SDRAM_TYPE_DDR4		5

#define DDR_BL4		4	/* burst length 4 */
#define DDR_BC4		DDR_BL4	/* burst chop for ddr3 */
#define DDR_OTF		6	/* on-the-fly BC4 and BL8 */
#define DDR_BL8		8	/* burst length 8 */

#define DDR3_RTT_OFF		0
#define DDR3_RTT_60_OHM		1 /* RTT_Nom = RZQ/4 */
#define DDR3_RTT_120_OHM	2 /* RTT_Nom = RZQ/2 */
#define DDR3_RTT_40_OHM		3 /* RTT_Nom = RZQ/6 */
#define DDR3_RTT_20_OHM		4 /* RTT_Nom = RZQ/12 */
#define DDR3_RTT_30_OHM		5 /* RTT_Nom = RZQ/8 */

#define DDR4_RTT_OFF		0
#define DDR4_RTT_60_OHM		1	/* RZQ/4 */
#define DDR4_RTT_120_OHM	2	/* RZQ/2 */
#define DDR4_RTT_40_OHM		3	/* RZQ/6 */
#define DDR4_RTT_240_OHM	4	/* RZQ/1 */
#define DDR4_RTT_48_OHM		5	/* RZQ/5 */
#define DDR4_RTT_80_OHM		6	/* RZQ/3 */
#define DDR4_RTT_34_OHM		7	/* RZQ/7 */

#define DDR2_RTT_OFF		0
#define DDR2_RTT_75_OHM		1
#define DDR2_RTT_150_OHM	2
#define DDR2_RTT_50_OHM		3

#if defined(CONFIG_SYS_FSL_DDR1)
#define FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR	(1)
typedef ddr1_spd_eeprom_t generic_spd_eeprom_t;
#ifndef CONFIG_FSL_SDRAM_TYPE
#define CONFIG_FSL_SDRAM_TYPE	SDRAM_TYPE_DDR1
#endif
#elif defined(CONFIG_SYS_FSL_DDR2)
#define FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR	(3)
typedef ddr2_spd_eeprom_t generic_spd_eeprom_t;
#ifndef CONFIG_FSL_SDRAM_TYPE
#define CONFIG_FSL_SDRAM_TYPE	SDRAM_TYPE_DDR2
#endif
#elif defined(CONFIG_SYS_FSL_DDR3)
typedef ddr3_spd_eeprom_t generic_spd_eeprom_t;
#ifndef CONFIG_FSL_SDRAM_TYPE
#define CONFIG_FSL_SDRAM_TYPE	SDRAM_TYPE_DDR3
#endif
#elif defined(CONFIG_SYS_FSL_DDR4)
#define FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR	(3)	/* FIXME */
typedef struct ddr4_spd_eeprom_s generic_spd_eeprom_t;
#ifndef CONFIG_FSL_SDRAM_TYPE
#define CONFIG_FSL_SDRAM_TYPE	SDRAM_TYPE_DDR4
#endif
#endif	/* #if defined(CONFIG_SYS_FSL_DDR1) */

#define FSL_DDR_ODT_NEVER		0x0
#define FSL_DDR_ODT_CS			0x1
#define FSL_DDR_ODT_ALL_OTHER_CS	0x2
#define FSL_DDR_ODT_OTHER_DIMM		0x3
#define FSL_DDR_ODT_ALL			0x4
#define FSL_DDR_ODT_SAME_DIMM		0x5
#define FSL_DDR_ODT_CS_AND_OTHER_DIMM	0x6
#define FSL_DDR_ODT_OTHER_CS_ONSAMEDIMM	0x7

/* define bank(chip select) interleaving mode */
#define FSL_DDR_CS0_CS1			0x40
#define FSL_DDR_CS2_CS3			0x20
#define FSL_DDR_CS0_CS1_AND_CS2_CS3	(FSL_DDR_CS0_CS1 | FSL_DDR_CS2_CS3)
#define FSL_DDR_CS0_CS1_CS2_CS3		(FSL_DDR_CS0_CS1_AND_CS2_CS3 | 0x04)

/* define memory controller interleaving mode */
#define FSL_DDR_CACHE_LINE_INTERLEAVING	0x0
#define FSL_DDR_PAGE_INTERLEAVING	0x1
#define FSL_DDR_BANK_INTERLEAVING	0x2
#define FSL_DDR_SUPERBANK_INTERLEAVING	0x3
#define FSL_DDR_256B_INTERLEAVING	0x8
#define FSL_DDR_3WAY_1KB_INTERLEAVING	0xA
#define FSL_DDR_3WAY_4KB_INTERLEAVING	0xC
#define FSL_DDR_3WAY_8KB_INTERLEAVING	0xD
/* placeholder for 4-way interleaving */
#define FSL_DDR_4WAY_1KB_INTERLEAVING	0x1A
#define FSL_DDR_4WAY_4KB_INTERLEAVING	0x1C
#define FSL_DDR_4WAY_8KB_INTERLEAVING	0x1D

#define SDRAM_CS_CONFIG_EN		0x80000000

/* DDR_SDRAM_CFG - DDR SDRAM Control Configuration
 */
#define SDRAM_CFG_MEM_EN		0x80000000
#define SDRAM_CFG_SREN			0x40000000
#define SDRAM_CFG_ECC_EN		0x20000000
#define SDRAM_CFG_RD_EN			0x10000000
#define SDRAM_CFG_SDRAM_TYPE_DDR1	0x02000000
#define SDRAM_CFG_SDRAM_TYPE_DDR2	0x03000000
#define SDRAM_CFG_SDRAM_TYPE_MASK	0x07000000
#define SDRAM_CFG_SDRAM_TYPE_SHIFT	24
#define SDRAM_CFG_DYN_PWR		0x00200000
#define SDRAM_CFG_DBW_MASK		0x00180000
#define SDRAM_CFG_DBW_SHIFT		19
#define SDRAM_CFG_32_BE			0x00080000
#define SDRAM_CFG_16_BE			0x00100000
#define SDRAM_CFG_8_BE			0x00040000
#define SDRAM_CFG_NCAP			0x00020000
#define SDRAM_CFG_2T_EN			0x00008000
#define SDRAM_CFG_BI			0x00000001

#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define SDRAM_CFG2_AP_EN		0x00000020
#define SDRAM_CFG2_ODT_CFG_MASK		0x00600000
#define SDRAM_CFG2_ODT_NEVER		0
#define SDRAM_CFG2_ODT_ONLY_WRITE	1
#define SDRAM_CFG2_ODT_ONLY_READ	2
#define SDRAM_CFG2_ODT_ALWAYS		3

#define SDRAM_INTERVAL_BSTOPRE	0x3FFF
#define TIMING_CFG_2_CPO_MASK	0x0F800000

#if defined(CONFIG_SYS_FSL_DDR_VER) && \
	(CONFIG_SYS_FSL_DDR_VER > FSL_DDR_VER_4_4)
#define RD_TO_PRE_MASK		0xf
#define RD_TO_PRE_SHIFT		13
#define WR_DATA_DELAY_MASK	0xf
#define WR_DATA_DELAY_SHIFT	9
#else
#define RD_TO_PRE_MASK		0x7
#define RD_TO_PRE_SHIFT		13
#define WR_DATA_DELAY_MASK	0x7
#define WR_DATA_DELAY_SHIFT	10
#endif

/* DDR_EOR register */
#define DDR_EOR_RD_REOD_DIS	0x07000000
#define DDR_EOR_WD_REOD_DIS	0x00100000

/* DDR_MD_CNTL */
#define MD_CNTL_MD_EN		0x80000000
#define MD_CNTL_CS_SEL_CS0	0x00000000
#define MD_CNTL_CS_SEL_CS1	0x10000000
#define MD_CNTL_CS_SEL_CS2	0x20000000
#define MD_CNTL_CS_SEL_CS3	0x30000000
#define MD_CNTL_CS_SEL_CS0_CS1	0x40000000
#define MD_CNTL_CS_SEL_CS2_CS3	0x50000000
#define MD_CNTL_MD_SEL_MR	0x00000000
#define MD_CNTL_MD_SEL_EMR	0x01000000
#define MD_CNTL_MD_SEL_EMR2	0x02000000
#define MD_CNTL_MD_SEL_EMR3	0x03000000
#define MD_CNTL_SET_REF		0x00800000
#define MD_CNTL_SET_PRE		0x00400000
#define MD_CNTL_CKE_CNTL_LOW	0x00100000
#define MD_CNTL_CKE_CNTL_HIGH	0x00200000
#define MD_CNTL_WRCW		0x00080000
#define MD_CNTL_MD_VALUE(x)	(x & 0x0000FFFF)
#define MD_CNTL_CS_SEL(x)	(((x) & 0x7) << 28)
#define MD_CNTL_MD_SEL(x)	(((x) & 0xf) << 24)

/* DDR_CDR1 */
#define DDR_CDR1_DHC_EN	0x80000000
#define DDR_CDR1_V0PT9_EN	0x40000000
#define DDR_CDR1_ODT_SHIFT	17
#define DDR_CDR1_ODT_MASK	0x6
#define DDR_CDR2_ODT_MASK	0x1
#define DDR_CDR1_ODT(x) ((x & DDR_CDR1_ODT_MASK) << DDR_CDR1_ODT_SHIFT)
#define DDR_CDR2_ODT(x) (x & DDR_CDR2_ODT_MASK)
#define DDR_CDR2_VREF_OVRD(x)	(0x00008080 | ((((x) - 37) & 0x3F) << 8))
#define DDR_CDR2_VREF_TRAIN_EN	0x00000080
#define DDR_CDR2_VREF_RANGE_2	0x00000040

/* DDR ERR_DISABLE */
#define DDR_ERR_DISABLE_APED	(1 << 8)  /* Address parity error disable */

/* Mode Registers */
#define DDR_MR5_CA_PARITY_LAT_4_CLK	0x1 /* for DDR4-1600/1866/2133 */
#define DDR_MR5_CA_PARITY_LAT_5_CLK	0x2 /* for DDR4-2400 */

/* DEBUG_26 register */
#define DDR_CAS_TO_PRE_SUB_MASK  0x0000f000 /* CAS to preamble subtract value */
#define DDR_CAS_TO_PRE_SUB_SHIFT 12

/* DEBUG_29 register */
#define DDR_TX_BD_DIS	(1 << 10) /* Transmit Bit Deskew Disable */


#if (defined(CONFIG_SYS_FSL_DDR_VER) && \
	(CONFIG_SYS_FSL_DDR_VER >= FSL_DDR_VER_4_7))
#ifdef CONFIG_SYS_FSL_DDR3L
#define DDR_CDR_ODT_OFF		0x0
#define DDR_CDR_ODT_120ohm	0x1
#define DDR_CDR_ODT_200ohm	0x2
#define DDR_CDR_ODT_75ohm	0x3
#define DDR_CDR_ODT_60ohm	0x5
#define DDR_CDR_ODT_46ohm	0x7
#elif defined(CONFIG_SYS_FSL_DDR4)
#define DDR_CDR_ODT_OFF		0x0
#define DDR_CDR_ODT_100ohm	0x1
#define DDR_CDR_ODT_120OHM	0x2
#define DDR_CDR_ODT_80ohm	0x3
#define DDR_CDR_ODT_60ohm	0x4
#define DDR_CDR_ODT_40ohm	0x5
#define DDR_CDR_ODT_50ohm	0x6
#define DDR_CDR_ODT_30ohm	0x7
#else
#define DDR_CDR_ODT_OFF		0x0
#define DDR_CDR_ODT_120ohm	0x1
#define DDR_CDR_ODT_180ohm	0x2
#define DDR_CDR_ODT_75ohm	0x3
#define DDR_CDR_ODT_110ohm	0x4
#define DDR_CDR_ODT_60hm	0x5
#define DDR_CDR_ODT_70ohm	0x6
#define DDR_CDR_ODT_47ohm	0x7
#endif /* DDR3L */
#else
#define DDR_CDR_ODT_75ohm	0x0
#define DDR_CDR_ODT_55ohm	0x1
#define DDR_CDR_ODT_60ohm	0x2
#define DDR_CDR_ODT_50ohm	0x3
#define DDR_CDR_ODT_150ohm	0x4
#define DDR_CDR_ODT_43ohm	0x5
#define DDR_CDR_ODT_120ohm	0x6
#endif

#define DDR_INIT_ADDR_EXT_UIA	(1 << 31)

/* Record of register values computed */
typedef struct fsl_ddr_cfg_regs_s {
	struct {
		unsigned int bnds;
		unsigned int config;
		unsigned int config_2;
	} cs[CONFIG_CHIP_SELECTS_PER_CTRL];
	unsigned int timing_cfg_3;
	unsigned int timing_cfg_0;
	unsigned int timing_cfg_1;
	unsigned int timing_cfg_2;
	unsigned int ddr_sdram_cfg;
	unsigned int ddr_sdram_cfg_2;
	unsigned int ddr_sdram_cfg_3;
	unsigned int ddr_sdram_mode;
	unsigned int ddr_sdram_mode_2;
	unsigned int ddr_sdram_mode_3;
	unsigned int ddr_sdram_mode_4;
	unsigned int ddr_sdram_mode_5;
	unsigned int ddr_sdram_mode_6;
	unsigned int ddr_sdram_mode_7;
	unsigned int ddr_sdram_mode_8;
	unsigned int ddr_sdram_mode_9;
	unsigned int ddr_sdram_mode_10;
	unsigned int ddr_sdram_mode_11;
	unsigned int ddr_sdram_mode_12;
	unsigned int ddr_sdram_mode_13;
	unsigned int ddr_sdram_mode_14;
	unsigned int ddr_sdram_mode_15;
	unsigned int ddr_sdram_mode_16;
	unsigned int ddr_sdram_md_cntl;
	unsigned int ddr_sdram_interval;
	unsigned int ddr_data_init;
	unsigned int ddr_sdram_clk_cntl;
	unsigned int ddr_init_addr;
	unsigned int ddr_init_ext_addr;
	unsigned int timing_cfg_4;
	unsigned int timing_cfg_5;
	unsigned int timing_cfg_6;
	unsigned int timing_cfg_7;
	unsigned int timing_cfg_8;
	unsigned int timing_cfg_9;
	unsigned int ddr_zq_cntl;
	unsigned int ddr_wrlvl_cntl;
	unsigned int ddr_wrlvl_cntl_2;
	unsigned int ddr_wrlvl_cntl_3;
	unsigned int ddr_sr_cntr;
	unsigned int ddr_sdram_rcw_1;
	unsigned int ddr_sdram_rcw_2;
	unsigned int ddr_sdram_rcw_3;
	unsigned int ddr_sdram_rcw_4;
	unsigned int ddr_sdram_rcw_5;
	unsigned int ddr_sdram_rcw_6;
	unsigned int dq_map_0;
	unsigned int dq_map_1;
	unsigned int dq_map_2;
	unsigned int dq_map_3;
	unsigned int ddr_eor;
	unsigned int ddr_cdr1;
	unsigned int ddr_cdr2;
	unsigned int err_disable;
	unsigned int err_int_en;
	unsigned int debug[64];
} fsl_ddr_cfg_regs_t;

typedef struct memctl_options_partial_s {
	unsigned int all_dimms_ecc_capable;
	unsigned int all_dimms_tckmax_ps;
	unsigned int all_dimms_burst_lengths_bitmask;
	unsigned int all_dimms_registered;
	unsigned int all_dimms_unbuffered;
	/*	unsigned int lowest_common_spd_caslat; */
	unsigned int all_dimms_minimum_trcd_ps;
} memctl_options_partial_t;

#define DDR_DATA_BUS_WIDTH_64 0
#define DDR_DATA_BUS_WIDTH_32 1
#define DDR_DATA_BUS_WIDTH_16 2
#define DDR_CSWL_CS0	0x04000001
/*
 * Generalized parameters for memory controller configuration,
 * might be a little specific to the FSL memory controller
 */
typedef struct memctl_options_s {
	/*
	 * Memory organization parameters
	 *
	 * if DIMM is present in the system
	 * where DIMMs are with respect to chip select
	 * where chip selects are with respect to memory boundaries
	 */
	unsigned int registered_dimm_en;    /* use registered DIMM support */

	/* Options local to a Chip Select */
	struct cs_local_opts_s {
		unsigned int auto_precharge;
		unsigned int odt_rd_cfg;
		unsigned int odt_wr_cfg;
		unsigned int odt_rtt_norm;
		unsigned int odt_rtt_wr;
	} cs_local_opts[CONFIG_CHIP_SELECTS_PER_CTRL];

	/* Special configurations for chip select */
	unsigned int memctl_interleaving;
	unsigned int memctl_interleaving_mode;
	unsigned int ba_intlv_ctl;
	unsigned int addr_hash;

	/* Operational mode parameters */
	unsigned int ecc_mode;	 /* Use ECC? */
	/* Initialize ECC using memory controller? */
	unsigned int ecc_init_using_memctl;
	unsigned int dqs_config;	/* Use DQS? maybe only with DDR2? */
	/* SREN - self-refresh during sleep */
	unsigned int self_refresh_in_sleep;
	/* SR_IE - Self-refresh interrupt enable */
	unsigned int self_refresh_interrupt_en;
	unsigned int dynamic_power;	/* DYN_PWR */
	/* memory data width to use (16-bit, 32-bit, 64-bit) */
	unsigned int data_bus_width;
	unsigned int burst_length;	/* BL4, OTF and BL8 */
	/* On-The-Fly Burst Chop enable */
	unsigned int otf_burst_chop_en;
	/* mirrior DIMMs for DDR3 */
	unsigned int mirrored_dimm;
	unsigned int quad_rank_present;
	unsigned int ap_en;	/* address parity enable for RDIMM/DDR4-UDIMM */
	unsigned int x4_en;	/* enable x4 devices */
	unsigned int package_3ds;

	/* Global Timing Parameters */
	unsigned int cas_latency_override;
	unsigned int cas_latency_override_value;
	unsigned int use_derated_caslat;
	unsigned int additive_latency_override;
	unsigned int additive_latency_override_value;

	unsigned int clk_adjust;		/* */
	unsigned int cpo_override;		/* override timing_cfg_2[CPO]*/
	unsigned int cpo_sample;		/* optimize debug_29[24:31] */
	unsigned int write_data_delay;		/* DQS adjust */

	unsigned int cswl_override;
	unsigned int wrlvl_override;
	unsigned int wrlvl_sample;		/* Write leveling */
	unsigned int wrlvl_start;
	unsigned int wrlvl_ctl_2;
	unsigned int wrlvl_ctl_3;

	unsigned int half_strength_driver_enable;
	unsigned int twot_en;
	unsigned int threet_en;
	unsigned int bstopre;
	unsigned int tfaw_window_four_activates_ps;	/* tFAW --  FOUR_ACT */

	/* Rtt impedance */
	unsigned int rtt_override;		/* rtt_override enable */
	unsigned int rtt_override_value;	/* that is Rtt_Nom for DDR3 */
	unsigned int rtt_wr_override_value;	/* this is Rtt_WR for DDR3 */

	/* Automatic self refresh */
	unsigned int auto_self_refresh_en;
	unsigned int sr_it;
	/* ZQ calibration */
	unsigned int zq_en;
	/* Write leveling */
	unsigned int wrlvl_en;
	/* RCW override for RDIMM */
	unsigned int rcw_override;
	unsigned int rcw_1;
	unsigned int rcw_2;
	unsigned int rcw_3;
	/* control register 1 */
	unsigned int ddr_cdr1;
	unsigned int ddr_cdr2;

	unsigned int trwt_override;
	unsigned int trwt;			/* read-to-write turnaround */
} memctl_options_t;

phys_size_t fsl_ddr_sdram(void);
phys_size_t fsl_ddr_sdram_size(void);
phys_size_t fsl_other_ddr_sdram(unsigned long long base,
				unsigned int first_ctrl,
				unsigned int num_ctrls,
				unsigned int dimm_slots_per_ctrl,
				int (*board_need_reset)(void),
				void (*board_reset)(void),
				void (*board_de_reset)(void));
extern int fsl_use_spd(void);
void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num, int step);
u32 fsl_ddr_get_intl3r(void);
void print_ddr_info(unsigned int start_ctrl);

static void __board_assert_mem_reset(void)
{
}

static void __board_deassert_mem_reset(void)
{
}

void board_assert_mem_reset(void)
	__attribute__((weak, alias("__board_assert_mem_reset")));

void board_deassert_mem_reset(void)
	__attribute__((weak, alias("__board_deassert_mem_reset")));

static int __board_need_mem_reset(void)
{
	return 0;
}

int board_need_mem_reset(void)
	__attribute__((weak, alias("__board_need_mem_reset")));

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void);
bool is_warm_boot(void);
int fsl_dp_resume(void);
#endif

/*
 * The 85xx boards have a common prototype for fixed_sdram so put the
 * declaration here.
 */
#ifdef CONFIG_MPC85xx
extern phys_size_t fixed_sdram(void);
#endif

#if defined(CONFIG_DDR_ECC)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif


typedef struct fixed_ddr_parm{
	int min_freq;
	int max_freq;
	fsl_ddr_cfg_regs_t *ddr_settings;
} fixed_ddr_parm_t;

/**
 * fsl_initdram() - Set up the SDRAM
 *
 * @return 0 if OK, -ve on error
 */
int fsl_initdram(void);

#endif
