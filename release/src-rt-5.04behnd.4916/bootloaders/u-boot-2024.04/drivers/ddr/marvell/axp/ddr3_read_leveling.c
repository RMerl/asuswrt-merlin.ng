// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_hw_training.h"

/*
 * Debug
 */
#define DEBUG_RL_C(s, d, l) \
	DEBUG_RL_S(s); DEBUG_RL_D(d, l); DEBUG_RL_S("\n")
#define DEBUG_RL_FULL_C(s, d, l) \
	DEBUG_RL_FULL_S(s); DEBUG_RL_FULL_D(d, l); DEBUG_RL_FULL_S("\n")

#ifdef MV_DEBUG_RL
#define DEBUG_RL_S(s) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%s", s)
#define DEBUG_RL_D(d, l) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%x", d)
#else
#define DEBUG_RL_S(s)
#define DEBUG_RL_D(d, l)
#endif

#ifdef MV_DEBUG_RL_FULL
#define DEBUG_RL_FULL_S(s)		puts(s)
#define DEBUG_RL_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_RL_FULL_S(s)
#define DEBUG_RL_FULL_D(d, l)
#endif

extern u32 rl_pattern[LEN_STD_PATTERN];

#ifdef RL_MODE
static int ddr3_read_leveling_single_cs_rl_mode(u32 cs, u32 freq,
						int ratio_2to1, u32 ecc,
						MV_DRAM_INFO *dram_info);
#else
static int ddr3_read_leveling_single_cs_window_mode(u32 cs, u32 freq,
						    int ratio_2to1, u32 ecc,
						    MV_DRAM_INFO *dram_info);
#endif

/*
 * Name:     ddr3_read_leveling_hw
 * Desc:     Execute the Read leveling phase by HW
 * Args:     dram_info - main struct
 *           freq      - current sequence frequency
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_read_leveling_hw(u32 freq, MV_DRAM_INFO *dram_info)
{
	u32 reg;

	/* Debug message - Start Read leveling procedure */
	DEBUG_RL_S("DDR3 - Read Leveling - Starting HW RL procedure\n");

	/* Start Auto Read Leveling procedure */
	reg = 1 << REG_DRAM_TRAINING_RL_OFFS;
	/* Config the retest number */
	reg |= (COUNT_HW_RL << REG_DRAM_TRAINING_RETEST_OFFS);

	/* Enable CS in the automatic process */
	reg |= (dram_info->cs_ena << REG_DRAM_TRAINING_CS_OFFS);

	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	reg = reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) |
		(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_SHADOW_ADDR, reg);

	/* Wait */
	do {
		reg = reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) &
			(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	} while (reg);		/* Wait for '0' */

	/* Check if Successful */
	if (reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) &
	    (1 << REG_DRAM_TRAINING_ERROR_OFFS)) {
		u32 delay, phase, pup, cs;

		dram_info->rl_max_phase = 0;
		dram_info->rl_min_phase = 10;

		/* Read results to arrays */
		for (cs = 0; cs < MAX_CS; cs++) {
			if (dram_info->cs_ena & (1 << cs)) {
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					if (pup == dram_info->num_of_std_pups
					    && dram_info->ecc_ena)
						pup = ECC_PUP;
					reg =
					    ddr3_read_pup_reg(PUP_RL_MODE, cs,
							      pup);
					phase = (reg >> REG_PHY_PHASE_OFFS) &
						PUP_PHASE_MASK;
					delay = reg & PUP_DELAY_MASK;
					dram_info->rl_val[cs][pup][P] = phase;
					if (phase > dram_info->rl_max_phase)
						dram_info->rl_max_phase = phase;
					if (phase < dram_info->rl_min_phase)
						dram_info->rl_min_phase = phase;
					dram_info->rl_val[cs][pup][D] = delay;
					dram_info->rl_val[cs][pup][S] =
					    RL_FINAL_STATE;
					reg =
					    ddr3_read_pup_reg(PUP_RL_MODE + 0x1,
							      cs, pup);
					dram_info->rl_val[cs][pup][DQS] =
					    (reg & 0x3F);
				}
#ifdef MV_DEBUG_RL
				/* Print results */
				DEBUG_RL_C("DDR3 - Read Leveling - Results for CS - ",
					   (u32) cs, 1);

				for (pup = 0;
				     pup < (dram_info->num_of_total_pups);
				     pup++) {
					if (pup == dram_info->num_of_std_pups
					    && dram_info->ecc_ena)
						pup = ECC_PUP;
					DEBUG_RL_S("DDR3 - Read Leveling - PUP: ");
					DEBUG_RL_D((u32) pup, 1);
					DEBUG_RL_S(", Phase: ");
					DEBUG_RL_D((u32) dram_info->
						   rl_val[cs][pup][P], 1);
					DEBUG_RL_S(", Delay: ");
					DEBUG_RL_D((u32) dram_info->
						   rl_val[cs][pup][D], 2);
					DEBUG_RL_S("\n");
				}
#endif
			}
		}

		dram_info->rd_rdy_dly =
			reg_read(REG_READ_DATA_READY_DELAYS_ADDR) &
			REG_READ_DATA_SAMPLE_DELAYS_MASK;
		dram_info->rd_smpl_dly =
			reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR) &
			REG_READ_DATA_READY_DELAYS_MASK;
#ifdef MV_DEBUG_RL
		DEBUG_RL_C("DDR3 - Read Leveling - Read Sample Delay: ",
			   dram_info->rd_smpl_dly, 2);
		DEBUG_RL_C("DDR3 - Read Leveling - Read Ready Delay: ",
			   dram_info->rd_rdy_dly, 2);
		DEBUG_RL_S("DDR3 - Read Leveling - HW RL Ended Successfully\n");
#endif
		return MV_OK;

	} else {
		DEBUG_RL_S("DDR3 - Read Leveling - HW RL Error\n");
		return MV_FAIL;
	}
}

/*
 * Name:     ddr3_read_leveling_sw
 * Desc:     Execute the Read leveling phase by SW
 * Args:     dram_info - main struct
 *           freq      - current sequence frequency
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_read_leveling_sw(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info)
{
	u32 reg, cs, ecc, pup_num, phase, delay, pup;
	int status;

	/* Debug message - Start Read leveling procedure */
	DEBUG_RL_S("DDR3 - Read Leveling - Starting SW RL procedure\n");

	/* Enable SW Read Leveling */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	reg &= ~(1 << REG_DRAM_TRAINING_2_RL_MODE_OFFS);
	/* [0]=1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

#ifdef RL_MODE
	reg = (dram_info->cs_ena << REG_DRAM_TRAINING_CS_OFFS) |
		(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */
#endif

	/* Loop for each CS */
	for (cs = 0; cs < dram_info->num_cs; cs++) {
		DEBUG_RL_C("DDR3 - Read Leveling - CS - ", (u32) cs, 1);

		for (ecc = 0; ecc <= (dram_info->ecc_ena); ecc++) {
			/* ECC Support - Switch ECC Mux on ecc=1 */
			reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
				~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
			reg |= (dram_info->ecc_ena *
				ecc << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
			reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

			if (ecc)
				DEBUG_RL_S("DDR3 - Read Leveling - ECC Mux Enabled\n");
			else
				DEBUG_RL_S("DDR3 - Read Leveling - ECC Mux Disabled\n");

			/* Set current sample delays */
			reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_SAMPLE_DELAYS_MASK <<
				 (REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			reg |= (dram_info->cl <<
				(REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR, reg);

			/* Set current Ready delay */
			reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
				 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
			if (!ratio_2to1) {
				/* 1:1 mode */
				reg |= ((dram_info->cl + 1) <<
					(REG_READ_DATA_READY_DELAYS_OFFS * cs));
			} else {
				/* 2:1 mode */
				reg |= ((dram_info->cl + 2) <<
					(REG_READ_DATA_READY_DELAYS_OFFS * cs));
			}
			reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);

			/* Read leveling Single CS[cs] */
#ifdef RL_MODE
			status =
			    ddr3_read_leveling_single_cs_rl_mode(cs, freq,
								 ratio_2to1,
								 ecc,
								 dram_info);
			if (MV_OK != status)
				return status;
#else
			status =
			    ddr3_read_leveling_single_cs_window_mode(cs, freq,
								     ratio_2to1,
								     ecc,
								     dram_info)
			    if (MV_OK != status)
				return status;
#endif
		}

		/* Print results */
		DEBUG_RL_C("DDR3 - Read Leveling - Results for CS - ", (u32) cs,
			   1);

		for (pup = 0;
		     pup < (dram_info->num_of_std_pups + dram_info->ecc_ena);
		     pup++) {
			DEBUG_RL_S("DDR3 - Read Leveling - PUP: ");
			DEBUG_RL_D((u32) pup, 1);
			DEBUG_RL_S(", Phase: ");
			DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][P], 1);
			DEBUG_RL_S(", Delay: ");
			DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][D], 2);
			DEBUG_RL_S("\n");
		}

		DEBUG_RL_C("DDR3 - Read Leveling - Read Sample Delay: ",
			   dram_info->rd_smpl_dly, 2);
		DEBUG_RL_C("DDR3 - Read Leveling - Read Ready Delay: ",
			   dram_info->rd_rdy_dly, 2);

		/* Configure PHY with average of 3 locked leveling settings */
		for (pup = 0;
		     pup < (dram_info->num_of_std_pups + dram_info->ecc_ena);
		     pup++) {
			/* ECC support - bit 8 */
			pup_num = (pup == dram_info->num_of_std_pups) ? ECC_BIT : pup;

			/* For now, set last cnt result */
			phase = dram_info->rl_val[cs][pup][P];
			delay = dram_info->rl_val[cs][pup][D];
			ddr3_write_pup_reg(PUP_RL_MODE, cs, pup_num, phase,
					   delay);
		}
	}

	/* Reset PHY read FIFO */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	do {
		reg = (reg_read(REG_DRAM_TRAINING_2_ADDR)) &
			(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
	} while (reg);		/* Wait for '0' */

	/* ECC Support - Switch ECC Mux off ecc=0 */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

#ifdef RL_MODE
	reg_write(REG_DRAM_TRAINING_ADDR, 0);	/* 0x15B0 - Training Register */
#endif

	/* Disable SW Read Leveling */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* [0] = 0 - Disable SW override  */
	reg = (reg | (0x1 << REG_DRAM_TRAINING_2_RL_MODE_OFFS));
	/* [3] = 1 - Disable RL MODE */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	DEBUG_RL_S("DDR3 - Read Leveling - Finished RL procedure for all CS\n");
	return MV_OK;
}

#ifdef RL_MODE
/*
 * overrun() extracted from ddr3_read_leveling_single_cs_rl_mode().
 * This just got too much indented making it hard to read / edit.
 */
static void overrun(u32 cs, MV_DRAM_INFO *info, u32 pup, u32 locked_pups,
		    u32 *locked_sum, u32 ecc, int *first_octet_locked,
		    int *counter_in_progress, int final_delay, u32 delay,
		    u32 phase)
{
	/* If no OverRun */
	if (((~locked_pups >> pup) & 0x1) && (final_delay == 0)) {
		int idx;

		idx = pup + ecc * ECC_BIT;

		/* PUP passed, start examining */
		if (info->rl_val[cs][idx][S] == RL_UNLOCK_STATE) {
			/* Must be RL_UNLOCK_STATE */
			/* Match expected value ? - Update State Machine */
			if (info->rl_val[cs][idx][C] < RL_RETRY_COUNT) {
				DEBUG_RL_FULL_C("DDR3 - Read Leveling - We have no overrun and a match on pup: ",
						(u32)pup, 1);
				info->rl_val[cs][idx][C]++;

				/* If pup got to last state - lock the delays */
				if (info->rl_val[cs][idx][C] == RL_RETRY_COUNT) {
					info->rl_val[cs][idx][C] = 0;
					info->rl_val[cs][idx][DS] = delay;
					info->rl_val[cs][idx][PS] = phase;

					/* Go to Final State */
					info->rl_val[cs][idx][S] = RL_FINAL_STATE;
					*locked_sum = *locked_sum + 1;
					DEBUG_RL_FULL_C("DDR3 - Read Leveling - We have locked pup: ",
							(u32)pup, 1);

					/*
					 * If first lock - need to lock delays
					 */
					if (*first_octet_locked == 0) {
						DEBUG_RL_FULL_C("DDR3 - Read Leveling - We got first lock on pup: ",
								(u32)pup, 1);
						*first_octet_locked = 1;
					}

					/*
					 * If pup is in not in final state but
					 * there was match - dont increment
					 * counter
					 */
				} else {
					*counter_in_progress = 1;
				}
			}
		}
	}
}

/*
 * Name:     ddr3_read_leveling_single_cs_rl_mode
 * Desc:     Execute Read leveling for single Chip select
 * Args:     cs        - current chip select
 *           freq      - current sequence frequency
 *           ecc       - ecc iteration indication
 *           dram_info - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
static int ddr3_read_leveling_single_cs_rl_mode(u32 cs, u32 freq,
						int ratio_2to1, u32 ecc,
						MV_DRAM_INFO *dram_info)
{
	u32 reg, delay, phase, pup, rd_sample_delay, add, locked_pups,
		repeat_max_cnt, sdram_offset, locked_sum;
	u32 phase_min, ui_max_delay;
	int all_locked, first_octet_locked, counter_in_progress;
	int final_delay = 0;

	DEBUG_RL_FULL_C("DDR3 - Read Leveling - Single CS - ", (u32) cs, 1);

	/* Init values */
	phase = 0;
	delay = 0;
	rd_sample_delay = dram_info->cl;
	all_locked = 0;
	first_octet_locked = 0;
	repeat_max_cnt = 0;
	locked_sum = 0;

	for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc);
	     pup++)
		dram_info->rl_val[cs][pup + ecc * ECC_BIT][S] = 0;

	/* Main loop */
	while (!all_locked) {
		counter_in_progress = 0;

		DEBUG_RL_FULL_S("DDR3 - Read Leveling - RdSmplDly = ");
		DEBUG_RL_FULL_D(rd_sample_delay, 2);
		DEBUG_RL_FULL_S(", RdRdyDly = ");
		DEBUG_RL_FULL_D(dram_info->rd_rdy_dly, 2);
		DEBUG_RL_FULL_S(", Phase = ");
		DEBUG_RL_FULL_D(phase, 1);
		DEBUG_RL_FULL_S(", Delay = ");
		DEBUG_RL_FULL_D(delay, 2);
		DEBUG_RL_FULL_S("\n");

		/*
		 * Broadcast to all PUPs current RL delays: DQS phase,
		 * leveling delay
		 */
		ddr3_write_pup_reg(PUP_RL_MODE, cs, PUP_BC, phase, delay);

		/* Reset PHY read FIFO */
		reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
			(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
		/* 0x15B8 - Training SW 2 Register */
		reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

		do {
			reg = (reg_read(REG_DRAM_TRAINING_2_ADDR)) &
				(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
		} while (reg);	/* Wait for '0' */

		/* Read pattern from SDRAM */
		sdram_offset = cs * (SDRAM_CS_SIZE + 1) + SDRAM_RL_OFFS;
		locked_pups = 0;
		if (MV_OK !=
		    ddr3_sdram_compare(dram_info, 0xFF, &locked_pups,
				       rl_pattern, LEN_STD_PATTERN,
				       sdram_offset, 0, 0, NULL, 0))
			return MV_DDR3_TRAINING_ERR_RD_LVL_RL_PATTERN;

		/* Octet evaluation */
		/* pup_num = Q or 1 for ECC */
		for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc); pup++) {
			/* Check Overrun */
			if (!((reg_read(REG_DRAM_TRAINING_2_ADDR) >>
			       (REG_DRAM_TRAINING_2_OVERRUN_OFFS + pup)) & 0x1)) {
				overrun(cs, dram_info, pup, locked_pups,
					&locked_sum, ecc, &first_octet_locked,
					&counter_in_progress, final_delay,
					delay, phase);
			} else {
				DEBUG_RL_FULL_C("DDR3 - Read Leveling - We got overrun on pup: ",
						(u32)pup, 1);
			}
		}

		if (locked_sum == (dram_info->num_of_std_pups *
				   (1 - ecc) + ecc)) {
			all_locked = 1;
			DEBUG_RL_FULL_S("DDR3 - Read Leveling - Single Cs - All pups locked\n");
		}

		/*
		 * This is a fix for unstable condition where pups are
		 * toggling between match and no match
		 */
		/*
		 * If some of the pups is >1 <3, check if we did it too
		 * many times
		 */
		if (counter_in_progress == 1) {
			/* Notify at least one Counter is >=1 and < 3 */
			if (repeat_max_cnt < RL_RETRY_COUNT) {
				repeat_max_cnt++;
				counter_in_progress = 1;
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - Counter is >=1 and <3\n");
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - So we will not increment the delay to see if locked again\n");
			} else {
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - repeat_max_cnt reached max so now we will increment the delay\n");
				counter_in_progress = 0;
			}
		}

		/*
		 * Check some of the pups are in the middle of state machine
		 * and don't increment the delays
		 */
		if (!counter_in_progress && !all_locked) {
			int idx;

			idx = pup + ecc * ECC_BIT;

			repeat_max_cnt = 0;
			/* if 1:1 mode */
			if ((!ratio_2to1) && ((phase == 0) || (phase == 4)))
				ui_max_delay = MAX_DELAY_INV;
			else
				ui_max_delay = MAX_DELAY;

			/* Increment Delay */
			if (delay < ui_max_delay) {
				delay++;
				/*
				 * Mark the last delay/pahse place for
				 * window final place
				 */
				if (delay == ui_max_delay) {
					if ((!ratio_2to1 && phase ==
					     MAX_PHASE_RL_L_1TO1)
					    || (ratio_2to1 && phase ==
						MAX_PHASE_RL_L_2TO1))
						final_delay = 1;
				}
			} else {
				/* Phase+CL Incrementation */
				delay = 0;

				if (!ratio_2to1) {
					/* 1:1 mode */
					if (first_octet_locked) {
						/* some Pup was Locked */
						if (phase < MAX_PHASE_RL_L_1TO1) {
							if (phase == 1) {
								phase = 4;
							} else {
								phase++;
								delay = MIN_DELAY_PHASE_1_LIMIT;
							}
						} else {
							DEBUG_RL_FULL_S("DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
							DEBUG_RL_S("1)DDR3 - Read Leveling - ERROR - NOT all PUPs Locked n");
							return MV_DDR3_TRAINING_ERR_RD_LVL_RL_PUP_UNLOCK;
						}
					} else {
						/* NO Pup was Locked */
						if (phase < MAX_PHASE_RL_UL_1TO1) {
							phase++;
							delay =
							    MIN_DELAY_PHASE_1_LIMIT;
						} else {
							phase = 0;
						}
					}
				} else {
					/* 2:1 mode */
					if (first_octet_locked) {
						/* some Pup was Locked */
						if (phase < MAX_PHASE_RL_L_2TO1) {
							phase++;
						} else {
							DEBUG_RL_FULL_S("DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
							DEBUG_RL_S("2)DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
							for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc); pup++) {
								/* pup_num = Q or 1 for ECC */
								if (dram_info->rl_val[cs][idx][S]
								    == 0) {
									DEBUG_RL_C("Failed byte is = ",
										   pup, 1);
								}
							}
							return MV_DDR3_TRAINING_ERR_RD_LVL_RL_PUP_UNLOCK;
						}
					} else {
						/* No Pup was Locked */
						if (phase < MAX_PHASE_RL_UL_2TO1)
							phase++;
						else
							phase = 0;
					}
				}

				/*
				 * If we finished a full Phases cycle (so now
				 * phase = 0, need to increment rd_sample_dly
				 */
				if (phase == 0 && first_octet_locked == 0) {
					rd_sample_delay++;
					if (rd_sample_delay == 0x10) {
						DEBUG_RL_FULL_S("DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
						DEBUG_RL_S("3)DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
						for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc); pup++) {
							/* pup_num = Q or 1 for ECC */
							if (dram_info->
							    rl_val[cs][idx][S] == 0) {
								DEBUG_RL_C("Failed byte is = ",
									   pup, 1);
							}
						}
						return MV_DDR3_TRAINING_ERR_RD_LVL_PUP_UNLOCK;
					}

					/* Set current rd_sample_delay  */
					reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
					reg &= ~(REG_READ_DATA_SAMPLE_DELAYS_MASK
					      << (REG_READ_DATA_SAMPLE_DELAYS_OFFS
						  * cs));
					reg |= (rd_sample_delay <<
						(REG_READ_DATA_SAMPLE_DELAYS_OFFS *
						 cs));
					reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR,
						  reg);
				}

				/*
				 * Set current rdReadyDelay according to the
				 * hash table (Need to do this in every phase
				 * change)
				 */
				if (!ratio_2to1) {
					/* 1:1 mode */
					add = reg_read(REG_TRAINING_DEBUG_2_ADDR);
					switch (phase) {
					case 0:
						add = (add >>
						       REG_TRAINING_DEBUG_2_OFFS);
						break;
					case 1:
						add = (add >>
						       (REG_TRAINING_DEBUG_2_OFFS
							+ 3));
						break;
					case 4:
						add = (add >>
						       (REG_TRAINING_DEBUG_2_OFFS
							+ 6));
						break;
					case 5:
						add = (add >>
						       (REG_TRAINING_DEBUG_2_OFFS
							+ 9));
						break;
					}
					add &= REG_TRAINING_DEBUG_2_MASK;
				} else {
					/* 2:1 mode */
					add = reg_read(REG_TRAINING_DEBUG_3_ADDR);
					add = (add >>
					       (phase *
						REG_TRAINING_DEBUG_3_OFFS));
					add &= REG_TRAINING_DEBUG_3_MASK;
				}

				reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
				reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
					 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
				reg |= ((rd_sample_delay + add) <<
					(REG_READ_DATA_READY_DELAYS_OFFS * cs));
				reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
				dram_info->rd_smpl_dly = rd_sample_delay;
				dram_info->rd_rdy_dly = rd_sample_delay + add;
			}

			/* Reset counters for pups with states<RD_STATE_COUNT */
			for (pup = 0; pup <
				     (dram_info->num_of_std_pups * (1 - ecc) + ecc);
			     pup++) {
				if (dram_info->rl_val[cs][idx][C] < RL_RETRY_COUNT)
					dram_info->rl_val[cs][idx][C] = 0;
			}
		}
	}

	phase_min = 10;

	for (pup = 0; pup < (dram_info->num_of_std_pups); pup++) {
		if (dram_info->rl_val[cs][pup][PS] < phase_min)
			phase_min = dram_info->rl_val[cs][pup][PS];
	}

	/*
	 * Set current rdReadyDelay according to the hash table (Need to
	 * do this in every phase change)
	 */
	if (!ratio_2to1) {
		/* 1:1 mode */
		add = reg_read(REG_TRAINING_DEBUG_2_ADDR);
		switch (phase_min) {
		case 0:
			add = (add >> REG_TRAINING_DEBUG_2_OFFS);
			break;
		case 1:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 3));
			break;
		case 4:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 6));
			break;
		case 5:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 9));
			break;
		}
		add &= REG_TRAINING_DEBUG_2_MASK;
	} else {
		/* 2:1 mode */
		add = reg_read(REG_TRAINING_DEBUG_3_ADDR);
		add = (add >> (phase_min * REG_TRAINING_DEBUG_3_OFFS));
		add &= REG_TRAINING_DEBUG_3_MASK;
	}

	reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
	reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
		 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
	reg |= ((rd_sample_delay + add) << (REG_READ_DATA_READY_DELAYS_OFFS * cs));
	reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
	dram_info->rd_rdy_dly = rd_sample_delay + add;

	for (cs = 0; cs < dram_info->num_cs; cs++) {
		for (pup = 0; pup < dram_info->num_of_total_pups; pup++) {
			reg = ddr3_read_pup_reg(PUP_RL_MODE + 0x1, cs, pup);
			dram_info->rl_val[cs][pup][DQS] = (reg & 0x3F);
		}
	}

	return MV_OK;
}

#else

/*
 * Name:     ddr3_read_leveling_single_cs_window_mode
 * Desc:     Execute Read leveling for single Chip select
 * Args:     cs        - current chip select
 *           freq      - current sequence frequency
 *           ecc       - ecc iteration indication
 *           dram_info - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
static int ddr3_read_leveling_single_cs_window_mode(u32 cs, u32 freq,
						    int ratio_2to1, u32 ecc,
						    MV_DRAM_INFO *dram_info)
{
	u32 reg, delay, phase, sum, pup, rd_sample_delay, add, locked_pups,
	    repeat_max_cnt, sdram_offset, final_sum, locked_sum;
	u32 delay_s, delay_e, tmp, phase_min, ui_max_delay;
	int all_locked, first_octet_locked, counter_in_progress;
	int final_delay = 0;

	DEBUG_RL_FULL_C("DDR3 - Read Leveling - Single CS - ", (u32) cs, 1);

	/* Init values */
	phase = 0;
	delay = 0;
	rd_sample_delay = dram_info->cl;
	all_locked = 0;
	first_octet_locked = 0;
	repeat_max_cnt = 0;
	sum = 0;
	final_sum = 0;
	locked_sum = 0;

	for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc);
	     pup++)
		dram_info->rl_val[cs][pup + ecc * ECC_BIT][S] = 0;

	/* Main loop */
	while (!all_locked) {
		counter_in_progress = 0;

		DEBUG_RL_FULL_S("DDR3 - Read Leveling - RdSmplDly = ");
		DEBUG_RL_FULL_D(rd_sample_delay, 2);
		DEBUG_RL_FULL_S(", RdRdyDly = ");
		DEBUG_RL_FULL_D(dram_info->rd_rdy_dly, 2);
		DEBUG_RL_FULL_S(", Phase = ");
		DEBUG_RL_FULL_D(phase, 1);
		DEBUG_RL_FULL_S(", Delay = ");
		DEBUG_RL_FULL_D(delay, 2);
		DEBUG_RL_FULL_S("\n");

		/*
		 * Broadcast to all PUPs current RL delays: DQS phase,leveling
		 * delay
		 */
		ddr3_write_pup_reg(PUP_RL_MODE, cs, PUP_BC, phase, delay);

		/* Reset PHY read FIFO */
		reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
			(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
		/* 0x15B8 - Training SW 2 Register */
		reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

		do {
			reg = (reg_read(REG_DRAM_TRAINING_2_ADDR)) &
				(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
		} while (reg);	/* Wait for '0' */

		/* Read pattern from SDRAM */
		sdram_offset = cs * (SDRAM_CS_SIZE + 1) + SDRAM_RL_OFFS;
		locked_pups = 0;
		if (MV_OK !=
		    ddr3_sdram_compare(dram_info, 0xFF, &locked_pups,
				       rl_pattern, LEN_STD_PATTERN,
				       sdram_offset, 0, 0, NULL, 0))
			return MV_DDR3_TRAINING_ERR_RD_LVL_WIN_PATTERN;

		/* Octet evaluation */
		for (pup = 0; pup < (dram_info->num_of_std_pups *
				     (1 - ecc) + ecc); pup++) {
			/* pup_num = Q or 1 for ECC */
			int idx;

			idx = pup + ecc * ECC_BIT;

			/* Check Overrun */
			if (!((reg_read(REG_DRAM_TRAINING_2_ADDR) >>
			      (REG_DRAM_TRAINING_2_OVERRUN_OFFS +
			       pup)) & 0x1)) {
				/* If no OverRun */

				/* Inside the window */
				if (dram_info->rl_val[cs][idx][S] == RL_WINDOW_STATE) {
					/*
					 * Match expected value ? - Update
					 * State Machine
					 */
					if (((~locked_pups >> pup) & 0x1)
					    && (final_delay == 0)) {
						/* Match - Still inside the Window */
						DEBUG_RL_FULL_C("DDR3 - Read Leveling - We got another match inside the window  for pup: ",
								(u32)pup, 1);

					} else {
						/* We got fail -> this is the end of the window */
						dram_info->rl_val[cs][idx][DE] = delay;
						dram_info->rl_val[cs][idx][PE] = phase;
						/* Go to Final State */
						dram_info->rl_val[cs][idx][S]++;
						final_sum++;
						DEBUG_RL_FULL_C("DDR3 - Read Leveling - We finished the window for pup: ",
								(u32)pup, 1);
					}

					/* Before the start of the window */
				} else if (dram_info->rl_val[cs][idx][S] ==
					   RL_UNLOCK_STATE) {
					/* Must be RL_UNLOCK_STATE */
					/*
					 * Match expected value ? - Update
					 * State Machine
					 */
					if (dram_info->rl_val[cs][idx][C] <
					    RL_RETRY_COUNT) {
						if (((~locked_pups >> pup) & 0x1)) {
							/* Match */
							DEBUG_RL_FULL_C("DDR3 - Read Leveling - We have no overrun and a match on pup: ",
									(u32)pup, 1);
							dram_info->rl_val[cs][idx][C]++;

							/* If pup got to last state - lock the delays */
							if (dram_info->rl_val[cs][idx][C] ==
							    RL_RETRY_COUNT) {
								dram_info->rl_val[cs][idx][C] = 0;
								dram_info->rl_val[cs][idx][DS] =
									delay;
								dram_info->rl_val[cs][idx][PS] =
									phase;
								dram_info->rl_val[cs][idx][S]++;	/* Go to Window State */
								locked_sum++;
								/* Will count the pups that got locked */

								/* IF First lock - need to lock delays */
								if (first_octet_locked == 0) {
									DEBUG_RL_FULL_C("DDR3 - Read Leveling - We got first lock on pup: ",
											(u32)pup, 1);
									first_octet_locked
									    =
									    1;
								}
							}

							/* if pup is in not in final state but there was match - dont increment counter */
							else {
								counter_in_progress
								    = 1;
							}
						}
					}
				}
			} else {
				DEBUG_RL_FULL_C("DDR3 - Read Leveling - We got overrun on pup: ",
						(u32)pup, 1);
				counter_in_progress = 1;
			}
		}

		if (final_sum == (dram_info->num_of_std_pups * (1 - ecc) + ecc)) {
			all_locked = 1;
			DEBUG_RL_FULL_S("DDR3 - Read Leveling - Single Cs - All pups locked\n");
		}

		/*
		 * This is a fix for unstable condition where pups are
		 * toggling between match and no match
		 */
		/*
		 * If some of the pups is >1 <3, check if we did it too many
		 * times
		 */
		if (counter_in_progress == 1) {
			if (repeat_max_cnt < RL_RETRY_COUNT) {
				/* Notify at least one Counter is >=1 and < 3 */
				repeat_max_cnt++;
				counter_in_progress = 1;
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - Counter is >=1 and <3\n");
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - So we will not increment the delay to see if locked again\n");
			} else {
				DEBUG_RL_FULL_S("DDR3 - Read Leveling - repeat_max_cnt reached max so now we will increment the delay\n");
				counter_in_progress = 0;
			}
		}

		/*
		 * Check some of the pups are in the middle of state machine
		 * and don't increment the delays
		 */
		if (!counter_in_progress && !all_locked) {
			repeat_max_cnt = 0;
			if (!ratio_2to1)
				ui_max_delay = MAX_DELAY_INV;
			else
				ui_max_delay = MAX_DELAY;

			/* Increment Delay */
			if (delay < ui_max_delay) {
				/* Delay Incrementation */
				delay++;
				if (delay == ui_max_delay) {
					/*
					 * Mark the last delay/pahse place
					 * for window final place
					 */
					if ((!ratio_2to1
					     && phase == MAX_PHASE_RL_L_1TO1)
					    || (ratio_2to1
						&& phase ==
						MAX_PHASE_RL_L_2TO1))
						final_delay = 1;
				}
			} else {
				/* Phase+CL Incrementation */
				delay = 0;
				if (!ratio_2to1) {
					/* 1:1 mode */
					if (first_octet_locked) {
						/* some pupet was Locked */
						if (phase < MAX_PHASE_RL_L_1TO1) {
#ifdef RL_WINDOW_WA
							if (phase == 0)
#else
							if (phase == 1)
#endif
								phase = 4;
							else
								phase++;
						} else {
							DEBUG_RL_FULL_S("DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
							return MV_DDR3_TRAINING_ERR_RD_LVL_WIN_PUP_UNLOCK;
						}
					} else {
						/* No Pup was Locked */
						if (phase < MAX_PHASE_RL_UL_1TO1) {
#ifdef RL_WINDOW_WA
							if (phase == 0)
								phase = 4;
#else
							phase++;
#endif
						} else
							phase = 0;
					}
				} else {
					/* 2:1 mode */
					if (first_octet_locked) {
						/* Some Pup was Locked */
						if (phase < MAX_PHASE_RL_L_2TO1) {
							phase++;
						} else {
							DEBUG_RL_FULL_S("DDR3 - Read Leveling - ERROR - NOT all PUPs Locked\n");
							return MV_DDR3_TRAINING_ERR_RD_LVL_WIN_PUP_UNLOCK;
						}
					} else {
						/* No Pup was Locked */
						if (phase < MAX_PHASE_RL_UL_2TO1)
							phase++;
						else
							phase = 0;
					}
				}

				/*
				 * If we finished a full Phases cycle (so
				 * now phase = 0, need to increment
				 * rd_sample_dly
				 */
				if (phase == 0 && first_octet_locked == 0) {
					rd_sample_delay++;

					/* Set current rd_sample_delay  */
					reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
					reg &= ~(REG_READ_DATA_SAMPLE_DELAYS_MASK <<
						 (REG_READ_DATA_SAMPLE_DELAYS_OFFS
						  * cs));
					reg |= (rd_sample_delay <<
						(REG_READ_DATA_SAMPLE_DELAYS_OFFS *
						 cs));
					reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR,
						  reg);
				}

				/*
				 * Set current rdReadyDelay according to the
				 * hash table (Need to do this in every phase
				 * change)
				 */
				if (!ratio_2to1) {
					/* 1:1 mode */
					add = reg_read(REG_TRAINING_DEBUG_2_ADDR);
					switch (phase) {
					case 0:
						add = add >>
							REG_TRAINING_DEBUG_2_OFFS;
						break;
					case 1:
						add = add >>
							(REG_TRAINING_DEBUG_2_OFFS
							 + 3);
						break;
					case 4:
						add = add >>
							(REG_TRAINING_DEBUG_2_OFFS
							 + 6);
						break;
					case 5:
						add = add >>
							(REG_TRAINING_DEBUG_2_OFFS
							 + 9);
						break;
					}
				} else {
					/* 2:1 mode */
					add = reg_read(REG_TRAINING_DEBUG_3_ADDR);
					add = (add >> phase *
					       REG_TRAINING_DEBUG_3_OFFS);
				}
				add &= REG_TRAINING_DEBUG_2_MASK;
				reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
				reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
					 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
				reg |= ((rd_sample_delay + add) <<
					(REG_READ_DATA_READY_DELAYS_OFFS * cs));
				reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
				dram_info->rd_smpl_dly = rd_sample_delay;
				dram_info->rd_rdy_dly = rd_sample_delay + add;
			}

			/* Reset counters for pups with states<RD_STATE_COUNT */
			for (pup = 0;
			     pup <
			     (dram_info->num_of_std_pups * (1 - ecc) + ecc);
			     pup++) {
				if (dram_info->rl_val[cs][idx][C] < RL_RETRY_COUNT)
					dram_info->rl_val[cs][idx][C] = 0;
			}
		}
	}

	phase_min = 10;

	for (pup = 0; pup < (dram_info->num_of_std_pups); pup++) {
		DEBUG_RL_S("DDR3 - Read Leveling - Window info - PUP: ");
		DEBUG_RL_D((u32) pup, 1);
		DEBUG_RL_S(", PS: ");
		DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][PS], 1);
		DEBUG_RL_S(", DS: ");
		DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][DS], 2);
		DEBUG_RL_S(", PE: ");
		DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][PE], 1);
		DEBUG_RL_S(", DE: ");
		DEBUG_RL_D((u32) dram_info->rl_val[cs][pup][DE], 2);
		DEBUG_RL_S("\n");
	}

	/* Find center of the window procedure */
	for (pup = 0; pup < (dram_info->num_of_std_pups * (1 - ecc) + ecc);
	     pup++) {
#ifdef RL_WINDOW_WA
		if (!ratio_2to1) {	/* 1:1 mode */
			if (dram_info->rl_val[cs][idx][PS] == 4)
				dram_info->rl_val[cs][idx][PS] = 1;
			if (dram_info->rl_val[cs][idx][PE] == 4)
				dram_info->rl_val[cs][idx][PE] = 1;

			delay_s = dram_info->rl_val[cs][idx][PS] *
				MAX_DELAY_INV + dram_info->rl_val[cs][idx][DS];
			delay_e = dram_info->rl_val[cs][idx][PE] *
				MAX_DELAY_INV + dram_info->rl_val[cs][idx][DE];

			tmp = (delay_e - delay_s) / 2 + delay_s;
			phase = tmp / MAX_DELAY_INV;
			if (phase == 1)	/* 1:1 mode */
				phase = 4;

			if (phase < phase_min)	/* for the read ready delay */
				phase_min = phase;

			dram_info->rl_val[cs][idx][P] = phase;
			dram_info->rl_val[cs][idx][D] = tmp % MAX_DELAY_INV;

		} else {
			delay_s = dram_info->rl_val[cs][idx][PS] *
				MAX_DELAY + dram_info->rl_val[cs][idx][DS];
			delay_e = dram_info->rl_val[cs][idx][PE] *
				MAX_DELAY + dram_info->rl_val[cs][idx][DE];

			tmp = (delay_e - delay_s) / 2 + delay_s;
			phase = tmp / MAX_DELAY;

			if (phase < phase_min)	/* for the read ready delay */
				phase_min = phase;

			dram_info->rl_val[cs][idx][P] = phase;
			dram_info->rl_val[cs][idx][D] = tmp % MAX_DELAY;
		}
#else
		if (!ratio_2to1) {	/* 1:1 mode */
			if (dram_info->rl_val[cs][idx][PS] > 1)
				dram_info->rl_val[cs][idx][PS] -= 2;
			if (dram_info->rl_val[cs][idx][PE] > 1)
				dram_info->rl_val[cs][idx][PE] -= 2;
		}

		delay_s = dram_info->rl_val[cs][idx][PS] * MAX_DELAY +
			dram_info->rl_val[cs][idx][DS];
		delay_e = dram_info->rl_val[cs][idx][PE] * MAX_DELAY +
			dram_info->rl_val[cs][idx][DE];

		tmp = (delay_e - delay_s) / 2 + delay_s;
		phase = tmp / MAX_DELAY;
		if (!ratio_2to1 && phase > 1)	/* 1:1 mode */
			phase += 2;

		if (phase < phase_min)	/* for the read ready delay */
			phase_min = phase;

		dram_info->rl_val[cs][idx][P] = phase;
		dram_info->rl_val[cs][idx][D] = tmp % MAX_DELAY;
#endif
	}

	/* Set current rdReadyDelay according to the hash table (Need to do this in every phase change) */
	if (!ratio_2to1) {	/* 1:1 mode */
		add = reg_read(REG_TRAINING_DEBUG_2_ADDR);
		switch (phase_min) {
		case 0:
			add = (add >> REG_TRAINING_DEBUG_2_OFFS);
			break;
		case 1:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 3));
			break;
		case 4:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 6));
			break;
		case 5:
			add = (add >> (REG_TRAINING_DEBUG_2_OFFS + 9));
			break;
		}
	} else {		/* 2:1 mode */
		add = reg_read(REG_TRAINING_DEBUG_3_ADDR);
		add = (add >> phase_min * REG_TRAINING_DEBUG_3_OFFS);
	}

	add &= REG_TRAINING_DEBUG_2_MASK;
	reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
	reg &=
	    ~(REG_READ_DATA_READY_DELAYS_MASK <<
	      (REG_READ_DATA_READY_DELAYS_OFFS * cs));
	reg |=
	    ((rd_sample_delay + add) << (REG_READ_DATA_READY_DELAYS_OFFS * cs));
	reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
	dram_info->rd_rdy_dly = rd_sample_delay + add;

	for (cs = 0; cs < dram_info->num_cs; cs++) {
		for (pup = 0; pup < dram_info->num_of_total_pups; pup++) {
			reg = ddr3_read_pup_reg(PUP_RL_MODE + 0x1, cs, pup);
			dram_info->rl_val[cs][pup][DQS] = (reg & 0x3F);
		}
	}

	return MV_OK;
}
#endif
