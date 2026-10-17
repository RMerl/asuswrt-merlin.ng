/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_LMCX_DEFS_H__
#define __CVMX_LMCX_DEFS_H__

#define CVMX_LMCX_BANK_CONFLICT1(offs)			\
	((0x000360ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_BANK_CONFLICT2(offs)			\
	((0x000368ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_BIST_RESULT(offs)			\
	((0x0000F8ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_CHAR_CTL(offs)			\
	((0x000220ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_DQ_ERR_COUNT(offs)		\
	((0x000040ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_MASK0(offs)			\
	((0x000228ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_MASK1(offs)			\
	((0x000230ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_MASK2(offs)			\
	((0x000238ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_MASK3(offs)			\
	((0x000240ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CHAR_MASK4(offs)			\
	((0x000318ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_COMP_CTL(offs)			\
	((0x000028ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_COMP_CTL2(offs)			\
	((0x0001B8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CONFIG(offs)				\
	((0x000188ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CONTROL(offs)				\
	((0x000190ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_CTL(offs)				\
	((0x000010ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_CTL1(offs)				\
	((0x000090ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DBTRAIN_CTL(offs)			\
	((0x0003F8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DCLK_CNT(offs)			\
	((0x0001E0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DCLK_CNT_HI(offs)			\
	((0x000070ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DCLK_CNT_LO(offs)			\
	((0x000068ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DCLK_CTL(offs)			\
	((0x0000B8ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DDR2_CTL(offs)			\
	((0x000018ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DDR4_DIMM_CTL(offs)			\
	((0x0003F0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DDR_PLL_CTL(offs)			\
	((0x000258ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DELAY_CFG(offs)			\
	((0x000088ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DIMMX_DDR4_PARAMS0(offs, id)				\
	((0x0000D0ull) + (((offs) & 1) + ((id) & 3) * 0x200000ull) * 8)
#define CVMX_LMCX_DIMMX_DDR4_PARAMS1(offs, id)				\
	((0x000140ull) + (((offs) & 1) + ((id) & 3) * 0x200000ull) * 8)
#define CVMX_LMCX_DIMMX_PARAMS(offs, id)				\
	((0x000270ull) + (((offs) & 1) + ((id) & 3) * 0x200000ull) * 8)
#define CVMX_LMCX_DIMM_CTL(offs)			\
	((0x000310ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DLL_CTL(offs)				\
	((0x0000C0ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_DLL_CTL2(offs)			\
	((0x0001C8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_DLL_CTL3(offs)			\
	((0x000218ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_ECC_PARITY_TEST(offs)			\
	((0x000108ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_EXT_CONFIG(offs)			\
	((0x000030ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_EXT_CONFIG2(offs)			\
	((0x000090ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_GENERAL_PURPOSE0(offs)		\
	((0x000340ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_GENERAL_PURPOSE1(offs)		\
	((0x000348ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_GENERAL_PURPOSE2(offs)		\
	((0x000350ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_IFB_CNT(offs)				\
	((0x0001D0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_IFB_CNT_HI(offs)			\
	((0x000050ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_IFB_CNT_LO(offs)			\
	((0x000048ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_INT(offs)				\
	((0x0001F0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_INT_EN(offs)				\
	((0x0001E8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_LANEX_CRC_SWIZ(x, id)					\
	((0x000380ull) + (((offs) & 15) + ((id) & 3) * 0x200000ull) * 8)
#define CVMX_LMCX_MEM_CFG0(offs)			\
	((0x000000ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_MEM_CFG1(offs)			\
	((0x000008ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_MODEREG_PARAMS0(offs)			\
	((0x0001A8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MODEREG_PARAMS1(offs)			\
	((0x000260ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MODEREG_PARAMS2(offs)			\
	((0x000050ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MODEREG_PARAMS3(offs)			\
	((0x000058ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MPR_DATA0(offs)			\
	((0x000070ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MPR_DATA1(offs)			\
	((0x000078ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MPR_DATA2(offs)			\
	((0x000080ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_MR_MPR_CTL(offs)			\
	((0x000068ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_NS_CTL(offs)				\
	((0x000178ull) + ((offs) & 3) * 0x1000000ull)

static inline uint64_t CVMX_LMCX_NXM(unsigned long offs)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return (0x0000C8ull) + (offs) * 0x60000000ull;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return (0x0000C8ull) + (offs) * 0x1000000ull;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return (0x0000C8ull) + (offs) * 0x1000000ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return (0x0000C8ull) + (offs) * 0x1000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return (0x0000C8ull) + (offs) * 0x1000000ull;
	}
	return (0x0000C8ull) + (offs) * 0x1000000ull;
}

#define CVMX_LMCX_NXM_FADR(offs)			\
	((0x000028ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_OPS_CNT(offs)				\
	((0x0001D8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_OPS_CNT_HI(offs)			\
	((0x000060ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_OPS_CNT_LO(offs)			\
	((0x000058ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_PHY_CTL(offs)				\
	((0x000210ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_PHY_CTL2(offs)			\
	((0x000250ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_PLL_BWCTL(offs)		\
	((0x000040ull))
#define CVMX_LMCX_PLL_CTL(offs)				\
	((0x0000A8ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_PLL_STATUS(offs)			\
	((0x0000B0ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_PPR_CTL(offs)				\
	((0x0003E0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_READ_LEVEL_CTL(offs)			\
	((0x000140ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_READ_LEVEL_DBG(offs)			\
	((0x000148ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_READ_LEVEL_RANKX(offs, id)				\
	((0x000100ull) + (((offs) & 3) + ((id) & 1) * 0xC000000ull) * 8)
#define CVMX_LMCX_REF_STATUS(offs)			\
	((0x0000A0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RESET_CTL(offs)			\
	((0x000180ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RETRY_CONFIG(offs)			\
	((0x000110ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RETRY_STATUS(offs)			\
	((0x000118ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RLEVEL_CTL(offs)			\
	((0x0002A0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RLEVEL_DBG(offs)			\
	((0x0002A8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_RLEVEL_RANKX(offs, id)				\
	((0x000280ull) + (((offs) & 3) + ((id) & 3) * 0x200000ull) * 8)
#define CVMX_LMCX_RODT_COMP_CTL(offs)			\
	((0x0000A0ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_RODT_CTL(offs)			\
	((0x000078ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_RODT_MASK(offs)			\
	((0x000268ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SCRAMBLED_FADR(offs)			\
	((0x000330ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SCRAMBLE_CFG0(offs)			\
	((0x000320ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SCRAMBLE_CFG1(offs)			\
	((0x000328ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SCRAMBLE_CFG2(offs)			\
	((0x000338ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SEQ_CTL(offs)				\
	((0x000048ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SLOT_CTL0(offs)			\
	((0x0001F8ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SLOT_CTL1(offs)			\
	((0x000200ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SLOT_CTL2(offs)			\
	((0x000208ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_SLOT_CTL3(offs)			\
	((0x000248ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_TIMING_PARAMS0(offs)			\
	((0x000198ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_TIMING_PARAMS1(offs)			\
	((0x0001A0ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_TIMING_PARAMS2(offs)			\
	((0x000060ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_TRO_CTL(offs)				\
	((0x000248ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_TRO_STAT(offs)			\
	((0x000250ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_WLEVEL_CTL(offs)			\
	((0x000300ull) + ((offs) & 3) * 0x1000000ull)
#define CVMX_LMCX_WLEVEL_DBG(offs)			\
	((0x000308ull) + ((offs) & 3) * 0x1000000ull)

static inline uint64_t CVMX_LMCX_WLEVEL_RANKX(unsigned long offs,
					      unsigned long id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return (0x0002C0ull) + ((offs) + (id) * 0x200000ull) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return (0x0002C0ull) + ((offs) + (id) * 0x200000ull) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return (0x0002C0ull) + ((offs) +
						(id) * 0x200000ull) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return (0x0002C0ull) + ((offs) +
						(id) * 0x200000ull) * 8;

	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return (0x0002B0ull) + ((offs) + (id) * 0x0ull) * 8;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return (0x0002B0ull) + ((offs) + (id) * 0x200000ull) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return (0x0002B0ull) + ((offs) + (id) * 0x200000ull) * 8;
	}
	return (0x0002C0ull) + ((offs) + (id) * 0x200000ull) * 8;
}

#define CVMX_LMCX_WODT_CTL0(offs)			\
	((0x000030ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_WODT_CTL1(offs)			\
	((0x000080ull) + ((offs) & 1) * 0x60000000ull)
#define CVMX_LMCX_WODT_MASK(offs)			\
	((0x0001B0ull) + ((offs) & 3) * 0x1000000ull)

/**
 * cvmx_lmc#_char_ctl
 *
 * This register provides an assortment of various control fields needed
 * to characterize the DDR3 interface.
 */
union cvmx_lmcx_char_ctl {
	u64 u64;
	struct cvmx_lmcx_char_ctl_s {
		uint64_t reserved_54_63:10;
		uint64_t dq_char_byte_check:1;
		uint64_t dq_char_check_lock:1;
		uint64_t dq_char_check_enable:1;
		uint64_t dq_char_bit_sel:3;
		uint64_t dq_char_byte_sel:4;
		uint64_t dr:1;
		uint64_t skew_on:1;
		uint64_t en:1;
		uint64_t sel:1;
		uint64_t prog:8;
		uint64_t prbs:32;
	} s;
	struct cvmx_lmcx_char_ctl_cn61xx {
		uint64_t reserved_44_63:20;
		uint64_t dr:1;
		uint64_t skew_on:1;
		uint64_t en:1;
		uint64_t sel:1;
		uint64_t prog:8;
		uint64_t prbs:32;
	} cn61xx;
	struct cvmx_lmcx_char_ctl_cn63xx {
		uint64_t reserved_42_63:22;
		uint64_t en:1;
		uint64_t sel:1;
		uint64_t prog:8;
		uint64_t prbs:32;
	} cn63xx;
	struct cvmx_lmcx_char_ctl_cn63xx cn63xxp1;
	struct cvmx_lmcx_char_ctl_cn61xx cn66xx;
	struct cvmx_lmcx_char_ctl_cn61xx cn68xx;
	struct cvmx_lmcx_char_ctl_cn63xx cn68xxp1;
	struct cvmx_lmcx_char_ctl_cn70xx {
		uint64_t reserved_53_63:11;
		uint64_t dq_char_check_lock:1;
		uint64_t dq_char_check_enable:1;
		uint64_t dq_char_bit_sel:3;
		uint64_t dq_char_byte_sel:4;
		uint64_t dr:1;
		uint64_t skew_on:1;
		uint64_t en:1;
		uint64_t sel:1;
		uint64_t prog:8;
		uint64_t prbs:32;
	} cn70xx;
	struct cvmx_lmcx_char_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_char_ctl_s cn73xx;
	struct cvmx_lmcx_char_ctl_s cn78xx;
	struct cvmx_lmcx_char_ctl_s cn78xxp1;
	struct cvmx_lmcx_char_ctl_cn61xx cnf71xx;
	struct cvmx_lmcx_char_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_comp_ctl2
 *
 * LMC_COMP_CTL2 = LMC Compensation control
 *
 */
union cvmx_lmcx_comp_ctl2 {
	u64 u64;
	struct cvmx_lmcx_comp_ctl2_s {
		uint64_t reserved_51_63:13;
		uint64_t rclk_char_mode:1;
		uint64_t reserved_40_49:10;
		uint64_t ptune_offset:4;
		uint64_t reserved_12_35:24;
		uint64_t cmd_ctl:4;
		uint64_t ck_ctl:4;
		uint64_t dqx_ctl:4;
	} s;
	struct cvmx_lmcx_comp_ctl2_cn61xx {
		uint64_t reserved_34_63:30;
		uint64_t ddr__ptune:4;
		uint64_t ddr__ntune:4;
		uint64_t m180:1;
		uint64_t byp:1;
		uint64_t ptune:4;
		uint64_t ntune:4;
		uint64_t rodt_ctl:4;
		uint64_t cmd_ctl:4;
		uint64_t ck_ctl:4;
		uint64_t dqx_ctl:4;
	} cn61xx;
	struct cvmx_lmcx_comp_ctl2_cn61xx cn63xx;
	struct cvmx_lmcx_comp_ctl2_cn61xx cn63xxp1;
	struct cvmx_lmcx_comp_ctl2_cn61xx cn66xx;
	struct cvmx_lmcx_comp_ctl2_cn61xx cn68xx;
	struct cvmx_lmcx_comp_ctl2_cn61xx cn68xxp1;
	struct cvmx_lmcx_comp_ctl2_cn70xx {
		uint64_t reserved_51_63:13;
		uint64_t rclk_char_mode:1;
		uint64_t ddr__ptune:5;
		uint64_t ddr__ntune:5;
		uint64_t ptune_offset:4;
		uint64_t ntune_offset:4;
		uint64_t m180:1;
		uint64_t byp:1;
		uint64_t ptune:5;
		uint64_t ntune:5;
		uint64_t rodt_ctl:4;
		uint64_t control_ctl:4;
		uint64_t cmd_ctl:4;
		uint64_t ck_ctl:4;
		uint64_t dqx_ctl:4;
	} cn70xx;
	struct cvmx_lmcx_comp_ctl2_cn70xx cn70xxp1;
	struct cvmx_lmcx_comp_ctl2_cn70xx cn73xx;
	struct cvmx_lmcx_comp_ctl2_cn70xx cn78xx;
	struct cvmx_lmcx_comp_ctl2_cn70xx cn78xxp1;
	struct cvmx_lmcx_comp_ctl2_cn61xx cnf71xx;
	struct cvmx_lmcx_comp_ctl2_cn70xx cnf75xx;
};

/**
 * cvmx_lmc#_config
 *
 * This register controls certain parameters required for memory configuration.
 * Note the following:
 * * Priority order for hardware write operations to
 * LMC()_CONFIG/LMC()_FADR/LMC()_ECC_SYND: DED error > SEC error.
 * * The self-refresh entry sequence(s) power the DLL up/down (depending on
 * LMC()_MODEREG_PARAMS0[DLL]) when LMC()_CONFIG[SREF_WITH_DLL] is set.
 * * Prior to the self-refresh exit sequence, LMC()_MODEREG_PARAMS0 should
 * be reprogrammed
 * (if needed) to the appropriate values.
 *
 * See LMC initialization sequence for the LMC bringup sequence.
 */
union cvmx_lmcx_config {
	u64 u64;
	struct cvmx_lmcx_config_s {
		uint64_t lrdimm_ena:1;
		uint64_t bg2_enable:1;
		uint64_t mode_x4dev:1;
		uint64_t mode32b:1;
		uint64_t scrz:1;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t reserved_18_39:22;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} s;
	struct cvmx_lmcx_config_cn61xx {
		uint64_t reserved_61_63:3;
		uint64_t mode32b:1;
		uint64_t scrz:1;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t sequence:3;
		uint64_t ref_zqcs_int:19;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} cn61xx;
	struct cvmx_lmcx_config_cn63xx {
		uint64_t reserved_59_63:5;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t sequence:3;
		uint64_t ref_zqcs_int:19;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} cn63xx;
	struct cvmx_lmcx_config_cn63xxp1 {
		uint64_t reserved_55_63:9;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t sequence:3;
		uint64_t ref_zqcs_int:19;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} cn63xxp1;
	struct cvmx_lmcx_config_cn66xx {
		uint64_t reserved_60_63:4;
		uint64_t scrz:1;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t sequence:3;
		uint64_t ref_zqcs_int:19;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} cn66xx;
	struct cvmx_lmcx_config_cn63xx cn68xx;
	struct cvmx_lmcx_config_cn63xx cn68xxp1;
	struct cvmx_lmcx_config_cn70xx {
		uint64_t reserved_63_63:1;
		uint64_t bg2_enable:1;
		uint64_t mode_x4dev:1;
		uint64_t mode32b:1;
		uint64_t scrz:1;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t ref_zqcs_int:22;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t reserved_0_0:1;
	} cn70xx;
	struct cvmx_lmcx_config_cn70xx cn70xxp1;
	struct cvmx_lmcx_config_cn73xx {
		uint64_t lrdimm_ena:1;
		uint64_t bg2_enable:1;
		uint64_t mode_x4dev:1;
		uint64_t mode32b:1;
		uint64_t scrz:1;
		uint64_t early_unload_d1_r1:1;
		uint64_t early_unload_d1_r0:1;
		uint64_t early_unload_d0_r1:1;
		uint64_t early_unload_d0_r0:1;
		uint64_t init_status:4;
		uint64_t mirrmask:4;
		uint64_t rankmask:4;
		uint64_t rank_ena:1;
		uint64_t sref_with_dll:1;
		uint64_t early_dqx:1;
		uint64_t ref_zqcs_int:22;
		uint64_t reset:1;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t reserved_0_0:1;
	} cn73xx;
	struct cvmx_lmcx_config_cn73xx cn78xx;
	struct cvmx_lmcx_config_cn73xx cn78xxp1;
	struct cvmx_lmcx_config_cn61xx cnf71xx;
	struct cvmx_lmcx_config_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_control
 *
 * LMC_CONTROL = LMC Control
 * This register is an assortment of various control fields needed by the
 * memory controller
 */
union cvmx_lmcx_control {
	u64 u64;
	struct cvmx_lmcx_control_s {
		uint64_t scramble_ena:1;
		uint64_t thrcnt:12;
		uint64_t persub:8;
		uint64_t thrmax:4;
		uint64_t crm_cnt:5;
		uint64_t crm_thr:5;
		uint64_t crm_max:5;
		uint64_t rodt_bprch:1;
		uint64_t wodt_bprch:1;
		uint64_t bprch:2;
		uint64_t ext_zqcs_dis:1;
		uint64_t int_zqcs_dis:1;
		uint64_t auto_dclkdis:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t nxm_write_en:1;
		uint64_t elev_prio_dis:1;
		uint64_t inorder_wr:1;
		uint64_t inorder_rd:1;
		uint64_t throttle_wr:1;
		uint64_t throttle_rd:1;
		uint64_t fprch2:2;
		uint64_t pocas:1;
		uint64_t ddr2t:1;
		uint64_t bwcnt:1;
		uint64_t rdimm_ena:1;
	} s;
	struct cvmx_lmcx_control_s cn61xx;
	struct cvmx_lmcx_control_cn63xx {
		uint64_t reserved_24_63:40;
		uint64_t rodt_bprch:1;
		uint64_t wodt_bprch:1;
		uint64_t bprch:2;
		uint64_t ext_zqcs_dis:1;
		uint64_t int_zqcs_dis:1;
		uint64_t auto_dclkdis:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t nxm_write_en:1;
		uint64_t elev_prio_dis:1;
		uint64_t inorder_wr:1;
		uint64_t inorder_rd:1;
		uint64_t throttle_wr:1;
		uint64_t throttle_rd:1;
		uint64_t fprch2:2;
		uint64_t pocas:1;
		uint64_t ddr2t:1;
		uint64_t bwcnt:1;
		uint64_t rdimm_ena:1;
	} cn63xx;
	struct cvmx_lmcx_control_cn63xx cn63xxp1;
	struct cvmx_lmcx_control_cn66xx {
		uint64_t scramble_ena:1;
		uint64_t reserved_24_62:39;
		uint64_t rodt_bprch:1;
		uint64_t wodt_bprch:1;
		uint64_t bprch:2;
		uint64_t ext_zqcs_dis:1;
		uint64_t int_zqcs_dis:1;
		uint64_t auto_dclkdis:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t nxm_write_en:1;
		uint64_t elev_prio_dis:1;
		uint64_t inorder_wr:1;
		uint64_t inorder_rd:1;
		uint64_t throttle_wr:1;
		uint64_t throttle_rd:1;
		uint64_t fprch2:2;
		uint64_t pocas:1;
		uint64_t ddr2t:1;
		uint64_t bwcnt:1;
		uint64_t rdimm_ena:1;
	} cn66xx;
	struct cvmx_lmcx_control_cn68xx {
		uint64_t reserved_63_63:1;
		uint64_t thrcnt:12;
		uint64_t persub:8;
		uint64_t thrmax:4;
		uint64_t crm_cnt:5;
		uint64_t crm_thr:5;
		uint64_t crm_max:5;
		uint64_t rodt_bprch:1;
		uint64_t wodt_bprch:1;
		uint64_t bprch:2;
		uint64_t ext_zqcs_dis:1;
		uint64_t int_zqcs_dis:1;
		uint64_t auto_dclkdis:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t nxm_write_en:1;
		uint64_t elev_prio_dis:1;
		uint64_t inorder_wr:1;
		uint64_t inorder_rd:1;
		uint64_t throttle_wr:1;
		uint64_t throttle_rd:1;
		uint64_t fprch2:2;
		uint64_t pocas:1;
		uint64_t ddr2t:1;
		uint64_t bwcnt:1;
		uint64_t rdimm_ena:1;
	} cn68xx;
	struct cvmx_lmcx_control_cn68xx cn68xxp1;
	struct cvmx_lmcx_control_s cn70xx;
	struct cvmx_lmcx_control_s cn70xxp1;
	struct cvmx_lmcx_control_s cn73xx;
	struct cvmx_lmcx_control_s cn78xx;
	struct cvmx_lmcx_control_s cn78xxp1;
	struct cvmx_lmcx_control_cn66xx cnf71xx;
	struct cvmx_lmcx_control_s cnf75xx;
};

/**
 * cvmx_lmc#_ctl
 *
 * LMC_CTL = LMC Control
 * This register is an assortment of various control fields needed by the
 * memory controller
 */
union cvmx_lmcx_ctl {
	u64 u64;
	struct cvmx_lmcx_ctl_s {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t pll_div2:1;
		uint64_t pll_bypass:1;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t reserved_10_11:2;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} s;
	struct cvmx_lmcx_ctl_cn30xx {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t pll_div2:1;
		uint64_t pll_bypass:1;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t dreset:1;
		uint64_t mode32b:1;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} cn30xx;
	struct cvmx_lmcx_ctl_cn30xx cn31xx;
	struct cvmx_lmcx_ctl_cn38xx {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t reserved_16_17:2;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t set_zero:1;
		uint64_t mode128b:1;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} cn38xx;
	struct cvmx_lmcx_ctl_cn38xx cn38xxp2;
	struct cvmx_lmcx_ctl_cn50xx {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t reserved_17_17:1;
		uint64_t pll_bypass:1;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t dreset:1;
		uint64_t mode32b:1;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} cn50xx;
	struct cvmx_lmcx_ctl_cn52xx {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t reserved_16_17:2;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t dreset:1;
		uint64_t mode32b:1;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} cn52xx;
	struct cvmx_lmcx_ctl_cn52xx cn52xxp1;
	struct cvmx_lmcx_ctl_cn52xx cn56xx;
	struct cvmx_lmcx_ctl_cn52xx cn56xxp1;
	struct cvmx_lmcx_ctl_cn58xx {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:4;
		uint64_t ddr__pctl:4;
		uint64_t slow_scf:1;
		uint64_t xor_bank:1;
		uint64_t max_write_batch:4;
		uint64_t reserved_16_17:2;
		uint64_t rdimm_ena:1;
		uint64_t r2r_slot:1;
		uint64_t inorder_mwf:1;
		uint64_t inorder_mrf:1;
		uint64_t dreset:1;
		uint64_t mode128b:1;
		uint64_t fprch2:1;
		uint64_t bprch:1;
		uint64_t sil_lat:2;
		uint64_t tskw:2;
		uint64_t qs_dic:2;
		uint64_t dic:2;
	} cn58xx;
	struct cvmx_lmcx_ctl_cn58xx cn58xxp1;
};

/**
 * cvmx_lmc#_ctl1
 *
 * LMC_CTL1 = LMC Control1
 * This register is an assortment of various control fields needed by the
 * memory controller
 */
union cvmx_lmcx_ctl1 {
	u64 u64;
	struct cvmx_lmcx_ctl1_s {
		uint64_t reserved_21_63:43;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t sequence:3;
		uint64_t sil_mode:1;
		uint64_t dcc_enable:1;
		uint64_t reserved_2_7:6;
		uint64_t data_layout:2;
	} s;
	struct cvmx_lmcx_ctl1_cn30xx {
		uint64_t reserved_2_63:62;
		uint64_t data_layout:2;
	} cn30xx;
	struct cvmx_lmcx_ctl1_cn50xx {
		uint64_t reserved_10_63:54;
		uint64_t sil_mode:1;
		uint64_t dcc_enable:1;
		uint64_t reserved_2_7:6;
		uint64_t data_layout:2;
	} cn50xx;
	struct cvmx_lmcx_ctl1_cn52xx {
		uint64_t reserved_21_63:43;
		uint64_t ecc_adr:1;
		uint64_t forcewrite:4;
		uint64_t idlepower:3;
		uint64_t sequence:3;
		uint64_t sil_mode:1;
		uint64_t dcc_enable:1;
		uint64_t reserved_0_7:8;
	} cn52xx;
	struct cvmx_lmcx_ctl1_cn52xx cn52xxp1;
	struct cvmx_lmcx_ctl1_cn52xx cn56xx;
	struct cvmx_lmcx_ctl1_cn52xx cn56xxp1;
	struct cvmx_lmcx_ctl1_cn58xx {
		uint64_t reserved_10_63:54;
		uint64_t sil_mode:1;
		uint64_t dcc_enable:1;
		uint64_t reserved_0_7:8;
	} cn58xx;
	struct cvmx_lmcx_ctl1_cn58xx cn58xxp1;
};

/**
 * cvmx_lmc#_dbtrain_ctl
 *
 * Reserved.
 *
 */
union cvmx_lmcx_dbtrain_ctl {
	u64 u64;
	struct cvmx_lmcx_dbtrain_ctl_s {
		uint64_t reserved_63_63:1;
		uint64_t lfsr_pattern_sel:1;
		uint64_t cmd_count_ext:2;
		uint64_t db_output_impedance:3;
		uint64_t db_sel:1;
		uint64_t tccd_sel:1;
		uint64_t rw_train:1;
		uint64_t read_dq_count:7;
		uint64_t read_cmd_count:5;
		uint64_t write_ena:1;
		uint64_t activate:1;
		uint64_t prank:2;
		uint64_t lrank:3;
		uint64_t row_a:18;
		uint64_t bg:2;
		uint64_t ba:2;
		uint64_t column_a:13;
	} s;
	struct cvmx_lmcx_dbtrain_ctl_cn73xx {
		uint64_t reserved_60_63:4;
		uint64_t db_output_impedance:3;
		uint64_t db_sel:1;
		uint64_t tccd_sel:1;
		uint64_t rw_train:1;
		uint64_t read_dq_count:7;
		uint64_t read_cmd_count:5;
		uint64_t write_ena:1;
		uint64_t activate:1;
		uint64_t prank:2;
		uint64_t lrank:3;
		uint64_t row_a:18;
		uint64_t bg:2;
		uint64_t ba:2;
		uint64_t column_a:13;
	} cn73xx;
	struct cvmx_lmcx_dbtrain_ctl_s cn78xx;
	struct cvmx_lmcx_dbtrain_ctl_cnf75xx {
		uint64_t reserved_62_63:2;
		uint64_t cmd_count_ext:2;
		uint64_t db_output_impedance:3;
		uint64_t db_sel:1;
		uint64_t tccd_sel:1;
		uint64_t rw_train:1;
		uint64_t read_dq_count:7;
		uint64_t read_cmd_count:5;
		uint64_t write_ena:1;
		uint64_t activate:1;
		uint64_t prank:2;
		uint64_t lrank:3;
		uint64_t row_a:18;
		uint64_t bg:2;
		uint64_t ba:2;
		uint64_t column_a:13;
	} cnf75xx;
};

/**
 * cvmx_lmc#_dclk_cnt
 *
 * LMC_DCLK_CNT  = Performance Counters
 *
 */
union cvmx_lmcx_dclk_cnt {
	u64 u64;
	struct cvmx_lmcx_dclk_cnt_s {
		uint64_t dclkcnt:64;
	} s;
	struct cvmx_lmcx_dclk_cnt_s cn61xx;
	struct cvmx_lmcx_dclk_cnt_s cn63xx;
	struct cvmx_lmcx_dclk_cnt_s cn63xxp1;
	struct cvmx_lmcx_dclk_cnt_s cn66xx;
	struct cvmx_lmcx_dclk_cnt_s cn68xx;
	struct cvmx_lmcx_dclk_cnt_s cn68xxp1;
	struct cvmx_lmcx_dclk_cnt_s cn70xx;
	struct cvmx_lmcx_dclk_cnt_s cn70xxp1;
	struct cvmx_lmcx_dclk_cnt_s cn73xx;
	struct cvmx_lmcx_dclk_cnt_s cn78xx;
	struct cvmx_lmcx_dclk_cnt_s cn78xxp1;
	struct cvmx_lmcx_dclk_cnt_s cnf71xx;
	struct cvmx_lmcx_dclk_cnt_s cnf75xx;
};

/**
 * cvmx_lmc#_dclk_cnt_hi
 *
 * LMC_DCLK_CNT_HI  = Performance Counters
 *
 */
union cvmx_lmcx_dclk_cnt_hi {
	u64 u64;
	struct cvmx_lmcx_dclk_cnt_hi_s {
		uint64_t reserved_32_63:32;
		uint64_t dclkcnt_hi:32;
	} s;
	struct cvmx_lmcx_dclk_cnt_hi_s cn30xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn31xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn38xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn38xxp2;
	struct cvmx_lmcx_dclk_cnt_hi_s cn50xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn52xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn52xxp1;
	struct cvmx_lmcx_dclk_cnt_hi_s cn56xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn56xxp1;
	struct cvmx_lmcx_dclk_cnt_hi_s cn58xx;
	struct cvmx_lmcx_dclk_cnt_hi_s cn58xxp1;
};

/**
 * cvmx_lmc#_dclk_cnt_lo
 *
 * LMC_DCLK_CNT_LO  = Performance Counters
 *
 */
union cvmx_lmcx_dclk_cnt_lo {
	u64 u64;
	struct cvmx_lmcx_dclk_cnt_lo_s {
		uint64_t reserved_32_63:32;
		uint64_t dclkcnt_lo:32;
	} s;
	struct cvmx_lmcx_dclk_cnt_lo_s cn30xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn31xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn38xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn38xxp2;
	struct cvmx_lmcx_dclk_cnt_lo_s cn50xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn52xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn52xxp1;
	struct cvmx_lmcx_dclk_cnt_lo_s cn56xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn56xxp1;
	struct cvmx_lmcx_dclk_cnt_lo_s cn58xx;
	struct cvmx_lmcx_dclk_cnt_lo_s cn58xxp1;
};

/**
 * cvmx_lmc#_dclk_ctl
 *
 * LMC_DCLK_CTL = LMC DCLK generation control
 *
 *
 * Notes:
 * This CSR is only relevant for LMC1. LMC0_DCLK_CTL is not used.
 *
 */
union cvmx_lmcx_dclk_ctl {
	u64 u64;
	struct cvmx_lmcx_dclk_ctl_s {
		uint64_t reserved_8_63:56;
		uint64_t off90_ena:1;
		uint64_t dclk90_byp:1;
		uint64_t dclk90_ld:1;
		uint64_t dclk90_vlu:5;
	} s;
	struct cvmx_lmcx_dclk_ctl_s cn56xx;
	struct cvmx_lmcx_dclk_ctl_s cn56xxp1;
};

/**
 * cvmx_lmc#_ddr2_ctl
 *
 * LMC_DDR2_CTL = LMC DDR2 & DLL Control Register
 *
 */
union cvmx_lmcx_ddr2_ctl {
	u64 u64;
	struct cvmx_lmcx_ddr2_ctl_s {
		uint64_t reserved_32_63:32;
		uint64_t bank8:1;
		uint64_t burst8:1;
		uint64_t addlat:3;
		uint64_t pocas:1;
		uint64_t bwcnt:1;
		uint64_t twr:3;
		uint64_t silo_hc:1;
		uint64_t ddr_eof:4;
		uint64_t tfaw:5;
		uint64_t crip_mode:1;
		uint64_t ddr2t:1;
		uint64_t odt_ena:1;
		uint64_t qdll_ena:1;
		uint64_t dll90_vlu:5;
		uint64_t dll90_byp:1;
		uint64_t rdqs:1;
		uint64_t ddr2:1;
	} s;
	struct cvmx_lmcx_ddr2_ctl_cn30xx {
		uint64_t reserved_32_63:32;
		uint64_t bank8:1;
		uint64_t burst8:1;
		uint64_t addlat:3;
		uint64_t pocas:1;
		uint64_t bwcnt:1;
		uint64_t twr:3;
		uint64_t silo_hc:1;
		uint64_t ddr_eof:4;
		uint64_t tfaw:5;
		uint64_t crip_mode:1;
		uint64_t ddr2t:1;
		uint64_t odt_ena:1;
		uint64_t qdll_ena:1;
		uint64_t dll90_vlu:5;
		uint64_t dll90_byp:1;
		uint64_t reserved_1_1:1;
		uint64_t ddr2:1;
	} cn30xx;
	struct cvmx_lmcx_ddr2_ctl_cn30xx cn31xx;
	struct cvmx_lmcx_ddr2_ctl_s cn38xx;
	struct cvmx_lmcx_ddr2_ctl_s cn38xxp2;
	struct cvmx_lmcx_ddr2_ctl_s cn50xx;
	struct cvmx_lmcx_ddr2_ctl_s cn52xx;
	struct cvmx_lmcx_ddr2_ctl_s cn52xxp1;
	struct cvmx_lmcx_ddr2_ctl_s cn56xx;
	struct cvmx_lmcx_ddr2_ctl_s cn56xxp1;
	struct cvmx_lmcx_ddr2_ctl_s cn58xx;
	struct cvmx_lmcx_ddr2_ctl_s cn58xxp1;
};

/**
 * cvmx_lmc#_ddr4_dimm_ctl
 *
 * Bits 0-21 of this register are used only when LMC()_CONTROL[RDIMM_ENA] = 1.
 *
 * During an RCW initialization sequence, bits 0-21 control LMC's write
 * operations to the extended DDR4 control words in the JEDEC standard
 * registering clock driver on an RDIMM.
 */
union cvmx_lmcx_ddr4_dimm_ctl {
	u64 u64;
	struct cvmx_lmcx_ddr4_dimm_ctl_s {
		uint64_t reserved_28_63:36;
		uint64_t rank_timing_enable:1;
		uint64_t bodt_trans_mode:1;
		uint64_t trans_mode_ena:1;
		uint64_t read_preamble_mode:1;
		uint64_t buff_config_da3:1;
		uint64_t mpr_over_ena:1;
		uint64_t ddr4_dimm1_wmask:11;
		uint64_t ddr4_dimm0_wmask:11;
	} s;
	struct cvmx_lmcx_ddr4_dimm_ctl_cn70xx {
		uint64_t reserved_22_63:42;
		uint64_t ddr4_dimm1_wmask:11;
		uint64_t ddr4_dimm0_wmask:11;
	} cn70xx;
	struct cvmx_lmcx_ddr4_dimm_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_ddr4_dimm_ctl_s cn73xx;
	struct cvmx_lmcx_ddr4_dimm_ctl_s cn78xx;
	struct cvmx_lmcx_ddr4_dimm_ctl_s cn78xxp1;
	struct cvmx_lmcx_ddr4_dimm_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_ddr_pll_ctl
 *
 * This register controls the DDR_CK frequency. For details, refer to CK
 * speed programming. See LMC initialization sequence for the initialization
 * sequence.
 * DDR PLL bringup sequence:
 *
 * 1. Write [CLKF], [CLKR], [DDR_PS_EN].
 *
 * 2. Wait 128 ref clock cycles (7680 core-clock cycles).
 *
 * 3. Write 1 to [RESET_N].
 *
 * 4. Wait 1152 ref clocks (1152*16 core-clock cycles).
 *
 * 5. Write 0 to [DDR_DIV_RESET].
 *
 * 6. Wait 10 ref clock cycles (160 core-clock cycles) before bringing up
 * the DDR interface.
 */
union cvmx_lmcx_ddr_pll_ctl {
	u64 u64;
	struct cvmx_lmcx_ddr_pll_ctl_s {
		uint64_t reserved_45_63:19;
		uint64_t dclk_alt_refclk_sel:1;
		uint64_t bwadj:12;
		uint64_t dclk_invert:1;
		uint64_t phy_dcok:1;
		uint64_t ddr4_mode:1;
		uint64_t pll_fbslip:1;
		uint64_t pll_lock:1;
		uint64_t reserved_18_26:9;
		uint64_t diffamp:4;
		uint64_t cps:3;
		uint64_t reserved_8_10:3;
		uint64_t reset_n:1;
		uint64_t clkf:7;
	} s;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx {
		uint64_t reserved_27_63:37;
		uint64_t jtg_test_mode:1;
		uint64_t dfm_div_reset:1;
		uint64_t dfm_ps_en:3;
		uint64_t ddr_div_reset:1;
		uint64_t ddr_ps_en:3;
		uint64_t diffamp:4;
		uint64_t cps:3;
		uint64_t cpb:3;
		uint64_t reset_n:1;
		uint64_t clkf:7;
	} cn61xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cn63xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cn63xxp1;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cn66xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cn68xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cn68xxp1;
	struct cvmx_lmcx_ddr_pll_ctl_cn70xx {
		uint64_t reserved_31_63:33;
		uint64_t phy_dcok:1;
		uint64_t ddr4_mode:1;
		uint64_t pll_fbslip:1;
		uint64_t pll_lock:1;
		uint64_t pll_rfslip:1;
		uint64_t clkr:2;
		uint64_t jtg_test_mode:1;
		uint64_t ddr_div_reset:1;
		uint64_t ddr_ps_en:4;
		uint64_t reserved_8_17:10;
		uint64_t reset_n:1;
		uint64_t clkf:7;
	} cn70xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_ddr_pll_ctl_cn73xx {
		uint64_t reserved_45_63:19;
		uint64_t dclk_alt_refclk_sel:1;
		uint64_t bwadj:12;
		uint64_t dclk_invert:1;
		uint64_t phy_dcok:1;
		uint64_t ddr4_mode:1;
		uint64_t pll_fbslip:1;
		uint64_t pll_lock:1;
		uint64_t pll_rfslip:1;
		uint64_t clkr:2;
		uint64_t jtg_test_mode:1;
		uint64_t ddr_div_reset:1;
		uint64_t ddr_ps_en:4;
		uint64_t reserved_9_17:9;
		uint64_t clkf_ext:1;
		uint64_t reset_n:1;
		uint64_t clkf:7;
	} cn73xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn73xx cn78xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn73xx cn78xxp1;
	struct cvmx_lmcx_ddr_pll_ctl_cn61xx cnf71xx;
	struct cvmx_lmcx_ddr_pll_ctl_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_delay_cfg
 *
 * LMC_DELAY_CFG = Open-loop delay line settings
 *
 *
 * Notes:
 * The DQ bits add OUTGOING delay only to dq, dqs_[p,n], cb, cbs_[p,n], dqm.
 * Delay is approximately 50-80ps per setting depending on process/voltage.
 * There is no need to add incoming delay since by default all strobe bits
 * are delayed internally by 90 degrees (as was always the case in previous
 * passes and past chips.
 *
 * The CMD add delay to all command bits DDR_RAS, DDR_CAS, DDR_A<15:0>,
 * DDR_BA<2:0>, DDR_n_CS<1:0>_L, DDR_WE, DDR_CKE and DDR_ODT_<7:0>.
 * Again, delay is 50-80ps per tap.
 *
 * The CLK bits add delay to all clock signals DDR_CK_<5:0>_P and
 * DDR_CK_<5:0>_N.  Again, delay is 50-80ps per tap.
 *
 * The usage scenario is the following: There is too much delay on command
 * signals and setup on command is not met. The user can then delay the
 * clock until setup is met.
 *
 * At the same time though, dq/dqs should be delayed because there is also
 * a DDR spec tying dqs with clock. If clock is too much delayed with
 * respect to dqs, writes will start to fail.
 *
 * This scheme should eliminate the board need of adding routing delay to
 * clock signals to make high frequencies work.
 */
union cvmx_lmcx_delay_cfg {
	u64 u64;
	struct cvmx_lmcx_delay_cfg_s {
		uint64_t reserved_15_63:49;
		uint64_t dq:5;
		uint64_t cmd:5;
		uint64_t clk:5;
	} s;
	struct cvmx_lmcx_delay_cfg_s cn30xx;
	struct cvmx_lmcx_delay_cfg_cn38xx {
		uint64_t reserved_14_63:50;
		uint64_t dq:4;
		uint64_t reserved_9_9:1;
		uint64_t cmd:4;
		uint64_t reserved_4_4:1;
		uint64_t clk:4;
	} cn38xx;
	struct cvmx_lmcx_delay_cfg_cn38xx cn50xx;
	struct cvmx_lmcx_delay_cfg_cn38xx cn52xx;
	struct cvmx_lmcx_delay_cfg_cn38xx cn52xxp1;
	struct cvmx_lmcx_delay_cfg_cn38xx cn56xx;
	struct cvmx_lmcx_delay_cfg_cn38xx cn56xxp1;
	struct cvmx_lmcx_delay_cfg_cn38xx cn58xx;
	struct cvmx_lmcx_delay_cfg_cn38xx cn58xxp1;
};

/**
 * cvmx_lmc#_dimm#_ddr4_params0
 *
 * This register contains values to be programmed into the extra DDR4 control
 * words in the corresponding (registered) DIMM. These are control words
 * RC1x through RC8x.
 */
union cvmx_lmcx_dimmx_ddr4_params0 {
	u64 u64;
	struct cvmx_lmcx_dimmx_ddr4_params0_s {
		uint64_t rc8x:8;
		uint64_t rc7x:8;
		uint64_t rc6x:8;
		uint64_t rc5x:8;
		uint64_t rc4x:8;
		uint64_t rc3x:8;
		uint64_t rc2x:8;
		uint64_t rc1x:8;
	} s;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cn70xx;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cn70xxp1;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cn73xx;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cn78xx;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cn78xxp1;
	struct cvmx_lmcx_dimmx_ddr4_params0_s cnf75xx;
};

/**
 * cvmx_lmc#_dimm#_ddr4_params1
 *
 * This register contains values to be programmed into the extra DDR4 control
 * words in the corresponding (registered) DIMM. These are control words
 * RC9x through RCBx.
 */
union cvmx_lmcx_dimmx_ddr4_params1 {
	u64 u64;
	struct cvmx_lmcx_dimmx_ddr4_params1_s {
		uint64_t reserved_24_63:40;
		uint64_t rcbx:8;
		uint64_t rcax:8;
		uint64_t rc9x:8;
	} s;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cn70xx;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cn70xxp1;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cn73xx;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cn78xx;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cn78xxp1;
	struct cvmx_lmcx_dimmx_ddr4_params1_s cnf75xx;
};

/**
 * cvmx_lmc#_dimm#_params
 *
 * This register contains values to be programmed into each control word in
 * the corresponding (registered) DIMM. The control words allow optimization
 * of the device properties for different raw card designs. Note that LMC
 * only uses this CSR when LMC()_CONTROL[RDIMM_ENA]=1. During a power-up/init
 * sequence, LMC writes these fields into the control words in the JEDEC
 * standard DDR3 SSTE32882 registering clock driver or DDR4 Register
 * DDR4RCD01 on an RDIMM when corresponding LMC()_DIMM_CTL[DIMM*_WMASK]
 * bits are set.
 */
union cvmx_lmcx_dimmx_params {
	u64 u64;
	struct cvmx_lmcx_dimmx_params_s {
		uint64_t rc15:4;
		uint64_t rc14:4;
		uint64_t rc13:4;
		uint64_t rc12:4;
		uint64_t rc11:4;
		uint64_t rc10:4;
		uint64_t rc9:4;
		uint64_t rc8:4;
		uint64_t rc7:4;
		uint64_t rc6:4;
		uint64_t rc5:4;
		uint64_t rc4:4;
		uint64_t rc3:4;
		uint64_t rc2:4;
		uint64_t rc1:4;
		uint64_t rc0:4;
	} s;
	struct cvmx_lmcx_dimmx_params_s cn61xx;
	struct cvmx_lmcx_dimmx_params_s cn63xx;
	struct cvmx_lmcx_dimmx_params_s cn63xxp1;
	struct cvmx_lmcx_dimmx_params_s cn66xx;
	struct cvmx_lmcx_dimmx_params_s cn68xx;
	struct cvmx_lmcx_dimmx_params_s cn68xxp1;
	struct cvmx_lmcx_dimmx_params_s cn70xx;
	struct cvmx_lmcx_dimmx_params_s cn70xxp1;
	struct cvmx_lmcx_dimmx_params_s cn73xx;
	struct cvmx_lmcx_dimmx_params_s cn78xx;
	struct cvmx_lmcx_dimmx_params_s cn78xxp1;
	struct cvmx_lmcx_dimmx_params_s cnf71xx;
	struct cvmx_lmcx_dimmx_params_s cnf75xx;
};

/**
 * cvmx_lmc#_dimm_ctl
 *
 * Note that this CSR is only used when LMC()_CONTROL[RDIMM_ENA] = 1. During
 * a power-up/init sequence, this CSR controls LMC's write operations to the
 * control words in the JEDEC standard DDR3 SSTE32882 registering clock
 * driver or DDR4 Register DDR4RCD01 on an RDIMM.
 */
union cvmx_lmcx_dimm_ctl {
	u64 u64;
	struct cvmx_lmcx_dimm_ctl_s {
		uint64_t reserved_46_63:18;
		uint64_t parity:1;
		uint64_t tcws:13;
		uint64_t dimm1_wmask:16;
		uint64_t dimm0_wmask:16;
	} s;
	struct cvmx_lmcx_dimm_ctl_s cn61xx;
	struct cvmx_lmcx_dimm_ctl_s cn63xx;
	struct cvmx_lmcx_dimm_ctl_s cn63xxp1;
	struct cvmx_lmcx_dimm_ctl_s cn66xx;
	struct cvmx_lmcx_dimm_ctl_s cn68xx;
	struct cvmx_lmcx_dimm_ctl_s cn68xxp1;
	struct cvmx_lmcx_dimm_ctl_s cn70xx;
	struct cvmx_lmcx_dimm_ctl_s cn70xxp1;
	struct cvmx_lmcx_dimm_ctl_s cn73xx;
	struct cvmx_lmcx_dimm_ctl_s cn78xx;
	struct cvmx_lmcx_dimm_ctl_s cn78xxp1;
	struct cvmx_lmcx_dimm_ctl_s cnf71xx;
	struct cvmx_lmcx_dimm_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_dll_ctl
 *
 * LMC_DLL_CTL = LMC DLL control and DCLK reset
 *
 */
union cvmx_lmcx_dll_ctl {
	u64 u64;
	struct cvmx_lmcx_dll_ctl_s {
		uint64_t reserved_8_63:56;
		uint64_t dreset:1;
		uint64_t dll90_byp:1;
		uint64_t dll90_ena:1;
		uint64_t dll90_vlu:5;
	} s;
	struct cvmx_lmcx_dll_ctl_s cn52xx;
	struct cvmx_lmcx_dll_ctl_s cn52xxp1;
	struct cvmx_lmcx_dll_ctl_s cn56xx;
	struct cvmx_lmcx_dll_ctl_s cn56xxp1;
};

/**
 * cvmx_lmc#_dll_ctl2
 *
 * See LMC initialization sequence for the initialization sequence.
 *
 */
union cvmx_lmcx_dll_ctl2 {
	u64 u64;
	struct cvmx_lmcx_dll_ctl2_s {
		uint64_t reserved_0_63:64;
	} s;
	struct cvmx_lmcx_dll_ctl2_cn61xx {
		uint64_t reserved_16_63:48;
		uint64_t intf_en:1;
		uint64_t dll_bringup:1;
		uint64_t dreset:1;
		uint64_t quad_dll_ena:1;
		uint64_t byp_sel:4;
		uint64_t byp_setting:8;
	} cn61xx;
	struct cvmx_lmcx_dll_ctl2_cn63xx {
		uint64_t reserved_15_63:49;
		uint64_t dll_bringup:1;
		uint64_t dreset:1;
		uint64_t quad_dll_ena:1;
		uint64_t byp_sel:4;
		uint64_t byp_setting:8;
	} cn63xx;
	struct cvmx_lmcx_dll_ctl2_cn63xx cn63xxp1;
	struct cvmx_lmcx_dll_ctl2_cn63xx cn66xx;
	struct cvmx_lmcx_dll_ctl2_cn61xx cn68xx;
	struct cvmx_lmcx_dll_ctl2_cn61xx cn68xxp1;
	struct cvmx_lmcx_dll_ctl2_cn70xx {
		uint64_t reserved_17_63:47;
		uint64_t intf_en:1;
		uint64_t dll_bringup:1;
		uint64_t dreset:1;
		uint64_t quad_dll_ena:1;
		uint64_t byp_sel:4;
		uint64_t byp_setting:9;
	} cn70xx;
	struct cvmx_lmcx_dll_ctl2_cn70xx cn70xxp1;
	struct cvmx_lmcx_dll_ctl2_cn70xx cn73xx;
	struct cvmx_lmcx_dll_ctl2_cn70xx cn78xx;
	struct cvmx_lmcx_dll_ctl2_cn70xx cn78xxp1;
	struct cvmx_lmcx_dll_ctl2_cn61xx cnf71xx;
	struct cvmx_lmcx_dll_ctl2_cn70xx cnf75xx;
};

/**
 * cvmx_lmc#_dll_ctl3
 *
 * LMC_DLL_CTL3 = LMC DLL control and DCLK reset
 *
 */
union cvmx_lmcx_dll_ctl3 {
	u64 u64;
	struct cvmx_lmcx_dll_ctl3_s {
		uint64_t reserved_50_63:14;
		uint64_t wr_deskew_ena:1;
		uint64_t wr_deskew_ld:1;
		uint64_t bit_select:4;
		uint64_t reserved_0_43:44;
	} s;
	struct cvmx_lmcx_dll_ctl3_cn61xx {
		uint64_t reserved_41_63:23;
		uint64_t dclk90_fwd:1;
		uint64_t ddr_90_dly_byp:1;
		uint64_t dclk90_recal_dis:1;
		uint64_t dclk90_byp_sel:1;
		uint64_t dclk90_byp_setting:8;
		uint64_t dll_fast:1;
		uint64_t dll90_setting:8;
		uint64_t fine_tune_mode:1;
		uint64_t dll_mode:1;
		uint64_t dll90_byte_sel:4;
		uint64_t offset_ena:1;
		uint64_t load_offset:1;
		uint64_t mode_sel:2;
		uint64_t byte_sel:4;
		uint64_t offset:6;
	} cn61xx;
	struct cvmx_lmcx_dll_ctl3_cn63xx {
		uint64_t reserved_29_63:35;
		uint64_t dll_fast:1;
		uint64_t dll90_setting:8;
		uint64_t fine_tune_mode:1;
		uint64_t dll_mode:1;
		uint64_t dll90_byte_sel:4;
		uint64_t offset_ena:1;
		uint64_t load_offset:1;
		uint64_t mode_sel:2;
		uint64_t byte_sel:4;
		uint64_t offset:6;
	} cn63xx;
	struct cvmx_lmcx_dll_ctl3_cn63xx cn63xxp1;
	struct cvmx_lmcx_dll_ctl3_cn63xx cn66xx;
	struct cvmx_lmcx_dll_ctl3_cn61xx cn68xx;
	struct cvmx_lmcx_dll_ctl3_cn61xx cn68xxp1;
	struct cvmx_lmcx_dll_ctl3_cn70xx {
		uint64_t reserved_44_63:20;
		uint64_t dclk90_fwd:1;
		uint64_t ddr_90_dly_byp:1;
		uint64_t dclk90_recal_dis:1;
		uint64_t dclk90_byp_sel:1;
		uint64_t dclk90_byp_setting:9;
		uint64_t dll_fast:1;
		uint64_t dll90_setting:9;
		uint64_t fine_tune_mode:1;
		uint64_t dll_mode:1;
		uint64_t dll90_byte_sel:4;
		uint64_t offset_ena:1;
		uint64_t load_offset:1;
		uint64_t mode_sel:2;
		uint64_t byte_sel:4;
		uint64_t offset:7;
	} cn70xx;
	struct cvmx_lmcx_dll_ctl3_cn70xx cn70xxp1;
	struct cvmx_lmcx_dll_ctl3_cn73xx {
		uint64_t reserved_50_63:14;
		uint64_t wr_deskew_ena:1;
		uint64_t wr_deskew_ld:1;
		uint64_t bit_select:4;
		uint64_t dclk90_fwd:1;
		uint64_t ddr_90_dly_byp:1;
		uint64_t dclk90_recal_dis:1;
		uint64_t dclk90_byp_sel:1;
		uint64_t dclk90_byp_setting:9;
		uint64_t dll_fast:1;
		uint64_t dll90_setting:9;
		uint64_t fine_tune_mode:1;
		uint64_t dll_mode:1;
		uint64_t dll90_byte_sel:4;
		uint64_t offset_ena:1;
		uint64_t load_offset:1;
		uint64_t mode_sel:2;
		uint64_t byte_sel:4;
		uint64_t offset:7;
	} cn73xx;
	struct cvmx_lmcx_dll_ctl3_cn73xx cn78xx;
	struct cvmx_lmcx_dll_ctl3_cn73xx cn78xxp1;
	struct cvmx_lmcx_dll_ctl3_cn61xx cnf71xx;
	struct cvmx_lmcx_dll_ctl3_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_dual_memcfg
 *
 * This register controls certain parameters of dual-memory configuration.
 *
 * This register enables the design to have two separate memory
 * configurations, selected dynamically by the reference address. Note
 * however, that both configurations share LMC()_CONTROL[XOR_BANK],
 * LMC()_CONFIG [PBANK_LSB], LMC()_CONFIG[RANK_ENA], and all timing parameters.
 *
 * In this description:
 * * config0 refers to the normal memory configuration that is defined by the
 * LMC()_CONFIG[ROW_LSB] parameter
 * * config1 refers to the dual (or second) memory configuration that is
 * defined by this register.
 */
union cvmx_lmcx_dual_memcfg {
	u64 u64;
	struct cvmx_lmcx_dual_memcfg_s {
		uint64_t reserved_20_63:44;
		uint64_t bank8:1;
		uint64_t row_lsb:3;
		uint64_t reserved_8_15:8;
		uint64_t cs_mask:8;
	} s;
	struct cvmx_lmcx_dual_memcfg_s cn50xx;
	struct cvmx_lmcx_dual_memcfg_s cn52xx;
	struct cvmx_lmcx_dual_memcfg_s cn52xxp1;
	struct cvmx_lmcx_dual_memcfg_s cn56xx;
	struct cvmx_lmcx_dual_memcfg_s cn56xxp1;
	struct cvmx_lmcx_dual_memcfg_s cn58xx;
	struct cvmx_lmcx_dual_memcfg_s cn58xxp1;
	struct cvmx_lmcx_dual_memcfg_cn61xx {
		uint64_t reserved_19_63:45;
		uint64_t row_lsb:3;
		uint64_t reserved_8_15:8;
		uint64_t cs_mask:8;
	} cn61xx;
	struct cvmx_lmcx_dual_memcfg_cn61xx cn63xx;
	struct cvmx_lmcx_dual_memcfg_cn61xx cn63xxp1;
	struct cvmx_lmcx_dual_memcfg_cn61xx cn66xx;
	struct cvmx_lmcx_dual_memcfg_cn61xx cn68xx;
	struct cvmx_lmcx_dual_memcfg_cn61xx cn68xxp1;
	struct cvmx_lmcx_dual_memcfg_cn70xx {
		uint64_t reserved_19_63:45;
		uint64_t row_lsb:3;
		uint64_t reserved_4_15:12;
		uint64_t cs_mask:4;
	} cn70xx;
	struct cvmx_lmcx_dual_memcfg_cn70xx cn70xxp1;
	struct cvmx_lmcx_dual_memcfg_cn70xx cn73xx;
	struct cvmx_lmcx_dual_memcfg_cn70xx cn78xx;
	struct cvmx_lmcx_dual_memcfg_cn70xx cn78xxp1;
	struct cvmx_lmcx_dual_memcfg_cn61xx cnf71xx;
	struct cvmx_lmcx_dual_memcfg_cn70xx cnf75xx;
};

/**
 * cvmx_lmc#_ecc_parity_test
 *
 * This register has bits to control the generation of ECC and command
 * address parity errors. ECC error is generated by enabling
 * [CA_PARITY_CORRUPT_ENA] and selecting any of the [ECC_CORRUPT_IDX]
 * index of the dataword from the cacheline to be corrupted.
 * User needs to select which bit of the 128-bit dataword to corrupt by
 * asserting any of the CHAR_MASK0 and CHAR_MASK2 bits. (CHAR_MASK0 and
 * CHAR_MASK2 corresponds to the lower and upper 64-bit signal that can
 * corrupt any individual bit of the data).
 *
 * Command address parity error is generated by enabling
 * [CA_PARITY_CORRUPT_ENA] and selecting the DDR command that the parity
 * is to be corrupted with through [CA_PARITY_SEL].
 */
union cvmx_lmcx_ecc_parity_test {
	u64 u64;
	struct cvmx_lmcx_ecc_parity_test_s {
		uint64_t reserved_12_63:52;
		uint64_t ecc_corrupt_ena:1;
		uint64_t ecc_corrupt_idx:3;
		uint64_t reserved_6_7:2;
		uint64_t ca_parity_corrupt_ena:1;
		uint64_t ca_parity_sel:5;
	} s;
	struct cvmx_lmcx_ecc_parity_test_s cn73xx;
	struct cvmx_lmcx_ecc_parity_test_s cn78xx;
	struct cvmx_lmcx_ecc_parity_test_s cn78xxp1;
	struct cvmx_lmcx_ecc_parity_test_s cnf75xx;
};

/**
 * cvmx_lmc#_ecc_synd
 *
 * LMC_ECC_SYND = MRD ECC Syndromes
 *
 */
union cvmx_lmcx_ecc_synd {
	u64 u64;
	struct cvmx_lmcx_ecc_synd_s {
		uint64_t reserved_32_63:32;
		uint64_t mrdsyn3:8;
		uint64_t mrdsyn2:8;
		uint64_t mrdsyn1:8;
		uint64_t mrdsyn0:8;
	} s;
	struct cvmx_lmcx_ecc_synd_s cn30xx;
	struct cvmx_lmcx_ecc_synd_s cn31xx;
	struct cvmx_lmcx_ecc_synd_s cn38xx;
	struct cvmx_lmcx_ecc_synd_s cn38xxp2;
	struct cvmx_lmcx_ecc_synd_s cn50xx;
	struct cvmx_lmcx_ecc_synd_s cn52xx;
	struct cvmx_lmcx_ecc_synd_s cn52xxp1;
	struct cvmx_lmcx_ecc_synd_s cn56xx;
	struct cvmx_lmcx_ecc_synd_s cn56xxp1;
	struct cvmx_lmcx_ecc_synd_s cn58xx;
	struct cvmx_lmcx_ecc_synd_s cn58xxp1;
	struct cvmx_lmcx_ecc_synd_s cn61xx;
	struct cvmx_lmcx_ecc_synd_s cn63xx;
	struct cvmx_lmcx_ecc_synd_s cn63xxp1;
	struct cvmx_lmcx_ecc_synd_s cn66xx;
	struct cvmx_lmcx_ecc_synd_s cn68xx;
	struct cvmx_lmcx_ecc_synd_s cn68xxp1;
	struct cvmx_lmcx_ecc_synd_s cn70xx;
	struct cvmx_lmcx_ecc_synd_s cn70xxp1;
	struct cvmx_lmcx_ecc_synd_s cn73xx;
	struct cvmx_lmcx_ecc_synd_s cn78xx;
	struct cvmx_lmcx_ecc_synd_s cn78xxp1;
	struct cvmx_lmcx_ecc_synd_s cnf71xx;
	struct cvmx_lmcx_ecc_synd_s cnf75xx;
};

/**
 * cvmx_lmc#_ext_config
 *
 * This register has additional configuration and control bits for the LMC.
 *
 */
union cvmx_lmcx_ext_config {
	u64 u64;
	struct cvmx_lmcx_ext_config_s {
		uint64_t reserved_61_63:3;
		uint64_t bc4_dqs_ena:1;
		uint64_t ref_block:1;
		uint64_t mrs_side:1;
		uint64_t mrs_one_side:1;
		uint64_t mrs_bside_invert_disable:1;
		uint64_t dimm_sel_invert_off:1;
		uint64_t dimm_sel_force_invert:1;
		uint64_t coalesce_address_mode:1;
		uint64_t dimm1_cid:2;
		uint64_t dimm0_cid:2;
		uint64_t rcd_parity_check:1;
		uint64_t reserved_46_47:2;
		uint64_t error_alert_n_sample:1;
		uint64_t ea_int_polarity:1;
		uint64_t reserved_43_43:1;
		uint64_t par_addr_mask:3;
		uint64_t reserved_38_39:2;
		uint64_t mrs_cmd_override:1;
		uint64_t mrs_cmd_select:1;
		uint64_t reserved_33_35:3;
		uint64_t invert_data:1;
		uint64_t reserved_30_31:2;
		uint64_t cmd_rti:1;
		uint64_t cal_ena:1;
		uint64_t reserved_27_27:1;
		uint64_t par_include_a17:1;
		uint64_t par_include_bg1:1;
		uint64_t gen_par:1;
		uint64_t reserved_21_23:3;
		uint64_t vrefint_seq_deskew:1;
		uint64_t read_ena_bprch:1;
		uint64_t read_ena_fprch:1;
		uint64_t slot_ctl_reset_force:1;
		uint64_t ref_int_lsbs:9;
		uint64_t drive_ena_bprch:1;
		uint64_t drive_ena_fprch:1;
		uint64_t dlcram_flip_synd:2;
		uint64_t dlcram_cor_dis:1;
		uint64_t dlc_nxm_rd:1;
		uint64_t l2c_nxm_rd:1;
		uint64_t l2c_nxm_wr:1;
	} s;
	struct cvmx_lmcx_ext_config_cn70xx {
		uint64_t reserved_21_63:43;
		uint64_t vrefint_seq_deskew:1;
		uint64_t read_ena_bprch:1;
		uint64_t read_ena_fprch:1;
		uint64_t slot_ctl_reset_force:1;
		uint64_t ref_int_lsbs:9;
		uint64_t drive_ena_bprch:1;
		uint64_t drive_ena_fprch:1;
		uint64_t dlcram_flip_synd:2;
		uint64_t dlcram_cor_dis:1;
		uint64_t dlc_nxm_rd:1;
		uint64_t l2c_nxm_rd:1;
		uint64_t l2c_nxm_wr:1;
	} cn70xx;
	struct cvmx_lmcx_ext_config_cn70xx cn70xxp1;
	struct cvmx_lmcx_ext_config_cn73xx {
		uint64_t reserved_60_63:4;
		uint64_t ref_block:1;
		uint64_t mrs_side:1;
		uint64_t mrs_one_side:1;
		uint64_t mrs_bside_invert_disable:1;
		uint64_t dimm_sel_invert_off:1;
		uint64_t dimm_sel_force_invert:1;
		uint64_t coalesce_address_mode:1;
		uint64_t dimm1_cid:2;
		uint64_t dimm0_cid:2;
		uint64_t rcd_parity_check:1;
		uint64_t reserved_46_47:2;
		uint64_t error_alert_n_sample:1;
		uint64_t ea_int_polarity:1;
		uint64_t reserved_43_43:1;
		uint64_t par_addr_mask:3;
		uint64_t reserved_38_39:2;
		uint64_t mrs_cmd_override:1;
		uint64_t mrs_cmd_select:1;
		uint64_t reserved_33_35:3;
		uint64_t invert_data:1;
		uint64_t reserved_30_31:2;
		uint64_t cmd_rti:1;
		uint64_t cal_ena:1;
		uint64_t reserved_27_27:1;
		uint64_t par_include_a17:1;
		uint64_t par_include_bg1:1;
		uint64_t gen_par:1;
		uint64_t reserved_21_23:3;
		uint64_t vrefint_seq_deskew:1;
		uint64_t read_ena_bprch:1;
		uint64_t read_ena_fprch:1;
		uint64_t slot_ctl_reset_force:1;
		uint64_t ref_int_lsbs:9;
		uint64_t drive_ena_bprch:1;
		uint64_t drive_ena_fprch:1;
		uint64_t dlcram_flip_synd:2;
		uint64_t dlcram_cor_dis:1;
		uint64_t dlc_nxm_rd:1;
		uint64_t l2c_nxm_rd:1;
		uint64_t l2c_nxm_wr:1;
	} cn73xx;
	struct cvmx_lmcx_ext_config_s cn78xx;
	struct cvmx_lmcx_ext_config_s cn78xxp1;
	struct cvmx_lmcx_ext_config_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_ext_config2
 *
 * This register has additional configuration and control bits for the LMC.
 *
 */
union cvmx_lmcx_ext_config2 {
	u64 u64;
	struct cvmx_lmcx_ext_config2_s {
		uint64_t reserved_27_63:37;
		uint64_t sref_auto_idle_thres:5;
		uint64_t sref_auto_enable:1;
		uint64_t delay_unload_r3:1;
		uint64_t delay_unload_r2:1;
		uint64_t delay_unload_r1:1;
		uint64_t delay_unload_r0:1;
		uint64_t early_dqx2:1;
		uint64_t xor_bank_sel:4;
		uint64_t reserved_10_11:2;
		uint64_t row_col_switch:1;
		uint64_t trr_on:1;
		uint64_t mac:3;
		uint64_t macram_scrub_done:1;
		uint64_t macram_scrub:1;
		uint64_t macram_flip_synd:2;
		uint64_t macram_cor_dis:1;
	} s;
	struct cvmx_lmcx_ext_config2_cn73xx {
		uint64_t reserved_10_63:54;
		uint64_t row_col_switch:1;
		uint64_t trr_on:1;
		uint64_t mac:3;
		uint64_t macram_scrub_done:1;
		uint64_t macram_scrub:1;
		uint64_t macram_flip_synd:2;
		uint64_t macram_cor_dis:1;
	} cn73xx;
	struct cvmx_lmcx_ext_config2_s cn78xx;
	struct cvmx_lmcx_ext_config2_cnf75xx {
		uint64_t reserved_21_63:43;
		uint64_t delay_unload_r3:1;
		uint64_t delay_unload_r2:1;
		uint64_t delay_unload_r1:1;
		uint64_t delay_unload_r0:1;
		uint64_t early_dqx2:1;
		uint64_t xor_bank_sel:4;
		uint64_t reserved_10_11:2;
		uint64_t row_col_switch:1;
		uint64_t trr_on:1;
		uint64_t mac:3;
		uint64_t macram_scrub_done:1;
		uint64_t macram_scrub:1;
		uint64_t macram_flip_synd:2;
		uint64_t macram_cor_dis:1;
	} cnf75xx;
};

/**
 * cvmx_lmc#_fadr
 *
 * This register only captures the first transaction with ECC errors. A DED
 * error can over-write this register with its failing addresses if the
 * first error was a SEC. If you write LMC()_INT -> SEC_ERR/DED_ERR, it
 * clears the error bits and captures the next failing address. If FDIMM
 * is 1, that means the error is in the high DIMM. LMC()_FADR captures the
 * failing pre-scrambled address location (split into DIMM, bunk, bank, etc).
 * If scrambling is off, then LMC()_FADR will also capture the failing
 * physical location in the DRAM parts. LMC()_SCRAMBLED_FADR captures the
 * actual failing address location in the physical DRAM parts, i.e.,
 * If scrambling is on, LMC()_SCRAMBLED_FADR contains the failing physical
 * location in the DRAM parts (split into DIMM, bunk, bank, etc.)
 * If scrambling is off, the pre-scramble and post-scramble addresses are
 * the same; and so the contents of LMC()_SCRAMBLED_FADR match the contents
 * of LMC()_FADR.
 */
union cvmx_lmcx_fadr {
	u64 u64;
	struct cvmx_lmcx_fadr_s {
		uint64_t reserved_43_63:21;
		uint64_t fcid:3;
		uint64_t fill_order:2;
		uint64_t reserved_0_37:38;
	} s;
	struct cvmx_lmcx_fadr_cn30xx {
		uint64_t reserved_32_63:32;
		uint64_t fdimm:2;
		uint64_t fbunk:1;
		uint64_t fbank:3;
		uint64_t frow:14;
		uint64_t fcol:12;
	} cn30xx;
	struct cvmx_lmcx_fadr_cn30xx cn31xx;
	struct cvmx_lmcx_fadr_cn30xx cn38xx;
	struct cvmx_lmcx_fadr_cn30xx cn38xxp2;
	struct cvmx_lmcx_fadr_cn30xx cn50xx;
	struct cvmx_lmcx_fadr_cn30xx cn52xx;
	struct cvmx_lmcx_fadr_cn30xx cn52xxp1;
	struct cvmx_lmcx_fadr_cn30xx cn56xx;
	struct cvmx_lmcx_fadr_cn30xx cn56xxp1;
	struct cvmx_lmcx_fadr_cn30xx cn58xx;
	struct cvmx_lmcx_fadr_cn30xx cn58xxp1;
	struct cvmx_lmcx_fadr_cn61xx {
		uint64_t reserved_36_63:28;
		uint64_t fdimm:2;
		uint64_t fbunk:1;
		uint64_t fbank:3;
		uint64_t frow:16;
		uint64_t fcol:14;
	} cn61xx;
	struct cvmx_lmcx_fadr_cn61xx cn63xx;
	struct cvmx_lmcx_fadr_cn61xx cn63xxp1;
	struct cvmx_lmcx_fadr_cn61xx cn66xx;
	struct cvmx_lmcx_fadr_cn61xx cn68xx;
	struct cvmx_lmcx_fadr_cn61xx cn68xxp1;
	struct cvmx_lmcx_fadr_cn70xx {
		uint64_t reserved_40_63:24;
		uint64_t fill_order:2;
		uint64_t fdimm:1;
		uint64_t fbunk:1;
		uint64_t fbank:4;
		uint64_t frow:18;
		uint64_t fcol:14;
	} cn70xx;
	struct cvmx_lmcx_fadr_cn70xx cn70xxp1;
	struct cvmx_lmcx_fadr_cn73xx {
		uint64_t reserved_43_63:21;
		uint64_t fcid:3;
		uint64_t fill_order:2;
		uint64_t fdimm:1;
		uint64_t fbunk:1;
		uint64_t fbank:4;
		uint64_t frow:18;
		uint64_t fcol:14;
	} cn73xx;
	struct cvmx_lmcx_fadr_cn73xx cn78xx;
	struct cvmx_lmcx_fadr_cn73xx cn78xxp1;
	struct cvmx_lmcx_fadr_cn61xx cnf71xx;
	struct cvmx_lmcx_fadr_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_general_purpose0
 */
union cvmx_lmcx_general_purpose0 {
	u64 u64;
	struct cvmx_lmcx_general_purpose0_s {
		uint64_t data:64;
	} s;
	struct cvmx_lmcx_general_purpose0_s cn73xx;
	struct cvmx_lmcx_general_purpose0_s cn78xx;
	struct cvmx_lmcx_general_purpose0_s cnf75xx;
};

/**
 * cvmx_lmc#_general_purpose1
 */
union cvmx_lmcx_general_purpose1 {
	u64 u64;
	struct cvmx_lmcx_general_purpose1_s {
		uint64_t data:64;
	} s;
	struct cvmx_lmcx_general_purpose1_s cn73xx;
	struct cvmx_lmcx_general_purpose1_s cn78xx;
	struct cvmx_lmcx_general_purpose1_s cnf75xx;
};

/**
 * cvmx_lmc#_general_purpose2
 */
union cvmx_lmcx_general_purpose2 {
	u64 u64;
	struct cvmx_lmcx_general_purpose2_s {
		uint64_t reserved_16_63:48;
		uint64_t data:16;
	} s;
	struct cvmx_lmcx_general_purpose2_s cn73xx;
	struct cvmx_lmcx_general_purpose2_s cn78xx;
	struct cvmx_lmcx_general_purpose2_s cnf75xx;
};

/**
 * cvmx_lmc#_ifb_cnt
 *
 * LMC_IFB_CNT  = Performance Counters
 *
 */
union cvmx_lmcx_ifb_cnt {
	u64 u64;
	struct cvmx_lmcx_ifb_cnt_s {
		uint64_t ifbcnt:64;
	} s;
	struct cvmx_lmcx_ifb_cnt_s cn61xx;
	struct cvmx_lmcx_ifb_cnt_s cn63xx;
	struct cvmx_lmcx_ifb_cnt_s cn63xxp1;
	struct cvmx_lmcx_ifb_cnt_s cn66xx;
	struct cvmx_lmcx_ifb_cnt_s cn68xx;
	struct cvmx_lmcx_ifb_cnt_s cn68xxp1;
	struct cvmx_lmcx_ifb_cnt_s cn70xx;
	struct cvmx_lmcx_ifb_cnt_s cn70xxp1;
	struct cvmx_lmcx_ifb_cnt_s cn73xx;
	struct cvmx_lmcx_ifb_cnt_s cn78xx;
	struct cvmx_lmcx_ifb_cnt_s cn78xxp1;
	struct cvmx_lmcx_ifb_cnt_s cnf71xx;
	struct cvmx_lmcx_ifb_cnt_s cnf75xx;
};

/**
 * cvmx_lmc#_ifb_cnt_hi
 *
 * LMC_IFB_CNT_HI  = Performance Counters
 *
 */
union cvmx_lmcx_ifb_cnt_hi {
	u64 u64;
	struct cvmx_lmcx_ifb_cnt_hi_s {
		uint64_t reserved_32_63:32;
		uint64_t ifbcnt_hi:32;
	} s;
	struct cvmx_lmcx_ifb_cnt_hi_s cn30xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn31xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn38xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn38xxp2;
	struct cvmx_lmcx_ifb_cnt_hi_s cn50xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn52xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn52xxp1;
	struct cvmx_lmcx_ifb_cnt_hi_s cn56xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn56xxp1;
	struct cvmx_lmcx_ifb_cnt_hi_s cn58xx;
	struct cvmx_lmcx_ifb_cnt_hi_s cn58xxp1;
};

/**
 * cvmx_lmc#_ifb_cnt_lo
 *
 * LMC_IFB_CNT_LO  = Performance Counters
 *
 */
union cvmx_lmcx_ifb_cnt_lo {
	u64 u64;
	struct cvmx_lmcx_ifb_cnt_lo_s {
		uint64_t reserved_32_63:32;
		uint64_t ifbcnt_lo:32;
	} s;
	struct cvmx_lmcx_ifb_cnt_lo_s cn30xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn31xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn38xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn38xxp2;
	struct cvmx_lmcx_ifb_cnt_lo_s cn50xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn52xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn52xxp1;
	struct cvmx_lmcx_ifb_cnt_lo_s cn56xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn56xxp1;
	struct cvmx_lmcx_ifb_cnt_lo_s cn58xx;
	struct cvmx_lmcx_ifb_cnt_lo_s cn58xxp1;
};

/**
 * cvmx_lmc#_int
 *
 * This register contains the different interrupt-summary bits of the LMC.
 *
 */
union cvmx_lmcx_int {
	u64 u64;
	struct cvmx_lmcx_int_s {
		uint64_t reserved_14_63:50;
		uint64_t macram_ded_err:1;
		uint64_t macram_sec_err:1;
		uint64_t ddr_err:1;
		uint64_t dlcram_ded_err:1;
		uint64_t dlcram_sec_err:1;
		uint64_t ded_err:4;
		uint64_t sec_err:4;
		uint64_t nxm_wr_err:1;
	} s;
	struct cvmx_lmcx_int_cn61xx {
		uint64_t reserved_9_63:55;
		uint64_t ded_err:4;
		uint64_t sec_err:4;
		uint64_t nxm_wr_err:1;
	} cn61xx;
	struct cvmx_lmcx_int_cn61xx cn63xx;
	struct cvmx_lmcx_int_cn61xx cn63xxp1;
	struct cvmx_lmcx_int_cn61xx cn66xx;
	struct cvmx_lmcx_int_cn61xx cn68xx;
	struct cvmx_lmcx_int_cn61xx cn68xxp1;
	struct cvmx_lmcx_int_cn70xx {
		uint64_t reserved_12_63:52;
		uint64_t ddr_err:1;
		uint64_t dlcram_ded_err:1;
		uint64_t dlcram_sec_err:1;
		uint64_t ded_err:4;
		uint64_t sec_err:4;
		uint64_t nxm_wr_err:1;
	} cn70xx;
	struct cvmx_lmcx_int_cn70xx cn70xxp1;
	struct cvmx_lmcx_int_s cn73xx;
	struct cvmx_lmcx_int_s cn78xx;
	struct cvmx_lmcx_int_s cn78xxp1;
	struct cvmx_lmcx_int_cn61xx cnf71xx;
	struct cvmx_lmcx_int_s cnf75xx;
};

/**
 * cvmx_lmc#_int_en
 *
 * Unused CSR in O75.
 *
 */
union cvmx_lmcx_int_en {
	u64 u64;
	struct cvmx_lmcx_int_en_s {
		uint64_t reserved_6_63:58;
		uint64_t ddr_error_alert_ena:1;
		uint64_t dlcram_ded_ena:1;
		uint64_t dlcram_sec_ena:1;
		uint64_t intr_ded_ena:1;
		uint64_t intr_sec_ena:1;
		uint64_t intr_nxm_wr_ena:1;
	} s;
	struct cvmx_lmcx_int_en_cn61xx {
		uint64_t reserved_3_63:61;
		uint64_t intr_ded_ena:1;
		uint64_t intr_sec_ena:1;
		uint64_t intr_nxm_wr_ena:1;
	} cn61xx;
	struct cvmx_lmcx_int_en_cn61xx cn63xx;
	struct cvmx_lmcx_int_en_cn61xx cn63xxp1;
	struct cvmx_lmcx_int_en_cn61xx cn66xx;
	struct cvmx_lmcx_int_en_cn61xx cn68xx;
	struct cvmx_lmcx_int_en_cn61xx cn68xxp1;
	struct cvmx_lmcx_int_en_s cn70xx;
	struct cvmx_lmcx_int_en_s cn70xxp1;
	struct cvmx_lmcx_int_en_s cn73xx;
	struct cvmx_lmcx_int_en_s cn78xx;
	struct cvmx_lmcx_int_en_s cn78xxp1;
	struct cvmx_lmcx_int_en_cn61xx cnf71xx;
	struct cvmx_lmcx_int_en_s cnf75xx;
};

/**
 * cvmx_lmc#_lane#_crc_swiz
 *
 * This register contains the CRC bit swizzle for even and odd ranks.
 *
 */
union cvmx_lmcx_lanex_crc_swiz {
	u64 u64;
	struct cvmx_lmcx_lanex_crc_swiz_s {
		uint64_t reserved_56_63:8;
		uint64_t r1_swiz7:3;
		uint64_t r1_swiz6:3;
		uint64_t r1_swiz5:3;
		uint64_t r1_swiz4:3;
		uint64_t r1_swiz3:3;
		uint64_t r1_swiz2:3;
		uint64_t r1_swiz1:3;
		uint64_t r1_swiz0:3;
		uint64_t reserved_24_31:8;
		uint64_t r0_swiz7:3;
		uint64_t r0_swiz6:3;
		uint64_t r0_swiz5:3;
		uint64_t r0_swiz4:3;
		uint64_t r0_swiz3:3;
		uint64_t r0_swiz2:3;
		uint64_t r0_swiz1:3;
		uint64_t r0_swiz0:3;
	} s;
	struct cvmx_lmcx_lanex_crc_swiz_s cn73xx;
	struct cvmx_lmcx_lanex_crc_swiz_s cn78xx;
	struct cvmx_lmcx_lanex_crc_swiz_s cn78xxp1;
	struct cvmx_lmcx_lanex_crc_swiz_s cnf75xx;
};

/**
 * cvmx_lmc#_mem_cfg0
 *
 * Specify the RSL base addresses for the block
 *
 *                  LMC_MEM_CFG0 = LMC Memory Configuration Register0
 *
 * This register controls certain parameters of  Memory Configuration
 */
union cvmx_lmcx_mem_cfg0 {
	u64 u64;
	struct cvmx_lmcx_mem_cfg0_s {
		uint64_t reserved_32_63:32;
		uint64_t reset:1;
		uint64_t silo_qc:1;
		uint64_t bunk_ena:1;
		uint64_t ded_err:4;
		uint64_t sec_err:4;
		uint64_t intr_ded_ena:1;
		uint64_t intr_sec_ena:1;
		uint64_t tcl:4;
		uint64_t ref_int:6;
		uint64_t pbank_lsb:4;
		uint64_t row_lsb:3;
		uint64_t ecc_ena:1;
		uint64_t init_start:1;
	} s;
	struct cvmx_lmcx_mem_cfg0_s cn30xx;
	struct cvmx_lmcx_mem_cfg0_s cn31xx;
	struct cvmx_lmcx_mem_cfg0_s cn38xx;
	struct cvmx_lmcx_mem_cfg0_s cn38xxp2;
	struct cvmx_lmcx_mem_cfg0_s cn50xx;
	struct cvmx_lmcx_mem_cfg0_s cn52xx;
	struct cvmx_lmcx_mem_cfg0_s cn52xxp1;
	struct cvmx_lmcx_mem_cfg0_s cn56xx;
	struct cvmx_lmcx_mem_cfg0_s cn56xxp1;
	struct cvmx_lmcx_mem_cfg0_s cn58xx;
	struct cvmx_lmcx_mem_cfg0_s cn58xxp1;
};

/**
 * cvmx_lmc#_mem_cfg1
 *
 * LMC_MEM_CFG1 = LMC Memory Configuration Register1
 *
 * This register controls the External Memory Configuration Timing Parameters.
 * Please refer to the appropriate DDR part spec from your memory vendor for
 * the various values in this CSR. The details of each of these timing
 * parameters can be found in the JEDEC spec or the vendor spec of the
 * memory parts.
 */
union cvmx_lmcx_mem_cfg1 {
	u64 u64;
	struct cvmx_lmcx_mem_cfg1_s {
		uint64_t reserved_32_63:32;
		uint64_t comp_bypass:1;
		uint64_t trrd:3;
		uint64_t caslat:3;
		uint64_t tmrd:3;
		uint64_t trfc:5;
		uint64_t trp:4;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:5;
	} s;
	struct cvmx_lmcx_mem_cfg1_s cn30xx;
	struct cvmx_lmcx_mem_cfg1_s cn31xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx {
		uint64_t reserved_31_63:33;
		uint64_t trrd:3;
		uint64_t caslat:3;
		uint64_t tmrd:3;
		uint64_t trfc:5;
		uint64_t trp:4;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:5;
	} cn38xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn38xxp2;
	struct cvmx_lmcx_mem_cfg1_s cn50xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn52xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn52xxp1;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn56xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn56xxp1;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn58xx;
	struct cvmx_lmcx_mem_cfg1_cn38xx cn58xxp1;
};

/**
 * cvmx_lmc#_modereg_params0
 *
 * These parameters are written into the DDR3/DDR4 MR0, MR1, MR2 and MR3
 * registers.
 *
 */
union cvmx_lmcx_modereg_params0 {
	u64 u64;
	struct cvmx_lmcx_modereg_params0_s {
		uint64_t reserved_28_63:36;
		uint64_t wrp_ext:1;
		uint64_t cl_ext:1;
		uint64_t al_ext:1;
		uint64_t ppd:1;
		uint64_t wrp:3;
		uint64_t dllr:1;
		uint64_t tm:1;
		uint64_t rbt:1;
		uint64_t cl:4;
		uint64_t bl:2;
		uint64_t qoff:1;
		uint64_t tdqs:1;
		uint64_t wlev:1;
		uint64_t al:2;
		uint64_t dll:1;
		uint64_t mpr:1;
		uint64_t mprloc:2;
		uint64_t cwl:3;
	} s;
	struct cvmx_lmcx_modereg_params0_cn61xx {
		uint64_t reserved_25_63:39;
		uint64_t ppd:1;
		uint64_t wrp:3;
		uint64_t dllr:1;
		uint64_t tm:1;
		uint64_t rbt:1;
		uint64_t cl:4;
		uint64_t bl:2;
		uint64_t qoff:1;
		uint64_t tdqs:1;
		uint64_t wlev:1;
		uint64_t al:2;
		uint64_t dll:1;
		uint64_t mpr:1;
		uint64_t mprloc:2;
		uint64_t cwl:3;
	} cn61xx;
	struct cvmx_lmcx_modereg_params0_cn61xx cn63xx;
	struct cvmx_lmcx_modereg_params0_cn61xx cn63xxp1;
	struct cvmx_lmcx_modereg_params0_cn61xx cn66xx;
	struct cvmx_lmcx_modereg_params0_cn61xx cn68xx;
	struct cvmx_lmcx_modereg_params0_cn61xx cn68xxp1;
	struct cvmx_lmcx_modereg_params0_cn61xx cn70xx;
	struct cvmx_lmcx_modereg_params0_cn61xx cn70xxp1;
	struct cvmx_lmcx_modereg_params0_s cn73xx;
	struct cvmx_lmcx_modereg_params0_s cn78xx;
	struct cvmx_lmcx_modereg_params0_s cn78xxp1;
	struct cvmx_lmcx_modereg_params0_cn61xx cnf71xx;
	struct cvmx_lmcx_modereg_params0_s cnf75xx;
};

/**
 * cvmx_lmc#_modereg_params1
 *
 * These parameters are written into the DDR3 MR0, MR1, MR2 and MR3 registers.
 *
 */
union cvmx_lmcx_modereg_params1 {
	u64 u64;
	struct cvmx_lmcx_modereg_params1_s {
		uint64_t reserved_55_63:9;
		uint64_t rtt_wr_11_ext:1;
		uint64_t rtt_wr_10_ext:1;
		uint64_t rtt_wr_01_ext:1;
		uint64_t rtt_wr_00_ext:1;
		uint64_t db_output_impedance:3;
		uint64_t rtt_nom_11:3;
		uint64_t dic_11:2;
		uint64_t rtt_wr_11:2;
		uint64_t srt_11:1;
		uint64_t asr_11:1;
		uint64_t pasr_11:3;
		uint64_t rtt_nom_10:3;
		uint64_t dic_10:2;
		uint64_t rtt_wr_10:2;
		uint64_t srt_10:1;
		uint64_t asr_10:1;
		uint64_t pasr_10:3;
		uint64_t rtt_nom_01:3;
		uint64_t dic_01:2;
		uint64_t rtt_wr_01:2;
		uint64_t srt_01:1;
		uint64_t asr_01:1;
		uint64_t pasr_01:3;
		uint64_t rtt_nom_00:3;
		uint64_t dic_00:2;
		uint64_t rtt_wr_00:2;
		uint64_t srt_00:1;
		uint64_t asr_00:1;
		uint64_t pasr_00:3;
	} s;
	struct cvmx_lmcx_modereg_params1_cn61xx {
		uint64_t reserved_48_63:16;
		uint64_t rtt_nom_11:3;
		uint64_t dic_11:2;
		uint64_t rtt_wr_11:2;
		uint64_t srt_11:1;
		uint64_t asr_11:1;
		uint64_t pasr_11:3;
		uint64_t rtt_nom_10:3;
		uint64_t dic_10:2;
		uint64_t rtt_wr_10:2;
		uint64_t srt_10:1;
		uint64_t asr_10:1;
		uint64_t pasr_10:3;
		uint64_t rtt_nom_01:3;
		uint64_t dic_01:2;
		uint64_t rtt_wr_01:2;
		uint64_t srt_01:1;
		uint64_t asr_01:1;
		uint64_t pasr_01:3;
		uint64_t rtt_nom_00:3;
		uint64_t dic_00:2;
		uint64_t rtt_wr_00:2;
		uint64_t srt_00:1;
		uint64_t asr_00:1;
		uint64_t pasr_00:3;
	} cn61xx;
	struct cvmx_lmcx_modereg_params1_cn61xx cn63xx;
	struct cvmx_lmcx_modereg_params1_cn61xx cn63xxp1;
	struct cvmx_lmcx_modereg_params1_cn61xx cn66xx;
	struct cvmx_lmcx_modereg_params1_cn61xx cn68xx;
	struct cvmx_lmcx_modereg_params1_cn61xx cn68xxp1;
	struct cvmx_lmcx_modereg_params1_cn61xx cn70xx;
	struct cvmx_lmcx_modereg_params1_cn61xx cn70xxp1;
	struct cvmx_lmcx_modereg_params1_s cn73xx;
	struct cvmx_lmcx_modereg_params1_s cn78xx;
	struct cvmx_lmcx_modereg_params1_s cn78xxp1;
	struct cvmx_lmcx_modereg_params1_cn61xx cnf71xx;
	struct cvmx_lmcx_modereg_params1_s cnf75xx;
};

/**
 * cvmx_lmc#_modereg_params2
 *
 * These parameters are written into the DDR4 mode registers.
 *
 */
union cvmx_lmcx_modereg_params2 {
	u64 u64;
	struct cvmx_lmcx_modereg_params2_s {
		uint64_t reserved_41_63:23;
		uint64_t vrefdq_train_en:1;
		uint64_t vref_range_11:1;
		uint64_t vref_value_11:6;
		uint64_t rtt_park_11:3;
		uint64_t vref_range_10:1;
		uint64_t vref_value_10:6;
		uint64_t rtt_park_10:3;
		uint64_t vref_range_01:1;
		uint64_t vref_value_01:6;
		uint64_t rtt_park_01:3;
		uint64_t vref_range_00:1;
		uint64_t vref_value_00:6;
		uint64_t rtt_park_00:3;
	} s;
	struct cvmx_lmcx_modereg_params2_s cn70xx;
	struct cvmx_lmcx_modereg_params2_cn70xxp1 {
		uint64_t reserved_40_63:24;
		uint64_t vref_range_11:1;
		uint64_t vref_value_11:6;
		uint64_t rtt_park_11:3;
		uint64_t vref_range_10:1;
		uint64_t vref_value_10:6;
		uint64_t rtt_park_10:3;
		uint64_t vref_range_01:1;
		uint64_t vref_value_01:6;
		uint64_t rtt_park_01:3;
		uint64_t vref_range_00:1;
		uint64_t vref_value_00:6;
		uint64_t rtt_park_00:3;
	} cn70xxp1;
	struct cvmx_lmcx_modereg_params2_s cn73xx;
	struct cvmx_lmcx_modereg_params2_s cn78xx;
	struct cvmx_lmcx_modereg_params2_s cn78xxp1;
	struct cvmx_lmcx_modereg_params2_s cnf75xx;
};

/**
 * cvmx_lmc#_modereg_params3
 *
 * These parameters are written into the DDR4 mode registers.
 *
 */
union cvmx_lmcx_modereg_params3 {
	u64 u64;
	struct cvmx_lmcx_modereg_params3_s {
		uint64_t reserved_39_63:25;
		uint64_t xrank_add_tccd_l:3;
		uint64_t xrank_add_tccd_s:3;
		uint64_t mpr_fmt:2;
		uint64_t wr_cmd_lat:2;
		uint64_t fgrm:3;
		uint64_t temp_sense:1;
		uint64_t pda:1;
		uint64_t gd:1;
		uint64_t crc:1;
		uint64_t lpasr:2;
		uint64_t tccd_l:3;
		uint64_t rd_dbi:1;
		uint64_t wr_dbi:1;
		uint64_t dm:1;
		uint64_t ca_par_pers:1;
		uint64_t odt_pd:1;
		uint64_t par_lat_mode:3;
		uint64_t wr_preamble:1;
		uint64_t rd_preamble:1;
		uint64_t sre_abort:1;
		uint64_t cal:3;
		uint64_t vref_mon:1;
		uint64_t tc_ref:1;
		uint64_t max_pd:1;
	} s;
	struct cvmx_lmcx_modereg_params3_cn70xx {
		uint64_t reserved_33_63:31;
		uint64_t mpr_fmt:2;
		uint64_t wr_cmd_lat:2;
		uint64_t fgrm:3;
		uint64_t temp_sense:1;
		uint64_t pda:1;
		uint64_t gd:1;
		uint64_t crc:1;
		uint64_t lpasr:2;
		uint64_t tccd_l:3;
		uint64_t rd_dbi:1;
		uint64_t wr_dbi:1;
		uint64_t dm:1;
		uint64_t ca_par_pers:1;
		uint64_t odt_pd:1;
		uint64_t par_lat_mode:3;
		uint64_t wr_preamble:1;
		uint64_t rd_preamble:1;
		uint64_t sre_abort:1;
		uint64_t cal:3;
		uint64_t vref_mon:1;
		uint64_t tc_ref:1;
		uint64_t max_pd:1;
	} cn70xx;
	struct cvmx_lmcx_modereg_params3_cn70xx cn70xxp1;
	struct cvmx_lmcx_modereg_params3_s cn73xx;
	struct cvmx_lmcx_modereg_params3_s cn78xx;
	struct cvmx_lmcx_modereg_params3_s cn78xxp1;
	struct cvmx_lmcx_modereg_params3_s cnf75xx;
};

/**
 * cvmx_lmc#_mpr_data0
 *
 * This register provides bits <63:0> of MPR data register.
 *
 */
union cvmx_lmcx_mpr_data0 {
	u64 u64;
	struct cvmx_lmcx_mpr_data0_s {
		uint64_t mpr_data:64;
	} s;
	struct cvmx_lmcx_mpr_data0_s cn70xx;
	struct cvmx_lmcx_mpr_data0_s cn70xxp1;
	struct cvmx_lmcx_mpr_data0_s cn73xx;
	struct cvmx_lmcx_mpr_data0_s cn78xx;
	struct cvmx_lmcx_mpr_data0_s cn78xxp1;
	struct cvmx_lmcx_mpr_data0_s cnf75xx;
};

/**
 * cvmx_lmc#_mpr_data1
 *
 * This register provides bits <127:64> of MPR data register.
 *
 */
union cvmx_lmcx_mpr_data1 {
	u64 u64;
	struct cvmx_lmcx_mpr_data1_s {
		uint64_t mpr_data:64;
	} s;
	struct cvmx_lmcx_mpr_data1_s cn70xx;
	struct cvmx_lmcx_mpr_data1_s cn70xxp1;
	struct cvmx_lmcx_mpr_data1_s cn73xx;
	struct cvmx_lmcx_mpr_data1_s cn78xx;
	struct cvmx_lmcx_mpr_data1_s cn78xxp1;
	struct cvmx_lmcx_mpr_data1_s cnf75xx;
};

/**
 * cvmx_lmc#_mpr_data2
 *
 * This register provides bits <143:128> of MPR data register.
 *
 */
union cvmx_lmcx_mpr_data2 {
	u64 u64;
	struct cvmx_lmcx_mpr_data2_s {
		uint64_t reserved_16_63:48;
		uint64_t mpr_data:16;
	} s;
	struct cvmx_lmcx_mpr_data2_s cn70xx;
	struct cvmx_lmcx_mpr_data2_s cn70xxp1;
	struct cvmx_lmcx_mpr_data2_s cn73xx;
	struct cvmx_lmcx_mpr_data2_s cn78xx;
	struct cvmx_lmcx_mpr_data2_s cn78xxp1;
	struct cvmx_lmcx_mpr_data2_s cnf75xx;
};

/**
 * cvmx_lmc#_mr_mpr_ctl
 *
 * This register provides the control functions when programming the MPR
 * of DDR4 DRAMs.
 *
 */
union cvmx_lmcx_mr_mpr_ctl {
	u64 u64;
	struct cvmx_lmcx_mr_mpr_ctl_s {
		uint64_t reserved_61_63:3;
		uint64_t mr_wr_secure_key_ena:1;
		uint64_t pba_func_space:3;
		uint64_t mr_wr_bg1:1;
		uint64_t mpr_sample_dq_enable:1;
		uint64_t pda_early_dqx:1;
		uint64_t mr_wr_pba_enable:1;
		uint64_t mr_wr_use_default_value:1;
		uint64_t mpr_whole_byte_enable:1;
		uint64_t mpr_byte_select:4;
		uint64_t mpr_bit_select:2;
		uint64_t mpr_wr:1;
		uint64_t mpr_loc:2;
		uint64_t mr_wr_pda_enable:1;
		uint64_t mr_wr_pda_mask:18;
		uint64_t mr_wr_rank:2;
		uint64_t mr_wr_sel:3;
		uint64_t mr_wr_addr:18;
	} s;
	struct cvmx_lmcx_mr_mpr_ctl_cn70xx {
		uint64_t reserved_52_63:12;
		uint64_t mpr_whole_byte_enable:1;
		uint64_t mpr_byte_select:4;
		uint64_t mpr_bit_select:2;
		uint64_t mpr_wr:1;
		uint64_t mpr_loc:2;
		uint64_t mr_wr_pda_enable:1;
		uint64_t mr_wr_pda_mask:18;
		uint64_t mr_wr_rank:2;
		uint64_t mr_wr_sel:3;
		uint64_t mr_wr_addr:18;
	} cn70xx;
	struct cvmx_lmcx_mr_mpr_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_mr_mpr_ctl_s cn73xx;
	struct cvmx_lmcx_mr_mpr_ctl_s cn78xx;
	struct cvmx_lmcx_mr_mpr_ctl_s cn78xxp1;
	struct cvmx_lmcx_mr_mpr_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_ns_ctl
 *
 * This register contains control parameters for handling nonsecure accesses.
 *
 */
union cvmx_lmcx_ns_ctl {
	u64 u64;
	struct cvmx_lmcx_ns_ctl_s {
		uint64_t reserved_26_63:38;
		uint64_t ns_scramble_dis:1;
		uint64_t reserved_18_24:7;
		uint64_t adr_offset:18;
	} s;
	struct cvmx_lmcx_ns_ctl_s cn73xx;
	struct cvmx_lmcx_ns_ctl_s cn78xx;
	struct cvmx_lmcx_ns_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_nxm
 *
 * Following is the decoding for mem_msb/rank:
 * 0x0: mem_msb = mem_adr[25].
 * 0x1: mem_msb = mem_adr[26].
 * 0x2: mem_msb = mem_adr[27].
 * 0x3: mem_msb = mem_adr[28].
 * 0x4: mem_msb = mem_adr[29].
 * 0x5: mem_msb = mem_adr[30].
 * 0x6: mem_msb = mem_adr[31].
 * 0x7: mem_msb = mem_adr[32].
 * 0x8: mem_msb = mem_adr[33].
 * 0x9: mem_msb = mem_adr[34].
 * 0xA: mem_msb = mem_adr[35].
 * 0xB: mem_msb = mem_adr[36].
 * 0xC-0xF = Reserved.
 *
 * For example, for a DIMM made of Samsung's K4B1G0846C-ZCF7 1Gb
 * (16M * 8 bit * 8 bank) parts, the column address width = 10; so with
 * 10b of col, 3b of bus, 3b of bank, row_lsb = 16.
 * Therefore, row = mem_adr[29:16] and mem_msb = 4.
 *
 * Note also that addresses greater than the max defined space (pbank_msb)
 * are also treated as NXM accesses.
 */
union cvmx_lmcx_nxm {
	u64 u64;
	struct cvmx_lmcx_nxm_s {
		uint64_t reserved_40_63:24;
		uint64_t mem_msb_d3_r1:4;
		uint64_t mem_msb_d3_r0:4;
		uint64_t mem_msb_d2_r1:4;
		uint64_t mem_msb_d2_r0:4;
		uint64_t mem_msb_d1_r1:4;
		uint64_t mem_msb_d1_r0:4;
		uint64_t mem_msb_d0_r1:4;
		uint64_t mem_msb_d0_r0:4;
		uint64_t cs_mask:8;
	} s;
	struct cvmx_lmcx_nxm_cn52xx {
		uint64_t reserved_8_63:56;
		uint64_t cs_mask:8;
	} cn52xx;
	struct cvmx_lmcx_nxm_cn52xx cn56xx;
	struct cvmx_lmcx_nxm_cn52xx cn58xx;
	struct cvmx_lmcx_nxm_s cn61xx;
	struct cvmx_lmcx_nxm_s cn63xx;
	struct cvmx_lmcx_nxm_s cn63xxp1;
	struct cvmx_lmcx_nxm_s cn66xx;
	struct cvmx_lmcx_nxm_s cn68xx;
	struct cvmx_lmcx_nxm_s cn68xxp1;
	struct cvmx_lmcx_nxm_cn70xx {
		uint64_t reserved_24_63:40;
		uint64_t mem_msb_d1_r1:4;
		uint64_t mem_msb_d1_r0:4;
		uint64_t mem_msb_d0_r1:4;
		uint64_t mem_msb_d0_r0:4;
		uint64_t reserved_4_7:4;
		uint64_t cs_mask:4;
	} cn70xx;
	struct cvmx_lmcx_nxm_cn70xx cn70xxp1;
	struct cvmx_lmcx_nxm_cn70xx cn73xx;
	struct cvmx_lmcx_nxm_cn70xx cn78xx;
	struct cvmx_lmcx_nxm_cn70xx cn78xxp1;
	struct cvmx_lmcx_nxm_s cnf71xx;
	struct cvmx_lmcx_nxm_cn70xx cnf75xx;
};

/**
 * cvmx_lmc#_nxm_fadr
 *
 * This register captures only the first transaction with a NXM error while
 * an interrupt is pending, and only captures a subsequent event once the
 * interrupt is cleared by writing a one to LMC()_INT[NXM_ERR]. It captures
 * the actual L2C-LMC address provided to the LMC that caused the NXM error.
 * A read or write NXM error is captured only if enabled using the NXM
 * event enables.
 */
union cvmx_lmcx_nxm_fadr {
	u64 u64;
	struct cvmx_lmcx_nxm_fadr_s {
		uint64_t reserved_40_63:24;
		uint64_t nxm_faddr_ext:1;
		uint64_t nxm_src:1;
		uint64_t nxm_type:1;
		uint64_t nxm_faddr:37;
	} s;
	struct cvmx_lmcx_nxm_fadr_cn70xx {
		uint64_t reserved_39_63:25;
		uint64_t nxm_src:1;
		uint64_t nxm_type:1;
		uint64_t nxm_faddr:37;
	} cn70xx;
	struct cvmx_lmcx_nxm_fadr_cn70xx cn70xxp1;
	struct cvmx_lmcx_nxm_fadr_s cn73xx;
	struct cvmx_lmcx_nxm_fadr_s cn78xx;
	struct cvmx_lmcx_nxm_fadr_s cn78xxp1;
	struct cvmx_lmcx_nxm_fadr_s cnf75xx;
};

/**
 * cvmx_lmc#_ops_cnt
 *
 * LMC_OPS_CNT  = Performance Counters
 *
 */
union cvmx_lmcx_ops_cnt {
	u64 u64;
	struct cvmx_lmcx_ops_cnt_s {
		uint64_t opscnt:64;
	} s;
	struct cvmx_lmcx_ops_cnt_s cn61xx;
	struct cvmx_lmcx_ops_cnt_s cn63xx;
	struct cvmx_lmcx_ops_cnt_s cn63xxp1;
	struct cvmx_lmcx_ops_cnt_s cn66xx;
	struct cvmx_lmcx_ops_cnt_s cn68xx;
	struct cvmx_lmcx_ops_cnt_s cn68xxp1;
	struct cvmx_lmcx_ops_cnt_s cn70xx;
	struct cvmx_lmcx_ops_cnt_s cn70xxp1;
	struct cvmx_lmcx_ops_cnt_s cn73xx;
	struct cvmx_lmcx_ops_cnt_s cn78xx;
	struct cvmx_lmcx_ops_cnt_s cn78xxp1;
	struct cvmx_lmcx_ops_cnt_s cnf71xx;
	struct cvmx_lmcx_ops_cnt_s cnf75xx;
};

/**
 * cvmx_lmc#_ops_cnt_hi
 *
 * LMC_OPS_CNT_HI  = Performance Counters
 *
 */
union cvmx_lmcx_ops_cnt_hi {
	u64 u64;
	struct cvmx_lmcx_ops_cnt_hi_s {
		uint64_t reserved_32_63:32;
		uint64_t opscnt_hi:32;
	} s;
	struct cvmx_lmcx_ops_cnt_hi_s cn30xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn31xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn38xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn38xxp2;
	struct cvmx_lmcx_ops_cnt_hi_s cn50xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn52xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn52xxp1;
	struct cvmx_lmcx_ops_cnt_hi_s cn56xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn56xxp1;
	struct cvmx_lmcx_ops_cnt_hi_s cn58xx;
	struct cvmx_lmcx_ops_cnt_hi_s cn58xxp1;
};

/**
 * cvmx_lmc#_ops_cnt_lo
 *
 * LMC_OPS_CNT_LO  = Performance Counters
 *
 */
union cvmx_lmcx_ops_cnt_lo {
	u64 u64;
	struct cvmx_lmcx_ops_cnt_lo_s {
		uint64_t reserved_32_63:32;
		uint64_t opscnt_lo:32;
	} s;
	struct cvmx_lmcx_ops_cnt_lo_s cn30xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn31xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn38xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn38xxp2;
	struct cvmx_lmcx_ops_cnt_lo_s cn50xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn52xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn52xxp1;
	struct cvmx_lmcx_ops_cnt_lo_s cn56xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn56xxp1;
	struct cvmx_lmcx_ops_cnt_lo_s cn58xx;
	struct cvmx_lmcx_ops_cnt_lo_s cn58xxp1;
};

/**
 * cvmx_lmc#_phy_ctl
 *
 * LMC_PHY_CTL = LMC PHY Control
 *
 */
union cvmx_lmcx_phy_ctl {
	u64 u64;
	struct cvmx_lmcx_phy_ctl_s {
		uint64_t reserved_61_63:3;
		uint64_t dsk_dbg_load_dis:1;
		uint64_t dsk_dbg_overwrt_ena:1;
		uint64_t dsk_dbg_wr_mode:1;
		uint64_t data_rate_loopback:1;
		uint64_t dq_shallow_loopback:1;
		uint64_t dm_disable:1;
		uint64_t c1_sel:2;
		uint64_t c0_sel:2;
		uint64_t phy_reset:1;
		uint64_t dsk_dbg_rd_complete:1;
		uint64_t dsk_dbg_rd_data:10;
		uint64_t dsk_dbg_rd_start:1;
		uint64_t dsk_dbg_clk_scaler:2;
		uint64_t dsk_dbg_offset:2;
		uint64_t dsk_dbg_num_bits_sel:1;
		uint64_t dsk_dbg_byte_sel:4;
		uint64_t dsk_dbg_bit_sel:4;
		uint64_t dbi_mode_ena:1;
		uint64_t ddr_error_n_ena:1;
		uint64_t ref_pin_on:1;
		uint64_t dac_on:1;
		uint64_t int_pad_loopback_ena:1;
		uint64_t int_phy_loopback_ena:1;
		uint64_t phy_dsk_reset:1;
		uint64_t phy_dsk_byp:1;
		uint64_t phy_pwr_save_disable:1;
		uint64_t ten:1;
		uint64_t rx_always_on:1;
		uint64_t lv_mode:1;
		uint64_t ck_tune1:1;
		uint64_t ck_dlyout1:4;
		uint64_t ck_tune0:1;
		uint64_t ck_dlyout0:4;
		uint64_t loopback:1;
		uint64_t loopback_pos:1;
		uint64_t ts_stagger:1;
	} s;
	struct cvmx_lmcx_phy_ctl_cn61xx {
		uint64_t reserved_15_63:49;
		uint64_t rx_always_on:1;
		uint64_t lv_mode:1;
		uint64_t ck_tune1:1;
		uint64_t ck_dlyout1:4;
		uint64_t ck_tune0:1;
		uint64_t ck_dlyout0:4;
		uint64_t loopback:1;
		uint64_t loopback_pos:1;
		uint64_t ts_stagger:1;
	} cn61xx;
	struct cvmx_lmcx_phy_ctl_cn61xx cn63xx;
	struct cvmx_lmcx_phy_ctl_cn63xxp1 {
		uint64_t reserved_14_63:50;
		uint64_t lv_mode:1;
		uint64_t ck_tune1:1;
		uint64_t ck_dlyout1:4;
		uint64_t ck_tune0:1;
		uint64_t ck_dlyout0:4;
		uint64_t loopback:1;
		uint64_t loopback_pos:1;
		uint64_t ts_stagger:1;
	} cn63xxp1;
	struct cvmx_lmcx_phy_ctl_cn61xx cn66xx;
	struct cvmx_lmcx_phy_ctl_cn61xx cn68xx;
	struct cvmx_lmcx_phy_ctl_cn61xx cn68xxp1;
	struct cvmx_lmcx_phy_ctl_cn70xx {
		uint64_t reserved_51_63:13;
		uint64_t phy_reset:1;
		uint64_t dsk_dbg_rd_complete:1;
		uint64_t dsk_dbg_rd_data:10;
		uint64_t dsk_dbg_rd_start:1;
		uint64_t dsk_dbg_clk_scaler:2;
		uint64_t dsk_dbg_offset:2;
		uint64_t dsk_dbg_num_bits_sel:1;
		uint64_t dsk_dbg_byte_sel:4;
		uint64_t dsk_dbg_bit_sel:4;
		uint64_t dbi_mode_ena:1;
		uint64_t ddr_error_n_ena:1;
		uint64_t ref_pin_on:1;
		uint64_t dac_on:1;
		uint64_t int_pad_loopback_ena:1;
		uint64_t int_phy_loopback_ena:1;
		uint64_t phy_dsk_reset:1;
		uint64_t phy_dsk_byp:1;
		uint64_t phy_pwr_save_disable:1;
		uint64_t ten:1;
		uint64_t rx_always_on:1;
		uint64_t lv_mode:1;
		uint64_t ck_tune1:1;
		uint64_t ck_dlyout1:4;
		uint64_t ck_tune0:1;
		uint64_t ck_dlyout0:4;
		uint64_t loopback:1;
		uint64_t loopback_pos:1;
		uint64_t ts_stagger:1;
	} cn70xx;
	struct cvmx_lmcx_phy_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_phy_ctl_cn73xx {
		uint64_t reserved_58_63:6;
		uint64_t data_rate_loopback:1;
		uint64_t dq_shallow_loopback:1;
		uint64_t dm_disable:1;
		uint64_t c1_sel:2;
		uint64_t c0_sel:2;
		uint64_t phy_reset:1;
		uint64_t dsk_dbg_rd_complete:1;
		uint64_t dsk_dbg_rd_data:10;
		uint64_t dsk_dbg_rd_start:1;
		uint64_t dsk_dbg_clk_scaler:2;
		uint64_t dsk_dbg_offset:2;
		uint64_t dsk_dbg_num_bits_sel:1;
		uint64_t dsk_dbg_byte_sel:4;
		uint64_t dsk_dbg_bit_sel:4;
		uint64_t dbi_mode_ena:1;
		uint64_t ddr_error_n_ena:1;
		uint64_t ref_pin_on:1;
		uint64_t dac_on:1;
		uint64_t int_pad_loopback_ena:1;
		uint64_t int_phy_loopback_ena:1;
		uint64_t phy_dsk_reset:1;
		uint64_t phy_dsk_byp:1;
		uint64_t phy_pwr_save_disable:1;
		uint64_t ten:1;
		uint64_t rx_always_on:1;
		uint64_t lv_mode:1;
		uint64_t ck_tune1:1;
		uint64_t ck_dlyout1:4;
		uint64_t ck_tune0:1;
		uint64_t ck_dlyout0:4;
		uint64_t loopback:1;
		uint64_t loopback_pos:1;
		uint64_t ts_stagger:1;
	} cn73xx;
	struct cvmx_lmcx_phy_ctl_s cn78xx;
	struct cvmx_lmcx_phy_ctl_s cn78xxp1;
	struct cvmx_lmcx_phy_ctl_cn61xx cnf71xx;
	struct cvmx_lmcx_phy_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_phy_ctl2
 */
union cvmx_lmcx_phy_ctl2 {
	u64 u64;
	struct cvmx_lmcx_phy_ctl2_s {
		uint64_t reserved_27_63:37;
		uint64_t dqs8_dsk_adj:3;
		uint64_t dqs7_dsk_adj:3;
		uint64_t dqs6_dsk_adj:3;
		uint64_t dqs5_dsk_adj:3;
		uint64_t dqs4_dsk_adj:3;
		uint64_t dqs3_dsk_adj:3;
		uint64_t dqs2_dsk_adj:3;
		uint64_t dqs1_dsk_adj:3;
		uint64_t dqs0_dsk_adj:3;
	} s;
	struct cvmx_lmcx_phy_ctl2_s cn78xx;
	struct cvmx_lmcx_phy_ctl2_s cnf75xx;
};

/**
 * cvmx_lmc#_pll_bwctl
 *
 * LMC_PLL_BWCTL  = DDR PLL Bandwidth Control Register
 *
 */
union cvmx_lmcx_pll_bwctl {
	u64 u64;
	struct cvmx_lmcx_pll_bwctl_s {
		uint64_t reserved_5_63:59;
		uint64_t bwupd:1;
		uint64_t bwctl:4;
	} s;
	struct cvmx_lmcx_pll_bwctl_s cn30xx;
	struct cvmx_lmcx_pll_bwctl_s cn31xx;
	struct cvmx_lmcx_pll_bwctl_s cn38xx;
	struct cvmx_lmcx_pll_bwctl_s cn38xxp2;
};

/**
 * cvmx_lmc#_pll_ctl
 *
 * LMC_PLL_CTL = LMC pll control
 *
 *
 * Notes:
 * This CSR is only relevant for LMC0. LMC1_PLL_CTL is not used.
 *
 * Exactly one of EN2, EN4, EN6, EN8, EN12, EN16 must be set.
 *
 * The resultant DDR_CK frequency is the DDR2_REF_CLK
 * frequency multiplied by:
 *
 *     (CLKF + 1) / ((CLKR + 1) * EN(2,4,6,8,12,16))
 *
 * The PLL frequency, which is:
 *
 *     (DDR2_REF_CLK freq) * ((CLKF + 1) / (CLKR + 1))
 *
 * must reside between 1.2 and 2.5 GHz. A faster PLL frequency is
 * desirable if there is a choice.
 */
union cvmx_lmcx_pll_ctl {
	u64 u64;
	struct cvmx_lmcx_pll_ctl_s {
		uint64_t reserved_30_63:34;
		uint64_t bypass:1;
		uint64_t fasten_n:1;
		uint64_t div_reset:1;
		uint64_t reset_n:1;
		uint64_t clkf:12;
		uint64_t clkr:6;
		uint64_t reserved_6_7:2;
		uint64_t en16:1;
		uint64_t en12:1;
		uint64_t en8:1;
		uint64_t en6:1;
		uint64_t en4:1;
		uint64_t en2:1;
	} s;
	struct cvmx_lmcx_pll_ctl_cn50xx {
		uint64_t reserved_29_63:35;
		uint64_t fasten_n:1;
		uint64_t div_reset:1;
		uint64_t reset_n:1;
		uint64_t clkf:12;
		uint64_t clkr:6;
		uint64_t reserved_6_7:2;
		uint64_t en16:1;
		uint64_t en12:1;
		uint64_t en8:1;
		uint64_t en6:1;
		uint64_t en4:1;
		uint64_t en2:1;
	} cn50xx;
	struct cvmx_lmcx_pll_ctl_s cn52xx;
	struct cvmx_lmcx_pll_ctl_s cn52xxp1;
	struct cvmx_lmcx_pll_ctl_cn50xx cn56xx;
	struct cvmx_lmcx_pll_ctl_cn56xxp1 {
		uint64_t reserved_28_63:36;
		uint64_t div_reset:1;
		uint64_t reset_n:1;
		uint64_t clkf:12;
		uint64_t clkr:6;
		uint64_t reserved_6_7:2;
		uint64_t en16:1;
		uint64_t en12:1;
		uint64_t en8:1;
		uint64_t en6:1;
		uint64_t en4:1;
		uint64_t en2:1;
	} cn56xxp1;
	struct cvmx_lmcx_pll_ctl_cn56xxp1 cn58xx;
	struct cvmx_lmcx_pll_ctl_cn56xxp1 cn58xxp1;
};

/**
 * cvmx_lmc#_pll_status
 *
 * LMC_PLL_STATUS = LMC pll status
 *
 */
union cvmx_lmcx_pll_status {
	u64 u64;
	struct cvmx_lmcx_pll_status_s {
		uint64_t reserved_32_63:32;
		uint64_t ddr__nctl:5;
		uint64_t ddr__pctl:5;
		uint64_t reserved_2_21:20;
		uint64_t rfslip:1;
		uint64_t fbslip:1;
	} s;
	struct cvmx_lmcx_pll_status_s cn50xx;
	struct cvmx_lmcx_pll_status_s cn52xx;
	struct cvmx_lmcx_pll_status_s cn52xxp1;
	struct cvmx_lmcx_pll_status_s cn56xx;
	struct cvmx_lmcx_pll_status_s cn56xxp1;
	struct cvmx_lmcx_pll_status_s cn58xx;
	struct cvmx_lmcx_pll_status_cn58xxp1 {
		uint64_t reserved_2_63:62;
		uint64_t rfslip:1;
		uint64_t fbslip:1;
	} cn58xxp1;
};

/**
 * cvmx_lmc#_ppr_ctl
 *
 * This register contains programmable timing and control parameters used
 * when running the post package repair sequence. The timing fields
 * PPR_CTL[TPGMPST], PPR_CTL[TPGM_EXIT] and PPR_CTL[TPGM] need to be set as
 * to satisfy the minimum values mentioned in the JEDEC DDR4 spec before
 * running the PPR sequence. See LMC()_SEQ_CTL[SEQ_SEL,INIT_START] to run
 * the PPR sequence.
 *
 * Running hard PPR may require LMC to issue security key as four consecutive
 * MR0 commands, each with a unique address field A[17:0]. Set the security
 * key in the general purpose CSRs as follows:
 *
 * _ Security key 0 = LMC()_GENERAL_PURPOSE0[DATA]<17:0>.
 * _ Security key 1 = LMC()_GENERAL_PURPOSE0[DATA]<35:18>.
 * _ Security key 2 = LMC()_GENERAL_PURPOSE1[DATA]<17:0>.
 * _ Security key 3 = LMC()_GENERAL_PURPOSE1[DATA]<35:18>.
 */
union cvmx_lmcx_ppr_ctl {
	u64 u64;
	struct cvmx_lmcx_ppr_ctl_s {
		uint64_t reserved_27_63:37;
		uint64_t lrank_sel:3;
		uint64_t skip_issue_security:1;
		uint64_t sppr:1;
		uint64_t tpgm:10;
		uint64_t tpgm_exit:5;
		uint64_t tpgmpst:7;
	} s;
	struct cvmx_lmcx_ppr_ctl_cn73xx {
		uint64_t reserved_24_63:40;
		uint64_t skip_issue_security:1;
		uint64_t sppr:1;
		uint64_t tpgm:10;
		uint64_t tpgm_exit:5;
		uint64_t tpgmpst:7;
	} cn73xx;
	struct cvmx_lmcx_ppr_ctl_s cn78xx;
	struct cvmx_lmcx_ppr_ctl_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_read_level_ctl
 *
 * Notes:
 * The HW writes and reads the cache block selected by ROW, COL, BNK and
 * the rank as part of a read-leveling sequence for a rank.
 * A cache block write is 16 72-bit words. PATTERN selects the write value.
 * For the first 8 words, the write value is the bit PATTERN<i> duplicated
 * into a 72-bit vector. The write value of the last 8 words is the inverse
 * of the write value of the first 8 words. See LMC*_READ_LEVEL_RANK*.
 */
union cvmx_lmcx_read_level_ctl {
	u64 u64;
	struct cvmx_lmcx_read_level_ctl_s {
		uint64_t reserved_44_63:20;
		uint64_t rankmask:4;
		uint64_t pattern:8;
		uint64_t row:16;
		uint64_t col:12;
		uint64_t reserved_3_3:1;
		uint64_t bnk:3;
	} s;
	struct cvmx_lmcx_read_level_ctl_s cn52xx;
	struct cvmx_lmcx_read_level_ctl_s cn52xxp1;
	struct cvmx_lmcx_read_level_ctl_s cn56xx;
	struct cvmx_lmcx_read_level_ctl_s cn56xxp1;
};

/**
 * cvmx_lmc#_read_level_dbg
 *
 * Notes:
 * A given read of LMC*_READ_LEVEL_DBG returns the read-leveling pass/fail
 * results for all possible delay settings (i.e. the BITMASK) for only one
 * byte in the last rank that the HW read-leveled.
 * LMC*_READ_LEVEL_DBG[BYTE] selects the particular byte.
 * To get these pass/fail results for another different rank, you must run
 * the hardware read-leveling again. For example, it is possible to get the
 * BITMASK results for every byte of every rank if you run read-leveling
 * separately for each rank, probing LMC*_READ_LEVEL_DBG between each
 * read-leveling.
 */
union cvmx_lmcx_read_level_dbg {
	u64 u64;
	struct cvmx_lmcx_read_level_dbg_s {
		uint64_t reserved_32_63:32;
		uint64_t bitmask:16;
		uint64_t reserved_4_15:12;
		uint64_t byte:4;
	} s;
	struct cvmx_lmcx_read_level_dbg_s cn52xx;
	struct cvmx_lmcx_read_level_dbg_s cn52xxp1;
	struct cvmx_lmcx_read_level_dbg_s cn56xx;
	struct cvmx_lmcx_read_level_dbg_s cn56xxp1;
};

/**
 * cvmx_lmc#_read_level_rank#
 *
 * Notes:
 * This is four CSRs per LMC, one per each rank.
 * Each CSR is written by HW during a read-leveling sequence for the rank.
 * (HW sets STATUS==3 after HW read-leveling completes for the rank.)
 * Each CSR may also be written by SW, but not while a read-leveling sequence
 * is in progress. (HW sets STATUS==1 after a CSR write.)
 * Deskew setting is measured in units of 1/4 DCLK, so the above BYTE*
 * values can range over 4 DCLKs.
 * SW initiates a HW read-leveling sequence by programming
 * LMC*_READ_LEVEL_CTL and writing INIT_START=1 with SEQUENCE=1.
 * See LMC*_READ_LEVEL_CTL.
 */
union cvmx_lmcx_read_level_rankx {
	u64 u64;
	struct cvmx_lmcx_read_level_rankx_s {
		uint64_t reserved_38_63:26;
		uint64_t status:2;
		uint64_t byte8:4;
		uint64_t byte7:4;
		uint64_t byte6:4;
		uint64_t byte5:4;
		uint64_t byte4:4;
		uint64_t byte3:4;
		uint64_t byte2:4;
		uint64_t byte1:4;
		uint64_t byte0:4;
	} s;
	struct cvmx_lmcx_read_level_rankx_s cn52xx;
	struct cvmx_lmcx_read_level_rankx_s cn52xxp1;
	struct cvmx_lmcx_read_level_rankx_s cn56xx;
	struct cvmx_lmcx_read_level_rankx_s cn56xxp1;
};

/**
 * cvmx_lmc#_ref_status
 *
 * This register contains the status of the refresh pending counter.
 *
 */
union cvmx_lmcx_ref_status {
	u64 u64;
	struct cvmx_lmcx_ref_status_s {
		uint64_t reserved_4_63:60;
		uint64_t ref_pend_max_clr:1;
		uint64_t ref_count:3;
	} s;
	struct cvmx_lmcx_ref_status_s cn73xx;
	struct cvmx_lmcx_ref_status_s cn78xx;
	struct cvmx_lmcx_ref_status_s cnf75xx;
};

/**
 * cvmx_lmc#_reset_ctl
 *
 * Specify the RSL base addresses for the block.
 *
 */
union cvmx_lmcx_reset_ctl {
	u64 u64;
	struct cvmx_lmcx_reset_ctl_s {
		uint64_t reserved_4_63:60;
		uint64_t ddr3psv:1;
		uint64_t ddr3psoft:1;
		uint64_t ddr3pwarm:1;
		uint64_t ddr3rst:1;
	} s;
	struct cvmx_lmcx_reset_ctl_s cn61xx;
	struct cvmx_lmcx_reset_ctl_s cn63xx;
	struct cvmx_lmcx_reset_ctl_s cn63xxp1;
	struct cvmx_lmcx_reset_ctl_s cn66xx;
	struct cvmx_lmcx_reset_ctl_s cn68xx;
	struct cvmx_lmcx_reset_ctl_s cn68xxp1;
	struct cvmx_lmcx_reset_ctl_s cn70xx;
	struct cvmx_lmcx_reset_ctl_s cn70xxp1;
	struct cvmx_lmcx_reset_ctl_s cn73xx;
	struct cvmx_lmcx_reset_ctl_s cn78xx;
	struct cvmx_lmcx_reset_ctl_s cn78xxp1;
	struct cvmx_lmcx_reset_ctl_s cnf71xx;
	struct cvmx_lmcx_reset_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_retry_config
 *
 * This register configures automatic retry operation.
 *
 */
union cvmx_lmcx_retry_config {
	u64 u64;
	struct cvmx_lmcx_retry_config_s {
		uint64_t reserved_56_63:8;
		uint64_t max_errors:24;
		uint64_t reserved_13_31:19;
		uint64_t error_continue:1;
		uint64_t reserved_9_11:3;
		uint64_t auto_error_continue:1;
		uint64_t reserved_5_7:3;
		uint64_t pulse_count_auto_clr:1;
		uint64_t reserved_1_3:3;
		uint64_t retry_enable:1;
	} s;
	struct cvmx_lmcx_retry_config_s cn73xx;
	struct cvmx_lmcx_retry_config_s cn78xx;
	struct cvmx_lmcx_retry_config_s cnf75xx;
};

/**
 * cvmx_lmc#_retry_status
 *
 * This register provides status on automatic retry operation.
 *
 */
union cvmx_lmcx_retry_status {
	u64 u64;
	struct cvmx_lmcx_retry_status_s {
		uint64_t clear_error_count:1;
		uint64_t clear_error_pulse_count:1;
		uint64_t reserved_57_61:5;
		uint64_t error_pulse_count_valid:1;
		uint64_t error_pulse_count_sat:1;
		uint64_t reserved_52_54:3;
		uint64_t error_pulse_count:4;
		uint64_t reserved_45_47:3;
		uint64_t error_sequence:5;
		uint64_t reserved_33_39:7;
		uint64_t error_type:1;
		uint64_t reserved_24_31:8;
		uint64_t error_count:24;
	} s;
	struct cvmx_lmcx_retry_status_s cn73xx;
	struct cvmx_lmcx_retry_status_s cn78xx;
	struct cvmx_lmcx_retry_status_s cnf75xx;
};

/**
 * cvmx_lmc#_rlevel_ctl
 */
union cvmx_lmcx_rlevel_ctl {
	u64 u64;
	struct cvmx_lmcx_rlevel_ctl_s {
		uint64_t reserved_33_63:31;
		uint64_t tccd_sel:1;
		uint64_t pattern:8;
		uint64_t reserved_22_23:2;
		uint64_t delay_unload_3:1;
		uint64_t delay_unload_2:1;
		uint64_t delay_unload_1:1;
		uint64_t delay_unload_0:1;
		uint64_t bitmask:8;
		uint64_t or_dis:1;
		uint64_t offset_en:1;
		uint64_t offset:4;
		uint64_t byte:4;
	} s;
	struct cvmx_lmcx_rlevel_ctl_cn61xx {
		uint64_t reserved_22_63:42;
		uint64_t delay_unload_3:1;
		uint64_t delay_unload_2:1;
		uint64_t delay_unload_1:1;
		uint64_t delay_unload_0:1;
		uint64_t bitmask:8;
		uint64_t or_dis:1;
		uint64_t offset_en:1;
		uint64_t offset:4;
		uint64_t byte:4;
	} cn61xx;
	struct cvmx_lmcx_rlevel_ctl_cn61xx cn63xx;
	struct cvmx_lmcx_rlevel_ctl_cn63xxp1 {
		uint64_t reserved_9_63:55;
		uint64_t offset_en:1;
		uint64_t offset:4;
		uint64_t byte:4;
	} cn63xxp1;
	struct cvmx_lmcx_rlevel_ctl_cn61xx cn66xx;
	struct cvmx_lmcx_rlevel_ctl_cn61xx cn68xx;
	struct cvmx_lmcx_rlevel_ctl_cn61xx cn68xxp1;
	struct cvmx_lmcx_rlevel_ctl_cn70xx {
		uint64_t reserved_32_63:32;
		uint64_t pattern:8;
		uint64_t reserved_22_23:2;
		uint64_t delay_unload_3:1;
		uint64_t delay_unload_2:1;
		uint64_t delay_unload_1:1;
		uint64_t delay_unload_0:1;
		uint64_t bitmask:8;
		uint64_t or_dis:1;
		uint64_t offset_en:1;
		uint64_t offset:4;
		uint64_t byte:4;
	} cn70xx;
	struct cvmx_lmcx_rlevel_ctl_cn70xx cn70xxp1;
	struct cvmx_lmcx_rlevel_ctl_cn70xx cn73xx;
	struct cvmx_lmcx_rlevel_ctl_s cn78xx;
	struct cvmx_lmcx_rlevel_ctl_s cn78xxp1;
	struct cvmx_lmcx_rlevel_ctl_cn61xx cnf71xx;
	struct cvmx_lmcx_rlevel_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_rlevel_dbg
 *
 * A given read of LMC()_RLEVEL_DBG returns the read leveling pass/fail
 * results for all possible delay settings (i.e. the BITMASK) for only
 * one byte in the last rank that the hardware ran read leveling on.
 * LMC()_RLEVEL_CTL[BYTE] selects the particular byte. To get these
 * pass/fail results for a different rank, you must run the hardware
 * read leveling again. For example, it is possible to get the [BITMASK]
 * results for every byte of every rank if you run read leveling separately
 * for each rank, probing LMC()_RLEVEL_DBG between each read- leveling.
 */
union cvmx_lmcx_rlevel_dbg {
	u64 u64;
	struct cvmx_lmcx_rlevel_dbg_s {
		uint64_t bitmask:64;
	} s;
	struct cvmx_lmcx_rlevel_dbg_s cn61xx;
	struct cvmx_lmcx_rlevel_dbg_s cn63xx;
	struct cvmx_lmcx_rlevel_dbg_s cn63xxp1;
	struct cvmx_lmcx_rlevel_dbg_s cn66xx;
	struct cvmx_lmcx_rlevel_dbg_s cn68xx;
	struct cvmx_lmcx_rlevel_dbg_s cn68xxp1;
	struct cvmx_lmcx_rlevel_dbg_s cn70xx;
	struct cvmx_lmcx_rlevel_dbg_s cn70xxp1;
	struct cvmx_lmcx_rlevel_dbg_s cn73xx;
	struct cvmx_lmcx_rlevel_dbg_s cn78xx;
	struct cvmx_lmcx_rlevel_dbg_s cn78xxp1;
	struct cvmx_lmcx_rlevel_dbg_s cnf71xx;
	struct cvmx_lmcx_rlevel_dbg_s cnf75xx;
};

/**
 * cvmx_lmc#_rlevel_rank#
 *
 * Four of these CSRs exist per LMC, one for each rank. Read level setting
 * is measured in units of 1/4 CK, so the BYTEn values can range over 16 CK
 * cycles. Each CSR is written by hardware during a read leveling sequence
 * for the rank. (Hardware sets [STATUS] to 3 after hardware read leveling
 * completes for the rank.)
 *
 * If hardware is unable to find a match per LMC()_RLEVEL_CTL[OFFSET_EN] and
 * LMC()_RLEVEL_CTL[OFFSET], then hardware sets
 * LMC()_RLEVEL_RANK()[BYTEn<5:0>] to 0x0.
 *
 * Each CSR may also be written by software, but not while a read leveling
 * sequence is in progress. (Hardware sets [STATUS] to 1 after a CSR write.)
 * Software initiates a hardware read leveling sequence by programming
 * LMC()_RLEVEL_CTL and writing [INIT_START] = 1 with [SEQ_SEL]=1.
 * See LMC()_RLEVEL_CTL.
 *
 * LMC()_RLEVEL_RANKi values for ranks i without attached DRAM should be set
 * such that they do not increase the range of possible BYTE values for any
 * byte lane. The easiest way to do this is to set LMC()_RLEVEL_RANKi =
 * LMC()_RLEVEL_RANKj, where j is some rank with attached DRAM whose
 * LMC()_RLEVEL_RANKj is already fully initialized.
 */
union cvmx_lmcx_rlevel_rankx {
	u64 u64;
	struct cvmx_lmcx_rlevel_rankx_s {
		uint64_t reserved_56_63:8;
		uint64_t status:2;
		uint64_t byte8:6;
		uint64_t byte7:6;
		uint64_t byte6:6;
		uint64_t byte5:6;
		uint64_t byte4:6;
		uint64_t byte3:6;
		uint64_t byte2:6;
		uint64_t byte1:6;
		uint64_t byte0:6;
	} s;
	struct cvmx_lmcx_rlevel_rankx_s cn61xx;
	struct cvmx_lmcx_rlevel_rankx_s cn63xx;
	struct cvmx_lmcx_rlevel_rankx_s cn63xxp1;
	struct cvmx_lmcx_rlevel_rankx_s cn66xx;
	struct cvmx_lmcx_rlevel_rankx_s cn68xx;
	struct cvmx_lmcx_rlevel_rankx_s cn68xxp1;
	struct cvmx_lmcx_rlevel_rankx_s cn70xx;
	struct cvmx_lmcx_rlevel_rankx_s cn70xxp1;
	struct cvmx_lmcx_rlevel_rankx_s cn73xx;
	struct cvmx_lmcx_rlevel_rankx_s cn78xx;
	struct cvmx_lmcx_rlevel_rankx_s cn78xxp1;
	struct cvmx_lmcx_rlevel_rankx_s cnf71xx;
	struct cvmx_lmcx_rlevel_rankx_s cnf75xx;
};

/**
 * cvmx_lmc#_rodt_comp_ctl
 *
 * LMC_RODT_COMP_CTL = LMC Compensation control
 *
 */
union cvmx_lmcx_rodt_comp_ctl {
	u64 u64;
	struct cvmx_lmcx_rodt_comp_ctl_s {
		uint64_t reserved_17_63:47;
		uint64_t enable:1;
		uint64_t reserved_12_15:4;
		uint64_t nctl:4;
		uint64_t reserved_5_7:3;
		uint64_t pctl:5;
	} s;
	struct cvmx_lmcx_rodt_comp_ctl_s cn50xx;
	struct cvmx_lmcx_rodt_comp_ctl_s cn52xx;
	struct cvmx_lmcx_rodt_comp_ctl_s cn52xxp1;
	struct cvmx_lmcx_rodt_comp_ctl_s cn56xx;
	struct cvmx_lmcx_rodt_comp_ctl_s cn56xxp1;
	struct cvmx_lmcx_rodt_comp_ctl_s cn58xx;
	struct cvmx_lmcx_rodt_comp_ctl_s cn58xxp1;
};

/**
 * cvmx_lmc#_rodt_ctl
 *
 * LMC_RODT_CTL = Obsolete LMC Read OnDieTermination control
 * See the description in LMC_WODT_CTL1. On Reads, Octeon only supports
 * turning on ODT's in the lower 2 DIMM's with the masks as below.
 *
 * Notes:
 * When a given RANK in position N is selected, the RODT _HI and _LO masks
 * for that position are used.
 * Mask[3:0] is used for RODT control of the RANKs in positions 3, 2, 1,
 * and 0, respectively.
 * In  64b mode, DIMMs are assumed to be ordered in the following order:
 *  position 3: [unused        , DIMM1_RANK1_LO]
 *  position 2: [unused        , DIMM1_RANK0_LO]
 *  position 1: [unused        , DIMM0_RANK1_LO]
 *  position 0: [unused        , DIMM0_RANK0_LO]
 * In 128b mode, DIMMs are assumed to be ordered in the following order:
 *  position 3: [DIMM3_RANK1_HI, DIMM1_RANK1_LO]
 *  position 2: [DIMM3_RANK0_HI, DIMM1_RANK0_LO]
 *  position 1: [DIMM2_RANK1_HI, DIMM0_RANK1_LO]
 *  position 0: [DIMM2_RANK0_HI, DIMM0_RANK0_LO]
 */
union cvmx_lmcx_rodt_ctl {
	u64 u64;
	struct cvmx_lmcx_rodt_ctl_s {
		uint64_t reserved_32_63:32;
		uint64_t rodt_hi3:4;
		uint64_t rodt_hi2:4;
		uint64_t rodt_hi1:4;
		uint64_t rodt_hi0:4;
		uint64_t rodt_lo3:4;
		uint64_t rodt_lo2:4;
		uint64_t rodt_lo1:4;
		uint64_t rodt_lo0:4;
	} s;
	struct cvmx_lmcx_rodt_ctl_s cn30xx;
	struct cvmx_lmcx_rodt_ctl_s cn31xx;
	struct cvmx_lmcx_rodt_ctl_s cn38xx;
	struct cvmx_lmcx_rodt_ctl_s cn38xxp2;
	struct cvmx_lmcx_rodt_ctl_s cn50xx;
	struct cvmx_lmcx_rodt_ctl_s cn52xx;
	struct cvmx_lmcx_rodt_ctl_s cn52xxp1;
	struct cvmx_lmcx_rodt_ctl_s cn56xx;
	struct cvmx_lmcx_rodt_ctl_s cn56xxp1;
	struct cvmx_lmcx_rodt_ctl_s cn58xx;
	struct cvmx_lmcx_rodt_ctl_s cn58xxp1;
};

/**
 * cvmx_lmc#_rodt_mask
 *
 * System designers may desire to terminate DQ/DQS lines for higher frequency
 * DDR operations, especially on a multirank system. DDR3 DQ/DQS I/Os have
 * built-in termination resistors that can be turned on or off by the
 * controller, after meeting TAOND and TAOF timing requirements.
 *
 * Each rank has its own ODT pin that fans out to all the memory parts in
 * that DIMM. System designers may prefer different combinations of ODT ONs
 * for read operations into different ranks. CNXXXX supports full
 * programmability by way of the mask register below. Each rank position has
 * its own 4-bit programmable field. When the controller does a read to that
 * rank, it sets the 4 ODT pins to the MASK pins below. For example, when
 * doing a read from Rank0, a system designer may desire to terminate the
 * lines with the resistor on DIMM0/Rank1. The mask [RODT_D0_R0] would then
 * be [0010].
 *
 * CNXXXX drives the appropriate mask values on the ODT pins by default.
 * If this feature is not required, write 0x0 in this register. Note that,
 * as per the JEDEC DDR3 specifications, the ODT pin for the rank that is
 * being read should always be 0x0. When a given RANK is selected, the RODT
 * mask for that rank is used. The resulting RODT mask is driven to the
 * DIMMs in the following manner:
 */
union cvmx_lmcx_rodt_mask {
	u64 u64;
	struct cvmx_lmcx_rodt_mask_s {
		uint64_t rodt_d3_r1:8;
		uint64_t rodt_d3_r0:8;
		uint64_t rodt_d2_r1:8;
		uint64_t rodt_d2_r0:8;
		uint64_t rodt_d1_r1:8;
		uint64_t rodt_d1_r0:8;
		uint64_t rodt_d0_r1:8;
		uint64_t rodt_d0_r0:8;
	} s;
	struct cvmx_lmcx_rodt_mask_s cn61xx;
	struct cvmx_lmcx_rodt_mask_s cn63xx;
	struct cvmx_lmcx_rodt_mask_s cn63xxp1;
	struct cvmx_lmcx_rodt_mask_s cn66xx;
	struct cvmx_lmcx_rodt_mask_s cn68xx;
	struct cvmx_lmcx_rodt_mask_s cn68xxp1;
	struct cvmx_lmcx_rodt_mask_cn70xx {
		uint64_t reserved_28_63:36;
		uint64_t rodt_d1_r1:4;
		uint64_t reserved_20_23:4;
		uint64_t rodt_d1_r0:4;
		uint64_t reserved_12_15:4;
		uint64_t rodt_d0_r1:4;
		uint64_t reserved_4_7:4;
		uint64_t rodt_d0_r0:4;
	} cn70xx;
	struct cvmx_lmcx_rodt_mask_cn70xx cn70xxp1;
	struct cvmx_lmcx_rodt_mask_cn70xx cn73xx;
	struct cvmx_lmcx_rodt_mask_cn70xx cn78xx;
	struct cvmx_lmcx_rodt_mask_cn70xx cn78xxp1;
	struct cvmx_lmcx_rodt_mask_s cnf71xx;
	struct cvmx_lmcx_rodt_mask_cn70xx cnf75xx;
};

/**
 * cvmx_lmc#_scramble_cfg0
 *
 * LMC_SCRAMBLE_CFG0 = LMC Scramble Config0
 *
 */
union cvmx_lmcx_scramble_cfg0 {
	u64 u64;
	struct cvmx_lmcx_scramble_cfg0_s {
		uint64_t key:64;
	} s;
	struct cvmx_lmcx_scramble_cfg0_s cn61xx;
	struct cvmx_lmcx_scramble_cfg0_s cn66xx;
	struct cvmx_lmcx_scramble_cfg0_s cn70xx;
	struct cvmx_lmcx_scramble_cfg0_s cn70xxp1;
	struct cvmx_lmcx_scramble_cfg0_s cn73xx;
	struct cvmx_lmcx_scramble_cfg0_s cn78xx;
	struct cvmx_lmcx_scramble_cfg0_s cn78xxp1;
	struct cvmx_lmcx_scramble_cfg0_s cnf71xx;
	struct cvmx_lmcx_scramble_cfg0_s cnf75xx;
};

/**
 * cvmx_lmc#_scramble_cfg1
 *
 * These registers set the aliasing that uses the lowest, legal chip select(s).
 *
 */
union cvmx_lmcx_scramble_cfg1 {
	u64 u64;
	struct cvmx_lmcx_scramble_cfg1_s {
		uint64_t key:64;
	} s;
	struct cvmx_lmcx_scramble_cfg1_s cn61xx;
	struct cvmx_lmcx_scramble_cfg1_s cn66xx;
	struct cvmx_lmcx_scramble_cfg1_s cn70xx;
	struct cvmx_lmcx_scramble_cfg1_s cn70xxp1;
	struct cvmx_lmcx_scramble_cfg1_s cn73xx;
	struct cvmx_lmcx_scramble_cfg1_s cn78xx;
	struct cvmx_lmcx_scramble_cfg1_s cn78xxp1;
	struct cvmx_lmcx_scramble_cfg1_s cnf71xx;
	struct cvmx_lmcx_scramble_cfg1_s cnf75xx;
};

/**
 * cvmx_lmc#_scramble_cfg2
 */
union cvmx_lmcx_scramble_cfg2 {
	u64 u64;
	struct cvmx_lmcx_scramble_cfg2_s {
		uint64_t key:64;
	} s;
	struct cvmx_lmcx_scramble_cfg2_s cn73xx;
	struct cvmx_lmcx_scramble_cfg2_s cn78xx;
	struct cvmx_lmcx_scramble_cfg2_s cnf75xx;
};

/**
 * cvmx_lmc#_scrambled_fadr
 *
 * LMC()_FADR captures the failing pre-scrambled address location (split into
 * DIMM, bunk, bank, etc). If scrambling is off, LMC()_FADR also captures the
 * failing physical location in the DRAM parts. LMC()_SCRAMBLED_FADR captures
 * the actual failing address location in the physical DRAM parts, i.e.:
 *
 * * If scrambling is on, LMC()_SCRAMBLED_FADR contains the failing physical
 * location in the
 * DRAM parts (split into DIMM, bunk, bank, etc).
 *
 * * If scrambling is off, the pre-scramble and post-scramble addresses are
 * the same, and so the
 * contents of LMC()_SCRAMBLED_FADR match the contents of LMC()_FADR.
 *
 * This register only captures the first transaction with ECC errors. A DED
 * error can over-write this register with its failing addresses if the first
 * error was a SEC. If you write LMC()_CONFIG -> SEC_ERR/DED_ERR, it clears
 * the error bits and captures the next failing address. If [FDIMM] is 1,
 * that means the error is in the higher DIMM.
 */
union cvmx_lmcx_scrambled_fadr {
	u64 u64;
	struct cvmx_lmcx_scrambled_fadr_s {
		uint64_t reserved_43_63:21;
		uint64_t fcid:3;
		uint64_t fill_order:2;
		uint64_t reserved_14_37:24;
		uint64_t fcol:14;
	} s;
	struct cvmx_lmcx_scrambled_fadr_cn61xx {
		uint64_t reserved_36_63:28;
		uint64_t fdimm:2;
		uint64_t fbunk:1;
		uint64_t fbank:3;
		uint64_t frow:16;
		uint64_t fcol:14;
	} cn61xx;
	struct cvmx_lmcx_scrambled_fadr_cn61xx cn66xx;
	struct cvmx_lmcx_scrambled_fadr_cn70xx {
		uint64_t reserved_40_63:24;
		uint64_t fill_order:2;
		uint64_t fdimm:1;
		uint64_t fbunk:1;
		uint64_t fbank:4;
		uint64_t frow:18;
		uint64_t fcol:14;
	} cn70xx;
	struct cvmx_lmcx_scrambled_fadr_cn70xx cn70xxp1;
	struct cvmx_lmcx_scrambled_fadr_cn73xx {
		uint64_t reserved_43_63:21;
		uint64_t fcid:3;
		uint64_t fill_order:2;
		uint64_t fdimm:1;
		uint64_t fbunk:1;
		uint64_t fbank:4;
		uint64_t frow:18;
		uint64_t fcol:14;
	} cn73xx;
	struct cvmx_lmcx_scrambled_fadr_cn73xx cn78xx;
	struct cvmx_lmcx_scrambled_fadr_cn73xx cn78xxp1;
	struct cvmx_lmcx_scrambled_fadr_cn61xx cnf71xx;
	struct cvmx_lmcx_scrambled_fadr_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_seq_ctl
 *
 * This register is used to initiate the various control sequences in the LMC.
 *
 */
union cvmx_lmcx_seq_ctl {
	u64 u64;
	struct cvmx_lmcx_seq_ctl_s {
		uint64_t reserved_6_63:58;
		uint64_t seq_complete:1;
		uint64_t seq_sel:4;
		uint64_t init_start:1;
	} s;
	struct cvmx_lmcx_seq_ctl_s cn70xx;
	struct cvmx_lmcx_seq_ctl_s cn70xxp1;
	struct cvmx_lmcx_seq_ctl_s cn73xx;
	struct cvmx_lmcx_seq_ctl_s cn78xx;
	struct cvmx_lmcx_seq_ctl_s cn78xxp1;
	struct cvmx_lmcx_seq_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_slot_ctl0
 *
 * This register is an assortment of control fields needed by the memory
 * controller. If software has not previously written to this register
 * (since the last DRESET), hardware updates the fields in this register to
 * the minimum allowed value when any of LMC()_RLEVEL_RANK(),
 * LMC()_WLEVEL_RANK(), LMC()_CONTROL, and LMC()_MODEREG_PARAMS0 registers
 * change. Ideally, only read this register after LMC has been initialized and
 * LMC()_RLEVEL_RANK(), LMC()_WLEVEL_RANK() have valid data.
 *
 * The interpretation of the fields in this register depends on
 * LMC(0)_CONFIG[DDR2T]:
 *
 * * If LMC()_CONFIG[DDR2T]=1, (FieldValue + 4) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and
 * second types from different cache blocks.
 *
 * If LMC()_CONFIG[DDR2T]=0, (FieldValue + 3) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and second
 * types from different cache blocks.
 * FieldValue = 0 is always illegal in this case.
 * The hardware-calculated minimums for these fields are shown in
 * LMC(0)_SLOT_CTL0 Hardware-Calculated Minimums.
 */
union cvmx_lmcx_slot_ctl0 {
	u64 u64;
	struct cvmx_lmcx_slot_ctl0_s {
		uint64_t reserved_50_63:14;
		uint64_t w2r_l_init_ext:1;
		uint64_t w2r_init_ext:1;
		uint64_t w2w_l_init:6;
		uint64_t w2r_l_init:6;
		uint64_t r2w_l_init:6;
		uint64_t r2r_l_init:6;
		uint64_t w2w_init:6;
		uint64_t w2r_init:6;
		uint64_t r2w_init:6;
		uint64_t r2r_init:6;
	} s;
	struct cvmx_lmcx_slot_ctl0_cn61xx {
		uint64_t reserved_24_63:40;
		uint64_t w2w_init:6;
		uint64_t w2r_init:6;
		uint64_t r2w_init:6;
		uint64_t r2r_init:6;
	} cn61xx;
	struct cvmx_lmcx_slot_ctl0_cn61xx cn63xx;
	struct cvmx_lmcx_slot_ctl0_cn61xx cn63xxp1;
	struct cvmx_lmcx_slot_ctl0_cn61xx cn66xx;
	struct cvmx_lmcx_slot_ctl0_cn61xx cn68xx;
	struct cvmx_lmcx_slot_ctl0_cn61xx cn68xxp1;
	struct cvmx_lmcx_slot_ctl0_cn70xx {
		uint64_t reserved_48_63:16;
		uint64_t w2w_l_init:6;
		uint64_t w2r_l_init:6;
		uint64_t r2w_l_init:6;
		uint64_t r2r_l_init:6;
		uint64_t w2w_init:6;
		uint64_t w2r_init:6;
		uint64_t r2w_init:6;
		uint64_t r2r_init:6;
	} cn70xx;
	struct cvmx_lmcx_slot_ctl0_cn70xx cn70xxp1;
	struct cvmx_lmcx_slot_ctl0_s cn73xx;
	struct cvmx_lmcx_slot_ctl0_s cn78xx;
	struct cvmx_lmcx_slot_ctl0_s cn78xxp1;
	struct cvmx_lmcx_slot_ctl0_cn61xx cnf71xx;
	struct cvmx_lmcx_slot_ctl0_s cnf75xx;
};

/**
 * cvmx_lmc#_slot_ctl1
 *
 * This register is an assortment of control fields needed by the memory
 * controller. If software has not previously written to this register
 * (since the last DRESET), hardware updates the fields in this register to
 * the minimum allowed value when any of LMC()_RLEVEL_RANK(),
 * LMC()_WLEVEL_RANK(), LMC()_CONTROL and LMC()_MODEREG_PARAMS0 change.
 * Ideally, only read this register after LMC has been initialized and
 * LMC()_RLEVEL_RANK(), LMC()_WLEVEL_RANK() have valid data.
 *
 * The interpretation of the fields in this CSR depends on
 * LMC(0)_CONFIG[DDR2T]:
 *
 * * If LMC()_CONFIG[DDR2T]=1, (FieldValue + 4) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and
 * second types from different cache blocks.
 *
 * * If LMC()_CONFIG[DDR2T]=0, (FieldValue + 3) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and
 * second types from different cache blocks.
 * FieldValue = 0 is always illegal in this case.
 *
 * The hardware-calculated minimums for these fields are shown in
 * LMC(0)_SLOT_CTL1 Hardware-Calculated Minimums.
 */
union cvmx_lmcx_slot_ctl1 {
	u64 u64;
	struct cvmx_lmcx_slot_ctl1_s {
		uint64_t reserved_24_63:40;
		uint64_t w2w_xrank_init:6;
		uint64_t w2r_xrank_init:6;
		uint64_t r2w_xrank_init:6;
		uint64_t r2r_xrank_init:6;
	} s;
	struct cvmx_lmcx_slot_ctl1_s cn61xx;
	struct cvmx_lmcx_slot_ctl1_s cn63xx;
	struct cvmx_lmcx_slot_ctl1_s cn63xxp1;
	struct cvmx_lmcx_slot_ctl1_s cn66xx;
	struct cvmx_lmcx_slot_ctl1_s cn68xx;
	struct cvmx_lmcx_slot_ctl1_s cn68xxp1;
	struct cvmx_lmcx_slot_ctl1_s cn70xx;
	struct cvmx_lmcx_slot_ctl1_s cn70xxp1;
	struct cvmx_lmcx_slot_ctl1_s cn73xx;
	struct cvmx_lmcx_slot_ctl1_s cn78xx;
	struct cvmx_lmcx_slot_ctl1_s cn78xxp1;
	struct cvmx_lmcx_slot_ctl1_s cnf71xx;
	struct cvmx_lmcx_slot_ctl1_s cnf75xx;
};

/**
 * cvmx_lmc#_slot_ctl2
 *
 * This register is an assortment of control fields needed by the memory
 * controller. If software has not previously written to this register
 * (since the last DRESET), hardware updates the fields in this register
 * to the minimum allowed value when any of LMC()_RLEVEL_RANK(),
 * LMC()_WLEVEL_RANK(), LMC()_CONTROL and LMC()_MODEREG_PARAMS0 change.
 * Ideally, only read this register after LMC has been initialized and
 * LMC()_RLEVEL_RANK(), LMC()_WLEVEL_RANK() have valid data.
 *
 * The interpretation of the fields in this CSR depends on LMC(0)_CONFIG[DDR2T]:
 *
 * * If LMC()_CONFIG[DDR2T] = 1, (FieldValue + 4) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and
 * second types from different cache blocks.
 *
 * * If LMC()_CONFIG[DDR2T] = 0, (FieldValue + 3) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and second
 * types from different cache blocks.
 * FieldValue = 0 is always illegal in this case.
 *
 * The hardware-calculated minimums for these fields are shown in LMC Registers.
 */
union cvmx_lmcx_slot_ctl2 {
	u64 u64;
	struct cvmx_lmcx_slot_ctl2_s {
		uint64_t reserved_24_63:40;
		uint64_t w2w_xdimm_init:6;
		uint64_t w2r_xdimm_init:6;
		uint64_t r2w_xdimm_init:6;
		uint64_t r2r_xdimm_init:6;
	} s;
	struct cvmx_lmcx_slot_ctl2_s cn61xx;
	struct cvmx_lmcx_slot_ctl2_s cn63xx;
	struct cvmx_lmcx_slot_ctl2_s cn63xxp1;
	struct cvmx_lmcx_slot_ctl2_s cn66xx;
	struct cvmx_lmcx_slot_ctl2_s cn68xx;
	struct cvmx_lmcx_slot_ctl2_s cn68xxp1;
	struct cvmx_lmcx_slot_ctl2_s cn70xx;
	struct cvmx_lmcx_slot_ctl2_s cn70xxp1;
	struct cvmx_lmcx_slot_ctl2_s cn73xx;
	struct cvmx_lmcx_slot_ctl2_s cn78xx;
	struct cvmx_lmcx_slot_ctl2_s cn78xxp1;
	struct cvmx_lmcx_slot_ctl2_s cnf71xx;
	struct cvmx_lmcx_slot_ctl2_s cnf75xx;
};

/**
 * cvmx_lmc#_slot_ctl3
 *
 * This register is an assortment of control fields needed by the memory
 * controller. If software has not previously written to this register
 * (since the last DRESET), hardware updates the fields in this register
 * to the minimum allowed value when any of LMC()_RLEVEL_RANK(),
 * LMC()_WLEVEL_RANK(), LMC()_CONTROL and LMC()_MODEREG_PARAMS0 change.
 * Ideally, only read this register after LMC has been initialized and
 * LMC()_RLEVEL_RANK(), LMC()_WLEVEL_RANK() have valid data.
 *
 * The interpretation of the fields in this CSR depends on LMC(0)_CONFIG[DDR2T]:
 *
 * * If LMC()_CONFIG[DDR2T] = 1, (FieldValue + 4) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and
 * second types from different cache blocks.
 *
 * * If LMC()_CONFIG[DDR2T] = 0, (FieldValue + 3) is the minimum CK cycles
 * between when the DRAM part registers CAS commands of the first and second
 * types from different cache blocks.
 * FieldValue = 0 is always illegal in this case.
 *
 * The hardware-calculated minimums for these fields are shown in LMC Registers.
 */
union cvmx_lmcx_slot_ctl3 {
	u64 u64;
	struct cvmx_lmcx_slot_ctl3_s {
		uint64_t reserved_50_63:14;
		uint64_t w2r_l_xrank_init_ext:1;
		uint64_t w2r_xrank_init_ext:1;
		uint64_t w2w_l_xrank_init:6;
		uint64_t w2r_l_xrank_init:6;
		uint64_t r2w_l_xrank_init:6;
		uint64_t r2r_l_xrank_init:6;
		uint64_t w2w_xrank_init:6;
		uint64_t w2r_xrank_init:6;
		uint64_t r2w_xrank_init:6;
		uint64_t r2r_xrank_init:6;
	} s;
	struct cvmx_lmcx_slot_ctl3_s cn73xx;
	struct cvmx_lmcx_slot_ctl3_s cn78xx;
	struct cvmx_lmcx_slot_ctl3_s cnf75xx;
};

/**
 * cvmx_lmc#_timing_params0
 */
union cvmx_lmcx_timing_params0 {
	u64 u64;
	struct cvmx_lmcx_timing_params0_s {
		uint64_t reserved_54_63:10;
		uint64_t tbcw:6;
		uint64_t reserved_26_47:22;
		uint64_t tmrd:4;
		uint64_t reserved_8_21:14;
		uint64_t tckeon:8;
	} s;
	struct cvmx_lmcx_timing_params0_cn61xx {
		uint64_t reserved_47_63:17;
		uint64_t trp_ext:1;
		uint64_t tcksre:4;
		uint64_t trp:4;
		uint64_t tzqinit:4;
		uint64_t tdllk:4;
		uint64_t tmod:4;
		uint64_t tmrd:4;
		uint64_t txpr:4;
		uint64_t tcke:4;
		uint64_t tzqcs:4;
		uint64_t reserved_0_9:10;
	} cn61xx;
	struct cvmx_lmcx_timing_params0_cn61xx cn63xx;
	struct cvmx_lmcx_timing_params0_cn63xxp1 {
		uint64_t reserved_46_63:18;
		uint64_t tcksre:4;
		uint64_t trp:4;
		uint64_t tzqinit:4;
		uint64_t tdllk:4;
		uint64_t tmod:4;
		uint64_t tmrd:4;
		uint64_t txpr:4;
		uint64_t tcke:4;
		uint64_t tzqcs:4;
		uint64_t tckeon:10;
	} cn63xxp1;
	struct cvmx_lmcx_timing_params0_cn61xx cn66xx;
	struct cvmx_lmcx_timing_params0_cn61xx cn68xx;
	struct cvmx_lmcx_timing_params0_cn61xx cn68xxp1;
	struct cvmx_lmcx_timing_params0_cn70xx {
		uint64_t reserved_48_63:16;
		uint64_t tcksre:4;
		uint64_t trp:5;
		uint64_t tzqinit:4;
		uint64_t tdllk:4;
		uint64_t tmod:5;
		uint64_t tmrd:4;
		uint64_t txpr:6;
		uint64_t tcke:4;
		uint64_t tzqcs:4;
		uint64_t reserved_0_7:8;
	} cn70xx;
	struct cvmx_lmcx_timing_params0_cn70xx cn70xxp1;
	struct cvmx_lmcx_timing_params0_cn73xx {
		uint64_t reserved_54_63:10;
		uint64_t tbcw:6;
		uint64_t tcksre:4;
		uint64_t trp:5;
		uint64_t tzqinit:4;
		uint64_t tdllk:4;
		uint64_t tmod:5;
		uint64_t tmrd:4;
		uint64_t txpr:6;
		uint64_t tcke:4;
		uint64_t tzqcs:4;
		uint64_t reserved_0_7:8;
	} cn73xx;
	struct cvmx_lmcx_timing_params0_cn73xx cn78xx;
	struct cvmx_lmcx_timing_params0_cn73xx cn78xxp1;
	struct cvmx_lmcx_timing_params0_cn61xx cnf71xx;
	struct cvmx_lmcx_timing_params0_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_timing_params1
 */
union cvmx_lmcx_timing_params1 {
	u64 u64;
	struct cvmx_lmcx_timing_params1_s {
		uint64_t reserved_59_63:5;
		uint64_t txp_ext:1;
		uint64_t trcd_ext:1;
		uint64_t tpdm_full_cycle_ena:1;
		uint64_t trfc_dlr:7;
		uint64_t reserved_4_48:45;
		uint64_t tmprr:4;
	} s;
	struct cvmx_lmcx_timing_params1_cn61xx {
		uint64_t reserved_47_63:17;
		uint64_t tras_ext:1;
		uint64_t txpdll:5;
		uint64_t tfaw:5;
		uint64_t twldqsen:4;
		uint64_t twlmrd:4;
		uint64_t txp:3;
		uint64_t trrd:3;
		uint64_t trfc:5;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:5;
		uint64_t tmprr:4;
	} cn61xx;
	struct cvmx_lmcx_timing_params1_cn61xx cn63xx;
	struct cvmx_lmcx_timing_params1_cn63xxp1 {
		uint64_t reserved_46_63:18;
		uint64_t txpdll:5;
		uint64_t tfaw:5;
		uint64_t twldqsen:4;
		uint64_t twlmrd:4;
		uint64_t txp:3;
		uint64_t trrd:3;
		uint64_t trfc:5;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:5;
		uint64_t tmprr:4;
	} cn63xxp1;
	struct cvmx_lmcx_timing_params1_cn61xx cn66xx;
	struct cvmx_lmcx_timing_params1_cn61xx cn68xx;
	struct cvmx_lmcx_timing_params1_cn61xx cn68xxp1;
	struct cvmx_lmcx_timing_params1_cn70xx {
		uint64_t reserved_49_63:15;
		uint64_t txpdll:5;
		uint64_t tfaw:5;
		uint64_t twldqsen:4;
		uint64_t twlmrd:4;
		uint64_t txp:3;
		uint64_t trrd:3;
		uint64_t trfc:7;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:6;
		uint64_t tmprr:4;
	} cn70xx;
	struct cvmx_lmcx_timing_params1_cn70xx cn70xxp1;
	struct cvmx_lmcx_timing_params1_cn73xx {
		uint64_t reserved_59_63:5;
		uint64_t txp_ext:1;
		uint64_t trcd_ext:1;
		uint64_t tpdm_full_cycle_ena:1;
		uint64_t trfc_dlr:7;
		uint64_t txpdll:5;
		uint64_t tfaw:5;
		uint64_t twldqsen:4;
		uint64_t twlmrd:4;
		uint64_t txp:3;
		uint64_t trrd:3;
		uint64_t trfc:7;
		uint64_t twtr:4;
		uint64_t trcd:4;
		uint64_t tras:6;
		uint64_t tmprr:4;
	} cn73xx;
	struct cvmx_lmcx_timing_params1_cn73xx cn78xx;
	struct cvmx_lmcx_timing_params1_cn73xx cn78xxp1;
	struct cvmx_lmcx_timing_params1_cn61xx cnf71xx;
	struct cvmx_lmcx_timing_params1_cn73xx cnf75xx;
};

/**
 * cvmx_lmc#_timing_params2
 *
 * This register sets timing parameters for DDR4.
 *
 */
union cvmx_lmcx_timing_params2 {
	u64 u64;
	struct cvmx_lmcx_timing_params2_s {
		uint64_t reserved_16_63:48;
		uint64_t trrd_l_ext:1;
		uint64_t trtp:4;
		uint64_t t_rw_op_max:4;
		uint64_t twtr_l:4;
		uint64_t trrd_l:3;
	} s;
	struct cvmx_lmcx_timing_params2_cn70xx {
		uint64_t reserved_15_63:49;
		uint64_t trtp:4;
		uint64_t t_rw_op_max:4;
		uint64_t twtr_l:4;
		uint64_t trrd_l:3;
	} cn70xx;
	struct cvmx_lmcx_timing_params2_cn70xx cn70xxp1;
	struct cvmx_lmcx_timing_params2_s cn73xx;
	struct cvmx_lmcx_timing_params2_s cn78xx;
	struct cvmx_lmcx_timing_params2_s cn78xxp1;
	struct cvmx_lmcx_timing_params2_s cnf75xx;
};

/**
 * cvmx_lmc#_tro_ctl
 *
 * LMC_TRO_CTL = LMC Temperature Ring Osc Control
 * This register is an assortment of various control fields needed to
 * control the temperature ring oscillator
 *
 * Notes:
 * To bring up the temperature ring oscillator, write TRESET to 0, and
 * follow by initializing RCLK_CNT to desired value
 */
union cvmx_lmcx_tro_ctl {
	u64 u64;
	struct cvmx_lmcx_tro_ctl_s {
		uint64_t reserved_33_63:31;
		uint64_t rclk_cnt:32;
		uint64_t treset:1;
	} s;
	struct cvmx_lmcx_tro_ctl_s cn61xx;
	struct cvmx_lmcx_tro_ctl_s cn63xx;
	struct cvmx_lmcx_tro_ctl_s cn63xxp1;
	struct cvmx_lmcx_tro_ctl_s cn66xx;
	struct cvmx_lmcx_tro_ctl_s cn68xx;
	struct cvmx_lmcx_tro_ctl_s cn68xxp1;
	struct cvmx_lmcx_tro_ctl_s cnf71xx;
};

/**
 * cvmx_lmc#_tro_stat
 *
 * LMC_TRO_STAT = LMC Temperature Ring Osc Status
 * This register is an assortment of various control fields needed to
 * control the temperature ring oscillator
 */
union cvmx_lmcx_tro_stat {
	u64 u64;
	struct cvmx_lmcx_tro_stat_s {
		uint64_t reserved_32_63:32;
		uint64_t ring_cnt:32;
	} s;
	struct cvmx_lmcx_tro_stat_s cn61xx;
	struct cvmx_lmcx_tro_stat_s cn63xx;
	struct cvmx_lmcx_tro_stat_s cn63xxp1;
	struct cvmx_lmcx_tro_stat_s cn66xx;
	struct cvmx_lmcx_tro_stat_s cn68xx;
	struct cvmx_lmcx_tro_stat_s cn68xxp1;
	struct cvmx_lmcx_tro_stat_s cnf71xx;
};

/**
 * cvmx_lmc#_wlevel_ctl
 */
union cvmx_lmcx_wlevel_ctl {
	u64 u64;
	struct cvmx_lmcx_wlevel_ctl_s {
		uint64_t reserved_22_63:42;
		uint64_t rtt_nom:3;
		uint64_t bitmask:8;
		uint64_t or_dis:1;
		uint64_t sset:1;
		uint64_t lanemask:9;
	} s;
	struct cvmx_lmcx_wlevel_ctl_s cn61xx;
	struct cvmx_lmcx_wlevel_ctl_s cn63xx;
	struct cvmx_lmcx_wlevel_ctl_cn63xxp1 {
		uint64_t reserved_10_63:54;
		uint64_t sset:1;
		uint64_t lanemask:9;
	} cn63xxp1;
	struct cvmx_lmcx_wlevel_ctl_s cn66xx;
	struct cvmx_lmcx_wlevel_ctl_s cn68xx;
	struct cvmx_lmcx_wlevel_ctl_s cn68xxp1;
	struct cvmx_lmcx_wlevel_ctl_s cn70xx;
	struct cvmx_lmcx_wlevel_ctl_s cn70xxp1;
	struct cvmx_lmcx_wlevel_ctl_s cn73xx;
	struct cvmx_lmcx_wlevel_ctl_s cn78xx;
	struct cvmx_lmcx_wlevel_ctl_s cn78xxp1;
	struct cvmx_lmcx_wlevel_ctl_s cnf71xx;
	struct cvmx_lmcx_wlevel_ctl_s cnf75xx;
};

/**
 * cvmx_lmc#_wlevel_dbg
 *
 * A given write of LMC()_WLEVEL_DBG returns the write leveling pass/fail
 * results for all possible delay settings (i.e. the BITMASK) for only one
 * byte in the last rank that the hardware write leveled.
 * LMC()_WLEVEL_DBG[BYTE] selects the particular byte. To get these
 * pass/fail results for a different rank, you must run the hardware write
 * leveling again. For example, it is possible to get the [BITMASK] results
 * for every byte of every rank if you run write leveling separately for
 * each rank, probing LMC()_WLEVEL_DBG between each write-leveling.
 */
union cvmx_lmcx_wlevel_dbg {
	u64 u64;
	struct cvmx_lmcx_wlevel_dbg_s {
		uint64_t reserved_12_63:52;
		uint64_t bitmask:8;
		uint64_t byte:4;
	} s;
	struct cvmx_lmcx_wlevel_dbg_s cn61xx;
	struct cvmx_lmcx_wlevel_dbg_s cn63xx;
	struct cvmx_lmcx_wlevel_dbg_s cn63xxp1;
	struct cvmx_lmcx_wlevel_dbg_s cn66xx;
	struct cvmx_lmcx_wlevel_dbg_s cn68xx;
	struct cvmx_lmcx_wlevel_dbg_s cn68xxp1;
	struct cvmx_lmcx_wlevel_dbg_s cn70xx;
	struct cvmx_lmcx_wlevel_dbg_s cn70xxp1;
	struct cvmx_lmcx_wlevel_dbg_s cn73xx;
	struct cvmx_lmcx_wlevel_dbg_s cn78xx;
	struct cvmx_lmcx_wlevel_dbg_s cn78xxp1;
	struct cvmx_lmcx_wlevel_dbg_s cnf71xx;
	struct cvmx_lmcx_wlevel_dbg_s cnf75xx;
};

/**
 * cvmx_lmc#_wlevel_rank#
 *
 * Four of these CSRs exist per LMC, one for each rank. Write level setting
 * is measured in units of 1/8 CK, so the below BYTEn values can range over
 * 4 CK cycles. Assuming LMC()_WLEVEL_CTL[SSET]=0, the BYTEn<2:0> values are
 * not used during write leveling, and they are overwritten by the hardware
 * as part of the write leveling sequence. (Hardware sets [STATUS] to 3 after
 * hardware write leveling completes for the rank). Software needs to set
 * BYTEn<4:3> bits.
 *
 * Each CSR may also be written by software, but not while a write leveling
 * sequence is in progress. (Hardware sets [STATUS] to 1 after a CSR write.)
 * Software initiates a hardware write-leveling sequence by programming
 * LMC()_WLEVEL_CTL and writing RANKMASK and INIT_START=1 with SEQ_SEL=6 in
 * LMC*0_CONFIG.
 *
 * LMC will then step through and accumulate write leveling results for 8
 * unique delay settings (twice), starting at a delay of LMC()_WLEVEL_RANK()
 * [BYTEn<4:3>]* 8 CK increasing by 1/8 CK each setting. Hardware will then
 * set LMC()_WLEVEL_RANK()[BYTEn<2:0>] to indicate the first write leveling
 * result of 1 that followed a result of 0 during the sequence by searching
 * for a '1100' pattern in the generated bitmask, except that LMC will always
 * write LMC()_WLEVEL_RANK()[BYTEn<0>]=0. If hardware is unable to find a match
 * for a '1100' pattern, then hardware sets LMC()_WLEVEL_RANK() [BYTEn<2:0>]
 * to 0x4. See LMC()_WLEVEL_CTL.
 *
 * LMC()_WLEVEL_RANKi values for ranks i without attached DRAM should be set
 * such that they do not increase the range of possible BYTE values for any
 * byte lane. The easiest way to do this is to set LMC()_WLEVEL_RANKi =
 * LMC()_WLEVEL_RANKj, where j is some rank with attached DRAM whose
 * LMC()_WLEVEL_RANKj is already fully initialized.
 */
union cvmx_lmcx_wlevel_rankx {
	u64 u64;
	struct cvmx_lmcx_wlevel_rankx_s {
		uint64_t reserved_47_63:17;
		uint64_t status:2;
		uint64_t byte8:5;
		uint64_t byte7:5;
		uint64_t byte6:5;
		uint64_t byte5:5;
		uint64_t byte4:5;
		uint64_t byte3:5;
		uint64_t byte2:5;
		uint64_t byte1:5;
		uint64_t byte0:5;
	} s;
	struct cvmx_lmcx_wlevel_rankx_s cn61xx;
	struct cvmx_lmcx_wlevel_rankx_s cn63xx;
	struct cvmx_lmcx_wlevel_rankx_s cn63xxp1;
	struct cvmx_lmcx_wlevel_rankx_s cn66xx;
	struct cvmx_lmcx_wlevel_rankx_s cn68xx;
	struct cvmx_lmcx_wlevel_rankx_s cn68xxp1;
	struct cvmx_lmcx_wlevel_rankx_s cn70xx;
	struct cvmx_lmcx_wlevel_rankx_s cn70xxp1;
	struct cvmx_lmcx_wlevel_rankx_s cn73xx;
	struct cvmx_lmcx_wlevel_rankx_s cn78xx;
	struct cvmx_lmcx_wlevel_rankx_s cn78xxp1;
	struct cvmx_lmcx_wlevel_rankx_s cnf71xx;
	struct cvmx_lmcx_wlevel_rankx_s cnf75xx;
};

/**
 * cvmx_lmc#_wodt_ctl0
 *
 * LMC_WODT_CTL0 = LMC Write OnDieTermination control
 * See the description in LMC_WODT_CTL1.
 *
 * Notes:
 * Together, the LMC_WODT_CTL1 and LMC_WODT_CTL0 CSRs control the write
 * ODT mask.  See LMC_WODT_CTL1.
 *
 */
union cvmx_lmcx_wodt_ctl0 {
	u64 u64;
	struct cvmx_lmcx_wodt_ctl0_s {
		uint64_t reserved_0_63:64;
	} s;
	struct cvmx_lmcx_wodt_ctl0_cn30xx {
		uint64_t reserved_32_63:32;
		uint64_t wodt_d1_r1:8;
		uint64_t wodt_d1_r0:8;
		uint64_t wodt_d0_r1:8;
		uint64_t wodt_d0_r0:8;
	} cn30xx;
	struct cvmx_lmcx_wodt_ctl0_cn30xx cn31xx;
	struct cvmx_lmcx_wodt_ctl0_cn38xx {
		uint64_t reserved_32_63:32;
		uint64_t wodt_hi3:4;
		uint64_t wodt_hi2:4;
		uint64_t wodt_hi1:4;
		uint64_t wodt_hi0:4;
		uint64_t wodt_lo3:4;
		uint64_t wodt_lo2:4;
		uint64_t wodt_lo1:4;
		uint64_t wodt_lo0:4;
	} cn38xx;
	struct cvmx_lmcx_wodt_ctl0_cn38xx cn38xxp2;
	struct cvmx_lmcx_wodt_ctl0_cn38xx cn50xx;
	struct cvmx_lmcx_wodt_ctl0_cn30xx cn52xx;
	struct cvmx_lmcx_wodt_ctl0_cn30xx cn52xxp1;
	struct cvmx_lmcx_wodt_ctl0_cn30xx cn56xx;
	struct cvmx_lmcx_wodt_ctl0_cn30xx cn56xxp1;
	struct cvmx_lmcx_wodt_ctl0_cn38xx cn58xx;
	struct cvmx_lmcx_wodt_ctl0_cn38xx cn58xxp1;
};

/**
 * cvmx_lmc#_wodt_ctl1
 *
 * LMC_WODT_CTL1 = LMC Write OnDieTermination control
 * System designers may desire to terminate DQ/DQS/DM lines for higher
 * frequency DDR operations (667MHz and faster), especially on a multi-rank
 * system. DDR2 DQ/DM/DQS I/O's have built in Termination resistor that can
 * be turned on or off by the controller, after meeting tAOND and tAOF
 * timing requirements. Each Rank has its own ODT pin that fans out to all
 * the memory parts in that DIMM. System designers may prefer different
 * combinations of ODT ON's for read and write into different ranks. Octeon
 * supports full programmability by way of the mask register below.
 * Each Rank position has its own 8-bit programmable field.
 * When the controller does a write to that rank, it sets the 8 ODT pins
 * to the MASK pins below. For eg., When doing a write into Rank0, a system
 * designer may desire to terminate the lines with the resistor on
 * Dimm0/Rank1. The mask WODT_D0_R0 would then be [00000010]. If ODT feature
 * is not desired, the DDR parts can be programmed to not look at these pins by
 * writing 0 in QS_DIC. Octeon drives the appropriate mask values on the ODT
 * pins by default.
 * If this feature is not required, write 0 in this register.
 *
 * Notes:
 * Together, the LMC_WODT_CTL1 and LMC_WODT_CTL0 CSRs control the write
 * ODT mask. When a given RANK is selected, the WODT mask for that RANK
 * is used.  The resulting WODT mask is driven to the DIMMs in the following
 * manner:
 *            BUNK_ENA=1     BUNK_ENA=0
 * Mask[7] -> DIMM3, RANK1    DIMM3
 * Mask[6] -> DIMM3, RANK0
 * Mask[5] -> DIMM2, RANK1    DIMM2
 * Mask[4] -> DIMM2, RANK0
 * Mask[3] -> DIMM1, RANK1    DIMM1
 * Mask[2] -> DIMM1, RANK0
 * Mask[1] -> DIMM0, RANK1    DIMM0
 * Mask[0] -> DIMM0, RANK0
 */
union cvmx_lmcx_wodt_ctl1 {
	u64 u64;
	struct cvmx_lmcx_wodt_ctl1_s {
		uint64_t reserved_32_63:32;
		uint64_t wodt_d3_r1:8;
		uint64_t wodt_d3_r0:8;
		uint64_t wodt_d2_r1:8;
		uint64_t wodt_d2_r0:8;
	} s;
	struct cvmx_lmcx_wodt_ctl1_s cn30xx;
	struct cvmx_lmcx_wodt_ctl1_s cn31xx;
	struct cvmx_lmcx_wodt_ctl1_s cn52xx;
	struct cvmx_lmcx_wodt_ctl1_s cn52xxp1;
	struct cvmx_lmcx_wodt_ctl1_s cn56xx;
	struct cvmx_lmcx_wodt_ctl1_s cn56xxp1;
};

/**
 * cvmx_lmc#_wodt_mask
 *
 * System designers may desire to terminate DQ/DQS lines for higher-frequency
 * DDR operations, especially on a multirank system. DDR3 DQ/DQS I/Os have
 * built-in termination resistors that can be turned on or off by the
 * controller, after meeting TAOND and TAOF timing requirements. Each rank
 * has its own ODT pin that fans out to all of the memory parts in that DIMM.
 * System designers may prefer different combinations of ODT ONs for write
 * operations into different ranks. CNXXXX supports full programmability by
 * way of the mask register below. Each rank position has its own 8-bit
 * programmable field. When the controller does a write to that rank,
 * it sets the four ODT pins to the mask pins below. For example, when
 * doing a write into Rank0, a system designer may desire to terminate the
 * lines with the resistor on DIMM0/Rank1. The mask [WODT_D0_R0] would then
 * be [00000010].
 *
 * CNXXXX drives the appropriate mask values on the ODT pins by default.
 * If this feature is not required, write 0x0 in this register. When a
 * given RANK is selected, the WODT mask for that RANK is used. The
 * resulting WODT mask is driven to the DIMMs in the following manner:
 */
union cvmx_lmcx_wodt_mask {
	u64 u64;
	struct cvmx_lmcx_wodt_mask_s {
		uint64_t wodt_d3_r1:8;
		uint64_t wodt_d3_r0:8;
		uint64_t wodt_d2_r1:8;
		uint64_t wodt_d2_r0:8;
		uint64_t wodt_d1_r1:8;
		uint64_t wodt_d1_r0:8;
		uint64_t wodt_d0_r1:8;
		uint64_t wodt_d0_r0:8;
	} s;
	struct cvmx_lmcx_wodt_mask_s cn61xx;
	struct cvmx_lmcx_wodt_mask_s cn63xx;
	struct cvmx_lmcx_wodt_mask_s cn63xxp1;
	struct cvmx_lmcx_wodt_mask_s cn66xx;
	struct cvmx_lmcx_wodt_mask_s cn68xx;
	struct cvmx_lmcx_wodt_mask_s cn68xxp1;
	struct cvmx_lmcx_wodt_mask_cn70xx {
		uint64_t reserved_28_63:36;
		uint64_t wodt_d1_r1:4;
		uint64_t reserved_20_23:4;
		uint64_t wodt_d1_r0:4;
		uint64_t reserved_12_15:4;
		uint64_t wodt_d0_r1:4;
		uint64_t reserved_4_7:4;
		uint64_t wodt_d0_r0:4;
	} cn70xx;
	struct cvmx_lmcx_wodt_mask_cn70xx cn70xxp1;
	struct cvmx_lmcx_wodt_mask_cn70xx cn73xx;
	struct cvmx_lmcx_wodt_mask_cn70xx cn78xx;
	struct cvmx_lmcx_wodt_mask_cn70xx cn78xxp1;
	struct cvmx_lmcx_wodt_mask_s cnf71xx;
	struct cvmx_lmcx_wodt_mask_cn70xx cnf75xx;
};

#endif
