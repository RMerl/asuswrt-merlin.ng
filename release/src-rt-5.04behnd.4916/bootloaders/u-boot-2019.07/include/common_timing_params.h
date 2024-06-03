/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

#ifndef COMMON_TIMING_PARAMS_H
#define COMMON_TIMING_PARAMS_H

typedef struct {
	/* parameters to constrict */

	unsigned int tckmin_x_ps;
	unsigned int tckmax_ps;
	unsigned int trcd_ps;
	unsigned int trp_ps;
	unsigned int tras_ps;
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
	unsigned int taamin_ps;
#endif

#ifdef CONFIG_SYS_FSL_DDR4
	unsigned int trfc1_ps;
	unsigned int trfc2_ps;
	unsigned int trfc4_ps;
	unsigned int trrds_ps;
	unsigned int trrdl_ps;
	unsigned int tccdl_ps;
	unsigned int trfc_slr_ps;
#else
	unsigned int twtr_ps;	/* maximum = 63750 ps */
	unsigned int trfc_ps;	/* maximum = 255 ns + 256 ns + .75 ns
					   = 511750 ps */

	unsigned int trrd_ps;	/* maximum = 63750 ps */
	unsigned int trtp_ps;	/* byte 38, spd->trtp */
#endif
	unsigned int twr_ps;	/* maximum = 63750 ps */
	unsigned int trc_ps;	/* maximum = 254 ns + .75 ns = 254750 ps */

	unsigned int refresh_rate_ps;
	unsigned int extended_op_srt;

#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
	unsigned int tis_ps;	/* byte 32, spd->ca_setup */
	unsigned int tih_ps;	/* byte 33, spd->ca_hold */
	unsigned int tds_ps;	/* byte 34, spd->data_setup */
	unsigned int tdh_ps;	/* byte 35, spd->data_hold */
	unsigned int tdqsq_max_ps;	/* byte 44, spd->tdqsq */
	unsigned int tqhs_ps;	/* byte 45, spd->tqhs */
#endif

	unsigned int ndimms_present;
	unsigned int lowest_common_spd_caslat;
	unsigned int highest_common_derated_caslat;
	unsigned int additive_latency;
	unsigned int all_dimms_burst_lengths_bitmask;
	unsigned int all_dimms_registered;
	unsigned int all_dimms_unbuffered;
	unsigned int all_dimms_ecc_capable;

	unsigned long long total_mem;
	unsigned long long base_address;

	/* DDR3 RDIMM */
	unsigned char rcw[16];	/* Register Control Word 0-15 */
} common_timing_params_t;

#endif
