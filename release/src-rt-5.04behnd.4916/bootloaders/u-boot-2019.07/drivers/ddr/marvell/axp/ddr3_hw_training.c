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

#include "ddr3_init.h"
#include "ddr3_hw_training.h"
#include "xor.h"

#ifdef MV88F78X60
#include "ddr3_patterns_64bit.h"
#else
#include "ddr3_patterns_16bit.h"
#if defined(MV88F672X)
#include "ddr3_patterns_16bit.h"
#endif
#endif

/*
 * Debug
 */

#define DEBUG_MAIN_C(s, d, l) \
	DEBUG_MAIN_S(s); DEBUG_MAIN_D(d, l); DEBUG_MAIN_S("\n")
#define DEBUG_MAIN_FULL_C(s, d, l) \
	DEBUG_MAIN_FULL_S(s); DEBUG_MAIN_FULL_D(d, l); DEBUG_MAIN_FULL_S("\n")

#ifdef MV_DEBUG_MAIN
#define DEBUG_MAIN_S(s)			puts(s)
#define DEBUG_MAIN_D(d, l)		printf("%x", d)
#else
#define DEBUG_MAIN_S(s)
#define DEBUG_MAIN_D(d, l)
#endif

#ifdef MV_DEBUG_MAIN_FULL
#define DEBUG_MAIN_FULL_S(s)		puts(s)
#define DEBUG_MAIN_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_MAIN_FULL_S(s)
#define DEBUG_MAIN_FULL_D(d, l)
#endif

#ifdef MV_DEBUG_SUSPEND_RESUME
#define DEBUG_SUSPEND_RESUME_S(s)	puts(s)
#define DEBUG_SUSPEND_RESUME_D(d, l)	printf("%x", d)
#else
#define DEBUG_SUSPEND_RESUME_S(s)
#define DEBUG_SUSPEND_RESUME_D(d, l)
#endif

static u32 ddr3_sw_wl_rl_debug;
static u32 ddr3_run_pbs = 1;

void ddr3_print_version(void)
{
	puts("DDR3 Training Sequence - Ver 5.7.");
}

void ddr3_set_sw_wl_rl_debug(u32 val)
{
	ddr3_sw_wl_rl_debug = val;
}

void ddr3_set_pbs(u32 val)
{
	ddr3_run_pbs = val;
}

int ddr3_hw_training(u32 target_freq, u32 ddr_width, int xor_bypass,
		     u32 scrub_offs, u32 scrub_size, int dqs_clk_aligned,
		     int debug_mode, int reg_dimm_skip_wl)
{
	/* A370 has no PBS mechanism */
	__maybe_unused u32 first_loop_flag = 0;
	u32 freq, reg;
	MV_DRAM_INFO dram_info;
	int ratio_2to1 = 0;
	int tmp_ratio = 1;
	int status;

	if (debug_mode)
		DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 1\n");

	memset(&dram_info, 0, sizeof(dram_info));
	dram_info.num_cs = ddr3_get_cs_num_from_reg();
	dram_info.cs_ena = ddr3_get_cs_ena_from_reg();
	dram_info.target_frequency = target_freq;
	dram_info.ddr_width = ddr_width;
	dram_info.num_of_std_pups = ddr_width / PUP_SIZE;
	dram_info.rl400_bug = 0;
	dram_info.multi_cs_mr_support = 0;
#ifdef MV88F67XX
	dram_info.rl400_bug = 1;
#endif

	/* Ignore ECC errors - if ECC is enabled */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR);
	if (reg & (1 << REG_SDRAM_CONFIG_ECC_OFFS)) {
		dram_info.ecc_ena = 1;
		reg |= (1 << REG_SDRAM_CONFIG_IERR_OFFS);
		reg_write(REG_SDRAM_CONFIG_ADDR, reg);
	} else {
		dram_info.ecc_ena = 0;
	}

	reg = reg_read(REG_SDRAM_CONFIG_ADDR);
	if (reg & (1 << REG_SDRAM_CONFIG_REGDIMM_OFFS))
		dram_info.reg_dimm = 1;
	else
		dram_info.reg_dimm = 0;

	dram_info.num_of_total_pups = ddr_width / PUP_SIZE + dram_info.ecc_ena;

	/* Get target 2T value */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	dram_info.mode_2t = (reg >> REG_DUNIT_CTRL_LOW_2T_OFFS) &
		REG_DUNIT_CTRL_LOW_2T_MASK;

	/* Get target CL value */
#ifdef MV88F67XX
	reg = reg_read(REG_DDR3_MR0_ADDR) >> 2;
#else
	reg = reg_read(REG_DDR3_MR0_CS_ADDR) >> 2;
#endif

	reg = (((reg >> 1) & 0xE) | (reg & 0x1)) & 0xF;
	dram_info.cl = ddr3_valid_cl_to_cl(reg);

	/* Get target CWL value */
#ifdef MV88F67XX
	reg = reg_read(REG_DDR3_MR2_ADDR) >> REG_DDR3_MR2_CWL_OFFS;
#else
	reg = reg_read(REG_DDR3_MR2_CS_ADDR) >> REG_DDR3_MR2_CWL_OFFS;
#endif

	reg &= REG_DDR3_MR2_CWL_MASK;
	dram_info.cwl = reg;
#if !defined(MV88F67XX)
	/* A370 has no PBS mechanism */
#if defined(MV88F78X60)
	if ((dram_info.target_frequency > DDR_400) && (ddr3_run_pbs))
		first_loop_flag = 1;
#else
	/* first_loop_flag = 1; skip mid freq at ALP/A375 */
	if ((dram_info.target_frequency > DDR_400) && (ddr3_run_pbs) &&
	    (mv_ctrl_revision_get() >= UMC_A0))
		first_loop_flag = 1;
	else
		first_loop_flag = 0;
#endif
#endif

	freq = dram_info.target_frequency;

	/* Set ODT to always on */
	ddr3_odt_activate(1);

	/* Init XOR */
	mv_sys_xor_init(&dram_info);

	/* Get DRAM/HCLK ratio */
	if (reg_read(REG_DDR_IO_ADDR) & (1 << REG_DDR_IO_CLK_RATIO_OFFS))
		ratio_2to1 = 1;

	/*
	 * Xor Bypass - ECC support in AXP is currently available for 1:1
	 * modes frequency modes.
	 * Not all frequency modes support the ddr3 training sequence
	 * (Only 1200/300).
	 * Xor Bypass allows using the Xor initializations and scrubbing
	 * inside the ddr3 training sequence without running the training
	 * itself.
	 */
	if (xor_bypass == 0) {
		if (ddr3_run_pbs) {
			DEBUG_MAIN_S("DDR3 Training Sequence - Run with PBS.\n");
		} else {
			DEBUG_MAIN_S("DDR3 Training Sequence - Run without PBS.\n");
		}

		if (dram_info.target_frequency > DFS_MARGIN) {
			tmp_ratio = 0;
			freq = DDR_100;

			if (dram_info.reg_dimm == 1)
				freq = DDR_300;

			if (MV_OK != ddr3_dfs_high_2_low(freq, &dram_info)) {
				/* Set low - 100Mhz DDR Frequency by HW */
				DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Dfs High2Low)\n");
				return MV_DDR3_TRAINING_ERR_DFS_H2L;
			}

			if ((dram_info.reg_dimm == 1) &&
			    (reg_dimm_skip_wl == 0)) {
				if (MV_OK !=
				    ddr3_write_leveling_hw_reg_dimm(freq,
								    &dram_info))
					DEBUG_MAIN_S("DDR3 Training Sequence - Registered DIMM Low WL - SKIP\n");
			}

			if (ddr3_get_log_level() >= MV_LOG_LEVEL_1)
				ddr3_print_freq(freq);

			if (debug_mode)
				DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 2\n");
		} else {
			if (!dqs_clk_aligned) {
#ifdef MV88F67XX
				/*
				 * If running training sequence without DFS,
				 * we must run Write leveling before writing
				 * the patterns
				 */

				/*
				 * ODT - Multi CS system use SW WL,
				 * Single CS System use HW WL
				 */
				if (dram_info.cs_ena > 1) {
					if (MV_OK !=
					    ddr3_write_leveling_sw(
						    freq, tmp_ratio,
						    &dram_info)) {
						DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Sw)\n");
						return MV_DDR3_TRAINING_ERR_WR_LVL_SW;
					}
				} else {
					if (MV_OK !=
					    ddr3_write_leveling_hw(freq,
								   &dram_info)) {
						DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hw)\n");
						return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
					}
				}
#else
				if (MV_OK != ddr3_write_leveling_hw(
					    freq, &dram_info)) {
					DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hw)\n");
					if (ddr3_sw_wl_rl_debug) {
						if (MV_OK !=
						    ddr3_write_leveling_sw(
							    freq, tmp_ratio,
							    &dram_info)) {
							DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Sw)\n");
							return MV_DDR3_TRAINING_ERR_WR_LVL_SW;
						}
					} else {
						return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
					}
				}
#endif
			}

			if (debug_mode)
				DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 3\n");
		}

		if (MV_OK != ddr3_load_patterns(&dram_info, 0)) {
			DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Loading Patterns)\n");
			return MV_DDR3_TRAINING_ERR_LOAD_PATTERNS;
		}

		/*
		 * TODO:
		 * The mainline U-Boot port of the bin_hdr DDR training code
		 * needs a delay of minimum 20ms here (10ms is a bit too short
		 * and the CPU hangs). The bin_hdr code doesn't have this delay.
		 * To be save here, lets add a delay of 50ms here.
		 *
		 * Tested on the Marvell DB-MV784MP-GP board
		 */
		mdelay(50);

		do {
			freq = dram_info.target_frequency;
			tmp_ratio = ratio_2to1;
			DEBUG_MAIN_FULL_S("DDR3 Training Sequence - DEBUG - 4\n");

#if defined(MV88F78X60)
			/*
			 * There is a difference on the DFS frequency at the
			 * first iteration of this loop
			 */
			if (first_loop_flag) {
				freq = DDR_400;
				tmp_ratio = 0;
			}
#endif

			if (MV_OK != ddr3_dfs_low_2_high(freq, tmp_ratio,
							 &dram_info)) {
				DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Dfs Low2High)\n");
				return MV_DDR3_TRAINING_ERR_DFS_H2L;
			}

			if (ddr3_get_log_level() >= MV_LOG_LEVEL_1) {
				ddr3_print_freq(freq);
			}

			if (debug_mode)
				DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 5\n");

			/* Write leveling */
			if (!dqs_clk_aligned) {
#ifdef MV88F67XX
				/*
				 * ODT - Multi CS system that not support Multi
				 * CS MRS commands must use SW WL
				 */
				if (dram_info.cs_ena > 1) {
					if (MV_OK != ddr3_write_leveling_sw(
						    freq, tmp_ratio, &dram_info)) {
						DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Sw)\n");
						return MV_DDR3_TRAINING_ERR_WR_LVL_SW;
					}
				} else {
					if (MV_OK != ddr3_write_leveling_hw(
						    freq, &dram_info)) {
						DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hw)\n");
						return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
					}
				}
#else
				if ((dram_info.reg_dimm == 1) &&
				    (freq == DDR_400)) {
					if (reg_dimm_skip_wl == 0) {
						if (MV_OK != ddr3_write_leveling_hw_reg_dimm(
							    freq, &dram_info))
							DEBUG_MAIN_S("DDR3 Training Sequence - Registered DIMM WL - SKIP\n");
					}
				} else {
					if (MV_OK != ddr3_write_leveling_hw(
						    freq, &dram_info)) {
						DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hw)\n");
						if (ddr3_sw_wl_rl_debug) {
							if (MV_OK != ddr3_write_leveling_sw(
								    freq, tmp_ratio, &dram_info)) {
								DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Sw)\n");
								return MV_DDR3_TRAINING_ERR_WR_LVL_SW;
							}
						} else {
							return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
						}
					}
				}
#endif
				if (debug_mode)
					DEBUG_MAIN_S
					    ("DDR3 Training Sequence - DEBUG - 6\n");
			}

			/* Read Leveling */
			/*
			 * Armada 370 - Support for HCLK @ 400MHZ - must use
			 * SW read leveling
			 */
			if (freq == DDR_400 && dram_info.rl400_bug) {
				status = ddr3_read_leveling_sw(freq, tmp_ratio,
						       &dram_info);
				if (MV_OK != status) {
					DEBUG_MAIN_S
					    ("DDR3 Training Sequence - FAILED (Read Leveling Sw)\n");
					return status;
				}
			} else {
				if (MV_OK != ddr3_read_leveling_hw(
					    freq, &dram_info)) {
					DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Read Leveling Hw)\n");
					if (ddr3_sw_wl_rl_debug) {
						if (MV_OK != ddr3_read_leveling_sw(
							    freq, tmp_ratio,
							    &dram_info)) {
							DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Read Leveling Sw)\n");
							return MV_DDR3_TRAINING_ERR_WR_LVL_SW;
						}
					} else {
						return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
					}
				}
			}

			if (debug_mode)
				DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 7\n");

			if (MV_OK != ddr3_wl_supplement(&dram_info)) {
				DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hi-Freq Sup)\n");
				return MV_DDR3_TRAINING_ERR_WR_LVL_HI_FREQ;
			}

			if (debug_mode)
				DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 8\n");
#if !defined(MV88F67XX)
			/* A370 has no PBS mechanism */
#if defined(MV88F78X60) || defined(MV88F672X)
			if (first_loop_flag == 1) {
				first_loop_flag = 0;

				status = MV_OK;
				status = ddr3_pbs_rx(&dram_info);
				if (MV_OK != status) {
					DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (PBS RX)\n");
					return status;
				}

				if (debug_mode)
					DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 9\n");

				status = ddr3_pbs_tx(&dram_info);
				if (MV_OK != status) {
					DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (PBS TX)\n");
					return status;
				}

				if (debug_mode)
					DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 10\n");
			}
#endif
#endif
		} while (freq != dram_info.target_frequency);

		status = ddr3_dqs_centralization_rx(&dram_info);
		if (MV_OK != status) {
			DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (DQS Centralization RX)\n");
			return status;
		}

		if (debug_mode)
			DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 11\n");

		status = ddr3_dqs_centralization_tx(&dram_info);
		if (MV_OK != status) {
			DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (DQS Centralization TX)\n");
			return status;
		}

		if (debug_mode)
			DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 12\n");
	}

	ddr3_set_performance_params(&dram_info);

	if (dram_info.ecc_ena) {
		/* Need to SCRUB the DRAM memory area to load U-Boot */
		mv_sys_xor_finish();
		dram_info.num_cs = 1;
		dram_info.cs_ena = 1;
		mv_sys_xor_init(&dram_info);
		mv_xor_mem_init(0, scrub_offs, scrub_size, 0xdeadbeef,
				0xdeadbeef);

		/* Wait for previous transfer completion */
		while (mv_xor_state_get(0) != MV_IDLE)
			;

		if (debug_mode)
			DEBUG_MAIN_S("DDR3 Training Sequence - DEBUG - 13\n");
	}

	/* Return XOR State */
	mv_sys_xor_finish();

#if defined(MV88F78X60)
	/* Save training results in memeory for resume state */
	ddr3_save_training(&dram_info);
#endif
	/* Clear ODT always on */
	ddr3_odt_activate(0);

	/* Configure Dynamic read ODT */
	ddr3_odt_read_dynamic_config(&dram_info);

	return MV_OK;
}

void ddr3_set_performance_params(MV_DRAM_INFO *dram_info)
{
	u32 twr2wr, trd2rd, trd2wr_wr2rd;
	u32 tmp1, tmp2, reg;

	DEBUG_MAIN_FULL_C("Max WL Phase: ", dram_info->wl_max_phase, 2);
	DEBUG_MAIN_FULL_C("Min WL Phase: ", dram_info->wl_min_phase, 2);
	DEBUG_MAIN_FULL_C("Max RL Phase: ", dram_info->rl_max_phase, 2);
	DEBUG_MAIN_FULL_C("Min RL Phase: ", dram_info->rl_min_phase, 2);

	if (dram_info->wl_max_phase < 2)
		twr2wr = 0x2;
	else
		twr2wr = 0x3;

	trd2rd = 0x1 + (dram_info->rl_max_phase + 1) / 2 +
		(dram_info->rl_max_phase + 1) % 2;

	tmp1 = (dram_info->rl_max_phase - dram_info->wl_min_phase) / 2 +
		(((dram_info->rl_max_phase - dram_info->wl_min_phase) % 2) >
		 0 ? 1 : 0);
	tmp2 = (dram_info->wl_max_phase - dram_info->rl_min_phase) / 2 +
		((dram_info->wl_max_phase - dram_info->rl_min_phase) % 2 >
		 0 ? 1 : 0);
	trd2wr_wr2rd = (tmp1 >= tmp2) ? tmp1 : tmp2;

	trd2wr_wr2rd += 2;
	trd2rd += 2;
	twr2wr += 2;

	DEBUG_MAIN_FULL_C("WR 2 WR: ", twr2wr, 2);
	DEBUG_MAIN_FULL_C("RD 2 RD: ", trd2rd, 2);
	DEBUG_MAIN_FULL_C("RD 2 WR / WR 2 RD: ", trd2wr_wr2rd, 2);

	reg = reg_read(REG_SDRAM_TIMING_HIGH_ADDR);

	reg &= ~(REG_SDRAM_TIMING_H_W2W_MASK << REG_SDRAM_TIMING_H_W2W_OFFS);
	reg |= ((twr2wr & REG_SDRAM_TIMING_H_W2W_MASK) <<
		REG_SDRAM_TIMING_H_W2W_OFFS);

	reg &= ~(REG_SDRAM_TIMING_H_R2R_MASK << REG_SDRAM_TIMING_H_R2R_OFFS);
	reg &= ~(REG_SDRAM_TIMING_H_R2R_H_MASK <<
		 REG_SDRAM_TIMING_H_R2R_H_OFFS);
	reg |= ((trd2rd & REG_SDRAM_TIMING_H_R2R_MASK) <<
		REG_SDRAM_TIMING_H_R2R_OFFS);
	reg |= (((trd2rd >> 2) & REG_SDRAM_TIMING_H_R2R_H_MASK) <<
		REG_SDRAM_TIMING_H_R2R_H_OFFS);

	reg &= ~(REG_SDRAM_TIMING_H_R2W_W2R_MASK <<
		 REG_SDRAM_TIMING_H_R2W_W2R_OFFS);
	reg &= ~(REG_SDRAM_TIMING_H_R2W_W2R_H_MASK <<
		 REG_SDRAM_TIMING_H_R2W_W2R_H_OFFS);
	reg |= ((trd2wr_wr2rd & REG_SDRAM_TIMING_H_R2W_W2R_MASK) <<
		REG_SDRAM_TIMING_H_R2W_W2R_OFFS);
	reg |= (((trd2wr_wr2rd >> 2) & REG_SDRAM_TIMING_H_R2W_W2R_H_MASK) <<
		REG_SDRAM_TIMING_H_R2W_W2R_H_OFFS);

	reg_write(REG_SDRAM_TIMING_HIGH_ADDR, reg);
}

/*
 * Perform DDR3 PUP Indirect Write
 */
void ddr3_write_pup_reg(u32 mode, u32 cs, u32 pup, u32 phase, u32 delay)
{
	u32 reg = 0;

	if (pup == PUP_BC)
		reg |= (1 << REG_PHY_BC_OFFS);
	else
		reg |= (pup << REG_PHY_PUP_OFFS);

	reg |= ((0x4 * cs + mode) << REG_PHY_CS_OFFS);
	reg |= (phase << REG_PHY_PHASE_OFFS) | delay;

	if (mode == PUP_WL_MODE)
		reg |= ((INIT_WL_DELAY + delay) << REG_PHY_DQS_REF_DLY_OFFS);

	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */
	reg |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */

	do {
		reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
			REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
	} while (reg);	/* Wait for '0' to mark the end of the transaction */

	/* If read Leveling mode - need to write to register 3 separetly */
	if (mode == PUP_RL_MODE) {
		reg = 0;

		if (pup == PUP_BC)
			reg |= (1 << REG_PHY_BC_OFFS);
		else
			reg |= (pup << REG_PHY_PUP_OFFS);

		reg |= ((0x4 * cs + mode + 1) << REG_PHY_CS_OFFS);
		reg |= (INIT_RL_DELAY);

		reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg); /* 0x16A0 */
		reg |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
		reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg); /* 0x16A0 */

		do {
			reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
				REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
		} while (reg);
	}
}

/*
 * Perform DDR3 PUP Indirect Read
 */
u32 ddr3_read_pup_reg(u32 mode, u32 cs, u32 pup)
{
	u32 reg;

	reg = (pup << REG_PHY_PUP_OFFS) |
		((0x4 * cs + mode) << REG_PHY_CS_OFFS);
	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */

	reg |= REG_PHY_REGISTRY_FILE_ACCESS_OP_RD;
	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */

	do {
		reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
			REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
	} while (reg);	/* Wait for '0' to mark the end of the transaction */

	return reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR);	/* 0x16A0 */
}

/*
 * Set training patterns
 */
int ddr3_load_patterns(MV_DRAM_INFO *dram_info, int resume)
{
	u32 reg;

	/* Enable SW override - Required for the ECC Pup */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);

	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = (1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	if (resume == 0) {
#if defined(MV88F78X60) || defined(MV88F672X)
		ddr3_load_pbs_patterns(dram_info);
#endif
		ddr3_load_dqs_patterns(dram_info);
	}

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_1_ADDR) |
		(1 << REG_DRAM_TRAINING_1_TRNBPOINT_OFFS);
	reg_write(REG_DRAM_TRAINING_1_ADDR, reg);

	/* Set Base Addr */
#if defined(MV88F67XX)
	reg_write(REG_DRAM_TRAINING_PATTERN_BASE_ADDR, 0);
#else
	if (resume == 0)
		reg_write(REG_DRAM_TRAINING_PATTERN_BASE_ADDR, 0);
	else
		reg_write(REG_DRAM_TRAINING_PATTERN_BASE_ADDR,
			  RESUME_RL_PATTERNS_ADDR);
#endif

	/* Set Patterns */
	if (resume == 0) {
		reg = (dram_info->cs_ena << REG_DRAM_TRAINING_CS_OFFS) |
			(1 << REG_DRAM_TRAINING_PATTERNS_OFFS);
	} else {
		reg = (0x1 << REG_DRAM_TRAINING_CS_OFFS) |
			(1 << REG_DRAM_TRAINING_PATTERNS_OFFS);
	}

	reg |= (1 << REG_DRAM_TRAINING_AUTO_OFFS);

	reg_write(REG_DRAM_TRAINING_ADDR, reg);

	udelay(100);

	/* Check if Successful */
	if (reg_read(REG_DRAM_TRAINING_ADDR) &
	    (1 << REG_DRAM_TRAINING_ERROR_OFFS))
		return MV_OK;
	else
		return MV_FAIL;
}

#if !defined(MV88F67XX)
/*
 * Name:     ddr3_save_training(MV_DRAM_INFO *dram_info)
 * Desc:     saves the training results to memeory (RL,WL,PBS,Rx/Tx
 *           Centeralization)
 * Args:     MV_DRAM_INFO *dram_info
 * Notes:
 * Returns:  None.
 */
void ddr3_save_training(MV_DRAM_INFO *dram_info)
{
	u32 val, pup, tmp_cs, cs, i, dq;
	u32 crc = 0;
	u32 regs = 0;
	u32 *sdram_offset = (u32 *)RESUME_TRAINING_VALUES_ADDR;
	u32 mode_config[MAX_TRAINING_MODE];

	mode_config[DQS_WR_MODE] = PUP_DQS_WR;
	mode_config[WL_MODE_] = PUP_WL_MODE;
	mode_config[RL_MODE_] = PUP_RL_MODE;
	mode_config[DQS_RD_MODE] = PUP_DQS_RD;
	mode_config[PBS_TX_DM_MODE] = PUP_PBS_TX_DM;
	mode_config[PBS_TX_MODE] = PUP_PBS_TX;
	mode_config[PBS_RX_MODE] = PUP_PBS_RX;

	/* num of training modes */
	for (i = 0; i < MAX_TRAINING_MODE; i++) {
		tmp_cs = dram_info->cs_ena;
		/* num of CS */
		for (cs = 0; cs < MAX_CS; cs++) {
			if (tmp_cs & (1 << cs)) {
				/* num of PUPs */
				for (pup = 0; pup < dram_info->num_of_total_pups;
				     pup++) {
					if (pup == dram_info->num_of_std_pups &&
					    dram_info->ecc_ena)
						pup = ECC_PUP;
					if (i == PBS_TX_DM_MODE) {
						/*
						 * Change CS bitmask because
						 * PBS works only with CS0
						 */
						tmp_cs = 0x1;
						val = ddr3_read_pup_reg(
							mode_config[i], CS0, pup);
					} else if (i == PBS_TX_MODE ||
						   i == PBS_RX_MODE) {
						/*
						 * Change CS bitmask because
						 * PBS works only with CS0
						 */
						tmp_cs = 0x1;
						for (dq = 0; dq <= DQ_NUM;
						     dq++) {
							val = ddr3_read_pup_reg(
								mode_config[i] + dq,
								CS0,
								pup);
							(*sdram_offset) = val;
							crc += *sdram_offset;
							sdram_offset++;
							regs++;
						}
						continue;
					} else {
						val = ddr3_read_pup_reg(
							mode_config[i], cs, pup);
					}

					*sdram_offset = val;
					crc += *sdram_offset;
					sdram_offset++;
					regs++;
				}
			}
		}
	}

	*sdram_offset = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
	crc += *sdram_offset;
	sdram_offset++;
	regs++;
	*sdram_offset = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
	crc += *sdram_offset;
	sdram_offset++;
	regs++;
	sdram_offset = (u32 *)NUM_OF_REGISTER_ADDR;
	*sdram_offset = regs;
	DEBUG_SUSPEND_RESUME_S("Training Results CheckSum write= ");
	DEBUG_SUSPEND_RESUME_D(crc, 8);
	DEBUG_SUSPEND_RESUME_S("\n");
	sdram_offset = (u32 *)CHECKSUM_RESULT_ADDR;
	*sdram_offset = crc;
}

/*
 * Name:     ddr3_read_training_results()
 * Desc:     Reads the training results from memeory (RL,WL,PBS,Rx/Tx
 *           Centeralization)
 *           and writes them to the relevant registers
 * Args:     MV_DRAM_INFO *dram_info
 * Notes:
 * Returns:  None.
 */
int ddr3_read_training_results(void)
{
	u32 val, reg, idx, dqs_wr_idx = 0, crc = 0;
	u32 *sdram_offset = (u32 *)RESUME_TRAINING_VALUES_ADDR;
	u32 training_val[RESUME_TRAINING_VALUES_MAX] = { 0 };
	u32 regs = *((u32 *)NUM_OF_REGISTER_ADDR);

	/*
	 * Read Training results & Dunit registers from memory and write
	 * it to an array
	 */
	for (idx = 0; idx < regs; idx++) {
		training_val[idx] = *sdram_offset;
		crc += *sdram_offset;
		sdram_offset++;
	}

	sdram_offset = (u32 *)CHECKSUM_RESULT_ADDR;

	if ((*sdram_offset) == crc) {
		DEBUG_SUSPEND_RESUME_S("Training Results CheckSum read PASS= ");
		DEBUG_SUSPEND_RESUME_D(crc, 8);
		DEBUG_SUSPEND_RESUME_S("\n");
	} else {
		DEBUG_MAIN_S("Wrong Training Results CheckSum\n");
		return MV_FAIL;
	}

	/*
	 * We iterate through all the registers except for the last 2 since
	 * they are Dunit registers (and not PHY registers)
	 */
	for (idx = 0; idx < (regs - 2); idx++) {
		val = training_val[idx];
		reg = (val >> REG_PHY_CS_OFFS) & 0x3F; /*read the phy address */

		/* Check if the values belongs to the DQS WR */
		if (reg == PUP_WL_MODE) {
			/* bit[5:0] in DQS_WR are delay */
			val = (training_val[dqs_wr_idx++] & 0x3F);
			/*
			 * bit[15:10] are DQS_WR delay & bit[9:0] are
			 * WL phase & delay
			 */
			val = (val << REG_PHY_DQS_REF_DLY_OFFS) |
				(training_val[idx] & 0x3C003FF);
			/* Add Request pending and write operation bits */
			val |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
		} else if (reg == PUP_DQS_WR) {
			/*
			 * Do nothing since DQS_WR will be done in PUP_WL_MODE
			 */
			continue;
		}

		val |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
		reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, val);
		do {
			val = (reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR)) &
				REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
		} while (val);	/* Wait for '0' to mark the end of the transaction */
	}

	/* write last 2 Dunit configurations */
	val = training_val[idx];
	reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR, val);	/* reg 0x1538 */
	val = training_val[idx + 1];
	reg_write(REG_READ_DATA_READY_DELAYS_ADDR, val);	/* reg 0x153c */

	return MV_OK;
}

/*
 * Name:     ddr3_check_if_resume_mode()
 * Desc:     Reads the address (0x3000) of the Resume Magic word (0xDEADB002)
 * Args:     MV_DRAM_INFO *dram_info
 * Notes:
 * Returns:  return (magic_word == SUSPEND_MAGIC_WORD)
 */
int ddr3_check_if_resume_mode(MV_DRAM_INFO *dram_info, u32 freq)
{
	u32 magic_word;
	u32 *sdram_offset = (u32 *)BOOT_INFO_ADDR;

	if (dram_info->reg_dimm != 1) {
		/*
		 * Perform write levleling in order initiate the phy with
		 * low frequency
		 */
		if (MV_OK != ddr3_write_leveling_hw(freq, dram_info)) {
			DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Write Leveling Hw)\n");
			return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
		}
	}

	if (MV_OK != ddr3_load_patterns(dram_info, 1)) {
		DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Loading Patterns)\n");
		return MV_DDR3_TRAINING_ERR_LOAD_PATTERNS;
	}

	/* Enable CS0 only for RL */
	dram_info->cs_ena = 0x1;

	/* Perform Read levleling in order to get stable memory */
	if (MV_OK != ddr3_read_leveling_hw(freq, dram_info)) {
		DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Read Leveling Hw)\n");
		return MV_DDR3_TRAINING_ERR_WR_LVL_HW;
	}

	/* Back to relevant CS */
	dram_info->cs_ena = ddr3_get_cs_ena_from_reg();

	magic_word = *sdram_offset;
	return magic_word == SUSPEND_MAGIC_WORD;
}

/*
 * Name:     ddr3_training_suspend_resume()
 * Desc:     Execute the Resume state
 * Args:     MV_DRAM_INFO *dram_info
 * Notes:
 * Returns:  return (magic_word == SUSPEND_MAGIC_WORD)
 */
int ddr3_training_suspend_resume(MV_DRAM_INFO *dram_info)
{
	u32 freq, reg;
	int tmp_ratio;

	/* Configure DDR */
	if (MV_OK != ddr3_read_training_results())
		return MV_FAIL;

	/* Reset read FIFO */
	reg = reg_read(REG_DRAM_TRAINING_ADDR);

	/* Start Auto Read Leveling procedure */
	reg |= (1 << REG_DRAM_TRAINING_RL_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg |= ((1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS) +
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS));

	/* [0] = 1 - Enable SW override, [4] = 1 - FIFO reset  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	udelay(2);

	reg = reg_read(REG_DRAM_TRAINING_ADDR);
	/* Clear Auto Read Leveling procedure */
	reg &= ~(1 << REG_DRAM_TRAINING_RL_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	/* Return to target frequency */
	freq = dram_info->target_frequency;
	tmp_ratio = 1;
	if (MV_OK != ddr3_dfs_low_2_high(freq, tmp_ratio, dram_info)) {
		DEBUG_MAIN_S("DDR3 Training Sequence - FAILED (Dfs Low2High)\n");
		return MV_DDR3_TRAINING_ERR_DFS_H2L;
	}

	if (dram_info->ecc_ena) {
		/* Scabbling the RL area pattern and the training area */
		mv_sys_xor_finish();
		dram_info->num_cs = 1;
		dram_info->cs_ena = 1;
		mv_sys_xor_init(dram_info);
		mv_xor_mem_init(0, RESUME_RL_PATTERNS_ADDR,
				RESUME_RL_PATTERNS_SIZE, 0xFFFFFFFF, 0xFFFFFFFF);

		/* Wait for previous transfer completion */

		while (mv_xor_state_get(0) != MV_IDLE)
			;

		/* Return XOR State */
		mv_sys_xor_finish();
	}

	return MV_OK;
}
#endif

void ddr3_print_freq(u32 freq)
{
	u32 tmp_freq;

	switch (freq) {
	case 0:
		tmp_freq = 100;
		break;
	case 1:
		tmp_freq = 300;
		break;
	case 2:
		tmp_freq = 360;
		break;
	case 3:
		tmp_freq = 400;
		break;
	case 4:
		tmp_freq = 444;
		break;
	case 5:
		tmp_freq = 500;
		break;
	case 6:
		tmp_freq = 533;
		break;
	case 7:
		tmp_freq = 600;
		break;
	case 8:
		tmp_freq = 666;
		break;
	case 9:
		tmp_freq = 720;
		break;
	case 10:
		tmp_freq = 800;
		break;
	default:
		tmp_freq = 100;
	}

	printf("Current frequency is: %dMHz\n", tmp_freq);
}

int ddr3_get_min_max_read_sample_delay(u32 cs_enable, u32 reg, u32 *min,
				       u32 *max, u32 *cs_max)
{
	u32 cs, delay;

	*min = 0xFFFFFFFF;
	*max = 0x0;

	for (cs = 0; cs < MAX_CS; cs++) {
		if ((cs_enable & (1 << cs)) == 0)
			continue;

		delay = ((reg >> (cs * 8)) & 0x1F);

		if (delay < *min)
			*min = delay;

		if (delay > *max) {
			*max = delay;
			*cs_max = cs;
		}
	}

	return MV_OK;
}

int ddr3_get_min_max_rl_phase(MV_DRAM_INFO *dram_info, u32 *min, u32 *max,
			      u32 cs)
{
	u32 pup, reg, phase;

	*min = 0xFFFFFFFF;
	*max = 0x0;

	for (pup = 0; pup < dram_info->num_of_total_pups; pup++) {
		reg = ddr3_read_pup_reg(PUP_RL_MODE, cs, pup);
		phase = ((reg >> 8) & 0x7);

		if (phase < *min)
			*min = phase;

		if (phase > *max)
			*max = phase;
	}

	return MV_OK;
}

int ddr3_odt_activate(int activate)
{
	u32 reg, mask;

	mask = (1 << REG_DUNIT_ODT_CTRL_OVRD_OFFS) |
		(1 << REG_DUNIT_ODT_CTRL_OVRD_VAL_OFFS);
	/* {0x0000149C}  -   DDR Dunit ODT Control Register */
	reg = reg_read(REG_DUNIT_ODT_CTRL_ADDR);
	if (activate)
		reg |= mask;
	else
		reg &= ~mask;

	reg_write(REG_DUNIT_ODT_CTRL_ADDR, reg);

	return MV_OK;
}

int ddr3_odt_read_dynamic_config(MV_DRAM_INFO *dram_info)
{
	u32 min_read_sample_delay, max_read_sample_delay, max_rl_phase;
	u32 min, max, cs_max;
	u32 cs_ena, reg;

	reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
	cs_ena = ddr3_get_cs_ena_from_reg();

	/* Get minimum and maximum of read sample delay of all CS */
	ddr3_get_min_max_read_sample_delay(cs_ena, reg, &min_read_sample_delay,
					   &max_read_sample_delay, &cs_max);

	/*
	 * Get minimum and maximum read leveling phase which belongs to the
	 * maximal read sample delay
	 */
	ddr3_get_min_max_rl_phase(dram_info, &min, &max, cs_max);
	max_rl_phase = max;

	/* DDR ODT Timing (Low) Register calculation */
	reg = reg_read(REG_ODT_TIME_LOW_ADDR);
	reg &= ~(0x1FF << REG_ODT_ON_CTL_RD_OFFS);
	reg |= (((min_read_sample_delay - 1) & 0xF) << REG_ODT_ON_CTL_RD_OFFS);
	reg |= (((max_read_sample_delay + 4 + (((max_rl_phase + 1) / 2) + 1)) &
		 0x1F) << REG_ODT_OFF_CTL_RD_OFFS);
	reg_write(REG_ODT_TIME_LOW_ADDR, reg);

	return MV_OK;
}
