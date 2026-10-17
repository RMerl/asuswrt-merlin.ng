/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon lbk.
 */

#ifndef __CVMX_LBK_DEFS_H__
#define __CVMX_LBK_DEFS_H__

#define CVMX_LBK_BIST_RESULT	   (0x0001180012000020ull)
#define CVMX_LBK_CHX_PKIND(offset) (0x0001180012000200ull + ((offset) & 63) * 8)
#define CVMX_LBK_CLK_GATE_CTL	   (0x0001180012000008ull)
#define CVMX_LBK_DAT_ERR_INFO	   (0x0001180012000050ull)
#define CVMX_LBK_ECC_CFG	   (0x0001180012000060ull)
#define CVMX_LBK_INT		   (0x0001180012000040ull)
#define CVMX_LBK_SFT_RST	   (0x0001180012000000ull)

/**
 * cvmx_lbk_bist_result
 *
 * This register provides access to the internal BIST results. Each bit is the
 * BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_lbk_bist_result {
	u64 u64;
	struct cvmx_lbk_bist_result_s {
		u64 reserved_1_63 : 63;
		u64 dat : 1;
	} s;
	struct cvmx_lbk_bist_result_s cn73xx;
	struct cvmx_lbk_bist_result_s cn78xx;
	struct cvmx_lbk_bist_result_s cn78xxp1;
	struct cvmx_lbk_bist_result_s cnf75xx;
};

typedef union cvmx_lbk_bist_result cvmx_lbk_bist_result_t;

/**
 * cvmx_lbk_ch#_pkind
 */
union cvmx_lbk_chx_pkind {
	u64 u64;
	struct cvmx_lbk_chx_pkind_s {
		u64 reserved_6_63 : 58;
		u64 pkind : 6;
	} s;
	struct cvmx_lbk_chx_pkind_s cn73xx;
	struct cvmx_lbk_chx_pkind_s cn78xx;
	struct cvmx_lbk_chx_pkind_s cn78xxp1;
	struct cvmx_lbk_chx_pkind_s cnf75xx;
};

typedef union cvmx_lbk_chx_pkind cvmx_lbk_chx_pkind_t;

/**
 * cvmx_lbk_clk_gate_ctl
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lbk_clk_gate_ctl {
	u64 u64;
	struct cvmx_lbk_clk_gate_ctl_s {
		u64 reserved_1_63 : 63;
		u64 dis : 1;
	} s;
	struct cvmx_lbk_clk_gate_ctl_s cn73xx;
	struct cvmx_lbk_clk_gate_ctl_s cn78xx;
	struct cvmx_lbk_clk_gate_ctl_s cn78xxp1;
	struct cvmx_lbk_clk_gate_ctl_s cnf75xx;
};

typedef union cvmx_lbk_clk_gate_ctl cvmx_lbk_clk_gate_ctl_t;

/**
 * cvmx_lbk_dat_err_info
 */
union cvmx_lbk_dat_err_info {
	u64 u64;
	struct cvmx_lbk_dat_err_info_s {
		u64 reserved_58_63 : 6;
		u64 dbe_ecc_out : 9;
		u64 dbe_synd : 9;
		u64 dbe_addr : 8;
		u64 reserved_26_31 : 6;
		u64 sbe_ecc_out : 9;
		u64 sbe_synd : 9;
		u64 sbe_addr : 8;
	} s;
	struct cvmx_lbk_dat_err_info_s cn73xx;
	struct cvmx_lbk_dat_err_info_s cn78xx;
	struct cvmx_lbk_dat_err_info_s cn78xxp1;
	struct cvmx_lbk_dat_err_info_s cnf75xx;
};

typedef union cvmx_lbk_dat_err_info cvmx_lbk_dat_err_info_t;

/**
 * cvmx_lbk_ecc_cfg
 */
union cvmx_lbk_ecc_cfg {
	u64 u64;
	struct cvmx_lbk_ecc_cfg_s {
		u64 reserved_3_63 : 61;
		u64 dat_flip : 2;
		u64 dat_cdis : 1;
	} s;
	struct cvmx_lbk_ecc_cfg_s cn73xx;
	struct cvmx_lbk_ecc_cfg_s cn78xx;
	struct cvmx_lbk_ecc_cfg_s cn78xxp1;
	struct cvmx_lbk_ecc_cfg_s cnf75xx;
};

typedef union cvmx_lbk_ecc_cfg cvmx_lbk_ecc_cfg_t;

/**
 * cvmx_lbk_int
 */
union cvmx_lbk_int {
	u64 u64;
	struct cvmx_lbk_int_s {
		u64 reserved_6_63 : 58;
		u64 chan_oflow : 1;
		u64 chan_uflow : 1;
		u64 dat_oflow : 1;
		u64 dat_uflow : 1;
		u64 dat_dbe : 1;
		u64 dat_sbe : 1;
	} s;
	struct cvmx_lbk_int_s cn73xx;
	struct cvmx_lbk_int_s cn78xx;
	struct cvmx_lbk_int_s cn78xxp1;
	struct cvmx_lbk_int_s cnf75xx;
};

typedef union cvmx_lbk_int cvmx_lbk_int_t;

/**
 * cvmx_lbk_sft_rst
 */
union cvmx_lbk_sft_rst {
	u64 u64;
	struct cvmx_lbk_sft_rst_s {
		u64 reserved_1_63 : 63;
		u64 reset : 1;
	} s;
	struct cvmx_lbk_sft_rst_s cn73xx;
	struct cvmx_lbk_sft_rst_s cn78xx;
	struct cvmx_lbk_sft_rst_s cn78xxp1;
	struct cvmx_lbk_sft_rst_s cnf75xx;
};

typedef union cvmx_lbk_sft_rst cvmx_lbk_sft_rst_t;

#endif
