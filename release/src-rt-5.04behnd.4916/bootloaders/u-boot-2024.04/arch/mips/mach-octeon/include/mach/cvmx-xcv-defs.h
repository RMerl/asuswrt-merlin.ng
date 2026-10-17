/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon xcv.
 */

#ifndef __CVMX_XCV_DEFS_H__
#define __CVMX_XCV_DEFS_H__

#define CVMX_XCV_BATCH_CRD_RET (0x00011800DB000100ull)
#define CVMX_XCV_COMP_CTL      (0x00011800DB000020ull)
#define CVMX_XCV_CTL	       (0x00011800DB000030ull)
#define CVMX_XCV_DLL_CTL       (0x00011800DB000010ull)
#define CVMX_XCV_ECO	       (0x00011800DB000200ull)
#define CVMX_XCV_INBND_STATUS  (0x00011800DB000080ull)
#define CVMX_XCV_INT	       (0x00011800DB000040ull)
#define CVMX_XCV_RESET	       (0x00011800DB000000ull)

/**
 * cvmx_xcv_batch_crd_ret
 */
union cvmx_xcv_batch_crd_ret {
	u64 u64;
	struct cvmx_xcv_batch_crd_ret_s {
		u64 reserved_1_63 : 63;
		u64 crd_ret : 1;
	} s;
	struct cvmx_xcv_batch_crd_ret_s cn73xx;
};

typedef union cvmx_xcv_batch_crd_ret cvmx_xcv_batch_crd_ret_t;

/**
 * cvmx_xcv_comp_ctl
 *
 * This register controls programmable compensation.
 *
 */
union cvmx_xcv_comp_ctl {
	u64 u64;
	struct cvmx_xcv_comp_ctl_s {
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
		u64 pctl_lock : 1;
		u64 pctl_sat : 1;
		u64 reserved_28_28 : 1;
		u64 nctl_lock : 1;
		u64 reserved_1_26 : 26;
		u64 nctl_sat : 1;
	} s;
	struct cvmx_xcv_comp_ctl_s cn73xx;
};

typedef union cvmx_xcv_comp_ctl cvmx_xcv_comp_ctl_t;

/**
 * cvmx_xcv_ctl
 *
 * This register contains the status control bits.
 *
 */
union cvmx_xcv_ctl {
	u64 u64;
	struct cvmx_xcv_ctl_s {
		u64 reserved_4_63 : 60;
		u64 lpbk_ext : 1;
		u64 lpbk_int : 1;
		u64 speed : 2;
	} s;
	struct cvmx_xcv_ctl_s cn73xx;
};

typedef union cvmx_xcv_ctl cvmx_xcv_ctl_t;

/**
 * cvmx_xcv_dll_ctl
 *
 * The RGMII timing specification requires that devices transmit clock and
 * data synchronously. The specification requires external sources (namely
 * the PC board trace routes) to introduce the appropriate 1.5 to 2.0 ns of
 * delay.
 *
 * To eliminate the need for the PC board delays, the RGMII interface has optional
 * on-board DLLs for both transmit and receive. For correct operation, at most one
 * of the transmitter, board, or receiver involved in an RGMII link should
 * introduce delay. By default/reset, the RGMII receivers delay the received clock,
 * and the RGMII transmitters do not delay the transmitted clock. Whether this
 * default works as-is with a given link partner depends on the behavior of the
 * link partner and the PC board.
 *
 * These are the possible modes of RGMII receive operation:
 *
 * * XCV_DLL_CTL[CLKRX_BYP] = 0 (reset value) - The RGMII
 * receive interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote
 * transmitter nor the PC board delays the clock.
 *
 * * XCV_DLL_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The
 * RGMII receive interface introduces no clock delay. This mode
 * is appropriate if either the remote transmitter or the PC board
 * delays the clock.
 *
 * These are the possible modes of RGMII transmit operation:
 *
 * * XCV_DLL_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) -
 * The RGMII transmit interface introduces no clock
 * delay. This mode is appropriate is either the remote receiver
 * or the PC board delays the clock.
 *
 * * XCV_DLL_CTL[CLKTX_BYP] = 0 - The RGMII transmit
 * interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote receiver
 * nor the PC board delays the clock.
 */
union cvmx_xcv_dll_ctl {
	u64 u64;
	struct cvmx_xcv_dll_ctl_s {
		u64 reserved_32_63 : 32;
		u64 lock : 1;
		u64 clk_set : 7;
		u64 clkrx_byp : 1;
		u64 clkrx_set : 7;
		u64 clktx_byp : 1;
		u64 clktx_set : 7;
		u64 reserved_2_7 : 6;
		u64 refclk_sel : 2;
	} s;
	struct cvmx_xcv_dll_ctl_s cn73xx;
};

typedef union cvmx_xcv_dll_ctl cvmx_xcv_dll_ctl_t;

/**
 * cvmx_xcv_eco
 */
union cvmx_xcv_eco {
	u64 u64;
	struct cvmx_xcv_eco_s {
		u64 reserved_16_63 : 48;
		u64 eco_rw : 16;
	} s;
	struct cvmx_xcv_eco_s cn73xx;
};

typedef union cvmx_xcv_eco cvmx_xcv_eco_t;

/**
 * cvmx_xcv_inbnd_status
 *
 * This register contains RGMII in-band status.
 *
 */
union cvmx_xcv_inbnd_status {
	u64 u64;
	struct cvmx_xcv_inbnd_status_s {
		u64 reserved_4_63 : 60;
		u64 duplex : 1;
		u64 speed : 2;
		u64 link : 1;
	} s;
	struct cvmx_xcv_inbnd_status_s cn73xx;
};

typedef union cvmx_xcv_inbnd_status cvmx_xcv_inbnd_status_t;

/**
 * cvmx_xcv_int
 *
 * This register controls interrupts.
 *
 */
union cvmx_xcv_int {
	u64 u64;
	struct cvmx_xcv_int_s {
		u64 reserved_7_63 : 57;
		u64 tx_ovrflw : 1;
		u64 tx_undflw : 1;
		u64 incomp_byte : 1;
		u64 duplex : 1;
		u64 reserved_2_2 : 1;
		u64 speed : 1;
		u64 link : 1;
	} s;
	struct cvmx_xcv_int_s cn73xx;
};

typedef union cvmx_xcv_int cvmx_xcv_int_t;

/**
 * cvmx_xcv_reset
 *
 * This register controls reset.
 *
 */
union cvmx_xcv_reset {
	u64 u64;
	struct cvmx_xcv_reset_s {
		u64 enable : 1;
		u64 reserved_16_62 : 47;
		u64 clkrst : 1;
		u64 reserved_12_14 : 3;
		u64 dllrst : 1;
		u64 reserved_8_10 : 3;
		u64 comp : 1;
		u64 reserved_4_6 : 3;
		u64 tx_pkt_rst_n : 1;
		u64 tx_dat_rst_n : 1;
		u64 rx_pkt_rst_n : 1;
		u64 rx_dat_rst_n : 1;
	} s;
	struct cvmx_xcv_reset_s cn73xx;
};

typedef union cvmx_xcv_reset cvmx_xcv_reset_t;

#endif
