/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common internal memory map for some Freescale SoCs
 *
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_IMMAP_H
#define __FSL_IMMAP_H
/*
 * DDR memory controller registers
 * This structure works for mpc83xx (DDR2 and DDR3), mpc85xx, mpc86xx.
 */
struct ccsr_ddr {
	u32	cs0_bnds;		/* Chip Select 0 Memory Bounds */
	u8	res_04[4];
	u32	cs1_bnds;		/* Chip Select 1 Memory Bounds */
	u8	res_0c[4];
	u32	cs2_bnds;		/* Chip Select 2 Memory Bounds */
	u8	res_14[4];
	u32	cs3_bnds;		/* Chip Select 3 Memory Bounds */
	u8	res_1c[100];
	u32	cs0_config;		/* Chip Select Configuration */
	u32	cs1_config;		/* Chip Select Configuration */
	u32	cs2_config;		/* Chip Select Configuration */
	u32	cs3_config;		/* Chip Select Configuration */
	u8	res_90[48];
	u32	cs0_config_2;		/* Chip Select Configuration 2 */
	u32	cs1_config_2;		/* Chip Select Configuration 2 */
	u32	cs2_config_2;		/* Chip Select Configuration 2 */
	u32	cs3_config_2;		/* Chip Select Configuration 2 */
	u8	res_d0[48];
	u32	timing_cfg_3;		/* SDRAM Timing Configuration 3 */
	u32	timing_cfg_0;		/* SDRAM Timing Configuration 0 */
	u32	timing_cfg_1;		/* SDRAM Timing Configuration 1 */
	u32	timing_cfg_2;		/* SDRAM Timing Configuration 2 */
	u32	sdram_cfg;		/* SDRAM Control Configuration */
	u32	sdram_cfg_2;		/* SDRAM Control Configuration 2 */
	u32	sdram_mode;		/* SDRAM Mode Configuration */
	u32	sdram_mode_2;		/* SDRAM Mode Configuration 2 */
	u32	sdram_md_cntl;		/* SDRAM Mode Control */
	u32	sdram_interval;		/* SDRAM Interval Configuration */
	u32	sdram_data_init;	/* SDRAM Data initialization */
	u8	res_12c[4];
	u32	sdram_clk_cntl;		/* SDRAM Clock Control */
	u8	res_134[20];
	u32	init_addr;		/* training init addr */
	u32	init_ext_addr;		/* training init extended addr */
	u8	res_150[16];
	u32	timing_cfg_4;		/* SDRAM Timing Configuration 4 */
	u32	timing_cfg_5;		/* SDRAM Timing Configuration 5 */
	u32	timing_cfg_6;		/* SDRAM Timing Configuration 6 */
	u32	timing_cfg_7;		/* SDRAM Timing Configuration 7 */
	u32	ddr_zq_cntl;		/* ZQ calibration control*/
	u32	ddr_wrlvl_cntl;		/* write leveling control*/
	u8	reg_178[4];
	u32	ddr_sr_cntr;		/* self refresh counter */
	u32	ddr_sdram_rcw_1;	/* Control Words 1 */
	u32	ddr_sdram_rcw_2;	/* Control Words 2 */
	u8	reg_188[8];
	u32	ddr_wrlvl_cntl_2;	/* write leveling control 2 */
	u32	ddr_wrlvl_cntl_3;	/* write leveling control 3 */
	u8	res_198[0x1a0-0x198];
	u32	ddr_sdram_rcw_3;
	u32	ddr_sdram_rcw_4;
	u32	ddr_sdram_rcw_5;
	u32	ddr_sdram_rcw_6;
	u8	res_1b0[0x200-0x1b0];
	u32	sdram_mode_3;		/* SDRAM Mode Configuration 3 */
	u32	sdram_mode_4;		/* SDRAM Mode Configuration 4 */
	u32	sdram_mode_5;		/* SDRAM Mode Configuration 5 */
	u32	sdram_mode_6;		/* SDRAM Mode Configuration 6 */
	u32	sdram_mode_7;		/* SDRAM Mode Configuration 7 */
	u32	sdram_mode_8;		/* SDRAM Mode Configuration 8 */
	u8	res_218[0x220-0x218];
	u32	sdram_mode_9;		/* SDRAM Mode Configuration 9 */
	u32	sdram_mode_10;		/* SDRAM Mode Configuration 10 */
	u32	sdram_mode_11;		/* SDRAM Mode Configuration 11 */
	u32	sdram_mode_12;		/* SDRAM Mode Configuration 12 */
	u32	sdram_mode_13;		/* SDRAM Mode Configuration 13 */
	u32	sdram_mode_14;		/* SDRAM Mode Configuration 14 */
	u32	sdram_mode_15;		/* SDRAM Mode Configuration 15 */
	u32	sdram_mode_16;		/* SDRAM Mode Configuration 16 */
	u8	res_240[0x250-0x240];
	u32	timing_cfg_8;		/* SDRAM Timing Configuration 8 */
	u32	timing_cfg_9;		/* SDRAM Timing Configuration 9 */
	u8	res_258[0x260-0x258];
	u32	sdram_cfg_3;
	u8	res_264[0x400-0x264];
	u32	dq_map_0;
	u32	dq_map_1;
	u32	dq_map_2;
	u32	dq_map_3;
	u8	res_410[0xb20-0x410];
	u32	ddr_dsr1;		/* Debug Status 1 */
	u32	ddr_dsr2;		/* Debug Status 2 */
	u32	ddr_cdr1;		/* Control Driver 1 */
	u32	ddr_cdr2;		/* Control Driver 2 */
	u8	res_b30[200];
	u32	ip_rev1;		/* IP Block Revision 1 */
	u32	ip_rev2;		/* IP Block Revision 2 */
	u32	eor;			/* Enhanced Optimization Register */
	u8	res_c04[252];
	u32	mtcr;			/* Memory Test Control Register */
	u8	res_d04[28];
	u32	mtp1;			/* Memory Test Pattern 1 */
	u32	mtp2;			/* Memory Test Pattern 2 */
	u32	mtp3;			/* Memory Test Pattern 3 */
	u32	mtp4;			/* Memory Test Pattern 4 */
	u32	mtp5;			/* Memory Test Pattern 5 */
	u32	mtp6;			/* Memory Test Pattern 6 */
	u32	mtp7;			/* Memory Test Pattern 7 */
	u32	mtp8;			/* Memory Test Pattern 8 */
	u32	mtp9;			/* Memory Test Pattern 9 */
	u32	mtp10;			/* Memory Test Pattern 10 */
	u8	res_d48[184];
	u32	data_err_inject_hi;	/* Data Path Err Injection Mask High */
	u32	data_err_inject_lo;	/* Data Path Err Injection Mask Low */
	u32	ecc_err_inject;		/* Data Path Err Injection Mask ECC */
	u8	res_e0c[20];
	u32	capture_data_hi;	/* Data Path Read Capture High */
	u32	capture_data_lo;	/* Data Path Read Capture Low */
	u32	capture_ecc;		/* Data Path Read Capture ECC */
	u8	res_e2c[20];
	u32	err_detect;		/* Error Detect */
	u32	err_disable;		/* Error Disable */
	u32	err_int_en;
	u32	capture_attributes;	/* Error Attrs Capture */
	u32	capture_address;	/* Error Addr Capture */
	u32	capture_ext_address;	/* Error Extended Addr Capture */
	u32	err_sbe;		/* Single-Bit ECC Error Management */
	u8	res_e5c[164];
	u32     debug[64];		/* debug_1 to debug_64 */
};

#ifdef CONFIG_SYS_FSL_HAS_CCI400
#define CCI400_CTRLORD_TERM_BARRIER	0x00000008
#define CCI400_CTRLORD_EN_BARRIER	0
#define CCI400_SHAORD_NON_SHAREABLE	0x00000002
#define CCI400_DVM_MESSAGE_REQ_EN	0x00000002
#define CCI400_SNOOP_REQ_EN		0x00000001

/* CCI-400 registers */
struct ccsr_cci400 {
	u32 ctrl_ord;			/* Control Override */
	u32 spec_ctrl;			/* Speculation Control */
	u32 secure_access;		/* Secure Access */
	u32 status;			/* Status */
	u32 impr_err;			/* Imprecise Error */
	u8 res_14[0x100 - 0x14];
	u32 pmcr;			/* Performance Monitor Control */
	u8 res_104[0xfd0 - 0x104];
	u32 pid[8];			/* Peripheral ID */
	u32 cid[4];			/* Component ID */
	struct {
		u32 snoop_ctrl;		/* Snoop Control */
		u32 sha_ord;		/* Shareable Override */
		u8 res_1008[0x1100 - 0x1008];
		u32 rc_qos_ord;		/* read channel QoS Value Override */
		u32 wc_qos_ord;		/* read channel QoS Value Override */
		u8 res_1108[0x110c - 0x1108];
		u32 qos_ctrl;		/* QoS Control */
		u32 max_ot;		/* Max OT */
		u8 res_1114[0x1130 - 0x1114];
		u32 target_lat;		/* Target Latency */
		u32 latency_regu;	/* Latency Regulation */
		u32 qos_range;		/* QoS Range */
		u8 res_113c[0x2000 - 0x113c];
	} slave[5];			/* Slave Interface */
	u8 res_6000[0x9004 - 0x6000];
	u32 cycle_counter;		/* Cycle counter */
	u32 count_ctrl;			/* Count Control */
	u32 overflow_status;		/* Overflow Flag Status */
	u8 res_9010[0xa000 - 0x9010];
	struct {
		u32 event_select;	/* Event Select */
		u32 event_count;	/* Event Count */
		u32 counter_ctrl;	/* Counter Control */
		u32 overflow_status;	/* Overflow Flag Status */
		u8 res_a010[0xb000 - 0xa010];
	} pcounter[4];			/* Performance Counter */
	u8 res_e004[0x10000 - 0xe004];
};
#endif

#endif /* __FSL_IMMAP_H */
