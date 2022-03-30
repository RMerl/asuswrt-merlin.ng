// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <console.h>
#include <clk.h>
#include <ram.h>
#include <reset.h>
#include <asm/io.h>

#include "stm32mp1_ddr_regs.h"
#include "stm32mp1_ddr.h"
#include "stm32mp1_tests.h"

#define MAX_DQS_PHASE_IDX _144deg
#define MAX_DQS_UNIT_IDX 7
#define MAX_GSL_IDX 5
#define MAX_GPS_IDX 3

/* Number of bytes used in this SW. ( min 1--> max 4). */
#define NUM_BYTES 4

enum dqs_phase_enum {
	_36deg = 0,
	_54deg = 1,
	_72deg = 2,
	_90deg = 3,
	_108deg = 4,
	_126deg = 5,
	_144deg = 6
};

/* BIST Result struct */
struct BIST_result {
	/* Overall test result:
	 * 0 Fail (any bit failed) ,
	 * 1 Success (All bits success)
	 */
	bool test_result;
	/* 1: true, all fail /  0: False, not all bits fail */
	bool all_bits_fail;
	bool bit_i_test_result[8];  /* 0 fail / 1 success */
};

/* a struct that defines tuning parameters of a byte. */
struct tuning_position {
	u8 phase; /* DQS phase */
	u8 unit; /* DQS unit delay */
	u32 bits_delay; /* Bits deskew in this byte */
};

/* 36deg, 54deg, 72deg, 90deg, 108deg, 126deg, 144deg */
const u8 dx_dll_phase[7] = {3, 2, 1, 0, 14, 13, 12};

static u8 BIST_error_max = 1;
static u32 BIST_seed = 0x1234ABCD;

static u8 get_nb_bytes(struct stm32mp1_ddrctl *ctl)
{
	u32 data_bus = readl(&ctl->mstr) & DDRCTRL_MSTR_DATA_BUS_WIDTH_MASK;
	u8 nb_bytes = NUM_BYTES;

	switch (data_bus) {
	case DDRCTRL_MSTR_DATA_BUS_WIDTH_HALF:
		nb_bytes /= 2;
		break;
	case DDRCTRL_MSTR_DATA_BUS_WIDTH_QUARTER:
		nb_bytes /= 4;
		break;
	default:
		break;
	}

	return nb_bytes;
}

static void itm_soft_reset(struct stm32mp1_ddrphy *phy)
{
	stm32mp1_ddrphy_init(phy, DDRPHYC_PIR_ITMSRST);
}

/* Read DQ unit delay register and provides the retrieved value for DQS
 * We are assuming that we have the same delay when clocking
 * by DQS and when clocking by DQSN
 */
static u8 DQ_unit_index(struct stm32mp1_ddrphy *phy, u8 byte, u8 bit)
{
	u32 index;
	u32 addr = DXNDQTR(phy, byte);

	/* We are assuming that we have the same delay when clocking by DQS
	 * and when clocking by DQSN : use only the low bits
	 */
	index = (readl(addr) >> DDRPHYC_DXNDQTR_DQDLY_SHIFT(bit))
		& DDRPHYC_DXNDQTR_DQDLY_LOW_MASK;

	pr_debug("%s: [%x]: %x => DQ unit index = %x\n",
		 __func__, addr, readl(addr), index);

	return index;
}

/* Sets the DQS phase delay for a byte lane.
 *phase delay is specified by giving the index of the desired delay
 * in the dx_dll_phase array.
 */
static void DQS_phase_delay(struct stm32mp1_ddrphy *phy, u8 byte, u8 phase_idx)
{
	u8 sdphase_val = 0;

	/*	Write DXNDLLCR.SDPHASE = dx_dll_phase(phase_index); */
	sdphase_val = dx_dll_phase[phase_idx];
	clrsetbits_le32(DXNDLLCR(phy, byte),
			DDRPHYC_DXNDLLCR_SDPHASE_MASK,
			sdphase_val << DDRPHYC_DXNDLLCR_SDPHASE_SHIFT);
}

/* Sets the DQS unit delay for a byte lane.
 * unit delay is specified by giving the index of the desired delay
 * for dgsdly and dqsndly (same value).
 */
static void DQS_unit_delay(struct stm32mp1_ddrphy *phy,
			   u8 byte, u8 unit_dly_idx)
{
	/* Write the same value in DXNDQSTR.DQSDLY and DXNDQSTR.DQSNDLY */
	clrsetbits_le32(DXNDQSTR(phy, byte),
			DDRPHYC_DXNDQSTR_DQSDLY_MASK |
			DDRPHYC_DXNDQSTR_DQSNDLY_MASK,
			(unit_dly_idx << DDRPHYC_DXNDQSTR_DQSDLY_SHIFT) |
			(unit_dly_idx << DDRPHYC_DXNDQSTR_DQSNDLY_SHIFT));

	/* After changing this value, an ITM soft reset (PIR.ITMSRST=1,
	 * plus PIR.INIT=1) must be issued.
	 */
	stm32mp1_ddrphy_init(phy, DDRPHYC_PIR_ITMSRST);
}

/* Sets the DQ unit delay for a bit line in particular byte lane.
 * unit delay is specified by giving the desired delay
 */
static void set_DQ_unit_delay(struct stm32mp1_ddrphy *phy,
			      u8 byte, u8 bit,
			      u8 dq_delay_index)
{
	u8 dq_bit_delay_val = dq_delay_index | (dq_delay_index << 2);

	/* same value on delay for clock DQ an DQS_b */
	clrsetbits_le32(DXNDQTR(phy, byte),
			DDRPHYC_DXNDQTR_DQDLY_MASK
			<< DDRPHYC_DXNDQTR_DQDLY_SHIFT(bit),
			dq_bit_delay_val << DDRPHYC_DXNDQTR_DQDLY_SHIFT(bit));
}

static void set_r0dgsl_delay(struct stm32mp1_ddrphy *phy,
			     u8 byte, u8 r0dgsl_idx)
{
	clrsetbits_le32(DXNDQSTR(phy, byte),
			DDRPHYC_DXNDQSTR_R0DGSL_MASK,
			r0dgsl_idx << DDRPHYC_DXNDQSTR_R0DGSL_SHIFT);
}

static void set_r0dgps_delay(struct stm32mp1_ddrphy *phy,
			     u8 byte, u8 r0dgps_idx)
{
	clrsetbits_le32(DXNDQSTR(phy, byte),
			DDRPHYC_DXNDQSTR_R0DGPS_MASK,
			r0dgps_idx << DDRPHYC_DXNDQSTR_R0DGPS_SHIFT);
}

/* Basic BIST configuration for data lane tests. */
static void config_BIST(struct stm32mp1_ddrphy *phy)
{
	/* Selects the SDRAM bank address to be used during BIST. */
	u32 bbank = 0;
	/* Selects the SDRAM row address to be used during BIST. */
	u32 brow = 0;
	/* Selects the SDRAM column address to be used during BIST. */
	u32 bcol = 0;
	/* Selects the value by which the SDRAM address is incremented
	 * for each write/read access.
	 */
	u32 bainc = 0x00000008;
	/* Specifies the maximum SDRAM rank to be used during BIST.
	 * The default value is set to maximum ranks minus 1.
	 * must be 0 with single rank
	 */
	u32 bmrank = 0;
	/* Selects the SDRAM rank to be used during BIST.
	 * must be 0 with single rank
	 */
	u32 brank = 0;
	/* Specifies the maximum SDRAM bank address to be used during
	 * BIST before the address & increments to the next rank.
	 */
	u32 bmbank = 1;
	/* Specifies the maximum SDRAM row address to be used during
	 * BIST before the address & increments to the next bank.
	 */
	u32 bmrow = 0x7FFF; /* To check */
	/* Specifies the maximum SDRAM column address to be used during
	 * BIST before the address & increments to the next row.
	 */
	u32 bmcol = 0x3FF;  /* To check */
	u32 bmode_conf = 0x00000001;  /* DRam mode */
	u32 bdxen_conf = 0x00000001;  /* BIST on Data byte */
	u32 bdpat_conf = 0x00000002;  /* Select LFSR pattern */

	/*Setup BIST for DRAM mode,  and LFSR-random data pattern.*/
	/*Write BISTRR.BMODE = 1?b1;*/
	/*Write BISTRR.BDXEN = 1?b1;*/
	/*Write BISTRR.BDPAT = 2?b10;*/

	/* reset BIST */
	writel(0x3, &phy->bistrr);

	writel((bmode_conf << 3) | (bdxen_conf << 14) | (bdpat_conf << 17),
	       &phy->bistrr);

	/*Setup BIST Word Count*/
	/*Write BISTWCR.BWCNT = 16?b0008;*/
	writel(0x00000200, &phy->bistwcr); /* A multiple of BL/2 */

	writel(bcol | (brow << 12) | (bbank << 28), &phy->bistar0);
	writel(brank | (bmrank << 2) | (bainc << 4), &phy->bistar1);

	/* To check this line : */
	writel(bmcol | (bmrow << 12) | (bmbank << 28), &phy->bistar2);
}

/* Select the Byte lane to be tested by BIST. */
static void BIST_datx8_sel(struct stm32mp1_ddrphy *phy, u8 datx8)
{
	clrsetbits_le32(&phy->bistrr,
			DDRPHYC_BISTRR_BDXSEL_MASK,
			datx8 << DDRPHYC_BISTRR_BDXSEL_SHIFT);

	/*(For example, selecting Byte Lane 3, BISTRR.BDXSEL = 4?b0011)*/
	/* Write BISTRR.BDXSEL = datx8; */
}

/* Perform BIST Write_Read test on a byte lane and return test result. */
static void BIST_test(struct stm32mp1_ddrphy *phy, u8 byte,
		      struct BIST_result *bist)
{
	bool result = true; /* BIST_SUCCESS */
	u32 cnt = 0;
	u32 error = 0;

	bist->test_result = true;

run:
	itm_soft_reset(phy);

	/*Perform BIST Reset*/
	/* Write BISTRR.BINST = 3?b011; */
	clrsetbits_le32(&phy->bistrr,
			0x00000007,
			0x00000003);

	/*Re-seed LFSR*/
	/* Write BISTLSR.SEED = 32'h1234ABCD; */
	if (BIST_seed)
		writel(BIST_seed, &phy->bistlsr);
	else
		writel(rand(), &phy->bistlsr);

	/* some delay to reset BIST */
	mdelay(1);

	/*Perform BIST Run*/
	clrsetbits_le32(&phy->bistrr,
			0x00000007,
			0x00000001);
	/* Write BISTRR.BINST = 3?b001; */

	/* Wait for a number of CTL clocks before reading BIST register*/
	/* Wait 300 ctl_clk cycles;  ... IS it really needed?? */
	/* Perform BIST Instruction Stop*/
	/* Write BISTRR.BINST = 3?b010;*/

	/* poll on BISTGSR.BDONE. If 0, wait.  ++TODO Add timeout */
	while (!(readl(&phy->bistgsr) & DDRPHYC_BISTGSR_BDDONE))
		;

	/*Check if received correct number of words*/
	/* if (Read BISTWCSR.DXWCNT = Read BISTWCR.BWCNT) */
	if (((readl(&phy->bistwcsr)) >> DDRPHYC_BISTWCSR_DXWCNT_SHIFT) ==
	    readl(&phy->bistwcr)) {
		/*Determine if there is a data comparison error*/
		/* if (Read BISTGSR.BDXERR = 1?b0) */
		if (readl(&phy->bistgsr) & DDRPHYC_BISTGSR_BDXERR)
			result = false; /* BIST_FAIL; */
		else
			result = true; /* BIST_SUCCESS; */
	} else {
		result = false; /* BIST_FAIL; */
	}

	/* loop while success */
	cnt++;
	if (result && cnt != 1000)
		goto run;

	if (!result)
		error++;

	if (error < BIST_error_max) {
		if (cnt != 1000)
			goto run;
		bist->test_result = true;
	} else {
		bist->test_result = false;
	}
}

/* After running the deskew algo, this function applies the new DQ delays
 * by reading them from the array "deskew_delay"and writing in PHY registers.
 * The bits that are not deskewed parfectly (too much skew on them,
 * or data eye very wide) are marked in the array deskew_non_converge.
 */
static void apply_deskew_results(struct stm32mp1_ddrphy *phy, u8 byte,
				 u8 deskew_delay[NUM_BYTES][8],
				 u8 deskew_non_converge[NUM_BYTES][8])
{
	u8  bit_i;
	u8  index;

	for (bit_i = 0; bit_i < 8; bit_i++) {
		set_DQ_unit_delay(phy, byte, bit_i, deskew_delay[byte][bit_i]);
		index = DQ_unit_index(phy, byte, bit_i);
		pr_debug("Byte %d ; bit %d : The new DQ delay (%d) index=%d [delta=%d, 3 is the default]",
			 byte, bit_i, deskew_delay[byte][bit_i],
			 index, index - 3);
		printf("Byte %d, bit %d, DQ delay = %d",
		       byte, bit_i, deskew_delay[byte][bit_i]);
		if (deskew_non_converge[byte][bit_i] == 1)
			pr_debug(" - not converged : still more skew");
		printf("\n");
	}
}

/* DQ Bit de-skew algorithm.
 * Deskews data lines as much as possible.
 * 1. Add delay to DQS line until finding the failure
 *    (normally a hold time violation)
 * 2. Reduce DQS line by small steps until finding the very first time
 *    we go back to "Pass" condition.
 * 3. For each DQ line, Reduce DQ delay until finding the very first failure
 *    (normally a hold time fail)
 * 4. When all bits are at their first failure delay, we can consider them
 *    aligned.
 * Handle conrer situation (Can't find Pass-fail, or fail-pass transitions
 * at any step)
 * TODO Provide a return Status. Improve doc
 */
static enum test_result bit_deskew(struct stm32mp1_ddrctl *ctl,
				   struct stm32mp1_ddrphy *phy, char *string)
{
	/* New DQ delay value (index), set during Deskew algo */
	u8 deskew_delay[NUM_BYTES][8];
	/*If there is still skew on a bit, mark this bit. */
	u8 deskew_non_converge[NUM_BYTES][8];
	struct BIST_result result;
	s8 dqs_unit_delay_index = 0;
	u8 datx8 = 0;
	u8 bit_i = 0;
	s8 phase_idx = 0;
	s8 bit_i_delay_index = 0;
	u8 success = 0;
	struct tuning_position last_right_ok;
	u8 force_stop = 0;
	u8 fail_found;
	u8 error = 0;
	u8 nb_bytes = get_nb_bytes(ctl);
	/* u8 last_pass_dqs_unit = 0; */

	memset(deskew_delay, 0, sizeof(deskew_delay));
	memset(deskew_non_converge, 0, sizeof(deskew_non_converge));

	/*Disable DQS Drift Compensation*/
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_DFTCMP);
	/*Disable all bytes*/
	/* Disable automatic power down of DLL and IOs when disabling
	 * a byte (To avoid having to add programming and  delay
	 * for a DLL re-lock when later re-enabling a disabled Byte Lane)
	 */
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_PDDISDX);

	/* Disable all data bytes */
	clrbits_le32(&phy->dx0gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx1gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx2gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx3gcr, DDRPHYC_DXNGCR_DXEN);

	/* Config the BIST block */
	config_BIST(phy);
	pr_debug("BIST Config done.\n");

	/* Train each byte */
	for (datx8 = 0; datx8 < nb_bytes; datx8++) {
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				datx8 + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		pr_debug("\n======================\n");
		pr_debug("Start deskew byte %d .\n", datx8);
		pr_debug("======================\n");
		/* Enable Byte (DXNGCR, bit DXEN) */
		setbits_le32(DXNGCR(phy, datx8), DDRPHYC_DXNGCR_DXEN);

		/* Select the byte lane for comparison of read data */
		BIST_datx8_sel(phy, datx8);

		/* Set all DQDLYn to maximum value. All bits within the byte
		 * will be delayed with DQSTR = 2 instead of max = 3
		 * to avoid inter bits fail influence
		 */
		writel(0xAAAAAAAA, DXNDQTR(phy, datx8));

		/* Set the DQS phase delay to 90 DEG (default).
		 * What is defined here is the index of the desired config
		 * in the PHASE array.
		 */
		phase_idx = _90deg;

		/* Set DQS unit delay to the max value. */
		dqs_unit_delay_index = MAX_DQS_UNIT_IDX;
		DQS_unit_delay(phy, datx8, dqs_unit_delay_index);
		DQS_phase_delay(phy, datx8, phase_idx);

		/* Issue a DLL soft reset */
		clrbits_le32(DXNDLLCR(phy, datx8), DDRPHYC_DXNDLLCR_DLLSRST);
		setbits_le32(DXNDLLCR(phy, datx8), DDRPHYC_DXNDLLCR_DLLSRST);

		/* Test this typical init condition */
		BIST_test(phy, datx8, &result);
		success = result.test_result;

		/* If the test pass in this typical condition,
		 * start the algo with it.
		 * Else, look for Pass init condition
		 */
		if (!success) {
			pr_debug("Fail at init condtion. Let's look for a good init condition.\n");
			success = 0; /* init */
			/* Make sure we start with a PASS condition before
			 * looking for a fail condition.
			 * Find the first PASS PHASE condition
			 */

			/* escape if we find a PASS */
			pr_debug("increase Phase idx\n");
			while (!success && (phase_idx <= MAX_DQS_PHASE_IDX)) {
				DQS_phase_delay(phy, datx8, phase_idx);
				BIST_test(phy, datx8, &result);
				success = result.test_result;
				phase_idx++;
			}
			/* if ended with success
			 * ==>> Restore the fist success condition
			 */
			if (success)
				phase_idx--; /* because it ended with ++ */
		}
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				datx8 + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		/* We couldn't find a successful condition, its seems
		 * we have hold violation, lets try reduce DQS_unit Delay
		 */
		if (!success) {
			/* We couldn't find a successful condition, its seems
			 * we have hold violation, lets try reduce DQS_unit
			 * Delay
			 */
			pr_debug("Still fail. Try decrease DQS Unit delay\n");

			phase_idx = 0;
			dqs_unit_delay_index = 0;
			DQS_phase_delay(phy, datx8, phase_idx);

			/* escape if we find a PASS */
			while (!success &&
			       (dqs_unit_delay_index <=
				MAX_DQS_UNIT_IDX)) {
				DQS_unit_delay(phy, datx8,
					       dqs_unit_delay_index);
				BIST_test(phy, datx8, &result);
				success = result.test_result;
				dqs_unit_delay_index++;
			}
			if (success) {
				/* Restore the first success condition*/
				dqs_unit_delay_index--;
				/* last_pass_dqs_unit = dqs_unit_delay_index;*/
				DQS_unit_delay(phy, datx8,
					       dqs_unit_delay_index);
			} else {
				/* No need to continue,
				 * there is no pass region.
				 */
				force_stop = 1;
			}
		}

		/* There is an initial PASS condition
		 * Look for the first failing condition by PHASE stepping.
		 * This part of the algo can finish without converging.
		 */
		if (force_stop) {
			printf("Result: Failed ");
			printf("[Cannot Deskew lines, ");
			printf("there is no PASS region]\n");
			error++;
			continue;
		}
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				datx8 + 1, nb_bytes, error);
			return TEST_FAILED;
		}

		pr_debug("there is a pass region for phase idx %d\n",
			 phase_idx);
		pr_debug("Step1: Find the first failing condition\n");
		/* Look for the first failing condition by PHASE stepping.
		 * This part of the algo can finish without converging.
		 */

		/* escape if we find a fail (hold time violation)
		 * condition at any bit or if out of delay range.
		 */
		while (success && (phase_idx <= MAX_DQS_PHASE_IDX)) {
			DQS_phase_delay(phy, datx8, phase_idx);
			BIST_test(phy, datx8, &result);
			success = result.test_result;
			phase_idx++;
		}
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				datx8 + 1, nb_bytes, error);
			return TEST_FAILED;
		}

		/* if the loop ended with a failing condition at any bit,
		 * lets look for the first previous success condition by unit
		 * stepping (minimal delay)
		 */
		if (!success) {
			pr_debug("Fail region (PHASE) found phase idx %d\n",
				 phase_idx);
			pr_debug("Let's look for first success by DQS Unit steps\n");
			/* This part, the algo always converge */
			phase_idx--;

			/* escape if we find a success condition
			 * or if out of delay range.
			 */
			while (!success && dqs_unit_delay_index >= 0) {
				DQS_unit_delay(phy, datx8,
					       dqs_unit_delay_index);
				BIST_test(phy, datx8, &result);
				success = result.test_result;
				dqs_unit_delay_index--;
			}
			/* if the loop ended with a success condition,
			 * the last delay Right OK (before hold violation)
			 *  condition is then defined as following:
			 */
			if (success) {
				/* Hold the dely parameters of the the last
				 * delay Right OK condition.
				 * -1 to get back to current condition
				 */
				last_right_ok.phase = phase_idx;
				/*+1 to get back to current condition */
				last_right_ok.unit = dqs_unit_delay_index + 1;
				last_right_ok.bits_delay = 0xFFFFFFFF;
				pr_debug("Found %d\n", dqs_unit_delay_index);
			} else {
				/* the last OK condition is then with the
				 * previous phase_idx.
				 * -2 instead of -1 because at the last
				 * iteration of the while(),
				 * we incremented phase_idx
				 */
				last_right_ok.phase = phase_idx - 1;
				/* Nominal+1. Because we want the previous
				 * delay after reducing the phase delay.
				 */
				last_right_ok.unit = 1;
				last_right_ok.bits_delay = 0xFFFFFFFF;
				pr_debug("Not Found : try previous phase %d\n",
					 phase_idx - 1);

				DQS_phase_delay(phy, datx8, phase_idx - 1);
				dqs_unit_delay_index = 0;
				success = true;
				while (success &&
				       (dqs_unit_delay_index <
					MAX_DQS_UNIT_IDX)) {
					DQS_unit_delay(phy, datx8,
						       dqs_unit_delay_index);
					BIST_test(phy, datx8, &result);
					success = result.test_result;
					dqs_unit_delay_index++;
					pr_debug("dqs_unit_delay_index = %d, result = %d\n",
						 dqs_unit_delay_index, success);
				}

				if (!success) {
					last_right_ok.unit =
						 dqs_unit_delay_index - 1;
				} else {
					last_right_ok.unit = 0;
					pr_debug("ERROR: failed region not FOUND");
				}
			}
		} else {
			/* we can't find a failing  condition at all bits
			 * ==> Just hold the last test condition
			 * (the max DQS delay)
			 * which is the most likely,
			 * the closest to a hold violation
			 * If we can't find a Fail condition after
			 * the Pass region, stick at this position
			 * In order to have max chances to find a fail
			 * when reducing DQ delays.
			 */
			last_right_ok.phase = MAX_DQS_PHASE_IDX;
			last_right_ok.unit = MAX_DQS_UNIT_IDX;
			last_right_ok.bits_delay = 0xFFFFFFFF;
			pr_debug("Can't find the a fail condition\n");
		}

		/* step 2:
		 * if we arrive at this stage, it means that we found the last
		 * Right OK condition (by tweeking the DQS delay). Or we simply
		 * pushed DQS delay to the max
		 * This means that by reducing the delay on some DQ bits,
		 * we should find a failing condition.
		 */
		printf("Byte %d, DQS unit = %d, phase = %d\n",
		       datx8, last_right_ok.unit, last_right_ok.phase);
		pr_debug("Step2, unit = %d, phase = %d, bits delay=%x\n",
			 last_right_ok.unit, last_right_ok.phase,
			 last_right_ok.bits_delay);

		/* Restore the last_right_ok condtion. */
		DQS_unit_delay(phy, datx8, last_right_ok.unit);
		DQS_phase_delay(phy, datx8, last_right_ok.phase);
		writel(last_right_ok.bits_delay, DXNDQTR(phy, datx8));

		/* train each bit
		 * reduce delay on each bit, and perform a write/read test
		 * and stop at the very first time it fails.
		 * the goal is the find the first failing condition
		 * for each bit.
		 * When we achieve this condition<  for all the bits,
		 * we are sure they are aligned (+/- step resolution)
		 */
		fail_found = 0;
		for (bit_i = 0; bit_i < 8; bit_i++) {
			if (ctrlc()) {
				sprintf(string,
					"interrupted at byte %d/%d, error=%d",
					datx8 + 1, nb_bytes, error);
				return error;
			}
			pr_debug("deskewing bit %d:\n", bit_i);
			success = 1; /* init */
			/* Set all DQDLYn to maximum value.
			 * Only bit_i will be down-delayed
			 * ==> if we have a fail, it will be definitely
			 *     from bit_i
			 */
			writel(0xFFFFFFFF, DXNDQTR(phy, datx8));
			/* Arriving at this stage,
			 * we have a success condition with delay = 3;
			 */
			bit_i_delay_index = 3;

			/* escape if bit delay is out of range or
			 * if a fatil occurs
			 */
			while ((bit_i_delay_index >= 0) && success) {
				set_DQ_unit_delay(phy, datx8,
						  bit_i,
						  bit_i_delay_index);
				BIST_test(phy, datx8, &result);
				success = result.test_result;
				bit_i_delay_index--;
			}

			/* if escape with a fail condition
			 * ==> save this position for bit_i
			 */
			if (!success) {
				/* save the delay position.
				 * Add 1 because the while loop ended with a --,
				 * and that we need to hold the last success
				 *  delay
				 */
				deskew_delay[datx8][bit_i] =
					bit_i_delay_index + 2;
				if (deskew_delay[datx8][bit_i] > 3)
					deskew_delay[datx8][bit_i] = 3;

				/* A flag that states we found at least a fail
				 * at one bit.
				 */
				fail_found = 1;
				pr_debug("Fail found on bit %d, for delay = %d => deskew[%d][%d] = %d\n",
					 bit_i, bit_i_delay_index + 1,
					 datx8, bit_i,
					 deskew_delay[datx8][bit_i]);
			} else {
				/* if we can find a success condition by
				 * back-delaying this bit, just set the delay
				 * to 0 (the best deskew
				 * possible) and mark the bit.
				 */
				deskew_delay[datx8][bit_i] = 0;
				/* set a flag that will be used later
				 * in the report.
				 */
				deskew_non_converge[datx8][bit_i] = 1;
				pr_debug("Fail not found on bit %d => deskew[%d][%d] = %d\n",
					 bit_i, datx8, bit_i,
					 deskew_delay[datx8][bit_i]);
			}
		}
		pr_debug("**********byte %d tuning complete************\n",
			 datx8);
		/* If we can't find any failure by back delaying DQ lines,
		 * hold the default values
		 */
		if (!fail_found) {
			for (bit_i = 0; bit_i < 8; bit_i++)
				deskew_delay[datx8][bit_i] = 0;
			pr_debug("The Deskew algorithm can't converge, there is too much margin in your design. Good job!\n");
		}

		apply_deskew_results(phy, datx8, deskew_delay,
				     deskew_non_converge);
		/* Restore nominal value for DQS delay */
		DQS_phase_delay(phy, datx8, 3);
		DQS_unit_delay(phy, datx8, 3);
		/* disable byte after byte bits deskew */
		clrbits_le32(DXNGCR(phy, datx8), DDRPHYC_DXNGCR_DXEN);
	}  /* end of byte deskew */

	/* re-enable all data bytes */
	setbits_le32(&phy->dx0gcr, DDRPHYC_DXNGCR_DXEN);
	setbits_le32(&phy->dx1gcr, DDRPHYC_DXNGCR_DXEN);
	setbits_le32(&phy->dx2gcr, DDRPHYC_DXNGCR_DXEN);
	setbits_le32(&phy->dx3gcr, DDRPHYC_DXNGCR_DXEN);

	if (error) {
		sprintf(string, "error = %d", error);
		return TEST_FAILED;
	}

	return TEST_PASSED;
} /* end function */

/* Trim DQS timings and set it in the centre of data eye.
 * Look for a PPPPF region, then look for a FPPP region and finally select
 * the mid of the FPPPPPF region
 */
static enum test_result eye_training(struct stm32mp1_ddrctl *ctl,
				     struct stm32mp1_ddrphy *phy, char *string)
{
	/*Stores the DQS trim values (PHASE index, unit index) */
	u8 eye_training_val[NUM_BYTES][2];
	u8 byte = 0;
	struct BIST_result result;
	s8 dqs_unit_delay_index = 0;
	s8 phase_idx = 0;
	s8 dqs_unit_delay_index_pass = 0;
	s8 phase_idx_pass = 0;
	u8 success = 0;
	u8 left_phase_bound_found, right_phase_bound_found;
	u8 left_unit_bound_found, right_unit_bound_found;
	u8 left_bound_found, right_bound_found;
	struct tuning_position left_bound, right_bound;
	u8 error = 0;
	u8 nb_bytes = get_nb_bytes(ctl);

	/*Disable DQS Drift Compensation*/
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_DFTCMP);
	/*Disable all bytes*/
	/* Disable automatic power down of DLL and IOs when disabling a byte
	 * (To avoid having to add programming and  delay
	 * for a DLL re-lock when later re-enabling a disabled Byte Lane)
	 */
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_PDDISDX);

	/*Disable all data bytes */
	clrbits_le32(&phy->dx0gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx1gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx2gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx3gcr, DDRPHYC_DXNGCR_DXEN);

	/* Config the BIST block */
	config_BIST(phy);

	for (byte = 0; byte < nb_bytes; byte++) {
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				byte + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		right_bound.phase = 0;
		right_bound.unit = 0;

		left_bound.phase = 0;
		left_bound.unit = 0;

		left_phase_bound_found = 0;
		right_phase_bound_found = 0;

		left_unit_bound_found = 0;
		right_unit_bound_found = 0;

		left_bound_found = 0;
		right_bound_found = 0;

		/* Enable Byte (DXNGCR, bit DXEN) */
		setbits_le32(DXNGCR(phy, byte), DDRPHYC_DXNGCR_DXEN);

		/* Select the byte lane for comparison of read data */
		BIST_datx8_sel(phy, byte);

		/* Set DQS phase delay to the nominal value. */
		phase_idx = _90deg;
		phase_idx_pass = phase_idx;

		/* Set DQS unit delay to the nominal value. */
		dqs_unit_delay_index = 3;
		dqs_unit_delay_index_pass = dqs_unit_delay_index;
		success = 0;

		pr_debug("STEP0: Find Init delay\n");
		/* STEP0: Find Init delay: a delay that put the system
		 * in a "Pass" condition then (TODO) update
		 * dqs_unit_delay_index_pass & phase_idx_pass
		 */
		DQS_unit_delay(phy, byte, dqs_unit_delay_index);
		DQS_phase_delay(phy, byte, phase_idx);
		BIST_test(phy, byte, &result);
		success = result.test_result;
		/* If we have a fail in the nominal condition */
		if (!success) {
			/* Look at the left */
			while (phase_idx >= 0 && !success) {
				phase_idx--;
				DQS_phase_delay(phy, byte, phase_idx);
				BIST_test(phy, byte, &result);
				success = result.test_result;
			}
		}
		if (!success) {
			/* if we can't find pass condition,
			 * then look at the right
			 */
			phase_idx = _90deg;
			while (phase_idx <= MAX_DQS_PHASE_IDX &&
			       !success) {
				phase_idx++;
				DQS_phase_delay(phy, byte,
						phase_idx);
				BIST_test(phy, byte, &result);
				success = result.test_result;
			}
		}
		/* save the pass condition */
		if (success) {
			phase_idx_pass = phase_idx;
		} else {
			printf("Result: Failed ");
			printf("[Cannot DQS timings, ");
			printf("there is no PASS region]\n");
			error++;
			continue;
		}

		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				byte + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		pr_debug("STEP1: Find LEFT PHASE DQS Bound\n");
		/* STEP1: Find LEFT PHASE DQS Bound */
		while ((phase_idx >= 0) &&
		       (phase_idx <= MAX_DQS_PHASE_IDX) &&
		       !left_phase_bound_found) {
			DQS_unit_delay(phy, byte,
				       dqs_unit_delay_index);
			DQS_phase_delay(phy, byte,
					phase_idx);
			BIST_test(phy, byte, &result);
			success = result.test_result;

			/*TODO: Manage the case were at the beginning
			 * there is already a fail
			 */
			if (!success) {
				/* the last pass condition */
				left_bound.phase = ++phase_idx;
				left_phase_bound_found = 1;
			} else if (success) {
				phase_idx--;
			}
		}
		if (!left_phase_bound_found) {
			left_bound.phase = 0;
			phase_idx = 0;
		}
		/* If not found, lets take 0 */

		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				byte + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		pr_debug("STEP2: Find UNIT left bound\n");
		/* STEP2: Find UNIT left bound */
		while ((dqs_unit_delay_index >= 0) &&
		       !left_unit_bound_found) {
			DQS_unit_delay(phy, byte,
				       dqs_unit_delay_index);
			DQS_phase_delay(phy, byte, phase_idx);
			BIST_test(phy, byte, &result);
			success = result.test_result;
			if (!success) {
				left_bound.unit =
					++dqs_unit_delay_index;
				left_unit_bound_found = 1;
				left_bound_found = 1;
			} else if (success) {
				dqs_unit_delay_index--;
			}
		}

		/* If not found, lets take 0 */
		if (!left_unit_bound_found)
			left_bound.unit = 0;

		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				byte + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		pr_debug("STEP3: Find PHase right bound\n");
		/* STEP3: Find PHase right bound, start with "pass"
		 * condition
		 */

		/* Set DQS phase delay to the pass value. */
		phase_idx = phase_idx_pass;

		/* Set DQS unit delay to the pass value. */
		dqs_unit_delay_index = dqs_unit_delay_index_pass;

		while ((phase_idx <= MAX_DQS_PHASE_IDX) &&
		       !right_phase_bound_found) {
			DQS_unit_delay(phy, byte,
				       dqs_unit_delay_index);
			DQS_phase_delay(phy, byte, phase_idx);
			BIST_test(phy, byte, &result);
			success = result.test_result;
			if (!success) {
				/* the last pass condition */
				right_bound.phase = --phase_idx;
				right_phase_bound_found = 1;
			} else if (success) {
				phase_idx++;
			}
		}

		/* If not found, lets take the max value */
		if (!right_phase_bound_found) {
			right_bound.phase = MAX_DQS_PHASE_IDX;
			phase_idx = MAX_DQS_PHASE_IDX;
		}

		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d, error=%d",
				byte + 1, nb_bytes, error);
			return TEST_FAILED;
		}
		pr_debug("STEP4: Find UNIT right bound\n");
		/* STEP4: Find UNIT right bound */
		while ((dqs_unit_delay_index <= MAX_DQS_UNIT_IDX) &&
		       !right_unit_bound_found) {
			DQS_unit_delay(phy, byte,
				       dqs_unit_delay_index);
			DQS_phase_delay(phy, byte, phase_idx);
			BIST_test(phy, byte, &result);
			success = result.test_result;
			if (!success) {
				right_bound.unit =
					--dqs_unit_delay_index;
				right_unit_bound_found = 1;
				right_bound_found = 1;
			} else if (success) {
				dqs_unit_delay_index++;
			}
		}
		/* If not found, lets take the max value */
		if (!right_unit_bound_found)
			right_bound.unit = MAX_DQS_UNIT_IDX;

		/* If we found a regular FAil Pass FAil pattern
		 * FFPPPPPPFF
		 * OR PPPPPFF  Or FFPPPPP
		 */

		if (left_bound_found || right_bound_found) {
			eye_training_val[byte][0] = (right_bound.phase +
						 left_bound.phase) / 2;
			eye_training_val[byte][1] = (right_bound.unit +
						 left_bound.unit) / 2;

			/* If we already lost 1/2PHASE Tuning,
			 * let's try to recover by ++ on unit
			 */
			if (((right_bound.phase + left_bound.phase) % 2 == 1) &&
			    eye_training_val[byte][1] != MAX_DQS_UNIT_IDX)
				eye_training_val[byte][1]++;
			pr_debug("** found phase : %d -  %d & unit %d - %d\n",
				 right_bound.phase, left_bound.phase,
				 right_bound.unit, left_bound.unit);
			pr_debug("** calculating mid region: phase: %d  unit: %d (nominal is 3)\n",
				 eye_training_val[byte][0],
				 eye_training_val[byte][1]);
		} else {
			/* PPPPPPPPPP, we're already good.
			 * Set nominal values.
			 */
			eye_training_val[byte][0] = 3;
			eye_training_val[byte][1] = 3;
		}
		DQS_phase_delay(phy, byte, eye_training_val[byte][0]);
		DQS_unit_delay(phy, byte, eye_training_val[byte][1]);

		printf("Byte %d, DQS unit = %d, phase = %d\n",
		       byte,
		       eye_training_val[byte][1],
		       eye_training_val[byte][0]);
	}

	if (error) {
		sprintf(string, "error = %d", error);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

static void display_reg_results(struct stm32mp1_ddrphy *phy, u8 byte)
{
	u8 i = 0;

	printf("Byte %d Dekew result, bit0 delay, bit1 delay...bit8 delay\n  ",
	       byte);

	for (i = 0; i < 8; i++)
		printf("%d ", DQ_unit_index(phy, byte, i));
	printf("\n");

	printf("dxndllcr: [%08x] val:%08x\n",
	       DXNDLLCR(phy, byte),
	       readl(DXNDLLCR(phy, byte)));
	printf("dxnqdstr: [%08x] val:%08x\n",
	       DXNDQSTR(phy, byte),
	       readl(DXNDQSTR(phy, byte)));
	printf("dxndqtr: [%08x] val:%08x\n",
	       DXNDQTR(phy, byte),
	       readl(DXNDQTR(phy, byte)));
}

/* analyse the dgs gating log table, and determine the midpoint.*/
static u8 set_midpoint_read_dqs_gating(struct stm32mp1_ddrphy *phy, u8 byte,
				       u8 dqs_gating[NUM_BYTES]
						    [MAX_GSL_IDX + 1]
						    [MAX_GPS_IDX + 1])
{
	/* stores the dqs gate values (gsl index, gps index) */
	u8 dqs_gate_values[NUM_BYTES][2];
	u8 gsl_idx, gps_idx = 0;
	u8 left_bound_idx[2] = {0, 0};
	u8 right_bound_idx[2] = {0, 0};
	u8 left_bound_found = 0;
	u8 right_bound_found = 0;
	u8 intermittent = 0;
	u8 value;

	for (gsl_idx = 0; gsl_idx <= MAX_GSL_IDX; gsl_idx++) {
		for (gps_idx = 0; gps_idx <= MAX_GPS_IDX; gps_idx++) {
			value = dqs_gating[byte][gsl_idx][gps_idx];
			if (value == 1 && left_bound_found == 0) {
				left_bound_idx[0] = gsl_idx;
				left_bound_idx[1] = gps_idx;
				left_bound_found = 1;
			} else if (value == 0 &&
				   left_bound_found == 1 &&
				   !right_bound_found) {
				if (gps_idx == 0) {
					right_bound_idx[0] = gsl_idx - 1;
					right_bound_idx[1] = MAX_GPS_IDX;
				} else {
					right_bound_idx[0] = gsl_idx;
					right_bound_idx[1] = gps_idx - 1;
				}
				right_bound_found = 1;
			} else if (value == 1 &&
				   right_bound_found == 1) {
				intermittent = 1;
			}
		}
	}

	/* if only ppppppp is found, there is no mid region. */
	if (left_bound_idx[0] == 0 && left_bound_idx[1] == 0 &&
	    right_bound_idx[0] == 0 && right_bound_idx[1] == 0)
		intermittent = 1;

	/*if we found a regular fail pass fail pattern ffppppppff
	 * or pppppff  or ffppppp
	 */
	if (!intermittent) {
		/*if we found a regular fail pass fail pattern ffppppppff
		 * or pppppff  or ffppppp
		 */
		if (left_bound_found || right_bound_found) {
			pr_debug("idx0(%d): %d %d      idx1(%d) : %d %d\n",
				 left_bound_found,
				 right_bound_idx[0], left_bound_idx[0],
				 right_bound_found,
				 right_bound_idx[1], left_bound_idx[1]);
			dqs_gate_values[byte][0] =
				(right_bound_idx[0] + left_bound_idx[0]) / 2;
			dqs_gate_values[byte][1] =
				(right_bound_idx[1] + left_bound_idx[1]) / 2;
			/* if we already lost 1/2gsl tuning,
			 * let's try to recover by ++ on gps
			 */
			if (((right_bound_idx[0] +
			      left_bound_idx[0]) % 2 == 1) &&
			    dqs_gate_values[byte][1] != MAX_GPS_IDX)
				dqs_gate_values[byte][1]++;
			/* if we already lost 1/2gsl tuning and gps is on max*/
			else if (((right_bound_idx[0] +
				   left_bound_idx[0]) % 2 == 1) &&
				 dqs_gate_values[byte][1] == MAX_GPS_IDX) {
				dqs_gate_values[byte][1] = 0;
				dqs_gate_values[byte][0]++;
			}
			/* if we have gsl left and write limit too close
			 * (difference=1)
			 */
			if (((right_bound_idx[0] - left_bound_idx[0]) == 1)) {
				dqs_gate_values[byte][1] = (left_bound_idx[1] +
							    right_bound_idx[1] +
							    4) / 2;
				if (dqs_gate_values[byte][1] >= 4) {
					dqs_gate_values[byte][0] =
						right_bound_idx[0];
					dqs_gate_values[byte][1] -= 4;
				} else {
					dqs_gate_values[byte][0] =
						left_bound_idx[0];
				}
			}
			pr_debug("*******calculating mid region: system latency: %d  phase: %d********\n",
				 dqs_gate_values[byte][0],
				 dqs_gate_values[byte][1]);
			pr_debug("*******the nominal values were system latency: 0  phase: 2*******\n");
			set_r0dgsl_delay(phy, byte, dqs_gate_values[byte][0]);
			set_r0dgps_delay(phy, byte, dqs_gate_values[byte][1]);
		}
	} else {
		/* if intermitant, restore defaut values */
		pr_debug("dqs gating:no regular fail/pass/fail found. defaults values restored.\n");
		set_r0dgsl_delay(phy, byte, 0);
		set_r0dgps_delay(phy, byte, 2);
	}

	/* return 0 if intermittent or if both left_bound
	 * and right_bound are not found
	 */
	return !(intermittent || (left_bound_found && right_bound_found));
}

static enum test_result read_dqs_gating(struct stm32mp1_ddrctl *ctl,
					struct stm32mp1_ddrphy *phy,
					char *string)
{
	/* stores the log of pass/fail */
	u8 dqs_gating[NUM_BYTES][MAX_GSL_IDX + 1][MAX_GPS_IDX + 1];
	u8 byte, gsl_idx, gps_idx = 0;
	struct BIST_result result;
	u8 success = 0;
	u8 nb_bytes = get_nb_bytes(ctl);

	memset(dqs_gating, 0x0, sizeof(dqs_gating));

	/*disable dqs drift compensation*/
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_DFTCMP);
	/*disable all bytes*/
	/* disable automatic power down of dll and ios when disabling a byte
	 * (to avoid having to add programming and  delay
	 * for a dll re-lock when later re-enabling a disabled byte lane)
	 */
	clrbits_le32(&phy->pgcr, DDRPHYC_PGCR_PDDISDX);

	/* disable all data bytes */
	clrbits_le32(&phy->dx0gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx1gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx2gcr, DDRPHYC_DXNGCR_DXEN);
	clrbits_le32(&phy->dx3gcr, DDRPHYC_DXNGCR_DXEN);

	/* config the bist block */
	config_BIST(phy);

	for (byte = 0; byte < nb_bytes; byte++) {
		if (ctrlc()) {
			sprintf(string, "interrupted at byte %d/%d",
				byte + 1, nb_bytes);
			return TEST_FAILED;
		}
		/* enable byte x (dxngcr, bit dxen) */
		setbits_le32(DXNGCR(phy, byte), DDRPHYC_DXNGCR_DXEN);

		/* select the byte lane for comparison of read data */
		BIST_datx8_sel(phy, byte);
		for (gsl_idx = 0; gsl_idx <= MAX_GSL_IDX; gsl_idx++) {
			for (gps_idx = 0; gps_idx <= MAX_GPS_IDX; gps_idx++) {
				if (ctrlc()) {
					sprintf(string,
						"interrupted at byte %d/%d",
						byte + 1, nb_bytes);
					return TEST_FAILED;
				}
				/* write cfg to dxndqstr */
				set_r0dgsl_delay(phy, byte, gsl_idx);
				set_r0dgps_delay(phy, byte, gps_idx);

				BIST_test(phy, byte, &result);
				success = result.test_result;
				if (success)
					dqs_gating[byte][gsl_idx][gps_idx] = 1;
				itm_soft_reset(phy);
			}
		}
		set_midpoint_read_dqs_gating(phy, byte, dqs_gating);
		/* dummy reads */
		readl(0xc0000000);
		readl(0xc0000000);
	}

	/* re-enable drift compensation */
	/* setbits_le32(&phy->pgcr, DDRPHYC_PGCR_DFTCMP); */
	return TEST_PASSED;
}

/****************************************************************
 * TEST
 ****************************************************************
 */
static enum test_result do_read_dqs_gating(struct stm32mp1_ddrctl *ctl,
					   struct stm32mp1_ddrphy *phy,
					   char *string, int argc,
					   char *argv[])
{
	u32 rfshctl3 = readl(&ctl->rfshctl3);
	u32 pwrctl = readl(&ctl->pwrctl);
	enum test_result res;

	stm32mp1_refresh_disable(ctl);
	res = read_dqs_gating(ctl, phy, string);
	stm32mp1_refresh_restore(ctl, rfshctl3, pwrctl);

	return res;
}

static enum test_result do_bit_deskew(struct stm32mp1_ddrctl *ctl,
				      struct stm32mp1_ddrphy *phy,
				      char *string, int argc, char *argv[])
{
	u32 rfshctl3 = readl(&ctl->rfshctl3);
	u32 pwrctl = readl(&ctl->pwrctl);
	enum test_result res;

	stm32mp1_refresh_disable(ctl);
	res = bit_deskew(ctl, phy, string);
	stm32mp1_refresh_restore(ctl, rfshctl3, pwrctl);

	return res;
}

static enum test_result do_eye_training(struct stm32mp1_ddrctl *ctl,
					struct stm32mp1_ddrphy *phy,
					char *string, int argc, char *argv[])
{
	u32 rfshctl3 = readl(&ctl->rfshctl3);
	u32 pwrctl = readl(&ctl->pwrctl);
	enum test_result res;

	stm32mp1_refresh_disable(ctl);
	res = eye_training(ctl, phy, string);
	stm32mp1_refresh_restore(ctl, rfshctl3, pwrctl);

	return res;
}

static enum test_result do_display(struct stm32mp1_ddrctl *ctl,
				   struct stm32mp1_ddrphy *phy,
				   char *string, int argc, char *argv[])
{
	int byte;
	u8 nb_bytes = get_nb_bytes(ctl);

	for (byte = 0; byte < nb_bytes; byte++)
		display_reg_results(phy, byte);

	return TEST_PASSED;
}

static enum test_result do_bist_config(struct stm32mp1_ddrctl *ctl,
				       struct stm32mp1_ddrphy *phy,
				       char *string, int argc, char *argv[])
{
	unsigned long value;

	if (argc > 0) {
		if (strict_strtoul(argv[0], 0, &value) < 0) {
			sprintf(string, "invalid nbErr %s", argv[0]);
			return TEST_FAILED;
		}
		BIST_error_max = value;
	}
	if (argc > 1) {
		if (strict_strtoul(argv[1], 0, &value) < 0) {
			sprintf(string, "invalid Seed %s", argv[1]);
			return TEST_FAILED;
		}
		BIST_seed = value;
	}
	printf("Bist.nbErr = %d\n", BIST_error_max);
	if (BIST_seed)
		printf("Bist.Seed = 0x%x\n", BIST_seed);
	else
		printf("Bist.Seed = random\n");

	return TEST_PASSED;
}

/****************************************************************
 * TEST Description
 ****************************************************************
 */

const struct test_desc tuning[] = {
	{do_read_dqs_gating, "Read DQS gating",
		"software read DQS Gating", "", 0 },
	{do_bit_deskew, "Bit de-skew", "", "", 0 },
	{do_eye_training, "Eye Training", "or DQS training", "", 0 },
	{do_display, "Display registers", "", "", 0 },
	{do_bist_config, "Bist config", "[nbErr] [seed]",
	 "configure Bist test", 2},
};

const int tuning_nb = ARRAY_SIZE(tuning);
