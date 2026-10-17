/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_GSERX_DEFS_H__
#define __CVMX_GSERX_DEFS_H__

#define CVMX_GSERX_DLMX_TX_AMPLITUDE(offset, block_id) (0x0001180090003008ull)
#define CVMX_GSERX_DLMX_TX_PREEMPH(offset, block_id)   (0x0001180090003028ull)
#define CVMX_GSERX_DLMX_MPLL_EN(offset, block_id)      (0x0001180090001020ull)
#define CVMX_GSERX_DLMX_REF_SSP_EN(offset, block_id)   (0x0001180090001048ull)
#define CVMX_GSERX_DLMX_TX_RATE(offset, block_id)      (0x0001180090003030ull)
#define CVMX_GSERX_DLMX_TX_EN(offset, block_id)	       (0x0001180090003020ull)
#define CVMX_GSERX_DLMX_TX_CM_EN(offset, block_id)     (0x0001180090003010ull)
#define CVMX_GSERX_DLMX_TX_RESET(offset, block_id)     (0x0001180090003038ull)
#define CVMX_GSERX_DLMX_TX_DATA_EN(offset, block_id)   (0x0001180090003018ull)
#define CVMX_GSERX_DLMX_RX_RATE(offset, block_id)      (0x0001180090002028ull)
#define CVMX_GSERX_DLMX_RX_PLL_EN(offset, block_id)    (0x0001180090002020ull)
#define CVMX_GSERX_DLMX_RX_DATA_EN(offset, block_id)   (0x0001180090002008ull)
#define CVMX_GSERX_DLMX_RX_RESET(offset, block_id)     (0x0001180090002030ull)

#define CVMX_GSERX_DLMX_TX_STATUS(offset, block_id)                                                \
	(0x0001180090003000ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_RX_STATUS(offset, block_id)                                                \
	(0x0001180090002000ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)

static inline u64 CVMX_GSERX_SATA_STATUS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x0001180090100200ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x0001180090100900ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180090100900ull + (offset) * 0x1000000ull;
	}
	return 0x0001180090100900ull + (offset) * 0x1000000ull;
}

#define CVMX_GSERX_DLMX_RX_EQ(offset, block_id)                                                    \
	(0x0001180090002010ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_REF_USE_PAD(offset, block_id)                                              \
	(0x0001180090001050ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_REFCLK_SEL(offset, block_id)                                               \
	(0x0001180090000008ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_PHY_RESET(offset, block_id)                                                \
	(0x0001180090001038ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_TEST_POWERDOWN(offset, block_id)                                           \
	(0x0001180090001060ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_REF_CLKDIV2(offset, block_id)                                              \
	(0x0001180090001040ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_MPLL_MULTIPLIER(offset, block_id)                                          \
	(0x0001180090001030ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)
#define CVMX_GSERX_DLMX_MPLL_STATUS(offset, block_id)                                              \
	(0x0001180090001000ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)

#define CVMX_GSERX_BR_RXX_CTL(offset, block_id)                                                    \
	(0x0001180090000400ull + (((offset) & 3) + ((block_id) & 15) * 0x20000ull) * 128)
#define CVMX_GSERX_BR_RXX_EER(offset, block_id)                                                    \
	(0x0001180090000418ull + (((offset) & 3) + ((block_id) & 15) * 0x20000ull) * 128)

#define CVMX_GSERX_PCIE_PIPE_PORT_SEL(offset) (0x0001180090080460ull)
#define CVMX_GSERX_PCIE_PIPE_RST(offset)      (0x0001180090080448ull)

#define CVMX_GSERX_SATA_CFG(offset)	   (0x0001180090100208ull)
#define CVMX_GSERX_SATA_REF_SSP_EN(offset) (0x0001180090100600ull)

static inline u64 CVMX_GSERX_SATA_LANE_RST(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x0001180090100210ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x0001180090000908ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180090000908ull + (offset) * 0x1000000ull;
	}
	return 0x0001180090000908ull + (offset) * 0x1000000ull;
}

#define CVMX_GSERX_EQ_WAIT_TIME(offset) (0x00011800904E0000ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_GLBL_MISC_CONFIG_1(offset) (0x0001180090460030ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_GLBL_PLL_CFG_3(offset)     (0x0001180090460018ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_PHYX_OVRD_IN_LO(offset, block_id)                                               \
	(0x0001180090400088ull + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 524288)

#define CVMX_GSERX_RX_PWR_CTRL_P1(offset) (0x00011800904600B0ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_RX_PWR_CTRL_P2(offset) (0x00011800904600B8ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_RX_EIE_DETSTS(offset)  (0x0001180090000150ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_LANE_MODE(offset) (0x0001180090000118ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_LANE_VMA_FINE_CTRL_0(offset)                                                    \
	(0x00011800904E01C8ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_LANEX_LBERT_CFG(offset, block_id)                                               \
	(0x00011800904C0020ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANEX_MISC_CFG_0(offset, block_id)                                              \
	(0x00011800904C0000ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANE_PX_MODE_0(offset, block_id)                                                \
	(0x00011800904E0040ull + (((offset) & 15) + ((block_id) & 15) * 0x80000ull) * 32)
#define CVMX_GSERX_LANE_PX_MODE_1(offset, block_id)                                                \
	(0x00011800904E0048ull + (((offset) & 15) + ((block_id) & 15) * 0x80000ull) * 32)

#define CVMX_GSERX_LANEX_RX_CFG_0(offset, block_id)                                                \
	(0x0001180090440000ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CFG_1(offset, block_id)                                                \
	(0x0001180090440008ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CFG_2(offset, block_id)                                                \
	(0x0001180090440010ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CFG_3(offset, block_id)                                                \
	(0x0001180090440018ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CFG_4(offset, block_id)                                                \
	(0x0001180090440020ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CFG_5(offset, block_id)                                                \
	(0x0001180090440028ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_CTLE_CTRL(offset, block_id)                                            \
	(0x0001180090440058ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANEX_RX_LOOP_CTRL(offset, block_id)                                            \
	(0x0001180090440048ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_VALBBD_CTRL_0(offset, block_id)                                        \
	(0x0001180090440240ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_VALBBD_CTRL_1(offset, block_id)                                        \
	(0x0001180090440248ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_VALBBD_CTRL_2(offset, block_id)                                        \
	(0x0001180090440250ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_RX_MISC_OVRRD(offset, block_id)                                           \
	(0x0001180090440258ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANEX_TX_CFG_0(offset, block_id)                                                \
	(0x00011800904400A8ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_TX_CFG_1(offset, block_id)                                                \
	(0x00011800904400B0ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_TX_CFG_2(offset, block_id)                                                \
	(0x00011800904400B8ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_TX_CFG_3(offset, block_id)                                                \
	(0x00011800904400C0ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_TX_PRE_EMPHASIS(offset, block_id)                                         \
	(0x00011800904400C8ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_RX_TXDIR_CTRL_0(offset) (0x00011800904600E8ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_RX_TXDIR_CTRL_1(offset) (0x00011800904600F0ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_RX_TXDIR_CTRL_2(offset) (0x00011800904600F8ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_LANEX_PCS_CTLIFC_0(offset, block_id)                                            \
	(0x00011800904C0060ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_PCS_CTLIFC_1(offset, block_id)                                            \
	(0x00011800904C0068ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)
#define CVMX_GSERX_LANEX_PCS_CTLIFC_2(offset, block_id)                                            \
	(0x00011800904C0070ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANEX_PWR_CTRL(offset, block_id)                                                \
	(0x00011800904400D8ull + (((offset) & 3) + ((block_id) & 15) * 0x10ull) * 1048576)

#define CVMX_GSERX_LANE_VMA_FINE_CTRL_2(offset)                                                    \
	(0x00011800904E01D8ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_PLL_STAT(offset) (0x0001180090000010ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_QLM_STAT(offset) (0x00011800900000A0ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_PLL_PX_MODE_0(offset, block_id)                                                 \
	(0x00011800904E0030ull + (((offset) & 15) + ((block_id) & 15) * 0x80000ull) * 32)
#define CVMX_GSERX_PLL_PX_MODE_1(offset, block_id)                                                 \
	(0x00011800904E0038ull + (((offset) & 15) + ((block_id) & 15) * 0x80000ull) * 32)

#define CVMX_GSERX_SLICE_CFG(offset) (0x0001180090460060ull + ((offset) & 15) * 0x1000000ull)

#define CVMX_GSERX_SLICEX_PCIE1_MODE(offset, block_id)                                             \
	(0x0001180090460228ull + (((offset) & 1) + ((block_id) & 15) * 0x8ull) * 2097152)
#define CVMX_GSERX_SLICEX_PCIE2_MODE(offset, block_id)                                             \
	(0x0001180090460230ull + (((offset) & 1) + ((block_id) & 15) * 0x8ull) * 2097152)
#define CVMX_GSERX_SLICEX_PCIE3_MODE(offset, block_id)                                             \
	(0x0001180090460238ull + (((offset) & 1) + ((block_id) & 15) * 0x8ull) * 2097152)
#define CVMX_GSERX_SLICEX_RX_SDLL_CTRL(offset, block_id)                                           \
	(0x0001180090460220ull + (((offset) & 1) + ((block_id) & 15) * 0x8ull) * 2097152)

#define CVMX_GSERX_REFCLK_SEL(offset) (0x0001180090000008ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_PHY_CTL(offset)    (0x0001180090000000ull + ((offset) & 15) * 0x1000000ull)
#define CVMX_GSERX_CFG(offset)	      (0x0001180090000080ull + ((offset) & 15) * 0x1000000ull)

/**
 * cvmx_gser#_cfg
 */
union cvmx_gserx_cfg {
	u64 u64;
	struct cvmx_gserx_cfg_s {
		u64 reserved_9_63 : 55;
		u64 rmac_pipe : 1;
		u64 rmac : 1;
		u64 srio : 1;
		u64 sata : 1;
		u64 bgx_quad : 1;
		u64 bgx_dual : 1;
		u64 bgx : 1;
		u64 ila : 1;
		u64 pcie : 1;
	} s;
	struct cvmx_gserx_cfg_cn73xx {
		u64 reserved_6_63 : 58;
		u64 sata : 1;
		u64 bgx_quad : 1;
		u64 bgx_dual : 1;
		u64 bgx : 1;
		u64 ila : 1;
		u64 pcie : 1;
	} cn73xx;
	struct cvmx_gserx_cfg_cn78xx {
		u64 reserved_5_63 : 59;
		u64 bgx_quad : 1;
		u64 bgx_dual : 1;
		u64 bgx : 1;
		u64 ila : 1;
		u64 pcie : 1;
	} cn78xx;
	struct cvmx_gserx_cfg_cn78xx cn78xxp1;
	struct cvmx_gserx_cfg_s cnf75xx;
};

typedef union cvmx_gserx_cfg cvmx_gserx_cfg_t;

/**
 * cvmx_gser#_eq_wait_time
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_eq_wait_time {
	u64 u64;
	struct cvmx_gserx_eq_wait_time_s {
		u64 reserved_8_63 : 56;
		u64 rxeq_wait_cnt : 4;
		u64 txeq_wait_cnt : 4;
	} s;
	struct cvmx_gserx_eq_wait_time_s cn73xx;
	struct cvmx_gserx_eq_wait_time_s cn78xx;
	struct cvmx_gserx_eq_wait_time_s cn78xxp1;
	struct cvmx_gserx_eq_wait_time_s cnf75xx;
};

typedef union cvmx_gserx_eq_wait_time cvmx_gserx_eq_wait_time_t;

/**
 * cvmx_gser#_glbl_misc_config_1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_glbl_misc_config_1 {
	u64 u64;
	struct cvmx_gserx_glbl_misc_config_1_s {
		u64 reserved_10_63 : 54;
		u64 pcs_sds_vref_tr : 4;
		u64 pcs_sds_trim_chp_reg : 2;
		u64 pcs_sds_vco_reg_tr : 2;
		u64 pcs_sds_cvbg_en : 1;
		u64 pcs_sds_extvbg_en : 1;
	} s;
	struct cvmx_gserx_glbl_misc_config_1_s cn73xx;
	struct cvmx_gserx_glbl_misc_config_1_s cn78xx;
	struct cvmx_gserx_glbl_misc_config_1_s cn78xxp1;
	struct cvmx_gserx_glbl_misc_config_1_s cnf75xx;
};

typedef union cvmx_gserx_glbl_misc_config_1 cvmx_gserx_glbl_misc_config_1_t;

/**
 * cvmx_gser#_glbl_pll_cfg_3
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_glbl_pll_cfg_3 {
	u64 u64;
	struct cvmx_gserx_glbl_pll_cfg_3_s {
		u64 reserved_10_63 : 54;
		u64 pcs_sds_pll_vco_amp : 2;
		u64 pll_bypass_uq : 1;
		u64 pll_vctrl_sel_ovrrd_en : 1;
		u64 pll_vctrl_sel_ovrrd_val : 2;
		u64 pll_vctrl_sel_lcvco_val : 2;
		u64 pll_vctrl_sel_rovco_val : 2;
	} s;
	struct cvmx_gserx_glbl_pll_cfg_3_s cn73xx;
	struct cvmx_gserx_glbl_pll_cfg_3_s cn78xx;
	struct cvmx_gserx_glbl_pll_cfg_3_s cn78xxp1;
	struct cvmx_gserx_glbl_pll_cfg_3_s cnf75xx;
};

typedef union cvmx_gserx_glbl_pll_cfg_3 cvmx_gserx_glbl_pll_cfg_3_t;

/**
 * cvmx_gser#_dlm#_rx_data_en
 *
 * DLM Receiver Enable.
 *
 */
union cvmx_gserx_dlmx_rx_data_en {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_data_en_s {
		u64 reserved_9_63 : 55;
		u64 rx1_data_en : 1;
		u64 reserved_1_7 : 7;
		u64 rx0_data_en : 1;
	} s;
	struct cvmx_gserx_dlmx_rx_data_en_s cn70xx;
	struct cvmx_gserx_dlmx_rx_data_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_data_en cvmx_gserx_dlmx_rx_data_en_t;

/**
 * cvmx_gser#_dlm#_rx_pll_en
 *
 * DLM0 DPLL Enable.
 *
 */
union cvmx_gserx_dlmx_rx_pll_en {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_pll_en_s {
		u64 reserved_9_63 : 55;
		u64 rx1_pll_en : 1;
		u64 reserved_1_7 : 7;
		u64 rx0_pll_en : 1;
	} s;
	struct cvmx_gserx_dlmx_rx_pll_en_s cn70xx;
	struct cvmx_gserx_dlmx_rx_pll_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_pll_en cvmx_gserx_dlmx_rx_pll_en_t;

/**
 * cvmx_gser#_dlm#_rx_rate
 *
 * DLM0 Rx Data Rate.
 *
 */
union cvmx_gserx_dlmx_rx_rate {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_rate_s {
		u64 reserved_10_63 : 54;
		u64 rx1_rate : 2;
		u64 reserved_2_7 : 6;
		u64 rx0_rate : 2;
	} s;
	struct cvmx_gserx_dlmx_rx_rate_s cn70xx;
	struct cvmx_gserx_dlmx_rx_rate_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_rate cvmx_gserx_dlmx_rx_rate_t;

/**
 * cvmx_gser#_dlm#_rx_reset
 *
 * DLM0 Receiver Reset.
 *
 */
union cvmx_gserx_dlmx_rx_reset {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_reset_s {
		u64 reserved_9_63 : 55;
		u64 rx1_reset : 1;
		u64 reserved_1_7 : 7;
		u64 rx0_reset : 1;
	} s;
	struct cvmx_gserx_dlmx_rx_reset_s cn70xx;
	struct cvmx_gserx_dlmx_rx_reset_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_reset cvmx_gserx_dlmx_rx_reset_t;

/**
 * cvmx_gser#_dlm#_test_powerdown
 *
 * DLM Test Powerdown.
 *
 */
union cvmx_gserx_dlmx_test_powerdown {
	u64 u64;
	struct cvmx_gserx_dlmx_test_powerdown_s {
		u64 reserved_1_63 : 63;
		u64 test_powerdown : 1;
	} s;
	struct cvmx_gserx_dlmx_test_powerdown_s cn70xx;
	struct cvmx_gserx_dlmx_test_powerdown_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_test_powerdown cvmx_gserx_dlmx_test_powerdown_t;

/**
 * cvmx_gser#_dlm#_tx_amplitude
 *
 * DLM0 Tx Amplitude (Full Swing Mode).
 *
 */
union cvmx_gserx_dlmx_tx_amplitude {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_amplitude_s {
		u64 reserved_15_63 : 49;
		u64 tx1_amplitude : 7;
		u64 reserved_7_7 : 1;
		u64 tx0_amplitude : 7;
	} s;
	struct cvmx_gserx_dlmx_tx_amplitude_s cn70xx;
	struct cvmx_gserx_dlmx_tx_amplitude_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_amplitude cvmx_gserx_dlmx_tx_amplitude_t;

/**
 * cvmx_gser#_dlm#_tx_en
 *
 * DLM Transmit Clocking and Data Sampling Enable.
 *
 */
union cvmx_gserx_dlmx_tx_en {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_en_s {
		u64 reserved_9_63 : 55;
		u64 tx1_en : 1;
		u64 reserved_1_7 : 7;
		u64 tx0_en : 1;
	} s;
	struct cvmx_gserx_dlmx_tx_en_s cn70xx;
	struct cvmx_gserx_dlmx_tx_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_en cvmx_gserx_dlmx_tx_en_t;

/**
 * cvmx_gser#_dlm#_tx_preemph
 *
 * DLM0 Tx Deemphasis.
 *
 */
union cvmx_gserx_dlmx_tx_preemph {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_preemph_s {
		u64 reserved_15_63 : 49;
		u64 tx1_preemph : 7;
		u64 reserved_7_7 : 1;
		u64 tx0_preemph : 7;
	} s;
	struct cvmx_gserx_dlmx_tx_preemph_s cn70xx;
	struct cvmx_gserx_dlmx_tx_preemph_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_preemph cvmx_gserx_dlmx_tx_preemph_t;

/**
 * cvmx_gser#_dlm#_tx_status
 *
 * DLM Transmit Common Mode Control Status.
 *
 */
union cvmx_gserx_dlmx_tx_status {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_status_s {
		u64 reserved_10_63 : 54;
		u64 tx1_cm_status : 1;
		u64 tx1_status : 1;
		u64 reserved_2_7 : 6;
		u64 tx0_cm_status : 1;
		u64 tx0_status : 1;
	} s;
	struct cvmx_gserx_dlmx_tx_status_s cn70xx;
	struct cvmx_gserx_dlmx_tx_status_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_status cvmx_gserx_dlmx_tx_status_t;

/**
 * cvmx_gser#_dlm#_rx_status
 *
 * DLM Receive DPLL State Indicator.
 *
 */
union cvmx_gserx_dlmx_rx_status {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_status_s {
		u64 reserved_9_63 : 55;
		u64 rx1_status : 1;
		u64 reserved_1_7 : 7;
		u64 rx0_status : 1;
	} s;
	struct cvmx_gserx_dlmx_rx_status_s cn70xx;
	struct cvmx_gserx_dlmx_rx_status_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_status cvmx_gserx_dlmx_rx_status_t;

/**
 * cvmx_gser#_dlm#_tx_rate
 *
 * DLM0 Tx Data Rate.
 *
 */
union cvmx_gserx_dlmx_tx_rate {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_rate_s {
		u64 reserved_10_63 : 54;
		u64 tx1_rate : 2;
		u64 reserved_2_7 : 6;
		u64 tx0_rate : 2;
	} s;
	struct cvmx_gserx_dlmx_tx_rate_s cn70xx;
	struct cvmx_gserx_dlmx_tx_rate_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_rate cvmx_gserx_dlmx_tx_rate_t;

/**
 * cvmx_gser#_sata_status
 *
 * SATA PHY Ready Status.
 *
 */
union cvmx_gserx_sata_status {
	u64 u64;
	struct cvmx_gserx_sata_status_s {
		u64 reserved_2_63 : 62;
		u64 p1_rdy : 1;
		u64 p0_rdy : 1;
	} s;
	struct cvmx_gserx_sata_status_s cn70xx;
	struct cvmx_gserx_sata_status_s cn70xxp1;
	struct cvmx_gserx_sata_status_s cn73xx;
	struct cvmx_gserx_sata_status_s cnf75xx;
};

typedef union cvmx_gserx_sata_status cvmx_gserx_sata_status_t;

/**
 * cvmx_gser#_dlm#_tx_data_en
 *
 * DLM0 Transmit Driver Enable.
 *
 */
union cvmx_gserx_dlmx_tx_data_en {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_data_en_s {
		u64 reserved_9_63 : 55;
		u64 tx1_data_en : 1;
		u64 reserved_1_7 : 7;
		u64 tx0_data_en : 1;
	} s;
	struct cvmx_gserx_dlmx_tx_data_en_s cn70xx;
	struct cvmx_gserx_dlmx_tx_data_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_data_en cvmx_gserx_dlmx_tx_data_en_t;

/**
 * cvmx_gser#_dlm#_tx_cm_en
 *
 * DLM0 Transmit Common-Mode Control Enable.
 *
 */
union cvmx_gserx_dlmx_tx_cm_en {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_cm_en_s {
		u64 reserved_9_63 : 55;
		u64 tx1_cm_en : 1;
		u64 reserved_1_7 : 7;
		u64 tx0_cm_en : 1;
	} s;
	struct cvmx_gserx_dlmx_tx_cm_en_s cn70xx;
	struct cvmx_gserx_dlmx_tx_cm_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_cm_en cvmx_gserx_dlmx_tx_cm_en_t;

/**
 * cvmx_gser#_dlm#_tx_reset
 *
 * DLM0 Tx Reset.
 *
 */
union cvmx_gserx_dlmx_tx_reset {
	u64 u64;
	struct cvmx_gserx_dlmx_tx_reset_s {
		u64 reserved_9_63 : 55;
		u64 tx1_reset : 1;
		u64 reserved_1_7 : 7;
		u64 tx0_reset : 1;
	} s;
	struct cvmx_gserx_dlmx_tx_reset_s cn70xx;
	struct cvmx_gserx_dlmx_tx_reset_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_tx_reset cvmx_gserx_dlmx_tx_reset_t;

/**
 * cvmx_gser#_dlm#_mpll_status
 *
 * DLM PLL Lock Status.
 *
 */
union cvmx_gserx_dlmx_mpll_status {
	u64 u64;
	struct cvmx_gserx_dlmx_mpll_status_s {
		u64 reserved_1_63 : 63;
		u64 mpll_status : 1;
	} s;
	struct cvmx_gserx_dlmx_mpll_status_s cn70xx;
	struct cvmx_gserx_dlmx_mpll_status_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_mpll_status cvmx_gserx_dlmx_mpll_status_t;

/**
 * cvmx_gser#_dlm#_phy_reset
 *
 * DLM Core and State Machine Reset.
 *
 */
union cvmx_gserx_dlmx_phy_reset {
	u64 u64;
	struct cvmx_gserx_dlmx_phy_reset_s {
		u64 reserved_1_63 : 63;
		u64 phy_reset : 1;
	} s;
	struct cvmx_gserx_dlmx_phy_reset_s cn70xx;
	struct cvmx_gserx_dlmx_phy_reset_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_phy_reset cvmx_gserx_dlmx_phy_reset_t;

/**
 * cvmx_gser#_dlm#_ref_clkdiv2
 *
 * DLM Input Reference Clock Divider Control.
 *
 */
union cvmx_gserx_dlmx_ref_clkdiv2 {
	u64 u64;
	struct cvmx_gserx_dlmx_ref_clkdiv2_s {
		u64 reserved_1_63 : 63;
		u64 ref_clkdiv2 : 1;
	} s;
	struct cvmx_gserx_dlmx_ref_clkdiv2_s cn70xx;
	struct cvmx_gserx_dlmx_ref_clkdiv2_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_ref_clkdiv2 cvmx_gserx_dlmx_ref_clkdiv2_t;

/**
 * cvmx_gser#_dlm#_ref_ssp_en
 *
 * DLM0 Reference Clock Enable for the PHY.
 *
 */
union cvmx_gserx_dlmx_ref_ssp_en {
	u64 u64;
	struct cvmx_gserx_dlmx_ref_ssp_en_s {
		u64 reserved_1_63 : 63;
		u64 ref_ssp_en : 1;
	} s;
	struct cvmx_gserx_dlmx_ref_ssp_en_s cn70xx;
	struct cvmx_gserx_dlmx_ref_ssp_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_ref_ssp_en cvmx_gserx_dlmx_ref_ssp_en_t;

union cvmx_gserx_dlmx_mpll_en {
	u64 u64;
	struct cvmx_gserx_dlmx_mpll_en_s {
		u64 reserved_1_63 : 63;
		u64 mpll_en : 1;
	} s;
	struct cvmx_gserx_dlmx_mpll_en_s cn70xx;
	struct cvmx_gserx_dlmx_mpll_en_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_mpll_en cvmx_gserx_dlmx_mpll_en_t;

/**
 * cvmx_gser#_dlm#_rx_eq
 *
 * DLM Receiver Equalization Setting.
 *
 */
union cvmx_gserx_dlmx_rx_eq {
	u64 u64;
	struct cvmx_gserx_dlmx_rx_eq_s {
		u64 reserved_11_63 : 53;
		u64 rx1_eq : 3;
		u64 reserved_3_7 : 5;
		u64 rx0_eq : 3;
	} s;
	struct cvmx_gserx_dlmx_rx_eq_s cn70xx;
	struct cvmx_gserx_dlmx_rx_eq_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_rx_eq cvmx_gserx_dlmx_rx_eq_t;

/**
 * cvmx_gser#_dlm#_mpll_multiplier
 *
 * DLM MPLL Frequency Multiplier Control.
 *
 */
union cvmx_gserx_dlmx_mpll_multiplier {
	u64 u64;
	struct cvmx_gserx_dlmx_mpll_multiplier_s {
		u64 reserved_7_63 : 57;
		u64 mpll_multiplier : 7;
	} s;
	struct cvmx_gserx_dlmx_mpll_multiplier_s cn70xx;
	struct cvmx_gserx_dlmx_mpll_multiplier_s cn70xxp1;
};

typedef union cvmx_gserx_dlmx_mpll_multiplier cvmx_gserx_dlmx_mpll_multiplier_t;

/**
 * cvmx_gser#_br_rx#_ctl
 */
union cvmx_gserx_br_rxx_ctl {
	u64 u64;
	struct cvmx_gserx_br_rxx_ctl_s {
		u64 reserved_4_63 : 60;
		u64 rxt_adtmout_disable : 1;
		u64 rxt_swm : 1;
		u64 rxt_preset : 1;
		u64 rxt_initialize : 1;
	} s;
	struct cvmx_gserx_br_rxx_ctl_s cn73xx;
	struct cvmx_gserx_br_rxx_ctl_s cn78xx;
	struct cvmx_gserx_br_rxx_ctl_cn78xxp1 {
		u64 reserved_3_63 : 61;
		u64 rxt_swm : 1;
		u64 rxt_preset : 1;
		u64 rxt_initialize : 1;
	} cn78xxp1;
	struct cvmx_gserx_br_rxx_ctl_s cnf75xx;
};

typedef union cvmx_gserx_br_rxx_ctl cvmx_gserx_br_rxx_ctl_t;

/**
 * cvmx_gser#_br_rx#_eer
 *
 * GSER software BASE-R RX link training equalization evaluation request (EER). A write to
 * [RXT_EER] initiates a equalization request to the RAW PCS. A read of this register returns the
 * equalization status message and a valid bit indicating it was updated. These registers are for
 * diagnostic use only.
 */
union cvmx_gserx_br_rxx_eer {
	u64 u64;
	struct cvmx_gserx_br_rxx_eer_s {
		u64 reserved_16_63 : 48;
		u64 rxt_eer : 1;
		u64 rxt_esv : 1;
		u64 rxt_esm : 14;
	} s;
	struct cvmx_gserx_br_rxx_eer_s cn73xx;
	struct cvmx_gserx_br_rxx_eer_s cn78xx;
	struct cvmx_gserx_br_rxx_eer_s cn78xxp1;
	struct cvmx_gserx_br_rxx_eer_s cnf75xx;
};

typedef union cvmx_gserx_br_rxx_eer cvmx_gserx_br_rxx_eer_t;

/**
 * cvmx_gser#_pcie_pipe_port_sel
 *
 * PCIE PIPE Enable Request.
 *
 */
union cvmx_gserx_pcie_pipe_port_sel {
	u64 u64;
	struct cvmx_gserx_pcie_pipe_port_sel_s {
		u64 reserved_3_63 : 61;
		u64 cfg_pem1_dlm2 : 1;
		u64 pipe_port_sel : 2;
	} s;
	struct cvmx_gserx_pcie_pipe_port_sel_s cn70xx;
	struct cvmx_gserx_pcie_pipe_port_sel_s cn70xxp1;
};

typedef union cvmx_gserx_pcie_pipe_port_sel cvmx_gserx_pcie_pipe_port_sel_t;

/**
 * cvmx_gser#_pcie_pipe_rst
 *
 * PCIE PIPE Reset.
 *
 */
union cvmx_gserx_pcie_pipe_rst {
	u64 u64;
	struct cvmx_gserx_pcie_pipe_rst_s {
		u64 reserved_4_63 : 60;
		u64 pipe3_rst : 1;
		u64 pipe2_rst : 1;
		u64 pipe1_rst : 1;
		u64 pipe0_rst : 1;
	} s;
	struct cvmx_gserx_pcie_pipe_rst_s cn70xx;
	struct cvmx_gserx_pcie_pipe_rst_s cn70xxp1;
};

typedef union cvmx_gserx_pcie_pipe_rst cvmx_gserx_pcie_pipe_rst_t;

/**
 * cvmx_gser#_sata_cfg
 *
 * SATA Config Enable.
 *
 */
union cvmx_gserx_sata_cfg {
	u64 u64;
	struct cvmx_gserx_sata_cfg_s {
		u64 reserved_1_63 : 63;
		u64 sata_en : 1;
	} s;
	struct cvmx_gserx_sata_cfg_s cn70xx;
	struct cvmx_gserx_sata_cfg_s cn70xxp1;
};

typedef union cvmx_gserx_sata_cfg cvmx_gserx_sata_cfg_t;

/**
 * cvmx_gser#_sata_lane_rst
 *
 * Lane Reset Control.
 *
 */
union cvmx_gserx_sata_lane_rst {
	u64 u64;
	struct cvmx_gserx_sata_lane_rst_s {
		u64 reserved_2_63 : 62;
		u64 l1_rst : 1;
		u64 l0_rst : 1;
	} s;
	struct cvmx_gserx_sata_lane_rst_s cn70xx;
	struct cvmx_gserx_sata_lane_rst_s cn70xxp1;
	struct cvmx_gserx_sata_lane_rst_s cn73xx;
	struct cvmx_gserx_sata_lane_rst_s cnf75xx;
};

typedef union cvmx_gserx_sata_lane_rst cvmx_gserx_sata_lane_rst_t;

/**
 * cvmx_gser#_sata_ref_ssp_en
 *
 * SATA Reference Clock Enable for the PHY.
 *
 */
union cvmx_gserx_sata_ref_ssp_en {
	u64 u64;
	struct cvmx_gserx_sata_ref_ssp_en_s {
		u64 reserved_1_63 : 63;
		u64 ref_ssp_en : 1;
	} s;
	struct cvmx_gserx_sata_ref_ssp_en_s cn70xx;
	struct cvmx_gserx_sata_ref_ssp_en_s cn70xxp1;
};

typedef union cvmx_gserx_sata_ref_ssp_en cvmx_gserx_sata_ref_ssp_en_t;

/**
 * cvmx_gser#_phy#_ovrd_in_lo
 *
 * PHY Override Input Low Register.
 *
 */
union cvmx_gserx_phyx_ovrd_in_lo {
	u64 u64;
	struct cvmx_gserx_phyx_ovrd_in_lo_s {
		u64 reserved_16_63 : 48;
		u64 res_ack_in_ovrd : 1;
		u64 res_ack_in : 1;
		u64 res_req_in_ovrd : 1;
		u64 res_req_in : 1;
		u64 rtune_req_ovrd : 1;
		u64 rtune_req : 1;
		u64 mpll_multiplier_ovrd : 1;
		u64 mpll_multiplier : 7;
		u64 mpll_en_ovrd : 1;
		u64 mpll_en : 1;
	} s;
	struct cvmx_gserx_phyx_ovrd_in_lo_s cn70xx;
	struct cvmx_gserx_phyx_ovrd_in_lo_s cn70xxp1;
};

typedef union cvmx_gserx_phyx_ovrd_in_lo cvmx_gserx_phyx_ovrd_in_lo_t;

/**
 * cvmx_gser#_phy_ctl
 *
 * This register contains general PHY/PLL control of the RAW PCS.
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_phy_ctl {
	u64 u64;
	struct cvmx_gserx_phy_ctl_s {
		u64 reserved_2_63 : 62;
		u64 phy_reset : 1;
		u64 phy_pd : 1;
	} s;
	struct cvmx_gserx_phy_ctl_s cn73xx;
	struct cvmx_gserx_phy_ctl_s cn78xx;
	struct cvmx_gserx_phy_ctl_s cn78xxp1;
	struct cvmx_gserx_phy_ctl_s cnf75xx;
};

typedef union cvmx_gserx_phy_ctl cvmx_gserx_phy_ctl_t;

/**
 * cvmx_gser#_rx_pwr_ctrl_p1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_rx_pwr_ctrl_p1 {
	u64 u64;
	struct cvmx_gserx_rx_pwr_ctrl_p1_s {
		u64 reserved_14_63 : 50;
		u64 p1_rx_resetn : 1;
		u64 pq_rx_allow_pll_pd : 1;
		u64 pq_rx_pcs_reset : 1;
		u64 p1_rx_agc_en : 1;
		u64 p1_rx_dfe_en : 1;
		u64 p1_rx_cdr_en : 1;
		u64 p1_rx_cdr_coast : 1;
		u64 p1_rx_cdr_clr : 1;
		u64 p1_rx_subblk_pd : 5;
		u64 p1_rx_chpd : 1;
	} s;
	struct cvmx_gserx_rx_pwr_ctrl_p1_s cn73xx;
	struct cvmx_gserx_rx_pwr_ctrl_p1_s cn78xx;
	struct cvmx_gserx_rx_pwr_ctrl_p1_s cn78xxp1;
	struct cvmx_gserx_rx_pwr_ctrl_p1_s cnf75xx;
};

typedef union cvmx_gserx_rx_pwr_ctrl_p1 cvmx_gserx_rx_pwr_ctrl_p1_t;

/**
 * cvmx_gser#_rx_pwr_ctrl_p2
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_rx_pwr_ctrl_p2 {
	u64 u64;
	struct cvmx_gserx_rx_pwr_ctrl_p2_s {
		u64 reserved_14_63 : 50;
		u64 p2_rx_resetn : 1;
		u64 p2_rx_allow_pll_pd : 1;
		u64 p2_rx_pcs_reset : 1;
		u64 p2_rx_agc_en : 1;
		u64 p2_rx_dfe_en : 1;
		u64 p2_rx_cdr_en : 1;
		u64 p2_rx_cdr_coast : 1;
		u64 p2_rx_cdr_clr : 1;
		u64 p2_rx_subblk_pd : 5;
		u64 p2_rx_chpd : 1;
	} s;
	struct cvmx_gserx_rx_pwr_ctrl_p2_s cn73xx;
	struct cvmx_gserx_rx_pwr_ctrl_p2_s cn78xx;
	struct cvmx_gserx_rx_pwr_ctrl_p2_s cn78xxp1;
	struct cvmx_gserx_rx_pwr_ctrl_p2_s cnf75xx;
};

typedef union cvmx_gserx_rx_pwr_ctrl_p2 cvmx_gserx_rx_pwr_ctrl_p2_t;

/**
 * cvmx_gser#_rx_txdir_ctrl_0
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_rx_txdir_ctrl_0 {
	u64 u64;
	struct cvmx_gserx_rx_txdir_ctrl_0_s {
		u64 reserved_13_63 : 51;
		u64 rx_boost_hi_thrs : 4;
		u64 rx_boost_lo_thrs : 4;
		u64 rx_boost_hi_val : 5;
	} s;
	struct cvmx_gserx_rx_txdir_ctrl_0_s cn73xx;
	struct cvmx_gserx_rx_txdir_ctrl_0_s cn78xx;
	struct cvmx_gserx_rx_txdir_ctrl_0_s cn78xxp1;
	struct cvmx_gserx_rx_txdir_ctrl_0_s cnf75xx;
};

typedef union cvmx_gserx_rx_txdir_ctrl_0 cvmx_gserx_rx_txdir_ctrl_0_t;

/**
 * cvmx_gser#_rx_txdir_ctrl_1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_rx_txdir_ctrl_1 {
	u64 u64;
	struct cvmx_gserx_rx_txdir_ctrl_1_s {
		u64 reserved_12_63 : 52;
		u64 rx_precorr_chg_dir : 1;
		u64 rx_tap1_chg_dir : 1;
		u64 rx_tap1_hi_thrs : 5;
		u64 rx_tap1_lo_thrs : 5;
	} s;
	struct cvmx_gserx_rx_txdir_ctrl_1_s cn73xx;
	struct cvmx_gserx_rx_txdir_ctrl_1_s cn78xx;
	struct cvmx_gserx_rx_txdir_ctrl_1_s cn78xxp1;
	struct cvmx_gserx_rx_txdir_ctrl_1_s cnf75xx;
};

typedef union cvmx_gserx_rx_txdir_ctrl_1 cvmx_gserx_rx_txdir_ctrl_1_t;

/**
 * cvmx_gser#_rx_txdir_ctrl_2
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_rx_txdir_ctrl_2 {
	u64 u64;
	struct cvmx_gserx_rx_txdir_ctrl_2_s {
		u64 reserved_16_63 : 48;
		u64 rx_precorr_hi_thrs : 8;
		u64 rx_precorr_lo_thrs : 8;
	} s;
	struct cvmx_gserx_rx_txdir_ctrl_2_s cn73xx;
	struct cvmx_gserx_rx_txdir_ctrl_2_s cn78xx;
	struct cvmx_gserx_rx_txdir_ctrl_2_s cn78xxp1;
	struct cvmx_gserx_rx_txdir_ctrl_2_s cnf75xx;
};

typedef union cvmx_gserx_rx_txdir_ctrl_2 cvmx_gserx_rx_txdir_ctrl_2_t;

/**
 * cvmx_gser#_rx_eie_detsts
 */
union cvmx_gserx_rx_eie_detsts {
	u64 u64;
	struct cvmx_gserx_rx_eie_detsts_s {
		u64 reserved_12_63 : 52;
		u64 cdrlock : 4;
		u64 eiests : 4;
		u64 eieltch : 4;
	} s;
	struct cvmx_gserx_rx_eie_detsts_s cn73xx;
	struct cvmx_gserx_rx_eie_detsts_s cn78xx;
	struct cvmx_gserx_rx_eie_detsts_s cn78xxp1;
	struct cvmx_gserx_rx_eie_detsts_s cnf75xx;
};

typedef union cvmx_gserx_rx_eie_detsts cvmx_gserx_rx_eie_detsts_t;

/**
 * cvmx_gser#_refclk_sel
 *
 * This register selects the reference clock.
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 *
 * Not used with GSER6, GSER7, and GSER8.
 */
union cvmx_gserx_refclk_sel {
	u64 u64;
	struct cvmx_gserx_refclk_sel_s {
		u64 reserved_3_63 : 61;
		u64 pcie_refclk125 : 1;
		u64 com_clk_sel : 1;
		u64 use_com1 : 1;
	} s;
	struct cvmx_gserx_refclk_sel_s cn73xx;
	struct cvmx_gserx_refclk_sel_s cn78xx;
	struct cvmx_gserx_refclk_sel_s cn78xxp1;
	struct cvmx_gserx_refclk_sel_s cnf75xx;
};

typedef union cvmx_gserx_refclk_sel cvmx_gserx_refclk_sel_t;

/**
 * cvmx_gser#_lane#_lbert_cfg
 *
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_lbert_cfg {
	u64 u64;
	struct cvmx_gserx_lanex_lbert_cfg_s {
		u64 reserved_16_63 : 48;
		u64 lbert_pg_err_insert : 1;
		u64 lbert_pm_sync_start : 1;
		u64 lbert_pg_en : 1;
		u64 lbert_pg_width : 2;
		u64 lbert_pg_mode : 4;
		u64 lbert_pm_en : 1;
		u64 lbert_pm_width : 2;
		u64 lbert_pm_mode : 4;
	} s;
	struct cvmx_gserx_lanex_lbert_cfg_s cn73xx;
	struct cvmx_gserx_lanex_lbert_cfg_s cn78xx;
	struct cvmx_gserx_lanex_lbert_cfg_s cn78xxp1;
	struct cvmx_gserx_lanex_lbert_cfg_s cnf75xx;
};

typedef union cvmx_gserx_lanex_lbert_cfg cvmx_gserx_lanex_lbert_cfg_t;

/**
 * cvmx_gser#_lane#_misc_cfg_0
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_misc_cfg_0 {
	u64 u64;
	struct cvmx_gserx_lanex_misc_cfg_0_s {
		u64 reserved_16_63 : 48;
		u64 use_pma_polarity : 1;
		u64 cfg_pcs_loopback : 1;
		u64 pcs_tx_mode_ovrrd_en : 1;
		u64 pcs_rx_mode_ovrrd_en : 1;
		u64 cfg_eie_det_cnt : 4;
		u64 eie_det_stl_on_time : 3;
		u64 eie_det_stl_off_time : 3;
		u64 tx_bit_order : 1;
		u64 rx_bit_order : 1;
	} s;
	struct cvmx_gserx_lanex_misc_cfg_0_s cn73xx;
	struct cvmx_gserx_lanex_misc_cfg_0_s cn78xx;
	struct cvmx_gserx_lanex_misc_cfg_0_s cn78xxp1;
	struct cvmx_gserx_lanex_misc_cfg_0_s cnf75xx;
};

typedef union cvmx_gserx_lanex_misc_cfg_0 cvmx_gserx_lanex_misc_cfg_0_t;

/**
 * cvmx_gser#_lane_p#_mode_0
 *
 * These are the RAW PCS lane settings mode 0 registers. There is one register per
 * 4 lanes per GSER per GSER_LMODE_E value (0..11). Only one entry is used at any given time in a
 * given GSER lane - the one selected by the corresponding GSER()_LANE_MODE[LMODE].
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_px_mode_0 {
	u64 u64;
	struct cvmx_gserx_lane_px_mode_0_s {
		u64 reserved_15_63 : 49;
		u64 ctle : 2;
		u64 pcie : 1;
		u64 tx_ldiv : 2;
		u64 rx_ldiv : 2;
		u64 srate : 3;
		u64 reserved_4_4 : 1;
		u64 tx_mode : 2;
		u64 rx_mode : 2;
	} s;
	struct cvmx_gserx_lane_px_mode_0_s cn73xx;
	struct cvmx_gserx_lane_px_mode_0_s cn78xx;
	struct cvmx_gserx_lane_px_mode_0_s cn78xxp1;
	struct cvmx_gserx_lane_px_mode_0_s cnf75xx;
};

typedef union cvmx_gserx_lane_px_mode_0 cvmx_gserx_lane_px_mode_0_t;

/**
 * cvmx_gser#_lane_p#_mode_1
 *
 * These are the RAW PCS lane settings mode 1 registers. There is one register per 4 lanes,
 * (0..3) per GSER per GSER_LMODE_E value (0..11). Only one entry is used at any given time in a
 * given
 * GSER lane - the one selected by the corresponding GSER()_LANE_MODE[LMODE].
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_px_mode_1 {
	u64 u64;
	struct cvmx_gserx_lane_px_mode_1_s {
		u64 reserved_16_63 : 48;
		u64 vma_fine_cfg_sel : 1;
		u64 vma_mm : 1;
		u64 cdr_fgain : 4;
		u64 ph_acc_adj : 10;
	} s;
	struct cvmx_gserx_lane_px_mode_1_s cn73xx;
	struct cvmx_gserx_lane_px_mode_1_s cn78xx;
	struct cvmx_gserx_lane_px_mode_1_s cn78xxp1;
	struct cvmx_gserx_lane_px_mode_1_s cnf75xx;
};

typedef union cvmx_gserx_lane_px_mode_1 cvmx_gserx_lane_px_mode_1_t;

/**
 * cvmx_gser#_lane#_rx_loop_ctrl
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_loop_ctrl {
	u64 u64;
	struct cvmx_gserx_lanex_rx_loop_ctrl_s {
		u64 reserved_12_63 : 52;
		u64 fast_dll_lock : 1;
		u64 fast_ofst_cncl : 1;
		u64 cfg_rx_lctrl : 10;
	} s;
	struct cvmx_gserx_lanex_rx_loop_ctrl_s cn73xx;
	struct cvmx_gserx_lanex_rx_loop_ctrl_s cn78xx;
	struct cvmx_gserx_lanex_rx_loop_ctrl_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_loop_ctrl_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_loop_ctrl cvmx_gserx_lanex_rx_loop_ctrl_t;

/**
 * cvmx_gser#_lane#_rx_valbbd_ctrl_0
 *
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_valbbd_ctrl_0 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_0_s {
		u64 reserved_14_63 : 50;
		u64 agc_gain : 2;
		u64 dfe_gain : 2;
		u64 dfe_c5_mval : 4;
		u64 dfe_c5_msgn : 1;
		u64 dfe_c4_mval : 4;
		u64 dfe_c4_msgn : 1;
	} s;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_0_s cn73xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_0_s cn78xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_0_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_0_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_valbbd_ctrl_0 cvmx_gserx_lanex_rx_valbbd_ctrl_0_t;

/**
 * cvmx_gser#_lane#_rx_valbbd_ctrl_1
 *
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_valbbd_ctrl_1 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_1_s {
		u64 reserved_15_63 : 49;
		u64 dfe_c3_mval : 4;
		u64 dfe_c3_msgn : 1;
		u64 dfe_c2_mval : 4;
		u64 dfe_c2_msgn : 1;
		u64 dfe_c1_mval : 4;
		u64 dfe_c1_msgn : 1;
	} s;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_1_s cn73xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_1_s cn78xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_1_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_1_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_valbbd_ctrl_1 cvmx_gserx_lanex_rx_valbbd_ctrl_1_t;

/**
 * cvmx_gser#_lane#_rx_valbbd_ctrl_2
 *
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_valbbd_ctrl_2 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_2_s {
		u64 reserved_6_63 : 58;
		u64 dfe_ovrd_en : 1;
		u64 dfe_c5_ovrd_val : 1;
		u64 dfe_c4_ovrd_val : 1;
		u64 dfe_c3_ovrd_val : 1;
		u64 dfe_c2_ovrd_val : 1;
		u64 dfe_c1_ovrd_val : 1;
	} s;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_2_s cn73xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_2_s cn78xx;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_2_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_valbbd_ctrl_2_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_valbbd_ctrl_2 cvmx_gserx_lanex_rx_valbbd_ctrl_2_t;

/**
 * cvmx_gser#_lane_vma_fine_ctrl_0
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_vma_fine_ctrl_0 {
	u64 u64;
	struct cvmx_gserx_lane_vma_fine_ctrl_0_s {
		u64 reserved_16_63 : 48;
		u64 rx_sdll_iq_max_fine : 4;
		u64 rx_sdll_iq_min_fine : 4;
		u64 rx_sdll_iq_step_fine : 2;
		u64 vma_window_wait_fine : 3;
		u64 lms_wait_time_fine : 3;
	} s;
	struct cvmx_gserx_lane_vma_fine_ctrl_0_s cn73xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_0_s cn78xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_0_s cn78xxp1;
	struct cvmx_gserx_lane_vma_fine_ctrl_0_s cnf75xx;
};

typedef union cvmx_gserx_lane_vma_fine_ctrl_0 cvmx_gserx_lane_vma_fine_ctrl_0_t;

/**
 * cvmx_gser#_lane_vma_fine_ctrl_1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_vma_fine_ctrl_1 {
	u64 u64;
	struct cvmx_gserx_lane_vma_fine_ctrl_1_s {
		u64 reserved_10_63 : 54;
		u64 rx_ctle_peak_max_fine : 4;
		u64 rx_ctle_peak_min_fine : 4;
		u64 rx_ctle_peak_step_fine : 2;
	} s;
	struct cvmx_gserx_lane_vma_fine_ctrl_1_s cn73xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_1_s cn78xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_1_s cn78xxp1;
	struct cvmx_gserx_lane_vma_fine_ctrl_1_s cnf75xx;
};

typedef union cvmx_gserx_lane_vma_fine_ctrl_1 cvmx_gserx_lane_vma_fine_ctrl_1_t;

/**
 * cvmx_gser#_lane_vma_fine_ctrl_2
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_vma_fine_ctrl_2 {
	u64 u64;
	struct cvmx_gserx_lane_vma_fine_ctrl_2_s {
		u64 reserved_10_63 : 54;
		u64 rx_prectle_gain_max_fine : 4;
		u64 rx_prectle_gain_min_fine : 4;
		u64 rx_prectle_gain_step_fine : 2;
	} s;
	struct cvmx_gserx_lane_vma_fine_ctrl_2_s cn73xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_2_s cn78xx;
	struct cvmx_gserx_lane_vma_fine_ctrl_2_s cn78xxp1;
	struct cvmx_gserx_lane_vma_fine_ctrl_2_s cnf75xx;
};

typedef union cvmx_gserx_lane_vma_fine_ctrl_2 cvmx_gserx_lane_vma_fine_ctrl_2_t;

/**
 * cvmx_gser#_lane#_pwr_ctrl
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_pwr_ctrl {
	u64 u64;
	struct cvmx_gserx_lanex_pwr_ctrl_s {
		u64 reserved_15_63 : 49;
		u64 tx_sds_fifo_reset_ovrrd_en : 1;
		u64 tx_sds_fifo_reset_ovrrd_val : 1;
		u64 tx_pcs_reset_ovrrd_val : 1;
		u64 rx_pcs_reset_ovrrd_val : 1;
		u64 reserved_9_10 : 2;
		u64 rx_resetn_ovrrd_en : 1;
		u64 rx_resetn_ovrrd_val : 1;
		u64 rx_lctrl_ovrrd_en : 1;
		u64 rx_lctrl_ovrrd_val : 1;
		u64 tx_tristate_en_ovrrd_en : 1;
		u64 tx_pcs_reset_ovrrd_en : 1;
		u64 tx_elec_idle_ovrrd_en : 1;
		u64 tx_pd_ovrrd_en : 1;
		u64 tx_p2s_resetn_ovrrd_en : 1;
	} s;
	struct cvmx_gserx_lanex_pwr_ctrl_cn73xx {
		u64 reserved_15_63 : 49;
		u64 tx_sds_fifo_reset_ovrrd_en : 1;
		u64 tx_sds_fifo_reset_ovrrd_val : 1;
		u64 tx_pcs_reset_ovrrd_val : 1;
		u64 rx_pcs_reset_ovrrd_val : 1;
		u64 reserved_10_9 : 2;
		u64 rx_resetn_ovrrd_en : 1;
		u64 rx_resetn_ovrrd_val : 1;
		u64 rx_lctrl_ovrrd_en : 1;
		u64 rx_lctrl_ovrrd_val : 1;
		u64 tx_tristate_en_ovrrd_en : 1;
		u64 tx_pcs_reset_ovrrd_en : 1;
		u64 tx_elec_idle_ovrrd_en : 1;
		u64 tx_pd_ovrrd_en : 1;
		u64 tx_p2s_resetn_ovrrd_en : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_pwr_ctrl_cn73xx cn78xx;
	struct cvmx_gserx_lanex_pwr_ctrl_s cn78xxp1;
	struct cvmx_gserx_lanex_pwr_ctrl_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_pwr_ctrl cvmx_gserx_lanex_pwr_ctrl_t;

/**
 * cvmx_gser#_lane#_rx_cfg_0
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_0 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_0_s {
		u64 reserved_16_63 : 48;
		u64 rx_datarate_ovrrd_en : 1;
		u64 reserved_14_14 : 1;
		u64 rx_resetn_ovrrd_val : 1;
		u64 pcs_sds_rx_eyemon_en : 1;
		u64 pcs_sds_rx_pcm_ctrl : 4;
		u64 rx_datarate_ovrrd_val : 2;
		u64 cfg_rx_pol_invert : 1;
		u64 rx_subblk_pd_ovrrd_val : 5;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_0_cn73xx {
		u64 reserved_16_63 : 48;
		u64 rx_datarate_ovrrd_en : 1;
		u64 pcs_rx_tristate_enable : 1;
		u64 rx_resetn_ovrrd_val : 1;
		u64 pcs_sds_rx_eyemon_en : 1;
		u64 pcs_sds_rx_pcm_ctrl : 4;
		u64 rx_datarate_ovrrd_val : 2;
		u64 cfg_rx_pol_invert : 1;
		u64 rx_subblk_pd_ovrrd_val : 5;
	} cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_0_cn78xx {
		u64 reserved_16_63 : 48;
		u64 rx_datarate_ovrrd_en : 1;
		u64 pcs_sds_rx_tristate_enable : 1;
		u64 rx_resetn_ovrrd_val : 1;
		u64 pcs_sds_rx_eyemon_en : 1;
		u64 pcs_sds_rx_pcm_ctrl : 4;
		u64 rx_datarate_ovrrd_val : 2;
		u64 cfg_rx_pol_invert : 1;
		u64 rx_subblk_pd_ovrrd_val : 5;
	} cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_0_cn78xx cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_0_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_0 cvmx_gserx_lanex_rx_cfg_0_t;

/**
 * cvmx_gser#_lane#_rx_cfg_1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_1 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_1_s {
		u64 reserved_16_63 : 48;
		u64 rx_chpd_ovrrd_val : 1;
		u64 pcs_sds_rx_os_men : 1;
		u64 eie_en_ovrrd_en : 1;
		u64 eie_en_ovrrd_val : 1;
		u64 reserved_11_11 : 1;
		u64 rx_pcie_mode_ovrrd_en : 1;
		u64 rx_pcie_mode_ovrrd_val : 1;
		u64 cfg_rx_dll_locken : 1;
		u64 pcs_sds_rx_cdr_ssc_mode : 8;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_1_s cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_1_s cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_1_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_1_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_1 cvmx_gserx_lanex_rx_cfg_1_t;

/**
 * cvmx_gser#_lane#_rx_cfg_2
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_2 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_2_s {
		u64 reserved_15_63 : 49;
		u64 pcs_sds_rx_terminate_to_vdda : 1;
		u64 pcs_sds_rx_sampler_boost : 2;
		u64 pcs_sds_rx_sampler_boost_en : 1;
		u64 reserved_10_10 : 1;
		u64 rx_sds_rx_agc_mval : 10;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_2_s cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_2_s cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_2_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_2_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_2 cvmx_gserx_lanex_rx_cfg_2_t;

/**
 * cvmx_gser#_lane#_rx_cfg_3
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_3 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_3_s {
		u64 reserved_16_63 : 48;
		u64 cfg_rx_errdet_ctrl : 16;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_3_s cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_3_s cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_3_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_3_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_3 cvmx_gserx_lanex_rx_cfg_3_t;

/**
 * cvmx_gser#_lane#_rx_cfg_4
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_4 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_4_s {
		u64 reserved_16_63 : 48;
		u64 cfg_rx_errdet_ctrl : 16;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_4_s cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_4_s cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_4_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_4_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_4 cvmx_gserx_lanex_rx_cfg_4_t;

/**
 * cvmx_gser#_lane#_rx_cfg_5
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_cfg_5 {
	u64 u64;
	struct cvmx_gserx_lanex_rx_cfg_5_s {
		u64 reserved_5_63 : 59;
		u64 rx_agc_men_ovrrd_en : 1;
		u64 rx_agc_men_ovrrd_val : 1;
		u64 rx_widthsel_ovrrd_en : 1;
		u64 rx_widthsel_ovrrd_val : 2;
	} s;
	struct cvmx_gserx_lanex_rx_cfg_5_s cn73xx;
	struct cvmx_gserx_lanex_rx_cfg_5_s cn78xx;
	struct cvmx_gserx_lanex_rx_cfg_5_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_cfg_5_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_cfg_5 cvmx_gserx_lanex_rx_cfg_5_t;

/**
 * cvmx_gser#_lane#_rx_ctle_ctrl
 *
 * These are the RAW PCS per-lane RX CTLE control registers.
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_ctle_ctrl {
	u64 u64;
	struct cvmx_gserx_lanex_rx_ctle_ctrl_s {
		u64 reserved_16_63 : 48;
		u64 pcs_sds_rx_ctle_bias_ctrl : 2;
		u64 pcs_sds_rx_ctle_zero : 4;
		u64 rx_ctle_pole_ovrrd_en : 1;
		u64 rx_ctle_pole_ovrrd_val : 4;
		u64 pcs_sds_rx_ctle_pole_max : 2;
		u64 pcs_sds_rx_ctle_pole_min : 2;
		u64 pcs_sds_rx_ctle_pole_step : 1;
	} s;
	struct cvmx_gserx_lanex_rx_ctle_ctrl_s cn73xx;
	struct cvmx_gserx_lanex_rx_ctle_ctrl_s cn78xx;
	struct cvmx_gserx_lanex_rx_ctle_ctrl_s cn78xxp1;
	struct cvmx_gserx_lanex_rx_ctle_ctrl_s cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_ctle_ctrl cvmx_gserx_lanex_rx_ctle_ctrl_t;

/**
 * cvmx_gser#_lane#_rx_misc_ovrrd
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_rx_misc_ovrrd {
	u64 u64;
	struct cvmx_gserx_lanex_rx_misc_ovrrd_s {
		u64 reserved_14_63 : 50;
		u64 cfg_rx_oob_clk_en_ovrrd_val : 1;
		u64 cfg_rx_oob_clk_en_ovrrd_en : 1;
		u64 cfg_rx_eie_det_ovrrd_val : 1;
		u64 cfg_rx_eie_det_ovrrd_en : 1;
		u64 cfg_rx_cdr_ctrl_ovrrd_en : 1;
		u64 cfg_rx_eq_eval_ovrrd_val : 1;
		u64 cfg_rx_eq_eval_ovrrd_en : 1;
		u64 reserved_6_6 : 1;
		u64 cfg_rx_dll_locken_ovrrd_en : 1;
		u64 cfg_rx_errdet_ctrl_ovrrd_en : 1;
		u64 reserved_1_3 : 3;
		u64 cfg_rxeq_eval_restore_en : 1;
	} s;
	struct cvmx_gserx_lanex_rx_misc_ovrrd_cn73xx {
		u64 reserved_14_63 : 50;
		u64 cfg_rx_oob_clk_en_ovrrd_val : 1;
		u64 cfg_rx_oob_clk_en_ovrrd_en : 1;
		u64 cfg_rx_eie_det_ovrrd_val : 1;
		u64 cfg_rx_eie_det_ovrrd_en : 1;
		u64 cfg_rx_cdr_ctrl_ovrrd_en : 1;
		u64 cfg_rx_eq_eval_ovrrd_val : 1;
		u64 cfg_rx_eq_eval_ovrrd_en : 1;
		u64 reserved_6_6 : 1;
		u64 cfg_rx_dll_locken_ovrrd_en : 1;
		u64 cfg_rx_errdet_ctrl_ovrrd_en : 1;
		u64 reserved_3_1 : 3;
		u64 cfg_rxeq_eval_restore_en : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_rx_misc_ovrrd_cn73xx cn78xx;
	struct cvmx_gserx_lanex_rx_misc_ovrrd_cn78xxp1 {
		u64 reserved_14_63 : 50;
		u64 cfg_rx_oob_clk_en_ovrrd_val : 1;
		u64 cfg_rx_oob_clk_en_ovrrd_en : 1;
		u64 cfg_rx_eie_det_ovrrd_val : 1;
		u64 cfg_rx_eie_det_ovrrd_en : 1;
		u64 cfg_rx_cdr_ctrl_ovrrd_en : 1;
		u64 cfg_rx_eq_eval_ovrrd_val : 1;
		u64 cfg_rx_eq_eval_ovrrd_en : 1;
		u64 reserved_6_6 : 1;
		u64 cfg_rx_dll_locken_ovrrd_en : 1;
		u64 cfg_rx_errdet_ctrl_ovrrd_en : 1;
		u64 reserved_0_3 : 4;
	} cn78xxp1;
	struct cvmx_gserx_lanex_rx_misc_ovrrd_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_rx_misc_ovrrd cvmx_gserx_lanex_rx_misc_ovrrd_t;

/**
 * cvmx_gser#_lane#_tx_cfg_0
 *
 * These registers are reset by hardware only during chip cold reset. The
 * values of the CSR fields in these registers do not change during chip
 * warm or soft resets.
 */
union cvmx_gserx_lanex_tx_cfg_0 {
	u64 u64;
	struct cvmx_gserx_lanex_tx_cfg_0_s {
		u64 reserved_16_63 : 48;
		u64 tx_tristate_en_ovrrd_val : 1;
		u64 tx_chpd_ovrrd_val : 1;
		u64 reserved_10_13 : 4;
		u64 tx_resetn_ovrrd_val : 1;
		u64 tx_cm_mode : 1;
		u64 cfg_tx_swing : 5;
		u64 fast_rdet_mode : 1;
		u64 fast_tristate_mode : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_gserx_lanex_tx_cfg_0_cn73xx {
		u64 reserved_16_63 : 48;
		u64 tx_tristate_en_ovrrd_val : 1;
		u64 tx_chpd_ovrrd_val : 1;
		u64 reserved_13_10 : 4;
		u64 tx_resetn_ovrrd_val : 1;
		u64 tx_cm_mode : 1;
		u64 cfg_tx_swing : 5;
		u64 fast_rdet_mode : 1;
		u64 fast_tristate_mode : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_tx_cfg_0_cn73xx cn78xx;
	struct cvmx_gserx_lanex_tx_cfg_0_s cn78xxp1;
	struct cvmx_gserx_lanex_tx_cfg_0_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_tx_cfg_0 cvmx_gserx_lanex_tx_cfg_0_t;

/**
 * cvmx_gser#_lane#_tx_cfg_1
 *
 * These registers are reset by hardware only during chip cold reset. The
 * values of the CSR fields in these registers do not change during chip
 * warm or soft resets.
 */
union cvmx_gserx_lanex_tx_cfg_1 {
	u64 u64;
	struct cvmx_gserx_lanex_tx_cfg_1_s {
		u64 reserved_15_63 : 49;
		u64 tx_widthsel_ovrrd_en : 1;
		u64 tx_widthsel_ovrrd_val : 2;
		u64 tx_vboost_en_ovrrd_en : 1;
		u64 tx_turbo_en_ovrrd_en : 1;
		u64 tx_swing_ovrrd_en : 1;
		u64 tx_premptap_ovrrd_val : 1;
		u64 tx_elec_idle_ovrrd_en : 1;
		u64 smpl_rate_ovrrd_en : 1;
		u64 smpl_rate_ovrrd_val : 3;
		u64 tx_datarate_ovrrd_en : 1;
		u64 tx_datarate_ovrrd_val : 2;
	} s;
	struct cvmx_gserx_lanex_tx_cfg_1_s cn73xx;
	struct cvmx_gserx_lanex_tx_cfg_1_s cn78xx;
	struct cvmx_gserx_lanex_tx_cfg_1_s cn78xxp1;
	struct cvmx_gserx_lanex_tx_cfg_1_s cnf75xx;
};

typedef union cvmx_gserx_lanex_tx_cfg_1 cvmx_gserx_lanex_tx_cfg_1_t;

/**
 * cvmx_gser#_lane#_tx_cfg_2
 *
 * These registers are for diagnostic use only. These registers are reset by hardware only during
 * chip cold reset. The values of the CSR fields in these registers do not change during chip
 * warm or soft resets.
 */
union cvmx_gserx_lanex_tx_cfg_2 {
	u64 u64;
	struct cvmx_gserx_lanex_tx_cfg_2_s {
		u64 reserved_16_63 : 48;
		u64 pcs_sds_tx_dcc_en : 1;
		u64 reserved_3_14 : 12;
		u64 rcvr_test_ovrrd_en : 1;
		u64 rcvr_test_ovrrd_val : 1;
		u64 tx_rx_detect_dis_ovrrd_val : 1;
	} s;
	struct cvmx_gserx_lanex_tx_cfg_2_cn73xx {
		u64 reserved_16_63 : 48;
		u64 pcs_sds_tx_dcc_en : 1;
		u64 reserved_14_3 : 12;
		u64 rcvr_test_ovrrd_en : 1;
		u64 rcvr_test_ovrrd_val : 1;
		u64 tx_rx_detect_dis_ovrrd_val : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_tx_cfg_2_cn73xx cn78xx;
	struct cvmx_gserx_lanex_tx_cfg_2_s cn78xxp1;
	struct cvmx_gserx_lanex_tx_cfg_2_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_tx_cfg_2 cvmx_gserx_lanex_tx_cfg_2_t;

/**
 * cvmx_gser#_lane#_tx_cfg_3
 *
 * These registers are for diagnostic use only. These registers are reset by hardware only during
 * chip cold reset. The values of the CSR fields in these registers do not change during chip
 * warm or soft resets.
 */
union cvmx_gserx_lanex_tx_cfg_3 {
	u64 u64;
	struct cvmx_gserx_lanex_tx_cfg_3_s {
		u64 reserved_15_63 : 49;
		u64 cfg_tx_vboost_en : 1;
		u64 reserved_7_13 : 7;
		u64 pcs_sds_tx_gain : 3;
		u64 pcs_sds_tx_srate_sel : 3;
		u64 cfg_tx_turbo_en : 1;
	} s;
	struct cvmx_gserx_lanex_tx_cfg_3_cn73xx {
		u64 reserved_15_63 : 49;
		u64 cfg_tx_vboost_en : 1;
		u64 reserved_13_7 : 7;
		u64 pcs_sds_tx_gain : 3;
		u64 pcs_sds_tx_srate_sel : 3;
		u64 cfg_tx_turbo_en : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_tx_cfg_3_cn73xx cn78xx;
	struct cvmx_gserx_lanex_tx_cfg_3_s cn78xxp1;
	struct cvmx_gserx_lanex_tx_cfg_3_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_tx_cfg_3 cvmx_gserx_lanex_tx_cfg_3_t;

/**
 * cvmx_gser#_lane#_tx_pre_emphasis
 *
 * These registers are reset by hardware only during chip cold reset. The
 * values of the CSR fields in these registers do not change during chip
 * warm or soft resets.
 */
union cvmx_gserx_lanex_tx_pre_emphasis {
	u64 u64;
	struct cvmx_gserx_lanex_tx_pre_emphasis_s {
		u64 reserved_9_63 : 55;
		u64 cfg_tx_premptap : 9;
	} s;
	struct cvmx_gserx_lanex_tx_pre_emphasis_s cn73xx;
	struct cvmx_gserx_lanex_tx_pre_emphasis_s cn78xx;
	struct cvmx_gserx_lanex_tx_pre_emphasis_s cn78xxp1;
	struct cvmx_gserx_lanex_tx_pre_emphasis_s cnf75xx;
};

typedef union cvmx_gserx_lanex_tx_pre_emphasis cvmx_gserx_lanex_tx_pre_emphasis_t;

/**
 * cvmx_gser#_lane#_pcs_ctlifc_0
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_pcs_ctlifc_0 {
	u64 u64;
	struct cvmx_gserx_lanex_pcs_ctlifc_0_s {
		u64 reserved_14_63 : 50;
		u64 cfg_tx_vboost_en_ovrrd_val : 1;
		u64 cfg_tx_coeff_req_ovrrd_val : 1;
		u64 cfg_rx_cdr_coast_req_ovrrd_val : 1;
		u64 cfg_tx_detrx_en_req_ovrrd_val : 1;
		u64 cfg_soft_reset_req_ovrrd_val : 1;
		u64 cfg_lane_pwr_off_ovrrd_val : 1;
		u64 cfg_tx_mode_ovrrd_val : 2;
		u64 cfg_tx_pstate_req_ovrrd_val : 2;
		u64 cfg_lane_mode_req_ovrrd_val : 4;
	} s;
	struct cvmx_gserx_lanex_pcs_ctlifc_0_s cn73xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_0_s cn78xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_0_s cn78xxp1;
	struct cvmx_gserx_lanex_pcs_ctlifc_0_s cnf75xx;
};

typedef union cvmx_gserx_lanex_pcs_ctlifc_0 cvmx_gserx_lanex_pcs_ctlifc_0_t;

/**
 * cvmx_gser#_lane#_pcs_ctlifc_1
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_pcs_ctlifc_1 {
	u64 u64;
	struct cvmx_gserx_lanex_pcs_ctlifc_1_s {
		u64 reserved_9_63 : 55;
		u64 cfg_rx_pstate_req_ovrrd_val : 2;
		u64 reserved_2_6 : 5;
		u64 cfg_rx_mode_ovrrd_val : 2;
	} s;
	struct cvmx_gserx_lanex_pcs_ctlifc_1_cn73xx {
		u64 reserved_9_63 : 55;
		u64 cfg_rx_pstate_req_ovrrd_val : 2;
		u64 reserved_6_2 : 5;
		u64 cfg_rx_mode_ovrrd_val : 2;
	} cn73xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_1_cn73xx cn78xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_1_s cn78xxp1;
	struct cvmx_gserx_lanex_pcs_ctlifc_1_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_pcs_ctlifc_1 cvmx_gserx_lanex_pcs_ctlifc_1_t;

/**
 * cvmx_gser#_lane#_pcs_ctlifc_2
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lanex_pcs_ctlifc_2 {
	u64 u64;
	struct cvmx_gserx_lanex_pcs_ctlifc_2_s {
		u64 reserved_16_63 : 48;
		u64 ctlifc_ovrrd_req : 1;
		u64 reserved_9_14 : 6;
		u64 cfg_tx_vboost_en_ovrrd_en : 1;
		u64 cfg_tx_coeff_req_ovrrd_en : 1;
		u64 cfg_rx_cdr_coast_req_ovrrd_en : 1;
		u64 cfg_tx_detrx_en_req_ovrrd_en : 1;
		u64 cfg_soft_reset_req_ovrrd_en : 1;
		u64 cfg_lane_pwr_off_ovrrd_en : 1;
		u64 cfg_tx_pstate_req_ovrrd_en : 1;
		u64 cfg_rx_pstate_req_ovrrd_en : 1;
		u64 cfg_lane_mode_req_ovrrd_en : 1;
	} s;
	struct cvmx_gserx_lanex_pcs_ctlifc_2_cn73xx {
		u64 reserved_16_63 : 48;
		u64 ctlifc_ovrrd_req : 1;
		u64 reserved_14_9 : 6;
		u64 cfg_tx_vboost_en_ovrrd_en : 1;
		u64 cfg_tx_coeff_req_ovrrd_en : 1;
		u64 cfg_rx_cdr_coast_req_ovrrd_en : 1;
		u64 cfg_tx_detrx_en_req_ovrrd_en : 1;
		u64 cfg_soft_reset_req_ovrrd_en : 1;
		u64 cfg_lane_pwr_off_ovrrd_en : 1;
		u64 cfg_tx_pstate_req_ovrrd_en : 1;
		u64 cfg_rx_pstate_req_ovrrd_en : 1;
		u64 cfg_lane_mode_req_ovrrd_en : 1;
	} cn73xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_2_cn73xx cn78xx;
	struct cvmx_gserx_lanex_pcs_ctlifc_2_s cn78xxp1;
	struct cvmx_gserx_lanex_pcs_ctlifc_2_cn73xx cnf75xx;
};

typedef union cvmx_gserx_lanex_pcs_ctlifc_2 cvmx_gserx_lanex_pcs_ctlifc_2_t;

/**
 * cvmx_gser#_lane_mode
 *
 * These registers are reset by hardware only during chip cold reset. The values of the CSR
 * fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_lane_mode {
	u64 u64;
	struct cvmx_gserx_lane_mode_s {
		u64 reserved_4_63 : 60;
		u64 lmode : 4;
	} s;
	struct cvmx_gserx_lane_mode_s cn73xx;
	struct cvmx_gserx_lane_mode_s cn78xx;
	struct cvmx_gserx_lane_mode_s cn78xxp1;
	struct cvmx_gserx_lane_mode_s cnf75xx;
};

typedef union cvmx_gserx_lane_mode cvmx_gserx_lane_mode_t;

/**
 * cvmx_gser#_pll_p#_mode_0
 *
 * These are the RAW PCS PLL global settings mode 0 registers. There is one register per GSER per
 * GSER_LMODE_E value (0..11). Only one entry is used at any given time in a given GSER - the one
 * selected by the corresponding GSER()_LANE_MODE[LMODE].
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during subsequent chip warm or
 * soft resets.
 */
union cvmx_gserx_pll_px_mode_0 {
	u64 u64;
	struct cvmx_gserx_pll_px_mode_0_s {
		u64 reserved_16_63 : 48;
		u64 pll_icp : 4;
		u64 pll_rloop : 3;
		u64 pll_pcs_div : 9;
	} s;
	struct cvmx_gserx_pll_px_mode_0_s cn73xx;
	struct cvmx_gserx_pll_px_mode_0_s cn78xx;
	struct cvmx_gserx_pll_px_mode_0_s cn78xxp1;
	struct cvmx_gserx_pll_px_mode_0_s cnf75xx;
};

typedef union cvmx_gserx_pll_px_mode_0 cvmx_gserx_pll_px_mode_0_t;

/**
 * cvmx_gser#_pll_p#_mode_1
 *
 * These are the RAW PCS PLL global settings mode 1 registers. There is one register per GSER per
 * GSER_LMODE_E value (0..11). Only one entry is used at any given time in a given GSER - the one
 * selected by the corresponding GSER()_LANE_MODE[LMODE].
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in this register do not change during subsequent chip warm or
 * soft resets.
 */
union cvmx_gserx_pll_px_mode_1 {
	u64 u64;
	struct cvmx_gserx_pll_px_mode_1_s {
		u64 reserved_14_63 : 50;
		u64 pll_16p5en : 1;
		u64 pll_cpadj : 2;
		u64 pll_pcie3en : 1;
		u64 pll_opr : 1;
		u64 pll_div : 9;
	} s;
	struct cvmx_gserx_pll_px_mode_1_s cn73xx;
	struct cvmx_gserx_pll_px_mode_1_s cn78xx;
	struct cvmx_gserx_pll_px_mode_1_s cn78xxp1;
	struct cvmx_gserx_pll_px_mode_1_s cnf75xx;
};

typedef union cvmx_gserx_pll_px_mode_1 cvmx_gserx_pll_px_mode_1_t;

/**
 * cvmx_gser#_pll_stat
 */
union cvmx_gserx_pll_stat {
	u64 u64;
	struct cvmx_gserx_pll_stat_s {
		u64 reserved_1_63 : 63;
		u64 pll_lock : 1;
	} s;
	struct cvmx_gserx_pll_stat_s cn73xx;
	struct cvmx_gserx_pll_stat_s cn78xx;
	struct cvmx_gserx_pll_stat_s cn78xxp1;
	struct cvmx_gserx_pll_stat_s cnf75xx;
};

typedef union cvmx_gserx_pll_stat cvmx_gserx_pll_stat_t;

/**
 * cvmx_gser#_qlm_stat
 */
union cvmx_gserx_qlm_stat {
	u64 u64;
	struct cvmx_gserx_qlm_stat_s {
		u64 reserved_2_63 : 62;
		u64 rst_rdy : 1;
		u64 dcok : 1;
	} s;
	struct cvmx_gserx_qlm_stat_s cn73xx;
	struct cvmx_gserx_qlm_stat_s cn78xx;
	struct cvmx_gserx_qlm_stat_s cn78xxp1;
	struct cvmx_gserx_qlm_stat_s cnf75xx;
};

typedef union cvmx_gserx_qlm_stat cvmx_gserx_qlm_stat_t;

/**
 * cvmx_gser#_slice_cfg
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 */
union cvmx_gserx_slice_cfg {
	u64 u64;
	struct cvmx_gserx_slice_cfg_s {
		u64 reserved_12_63 : 52;
		u64 tx_rx_detect_lvl_enc : 4;
		u64 reserved_6_7 : 2;
		u64 pcs_sds_rx_pcie_pterm : 2;
		u64 pcs_sds_rx_pcie_nterm : 2;
		u64 pcs_sds_tx_stress_eye : 2;
	} s;
	struct cvmx_gserx_slice_cfg_cn73xx {
		u64 reserved_12_63 : 52;
		u64 tx_rx_detect_lvl_enc : 4;
		u64 reserved_7_6 : 2;
		u64 pcs_sds_rx_pcie_pterm : 2;
		u64 pcs_sds_rx_pcie_nterm : 2;
		u64 pcs_sds_tx_stress_eye : 2;
	} cn73xx;
	struct cvmx_gserx_slice_cfg_cn73xx cn78xx;
	struct cvmx_gserx_slice_cfg_s cn78xxp1;
	struct cvmx_gserx_slice_cfg_cn73xx cnf75xx;
};

typedef union cvmx_gserx_slice_cfg cvmx_gserx_slice_cfg_t;

/**
 * cvmx_gser#_slice#_pcie1_mode
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 *
 * Slice 1 does not exist on GSER0, GSER1, GSER4, GSER5, GSER6, GSER7, and GSER8.
 */
union cvmx_gserx_slicex_pcie1_mode {
	u64 u64;
	struct cvmx_gserx_slicex_pcie1_mode_s {
		u64 reserved_15_63 : 49;
		u64 slice_spare_1_0 : 2;
		u64 rx_ldll_isel : 2;
		u64 rx_sdll_isel : 2;
		u64 rx_pi_bwsel : 3;
		u64 rx_ldll_bwsel : 3;
		u64 rx_sdll_bwsel : 3;
	} s;
	struct cvmx_gserx_slicex_pcie1_mode_s cn73xx;
	struct cvmx_gserx_slicex_pcie1_mode_s cn78xx;
	struct cvmx_gserx_slicex_pcie1_mode_s cn78xxp1;
	struct cvmx_gserx_slicex_pcie1_mode_s cnf75xx;
};

typedef union cvmx_gserx_slicex_pcie1_mode cvmx_gserx_slicex_pcie1_mode_t;

/**
 * cvmx_gser#_slice#_pcie2_mode
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 *
 * Slice 1 does not exist on GSER0, GSER1, GSER4, GSER5, GSER6, GSER7, and GSER8.
 */
union cvmx_gserx_slicex_pcie2_mode {
	u64 u64;
	struct cvmx_gserx_slicex_pcie2_mode_s {
		u64 reserved_15_63 : 49;
		u64 slice_spare_1_0 : 2;
		u64 rx_ldll_isel : 2;
		u64 rx_sdll_isel : 2;
		u64 rx_pi_bwsel : 3;
		u64 rx_ldll_bwsel : 3;
		u64 rx_sdll_bwsel : 3;
	} s;
	struct cvmx_gserx_slicex_pcie2_mode_s cn73xx;
	struct cvmx_gserx_slicex_pcie2_mode_s cn78xx;
	struct cvmx_gserx_slicex_pcie2_mode_s cn78xxp1;
	struct cvmx_gserx_slicex_pcie2_mode_s cnf75xx;
};

typedef union cvmx_gserx_slicex_pcie2_mode cvmx_gserx_slicex_pcie2_mode_t;

/**
 * cvmx_gser#_slice#_pcie3_mode
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 *
 * Slice 1 does not exist on GSER0, GSER1, GSER4, GSER5, GSER6, GSER7, and GSER8.
 */
union cvmx_gserx_slicex_pcie3_mode {
	u64 u64;
	struct cvmx_gserx_slicex_pcie3_mode_s {
		u64 reserved_15_63 : 49;
		u64 slice_spare_1_0 : 2;
		u64 rx_ldll_isel : 2;
		u64 rx_sdll_isel : 2;
		u64 rx_pi_bwsel : 3;
		u64 rx_ldll_bwsel : 3;
		u64 rx_sdll_bwsel : 3;
	} s;
	struct cvmx_gserx_slicex_pcie3_mode_s cn73xx;
	struct cvmx_gserx_slicex_pcie3_mode_s cn78xx;
	struct cvmx_gserx_slicex_pcie3_mode_s cn78xxp1;
	struct cvmx_gserx_slicex_pcie3_mode_s cnf75xx;
};

typedef union cvmx_gserx_slicex_pcie3_mode cvmx_gserx_slicex_pcie3_mode_t;

/**
 * cvmx_gser#_slice#_rx_sdll_ctrl
 *
 * These registers are for diagnostic use only.
 * These registers are reset by hardware only during chip cold reset.
 * The values of the CSR fields in these registers do not change during chip warm or soft resets.
 *
 * Slice 1 does not exist on GSER0, GSER1, GSER4, GSER5, GSER6, GSER7, and GSER8.
 */
union cvmx_gserx_slicex_rx_sdll_ctrl {
	u64 u64;
	struct cvmx_gserx_slicex_rx_sdll_ctrl_s {
		u64 reserved_16_63 : 48;
		u64 pcs_sds_oob_clk_ctrl : 2;
		u64 reserved_7_13 : 7;
		u64 pcs_sds_rx_sdll_tune : 3;
		u64 pcs_sds_rx_sdll_swsel : 4;
	} s;
	struct cvmx_gserx_slicex_rx_sdll_ctrl_cn73xx {
		u64 reserved_16_63 : 48;
		u64 pcs_sds_oob_clk_ctrl : 2;
		u64 reserved_13_7 : 7;
		u64 pcs_sds_rx_sdll_tune : 3;
		u64 pcs_sds_rx_sdll_swsel : 4;
	} cn73xx;
	struct cvmx_gserx_slicex_rx_sdll_ctrl_cn73xx cn78xx;
	struct cvmx_gserx_slicex_rx_sdll_ctrl_s cn78xxp1;
	struct cvmx_gserx_slicex_rx_sdll_ctrl_cn73xx cnf75xx;
};

typedef union cvmx_gserx_slicex_rx_sdll_ctrl cvmx_gserx_slicex_rx_sdll_ctrl_t;

#endif
