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
#define DEBUG_PBS_FULL_C(s, d, l) \
	DEBUG_PBS_FULL_S(s); DEBUG_PBS_FULL_D(d, l); DEBUG_PBS_FULL_S("\n")
#define DEBUG_PBS_C(s, d, l) \
	DEBUG_PBS_S(s); DEBUG_PBS_D(d, l); DEBUG_PBS_S("\n")

#ifdef MV_DEBUG_PBS
#define DEBUG_PBS_S(s)			puts(s)
#define DEBUG_PBS_D(d, l)		printf("%x", d)
#else
#define DEBUG_PBS_S(s)
#define DEBUG_PBS_D(d, l)
#endif

#ifdef MV_DEBUG_FULL_PBS
#define DEBUG_PBS_FULL_S(s)		puts(s)
#define DEBUG_PBS_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_PBS_FULL_S(s)
#define DEBUG_PBS_FULL_D(d, l)
#endif

#if defined(MV88F78X60) || defined(MV88F672X)

/* Temp array for skew data storage */
static u32 skew_array[(MAX_PUP_NUM) * DQ_NUM] = { 0 };

/* PBS locked dq (per pup) */
extern u32 pbs_locked_dq[MAX_PUP_NUM][DQ_NUM];
extern u32 pbs_locked_dm[MAX_PUP_NUM];
extern u32 pbs_locked_value[MAX_PUP_NUM][DQ_NUM];

#if defined(MV88F672X)
extern u32 pbs_pattern[2][LEN_16BIT_PBS_PATTERN];
extern u32 pbs_pattern_32b[2][LEN_PBS_PATTERN];
#else
extern u32 pbs_pattern_32b[2][LEN_PBS_PATTERN];
extern u32 pbs_pattern_64b[2][LEN_PBS_PATTERN];
#endif

extern u32 pbs_dq_mapping[PUP_NUM_64BIT + 1][DQ_NUM];

static int ddr3_tx_shift_dqs_adll_step_before_fail(MV_DRAM_INFO *dram_info,
		u32 cur_pup, u32 pbs_pattern_idx, u32 ecc);
static int ddr3_rx_shift_dqs_to_first_fail(MV_DRAM_INFO *dram_info, u32 cur_pup,
		u32 pbs_pattern_idx, u32 ecc);
static int ddr3_pbs_per_bit(MV_DRAM_INFO *dram_info, int *start_over, int is_tx,
		u32 *pcur_pup, u32 pbs_pattern_idx, u32 ecc);
static int ddr3_set_pbs_results(MV_DRAM_INFO *dram_info, int is_tx);
static void ddr3_pbs_write_pup_dqs_reg(u32 cs, u32 pup, u32 dqs_delay);

/*
 * Name:     ddr3_pbs_tx
 * Desc:     Execute the PBS TX phase.
 * Args:     dram_info   ddr3 training information struct
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_pbs_tx(MV_DRAM_INFO *dram_info)
{
	/* Array of Deskew results */

	/*
	 * Array to hold the total sum of skew from all iterations
	 * (for average purpose)
	 */
	u32 skew_sum_array[MAX_PUP_NUM][DQ_NUM] = { {0} };

	/*
	 * Array to hold the total average skew from both patterns
	 * (for average purpose)
	 */
	u32 pattern_skew_array[MAX_PUP_NUM][DQ_NUM] = { {0} };

	u32 pbs_rep_time = 0;	/* counts number of loop in case of fail */
	/* bit array for unlock pups - used to repeat on the RX operation */
	u32 cur_pup;
	u32 max_pup;
	u32 pbs_retry;
	u32 pup, dq, pups, cur_max_pup, valid_pup, reg;
	u32 pattern_idx;
	u32 ecc;
	/* indicates whether we need to start the loop again */
	int start_over;

	DEBUG_PBS_S("DDR3 - PBS TX - Starting PBS TX procedure\n");

	pups = dram_info->num_of_total_pups;
	max_pup = dram_info->num_of_total_pups;

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_PBS_S("DDR3 - PBS RX - SW Override Enabled\n");

	reg = 1 << REG_DRAM_TRAINING_AUTO_OFFS;
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	/* Running twice for 2 different patterns. each patterns - 3 times */
	for (pattern_idx = 0; pattern_idx < COUNT_PBS_PATTERN; pattern_idx++) {
		DEBUG_PBS_C("DDR3 - PBS TX - Working with pattern - ",
			    pattern_idx, 1);

		/* Reset sum array */
		for (pup = 0; pup < pups; pup++) {
			for (dq = 0; dq < DQ_NUM; dq++)
				skew_sum_array[pup][dq] = 0;
		}

		/*
		 * Perform PBS several of times (3 for each pattern).
		 * At the end, we'll use the average
		 */
		/* If there is ECC, do each PBS again with mux change */
		for (pbs_retry = 0; pbs_retry < COUNT_PBS_REPEAT; pbs_retry++) {
			for (ecc = 0; ecc < (dram_info->ecc_ena + 1); ecc++) {

				/*
				 * This parameter stores the current PUP
				 * num - ecc mode dependent - 4-8 / 1 pups
				 */
				cur_max_pup = (1 - ecc) *
					dram_info->num_of_std_pups + ecc;

				if (ecc) {
					/* Only 1 pup in this case */
					valid_pup = 0x1;
				} else if (cur_max_pup > 4) {
					/* 64 bit - 8 pups */
					valid_pup = 0xFF;
				} else if (cur_max_pup == 4) {
					/* 32 bit - 4 pups */
					valid_pup = 0xF;
				} else {
					/* 16 bit - 2 pups */
					valid_pup = 0x3;
				}

				/* ECC Support - Switch ECC Mux on ecc=1 */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg |= (dram_info->ecc_ena * ecc <<
					REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

				if (ecc)
					DEBUG_PBS_S("DDR3 - PBS Tx - ECC Mux Enabled\n");
				else
					DEBUG_PBS_S("DDR3 - PBS Tx - ECC Mux Disabled\n");

				/* Init iteration values */
				/* Clear the locked DQs */
				for (pup = 0; pup < cur_max_pup; pup++) {
					for (dq = 0; dq < DQ_NUM; dq++) {
						pbs_locked_dq[
							pup + ecc *
							(max_pup - 1)][dq] =
							0;
					}
				}

				pbs_rep_time = 0;
				cur_pup = valid_pup;
				start_over = 0;

				/*
				 * Run loop On current Pattern and current
				 * pattern iteration (just to cover the false
				 * fail problem)
				 */
				do {
					DEBUG_PBS_S("DDR3 - PBS Tx - Pbs Rep Loop is ");
					DEBUG_PBS_D(pbs_rep_time, 1);
					DEBUG_PBS_S(", for Retry No.");
					DEBUG_PBS_D(pbs_retry, 1);
					DEBUG_PBS_S("\n");

					/* Set all PBS values to MIN (0) */
					DEBUG_PBS_S("DDR3 - PBS Tx - Set all PBS values to MIN\n");

					for (dq = 0; dq < DQ_NUM; dq++) {
						ddr3_write_pup_reg(
							PUP_PBS_TX +
							pbs_dq_mapping[pup *
								(1 - ecc) +
								ecc * ECC_PUP]
							[dq], CS0, (1 - ecc) *
							PUP_BC + ecc * ECC_PUP, 0,
							0);
					}

					/*
					 * Shift DQ ADLL right, One step before
					 * fail
					 */
					DEBUG_PBS_S("DDR3 - PBS Tx - ADLL shift right one phase before fail\n");

					if (MV_OK != ddr3_tx_shift_dqs_adll_step_before_fail
					    (dram_info, cur_pup, pattern_idx,
					     ecc))
						return MV_DDR3_TRAINING_ERR_PBS_ADLL_SHR_1PHASE;

					/* PBS For each bit */
					DEBUG_PBS_S("DDR3 - PBS Tx - perform PBS for each bit\n");

					/*
					 * In this stage - start_over = 0
					 */
					if (MV_OK != ddr3_pbs_per_bit(
						    dram_info, &start_over, 1,
						    &cur_pup, pattern_idx, ecc))
						return MV_DDR3_TRAINING_ERR_PBS_TX_PER_BIT;

				} while ((start_over == 1) &&
					 (++pbs_rep_time < COUNT_PBS_STARTOVER));

				if (pbs_rep_time == COUNT_PBS_STARTOVER &&
				    start_over == 1) {
					DEBUG_PBS_S("DDR3 - PBS Tx - FAIL - Adll reach max value\n");
					return MV_DDR3_TRAINING_ERR_PBS_TX_MAX_VAL;
				}

				DEBUG_PBS_FULL_C("DDR3 - PBS TX - values for iteration - ",
						 pbs_retry, 1);
				for (pup = 0; pup < cur_max_pup; pup++) {
					/*
					 * To minimize delay elements, inc
					 * from pbs value the min pbs val
					 */
					DEBUG_PBS_S("DDR3 - PBS - PUP");
					DEBUG_PBS_D((pup + (ecc * ECC_PUP)), 1);
					DEBUG_PBS_S(": ");

					for (dq = 0; dq < DQ_NUM; dq++) {
						/* Set skew value for all dq */
						/*
						 * Bit# Deskew <- Bit# Deskew -
						 * last / first  failing bit
						 * Deskew For all bits (per PUP)
						 * (minimize delay elements)
						 */
						DEBUG_PBS_S("DQ");
						DEBUG_PBS_D(dq, 1);
						DEBUG_PBS_S("-");
						DEBUG_PBS_D(skew_array
							    [((pup) * DQ_NUM) +
							     dq], 2);
						DEBUG_PBS_S(", ");
					}
					DEBUG_PBS_S("\n");
				}

				/*
				 * Collect the results we got on this trial
				 * of PBS
				 */
				for (pup = 0; pup < cur_max_pup; pup++) {
					for (dq = 0; dq < DQ_NUM; dq++) {
						skew_sum_array[pup + (ecc * (max_pup - 1))]
							[dq] += skew_array
							[((pup) * DQ_NUM) + dq];
					}
				}

				/* ECC Support - Disable ECC MUX */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
			}
		}

		DEBUG_PBS_C("DDR3 - PBS TX - values for current pattern - ",
			    pattern_idx, 1);
		for (pup = 0; pup < max_pup; pup++) {
			/*
			 * To minimize delay elements, inc from pbs value the
			 * min pbs val
			 */
			DEBUG_PBS_S("DDR3 - PBS - PUP");
			DEBUG_PBS_D(pup, 1);
			DEBUG_PBS_S(": ");

			for (dq = 0; dq < DQ_NUM; dq++) {
				/* set skew value for all dq */
				/* Bit# Deskew <- Bit# Deskew - last / first  failing bit Deskew For all bits (per PUP) (minimize delay elements) */
				DEBUG_PBS_S("DQ");
				DEBUG_PBS_D(dq, 1);
				DEBUG_PBS_S("-");
				DEBUG_PBS_D(skew_sum_array[pup][dq] /
					    COUNT_PBS_REPEAT, 2);
				DEBUG_PBS_S(", ");
			}
			DEBUG_PBS_S("\n");
		}

		/*
		 * Calculate the average skew for current pattern for each
		 * pup and each bit
		 */
		DEBUG_PBS_C("DDR3 - PBS TX - Average for pattern - ",
			    pattern_idx, 1);

		for (pup = 0; pup < max_pup; pup++) {
			/*
			 * FOR ECC only :: found min and max value for current
			 * pattern skew array
			 */
			/* Loop for all dqs */
			for (dq = 0; dq < DQ_NUM; dq++) {
				pattern_skew_array[pup][dq] +=
					(skew_sum_array[pup][dq] /
					 COUNT_PBS_REPEAT);
			}
		}
	}

	/* Calculate the average skew */
	for (pup = 0; pup < max_pup; pup++) {
		for (dq = 0; dq < DQ_NUM; dq++)
			skew_array[((pup) * DQ_NUM) + dq] =
				pattern_skew_array[pup][dq] / COUNT_PBS_PATTERN;
	}

	DEBUG_PBS_S("DDR3 - PBS TX - Average for all patterns:\n");
	for (pup = 0; pup < max_pup; pup++) {
		/*
		 * To minimize delay elements, inc from pbs value the min
		 * pbs val
		 */
		DEBUG_PBS_S("DDR3 - PBS - PUP");
		DEBUG_PBS_D(pup, 1);
		DEBUG_PBS_S(": ");

		for (dq = 0; dq < DQ_NUM; dq++) {
			/* Set skew value for all dq */
			/*
			 * Bit# Deskew <- Bit# Deskew - last / first
			 * failing bit Deskew For all bits (per PUP)
			 * (minimize delay elements)
			 */
			DEBUG_PBS_S("DQ");
			DEBUG_PBS_D(dq, 1);
			DEBUG_PBS_S("-");
			DEBUG_PBS_D(skew_array[(pup * DQ_NUM) + dq], 2);
			DEBUG_PBS_S(", ");
		}
		DEBUG_PBS_S("\n");
	}

	/* Return ADLL to default value */
	for (pup = 0; pup < max_pup; pup++) {
		if (pup == (max_pup - 1) && dram_info->ecc_ena)
			pup = ECC_PUP;
		ddr3_pbs_write_pup_dqs_reg(CS0, pup, INIT_WL_DELAY);
	}

	/* Set averaged PBS results */
	ddr3_set_pbs_results(dram_info, 1);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_1_ADDR) |
		(1 << REG_DRAM_TRAINING_1_TRNBPOINT_OFFS);
	reg_write(REG_DRAM_TRAINING_1_ADDR, reg);

	DEBUG_PBS_S("DDR3 - PBS Tx - PBS TX ended successfuly\n");

	return MV_OK;
}

/*
 * Name:     ddr3_tx_shift_dqs_adll_step_before_fail
 * Desc:     Execute the Tx shift DQ phase.
 * Args:     dram_info            ddr3 training information struct
 *           cur_pup              bit array of the function active pups.
 *           pbs_pattern_idx      Index of PBS pattern
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_tx_shift_dqs_adll_step_before_fail(MV_DRAM_INFO *dram_info,
						   u32 cur_pup,
						   u32 pbs_pattern_idx, u32 ecc)
{
	u32 unlock_pup;		/* bit array of unlock pups  */
	u32 new_lockup_pup;	/* bit array of compare failed pups */
	u32 adll_val = 4;	/* INIT_WL_DELAY */
	u32 cur_max_pup, pup;
	u32 dqs_dly_set[MAX_PUP_NUM] = { 0 };
	u32 *pattern_ptr;

	/* Choose pattern */
	switch (dram_info->ddr_width) {
#if defined(MV88F672X)
	case 16:
		pattern_ptr = (u32 *)&pbs_pattern[pbs_pattern_idx];
		break;
#endif
	case 32:
		pattern_ptr = (u32 *)&pbs_pattern_32b[pbs_pattern_idx];
		break;
#if defined(MV88F78X60)
	case 64:
		pattern_ptr = (u32 *)&pbs_pattern_64b[pbs_pattern_idx];
		break;
#endif
	default:
		return MV_FAIL;
	}

	/* Set current pup number */
	if (cur_pup == 0x1)	/* Ecc mode */
		cur_max_pup = 1;
	else
		cur_max_pup = dram_info->num_of_std_pups;

	unlock_pup = cur_pup;	/* '1' for each unlocked pup */

	/* Loop on all ADLL Vaules */
	do {
		/* Loop until found first fail */
		adll_val++;

		/*
		 * Increment (Move to right - ADLL) DQ TX delay
		 * (broadcast to all Data PUPs)
		 */
		for (pup = 0; pup < cur_max_pup; pup++)
			ddr3_pbs_write_pup_dqs_reg(CS0,
						   pup * (1 - ecc) +
						   ECC_PUP * ecc, adll_val);

		/*
		 * Write and Read, compare results (read was already verified)
		 */
		/* 0 - all locked */
		new_lockup_pup = 0;

		if (MV_OK != ddr3_sdram_compare(dram_info, unlock_pup,
						&new_lockup_pup,
						pattern_ptr, LEN_PBS_PATTERN,
						SDRAM_PBS_TX_OFFS, 1, 0,
						NULL,
						0))
			return MV_FAIL;

		unlock_pup &= ~new_lockup_pup;

		DEBUG_PBS_FULL_S("Shift DQS by 2 steps for PUPs: ");
		DEBUG_PBS_FULL_D(unlock_pup, 2);
		DEBUG_PBS_FULL_C(", Set ADLL value = ", adll_val, 2);

		/* If any PUP failed there is '1' to mark the PUP */
		if (new_lockup_pup != 0) {
			/*
			 * Decrement (Move Back to Left two steps - ADLL)
			 * DQ TX delay for current failed pups and save
			 */
			for (pup = 0; pup < cur_max_pup; pup++) {
				if (((new_lockup_pup >> pup) & 0x1) &&
				    dqs_dly_set[pup] == 0)
					dqs_dly_set[pup] = adll_val - 1;
			}
		}
	} while ((unlock_pup != 0) && (adll_val != ADLL_MAX));

	if (unlock_pup != 0) {
		DEBUG_PBS_FULL_S("DDR3 - PBS Tx - Shift DQ - Adll value reached maximum\n");

		for (pup = 0; pup < cur_max_pup; pup++) {
			if (((unlock_pup >> pup) & 0x1) &&
			    dqs_dly_set[pup] == 0)
				dqs_dly_set[pup] = adll_val - 1;
		}
	}

	DEBUG_PBS_FULL_C("PBS TX one step before fail last pups locked Adll ",
			 adll_val - 2, 2);

	/* Set the PUP DQS DLY Values */
	for (pup = 0; pup < cur_max_pup; pup++)
		ddr3_pbs_write_pup_dqs_reg(CS0, pup * (1 - ecc) + ECC_PUP * ecc,
					   dqs_dly_set[pup]);

	/* Found one phase before fail */
	return MV_OK;
}

/*
 * Name:     ddr3_pbs_rx
 * Desc:     Execute the PBS RX phase.
 * Args:     dram_info   ddr3 training information struct
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_pbs_rx(MV_DRAM_INFO *dram_info)
{
	/*
	 * Array to hold the total sum of skew from all iterations
	 * (for average purpose)
	 */
	u32 skew_sum_array[MAX_PUP_NUM][DQ_NUM] = { {0} };

	/*
	 * Array to hold the total average skew from both patterns
	 * (for average purpose)
	 */
	u32 pattern_skew_array[MAX_PUP_NUM][DQ_NUM] = { {0} };

	u32 pbs_rep_time = 0;	/* counts number of loop in case of fail */
	/* bit array for unlock pups - used to repeat on the RX operation */
	u32 cur_pup;
	u32 max_pup;
	u32 pbs_retry;
	u32 pup, dq, pups, cur_max_pup, valid_pup, reg;
	u32 pattern_idx;
	u32 ecc;
	/* indicates whether we need to start the loop again */
	int start_over;
	int status;

	DEBUG_PBS_S("DDR3 - PBS RX - Starting PBS RX procedure\n");

	pups = dram_info->num_of_total_pups;
	max_pup = dram_info->num_of_total_pups;

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_PBS_FULL_S("DDR3 - PBS RX - SW Override Enabled\n");

	reg = 1 << REG_DRAM_TRAINING_AUTO_OFFS;
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	/* Running twice for 2 different patterns. each patterns - 3 times */
	for (pattern_idx = 0; pattern_idx < COUNT_PBS_PATTERN; pattern_idx++) {
		DEBUG_PBS_FULL_C("DDR3 - PBS RX - Working with pattern - ",
				 pattern_idx, 1);

		/* Reset sum array */
		for (pup = 0; pup < pups; pup++) {
			for (dq = 0; dq < DQ_NUM; dq++)
				skew_sum_array[pup][dq] = 0;
		}

		/*
		 * Perform PBS several of times (3 for each pattern).
		 * At the end, we'll use the average
		 */
		/* If there is ECC, do each PBS again with mux change */
		for (pbs_retry = 0; pbs_retry < COUNT_PBS_REPEAT; pbs_retry++) {
			for (ecc = 0; ecc < (dram_info->ecc_ena + 1); ecc++) {
				/*
				 * This parameter stores the current PUP
				 * num - ecc mode dependent - 4-8 / 1 pups
				 */
				cur_max_pup = (1 - ecc) *
					dram_info->num_of_std_pups + ecc;

				if (ecc) {
					/* Only 1 pup in this case */
					valid_pup = 0x1;
				} else if (cur_max_pup > 4) {
					/* 64 bit - 8 pups */
					valid_pup = 0xFF;
				} else if (cur_max_pup == 4) {
					/* 32 bit - 4 pups */
					valid_pup = 0xF;
				} else {
					/* 16 bit - 2 pups */
					valid_pup = 0x3;
				}

				/* ECC Support - Switch ECC Mux on ecc=1 */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg |= (dram_info->ecc_ena * ecc <<
					REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

				if (ecc)
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - ECC Mux Enabled\n");
				else
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - ECC Mux Disabled\n");

				/* Init iteration values */
				/* Clear the locked DQs */
				for (pup = 0; pup < cur_max_pup; pup++) {
					for (dq = 0; dq < DQ_NUM; dq++) {
						pbs_locked_dq[
							pup + ecc * (max_pup - 1)][dq] =
							0;
					}
				}

				pbs_rep_time = 0;
				cur_pup = valid_pup;
				start_over = 0;

				/*
				 * Run loop On current Pattern and current
				 * pattern iteration (just to cover the false
				 * fail problem
				 */
				do {
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - Pbs Rep Loop is ");
					DEBUG_PBS_FULL_D(pbs_rep_time, 1);
					DEBUG_PBS_FULL_S(", for Retry No.");
					DEBUG_PBS_FULL_D(pbs_retry, 1);
					DEBUG_PBS_FULL_S("\n");

					/* Set all PBS values to MAX (31) */
					for (pup = 0; pup < cur_max_pup; pup++) {
						for (dq = 0; dq < DQ_NUM; dq++)
							ddr3_write_pup_reg(
								PUP_PBS_RX +
								pbs_dq_mapping[
								pup * (1 - ecc)
								+ ecc * ECC_PUP]
								[dq], CS0,
								pup + ecc * ECC_PUP,
								0, MAX_PBS);
					}

					/* Set all DQS PBS values to MIN (0) */
					for (pup = 0; pup < cur_max_pup; pup++) {
						ddr3_write_pup_reg(PUP_PBS_RX +
								   DQ_NUM, CS0,
								   pup +
								   ecc *
								   ECC_PUP, 0,
								   0);
					}

					/* Shift DQS, To first Fail */
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - Shift RX DQS to first fail\n");

					status = ddr3_rx_shift_dqs_to_first_fail
						(dram_info, cur_pup,
						 pattern_idx, ecc);
					if (MV_OK != status) {
						DEBUG_PBS_S("DDR3 - PBS Rx - ddr3_rx_shift_dqs_to_first_fail failed.\n");
						DEBUG_PBS_D(status, 8);
						DEBUG_PBS_S("\nDDR3 - PBS Rx - SKIP.\n");

						/* Reset read FIFO */
						reg = reg_read(REG_DRAM_TRAINING_ADDR);
						/* Start Auto Read Leveling procedure */
						reg |= (1 << REG_DRAM_TRAINING_RL_OFFS);
						/* 0x15B0 - Training Register */
						reg_write(REG_DRAM_TRAINING_ADDR, reg);

						reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
						reg |= ((1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS)
							+ (1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS));
						/* [0] = 1 - Enable SW override, [4] = 1 - FIFO reset  */
						/* 0x15B8 - Training SW 2 Register */
						reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

						do {
							reg = (reg_read(REG_DRAM_TRAINING_2_ADDR))
								& (1 <<	REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
						} while (reg);	/* Wait for '0' */

						reg = reg_read(REG_DRAM_TRAINING_ADDR);
						/* Clear Auto Read Leveling procedure */
						reg &= ~(1 << REG_DRAM_TRAINING_RL_OFFS);
						/* 0x15B0 - Training Register */
						reg_write(REG_DRAM_TRAINING_ADDR, reg);

						/* Set ADLL to 15 */
						for (pup = 0; pup < max_pup;
						     pup++) {
							ddr3_write_pup_reg
							    (PUP_DQS_RD, CS0,
							     pup +
							     (ecc * ECC_PUP), 0,
							     15);
						}

						/* Set all PBS values to MIN (0) */
						for (pup = 0; pup < cur_max_pup;
						     pup++) {
							for (dq = 0;
							     dq < DQ_NUM; dq++)
								ddr3_write_pup_reg
								    (PUP_PBS_RX +
								     pbs_dq_mapping
								     [pup * (1 - ecc) +
								      ecc * ECC_PUP]
								     [dq], CS0,
								     pup + ecc * ECC_PUP,
								     0, MIN_PBS);
						}

						return MV_OK;
					}

					/* PBS For each bit */
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - perform PBS for each bit\n");
					/* in this stage - start_over = 0; */
					if (MV_OK != ddr3_pbs_per_bit(
						    dram_info, &start_over,
						    0, &cur_pup,
						    pattern_idx, ecc)) {
						DEBUG_PBS_S("DDR3 - PBS Rx - ddr3_pbs_per_bit failed.");
						return MV_DDR3_TRAINING_ERR_PBS_RX_PER_BIT;
					}

				} while ((start_over == 1) &&
					 (++pbs_rep_time < COUNT_PBS_STARTOVER));

				if (pbs_rep_time == COUNT_PBS_STARTOVER &&
				    start_over == 1) {
					DEBUG_PBS_FULL_S("DDR3 - PBS Rx - FAIL - Algorithm failed doing RX PBS\n");
					return MV_DDR3_TRAINING_ERR_PBS_RX_MAX_VAL;
				}

				/* Return DQS ADLL to default value - 15 */
				/* Set all DQS PBS values to MIN (0) */
				for (pup = 0; pup < cur_max_pup; pup++)
					ddr3_write_pup_reg(PUP_DQS_RD, CS0,
							   pup + ecc * ECC_PUP,
							   0, INIT_RL_DELAY);

				DEBUG_PBS_FULL_C("DDR3 - PBS RX - values for iteration - ",
						 pbs_retry, 1);
				for (pup = 0; pup < cur_max_pup; pup++) {
					/*
					 * To minimize delay elements, inc from
					 * pbs value the min pbs val
					 */
					DEBUG_PBS_FULL_S("DDR3 - PBS - PUP");
					DEBUG_PBS_FULL_D((pup +
							  (ecc * ECC_PUP)), 1);
					DEBUG_PBS_FULL_S(": ");

					for (dq = 0; dq < DQ_NUM; dq++) {
						/* Set skew value for all dq */
						/*
						 * Bit# Deskew <- Bit# Deskew -
						 * last / first  failing bit
						 * Deskew For all bits (per PUP)
						 * (minimize delay elements)
						 */
						DEBUG_PBS_FULL_S("DQ");
						DEBUG_PBS_FULL_D(dq, 1);
						DEBUG_PBS_FULL_S("-");
						DEBUG_PBS_FULL_D(skew_array
								 [((pup) *
								   DQ_NUM) +
								  dq], 2);
						DEBUG_PBS_FULL_S(", ");
					}
					DEBUG_PBS_FULL_S("\n");
				}

				/*
				 * Collect the results we got on this trial
				 * of PBS
				 */
				for (pup = 0; pup < cur_max_pup; pup++) {
					for (dq = 0; dq < DQ_NUM; dq++) {
						skew_sum_array
							[pup + (ecc * (max_pup - 1))]
							[dq] +=
							skew_array[((pup) * DQ_NUM) + dq];
					}
				}

				/* ECC Support - Disable ECC MUX */
				reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
					~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
				reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
			}
		}

		/*
		 * Calculate the average skew for current pattern for each
		 * pup and each bit
		 */
		DEBUG_PBS_FULL_C("DDR3 - PBS RX - Average for pattern - ",
				 pattern_idx, 1);
		for (pup = 0; pup < max_pup; pup++) {
			/*
			 * FOR ECC only :: found min and max value for
			 * current pattern skew array
			 */
			/* Loop for all dqs */
			for (dq = 0; dq < DQ_NUM; dq++) {
				pattern_skew_array[pup][dq] +=
					(skew_sum_array[pup][dq] /
					 COUNT_PBS_REPEAT);
			}
		}

		DEBUG_PBS_C("DDR3 - PBS RX - values for current pattern - ",
			    pattern_idx, 1);
		for (pup = 0; pup < max_pup; pup++) {
			/*
			 * To minimize delay elements, inc from pbs value the
			 * min pbs val
			 */
			DEBUG_PBS_S("DDR3 - PBS RX - PUP");
			DEBUG_PBS_D(pup, 1);
			DEBUG_PBS_S(": ");

			for (dq = 0; dq < DQ_NUM; dq++) {
				/* Set skew value for all dq */
				/*
				 * Bit# Deskew <- Bit# Deskew - last / first
				 * failing bit Deskew For all bits (per PUP)
				 * (minimize delay elements)
				 */
				DEBUG_PBS_S("DQ");
				DEBUG_PBS_D(dq, 1);
				DEBUG_PBS_S("-");
				DEBUG_PBS_D(skew_sum_array[pup][dq] /
					    COUNT_PBS_REPEAT, 2);
				DEBUG_PBS_S(", ");
			}
			DEBUG_PBS_S("\n");
		}
	}

	/* Calculate the average skew */
	for (pup = 0; pup < max_pup; pup++) {
		for (dq = 0; dq < DQ_NUM; dq++)
			skew_array[((pup) * DQ_NUM) + dq] =
				pattern_skew_array[pup][dq] / COUNT_PBS_PATTERN;
	}

	DEBUG_PBS_S("DDR3 - PBS RX - Average for all patterns:\n");
	for (pup = 0; pup < max_pup; pup++) {
		/*
		 * To minimize delay elements, inc from pbs value the
		 * min pbs val
		 */
		DEBUG_PBS_S("DDR3 - PBS - PUP");
		DEBUG_PBS_D(pup, 1);
		DEBUG_PBS_S(": ");

		for (dq = 0; dq < DQ_NUM; dq++) {
			/* Set skew value for all dq */
			/*
			 * Bit# Deskew <- Bit# Deskew - last / first
			 * failing bit Deskew For all bits (per PUP)
			 * (minimize delay elements)
			 */
			DEBUG_PBS_S("DQ");
			DEBUG_PBS_D(dq, 1);
			DEBUG_PBS_S("-");
			DEBUG_PBS_D(skew_array[(pup * DQ_NUM) + dq], 2);
			DEBUG_PBS_S(", ");
		}
		DEBUG_PBS_S("\n");
	}

	/* Return ADLL to default value */
	ddr3_write_pup_reg(PUP_DQS_RD, CS0, PUP_BC, 0, INIT_RL_DELAY);

	/* Set averaged PBS results */
	ddr3_set_pbs_results(dram_info, 0);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_1_ADDR) |
		(1 << REG_DRAM_TRAINING_1_TRNBPOINT_OFFS);
	reg_write(REG_DRAM_TRAINING_1_ADDR, reg);

	DEBUG_PBS_FULL_S("DDR3 - PBS RX - ended successfuly\n");

	return MV_OK;
}

/*
 * Name:     ddr3_rx_shift_dqs_to_first_fail
 * Desc:     Execute the Rx shift DQ phase.
 * Args:     dram_info           ddr3 training information struct
 *           cur_pup             bit array of the function active pups.
 *           pbs_pattern_idx     Index of PBS pattern
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_rx_shift_dqs_to_first_fail(MV_DRAM_INFO *dram_info, u32 cur_pup,
					   u32 pbs_pattern_idx, u32 ecc)
{
	u32 unlock_pup;		/* bit array of unlock pups  */
	u32 new_lockup_pup;	/* bit array of compare failed pups */
	u32 adll_val = MAX_DELAY;
	u32 dqs_deskew_val = 0;	/* current value of DQS PBS deskew */
	u32 cur_max_pup, pup, pass_pup;
	u32 *pattern_ptr;

	/* Choose pattern */
	switch (dram_info->ddr_width) {
#if defined(MV88F672X)
	case 16:
		pattern_ptr = (u32 *)&pbs_pattern[pbs_pattern_idx];
		break;
#endif
	case 32:
		pattern_ptr = (u32 *)&pbs_pattern_32b[pbs_pattern_idx];
		break;
#if defined(MV88F78X60)
	case 64:
		pattern_ptr = (u32 *)&pbs_pattern_64b[pbs_pattern_idx];
		break;
#endif
	default:
		return MV_FAIL;
	}

	/* Set current pup number */
	if (cur_pup == 0x1)	/* Ecc mode */
		cur_max_pup = 1;
	else
		cur_max_pup = dram_info->num_of_std_pups;

	unlock_pup = cur_pup;	/* '1' for each unlocked pup */

	DEBUG_PBS_FULL_S("DDR3 - PBS RX - Shift DQS - Starting...\n");

	/* Set DQS ADLL to MAX */
	DEBUG_PBS_FULL_S("DDR3 - PBS RX - Shift DQS - Set DQS ADLL to Max for all PUPs\n");
	for (pup = 0; pup < cur_max_pup; pup++)
		ddr3_write_pup_reg(PUP_DQS_RD, CS0, pup + ecc * ECC_PUP, 0,
				   MAX_DELAY);

	/* Loop on all ADLL Vaules */
	do {
		/* Loop until found fail for all pups */
		new_lockup_pup = 0;
		if (MV_OK != ddr3_sdram_compare(dram_info, unlock_pup,
						&new_lockup_pup,
						pattern_ptr, LEN_PBS_PATTERN,
						SDRAM_PBS_I_OFFS +
						pbs_pattern_idx * SDRAM_PBS_NEXT_OFFS,
						0, 0, NULL, 0)) {
			DEBUG_PBS_S("DDR3 - PBS Rx - Shift DQS - MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP(ddr3_sdram_compare)\n");
			return MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP;
		}

		if ((new_lockup_pup != 0) && (dqs_deskew_val <= 1)) {
			/* Fail on start with first deskew value */
			/* Decrement DQS ADLL */
			--adll_val;
			if (adll_val == ADLL_MIN) {
				DEBUG_PBS_S("DDR3 - PBS Rx - Shift DQS - fail on start with first deskew value\n");
				return MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP;
			}
			ddr3_write_pup_reg(PUP_DQS_RD, CS0, pup + ecc * ECC_PUP,
					   0, adll_val);
			continue;
		}

		/* Update all new locked pups */
		unlock_pup &= ~new_lockup_pup;

		if ((unlock_pup == 0) || (dqs_deskew_val == MAX_PBS)) {
			if (dqs_deskew_val == MAX_PBS) {
				/*
				 * Reach max value of dqs deskew or get fail
				 * for all pups
				 */
				DEBUG_PBS_FULL_S("DDR3 - PBS RX - Shift DQS - DQS deskew reached maximum value\n");
			}
			break;
		}

		DEBUG_PBS_FULL_S("DDR3 - PBS RX - Shift DQS - Inc DQS deskew for PUPs: ");
		DEBUG_PBS_FULL_D(unlock_pup, 2);
		DEBUG_PBS_FULL_C(", deskew = ", dqs_deskew_val, 2);

		/* Increment DQS deskew elements - Only for unlocked pups */
		dqs_deskew_val++;
		for (pup = 0; pup < cur_max_pup; pup++) {
			if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
				ddr3_write_pup_reg(PUP_PBS_RX + DQS_DQ_NUM, CS0,
						   pup + ecc * ECC_PUP, 0,
						   dqs_deskew_val);
			}
		}
	} while (1);

	DEBUG_PBS_FULL_S("DDR3 - PBS RX - Shift DQS - ADLL shift one step before fail\n");
	/* Continue to ADLL shift one step before fail */
	unlock_pup = cur_pup;
	do {
		/* Loop until pass compare for all pups */
		new_lockup_pup = 0;
		/* Read and compare results  */
		if (MV_OK != ddr3_sdram_compare(dram_info, unlock_pup, &new_lockup_pup,
						pattern_ptr, LEN_PBS_PATTERN,
						SDRAM_PBS_I_OFFS +
						pbs_pattern_idx * SDRAM_PBS_NEXT_OFFS,
						1, 0, NULL, 0)) {
			DEBUG_PBS_S("DDR3 - PBS Rx - Shift DQS - MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP(ddr3_sdram_compare)\n");
			return MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_SRAM_CMP;
		}

		/*
		 * Get mask for pup which passed so their adll will be
		 * changed to 2 steps before fails
		 */
		pass_pup = unlock_pup & ~new_lockup_pup;

		DEBUG_PBS_FULL_S("Shift DQS by 2 steps for PUPs: ");
		DEBUG_PBS_FULL_D(pass_pup, 2);
		DEBUG_PBS_FULL_C(", Set ADLL value = ", (adll_val - 2), 2);

		/* Only for pass pups   */
		for (pup = 0; pup < cur_max_pup; pup++) {
			if (IS_PUP_ACTIVE(pass_pup, pup) == 1) {
				ddr3_write_pup_reg(PUP_DQS_RD, CS0,
						   pup + ecc * ECC_PUP, 0,
						   (adll_val - 2));
			}
		}

		/* Locked pups that compare success  */
		unlock_pup &= new_lockup_pup;

		if (unlock_pup == 0) {
			/* All pups locked */
			break;
		}

		/* Found error */
		if (adll_val == 0) {
			DEBUG_PBS_FULL_S("DDR3 - PBS Rx - Shift DQS - Adll reach min value\n");
			return MV_DDR3_TRAINING_ERR_PBS_SHIFT_QDS_MAX_VAL;
		}

		/*
		 * Decrement (Move Back to Left one phase - ADLL) dqs RX delay
		 */
		adll_val--;
		for (pup = 0; pup < cur_max_pup; pup++) {
			if (IS_PUP_ACTIVE(unlock_pup, pup) == 1) {
				ddr3_write_pup_reg(PUP_DQS_RD, CS0,
						   pup + ecc * ECC_PUP, 0,
						   adll_val);
			}
		}
	} while (1);

	return MV_OK;
}

/*
 * lock_pups() extracted from ddr3_pbs_per_bit(). This just got too
 * much indented making it hard to read / edit.
 */
static void lock_pups(u32 pup, u32 *pup_locked, u8 *unlock_pup_dq_array,
		      u32 pbs_curr_val, u32 start_pbs, u32 ecc, int is_tx)
{
	u32 dq;
	int idx;

	/* Lock PBS value for all remaining PUPs bits */
	DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - Lock PBS value for all remaining PUPs bits, pup ");
	DEBUG_PBS_FULL_D(pup, 1);
	DEBUG_PBS_FULL_C(" pbs value ", pbs_curr_val, 2);

	idx = pup * (1 - ecc) + ecc * ECC_PUP;
	*pup_locked &= ~(1 << pup);

	for (dq = 0; dq < DQ_NUM; dq++) {
		if (IS_PUP_ACTIVE(unlock_pup_dq_array[dq], pup) == 1) {
			int offs;

			/* Lock current dq */
			unlock_pup_dq_array[dq] &= ~(1 << pup);
			skew_array[(pup * DQ_NUM) + dq] = pbs_curr_val;

			if (is_tx == 1)
				offs = PUP_PBS_TX;
			else
				offs = PUP_PBS_RX;

			ddr3_write_pup_reg(offs +
					   pbs_dq_mapping[idx][dq], CS0,
					   idx, 0, start_pbs);
		}
	}
}

/*
 * Name:     ddr3_pbs_per_bit
 * Desc:     Execute the Per Bit Skew phase.
 * Args:     start_over      Return whether need to start over the algorithm
 *           is_tx           Indicate whether Rx or Tx
 *           pcur_pup        bit array of the function active pups. return the
 *                           pups that need to repeat on the PBS
 *           pbs_pattern_idx Index of PBS pattern
 *
 * Notes:    Current implementation supports double activation of this function.
 *           i.e. in order to activate this function (using start_over) more than
 *           twice, the implementation should change.
 *           imlementation limitation are marked using
 *           ' CHIP-ONLY! - Implementation Limitation '
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_pbs_per_bit(MV_DRAM_INFO *dram_info, int *start_over, int is_tx,
			    u32 *pcur_pup, u32 pbs_pattern_idx, u32 ecc)
{
	/*
	 * Bit array to indicate if we already get fail on bit per pup & dq bit
	 */
	u8 unlock_pup_dq_array[DQ_NUM] = {
		*pcur_pup, *pcur_pup, *pcur_pup, *pcur_pup, *pcur_pup,
		*pcur_pup, *pcur_pup, *pcur_pup
	};

	u8 cmp_unlock_pup_dq_array[COUNT_PBS_COMP_RETRY_NUM][DQ_NUM];
	u32 pup, dq;
	/* value of pbs is according to RX or TX */
	u32 start_pbs, last_pbs;
	u32 pbs_curr_val;
	/* bit array that indicates all dq of the pup locked */
	u32 pup_locked;
	u32 first_fail[MAX_PUP_NUM] = { 0 };	/* count first fail per pup */
	/* indicates whether we get first fail per pup */
	int first_failed[MAX_PUP_NUM] = { 0 };
	/* bit array that indicates pup already get fail */
	u32 sum_pup_fail;
	/* use to calculate diff between curr pbs to first fail pbs */
	u32 calc_pbs_diff;
	u32 pbs_cmp_retry;
	u32 max_pup;

	/* Set init values for retry array - 8 retry */
	for (pbs_cmp_retry = 0; pbs_cmp_retry < COUNT_PBS_COMP_RETRY_NUM;
	     pbs_cmp_retry++) {
		for (dq = 0; dq < DQ_NUM; dq++)
			cmp_unlock_pup_dq_array[pbs_cmp_retry][dq] = *pcur_pup;
	}

	memset(&skew_array, 0, MAX_PUP_NUM * DQ_NUM * sizeof(u32));

	DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - Started\n");

	/* The pbs value depends if rx or tx */
	if (is_tx == 1) {
		start_pbs = MIN_PBS;
		last_pbs = MAX_PBS;
	} else {
		start_pbs = MAX_PBS;
		last_pbs = MIN_PBS;
	}

	pbs_curr_val = start_pbs;
	pup_locked = *pcur_pup;

	/* Set current pup number */
	if (pup_locked == 0x1)	/* Ecc mode */
		max_pup = 1;
	else
		max_pup = dram_info->num_of_std_pups;

	do {
		/* Increment/ decrement PBS for un-lock bits only */
		if (is_tx == 1)
			pbs_curr_val++;
		else
			pbs_curr_val--;

		/* Set Current PBS delay  */
		for (dq = 0; dq < DQ_NUM; dq++) {
			/* Check DQ bits to see if locked in all pups */
			if (unlock_pup_dq_array[dq] == 0) {
				DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - All pups are locked for DQ ");
				DEBUG_PBS_FULL_D(dq, 1);
				DEBUG_PBS_FULL_S("\n");
				continue;
			}

			for (pup = 0; pup < max_pup; pup++) {
				int idx;

				idx = pup * (1 - ecc) + ecc * ECC_PUP;

				if (IS_PUP_ACTIVE(unlock_pup_dq_array[dq], pup)
				    == 0)
					continue;

				if (is_tx == 1)
					ddr3_write_pup_reg(
						PUP_PBS_TX + pbs_dq_mapping[idx][dq],
						CS0, idx, 0, pbs_curr_val);
				else
					ddr3_write_pup_reg(
						PUP_PBS_RX + pbs_dq_mapping[idx][dq],
						CS0, idx, 0, pbs_curr_val);
			}
		}

		/*
		 * Write Read and compare results - run the test
		 * DDR_PBS_COMP_RETRY_NUM times
		 */
		/* Run number of read and write to verify */
		for (pbs_cmp_retry = 0;
		     pbs_cmp_retry < COUNT_PBS_COMP_RETRY_NUM;
		     pbs_cmp_retry++) {

			if (MV_OK !=
			    ddr3_sdram_pbs_compare(dram_info, pup_locked, is_tx,
						   pbs_pattern_idx,
						   pbs_curr_val, start_pbs,
						   skew_array,
						   cmp_unlock_pup_dq_array
						   [pbs_cmp_retry], ecc))
				return MV_FAIL;

			for (pup = 0; pup < max_pup; pup++) {
				for (dq = 0; dq < DQ_NUM; dq++) {
					if ((IS_PUP_ACTIVE(unlock_pup_dq_array[dq],
							   pup) == 1)
					    && (IS_PUP_ACTIVE(cmp_unlock_pup_dq_array
					      [pbs_cmp_retry][dq],
					      pup) == 0)) {
						DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - PbsCurrVal: ");
						DEBUG_PBS_FULL_D(pbs_curr_val, 2);
						DEBUG_PBS_FULL_S(" PUP: ");
						DEBUG_PBS_FULL_D(pup, 1);
						DEBUG_PBS_FULL_S(" DQ: ");
						DEBUG_PBS_FULL_D(dq, 1);
						DEBUG_PBS_FULL_S(" - failed\n");
					}
				}
			}

			for (dq = 0; dq < DQ_NUM; dq++) {
				unlock_pup_dq_array[dq] &=
				    cmp_unlock_pup_dq_array[pbs_cmp_retry][dq];
			}
		}

		pup_locked = 0;
		sum_pup_fail = *pcur_pup;

		/* Check which DQ is failed */
		for (dq = 0; dq < DQ_NUM; dq++) {
			/* Summarize the locked pup */
			pup_locked |= unlock_pup_dq_array[dq];

			/* Check if get fail */
			sum_pup_fail &= unlock_pup_dq_array[dq];
		}

		/* If all PUPS are locked in all DQ - Break */
		if (pup_locked == 0) {
			/* All pups are locked */
			*start_over = 0;
			DEBUG_PBS_FULL_S("DDR3 - PBS Per bit -  All bit in all pups are successfully locked\n");
			break;
		}

		/* PBS deskew elements reach max ? */
		if (pbs_curr_val == last_pbs) {
			DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - PBS deskew elements reach max\n");
			/* CHIP-ONLY! - Implementation Limitation */
			*start_over = (sum_pup_fail != 0) && (!(*start_over));
			*pcur_pup = pup_locked;

			DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - StartOver: ");
			DEBUG_PBS_FULL_D(*start_over, 1);
			DEBUG_PBS_FULL_S("  pup_locked: ");
			DEBUG_PBS_FULL_D(pup_locked, 2);
			DEBUG_PBS_FULL_S("  sum_pup_fail: ");
			DEBUG_PBS_FULL_D(sum_pup_fail, 2);
			DEBUG_PBS_FULL_S("\n");

			/* Lock PBS value for all remaining  bits */
			for (pup = 0; pup < max_pup; pup++) {
				/* Check if current pup already received error */
				if (IS_PUP_ACTIVE(pup_locked, pup) == 1) {
					/* Valid pup for current function */
					if (IS_PUP_ACTIVE(sum_pup_fail, pup) ==
					    1 && (*start_over == 1)) {
						DEBUG_PBS_FULL_C("DDR3 - PBS Per bit - skipping lock of pup (first loop of pbs)",
								 pup, 1);
						continue;
					} else
					    if (IS_PUP_ACTIVE(sum_pup_fail, pup)
						== 1) {
						DEBUG_PBS_FULL_C("DDR3 - PBS Per bit - Locking pup %d (even though it wasn't supposed to be locked)",
								 pup, 1);
					}

					/* Already got fail on the PUP */
					/* Lock PBS value for all remaining bits */
					DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - Locking remaning DQs for pup - ");
					DEBUG_PBS_FULL_D(pup, 1);
					DEBUG_PBS_FULL_S(": ");

					for (dq = 0; dq < DQ_NUM; dq++) {
						if (IS_PUP_ACTIVE
						    (unlock_pup_dq_array[dq],
						     pup) == 1) {
							DEBUG_PBS_FULL_D(dq, 1);
							DEBUG_PBS_FULL_S(",");
							/* set current PBS */
							skew_array[((pup) *
								    DQ_NUM) +
								   dq] =
							    pbs_curr_val;
						}
					}

					if (*start_over == 1) {
						/*
						 * Reset this pup bit - when
						 * restart the PBS, ignore this
						 * pup
						 */
						*pcur_pup &= ~(1 << pup);
					}
					DEBUG_PBS_FULL_S("\n");
				} else {
					DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - Pup ");
					DEBUG_PBS_FULL_D(pup, 1);
					DEBUG_PBS_FULL_C(" is not set in puplocked - ",
							 pup_locked, 1);
				}
			}

			/* Need to start the PBS again */
			if (*start_over == 1) {
				DEBUG_PBS_FULL_S("DDR3 - PBS Per bit - false fail - returning to start\n");
				return MV_OK;
			}
			break;
		}

		/* Diff Check */
		for (pup = 0; pup < max_pup; pup++) {
			if (IS_PUP_ACTIVE(pup_locked, pup) == 1) {
				/* pup is not locked */
				if (first_failed[pup] == 0) {
					/* No first fail until now */
					if (IS_PUP_ACTIVE(sum_pup_fail, pup) ==
					    0) {
						/* Get first fail */
						DEBUG_PBS_FULL_C("DDR3 - PBS Per bit - First fail in pup ",
								 pup, 1);
						first_failed[pup] = 1;
						first_fail[pup] = pbs_curr_val;
					}
				} else {
					/* Already got first fail */
					if (is_tx == 1) {
						/* TX - inc pbs */
						calc_pbs_diff =	pbs_curr_val -
							first_fail[pup];
					} else {
						/* RX - dec pbs */
						calc_pbs_diff = first_fail[pup] -
							pbs_curr_val;
					}

					if (calc_pbs_diff >= PBS_DIFF_LIMIT) {
						lock_pups(pup, &pup_locked,
							  unlock_pup_dq_array,
							  pbs_curr_val,
							  start_pbs, ecc, is_tx);
					}
				}
			}
		}
	} while (1);

	return MV_OK;
}

/*
 * Name:         ddr3_set_pbs_results
 * Desc:         Set to HW the PBS phase results.
 * Args:         is_tx       Indicates whether to set Tx or RX results
 * Notes:
 * Returns:      MV_OK if success, other error code if fail.
 */
static int ddr3_set_pbs_results(MV_DRAM_INFO *dram_info, int is_tx)
{
	u32 pup, phys_pup, dq;
	u32 max_pup;		/* number of valid pups */
	u32 pbs_min;		/* minimal pbs val per pup */
	u32 pbs_max;		/* maximum pbs val per pup */
	u32 val[9];

	max_pup = dram_info->num_of_total_pups;
	DEBUG_PBS_FULL_S("DDR3 - PBS - ddr3_set_pbs_results:\n");

	/* Loop for all dqs & pups */
	for (pup = 0; pup < max_pup; pup++) {
		if (pup == (max_pup - 1) && dram_info->ecc_ena)
			phys_pup = ECC_PUP;
		else
			phys_pup = pup;

		/*
		 * To minimize delay elements, inc from pbs value the min
		 * pbs val
		 */
		pbs_min = MAX_PBS;
		pbs_max = 0;
		for (dq = 0; dq < DQ_NUM; dq++) {
			if (pbs_min > skew_array[(pup * DQ_NUM) + dq])
				pbs_min = skew_array[(pup * DQ_NUM) + dq];

			if (pbs_max < skew_array[(pup * DQ_NUM) + dq])
				pbs_max = skew_array[(pup * DQ_NUM) + dq];
		}

		pbs_max -= pbs_min;

		DEBUG_PBS_FULL_S("DDR3 - PBS - PUP");
		DEBUG_PBS_FULL_D(phys_pup, 1);
		DEBUG_PBS_FULL_S(": Min Val = ");
		DEBUG_PBS_FULL_D(pbs_min, 2);
		DEBUG_PBS_FULL_C(", Max Val = ", pbs_max, 2);

		val[pup] = 0;

		for (dq = 0; dq < DQ_NUM; dq++) {
			int idx;
			int offs;

			/* Set skew value for all dq */
			/*
			 * Bit# Deskew <- Bit# Deskew - last / first
			 * failing bit Deskew For all bits (per PUP)
			 * (minimize delay elements)
			 */

			DEBUG_PBS_FULL_S("DQ");
			DEBUG_PBS_FULL_D(dq, 1);
			DEBUG_PBS_FULL_S("-");
			DEBUG_PBS_FULL_D((skew_array[(pup * DQ_NUM) + dq] -
					  pbs_min), 2);
			DEBUG_PBS_FULL_S(", ");

			idx = (pup * DQ_NUM) + dq;

			if (is_tx == 1)
				offs = PUP_PBS_TX;
			else
				offs = PUP_PBS_RX;

			ddr3_write_pup_reg(offs + pbs_dq_mapping[phys_pup][dq],
					   CS0, phys_pup, 0,
					   skew_array[idx] - pbs_min);

			if (is_tx == 1)
				val[pup] += skew_array[idx] - pbs_min;
		}

		DEBUG_PBS_FULL_S("\n");

		/* Set the DQS the half of the Max PBS of the DQs  */
		if (is_tx == 1) {
			ddr3_write_pup_reg(PUP_PBS_TX + 8, CS0, phys_pup, 0,
					   pbs_max / 2);
			ddr3_write_pup_reg(PUP_PBS_TX + 0xa, CS0, phys_pup, 0,
					   val[pup] / 8);
		} else
			ddr3_write_pup_reg(PUP_PBS_RX + 8, CS0, phys_pup, 0,
					   pbs_max / 2);
	}

	return MV_OK;
}

static void ddr3_pbs_write_pup_dqs_reg(u32 cs, u32 pup, u32 dqs_delay)
{
	u32 reg, delay;

	reg = (ddr3_read_pup_reg(PUP_WL_MODE, cs, pup) & 0x3FF);
	delay = reg & PUP_DELAY_MASK;
	reg |= ((dqs_delay + delay) << REG_PHY_DQS_REF_DLY_OFFS);
	reg |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
	reg |= (pup << REG_PHY_PUP_OFFS);
	reg |= ((0x4 * cs + PUP_WL_MODE) << REG_PHY_CS_OFFS);

	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */
	do {
		reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
			REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
	} while (reg);	/* Wait for '0' to mark the end of the transaction */

	udelay(10);
}

/*
 * Set training patterns
 */
int ddr3_load_pbs_patterns(MV_DRAM_INFO *dram_info)
{
	u32 cs, cs_count, cs_tmp;
	u32 sdram_addr;
	u32 *pattern_ptr0, *pattern_ptr1;

	/* Choose pattern */
	switch (dram_info->ddr_width) {
#if defined(MV88F672X)
	case 16:
		pattern_ptr0 = (u32 *)&pbs_pattern[0];
		pattern_ptr1 = (u32 *)&pbs_pattern[1];
		break;
#endif
	case 32:
		pattern_ptr0 = (u32 *)&pbs_pattern_32b[0];
		pattern_ptr1 = (u32 *)&pbs_pattern_32b[1];
		break;
#if defined(MV88F78X60)
	case 64:
		pattern_ptr0 = (u32 *)&pbs_pattern_64b[0];
		pattern_ptr1 = (u32 *)&pbs_pattern_64b[1];
		break;
#endif
	default:
		return MV_FAIL;
	}

	/* Loop for each CS */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			cs_count = 0;
			for (cs_tmp = 0; cs_tmp < cs; cs_tmp++) {
				if (dram_info->cs_ena & (1 << cs_tmp))
					cs_count++;
			}

			/* Init PBS I pattern */
			sdram_addr = (cs_count * (SDRAM_CS_SIZE + 1) +
				      SDRAM_PBS_I_OFFS);
			if (MV_OK !=
			    ddr3_sdram_compare(dram_info, (u32) NULL, NULL,
					       pattern_ptr0, LEN_STD_PATTERN,
					       sdram_addr, 1, 0, NULL,
					       0))
				return MV_FAIL;

			/* Init PBS II pattern */
			sdram_addr = (cs_count * (SDRAM_CS_SIZE + 1) +
				      SDRAM_PBS_II_OFFS);
			if (MV_OK !=
			    ddr3_sdram_compare(dram_info, (u32) NULL, NULL,
					       pattern_ptr1, LEN_STD_PATTERN,
					       sdram_addr, 1, 0, NULL,
					       0))
				return MV_FAIL;
		}
	}

	return MV_OK;
}
#endif
