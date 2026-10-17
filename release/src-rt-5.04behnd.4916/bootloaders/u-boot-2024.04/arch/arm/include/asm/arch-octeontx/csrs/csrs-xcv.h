/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_XCV_H__
#define __CSRS_XCV_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * XCV.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration xcv_bar_e
 *
 * XCV Base Address Register Enumeration Enumerates the base address
 * registers.
 */
#define XCV_BAR_E_XCVX_PF_BAR0(a) (0x87e0db000000ll + 0ll * (a))
#define XCV_BAR_E_XCVX_PF_BAR0_SIZE 0x100000ull
#define XCV_BAR_E_XCVX_PF_BAR4(a) (0x87e0dbf00000ll + 0ll * (a))
#define XCV_BAR_E_XCVX_PF_BAR4_SIZE 0x100000ull

/**
 * Enumeration xcv_int_vec_e
 *
 * XCV MSI-X Vector Enumeration Enumerates the MSI-X interrupt vectors.
 */
#define XCV_INT_VEC_E_INT (0)

/**
 * Register (RSL) xcv#_batch_crd_ret
 *
 * XCV Batch Credit Return Register
 */
union xcvx_batch_crd_ret {
	u64 u;
	struct xcvx_batch_crd_ret_s {
		u64 crd_ret                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct xcvx_batch_crd_ret_s cn; */
};

static inline u64 XCVX_BATCH_CRD_RET(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_BATCH_CRD_RET(u64 a)
{
	return 0x100 + 0 * a;
}

/**
 * Register (RSL) xcv#_comp_ctl
 *
 * XCV Compensation Controller Register This register controls
 * programmable compensation.
 */
union xcvx_comp_ctl {
	u64 u;
	struct xcvx_comp_ctl_s {
		u64 nctl_sat                         : 1;
		u64 reserved_1_26                    : 26;
		u64 nctl_lock                        : 1;
		u64 reserved_28                      : 1;
		u64 pctl_sat                         : 1;
		u64 pctl_lock                        : 1;
		u64 reserved_31                      : 1;
		u64 drv_nctl                         : 5;
		u64 reserved_37_39                   : 3;
		u64 drv_pctl                         : 5;
		u64 reserved_45_47                   : 3;
		u64 cmp_nctl                         : 5;
		u64 reserved_53_55                   : 3;
		u64 cmp_pctl                         : 5;
		u64 reserved_61_62                   : 2;
		u64 drv_byp                          : 1;
	} s;
	/* struct xcvx_comp_ctl_s cn; */
};

static inline u64 XCVX_COMP_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_COMP_CTL(u64 a)
{
	return 0x20 + 0 * a;
}

/**
 * Register (RSL) xcv#_ctl
 *
 * XCV Control Register This register contains the status control bits.
 */
union xcvx_ctl {
	u64 u;
	struct xcvx_ctl_s {
		u64 speed                            : 2;
		u64 lpbk_int                         : 1;
		u64 lpbk_ext                         : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct xcvx_ctl_s cn; */
};

static inline u64 XCVX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_CTL(u64 a)
{
	return 0x30 + 0 * a;
}

/**
 * Register (RSL) xcv#_dll_ctl
 *
 * XCV DLL Controller Register The RGMII timing specification requires
 * that devices transmit clock and data synchronously. The specification
 * requires external sources (namely the PC board trace routes) to
 * introduce the appropriate 1.5 to 2.0 ns of delay.  To eliminate the
 * need for the PC board delays, the RGMII interface has optional on-
 * board DLLs for both transmit and receive. For correct operation, at
 * most one of the transmitter, board, or receiver involved in an RGMII
 * link should introduce delay. By default/reset, the RGMII receivers
 * delay the received clock, and the RGMII transmitters do not delay the
 * transmitted clock. Whether this default works as-is with a given link
 * partner depends on the behavior of the link partner and the PC board.
 * These are the possible modes of RGMII receive operation:  *
 * XCV()_DLL_CTL[CLKRX_BYP] = 0 (reset value) - The RGMII receive
 * interface introduces clock delay using its internal DLL. This mode is
 * appropriate if neither the remote transmitter nor the PC board delays
 * the clock.  * XCV()_DLL_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The
 * RGMII receive interface introduces no clock delay. This mode is
 * appropriate if either the remote transmitter or the PC board delays
 * the clock.  These are the possible modes of RGMII transmit operation:
 * * XCV()_DLL_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) - The
 * RGMII transmit interface introduces no clock delay. This mode is
 * appropriate is either the remote receiver or the PC board delays the
 * clock.  * XCV()_DLL_CTL[CLKTX_BYP] = 0 - The RGMII transmit interface
 * introduces clock delay using its internal DLL. This mode is
 * appropriate if neither the remote receiver nor the PC board delays the
 * clock.
 */
union xcvx_dll_ctl {
	u64 u;
	struct xcvx_dll_ctl_s {
		u64 refclk_sel                       : 2;
		u64 reserved_2_7                     : 6;
		u64 clktx_set                        : 7;
		u64 clktx_byp                        : 1;
		u64 clkrx_set                        : 7;
		u64 clkrx_byp                        : 1;
		u64 clk_set                          : 7;
		u64 lock                             : 1;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct xcvx_dll_ctl_s cn; */
};

static inline u64 XCVX_DLL_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_DLL_CTL(u64 a)
{
	return 0x10 + 0 * a;
}

/**
 * Register (RSL) xcv#_eco
 *
 * INTERNAL: XCV ECO Register
 */
union xcvx_eco {
	u64 u;
	struct xcvx_eco_s {
		u64 eco_rw                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct xcvx_eco_s cn; */
};

static inline u64 XCVX_ECO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_ECO(u64 a)
{
	return 0x200 + 0 * a;
}

/**
 * Register (RSL) xcv#_inbnd_status
 *
 * XCV Inband Status Register This register contains RGMII inband status.
 */
union xcvx_inbnd_status {
	u64 u;
	struct xcvx_inbnd_status_s {
		u64 link                             : 1;
		u64 speed                            : 2;
		u64 duplex                           : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct xcvx_inbnd_status_s cn; */
};

static inline u64 XCVX_INBND_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_INBND_STATUS(u64 a)
{
	return 0x80 + 0 * a;
}

/**
 * Register (RSL) xcv#_int
 *
 * XCV Interrupt Register This register flags error for TX FIFO overflow,
 * TX FIFO underflow and incomplete byte for 10/100 Mode. It also flags
 * status change for link duplex, link speed and link up/down.
 */
union xcvx_int {
	u64 u;
	struct xcvx_int_s {
		u64 link                             : 1;
		u64 speed                            : 1;
		u64 reserved_2                       : 1;
		u64 duplex                           : 1;
		u64 incomp_byte                      : 1;
		u64 tx_undflw                        : 1;
		u64 tx_ovrflw                        : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct xcvx_int_s cn; */
};

static inline u64 XCVX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_INT(u64 a)
{
	return 0x40 + 0 * a;
}

/**
 * Register (RSL) xcv#_int_ena_w1c
 *
 * Loopback Error Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union xcvx_int_ena_w1c {
	u64 u;
	struct xcvx_int_ena_w1c_s {
		u64 link                             : 1;
		u64 speed                            : 1;
		u64 reserved_2                       : 1;
		u64 duplex                           : 1;
		u64 incomp_byte                      : 1;
		u64 tx_undflw                        : 1;
		u64 tx_ovrflw                        : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct xcvx_int_ena_w1c_s cn; */
};

static inline u64 XCVX_INT_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_INT_ENA_W1C(u64 a)
{
	return 0x50 + 0 * a;
}

/**
 * Register (RSL) xcv#_int_ena_w1s
 *
 * Loopback Error Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union xcvx_int_ena_w1s {
	u64 u;
	struct xcvx_int_ena_w1s_s {
		u64 link                             : 1;
		u64 speed                            : 1;
		u64 reserved_2                       : 1;
		u64 duplex                           : 1;
		u64 incomp_byte                      : 1;
		u64 tx_undflw                        : 1;
		u64 tx_ovrflw                        : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct xcvx_int_ena_w1s_s cn; */
};

static inline u64 XCVX_INT_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_INT_ENA_W1S(u64 a)
{
	return 0x58 + 0 * a;
}

/**
 * Register (RSL) xcv#_int_w1s
 *
 * Loopback Error Interrupt Set Register This register sets interrupt
 * bits.
 */
union xcvx_int_w1s {
	u64 u;
	struct xcvx_int_w1s_s {
		u64 link                             : 1;
		u64 speed                            : 1;
		u64 reserved_2                       : 1;
		u64 duplex                           : 1;
		u64 incomp_byte                      : 1;
		u64 tx_undflw                        : 1;
		u64 tx_ovrflw                        : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct xcvx_int_w1s_s cn; */
};

static inline u64 XCVX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_INT_W1S(u64 a)
{
	return 0x48 + 0 * a;
}

/**
 * Register (RSL) xcv#_msix_pba#
 *
 * XCV MSI-X Pending Bit Array Registers This register is the MSI-X PBA
 * table; the bit number is indexed by the XCV_INT_VEC_E enumeration.
 */
union xcvx_msix_pbax {
	u64 u;
	struct xcvx_msix_pbax_s {
		u64 pend                             : 64;
	} s;
	/* struct xcvx_msix_pbax_s cn; */
};

static inline u64 XCVX_MSIX_PBAX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_MSIX_PBAX(u64 a, u64 b)
{
	return 0xf0000 + 0 * a + 8 * b;
}

/**
 * Register (RSL) xcv#_msix_vec#_addr
 *
 * XCV MSI-X Vector-Table Address Register This register is the MSI-X
 * vector table, indexed by the XCV_INT_VEC_E enumeration.
 */
union xcvx_msix_vecx_addr {
	u64 u;
	struct xcvx_msix_vecx_addr_s {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 47;
		u64 reserved_49_63                   : 15;
	} s;
	/* struct xcvx_msix_vecx_addr_s cn; */
};

static inline u64 XCVX_MSIX_VECX_ADDR(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_MSIX_VECX_ADDR(u64 a, u64 b)
{
	return 0 + 0 * a + 0x10 * b;
}

/**
 * Register (RSL) xcv#_msix_vec#_ctl
 *
 * XCV MSI-X Vector-Table Control and Data Register This register is the
 * MSI-X vector table, indexed by the XCV_INT_VEC_E enumeration.
 */
union xcvx_msix_vecx_ctl {
	u64 u;
	struct xcvx_msix_vecx_ctl_s {
		u64 data                             : 20;
		u64 reserved_20_31                   : 12;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct xcvx_msix_vecx_ctl_s cn; */
};

static inline u64 XCVX_MSIX_VECX_CTL(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_MSIX_VECX_CTL(u64 a, u64 b)
{
	return 8 + 0 * a + 0x10 * b;
}

/**
 * Register (RSL) xcv#_reset
 *
 * XCV Reset Registers This register controls reset.
 */
union xcvx_reset {
	u64 u;
	struct xcvx_reset_s {
		u64 rx_dat_rst_n                     : 1;
		u64 rx_pkt_rst_n                     : 1;
		u64 tx_dat_rst_n                     : 1;
		u64 tx_pkt_rst_n                     : 1;
		u64 reserved_4_6                     : 3;
		u64 comp                             : 1;
		u64 reserved_8_10                    : 3;
		u64 dllrst                           : 1;
		u64 reserved_12_14                   : 3;
		u64 clkrst                           : 1;
		u64 reserved_16_62                   : 47;
		u64 enable                           : 1;
	} s;
	/* struct xcvx_reset_s cn; */
};

static inline u64 XCVX_RESET(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 XCVX_RESET(u64 a)
{
	return 0 + 0 * a;
}

#endif /* __CSRS_XCV_H__ */
