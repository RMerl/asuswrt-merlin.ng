/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sso.
 */

#ifndef __CVMX_SSO_DEFS_H__
#define __CVMX_SSO_DEFS_H__

#define CVMX_SSO_ACTIVE_CYCLES		(0x00016700000010E8ull)
#define CVMX_SSO_ACTIVE_CYCLESX(offset) (0x0001670000001100ull + ((offset) & 3) * 8)
#define CVMX_SSO_AW_ADD			(0x0001670000002080ull)
#define CVMX_SSO_AW_CFG			(0x00016700000010F0ull)
#define CVMX_SSO_AW_ECO			(0x0001670000001030ull)
#define CVMX_SSO_AW_READ_ARB		(0x0001670000002090ull)
#define CVMX_SSO_AW_STATUS		(0x00016700000010E0ull)
#define CVMX_SSO_AW_TAG_LATENCY_PC	(0x00016700000020A8ull)
#define CVMX_SSO_AW_TAG_REQ_PC		(0x00016700000020A0ull)
#define CVMX_SSO_AW_WE			(0x0001670000001080ull)
#define CVMX_SSO_BIST_STAT		(0x0001670000001078ull)
#define CVMX_SSO_BIST_STATUS0		(0x0001670000001200ull)
#define CVMX_SSO_BIST_STATUS1		(0x0001670000001208ull)
#define CVMX_SSO_BIST_STATUS2		(0x0001670000001210ull)
#define CVMX_SSO_CFG			(0x0001670000001088ull)
#define CVMX_SSO_DS_PC			(0x0001670000001070ull)
#define CVMX_SSO_ECC_CTL0		(0x0001670000001280ull)
#define CVMX_SSO_ECC_CTL1		(0x0001670000001288ull)
#define CVMX_SSO_ECC_CTL2		(0x0001670000001290ull)
#define CVMX_SSO_ERR			(0x0001670000001038ull)
#define CVMX_SSO_ERR0			(0x0001670000001240ull)
#define CVMX_SSO_ERR1			(0x0001670000001248ull)
#define CVMX_SSO_ERR2			(0x0001670000001250ull)
#define CVMX_SSO_ERR_ENB		(0x0001670000001030ull)
#define CVMX_SSO_FIDX_ECC_CTL		(0x00016700000010D0ull)
#define CVMX_SSO_FIDX_ECC_ST		(0x00016700000010D8ull)
#define CVMX_SSO_FPAGE_CNT		(0x0001670000001090ull)
#define CVMX_SSO_GRPX_AQ_CNT(offset)	(0x0001670020000700ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_AQ_THR(offset)	(0x0001670020000800ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_DS_PC(offset)	(0x0001670020001400ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_EXT_PC(offset)	(0x0001670020001100ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_IAQ_THR(offset)	(0x0001670020000000ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_INT(offset)	(0x0001670020000400ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_INT_CNT(offset)	(0x0001670020000600ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_INT_THR(offset)	(0x0001670020000500ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_PRI(offset)	(0x0001670020000200ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_TAQ_THR(offset)	(0x0001670020000100ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_TS_PC(offset)	(0x0001670020001300ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_WA_PC(offset)	(0x0001670020001200ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GRPX_WS_PC(offset)	(0x0001670020001000ull + ((offset) & 255) * 0x10000ull)
#define CVMX_SSO_GWE_CFG		(0x0001670000001098ull)
#define CVMX_SSO_GWE_RANDOM		(0x00016700000010B0ull)
#define CVMX_SSO_GW_ECO			(0x0001670000001038ull)
#define CVMX_SSO_IDX_ECC_CTL		(0x00016700000010C0ull)
#define CVMX_SSO_IDX_ECC_ST		(0x00016700000010C8ull)
#define CVMX_SSO_IENTX_LINKS(offset)	(0x00016700A0060000ull + ((offset) & 4095) * 8)
#define CVMX_SSO_IENTX_PENDTAG(offset)	(0x00016700A0040000ull + ((offset) & 4095) * 8)
#define CVMX_SSO_IENTX_QLINKS(offset)	(0x00016700A0080000ull + ((offset) & 4095) * 8)
#define CVMX_SSO_IENTX_TAG(offset)	(0x00016700A0000000ull + ((offset) & 4095) * 8)
#define CVMX_SSO_IENTX_WQPGRP(offset)	(0x00016700A0020000ull + ((offset) & 4095) * 8)
#define CVMX_SSO_IPL_CONFX(offset)	(0x0001670080080000ull + ((offset) & 255) * 8)
#define CVMX_SSO_IPL_DESCHEDX(offset)	(0x0001670080060000ull + ((offset) & 255) * 8)
#define CVMX_SSO_IPL_FREEX(offset)	(0x0001670080000000ull + ((offset) & 7) * 8)
#define CVMX_SSO_IPL_IAQX(offset)	(0x0001670080040000ull + ((offset) & 255) * 8)
#define CVMX_SSO_IQ_CNTX(offset)	(0x0001670000009000ull + ((offset) & 7) * 8)
#define CVMX_SSO_IQ_COM_CNT		(0x0001670000001058ull)
#define CVMX_SSO_IQ_INT			(0x0001670000001048ull)
#define CVMX_SSO_IQ_INT_EN		(0x0001670000001050ull)
#define CVMX_SSO_IQ_THRX(offset)	(0x000167000000A000ull + ((offset) & 7) * 8)
#define CVMX_SSO_NOS_CNT		(0x0001670000001040ull)
#define CVMX_SSO_NW_TIM			(0x0001670000001028ull)
#define CVMX_SSO_OTH_ECC_CTL		(0x00016700000010B0ull)
#define CVMX_SSO_OTH_ECC_ST		(0x00016700000010B8ull)
#define CVMX_SSO_PAGE_CNT		(0x0001670000001090ull)
#define CVMX_SSO_PND_ECC_CTL		(0x00016700000010A0ull)
#define CVMX_SSO_PND_ECC_ST		(0x00016700000010A8ull)
#define CVMX_SSO_PPX_ARB(offset)	(0x0001670040000000ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_PPX_GRP_MSK(offset)	(0x0001670000006000ull + ((offset) & 31) * 8)
#define CVMX_SSO_PPX_QOS_PRI(offset)	(0x0001670000003000ull + ((offset) & 31) * 8)
#define CVMX_SSO_PPX_SX_GRPMSKX(a, b, c)                                                           \
	(0x0001670040001000ull + ((a) << 16) + ((b) << 5) + ((c) << 3))
#define CVMX_SSO_PP_STRICT	  (0x00016700000010E0ull)
#define CVMX_SSO_QOSX_RND(offset) (0x0001670000002000ull + ((offset) & 7) * 8)
#define CVMX_SSO_QOS_THRX(offset) (0x000167000000B000ull + ((offset) & 7) * 8)
#define CVMX_SSO_QOS_WE		  (0x0001670000001080ull)
#define CVMX_SSO_RESET		  CVMX_SSO_RESET_FUNC()
static inline u64 CVMX_SSO_RESET_FUNC(void)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x00016700000010F8ull;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x00016700000010F8ull;
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x00016700000010F8ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00016700000010F0ull;
	}
	return 0x00016700000010F8ull;
}

#define CVMX_SSO_RWQ_HEAD_PTRX(offset)	(0x000167000000C000ull + ((offset) & 7) * 8)
#define CVMX_SSO_RWQ_POP_FPTR		(0x000167000000C408ull)
#define CVMX_SSO_RWQ_PSH_FPTR		(0x000167000000C400ull)
#define CVMX_SSO_RWQ_TAIL_PTRX(offset)	(0x000167000000C200ull + ((offset) & 7) * 8)
#define CVMX_SSO_SL_PPX_LINKS(offset)	(0x0001670060000040ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_SL_PPX_PENDTAG(offset) (0x0001670060000000ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_SL_PPX_PENDWQP(offset) (0x0001670060000010ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_SL_PPX_TAG(offset)	(0x0001670060000020ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_SL_PPX_WQP(offset)	(0x0001670060000030ull + ((offset) & 63) * 0x10000ull)
#define CVMX_SSO_TAQX_LINK(offset)	(0x00016700C0000000ull + ((offset) & 2047) * 4096)
#define CVMX_SSO_TAQX_WAEX_TAG(offset, block_id)                                                   \
	(0x00016700D0000000ull + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16)
#define CVMX_SSO_TAQX_WAEX_WQP(offset, block_id)                                                   \
	(0x00016700D0000008ull + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16)
#define CVMX_SSO_TAQ_ADD		(0x00016700000020E0ull)
#define CVMX_SSO_TAQ_CNT		(0x00016700000020C0ull)
#define CVMX_SSO_TIAQX_STATUS(offset)	(0x00016700000C0000ull + ((offset) & 255) * 8)
#define CVMX_SSO_TOAQX_STATUS(offset)	(0x00016700000D0000ull + ((offset) & 255) * 8)
#define CVMX_SSO_TS_PC			(0x0001670000001068ull)
#define CVMX_SSO_WA_COM_PC		(0x0001670000001060ull)
#define CVMX_SSO_WA_PCX(offset)		(0x0001670000005000ull + ((offset) & 7) * 8)
#define CVMX_SSO_WQ_INT			(0x0001670000001000ull)
#define CVMX_SSO_WQ_INT_CNTX(offset)	(0x0001670000008000ull + ((offset) & 63) * 8)
#define CVMX_SSO_WQ_INT_PC		(0x0001670000001020ull)
#define CVMX_SSO_WQ_INT_THRX(offset)	(0x0001670000007000ull + ((offset) & 63) * 8)
#define CVMX_SSO_WQ_IQ_DIS		(0x0001670000001010ull)
#define CVMX_SSO_WS_CFG			(0x0001670000001088ull)
#define CVMX_SSO_WS_ECO			(0x0001670000001048ull)
#define CVMX_SSO_WS_PCX(offset)		(0x0001670000004000ull + ((offset) & 63) * 8)
#define CVMX_SSO_XAQX_HEAD_NEXT(offset) (0x00016700000A0000ull + ((offset) & 255) * 8)
#define CVMX_SSO_XAQX_HEAD_PTR(offset)	(0x0001670000080000ull + ((offset) & 255) * 8)
#define CVMX_SSO_XAQX_TAIL_NEXT(offset) (0x00016700000B0000ull + ((offset) & 255) * 8)
#define CVMX_SSO_XAQX_TAIL_PTR(offset)	(0x0001670000090000ull + ((offset) & 255) * 8)
#define CVMX_SSO_XAQ_AURA		(0x0001670000002100ull)
#define CVMX_SSO_XAQ_LATENCY_PC		(0x00016700000020B8ull)
#define CVMX_SSO_XAQ_REQ_PC		(0x00016700000020B0ull)

/**
 * cvmx_sso_active_cycles
 *
 * SSO_ACTIVE_CYCLES = SSO cycles SSO active
 *
 * This register counts every sclk cycle that the SSO clocks are active.
 * **NOTE: Added in pass 2.0
 */
union cvmx_sso_active_cycles {
	u64 u64;
	struct cvmx_sso_active_cycles_s {
		u64 act_cyc : 64;
	} s;
	struct cvmx_sso_active_cycles_s cn68xx;
};

typedef union cvmx_sso_active_cycles cvmx_sso_active_cycles_t;

/**
 * cvmx_sso_active_cycles#
 *
 * This register counts every coprocessor clock (SCLK) cycle that the SSO clocks are active.
 *
 */
union cvmx_sso_active_cyclesx {
	u64 u64;
	struct cvmx_sso_active_cyclesx_s {
		u64 act_cyc : 64;
	} s;
	struct cvmx_sso_active_cyclesx_s cn73xx;
	struct cvmx_sso_active_cyclesx_s cn78xx;
	struct cvmx_sso_active_cyclesx_s cn78xxp1;
	struct cvmx_sso_active_cyclesx_s cnf75xx;
};

typedef union cvmx_sso_active_cyclesx cvmx_sso_active_cyclesx_t;

/**
 * cvmx_sso_aw_add
 */
union cvmx_sso_aw_add {
	u64 u64;
	struct cvmx_sso_aw_add_s {
		u64 reserved_30_63 : 34;
		u64 rsvd_free : 14;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_sso_aw_add_s cn73xx;
	struct cvmx_sso_aw_add_s cn78xx;
	struct cvmx_sso_aw_add_s cn78xxp1;
	struct cvmx_sso_aw_add_s cnf75xx;
};

typedef union cvmx_sso_aw_add cvmx_sso_aw_add_t;

/**
 * cvmx_sso_aw_cfg
 *
 * This register controls the operation of the add-work block (AW).
 *
 */
union cvmx_sso_aw_cfg {
	u64 u64;
	struct cvmx_sso_aw_cfg_s {
		u64 reserved_9_63 : 55;
		u64 ldt_short : 1;
		u64 lol : 1;
		u64 xaq_alloc_dis : 1;
		u64 ocla_bp : 1;
		u64 xaq_byp_dis : 1;
		u64 stt : 1;
		u64 ldt : 1;
		u64 ldwb : 1;
		u64 rwen : 1;
	} s;
	struct cvmx_sso_aw_cfg_s cn73xx;
	struct cvmx_sso_aw_cfg_s cn78xx;
	struct cvmx_sso_aw_cfg_s cn78xxp1;
	struct cvmx_sso_aw_cfg_s cnf75xx;
};

typedef union cvmx_sso_aw_cfg cvmx_sso_aw_cfg_t;

/**
 * cvmx_sso_aw_eco
 */
union cvmx_sso_aw_eco {
	u64 u64;
	struct cvmx_sso_aw_eco_s {
		u64 reserved_8_63 : 56;
		u64 eco_rw : 8;
	} s;
	struct cvmx_sso_aw_eco_s cn73xx;
	struct cvmx_sso_aw_eco_s cnf75xx;
};

typedef union cvmx_sso_aw_eco cvmx_sso_aw_eco_t;

/**
 * cvmx_sso_aw_read_arb
 *
 * This register fine tunes the AW read arbiter and is for diagnostic use.
 *
 */
union cvmx_sso_aw_read_arb {
	u64 u64;
	struct cvmx_sso_aw_read_arb_s {
		u64 reserved_30_63 : 34;
		u64 xaq_lev : 6;
		u64 reserved_21_23 : 3;
		u64 xaq_min : 5;
		u64 reserved_14_15 : 2;
		u64 aw_tag_lev : 6;
		u64 reserved_5_7 : 3;
		u64 aw_tag_min : 5;
	} s;
	struct cvmx_sso_aw_read_arb_s cn73xx;
	struct cvmx_sso_aw_read_arb_s cn78xx;
	struct cvmx_sso_aw_read_arb_s cn78xxp1;
	struct cvmx_sso_aw_read_arb_s cnf75xx;
};

typedef union cvmx_sso_aw_read_arb cvmx_sso_aw_read_arb_t;

/**
 * cvmx_sso_aw_status
 *
 * This register indicates the status of the add-work block (AW).
 *
 */
union cvmx_sso_aw_status {
	u64 u64;
	struct cvmx_sso_aw_status_s {
		u64 reserved_6_63 : 58;
		u64 xaq_buf_cached : 6;
	} s;
	struct cvmx_sso_aw_status_s cn73xx;
	struct cvmx_sso_aw_status_s cn78xx;
	struct cvmx_sso_aw_status_s cn78xxp1;
	struct cvmx_sso_aw_status_s cnf75xx;
};

typedef union cvmx_sso_aw_status cvmx_sso_aw_status_t;

/**
 * cvmx_sso_aw_tag_latency_pc
 */
union cvmx_sso_aw_tag_latency_pc {
	u64 u64;
	struct cvmx_sso_aw_tag_latency_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_sso_aw_tag_latency_pc_s cn73xx;
	struct cvmx_sso_aw_tag_latency_pc_s cn78xx;
	struct cvmx_sso_aw_tag_latency_pc_s cn78xxp1;
	struct cvmx_sso_aw_tag_latency_pc_s cnf75xx;
};

typedef union cvmx_sso_aw_tag_latency_pc cvmx_sso_aw_tag_latency_pc_t;

/**
 * cvmx_sso_aw_tag_req_pc
 */
union cvmx_sso_aw_tag_req_pc {
	u64 u64;
	struct cvmx_sso_aw_tag_req_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_sso_aw_tag_req_pc_s cn73xx;
	struct cvmx_sso_aw_tag_req_pc_s cn78xx;
	struct cvmx_sso_aw_tag_req_pc_s cn78xxp1;
	struct cvmx_sso_aw_tag_req_pc_s cnf75xx;
};

typedef union cvmx_sso_aw_tag_req_pc cvmx_sso_aw_tag_req_pc_t;

/**
 * cvmx_sso_aw_we
 */
union cvmx_sso_aw_we {
	u64 u64;
	struct cvmx_sso_aw_we_s {
		u64 reserved_29_63 : 35;
		u64 rsvd_free : 13;
		u64 reserved_13_15 : 3;
		u64 free_cnt : 13;
	} s;
	struct cvmx_sso_aw_we_s cn73xx;
	struct cvmx_sso_aw_we_s cn78xx;
	struct cvmx_sso_aw_we_s cn78xxp1;
	struct cvmx_sso_aw_we_s cnf75xx;
};

typedef union cvmx_sso_aw_we cvmx_sso_aw_we_t;

/**
 * cvmx_sso_bist_stat
 *
 * SSO_BIST_STAT = SSO BIST Status Register
 *
 * Contains the BIST status for the SSO memories ('0' = pass, '1' = fail).
 * Note that PP BIST status is not reported here as it was in previous designs.
 *
 *   There may be more for DDR interface buffers.
 *   It's possible that a RAM will be used for SSO_PP_QOS_RND.
 */
union cvmx_sso_bist_stat {
	u64 u64;
	struct cvmx_sso_bist_stat_s {
		u64 reserved_62_63 : 2;
		u64 odu_pref : 2;
		u64 reserved_54_59 : 6;
		u64 fptr : 2;
		u64 reserved_45_51 : 7;
		u64 rwo_dat : 1;
		u64 rwo : 2;
		u64 reserved_35_41 : 7;
		u64 rwi_dat : 1;
		u64 reserved_32_33 : 2;
		u64 soc : 1;
		u64 reserved_28_30 : 3;
		u64 ncbo : 4;
		u64 reserved_21_23 : 3;
		u64 index : 1;
		u64 reserved_17_19 : 3;
		u64 fidx : 1;
		u64 reserved_10_15 : 6;
		u64 pend : 2;
		u64 reserved_2_7 : 6;
		u64 oth : 2;
	} s;
	struct cvmx_sso_bist_stat_s cn68xx;
	struct cvmx_sso_bist_stat_cn68xxp1 {
		u64 reserved_54_63 : 10;
		u64 fptr : 2;
		u64 reserved_45_51 : 7;
		u64 rwo_dat : 1;
		u64 rwo : 2;
		u64 reserved_35_41 : 7;
		u64 rwi_dat : 1;
		u64 reserved_32_33 : 2;
		u64 soc : 1;
		u64 reserved_28_30 : 3;
		u64 ncbo : 4;
		u64 reserved_21_23 : 3;
		u64 index : 1;
		u64 reserved_17_19 : 3;
		u64 fidx : 1;
		u64 reserved_10_15 : 6;
		u64 pend : 2;
		u64 reserved_2_7 : 6;
		u64 oth : 2;
	} cn68xxp1;
};

typedef union cvmx_sso_bist_stat cvmx_sso_bist_stat_t;

/**
 * cvmx_sso_bist_status0
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status0 {
	u64 u64;
	struct cvmx_sso_bist_status0_s {
		u64 reserved_10_63 : 54;
		u64 bist : 10;
	} s;
	struct cvmx_sso_bist_status0_s cn73xx;
	struct cvmx_sso_bist_status0_s cn78xx;
	struct cvmx_sso_bist_status0_s cn78xxp1;
	struct cvmx_sso_bist_status0_s cnf75xx;
};

typedef union cvmx_sso_bist_status0 cvmx_sso_bist_status0_t;

/**
 * cvmx_sso_bist_status1
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status1 {
	u64 u64;
	struct cvmx_sso_bist_status1_s {
		u64 reserved_7_63 : 57;
		u64 bist : 7;
	} s;
	struct cvmx_sso_bist_status1_s cn73xx;
	struct cvmx_sso_bist_status1_s cn78xx;
	struct cvmx_sso_bist_status1_s cn78xxp1;
	struct cvmx_sso_bist_status1_s cnf75xx;
};

typedef union cvmx_sso_bist_status1 cvmx_sso_bist_status1_t;

/**
 * cvmx_sso_bist_status2
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status2 {
	u64 u64;
	struct cvmx_sso_bist_status2_s {
		u64 reserved_9_63 : 55;
		u64 bist : 9;
	} s;
	struct cvmx_sso_bist_status2_s cn73xx;
	struct cvmx_sso_bist_status2_s cn78xx;
	struct cvmx_sso_bist_status2_s cn78xxp1;
	struct cvmx_sso_bist_status2_s cnf75xx;
};

typedef union cvmx_sso_bist_status2 cvmx_sso_bist_status2_t;

/**
 * cvmx_sso_cfg
 *
 * SSO_CFG = SSO Config
 *
 * This register is an assortment of various SSO configuration bits.
 */
union cvmx_sso_cfg {
	u64 u64;
	struct cvmx_sso_cfg_s {
		u64 reserved_16_63 : 48;
		u64 qck_gw_rsp_adj : 3;
		u64 qck_gw_rsp_dis : 1;
		u64 qck_sw_dis : 1;
		u64 rwq_alloc_dis : 1;
		u64 soc_ccam_dis : 1;
		u64 sso_cclk_dis : 1;
		u64 rwo_flush : 1;
		u64 wfe_thr : 1;
		u64 rwio_byp_dis : 1;
		u64 rwq_byp_dis : 1;
		u64 stt : 1;
		u64 ldt : 1;
		u64 dwb : 1;
		u64 rwen : 1;
	} s;
	struct cvmx_sso_cfg_s cn68xx;
	struct cvmx_sso_cfg_cn68xxp1 {
		u64 reserved_8_63 : 56;
		u64 rwo_flush : 1;
		u64 wfe_thr : 1;
		u64 rwio_byp_dis : 1;
		u64 rwq_byp_dis : 1;
		u64 stt : 1;
		u64 ldt : 1;
		u64 dwb : 1;
		u64 rwen : 1;
	} cn68xxp1;
};

typedef union cvmx_sso_cfg cvmx_sso_cfg_t;

/**
 * cvmx_sso_ds_pc
 *
 * SSO_DS_PC = SSO De-Schedule Performance Counter
 *
 * Counts the number of de-schedule requests.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ds_pc {
	u64 u64;
	struct cvmx_sso_ds_pc_s {
		u64 ds_pc : 64;
	} s;
	struct cvmx_sso_ds_pc_s cn68xx;
	struct cvmx_sso_ds_pc_s cn68xxp1;
};

typedef union cvmx_sso_ds_pc cvmx_sso_ds_pc_t;

/**
 * cvmx_sso_ecc_ctl0
 */
union cvmx_sso_ecc_ctl0 {
	u64 u64;
	struct cvmx_sso_ecc_ctl0_s {
		u64 reserved_30_63 : 34;
		u64 toaqt_flip : 2;
		u64 toaqt_cdis : 1;
		u64 toaqh_flip : 2;
		u64 toaqh_cdis : 1;
		u64 tiaqt_flip : 2;
		u64 tiaqt_cdis : 1;
		u64 tiaqh_flip : 2;
		u64 tiaqh_cdis : 1;
		u64 llm_flip : 2;
		u64 llm_cdis : 1;
		u64 inp_flip : 2;
		u64 inp_cdis : 1;
		u64 qtc_flip : 2;
		u64 qtc_cdis : 1;
		u64 xaq_flip : 2;
		u64 xaq_cdis : 1;
		u64 fff_flip : 2;
		u64 fff_cdis : 1;
		u64 wes_flip : 2;
		u64 wes_cdis : 1;
	} s;
	struct cvmx_sso_ecc_ctl0_s cn73xx;
	struct cvmx_sso_ecc_ctl0_s cn78xx;
	struct cvmx_sso_ecc_ctl0_s cn78xxp1;
	struct cvmx_sso_ecc_ctl0_s cnf75xx;
};

typedef union cvmx_sso_ecc_ctl0 cvmx_sso_ecc_ctl0_t;

/**
 * cvmx_sso_ecc_ctl1
 */
union cvmx_sso_ecc_ctl1 {
	u64 u64;
	struct cvmx_sso_ecc_ctl1_s {
		u64 reserved_21_63 : 43;
		u64 thrint_flip : 2;
		u64 thrint_cdis : 1;
		u64 mask_flip : 2;
		u64 mask_cdis : 1;
		u64 gdw_flip : 2;
		u64 gdw_cdis : 1;
		u64 qidx_flip : 2;
		u64 qidx_cdis : 1;
		u64 tptr_flip : 2;
		u64 tptr_cdis : 1;
		u64 hptr_flip : 2;
		u64 hptr_cdis : 1;
		u64 cntr_flip : 2;
		u64 cntr_cdis : 1;
	} s;
	struct cvmx_sso_ecc_ctl1_s cn73xx;
	struct cvmx_sso_ecc_ctl1_s cn78xx;
	struct cvmx_sso_ecc_ctl1_s cn78xxp1;
	struct cvmx_sso_ecc_ctl1_s cnf75xx;
};

typedef union cvmx_sso_ecc_ctl1 cvmx_sso_ecc_ctl1_t;

/**
 * cvmx_sso_ecc_ctl2
 */
union cvmx_sso_ecc_ctl2 {
	u64 u64;
	struct cvmx_sso_ecc_ctl2_s {
		u64 reserved_15_63 : 49;
		u64 ncbo_flip : 2;
		u64 ncbo_cdis : 1;
		u64 pnd_flip : 2;
		u64 pnd_cdis : 1;
		u64 oth_flip : 2;
		u64 oth_cdis : 1;
		u64 nidx_flip : 2;
		u64 nidx_cdis : 1;
		u64 pidx_flip : 2;
		u64 pidx_cdis : 1;
	} s;
	struct cvmx_sso_ecc_ctl2_s cn73xx;
	struct cvmx_sso_ecc_ctl2_s cn78xx;
	struct cvmx_sso_ecc_ctl2_s cn78xxp1;
	struct cvmx_sso_ecc_ctl2_s cnf75xx;
};

typedef union cvmx_sso_ecc_ctl2 cvmx_sso_ecc_ctl2_t;

/**
 * cvmx_sso_err
 *
 * SSO_ERR = SSO Error Register
 *
 * Contains ECC and other misc error bits.
 *
 * <45> The free page error bit will assert when SSO_FPAGE_CNT <= 16 and
 *      SSO_CFG[RWEN] is 1.  Software will want to disable the interrupt
 *      associated with this error when recovering SSO pointers from the
 *      FPA and SSO.
 *
 * This register also contains the illegal operation error bits:
 *
 * <42> Received ADDWQ with tag specified as EMPTY
 * <41> Received illegal opcode
 * <40> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE
 *      from WS with CLR_NSCHED pending
 * <39> Received CLR_NSCHED
 *      from WS with SWTAG_DESCH/DESCH/CLR_NSCHED pending
 * <38> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE
 *      from WS with ALLOC_WE pending
 * <37> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE/CLR_NSCHED
 *      from WS with GET_WORK pending
 * <36> Received SWTAG_FULL/SWTAG_DESCH
 *      with tag specified as UNSCHEDULED
 * <35> Received SWTAG/SWTAG_FULL/SWTAG_DESCH
 *      with tag specified as EMPTY
 * <34> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/GET_WORK
 *      from WS with pending tag switch to ORDERED or ATOMIC
 * <33> Received SWTAG/SWTAG_DESCH/DESCH/UPD_WQP
 *      from WS in UNSCHEDULED state
 * <32> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP
 *      from WS in EMPTY state
 */
union cvmx_sso_err {
	u64 u64;
	struct cvmx_sso_err_s {
		u64 reserved_48_63 : 16;
		u64 bfp : 1;
		u64 awe : 1;
		u64 fpe : 1;
		u64 reserved_43_44 : 2;
		u64 iop : 11;
		u64 reserved_12_31 : 20;
		u64 pnd_dbe0 : 1;
		u64 pnd_sbe0 : 1;
		u64 pnd_dbe1 : 1;
		u64 pnd_sbe1 : 1;
		u64 oth_dbe0 : 1;
		u64 oth_sbe0 : 1;
		u64 oth_dbe1 : 1;
		u64 oth_sbe1 : 1;
		u64 idx_dbe : 1;
		u64 idx_sbe : 1;
		u64 fidx_dbe : 1;
		u64 fidx_sbe : 1;
	} s;
	struct cvmx_sso_err_s cn68xx;
	struct cvmx_sso_err_s cn68xxp1;
};

typedef union cvmx_sso_err cvmx_sso_err_t;

/**
 * cvmx_sso_err0
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err0 {
	u64 u64;
	struct cvmx_sso_err0_s {
		u64 reserved_52_63 : 12;
		u64 toaqt_dbe : 1;
		u64 toaqt_sbe : 1;
		u64 toaqh_dbe : 1;
		u64 toaqh_sbe : 1;
		u64 tiaqt_dbe : 1;
		u64 tiaqt_sbe : 1;
		u64 tiaqh_dbe : 1;
		u64 tiaqh_sbe : 1;
		u64 llm_dbe : 1;
		u64 llm_sbe : 1;
		u64 inp_dbe : 1;
		u64 inp_sbe : 1;
		u64 qtc_dbe : 1;
		u64 qtc_sbe : 1;
		u64 xaq_dbe : 1;
		u64 xaq_sbe : 1;
		u64 fff_dbe : 1;
		u64 fff_sbe : 1;
		u64 wes_dbe : 1;
		u64 wes_sbe : 1;
		u64 reserved_6_31 : 26;
		u64 addwq_dropped : 1;
		u64 awempty : 1;
		u64 grpdis : 1;
		u64 bfp : 1;
		u64 awe : 1;
		u64 fpe : 1;
	} s;
	struct cvmx_sso_err0_s cn73xx;
	struct cvmx_sso_err0_s cn78xx;
	struct cvmx_sso_err0_s cn78xxp1;
	struct cvmx_sso_err0_s cnf75xx;
};

typedef union cvmx_sso_err0 cvmx_sso_err0_t;

/**
 * cvmx_sso_err1
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err1 {
	u64 u64;
	struct cvmx_sso_err1_s {
		u64 reserved_14_63 : 50;
		u64 thrint_dbe : 1;
		u64 thrint_sbe : 1;
		u64 mask_dbe : 1;
		u64 mask_sbe : 1;
		u64 gdw_dbe : 1;
		u64 gdw_sbe : 1;
		u64 qidx_dbe : 1;
		u64 qidx_sbe : 1;
		u64 tptr_dbe : 1;
		u64 tptr_sbe : 1;
		u64 hptr_dbe : 1;
		u64 hptr_sbe : 1;
		u64 cntr_dbe : 1;
		u64 cntr_sbe : 1;
	} s;
	struct cvmx_sso_err1_s cn73xx;
	struct cvmx_sso_err1_s cn78xx;
	struct cvmx_sso_err1_s cn78xxp1;
	struct cvmx_sso_err1_s cnf75xx;
};

typedef union cvmx_sso_err1 cvmx_sso_err1_t;

/**
 * cvmx_sso_err2
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err2 {
	u64 u64;
	struct cvmx_sso_err2_s {
		u64 reserved_42_63 : 22;
		u64 ncbo_dbe : 1;
		u64 ncbo_sbe : 1;
		u64 pnd_dbe : 1;
		u64 pnd_sbe : 1;
		u64 oth_dbe : 1;
		u64 oth_sbe : 1;
		u64 nidx_dbe : 1;
		u64 nidx_sbe : 1;
		u64 pidx_dbe : 1;
		u64 pidx_sbe : 1;
		u64 reserved_13_31 : 19;
		u64 iop : 13;
	} s;
	struct cvmx_sso_err2_s cn73xx;
	struct cvmx_sso_err2_s cn78xx;
	struct cvmx_sso_err2_s cn78xxp1;
	struct cvmx_sso_err2_s cnf75xx;
};

typedef union cvmx_sso_err2 cvmx_sso_err2_t;

/**
 * cvmx_sso_err_enb
 *
 * SSO_ERR_ENB = SSO Error Enable Register
 *
 * Contains the interrupt enables corresponding to SSO_ERR.
 */
union cvmx_sso_err_enb {
	u64 u64;
	struct cvmx_sso_err_enb_s {
		u64 reserved_48_63 : 16;
		u64 bfp_ie : 1;
		u64 awe_ie : 1;
		u64 fpe_ie : 1;
		u64 reserved_43_44 : 2;
		u64 iop_ie : 11;
		u64 reserved_12_31 : 20;
		u64 pnd_dbe0_ie : 1;
		u64 pnd_sbe0_ie : 1;
		u64 pnd_dbe1_ie : 1;
		u64 pnd_sbe1_ie : 1;
		u64 oth_dbe0_ie : 1;
		u64 oth_sbe0_ie : 1;
		u64 oth_dbe1_ie : 1;
		u64 oth_sbe1_ie : 1;
		u64 idx_dbe_ie : 1;
		u64 idx_sbe_ie : 1;
		u64 fidx_dbe_ie : 1;
		u64 fidx_sbe_ie : 1;
	} s;
	struct cvmx_sso_err_enb_s cn68xx;
	struct cvmx_sso_err_enb_s cn68xxp1;
};

typedef union cvmx_sso_err_enb cvmx_sso_err_enb_t;

/**
 * cvmx_sso_fidx_ecc_ctl
 *
 * SSO_FIDX_ECC_CTL = SSO FIDX ECC Control
 *
 */
union cvmx_sso_fidx_ecc_ctl {
	u64 u64;
	struct cvmx_sso_fidx_ecc_ctl_s {
		u64 reserved_3_63 : 61;
		u64 flip_synd : 2;
		u64 ecc_ena : 1;
	} s;
	struct cvmx_sso_fidx_ecc_ctl_s cn68xx;
	struct cvmx_sso_fidx_ecc_ctl_s cn68xxp1;
};

typedef union cvmx_sso_fidx_ecc_ctl cvmx_sso_fidx_ecc_ctl_t;

/**
 * cvmx_sso_fidx_ecc_st
 *
 * SSO_FIDX_ECC_ST = SSO FIDX ECC Status
 *
 */
union cvmx_sso_fidx_ecc_st {
	u64 u64;
	struct cvmx_sso_fidx_ecc_st_s {
		u64 reserved_27_63 : 37;
		u64 addr : 11;
		u64 reserved_9_15 : 7;
		u64 syndrom : 5;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_sso_fidx_ecc_st_s cn68xx;
	struct cvmx_sso_fidx_ecc_st_s cn68xxp1;
};

typedef union cvmx_sso_fidx_ecc_st cvmx_sso_fidx_ecc_st_t;

/**
 * cvmx_sso_fpage_cnt
 *
 * SSO_FPAGE_CNT = SSO Free Page Cnt
 *
 * This register keeps track of the number of free pages pointers available for use in external memory.
 */
union cvmx_sso_fpage_cnt {
	u64 u64;
	struct cvmx_sso_fpage_cnt_s {
		u64 reserved_32_63 : 32;
		u64 fpage_cnt : 32;
	} s;
	struct cvmx_sso_fpage_cnt_s cn68xx;
	struct cvmx_sso_fpage_cnt_s cn68xxp1;
};

typedef union cvmx_sso_fpage_cnt cvmx_sso_fpage_cnt_t;

/**
 * cvmx_sso_grp#_aq_cnt
 */
union cvmx_sso_grpx_aq_cnt {
	u64 u64;
	struct cvmx_sso_grpx_aq_cnt_s {
		u64 reserved_33_63 : 31;
		u64 aq_cnt : 33;
	} s;
	struct cvmx_sso_grpx_aq_cnt_s cn73xx;
	struct cvmx_sso_grpx_aq_cnt_s cn78xx;
	struct cvmx_sso_grpx_aq_cnt_s cn78xxp1;
	struct cvmx_sso_grpx_aq_cnt_s cnf75xx;
};

typedef union cvmx_sso_grpx_aq_cnt cvmx_sso_grpx_aq_cnt_t;

/**
 * cvmx_sso_grp#_aq_thr
 */
union cvmx_sso_grpx_aq_thr {
	u64 u64;
	struct cvmx_sso_grpx_aq_thr_s {
		u64 reserved_33_63 : 31;
		u64 aq_thr : 33;
	} s;
	struct cvmx_sso_grpx_aq_thr_s cn73xx;
	struct cvmx_sso_grpx_aq_thr_s cn78xx;
	struct cvmx_sso_grpx_aq_thr_s cn78xxp1;
	struct cvmx_sso_grpx_aq_thr_s cnf75xx;
};

typedef union cvmx_sso_grpx_aq_thr cvmx_sso_grpx_aq_thr_t;

/**
 * cvmx_sso_grp#_ds_pc
 *
 * Counts the number of deschedule requests for each group. Counter rolls over through zero when
 * max value exceeded.
 */
union cvmx_sso_grpx_ds_pc {
	u64 u64;
	struct cvmx_sso_grpx_ds_pc_s {
		u64 cnt : 64;
	} s;
	struct cvmx_sso_grpx_ds_pc_s cn73xx;
	struct cvmx_sso_grpx_ds_pc_s cn78xx;
	struct cvmx_sso_grpx_ds_pc_s cn78xxp1;
	struct cvmx_sso_grpx_ds_pc_s cnf75xx;
};

typedef union cvmx_sso_grpx_ds_pc cvmx_sso_grpx_ds_pc_t;

/**
 * cvmx_sso_grp#_ext_pc
 *
 * Counts the number of cache lines of WAEs sent to L2/DDR. Counter rolls over through zero when
 * max value exceeded.
 */
union cvmx_sso_grpx_ext_pc {
	u64 u64;
	struct cvmx_sso_grpx_ext_pc_s {
		u64 cnt : 64;
	} s;
	struct cvmx_sso_grpx_ext_pc_s cn73xx;
	struct cvmx_sso_grpx_ext_pc_s cn78xx;
	struct cvmx_sso_grpx_ext_pc_s cn78xxp1;
	struct cvmx_sso_grpx_ext_pc_s cnf75xx;
};

typedef union cvmx_sso_grpx_ext_pc cvmx_sso_grpx_ext_pc_t;

/**
 * cvmx_sso_grp#_iaq_thr
 *
 * These registers contain the thresholds for allocating SSO in-unit admission queue entries, see
 * In-Unit Thresholds.
 */
union cvmx_sso_grpx_iaq_thr {
	u64 u64;
	struct cvmx_sso_grpx_iaq_thr_s {
		u64 reserved_61_63 : 3;
		u64 grp_cnt : 13;
		u64 reserved_45_47 : 3;
		u64 max_thr : 13;
		u64 reserved_13_31 : 19;
		u64 rsvd_thr : 13;
	} s;
	struct cvmx_sso_grpx_iaq_thr_s cn73xx;
	struct cvmx_sso_grpx_iaq_thr_s cn78xx;
	struct cvmx_sso_grpx_iaq_thr_s cn78xxp1;
	struct cvmx_sso_grpx_iaq_thr_s cnf75xx;
};

typedef union cvmx_sso_grpx_iaq_thr cvmx_sso_grpx_iaq_thr_t;

/**
 * cvmx_sso_grp#_int
 *
 * Contains the per-group interrupts and are used to clear these interrupts. For more information
 * on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int {
	u64 u64;
	struct cvmx_sso_grpx_int_s {
		u64 exe_dis : 1;
		u64 reserved_2_62 : 61;
		u64 exe_int : 1;
		u64 aq_int : 1;
	} s;
	struct cvmx_sso_grpx_int_s cn73xx;
	struct cvmx_sso_grpx_int_s cn78xx;
	struct cvmx_sso_grpx_int_s cn78xxp1;
	struct cvmx_sso_grpx_int_s cnf75xx;
};

typedef union cvmx_sso_grpx_int cvmx_sso_grpx_int_t;

/**
 * cvmx_sso_grp#_int_cnt
 *
 * These registers contain a read-only copy of the counts used to trigger work-queue interrupts
 * (one per group). For more information on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int_cnt {
	u64 u64;
	struct cvmx_sso_grpx_int_cnt_s {
		u64 reserved_61_63 : 3;
		u64 tc_cnt : 13;
		u64 reserved_45_47 : 3;
		u64 cq_cnt : 13;
		u64 reserved_29_31 : 3;
		u64 ds_cnt : 13;
		u64 reserved_13_15 : 3;
		u64 iaq_cnt : 13;
	} s;
	struct cvmx_sso_grpx_int_cnt_s cn73xx;
	struct cvmx_sso_grpx_int_cnt_s cn78xx;
	struct cvmx_sso_grpx_int_cnt_s cn78xxp1;
	struct cvmx_sso_grpx_int_cnt_s cnf75xx;
};

typedef union cvmx_sso_grpx_int_cnt cvmx_sso_grpx_int_cnt_t;

/**
 * cvmx_sso_grp#_int_thr
 *
 * These registers contain the thresholds for enabling and setting work-queue interrupts (one per
 * group). For more information on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int_thr {
	u64 u64;
	struct cvmx_sso_grpx_int_thr_s {
		u64 tc_en : 1;
		u64 reserved_61_62 : 2;
		u64 tc_thr : 13;
		u64 reserved_45_47 : 3;
		u64 cq_thr : 13;
		u64 reserved_29_31 : 3;
		u64 ds_thr : 13;
		u64 reserved_13_15 : 3;
		u64 iaq_thr : 13;
	} s;
	struct cvmx_sso_grpx_int_thr_s cn73xx;
	struct cvmx_sso_grpx_int_thr_s cn78xx;
	struct cvmx_sso_grpx_int_thr_s cn78xxp1;
	struct cvmx_sso_grpx_int_thr_s cnf75xx;
};

typedef union cvmx_sso_grpx_int_thr cvmx_sso_grpx_int_thr_t;

/**
 * cvmx_sso_grp#_pri
 *
 * Controls the priority and group affinity arbitration for each group.
 *
 */
union cvmx_sso_grpx_pri {
	u64 u64;
	struct cvmx_sso_grpx_pri_s {
		u64 reserved_30_63 : 34;
		u64 wgt_left : 6;
		u64 reserved_22_23 : 2;
		u64 weight : 6;
		u64 reserved_12_15 : 4;
		u64 affinity : 4;
		u64 reserved_3_7 : 5;
		u64 pri : 3;
	} s;
	struct cvmx_sso_grpx_pri_s cn73xx;
	struct cvmx_sso_grpx_pri_s cn78xx;
	struct cvmx_sso_grpx_pri_s cn78xxp1;
	struct cvmx_sso_grpx_pri_s cnf75xx;
};

typedef union cvmx_sso_grpx_pri cvmx_sso_grpx_pri_t;

/**
 * cvmx_sso_grp#_taq_thr
 *
 * These registers contain the thresholds for allocating SSO transitory admission queue storage
 * buffers, see Transitory-Admission Thresholds.
 */
union cvmx_sso_grpx_taq_thr {
	u64 u64;
	struct cvmx_sso_grpx_taq_thr_s {
		u64 reserved_59_63 : 5;
		u64 grp_cnt : 11;
		u64 reserved_43_47 : 5;
		u64 max_thr : 11;
		u64 reserved_11_31 : 21;
		u64 rsvd_thr : 11;
	} s;
	struct cvmx_sso_grpx_taq_thr_s cn73xx;
	struct cvmx_sso_grpx_taq_thr_s cn78xx;
	struct cvmx_sso_grpx_taq_thr_s cn78xxp1;
	struct cvmx_sso_grpx_taq_thr_s cnf75xx;
};

typedef union cvmx_sso_grpx_taq_thr cvmx_sso_grpx_taq_thr_t;

/**
 * cvmx_sso_grp#_ts_pc
 *
 * Counts the number of tag switch requests for each group being switched to. Counter rolls over
 * through zero when max value exceeded.
 */
union cvmx_sso_grpx_ts_pc {
	u64 u64;
	struct cvmx_sso_grpx_ts_pc_s {
		u64 cnt : 64;
	} s;
	struct cvmx_sso_grpx_ts_pc_s cn73xx;
	struct cvmx_sso_grpx_ts_pc_s cn78xx;
	struct cvmx_sso_grpx_ts_pc_s cn78xxp1;
	struct cvmx_sso_grpx_ts_pc_s cnf75xx;
};

typedef union cvmx_sso_grpx_ts_pc cvmx_sso_grpx_ts_pc_t;

/**
 * cvmx_sso_grp#_wa_pc
 *
 * Counts the number of add new work requests for each group. The counter rolls over through zero
 * when the max value exceeded.
 */
union cvmx_sso_grpx_wa_pc {
	u64 u64;
	struct cvmx_sso_grpx_wa_pc_s {
		u64 cnt : 64;
	} s;
	struct cvmx_sso_grpx_wa_pc_s cn73xx;
	struct cvmx_sso_grpx_wa_pc_s cn78xx;
	struct cvmx_sso_grpx_wa_pc_s cn78xxp1;
	struct cvmx_sso_grpx_wa_pc_s cnf75xx;
};

typedef union cvmx_sso_grpx_wa_pc cvmx_sso_grpx_wa_pc_t;

/**
 * cvmx_sso_grp#_ws_pc
 *
 * Counts the number of work schedules for each group. The counter rolls over through zero when
 * the maximum value is exceeded.
 */
union cvmx_sso_grpx_ws_pc {
	u64 u64;
	struct cvmx_sso_grpx_ws_pc_s {
		u64 cnt : 64;
	} s;
	struct cvmx_sso_grpx_ws_pc_s cn73xx;
	struct cvmx_sso_grpx_ws_pc_s cn78xx;
	struct cvmx_sso_grpx_ws_pc_s cn78xxp1;
	struct cvmx_sso_grpx_ws_pc_s cnf75xx;
};

typedef union cvmx_sso_grpx_ws_pc cvmx_sso_grpx_ws_pc_t;

/**
 * cvmx_sso_gw_eco
 */
union cvmx_sso_gw_eco {
	u64 u64;
	struct cvmx_sso_gw_eco_s {
		u64 reserved_8_63 : 56;
		u64 eco_rw : 8;
	} s;
	struct cvmx_sso_gw_eco_s cn73xx;
	struct cvmx_sso_gw_eco_s cnf75xx;
};

typedef union cvmx_sso_gw_eco cvmx_sso_gw_eco_t;

/**
 * cvmx_sso_gwe_cfg
 *
 * This register controls the operation of the get-work examiner (GWE).
 *
 */
union cvmx_sso_gwe_cfg {
	u64 u64;
	struct cvmx_sso_gwe_cfg_s {
		u64 reserved_12_63 : 52;
		u64 odu_ffpgw_dis : 1;
		u64 gwe_rfpgw_dis : 1;
		u64 odu_prf_dis : 1;
		u64 reserved_0_8 : 9;
	} s;
	struct cvmx_sso_gwe_cfg_cn68xx {
		u64 reserved_12_63 : 52;
		u64 odu_ffpgw_dis : 1;
		u64 gwe_rfpgw_dis : 1;
		u64 odu_prf_dis : 1;
		u64 odu_bmp_dis : 1;
		u64 reserved_5_7 : 3;
		u64 gwe_hvy_dis : 1;
		u64 gwe_poe : 1;
		u64 gwe_fpor : 1;
		u64 gwe_rah : 1;
		u64 gwe_dis : 1;
	} cn68xx;
	struct cvmx_sso_gwe_cfg_cn68xxp1 {
		u64 reserved_4_63 : 60;
		u64 gwe_poe : 1;
		u64 gwe_fpor : 1;
		u64 gwe_rah : 1;
		u64 gwe_dis : 1;
	} cn68xxp1;
	struct cvmx_sso_gwe_cfg_cn73xx {
		u64 reserved_9_63 : 55;
		u64 dis_wgt_credit : 1;
		u64 ws_retries : 8;
	} cn73xx;
	struct cvmx_sso_gwe_cfg_cn73xx cn78xx;
	struct cvmx_sso_gwe_cfg_cn73xx cn78xxp1;
	struct cvmx_sso_gwe_cfg_cn73xx cnf75xx;
};

typedef union cvmx_sso_gwe_cfg cvmx_sso_gwe_cfg_t;

/**
 * cvmx_sso_gwe_random
 *
 * This register contains the random search start position for the get-work examiner (GWE).
 *
 */
union cvmx_sso_gwe_random {
	u64 u64;
	struct cvmx_sso_gwe_random_s {
		u64 reserved_16_63 : 48;
		u64 rnd : 16;
	} s;
	struct cvmx_sso_gwe_random_s cn73xx;
	struct cvmx_sso_gwe_random_s cn78xx;
	struct cvmx_sso_gwe_random_s cn78xxp1;
	struct cvmx_sso_gwe_random_s cnf75xx;
};

typedef union cvmx_sso_gwe_random cvmx_sso_gwe_random_t;

/**
 * cvmx_sso_idx_ecc_ctl
 *
 * SSO_IDX_ECC_CTL = SSO IDX ECC Control
 *
 */
union cvmx_sso_idx_ecc_ctl {
	u64 u64;
	struct cvmx_sso_idx_ecc_ctl_s {
		u64 reserved_3_63 : 61;
		u64 flip_synd : 2;
		u64 ecc_ena : 1;
	} s;
	struct cvmx_sso_idx_ecc_ctl_s cn68xx;
	struct cvmx_sso_idx_ecc_ctl_s cn68xxp1;
};

typedef union cvmx_sso_idx_ecc_ctl cvmx_sso_idx_ecc_ctl_t;

/**
 * cvmx_sso_idx_ecc_st
 *
 * SSO_IDX_ECC_ST = SSO IDX ECC Status
 *
 */
union cvmx_sso_idx_ecc_st {
	u64 u64;
	struct cvmx_sso_idx_ecc_st_s {
		u64 reserved_27_63 : 37;
		u64 addr : 11;
		u64 reserved_9_15 : 7;
		u64 syndrom : 5;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_sso_idx_ecc_st_s cn68xx;
	struct cvmx_sso_idx_ecc_st_s cn68xxp1;
};

typedef union cvmx_sso_idx_ecc_st cvmx_sso_idx_ecc_st_t;

/**
 * cvmx_sso_ient#_links
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_links {
	u64 u64;
	struct cvmx_sso_ientx_links_s {
		u64 reserved_28_63 : 36;
		u64 prev_index : 12;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_sso_ientx_links_cn73xx {
		u64 reserved_26_63 : 38;
		u64 prev_index : 10;
		u64 reserved_11_15 : 5;
		u64 next_index_vld : 1;
		u64 next_index : 10;
	} cn73xx;
	struct cvmx_sso_ientx_links_cn78xx {
		u64 reserved_28_63 : 36;
		u64 prev_index : 12;
		u64 reserved_13_15 : 3;
		u64 next_index_vld : 1;
		u64 next_index : 12;
	} cn78xx;
	struct cvmx_sso_ientx_links_cn78xx cn78xxp1;
	struct cvmx_sso_ientx_links_cn73xx cnf75xx;
};

typedef union cvmx_sso_ientx_links cvmx_sso_ientx_links_t;

/**
 * cvmx_sso_ient#_pendtag
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_pendtag {
	u64 u64;
	struct cvmx_sso_ientx_pendtag_s {
		u64 reserved_38_63 : 26;
		u64 pend_switch : 1;
		u64 reserved_34_36 : 3;
		u64 pend_tt : 2;
		u64 pend_tag : 32;
	} s;
	struct cvmx_sso_ientx_pendtag_s cn73xx;
	struct cvmx_sso_ientx_pendtag_s cn78xx;
	struct cvmx_sso_ientx_pendtag_s cn78xxp1;
	struct cvmx_sso_ientx_pendtag_s cnf75xx;
};

typedef union cvmx_sso_ientx_pendtag cvmx_sso_ientx_pendtag_t;

/**
 * cvmx_sso_ient#_qlinks
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_qlinks {
	u64 u64;
	struct cvmx_sso_ientx_qlinks_s {
		u64 reserved_12_63 : 52;
		u64 next_index : 12;
	} s;
	struct cvmx_sso_ientx_qlinks_s cn73xx;
	struct cvmx_sso_ientx_qlinks_s cn78xx;
	struct cvmx_sso_ientx_qlinks_s cn78xxp1;
	struct cvmx_sso_ientx_qlinks_s cnf75xx;
};

typedef union cvmx_sso_ientx_qlinks cvmx_sso_ientx_qlinks_t;

/**
 * cvmx_sso_ient#_tag
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_tag {
	u64 u64;
	struct cvmx_sso_ientx_tag_s {
		u64 reserved_39_63 : 25;
		u64 tailc : 1;
		u64 tail : 1;
		u64 reserved_34_36 : 3;
		u64 tt : 2;
		u64 tag : 32;
	} s;
	struct cvmx_sso_ientx_tag_s cn73xx;
	struct cvmx_sso_ientx_tag_s cn78xx;
	struct cvmx_sso_ientx_tag_s cn78xxp1;
	struct cvmx_sso_ientx_tag_s cnf75xx;
};

typedef union cvmx_sso_ientx_tag cvmx_sso_ientx_tag_t;

/**
 * cvmx_sso_ient#_wqpgrp
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_wqpgrp {
	u64 u64;
	struct cvmx_sso_ientx_wqpgrp_s {
		u64 reserved_62_63 : 2;
		u64 head : 1;
		u64 nosched : 1;
		u64 reserved_58_59 : 2;
		u64 grp : 10;
		u64 reserved_42_47 : 6;
		u64 wqp : 42;
	} s;
	struct cvmx_sso_ientx_wqpgrp_cn73xx {
		u64 reserved_62_63 : 2;
		u64 head : 1;
		u64 nosched : 1;
		u64 reserved_56_59 : 4;
		u64 grp : 8;
		u64 reserved_42_47 : 6;
		u64 wqp : 42;
	} cn73xx;
	struct cvmx_sso_ientx_wqpgrp_s cn78xx;
	struct cvmx_sso_ientx_wqpgrp_s cn78xxp1;
	struct cvmx_sso_ientx_wqpgrp_cn73xx cnf75xx;
};

typedef union cvmx_sso_ientx_wqpgrp cvmx_sso_ientx_wqpgrp_t;

/**
 * cvmx_sso_ipl_conf#
 *
 * Returns list status for the conflicted list indexed by group.  Register
 * fields are identical to those in SSO_IPL_IAQ() above.
 */
union cvmx_sso_ipl_confx {
	u64 u64;
	struct cvmx_sso_ipl_confx_s {
		u64 reserved_28_63 : 36;
		u64 queue_val : 1;
		u64 queue_one : 1;
		u64 reserved_25_25 : 1;
		u64 queue_head : 12;
		u64 reserved_12_12 : 1;
		u64 queue_tail : 12;
	} s;
	struct cvmx_sso_ipl_confx_s cn73xx;
	struct cvmx_sso_ipl_confx_s cn78xx;
	struct cvmx_sso_ipl_confx_s cn78xxp1;
	struct cvmx_sso_ipl_confx_s cnf75xx;
};

typedef union cvmx_sso_ipl_confx cvmx_sso_ipl_confx_t;

/**
 * cvmx_sso_ipl_desched#
 *
 * Returns list status for the deschedule list indexed by group.  Register
 * fields are identical to those in SSO_IPL_IAQ() above.
 */
union cvmx_sso_ipl_deschedx {
	u64 u64;
	struct cvmx_sso_ipl_deschedx_s {
		u64 reserved_28_63 : 36;
		u64 queue_val : 1;
		u64 queue_one : 1;
		u64 reserved_25_25 : 1;
		u64 queue_head : 12;
		u64 reserved_12_12 : 1;
		u64 queue_tail : 12;
	} s;
	struct cvmx_sso_ipl_deschedx_s cn73xx;
	struct cvmx_sso_ipl_deschedx_s cn78xx;
	struct cvmx_sso_ipl_deschedx_s cn78xxp1;
	struct cvmx_sso_ipl_deschedx_s cnf75xx;
};

typedef union cvmx_sso_ipl_deschedx cvmx_sso_ipl_deschedx_t;

/**
 * cvmx_sso_ipl_free#
 *
 * Returns list status.
 *
 */
union cvmx_sso_ipl_freex {
	u64 u64;
	struct cvmx_sso_ipl_freex_s {
		u64 reserved_62_63 : 2;
		u64 qnum_head : 3;
		u64 qnum_tail : 3;
		u64 reserved_28_55 : 28;
		u64 queue_val : 1;
		u64 reserved_25_26 : 2;
		u64 queue_head : 12;
		u64 reserved_12_12 : 1;
		u64 queue_tail : 12;
	} s;
	struct cvmx_sso_ipl_freex_cn73xx {
		u64 reserved_62_63 : 2;
		u64 qnum_head : 3;
		u64 qnum_tail : 3;
		u64 reserved_28_55 : 28;
		u64 queue_val : 1;
		u64 reserved_23_26 : 4;
		u64 queue_head : 10;
		u64 reserved_10_12 : 3;
		u64 queue_tail : 10;
	} cn73xx;
	struct cvmx_sso_ipl_freex_s cn78xx;
	struct cvmx_sso_ipl_freex_s cn78xxp1;
	struct cvmx_sso_ipl_freex_cn73xx cnf75xx;
};

typedef union cvmx_sso_ipl_freex cvmx_sso_ipl_freex_t;

/**
 * cvmx_sso_ipl_iaq#
 *
 * Returns list status for the internal admission queue indexed by group.
 *
 */
union cvmx_sso_ipl_iaqx {
	u64 u64;
	struct cvmx_sso_ipl_iaqx_s {
		u64 reserved_28_63 : 36;
		u64 queue_val : 1;
		u64 queue_one : 1;
		u64 reserved_25_25 : 1;
		u64 queue_head : 12;
		u64 reserved_12_12 : 1;
		u64 queue_tail : 12;
	} s;
	struct cvmx_sso_ipl_iaqx_s cn73xx;
	struct cvmx_sso_ipl_iaqx_s cn78xx;
	struct cvmx_sso_ipl_iaqx_s cn78xxp1;
	struct cvmx_sso_ipl_iaqx_s cnf75xx;
};

typedef union cvmx_sso_ipl_iaqx cvmx_sso_ipl_iaqx_t;

/**
 * cvmx_sso_iq_cnt#
 *
 * CSR reserved addresses: (64): 0x8200..0x83f8
 * CSR align addresses: ===========================================================================================================
 * SSO_IQ_CNTX = SSO Input Queue Count Register
 *               (one per QOS level)
 *
 * Contains a read-only count of the number of work queue entries for each QOS
 * level. Counts both in-unit and in-memory entries.
 */
union cvmx_sso_iq_cntx {
	u64 u64;
	struct cvmx_sso_iq_cntx_s {
		u64 reserved_32_63 : 32;
		u64 iq_cnt : 32;
	} s;
	struct cvmx_sso_iq_cntx_s cn68xx;
	struct cvmx_sso_iq_cntx_s cn68xxp1;
};

typedef union cvmx_sso_iq_cntx cvmx_sso_iq_cntx_t;

/**
 * cvmx_sso_iq_com_cnt
 *
 * SSO_IQ_COM_CNT = SSO Input Queue Combined Count Register
 *
 * Contains a read-only count of the total number of work queue entries in all
 * QOS levels.  Counts both in-unit and in-memory entries.
 */
union cvmx_sso_iq_com_cnt {
	u64 u64;
	struct cvmx_sso_iq_com_cnt_s {
		u64 reserved_32_63 : 32;
		u64 iq_cnt : 32;
	} s;
	struct cvmx_sso_iq_com_cnt_s cn68xx;
	struct cvmx_sso_iq_com_cnt_s cn68xxp1;
};

typedef union cvmx_sso_iq_com_cnt cvmx_sso_iq_com_cnt_t;

/**
 * cvmx_sso_iq_int
 *
 * SSO_IQ_INT = SSO Input Queue Interrupt Register
 *
 * Contains the bits (one per QOS level) that can trigger the input queue
 * interrupt.  An IQ_INT bit will be set if SSO_IQ_CNT#QOS# changes and the
 * resulting value is equal to SSO_IQ_THR#QOS#.
 */
union cvmx_sso_iq_int {
	u64 u64;
	struct cvmx_sso_iq_int_s {
		u64 reserved_8_63 : 56;
		u64 iq_int : 8;
	} s;
	struct cvmx_sso_iq_int_s cn68xx;
	struct cvmx_sso_iq_int_s cn68xxp1;
};

typedef union cvmx_sso_iq_int cvmx_sso_iq_int_t;

/**
 * cvmx_sso_iq_int_en
 *
 * SSO_IQ_INT_EN = SSO Input Queue Interrupt Enable Register
 *
 * Contains the bits (one per QOS level) that enable the input queue interrupt.
 */
union cvmx_sso_iq_int_en {
	u64 u64;
	struct cvmx_sso_iq_int_en_s {
		u64 reserved_8_63 : 56;
		u64 int_en : 8;
	} s;
	struct cvmx_sso_iq_int_en_s cn68xx;
	struct cvmx_sso_iq_int_en_s cn68xxp1;
};

typedef union cvmx_sso_iq_int_en cvmx_sso_iq_int_en_t;

/**
 * cvmx_sso_iq_thr#
 *
 * CSR reserved addresses: (24): 0x9040..0x90f8
 * CSR align addresses: ===========================================================================================================
 * SSO_IQ_THRX = SSO Input Queue Threshold Register
 *               (one per QOS level)
 *
 * Threshold value for triggering input queue interrupts.
 */
union cvmx_sso_iq_thrx {
	u64 u64;
	struct cvmx_sso_iq_thrx_s {
		u64 reserved_32_63 : 32;
		u64 iq_thr : 32;
	} s;
	struct cvmx_sso_iq_thrx_s cn68xx;
	struct cvmx_sso_iq_thrx_s cn68xxp1;
};

typedef union cvmx_sso_iq_thrx cvmx_sso_iq_thrx_t;

/**
 * cvmx_sso_nos_cnt
 *
 * Contains the number of work-queue entries on the no-schedule list.
 *
 */
union cvmx_sso_nos_cnt {
	u64 u64;
	struct cvmx_sso_nos_cnt_s {
		u64 reserved_13_63 : 51;
		u64 nos_cnt : 13;
	} s;
	struct cvmx_sso_nos_cnt_cn68xx {
		u64 reserved_12_63 : 52;
		u64 nos_cnt : 12;
	} cn68xx;
	struct cvmx_sso_nos_cnt_cn68xx cn68xxp1;
	struct cvmx_sso_nos_cnt_s cn73xx;
	struct cvmx_sso_nos_cnt_s cn78xx;
	struct cvmx_sso_nos_cnt_s cn78xxp1;
	struct cvmx_sso_nos_cnt_s cnf75xx;
};

typedef union cvmx_sso_nos_cnt cvmx_sso_nos_cnt_t;

/**
 * cvmx_sso_nw_tim
 *
 * Sets the minimum period for a new-work-request timeout. The period is specified in n-1
 * notation, with the increment value of 1024 clock cycles. Thus, a value of 0x0 in this register
 * translates to 1024 cycles, 0x1 translates to 2048 cycles, 0x2 translates to 3072 cycles, etc.
 */
union cvmx_sso_nw_tim {
	u64 u64;
	struct cvmx_sso_nw_tim_s {
		u64 reserved_10_63 : 54;
		u64 nw_tim : 10;
	} s;
	struct cvmx_sso_nw_tim_s cn68xx;
	struct cvmx_sso_nw_tim_s cn68xxp1;
	struct cvmx_sso_nw_tim_s cn73xx;
	struct cvmx_sso_nw_tim_s cn78xx;
	struct cvmx_sso_nw_tim_s cn78xxp1;
	struct cvmx_sso_nw_tim_s cnf75xx;
};

typedef union cvmx_sso_nw_tim cvmx_sso_nw_tim_t;

/**
 * cvmx_sso_oth_ecc_ctl
 *
 * SSO_OTH_ECC_CTL = SSO OTH ECC Control
 *
 */
union cvmx_sso_oth_ecc_ctl {
	u64 u64;
	struct cvmx_sso_oth_ecc_ctl_s {
		u64 reserved_6_63 : 58;
		u64 flip_synd1 : 2;
		u64 ecc_ena1 : 1;
		u64 flip_synd0 : 2;
		u64 ecc_ena0 : 1;
	} s;
	struct cvmx_sso_oth_ecc_ctl_s cn68xx;
	struct cvmx_sso_oth_ecc_ctl_s cn68xxp1;
};

typedef union cvmx_sso_oth_ecc_ctl cvmx_sso_oth_ecc_ctl_t;

/**
 * cvmx_sso_oth_ecc_st
 *
 * SSO_OTH_ECC_ST = SSO OTH ECC Status
 *
 */
union cvmx_sso_oth_ecc_st {
	u64 u64;
	struct cvmx_sso_oth_ecc_st_s {
		u64 reserved_59_63 : 5;
		u64 addr1 : 11;
		u64 reserved_43_47 : 5;
		u64 syndrom1 : 7;
		u64 reserved_27_35 : 9;
		u64 addr0 : 11;
		u64 reserved_11_15 : 5;
		u64 syndrom0 : 7;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_sso_oth_ecc_st_s cn68xx;
	struct cvmx_sso_oth_ecc_st_s cn68xxp1;
};

typedef union cvmx_sso_oth_ecc_st cvmx_sso_oth_ecc_st_t;

/**
 * cvmx_sso_page_cnt
 */
union cvmx_sso_page_cnt {
	u64 u64;
	struct cvmx_sso_page_cnt_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_sso_page_cnt_s cn73xx;
	struct cvmx_sso_page_cnt_s cn78xx;
	struct cvmx_sso_page_cnt_s cn78xxp1;
	struct cvmx_sso_page_cnt_s cnf75xx;
};

typedef union cvmx_sso_page_cnt cvmx_sso_page_cnt_t;

/**
 * cvmx_sso_pnd_ecc_ctl
 *
 * SSO_PND_ECC_CTL = SSO PND ECC Control
 *
 */
union cvmx_sso_pnd_ecc_ctl {
	u64 u64;
	struct cvmx_sso_pnd_ecc_ctl_s {
		u64 reserved_6_63 : 58;
		u64 flip_synd1 : 2;
		u64 ecc_ena1 : 1;
		u64 flip_synd0 : 2;
		u64 ecc_ena0 : 1;
	} s;
	struct cvmx_sso_pnd_ecc_ctl_s cn68xx;
	struct cvmx_sso_pnd_ecc_ctl_s cn68xxp1;
};

typedef union cvmx_sso_pnd_ecc_ctl cvmx_sso_pnd_ecc_ctl_t;

/**
 * cvmx_sso_pnd_ecc_st
 *
 * SSO_PND_ECC_ST = SSO PND ECC Status
 *
 */
union cvmx_sso_pnd_ecc_st {
	u64 u64;
	struct cvmx_sso_pnd_ecc_st_s {
		u64 reserved_59_63 : 5;
		u64 addr1 : 11;
		u64 reserved_43_47 : 5;
		u64 syndrom1 : 7;
		u64 reserved_27_35 : 9;
		u64 addr0 : 11;
		u64 reserved_11_15 : 5;
		u64 syndrom0 : 7;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_sso_pnd_ecc_st_s cn68xx;
	struct cvmx_sso_pnd_ecc_st_s cn68xxp1;
};

typedef union cvmx_sso_pnd_ecc_st cvmx_sso_pnd_ecc_st_t;

/**
 * cvmx_sso_pp#_arb
 *
 * For diagnostic use, returns the group affinity arbitration state for each core.
 *
 */
union cvmx_sso_ppx_arb {
	u64 u64;
	struct cvmx_sso_ppx_arb_s {
		u64 reserved_20_63 : 44;
		u64 aff_left : 4;
		u64 reserved_8_15 : 8;
		u64 last_grp : 8;
	} s;
	struct cvmx_sso_ppx_arb_s cn73xx;
	struct cvmx_sso_ppx_arb_s cn78xx;
	struct cvmx_sso_ppx_arb_s cn78xxp1;
	struct cvmx_sso_ppx_arb_s cnf75xx;
};

typedef union cvmx_sso_ppx_arb cvmx_sso_ppx_arb_t;

/**
 * cvmx_sso_pp#_grp_msk
 *
 * CSR reserved addresses: (24): 0x5040..0x50f8
 * CSR align addresses: ===========================================================================================================
 * SSO_PPX_GRP_MSK = SSO PP Group Mask Register
 *                   (one bit per group per PP)
 *
 * Selects which group(s) a PP belongs to.  A '1' in any bit position sets the
 * PP's membership in the corresponding group.  A value of 0x0 will prevent the
 * PP from receiving new work.
 *
 * Note that these do not contain QOS level priorities for each PP.  This is a
 * change from previous POW designs.
 */
union cvmx_sso_ppx_grp_msk {
	u64 u64;
	struct cvmx_sso_ppx_grp_msk_s {
		u64 grp_msk : 64;
	} s;
	struct cvmx_sso_ppx_grp_msk_s cn68xx;
	struct cvmx_sso_ppx_grp_msk_s cn68xxp1;
};

typedef union cvmx_sso_ppx_grp_msk cvmx_sso_ppx_grp_msk_t;

/**
 * cvmx_sso_pp#_qos_pri
 *
 * CSR reserved addresses: (56): 0x2040..0x21f8
 * CSR align addresses: ===========================================================================================================
 * SSO_PP(0..31)_QOS_PRI = SSO PP QOS Priority Register
 *                                (one field per IQ per PP)
 *
 * Contains the QOS level priorities for each PP.
 *      0x0       is the highest priority
 *      0x7       is the lowest priority
 *      0xf       prevents the PP from receiving work from that QOS level
 *      0x8-0xe   Reserved
 *
 * For a given PP, priorities should begin at 0x0, and remain contiguous
 * throughout the range.  Failure to do so may result in severe
 * performance degradation.
 *
 *
 * Priorities for IQs 0..7
 */
union cvmx_sso_ppx_qos_pri {
	u64 u64;
	struct cvmx_sso_ppx_qos_pri_s {
		u64 reserved_60_63 : 4;
		u64 qos7_pri : 4;
		u64 reserved_52_55 : 4;
		u64 qos6_pri : 4;
		u64 reserved_44_47 : 4;
		u64 qos5_pri : 4;
		u64 reserved_36_39 : 4;
		u64 qos4_pri : 4;
		u64 reserved_28_31 : 4;
		u64 qos3_pri : 4;
		u64 reserved_20_23 : 4;
		u64 qos2_pri : 4;
		u64 reserved_12_15 : 4;
		u64 qos1_pri : 4;
		u64 reserved_4_7 : 4;
		u64 qos0_pri : 4;
	} s;
	struct cvmx_sso_ppx_qos_pri_s cn68xx;
	struct cvmx_sso_ppx_qos_pri_s cn68xxp1;
};

typedef union cvmx_sso_ppx_qos_pri cvmx_sso_ppx_qos_pri_t;

/**
 * cvmx_sso_pp#_s#_grpmsk#
 *
 * These registers select which group or groups a core belongs to. There are 2 sets of masks per
 * core, each with 1 register corresponding to 64 groups.
 */
union cvmx_sso_ppx_sx_grpmskx {
	u64 u64;
	struct cvmx_sso_ppx_sx_grpmskx_s {
		u64 grp_msk : 64;
	} s;
	struct cvmx_sso_ppx_sx_grpmskx_s cn73xx;
	struct cvmx_sso_ppx_sx_grpmskx_s cn78xx;
	struct cvmx_sso_ppx_sx_grpmskx_s cn78xxp1;
	struct cvmx_sso_ppx_sx_grpmskx_s cnf75xx;
};

typedef union cvmx_sso_ppx_sx_grpmskx cvmx_sso_ppx_sx_grpmskx_t;

/**
 * cvmx_sso_pp_strict
 *
 * SSO_PP_STRICT = SSO Strict Priority
 *
 * This register controls getting work from the input queues.  If the bit
 * corresponding to a PP is set, that PP will not take work off the input
 * queues until it is known that there is no higher-priority work available.
 *
 * Setting SSO_PP_STRICT may incur a performance penalty if highest-priority
 * work is not found early.
 *
 * It is possible to starve a PP of work with SSO_PP_STRICT.  If the
 * SSO_PPX_GRP_MSK for a PP masks-out much of the work added to the input
 * queues that are higher-priority for that PP, and if there is a constant
 * stream of work through one or more of those higher-priority input queues,
 * then that PP may not accept work from lower-priority input queues.  This can
 * be alleviated by ensuring that most or all the work added to the
 * higher-priority input queues for a PP with SSO_PP_STRICT set are in a group
 * acceptable to that PP.
 *
 * It is also possible to neglect work in an input queue if SSO_PP_STRICT is
 * used.  If an input queue is a lower-priority queue for all PPs, and if all
 * the PPs have their corresponding bit in SSO_PP_STRICT set, then work may
 * never be taken (or be seldom taken) from that queue.  This can be alleviated
 * by ensuring that work in all input queues can be serviced by one or more PPs
 * that do not have SSO_PP_STRICT set, or that the input queue is the
 * highest-priority input queue for one or more PPs that do have SSO_PP_STRICT
 * set.
 */
union cvmx_sso_pp_strict {
	u64 u64;
	struct cvmx_sso_pp_strict_s {
		u64 reserved_32_63 : 32;
		u64 pp_strict : 32;
	} s;
	struct cvmx_sso_pp_strict_s cn68xx;
	struct cvmx_sso_pp_strict_s cn68xxp1;
};

typedef union cvmx_sso_pp_strict cvmx_sso_pp_strict_t;

/**
 * cvmx_sso_qos#_rnd
 *
 * CSR align addresses: ===========================================================================================================
 * SSO_QOS(0..7)_RND = SSO QOS Issue Round Register
 *                (one per IQ)
 *
 * The number of arbitration rounds each QOS level participates in.
 */
union cvmx_sso_qosx_rnd {
	u64 u64;
	struct cvmx_sso_qosx_rnd_s {
		u64 reserved_8_63 : 56;
		u64 rnds_qos : 8;
	} s;
	struct cvmx_sso_qosx_rnd_s cn68xx;
	struct cvmx_sso_qosx_rnd_s cn68xxp1;
};

typedef union cvmx_sso_qosx_rnd cvmx_sso_qosx_rnd_t;

/**
 * cvmx_sso_qos_thr#
 *
 * CSR reserved addresses: (24): 0xa040..0xa0f8
 * CSR align addresses: ===========================================================================================================
 * SSO_QOS_THRX = SSO QOS Threshold Register
 *                (one per QOS level)
 *
 * Contains the thresholds for allocating SSO internal storage buffers.  If the
 * number of remaining free buffers drops below the minimum threshold (MIN_THR)
 * or the number of allocated buffers for this QOS level rises above the
 * maximum threshold (MAX_THR), future incoming work queue entries will be
 * buffered externally rather than internally.  This register also contains the
 * number of internal buffers currently allocated to this QOS level (BUF_CNT).
 */
union cvmx_sso_qos_thrx {
	u64 u64;
	struct cvmx_sso_qos_thrx_s {
		u64 reserved_40_63 : 24;
		u64 buf_cnt : 12;
		u64 reserved_26_27 : 2;
		u64 max_thr : 12;
		u64 reserved_12_13 : 2;
		u64 min_thr : 12;
	} s;
	struct cvmx_sso_qos_thrx_s cn68xx;
	struct cvmx_sso_qos_thrx_s cn68xxp1;
};

typedef union cvmx_sso_qos_thrx cvmx_sso_qos_thrx_t;

/**
 * cvmx_sso_qos_we
 *
 * SSO_QOS_WE = SSO WE Buffers
 *
 * This register contains a read-only count of the current number of free
 * buffers (FREE_CNT) and the total number of tag chain heads on the de-schedule list
 * (DES_CNT) (which is not the same as the total number of entries on all of the descheduled
 * tag chains.)
 */
union cvmx_sso_qos_we {
	u64 u64;
	struct cvmx_sso_qos_we_s {
		u64 reserved_26_63 : 38;
		u64 des_cnt : 12;
		u64 reserved_12_13 : 2;
		u64 free_cnt : 12;
	} s;
	struct cvmx_sso_qos_we_s cn68xx;
	struct cvmx_sso_qos_we_s cn68xxp1;
};

typedef union cvmx_sso_qos_we cvmx_sso_qos_we_t;

/**
 * cvmx_sso_reset
 *
 * Writing a 1 to SSO_RESET[RESET] resets the SSO. After receiving a store to this CSR, the SSO
 * must not be sent any other operations for 2500 coprocessor (SCLK) cycles. Note that the
 * contents of this register are reset along with the rest of the SSO.
 */
union cvmx_sso_reset {
	u64 u64;
	struct cvmx_sso_reset_s {
		u64 busy : 1;
		u64 reserved_1_62 : 62;
		u64 reset : 1;
	} s;
	struct cvmx_sso_reset_cn68xx {
		u64 reserved_1_63 : 63;
		u64 reset : 1;
	} cn68xx;
	struct cvmx_sso_reset_s cn73xx;
	struct cvmx_sso_reset_s cn78xx;
	struct cvmx_sso_reset_s cn78xxp1;
	struct cvmx_sso_reset_s cnf75xx;
};

typedef union cvmx_sso_reset cvmx_sso_reset_t;

/**
 * cvmx_sso_rwq_head_ptr#
 *
 * CSR reserved addresses: (24): 0xb040..0xb0f8
 * CSR align addresses: ===========================================================================================================
 * SSO_RWQ_HEAD_PTRX = SSO Remote Queue Head Register
 *                (one per QOS level)
 * Contains the ptr to the first entry of the remote linked list(s) for a particular
 * QoS level. SW should initialize the remote linked list(s) by programming
 * SSO_RWQ_HEAD_PTRX and SSO_RWQ_TAIL_PTRX to identical values.
 */
union cvmx_sso_rwq_head_ptrx {
	u64 u64;
	struct cvmx_sso_rwq_head_ptrx_s {
		u64 reserved_38_63 : 26;
		u64 ptr : 31;
		u64 reserved_5_6 : 2;
		u64 rctr : 5;
	} s;
	struct cvmx_sso_rwq_head_ptrx_s cn68xx;
	struct cvmx_sso_rwq_head_ptrx_s cn68xxp1;
};

typedef union cvmx_sso_rwq_head_ptrx cvmx_sso_rwq_head_ptrx_t;

/**
 * cvmx_sso_rwq_pop_fptr
 *
 * SSO_RWQ_POP_FPTR = SSO Pop Free Pointer
 *
 * This register is used by SW to remove pointers for buffer-reallocation and diagnostics, and
 * should only be used when SSO is idle.
 *
 * To remove ALL pointers, software must insure that there are modulus 16
 * pointers in the FPA.  To do this, SSO_CFG.RWQ_BYP_DIS must be set, the FPA
 * pointer count read, and enough fake buffers pushed via SSO_RWQ_PSH_FPTR to
 * bring the FPA pointer count up to mod 16.
 */
union cvmx_sso_rwq_pop_fptr {
	u64 u64;
	struct cvmx_sso_rwq_pop_fptr_s {
		u64 val : 1;
		u64 reserved_38_62 : 25;
		u64 fptr : 31;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_sso_rwq_pop_fptr_s cn68xx;
	struct cvmx_sso_rwq_pop_fptr_s cn68xxp1;
};

typedef union cvmx_sso_rwq_pop_fptr cvmx_sso_rwq_pop_fptr_t;

/**
 * cvmx_sso_rwq_psh_fptr
 *
 * CSR reserved addresses: (56): 0xc240..0xc3f8
 * SSO_RWQ_PSH_FPTR = SSO Free Pointer FIFO
 *
 * This register is used by SW to initialize the SSO with a pool of free
 * pointers by writing the FPTR field whenever FULL = 0. Free pointers are
 * fetched/released from/to the pool when accessing WQE entries stored remotely
 * (in remote linked lists).  Free pointers should be 128 byte aligned, each of
 * 256 bytes. This register should only be used when SSO is idle.
 *
 * Software needs to set aside buffering for
 *      8 + 48 + ROUNDUP(N/26)
 *
 * where as many as N DRAM work queue entries may be used.  The first 8 buffers
 * are used to setup the SSO_RWQ_HEAD_PTR and SSO_RWQ_TAIL_PTRs, and the
 * remainder are pushed via this register.
 *
 * IMPLEMENTATION NOTES--NOT FOR SPEC:
 *      48 avoids false out of buffer error due to (16) FPA and in-sso FPA buffering (32)
 *      26 is number of WAE's per 256B buffer
 */
union cvmx_sso_rwq_psh_fptr {
	u64 u64;
	struct cvmx_sso_rwq_psh_fptr_s {
		u64 full : 1;
		u64 reserved_38_62 : 25;
		u64 fptr : 31;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_sso_rwq_psh_fptr_s cn68xx;
	struct cvmx_sso_rwq_psh_fptr_s cn68xxp1;
};

typedef union cvmx_sso_rwq_psh_fptr cvmx_sso_rwq_psh_fptr_t;

/**
 * cvmx_sso_rwq_tail_ptr#
 *
 * CSR reserved addresses: (56): 0xc040..0xc1f8
 * SSO_RWQ_TAIL_PTRX = SSO Remote Queue Tail Register
 *                (one per QOS level)
 * Contains the ptr to the last entry of the remote linked list(s) for a particular
 * QoS level. SW must initialize the remote linked list(s) by programming
 * SSO_RWQ_HEAD_PTRX and SSO_RWQ_TAIL_PTRX to identical values.
 */
union cvmx_sso_rwq_tail_ptrx {
	u64 u64;
	struct cvmx_sso_rwq_tail_ptrx_s {
		u64 reserved_38_63 : 26;
		u64 ptr : 31;
		u64 reserved_5_6 : 2;
		u64 rctr : 5;
	} s;
	struct cvmx_sso_rwq_tail_ptrx_s cn68xx;
	struct cvmx_sso_rwq_tail_ptrx_s cn68xxp1;
};

typedef union cvmx_sso_rwq_tail_ptrx cvmx_sso_rwq_tail_ptrx_t;

/**
 * cvmx_sso_sl_pp#_links
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_links {
	u64 u64;
	struct cvmx_sso_sl_ppx_links_s {
		u64 tailc : 1;
		u64 reserved_60_62 : 3;
		u64 index : 12;
		u64 reserved_38_47 : 10;
		u64 grp : 10;
		u64 head : 1;
		u64 tail : 1;
		u64 reserved_0_25 : 26;
	} s;
	struct cvmx_sso_sl_ppx_links_cn73xx {
		u64 tailc : 1;
		u64 reserved_58_62 : 5;
		u64 index : 10;
		u64 reserved_36_47 : 12;
		u64 grp : 8;
		u64 head : 1;
		u64 tail : 1;
		u64 reserved_21_25 : 5;
		u64 revlink_index : 10;
		u64 link_index_vld : 1;
		u64 link_index : 10;
	} cn73xx;
	struct cvmx_sso_sl_ppx_links_cn78xx {
		u64 tailc : 1;
		u64 reserved_60_62 : 3;
		u64 index : 12;
		u64 reserved_38_47 : 10;
		u64 grp : 10;
		u64 head : 1;
		u64 tail : 1;
		u64 reserved_25_25 : 1;
		u64 revlink_index : 12;
		u64 link_index_vld : 1;
		u64 link_index : 12;
	} cn78xx;
	struct cvmx_sso_sl_ppx_links_cn78xx cn78xxp1;
	struct cvmx_sso_sl_ppx_links_cn73xx cnf75xx;
};

typedef union cvmx_sso_sl_ppx_links cvmx_sso_sl_ppx_links_t;

/**
 * cvmx_sso_sl_pp#_pendtag
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_pendtag {
	u64 u64;
	struct cvmx_sso_sl_ppx_pendtag_s {
		u64 pend_switch : 1;
		u64 pend_get_work : 1;
		u64 pend_get_work_wait : 1;
		u64 pend_nosched : 1;
		u64 pend_nosched_clr : 1;
		u64 pend_desched : 1;
		u64 pend_alloc_we : 1;
		u64 pend_gw_insert : 1;
		u64 reserved_34_55 : 22;
		u64 pend_tt : 2;
		u64 pend_tag : 32;
	} s;
	struct cvmx_sso_sl_ppx_pendtag_s cn73xx;
	struct cvmx_sso_sl_ppx_pendtag_s cn78xx;
	struct cvmx_sso_sl_ppx_pendtag_s cn78xxp1;
	struct cvmx_sso_sl_ppx_pendtag_s cnf75xx;
};

typedef union cvmx_sso_sl_ppx_pendtag cvmx_sso_sl_ppx_pendtag_t;

/**
 * cvmx_sso_sl_pp#_pendwqp
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_pendwqp {
	u64 u64;
	struct cvmx_sso_sl_ppx_pendwqp_s {
		u64 pend_switch : 1;
		u64 pend_get_work : 1;
		u64 pend_get_work_wait : 1;
		u64 pend_nosched : 1;
		u64 pend_nosched_clr : 1;
		u64 pend_desched : 1;
		u64 pend_alloc_we : 1;
		u64 reserved_56_56 : 1;
		u64 pend_index : 12;
		u64 reserved_42_43 : 2;
		u64 pend_wqp : 42;
	} s;
	struct cvmx_sso_sl_ppx_pendwqp_cn73xx {
		u64 pend_switch : 1;
		u64 pend_get_work : 1;
		u64 pend_get_work_wait : 1;
		u64 pend_nosched : 1;
		u64 pend_nosched_clr : 1;
		u64 pend_desched : 1;
		u64 pend_alloc_we : 1;
		u64 reserved_54_56 : 3;
		u64 pend_index : 10;
		u64 reserved_42_43 : 2;
		u64 pend_wqp : 42;
	} cn73xx;
	struct cvmx_sso_sl_ppx_pendwqp_s cn78xx;
	struct cvmx_sso_sl_ppx_pendwqp_s cn78xxp1;
	struct cvmx_sso_sl_ppx_pendwqp_cn73xx cnf75xx;
};

typedef union cvmx_sso_sl_ppx_pendwqp cvmx_sso_sl_ppx_pendwqp_t;

/**
 * cvmx_sso_sl_pp#_tag
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_tag {
	u64 u64;
	struct cvmx_sso_sl_ppx_tag_s {
		u64 tailc : 1;
		u64 reserved_60_62 : 3;
		u64 index : 12;
		u64 reserved_46_47 : 2;
		u64 grp : 10;
		u64 head : 1;
		u64 tail : 1;
		u64 tt : 2;
		u64 tag : 32;
	} s;
	struct cvmx_sso_sl_ppx_tag_cn73xx {
		u64 tailc : 1;
		u64 reserved_58_62 : 5;
		u64 index : 10;
		u64 reserved_44_47 : 4;
		u64 grp : 8;
		u64 head : 1;
		u64 tail : 1;
		u64 tt : 2;
		u64 tag : 32;
	} cn73xx;
	struct cvmx_sso_sl_ppx_tag_s cn78xx;
	struct cvmx_sso_sl_ppx_tag_s cn78xxp1;
	struct cvmx_sso_sl_ppx_tag_cn73xx cnf75xx;
};

typedef union cvmx_sso_sl_ppx_tag cvmx_sso_sl_ppx_tag_t;

/**
 * cvmx_sso_sl_pp#_wqp
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_wqp {
	u64 u64;
	struct cvmx_sso_sl_ppx_wqp_s {
		u64 reserved_58_63 : 6;
		u64 grp : 10;
		u64 reserved_42_47 : 6;
		u64 wqp : 42;
	} s;
	struct cvmx_sso_sl_ppx_wqp_cn73xx {
		u64 reserved_56_63 : 8;
		u64 grp : 8;
		u64 reserved_42_47 : 6;
		u64 wqp : 42;
	} cn73xx;
	struct cvmx_sso_sl_ppx_wqp_s cn78xx;
	struct cvmx_sso_sl_ppx_wqp_s cn78xxp1;
	struct cvmx_sso_sl_ppx_wqp_cn73xx cnf75xx;
};

typedef union cvmx_sso_sl_ppx_wqp cvmx_sso_sl_ppx_wqp_t;

/**
 * cvmx_sso_taq#_link
 *
 * Returns TAQ status for a given line.
 *
 */
union cvmx_sso_taqx_link {
	u64 u64;
	struct cvmx_sso_taqx_link_s {
		u64 reserved_11_63 : 53;
		u64 next : 11;
	} s;
	struct cvmx_sso_taqx_link_s cn73xx;
	struct cvmx_sso_taqx_link_s cn78xx;
	struct cvmx_sso_taqx_link_s cn78xxp1;
	struct cvmx_sso_taqx_link_s cnf75xx;
};

typedef union cvmx_sso_taqx_link cvmx_sso_taqx_link_t;

/**
 * cvmx_sso_taq#_wae#_tag
 *
 * Returns TAQ status for a given line and WAE within that line.
 *
 */
union cvmx_sso_taqx_waex_tag {
	u64 u64;
	struct cvmx_sso_taqx_waex_tag_s {
		u64 reserved_34_63 : 30;
		u64 tt : 2;
		u64 tag : 32;
	} s;
	struct cvmx_sso_taqx_waex_tag_s cn73xx;
	struct cvmx_sso_taqx_waex_tag_s cn78xx;
	struct cvmx_sso_taqx_waex_tag_s cn78xxp1;
	struct cvmx_sso_taqx_waex_tag_s cnf75xx;
};

typedef union cvmx_sso_taqx_waex_tag cvmx_sso_taqx_waex_tag_t;

/**
 * cvmx_sso_taq#_wae#_wqp
 *
 * Returns TAQ status for a given line and WAE within that line.
 *
 */
union cvmx_sso_taqx_waex_wqp {
	u64 u64;
	struct cvmx_sso_taqx_waex_wqp_s {
		u64 reserved_42_63 : 22;
		u64 wqp : 42;
	} s;
	struct cvmx_sso_taqx_waex_wqp_s cn73xx;
	struct cvmx_sso_taqx_waex_wqp_s cn78xx;
	struct cvmx_sso_taqx_waex_wqp_s cn78xxp1;
	struct cvmx_sso_taqx_waex_wqp_s cnf75xx;
};

typedef union cvmx_sso_taqx_waex_wqp cvmx_sso_taqx_waex_wqp_t;

/**
 * cvmx_sso_taq_add
 */
union cvmx_sso_taq_add {
	u64 u64;
	struct cvmx_sso_taq_add_s {
		u64 reserved_29_63 : 35;
		u64 rsvd_free : 13;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_sso_taq_add_s cn73xx;
	struct cvmx_sso_taq_add_s cn78xx;
	struct cvmx_sso_taq_add_s cn78xxp1;
	struct cvmx_sso_taq_add_s cnf75xx;
};

typedef union cvmx_sso_taq_add cvmx_sso_taq_add_t;

/**
 * cvmx_sso_taq_cnt
 */
union cvmx_sso_taq_cnt {
	u64 u64;
	struct cvmx_sso_taq_cnt_s {
		u64 reserved_27_63 : 37;
		u64 rsvd_free : 11;
		u64 reserved_11_15 : 5;
		u64 free_cnt : 11;
	} s;
	struct cvmx_sso_taq_cnt_s cn73xx;
	struct cvmx_sso_taq_cnt_s cn78xx;
	struct cvmx_sso_taq_cnt_s cn78xxp1;
	struct cvmx_sso_taq_cnt_s cnf75xx;
};

typedef union cvmx_sso_taq_cnt cvmx_sso_taq_cnt_t;

/**
 * cvmx_sso_tiaq#_status
 *
 * Returns TAQ inbound status indexed by group.
 *
 */
union cvmx_sso_tiaqx_status {
	u64 u64;
	struct cvmx_sso_tiaqx_status_s {
		u64 wae_head : 4;
		u64 wae_tail : 4;
		u64 reserved_47_55 : 9;
		u64 wae_used : 15;
		u64 reserved_23_31 : 9;
		u64 ent_head : 11;
		u64 reserved_11_11 : 1;
		u64 ent_tail : 11;
	} s;
	struct cvmx_sso_tiaqx_status_s cn73xx;
	struct cvmx_sso_tiaqx_status_s cn78xx;
	struct cvmx_sso_tiaqx_status_s cn78xxp1;
	struct cvmx_sso_tiaqx_status_s cnf75xx;
};

typedef union cvmx_sso_tiaqx_status cvmx_sso_tiaqx_status_t;

/**
 * cvmx_sso_toaq#_status
 *
 * Returns TAQ outbound status indexed by group.
 *
 */
union cvmx_sso_toaqx_status {
	u64 u64;
	struct cvmx_sso_toaqx_status_s {
		u64 reserved_62_63 : 2;
		u64 ext_vld : 1;
		u64 partial : 1;
		u64 wae_tail : 4;
		u64 reserved_43_55 : 13;
		u64 cl_used : 11;
		u64 reserved_23_31 : 9;
		u64 ent_head : 11;
		u64 reserved_11_11 : 1;
		u64 ent_tail : 11;
	} s;
	struct cvmx_sso_toaqx_status_s cn73xx;
	struct cvmx_sso_toaqx_status_s cn78xx;
	struct cvmx_sso_toaqx_status_s cn78xxp1;
	struct cvmx_sso_toaqx_status_s cnf75xx;
};

typedef union cvmx_sso_toaqx_status cvmx_sso_toaqx_status_t;

/**
 * cvmx_sso_ts_pc
 *
 * SSO_TS_PC = SSO Tag Switch Performance Counter
 *
 * Counts the number of tag switch requests.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ts_pc {
	u64 u64;
	struct cvmx_sso_ts_pc_s {
		u64 ts_pc : 64;
	} s;
	struct cvmx_sso_ts_pc_s cn68xx;
	struct cvmx_sso_ts_pc_s cn68xxp1;
};

typedef union cvmx_sso_ts_pc cvmx_sso_ts_pc_t;

/**
 * cvmx_sso_wa_com_pc
 *
 * SSO_WA_COM_PC = SSO Work Add Combined Performance Counter
 *
 * Counts the number of add new work requests for all QOS levels.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_wa_com_pc {
	u64 u64;
	struct cvmx_sso_wa_com_pc_s {
		u64 wa_pc : 64;
	} s;
	struct cvmx_sso_wa_com_pc_s cn68xx;
	struct cvmx_sso_wa_com_pc_s cn68xxp1;
};

typedef union cvmx_sso_wa_com_pc cvmx_sso_wa_com_pc_t;

/**
 * cvmx_sso_wa_pc#
 *
 * CSR reserved addresses: (64): 0x4200..0x43f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WA_PCX = SSO Work Add Performance Counter
 *             (one per QOS level)
 *
 * Counts the number of add new work requests for each QOS level.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_wa_pcx {
	u64 u64;
	struct cvmx_sso_wa_pcx_s {
		u64 wa_pc : 64;
	} s;
	struct cvmx_sso_wa_pcx_s cn68xx;
	struct cvmx_sso_wa_pcx_s cn68xxp1;
};

typedef union cvmx_sso_wa_pcx cvmx_sso_wa_pcx_t;

/**
 * cvmx_sso_wq_int
 *
 * Note, the old POW offsets ran from 0x0 to 0x3f8, leaving the next available slot at 0x400.
 * To ensure no overlap, start on 4k boundary: 0x1000.
 * SSO_WQ_INT = SSO Work Queue Interrupt Register
 *
 * Contains the bits (one per group) that set work queue interrupts and are
 * used to clear these interrupts.  For more information regarding this
 * register, see the interrupt section of the SSO spec.
 */
union cvmx_sso_wq_int {
	u64 u64;
	struct cvmx_sso_wq_int_s {
		u64 wq_int : 64;
	} s;
	struct cvmx_sso_wq_int_s cn68xx;
	struct cvmx_sso_wq_int_s cn68xxp1;
};

typedef union cvmx_sso_wq_int cvmx_sso_wq_int_t;

/**
 * cvmx_sso_wq_int_cnt#
 *
 * CSR reserved addresses: (64): 0x7200..0x73f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WQ_INT_CNTX = SSO Work Queue Interrupt Count Register
 *                   (one per group)
 *
 * Contains a read-only copy of the counts used to trigger work queue
 * interrupts.  For more information regarding this register, see the interrupt
 * section.
 */
union cvmx_sso_wq_int_cntx {
	u64 u64;
	struct cvmx_sso_wq_int_cntx_s {
		u64 reserved_32_63 : 32;
		u64 tc_cnt : 4;
		u64 reserved_26_27 : 2;
		u64 ds_cnt : 12;
		u64 reserved_12_13 : 2;
		u64 iq_cnt : 12;
	} s;
	struct cvmx_sso_wq_int_cntx_s cn68xx;
	struct cvmx_sso_wq_int_cntx_s cn68xxp1;
};

typedef union cvmx_sso_wq_int_cntx cvmx_sso_wq_int_cntx_t;

/**
 * cvmx_sso_wq_int_pc
 *
 * Contains the threshold value for the work-executable interrupt periodic counter and also a
 * read-only copy of the periodic counter. For more information on this register, refer to
 * Interrupts.
 */
union cvmx_sso_wq_int_pc {
	u64 u64;
	struct cvmx_sso_wq_int_pc_s {
		u64 reserved_60_63 : 4;
		u64 pc : 28;
		u64 reserved_28_31 : 4;
		u64 pc_thr : 20;
		u64 reserved_0_7 : 8;
	} s;
	struct cvmx_sso_wq_int_pc_s cn68xx;
	struct cvmx_sso_wq_int_pc_s cn68xxp1;
	struct cvmx_sso_wq_int_pc_s cn73xx;
	struct cvmx_sso_wq_int_pc_s cn78xx;
	struct cvmx_sso_wq_int_pc_s cn78xxp1;
	struct cvmx_sso_wq_int_pc_s cnf75xx;
};

typedef union cvmx_sso_wq_int_pc cvmx_sso_wq_int_pc_t;

/**
 * cvmx_sso_wq_int_thr#
 *
 * CSR reserved addresses: (96): 0x6100..0x63f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WQ_INT_THR(0..63) = SSO Work Queue Interrupt Threshold Registers
 *                         (one per group)
 *
 * Contains the thresholds for enabling and setting work queue interrupts.  For
 * more information, see the interrupt section.
 *
 * Note: Up to 16 of the SSO's internal storage buffers can be allocated
 * for hardware use and are therefore not available for incoming work queue
 * entries.  Additionally, any WS that is not in the EMPTY state consumes a
 * buffer.  Thus in a 32 PP system, it is not advisable to set either IQ_THR or
 * DS_THR to greater than 2048 - 16 - 32*2 = 1968.  Doing so may prevent the
 * interrupt from ever triggering.
 *
 * Priorities for QOS levels 0..7
 */
union cvmx_sso_wq_int_thrx {
	u64 u64;
	struct cvmx_sso_wq_int_thrx_s {
		u64 reserved_33_63 : 31;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_26_27 : 2;
		u64 ds_thr : 12;
		u64 reserved_12_13 : 2;
		u64 iq_thr : 12;
	} s;
	struct cvmx_sso_wq_int_thrx_s cn68xx;
	struct cvmx_sso_wq_int_thrx_s cn68xxp1;
};

typedef union cvmx_sso_wq_int_thrx cvmx_sso_wq_int_thrx_t;

/**
 * cvmx_sso_wq_iq_dis
 *
 * CSR reserved addresses: (1): 0x1008..0x1008
 * SSO_WQ_IQ_DIS = SSO Input Queue Interrupt Temporary Disable Mask
 *
 * Contains the input queue interrupt temporary disable bits (one per group).
 * For more information regarding this register, see the interrupt section.
 */
union cvmx_sso_wq_iq_dis {
	u64 u64;
	struct cvmx_sso_wq_iq_dis_s {
		u64 iq_dis : 64;
	} s;
	struct cvmx_sso_wq_iq_dis_s cn68xx;
	struct cvmx_sso_wq_iq_dis_s cn68xxp1;
};

typedef union cvmx_sso_wq_iq_dis cvmx_sso_wq_iq_dis_t;

/**
 * cvmx_sso_ws_cfg
 *
 * This register contains various SSO work-slot configuration bits.
 *
 */
union cvmx_sso_ws_cfg {
	u64 u64;
	struct cvmx_sso_ws_cfg_s {
		u64 reserved_56_63 : 8;
		u64 ocla_bp : 8;
		u64 reserved_7_47 : 41;
		u64 aw_clk_dis : 1;
		u64 gw_clk_dis : 1;
		u64 disable_pw : 1;
		u64 arbc_step_en : 1;
		u64 ncbo_step_en : 1;
		u64 soc_ccam_dis : 1;
		u64 sso_cclk_dis : 1;
	} s;
	struct cvmx_sso_ws_cfg_s cn73xx;
	struct cvmx_sso_ws_cfg_cn78xx {
		u64 reserved_56_63 : 8;
		u64 ocla_bp : 8;
		u64 reserved_5_47 : 43;
		u64 disable_pw : 1;
		u64 arbc_step_en : 1;
		u64 ncbo_step_en : 1;
		u64 soc_ccam_dis : 1;
		u64 sso_cclk_dis : 1;
	} cn78xx;
	struct cvmx_sso_ws_cfg_cn78xx cn78xxp1;
	struct cvmx_sso_ws_cfg_s cnf75xx;
};

typedef union cvmx_sso_ws_cfg cvmx_sso_ws_cfg_t;

/**
 * cvmx_sso_ws_eco
 */
union cvmx_sso_ws_eco {
	u64 u64;
	struct cvmx_sso_ws_eco_s {
		u64 reserved_8_63 : 56;
		u64 eco_rw : 8;
	} s;
	struct cvmx_sso_ws_eco_s cn73xx;
	struct cvmx_sso_ws_eco_s cnf75xx;
};

typedef union cvmx_sso_ws_eco cvmx_sso_ws_eco_t;

/**
 * cvmx_sso_ws_pc#
 *
 * CSR reserved addresses: (225): 0x3100..0x3800
 * CSR align addresses: ===========================================================================================================
 * SSO_WS_PCX = SSO Work Schedule Performance Counter
 *              (one per group)
 *
 * Counts the number of work schedules for each group.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ws_pcx {
	u64 u64;
	struct cvmx_sso_ws_pcx_s {
		u64 ws_pc : 64;
	} s;
	struct cvmx_sso_ws_pcx_s cn68xx;
	struct cvmx_sso_ws_pcx_s cn68xxp1;
};

typedef union cvmx_sso_ws_pcx cvmx_sso_ws_pcx_t;

/**
 * cvmx_sso_xaq#_head_next
 *
 * These registers contain the pointer to the next buffer to become the head when the final cache
 * line in this buffer is read.
 */
union cvmx_sso_xaqx_head_next {
	u64 u64;
	struct cvmx_sso_xaqx_head_next_s {
		u64 reserved_42_63 : 22;
		u64 ptr : 35;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_sso_xaqx_head_next_s cn73xx;
	struct cvmx_sso_xaqx_head_next_s cn78xx;
	struct cvmx_sso_xaqx_head_next_s cn78xxp1;
	struct cvmx_sso_xaqx_head_next_s cnf75xx;
};

typedef union cvmx_sso_xaqx_head_next cvmx_sso_xaqx_head_next_t;

/**
 * cvmx_sso_xaq#_head_ptr
 *
 * These registers contain the pointer to the first entry of the external linked list(s) for a
 * particular group. Software must initialize the external linked list(s) by programming
 * SSO_XAQ()_HEAD_PTR, SSO_XAQ()_HEAD_NEXT, SSO_XAQ()_TAIL_PTR and
 * SSO_XAQ()_TAIL_NEXT to identical values.
 */
union cvmx_sso_xaqx_head_ptr {
	u64 u64;
	struct cvmx_sso_xaqx_head_ptr_s {
		u64 reserved_42_63 : 22;
		u64 ptr : 35;
		u64 reserved_5_6 : 2;
		u64 cl : 5;
	} s;
	struct cvmx_sso_xaqx_head_ptr_s cn73xx;
	struct cvmx_sso_xaqx_head_ptr_s cn78xx;
	struct cvmx_sso_xaqx_head_ptr_s cn78xxp1;
	struct cvmx_sso_xaqx_head_ptr_s cnf75xx;
};

typedef union cvmx_sso_xaqx_head_ptr cvmx_sso_xaqx_head_ptr_t;

/**
 * cvmx_sso_xaq#_tail_next
 *
 * These registers contain the pointer to the next buffer to become the tail when the final cache
 * line in this buffer is written.  Register fields are identical to those in
 * SSO_XAQ()_HEAD_NEXT above.
 */
union cvmx_sso_xaqx_tail_next {
	u64 u64;
	struct cvmx_sso_xaqx_tail_next_s {
		u64 reserved_42_63 : 22;
		u64 ptr : 35;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_sso_xaqx_tail_next_s cn73xx;
	struct cvmx_sso_xaqx_tail_next_s cn78xx;
	struct cvmx_sso_xaqx_tail_next_s cn78xxp1;
	struct cvmx_sso_xaqx_tail_next_s cnf75xx;
};

typedef union cvmx_sso_xaqx_tail_next cvmx_sso_xaqx_tail_next_t;

/**
 * cvmx_sso_xaq#_tail_ptr
 *
 * These registers contain the pointer to the last entry of the external linked list(s) for a
 * particular group.  Register fields are identical to those in SSO_XAQ()_HEAD_PTR above.
 * Software must initialize the external linked list(s) by programming
 * SSO_XAQ()_HEAD_PTR, SSO_XAQ()_HEAD_NEXT, SSO_XAQ()_TAIL_PTR and
 * SSO_XAQ()_TAIL_NEXT to identical values.
 */
union cvmx_sso_xaqx_tail_ptr {
	u64 u64;
	struct cvmx_sso_xaqx_tail_ptr_s {
		u64 reserved_42_63 : 22;
		u64 ptr : 35;
		u64 reserved_5_6 : 2;
		u64 cl : 5;
	} s;
	struct cvmx_sso_xaqx_tail_ptr_s cn73xx;
	struct cvmx_sso_xaqx_tail_ptr_s cn78xx;
	struct cvmx_sso_xaqx_tail_ptr_s cn78xxp1;
	struct cvmx_sso_xaqx_tail_ptr_s cnf75xx;
};

typedef union cvmx_sso_xaqx_tail_ptr cvmx_sso_xaqx_tail_ptr_t;

/**
 * cvmx_sso_xaq_aura
 */
union cvmx_sso_xaq_aura {
	u64 u64;
	struct cvmx_sso_xaq_aura_s {
		u64 reserved_12_63 : 52;
		u64 node : 2;
		u64 laura : 10;
	} s;
	struct cvmx_sso_xaq_aura_s cn73xx;
	struct cvmx_sso_xaq_aura_s cn78xx;
	struct cvmx_sso_xaq_aura_s cn78xxp1;
	struct cvmx_sso_xaq_aura_s cnf75xx;
};

typedef union cvmx_sso_xaq_aura cvmx_sso_xaq_aura_t;

/**
 * cvmx_sso_xaq_latency_pc
 */
union cvmx_sso_xaq_latency_pc {
	u64 u64;
	struct cvmx_sso_xaq_latency_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_sso_xaq_latency_pc_s cn73xx;
	struct cvmx_sso_xaq_latency_pc_s cn78xx;
	struct cvmx_sso_xaq_latency_pc_s cn78xxp1;
	struct cvmx_sso_xaq_latency_pc_s cnf75xx;
};

typedef union cvmx_sso_xaq_latency_pc cvmx_sso_xaq_latency_pc_t;

/**
 * cvmx_sso_xaq_req_pc
 */
union cvmx_sso_xaq_req_pc {
	u64 u64;
	struct cvmx_sso_xaq_req_pc_s {
		u64 count : 64;
	} s;
	struct cvmx_sso_xaq_req_pc_s cn73xx;
	struct cvmx_sso_xaq_req_pc_s cn78xx;
	struct cvmx_sso_xaq_req_pc_s cn78xxp1;
	struct cvmx_sso_xaq_req_pc_s cnf75xx;
};

typedef union cvmx_sso_xaq_req_pc cvmx_sso_xaq_req_pc_t;

#endif
