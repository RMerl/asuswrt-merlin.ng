/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright Altera Corporation (C) 2014-2015
 */
#ifndef	_SOCFPGA_SDRAM_GEN5_H_
#define	_SOCFPGA_SDRAM_GEN5_H_

#ifndef __ASSEMBLY__

const struct socfpga_sdram_config *socfpga_get_sdram_config(void);

void socfpga_get_seq_ac_init(const u32 **init, unsigned int *nelem);
void socfpga_get_seq_inst_init(const u32 **init, unsigned int *nelem);
const struct socfpga_sdram_rw_mgr_config *socfpga_get_sdram_rwmgr_config(void);
const struct socfpga_sdram_io_config *socfpga_get_sdram_io_config(void);
const struct socfpga_sdram_misc_config *socfpga_get_sdram_misc_config(void);

#define SDR_CTRLGRP_ADDRESS	(SOCFPGA_SDR_ADDRESS | 0x5000)

struct socfpga_sdr_ctrl {
	u32	ctrl_cfg;
	u32	dram_timing1;
	u32	dram_timing2;
	u32	dram_timing3;
	u32	dram_timing4;	/* 0x10 */
	u32	lowpwr_timing;
	u32	dram_odt;
	u32	extratime1;
	u32	__padding0[3];
	u32	dram_addrw;	/* 0x2c */
	u32	dram_if_width;	/* 0x30 */
	u32	dram_dev_width;
	u32	dram_sts;
	u32	dram_intr;
	u32	sbe_count;	/* 0x40 */
	u32	dbe_count;
	u32	err_addr;
	u32	drop_count;
	u32	drop_addr;	/* 0x50 */
	u32	lowpwr_eq;
	u32	lowpwr_ack;
	u32	static_cfg;
	u32	ctrl_width;	/* 0x60 */
	u32	cport_width;
	u32	cport_wmap;
	u32	cport_rmap;
	u32	rfifo_cmap;	/* 0x70 */
	u32	wfifo_cmap;
	u32	cport_rdwr;
	u32	port_cfg;
	u32	fpgaport_rst;	/* 0x80 */
	u32	__padding1;
	u32	fifo_cfg;
	u32	protport_default;
	u32	prot_rule_addr;	/* 0x90 */
	u32	prot_rule_id;
	u32	prot_rule_data;
	u32	prot_rule_rdwr;
	u32	__padding2[3];
	u32	mp_priority;	/* 0xac */
	u32	mp_weight0;	/* 0xb0 */
	u32	mp_weight1;
	u32	mp_weight2;
	u32	mp_weight3;
	u32	mp_pacing0;	/* 0xc0 */
	u32	mp_pacing1;
	u32	mp_pacing2;
	u32	mp_pacing3;
	u32	mp_threshold0;	/* 0xd0 */
	u32	mp_threshold1;
	u32	mp_threshold2;
	u32	__padding3[29];
	u32	phy_ctrl0;	/* 0x150 */
	u32	phy_ctrl1;
	u32	phy_ctrl2;
};

/* SDRAM configuration structure for the SPL. */
struct socfpga_sdram_config {
	u32	ctrl_cfg;
	u32	dram_timing1;
	u32	dram_timing2;
	u32	dram_timing3;
	u32	dram_timing4;
	u32	lowpwr_timing;
	u32	dram_odt;
	u32	extratime1;
	u32	dram_addrw;
	u32	dram_if_width;
	u32	dram_dev_width;
	u32	dram_intr;
	u32	lowpwr_eq;
	u32	static_cfg;
	u32	ctrl_width;
	u32	cport_width;
	u32	cport_wmap;
	u32	cport_rmap;
	u32	rfifo_cmap;
	u32	wfifo_cmap;
	u32	cport_rdwr;
	u32	port_cfg;
	u32	fpgaport_rst;
	u32	fifo_cfg;
	u32	mp_priority;
	u32	mp_weight0;
	u32	mp_weight1;
	u32	mp_weight2;
	u32	mp_weight3;
	u32	mp_pacing0;
	u32	mp_pacing1;
	u32	mp_pacing2;
	u32	mp_pacing3;
	u32	mp_threshold0;
	u32	mp_threshold1;
	u32	mp_threshold2;
	u32	phy_ctrl0;
};

struct socfpga_sdram_rw_mgr_config {
	u8	activate_0_and_1;
	u8	activate_0_and_1_wait1;
	u8	activate_0_and_1_wait2;
	u8	activate_1;
	u8	clear_dqs_enable;
	u8	guaranteed_read;
	u8	guaranteed_read_cont;
	u8	guaranteed_write;
	u8	guaranteed_write_wait0;
	u8	guaranteed_write_wait1;
	u8	guaranteed_write_wait2;
	u8	guaranteed_write_wait3;
	u8	idle;
	u8	idle_loop1;
	u8	idle_loop2;
	u8	init_reset_0_cke_0;
	u8	init_reset_1_cke_0;
	u8	lfsr_wr_rd_bank_0;
	u8	lfsr_wr_rd_bank_0_data;
	u8	lfsr_wr_rd_bank_0_dqs;
	u8	lfsr_wr_rd_bank_0_nop;
	u8	lfsr_wr_rd_bank_0_wait;
	u8	lfsr_wr_rd_bank_0_wl_1;
	u8	lfsr_wr_rd_dm_bank_0;
	u8	lfsr_wr_rd_dm_bank_0_data;
	u8	lfsr_wr_rd_dm_bank_0_dqs;
	u8	lfsr_wr_rd_dm_bank_0_nop;
	u8	lfsr_wr_rd_dm_bank_0_wait;
	u8	lfsr_wr_rd_dm_bank_0_wl_1;
	union {
		u8	mrs0_dll_reset;
		u8	mr_dll_reset;
	};
	union {
		u8	mrs0_dll_reset_mirr;
		u8	emr_ocd_enable;
	};
	union {
		u8	mrs0_user;
		u8	mr_user;
	};
	union {
		u8	mrs0_user_mirr;
		u8	mr_calib;
	};
	union {
		u8	mrs1;
		u8	emr;
	};
	union {
		u8	mrs2;
		u8	emr2;
	};
	union {
		u8	mrs3;
		u8	emr3;
	};
	u8	mrs1_mirr;
	u8	mrs2_mirr;
	u8	mrs3_mirr;
	u8	precharge_all;
	u8	read_b2b;
	u8	read_b2b_wait1;
	u8	read_b2b_wait2;
	union {
		u8	refresh;
		u8	refresh_all;
	};
	u8	rreturn;
	u8	sgle_read;
	union {
		u8	zqcl;
		u8	nop;
	};

	u8	true_mem_data_mask_width;
	u8	mem_address_mirroring;
	u8	mem_data_mask_width;
	u8	mem_data_width;
	u8	mem_dq_per_read_dqs;
	u8	mem_dq_per_write_dqs;
	u8	mem_if_read_dqs_width;
	u8	mem_if_write_dqs_width;
	u8	mem_number_of_cs_per_dimm;
	u8	mem_number_of_ranks;
	u8	mem_virtual_groups_per_read_dqs;
	u8	mem_virtual_groups_per_write_dqs;
};

struct socfpga_sdram_io_config {
	u16	delay_per_opa_tap;
	u8	delay_per_dchain_tap;
	u8	delay_per_dqs_en_dchain_tap;
	u8	dll_chain_length;
	u8	dqdqs_out_phase_max;
	u8	dqs_en_delay_max;
	u8	dqs_en_delay_offset;
	u8	dqs_en_phase_max;
	u8	dqs_in_delay_max;
	u8	dqs_in_reserve;
	u8	dqs_out_reserve;
	u8	io_in_delay_max;
	u8	io_out1_delay_max;
	u8	io_out2_delay_max;
	u8	shift_dqs_en_when_shift_dqs;
};

struct socfpga_sdram_misc_config {
	u32	reg_file_init_seq_signature;
	u16	afi_clk_freq;
	u8	afi_rate_ratio;
	u8	calib_lfifo_offset;
	u8	calib_vfifo_offset;
	u8	enable_super_quick_calibration;
	u8	max_latency_count_width;
	u8	read_valid_fifo_size;
	u8	tinit_cntr0_val;
	u8	tinit_cntr1_val;
	u8	tinit_cntr2_val;
	u8	treset_cntr0_val;
	u8	treset_cntr1_val;
	u8	treset_cntr2_val;
};

#define SDR_CTRLGRP_CTRLCFG_NODMPINS_LSB 23
#define SDR_CTRLGRP_CTRLCFG_NODMPINS_MASK 0x00800000
#define SDR_CTRLGRP_CTRLCFG_DQSTRKEN_LSB 22
#define SDR_CTRLGRP_CTRLCFG_DQSTRKEN_MASK 0x00400000
#define SDR_CTRLGRP_CTRLCFG_STARVELIMIT_LSB 16
#define SDR_CTRLGRP_CTRLCFG_STARVELIMIT_MASK 0x003f0000
#define SDR_CTRLGRP_CTRLCFG_REORDEREN_LSB 15
#define SDR_CTRLGRP_CTRLCFG_REORDEREN_MASK 0x00008000
#define SDR_CTRLGRP_CTRLCFG_ECCCORREN_LSB 11
#define SDR_CTRLGRP_CTRLCFG_ECCCORREN_MASK 0x00000800
#define SDR_CTRLGRP_CTRLCFG_ECCEN_LSB 10
#define SDR_CTRLGRP_CTRLCFG_ECCEN_MASK 0x00000400
#define SDR_CTRLGRP_CTRLCFG_ADDRORDER_LSB 8
#define SDR_CTRLGRP_CTRLCFG_ADDRORDER_MASK 0x00000300
#define SDR_CTRLGRP_CTRLCFG_MEMBL_LSB 3
#define SDR_CTRLGRP_CTRLCFG_MEMBL_MASK 0x000000f8
#define SDR_CTRLGRP_CTRLCFG_MEMTYPE_LSB 0
#define SDR_CTRLGRP_CTRLCFG_MEMTYPE_MASK 0x00000007
/* Register template: sdr::ctrlgrp::dramtiming1                            */
#define SDR_CTRLGRP_DRAMTIMING1_TRFC_LSB 24
#define SDR_CTRLGRP_DRAMTIMING1_TRFC_MASK 0xff000000
#define SDR_CTRLGRP_DRAMTIMING1_TFAW_LSB 18
#define SDR_CTRLGRP_DRAMTIMING1_TFAW_MASK 0x00fc0000
#define SDR_CTRLGRP_DRAMTIMING1_TRRD_LSB 14
#define SDR_CTRLGRP_DRAMTIMING1_TRRD_MASK 0x0003c000
#define SDR_CTRLGRP_DRAMTIMING1_TCL_LSB 9
#define SDR_CTRLGRP_DRAMTIMING1_TCL_MASK 0x00003e00
#define SDR_CTRLGRP_DRAMTIMING1_TAL_LSB 4
#define SDR_CTRLGRP_DRAMTIMING1_TAL_MASK 0x000001f0
#define SDR_CTRLGRP_DRAMTIMING1_TCWL_LSB 0
#define SDR_CTRLGRP_DRAMTIMING1_TCWL_MASK 0x0000000f
/* Register template: sdr::ctrlgrp::dramtiming2                            */
#define SDR_CTRLGRP_DRAMTIMING2_TWTR_LSB 25
#define SDR_CTRLGRP_DRAMTIMING2_TWTR_MASK 0x1e000000
#define SDR_CTRLGRP_DRAMTIMING2_TWR_LSB 21
#define SDR_CTRLGRP_DRAMTIMING2_TWR_MASK 0x01e00000
#define SDR_CTRLGRP_DRAMTIMING2_TRP_LSB 17
#define SDR_CTRLGRP_DRAMTIMING2_TRP_MASK 0x001e0000
#define SDR_CTRLGRP_DRAMTIMING2_TRCD_LSB 13
#define SDR_CTRLGRP_DRAMTIMING2_TRCD_MASK 0x0001e000
#define SDR_CTRLGRP_DRAMTIMING2_TREFI_LSB 0
#define SDR_CTRLGRP_DRAMTIMING2_TREFI_MASK 0x00001fff
/* Register template: sdr::ctrlgrp::dramtiming3                            */
#define SDR_CTRLGRP_DRAMTIMING3_TCCD_LSB 19
#define SDR_CTRLGRP_DRAMTIMING3_TCCD_MASK 0x00780000
#define SDR_CTRLGRP_DRAMTIMING3_TMRD_LSB 15
#define SDR_CTRLGRP_DRAMTIMING3_TMRD_MASK 0x00078000
#define SDR_CTRLGRP_DRAMTIMING3_TRC_LSB 9
#define SDR_CTRLGRP_DRAMTIMING3_TRC_MASK 0x00007e00
#define SDR_CTRLGRP_DRAMTIMING3_TRAS_LSB 4
#define SDR_CTRLGRP_DRAMTIMING3_TRAS_MASK 0x000001f0
#define SDR_CTRLGRP_DRAMTIMING3_TRTP_LSB 0
#define SDR_CTRLGRP_DRAMTIMING3_TRTP_MASK 0x0000000f
/* Register template: sdr::ctrlgrp::dramtiming4                            */
#define SDR_CTRLGRP_DRAMTIMING4_MINPWRSAVECYCLES_LSB 20
#define SDR_CTRLGRP_DRAMTIMING4_MINPWRSAVECYCLES_MASK 0x00f00000
#define SDR_CTRLGRP_DRAMTIMING4_PWRDOWNEXIT_LSB 10
#define SDR_CTRLGRP_DRAMTIMING4_PWRDOWNEXIT_MASK 0x000ffc00
#define SDR_CTRLGRP_DRAMTIMING4_SELFRFSHEXIT_LSB 0
#define SDR_CTRLGRP_DRAMTIMING4_SELFRFSHEXIT_MASK 0x000003ff
/* Register template: sdr::ctrlgrp::lowpwrtiming                           */
#define SDR_CTRLGRP_LOWPWRTIMING_CLKDISABLECYCLES_LSB 16
#define SDR_CTRLGRP_LOWPWRTIMING_CLKDISABLECYCLES_MASK 0x000f0000
#define SDR_CTRLGRP_LOWPWRTIMING_AUTOPDCYCLES_LSB 0
#define SDR_CTRLGRP_LOWPWRTIMING_AUTOPDCYCLES_MASK 0x0000ffff
/* Register template: sdr::ctrlgrp::dramaddrw                              */
#define SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB 13
#define SDR_CTRLGRP_DRAMADDRW_CSBITS_MASK 0x0000e000
#define SDR_CTRLGRP_DRAMADDRW_BANKBITS_LSB 10
#define SDR_CTRLGRP_DRAMADDRW_BANKBITS_MASK 0x00001c00
#define SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB 5
#define SDR_CTRLGRP_DRAMADDRW_ROWBITS_MASK 0x000003e0
#define SDR_CTRLGRP_DRAMADDRW_COLBITS_LSB 0
#define SDR_CTRLGRP_DRAMADDRW_COLBITS_MASK 0x0000001f
/* Register template: sdr::ctrlgrp::dramifwidth                            */
#define SDR_CTRLGRP_DRAMIFWIDTH_IFWIDTH_LSB 0
#define SDR_CTRLGRP_DRAMIFWIDTH_IFWIDTH_MASK 0x000000ff
/* Register template: sdr::ctrlgrp::dramdevwidth                           */
#define SDR_CTRLGRP_DRAMDEVWIDTH_DEVWIDTH_LSB 0
#define SDR_CTRLGRP_DRAMDEVWIDTH_DEVWIDTH_MASK 0x0000000f
/* Register template: sdr::ctrlgrp::dramintr                               */
#define SDR_CTRLGRP_DRAMINTR_INTREN_LSB 0
#define SDR_CTRLGRP_DRAMINTR_INTREN_MASK 0x00000001
#define SDR_CTRLGRP_LOWPWREQ_SELFRFSHMASK_LSB 4
#define SDR_CTRLGRP_LOWPWREQ_SELFRFSHMASK_MASK 0x00000030
/* Register template: sdr::ctrlgrp::staticcfg                              */
#define SDR_CTRLGRP_STATICCFG_APPLYCFG_LSB 3
#define SDR_CTRLGRP_STATICCFG_APPLYCFG_MASK 0x00000008
#define SDR_CTRLGRP_STATICCFG_USEECCASDATA_LSB 2
#define SDR_CTRLGRP_STATICCFG_USEECCASDATA_MASK 0x00000004
#define SDR_CTRLGRP_STATICCFG_MEMBL_LSB 0
#define SDR_CTRLGRP_STATICCFG_MEMBL_MASK 0x00000003
/* Register template: sdr::ctrlgrp::ctrlwidth                              */
#define SDR_CTRLGRP_CTRLWIDTH_CTRLWIDTH_LSB 0
#define SDR_CTRLGRP_CTRLWIDTH_CTRLWIDTH_MASK 0x00000003
/* Register template: sdr::ctrlgrp::cportwidth                             */
#define SDR_CTRLGRP_CPORTWIDTH_CMDPORTWIDTH_LSB 0
#define SDR_CTRLGRP_CPORTWIDTH_CMDPORTWIDTH_MASK 0x000fffff
/* Register template: sdr::ctrlgrp::cportwmap                              */
#define SDR_CTRLGRP_CPORTWMAP_CPORTWFIFOMAP_LSB 0
#define SDR_CTRLGRP_CPORTWMAP_CPORTWFIFOMAP_MASK 0x3fffffff
/* Register template: sdr::ctrlgrp::cportrmap                              */
#define SDR_CTRLGRP_CPORTRMAP_CPORTRFIFOMAP_LSB 0
#define SDR_CTRLGRP_CPORTRMAP_CPORTRFIFOMAP_MASK 0x3fffffff
/* Register template: sdr::ctrlgrp::rfifocmap                              */
#define SDR_CTRLGRP_RFIFOCMAP_RFIFOCPORTMAP_LSB 0
#define SDR_CTRLGRP_RFIFOCMAP_RFIFOCPORTMAP_MASK 0x00ffffff
/* Register template: sdr::ctrlgrp::wfifocmap                              */
#define SDR_CTRLGRP_WFIFOCMAP_WFIFOCPORTMAP_LSB 0
#define SDR_CTRLGRP_WFIFOCMAP_WFIFOCPORTMAP_MASK 0x00ffffff
/* Register template: sdr::ctrlgrp::cportrdwr                              */
#define SDR_CTRLGRP_CPORTRDWR_CPORTRDWR_LSB 0
#define SDR_CTRLGRP_CPORTRDWR_CPORTRDWR_MASK 0x000fffff
/* Register template: sdr::ctrlgrp::portcfg                                */
#define SDR_CTRLGRP_PORTCFG_AUTOPCHEN_LSB 10
#define SDR_CTRLGRP_PORTCFG_AUTOPCHEN_MASK 0x000ffc00
#define SDR_CTRLGRP_PORTCFG_PORTPROTOCOL_LSB 0
#define SDR_CTRLGRP_PORTCFG_PORTPROTOCOL_MASK 0x000003ff
/* Register template: sdr::ctrlgrp::fifocfg                                */
#define SDR_CTRLGRP_FIFOCFG_INCSYNC_LSB 10
#define SDR_CTRLGRP_FIFOCFG_INCSYNC_MASK 0x00000400
#define SDR_CTRLGRP_FIFOCFG_SYNCMODE_LSB 0
#define SDR_CTRLGRP_FIFOCFG_SYNCMODE_MASK 0x000003ff
/* Register template: sdr::ctrlgrp::mppriority                             */
#define SDR_CTRLGRP_MPPRIORITY_USERPRIORITY_LSB 0
#define SDR_CTRLGRP_MPPRIORITY_USERPRIORITY_MASK 0x3fffffff
/* Register template: sdr::ctrlgrp::mpweight::mpweight_0                   */
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_0_STATICWEIGHT_31_0_LSB 0
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_0_STATICWEIGHT_31_0_MASK 0xffffffff
/* Register template: sdr::ctrlgrp::mpweight::mpweight_1                   */
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_SUMOFWEIGHTS_13_0_LSB 18
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_SUMOFWEIGHTS_13_0_MASK 0xfffc0000
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_STATICWEIGHT_49_32_LSB 0
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_STATICWEIGHT_49_32_MASK 0x0003ffff
/* Register template: sdr::ctrlgrp::mpweight::mpweight_2                   */
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_2_SUMOFWEIGHTS_45_14_LSB 0
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_2_SUMOFWEIGHTS_45_14_MASK 0xffffffff
/* Register template: sdr::ctrlgrp::mpweight::mpweight_3                   */
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_3_SUMOFWEIGHTS_63_46_LSB 0
#define SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_3_SUMOFWEIGHTS_63_46_MASK 0x0003ffff
/* Register template: sdr::ctrlgrp::mppacing::mppacing_0                   */
#define SDR_CTRLGRP_MPPACING_MPPACING_0_THRESHOLD1_31_0_LSB 0
#define SDR_CTRLGRP_MPPACING_MPPACING_0_THRESHOLD1_31_0_MASK 0xffffffff
/* Register template: sdr::ctrlgrp::mppacing::mppacing_1                   */
#define SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD2_3_0_LSB 28
#define SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD2_3_0_MASK 0xf0000000
#define SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD1_59_32_LSB 0
#define SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD1_59_32_MASK 0x0fffffff
/* Register template: sdr::ctrlgrp::mppacing::mppacing_2                   */
#define SDR_CTRLGRP_MPPACING_MPPACING_2_THRESHOLD2_35_4_LSB 0
#define SDR_CTRLGRP_MPPACING_MPPACING_2_THRESHOLD2_35_4_MASK 0xffffffff
/* Register template: sdr::ctrlgrp::mppacing::mppacing_3                   */
#define SDR_CTRLGRP_MPPACING_MPPACING_3_THRESHOLD2_59_36_LSB 0
#define SDR_CTRLGRP_MPPACING_MPPACING_3_THRESHOLD2_59_36_MASK 0x00ffffff
/* Register template: sdr::ctrlgrp::mpthresholdrst::mpthresholdrst_0       */
#define \
SDR_CTRLGRP_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0_LSB 0
#define  \
SDR_CTRLGRP_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0_MASK \
0xffffffff
/* Register template: sdr::ctrlgrp::mpthresholdrst::mpthresholdrst_1       */
#define \
SDR_CTRLGRP_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32_LSB 0
#define \
SDR_CTRLGRP_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32_MASK \
0xffffffff
/* Register template: sdr::ctrlgrp::mpthresholdrst::mpthresholdrst_2       */
#define \
SDR_CTRLGRP_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64_LSB 0
#define \
SDR_CTRLGRP_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64_MASK \
0x0000ffff
/* Register template: sdr::ctrlgrp::remappriority                          */
#define SDR_CTRLGRP_REMAPPRIORITY_PRIORITYREMAP_LSB 0
#define SDR_CTRLGRP_REMAPPRIORITY_PRIORITYREMAP_MASK 0x000000ff
/* Register template: sdr::ctrlgrp::phyctrl::phyctrl_0                     */
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_LSB 12
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_WIDTH 20
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_SET(x) \
 (((x) << 12) & 0xfffff000)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ADDLATSEL_SET(x) \
 (((x) << 10) & 0x00000c00)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSLOGICDELAYEN_SET(x) \
 (((x) << 6) & 0x000000c0)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_RESETDELAYEN_SET(x) \
 (((x) << 8) & 0x00000100)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_LPDDRDIS_SET(x) \
 (((x) << 9) & 0x00000200)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSDELAYEN_SET(x) \
 (((x) << 4) & 0x00000030)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQDELAYEN_SET(x) \
 (((x) << 2) & 0x0000000c)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ACDELAYEN_SET(x) \
 (((x) << 0) & 0x00000003)
/* Register template: sdr::ctrlgrp::phyctrl::phyctrl_1                     */
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_WIDTH 20
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_SET(x) \
 (((x) << 12) & 0xfffff000)
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_SAMPLECOUNT_31_20_SET(x) \
 (((x) << 0) & 0x00000fff)
/* Register template: sdr::ctrlgrp::phyctrl::phyctrl_2                     */
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_2_LONGIDLESAMPLECOUNT_31_20_SET(x) \
 (((x) << 0) & 0x00000fff)
/* Register template: sdr::ctrlgrp::dramodt                                */
#define SDR_CTRLGRP_DRAMODT_READ_LSB 4
#define SDR_CTRLGRP_DRAMODT_READ_MASK 0x000000f0
#define SDR_CTRLGRP_DRAMODT_WRITE_LSB 0
#define SDR_CTRLGRP_DRAMODT_WRITE_MASK 0x0000000f
/* Field instance: sdr::ctrlgrp::dramsts                                   */
#define SDR_CTRLGRP_DRAMSTS_DBEERR_MASK 0x00000008
#define SDR_CTRLGRP_DRAMSTS_SBEERR_MASK 0x00000004
/* Register template: sdr::ctrlgrp::extratime1                             */
#define SDR_CTRLGRP_EXTRATIME1_RD_TO_WR_LSB 20
#define SDR_CTRLGRP_EXTRATIME1_RD_TO_WR_BC_LSB 24
#define SDR_CTRLGRP_EXTRATIME1_RD_TO_WR_DIFF_LSB 28

/* SDRAM width macro for configuration with ECC */
#define SDRAM_WIDTH_32BIT_WITH_ECC	40
#define SDRAM_WIDTH_16BIT_WITH_ECC	24

#endif
#endif /* _SOCFPGA_SDRAM_GEN5_H_ */
