// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>

const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn63xx[] = {
	{ "prbs_err_cnt", 299, 252 },	   // prbs_err_cnt[47..0]
	{ "prbs_lock", 251, 251 },	   // prbs_lock
	{ "jtg_prbs_rst_n", 250, 250 },	   // jtg_prbs_rst_n
	{ "jtg_run_prbs31", 249, 249 },	   // jtg_run_prbs31
	{ "jtg_run_prbs7", 248, 248 },	   // jtg_run_prbs7
	{ "Unused1", 247, 245 },	   // 0
	{ "cfg_pwrup_set", 244, 244 },	   // cfg_pwrup_set
	{ "cfg_pwrup_clr", 243, 243 },	   // cfg_pwrup_clr
	{ "cfg_rst_n_set", 242, 242 },	   // cfg_rst_n_set
	{ "cfg_rst_n_clr", 241, 241 },	   // cfg_rst_n_clr
	{ "cfg_tx_idle_set", 240, 240 },   // cfg_tx_idle_set
	{ "cfg_tx_idle_clr", 239, 239 },   // cfg_tx_idle_clr
	{ "cfg_tx_byp", 238, 238 },	   // cfg_tx_byp
	{ "cfg_tx_byp_inv", 237, 237 },	   // cfg_tx_byp_inv
	{ "cfg_tx_byp_val", 236, 227 },	   // cfg_tx_byp_val[9..0]
	{ "cfg_loopback", 226, 226 },	   // cfg_loopback
	{ "shlpbck", 225, 224 },	   // shlpbck[1..0]
	{ "sl_enable", 223, 223 },	   // sl_enable
	{ "sl_posedge_sample", 222, 222 }, // sl_posedge_sample
	{ "trimen", 221, 220 },		   // trimen[1..0]
	{ "serdes_tx_byp", 219, 219 },	   // serdes_tx_byp
	{ "serdes_pll_byp", 218, 218 },	   // serdes_pll_byp
	{ "lowf_byp", 217, 217 },	   // lowf_byp
	{ "spdsel_byp", 216, 216 },	   // spdsel_byp
	{ "div4_byp", 215, 215 },	   // div4_byp
	{ "clkf_byp", 214, 208 },	   // clkf_byp[6..0]
	{ "Unused2", 207, 206 },	   // 0
	{ "biasdrv_hs_ls_byp", 205, 201 }, // biasdrv_hs_ls_byp[4..0]
	{ "tcoeff_hf_ls_byp", 200, 197 },  // tcoeff_hf_ls_byp[3..0]
	{ "biasdrv_hf_byp", 196, 192 },	   // biasdrv_hf_byp[4..0]
	{ "tcoeff_hf_byp", 191, 188 },	   // tcoeff_hf_byp[3..0]
	{ "Unused3", 187, 186 },	   // 0
	{ "biasdrv_lf_ls_byp", 185, 181 }, // biasdrv_lf_ls_byp[4..0]
	{ "tcoeff_lf_ls_byp", 180, 177 },  // tcoeff_lf_ls_byp[3..0]
	{ "biasdrv_lf_byp", 176, 172 },	   // biasdrv_lf_byp[4..0]
	{ "tcoeff_lf_byp", 171, 168 },	   // tcoeff_lf_byp[3..0]
	{ "Unused4", 167, 167 },	   // 0
	{ "interpbw", 166, 162 },	   // interpbw[4..0]
	{ "pll_cpb", 161, 159 },	   // pll_cpb[2..0]
	{ "pll_cps", 158, 156 },	   // pll_cps[2..0]
	{ "pll_diffamp", 155, 152 },	   // pll_diffamp[3..0]
	{ "Unused5", 151, 150 },	   // 0
	{ "cfg_rx_idle_set", 149, 149 },   // cfg_rx_idle_set
	{ "cfg_rx_idle_clr", 148, 148 },   // cfg_rx_idle_clr
	{ "cfg_rx_idle_thr", 147, 144 },   // cfg_rx_idle_thr[3..0]
	{ "cfg_com_thr", 143, 140 },	   // cfg_com_thr[3..0]
	{ "cfg_rx_offset", 139, 136 },	   // cfg_rx_offset[3..0]
	{ "cfg_skp_max", 135, 132 },	   // cfg_skp_max[3..0]
	{ "cfg_skp_min", 131, 128 },	   // cfg_skp_min[3..0]
	{ "cfg_fast_pwrup", 127, 127 },	   // cfg_fast_pwrup
	{ "Unused6", 126, 100 },	   // 0
	{ "detected_n", 99, 99 },	   // detected_n
	{ "detected_p", 98, 98 },	   // detected_p
	{ "dbg_res_rx", 97, 94 },	   // dbg_res_rx[3..0]
	{ "dbg_res_tx", 93, 90 },	   // dbg_res_tx[3..0]
	{ "cfg_tx_pol_set", 89, 89 },	   // cfg_tx_pol_set
	{ "cfg_tx_pol_clr", 88, 88 },	   // cfg_tx_pol_clr
	{ "cfg_rx_pol_set", 87, 87 },	   // cfg_rx_pol_set
	{ "cfg_rx_pol_clr", 86, 86 },	   // cfg_rx_pol_clr
	{ "cfg_rxd_set", 85, 85 },	   // cfg_rxd_set
	{ "cfg_rxd_clr", 84, 84 },	   // cfg_rxd_clr
	{ "cfg_rxd_wait", 83, 80 },	   // cfg_rxd_wait[3..0]
	{ "cfg_cdr_limit", 79, 79 },	   // cfg_cdr_limit
	{ "cfg_cdr_rotate", 78, 78 },	   // cfg_cdr_rotate
	{ "cfg_cdr_bw_ctl", 77, 76 },	   // cfg_cdr_bw_ctl[1..0]
	{ "cfg_cdr_trunc", 75, 74 },	   // cfg_cdr_trunc[1..0]
	{ "cfg_cdr_rqoffs", 73, 64 },	   // cfg_cdr_rqoffs[9..0]
	{ "cfg_cdr_inc2", 63, 58 },	   // cfg_cdr_inc2[5..0]
	{ "cfg_cdr_inc1", 57, 52 },	   // cfg_cdr_inc1[5..0]
	{ "fusopt_voter_sync", 51, 51 },   // fusopt_voter_sync
	{ "rndt", 50, 50 },		   // rndt
	{ "hcya", 49, 49 },		   // hcya
	{ "hyst", 48, 48 },		   // hyst
	{ "idle_dac", 47, 45 },		   // idle_dac[2..0]
	{ "bg_ref_sel", 44, 44 },	   // bg_ref_sel
	{ "ic50dac", 43, 39 },		   // ic50dac[4..0]
	{ "ir50dac", 38, 34 },		   // ir50dac[4..0]
	{ "tx_rout_comp_bypass", 33, 33 }, // tx_rout_comp_bypass
	{ "tx_rout_comp_value", 32, 29 },  // tx_rout_comp_value[3..0]
	{ "tx_res_offset", 28, 25 },	   // tx_res_offset[3..0]
	{ "rx_rout_comp_bypass", 24, 24 }, // rx_rout_comp_bypass
	{ "rx_rout_comp_value", 23, 20 },  // rx_rout_comp_value[3..0]
	{ "rx_res_offset", 19, 16 },	   // rx_res_offset[3..0]
	{ "rx_cap_gen2", 15, 12 },	   // rx_cap_gen2[3..0]
	{ "rx_eq_gen2", 11, 8 },	   // rx_eq_gen2[3..0]
	{ "rx_cap_gen1", 7, 4 },	   // rx_cap_gen1[3..0]
	{ "rx_eq_gen1", 3, 0 },		   // rx_eq_gen1[3..0]
	{ NULL, -1, -1 }
};

const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn66xx[] = {
	{ "prbs_err_cnt", 303, 256 },	   // prbs_err_cnt[47..0]
	{ "prbs_lock", 255, 255 },	   // prbs_lock
	{ "jtg_prbs_rx_rst_n", 254, 254 }, // jtg_prbs_rx_rst_n
	{ "jtg_prbs_tx_rst_n", 253, 253 }, // jtg_prbs_tx_rst_n
	{ "jtg_prbs_mode", 252, 251 },	   // jtg_prbs_mode[252:251]
	{ "jtg_prbs_rst_n", 250, 250 },	   // jtg_prbs_rst_n
	{ "jtg_run_prbs31", 249,
	  249 }, // jtg_run_prbs31 - Use jtg_prbs_mode instead
	{ "jtg_run_prbs7", 248,
	  248 },		  // jtg_run_prbs7 - Use jtg_prbs_mode instead
	{ "Unused1", 247, 246 },  // 0
	{ "div5_byp", 245, 245 }, // div5_byp
	{ "cfg_pwrup_set", 244, 244 },	   // cfg_pwrup_set
	{ "cfg_pwrup_clr", 243, 243 },	   // cfg_pwrup_clr
	{ "cfg_rst_n_set", 242, 242 },	   // cfg_rst_n_set
	{ "cfg_rst_n_clr", 241, 241 },	   // cfg_rst_n_clr
	{ "cfg_tx_idle_set", 240, 240 },   // cfg_tx_idle_set
	{ "cfg_tx_idle_clr", 239, 239 },   // cfg_tx_idle_clr
	{ "cfg_tx_byp", 238, 238 },	   // cfg_tx_byp
	{ "cfg_tx_byp_inv", 237, 237 },	   // cfg_tx_byp_inv
	{ "cfg_tx_byp_val", 236, 227 },	   // cfg_tx_byp_val[9..0]
	{ "cfg_loopback", 226, 226 },	   // cfg_loopback
	{ "shlpbck", 225, 224 },	   // shlpbck[1..0]
	{ "sl_enable", 223, 223 },	   // sl_enable
	{ "sl_posedge_sample", 222, 222 }, // sl_posedge_sample
	{ "trimen", 221, 220 },		   // trimen[1..0]
	{ "serdes_tx_byp", 219, 219 },	   // serdes_tx_byp
	{ "serdes_pll_byp", 218, 218 },	   // serdes_pll_byp
	{ "lowf_byp", 217, 217 },	   // lowf_byp
	{ "spdsel_byp", 216, 216 },	   // spdsel_byp
	{ "div4_byp", 215, 215 },	   // div4_byp
	{ "clkf_byp", 214, 208 },	   // clkf_byp[6..0]
	{ "biasdrv_hs_ls_byp", 207, 203 }, // biasdrv_hs_ls_byp[4..0]
	{ "tcoeff_hf_ls_byp", 202, 198 },  // tcoeff_hf_ls_byp[4..0]
	{ "biasdrv_hf_byp", 197, 193 },	   // biasdrv_hf_byp[4..0]
	{ "tcoeff_hf_byp", 192, 188 },	   // tcoeff_hf_byp[4..0]
	{ "biasdrv_lf_ls_byp", 187, 183 }, // biasdrv_lf_ls_byp[4..0]
	{ "tcoeff_lf_ls_byp", 182, 178 },  // tcoeff_lf_ls_byp[4..0]
	{ "biasdrv_lf_byp", 177, 173 },	   // biasdrv_lf_byp[4..0]
	{ "tcoeff_lf_byp", 172, 168 },	   // tcoeff_lf_byp[4..0]
	{ "Unused4", 167, 167 },	   // 0
	{ "interpbw", 166, 162 },	   // interpbw[4..0]
	{ "pll_cpb", 161, 159 },	   // pll_cpb[2..0]
	{ "pll_cps", 158, 156 },	   // pll_cps[2..0]
	{ "pll_diffamp", 155, 152 },	   // pll_diffamp[3..0]
	{ "cfg_err_thr", 151, 150 },	   // cfg_err_thr
	{ "cfg_rx_idle_set", 149, 149 },   // cfg_rx_idle_set
	{ "cfg_rx_idle_clr", 148, 148 },   // cfg_rx_idle_clr
	{ "cfg_rx_idle_thr", 147, 144 },   // cfg_rx_idle_thr[3..0]
	{ "cfg_com_thr", 143, 140 },	   // cfg_com_thr[3..0]
	{ "cfg_rx_offset", 139, 136 },	   // cfg_rx_offset[3..0]
	{ "cfg_skp_max", 135, 132 },	   // cfg_skp_max[3..0]
	{ "cfg_skp_min", 131, 128 },	   // cfg_skp_min[3..0]
	{ "cfg_fast_pwrup", 127, 127 },	   // cfg_fast_pwrup
	{ "Unused6", 126, 101 },	   // 0
	{ "cfg_indep_dis", 100, 100 },	   // cfg_indep_dis
	{ "detected_n", 99, 99 },	   // detected_n
	{ "detected_p", 98, 98 },	   // detected_p
	{ "dbg_res_rx", 97, 94 },	   // dbg_res_rx[3..0]
	{ "dbg_res_tx", 93, 90 },	   // dbg_res_tx[3..0]
	{ "cfg_tx_pol_set", 89, 89 },	   // cfg_tx_pol_set
	{ "cfg_tx_pol_clr", 88, 88 },	   // cfg_tx_pol_clr
	{ "cfg_rx_pol_set", 87, 87 },	   // cfg_rx_pol_set
	{ "cfg_rx_pol_clr", 86, 86 },	   // cfg_rx_pol_clr
	{ "cfg_rxd_set", 85, 85 },	   // cfg_rxd_set
	{ "cfg_rxd_clr", 84, 84 },	   // cfg_rxd_clr
	{ "cfg_rxd_wait", 83, 80 },	   // cfg_rxd_wait[3..0]
	{ "cfg_cdr_limit", 79, 79 },	   // cfg_cdr_limit
	{ "cfg_cdr_rotate", 78, 78 },	   // cfg_cdr_rotate
	{ "cfg_cdr_bw_ctl", 77, 76 },	   // cfg_cdr_bw_ctl[1..0]
	{ "cfg_cdr_trunc", 75, 74 },	   // cfg_cdr_trunc[1..0]
	{ "cfg_cdr_rqoffs", 73, 64 },	   // cfg_cdr_rqoffs[9..0]
	{ "cfg_cdr_inc2", 63, 58 },	   // cfg_cdr_inc2[5..0]
	{ "cfg_cdr_inc1", 57, 52 },	   // cfg_cdr_inc1[5..0]
	{ "fusopt_voter_sync", 51, 51 },   // fusopt_voter_sync
	{ "rndt", 50, 50 },		   // rndt
	{ "hcya", 49, 49 },		   // hcya
	{ "hyst", 48, 48 },		   // hyst
	{ "idle_dac", 47, 45 },		   // idle_dac[2..0]
	{ "bg_ref_sel", 44, 44 },	   // bg_ref_sel
	{ "ic50dac", 43, 39 },		   // ic50dac[4..0]
	{ "ir50dac", 38, 34 },		   // ir50dac[4..0]
	{ "tx_rout_comp_bypass", 33, 33 }, // tx_rout_comp_bypass
	{ "tx_rout_comp_value", 32, 29 },  // tx_rout_comp_value[3..0]
	{ "tx_res_offset", 28, 25 },	   // tx_res_offset[3..0]
	{ "rx_rout_comp_bypass", 24, 24 }, // rx_rout_comp_bypass
	{ "rx_rout_comp_value", 23, 20 },  // rx_rout_comp_value[3..0]
	{ "rx_res_offset", 19, 16 },	   // rx_res_offset[3..0]
	{ "rx_cap_gen2", 15, 12 },	   // rx_cap_gen2[3..0]
	{ "rx_eq_gen2", 11, 8 },	   // rx_eq_gen2[3..0]
	{ "rx_cap_gen1", 7, 4 },	   // rx_cap_gen1[3..0]
	{ "rx_eq_gen1", 3, 0 },		   // rx_eq_gen1[3..0]
	{ NULL, -1, -1 }
};

const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn68xx[] = {
	{ "prbs_err_cnt", 303, 256 },	   // prbs_err_cnt[47..0]
	{ "prbs_lock", 255, 255 },	   // prbs_lock
	{ "jtg_prbs_rx_rst_n", 254, 254 }, // jtg_prbs_rx_rst_n
	{ "jtg_prbs_tx_rst_n", 253, 253 }, // jtg_prbs_tx_rst_n
	{ "jtg_prbs_mode", 252, 251 },	   // jtg_prbs_mode[252:251]
	{ "jtg_prbs_rst_n", 250, 250 },	   // jtg_prbs_rst_n
	{ "jtg_run_prbs31", 249,
	  249 }, // jtg_run_prbs31 - Use jtg_prbs_mode instead
	{ "jtg_run_prbs7", 248,
	  248 },		 // jtg_run_prbs7 - Use jtg_prbs_mode instead
	{ "Unused1", 247, 245 }, // 0
	{ "cfg_pwrup_set", 244, 244 },	   // cfg_pwrup_set
	{ "cfg_pwrup_clr", 243, 243 },	   // cfg_pwrup_clr
	{ "cfg_rst_n_set", 242, 242 },	   // cfg_rst_n_set
	{ "cfg_rst_n_clr", 241, 241 },	   // cfg_rst_n_clr
	{ "cfg_tx_idle_set", 240, 240 },   // cfg_tx_idle_set
	{ "cfg_tx_idle_clr", 239, 239 },   // cfg_tx_idle_clr
	{ "cfg_tx_byp", 238, 238 },	   // cfg_tx_byp
	{ "cfg_tx_byp_inv", 237, 237 },	   // cfg_tx_byp_inv
	{ "cfg_tx_byp_val", 236, 227 },	   // cfg_tx_byp_val[9..0]
	{ "cfg_loopback", 226, 226 },	   // cfg_loopback
	{ "shlpbck", 225, 224 },	   // shlpbck[1..0]
	{ "sl_enable", 223, 223 },	   // sl_enable
	{ "sl_posedge_sample", 222, 222 }, // sl_posedge_sample
	{ "trimen", 221, 220 },		   // trimen[1..0]
	{ "serdes_tx_byp", 219, 219 },	   // serdes_tx_byp
	{ "serdes_pll_byp", 218, 218 },	   // serdes_pll_byp
	{ "lowf_byp", 217, 217 },	   // lowf_byp
	{ "spdsel_byp", 216, 216 },	   // spdsel_byp
	{ "div4_byp", 215, 215 },	   // div4_byp
	{ "clkf_byp", 214, 208 },	   // clkf_byp[6..0]
	{ "biasdrv_hs_ls_byp", 207, 203 }, // biasdrv_hs_ls_byp[4..0]
	{ "tcoeff_hf_ls_byp", 202, 198 },  // tcoeff_hf_ls_byp[4..0]
	{ "biasdrv_hf_byp", 197, 193 },	   // biasdrv_hf_byp[4..0]
	{ "tcoeff_hf_byp", 192, 188 },	   // tcoeff_hf_byp[4..0]
	{ "biasdrv_lf_ls_byp", 187, 183 }, // biasdrv_lf_ls_byp[4..0]
	{ "tcoeff_lf_ls_byp", 182, 178 },  // tcoeff_lf_ls_byp[4..0]
	{ "biasdrv_lf_byp", 177, 173 },	   // biasdrv_lf_byp[4..0]
	{ "tcoeff_lf_byp", 172, 168 },	   // tcoeff_lf_byp[4..0]
	{ "Unused4", 167, 167 },	   // 0
	{ "interpbw", 166, 162 },	   // interpbw[4..0]
	{ "pll_cpb", 161, 159 },	   // pll_cpb[2..0]
	{ "pll_cps", 158, 156 },	   // pll_cps[2..0]
	{ "pll_diffamp", 155, 152 },	   // pll_diffamp[3..0]
	{ "cfg_err_thr", 151, 150 },	   // cfg_err_thr
	{ "cfg_rx_idle_set", 149, 149 },   // cfg_rx_idle_set
	{ "cfg_rx_idle_clr", 148, 148 },   // cfg_rx_idle_clr
	{ "cfg_rx_idle_thr", 147, 144 },   // cfg_rx_idle_thr[3..0]
	{ "cfg_com_thr", 143, 140 },	   // cfg_com_thr[3..0]
	{ "cfg_rx_offset", 139, 136 },	   // cfg_rx_offset[3..0]
	{ "cfg_skp_max", 135, 132 },	   // cfg_skp_max[3..0]
	{ "cfg_skp_min", 131, 128 },	   // cfg_skp_min[3..0]
	{ "cfg_fast_pwrup", 127, 127 },	   // cfg_fast_pwrup
	{ "Unused6", 126, 100 },	   // 0
	{ "detected_n", 99, 99 },	   // detected_n
	{ "detected_p", 98, 98 },	   // detected_p
	{ "dbg_res_rx", 97, 94 },	   // dbg_res_rx[3..0]
	{ "dbg_res_tx", 93, 90 },	   // dbg_res_tx[3..0]
	{ "cfg_tx_pol_set", 89, 89 },	   // cfg_tx_pol_set
	{ "cfg_tx_pol_clr", 88, 88 },	   // cfg_tx_pol_clr
	{ "cfg_rx_pol_set", 87, 87 },	   // cfg_rx_pol_set
	{ "cfg_rx_pol_clr", 86, 86 },	   // cfg_rx_pol_clr
	{ "cfg_rxd_set", 85, 85 },	   // cfg_rxd_set
	{ "cfg_rxd_clr", 84, 84 },	   // cfg_rxd_clr
	{ "cfg_rxd_wait", 83, 80 },	   // cfg_rxd_wait[3..0]
	{ "cfg_cdr_limit", 79, 79 },	   // cfg_cdr_limit
	{ "cfg_cdr_rotate", 78, 78 },	   // cfg_cdr_rotate
	{ "cfg_cdr_bw_ctl", 77, 76 },	   // cfg_cdr_bw_ctl[1..0]
	{ "cfg_cdr_trunc", 75, 74 },	   // cfg_cdr_trunc[1..0]
	{ "cfg_cdr_rqoffs", 73, 64 },	   // cfg_cdr_rqoffs[9..0]
	{ "cfg_cdr_inc2", 63, 58 },	   // cfg_cdr_inc2[5..0]
	{ "cfg_cdr_inc1", 57, 52 },	   // cfg_cdr_inc1[5..0]
	{ "fusopt_voter_sync", 51, 51 },   // fusopt_voter_sync
	{ "rndt", 50, 50 },		   // rndt
	{ "hcya", 49, 49 },		   // hcya
	{ "hyst", 48, 48 },		   // hyst
	{ "idle_dac", 47, 45 },		   // idle_dac[2..0]
	{ "bg_ref_sel", 44, 44 },	   // bg_ref_sel
	{ "ic50dac", 43, 39 },		   // ic50dac[4..0]
	{ "ir50dac", 38, 34 },		   // ir50dac[4..0]
	{ "tx_rout_comp_bypass", 33, 33 }, // tx_rout_comp_bypass
	{ "tx_rout_comp_value", 32, 29 },  // tx_rout_comp_value[3..0]
	{ "tx_res_offset", 28, 25 },	   // tx_res_offset[3..0]
	{ "rx_rout_comp_bypass", 24, 24 }, // rx_rout_comp_bypass
	{ "rx_rout_comp_value", 23, 20 },  // rx_rout_comp_value[3..0]
	{ "rx_res_offset", 19, 16 },	   // rx_res_offset[3..0]
	{ "rx_cap_gen2", 15, 12 },	   // rx_cap_gen2[3..0]
	{ "rx_eq_gen2", 11, 8 },	   // rx_eq_gen2[3..0]
	{ "rx_cap_gen1", 7, 4 },	   // rx_cap_gen1[3..0]
	{ "rx_eq_gen1", 3, 0 },		   // rx_eq_gen1[3..0]
	{ NULL, -1, -1 }
};
