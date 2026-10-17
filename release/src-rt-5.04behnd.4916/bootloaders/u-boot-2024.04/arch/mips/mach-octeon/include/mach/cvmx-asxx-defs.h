/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon asxx.
 */

#ifndef __CVMX_ASXX_DEFS_H__
#define __CVMX_ASXX_DEFS_H__

#define CVMX_ASXX_GMII_RX_CLK_SET(offset)    (0x00011800B0000180ull)
#define CVMX_ASXX_GMII_RX_DAT_SET(offset)    (0x00011800B0000188ull)
#define CVMX_ASXX_INT_EN(offset)	     (0x00011800B0000018ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_INT_REG(offset)	     (0x00011800B0000010ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_MII_RX_DAT_SET(offset)     (0x00011800B0000190ull)
#define CVMX_ASXX_PRT_LOOP(offset)	     (0x00011800B0000040ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_BYPASS(offset)	     (0x00011800B0000248ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_BYPASS_SETTING(offset) (0x00011800B0000250ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_COMP(offset)	     (0x00011800B0000220ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_DATA_DRV(offset)	     (0x00011800B0000218ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_FCRAM_MODE(offset)     (0x00011800B0000210ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_NCTL_STRONG(offset)    (0x00011800B0000230ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_NCTL_WEAK(offset)	     (0x00011800B0000240ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_PCTL_STRONG(offset)    (0x00011800B0000228ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_PCTL_WEAK(offset)	     (0x00011800B0000238ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RLD_SETTING(offset)	     (0x00011800B0000258ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RX_CLK_SETX(offset, block_id)                                                    \
	(0x00011800B0000020ull + (((offset) & 3) + ((block_id) & 1) * 0x1000000ull) * 8)
#define CVMX_ASXX_RX_PRT_EN(offset)    (0x00011800B0000000ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RX_WOL(offset)       (0x00011800B0000100ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RX_WOL_MSK(offset)   (0x00011800B0000108ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RX_WOL_POWOK(offset) (0x00011800B0000118ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_RX_WOL_SIG(offset)   (0x00011800B0000110ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_TX_CLK_SETX(offset, block_id)                                                    \
	(0x00011800B0000048ull + (((offset) & 3) + ((block_id) & 1) * 0x1000000ull) * 8)
#define CVMX_ASXX_TX_COMP_BYP(offset) (0x00011800B0000068ull + ((offset) & 1) * 0x8000000ull)
#define CVMX_ASXX_TX_HI_WATERX(offset, block_id)                                                   \
	(0x00011800B0000080ull + (((offset) & 3) + ((block_id) & 1) * 0x1000000ull) * 8)
#define CVMX_ASXX_TX_PRT_EN(offset) (0x00011800B0000008ull + ((offset) & 1) * 0x8000000ull)

/**
 * cvmx_asx#_gmii_rx_clk_set
 *
 * ASX_GMII_RX_CLK_SET = GMII Clock delay setting
 *
 */
union cvmx_asxx_gmii_rx_clk_set {
	u64 u64;
	struct cvmx_asxx_gmii_rx_clk_set_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_gmii_rx_clk_set_s cn30xx;
	struct cvmx_asxx_gmii_rx_clk_set_s cn31xx;
	struct cvmx_asxx_gmii_rx_clk_set_s cn50xx;
};

typedef union cvmx_asxx_gmii_rx_clk_set cvmx_asxx_gmii_rx_clk_set_t;

/**
 * cvmx_asx#_gmii_rx_dat_set
 *
 * ASX_GMII_RX_DAT_SET = GMII Clock delay setting
 *
 */
union cvmx_asxx_gmii_rx_dat_set {
	u64 u64;
	struct cvmx_asxx_gmii_rx_dat_set_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_gmii_rx_dat_set_s cn30xx;
	struct cvmx_asxx_gmii_rx_dat_set_s cn31xx;
	struct cvmx_asxx_gmii_rx_dat_set_s cn50xx;
};

typedef union cvmx_asxx_gmii_rx_dat_set cvmx_asxx_gmii_rx_dat_set_t;

/**
 * cvmx_asx#_int_en
 *
 * ASX_INT_EN = Interrupt Enable
 *
 */
union cvmx_asxx_int_en {
	u64 u64;
	struct cvmx_asxx_int_en_s {
		u64 reserved_12_63 : 52;
		u64 txpsh : 4;
		u64 txpop : 4;
		u64 ovrflw : 4;
	} s;
	struct cvmx_asxx_int_en_cn30xx {
		u64 reserved_11_63 : 53;
		u64 txpsh : 3;
		u64 reserved_7_7 : 1;
		u64 txpop : 3;
		u64 reserved_3_3 : 1;
		u64 ovrflw : 3;
	} cn30xx;
	struct cvmx_asxx_int_en_cn30xx cn31xx;
	struct cvmx_asxx_int_en_s cn38xx;
	struct cvmx_asxx_int_en_s cn38xxp2;
	struct cvmx_asxx_int_en_cn30xx cn50xx;
	struct cvmx_asxx_int_en_s cn58xx;
	struct cvmx_asxx_int_en_s cn58xxp1;
};

typedef union cvmx_asxx_int_en cvmx_asxx_int_en_t;

/**
 * cvmx_asx#_int_reg
 *
 * ASX_INT_REG = Interrupt Register
 *
 */
union cvmx_asxx_int_reg {
	u64 u64;
	struct cvmx_asxx_int_reg_s {
		u64 reserved_12_63 : 52;
		u64 txpsh : 4;
		u64 txpop : 4;
		u64 ovrflw : 4;
	} s;
	struct cvmx_asxx_int_reg_cn30xx {
		u64 reserved_11_63 : 53;
		u64 txpsh : 3;
		u64 reserved_7_7 : 1;
		u64 txpop : 3;
		u64 reserved_3_3 : 1;
		u64 ovrflw : 3;
	} cn30xx;
	struct cvmx_asxx_int_reg_cn30xx cn31xx;
	struct cvmx_asxx_int_reg_s cn38xx;
	struct cvmx_asxx_int_reg_s cn38xxp2;
	struct cvmx_asxx_int_reg_cn30xx cn50xx;
	struct cvmx_asxx_int_reg_s cn58xx;
	struct cvmx_asxx_int_reg_s cn58xxp1;
};

typedef union cvmx_asxx_int_reg cvmx_asxx_int_reg_t;

/**
 * cvmx_asx#_mii_rx_dat_set
 *
 * ASX_MII_RX_DAT_SET = GMII Clock delay setting
 *
 */
union cvmx_asxx_mii_rx_dat_set {
	u64 u64;
	struct cvmx_asxx_mii_rx_dat_set_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_mii_rx_dat_set_s cn30xx;
	struct cvmx_asxx_mii_rx_dat_set_s cn50xx;
};

typedef union cvmx_asxx_mii_rx_dat_set cvmx_asxx_mii_rx_dat_set_t;

/**
 * cvmx_asx#_prt_loop
 *
 * ASX_PRT_LOOP = Internal Loopback mode - TX FIFO output goes into RX FIFO (and maybe pins)
 *
 */
union cvmx_asxx_prt_loop {
	u64 u64;
	struct cvmx_asxx_prt_loop_s {
		u64 reserved_8_63 : 56;
		u64 ext_loop : 4;
		u64 int_loop : 4;
	} s;
	struct cvmx_asxx_prt_loop_cn30xx {
		u64 reserved_7_63 : 57;
		u64 ext_loop : 3;
		u64 reserved_3_3 : 1;
		u64 int_loop : 3;
	} cn30xx;
	struct cvmx_asxx_prt_loop_cn30xx cn31xx;
	struct cvmx_asxx_prt_loop_s cn38xx;
	struct cvmx_asxx_prt_loop_s cn38xxp2;
	struct cvmx_asxx_prt_loop_cn30xx cn50xx;
	struct cvmx_asxx_prt_loop_s cn58xx;
	struct cvmx_asxx_prt_loop_s cn58xxp1;
};

typedef union cvmx_asxx_prt_loop cvmx_asxx_prt_loop_t;

/**
 * cvmx_asx#_rld_bypass
 *
 * ASX_RLD_BYPASS
 *
 */
union cvmx_asxx_rld_bypass {
	u64 u64;
	struct cvmx_asxx_rld_bypass_s {
		u64 reserved_1_63 : 63;
		u64 bypass : 1;
	} s;
	struct cvmx_asxx_rld_bypass_s cn38xx;
	struct cvmx_asxx_rld_bypass_s cn38xxp2;
	struct cvmx_asxx_rld_bypass_s cn58xx;
	struct cvmx_asxx_rld_bypass_s cn58xxp1;
};

typedef union cvmx_asxx_rld_bypass cvmx_asxx_rld_bypass_t;

/**
 * cvmx_asx#_rld_bypass_setting
 *
 * ASX_RLD_BYPASS_SETTING
 *
 */
union cvmx_asxx_rld_bypass_setting {
	u64 u64;
	struct cvmx_asxx_rld_bypass_setting_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_rld_bypass_setting_s cn38xx;
	struct cvmx_asxx_rld_bypass_setting_s cn38xxp2;
	struct cvmx_asxx_rld_bypass_setting_s cn58xx;
	struct cvmx_asxx_rld_bypass_setting_s cn58xxp1;
};

typedef union cvmx_asxx_rld_bypass_setting cvmx_asxx_rld_bypass_setting_t;

/**
 * cvmx_asx#_rld_comp
 *
 * ASX_RLD_COMP
 *
 */
union cvmx_asxx_rld_comp {
	u64 u64;
	struct cvmx_asxx_rld_comp_s {
		u64 reserved_9_63 : 55;
		u64 pctl : 5;
		u64 nctl : 4;
	} s;
	struct cvmx_asxx_rld_comp_cn38xx {
		u64 reserved_8_63 : 56;
		u64 pctl : 4;
		u64 nctl : 4;
	} cn38xx;
	struct cvmx_asxx_rld_comp_cn38xx cn38xxp2;
	struct cvmx_asxx_rld_comp_s cn58xx;
	struct cvmx_asxx_rld_comp_s cn58xxp1;
};

typedef union cvmx_asxx_rld_comp cvmx_asxx_rld_comp_t;

/**
 * cvmx_asx#_rld_data_drv
 *
 * ASX_RLD_DATA_DRV
 *
 */
union cvmx_asxx_rld_data_drv {
	u64 u64;
	struct cvmx_asxx_rld_data_drv_s {
		u64 reserved_8_63 : 56;
		u64 pctl : 4;
		u64 nctl : 4;
	} s;
	struct cvmx_asxx_rld_data_drv_s cn38xx;
	struct cvmx_asxx_rld_data_drv_s cn38xxp2;
	struct cvmx_asxx_rld_data_drv_s cn58xx;
	struct cvmx_asxx_rld_data_drv_s cn58xxp1;
};

typedef union cvmx_asxx_rld_data_drv cvmx_asxx_rld_data_drv_t;

/**
 * cvmx_asx#_rld_fcram_mode
 *
 * ASX_RLD_FCRAM_MODE
 *
 */
union cvmx_asxx_rld_fcram_mode {
	u64 u64;
	struct cvmx_asxx_rld_fcram_mode_s {
		u64 reserved_1_63 : 63;
		u64 mode : 1;
	} s;
	struct cvmx_asxx_rld_fcram_mode_s cn38xx;
	struct cvmx_asxx_rld_fcram_mode_s cn38xxp2;
};

typedef union cvmx_asxx_rld_fcram_mode cvmx_asxx_rld_fcram_mode_t;

/**
 * cvmx_asx#_rld_nctl_strong
 *
 * ASX_RLD_NCTL_STRONG
 *
 */
union cvmx_asxx_rld_nctl_strong {
	u64 u64;
	struct cvmx_asxx_rld_nctl_strong_s {
		u64 reserved_5_63 : 59;
		u64 nctl : 5;
	} s;
	struct cvmx_asxx_rld_nctl_strong_s cn38xx;
	struct cvmx_asxx_rld_nctl_strong_s cn38xxp2;
	struct cvmx_asxx_rld_nctl_strong_s cn58xx;
	struct cvmx_asxx_rld_nctl_strong_s cn58xxp1;
};

typedef union cvmx_asxx_rld_nctl_strong cvmx_asxx_rld_nctl_strong_t;

/**
 * cvmx_asx#_rld_nctl_weak
 *
 * ASX_RLD_NCTL_WEAK
 *
 */
union cvmx_asxx_rld_nctl_weak {
	u64 u64;
	struct cvmx_asxx_rld_nctl_weak_s {
		u64 reserved_5_63 : 59;
		u64 nctl : 5;
	} s;
	struct cvmx_asxx_rld_nctl_weak_s cn38xx;
	struct cvmx_asxx_rld_nctl_weak_s cn38xxp2;
	struct cvmx_asxx_rld_nctl_weak_s cn58xx;
	struct cvmx_asxx_rld_nctl_weak_s cn58xxp1;
};

typedef union cvmx_asxx_rld_nctl_weak cvmx_asxx_rld_nctl_weak_t;

/**
 * cvmx_asx#_rld_pctl_strong
 *
 * ASX_RLD_PCTL_STRONG
 *
 */
union cvmx_asxx_rld_pctl_strong {
	u64 u64;
	struct cvmx_asxx_rld_pctl_strong_s {
		u64 reserved_5_63 : 59;
		u64 pctl : 5;
	} s;
	struct cvmx_asxx_rld_pctl_strong_s cn38xx;
	struct cvmx_asxx_rld_pctl_strong_s cn38xxp2;
	struct cvmx_asxx_rld_pctl_strong_s cn58xx;
	struct cvmx_asxx_rld_pctl_strong_s cn58xxp1;
};

typedef union cvmx_asxx_rld_pctl_strong cvmx_asxx_rld_pctl_strong_t;

/**
 * cvmx_asx#_rld_pctl_weak
 *
 * ASX_RLD_PCTL_WEAK
 *
 */
union cvmx_asxx_rld_pctl_weak {
	u64 u64;
	struct cvmx_asxx_rld_pctl_weak_s {
		u64 reserved_5_63 : 59;
		u64 pctl : 5;
	} s;
	struct cvmx_asxx_rld_pctl_weak_s cn38xx;
	struct cvmx_asxx_rld_pctl_weak_s cn38xxp2;
	struct cvmx_asxx_rld_pctl_weak_s cn58xx;
	struct cvmx_asxx_rld_pctl_weak_s cn58xxp1;
};

typedef union cvmx_asxx_rld_pctl_weak cvmx_asxx_rld_pctl_weak_t;

/**
 * cvmx_asx#_rld_setting
 *
 * ASX_RLD_SETTING
 *
 */
union cvmx_asxx_rld_setting {
	u64 u64;
	struct cvmx_asxx_rld_setting_s {
		u64 reserved_13_63 : 51;
		u64 dfaset : 5;
		u64 dfalag : 1;
		u64 dfalead : 1;
		u64 dfalock : 1;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_rld_setting_cn38xx {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} cn38xx;
	struct cvmx_asxx_rld_setting_cn38xx cn38xxp2;
	struct cvmx_asxx_rld_setting_s cn58xx;
	struct cvmx_asxx_rld_setting_s cn58xxp1;
};

typedef union cvmx_asxx_rld_setting cvmx_asxx_rld_setting_t;

/**
 * cvmx_asx#_rx_clk_set#
 *
 * ASX_RX_CLK_SET = RGMII Clock delay setting
 *
 *
 * Notes:
 * Setting to place on the open-loop RXC (RGMII receive clk)
 * delay line, which can delay the received clock. This
 * can be used if the board and/or transmitting device
 * has not otherwise delayed the clock.
 *
 * A value of SETTING=0 disables the delay line. The delay
 * line should be disabled unless the transmitter or board
 * does not delay the clock.
 *
 * Note that this delay line provides only a coarse control
 * over the delay. Generally, it can only reliably provide
 * a delay in the range 1.25-2.5ns, which may not be adequate
 * for some system applications.
 *
 * The open loop delay line selects
 * from among a series of tap positions. Each incremental
 * tap position adds a delay of 50ps to 135ps per tap, depending
 * on the chip, its temperature, and the voltage.
 * To achieve from 1.25-2.5ns of delay on the received
 * clock, a fixed value of SETTING=24 may work.
 * For more precision, we recommend the following settings
 * based on the chip voltage:
 *
 *    VDD           SETTING
 *  -----------------------------
 *    1.0             18
 *    1.05            19
 *    1.1             21
 *    1.15            22
 *    1.2             23
 *    1.25            24
 *    1.3             25
 */
union cvmx_asxx_rx_clk_setx {
	u64 u64;
	struct cvmx_asxx_rx_clk_setx_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_rx_clk_setx_s cn30xx;
	struct cvmx_asxx_rx_clk_setx_s cn31xx;
	struct cvmx_asxx_rx_clk_setx_s cn38xx;
	struct cvmx_asxx_rx_clk_setx_s cn38xxp2;
	struct cvmx_asxx_rx_clk_setx_s cn50xx;
	struct cvmx_asxx_rx_clk_setx_s cn58xx;
	struct cvmx_asxx_rx_clk_setx_s cn58xxp1;
};

typedef union cvmx_asxx_rx_clk_setx cvmx_asxx_rx_clk_setx_t;

/**
 * cvmx_asx#_rx_prt_en
 *
 * ASX_RX_PRT_EN = RGMII Port Enable
 *
 */
union cvmx_asxx_rx_prt_en {
	u64 u64;
	struct cvmx_asxx_rx_prt_en_s {
		u64 reserved_4_63 : 60;
		u64 prt_en : 4;
	} s;
	struct cvmx_asxx_rx_prt_en_cn30xx {
		u64 reserved_3_63 : 61;
		u64 prt_en : 3;
	} cn30xx;
	struct cvmx_asxx_rx_prt_en_cn30xx cn31xx;
	struct cvmx_asxx_rx_prt_en_s cn38xx;
	struct cvmx_asxx_rx_prt_en_s cn38xxp2;
	struct cvmx_asxx_rx_prt_en_cn30xx cn50xx;
	struct cvmx_asxx_rx_prt_en_s cn58xx;
	struct cvmx_asxx_rx_prt_en_s cn58xxp1;
};

typedef union cvmx_asxx_rx_prt_en cvmx_asxx_rx_prt_en_t;

/**
 * cvmx_asx#_rx_wol
 *
 * ASX_RX_WOL = RGMII RX Wake on LAN status register
 *
 */
union cvmx_asxx_rx_wol {
	u64 u64;
	struct cvmx_asxx_rx_wol_s {
		u64 reserved_2_63 : 62;
		u64 status : 1;
		u64 enable : 1;
	} s;
	struct cvmx_asxx_rx_wol_s cn38xx;
	struct cvmx_asxx_rx_wol_s cn38xxp2;
};

typedef union cvmx_asxx_rx_wol cvmx_asxx_rx_wol_t;

/**
 * cvmx_asx#_rx_wol_msk
 *
 * ASX_RX_WOL_MSK = RGMII RX Wake on LAN byte mask
 *
 */
union cvmx_asxx_rx_wol_msk {
	u64 u64;
	struct cvmx_asxx_rx_wol_msk_s {
		u64 msk : 64;
	} s;
	struct cvmx_asxx_rx_wol_msk_s cn38xx;
	struct cvmx_asxx_rx_wol_msk_s cn38xxp2;
};

typedef union cvmx_asxx_rx_wol_msk cvmx_asxx_rx_wol_msk_t;

/**
 * cvmx_asx#_rx_wol_powok
 *
 * ASX_RX_WOL_POWOK = RGMII RX Wake on LAN Power OK
 *
 */
union cvmx_asxx_rx_wol_powok {
	u64 u64;
	struct cvmx_asxx_rx_wol_powok_s {
		u64 reserved_1_63 : 63;
		u64 powerok : 1;
	} s;
	struct cvmx_asxx_rx_wol_powok_s cn38xx;
	struct cvmx_asxx_rx_wol_powok_s cn38xxp2;
};

typedef union cvmx_asxx_rx_wol_powok cvmx_asxx_rx_wol_powok_t;

/**
 * cvmx_asx#_rx_wol_sig
 *
 * ASX_RX_WOL_SIG = RGMII RX Wake on LAN CRC signature
 *
 */
union cvmx_asxx_rx_wol_sig {
	u64 u64;
	struct cvmx_asxx_rx_wol_sig_s {
		u64 reserved_32_63 : 32;
		u64 sig : 32;
	} s;
	struct cvmx_asxx_rx_wol_sig_s cn38xx;
	struct cvmx_asxx_rx_wol_sig_s cn38xxp2;
};

typedef union cvmx_asxx_rx_wol_sig cvmx_asxx_rx_wol_sig_t;

/**
 * cvmx_asx#_tx_clk_set#
 *
 * ASX_TX_CLK_SET = RGMII Clock delay setting
 *
 *
 * Notes:
 * Setting to place on the open-loop TXC (RGMII transmit clk)
 * delay line, which can delay the transmited clock. This
 * can be used if the board and/or transmitting device
 * has not otherwise delayed the clock.
 *
 * A value of SETTING=0 disables the delay line. The delay
 * line should be disabled unless the transmitter or board
 * does not delay the clock.
 *
 * Note that this delay line provides only a coarse control
 * over the delay. Generally, it can only reliably provide
 * a delay in the range 1.25-2.5ns, which may not be adequate
 * for some system applications.
 *
 * The open loop delay line selects
 * from among a series of tap positions. Each incremental
 * tap position adds a delay of 50ps to 135ps per tap, depending
 * on the chip, its temperature, and the voltage.
 * To achieve from 1.25-2.5ns of delay on the received
 * clock, a fixed value of SETTING=24 may work.
 * For more precision, we recommend the following settings
 * based on the chip voltage:
 *
 *    VDD           SETTING
 *  -----------------------------
 *    1.0             18
 *    1.05            19
 *    1.1             21
 *    1.15            22
 *    1.2             23
 *    1.25            24
 *    1.3             25
 */
union cvmx_asxx_tx_clk_setx {
	u64 u64;
	struct cvmx_asxx_tx_clk_setx_s {
		u64 reserved_5_63 : 59;
		u64 setting : 5;
	} s;
	struct cvmx_asxx_tx_clk_setx_s cn30xx;
	struct cvmx_asxx_tx_clk_setx_s cn31xx;
	struct cvmx_asxx_tx_clk_setx_s cn38xx;
	struct cvmx_asxx_tx_clk_setx_s cn38xxp2;
	struct cvmx_asxx_tx_clk_setx_s cn50xx;
	struct cvmx_asxx_tx_clk_setx_s cn58xx;
	struct cvmx_asxx_tx_clk_setx_s cn58xxp1;
};

typedef union cvmx_asxx_tx_clk_setx cvmx_asxx_tx_clk_setx_t;

/**
 * cvmx_asx#_tx_comp_byp
 *
 * ASX_TX_COMP_BYP = RGMII Clock delay setting
 *
 */
union cvmx_asxx_tx_comp_byp {
	u64 u64;
	struct cvmx_asxx_tx_comp_byp_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_asxx_tx_comp_byp_cn30xx {
		u64 reserved_9_63 : 55;
		u64 bypass : 1;
		u64 pctl : 4;
		u64 nctl : 4;
	} cn30xx;
	struct cvmx_asxx_tx_comp_byp_cn30xx cn31xx;
	struct cvmx_asxx_tx_comp_byp_cn38xx {
		u64 reserved_8_63 : 56;
		u64 pctl : 4;
		u64 nctl : 4;
	} cn38xx;
	struct cvmx_asxx_tx_comp_byp_cn38xx cn38xxp2;
	struct cvmx_asxx_tx_comp_byp_cn50xx {
		u64 reserved_17_63 : 47;
		u64 bypass : 1;
		u64 reserved_13_15 : 3;
		u64 pctl : 5;
		u64 reserved_5_7 : 3;
		u64 nctl : 5;
	} cn50xx;
	struct cvmx_asxx_tx_comp_byp_cn58xx {
		u64 reserved_13_63 : 51;
		u64 pctl : 5;
		u64 reserved_5_7 : 3;
		u64 nctl : 5;
	} cn58xx;
	struct cvmx_asxx_tx_comp_byp_cn58xx cn58xxp1;
};

typedef union cvmx_asxx_tx_comp_byp cvmx_asxx_tx_comp_byp_t;

/**
 * cvmx_asx#_tx_hi_water#
 *
 * ASX_TX_HI_WATER = RGMII TX FIFO Hi WaterMark
 *
 */
union cvmx_asxx_tx_hi_waterx {
	u64 u64;
	struct cvmx_asxx_tx_hi_waterx_s {
		u64 reserved_4_63 : 60;
		u64 mark : 4;
	} s;
	struct cvmx_asxx_tx_hi_waterx_cn30xx {
		u64 reserved_3_63 : 61;
		u64 mark : 3;
	} cn30xx;
	struct cvmx_asxx_tx_hi_waterx_cn30xx cn31xx;
	struct cvmx_asxx_tx_hi_waterx_s cn38xx;
	struct cvmx_asxx_tx_hi_waterx_s cn38xxp2;
	struct cvmx_asxx_tx_hi_waterx_cn30xx cn50xx;
	struct cvmx_asxx_tx_hi_waterx_s cn58xx;
	struct cvmx_asxx_tx_hi_waterx_s cn58xxp1;
};

typedef union cvmx_asxx_tx_hi_waterx cvmx_asxx_tx_hi_waterx_t;

/**
 * cvmx_asx#_tx_prt_en
 *
 * ASX_TX_PRT_EN = RGMII Port Enable
 *
 */
union cvmx_asxx_tx_prt_en {
	u64 u64;
	struct cvmx_asxx_tx_prt_en_s {
		u64 reserved_4_63 : 60;
		u64 prt_en : 4;
	} s;
	struct cvmx_asxx_tx_prt_en_cn30xx {
		u64 reserved_3_63 : 61;
		u64 prt_en : 3;
	} cn30xx;
	struct cvmx_asxx_tx_prt_en_cn30xx cn31xx;
	struct cvmx_asxx_tx_prt_en_s cn38xx;
	struct cvmx_asxx_tx_prt_en_s cn38xxp2;
	struct cvmx_asxx_tx_prt_en_cn30xx cn50xx;
	struct cvmx_asxx_tx_prt_en_s cn58xx;
	struct cvmx_asxx_tx_prt_en_s cn58xxp1;
};

typedef union cvmx_asxx_tx_prt_en cvmx_asxx_tx_prt_en_t;

#endif
