/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_CGX_H__
#define __CSRS_CGX_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * CGX.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration cgx_bar_e
 *
 * CGX Base Address Register Enumeration Enumerates the base address
 * registers.
 */
#define CGX_BAR_E_CGXX_PF_BAR0(a) (0x87e0e0000000ll + 0x1000000ll * (a))
#define CGX_BAR_E_CGXX_PF_BAR0_SIZE 0x100000ull
#define CGX_BAR_E_CGXX_PF_BAR4(a) (0x87e0e0400000ll + 0x1000000ll * (a))
#define CGX_BAR_E_CGXX_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration cgx_int_vec_e
 *
 * CGX MSI-X Vector Enumeration Enumeration the MSI-X interrupt vectors.
 */
#define CGX_INT_VEC_E_CMRX_INT(a) (0 + 9 * (a))
#define CGX_INT_VEC_E_CMRX_SW(a) (0x26 + (a))
#define CGX_INT_VEC_E_CMR_MEM_INT (0x24)
#define CGX_INT_VEC_E_GMPX_GMI_RX_INT(a) (5 + 9 * (a))
#define CGX_INT_VEC_E_GMPX_GMI_TX_INT(a) (6 + 9 * (a))
#define CGX_INT_VEC_E_GMPX_GMI_WOL_INT(a) (7 + 9 * (a))
#define CGX_INT_VEC_E_GMPX_PCS_INT(a) (4 + 9 * (a))
#define CGX_INT_VEC_E_SMUX_RX_INT(a) (2 + 9 * (a))
#define CGX_INT_VEC_E_SMUX_RX_WOL_INT(a) (8 + 9 * (a))
#define CGX_INT_VEC_E_SMUX_TX_INT(a) (3 + 9 * (a))
#define CGX_INT_VEC_E_SPUX_INT(a) (1 + 9 * (a))
#define CGX_INT_VEC_E_SW (0x25)

/**
 * Enumeration cgx_lmac_types_e
 *
 * CGX LMAC Type Enumeration Enumerates the LMAC Types that CGX supports.
 */
#define CGX_LMAC_TYPES_E_FIFTYG_R (8)
#define CGX_LMAC_TYPES_E_FORTYG_R (4)
#define CGX_LMAC_TYPES_E_HUNDREDG_R (9)
#define CGX_LMAC_TYPES_E_QSGMII (6)
#define CGX_LMAC_TYPES_E_RGMII (5)
#define CGX_LMAC_TYPES_E_RXAUI (2)
#define CGX_LMAC_TYPES_E_SGMII (0)
#define CGX_LMAC_TYPES_E_TENG_R (3)
#define CGX_LMAC_TYPES_E_TWENTYFIVEG_R (7)
#define CGX_LMAC_TYPES_E_USXGMII (0xa)
#define CGX_LMAC_TYPES_E_XAUI (1)

/**
 * Enumeration cgx_opcode_e
 *
 * INTERNAL: CGX Error Opcode Enumeration  Enumerates the error opcodes
 * created by CGX and presented to NCSI/NIX.
 */
#define CGX_OPCODE_E_RE_FCS (7)
#define CGX_OPCODE_E_RE_FCS_RCV (8)
#define CGX_OPCODE_E_RE_JABBER (2)
#define CGX_OPCODE_E_RE_NONE (0)
#define CGX_OPCODE_E_RE_PARTIAL (1)
#define CGX_OPCODE_E_RE_RX_CTL (0xb)
#define CGX_OPCODE_E_RE_SKIP (0xc)
#define CGX_OPCODE_E_RE_TERMINATE (9)

/**
 * Enumeration cgx_spu_br_train_cst_e
 *
 * INTERNAL: CGX Training Coefficient Status Enumeration  2-bit status
 * for each coefficient as defined in IEEE 802.3, Table 72-5.
 */
#define CGX_SPU_BR_TRAIN_CST_E_MAXIMUM (3)
#define CGX_SPU_BR_TRAIN_CST_E_MINIMUM (2)
#define CGX_SPU_BR_TRAIN_CST_E_NOT_UPDATED (0)
#define CGX_SPU_BR_TRAIN_CST_E_UPDATED (1)

/**
 * Enumeration cgx_spu_br_train_cup_e
 *
 * INTERNAL:CGX Training Coefficient Enumeration  2-bit command for each
 * coefficient as defined in IEEE 802.3, Table 72-4.
 */
#define CGX_SPU_BR_TRAIN_CUP_E_DECREMENT (1)
#define CGX_SPU_BR_TRAIN_CUP_E_HOLD (0)
#define CGX_SPU_BR_TRAIN_CUP_E_INCREMENT (2)
#define CGX_SPU_BR_TRAIN_CUP_E_RSV_CMD (3)

/**
 * Enumeration cgx_usxgmii_rate_e
 *
 * CGX USXGMII Rate Enumeration Enumerates the USXGMII sub-port type
 * rate, CGX()_SPU()_CONTROL1[USXGMII_RATE].  Selecting a rate higher
 * than the maximum allowed for a given port sub-type (specified by
 * CGX()_SPU()_CONTROL1[USXGMII_TYPE]), e.g., selecting ::RATE_2HG (2.5
 * Gbps) for CGX_USXGMII_TYPE_E::SXGMII_2G, will cause unpredictable
 * behavior. USXGMII hardware-based autonegotiation may change this
 * setting.
 */
#define CGX_USXGMII_RATE_E_RATE_100M (1)
#define CGX_USXGMII_RATE_E_RATE_10G (5)
#define CGX_USXGMII_RATE_E_RATE_10M (0)
#define CGX_USXGMII_RATE_E_RATE_1G (2)
#define CGX_USXGMII_RATE_E_RATE_20G (6)
#define CGX_USXGMII_RATE_E_RATE_2HG (3)
#define CGX_USXGMII_RATE_E_RATE_5G (4)
#define CGX_USXGMII_RATE_E_RSV_RATE (7)

/**
 * Enumeration cgx_usxgmii_type_e
 *
 * CGX USXGMII Port Sub-Type Enumeration Enumerates the USXGMII sub-port
 * type, CGX()_SPU()_CONTROL1[USXGMII_TYPE].  The description indicates
 * the maximum rate and the maximum number of ports (LMACs) for each sub-
 * type. The minimum rate for any port is 10M. The rate selection for
 * each LMAC is made using CGX()_SPU()_CONTROL1[USXGMII_RATE] and the
 * number of active ports/LMACs is implicitly determined by the value
 * given to CGX()_CMR()_CONFIG[ENABLE] for each LMAC.  Selecting a rate
 * higher than the maximum allowed for a given port sub-type or enabling
 * more LMACs than the maximum allowed for a given port sub-type will
 * cause unpredictable behavior.
 */
#define CGX_USXGMII_TYPE_E_DXGMII_10G (3)
#define CGX_USXGMII_TYPE_E_DXGMII_20G (5)
#define CGX_USXGMII_TYPE_E_DXGMII_5G (4)
#define CGX_USXGMII_TYPE_E_QXGMII_10G (7)
#define CGX_USXGMII_TYPE_E_QXGMII_20G (6)
#define CGX_USXGMII_TYPE_E_SXGMII_10G (0)
#define CGX_USXGMII_TYPE_E_SXGMII_2G (2)
#define CGX_USXGMII_TYPE_E_SXGMII_5G (1)

/**
 * Structure cgx_spu_br_lane_train_status_s
 *
 * INTERNAL:CGX Lane Training Status Structure  This is the group of lane
 * status bits for a single lane in the BASE-R PMD status register (MDIO
 * address 1.151) as defined in IEEE 802.3ba-2010, Table 45-55.
 */
union cgx_spu_br_lane_train_status_s {
	u32 u;
	struct cgx_spu_br_lane_train_status_s_s {
		u32 rx_trained                       : 1;
		u32 frame_lock                       : 1;
		u32 training                         : 1;
		u32 training_failure                 : 1;
		u32 reserved_4_31                    : 28;
	} s;
	/* struct cgx_spu_br_lane_train_status_s_s cn; */
};

/**
 * Structure cgx_spu_br_train_cup_s
 *
 * INTERNAL:CGX Lane Training Coefficient Structure  This is the
 * coefficient update field of the BASE-R link training packet as defined
 * in IEEE 802.3, Table 72-4.
 */
union cgx_spu_br_train_cup_s {
	u32 u;
	struct cgx_spu_br_train_cup_s_s {
		u32 pre_cup                          : 2;
		u32 main_cup                         : 2;
		u32 post_cup                         : 2;
		u32 reserved_6_11                    : 6;
		u32 init                             : 1;
		u32 preset                           : 1;
		u32 reserved_14_31                   : 18;
	} s;
	struct cgx_spu_br_train_cup_s_cn {
		u32 pre_cup                          : 2;
		u32 main_cup                         : 2;
		u32 post_cup                         : 2;
		u32 reserved_6_11                    : 6;
		u32 init                             : 1;
		u32 preset                           : 1;
		u32 reserved_14_15                   : 2;
		u32 reserved_16_31                   : 16;
	} cn;
};

/**
 * Structure cgx_spu_br_train_rep_s
 *
 * INTERNAL:CGX Training Report Structure  This is the status report
 * field of the BASE-R link training packet as defined in IEEE 802.3,
 * Table 72-5.
 */
union cgx_spu_br_train_rep_s {
	u32 u;
	struct cgx_spu_br_train_rep_s_s {
		u32 pre_cst                          : 2;
		u32 main_cst                         : 2;
		u32 post_cst                         : 2;
		u32 reserved_6_14                    : 9;
		u32 rx_ready                         : 1;
		u32 reserved_16_31                   : 16;
	} s;
	/* struct cgx_spu_br_train_rep_s_s cn; */
};

/**
 * Structure cgx_spu_sds_cu_s
 *
 * INTERNAL: CGX Training Coeffiecient Structure  This structure is
 * similar to CGX_SPU_BR_TRAIN_CUP_S format, but with reserved fields
 * removed and [RCVR_READY] field added.
 */
union cgx_spu_sds_cu_s {
	u32 u;
	struct cgx_spu_sds_cu_s_s {
		u32 pre_cu                           : 2;
		u32 main_cu                          : 2;
		u32 post_cu                          : 2;
		u32 initialize                       : 1;
		u32 preset                           : 1;
		u32 rcvr_ready                       : 1;
		u32 reserved_9_31                    : 23;
	} s;
	/* struct cgx_spu_sds_cu_s_s cn; */
};

/**
 * Structure cgx_spu_sds_skew_status_s
 *
 * CGX Skew Status Structure Provides receive skew information detected
 * for a physical SerDes lane when it is assigned to a multilane
 * LMAC/LPCS. Contents are valid when RX deskew is done for the
 * associated LMAC/LPCS.
 */
union cgx_spu_sds_skew_status_s {
	u32 u;
	struct cgx_spu_sds_skew_status_s_s {
		u32 am_timestamp                     : 12;
		u32 reserved_12_15                   : 4;
		u32 am_lane_id                       : 5;
		u32 reserved_21_22                   : 2;
		u32 lane_skew                        : 7;
		u32 reserved_30_31                   : 2;
	} s;
	/* struct cgx_spu_sds_skew_status_s_s cn; */
};

/**
 * Structure cgx_spu_sds_sr_s
 *
 * INTERNAL: CGX Lane Training Coefficient Structure  Similar to
 * CGX_SPU_BR_TRAIN_REP_S format, but with reserved and RX ready fields
 * removed.
 */
union cgx_spu_sds_sr_s {
	u32 u;
	struct cgx_spu_sds_sr_s_s {
		u32 pre_status                       : 2;
		u32 main_status                      : 2;
		u32 post_status                      : 2;
		u32 reserved_6_31                    : 26;
	} s;
	/* struct cgx_spu_sds_sr_s_s cn; */
};

/**
 * Register (RSL) cgx#_active_pc
 *
 * CGX ACTIVE PC Register This register counts the conditional clocks for
 * power management.
 */
union cgxx_active_pc {
	u64 u;
	struct cgxx_active_pc_s {
		u64 cnt                              : 64;
	} s;
	/* struct cgxx_active_pc_s cn; */
};

static inline u64 CGXX_ACTIVE_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_ACTIVE_PC(void)
{
	return 0x2010;
}

/**
 * Register (RSL) cgx#_cmr#_activity
 *
 * CGX CMR Activity Registers
 */
union cgxx_cmrx_activity {
	u64 u;
	struct cgxx_cmrx_activity_s {
		u64 act_tx_lo                        : 1;
		u64 act_tx_hi                        : 1;
		u64 pause_tx                         : 1;
		u64 act_rx_lo                        : 1;
		u64 act_rx_hi                        : 1;
		u64 pause_rx                         : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_cmrx_activity_s cn; */
};

static inline u64 CGXX_CMRX_ACTIVITY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_ACTIVITY(u64 a)
{
	return 0x5f8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_config
 *
 * CGX CMR Configuration Registers Logical MAC/PCS configuration
 * registers; one per LMAC. The maximum number of LMACs (and maximum LMAC
 * ID) that can be enabled by these registers is limited by
 * CGX()_CMR_RX_LMACS[LMACS] and CGX()_CMR_TX_LMACS[LMACS].  Internal:
 * \<pre\> Example configurations:   ------------------------------------
 * ---------------------------------------   Configuration
 * LMACS  Register             [ENABLE]    [LMAC_TYPE]   ----------------
 * -----------------------------------------------------------
 * 1x50G+1x25G+1xSGMII     4      CGXn_CMR0_CONFIG     1           8
 * CGXn_CMR1_CONFIG     0           --
 * CGXn_CMR2_CONFIG     1           7
 * CGXn_CMR3_CONFIG     1           0   ---------------------------------
 * ------------------------------------------   USXGMII
 * 1-4    CGXn_CMR0_CONFIG     1           a
 * CGXn_CMR1_CONFIG     1           a
 * CGXn_CMR2_CONFIG     1           a
 * CGXn_CMR3_CONFIG     1           a   ---------------------------------
 * ------------------------------------------   1x100GBASE-R4           1
 * CGXn_CMR0_CONFIG     1           9
 * CGXn_CMR1_CONFIG     0           --
 * CGXn_CMR2_CONFIG     0           --
 * CGXn_CMR3_CONFIG     0           --   --------------------------------
 * -------------------------------------------   2x50GBASE-R2
 * 2      CGXn_CMR0_CONFIG     1           8
 * CGXn_CMR1_CONFIG     1           8
 * CGXn_CMR2_CONFIG     0           --
 * CGXn_CMR3_CONFIG     0           --   --------------------------------
 * -------------------------------------------   4x25GBASE-R
 * 4      CGXn_CMR0_CONFIG     1           7
 * CGXn_CMR1_CONFIG     1           7
 * CGXn_CMR2_CONFIG     1           7
 * CGXn_CMR3_CONFIG     1           7   ---------------------------------
 * ------------------------------------------   QSGMII                  4
 * CGXn_CMR0_CONFIG     1           6
 * CGXn_CMR1_CONFIG     1           6
 * CGXn_CMR2_CONFIG     1           6
 * CGXn_CMR3_CONFIG     1           6   ---------------------------------
 * ------------------------------------------   1x40GBASE-R4            1
 * CGXn_CMR0_CONFIG     1           4
 * CGXn_CMR1_CONFIG     0           --
 * CGXn_CMR2_CONFIG     0           --
 * CGXn_CMR3_CONFIG     0           --   --------------------------------
 * -------------------------------------------   4x10GBASE-R
 * 4      CGXn_CMR0_CONFIG     1           3
 * CGXn_CMR1_CONFIG     1           3
 * CGXn_CMR2_CONFIG     1           3
 * CGXn_CMR3_CONFIG     1           3   ---------------------------------
 * ------------------------------------------   2xRXAUI                 2
 * CGXn_CMR0_CONFIG     1           2
 * CGXn_CMR1_CONFIG     1           2
 * CGXn_CMR2_CONFIG     0           --
 * CGXn_CMR3_CONFIG     0           --   --------------------------------
 * -------------------------------------------   1x10GBASE-X/XAUI/DXAUI
 * 1      CGXn_CMR0_CONFIG     1           1
 * CGXn_CMR1_CONFIG     0           --
 * CGXn_CMR2_CONFIG     0           --
 * CGXn_CMR3_CONFIG     0           --   --------------------------------
 * -------------------------------------------   4xSGMII/1000BASE-X
 * 4      CGXn_CMR0_CONFIG     1           0
 * CGXn_CMR1_CONFIG     1           0
 * CGXn_CMR2_CONFIG     1           0
 * CGXn_CMR3_CONFIG     1           0   ---------------------------------
 * ------------------------------------------ \</pre\>
 */
union cgxx_cmrx_config {
	u64 u;
	struct cgxx_cmrx_config_s {
		u64 lane_to_sds                      : 8;
		u64 reserved_8_39                    : 32;
		u64 lmac_type                        : 4;
		u64 unused                           : 8;
		u64 int_beat_gen                     : 1;
		u64 data_pkt_tx_en                   : 1;
		u64 data_pkt_rx_en                   : 1;
		u64 enable                           : 1;
		u64 x2p_select                       : 3;
		u64 p2x_select                       : 3;
		u64 reserved_62_63                   : 2;
	} s;
	/* struct cgxx_cmrx_config_s cn; */
};

static inline u64 CGXX_CMRX_CONFIG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_CONFIG(u64 a)
{
	return 0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_int
 *
 * CGX CMR Interrupt Register
 */
union cgxx_cmrx_int {
	u64 u;
	struct cgxx_cmrx_int_s {
		u64 pause_drp                        : 1;
		u64 overflw                          : 1;
		u64 nic_nxc                          : 1;
		u64 nix0_nxc                         : 1;
		u64 nix1_nxc                         : 1;
		u64 nix0_e_nxc                       : 1;
		u64 nix1_e_nxc                       : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_int_s cn; */
};

static inline u64 CGXX_CMRX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_INT(u64 a)
{
	return 0x40 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_int_ena_w1c
 *
 * CGX CMR Interrupt Enable Clear Register This register clears interrupt
 * enable bits.
 */
union cgxx_cmrx_int_ena_w1c {
	u64 u;
	struct cgxx_cmrx_int_ena_w1c_s {
		u64 pause_drp                        : 1;
		u64 overflw                          : 1;
		u64 nic_nxc                          : 1;
		u64 nix0_nxc                         : 1;
		u64 nix1_nxc                         : 1;
		u64 nix0_e_nxc                       : 1;
		u64 nix1_e_nxc                       : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_int_ena_w1c_s cn; */
};

static inline u64 CGXX_CMRX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_INT_ENA_W1C(u64 a)
{
	return 0x50 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_int_ena_w1s
 *
 * CGX CMR Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union cgxx_cmrx_int_ena_w1s {
	u64 u;
	struct cgxx_cmrx_int_ena_w1s_s {
		u64 pause_drp                        : 1;
		u64 overflw                          : 1;
		u64 nic_nxc                          : 1;
		u64 nix0_nxc                         : 1;
		u64 nix1_nxc                         : 1;
		u64 nix0_e_nxc                       : 1;
		u64 nix1_e_nxc                       : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_int_ena_w1s_s cn; */
};

static inline u64 CGXX_CMRX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_INT_ENA_W1S(u64 a)
{
	return 0x58 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_int_w1s
 *
 * CGX CMR Interrupt Set Register This register sets interrupt bits.
 */
union cgxx_cmrx_int_w1s {
	u64 u;
	struct cgxx_cmrx_int_w1s_s {
		u64 pause_drp                        : 1;
		u64 overflw                          : 1;
		u64 nic_nxc                          : 1;
		u64 nix0_nxc                         : 1;
		u64 nix1_nxc                         : 1;
		u64 nix0_e_nxc                       : 1;
		u64 nix1_e_nxc                       : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_int_w1s_s cn; */
};

static inline u64 CGXX_CMRX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_INT_W1S(u64 a)
{
	return 0x48 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_led_timing
 *
 * CGX MAC LED Activity Timing Registers
 */
union cgxx_cmrx_led_timing {
	u64 u;
	struct cgxx_cmrx_led_timing_s {
		u64 extension                        : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct cgxx_cmrx_led_timing_s cn; */
};

static inline u64 CGXX_CMRX_LED_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_LED_TIMING(u64 a)
{
	return 0x5f0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_prt_cbfc_ctl
 *
 * CGX CMR LMAC PFC Control Registers See CGX()_CMR()_RX_LOGL_XOFF[XOFF].
 */
union cgxx_cmrx_prt_cbfc_ctl {
	u64 u;
	struct cgxx_cmrx_prt_cbfc_ctl_s {
		u64 reserved_0_15                    : 16;
		u64 phys_bp                          : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_cmrx_prt_cbfc_ctl_s cn; */
};

static inline u64 CGXX_CMRX_PRT_CBFC_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_PRT_CBFC_CTL(u64 a)
{
	return 0x608 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_bp_drop
 *
 * CGX Receive Backpressure Drop Register
 */
union cgxx_cmrx_rx_bp_drop {
	u64 u;
	struct cgxx_cmrx_rx_bp_drop_s {
		u64 mark                             : 7;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_rx_bp_drop_s cn; */
};

static inline u64 CGXX_CMRX_RX_BP_DROP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_BP_DROP(u64 a)
{
	return 0xd8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_bp_off
 *
 * CGX Receive Backpressure Off Register
 */
union cgxx_cmrx_rx_bp_off {
	u64 u;
	struct cgxx_cmrx_rx_bp_off_s {
		u64 mark                             : 7;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_cmrx_rx_bp_off_s cn; */
};

static inline u64 CGXX_CMRX_RX_BP_OFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_BP_OFF(u64 a)
{
	return 0xe8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_bp_on
 *
 * CGX Receive Backpressure On Register
 */
union cgxx_cmrx_rx_bp_on {
	u64 u;
	struct cgxx_cmrx_rx_bp_on_s {
		u64 mark                             : 13;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct cgxx_cmrx_rx_bp_on_s cn; */
};

static inline u64 CGXX_CMRX_RX_BP_ON(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_BP_ON(u64 a)
{
	return 0xe0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_bp_status
 *
 * CGX CMR Receive Backpressure Status Registers
 */
union cgxx_cmrx_rx_bp_status {
	u64 u;
	struct cgxx_cmrx_rx_bp_status_s {
		u64 bp                               : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmrx_rx_bp_status_s cn; */
};

static inline u64 CGXX_CMRX_RX_BP_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_BP_STATUS(u64 a)
{
	return 0xf0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_dmac_ctl0
 *
 * CGX CMR Receive DMAC Address-Control0 Register DMAC CAM control
 * register for use by X2P/NIX bound traffic. Received packets are only
 * passed to X2P/NIX when the DMAC0 filter result is ACCEPT and STEERING0
 * filter result is PASS. See also CGX()_CMR_RX_DMAC()_CAM0 and
 * CGX()_CMR_RX_STEERING0().  Internal: "* ALGORITHM Here is some pseudo
 * code that represents the address filter behavior. \<pre\>
 * dmac_addr_filter(uint8 prt, uint48 dmac) { for (lmac=0, lmac\<4,
 * lmac++) {   if (is_bcst(dmac))                               //
 * broadcast accept     return (CGX()_CMR(lmac)_RX_DMAC_CTL0[BCST_ACCEPT]
 * ? ACCEPT : REJECT);   if (is_mcst(dmac) &&
 * CGX()_CMR(lmac)_RX_DMAC_CTL0[MCST_MODE] == 0)   // multicast reject
 * return REJECT;   if (is_mcst(dmac) &&
 * CGX()_CMR(lmac)_RX_DMAC_CTL0[MCST_MODE] == 1)   // multicast accept
 * return ACCEPT;   else        // DMAC CAM filter     cam_hit = 0;   for
 * (i=0; i\<32; i++) {     cam = CGX()_CMR_RX_DMAC(i)_CAM0;     if
 * (cam[EN] && cam[ID] == lmac && cam[ADR] == dmac) {       cam_hit = 1;
 * break;     }   }   if (cam_hit) {     return
 * (CGX()_CMR(lmac)_RX_DMAC_CTL0[CAM_ACCEPT] ? ACCEPT : REJECT);   else
 * return (CGX()_CMR(lmac)_RX_DMAC_CTL0[CAM_ACCEPT] ? REJECT : ACCEPT);
 * } } \</pre\>"
 */
union cgxx_cmrx_rx_dmac_ctl0 {
	u64 u;
	struct cgxx_cmrx_rx_dmac_ctl0_s {
		u64 bcst_accept                      : 1;
		u64 mcst_mode                        : 2;
		u64 cam_accept                       : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_cmrx_rx_dmac_ctl0_s cn; */
};

static inline u64 CGXX_CMRX_RX_DMAC_CTL0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_DMAC_CTL0(u64 a)
{
	return 0x1f8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_dmac_ctl1
 *
 * CGX CMR Receive DMAC Address-Control1 Register DMAC CAM control
 * register for use by NCSI bound traffic. Received packets are only
 * passed to NCSI when the DMAC1 filter result is ACCEPT and STEERING1
 * filter result is PASS. See also CGX()_CMR_RX_DMAC()_CAM1 and
 * CGX()_CMR_RX_STEERING1(). For use with the LMAC associated with NCSI;
 * see CGX()_CMR_GLOBAL_CONFIG[NCSI_LMAC_ID].  Internal: ALGORITHM: See
 * CGX()_CMR()_RX_DMAC_CTL0.
 */
union cgxx_cmrx_rx_dmac_ctl1 {
	u64 u;
	struct cgxx_cmrx_rx_dmac_ctl1_s {
		u64 bcst_accept                      : 1;
		u64 mcst_mode                        : 2;
		u64 cam_accept                       : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_cmrx_rx_dmac_ctl1_s cn; */
};

static inline u64 CGXX_CMRX_RX_DMAC_CTL1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_DMAC_CTL1(u64 a)
{
	return 0x3f8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_fifo_len
 *
 * CGX CMR Receive Fifo Length Registers
 */
union cgxx_cmrx_rx_fifo_len {
	u64 u;
	struct cgxx_cmrx_rx_fifo_len_s {
		u64 fifo_len                         : 14;
		u64 busy                             : 1;
		u64 fifo_len_e                       : 14;
		u64 busy_e                           : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct cgxx_cmrx_rx_fifo_len_s cn; */
};

static inline u64 CGXX_CMRX_RX_FIFO_LEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_FIFO_LEN(u64 a)
{
	return 0x108 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_id_map
 *
 * CGX CMR Receive ID Map Register These registers set the RX LMAC ID
 * mapping for X2P/NIX.
 */
union cgxx_cmrx_rx_id_map {
	u64 u;
	struct cgxx_cmrx_rx_id_map_s {
		u64 pknd                             : 6;
		u64 unused                           : 2;
		u64 rid                              : 7;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct cgxx_cmrx_rx_id_map_s cn; */
};

static inline u64 CGXX_CMRX_RX_ID_MAP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_ID_MAP(u64 a)
{
	return 0x60 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_logl_xoff
 *
 * CGX CMR Receive Logical XOFF Registers
 */
union cgxx_cmrx_rx_logl_xoff {
	u64 u;
	struct cgxx_cmrx_rx_logl_xoff_s {
		u64 xoff                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_cmrx_rx_logl_xoff_s cn; */
};

static inline u64 CGXX_CMRX_RX_LOGL_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_LOGL_XOFF(u64 a)
{
	return 0xf8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_logl_xon
 *
 * CGX CMR Receive Logical XON Registers
 */
union cgxx_cmrx_rx_logl_xon {
	u64 u;
	struct cgxx_cmrx_rx_logl_xon_s {
		u64 xon                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_cmrx_rx_logl_xon_s cn; */
};

static inline u64 CGXX_CMRX_RX_LOGL_XON(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_LOGL_XON(u64 a)
{
	return 0x100 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_merge_stat0
 *
 * CGX RX Preemption Status Register 0
 */
union cgxx_cmrx_rx_merge_stat0 {
	u64 u;
	struct cgxx_cmrx_rx_merge_stat0_s {
		u64 fa_err_cnt                       : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_merge_stat0_s cn; */
};

static inline u64 CGXX_CMRX_RX_MERGE_STAT0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_MERGE_STAT0(u64 a)
{
	return 0x138 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_merge_stat1
 *
 * CGX RX Preemption Status Register 1
 */
union cgxx_cmrx_rx_merge_stat1 {
	u64 u;
	struct cgxx_cmrx_rx_merge_stat1_s {
		u64 fs_err_cnt                       : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_merge_stat1_s cn; */
};

static inline u64 CGXX_CMRX_RX_MERGE_STAT1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_MERGE_STAT1(u64 a)
{
	return 0x140 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_merge_stat2
 *
 * CGX RX Preemption Status Register 2
 */
union cgxx_cmrx_rx_merge_stat2 {
	u64 u;
	struct cgxx_cmrx_rx_merge_stat2_s {
		u64 fa_ok_cnt                        : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_merge_stat2_s cn; */
};

static inline u64 CGXX_CMRX_RX_MERGE_STAT2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_MERGE_STAT2(u64 a)
{
	return 0x148 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_merge_stat3
 *
 * CGX RX Preemption Status Register 3
 */
union cgxx_cmrx_rx_merge_stat3 {
	u64 u;
	struct cgxx_cmrx_rx_merge_stat3_s {
		u64 ff_cnt                           : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_merge_stat3_s cn; */
};

static inline u64 CGXX_CMRX_RX_MERGE_STAT3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_MERGE_STAT3(u64 a)
{
	return 0x150 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_merge_stat4
 *
 * CGX RX Preemption Status Register 4
 */
union cgxx_cmrx_rx_merge_stat4 {
	u64 u;
	struct cgxx_cmrx_rx_merge_stat4_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_merge_stat4_s cn; */
};

static inline u64 CGXX_CMRX_RX_MERGE_STAT4(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_MERGE_STAT4(u64 a)
{
	return 0x158 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_pause_drop_time
 *
 * CGX CMR Receive Pause Drop-Time Register
 */
union cgxx_cmrx_rx_pause_drop_time {
	u64 u;
	struct cgxx_cmrx_rx_pause_drop_time_s {
		u64 pause_time                       : 16;
		u64 pause_time_e                     : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_cmrx_rx_pause_drop_time_s cn; */
};

static inline u64 CGXX_CMRX_RX_PAUSE_DROP_TIME(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_PAUSE_DROP_TIME(u64 a)
{
	return 0x68 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat0
 *
 * CGX Receive Status Register 0 These registers provide a count of
 * received packets that meet the following conditions: * are not
 * recognized as ERROR packets(any OPCODE). * are not recognized as PAUSE
 * packets. * are not dropped due FIFO full status. * are not dropped due
 * DMAC0 or STEERING0 filtering.  Internal: "This pseudo code represents
 * the RX STAT0 through STAT8 accounting: \<pre\> If (errored)   incr
 * RX_STAT8 else if (ctrl packet, i.e. Pause/PFC)   incr RX_STAT2,3 else
 * if (fifo full drop)   incr RX_STAT6,7 else if (DMAC0/VLAN0 filter
 * drop)   incr RX_STAT4,5 if not a filter+decision else   incr
 * RX_STAT0,1 end \</pre\>"
 */
union cgxx_cmrx_rx_stat0 {
	u64 u;
	struct cgxx_cmrx_rx_stat0_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat0_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT0(u64 a)
{
	return 0x70 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat1
 *
 * CGX Receive Status Register 1 These registers provide a count of
 * octets of received packets.
 */
union cgxx_cmrx_rx_stat1 {
	u64 u;
	struct cgxx_cmrx_rx_stat1_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat1_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT1(u64 a)
{
	return 0x78 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat2
 *
 * CGX Receive Status Register 2 These registers provide a count of
 * received packets that meet the following conditions: * are not
 * recognized as ERROR packets(any OPCODE). * are recognized as PAUSE
 * packets.  Pause packets can be optionally dropped or forwarded based
 * on
 * CGX()_SMU()_RX_FRM_CTL[CTL_DRP]/CGX()_GMP_GMI_RX()_FRM_CTL[CTL_DRP].
 * This count increments regardless of whether the packet is dropped.
 */
union cgxx_cmrx_rx_stat2 {
	u64 u;
	struct cgxx_cmrx_rx_stat2_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat2_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT2(u64 a)
{
	return 0x80 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat3
 *
 * CGX Receive Status Register 3 These registers provide a count of
 * octets of received PAUSE and control packets.
 */
union cgxx_cmrx_rx_stat3 {
	u64 u;
	struct cgxx_cmrx_rx_stat3_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat3_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT3(u64 a)
{
	return 0x88 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat4
 *
 * CGX Receive Status Register 4 These registers provide a count of
 * received packets that meet the following conditions: * are not
 * recognized as ERROR packets(any OPCODE). * are not recognized as PAUSE
 * packets. * are not dropped due FIFO full status. * are dropped due
 * DMAC0 or STEERING0 filtering.  16B packets or smaller (20B in case of
 * FCS strip) as the result of truncation or other means are not dropped
 * by CGX (unless filter and decision is also asserted) and will never
 * appear in this count. Should the MAC signal to the CMR that the packet
 * be filtered upon decision before the end of packet, then STAT4 and
 * STAT5 will not be updated.
 */
union cgxx_cmrx_rx_stat4 {
	u64 u;
	struct cgxx_cmrx_rx_stat4_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat4_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT4(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT4(u64 a)
{
	return 0x90 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat5
 *
 * CGX Receive Status Register 5 These registers provide a count of
 * octets of filtered DMAC0 or VLAN STEERING0 packets.
 */
union cgxx_cmrx_rx_stat5 {
	u64 u;
	struct cgxx_cmrx_rx_stat5_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat5_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT5(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT5(u64 a)
{
	return 0x98 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat6
 *
 * CGX Receive Status Register 6 These registers provide a count of
 * received packets that meet the following conditions: * are not
 * recognized as ERROR packets(any OPCODE). * are not recognized as PAUSE
 * packets. * are dropped due FIFO full status.  They do not count any
 * packet that is truncated at the point of overflow and sent on to the
 * NIX. The truncated packet will be marked with error and increment
 * STAT8. These registers count all entire packets dropped by the FIFO
 * for a given LMAC.
 */
union cgxx_cmrx_rx_stat6 {
	u64 u;
	struct cgxx_cmrx_rx_stat6_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat6_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT6(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT6(u64 a)
{
	return 0xa0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat7
 *
 * CGX Receive Status Register 7 These registers provide a count of
 * octets of received packets that were dropped due to a full receive
 * FIFO.
 */
union cgxx_cmrx_rx_stat7 {
	u64 u;
	struct cgxx_cmrx_rx_stat7_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat7_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT7(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT7(u64 a)
{
	return 0xa8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat8
 *
 * CGX Receive Status Register 8 These registers provide a count of
 * received packets that meet the following conditions:  * are recognized
 * as ERROR packets(any OPCODE).
 */
union cgxx_cmrx_rx_stat8 {
	u64 u;
	struct cgxx_cmrx_rx_stat8_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat8_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT8(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT8(u64 a)
{
	return 0xb0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_rx_stat_pri#_xoff
 *
 * CGX CMR RX XON to XOFF transition Registers
 */
union cgxx_cmrx_rx_stat_prix_xoff {
	u64 u;
	struct cgxx_cmrx_rx_stat_prix_xoff_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_rx_stat_prix_xoff_s cn; */
};

static inline u64 CGXX_CMRX_RX_STAT_PRIX_XOFF(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_RX_STAT_PRIX_XOFF(u64 a, u64 b)
{
	return 0x7c0 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_cmr#_scratch#
 *
 * CGX CMR Scratch Registers
 */
union cgxx_cmrx_scratchx {
	u64 u;
	struct cgxx_cmrx_scratchx_s {
		u64 scratch                          : 64;
	} s;
	/* struct cgxx_cmrx_scratchx_s cn; */
};

static inline u64 CGXX_CMRX_SCRATCHX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_SCRATCHX(u64 a, u64 b)
{
	return 0x1050 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_cmr#_sw_int
 *
 * CGX CMR Interrupt Register
 */
union cgxx_cmrx_sw_int {
	u64 u;
	struct cgxx_cmrx_sw_int_s {
		u64 sw_set                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmrx_sw_int_s cn; */
};

static inline u64 CGXX_CMRX_SW_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_SW_INT(u64 a)
{
	return 0x180 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_sw_int_ena_w1c
 *
 * CGX CMR Interrupt Enable Clear Register This register clears interrupt
 * enable bits.
 */
union cgxx_cmrx_sw_int_ena_w1c {
	u64 u;
	struct cgxx_cmrx_sw_int_ena_w1c_s {
		u64 sw_set                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmrx_sw_int_ena_w1c_s cn; */
};

static inline u64 CGXX_CMRX_SW_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_SW_INT_ENA_W1C(u64 a)
{
	return 0x190 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_sw_int_ena_w1s
 *
 * CGX CMR Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union cgxx_cmrx_sw_int_ena_w1s {
	u64 u;
	struct cgxx_cmrx_sw_int_ena_w1s_s {
		u64 sw_set                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmrx_sw_int_ena_w1s_s cn; */
};

static inline u64 CGXX_CMRX_SW_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_SW_INT_ENA_W1S(u64 a)
{
	return 0x198 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_sw_int_w1s
 *
 * CGX CMR Interrupt Set Register This register sets interrupt bits.
 */
union cgxx_cmrx_sw_int_w1s {
	u64 u;
	struct cgxx_cmrx_sw_int_w1s_s {
		u64 sw_set                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmrx_sw_int_w1s_s cn; */
};

static inline u64 CGXX_CMRX_SW_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_SW_INT_W1S(u64 a)
{
	return 0x188 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_channel
 *
 * CGX CMR Transmit-Channels Registers
 */
union cgxx_cmrx_tx_channel {
	u64 u;
	struct cgxx_cmrx_tx_channel_s {
		u64 msk                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_cmrx_tx_channel_s cn; */
};

static inline u64 CGXX_CMRX_TX_CHANNEL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_CHANNEL(u64 a)
{
	return 0x600 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_fifo_len
 *
 * CGX CMR Transmit Fifo Length Registers
 */
union cgxx_cmrx_tx_fifo_len {
	u64 u;
	struct cgxx_cmrx_tx_fifo_len_s {
		u64 fifo_len                         : 14;
		u64 lmac_idle                        : 1;
		u64 fifo_e_len                       : 14;
		u64 lmac_e_idle                      : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct cgxx_cmrx_tx_fifo_len_s cn; */
};

static inline u64 CGXX_CMRX_TX_FIFO_LEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_FIFO_LEN(u64 a)
{
	return 0x618 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_hg2_status
 *
 * CGX CMR Transmit HiGig2 Status Registers
 */
union cgxx_cmrx_tx_hg2_status {
	u64 u;
	struct cgxx_cmrx_tx_hg2_status_s {
		u64 lgtim2go                         : 16;
		u64 xof                              : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_cmrx_tx_hg2_status_s cn; */
};

static inline u64 CGXX_CMRX_TX_HG2_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_HG2_STATUS(u64 a)
{
	return 0x610 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_merge_stat0
 *
 * CGX TX Preemption Status Register 0
 */
union cgxx_cmrx_tx_merge_stat0 {
	u64 u;
	struct cgxx_cmrx_tx_merge_stat0_s {
		u64 ff_cnt                           : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_merge_stat0_s cn; */
};

static inline u64 CGXX_CMRX_TX_MERGE_STAT0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_MERGE_STAT0(u64 a)
{
	return 0x160 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_ovr_bp
 *
 * CGX CMR Transmit-Channels Backpressure Override Registers
 */
union cgxx_cmrx_tx_ovr_bp {
	u64 u;
	struct cgxx_cmrx_tx_ovr_bp_s {
		u64 tx_chan_bp                       : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_cmrx_tx_ovr_bp_s cn; */
};

static inline u64 CGXX_CMRX_TX_OVR_BP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_OVR_BP(u64 a)
{
	return 0x620 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat0
 *
 * CGX CMR Transmit Statistics Registers 0
 */
union cgxx_cmrx_tx_stat0 {
	u64 u;
	struct cgxx_cmrx_tx_stat0_s {
		u64 xscol                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat0_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT0(u64 a)
{
	return 0x700 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat1
 *
 * CGX CMR Transmit Statistics Registers 1
 */
union cgxx_cmrx_tx_stat1 {
	u64 u;
	struct cgxx_cmrx_tx_stat1_s {
		u64 xsdef                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat1_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT1(u64 a)
{
	return 0x708 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat10
 *
 * CGX CMR Transmit Statistics Registers 10
 */
union cgxx_cmrx_tx_stat10 {
	u64 u;
	struct cgxx_cmrx_tx_stat10_s {
		u64 hist4                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat10_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT10(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT10(u64 a)
{
	return 0x750 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat11
 *
 * CGX CMR Transmit Statistics Registers 11
 */
union cgxx_cmrx_tx_stat11 {
	u64 u;
	struct cgxx_cmrx_tx_stat11_s {
		u64 hist5                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat11_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT11(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT11(u64 a)
{
	return 0x758 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat12
 *
 * CGX CMR Transmit Statistics Registers 12
 */
union cgxx_cmrx_tx_stat12 {
	u64 u;
	struct cgxx_cmrx_tx_stat12_s {
		u64 hist6                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat12_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT12(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT12(u64 a)
{
	return 0x760 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat13
 *
 * CGX CMR Transmit Statistics Registers 13
 */
union cgxx_cmrx_tx_stat13 {
	u64 u;
	struct cgxx_cmrx_tx_stat13_s {
		u64 hist7                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat13_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT13(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT13(u64 a)
{
	return 0x768 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat14
 *
 * CGX CMR Transmit Statistics Registers 14
 */
union cgxx_cmrx_tx_stat14 {
	u64 u;
	struct cgxx_cmrx_tx_stat14_s {
		u64 bcst                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat14_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT14(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT14(u64 a)
{
	return 0x770 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat15
 *
 * CGX CMR Transmit Statistics Registers 15
 */
union cgxx_cmrx_tx_stat15 {
	u64 u;
	struct cgxx_cmrx_tx_stat15_s {
		u64 mcst                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat15_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT15(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT15(u64 a)
{
	return 0x778 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat16
 *
 * CGX CMR Transmit Statistics Registers 16
 */
union cgxx_cmrx_tx_stat16 {
	u64 u;
	struct cgxx_cmrx_tx_stat16_s {
		u64 undflw                           : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat16_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT16(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT16(u64 a)
{
	return 0x780 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat17
 *
 * CGX CMR Transmit Statistics Registers 17
 */
union cgxx_cmrx_tx_stat17 {
	u64 u;
	struct cgxx_cmrx_tx_stat17_s {
		u64 ctl                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat17_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT17(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT17(u64 a)
{
	return 0x788 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat2
 *
 * CGX CMR Transmit Statistics Registers 2
 */
union cgxx_cmrx_tx_stat2 {
	u64 u;
	struct cgxx_cmrx_tx_stat2_s {
		u64 mcol                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat2_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT2(u64 a)
{
	return 0x710 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat3
 *
 * CGX CMR Transmit Statistics Registers 3
 */
union cgxx_cmrx_tx_stat3 {
	u64 u;
	struct cgxx_cmrx_tx_stat3_s {
		u64 scol                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat3_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT3(u64 a)
{
	return 0x718 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat4
 *
 * CGX CMR Transmit Statistics Registers 4
 */
union cgxx_cmrx_tx_stat4 {
	u64 u;
	struct cgxx_cmrx_tx_stat4_s {
		u64 octs                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat4_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT4(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT4(u64 a)
{
	return 0x720 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat5
 *
 * CGX CMR Transmit Statistics Registers 5
 */
union cgxx_cmrx_tx_stat5 {
	u64 u;
	struct cgxx_cmrx_tx_stat5_s {
		u64 pkts                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat5_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT5(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT5(u64 a)
{
	return 0x728 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat6
 *
 * CGX CMR Transmit Statistics Registers 6
 */
union cgxx_cmrx_tx_stat6 {
	u64 u;
	struct cgxx_cmrx_tx_stat6_s {
		u64 hist0                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat6_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT6(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT6(u64 a)
{
	return 0x730 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat7
 *
 * CGX CMR Transmit Statistics Registers 7
 */
union cgxx_cmrx_tx_stat7 {
	u64 u;
	struct cgxx_cmrx_tx_stat7_s {
		u64 hist1                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat7_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT7(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT7(u64 a)
{
	return 0x738 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat8
 *
 * CGX CMR Transmit Statistics Registers 8
 */
union cgxx_cmrx_tx_stat8 {
	u64 u;
	struct cgxx_cmrx_tx_stat8_s {
		u64 hist2                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat8_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT8(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT8(u64 a)
{
	return 0x740 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat9
 *
 * CGX CMR Transmit Statistics Registers 9
 */
union cgxx_cmrx_tx_stat9 {
	u64 u;
	struct cgxx_cmrx_tx_stat9_s {
		u64 hist3                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat9_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT9(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT9(u64 a)
{
	return 0x748 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_cmr#_tx_stat_pri#_xoff
 *
 * CGX CMR TX XON to XOFF transition Registers
 */
union cgxx_cmrx_tx_stat_prix_xoff {
	u64 u;
	struct cgxx_cmrx_tx_stat_prix_xoff_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmrx_tx_stat_prix_xoff_s cn; */
};

static inline u64 CGXX_CMRX_TX_STAT_PRIX_XOFF(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMRX_TX_STAT_PRIX_XOFF(u64 a, u64 b)
{
	return 0x800 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_cmr_bad
 *
 * CGX CMR Bad Registers
 */
union cgxx_cmr_bad {
	u64 u;
	struct cgxx_cmr_bad_s {
		u64 rxb_nxl                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmr_bad_s cn; */
};

static inline u64 CGXX_CMR_BAD(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_BAD(void)
{
	return 0x1020;
}

/**
 * Register (RSL) cgx#_cmr_chan_msk_and
 *
 * CGX CMR Backpressure Channel Mask AND Registers
 */
union cgxx_cmr_chan_msk_and {
	u64 u;
	struct cgxx_cmr_chan_msk_and_s {
		u64 msk_and                          : 64;
	} s;
	/* struct cgxx_cmr_chan_msk_and_s cn; */
};

static inline u64 CGXX_CMR_CHAN_MSK_AND(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_CHAN_MSK_AND(void)
{
	return 0x110;
}

/**
 * Register (RSL) cgx#_cmr_chan_msk_or
 *
 * CGX Backpressure Channel Mask OR Registers
 */
union cgxx_cmr_chan_msk_or {
	u64 u;
	struct cgxx_cmr_chan_msk_or_s {
		u64 msk_or                           : 64;
	} s;
	/* struct cgxx_cmr_chan_msk_or_s cn; */
};

static inline u64 CGXX_CMR_CHAN_MSK_OR(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_CHAN_MSK_OR(void)
{
	return 0x118;
}

/**
 * Register (RSL) cgx#_cmr_eco
 *
 * INTERNAL: CGX ECO Registers
 */
union cgxx_cmr_eco {
	u64 u;
	struct cgxx_cmr_eco_s {
		u64 eco_rw                           : 32;
		u64 eco_ro                           : 32;
	} s;
	/* struct cgxx_cmr_eco_s cn; */
};

static inline u64 CGXX_CMR_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_ECO(void)
{
	return 0x1028;
}

/**
 * Register (RSL) cgx#_cmr_global_config
 *
 * CGX CMR Global Configuration Register These registers configure the
 * global CMR, PCS, and MAC.
 */
union cgxx_cmr_global_config {
	u64 u;
	struct cgxx_cmr_global_config_s {
		u64 pmux_sds_sel                     : 1;
		u64 cgx_clk_enable                   : 1;
		u64 cmr_x2p_reset                    : 3;
		u64 interleave_mode                  : 1;
		u64 fcs_strip                        : 1;
		u64 ncsi_lmac_id                     : 2;
		u64 cmr_ncsi_drop                    : 1;
		u64 cmr_ncsi_reset                   : 1;
		u64 cmr_ncsi_tag_cnt                 : 13;
		u64 cmr_clken_ovrd                   : 1;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct cgxx_cmr_global_config_s cn; */
};

static inline u64 CGXX_CMR_GLOBAL_CONFIG(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_GLOBAL_CONFIG(void)
{
	return 8;
}

/**
 * Register (RSL) cgx#_cmr_mem_int
 *
 * CGX CMR Memory Interrupt Register
 */
union cgxx_cmr_mem_int {
	u64 u;
	struct cgxx_cmr_mem_int_s {
		u64 gmp_in_overfl                    : 1;
		u64 smu_in_overfl                    : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_cmr_mem_int_s cn; */
};

static inline u64 CGXX_CMR_MEM_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_MEM_INT(void)
{
	return 0x10;
}

/**
 * Register (RSL) cgx#_cmr_mem_int_ena_w1c
 *
 * CGX CMR Memory Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union cgxx_cmr_mem_int_ena_w1c {
	u64 u;
	struct cgxx_cmr_mem_int_ena_w1c_s {
		u64 gmp_in_overfl                    : 1;
		u64 smu_in_overfl                    : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_cmr_mem_int_ena_w1c_s cn; */
};

static inline u64 CGXX_CMR_MEM_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_MEM_INT_ENA_W1C(void)
{
	return 0x20;
}

/**
 * Register (RSL) cgx#_cmr_mem_int_ena_w1s
 *
 * CGX CMR Memory Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union cgxx_cmr_mem_int_ena_w1s {
	u64 u;
	struct cgxx_cmr_mem_int_ena_w1s_s {
		u64 gmp_in_overfl                    : 1;
		u64 smu_in_overfl                    : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_cmr_mem_int_ena_w1s_s cn; */
};

static inline u64 CGXX_CMR_MEM_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_MEM_INT_ENA_W1S(void)
{
	return 0x28;
}

/**
 * Register (RSL) cgx#_cmr_mem_int_w1s
 *
 * CGX CMR Memory Interrupt Set Register This register sets interrupt
 * bits.
 */
union cgxx_cmr_mem_int_w1s {
	u64 u;
	struct cgxx_cmr_mem_int_w1s_s {
		u64 gmp_in_overfl                    : 1;
		u64 smu_in_overfl                    : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_cmr_mem_int_w1s_s cn; */
};

static inline u64 CGXX_CMR_MEM_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_MEM_INT_W1S(void)
{
	return 0x18;
}

/**
 * Register (RSL) cgx#_cmr_nic_nxc_adr
 *
 * CGX CMR NIC NXC Exception Registers
 */
union cgxx_cmr_nic_nxc_adr {
	u64 u;
	struct cgxx_cmr_nic_nxc_adr_s {
		u64 channel                          : 12;
		u64 lmac_id                          : 4;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_cmr_nic_nxc_adr_s cn; */
};

static inline u64 CGXX_CMR_NIC_NXC_ADR(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_NIC_NXC_ADR(void)
{
	return 0x1030;
}

/**
 * Register (RSL) cgx#_cmr_nix0_nxc_adr
 *
 * CGX CMR NIX0 NXC Exception Registers
 */
union cgxx_cmr_nix0_nxc_adr {
	u64 u;
	struct cgxx_cmr_nix0_nxc_adr_s {
		u64 channel                          : 12;
		u64 lmac_id                          : 4;
		u64 channel_e                        : 12;
		u64 lmac_e_id                        : 4;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_cmr_nix0_nxc_adr_s cn; */
};

static inline u64 CGXX_CMR_NIX0_NXC_ADR(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_NIX0_NXC_ADR(void)
{
	return 0x1038;
}

/**
 * Register (RSL) cgx#_cmr_nix1_nxc_adr
 *
 * CGX CMR NIX1 NXC Exception Registers
 */
union cgxx_cmr_nix1_nxc_adr {
	u64 u;
	struct cgxx_cmr_nix1_nxc_adr_s {
		u64 channel                          : 12;
		u64 lmac_id                          : 4;
		u64 channel_e                        : 12;
		u64 lmac_e_id                        : 4;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_cmr_nix1_nxc_adr_s cn; */
};

static inline u64 CGXX_CMR_NIX1_NXC_ADR(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_NIX1_NXC_ADR(void)
{
	return 0x1040;
}

/**
 * Register (RSL) cgx#_cmr_p2x#_count
 *
 * CGX P2X Activity Register
 */
union cgxx_cmr_p2xx_count {
	u64 u;
	struct cgxx_cmr_p2xx_count_s {
		u64 p2x_cnt                          : 64;
	} s;
	/* struct cgxx_cmr_p2xx_count_s cn; */
};

static inline u64 CGXX_CMR_P2XX_COUNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_P2XX_COUNT(u64 a)
{
	return 0x168 + 0x1000 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_dmac#_cam0
 *
 * CGX CMR Receive CAM Registers These registers provide access to the 32
 * DMAC CAM0 entries in CGX, for use by X2P/NIX bound traffic.
 */
union cgxx_cmr_rx_dmacx_cam0 {
	u64 u;
	struct cgxx_cmr_rx_dmacx_cam0_s {
		u64 adr                              : 48;
		u64 en                               : 1;
		u64 id                               : 2;
		u64 reserved_51_63                   : 13;
	} s;
	/* struct cgxx_cmr_rx_dmacx_cam0_s cn; */
};

static inline u64 CGXX_CMR_RX_DMACX_CAM0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_DMACX_CAM0(u64 a)
{
	return 0x200 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_dmac#_cam1
 *
 * CGX CMR Receive CAM Registers These registers provide access to the 32
 * DMAC CAM entries in CGX for use by NCSI bound traffic. See
 * CGX()_CMR_GLOBAL_CONFIG[NCSI_LMAC_ID] and CGX()_CMR_RX_STEERING1()
 * registers.
 */
union cgxx_cmr_rx_dmacx_cam1 {
	u64 u;
	struct cgxx_cmr_rx_dmacx_cam1_s {
		u64 adr                              : 48;
		u64 en                               : 1;
		u64 id                               : 2;
		u64 reserved_51_63                   : 13;
	} s;
	/* struct cgxx_cmr_rx_dmacx_cam1_s cn; */
};

static inline u64 CGXX_CMR_RX_DMACX_CAM1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_DMACX_CAM1(u64 a)
{
	return 0x400 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_lmacs
 *
 * CGX CMR Receive Logical MACs Registers
 */
union cgxx_cmr_rx_lmacs {
	u64 u;
	struct cgxx_cmr_rx_lmacs_s {
		u64 lmacs                            : 3;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct cgxx_cmr_rx_lmacs_s cn; */
};

static inline u64 CGXX_CMR_RX_LMACS(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_LMACS(void)
{
	return 0x128;
}

/**
 * Register (RSL) cgx#_cmr_rx_ovr_bp
 *
 * CGX CMR Receive-Ports Backpressure Override Registers Per-LMAC
 * backpressure override register. For SMU, CGX()_CMR_RX_OVR_BP[EN]\<0\>
 * must be set to one and CGX()_CMR_RX_OVR_BP[BP]\<0\> must be cleared to
 * zero (to forcibly disable hardware-automatic 802.3 PAUSE packet
 * generation) with the HiGig2 Protocol when
 * CGX()_SMU()_HG2_CONTROL[HG2TX_EN]=0. (The HiGig2 protocol is indicated
 * by CGX()_SMU()_TX_CTL[HG_EN]=1 and CGX()_SMU()_RX_UDD_SKP[LEN]=16).
 * Hardware can only auto-generate backpressure through HiGig2 messages
 * (optionally, when CGX()_SMU()_HG2_CONTROL[HG2TX_EN]=1) with the HiGig2
 * protocol.
 */
union cgxx_cmr_rx_ovr_bp {
	u64 u;
	struct cgxx_cmr_rx_ovr_bp_s {
		u64 ign_fifo_bp                      : 4;
		u64 bp                               : 4;
		u64 en                               : 4;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct cgxx_cmr_rx_ovr_bp_s cn; */
};

static inline u64 CGXX_CMR_RX_OVR_BP(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_OVR_BP(void)
{
	return 0x130;
}

/**
 * Register (RSL) cgx#_cmr_rx_stat10
 *
 * CGX Receive Status Register 10 These registers provide a count of
 * octets of filtered DMAC1 or VLAN STEERING1 packets.
 */
union cgxx_cmr_rx_stat10 {
	u64 u;
	struct cgxx_cmr_rx_stat10_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmr_rx_stat10_s cn; */
};

static inline u64 CGXX_CMR_RX_STAT10(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STAT10(void)
{
	return 0xc0;
}

/**
 * Register (RSL) cgx#_cmr_rx_stat11
 *
 * CGX Receive Status Register 11 This registers provides a count of
 * packets dropped at the NCSI interface. This includes drops due to
 * CGX()_CMR_GLOBAL_CONFIG[CMR_NCSI_DROP] or NCSI FIFO full. The count of
 * dropped NCSI packets is not accounted for in any other stats
 * registers.
 */
union cgxx_cmr_rx_stat11 {
	u64 u;
	struct cgxx_cmr_rx_stat11_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmr_rx_stat11_s cn; */
};

static inline u64 CGXX_CMR_RX_STAT11(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STAT11(void)
{
	return 0xc8;
}

/**
 * Register (RSL) cgx#_cmr_rx_stat12
 *
 * CGX Receive Status Register 12 This register provide a count of octets
 * of dropped at the NCSI interface.
 */
union cgxx_cmr_rx_stat12 {
	u64 u;
	struct cgxx_cmr_rx_stat12_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmr_rx_stat12_s cn; */
};

static inline u64 CGXX_CMR_RX_STAT12(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STAT12(void)
{
	return 0xd0;
}

/**
 * Register (RSL) cgx#_cmr_rx_stat9
 *
 * CGX Receive Status Register 9 These registers provide a count of all
 * received packets that were dropped by the DMAC1 or VLAN STEERING1
 * filter. Packets that are dropped by the DMAC1 or VLAN STEERING1
 * filters are counted here regardless of whether they were ERR packets,
 * but does not include those reported in CGX()_CMR()_RX_STAT6. 16B
 * packets or smaller (20B in case of FCS strip) as the result of
 * truncation or other means are not dropped by CGX (unless filter and
 * decision is also asserted) and will never appear in this count. Should
 * the MAC signal to the CMR that the packet be filtered upon decision
 * before the end of packet, then STAT9 and STAT10 will not be updated.
 */
union cgxx_cmr_rx_stat9 {
	u64 u;
	struct cgxx_cmr_rx_stat9_s {
		u64 cnt                              : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_cmr_rx_stat9_s cn; */
};

static inline u64 CGXX_CMR_RX_STAT9(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STAT9(void)
{
	return 0xb8;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering0#
 *
 * CGX CMR Receive Steering0 Registers These registers, along with
 * CGX()_CMR_RX_STEERING_VETYPE0(), provide eight filters for identifying
 * and steering receive traffic to X2P/NIX. Received packets are only
 * passed to X2P/NIX when the DMAC0 filter result is ACCEPT and STEERING0
 * filter result is PASS. See also CGX()_CMR()_RX_DMAC_CTL0.  Internal:
 * "* ALGORITHM \<pre\> rx_steering(uint48 pkt_dmac, uint16 pkt_etype,
 * uint16 pkt_vlan_id) {    for (int i = 0; i \< 8; i++) {       steer =
 * CGX()_CMR_RX_STEERING0(i);       vetype =
 * CGX()_CMR_RX_STEERING_VETYPE0(i);       if (steer[MCST_EN] ||
 * steer[DMAC_EN] || vetype[VLAN_EN] || vetype[VLAN_TAG_EN]) {
 * // Filter is enabled.          if (   (!steer[MCST_EN] ||
 * is_mcst(pkt_dmac))              && (!steer[DMAC_EN] || pkt_dmac ==
 * steer[DMAC])              && (!vetype[VLAN_EN] || pkt_vlan_id ==
 * vetype[VLAN_ID])              && (!vetype[VLAN_TAG_EN] || pkt_etype ==
 * vetype[VLAN_ETYPE]) )          {             // Filter match (all
 * enabled matching criteria are met).             return steer[PASS];
 * }       }    }    return CGX()_CMR_RX_STEERING_DEFAULT0[PASS]; // No
 * match } \</pre\>"
 */
union cgxx_cmr_rx_steering0x {
	u64 u;
	struct cgxx_cmr_rx_steering0x_s {
		u64 dmac                             : 48;
		u64 dmac_en                          : 1;
		u64 mcst_en                          : 1;
		u64 pass                             : 1;
		u64 reserved_51_63                   : 13;
	} s;
	/* struct cgxx_cmr_rx_steering0x_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING0X(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING0X(u64 a)
{
	return 0x300 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering1#
 *
 * CGX CMR Receive Steering1 Registers These registers, along with
 * CGX()_CMR_RX_STEERING_VETYPE1(), provide eight filters for identifying
 * and steering NCSI receive traffic. Received packets are only passed to
 * NCSI when the DMAC1 filter result is ACCEPT and STEERING1 filter
 * result is PASS. See also CGX()_CMR_RX_DMAC()_CAM1 and
 * CGX()_CMR_RX_STEERING1(). For use with the LMAC associated with NCSI.
 * See CGX()_CMR_GLOBAL_CONFIG[NCSI_LMAC_ID].  Internal: ALGORITHM: See
 * CGX()_CMR_RX_STEERING0().
 */
union cgxx_cmr_rx_steering1x {
	u64 u;
	struct cgxx_cmr_rx_steering1x_s {
		u64 dmac                             : 48;
		u64 dmac_en                          : 1;
		u64 mcst_en                          : 1;
		u64 pass                             : 1;
		u64 reserved_51_63                   : 13;
	} s;
	/* struct cgxx_cmr_rx_steering1x_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING1X(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING1X(u64 a)
{
	return 0x500 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering_default0
 *
 * CGX CMR Receive Steering Default0 Destination Register For determining
 * destination of traffic that does not meet matching algorithm described
 * in registers CGX()_CMR_RX_STEERING0() and
 * CGX()_CMR_RX_STEERING_VETYPE0(). All 16B packets or smaller (20B in
 * case of FCS strip) as the result of truncation will steer to default
 * destination
 */
union cgxx_cmr_rx_steering_default0 {
	u64 u;
	struct cgxx_cmr_rx_steering_default0_s {
		u64 pass                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmr_rx_steering_default0_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING_DEFAULT0(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING_DEFAULT0(void)
{
	return 0x3f0;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering_default1
 *
 * CGX CMR Receive Steering Default1 Destination Register For use with
 * the lmac_id associated with NCSI. See
 * CGX()_CMR_GLOBAL_CONFIG[NCSI_LMAC_ID]. For determining destination of
 * traffic that does not meet matching algorithm described in registers
 * CGX()_CMR_RX_STEERING1() and CGX()_CMR_RX_STEERING_VETYPE1(). All 16B
 * packets or smaller (20B in case of FCS strip) as the result of
 * truncation will steer to default destination
 */
union cgxx_cmr_rx_steering_default1 {
	u64 u;
	struct cgxx_cmr_rx_steering_default1_s {
		u64 pass                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_cmr_rx_steering_default1_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING_DEFAULT1(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING_DEFAULT1(void)
{
	return 0x5e0;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering_vetype0#
 *
 * CGX CMR Receive VLAN Ethertype1 Register These registers, along with
 * CGX()_CMR_RX_STEERING0(), provide eight filters for identifying and
 * steering X2P/NIX receive traffic.
 */
union cgxx_cmr_rx_steering_vetype0x {
	u64 u;
	struct cgxx_cmr_rx_steering_vetype0x_s {
		u64 vlan_etype                       : 16;
		u64 vlan_tag_en                      : 1;
		u64 vlan_id                          : 12;
		u64 vlan_en                          : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct cgxx_cmr_rx_steering_vetype0x_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING_VETYPE0X(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING_VETYPE0X(u64 a)
{
	return 0x380 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_rx_steering_vetype1#
 *
 * CGX CMR Receive VLAN Ethertype1 Register For use with the lmac_id
 * associated with NCSI. See CGX()_CMR_GLOBAL_CONFIG[NCSI_LMAC_ID]. These
 * registers, along with CGX()_CMR_RX_STEERING1(), provide eight filters
 * for identifying and steering NCSI receive traffic.
 */
union cgxx_cmr_rx_steering_vetype1x {
	u64 u;
	struct cgxx_cmr_rx_steering_vetype1x_s {
		u64 vlan_etype                       : 16;
		u64 vlan_tag_en                      : 1;
		u64 vlan_id                          : 12;
		u64 vlan_en                          : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct cgxx_cmr_rx_steering_vetype1x_s cn; */
};

static inline u64 CGXX_CMR_RX_STEERING_VETYPE1X(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_RX_STEERING_VETYPE1X(u64 a)
{
	return 0x580 + 8 * a;
}

/**
 * Register (RSL) cgx#_cmr_tx_lmacs
 *
 * CGX CMR Transmit Logical MACs Registers This register sets the number
 * of LMACs allowed on the TX interface. The value is important for
 * defining the partitioning of the transmit FIFO.
 */
union cgxx_cmr_tx_lmacs {
	u64 u;
	struct cgxx_cmr_tx_lmacs_s {
		u64 lmacs                            : 3;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct cgxx_cmr_tx_lmacs_s cn; */
};

static inline u64 CGXX_CMR_TX_LMACS(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_TX_LMACS(void)
{
	return 0x1000;
}

/**
 * Register (RSL) cgx#_cmr_x2p#_count
 *
 * CGX X2P Activity Register
 */
union cgxx_cmr_x2px_count {
	u64 u;
	struct cgxx_cmr_x2px_count_s {
		u64 x2p_cnt                          : 64;
	} s;
	/* struct cgxx_cmr_x2px_count_s cn; */
};

static inline u64 CGXX_CMR_X2PX_COUNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CMR_X2PX_COUNT(u64 a)
{
	return 0x170 + 0x1000 * a;
}

/**
 * Register (RSL) cgx#_const
 *
 * CGX CONST Registers This register contains constants for software
 * discovery.
 */
union cgxx_const {
	u64 u;
	struct cgxx_const_s {
		u64 tx_fifosz                        : 24;
		u64 lmacs                            : 8;
		u64 rx_fifosz                        : 24;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct cgxx_const_s cn; */
};

static inline u64 CGXX_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CONST(void)
{
	return 0x2000;
}

/**
 * Register (RSL) cgx#_const1
 *
 * CGX CONST1 Registers This register contains constants for software
 * discovery.
 */
union cgxx_const1 {
	u64 u;
	struct cgxx_const1_s {
		u64 types                            : 11;
		u64 res_types                        : 21;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_const1_s cn; */
};

static inline u64 CGXX_CONST1(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_CONST1(void)
{
	return 0x2008;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_rx_wol_ctrl0
 *
 * CGX GMP GMI RX Wake-on-LAN Control 0 Registers
 */
union cgxx_gmp_gmix_rx_wol_ctrl0 {
	u64 u;
	struct cgxx_gmp_gmix_rx_wol_ctrl0_s {
		u64 dmac                             : 48;
		u64 pswd_len                         : 4;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct cgxx_gmp_gmix_rx_wol_ctrl0_s cn; */
};

static inline u64 CGXX_GMP_GMIX_RX_WOL_CTRL0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_RX_WOL_CTRL0(u64 a)
{
	return 0x38a00 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_rx_wol_ctrl1
 *
 * CGX GMP GMI RX Wake-on-LAN Control 1 Registers
 */
union cgxx_gmp_gmix_rx_wol_ctrl1 {
	u64 u;
	struct cgxx_gmp_gmix_rx_wol_ctrl1_s {
		u64 pswd                             : 64;
	} s;
	/* struct cgxx_gmp_gmix_rx_wol_ctrl1_s cn; */
};

static inline u64 CGXX_GMP_GMIX_RX_WOL_CTRL1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_RX_WOL_CTRL1(u64 a)
{
	return 0x38a08 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_tx_eee
 *
 * INTERNAL: CGX GMP GMI TX EEE Configure Registers  Reserved. Internal:
 * These registers control when GMP GMI TX requests to enter or exist
 * LPI. Those registers take effect only when EEE is supported and
 * enabled for a given LMAC.
 */
union cgxx_gmp_gmix_tx_eee {
	u64 u;
	struct cgxx_gmp_gmix_tx_eee_s {
		u64 idle_thresh                      : 28;
		u64 reserved_28                      : 1;
		u64 force_lpi                        : 1;
		u64 wakeup                           : 1;
		u64 auto_lpi                         : 1;
		u64 idle_cnt                         : 28;
		u64 tx_lpi                           : 1;
		u64 tx_lpi_wait                      : 1;
		u64 sync_status_lpi_enable           : 1;
		u64 reserved_63                      : 1;
	} s;
	/* struct cgxx_gmp_gmix_tx_eee_s cn; */
};

static inline u64 CGXX_GMP_GMIX_TX_EEE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_TX_EEE(u64 a)
{
	return 0x38800 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_tx_eee_cfg1
 *
 * INTERNAL: CGX GMP GMI TX EEE Configure More Configuration Registers
 * Reserved. Internal: Controls the GMP exiting of LPI and starting to
 * send data.
 */
union cgxx_gmp_gmix_tx_eee_cfg1 {
	u64 u;
	struct cgxx_gmp_gmix_tx_eee_cfg1_s {
		u64 wake2data_time                   : 24;
		u64 reserved_24_35                   : 12;
		u64 tx_eee_enable                    : 1;
		u64 reserved_37_39                   : 3;
		u64 sync2lpi_time                    : 21;
		u64 reserved_61_63                   : 3;
	} s;
	struct cgxx_gmp_gmix_tx_eee_cfg1_cn {
		u64 wake2data_time                   : 24;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_35                   : 4;
		u64 tx_eee_enable                    : 1;
		u64 reserved_37_39                   : 3;
		u64 sync2lpi_time                    : 21;
		u64 reserved_61_63                   : 3;
	} cn;
};

static inline u64 CGXX_GMP_GMIX_TX_EEE_CFG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_TX_EEE_CFG1(u64 a)
{
	return 0x38808 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_wol_int
 *
 * CGX GMP GMI RX WOL Interrupt Registers These registers allow WOL
 * interrupts to be sent to the control processor.
 */
union cgxx_gmp_gmix_wol_int {
	u64 u;
	struct cgxx_gmp_gmix_wol_int_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmix_wol_int_s cn; */
};

static inline u64 CGXX_GMP_GMIX_WOL_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_WOL_INT(u64 a)
{
	return 0x38a80 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_wol_int_ena_w1c
 *
 * CGX GMP GMI RX WOL Interrupt Enable Clear Registers This register
 * clears interrupt enable bits.
 */
union cgxx_gmp_gmix_wol_int_ena_w1c {
	u64 u;
	struct cgxx_gmp_gmix_wol_int_ena_w1c_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmix_wol_int_ena_w1c_s cn; */
};

static inline u64 CGXX_GMP_GMIX_WOL_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_WOL_INT_ENA_W1C(u64 a)
{
	return 0x38a90 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_wol_int_ena_w1s
 *
 * CGX GMP GMI RX WOL Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union cgxx_gmp_gmix_wol_int_ena_w1s {
	u64 u;
	struct cgxx_gmp_gmix_wol_int_ena_w1s_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmix_wol_int_ena_w1s_s cn; */
};

static inline u64 CGXX_GMP_GMIX_WOL_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_WOL_INT_ENA_W1S(u64 a)
{
	return 0x38a98 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi#_wol_int_w1s
 *
 * CGX GMP GMI RX WOL Interrupt Set Registers This register sets
 * interrupt bits.
 */
union cgxx_gmp_gmix_wol_int_w1s {
	u64 u;
	struct cgxx_gmp_gmix_wol_int_w1s_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmix_wol_int_w1s_s cn; */
};

static inline u64 CGXX_GMP_GMIX_WOL_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMIX_WOL_INT_W1S(u64 a)
{
	return 0x38a88 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_prt#_cfg
 *
 * CGX GMP GMI LMAC Configuration Registers This register controls the
 * configuration of the LMAC.
 */
union cgxx_gmp_gmi_prtx_cfg {
	u64 u;
	struct cgxx_gmp_gmi_prtx_cfg_s {
		u64 reserved_0                       : 1;
		u64 speed                            : 1;
		u64 duplex                           : 1;
		u64 slottime                         : 1;
		u64 reserved_4_7                     : 4;
		u64 speed_msb                        : 1;
		u64 reserved_9_11                    : 3;
		u64 rx_idle                          : 1;
		u64 tx_idle                          : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_gmp_gmi_prtx_cfg_s cn; */
};

static inline u64 CGXX_GMP_GMI_PRTX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_PRTX_CFG(u64 a)
{
	return 0x38020 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_decision
 *
 * CGX GMP Packet-Decision Registers This register specifies the byte
 * count used to determine when to accept or to filter a packet. As each
 * byte in a packet is received by GMI, the L2 byte count is compared
 * against [CNT]. In normal operation, the L2 header begins after the
 * PREAMBLE + SFD (CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK] = 1) and any
 * optional UDD skip data (CGX()_GMP_GMI_RX()_UDD_SKP[LEN]).  Internal:
 * Notes: As each byte in a packet is received by GMI, the L2 byte count
 * is compared against the [CNT].  The L2 byte count is the number of
 * bytes from the beginning of the L2 header (DMAC).  In normal
 * operation, the L2 header begins after the PREAMBLE+SFD
 * (CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK]=1) and any optional UDD skip data
 * (CGX()_GMP_GMI_RX()_UDD_SKP[LEN]). When
 * CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK] is clear, PREAMBLE+SFD are
 * prepended to the packet and would require UDD skip length to account
 * for them.  Full Duplex: _   L2 Size \<  [CNT] - Accept packet. No
 * filtering is applied. _   L2 Size \>= [CNT] - Apply filter. Accept
 * packet based on PAUSE packet filter.  Half Duplex: _   L2 Size \<
 * [CNT] - Drop packet. Packet is unconditionally dropped. _   L2 Size
 * \>= [CNT] - Accept packet.  where L2_size = MAX(0, total_packet_size -
 * CGX()_GMP_GMI_RX()_UDD_SKP[LEN] -
 * ((CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK]==1)*8)).
 */
union cgxx_gmp_gmi_rxx_decision {
	u64 u;
	struct cgxx_gmp_gmi_rxx_decision_s {
		u64 cnt                              : 5;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct cgxx_gmp_gmi_rxx_decision_s cn; */
};

static inline u64 CGXX_GMP_GMI_RXX_DECISION(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_DECISION(u64 a)
{
	return 0x38040 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_frm_chk
 *
 * CGX GMP Frame Check Registers
 */
union cgxx_gmp_gmi_rxx_frm_chk {
	u64 u;
	struct cgxx_gmp_gmi_rxx_frm_chk_s {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 reserved_2                       : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 reserved_5_6                     : 2;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct cgxx_gmp_gmi_rxx_frm_chk_s cn; */
};

static inline u64 CGXX_GMP_GMI_RXX_FRM_CHK(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_FRM_CHK(u64 a)
{
	return 0x38030 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_frm_ctl
 *
 * CGX GMP Frame Control Registers This register controls the handling of
 * the frames. The [CTL_BCK] and [CTL_DRP] bits control how the hardware
 * handles incoming PAUSE packets. The most common modes of operation: _
 * [CTL_BCK] = 1, [CTL_DRP] = 1: hardware handles everything. _ [CTL_BCK]
 * = 0, [CTL_DRP] = 0: software sees all PAUSE frames. _ [CTL_BCK] = 0,
 * [CTL_DRP] = 1: all PAUSE frames are completely ignored.  These control
 * bits should be set to [CTL_BCK] = 0, [CTL_DRP] = 0 in half-duplex
 * mode. Since PAUSE packets only apply to full duplex operation, any
 * PAUSE packet would constitute an exception which should be handled by
 * the processing cores. PAUSE packets should not be forwarded.
 * Internal: Notes: [PRE_STRP]: When [PRE_CHK] is set (indicating that
 * the PREAMBLE will be sent), [PRE_STRP] determines if the PREAMBLE+SFD
 * bytes are thrown away or sent to the Octane core as part of the
 * packet. In either mode, the PREAMBLE+SFD bytes are not counted toward
 * the packet size when checking against the MIN and MAX bounds.
 * Furthermore, the bytes are skipped when locating the start of the L2
 * header for DMAC and Control frame recognition.
 */
union cgxx_gmp_gmi_rxx_frm_ctl {
	u64 u;
	struct cgxx_gmp_gmi_rxx_frm_ctl_s {
		u64 pre_chk                          : 1;
		u64 pre_strp                         : 1;
		u64 ctl_drp                          : 1;
		u64 ctl_bck                          : 1;
		u64 ctl_mcst                         : 1;
		u64 ctl_smac                         : 1;
		u64 pre_free                         : 1;
		u64 reserved_7_8                     : 2;
		u64 pre_align                        : 1;
		u64 null_dis                         : 1;
		u64 reserved_11                      : 1;
		u64 ptp_mode                         : 1;
		u64 rx_fc_type                       : 1;
		u64 reserved_14_63                   : 50;
	} s;
	struct cgxx_gmp_gmi_rxx_frm_ctl_cn {
		u64 pre_chk                          : 1;
		u64 pre_strp                         : 1;
		u64 ctl_drp                          : 1;
		u64 ctl_bck                          : 1;
		u64 ctl_mcst                         : 1;
		u64 ctl_smac                         : 1;
		u64 pre_free                         : 1;
		u64 reserved_7                       : 1;
		u64 reserved_8                       : 1;
		u64 pre_align                        : 1;
		u64 null_dis                         : 1;
		u64 reserved_11                      : 1;
		u64 ptp_mode                         : 1;
		u64 rx_fc_type                       : 1;
		u64 reserved_14_63                   : 50;
	} cn;
};

static inline u64 CGXX_GMP_GMI_RXX_FRM_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_FRM_CTL(u64 a)
{
	return 0x38028 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_ifg
 *
 * CGX GMI Minimum Interframe-Gap Cycles Registers This register
 * specifies the minimum number of interframe-gap (IFG) cycles between
 * packets.
 */
union cgxx_gmp_gmi_rxx_ifg {
	u64 u;
	struct cgxx_gmp_gmi_rxx_ifg_s {
		u64 ifg                              : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_gmp_gmi_rxx_ifg_s cn; */
};

static inline u64 CGXX_GMP_GMI_RXX_IFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_IFG(u64 a)
{
	return 0x38058 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_int
 *
 * CGX GMP GMI RX Interrupt Registers These registers allow interrupts to
 * be sent to the control processor. * Exception conditions \<10:0\> can
 * also set the rcv/opcode in the received packet's work-queue entry.
 * CGX()_GMP_GMI_RX()_FRM_CHK provides a bit mask for configuring which
 * conditions set the error. In half duplex operation, the expectation is
 * that collisions will appear as either MINERR or CAREXT errors.
 * Internal: Notes: (1) exception conditions 10:0 can also set the
 * rcv/opcode in the received packet's workQ entry.  The
 * CGX()_GMP_GMI_RX()_FRM_CHK register provides a bit mask for
 * configuring which conditions set the error.  (2) in half duplex
 * operation, the expectation is that collisions will appear as either
 * MINERR o r CAREXT errors.  (3) JABBER An RX jabber error indicates
 * that a packet was received which is longer than the maximum allowed
 * packet as defined by the system.  GMI will truncate the packet at the
 * JABBER count. Failure to do so could lead to system instabilty.  (4)
 * NIBERR This error is illegal at 1000Mbs speeds
 * (CGX()_GMP_GMI_PRT()_CFG[SPEED]==0) and will never assert.  (5) MINERR
 * total frame DA+SA+TL+DATA+PAD+FCS \< 64  (6) ALNERR Indicates that the
 * packet received was not an integer number of bytes.  If FCS checking
 * is enabled, ALNERR will only assert if the FCS is bad.  If FCS
 * checking is disabled, ALNERR will assert in all non-integer frame
 * cases.  (7) Collisions Collisions can only occur in half-duplex mode.
 * A collision is assumed by the receiver when the slottime
 * (CGX()_GMP_GMI_PRT()_CFG[SLOTTIME]) is not satisfied.  In 10/100 mode,
 * this will result in a frame \< SLOTTIME.  In 1000 mode, it could
 * result either in frame \< SLOTTIME or a carrier extend error with the
 * SLOTTIME.  These conditions are visible by... . transfer ended before
 * slottime COLDET . carrier extend error           CAREXT  (A) LENERR
 * Length errors occur when the received packet does not match the length
 * field.  LENERR is only checked for packets between 64 and 1500 bytes.
 * For untagged frames, the length must exact match.  For tagged frames
 * the length or length+4 must match.  (B) PCTERR checks that the frame
 * begins with a valid PREAMBLE sequence. Does not check the number of
 * PREAMBLE cycles.  (C) OVRERR *DON'T PUT IN HRM* OVRERR is an
 * architectural assertion check internal to GMI to make sure no
 * assumption was violated.  In a correctly operating system, this
 * interrupt can never fire. GMI has an internal arbiter which selects
 * which of four ports to buffer in the main RX FIFO.  If we normally
 * buffer eight bytes, then each port will typically push a tick every
 * eight cycles if the packet interface is going as fast as possible.  If
 * there are four ports, they push every two cycles.  So that's the
 * assumption.  That the inbound module will always be able to consume
 * the tick before another is produced.  If that doesn't happen that's
 * when OVRERR will assert."
 */
union cgxx_gmp_gmi_rxx_int {
	u64 u;
	struct cgxx_gmp_gmi_rxx_int_s {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_63                   : 52;
	} s;
	struct cgxx_gmp_gmi_rxx_int_cn {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_15                   : 4;
		u64 reserved_16_63                   : 48;
	} cn;
};

static inline u64 CGXX_GMP_GMI_RXX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_INT(u64 a)
{
	return 0x38000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_int_ena_w1c
 *
 * CGX GMP GMI RX Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_gmp_gmi_rxx_int_ena_w1c {
	u64 u;
	struct cgxx_gmp_gmi_rxx_int_ena_w1c_s {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_63                   : 52;
	} s;
	struct cgxx_gmp_gmi_rxx_int_ena_w1c_cn {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_15                   : 4;
		u64 reserved_16_63                   : 48;
	} cn;
};

static inline u64 CGXX_GMP_GMI_RXX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_INT_ENA_W1C(u64 a)
{
	return 0x38010 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_int_ena_w1s
 *
 * CGX GMP GMI RX Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union cgxx_gmp_gmi_rxx_int_ena_w1s {
	u64 u;
	struct cgxx_gmp_gmi_rxx_int_ena_w1s_s {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_63                   : 52;
	} s;
	struct cgxx_gmp_gmi_rxx_int_ena_w1s_cn {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_15                   : 4;
		u64 reserved_16_63                   : 48;
	} cn;
};

static inline u64 CGXX_GMP_GMI_RXX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_INT_ENA_W1S(u64 a)
{
	return 0x38018 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_int_w1s
 *
 * CGX GMP GMI RX Interrupt Set Registers This register sets interrupt
 * bits.
 */
union cgxx_gmp_gmi_rxx_int_w1s {
	u64 u;
	struct cgxx_gmp_gmi_rxx_int_w1s_s {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_63                   : 52;
	} s;
	struct cgxx_gmp_gmi_rxx_int_w1s_cn {
		u64 minerr                           : 1;
		u64 carext                           : 1;
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 ovrerr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 falerr                           : 1;
		u64 coldet                           : 1;
		u64 ifgerr                           : 1;
		u64 reserved_12_15                   : 4;
		u64 reserved_16_63                   : 48;
	} cn;
};

static inline u64 CGXX_GMP_GMI_RXX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_INT_W1S(u64 a)
{
	return 0x38008 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_jabber
 *
 * CGX GMP Maximum Packet-Size Registers This register specifies the
 * maximum size for packets, beyond which the GMI truncates.
 */
union cgxx_gmp_gmi_rxx_jabber {
	u64 u;
	struct cgxx_gmp_gmi_rxx_jabber_s {
		u64 cnt                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_rxx_jabber_s cn; */
};

static inline u64 CGXX_GMP_GMI_RXX_JABBER(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_JABBER(u64 a)
{
	return 0x38038 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_rx#_udd_skp
 *
 * CGX GMP GMI User-Defined Data Skip Registers This register specifies
 * the amount of user-defined data (UDD) added before the start of the
 * L2C data.  Internal: Notes: (1) The skip bytes are part of the packet
 * and will be handled by NIX.  (2) The system can determine if the UDD
 * bytes are included in the FCS check by using the FCSSEL field - if the
 * FCS check is enabled.  (3) Assume that the preamble/sfd is always at
 * the start of the frame - even before UDD bytes.  In most cases, there
 * will be no preamble in these cases since it will be packet interface
 * in direct communication to another packet interface (MAC to MAC)
 * without a PHY involved.  (4) We can still do address filtering and
 * control packet filtering is the user desires.  (5)
 * CGX()_GMP_GMI_RX()_UDD_SKP[LEN] must be 0 in half-duplex operation
 * unless CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK] is clear.  If
 * CGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK] is clear, then
 * CGX()_GMP_GMI_RX()_UDD_SKP[LEN] will normally be 8.  (6) In all cases,
 * the UDD bytes will be sent down the packet interface as part of the
 * packet.  The UDD bytes are never stripped from the actual packet.
 */
union cgxx_gmp_gmi_rxx_udd_skp {
	u64 u;
	struct cgxx_gmp_gmi_rxx_udd_skp_s {
		u64 len                              : 7;
		u64 reserved_7                       : 1;
		u64 fcssel                           : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct cgxx_gmp_gmi_rxx_udd_skp_s cn; */
};

static inline u64 CGXX_GMP_GMI_RXX_UDD_SKP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_RXX_UDD_SKP(u64 a)
{
	return 0x38048 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_smac#
 *
 * CGX GMI SMAC Registers
 */
union cgxx_gmp_gmi_smacx {
	u64 u;
	struct cgxx_gmp_gmi_smacx_s {
		u64 smac                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_gmp_gmi_smacx_s cn; */
};

static inline u64 CGXX_GMP_GMI_SMACX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_SMACX(u64 a)
{
	return 0x38230 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_append
 *
 * CGX GMI TX Append Control Registers
 */
union cgxx_gmp_gmi_txx_append {
	u64 u;
	struct cgxx_gmp_gmi_txx_append_s {
		u64 preamble                         : 1;
		u64 pad                              : 1;
		u64 fcs                              : 1;
		u64 force_fcs                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_gmp_gmi_txx_append_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_APPEND(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_APPEND(u64 a)
{
	return 0x38218 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_burst
 *
 * CGX GMI TX Burst-Counter Registers
 */
union cgxx_gmp_gmi_txx_burst {
	u64 u;
	struct cgxx_gmp_gmi_txx_burst_s {
		u64 burst                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_txx_burst_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_BURST(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_BURST(u64 a)
{
	return 0x38228 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_ctl
 *
 * CGX GMI Transmit Control Registers
 */
union cgxx_gmp_gmi_txx_ctl {
	u64 u;
	struct cgxx_gmp_gmi_txx_ctl_s {
		u64 xscol_en                         : 1;
		u64 xsdef_en                         : 1;
		u64 tx_fc_type                       : 1;
		u64 link_drain                       : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_gmp_gmi_txx_ctl_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_CTL(u64 a)
{
	return 0x38270 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_int
 *
 * CGX GMI TX Interrupt Registers
 */
union cgxx_gmp_gmi_txx_int {
	u64 u;
	struct cgxx_gmp_gmi_txx_int_s {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_63                    : 59;
	} s;
	struct cgxx_gmp_gmi_txx_int_cn {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_7                     : 3;
		u64 reserved_8                       : 1;
		u64 reserved_9_63                    : 55;
	} cn;
};

static inline u64 CGXX_GMP_GMI_TXX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_INT(u64 a)
{
	return 0x38500 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_int_ena_w1c
 *
 * CGX GMI TX Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_gmp_gmi_txx_int_ena_w1c {
	u64 u;
	struct cgxx_gmp_gmi_txx_int_ena_w1c_s {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_63                    : 59;
	} s;
	struct cgxx_gmp_gmi_txx_int_ena_w1c_cn {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_7                     : 3;
		u64 reserved_8                       : 1;
		u64 reserved_9_63                    : 55;
	} cn;
};

static inline u64 CGXX_GMP_GMI_TXX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_INT_ENA_W1C(u64 a)
{
	return 0x38510 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_int_ena_w1s
 *
 * CGX GMI TX Interrupt Enable Set Registers This register sets interrupt
 * enable bits.
 */
union cgxx_gmp_gmi_txx_int_ena_w1s {
	u64 u;
	struct cgxx_gmp_gmi_txx_int_ena_w1s_s {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_63                    : 59;
	} s;
	struct cgxx_gmp_gmi_txx_int_ena_w1s_cn {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_7                     : 3;
		u64 reserved_8                       : 1;
		u64 reserved_9_63                    : 55;
	} cn;
};

static inline u64 CGXX_GMP_GMI_TXX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_INT_ENA_W1S(u64 a)
{
	return 0x38518 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_int_w1s
 *
 * CGX GMI TX Interrupt Set Registers This register sets interrupt bits.
 */
union cgxx_gmp_gmi_txx_int_w1s {
	u64 u;
	struct cgxx_gmp_gmi_txx_int_w1s_s {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_63                    : 59;
	} s;
	struct cgxx_gmp_gmi_txx_int_w1s_cn {
		u64 undflw                           : 1;
		u64 xscol                            : 1;
		u64 xsdef                            : 1;
		u64 late_col                         : 1;
		u64 ptp_lost                         : 1;
		u64 reserved_5_7                     : 3;
		u64 reserved_8                       : 1;
		u64 reserved_9_63                    : 55;
	} cn;
};

static inline u64 CGXX_GMP_GMI_TXX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_INT_W1S(u64 a)
{
	return 0x38508 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_min_pkt
 *
 * CGX GMI TX Minimum-Size-Packet Registers
 */
union cgxx_gmp_gmi_txx_min_pkt {
	u64 u;
	struct cgxx_gmp_gmi_txx_min_pkt_s {
		u64 min_size                         : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct cgxx_gmp_gmi_txx_min_pkt_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_MIN_PKT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_MIN_PKT(u64 a)
{
	return 0x38240 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_pause_pkt_interval
 *
 * CGX GMI TX PAUSE-Packet Transmission-Interval Registers This register
 * specifies how often PAUSE packets are sent. Internal: Notes: Choosing
 * proper values of CGX()_GMP_GMI_TX()_PAUSE_PKT_TIME[PTIME] and
 * CGX()_GMP_GMI_TX()_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to
 * the system designer.  It is suggested that TIME be much greater than
 * INTERVAL and CGX()_GMP_GMI_TX()_PAUSE_ZERO[SEND] be set.  This allows
 * a periodic refresh of the PAUSE count and then when the backpressure
 * condition is lifted, a PAUSE packet with TIME==0 will be sent
 * indicating that Octane is ready for additional data.  If the system
 * chooses to not set CGX()_GMP_GMI_TX()_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they
 * satisify the following rule:  _ INTERVAL \<= TIME - (largest_pkt_size
 * + IFG + pause_pkt_size)  where largest_pkt_size is that largest packet
 * that the system can send (normally 1518B), IFG is the interframe gap
 * and pause_pkt_size is the size of the PAUSE packet (normally 64B).
 */
union cgxx_gmp_gmi_txx_pause_pkt_interval {
	u64 u;
	struct cgxx_gmp_gmi_txx_pause_pkt_interval_s {
		u64 interval                         : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_txx_pause_pkt_interval_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(u64 a)
{
	return 0x38248 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_pause_pkt_time
 *
 * CGX GMI TX PAUSE Packet PAUSE-Time Registers
 */
union cgxx_gmp_gmi_txx_pause_pkt_time {
	u64 u;
	struct cgxx_gmp_gmi_txx_pause_pkt_time_s {
		u64 ptime                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_txx_pause_pkt_time_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(u64 a)
{
	return 0x38238 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_pause_togo
 *
 * CGX GMI TX Time-to-Backpressure Registers
 */
union cgxx_gmp_gmi_txx_pause_togo {
	u64 u;
	struct cgxx_gmp_gmi_txx_pause_togo_s {
		u64 ptime                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_txx_pause_togo_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_PAUSE_TOGO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_PAUSE_TOGO(u64 a)
{
	return 0x38258 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_pause_zero
 *
 * CGX GMI TX PAUSE-Zero-Enable Registers
 */
union cgxx_gmp_gmi_txx_pause_zero {
	u64 u;
	struct cgxx_gmp_gmi_txx_pause_zero_s {
		u64 send                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmi_txx_pause_zero_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_PAUSE_ZERO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_PAUSE_ZERO(u64 a)
{
	return 0x38260 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_sgmii_ctl
 *
 * CGX SGMII Control Registers
 */
union cgxx_gmp_gmi_txx_sgmii_ctl {
	u64 u;
	struct cgxx_gmp_gmi_txx_sgmii_ctl_s {
		u64 align                            : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_gmp_gmi_txx_sgmii_ctl_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_SGMII_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_SGMII_CTL(u64 a)
{
	return 0x38300 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_slot
 *
 * CGX GMI TX Slottime Counter Registers
 */
union cgxx_gmp_gmi_txx_slot {
	u64 u;
	struct cgxx_gmp_gmi_txx_slot_s {
		u64 slot                             : 10;
		u64 reserved_10_63                   : 54;
	} s;
	/* struct cgxx_gmp_gmi_txx_slot_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_SLOT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_SLOT(u64 a)
{
	return 0x38220 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_soft_pause
 *
 * CGX GMI TX Software PAUSE Registers
 */
union cgxx_gmp_gmi_txx_soft_pause {
	u64 u;
	struct cgxx_gmp_gmi_txx_soft_pause_s {
		u64 ptime                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_txx_soft_pause_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_SOFT_PAUSE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_SOFT_PAUSE(u64 a)
{
	return 0x38250 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx#_thresh
 *
 * CGX GMI TX Threshold Registers
 */
union cgxx_gmp_gmi_txx_thresh {
	u64 u;
	struct cgxx_gmp_gmi_txx_thresh_s {
		u64 cnt                              : 11;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct cgxx_gmp_gmi_txx_thresh_s cn; */
};

static inline u64 CGXX_GMP_GMI_TXX_THRESH(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TXX_THRESH(u64 a)
{
	return 0x38210 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_col_attempt
 *
 * CGX TX Collision Attempts Before Dropping Frame Registers
 */
union cgxx_gmp_gmi_tx_col_attempt {
	u64 u;
	struct cgxx_gmp_gmi_tx_col_attempt_s {
		u64 limit                            : 5;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct cgxx_gmp_gmi_tx_col_attempt_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_COL_ATTEMPT(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_COL_ATTEMPT(void)
{
	return 0x39010;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_ifg
 *
 * CGX GMI TX Interframe-Gap Cycles Registers Consider the following when
 * programming IFG1 and IFG2: * For 10/100/1000 Mb/s half-duplex systems
 * that require IEEE 802.3 compatibility, IFG1 must be in the range of
 * 1-8, [IFG2] must be in the range of 4-12, and the [IFG1] + [IFG2] sum
 * must be 12. * For 10/100/1000 Mb/s full-duplex systems that require
 * IEEE 802.3 compatibility, IFG1 must be in the range of 1-11, [IFG2]
 * must be in the range of 1-11, and the [IFG1] + [IFG2] sum must be 12.
 * For all other systems, IFG1 and IFG2 can be any value in the range of
 * 1-15, allowing for a total possible IFG sum of 2-30.
 */
union cgxx_gmp_gmi_tx_ifg {
	u64 u;
	struct cgxx_gmp_gmi_tx_ifg_s {
		u64 ifg1                             : 4;
		u64 ifg2                             : 4;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct cgxx_gmp_gmi_tx_ifg_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_IFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_IFG(void)
{
	return 0x39000;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_jam
 *
 * CGX GMI TX JAM Pattern Registers This register provides the pattern
 * used in JAM bytes.
 */
union cgxx_gmp_gmi_tx_jam {
	u64 u;
	struct cgxx_gmp_gmi_tx_jam_s {
		u64 jam                              : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct cgxx_gmp_gmi_tx_jam_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_JAM(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_JAM(void)
{
	return 0x39008;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_lfsr
 *
 * CGX GMI TX LFSR Registers This register shows the contents of the
 * linear feedback shift register (LFSR), which is used to implement
 * truncated binary exponential backoff.
 */
union cgxx_gmp_gmi_tx_lfsr {
	u64 u;
	struct cgxx_gmp_gmi_tx_lfsr_s {
		u64 lfsr                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_tx_lfsr_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_LFSR(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_LFSR(void)
{
	return 0x39028;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_pause_pkt_dmac
 *
 * CGX TX PAUSE-Packet DMAC-Field Registers
 */
union cgxx_gmp_gmi_tx_pause_pkt_dmac {
	u64 u;
	struct cgxx_gmp_gmi_tx_pause_pkt_dmac_s {
		u64 dmac                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_gmp_gmi_tx_pause_pkt_dmac_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(void)
{
	return 0x39018;
}

/**
 * Register (RSL) cgx#_gmp_gmi_tx_pause_pkt_type
 *
 * CGX GMI TX PAUSE-Packet-PTYPE Field Registers This register provides
 * the PTYPE field that is placed in outbound PAUSE packets.
 */
union cgxx_gmp_gmi_tx_pause_pkt_type {
	u64 u;
	struct cgxx_gmp_gmi_tx_pause_pkt_type_s {
		u64 ptype                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_gmi_tx_pause_pkt_type_s cn; */
};

static inline u64 CGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(void)
{
	return 0x39020;
}

/**
 * Register (RSL) cgx#_gmp_misc#_cfg
 *
 * CGX GMP PCS Miscellaneous Control Registers This register contains
 * general configuration that should not need to be changed from reset
 * settings.  Internal: Per lmac diagnostic and chicken bits.
 */
union cgxx_gmp_miscx_cfg {
	u64 u;
	struct cgxx_gmp_miscx_cfg_s {
		u64 tx_eee_quiet_credit_mode         : 1;
		u64 tx_eee_wait_gmi_fast_idle        : 1;
		u64 tx_qsgmii_port0_init             : 1;
		u64 tx_eee_rx_sync_status_enable     : 1;
		u64 pcs_alt_an                       : 1;
		u64 reserved_5_7                     : 3;
		u64 rx_pcs_sync_signal_detect        : 1;
		u64 rx_pcs_sync_timeout              : 1;
		u64 rx_pcs_eee_mode_enable           : 1;
		u64 rx_pcs_lpi_enable                : 1;
		u64 rx_pcs_802_rx_k                  : 1;
		u64 rx_pcs_alt_qlb2i                 : 1;
		u64 reserved_14_15                   : 2;
		u64 rx_cgp_gser_throttle             : 1;
		u64 rx_cgp_edet_filter               : 1;
		u64 rx_cgp_edet_qlm_val              : 1;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct cgxx_gmp_miscx_cfg_s cn; */
};

static inline u64 CGXX_GMP_MISCX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_MISCX_CFG(u64 a)
{
	return 0x34000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_an_expansion
 *
 * CGX GMP PCS AN Expansion register Register 6 AN status
 */
union cgxx_gmp_pcsx_an_expansion {
	u64 u;
	struct cgxx_gmp_pcsx_an_expansion_s {
		u64 reserved_0                       : 1;
		u64 page_received                    : 1;
		u64 next_page_able                   : 1;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct cgxx_gmp_pcsx_an_expansion_s cn; */
};

static inline u64 CGXX_GMP_PCSX_AN_EXPANSION(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_AN_EXPANSION(u64 a)
{
	return 0x30a60 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_an_lp_abil_np
 *
 * CGX GMP PCS AN Link Partner Ability Next Page Register 8 This register
 * contains the advertised ability of the link partners Next Page. The
 * definition for this register is provided in 32.5.4.2 for changes to
 * 28.2.4.1.4.
 */
union cgxx_gmp_pcsx_an_lp_abil_np {
	u64 u;
	struct cgxx_gmp_pcsx_an_lp_abil_np_s {
		u64 m_u                              : 11;
		u64 toggle                           : 1;
		u64 ack2                             : 1;
		u64 mp                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcsx_an_lp_abil_np_s cn; */
};

static inline u64 CGXX_GMP_PCSX_AN_LP_ABIL_NP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_AN_LP_ABIL_NP(u64 a)
{
	return 0x30a80 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_an_np_tx
 *
 * CGX GMP PCS AN Next Page Transmit Register 7 Software programs this
 * register with the contents of the AN message next page or unformatted
 * next page link code word to be transmitted during autonegotiation.
 * Next page exchange occurs after the base link code words have been
 * exchanged if either end of the link segment sets the NP bit to 1,
 * indicating that it has at least one next page to send. Once initiated,
 * next page exchange continues until both ends of the link segment set
 * their NP bits to 0. Both sides must be NP capable to use NP exchanges.
 */
union cgxx_gmp_pcsx_an_np_tx {
	u64 u;
	struct cgxx_gmp_pcsx_an_np_tx_s {
		u64 m_u                              : 11;
		u64 toggle                           : 1;
		u64 ack2                             : 1;
		u64 mp                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcsx_an_np_tx_s cn; */
};

static inline u64 CGXX_GMP_PCSX_AN_NP_TX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_AN_NP_TX(u64 a)
{
	return 0x30a70 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_dbg_control
 *
 * CGX PCS Debug Control Registers
 */
union cgxx_gmp_pcsx_dbg_control {
	u64 u;
	struct cgxx_gmp_pcsx_dbg_control_s {
		u64 us_clk_period                    : 7;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_gmp_pcsx_dbg_control_s cn; */
};

static inline u64 CGXX_GMP_PCSX_DBG_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_DBG_CONTROL(u64 a)
{
	return 0x31000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_rx_eee_wake
 *
 * INTERNAL: CGX GMP PCS  RX EEE Wake Error Counter  Registers  Reserved.
 * Internal: This register is used by PHY types that support EEE to count
 * wake time faults where the PHY fails to complete its normal wake
 * sequence within the time required for the specific PHY type. The
 * definition of the fault event to be counted is defined for each PHY
 * and may occur during a refresh or a wake-up as defined by the PHY.
 * This 16-bit counter shall be reset to all zeros upon execution of the
 * PCS reset. This counter shall be held at all ones in the case of
 * overflow.
 */
union cgxx_gmp_pcsx_rx_eee_wake {
	u64 u;
	struct cgxx_gmp_pcsx_rx_eee_wake_s {
		u64 error_counter                    : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcsx_rx_eee_wake_s cn; */
};

static inline u64 CGXX_GMP_PCSX_RX_EEE_WAKE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_RX_EEE_WAKE(u64 a)
{
	return 0x30910 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_rx_lpi_timing
 *
 * INTERNAL: CGX GMP PCS  RX EEE LPI Timing Parameters Registers
 * Reserved. Internal: Receiver LPI timing parameters Tqr, Twr and Twtf.
 */
union cgxx_gmp_pcsx_rx_lpi_timing {
	u64 u;
	struct cgxx_gmp_pcsx_rx_lpi_timing_s {
		u64 twtf                             : 18;
		u64 reserved_18_19                   : 2;
		u64 twr                              : 12;
		u64 tqr                              : 20;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct cgxx_gmp_pcsx_rx_lpi_timing_s cn; */
};

static inline u64 CGXX_GMP_PCSX_RX_LPI_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_RX_LPI_TIMING(u64 a)
{
	return 0x30900 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_status1
 *
 * CGX GMP PCS Status 1 Register PCS LPI Status, Link OK.  Register 3.1
 */
union cgxx_gmp_pcsx_status1 {
	u64 u;
	struct cgxx_gmp_pcsx_status1_s {
		u64 reserved_0_1                     : 2;
		u64 receive_link_status              : 1;
		u64 reserved_3_7                     : 5;
		u64 rx_lpi_indication                : 1;
		u64 tx_lpi_indication                : 1;
		u64 rx_lpi_received                  : 1;
		u64 tx_lpi_received                  : 1;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct cgxx_gmp_pcsx_status1_s cn; */
};

static inline u64 CGXX_GMP_PCSX_STATUS1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_STATUS1(u64 a)
{
	return 0x30880 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs#_tx_lpi_timing
 *
 * INTERNAL: CGX GMP GMI  TX EEE LPI Timing Parameters Registers
 * Reserved. Internal: Transmitter LPI timing parameters Tsl, Tql and
 * Tul.
 */
union cgxx_gmp_pcsx_tx_lpi_timing {
	u64 u;
	struct cgxx_gmp_pcsx_tx_lpi_timing_s {
		u64 tql                              : 19;
		u64 reserved_19_31                   : 13;
		u64 tul                              : 12;
		u64 reserved_44_47                   : 4;
		u64 tsl                              : 12;
		u64 reserved_60_63                   : 4;
	} s;
	/* struct cgxx_gmp_pcsx_tx_lpi_timing_s cn; */
};

static inline u64 CGXX_GMP_PCSX_TX_LPI_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCSX_TX_LPI_TIMING(u64 a)
{
	return 0x30800 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_an#_adv
 *
 * CGX GMP PCS Autonegotiation Advertisement Registers
 */
union cgxx_gmp_pcs_anx_adv {
	u64 u;
	struct cgxx_gmp_pcs_anx_adv_s {
		u64 reserved_0_4                     : 5;
		u64 fd                               : 1;
		u64 hfd                              : 1;
		u64 pause                            : 2;
		u64 reserved_9_11                    : 3;
		u64 rem_flt                          : 2;
		u64 reserved_14                      : 1;
		u64 np                               : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_anx_adv_s cn; */
};

static inline u64 CGXX_GMP_PCS_ANX_ADV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_ANX_ADV(u64 a)
{
	return 0x30010 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_an#_ext_st
 *
 * CGX GMO PCS Autonegotiation Extended Status Registers
 */
union cgxx_gmp_pcs_anx_ext_st {
	u64 u;
	struct cgxx_gmp_pcs_anx_ext_st_s {
		u64 reserved_0_11                    : 12;
		u64 thou_thd                         : 1;
		u64 thou_tfd                         : 1;
		u64 thou_xhd                         : 1;
		u64 thou_xfd                         : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_anx_ext_st_s cn; */
};

static inline u64 CGXX_GMP_PCS_ANX_EXT_ST(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_ANX_EXT_ST(u64 a)
{
	return 0x30028 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_an#_lp_abil
 *
 * CGX GMP PCS Autonegotiation Link Partner Ability Registers This is the
 * autonegotiation link partner ability register 5 as per IEEE 802.3,
 * Clause 37.
 */
union cgxx_gmp_pcs_anx_lp_abil {
	u64 u;
	struct cgxx_gmp_pcs_anx_lp_abil_s {
		u64 reserved_0_4                     : 5;
		u64 fd                               : 1;
		u64 hfd                              : 1;
		u64 pause                            : 2;
		u64 reserved_9_11                    : 3;
		u64 rem_flt                          : 2;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_anx_lp_abil_s cn; */
};

static inline u64 CGXX_GMP_PCS_ANX_LP_ABIL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_ANX_LP_ABIL(u64 a)
{
	return 0x30018 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_an#_results
 *
 * CGX GMP PCS Autonegotiation Results Registers This register is not
 * valid when CGX()_GMP_PCS_MISC()_CTL[AN_OVRD] is set to 1. If
 * CGX()_GMP_PCS_MISC()_CTL[AN_OVRD] is set to 0 and
 * CGX()_GMP_PCS_AN()_RESULTS[AN_CPT] is set to 1, this register is
 * valid.
 */
union cgxx_gmp_pcs_anx_results {
	u64 u;
	struct cgxx_gmp_pcs_anx_results_s {
		u64 link_ok                          : 1;
		u64 dup                              : 1;
		u64 an_cpt                           : 1;
		u64 spd                              : 2;
		u64 pause                            : 2;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_gmp_pcs_anx_results_s cn; */
};

static inline u64 CGXX_GMP_PCS_ANX_RESULTS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_ANX_RESULTS(u64 a)
{
	return 0x30020 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_int#
 *
 * CGX GMP PCS Interrupt Registers
 */
union cgxx_gmp_pcs_intx {
	u64 u;
	struct cgxx_gmp_pcs_intx_s {
		u64 lnkspd                           : 1;
		u64 xmit                             : 1;
		u64 an_err                           : 1;
		u64 txfifu                           : 1;
		u64 txfifo                           : 1;
		u64 txbad                            : 1;
		u64 rxerr                            : 1;
		u64 rxbad                            : 1;
		u64 rxlock                           : 1;
		u64 an_bad                           : 1;
		u64 sync_bad                         : 1;
		u64 dup                              : 1;
		u64 dbg_sync                         : 1;
		u64 reserved_13_15                   : 3;
		u64 an_page_received                 : 1;
		u64 an_complete                      : 1;
		u64 reserved_18_19                   : 2;
		u64 eee_tx_change                    : 1;
		u64 eee_rx_change                    : 1;
		u64 eee_rx_link_fail                 : 1;
		u64 reserved_23_63                   : 41;
	} s;
	/* struct cgxx_gmp_pcs_intx_s cn; */
};

static inline u64 CGXX_GMP_PCS_INTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_INTX(u64 a)
{
	return 0x30080 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_int#_ena_w1c
 *
 * CGX GMP PCS Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_gmp_pcs_intx_ena_w1c {
	u64 u;
	struct cgxx_gmp_pcs_intx_ena_w1c_s {
		u64 lnkspd                           : 1;
		u64 xmit                             : 1;
		u64 an_err                           : 1;
		u64 txfifu                           : 1;
		u64 txfifo                           : 1;
		u64 txbad                            : 1;
		u64 rxerr                            : 1;
		u64 rxbad                            : 1;
		u64 rxlock                           : 1;
		u64 an_bad                           : 1;
		u64 sync_bad                         : 1;
		u64 dup                              : 1;
		u64 dbg_sync                         : 1;
		u64 reserved_13_15                   : 3;
		u64 an_page_received                 : 1;
		u64 an_complete                      : 1;
		u64 reserved_18_19                   : 2;
		u64 eee_tx_change                    : 1;
		u64 eee_rx_change                    : 1;
		u64 eee_rx_link_fail                 : 1;
		u64 reserved_23_63                   : 41;
	} s;
	/* struct cgxx_gmp_pcs_intx_ena_w1c_s cn; */
};

static inline u64 CGXX_GMP_PCS_INTX_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_INTX_ENA_W1C(u64 a)
{
	return 0x30090 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_int#_ena_w1s
 *
 * CGX GMP PCS Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union cgxx_gmp_pcs_intx_ena_w1s {
	u64 u;
	struct cgxx_gmp_pcs_intx_ena_w1s_s {
		u64 lnkspd                           : 1;
		u64 xmit                             : 1;
		u64 an_err                           : 1;
		u64 txfifu                           : 1;
		u64 txfifo                           : 1;
		u64 txbad                            : 1;
		u64 rxerr                            : 1;
		u64 rxbad                            : 1;
		u64 rxlock                           : 1;
		u64 an_bad                           : 1;
		u64 sync_bad                         : 1;
		u64 dup                              : 1;
		u64 dbg_sync                         : 1;
		u64 reserved_13_15                   : 3;
		u64 an_page_received                 : 1;
		u64 an_complete                      : 1;
		u64 reserved_18_19                   : 2;
		u64 eee_tx_change                    : 1;
		u64 eee_rx_change                    : 1;
		u64 eee_rx_link_fail                 : 1;
		u64 reserved_23_63                   : 41;
	} s;
	/* struct cgxx_gmp_pcs_intx_ena_w1s_s cn; */
};

static inline u64 CGXX_GMP_PCS_INTX_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_INTX_ENA_W1S(u64 a)
{
	return 0x30098 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_int#_w1s
 *
 * CGX GMP PCS Interrupt Set Registers This register sets interrupt bits.
 */
union cgxx_gmp_pcs_intx_w1s {
	u64 u;
	struct cgxx_gmp_pcs_intx_w1s_s {
		u64 lnkspd                           : 1;
		u64 xmit                             : 1;
		u64 an_err                           : 1;
		u64 txfifu                           : 1;
		u64 txfifo                           : 1;
		u64 txbad                            : 1;
		u64 rxerr                            : 1;
		u64 rxbad                            : 1;
		u64 rxlock                           : 1;
		u64 an_bad                           : 1;
		u64 sync_bad                         : 1;
		u64 dup                              : 1;
		u64 dbg_sync                         : 1;
		u64 reserved_13_15                   : 3;
		u64 an_page_received                 : 1;
		u64 an_complete                      : 1;
		u64 reserved_18_19                   : 2;
		u64 eee_tx_change                    : 1;
		u64 eee_rx_change                    : 1;
		u64 eee_rx_link_fail                 : 1;
		u64 reserved_23_63                   : 41;
	} s;
	/* struct cgxx_gmp_pcs_intx_w1s_s cn; */
};

static inline u64 CGXX_GMP_PCS_INTX_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_INTX_W1S(u64 a)
{
	return 0x30088 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_link#_timer
 *
 * CGX GMP PCS Link Timer Registers This is the 1.6 ms nominal link timer
 * register.
 */
union cgxx_gmp_pcs_linkx_timer {
	u64 u;
	struct cgxx_gmp_pcs_linkx_timer_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_linkx_timer_s cn; */
};

static inline u64 CGXX_GMP_PCS_LINKX_TIMER(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_LINKX_TIMER(u64 a)
{
	return 0x30040 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_misc#_ctl
 *
 * CGX GMP SGMII Miscellaneous Control Registers Internal: SGMII bit [12]
 * is really a misnomer, it is a decode  of pi_qlm_cfg pins to indicate
 * SGMII or 1000Base-X modes.  Note: The SGMII AN Advertisement Register
 * above will be sent during Auto Negotiation if [MAC_PHY] is set (1=PHY
 * mode). If the bit is not set (0=MAC mode), the tx_Config_Reg\<14\>
 * becomes ACK bit and tx_Config_Reg\<0\> is always 1. All other bits in
 * tx_Config_Reg sent will be 0. The PHY dictates the Auto Negotiation
 * results.
 */
union cgxx_gmp_pcs_miscx_ctl {
	u64 u;
	struct cgxx_gmp_pcs_miscx_ctl_s {
		u64 samp_pt                          : 7;
		u64 an_ovrd                          : 1;
		u64 mode                             : 1;
		u64 mac_phy                          : 1;
		u64 loopbck2                         : 1;
		u64 gmxeno                           : 1;
		u64 reserved_12                      : 1;
		u64 disp_en                          : 1;
		u64 reserved_14_15                   : 2;
		u64 qsgmii_comma_wd                  : 16;
		u64 qsgmii_comma_wd_en               : 1;
		u64 reserved_33_63                   : 31;
	} s;
	struct cgxx_gmp_pcs_miscx_ctl_cn {
		u64 samp_pt                          : 7;
		u64 an_ovrd                          : 1;
		u64 mode                             : 1;
		u64 mac_phy                          : 1;
		u64 loopbck2                         : 1;
		u64 gmxeno                           : 1;
		u64 reserved_12                      : 1;
		u64 disp_en                          : 1;
		u64 reserved_14_15                   : 2;
		u64 qsgmii_comma_wd                  : 16;
		u64 qsgmii_comma_wd_en               : 1;
		u64 reserved_33_35                   : 3;
		u64 reserved_36_63                   : 28;
	} cn;
};

static inline u64 CGXX_GMP_PCS_MISCX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_MISCX_CTL(u64 a)
{
	return 0x30078 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_mr#_control
 *
 * CGX GMP PCS Control Registers
 */
union cgxx_gmp_pcs_mrx_control {
	u64 u;
	struct cgxx_gmp_pcs_mrx_control_s {
		u64 reserved_0_4                     : 5;
		u64 uni                              : 1;
		u64 spdmsb                           : 1;
		u64 coltst                           : 1;
		u64 dup                              : 1;
		u64 rst_an                           : 1;
		u64 reserved_10                      : 1;
		u64 pwr_dn                           : 1;
		u64 an_en                            : 1;
		u64 spdlsb                           : 1;
		u64 loopbck1                         : 1;
		u64 reset                            : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_mrx_control_s cn; */
};

static inline u64 CGXX_GMP_PCS_MRX_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_MRX_CONTROL(u64 a)
{
	return 0x30000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_mr#_status
 *
 * CGX GMP PCS Status Registers Bits \<15:9\> in this register indicate
 * the ability to operate when CGX()_GMP_PCS_MISC()_CTL[MAC_PHY] is set
 * to MAC mode. Bits \<15:9\> are always read as 0, indicating that the
 * chip cannot operate in the corresponding modes. The field [RM_FLT] is
 * a 'don't care' when the selected mode is SGMII/QSGMII.
 */
union cgxx_gmp_pcs_mrx_status {
	u64 u;
	struct cgxx_gmp_pcs_mrx_status_s {
		u64 extnd                            : 1;
		u64 reserved_1                       : 1;
		u64 lnk_st                           : 1;
		u64 an_abil                          : 1;
		u64 rm_flt                           : 1;
		u64 an_cpt                           : 1;
		u64 prb_sup                          : 1;
		u64 reserved_7                       : 1;
		u64 ext_st                           : 1;
		u64 hun_t2hd                         : 1;
		u64 hun_t2fd                         : 1;
		u64 ten_hd                           : 1;
		u64 ten_fd                           : 1;
		u64 hun_xhd                          : 1;
		u64 hun_xfd                          : 1;
		u64 hun_t4                           : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_mrx_status_s cn; */
};

static inline u64 CGXX_GMP_PCS_MRX_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_MRX_STATUS(u64 a)
{
	return 0x30008 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_rx#_states
 *
 * CGX GMP PCS RX State-Machines States Registers
 */
union cgxx_gmp_pcs_rxx_states {
	u64 u;
	struct cgxx_gmp_pcs_rxx_states_s {
		u64 an_st                            : 4;
		u64 an_bad                           : 1;
		u64 sync                             : 4;
		u64 sync_bad                         : 1;
		u64 rx_st                            : 5;
		u64 rx_bad                           : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_rxx_states_s cn; */
};

static inline u64 CGXX_GMP_PCS_RXX_STATES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_RXX_STATES(u64 a)
{
	return 0x30058 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_rx#_sync
 *
 * CGX GMP PCS Code Group Synchronization Registers
 */
union cgxx_gmp_pcs_rxx_sync {
	u64 u;
	struct cgxx_gmp_pcs_rxx_sync_s {
		u64 bit_lock                         : 1;
		u64 sync                             : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_gmp_pcs_rxx_sync_s cn; */
};

static inline u64 CGXX_GMP_PCS_RXX_SYNC(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_RXX_SYNC(u64 a)
{
	return 0x30050 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_sgm#_an_adv
 *
 * CGX GMP PCS SGMII Autonegotiation Advertisement Registers This is the
 * SGMII autonegotiation advertisement register (sent out as
 * tx_Config_Reg\<15:0\> as defined in IEEE 802.3 clause 37). This
 * register is sent during autonegotiation if
 * CGX()_GMP_PCS_MISC()_CTL[MAC_PHY] is set (1 = PHY mode). If the bit is
 * not set (0 = MAC mode), then tx_Config_Reg\<14\> becomes ACK bit and
 * tx_Config_Reg\<0\> is always 1. All other bits in tx_Config_Reg sent
 * will be 0. The PHY dictates the autonegotiation results.
 */
union cgxx_gmp_pcs_sgmx_an_adv {
	u64 u;
	struct cgxx_gmp_pcs_sgmx_an_adv_s {
		u64 one                              : 1;
		u64 reserved_1_9                     : 9;
		u64 speed                            : 2;
		u64 dup                              : 1;
		u64 reserved_13                      : 1;
		u64 ack                              : 1;
		u64 link                             : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_gmp_pcs_sgmx_an_adv_s cn; */
};

static inline u64 CGXX_GMP_PCS_SGMX_AN_ADV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_SGMX_AN_ADV(u64 a)
{
	return 0x30068 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_sgm#_lp_adv
 *
 * CGX GMP PCS SGMII Link-Partner-Advertisement Registers This is the
 * SGMII link partner advertisement register (received as
 * rx_Config_Reg\<15:0\> as defined in IEEE 802.3 clause 37).
 */
union cgxx_gmp_pcs_sgmx_lp_adv {
	u64 u;
	struct cgxx_gmp_pcs_sgmx_lp_adv_s {
		u64 one                              : 1;
		u64 reserved_1_9                     : 9;
		u64 speed                            : 2;
		u64 dup                              : 1;
		u64 reserved_13_14                   : 2;
		u64 link                             : 1;
		u64 reserved_16_63                   : 48;
	} s;
	struct cgxx_gmp_pcs_sgmx_lp_adv_cn {
		u64 one                              : 1;
		u64 reserved_1_9                     : 9;
		u64 speed                            : 2;
		u64 dup                              : 1;
		u64 reserved_13                      : 1;
		u64 reserved_14                      : 1;
		u64 link                             : 1;
		u64 reserved_16_63                   : 48;
	} cn;
};

static inline u64 CGXX_GMP_PCS_SGMX_LP_ADV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_SGMX_LP_ADV(u64 a)
{
	return 0x30070 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_tx#_states
 *
 * CGX GMP PCS TX State-Machines States Registers
 */
union cgxx_gmp_pcs_txx_states {
	u64 u;
	struct cgxx_gmp_pcs_txx_states_s {
		u64 ord_st                           : 4;
		u64 tx_bad                           : 1;
		u64 xmit                             : 2;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_gmp_pcs_txx_states_s cn; */
};

static inline u64 CGXX_GMP_PCS_TXX_STATES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_TXX_STATES(u64 a)
{
	return 0x30060 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_gmp_pcs_tx_rx#_polarity
 *
 * CGX GMP PCS TX/RX Polarity Registers
 * CGX()_GMP_PCS_TX_RX()_POLARITY[AUTORXPL] shows correct polarity needed
 * on the link receive path after code group synchronization is achieved.
 * When LMAC_TYPE=QSGMII, only lane 0 polarity data and settings are
 * relevant and settings for lanes 1, 2 and 3 are unused.
 */
union cgxx_gmp_pcs_tx_rxx_polarity {
	u64 u;
	struct cgxx_gmp_pcs_tx_rxx_polarity_s {
		u64 txplrt                           : 1;
		u64 rxplrt                           : 1;
		u64 autorxpl                         : 1;
		u64 rxovrd                           : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_gmp_pcs_tx_rxx_polarity_s cn; */
};

static inline u64 CGXX_GMP_PCS_TX_RXX_POLARITY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_GMP_PCS_TX_RXX_POLARITY(u64 a)
{
	return 0x30048 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_msix_pba#
 *
 * CGX MSI-X Pending Bit Array Registers This register is the MSI-X PBA
 * table, the bit number is indexed by the CGX_INT_VEC_E enumeration.
 */
union cgxx_msix_pbax {
	u64 u;
	struct cgxx_msix_pbax_s {
		u64 pend                             : 64;
	} s;
	/* struct cgxx_msix_pbax_s cn; */
};

static inline u64 CGXX_MSIX_PBAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_MSIX_PBAX(u64 a)
{
	return 0xf0000 + 8 * a;
}

/**
 * Register (RSL) cgx#_msix_vec#_addr
 *
 * CGX MSI-X Vector Table Address Registers This register is the MSI-X
 * vector table, indexed by the CGX_INT_VEC_E enumeration.
 */
union cgxx_msix_vecx_addr {
	u64 u;
	struct cgxx_msix_vecx_addr_s {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 51;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct cgxx_msix_vecx_addr_s cn; */
};

static inline u64 CGXX_MSIX_VECX_ADDR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_MSIX_VECX_ADDR(u64 a)
{
	return 0 + 0x10 * a;
}

/**
 * Register (RSL) cgx#_msix_vec#_ctl
 *
 * CGX MSI-X Vector Table Control and Data Registers This register is the
 * MSI-X vector table, indexed by the CGX_INT_VEC_E enumeration.
 */
union cgxx_msix_vecx_ctl {
	u64 u;
	struct cgxx_msix_vecx_ctl_s {
		u64 data                             : 32;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct cgxx_msix_vecx_ctl_s cn; */
};

static inline u64 CGXX_MSIX_VECX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_MSIX_VECX_CTL(u64 a)
{
	return 8 + 0x10 * a;
}

/**
 * Register (RSL) cgx#_smu#_bp_test
 *
 * INTERNAL: CGX SMU TX Backpressure Test Registers
 */
union cgxx_smux_bp_test {
	u64 u;
	struct cgxx_smux_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_47                   : 24;
		u64 enable                           : 4;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct cgxx_smux_bp_test_s cn; */
};

static inline u64 CGXX_SMUX_BP_TEST(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_BP_TEST(u64 a)
{
	return 0x20230 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_cbfc_ctl
 *
 * CGX SMU PFC Control Registers Internal: INTERNAL: XOFF for a specific
 * class/channel \<i\> is XOFF\<i\> = ([PHYS_EN]\<i\> & cmr_rx_phys_bp) |
 * ([LOGL_EN]\<i\> & cmr_rx_logl_xoff\<i\>).
 */
union cgxx_smux_cbfc_ctl {
	u64 u;
	struct cgxx_smux_cbfc_ctl_s {
		u64 rx_en                            : 1;
		u64 tx_en                            : 1;
		u64 drp_en                           : 1;
		u64 bck_en                           : 1;
		u64 reserved_4_31                    : 28;
		u64 logl_en                          : 16;
		u64 phys_en                          : 16;
	} s;
	/* struct cgxx_smux_cbfc_ctl_s cn; */
};

static inline u64 CGXX_SMUX_CBFC_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_CBFC_CTL(u64 a)
{
	return 0x20218 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_ctrl
 *
 * CGX SMU Control Registers
 */
union cgxx_smux_ctrl {
	u64 u;
	struct cgxx_smux_ctrl_s {
		u64 rx_idle                          : 1;
		u64 tx_idle                          : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_smux_ctrl_s cn; */
};

static inline u64 CGXX_SMUX_CTRL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_CTRL(u64 a)
{
	return 0x20200 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_ext_loopback
 *
 * CGX SMU External Loopback Registers In loopback mode, the IFG1+IFG2 of
 * local and remote parties must match exactly; otherwise loopback FIFO
 * will overrun: CGX()_SMU()_TX_INT[LB_OVRFLW].
 */
union cgxx_smux_ext_loopback {
	u64 u;
	struct cgxx_smux_ext_loopback_s {
		u64 thresh                           : 6;
		u64 reserved_6_7                     : 2;
		u64 depth                            : 6;
		u64 reserved_14_15                   : 2;
		u64 en                               : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct cgxx_smux_ext_loopback_s cn; */
};

static inline u64 CGXX_SMUX_EXT_LOOPBACK(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_EXT_LOOPBACK(u64 a)
{
	return 0x20208 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_hg2_control
 *
 * CGX SMU HiGig2 Control Registers HiGig2 TX- and RX-enable are normally
 * set together for HiGig2 messaging. Setting just the TX or RX bit
 * results in only the HG2 message transmit or receive capability.
 * Setting [PHYS_EN] and [LOGL_EN] to 1 allows link PAUSE or backpressure
 * to NIX as per the received HiGig2 message. Setting these fields to 0
 * disables link PAUSE and backpressure to NIX in response to received
 * messages.  CGX()_SMU()_TX_CTL[HG_EN] must be set (to enable HiGig)
 * whenever either [HG2TX_EN] or [HG2RX_EN] are set.
 * CGX()_SMU()_RX_UDD_SKP[LEN] must be set to 16 (to select HiGig2)
 * whenever either [HG2TX_EN] or [HG2RX_EN] are set.
 * CGX()_CMR_RX_OVR_BP[EN]\<0\> must be set and
 * CGX()_CMR_RX_OVR_BP[BP]\<0\> must be cleared to 0 (to forcibly disable
 * hardware-automatic 802.3 PAUSE packet generation) with the HiGig2
 * Protocol when [HG2TX_EN] = 0. (The HiGig2 protocol is indicated by
 * CGX()_SMU()_TX_CTL[HG_EN] = 1 and CGX()_SMU()_RX_UDD_SKP[LEN]=16.)
 * Hardware can only autogenerate backpressure via HiGig2 messages
 * (optionally, when [HG2TX_EN] = 1) with the HiGig2 protocol.
 */
union cgxx_smux_hg2_control {
	u64 u;
	struct cgxx_smux_hg2_control_s {
		u64 logl_en                          : 16;
		u64 phys_en                          : 1;
		u64 hg2rx_en                         : 1;
		u64 hg2tx_en                         : 1;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct cgxx_smux_hg2_control_s cn; */
};

static inline u64 CGXX_SMUX_HG2_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_HG2_CONTROL(u64 a)
{
	return 0x20210 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_mmsi_ctl_sta
 *
 * CGX SMU MAC Merge Service Interface (MMSI) Control/Status Registers
 * MMSI control and status registers for frame preemption mode. Refer to
 * IEEE 802.3br, Clause 99.
 */
union cgxx_smux_mmsi_ctl_sta {
	u64 u;
	struct cgxx_smux_mmsi_ctl_sta_s {
		u64 p_en                             : 1;
		u64 dis_v                            : 1;
		u64 afs                              : 2;
		u64 v_sta                            : 3;
		u64 tx_pactive                       : 1;
		u64 reserved_8_31                    : 24;
		u64 v_time                           : 24;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct cgxx_smux_mmsi_ctl_sta_s cn; */
};

static inline u64 CGXX_SMUX_MMSI_CTL_STA(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_MMSI_CTL_STA(u64 a)
{
	return 0x20220 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_bad_col_ctrl
 *
 * CGX SMU RX Bad Column High Registers
 */
union cgxx_smux_rx_bad_col_ctrl {
	u64 u;
	struct cgxx_smux_rx_bad_col_ctrl_s {
		u64 lane_rxc                         : 16;
		u64 state                            : 3;
		u64 val                              : 1;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct cgxx_smux_rx_bad_col_ctrl_s cn; */
};

static inline u64 CGXX_SMUX_RX_BAD_COL_CTRL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_BAD_COL_CTRL(u64 a)
{
	return 0x20060 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_bad_col_data_hi
 *
 * CGX SMU RX Bad Column Low Registers
 */
union cgxx_smux_rx_bad_col_data_hi {
	u64 u;
	struct cgxx_smux_rx_bad_col_data_hi_s {
		u64 lane_rxd                         : 64;
	} s;
	/* struct cgxx_smux_rx_bad_col_data_hi_s cn; */
};

static inline u64 CGXX_SMUX_RX_BAD_COL_DATA_HI(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_BAD_COL_DATA_HI(u64 a)
{
	return 0x20058 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_bad_col_data_lo
 *
 * CGX SMU RX Bad Column Low Registers
 */
union cgxx_smux_rx_bad_col_data_lo {
	u64 u;
	struct cgxx_smux_rx_bad_col_data_lo_s {
		u64 lane_rxd                         : 64;
	} s;
	/* struct cgxx_smux_rx_bad_col_data_lo_s cn; */
};

static inline u64 CGXX_SMUX_RX_BAD_COL_DATA_LO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_BAD_COL_DATA_LO(u64 a)
{
	return 0x20050 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_ctl
 *
 * CGX SMU RX Control Registers
 */
union cgxx_smux_rx_ctl {
	u64 u;
	struct cgxx_smux_rx_ctl_s {
		u64 status                           : 2;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_smux_rx_ctl_s cn; */
};

static inline u64 CGXX_SMUX_RX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_CTL(u64 a)
{
	return 0x20048 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_decision
 *
 * CGX SMU Packet Decision Registers This register specifies the byte
 * count used to determine when to accept or to filter a packet. As each
 * byte in a packet is received by CGX, the L2 byte count (i.e. the
 * number of bytes from the beginning of the L2 header (DMAC)) is
 * compared against CNT. In normal operation, the L2 header begins after
 * the PREAMBLE + SFD (CGX()_SMU()_RX_FRM_CTL[PRE_CHK] = 1) and any
 * optional UDD skip data (CGX()_SMU()_RX_UDD_SKP[LEN]).
 */
union cgxx_smux_rx_decision {
	u64 u;
	struct cgxx_smux_rx_decision_s {
		u64 cnt                              : 5;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct cgxx_smux_rx_decision_s cn; */
};

static inline u64 CGXX_SMUX_RX_DECISION(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_DECISION(u64 a)
{
	return 0x20038 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_frm_chk
 *
 * CGX SMU RX Frame Check Registers The CSRs provide the enable bits for
 * a subset of errors passed to CMR encoded.
 */
union cgxx_smux_rx_frm_chk {
	u64 u;
	struct cgxx_smux_rx_frm_chk_s {
		u64 reserved_0_2                     : 3;
		u64 jabber                           : 1;
		u64 fcserr_d                         : 1;
		u64 fcserr_c                         : 1;
		u64 reserved_6                       : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct cgxx_smux_rx_frm_chk_s cn; */
};

static inline u64 CGXX_SMUX_RX_FRM_CHK(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_FRM_CHK(u64 a)
{
	return 0x20028 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_frm_ctl
 *
 * CGX SMU RX Frame Control Registers This register controls the handling
 * of the frames. The [CTL_BCK] and [CTL_DRP] bits control how the
 * hardware handles incoming PAUSE packets. The most common modes of
 * operation: _ [CTL_BCK] = 1, [CTL_DRP] = 1: hardware handles everything
 * _ [CTL_BCK] = 0, [CTL_DRP] = 0: software sees all PAUSE frames _
 * [CTL_BCK] = 0, [CTL_DRP] = 1: all PAUSE frames are completely ignored
 * These control bits should be set to [CTL_BCK] = 0, [CTL_DRP] = 0 in
 * half-duplex mode. Since PAUSE packets only apply to full duplex
 * operation, any PAUSE packet would constitute an exception which should
 * be handled by the processing cores. PAUSE packets should not be
 * forwarded.
 */
union cgxx_smux_rx_frm_ctl {
	u64 u;
	struct cgxx_smux_rx_frm_ctl_s {
		u64 pre_chk                          : 1;
		u64 pre_strp                         : 1;
		u64 ctl_drp                          : 1;
		u64 ctl_bck                          : 1;
		u64 ctl_mcst                         : 1;
		u64 ctl_smac                         : 1;
		u64 reserved_6_11                    : 6;
		u64 ptp_mode                         : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct cgxx_smux_rx_frm_ctl_s cn; */
};

static inline u64 CGXX_SMUX_RX_FRM_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_FRM_CTL(u64 a)
{
	return 0x20020 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_int
 *
 * CGX SMU Receive Interrupt Registers SMU Interrupt Register. Internal:
 * Exception conditions \<9\> and \<4:0\> can also set the rcv/opcode in
 * the received packet's work queue entry. CGX()_SMU()_RX_FRM_CHK
 * provides a bit mask for configuring which conditions set the error.
 */
union cgxx_smux_rx_int {
	u64 u;
	struct cgxx_smux_rx_int_s {
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 loc_fault                        : 1;
		u64 rem_fault                        : 1;
		u64 bad_seq                          : 1;
		u64 bad_term                         : 1;
		u64 hg2fld                           : 1;
		u64 hg2cc                            : 1;
		u64 badver                           : 1;
		u64 badrsp                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_smux_rx_int_s cn; */
};

static inline u64 CGXX_SMUX_RX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_INT(u64 a)
{
	return 0x20000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_int_ena_w1c
 *
 * CGX SMU Receive Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_smux_rx_int_ena_w1c {
	u64 u;
	struct cgxx_smux_rx_int_ena_w1c_s {
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 loc_fault                        : 1;
		u64 rem_fault                        : 1;
		u64 bad_seq                          : 1;
		u64 bad_term                         : 1;
		u64 hg2fld                           : 1;
		u64 hg2cc                            : 1;
		u64 badver                           : 1;
		u64 badrsp                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_smux_rx_int_ena_w1c_s cn; */
};

static inline u64 CGXX_SMUX_RX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_INT_ENA_W1C(u64 a)
{
	return 0x20010 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_int_ena_w1s
 *
 * CGX SMU Receive Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union cgxx_smux_rx_int_ena_w1s {
	u64 u;
	struct cgxx_smux_rx_int_ena_w1s_s {
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 loc_fault                        : 1;
		u64 rem_fault                        : 1;
		u64 bad_seq                          : 1;
		u64 bad_term                         : 1;
		u64 hg2fld                           : 1;
		u64 hg2cc                            : 1;
		u64 badver                           : 1;
		u64 badrsp                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_smux_rx_int_ena_w1s_s cn; */
};

static inline u64 CGXX_SMUX_RX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_INT_ENA_W1S(u64 a)
{
	return 0x20018 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_int_w1s
 *
 * CGX SMU Receive Interrupt Set Registers This register sets interrupt
 * bits.
 */
union cgxx_smux_rx_int_w1s {
	u64 u;
	struct cgxx_smux_rx_int_w1s_s {
		u64 jabber                           : 1;
		u64 fcserr                           : 1;
		u64 rcverr                           : 1;
		u64 skperr                           : 1;
		u64 pcterr                           : 1;
		u64 rsverr                           : 1;
		u64 loc_fault                        : 1;
		u64 rem_fault                        : 1;
		u64 bad_seq                          : 1;
		u64 bad_term                         : 1;
		u64 hg2fld                           : 1;
		u64 hg2cc                            : 1;
		u64 badver                           : 1;
		u64 badrsp                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_smux_rx_int_w1s_s cn; */
};

static inline u64 CGXX_SMUX_RX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_INT_W1S(u64 a)
{
	return 0x20008 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_jabber
 *
 * CGX SMU Maximum Packet-Size Registers This register specifies the
 * maximum size for packets, beyond which the SMU truncates. Internal:
 * JABBER[CNT] is checked against the packet that arrives from SPU.  The
 * checking is performed before preamble is stripped or PTP is inserted.
 * If present, preamble is counted as eight bytes of the incoming packet.
 */
union cgxx_smux_rx_jabber {
	u64 u;
	struct cgxx_smux_rx_jabber_s {
		u64 cnt                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_rx_jabber_s cn; */
};

static inline u64 CGXX_SMUX_RX_JABBER(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_JABBER(u64 a)
{
	return 0x20030 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_udd_skp
 *
 * CGX SMU User-Defined Data Skip Registers Internal: (1) The skip bytes
 * are part of the packet and will be sent down the NCB packet interface
 * and will be handled by NIX.  (2) The system can determine if the UDD
 * bytes are included in the FCS check by using the FCSSEL field if the
 * FCS check is enabled.  (3) Assume that the preamble/sfd is always at
 * the start of the frame even before UDD bytes.  In most cases, there
 * will be no preamble in these cases since it will be packet interface
 * in direct communication to another packet interface (MAC to MAC)
 * without a PHY involved.  (4) We can still do address filtering and
 * control packet filtering if the user desires.  (5) In all cases, the
 * UDD bytes will be sent down the packet interface as part of the
 * packet.  The UDD bytes are never stripped from the actual packet.
 */
union cgxx_smux_rx_udd_skp {
	u64 u;
	struct cgxx_smux_rx_udd_skp_s {
		u64 len                              : 7;
		u64 reserved_7                       : 1;
		u64 fcssel                           : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct cgxx_smux_rx_udd_skp_s cn; */
};

static inline u64 CGXX_SMUX_RX_UDD_SKP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_UDD_SKP(u64 a)
{
	return 0x20040 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_ctrl0
 *
 * CGX SMU RX Wake-on-LAN Control 0 Registers
 */
union cgxx_smux_rx_wol_ctrl0 {
	u64 u;
	struct cgxx_smux_rx_wol_ctrl0_s {
		u64 dmac                             : 48;
		u64 pswd_len                         : 4;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct cgxx_smux_rx_wol_ctrl0_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_CTRL0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_CTRL0(u64 a)
{
	return 0x20068 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_ctrl1
 *
 * CGX SMU RX Wake-on-LAN Control 1 Registers
 */
union cgxx_smux_rx_wol_ctrl1 {
	u64 u;
	struct cgxx_smux_rx_wol_ctrl1_s {
		u64 pswd                             : 64;
	} s;
	/* struct cgxx_smux_rx_wol_ctrl1_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_CTRL1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_CTRL1(u64 a)
{
	return 0x20070 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_int
 *
 * CGX SMU RX WOL Interrupt Registers These registers allow WOL
 * interrupts to be sent to the control processor.
 */
union cgxx_smux_rx_wol_int {
	u64 u;
	struct cgxx_smux_rx_wol_int_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_smux_rx_wol_int_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_INT(u64 a)
{
	return 0x20078 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_int_ena_w1c
 *
 * CGX SMU RX WOL Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_smux_rx_wol_int_ena_w1c {
	u64 u;
	struct cgxx_smux_rx_wol_int_ena_w1c_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_smux_rx_wol_int_ena_w1c_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_INT_ENA_W1C(u64 a)
{
	return 0x20088 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_int_ena_w1s
 *
 * CGX SMU RX WOL Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union cgxx_smux_rx_wol_int_ena_w1s {
	u64 u;
	struct cgxx_smux_rx_wol_int_ena_w1s_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_smux_rx_wol_int_ena_w1s_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_INT_ENA_W1S(u64 a)
{
	return 0x20090 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_rx_wol_int_w1s
 *
 * CGX SMU RX WOL Interrupt Set Registers This register sets interrupt
 * bits.
 */
union cgxx_smux_rx_wol_int_w1s {
	u64 u;
	struct cgxx_smux_rx_wol_int_w1s_s {
		u64 wol_rcvd                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_smux_rx_wol_int_w1s_s cn; */
};

static inline u64 CGXX_SMUX_RX_WOL_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_RX_WOL_INT_W1S(u64 a)
{
	return 0x20080 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_smac
 *
 * CGX SMU SMAC Registers
 */
union cgxx_smux_smac {
	u64 u;
	struct cgxx_smux_smac_s {
		u64 smac                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_smux_smac_s cn; */
};

static inline u64 CGXX_SMUX_SMAC(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_SMAC(u64 a)
{
	return 0x20108 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_append
 *
 * CGX SMU TX Append Control Registers For more details on the
 * interactions between FCS and PAD, see also the description of
 * CGX()_SMU()_TX_MIN_PKT[MIN_SIZE].
 */
union cgxx_smux_tx_append {
	u64 u;
	struct cgxx_smux_tx_append_s {
		u64 preamble                         : 1;
		u64 pad                              : 1;
		u64 fcs_d                            : 1;
		u64 fcs_c                            : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_smux_tx_append_s cn; */
};

static inline u64 CGXX_SMUX_TX_APPEND(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_APPEND(u64 a)
{
	return 0x20100 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_ctl
 *
 * CGX SMU Transmit Control Registers
 */
union cgxx_smux_tx_ctl {
	u64 u;
	struct cgxx_smux_tx_ctl_s {
		u64 dic_en                           : 1;
		u64 uni_en                           : 1;
		u64 x4a_dis                          : 1;
		u64 mia_en                           : 1;
		u64 ls                               : 2;
		u64 ls_byp                           : 1;
		u64 l2p_bp_conv                      : 1;
		u64 hg_en                            : 1;
		u64 hg_pause_hgi                     : 2;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct cgxx_smux_tx_ctl_s cn; */
};

static inline u64 CGXX_SMUX_TX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_CTL(u64 a)
{
	return 0x20178 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_dack
 *
 * CGX SMU TX Drop Counters Registers
 */
union cgxx_smux_tx_dack {
	u64 u;
	struct cgxx_smux_tx_dack_s {
		u64 dpi_sdrop_ack                    : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_tx_dack_s cn; */
};

static inline u64 CGXX_SMUX_TX_DACK(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_DACK(u64 a)
{
	return 0x201b0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_dcnt
 *
 * CGX SMU TX Drop Counters Registers
 */
union cgxx_smux_tx_dcnt {
	u64 u;
	struct cgxx_smux_tx_dcnt_s {
		u64 dpi_sdrop_cnt                    : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_tx_dcnt_s cn; */
};

static inline u64 CGXX_SMUX_TX_DCNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_DCNT(u64 a)
{
	return 0x201a8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_eee
 *
 * INTERNAL: CGX SMU TX EEE Configure Registers  Resvered. Internal:
 * These registers control when SMU TX requests to enter or exist LPI.
 * Those registers take effect only when EEE is supported and enabled for
 * a given LMAC.
 */
union cgxx_smux_tx_eee {
	u64 u;
	struct cgxx_smux_tx_eee_s {
		u64 idle_thresh                      : 28;
		u64 reserved_28                      : 1;
		u64 force_lpi                        : 1;
		u64 wakeup                           : 1;
		u64 auto_lpi                         : 1;
		u64 idle_cnt                         : 28;
		u64 reserved_60_61                   : 2;
		u64 tx_lpi_wake                      : 1;
		u64 tx_lpi                           : 1;
	} s;
	/* struct cgxx_smux_tx_eee_s cn; */
};

static inline u64 CGXX_SMUX_TX_EEE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_EEE(u64 a)
{
	return 0x20190 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_eee_timer_status
 *
 * INTERNAL: CGX SMU TX EEE TIMER STATUS Registers  Reserved. Internal:
 * These registers configure SMU TX EEE timing parameters.
 */
union cgxx_smux_tx_eee_timer_status {
	u64 u;
	struct cgxx_smux_tx_eee_timer_status_s {
		u64 lpi_wake_cnt                     : 16;
		u64 reserved_16_30                   : 15;
		u64 wake_timer_done                  : 1;
		u64 link_ok_cnt                      : 30;
		u64 reserved_62                      : 1;
		u64 link_timer_done                  : 1;
	} s;
	/* struct cgxx_smux_tx_eee_timer_status_s cn; */
};

static inline u64 CGXX_SMUX_TX_EEE_TIMER_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_EEE_TIMER_STATUS(u64 a)
{
	return 0x201a0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_eee_timing
 *
 * INTERNAL: CGX SMU TX EEE TIMING Parameter Registers  Reserved.
 * Internal: These registers configure SMU TX EEE timing parameters.
 */
union cgxx_smux_tx_eee_timing {
	u64 u;
	struct cgxx_smux_tx_eee_timing_s {
		u64 w_sys_tx_min                     : 16;
		u64 reserved_16_31                   : 16;
		u64 link_ok_min                      : 30;
		u64 reserved_62_63                   : 2;
	} s;
	/* struct cgxx_smux_tx_eee_timing_s cn; */
};

static inline u64 CGXX_SMUX_TX_EEE_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_EEE_TIMING(u64 a)
{
	return 0x20198 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_ifg
 *
 * CGX SMU TX Interframe-Gap Cycles Registers Programming IFG1 and IFG2:
 * * For XAUI/RXAUI/10G/25G/40G/50G/100G systems that require IEEE 802.3
 * compatibility, the [IFG1]+[IFG2] sum must be 12. * In loopback mode,
 * the [IFG1]+[IFG2] of local and remote parties must match exactly;
 * otherwise loopback FIFO will overrun: CGX()_SMU()_TX_INT[LB_OVRFLW]. *
 * When CGX()_SMU()_TX_CTL[DIC_EN] is set, [IFG1]+[IFG2] sum must be at
 * least 8. The behavior of smaller values is un-determined. * When
 * CGX()_SMU()_TX_CTL[DIC_EN] is cleared, the minimum value of
 * [IFG1]+[IFG2] is 1 for 40G/50G/100G LMAC_TYPE configurations and 5 for
 * all other values. The behavior of smaller values is un-determined.
 * Internal: When CGX()_SMU()_TX_CTL[DIC_EN] is set, SMU TX treats
 * ([IFG1]+[IFG2]) \< 8 as 8 for 40G/50G/100G MACs and ([IFG1]+[IFG2]) \<
 * 8 as 8 for other MACs. When CGX()_SMU()_TX_CTL[DIC_EN] is cleared, SMU
 * TX can work correctly with any IFG1 and IFG2.
 */
union cgxx_smux_tx_ifg {
	u64 u;
	struct cgxx_smux_tx_ifg_s {
		u64 ifg1                             : 4;
		u64 ifg2                             : 4;
		u64 mia_amt                          : 2;
		u64 reserved_10_15                   : 6;
		u64 mia_cnt                          : 8;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct cgxx_smux_tx_ifg_s cn; */
};

static inline u64 CGXX_SMUX_TX_IFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_IFG(u64 a)
{
	return 0x20160 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_int
 *
 * CGX SMU TX Interrupt Registers
 */
union cgxx_smux_tx_int {
	u64 u;
	struct cgxx_smux_tx_int_s {
		u64 undflw                           : 1;
		u64 xchange                          : 1;
		u64 fake_commit                      : 1;
		u64 lb_undflw                        : 1;
		u64 lb_ovrflw                        : 1;
		u64 dpi_sdrop                        : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_smux_tx_int_s cn; */
};

static inline u64 CGXX_SMUX_TX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_INT(u64 a)
{
	return 0x20140 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_int_ena_w1c
 *
 * CGX SMU TX Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_smux_tx_int_ena_w1c {
	u64 u;
	struct cgxx_smux_tx_int_ena_w1c_s {
		u64 undflw                           : 1;
		u64 xchange                          : 1;
		u64 fake_commit                      : 1;
		u64 lb_undflw                        : 1;
		u64 lb_ovrflw                        : 1;
		u64 dpi_sdrop                        : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_smux_tx_int_ena_w1c_s cn; */
};

static inline u64 CGXX_SMUX_TX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_INT_ENA_W1C(u64 a)
{
	return 0x20150 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_int_ena_w1s
 *
 * CGX SMU TX Interrupt Enable Set Registers This register sets interrupt
 * enable bits.
 */
union cgxx_smux_tx_int_ena_w1s {
	u64 u;
	struct cgxx_smux_tx_int_ena_w1s_s {
		u64 undflw                           : 1;
		u64 xchange                          : 1;
		u64 fake_commit                      : 1;
		u64 lb_undflw                        : 1;
		u64 lb_ovrflw                        : 1;
		u64 dpi_sdrop                        : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_smux_tx_int_ena_w1s_s cn; */
};

static inline u64 CGXX_SMUX_TX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_INT_ENA_W1S(u64 a)
{
	return 0x20158 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_int_w1s
 *
 * CGX SMU TX Interrupt Set Registers This register sets interrupt bits.
 */
union cgxx_smux_tx_int_w1s {
	u64 u;
	struct cgxx_smux_tx_int_w1s_s {
		u64 undflw                           : 1;
		u64 xchange                          : 1;
		u64 fake_commit                      : 1;
		u64 lb_undflw                        : 1;
		u64 lb_ovrflw                        : 1;
		u64 dpi_sdrop                        : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_smux_tx_int_w1s_s cn; */
};

static inline u64 CGXX_SMUX_TX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_INT_W1S(u64 a)
{
	return 0x20148 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_min_pkt
 *
 * CGX SMU TX Minimum-Size-Packet Registers Internal: [MIN_SIZE] less
 * than 16 will be ignored by hardware which will use 16 instead.
 */
union cgxx_smux_tx_min_pkt {
	u64 u;
	struct cgxx_smux_tx_min_pkt_s {
		u64 min_size                         : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct cgxx_smux_tx_min_pkt_s cn; */
};

static inline u64 CGXX_SMUX_TX_MIN_PKT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_MIN_PKT(u64 a)
{
	return 0x20118 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_pkt_dmac
 *
 * CGX SMU TX PAUSE-Packet DMAC-Field Registers This register provides
 * the DMAC value that is placed in outbound PAUSE packets.
 */
union cgxx_smux_tx_pause_pkt_dmac {
	u64 u;
	struct cgxx_smux_tx_pause_pkt_dmac_s {
		u64 dmac                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_smux_tx_pause_pkt_dmac_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_PKT_DMAC(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_PKT_DMAC(u64 a)
{
	return 0x20168 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_pkt_interval
 *
 * CGX SMU TX PAUSE-Packet Transmission-Interval Registers This register
 * specifies how often PAUSE packets are sent.
 */
union cgxx_smux_tx_pause_pkt_interval {
	u64 u;
	struct cgxx_smux_tx_pause_pkt_interval_s {
		u64 interval                         : 16;
		u64 hg2_intra_interval               : 16;
		u64 hg2_intra_en                     : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct cgxx_smux_tx_pause_pkt_interval_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_PKT_INTERVAL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_PKT_INTERVAL(u64 a)
{
	return 0x20120 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_pkt_time
 *
 * CGX SMU TX PAUSE Packet Time Registers
 */
union cgxx_smux_tx_pause_pkt_time {
	u64 u;
	struct cgxx_smux_tx_pause_pkt_time_s {
		u64 p_time                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_tx_pause_pkt_time_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_PKT_TIME(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_PKT_TIME(u64 a)
{
	return 0x20110 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_pkt_type
 *
 * CGX SMU TX PAUSE-Packet P_TYPE-Field Registers This register provides
 * the P_TYPE field that is placed in outbound PAUSE packets.
 */
union cgxx_smux_tx_pause_pkt_type {
	u64 u;
	struct cgxx_smux_tx_pause_pkt_type_s {
		u64 p_type                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_tx_pause_pkt_type_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_PKT_TYPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_PKT_TYPE(u64 a)
{
	return 0x20170 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_togo
 *
 * CGX SMU TX Time-to-Backpressure Registers
 */
union cgxx_smux_tx_pause_togo {
	u64 u;
	struct cgxx_smux_tx_pause_togo_s {
		u64 p_time                           : 16;
		u64 msg_time                         : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_smux_tx_pause_togo_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_TOGO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_TOGO(u64 a)
{
	return 0x20130 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_pause_zero
 *
 * CGX SMU TX PAUSE Zero Registers
 */
union cgxx_smux_tx_pause_zero {
	u64 u;
	struct cgxx_smux_tx_pause_zero_s {
		u64 send                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_smux_tx_pause_zero_s cn; */
};

static inline u64 CGXX_SMUX_TX_PAUSE_ZERO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_PAUSE_ZERO(u64 a)
{
	return 0x20138 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_soft_pause
 *
 * CGX SMU TX Soft PAUSE Registers
 */
union cgxx_smux_tx_soft_pause {
	u64 u;
	struct cgxx_smux_tx_soft_pause_s {
		u64 p_time                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_smux_tx_soft_pause_s cn; */
};

static inline u64 CGXX_SMUX_TX_SOFT_PAUSE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_SOFT_PAUSE(u64 a)
{
	return 0x20128 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_smu#_tx_thresh
 *
 * CGX SMU TX Threshold Registers
 */
union cgxx_smux_tx_thresh {
	u64 u;
	struct cgxx_smux_tx_thresh_s {
		u64 cnt                              : 12;
		u64 reserved_12_15                   : 4;
		u64 dpi_thresh                       : 5;
		u64 reserved_21_23                   : 3;
		u64 dpi_depth                        : 5;
		u64 reserved_29_31                   : 3;
		u64 ecnt                             : 12;
		u64 reserved_44_63                   : 20;
	} s;
	/* struct cgxx_smux_tx_thresh_s cn; */
};

static inline u64 CGXX_SMUX_TX_THRESH(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SMUX_TX_THRESH(u64 a)
{
	return 0x20180 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_adv
 *
 * CGX SPU Autonegotiation Advertisement Registers Software programs this
 * register with the contents of the AN-link code word base page to be
 * transmitted during autonegotiation. (See IEEE 802.3 section 73.6 for
 * details.) Any write operations to this register prior to completion of
 * autonegotiation, as indicated by CGX()_SPU()_AN_STATUS[AN_COMPLETE],
 * should be followed by a renegotiation in order for the new values to
 * take effect. Renegotiation is initiated by setting
 * CGX()_SPU()_AN_CONTROL[AN_RESTART]. Once autonegotiation has
 * completed, software can examine this register along with
 * CGX()_SPU()_AN_LP_BASE to determine the highest common denominator
 * technology.
 */
union cgxx_spux_an_adv {
	u64 u;
	struct cgxx_spux_an_adv_s {
		u64 s                                : 5;
		u64 e                                : 5;
		u64 pause                            : 1;
		u64 asm_dir                          : 1;
		u64 xnp_able                         : 1;
		u64 rf                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 t                                : 5;
		u64 a1g_kx                           : 1;
		u64 a10g_kx4                         : 1;
		u64 a10g_kr                          : 1;
		u64 a40g_kr4                         : 1;
		u64 a40g_cr4                         : 1;
		u64 a100g_cr10                       : 1;
		u64 a100g_kp4                        : 1;
		u64 a100g_kr4                        : 1;
		u64 a100g_cr4                        : 1;
		u64 a25g_krs_crs                     : 1;
		u64 a25g_kr_cr                       : 1;
		u64 arsv                             : 12;
		u64 a25g_rs_fec_req                  : 1;
		u64 a25g_br_fec_req                  : 1;
		u64 fec_able                         : 1;
		u64 fec_req                          : 1;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_spux_an_adv_s cn; */
};

static inline u64 CGXX_SPUX_AN_ADV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_ADV(u64 a)
{
	return 0x10198 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_bp_status
 *
 * CGX SPU Autonegotiation Backplane Ethernet & BASE-R Copper Status
 * Registers The contents of this register are updated during
 * autonegotiation and are valid when CGX()_SPU()_AN_STATUS[AN_COMPLETE]
 * is set. At that time, one of the port type bits will be set depending
 * on the AN priority resolution. The port types are listed in order of
 * decreasing priority. If a BASE-R type is negotiated then [FEC] or
 * [RS_FEC] will be set to indicate whether/which FEC operation has been
 * negotiated and will be clear otherwise.
 */
union cgxx_spux_an_bp_status {
	u64 u;
	struct cgxx_spux_an_bp_status_s {
		u64 bp_an_able                       : 1;
		u64 n1g_kx                           : 1;
		u64 n10g_kx4                         : 1;
		u64 n10g_kr                          : 1;
		u64 n25g_kr1                         : 1;
		u64 n25g_cr1                         : 1;
		u64 n25g_krs_crs                     : 1;
		u64 n25g_kr_cr                       : 1;
		u64 n40g_kr4                         : 1;
		u64 n40g_cr4                         : 1;
		u64 n50g_kr2                         : 1;
		u64 n50g_cr2                         : 1;
		u64 n100g_cr10                       : 1;
		u64 n100g_kp4                        : 1;
		u64 n100g_kr4                        : 1;
		u64 n100g_cr4                        : 1;
		u64 fec                              : 1;
		u64 rs_fec                           : 1;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct cgxx_spux_an_bp_status_s cn; */
};

static inline u64 CGXX_SPUX_AN_BP_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_BP_STATUS(u64 a)
{
	return 0x101b8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_control
 *
 * CGX SPU Autonegotiation Control Registers
 */
union cgxx_spux_an_control {
	u64 u;
	struct cgxx_spux_an_control_s {
		u64 reserved_0_8                     : 9;
		u64 an_restart                       : 1;
		u64 reserved_10_11                   : 2;
		u64 an_en                            : 1;
		u64 xnp_en                           : 1;
		u64 reserved_14                      : 1;
		u64 an_reset                         : 1;
		u64 an_arb_link_chk_en               : 1;
		u64 usx_an_arb_link_chk_en           : 1;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct cgxx_spux_an_control_s cn; */
};

static inline u64 CGXX_SPUX_AN_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_CONTROL(u64 a)
{
	return 0x10188 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_lp_base
 *
 * CGX SPU Autonegotiation Link-Partner Base-Page Ability Registers This
 * register captures the contents of the latest AN link code word base
 * page received from the link partner during autonegotiation. (See IEEE
 * 802.3 section 73.6 for details.) CGX()_SPU()_AN_STATUS[PAGE_RX] is set
 * when this register is updated by hardware.
 */
union cgxx_spux_an_lp_base {
	u64 u;
	struct cgxx_spux_an_lp_base_s {
		u64 s                                : 5;
		u64 e                                : 5;
		u64 pause                            : 1;
		u64 asm_dir                          : 1;
		u64 xnp_able                         : 1;
		u64 rf                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 t                                : 5;
		u64 a1g_kx                           : 1;
		u64 a10g_kx4                         : 1;
		u64 a10g_kr                          : 1;
		u64 a40g_kr4                         : 1;
		u64 a40g_cr4                         : 1;
		u64 a100g_cr10                       : 1;
		u64 a100g_kp4                        : 1;
		u64 a100g_kr4                        : 1;
		u64 a100g_cr4                        : 1;
		u64 a25g_krs_crs                     : 1;
		u64 a25g_kr_cr                       : 1;
		u64 arsv                             : 12;
		u64 a25g_rs_fec_req                  : 1;
		u64 a25g_br_fec_req                  : 1;
		u64 fec_able                         : 1;
		u64 fec_req                          : 1;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_spux_an_lp_base_s cn; */
};

static inline u64 CGXX_SPUX_AN_LP_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_LP_BASE(u64 a)
{
	return 0x101a0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_lp_xnp
 *
 * CGX SPU Autonegotiation Link Partner Extended Next Page Ability
 * Registers This register captures the contents of the latest next page
 * code word received from the link partner during autonegotiation, if
 * any. See IEEE 802.3 section 73.7.7 for details.
 */
union cgxx_spux_an_lp_xnp {
	u64 u;
	struct cgxx_spux_an_lp_xnp_s {
		u64 m_u                              : 11;
		u64 toggle                           : 1;
		u64 ack2                             : 1;
		u64 mp                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 u                                : 32;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_spux_an_lp_xnp_s cn; */
};

static inline u64 CGXX_SPUX_AN_LP_XNP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_LP_XNP(u64 a)
{
	return 0x101b0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_status
 *
 * CGX SPU Autonegotiation Status Registers
 */
union cgxx_spux_an_status {
	u64 u;
	struct cgxx_spux_an_status_s {
		u64 lp_an_able                       : 1;
		u64 reserved_1                       : 1;
		u64 link_status                      : 1;
		u64 an_able                          : 1;
		u64 rmt_flt                          : 1;
		u64 an_complete                      : 1;
		u64 page_rx                          : 1;
		u64 xnp_stat                         : 1;
		u64 reserved_8                       : 1;
		u64 prl_flt                          : 1;
		u64 reserved_10_63                   : 54;
	} s;
	/* struct cgxx_spux_an_status_s cn; */
};

static inline u64 CGXX_SPUX_AN_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_STATUS(u64 a)
{
	return 0x10190 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_an_xnp_tx
 *
 * CGX SPU Autonegotiation Extended Next Page Transmit Registers Software
 * programs this register with the contents of the AN message next page
 * or unformatted next page link code word to be transmitted during
 * autonegotiation. Next page exchange occurs after the base link code
 * words have been exchanged if either end of the link segment sets the
 * NP bit to 1, indicating that it has at least one next page to send.
 * Once initiated, next page exchange continues until both ends of the
 * link segment set their NP bits to 0. See IEEE 802.3 section 73.7.7 for
 * details.
 */
union cgxx_spux_an_xnp_tx {
	u64 u;
	struct cgxx_spux_an_xnp_tx_s {
		u64 m_u                              : 11;
		u64 toggle                           : 1;
		u64 ack2                             : 1;
		u64 mp                               : 1;
		u64 ack                              : 1;
		u64 np                               : 1;
		u64 u                                : 32;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct cgxx_spux_an_xnp_tx_s cn; */
};

static inline u64 CGXX_SPUX_AN_XNP_TX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_AN_XNP_TX(u64 a)
{
	return 0x101a8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_algn_status
 *
 * CGX SPU Multilane BASE-R PCS Alignment-Status Registers This register
 * implements the IEEE 802.3 multilane BASE-R PCS alignment status 1-4
 * registers (3.50-3.53). It is valid only when the LPCS type is
 * 40GBASE-R, 50GBASE-R, 100GBASE-R, (CGX()_CMR()_CONFIG[LMAC_TYPE] =
 * CGX_LMAC_TYPES_E::FORTYG_R,FIFTYG_R,HUNDREDG_R), and always returns
 * 0x0 for all other LPCS types. Service interfaces (lanes) 19-0 (100G)
 * and 3-0 (all others) are mapped to PCS lanes 19-0 or 3-0 via
 * CGX()_SPU()_BR_LANE_MAP()[LN_MAPPING]. For 100G, logical lane 0 fans
 * out to service interfaces 0-4, logical lane 1 fans out to service
 * interfaces 5-9, ... etc. For all other modes, logical lanes and
 * service interfaces are identical. Logical interfaces (lanes) map to
 * SerDes lanes via CGX()_CMR()_CONFIG[LANE_TO_SDS] (programmable).
 */
union cgxx_spux_br_algn_status {
	u64 u;
	struct cgxx_spux_br_algn_status_s {
		u64 block_lock                       : 20;
		u64 reserved_20_29                   : 10;
		u64 alignd                           : 1;
		u64 reserved_31_40                   : 10;
		u64 marker_lock                      : 20;
		u64 reserved_61_63                   : 3;
	} s;
	/* struct cgxx_spux_br_algn_status_s cn; */
};

static inline u64 CGXX_SPUX_BR_ALGN_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_ALGN_STATUS(u64 a)
{
	return 0x10050 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_lane_map#
 *
 * CGX SPU 40,50,100GBASE-R Lane-Mapping Registers This register
 * implements the IEEE 802.3 lane 0-19 mapping registers (3.400-3.403).
 * It is valid only when the LPCS type is 40GBASE-R, 50GBASE-R,
 * 100GBASE-R, USXGMII (CGX()_CMR()_CONFIG[LMAC_TYPE]), and always
 * returns 0x0 for all other LPCS types. The LNx_MAPPING field for each
 * programmed PCS lane (called service interface in 802.3) is valid when
 * that lane has achieved alignment marker lock on the receive side (i.e.
 * the associated CGX()_SPU()_BR_ALGN_STATUS[MARKER_LOCK] = 1), and is
 * invalid otherwise. When valid, it returns the actual detected receive
 * PCS lane number based on the received alignment marker contents
 * received on that service interface.  In RS-FEC mode the LNx_MAPPING
 * field is valid when that lane has achieved alignment marker lock on
 * the receive side (i.e. the associated
 * CGX()_SPU()_RSFEC_STATUS[AMPS_LOCK] = 1), and is invalid otherwise.
 * When valid, it returns the actual detected receive FEC lane number
 * based on the received alignment marker contents received on that
 * logical lane therefore expect for RS-FEC that LNx_MAPPING = x.  The
 * mapping is flexible because IEEE 802.3 allows multilane BASE-R receive
 * lanes to be re-ordered. Note that for the transmit side, each logical
 * lane is mapped to a physical SerDes lane based on the programming of
 * CGX()_CMR()_CONFIG[LANE_TO_SDS]. For the receive side,
 * CGX()_CMR()_CONFIG[LANE_TO_SDS] specifies the logical lane to physical
 * SerDes lane mapping, and this register specifies the service interface
 * (or lane) to PCS lane mapping.
 */
union cgxx_spux_br_lane_mapx {
	u64 u;
	struct cgxx_spux_br_lane_mapx_s {
		u64 ln_mapping                       : 6;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_spux_br_lane_mapx_s cn; */
};

static inline u64 CGXX_SPUX_BR_LANE_MAPX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_LANE_MAPX(u64 a, u64 b)
{
	return 0x10600 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_control
 *
 * CGX SPU BASE-R PMD Control Registers
 */
union cgxx_spux_br_pmd_control {
	u64 u;
	struct cgxx_spux_br_pmd_control_s {
		u64 train_restart                    : 1;
		u64 train_en                         : 1;
		u64 use_lane_poly                    : 1;
		u64 max_wait_disable                 : 1;
		u64 reserved_4_63                    : 60;
	} s;
	struct cgxx_spux_br_pmd_control_cn96xx {
		u64 train_restart                    : 1;
		u64 train_en                         : 1;
		u64 use_lane_poly                    : 1;
		u64 reserved_3_63                    : 61;
	} cn96xx;
	/* struct cgxx_spux_br_pmd_control_s cnf95xxp1; */
	/* struct cgxx_spux_br_pmd_control_cn96xx cnf95xxp2; */
};

static inline u64 CGXX_SPUX_BR_PMD_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_CONTROL(u64 a)
{
	return 0x100a8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_ld_cup
 *
 * INTERNAL:CGX SPU BASE-R PMD Local Device Coefficient Update Registers
 * This register implements MDIO register 1.154 of 802.3-2012 Section 5
 * CL45 for 10GBASE-R and and of 802.3by-2016 CL45 for 25GBASE-R. Note
 * that for 10G, 25G LN0_ only is used.  It implements  MDIO registers
 * 1.1300-1.1303 for all other BASE-R modes (40G, 50G, 100G) per
 * 802.3bj-2014 CL45. Note that for 50G LN0_ and LN1_ only are used.  The
 * fields in this register are read/write even though they are specified
 * as read-only in 802.3.  The register is automatically cleared at the
 * start of training. When link training is in progress, each field
 * reflects the contents of the coefficient update field in the
 * associated lane's outgoing training frame.  If
 * CGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set, then this register
 * must be updated by software during link training and hardware updates
 * are disabled. If CGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is clear,
 * this register is automatically updated by hardware, and it should not
 * be written by software. The lane fields in this register are indexed
 * by logical PCS lane ID.
 */
union cgxx_spux_br_pmd_ld_cup {
	u64 u;
	struct cgxx_spux_br_pmd_ld_cup_s {
		u64 ln0_cup                          : 16;
		u64 ln1_cup                          : 16;
		u64 ln2_cup                          : 16;
		u64 ln3_cup                          : 16;
	} s;
	/* struct cgxx_spux_br_pmd_ld_cup_s cn; */
};

static inline u64 CGXX_SPUX_BR_PMD_LD_CUP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_LD_CUP(u64 a)
{
	return 0x100c8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_ld_rep
 *
 * INTERNAL:CGX SPU BASE-R PMD Local Device Status Report Registers  This
 * register implements MDIO register 1.155 of 802.3-2012 Section 5 CL45
 * for 10GBASE-R and and of 802.3by-2016 CL45 for 25GBASE-R. Note that
 * for 10G, 25G LN0_ only is used.  It implements  MDIO registers
 * 1.1400-1.1403 for all other BASE-R modes (40G, 50G, 100G) per
 * 802.3bj-2014 CL45. Note that for 50G LN0_ and LN1_ only are used.  The
 * fields in this register are read/write even though they are specified
 * as read-only in 802.3.  The register is automatically cleared at the
 * start of training. Each field reflects the contents of the status
 * report field in the associated lane's outgoing training frame.  If
 * CGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set, then this register
 * must be updated by software during link training and hardware updates
 * are disabled. If CGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is clear,
 * this register is automatically updated by hardware, and it should not
 * be written by software. The lane fields in this register are indexed
 * by logical PCS lane ID.
 */
union cgxx_spux_br_pmd_ld_rep {
	u64 u;
	struct cgxx_spux_br_pmd_ld_rep_s {
		u64 ln0_rep                          : 16;
		u64 ln1_rep                          : 16;
		u64 ln2_rep                          : 16;
		u64 ln3_rep                          : 16;
	} s;
	/* struct cgxx_spux_br_pmd_ld_rep_s cn; */
};

static inline u64 CGXX_SPUX_BR_PMD_LD_REP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_LD_REP(u64 a)
{
	return 0x100d0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_lp_cup
 *
 * INTERNAL:CGX SPU BASE-R PMD Link Partner Coefficient Update Registers
 * This register implements MDIO register 1.152 of 802.3-2012 Section 5
 * CL45 for 10GBASE-R and and of 802.3by-2016 CL45 for 25GBASE-R. Note
 * that for 10G, 25G LN0_ only is used.  It implements  MDIO registers
 * 1.1100-1.1103 for all other BASE-R modes (40G, 50G, 100G) per
 * 802.3bj-2014 CL45. Note that for 50G LN0_ and LN1_ only are used.  The
 * register is automatically cleared at the start of training. Each field
 * reflects the contents of the coefficient update field in the lane's
 * most recently received training frame. This register should not be
 * written when link training is enabled, i.e. when
 * CGX()_SPU()_BR_PMD_CONTROL[TRAIN_EN] is set. The lane fields in this
 * register are indexed by logical PCS lane ID.
 */
union cgxx_spux_br_pmd_lp_cup {
	u64 u;
	struct cgxx_spux_br_pmd_lp_cup_s {
		u64 ln0_cup                          : 16;
		u64 ln1_cup                          : 16;
		u64 ln2_cup                          : 16;
		u64 ln3_cup                          : 16;
	} s;
	/* struct cgxx_spux_br_pmd_lp_cup_s cn; */
};

static inline u64 CGXX_SPUX_BR_PMD_LP_CUP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_LP_CUP(u64 a)
{
	return 0x100b8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_lp_rep
 *
 * INTERNAL:CGX SPU BASE-R PMD Link Partner Status Report Registers  This
 * register implements MDIO register 1.153 of 802.3-2012 Section 5 CL45
 * for 10GBASE-R and and of 802.3by-2016 CL45 for 25GBASE-R. Note that
 * for 10G, 25G LN0_ only is used.  It implements  MDIO registers
 * 1.1200-1.1203 for all other BASE-R modes (40G, 50G, 100G) per
 * 802.3bj-2014 CL45. Note that for 50G LN0_ and LN1_ only are used.  The
 * register is automatically cleared at the start of training. Each field
 * reflects the contents of the coefficient update field in the lane's
 * most recently received training frame. This register should not be
 * written when link training is enabled, i.e. when
 * CGX()_SPU()_BR_PMD_CONTROL[TRAIN_EN] is set. The lane fields in this
 * register are indexed by logical PCS lane ID.
 */
union cgxx_spux_br_pmd_lp_rep {
	u64 u;
	struct cgxx_spux_br_pmd_lp_rep_s {
		u64 ln0_rep                          : 16;
		u64 ln1_rep                          : 16;
		u64 ln2_rep                          : 16;
		u64 ln3_rep                          : 16;
	} s;
	/* struct cgxx_spux_br_pmd_lp_rep_s cn; */
};

static inline u64 CGXX_SPUX_BR_PMD_LP_REP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_LP_REP(u64 a)
{
	return 0x100c0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_pmd_status
 *
 * INTERNAL:CGX SPU BASE-R PMD Status Registers  The lane fields in this
 * register are indexed by logical PCS lane ID. The lane 0 field (LN0_*)
 * is valid for 10GBASE-R, 25GBASE-R, 40GBASE-R, 50GBASE-R and
 * 100GBASE-R. The lane 1 field (LN1_*) is valid for 40GBASE-R, 50GBASE-R
 * and 100GBASE-R. The remaining fields (LN2_*, LN3_*) are only valid for
 * 40GBASE-R and 100GBASE-R.
 */
union cgxx_spux_br_pmd_status {
	u64 u;
	struct cgxx_spux_br_pmd_status_s {
		u64 ln0_train_status                 : 4;
		u64 ln1_train_status                 : 4;
		u64 ln2_train_status                 : 4;
		u64 ln3_train_status                 : 4;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_br_pmd_status_s cn; */
};

static inline u64 CGXX_SPUX_BR_PMD_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_PMD_STATUS(u64 a)
{
	return 0x100b0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_status1
 *
 * CGX SPU BASE-R Status 1 Registers
 */
union cgxx_spux_br_status1 {
	u64 u;
	struct cgxx_spux_br_status1_s {
		u64 blk_lock                         : 1;
		u64 hi_ber                           : 1;
		u64 prbs31                           : 1;
		u64 prbs9                            : 1;
		u64 reserved_4_11                    : 8;
		u64 rcv_lnk                          : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct cgxx_spux_br_status1_s cn; */
};

static inline u64 CGXX_SPUX_BR_STATUS1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_STATUS1(u64 a)
{
	return 0x10030 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_status2
 *
 * CGX SPU BASE-R Status 2 Registers This register implements a
 * combination of the following IEEE 802.3 registers: * BASE-R PCS status
 * 2 (MDIO address 3.33). * BASE-R BER high-order counter (MDIO address
 * 3.44). * Errored-blocks high-order counter (MDIO address 3.45).  Note
 * that the relative locations of some fields have been moved from IEEE
 * 802.3 in order to make the register layout more software friendly: the
 * BER counter high-order and low-order bits from sections 3.44 and 3.33
 * have been combined into the contiguous, 22-bit [BER_CNT] field;
 * likewise, the errored-blocks counter high-order and low-order bits
 * from section 3.45 have been combined into the contiguous, 22-bit
 * [ERR_BLKS] field.
 */
union cgxx_spux_br_status2 {
	u64 u;
	struct cgxx_spux_br_status2_s {
		u64 reserved_0_13                    : 14;
		u64 latched_ber                      : 1;
		u64 latched_lock                     : 1;
		u64 ber_cnt                          : 22;
		u64 reserved_38_39                   : 2;
		u64 err_blks                         : 22;
		u64 reserved_62_63                   : 2;
	} s;
	/* struct cgxx_spux_br_status2_s cn; */
};

static inline u64 CGXX_SPUX_BR_STATUS2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_STATUS2(u64 a)
{
	return 0x10038 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_tp_control
 *
 * CGX SPU BASE-R Test-Pattern Control Registers Refer to the test
 * pattern methodology described in 802.3 sections 49.2.8 and 82.2.10.
 */
union cgxx_spux_br_tp_control {
	u64 u;
	struct cgxx_spux_br_tp_control_s {
		u64 dp_sel                           : 1;
		u64 tp_sel                           : 1;
		u64 rx_tp_en                         : 1;
		u64 tx_tp_en                         : 1;
		u64 prbs31_tx                        : 1;
		u64 prbs31_rx                        : 1;
		u64 prbs9_tx                         : 1;
		u64 scramble_tp                      : 2;
		u64 pr_tp_data_type                  : 1;
		u64 reserved_10_63                   : 54;
	} s;
	/* struct cgxx_spux_br_tp_control_s cn; */
};

static inline u64 CGXX_SPUX_BR_TP_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_TP_CONTROL(u64 a)
{
	return 0x10040 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_tp_err_cnt
 *
 * CGX SPU BASE-R Test-Pattern Error-Count Registers This register
 * provides the BASE-R PCS test-pattern error counter.
 */
union cgxx_spux_br_tp_err_cnt {
	u64 u;
	struct cgxx_spux_br_tp_err_cnt_s {
		u64 err_cnt                          : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_br_tp_err_cnt_s cn; */
};

static inline u64 CGXX_SPUX_BR_TP_ERR_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_TP_ERR_CNT(u64 a)
{
	return 0x10048 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_tp_seed_a
 *
 * CGX SPU BASE-R Test-Pattern Seed A Registers Refer to the test pattern
 * methodology described in 802.3 sections 49.2.8 and 82.2.10.
 */
union cgxx_spux_br_tp_seed_a {
	u64 u;
	struct cgxx_spux_br_tp_seed_a_s {
		u64 tp_seed_a                        : 58;
		u64 reserved_58_63                   : 6;
	} s;
	/* struct cgxx_spux_br_tp_seed_a_s cn; */
};

static inline u64 CGXX_SPUX_BR_TP_SEED_A(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_TP_SEED_A(u64 a)
{
	return 0x10060 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_br_tp_seed_b
 *
 * CGX SPU BASE-R Test-Pattern Seed B Registers Refer to the test pattern
 * methodology described in 802.3 sections 49.2.8 and 82.2.10.
 */
union cgxx_spux_br_tp_seed_b {
	u64 u;
	struct cgxx_spux_br_tp_seed_b_s {
		u64 tp_seed_b                        : 58;
		u64 reserved_58_63                   : 6;
	} s;
	/* struct cgxx_spux_br_tp_seed_b_s cn; */
};

static inline u64 CGXX_SPUX_BR_TP_SEED_B(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BR_TP_SEED_B(u64 a)
{
	return 0x10068 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_bx_status
 *
 * CGX SPU BASE-X Status Registers
 */
union cgxx_spux_bx_status {
	u64 u;
	struct cgxx_spux_bx_status_s {
		u64 lsync                            : 4;
		u64 reserved_4_10                    : 7;
		u64 pattst                           : 1;
		u64 alignd                           : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct cgxx_spux_bx_status_s cn; */
};

static inline u64 CGXX_SPUX_BX_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_BX_STATUS(u64 a)
{
	return 0x10028 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_control1
 *
 * CGX SPU Control 1 Registers
 */
union cgxx_spux_control1 {
	u64 u;
	struct cgxx_spux_control1_s {
		u64 reserved_0_1                     : 2;
		u64 spd                              : 4;
		u64 spdsel0                          : 1;
		u64 reserved_7_10                    : 4;
		u64 lo_pwr                           : 1;
		u64 reserved_12                      : 1;
		u64 spdsel1                          : 1;
		u64 loopbck                          : 1;
		u64 reset                            : 1;
		u64 usxgmii_type                     : 3;
		u64 usxgmii_rate                     : 3;
		u64 disable_am                       : 1;
		u64 reserved_23_63                   : 41;
	} s;
	struct cgxx_spux_control1_cn96xxp1 {
		u64 reserved_0_1                     : 2;
		u64 spd                              : 4;
		u64 spdsel0                          : 1;
		u64 reserved_7_10                    : 4;
		u64 lo_pwr                           : 1;
		u64 reserved_12                      : 1;
		u64 spdsel1                          : 1;
		u64 loopbck                          : 1;
		u64 reset                            : 1;
		u64 usxgmii_type                     : 3;
		u64 usxgmii_rate                     : 3;
		u64 reserved_22_63                   : 42;
	} cn96xxp1;
	/* struct cgxx_spux_control1_s cn96xxp3; */
	/* struct cgxx_spux_control1_cn96xxp1 cnf95xxp1; */
	struct cgxx_spux_control1_cnf95xxp2 {
		u64 reserved_0_1                     : 2;
		u64 spd                              : 4;
		u64 spdsel0                          : 1;
		u64 reserved_7_10                    : 4;
		u64 lo_pwr                           : 1;
		u64 reserved_12                      : 1;
		u64 spdsel1                          : 1;
		u64 loopbck                          : 1;
		u64 reset                            : 1;
		u64 usxgmii_type                     : 3;
		u64 usxgmii_rate                     : 3;
		u64 reserved_22                      : 1;
		u64 reserved_23_63                   : 41;
	} cnf95xxp2;
};

static inline u64 CGXX_SPUX_CONTROL1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_CONTROL1(u64 a)
{
	return 0x10000 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_control2
 *
 * CGX SPU Control 2 Registers
 */
union cgxx_spux_control2 {
	u64 u;
	struct cgxx_spux_control2_s {
		u64 pcs_type                         : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct cgxx_spux_control2_s cn; */
};

static inline u64 CGXX_SPUX_CONTROL2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_CONTROL2(u64 a)
{
	return 0x10018 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_fec_abil
 *
 * CGX SPU Forward Error Correction Ability Registers
 */
union cgxx_spux_fec_abil {
	u64 u;
	struct cgxx_spux_fec_abil_s {
		u64 fec_abil                         : 1;
		u64 err_abil                         : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct cgxx_spux_fec_abil_s cn; */
};

static inline u64 CGXX_SPUX_FEC_ABIL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_FEC_ABIL(u64 a)
{
	return 0x100d8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_fec_control
 *
 * CGX SPU Forward Error Correction Control Registers
 */
union cgxx_spux_fec_control {
	u64 u;
	struct cgxx_spux_fec_control_s {
		u64 fec_en                           : 2;
		u64 err_en                           : 1;
		u64 fec_byp_ind_en                   : 1;
		u64 fec_byp_cor_en                   : 1;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct cgxx_spux_fec_control_s cn; */
};

static inline u64 CGXX_SPUX_FEC_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_FEC_CONTROL(u64 a)
{
	return 0x100e0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_fec_ln#_rsfec_err
 *
 * CGX SPU Reed-Solomon FEC Symbol Error Counter for FEC Lanes 0-3
 * Registers This register is valid only when Reed-Solomon FEC is
 * enabled. The symbol error counters are defined in 802.3 section
 * 91.6.11 (for 100G and extended to 50G) and 802.3by-2016 section
 * 108.6.9 (for 25G and extended to USXGMII). The counter is reset to all
 * zeros when the register is read, and held at all ones in case of
 * overflow.  The reset operation takes precedence over the increment
 * operation; if the register is read on the same clock cycle as an
 * increment operation, the counter is reset to all zeros and the
 * increment operation is lost. The counters are writable for test
 * purposes, rather than read-only as specified in IEEE 802.3.
 */
union cgxx_spux_fec_lnx_rsfec_err {
	u64 u;
	struct cgxx_spux_fec_lnx_rsfec_err_s {
		u64 symb_err_cnt                     : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spux_fec_lnx_rsfec_err_s cn; */
};

static inline u64 CGXX_SPUX_FEC_LNX_RSFEC_ERR(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_FEC_LNX_RSFEC_ERR(u64 a, u64 b)
{
	return 0x10900 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_spu#_int
 *
 * CGX SPU Interrupt Registers
 */
union cgxx_spux_int {
	u64 u;
	struct cgxx_spux_int_s {
		u64 rx_link_up                       : 1;
		u64 rx_link_down                     : 1;
		u64 err_blk                          : 1;
		u64 bitlckls                         : 1;
		u64 synlos                           : 1;
		u64 algnlos                          : 1;
		u64 dbg_sync                         : 1;
		u64 bip_err                          : 1;
		u64 fec_corr                         : 1;
		u64 fec_uncorr                       : 1;
		u64 an_page_rx                       : 1;
		u64 an_link_good                     : 1;
		u64 an_complete                      : 1;
		u64 training_done                    : 1;
		u64 training_failure                 : 1;
		u64 fec_align_status                 : 1;
		u64 rsfec_corr                       : 1;
		u64 rsfec_uncorr                     : 1;
		u64 hi_ser                           : 1;
		u64 usx_an_lnk_st                    : 1;
		u64 usx_an_cpt                       : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct cgxx_spux_int_s cn; */
};

static inline u64 CGXX_SPUX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_INT(u64 a)
{
	return 0x10220 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_int_ena_w1c
 *
 * CGX SPU Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union cgxx_spux_int_ena_w1c {
	u64 u;
	struct cgxx_spux_int_ena_w1c_s {
		u64 rx_link_up                       : 1;
		u64 rx_link_down                     : 1;
		u64 err_blk                          : 1;
		u64 bitlckls                         : 1;
		u64 synlos                           : 1;
		u64 algnlos                          : 1;
		u64 dbg_sync                         : 1;
		u64 bip_err                          : 1;
		u64 fec_corr                         : 1;
		u64 fec_uncorr                       : 1;
		u64 an_page_rx                       : 1;
		u64 an_link_good                     : 1;
		u64 an_complete                      : 1;
		u64 training_done                    : 1;
		u64 training_failure                 : 1;
		u64 fec_align_status                 : 1;
		u64 rsfec_corr                       : 1;
		u64 rsfec_uncorr                     : 1;
		u64 hi_ser                           : 1;
		u64 usx_an_lnk_st                    : 1;
		u64 usx_an_cpt                       : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct cgxx_spux_int_ena_w1c_s cn; */
};

static inline u64 CGXX_SPUX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_INT_ENA_W1C(u64 a)
{
	return 0x10230 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_int_ena_w1s
 *
 * CGX SPU Interrupt Enable Set Registers This register sets interrupt
 * enable bits.
 */
union cgxx_spux_int_ena_w1s {
	u64 u;
	struct cgxx_spux_int_ena_w1s_s {
		u64 rx_link_up                       : 1;
		u64 rx_link_down                     : 1;
		u64 err_blk                          : 1;
		u64 bitlckls                         : 1;
		u64 synlos                           : 1;
		u64 algnlos                          : 1;
		u64 dbg_sync                         : 1;
		u64 bip_err                          : 1;
		u64 fec_corr                         : 1;
		u64 fec_uncorr                       : 1;
		u64 an_page_rx                       : 1;
		u64 an_link_good                     : 1;
		u64 an_complete                      : 1;
		u64 training_done                    : 1;
		u64 training_failure                 : 1;
		u64 fec_align_status                 : 1;
		u64 rsfec_corr                       : 1;
		u64 rsfec_uncorr                     : 1;
		u64 hi_ser                           : 1;
		u64 usx_an_lnk_st                    : 1;
		u64 usx_an_cpt                       : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct cgxx_spux_int_ena_w1s_s cn; */
};

static inline u64 CGXX_SPUX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_INT_ENA_W1S(u64 a)
{
	return 0x10238 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_int_w1s
 *
 * CGX SPU Interrupt Set Registers This register sets interrupt bits.
 */
union cgxx_spux_int_w1s {
	u64 u;
	struct cgxx_spux_int_w1s_s {
		u64 rx_link_up                       : 1;
		u64 rx_link_down                     : 1;
		u64 err_blk                          : 1;
		u64 bitlckls                         : 1;
		u64 synlos                           : 1;
		u64 algnlos                          : 1;
		u64 dbg_sync                         : 1;
		u64 bip_err                          : 1;
		u64 fec_corr                         : 1;
		u64 fec_uncorr                       : 1;
		u64 an_page_rx                       : 1;
		u64 an_link_good                     : 1;
		u64 an_complete                      : 1;
		u64 training_done                    : 1;
		u64 training_failure                 : 1;
		u64 fec_align_status                 : 1;
		u64 rsfec_corr                       : 1;
		u64 rsfec_uncorr                     : 1;
		u64 hi_ser                           : 1;
		u64 usx_an_lnk_st                    : 1;
		u64 usx_an_cpt                       : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct cgxx_spux_int_w1s_s cn; */
};

static inline u64 CGXX_SPUX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_INT_W1S(u64 a)
{
	return 0x10228 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_ln#_br_bip_err_cnt
 *
 * CGX SPU 40,50,100GBASE-R BIP Error-Counter Registers This register
 * implements the IEEE 802.3 BIP error-counter registers for PCS lanes
 * 0-19 (3.200-3.203). It is valid only when the LPCS type is 40GBASE-R,
 * 50GBASE-R, 100GBASE-R, (CGX()_CMR()_CONFIG[LMAC_TYPE]), and always
 * returns 0x0 for all other LPCS types. The counters are indexed by the
 * RX PCS lane number based on the alignment marker detected on each lane
 * and captured in CGX()_SPU()_BR_LANE_MAP(). Each counter counts the BIP
 * errors for its PCS lane, and is held at all ones in case of overflow.
 * The counters are reset to all zeros when this register is read by
 * software.  The reset operation takes precedence over the increment
 * operation; if the register is read on the same clock cycle as an
 * increment operation, the counter is reset to all zeros and the
 * increment operation is lost. The counters are writable for test
 * purposes, rather than read-only as specified in IEEE 802.3.
 */
union cgxx_spux_lnx_br_bip_err_cnt {
	u64 u;
	struct cgxx_spux_lnx_br_bip_err_cnt_s {
		u64 bip_err_cnt                      : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_lnx_br_bip_err_cnt_s cn; */
};

static inline u64 CGXX_SPUX_LNX_BR_BIP_ERR_CNT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_LNX_BR_BIP_ERR_CNT(u64 a, u64 b)
{
	return 0x10500 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_spu#_ln#_fec_corr_blks
 *
 * CGX SPU FEC Corrected-Blocks Counters 0-19 Registers This register is
 * valid only when the LPCS type is BASE-R
 * (CGX()_CMR()_CONFIG[LMAC_TYPE]) and applies to BASE-R FEC and Reed-
 * Solomon FEC (RS-FEC). When BASE-R FEC is enabled, the FEC corrected-
 * block counters are defined in IEEE 802.3 section 74.8.4.1. Each
 * corrected-blocks counter increments by one for a corrected FEC block,
 * i.e. an FEC block that has been received with invalid parity on the
 * associated PCS lane and has been corrected by the FEC decoder. The
 * counter is reset to all zeros when the register is read, and held at
 * all ones in case of overflow.  The reset operation takes precedence
 * over the increment operation; if the register is read on the same
 * clock cycle as an increment operation, the counter is reset to all
 * zeros and the increment operation is lost. The counters are writable
 * for test purposes, rather than read-only as specified in IEEE 802.3.
 */
union cgxx_spux_lnx_fec_corr_blks {
	u64 u;
	struct cgxx_spux_lnx_fec_corr_blks_s {
		u64 ln_corr_blks                     : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spux_lnx_fec_corr_blks_s cn; */
};

static inline u64 CGXX_SPUX_LNX_FEC_CORR_BLKS(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_LNX_FEC_CORR_BLKS(u64 a, u64 b)
{
	return 0x10700 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_spu#_ln#_fec_uncorr_blks
 *
 * CGX SPU FEC Uncorrected-Blocks Counters 0-19 Registers This register
 * is valid only when the LPCS type is BASE-R
 * (CGX()_CMR()_CONFIG[LMAC_TYPE]) and applies to BASE-R FEC and Reed-
 * Solomon FEC (RS-FEC). When BASE-R FEC is enabled, the FEC corrected-
 * block counters are defined in IEEE 802.3 section 74.8.4.2. Each
 * uncorrected-blocks counter increments by one for an uncorrected FEC
 * block, i.e. an FEC block that has been received with invalid parity on
 * the associated PCS lane and has not been corrected by the FEC decoder.
 * The counter is reset to all zeros when the register is read, and held
 * at all ones in case of overflow.  The reset operation takes precedence
 * over the increment operation; if the register is read on the same
 * clock cycle as an increment operation, the counter is reset to all
 * zeros and the increment operation is lost. The counters are writable
 * for test purposes, rather than read-only as specified in IEEE 802.3.
 */
union cgxx_spux_lnx_fec_uncorr_blks {
	u64 u;
	struct cgxx_spux_lnx_fec_uncorr_blks_s {
		u64 ln_uncorr_blks                   : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spux_lnx_fec_uncorr_blks_s cn; */
};

static inline u64 CGXX_SPUX_LNX_FEC_UNCORR_BLKS(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_LNX_FEC_UNCORR_BLKS(u64 a, u64 b)
{
	return 0x10800 + 0x40000 * a + 8 * b;
}

/**
 * Register (RSL) cgx#_spu#_lpcs_states
 *
 * CGX SPU BASE-X Transmit/Receive States Registers
 */
union cgxx_spux_lpcs_states {
	u64 u;
	struct cgxx_spux_lpcs_states_s {
		u64 deskew_sm                        : 3;
		u64 reserved_3                       : 1;
		u64 deskew_am_found                  : 20;
		u64 bx_rx_sm                         : 2;
		u64 reserved_26_27                   : 2;
		u64 br_rx_sm                         : 3;
		u64 reserved_31_63                   : 33;
	} s;
	/* struct cgxx_spux_lpcs_states_s cn; */
};

static inline u64 CGXX_SPUX_LPCS_STATES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_LPCS_STATES(u64 a)
{
	return 0x10208 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_misc_control
 *
 * CGX SPU Miscellaneous Control Registers "* RX logical PCS lane
 * polarity vector \<3:0\> = [XOR_RXPLRT]\<3:0\> ^ {4{[RXPLRT]}}. * TX
 * logical PCS lane polarity vector \<3:0\> = [XOR_TXPLRT]\<3:0\> ^
 * {4{[TXPLRT]}}.  In short, keep [RXPLRT] and [TXPLRT] cleared, and use
 * [XOR_RXPLRT] and [XOR_TXPLRT] fields to define the polarity per
 * logical PCS lane. Only bit 0 of vector is used for 10GBASE-R, and only
 * bits 1:0 of vector are used for RXAUI."
 */
union cgxx_spux_misc_control {
	u64 u;
	struct cgxx_spux_misc_control_s {
		u64 txplrt                           : 1;
		u64 rxplrt                           : 1;
		u64 xor_txplrt                       : 4;
		u64 xor_rxplrt                       : 4;
		u64 intlv_rdisp                      : 1;
		u64 skip_after_term                  : 1;
		u64 rx_packet_dis                    : 1;
		u64 rx_edet_signal_ok                : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct cgxx_spux_misc_control_s cn; */
};

static inline u64 CGXX_SPUX_MISC_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_MISC_CONTROL(u64 a)
{
	return 0x10218 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_rsfec_corr
 *
 * CGX SPU Reed-Solomon FEC Corrected Codeword Counter Register This
 * register implements the IEEE 802.3 RS-FEC corrected codewords counter
 * described in 802.3 section 91.6.8 (for 100G and extended to 50G) and
 * 802.3by-2016 section 108.6.7 (for 25G and extended to USXGMII).
 */
union cgxx_spux_rsfec_corr {
	u64 u;
	struct cgxx_spux_rsfec_corr_s {
		u64 cw_cnt                           : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spux_rsfec_corr_s cn; */
};

static inline u64 CGXX_SPUX_RSFEC_CORR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RSFEC_CORR(u64 a)
{
	return 0x10088 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_rsfec_status
 *
 * CGX SPU Reed-Solomon FEC Status Registers This register implements the
 * IEEE 802.3 RS-FEC status and lane mapping registers as described in
 * 802.3 section 91.6 (for 100G and extended to 50G) and 802.3by-2016
 * section 108-6 (for 25G and extended to USXGMII).
 */
union cgxx_spux_rsfec_status {
	u64 u;
	struct cgxx_spux_rsfec_status_s {
		u64 fec_lane_mapping                 : 8;
		u64 fec_align_status                 : 1;
		u64 amps_lock                        : 4;
		u64 hi_ser                           : 1;
		u64 fec_byp_ind_abil                 : 1;
		u64 fec_byp_cor_abil                 : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_rsfec_status_s cn; */
};

static inline u64 CGXX_SPUX_RSFEC_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RSFEC_STATUS(u64 a)
{
	return 0x10080 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_rsfec_uncorr
 *
 * CGX SPU Reed-Solomon FEC Uncorrected Codeword Counter Register This
 * register implements the IEEE 802.3 RS-FEC uncorrected codewords
 * counter described in 802.3 section 91.6.9 (for 100G and extended to
 * 50G) and 802.3by-2016 section 108.6.8 (for 25G and extended to
 * USXGMII).
 */
union cgxx_spux_rsfec_uncorr {
	u64 u;
	struct cgxx_spux_rsfec_uncorr_s {
		u64 cw_cnt                           : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spux_rsfec_uncorr_s cn; */
};

static inline u64 CGXX_SPUX_RSFEC_UNCORR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RSFEC_UNCORR(u64 a)
{
	return 0x10090 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_rx_eee_wake
 *
 * INTERNAL: CGX SPU  RX EEE Wake Error Counter  Registers  Reserved.
 * Internal: A counter that is incremented each time that the LPI receive
 * state diagram enters the RX_WTF state indicating that a wake time
 * fault has been detected.
 */
union cgxx_spux_rx_eee_wake {
	u64 u;
	struct cgxx_spux_rx_eee_wake_s {
		u64 wtf_error_counter                : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_rx_eee_wake_s cn; */
};

static inline u64 CGXX_SPUX_RX_EEE_WAKE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RX_EEE_WAKE(u64 a)
{
	return 0x103e0 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_rx_lpi_timing
 *
 * INTERNAL: CGX SPU RX EEE LPI Timing Parameters Registers  Reserved.
 * Internal: This register specifies receiver LPI timing parameters Tqr,
 * Twr and Twtf.
 */
union cgxx_spux_rx_lpi_timing {
	u64 u;
	struct cgxx_spux_rx_lpi_timing_s {
		u64 twtf                             : 20;
		u64 twr                              : 20;
		u64 tqr                              : 20;
		u64 reserved_60_61                   : 2;
		u64 rx_lpi_fw                        : 1;
		u64 rx_lpi_en                        : 1;
	} s;
	/* struct cgxx_spux_rx_lpi_timing_s cn; */
};

static inline u64 CGXX_SPUX_RX_LPI_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RX_LPI_TIMING(u64 a)
{
	return 0x103c0 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_rx_lpi_timing2
 *
 * INTERNAL: CGX SPU RX EEE LPI Timing2 Parameters Registers  Reserved.
 * Internal: This register specifies receiver LPI timing parameters
 * hold_off_timer.
 */
union cgxx_spux_rx_lpi_timing2 {
	u64 u;
	struct cgxx_spux_rx_lpi_timing2_s {
		u64 hold_off_timer                   : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct cgxx_spux_rx_lpi_timing2_s cn; */
};

static inline u64 CGXX_SPUX_RX_LPI_TIMING2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RX_LPI_TIMING2(u64 a)
{
	return 0x10420 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_rx_mrk_cnt
 *
 * CGX SPU Receiver Marker Interval Count Control Registers
 */
union cgxx_spux_rx_mrk_cnt {
	u64 u;
	struct cgxx_spux_rx_mrk_cnt_s {
		u64 mrk_cnt                          : 20;
		u64 reserved_20_43                   : 24;
		u64 by_mrk_100g                      : 1;
		u64 reserved_45_47                   : 3;
		u64 ram_mrk_cnt                      : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct cgxx_spux_rx_mrk_cnt_s cn; */
};

static inline u64 CGXX_SPUX_RX_MRK_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_RX_MRK_CNT(u64 a)
{
	return 0x103a0 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_spd_abil
 *
 * CGX SPU PCS Speed Ability Registers
 */
union cgxx_spux_spd_abil {
	u64 u;
	struct cgxx_spux_spd_abil_s {
		u64 tengb                            : 1;
		u64 tenpasst                         : 1;
		u64 usxgmii                          : 1;
		u64 twentyfivegb                     : 1;
		u64 fortygb                          : 1;
		u64 fiftygb                          : 1;
		u64 hundredgb                        : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct cgxx_spux_spd_abil_s cn; */
};

static inline u64 CGXX_SPUX_SPD_ABIL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_SPD_ABIL(u64 a)
{
	return 0x10010 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_status1
 *
 * CGX SPU Status 1 Registers
 */
union cgxx_spux_status1 {
	u64 u;
	struct cgxx_spux_status1_s {
		u64 reserved_0                       : 1;
		u64 lpable                           : 1;
		u64 rcv_lnk                          : 1;
		u64 reserved_3_6                     : 4;
		u64 flt                              : 1;
		u64 rx_lpi_indication                : 1;
		u64 tx_lpi_indication                : 1;
		u64 rx_lpi_received                  : 1;
		u64 tx_lpi_received                  : 1;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct cgxx_spux_status1_s cn; */
};

static inline u64 CGXX_SPUX_STATUS1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_STATUS1(u64 a)
{
	return 0x10008 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_status2
 *
 * CGX SPU Status 2 Registers
 */
union cgxx_spux_status2 {
	u64 u;
	struct cgxx_spux_status2_s {
		u64 tengb_r                          : 1;
		u64 tengb_x                          : 1;
		u64 tengb_w                          : 1;
		u64 tengb_t                          : 1;
		u64 usxgmii_r                        : 1;
		u64 twentyfivegb_r                   : 1;
		u64 fortygb_r                        : 1;
		u64 fiftygb_r                        : 1;
		u64 hundredgb_r                      : 1;
		u64 reserved_9                       : 1;
		u64 rcvflt                           : 1;
		u64 xmtflt                           : 1;
		u64 reserved_12_13                   : 2;
		u64 dev                              : 2;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_status2_s cn; */
};

static inline u64 CGXX_SPUX_STATUS2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_STATUS2(u64 a)
{
	return 0x10020 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_tx_lpi_timing
 *
 * INTERNAL: CGX SPU TX EEE LPI Timing Parameters Registers  Reserved.
 * Internal: Transmit LPI timing parameters Tsl, Tql and Tul
 */
union cgxx_spux_tx_lpi_timing {
	u64 u;
	struct cgxx_spux_tx_lpi_timing_s {
		u64 tql                              : 19;
		u64 reserved_19_31                   : 13;
		u64 tul                              : 12;
		u64 reserved_44_47                   : 4;
		u64 tsl                              : 12;
		u64 reserved_60                      : 1;
		u64 tx_lpi_ignore_twl                : 1;
		u64 tx_lpi_fw                        : 1;
		u64 tx_lpi_en                        : 1;
	} s;
	/* struct cgxx_spux_tx_lpi_timing_s cn; */
};

static inline u64 CGXX_SPUX_TX_LPI_TIMING(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_TX_LPI_TIMING(u64 a)
{
	return 0x10400 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_tx_lpi_timing2
 *
 * INTERNAL: CGX SPU TX EEE LPI Timing2 Parameters Registers  Reserved.
 * Internal: This register specifies transmit LPI timer parameters.
 */
union cgxx_spux_tx_lpi_timing2 {
	u64 u;
	struct cgxx_spux_tx_lpi_timing2_s {
		u64 t1u                              : 8;
		u64 reserved_8_11                    : 4;
		u64 twl                              : 12;
		u64 reserved_24_31                   : 8;
		u64 twl2                             : 12;
		u64 reserved_44_47                   : 4;
		u64 tbyp                             : 12;
		u64 reserved_60_63                   : 4;
	} s;
	/* struct cgxx_spux_tx_lpi_timing2_s cn; */
};

static inline u64 CGXX_SPUX_TX_LPI_TIMING2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_TX_LPI_TIMING2(u64 a)
{
	return 0x10440 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_tx_mrk_cnt
 *
 * CGX SPU Transmitter Marker Interval Count Control Registers
 */
union cgxx_spux_tx_mrk_cnt {
	u64 u;
	struct cgxx_spux_tx_mrk_cnt_s {
		u64 mrk_cnt                          : 20;
		u64 reserved_20_43                   : 24;
		u64 by_mrk_100g                      : 1;
		u64 reserved_45_47                   : 3;
		u64 ram_mrk_cnt                      : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct cgxx_spux_tx_mrk_cnt_s cn; */
};

static inline u64 CGXX_SPUX_TX_MRK_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_TX_MRK_CNT(u64 a)
{
	return 0x10380 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_adv
 *
 * CGX SPU USXGMII Autonegotiation Advertisement Registers Software
 * programs this register with the contents of the AN-link code word base
 * page to be transmitted during autonegotiation. Any write operations to
 * this register prior to completion of autonegotiation should be
 * followed by a renegotiation in order for the new values to take
 * effect. Once autonegotiation has completed, software can examine this
 * register along with CGX()_SPU()_USX_AN_ADV to determine the highest
 * common denominator technology. The format for this register is from
 * USXGMII Multiport specification section 1.1.2 Table 2.
 */
union cgxx_spux_usx_an_adv {
	u64 u;
	struct cgxx_spux_usx_an_adv_s {
		u64 set                              : 1;
		u64 reserved_1_6                     : 6;
		u64 eee_clk_stop_abil                : 1;
		u64 eee_abil                         : 1;
		u64 spd                              : 3;
		u64 dplx                             : 1;
		u64 reserved_13_14                   : 2;
		u64 lnk_st                           : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_usx_an_adv_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_ADV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_ADV(u64 a)
{
	return 0x101d0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_control
 *
 * CGX SPU USXGMII Autonegotiation Control Register
 */
union cgxx_spux_usx_an_control {
	u64 u;
	struct cgxx_spux_usx_an_control_s {
		u64 reserved_0_8                     : 9;
		u64 rst_an                           : 1;
		u64 reserved_10_11                   : 2;
		u64 an_en                            : 1;
		u64 reserved_13_14                   : 2;
		u64 an_reset                         : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_usx_an_control_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_CONTROL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_CONTROL(u64 a)
{
	return 0x101c0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_expansion
 *
 * CGX SPU USXGMII Autonegotiation Expansion Register This register is
 * only used to signal page reception.
 */
union cgxx_spux_usx_an_expansion {
	u64 u;
	struct cgxx_spux_usx_an_expansion_s {
		u64 reserved_0                       : 1;
		u64 an_page_received                 : 1;
		u64 next_page_able                   : 1;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct cgxx_spux_usx_an_expansion_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_EXPANSION(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_EXPANSION(u64 a)
{
	return 0x101e0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_flow_ctrl
 *
 * CGX SPU USXGMII Flow Control Registers This register is used by
 * software to affect USXGMII AN hardware behavior.
 */
union cgxx_spux_usx_an_flow_ctrl {
	u64 u;
	struct cgxx_spux_usx_an_flow_ctrl_s {
		u64 start_idle_detect                : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct cgxx_spux_usx_an_flow_ctrl_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_FLOW_CTRL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_FLOW_CTRL(u64 a)
{
	return 0x101e8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_link_timer
 *
 * CGX SPU USXGMII Link Timer Registers This is the link timer register.
 */
union cgxx_spux_usx_an_link_timer {
	u64 u;
	struct cgxx_spux_usx_an_link_timer_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_usx_an_link_timer_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_LINK_TIMER(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_LINK_TIMER(u64 a)
{
	return 0x101f0 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_lp_abil
 *
 * CGX SPU USXGMII Autonegotiation Link-Partner Advertisement Registers
 * This register captures the contents of the latest AN link code word
 * base page received from the link partner during autonegotiation. This
 * is register 5 per IEEE 802.3, Clause 37.
 * CGX()_SPU()_USX_AN_EXPANSION[AN_PAGE_RECEIVED] is set when this
 * register is updated by hardware.
 */
union cgxx_spux_usx_an_lp_abil {
	u64 u;
	struct cgxx_spux_usx_an_lp_abil_s {
		u64 set                              : 1;
		u64 reserved_1_6                     : 6;
		u64 eee_clk_stop_abil                : 1;
		u64 eee_abil                         : 1;
		u64 spd                              : 3;
		u64 dplx                             : 1;
		u64 reserved_13_14                   : 2;
		u64 lnk_st                           : 1;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct cgxx_spux_usx_an_lp_abil_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_LP_ABIL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_LP_ABIL(u64 a)
{
	return 0x101d8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu#_usx_an_status
 *
 * CGX SPU USXGMII Autonegotiation Status Register
 */
union cgxx_spux_usx_an_status {
	u64 u;
	struct cgxx_spux_usx_an_status_s {
		u64 extnd                            : 1;
		u64 reserved_1                       : 1;
		u64 lnk_st                           : 1;
		u64 an_abil                          : 1;
		u64 rmt_flt                          : 1;
		u64 an_cpt                           : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_spux_usx_an_status_s cn; */
};

static inline u64 CGXX_SPUX_USX_AN_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPUX_USX_AN_STATUS(u64 a)
{
	return 0x101c8 + 0x40000 * a;
}

/**
 * Register (RSL) cgx#_spu_dbg_control
 *
 * CGX SPU Debug Control Registers
 */
union cgxx_spu_dbg_control {
	u64 u;
	struct cgxx_spu_dbg_control_s {
		u64 marker_rxp                       : 15;
		u64 reserved_15                      : 1;
		u64 scramble_dis                     : 1;
		u64 reserved_17_18                   : 2;
		u64 br_pmd_train_soft_en             : 1;
		u64 reserved_20_27                   : 8;
		u64 timestamp_norm_dis               : 1;
		u64 an_nonce_match_dis               : 1;
		u64 br_ber_mon_dis                   : 1;
		u64 rf_cw_mon_erly_restart_dis       : 1;
		u64 us_clk_period                    : 12;
		u64 ms_clk_period                    : 12;
		u64 reserved_56_63                   : 8;
	} s;
	struct cgxx_spu_dbg_control_cn96xxp1 {
		u64 marker_rxp                       : 15;
		u64 reserved_15                      : 1;
		u64 scramble_dis                     : 1;
		u64 reserved_17_18                   : 2;
		u64 br_pmd_train_soft_en             : 1;
		u64 reserved_20_27                   : 8;
		u64 timestamp_norm_dis               : 1;
		u64 an_nonce_match_dis               : 1;
		u64 br_ber_mon_dis                   : 1;
		u64 reserved_31                      : 1;
		u64 us_clk_period                    : 12;
		u64 ms_clk_period                    : 12;
		u64 reserved_56_63                   : 8;
	} cn96xxp1;
	/* struct cgxx_spu_dbg_control_s cn96xxp3; */
	/* struct cgxx_spu_dbg_control_cn96xxp1 cnf95xxp1; */
	/* struct cgxx_spu_dbg_control_s cnf95xxp2; */
};

static inline u64 CGXX_SPU_DBG_CONTROL(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPU_DBG_CONTROL(void)
{
	return 0x10300;
}

/**
 * Register (RSL) cgx#_spu_sds#_skew_status
 *
 * CGX SPU SerDes Lane Skew Status Registers This register provides
 * SerDes lane skew status. One register per physical SerDes lane.
 */
union cgxx_spu_sdsx_skew_status {
	u64 u;
	struct cgxx_spu_sdsx_skew_status_s {
		u64 skew_status                      : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct cgxx_spu_sdsx_skew_status_s cn; */
};

static inline u64 CGXX_SPU_SDSX_SKEW_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPU_SDSX_SKEW_STATUS(u64 a)
{
	return 0x10340 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu_sds#_states
 *
 * CGX SPU SerDes States Registers This register provides SerDes lane
 * states. One register per physical SerDes lane.
 */
union cgxx_spu_sdsx_states {
	u64 u;
	struct cgxx_spu_sdsx_states_s {
		u64 bx_sync_sm                       : 4;
		u64 br_sh_cnt                        : 11;
		u64 br_block_lock                    : 1;
		u64 br_sh_invld_cnt                  : 7;
		u64 reserved_23                      : 1;
		u64 fec_sync_cnt                     : 4;
		u64 fec_block_sync                   : 1;
		u64 reserved_29                      : 1;
		u64 an_rx_sm                         : 2;
		u64 an_arb_sm                        : 3;
		u64 reserved_35                      : 1;
		u64 train_lock_bad_markers           : 3;
		u64 train_lock_found_1st_marker      : 1;
		u64 train_frame_lock                 : 1;
		u64 train_code_viol                  : 1;
		u64 train_sm                         : 3;
		u64 reserved_45_47                   : 3;
		u64 am_lock_sm                       : 2;
		u64 am_lock_invld_cnt                : 2;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct cgxx_spu_sdsx_states_s cn; */
};

static inline u64 CGXX_SPU_SDSX_STATES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPU_SDSX_STATES(u64 a)
{
	return 0x10360 + 8 * a;
}

/**
 * Register (RSL) cgx#_spu_usxgmii_control
 *
 * CGX SPU Common USXGMII Control Register This register is the common
 * control register that enables USXGMII Mode. The fields in this
 * register are preserved across any LMAC soft-resets. For an LMAC in
 * soft- reset state in USXGMII mode, the CGX will transmit Remote Fault
 * BASE-R blocks.
 */
union cgxx_spu_usxgmii_control {
	u64 u;
	struct cgxx_spu_usxgmii_control_s {
		u64 enable                           : 1;
		u64 usxgmii_type                     : 3;
		u64 sds_id                           : 2;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct cgxx_spu_usxgmii_control_s cn; */
};

static inline u64 CGXX_SPU_USXGMII_CONTROL(void)
	__attribute__ ((pure, always_inline));
static inline u64 CGXX_SPU_USXGMII_CONTROL(void)
{
	return 0x10920;
}

#endif /* __CSRS_CGX_H__ */
