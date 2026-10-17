/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon fpa.
 */

#ifndef __CVMX_FPA_DEFS_H__
#define __CVMX_FPA_DEFS_H__

#define CVMX_FPA_ADDR_RANGE_ERROR CVMX_FPA_ADDR_RANGE_ERROR_FUNC()
static inline u64 CVMX_FPA_ADDR_RANGE_ERROR_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000458ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001280000000458ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001280000000458ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001280000000458ull;
	}
	return 0x0001280000000458ull;
}

#define CVMX_FPA_AURAX_CFG(offset)	     (0x0001280020100000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_CNT(offset)	     (0x0001280020200000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_CNT_ADD(offset)	     (0x0001280020300000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_CNT_LEVELS(offset)    (0x0001280020800000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_CNT_LIMIT(offset)     (0x0001280020400000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_CNT_THRESHOLD(offset) (0x0001280020500000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_INT(offset)	     (0x0001280020600000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_POOL(offset)	     (0x0001280020000000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_AURAX_POOL_LEVELS(offset)   (0x0001280020700000ull + ((offset) & 1023) * 8)
#define CVMX_FPA_BIST_STATUS		     CVMX_FPA_BIST_STATUS_FUNC()
static inline u64 CVMX_FPA_BIST_STATUS_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800280000E8ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00012800000000E8ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00012800000000E8ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00012800000000E8ull;
	}
	return 0x00012800000000E8ull;
}

#ifndef CVMX_FPA_CLK_COUNT // test-only (also in octeon_ddr.h)
#define CVMX_FPA_CLK_COUNT (0x00012800000000F0ull)
#endif
#define CVMX_FPA_CTL_STATUS		 (0x0001180028000050ull)
#define CVMX_FPA_ECC_CTL		 (0x0001280000000058ull)
#define CVMX_FPA_ECC_INT		 (0x0001280000000068ull)
#define CVMX_FPA_ERR_INT		 (0x0001280000000040ull)
#define CVMX_FPA_FPF0_MARKS		 (0x0001180028000000ull)
#define CVMX_FPA_FPF0_SIZE		 (0x0001180028000058ull)
#define CVMX_FPA_FPF1_MARKS		 CVMX_FPA_FPFX_MARKS(1)
#define CVMX_FPA_FPF2_MARKS		 CVMX_FPA_FPFX_MARKS(2)
#define CVMX_FPA_FPF3_MARKS		 CVMX_FPA_FPFX_MARKS(3)
#define CVMX_FPA_FPF4_MARKS		 CVMX_FPA_FPFX_MARKS(4)
#define CVMX_FPA_FPF5_MARKS		 CVMX_FPA_FPFX_MARKS(5)
#define CVMX_FPA_FPF6_MARKS		 CVMX_FPA_FPFX_MARKS(6)
#define CVMX_FPA_FPF7_MARKS		 CVMX_FPA_FPFX_MARKS(7)
#define CVMX_FPA_FPF8_MARKS		 (0x0001180028000240ull)
#define CVMX_FPA_FPF8_SIZE		 (0x0001180028000248ull)
#define CVMX_FPA_FPFX_MARKS(offset)	 (0x0001180028000008ull + ((offset) & 7) * 8 - 8 * 1)
#define CVMX_FPA_FPFX_SIZE(offset)	 (0x0001180028000060ull + ((offset) & 7) * 8 - 8 * 1)
#define CVMX_FPA_GEN_CFG		 (0x0001280000000050ull)
#define CVMX_FPA_INT_ENB		 (0x0001180028000048ull)
#define CVMX_FPA_INT_SUM		 (0x0001180028000040ull)
#define CVMX_FPA_PACKET_THRESHOLD	 (0x0001180028000460ull)
#define CVMX_FPA_POOLX_AVAILABLE(offset) (0x0001280010300000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_CFG(offset)	 (0x0001280010000000ull + ((offset) & 63) * 8)
static inline u64 CVMX_FPA_POOLX_END_ADDR(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000358ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000358ull + (offset) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001280010600000ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001280010600000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001280010600000ull + (offset) * 8;
	}
	return 0x0001280010600000ull + (offset) * 8;
}

#define CVMX_FPA_POOLX_FPF_MARKS(offset)  (0x0001280010100000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_INT(offset)	  (0x0001280010A00000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_OP_PC(offset)	  (0x0001280010F00000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_STACK_ADDR(offset) (0x0001280010900000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_STACK_BASE(offset) (0x0001280010700000ull + ((offset) & 63) * 8)
#define CVMX_FPA_POOLX_STACK_END(offset)  (0x0001280010800000ull + ((offset) & 63) * 8)
static inline u64 CVMX_FPA_POOLX_START_ADDR(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000258ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000258ull + (offset) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001280010500000ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001280010500000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001280010500000ull + (offset) * 8;
	}
	return 0x0001280010500000ull + (offset) * 8;
}

static inline u64 CVMX_FPA_POOLX_THRESHOLD(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000140ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180028000140ull + (offset) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001280010400000ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001280010400000ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001280010400000ull + (offset) * 8;
	}
	return 0x0001280010400000ull + (offset) * 8;
}

#define CVMX_FPA_QUE0_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(0)
#define CVMX_FPA_QUE1_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(1)
#define CVMX_FPA_QUE2_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(2)
#define CVMX_FPA_QUE3_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(3)
#define CVMX_FPA_QUE4_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(4)
#define CVMX_FPA_QUE5_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(5)
#define CVMX_FPA_QUE6_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(6)
#define CVMX_FPA_QUE7_PAGE_INDEX	 CVMX_FPA_QUEX_PAGE_INDEX(7)
#define CVMX_FPA_QUE8_PAGE_INDEX	 (0x0001180028000250ull)
#define CVMX_FPA_QUEX_AVAILABLE(offset)	 (0x0001180028000098ull + ((offset) & 15) * 8)
#define CVMX_FPA_QUEX_PAGE_INDEX(offset) (0x00011800280000F0ull + ((offset) & 7) * 8)
#define CVMX_FPA_QUE_ACT		 (0x0001180028000138ull)
#define CVMX_FPA_QUE_EXP		 (0x0001180028000130ull)
#define CVMX_FPA_RD_LATENCY_PC		 (0x0001280000000610ull)
#define CVMX_FPA_RD_REQ_PC		 (0x0001280000000600ull)
#define CVMX_FPA_RED_DELAY		 (0x0001280000000100ull)
#define CVMX_FPA_SFT_RST		 (0x0001280000000000ull)
#define CVMX_FPA_WART_CTL		 (0x00011800280000D8ull)
#define CVMX_FPA_WART_STATUS		 (0x00011800280000E0ull)
#define CVMX_FPA_WQE_THRESHOLD		 (0x0001180028000468ull)

/**
 * cvmx_fpa_addr_range_error
 *
 * When any FPA_POOL()_INT[RANGE] error occurs, this register is latched with additional
 * error information.
 */
union cvmx_fpa_addr_range_error {
	u64 u64;
	struct cvmx_fpa_addr_range_error_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_fpa_addr_range_error_cn61xx {
		u64 reserved_38_63 : 26;
		u64 pool : 5;
		u64 addr : 33;
	} cn61xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn66xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn68xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn68xxp1;
	struct cvmx_fpa_addr_range_error_cn61xx cn70xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn70xxp1;
	struct cvmx_fpa_addr_range_error_cn73xx {
		u64 reserved_54_63 : 10;
		u64 pool : 6;
		u64 reserved_42_47 : 6;
		u64 addr : 42;
	} cn73xx;
	struct cvmx_fpa_addr_range_error_cn73xx cn78xx;
	struct cvmx_fpa_addr_range_error_cn73xx cn78xxp1;
	struct cvmx_fpa_addr_range_error_cn61xx cnf71xx;
	struct cvmx_fpa_addr_range_error_cn73xx cnf75xx;
};

typedef union cvmx_fpa_addr_range_error cvmx_fpa_addr_range_error_t;

/**
 * cvmx_fpa_aura#_cfg
 *
 * This register configures aura backpressure, etc.
 *
 */
union cvmx_fpa_aurax_cfg {
	u64 u64;
	struct cvmx_fpa_aurax_cfg_s {
		u64 reserved_10_63 : 54;
		u64 ptr_dis : 1;
		u64 avg_con : 9;
	} s;
	struct cvmx_fpa_aurax_cfg_s cn73xx;
	struct cvmx_fpa_aurax_cfg_s cn78xx;
	struct cvmx_fpa_aurax_cfg_s cn78xxp1;
	struct cvmx_fpa_aurax_cfg_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cfg cvmx_fpa_aurax_cfg_t;

/**
 * cvmx_fpa_aura#_cnt
 */
union cvmx_fpa_aurax_cnt {
	u64 u64;
	struct cvmx_fpa_aurax_cnt_s {
		u64 reserved_40_63 : 24;
		u64 cnt : 40;
	} s;
	struct cvmx_fpa_aurax_cnt_s cn73xx;
	struct cvmx_fpa_aurax_cnt_s cn78xx;
	struct cvmx_fpa_aurax_cnt_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cnt cvmx_fpa_aurax_cnt_t;

/**
 * cvmx_fpa_aura#_cnt_add
 */
union cvmx_fpa_aurax_cnt_add {
	u64 u64;
	struct cvmx_fpa_aurax_cnt_add_s {
		u64 reserved_40_63 : 24;
		u64 cnt : 40;
	} s;
	struct cvmx_fpa_aurax_cnt_add_s cn73xx;
	struct cvmx_fpa_aurax_cnt_add_s cn78xx;
	struct cvmx_fpa_aurax_cnt_add_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_add_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cnt_add cvmx_fpa_aurax_cnt_add_t;

/**
 * cvmx_fpa_aura#_cnt_levels
 */
union cvmx_fpa_aurax_cnt_levels {
	u64 u64;
	struct cvmx_fpa_aurax_cnt_levels_s {
		u64 reserved_41_63 : 23;
		u64 drop_dis : 1;
		u64 bp_ena : 1;
		u64 red_ena : 1;
		u64 shift : 6;
		u64 bp : 8;
		u64 drop : 8;
		u64 pass : 8;
		u64 level : 8;
	} s;
	struct cvmx_fpa_aurax_cnt_levels_s cn73xx;
	struct cvmx_fpa_aurax_cnt_levels_s cn78xx;
	struct cvmx_fpa_aurax_cnt_levels_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_levels_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cnt_levels cvmx_fpa_aurax_cnt_levels_t;

/**
 * cvmx_fpa_aura#_cnt_limit
 */
union cvmx_fpa_aurax_cnt_limit {
	u64 u64;
	struct cvmx_fpa_aurax_cnt_limit_s {
		u64 reserved_40_63 : 24;
		u64 limit : 40;
	} s;
	struct cvmx_fpa_aurax_cnt_limit_s cn73xx;
	struct cvmx_fpa_aurax_cnt_limit_s cn78xx;
	struct cvmx_fpa_aurax_cnt_limit_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_limit_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cnt_limit cvmx_fpa_aurax_cnt_limit_t;

/**
 * cvmx_fpa_aura#_cnt_threshold
 */
union cvmx_fpa_aurax_cnt_threshold {
	u64 u64;
	struct cvmx_fpa_aurax_cnt_threshold_s {
		u64 reserved_40_63 : 24;
		u64 thresh : 40;
	} s;
	struct cvmx_fpa_aurax_cnt_threshold_s cn73xx;
	struct cvmx_fpa_aurax_cnt_threshold_s cn78xx;
	struct cvmx_fpa_aurax_cnt_threshold_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_threshold_s cnf75xx;
};

typedef union cvmx_fpa_aurax_cnt_threshold cvmx_fpa_aurax_cnt_threshold_t;

/**
 * cvmx_fpa_aura#_int
 */
union cvmx_fpa_aurax_int {
	u64 u64;
	struct cvmx_fpa_aurax_int_s {
		u64 reserved_1_63 : 63;
		u64 thresh : 1;
	} s;
	struct cvmx_fpa_aurax_int_s cn73xx;
	struct cvmx_fpa_aurax_int_s cn78xx;
	struct cvmx_fpa_aurax_int_s cn78xxp1;
	struct cvmx_fpa_aurax_int_s cnf75xx;
};

typedef union cvmx_fpa_aurax_int cvmx_fpa_aurax_int_t;

/**
 * cvmx_fpa_aura#_pool
 *
 * Provides the mapping from each aura to the pool number.
 *
 */
union cvmx_fpa_aurax_pool {
	u64 u64;
	struct cvmx_fpa_aurax_pool_s {
		u64 reserved_6_63 : 58;
		u64 pool : 6;
	} s;
	struct cvmx_fpa_aurax_pool_s cn73xx;
	struct cvmx_fpa_aurax_pool_s cn78xx;
	struct cvmx_fpa_aurax_pool_s cn78xxp1;
	struct cvmx_fpa_aurax_pool_s cnf75xx;
};

typedef union cvmx_fpa_aurax_pool cvmx_fpa_aurax_pool_t;

/**
 * cvmx_fpa_aura#_pool_levels
 */
union cvmx_fpa_aurax_pool_levels {
	u64 u64;
	struct cvmx_fpa_aurax_pool_levels_s {
		u64 reserved_41_63 : 23;
		u64 drop_dis : 1;
		u64 bp_ena : 1;
		u64 red_ena : 1;
		u64 shift : 6;
		u64 bp : 8;
		u64 drop : 8;
		u64 pass : 8;
		u64 level : 8;
	} s;
	struct cvmx_fpa_aurax_pool_levels_s cn73xx;
	struct cvmx_fpa_aurax_pool_levels_s cn78xx;
	struct cvmx_fpa_aurax_pool_levels_s cn78xxp1;
	struct cvmx_fpa_aurax_pool_levels_s cnf75xx;
};

typedef union cvmx_fpa_aurax_pool_levels cvmx_fpa_aurax_pool_levels_t;

/**
 * cvmx_fpa_bist_status
 *
 * This register provides the result of the BIST run on the FPA memories.
 *
 */
union cvmx_fpa_bist_status {
	u64 u64;
	struct cvmx_fpa_bist_status_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_fpa_bist_status_cn30xx {
		u64 reserved_5_63 : 59;
		u64 frd : 1;
		u64 fpf0 : 1;
		u64 fpf1 : 1;
		u64 ffr : 1;
		u64 fdr : 1;
	} cn30xx;
	struct cvmx_fpa_bist_status_cn30xx cn31xx;
	struct cvmx_fpa_bist_status_cn30xx cn38xx;
	struct cvmx_fpa_bist_status_cn30xx cn38xxp2;
	struct cvmx_fpa_bist_status_cn30xx cn50xx;
	struct cvmx_fpa_bist_status_cn30xx cn52xx;
	struct cvmx_fpa_bist_status_cn30xx cn52xxp1;
	struct cvmx_fpa_bist_status_cn30xx cn56xx;
	struct cvmx_fpa_bist_status_cn30xx cn56xxp1;
	struct cvmx_fpa_bist_status_cn30xx cn58xx;
	struct cvmx_fpa_bist_status_cn30xx cn58xxp1;
	struct cvmx_fpa_bist_status_cn30xx cn61xx;
	struct cvmx_fpa_bist_status_cn30xx cn63xx;
	struct cvmx_fpa_bist_status_cn30xx cn63xxp1;
	struct cvmx_fpa_bist_status_cn30xx cn66xx;
	struct cvmx_fpa_bist_status_cn30xx cn68xx;
	struct cvmx_fpa_bist_status_cn30xx cn68xxp1;
	struct cvmx_fpa_bist_status_cn30xx cn70xx;
	struct cvmx_fpa_bist_status_cn30xx cn70xxp1;
	struct cvmx_fpa_bist_status_cn73xx {
		u64 reserved_38_63 : 26;
		u64 status : 38;
	} cn73xx;
	struct cvmx_fpa_bist_status_cn73xx cn78xx;
	struct cvmx_fpa_bist_status_cn73xx cn78xxp1;
	struct cvmx_fpa_bist_status_cn30xx cnf71xx;
	struct cvmx_fpa_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_fpa_bist_status cvmx_fpa_bist_status_t;

/**
 * cvmx_fpa_clk_count
 *
 * This register counts the number of coprocessor-clock cycles since the deassertion of reset.
 *
 */
union cvmx_fpa_clk_count {
	u64 u64;
	struct cvmx_fpa_clk_count_s {
		u64 clk_cnt : 64;
	} s;
	struct cvmx_fpa_clk_count_s cn73xx;
	struct cvmx_fpa_clk_count_s cn78xx;
	struct cvmx_fpa_clk_count_s cn78xxp1;
	struct cvmx_fpa_clk_count_s cnf75xx;
};

typedef union cvmx_fpa_clk_count cvmx_fpa_clk_count_t;

/**
 * cvmx_fpa_ctl_status
 *
 * The FPA's interrupt enable register.
 *
 */
union cvmx_fpa_ctl_status {
	u64 u64;
	struct cvmx_fpa_ctl_status_s {
		u64 reserved_21_63 : 43;
		u64 free_en : 1;
		u64 ret_off : 1;
		u64 req_off : 1;
		u64 reset : 1;
		u64 use_ldt : 1;
		u64 use_stt : 1;
		u64 enb : 1;
		u64 mem1_err : 7;
		u64 mem0_err : 7;
	} s;
	struct cvmx_fpa_ctl_status_cn30xx {
		u64 reserved_18_63 : 46;
		u64 reset : 1;
		u64 use_ldt : 1;
		u64 use_stt : 1;
		u64 enb : 1;
		u64 mem1_err : 7;
		u64 mem0_err : 7;
	} cn30xx;
	struct cvmx_fpa_ctl_status_cn30xx cn31xx;
	struct cvmx_fpa_ctl_status_cn30xx cn38xx;
	struct cvmx_fpa_ctl_status_cn30xx cn38xxp2;
	struct cvmx_fpa_ctl_status_cn30xx cn50xx;
	struct cvmx_fpa_ctl_status_cn30xx cn52xx;
	struct cvmx_fpa_ctl_status_cn30xx cn52xxp1;
	struct cvmx_fpa_ctl_status_cn30xx cn56xx;
	struct cvmx_fpa_ctl_status_cn30xx cn56xxp1;
	struct cvmx_fpa_ctl_status_cn30xx cn58xx;
	struct cvmx_fpa_ctl_status_cn30xx cn58xxp1;
	struct cvmx_fpa_ctl_status_s cn61xx;
	struct cvmx_fpa_ctl_status_s cn63xx;
	struct cvmx_fpa_ctl_status_cn30xx cn63xxp1;
	struct cvmx_fpa_ctl_status_s cn66xx;
	struct cvmx_fpa_ctl_status_s cn68xx;
	struct cvmx_fpa_ctl_status_s cn68xxp1;
	struct cvmx_fpa_ctl_status_s cn70xx;
	struct cvmx_fpa_ctl_status_s cn70xxp1;
	struct cvmx_fpa_ctl_status_s cnf71xx;
};

typedef union cvmx_fpa_ctl_status cvmx_fpa_ctl_status_t;

/**
 * cvmx_fpa_ecc_ctl
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_fpa_ecc_ctl {
	u64 u64;
	struct cvmx_fpa_ecc_ctl_s {
		u64 reserved_62_63 : 2;
		u64 ram_flip1 : 20;
		u64 reserved_41_41 : 1;
		u64 ram_flip0 : 20;
		u64 reserved_20_20 : 1;
		u64 ram_cdis : 20;
	} s;
	struct cvmx_fpa_ecc_ctl_s cn73xx;
	struct cvmx_fpa_ecc_ctl_s cn78xx;
	struct cvmx_fpa_ecc_ctl_s cn78xxp1;
	struct cvmx_fpa_ecc_ctl_s cnf75xx;
};

typedef union cvmx_fpa_ecc_ctl cvmx_fpa_ecc_ctl_t;

/**
 * cvmx_fpa_ecc_int
 *
 * This register contains ECC error interrupt summary bits.
 *
 */
union cvmx_fpa_ecc_int {
	u64 u64;
	struct cvmx_fpa_ecc_int_s {
		u64 reserved_52_63 : 12;
		u64 ram_dbe : 20;
		u64 reserved_20_31 : 12;
		u64 ram_sbe : 20;
	} s;
	struct cvmx_fpa_ecc_int_s cn73xx;
	struct cvmx_fpa_ecc_int_s cn78xx;
	struct cvmx_fpa_ecc_int_s cn78xxp1;
	struct cvmx_fpa_ecc_int_s cnf75xx;
};

typedef union cvmx_fpa_ecc_int cvmx_fpa_ecc_int_t;

/**
 * cvmx_fpa_err_int
 *
 * This register contains the global (non-pool) error interrupt summary bits of the FPA.
 *
 */
union cvmx_fpa_err_int {
	u64 u64;
	struct cvmx_fpa_err_int_s {
		u64 reserved_4_63 : 60;
		u64 hw_sub : 1;
		u64 hw_add : 1;
		u64 cnt_sub : 1;
		u64 cnt_add : 1;
	} s;
	struct cvmx_fpa_err_int_s cn73xx;
	struct cvmx_fpa_err_int_s cn78xx;
	struct cvmx_fpa_err_int_s cn78xxp1;
	struct cvmx_fpa_err_int_s cnf75xx;
};

typedef union cvmx_fpa_err_int cvmx_fpa_err_int_t;

/**
 * cvmx_fpa_fpf#_marks
 *
 * "The high and low watermark register that determines when we write and read free pages from
 * L2C
 * for Queue 1. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend
 * value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)"
 */
union cvmx_fpa_fpfx_marks {
	u64 u64;
	struct cvmx_fpa_fpfx_marks_s {
		u64 reserved_22_63 : 42;
		u64 fpf_wr : 11;
		u64 fpf_rd : 11;
	} s;
	struct cvmx_fpa_fpfx_marks_s cn38xx;
	struct cvmx_fpa_fpfx_marks_s cn38xxp2;
	struct cvmx_fpa_fpfx_marks_s cn56xx;
	struct cvmx_fpa_fpfx_marks_s cn56xxp1;
	struct cvmx_fpa_fpfx_marks_s cn58xx;
	struct cvmx_fpa_fpfx_marks_s cn58xxp1;
	struct cvmx_fpa_fpfx_marks_s cn61xx;
	struct cvmx_fpa_fpfx_marks_s cn63xx;
	struct cvmx_fpa_fpfx_marks_s cn63xxp1;
	struct cvmx_fpa_fpfx_marks_s cn66xx;
	struct cvmx_fpa_fpfx_marks_s cn68xx;
	struct cvmx_fpa_fpfx_marks_s cn68xxp1;
	struct cvmx_fpa_fpfx_marks_s cn70xx;
	struct cvmx_fpa_fpfx_marks_s cn70xxp1;
	struct cvmx_fpa_fpfx_marks_s cnf71xx;
};

typedef union cvmx_fpa_fpfx_marks cvmx_fpa_fpfx_marks_t;

/**
 * cvmx_fpa_fpf#_size
 *
 * "FPA_FPFX_SIZE = FPA's Queue 1-7 Free Page FIFO Size
 * The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 8 (0-7) FPA_FPF#_SIZE registers must be limited to 2048."
 */
union cvmx_fpa_fpfx_size {
	u64 u64;
	struct cvmx_fpa_fpfx_size_s {
		u64 reserved_11_63 : 53;
		u64 fpf_siz : 11;
	} s;
	struct cvmx_fpa_fpfx_size_s cn38xx;
	struct cvmx_fpa_fpfx_size_s cn38xxp2;
	struct cvmx_fpa_fpfx_size_s cn56xx;
	struct cvmx_fpa_fpfx_size_s cn56xxp1;
	struct cvmx_fpa_fpfx_size_s cn58xx;
	struct cvmx_fpa_fpfx_size_s cn58xxp1;
	struct cvmx_fpa_fpfx_size_s cn61xx;
	struct cvmx_fpa_fpfx_size_s cn63xx;
	struct cvmx_fpa_fpfx_size_s cn63xxp1;
	struct cvmx_fpa_fpfx_size_s cn66xx;
	struct cvmx_fpa_fpfx_size_s cn68xx;
	struct cvmx_fpa_fpfx_size_s cn68xxp1;
	struct cvmx_fpa_fpfx_size_s cn70xx;
	struct cvmx_fpa_fpfx_size_s cn70xxp1;
	struct cvmx_fpa_fpfx_size_s cnf71xx;
};

typedef union cvmx_fpa_fpfx_size cvmx_fpa_fpfx_size_t;

/**
 * cvmx_fpa_fpf0_marks
 *
 * "The high and low watermark register that determines when we write and read free pages from
 * L2C
 * for Queue 0. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend
 * value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)"
 */
union cvmx_fpa_fpf0_marks {
	u64 u64;
	struct cvmx_fpa_fpf0_marks_s {
		u64 reserved_24_63 : 40;
		u64 fpf_wr : 12;
		u64 fpf_rd : 12;
	} s;
	struct cvmx_fpa_fpf0_marks_s cn38xx;
	struct cvmx_fpa_fpf0_marks_s cn38xxp2;
	struct cvmx_fpa_fpf0_marks_s cn56xx;
	struct cvmx_fpa_fpf0_marks_s cn56xxp1;
	struct cvmx_fpa_fpf0_marks_s cn58xx;
	struct cvmx_fpa_fpf0_marks_s cn58xxp1;
	struct cvmx_fpa_fpf0_marks_s cn61xx;
	struct cvmx_fpa_fpf0_marks_s cn63xx;
	struct cvmx_fpa_fpf0_marks_s cn63xxp1;
	struct cvmx_fpa_fpf0_marks_s cn66xx;
	struct cvmx_fpa_fpf0_marks_s cn68xx;
	struct cvmx_fpa_fpf0_marks_s cn68xxp1;
	struct cvmx_fpa_fpf0_marks_s cn70xx;
	struct cvmx_fpa_fpf0_marks_s cn70xxp1;
	struct cvmx_fpa_fpf0_marks_s cnf71xx;
};

typedef union cvmx_fpa_fpf0_marks cvmx_fpa_fpf0_marks_t;

/**
 * cvmx_fpa_fpf0_size
 *
 * "The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 8 (0-7) FPA_FPF#_SIZE registers must be limited to 2048."
 */
union cvmx_fpa_fpf0_size {
	u64 u64;
	struct cvmx_fpa_fpf0_size_s {
		u64 reserved_12_63 : 52;
		u64 fpf_siz : 12;
	} s;
	struct cvmx_fpa_fpf0_size_s cn38xx;
	struct cvmx_fpa_fpf0_size_s cn38xxp2;
	struct cvmx_fpa_fpf0_size_s cn56xx;
	struct cvmx_fpa_fpf0_size_s cn56xxp1;
	struct cvmx_fpa_fpf0_size_s cn58xx;
	struct cvmx_fpa_fpf0_size_s cn58xxp1;
	struct cvmx_fpa_fpf0_size_s cn61xx;
	struct cvmx_fpa_fpf0_size_s cn63xx;
	struct cvmx_fpa_fpf0_size_s cn63xxp1;
	struct cvmx_fpa_fpf0_size_s cn66xx;
	struct cvmx_fpa_fpf0_size_s cn68xx;
	struct cvmx_fpa_fpf0_size_s cn68xxp1;
	struct cvmx_fpa_fpf0_size_s cn70xx;
	struct cvmx_fpa_fpf0_size_s cn70xxp1;
	struct cvmx_fpa_fpf0_size_s cnf71xx;
};

typedef union cvmx_fpa_fpf0_size cvmx_fpa_fpf0_size_t;

/**
 * cvmx_fpa_fpf8_marks
 *
 * Reserved through 0x238 for additional thresholds
 *
 *                  FPA_FPF8_MARKS = FPA's Queue 8 Free Page FIFO Read Write Marks
 *
 * The high and low watermark register that determines when we write and read free pages from L2C
 * for Queue 8. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)
 */
union cvmx_fpa_fpf8_marks {
	u64 u64;
	struct cvmx_fpa_fpf8_marks_s {
		u64 reserved_22_63 : 42;
		u64 fpf_wr : 11;
		u64 fpf_rd : 11;
	} s;
	struct cvmx_fpa_fpf8_marks_s cn68xx;
	struct cvmx_fpa_fpf8_marks_s cn68xxp1;
};

typedef union cvmx_fpa_fpf8_marks cvmx_fpa_fpf8_marks_t;

/**
 * cvmx_fpa_fpf8_size
 *
 * FPA_FPF8_SIZE = FPA's Queue 8 Free Page FIFO Size
 *
 * The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 9 (0-8) FPA_FPF#_SIZE registers must be limited to 2048.
 */
union cvmx_fpa_fpf8_size {
	u64 u64;
	struct cvmx_fpa_fpf8_size_s {
		u64 reserved_12_63 : 52;
		u64 fpf_siz : 12;
	} s;
	struct cvmx_fpa_fpf8_size_s cn68xx;
	struct cvmx_fpa_fpf8_size_s cn68xxp1;
};

typedef union cvmx_fpa_fpf8_size cvmx_fpa_fpf8_size_t;

/**
 * cvmx_fpa_gen_cfg
 *
 * This register provides FPA control and status information.
 *
 */
union cvmx_fpa_gen_cfg {
	u64 u64;
	struct cvmx_fpa_gen_cfg_s {
		u64 reserved_12_63 : 52;
		u64 halfrate : 1;
		u64 ocla_bp : 1;
		u64 lvl_dly : 6;
		u64 pools : 2;
		u64 avg_en : 1;
		u64 clk_override : 1;
	} s;
	struct cvmx_fpa_gen_cfg_s cn73xx;
	struct cvmx_fpa_gen_cfg_s cn78xx;
	struct cvmx_fpa_gen_cfg_s cn78xxp1;
	struct cvmx_fpa_gen_cfg_s cnf75xx;
};

typedef union cvmx_fpa_gen_cfg cvmx_fpa_gen_cfg_t;

/**
 * cvmx_fpa_int_enb
 *
 * The FPA's interrupt enable register.
 *
 */
union cvmx_fpa_int_enb {
	u64 u64;
	struct cvmx_fpa_int_enb_s {
		u64 reserved_50_63 : 14;
		u64 paddr_e : 1;
		u64 reserved_44_48 : 5;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} s;
	struct cvmx_fpa_int_enb_cn30xx {
		u64 reserved_28_63 : 36;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn30xx;
	struct cvmx_fpa_int_enb_cn30xx cn31xx;
	struct cvmx_fpa_int_enb_cn30xx cn38xx;
	struct cvmx_fpa_int_enb_cn30xx cn38xxp2;
	struct cvmx_fpa_int_enb_cn30xx cn50xx;
	struct cvmx_fpa_int_enb_cn30xx cn52xx;
	struct cvmx_fpa_int_enb_cn30xx cn52xxp1;
	struct cvmx_fpa_int_enb_cn30xx cn56xx;
	struct cvmx_fpa_int_enb_cn30xx cn56xxp1;
	struct cvmx_fpa_int_enb_cn30xx cn58xx;
	struct cvmx_fpa_int_enb_cn30xx cn58xxp1;
	struct cvmx_fpa_int_enb_cn61xx {
		u64 reserved_50_63 : 14;
		u64 paddr_e : 1;
		u64 res_44 : 5;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn61xx;
	struct cvmx_fpa_int_enb_cn63xx {
		u64 reserved_44_63 : 20;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn63xx;
	struct cvmx_fpa_int_enb_cn30xx cn63xxp1;
	struct cvmx_fpa_int_enb_cn61xx cn66xx;
	struct cvmx_fpa_int_enb_cn68xx {
		u64 reserved_50_63 : 14;
		u64 paddr_e : 1;
		u64 pool8th : 1;
		u64 q8_perr : 1;
		u64 q8_coff : 1;
		u64 q8_und : 1;
		u64 free8 : 1;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn68xx;
	struct cvmx_fpa_int_enb_cn68xx cn68xxp1;
	struct cvmx_fpa_int_enb_cn61xx cn70xx;
	struct cvmx_fpa_int_enb_cn61xx cn70xxp1;
	struct cvmx_fpa_int_enb_cn61xx cnf71xx;
};

typedef union cvmx_fpa_int_enb cvmx_fpa_int_enb_t;

/**
 * cvmx_fpa_int_sum
 *
 * Contains the different interrupt summary bits of the FPA.
 *
 */
union cvmx_fpa_int_sum {
	u64 u64;
	struct cvmx_fpa_int_sum_s {
		u64 reserved_50_63 : 14;
		u64 paddr_e : 1;
		u64 pool8th : 1;
		u64 q8_perr : 1;
		u64 q8_coff : 1;
		u64 q8_und : 1;
		u64 free8 : 1;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} s;
	struct cvmx_fpa_int_sum_cn30xx {
		u64 reserved_28_63 : 36;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn30xx;
	struct cvmx_fpa_int_sum_cn30xx cn31xx;
	struct cvmx_fpa_int_sum_cn30xx cn38xx;
	struct cvmx_fpa_int_sum_cn30xx cn38xxp2;
	struct cvmx_fpa_int_sum_cn30xx cn50xx;
	struct cvmx_fpa_int_sum_cn30xx cn52xx;
	struct cvmx_fpa_int_sum_cn30xx cn52xxp1;
	struct cvmx_fpa_int_sum_cn30xx cn56xx;
	struct cvmx_fpa_int_sum_cn30xx cn56xxp1;
	struct cvmx_fpa_int_sum_cn30xx cn58xx;
	struct cvmx_fpa_int_sum_cn30xx cn58xxp1;
	struct cvmx_fpa_int_sum_cn61xx {
		u64 reserved_50_63 : 14;
		u64 paddr_e : 1;
		u64 reserved_44_48 : 5;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn61xx;
	struct cvmx_fpa_int_sum_cn63xx {
		u64 reserved_44_63 : 20;
		u64 free7 : 1;
		u64 free6 : 1;
		u64 free5 : 1;
		u64 free4 : 1;
		u64 free3 : 1;
		u64 free2 : 1;
		u64 free1 : 1;
		u64 free0 : 1;
		u64 pool7th : 1;
		u64 pool6th : 1;
		u64 pool5th : 1;
		u64 pool4th : 1;
		u64 pool3th : 1;
		u64 pool2th : 1;
		u64 pool1th : 1;
		u64 pool0th : 1;
		u64 q7_perr : 1;
		u64 q7_coff : 1;
		u64 q7_und : 1;
		u64 q6_perr : 1;
		u64 q6_coff : 1;
		u64 q6_und : 1;
		u64 q5_perr : 1;
		u64 q5_coff : 1;
		u64 q5_und : 1;
		u64 q4_perr : 1;
		u64 q4_coff : 1;
		u64 q4_und : 1;
		u64 q3_perr : 1;
		u64 q3_coff : 1;
		u64 q3_und : 1;
		u64 q2_perr : 1;
		u64 q2_coff : 1;
		u64 q2_und : 1;
		u64 q1_perr : 1;
		u64 q1_coff : 1;
		u64 q1_und : 1;
		u64 q0_perr : 1;
		u64 q0_coff : 1;
		u64 q0_und : 1;
		u64 fed1_dbe : 1;
		u64 fed1_sbe : 1;
		u64 fed0_dbe : 1;
		u64 fed0_sbe : 1;
	} cn63xx;
	struct cvmx_fpa_int_sum_cn30xx cn63xxp1;
	struct cvmx_fpa_int_sum_cn61xx cn66xx;
	struct cvmx_fpa_int_sum_s cn68xx;
	struct cvmx_fpa_int_sum_s cn68xxp1;
	struct cvmx_fpa_int_sum_cn61xx cn70xx;
	struct cvmx_fpa_int_sum_cn61xx cn70xxp1;
	struct cvmx_fpa_int_sum_cn61xx cnf71xx;
};

typedef union cvmx_fpa_int_sum cvmx_fpa_int_sum_t;

/**
 * cvmx_fpa_packet_threshold
 *
 * When the value of FPA_QUE0_AVAILABLE[QUE_SIZ] is Less than the value of this register a low
 * pool count signal is sent to the
 * PCIe packet instruction engine (to make it stop reading instructions) and to the Packet-
 * Arbiter informing it to not give grants
 * to packets MAC with the exception of the PCIe MAC.
 */
union cvmx_fpa_packet_threshold {
	u64 u64;
	struct cvmx_fpa_packet_threshold_s {
		u64 reserved_32_63 : 32;
		u64 thresh : 32;
	} s;
	struct cvmx_fpa_packet_threshold_s cn61xx;
	struct cvmx_fpa_packet_threshold_s cn63xx;
	struct cvmx_fpa_packet_threshold_s cn66xx;
	struct cvmx_fpa_packet_threshold_s cn68xx;
	struct cvmx_fpa_packet_threshold_s cn68xxp1;
	struct cvmx_fpa_packet_threshold_s cn70xx;
	struct cvmx_fpa_packet_threshold_s cn70xxp1;
	struct cvmx_fpa_packet_threshold_s cnf71xx;
};

typedef union cvmx_fpa_packet_threshold cvmx_fpa_packet_threshold_t;

/**
 * cvmx_fpa_pool#_available
 */
union cvmx_fpa_poolx_available {
	u64 u64;
	struct cvmx_fpa_poolx_available_s {
		u64 reserved_36_63 : 28;
		u64 count : 36;
	} s;
	struct cvmx_fpa_poolx_available_s cn73xx;
	struct cvmx_fpa_poolx_available_s cn78xx;
	struct cvmx_fpa_poolx_available_s cn78xxp1;
	struct cvmx_fpa_poolx_available_s cnf75xx;
};

typedef union cvmx_fpa_poolx_available cvmx_fpa_poolx_available_t;

/**
 * cvmx_fpa_pool#_cfg
 */
union cvmx_fpa_poolx_cfg {
	u64 u64;
	struct cvmx_fpa_poolx_cfg_s {
		u64 reserved_43_63 : 21;
		u64 buf_size : 11;
		u64 reserved_31_31 : 1;
		u64 buf_offset : 15;
		u64 reserved_5_15 : 11;
		u64 l_type : 2;
		u64 s_type : 1;
		u64 nat_align : 1;
		u64 ena : 1;
	} s;
	struct cvmx_fpa_poolx_cfg_s cn73xx;
	struct cvmx_fpa_poolx_cfg_s cn78xx;
	struct cvmx_fpa_poolx_cfg_s cn78xxp1;
	struct cvmx_fpa_poolx_cfg_s cnf75xx;
};

typedef union cvmx_fpa_poolx_cfg cvmx_fpa_poolx_cfg_t;

/**
 * cvmx_fpa_pool#_end_addr
 *
 * Pointers sent to this pool after alignment must be equal to or less than this address.
 *
 */
union cvmx_fpa_poolx_end_addr {
	u64 u64;
	struct cvmx_fpa_poolx_end_addr_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_fpa_poolx_end_addr_cn61xx {
		u64 reserved_33_63 : 31;
		u64 addr : 33;
	} cn61xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn66xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn68xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn68xxp1;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn70xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_end_addr_cn73xx {
		u64 reserved_42_63 : 22;
		u64 addr : 35;
		u64 reserved_0_6 : 7;
	} cn73xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cn78xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cn78xxp1;
	struct cvmx_fpa_poolx_end_addr_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cnf75xx;
};

typedef union cvmx_fpa_poolx_end_addr cvmx_fpa_poolx_end_addr_t;

/**
 * cvmx_fpa_pool#_fpf_marks
 *
 * The low watermark register that determines when we read free pages from L2C.
 *
 */
union cvmx_fpa_poolx_fpf_marks {
	u64 u64;
	struct cvmx_fpa_poolx_fpf_marks_s {
		u64 reserved_27_63 : 37;
		u64 fpf_rd : 11;
		u64 reserved_11_15 : 5;
		u64 fpf_level : 11;
	} s;
	struct cvmx_fpa_poolx_fpf_marks_s cn73xx;
	struct cvmx_fpa_poolx_fpf_marks_s cn78xx;
	struct cvmx_fpa_poolx_fpf_marks_s cn78xxp1;
	struct cvmx_fpa_poolx_fpf_marks_s cnf75xx;
};

typedef union cvmx_fpa_poolx_fpf_marks cvmx_fpa_poolx_fpf_marks_t;

/**
 * cvmx_fpa_pool#_int
 *
 * This register indicates pool interrupts.
 *
 */
union cvmx_fpa_poolx_int {
	u64 u64;
	struct cvmx_fpa_poolx_int_s {
		u64 reserved_4_63 : 60;
		u64 thresh : 1;
		u64 range : 1;
		u64 crcerr : 1;
		u64 ovfls : 1;
	} s;
	struct cvmx_fpa_poolx_int_s cn73xx;
	struct cvmx_fpa_poolx_int_s cn78xx;
	struct cvmx_fpa_poolx_int_s cn78xxp1;
	struct cvmx_fpa_poolx_int_s cnf75xx;
};

typedef union cvmx_fpa_poolx_int cvmx_fpa_poolx_int_t;

/**
 * cvmx_fpa_pool#_op_pc
 */
union cvmx_fpa_poolx_op_pc {
	u64 u64;
	struct cvmx_fpa_poolx_op_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_fpa_poolx_op_pc_s cn73xx;
	struct cvmx_fpa_poolx_op_pc_s cn78xx;
	struct cvmx_fpa_poolx_op_pc_s cn78xxp1;
	struct cvmx_fpa_poolx_op_pc_s cnf75xx;
};

typedef union cvmx_fpa_poolx_op_pc cvmx_fpa_poolx_op_pc_t;

/**
 * cvmx_fpa_pool#_stack_addr
 */
union cvmx_fpa_poolx_stack_addr {
	u64 u64;
	struct cvmx_fpa_poolx_stack_addr_s {
		u64 reserved_42_63 : 22;
		u64 addr : 35;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_fpa_poolx_stack_addr_s cn73xx;
	struct cvmx_fpa_poolx_stack_addr_s cn78xx;
	struct cvmx_fpa_poolx_stack_addr_s cn78xxp1;
	struct cvmx_fpa_poolx_stack_addr_s cnf75xx;
};

typedef union cvmx_fpa_poolx_stack_addr cvmx_fpa_poolx_stack_addr_t;

/**
 * cvmx_fpa_pool#_stack_base
 */
union cvmx_fpa_poolx_stack_base {
	u64 u64;
	struct cvmx_fpa_poolx_stack_base_s {
		u64 reserved_42_63 : 22;
		u64 addr : 35;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_fpa_poolx_stack_base_s cn73xx;
	struct cvmx_fpa_poolx_stack_base_s cn78xx;
	struct cvmx_fpa_poolx_stack_base_s cn78xxp1;
	struct cvmx_fpa_poolx_stack_base_s cnf75xx;
};

typedef union cvmx_fpa_poolx_stack_base cvmx_fpa_poolx_stack_base_t;

/**
 * cvmx_fpa_pool#_stack_end
 */
union cvmx_fpa_poolx_stack_end {
	u64 u64;
	struct cvmx_fpa_poolx_stack_end_s {
		u64 reserved_42_63 : 22;
		u64 addr : 35;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_fpa_poolx_stack_end_s cn73xx;
	struct cvmx_fpa_poolx_stack_end_s cn78xx;
	struct cvmx_fpa_poolx_stack_end_s cn78xxp1;
	struct cvmx_fpa_poolx_stack_end_s cnf75xx;
};

typedef union cvmx_fpa_poolx_stack_end cvmx_fpa_poolx_stack_end_t;

/**
 * cvmx_fpa_pool#_start_addr
 *
 * Pointers sent to this pool after alignment must be equal to or greater than this address.
 *
 */
union cvmx_fpa_poolx_start_addr {
	u64 u64;
	struct cvmx_fpa_poolx_start_addr_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_fpa_poolx_start_addr_cn61xx {
		u64 reserved_33_63 : 31;
		u64 addr : 33;
	} cn61xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn66xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn68xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn68xxp1;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn70xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_start_addr_cn73xx {
		u64 reserved_42_63 : 22;
		u64 addr : 35;
		u64 reserved_0_6 : 7;
	} cn73xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cn78xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cn78xxp1;
	struct cvmx_fpa_poolx_start_addr_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cnf75xx;
};

typedef union cvmx_fpa_poolx_start_addr cvmx_fpa_poolx_start_addr_t;

/**
 * cvmx_fpa_pool#_threshold
 *
 * FPA_POOLX_THRESHOLD = FPA's Pool 0-7 Threshold
 * When the value of FPA_QUEX_AVAILABLE is equal to FPA_POOLX_THRESHOLD[THRESH] when a pointer is
 * allocated
 * or deallocated, set interrupt FPA_INT_SUM[POOLXTH].
 */
union cvmx_fpa_poolx_threshold {
	u64 u64;
	struct cvmx_fpa_poolx_threshold_s {
		u64 reserved_36_63 : 28;
		u64 thresh : 36;
	} s;
	struct cvmx_fpa_poolx_threshold_cn61xx {
		u64 reserved_29_63 : 35;
		u64 thresh : 29;
	} cn61xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn63xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn66xx;
	struct cvmx_fpa_poolx_threshold_cn68xx {
		u64 reserved_32_63 : 32;
		u64 thresh : 32;
	} cn68xx;
	struct cvmx_fpa_poolx_threshold_cn68xx cn68xxp1;
	struct cvmx_fpa_poolx_threshold_cn61xx cn70xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_threshold_s cn73xx;
	struct cvmx_fpa_poolx_threshold_s cn78xx;
	struct cvmx_fpa_poolx_threshold_s cn78xxp1;
	struct cvmx_fpa_poolx_threshold_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_threshold_s cnf75xx;
};

typedef union cvmx_fpa_poolx_threshold cvmx_fpa_poolx_threshold_t;

/**
 * cvmx_fpa_que#_available
 *
 * FPA_QUEX_PAGES_AVAILABLE = FPA's Queue 0-7 Free Page Available Register
 * The number of page pointers that are available in the FPA and local DRAM.
 */
union cvmx_fpa_quex_available {
	u64 u64;
	struct cvmx_fpa_quex_available_s {
		u64 reserved_32_63 : 32;
		u64 que_siz : 32;
	} s;
	struct cvmx_fpa_quex_available_cn30xx {
		u64 reserved_29_63 : 35;
		u64 que_siz : 29;
	} cn30xx;
	struct cvmx_fpa_quex_available_cn30xx cn31xx;
	struct cvmx_fpa_quex_available_cn30xx cn38xx;
	struct cvmx_fpa_quex_available_cn30xx cn38xxp2;
	struct cvmx_fpa_quex_available_cn30xx cn50xx;
	struct cvmx_fpa_quex_available_cn30xx cn52xx;
	struct cvmx_fpa_quex_available_cn30xx cn52xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn56xx;
	struct cvmx_fpa_quex_available_cn30xx cn56xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn58xx;
	struct cvmx_fpa_quex_available_cn30xx cn58xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn61xx;
	struct cvmx_fpa_quex_available_cn30xx cn63xx;
	struct cvmx_fpa_quex_available_cn30xx cn63xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn66xx;
	struct cvmx_fpa_quex_available_s cn68xx;
	struct cvmx_fpa_quex_available_s cn68xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn70xx;
	struct cvmx_fpa_quex_available_cn30xx cn70xxp1;
	struct cvmx_fpa_quex_available_cn30xx cnf71xx;
};

typedef union cvmx_fpa_quex_available cvmx_fpa_quex_available_t;

/**
 * cvmx_fpa_que#_page_index
 *
 * The present index page for queue 0 of the FPA.
 * This number reflects the number of pages of pointers that have been written to memory
 * for this queue.
 */
union cvmx_fpa_quex_page_index {
	u64 u64;
	struct cvmx_fpa_quex_page_index_s {
		u64 reserved_25_63 : 39;
		u64 pg_num : 25;
	} s;
	struct cvmx_fpa_quex_page_index_s cn30xx;
	struct cvmx_fpa_quex_page_index_s cn31xx;
	struct cvmx_fpa_quex_page_index_s cn38xx;
	struct cvmx_fpa_quex_page_index_s cn38xxp2;
	struct cvmx_fpa_quex_page_index_s cn50xx;
	struct cvmx_fpa_quex_page_index_s cn52xx;
	struct cvmx_fpa_quex_page_index_s cn52xxp1;
	struct cvmx_fpa_quex_page_index_s cn56xx;
	struct cvmx_fpa_quex_page_index_s cn56xxp1;
	struct cvmx_fpa_quex_page_index_s cn58xx;
	struct cvmx_fpa_quex_page_index_s cn58xxp1;
	struct cvmx_fpa_quex_page_index_s cn61xx;
	struct cvmx_fpa_quex_page_index_s cn63xx;
	struct cvmx_fpa_quex_page_index_s cn63xxp1;
	struct cvmx_fpa_quex_page_index_s cn66xx;
	struct cvmx_fpa_quex_page_index_s cn68xx;
	struct cvmx_fpa_quex_page_index_s cn68xxp1;
	struct cvmx_fpa_quex_page_index_s cn70xx;
	struct cvmx_fpa_quex_page_index_s cn70xxp1;
	struct cvmx_fpa_quex_page_index_s cnf71xx;
};

typedef union cvmx_fpa_quex_page_index cvmx_fpa_quex_page_index_t;

/**
 * cvmx_fpa_que8_page_index
 *
 * FPA_QUE8_PAGE_INDEX = FPA's Queue7 Page Index
 *
 * The present index page for queue 7 of the FPA.
 * This number reflects the number of pages of pointers that have been written to memory
 * for this queue.
 * Because the address space is 38-bits the number of 128 byte pages could cause this register value to wrap.
 */
union cvmx_fpa_que8_page_index {
	u64 u64;
	struct cvmx_fpa_que8_page_index_s {
		u64 reserved_25_63 : 39;
		u64 pg_num : 25;
	} s;
	struct cvmx_fpa_que8_page_index_s cn68xx;
	struct cvmx_fpa_que8_page_index_s cn68xxp1;
};

typedef union cvmx_fpa_que8_page_index cvmx_fpa_que8_page_index_t;

/**
 * cvmx_fpa_que_act
 *
 * "When a INT_SUM[PERR#] occurs this will be latched with the value read from L2C.
 * This is latched on the first error and will not latch again unitl all errors are cleared."
 */
union cvmx_fpa_que_act {
	u64 u64;
	struct cvmx_fpa_que_act_s {
		u64 reserved_29_63 : 35;
		u64 act_que : 3;
		u64 act_indx : 26;
	} s;
	struct cvmx_fpa_que_act_s cn30xx;
	struct cvmx_fpa_que_act_s cn31xx;
	struct cvmx_fpa_que_act_s cn38xx;
	struct cvmx_fpa_que_act_s cn38xxp2;
	struct cvmx_fpa_que_act_s cn50xx;
	struct cvmx_fpa_que_act_s cn52xx;
	struct cvmx_fpa_que_act_s cn52xxp1;
	struct cvmx_fpa_que_act_s cn56xx;
	struct cvmx_fpa_que_act_s cn56xxp1;
	struct cvmx_fpa_que_act_s cn58xx;
	struct cvmx_fpa_que_act_s cn58xxp1;
	struct cvmx_fpa_que_act_s cn61xx;
	struct cvmx_fpa_que_act_s cn63xx;
	struct cvmx_fpa_que_act_s cn63xxp1;
	struct cvmx_fpa_que_act_s cn66xx;
	struct cvmx_fpa_que_act_s cn68xx;
	struct cvmx_fpa_que_act_s cn68xxp1;
	struct cvmx_fpa_que_act_s cn70xx;
	struct cvmx_fpa_que_act_s cn70xxp1;
	struct cvmx_fpa_que_act_s cnf71xx;
};

typedef union cvmx_fpa_que_act cvmx_fpa_que_act_t;

/**
 * cvmx_fpa_que_exp
 *
 * "When a INT_SUM[PERR#] occurs this will be latched with the expected value.
 * This is latched on the first error and will not latch again unitl all errors are cleared."
 */
union cvmx_fpa_que_exp {
	u64 u64;
	struct cvmx_fpa_que_exp_s {
		u64 reserved_29_63 : 35;
		u64 exp_que : 3;
		u64 exp_indx : 26;
	} s;
	struct cvmx_fpa_que_exp_s cn30xx;
	struct cvmx_fpa_que_exp_s cn31xx;
	struct cvmx_fpa_que_exp_s cn38xx;
	struct cvmx_fpa_que_exp_s cn38xxp2;
	struct cvmx_fpa_que_exp_s cn50xx;
	struct cvmx_fpa_que_exp_s cn52xx;
	struct cvmx_fpa_que_exp_s cn52xxp1;
	struct cvmx_fpa_que_exp_s cn56xx;
	struct cvmx_fpa_que_exp_s cn56xxp1;
	struct cvmx_fpa_que_exp_s cn58xx;
	struct cvmx_fpa_que_exp_s cn58xxp1;
	struct cvmx_fpa_que_exp_s cn61xx;
	struct cvmx_fpa_que_exp_s cn63xx;
	struct cvmx_fpa_que_exp_s cn63xxp1;
	struct cvmx_fpa_que_exp_s cn66xx;
	struct cvmx_fpa_que_exp_s cn68xx;
	struct cvmx_fpa_que_exp_s cn68xxp1;
	struct cvmx_fpa_que_exp_s cn70xx;
	struct cvmx_fpa_que_exp_s cn70xxp1;
	struct cvmx_fpa_que_exp_s cnf71xx;
};

typedef union cvmx_fpa_que_exp cvmx_fpa_que_exp_t;

/**
 * cvmx_fpa_rd_latency_pc
 */
union cvmx_fpa_rd_latency_pc {
	u64 u64;
	struct cvmx_fpa_rd_latency_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_fpa_rd_latency_pc_s cn73xx;
	struct cvmx_fpa_rd_latency_pc_s cn78xx;
	struct cvmx_fpa_rd_latency_pc_s cn78xxp1;
	struct cvmx_fpa_rd_latency_pc_s cnf75xx;
};

typedef union cvmx_fpa_rd_latency_pc cvmx_fpa_rd_latency_pc_t;

/**
 * cvmx_fpa_rd_req_pc
 */
union cvmx_fpa_rd_req_pc {
	u64 u64;
	struct cvmx_fpa_rd_req_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_fpa_rd_req_pc_s cn73xx;
	struct cvmx_fpa_rd_req_pc_s cn78xx;
	struct cvmx_fpa_rd_req_pc_s cn78xxp1;
	struct cvmx_fpa_rd_req_pc_s cnf75xx;
};

typedef union cvmx_fpa_rd_req_pc cvmx_fpa_rd_req_pc_t;

/**
 * cvmx_fpa_red_delay
 */
union cvmx_fpa_red_delay {
	u64 u64;
	struct cvmx_fpa_red_delay_s {
		u64 reserved_14_63 : 50;
		u64 avg_dly : 14;
	} s;
	struct cvmx_fpa_red_delay_s cn73xx;
	struct cvmx_fpa_red_delay_s cn78xx;
	struct cvmx_fpa_red_delay_s cn78xxp1;
	struct cvmx_fpa_red_delay_s cnf75xx;
};

typedef union cvmx_fpa_red_delay cvmx_fpa_red_delay_t;

/**
 * cvmx_fpa_sft_rst
 *
 * Allows soft reset.
 *
 */
union cvmx_fpa_sft_rst {
	u64 u64;
	struct cvmx_fpa_sft_rst_s {
		u64 busy : 1;
		u64 reserved_1_62 : 62;
		u64 rst : 1;
	} s;
	struct cvmx_fpa_sft_rst_s cn73xx;
	struct cvmx_fpa_sft_rst_s cn78xx;
	struct cvmx_fpa_sft_rst_s cn78xxp1;
	struct cvmx_fpa_sft_rst_s cnf75xx;
};

typedef union cvmx_fpa_sft_rst cvmx_fpa_sft_rst_t;

/**
 * cvmx_fpa_wart_ctl
 *
 * FPA_WART_CTL = FPA's WART Control
 *
 * Control and status for the WART block.
 */
union cvmx_fpa_wart_ctl {
	u64 u64;
	struct cvmx_fpa_wart_ctl_s {
		u64 reserved_16_63 : 48;
		u64 ctl : 16;
	} s;
	struct cvmx_fpa_wart_ctl_s cn30xx;
	struct cvmx_fpa_wart_ctl_s cn31xx;
	struct cvmx_fpa_wart_ctl_s cn38xx;
	struct cvmx_fpa_wart_ctl_s cn38xxp2;
	struct cvmx_fpa_wart_ctl_s cn50xx;
	struct cvmx_fpa_wart_ctl_s cn52xx;
	struct cvmx_fpa_wart_ctl_s cn52xxp1;
	struct cvmx_fpa_wart_ctl_s cn56xx;
	struct cvmx_fpa_wart_ctl_s cn56xxp1;
	struct cvmx_fpa_wart_ctl_s cn58xx;
	struct cvmx_fpa_wart_ctl_s cn58xxp1;
};

typedef union cvmx_fpa_wart_ctl cvmx_fpa_wart_ctl_t;

/**
 * cvmx_fpa_wart_status
 *
 * FPA_WART_STATUS = FPA's WART Status
 *
 * Control and status for the WART block.
 */
union cvmx_fpa_wart_status {
	u64 u64;
	struct cvmx_fpa_wart_status_s {
		u64 reserved_32_63 : 32;
		u64 status : 32;
	} s;
	struct cvmx_fpa_wart_status_s cn30xx;
	struct cvmx_fpa_wart_status_s cn31xx;
	struct cvmx_fpa_wart_status_s cn38xx;
	struct cvmx_fpa_wart_status_s cn38xxp2;
	struct cvmx_fpa_wart_status_s cn50xx;
	struct cvmx_fpa_wart_status_s cn52xx;
	struct cvmx_fpa_wart_status_s cn52xxp1;
	struct cvmx_fpa_wart_status_s cn56xx;
	struct cvmx_fpa_wart_status_s cn56xxp1;
	struct cvmx_fpa_wart_status_s cn58xx;
	struct cvmx_fpa_wart_status_s cn58xxp1;
};

typedef union cvmx_fpa_wart_status cvmx_fpa_wart_status_t;

/**
 * cvmx_fpa_wqe_threshold
 *
 * "When the value of FPA_QUE#_AVAILABLE[QUE_SIZ] (\# is determined by the value of
 * IPD_WQE_FPA_QUEUE) is Less than the value of this
 * register a low pool count signal is sent to the PCIe packet instruction engine (to make it
 * stop reading instructions) and to the
 * Packet-Arbiter informing it to not give grants to packets MAC with the exception of the PCIe
 * MAC."
 */
union cvmx_fpa_wqe_threshold {
	u64 u64;
	struct cvmx_fpa_wqe_threshold_s {
		u64 reserved_32_63 : 32;
		u64 thresh : 32;
	} s;
	struct cvmx_fpa_wqe_threshold_s cn61xx;
	struct cvmx_fpa_wqe_threshold_s cn63xx;
	struct cvmx_fpa_wqe_threshold_s cn66xx;
	struct cvmx_fpa_wqe_threshold_s cn68xx;
	struct cvmx_fpa_wqe_threshold_s cn68xxp1;
	struct cvmx_fpa_wqe_threshold_s cn70xx;
	struct cvmx_fpa_wqe_threshold_s cn70xxp1;
	struct cvmx_fpa_wqe_threshold_s cnf71xx;
};

typedef union cvmx_fpa_wqe_threshold cvmx_fpa_wqe_threshold_t;

#endif
