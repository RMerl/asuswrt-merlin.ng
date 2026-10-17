/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon dpi.
 */

#ifndef __CVMX_DPI_DEFS_H__
#define __CVMX_DPI_DEFS_H__

#define CVMX_DPI_BIST_STATUS		     (0x0001DF0000000000ull)
#define CVMX_DPI_CTL			     (0x0001DF0000000040ull)
#define CVMX_DPI_DMAX_COUNTS(offset)	     (0x0001DF0000000300ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_DBELL(offset)	     (0x0001DF0000000200ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_ERR_RSP_STATUS(offset) (0x0001DF0000000A80ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_IBUFF_SADDR(offset)    (0x0001DF0000000280ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_IFLIGHT(offset)	     (0x0001DF0000000A00ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_NADDR(offset)	     (0x0001DF0000000380ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_REQBNK0(offset)	     (0x0001DF0000000400ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_REQBNK1(offset)	     (0x0001DF0000000480ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMAX_REQQ_CTL(offset)	     (0x0001DF0000000180ull + ((offset) & 7) * 8)
#define CVMX_DPI_DMA_CONTROL		     (0x0001DF0000000048ull)
#define CVMX_DPI_DMA_ENGX_EN(offset)	     (0x0001DF0000000080ull + ((offset) & 7) * 8)
static inline u64 CVMX_DPI_DMA_PPX_CNT(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x0001DF0000000B00ull + (offset) * 8;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x0001DF0000000B00ull + (offset) * 8;
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		return 0x0001DF0000000C00ull + (offset) * 8;
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001DF0000000C00ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001DF0000000C00ull + (offset) * 8;
	}
	return 0x0001DF0000000C00ull + (offset) * 8;
}

#define CVMX_DPI_DMA_PP_INT	      (0x0001DF0000000038ull)
#define CVMX_DPI_ECC_CTL	      (0x0001DF0000000018ull)
#define CVMX_DPI_ECC_INT	      (0x0001DF0000000020ull)
#define CVMX_DPI_ENGX_BUF(offset)     (0x0001DF0000000880ull + ((offset) & 7) * 8)
#define CVMX_DPI_INFO_REG	      (0x0001DF0000000980ull)
#define CVMX_DPI_INT_EN		      (0x0001DF0000000010ull)
#define CVMX_DPI_INT_REG	      (0x0001DF0000000008ull)
#define CVMX_DPI_NCBX_CFG(offset)     (0x0001DF0000000800ull)
#define CVMX_DPI_NCB_CTL	      (0x0001DF0000000028ull)
#define CVMX_DPI_PINT_INFO	      (0x0001DF0000000830ull)
#define CVMX_DPI_PKT_ERR_RSP	      (0x0001DF0000000078ull)
#define CVMX_DPI_REQ_ERR_RSP	      (0x0001DF0000000058ull)
#define CVMX_DPI_REQ_ERR_RSP_EN	      (0x0001DF0000000068ull)
#define CVMX_DPI_REQ_ERR_RST	      (0x0001DF0000000060ull)
#define CVMX_DPI_REQ_ERR_RST_EN	      (0x0001DF0000000070ull)
#define CVMX_DPI_REQ_ERR_SKIP_COMP    (0x0001DF0000000838ull)
#define CVMX_DPI_REQ_GBL_EN	      (0x0001DF0000000050ull)
#define CVMX_DPI_SLI_PRTX_CFG(offset) (0x0001DF0000000900ull + ((offset) & 3) * 8)
static inline u64 CVMX_DPI_SLI_PRTX_ERR(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return 0x0001DF0000000920ull + (offset) * 8;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			return 0x0001DF0000000920ull + (offset) * 8;
		return 0x0001DF0000000920ull + (offset) * 8;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:

		if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1))
			return 0x0001DF0000000928ull + (offset) * 8;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2))
			return 0x0001DF0000000920ull + (offset) * 8;
		return 0x0001DF0000000920ull + (offset) * 8;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		return 0x0001DF0000000920ull + (offset) * 8;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0001DF0000000928ull + (offset) * 8;
	}
	return 0x0001DF0000000920ull + (offset) * 8;
}

#define CVMX_DPI_SLI_PRTX_ERR_INFO(offset) (0x0001DF0000000940ull + ((offset) & 3) * 8)
#define CVMX_DPI_SRIO_RX_BELLX(offset)	   (0x0001DF0000080200ull + ((offset) & 31) * 8)
#define CVMX_DPI_SRIO_RX_BELL_SEQX(offset) (0x0001DF0000080400ull + ((offset) & 31) * 8)
#define CVMX_DPI_SWA_Q_VMID		   (0x0001DF0000000030ull)

/**
 * cvmx_dpi_bist_status
 *
 * This is the built-in self-test (BIST) status register. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_dpi_bist_status {
	u64 u64;
	struct cvmx_dpi_bist_status_s {
		u64 reserved_57_63 : 7;
		u64 bist : 57;
	} s;
	struct cvmx_dpi_bist_status_cn61xx {
		u64 reserved_47_63 : 17;
		u64 bist : 47;
	} cn61xx;
	struct cvmx_dpi_bist_status_cn63xx {
		u64 reserved_45_63 : 19;
		u64 bist : 45;
	} cn63xx;
	struct cvmx_dpi_bist_status_cn63xxp1 {
		u64 reserved_37_63 : 27;
		u64 bist : 37;
	} cn63xxp1;
	struct cvmx_dpi_bist_status_cn61xx cn66xx;
	struct cvmx_dpi_bist_status_cn63xx cn68xx;
	struct cvmx_dpi_bist_status_cn63xx cn68xxp1;
	struct cvmx_dpi_bist_status_cn61xx cn70xx;
	struct cvmx_dpi_bist_status_cn61xx cn70xxp1;
	struct cvmx_dpi_bist_status_s cn73xx;
	struct cvmx_dpi_bist_status_s cn78xx;
	struct cvmx_dpi_bist_status_cn78xxp1 {
		u64 reserved_51_63 : 13;
		u64 bist : 51;
	} cn78xxp1;
	struct cvmx_dpi_bist_status_cn61xx cnf71xx;
	struct cvmx_dpi_bist_status_s cnf75xx;
};

typedef union cvmx_dpi_bist_status cvmx_dpi_bist_status_t;

/**
 * cvmx_dpi_ctl
 *
 * This register provides the enable bit for the DMA and packet state machines.
 *
 */
union cvmx_dpi_ctl {
	u64 u64;
	struct cvmx_dpi_ctl_s {
		u64 reserved_2_63 : 62;
		u64 clk : 1;
		u64 en : 1;
	} s;
	struct cvmx_dpi_ctl_cn61xx {
		u64 reserved_1_63 : 63;
		u64 en : 1;
	} cn61xx;
	struct cvmx_dpi_ctl_s cn63xx;
	struct cvmx_dpi_ctl_s cn63xxp1;
	struct cvmx_dpi_ctl_s cn66xx;
	struct cvmx_dpi_ctl_s cn68xx;
	struct cvmx_dpi_ctl_s cn68xxp1;
	struct cvmx_dpi_ctl_cn61xx cn70xx;
	struct cvmx_dpi_ctl_cn61xx cn70xxp1;
	struct cvmx_dpi_ctl_cn61xx cn73xx;
	struct cvmx_dpi_ctl_cn61xx cn78xx;
	struct cvmx_dpi_ctl_cn61xx cn78xxp1;
	struct cvmx_dpi_ctl_cn61xx cnf71xx;
	struct cvmx_dpi_ctl_cn61xx cnf75xx;
};

typedef union cvmx_dpi_ctl cvmx_dpi_ctl_t;

/**
 * cvmx_dpi_dma#_counts
 *
 * These registers provide values for determining the number of instructions in the local
 * instruction FIFO.
 */
union cvmx_dpi_dmax_counts {
	u64 u64;
	struct cvmx_dpi_dmax_counts_s {
		u64 reserved_39_63 : 25;
		u64 fcnt : 7;
		u64 dbell : 32;
	} s;
	struct cvmx_dpi_dmax_counts_s cn61xx;
	struct cvmx_dpi_dmax_counts_s cn63xx;
	struct cvmx_dpi_dmax_counts_s cn63xxp1;
	struct cvmx_dpi_dmax_counts_s cn66xx;
	struct cvmx_dpi_dmax_counts_s cn68xx;
	struct cvmx_dpi_dmax_counts_s cn68xxp1;
	struct cvmx_dpi_dmax_counts_s cn70xx;
	struct cvmx_dpi_dmax_counts_s cn70xxp1;
	struct cvmx_dpi_dmax_counts_s cn73xx;
	struct cvmx_dpi_dmax_counts_s cn78xx;
	struct cvmx_dpi_dmax_counts_s cn78xxp1;
	struct cvmx_dpi_dmax_counts_s cnf71xx;
	struct cvmx_dpi_dmax_counts_s cnf75xx;
};

typedef union cvmx_dpi_dmax_counts cvmx_dpi_dmax_counts_t;

/**
 * cvmx_dpi_dma#_dbell
 *
 * This is the door bell register for the eight DMA instruction queues.
 *
 */
union cvmx_dpi_dmax_dbell {
	u64 u64;
	struct cvmx_dpi_dmax_dbell_s {
		u64 reserved_16_63 : 48;
		u64 dbell : 16;
	} s;
	struct cvmx_dpi_dmax_dbell_s cn61xx;
	struct cvmx_dpi_dmax_dbell_s cn63xx;
	struct cvmx_dpi_dmax_dbell_s cn63xxp1;
	struct cvmx_dpi_dmax_dbell_s cn66xx;
	struct cvmx_dpi_dmax_dbell_s cn68xx;
	struct cvmx_dpi_dmax_dbell_s cn68xxp1;
	struct cvmx_dpi_dmax_dbell_s cn70xx;
	struct cvmx_dpi_dmax_dbell_s cn70xxp1;
	struct cvmx_dpi_dmax_dbell_s cn73xx;
	struct cvmx_dpi_dmax_dbell_s cn78xx;
	struct cvmx_dpi_dmax_dbell_s cn78xxp1;
	struct cvmx_dpi_dmax_dbell_s cnf71xx;
	struct cvmx_dpi_dmax_dbell_s cnf75xx;
};

typedef union cvmx_dpi_dmax_dbell cvmx_dpi_dmax_dbell_t;

/**
 * cvmx_dpi_dma#_err_rsp_status
 */
union cvmx_dpi_dmax_err_rsp_status {
	u64 u64;
	struct cvmx_dpi_dmax_err_rsp_status_s {
		u64 reserved_6_63 : 58;
		u64 status : 6;
	} s;
	struct cvmx_dpi_dmax_err_rsp_status_s cn61xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn66xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn68xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn68xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cn70xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn70xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cn73xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn78xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn78xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cnf71xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cnf75xx;
};

typedef union cvmx_dpi_dmax_err_rsp_status cvmx_dpi_dmax_err_rsp_status_t;

/**
 * cvmx_dpi_dma#_ibuff_saddr
 *
 * These registers provide the address to start reading instructions for the eight DMA
 * instruction queues.
 */
union cvmx_dpi_dmax_ibuff_saddr {
	u64 u64;
	struct cvmx_dpi_dmax_ibuff_saddr_s {
		u64 reserved_62_63 : 2;
		u64 csize : 14;
		u64 reserved_0_47 : 48;
	} s;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx {
		u64 reserved_62_63 : 2;
		u64 csize : 14;
		u64 reserved_41_47 : 7;
		u64 idle : 1;
		u64 reserved_36_39 : 4;
		u64 saddr : 29;
		u64 reserved_0_6 : 7;
	} cn61xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn63xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn63xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn66xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn68xx {
		u64 reserved_62_63 : 2;
		u64 csize : 14;
		u64 reserved_41_47 : 7;
		u64 idle : 1;
		u64 saddr : 33;
		u64 reserved_0_6 : 7;
	} cn68xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn68xx cn68xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn70xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn70xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx {
		u64 idle : 1;
		u64 reserved_62_62 : 1;
		u64 csize : 14;
		u64 reserved_42_47 : 6;
		u64 saddr : 35;
		u64 reserved_0_6 : 7;
	} cn73xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cn78xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cn78xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cnf71xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cnf75xx;
};

typedef union cvmx_dpi_dmax_ibuff_saddr cvmx_dpi_dmax_ibuff_saddr_t;

/**
 * cvmx_dpi_dma#_iflight
 */
union cvmx_dpi_dmax_iflight {
	u64 u64;
	struct cvmx_dpi_dmax_iflight_s {
		u64 reserved_3_63 : 61;
		u64 cnt : 3;
	} s;
	struct cvmx_dpi_dmax_iflight_s cn61xx;
	struct cvmx_dpi_dmax_iflight_s cn66xx;
	struct cvmx_dpi_dmax_iflight_s cn68xx;
	struct cvmx_dpi_dmax_iflight_s cn68xxp1;
	struct cvmx_dpi_dmax_iflight_s cn70xx;
	struct cvmx_dpi_dmax_iflight_s cn70xxp1;
	struct cvmx_dpi_dmax_iflight_s cn73xx;
	struct cvmx_dpi_dmax_iflight_s cn78xx;
	struct cvmx_dpi_dmax_iflight_s cn78xxp1;
	struct cvmx_dpi_dmax_iflight_s cnf71xx;
	struct cvmx_dpi_dmax_iflight_s cnf75xx;
};

typedef union cvmx_dpi_dmax_iflight cvmx_dpi_dmax_iflight_t;

/**
 * cvmx_dpi_dma#_naddr
 *
 * These registers provide the L2C addresses to read the next Ichunk data.
 *
 */
union cvmx_dpi_dmax_naddr {
	u64 u64;
	struct cvmx_dpi_dmax_naddr_s {
		u64 reserved_42_63 : 22;
		u64 addr : 42;
	} s;
	struct cvmx_dpi_dmax_naddr_cn61xx {
		u64 reserved_36_63 : 28;
		u64 addr : 36;
	} cn61xx;
	struct cvmx_dpi_dmax_naddr_cn61xx cn63xx;
	struct cvmx_dpi_dmax_naddr_cn61xx cn63xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx cn66xx;
	struct cvmx_dpi_dmax_naddr_cn68xx {
		u64 reserved_40_63 : 24;
		u64 addr : 40;
	} cn68xx;
	struct cvmx_dpi_dmax_naddr_cn68xx cn68xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx cn70xx;
	struct cvmx_dpi_dmax_naddr_cn61xx cn70xxp1;
	struct cvmx_dpi_dmax_naddr_s cn73xx;
	struct cvmx_dpi_dmax_naddr_s cn78xx;
	struct cvmx_dpi_dmax_naddr_s cn78xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx cnf71xx;
	struct cvmx_dpi_dmax_naddr_s cnf75xx;
};

typedef union cvmx_dpi_dmax_naddr cvmx_dpi_dmax_naddr_t;

/**
 * cvmx_dpi_dma#_reqbnk0
 *
 * These registers provide the current contents of the request state machine, bank 0.
 *
 */
union cvmx_dpi_dmax_reqbnk0 {
	u64 u64;
	struct cvmx_dpi_dmax_reqbnk0_s {
		u64 state : 64;
	} s;
	struct cvmx_dpi_dmax_reqbnk0_s cn61xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn63xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn63xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s cn66xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn68xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn68xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s cn70xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn70xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s cn73xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn78xx;
	struct cvmx_dpi_dmax_reqbnk0_s cn78xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s cnf71xx;
	struct cvmx_dpi_dmax_reqbnk0_s cnf75xx;
};

typedef union cvmx_dpi_dmax_reqbnk0 cvmx_dpi_dmax_reqbnk0_t;

/**
 * cvmx_dpi_dma#_reqbnk1
 *
 * These registers provide the current contents of the request state machine, bank 1.
 *
 */
union cvmx_dpi_dmax_reqbnk1 {
	u64 u64;
	struct cvmx_dpi_dmax_reqbnk1_s {
		u64 state : 64;
	} s;
	struct cvmx_dpi_dmax_reqbnk1_s cn61xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn63xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn63xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s cn66xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn68xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn68xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s cn70xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn70xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s cn73xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn78xx;
	struct cvmx_dpi_dmax_reqbnk1_s cn78xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s cnf71xx;
	struct cvmx_dpi_dmax_reqbnk1_s cnf75xx;
};

typedef union cvmx_dpi_dmax_reqbnk1 cvmx_dpi_dmax_reqbnk1_t;

/**
 * cvmx_dpi_dma#_reqq_ctl
 *
 * This register contains the control bits for transactions on the eight request queues.
 *
 */
union cvmx_dpi_dmax_reqq_ctl {
	u64 u64;
	struct cvmx_dpi_dmax_reqq_ctl_s {
		u64 reserved_9_63 : 55;
		u64 st_cmd : 1;
		u64 reserved_2_7 : 6;
		u64 ld_cmd : 2;
	} s;
	struct cvmx_dpi_dmax_reqq_ctl_s cn73xx;
	struct cvmx_dpi_dmax_reqq_ctl_s cn78xx;
	struct cvmx_dpi_dmax_reqq_ctl_s cn78xxp1;
	struct cvmx_dpi_dmax_reqq_ctl_s cnf75xx;
};

typedef union cvmx_dpi_dmax_reqq_ctl cvmx_dpi_dmax_reqq_ctl_t;

/**
 * cvmx_dpi_dma_control
 *
 * This register controls the operation of DMA input and output.
 *
 */
union cvmx_dpi_dma_control {
	u64 u64;
	struct cvmx_dpi_dma_control_s {
		u64 reserved_62_63 : 2;
		u64 dici_mode : 1;
		u64 pkt_en1 : 1;
		u64 ffp_dis : 1;
		u64 commit_mode : 1;
		u64 pkt_hp : 1;
		u64 pkt_en : 1;
		u64 reserved_54_55 : 2;
		u64 dma_enb : 6;
		u64 wqecsdis : 1;
		u64 wqecsoff : 7;
		u64 zbwcsen : 1;
		u64 wqecsmode : 2;
		u64 reserved_35_36 : 2;
		u64 ncb_tag : 1;
		u64 b0_lend : 1;
		u64 reserved_20_32 : 13;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 reserved_0_13 : 14;
	} s;
	struct cvmx_dpi_dma_control_cn61xx {
		u64 reserved_62_63 : 2;
		u64 dici_mode : 1;
		u64 pkt_en1 : 1;
		u64 ffp_dis : 1;
		u64 commit_mode : 1;
		u64 pkt_hp : 1;
		u64 pkt_en : 1;
		u64 reserved_54_55 : 2;
		u64 dma_enb : 6;
		u64 reserved_34_47 : 14;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 reserved_0_13 : 14;
	} cn61xx;
	struct cvmx_dpi_dma_control_cn63xx {
		u64 reserved_61_63 : 3;
		u64 pkt_en1 : 1;
		u64 ffp_dis : 1;
		u64 commit_mode : 1;
		u64 pkt_hp : 1;
		u64 pkt_en : 1;
		u64 reserved_54_55 : 2;
		u64 dma_enb : 6;
		u64 reserved_34_47 : 14;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 reserved_0_13 : 14;
	} cn63xx;
	struct cvmx_dpi_dma_control_cn63xxp1 {
		u64 reserved_59_63 : 5;
		u64 commit_mode : 1;
		u64 pkt_hp : 1;
		u64 pkt_en : 1;
		u64 reserved_54_55 : 2;
		u64 dma_enb : 6;
		u64 reserved_34_47 : 14;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 reserved_0_13 : 14;
	} cn63xxp1;
	struct cvmx_dpi_dma_control_cn63xx cn66xx;
	struct cvmx_dpi_dma_control_cn61xx cn68xx;
	struct cvmx_dpi_dma_control_cn63xx cn68xxp1;
	struct cvmx_dpi_dma_control_cn61xx cn70xx;
	struct cvmx_dpi_dma_control_cn61xx cn70xxp1;
	struct cvmx_dpi_dma_control_cn73xx {
		u64 reserved_60_63 : 4;
		u64 ffp_dis : 1;
		u64 commit_mode : 1;
		u64 reserved_57_57 : 1;
		u64 pkt_en : 1;
		u64 reserved_54_55 : 2;
		u64 dma_enb : 6;
		u64 wqecsdis : 1;
		u64 wqecsoff : 7;
		u64 zbwcsen : 1;
		u64 wqecsmode : 2;
		u64 reserved_35_36 : 2;
		u64 ncb_tag : 1;
		u64 b0_lend : 1;
		u64 ldwb : 1;
		u64 aura_ichk : 12;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 reserved_0_13 : 14;
	} cn73xx;
	struct cvmx_dpi_dma_control_cn73xx cn78xx;
	struct cvmx_dpi_dma_control_cn73xx cn78xxp1;
	struct cvmx_dpi_dma_control_cn61xx cnf71xx;
	struct cvmx_dpi_dma_control_cn73xx cnf75xx;
};

typedef union cvmx_dpi_dma_control cvmx_dpi_dma_control_t;

/**
 * cvmx_dpi_dma_eng#_en
 *
 * These registers provide control for the DMA engines.
 *
 */
union cvmx_dpi_dma_engx_en {
	u64 u64;
	struct cvmx_dpi_dma_engx_en_s {
		u64 reserved_39_63 : 25;
		u64 eng_molr : 7;
		u64 reserved_8_31 : 24;
		u64 qen : 8;
	} s;
	struct cvmx_dpi_dma_engx_en_cn61xx {
		u64 reserved_8_63 : 56;
		u64 qen : 8;
	} cn61xx;
	struct cvmx_dpi_dma_engx_en_cn61xx cn63xx;
	struct cvmx_dpi_dma_engx_en_cn61xx cn63xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx cn66xx;
	struct cvmx_dpi_dma_engx_en_cn61xx cn68xx;
	struct cvmx_dpi_dma_engx_en_cn61xx cn68xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx cn70xx;
	struct cvmx_dpi_dma_engx_en_cn61xx cn70xxp1;
	struct cvmx_dpi_dma_engx_en_s cn73xx;
	struct cvmx_dpi_dma_engx_en_s cn78xx;
	struct cvmx_dpi_dma_engx_en_s cn78xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx cnf71xx;
	struct cvmx_dpi_dma_engx_en_s cnf75xx;
};

typedef union cvmx_dpi_dma_engx_en cvmx_dpi_dma_engx_en_t;

/**
 * cvmx_dpi_dma_pp#_cnt
 *
 * DPI_DMA_PP[0..3]_CNT  = DMA per PP Instr Done Counter
 * When DMA Instruction Completion Interrupt Mode DPI_DMA_CONTROL.DICI_MODE is enabled, every dma
 * instruction
 * that has the WQP=0 and a PTR value of 1..4 will incremrement DPI_DMA_PPx_CNT value-1 counter.
 * Instructions with WQP=0 and PTR values higher then 0x3F will still send a zero byte write.
 * Hardware reserves that values 5..63 for future use and will treat them as a PTR of 0 and do
 * nothing.
 */
union cvmx_dpi_dma_ppx_cnt {
	u64 u64;
	struct cvmx_dpi_dma_ppx_cnt_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_dpi_dma_ppx_cnt_s cn61xx;
	struct cvmx_dpi_dma_ppx_cnt_s cn68xx;
	struct cvmx_dpi_dma_ppx_cnt_s cn70xx;
	struct cvmx_dpi_dma_ppx_cnt_s cn70xxp1;
	struct cvmx_dpi_dma_ppx_cnt_s cn73xx;
	struct cvmx_dpi_dma_ppx_cnt_s cn78xx;
	struct cvmx_dpi_dma_ppx_cnt_s cn78xxp1;
	struct cvmx_dpi_dma_ppx_cnt_s cnf71xx;
	struct cvmx_dpi_dma_ppx_cnt_s cnf75xx;
};

typedef union cvmx_dpi_dma_ppx_cnt cvmx_dpi_dma_ppx_cnt_t;

/**
 * cvmx_dpi_dma_pp_int
 */
union cvmx_dpi_dma_pp_int {
	u64 u64;
	struct cvmx_dpi_dma_pp_int_s {
		u64 reserved_48_63 : 16;
		u64 complete : 48;
	} s;
	struct cvmx_dpi_dma_pp_int_cn73xx {
		u64 reserved_16_63 : 48;
		u64 complete : 16;
	} cn73xx;
	struct cvmx_dpi_dma_pp_int_s cn78xx;
	struct cvmx_dpi_dma_pp_int_s cn78xxp1;
	struct cvmx_dpi_dma_pp_int_cn73xx cnf75xx;
};

typedef union cvmx_dpi_dma_pp_int cvmx_dpi_dma_pp_int_t;

/**
 * cvmx_dpi_ecc_ctl
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_dpi_ecc_ctl {
	u64 u64;
	struct cvmx_dpi_ecc_ctl_s {
		u64 reserved_33_63 : 31;
		u64 ram_cdis : 1;
		u64 reserved_17_31 : 15;
		u64 ram_flip1 : 1;
		u64 reserved_1_15 : 15;
		u64 ram_flip0 : 1;
	} s;
	struct cvmx_dpi_ecc_ctl_s cn73xx;
	struct cvmx_dpi_ecc_ctl_s cn78xx;
	struct cvmx_dpi_ecc_ctl_s cn78xxp1;
	struct cvmx_dpi_ecc_ctl_s cnf75xx;
};

typedef union cvmx_dpi_ecc_ctl cvmx_dpi_ecc_ctl_t;

/**
 * cvmx_dpi_ecc_int
 *
 * This register contains ECC error interrupt summary bits.
 *
 */
union cvmx_dpi_ecc_int {
	u64 u64;
	struct cvmx_dpi_ecc_int_s {
		u64 reserved_47_63 : 17;
		u64 ram_sbe : 15;
		u64 reserved_15_31 : 17;
		u64 ram_dbe : 15;
	} s;
	struct cvmx_dpi_ecc_int_s cn73xx;
	struct cvmx_dpi_ecc_int_s cn78xx;
	struct cvmx_dpi_ecc_int_s cn78xxp1;
	struct cvmx_dpi_ecc_int_s cnf75xx;
};

typedef union cvmx_dpi_ecc_int cvmx_dpi_ecc_int_t;

/**
 * cvmx_dpi_eng#_buf
 *
 * Notes:
 * The total amount of storage allocated to the 6 DPI DMA engines (via DPI_ENG*_BUF[BLKS]) must not exceed 8KB.
 *
 */
union cvmx_dpi_engx_buf {
	u64 u64;
	struct cvmx_dpi_engx_buf_s {
		u64 reserved_38_63 : 26;
		u64 compblks : 6;
		u64 reserved_10_31 : 22;
		u64 base : 6;
		u64 blks : 4;
	} s;
	struct cvmx_dpi_engx_buf_cn61xx {
		u64 reserved_37_63 : 27;
		u64 compblks : 5;
		u64 reserved_9_31 : 23;
		u64 base : 5;
		u64 blks : 4;
	} cn61xx;
	struct cvmx_dpi_engx_buf_cn63xx {
		u64 reserved_8_63 : 56;
		u64 base : 4;
		u64 blks : 4;
	} cn63xx;
	struct cvmx_dpi_engx_buf_cn63xx cn63xxp1;
	struct cvmx_dpi_engx_buf_cn61xx cn66xx;
	struct cvmx_dpi_engx_buf_cn61xx cn68xx;
	struct cvmx_dpi_engx_buf_cn61xx cn68xxp1;
	struct cvmx_dpi_engx_buf_cn61xx cn70xx;
	struct cvmx_dpi_engx_buf_cn61xx cn70xxp1;
	struct cvmx_dpi_engx_buf_s cn73xx;
	struct cvmx_dpi_engx_buf_s cn78xx;
	struct cvmx_dpi_engx_buf_s cn78xxp1;
	struct cvmx_dpi_engx_buf_cn61xx cnf71xx;
	struct cvmx_dpi_engx_buf_s cnf75xx;
};

typedef union cvmx_dpi_engx_buf cvmx_dpi_engx_buf_t;

/**
 * cvmx_dpi_info_reg
 */
union cvmx_dpi_info_reg {
	u64 u64;
	struct cvmx_dpi_info_reg_s {
		u64 reserved_8_63 : 56;
		u64 ffp : 4;
		u64 reserved_2_3 : 2;
		u64 ncb : 1;
		u64 rsl : 1;
	} s;
	struct cvmx_dpi_info_reg_s cn61xx;
	struct cvmx_dpi_info_reg_s cn63xx;
	struct cvmx_dpi_info_reg_cn63xxp1 {
		u64 reserved_2_63 : 62;
		u64 ncb : 1;
		u64 rsl : 1;
	} cn63xxp1;
	struct cvmx_dpi_info_reg_s cn66xx;
	struct cvmx_dpi_info_reg_s cn68xx;
	struct cvmx_dpi_info_reg_s cn68xxp1;
	struct cvmx_dpi_info_reg_s cn70xx;
	struct cvmx_dpi_info_reg_s cn70xxp1;
	struct cvmx_dpi_info_reg_s cn73xx;
	struct cvmx_dpi_info_reg_s cn78xx;
	struct cvmx_dpi_info_reg_s cn78xxp1;
	struct cvmx_dpi_info_reg_s cnf71xx;
	struct cvmx_dpi_info_reg_s cnf75xx;
};

typedef union cvmx_dpi_info_reg cvmx_dpi_info_reg_t;

/**
 * cvmx_dpi_int_en
 */
union cvmx_dpi_int_en {
	u64 u64;
	struct cvmx_dpi_int_en_s {
		u64 reserved_28_63 : 36;
		u64 sprt3_rst : 1;
		u64 sprt2_rst : 1;
		u64 sprt1_rst : 1;
		u64 sprt0_rst : 1;
		u64 reserved_23_23 : 1;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_2_7 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} s;
	struct cvmx_dpi_int_en_s cn61xx;
	struct cvmx_dpi_int_en_cn63xx {
		u64 reserved_26_63 : 38;
		u64 sprt1_rst : 1;
		u64 sprt0_rst : 1;
		u64 reserved_23_23 : 1;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_2_7 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} cn63xx;
	struct cvmx_dpi_int_en_cn63xx cn63xxp1;
	struct cvmx_dpi_int_en_s cn66xx;
	struct cvmx_dpi_int_en_cn63xx cn68xx;
	struct cvmx_dpi_int_en_cn63xx cn68xxp1;
	struct cvmx_dpi_int_en_cn70xx {
		u64 reserved_28_63 : 36;
		u64 sprt3_rst : 1;
		u64 sprt2_rst : 1;
		u64 sprt1_rst : 1;
		u64 sprt0_rst : 1;
		u64 reserved_23_23 : 1;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_7_2 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} cn70xx;
	struct cvmx_dpi_int_en_cn70xx cn70xxp1;
	struct cvmx_dpi_int_en_s cnf71xx;
};

typedef union cvmx_dpi_int_en cvmx_dpi_int_en_t;

/**
 * cvmx_dpi_int_reg
 *
 * This register contains error flags for DPI.
 *
 */
union cvmx_dpi_int_reg {
	u64 u64;
	struct cvmx_dpi_int_reg_s {
		u64 reserved_28_63 : 36;
		u64 sprt3_rst : 1;
		u64 sprt2_rst : 1;
		u64 sprt1_rst : 1;
		u64 sprt0_rst : 1;
		u64 reserved_23_23 : 1;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_2_7 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} s;
	struct cvmx_dpi_int_reg_s cn61xx;
	struct cvmx_dpi_int_reg_cn63xx {
		u64 reserved_26_63 : 38;
		u64 sprt1_rst : 1;
		u64 sprt0_rst : 1;
		u64 reserved_23_23 : 1;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_2_7 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} cn63xx;
	struct cvmx_dpi_int_reg_cn63xx cn63xxp1;
	struct cvmx_dpi_int_reg_s cn66xx;
	struct cvmx_dpi_int_reg_cn63xx cn68xx;
	struct cvmx_dpi_int_reg_cn63xx cn68xxp1;
	struct cvmx_dpi_int_reg_s cn70xx;
	struct cvmx_dpi_int_reg_s cn70xxp1;
	struct cvmx_dpi_int_reg_cn73xx {
		u64 reserved_23_63 : 41;
		u64 req_badfil : 1;
		u64 req_inull : 1;
		u64 req_anull : 1;
		u64 req_undflw : 1;
		u64 req_ovrflw : 1;
		u64 req_badlen : 1;
		u64 req_badadr : 1;
		u64 dmadbo : 8;
		u64 reserved_2_7 : 6;
		u64 nfovr : 1;
		u64 nderr : 1;
	} cn73xx;
	struct cvmx_dpi_int_reg_cn73xx cn78xx;
	struct cvmx_dpi_int_reg_s cn78xxp1;
	struct cvmx_dpi_int_reg_s cnf71xx;
	struct cvmx_dpi_int_reg_cn73xx cnf75xx;
};

typedef union cvmx_dpi_int_reg cvmx_dpi_int_reg_t;

/**
 * cvmx_dpi_ncb#_cfg
 */
union cvmx_dpi_ncbx_cfg {
	u64 u64;
	struct cvmx_dpi_ncbx_cfg_s {
		u64 reserved_6_63 : 58;
		u64 molr : 6;
	} s;
	struct cvmx_dpi_ncbx_cfg_s cn61xx;
	struct cvmx_dpi_ncbx_cfg_s cn66xx;
	struct cvmx_dpi_ncbx_cfg_s cn68xx;
	struct cvmx_dpi_ncbx_cfg_s cn70xx;
	struct cvmx_dpi_ncbx_cfg_s cn70xxp1;
	struct cvmx_dpi_ncbx_cfg_s cn73xx;
	struct cvmx_dpi_ncbx_cfg_s cn78xx;
	struct cvmx_dpi_ncbx_cfg_s cn78xxp1;
	struct cvmx_dpi_ncbx_cfg_s cnf71xx;
	struct cvmx_dpi_ncbx_cfg_s cnf75xx;
};

typedef union cvmx_dpi_ncbx_cfg cvmx_dpi_ncbx_cfg_t;

/**
 * cvmx_dpi_ncb_ctl
 *
 * This register chooses which NCB interface DPI uses for L2/DRAM reads/writes.
 *
 */
union cvmx_dpi_ncb_ctl {
	u64 u64;
	struct cvmx_dpi_ncb_ctl_s {
		u64 reserved_25_63 : 39;
		u64 ncbsel_prt_xor_dis : 1;
		u64 reserved_21_23 : 3;
		u64 ncbsel_zbw : 1;
		u64 reserved_17_19 : 3;
		u64 ncbsel_req : 1;
		u64 reserved_13_15 : 3;
		u64 ncbsel_dst : 1;
		u64 reserved_9_11 : 3;
		u64 ncbsel_src : 1;
		u64 reserved_1_7 : 7;
		u64 prt : 1;
	} s;
	struct cvmx_dpi_ncb_ctl_cn73xx {
		u64 reserved_25_63 : 39;
		u64 ncbsel_prt_xor_dis : 1;
		u64 reserved_21_23 : 3;
		u64 ncbsel_zbw : 1;
		u64 reserved_17_19 : 3;
		u64 ncbsel_req : 1;
		u64 reserved_13_15 : 3;
		u64 ncbsel_dst : 1;
		u64 reserved_9_11 : 3;
		u64 ncbsel_src : 1;
		u64 reserved_0_7 : 8;
	} cn73xx;
	struct cvmx_dpi_ncb_ctl_s cn78xx;
	struct cvmx_dpi_ncb_ctl_s cn78xxp1;
	struct cvmx_dpi_ncb_ctl_cn73xx cnf75xx;
};

typedef union cvmx_dpi_ncb_ctl cvmx_dpi_ncb_ctl_t;

/**
 * cvmx_dpi_pint_info
 *
 * This register provides DPI packet interrupt information.
 *
 */
union cvmx_dpi_pint_info {
	u64 u64;
	struct cvmx_dpi_pint_info_s {
		u64 reserved_14_63 : 50;
		u64 iinfo : 6;
		u64 reserved_6_7 : 2;
		u64 sinfo : 6;
	} s;
	struct cvmx_dpi_pint_info_s cn61xx;
	struct cvmx_dpi_pint_info_s cn63xx;
	struct cvmx_dpi_pint_info_s cn63xxp1;
	struct cvmx_dpi_pint_info_s cn66xx;
	struct cvmx_dpi_pint_info_s cn68xx;
	struct cvmx_dpi_pint_info_s cn68xxp1;
	struct cvmx_dpi_pint_info_s cn70xx;
	struct cvmx_dpi_pint_info_s cn70xxp1;
	struct cvmx_dpi_pint_info_s cn73xx;
	struct cvmx_dpi_pint_info_s cn78xx;
	struct cvmx_dpi_pint_info_s cn78xxp1;
	struct cvmx_dpi_pint_info_s cnf71xx;
	struct cvmx_dpi_pint_info_s cnf75xx;
};

typedef union cvmx_dpi_pint_info cvmx_dpi_pint_info_t;

/**
 * cvmx_dpi_pkt_err_rsp
 */
union cvmx_dpi_pkt_err_rsp {
	u64 u64;
	struct cvmx_dpi_pkt_err_rsp_s {
		u64 reserved_1_63 : 63;
		u64 pkterr : 1;
	} s;
	struct cvmx_dpi_pkt_err_rsp_s cn61xx;
	struct cvmx_dpi_pkt_err_rsp_s cn63xx;
	struct cvmx_dpi_pkt_err_rsp_s cn63xxp1;
	struct cvmx_dpi_pkt_err_rsp_s cn66xx;
	struct cvmx_dpi_pkt_err_rsp_s cn68xx;
	struct cvmx_dpi_pkt_err_rsp_s cn68xxp1;
	struct cvmx_dpi_pkt_err_rsp_s cn70xx;
	struct cvmx_dpi_pkt_err_rsp_s cn70xxp1;
	struct cvmx_dpi_pkt_err_rsp_s cn73xx;
	struct cvmx_dpi_pkt_err_rsp_s cn78xx;
	struct cvmx_dpi_pkt_err_rsp_s cn78xxp1;
	struct cvmx_dpi_pkt_err_rsp_s cnf71xx;
	struct cvmx_dpi_pkt_err_rsp_s cnf75xx;
};

typedef union cvmx_dpi_pkt_err_rsp cvmx_dpi_pkt_err_rsp_t;

/**
 * cvmx_dpi_req_err_rsp
 */
union cvmx_dpi_req_err_rsp {
	u64 u64;
	struct cvmx_dpi_req_err_rsp_s {
		u64 reserved_8_63 : 56;
		u64 qerr : 8;
	} s;
	struct cvmx_dpi_req_err_rsp_s cn61xx;
	struct cvmx_dpi_req_err_rsp_s cn63xx;
	struct cvmx_dpi_req_err_rsp_s cn63xxp1;
	struct cvmx_dpi_req_err_rsp_s cn66xx;
	struct cvmx_dpi_req_err_rsp_s cn68xx;
	struct cvmx_dpi_req_err_rsp_s cn68xxp1;
	struct cvmx_dpi_req_err_rsp_s cn70xx;
	struct cvmx_dpi_req_err_rsp_s cn70xxp1;
	struct cvmx_dpi_req_err_rsp_s cn73xx;
	struct cvmx_dpi_req_err_rsp_s cn78xx;
	struct cvmx_dpi_req_err_rsp_s cn78xxp1;
	struct cvmx_dpi_req_err_rsp_s cnf71xx;
	struct cvmx_dpi_req_err_rsp_s cnf75xx;
};

typedef union cvmx_dpi_req_err_rsp cvmx_dpi_req_err_rsp_t;

/**
 * cvmx_dpi_req_err_rsp_en
 */
union cvmx_dpi_req_err_rsp_en {
	u64 u64;
	struct cvmx_dpi_req_err_rsp_en_s {
		u64 reserved_8_63 : 56;
		u64 en : 8;
	} s;
	struct cvmx_dpi_req_err_rsp_en_s cn61xx;
	struct cvmx_dpi_req_err_rsp_en_s cn63xx;
	struct cvmx_dpi_req_err_rsp_en_s cn63xxp1;
	struct cvmx_dpi_req_err_rsp_en_s cn66xx;
	struct cvmx_dpi_req_err_rsp_en_s cn68xx;
	struct cvmx_dpi_req_err_rsp_en_s cn68xxp1;
	struct cvmx_dpi_req_err_rsp_en_s cn70xx;
	struct cvmx_dpi_req_err_rsp_en_s cn70xxp1;
	struct cvmx_dpi_req_err_rsp_en_s cn73xx;
	struct cvmx_dpi_req_err_rsp_en_s cn78xx;
	struct cvmx_dpi_req_err_rsp_en_s cn78xxp1;
	struct cvmx_dpi_req_err_rsp_en_s cnf71xx;
	struct cvmx_dpi_req_err_rsp_en_s cnf75xx;
};

typedef union cvmx_dpi_req_err_rsp_en cvmx_dpi_req_err_rsp_en_t;

/**
 * cvmx_dpi_req_err_rst
 */
union cvmx_dpi_req_err_rst {
	u64 u64;
	struct cvmx_dpi_req_err_rst_s {
		u64 reserved_8_63 : 56;
		u64 qerr : 8;
	} s;
	struct cvmx_dpi_req_err_rst_s cn61xx;
	struct cvmx_dpi_req_err_rst_s cn63xx;
	struct cvmx_dpi_req_err_rst_s cn63xxp1;
	struct cvmx_dpi_req_err_rst_s cn66xx;
	struct cvmx_dpi_req_err_rst_s cn68xx;
	struct cvmx_dpi_req_err_rst_s cn68xxp1;
	struct cvmx_dpi_req_err_rst_s cn70xx;
	struct cvmx_dpi_req_err_rst_s cn70xxp1;
	struct cvmx_dpi_req_err_rst_s cn73xx;
	struct cvmx_dpi_req_err_rst_s cn78xx;
	struct cvmx_dpi_req_err_rst_s cn78xxp1;
	struct cvmx_dpi_req_err_rst_s cnf71xx;
	struct cvmx_dpi_req_err_rst_s cnf75xx;
};

typedef union cvmx_dpi_req_err_rst cvmx_dpi_req_err_rst_t;

/**
 * cvmx_dpi_req_err_rst_en
 */
union cvmx_dpi_req_err_rst_en {
	u64 u64;
	struct cvmx_dpi_req_err_rst_en_s {
		u64 reserved_8_63 : 56;
		u64 en : 8;
	} s;
	struct cvmx_dpi_req_err_rst_en_s cn61xx;
	struct cvmx_dpi_req_err_rst_en_s cn63xx;
	struct cvmx_dpi_req_err_rst_en_s cn63xxp1;
	struct cvmx_dpi_req_err_rst_en_s cn66xx;
	struct cvmx_dpi_req_err_rst_en_s cn68xx;
	struct cvmx_dpi_req_err_rst_en_s cn68xxp1;
	struct cvmx_dpi_req_err_rst_en_s cn70xx;
	struct cvmx_dpi_req_err_rst_en_s cn70xxp1;
	struct cvmx_dpi_req_err_rst_en_s cn73xx;
	struct cvmx_dpi_req_err_rst_en_s cn78xx;
	struct cvmx_dpi_req_err_rst_en_s cn78xxp1;
	struct cvmx_dpi_req_err_rst_en_s cnf71xx;
	struct cvmx_dpi_req_err_rst_en_s cnf75xx;
};

typedef union cvmx_dpi_req_err_rst_en cvmx_dpi_req_err_rst_en_t;

/**
 * cvmx_dpi_req_err_skip_comp
 */
union cvmx_dpi_req_err_skip_comp {
	u64 u64;
	struct cvmx_dpi_req_err_skip_comp_s {
		u64 reserved_24_63 : 40;
		u64 en_rst : 8;
		u64 reserved_8_15 : 8;
		u64 en_rsp : 8;
	} s;
	struct cvmx_dpi_req_err_skip_comp_s cn61xx;
	struct cvmx_dpi_req_err_skip_comp_s cn66xx;
	struct cvmx_dpi_req_err_skip_comp_s cn68xx;
	struct cvmx_dpi_req_err_skip_comp_s cn68xxp1;
	struct cvmx_dpi_req_err_skip_comp_s cn70xx;
	struct cvmx_dpi_req_err_skip_comp_s cn70xxp1;
	struct cvmx_dpi_req_err_skip_comp_s cn73xx;
	struct cvmx_dpi_req_err_skip_comp_s cn78xx;
	struct cvmx_dpi_req_err_skip_comp_s cn78xxp1;
	struct cvmx_dpi_req_err_skip_comp_s cnf71xx;
	struct cvmx_dpi_req_err_skip_comp_s cnf75xx;
};

typedef union cvmx_dpi_req_err_skip_comp cvmx_dpi_req_err_skip_comp_t;

/**
 * cvmx_dpi_req_gbl_en
 */
union cvmx_dpi_req_gbl_en {
	u64 u64;
	struct cvmx_dpi_req_gbl_en_s {
		u64 reserved_8_63 : 56;
		u64 qen : 8;
	} s;
	struct cvmx_dpi_req_gbl_en_s cn61xx;
	struct cvmx_dpi_req_gbl_en_s cn63xx;
	struct cvmx_dpi_req_gbl_en_s cn63xxp1;
	struct cvmx_dpi_req_gbl_en_s cn66xx;
	struct cvmx_dpi_req_gbl_en_s cn68xx;
	struct cvmx_dpi_req_gbl_en_s cn68xxp1;
	struct cvmx_dpi_req_gbl_en_s cn70xx;
	struct cvmx_dpi_req_gbl_en_s cn70xxp1;
	struct cvmx_dpi_req_gbl_en_s cn73xx;
	struct cvmx_dpi_req_gbl_en_s cn78xx;
	struct cvmx_dpi_req_gbl_en_s cn78xxp1;
	struct cvmx_dpi_req_gbl_en_s cnf71xx;
	struct cvmx_dpi_req_gbl_en_s cnf75xx;
};

typedef union cvmx_dpi_req_gbl_en cvmx_dpi_req_gbl_en_t;

/**
 * cvmx_dpi_sli_prt#_cfg
 *
 * This register configures the max read request size, max payload size, and max number of SLI
 * tags in use. Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_cfg {
	u64 u64;
	struct cvmx_dpi_sli_prtx_cfg_s {
		u64 reserved_29_63 : 35;
		u64 ncbsel : 1;
		u64 reserved_25_27 : 3;
		u64 halt : 1;
		u64 qlm_cfg : 4;
		u64 reserved_17_19 : 3;
		u64 rd_mode : 1;
		u64 reserved_15_15 : 1;
		u64 molr : 7;
		u64 mps_lim : 1;
		u64 reserved_5_6 : 2;
		u64 mps : 1;
		u64 mrrs_lim : 1;
		u64 reserved_2_2 : 1;
		u64 mrrs : 2;
	} s;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx {
		u64 reserved_25_63 : 39;
		u64 halt : 1;
		u64 qlm_cfg : 4;
		u64 reserved_17_19 : 3;
		u64 rd_mode : 1;
		u64 reserved_14_15 : 2;
		u64 molr : 6;
		u64 mps_lim : 1;
		u64 reserved_5_6 : 2;
		u64 mps : 1;
		u64 mrrs_lim : 1;
		u64 reserved_2_2 : 1;
		u64 mrrs : 2;
	} cn61xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx {
		u64 reserved_25_63 : 39;
		u64 halt : 1;
		u64 reserved_21_23 : 3;
		u64 qlm_cfg : 1;
		u64 reserved_17_19 : 3;
		u64 rd_mode : 1;
		u64 reserved_14_15 : 2;
		u64 molr : 6;
		u64 mps_lim : 1;
		u64 reserved_5_6 : 2;
		u64 mps : 1;
		u64 mrrs_lim : 1;
		u64 reserved_2_2 : 1;
		u64 mrrs : 2;
	} cn63xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx cn63xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx cn66xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx cn68xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx cn68xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn70xx {
		u64 reserved_25_63 : 39;
		u64 halt : 1;
		u64 reserved_17_23 : 7;
		u64 rd_mode : 1;
		u64 reserved_14_15 : 2;
		u64 molr : 6;
		u64 mps_lim : 1;
		u64 reserved_5_6 : 2;
		u64 mps : 1;
		u64 mrrs_lim : 1;
		u64 reserved_2_2 : 1;
		u64 mrrs : 2;
	} cn70xx;
	struct cvmx_dpi_sli_prtx_cfg_cn70xx cn70xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx {
		u64 reserved_29_63 : 35;
		u64 ncbsel : 1;
		u64 reserved_25_27 : 3;
		u64 halt : 1;
		u64 reserved_21_23 : 3;
		u64 qlm_cfg : 1;
		u64 reserved_17_19 : 3;
		u64 rd_mode : 1;
		u64 reserved_15_15 : 1;
		u64 molr : 7;
		u64 mps_lim : 1;
		u64 reserved_5_6 : 2;
		u64 mps : 1;
		u64 mrrs_lim : 1;
		u64 reserved_2_2 : 1;
		u64 mrrs : 2;
	} cn73xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx cn78xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx cn78xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx cnf71xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx cnf75xx;
};

typedef union cvmx_dpi_sli_prtx_cfg cvmx_dpi_sli_prtx_cfg_t;

/**
 * cvmx_dpi_sli_prt#_err
 *
 * This register logs the address associated with the reported SLI error response.
 * Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_err {
	u64 u64;
	struct cvmx_dpi_sli_prtx_err_s {
		u64 addr : 61;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_dpi_sli_prtx_err_s cn61xx;
	struct cvmx_dpi_sli_prtx_err_s cn63xx;
	struct cvmx_dpi_sli_prtx_err_s cn63xxp1;
	struct cvmx_dpi_sli_prtx_err_s cn66xx;
	struct cvmx_dpi_sli_prtx_err_s cn68xx;
	struct cvmx_dpi_sli_prtx_err_s cn68xxp1;
	struct cvmx_dpi_sli_prtx_err_s cn70xx;
	struct cvmx_dpi_sli_prtx_err_s cn70xxp1;
	struct cvmx_dpi_sli_prtx_err_s cn73xx;
	struct cvmx_dpi_sli_prtx_err_s cn78xx;
	struct cvmx_dpi_sli_prtx_err_s cn78xxp1;
	struct cvmx_dpi_sli_prtx_err_s cnf71xx;
	struct cvmx_dpi_sli_prtx_err_s cnf75xx;
};

typedef union cvmx_dpi_sli_prtx_err cvmx_dpi_sli_prtx_err_t;

/**
 * cvmx_dpi_sli_prt#_err_info
 *
 * This register logs information associated with the reported SLI error response.
 * Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_err_info {
	u64 u64;
	struct cvmx_dpi_sli_prtx_err_info_s {
		u64 reserved_9_63 : 55;
		u64 lock : 1;
		u64 reserved_5_7 : 3;
		u64 type : 1;
		u64 reserved_3_3 : 1;
		u64 reqq : 3;
	} s;
	struct cvmx_dpi_sli_prtx_err_info_s cn61xx;
	struct cvmx_dpi_sli_prtx_err_info_s cn63xx;
	struct cvmx_dpi_sli_prtx_err_info_s cn63xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s cn66xx;
	struct cvmx_dpi_sli_prtx_err_info_s cn68xx;
	struct cvmx_dpi_sli_prtx_err_info_s cn68xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s cn70xx;
	struct cvmx_dpi_sli_prtx_err_info_s cn70xxp1;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx {
		u64 reserved_32_63 : 32;
		u64 pvf : 16;
		u64 reserved_9_15 : 7;
		u64 lock : 1;
		u64 reserved_5_7 : 3;
		u64 type : 1;
		u64 reserved_3_3 : 1;
		u64 reqq : 3;
	} cn73xx;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx cn78xx;
	struct cvmx_dpi_sli_prtx_err_info_cn78xxp1 {
		u64 reserved_23_63 : 41;
		u64 vf : 7;
		u64 reserved_9_15 : 7;
		u64 lock : 1;
		u64 reserved_5_7 : 3;
		u64 type : 1;
		u64 reserved_3_3 : 1;
		u64 reqq : 3;
	} cn78xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s cnf71xx;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx cnf75xx;
};

typedef union cvmx_dpi_sli_prtx_err_info cvmx_dpi_sli_prtx_err_info_t;

/**
 * cvmx_dpi_srio_rx_bell#
 *
 * Reading this register pops an entry off the corresponding SRIO RX doorbell FIFO.
 * The chip supports 16 FIFOs per SRIO interface for a total of 32 FIFOs/Registers.
 * The MSB of the registers indicates the MAC while the 4 LSBs indicate the FIFO.
 * Information on the doorbell allocation can be found in SRIO()_RX_BELL_CTRL.
 */
union cvmx_dpi_srio_rx_bellx {
	u64 u64;
	struct cvmx_dpi_srio_rx_bellx_s {
		u64 reserved_48_63 : 16;
		u64 data : 16;
		u64 sid : 16;
		u64 count : 8;
		u64 reserved_5_7 : 3;
		u64 dest_id : 1;
		u64 id16 : 1;
		u64 reserved_2_2 : 1;
		u64 dpriority : 2;
	} s;
	struct cvmx_dpi_srio_rx_bellx_s cnf75xx;
};

typedef union cvmx_dpi_srio_rx_bellx cvmx_dpi_srio_rx_bellx_t;

/**
 * cvmx_dpi_srio_rx_bell_seq#
 *
 * This register contains the value of the sequence counter when the doorbell
 * was received and a shadow copy of the Bell FIFO Count that can be read without
 * emptying the FIFO.  This register must be read prior to corresponding
 * DPI_SRIO_RX_BELL register to link the doorbell and sequence number.
 *
 * Information on the Doorbell Allocation can be found in SRIO()_RX_BELL_CTRL.
 */
union cvmx_dpi_srio_rx_bell_seqx {
	u64 u64;
	struct cvmx_dpi_srio_rx_bell_seqx_s {
		u64 reserved_40_63 : 24;
		u64 count : 8;
		u64 sid : 32;
	} s;
	struct cvmx_dpi_srio_rx_bell_seqx_s cnf75xx;
};

typedef union cvmx_dpi_srio_rx_bell_seqx cvmx_dpi_srio_rx_bell_seqx_t;

/**
 * cvmx_dpi_swa_q_vmid
 *
 * Not used.
 *
 */
union cvmx_dpi_swa_q_vmid {
	u64 u64;
	struct cvmx_dpi_swa_q_vmid_s {
		u64 vmid7 : 8;
		u64 vmid6 : 8;
		u64 vmid5 : 8;
		u64 vmid4 : 8;
		u64 vmid3 : 8;
		u64 vmid2 : 8;
		u64 vmid1 : 8;
		u64 vmid0 : 8;
	} s;
	struct cvmx_dpi_swa_q_vmid_s cn73xx;
	struct cvmx_dpi_swa_q_vmid_s cn78xx;
	struct cvmx_dpi_swa_q_vmid_s cn78xxp1;
	struct cvmx_dpi_swa_q_vmid_s cnf75xx;
};

typedef union cvmx_dpi_swa_q_vmid cvmx_dpi_swa_q_vmid_t;

#endif
