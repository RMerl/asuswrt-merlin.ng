// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fsl_ddr_sdram.h>

#include <fsl_ddr.h>
/*
 * Calculate the Density of each Physical Rank.
 * Returned size is in bytes.
 *
 * Study these table from Byte 31 of JEDEC SPD Spec.
 *
 *		DDR I	DDR II
 *	Bit	Size	Size
 *	---	-----	------
 *	7 high	512MB	512MB
 *	6	256MB	256MB
 *	5	128MB	128MB
 *	4	 64MB	 16GB
 *	3	 32MB	  8GB
 *	2	 16MB	  4GB
 *	1	  2GB	  2GB
 *	0 low	  1GB	  1GB
 *
 * Reorder Table to be linear by stripping the bottom
 * 2 or 5 bits off and shifting them up to the top.
 *
 */
static unsigned long long
compute_ranksize(unsigned int mem_type, unsigned char row_dens)
{
	unsigned long long bsize;

	/* Bottom 5 bits up to the top. */
	bsize = ((row_dens >> 5) | ((row_dens & 31) << 3));
	bsize <<= 27ULL;
	debug("DDR: DDR II rank density = 0x%16llx\n", bsize);

	return bsize;
}

/*
 * Convert a two-nibble BCD value into a cycle time.
 * While the spec calls for nano-seconds, picos are returned.
 *
 * This implements the tables for bytes 9, 23 and 25 for both
 * DDR I and II.  No allowance for distinguishing the invalid
 * fields absent for DDR I yet present in DDR II is made.
 * (That is, cycle times of .25, .33, .66 and .75 ns are
 * allowed for both DDR II and I.)
 */
static unsigned int
convert_bcd_tenths_to_cycle_time_ps(unsigned int spd_val)
{
	/* Table look up the lower nibble, allow DDR I & II. */
	unsigned int tenths_ps[16] = {
		0,
		100,
		200,
		300,
		400,
		500,
		600,
		700,
		800,
		900,
		250,	/* This and the next 3 entries valid ... */
		330,	/* ...  only for tCK calculations. */
		660,
		750,
		0,	/* undefined */
		0	/* undefined */
	};

	unsigned int whole_ns = (spd_val & 0xF0) >> 4;
	unsigned int tenth_ns = spd_val & 0x0F;
	unsigned int ps = whole_ns * 1000 + tenths_ps[tenth_ns];

	return ps;
}

static unsigned int
convert_bcd_hundredths_to_cycle_time_ps(unsigned int spd_val)
{
	unsigned int tenth_ns = (spd_val & 0xF0) >> 4;
	unsigned int hundredth_ns = spd_val & 0x0F;
	unsigned int ps = tenth_ns * 100 + hundredth_ns * 10;

	return ps;
}

static unsigned int byte40_table_ps[8] = {
	0,
	250,
	330,
	500,
	660,
	750,
	0,	/* supposed to be RFC, but not sure what that means */
	0	/* Undefined */
};

static unsigned int
compute_trfc_ps_from_spd(unsigned char trctrfc_ext, unsigned char trfc)
{
	return (((trctrfc_ext & 0x1) * 256) + trfc) * 1000
		+ byte40_table_ps[(trctrfc_ext >> 1) & 0x7];
}

static unsigned int
compute_trc_ps_from_spd(unsigned char trctrfc_ext, unsigned char trc)
{
	return trc * 1000 + byte40_table_ps[(trctrfc_ext >> 4) & 0x7];
}

/*
 * Determine Refresh Rate.  Ignore self refresh bit on DDR I.
 * Table from SPD Spec, Byte 12, converted to picoseconds and
 * filled in with "default" normal values.
 */
static unsigned int
determine_refresh_rate_ps(const unsigned int spd_refresh)
{
	unsigned int refresh_time_ps[8] = {
		15625000,	/* 0 Normal    1.00x */
		3900000,	/* 1 Reduced    .25x */
		7800000,	/* 2 Extended   .50x */
		31300000,	/* 3 Extended  2.00x */
		62500000,	/* 4 Extended  4.00x */
		125000000,	/* 5 Extended  8.00x */
		15625000,	/* 6 Normal    1.00x  filler */
		15625000,	/* 7 Normal    1.00x  filler */
	};

	return refresh_time_ps[spd_refresh & 0x7];
}

/*
 * The purpose of this function is to compute a suitable
 * CAS latency given the DRAM clock period.  The SPD only
 * defines at most 3 CAS latencies.  Typically the slower in
 * frequency the DIMM runs at, the shorter its CAS latency can.
 * be.  If the DIMM is operating at a sufficiently low frequency,
 * it may be able to run at a CAS latency shorter than the
 * shortest SPD-defined CAS latency.
 *
 * If a CAS latency is not found, 0 is returned.
 *
 * Do this by finding in the standard speed bin table the longest
 * tCKmin that doesn't exceed the value of mclk_ps (tCK).
 *
 * An assumption made is that the SDRAM device allows the
 * CL to be programmed for a value that is lower than those
 * advertised by the SPD.  This is not always the case,
 * as those modes not defined in the SPD are optional.
 *
 * CAS latency de-rating based upon values JEDEC Standard No. 79-2C
 * Table 40, "DDR2 SDRAM stanadard speed bins and tCK, tRCD, tRP, tRAS,
 * and tRC for corresponding bin"
 *
 * ordinal 2, ddr2_speed_bins[1] contains tCK for CL=3
 * Not certain if any good value exists for CL=2
 */
				 /* CL2   CL3   CL4   CL5   CL6  CL7*/
unsigned short ddr2_speed_bins[] = {   0, 5000, 3750, 3000, 2500, 1875 };

unsigned int
compute_derated_DDR2_CAS_latency(unsigned int mclk_ps)
{
	const unsigned int num_speed_bins = ARRAY_SIZE(ddr2_speed_bins);
	unsigned int lowest_tCKmin_found = 0;
	unsigned int lowest_tCKmin_CL = 0;
	unsigned int i;

	debug("mclk_ps = %u\n", mclk_ps);

	for (i = 0; i < num_speed_bins; i++) {
		unsigned int x = ddr2_speed_bins[i];
		debug("i=%u, x = %u, lowest_tCKmin_found = %u\n",
		      i, x, lowest_tCKmin_found);
		if (x && x <= mclk_ps && x >= lowest_tCKmin_found ) {
			lowest_tCKmin_found = x;
			lowest_tCKmin_CL = i + 2;
		}
	}

	debug("lowest_tCKmin_CL = %u\n", lowest_tCKmin_CL);

	return lowest_tCKmin_CL;
}

/*
 * ddr_compute_dimm_parameters for DDR2 SPD
 *
 * Compute DIMM parameters based upon the SPD information in spd.
 * Writes the results to the dimm_params_t structure pointed by pdimm.
 *
 * FIXME: use #define for the retvals
 */
unsigned int ddr_compute_dimm_parameters(const unsigned int ctrl_num,
					 const ddr2_spd_eeprom_t *spd,
					 dimm_params_t *pdimm,
					 unsigned int dimm_number)
{
	unsigned int retval;

	if (spd->mem_type) {
		if (spd->mem_type != SPD_MEMTYPE_DDR2) {
			printf("DIMM %u: is not a DDR2 SPD.\n", dimm_number);
			return 1;
		}
	} else {
		memset(pdimm, 0, sizeof(dimm_params_t));
		return 1;
	}

	retval = ddr2_spd_check(spd);
	if (retval) {
		printf("DIMM %u: failed checksum\n", dimm_number);
		return 2;
	}

	/*
	 * The part name in ASCII in the SPD EEPROM is not null terminated.
	 * Guarantee null termination here by presetting all bytes to 0
	 * and copying the part name in ASCII from the SPD onto it
	 */
	memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
	memcpy(pdimm->mpart, spd->mpart, sizeof(pdimm->mpart) - 1);

	/* DIMM organization parameters */
	pdimm->n_ranks = (spd->mod_ranks & 0x7) + 1;
	pdimm->rank_density = compute_ranksize(spd->mem_type, spd->rank_dens);
	pdimm->capacity = pdimm->n_ranks * pdimm->rank_density;
	pdimm->data_width = spd->dataw;
	pdimm->primary_sdram_width = spd->primw;
	pdimm->ec_sdram_width = spd->ecw;

	/* These are all the types defined by the JEDEC DDR2 SPD 1.3 spec */
	switch (spd->dimm_type) {
	case DDR2_SPD_DIMMTYPE_RDIMM:
	case DDR2_SPD_DIMMTYPE_72B_SO_RDIMM:
	case DDR2_SPD_DIMMTYPE_MINI_RDIMM:
		/* Registered/buffered DIMMs */
		pdimm->registered_dimm = 1;
		break;

	case DDR2_SPD_DIMMTYPE_UDIMM:
	case DDR2_SPD_DIMMTYPE_SO_DIMM:
	case DDR2_SPD_DIMMTYPE_MICRO_DIMM:
	case DDR2_SPD_DIMMTYPE_MINI_UDIMM:
		/* Unbuffered DIMMs */
		pdimm->registered_dimm = 0;
		break;

	case DDR2_SPD_DIMMTYPE_72B_SO_CDIMM:
	default:
		printf("unknown dimm_type 0x%02X\n", spd->dimm_type);
		return 1;
	}

	/* SDRAM device parameters */
	pdimm->n_row_addr = spd->nrow_addr;
	pdimm->n_col_addr = spd->ncol_addr;
	pdimm->n_banks_per_sdram_device = spd->nbanks;
	pdimm->edc_config = spd->config;
	pdimm->burst_lengths_bitmask = spd->burstl;

	/*
	 * Calculate the Maximum Data Rate based on the Minimum Cycle time.
	 * The SPD clk_cycle field (tCKmin) is measured in tenths of
	 * nanoseconds and represented as BCD.
	 */
	pdimm->tckmin_x_ps
		= convert_bcd_tenths_to_cycle_time_ps(spd->clk_cycle);
	pdimm->tckmin_x_minus_1_ps
		= convert_bcd_tenths_to_cycle_time_ps(spd->clk_cycle2);
	pdimm->tckmin_x_minus_2_ps
		= convert_bcd_tenths_to_cycle_time_ps(spd->clk_cycle3);

	pdimm->tckmax_ps = convert_bcd_tenths_to_cycle_time_ps(spd->tckmax);

	/*
	 * Compute CAS latencies defined by SPD
	 * The SPD caslat_x should have at least 1 and at most 3 bits set.
	 *
	 * If cas_lat after masking is 0, the __ilog2 function returns
	 * 255 into the variable.   This behavior is abused once.
	 */
	pdimm->caslat_x  = __ilog2(spd->cas_lat);
	pdimm->caslat_x_minus_1 = __ilog2(spd->cas_lat
					  & ~(1 << pdimm->caslat_x));
	pdimm->caslat_x_minus_2 = __ilog2(spd->cas_lat
					  & ~(1 << pdimm->caslat_x)
					  & ~(1 << pdimm->caslat_x_minus_1));

	/* Compute CAS latencies below that defined by SPD */
	pdimm->caslat_lowest_derated = compute_derated_DDR2_CAS_latency(
					get_memory_clk_period_ps(ctrl_num));

	/* Compute timing parameters */
	pdimm->trcd_ps = spd->trcd * 250;
	pdimm->trp_ps = spd->trp * 250;
	pdimm->tras_ps = spd->tras * 1000;

	pdimm->twr_ps = spd->twr * 250;
	pdimm->twtr_ps = spd->twtr * 250;
	pdimm->trfc_ps = compute_trfc_ps_from_spd(spd->trctrfc_ext, spd->trfc);

	pdimm->trrd_ps = spd->trrd * 250;
	pdimm->trc_ps = compute_trc_ps_from_spd(spd->trctrfc_ext, spd->trc);

	pdimm->refresh_rate_ps = determine_refresh_rate_ps(spd->refresh);

	pdimm->tis_ps = convert_bcd_hundredths_to_cycle_time_ps(spd->ca_setup);
	pdimm->tih_ps = convert_bcd_hundredths_to_cycle_time_ps(spd->ca_hold);
	pdimm->tds_ps
		= convert_bcd_hundredths_to_cycle_time_ps(spd->data_setup);
	pdimm->tdh_ps
		= convert_bcd_hundredths_to_cycle_time_ps(spd->data_hold);

	pdimm->trtp_ps = spd->trtp * 250;
	pdimm->tdqsq_max_ps = spd->tdqsq * 10;
	pdimm->tqhs_ps = spd->tqhs * 10;

	return 0;
}
