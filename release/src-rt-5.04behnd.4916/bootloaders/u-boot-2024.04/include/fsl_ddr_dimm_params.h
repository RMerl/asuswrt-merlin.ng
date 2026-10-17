/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2008-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 */

#ifndef DDR2_DIMM_PARAMS_H
#define DDR2_DIMM_PARAMS_H

#define EDC_DATA_PARITY	1
#define EDC_ECC		2
#define EDC_AC_PARITY	4

/* Parameters for a DDR dimm computed from the SPD */
typedef struct dimm_params_s {

	/* DIMM organization parameters */
	char mpart[19];		/* guaranteed null terminated */

	unsigned int n_ranks;
	unsigned int die_density;
	unsigned long long rank_density;
	unsigned long long capacity;
	unsigned int data_width;
	unsigned int primary_sdram_width;
	unsigned int ec_sdram_width;
	unsigned int registered_dimm;
	unsigned int package_3ds;	/* number of dies in 3DS DIMM */
	unsigned int device_width;	/* x4, x8, x16 components */

	/* SDRAM device parameters */
	unsigned int n_row_addr;
	unsigned int n_col_addr;
	unsigned int edc_config;	/* 0 = none, 1 = parity, 2 = ECC */
#ifdef CONFIG_SYS_FSL_DDR4
	unsigned int bank_addr_bits;
	unsigned int bank_group_bits;
#else
	unsigned int n_banks_per_sdram_device;
#endif
	unsigned int burst_lengths_bitmask;	/* BL=4 bit 2, BL=8 = bit 3 */

	/* used in computing base address of DIMMs */
	unsigned long long base_address;
	/* mirrored DIMMs */
	unsigned int mirrored_dimm;	/* only for ddr3 */

	/* DIMM timing parameters */

	int mtb_ps;	/* medium timebase ps */
	int ftb_10th_ps; /* fine timebase, in 1/10 ps */
	int taa_ps;	/* minimum CAS latency time */
	int tfaw_ps;	/* four active window delay */

	/*
	 * SDRAM clock periods
	 * The range for these are 1000-10000 so a short should be sufficient
	 */
	int tckmin_x_ps;
	int tckmin_x_minus_1_ps;
	int tckmin_x_minus_2_ps;
	int tckmax_ps;

	/* SPD-defined CAS latencies */
	unsigned int caslat_x;
	unsigned int caslat_x_minus_1;
	unsigned int caslat_x_minus_2;

	unsigned int caslat_lowest_derated;	/* Derated CAS latency */

	/* basic timing parameters */
	int trcd_ps;
	int trp_ps;
	int tras_ps;

#ifdef CONFIG_SYS_FSL_DDR4
	int trfc1_ps;
	int trfc2_ps;
	int trfc4_ps;
	int trrds_ps;
	int trrdl_ps;
	int tccdl_ps;
	int trfc_slr_ps;
#else
	int twr_ps;	/* maximum = 63750 ps */
	int trfc_ps;	/* max = 255 ns + 256 ns + .75 ns
				       = 511750 ps */
	int trrd_ps;	/* maximum = 63750 ps */
	int twtr_ps;	/* maximum = 63750 ps */
	int trtp_ps;	/* byte 38, spd->trtp */
#endif

	int trc_ps;	/* maximum = 254 ns + .75 ns = 254750 ps */

	int refresh_rate_ps;
	int extended_op_srt;

#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
	int tis_ps;	/* byte 32, spd->ca_setup */
	int tih_ps;	/* byte 33, spd->ca_hold */
	int tds_ps;	/* byte 34, spd->data_setup */
	int tdh_ps;	/* byte 35, spd->data_hold */
	int tdqsq_max_ps;	/* byte 44, spd->tdqsq */
	int tqhs_ps;	/* byte 45, spd->tqhs */
#endif

	/* DDR3 & DDR4 RDIMM */
	unsigned char rcw[16];	/* Register Control Word 0-15 */
#ifdef CONFIG_SYS_FSL_DDR4
	unsigned int dq_mapping[18];
	unsigned int dq_mapping_ors;
#endif
} dimm_params_t;

unsigned int ddr_compute_dimm_parameters(const unsigned int ctrl_num,
					 const generic_spd_eeprom_t *spd,
					 dimm_params_t *pdimm,
					 unsigned int dimm_number);

#endif
