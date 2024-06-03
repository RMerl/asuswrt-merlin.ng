// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright Altera Corporation (C) 2012-2015
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sdram.h>
#include <errno.h>
#include "sequencer.h"

static struct socfpga_sdr_rw_load_manager *sdr_rw_load_mgr_regs =
	(struct socfpga_sdr_rw_load_manager *)
		(SDR_PHYGRP_RWMGRGRP_ADDRESS | 0x800);
static struct socfpga_sdr_rw_load_jump_manager *sdr_rw_load_jump_mgr_regs =
	(struct socfpga_sdr_rw_load_jump_manager *)
		(SDR_PHYGRP_RWMGRGRP_ADDRESS | 0xC00);
static struct socfpga_sdr_reg_file *sdr_reg_file =
	(struct socfpga_sdr_reg_file *)SDR_PHYGRP_REGFILEGRP_ADDRESS;
static struct socfpga_sdr_scc_mgr *sdr_scc_mgr =
	(struct socfpga_sdr_scc_mgr *)
		(SDR_PHYGRP_SCCGRP_ADDRESS | 0xe00);
static struct socfpga_phy_mgr_cmd *phy_mgr_cmd =
	(struct socfpga_phy_mgr_cmd *)SDR_PHYGRP_PHYMGRGRP_ADDRESS;
static struct socfpga_phy_mgr_cfg *phy_mgr_cfg =
	(struct socfpga_phy_mgr_cfg *)
		(SDR_PHYGRP_PHYMGRGRP_ADDRESS | 0x40);
static struct socfpga_data_mgr *data_mgr =
	(struct socfpga_data_mgr *)SDR_PHYGRP_DATAMGRGRP_ADDRESS;
static struct socfpga_sdr_ctrl *sdr_ctrl =
	(struct socfpga_sdr_ctrl *)SDR_CTRLGRP_ADDRESS;

const struct socfpga_sdram_rw_mgr_config *rwcfg;
const struct socfpga_sdram_io_config *iocfg;
const struct socfpga_sdram_misc_config *misccfg;

#define DELTA_D		1

/*
 * In order to reduce ROM size, most of the selectable calibration steps are
 * decided at compile time based on the user's calibration mode selection,
 * as captured by the STATIC_CALIB_STEPS selection below.
 *
 * However, to support simulation-time selection of fast simulation mode, where
 * we skip everything except the bare minimum, we need a few of the steps to
 * be dynamic.  In those cases, we either use the DYNAMIC_CALIB_STEPS for the
 * check, which is based on the rtl-supplied value, or we dynamically compute
 * the value to use based on the dynamically-chosen calibration mode
 */

#define DLEVEL 0
#define STATIC_IN_RTL_SIM 0
#define STATIC_SKIP_DELAY_LOOPS 0

#define STATIC_CALIB_STEPS (STATIC_IN_RTL_SIM | CALIB_SKIP_FULL_TEST | \
	STATIC_SKIP_DELAY_LOOPS)

/* calibration steps requested by the rtl */
static u16 dyn_calib_steps;

/*
 * To make CALIB_SKIP_DELAY_LOOPS a dynamic conditional option
 * instead of static, we use boolean logic to select between
 * non-skip and skip values
 *
 * The mask is set to include all bits when not-skipping, but is
 * zero when skipping
 */

static u16 skip_delay_mask;	/* mask off bits when skipping/not-skipping */

#define SKIP_DELAY_LOOP_VALUE_OR_ZERO(non_skip_value) \
	((non_skip_value) & skip_delay_mask)

static struct gbl_type *gbl;
static struct param_type *param;

static void set_failing_group_stage(u32 group, u32 stage,
	u32 substage)
{
	/*
	 * Only set the global stage if there was not been any other
	 * failing group
	 */
	if (gbl->error_stage == CAL_STAGE_NIL)	{
		gbl->error_substage = substage;
		gbl->error_stage = stage;
		gbl->error_group = group;
	}
}

static void reg_file_set_group(u16 set_group)
{
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xffff0000, set_group << 16);
}

static void reg_file_set_stage(u8 set_stage)
{
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xffff, set_stage & 0xff);
}

static void reg_file_set_sub_stage(u8 set_sub_stage)
{
	set_sub_stage &= 0xff;
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xff00, set_sub_stage << 8);
}

/**
 * phy_mgr_initialize() - Initialize PHY Manager
 *
 * Initialize PHY Manager.
 */
static void phy_mgr_initialize(void)
{
	u32 ratio;

	debug("%s:%d\n", __func__, __LINE__);
	/* Calibration has control over path to memory */
	/*
	 * In Hard PHY this is a 2-bit control:
	 * 0: AFI Mux Select
	 * 1: DDIO Mux Select
	 */
	writel(0x3, &phy_mgr_cfg->mux_sel);

	/* USER memory clock is not stable we begin initialization  */
	writel(0, &phy_mgr_cfg->reset_mem_stbl);

	/* USER calibration status all set to zero */
	writel(0, &phy_mgr_cfg->cal_status);

	writel(0, &phy_mgr_cfg->cal_debug_info);

	/* Init params only if we do NOT skip calibration. */
	if ((dyn_calib_steps & CALIB_SKIP_ALL) == CALIB_SKIP_ALL)
		return;

	ratio = rwcfg->mem_dq_per_read_dqs /
		rwcfg->mem_virtual_groups_per_read_dqs;
	param->read_correct_mask_vg = (1 << ratio) - 1;
	param->write_correct_mask_vg = (1 << ratio) - 1;
	param->read_correct_mask = (1 << rwcfg->mem_dq_per_read_dqs) - 1;
	param->write_correct_mask = (1 << rwcfg->mem_dq_per_write_dqs) - 1;
}

/**
 * set_rank_and_odt_mask() - Set Rank and ODT mask
 * @rank:	Rank mask
 * @odt_mode:	ODT mode, OFF or READ_WRITE
 *
 * Set Rank and ODT mask (On-Die Termination).
 */
static void set_rank_and_odt_mask(const u32 rank, const u32 odt_mode)
{
	u32 odt_mask_0 = 0;
	u32 odt_mask_1 = 0;
	u32 cs_and_odt_mask;

	if (odt_mode == RW_MGR_ODT_MODE_OFF) {
		odt_mask_0 = 0x0;
		odt_mask_1 = 0x0;
	} else {	/* RW_MGR_ODT_MODE_READ_WRITE */
		switch (rwcfg->mem_number_of_ranks) {
		case 1:	/* 1 Rank */
			/* Read: ODT = 0 ; Write: ODT = 1 */
			odt_mask_0 = 0x0;
			odt_mask_1 = 0x1;
			break;
		case 2:	/* 2 Ranks */
			if (rwcfg->mem_number_of_cs_per_dimm == 1) {
				/*
				 * - Dual-Slot , Single-Rank (1 CS per DIMM)
				 *   OR
				 * - RDIMM, 4 total CS (2 CS per DIMM, 2 DIMM)
				 *
				 * Since MEM_NUMBER_OF_RANKS is 2, they
				 * are both single rank with 2 CS each
				 * (special for RDIMM).
				 *
				 * Read: Turn on ODT on the opposite rank
				 * Write: Turn on ODT on all ranks
				 */
				odt_mask_0 = 0x3 & ~(1 << rank);
				odt_mask_1 = 0x3;
			} else {
				/*
				 * - Single-Slot , Dual-Rank (2 CS per DIMM)
				 *
				 * Read: Turn on ODT off on all ranks
				 * Write: Turn on ODT on active rank
				 */
				odt_mask_0 = 0x0;
				odt_mask_1 = 0x3 & (1 << rank);
			}
			break;
		case 4:	/* 4 Ranks */
			/* Read:
			 * ----------+-----------------------+
			 *           |         ODT           |
			 * Read From +-----------------------+
			 *   Rank    |  3  |  2  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *     0     |  0  |  1  |  0  |  0  |
			 *     1     |  1  |  0  |  0  |  0  |
			 *     2     |  0  |  0  |  0  |  1  |
			 *     3     |  0  |  0  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *
			 * Write:
			 * ----------+-----------------------+
			 *           |         ODT           |
			 * Write To  +-----------------------+
			 *   Rank    |  3  |  2  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *     0     |  0  |  1  |  0  |  1  |
			 *     1     |  1  |  0  |  1  |  0  |
			 *     2     |  0  |  1  |  0  |  1  |
			 *     3     |  1  |  0  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 */
			switch (rank) {
			case 0:
				odt_mask_0 = 0x4;
				odt_mask_1 = 0x5;
				break;
			case 1:
				odt_mask_0 = 0x8;
				odt_mask_1 = 0xA;
				break;
			case 2:
				odt_mask_0 = 0x1;
				odt_mask_1 = 0x5;
				break;
			case 3:
				odt_mask_0 = 0x2;
				odt_mask_1 = 0xA;
				break;
			}
			break;
		}
	}

	cs_and_odt_mask = (0xFF & ~(1 << rank)) |
			  ((0xFF & odt_mask_0) << 8) |
			  ((0xFF & odt_mask_1) << 16);
	writel(cs_and_odt_mask, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				RW_MGR_SET_CS_AND_ODT_MASK_OFFSET);
}

/**
 * scc_mgr_set() - Set SCC Manager register
 * @off:	Base offset in SCC Manager space
 * @grp:	Read/Write group
 * @val:	Value to be set
 *
 * This function sets the SCC Manager (Scan Chain Control Manager) register.
 */
static void scc_mgr_set(u32 off, u32 grp, u32 val)
{
	writel(val, SDR_PHYGRP_SCCGRP_ADDRESS | off | (grp << 2));
}

/**
 * scc_mgr_initialize() - Initialize SCC Manager registers
 *
 * Initialize SCC Manager registers.
 */
static void scc_mgr_initialize(void)
{
	/*
	 * Clear register file for HPS. 16 (2^4) is the size of the
	 * full register file in the scc mgr:
	 *	RFILE_DEPTH = 1 + log2(MEM_DQ_PER_DQS + 1 + MEM_DM_PER_DQS +
	 *                             MEM_IF_READ_DQS_WIDTH - 1);
	 */
	int i;

	for (i = 0; i < 16; i++) {
		debug_cond(DLEVEL >= 1, "%s:%d: Clearing SCC RFILE index %u\n",
			   __func__, __LINE__, i);
		scc_mgr_set(SCC_MGR_HHP_RFILE_OFFSET, i, 0);
	}
}

static void scc_mgr_set_dqdqs_output_phase(u32 write_group, u32 phase)
{
	scc_mgr_set(SCC_MGR_DQDQS_OUT_PHASE_OFFSET, write_group, phase);
}

static void scc_mgr_set_dqs_bus_in_delay(u32 read_group, u32 delay)
{
	scc_mgr_set(SCC_MGR_DQS_IN_DELAY_OFFSET, read_group, delay);
}

static void scc_mgr_set_dqs_en_phase(u32 read_group, u32 phase)
{
	scc_mgr_set(SCC_MGR_DQS_EN_PHASE_OFFSET, read_group, phase);
}

static void scc_mgr_set_dqs_en_delay(u32 read_group, u32 delay)
{
	scc_mgr_set(SCC_MGR_DQS_EN_DELAY_OFFSET, read_group, delay);
}

static void scc_mgr_set_dq_in_delay(u32 dq_in_group, u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_IN_DELAY_OFFSET, dq_in_group, delay);
}

static void scc_mgr_set_dqs_io_in_delay(u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_IN_DELAY_OFFSET, rwcfg->mem_dq_per_write_dqs,
		    delay);
}

static void scc_mgr_set_dm_in_delay(u32 dm, u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_IN_DELAY_OFFSET,
		    rwcfg->mem_dq_per_write_dqs + 1 + dm,
		    delay);
}

static void scc_mgr_set_dq_out1_delay(u32 dq_in_group, u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_OUT1_DELAY_OFFSET, dq_in_group, delay);
}

static void scc_mgr_set_dqs_out1_delay(u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_OUT1_DELAY_OFFSET, rwcfg->mem_dq_per_write_dqs,
		    delay);
}

static void scc_mgr_set_dm_out1_delay(u32 dm, u32 delay)
{
	scc_mgr_set(SCC_MGR_IO_OUT1_DELAY_OFFSET,
		    rwcfg->mem_dq_per_write_dqs + 1 + dm,
		    delay);
}

/* load up dqs config settings */
static void scc_mgr_load_dqs(u32 dqs)
{
	writel(dqs, &sdr_scc_mgr->dqs_ena);
}

/* load up dqs io config settings */
static void scc_mgr_load_dqs_io(void)
{
	writel(0, &sdr_scc_mgr->dqs_io_ena);
}

/* load up dq config settings */
static void scc_mgr_load_dq(u32 dq_in_group)
{
	writel(dq_in_group, &sdr_scc_mgr->dq_ena);
}

/* load up dm config settings */
static void scc_mgr_load_dm(u32 dm)
{
	writel(dm, &sdr_scc_mgr->dm_ena);
}

/**
 * scc_mgr_set_all_ranks() - Set SCC Manager register for all ranks
 * @off:	Base offset in SCC Manager space
 * @grp:	Read/Write group
 * @val:	Value to be set
 * @update:	If non-zero, trigger SCC Manager update for all ranks
 *
 * This function sets the SCC Manager (Scan Chain Control Manager) register
 * and optionally triggers the SCC update for all ranks.
 */
static void scc_mgr_set_all_ranks(const u32 off, const u32 grp, const u32 val,
				  const int update)
{
	u32 r;

	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		scc_mgr_set(off, grp, val);

		if (update || (r == 0)) {
			writel(grp, &sdr_scc_mgr->dqs_ena);
			writel(0, &sdr_scc_mgr->update);
		}
	}
}

static void scc_mgr_set_dqs_en_phase_all_ranks(u32 read_group, u32 phase)
{
	/*
	 * USER although the h/w doesn't support different phases per
	 * shadow register, for simplicity our scc manager modeling
	 * keeps different phase settings per shadow reg, and it's
	 * important for us to keep them in sync to match h/w.
	 * for efficiency, the scan chain update should occur only
	 * once to sr0.
	 */
	scc_mgr_set_all_ranks(SCC_MGR_DQS_EN_PHASE_OFFSET,
			      read_group, phase, 0);
}

static void scc_mgr_set_dqdqs_output_phase_all_ranks(u32 write_group,
						     u32 phase)
{
	/*
	 * USER although the h/w doesn't support different phases per
	 * shadow register, for simplicity our scc manager modeling
	 * keeps different phase settings per shadow reg, and it's
	 * important for us to keep them in sync to match h/w.
	 * for efficiency, the scan chain update should occur only
	 * once to sr0.
	 */
	scc_mgr_set_all_ranks(SCC_MGR_DQDQS_OUT_PHASE_OFFSET,
			      write_group, phase, 0);
}

static void scc_mgr_set_dqs_en_delay_all_ranks(u32 read_group,
					       u32 delay)
{
	/*
	 * In shadow register mode, the T11 settings are stored in
	 * registers in the core, which are updated by the DQS_ENA
	 * signals. Not issuing the SCC_MGR_UPD command allows us to
	 * save lots of rank switching overhead, by calling
	 * select_shadow_regs_for_update with update_scan_chains
	 * set to 0.
	 */
	scc_mgr_set_all_ranks(SCC_MGR_DQS_EN_DELAY_OFFSET,
			      read_group, delay, 1);
}

/**
 * scc_mgr_set_oct_out1_delay() - Set OCT output delay
 * @write_group:	Write group
 * @delay:		Delay value
 *
 * This function sets the OCT output delay in SCC manager.
 */
static void scc_mgr_set_oct_out1_delay(const u32 write_group, const u32 delay)
{
	const int ratio = rwcfg->mem_if_read_dqs_width /
			  rwcfg->mem_if_write_dqs_width;
	const int base = write_group * ratio;
	int i;
	/*
	 * Load the setting in the SCC manager
	 * Although OCT affects only write data, the OCT delay is controlled
	 * by the DQS logic block which is instantiated once per read group.
	 * For protocols where a write group consists of multiple read groups,
	 * the setting must be set multiple times.
	 */
	for (i = 0; i < ratio; i++)
		scc_mgr_set(SCC_MGR_OCT_OUT1_DELAY_OFFSET, base + i, delay);
}

/**
 * scc_mgr_set_hhp_extras() - Set HHP extras.
 *
 * Load the fixed setting in the SCC manager HHP extras.
 */
static void scc_mgr_set_hhp_extras(void)
{
	/*
	 * Load the fixed setting in the SCC manager
	 * bits: 0:0 = 1'b1	- DQS bypass
	 * bits: 1:1 = 1'b1	- DQ bypass
	 * bits: 4:2 = 3'b001	- rfifo_mode
	 * bits: 6:5 = 2'b01	- rfifo clock_select
	 * bits: 7:7 = 1'b0	- separate gating from ungating setting
	 * bits: 8:8 = 1'b0	- separate OE from Output delay setting
	 */
	const u32 value = (0 << 8) | (0 << 7) | (1 << 5) |
			  (1 << 2) | (1 << 1) | (1 << 0);
	const u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS |
			 SCC_MGR_HHP_GLOBALS_OFFSET |
			 SCC_MGR_HHP_EXTRAS_OFFSET;

	debug_cond(DLEVEL >= 1, "%s:%d Setting HHP Extras\n",
		   __func__, __LINE__);
	writel(value, addr);
	debug_cond(DLEVEL >= 1, "%s:%d Done Setting HHP Extras\n",
		   __func__, __LINE__);
}

/**
 * scc_mgr_zero_all() - Zero all DQS config
 *
 * Zero all DQS config.
 */
static void scc_mgr_zero_all(void)
{
	int i, r;

	/*
	 * USER Zero all DQS config settings, across all groups and all
	 * shadow registers
	 */
	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		for (i = 0; i < rwcfg->mem_if_read_dqs_width; i++) {
			/*
			 * The phases actually don't exist on a per-rank basis,
			 * but there's no harm updating them several times, so
			 * let's keep the code simple.
			 */
			scc_mgr_set_dqs_bus_in_delay(i, iocfg->dqs_in_reserve);
			scc_mgr_set_dqs_en_phase(i, 0);
			scc_mgr_set_dqs_en_delay(i, 0);
		}

		for (i = 0; i < rwcfg->mem_if_write_dqs_width; i++) {
			scc_mgr_set_dqdqs_output_phase(i, 0);
			/* Arria V/Cyclone V don't have out2. */
			scc_mgr_set_oct_out1_delay(i, iocfg->dqs_out_reserve);
		}
	}

	/* Multicast to all DQS group enables. */
	writel(0xff, &sdr_scc_mgr->dqs_ena);
	writel(0, &sdr_scc_mgr->update);
}

/**
 * scc_set_bypass_mode() - Set bypass mode and trigger SCC update
 * @write_group:	Write group
 *
 * Set bypass mode and trigger SCC update.
 */
static void scc_set_bypass_mode(const u32 write_group)
{
	/* Multicast to all DQ enables. */
	writel(0xff, &sdr_scc_mgr->dq_ena);
	writel(0xff, &sdr_scc_mgr->dm_ena);

	/* Update current DQS IO enable. */
	writel(0, &sdr_scc_mgr->dqs_io_ena);

	/* Update the DQS logic. */
	writel(write_group, &sdr_scc_mgr->dqs_ena);

	/* Hit update. */
	writel(0, &sdr_scc_mgr->update);
}

/**
 * scc_mgr_load_dqs_for_write_group() - Load DQS settings for Write Group
 * @write_group:	Write group
 *
 * Load DQS settings for Write Group, do not trigger SCC update.
 */
static void scc_mgr_load_dqs_for_write_group(const u32 write_group)
{
	const int ratio = rwcfg->mem_if_read_dqs_width /
			  rwcfg->mem_if_write_dqs_width;
	const int base = write_group * ratio;
	int i;
	/*
	 * Load the setting in the SCC manager
	 * Although OCT affects only write data, the OCT delay is controlled
	 * by the DQS logic block which is instantiated once per read group.
	 * For protocols where a write group consists of multiple read groups,
	 * the setting must be set multiple times.
	 */
	for (i = 0; i < ratio; i++)
		writel(base + i, &sdr_scc_mgr->dqs_ena);
}

/**
 * scc_mgr_zero_group() - Zero all configs for a group
 *
 * Zero DQ, DM, DQS and OCT configs for a group.
 */
static void scc_mgr_zero_group(const u32 write_group, const int out_only)
{
	int i, r;

	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		/* Zero all DQ config settings. */
		for (i = 0; i < rwcfg->mem_dq_per_write_dqs; i++) {
			scc_mgr_set_dq_out1_delay(i, 0);
			if (!out_only)
				scc_mgr_set_dq_in_delay(i, 0);
		}

		/* Multicast to all DQ enables. */
		writel(0xff, &sdr_scc_mgr->dq_ena);

		/* Zero all DM config settings. */
		for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++) {
			if (!out_only)
				scc_mgr_set_dm_in_delay(i, 0);
			scc_mgr_set_dm_out1_delay(i, 0);
		}

		/* Multicast to all DM enables. */
		writel(0xff, &sdr_scc_mgr->dm_ena);

		/* Zero all DQS IO settings. */
		if (!out_only)
			scc_mgr_set_dqs_io_in_delay(0);

		/* Arria V/Cyclone V don't have out2. */
		scc_mgr_set_dqs_out1_delay(iocfg->dqs_out_reserve);
		scc_mgr_set_oct_out1_delay(write_group, iocfg->dqs_out_reserve);
		scc_mgr_load_dqs_for_write_group(write_group);

		/* Multicast to all DQS IO enables (only 1 in total). */
		writel(0, &sdr_scc_mgr->dqs_io_ena);

		/* Hit update to zero everything. */
		writel(0, &sdr_scc_mgr->update);
	}
}

/*
 * apply and load a particular input delay for the DQ pins in a group
 * group_bgn is the index of the first dq pin (in the write group)
 */
static void scc_mgr_apply_group_dq_in_delay(u32 group_bgn, u32 delay)
{
	u32 i, p;

	for (i = 0, p = group_bgn; i < rwcfg->mem_dq_per_read_dqs; i++, p++) {
		scc_mgr_set_dq_in_delay(p, delay);
		scc_mgr_load_dq(p);
	}
}

/**
 * scc_mgr_apply_group_dq_out1_delay() - Apply and load an output delay for the DQ pins in a group
 * @delay:		Delay value
 *
 * Apply and load a particular output delay for the DQ pins in a group.
 */
static void scc_mgr_apply_group_dq_out1_delay(const u32 delay)
{
	int i;

	for (i = 0; i < rwcfg->mem_dq_per_write_dqs; i++) {
		scc_mgr_set_dq_out1_delay(i, delay);
		scc_mgr_load_dq(i);
	}
}

/* apply and load a particular output delay for the DM pins in a group */
static void scc_mgr_apply_group_dm_out1_delay(u32 delay1)
{
	u32 i;

	for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++) {
		scc_mgr_set_dm_out1_delay(i, delay1);
		scc_mgr_load_dm(i);
	}
}


/* apply and load delay on both DQS and OCT out1 */
static void scc_mgr_apply_group_dqs_io_and_oct_out1(u32 write_group,
						    u32 delay)
{
	scc_mgr_set_dqs_out1_delay(delay);
	scc_mgr_load_dqs_io();

	scc_mgr_set_oct_out1_delay(write_group, delay);
	scc_mgr_load_dqs_for_write_group(write_group);
}

/**
 * scc_mgr_apply_group_all_out_delay_add() - Apply a delay to the entire output side: DQ, DM, DQS, OCT
 * @write_group:	Write group
 * @delay:		Delay value
 *
 * Apply a delay to the entire output side: DQ, DM, DQS, OCT.
 */
static void scc_mgr_apply_group_all_out_delay_add(const u32 write_group,
						  const u32 delay)
{
	u32 i, new_delay;

	/* DQ shift */
	for (i = 0; i < rwcfg->mem_dq_per_write_dqs; i++)
		scc_mgr_load_dq(i);

	/* DM shift */
	for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++)
		scc_mgr_load_dm(i);

	/* DQS shift */
	new_delay = READ_SCC_DQS_IO_OUT2_DELAY + delay;
	if (new_delay > iocfg->io_out2_delay_max) {
		debug_cond(DLEVEL >= 1,
			   "%s:%d (%u, %u) DQS: %u > %d; adding %u to OUT1\n",
			   __func__, __LINE__, write_group, delay, new_delay,
			   iocfg->io_out2_delay_max,
			   new_delay - iocfg->io_out2_delay_max);
		new_delay -= iocfg->io_out2_delay_max;
		scc_mgr_set_dqs_out1_delay(new_delay);
	}

	scc_mgr_load_dqs_io();

	/* OCT shift */
	new_delay = READ_SCC_OCT_OUT2_DELAY + delay;
	if (new_delay > iocfg->io_out2_delay_max) {
		debug_cond(DLEVEL >= 1,
			   "%s:%d (%u, %u) DQS: %u > %d; adding %u to OUT1\n",
			   __func__, __LINE__, write_group, delay,
			   new_delay, iocfg->io_out2_delay_max,
			   new_delay - iocfg->io_out2_delay_max);
		new_delay -= iocfg->io_out2_delay_max;
		scc_mgr_set_oct_out1_delay(write_group, new_delay);
	}

	scc_mgr_load_dqs_for_write_group(write_group);
}

/**
 * scc_mgr_apply_group_all_out_delay_add() - Apply a delay to the entire output side to all ranks
 * @write_group:	Write group
 * @delay:		Delay value
 *
 * Apply a delay to the entire output side (DQ, DM, DQS, OCT) to all ranks.
 */
static void
scc_mgr_apply_group_all_out_delay_add_all_ranks(const u32 write_group,
						const u32 delay)
{
	int r;

	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		scc_mgr_apply_group_all_out_delay_add(write_group, delay);
		writel(0, &sdr_scc_mgr->update);
	}
}

/**
 * set_jump_as_return() - Return instruction optimization
 *
 * Optimization used to recover some slots in ddr3 inst_rom could be
 * applied to other protocols if we wanted to
 */
static void set_jump_as_return(void)
{
	/*
	 * To save space, we replace return with jump to special shared
	 * RETURN instruction so we set the counter to large value so that
	 * we always jump.
	 */
	writel(0xff, &sdr_rw_load_mgr_regs->load_cntr0);
	writel(rwcfg->rreturn, &sdr_rw_load_jump_mgr_regs->load_jump_add0);
}

/**
 * delay_for_n_mem_clocks() - Delay for N memory clocks
 * @clocks:	Length of the delay
 *
 * Delay for N memory clocks.
 */
static void delay_for_n_mem_clocks(const u32 clocks)
{
	u32 afi_clocks;
	u16 c_loop;
	u8 inner;
	u8 outer;

	debug("%s:%d: clocks=%u ... start\n", __func__, __LINE__, clocks);

	/* Scale (rounding up) to get afi clocks. */
	afi_clocks = DIV_ROUND_UP(clocks, misccfg->afi_rate_ratio);
	if (afi_clocks)	/* Temporary underflow protection */
		afi_clocks--;

	/*
	 * Note, we don't bother accounting for being off a little
	 * bit because of a few extra instructions in outer loops.
	 * Note, the loops have a test at the end, and do the test
	 * before the decrement, and so always perform the loop
	 * 1 time more than the counter value
	 */
	c_loop = afi_clocks >> 16;
	outer = c_loop ? 0xff : (afi_clocks >> 8);
	inner = outer ? 0xff : afi_clocks;

	/*
	 * rom instructions are structured as follows:
	 *
	 *    IDLE_LOOP2: jnz cntr0, TARGET_A
	 *    IDLE_LOOP1: jnz cntr1, TARGET_B
	 *                return
	 *
	 * so, when doing nested loops, TARGET_A is set to IDLE_LOOP2, and
	 * TARGET_B is set to IDLE_LOOP2 as well
	 *
	 * if we have no outer loop, though, then we can use IDLE_LOOP1 only,
	 * and set TARGET_B to IDLE_LOOP1 and we skip IDLE_LOOP2 entirely
	 *
	 * a little confusing, but it helps save precious space in the inst_rom
	 * and sequencer rom and keeps the delays more accurate and reduces
	 * overhead
	 */
	if (afi_clocks < 0x100) {
		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(inner),
		       &sdr_rw_load_mgr_regs->load_cntr1);

		writel(rwcfg->idle_loop1,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(rwcfg->idle_loop1, SDR_PHYGRP_RWMGRGRP_ADDRESS |
					  RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	} else {
		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(inner),
		       &sdr_rw_load_mgr_regs->load_cntr0);

		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(outer),
		       &sdr_rw_load_mgr_regs->load_cntr1);

		writel(rwcfg->idle_loop2,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(rwcfg->idle_loop2,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		do {
			writel(rwcfg->idle_loop2,
			       SDR_PHYGRP_RWMGRGRP_ADDRESS |
			       RW_MGR_RUN_SINGLE_GROUP_OFFSET);
		} while (c_loop-- != 0);
	}
	debug("%s:%d clocks=%u ... end\n", __func__, __LINE__, clocks);
}

/**
 * rw_mgr_mem_init_load_regs() - Load instruction registers
 * @cntr0:	Counter 0 value
 * @cntr1:	Counter 1 value
 * @cntr2:	Counter 2 value
 * @jump:	Jump instruction value
 *
 * Load instruction registers.
 */
static void rw_mgr_mem_init_load_regs(u32 cntr0, u32 cntr1, u32 cntr2, u32 jump)
{
	u32 grpaddr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
			   RW_MGR_RUN_SINGLE_GROUP_OFFSET;

	/* Load counters */
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(cntr0),
	       &sdr_rw_load_mgr_regs->load_cntr0);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(cntr1),
	       &sdr_rw_load_mgr_regs->load_cntr1);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(cntr2),
	       &sdr_rw_load_mgr_regs->load_cntr2);

	/* Load jump address */
	writel(jump, &sdr_rw_load_jump_mgr_regs->load_jump_add0);
	writel(jump, &sdr_rw_load_jump_mgr_regs->load_jump_add1);
	writel(jump, &sdr_rw_load_jump_mgr_regs->load_jump_add2);

	/* Execute count instruction */
	writel(jump, grpaddr);
}

/**
 * rw_mgr_mem_load_user() - Load user calibration values
 * @fin1:	Final instruction 1
 * @fin2:	Final instruction 2
 * @precharge:	If 1, precharge the banks at the end
 *
 * Load user calibration values and optionally precharge the banks.
 */
static void rw_mgr_mem_load_user(const u32 fin1, const u32 fin2,
				 const int precharge)
{
	u32 grpaddr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
		      RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	u32 r;

	for (r = 0; r < rwcfg->mem_number_of_ranks; r++) {
		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_OFF);

		/* precharge all banks ... */
		if (precharge)
			writel(rwcfg->precharge_all, grpaddr);

		/*
		 * USER Use Mirror-ed commands for odd ranks if address
		 * mirrorring is on
		 */
		if ((rwcfg->mem_address_mirroring >> r) & 0x1) {
			set_jump_as_return();
			writel(rwcfg->mrs2_mirr, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(rwcfg->mrs3_mirr, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(rwcfg->mrs1_mirr, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(fin1, grpaddr);
		} else {
			set_jump_as_return();
			writel(rwcfg->mrs2, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(rwcfg->mrs3, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(rwcfg->mrs1, grpaddr);
			set_jump_as_return();
			writel(fin2, grpaddr);
		}

		if (precharge)
			continue;

		set_jump_as_return();
		writel(rwcfg->zqcl, grpaddr);

		/* tZQinit = tDLLK = 512 ck cycles */
		delay_for_n_mem_clocks(512);
	}
}

/**
 * rw_mgr_mem_initialize() - Initialize RW Manager
 *
 * Initialize RW Manager.
 */
static void rw_mgr_mem_initialize(void)
{
	debug("%s:%d\n", __func__, __LINE__);

	/* The reset / cke part of initialization is broadcasted to all ranks */
	writel(RW_MGR_RANK_ALL, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				RW_MGR_SET_CS_AND_ODT_MASK_OFFSET);

	/*
	 * Here's how you load register for a loop
	 * Counters are located @ 0x800
	 * Jump address are located @ 0xC00
	 * For both, registers 0 to 3 are selected using bits 3 and 2, like
	 * in 0x800, 0x804, 0x808, 0x80C and 0xC00, 0xC04, 0xC08, 0xC0C
	 * I know this ain't pretty, but Avalon bus throws away the 2 least
	 * significant bits
	 */

	/* Start with memory RESET activated */

	/* tINIT = 200us */

	/*
	 * 200us @ 266MHz (3.75 ns) ~ 54000 clock cycles
	 * If a and b are the number of iteration in 2 nested loops
	 * it takes the following number of cycles to complete the operation:
	 * number_of_cycles = ((2 + n) * a + 2) * b
	 * where n is the number of instruction in the inner loop
	 * One possible solution is n = 0 , a = 256 , b = 106 => a = FF,
	 * b = 6A
	 */
	rw_mgr_mem_init_load_regs(misccfg->tinit_cntr0_val,
				  misccfg->tinit_cntr1_val,
				  misccfg->tinit_cntr2_val,
				  rwcfg->init_reset_0_cke_0);

	/* Indicate that memory is stable. */
	writel(1, &phy_mgr_cfg->reset_mem_stbl);

	/*
	 * transition the RESET to high
	 * Wait for 500us
	 */

	/*
	 * 500us @ 266MHz (3.75 ns) ~ 134000 clock cycles
	 * If a and b are the number of iteration in 2 nested loops
	 * it takes the following number of cycles to complete the operation
	 * number_of_cycles = ((2 + n) * a + 2) * b
	 * where n is the number of instruction in the inner loop
	 * One possible solution is n = 2 , a = 131 , b = 256 => a = 83,
	 * b = FF
	 */
	rw_mgr_mem_init_load_regs(misccfg->treset_cntr0_val,
				  misccfg->treset_cntr1_val,
				  misccfg->treset_cntr2_val,
				  rwcfg->init_reset_1_cke_0);

	/* Bring up clock enable. */

	/* tXRP < 250 ck cycles */
	delay_for_n_mem_clocks(250);

	rw_mgr_mem_load_user(rwcfg->mrs0_dll_reset_mirr, rwcfg->mrs0_dll_reset,
			     0);
}

/**
 * rw_mgr_mem_handoff() - Hand off the memory to user
 *
 * At the end of calibration we have to program the user settings in
 * and hand off the memory to the user.
 */
static void rw_mgr_mem_handoff(void)
{
	rw_mgr_mem_load_user(rwcfg->mrs0_user_mirr, rwcfg->mrs0_user, 1);
	/*
	 * Need to wait tMOD (12CK or 15ns) time before issuing other
	 * commands, but we will have plenty of NIOS cycles before actual
	 * handoff so its okay.
	 */
}

/**
 * rw_mgr_mem_calibrate_write_test_issue() - Issue write test command
 * @group:	Write Group
 * @use_dm:	Use DM
 *
 * Issue write test command. Two variants are provided, one that just tests
 * a write pattern and another that tests datamask functionality.
 */
static void rw_mgr_mem_calibrate_write_test_issue(u32 group,
						  u32 test_dm)
{
	const u32 quick_write_mode =
		(STATIC_CALIB_STEPS & CALIB_SKIP_WRITES) &&
		misccfg->enable_super_quick_calibration;
	u32 mcc_instruction;
	u32 rw_wl_nop_cycles;

	/*
	 * Set counter and jump addresses for the right
	 * number of NOP cycles.
	 * The number of supported NOP cycles can range from -1 to infinity
	 * Three different cases are handled:
	 *
	 * 1. For a number of NOP cycles greater than 0, the RW Mgr looping
	 *    mechanism will be used to insert the right number of NOPs
	 *
	 * 2. For a number of NOP cycles equals to 0, the micro-instruction
	 *    issuing the write command will jump straight to the
	 *    micro-instruction that turns on DQS (for DDRx), or outputs write
	 *    data (for RLD), skipping
	 *    the NOP micro-instruction all together
	 *
	 * 3. A number of NOP cycles equal to -1 indicates that DQS must be
	 *    turned on in the same micro-instruction that issues the write
	 *    command. Then we need
	 *    to directly jump to the micro-instruction that sends out the data
	 *
	 * NOTE: Implementing this mechanism uses 2 RW Mgr jump-counters
	 *       (2 and 3). One jump-counter (0) is used to perform multiple
	 *       write-read operations.
	 *       one counter left to issue this command in "multiple-group" mode
	 */

	rw_wl_nop_cycles = gbl->rw_wl_nop_cycles;

	if (rw_wl_nop_cycles == -1) {
		/*
		 * CNTR 2 - We want to execute the special write operation that
		 * turns on DQS right away and then skip directly to the
		 * instruction that sends out the data. We set the counter to a
		 * large number so that the jump is always taken.
		 */
		writel(0xFF, &sdr_rw_load_mgr_regs->load_cntr2);

		/* CNTR 3 - Not used */
		if (test_dm) {
			mcc_instruction = rwcfg->lfsr_wr_rd_dm_bank_0_wl_1;
			writel(rwcfg->lfsr_wr_rd_dm_bank_0_data,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
			writel(rwcfg->lfsr_wr_rd_dm_bank_0_nop,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add3);
		} else {
			mcc_instruction = rwcfg->lfsr_wr_rd_bank_0_wl_1;
			writel(rwcfg->lfsr_wr_rd_bank_0_data,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
			writel(rwcfg->lfsr_wr_rd_bank_0_nop,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add3);
		}
	} else if (rw_wl_nop_cycles == 0) {
		/*
		 * CNTR 2 - We want to skip the NOP operation and go straight
		 * to the DQS enable instruction. We set the counter to a large
		 * number so that the jump is always taken.
		 */
		writel(0xFF, &sdr_rw_load_mgr_regs->load_cntr2);

		/* CNTR 3 - Not used */
		if (test_dm) {
			mcc_instruction = rwcfg->lfsr_wr_rd_dm_bank_0;
			writel(rwcfg->lfsr_wr_rd_dm_bank_0_dqs,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
		} else {
			mcc_instruction = rwcfg->lfsr_wr_rd_bank_0;
			writel(rwcfg->lfsr_wr_rd_bank_0_dqs,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
		}
	} else {
		/*
		 * CNTR 2 - In this case we want to execute the next instruction
		 * and NOT take the jump. So we set the counter to 0. The jump
		 * address doesn't count.
		 */
		writel(0x0, &sdr_rw_load_mgr_regs->load_cntr2);
		writel(0x0, &sdr_rw_load_jump_mgr_regs->load_jump_add2);

		/*
		 * CNTR 3 - Set the nop counter to the number of cycles we
		 * need to loop for, minus 1.
		 */
		writel(rw_wl_nop_cycles - 1, &sdr_rw_load_mgr_regs->load_cntr3);
		if (test_dm) {
			mcc_instruction = rwcfg->lfsr_wr_rd_dm_bank_0;
			writel(rwcfg->lfsr_wr_rd_dm_bank_0_nop,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add3);
		} else {
			mcc_instruction = rwcfg->lfsr_wr_rd_bank_0;
			writel(rwcfg->lfsr_wr_rd_bank_0_nop,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add3);
		}
	}

	writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
		  RW_MGR_RESET_READ_DATAPATH_OFFSET);

	if (quick_write_mode)
		writel(0x08, &sdr_rw_load_mgr_regs->load_cntr0);
	else
		writel(0x40, &sdr_rw_load_mgr_regs->load_cntr0);

	writel(mcc_instruction, &sdr_rw_load_jump_mgr_regs->load_jump_add0);

	/*
	 * CNTR 1 - This is used to ensure enough time elapses
	 * for read data to come back.
	 */
	writel(0x30, &sdr_rw_load_mgr_regs->load_cntr1);

	if (test_dm) {
		writel(rwcfg->lfsr_wr_rd_dm_bank_0_wait,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);
	} else {
		writel(rwcfg->lfsr_wr_rd_bank_0_wait,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);
	}

	writel(mcc_instruction, (SDR_PHYGRP_RWMGRGRP_ADDRESS |
				RW_MGR_RUN_SINGLE_GROUP_OFFSET) +
				(group << 2));
}

/**
 * rw_mgr_mem_calibrate_write_test() - Test writes, check for single/multiple pass
 * @rank_bgn:		Rank number
 * @write_group:	Write Group
 * @use_dm:		Use DM
 * @all_correct:	All bits must be correct in the mask
 * @bit_chk:		Resulting bit mask after the test
 * @all_ranks:		Test all ranks
 *
 * Test writes, can check for a single bit pass or multiple bit pass.
 */
static int
rw_mgr_mem_calibrate_write_test(const u32 rank_bgn, const u32 write_group,
				const u32 use_dm, const u32 all_correct,
				u32 *bit_chk, const u32 all_ranks)
{
	const u32 rank_end = all_ranks ?
				rwcfg->mem_number_of_ranks :
				(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	const u32 shift_ratio = rwcfg->mem_dq_per_write_dqs /
				rwcfg->mem_virtual_groups_per_write_dqs;
	const u32 correct_mask_vg = param->write_correct_mask_vg;

	u32 tmp_bit_chk, base_rw_mgr;
	int vg, r;

	*bit_chk = param->write_correct_mask;

	for (r = rank_bgn; r < rank_end; r++) {
		/* Set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		tmp_bit_chk = 0;
		for (vg = rwcfg->mem_virtual_groups_per_write_dqs - 1;
		     vg >= 0; vg--) {
			/* Reset the FIFOs to get pointers to known state. */
			writel(0, &phy_mgr_cmd->fifo_reset);

			rw_mgr_mem_calibrate_write_test_issue(
				write_group *
				rwcfg->mem_virtual_groups_per_write_dqs + vg,
				use_dm);

			base_rw_mgr = readl(SDR_PHYGRP_RWMGRGRP_ADDRESS);
			tmp_bit_chk <<= shift_ratio;
			tmp_bit_chk |= (correct_mask_vg & ~(base_rw_mgr));
		}

		*bit_chk &= tmp_bit_chk;
	}

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
	if (all_correct) {
		debug_cond(DLEVEL >= 2,
			   "write_test(%u,%u,ALL) : %u == %u => %i\n",
			   write_group, use_dm, *bit_chk,
			   param->write_correct_mask,
			   *bit_chk == param->write_correct_mask);
		return *bit_chk == param->write_correct_mask;
	} else {
		debug_cond(DLEVEL >= 2,
			   "write_test(%u,%u,ONE) : %u != %i => %i\n",
			   write_group, use_dm, *bit_chk, 0, *bit_chk != 0);
		return *bit_chk != 0x00;
	}
}

/**
 * rw_mgr_mem_calibrate_read_test_patterns() - Read back test patterns
 * @rank_bgn:	Rank number
 * @group:	Read/Write Group
 * @all_ranks:	Test all ranks
 *
 * Performs a guaranteed read on the patterns we are going to use during a
 * read test to ensure memory works.
 */
static int
rw_mgr_mem_calibrate_read_test_patterns(const u32 rank_bgn, const u32 group,
					const u32 all_ranks)
{
	const u32 addr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
			 RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	const u32 addr_offset =
			 (group * rwcfg->mem_virtual_groups_per_read_dqs) << 2;
	const u32 rank_end = all_ranks ?
				rwcfg->mem_number_of_ranks :
				(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	const u32 shift_ratio = rwcfg->mem_dq_per_read_dqs /
				rwcfg->mem_virtual_groups_per_read_dqs;
	const u32 correct_mask_vg = param->read_correct_mask_vg;

	u32 tmp_bit_chk, base_rw_mgr, bit_chk;
	int vg, r;
	int ret = 0;

	bit_chk = param->read_correct_mask;

	for (r = rank_bgn; r < rank_end; r++) {
		/* Set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		/* Load up a constant bursts of read commands */
		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr0);
		writel(rwcfg->guaranteed_read,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr1);
		writel(rwcfg->guaranteed_read_cont,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		tmp_bit_chk = 0;
		for (vg = rwcfg->mem_virtual_groups_per_read_dqs - 1;
		     vg >= 0; vg--) {
			/* Reset the FIFOs to get pointers to known state. */
			writel(0, &phy_mgr_cmd->fifo_reset);
			writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				  RW_MGR_RESET_READ_DATAPATH_OFFSET);
			writel(rwcfg->guaranteed_read,
			       addr + addr_offset + (vg << 2));

			base_rw_mgr = readl(SDR_PHYGRP_RWMGRGRP_ADDRESS);
			tmp_bit_chk <<= shift_ratio;
			tmp_bit_chk |= correct_mask_vg & ~base_rw_mgr;
		}

		bit_chk &= tmp_bit_chk;
	}

	writel(rwcfg->clear_dqs_enable, addr + (group << 2));

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);

	if (bit_chk != param->read_correct_mask)
		ret = -EIO;

	debug_cond(DLEVEL >= 1,
		   "%s:%d test_load_patterns(%u,ALL) => (%u == %u) => %i\n",
		   __func__, __LINE__, group, bit_chk,
		   param->read_correct_mask, ret);

	return ret;
}

/**
 * rw_mgr_mem_calibrate_read_load_patterns() - Load up the patterns for read test
 * @rank_bgn:	Rank number
 * @all_ranks:	Test all ranks
 *
 * Load up the patterns we are going to use during a read test.
 */
static void rw_mgr_mem_calibrate_read_load_patterns(const u32 rank_bgn,
						    const int all_ranks)
{
	const u32 rank_end = all_ranks ?
			rwcfg->mem_number_of_ranks :
			(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	u32 r;

	debug("%s:%d\n", __func__, __LINE__);

	for (r = rank_bgn; r < rank_end; r++) {
		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		/* Load up a constant bursts */
		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr0);

		writel(rwcfg->guaranteed_write_wait0,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr1);

		writel(rwcfg->guaranteed_write_wait1,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(0x04, &sdr_rw_load_mgr_regs->load_cntr2);

		writel(rwcfg->guaranteed_write_wait2,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add2);

		writel(0x04, &sdr_rw_load_mgr_regs->load_cntr3);

		writel(rwcfg->guaranteed_write_wait3,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add3);

		writel(rwcfg->guaranteed_write, SDR_PHYGRP_RWMGRGRP_ADDRESS |
						RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	}

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
}

/**
 * rw_mgr_mem_calibrate_read_test() - Perform READ test on single rank
 * @rank_bgn:		Rank number
 * @group:		Read/Write group
 * @num_tries:		Number of retries of the test
 * @all_correct:	All bits must be correct in the mask
 * @bit_chk:		Resulting bit mask after the test
 * @all_groups:		Test all R/W groups
 * @all_ranks:		Test all ranks
 *
 * Try a read and see if it returns correct data back. Test has dummy reads
 * inserted into the mix used to align DQS enable. Test has more thorough
 * checks than the regular read test.
 */
static int
rw_mgr_mem_calibrate_read_test(const u32 rank_bgn, const u32 group,
			       const u32 num_tries, const u32 all_correct,
			       u32 *bit_chk,
			       const u32 all_groups, const u32 all_ranks)
{
	const u32 rank_end = all_ranks ? rwcfg->mem_number_of_ranks :
		(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	const u32 quick_read_mode =
		((STATIC_CALIB_STEPS & CALIB_SKIP_DELAY_SWEEPS) &&
		 misccfg->enable_super_quick_calibration);
	u32 correct_mask_vg = param->read_correct_mask_vg;
	u32 tmp_bit_chk;
	u32 base_rw_mgr;
	u32 addr;

	int r, vg, ret;

	*bit_chk = param->read_correct_mask;

	for (r = rank_bgn; r < rank_end; r++) {
		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		writel(0x10, &sdr_rw_load_mgr_regs->load_cntr1);

		writel(rwcfg->read_b2b_wait1,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(0x10, &sdr_rw_load_mgr_regs->load_cntr2);
		writel(rwcfg->read_b2b_wait2,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add2);

		if (quick_read_mode)
			writel(0x1, &sdr_rw_load_mgr_regs->load_cntr0);
			/* need at least two (1+1) reads to capture failures */
		else if (all_groups)
			writel(0x06, &sdr_rw_load_mgr_regs->load_cntr0);
		else
			writel(0x32, &sdr_rw_load_mgr_regs->load_cntr0);

		writel(rwcfg->read_b2b,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add0);
		if (all_groups)
			writel(rwcfg->mem_if_read_dqs_width *
			       rwcfg->mem_virtual_groups_per_read_dqs - 1,
			       &sdr_rw_load_mgr_regs->load_cntr3);
		else
			writel(0x0, &sdr_rw_load_mgr_regs->load_cntr3);

		writel(rwcfg->read_b2b,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add3);

		tmp_bit_chk = 0;
		for (vg = rwcfg->mem_virtual_groups_per_read_dqs - 1; vg >= 0;
		     vg--) {
			/* Reset the FIFOs to get pointers to known state. */
			writel(0, &phy_mgr_cmd->fifo_reset);
			writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				  RW_MGR_RESET_READ_DATAPATH_OFFSET);

			if (all_groups) {
				addr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
				       RW_MGR_RUN_ALL_GROUPS_OFFSET;
			} else {
				addr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
				       RW_MGR_RUN_SINGLE_GROUP_OFFSET;
			}

			writel(rwcfg->read_b2b, addr +
			       ((group *
				 rwcfg->mem_virtual_groups_per_read_dqs +
				 vg) << 2));

			base_rw_mgr = readl(SDR_PHYGRP_RWMGRGRP_ADDRESS);
			tmp_bit_chk <<= rwcfg->mem_dq_per_read_dqs /
					rwcfg->mem_virtual_groups_per_read_dqs;
			tmp_bit_chk |= correct_mask_vg & ~(base_rw_mgr);
		}

		*bit_chk &= tmp_bit_chk;
	}

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	writel(rwcfg->clear_dqs_enable, addr + (group << 2));

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);

	if (all_correct) {
		ret = (*bit_chk == param->read_correct_mask);
		debug_cond(DLEVEL >= 2,
			   "%s:%d read_test(%u,ALL,%u) => (%u == %u) => %i\n",
			   __func__, __LINE__, group, all_groups, *bit_chk,
			   param->read_correct_mask, ret);
	} else	{
		ret = (*bit_chk != 0x00);
		debug_cond(DLEVEL >= 2,
			   "%s:%d read_test(%u,ONE,%u) => (%u != %u) => %i\n",
			   __func__, __LINE__, group, all_groups, *bit_chk,
			   0, ret);
	}

	return ret;
}

/**
 * rw_mgr_mem_calibrate_read_test_all_ranks() - Perform READ test on all ranks
 * @grp:		Read/Write group
 * @num_tries:		Number of retries of the test
 * @all_correct:	All bits must be correct in the mask
 * @all_groups:		Test all R/W groups
 *
 * Perform a READ test across all memory ranks.
 */
static int
rw_mgr_mem_calibrate_read_test_all_ranks(const u32 grp, const u32 num_tries,
					 const u32 all_correct,
					 const u32 all_groups)
{
	u32 bit_chk;
	return rw_mgr_mem_calibrate_read_test(0, grp, num_tries, all_correct,
					      &bit_chk, all_groups, 1);
}

/**
 * rw_mgr_incr_vfifo() - Increase VFIFO value
 * @grp:	Read/Write group
 *
 * Increase VFIFO value.
 */
static void rw_mgr_incr_vfifo(const u32 grp)
{
	writel(grp, &phy_mgr_cmd->inc_vfifo_hard_phy);
}

/**
 * rw_mgr_decr_vfifo() - Decrease VFIFO value
 * @grp:	Read/Write group
 *
 * Decrease VFIFO value.
 */
static void rw_mgr_decr_vfifo(const u32 grp)
{
	u32 i;

	for (i = 0; i < misccfg->read_valid_fifo_size - 1; i++)
		rw_mgr_incr_vfifo(grp);
}

/**
 * find_vfifo_failing_read() - Push VFIFO to get a failing read
 * @grp:	Read/Write group
 *
 * Push VFIFO until a failing read happens.
 */
static int find_vfifo_failing_read(const u32 grp)
{
	u32 v, ret, fail_cnt = 0;

	for (v = 0; v < misccfg->read_valid_fifo_size; v++) {
		debug_cond(DLEVEL >= 2, "%s:%d: vfifo %u\n",
			   __func__, __LINE__, v);
		ret = rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
						PASS_ONE_BIT, 0);
		if (!ret) {
			fail_cnt++;

			if (fail_cnt == 2)
				return v;
		}

		/* Fiddle with FIFO. */
		rw_mgr_incr_vfifo(grp);
	}

	/* No failing read found! Something must have gone wrong. */
	debug_cond(DLEVEL >= 2, "%s:%d: vfifo failed\n", __func__, __LINE__);
	return 0;
}

/**
 * sdr_find_phase_delay() - Find DQS enable phase or delay
 * @working:	If 1, look for working phase/delay, if 0, look for non-working
 * @delay:	If 1, look for delay, if 0, look for phase
 * @grp:	Read/Write group
 * @work:	Working window position
 * @work_inc:	Working window increment
 * @pd:		DQS Phase/Delay Iterator
 *
 * Find working or non-working DQS enable phase setting.
 */
static int sdr_find_phase_delay(int working, int delay, const u32 grp,
				u32 *work, const u32 work_inc, u32 *pd)
{
	const u32 max = delay ? iocfg->dqs_en_delay_max :
				iocfg->dqs_en_phase_max;
	u32 ret;

	for (; *pd <= max; (*pd)++) {
		if (delay)
			scc_mgr_set_dqs_en_delay_all_ranks(grp, *pd);
		else
			scc_mgr_set_dqs_en_phase_all_ranks(grp, *pd);

		ret = rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
					PASS_ONE_BIT, 0);
		if (!working)
			ret = !ret;

		if (ret)
			return 0;

		if (work)
			*work += work_inc;
	}

	return -EINVAL;
}
/**
 * sdr_find_phase() - Find DQS enable phase
 * @working:	If 1, look for working phase, if 0, look for non-working phase
 * @grp:	Read/Write group
 * @work:	Working window position
 * @i:		Iterator
 * @p:		DQS Phase Iterator
 *
 * Find working or non-working DQS enable phase setting.
 */
static int sdr_find_phase(int working, const u32 grp, u32 *work,
			  u32 *i, u32 *p)
{
	const u32 end = misccfg->read_valid_fifo_size + (working ? 0 : 1);
	int ret;

	for (; *i < end; (*i)++) {
		if (working)
			*p = 0;

		ret = sdr_find_phase_delay(working, 0, grp, work,
					   iocfg->delay_per_opa_tap, p);
		if (!ret)
			return 0;

		if (*p > iocfg->dqs_en_phase_max) {
			/* Fiddle with FIFO. */
			rw_mgr_incr_vfifo(grp);
			if (!working)
				*p = 0;
		}
	}

	return -EINVAL;
}

/**
 * sdr_working_phase() - Find working DQS enable phase
 * @grp:	Read/Write group
 * @work_bgn:	Working window start position
 * @d:		dtaps output value
 * @p:		DQS Phase Iterator
 * @i:		Iterator
 *
 * Find working DQS enable phase setting.
 */
static int sdr_working_phase(const u32 grp, u32 *work_bgn, u32 *d,
			     u32 *p, u32 *i)
{
	const u32 dtaps_per_ptap = iocfg->delay_per_opa_tap /
				   iocfg->delay_per_dqs_en_dchain_tap;
	int ret;

	*work_bgn = 0;

	for (*d = 0; *d <= dtaps_per_ptap; (*d)++) {
		*i = 0;
		scc_mgr_set_dqs_en_delay_all_ranks(grp, *d);
		ret = sdr_find_phase(1, grp, work_bgn, i, p);
		if (!ret)
			return 0;
		*work_bgn += iocfg->delay_per_dqs_en_dchain_tap;
	}

	/* Cannot find working solution */
	debug_cond(DLEVEL >= 2, "%s:%d find_dqs_en_phase: no vfifo/ptap/dtap\n",
		   __func__, __LINE__);
	return -EINVAL;
}

/**
 * sdr_backup_phase() - Find DQS enable backup phase
 * @grp:	Read/Write group
 * @work_bgn:	Working window start position
 * @p:		DQS Phase Iterator
 *
 * Find DQS enable backup phase setting.
 */
static void sdr_backup_phase(const u32 grp, u32 *work_bgn, u32 *p)
{
	u32 tmp_delay, d;
	int ret;

	/* Special case code for backing up a phase */
	if (*p == 0) {
		*p = iocfg->dqs_en_phase_max;
		rw_mgr_decr_vfifo(grp);
	} else {
		(*p)--;
	}
	tmp_delay = *work_bgn - iocfg->delay_per_opa_tap;
	scc_mgr_set_dqs_en_phase_all_ranks(grp, *p);

	for (d = 0; d <= iocfg->dqs_en_delay_max && tmp_delay < *work_bgn;
	     d++) {
		scc_mgr_set_dqs_en_delay_all_ranks(grp, d);

		ret = rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
					PASS_ONE_BIT, 0);
		if (ret) {
			*work_bgn = tmp_delay;
			break;
		}

		tmp_delay += iocfg->delay_per_dqs_en_dchain_tap;
	}

	/* Restore VFIFO to old state before we decremented it (if needed). */
	(*p)++;
	if (*p > iocfg->dqs_en_phase_max) {
		*p = 0;
		rw_mgr_incr_vfifo(grp);
	}

	scc_mgr_set_dqs_en_delay_all_ranks(grp, 0);
}

/**
 * sdr_nonworking_phase() - Find non-working DQS enable phase
 * @grp:	Read/Write group
 * @work_end:	Working window end position
 * @p:		DQS Phase Iterator
 * @i:		Iterator
 *
 * Find non-working DQS enable phase setting.
 */
static int sdr_nonworking_phase(const u32 grp, u32 *work_end, u32 *p, u32 *i)
{
	int ret;

	(*p)++;
	*work_end += iocfg->delay_per_opa_tap;
	if (*p > iocfg->dqs_en_phase_max) {
		/* Fiddle with FIFO. */
		*p = 0;
		rw_mgr_incr_vfifo(grp);
	}

	ret = sdr_find_phase(0, grp, work_end, i, p);
	if (ret) {
		/* Cannot see edge of failing read. */
		debug_cond(DLEVEL >= 2, "%s:%d: end: failed\n",
			   __func__, __LINE__);
	}

	return ret;
}

/**
 * sdr_find_window_center() - Find center of the working DQS window.
 * @grp:	Read/Write group
 * @work_bgn:	First working settings
 * @work_end:	Last working settings
 *
 * Find center of the working DQS enable window.
 */
static int sdr_find_window_center(const u32 grp, const u32 work_bgn,
				  const u32 work_end)
{
	u32 work_mid;
	int tmp_delay = 0;
	int i, p, d;

	work_mid = (work_bgn + work_end) / 2;

	debug_cond(DLEVEL >= 2, "work_bgn=%d work_end=%d work_mid=%d\n",
		   work_bgn, work_end, work_mid);
	/* Get the middle delay to be less than a VFIFO delay */
	tmp_delay = (iocfg->dqs_en_phase_max + 1) * iocfg->delay_per_opa_tap;

	debug_cond(DLEVEL >= 2, "vfifo ptap delay %d\n", tmp_delay);
	work_mid %= tmp_delay;
	debug_cond(DLEVEL >= 2, "new work_mid %d\n", work_mid);

	tmp_delay = rounddown(work_mid, iocfg->delay_per_opa_tap);
	if (tmp_delay > iocfg->dqs_en_phase_max * iocfg->delay_per_opa_tap)
		tmp_delay = iocfg->dqs_en_phase_max * iocfg->delay_per_opa_tap;
	p = tmp_delay / iocfg->delay_per_opa_tap;

	debug_cond(DLEVEL >= 2, "new p %d, tmp_delay=%d\n", p, tmp_delay);

	d = DIV_ROUND_UP(work_mid - tmp_delay,
			 iocfg->delay_per_dqs_en_dchain_tap);
	if (d > iocfg->dqs_en_delay_max)
		d = iocfg->dqs_en_delay_max;
	tmp_delay += d * iocfg->delay_per_dqs_en_dchain_tap;

	debug_cond(DLEVEL >= 2, "new d %d, tmp_delay=%d\n", d, tmp_delay);

	scc_mgr_set_dqs_en_phase_all_ranks(grp, p);
	scc_mgr_set_dqs_en_delay_all_ranks(grp, d);

	/*
	 * push vfifo until we can successfully calibrate. We can do this
	 * because the largest possible margin in 1 VFIFO cycle.
	 */
	for (i = 0; i < misccfg->read_valid_fifo_size; i++) {
		debug_cond(DLEVEL >= 2, "find_dqs_en_phase: center\n");
		if (rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
							     PASS_ONE_BIT,
							     0)) {
			debug_cond(DLEVEL >= 2,
				   "%s:%d center: found: ptap=%u dtap=%u\n",
				   __func__, __LINE__, p, d);
			return 0;
		}

		/* Fiddle with FIFO. */
		rw_mgr_incr_vfifo(grp);
	}

	debug_cond(DLEVEL >= 2, "%s:%d center: failed.\n",
		   __func__, __LINE__);
	return -EINVAL;
}

/**
 * rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase() - Find a good DQS enable to use
 * @grp:	Read/Write Group
 *
 * Find a good DQS enable to use.
 */
static int rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase(const u32 grp)
{
	u32 d, p, i;
	u32 dtaps_per_ptap;
	u32 work_bgn, work_end;
	u32 found_passing_read, found_failing_read = 0, initial_failing_dtap;
	int ret;

	debug("%s:%d %u\n", __func__, __LINE__, grp);

	reg_file_set_sub_stage(CAL_SUBSTAGE_VFIFO_CENTER);

	scc_mgr_set_dqs_en_delay_all_ranks(grp, 0);
	scc_mgr_set_dqs_en_phase_all_ranks(grp, 0);

	/* Step 0: Determine number of delay taps for each phase tap. */
	dtaps_per_ptap = iocfg->delay_per_opa_tap /
			 iocfg->delay_per_dqs_en_dchain_tap;

	/* Step 1: First push vfifo until we get a failing read. */
	find_vfifo_failing_read(grp);

	/* Step 2: Find first working phase, increment in ptaps. */
	work_bgn = 0;
	ret = sdr_working_phase(grp, &work_bgn, &d, &p, &i);
	if (ret)
		return ret;

	work_end = work_bgn;

	/*
	 * If d is 0 then the working window covers a phase tap and we can
	 * follow the old procedure. Otherwise, we've found the beginning
	 * and we need to increment the dtaps until we find the end.
	 */
	if (d == 0) {
		/*
		 * Step 3a: If we have room, back off by one and
		 *          increment in dtaps.
		 */
		sdr_backup_phase(grp, &work_bgn, &p);

		/*
		 * Step 4a: go forward from working phase to non working
		 * phase, increment in ptaps.
		 */
		ret = sdr_nonworking_phase(grp, &work_end, &p, &i);
		if (ret)
			return ret;

		/* Step 5a: Back off one from last, increment in dtaps. */

		/* Special case code for backing up a phase */
		if (p == 0) {
			p = iocfg->dqs_en_phase_max;
			rw_mgr_decr_vfifo(grp);
		} else {
			p = p - 1;
		}

		work_end -= iocfg->delay_per_opa_tap;
		scc_mgr_set_dqs_en_phase_all_ranks(grp, p);

		d = 0;

		debug_cond(DLEVEL >= 2, "%s:%d p: ptap=%u\n",
			   __func__, __LINE__, p);
	}

	/* The dtap increment to find the failing edge is done here. */
	sdr_find_phase_delay(0, 1, grp, &work_end,
			     iocfg->delay_per_dqs_en_dchain_tap, &d);

	/* Go back to working dtap */
	if (d != 0)
		work_end -= iocfg->delay_per_dqs_en_dchain_tap;

	debug_cond(DLEVEL >= 2,
		   "%s:%d p/d: ptap=%u dtap=%u end=%u\n",
		   __func__, __LINE__, p, d - 1, work_end);

	if (work_end < work_bgn) {
		/* nil range */
		debug_cond(DLEVEL >= 2, "%s:%d end-2: failed\n",
			   __func__, __LINE__);
		return -EINVAL;
	}

	debug_cond(DLEVEL >= 2, "%s:%d found range [%u,%u]\n",
		   __func__, __LINE__, work_bgn, work_end);

	/*
	 * We need to calculate the number of dtaps that equal a ptap.
	 * To do that we'll back up a ptap and re-find the edge of the
	 * window using dtaps
	 */
	debug_cond(DLEVEL >= 2, "%s:%d calculate dtaps_per_ptap for tracking\n",
		   __func__, __LINE__);

	/* Special case code for backing up a phase */
	if (p == 0) {
		p = iocfg->dqs_en_phase_max;
		rw_mgr_decr_vfifo(grp);
		debug_cond(DLEVEL >= 2, "%s:%d backedup cycle/phase: p=%u\n",
			   __func__, __LINE__, p);
	} else {
		p = p - 1;
		debug_cond(DLEVEL >= 2, "%s:%d backedup phase only: p=%u",
			   __func__, __LINE__, p);
	}

	scc_mgr_set_dqs_en_phase_all_ranks(grp, p);

	/*
	 * Increase dtap until we first see a passing read (in case the
	 * window is smaller than a ptap), and then a failing read to
	 * mark the edge of the window again.
	 */

	/* Find a passing read. */
	debug_cond(DLEVEL >= 2, "%s:%d find passing read\n",
		   __func__, __LINE__);

	initial_failing_dtap = d;

	found_passing_read = !sdr_find_phase_delay(1, 1, grp, NULL, 0, &d);
	if (found_passing_read) {
		/* Find a failing read. */
		debug_cond(DLEVEL >= 2, "%s:%d find failing read\n",
			   __func__, __LINE__);
		d++;
		found_failing_read = !sdr_find_phase_delay(0, 1, grp, NULL, 0,
							   &d);
	} else {
		debug_cond(DLEVEL >= 1,
			   "%s:%d failed to calculate dtaps per ptap. Fall back on static value\n",
			   __func__, __LINE__);
	}

	/*
	 * The dynamically calculated dtaps_per_ptap is only valid if we
	 * found a passing/failing read. If we didn't, it means d hit the max
	 * (iocfg->dqs_en_delay_max). Otherwise, dtaps_per_ptap retains its
	 * statically calculated value.
	 */
	if (found_passing_read && found_failing_read)
		dtaps_per_ptap = d - initial_failing_dtap;

	writel(dtaps_per_ptap, &sdr_reg_file->dtaps_per_ptap);
	debug_cond(DLEVEL >= 2, "%s:%d dtaps_per_ptap=%u - %u = %u",
		   __func__, __LINE__, d, initial_failing_dtap, dtaps_per_ptap);

	/* Step 6: Find the centre of the window. */
	ret = sdr_find_window_center(grp, work_bgn, work_end);

	return ret;
}

/**
 * search_stop_check() - Check if the detected edge is valid
 * @write:		Perform read (Stage 2) or write (Stage 3) calibration
 * @d:			DQS delay
 * @rank_bgn:		Rank number
 * @write_group:	Write Group
 * @read_group:		Read Group
 * @bit_chk:		Resulting bit mask after the test
 * @sticky_bit_chk:	Resulting sticky bit mask after the test
 * @use_read_test:	Perform read test
 *
 * Test if the found edge is valid.
 */
static u32 search_stop_check(const int write, const int d, const int rank_bgn,
			     const u32 write_group, const u32 read_group,
			     u32 *bit_chk, u32 *sticky_bit_chk,
			     const u32 use_read_test)
{
	const u32 ratio = rwcfg->mem_if_read_dqs_width /
			  rwcfg->mem_if_write_dqs_width;
	const u32 correct_mask = write ? param->write_correct_mask :
					 param->read_correct_mask;
	const u32 per_dqs = write ? rwcfg->mem_dq_per_write_dqs :
				    rwcfg->mem_dq_per_read_dqs;
	u32 ret;
	/*
	 * Stop searching when the read test doesn't pass AND when
	 * we've seen a passing read on every bit.
	 */
	if (write) {			/* WRITE-ONLY */
		ret = !rw_mgr_mem_calibrate_write_test(rank_bgn, write_group,
							 0, PASS_ONE_BIT,
							 bit_chk, 0);
	} else if (use_read_test) {	/* READ-ONLY */
		ret = !rw_mgr_mem_calibrate_read_test(rank_bgn, read_group,
							NUM_READ_PB_TESTS,
							PASS_ONE_BIT, bit_chk,
							0, 0);
	} else {			/* READ-ONLY */
		rw_mgr_mem_calibrate_write_test(rank_bgn, write_group, 0,
						PASS_ONE_BIT, bit_chk, 0);
		*bit_chk = *bit_chk >> (per_dqs *
			(read_group - (write_group * ratio)));
		ret = (*bit_chk == 0);
	}
	*sticky_bit_chk = *sticky_bit_chk | *bit_chk;
	ret = ret && (*sticky_bit_chk == correct_mask);
	debug_cond(DLEVEL >= 2,
		   "%s:%d center(left): dtap=%u => %u == %u && %u",
		   __func__, __LINE__, d,
		   *sticky_bit_chk, correct_mask, ret);
	return ret;
}

/**
 * search_left_edge() - Find left edge of DQ/DQS working phase
 * @write:		Perform read (Stage 2) or write (Stage 3) calibration
 * @rank_bgn:		Rank number
 * @write_group:	Write Group
 * @read_group:		Read Group
 * @test_bgn:		Rank number to begin the test
 * @sticky_bit_chk:	Resulting sticky bit mask after the test
 * @left_edge:		Left edge of the DQ/DQS phase
 * @right_edge:		Right edge of the DQ/DQS phase
 * @use_read_test:	Perform read test
 *
 * Find left edge of DQ/DQS working phase.
 */
static void search_left_edge(const int write, const int rank_bgn,
	const u32 write_group, const u32 read_group, const u32 test_bgn,
	u32 *sticky_bit_chk,
	int *left_edge, int *right_edge, const u32 use_read_test)
{
	const u32 delay_max = write ? iocfg->io_out1_delay_max :
				      iocfg->io_in_delay_max;
	const u32 dqs_max = write ? iocfg->io_out1_delay_max :
				    iocfg->dqs_in_delay_max;
	const u32 per_dqs = write ? rwcfg->mem_dq_per_write_dqs :
				    rwcfg->mem_dq_per_read_dqs;
	u32 stop, bit_chk;
	int i, d;

	for (d = 0; d <= dqs_max; d++) {
		if (write)
			scc_mgr_apply_group_dq_out1_delay(d);
		else
			scc_mgr_apply_group_dq_in_delay(test_bgn, d);

		writel(0, &sdr_scc_mgr->update);

		stop = search_stop_check(write, d, rank_bgn, write_group,
					 read_group, &bit_chk, sticky_bit_chk,
					 use_read_test);
		if (stop == 1)
			break;

		/* stop != 1 */
		for (i = 0; i < per_dqs; i++) {
			if (bit_chk & 1) {
				/*
				 * Remember a passing test as
				 * the left_edge.
				 */
				left_edge[i] = d;
			} else {
				/*
				 * If a left edge has not been seen
				 * yet, then a future passing test
				 * will mark this edge as the right
				 * edge.
				 */
				if (left_edge[i] == delay_max + 1)
					right_edge[i] = -(d + 1);
			}
			bit_chk >>= 1;
		}
	}

	/* Reset DQ delay chains to 0 */
	if (write)
		scc_mgr_apply_group_dq_out1_delay(0);
	else
		scc_mgr_apply_group_dq_in_delay(test_bgn, 0);

	*sticky_bit_chk = 0;
	for (i = per_dqs - 1; i >= 0; i--) {
		debug_cond(DLEVEL >= 2,
			   "%s:%d vfifo_center: left_edge[%u]: %d right_edge[%u]: %d\n",
			   __func__, __LINE__, i, left_edge[i],
			   i, right_edge[i]);

		/*
		 * Check for cases where we haven't found the left edge,
		 * which makes our assignment of the the right edge invalid.
		 * Reset it to the illegal value.
		 */
		if ((left_edge[i] == delay_max + 1) &&
		    (right_edge[i] != delay_max + 1)) {
			right_edge[i] = delay_max + 1;
			debug_cond(DLEVEL >= 2,
				   "%s:%d vfifo_center: reset right_edge[%u]: %d\n",
				   __func__, __LINE__, i, right_edge[i]);
		}

		/*
		 * Reset sticky bit
		 * READ: except for bits where we have seen both
		 *       the left and right edge.
		 * WRITE: except for bits where we have seen the
		 *        left edge.
		 */
		*sticky_bit_chk <<= 1;
		if (write) {
			if (left_edge[i] != delay_max + 1)
				*sticky_bit_chk |= 1;
		} else {
			if ((left_edge[i] != delay_max + 1) &&
			    (right_edge[i] != delay_max + 1))
				*sticky_bit_chk |= 1;
		}
	}
}

/**
 * search_right_edge() - Find right edge of DQ/DQS working phase
 * @write:		Perform read (Stage 2) or write (Stage 3) calibration
 * @rank_bgn:		Rank number
 * @write_group:	Write Group
 * @read_group:		Read Group
 * @start_dqs:		DQS start phase
 * @start_dqs_en:	DQS enable start phase
 * @sticky_bit_chk:	Resulting sticky bit mask after the test
 * @left_edge:		Left edge of the DQ/DQS phase
 * @right_edge:		Right edge of the DQ/DQS phase
 * @use_read_test:	Perform read test
 *
 * Find right edge of DQ/DQS working phase.
 */
static int search_right_edge(const int write, const int rank_bgn,
	const u32 write_group, const u32 read_group,
	const int start_dqs, const int start_dqs_en,
	u32 *sticky_bit_chk,
	int *left_edge, int *right_edge, const u32 use_read_test)
{
	const u32 delay_max = write ? iocfg->io_out1_delay_max :
				      iocfg->io_in_delay_max;
	const u32 dqs_max = write ? iocfg->io_out1_delay_max :
				    iocfg->dqs_in_delay_max;
	const u32 per_dqs = write ? rwcfg->mem_dq_per_write_dqs :
				    rwcfg->mem_dq_per_read_dqs;
	u32 stop, bit_chk;
	int i, d;

	for (d = 0; d <= dqs_max - start_dqs; d++) {
		if (write) {	/* WRITE-ONLY */
			scc_mgr_apply_group_dqs_io_and_oct_out1(write_group,
								d + start_dqs);
		} else {	/* READ-ONLY */
			scc_mgr_set_dqs_bus_in_delay(read_group, d + start_dqs);
			if (iocfg->shift_dqs_en_when_shift_dqs) {
				u32 delay = d + start_dqs_en;
				if (delay > iocfg->dqs_en_delay_max)
					delay = iocfg->dqs_en_delay_max;
				scc_mgr_set_dqs_en_delay(read_group, delay);
			}
			scc_mgr_load_dqs(read_group);
		}

		writel(0, &sdr_scc_mgr->update);

		stop = search_stop_check(write, d, rank_bgn, write_group,
					 read_group, &bit_chk, sticky_bit_chk,
					 use_read_test);
		if (stop == 1) {
			if (write && (d == 0)) {	/* WRITE-ONLY */
				for (i = 0; i < rwcfg->mem_dq_per_write_dqs;
				     i++) {
					/*
					 * d = 0 failed, but it passed when
					 * testing the left edge, so it must be
					 * marginal, set it to -1
					 */
					if (right_edge[i] == delay_max + 1 &&
					    left_edge[i] != delay_max + 1)
						right_edge[i] = -1;
				}
			}
			break;
		}

		/* stop != 1 */
		for (i = 0; i < per_dqs; i++) {
			if (bit_chk & 1) {
				/*
				 * Remember a passing test as
				 * the right_edge.
				 */
				right_edge[i] = d;
			} else {
				if (d != 0) {
					/*
					 * If a right edge has not
					 * been seen yet, then a future
					 * passing test will mark this
					 * edge as the left edge.
					 */
					if (right_edge[i] == delay_max + 1)
						left_edge[i] = -(d + 1);
				} else {
					/*
					 * d = 0 failed, but it passed
					 * when testing the left edge,
					 * so it must be marginal, set
					 * it to -1
					 */
					if (right_edge[i] == delay_max + 1 &&
					    left_edge[i] != delay_max + 1)
						right_edge[i] = -1;
					/*
					 * If a right edge has not been
					 * seen yet, then a future
					 * passing test will mark this
					 * edge as the left edge.
					 */
					else if (right_edge[i] == delay_max + 1)
						left_edge[i] = -(d + 1);
				}
			}

			debug_cond(DLEVEL >= 2, "%s:%d center[r,d=%u]: ",
				   __func__, __LINE__, d);
			debug_cond(DLEVEL >= 2,
				   "bit_chk_test=%i left_edge[%u]: %d ",
				   bit_chk & 1, i, left_edge[i]);
			debug_cond(DLEVEL >= 2, "right_edge[%u]: %d\n", i,
				   right_edge[i]);
			bit_chk >>= 1;
		}
	}

	/* Check that all bits have a window */
	for (i = 0; i < per_dqs; i++) {
		debug_cond(DLEVEL >= 2,
			   "%s:%d write_center: left_edge[%u]: %d right_edge[%u]: %d",
			   __func__, __LINE__, i, left_edge[i],
			   i, right_edge[i]);
		if ((left_edge[i] == dqs_max + 1) ||
		    (right_edge[i] == dqs_max + 1))
			return i + 1;	/* FIXME: If we fail, retval > 0 */
	}

	return 0;
}

/**
 * get_window_mid_index() - Find the best middle setting of DQ/DQS phase
 * @write:		Perform read (Stage 2) or write (Stage 3) calibration
 * @left_edge:		Left edge of the DQ/DQS phase
 * @right_edge:		Right edge of the DQ/DQS phase
 * @mid_min:		Best DQ/DQS phase middle setting
 *
 * Find index and value of the middle of the DQ/DQS working phase.
 */
static int get_window_mid_index(const int write, int *left_edge,
				int *right_edge, int *mid_min)
{
	const u32 per_dqs = write ? rwcfg->mem_dq_per_write_dqs :
				    rwcfg->mem_dq_per_read_dqs;
	int i, mid, min_index;

	/* Find middle of window for each DQ bit */
	*mid_min = left_edge[0] - right_edge[0];
	min_index = 0;
	for (i = 1; i < per_dqs; i++) {
		mid = left_edge[i] - right_edge[i];
		if (mid < *mid_min) {
			*mid_min = mid;
			min_index = i;
		}
	}

	/*
	 * -mid_min/2 represents the amount that we need to move DQS.
	 * If mid_min is odd and positive we'll need to add one to make
	 * sure the rounding in further calculations is correct (always
	 * bias to the right), so just add 1 for all positive values.
	 */
	if (*mid_min > 0)
		(*mid_min)++;
	*mid_min = *mid_min / 2;

	debug_cond(DLEVEL >= 1, "%s:%d vfifo_center: *mid_min=%d (index=%u)\n",
		   __func__, __LINE__, *mid_min, min_index);
	return min_index;
}

/**
 * center_dq_windows() - Center the DQ/DQS windows
 * @write:		Perform read (Stage 2) or write (Stage 3) calibration
 * @left_edge:		Left edge of the DQ/DQS phase
 * @right_edge:		Right edge of the DQ/DQS phase
 * @mid_min:		Adjusted DQ/DQS phase middle setting
 * @orig_mid_min:	Original DQ/DQS phase middle setting
 * @min_index:		DQ/DQS phase middle setting index
 * @test_bgn:		Rank number to begin the test
 * @dq_margin:		Amount of shift for the DQ
 * @dqs_margin:		Amount of shift for the DQS
 *
 * Align the DQ/DQS windows in each group.
 */
static void center_dq_windows(const int write, int *left_edge, int *right_edge,
			      const int mid_min, const int orig_mid_min,
			      const int min_index, const int test_bgn,
			      int *dq_margin, int *dqs_margin)
{
	const s32 delay_max = write ? iocfg->io_out1_delay_max :
				      iocfg->io_in_delay_max;
	const s32 per_dqs = write ? rwcfg->mem_dq_per_write_dqs :
				    rwcfg->mem_dq_per_read_dqs;
	const s32 delay_off = write ? SCC_MGR_IO_OUT1_DELAY_OFFSET :
				      SCC_MGR_IO_IN_DELAY_OFFSET;
	const s32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | delay_off;

	s32 temp_dq_io_delay1;
	int shift_dq, i, p;

	/* Initialize data for export structures */
	*dqs_margin = delay_max + 1;
	*dq_margin  = delay_max + 1;

	/* add delay to bring centre of all DQ windows to the same "level" */
	for (i = 0, p = test_bgn; i < per_dqs; i++, p++) {
		/* Use values before divide by 2 to reduce round off error */
		shift_dq = (left_edge[i] - right_edge[i] -
			(left_edge[min_index] - right_edge[min_index]))/2  +
			(orig_mid_min - mid_min);

		debug_cond(DLEVEL >= 2,
			   "vfifo_center: before: shift_dq[%u]=%d\n",
			   i, shift_dq);

		temp_dq_io_delay1 = readl(addr + (i << 2));

		if (shift_dq + temp_dq_io_delay1 > delay_max)
			shift_dq = delay_max - temp_dq_io_delay1;
		else if (shift_dq + temp_dq_io_delay1 < 0)
			shift_dq = -temp_dq_io_delay1;

		debug_cond(DLEVEL >= 2,
			   "vfifo_center: after: shift_dq[%u]=%d\n",
			   i, shift_dq);

		if (write)
			scc_mgr_set_dq_out1_delay(i,
						  temp_dq_io_delay1 + shift_dq);
		else
			scc_mgr_set_dq_in_delay(p,
						temp_dq_io_delay1 + shift_dq);

		scc_mgr_load_dq(p);

		debug_cond(DLEVEL >= 2,
			   "vfifo_center: margin[%u]=[%d,%d]\n", i,
			   left_edge[i] - shift_dq + (-mid_min),
			   right_edge[i] + shift_dq - (-mid_min));

		/* To determine values for export structures */
		if (left_edge[i] - shift_dq + (-mid_min) < *dq_margin)
			*dq_margin = left_edge[i] - shift_dq + (-mid_min);

		if (right_edge[i] + shift_dq - (-mid_min) < *dqs_margin)
			*dqs_margin = right_edge[i] + shift_dq - (-mid_min);
	}
}

/**
 * rw_mgr_mem_calibrate_vfifo_center() - Per-bit deskew DQ and centering
 * @rank_bgn:		Rank number
 * @rw_group:		Read/Write Group
 * @test_bgn:		Rank at which the test begins
 * @use_read_test:	Perform a read test
 * @update_fom:		Update FOM
 *
 * Per-bit deskew DQ and centering.
 */
static int rw_mgr_mem_calibrate_vfifo_center(const u32 rank_bgn,
			const u32 rw_group, const u32 test_bgn,
			const int use_read_test, const int update_fom)
{
	const u32 addr =
		SDR_PHYGRP_SCCGRP_ADDRESS + SCC_MGR_DQS_IN_DELAY_OFFSET +
		(rw_group << 2);
	/*
	 * Store these as signed since there are comparisons with
	 * signed numbers.
	 */
	u32 sticky_bit_chk;
	int32_t left_edge[rwcfg->mem_dq_per_read_dqs];
	int32_t right_edge[rwcfg->mem_dq_per_read_dqs];
	int32_t orig_mid_min, mid_min;
	int32_t new_dqs, start_dqs, start_dqs_en = 0, final_dqs_en;
	int32_t dq_margin, dqs_margin;
	int i, min_index;
	int ret;

	debug("%s:%d: %u %u", __func__, __LINE__, rw_group, test_bgn);

	start_dqs = readl(addr);
	if (iocfg->shift_dqs_en_when_shift_dqs)
		start_dqs_en = readl(addr - iocfg->dqs_en_delay_offset);

	/* set the left and right edge of each bit to an illegal value */
	/* use (iocfg->io_in_delay_max + 1) as an illegal value */
	sticky_bit_chk = 0;
	for (i = 0; i < rwcfg->mem_dq_per_read_dqs; i++) {
		left_edge[i]  = iocfg->io_in_delay_max + 1;
		right_edge[i] = iocfg->io_in_delay_max + 1;
	}

	/* Search for the left edge of the window for each bit */
	search_left_edge(0, rank_bgn, rw_group, rw_group, test_bgn,
			 &sticky_bit_chk,
			 left_edge, right_edge, use_read_test);


	/* Search for the right edge of the window for each bit */
	ret = search_right_edge(0, rank_bgn, rw_group, rw_group,
				start_dqs, start_dqs_en,
				&sticky_bit_chk,
				left_edge, right_edge, use_read_test);
	if (ret) {
		/*
		 * Restore delay chain settings before letting the loop
		 * in rw_mgr_mem_calibrate_vfifo to retry different
		 * dqs/ck relationships.
		 */
		scc_mgr_set_dqs_bus_in_delay(rw_group, start_dqs);
		if (iocfg->shift_dqs_en_when_shift_dqs)
			scc_mgr_set_dqs_en_delay(rw_group, start_dqs_en);

		scc_mgr_load_dqs(rw_group);
		writel(0, &sdr_scc_mgr->update);

		debug_cond(DLEVEL >= 1,
			   "%s:%d vfifo_center: failed to find edge [%u]: %d %d",
			   __func__, __LINE__, i, left_edge[i], right_edge[i]);
		if (use_read_test) {
			set_failing_group_stage(rw_group *
				rwcfg->mem_dq_per_read_dqs + i,
				CAL_STAGE_VFIFO,
				CAL_SUBSTAGE_VFIFO_CENTER);
		} else {
			set_failing_group_stage(rw_group *
				rwcfg->mem_dq_per_read_dqs + i,
				CAL_STAGE_VFIFO_AFTER_WRITES,
				CAL_SUBSTAGE_VFIFO_CENTER);
		}
		return -EIO;
	}

	min_index = get_window_mid_index(0, left_edge, right_edge, &mid_min);

	/* Determine the amount we can change DQS (which is -mid_min) */
	orig_mid_min = mid_min;
	new_dqs = start_dqs - mid_min;
	if (new_dqs > iocfg->dqs_in_delay_max)
		new_dqs = iocfg->dqs_in_delay_max;
	else if (new_dqs < 0)
		new_dqs = 0;

	mid_min = start_dqs - new_dqs;
	debug_cond(DLEVEL >= 1, "vfifo_center: new mid_min=%d new_dqs=%d\n",
		   mid_min, new_dqs);

	if (iocfg->shift_dqs_en_when_shift_dqs) {
		if (start_dqs_en - mid_min > iocfg->dqs_en_delay_max)
			mid_min += start_dqs_en - mid_min -
				   iocfg->dqs_en_delay_max;
		else if (start_dqs_en - mid_min < 0)
			mid_min += start_dqs_en - mid_min;
	}
	new_dqs = start_dqs - mid_min;

	debug_cond(DLEVEL >= 1,
		   "vfifo_center: start_dqs=%d start_dqs_en=%d new_dqs=%d mid_min=%d\n",
		   start_dqs,
		   iocfg->shift_dqs_en_when_shift_dqs ? start_dqs_en : -1,
		   new_dqs, mid_min);

	/* Add delay to bring centre of all DQ windows to the same "level". */
	center_dq_windows(0, left_edge, right_edge, mid_min, orig_mid_min,
			  min_index, test_bgn, &dq_margin, &dqs_margin);

	/* Move DQS-en */
	if (iocfg->shift_dqs_en_when_shift_dqs) {
		final_dqs_en = start_dqs_en - mid_min;
		scc_mgr_set_dqs_en_delay(rw_group, final_dqs_en);
		scc_mgr_load_dqs(rw_group);
	}

	/* Move DQS */
	scc_mgr_set_dqs_bus_in_delay(rw_group, new_dqs);
	scc_mgr_load_dqs(rw_group);
	debug_cond(DLEVEL >= 2,
		   "%s:%d vfifo_center: dq_margin=%d dqs_margin=%d",
		   __func__, __LINE__, dq_margin, dqs_margin);

	/*
	 * Do not remove this line as it makes sure all of our decisions
	 * have been applied. Apply the update bit.
	 */
	writel(0, &sdr_scc_mgr->update);

	if ((dq_margin < 0) || (dqs_margin < 0))
		return -EINVAL;

	return 0;
}

/**
 * rw_mgr_mem_calibrate_guaranteed_write() - Perform guaranteed write into the device
 * @rw_group:	Read/Write Group
 * @phase:	DQ/DQS phase
 *
 * Because initially no communication ca be reliably performed with the memory
 * device, the sequencer uses a guaranteed write mechanism to write data into
 * the memory device.
 */
static int rw_mgr_mem_calibrate_guaranteed_write(const u32 rw_group,
						 const u32 phase)
{
	int ret;

	/* Set a particular DQ/DQS phase. */
	scc_mgr_set_dqdqs_output_phase_all_ranks(rw_group, phase);

	debug_cond(DLEVEL >= 1, "%s:%d guaranteed write: g=%u p=%u\n",
		   __func__, __LINE__, rw_group, phase);

	/*
	 * Altera EMI_RM 2015.05.04 :: Figure 1-25
	 * Load up the patterns used by read calibration using the
	 * current DQDQS phase.
	 */
	rw_mgr_mem_calibrate_read_load_patterns(0, 1);

	if (gbl->phy_debug_mode_flags & PHY_DEBUG_DISABLE_GUARANTEED_READ)
		return 0;

	/*
	 * Altera EMI_RM 2015.05.04 :: Figure 1-26
	 * Back-to-Back reads of the patterns used for calibration.
	 */
	ret = rw_mgr_mem_calibrate_read_test_patterns(0, rw_group, 1);
	if (ret)
		debug_cond(DLEVEL >= 1,
			   "%s:%d Guaranteed read test failed: g=%u p=%u\n",
			   __func__, __LINE__, rw_group, phase);
	return ret;
}

/**
 * rw_mgr_mem_calibrate_dqs_enable_calibration() - DQS Enable Calibration
 * @rw_group:	Read/Write Group
 * @test_bgn:	Rank at which the test begins
 *
 * DQS enable calibration ensures reliable capture of the DQ signal without
 * glitches on the DQS line.
 */
static int rw_mgr_mem_calibrate_dqs_enable_calibration(const u32 rw_group,
						       const u32 test_bgn)
{
	/*
	 * Altera EMI_RM 2015.05.04 :: Figure 1-27
	 * DQS and DQS Eanble Signal Relationships.
	 */

	/* We start at zero, so have one less dq to devide among */
	const u32 delay_step = iocfg->io_in_delay_max /
			       (rwcfg->mem_dq_per_read_dqs - 1);
	int ret;
	u32 i, p, d, r;

	debug("%s:%d (%u,%u)\n", __func__, __LINE__, rw_group, test_bgn);

	/* Try different dq_in_delays since the DQ path is shorter than DQS. */
	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		for (i = 0, p = test_bgn, d = 0;
		     i < rwcfg->mem_dq_per_read_dqs;
		     i++, p++, d += delay_step) {
			debug_cond(DLEVEL >= 1,
				   "%s:%d: g=%u r=%u i=%u p=%u d=%u\n",
				   __func__, __LINE__, rw_group, r, i, p, d);

			scc_mgr_set_dq_in_delay(p, d);
			scc_mgr_load_dq(p);
		}

		writel(0, &sdr_scc_mgr->update);
	}

	/*
	 * Try rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase across different
	 * dq_in_delay values
	 */
	ret = rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase(rw_group);

	debug_cond(DLEVEL >= 1,
		   "%s:%d: g=%u found=%u; Reseting delay chain to zero\n",
		   __func__, __LINE__, rw_group, !ret);

	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		scc_mgr_apply_group_dq_in_delay(test_bgn, 0);
		writel(0, &sdr_scc_mgr->update);
	}

	return ret;
}

/**
 * rw_mgr_mem_calibrate_dq_dqs_centering() - Centering DQ/DQS
 * @rw_group:		Read/Write Group
 * @test_bgn:		Rank at which the test begins
 * @use_read_test:	Perform a read test
 * @update_fom:		Update FOM
 *
 * The centerin DQ/DQS stage attempts to align DQ and DQS signals on reads
 * within a group.
 */
static int
rw_mgr_mem_calibrate_dq_dqs_centering(const u32 rw_group, const u32 test_bgn,
				      const int use_read_test,
				      const int update_fom)

{
	int ret, grp_calibrated;
	u32 rank_bgn, sr;

	/*
	 * Altera EMI_RM 2015.05.04 :: Figure 1-28
	 * Read per-bit deskew can be done on a per shadow register basis.
	 */
	grp_calibrated = 1;
	for (rank_bgn = 0, sr = 0;
	     rank_bgn < rwcfg->mem_number_of_ranks;
	     rank_bgn += NUM_RANKS_PER_SHADOW_REG, sr++) {
		ret = rw_mgr_mem_calibrate_vfifo_center(rank_bgn, rw_group,
							test_bgn,
							use_read_test,
							update_fom);
		if (!ret)
			continue;

		grp_calibrated = 0;
	}

	if (!grp_calibrated)
		return -EIO;

	return 0;
}

/**
 * rw_mgr_mem_calibrate_vfifo() - Calibrate the read valid prediction FIFO
 * @rw_group:		Read/Write Group
 * @test_bgn:		Rank at which the test begins
 *
 * Stage 1: Calibrate the read valid prediction FIFO.
 *
 * This function implements UniPHY calibration Stage 1, as explained in
 * detail in Altera EMI_RM 2015.05.04 , "UniPHY Calibration Stages".
 *
 * - read valid prediction will consist of finding:
 *   - DQS enable phase and DQS enable delay (DQS Enable Calibration)
 *   - DQS input phase  and DQS input delay (DQ/DQS Centering)
 *  - we also do a per-bit deskew on the DQ lines.
 */
static int rw_mgr_mem_calibrate_vfifo(const u32 rw_group, const u32 test_bgn)
{
	u32 p, d;
	u32 dtaps_per_ptap;
	u32 failed_substage;

	int ret;

	debug("%s:%d: %u %u\n", __func__, __LINE__, rw_group, test_bgn);

	/* Update info for sims */
	reg_file_set_group(rw_group);
	reg_file_set_stage(CAL_STAGE_VFIFO);
	reg_file_set_sub_stage(CAL_SUBSTAGE_GUARANTEED_READ);

	failed_substage = CAL_SUBSTAGE_GUARANTEED_READ;

	/* USER Determine number of delay taps for each phase tap. */
	dtaps_per_ptap = DIV_ROUND_UP(iocfg->delay_per_opa_tap,
				      iocfg->delay_per_dqs_en_dchain_tap) - 1;

	for (d = 0; d <= dtaps_per_ptap; d += 2) {
		/*
		 * In RLDRAMX we may be messing the delay of pins in
		 * the same write rw_group but outside of the current read
		 * the rw_group, but that's ok because we haven't calibrated
		 * output side yet.
		 */
		if (d > 0) {
			scc_mgr_apply_group_all_out_delay_add_all_ranks(
								rw_group, d);
		}

		for (p = 0; p <= iocfg->dqdqs_out_phase_max; p++) {
			/* 1) Guaranteed Write */
			ret = rw_mgr_mem_calibrate_guaranteed_write(rw_group, p);
			if (ret)
				break;

			/* 2) DQS Enable Calibration */
			ret = rw_mgr_mem_calibrate_dqs_enable_calibration(rw_group,
									  test_bgn);
			if (ret) {
				failed_substage = CAL_SUBSTAGE_DQS_EN_PHASE;
				continue;
			}

			/* 3) Centering DQ/DQS */
			/*
			 * If doing read after write calibration, do not update
			 * FOM now. Do it then.
			 */
			ret = rw_mgr_mem_calibrate_dq_dqs_centering(rw_group,
								test_bgn, 1, 0);
			if (ret) {
				failed_substage = CAL_SUBSTAGE_VFIFO_CENTER;
				continue;
			}

			/* All done. */
			goto cal_done_ok;
		}
	}

	/* Calibration Stage 1 failed. */
	set_failing_group_stage(rw_group, CAL_STAGE_VFIFO, failed_substage);
	return 0;

	/* Calibration Stage 1 completed OK. */
cal_done_ok:
	/*
	 * Reset the delay chains back to zero if they have moved > 1
	 * (check for > 1 because loop will increase d even when pass in
	 * first case).
	 */
	if (d > 2)
		scc_mgr_zero_group(rw_group, 1);

	return 1;
}

/**
 * rw_mgr_mem_calibrate_vfifo_end() - DQ/DQS Centering.
 * @rw_group:		Read/Write Group
 * @test_bgn:		Rank at which the test begins
 *
 * Stage 3: DQ/DQS Centering.
 *
 * This function implements UniPHY calibration Stage 3, as explained in
 * detail in Altera EMI_RM 2015.05.04 , "UniPHY Calibration Stages".
 */
static int rw_mgr_mem_calibrate_vfifo_end(const u32 rw_group,
					  const u32 test_bgn)
{
	int ret;

	debug("%s:%d %u %u", __func__, __LINE__, rw_group, test_bgn);

	/* Update info for sims. */
	reg_file_set_group(rw_group);
	reg_file_set_stage(CAL_STAGE_VFIFO_AFTER_WRITES);
	reg_file_set_sub_stage(CAL_SUBSTAGE_VFIFO_CENTER);

	ret = rw_mgr_mem_calibrate_dq_dqs_centering(rw_group, test_bgn, 0, 1);
	if (ret)
		set_failing_group_stage(rw_group,
					CAL_STAGE_VFIFO_AFTER_WRITES,
					CAL_SUBSTAGE_VFIFO_CENTER);
	return ret;
}

/**
 * rw_mgr_mem_calibrate_lfifo() - Minimize latency
 *
 * Stage 4: Minimize latency.
 *
 * This function implements UniPHY calibration Stage 4, as explained in
 * detail in Altera EMI_RM 2015.05.04 , "UniPHY Calibration Stages".
 * Calibrate LFIFO to find smallest read latency.
 */
static u32 rw_mgr_mem_calibrate_lfifo(void)
{
	int found_one = 0;

	debug("%s:%d\n", __func__, __LINE__);

	/* Update info for sims. */
	reg_file_set_stage(CAL_STAGE_LFIFO);
	reg_file_set_sub_stage(CAL_SUBSTAGE_READ_LATENCY);

	/* Load up the patterns used by read calibration for all ranks */
	rw_mgr_mem_calibrate_read_load_patterns(0, 1);

	do {
		writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
		debug_cond(DLEVEL >= 2, "%s:%d lfifo: read_lat=%u",
			   __func__, __LINE__, gbl->curr_read_lat);

		if (!rw_mgr_mem_calibrate_read_test_all_ranks(0, NUM_READ_TESTS,
							      PASS_ALL_BITS, 1))
			break;

		found_one = 1;
		/*
		 * Reduce read latency and see if things are
		 * working correctly.
		 */
		gbl->curr_read_lat--;
	} while (gbl->curr_read_lat > 0);

	/* Reset the fifos to get pointers to known state. */
	writel(0, &phy_mgr_cmd->fifo_reset);

	if (found_one) {
		/* Add a fudge factor to the read latency that was determined */
		gbl->curr_read_lat += 2;
		writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
		debug_cond(DLEVEL >= 2,
			   "%s:%d lfifo: success: using read_lat=%u\n",
			   __func__, __LINE__, gbl->curr_read_lat);
	} else {
		set_failing_group_stage(0xff, CAL_STAGE_LFIFO,
					CAL_SUBSTAGE_READ_LATENCY);

		debug_cond(DLEVEL >= 2,
			   "%s:%d lfifo: failed at initial read_lat=%u\n",
			   __func__, __LINE__, gbl->curr_read_lat);
	}

	return found_one;
}

/**
 * search_window() - Search for the/part of the window with DM/DQS shift
 * @search_dm:		If 1, search for the DM shift, if 0, search for DQS shift
 * @rank_bgn:		Rank number
 * @write_group:	Write Group
 * @bgn_curr:		Current window begin
 * @end_curr:		Current window end
 * @bgn_best:		Current best window begin
 * @end_best:		Current best window end
 * @win_best:		Size of the best window
 * @new_dqs:		New DQS value (only applicable if search_dm = 0).
 *
 * Search for the/part of the window with DM/DQS shift.
 */
static void search_window(const int search_dm,
			  const u32 rank_bgn, const u32 write_group,
			  int *bgn_curr, int *end_curr, int *bgn_best,
			  int *end_best, int *win_best, int new_dqs)
{
	u32 bit_chk;
	const int max = iocfg->io_out1_delay_max - new_dqs;
	int d, di;

	/* Search for the/part of the window with DM/DQS shift. */
	for (di = max; di >= 0; di -= DELTA_D) {
		if (search_dm) {
			d = di;
			scc_mgr_apply_group_dm_out1_delay(d);
		} else {
			/* For DQS, we go from 0...max */
			d = max - di;
			/*
			 * Note: This only shifts DQS, so are we limiting
			 *       ourselves to width of DQ unnecessarily.
			 */
			scc_mgr_apply_group_dqs_io_and_oct_out1(write_group,
								d + new_dqs);
		}

		writel(0, &sdr_scc_mgr->update);

		if (rw_mgr_mem_calibrate_write_test(rank_bgn, write_group, 1,
						    PASS_ALL_BITS, &bit_chk,
						    0)) {
			/* Set current end of the window. */
			*end_curr = search_dm ? -d : d;

			/*
			 * If a starting edge of our window has not been seen
			 * this is our current start of the DM window.
			 */
			if (*bgn_curr == iocfg->io_out1_delay_max + 1)
				*bgn_curr = search_dm ? -d : d;

			/*
			 * If current window is bigger than best seen.
			 * Set best seen to be current window.
			 */
			if ((*end_curr - *bgn_curr + 1) > *win_best) {
				*win_best = *end_curr - *bgn_curr + 1;
				*bgn_best = *bgn_curr;
				*end_best = *end_curr;
			}
		} else {
			/* We just saw a failing test. Reset temp edge. */
			*bgn_curr = iocfg->io_out1_delay_max + 1;
			*end_curr = iocfg->io_out1_delay_max + 1;

			/* Early exit is only applicable to DQS. */
			if (search_dm)
				continue;

			/*
			 * Early exit optimization: if the remaining delay
			 * chain space is less than already seen largest
			 * window we can exit.
			 */
			if (*win_best - 1 > iocfg->io_out1_delay_max - new_dqs - d)
				break;
		}
	}
}

/*
 * rw_mgr_mem_calibrate_writes_center() - Center all windows
 * @rank_bgn:		Rank number
 * @write_group:	Write group
 * @test_bgn:		Rank at which the test begins
 *
 * Center all windows. Do per-bit-deskew to possibly increase size of
 * certain windows.
 */
static int
rw_mgr_mem_calibrate_writes_center(const u32 rank_bgn, const u32 write_group,
				   const u32 test_bgn)
{
	int i;
	u32 sticky_bit_chk;
	u32 min_index;
	int left_edge[rwcfg->mem_dq_per_write_dqs];
	int right_edge[rwcfg->mem_dq_per_write_dqs];
	int mid;
	int mid_min, orig_mid_min;
	int new_dqs, start_dqs;
	int dq_margin, dqs_margin, dm_margin;
	int bgn_curr = iocfg->io_out1_delay_max + 1;
	int end_curr = iocfg->io_out1_delay_max + 1;
	int bgn_best = iocfg->io_out1_delay_max + 1;
	int end_best = iocfg->io_out1_delay_max + 1;
	int win_best = 0;

	int ret;

	debug("%s:%d %u %u", __func__, __LINE__, write_group, test_bgn);

	dm_margin = 0;

	start_dqs = readl((SDR_PHYGRP_SCCGRP_ADDRESS |
			  SCC_MGR_IO_OUT1_DELAY_OFFSET) +
			  (rwcfg->mem_dq_per_write_dqs << 2));

	/* Per-bit deskew. */

	/*
	 * Set the left and right edge of each bit to an illegal value.
	 * Use (iocfg->io_out1_delay_max + 1) as an illegal value.
	 */
	sticky_bit_chk = 0;
	for (i = 0; i < rwcfg->mem_dq_per_write_dqs; i++) {
		left_edge[i]  = iocfg->io_out1_delay_max + 1;
		right_edge[i] = iocfg->io_out1_delay_max + 1;
	}

	/* Search for the left edge of the window for each bit. */
	search_left_edge(1, rank_bgn, write_group, 0, test_bgn,
			 &sticky_bit_chk,
			 left_edge, right_edge, 0);

	/* Search for the right edge of the window for each bit. */
	ret = search_right_edge(1, rank_bgn, write_group, 0,
				start_dqs, 0,
				&sticky_bit_chk,
				left_edge, right_edge, 0);
	if (ret) {
		set_failing_group_stage(test_bgn + ret - 1, CAL_STAGE_WRITES,
					CAL_SUBSTAGE_WRITES_CENTER);
		return -EINVAL;
	}

	min_index = get_window_mid_index(1, left_edge, right_edge, &mid_min);

	/* Determine the amount we can change DQS (which is -mid_min). */
	orig_mid_min = mid_min;
	new_dqs = start_dqs;
	mid_min = 0;
	debug_cond(DLEVEL >= 1,
		   "%s:%d write_center: start_dqs=%d new_dqs=%d mid_min=%d\n",
		   __func__, __LINE__, start_dqs, new_dqs, mid_min);

	/* Add delay to bring centre of all DQ windows to the same "level". */
	center_dq_windows(1, left_edge, right_edge, mid_min, orig_mid_min,
			  min_index, 0, &dq_margin, &dqs_margin);

	/* Move DQS */
	scc_mgr_apply_group_dqs_io_and_oct_out1(write_group, new_dqs);
	writel(0, &sdr_scc_mgr->update);

	/* Centre DM */
	debug_cond(DLEVEL >= 2, "%s:%d write_center: DM\n", __func__, __LINE__);

	/*
	 * Set the left and right edge of each bit to an illegal value.
	 * Use (iocfg->io_out1_delay_max + 1) as an illegal value.
	 */
	left_edge[0]  = iocfg->io_out1_delay_max + 1;
	right_edge[0] = iocfg->io_out1_delay_max + 1;

	/* Search for the/part of the window with DM shift. */
	search_window(1, rank_bgn, write_group, &bgn_curr, &end_curr,
		      &bgn_best, &end_best, &win_best, 0);

	/* Reset DM delay chains to 0. */
	scc_mgr_apply_group_dm_out1_delay(0);

	/*
	 * Check to see if the current window nudges up aganist 0 delay.
	 * If so we need to continue the search by shifting DQS otherwise DQS
	 * search begins as a new search.
	 */
	if (end_curr != 0) {
		bgn_curr = iocfg->io_out1_delay_max + 1;
		end_curr = iocfg->io_out1_delay_max + 1;
	}

	/* Search for the/part of the window with DQS shifts. */
	search_window(0, rank_bgn, write_group, &bgn_curr, &end_curr,
		      &bgn_best, &end_best, &win_best, new_dqs);

	/* Assign left and right edge for cal and reporting. */
	left_edge[0] = -1 * bgn_best;
	right_edge[0] = end_best;

	debug_cond(DLEVEL >= 2, "%s:%d dm_calib: left=%d right=%d\n",
		   __func__, __LINE__, left_edge[0], right_edge[0]);

	/* Move DQS (back to orig). */
	scc_mgr_apply_group_dqs_io_and_oct_out1(write_group, new_dqs);

	/* Move DM */

	/* Find middle of window for the DM bit. */
	mid = (left_edge[0] - right_edge[0]) / 2;

	/* Only move right, since we are not moving DQS/DQ. */
	if (mid < 0)
		mid = 0;

	/* dm_marign should fail if we never find a window. */
	if (win_best == 0)
		dm_margin = -1;
	else
		dm_margin = left_edge[0] - mid;

	scc_mgr_apply_group_dm_out1_delay(mid);
	writel(0, &sdr_scc_mgr->update);

	debug_cond(DLEVEL >= 2,
		   "%s:%d dm_calib: left=%d right=%d mid=%d dm_margin=%d\n",
		   __func__, __LINE__, left_edge[0], right_edge[0],
		   mid, dm_margin);
	/* Export values. */
	gbl->fom_out += dq_margin + dqs_margin;

	debug_cond(DLEVEL >= 2,
		   "%s:%d write_center: dq_margin=%d dqs_margin=%d dm_margin=%d\n",
		   __func__, __LINE__, dq_margin, dqs_margin, dm_margin);

	/*
	 * Do not remove this line as it makes sure all of our
	 * decisions have been applied.
	 */
	writel(0, &sdr_scc_mgr->update);

	if ((dq_margin < 0) || (dqs_margin < 0) || (dm_margin < 0))
		return -EINVAL;

	return 0;
}

/**
 * rw_mgr_mem_calibrate_writes() - Write Calibration Part One
 * @rank_bgn:		Rank number
 * @group:		Read/Write Group
 * @test_bgn:		Rank at which the test begins
 *
 * Stage 2: Write Calibration Part One.
 *
 * This function implements UniPHY calibration Stage 2, as explained in
 * detail in Altera EMI_RM 2015.05.04 , "UniPHY Calibration Stages".
 */
static int rw_mgr_mem_calibrate_writes(const u32 rank_bgn, const u32 group,
				       const u32 test_bgn)
{
	int ret;

	/* Update info for sims */
	debug("%s:%d %u %u\n", __func__, __LINE__, group, test_bgn);

	reg_file_set_group(group);
	reg_file_set_stage(CAL_STAGE_WRITES);
	reg_file_set_sub_stage(CAL_SUBSTAGE_WRITES_CENTER);

	ret = rw_mgr_mem_calibrate_writes_center(rank_bgn, group, test_bgn);
	if (ret)
		set_failing_group_stage(group, CAL_STAGE_WRITES,
					CAL_SUBSTAGE_WRITES_CENTER);

	return ret;
}

/**
 * mem_precharge_and_activate() - Precharge all banks and activate
 *
 * Precharge all banks and activate row 0 in bank "000..." and bank "111...".
 */
static void mem_precharge_and_activate(void)
{
	int r;

	for (r = 0; r < rwcfg->mem_number_of_ranks; r++) {
		/* Set rank. */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_OFF);

		/* Precharge all banks. */
		writel(rwcfg->precharge_all, SDR_PHYGRP_RWMGRGRP_ADDRESS |
					     RW_MGR_RUN_SINGLE_GROUP_OFFSET);

		writel(0x0F, &sdr_rw_load_mgr_regs->load_cntr0);
		writel(rwcfg->activate_0_and_1_wait1,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x0F, &sdr_rw_load_mgr_regs->load_cntr1);
		writel(rwcfg->activate_0_and_1_wait2,
		       &sdr_rw_load_jump_mgr_regs->load_jump_add1);

		/* Activate rows. */
		writel(rwcfg->activate_0_and_1, SDR_PHYGRP_RWMGRGRP_ADDRESS |
						RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	}
}

/**
 * mem_init_latency() - Configure memory RLAT and WLAT settings
 *
 * Configure memory RLAT and WLAT parameters.
 */
static void mem_init_latency(void)
{
	/*
	 * For AV/CV, LFIFO is hardened and always runs at full rate
	 * so max latency in AFI clocks, used here, is correspondingly
	 * smaller.
	 */
	const u32 max_latency = (1 << misccfg->max_latency_count_width) - 1;
	u32 rlat, wlat;

	debug("%s:%d\n", __func__, __LINE__);

	/*
	 * Read in write latency.
	 * WL for Hard PHY does not include additive latency.
	 */
	wlat = readl(&data_mgr->t_wl_add);
	wlat += readl(&data_mgr->mem_t_add);

	gbl->rw_wl_nop_cycles = wlat - 1;

	/* Read in readl latency. */
	rlat = readl(&data_mgr->t_rl_add);

	/* Set a pretty high read latency initially. */
	gbl->curr_read_lat = rlat + 16;
	if (gbl->curr_read_lat > max_latency)
		gbl->curr_read_lat = max_latency;

	writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);

	/* Advertise write latency. */
	writel(wlat, &phy_mgr_cfg->afi_wlat);
}

/**
 * @mem_skip_calibrate() - Set VFIFO and LFIFO to instant-on settings
 *
 * Set VFIFO and LFIFO to instant-on settings in skip calibration mode.
 */
static void mem_skip_calibrate(void)
{
	u32 vfifo_offset;
	u32 i, j, r;

	debug("%s:%d\n", __func__, __LINE__);
	/* Need to update every shadow register set used by the interface */
	for (r = 0; r < rwcfg->mem_number_of_ranks;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		/*
		 * Set output phase alignment settings appropriate for
		 * skip calibration.
		 */
		for (i = 0; i < rwcfg->mem_if_read_dqs_width; i++) {
			scc_mgr_set_dqs_en_phase(i, 0);
			if (iocfg->dll_chain_length == 6)
				scc_mgr_set_dqdqs_output_phase(i, 6);
			else
				scc_mgr_set_dqdqs_output_phase(i, 7);
			/*
			 * Case:33398
			 *
			 * Write data arrives to the I/O two cycles before write
			 * latency is reached (720 deg).
			 *   -> due to bit-slip in a/c bus
			 *   -> to allow board skew where dqs is longer than ck
			 *      -> how often can this happen!?
			 *      -> can claim back some ptaps for high freq
			 *       support if we can relax this, but i digress...
			 *
			 * The write_clk leads mem_ck by 90 deg
			 * The minimum ptap of the OPA is 180 deg
			 * Each ptap has (360 / IO_DLL_CHAIN_LENGH) deg of delay
			 * The write_clk is always delayed by 2 ptaps
			 *
			 * Hence, to make DQS aligned to CK, we need to delay
			 * DQS by:
			 *    (720 - 90 - 180 - 2) *
			 *      (360 / iocfg->dll_chain_length)
			 *
			 * Dividing the above by (360 / iocfg->dll_chain_length)
			 * gives us the number of ptaps, which simplies to:
			 *
			 *    (1.25 * iocfg->dll_chain_length - 2)
			 */
			scc_mgr_set_dqdqs_output_phase(i,
				       ((125 * iocfg->dll_chain_length) / 100) - 2);
		}
		writel(0xff, &sdr_scc_mgr->dqs_ena);
		writel(0xff, &sdr_scc_mgr->dqs_io_ena);

		for (i = 0; i < rwcfg->mem_if_write_dqs_width; i++) {
			writel(i, SDR_PHYGRP_SCCGRP_ADDRESS |
				  SCC_MGR_GROUP_COUNTER_OFFSET);
		}
		writel(0xff, &sdr_scc_mgr->dq_ena);
		writel(0xff, &sdr_scc_mgr->dm_ena);
		writel(0, &sdr_scc_mgr->update);
	}

	/* Compensate for simulation model behaviour */
	for (i = 0; i < rwcfg->mem_if_read_dqs_width; i++) {
		scc_mgr_set_dqs_bus_in_delay(i, 10);
		scc_mgr_load_dqs(i);
	}
	writel(0, &sdr_scc_mgr->update);

	/*
	 * ArriaV has hard FIFOs that can only be initialized by incrementing
	 * in sequencer.
	 */
	vfifo_offset = misccfg->calib_vfifo_offset;
	for (j = 0; j < vfifo_offset; j++)
		writel(0xff, &phy_mgr_cmd->inc_vfifo_hard_phy);
	writel(0, &phy_mgr_cmd->fifo_reset);

	/*
	 * For Arria V and Cyclone V with hard LFIFO, we get the skip-cal
	 * setting from generation-time constant.
	 */
	gbl->curr_read_lat = misccfg->calib_lfifo_offset;
	writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
}

/**
 * mem_calibrate() - Memory calibration entry point.
 *
 * Perform memory calibration.
 */
static u32 mem_calibrate(void)
{
	u32 i;
	u32 rank_bgn, sr;
	u32 write_group, write_test_bgn;
	u32 read_group, read_test_bgn;
	u32 run_groups, current_run;
	u32 failing_groups = 0;
	u32 group_failed = 0;

	const u32 rwdqs_ratio = rwcfg->mem_if_read_dqs_width /
				rwcfg->mem_if_write_dqs_width;

	debug("%s:%d\n", __func__, __LINE__);

	/* Initialize the data settings */
	gbl->error_substage = CAL_SUBSTAGE_NIL;
	gbl->error_stage = CAL_STAGE_NIL;
	gbl->error_group = 0xff;
	gbl->fom_in = 0;
	gbl->fom_out = 0;

	/* Initialize WLAT and RLAT. */
	mem_init_latency();

	/* Initialize bit slips. */
	mem_precharge_and_activate();

	for (i = 0; i < rwcfg->mem_if_read_dqs_width; i++) {
		writel(i, SDR_PHYGRP_SCCGRP_ADDRESS |
			  SCC_MGR_GROUP_COUNTER_OFFSET);
		/* Only needed once to set all groups, pins, DQ, DQS, DM. */
		if (i == 0)
			scc_mgr_set_hhp_extras();

		scc_set_bypass_mode(i);
	}

	/* Calibration is skipped. */
	if ((dyn_calib_steps & CALIB_SKIP_ALL) == CALIB_SKIP_ALL) {
		/*
		 * Set VFIFO and LFIFO to instant-on settings in skip
		 * calibration mode.
		 */
		mem_skip_calibrate();

		/*
		 * Do not remove this line as it makes sure all of our
		 * decisions have been applied.
		 */
		writel(0, &sdr_scc_mgr->update);
		return 1;
	}

	/* Calibration is not skipped. */
	for (i = 0; i < NUM_CALIB_REPEAT; i++) {
		/*
		 * Zero all delay chain/phase settings for all
		 * groups and all shadow register sets.
		 */
		scc_mgr_zero_all();

		run_groups = ~0;

		for (write_group = 0, write_test_bgn = 0; write_group
			< rwcfg->mem_if_write_dqs_width; write_group++,
			write_test_bgn += rwcfg->mem_dq_per_write_dqs) {
			/* Initialize the group failure */
			group_failed = 0;

			current_run = run_groups & ((1 <<
				RW_MGR_NUM_DQS_PER_WRITE_GROUP) - 1);
			run_groups = run_groups >>
				RW_MGR_NUM_DQS_PER_WRITE_GROUP;

			if (current_run == 0)
				continue;

			writel(write_group, SDR_PHYGRP_SCCGRP_ADDRESS |
					    SCC_MGR_GROUP_COUNTER_OFFSET);
			scc_mgr_zero_group(write_group, 0);

			for (read_group = write_group * rwdqs_ratio,
			     read_test_bgn = 0;
			     read_group < (write_group + 1) * rwdqs_ratio;
			     read_group++,
			     read_test_bgn += rwcfg->mem_dq_per_read_dqs) {
				if (STATIC_CALIB_STEPS & CALIB_SKIP_VFIFO)
					continue;

				/* Calibrate the VFIFO */
				if (rw_mgr_mem_calibrate_vfifo(read_group,
							       read_test_bgn))
					continue;

				if (!(gbl->phy_debug_mode_flags &
				      PHY_DEBUG_SWEEP_ALL_GROUPS))
					return 0;

				/* The group failed, we're done. */
				goto grp_failed;
			}

			/* Calibrate the output side */
			for (rank_bgn = 0, sr = 0;
			     rank_bgn < rwcfg->mem_number_of_ranks;
			     rank_bgn += NUM_RANKS_PER_SHADOW_REG, sr++) {
				if (STATIC_CALIB_STEPS & CALIB_SKIP_WRITES)
					continue;

				/* Not needed in quick mode! */
				if (STATIC_CALIB_STEPS &
				    CALIB_SKIP_DELAY_SWEEPS)
					continue;

				/* Calibrate WRITEs */
				if (!rw_mgr_mem_calibrate_writes(rank_bgn,
								 write_group,
								 write_test_bgn))
					continue;

				group_failed = 1;
				if (!(gbl->phy_debug_mode_flags &
				      PHY_DEBUG_SWEEP_ALL_GROUPS))
					return 0;
			}

			/* Some group failed, we're done. */
			if (group_failed)
				goto grp_failed;

			for (read_group = write_group * rwdqs_ratio,
			     read_test_bgn = 0;
			     read_group < (write_group + 1) * rwdqs_ratio;
			     read_group++,
			     read_test_bgn += rwcfg->mem_dq_per_read_dqs) {
				if (STATIC_CALIB_STEPS & CALIB_SKIP_WRITES)
					continue;

				if (!rw_mgr_mem_calibrate_vfifo_end(read_group,
								    read_test_bgn))
					continue;

				if (!(gbl->phy_debug_mode_flags &
				      PHY_DEBUG_SWEEP_ALL_GROUPS))
					return 0;

				/* The group failed, we're done. */
				goto grp_failed;
			}

			/* No group failed, continue as usual. */
			continue;

grp_failed:		/* A group failed, increment the counter. */
			failing_groups++;
		}

		/*
		 * USER If there are any failing groups then report
		 * the failure.
		 */
		if (failing_groups != 0)
			return 0;

		if (STATIC_CALIB_STEPS & CALIB_SKIP_LFIFO)
			continue;

		/* Calibrate the LFIFO */
		if (!rw_mgr_mem_calibrate_lfifo())
			return 0;
	}

	/*
	 * Do not remove this line as it makes sure all of our decisions
	 * have been applied.
	 */
	writel(0, &sdr_scc_mgr->update);
	return 1;
}

/**
 * run_mem_calibrate() - Perform memory calibration
 *
 * This function triggers the entire memory calibration procedure.
 */
static int run_mem_calibrate(void)
{
	int pass;
	u32 ctrl_cfg;

	debug("%s:%d\n", __func__, __LINE__);

	/* Reset pass/fail status shown on afi_cal_success/fail */
	writel(PHY_MGR_CAL_RESET, &phy_mgr_cfg->cal_status);

	/* Stop tracking manager. */
	ctrl_cfg = readl(&sdr_ctrl->ctrl_cfg);
	writel(ctrl_cfg & ~SDR_CTRLGRP_CTRLCFG_DQSTRKEN_MASK,
	       &sdr_ctrl->ctrl_cfg);

	phy_mgr_initialize();
	rw_mgr_mem_initialize();

	/* Perform the actual memory calibration. */
	pass = mem_calibrate();

	mem_precharge_and_activate();
	writel(0, &phy_mgr_cmd->fifo_reset);

	/* Handoff. */
	rw_mgr_mem_handoff();
	/*
	 * In Hard PHY this is a 2-bit control:
	 * 0: AFI Mux Select
	 * 1: DDIO Mux Select
	 */
	writel(0x2, &phy_mgr_cfg->mux_sel);

	/* Start tracking manager. */
	writel(ctrl_cfg, &sdr_ctrl->ctrl_cfg);

	return pass;
}

/**
 * debug_mem_calibrate() - Report result of memory calibration
 * @pass:	Value indicating whether calibration passed or failed
 *
 * This function reports the results of the memory calibration
 * and writes debug information into the register file.
 */
static void debug_mem_calibrate(int pass)
{
	u32 debug_info;

	if (pass) {
		debug("%s: CALIBRATION PASSED\n", __FILE__);

		gbl->fom_in /= 2;
		gbl->fom_out /= 2;

		if (gbl->fom_in > 0xff)
			gbl->fom_in = 0xff;

		if (gbl->fom_out > 0xff)
			gbl->fom_out = 0xff;

		/* Update the FOM in the register file */
		debug_info = gbl->fom_in;
		debug_info |= gbl->fom_out << 8;
		writel(debug_info, &sdr_reg_file->fom);

		writel(debug_info, &phy_mgr_cfg->cal_debug_info);
		writel(PHY_MGR_CAL_SUCCESS, &phy_mgr_cfg->cal_status);
	} else {
		debug("%s: CALIBRATION FAILED\n", __FILE__);

		debug_info = gbl->error_stage;
		debug_info |= gbl->error_substage << 8;
		debug_info |= gbl->error_group << 16;

		writel(debug_info, &sdr_reg_file->failing_stage);
		writel(debug_info, &phy_mgr_cfg->cal_debug_info);
		writel(PHY_MGR_CAL_FAIL, &phy_mgr_cfg->cal_status);

		/* Update the failing group/stage in the register file */
		debug_info = gbl->error_stage;
		debug_info |= gbl->error_substage << 8;
		debug_info |= gbl->error_group << 16;
		writel(debug_info, &sdr_reg_file->failing_stage);
	}

	debug("%s: Calibration complete\n", __FILE__);
}

/**
 * hc_initialize_rom_data() - Initialize ROM data
 *
 * Initialize ROM data.
 */
static void hc_initialize_rom_data(void)
{
	unsigned int nelem = 0;
	const u32 *rom_init;
	u32 i, addr;

	socfpga_get_seq_inst_init(&rom_init, &nelem);
	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_INST_ROM_WRITE_OFFSET;
	for (i = 0; i < nelem; i++)
		writel(rom_init[i], addr + (i << 2));

	socfpga_get_seq_ac_init(&rom_init, &nelem);
	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_AC_ROM_WRITE_OFFSET;
	for (i = 0; i < nelem; i++)
		writel(rom_init[i], addr + (i << 2));
}

/**
 * initialize_reg_file() - Initialize SDR register file
 *
 * Initialize SDR register file.
 */
static void initialize_reg_file(void)
{
	/* Initialize the register file with the correct data */
	writel(misccfg->reg_file_init_seq_signature, &sdr_reg_file->signature);
	writel(0, &sdr_reg_file->debug_data_addr);
	writel(0, &sdr_reg_file->cur_stage);
	writel(0, &sdr_reg_file->fom);
	writel(0, &sdr_reg_file->failing_stage);
	writel(0, &sdr_reg_file->debug1);
	writel(0, &sdr_reg_file->debug2);
}

/**
 * initialize_hps_phy() - Initialize HPS PHY
 *
 * Initialize HPS PHY.
 */
static void initialize_hps_phy(void)
{
	u32 reg;
	/*
	 * Tracking also gets configured here because it's in the
	 * same register.
	 */
	u32 trk_sample_count = 7500;
	u32 trk_long_idle_sample_count = (10 << 16) | 100;
	/*
	 * Format is number of outer loops in the 16 MSB, sample
	 * count in 16 LSB.
	 */

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ACDELAYEN_SET(2);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSLOGICDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_RESETDELAYEN_SET(0);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_LPDDRDIS_SET(1);
	/*
	 * This field selects the intrinsic latency to RDATA_EN/FULL path.
	 * 00-bypass, 01- add 5 cycles, 10- add 10 cycles, 11- add 15 cycles.
	 */
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ADDLATSEL_SET(0);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_SET(
		trk_sample_count);
	writel(reg, &sdr_ctrl->phy_ctrl0);

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_SAMPLECOUNT_31_20_SET(
		trk_sample_count >>
		SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_WIDTH);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_SET(
		trk_long_idle_sample_count);
	writel(reg, &sdr_ctrl->phy_ctrl1);

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_2_LONGIDLESAMPLECOUNT_31_20_SET(
		trk_long_idle_sample_count >>
		SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_WIDTH);
	writel(reg, &sdr_ctrl->phy_ctrl2);
}

/**
 * initialize_tracking() - Initialize tracking
 *
 * Initialize the register file with usable initial data.
 */
static void initialize_tracking(void)
{
	/*
	 * Initialize the register file with the correct data.
	 * Compute usable version of value in case we skip full
	 * computation later.
	 */
	writel(DIV_ROUND_UP(iocfg->delay_per_opa_tap,
			    iocfg->delay_per_dchain_tap) - 1,
	       &sdr_reg_file->dtaps_per_ptap);

	/* trk_sample_count */
	writel(7500, &sdr_reg_file->trk_sample_count);

	/* longidle outer loop [15:0] */
	writel((10 << 16) | (100 << 0), &sdr_reg_file->trk_longidle);

	/*
	 * longidle sample count [31:24]
	 * trfc, worst case of 933Mhz 4Gb [23:16]
	 * trcd, worst case [15:8]
	 * vfifo wait [7:0]
	 */
	writel((243 << 24) | (14 << 16) | (10 << 8) | (4 << 0),
	       &sdr_reg_file->delays);

	/* mux delay */
	writel((rwcfg->idle << 24) | (rwcfg->activate_1 << 16) |
	       (rwcfg->sgle_read << 8) | (rwcfg->precharge_all << 0),
	       &sdr_reg_file->trk_rw_mgr_addr);

	writel(rwcfg->mem_if_read_dqs_width,
	       &sdr_reg_file->trk_read_dqs_width);

	/* trefi [7:0] */
	writel((rwcfg->refresh_all << 24) | (1000 << 0),
	       &sdr_reg_file->trk_rfsh);
}

int sdram_calibration_full(struct socfpga_sdr *sdr)
{
	struct param_type my_param;
	struct gbl_type my_gbl;
	u32 pass;

	/*
	 * For size reasons, this file uses hard coded addresses.
	 * Check if we are called with the correct address.
	 */
	if (sdr != (struct socfpga_sdr *)SOCFPGA_SDR_ADDRESS)
		return -ENODEV;

	memset(&my_param, 0, sizeof(my_param));
	memset(&my_gbl, 0, sizeof(my_gbl));

	param = &my_param;
	gbl = &my_gbl;

	rwcfg = socfpga_get_sdram_rwmgr_config();
	iocfg = socfpga_get_sdram_io_config();
	misccfg = socfpga_get_sdram_misc_config();

	/* Set the calibration enabled by default */
	gbl->phy_debug_mode_flags |= PHY_DEBUG_ENABLE_CAL_RPT;
	/*
	 * Only sweep all groups (regardless of fail state) by default
	 * Set enabled read test by default.
	 */
#if DISABLE_GUARANTEED_READ
	gbl->phy_debug_mode_flags |= PHY_DEBUG_DISABLE_GUARANTEED_READ;
#endif
	/* Initialize the register file */
	initialize_reg_file();

	/* Initialize any PHY CSR */
	initialize_hps_phy();

	scc_mgr_initialize();

	initialize_tracking();

	debug("%s: Preparing to start memory calibration\n", __FILE__);

	debug("%s:%d\n", __func__, __LINE__);
	debug_cond(DLEVEL >= 1,
		   "DDR3 FULL_RATE ranks=%u cs/dimm=%u dq/dqs=%u,%u vg/dqs=%u,%u ",
		   rwcfg->mem_number_of_ranks, rwcfg->mem_number_of_cs_per_dimm,
		   rwcfg->mem_dq_per_read_dqs, rwcfg->mem_dq_per_write_dqs,
		   rwcfg->mem_virtual_groups_per_read_dqs,
		   rwcfg->mem_virtual_groups_per_write_dqs);
	debug_cond(DLEVEL >= 1,
		   "dqs=%u,%u dq=%u dm=%u ptap_delay=%u dtap_delay=%u ",
		   rwcfg->mem_if_read_dqs_width, rwcfg->mem_if_write_dqs_width,
		   rwcfg->mem_data_width, rwcfg->mem_data_mask_width,
		   iocfg->delay_per_opa_tap, iocfg->delay_per_dchain_tap);
	debug_cond(DLEVEL >= 1, "dtap_dqsen_delay=%u, dll=%u",
		   iocfg->delay_per_dqs_en_dchain_tap, iocfg->dll_chain_length);
	debug_cond(DLEVEL >= 1,
		   "max values: en_p=%u dqdqs_p=%u en_d=%u dqs_in_d=%u ",
		   iocfg->dqs_en_phase_max, iocfg->dqdqs_out_phase_max,
		   iocfg->dqs_en_delay_max, iocfg->dqs_in_delay_max);
	debug_cond(DLEVEL >= 1, "io_in_d=%u io_out1_d=%u io_out2_d=%u ",
		   iocfg->io_in_delay_max, iocfg->io_out1_delay_max,
		   iocfg->io_out2_delay_max);
	debug_cond(DLEVEL >= 1, "dqs_in_reserve=%u dqs_out_reserve=%u\n",
		   iocfg->dqs_in_reserve, iocfg->dqs_out_reserve);

	hc_initialize_rom_data();

	/* update info for sims */
	reg_file_set_stage(CAL_STAGE_NIL);
	reg_file_set_group(0);

	/*
	 * Load global needed for those actions that require
	 * some dynamic calibration support.
	 */
	dyn_calib_steps = STATIC_CALIB_STEPS;
	/*
	 * Load global to allow dynamic selection of delay loop settings
	 * based on calibration mode.
	 */
	if (!(dyn_calib_steps & CALIB_SKIP_DELAY_LOOPS))
		skip_delay_mask = 0xff;
	else
		skip_delay_mask = 0x0;

	pass = run_mem_calibrate();
	debug_mem_calibrate(pass);
	return pass;
}
