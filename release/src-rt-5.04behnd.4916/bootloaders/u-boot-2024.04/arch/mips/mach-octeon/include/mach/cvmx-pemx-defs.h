/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pemx.
 */

#ifndef __CVMX_PEMX_DEFS_H__
#define __CVMX_PEMX_DEFS_H__

static inline u64 CVMX_PEMX_BAR1_INDEXX(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000A8ull + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return 0x00011800C0000100ull + ((offset) + (block_id) * 0x200000ull) * 8;
}

static inline u64 CVMX_PEMX_BAR2_MASK(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000B0ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000B0ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C00000B0ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C00000B0ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C10000B0ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000130ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C00000B0ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_BAR_CTL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000A8ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000A8ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C00000A8ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C00000A8ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C10000A8ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000128ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C00000A8ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_BIST_STATUS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000018ull + (offset) * 0x1000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000018ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000440ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C0000440ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C0000440ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C1000440ull + (offset) * 0x1000000ull - 16777216 * 1;
	}
	return 0x00011800C0000440ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_BIST_STATUS2(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000420ull + (offset) * 0x1000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000440ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000420ull + (offset) * 0x1000000ull;
}

#define CVMX_PEMX_CFG(offset)		(0x00011800C0000410ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CFG_RD(offset)	(0x00011800C0000030ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CFG_WR(offset)	(0x00011800C0000028ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CLK_EN(offset)	(0x00011800C0000400ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CPL_LUT_VALID(offset) (0x00011800C0000098ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CTL_STATUS(offset)	(0x00011800C0000000ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_CTL_STATUS2(offset)	(0x00011800C0000008ull + ((offset) & 3) * 0x1000000ull)
static inline u64 CVMX_PEMX_DBG_INFO(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000D0ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000D0ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C00000D0ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C00000D0ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C10000D0ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000008ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C00000D0ull + (offset) * 0x1000000ull;
}

#define CVMX_PEMX_DBG_INFO_EN(offset) (0x00011800C00000A0ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_DIAG_STATUS(offset) (0x00011800C0000020ull + ((offset) & 3) * 0x1000000ull)
static inline u64 CVMX_PEMX_ECC_ENA(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000448ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C0000448ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C0000448ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C1000448ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000C0ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000448ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_ECC_SYND_CTRL(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000450ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C0000450ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C0000450ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C1000450ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000C8ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000450ull + (offset) * 0x1000000ull;
}

#define CVMX_PEMX_ECO(offset)		     (0x00011800C0000010ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_FLR_GLBLCNT_CTL(offset)    (0x00011800C0000210ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_FLR_PF0_VF_STOPREQ(offset) (0x00011800C0000220ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_FLR_PF_STOPREQ(offset)     (0x00011800C0000218ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_FLR_STOPREQ_CTL(offset)    (0x00011800C0000238ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_FLR_ZOMBIE_CTL(offset)     (0x00011800C0000230ull + ((offset) & 3) * 0x1000000ull)
static inline u64 CVMX_PEMX_INB_READ_CREDITS(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000B8ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C00000B8ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C00000B8ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C00000B8ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C10000B8ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000138ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C00000B8ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_INT_ENB(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000410ull + (offset) * 0x1000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000430ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000410ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_INT_ENB_INT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000418ull + (offset) * 0x1000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000438ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000418ull + (offset) * 0x1000000ull;
}

static inline u64 CVMX_PEMX_INT_SUM(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000428ull + (offset) * 0x1000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000428ull + (offset) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00011800C0000428ull + (offset) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00011800C0000428ull + (offset) * 0x1000000ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00011800C1000428ull + (offset) * 0x1000000ull - 16777216 * 1;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800C0000408ull + (offset) * 0x1000000ull;
	}
	return 0x00011800C0000428ull + (offset) * 0x1000000ull;
}

#define CVMX_PEMX_ON(offset)		 (0x00011800C0000420ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_P2N_BAR0_START(offset) (0x00011800C0000080ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_P2N_BAR1_START(offset) (0x00011800C0000088ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_P2N_BAR2_START(offset) (0x00011800C0000090ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_P2P_BARX_END(offset, block_id)                                                   \
	(0x00011800C0000048ull + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16)
#define CVMX_PEMX_P2P_BARX_START(offset, block_id)                                                 \
	(0x00011800C0000040ull + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16)
#define CVMX_PEMX_QLM(offset)	      (0x00011800C0000418ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_SPI_CTL(offset)     (0x00011800C0000180ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_SPI_DATA(offset)    (0x00011800C0000188ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_STRAP(offset)	      (0x00011800C0000408ull + ((offset) & 3) * 0x1000000ull)
#define CVMX_PEMX_TLP_CREDITS(offset) (0x00011800C0000038ull + ((offset) & 3) * 0x1000000ull)

/**
 * cvmx_pem#_bar1_index#
 *
 * This register contains the address index and control bits for access to memory ranges of BAR1.
 * The index is built from supplied address [25:22].
 */
union cvmx_pemx_bar1_indexx {
	u64 u64;
	struct cvmx_pemx_bar1_indexx_s {
		u64 reserved_24_63 : 40;
		u64 addr_idx : 20;
		u64 ca : 1;
		u64 end_swp : 2;
		u64 addr_v : 1;
	} s;
	struct cvmx_pemx_bar1_indexx_cn61xx {
		u64 reserved_20_63 : 44;
		u64 addr_idx : 16;
		u64 ca : 1;
		u64 end_swp : 2;
		u64 addr_v : 1;
	} cn61xx;
	struct cvmx_pemx_bar1_indexx_cn61xx cn63xx;
	struct cvmx_pemx_bar1_indexx_cn61xx cn63xxp1;
	struct cvmx_pemx_bar1_indexx_cn61xx cn66xx;
	struct cvmx_pemx_bar1_indexx_cn61xx cn68xx;
	struct cvmx_pemx_bar1_indexx_cn61xx cn68xxp1;
	struct cvmx_pemx_bar1_indexx_s cn70xx;
	struct cvmx_pemx_bar1_indexx_s cn70xxp1;
	struct cvmx_pemx_bar1_indexx_s cn73xx;
	struct cvmx_pemx_bar1_indexx_s cn78xx;
	struct cvmx_pemx_bar1_indexx_s cn78xxp1;
	struct cvmx_pemx_bar1_indexx_cn61xx cnf71xx;
	struct cvmx_pemx_bar1_indexx_s cnf75xx;
};

typedef union cvmx_pemx_bar1_indexx cvmx_pemx_bar1_indexx_t;

/**
 * cvmx_pem#_bar2_mask
 *
 * This register contains the mask pattern that is ANDed with the address from the PCIe core for
 * BAR2 hits. This allows the effective size of RC BAR2 to be shrunk. Must not be changed
 * from its reset value in EP mode.
 */
union cvmx_pemx_bar2_mask {
	u64 u64;
	struct cvmx_pemx_bar2_mask_s {
		u64 reserved_45_63 : 19;
		u64 mask : 42;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_pemx_bar2_mask_cn61xx {
		u64 reserved_38_63 : 26;
		u64 mask : 35;
		u64 reserved_0_2 : 3;
	} cn61xx;
	struct cvmx_pemx_bar2_mask_cn61xx cn66xx;
	struct cvmx_pemx_bar2_mask_cn61xx cn68xx;
	struct cvmx_pemx_bar2_mask_cn61xx cn68xxp1;
	struct cvmx_pemx_bar2_mask_cn61xx cn70xx;
	struct cvmx_pemx_bar2_mask_cn61xx cn70xxp1;
	struct cvmx_pemx_bar2_mask_cn73xx {
		u64 reserved_42_63 : 22;
		u64 mask : 39;
		u64 reserved_0_2 : 3;
	} cn73xx;
	struct cvmx_pemx_bar2_mask_s cn78xx;
	struct cvmx_pemx_bar2_mask_cn73xx cn78xxp1;
	struct cvmx_pemx_bar2_mask_cn61xx cnf71xx;
	struct cvmx_pemx_bar2_mask_cn73xx cnf75xx;
};

typedef union cvmx_pemx_bar2_mask cvmx_pemx_bar2_mask_t;

/**
 * cvmx_pem#_bar_ctl
 *
 * This register contains control for BAR accesses.
 *
 */
union cvmx_pemx_bar_ctl {
	u64 u64;
	struct cvmx_pemx_bar_ctl_s {
		u64 reserved_7_63 : 57;
		u64 bar1_siz : 3;
		u64 bar2_enb : 1;
		u64 bar2_esx : 2;
		u64 bar2_cax : 1;
	} s;
	struct cvmx_pemx_bar_ctl_s cn61xx;
	struct cvmx_pemx_bar_ctl_s cn63xx;
	struct cvmx_pemx_bar_ctl_s cn63xxp1;
	struct cvmx_pemx_bar_ctl_s cn66xx;
	struct cvmx_pemx_bar_ctl_s cn68xx;
	struct cvmx_pemx_bar_ctl_s cn68xxp1;
	struct cvmx_pemx_bar_ctl_s cn70xx;
	struct cvmx_pemx_bar_ctl_s cn70xxp1;
	struct cvmx_pemx_bar_ctl_s cn73xx;
	struct cvmx_pemx_bar_ctl_s cn78xx;
	struct cvmx_pemx_bar_ctl_s cn78xxp1;
	struct cvmx_pemx_bar_ctl_s cnf71xx;
	struct cvmx_pemx_bar_ctl_s cnf75xx;
};

typedef union cvmx_pemx_bar_ctl cvmx_pemx_bar_ctl_t;

/**
 * cvmx_pem#_bist_status
 *
 * This register contains results from BIST runs of PEM's memories.
 *
 */
union cvmx_pemx_bist_status {
	u64 u64;
	struct cvmx_pemx_bist_status_s {
		u64 reserved_16_63 : 48;
		u64 retryc : 1;
		u64 reserved_14_14 : 1;
		u64 rqhdrb0 : 1;
		u64 rqhdrb1 : 1;
		u64 rqdatab0 : 1;
		u64 rqdatab1 : 1;
		u64 tlpn_d0 : 1;
		u64 tlpn_d1 : 1;
		u64 reserved_0_7 : 8;
	} s;
	struct cvmx_pemx_bist_status_cn61xx {
		u64 reserved_8_63 : 56;
		u64 retry : 1;
		u64 rqdata0 : 1;
		u64 rqdata1 : 1;
		u64 rqdata2 : 1;
		u64 rqdata3 : 1;
		u64 rqhdr1 : 1;
		u64 rqhdr0 : 1;
		u64 sot : 1;
	} cn61xx;
	struct cvmx_pemx_bist_status_cn61xx cn63xx;
	struct cvmx_pemx_bist_status_cn61xx cn63xxp1;
	struct cvmx_pemx_bist_status_cn61xx cn66xx;
	struct cvmx_pemx_bist_status_cn61xx cn68xx;
	struct cvmx_pemx_bist_status_cn61xx cn68xxp1;
	struct cvmx_pemx_bist_status_cn70xx {
		u64 reserved_6_63 : 58;
		u64 retry : 1;
		u64 sot : 1;
		u64 rqhdr0 : 1;
		u64 rqhdr1 : 1;
		u64 rqdata0 : 1;
		u64 rqdata1 : 1;
	} cn70xx;
	struct cvmx_pemx_bist_status_cn70xx cn70xxp1;
	struct cvmx_pemx_bist_status_cn73xx {
		u64 reserved_16_63 : 48;
		u64 retryc : 1;
		u64 sot : 1;
		u64 rqhdrb0 : 1;
		u64 rqhdrb1 : 1;
		u64 rqdatab0 : 1;
		u64 rqdatab1 : 1;
		u64 tlpn_d0 : 1;
		u64 tlpn_d1 : 1;
		u64 tlpn_ctl : 1;
		u64 tlpp_d0 : 1;
		u64 tlpp_d1 : 1;
		u64 tlpp_ctl : 1;
		u64 tlpc_d0 : 1;
		u64 tlpc_d1 : 1;
		u64 tlpc_ctl : 1;
		u64 m2s : 1;
	} cn73xx;
	struct cvmx_pemx_bist_status_cn73xx cn78xx;
	struct cvmx_pemx_bist_status_cn73xx cn78xxp1;
	struct cvmx_pemx_bist_status_cn61xx cnf71xx;
	struct cvmx_pemx_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pemx_bist_status cvmx_pemx_bist_status_t;

/**
 * cvmx_pem#_bist_status2
 *
 * "PEM#_BIST_STATUS2 = PEM BIST Status Register
 * Results from BIST runs of PEM's memories."
 */
union cvmx_pemx_bist_status2 {
	u64 u64;
	struct cvmx_pemx_bist_status2_s {
		u64 reserved_13_63 : 51;
		u64 tlpn_d : 1;
		u64 tlpn_ctl : 1;
		u64 tlpp_d : 1;
		u64 reserved_0_9 : 10;
	} s;
	struct cvmx_pemx_bist_status2_cn61xx {
		u64 reserved_10_63 : 54;
		u64 e2p_cpl : 1;
		u64 e2p_n : 1;
		u64 e2p_p : 1;
		u64 peai_p2e : 1;
		u64 pef_tpf1 : 1;
		u64 pef_tpf0 : 1;
		u64 pef_tnf : 1;
		u64 pef_tcf1 : 1;
		u64 pef_tc0 : 1;
		u64 ppf : 1;
	} cn61xx;
	struct cvmx_pemx_bist_status2_cn61xx cn63xx;
	struct cvmx_pemx_bist_status2_cn61xx cn63xxp1;
	struct cvmx_pemx_bist_status2_cn61xx cn66xx;
	struct cvmx_pemx_bist_status2_cn61xx cn68xx;
	struct cvmx_pemx_bist_status2_cn61xx cn68xxp1;
	struct cvmx_pemx_bist_status2_cn70xx {
		u64 reserved_14_63 : 50;
		u64 peai_p2e : 1;
		u64 tlpn_d : 1;
		u64 tlpn_ctl : 1;
		u64 tlpp_d : 1;
		u64 tlpp_ctl : 1;
		u64 tlpc_d : 1;
		u64 tlpc_ctl : 1;
		u64 tlpan_d : 1;
		u64 tlpan_ctl : 1;
		u64 tlpap_d : 1;
		u64 tlpap_ctl : 1;
		u64 tlpac_d : 1;
		u64 tlpac_ctl : 1;
		u64 m2s : 1;
	} cn70xx;
	struct cvmx_pemx_bist_status2_cn70xx cn70xxp1;
	struct cvmx_pemx_bist_status2_cn61xx cnf71xx;
};

typedef union cvmx_pemx_bist_status2 cvmx_pemx_bist_status2_t;

/**
 * cvmx_pem#_cfg
 *
 * Configuration of the PCIe Application.
 *
 */
union cvmx_pemx_cfg {
	u64 u64;
	struct cvmx_pemx_cfg_s {
		u64 reserved_5_63 : 59;
		u64 laneswap : 1;
		u64 reserved_2_3 : 2;
		u64 md : 2;
	} s;
	struct cvmx_pemx_cfg_cn70xx {
		u64 reserved_5_63 : 59;
		u64 laneswap : 1;
		u64 hostmd : 1;
		u64 md : 3;
	} cn70xx;
	struct cvmx_pemx_cfg_cn70xx cn70xxp1;
	struct cvmx_pemx_cfg_cn73xx {
		u64 reserved_5_63 : 59;
		u64 laneswap : 1;
		u64 lanes8 : 1;
		u64 hostmd : 1;
		u64 md : 2;
	} cn73xx;
	struct cvmx_pemx_cfg_cn73xx cn78xx;
	struct cvmx_pemx_cfg_cn73xx cn78xxp1;
	struct cvmx_pemx_cfg_cn73xx cnf75xx;
};

typedef union cvmx_pemx_cfg cvmx_pemx_cfg_t;

/**
 * cvmx_pem#_cfg_rd
 *
 * This register allows read access to the configuration in the PCIe core.
 *
 */
union cvmx_pemx_cfg_rd {
	u64 u64;
	struct cvmx_pemx_cfg_rd_s {
		u64 data : 32;
		u64 addr : 32;
	} s;
	struct cvmx_pemx_cfg_rd_s cn61xx;
	struct cvmx_pemx_cfg_rd_s cn63xx;
	struct cvmx_pemx_cfg_rd_s cn63xxp1;
	struct cvmx_pemx_cfg_rd_s cn66xx;
	struct cvmx_pemx_cfg_rd_s cn68xx;
	struct cvmx_pemx_cfg_rd_s cn68xxp1;
	struct cvmx_pemx_cfg_rd_s cn70xx;
	struct cvmx_pemx_cfg_rd_s cn70xxp1;
	struct cvmx_pemx_cfg_rd_s cn73xx;
	struct cvmx_pemx_cfg_rd_s cn78xx;
	struct cvmx_pemx_cfg_rd_s cn78xxp1;
	struct cvmx_pemx_cfg_rd_s cnf71xx;
	struct cvmx_pemx_cfg_rd_s cnf75xx;
};

typedef union cvmx_pemx_cfg_rd cvmx_pemx_cfg_rd_t;

/**
 * cvmx_pem#_cfg_wr
 *
 * This register allows write access to the configuration in the PCIe core.
 *
 */
union cvmx_pemx_cfg_wr {
	u64 u64;
	struct cvmx_pemx_cfg_wr_s {
		u64 data : 32;
		u64 addr : 32;
	} s;
	struct cvmx_pemx_cfg_wr_s cn61xx;
	struct cvmx_pemx_cfg_wr_s cn63xx;
	struct cvmx_pemx_cfg_wr_s cn63xxp1;
	struct cvmx_pemx_cfg_wr_s cn66xx;
	struct cvmx_pemx_cfg_wr_s cn68xx;
	struct cvmx_pemx_cfg_wr_s cn68xxp1;
	struct cvmx_pemx_cfg_wr_s cn70xx;
	struct cvmx_pemx_cfg_wr_s cn70xxp1;
	struct cvmx_pemx_cfg_wr_s cn73xx;
	struct cvmx_pemx_cfg_wr_s cn78xx;
	struct cvmx_pemx_cfg_wr_s cn78xxp1;
	struct cvmx_pemx_cfg_wr_s cnf71xx;
	struct cvmx_pemx_cfg_wr_s cnf75xx;
};

typedef union cvmx_pemx_cfg_wr cvmx_pemx_cfg_wr_t;

/**
 * cvmx_pem#_clk_en
 *
 * This register contains the clock enable for ECLK and PCE_CLK.
 *
 */
union cvmx_pemx_clk_en {
	u64 u64;
	struct cvmx_pemx_clk_en_s {
		u64 reserved_2_63 : 62;
		u64 pceclk_gate : 1;
		u64 csclk_gate : 1;
	} s;
	struct cvmx_pemx_clk_en_s cn70xx;
	struct cvmx_pemx_clk_en_s cn70xxp1;
	struct cvmx_pemx_clk_en_s cn73xx;
	struct cvmx_pemx_clk_en_s cn78xx;
	struct cvmx_pemx_clk_en_s cn78xxp1;
	struct cvmx_pemx_clk_en_s cnf75xx;
};

typedef union cvmx_pemx_clk_en cvmx_pemx_clk_en_t;

/**
 * cvmx_pem#_cpl_lut_valid
 *
 * This register specifies the bit set for an outstanding tag read.
 *
 */
union cvmx_pemx_cpl_lut_valid {
	u64 u64;
	struct cvmx_pemx_cpl_lut_valid_s {
		u64 tag : 64;
	} s;
	struct cvmx_pemx_cpl_lut_valid_cn61xx {
		u64 reserved_32_63 : 32;
		u64 tag : 32;
	} cn61xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn63xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn63xxp1;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn66xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn68xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn68xxp1;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn70xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn70xxp1;
	struct cvmx_pemx_cpl_lut_valid_s cn73xx;
	struct cvmx_pemx_cpl_lut_valid_s cn78xx;
	struct cvmx_pemx_cpl_lut_valid_s cn78xxp1;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cnf71xx;
	struct cvmx_pemx_cpl_lut_valid_s cnf75xx;
};

typedef union cvmx_pemx_cpl_lut_valid cvmx_pemx_cpl_lut_valid_t;

/**
 * cvmx_pem#_ctl_status
 *
 * This is a general control and status register of the PEM.
 *
 */
union cvmx_pemx_ctl_status {
	u64 u64;
	struct cvmx_pemx_ctl_status_s {
		u64 reserved_51_63 : 13;
		u64 inv_dpar : 1;
		u64 inv_hpar : 1;
		u64 inv_rpar : 1;
		u64 auto_sd : 1;
		u64 dnum : 5;
		u64 pbus : 8;
		u64 reserved_32_33 : 2;
		u64 cfg_rtry : 16;
		u64 reserved_12_15 : 4;
		u64 pm_xtoff : 1;
		u64 pm_xpme : 1;
		u64 ob_p_cmd : 1;
		u64 reserved_7_8 : 2;
		u64 nf_ecrc : 1;
		u64 dly_one : 1;
		u64 lnk_enb : 1;
		u64 ro_ctlp : 1;
		u64 fast_lm : 1;
		u64 inv_ecrc : 1;
		u64 inv_lcrc : 1;
	} s;
	struct cvmx_pemx_ctl_status_cn61xx {
		u64 reserved_48_63 : 16;
		u64 auto_sd : 1;
		u64 dnum : 5;
		u64 pbus : 8;
		u64 reserved_32_33 : 2;
		u64 cfg_rtry : 16;
		u64 reserved_12_15 : 4;
		u64 pm_xtoff : 1;
		u64 pm_xpme : 1;
		u64 ob_p_cmd : 1;
		u64 reserved_7_8 : 2;
		u64 nf_ecrc : 1;
		u64 dly_one : 1;
		u64 lnk_enb : 1;
		u64 ro_ctlp : 1;
		u64 fast_lm : 1;
		u64 inv_ecrc : 1;
		u64 inv_lcrc : 1;
	} cn61xx;
	struct cvmx_pemx_ctl_status_cn61xx cn63xx;
	struct cvmx_pemx_ctl_status_cn61xx cn63xxp1;
	struct cvmx_pemx_ctl_status_cn61xx cn66xx;
	struct cvmx_pemx_ctl_status_cn61xx cn68xx;
	struct cvmx_pemx_ctl_status_cn61xx cn68xxp1;
	struct cvmx_pemx_ctl_status_s cn70xx;
	struct cvmx_pemx_ctl_status_s cn70xxp1;
	struct cvmx_pemx_ctl_status_cn73xx {
		u64 reserved_51_63 : 13;
		u64 inv_dpar : 1;
		u64 reserved_48_49 : 2;
		u64 auto_sd : 1;
		u64 dnum : 5;
		u64 pbus : 8;
		u64 reserved_32_33 : 2;
		u64 cfg_rtry : 16;
		u64 reserved_12_15 : 4;
		u64 pm_xtoff : 1;
		u64 pm_xpme : 1;
		u64 ob_p_cmd : 1;
		u64 reserved_7_8 : 2;
		u64 nf_ecrc : 1;
		u64 dly_one : 1;
		u64 lnk_enb : 1;
		u64 ro_ctlp : 1;
		u64 fast_lm : 1;
		u64 inv_ecrc : 1;
		u64 inv_lcrc : 1;
	} cn73xx;
	struct cvmx_pemx_ctl_status_cn73xx cn78xx;
	struct cvmx_pemx_ctl_status_cn73xx cn78xxp1;
	struct cvmx_pemx_ctl_status_cn61xx cnf71xx;
	struct cvmx_pemx_ctl_status_cn73xx cnf75xx;
};

typedef union cvmx_pemx_ctl_status cvmx_pemx_ctl_status_t;

/**
 * cvmx_pem#_ctl_status2
 *
 * This register contains additional general control and status of the PEM.
 *
 */
union cvmx_pemx_ctl_status2 {
	u64 u64;
	struct cvmx_pemx_ctl_status2_s {
		u64 reserved_16_63 : 48;
		u64 no_fwd_prg : 16;
	} s;
	struct cvmx_pemx_ctl_status2_s cn73xx;
	struct cvmx_pemx_ctl_status2_s cn78xx;
	struct cvmx_pemx_ctl_status2_s cn78xxp1;
	struct cvmx_pemx_ctl_status2_s cnf75xx;
};

typedef union cvmx_pemx_ctl_status2 cvmx_pemx_ctl_status2_t;

/**
 * cvmx_pem#_dbg_info
 *
 * This is a debug information register of the PEM.
 *
 */
union cvmx_pemx_dbg_info {
	u64 u64;
	struct cvmx_pemx_dbg_info_s {
		u64 reserved_62_63 : 2;
		u64 m2s_c_dbe : 1;
		u64 m2s_c_sbe : 1;
		u64 m2s_d_dbe : 1;
		u64 m2s_d_sbe : 1;
		u64 qhdr_b1_dbe : 1;
		u64 qhdr_b1_sbe : 1;
		u64 qhdr_b0_dbe : 1;
		u64 qhdr_b0_sbe : 1;
		u64 rtry_dbe : 1;
		u64 rtry_sbe : 1;
		u64 reserved_50_51 : 2;
		u64 c_d1_dbe : 1;
		u64 c_d1_sbe : 1;
		u64 c_d0_dbe : 1;
		u64 c_d0_sbe : 1;
		u64 reserved_34_45 : 12;
		u64 datq_pe : 1;
		u64 reserved_31_32 : 2;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} s;
	struct cvmx_pemx_dbg_info_cn61xx {
		u64 reserved_31_63 : 33;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} cn61xx;
	struct cvmx_pemx_dbg_info_cn61xx cn63xx;
	struct cvmx_pemx_dbg_info_cn61xx cn63xxp1;
	struct cvmx_pemx_dbg_info_cn61xx cn66xx;
	struct cvmx_pemx_dbg_info_cn61xx cn68xx;
	struct cvmx_pemx_dbg_info_cn61xx cn68xxp1;
	struct cvmx_pemx_dbg_info_cn70xx {
		u64 reserved_46_63 : 18;
		u64 c_c_dbe : 1;
		u64 c_c_sbe : 1;
		u64 c_d_dbe : 1;
		u64 c_d_sbe : 1;
		u64 n_c_dbe : 1;
		u64 n_c_sbe : 1;
		u64 n_d_dbe : 1;
		u64 n_d_sbe : 1;
		u64 p_c_dbe : 1;
		u64 p_c_sbe : 1;
		u64 p_d_dbe : 1;
		u64 p_d_sbe : 1;
		u64 datq_pe : 1;
		u64 hdrq_pe : 1;
		u64 rtry_pe : 1;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} cn70xx;
	struct cvmx_pemx_dbg_info_cn70xx cn70xxp1;
	struct cvmx_pemx_dbg_info_cn73xx {
		u64 reserved_62_63 : 2;
		u64 m2s_c_dbe : 1;
		u64 m2s_c_sbe : 1;
		u64 m2s_d_dbe : 1;
		u64 m2s_d_sbe : 1;
		u64 qhdr_b1_dbe : 1;
		u64 qhdr_b1_sbe : 1;
		u64 qhdr_b0_dbe : 1;
		u64 qhdr_b0_sbe : 1;
		u64 rtry_dbe : 1;
		u64 rtry_sbe : 1;
		u64 c_c_dbe : 1;
		u64 c_c_sbe : 1;
		u64 c_d1_dbe : 1;
		u64 c_d1_sbe : 1;
		u64 c_d0_dbe : 1;
		u64 c_d0_sbe : 1;
		u64 n_c_dbe : 1;
		u64 n_c_sbe : 1;
		u64 n_d1_dbe : 1;
		u64 n_d1_sbe : 1;
		u64 n_d0_dbe : 1;
		u64 n_d0_sbe : 1;
		u64 p_c_dbe : 1;
		u64 p_c_sbe : 1;
		u64 p_d1_dbe : 1;
		u64 p_d1_sbe : 1;
		u64 p_d0_dbe : 1;
		u64 p_d0_sbe : 1;
		u64 datq_pe : 1;
		u64 bmd_e : 1;
		u64 lofp : 1;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} cn73xx;
	struct cvmx_pemx_dbg_info_cn73xx cn78xx;
	struct cvmx_pemx_dbg_info_cn78xxp1 {
		u64 reserved_58_63 : 6;
		u64 qhdr_b1_dbe : 1;
		u64 qhdr_b1_sbe : 1;
		u64 qhdr_b0_dbe : 1;
		u64 qhdr_b0_sbe : 1;
		u64 rtry_dbe : 1;
		u64 rtry_sbe : 1;
		u64 c_c_dbe : 1;
		u64 c_c_sbe : 1;
		u64 c_d1_dbe : 1;
		u64 c_d1_sbe : 1;
		u64 c_d0_dbe : 1;
		u64 c_d0_sbe : 1;
		u64 n_c_dbe : 1;
		u64 n_c_sbe : 1;
		u64 n_d1_dbe : 1;
		u64 n_d1_sbe : 1;
		u64 n_d0_dbe : 1;
		u64 n_d0_sbe : 1;
		u64 p_c_dbe : 1;
		u64 p_c_sbe : 1;
		u64 p_d1_dbe : 1;
		u64 p_d1_sbe : 1;
		u64 p_d0_dbe : 1;
		u64 p_d0_sbe : 1;
		u64 datq_pe : 1;
		u64 reserved_32_32 : 1;
		u64 lofp : 1;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} cn78xxp1;
	struct cvmx_pemx_dbg_info_cn61xx cnf71xx;
	struct cvmx_pemx_dbg_info_cn73xx cnf75xx;
};

typedef union cvmx_pemx_dbg_info cvmx_pemx_dbg_info_t;

/**
 * cvmx_pem#_dbg_info_en
 *
 * "PEM#_DBG_INFO_EN = PEM Debug Information Enable
 * Allows PEM_DBG_INFO to generate interrupts when cooresponding enable bit is set."
 */
union cvmx_pemx_dbg_info_en {
	u64 u64;
	struct cvmx_pemx_dbg_info_en_s {
		u64 reserved_46_63 : 18;
		u64 tpcdbe1 : 1;
		u64 tpcsbe1 : 1;
		u64 tpcdbe0 : 1;
		u64 tpcsbe0 : 1;
		u64 tnfdbe1 : 1;
		u64 tnfsbe1 : 1;
		u64 tnfdbe0 : 1;
		u64 tnfsbe0 : 1;
		u64 tpfdbe1 : 1;
		u64 tpfsbe1 : 1;
		u64 tpfdbe0 : 1;
		u64 tpfsbe0 : 1;
		u64 datq_pe : 1;
		u64 hdrq_pe : 1;
		u64 rtry_pe : 1;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} s;
	struct cvmx_pemx_dbg_info_en_cn61xx {
		u64 reserved_31_63 : 33;
		u64 ecrc_e : 1;
		u64 rawwpp : 1;
		u64 racpp : 1;
		u64 ramtlp : 1;
		u64 rarwdns : 1;
		u64 caar : 1;
		u64 racca : 1;
		u64 racur : 1;
		u64 rauc : 1;
		u64 rqo : 1;
		u64 fcuv : 1;
		u64 rpe : 1;
		u64 fcpvwt : 1;
		u64 dpeoosd : 1;
		u64 rtwdle : 1;
		u64 rdwdle : 1;
		u64 mre : 1;
		u64 rte : 1;
		u64 acto : 1;
		u64 rvdm : 1;
		u64 rumep : 1;
		u64 rptamrc : 1;
		u64 rpmerc : 1;
		u64 rfemrc : 1;
		u64 rnfemrc : 1;
		u64 rcemrc : 1;
		u64 rpoison : 1;
		u64 recrce : 1;
		u64 rtlplle : 1;
		u64 rtlpmal : 1;
		u64 spoison : 1;
	} cn61xx;
	struct cvmx_pemx_dbg_info_en_cn61xx cn63xx;
	struct cvmx_pemx_dbg_info_en_cn61xx cn63xxp1;
	struct cvmx_pemx_dbg_info_en_cn61xx cn66xx;
	struct cvmx_pemx_dbg_info_en_cn61xx cn68xx;
	struct cvmx_pemx_dbg_info_en_cn61xx cn68xxp1;
	struct cvmx_pemx_dbg_info_en_s cn70xx;
	struct cvmx_pemx_dbg_info_en_s cn70xxp1;
	struct cvmx_pemx_dbg_info_en_cn61xx cnf71xx;
};

typedef union cvmx_pemx_dbg_info_en cvmx_pemx_dbg_info_en_t;

/**
 * cvmx_pem#_diag_status
 *
 * This register contains selection control for the core diagnostic bus.
 *
 */
union cvmx_pemx_diag_status {
	u64 u64;
	struct cvmx_pemx_diag_status_s {
		u64 reserved_9_63 : 55;
		u64 pwrdwn : 3;
		u64 pm_dst : 3;
		u64 pm_stat : 1;
		u64 pm_en : 1;
		u64 aux_en : 1;
	} s;
	struct cvmx_pemx_diag_status_cn61xx {
		u64 reserved_4_63 : 60;
		u64 pm_dst : 1;
		u64 pm_stat : 1;
		u64 pm_en : 1;
		u64 aux_en : 1;
	} cn61xx;
	struct cvmx_pemx_diag_status_cn61xx cn63xx;
	struct cvmx_pemx_diag_status_cn61xx cn63xxp1;
	struct cvmx_pemx_diag_status_cn61xx cn66xx;
	struct cvmx_pemx_diag_status_cn61xx cn68xx;
	struct cvmx_pemx_diag_status_cn61xx cn68xxp1;
	struct cvmx_pemx_diag_status_cn70xx {
		u64 reserved_63_6 : 58;
		u64 pm_dst : 3;
		u64 pm_stat : 1;
		u64 pm_en : 1;
		u64 aux_en : 1;
	} cn70xx;
	struct cvmx_pemx_diag_status_cn70xx cn70xxp1;
	struct cvmx_pemx_diag_status_cn73xx {
		u64 reserved_63_9 : 55;
		u64 pwrdwn : 3;
		u64 pm_dst : 3;
		u64 pm_stat : 1;
		u64 pm_en : 1;
		u64 aux_en : 1;
	} cn73xx;
	struct cvmx_pemx_diag_status_cn73xx cn78xx;
	struct cvmx_pemx_diag_status_cn73xx cn78xxp1;
	struct cvmx_pemx_diag_status_cn61xx cnf71xx;
	struct cvmx_pemx_diag_status_cn73xx cnf75xx;
};

typedef union cvmx_pemx_diag_status cvmx_pemx_diag_status_t;

/**
 * cvmx_pem#_ecc_ena
 *
 * Contains enables for TLP FIFO ECC RAMs.
 *
 */
union cvmx_pemx_ecc_ena {
	u64 u64;
	struct cvmx_pemx_ecc_ena_s {
		u64 reserved_35_63 : 29;
		u64 qhdr_b1_ena : 1;
		u64 qhdr_b0_ena : 1;
		u64 rtry_ena : 1;
		u64 reserved_11_31 : 21;
		u64 m2s_c_ena : 1;
		u64 m2s_d_ena : 1;
		u64 c_c_ena : 1;
		u64 c_d1_ena : 1;
		u64 c_d0_ena : 1;
		u64 reserved_0_5 : 6;
	} s;
	struct cvmx_pemx_ecc_ena_cn70xx {
		u64 reserved_6_63 : 58;
		u64 tlp_nc_ena : 1;
		u64 tlp_nd_ena : 1;
		u64 tlp_pc_ena : 1;
		u64 tlp_pd_ena : 1;
		u64 tlp_cc_ena : 1;
		u64 tlp_cd_ena : 1;
	} cn70xx;
	struct cvmx_pemx_ecc_ena_cn70xx cn70xxp1;
	struct cvmx_pemx_ecc_ena_cn73xx {
		u64 reserved_35_63 : 29;
		u64 qhdr_b1_ena : 1;
		u64 qhdr_b0_ena : 1;
		u64 rtry_ena : 1;
		u64 reserved_11_31 : 21;
		u64 m2s_c_ena : 1;
		u64 m2s_d_ena : 1;
		u64 c_c_ena : 1;
		u64 c_d1_ena : 1;
		u64 c_d0_ena : 1;
		u64 n_c_ena : 1;
		u64 n_d1_ena : 1;
		u64 n_d0_ena : 1;
		u64 p_c_ena : 1;
		u64 p_d1_ena : 1;
		u64 p_d0_ena : 1;
	} cn73xx;
	struct cvmx_pemx_ecc_ena_cn73xx cn78xx;
	struct cvmx_pemx_ecc_ena_cn78xxp1 {
		u64 reserved_35_63 : 29;
		u64 qhdr_b1_ena : 1;
		u64 qhdr_b0_ena : 1;
		u64 rtry_ena : 1;
		u64 reserved_9_31 : 23;
		u64 c_c_ena : 1;
		u64 c_d1_ena : 1;
		u64 c_d0_ena : 1;
		u64 n_c_ena : 1;
		u64 n_d1_ena : 1;
		u64 n_d0_ena : 1;
		u64 p_c_ena : 1;
		u64 p_d1_ena : 1;
		u64 p_d0_ena : 1;
	} cn78xxp1;
	struct cvmx_pemx_ecc_ena_cn73xx cnf75xx;
};

typedef union cvmx_pemx_ecc_ena cvmx_pemx_ecc_ena_t;

/**
 * cvmx_pem#_ecc_synd_ctrl
 *
 * This register contains syndrome control for TLP FIFO ECC RAMs.
 *
 */
union cvmx_pemx_ecc_synd_ctrl {
	u64 u64;
	struct cvmx_pemx_ecc_synd_ctrl_s {
		u64 reserved_38_63 : 26;
		u64 qhdr_b1_syn : 2;
		u64 qhdr_b0_syn : 2;
		u64 rtry_syn : 2;
		u64 reserved_22_31 : 10;
		u64 m2s_c_syn : 2;
		u64 m2s_d_syn : 2;
		u64 c_c_syn : 2;
		u64 c_d1_syn : 2;
		u64 c_d0_syn : 2;
		u64 reserved_0_11 : 12;
	} s;
	struct cvmx_pemx_ecc_synd_ctrl_cn70xx {
		u64 reserved_12_63 : 52;
		u64 tlp_nc_syn : 2;
		u64 tlp_nd_syn : 2;
		u64 tlp_pc_syn : 2;
		u64 tlp_pd_syn : 2;
		u64 tlp_cc_syn : 2;
		u64 tlp_cd_syn : 2;
	} cn70xx;
	struct cvmx_pemx_ecc_synd_ctrl_cn70xx cn70xxp1;
	struct cvmx_pemx_ecc_synd_ctrl_cn73xx {
		u64 reserved_38_63 : 26;
		u64 qhdr_b1_syn : 2;
		u64 qhdr_b0_syn : 2;
		u64 rtry_syn : 2;
		u64 reserved_22_31 : 10;
		u64 m2s_c_syn : 2;
		u64 m2s_d_syn : 2;
		u64 c_c_syn : 2;
		u64 c_d1_syn : 2;
		u64 c_d0_syn : 2;
		u64 n_c_syn : 2;
		u64 n_d1_syn : 2;
		u64 n_d0_syn : 2;
		u64 p_c_syn : 2;
		u64 p_d1_syn : 2;
		u64 p_d0_syn : 2;
	} cn73xx;
	struct cvmx_pemx_ecc_synd_ctrl_cn73xx cn78xx;
	struct cvmx_pemx_ecc_synd_ctrl_cn78xxp1 {
		u64 reserved_38_63 : 26;
		u64 qhdr_b1_syn : 2;
		u64 qhdr_b0_syn : 2;
		u64 rtry_syn : 2;
		u64 reserved_18_31 : 14;
		u64 c_c_syn : 2;
		u64 c_d1_syn : 2;
		u64 c_d0_syn : 2;
		u64 n_c_syn : 2;
		u64 n_d1_syn : 2;
		u64 n_d0_syn : 2;
		u64 p_c_syn : 2;
		u64 p_d1_syn : 2;
		u64 p_d0_syn : 2;
	} cn78xxp1;
	struct cvmx_pemx_ecc_synd_ctrl_cn73xx cnf75xx;
};

typedef union cvmx_pemx_ecc_synd_ctrl cvmx_pemx_ecc_synd_ctrl_t;

/**
 * cvmx_pem#_eco
 */
union cvmx_pemx_eco {
	u64 u64;
	struct cvmx_pemx_eco_s {
		u64 reserved_8_63 : 56;
		u64 eco_rw : 8;
	} s;
	struct cvmx_pemx_eco_s cn73xx;
	struct cvmx_pemx_eco_s cn78xx;
	struct cvmx_pemx_eco_s cnf75xx;
};

typedef union cvmx_pemx_eco cvmx_pemx_eco_t;

/**
 * cvmx_pem#_flr_glblcnt_ctl
 */
union cvmx_pemx_flr_glblcnt_ctl {
	u64 u64;
	struct cvmx_pemx_flr_glblcnt_ctl_s {
		u64 reserved_4_63 : 60;
		u64 chge : 1;
		u64 inc : 1;
		u64 delta : 2;
	} s;
	struct cvmx_pemx_flr_glblcnt_ctl_s cn73xx;
	struct cvmx_pemx_flr_glblcnt_ctl_s cn78xx;
	struct cvmx_pemx_flr_glblcnt_ctl_s cnf75xx;
};

typedef union cvmx_pemx_flr_glblcnt_ctl cvmx_pemx_flr_glblcnt_ctl_t;

/**
 * cvmx_pem#_flr_pf0_vf_stopreq
 *
 * Hardware automatically sets the STOPREQ bit for the VF when it enters a
 * Function Level Reset (FLR).  Software is responsible for clearing the STOPREQ
 * bit but must not do so prior to hardware taking down the FLR, which could be
 * as long as 100ms.  It may be appropriate for software to wait longer before clearing
 * STOPREQ, software may need to drain deep DPI queues for example.
 * Whenever PEM receives a request mastered by CNXXXX over S2M (i.e. P or NP),
 * when STOPREQ is set for the function, PEM will discard the outgoing request
 * before sending it to the PCIe core.  If a NP, PEM will schedule an immediate
 * SWI_RSP_ERROR completion for the request - no timeout is required.
 *
 * STOPREQ mimics the behavior of PCIEEPVF()_CFG001[ME] for outbound requests that will
 * master the PCIe bus (P and NP).
 *
 * Note that STOPREQ will have no effect on completions returned by CNXXXX over the S2M,
 * nor on M2S traffic.
 */
union cvmx_pemx_flr_pf0_vf_stopreq {
	u64 u64;
	struct cvmx_pemx_flr_pf0_vf_stopreq_s {
		u64 vf_stopreq : 64;
	} s;
	struct cvmx_pemx_flr_pf0_vf_stopreq_s cn73xx;
	struct cvmx_pemx_flr_pf0_vf_stopreq_s cn78xx;
	struct cvmx_pemx_flr_pf0_vf_stopreq_s cnf75xx;
};

typedef union cvmx_pemx_flr_pf0_vf_stopreq cvmx_pemx_flr_pf0_vf_stopreq_t;

/**
 * cvmx_pem#_flr_pf_stopreq
 *
 * Hardware automatically sets the STOPREQ bit for the PF when it enters a
 * Function Level Reset (FLR).  Software is responsible for clearing the STOPREQ
 * bit but must not do so prior to hardware taking down the FLR, which could be
 * as long as 100ms.  It may be appropriate for software to wait longer before clearing
 * STOPREQ, software may need to drain deep DPI queues for example.
 * Whenever PEM receives a PF or child VF request mastered by CNXXXX over S2M (i.e. P or NP),
 * when STOPREQ is set for the function, PEM will discard the outgoing request
 * before sending it to the PCIe core.  If a NP, PEM will schedule an immediate
 * SWI_RSP_ERROR completion for the request - no timeout is required.
 *
 * STOPREQ mimics the behavior of PCIEEP()_CFG001[ME] for outbound requests that will
 * master the PCIe bus (P and NP).
 *
 * STOPREQ will have no effect on completions returned by CNXXXX over the S2M,
 * nor on M2S traffic.
 *
 * When a PF()_STOPREQ is set, none of the associated
 * PEM()_FLR_PF()_VF_STOPREQ[VF_STOPREQ] will be set.
 *
 * STOPREQ is reset when the MAC is reset, and is not reset after a chip soft reset.
 */
union cvmx_pemx_flr_pf_stopreq {
	u64 u64;
	struct cvmx_pemx_flr_pf_stopreq_s {
		u64 reserved_1_63 : 63;
		u64 pf0_stopreq : 1;
	} s;
	struct cvmx_pemx_flr_pf_stopreq_s cn73xx;
	struct cvmx_pemx_flr_pf_stopreq_s cn78xx;
	struct cvmx_pemx_flr_pf_stopreq_s cnf75xx;
};

typedef union cvmx_pemx_flr_pf_stopreq cvmx_pemx_flr_pf_stopreq_t;

/**
 * cvmx_pem#_flr_stopreq_ctl
 */
union cvmx_pemx_flr_stopreq_ctl {
	u64 u64;
	struct cvmx_pemx_flr_stopreq_ctl_s {
		u64 reserved_1_63 : 63;
		u64 stopreqclr : 1;
	} s;
	struct cvmx_pemx_flr_stopreq_ctl_s cn78xx;
	struct cvmx_pemx_flr_stopreq_ctl_s cnf75xx;
};

typedef union cvmx_pemx_flr_stopreq_ctl cvmx_pemx_flr_stopreq_ctl_t;

/**
 * cvmx_pem#_flr_zombie_ctl
 */
union cvmx_pemx_flr_zombie_ctl {
	u64 u64;
	struct cvmx_pemx_flr_zombie_ctl_s {
		u64 reserved_10_63 : 54;
		u64 exp : 10;
	} s;
	struct cvmx_pemx_flr_zombie_ctl_s cn73xx;
	struct cvmx_pemx_flr_zombie_ctl_s cn78xx;
	struct cvmx_pemx_flr_zombie_ctl_s cnf75xx;
};

typedef union cvmx_pemx_flr_zombie_ctl cvmx_pemx_flr_zombie_ctl_t;

/**
 * cvmx_pem#_inb_read_credits
 *
 * This register contains the number of in-flight read operations from PCIe core to SLI.
 *
 */
union cvmx_pemx_inb_read_credits {
	u64 u64;
	struct cvmx_pemx_inb_read_credits_s {
		u64 reserved_7_63 : 57;
		u64 num : 7;
	} s;
	struct cvmx_pemx_inb_read_credits_cn61xx {
		u64 reserved_6_63 : 58;
		u64 num : 6;
	} cn61xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn66xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn68xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn70xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn70xxp1;
	struct cvmx_pemx_inb_read_credits_s cn73xx;
	struct cvmx_pemx_inb_read_credits_s cn78xx;
	struct cvmx_pemx_inb_read_credits_s cn78xxp1;
	struct cvmx_pemx_inb_read_credits_cn61xx cnf71xx;
	struct cvmx_pemx_inb_read_credits_s cnf75xx;
};

typedef union cvmx_pemx_inb_read_credits cvmx_pemx_inb_read_credits_t;

/**
 * cvmx_pem#_int_enb
 *
 * "PEM#_INT_ENB = PEM Interrupt Enable
 * Enables interrupt conditions for the PEM to generate an RSL interrupt."
 */
union cvmx_pemx_int_enb {
	u64 u64;
	struct cvmx_pemx_int_enb_s {
		u64 reserved_14_63 : 50;
		u64 crs_dr : 1;
		u64 crs_er : 1;
		u64 rdlk : 1;
		u64 exc : 1;
		u64 un_bx : 1;
		u64 un_b2 : 1;
		u64 un_b1 : 1;
		u64 up_bx : 1;
		u64 up_b2 : 1;
		u64 up_b1 : 1;
		u64 pmem : 1;
		u64 pmei : 1;
		u64 se : 1;
		u64 aeri : 1;
	} s;
	struct cvmx_pemx_int_enb_s cn61xx;
	struct cvmx_pemx_int_enb_s cn63xx;
	struct cvmx_pemx_int_enb_s cn63xxp1;
	struct cvmx_pemx_int_enb_s cn66xx;
	struct cvmx_pemx_int_enb_s cn68xx;
	struct cvmx_pemx_int_enb_s cn68xxp1;
	struct cvmx_pemx_int_enb_s cn70xx;
	struct cvmx_pemx_int_enb_s cn70xxp1;
	struct cvmx_pemx_int_enb_s cnf71xx;
};

typedef union cvmx_pemx_int_enb cvmx_pemx_int_enb_t;

/**
 * cvmx_pem#_int_enb_int
 *
 * "PEM#_INT_ENB_INT = PEM Interrupt Enable
 * Enables interrupt conditions for the PEM to generate an RSL interrupt."
 */
union cvmx_pemx_int_enb_int {
	u64 u64;
	struct cvmx_pemx_int_enb_int_s {
		u64 reserved_14_63 : 50;
		u64 crs_dr : 1;
		u64 crs_er : 1;
		u64 rdlk : 1;
		u64 exc : 1;
		u64 un_bx : 1;
		u64 un_b2 : 1;
		u64 un_b1 : 1;
		u64 up_bx : 1;
		u64 up_b2 : 1;
		u64 up_b1 : 1;
		u64 pmem : 1;
		u64 pmei : 1;
		u64 se : 1;
		u64 aeri : 1;
	} s;
	struct cvmx_pemx_int_enb_int_s cn61xx;
	struct cvmx_pemx_int_enb_int_s cn63xx;
	struct cvmx_pemx_int_enb_int_s cn63xxp1;
	struct cvmx_pemx_int_enb_int_s cn66xx;
	struct cvmx_pemx_int_enb_int_s cn68xx;
	struct cvmx_pemx_int_enb_int_s cn68xxp1;
	struct cvmx_pemx_int_enb_int_s cn70xx;
	struct cvmx_pemx_int_enb_int_s cn70xxp1;
	struct cvmx_pemx_int_enb_int_s cnf71xx;
};

typedef union cvmx_pemx_int_enb_int cvmx_pemx_int_enb_int_t;

/**
 * cvmx_pem#_int_sum
 *
 * This register contains the different interrupt summary bits of the PEM.
 *
 */
union cvmx_pemx_int_sum {
	u64 u64;
	struct cvmx_pemx_int_sum_s {
		u64 intd : 1;
		u64 intc : 1;
		u64 intb : 1;
		u64 inta : 1;
		u64 reserved_14_59 : 46;
		u64 crs_dr : 1;
		u64 crs_er : 1;
		u64 rdlk : 1;
		u64 exc : 1;
		u64 un_bx : 1;
		u64 un_b2 : 1;
		u64 un_b1 : 1;
		u64 up_bx : 1;
		u64 up_b2 : 1;
		u64 up_b1 : 1;
		u64 pmem : 1;
		u64 pmei : 1;
		u64 se : 1;
		u64 aeri : 1;
	} s;
	struct cvmx_pemx_int_sum_cn61xx {
		u64 reserved_14_63 : 50;
		u64 crs_dr : 1;
		u64 crs_er : 1;
		u64 rdlk : 1;
		u64 exc : 1;
		u64 un_bx : 1;
		u64 un_b2 : 1;
		u64 un_b1 : 1;
		u64 up_bx : 1;
		u64 up_b2 : 1;
		u64 up_b1 : 1;
		u64 pmem : 1;
		u64 pmei : 1;
		u64 se : 1;
		u64 aeri : 1;
	} cn61xx;
	struct cvmx_pemx_int_sum_cn61xx cn63xx;
	struct cvmx_pemx_int_sum_cn61xx cn63xxp1;
	struct cvmx_pemx_int_sum_cn61xx cn66xx;
	struct cvmx_pemx_int_sum_cn61xx cn68xx;
	struct cvmx_pemx_int_sum_cn61xx cn68xxp1;
	struct cvmx_pemx_int_sum_cn61xx cn70xx;
	struct cvmx_pemx_int_sum_cn61xx cn70xxp1;
	struct cvmx_pemx_int_sum_cn73xx {
		u64 intd : 1;
		u64 intc : 1;
		u64 intb : 1;
		u64 inta : 1;
		u64 reserved_14_59 : 46;
		u64 crs_dr : 1;
		u64 crs_er : 1;
		u64 rdlk : 1;
		u64 reserved_10_10 : 1;
		u64 un_bx : 1;
		u64 un_b2 : 1;
		u64 un_b1 : 1;
		u64 up_bx : 1;
		u64 up_b2 : 1;
		u64 up_b1 : 1;
		u64 reserved_3_3 : 1;
		u64 pmei : 1;
		u64 se : 1;
		u64 aeri : 1;
	} cn73xx;
	struct cvmx_pemx_int_sum_cn73xx cn78xx;
	struct cvmx_pemx_int_sum_cn73xx cn78xxp1;
	struct cvmx_pemx_int_sum_cn61xx cnf71xx;
	struct cvmx_pemx_int_sum_cn73xx cnf75xx;
};

typedef union cvmx_pemx_int_sum cvmx_pemx_int_sum_t;

/**
 * cvmx_pem#_on
 *
 * This register indicates that PEM is configured and ready.
 *
 */
union cvmx_pemx_on {
	u64 u64;
	struct cvmx_pemx_on_s {
		u64 reserved_2_63 : 62;
		u64 pemoor : 1;
		u64 pemon : 1;
	} s;
	struct cvmx_pemx_on_s cn70xx;
	struct cvmx_pemx_on_s cn70xxp1;
	struct cvmx_pemx_on_s cn73xx;
	struct cvmx_pemx_on_s cn78xx;
	struct cvmx_pemx_on_s cn78xxp1;
	struct cvmx_pemx_on_s cnf75xx;
};

typedef union cvmx_pemx_on cvmx_pemx_on_t;

/**
 * cvmx_pem#_p2n_bar0_start
 *
 * This register specifies the starting address for memory requests that are to be forwarded to
 * the SLI in RC mode.
 */
union cvmx_pemx_p2n_bar0_start {
	u64 u64;
	struct cvmx_pemx_p2n_bar0_start_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pemx_p2n_bar0_start_cn61xx {
		u64 addr : 50;
		u64 reserved_0_13 : 14;
	} cn61xx;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn63xx;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn63xxp1;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn66xx;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn68xx;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn68xxp1;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn70xx;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cn70xxp1;
	struct cvmx_pemx_p2n_bar0_start_cn73xx {
		u64 addr : 41;
		u64 reserved_0_22 : 23;
	} cn73xx;
	struct cvmx_pemx_p2n_bar0_start_cn73xx cn78xx;
	struct cvmx_pemx_p2n_bar0_start_cn78xxp1 {
		u64 addr : 49;
		u64 reserved_0_14 : 15;
	} cn78xxp1;
	struct cvmx_pemx_p2n_bar0_start_cn61xx cnf71xx;
	struct cvmx_pemx_p2n_bar0_start_cn73xx cnf75xx;
};

typedef union cvmx_pemx_p2n_bar0_start cvmx_pemx_p2n_bar0_start_t;

/**
 * cvmx_pem#_p2n_bar1_start
 *
 * This register specifies the starting address for memory requests that are to be forwarded to
 * the SLI in RC mode.
 */
union cvmx_pemx_p2n_bar1_start {
	u64 u64;
	struct cvmx_pemx_p2n_bar1_start_s {
		u64 addr : 38;
		u64 reserved_0_25 : 26;
	} s;
	struct cvmx_pemx_p2n_bar1_start_s cn61xx;
	struct cvmx_pemx_p2n_bar1_start_s cn63xx;
	struct cvmx_pemx_p2n_bar1_start_s cn63xxp1;
	struct cvmx_pemx_p2n_bar1_start_s cn66xx;
	struct cvmx_pemx_p2n_bar1_start_s cn68xx;
	struct cvmx_pemx_p2n_bar1_start_s cn68xxp1;
	struct cvmx_pemx_p2n_bar1_start_s cn70xx;
	struct cvmx_pemx_p2n_bar1_start_s cn70xxp1;
	struct cvmx_pemx_p2n_bar1_start_s cn73xx;
	struct cvmx_pemx_p2n_bar1_start_s cn78xx;
	struct cvmx_pemx_p2n_bar1_start_s cn78xxp1;
	struct cvmx_pemx_p2n_bar1_start_s cnf71xx;
	struct cvmx_pemx_p2n_bar1_start_s cnf75xx;
};

typedef union cvmx_pemx_p2n_bar1_start cvmx_pemx_p2n_bar1_start_t;

/**
 * cvmx_pem#_p2n_bar2_start
 *
 * This register specifies the starting address for memory requests that are to be forwarded to
 * the SLI in RC mode.
 */
union cvmx_pemx_p2n_bar2_start {
	u64 u64;
	struct cvmx_pemx_p2n_bar2_start_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pemx_p2n_bar2_start_cn61xx {
		u64 addr : 23;
		u64 reserved_0_40 : 41;
	} cn61xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn63xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn63xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn66xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn68xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn68xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn70xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn70xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn73xx {
		u64 addr : 19;
		u64 reserved_0_44 : 45;
	} cn73xx;
	struct cvmx_pemx_p2n_bar2_start_cn73xx cn78xx;
	struct cvmx_pemx_p2n_bar2_start_cn73xx cn78xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cnf71xx;
	struct cvmx_pemx_p2n_bar2_start_cn73xx cnf75xx;
};

typedef union cvmx_pemx_p2n_bar2_start cvmx_pemx_p2n_bar2_start_t;

/**
 * cvmx_pem#_p2p_bar#_end
 *
 * This register specifies the ending address for memory requests that are to be forwarded to the
 * PCIe peer port.
 */
union cvmx_pemx_p2p_barx_end {
	u64 u64;
	struct cvmx_pemx_p2p_barx_end_s {
		u64 addr : 52;
		u64 reserved_0_11 : 12;
	} s;
	struct cvmx_pemx_p2p_barx_end_s cn63xx;
	struct cvmx_pemx_p2p_barx_end_s cn63xxp1;
	struct cvmx_pemx_p2p_barx_end_s cn66xx;
	struct cvmx_pemx_p2p_barx_end_s cn68xx;
	struct cvmx_pemx_p2p_barx_end_s cn68xxp1;
	struct cvmx_pemx_p2p_barx_end_s cn73xx;
	struct cvmx_pemx_p2p_barx_end_s cn78xx;
	struct cvmx_pemx_p2p_barx_end_s cn78xxp1;
	struct cvmx_pemx_p2p_barx_end_s cnf75xx;
};

typedef union cvmx_pemx_p2p_barx_end cvmx_pemx_p2p_barx_end_t;

/**
 * cvmx_pem#_p2p_bar#_start
 *
 * This register specifies the starting address for memory requests that are to be forwarded to
 * the PCIe peer port.
 */
union cvmx_pemx_p2p_barx_start {
	u64 u64;
	struct cvmx_pemx_p2p_barx_start_s {
		u64 addr : 52;
		u64 reserved_2_11 : 10;
		u64 dst : 2;
	} s;
	struct cvmx_pemx_p2p_barx_start_cn63xx {
		u64 addr : 52;
		u64 reserved_0_11 : 12;
	} cn63xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn63xxp1;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn66xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn68xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn68xxp1;
	struct cvmx_pemx_p2p_barx_start_s cn73xx;
	struct cvmx_pemx_p2p_barx_start_s cn78xx;
	struct cvmx_pemx_p2p_barx_start_s cn78xxp1;
	struct cvmx_pemx_p2p_barx_start_s cnf75xx;
};

typedef union cvmx_pemx_p2p_barx_start cvmx_pemx_p2p_barx_start_t;

/**
 * cvmx_pem#_qlm
 *
 * This register configures the PEM3 QLM.
 *
 */
union cvmx_pemx_qlm {
	u64 u64;
	struct cvmx_pemx_qlm_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pemx_qlm_cn73xx {
		u64 reserved_1_63 : 63;
		u64 pemdlmsel : 1;
	} cn73xx;
	struct cvmx_pemx_qlm_cn78xx {
		u64 reserved_1_63 : 63;
		u64 pem3qlm : 1;
	} cn78xx;
	struct cvmx_pemx_qlm_cn78xx cn78xxp1;
	struct cvmx_pemx_qlm_cn73xx cnf75xx;
};

typedef union cvmx_pemx_qlm cvmx_pemx_qlm_t;

/**
 * cvmx_pem#_spi_ctl
 *
 * PEM#_SPI_CTL register.
 *
 */
union cvmx_pemx_spi_ctl {
	u64 u64;
	struct cvmx_pemx_spi_ctl_s {
		u64 reserved_14_63 : 50;
		u64 start_busy : 1;
		u64 tvalid : 1;
		u64 cmd : 3;
		u64 adr : 9;
	} s;
	struct cvmx_pemx_spi_ctl_s cn70xx;
	struct cvmx_pemx_spi_ctl_s cn70xxp1;
	struct cvmx_pemx_spi_ctl_s cn73xx;
	struct cvmx_pemx_spi_ctl_s cn78xx;
	struct cvmx_pemx_spi_ctl_s cn78xxp1;
	struct cvmx_pemx_spi_ctl_s cnf75xx;
};

typedef union cvmx_pemx_spi_ctl cvmx_pemx_spi_ctl_t;

/**
 * cvmx_pem#_spi_data
 *
 * This register contains the most recently read or written SPI data and is unpredictable upon
 * power-up. Is valid after a PEM()_SPI_CTL[CMD]=READ/RDSR when hardware clears
 * PEM()_SPI_CTL[START_BUSY]. Is written after a PEM()_SPI_CTL[CMD]=WRITE/WRSR
 * when hardware clears PEM()_SPI_CTL[START_BUSY].
 */
union cvmx_pemx_spi_data {
	u64 u64;
	struct cvmx_pemx_spi_data_s {
		u64 preamble : 16;
		u64 reserved_45_47 : 3;
		u64 cs2 : 1;
		u64 adr : 12;
		u64 data : 32;
	} s;
	struct cvmx_pemx_spi_data_s cn70xx;
	struct cvmx_pemx_spi_data_s cn70xxp1;
	struct cvmx_pemx_spi_data_s cn73xx;
	struct cvmx_pemx_spi_data_s cn78xx;
	struct cvmx_pemx_spi_data_s cn78xxp1;
	struct cvmx_pemx_spi_data_s cnf75xx;
};

typedef union cvmx_pemx_spi_data cvmx_pemx_spi_data_t;

/**
 * cvmx_pem#_strap
 *
 * "Below are in pesc_csr
 * The input strapping pins"
 */
union cvmx_pemx_strap {
	u64 u64;
	struct cvmx_pemx_strap_s {
		u64 reserved_5_63 : 59;
		u64 miopem2dlm5sel : 1;
		u64 pilaneswap : 1;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_pemx_strap_cn70xx {
		u64 reserved_4_63 : 60;
		u64 pilaneswap : 1;
		u64 pimode : 3;
	} cn70xx;
	struct cvmx_pemx_strap_cn70xx cn70xxp1;
	struct cvmx_pemx_strap_cn73xx {
		u64 reserved_5_63 : 59;
		u64 miopem2dlm5sel : 1;
		u64 pilaneswap : 1;
		u64 pilanes8 : 1;
		u64 pimode : 2;
	} cn73xx;
	struct cvmx_pemx_strap_cn78xx {
		u64 reserved_4_63 : 60;
		u64 pilaneswap : 1;
		u64 pilanes8 : 1;
		u64 pimode : 2;
	} cn78xx;
	struct cvmx_pemx_strap_cn78xx cn78xxp1;
	struct cvmx_pemx_strap_cnf75xx {
		u64 reserved_5_63 : 59;
		u64 miopem2dlm5sel : 1;
		u64 pilaneswap : 1;
		u64 pilanes4 : 1;
		u64 pimode : 2;
	} cnf75xx;
};

typedef union cvmx_pemx_strap cvmx_pemx_strap_t;

/**
 * cvmx_pem#_tlp_credits
 *
 * This register specifies the number of credits for use in moving TLPs. When this register is
 * written, the credit values are reset to the register value. A write to this register should
 * take place before traffic flow starts.
 */
union cvmx_pemx_tlp_credits {
	u64 u64;
	struct cvmx_pemx_tlp_credits_s {
		u64 reserved_56_63 : 8;
		u64 peai_ppf : 8;
		u64 pem_cpl : 8;
		u64 pem_np : 8;
		u64 pem_p : 8;
		u64 sli_cpl : 8;
		u64 sli_np : 8;
		u64 sli_p : 8;
	} s;
	struct cvmx_pemx_tlp_credits_cn61xx {
		u64 reserved_56_63 : 8;
		u64 peai_ppf : 8;
		u64 reserved_24_47 : 24;
		u64 sli_cpl : 8;
		u64 sli_np : 8;
		u64 sli_p : 8;
	} cn61xx;
	struct cvmx_pemx_tlp_credits_s cn63xx;
	struct cvmx_pemx_tlp_credits_s cn63xxp1;
	struct cvmx_pemx_tlp_credits_s cn66xx;
	struct cvmx_pemx_tlp_credits_s cn68xx;
	struct cvmx_pemx_tlp_credits_s cn68xxp1;
	struct cvmx_pemx_tlp_credits_cn61xx cn70xx;
	struct cvmx_pemx_tlp_credits_cn61xx cn70xxp1;
	struct cvmx_pemx_tlp_credits_cn73xx {
		u64 reserved_48_63 : 16;
		u64 pem_cpl : 8;
		u64 pem_np : 8;
		u64 pem_p : 8;
		u64 sli_cpl : 8;
		u64 sli_np : 8;
		u64 sli_p : 8;
	} cn73xx;
	struct cvmx_pemx_tlp_credits_cn73xx cn78xx;
	struct cvmx_pemx_tlp_credits_cn73xx cn78xxp1;
	struct cvmx_pemx_tlp_credits_cn61xx cnf71xx;
	struct cvmx_pemx_tlp_credits_cn73xx cnf75xx;
};

typedef union cvmx_pemx_tlp_credits cvmx_pemx_tlp_credits_t;

#endif
