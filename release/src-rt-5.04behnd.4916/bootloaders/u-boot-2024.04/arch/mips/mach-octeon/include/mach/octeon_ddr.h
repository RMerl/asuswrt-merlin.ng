/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_DDR_H_
#define __OCTEON_DDR_H_

#include <env.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-lmcx-defs.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-l2c-defs.h>

/* Some "external" (non-LMC) registers */
#define CVMX_IPD_CLK_COUNT		0x00014F0000000338
#define CVMX_FPA_CLK_COUNT		0x00012800000000F0

#define CVMX_NODE_MEM_SHIFT	40

#define DDR_INTERFACE_MAX	4

/* Private data struct */
struct ddr_priv {
	void __iomem *lmc_base;
	void __iomem *l2c_base;

	bool ddr_clock_initialized[DDR_INTERFACE_MAX];
	bool ddr_memory_preserved;
	u32 flags;

	struct ram_info info;
};

/* Short cut to convert a number to megabytes */
#define MB(X)			((u64)(X) * (u64)(1024 * 1024))

#define octeon_is_cpuid(x)	(__OCTEON_IS_MODEL_COMPILE__(x, read_c0_prid()))

#define strtoull		simple_strtoull

/* Access LMC registers */
static inline u64 lmc_rd(struct ddr_priv *priv, u64 addr)
{
	return ioread64(priv->lmc_base + addr);
}

static inline void lmc_wr(struct ddr_priv *priv, u64 addr, u64 val)
{
	iowrite64(val, priv->lmc_base + addr);
}

/* Access L2C registers */
static inline u64 l2c_rd(struct ddr_priv *priv, u64 addr)
{
	return ioread64(priv->l2c_base + addr);
}

static inline void l2c_wr(struct ddr_priv *priv, u64 addr, u64 val)
{
	iowrite64(val, priv->l2c_base + addr);
}

/* Failsafe mode */
#define FLAG_FAILSAFE_MODE		0x01000
/* Note that the DDR clock initialized flags must be contiguous */
/* Clock for DDR 0 initialized */
#define FLAG_DDR0_CLK_INITIALIZED	0x02000
/* Clock for DDR 1 initialized */
#define FLAG_DDR1_CLK_INITIALIZED	0x04000
/* Clock for DDR 2 initialized */
#define FLAG_DDR2_CLK_INITIALIZED	0x08000
/* Clock for DDR 3 initialized */
#define FLAG_DDR3_CLK_INITIALIZED	0x10000
/* Loaded into RAM externally */
#define FLAG_RAM_RESIDENT		0x20000
/* Verbose DDR information */
#define FLAG_DDR_VERBOSE		0x40000
/* Check env. for DDR variables */
#define FLAG_DDR_DEBUG			0x80000
#define FLAG_DDR_TRACE_INIT		0x100000
#define FLAG_MEMORY_PRESERVED		0x200000
#define FLAG_DFM_VERBOSE		0x400000
#define FLAG_DFM_TRACE_INIT		0x800000
/* DFM memory clock initialized */
#define FLAG_DFM_CLK_INITIALIZED	0x1000000
/* EEPROM clock descr. missing */
#define FLAG_CLOCK_DESC_MISSING		0x2000000
/* EEPROM board descr. missing */
#define FLAG_BOARD_DESC_MISSING		0x4000000
#define FLAG_DDR_PROMPT			0x8000000

#ifndef DDR_NO_DEBUG
static inline int ddr_verbose(struct ddr_priv *priv)
{
	return !!(priv->flags & FLAG_DDR_VERBOSE);
}

static inline char *ddr_getenv_debug(struct ddr_priv *priv, char *name)
{
	if (priv->flags & FLAG_FAILSAFE_MODE)
		return NULL;

	if (priv->flags & FLAG_DDR_DEBUG)
		return env_get(name);

	return NULL;
}
#else
static inline int ddr_verbose(void)
{
	return 0;
}
#endif

/* turn the variable name into a string */
#define CVMX_TMP_STR(x) CVMX_TMP_STR2(x)
#define CVMX_TMP_STR2(x) #x

#define CVMX_CACHE(op, address, offset)					\
	asm volatile ("cache " CVMX_TMP_STR(op) ", "			\
		      CVMX_TMP_STR(offset) "(%[rbase])"			\
		      : : [rbase] "d" (address))

/* unlock the state */
#define CVMX_CACHE_WBIL2(address, offset)	\
	CVMX_CACHE(23, address, offset)

/* complete prefetches, invalidate entire dcache */
#define CVMX_DCACHE_INVALIDATE					\
	{ CVMX_SYNC; asm volatile ("cache 9, 0($0)" : : ); }

#define DEBUG_VALIDATE_BITMASK 0
#if DEBUG_VALIDATE_BITMASK
#define debug_bitmask_print printf
#else
#define debug_bitmask_print(...)
#endif

#define RLEVEL_BITMASK_TRAILING_BITS_ERROR      5
// FIXME? now less than TOOLONG
#define RLEVEL_BITMASK_BUBBLE_BITS_ERROR        11
#define RLEVEL_BITMASK_NARROW_ERROR             6
#define RLEVEL_BITMASK_BLANK_ERROR              100
#define RLEVEL_BITMASK_TOOLONG_ERROR            12
#define RLEVEL_NONSEQUENTIAL_DELAY_ERROR        50
#define RLEVEL_ADJACENT_DELAY_ERROR             30

/*
 * Apply a filter to the BITMASK results returned from Octeon
 * read-leveling to determine the most likely delay result.  This
 * computed delay may be used to qualify the delay result returned by
 * Octeon. Accumulate an error penalty for invalid characteristics of
 * the bitmask so that they can be used to select the most reliable
 * results.
 *
 * The algorithm searches for the largest contiguous MASK within a
 * maximum RANGE of bits beginning with the MSB.
 *
 * 1. a MASK with a WIDTH less than 4 will be penalized
 * 2. Bubbles in the bitmask that occur before or after the MASK
 *    will be penalized
 * 3. If there are no trailing bubbles then extra bits that occur
 *    beyond the maximum RANGE will be penalized.
 *
 *   +++++++++++++++++++++++++++++++++++++++++++++++++++
 *   +                                                 +
 *   +   e.g. bitmask = 27B00                          +
 *   +                                                 +
 *   +   63                  +--- mstart           0   +
 *   +   |                   |                     |   +
 *   +   |         +---------+     +--- fb         |   +
 *   +   |         |  range  |     |               |   +
 *   +   V         V         V     V               V   +
 *   +                                                 +
 *   +   0 0 ... 1 0 0 1 1 1 1 0 1 1 0 0 0 0 0 0 0 0   +
 *   +                                                 +
 *   +           ^     ^     ^                         +
 *   +           |     | mask|                         +
 *   +     lb ---+     +-----+                         +
 *   +                  width                          +
 *   +                                                 +
 *   +++++++++++++++++++++++++++++++++++++++++++++++++++
 */

struct rlevel_bitmask {
	u64 bm;
	u8 mstart;
	u8 width;
	int errs;
};

#define MASKRANGE_BITS	6
#define MASKRANGE	((1 << MASKRANGE_BITS) - 1)

/* data field addresses in the DDR2 SPD eeprom */
enum ddr2_spd_addrs {
	DDR2_SPD_BYTES_PROGRAMMED	= 0,
	DDR2_SPD_TOTAL_BYTES		= 1,
	DDR2_SPD_MEM_TYPE		= 2,
	DDR2_SPD_NUM_ROW_BITS		= 3,
	DDR2_SPD_NUM_COL_BITS		= 4,
	DDR2_SPD_NUM_RANKS		= 5,
	DDR2_SPD_CYCLE_CLX		= 9,
	DDR2_SPD_CONFIG_TYPE		= 11,
	DDR2_SPD_REFRESH		= 12,
	DDR2_SPD_SDRAM_WIDTH		= 13,
	DDR2_SPD_BURST_LENGTH		= 16,
	DDR2_SPD_NUM_BANKS		= 17,
	DDR2_SPD_CAS_LATENCY		= 18,
	DDR2_SPD_DIMM_TYPE		= 20,
	DDR2_SPD_CYCLE_CLX1		= 23,
	DDR2_SPD_CYCLE_CLX2		= 25,
	DDR2_SPD_TRP			= 27,
	DDR2_SPD_TRRD			= 28,
	DDR2_SPD_TRCD			= 29,
	DDR2_SPD_TRAS			= 30,
	DDR2_SPD_TWR			= 36,
	DDR2_SPD_TWTR			= 37,
	DDR2_SPD_TRFC_EXT		= 40,
	DDR2_SPD_TRFC			= 42,
	DDR2_SPD_CHECKSUM		= 63,
	DDR2_SPD_MFR_ID			= 64
};

/* data field addresses in the DDR2 SPD eeprom */
enum ddr3_spd_addrs {
	DDR3_SPD_BYTES_PROGRAMMED			=  0,
	DDR3_SPD_REVISION				=  1,
	DDR3_SPD_KEY_BYTE_DEVICE_TYPE			=  2,
	DDR3_SPD_KEY_BYTE_MODULE_TYPE			=  3,
	DDR3_SPD_DENSITY_BANKS				=  4,
	DDR3_SPD_ADDRESSING_ROW_COL_BITS		=  5,
	DDR3_SPD_NOMINAL_VOLTAGE			=  6,
	DDR3_SPD_MODULE_ORGANIZATION			=  7,
	DDR3_SPD_MEMORY_BUS_WIDTH			=  8,
	DDR3_SPD_FINE_TIMEBASE_DIVIDEND_DIVISOR		=  9,
	DDR3_SPD_MEDIUM_TIMEBASE_DIVIDEND		= 10,
	DDR3_SPD_MEDIUM_TIMEBASE_DIVISOR		= 11,
	DDR3_SPD_MINIMUM_CYCLE_TIME_TCKMIN		= 12,
	DDR3_SPD_CAS_LATENCIES_LSB			= 14,
	DDR3_SPD_CAS_LATENCIES_MSB			= 15,
	DDR3_SPD_MIN_CAS_LATENCY_TAAMIN			= 16,
	DDR3_SPD_MIN_WRITE_RECOVERY_TWRMIN		= 17,
	DDR3_SPD_MIN_RAS_CAS_DELAY_TRCDMIN		= 18,
	DDR3_SPD_MIN_ROW_ACTIVE_DELAY_TRRDMIN		= 19,
	DDR3_SPD_MIN_ROW_PRECHARGE_DELAY_TRPMIN		= 20,
	DDR3_SPD_UPPER_NIBBLES_TRAS_TRC			= 21,
	DDR3_SPD_MIN_ACTIVE_PRECHARGE_LSB_TRASMIN	= 22,
	DDR3_SPD_MIN_ACTIVE_REFRESH_LSB_TRCMIN		= 23,
	DDR3_SPD_MIN_REFRESH_RECOVERY_LSB_TRFCMIN	= 24,
	DDR3_SPD_MIN_REFRESH_RECOVERY_MSB_TRFCMIN       = 25,
	DDR3_SPD_MIN_INTERNAL_WRITE_READ_CMD_TWTRMIN    = 26,
	DDR3_SPD_MIN_INTERNAL_READ_PRECHARGE_CMD_TRTPMIN = 27,
	DDR3_SPD_UPPER_NIBBLE_TFAW                      = 28,
	DDR3_SPD_MIN_FOUR_ACTIVE_WINDOW_TFAWMIN         = 29,
	DDR3_SPD_SDRAM_OPTIONAL_FEATURES		= 30,
	DDR3_SPD_SDRAM_THERMAL_REFRESH_OPTIONS		= 31,
	DDR3_SPD_MODULE_THERMAL_SENSOR			= 32,
	DDR3_SPD_SDRAM_DEVICE_TYPE			= 33,
	DDR3_SPD_MINIMUM_CYCLE_TIME_FINE_TCKMIN		= 34,
	DDR3_SPD_MIN_CAS_LATENCY_FINE_TAAMIN		= 35,
	DDR3_SPD_MIN_RAS_CAS_DELAY_FINE_TRCDMIN		= 36,
	DDR3_SPD_MIN_ROW_PRECHARGE_DELAY_FINE_TRPMIN	= 37,
	DDR3_SPD_MIN_ACTIVE_REFRESH_LSB_FINE_TRCMIN	= 38,
	DDR3_SPD_REFERENCE_RAW_CARD                     = 62,
	DDR3_SPD_ADDRESS_MAPPING                        = 63,
	DDR3_SPD_REGISTER_MANUFACTURER_ID_LSB		= 65,
	DDR3_SPD_REGISTER_MANUFACTURER_ID_MSB		= 66,
	DDR3_SPD_REGISTER_REVISION_NUMBER		= 67,
	DDR3_SPD_MODULE_SERIAL_NUMBER                   = 122,
	DDR3_SPD_CYCLICAL_REDUNDANCY_CODE_LOWER_NIBBLE  = 126,
	DDR3_SPD_CYCLICAL_REDUNDANCY_CODE_UPPER_NIBBLE  = 127,
	DDR3_SPD_MODULE_PART_NUMBER                     = 128
};

/* data field addresses in the DDR4 SPD eeprom */
enum ddr4_spd_addrs {
	DDR4_SPD_BYTES_PROGRAMMED			=  0,
	DDR4_SPD_REVISION				=  1,
	DDR4_SPD_KEY_BYTE_DEVICE_TYPE			=  2,
	DDR4_SPD_KEY_BYTE_MODULE_TYPE			=  3,
	DDR4_SPD_DENSITY_BANKS				=  4,
	DDR4_SPD_ADDRESSING_ROW_COL_BITS		=  5,
	DDR4_SPD_PACKAGE_TYPE				=  6,
	DDR4_SPD_OPTIONAL_FEATURES			=  7,
	DDR4_SPD_THERMAL_REFRESH_OPTIONS		=  8,
	DDR4_SPD_OTHER_OPTIONAL_FEATURES		=  9,
	DDR4_SPD_SECONDARY_PACKAGE_TYPE			= 10,
	DDR4_SPD_MODULE_NOMINAL_VOLTAGE			= 11,
	DDR4_SPD_MODULE_ORGANIZATION			= 12,
	DDR4_SPD_MODULE_MEMORY_BUS_WIDTH		= 13,
	DDR4_SPD_MODULE_THERMAL_SENSOR			= 14,
	DDR4_SPD_RESERVED_BYTE15			= 15,
	DDR4_SPD_RESERVED_BYTE16			= 16,
	DDR4_SPD_TIMEBASES				= 17,
	DDR4_SPD_MINIMUM_CYCLE_TIME_TCKAVGMIN		= 18,
	DDR4_SPD_MAXIMUM_CYCLE_TIME_TCKAVGMAX		= 19,
	DDR4_SPD_CAS_LATENCIES_BYTE0			= 20,
	DDR4_SPD_CAS_LATENCIES_BYTE1			= 21,
	DDR4_SPD_CAS_LATENCIES_BYTE2			= 22,
	DDR4_SPD_CAS_LATENCIES_BYTE3			= 23,
	DDR4_SPD_MIN_CAS_LATENCY_TAAMIN			= 24,
	DDR4_SPD_MIN_RAS_CAS_DELAY_TRCDMIN		= 25,
	DDR4_SPD_MIN_ROW_PRECHARGE_DELAY_TRPMIN		= 26,
	DDR4_SPD_UPPER_NIBBLES_TRAS_TRC			= 27,
	DDR4_SPD_MIN_ACTIVE_PRECHARGE_LSB_TRASMIN	= 28,
	DDR4_SPD_MIN_ACTIVE_REFRESH_LSB_TRCMIN		= 29,
	DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC1MIN	= 30,
	DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC1MIN      = 31,
	DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC2MIN	= 32,
	DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC2MIN      = 33,
	DDR4_SPD_MIN_REFRESH_RECOVERY_LSB_TRFC4MIN	= 34,
	DDR4_SPD_MIN_REFRESH_RECOVERY_MSB_TRFC4MIN      = 35,
	DDR4_SPD_MIN_FOUR_ACTIVE_WINDOW_MSN_TFAWMIN     = 36,
	DDR4_SPD_MIN_FOUR_ACTIVE_WINDOW_LSB_TFAWMIN     = 37,
	DDR4_SPD_MIN_ROW_ACTIVE_DELAY_SAME_TRRD_SMIN	= 38,
	DDR4_SPD_MIN_ROW_ACTIVE_DELAY_DIFF_TRRD_LMIN	= 39,
	DDR4_SPD_MIN_CAS_TO_CAS_DELAY_TCCD_LMIN		= 40,
	DDR4_SPD_MIN_CAS_TO_CAS_DELAY_FINE_TCCD_LMIN	= 117,
	DDR4_SPD_MIN_ACT_TO_ACT_DELAY_SAME_FINE_TRRD_LMIN = 118,
	DDR4_SPD_MIN_ACT_TO_ACT_DELAY_DIFF_FINE_TRRD_SMIN = 119,
	DDR4_SPD_MIN_ACT_TO_ACT_REFRESH_DELAY_FINE_TRCMIN = 120,
	DDR4_SPD_MIN_ROW_PRECHARGE_DELAY_FINE_TRPMIN	= 121,
	DDR4_SPD_MIN_RAS_TO_CAS_DELAY_FINE_TRCDMIN	= 122,
	DDR4_SPD_MIN_CAS_LATENCY_FINE_TAAMIN		= 123,
	DDR4_SPD_MAX_CYCLE_TIME_FINE_TCKAVGMAX		= 124,
	DDR4_SPD_MIN_CYCLE_TIME_FINE_TCKAVGMIN		= 125,
	DDR4_SPD_CYCLICAL_REDUNDANCY_CODE_LOWER_NIBBLE  = 126,
	DDR4_SPD_CYCLICAL_REDUNDANCY_CODE_UPPER_NIBBLE  = 127,
	DDR4_SPD_REFERENCE_RAW_CARD			= 130,
	DDR4_SPD_UDIMM_ADDR_MAPPING_FROM_EDGE		= 131,
	DDR4_SPD_REGISTER_MANUFACTURER_ID_LSB		= 133,
	DDR4_SPD_REGISTER_MANUFACTURER_ID_MSB		= 134,
	DDR4_SPD_REGISTER_REVISION_NUMBER		= 135,
	DDR4_SPD_RDIMM_ADDR_MAPPING_FROM_REGISTER_TO_DRAM = 136,
	DDR4_SPD_RDIMM_REGISTER_DRIVE_STRENGTH_CTL	= 137,
	DDR4_SPD_RDIMM_REGISTER_DRIVE_STRENGTH_CK	= 138,
};

#define SPD_EEPROM_SIZE		(DDR4_SPD_RDIMM_REGISTER_DRIVE_STRENGTH_CK + 1)

struct impedence_values {
	unsigned char *rodt_ohms;
	unsigned char *rtt_nom_ohms;
	unsigned char *rtt_nom_table;
	unsigned char *rtt_wr_ohms;
	unsigned char *dic_ohms;
	short *drive_strength;
	short *dqx_strength;
};

#define RODT_OHMS_COUNT        8
#define RTT_NOM_OHMS_COUNT     8
#define RTT_NOM_TABLE_COUNT    8
#define RTT_WR_OHMS_COUNT      8
#define DIC_OHMS_COUNT         3
#define DRIVE_STRENGTH_COUNT  15

/*
 * Structure that provides DIMM information, either in the form of an SPD
 * TWSI address, or a pointer to an array that contains SPD data. One of
 * the two fields must be valid.
 */
struct dimm_config {
	u16 spd_addrs[2]; /* TWSI address of SPD, 0 if not used */
	u8 *spd_ptrs[2]; /* pointer to SPD data array, NULL if not used */
	int spd_cached[2];
	u8 spd_data[2][SPD_EEPROM_SIZE];
};

struct dimm_odt_config {
	u8 odt_ena;            /* FIX: dqx_ctl for Octeon 3 DDR4 */
	u64 odt_mask;          /* FIX: wodt_mask for Octeon 3 */
	union cvmx_lmcx_modereg_params1 modereg_params1;
	union cvmx_lmcx_modereg_params2 modereg_params2;
	u8 qs_dic;             /* FIX: rodt_ctl for Octeon 3 */
	u64 rodt_ctl;          /* FIX: rodt_mask for Octeon 3 */
	u8 dic;
};

struct ddr_delay_config {
	u32 ddr_board_delay;
	u8 lmc_delay_clk;
	u8 lmc_delay_cmd;
	u8 lmc_delay_dq;
};

/*
 * The parameters below make up the custom_lmc_config data structure.
 * This structure is used to customize the way that the LMC DRAM
 * Controller is configured for a particular board design.
 *
 * The HRM describes LMC Read Leveling which supports automatic
 * selection of per byte-lane delays.  When measuring the read delays
 * the LMC configuration software sweeps through a range of settings
 * for LMC0_COMP_CTL2[RODT_CTL], the Octeon II on-die-termination
 * resistance and LMC0_MODEREG_PARAMS1[RTT_NOM_XX], the DRAM
 * on-die-termination resistance.  The minimum and maximum parameters
 * for rtt_nom_idx and rodt_ctl listed below determine the ranges of
 * ODT settings used for the measurements.  Note that for rtt_nom an
 * index is used into a sorted table rather than the direct csr setting
 * in order to optimize the sweep.
 *
 * .min_rtt_nom_idx: 1=120ohms, 2=60ohms, 3=40ohms, 4=30ohms, 5=20ohms
 * .max_rtt_nom_idx: 1=120ohms, 2=60ohms, 3=40ohms, 4=30ohms, 5=20ohms
 * .min_rodt_ctl: 1=20ohms, 2=30ohms, 3=40ohms, 4=60ohms, 5=120ohms
 * .max_rodt_ctl: 1=20ohms, 2=30ohms, 3=40ohms, 4=60ohms, 5=120ohms
 *
 * The settings below control the Octeon II drive strength for the CK,
 * ADD/CMD, and DQ/DQS signals.  1=24ohms, 2=26.67ohms, 3=30ohms,
 * 4=34.3ohms, 5=40ohms, 6=48ohms, 6=60ohms.
 *
 * .dqx_ctl: Drive strength control for DDR_DQX/DDR_DQS_X_P/N drivers.
 * .ck_ctl: Drive strength control for
 * DDR_CK_X_P/DDR_DIMMX_CSX_L/DDR_DIMMX_ODT_X drivers.
 * .cmd_ctl: Drive strength control for CMD/A/RESET_L/CKEX drivers.
 *
 * The LMC controller software selects the most optimal CAS Latency
 * that complies with the appropriate SPD values and the frequency
 * that the DRAMS are being operated.  When operating the DRAMs at
 * frequencies substantially lower than their rated frequencies it
 * might be necessary to limit the minimum CAS Latency the LMC
 * controller software is allowed to select in order to make the DRAM
 * work reliably.
 *
 * .min_cas_latency: Minimum allowed CAS Latency
 *
 * The value used for LMC0_RLEVEL_CTL[OFFSET_EN] determine how the
 * read-leveling information that the Octeon II gathers is interpreted
 * to determine the per-byte read delays.
 *
 * .offset_en: Value used for LMC0_RLEVEL_CTL[OFFSET_EN].
 * .offset_udimm: Value used for LMC0_RLEVEL_CTL[OFFSET] for UDIMMS.
 * .offset_rdimm: Value used for LMC0_RLEVEL_CTL[OFFSET] for RDIMMS.
 *
 * The LMC configuration software sweeps through a range of ODT
 * settings while measuring the per-byte read delays.  During those
 * measurements the software makes an assessment of the quality of the
 * measurements in order to determine which measurements provide the
 * most accurate delays.  The automatic settings provide the option to
 * allow that same assessment to determine the most optimal RODT_CTL
 * and/or RTT_NOM settings.
 *
 * The automatic approach might provide the best means to determine
 * the settings used for initial poweron of a new design.  However,
 * the final settings should be determined by board analysis, testing,
 * and experience.
 *
 * .ddr_rtt_nom_auto: 1 means automatically set RTT_NOM value.
 * .ddr_rodt_ctl_auto: 1 means automatically set RODT_CTL value.
 *
 * .rlevel_compute: Enables software interpretation of per-byte read
 * delays using the measurements collected by the
 * Octeon II rather than completely relying on the
 * Octeon II to determine the delays.  1=software
 * computation is recomended since a more complete
 * analysis is implemented in software.
 *
 * .rlevel_comp_offset: Set to 2 unless instructed differently by Cavium.
 *
 * .rlevel_average_loops: Determines the number of times the read-leveling
 * sequence is run for each rank.  The results is
 * then averaged across the number of loops. The
 * default setting is 1.
 *
 * .ddr2t_udimm:
 * .ddr2t_rdimm: Turn on the DDR 2T mode. 2-cycle window for CMD and
 * address. This mode helps relieve setup time pressure
 * on the address and command bus. Please refer to
 * Micron's tech note tn_47_01 titled DDR2-533 Memory
 * Design Guide for Two Dimm Unbuffered Systems for
 * physical details.
 *
 * .disable_sequential_delay_check: As result of the flyby topology
 * prescribed in the JEDEC specifications the byte delays should
 * maintain a consistent increasing or decreasing trend across
 * the bytes on standard dimms.  This setting can be used disable
 * that check for unusual circumstances where the check is not
 * useful.
 *
 * .maximum_adjacent_rlevel_delay_increment: An additional sequential
 * delay check for the delays that result from the flyby
 * topology. This value specifies the maximum difference between
 * the delays of adjacent bytes.  A value of 0 disables this
 * check.
 *
 * .fprch2 Front Porch Enable: When set, the turn-off
 * time for the default DDR_DQ/DQS drivers is FPRCH2 CKs earlier.
 * 00 = 0 CKs
 * 01 = 1 CKs
 * 10 = 2 CKs
 *
 * .parity: The parity input signal PAR_IN on each dimm must be
 * strapped high or low on the board.  This bit is programmed
 * into LMC0_DIMM_CTL[PARITY] and it must be set to match the
 * board strapping.  This signal is typically strapped low.
 *
 * .mode32b: Enable 32-bit datapath mode.  Set to 1 if only 32 DQ pins
 * are used. (cn61xx, cn71xx)
 *
 * .measured_vref: Set to 1 to measure VREF; set to 0 to compute VREF.
 *
 * .dram_connection: Set to 1 if discrete DRAMs; set to 0 if using DIMMs.
 * This changes the algorithms used to compute VREF.
 *
 * .dll_write_offset: FIXME: Add description
 * .dll_read_offset:  FIXME: Add description
 */

struct rlevel_table {
	const char part[20];
	int speed;
	u64 rl_rank[4][4];
};

struct ddr3_custom_config {
	u8 min_rtt_nom_idx;
	u8 max_rtt_nom_idx;
	u8 min_rodt_ctl;
	u8 max_rodt_ctl;
	u8 dqx_ctl;
	u8 ck_ctl;
	u8 cmd_ctl;
	u8 ctl_ctl;
	u8 min_cas_latency;
	u8 offset_en;
	u8 offset_udimm;
	u8 offset_rdimm;
	u8 rlevel_compute;
	u8 ddr_rtt_nom_auto;
	u8 ddr_rodt_ctl_auto;
	u8 rlevel_comp_offset_udimm;
	u8 rlevel_comp_offset_rdimm;
	int8_t ptune_offset;
	int8_t ntune_offset;
	u8 rlevel_average_loops;
	u8 ddr2t_udimm;
	u8 ddr2t_rdimm;
	u8 disable_sequential_delay_check;
	u8 maximum_adjacent_rlevel_delay_increment;
	u8 parity;
	u8 fprch2;
	u8 mode32b;
	u8 measured_vref;
	u8 dram_connection;
	const int8_t *dll_write_offset;
	const int8_t *dll_read_offset;
	struct rlevel_table *rl_tbl;
};

#define DDR_CFG_T_MAX_DIMMS     5

struct ddr_conf {
	struct dimm_config dimm_config_table[DDR_CFG_T_MAX_DIMMS];
	struct dimm_odt_config odt_1rank_config[4];
	struct dimm_odt_config odt_2rank_config[4];
	struct dimm_odt_config odt_4rank_config[4];
	struct ddr_delay_config unbuffered;
	struct ddr_delay_config registered;
	struct ddr3_custom_config custom_lmc_config;
};

/* Divide and round results to the nearest integer. */
static inline u64 divide_nint(u64 dividend, u64 divisor)
{
	u64 quotent, remainder;

	quotent   = dividend / divisor;
	remainder = dividend % divisor;
	return (quotent + ((remainder * 2) >= divisor));
}

/* Divide and round results up to the next higher integer. */
static inline u64 divide_roundup(u64 dividend, u64 divisor)
{
	return ((dividend + divisor - 1) / divisor);
}

enum ddr_type {
	DDR3_DRAM = 3,
	DDR4_DRAM = 4,
};

#define rttnom_none   0         /* Rtt_Nom disabled */
#define rttnom_60ohm  1         /* RZQ/4  = 240/4  =  60 ohms */
#define rttnom_120ohm 2         /* RZQ/2  = 240/2  = 120 ohms */
#define rttnom_40ohm  3         /* RZQ/6  = 240/6  =  40 ohms */
#define rttnom_20ohm  4         /* RZQ/12 = 240/12 =  20 ohms */
#define rttnom_30ohm  5         /* RZQ/8  = 240/8  =  30 ohms */
#define rttnom_rsrv1  6         /* Reserved */
#define rttnom_rsrv2  7         /* Reserved */

#define rttwr_none    0         /* Dynamic ODT off */
#define rttwr_60ohm   1         /* RZQ/4  = 240/4  =  60 ohms */
#define rttwr_120ohm  2         /* RZQ/2  = 240/2  = 120 ohms */
#define rttwr_rsrv1   3         /* Reserved */

#define dic_40ohm     0         /* RZQ/6  = 240/6  =  40 ohms */
#define dic_34ohm     1         /* RZQ/7  = 240/7  =  34 ohms */

#define driver_24_ohm   1
#define driver_27_ohm   2
#define driver_30_ohm   3
#define driver_34_ohm   4
#define driver_40_ohm   5
#define driver_48_ohm   6
#define driver_60_ohm   7

#define rodt_ctl_none     0
#define rodt_ctl_20_ohm   1
#define rodt_ctl_30_ohm   2
#define rodt_ctl_40_ohm   3
#define rodt_ctl_60_ohm   4
#define rodt_ctl_120_ohm  5

#define ddr4_rttnom_none   0         /* Rtt_Nom disabled */
#define ddr4_rttnom_60ohm  1         /* RZQ/4  = 240/4  =  60 ohms */
#define ddr4_rttnom_120ohm 2         /* RZQ/2  = 240/2  = 120 ohms */
#define ddr4_rttnom_40ohm  3         /* RZQ/6  = 240/6  =  40 ohms */
#define ddr4_rttnom_240ohm 4         /* RZQ/1  = 240/1  = 240 ohms */
#define ddr4_rttnom_48ohm  5         /* RZQ/5  = 240/5  =  48 ohms */
#define ddr4_rttnom_80ohm  6         /* RZQ/3  = 240/3  =  80 ohms */
#define ddr4_rttnom_34ohm  7         /* RZQ/7  = 240/7  =  34 ohms */

#define ddr4_rttwr_none    0         /* Dynamic ODT off */
#define ddr4_rttwr_120ohm  1         /* RZQ/2  = 240/2  = 120 ohms */
#define ddr4_rttwr_240ohm  2         /* RZQ/1  = 240/1  = 240 ohms */
#define ddr4_rttwr_hiz     3         /* HiZ */
/* This setting is available for cn78xx pass 2, and cn73xx & cnf75xx pass 1 */
#define ddr4_rttwr_80ohm   4         /* RZQ/3  = 240/3  =  80 ohms */

#define ddr4_dic_34ohm     0         /* RZQ/7  = 240/7  =  34 ohms */
#define ddr4_dic_48ohm     1         /* RZQ/5  = 240/5  =  48 ohms */

#define ddr4_rttpark_none   0         /* Rtt_Park disabled */
#define ddr4_rttpark_60ohm  1         /* RZQ/4  = 240/4  =  60 ohms */
#define ddr4_rttpark_120ohm 2         /* RZQ/2  = 240/2  = 120 ohms */
#define ddr4_rttpark_40ohm  3         /* RZQ/6  = 240/6  =  40 ohms */
#define ddr4_rttpark_240ohm 4         /* RZQ/1  = 240/1  = 240 ohms */
#define ddr4_rttpark_48ohm  5         /* RZQ/5  = 240/5  =  48 ohms */
#define ddr4_rttpark_80ohm  6         /* RZQ/3  = 240/3  =  80 ohms */
#define ddr4_rttpark_34ohm  7         /* RZQ/7  = 240/7  =  34 ohms */

#define ddr4_driver_26_ohm   2
#define ddr4_driver_30_ohm   3
#define ddr4_driver_34_ohm   4
#define ddr4_driver_40_ohm   5
#define ddr4_driver_48_ohm   6

#define ddr4_dqx_driver_24_ohm   1
#define ddr4_dqx_driver_27_ohm   2
#define ddr4_dqx_driver_30_ohm   3
#define ddr4_dqx_driver_34_ohm   4
#define ddr4_dqx_driver_40_ohm   5
#define ddr4_dqx_driver_48_ohm   6
#define ddr4_dqx_driver_60_ohm   7

#define ddr4_rodt_ctl_none     0
#define ddr4_rodt_ctl_40_ohm   1
#define ddr4_rodt_ctl_60_ohm   2
#define ddr4_rodt_ctl_80_ohm   3
#define ddr4_rodt_ctl_120_ohm  4
#define ddr4_rodt_ctl_240_ohm  5
#define ddr4_rodt_ctl_34_ohm   6
#define ddr4_rodt_ctl_48_ohm   7

#define DIMM_CONFIG_TERMINATOR	{ {0, 0}, {NULL, NULL} }

#define SET_DDR_DLL_CTL3(field, expr)				\
	do {							\
		if (octeon_is_cpuid(OCTEON_CN66XX) ||		\
		    octeon_is_cpuid(OCTEON_CN63XX))		\
			ddr_dll_ctl3.cn63xx.field = (expr);	\
		else if (octeon_is_cpuid(OCTEON_CN68XX) ||      \
			 octeon_is_cpuid(OCTEON_CN61XX) ||      \
			 octeon_is_cpuid(OCTEON_CNF71XX))       \
			ddr_dll_ctl3.cn61xx.field = (expr);	\
		else if (octeon_is_cpuid(OCTEON_CN70XX) ||	\
			 octeon_is_cpuid(OCTEON_CN78XX))        \
			ddr_dll_ctl3.cn70xx.field = (expr);	\
		else if (octeon_is_cpuid(OCTEON_CN73XX) ||	\
			 octeon_is_cpuid(OCTEON_CNF75XX))       \
			ddr_dll_ctl3.cn73xx.field = (expr);	\
		else                                            \
			debug("%s(): " #field			\
			      "not set for unknown chip\n",	\
			      __func__);			\
	} while (0)

#define ENCODE_DLL90_BYTE_SEL(byte_sel)					\
	(octeon_is_cpuid(OCTEON_CN70XX) ? ((9 + 7 - (byte_sel)) % 9) :	\
	 ((byte_sel) + 1))

/**
 * If debugging is disabled the ddr_print macro is not compatible
 * with this macro.
 */
# define GET_DDR_DLL_CTL3(field)		\
	((octeon_is_cpuid(OCTEON_CN66XX) ||	\
	  octeon_is_cpuid(OCTEON_CN63XX)) ?	\
	 ddr_dll_ctl3.cn63xx.field :		\
	 (octeon_is_cpuid(OCTEON_CN68XX) ||	\
	  octeon_is_cpuid(OCTEON_CN61XX) ||	\
	  octeon_is_cpuid(OCTEON_CNF71XX)) ?	\
	 ddr_dll_ctl3.cn61xx.field :		\
	 (octeon_is_cpuid(OCTEON_CN70XX) ||	\
	  octeon_is_cpuid(OCTEON_CN78XX)) ?	\
	 ddr_dll_ctl3.cn70xx.field :		\
	 (octeon_is_cpuid(OCTEON_CN73XX) ||	\
	  octeon_is_cpuid(OCTEON_CNF75XX)) ?	\
	 ddr_dll_ctl3.cn73xx.field : 0)

extern const char *ddr3_dimm_types[];
extern const char *ddr4_dimm_types[];

extern const struct dimm_odt_config disable_odt_config[];

#define RLEVEL_BYTE_BITS	6
#define RLEVEL_BYTE_MSK		((1ULL << 6) - 1)

/* Prototypes */
int get_ddr_type(struct dimm_config *dimm_config, int upper_dimm);
int get_dimm_module_type(struct dimm_config *dimm_config, int upper_dimm,
			 int ddr_type);
int read_spd(struct dimm_config *dimm_config, int dimm_index, int spd_field);
int read_spd_init(struct dimm_config *dimm_config, int dimm_index);
void report_dimm(struct dimm_config *dimm_config, int upper_dimm,
		 int dimm, int if_num);
int validate_dimm(struct ddr_priv *priv, struct dimm_config *dimm_config,
		  int dimm_index);
char *printable_rank_spec(char *buffer, int num_ranks, int dram_width,
			  int spd_package);

bool ddr_memory_preserved(struct ddr_priv *priv);

int get_wl_rank(union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank, int byte);
int get_rl_rank(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank, int byte);
void upd_wl_rank(union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank, int byte,
		 int delay);
void upd_rl_rank(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank, int byte,
		 int delay);

int compute_ddr3_rlevel_delay(u8 mstart, u8 width,
			      union cvmx_lmcx_rlevel_ctl rlevel_ctl);

int encode_row_lsb_ddr3(int row_lsb);
int encode_pbank_lsb_ddr3(int pbank_lsb);

int initialize_ddr_clock(struct ddr_priv *priv, struct ddr_conf *ddr_conf,
			 u32 cpu_hertz, u32 ddr_hertz, u32 ddr_ref_hertz,
			 int if_num, u32 if_mask);

void process_custom_dll_offsets(struct ddr_priv *priv, int if_num,
				const char *enable_str,
				const int8_t *offsets, const char *byte_str,
				int mode);
int nonseq_del(struct rlevel_byte_data *rlevel_byte, int start, int end,
	       int max_adj_delay_inc);
int roundup_ddr3_wlevel_bitmask(int bitmask);

void oct3_ddr3_seq(struct ddr_priv *priv, int rank_mask, int if_num,
		   int sequence);
void ddr_init_seq(struct ddr_priv *priv, int rank_mask, int if_num);

void rlevel_to_wlevel(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank,
		      union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank, int byte);

int validate_ddr3_rlevel_bitmask(struct rlevel_bitmask *rlevel_bitmask_p,
				 int ddr_type);

void change_dll_offset_enable(struct ddr_priv *priv, int if_num, int change);
unsigned short load_dll_offset(struct ddr_priv *priv, int if_num,
			       int dll_offset_mode,
			       int byte_offset, int byte);

u64 lmc_ddr3_rl_dbg_read(struct ddr_priv *priv, int if_num, int idx);
u64 lmc_ddr3_wl_dbg_read(struct ddr_priv *priv, int if_num, int idx);

void cvmx_maybe_tune_node(struct ddr_priv *priv, u32 ddr_speed);
void cvmx_dbi_switchover(struct ddr_priv *priv);

int init_octeon3_ddr3_interface(struct ddr_priv *priv,
				struct ddr_conf *ddr_conf,
				u32 ddr_hertz, u32 cpu_hertz, u32 ddr_ref_hertz,
				int if_num, u32 if_mask);

char *lookup_env(struct ddr_priv *priv, const char *format, ...);
char *lookup_env_ull(struct ddr_priv *priv, const char *format, ...);

/* Each board provides a board-specific config table via this function */
struct ddr_conf *octeon_ddr_conf_table_get(int *count, int *def_ddr_freq);

#endif /* __OCTEON_DDR_H_ */
