// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_hw_training.h"

/*
 * Debug
 */
#define DEBUG_DQS_C(s, d, l) \
	DEBUG_DQS_S(s); DEBUG_DQS_D(d, l); DEBUG_DQS_S("\n")
#define DEBUG_DQS_FULL_C(s, d, l) \
	DEBUG_DQS_FULL_S(s); DEBUG_DQS_FULL_D(d, l); DEBUG_DQS_FULL_S("\n")
#define DEBUG_DQS_RESULTS_C(s, d, l) \
	DEBUG_DQS_RESULTS_S(s); DEBUG_DQS_RESULTS_D(d, l); DEBUG_DQS_RESULTS_S("\n")
#define DEBUG_PER_DQ_C(s, d, l) \
	puts(s); printf("%x", d); puts("\n")

#define DEBUG_DQS_RESULTS_S(s) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%s", s)
#define DEBUG_DQS_RESULTS_D(d, l) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%x", d)

#define DEBUG_PER_DQ_S(s) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_3, "%s", s)
#define DEBUG_PER_DQ_D(d, l) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_3, "%x", d)
#define DEBUG_PER_DQ_DD(d, l) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_3, "%d", d)

#ifdef MV_DEBUG_DQS
#define DEBUG_DQS_S(s)			puts(s)
#define DEBUG_DQS_D(d, l)		printf("%x", d)
#else
#define DEBUG_DQS_S(s)
#define DEBUG_DQS_D(d, l)
#endif

#ifdef MV_DEBUG_DQS_FULL
#define DEBUG_DQS_FULL_S(s)		puts(s)
#define DEBUG_DQS_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_DQS_FULL_S(s)
#define DEBUG_DQS_FULL_D(d, l)
#endif

/* State machine for centralization - find low & high limit */
enum {
	PUP_ADLL_LIMITS_STATE_FAIL,
	PUP_ADLL_LIMITS_STATE_PASS,
	PUP_ADLL_LIMITS_STATE_FAIL_AFTER_PASS,
};

/* Hold centralization low results */
static int centralization_low_limit[MAX_PUP_NUM] = { 0 };
/* Hold centralization high results */
static int centralization_high_limit[MAX_PUP_NUM] = { 0 };

int ddr3_find_adll_limits(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc, int is_tx);
int ddr3_check_window_limits(u32 pup, int high_limit, int low_limit, int is_tx,
			  int *size_valid);
static int ddr3_center_calc(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
			    int is_tx);
int ddr3_special_pattern_i_search(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
			      int is_tx, u32 special_pattern_pup);
int ddr3_special_pattern_ii_search(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
				   int is_tx, u32 special_pattern_pup);
int ddr3_set_dqs_centralization_results(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
				    int is_tx);

#ifdef MV88F78X60
extern u32 killer_pattern_32b[DQ_NUM][LEN_SPECIAL_PATTERN];
extern u32 killer_pattern_64b[DQ_NUM][LEN_SPECIAL_PATTERN];
extern int per_bit_data[MAX_PUP_NUM][DQ_NUM];
#else
extern u32 killer_pattern[DQ_NUM][LEN_16BIT_KILLER_PATTERN];
extern u32 killer_pattern_32b[DQ_NUM][LEN_SPECIAL_PATTERN];
#if defined(MV88F672X)
extern int per_bit_data[MAX_PUP_NUM][DQ_NUM];
#endif
#endif
extern u32 special_pattern[DQ_NUM][LEN_SPECIAL_PATTERN];

static u32 *ddr3_dqs_choose_pattern(MV_DRAM_INFO *dram_info, u32 victim_dq)
{
	u32 *pattern_ptr;

	/* Choose pattern */
	switch (dram_info->ddr_width) {
#if defined(MV88F672X)
	case 16:
		pattern_ptr = (u32 *)&killer_pattern[victim_dq];
		break;
#endif
	case 32:
		pattern_ptr = (u32 *)&killer_pattern_32b[victim_dq];
		break;
#if defined(MV88F78X60)
	case 64:
		pattern_ptr = (u32 *)&killer_pattern_64b[victim_dq];
		break;
#endif
	default:
#if defined(MV88F78X60)
		pattern_ptr = (u32 *)&killer_pattern_32b[victim_dq];
#else
		pattern_ptr = (u32 *)&killer_pattern[victim_dq];
#endif
		break;
	}

	return pattern_ptr;
}

/*
 * Name:     ddr3_dqs_centralization_rx
 * Desc:     Execute the DQS centralization RX phase.
 * Args:     dram_info
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_dqs_centralization_rx(MV_DRAM_INFO *dram_info)
{
	u32 cs, ecc, reg;
	int status;

	DEBUG_DQS_S("DDR3 - DQS Centralization RX - Starting procedure\n");

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);

	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_DQS_S("DDR3 - DQS Centralization RX - SW Override Enabled\n");

	reg = (1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	/* Loop for each CS */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			DEBUG_DQS_FULL_C("DDR3 - DQS Centralization RX - CS - ",
					 (u32) cs, 1);

			for (ecc = 0; ecc < (dram_info->ecc_ena + 1); ecc++) {

				/* ECC Support - Switch ECC Mux on ecc=1 */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg |= (dram_info->ecc_ena *
					ecc << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

				if (ecc)
					DEBUG_DQS_FULL_S("DDR3 - DQS Centralization RX - ECC Mux Enabled\n");
				else
					DEBUG_DQS_FULL_S("DDR3 - DQS Centralization RX - ECC Mux Disabled\n");

				DEBUG_DQS_FULL_S("DDR3 - DQS Centralization RX - Find all limits\n");

				status = ddr3_find_adll_limits(dram_info, cs,
							       ecc, 0);
				if (MV_OK != status)
					return status;

				DEBUG_DQS_FULL_S("DDR3 - DQS Centralization RX - Start calculating center\n");

				status = ddr3_center_calc(dram_info, cs, ecc,
							  0);
				if (MV_OK != status)
					return status;
			}
		}
	}

	/* ECC Support - Disable ECC MUX */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_1_ADDR) |
		(1 << REG_DRAM_TRAINING_1_TRNBPOINT_OFFS);
	reg_write(REG_DRAM_TRAINING_1_ADDR, reg);

	return MV_OK;
}

/*
 * Name:     ddr3_dqs_centralization_tx
 * Desc:     Execute the DQS centralization TX phase.
 * Args:     dram_info
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_dqs_centralization_tx(MV_DRAM_INFO *dram_info)
{
	u32 cs, ecc, reg;
	int status;

	DEBUG_DQS_S("DDR3 - DQS Centralization TX - Starting procedure\n");

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);

	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_DQS_S("DDR3 - DQS Centralization TX - SW Override Enabled\n");

	reg = (1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	/* Loop for each CS */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			DEBUG_DQS_FULL_C("DDR3 - DQS Centralization TX - CS - ",
					 (u32) cs, 1);
			for (ecc = 0; ecc < (dram_info->ecc_ena + 1); ecc++) {
				/* ECC Support - Switch ECC Mux on ecc=1 */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg |= (dram_info->ecc_ena *
					ecc << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

				if (ecc)
					DEBUG_DQS_FULL_S("DDR3 - DQS Centralization TX - ECC Mux Enabled\n");
				else
					DEBUG_DQS_FULL_S("DDR3 - DQS Centralization TX - ECC Mux Disabled\n");

				DEBUG_DQS_FULL_S("DDR3 - DQS Centralization TX - Find all limits\n");

				status = ddr3_find_adll_limits(dram_info, cs,
							       ecc, 1);
				if (MV_OK != status)
					return status;

				DEBUG_DQS_FULL_S("DDR3 - DQS Centralization TX - Start calculating center\n");

				status = ddr3_center_calc(dram_info, cs, ecc,
							  1);
				if (MV_OK != status)
					return status;
			}
		}
	}

	/* ECC Support - Disable ECC MUX */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_1_ADDR) |
		(1 << REG_DRAM_TRAINING_1_TRNBPOINT_OFFS);
	reg_write(REG_DRAM_TRAINING_1_ADDR, reg);

	return MV_OK;
}

/*
 * Name:     ddr3_find_adll_limits
 * Desc:     Execute the Find ADLL limits phase.
 * Args:     dram_info
 *           cs
 *           ecc_ena
 *           is_tx             Indicate whether Rx or Tx
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_find_adll_limits(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc, int is_tx)
{
	u32 victim_dq, pup, tmp;
	u32 adll_addr;
	u32 max_pup;		/* maximal pup index */
	u32 pup_mask = 0;
	u32 unlock_pup;		/* bit array of un locked pups */
	u32 new_unlock_pup;	/* bit array of compare failed pups */
	u32 curr_adll;
	u32 adll_start_val;	/* adll start loop value - for rx or tx limit */
	u32 high_limit;	/* holds found High Limit */
	u32 low_limit;		/* holds found Low Limit */
	int win_valid;
	int update_win;
	u32 sdram_offset;
	u32 uj, cs_count, cs_tmp, ii;
	u32 *pattern_ptr;
	u32 dq;
	u32 adll_end_val;	/* adll end of loop val - for rx or tx limit */
	u8 analog_pbs[DQ_NUM][MAX_PUP_NUM][DQ_NUM][2];
	u8 analog_pbs_sum[MAX_PUP_NUM][DQ_NUM][2];
	int pup_adll_limit_state[MAX_PUP_NUM];	/* hold state of each pup */

	adll_addr = ((is_tx == 1) ? PUP_DQS_WR : PUP_DQS_RD);
	adll_end_val = ((is_tx == 1) ? ADLL_MIN : ADLL_MAX);
	adll_start_val = ((is_tx == 1) ? ADLL_MAX : ADLL_MIN);
	max_pup = (ecc + (1 - ecc) * dram_info->num_of_std_pups);

	DEBUG_DQS_FULL_S("DDR3 - DQS Find Limits - Starting Find ADLL Limits\n");

	/* init the array */
	for (pup = 0; pup < max_pup; pup++) {
		centralization_low_limit[pup] = ADLL_MIN;
		centralization_high_limit[pup] = ADLL_MAX;
	}

	/* Killer Pattern */
	cs_count = 0;
	for (cs_tmp = 0; cs_tmp < cs; cs_tmp++) {
		if (dram_info->cs_ena & (1 << cs_tmp))
			cs_count++;
	}
	sdram_offset = cs_count * (SDRAM_CS_SIZE + 1);
	sdram_offset += ((is_tx == 1) ?
			 SDRAM_DQS_TX_OFFS : SDRAM_DQS_RX_OFFS);

	/* Prepare pup masks */
	for (pup = 0; pup < max_pup; pup++)
		pup_mask |= (1 << pup);

	for (pup = 0; pup < max_pup; pup++) {
		for (dq = 0; dq < DQ_NUM; dq++) {
			analog_pbs_sum[pup][dq][0] = adll_start_val;
			analog_pbs_sum[pup][dq][1] = adll_end_val;
		}
	}

	/* Loop - use different pattern for each victim_dq */
	for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
		DEBUG_DQS_FULL_C("DDR3 - DQS Find Limits - Victim DQ - ",
				 (u32)victim_dq, 1);
		/*
		 * The pups 3 bit arrays represent state machine. with
		 * 3 stages for each pup.
		 * 1. fail and didn't get pass in earlier compares.
		 * 2. pass compare
		 * 3. fail after pass - end state.
		 * The window limits are the adll values where the adll
		 * was in the pass stage.
		 */

		/* Set all states to Fail (1st state) */
		for (pup = 0; pup < max_pup; pup++)
			pup_adll_limit_state[pup] = PUP_ADLL_LIMITS_STATE_FAIL;

		/* Set current valid pups */
		unlock_pup = pup_mask;

		/* Set ADLL to start value */
		curr_adll = adll_start_val;

#if defined(MV88F78X60)
		for (pup = 0; pup < max_pup; pup++) {
			for (dq = 0; dq < DQ_NUM; dq++) {
				analog_pbs[victim_dq][pup][dq][0] =
					adll_start_val;
				analog_pbs[victim_dq][pup][dq][1] =
					adll_end_val;
				per_bit_data[pup][dq] = 0;
			}
		}
#endif

		for (uj = 0; uj < ADLL_MAX; uj++) {
			DEBUG_DQS_FULL_C("DDR3 - DQS Find Limits - Setting ADLL to ",
					 curr_adll, 2);
			for (pup = 0; pup < max_pup; pup++) {
				if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
					tmp = ((is_tx == 1) ? curr_adll +
					       dram_info->wl_val[cs]
					       [pup * (1 - ecc) + ecc * ECC_PUP]
					       [D] : curr_adll);
					ddr3_write_pup_reg(adll_addr, cs, pup +
						(ecc * ECC_PUP), 0, tmp);
				}
			}

			/* Choose pattern */
			pattern_ptr = ddr3_dqs_choose_pattern(dram_info,
							      victim_dq);

			/* '1' - means pup failed, '0' - means pup pass */
			new_unlock_pup = 0;

			/* Read and compare results for Victim_DQ# */
			for (ii = 0; ii < 3; ii++) {
				u32 tmp = 0;
				if (MV_OK != ddr3_sdram_dqs_compare(dram_info,
							   unlock_pup, &tmp,
							   pattern_ptr,
							   LEN_KILLER_PATTERN,
							   sdram_offset +
							   LEN_KILLER_PATTERN *
							   4 * victim_dq,
							   is_tx, 0, NULL,
							   0))
					return MV_DDR3_TRAINING_ERR_DRAM_COMPARE;

				new_unlock_pup |= tmp;
			}

			pup = 0;
			DEBUG_DQS_FULL_C("DDR3 - DQS Find Limits - UnlockPup: ",
					 unlock_pup, 2);
			DEBUG_DQS_FULL_C("DDR3 - DQS Find Limits - NewUnlockPup: ",
					 new_unlock_pup, 2);

			/* Update pup state */
			for (pup = 0; pup < max_pup; pup++) {
				if (IS_PUP_ACTIVE(unlock_pup, pup) == 0) {
					DEBUG_DQS_FULL_C("DDR3 - DQS Find Limits - Skipping pup ",
							 pup, 1);
					continue;
				}

				/*
				 * Still didn't find the window limit of the pup
				 */
				if (IS_PUP_ACTIVE(new_unlock_pup, pup) == 1) {
					/* Current compare result == fail */
					if (pup_adll_limit_state[pup] ==
					    PUP_ADLL_LIMITS_STATE_PASS) {
						/*
						 * If now it failed but passed
						 * earlier
						 */
						DEBUG_DQS_S("DDR3 - DQS Find Limits - PASS to FAIL: CS - ");
						DEBUG_DQS_D(cs, 1);
						DEBUG_DQS_S(", DQ - ");
						DEBUG_DQS_D(victim_dq, 1);
						DEBUG_DQS_S(", Pup - ");
						DEBUG_DQS_D(pup, 1);
						DEBUG_DQS_S(", ADLL - ");
						DEBUG_DQS_D(curr_adll, 2);
						DEBUG_DQS_S("\n");

#if defined(MV88F78X60)
						for (dq = 0; dq < DQ_NUM; dq++) {
							if ((analog_pbs[victim_dq][pup][dq][0] != adll_start_val)
							    && (analog_pbs[victim_dq][pup]
								[dq][1] == adll_end_val))
								analog_pbs
									[victim_dq]
									[pup][dq]
									[1] =
									curr_adll;
						}
#endif
						win_valid = 1;
						update_win = 0;

						/* Keep min / max limit value */
						if (is_tx == 0) {
							/* RX - found upper limit */
							if (centralization_high_limit[pup] >
							    (curr_adll - 1)) {
								high_limit =
									curr_adll - 1;
								low_limit =
									centralization_low_limit[pup];
								update_win = 1;
							}
						} else {
							/* TX - found lower limit */
							if (centralization_low_limit[pup] < (curr_adll + 1)) {
								high_limit =
									centralization_high_limit
									[pup];
								low_limit =
									curr_adll + 1;
								update_win =
									1;
							}
						}

						if (update_win == 1) {
							/*
							 * Before updating
							 * window limits we need
							 * to check that the
							 * limits are valid
							 */
							if (MV_OK !=
							    ddr3_check_window_limits
							    (pup, high_limit,
							     low_limit, is_tx,
							     &win_valid))
								return MV_DDR3_TRAINING_ERR_WIN_LIMITS;

							if (win_valid == 1) {
								/*
								 * Window limits
								 * should be
								 * updated
								 */
								centralization_low_limit
									[pup] =
									low_limit;
								centralization_high_limit
									[pup] =
									high_limit;
							}
						}

						if (win_valid == 1) {
							/* Found end of window - lock the pup */
							pup_adll_limit_state[pup] =
								PUP_ADLL_LIMITS_STATE_FAIL_AFTER_PASS;
							unlock_pup &= ~(1 << pup);
						} else {
							/* Probably false pass - reset status */
							pup_adll_limit_state[pup] =
								PUP_ADLL_LIMITS_STATE_FAIL;

#if defined(MV88F78X60)
							/* Clear logging array of win size (per Dq) */
							for (dq = 0;
							     dq < DQ_NUM;
							     dq++) {
								analog_pbs
									[victim_dq]
									[pup][dq]
									[0] =
									adll_start_val;
								analog_pbs
									[victim_dq]
									[pup][dq]
									[1] =
									adll_end_val;
								per_bit_data
									[pup][dq]
									= 0;
							}
#endif
						}
					}
				} else {
					/* Current compare result == pass */
					if (pup_adll_limit_state[pup] ==
					    PUP_ADLL_LIMITS_STATE_FAIL) {
						/* If now it passed but failed earlier */
						DEBUG_DQS_S("DDR3 - DQS Find Limits - FAIL to PASS: CS - ");
						DEBUG_DQS_D(cs, 1);
						DEBUG_DQS_S(", DQ - ");
						DEBUG_DQS_D(victim_dq, 1);
						DEBUG_DQS_S(", Pup - ");
						DEBUG_DQS_D(pup, 1);
						DEBUG_DQS_S(", ADLL - ");
						DEBUG_DQS_D(curr_adll, 2);
						DEBUG_DQS_S("\n");

#if defined(MV88F78X60)
						for (dq = 0; dq < DQ_NUM;
						     dq++) {
							if (analog_pbs[victim_dq][pup][dq][0] == adll_start_val)
								analog_pbs
								    [victim_dq]
								    [pup][dq]
								    [0] =
								    curr_adll;
						}
#endif
						/* Found start of window */
						pup_adll_limit_state[pup] =
						    PUP_ADLL_LIMITS_STATE_PASS;

						/* Keep min / max limit value */
						if (is_tx == 0) {
							/* RX - found low limit */
							if (centralization_low_limit[pup] <= curr_adll)
								centralization_low_limit
								    [pup] =
								    curr_adll;
						} else {
							/* TX - found high limit */
							if (centralization_high_limit[pup] >= curr_adll)
								centralization_high_limit
								    [pup] =
								    curr_adll;
						}
					}
				}
			}

			if (unlock_pup == 0) {
				/* Found limit to all pups */
				DEBUG_DQS_FULL_S("DDR3 - DQS Find Limits - found PUP limit\n");
				break;
			}

			/*
			 * Increment / decrement (Move to right / left
			 * one phase - ADLL) dqs RX / TX delay (for all un
			 * lock pups
			 */
			if (is_tx == 0)
				curr_adll++;
			else
				curr_adll--;
		}

		if (unlock_pup != 0) {
			/*
			 * Found pups that didn't reach to the end of the
			 * state machine
			 */
			DEBUG_DQS_C("DDR3 - DQS Find Limits - Pups that didn't reached end of the state machine: ",
				    unlock_pup, 1);

			for (pup = 0; pup < max_pup; pup++) {
				if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
					if (pup_adll_limit_state[pup] ==
					    PUP_ADLL_LIMITS_STATE_FAIL) {
						/* ERROR - found fail for all window size */
						DEBUG_DQS_S("DDR3 - DQS Find Limits - Got FAIL for the complete range on pup - ");
						DEBUG_DQS_D(pup, 1);
						DEBUG_DQS_C(" victim DQ ",
							    victim_dq, 1);

						/* For debug - set min limit to illegal limit */
						centralization_low_limit[pup]
							= ADLL_ERROR;
						/*
						 * In case the pup is in mode
						 * PASS - the limit is the min
						 * / max adll, no need to
						 * update because of the results
						 * array default value
						 */
						return MV_DDR3_TRAINING_ERR_PUP_RANGE;
					}
				}
			}
		}
	}

	DEBUG_DQS_S("DDR3 - DQS Find Limits - DQ values per victim results:\n");
	for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
		for (pup = 0; pup < max_pup; pup++) {
			DEBUG_DQS_S("Victim DQ-");
			DEBUG_DQS_D(victim_dq, 1);
			DEBUG_DQS_S(", PUP-");
			DEBUG_DQS_D(pup, 1);
			for (dq = 0; dq < DQ_NUM; dq++) {
				DEBUG_DQS_S(", DQ-");
				DEBUG_DQS_D(dq, 1);
				DEBUG_DQS_S(",S-");
				DEBUG_DQS_D(analog_pbs[victim_dq][pup][dq]
					    [0], 2);
				DEBUG_DQS_S(",E-");
				DEBUG_DQS_D(analog_pbs[victim_dq][pup][dq]
					    [1], 2);

				if (is_tx == 0) {
					if (analog_pbs[victim_dq][pup][dq][0]
					    > analog_pbs_sum[pup][dq][0])
						analog_pbs_sum[pup][dq][0] =
						    analog_pbs[victim_dq][pup]
						    [dq][0];
					if (analog_pbs[victim_dq][pup][dq][1]
					    < analog_pbs_sum[pup][dq][1])
						analog_pbs_sum[pup][dq][1] =
						    analog_pbs[victim_dq][pup]
						    [dq][1];
				} else {
					if (analog_pbs[victim_dq][pup][dq][0]
					    < analog_pbs_sum[pup][dq][0])
						analog_pbs_sum[pup][dq][0] =
						    analog_pbs[victim_dq][pup]
						    [dq][0];
					if (analog_pbs[victim_dq][pup][dq][1]
					    > analog_pbs_sum[pup][dq][1])
						analog_pbs_sum[pup][dq][1] =
						    analog_pbs[victim_dq][pup]
						    [dq][1];
				}
			}
			DEBUG_DQS_S("\n");
		}
	}

	if (ddr3_get_log_level() >= MV_LOG_LEVEL_3) {
		u32 dq;

		DEBUG_PER_DQ_S("\n########## LOG LEVEL 3(Windows margins per-DQ) ##########\n");
		if (is_tx) {
			DEBUG_PER_DQ_C("DDR3 - TX  CS: ", cs, 1);
		} else {
			DEBUG_PER_DQ_C("DDR3 - RX  CS: ", cs, 1);
		}

		if (ecc == 0) {
			DEBUG_PER_DQ_S("\n DATA RESULTS:\n");
		} else {
			DEBUG_PER_DQ_S("\n ECC RESULTS:\n");
		}

		/* Since all dq has the same value we take 0 as representive */
		dq = 0;
		for (pup = 0; pup < max_pup; pup++) {
			if (ecc == 0) {
				DEBUG_PER_DQ_S("\nBYTE:");
				DEBUG_PER_DQ_D(pup, 1);
				DEBUG_PER_DQ_S("\n");
			} else {
				DEBUG_PER_DQ_S("\nECC BYTE:\n");
			}
			DEBUG_PER_DQ_S("  DQ's        LOW       HIGH       WIN-SIZE\n");
			DEBUG_PER_DQ_S("============================================\n");
			for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
				if (ecc == 0) {
					DEBUG_PER_DQ_S("DQ[");
					DEBUG_PER_DQ_DD((victim_dq +
							 DQ_NUM * pup), 2);
					DEBUG_PER_DQ_S("]");
				} else {
					DEBUG_PER_DQ_S("CB[");
					DEBUG_PER_DQ_DD(victim_dq, 2);
					DEBUG_PER_DQ_S("]");
				}
				if (is_tx) {
					DEBUG_PER_DQ_S("      0x");
					DEBUG_PER_DQ_D(analog_pbs[victim_dq][pup][dq][1], 2);	/* low value */
					DEBUG_PER_DQ_S("        0x");
					DEBUG_PER_DQ_D(analog_pbs[victim_dq][pup][dq][0], 2);	/* high value */
					DEBUG_PER_DQ_S("        0x");
					DEBUG_PER_DQ_D(analog_pbs[victim_dq][pup][dq][0] - analog_pbs[victim_dq][pup][dq][1], 2);	/* win-size */
				} else {
					DEBUG_PER_DQ_S("     0x");
					DEBUG_PER_DQ_D(analog_pbs[victim_dq][pup][dq][0], 2);	/* low value */
					DEBUG_PER_DQ_S("       0x");
					DEBUG_PER_DQ_D((analog_pbs[victim_dq][pup][dq][1] - 1), 2);	/* high value */
					DEBUG_PER_DQ_S("       0x");
					DEBUG_PER_DQ_D(analog_pbs[victim_dq][pup][dq][1] - analog_pbs[victim_dq][pup][dq][0], 2);	/* win-size */
				}
				DEBUG_PER_DQ_S("\n");
			}
		}
		DEBUG_PER_DQ_S("\n");
	}

	if (is_tx) {
		DEBUG_DQS_S("DDR3 - DQS TX - Find Limits - DQ values Summary:\n");
	} else {
		DEBUG_DQS_S("DDR3 - DQS RX - Find Limits - DQ values Summary:\n");
	}

	for (pup = 0; pup < max_pup; pup++) {
		DEBUG_DQS_S("PUP-");
		DEBUG_DQS_D(pup, 1);
		for (dq = 0; dq < DQ_NUM; dq++) {
			DEBUG_DQS_S(", DQ-");
			DEBUG_DQS_D(dq, 1);
			DEBUG_DQS_S(",S-");
			DEBUG_DQS_D(analog_pbs_sum[pup][dq][0], 2);
			DEBUG_DQS_S(",E-");
			DEBUG_DQS_D(analog_pbs_sum[pup][dq][1], 2);
		}
		DEBUG_DQS_S("\n");
	}

	if (is_tx) {
		DEBUG_DQS_S("DDR3 - DQS TX - Find Limits - DQ values Summary:\n");
	} else {
		DEBUG_DQS_S("DDR3 - DQS RX - Find Limits - DQ values Summary:\n");
	}

	for (pup = 0; pup < max_pup; pup++) {
		if (max_pup == 1) {
			/* For ECC PUP */
			DEBUG_DQS_S("DDR3 - DQS8");
		} else {
			DEBUG_DQS_S("DDR3 - DQS");
			DEBUG_DQS_D(pup, 1);
		}

		for (dq = 0; dq < DQ_NUM; dq++) {
			DEBUG_DQS_S(", DQ-");
			DEBUG_DQS_D(dq, 1);
			DEBUG_DQS_S("::S-");
			DEBUG_DQS_D(analog_pbs_sum[pup][dq][0], 2);
			DEBUG_DQS_S(",E-");
			DEBUG_DQS_D(analog_pbs_sum[pup][dq][1], 2);
		}
		DEBUG_DQS_S("\n");
	}

	DEBUG_DQS_S("DDR3 - DQS Find Limits - Ended\n");

	return MV_OK;
}

/*
 * Name:     ddr3_check_window_limits
 * Desc:     Check window High & Low limits.
 * Args:     pup                pup index
 *           high_limit           window high limit
 *           low_limit            window low limit
 *           is_tx                Indicate whether Rx or Tx
 *           size_valid          Indicate whether window size is valid
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_check_window_limits(u32 pup, int high_limit, int low_limit, int is_tx,
			     int *size_valid)
{
	DEBUG_DQS_FULL_S("DDR3 - DQS Check Win Limits - Starting\n");

	if (low_limit > high_limit) {
		DEBUG_DQS_S("DDR3 - DQS Check Win Limits - Pup ");
		DEBUG_DQS_D(pup, 1);
		DEBUG_DQS_S(" Low Limit grater than High Limit\n");
		*size_valid = 0;
		return MV_OK;
	}

	/*
	 * Check that window size is valid, if not it was probably false pass
	 * before
	 */
	if ((high_limit - low_limit) < MIN_WIN_SIZE) {
		/*
		 * Since window size is too small probably there was false
		 * pass
		 */
		*size_valid = 0;

		DEBUG_DQS_S("DDR3 - DQS Check Win Limits - Pup ");
		DEBUG_DQS_D(pup, 1);
		DEBUG_DQS_S(" Window size is smaller than MIN_WIN_SIZE\n");

	} else if ((high_limit - low_limit) > ADLL_MAX) {
		*size_valid = 0;

		DEBUG_DQS_S("DDR3 - DQS Check Win Limits - Pup ");
		DEBUG_DQS_D(pup, 1);
		DEBUG_DQS_S
		    (" Window size is bigger than max ADLL taps (31)  Exiting.\n");

		return MV_FAIL;

	} else {
		*size_valid = 1;

		DEBUG_DQS_FULL_S("DDR3 - DQS Check Win Limits - Pup ");
		DEBUG_DQS_FULL_D(pup, 1);
		DEBUG_DQS_FULL_C(" window size is ", (high_limit - low_limit),
				 2);
	}

	return MV_OK;
}

/*
 * Name:     ddr3_center_calc
 * Desc:     Execute the calculate the center of windows phase.
 * Args:     pDram Info
 *           is_tx             Indicate whether Rx or Tx
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_center_calc(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
			    int is_tx)
{
	/* bit array of pups that need specail search */
	u32 special_pattern_i_pup = 0;
	u32 special_pattern_ii_pup = 0;
	u32 pup;
	u32 max_pup;

	max_pup = (ecc + (1 - ecc) * dram_info->num_of_std_pups);

	for (pup = 0; pup < max_pup; pup++) {
		if (is_tx == 0) {
			/* Check special pattern I */
			/*
			 * Special pattern Low limit search - relevant only
			 * for Rx, win size < threshold and low limit = 0
			 */
			if (((centralization_high_limit[pup] -
			      centralization_low_limit[pup]) < VALID_WIN_THRS)
			    && (centralization_low_limit[pup] == MIN_DELAY))
				special_pattern_i_pup |= (1 << pup);

			/* Check special pattern II */
			/*
			 * Special pattern High limit search - relevant only
			 * for Rx, win size < threshold and high limit = 31
			 */
			if (((centralization_high_limit[pup] -
			      centralization_low_limit[pup]) < VALID_WIN_THRS)
			    && (centralization_high_limit[pup] == MAX_DELAY))
				special_pattern_ii_pup |= (1 << pup);
		}
	}

	/* Run special pattern Low limit search - for relevant pup */
	if (special_pattern_i_pup != 0) {
		DEBUG_DQS_S("DDR3 - DQS Center Calc - Entering special pattern I for Low limit search\n");
		if (MV_OK !=
		    ddr3_special_pattern_i_search(dram_info, cs, ecc, is_tx,
					      special_pattern_i_pup))
			return MV_DDR3_TRAINING_ERR_DQS_LOW_LIMIT_SEARCH;
	}

	/* Run special pattern High limit search - for relevant pup */
	if (special_pattern_ii_pup != 0) {
		DEBUG_DQS_S("DDR3 - DQS Center Calc - Entering special pattern II for High limit search\n");
		if (MV_OK !=
		    ddr3_special_pattern_ii_search(dram_info, cs, ecc, is_tx,
						   special_pattern_ii_pup))
			return MV_DDR3_TRAINING_ERR_DQS_HIGH_LIMIT_SEARCH;
	}

	/* Set adll to center = (General_High_limit + General_Low_limit)/2 */
	return ddr3_set_dqs_centralization_results(dram_info, cs, ecc, is_tx);
}

/*
 * Name:     ddr3_special_pattern_i_search
 * Desc:     Execute special pattern low limit search.
 * Args:
 *           special_pattern_pup  The pups that need the special search
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_special_pattern_i_search(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
				  int is_tx, u32 special_pattern_pup)
{
	u32 victim_dq;		/* loop index - victim DQ */
	u32 adll_idx;
	u32 pup;
	u32 unlock_pup;		/* bit array of the unlock pups  */
	u32 first_fail;	/* bit array - of pups that  get first fail */
	u32 new_lockup_pup;	/* bit array of compare failed pups */
	u32 pass_pup;		/* bit array of compare pass pup */
	u32 sdram_offset;
	u32 max_pup;
	u32 comp_val;
	u32 special_res[MAX_PUP_NUM];	/* hold tmp results */

	DEBUG_DQS_S("DDR3 - DQS - Special Pattern I Search - Starting\n");

	max_pup = ecc + (1 - ecc) * dram_info->num_of_std_pups;

	/* Init the temporary results to max ADLL value */
	for (pup = 0; pup < max_pup; pup++)
		special_res[pup] = ADLL_MAX;

	/* Run special pattern for all DQ - use the same pattern */
	for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
		unlock_pup = special_pattern_pup;
		first_fail = 0;

		sdram_offset = cs * SDRAM_CS_SIZE + SDRAM_DQS_RX_OFFS +
			LEN_KILLER_PATTERN * 4 * victim_dq;

		for (pup = 0; pup < max_pup; pup++) {
			/* Set adll value per PUP. adll = high limit per pup */
			if (IS_PUP_ACTIVE(unlock_pup, pup)) {
				/* only for pups that need special search */
				ddr3_write_pup_reg(PUP_DQS_RD, cs,
						   pup + (ecc * ECC_PUP), 0,
						   centralization_high_limit
						   [pup]);
			}
		}

		adll_idx = 0;
		do {
			/*
			 * Perform read and compare simultaneously for all
			 * un-locked MC use the special pattern mask
			 */
			new_lockup_pup = 0;

			if (MV_OK !=
			    ddr3_sdram_dqs_compare(dram_info, unlock_pup,
						   &new_lockup_pup,
						   special_pattern
						   [victim_dq],
						   LEN_SPECIAL_PATTERN,
						   sdram_offset, 0,
						   0, NULL, 1))
				return MV_FAIL;

			DEBUG_DQS_S("DDR3 - DQS - Special I - ADLL value is: ");
			DEBUG_DQS_D(adll_idx, 2);
			DEBUG_DQS_S(", UnlockPup: ");
			DEBUG_DQS_D(unlock_pup, 2);
			DEBUG_DQS_S(", NewLockPup: ");
			DEBUG_DQS_D(new_lockup_pup, 2);
			DEBUG_DQS_S("\n");

			if (unlock_pup != new_lockup_pup)
				DEBUG_DQS_S("DDR3 - DQS - Special I - Some Pup passed!\n");

			/* Search for pups with passed compare & already fail */
			pass_pup = first_fail & ~new_lockup_pup & unlock_pup;
			first_fail |= new_lockup_pup;
			unlock_pup &= ~pass_pup;

			/* Get pass pups */
			if (pass_pup != 0) {
				for (pup = 0; pup < max_pup; pup++) {
					if (IS_PUP_ACTIVE(pass_pup, pup) ==
					    1) {
						/* If pup passed and has first fail = 1 */
						/* keep min value of ADLL max value - current adll */
						/* (centralization_high_limit[pup] + adll_idx) = current adll !!! */
						comp_val =
						    (ADLL_MAX -
						     (centralization_high_limit
						      [pup] + adll_idx));

						DEBUG_DQS_C
						    ("DDR3 - DQS - Special I - Pup - ",
						     pup, 1);
						DEBUG_DQS_C
						    (" comp_val = ",
						     comp_val, 2);

						if (comp_val <
						    special_res[pup]) {
							special_res[pup] =
							    comp_val;
							centralization_low_limit
							    [pup] =
							    (-1) *
							    comp_val;

							DEBUG_DQS_C
							    ("DDR3 - DQS - Special I - Pup - ",
							     pup, 1);
							DEBUG_DQS_C
							    (" Changed Low limit to ",
							     centralization_low_limit
							     [pup], 2);
						}
					}
				}
			}

			/*
			 * Did all PUP found missing window?
			 * Check for each pup if adll (different for each pup)
			 * reach maximum if reach max value - lock the pup
			 * if not - increment (Move to right one phase - ADLL)
			 * dqs RX delay
			 */
			adll_idx++;
			for (pup = 0; pup < max_pup; pup++) {
				if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
					/* Check only unlocked pups */
					if ((centralization_high_limit[pup] +
					     adll_idx) >= ADLL_MAX) {
						/* reach maximum - lock the pup */
						DEBUG_DQS_C("DDR3 - DQS - Special I - reach maximum - lock pup ",
							    pup, 1);
						unlock_pup &= ~(1 << pup);
					} else {
						/* Didn't reach maximum - increment ADLL */
						ddr3_write_pup_reg(PUP_DQS_RD,
								   cs,
								   pup +
								   (ecc *
								    ECC_PUP), 0,
								   (centralization_high_limit
								    [pup] +
								    adll_idx));
					}
				}
			}
		} while (unlock_pup != 0);
	}

	return MV_OK;
}

/*
 * Name:     ddr3_special_pattern_ii_search
 * Desc:     Execute special pattern high limit search.
 * Args:
 *           special_pattern_pup  The pups that need the special search
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_special_pattern_ii_search(MV_DRAM_INFO *dram_info, u32 cs, u32 ecc,
				   int is_tx, u32 special_pattern_pup)
{
	u32 victim_dq;		/* loop index - victim DQ */
	u32 adll_idx;
	u32 pup;
	u32 unlock_pup;		/* bit array of the unlock pups  */
	u32 first_fail;	/* bit array - of pups that  get first fail */
	u32 new_lockup_pup;	/* bit array of compare failed pups */
	u32 pass_pup;		/* bit array of compare pass pup */
	u32 sdram_offset;
	u32 max_pup;
	u32 comp_val;
	u32 special_res[MAX_PUP_NUM];	/* hold tmp results */

	DEBUG_DQS_S("DDR3 - DQS - Special Pattern II Search - Starting\n");

	max_pup = (ecc + (1 - ecc) * dram_info->num_of_std_pups);

	/* init the tmporary results to max ADLL value */
	for (pup = 0; pup < max_pup; pup++)
		special_res[pup] = ADLL_MAX;

	sdram_offset = cs * SDRAM_CS_SIZE + SDRAM_DQS_RX_OFFS;

	/* run special pattern for all DQ - use the same pattern */
	for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
		unlock_pup = special_pattern_pup;
		first_fail = 0;

		for (pup = 0; pup < max_pup; pup++) {
			/* Set adll value per PUP. adll = 0 */
			if (IS_PUP_ACTIVE(unlock_pup, pup)) {
				/* Only for pups that need special search */
				ddr3_write_pup_reg(PUP_DQS_RD, cs,
						   pup + (ecc * ECC_PUP), 0,
						   ADLL_MIN);
			}
		}

		adll_idx = 0;
		do {
			/*
			 * Perform read and compare simultaneously for all
			 * un-locked MC use the special pattern mask
			 */
			new_lockup_pup = 0;

			if (MV_OK != ddr3_sdram_dqs_compare(
				    dram_info, unlock_pup, &new_lockup_pup,
				    special_pattern[victim_dq],
				    LEN_SPECIAL_PATTERN,
				    sdram_offset, 0, 0, NULL, 0))
				return MV_FAIL;

			DEBUG_DQS_S("DDR3 - DQS - Special II - ADLL value is ");
			DEBUG_DQS_D(adll_idx, 2);
			DEBUG_DQS_S("unlock_pup ");
			DEBUG_DQS_D(unlock_pup, 1);
			DEBUG_DQS_S("new_lockup_pup ");
			DEBUG_DQS_D(new_lockup_pup, 1);
			DEBUG_DQS_S("\n");

			if (unlock_pup != new_lockup_pup) {
				DEBUG_DQS_S("DDR3 - DQS - Special II - Some Pup passed!\n");
			}

			/* Search for pups with passed compare & already fail */
			pass_pup = first_fail & ~new_lockup_pup & unlock_pup;
			first_fail |= new_lockup_pup;
			unlock_pup &= ~pass_pup;

			/* Get pass pups */
			if (pass_pup != 0) {
				for (pup = 0; pup < max_pup; pup++) {
					if (IS_PUP_ACTIVE(pass_pup, pup) ==
					    1) {
						/* If pup passed and has first fail = 1 */
						/* keep min value of ADLL max value - current adll */
						/* (adll_idx) = current adll !!! */
						comp_val = adll_idx;

						DEBUG_DQS_C("DDR3 - DQS - Special II - Pup - ",
							    pup, 1);
						DEBUG_DQS_C(" comp_val = ",
							    comp_val, 1);

						if (comp_val <
						    special_res[pup]) {
							special_res[pup] =
							    comp_val;
							centralization_high_limit
							    [pup] =
							    ADLL_MAX +
							    comp_val;

							DEBUG_DQS_C
							    ("DDR3 - DQS - Special II - Pup - ",
							     pup, 1);
							DEBUG_DQS_C
							    (" Changed High limit to ",
							     centralization_high_limit
							     [pup], 2);
						}
					}
				}
			}

			/*
			 * Did all PUP found missing window?
			 * Check for each pup if adll (different for each pup)
			 * reach maximum if reach max value - lock the pup
			 * if not - increment (Move to right one phase - ADLL)
			 * dqs RX delay
			 */
			adll_idx++;
			for (pup = 0; pup < max_pup; pup++) {
				if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
					/* Check only unlocked pups */
					if ((adll_idx) >= ADLL_MAX) {
						/* Reach maximum - lock the pup */
						DEBUG_DQS_C("DDR3 - DQS - Special II - reach maximum - lock pup ",
							    pup, 1);
						unlock_pup &= ~(1 << pup);
					} else {
						/* Didn't reach maximum - increment ADLL */
						ddr3_write_pup_reg(PUP_DQS_RD,
								   cs,
								   pup +
								   (ecc *
								    ECC_PUP), 0,
								   (adll_idx));
					}
				}
			}
		} while (unlock_pup != 0);
	}

	return MV_OK;
}

/*
 * Name:     ddr3_set_dqs_centralization_results
 * Desc:     Set to HW the DQS centralization phase results.
 * Args:
 *           is_tx             Indicates whether to set Tx or RX results
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_set_dqs_centralization_results(MV_DRAM_INFO *dram_info, u32 cs,
					u32 ecc, int is_tx)
{
	u32 pup, pup_num;
	int addl_val;
	u32 max_pup;

	max_pup = (ecc + (1 - ecc) * dram_info->num_of_std_pups);

	DEBUG_DQS_RESULTS_S("\n############ LOG LEVEL 2(Windows margins) ############\n");

	if (is_tx) {
		DEBUG_DQS_RESULTS_C("DDR3 - DQS TX - Set Dqs Centralization Results - CS: ",
				    cs, 1);
	} else {
		DEBUG_DQS_RESULTS_C("DDR3 - DQS RX - Set Dqs Centralization Results - CS: ",
				    cs, 1);
	}

	/* Set adll to center = (General_High_limit + General_Low_limit)/2 */
	DEBUG_DQS_RESULTS_S("\nDQS    LOW     HIGH     WIN-SIZE      Set\n");
	DEBUG_DQS_RESULTS_S("==============================================\n");
	for (pup = 0; pup < max_pup; pup++) {
		addl_val = (centralization_high_limit[pup] +
			    centralization_low_limit[pup]) / 2;

		pup_num = pup * (1 - ecc) + ecc * ECC_PUP;

		DEBUG_DQS_RESULTS_D(pup_num, 1);
		DEBUG_DQS_RESULTS_S("     0x");
		DEBUG_DQS_RESULTS_D(centralization_low_limit[pup], 2);
		DEBUG_DQS_RESULTS_S("      0x");
		DEBUG_DQS_RESULTS_D(centralization_high_limit[pup], 2);
		DEBUG_DQS_RESULTS_S("      0x");
		DEBUG_DQS_RESULTS_D(centralization_high_limit[pup] -
				    centralization_low_limit[pup], 2);
		DEBUG_DQS_RESULTS_S("       0x");
		DEBUG_DQS_RESULTS_D(addl_val, 2);
		DEBUG_DQS_RESULTS_S("\n");

		if (addl_val < ADLL_MIN) {
			addl_val = ADLL_MIN;
			DEBUG_DQS_RESULTS_S("DDR3 - DQS - Setting ADLL value for Pup to MIN (since it was lower than 0)\n");
		}

		if (addl_val > ADLL_MAX) {
			addl_val = ADLL_MAX;
			DEBUG_DQS_RESULTS_S("DDR3 - DQS - Setting ADLL value for Pup to MAX (since it was higher than 31)\n");
		}

		if (is_tx) {
			ddr3_write_pup_reg(PUP_DQS_WR, cs, pup_num, 0,
					   addl_val +
					   dram_info->wl_val[cs][pup_num][D]);
		} else {
			ddr3_write_pup_reg(PUP_DQS_RD, cs, pup_num, 0,
					   addl_val);
		}
	}

	return MV_OK;
}

/*
 * Set training patterns
 */
int ddr3_load_dqs_patterns(MV_DRAM_INFO *dram_info)
{
	u32 cs, cs_count, cs_tmp, victim_dq;
	u32 sdram_addr;
	u32 *pattern_ptr;

	/* Loop for each CS */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			cs_count = 0;
			for (cs_tmp = 0; cs_tmp < cs; cs_tmp++) {
				if (dram_info->cs_ena & (1 << cs_tmp))
					cs_count++;
			}

			/* Init killer pattern */
			sdram_addr = (cs_count * (SDRAM_CS_SIZE + 1) +
				      SDRAM_DQS_RX_OFFS);
			for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
				pattern_ptr = ddr3_dqs_choose_pattern(dram_info,
								      victim_dq);
				if (MV_OK != ddr3_sdram_dqs_compare(
					    dram_info, (u32)NULL, NULL,
					    pattern_ptr, LEN_KILLER_PATTERN,
					    sdram_addr + LEN_KILLER_PATTERN *
					    4 * victim_dq, 1, 0, NULL,
					    0))
					return MV_DDR3_TRAINING_ERR_DQS_PATTERN;
			}

			/* Init special-killer pattern */
			sdram_addr = (cs_count * (SDRAM_CS_SIZE + 1) +
				      SDRAM_DQS_RX_SPECIAL_OFFS);
			for (victim_dq = 0; victim_dq < DQ_NUM; victim_dq++) {
				if (MV_OK != ddr3_sdram_dqs_compare(
					    dram_info, (u32)NULL, NULL,
					    special_pattern[victim_dq],
					    LEN_KILLER_PATTERN, sdram_addr +
					    LEN_KILLER_PATTERN * 4 * victim_dq,
					    1, 0, NULL, 0))
					return MV_DDR3_TRAINING_ERR_DQS_PATTERN;
			}
		}
	}

	return MV_OK;
}
