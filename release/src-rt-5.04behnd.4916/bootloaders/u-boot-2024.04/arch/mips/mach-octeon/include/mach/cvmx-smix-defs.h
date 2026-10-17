/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon smix.
 */

#ifndef __CVMX_SMIX_DEFS_H__
#define __CVMX_SMIX_DEFS_H__

static inline u64 CVMX_SMIX_CLK(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180000001818ull + (offset) * 256;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001180000003818ull + (offset) * 128;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001180000003818ull + (offset) * 128;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003818ull + (offset) * 128;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003818ull + (offset) * 128;
	}
	return 0x0001180000003818ull + (offset) * 128;
}

static inline u64 CVMX_SMIX_CMD(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180000001800ull + (offset) * 256;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001180000003800ull + (offset) * 128;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001180000003800ull + (offset) * 128;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003800ull + (offset) * 128;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003800ull + (offset) * 128;
	}
	return 0x0001180000003800ull + (offset) * 128;
}

static inline u64 CVMX_SMIX_EN(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180000001820ull + (offset) * 256;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001180000003820ull + (offset) * 128;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001180000003820ull + (offset) * 128;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003820ull + (offset) * 128;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003820ull + (offset) * 128;
	}
	return 0x0001180000003820ull + (offset) * 128;
}

static inline u64 CVMX_SMIX_RD_DAT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180000001810ull + (offset) * 256;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001180000003810ull + (offset) * 128;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001180000003810ull + (offset) * 128;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003810ull + (offset) * 128;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003810ull + (offset) * 128;
	}
	return 0x0001180000003810ull + (offset) * 128;
}

static inline u64 CVMX_SMIX_WR_DAT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001180000001808ull + (offset) * 256;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001180000003808ull + (offset) * 128;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001180000003808ull + (offset) * 128;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003808ull + (offset) * 128;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001180000003808ull + (offset) * 128;
	}
	return 0x0001180000003808ull + (offset) * 128;
}

/**
 * cvmx_smi#_clk
 *
 * This register determines the SMI timing characteristics.
 * If software wants to change SMI CLK timing parameters ([SAMPLE]/[SAMPLE_HI]), software
 * must delay the SMI_()_CLK CSR write by at least 512 coprocessor-clock cycles after the
 * previous SMI operation is finished.
 */
union cvmx_smix_clk {
	u64 u64;
	struct cvmx_smix_clk_s {
		u64 reserved_25_63 : 39;
		u64 mode : 1;
		u64 reserved_21_23 : 3;
		u64 sample_hi : 5;
		u64 sample_mode : 1;
		u64 reserved_14_14 : 1;
		u64 clk_idle : 1;
		u64 preamble : 1;
		u64 sample : 4;
		u64 phase : 8;
	} s;
	struct cvmx_smix_clk_cn30xx {
		u64 reserved_21_63 : 43;
		u64 sample_hi : 5;
		u64 sample_mode : 1;
		u64 reserved_14_14 : 1;
		u64 clk_idle : 1;
		u64 preamble : 1;
		u64 sample : 4;
		u64 phase : 8;
	} cn30xx;
	struct cvmx_smix_clk_cn30xx cn31xx;
	struct cvmx_smix_clk_cn30xx cn38xx;
	struct cvmx_smix_clk_cn30xx cn38xxp2;
	struct cvmx_smix_clk_s cn50xx;
	struct cvmx_smix_clk_s cn52xx;
	struct cvmx_smix_clk_s cn52xxp1;
	struct cvmx_smix_clk_s cn56xx;
	struct cvmx_smix_clk_s cn56xxp1;
	struct cvmx_smix_clk_cn30xx cn58xx;
	struct cvmx_smix_clk_cn30xx cn58xxp1;
	struct cvmx_smix_clk_s cn61xx;
	struct cvmx_smix_clk_s cn63xx;
	struct cvmx_smix_clk_s cn63xxp1;
	struct cvmx_smix_clk_s cn66xx;
	struct cvmx_smix_clk_s cn68xx;
	struct cvmx_smix_clk_s cn68xxp1;
	struct cvmx_smix_clk_s cn70xx;
	struct cvmx_smix_clk_s cn70xxp1;
	struct cvmx_smix_clk_s cn73xx;
	struct cvmx_smix_clk_s cn78xx;
	struct cvmx_smix_clk_s cn78xxp1;
	struct cvmx_smix_clk_s cnf71xx;
	struct cvmx_smix_clk_s cnf75xx;
};

typedef union cvmx_smix_clk cvmx_smix_clk_t;

/**
 * cvmx_smi#_cmd
 *
 * This register forces a read or write command to the PHY. Write operations to this register
 * create SMI transactions. Software will poll (depending on the transaction type).
 */
union cvmx_smix_cmd {
	u64 u64;
	struct cvmx_smix_cmd_s {
		u64 reserved_18_63 : 46;
		u64 phy_op : 2;
		u64 reserved_13_15 : 3;
		u64 phy_adr : 5;
		u64 reserved_5_7 : 3;
		u64 reg_adr : 5;
	} s;
	struct cvmx_smix_cmd_cn30xx {
		u64 reserved_17_63 : 47;
		u64 phy_op : 1;
		u64 reserved_13_15 : 3;
		u64 phy_adr : 5;
		u64 reserved_5_7 : 3;
		u64 reg_adr : 5;
	} cn30xx;
	struct cvmx_smix_cmd_cn30xx cn31xx;
	struct cvmx_smix_cmd_cn30xx cn38xx;
	struct cvmx_smix_cmd_cn30xx cn38xxp2;
	struct cvmx_smix_cmd_s cn50xx;
	struct cvmx_smix_cmd_s cn52xx;
	struct cvmx_smix_cmd_s cn52xxp1;
	struct cvmx_smix_cmd_s cn56xx;
	struct cvmx_smix_cmd_s cn56xxp1;
	struct cvmx_smix_cmd_cn30xx cn58xx;
	struct cvmx_smix_cmd_cn30xx cn58xxp1;
	struct cvmx_smix_cmd_s cn61xx;
	struct cvmx_smix_cmd_s cn63xx;
	struct cvmx_smix_cmd_s cn63xxp1;
	struct cvmx_smix_cmd_s cn66xx;
	struct cvmx_smix_cmd_s cn68xx;
	struct cvmx_smix_cmd_s cn68xxp1;
	struct cvmx_smix_cmd_s cn70xx;
	struct cvmx_smix_cmd_s cn70xxp1;
	struct cvmx_smix_cmd_s cn73xx;
	struct cvmx_smix_cmd_s cn78xx;
	struct cvmx_smix_cmd_s cn78xxp1;
	struct cvmx_smix_cmd_s cnf71xx;
	struct cvmx_smix_cmd_s cnf75xx;
};

typedef union cvmx_smix_cmd cvmx_smix_cmd_t;

/**
 * cvmx_smi#_en
 *
 * Enables the SMI interface.
 *
 */
union cvmx_smix_en {
	u64 u64;
	struct cvmx_smix_en_s {
		u64 reserved_1_63 : 63;
		u64 en : 1;
	} s;
	struct cvmx_smix_en_s cn30xx;
	struct cvmx_smix_en_s cn31xx;
	struct cvmx_smix_en_s cn38xx;
	struct cvmx_smix_en_s cn38xxp2;
	struct cvmx_smix_en_s cn50xx;
	struct cvmx_smix_en_s cn52xx;
	struct cvmx_smix_en_s cn52xxp1;
	struct cvmx_smix_en_s cn56xx;
	struct cvmx_smix_en_s cn56xxp1;
	struct cvmx_smix_en_s cn58xx;
	struct cvmx_smix_en_s cn58xxp1;
	struct cvmx_smix_en_s cn61xx;
	struct cvmx_smix_en_s cn63xx;
	struct cvmx_smix_en_s cn63xxp1;
	struct cvmx_smix_en_s cn66xx;
	struct cvmx_smix_en_s cn68xx;
	struct cvmx_smix_en_s cn68xxp1;
	struct cvmx_smix_en_s cn70xx;
	struct cvmx_smix_en_s cn70xxp1;
	struct cvmx_smix_en_s cn73xx;
	struct cvmx_smix_en_s cn78xx;
	struct cvmx_smix_en_s cn78xxp1;
	struct cvmx_smix_en_s cnf71xx;
	struct cvmx_smix_en_s cnf75xx;
};

typedef union cvmx_smix_en cvmx_smix_en_t;

/**
 * cvmx_smi#_rd_dat
 *
 * This register contains the data in a read operation.
 *
 */
union cvmx_smix_rd_dat {
	u64 u64;
	struct cvmx_smix_rd_dat_s {
		u64 reserved_18_63 : 46;
		u64 pending : 1;
		u64 val : 1;
		u64 dat : 16;
	} s;
	struct cvmx_smix_rd_dat_s cn30xx;
	struct cvmx_smix_rd_dat_s cn31xx;
	struct cvmx_smix_rd_dat_s cn38xx;
	struct cvmx_smix_rd_dat_s cn38xxp2;
	struct cvmx_smix_rd_dat_s cn50xx;
	struct cvmx_smix_rd_dat_s cn52xx;
	struct cvmx_smix_rd_dat_s cn52xxp1;
	struct cvmx_smix_rd_dat_s cn56xx;
	struct cvmx_smix_rd_dat_s cn56xxp1;
	struct cvmx_smix_rd_dat_s cn58xx;
	struct cvmx_smix_rd_dat_s cn58xxp1;
	struct cvmx_smix_rd_dat_s cn61xx;
	struct cvmx_smix_rd_dat_s cn63xx;
	struct cvmx_smix_rd_dat_s cn63xxp1;
	struct cvmx_smix_rd_dat_s cn66xx;
	struct cvmx_smix_rd_dat_s cn68xx;
	struct cvmx_smix_rd_dat_s cn68xxp1;
	struct cvmx_smix_rd_dat_s cn70xx;
	struct cvmx_smix_rd_dat_s cn70xxp1;
	struct cvmx_smix_rd_dat_s cn73xx;
	struct cvmx_smix_rd_dat_s cn78xx;
	struct cvmx_smix_rd_dat_s cn78xxp1;
	struct cvmx_smix_rd_dat_s cnf71xx;
	struct cvmx_smix_rd_dat_s cnf75xx;
};

typedef union cvmx_smix_rd_dat cvmx_smix_rd_dat_t;

/**
 * cvmx_smi#_wr_dat
 *
 * This register provides the data for a write operation.
 *
 */
union cvmx_smix_wr_dat {
	u64 u64;
	struct cvmx_smix_wr_dat_s {
		u64 reserved_18_63 : 46;
		u64 pending : 1;
		u64 val : 1;
		u64 dat : 16;
	} s;
	struct cvmx_smix_wr_dat_s cn30xx;
	struct cvmx_smix_wr_dat_s cn31xx;
	struct cvmx_smix_wr_dat_s cn38xx;
	struct cvmx_smix_wr_dat_s cn38xxp2;
	struct cvmx_smix_wr_dat_s cn50xx;
	struct cvmx_smix_wr_dat_s cn52xx;
	struct cvmx_smix_wr_dat_s cn52xxp1;
	struct cvmx_smix_wr_dat_s cn56xx;
	struct cvmx_smix_wr_dat_s cn56xxp1;
	struct cvmx_smix_wr_dat_s cn58xx;
	struct cvmx_smix_wr_dat_s cn58xxp1;
	struct cvmx_smix_wr_dat_s cn61xx;
	struct cvmx_smix_wr_dat_s cn63xx;
	struct cvmx_smix_wr_dat_s cn63xxp1;
	struct cvmx_smix_wr_dat_s cn66xx;
	struct cvmx_smix_wr_dat_s cn68xx;
	struct cvmx_smix_wr_dat_s cn68xxp1;
	struct cvmx_smix_wr_dat_s cn70xx;
	struct cvmx_smix_wr_dat_s cn70xxp1;
	struct cvmx_smix_wr_dat_s cn73xx;
	struct cvmx_smix_wr_dat_s cn78xx;
	struct cvmx_smix_wr_dat_s cn78xxp1;
	struct cvmx_smix_wr_dat_s cnf71xx;
	struct cvmx_smix_wr_dat_s cnf75xx;
};

typedef union cvmx_smix_wr_dat cvmx_smix_wr_dat_t;

#endif
