/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright Altera Corporation (C) 2012-2015
 */

#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#define RW_MGR_NUM_DM_PER_WRITE_GROUP (rwcfg->mem_data_mask_width \
	/ rwcfg->mem_if_write_dqs_width)
#define RW_MGR_NUM_TRUE_DM_PER_WRITE_GROUP (rwcfg->true_mem_data_mask_width \
	/ rwcfg->mem_if_write_dqs_width)

#define RW_MGR_NUM_DQS_PER_WRITE_GROUP (rwcfg->mem_if_read_dqs_width \
	/ rwcfg->mem_if_write_dqs_width)
#define NUM_RANKS_PER_SHADOW_REG (rwcfg->mem_number_of_ranks / NUM_SHADOW_REGS)

#define RW_MGR_RUN_SINGLE_GROUP_OFFSET		0x0
#define RW_MGR_RUN_ALL_GROUPS_OFFSET		0x0400
#define RW_MGR_RESET_READ_DATAPATH_OFFSET	0x1000
#define RW_MGR_SET_CS_AND_ODT_MASK_OFFSET	0x1400
#define RW_MGR_INST_ROM_WRITE_OFFSET		0x1800
#define RW_MGR_AC_ROM_WRITE_OFFSET		0x1C00

#define NUM_SHADOW_REGS			1

#define RW_MGR_RANK_NONE		0xFF
#define RW_MGR_RANK_ALL			0x00

#define RW_MGR_ODT_MODE_OFF		0
#define RW_MGR_ODT_MODE_READ_WRITE	1

#define NUM_CALIB_REPEAT		1

#define NUM_READ_TESTS			7
#define NUM_READ_PB_TESTS		7
#define NUM_WRITE_TESTS			15
#define NUM_WRITE_PB_TESTS		31

#define PASS_ALL_BITS			1
#define PASS_ONE_BIT			0

/* calibration stages */
#define CAL_STAGE_NIL			0
#define CAL_STAGE_VFIFO			1
#define CAL_STAGE_WLEVEL		2
#define CAL_STAGE_LFIFO			3
#define CAL_STAGE_WRITES		4
#define CAL_STAGE_FULLTEST		5
#define CAL_STAGE_REFRESH		6
#define CAL_STAGE_CAL_SKIPPED		7
#define CAL_STAGE_CAL_ABORTED		8
#define CAL_STAGE_VFIFO_AFTER_WRITES	9

/* calibration substages */
#define CAL_SUBSTAGE_NIL		0
#define CAL_SUBSTAGE_GUARANTEED_READ	1
#define CAL_SUBSTAGE_DQS_EN_PHASE	2
#define CAL_SUBSTAGE_VFIFO_CENTER	3
#define CAL_SUBSTAGE_WORKING_DELAY	1
#define CAL_SUBSTAGE_LAST_WORKING_DELAY	2
#define CAL_SUBSTAGE_WLEVEL_COPY	3
#define CAL_SUBSTAGE_WRITES_CENTER	1
#define CAL_SUBSTAGE_READ_LATENCY	1
#define CAL_SUBSTAGE_REFRESH		1

#define SCC_MGR_GROUP_COUNTER_OFFSET		0x0000
#define SCC_MGR_DQS_IN_DELAY_OFFSET		0x0100
#define SCC_MGR_DQS_EN_PHASE_OFFSET		0x0200
#define SCC_MGR_DQS_EN_DELAY_OFFSET		0x0300
#define SCC_MGR_DQDQS_OUT_PHASE_OFFSET		0x0400
#define SCC_MGR_OCT_OUT1_DELAY_OFFSET		0x0500
#define SCC_MGR_IO_OUT1_DELAY_OFFSET		0x0700
#define SCC_MGR_IO_IN_DELAY_OFFSET		0x0900

/* HHP-HPS-specific versions of some commands */
#define SCC_MGR_DQS_EN_DELAY_GATE_OFFSET	0x0600
#define SCC_MGR_IO_OE_DELAY_OFFSET		0x0800
#define SCC_MGR_HHP_GLOBALS_OFFSET		0x0A00
#define SCC_MGR_HHP_RFILE_OFFSET		0x0B00
#define SCC_MGR_AFI_CAL_INIT_OFFSET		0x0D00

#define SDR_PHYGRP_SCCGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x0)
#define SDR_PHYGRP_PHYMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x1000)
#define SDR_PHYGRP_RWMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x2000)
#define SDR_PHYGRP_DATAMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x4000)
#define SDR_PHYGRP_REGFILEGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x4800)

#define PHY_MGR_CAL_RESET		(0)
#define PHY_MGR_CAL_SUCCESS		(1)
#define PHY_MGR_CAL_FAIL		(2)

#define CALIB_SKIP_DELAY_LOOPS		(1 << 0)
#define CALIB_SKIP_ALL_BITS_CHK		(1 << 1)
#define CALIB_SKIP_DELAY_SWEEPS		(1 << 2)
#define CALIB_SKIP_VFIFO		(1 << 3)
#define CALIB_SKIP_LFIFO		(1 << 4)
#define CALIB_SKIP_WLEVEL		(1 << 5)
#define CALIB_SKIP_WRITES		(1 << 6)
#define CALIB_SKIP_FULL_TEST		(1 << 7)
#define CALIB_SKIP_ALL			(CALIB_SKIP_VFIFO | \
				CALIB_SKIP_LFIFO | CALIB_SKIP_WLEVEL | \
				CALIB_SKIP_WRITES | CALIB_SKIP_FULL_TEST)
#define CALIB_IN_RTL_SIM			(1 << 8)

/* Scan chain manager command addresses */
#define READ_SCC_OCT_OUT2_DELAY			0
#define READ_SCC_DQ_OUT2_DELAY			0
#define READ_SCC_DQS_IO_OUT2_DELAY		0
#define READ_SCC_DM_IO_OUT2_DELAY		0

/* HHP-HPS-specific values */
#define SCC_MGR_HHP_EXTRAS_OFFSET			0
#define SCC_MGR_HHP_DQSE_MAP_OFFSET			1

/* PHY Debug mode flag constants */
#define PHY_DEBUG_IN_DEBUG_MODE 0x00000001
#define PHY_DEBUG_ENABLE_CAL_RPT 0x00000002
#define PHY_DEBUG_ENABLE_MARGIN_RPT 0x00000004
#define PHY_DEBUG_SWEEP_ALL_GROUPS 0x00000008
#define PHY_DEBUG_DISABLE_GUARANTEED_READ 0x00000010
#define PHY_DEBUG_ENABLE_NON_DESTRUCTIVE_CALIBRATION 0x00000020

struct socfpga_sdr_rw_load_manager {
	u32	load_cntr0;
	u32	load_cntr1;
	u32	load_cntr2;
	u32	load_cntr3;
};

struct socfpga_sdr_rw_load_jump_manager {
	u32	load_jump_add0;
	u32	load_jump_add1;
	u32	load_jump_add2;
	u32	load_jump_add3;
};

struct socfpga_sdr_reg_file {
	u32 signature;
	u32 debug_data_addr;
	u32 cur_stage;
	u32 fom;
	u32 failing_stage;
	u32 debug1;
	u32 debug2;
	u32 dtaps_per_ptap;
	u32 trk_sample_count;
	u32 trk_longidle;
	u32 delays;
	u32 trk_rw_mgr_addr;
	u32 trk_read_dqs_width;
	u32 trk_rfsh;
};

/* parameter variable holder */
struct param_type {
	u32	read_correct_mask;
	u32	read_correct_mask_vg;
	u32	write_correct_mask;
	u32	write_correct_mask_vg;
};


/* global variable holder */
struct gbl_type {
	uint32_t phy_debug_mode_flags;

	/* current read latency */

	uint32_t curr_read_lat;

	/* error code */

	uint32_t error_substage;
	uint32_t error_stage;
	uint32_t error_group;

	/* figure-of-merit in, figure-of-merit out */

	uint32_t fom_in;
	uint32_t fom_out;

	/*USER Number of RW Mgr NOP cycles between
	write command and write data */
	uint32_t rw_wl_nop_cycles;
};

struct socfpga_sdr_scc_mgr {
	u32	dqs_ena;
	u32	dqs_io_ena;
	u32	dq_ena;
	u32	dm_ena;
	u32	__padding1[4];
	u32	update;
	u32	__padding2[7];
	u32	active_rank;
};

/* PHY manager configuration registers. */
struct socfpga_phy_mgr_cfg {
	u32	phy_rlat;
	u32	reset_mem_stbl;
	u32	mux_sel;
	u32	cal_status;
	u32	cal_debug_info;
	u32	vfifo_rd_en_ovrd;
	u32	afi_wlat;
	u32	afi_rlat;
};

/* PHY manager command addresses. */
struct socfpga_phy_mgr_cmd {
	u32	inc_vfifo_fr;
	u32	inc_vfifo_hard_phy;
	u32	fifo_reset;
	u32	inc_vfifo_fr_hr;
	u32	inc_vfifo_qr;
};

struct socfpga_data_mgr {
	u32	__padding1;
	u32	t_wl_add;
	u32	mem_t_add;
	u32	t_rl_add;
};

/* This struct describes the controller @ SOCFPGA_SDR_ADDRESS */
struct socfpga_sdr {
	/* SDR_PHYGRP_SCCGRP_ADDRESS */
	u8 _align1[0xe00];
	/* SDR_PHYGRP_SCCGRP_ADDRESS | 0xe00 */
	struct socfpga_sdr_scc_mgr sdr_scc_mgr;
	u8 _align2[0x1bc];
	/* SDR_PHYGRP_PHYMGRGRP_ADDRESS */
	struct socfpga_phy_mgr_cmd phy_mgr_cmd;
	u8 _align3[0x2c];
	/* SDR_PHYGRP_PHYMGRGRP_ADDRESS | 0x40 */
	struct socfpga_phy_mgr_cfg phy_mgr_cfg;
	u8 _align4[0xfa0];
	/* SDR_PHYGRP_RWMGRGRP_ADDRESS */
	u8 rwmgr_grp[0x800];
	/* SDR_PHYGRP_RWMGRGRP_ADDRESS | 0x800 */
	struct socfpga_sdr_rw_load_manager sdr_rw_load_mgr_regs;
	u8 _align5[0x3f0];
	/* SDR_PHYGRP_RWMGRGRP_ADDRESS | 0xC00 */
	struct socfpga_sdr_rw_load_jump_manager sdr_rw_load_jump_mgr_regs;
	u8 _align6[0x13f0];
	/* SDR_PHYGRP_DATAMGRGRP_ADDRESS */
	struct socfpga_data_mgr data_mgr;
	u8 _align7[0x7f0];
	/* SDR_PHYGRP_REGFILEGRP_ADDRESS */
	struct socfpga_sdr_reg_file sdr_reg_file;
	u8 _align8[0x7c8];
	/* SDR_CTRLGRP_ADDRESS */
	struct socfpga_sdr_ctrl sdr_ctrl;
	u8 _align9[0xea4];
};

int sdram_calibration_full(struct socfpga_sdr *sdr);

#endif /* _SEQUENCER_H_ */
