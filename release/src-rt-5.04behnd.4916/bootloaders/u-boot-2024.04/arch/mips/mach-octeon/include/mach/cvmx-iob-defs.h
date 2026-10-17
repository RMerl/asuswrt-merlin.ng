/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon iob.
 */

#ifndef __CVMX_IOB_DEFS_H__
#define __CVMX_IOB_DEFS_H__

#define CVMX_IOB_BIST_STATUS		(0x00011800F00007F8ull)
#define CVMX_IOB_CHIP_CUR_PWR		(0x00011800F0000828ull)
#define CVMX_IOB_CHIP_GLB_PWR_THROTTLE	(0x00011800F0000808ull)
#define CVMX_IOB_CHIP_PWR_OUT		(0x00011800F0000818ull)
#define CVMX_IOB_CTL_STATUS		(0x00011800F0000050ull)
#define CVMX_IOB_DWB_PRI_CNT		(0x00011800F0000028ull)
#define CVMX_IOB_FAU_TIMEOUT		(0x00011800F0000000ull)
#define CVMX_IOB_I2C_PRI_CNT		(0x00011800F0000010ull)
#define CVMX_IOB_INB_CONTROL_MATCH	(0x00011800F0000078ull)
#define CVMX_IOB_INB_CONTROL_MATCH_ENB	(0x00011800F0000088ull)
#define CVMX_IOB_INB_DATA_MATCH		(0x00011800F0000070ull)
#define CVMX_IOB_INB_DATA_MATCH_ENB	(0x00011800F0000080ull)
#define CVMX_IOB_INT_ENB		(0x00011800F0000060ull)
#define CVMX_IOB_INT_SUM		(0x00011800F0000058ull)
#define CVMX_IOB_N2C_L2C_PRI_CNT	(0x00011800F0000020ull)
#define CVMX_IOB_N2C_RSP_PRI_CNT	(0x00011800F0000008ull)
#define CVMX_IOB_OUTB_COM_PRI_CNT	(0x00011800F0000040ull)
#define CVMX_IOB_OUTB_CONTROL_MATCH	(0x00011800F0000098ull)
#define CVMX_IOB_OUTB_CONTROL_MATCH_ENB (0x00011800F00000A8ull)
#define CVMX_IOB_OUTB_DATA_MATCH	(0x00011800F0000090ull)
#define CVMX_IOB_OUTB_DATA_MATCH_ENB	(0x00011800F00000A0ull)
#define CVMX_IOB_OUTB_FPA_PRI_CNT	(0x00011800F0000048ull)
#define CVMX_IOB_OUTB_REQ_PRI_CNT	(0x00011800F0000038ull)
#define CVMX_IOB_P2C_REQ_PRI_CNT	(0x00011800F0000018ull)
#define CVMX_IOB_PKT_ERR		(0x00011800F0000068ull)
#define CVMX_IOB_PP_BIST_STATUS		(0x00011800F0000700ull)
#define CVMX_IOB_TO_CMB_CREDITS		(0x00011800F00000B0ull)
#define CVMX_IOB_TO_NCB_DID_00_CREDITS	(0x00011800F0000800ull)
#define CVMX_IOB_TO_NCB_DID_111_CREDITS (0x00011800F0000B78ull)
#define CVMX_IOB_TO_NCB_DID_223_CREDITS (0x00011800F0000EF8ull)
#define CVMX_IOB_TO_NCB_DID_24_CREDITS	(0x00011800F00008C0ull)
#define CVMX_IOB_TO_NCB_DID_32_CREDITS	(0x00011800F0000900ull)
#define CVMX_IOB_TO_NCB_DID_40_CREDITS	(0x00011800F0000940ull)
#define CVMX_IOB_TO_NCB_DID_55_CREDITS	(0x00011800F00009B8ull)
#define CVMX_IOB_TO_NCB_DID_64_CREDITS	(0x00011800F0000A00ull)
#define CVMX_IOB_TO_NCB_DID_79_CREDITS	(0x00011800F0000A78ull)
#define CVMX_IOB_TO_NCB_DID_96_CREDITS	(0x00011800F0000B00ull)
#define CVMX_IOB_TO_NCB_DID_98_CREDITS	(0x00011800F0000B10ull)

/**
 * cvmx_iob_bist_status
 *
 * The result of the BIST run on the IOB memories.
 *
 */
union cvmx_iob_bist_status {
	u64 u64;
	struct cvmx_iob_bist_status_s {
		u64 reserved_2_63 : 62;
		u64 ibd : 1;
		u64 icd : 1;
	} s;
	struct cvmx_iob_bist_status_cn30xx {
		u64 reserved_18_63 : 46;
		u64 icnrcb : 1;
		u64 icr0 : 1;
		u64 icr1 : 1;
		u64 icnr1 : 1;
		u64 icnr0 : 1;
		u64 ibdr0 : 1;
		u64 ibdr1 : 1;
		u64 ibr0 : 1;
		u64 ibr1 : 1;
		u64 icnrt : 1;
		u64 ibrq0 : 1;
		u64 ibrq1 : 1;
		u64 icrn0 : 1;
		u64 icrn1 : 1;
		u64 icrp0 : 1;
		u64 icrp1 : 1;
		u64 ibd : 1;
		u64 icd : 1;
	} cn30xx;
	struct cvmx_iob_bist_status_cn30xx cn31xx;
	struct cvmx_iob_bist_status_cn30xx cn38xx;
	struct cvmx_iob_bist_status_cn30xx cn38xxp2;
	struct cvmx_iob_bist_status_cn30xx cn50xx;
	struct cvmx_iob_bist_status_cn30xx cn52xx;
	struct cvmx_iob_bist_status_cn30xx cn52xxp1;
	struct cvmx_iob_bist_status_cn30xx cn56xx;
	struct cvmx_iob_bist_status_cn30xx cn56xxp1;
	struct cvmx_iob_bist_status_cn30xx cn58xx;
	struct cvmx_iob_bist_status_cn30xx cn58xxp1;
	struct cvmx_iob_bist_status_cn61xx {
		u64 reserved_23_63 : 41;
		u64 xmdfif : 1;
		u64 xmcfif : 1;
		u64 iorfif : 1;
		u64 rsdfif : 1;
		u64 iocfif : 1;
		u64 icnrcb : 1;
		u64 icr0 : 1;
		u64 icr1 : 1;
		u64 icnr1 : 1;
		u64 icnr0 : 1;
		u64 ibdr0 : 1;
		u64 ibdr1 : 1;
		u64 ibr0 : 1;
		u64 ibr1 : 1;
		u64 icnrt : 1;
		u64 ibrq0 : 1;
		u64 ibrq1 : 1;
		u64 icrn0 : 1;
		u64 icrn1 : 1;
		u64 icrp0 : 1;
		u64 icrp1 : 1;
		u64 ibd : 1;
		u64 icd : 1;
	} cn61xx;
	struct cvmx_iob_bist_status_cn61xx cn63xx;
	struct cvmx_iob_bist_status_cn61xx cn63xxp1;
	struct cvmx_iob_bist_status_cn61xx cn66xx;
	struct cvmx_iob_bist_status_cn68xx {
		u64 reserved_18_63 : 46;
		u64 xmdfif : 1;
		u64 xmcfif : 1;
		u64 iorfif : 1;
		u64 rsdfif : 1;
		u64 iocfif : 1;
		u64 icnrcb : 1;
		u64 icr0 : 1;
		u64 icr1 : 1;
		u64 icnr0 : 1;
		u64 ibr0 : 1;
		u64 ibr1 : 1;
		u64 icnrt : 1;
		u64 ibrq0 : 1;
		u64 ibrq1 : 1;
		u64 icrn0 : 1;
		u64 icrn1 : 1;
		u64 ibd : 1;
		u64 icd : 1;
	} cn68xx;
	struct cvmx_iob_bist_status_cn68xx cn68xxp1;
	struct cvmx_iob_bist_status_cn61xx cn70xx;
	struct cvmx_iob_bist_status_cn61xx cn70xxp1;
	struct cvmx_iob_bist_status_cn61xx cnf71xx;
};

typedef union cvmx_iob_bist_status cvmx_iob_bist_status_t;

/**
 * cvmx_iob_chip_cur_pwr
 */
union cvmx_iob_chip_cur_pwr {
	u64 u64;
	struct cvmx_iob_chip_cur_pwr_s {
		u64 reserved_8_63 : 56;
		u64 current_power_setting : 8;
	} s;
	struct cvmx_iob_chip_cur_pwr_s cn70xx;
	struct cvmx_iob_chip_cur_pwr_s cn70xxp1;
};

typedef union cvmx_iob_chip_cur_pwr cvmx_iob_chip_cur_pwr_t;

/**
 * cvmx_iob_chip_glb_pwr_throttle
 *
 * Controls the min/max power settings.
 *
 */
union cvmx_iob_chip_glb_pwr_throttle {
	u64 u64;
	struct cvmx_iob_chip_glb_pwr_throttle_s {
		u64 reserved_34_63 : 30;
		u64 pwr_bw : 2;
		u64 pwr_max : 8;
		u64 pwr_min : 8;
		u64 pwr_setting : 16;
	} s;
	struct cvmx_iob_chip_glb_pwr_throttle_s cn70xx;
	struct cvmx_iob_chip_glb_pwr_throttle_s cn70xxp1;
};

typedef union cvmx_iob_chip_glb_pwr_throttle cvmx_iob_chip_glb_pwr_throttle_t;

/**
 * cvmx_iob_chip_pwr_out
 *
 * Power numbers from the various partitions on the chip.
 *
 */
union cvmx_iob_chip_pwr_out {
	u64 u64;
	struct cvmx_iob_chip_pwr_out_s {
		u64 cpu_pwr : 16;
		u64 chip_power : 16;
		u64 coproc_power : 16;
		u64 avg_chip_power : 16;
	} s;
	struct cvmx_iob_chip_pwr_out_s cn70xx;
	struct cvmx_iob_chip_pwr_out_s cn70xxp1;
};

typedef union cvmx_iob_chip_pwr_out cvmx_iob_chip_pwr_out_t;

/**
 * cvmx_iob_ctl_status
 *
 * IOB Control Status = IOB Control and Status Register
 * Provides control for IOB functions.
 */
union cvmx_iob_ctl_status {
	u64 u64;
	struct cvmx_iob_ctl_status_s {
		u64 reserved_11_63 : 53;
		u64 fif_dly : 1;
		u64 xmc_per : 4;
		u64 reserved_3_5 : 3;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} s;
	struct cvmx_iob_ctl_status_cn30xx {
		u64 reserved_5_63 : 59;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn30xx;
	struct cvmx_iob_ctl_status_cn30xx cn31xx;
	struct cvmx_iob_ctl_status_cn30xx cn38xx;
	struct cvmx_iob_ctl_status_cn30xx cn38xxp2;
	struct cvmx_iob_ctl_status_cn30xx cn50xx;
	struct cvmx_iob_ctl_status_cn52xx {
		u64 reserved_6_63 : 58;
		u64 rr_mode : 1;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn52xx;
	struct cvmx_iob_ctl_status_cn30xx cn52xxp1;
	struct cvmx_iob_ctl_status_cn30xx cn56xx;
	struct cvmx_iob_ctl_status_cn30xx cn56xxp1;
	struct cvmx_iob_ctl_status_cn30xx cn58xx;
	struct cvmx_iob_ctl_status_cn30xx cn58xxp1;
	struct cvmx_iob_ctl_status_cn61xx {
		u64 reserved_11_63 : 53;
		u64 fif_dly : 1;
		u64 xmc_per : 4;
		u64 rr_mode : 1;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn61xx;
	struct cvmx_iob_ctl_status_cn63xx {
		u64 reserved_10_63 : 54;
		u64 xmc_per : 4;
		u64 rr_mode : 1;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn63xx;
	struct cvmx_iob_ctl_status_cn63xx cn63xxp1;
	struct cvmx_iob_ctl_status_cn61xx cn66xx;
	struct cvmx_iob_ctl_status_cn68xx {
		u64 reserved_11_63 : 53;
		u64 fif_dly : 1;
		u64 xmc_per : 4;
		u64 rsvr5 : 1;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn68xx;
	struct cvmx_iob_ctl_status_cn68xx cn68xxp1;
	struct cvmx_iob_ctl_status_cn70xx {
		u64 reserved_10_63 : 54;
		u64 xmc_per : 4;
		u64 rr_mode : 1;
		u64 rsv4 : 1;
		u64 rsv3 : 1;
		u64 pko_enb : 1;
		u64 dwb_enb : 1;
		u64 fau_end : 1;
	} cn70xx;
	struct cvmx_iob_ctl_status_cn70xx cn70xxp1;
	struct cvmx_iob_ctl_status_cn61xx cnf71xx;
};

typedef union cvmx_iob_ctl_status cvmx_iob_ctl_status_t;

/**
 * cvmx_iob_dwb_pri_cnt
 *
 * DWB To CMB Priority Counter = Don't Write Back to CMB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Don't Write Back request to
 * the L2C.
 */
union cvmx_iob_dwb_pri_cnt {
	u64 u64;
	struct cvmx_iob_dwb_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_dwb_pri_cnt_s cn38xx;
	struct cvmx_iob_dwb_pri_cnt_s cn38xxp2;
	struct cvmx_iob_dwb_pri_cnt_s cn52xx;
	struct cvmx_iob_dwb_pri_cnt_s cn52xxp1;
	struct cvmx_iob_dwb_pri_cnt_s cn56xx;
	struct cvmx_iob_dwb_pri_cnt_s cn56xxp1;
	struct cvmx_iob_dwb_pri_cnt_s cn58xx;
	struct cvmx_iob_dwb_pri_cnt_s cn58xxp1;
	struct cvmx_iob_dwb_pri_cnt_s cn61xx;
	struct cvmx_iob_dwb_pri_cnt_s cn63xx;
	struct cvmx_iob_dwb_pri_cnt_s cn63xxp1;
	struct cvmx_iob_dwb_pri_cnt_s cn66xx;
	struct cvmx_iob_dwb_pri_cnt_s cn70xx;
	struct cvmx_iob_dwb_pri_cnt_s cn70xxp1;
	struct cvmx_iob_dwb_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_dwb_pri_cnt cvmx_iob_dwb_pri_cnt_t;

/**
 * cvmx_iob_fau_timeout
 *
 * FAU Timeout = Fetch and Add Unit Tag-Switch Timeout
 * How many clokc ticks the FAU unit will wait for a tag-switch before timing out.
 * for Queue 0.
 */
union cvmx_iob_fau_timeout {
	u64 u64;
	struct cvmx_iob_fau_timeout_s {
		u64 reserved_13_63 : 51;
		u64 tout_enb : 1;
		u64 tout_val : 12;
	} s;
	struct cvmx_iob_fau_timeout_s cn30xx;
	struct cvmx_iob_fau_timeout_s cn31xx;
	struct cvmx_iob_fau_timeout_s cn38xx;
	struct cvmx_iob_fau_timeout_s cn38xxp2;
	struct cvmx_iob_fau_timeout_s cn50xx;
	struct cvmx_iob_fau_timeout_s cn52xx;
	struct cvmx_iob_fau_timeout_s cn52xxp1;
	struct cvmx_iob_fau_timeout_s cn56xx;
	struct cvmx_iob_fau_timeout_s cn56xxp1;
	struct cvmx_iob_fau_timeout_s cn58xx;
	struct cvmx_iob_fau_timeout_s cn58xxp1;
	struct cvmx_iob_fau_timeout_s cn61xx;
	struct cvmx_iob_fau_timeout_s cn63xx;
	struct cvmx_iob_fau_timeout_s cn63xxp1;
	struct cvmx_iob_fau_timeout_s cn66xx;
	struct cvmx_iob_fau_timeout_s cn68xx;
	struct cvmx_iob_fau_timeout_s cn68xxp1;
	struct cvmx_iob_fau_timeout_s cn70xx;
	struct cvmx_iob_fau_timeout_s cn70xxp1;
	struct cvmx_iob_fau_timeout_s cnf71xx;
};

typedef union cvmx_iob_fau_timeout cvmx_iob_fau_timeout_t;

/**
 * cvmx_iob_i2c_pri_cnt
 *
 * IPD To CMB Store Priority Counter = IPD to CMB Store Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of IPD Store access to the
 * CMB.
 */
union cvmx_iob_i2c_pri_cnt {
	u64 u64;
	struct cvmx_iob_i2c_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_i2c_pri_cnt_s cn38xx;
	struct cvmx_iob_i2c_pri_cnt_s cn38xxp2;
	struct cvmx_iob_i2c_pri_cnt_s cn52xx;
	struct cvmx_iob_i2c_pri_cnt_s cn52xxp1;
	struct cvmx_iob_i2c_pri_cnt_s cn56xx;
	struct cvmx_iob_i2c_pri_cnt_s cn56xxp1;
	struct cvmx_iob_i2c_pri_cnt_s cn58xx;
	struct cvmx_iob_i2c_pri_cnt_s cn58xxp1;
	struct cvmx_iob_i2c_pri_cnt_s cn61xx;
	struct cvmx_iob_i2c_pri_cnt_s cn63xx;
	struct cvmx_iob_i2c_pri_cnt_s cn63xxp1;
	struct cvmx_iob_i2c_pri_cnt_s cn66xx;
	struct cvmx_iob_i2c_pri_cnt_s cn70xx;
	struct cvmx_iob_i2c_pri_cnt_s cn70xxp1;
	struct cvmx_iob_i2c_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_i2c_pri_cnt cvmx_iob_i2c_pri_cnt_t;

/**
 * cvmx_iob_inb_control_match
 *
 * Match pattern for the inbound control to set the INB_MATCH_BIT.
 *
 */
union cvmx_iob_inb_control_match {
	u64 u64;
	struct cvmx_iob_inb_control_match_s {
		u64 reserved_29_63 : 35;
		u64 mask : 8;
		u64 opc : 4;
		u64 dst : 9;
		u64 src : 8;
	} s;
	struct cvmx_iob_inb_control_match_s cn30xx;
	struct cvmx_iob_inb_control_match_s cn31xx;
	struct cvmx_iob_inb_control_match_s cn38xx;
	struct cvmx_iob_inb_control_match_s cn38xxp2;
	struct cvmx_iob_inb_control_match_s cn50xx;
	struct cvmx_iob_inb_control_match_s cn52xx;
	struct cvmx_iob_inb_control_match_s cn52xxp1;
	struct cvmx_iob_inb_control_match_s cn56xx;
	struct cvmx_iob_inb_control_match_s cn56xxp1;
	struct cvmx_iob_inb_control_match_s cn58xx;
	struct cvmx_iob_inb_control_match_s cn58xxp1;
	struct cvmx_iob_inb_control_match_s cn61xx;
	struct cvmx_iob_inb_control_match_s cn63xx;
	struct cvmx_iob_inb_control_match_s cn63xxp1;
	struct cvmx_iob_inb_control_match_s cn66xx;
	struct cvmx_iob_inb_control_match_s cn68xx;
	struct cvmx_iob_inb_control_match_s cn68xxp1;
	struct cvmx_iob_inb_control_match_s cn70xx;
	struct cvmx_iob_inb_control_match_s cn70xxp1;
	struct cvmx_iob_inb_control_match_s cnf71xx;
};

typedef union cvmx_iob_inb_control_match cvmx_iob_inb_control_match_t;

/**
 * cvmx_iob_inb_control_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_INB_CONTROL_MATCH reister.
 *
 */
union cvmx_iob_inb_control_match_enb {
	u64 u64;
	struct cvmx_iob_inb_control_match_enb_s {
		u64 reserved_29_63 : 35;
		u64 mask : 8;
		u64 opc : 4;
		u64 dst : 9;
		u64 src : 8;
	} s;
	struct cvmx_iob_inb_control_match_enb_s cn30xx;
	struct cvmx_iob_inb_control_match_enb_s cn31xx;
	struct cvmx_iob_inb_control_match_enb_s cn38xx;
	struct cvmx_iob_inb_control_match_enb_s cn38xxp2;
	struct cvmx_iob_inb_control_match_enb_s cn50xx;
	struct cvmx_iob_inb_control_match_enb_s cn52xx;
	struct cvmx_iob_inb_control_match_enb_s cn52xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn56xx;
	struct cvmx_iob_inb_control_match_enb_s cn56xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn58xx;
	struct cvmx_iob_inb_control_match_enb_s cn58xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn61xx;
	struct cvmx_iob_inb_control_match_enb_s cn63xx;
	struct cvmx_iob_inb_control_match_enb_s cn63xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn66xx;
	struct cvmx_iob_inb_control_match_enb_s cn68xx;
	struct cvmx_iob_inb_control_match_enb_s cn68xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn70xx;
	struct cvmx_iob_inb_control_match_enb_s cn70xxp1;
	struct cvmx_iob_inb_control_match_enb_s cnf71xx;
};

typedef union cvmx_iob_inb_control_match_enb cvmx_iob_inb_control_match_enb_t;

/**
 * cvmx_iob_inb_data_match
 *
 * Match pattern for the inbound data to set the INB_MATCH_BIT.
 *
 */
union cvmx_iob_inb_data_match {
	u64 u64;
	struct cvmx_iob_inb_data_match_s {
		u64 data : 64;
	} s;
	struct cvmx_iob_inb_data_match_s cn30xx;
	struct cvmx_iob_inb_data_match_s cn31xx;
	struct cvmx_iob_inb_data_match_s cn38xx;
	struct cvmx_iob_inb_data_match_s cn38xxp2;
	struct cvmx_iob_inb_data_match_s cn50xx;
	struct cvmx_iob_inb_data_match_s cn52xx;
	struct cvmx_iob_inb_data_match_s cn52xxp1;
	struct cvmx_iob_inb_data_match_s cn56xx;
	struct cvmx_iob_inb_data_match_s cn56xxp1;
	struct cvmx_iob_inb_data_match_s cn58xx;
	struct cvmx_iob_inb_data_match_s cn58xxp1;
	struct cvmx_iob_inb_data_match_s cn61xx;
	struct cvmx_iob_inb_data_match_s cn63xx;
	struct cvmx_iob_inb_data_match_s cn63xxp1;
	struct cvmx_iob_inb_data_match_s cn66xx;
	struct cvmx_iob_inb_data_match_s cn68xx;
	struct cvmx_iob_inb_data_match_s cn68xxp1;
	struct cvmx_iob_inb_data_match_s cn70xx;
	struct cvmx_iob_inb_data_match_s cn70xxp1;
	struct cvmx_iob_inb_data_match_s cnf71xx;
};

typedef union cvmx_iob_inb_data_match cvmx_iob_inb_data_match_t;

/**
 * cvmx_iob_inb_data_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_INB_DATA_MATCH reister.
 *
 */
union cvmx_iob_inb_data_match_enb {
	u64 u64;
	struct cvmx_iob_inb_data_match_enb_s {
		u64 data : 64;
	} s;
	struct cvmx_iob_inb_data_match_enb_s cn30xx;
	struct cvmx_iob_inb_data_match_enb_s cn31xx;
	struct cvmx_iob_inb_data_match_enb_s cn38xx;
	struct cvmx_iob_inb_data_match_enb_s cn38xxp2;
	struct cvmx_iob_inb_data_match_enb_s cn50xx;
	struct cvmx_iob_inb_data_match_enb_s cn52xx;
	struct cvmx_iob_inb_data_match_enb_s cn52xxp1;
	struct cvmx_iob_inb_data_match_enb_s cn56xx;
	struct cvmx_iob_inb_data_match_enb_s cn56xxp1;
	struct cvmx_iob_inb_data_match_enb_s cn58xx;
	struct cvmx_iob_inb_data_match_enb_s cn58xxp1;
	struct cvmx_iob_inb_data_match_enb_s cn61xx;
	struct cvmx_iob_inb_data_match_enb_s cn63xx;
	struct cvmx_iob_inb_data_match_enb_s cn63xxp1;
	struct cvmx_iob_inb_data_match_enb_s cn66xx;
	struct cvmx_iob_inb_data_match_enb_s cn68xx;
	struct cvmx_iob_inb_data_match_enb_s cn68xxp1;
	struct cvmx_iob_inb_data_match_enb_s cn70xx;
	struct cvmx_iob_inb_data_match_enb_s cn70xxp1;
	struct cvmx_iob_inb_data_match_enb_s cnf71xx;
};

typedef union cvmx_iob_inb_data_match_enb cvmx_iob_inb_data_match_enb_t;

/**
 * cvmx_iob_int_enb
 *
 * The IOB's interrupt enable register.
 *
 */
union cvmx_iob_int_enb {
	u64 u64;
	struct cvmx_iob_int_enb_s {
		u64 reserved_8_63 : 56;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 p_dat : 1;
		u64 np_dat : 1;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} s;
	struct cvmx_iob_int_enb_cn30xx {
		u64 reserved_4_63 : 60;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} cn30xx;
	struct cvmx_iob_int_enb_cn30xx cn31xx;
	struct cvmx_iob_int_enb_cn30xx cn38xx;
	struct cvmx_iob_int_enb_cn30xx cn38xxp2;
	struct cvmx_iob_int_enb_cn50xx {
		u64 reserved_6_63 : 58;
		u64 p_dat : 1;
		u64 np_dat : 1;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} cn50xx;
	struct cvmx_iob_int_enb_cn50xx cn52xx;
	struct cvmx_iob_int_enb_cn50xx cn52xxp1;
	struct cvmx_iob_int_enb_cn50xx cn56xx;
	struct cvmx_iob_int_enb_cn50xx cn56xxp1;
	struct cvmx_iob_int_enb_cn50xx cn58xx;
	struct cvmx_iob_int_enb_cn50xx cn58xxp1;
	struct cvmx_iob_int_enb_cn50xx cn61xx;
	struct cvmx_iob_int_enb_cn50xx cn63xx;
	struct cvmx_iob_int_enb_cn50xx cn63xxp1;
	struct cvmx_iob_int_enb_cn50xx cn66xx;
	struct cvmx_iob_int_enb_cn68xx {
		u64 reserved_0_63 : 64;
	} cn68xx;
	struct cvmx_iob_int_enb_cn68xx cn68xxp1;
	struct cvmx_iob_int_enb_s cn70xx;
	struct cvmx_iob_int_enb_s cn70xxp1;
	struct cvmx_iob_int_enb_cn50xx cnf71xx;
};

typedef union cvmx_iob_int_enb cvmx_iob_int_enb_t;

/**
 * cvmx_iob_int_sum
 *
 * Contains the different interrupt summary bits of the IOB.
 *
 */
union cvmx_iob_int_sum {
	u64 u64;
	struct cvmx_iob_int_sum_s {
		u64 reserved_8_63 : 56;
		u64 outb_mat : 1;
		u64 inb_mat : 1;
		u64 p_dat : 1;
		u64 np_dat : 1;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} s;
	struct cvmx_iob_int_sum_cn30xx {
		u64 reserved_4_63 : 60;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} cn30xx;
	struct cvmx_iob_int_sum_cn30xx cn31xx;
	struct cvmx_iob_int_sum_cn30xx cn38xx;
	struct cvmx_iob_int_sum_cn30xx cn38xxp2;
	struct cvmx_iob_int_sum_cn50xx {
		u64 reserved_6_63 : 58;
		u64 p_dat : 1;
		u64 np_dat : 1;
		u64 p_eop : 1;
		u64 p_sop : 1;
		u64 np_eop : 1;
		u64 np_sop : 1;
	} cn50xx;
	struct cvmx_iob_int_sum_cn50xx cn52xx;
	struct cvmx_iob_int_sum_cn50xx cn52xxp1;
	struct cvmx_iob_int_sum_cn50xx cn56xx;
	struct cvmx_iob_int_sum_cn50xx cn56xxp1;
	struct cvmx_iob_int_sum_cn50xx cn58xx;
	struct cvmx_iob_int_sum_cn50xx cn58xxp1;
	struct cvmx_iob_int_sum_cn50xx cn61xx;
	struct cvmx_iob_int_sum_cn50xx cn63xx;
	struct cvmx_iob_int_sum_cn50xx cn63xxp1;
	struct cvmx_iob_int_sum_cn50xx cn66xx;
	struct cvmx_iob_int_sum_cn68xx {
		u64 reserved_0_63 : 64;
	} cn68xx;
	struct cvmx_iob_int_sum_cn68xx cn68xxp1;
	struct cvmx_iob_int_sum_s cn70xx;
	struct cvmx_iob_int_sum_s cn70xxp1;
	struct cvmx_iob_int_sum_cn50xx cnf71xx;
};

typedef union cvmx_iob_int_sum cvmx_iob_int_sum_t;

/**
 * cvmx_iob_n2c_l2c_pri_cnt
 *
 * NCB To CMB L2C Priority Counter = NCB to CMB L2C Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of NCB Store/Load access to
 * the CMB.
 */
union cvmx_iob_n2c_l2c_pri_cnt {
	u64 u64;
	struct cvmx_iob_n2c_l2c_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn38xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn38xxp2;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn52xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn52xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn56xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn56xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn58xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn58xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn61xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn63xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn63xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn66xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn70xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cn70xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_n2c_l2c_pri_cnt cvmx_iob_n2c_l2c_pri_cnt_t;

/**
 * cvmx_iob_n2c_rsp_pri_cnt
 *
 * NCB To CMB Response Priority Counter = NCB to CMB Response Priority Counter Enable and Timer
 * Value
 * Enables and supplies the timeout count for raising the priority of NCB Responses access to the
 * CMB.
 */
union cvmx_iob_n2c_rsp_pri_cnt {
	u64 u64;
	struct cvmx_iob_n2c_rsp_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn38xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn38xxp2;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn52xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn52xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn56xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn56xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn58xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn58xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn61xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn63xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn63xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn66xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn70xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cn70xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_n2c_rsp_pri_cnt cvmx_iob_n2c_rsp_pri_cnt_t;

/**
 * cvmx_iob_outb_com_pri_cnt
 *
 * Commit To NCB Priority Counter = Commit to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Commit request to the
 * Outbound NCB.
 */
union cvmx_iob_outb_com_pri_cnt {
	u64 u64;
	struct cvmx_iob_outb_com_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_outb_com_pri_cnt_s cn38xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn38xxp2;
	struct cvmx_iob_outb_com_pri_cnt_s cn52xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn52xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cn56xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn56xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cn58xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn58xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cn61xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn63xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn63xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cn66xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn68xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn68xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cn70xx;
	struct cvmx_iob_outb_com_pri_cnt_s cn70xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_outb_com_pri_cnt cvmx_iob_outb_com_pri_cnt_t;

/**
 * cvmx_iob_outb_control_match
 *
 * Match pattern for the outbound control to set the OUTB_MATCH_BIT.
 *
 */
union cvmx_iob_outb_control_match {
	u64 u64;
	struct cvmx_iob_outb_control_match_s {
		u64 reserved_26_63 : 38;
		u64 mask : 8;
		u64 eot : 1;
		u64 dst : 8;
		u64 src : 9;
	} s;
	struct cvmx_iob_outb_control_match_s cn30xx;
	struct cvmx_iob_outb_control_match_s cn31xx;
	struct cvmx_iob_outb_control_match_s cn38xx;
	struct cvmx_iob_outb_control_match_s cn38xxp2;
	struct cvmx_iob_outb_control_match_s cn50xx;
	struct cvmx_iob_outb_control_match_s cn52xx;
	struct cvmx_iob_outb_control_match_s cn52xxp1;
	struct cvmx_iob_outb_control_match_s cn56xx;
	struct cvmx_iob_outb_control_match_s cn56xxp1;
	struct cvmx_iob_outb_control_match_s cn58xx;
	struct cvmx_iob_outb_control_match_s cn58xxp1;
	struct cvmx_iob_outb_control_match_s cn61xx;
	struct cvmx_iob_outb_control_match_s cn63xx;
	struct cvmx_iob_outb_control_match_s cn63xxp1;
	struct cvmx_iob_outb_control_match_s cn66xx;
	struct cvmx_iob_outb_control_match_s cn68xx;
	struct cvmx_iob_outb_control_match_s cn68xxp1;
	struct cvmx_iob_outb_control_match_s cn70xx;
	struct cvmx_iob_outb_control_match_s cn70xxp1;
	struct cvmx_iob_outb_control_match_s cnf71xx;
};

typedef union cvmx_iob_outb_control_match cvmx_iob_outb_control_match_t;

/**
 * cvmx_iob_outb_control_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_OUTB_CONTROL_MATCH reister.
 *
 */
union cvmx_iob_outb_control_match_enb {
	u64 u64;
	struct cvmx_iob_outb_control_match_enb_s {
		u64 reserved_26_63 : 38;
		u64 mask : 8;
		u64 eot : 1;
		u64 dst : 8;
		u64 src : 9;
	} s;
	struct cvmx_iob_outb_control_match_enb_s cn30xx;
	struct cvmx_iob_outb_control_match_enb_s cn31xx;
	struct cvmx_iob_outb_control_match_enb_s cn38xx;
	struct cvmx_iob_outb_control_match_enb_s cn38xxp2;
	struct cvmx_iob_outb_control_match_enb_s cn50xx;
	struct cvmx_iob_outb_control_match_enb_s cn52xx;
	struct cvmx_iob_outb_control_match_enb_s cn52xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn56xx;
	struct cvmx_iob_outb_control_match_enb_s cn56xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn58xx;
	struct cvmx_iob_outb_control_match_enb_s cn58xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn61xx;
	struct cvmx_iob_outb_control_match_enb_s cn63xx;
	struct cvmx_iob_outb_control_match_enb_s cn63xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn66xx;
	struct cvmx_iob_outb_control_match_enb_s cn68xx;
	struct cvmx_iob_outb_control_match_enb_s cn68xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn70xx;
	struct cvmx_iob_outb_control_match_enb_s cn70xxp1;
	struct cvmx_iob_outb_control_match_enb_s cnf71xx;
};

typedef union cvmx_iob_outb_control_match_enb cvmx_iob_outb_control_match_enb_t;

/**
 * cvmx_iob_outb_data_match
 *
 * Match pattern for the outbound data to set the OUTB_MATCH_BIT.
 *
 */
union cvmx_iob_outb_data_match {
	u64 u64;
	struct cvmx_iob_outb_data_match_s {
		u64 data : 64;
	} s;
	struct cvmx_iob_outb_data_match_s cn30xx;
	struct cvmx_iob_outb_data_match_s cn31xx;
	struct cvmx_iob_outb_data_match_s cn38xx;
	struct cvmx_iob_outb_data_match_s cn38xxp2;
	struct cvmx_iob_outb_data_match_s cn50xx;
	struct cvmx_iob_outb_data_match_s cn52xx;
	struct cvmx_iob_outb_data_match_s cn52xxp1;
	struct cvmx_iob_outb_data_match_s cn56xx;
	struct cvmx_iob_outb_data_match_s cn56xxp1;
	struct cvmx_iob_outb_data_match_s cn58xx;
	struct cvmx_iob_outb_data_match_s cn58xxp1;
	struct cvmx_iob_outb_data_match_s cn61xx;
	struct cvmx_iob_outb_data_match_s cn63xx;
	struct cvmx_iob_outb_data_match_s cn63xxp1;
	struct cvmx_iob_outb_data_match_s cn66xx;
	struct cvmx_iob_outb_data_match_s cn68xx;
	struct cvmx_iob_outb_data_match_s cn68xxp1;
	struct cvmx_iob_outb_data_match_s cn70xx;
	struct cvmx_iob_outb_data_match_s cn70xxp1;
	struct cvmx_iob_outb_data_match_s cnf71xx;
};

typedef union cvmx_iob_outb_data_match cvmx_iob_outb_data_match_t;

/**
 * cvmx_iob_outb_data_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_OUTB_DATA_MATCH reister.
 *
 */
union cvmx_iob_outb_data_match_enb {
	u64 u64;
	struct cvmx_iob_outb_data_match_enb_s {
		u64 data : 64;
	} s;
	struct cvmx_iob_outb_data_match_enb_s cn30xx;
	struct cvmx_iob_outb_data_match_enb_s cn31xx;
	struct cvmx_iob_outb_data_match_enb_s cn38xx;
	struct cvmx_iob_outb_data_match_enb_s cn38xxp2;
	struct cvmx_iob_outb_data_match_enb_s cn50xx;
	struct cvmx_iob_outb_data_match_enb_s cn52xx;
	struct cvmx_iob_outb_data_match_enb_s cn52xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn56xx;
	struct cvmx_iob_outb_data_match_enb_s cn56xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn58xx;
	struct cvmx_iob_outb_data_match_enb_s cn58xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn61xx;
	struct cvmx_iob_outb_data_match_enb_s cn63xx;
	struct cvmx_iob_outb_data_match_enb_s cn63xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn66xx;
	struct cvmx_iob_outb_data_match_enb_s cn68xx;
	struct cvmx_iob_outb_data_match_enb_s cn68xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn70xx;
	struct cvmx_iob_outb_data_match_enb_s cn70xxp1;
	struct cvmx_iob_outb_data_match_enb_s cnf71xx;
};

typedef union cvmx_iob_outb_data_match_enb cvmx_iob_outb_data_match_enb_t;

/**
 * cvmx_iob_outb_fpa_pri_cnt
 *
 * FPA To NCB Priority Counter = FPA Returns to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of FPA Rreturn Page request to
 * the Outbound NCB.
 */
union cvmx_iob_outb_fpa_pri_cnt {
	u64 u64;
	struct cvmx_iob_outb_fpa_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn38xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn38xxp2;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn52xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn52xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn56xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn56xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn58xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn58xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn61xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn63xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn63xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn66xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn68xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn68xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn70xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s cn70xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_outb_fpa_pri_cnt cvmx_iob_outb_fpa_pri_cnt_t;

/**
 * cvmx_iob_outb_req_pri_cnt
 *
 * Request To NCB Priority Counter = Request to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Request transfers to the
 * Outbound NCB.
 */
union cvmx_iob_outb_req_pri_cnt {
	u64 u64;
	struct cvmx_iob_outb_req_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_outb_req_pri_cnt_s cn38xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn38xxp2;
	struct cvmx_iob_outb_req_pri_cnt_s cn52xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn52xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cn56xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn56xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cn58xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn58xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cn61xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn63xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn63xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cn66xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn68xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn68xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cn70xx;
	struct cvmx_iob_outb_req_pri_cnt_s cn70xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_outb_req_pri_cnt cvmx_iob_outb_req_pri_cnt_t;

/**
 * cvmx_iob_p2c_req_pri_cnt
 *
 * PKO To CMB Response Priority Counter = PKO to CMB Response Priority Counter Enable and Timer
 * Value
 * Enables and supplies the timeout count for raising the priority of PKO Load access to the CMB.
 */
union cvmx_iob_p2c_req_pri_cnt {
	u64 u64;
	struct cvmx_iob_p2c_req_pri_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt_enb : 1;
		u64 cnt_val : 15;
	} s;
	struct cvmx_iob_p2c_req_pri_cnt_s cn38xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn38xxp2;
	struct cvmx_iob_p2c_req_pri_cnt_s cn52xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn52xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s cn56xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn56xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s cn58xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn58xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s cn61xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn63xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn63xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s cn66xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn70xx;
	struct cvmx_iob_p2c_req_pri_cnt_s cn70xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s cnf71xx;
};

typedef union cvmx_iob_p2c_req_pri_cnt cvmx_iob_p2c_req_pri_cnt_t;

/**
 * cvmx_iob_pkt_err
 *
 * Provides status about the failing packet recevie error.
 *
 */
union cvmx_iob_pkt_err {
	u64 u64;
	struct cvmx_iob_pkt_err_s {
		u64 reserved_12_63 : 52;
		u64 vport : 6;
		u64 port : 6;
	} s;
	struct cvmx_iob_pkt_err_cn30xx {
		u64 reserved_6_63 : 58;
		u64 port : 6;
	} cn30xx;
	struct cvmx_iob_pkt_err_cn30xx cn31xx;
	struct cvmx_iob_pkt_err_cn30xx cn38xx;
	struct cvmx_iob_pkt_err_cn30xx cn38xxp2;
	struct cvmx_iob_pkt_err_cn30xx cn50xx;
	struct cvmx_iob_pkt_err_cn30xx cn52xx;
	struct cvmx_iob_pkt_err_cn30xx cn52xxp1;
	struct cvmx_iob_pkt_err_cn30xx cn56xx;
	struct cvmx_iob_pkt_err_cn30xx cn56xxp1;
	struct cvmx_iob_pkt_err_cn30xx cn58xx;
	struct cvmx_iob_pkt_err_cn30xx cn58xxp1;
	struct cvmx_iob_pkt_err_s cn61xx;
	struct cvmx_iob_pkt_err_s cn63xx;
	struct cvmx_iob_pkt_err_s cn63xxp1;
	struct cvmx_iob_pkt_err_s cn66xx;
	struct cvmx_iob_pkt_err_s cn70xx;
	struct cvmx_iob_pkt_err_s cn70xxp1;
	struct cvmx_iob_pkt_err_s cnf71xx;
};

typedef union cvmx_iob_pkt_err cvmx_iob_pkt_err_t;

/**
 * cvmx_iob_pp_bist_status
 *
 * The result of the BIST run on the PPs.
 *
 */
union cvmx_iob_pp_bist_status {
	u64 u64;
	struct cvmx_iob_pp_bist_status_s {
		u64 reserved_4_63 : 60;
		u64 pp_bstat : 4;
	} s;
	struct cvmx_iob_pp_bist_status_s cn70xx;
	struct cvmx_iob_pp_bist_status_s cn70xxp1;
};

typedef union cvmx_iob_pp_bist_status cvmx_iob_pp_bist_status_t;

/**
 * cvmx_iob_to_cmb_credits
 *
 * Controls the number of reads and writes that may be outstanding to the L2C (via the CMB).
 *
 */
union cvmx_iob_to_cmb_credits {
	u64 u64;
	struct cvmx_iob_to_cmb_credits_s {
		u64 reserved_6_63 : 58;
		u64 ncb_rd : 3;
		u64 ncb_wr : 3;
	} s;
	struct cvmx_iob_to_cmb_credits_cn52xx {
		u64 reserved_9_63 : 55;
		u64 pko_rd : 3;
		u64 ncb_rd : 3;
		u64 ncb_wr : 3;
	} cn52xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn61xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn63xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn63xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cn66xx;
	struct cvmx_iob_to_cmb_credits_cn68xx {
		u64 reserved_9_63 : 55;
		u64 dwb : 3;
		u64 ncb_rd : 3;
		u64 ncb_wr : 3;
	} cn68xx;
	struct cvmx_iob_to_cmb_credits_cn68xx cn68xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cn70xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn70xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cnf71xx;
};

typedef union cvmx_iob_to_cmb_credits cvmx_iob_to_cmb_credits_t;

/**
 * cvmx_iob_to_ncb_did_00_credits
 *
 * IOB_TO_NCB_DID_00_CREDITS = IOB NCB DID 00 Credits
 *
 * Number of credits for NCB DID 00.
 */
union cvmx_iob_to_ncb_did_00_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_00_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_00_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_00_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_00_credits cvmx_iob_to_ncb_did_00_credits_t;

/**
 * cvmx_iob_to_ncb_did_111_credits
 *
 * IOB_TO_NCB_DID_111_CREDITS = IOB NCB DID 111 Credits
 *
 * Number of credits for NCB DID 111.
 */
union cvmx_iob_to_ncb_did_111_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_111_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_111_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_111_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_111_credits cvmx_iob_to_ncb_did_111_credits_t;

/**
 * cvmx_iob_to_ncb_did_223_credits
 *
 * IOB_TO_NCB_DID_223_CREDITS = IOB NCB DID 223 Credits
 *
 * Number of credits for NCB DID 223.
 */
union cvmx_iob_to_ncb_did_223_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_223_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_223_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_223_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_223_credits cvmx_iob_to_ncb_did_223_credits_t;

/**
 * cvmx_iob_to_ncb_did_24_credits
 *
 * IOB_TO_NCB_DID_24_CREDITS = IOB NCB DID 24 Credits
 *
 * Number of credits for NCB DID 24.
 */
union cvmx_iob_to_ncb_did_24_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_24_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_24_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_24_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_24_credits cvmx_iob_to_ncb_did_24_credits_t;

/**
 * cvmx_iob_to_ncb_did_32_credits
 *
 * IOB_TO_NCB_DID_32_CREDITS = IOB NCB DID 32 Credits
 *
 * Number of credits for NCB DID 32.
 */
union cvmx_iob_to_ncb_did_32_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_32_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_32_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_32_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_32_credits cvmx_iob_to_ncb_did_32_credits_t;

/**
 * cvmx_iob_to_ncb_did_40_credits
 *
 * IOB_TO_NCB_DID_40_CREDITS = IOB NCB DID 40 Credits
 *
 * Number of credits for NCB DID 40.
 */
union cvmx_iob_to_ncb_did_40_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_40_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_40_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_40_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_40_credits cvmx_iob_to_ncb_did_40_credits_t;

/**
 * cvmx_iob_to_ncb_did_55_credits
 *
 * IOB_TO_NCB_DID_55_CREDITS = IOB NCB DID 55 Credits
 *
 * Number of credits for NCB DID 55.
 */
union cvmx_iob_to_ncb_did_55_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_55_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_55_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_55_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_55_credits cvmx_iob_to_ncb_did_55_credits_t;

/**
 * cvmx_iob_to_ncb_did_64_credits
 *
 * IOB_TO_NCB_DID_64_CREDITS = IOB NCB DID 64 Credits
 *
 * Number of credits for NCB DID 64.
 */
union cvmx_iob_to_ncb_did_64_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_64_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_64_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_64_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_64_credits cvmx_iob_to_ncb_did_64_credits_t;

/**
 * cvmx_iob_to_ncb_did_79_credits
 *
 * IOB_TO_NCB_DID_79_CREDITS = IOB NCB DID 79 Credits
 *
 * Number of credits for NCB DID 79.
 */
union cvmx_iob_to_ncb_did_79_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_79_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_79_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_79_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_79_credits cvmx_iob_to_ncb_did_79_credits_t;

/**
 * cvmx_iob_to_ncb_did_96_credits
 *
 * IOB_TO_NCB_DID_96_CREDITS = IOB NCB DID 96 Credits
 *
 * Number of credits for NCB DID 96.
 */
union cvmx_iob_to_ncb_did_96_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_96_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_96_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_96_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_96_credits cvmx_iob_to_ncb_did_96_credits_t;

/**
 * cvmx_iob_to_ncb_did_98_credits
 *
 * IOB_TO_NCB_DID_98_CREDITS = IOB NCB DID 96 Credits
 *
 * Number of credits for NCB DID 98.
 */
union cvmx_iob_to_ncb_did_98_credits {
	u64 u64;
	struct cvmx_iob_to_ncb_did_98_credits_s {
		u64 reserved_7_63 : 57;
		u64 crd : 7;
	} s;
	struct cvmx_iob_to_ncb_did_98_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_98_credits_s cn68xxp1;
};

typedef union cvmx_iob_to_ncb_did_98_credits cvmx_iob_to_ncb_did_98_credits_t;

#endif
