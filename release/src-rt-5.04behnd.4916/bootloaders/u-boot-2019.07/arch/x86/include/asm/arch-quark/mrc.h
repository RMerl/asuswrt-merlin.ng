/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#ifndef _MRC_H_
#define _MRC_H_

#define MRC_VERSION	0x0111

/* architectural definitions */
#define NUM_CHANNELS	1	/* number of channels */
#define NUM_RANKS	2	/* number of ranks per channel */
#define NUM_BYTE_LANES	4	/* number of byte lanes per channel */

/* software limitations */
#define MAX_CHANNELS	1
#define MAX_RANKS	2
#define MAX_BYTE_LANES	4

#define MAX_SOCKETS	1
#define MAX_SIDES	1
#define MAX_ROWS	(MAX_SIDES * MAX_SOCKETS)

/* Specify DRAM and channel width */
enum {
	X8,	/* DRAM width */
	X16,	/* DRAM width & Channel Width */
	X32	/* Channel Width */
};

/* Specify DRAM speed */
enum {
	DDRFREQ_800,
	DDRFREQ_1066
};

/* Specify DRAM type */
enum {
	DDR3,
	DDR3L
};

/*
 * density: 0=512Mb, 1=Gb, 2=2Gb, 3=4Gb
 * cl: DRAM CAS Latency in clocks
 * ras: ACT to PRE command period
 * wtr: Delay from start of internal write transaction to internal read command
 * rrd: ACT to ACT command period (JESD79 specific to page size 1K/2K)
 * faw: Four activate window (JESD79 specific to page size 1K/2K)
 *
 * ras/wtr/rrd/faw timings are in picoseconds
 *
 * Refer to JEDEC spec (or DRAM datasheet) when changing these values.
 */
struct dram_params {
	uint8_t density;
	uint8_t cl;
	uint32_t ras;
	uint32_t wtr;
	uint32_t rrd;
	uint32_t faw;
};

/*
 * Delay configuration for individual signals
 * Vref setting
 * Scrambler seed
 */
struct mrc_timings {
	uint32_t rcvn[NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	uint32_t rdqs[NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	uint32_t wdqs[NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	uint32_t wdq[NUM_CHANNELS][NUM_RANKS][NUM_BYTE_LANES];
	uint32_t vref[NUM_CHANNELS][NUM_BYTE_LANES];
	uint32_t wctl[NUM_CHANNELS][NUM_RANKS];
	uint32_t wcmd[NUM_CHANNELS];
	uint32_t scrambler_seed;
	/* need to save for the case of frequency change */
	uint8_t ddr_speed;
};

/* Boot mode defined as bit mask (1<<n) */
enum {
	BM_UNKNOWN,
	BM_COLD = 1,	/* full training */
	BM_FAST = 2,	/* restore timing parameters */
	BM_S3   = 4,	/* resume from S3 */
	BM_WARM = 8
};

/* MRC execution status */
#define MRC_SUCCESS	0	/* initialization ok */
#define MRC_E_MEMTEST	1	/* memtest failed */

/*
 * Memory Reference Code parameters
 *
 * It includes 3 parts:
 * - input parameters like boot mode and DRAM parameters
 * - context parameters for MRC internal state
 * - output parameters like initialization result and memory size
 */
struct mrc_params {
	/* Input parameters */
	uint32_t boot_mode;		/* BM_COLD, BM_FAST, BM_WARM, BM_S3 */
	/* DRAM parameters */
	uint8_t dram_width;		/* x8, x16 */
	uint8_t ddr_speed;		/* DDRFREQ_800, DDRFREQ_1066 */
	uint8_t ddr_type;		/* DDR3, DDR3L */
	uint8_t ecc_enables;		/* 0, 1 (memory size reduced to 7/8) */
	uint8_t scrambling_enables;	/* 0, 1 */
	/* 1, 3 (1'st rank has to be populated if 2'nd rank present) */
	uint32_t rank_enables;
	uint32_t channel_enables;	/* 1 only */
	uint32_t channel_width;		/* x16 only */
	/* 0, 1, 2 (mode 2 forced if ecc enabled) */
	uint32_t address_mode;
	/* REFRESH_RATE: 1=1.95us, 2=3.9us, 3=7.8us, others=RESERVED */
	uint8_t refresh_rate;
	/* SR_TEMP_RANGE: 0=normal, 1=extended, others=RESERVED */
	uint8_t sr_temp_range;
	/*
	 * RON_VALUE: 0=34ohm, 1=40ohm, others=RESERVED
	 * (select MRS1.DIC driver impedance control)
	 */
	uint8_t ron_value;
	/* RTT_NOM_VALUE: 0=40ohm, 1=60ohm, 2=120ohm, others=RESERVED */
	uint8_t rtt_nom_value;
	/* RD_ODT_VALUE: 0=off, 1=60ohm, 2=120ohm, 3=180ohm, others=RESERVED */
	uint8_t rd_odt_value;
	struct dram_params params;
	/* Internally used context parameters */
	uint32_t board_id;	/* board layout (use x8 or x16 memory) */
	uint32_t hte_setup;	/* when set hte reconfiguration requested */
	uint32_t menu_after_mrc;
	uint32_t power_down_disable;
	uint32_t tune_rcvn;
	uint32_t channel_size[NUM_CHANNELS];
	uint32_t column_bits[NUM_CHANNELS];
	uint32_t row_bits[NUM_CHANNELS];
	uint32_t mrs1;		/* register content saved during training */
	uint8_t first_run;
	/* Output parameters */
	/* initialization result (non zero specifies error code) */
	uint32_t status;
	/* total memory size in bytes (excludes ECC banks) */
	uint32_t mem_size;
	/* training results (also used on input) */
	struct mrc_timings timings;
};

/*
 * MRC memory initialization structure
 *
 * post_code: a 16-bit post code of a specific initialization routine
 * boot_path: bitwise or of BM_COLD, BM_FAST, BM_WARM and BM_S3
 * init_fn: real memory initialization routine
 */
struct mem_init {
	uint16_t post_code;
	uint16_t boot_path;
	void (*init_fn)(struct mrc_params *mrc_params);
};

/* MRC platform data flags */
#define MRC_FLAG_ECC_EN		0x00000001
#define MRC_FLAG_SCRAMBLE_EN	0x00000002
#define MRC_FLAG_MEMTEST_EN	0x00000004
/* 0b DDR "fly-by" topology else 1b DDR "tree" topology */
#define MRC_FLAG_TOP_TREE_EN	0x00000008
/* If set ODR signal is asserted to DRAM devices on writes */
#define MRC_FLAG_WR_ODT_EN	0x00000010

/**
 * mrc_init - Memory Reference Code initialization entry routine
 *
 * @mrc_params: parameters for MRC
 */
void mrc_init(struct mrc_params *mrc_params);

#endif /* _MRC_H_ */
