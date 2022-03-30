/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010, 2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _SDRAM_PARAM_H_
#define _SDRAM_PARAM_H_

/*
 * Defines the number of 32-bit words provided in each set of SDRAM parameters
 * for arbitration configuration data.
 */
#define BCT_SDRAM_ARB_CONFIG_WORDS 27

enum memory_type {
	MEMORY_TYPE_NONE = 0,
	MEMORY_TYPE_DDR,
	MEMORY_TYPE_LPDDR,
	MEMORY_TYPE_DDR2,
	MEMORY_TYPE_LPDDR2,
	MEMORY_TYPE_NUM,
	MEMORY_TYPE_FORCE32 = 0x7FFFFFFF
};

/* Defines the SDRAM parameter structure */
struct sdram_params {
	enum memory_type memory_type;
	u32 pllm_charge_pump_setup_control;
	u32 pllm_loop_filter_setup_control;
	u32 pllm_input_divider;
	u32 pllm_feedback_divider;
	u32 pllm_post_divider;
	u32 pllm_stable_time;
	u32 emc_clock_divider;
	u32 emc_auto_cal_interval;
	u32 emc_auto_cal_config;
	u32 emc_auto_cal_wait;
	u32 emc_pin_program_wait;
	u32 emc_rc;
	u32 emc_rfc;
	u32 emc_ras;
	u32 emc_rp;
	u32 emc_r2w;
	u32 emc_w2r;
	u32 emc_r2p;
	u32 emc_w2p;
	u32 emc_rd_rcd;
	u32 emc_wr_rcd;
	u32 emc_rrd;
	u32 emc_rext;
	u32 emc_wdv;
	u32 emc_quse;
	u32 emc_qrst;
	u32 emc_qsafe;
	u32 emc_rdv;
	u32 emc_refresh;
	u32 emc_burst_refresh_num;
	u32 emc_pdex2wr;
	u32 emc_pdex2rd;
	u32 emc_pchg2pden;
	u32 emc_act2pden;
	u32 emc_ar2pden;
	u32 emc_rw2pden;
	u32 emc_txsr;
	u32 emc_tcke;
	u32 emc_tfaw;
	u32 emc_trpab;
	u32 emc_tclkstable;
	u32 emc_tclkstop;
	u32 emc_trefbw;
	u32 emc_quseextra;
	u32 emc_fbioc_fg1;
	u32 emc_fbio_dqsib_dly;
	u32 emc_fbio_dqsib_dly_msb;
	u32 emc_fbio_quse_dly;
	u32 emc_fbio_quse_dly_msb;
	u32 emc_fbio_cfg5;
	u32 emc_fbio_cfg6;
	u32 emc_fbio_spare;
	u32 emc_mrs;
	u32 emc_emrs;
	u32 emc_mrw1;
	u32 emc_mrw2;
	u32 emc_mrw3;
	u32 emc_mrw_reset_command;
	u32 emc_mrw_reset_init_wait;
	u32 emc_adr_cfg;
	u32 emc_adr_cfg1;
	u32 emc_emem_cfg;
	u32 emc_low_latency_config;
	u32 emc_cfg;
	u32 emc_cfg2;
	u32 emc_dbg;
	u32 ahb_arbitration_xbar_ctrl;
	u32 emc_cfg_dig_dll;
	u32 emc_dll_xform_dqs;
	u32 emc_dll_xform_quse;
	u32 warm_boot_wait;
	u32 emc_ctt_term_ctrl;
	u32 emc_odt_write;
	u32 emc_odt_read;
	u32 emc_zcal_ref_cnt;
	u32 emc_zcal_wait_cnt;
	u32 emc_zcal_mrw_cmd;
	u32 emc_mrs_reset_dll;
	u32 emc_mrw_zq_init_dev0;
	u32 emc_mrw_zq_init_dev1;
	u32 emc_mrw_zq_init_wait;
	u32 emc_mrs_reset_dll_wait;
	u32 emc_emrs_emr2;
	u32 emc_emrs_emr3;
	u32 emc_emrs_ddr2_dll_enable;
	u32 emc_mrs_ddr2_dll_reset;
	u32 emc_emrs_ddr2_ocd_calib;
	u32 emc_edr2_wait;
	u32 emc_cfg_clktrim0;
	u32 emc_cfg_clktrim1;
	u32 emc_cfg_clktrim2;
	u32 pmc_ddr_pwr;
	u32 apb_misc_gp_xm2cfga_padctrl;
	u32 apb_misc_gp_xm2cfgc_padctrl;
	u32 apb_misc_gp_xm2cfgc_padctrl2;
	u32 apb_misc_gp_xm2cfgd_padctrl;
	u32 apb_misc_gp_xm2cfgd_padctrl2;
	u32 apb_misc_gp_xm2clkcfg_padctrl;
	u32 apb_misc_gp_xm2comp_padctrl;
	u32 apb_misc_gp_xm2vttgen_padctrl;
	u32 arbitration_config[BCT_SDRAM_ARB_CONFIG_WORDS];
};
#endif
