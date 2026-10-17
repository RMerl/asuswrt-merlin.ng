/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon agl.
 *
 */

#ifndef __CVMX_AGL_DEFS_H__
#define __CVMX_AGL_DEFS_H__

#define CVMX_AGL_GMX_BAD_REG			    (0x00011800E0000518ull)
#define CVMX_AGL_GMX_BIST			    (0x00011800E0000400ull)
#define CVMX_AGL_GMX_DRV_CTL			    (0x00011800E00007F0ull)
#define CVMX_AGL_GMX_INF_MODE			    (0x00011800E00007F8ull)
#define CVMX_AGL_GMX_PRTX_CFG(offset)		    (0x00011800E0000010ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM0(offset)	    (0x00011800E0000180ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM1(offset)	    (0x00011800E0000188ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM2(offset)	    (0x00011800E0000190ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM3(offset)	    (0x00011800E0000198ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM4(offset)	    (0x00011800E00001A0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM5(offset)	    (0x00011800E00001A8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CAM_EN(offset)	    (0x00011800E0000108ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_ADR_CTL(offset)	    (0x00011800E0000100ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_DECISION(offset)	    (0x00011800E0000040ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_FRM_CHK(offset)	    (0x00011800E0000020ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_FRM_CTL(offset)	    (0x00011800E0000018ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_FRM_MAX(offset)	    (0x00011800E0000030ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_FRM_MIN(offset)	    (0x00011800E0000028ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_IFG(offset)		    (0x00011800E0000058ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_INT_EN(offset)		    (0x00011800E0000008ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_INT_REG(offset)	    (0x00011800E0000000ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_JABBER(offset)		    (0x00011800E0000038ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_PAUSE_DROP_TIME(offset)    (0x00011800E0000068ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_RX_INBND(offset)	    (0x00011800E0000060ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_CTL(offset)	    (0x00011800E0000050ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_OCTS(offset)	    (0x00011800E0000088ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_OCTS_CTL(offset)	    (0x00011800E0000098ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_OCTS_DMAC(offset)    (0x00011800E00000A8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_OCTS_DRP(offset)	    (0x00011800E00000B8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_PKTS(offset)	    (0x00011800E0000080ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_PKTS_BAD(offset)	    (0x00011800E00000C0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_PKTS_CTL(offset)	    (0x00011800E0000090ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_PKTS_DMAC(offset)    (0x00011800E00000A0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_STATS_PKTS_DRP(offset)	    (0x00011800E00000B0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RXX_UDD_SKP(offset)	    (0x00011800E0000048ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_RX_BP_DROPX(offset)	    (0x00011800E0000420ull + ((offset) & 1) * 8)
#define CVMX_AGL_GMX_RX_BP_OFFX(offset)		    (0x00011800E0000460ull + ((offset) & 1) * 8)
#define CVMX_AGL_GMX_RX_BP_ONX(offset)		    (0x00011800E0000440ull + ((offset) & 1) * 8)
#define CVMX_AGL_GMX_RX_PRT_INFO		    (0x00011800E00004E8ull)
#define CVMX_AGL_GMX_RX_TX_STATUS		    (0x00011800E00007E8ull)
#define CVMX_AGL_GMX_SMACX(offset)		    (0x00011800E0000230ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_STAT_BP			    (0x00011800E0000520ull)
#define CVMX_AGL_GMX_TXX_APPEND(offset)		    (0x00011800E0000218ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_CLK(offset)		    (0x00011800E0000208ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_CTL(offset)		    (0x00011800E0000270ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_MIN_PKT(offset)	    (0x00011800E0000240ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_PAUSE_PKT_INTERVAL(offset) (0x00011800E0000248ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_PAUSE_PKT_TIME(offset)	    (0x00011800E0000238ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_PAUSE_TOGO(offset)	    (0x00011800E0000258ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_PAUSE_ZERO(offset)	    (0x00011800E0000260ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_SOFT_PAUSE(offset)	    (0x00011800E0000250ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT0(offset)		    (0x00011800E0000280ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT1(offset)		    (0x00011800E0000288ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT2(offset)		    (0x00011800E0000290ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT3(offset)		    (0x00011800E0000298ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT4(offset)		    (0x00011800E00002A0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT5(offset)		    (0x00011800E00002A8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT6(offset)		    (0x00011800E00002B0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT7(offset)		    (0x00011800E00002B8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT8(offset)		    (0x00011800E00002C0ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STAT9(offset)		    (0x00011800E00002C8ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_STATS_CTL(offset)	    (0x00011800E0000268ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TXX_THRESH(offset)		    (0x00011800E0000210ull + ((offset) & 1) * 2048)
#define CVMX_AGL_GMX_TX_BP			    (0x00011800E00004D0ull)
#define CVMX_AGL_GMX_TX_COL_ATTEMPT		    (0x00011800E0000498ull)
#define CVMX_AGL_GMX_TX_IFG			    (0x00011800E0000488ull)
#define CVMX_AGL_GMX_TX_INT_EN			    (0x00011800E0000508ull)
#define CVMX_AGL_GMX_TX_INT_REG			    (0x00011800E0000500ull)
#define CVMX_AGL_GMX_TX_JAM			    (0x00011800E0000490ull)
#define CVMX_AGL_GMX_TX_LFSR			    (0x00011800E00004F8ull)
#define CVMX_AGL_GMX_TX_OVR_BP			    (0x00011800E00004C8ull)
#define CVMX_AGL_GMX_TX_PAUSE_PKT_DMAC		    (0x00011800E00004A0ull)
#define CVMX_AGL_GMX_TX_PAUSE_PKT_TYPE		    (0x00011800E00004A8ull)
#define CVMX_AGL_GMX_WOL_CTL			    (0x00011800E0000780ull)
#define CVMX_AGL_PRTX_CTL(offset)		    (0x00011800E0002000ull + ((offset) & 1) * 8)

/**
 * cvmx_agl_gmx_bad_reg
 *
 * AGL_GMX_BAD_REG = A collection of things that have gone very, very wrong
 *
 *
 * Notes:
 * OUT_OVR[0], LOSTSTAT[0], OVRFLW, TXPOP, TXPSH    will be reset when MIX0_CTL[RESET] is set to 1.
 * OUT_OVR[1], LOSTSTAT[1], OVRFLW1, TXPOP1, TXPSH1 will be reset when MIX1_CTL[RESET] is set to 1.
 * STATOVR will be reset when both MIX0/1_CTL[RESET] are set to 1.
 */
union cvmx_agl_gmx_bad_reg {
	u64 u64;
	struct cvmx_agl_gmx_bad_reg_s {
		u64 reserved_38_63 : 26;
		u64 txpsh1 : 1;
		u64 txpop1 : 1;
		u64 ovrflw1 : 1;
		u64 txpsh : 1;
		u64 txpop : 1;
		u64 ovrflw : 1;
		u64 reserved_27_31 : 5;
		u64 statovr : 1;
		u64 reserved_24_25 : 2;
		u64 loststat : 2;
		u64 reserved_4_21 : 18;
		u64 out_ovr : 2;
		u64 reserved_0_1 : 2;
	} s;
	struct cvmx_agl_gmx_bad_reg_cn52xx {
		u64 reserved_38_63 : 26;
		u64 txpsh1 : 1;
		u64 txpop1 : 1;
		u64 ovrflw1 : 1;
		u64 txpsh : 1;
		u64 txpop : 1;
		u64 ovrflw : 1;
		u64 reserved_27_31 : 5;
		u64 statovr : 1;
		u64 reserved_23_25 : 3;
		u64 loststat : 1;
		u64 reserved_4_21 : 18;
		u64 out_ovr : 2;
		u64 reserved_0_1 : 2;
	} cn52xx;
	struct cvmx_agl_gmx_bad_reg_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_bad_reg_cn56xx {
		u64 reserved_35_63 : 29;
		u64 txpsh : 1;
		u64 txpop : 1;
		u64 ovrflw : 1;
		u64 reserved_27_31 : 5;
		u64 statovr : 1;
		u64 reserved_23_25 : 3;
		u64 loststat : 1;
		u64 reserved_3_21 : 19;
		u64 out_ovr : 1;
		u64 reserved_0_1 : 2;
	} cn56xx;
	struct cvmx_agl_gmx_bad_reg_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_bad_reg_s cn61xx;
	struct cvmx_agl_gmx_bad_reg_s cn63xx;
	struct cvmx_agl_gmx_bad_reg_s cn63xxp1;
	struct cvmx_agl_gmx_bad_reg_s cn66xx;
	struct cvmx_agl_gmx_bad_reg_s cn68xx;
	struct cvmx_agl_gmx_bad_reg_s cn68xxp1;
	struct cvmx_agl_gmx_bad_reg_cn56xx cn70xx;
	struct cvmx_agl_gmx_bad_reg_cn56xx cn70xxp1;
};

typedef union cvmx_agl_gmx_bad_reg cvmx_agl_gmx_bad_reg_t;

/**
 * cvmx_agl_gmx_bist
 *
 * AGL_GMX_BIST = GMX BIST Results
 *
 *
 * Notes:
 * Not reset when MIX*_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_bist {
	u64 u64;
	struct cvmx_agl_gmx_bist_s {
		u64 reserved_25_63 : 39;
		u64 status : 25;
	} s;
	struct cvmx_agl_gmx_bist_cn52xx {
		u64 reserved_10_63 : 54;
		u64 status : 10;
	} cn52xx;
	struct cvmx_agl_gmx_bist_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_bist_cn52xx cn56xx;
	struct cvmx_agl_gmx_bist_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_bist_s cn61xx;
	struct cvmx_agl_gmx_bist_s cn63xx;
	struct cvmx_agl_gmx_bist_s cn63xxp1;
	struct cvmx_agl_gmx_bist_s cn66xx;
	struct cvmx_agl_gmx_bist_s cn68xx;
	struct cvmx_agl_gmx_bist_s cn68xxp1;
	struct cvmx_agl_gmx_bist_s cn70xx;
	struct cvmx_agl_gmx_bist_s cn70xxp1;
};

typedef union cvmx_agl_gmx_bist cvmx_agl_gmx_bist_t;

/**
 * cvmx_agl_gmx_drv_ctl
 *
 * AGL_GMX_DRV_CTL = GMX Drive Control
 *
 *
 * Notes:
 * NCTL, PCTL, BYP_EN    will be reset when MIX0_CTL[RESET] is set to 1.
 * NCTL1, PCTL1, BYP_EN1 will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_drv_ctl {
	u64 u64;
	struct cvmx_agl_gmx_drv_ctl_s {
		u64 reserved_49_63 : 15;
		u64 byp_en1 : 1;
		u64 reserved_45_47 : 3;
		u64 pctl1 : 5;
		u64 reserved_37_39 : 3;
		u64 nctl1 : 5;
		u64 reserved_17_31 : 15;
		u64 byp_en : 1;
		u64 reserved_13_15 : 3;
		u64 pctl : 5;
		u64 reserved_5_7 : 3;
		u64 nctl : 5;
	} s;
	struct cvmx_agl_gmx_drv_ctl_s cn52xx;
	struct cvmx_agl_gmx_drv_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_drv_ctl_cn56xx {
		u64 reserved_17_63 : 47;
		u64 byp_en : 1;
		u64 reserved_13_15 : 3;
		u64 pctl : 5;
		u64 reserved_5_7 : 3;
		u64 nctl : 5;
	} cn56xx;
	struct cvmx_agl_gmx_drv_ctl_cn56xx cn56xxp1;
};

typedef union cvmx_agl_gmx_drv_ctl cvmx_agl_gmx_drv_ctl_t;

/**
 * cvmx_agl_gmx_inf_mode
 *
 * AGL_GMX_INF_MODE = Interface Mode
 *
 *
 * Notes:
 * Not reset when MIX*_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_inf_mode {
	u64 u64;
	struct cvmx_agl_gmx_inf_mode_s {
		u64 reserved_2_63 : 62;
		u64 en : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_agl_gmx_inf_mode_s cn52xx;
	struct cvmx_agl_gmx_inf_mode_s cn52xxp1;
	struct cvmx_agl_gmx_inf_mode_s cn56xx;
	struct cvmx_agl_gmx_inf_mode_s cn56xxp1;
};

typedef union cvmx_agl_gmx_inf_mode cvmx_agl_gmx_inf_mode_t;

/**
 * cvmx_agl_gmx_prt#_cfg
 *
 * AGL_GMX_PRT_CFG = Port description
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_prtx_cfg {
	u64 u64;
	struct cvmx_agl_gmx_prtx_cfg_s {
		u64 reserved_14_63 : 50;
		u64 tx_idle : 1;
		u64 rx_idle : 1;
		u64 reserved_9_11 : 3;
		u64 speed_msb : 1;
		u64 reserved_7_7 : 1;
		u64 burst : 1;
		u64 tx_en : 1;
		u64 rx_en : 1;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 en : 1;
	} s;
	struct cvmx_agl_gmx_prtx_cfg_cn52xx {
		u64 reserved_6_63 : 58;
		u64 tx_en : 1;
		u64 rx_en : 1;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 en : 1;
	} cn52xx;
	struct cvmx_agl_gmx_prtx_cfg_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_prtx_cfg_cn52xx cn56xx;
	struct cvmx_agl_gmx_prtx_cfg_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_prtx_cfg_s cn61xx;
	struct cvmx_agl_gmx_prtx_cfg_s cn63xx;
	struct cvmx_agl_gmx_prtx_cfg_s cn63xxp1;
	struct cvmx_agl_gmx_prtx_cfg_s cn66xx;
	struct cvmx_agl_gmx_prtx_cfg_s cn68xx;
	struct cvmx_agl_gmx_prtx_cfg_s cn68xxp1;
	struct cvmx_agl_gmx_prtx_cfg_s cn70xx;
	struct cvmx_agl_gmx_prtx_cfg_s cn70xxp1;
};

typedef union cvmx_agl_gmx_prtx_cfg cvmx_agl_gmx_prtx_cfg_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam0
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam0 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam0_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam0_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam0 cvmx_agl_gmx_rxx_adr_cam0_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam1
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam1 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam1_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam1_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam1 cvmx_agl_gmx_rxx_adr_cam1_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam2
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam2 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam2_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam2_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam2 cvmx_agl_gmx_rxx_adr_cam2_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam3
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam3 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam3_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam3_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam3 cvmx_agl_gmx_rxx_adr_cam3_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam4
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam4 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam4_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam4_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam4 cvmx_agl_gmx_rxx_adr_cam4_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam5
 *
 * AGL_GMX_RX_ADR_CAM = Address Filtering Control
 *
 */
union cvmx_agl_gmx_rxx_adr_cam5 {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam5_s {
		u64 adr : 64;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam5_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam5 cvmx_agl_gmx_rxx_adr_cam5_t;

/**
 * cvmx_agl_gmx_rx#_adr_cam_en
 *
 * AGL_GMX_RX_ADR_CAM_EN = Address Filtering Control Enable
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_adr_cam_en {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s {
		u64 reserved_8_63 : 56;
		u64 en : 8;
	} s;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_cam_en_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_cam_en cvmx_agl_gmx_rxx_adr_cam_en_t;

/**
 * cvmx_agl_gmx_rx#_adr_ctl
 *
 * AGL_GMX_RX_ADR_CTL = Address Filtering Control
 *
 *
 * Notes:
 * * ALGORITHM
 *   Here is some pseudo code that represents the address filter behavior.
 *
 *      @verbatim
 *      bool dmac_addr_filter(uint8 prt, uint48 dmac) [
 *        ASSERT(prt >= 0 && prt <= 3);
 *        if (is_bcst(dmac))                               // broadcast accept
 *          return (AGL_GMX_RX[prt]_ADR_CTL[BCST] ? ACCEPT : REJECT);
 *        if (is_mcst(dmac) & AGL_GMX_RX[prt]_ADR_CTL[MCST] == 1)   // multicast reject
 *          return REJECT;
 *        if (is_mcst(dmac) & AGL_GMX_RX[prt]_ADR_CTL[MCST] == 2)   // multicast accept
 *          return ACCEPT;
 *
 *        cam_hit = 0;
 *
 *        for (i=0; i<8; i++) [
 *          if (AGL_GMX_RX[prt]_ADR_CAM_EN[EN<i>] == 0)
 *            continue;
 *          uint48 unswizzled_mac_adr = 0x0;
 *          for (j=5; j>=0; j--) [
 *             unswizzled_mac_adr = (unswizzled_mac_adr << 8) | AGL_GMX_RX[prt]_ADR_CAM[j][ADR<i*8+7:i*8>];
 *          ]
 *          if (unswizzled_mac_adr == dmac) [
 *            cam_hit = 1;
 *            break;
 *          ]
 *        ]
 *
 *        if (cam_hit)
 *          return (AGL_GMX_RX[prt]_ADR_CTL[CAM_MODE] ? ACCEPT : REJECT);
 *        else
 *          return (AGL_GMX_RX[prt]_ADR_CTL[CAM_MODE] ? REJECT : ACCEPT);
 *      ]
 *      @endverbatim
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_adr_ctl {
	u64 u64;
	struct cvmx_agl_gmx_rxx_adr_ctl_s {
		u64 reserved_4_63 : 60;
		u64 cam_mode : 1;
		u64 mcst : 2;
		u64 bcst : 1;
	} s;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn52xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn56xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn61xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn63xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn66xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn68xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn70xx;
	struct cvmx_agl_gmx_rxx_adr_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_adr_ctl cvmx_agl_gmx_rxx_adr_ctl_t;

/**
 * cvmx_agl_gmx_rx#_decision
 *
 * AGL_GMX_RX_DECISION = The byte count to decide when to accept or filter a packet
 *
 *
 * Notes:
 * As each byte in a packet is received by GMX, the L2 byte count is compared
 * against the AGL_GMX_RX_DECISION[CNT].  The L2 byte count is the number of bytes
 * from the beginning of the L2 header (DMAC).  In normal operation, the L2
 * header begins after the PREAMBLE+SFD (AGL_GMX_RX_FRM_CTL[PRE_CHK]=1) and any
 * optional UDD skip data (AGL_GMX_RX_UDD_SKP[LEN]).
 *
 * When AGL_GMX_RX_FRM_CTL[PRE_CHK] is clear, PREAMBLE+SFD are prepended to the
 * packet and would require UDD skip length to account for them.
 *
 *                                                 L2 Size
 * Port Mode             <=AGL_GMX_RX_DECISION bytes (default=24)  >AGL_GMX_RX_DECISION bytes (default=24)
 *
 * MII/Full Duplex       accept packet                             apply filters
 *                       no filtering is applied                   accept packet based on DMAC and PAUSE packet filters
 *
 * MII/Half Duplex       drop packet                               apply filters
 *                       packet is unconditionally dropped         accept packet based on DMAC
 *
 * where l2_size = MAX(0, total_packet_size - AGL_GMX_RX_UDD_SKP[LEN] - ((AGL_GMX_RX_FRM_CTL[PRE_CHK]==1)*8)
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_decision {
	u64 u64;
	struct cvmx_agl_gmx_rxx_decision_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_agl_gmx_rxx_decision_s cn52xx;
	struct cvmx_agl_gmx_rxx_decision_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_decision_s cn56xx;
	struct cvmx_agl_gmx_rxx_decision_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_decision_s cn61xx;
	struct cvmx_agl_gmx_rxx_decision_s cn63xx;
	struct cvmx_agl_gmx_rxx_decision_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_decision_s cn66xx;
	struct cvmx_agl_gmx_rxx_decision_s cn68xx;
	struct cvmx_agl_gmx_rxx_decision_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_decision_s cn70xx;
	struct cvmx_agl_gmx_rxx_decision_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_decision cvmx_agl_gmx_rxx_decision_t;

/**
 * cvmx_agl_gmx_rx#_frm_chk
 *
 * AGL_GMX_RX_FRM_CHK = Which frame errors will set the ERR bit of the frame
 *
 *
 * Notes:
 * If AGL_GMX_RX_UDD_SKP[LEN] != 0, then LENERR will be forced to zero in HW.
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_frm_chk {
	u64 u64;
	struct cvmx_agl_gmx_rxx_frm_chk_s {
		u64 reserved_10_63 : 54;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_agl_gmx_rxx_frm_chk_cn52xx {
		u64 reserved_9_63 : 55;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 reserved_1_1 : 1;
		u64 minerr : 1;
	} cn52xx;
	struct cvmx_agl_gmx_rxx_frm_chk_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_rxx_frm_chk_cn52xx cn56xx;
	struct cvmx_agl_gmx_rxx_frm_chk_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn61xx;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn63xx;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn66xx;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn68xx;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn70xx;
	struct cvmx_agl_gmx_rxx_frm_chk_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_frm_chk cvmx_agl_gmx_rxx_frm_chk_t;

/**
 * cvmx_agl_gmx_rx#_frm_ctl
 *
 * AGL_GMX_RX_FRM_CTL = Frame Control
 *
 *
 * Notes:
 * * PRE_STRP
 *   When PRE_CHK is set (indicating that the PREAMBLE will be sent), PRE_STRP
 *   determines if the PREAMBLE+SFD bytes are thrown away or sent to the Octane
 *   core as part of the packet.
 *
 *   In either mode, the PREAMBLE+SFD bytes are not counted toward the packet
 *   size when checking against the MIN and MAX bounds.  Furthermore, the bytes
 *   are skipped when locating the start of the L2 header for DMAC and Control
 *   frame recognition.
 *
 * * CTL_BCK/CTL_DRP
 *   These bits control how the HW handles incoming PAUSE packets.  Here are
 *   the most common modes of operation:
 *     CTL_BCK=1,CTL_DRP=1   - HW does it all
 *     CTL_BCK=0,CTL_DRP=0   - SW sees all pause frames
 *     CTL_BCK=0,CTL_DRP=1   - all pause frames are completely ignored
 *
 *   These control bits should be set to CTL_BCK=0,CTL_DRP=0 in halfdup mode.
 *   Since PAUSE packets only apply to fulldup operation, any PAUSE packet
 *   would constitute an exception which should be handled by the processing
 *   cores.  PAUSE packets should not be forwarded.
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_frm_ctl {
	u64 u64;
	struct cvmx_agl_gmx_rxx_frm_ctl_s {
		u64 reserved_13_63 : 51;
		u64 ptp_mode : 1;
		u64 reserved_11_11 : 1;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} s;
	struct cvmx_agl_gmx_rxx_frm_ctl_cn52xx {
		u64 reserved_10_63 : 54;
		u64 pre_align : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} cn52xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_rxx_frm_ctl_cn52xx cn56xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn61xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn63xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn66xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn68xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn70xx;
	struct cvmx_agl_gmx_rxx_frm_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_frm_ctl cvmx_agl_gmx_rxx_frm_ctl_t;

/**
 * cvmx_agl_gmx_rx#_frm_max
 *
 * AGL_GMX_RX_FRM_MAX = Frame Max length
 *
 *
 * Notes:
 * When changing the LEN field, be sure that LEN does not exceed
 * AGL_GMX_RX_JABBER[CNT]. Failure to meet this constraint will cause packets that
 * are within the maximum length parameter to be rejected because they exceed
 * the AGL_GMX_RX_JABBER[CNT] limit.
 *
 * Notes:
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_frm_max {
	u64 u64;
	struct cvmx_agl_gmx_rxx_frm_max_s {
		u64 reserved_16_63 : 48;
		u64 len : 16;
	} s;
	struct cvmx_agl_gmx_rxx_frm_max_s cn52xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_frm_max_s cn56xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_frm_max_s cn61xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn63xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_frm_max_s cn66xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn68xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_frm_max_s cn70xx;
	struct cvmx_agl_gmx_rxx_frm_max_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_frm_max cvmx_agl_gmx_rxx_frm_max_t;

/**
 * cvmx_agl_gmx_rx#_frm_min
 *
 * AGL_GMX_RX_FRM_MIN = Frame Min length
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_frm_min {
	u64 u64;
	struct cvmx_agl_gmx_rxx_frm_min_s {
		u64 reserved_16_63 : 48;
		u64 len : 16;
	} s;
	struct cvmx_agl_gmx_rxx_frm_min_s cn52xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_frm_min_s cn56xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_frm_min_s cn61xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn63xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_frm_min_s cn66xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn68xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_frm_min_s cn70xx;
	struct cvmx_agl_gmx_rxx_frm_min_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_frm_min cvmx_agl_gmx_rxx_frm_min_t;

/**
 * cvmx_agl_gmx_rx#_ifg
 *
 * AGL_GMX_RX_IFG = RX Min IFG
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_ifg {
	u64 u64;
	struct cvmx_agl_gmx_rxx_ifg_s {
		u64 reserved_4_63 : 60;
		u64 ifg : 4;
	} s;
	struct cvmx_agl_gmx_rxx_ifg_s cn52xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_ifg_s cn56xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_ifg_s cn61xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn63xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_ifg_s cn66xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn68xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_ifg_s cn70xx;
	struct cvmx_agl_gmx_rxx_ifg_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_ifg cvmx_agl_gmx_rxx_ifg_t;

/**
 * cvmx_agl_gmx_rx#_int_en
 *
 * AGL_GMX_RX_INT_EN = Interrupt Enable
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_int_en {
	u64 u64;
	struct cvmx_agl_gmx_rxx_int_en_s {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 reserved_20_28 : 9;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_agl_gmx_rxx_int_en_cn52xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 reserved_1_1 : 1;
		u64 minerr : 1;
	} cn52xx;
	struct cvmx_agl_gmx_rxx_int_en_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_rxx_int_en_cn52xx cn56xx;
	struct cvmx_agl_gmx_rxx_int_en_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn61xx;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx cn63xx;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx cn63xxp1;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx cn66xx;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx cn68xx;
	struct cvmx_agl_gmx_rxx_int_en_cn61xx cn68xxp1;
	struct cvmx_agl_gmx_rxx_int_en_s cn70xx;
	struct cvmx_agl_gmx_rxx_int_en_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_int_en cvmx_agl_gmx_rxx_int_en_t;

/**
 * cvmx_agl_gmx_rx#_int_reg
 *
 * AGL_GMX_RX_INT_REG = Interrupt Register
 *
 *
 * Notes:
 * (1) exceptions will only be raised to the control processor if the
 *     corresponding bit in the AGL_GMX_RX_INT_EN register is set.
 *
 * (2) exception conditions 10:0 can also set the rcv/opcode in the received
 *     packet's workQ entry.  The AGL_GMX_RX_FRM_CHK register provides a bit mask
 *     for configuring which conditions set the error.
 *
 * (3) in half duplex operation, the expectation is that collisions will appear
 *     as MINERRs.
 *
 * (4) JABBER - An RX Jabber error indicates that a packet was received which
 *              is longer than the maximum allowed packet as defined by the
 *              system.  GMX will truncate the packet at the JABBER count.
 *              Failure to do so could lead to system instabilty.
 *
 * (6) MAXERR - for untagged frames, the total frame DA+SA+TL+DATA+PAD+FCS >
 *              AGL_GMX_RX_FRM_MAX.  For tagged frames, DA+SA+VLAN+TL+DATA+PAD+FCS
 *              > AGL_GMX_RX_FRM_MAX + 4*VLAN_VAL + 4*VLAN_STACKED.
 *
 * (7) MINERR - total frame DA+SA+TL+DATA+PAD+FCS < AGL_GMX_RX_FRM_MIN.
 *
 * (8) ALNERR - Indicates that the packet received was not an integer number of
 *              bytes.  If FCS checking is enabled, ALNERR will only assert if
 *              the FCS is bad.  If FCS checking is disabled, ALNERR will
 *              assert in all non-integer frame cases.
 *
 * (9) Collisions - Collisions can only occur in half-duplex mode.  A collision
 *                  is assumed by the receiver when the received
 *                  frame < AGL_GMX_RX_FRM_MIN - this is normally a MINERR
 *
 * (A) LENERR - Length errors occur when the received packet does not match the
 *              length field.  LENERR is only checked for packets between 64
 *              and 1500 bytes.  For untagged frames, the length must exact
 *              match.  For tagged frames the length or length+4 must match.
 *
 * (B) PCTERR - checks that the frame begins with a valid PREAMBLE sequence.
 *              Does not check the number of PREAMBLE cycles.
 *
 * (C) OVRERR -
 *
 *              OVRERR is an architectural assertion check internal to GMX to
 *              make sure no assumption was violated.  In a correctly operating
 *              system, this interrupt can never fire.
 *
 *              GMX has an internal arbiter which selects which of 4 ports to
 *              buffer in the main RX FIFO.  If we normally buffer 8 bytes,
 *              then each port will typically push a tick every 8 cycles - if
 *              the packet interface is going as fast as possible.  If there
 *              are four ports, they push every two cycles.  So that's the
 *              assumption.  That the inbound module will always be able to
 *              consume the tick before another is produced.  If that doesn't
 *              happen - that's when OVRERR will assert.
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_int_reg {
	u64 u64;
	struct cvmx_agl_gmx_rxx_int_reg_s {
		u64 reserved_30_63 : 34;
		u64 wol : 1;
		u64 reserved_20_28 : 9;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_agl_gmx_rxx_int_reg_cn52xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 reserved_16_18 : 3;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 reserved_9_9 : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 reserved_1_1 : 1;
		u64 minerr : 1;
	} cn52xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_rxx_int_reg_cn52xx cn56xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn52xx cn56xxp1;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx {
		u64 reserved_20_63 : 44;
		u64 pause_drp : 1;
		u64 phy_dupx : 1;
		u64 phy_spd : 1;
		u64 phy_link : 1;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 niberr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 lenerr : 1;
		u64 alnerr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 maxerr : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} cn61xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx cn63xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx cn63xxp1;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx cn66xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx cn68xx;
	struct cvmx_agl_gmx_rxx_int_reg_cn61xx cn68xxp1;
	struct cvmx_agl_gmx_rxx_int_reg_s cn70xx;
	struct cvmx_agl_gmx_rxx_int_reg_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_int_reg cvmx_agl_gmx_rxx_int_reg_t;

/**
 * cvmx_agl_gmx_rx#_jabber
 *
 * AGL_GMX_RX_JABBER = The max size packet after which GMX will truncate
 *
 *
 * Notes:
 * CNT must be 8-byte aligned such that CNT[2:0] == 0
 *
 *   The packet that will be sent to the packet input logic will have an
 *   additionl 8 bytes if AGL_GMX_RX_FRM_CTL[PRE_CHK] is set and
 *   AGL_GMX_RX_FRM_CTL[PRE_STRP] is clear.  The max packet that will be sent is
 *   defined as...
 *
 *        max_sized_packet = AGL_GMX_RX_JABBER[CNT]+((AGL_GMX_RX_FRM_CTL[PRE_CHK] & !AGL_GMX_RX_FRM_CTL[PRE_STRP])*8)
 *
 *   Be sure the CNT field value is at least as large as the
 *   AGL_GMX_RX_FRM_MAX[LEN] value. Failure to meet this constraint will cause
 *   packets that are within the AGL_GMX_RX_FRM_MAX[LEN] length to be rejected
 *   because they exceed the CNT limit.
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_jabber {
	u64 u64;
	struct cvmx_agl_gmx_rxx_jabber_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_agl_gmx_rxx_jabber_s cn52xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_jabber_s cn56xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_jabber_s cn61xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn63xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_jabber_s cn66xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn68xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_jabber_s cn70xx;
	struct cvmx_agl_gmx_rxx_jabber_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_jabber cvmx_agl_gmx_rxx_jabber_t;

/**
 * cvmx_agl_gmx_rx#_pause_drop_time
 *
 * AGL_GMX_RX_PAUSE_DROP_TIME = The TIME field in a PAUSE Packet which was dropped due to GMX RX FIFO full condition
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_pause_drop_time {
	u64 u64;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s {
		u64 reserved_16_63 : 48;
		u64 status : 16;
	} s;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn52xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn56xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn61xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn63xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn66xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn68xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn70xx;
	struct cvmx_agl_gmx_rxx_pause_drop_time_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_pause_drop_time cvmx_agl_gmx_rxx_pause_drop_time_t;

/**
 * cvmx_agl_gmx_rx#_rx_inbnd
 *
 * AGL_GMX_RX_INBND = RGMII InBand Link Status
 *
 */
union cvmx_agl_gmx_rxx_rx_inbnd {
	u64 u64;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s {
		u64 reserved_4_63 : 60;
		u64 duplex : 1;
		u64 speed : 2;
		u64 status : 1;
	} s;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn61xx;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn63xx;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn66xx;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn68xx;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn70xx;
	struct cvmx_agl_gmx_rxx_rx_inbnd_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_rx_inbnd cvmx_agl_gmx_rxx_rx_inbnd_t;

/**
 * cvmx_agl_gmx_rx#_stats_ctl
 *
 * AGL_GMX_RX_STATS_CTL = RX Stats Control register
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rxx_stats_ctl {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_ctl_s {
		u64 reserved_1_63 : 63;
		u64 rd_clr : 1;
	} s;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_ctl cvmx_agl_gmx_rxx_stats_ctl_t;

/**
 * cvmx_agl_gmx_rx#_stats_octs
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_stats_octs {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_octs_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_octs_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_octs cvmx_agl_gmx_rxx_stats_octs_t;

/**
 * cvmx_agl_gmx_rx#_stats_octs_ctl
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_stats_octs_ctl {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_octs_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_octs_ctl cvmx_agl_gmx_rxx_stats_octs_ctl_t;

/**
 * cvmx_agl_gmx_rx#_stats_octs_dmac
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_stats_octs_dmac {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_octs_dmac_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_octs_dmac cvmx_agl_gmx_rxx_stats_octs_dmac_t;

/**
 * cvmx_agl_gmx_rx#_stats_octs_drp
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_RX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_stats_octs_drp {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_octs_drp_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_octs_drp cvmx_agl_gmx_rxx_stats_octs_drp_t;

/**
 * cvmx_agl_gmx_rx#_stats_pkts
 *
 * Count of good received packets - packets that are not recognized as PAUSE
 * packets, dropped due the DMAC filter, dropped due FIFO full status, or
 * have any other OPCODE (FCS, Length, etc).
 */
union cvmx_agl_gmx_rxx_stats_pkts {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_pkts_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_pkts cvmx_agl_gmx_rxx_stats_pkts_t;

/**
 * cvmx_agl_gmx_rx#_stats_pkts_bad
 *
 * Count of all packets received with some error that were not dropped
 * either due to the dmac filter or lack of room in the receive FIFO.
 */
union cvmx_agl_gmx_rxx_stats_pkts_bad {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_bad_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_pkts_bad cvmx_agl_gmx_rxx_stats_pkts_bad_t;

/**
 * cvmx_agl_gmx_rx#_stats_pkts_ctl
 *
 * Count of all packets received that were recognized as Flow Control or
 * PAUSE packets.  PAUSE packets with any kind of error are counted in
 * AGL_GMX_RX_STATS_PKTS_BAD.  Pause packets can be optionally dropped or
 * forwarded based on the AGL_GMX_RX_FRM_CTL[CTL_DRP] bit.  This count
 * increments regardless of whether the packet is dropped.  Pause packets
 * will never be counted in AGL_GMX_RX_STATS_PKTS.  Packets dropped due the dmac
 * filter will be counted in AGL_GMX_RX_STATS_PKTS_DMAC and not here.
 */
union cvmx_agl_gmx_rxx_stats_pkts_ctl {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_pkts_ctl cvmx_agl_gmx_rxx_stats_pkts_ctl_t;

/**
 * cvmx_agl_gmx_rx#_stats_pkts_dmac
 *
 * Count of all packets received that were dropped by the dmac filter.
 * Packets that match the DMAC will be dropped and counted here regardless
 * of if they were bad packets.  These packets will never be counted in
 * AGL_GMX_RX_STATS_PKTS.
 * Some packets that were not able to satisify the DECISION_CNT may not
 * actually be dropped by Octeon, but they will be counted here as if they
 * were dropped.
 */
union cvmx_agl_gmx_rxx_stats_pkts_dmac {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_dmac_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_pkts_dmac cvmx_agl_gmx_rxx_stats_pkts_dmac_t;

/**
 * cvmx_agl_gmx_rx#_stats_pkts_drp
 *
 * Count of all packets received that were dropped due to a full receive
 * FIFO.  This counts good and bad packets received - all packets dropped by
 * the FIFO.  It does not count packets dropped by the dmac or pause packet
 * filters.
 */
union cvmx_agl_gmx_rxx_stats_pkts_drp {
	u64 u64;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn52xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn56xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn61xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn63xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn66xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn68xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn70xx;
	struct cvmx_agl_gmx_rxx_stats_pkts_drp_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_stats_pkts_drp cvmx_agl_gmx_rxx_stats_pkts_drp_t;

/**
 * cvmx_agl_gmx_rx#_udd_skp
 *
 * AGL_GMX_RX_UDD_SKP = Amount of User-defined data before the start of the L2 data
 *
 *
 * Notes:
 * (1) The skip bytes are part of the packet and will be sent down the NCB
 *     packet interface and will be handled by PKI.
 *
 * (2) The system can determine if the UDD bytes are included in the FCS check
 *     by using the FCSSEL field - if the FCS check is enabled.
 *
 * (3) Assume that the preamble/sfd is always at the start of the frame - even
 *     before UDD bytes.  In most cases, there will be no preamble in these
 *     cases since it will be MII to MII communication without a PHY
 *     involved.
 *
 * (4) We can still do address filtering and control packet filtering is the
 *     user desires.
 *
 * (5) UDD_SKP must be 0 in half-duplex operation unless
 *     AGL_GMX_RX_FRM_CTL[PRE_CHK] is clear.  If AGL_GMX_RX_FRM_CTL[PRE_CHK] is set,
 *     then UDD_SKP will normally be 8.
 *
 * (6) In all cases, the UDD bytes will be sent down the packet interface as
 *     part of the packet.  The UDD bytes are never stripped from the actual
 *     packet.
 *
 * (7) If LEN != 0, then AGL_GMX_RX_FRM_CHK[LENERR] will be disabled and AGL_GMX_RX_INT_REG[LENERR] will be zero
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rxx_udd_skp {
	u64 u64;
	struct cvmx_agl_gmx_rxx_udd_skp_s {
		u64 reserved_9_63 : 55;
		u64 fcssel : 1;
		u64 reserved_7_7 : 1;
		u64 len : 7;
	} s;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn52xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn52xxp1;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn56xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn56xxp1;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn61xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn63xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn63xxp1;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn66xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn68xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn68xxp1;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn70xx;
	struct cvmx_agl_gmx_rxx_udd_skp_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rxx_udd_skp cvmx_agl_gmx_rxx_udd_skp_t;

/**
 * cvmx_agl_gmx_rx_bp_drop#
 *
 * AGL_GMX_RX_BP_DROP = FIFO mark for packet drop
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rx_bp_dropx {
	u64 u64;
	struct cvmx_agl_gmx_rx_bp_dropx_s {
		u64 reserved_6_63 : 58;
		u64 mark : 6;
	} s;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn52xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn52xxp1;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn56xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn56xxp1;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn61xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn63xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn63xxp1;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn66xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn68xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn68xxp1;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn70xx;
	struct cvmx_agl_gmx_rx_bp_dropx_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rx_bp_dropx cvmx_agl_gmx_rx_bp_dropx_t;

/**
 * cvmx_agl_gmx_rx_bp_off#
 *
 * AGL_GMX_RX_BP_OFF = Lowater mark for packet drop
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rx_bp_offx {
	u64 u64;
	struct cvmx_agl_gmx_rx_bp_offx_s {
		u64 reserved_6_63 : 58;
		u64 mark : 6;
	} s;
	struct cvmx_agl_gmx_rx_bp_offx_s cn52xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn52xxp1;
	struct cvmx_agl_gmx_rx_bp_offx_s cn56xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn56xxp1;
	struct cvmx_agl_gmx_rx_bp_offx_s cn61xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn63xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn63xxp1;
	struct cvmx_agl_gmx_rx_bp_offx_s cn66xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn68xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn68xxp1;
	struct cvmx_agl_gmx_rx_bp_offx_s cn70xx;
	struct cvmx_agl_gmx_rx_bp_offx_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rx_bp_offx cvmx_agl_gmx_rx_bp_offx_t;

/**
 * cvmx_agl_gmx_rx_bp_on#
 *
 * AGL_GMX_RX_BP_ON = Hiwater mark for port/interface backpressure
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_rx_bp_onx {
	u64 u64;
	struct cvmx_agl_gmx_rx_bp_onx_s {
		u64 reserved_9_63 : 55;
		u64 mark : 9;
	} s;
	struct cvmx_agl_gmx_rx_bp_onx_s cn52xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn52xxp1;
	struct cvmx_agl_gmx_rx_bp_onx_s cn56xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn56xxp1;
	struct cvmx_agl_gmx_rx_bp_onx_s cn61xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn63xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn63xxp1;
	struct cvmx_agl_gmx_rx_bp_onx_s cn66xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn68xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn68xxp1;
	struct cvmx_agl_gmx_rx_bp_onx_s cn70xx;
	struct cvmx_agl_gmx_rx_bp_onx_s cn70xxp1;
};

typedef union cvmx_agl_gmx_rx_bp_onx cvmx_agl_gmx_rx_bp_onx_t;

/**
 * cvmx_agl_gmx_rx_prt_info
 *
 * AGL_GMX_RX_PRT_INFO = state information for the ports
 *
 *
 * Notes:
 * COMMIT[0], DROP[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * COMMIT[1], DROP[1] will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rx_prt_info {
	u64 u64;
	struct cvmx_agl_gmx_rx_prt_info_s {
		u64 reserved_18_63 : 46;
		u64 drop : 2;
		u64 reserved_2_15 : 14;
		u64 commit : 2;
	} s;
	struct cvmx_agl_gmx_rx_prt_info_s cn52xx;
	struct cvmx_agl_gmx_rx_prt_info_s cn52xxp1;
	struct cvmx_agl_gmx_rx_prt_info_cn56xx {
		u64 reserved_17_63 : 47;
		u64 drop : 1;
		u64 reserved_1_15 : 15;
		u64 commit : 1;
	} cn56xx;
	struct cvmx_agl_gmx_rx_prt_info_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_rx_prt_info_s cn61xx;
	struct cvmx_agl_gmx_rx_prt_info_s cn63xx;
	struct cvmx_agl_gmx_rx_prt_info_s cn63xxp1;
	struct cvmx_agl_gmx_rx_prt_info_s cn66xx;
	struct cvmx_agl_gmx_rx_prt_info_s cn68xx;
	struct cvmx_agl_gmx_rx_prt_info_s cn68xxp1;
	struct cvmx_agl_gmx_rx_prt_info_cn56xx cn70xx;
	struct cvmx_agl_gmx_rx_prt_info_cn56xx cn70xxp1;
};

typedef union cvmx_agl_gmx_rx_prt_info cvmx_agl_gmx_rx_prt_info_t;

/**
 * cvmx_agl_gmx_rx_tx_status
 *
 * AGL_GMX_RX_TX_STATUS = GMX RX/TX Status
 *
 *
 * Notes:
 * RX[0], TX[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * RX[1], TX[1] will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_rx_tx_status {
	u64 u64;
	struct cvmx_agl_gmx_rx_tx_status_s {
		u64 reserved_6_63 : 58;
		u64 tx : 2;
		u64 reserved_2_3 : 2;
		u64 rx : 2;
	} s;
	struct cvmx_agl_gmx_rx_tx_status_s cn52xx;
	struct cvmx_agl_gmx_rx_tx_status_s cn52xxp1;
	struct cvmx_agl_gmx_rx_tx_status_cn56xx {
		u64 reserved_5_63 : 59;
		u64 tx : 1;
		u64 reserved_1_3 : 3;
		u64 rx : 1;
	} cn56xx;
	struct cvmx_agl_gmx_rx_tx_status_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_rx_tx_status_s cn61xx;
	struct cvmx_agl_gmx_rx_tx_status_s cn63xx;
	struct cvmx_agl_gmx_rx_tx_status_s cn63xxp1;
	struct cvmx_agl_gmx_rx_tx_status_s cn66xx;
	struct cvmx_agl_gmx_rx_tx_status_s cn68xx;
	struct cvmx_agl_gmx_rx_tx_status_s cn68xxp1;
	struct cvmx_agl_gmx_rx_tx_status_cn56xx cn70xx;
	struct cvmx_agl_gmx_rx_tx_status_cn56xx cn70xxp1;
};

typedef union cvmx_agl_gmx_rx_tx_status cvmx_agl_gmx_rx_tx_status_t;

/**
 * cvmx_agl_gmx_smac#
 *
 * AGL_GMX_SMAC = Packet SMAC
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_smacx {
	u64 u64;
	struct cvmx_agl_gmx_smacx_s {
		u64 reserved_48_63 : 16;
		u64 smac : 48;
	} s;
	struct cvmx_agl_gmx_smacx_s cn52xx;
	struct cvmx_agl_gmx_smacx_s cn52xxp1;
	struct cvmx_agl_gmx_smacx_s cn56xx;
	struct cvmx_agl_gmx_smacx_s cn56xxp1;
	struct cvmx_agl_gmx_smacx_s cn61xx;
	struct cvmx_agl_gmx_smacx_s cn63xx;
	struct cvmx_agl_gmx_smacx_s cn63xxp1;
	struct cvmx_agl_gmx_smacx_s cn66xx;
	struct cvmx_agl_gmx_smacx_s cn68xx;
	struct cvmx_agl_gmx_smacx_s cn68xxp1;
	struct cvmx_agl_gmx_smacx_s cn70xx;
	struct cvmx_agl_gmx_smacx_s cn70xxp1;
};

typedef union cvmx_agl_gmx_smacx cvmx_agl_gmx_smacx_t;

/**
 * cvmx_agl_gmx_stat_bp
 *
 * AGL_GMX_STAT_BP = Number of cycles that the TX/Stats block has help up operation
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 *
 *
 * It has no relationship with the TX FIFO per se.  The TX engine sends packets
 * from PKO and upon completion, sends a command to the TX stats block for an
 * update based on the packet size.  The stats operation can take a few cycles -
 * normally not enough to be visible considering the 64B min packet size that is
 * ethernet convention.
 *
 * In the rare case in which SW attempted to schedule really, really, small packets
 * or the sclk (6xxx) is running ass-slow, then the stats updates may not happen in
 * real time and can back up the TX engine.
 *
 * This counter is the number of cycles in which the TX engine was stalled.  In
 * normal operation, it should always be zeros.
 */
union cvmx_agl_gmx_stat_bp {
	u64 u64;
	struct cvmx_agl_gmx_stat_bp_s {
		u64 reserved_17_63 : 47;
		u64 bp : 1;
		u64 cnt : 16;
	} s;
	struct cvmx_agl_gmx_stat_bp_s cn52xx;
	struct cvmx_agl_gmx_stat_bp_s cn52xxp1;
	struct cvmx_agl_gmx_stat_bp_s cn56xx;
	struct cvmx_agl_gmx_stat_bp_s cn56xxp1;
	struct cvmx_agl_gmx_stat_bp_s cn61xx;
	struct cvmx_agl_gmx_stat_bp_s cn63xx;
	struct cvmx_agl_gmx_stat_bp_s cn63xxp1;
	struct cvmx_agl_gmx_stat_bp_s cn66xx;
	struct cvmx_agl_gmx_stat_bp_s cn68xx;
	struct cvmx_agl_gmx_stat_bp_s cn68xxp1;
	struct cvmx_agl_gmx_stat_bp_s cn70xx;
	struct cvmx_agl_gmx_stat_bp_s cn70xxp1;
};

typedef union cvmx_agl_gmx_stat_bp cvmx_agl_gmx_stat_bp_t;

/**
 * cvmx_agl_gmx_tx#_append
 *
 * AGL_GMX_TX_APPEND = Packet TX Append Control
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_append {
	u64 u64;
	struct cvmx_agl_gmx_txx_append_s {
		u64 reserved_4_63 : 60;
		u64 force_fcs : 1;
		u64 fcs : 1;
		u64 pad : 1;
		u64 preamble : 1;
	} s;
	struct cvmx_agl_gmx_txx_append_s cn52xx;
	struct cvmx_agl_gmx_txx_append_s cn52xxp1;
	struct cvmx_agl_gmx_txx_append_s cn56xx;
	struct cvmx_agl_gmx_txx_append_s cn56xxp1;
	struct cvmx_agl_gmx_txx_append_s cn61xx;
	struct cvmx_agl_gmx_txx_append_s cn63xx;
	struct cvmx_agl_gmx_txx_append_s cn63xxp1;
	struct cvmx_agl_gmx_txx_append_s cn66xx;
	struct cvmx_agl_gmx_txx_append_s cn68xx;
	struct cvmx_agl_gmx_txx_append_s cn68xxp1;
	struct cvmx_agl_gmx_txx_append_s cn70xx;
	struct cvmx_agl_gmx_txx_append_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_append cvmx_agl_gmx_txx_append_t;

/**
 * cvmx_agl_gmx_tx#_clk
 *
 * AGL_GMX_TX_CLK = RGMII TX Clock Generation Register
 *
 *
 * Notes:
 * Normal Programming Values:
 *  (1) RGMII, 1000Mbs   (AGL_GMX_PRT_CFG[SPEED]==1), CLK_CNT == 1
 *  (2) RGMII, 10/100Mbs (AGL_GMX_PRT_CFG[SPEED]==0), CLK_CNT == 50/5
 *  (3) MII,   10/100Mbs (AGL_GMX_PRT_CFG[SPEED]==0), CLK_CNT == 1
 *
 * RGMII Example:
 *  Given a 125MHz PLL reference clock...
 *   CLK_CNT ==  1 ==> 125.0MHz TXC clock period (8ns* 1)
 *   CLK_CNT ==  5 ==>  25.0MHz TXC clock period (8ns* 5)
 *   CLK_CNT == 50 ==>   2.5MHz TXC clock period (8ns*50)
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_clk {
	u64 u64;
	struct cvmx_agl_gmx_txx_clk_s {
		u64 reserved_6_63 : 58;
		u64 clk_cnt : 6;
	} s;
	struct cvmx_agl_gmx_txx_clk_s cn61xx;
	struct cvmx_agl_gmx_txx_clk_s cn63xx;
	struct cvmx_agl_gmx_txx_clk_s cn63xxp1;
	struct cvmx_agl_gmx_txx_clk_s cn66xx;
	struct cvmx_agl_gmx_txx_clk_s cn68xx;
	struct cvmx_agl_gmx_txx_clk_s cn68xxp1;
	struct cvmx_agl_gmx_txx_clk_s cn70xx;
	struct cvmx_agl_gmx_txx_clk_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_clk cvmx_agl_gmx_txx_clk_t;

/**
 * cvmx_agl_gmx_tx#_ctl
 *
 * AGL_GMX_TX_CTL = TX Control register
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_ctl {
	u64 u64;
	struct cvmx_agl_gmx_txx_ctl_s {
		u64 reserved_2_63 : 62;
		u64 xsdef_en : 1;
		u64 xscol_en : 1;
	} s;
	struct cvmx_agl_gmx_txx_ctl_s cn52xx;
	struct cvmx_agl_gmx_txx_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_txx_ctl_s cn56xx;
	struct cvmx_agl_gmx_txx_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_txx_ctl_s cn61xx;
	struct cvmx_agl_gmx_txx_ctl_s cn63xx;
	struct cvmx_agl_gmx_txx_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_txx_ctl_s cn66xx;
	struct cvmx_agl_gmx_txx_ctl_s cn68xx;
	struct cvmx_agl_gmx_txx_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_txx_ctl_s cn70xx;
	struct cvmx_agl_gmx_txx_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_ctl cvmx_agl_gmx_txx_ctl_t;

/**
 * cvmx_agl_gmx_tx#_min_pkt
 *
 * AGL_GMX_TX_MIN_PKT = Packet TX Min Size Packet (PAD upto min size)
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_min_pkt {
	u64 u64;
	struct cvmx_agl_gmx_txx_min_pkt_s {
		u64 reserved_8_63 : 56;
		u64 min_size : 8;
	} s;
	struct cvmx_agl_gmx_txx_min_pkt_s cn52xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn52xxp1;
	struct cvmx_agl_gmx_txx_min_pkt_s cn56xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn56xxp1;
	struct cvmx_agl_gmx_txx_min_pkt_s cn61xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn63xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn63xxp1;
	struct cvmx_agl_gmx_txx_min_pkt_s cn66xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn68xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn68xxp1;
	struct cvmx_agl_gmx_txx_min_pkt_s cn70xx;
	struct cvmx_agl_gmx_txx_min_pkt_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_min_pkt cvmx_agl_gmx_txx_min_pkt_t;

/**
 * cvmx_agl_gmx_tx#_pause_pkt_interval
 *
 * AGL_GMX_TX_PAUSE_PKT_INTERVAL = Packet TX Pause Packet transmission interval - how often PAUSE packets will be sent
 *
 *
 * Notes:
 * Choosing proper values of AGL_GMX_TX_PAUSE_PKT_TIME[TIME] and
 * AGL_GMX_TX_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to the system
 * designer.  It is suggested that TIME be much greater than INTERVAL and
 * AGL_GMX_TX_PAUSE_ZERO[SEND] be set.  This allows a periodic refresh of the PAUSE
 * count and then when the backpressure condition is lifted, a PAUSE packet
 * with TIME==0 will be sent indicating that Octane is ready for additional
 * data.
 *
 * If the system chooses to not set AGL_GMX_TX_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they satisify the
 * following rule...
 *
 *    INTERVAL <= TIME - (largest_pkt_size + IFG + pause_pkt_size)
 *
 * where largest_pkt_size is that largest packet that the system can send
 * (normally 1518B), IFG is the interframe gap and pause_pkt_size is the size
 * of the PAUSE packet (normally 64B).
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_pause_pkt_interval {
	u64 u64;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s {
		u64 reserved_16_63 : 48;
		u64 interval : 16;
	} s;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn52xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn52xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn56xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn56xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn61xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn63xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn63xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn66xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn68xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn68xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn70xx;
	struct cvmx_agl_gmx_txx_pause_pkt_interval_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_pause_pkt_interval cvmx_agl_gmx_txx_pause_pkt_interval_t;

/**
 * cvmx_agl_gmx_tx#_pause_pkt_time
 *
 * AGL_GMX_TX_PAUSE_PKT_TIME = Packet TX Pause Packet pause_time field
 *
 *
 * Notes:
 * Choosing proper values of AGL_GMX_TX_PAUSE_PKT_TIME[TIME] and
 * AGL_GMX_TX_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to the system
 * designer.  It is suggested that TIME be much greater than INTERVAL and
 * AGL_GMX_TX_PAUSE_ZERO[SEND] be set.  This allows a periodic refresh of the PAUSE
 * count and then when the backpressure condition is lifted, a PAUSE packet
 * with TIME==0 will be sent indicating that Octane is ready for additional
 * data.
 *
 * If the system chooses to not set AGL_GMX_TX_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they satisify the
 * following rule...
 *
 *    INTERVAL <= TIME - (largest_pkt_size + IFG + pause_pkt_size)
 *
 * where largest_pkt_size is that largest packet that the system can send
 * (normally 1518B), IFG is the interframe gap and pause_pkt_size is the size
 * of the PAUSE packet (normally 64B).
 *
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_pause_pkt_time {
	u64 u64;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} s;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn52xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn52xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn56xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn56xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn61xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn63xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn63xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn66xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn68xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn68xxp1;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn70xx;
	struct cvmx_agl_gmx_txx_pause_pkt_time_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_pause_pkt_time cvmx_agl_gmx_txx_pause_pkt_time_t;

/**
 * cvmx_agl_gmx_tx#_pause_togo
 *
 * AGL_GMX_TX_PAUSE_TOGO = Packet TX Amount of time remaining to backpressure
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_pause_togo {
	u64 u64;
	struct cvmx_agl_gmx_txx_pause_togo_s {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} s;
	struct cvmx_agl_gmx_txx_pause_togo_s cn52xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn52xxp1;
	struct cvmx_agl_gmx_txx_pause_togo_s cn56xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn56xxp1;
	struct cvmx_agl_gmx_txx_pause_togo_s cn61xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn63xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn63xxp1;
	struct cvmx_agl_gmx_txx_pause_togo_s cn66xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn68xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn68xxp1;
	struct cvmx_agl_gmx_txx_pause_togo_s cn70xx;
	struct cvmx_agl_gmx_txx_pause_togo_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_pause_togo cvmx_agl_gmx_txx_pause_togo_t;

/**
 * cvmx_agl_gmx_tx#_pause_zero
 *
 * AGL_GMX_TX_PAUSE_ZERO = Packet TX Amount of time remaining to backpressure
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_pause_zero {
	u64 u64;
	struct cvmx_agl_gmx_txx_pause_zero_s {
		u64 reserved_1_63 : 63;
		u64 send : 1;
	} s;
	struct cvmx_agl_gmx_txx_pause_zero_s cn52xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn52xxp1;
	struct cvmx_agl_gmx_txx_pause_zero_s cn56xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn56xxp1;
	struct cvmx_agl_gmx_txx_pause_zero_s cn61xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn63xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn63xxp1;
	struct cvmx_agl_gmx_txx_pause_zero_s cn66xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn68xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn68xxp1;
	struct cvmx_agl_gmx_txx_pause_zero_s cn70xx;
	struct cvmx_agl_gmx_txx_pause_zero_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_pause_zero cvmx_agl_gmx_txx_pause_zero_t;

/**
 * cvmx_agl_gmx_tx#_soft_pause
 *
 * AGL_GMX_TX_SOFT_PAUSE = Packet TX Software Pause
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_soft_pause {
	u64 u64;
	struct cvmx_agl_gmx_txx_soft_pause_s {
		u64 reserved_16_63 : 48;
		u64 time : 16;
	} s;
	struct cvmx_agl_gmx_txx_soft_pause_s cn52xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn52xxp1;
	struct cvmx_agl_gmx_txx_soft_pause_s cn56xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn56xxp1;
	struct cvmx_agl_gmx_txx_soft_pause_s cn61xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn63xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn63xxp1;
	struct cvmx_agl_gmx_txx_soft_pause_s cn66xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn68xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn68xxp1;
	struct cvmx_agl_gmx_txx_soft_pause_s cn70xx;
	struct cvmx_agl_gmx_txx_soft_pause_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_soft_pause cvmx_agl_gmx_txx_soft_pause_t;

/**
 * cvmx_agl_gmx_tx#_stat0
 *
 * AGL_GMX_TX_STAT0 = AGL_GMX_TX_STATS_XSDEF / AGL_GMX_TX_STATS_XSCOL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat0 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat0_s {
		u64 xsdef : 32;
		u64 xscol : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat0_s cn52xx;
	struct cvmx_agl_gmx_txx_stat0_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat0_s cn56xx;
	struct cvmx_agl_gmx_txx_stat0_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat0_s cn61xx;
	struct cvmx_agl_gmx_txx_stat0_s cn63xx;
	struct cvmx_agl_gmx_txx_stat0_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat0_s cn66xx;
	struct cvmx_agl_gmx_txx_stat0_s cn68xx;
	struct cvmx_agl_gmx_txx_stat0_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat0_s cn70xx;
	struct cvmx_agl_gmx_txx_stat0_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat0 cvmx_agl_gmx_txx_stat0_t;

/**
 * cvmx_agl_gmx_tx#_stat1
 *
 * AGL_GMX_TX_STAT1 = AGL_GMX_TX_STATS_SCOL  / AGL_GMX_TX_STATS_MCOL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat1 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat1_s {
		u64 scol : 32;
		u64 mcol : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat1_s cn52xx;
	struct cvmx_agl_gmx_txx_stat1_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat1_s cn56xx;
	struct cvmx_agl_gmx_txx_stat1_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat1_s cn61xx;
	struct cvmx_agl_gmx_txx_stat1_s cn63xx;
	struct cvmx_agl_gmx_txx_stat1_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat1_s cn66xx;
	struct cvmx_agl_gmx_txx_stat1_s cn68xx;
	struct cvmx_agl_gmx_txx_stat1_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat1_s cn70xx;
	struct cvmx_agl_gmx_txx_stat1_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat1 cvmx_agl_gmx_txx_stat1_t;

/**
 * cvmx_agl_gmx_tx#_stat2
 *
 * AGL_GMX_TX_STAT2 = AGL_GMX_TX_STATS_OCTS
 *
 *
 * Notes:
 * - Octect counts are the sum of all data transmitted on the wire including
 *   packet data, pad bytes, fcs bytes, pause bytes, and jam bytes.  The octect
 *   counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat2 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat2_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_agl_gmx_txx_stat2_s cn52xx;
	struct cvmx_agl_gmx_txx_stat2_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat2_s cn56xx;
	struct cvmx_agl_gmx_txx_stat2_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat2_s cn61xx;
	struct cvmx_agl_gmx_txx_stat2_s cn63xx;
	struct cvmx_agl_gmx_txx_stat2_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat2_s cn66xx;
	struct cvmx_agl_gmx_txx_stat2_s cn68xx;
	struct cvmx_agl_gmx_txx_stat2_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat2_s cn70xx;
	struct cvmx_agl_gmx_txx_stat2_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat2 cvmx_agl_gmx_txx_stat2_t;

/**
 * cvmx_agl_gmx_tx#_stat3
 *
 * AGL_GMX_TX_STAT3 = AGL_GMX_TX_STATS_PKTS
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat3 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat3_s {
		u64 reserved_32_63 : 32;
		u64 pkts : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat3_s cn52xx;
	struct cvmx_agl_gmx_txx_stat3_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat3_s cn56xx;
	struct cvmx_agl_gmx_txx_stat3_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat3_s cn61xx;
	struct cvmx_agl_gmx_txx_stat3_s cn63xx;
	struct cvmx_agl_gmx_txx_stat3_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat3_s cn66xx;
	struct cvmx_agl_gmx_txx_stat3_s cn68xx;
	struct cvmx_agl_gmx_txx_stat3_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat3_s cn70xx;
	struct cvmx_agl_gmx_txx_stat3_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat3 cvmx_agl_gmx_txx_stat3_t;

/**
 * cvmx_agl_gmx_tx#_stat4
 *
 * AGL_GMX_TX_STAT4 = AGL_GMX_TX_STATS_HIST1 (64) / AGL_GMX_TX_STATS_HIST0 (<64)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat4 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat4_s {
		u64 hist1 : 32;
		u64 hist0 : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat4_s cn52xx;
	struct cvmx_agl_gmx_txx_stat4_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat4_s cn56xx;
	struct cvmx_agl_gmx_txx_stat4_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat4_s cn61xx;
	struct cvmx_agl_gmx_txx_stat4_s cn63xx;
	struct cvmx_agl_gmx_txx_stat4_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat4_s cn66xx;
	struct cvmx_agl_gmx_txx_stat4_s cn68xx;
	struct cvmx_agl_gmx_txx_stat4_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat4_s cn70xx;
	struct cvmx_agl_gmx_txx_stat4_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat4 cvmx_agl_gmx_txx_stat4_t;

/**
 * cvmx_agl_gmx_tx#_stat5
 *
 * AGL_GMX_TX_STAT5 = AGL_GMX_TX_STATS_HIST3 (128- 255) / AGL_GMX_TX_STATS_HIST2 (65- 127)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat5 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat5_s {
		u64 hist3 : 32;
		u64 hist2 : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat5_s cn52xx;
	struct cvmx_agl_gmx_txx_stat5_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat5_s cn56xx;
	struct cvmx_agl_gmx_txx_stat5_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat5_s cn61xx;
	struct cvmx_agl_gmx_txx_stat5_s cn63xx;
	struct cvmx_agl_gmx_txx_stat5_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat5_s cn66xx;
	struct cvmx_agl_gmx_txx_stat5_s cn68xx;
	struct cvmx_agl_gmx_txx_stat5_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat5_s cn70xx;
	struct cvmx_agl_gmx_txx_stat5_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat5 cvmx_agl_gmx_txx_stat5_t;

/**
 * cvmx_agl_gmx_tx#_stat6
 *
 * AGL_GMX_TX_STAT6 = AGL_GMX_TX_STATS_HIST5 (512-1023) / AGL_GMX_TX_STATS_HIST4 (256-511)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat6 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat6_s {
		u64 hist5 : 32;
		u64 hist4 : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat6_s cn52xx;
	struct cvmx_agl_gmx_txx_stat6_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat6_s cn56xx;
	struct cvmx_agl_gmx_txx_stat6_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat6_s cn61xx;
	struct cvmx_agl_gmx_txx_stat6_s cn63xx;
	struct cvmx_agl_gmx_txx_stat6_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat6_s cn66xx;
	struct cvmx_agl_gmx_txx_stat6_s cn68xx;
	struct cvmx_agl_gmx_txx_stat6_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat6_s cn70xx;
	struct cvmx_agl_gmx_txx_stat6_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat6 cvmx_agl_gmx_txx_stat6_t;

/**
 * cvmx_agl_gmx_tx#_stat7
 *
 * AGL_GMX_TX_STAT7 = AGL_GMX_TX_STATS_HIST7 (1024-1518) / AGL_GMX_TX_STATS_HIST6 (>1518)
 *
 *
 * Notes:
 * - Packet length is the sum of all data transmitted on the wire for the given
 *   packet including packet data, pad bytes, fcs bytes, pause bytes, and jam
 *   bytes.  The octect counts do not include PREAMBLE byte or EXTEND cycles.
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat7 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat7_s {
		u64 hist7 : 32;
		u64 hist6 : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat7_s cn52xx;
	struct cvmx_agl_gmx_txx_stat7_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat7_s cn56xx;
	struct cvmx_agl_gmx_txx_stat7_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat7_s cn61xx;
	struct cvmx_agl_gmx_txx_stat7_s cn63xx;
	struct cvmx_agl_gmx_txx_stat7_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat7_s cn66xx;
	struct cvmx_agl_gmx_txx_stat7_s cn68xx;
	struct cvmx_agl_gmx_txx_stat7_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat7_s cn70xx;
	struct cvmx_agl_gmx_txx_stat7_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat7 cvmx_agl_gmx_txx_stat7_t;

/**
 * cvmx_agl_gmx_tx#_stat8
 *
 * AGL_GMX_TX_STAT8 = AGL_GMX_TX_STATS_MCST  / AGL_GMX_TX_STATS_BCST
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Note, GMX determines if the packet is MCST or BCST from the DMAC of the
 *   packet.  GMX assumes that the DMAC lies in the first 6 bytes of the packet
 *   as per the 802.3 frame definition.  If the system requires additional data
 *   before the L2 header, then the MCST and BCST counters may not reflect
 *   reality and should be ignored by software.
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat8 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat8_s {
		u64 mcst : 32;
		u64 bcst : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat8_s cn52xx;
	struct cvmx_agl_gmx_txx_stat8_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat8_s cn56xx;
	struct cvmx_agl_gmx_txx_stat8_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat8_s cn61xx;
	struct cvmx_agl_gmx_txx_stat8_s cn63xx;
	struct cvmx_agl_gmx_txx_stat8_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat8_s cn66xx;
	struct cvmx_agl_gmx_txx_stat8_s cn68xx;
	struct cvmx_agl_gmx_txx_stat8_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat8_s cn70xx;
	struct cvmx_agl_gmx_txx_stat8_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat8 cvmx_agl_gmx_txx_stat8_t;

/**
 * cvmx_agl_gmx_tx#_stat9
 *
 * AGL_GMX_TX_STAT9 = AGL_GMX_TX_STATS_UNDFLW / AGL_GMX_TX_STATS_CTL
 *
 *
 * Notes:
 * - Cleared either by a write (of any value) or a read when AGL_GMX_TX_STATS_CTL[RD_CLR] is set
 * - Counters will wrap
 * - Not reset when MIX*_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_txx_stat9 {
	u64 u64;
	struct cvmx_agl_gmx_txx_stat9_s {
		u64 undflw : 32;
		u64 ctl : 32;
	} s;
	struct cvmx_agl_gmx_txx_stat9_s cn52xx;
	struct cvmx_agl_gmx_txx_stat9_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stat9_s cn56xx;
	struct cvmx_agl_gmx_txx_stat9_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stat9_s cn61xx;
	struct cvmx_agl_gmx_txx_stat9_s cn63xx;
	struct cvmx_agl_gmx_txx_stat9_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stat9_s cn66xx;
	struct cvmx_agl_gmx_txx_stat9_s cn68xx;
	struct cvmx_agl_gmx_txx_stat9_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stat9_s cn70xx;
	struct cvmx_agl_gmx_txx_stat9_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stat9 cvmx_agl_gmx_txx_stat9_t;

/**
 * cvmx_agl_gmx_tx#_stats_ctl
 *
 * AGL_GMX_TX_STATS_CTL = TX Stats Control register
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_stats_ctl {
	u64 u64;
	struct cvmx_agl_gmx_txx_stats_ctl_s {
		u64 reserved_1_63 : 63;
		u64 rd_clr : 1;
	} s;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn52xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn52xxp1;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn56xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn56xxp1;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn61xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn63xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn63xxp1;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn66xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn68xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn68xxp1;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn70xx;
	struct cvmx_agl_gmx_txx_stats_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_stats_ctl cvmx_agl_gmx_txx_stats_ctl_t;

/**
 * cvmx_agl_gmx_tx#_thresh
 *
 * AGL_GMX_TX_THRESH = Packet TX Threshold
 *
 *
 * Notes:
 * Additionally reset when MIX<prt>_CTL[RESET] is set to 1.
 *
 */
union cvmx_agl_gmx_txx_thresh {
	u64 u64;
	struct cvmx_agl_gmx_txx_thresh_s {
		u64 reserved_6_63 : 58;
		u64 cnt : 6;
	} s;
	struct cvmx_agl_gmx_txx_thresh_s cn52xx;
	struct cvmx_agl_gmx_txx_thresh_s cn52xxp1;
	struct cvmx_agl_gmx_txx_thresh_s cn56xx;
	struct cvmx_agl_gmx_txx_thresh_s cn56xxp1;
	struct cvmx_agl_gmx_txx_thresh_s cn61xx;
	struct cvmx_agl_gmx_txx_thresh_s cn63xx;
	struct cvmx_agl_gmx_txx_thresh_s cn63xxp1;
	struct cvmx_agl_gmx_txx_thresh_s cn66xx;
	struct cvmx_agl_gmx_txx_thresh_s cn68xx;
	struct cvmx_agl_gmx_txx_thresh_s cn68xxp1;
	struct cvmx_agl_gmx_txx_thresh_s cn70xx;
	struct cvmx_agl_gmx_txx_thresh_s cn70xxp1;
};

typedef union cvmx_agl_gmx_txx_thresh cvmx_agl_gmx_txx_thresh_t;

/**
 * cvmx_agl_gmx_tx_bp
 *
 * AGL_GMX_TX_BP = Packet TX BackPressure Register
 *
 *
 * Notes:
 * BP[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * BP[1] will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_tx_bp {
	u64 u64;
	struct cvmx_agl_gmx_tx_bp_s {
		u64 reserved_2_63 : 62;
		u64 bp : 2;
	} s;
	struct cvmx_agl_gmx_tx_bp_s cn52xx;
	struct cvmx_agl_gmx_tx_bp_s cn52xxp1;
	struct cvmx_agl_gmx_tx_bp_cn56xx {
		u64 reserved_1_63 : 63;
		u64 bp : 1;
	} cn56xx;
	struct cvmx_agl_gmx_tx_bp_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_tx_bp_s cn61xx;
	struct cvmx_agl_gmx_tx_bp_s cn63xx;
	struct cvmx_agl_gmx_tx_bp_s cn63xxp1;
	struct cvmx_agl_gmx_tx_bp_s cn66xx;
	struct cvmx_agl_gmx_tx_bp_s cn68xx;
	struct cvmx_agl_gmx_tx_bp_s cn68xxp1;
	struct cvmx_agl_gmx_tx_bp_cn56xx cn70xx;
	struct cvmx_agl_gmx_tx_bp_cn56xx cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_bp cvmx_agl_gmx_tx_bp_t;

/**
 * cvmx_agl_gmx_tx_col_attempt
 *
 * AGL_GMX_TX_COL_ATTEMPT = Packet TX collision attempts before dropping frame
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 */
union cvmx_agl_gmx_tx_col_attempt {
	u64 u64;
	struct cvmx_agl_gmx_tx_col_attempt_s {
		u64 reserved_5_63 : 59;
		u64 limit : 5;
	} s;
	struct cvmx_agl_gmx_tx_col_attempt_s cn52xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn52xxp1;
	struct cvmx_agl_gmx_tx_col_attempt_s cn56xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn56xxp1;
	struct cvmx_agl_gmx_tx_col_attempt_s cn61xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn63xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn63xxp1;
	struct cvmx_agl_gmx_tx_col_attempt_s cn66xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn68xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn68xxp1;
	struct cvmx_agl_gmx_tx_col_attempt_s cn70xx;
	struct cvmx_agl_gmx_tx_col_attempt_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_col_attempt cvmx_agl_gmx_tx_col_attempt_t;

/**
 * cvmx_agl_gmx_tx_ifg
 *
 * Common
 * AGL_GMX_TX_IFG = Packet TX Interframe Gap
 */
union cvmx_agl_gmx_tx_ifg {
	u64 u64;
	struct cvmx_agl_gmx_tx_ifg_s {
		u64 reserved_8_63 : 56;
		u64 ifg2 : 4;
		u64 ifg1 : 4;
	} s;
	struct cvmx_agl_gmx_tx_ifg_s cn52xx;
	struct cvmx_agl_gmx_tx_ifg_s cn52xxp1;
	struct cvmx_agl_gmx_tx_ifg_s cn56xx;
	struct cvmx_agl_gmx_tx_ifg_s cn56xxp1;
	struct cvmx_agl_gmx_tx_ifg_s cn61xx;
	struct cvmx_agl_gmx_tx_ifg_s cn63xx;
	struct cvmx_agl_gmx_tx_ifg_s cn63xxp1;
	struct cvmx_agl_gmx_tx_ifg_s cn66xx;
	struct cvmx_agl_gmx_tx_ifg_s cn68xx;
	struct cvmx_agl_gmx_tx_ifg_s cn68xxp1;
	struct cvmx_agl_gmx_tx_ifg_s cn70xx;
	struct cvmx_agl_gmx_tx_ifg_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_ifg cvmx_agl_gmx_tx_ifg_t;

/**
 * cvmx_agl_gmx_tx_int_en
 *
 * AGL_GMX_TX_INT_EN = Interrupt Enable
 *
 *
 * Notes:
 * UNDFLW[0], XSCOL[0], XSDEF[0], LATE_COL[0], PTP_LOST[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * UNDFLW[1], XSCOL[1], XSDEF[1], LATE_COL[1], PTP_LOST[1] will be reset when MIX1_CTL[RESET] is set to 1.
 * PKO_NXA will bee reset when both MIX0/1_CTL[RESET] are set to 1.
 */
union cvmx_agl_gmx_tx_int_en {
	u64 u64;
	struct cvmx_agl_gmx_tx_int_en_s {
		u64 reserved_22_63 : 42;
		u64 ptp_lost : 2;
		u64 reserved_18_19 : 2;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} s;
	struct cvmx_agl_gmx_tx_int_en_cn52xx {
		u64 reserved_18_63 : 46;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn52xx;
	struct cvmx_agl_gmx_tx_int_en_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_tx_int_en_cn56xx {
		u64 reserved_17_63 : 47;
		u64 late_col : 1;
		u64 reserved_13_15 : 3;
		u64 xsdef : 1;
		u64 reserved_9_11 : 3;
		u64 xscol : 1;
		u64 reserved_3_7 : 5;
		u64 undflw : 1;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn56xx;
	struct cvmx_agl_gmx_tx_int_en_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_tx_int_en_s cn61xx;
	struct cvmx_agl_gmx_tx_int_en_s cn63xx;
	struct cvmx_agl_gmx_tx_int_en_s cn63xxp1;
	struct cvmx_agl_gmx_tx_int_en_s cn66xx;
	struct cvmx_agl_gmx_tx_int_en_s cn68xx;
	struct cvmx_agl_gmx_tx_int_en_s cn68xxp1;
	struct cvmx_agl_gmx_tx_int_en_cn70xx {
		u64 reserved_21_63 : 43;
		u64 ptp_lost : 1;
		u64 reserved_17_19 : 3;
		u64 late_col : 1;
		u64 reserved_13_15 : 3;
		u64 xsdef : 1;
		u64 reserved_9_11 : 3;
		u64 xscol : 1;
		u64 reserved_3_7 : 5;
		u64 undflw : 1;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn70xx;
	struct cvmx_agl_gmx_tx_int_en_cn70xx cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_int_en cvmx_agl_gmx_tx_int_en_t;

/**
 * cvmx_agl_gmx_tx_int_reg
 *
 * AGL_GMX_TX_INT_REG = Interrupt Register
 *
 *
 * Notes:
 * UNDFLW[0], XSCOL[0], XSDEF[0], LATE_COL[0], PTP_LOST[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * UNDFLW[1], XSCOL[1], XSDEF[1], LATE_COL[1], PTP_LOST[1] will be reset when MIX1_CTL[RESET] is set to 1.
 * PKO_NXA will bee reset when both MIX0/1_CTL[RESET] are set to 1.
 */
union cvmx_agl_gmx_tx_int_reg {
	u64 u64;
	struct cvmx_agl_gmx_tx_int_reg_s {
		u64 reserved_22_63 : 42;
		u64 ptp_lost : 2;
		u64 reserved_18_19 : 2;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} s;
	struct cvmx_agl_gmx_tx_int_reg_cn52xx {
		u64 reserved_18_63 : 46;
		u64 late_col : 2;
		u64 reserved_14_15 : 2;
		u64 xsdef : 2;
		u64 reserved_10_11 : 2;
		u64 xscol : 2;
		u64 reserved_4_7 : 4;
		u64 undflw : 2;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn52xx;
	struct cvmx_agl_gmx_tx_int_reg_cn52xx cn52xxp1;
	struct cvmx_agl_gmx_tx_int_reg_cn56xx {
		u64 reserved_17_63 : 47;
		u64 late_col : 1;
		u64 reserved_13_15 : 3;
		u64 xsdef : 1;
		u64 reserved_9_11 : 3;
		u64 xscol : 1;
		u64 reserved_3_7 : 5;
		u64 undflw : 1;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn56xx;
	struct cvmx_agl_gmx_tx_int_reg_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_tx_int_reg_s cn61xx;
	struct cvmx_agl_gmx_tx_int_reg_s cn63xx;
	struct cvmx_agl_gmx_tx_int_reg_s cn63xxp1;
	struct cvmx_agl_gmx_tx_int_reg_s cn66xx;
	struct cvmx_agl_gmx_tx_int_reg_s cn68xx;
	struct cvmx_agl_gmx_tx_int_reg_s cn68xxp1;
	struct cvmx_agl_gmx_tx_int_reg_cn70xx {
		u64 reserved_21_63 : 43;
		u64 ptp_lost : 1;
		u64 reserved_17_19 : 3;
		u64 late_col : 1;
		u64 reserved_13_15 : 3;
		u64 xsdef : 1;
		u64 reserved_9_11 : 3;
		u64 xscol : 1;
		u64 reserved_3_7 : 5;
		u64 undflw : 1;
		u64 reserved_1_1 : 1;
		u64 pko_nxa : 1;
	} cn70xx;
	struct cvmx_agl_gmx_tx_int_reg_cn70xx cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_int_reg cvmx_agl_gmx_tx_int_reg_t;

/**
 * cvmx_agl_gmx_tx_jam
 *
 * AGL_GMX_TX_JAM = Packet TX Jam Pattern
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 */
union cvmx_agl_gmx_tx_jam {
	u64 u64;
	struct cvmx_agl_gmx_tx_jam_s {
		u64 reserved_8_63 : 56;
		u64 jam : 8;
	} s;
	struct cvmx_agl_gmx_tx_jam_s cn52xx;
	struct cvmx_agl_gmx_tx_jam_s cn52xxp1;
	struct cvmx_agl_gmx_tx_jam_s cn56xx;
	struct cvmx_agl_gmx_tx_jam_s cn56xxp1;
	struct cvmx_agl_gmx_tx_jam_s cn61xx;
	struct cvmx_agl_gmx_tx_jam_s cn63xx;
	struct cvmx_agl_gmx_tx_jam_s cn63xxp1;
	struct cvmx_agl_gmx_tx_jam_s cn66xx;
	struct cvmx_agl_gmx_tx_jam_s cn68xx;
	struct cvmx_agl_gmx_tx_jam_s cn68xxp1;
	struct cvmx_agl_gmx_tx_jam_s cn70xx;
	struct cvmx_agl_gmx_tx_jam_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_jam cvmx_agl_gmx_tx_jam_t;

/**
 * cvmx_agl_gmx_tx_lfsr
 *
 * AGL_GMX_TX_LFSR = LFSR used to implement truncated binary exponential backoff
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 */
union cvmx_agl_gmx_tx_lfsr {
	u64 u64;
	struct cvmx_agl_gmx_tx_lfsr_s {
		u64 reserved_16_63 : 48;
		u64 lfsr : 16;
	} s;
	struct cvmx_agl_gmx_tx_lfsr_s cn52xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn52xxp1;
	struct cvmx_agl_gmx_tx_lfsr_s cn56xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn56xxp1;
	struct cvmx_agl_gmx_tx_lfsr_s cn61xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn63xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn63xxp1;
	struct cvmx_agl_gmx_tx_lfsr_s cn66xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn68xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn68xxp1;
	struct cvmx_agl_gmx_tx_lfsr_s cn70xx;
	struct cvmx_agl_gmx_tx_lfsr_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_lfsr cvmx_agl_gmx_tx_lfsr_t;

/**
 * cvmx_agl_gmx_tx_ovr_bp
 *
 * AGL_GMX_TX_OVR_BP = Packet TX Override BackPressure
 *
 *
 * Notes:
 * IGN_FULL[0], BP[0], EN[0] will be reset when MIX0_CTL[RESET] is set to 1.
 * IGN_FULL[1], BP[1], EN[1] will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_gmx_tx_ovr_bp {
	u64 u64;
	struct cvmx_agl_gmx_tx_ovr_bp_s {
		u64 reserved_10_63 : 54;
		u64 en : 2;
		u64 reserved_6_7 : 2;
		u64 bp : 2;
		u64 reserved_2_3 : 2;
		u64 ign_full : 2;
	} s;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn52xx;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn52xxp1;
	struct cvmx_agl_gmx_tx_ovr_bp_cn56xx {
		u64 reserved_9_63 : 55;
		u64 en : 1;
		u64 reserved_5_7 : 3;
		u64 bp : 1;
		u64 reserved_1_3 : 3;
		u64 ign_full : 1;
	} cn56xx;
	struct cvmx_agl_gmx_tx_ovr_bp_cn56xx cn56xxp1;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn61xx;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn63xx;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn63xxp1;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn66xx;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn68xx;
	struct cvmx_agl_gmx_tx_ovr_bp_s cn68xxp1;
	struct cvmx_agl_gmx_tx_ovr_bp_cn56xx cn70xx;
	struct cvmx_agl_gmx_tx_ovr_bp_cn56xx cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_ovr_bp cvmx_agl_gmx_tx_ovr_bp_t;

/**
 * cvmx_agl_gmx_tx_pause_pkt_dmac
 *
 * AGL_GMX_TX_PAUSE_PKT_DMAC = Packet TX Pause Packet DMAC field
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 */
union cvmx_agl_gmx_tx_pause_pkt_dmac {
	u64 u64;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s {
		u64 reserved_48_63 : 16;
		u64 dmac : 48;
	} s;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn52xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn52xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn56xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn56xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn61xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn63xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn63xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn66xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn68xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn68xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn70xx;
	struct cvmx_agl_gmx_tx_pause_pkt_dmac_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_pause_pkt_dmac cvmx_agl_gmx_tx_pause_pkt_dmac_t;

/**
 * cvmx_agl_gmx_tx_pause_pkt_type
 *
 * AGL_GMX_TX_PAUSE_PKT_TYPE = Packet TX Pause Packet TYPE field
 *
 *
 * Notes:
 * Additionally reset when both MIX0/1_CTL[RESET] are set to 1.
 *
 */
union cvmx_agl_gmx_tx_pause_pkt_type {
	u64 u64;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s {
		u64 reserved_16_63 : 48;
		u64 type : 16;
	} s;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn52xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn52xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn56xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn56xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn61xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn63xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn63xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn66xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn68xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn68xxp1;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn70xx;
	struct cvmx_agl_gmx_tx_pause_pkt_type_s cn70xxp1;
};

typedef union cvmx_agl_gmx_tx_pause_pkt_type cvmx_agl_gmx_tx_pause_pkt_type_t;

/**
 * cvmx_agl_gmx_wol_ctl
 */
union cvmx_agl_gmx_wol_ctl {
	u64 u64;
	struct cvmx_agl_gmx_wol_ctl_s {
		u64 reserved_33_63 : 31;
		u64 magic_en : 1;
		u64 reserved_17_31 : 15;
		u64 direct_en : 1;
		u64 reserved_1_15 : 15;
		u64 en : 1;
	} s;
	struct cvmx_agl_gmx_wol_ctl_s cn70xx;
	struct cvmx_agl_gmx_wol_ctl_s cn70xxp1;
};

typedef union cvmx_agl_gmx_wol_ctl cvmx_agl_gmx_wol_ctl_t;

/**
 * cvmx_agl_prt#_ctl
 *
 * AGL_PRT_CTL = AGL Port Control
 *
 *
 * Notes:
 * The RGMII timing specification requires that devices transmit clock and
 * data synchronously. The specification requires external sources (namely
 * the PC board trace routes) to introduce the appropriate 1.5 to 2.0 ns of
 * delay.
 *
 * To eliminate the need for the PC board delays, the MIX RGMII interface
 * has optional onboard DLL's for both transmit and receive. For correct
 * operation, at most one of the transmitter, board, or receiver involved
 * in an RGMII link should introduce delay. By default/reset,
 * the MIX RGMII receivers delay the received clock, and the MIX
 * RGMII transmitters do not delay the transmitted clock. Whether this
 * default works as-is with a given link partner depends on the behavior
 * of the link partner and the PC board.
 *
 * These are the possible modes of MIX RGMII receive operation:
 *  o AGL_PRTx_CTL[CLKRX_BYP] = 0 (reset value) - The OCTEON MIX RGMII
 *    receive interface introduces clock delay using its internal DLL.
 *    This mode is appropriate if neither the remote
 *    transmitter nor the PC board delays the clock.
 *  o AGL_PRTx_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The OCTEON MIX
 *    RGMII receive interface introduces no clock delay. This mode
 *    is appropriate if either the remote transmitter or the PC board
 *    delays the clock.
 *
 * These are the possible modes of MIX RGMII transmit operation:
 *  o AGL_PRTx_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) -
 *    The OCTEON MIX RGMII transmit interface introduces no clock
 *    delay. This mode is appropriate is either the remote receiver
 *    or the PC board delays the clock.
 *  o AGL_PRTx_CTL[CLKTX_BYP] = 0 - The OCTEON MIX RGMII transmit
 *    interface introduces clock delay using its internal DLL.
 *    This mode is appropriate if neither the remote receiver
 *    nor the PC board delays the clock.
 *
 * AGL_PRT0_CTL will be reset when MIX0_CTL[RESET] is set to 1.
 * AGL_PRT1_CTL will be reset when MIX1_CTL[RESET] is set to 1.
 */
union cvmx_agl_prtx_ctl {
	u64 u64;
	struct cvmx_agl_prtx_ctl_s {
		u64 drv_byp : 1;
		u64 reserved_62_62 : 1;
		u64 cmp_pctl : 6;
		u64 reserved_54_55 : 2;
		u64 cmp_nctl : 6;
		u64 reserved_46_47 : 2;
		u64 drv_pctl : 6;
		u64 reserved_38_39 : 2;
		u64 drv_nctl : 6;
		u64 reserved_31_31 : 1;
		u64 clk_set : 7;
		u64 clkrx_byp : 1;
		u64 clkrx_set : 7;
		u64 clktx_byp : 1;
		u64 clktx_set : 7;
		u64 refclk_sel : 2;
		u64 reserved_5_5 : 1;
		u64 dllrst : 1;
		u64 comp : 1;
		u64 enable : 1;
		u64 clkrst : 1;
		u64 mode : 1;
	} s;
	struct cvmx_agl_prtx_ctl_cn61xx {
		u64 drv_byp : 1;
		u64 reserved_62_62 : 1;
		u64 cmp_pctl : 6;
		u64 reserved_54_55 : 2;
		u64 cmp_nctl : 6;
		u64 reserved_46_47 : 2;
		u64 drv_pctl : 6;
		u64 reserved_38_39 : 2;
		u64 drv_nctl : 6;
		u64 reserved_29_31 : 3;
		u64 clk_set : 5;
		u64 clkrx_byp : 1;
		u64 reserved_21_22 : 2;
		u64 clkrx_set : 5;
		u64 clktx_byp : 1;
		u64 reserved_13_14 : 2;
		u64 clktx_set : 5;
		u64 reserved_5_7 : 3;
		u64 dllrst : 1;
		u64 comp : 1;
		u64 enable : 1;
		u64 clkrst : 1;
		u64 mode : 1;
	} cn61xx;
	struct cvmx_agl_prtx_ctl_cn61xx cn63xx;
	struct cvmx_agl_prtx_ctl_cn61xx cn63xxp1;
	struct cvmx_agl_prtx_ctl_cn61xx cn66xx;
	struct cvmx_agl_prtx_ctl_cn61xx cn68xx;
	struct cvmx_agl_prtx_ctl_cn61xx cn68xxp1;
	struct cvmx_agl_prtx_ctl_cn70xx {
		u64 drv_byp : 1;
		u64 reserved_61_62 : 2;
		u64 cmp_pctl : 5;
		u64 reserved_53_55 : 3;
		u64 cmp_nctl : 5;
		u64 reserved_45_47 : 3;
		u64 drv_pctl : 5;
		u64 reserved_37_39 : 3;
		u64 drv_nctl : 5;
		u64 reserved_31_31 : 1;
		u64 clk_set : 7;
		u64 clkrx_byp : 1;
		u64 clkrx_set : 7;
		u64 clktx_byp : 1;
		u64 clktx_set : 7;
		u64 refclk_sel : 2;
		u64 reserved_5_5 : 1;
		u64 dllrst : 1;
		u64 comp : 1;
		u64 enable : 1;
		u64 clkrst : 1;
		u64 mode : 1;
	} cn70xx;
	struct cvmx_agl_prtx_ctl_cn70xx cn70xxp1;
};

typedef union cvmx_agl_prtx_ctl cvmx_agl_prtx_ctl_t;

#endif
