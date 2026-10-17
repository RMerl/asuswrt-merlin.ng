/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_MIO_DEFS_H__
#define __CVMX_MIO_DEFS_H__

#define CVMX_MIO_PTP_CLOCK_CFG	  (0x0001070000000F00ull)
#define CVMX_MIO_PTP_EVT_CNT	  (0x0001070000000F28ull)
#define CVMX_MIO_RST_BOOT	  (0x0001180000001600ull)
#define CVMX_MIO_RST_CTLX(offset) (0x0001180000001618ull + ((offset) & 1))
#define CVMX_MIO_QLMX_CFG(offset) (0x0001180000001590ull + ((offset) & 7) * 8)

/**
 * cvmx_mio_ptp_clock_cfg
 *
 * This register configures the timestamp architecture.
 *
 */
union cvmx_mio_ptp_clock_cfg {
	u64 u64;
	struct cvmx_mio_ptp_clock_cfg_s {
		u64 reserved_40_63 : 24;
		u64 ext_clk_edge : 2;
		u64 ckout_out4 : 1;
		u64 pps_out : 5;
		u64 pps_inv : 1;
		u64 pps_en : 1;
		u64 ckout_out : 4;
		u64 ckout_inv : 1;
		u64 ckout_en : 1;
		u64 evcnt_in : 6;
		u64 evcnt_edge : 1;
		u64 evcnt_en : 1;
		u64 tstmp_in : 6;
		u64 tstmp_edge : 1;
		u64 tstmp_en : 1;
		u64 ext_clk_in : 6;
		u64 ext_clk_en : 1;
		u64 ptp_en : 1;
	} s;
	struct cvmx_mio_ptp_clock_cfg_cn61xx {
		u64 reserved_42_63 : 22;
		u64 pps : 1;
		u64 ckout : 1;
		u64 ext_clk_edge : 2;
		u64 ckout_out4 : 1;
		u64 pps_out : 5;
		u64 pps_inv : 1;
		u64 pps_en : 1;
		u64 ckout_out : 4;
		u64 ckout_inv : 1;
		u64 ckout_en : 1;
		u64 evcnt_in : 6;
		u64 evcnt_edge : 1;
		u64 evcnt_en : 1;
		u64 tstmp_in : 6;
		u64 tstmp_edge : 1;
		u64 tstmp_en : 1;
		u64 ext_clk_in : 6;
		u64 ext_clk_en : 1;
		u64 ptp_en : 1;
	} cn61xx;
	struct cvmx_mio_ptp_clock_cfg_cn63xx {
		u64 reserved_24_63 : 40;
		u64 evcnt_in : 6;
		u64 evcnt_edge : 1;
		u64 evcnt_en : 1;
		u64 tstmp_in : 6;
		u64 tstmp_edge : 1;
		u64 tstmp_en : 1;
		u64 ext_clk_in : 6;
		u64 ext_clk_en : 1;
		u64 ptp_en : 1;
	} cn63xx;
	struct cvmx_mio_ptp_clock_cfg_cn63xx cn63xxp1;
	struct cvmx_mio_ptp_clock_cfg_s cn66xx;
	struct cvmx_mio_ptp_clock_cfg_cn61xx cn68xx;
	struct cvmx_mio_ptp_clock_cfg_cn63xx cn68xxp1;
	struct cvmx_mio_ptp_clock_cfg_cn70xx {
		u64 reserved_42_63 : 22;
		u64 ckout : 1;
		u64 pps : 1;
		u64 ext_clk_edge : 2;
		u64 reserved_32_37 : 6;
		u64 pps_inv : 1;
		u64 pps_en : 1;
		u64 reserved_26_29 : 4;
		u64 ckout_inv : 1;
		u64 ckout_en : 1;
		u64 evcnt_in : 6;
		u64 evcnt_edge : 1;
		u64 evcnt_en : 1;
		u64 tstmp_in : 6;
		u64 tstmp_edge : 1;
		u64 tstmp_en : 1;
		u64 ext_clk_in : 6;
		u64 ext_clk_en : 1;
		u64 ptp_en : 1;
	} cn70xx;
	struct cvmx_mio_ptp_clock_cfg_cn70xx cn70xxp1;
	struct cvmx_mio_ptp_clock_cfg_cn70xx cn73xx;
	struct cvmx_mio_ptp_clock_cfg_cn70xx cn78xx;
	struct cvmx_mio_ptp_clock_cfg_cn70xx cn78xxp1;
	struct cvmx_mio_ptp_clock_cfg_cn61xx cnf71xx;
	struct cvmx_mio_ptp_clock_cfg_cn70xx cnf75xx;
};

typedef union cvmx_mio_ptp_clock_cfg cvmx_mio_ptp_clock_cfg_t;

/**
 * cvmx_mio_ptp_evt_cnt
 *
 * This register contains the PTP event counter.
 *
 */
union cvmx_mio_ptp_evt_cnt {
	u64 u64;
	struct cvmx_mio_ptp_evt_cnt_s {
		u64 cntr : 64;
	} s;
	struct cvmx_mio_ptp_evt_cnt_s cn61xx;
	struct cvmx_mio_ptp_evt_cnt_s cn63xx;
	struct cvmx_mio_ptp_evt_cnt_s cn63xxp1;
	struct cvmx_mio_ptp_evt_cnt_s cn66xx;
	struct cvmx_mio_ptp_evt_cnt_s cn68xx;
	struct cvmx_mio_ptp_evt_cnt_s cn68xxp1;
	struct cvmx_mio_ptp_evt_cnt_s cn70xx;
	struct cvmx_mio_ptp_evt_cnt_s cn70xxp1;
	struct cvmx_mio_ptp_evt_cnt_s cn73xx;
	struct cvmx_mio_ptp_evt_cnt_s cn78xx;
	struct cvmx_mio_ptp_evt_cnt_s cn78xxp1;
	struct cvmx_mio_ptp_evt_cnt_s cnf71xx;
	struct cvmx_mio_ptp_evt_cnt_s cnf75xx;
};

typedef union cvmx_mio_ptp_evt_cnt cvmx_mio_ptp_evt_cnt_t;

/**
 * cvmx_mio_rst_boot
 *
 * Notes:
 * JTCSRDIS, EJTAGDIS, ROMEN reset to 1 in authentik mode; in all other modes they reset to 0.
 *
 */
union cvmx_mio_rst_boot {
	u64 u64;
	struct cvmx_mio_rst_boot_s {
		u64 chipkill : 1;
		u64 jtcsrdis : 1;
		u64 ejtagdis : 1;
		u64 romen : 1;
		u64 ckill_ppdis : 1;
		u64 jt_tstmode : 1;
		u64 reserved_50_57 : 8;
		u64 lboot_ext : 2;
		u64 reserved_44_47 : 4;
		u64 qlm4_spd : 4;
		u64 qlm3_spd : 4;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} s;
	struct cvmx_mio_rst_boot_cn61xx {
		u64 chipkill : 1;
		u64 jtcsrdis : 1;
		u64 ejtagdis : 1;
		u64 romen : 1;
		u64 ckill_ppdis : 1;
		u64 jt_tstmode : 1;
		u64 reserved_50_57 : 8;
		u64 lboot_ext : 2;
		u64 reserved_36_47 : 12;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} cn61xx;
	struct cvmx_mio_rst_boot_cn63xx {
		u64 reserved_36_63 : 28;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} cn63xx;
	struct cvmx_mio_rst_boot_cn63xx cn63xxp1;
	struct cvmx_mio_rst_boot_cn66xx {
		u64 chipkill : 1;
		u64 jtcsrdis : 1;
		u64 ejtagdis : 1;
		u64 romen : 1;
		u64 ckill_ppdis : 1;
		u64 reserved_50_58 : 9;
		u64 lboot_ext : 2;
		u64 reserved_36_47 : 12;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} cn66xx;
	struct cvmx_mio_rst_boot_cn68xx {
		u64 reserved_59_63 : 5;
		u64 jt_tstmode : 1;
		u64 reserved_44_57 : 14;
		u64 qlm4_spd : 4;
		u64 qlm3_spd : 4;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} cn68xx;
	struct cvmx_mio_rst_boot_cn68xxp1 {
		u64 reserved_44_63 : 20;
		u64 qlm4_spd : 4;
		u64 qlm3_spd : 4;
		u64 c_mul : 6;
		u64 pnr_mul : 6;
		u64 qlm2_spd : 4;
		u64 qlm1_spd : 4;
		u64 qlm0_spd : 4;
		u64 lboot : 10;
		u64 rboot : 1;
		u64 rboot_pin : 1;
	} cn68xxp1;
	struct cvmx_mio_rst_boot_cn61xx cnf71xx;
};

typedef union cvmx_mio_rst_boot cvmx_mio_rst_boot_t;

/**
 * cvmx_mio_rst_ctl#
 *
 * Notes:
 * GEN1_Only mode is enabled for PEM0 when QLM1_SPD[0] is set or when sclk < 550Mhz.
 * GEN1_Only mode is enabled for PEM1 when QLM1_SPD[1] is set or when sclk < 550Mhz.
 */
union cvmx_mio_rst_ctlx {
	u64 u64;
	struct cvmx_mio_rst_ctlx_s {
		u64 reserved_13_63 : 51;
		u64 in_rev_ln : 1;
		u64 rev_lanes : 1;
		u64 gen1_only : 1;
		u64 prst_link : 1;
		u64 rst_done : 1;
		u64 rst_link : 1;
		u64 host_mode : 1;
		u64 prtmode : 2;
		u64 rst_drv : 1;
		u64 rst_rcv : 1;
		u64 rst_chip : 1;
		u64 rst_val : 1;
	} s;
	struct cvmx_mio_rst_ctlx_s cn61xx;
	struct cvmx_mio_rst_ctlx_cn63xx {
		u64 reserved_10_63 : 54;
		u64 prst_link : 1;
		u64 rst_done : 1;
		u64 rst_link : 1;
		u64 host_mode : 1;
		u64 prtmode : 2;
		u64 rst_drv : 1;
		u64 rst_rcv : 1;
		u64 rst_chip : 1;
		u64 rst_val : 1;
	} cn63xx;
	struct cvmx_mio_rst_ctlx_cn63xxp1 {
		u64 reserved_9_63 : 55;
		u64 rst_done : 1;
		u64 rst_link : 1;
		u64 host_mode : 1;
		u64 prtmode : 2;
		u64 rst_drv : 1;
		u64 rst_rcv : 1;
		u64 rst_chip : 1;
		u64 rst_val : 1;
	} cn63xxp1;
	struct cvmx_mio_rst_ctlx_cn63xx cn66xx;
	struct cvmx_mio_rst_ctlx_cn63xx cn68xx;
	struct cvmx_mio_rst_ctlx_cn63xx cn68xxp1;
	struct cvmx_mio_rst_ctlx_s cnf71xx;
};

typedef union cvmx_mio_rst_ctlx cvmx_mio_rst_ctlx_t;

/**
 * cvmx_mio_qlm#_cfg
 *
 * Notes:
 * Certain QLM_SPD is valid only for certain QLM_CFG configuration, refer to HRM for valid
 * combinations.  These csrs are reset only on COLD_RESET.  The Reset values for QLM_SPD and QLM_CFG
 * are as follows:               MIO_QLM0_CFG  SPD=F, CFG=2 SGMII (AGX0)
 *                               MIO_QLM1_CFG  SPD=0, CFG=1 PCIE 2x1 (PEM0/PEM1)
 */
union cvmx_mio_qlmx_cfg {
	u64 u64;
	struct cvmx_mio_qlmx_cfg_s {
		u64 reserved_15_63 : 49;
		u64 prtmode : 1;
		u64 reserved_12_13 : 2;
		u64 qlm_spd : 4;
		u64 reserved_4_7 : 4;
		u64 qlm_cfg : 4;
	} s;
	struct cvmx_mio_qlmx_cfg_cn61xx {
		u64 reserved_15_63 : 49;
		u64 prtmode : 1;
		u64 reserved_12_13 : 2;
		u64 qlm_spd : 4;
		u64 reserved_2_7 : 6;
		u64 qlm_cfg : 2;
	} cn61xx;
	struct cvmx_mio_qlmx_cfg_cn66xx {
		u64 reserved_12_63 : 52;
		u64 qlm_spd : 4;
		u64 reserved_4_7 : 4;
		u64 qlm_cfg : 4;
	} cn66xx;
	struct cvmx_mio_qlmx_cfg_cn68xx {
		u64 reserved_12_63 : 52;
		u64 qlm_spd : 4;
		u64 reserved_3_7 : 5;
		u64 qlm_cfg : 3;
	} cn68xx;
	struct cvmx_mio_qlmx_cfg_cn68xx cn68xxp1;
	struct cvmx_mio_qlmx_cfg_cn61xx cnf71xx;
};

typedef union cvmx_mio_qlmx_cfg cvmx_mio_qlmx_cfg_t;

#endif
