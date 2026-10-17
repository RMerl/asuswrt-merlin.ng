// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2021 NXP Semiconductor
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <log.h>
#include <asm/bitops.h>

#include <fsl_ddr.h>

#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
static unsigned int
compute_cas_latency(const unsigned int ctrl_num,
		    const dimm_params_t *dimm_params,
		    common_timing_params_t *outpdimm,
		    unsigned int number_of_dimms)
{
	unsigned int i;
	unsigned int common_caslat;
	unsigned int caslat_actual;
	unsigned int retry = 16;
	unsigned int tmp = ~0;
	unsigned int mclk_ps = get_memory_clk_period_ps(ctrl_num);
#ifdef CONFIG_SYS_FSL_DDR3
	const unsigned int taamax = 20000;
#else
	const unsigned int taamax = 18000;
#endif

	/* compute the common CAS latency supported between slots */
	for (i = 0; i < number_of_dimms; i++) {
		if (dimm_params[i].n_ranks)
			tmp &= dimm_params[i].caslat_x;
	}
	common_caslat = tmp;

	if (!mclk_ps) {
		printf("DDR clock (MCLK cycle was 0 ps), So setting it to slowest DIMM(s) (tCKmin %u ps).\n",
		       outpdimm->tckmin_x_ps);
		mclk_ps = outpdimm->tckmin_x_ps;
	}

	/* validate if the memory clk is in the range of dimms */
	if (mclk_ps < outpdimm->tckmin_x_ps) {
		printf("DDR clock (MCLK cycle %u ps) is faster than "
			"the slowest DIMM(s) (tCKmin %u ps) can support.\n",
			mclk_ps, outpdimm->tckmin_x_ps);
	}
#ifdef CONFIG_SYS_FSL_DDR4
	if (mclk_ps > outpdimm->tckmax_ps) {
		printf("DDR clock (MCLK cycle %u ps) is slower than DIMM(s) (tCKmax %u ps) can support.\n",
		       mclk_ps, outpdimm->tckmax_ps);
	}
#endif
	/* determine the acutal cas latency */
	caslat_actual = (outpdimm->taamin_ps + mclk_ps - 1) / mclk_ps;
	/* check if the dimms support the CAS latency */
	while (!(common_caslat & (1 << caslat_actual)) && retry > 0) {
		caslat_actual++;
		retry--;
	}
	/* once the caculation of caslat_actual is completed
	 * we must verify that this CAS latency value does not
	 * exceed tAAmax, which is 20 ns for all DDR3 speed grades,
	 * 18ns for all DDR4 speed grades.
	 */
	if (caslat_actual * mclk_ps > taamax) {
		printf("The chosen cas latency %d is too large\n",
		       caslat_actual);
	}
	outpdimm->lowest_common_spd_caslat = caslat_actual;
	debug("lowest_common_spd_caslat is 0x%x\n", caslat_actual);

	return 0;
}
#else	/* for DDR1 and DDR2 */
static unsigned int
compute_cas_latency(const unsigned int ctrl_num,
		    const dimm_params_t *dimm_params,
		    common_timing_params_t *outpdimm,
		    unsigned int number_of_dimms)
{
	int i;
	const unsigned int mclk_ps = get_memory_clk_period_ps(ctrl_num);
	unsigned int lowest_good_caslat;
	unsigned int not_ok;
	unsigned int temp1, temp2;

	debug("using mclk_ps = %u\n", mclk_ps);
	if (mclk_ps > outpdimm->tckmax_ps) {
		printf("Warning: DDR clock (%u ps) is slower than DIMM(s) (tCKmax %u ps)\n",
		       mclk_ps, outpdimm->tckmax_ps);
	}

	/*
	 * Compute a CAS latency suitable for all DIMMs
	 *
	 * Strategy for SPD-defined latencies: compute only
	 * CAS latency defined by all DIMMs.
	 */

	/*
	 * Step 1: find CAS latency common to all DIMMs using bitwise
	 * operation.
	 */
	temp1 = 0xFF;
	for (i = 0; i < number_of_dimms; i++) {
		if (dimm_params[i].n_ranks) {
			temp2 = 0;
			temp2 |= 1 << dimm_params[i].caslat_x;
			temp2 |= 1 << dimm_params[i].caslat_x_minus_1;
			temp2 |= 1 << dimm_params[i].caslat_x_minus_2;
			/*
			 * If there was no entry for X-2 (X-1) in
			 * the SPD, then caslat_x_minus_2
			 * (caslat_x_minus_1) contains either 255 or
			 * 0xFFFFFFFF because that's what the glorious
			 * __ilog2 function returns for an input of 0.
			 * On 32-bit PowerPC, left shift counts with bit
			 * 26 set (that the value of 255 or 0xFFFFFFFF
			 * will have), cause the destination register to
			 * be 0.  That is why this works.
			 */
			temp1 &= temp2;
		}
	}

	/*
	 * Step 2: check each common CAS latency against tCK of each
	 * DIMM's SPD.
	 */
	lowest_good_caslat = 0;
	temp2 = 0;
	while (temp1) {
		not_ok = 0;
		temp2 =  __ilog2(temp1);
		debug("checking common caslat = %u\n", temp2);

		/* Check if this CAS latency will work on all DIMMs at tCK. */
		for (i = 0; i < number_of_dimms; i++) {
			if (!dimm_params[i].n_ranks)
				continue;

			if (dimm_params[i].caslat_x == temp2) {
				if (mclk_ps >= dimm_params[i].tckmin_x_ps) {
					debug("CL = %u ok on DIMM %u at tCK=%u ps with tCKmin_X_ps of %u\n",
					      temp2, i, mclk_ps,
					      dimm_params[i].tckmin_x_ps);
					continue;
				} else {
					not_ok++;
				}
			}

			if (dimm_params[i].caslat_x_minus_1 == temp2) {
				unsigned int tckmin_x_minus_1_ps
					= dimm_params[i].tckmin_x_minus_1_ps;
				if (mclk_ps >= tckmin_x_minus_1_ps) {
					debug("CL = %u ok on DIMM %u at tCK=%u ps with tckmin_x_minus_1_ps of %u\n",
					      temp2, i, mclk_ps,
					      tckmin_x_minus_1_ps);
					continue;
				} else {
					not_ok++;
				}
			}

			if (dimm_params[i].caslat_x_minus_2 == temp2) {
				unsigned int tckmin_x_minus_2_ps
					= dimm_params[i].tckmin_x_minus_2_ps;
				if (mclk_ps >= tckmin_x_minus_2_ps) {
					debug("CL = %u ok on DIMM %u at tCK=%u ps with tckmin_x_minus_2_ps of %u\n",
					      temp2, i, mclk_ps,
					      tckmin_x_minus_2_ps);
					continue;
				} else {
					not_ok++;
				}
			}
		}

		if (!not_ok)
			lowest_good_caslat = temp2;

		temp1 &= ~(1 << temp2);
	}

	debug("lowest common SPD-defined CAS latency = %u\n",
	      lowest_good_caslat);
	outpdimm->lowest_common_spd_caslat = lowest_good_caslat;


	/*
	 * Compute a common 'de-rated' CAS latency.
	 *
	 * The strategy here is to find the *highest* dereated cas latency
	 * with the assumption that all of the DIMMs will support a dereated
	 * CAS latency higher than or equal to their lowest dereated value.
	 */
	temp1 = 0;
	for (i = 0; i < number_of_dimms; i++)
		temp1 = max(temp1, dimm_params[i].caslat_lowest_derated);

	outpdimm->highest_common_derated_caslat = temp1;
	debug("highest common dereated CAS latency = %u\n", temp1);

	return 0;
}
#endif

/*
 * compute_lowest_common_dimm_parameters()
 *
 * Determine the worst-case DIMM timing parameters from the set of DIMMs
 * whose parameters have been computed into the array pointed to
 * by dimm_params.
 */
unsigned int
compute_lowest_common_dimm_parameters(const unsigned int ctrl_num,
				      const dimm_params_t *dimm_params,
				      common_timing_params_t *outpdimm,
				      const unsigned int number_of_dimms)
{
	unsigned int i, j;

	unsigned int tckmin_x_ps = 0;
	unsigned int tckmax_ps = 0xFFFFFFFF;
	unsigned int trcd_ps = 0;
	unsigned int trp_ps = 0;
	unsigned int tras_ps = 0;
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
	unsigned int taamin_ps = 0;
#endif
#ifdef CONFIG_SYS_FSL_DDR4
	unsigned int twr_ps = 15000;
	unsigned int trfc1_ps = 0;
	unsigned int trfc2_ps = 0;
	unsigned int trfc4_ps = 0;
	unsigned int trrds_ps = 0;
	unsigned int trrdl_ps = 0;
	unsigned int tccdl_ps = 0;
	unsigned int trfc_slr_ps = 0;
#else
	unsigned int twr_ps = 0;
	unsigned int twtr_ps = 0;
	unsigned int trfc_ps = 0;
	unsigned int trrd_ps = 0;
	unsigned int trtp_ps = 0;
#endif
	unsigned int trc_ps = 0;
	unsigned int refresh_rate_ps = 0;
	unsigned int extended_op_srt = 1;
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
	unsigned int tis_ps = 0;
	unsigned int tih_ps = 0;
	unsigned int tds_ps = 0;
	unsigned int tdh_ps = 0;
	unsigned int tdqsq_max_ps = 0;
	unsigned int tqhs_ps = 0;
#endif
	unsigned int temp1, temp2;
	unsigned int additive_latency = 0;

	temp1 = 0;
	for (i = 0; i < number_of_dimms; i++) {
		/*
		 * If there are no ranks on this DIMM,
		 * it probably doesn't exist, so skip it.
		 */
		if (dimm_params[i].n_ranks == 0) {
			temp1++;
			continue;
		}
		if (dimm_params[i].n_ranks == 4 && i != 0) {
			printf("Found Quad-rank DIMM in wrong bank, ignored."
				" Software may not run as expected.\n");
			temp1++;
			continue;
		}

		/*
		 * check if quad-rank DIMM is plugged if
		 * CONFIG_CHIP_SELECT_QUAD_CAPABLE is not defined
		 * Only the board with proper design is capable
		 */
#ifndef CONFIG_FSL_DDR_FIRST_SLOT_QUAD_CAPABLE
		if (dimm_params[i].n_ranks == 4 && \
		  CONFIG_CHIP_SELECTS_PER_CTRL/CONFIG_DIMM_SLOTS_PER_CTLR < 4) {
			printf("Found Quad-rank DIMM, not able to support.");
			temp1++;
			continue;
		}
#endif
		/*
		 * Find minimum tckmax_ps to find fastest slow speed,
		 * i.e., this is the slowest the whole system can go.
		 */
		tckmax_ps = min(tckmax_ps,
				(unsigned int)dimm_params[i].tckmax_ps);
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
		taamin_ps = max(taamin_ps,
				(unsigned int)dimm_params[i].taa_ps);
#endif
		tckmin_x_ps = max(tckmin_x_ps,
				  (unsigned int)dimm_params[i].tckmin_x_ps);
		trcd_ps = max(trcd_ps, (unsigned int)dimm_params[i].trcd_ps);
		trp_ps = max(trp_ps, (unsigned int)dimm_params[i].trp_ps);
		tras_ps = max(tras_ps, (unsigned int)dimm_params[i].tras_ps);
#ifdef CONFIG_SYS_FSL_DDR4
		trfc1_ps = max(trfc1_ps,
			       (unsigned int)dimm_params[i].trfc1_ps);
		trfc2_ps = max(trfc2_ps,
			       (unsigned int)dimm_params[i].trfc2_ps);
		trfc4_ps = max(trfc4_ps,
			       (unsigned int)dimm_params[i].trfc4_ps);
		trrds_ps = max(trrds_ps,
			       (unsigned int)dimm_params[i].trrds_ps);
		trrdl_ps = max(trrdl_ps,
			       (unsigned int)dimm_params[i].trrdl_ps);
		tccdl_ps = max(tccdl_ps,
			       (unsigned int)dimm_params[i].tccdl_ps);
		trfc_slr_ps = max(trfc_slr_ps,
				  (unsigned int)dimm_params[i].trfc_slr_ps);
#else
		twr_ps = max(twr_ps, (unsigned int)dimm_params[i].twr_ps);
		twtr_ps = max(twtr_ps, (unsigned int)dimm_params[i].twtr_ps);
		trfc_ps = max(trfc_ps, (unsigned int)dimm_params[i].trfc_ps);
		trrd_ps = max(trrd_ps, (unsigned int)dimm_params[i].trrd_ps);
		trtp_ps = max(trtp_ps, (unsigned int)dimm_params[i].trtp_ps);
#endif
		trc_ps = max(trc_ps, (unsigned int)dimm_params[i].trc_ps);
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
		tis_ps = max(tis_ps, (unsigned int)dimm_params[i].tis_ps);
		tih_ps = max(tih_ps, (unsigned int)dimm_params[i].tih_ps);
		tds_ps = max(tds_ps, (unsigned int)dimm_params[i].tds_ps);
		tdh_ps = max(tdh_ps, (unsigned int)dimm_params[i].tdh_ps);
		tqhs_ps = max(tqhs_ps, (unsigned int)dimm_params[i].tqhs_ps);
		/*
		 * Find maximum tdqsq_max_ps to find slowest.
		 *
		 * FIXME: is finding the slowest value the correct
		 * strategy for this parameter?
		 */
		tdqsq_max_ps = max(tdqsq_max_ps,
				   (unsigned int)dimm_params[i].tdqsq_max_ps);
#endif
		refresh_rate_ps = max(refresh_rate_ps,
				      (unsigned int)dimm_params[i].refresh_rate_ps);
		/* extended_op_srt is either 0 or 1, 0 having priority */
		extended_op_srt = min(extended_op_srt,
				      (unsigned int)dimm_params[i].extended_op_srt);
	}

	outpdimm->ndimms_present = number_of_dimms - temp1;

	if (temp1 == number_of_dimms) {
		debug("no dimms this memory controller\n");
		return 0;
	}

	outpdimm->tckmin_x_ps = tckmin_x_ps;
	outpdimm->tckmax_ps = tckmax_ps;
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
	outpdimm->taamin_ps = taamin_ps;
#endif
	outpdimm->trcd_ps = trcd_ps;
	outpdimm->trp_ps = trp_ps;
	outpdimm->tras_ps = tras_ps;
#ifdef CONFIG_SYS_FSL_DDR4
	outpdimm->trfc1_ps = trfc1_ps;
	outpdimm->trfc2_ps = trfc2_ps;
	outpdimm->trfc4_ps = trfc4_ps;
	outpdimm->trrds_ps = trrds_ps;
	outpdimm->trrdl_ps = trrdl_ps;
	outpdimm->tccdl_ps = tccdl_ps;
	outpdimm->trfc_slr_ps = trfc_slr_ps;
#else
	outpdimm->twtr_ps = twtr_ps;
	outpdimm->trfc_ps = trfc_ps;
	outpdimm->trrd_ps = trrd_ps;
	outpdimm->trtp_ps = trtp_ps;
#endif
	outpdimm->twr_ps = twr_ps;
	outpdimm->trc_ps = trc_ps;
	outpdimm->refresh_rate_ps = refresh_rate_ps;
	outpdimm->extended_op_srt = extended_op_srt;
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
	outpdimm->tis_ps = tis_ps;
	outpdimm->tih_ps = tih_ps;
	outpdimm->tds_ps = tds_ps;
	outpdimm->tdh_ps = tdh_ps;
	outpdimm->tdqsq_max_ps = tdqsq_max_ps;
	outpdimm->tqhs_ps = tqhs_ps;
#endif

	/* Determine common burst length for all DIMMs. */
	temp1 = 0xff;
	for (i = 0; i < number_of_dimms; i++) {
		if (dimm_params[i].n_ranks) {
			temp1 &= dimm_params[i].burst_lengths_bitmask;
		}
	}
	outpdimm->all_dimms_burst_lengths_bitmask = temp1;

	/* Determine if all DIMMs registered buffered. */
	temp1 = temp2 = 0;
	for (i = 0; i < number_of_dimms; i++) {
		if (dimm_params[i].n_ranks) {
			if (dimm_params[i].registered_dimm) {
				temp1 = 1;
#ifndef CONFIG_SPL_BUILD
				printf("Detected RDIMM %s\n",
					dimm_params[i].mpart);
#endif
			} else {
				temp2 = 1;
#ifndef CONFIG_SPL_BUILD
				printf("Detected UDIMM %s\n",
					dimm_params[i].mpart);
#endif
			}
#ifndef CONFIG_SPL_BUILD
			puts("       ");
#endif
		}
	}

	outpdimm->all_dimms_registered = 0;
	outpdimm->all_dimms_unbuffered = 0;
	if (temp1 && !temp2) {
		outpdimm->all_dimms_registered = 1;
	} else if (!temp1 && temp2) {
		outpdimm->all_dimms_unbuffered = 1;
	} else {
		printf("ERROR:  Mix of registered buffered and unbuffered "
				"DIMMs detected!\n");
	}

	temp1 = 0;
	if (outpdimm->all_dimms_registered)
		for (j = 0; j < 16; j++) {
			outpdimm->rcw[j] = dimm_params[0].rcw[j];
			for (i = 1; i < number_of_dimms; i++) {
				if (!dimm_params[i].n_ranks)
					continue;
				if (dimm_params[i].rcw[j] != dimm_params[0].rcw[j]) {
					temp1 = 1;
					break;
				}
			}
		}

	if (temp1 != 0)
		printf("ERROR: Mix different RDIMM detected!\n");

	/* calculate cas latency for all DDR types */
	if (compute_cas_latency(ctrl_num, dimm_params,
				outpdimm, number_of_dimms))
		return 1;

	/* Determine if all DIMMs ECC capable. */
	temp1 = 1;
	for (i = 0; i < number_of_dimms; i++) {
		if (dimm_params[i].n_ranks &&
			!(dimm_params[i].edc_config & EDC_ECC)) {
			temp1 = 0;
			break;
		}
	}
	if (temp1) {
		debug("all DIMMs ECC capable\n");
	} else {
		debug("Warning: not all DIMMs ECC capable, cant enable ECC\n");
	}
	outpdimm->all_dimms_ecc_capable = temp1;

	/*
	 * Compute additive latency.
	 *
	 * For DDR1, additive latency should be 0.
	 *
	 * For DDR2, with ODT enabled, use "a value" less than ACTTORW,
	 *	which comes from Trcd, and also note that:
	 *	    add_lat + caslat must be >= 4
	 *
	 * For DDR3, we use the AL=0
	 *
	 * When to use additive latency for DDR2:
	 *
	 * I. Because you are using CL=3 and need to do ODT on writes and
	 *    want functionality.
	 *    1. Are you going to use ODT? (Does your board not have
	 *      additional termination circuitry for DQ, DQS, DQS_,
	 *      DM, RDQS, RDQS_ for x4/x8 configs?)
	 *    2. If so, is your lowest supported CL going to be 3?
	 *    3. If so, then you must set AL=1 because
	 *
	 *       WL >= 3 for ODT on writes
	 *       RL = AL + CL
	 *       WL = RL - 1
	 *       ->
	 *       WL = AL + CL - 1
	 *       AL + CL - 1 >= 3
	 *       AL + CL >= 4
	 *  QED
	 *
	 *  RL >= 3 for ODT on reads
	 *  RL = AL + CL
	 *
	 *  Since CL aren't usually less than 2, AL=0 is a minimum,
	 *  so the WL-derived AL should be the  -- FIXME?
	 *
	 * II. Because you are using auto-precharge globally and want to
	 *     use additive latency (posted CAS) to get more bandwidth.
	 *     1. Are you going to use auto-precharge mode globally?
	 *
	 *        Use addtivie latency and compute AL to be 1 cycle less than
	 *        tRCD, i.e. the READ or WRITE command is in the cycle
	 *        immediately following the ACTIVATE command..
	 *
	 * III. Because you feel like it or want to do some sort of
	 *      degraded-performance experiment.
	 *     1.  Do you just want to use additive latency because you feel
	 *         like it?
	 *
	 * Validation:  AL is less than tRCD, and within the other
	 * read-to-precharge constraints.
	 */

	additive_latency = 0;

#if defined(CONFIG_SYS_FSL_DDR2)
	if ((outpdimm->lowest_common_spd_caslat < 4) &&
	    (picos_to_mclk(ctrl_num, trcd_ps) >
	     outpdimm->lowest_common_spd_caslat)) {
		additive_latency = picos_to_mclk(ctrl_num, trcd_ps) -
				   outpdimm->lowest_common_spd_caslat;
		if (mclk_to_picos(ctrl_num, additive_latency) > trcd_ps) {
			additive_latency = picos_to_mclk(ctrl_num, trcd_ps);
			debug("setting additive_latency to %u because it was "
				" greater than tRCD_ps\n", additive_latency);
		}
	}
#endif

	/*
	 * Validate additive latency
	 *
	 * AL <= tRCD(min)
	 */
	if (mclk_to_picos(ctrl_num, additive_latency) > trcd_ps) {
		printf("Error: invalid additive latency exceeds tRCD(min).\n");
		return 1;
	}

	/*
	 * RL = CL + AL;  RL >= 3 for ODT_RD_CFG to be enabled
	 * WL = RL - 1;  WL >= 3 for ODT_WL_CFG to be enabled
	 * ADD_LAT (the register) must be set to a value less
	 * than ACTTORW if WL = 1, then AL must be set to 1
	 * RD_TO_PRE (the register) must be set to a minimum
	 * tRTP + AL if AL is nonzero
	 */

	/*
	 * Additive latency will be applied only if the memctl option to
	 * use it.
	 */
	outpdimm->additive_latency = additive_latency;

	debug("tCKmin_ps = %u\n", outpdimm->tckmin_x_ps);
	debug("trcd_ps   = %u\n", outpdimm->trcd_ps);
	debug("trp_ps    = %u\n", outpdimm->trp_ps);
	debug("tras_ps   = %u\n", outpdimm->tras_ps);
#ifdef CONFIG_SYS_FSL_DDR4
	debug("trfc1_ps = %u\n", trfc1_ps);
	debug("trfc2_ps = %u\n", trfc2_ps);
	debug("trfc4_ps = %u\n", trfc4_ps);
	debug("trrds_ps = %u\n", trrds_ps);
	debug("trrdl_ps = %u\n", trrdl_ps);
	debug("tccdl_ps = %u\n", tccdl_ps);
	debug("trfc_slr_ps = %u\n", trfc_slr_ps);
#else
	debug("twtr_ps   = %u\n", outpdimm->twtr_ps);
	debug("trfc_ps   = %u\n", outpdimm->trfc_ps);
	debug("trrd_ps   = %u\n", outpdimm->trrd_ps);
#endif
	debug("twr_ps    = %u\n", outpdimm->twr_ps);
	debug("trc_ps    = %u\n", outpdimm->trc_ps);

	return 0;
}
