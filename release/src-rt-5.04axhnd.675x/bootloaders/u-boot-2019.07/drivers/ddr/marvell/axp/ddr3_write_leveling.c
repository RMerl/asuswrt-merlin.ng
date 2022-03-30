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
#define DEBUG_WL_C(s, d, l) \
	DEBUG_WL_S(s); DEBUG_WL_D(d, l); DEBUG_WL_S("\n")
#define DEBUG_WL_FULL_C(s, d, l) \
	DEBUG_WL_FULL_S(s); DEBUG_WL_FULL_D(d, l); DEBUG_WL_FULL_S("\n")

#ifdef MV_DEBUG_WL
#define DEBUG_WL_S(s)			puts(s)
#define DEBUG_WL_D(d, l)		printf("%x", d)
#define DEBUG_RL_S(s) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%s", s)
#define DEBUG_RL_D(d, l) \
	debug_cond(ddr3_get_log_level() >= MV_LOG_LEVEL_2, "%x", d)
#else
#define DEBUG_WL_S(s)
#define DEBUG_WL_D(d, l)
#endif

#ifdef MV_DEBUG_WL_FULL
#define DEBUG_WL_FULL_S(s)		puts(s)
#define DEBUG_WL_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_WL_FULL_S(s)
#define DEBUG_WL_FULL_D(d, l)
#endif

#define WL_SUP_EXPECTED_DATA		0x21
#define WL_SUP_READ_DRAM_ENTRY		0x8

static int ddr3_write_leveling_single_cs(u32 cs, u32 freq, int ratio_2to1,
					 u32 *result,
					 MV_DRAM_INFO *dram_info);
static void ddr3_write_ctrl_pup_reg(int bc_acc, u32 pup, u32 reg_addr,
				    u32 data);

extern u16 odt_static[ODT_OPT][MAX_CS];
extern u16 odt_dynamic[ODT_OPT][MAX_CS];
extern u32 wl_sup_pattern[LEN_WL_SUP_PATTERN];

/*
 * Name:     ddr3_write_leveling_hw
 * Desc:     Execute Write leveling phase by HW
 * Args:     freq      - current sequence frequency
 *           dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_write_leveling_hw(u32 freq, MV_DRAM_INFO *dram_info)
{
	u32 reg, phase, delay, cs, pup;
#ifdef MV88F67XX
	int dpde_flag = 0;
#endif
	/* Debug message - Start Read leveling procedure */
	DEBUG_WL_S("DDR3 - Write Leveling - Starting HW WL procedure\n");

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	if (reg & (1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS)) {
		dpde_flag = 1;
		reg_write(REG_DUNIT_CTRL_LOW_ADDR,
			  reg & ~(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS));
	}
#endif

	reg = 1 << REG_DRAM_TRAINING_WL_OFFS;
	/* Config the retest number */
	reg |= (COUNT_HW_WL << REG_DRAM_TRAINING_RETEST_OFFS);
	reg |= (dram_info->cs_ena << (REG_DRAM_TRAINING_CS_OFFS));
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	reg =  reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) |
		(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_SHADOW_ADDR, reg);

	/* Wait */
	do {
		reg = reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) &
			(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	} while (reg);		/* Wait for '0' */

	reg = reg_read(REG_DRAM_TRAINING_ADDR);
	/* Check if Successful */
	if (reg & (1 << REG_DRAM_TRAINING_ERROR_OFFS)) {
		/*
		 * Read results to arrays - Results are required for WL
		 * High freq Supplement and DQS Centralization
		 */
		for (cs = 0; cs < MAX_CS; cs++) {
			if (dram_info->cs_ena & (1 << cs)) {
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					if (pup == dram_info->num_of_std_pups
					    && dram_info->ecc_ena)
						pup = ECC_PUP;
					reg =
					    ddr3_read_pup_reg(PUP_WL_MODE, cs,
							      pup);
					phase =
					    (reg >> REG_PHY_PHASE_OFFS) &
					    PUP_PHASE_MASK;
					delay = reg & PUP_DELAY_MASK;
					dram_info->wl_val[cs][pup][P] = phase;
					dram_info->wl_val[cs][pup][D] = delay;
					dram_info->wl_val[cs][pup][S] =
					    WL_HI_FREQ_STATE - 1;
					reg =
					    ddr3_read_pup_reg(PUP_WL_MODE + 0x1,
							      cs, pup);
					dram_info->wl_val[cs][pup][DQS] =
					    (reg & 0x3F);
				}

#ifdef MV_DEBUG_WL
				/* Debug message - Print res for cs[i]: cs,PUP,Phase,Delay */
				DEBUG_WL_S("DDR3 - Write Leveling - Write Leveling Cs - ");
				DEBUG_WL_D((u32) cs, 1);
				DEBUG_WL_S(" Results:\n");
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					if (pup == dram_info->num_of_std_pups
					    && dram_info->ecc_ena)
						pup = ECC_PUP;
					DEBUG_WL_S("DDR3 - Write Leveling - PUP: ");
					DEBUG_WL_D((u32) pup, 1);
					DEBUG_WL_S(", Phase: ");
					DEBUG_WL_D((u32)
						   dram_info->wl_val[cs][pup]
						   [P], 1);
					DEBUG_WL_S(", Delay: ");
					DEBUG_WL_D((u32)
						   dram_info->wl_val[cs][pup]
						   [D], 2);
					DEBUG_WL_S("\n");
				}
#endif
			}
		}

		/* Dynamic pad issue (BTS669) during WL */
#ifdef MV88F67XX
		if (dpde_flag) {
			reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR) |
				(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS);
			reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);
		}
#endif

		DEBUG_WL_S("DDR3 - Write Leveling - HW WL Ended Successfully\n");

		return MV_OK;
	} else {
		DEBUG_WL_S("DDR3 - Write Leveling - HW WL Error\n");
		return MV_FAIL;
	}
}

/*
 * Name:     ddr3_wl_supplement
 * Desc:     Write Leveling Supplement
 * Args:     dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_wl_supplement(MV_DRAM_INFO *dram_info)
{
	u32 cs, cnt, pup_num, sum, phase, delay, max_pup_num, pup, sdram_offset;
	u32 tmp_count, ecc, reg;
	u32 ddr_width, tmp_pup, idx;
	u32 sdram_pup_val, uj;
	u32 one_clk_err = 0, align_err = 0, no_err = 0, err = 0, err_n = 0;
	u32 sdram_data[LEN_WL_SUP_PATTERN] __aligned(32) = { 0 };

	ddr_width = dram_info->ddr_width;
	no_err = 0;

	DEBUG_WL_S("DDR3 - Write Leveling Hi-Freq Supplement - Starting\n");

	switch (ddr_width) {
		/* Data error from pos-adge to pos-adge */
	case 16:
		one_clk_err = 4;
		align_err = 4;
		break;
	case 32:
		one_clk_err = 8;
		align_err = 8;
		break;
	case 64:
		one_clk_err = 0x10;
		align_err = 0x10;
		break;
	default:
		DEBUG_WL_S("Error - bus width!!!\n");
		return MV_FAIL;
	}

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);

	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_WL_S("DDR3 - Write Leveling Hi-Freq Supplement - SW Override Enabled\n");
	reg = (1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */
	tmp_count = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			sum = 0;
			/*
			 * 2 iterations loop: 1)actual WL results 2) fix WL
			 * if needed
			 */
			for (cnt = 0; cnt < COUNT_WL_HI_FREQ; cnt++) {
				DEBUG_WL_C("COUNT = ", cnt, 1);
				for (ecc = 0; ecc < (dram_info->ecc_ena + 1);
				     ecc++) {
					if (ecc) {
						DEBUG_WL_S("ECC PUP:\n");
					} else {
						DEBUG_WL_S("DATA PUP:\n");
					}

					max_pup_num =
					    dram_info->num_of_std_pups * (1 -
									  ecc) +
					    ecc;
					/* ECC Support - Switch ECC Mux on ecc=1 */
					reg =
					    (reg_read(REG_DRAM_TRAINING_2_ADDR)
					     & ~(1 <<
						 REG_DRAM_TRAINING_2_ECC_MUX_OFFS));
					reg |=
					    (dram_info->ecc_ena *
					     ecc <<
					     REG_DRAM_TRAINING_2_ECC_MUX_OFFS);
					reg_write(REG_DRAM_TRAINING_2_ADDR,
						  reg);
					ddr3_reset_phy_read_fifo();

					/* Write to memory */
					sdram_offset =
					    tmp_count * (SDRAM_CS_SIZE + 1) +
					    0x200;
					if (MV_OK != ddr3_dram_sram_burst((u32)
									  wl_sup_pattern,
									  sdram_offset,
									  LEN_WL_SUP_PATTERN))
						return MV_FAIL;

					/* Read from memory */
					if (MV_OK !=
					    ddr3_dram_sram_burst(sdram_offset,
								 (u32)
								 sdram_data,
								 LEN_WL_SUP_PATTERN))
						return MV_FAIL;

					/* Print the buffer */
					for (uj = 0; uj < LEN_WL_SUP_PATTERN;
					     uj++) {
						if ((uj % 4 == 0) && (uj != 0)) {
							DEBUG_WL_S("\n");
						}
						DEBUG_WL_D(sdram_data[uj],
							   8);
						DEBUG_WL_S(" ");
					}

					/* Check pup which DQS/DATA is error */
					for (pup = 0; pup < max_pup_num; pup++) {
						/* ECC support - bit 8 */
						pup_num = (ecc) ? ECC_PUP : pup;
						if (pup < 4) {	/* lower 32 bit */
							tmp_pup = pup;
							idx =
							    WL_SUP_READ_DRAM_ENTRY;
						} else {	/* higher 32 bit */
							tmp_pup = pup - 4;
							idx =
							    WL_SUP_READ_DRAM_ENTRY
							    + 1;
						}
						DEBUG_WL_S("\nCS: ");
						DEBUG_WL_D((u32) cs, 1);
						DEBUG_WL_S(" PUP: ");
						DEBUG_WL_D((u32) pup_num, 1);
						DEBUG_WL_S("\n");
						sdram_pup_val =
						    ((sdram_data[idx] >>
						      ((tmp_pup) * 8)) & 0xFF);
						DEBUG_WL_C("Actual Data = ",
							   sdram_pup_val, 2);
						DEBUG_WL_C("Expected Data = ",
							   (WL_SUP_EXPECTED_DATA
							    + pup), 2);
						/*
						 * ALINGHMENT: calculate
						 * expected data vs actual data
						 */
						err =
						    (WL_SUP_EXPECTED_DATA +
						     pup) - sdram_pup_val;
						/*
						 * CLOCK LONG: calculate
						 * expected data vs actual data
						 */
						err_n =
						    sdram_pup_val -
						    (WL_SUP_EXPECTED_DATA +
						     pup);
						DEBUG_WL_C("err = ", err, 2);
						DEBUG_WL_C("err_n = ", err_n,
							   2);
						if (err == no_err) {
							/* PUP is correct - increment State */
							dram_info->wl_val[cs]
							    [pup_num]
							    [S] = 1;
						} else if (err_n == one_clk_err) {
							/* clock is longer than DQS */
							phase =
							    ((dram_info->wl_val
							      [cs]
							      [pup_num][P] +
							      WL_HI_FREQ_SHIFT)
							     % MAX_PHASE_2TO1);
							dram_info->wl_val[cs]
							    [pup_num]
							    [P] = phase;
							delay =
							    dram_info->wl_val
							    [cs][pup_num]
							    [D];
							DEBUG_WL_S("#### Clock is longer than DQS more than one clk cycle ####\n");
							ddr3_write_pup_reg
							    (PUP_WL_MODE, cs,
							     pup * (1 - ecc) +
							     ECC_PUP * ecc,
							     phase, delay);
						} else if (err == align_err) {
							/* clock is align to DQS */
							phase =
							    dram_info->wl_val
							    [cs][pup_num]
							    [P];
							delay =
							    dram_info->wl_val
							    [cs][pup_num]
							    [D];
							DEBUG_WL_S("#### Alignment PUPS problem ####\n");
							if ((phase == 0)
							    || ((phase == 1)
								&& (delay <=
								    0x10))) {
								DEBUG_WL_S("#### Warning - Possible Layout Violation (DQS is longer than CLK)####\n");
							}

							phase = 0x0;
							delay = 0x0;
							dram_info->wl_val[cs]
							    [pup_num]
							    [P] = phase;
							dram_info->wl_val[cs]
							    [pup_num]
							    [D] = delay;
							ddr3_write_pup_reg
							    (PUP_WL_MODE, cs,
							     pup * (1 - ecc) +
							     ECC_PUP * ecc,
							     phase, delay);
						}
						/* Stop condition for ECC phase */
						pup = (ecc) ? max_pup_num : pup;
					}

					/* ECC Support - Disable ECC MUX */
					reg =
					    (reg_read(REG_DRAM_TRAINING_2_ADDR)
					     & ~(1 <<
						 REG_DRAM_TRAINING_2_ECC_MUX_OFFS));
					reg_write(REG_DRAM_TRAINING_2_ADDR,
						  reg);
				}
			}

			for (pup = 0; pup < dram_info->num_of_std_pups; pup++)
				sum += dram_info->wl_val[cs][pup][S];

			if (dram_info->ecc_ena)
				sum += dram_info->wl_val[cs][ECC_PUP][S];

			/* Checks if any pup is not locked after the change */
			if (sum < (WL_HI_FREQ_STATE * (dram_info->num_of_total_pups))) {
				DEBUG_WL_C("DDR3 - Write Leveling Hi-Freq Supplement - didn't work for Cs - ",
					   (u32) cs, 1);
				return MV_FAIL;
			}
			tmp_count++;
		}
	}

	dram_info->wl_max_phase = 0;
	dram_info->wl_min_phase = 10;

	/*
	 * Read results to arrays - Results are required for DQS Centralization
	 */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			for (pup = 0; pup < dram_info->num_of_total_pups; pup++) {
				if (pup == dram_info->num_of_std_pups
				    && dram_info->ecc_ena)
					pup = ECC_PUP;
				reg = ddr3_read_pup_reg(PUP_WL_MODE, cs, pup);
				phase =
				    (reg >> REG_PHY_PHASE_OFFS) &
				    PUP_PHASE_MASK;
				if (phase > dram_info->wl_max_phase)
					dram_info->wl_max_phase = phase;
				if (phase < dram_info->wl_min_phase)
					dram_info->wl_min_phase = phase;
			}
		}
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

	DEBUG_WL_S("DDR3 - Write Leveling Hi-Freq Supplement - Ended Successfully\n");

	return MV_OK;
}

/*
 * Name:     ddr3_write_leveling_hw_reg_dimm
 * Desc:     Execute Write leveling phase by HW
 * Args:     freq      - current sequence frequency
 *           dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_write_leveling_hw_reg_dimm(u32 freq, MV_DRAM_INFO *dram_info)
{
	u32 reg, phase, delay, cs, pup, pup_num;
	__maybe_unused int dpde_flag = 0;

	/* Debug message - Start Read leveling procedure */
	DEBUG_WL_S("DDR3 - Write Leveling - Starting HW WL procedure\n");

	if (dram_info->num_cs > 2) {
		DEBUG_WL_S("DDR3 - Write Leveling - HW WL Ended Successfully\n");
		return MV_NO_CHANGE;
	}

	/* If target freq = 400 move clock start point */
	/* Write to control PUP to Control Deskew Regs */
	if (freq <= DDR_400) {
		for (pup = 0; pup <= dram_info->num_of_total_pups; pup++) {
			/* PUP_DELAY_MASK 0x1F */
			/* reg = 0x0C10001F + (uj << 16); */
			ddr3_write_ctrl_pup_reg(1, pup, CNTRL_PUP_DESKEW + pup,
						0x1F);
		}
	}

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	if (reg & (1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS)) {
		dpde_flag = 1;
		reg_write(REG_DUNIT_CTRL_LOW_ADDR,
			  reg & ~(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS));
	}
#endif

	reg = (1 << REG_DRAM_TRAINING_WL_OFFS);
	/* Config the retest number */
	reg |= (COUNT_HW_WL << REG_DRAM_TRAINING_RETEST_OFFS);
	reg |= (dram_info->cs_ena << (REG_DRAM_TRAINING_CS_OFFS));
	reg_write(REG_DRAM_TRAINING_ADDR, reg);	/* 0x15B0 - Training Register */

	reg = reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) |
		(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	reg_write(REG_DRAM_TRAINING_SHADOW_ADDR, reg);

	/* Wait */
	do {
		reg = reg_read(REG_DRAM_TRAINING_SHADOW_ADDR) &
			(1 << REG_DRAM_TRAINING_AUTO_OFFS);
	} while (reg);		/* Wait for '0' */

	reg = reg_read(REG_DRAM_TRAINING_ADDR);
	/* Check if Successful */
	if (reg & (1 << REG_DRAM_TRAINING_ERROR_OFFS)) {
		/*
		 * Read results to arrays - Results are required for WL High
		 * freq Supplement and DQS Centralization
		 */
		for (cs = 0; cs < MAX_CS; cs++) {
			if (dram_info->cs_ena & (1 << cs)) {
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					if (pup == dram_info->num_of_std_pups
					    && dram_info->ecc_ena)
						pup = ECC_BIT;
					reg =
					    ddr3_read_pup_reg(PUP_WL_MODE, cs,
							      pup);
					phase =
					    (reg >> REG_PHY_PHASE_OFFS) &
					    PUP_PHASE_MASK;
					delay = reg & PUP_DELAY_MASK;
					dram_info->wl_val[cs][pup][P] = phase;
					dram_info->wl_val[cs][pup][D] = delay;
					if ((phase == 1) && (delay >= 0x1D)) {
						/*
						 * Need to do it here for
						 * uncorrect WL values
						 */
						ddr3_write_pup_reg(PUP_WL_MODE,
								   cs, pup, 0,
								   0);
						dram_info->wl_val[cs][pup][P] =
						    0;
						dram_info->wl_val[cs][pup][D] =
						    0;
					}
					dram_info->wl_val[cs][pup][S] =
					    WL_HI_FREQ_STATE - 1;
					reg =
					    ddr3_read_pup_reg(PUP_WL_MODE + 0x1,
							      cs, pup);
					dram_info->wl_val[cs][pup][DQS] =
					    (reg & 0x3F);
				}
#ifdef MV_DEBUG_WL
				/*
				 * Debug message - Print res for cs[i]:
				 * cs,PUP,Phase,Delay
				 */
				DEBUG_WL_S("DDR3 - Write Leveling - Write Leveling Cs - ");
				DEBUG_WL_D((u32) cs, 1);
				DEBUG_WL_S(" Results:\n");
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					DEBUG_WL_S
					    ("DDR3 - Write Leveling - PUP: ");
					DEBUG_WL_D((u32) pup, 1);
					DEBUG_WL_S(", Phase: ");
					DEBUG_WL_D((u32)
						   dram_info->wl_val[cs][pup]
						   [P], 1);
					DEBUG_WL_S(", Delay: ");
					DEBUG_WL_D((u32)
						   dram_info->wl_val[cs][pup]
						   [D], 2);
					DEBUG_WL_S("\n");
				}
#endif
			}
		}

#ifdef MV88F67XX
		/* Dynamic pad issue (BTS669) during WL */
		if (dpde_flag) {
			reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR) |
				(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS);
			reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);
		}
#endif
		DEBUG_WL_S("DDR3 - Write Leveling - HW WL Ended Successfully\n");

		/* If target freq = 400 move clock back */
		/* Write to control PUP to Control Deskew Regs */
		if (freq <= DDR_400) {
			for (pup = 0; pup <= dram_info->num_of_total_pups;
			     pup++) {
				ddr3_write_ctrl_pup_reg(1, pup,
							CNTRL_PUP_DESKEW + pup, 0);
			}
		}

		return MV_OK;
	} else {
		/* Configure Each PUP with locked leveling settings */
		for (cs = 0; cs < MAX_CS; cs++) {
			if (dram_info->cs_ena & (1 << cs)) {
				for (pup = 0;
				     pup < dram_info->num_of_total_pups;
				     pup++) {
					/* ECC support - bit 8 */
					pup_num = (pup == dram_info->num_of_std_pups) ?
						ECC_BIT : pup;
					ddr3_write_pup_reg(PUP_WL_MODE, cs,
							   pup_num, 0, 0);
				}
			}
		}

		reg_write(REG_DRAM_TRAINING_ADDR, 0);

		/* If target freq = 400 move clock back */
		/* Write to control PUP to Control Deskew Regs */
		if (freq <= DDR_400) {
			for (pup = 0; pup <= dram_info->num_of_total_pups;
			     pup++) {
				ddr3_write_ctrl_pup_reg(1, pup,
							CNTRL_PUP_DESKEW + pup, 0);
			}
		}

		DEBUG_WL_S("DDR3 - Write Leveling - HW WL Ended Successfully\n");
		return MV_NO_CHANGE;
	}
}

/*
 * Name:     ddr3_write_leveling_sw
 * Desc:     Execute Write leveling phase by SW
 * Args:     freq      - current sequence frequency
 *           dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_write_leveling_sw(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info)
{
	u32 reg, cs, cnt, pup, max_pup_num;
	u32 res[MAX_CS];
	max_pup_num = dram_info->num_of_total_pups;
	__maybe_unused int dpde_flag = 0;

	/* Debug message - Start Write leveling procedure */
	DEBUG_WL_S("DDR3 - Write Leveling - Starting SW WL procedure\n");

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	if (reg & (1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS)) {
		dpde_flag = 1;
		reg_write(REG_DUNIT_CTRL_LOW_ADDR,
			  reg & ~(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS));
	}
#endif

	/* Set Output buffer-off to all CS and correct ODT values */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_ODT_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];
			reg |= (1 << REG_DDR3_MR1_OUTBUF_DIS_OFFS);

			/* 0x15D0 - DDR3 MR0 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

	DEBUG_WL_FULL_S("DDR3 - Write Leveling - Qoff and RTT Values are set for all Cs\n");

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_WL_FULL_S("DDR3 - Write Leveling - SW Override Enabled\n");

	/* Enable PHY write leveling mode */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_WL_MODE_OFFS);
	/* [2] = 0 - TrnWLMode - Enable */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	/* Reset WL results arry */
	memset(dram_info->wl_val, 0, sizeof(u32) * MAX_CS * MAX_PUP_NUM * 7);

	/* Loop for each cs */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - Starting working with Cs - ",
					(u32) cs, 1);
			/* Refresh X9 current cs */
			DEBUG_WL_FULL_S("DDR3 - Write Leveling - Refresh X9\n");
			for (cnt = 0; cnt < COUNT_WL_RFRS; cnt++) {
				reg =
				    REG_SDRAM_OPERATION_CMD_RFRS & ~(1 <<
								     (REG_SDRAM_OPERATION_CS_OFFS
								      + cs));
				/* [3-0] = 0x2 - refresh, [11-8] - enable current cs */
				reg_write(REG_SDRAM_OPERATION_ADDR, reg);	/* 0x1418 - SDRAM Operation Register */

				do {
					reg =
					    ((reg_read
					      (REG_SDRAM_OPERATION_ADDR)) &
					     REG_SDRAM_OPERATION_CMD_RFRS_DONE);
				} while (reg);	/* Wait for '0' */
			}

			/* Configure MR1 in Cs[CsNum] - write leveling on, output buffer on */
			DEBUG_WL_FULL_S("DDR3 - Write Leveling - Configure MR1 for current Cs: WL-on,OB-on\n");
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_OUTBUF_WL_MASK;
			/* Set ODT Values */
			reg &= REG_DDR3_MR1_ODT_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];
			/* Enable WL MODE */
			reg |= (1 << REG_DDR3_MR1_WL_ENA_OFFS);
			/* [7]=1, [12]=0 - Output Buffer and write leveling enabled */
			reg_write(REG_DDR3_MR1_ADDR, reg);	/* 0x15D4 - DDR3 MR1 Register */
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);

			/* Write leveling  cs[cs] */
			if (MV_OK !=
			    ddr3_write_leveling_single_cs(cs, freq, ratio_2to1,
							  (u32 *)(res + cs),
							  dram_info)) {
				DEBUG_WL_FULL_C("DDR3 - Write Leveling single Cs - FAILED -  Cs - ",
						(u32) cs, 1);
				for (pup = 0; pup < max_pup_num; pup++) {
					if (((res[cs] >> pup) & 0x1) == 0) {
						DEBUG_WL_C("Failed Byte : ",
							   pup, 1);
					}
				}
				return MV_FAIL;
			}

			/* Set TrnWLDeUpd - After each CS is done */
			reg = reg_read(REG_TRAINING_WL_ADDR) |
				(1 << REG_TRAINING_WL_CS_DONE_OFFS);
			/* 0x16AC - Training Write leveling register */
			reg_write(REG_TRAINING_WL_ADDR, reg);

			/*
			 * Debug message - Finished Write leveling cs[cs] -
			 * each PUP Fail/Success
			 */
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - Finished Cs - ", (u32) cs,
					1);
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - The Results: 1-PUP locked, 0-PUP failed -",
					(u32) res[cs], 3);

			/*
			 * Configure MR1 in cs[cs] - write leveling off (0),
			 * output buffer off (1)
			 */
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_OUTBUF_WL_MASK;
			reg |= (1 << REG_DDR3_MR1_OUTBUF_DIS_OFFS);
			/* No need to sort ODT since it is same CS */
			/* 0x15D4 - DDR3 MR1 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

	/* Disable WL Mode */
	/* [2]=1 - TrnWLMode - Disable */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg |= (1 << REG_DRAM_TRAINING_2_WL_MODE_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Set Output buffer-on to all CS and correct ODT values */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_ODT_MASK;
			reg &= REG_DDR3_MR1_OUTBUF_WL_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];

			/* 0x15D0 - DDR3 MR1 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	if (dpde_flag) {
		reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR) |
			(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS);
		reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);
	}
#endif
	DEBUG_WL_FULL_S("DDR3 - Write Leveling - Finished WL procedure for all Cs\n");

	return MV_OK;
}

#if !defined(MV88F672X)
/*
 * Name:     ddr3_write_leveling_sw
 * Desc:     Execute Write leveling phase by SW
 * Args:     freq        - current sequence frequency
 *           dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
int ddr3_write_leveling_sw_reg_dimm(u32 freq, int ratio_2to1,
				    MV_DRAM_INFO *dram_info)
{
	u32 reg, cs, cnt, pup;
	u32 res[MAX_CS];
	__maybe_unused int dpde_flag = 0;

	/* Debug message - Start Write leveling procedure */
	DEBUG_WL_S("DDR3 - Write Leveling - Starting SW WL procedure\n");

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	if (reg & (1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS)) {
		dpde_flag = 1;
		reg_write(REG_DUNIT_CTRL_LOW_ADDR,
			  reg & ~(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS));
	}
#endif

	/* If target freq = 400 move clock start point */
	/* Write to control PUP to Control Deskew Regs */
	if (freq <= DDR_400) {
		for (pup = 0; pup <= dram_info->num_of_total_pups; pup++) {
			/* PUP_DELAY_MASK 0x1F */
			/* reg = 0x0C10001F + (uj << 16); */
			ddr3_write_ctrl_pup_reg(1, pup, CNTRL_PUP_DESKEW + pup,
						0x1F);
		}
	}

	/* Set Output buffer-off to all CS and correct ODT values */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_ODT_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];
			reg |= (1 << REG_DDR3_MR1_OUTBUF_DIS_OFFS);

			/* 0x15D0 - DDR3 MR0 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

	DEBUG_WL_FULL_S("DDR3 - Write Leveling - Qoff and RTT Values are set for all Cs\n");

	/* Enable SW override */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) |
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* [0] = 1 - Enable SW override  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);
	DEBUG_WL_FULL_S("DDR3 - Write Leveling - SW Override Enabled\n");

	/* Enable PHY write leveling mode */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
		~(1 << REG_DRAM_TRAINING_2_WL_MODE_OFFS);
	/* [2] = 0 - TrnWLMode - Enable */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Loop for each cs */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - Starting working with Cs - ",
					(u32) cs, 1);

			/* Refresh X9 current cs */
			DEBUG_WL_FULL_S("DDR3 - Write Leveling - Refresh X9\n");
			for (cnt = 0; cnt < COUNT_WL_RFRS; cnt++) {
				reg =
				    REG_SDRAM_OPERATION_CMD_RFRS & ~(1 <<
								     (REG_SDRAM_OPERATION_CS_OFFS
								      + cs));
				/* [3-0] = 0x2 - refresh, [11-8] - enable current cs */
				reg_write(REG_SDRAM_OPERATION_ADDR, reg);	/* 0x1418 - SDRAM Operation Register */

				do {
					reg =
					    ((reg_read
					      (REG_SDRAM_OPERATION_ADDR)) &
					     REG_SDRAM_OPERATION_CMD_RFRS_DONE);
				} while (reg);	/* Wait for '0' */
			}

			/*
			 * Configure MR1 in Cs[CsNum] - write leveling on,
			 * output buffer on
			 */
			DEBUG_WL_FULL_S("DDR3 - Write Leveling - Configure MR1 for current Cs: WL-on,OB-on\n");
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_OUTBUF_WL_MASK;
			/* Set ODT Values */
			reg &= REG_DDR3_MR1_ODT_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];
			/* Enable WL MODE */
			reg |= (1 << REG_DDR3_MR1_WL_ENA_OFFS);
			/*
			 * [7]=1, [12]=0 - Output Buffer and write leveling
			 * enabled
			 */
			/* 0x15D4 - DDR3 MR1 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);

			/* Write leveling  cs[cs] */
			if (MV_OK !=
			    ddr3_write_leveling_single_cs(cs, freq, ratio_2to1,
							  (u32 *)(res + cs),
							  dram_info)) {
				DEBUG_WL_FULL_C("DDR3 - Write Leveling single Cs - FAILED -  Cs - ",
						(u32) cs, 1);
				return MV_FAIL;
			}

			/* Set TrnWLDeUpd - After each CS is done */
			reg = reg_read(REG_TRAINING_WL_ADDR) |
				(1 << REG_TRAINING_WL_CS_DONE_OFFS);
			/* 0x16AC - Training Write leveling register */
			reg_write(REG_TRAINING_WL_ADDR, reg);

			/*
			 * Debug message - Finished Write leveling cs[cs] -
			 * each PUP Fail/Success
			 */
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - Finished Cs - ", (u32) cs,
					1);
			DEBUG_WL_FULL_C("DDR3 - Write Leveling - The Results: 1-PUP locked, 0-PUP failed -",
					(u32) res[cs], 3);

			/* Configure MR1 in cs[cs] - write leveling off (0), output buffer off (1) */
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_OUTBUF_WL_MASK;
			reg |= (1 << REG_DDR3_MR1_OUTBUF_DIS_OFFS);
			/* No need to sort ODT since it is same CS */
			/* 0x15D4 - DDR3 MR1 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

	/* Disable WL Mode */
	/* [2]=1 - TrnWLMode - Disable */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg |= (1 << REG_DRAM_TRAINING_2_WL_MODE_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Disable SW override - Must be in a different stage */
	/* [0]=0 - Enable SW override  */
	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg &= ~(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS);
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	/* Set Output buffer-on to all CS and correct ODT values */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_ADDR) &
				REG_DDR3_MR1_ODT_MASK;
			reg &= REG_DDR3_MR1_OUTBUF_WL_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];

			/* 0x15D0 - DDR3 MR1 Register */
			reg_write(REG_DDR3_MR1_ADDR, reg);
			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			udelay(MRS_DELAY);
		}
	}

#ifdef MV88F67XX
	/* Dynamic pad issue (BTS669) during WL */
	if (dpde_flag) {
		reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR) |
			(1 << REG_DUNIT_CTRL_LOW_DPDE_OFFS);
		reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);
	}
#endif

	/* If target freq = 400 move clock back */
	/* Write to control PUP to Control Deskew Regs */
	if (freq <= DDR_400) {
		for (pup = 0; pup <= dram_info->num_of_total_pups; pup++) {
			ddr3_write_ctrl_pup_reg(1, pup, CNTRL_PUP_DESKEW + pup,
						0);
		}
	}

	DEBUG_WL_FULL_S("DDR3 - Write Leveling - Finished WL procedure for all Cs\n");
	return MV_OK;
}
#endif

/*
 * Name:     ddr3_write_leveling_single_cs
 * Desc:     Execute Write leveling for single Chip select
 * Args:     cs          - current chip select
 *           freq        - current sequence frequency
 *           result      - res array
 *           dram_info   - main struct
 * Notes:
 * Returns:  MV_OK if success, MV_FAIL if fail.
 */
static int ddr3_write_leveling_single_cs(u32 cs, u32 freq, int ratio_2to1,
					 u32 *result, MV_DRAM_INFO *dram_info)
{
	u32 reg, pup_num, delay, phase, phaseMax, max_pup_num, pup,
		max_pup_mask;

	max_pup_num = dram_info->num_of_total_pups;
	*result = 0;
	u32 flag[MAX_PUP_NUM] = { 0 };

	DEBUG_WL_FULL_C("DDR3 - Write Leveling Single Cs - WL for Cs - ",
			(u32) cs, 1);

	switch (max_pup_num) {
	case 2:
		max_pup_mask = 0x3;
		break;
	case 4:
		max_pup_mask = 0xf;
		DEBUG_WL_C("max_pup_mask =  ", max_pup_mask, 3);
		break;
	case 5:
		max_pup_mask = 0x1f;
		DEBUG_WL_C("max_pup_mask =  ", max_pup_mask, 3);
		break;
	case 8:
		max_pup_mask = 0xff;
		DEBUG_WL_C("max_pup_mask =  ", max_pup_mask, 3);
		break;
	case 9:
		max_pup_mask = 0x1ff;
		DEBUG_WL_C("max_pup_mask =  ", max_pup_mask, 3);
		break;
	default:
		DEBUG_WL_C("ddr3_write_leveling_single_cs wrong max_pup_num =  ",
			   max_pup_num, 3);
		return MV_FAIL;
	}

	/* CS ODT Override */
	reg = reg_read(REG_SDRAM_ODT_CTRL_HIGH_ADDR) &
		REG_SDRAM_ODT_CTRL_HIGH_OVRD_MASK;
	reg |= (REG_SDRAM_ODT_CTRL_HIGH_OVRD_ENA << (2 * cs));
	/* Set 0x3 - Enable ODT on the curent cs and disable on other cs */
	/* 0x1498 - SDRAM ODT Control high */
	reg_write(REG_SDRAM_ODT_CTRL_HIGH_ADDR, reg);

	DEBUG_WL_FULL_S("DDR3 - Write Leveling Single Cs - ODT Asserted for current Cs\n");

	/* tWLMRD Delay */
	/* Delay of minimum 40 Dram clock cycles - 20 Tclk cycles */
	udelay(1);

	/* [1:0] - current cs number */
	reg = (reg_read(REG_TRAINING_WL_ADDR) & REG_TRAINING_WL_CS_MASK) | cs;
	reg |= (1 << REG_TRAINING_WL_UPD_OFFS);	/* [2] - trnWLCsUpd */
	/* 0x16AC - Training Write leveling register */
	reg_write(REG_TRAINING_WL_ADDR, reg);

	/* Broadcast to all PUPs: Reset DQS phase, reset leveling delay */
	ddr3_write_pup_reg(PUP_WL_MODE, cs, PUP_BC, 0, 0);

	/* Seek Edge */
	DEBUG_WL_FULL_S("DDR3 - Write Leveling Single Cs - Seek Edge - Current Cs\n");

	/* Drive DQS high for one cycle - All data PUPs */
	DEBUG_WL_FULL_S("DDR3 - Write Leveling Single Cs - Seek Edge - Driving DQS high for one cycle\n");
	if (!ratio_2to1) {
		reg = (reg_read(REG_TRAINING_WL_ADDR) &
		       REG_TRAINING_WL_RATIO_MASK) | REG_TRAINING_WL_1TO1;
	} else {
		reg = (reg_read(REG_TRAINING_WL_ADDR) &
		       REG_TRAINING_WL_RATIO_MASK) | REG_TRAINING_WL_2TO1;
	}
	/* 0x16AC - Training Write leveling register */
	reg_write(REG_TRAINING_WL_ADDR, reg);

	/* Wait tWLdelay */
	do {
		/* [29] - trnWLDelayExp */
		reg = (reg_read(REG_TRAINING_WL_ADDR)) &
			REG_TRAINING_WL_DELAYEXP_MASK;
	} while (reg == 0x0);	/* Wait for '1' */

	/* Read WL res */
	reg = (reg_read(REG_TRAINING_WL_ADDR) >> REG_TRAINING_WL_RESULTS_OFFS) &
		REG_TRAINING_WL_RESULTS_MASK;
	/* [28:20] - TrnWLResult */

	if (!ratio_2to1) /* Different phase options for 2:1 or 1:1 modes */
		phaseMax = MAX_PHASE_1TO1;
	else
		phaseMax = MAX_PHASE_2TO1;

	DEBUG_WL_FULL_S("DDR3 - Write Leveling Single Cs - Seek Edge - Shift DQS + Octet Leveling\n");

	/* Shift DQS + Octet leveling */
	for (phase = 0; phase < phaseMax; phase++) {
		for (delay = 0; delay < MAX_DELAY; delay++) {
			/*  Broadcast to all PUPs: DQS phase,leveling delay */
			ddr3_write_pup_reg(PUP_WL_MODE, cs, PUP_BC, phase,
					   delay);

			udelay(1);	/* Delay of  3 Tclk cycles */

			DEBUG_WL_FULL_S("DDR3 - Write Leveling Single Cs - Seek Edge: Phase = ");
			DEBUG_WL_FULL_D((u32) phase, 1);
			DEBUG_WL_FULL_S(", Delay = ");
			DEBUG_WL_FULL_D((u32) delay, 1);
			DEBUG_WL_FULL_S("\n");

			/* Drive DQS high for one cycle - All data PUPs */
			if (!ratio_2to1) {
				reg = (reg_read(REG_TRAINING_WL_ADDR) &
				       REG_TRAINING_WL_RATIO_MASK) |
					REG_TRAINING_WL_1TO1;
			} else {
				reg = (reg_read(REG_TRAINING_WL_ADDR) &
				       REG_TRAINING_WL_RATIO_MASK) |
					REG_TRAINING_WL_2TO1;
			}
			reg_write(REG_TRAINING_WL_ADDR, reg);	/* 0x16AC  */

			/* Wait tWLdelay */
			do {
				reg = (reg_read(REG_TRAINING_WL_ADDR)) &
					REG_TRAINING_WL_DELAYEXP_MASK;
			} while (reg == 0x0);	/* [29] Wait for '1' */

			/* Read WL res */
			reg = reg_read(REG_TRAINING_WL_ADDR);
			reg = (reg >> REG_TRAINING_WL_RESULTS_OFFS) &
				REG_TRAINING_WL_RESULTS_MASK;	/* [28:20] */

			DEBUG_WL_FULL_C("DDR3 - Write Leveling Single Cs - Seek Edge: Results =  ",
					(u32) reg, 3);

			/* Update State machine */
			for (pup = 0; pup < (max_pup_num); pup++) {
				/* ECC support - bit 8 */
				pup_num = (pup == dram_info->num_of_std_pups) ?
					ECC_BIT : pup;
				if (dram_info->wl_val[cs][pup][S] == 0) {
					/* Update phase to PUP */
					dram_info->wl_val[cs][pup][P] = phase;
					/* Update delay to PUP */
					dram_info->wl_val[cs][pup][D] = delay;
				}

				if (((reg >> pup_num) & 0x1) == 0)
					flag[pup_num] = 1;

				if (((reg >> pup_num) & 0x1)
				    && (flag[pup_num] == 1)
				    && (dram_info->wl_val[cs][pup][S] == 0)) {
					/*
					 * If the PUP is locked now and in last
					 * counter states
					 */
					/* Go to next state */
					dram_info->wl_val[cs][pup][S] = 1;
					/* Set res */
					*result = *result | (1 << pup_num);
				}
			}

			/* If all locked - Break the loops - Finished */
			if (*result == max_pup_mask) {
				phase = phaseMax;
				delay = MAX_DELAY;
				DEBUG_WL_S("DDR3 - Write Leveling Single Cs - Seek Edge: All Locked\n");
			}
		}
	}

	/* Debug message - Print res for cs[i]: cs,PUP,Phase,Delay */
	DEBUG_WL_C("DDR3 - Write Leveling - Results for CS - ", (u32) cs, 1);
	for (pup = 0; pup < (max_pup_num); pup++) {
		DEBUG_WL_S("DDR3 - Write Leveling - PUP: ");
		DEBUG_WL_D((u32) pup, 1);
		DEBUG_WL_S(", Phase: ");
		DEBUG_WL_D((u32) dram_info->wl_val[cs][pup][P], 1);
		DEBUG_WL_S(", Delay: ");
		DEBUG_WL_D((u32) dram_info->wl_val[cs][pup][D], 2);
		DEBUG_WL_S("\n");
	}

	/* Check if some not locked and return error */
	if (*result != max_pup_mask) {
		DEBUG_WL_S("DDR3 - Write Leveling - ERROR - not all PUPS were locked\n");
		return MV_FAIL;
	}

	/* Configure Each PUP with locked leveling settings */
	for (pup = 0; pup < (max_pup_num); pup++) {
		/* ECC support - bit 8 */
		pup_num = (pup == dram_info->num_of_std_pups) ? ECC_BIT : pup;
		phase = dram_info->wl_val[cs][pup][P];
		delay = dram_info->wl_val[cs][pup][D];
		ddr3_write_pup_reg(PUP_WL_MODE, cs, pup_num, phase, delay);
	}

	/* CS ODT Override */
	reg =  reg_read(REG_SDRAM_ODT_CTRL_HIGH_ADDR) &
		REG_SDRAM_ODT_CTRL_HIGH_OVRD_MASK;
	/* 0x1498 - SDRAM ODT Control high */
	reg_write(REG_SDRAM_ODT_CTRL_HIGH_ADDR, reg);

	return MV_OK;
}

/*
 * Perform DDR3 Control PUP Indirect Write
 */
static void ddr3_write_ctrl_pup_reg(int bc_acc, u32 pup, u32 reg_addr, u32 data)
{
	u32 reg = 0;

	/* Store value for write */
	reg = (data & 0xFFFF);

	/* Set bit 26 for control PHY access */
	reg |= (1 << REG_PHY_CNTRL_OFFS);

	/* Configure BC or UC access to PHYs */
	if (bc_acc == 1)
		reg |= (1 << REG_PHY_BC_OFFS);
	else
		reg |= (pup << REG_PHY_PUP_OFFS);

	/* Set PHY register address to write to */
	reg |= (reg_addr << REG_PHY_CS_OFFS);

	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */
	reg |= REG_PHY_REGISTRY_FILE_ACCESS_OP_WR;
	reg_write(REG_PHY_REGISTRY_FILE_ACCESS_ADDR, reg);	/* 0x16A0 */

	do {
		reg = (reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR)) &
			REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
	} while (reg);		/* Wait for '0' to mark the end of the transaction */
}
