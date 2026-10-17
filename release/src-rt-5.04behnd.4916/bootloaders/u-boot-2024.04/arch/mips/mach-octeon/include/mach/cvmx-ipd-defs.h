/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ipd.
 */

#ifndef __CVMX_IPD_DEFS_H__
#define __CVMX_IPD_DEFS_H__

#define CVMX_IPD_1ST_MBUFF_SKIP		    (0x00014F0000000000ull)
#define CVMX_IPD_1st_NEXT_PTR_BACK	    (0x00014F0000000150ull)
#define CVMX_IPD_2nd_NEXT_PTR_BACK	    (0x00014F0000000158ull)
#define CVMX_IPD_BIST_STATUS		    (0x00014F00000007F8ull)
#define CVMX_IPD_BPIDX_MBUF_TH(offset)	    (0x00014F0000002000ull + ((offset) & 63) * 8)
#define CVMX_IPD_BPID_BP_COUNTERX(offset)   (0x00014F0000003000ull + ((offset) & 63) * 8)
#define CVMX_IPD_BP_PRT_RED_END		    (0x00014F0000000328ull)
#define CVMX_IPD_CLK_COUNT		    (0x00014F0000000338ull)
#define CVMX_IPD_CREDITS		    (0x00014F0000004410ull)
#define CVMX_IPD_CTL_STATUS		    (0x00014F0000000018ull)
#define CVMX_IPD_ECC_CTL		    (0x00014F0000004408ull)
#define CVMX_IPD_FREE_PTR_FIFO_CTL	    (0x00014F0000000780ull)
#define CVMX_IPD_FREE_PTR_VALUE		    (0x00014F0000000788ull)
#define CVMX_IPD_HOLD_PTR_FIFO_CTL	    (0x00014F0000000790ull)
#define CVMX_IPD_INT_ENB		    (0x00014F0000000160ull)
#define CVMX_IPD_INT_SUM		    (0x00014F0000000168ull)
#define CVMX_IPD_NEXT_PKT_PTR		    (0x00014F00000007A0ull)
#define CVMX_IPD_NEXT_WQE_PTR		    (0x00014F00000007A8ull)
#define CVMX_IPD_NOT_1ST_MBUFF_SKIP	    (0x00014F0000000008ull)
#define CVMX_IPD_ON_BP_DROP_PKTX(offset)    (0x00014F0000004100ull)
#define CVMX_IPD_PACKET_MBUFF_SIZE	    (0x00014F0000000010ull)
#define CVMX_IPD_PKT_ERR		    (0x00014F00000003F0ull)
#define CVMX_IPD_PKT_PTR_VALID		    (0x00014F0000000358ull)
#define CVMX_IPD_PORTX_BP_PAGE_CNT(offset)  (0x00014F0000000028ull + ((offset) & 63) * 8)
#define CVMX_IPD_PORTX_BP_PAGE_CNT2(offset) (0x00014F0000000368ull + ((offset) & 63) * 8 - 8 * 36)
#define CVMX_IPD_PORTX_BP_PAGE_CNT3(offset) (0x00014F00000003D0ull + ((offset) & 63) * 8 - 8 * 40)
#define CVMX_IPD_PORT_BP_COUNTERS2_PAIRX(offset)                                                   \
	(0x00014F0000000388ull + ((offset) & 63) * 8 - 8 * 36)
#define CVMX_IPD_PORT_BP_COUNTERS3_PAIRX(offset)                                                   \
	(0x00014F00000003B0ull + ((offset) & 63) * 8 - 8 * 40)
#define CVMX_IPD_PORT_BP_COUNTERS4_PAIRX(offset)                                                   \
	(0x00014F0000000410ull + ((offset) & 63) * 8 - 8 * 44)
#define CVMX_IPD_PORT_BP_COUNTERS_PAIRX(offset) (0x00014F00000001B8ull + ((offset) & 63) * 8)
#define CVMX_IPD_PORT_PTR_FIFO_CTL		(0x00014F0000000798ull)
#define CVMX_IPD_PORT_QOS_INTX(offset)		(0x00014F0000000808ull + ((offset) & 7) * 8)
#define CVMX_IPD_PORT_QOS_INT_ENBX(offset)	(0x00014F0000000848ull + ((offset) & 7) * 8)
#define CVMX_IPD_PORT_QOS_X_CNT(offset)		(0x00014F0000000888ull + ((offset) & 511) * 8)
#define CVMX_IPD_PORT_SOPX(offset)		(0x00014F0000004400ull)
#define CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL		(0x00014F0000000348ull)
#define CVMX_IPD_PRC_PORT_PTR_FIFO_CTL		(0x00014F0000000350ull)
#define CVMX_IPD_PTR_COUNT			(0x00014F0000000320ull)
#define CVMX_IPD_PWP_PTR_FIFO_CTL		(0x00014F0000000340ull)
#define CVMX_IPD_QOS0_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(0)
#define CVMX_IPD_QOS1_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(1)
#define CVMX_IPD_QOS2_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(2)
#define CVMX_IPD_QOS3_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(3)
#define CVMX_IPD_QOS4_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(4)
#define CVMX_IPD_QOS5_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(5)
#define CVMX_IPD_QOS6_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(6)
#define CVMX_IPD_QOS7_RED_MARKS			CVMX_IPD_QOSX_RED_MARKS(7)
#define CVMX_IPD_QOSX_RED_MARKS(offset)		(0x00014F0000000178ull + ((offset) & 7) * 8)
#define CVMX_IPD_QUE0_FREE_PAGE_CNT		(0x00014F0000000330ull)
#define CVMX_IPD_RED_BPID_ENABLEX(offset)	(0x00014F0000004200ull)
#define CVMX_IPD_RED_DELAY			(0x00014F0000004300ull)
#define CVMX_IPD_RED_PORT_ENABLE		(0x00014F00000002D8ull)
#define CVMX_IPD_RED_PORT_ENABLE2		(0x00014F00000003A8ull)
#define CVMX_IPD_RED_QUE0_PARAM			CVMX_IPD_RED_QUEX_PARAM(0)
#define CVMX_IPD_RED_QUE1_PARAM			CVMX_IPD_RED_QUEX_PARAM(1)
#define CVMX_IPD_RED_QUE2_PARAM			CVMX_IPD_RED_QUEX_PARAM(2)
#define CVMX_IPD_RED_QUE3_PARAM			CVMX_IPD_RED_QUEX_PARAM(3)
#define CVMX_IPD_RED_QUE4_PARAM			CVMX_IPD_RED_QUEX_PARAM(4)
#define CVMX_IPD_RED_QUE5_PARAM			CVMX_IPD_RED_QUEX_PARAM(5)
#define CVMX_IPD_RED_QUE6_PARAM			CVMX_IPD_RED_QUEX_PARAM(6)
#define CVMX_IPD_RED_QUE7_PARAM			CVMX_IPD_RED_QUEX_PARAM(7)
#define CVMX_IPD_RED_QUEX_PARAM(offset)		(0x00014F00000002E0ull + ((offset) & 7) * 8)
#define CVMX_IPD_REQ_WGT			(0x00014F0000004418ull)
#define CVMX_IPD_SUB_PORT_BP_PAGE_CNT		(0x00014F0000000148ull)
#define CVMX_IPD_SUB_PORT_FCS			(0x00014F0000000170ull)
#define CVMX_IPD_SUB_PORT_QOS_CNT		(0x00014F0000000800ull)
#define CVMX_IPD_WQE_FPA_QUEUE			(0x00014F0000000020ull)
#define CVMX_IPD_WQE_PTR_VALID			(0x00014F0000000360ull)

/**
 * cvmx_ipd_1st_mbuff_skip
 *
 * The number of words that the IPD will skip when writing the first MBUFF.
 *
 */
union cvmx_ipd_1st_mbuff_skip {
	u64 u64;
	struct cvmx_ipd_1st_mbuff_skip_s {
		u64 reserved_6_63 : 58;
		u64 skip_sz : 6;
	} s;
	struct cvmx_ipd_1st_mbuff_skip_s cn30xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn31xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn38xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn38xxp2;
	struct cvmx_ipd_1st_mbuff_skip_s cn50xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn52xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn52xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cn56xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn56xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cn58xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn58xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cn61xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn63xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn63xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cn66xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn68xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn68xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cn70xx;
	struct cvmx_ipd_1st_mbuff_skip_s cn70xxp1;
	struct cvmx_ipd_1st_mbuff_skip_s cnf71xx;
};

typedef union cvmx_ipd_1st_mbuff_skip cvmx_ipd_1st_mbuff_skip_t;

/**
 * cvmx_ipd_1st_next_ptr_back
 *
 * IPD_1st_NEXT_PTR_BACK = IPD First Next Pointer Back Values
 * Contains the Back Field for use in creating the Next Pointer Header for the First MBUF
 */
union cvmx_ipd_1st_next_ptr_back {
	u64 u64;
	struct cvmx_ipd_1st_next_ptr_back_s {
		u64 reserved_4_63 : 60;
		u64 back : 4;
	} s;
	struct cvmx_ipd_1st_next_ptr_back_s cn30xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn31xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn38xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn38xxp2;
	struct cvmx_ipd_1st_next_ptr_back_s cn50xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn52xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn52xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cn56xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn56xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cn58xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn58xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cn61xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn63xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn63xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cn66xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn68xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn68xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cn70xx;
	struct cvmx_ipd_1st_next_ptr_back_s cn70xxp1;
	struct cvmx_ipd_1st_next_ptr_back_s cnf71xx;
};

typedef union cvmx_ipd_1st_next_ptr_back cvmx_ipd_1st_next_ptr_back_t;

/**
 * cvmx_ipd_2nd_next_ptr_back
 *
 * Contains the Back Field for use in creating the Next Pointer Header for the First MBUF
 *
 */
union cvmx_ipd_2nd_next_ptr_back {
	u64 u64;
	struct cvmx_ipd_2nd_next_ptr_back_s {
		u64 reserved_4_63 : 60;
		u64 back : 4;
	} s;
	struct cvmx_ipd_2nd_next_ptr_back_s cn30xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn31xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn38xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn38xxp2;
	struct cvmx_ipd_2nd_next_ptr_back_s cn50xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn52xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn52xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cn56xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn56xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cn58xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn58xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cn61xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn63xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn63xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cn66xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn68xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn68xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cn70xx;
	struct cvmx_ipd_2nd_next_ptr_back_s cn70xxp1;
	struct cvmx_ipd_2nd_next_ptr_back_s cnf71xx;
};

typedef union cvmx_ipd_2nd_next_ptr_back cvmx_ipd_2nd_next_ptr_back_t;

/**
 * cvmx_ipd_bist_status
 *
 * BIST Status for IPD's Memories.
 *
 */
union cvmx_ipd_bist_status {
	u64 u64;
	struct cvmx_ipd_bist_status_s {
		u64 reserved_23_63 : 41;
		u64 iiwo1 : 1;
		u64 iiwo0 : 1;
		u64 iio1 : 1;
		u64 iio0 : 1;
		u64 pbm4 : 1;
		u64 csr_mem : 1;
		u64 csr_ncmd : 1;
		u64 pwq_wqed : 1;
		u64 pwq_wp1 : 1;
		u64 pwq_pow : 1;
		u64 ipq_pbe1 : 1;
		u64 ipq_pbe0 : 1;
		u64 pbm3 : 1;
		u64 pbm2 : 1;
		u64 pbm1 : 1;
		u64 pbm0 : 1;
		u64 pbm_word : 1;
		u64 pwq1 : 1;
		u64 pwq0 : 1;
		u64 prc_off : 1;
		u64 ipd_old : 1;
		u64 ipd_new : 1;
		u64 pwp : 1;
	} s;
	struct cvmx_ipd_bist_status_cn30xx {
		u64 reserved_16_63 : 48;
		u64 pwq_wqed : 1;
		u64 pwq_wp1 : 1;
		u64 pwq_pow : 1;
		u64 ipq_pbe1 : 1;
		u64 ipq_pbe0 : 1;
		u64 pbm3 : 1;
		u64 pbm2 : 1;
		u64 pbm1 : 1;
		u64 pbm0 : 1;
		u64 pbm_word : 1;
		u64 pwq1 : 1;
		u64 pwq0 : 1;
		u64 prc_off : 1;
		u64 ipd_old : 1;
		u64 ipd_new : 1;
		u64 pwp : 1;
	} cn30xx;
	struct cvmx_ipd_bist_status_cn30xx cn31xx;
	struct cvmx_ipd_bist_status_cn30xx cn38xx;
	struct cvmx_ipd_bist_status_cn30xx cn38xxp2;
	struct cvmx_ipd_bist_status_cn30xx cn50xx;
	struct cvmx_ipd_bist_status_cn52xx {
		u64 reserved_18_63 : 46;
		u64 csr_mem : 1;
		u64 csr_ncmd : 1;
		u64 pwq_wqed : 1;
		u64 pwq_wp1 : 1;
		u64 pwq_pow : 1;
		u64 ipq_pbe1 : 1;
		u64 ipq_pbe0 : 1;
		u64 pbm3 : 1;
		u64 pbm2 : 1;
		u64 pbm1 : 1;
		u64 pbm0 : 1;
		u64 pbm_word : 1;
		u64 pwq1 : 1;
		u64 pwq0 : 1;
		u64 prc_off : 1;
		u64 ipd_old : 1;
		u64 ipd_new : 1;
		u64 pwp : 1;
	} cn52xx;
	struct cvmx_ipd_bist_status_cn52xx cn52xxp1;
	struct cvmx_ipd_bist_status_cn52xx cn56xx;
	struct cvmx_ipd_bist_status_cn52xx cn56xxp1;
	struct cvmx_ipd_bist_status_cn30xx cn58xx;
	struct cvmx_ipd_bist_status_cn30xx cn58xxp1;
	struct cvmx_ipd_bist_status_cn52xx cn61xx;
	struct cvmx_ipd_bist_status_cn52xx cn63xx;
	struct cvmx_ipd_bist_status_cn52xx cn63xxp1;
	struct cvmx_ipd_bist_status_cn52xx cn66xx;
	struct cvmx_ipd_bist_status_s cn68xx;
	struct cvmx_ipd_bist_status_s cn68xxp1;
	struct cvmx_ipd_bist_status_cn52xx cn70xx;
	struct cvmx_ipd_bist_status_cn52xx cn70xxp1;
	struct cvmx_ipd_bist_status_cn52xx cnf71xx;
};

typedef union cvmx_ipd_bist_status cvmx_ipd_bist_status_t;

/**
 * cvmx_ipd_bp_prt_red_end
 *
 * When IPD applies backpressure to a PORT and the corresponding bit in this register is set,
 * the RED Unit will drop packets for that port.
 */
union cvmx_ipd_bp_prt_red_end {
	u64 u64;
	struct cvmx_ipd_bp_prt_red_end_s {
		u64 reserved_48_63 : 16;
		u64 prt_enb : 48;
	} s;
	struct cvmx_ipd_bp_prt_red_end_cn30xx {
		u64 reserved_36_63 : 28;
		u64 prt_enb : 36;
	} cn30xx;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn31xx;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn38xx;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn38xxp2;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn50xx;
	struct cvmx_ipd_bp_prt_red_end_cn52xx {
		u64 reserved_40_63 : 24;
		u64 prt_enb : 40;
	} cn52xx;
	struct cvmx_ipd_bp_prt_red_end_cn52xx cn52xxp1;
	struct cvmx_ipd_bp_prt_red_end_cn52xx cn56xx;
	struct cvmx_ipd_bp_prt_red_end_cn52xx cn56xxp1;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn58xx;
	struct cvmx_ipd_bp_prt_red_end_cn30xx cn58xxp1;
	struct cvmx_ipd_bp_prt_red_end_s cn61xx;
	struct cvmx_ipd_bp_prt_red_end_cn63xx {
		u64 reserved_44_63 : 20;
		u64 prt_enb : 44;
	} cn63xx;
	struct cvmx_ipd_bp_prt_red_end_cn63xx cn63xxp1;
	struct cvmx_ipd_bp_prt_red_end_s cn66xx;
	struct cvmx_ipd_bp_prt_red_end_s cn70xx;
	struct cvmx_ipd_bp_prt_red_end_s cn70xxp1;
	struct cvmx_ipd_bp_prt_red_end_s cnf71xx;
};

typedef union cvmx_ipd_bp_prt_red_end cvmx_ipd_bp_prt_red_end_t;

/**
 * cvmx_ipd_bpid#_mbuf_th
 *
 * 0x2000 2FFF
 *
 *                  IPD_BPIDX_MBUF_TH = IPD BPID  MBUFF Threshold
 *
 * The number of MBUFFs in use by the BPID, that when exceeded, backpressure will be applied to the BPID.
 */
union cvmx_ipd_bpidx_mbuf_th {
	u64 u64;
	struct cvmx_ipd_bpidx_mbuf_th_s {
		u64 reserved_18_63 : 46;
		u64 bp_enb : 1;
		u64 page_cnt : 17;
	} s;
	struct cvmx_ipd_bpidx_mbuf_th_s cn68xx;
	struct cvmx_ipd_bpidx_mbuf_th_s cn68xxp1;
};

typedef union cvmx_ipd_bpidx_mbuf_th cvmx_ipd_bpidx_mbuf_th_t;

/**
 * cvmx_ipd_bpid_bp_counter#
 *
 * RESERVE SPACE UPTO 0x2FFF
 *
 * 0x3000 0x3ffff
 *
 * IPD_BPID_BP_COUNTERX = MBUF BPID Counters used to generate Back Pressure Per BPID.
 */
union cvmx_ipd_bpid_bp_counterx {
	u64 u64;
	struct cvmx_ipd_bpid_bp_counterx_s {
		u64 reserved_25_63 : 39;
		u64 cnt_val : 25;
	} s;
	struct cvmx_ipd_bpid_bp_counterx_s cn68xx;
	struct cvmx_ipd_bpid_bp_counterx_s cn68xxp1;
};

typedef union cvmx_ipd_bpid_bp_counterx cvmx_ipd_bpid_bp_counterx_t;

/**
 * cvmx_ipd_clk_count
 *
 * Counts the number of core clocks periods since the de-asserition of reset.
 *
 */
union cvmx_ipd_clk_count {
	u64 u64;
	struct cvmx_ipd_clk_count_s {
		u64 clk_cnt : 64;
	} s;
	struct cvmx_ipd_clk_count_s cn30xx;
	struct cvmx_ipd_clk_count_s cn31xx;
	struct cvmx_ipd_clk_count_s cn38xx;
	struct cvmx_ipd_clk_count_s cn38xxp2;
	struct cvmx_ipd_clk_count_s cn50xx;
	struct cvmx_ipd_clk_count_s cn52xx;
	struct cvmx_ipd_clk_count_s cn52xxp1;
	struct cvmx_ipd_clk_count_s cn56xx;
	struct cvmx_ipd_clk_count_s cn56xxp1;
	struct cvmx_ipd_clk_count_s cn58xx;
	struct cvmx_ipd_clk_count_s cn58xxp1;
	struct cvmx_ipd_clk_count_s cn61xx;
	struct cvmx_ipd_clk_count_s cn63xx;
	struct cvmx_ipd_clk_count_s cn63xxp1;
	struct cvmx_ipd_clk_count_s cn66xx;
	struct cvmx_ipd_clk_count_s cn68xx;
	struct cvmx_ipd_clk_count_s cn68xxp1;
	struct cvmx_ipd_clk_count_s cn70xx;
	struct cvmx_ipd_clk_count_s cn70xxp1;
	struct cvmx_ipd_clk_count_s cnf71xx;
};

typedef union cvmx_ipd_clk_count cvmx_ipd_clk_count_t;

/**
 * cvmx_ipd_credits
 *
 * IPD_CREDITS = IPD Credits
 *
 * The credits allowed for IPD.
 */
union cvmx_ipd_credits {
	u64 u64;
	struct cvmx_ipd_credits_s {
		u64 reserved_16_63 : 48;
		u64 iob_wrc : 8;
		u64 iob_wr : 8;
	} s;
	struct cvmx_ipd_credits_s cn68xx;
	struct cvmx_ipd_credits_s cn68xxp1;
};

typedef union cvmx_ipd_credits cvmx_ipd_credits_t;

/**
 * cvmx_ipd_ctl_status
 *
 * The number of words in a MBUFF used for packet data store.
 *
 */
union cvmx_ipd_ctl_status {
	u64 u64;
	struct cvmx_ipd_ctl_status_s {
		u64 reserved_18_63 : 46;
		u64 use_sop : 1;
		u64 rst_done : 1;
		u64 clken : 1;
		u64 no_wptr : 1;
		u64 pq_apkt : 1;
		u64 pq_nabuf : 1;
		u64 ipd_full : 1;
		u64 pkt_off : 1;
		u64 len_m8 : 1;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} s;
	struct cvmx_ipd_ctl_status_cn30xx {
		u64 reserved_10_63 : 54;
		u64 len_m8 : 1;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} cn30xx;
	struct cvmx_ipd_ctl_status_cn30xx cn31xx;
	struct cvmx_ipd_ctl_status_cn30xx cn38xx;
	struct cvmx_ipd_ctl_status_cn38xxp2 {
		u64 reserved_9_63 : 55;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} cn38xxp2;
	struct cvmx_ipd_ctl_status_cn50xx {
		u64 reserved_15_63 : 49;
		u64 no_wptr : 1;
		u64 pq_apkt : 1;
		u64 pq_nabuf : 1;
		u64 ipd_full : 1;
		u64 pkt_off : 1;
		u64 len_m8 : 1;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} cn50xx;
	struct cvmx_ipd_ctl_status_cn50xx cn52xx;
	struct cvmx_ipd_ctl_status_cn50xx cn52xxp1;
	struct cvmx_ipd_ctl_status_cn50xx cn56xx;
	struct cvmx_ipd_ctl_status_cn50xx cn56xxp1;
	struct cvmx_ipd_ctl_status_cn58xx {
		u64 reserved_12_63 : 52;
		u64 ipd_full : 1;
		u64 pkt_off : 1;
		u64 len_m8 : 1;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} cn58xx;
	struct cvmx_ipd_ctl_status_cn58xx cn58xxp1;
	struct cvmx_ipd_ctl_status_s cn61xx;
	struct cvmx_ipd_ctl_status_s cn63xx;
	struct cvmx_ipd_ctl_status_cn63xxp1 {
		u64 reserved_16_63 : 48;
		u64 clken : 1;
		u64 no_wptr : 1;
		u64 pq_apkt : 1;
		u64 pq_nabuf : 1;
		u64 ipd_full : 1;
		u64 pkt_off : 1;
		u64 len_m8 : 1;
		u64 reset : 1;
		u64 addpkt : 1;
		u64 naddbuf : 1;
		u64 pkt_lend : 1;
		u64 wqe_lend : 1;
		u64 pbp_en : 1;
		cvmx_ipd_mode_t opc_mode : 2;
		u64 ipd_en : 1;
	} cn63xxp1;
	struct cvmx_ipd_ctl_status_s cn66xx;
	struct cvmx_ipd_ctl_status_s cn68xx;
	struct cvmx_ipd_ctl_status_s cn68xxp1;
	struct cvmx_ipd_ctl_status_s cn70xx;
	struct cvmx_ipd_ctl_status_s cn70xxp1;
	struct cvmx_ipd_ctl_status_s cnf71xx;
};

typedef union cvmx_ipd_ctl_status cvmx_ipd_ctl_status_t;

/**
 * cvmx_ipd_ecc_ctl
 *
 * IPD_ECC_CTL = IPD ECC Control
 *
 * Allows inserting ECC errors for testing.
 */
union cvmx_ipd_ecc_ctl {
	u64 u64;
	struct cvmx_ipd_ecc_ctl_s {
		u64 reserved_8_63 : 56;
		u64 pm3_syn : 2;
		u64 pm2_syn : 2;
		u64 pm1_syn : 2;
		u64 pm0_syn : 2;
	} s;
	struct cvmx_ipd_ecc_ctl_s cn68xx;
	struct cvmx_ipd_ecc_ctl_s cn68xxp1;
};

typedef union cvmx_ipd_ecc_ctl cvmx_ipd_ecc_ctl_t;

/**
 * cvmx_ipd_free_ptr_fifo_ctl
 *
 * IPD_FREE_PTR_FIFO_CTL = IPD's FREE Pointer FIFO Control
 *
 * Allows reading of the Page-Pointers stored in the IPD's FREE Fifo.
 * See also the IPD_FREE_PTR_VALUE
 */
union cvmx_ipd_free_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_free_ptr_fifo_ctl_s {
		u64 reserved_32_63 : 32;
		u64 max_cnts : 7;
		u64 wraddr : 8;
		u64 praddr : 8;
		u64 cena : 1;
		u64 raddr : 8;
	} s;
	struct cvmx_ipd_free_ptr_fifo_ctl_s cn68xx;
	struct cvmx_ipd_free_ptr_fifo_ctl_s cn68xxp1;
};

typedef union cvmx_ipd_free_ptr_fifo_ctl cvmx_ipd_free_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_free_ptr_value
 *
 * IPD_FREE_PTR_VALUE = IPD's FREE Pointer Value
 *
 * The value of the pointer selected through the IPD_FREE_PTR_FIFO_CTL
 */
union cvmx_ipd_free_ptr_value {
	u64 u64;
	struct cvmx_ipd_free_ptr_value_s {
		u64 reserved_33_63 : 31;
		u64 ptr : 33;
	} s;
	struct cvmx_ipd_free_ptr_value_s cn68xx;
	struct cvmx_ipd_free_ptr_value_s cn68xxp1;
};

typedef union cvmx_ipd_free_ptr_value cvmx_ipd_free_ptr_value_t;

/**
 * cvmx_ipd_hold_ptr_fifo_ctl
 *
 * IPD_HOLD_PTR_FIFO_CTL = IPD's Holding Pointer FIFO Control
 *
 * Allows reading of the Page-Pointers stored in the IPD's Holding Fifo.
 */
union cvmx_ipd_hold_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_hold_ptr_fifo_ctl_s {
		u64 reserved_43_63 : 21;
		u64 ptr : 33;
		u64 max_pkt : 3;
		u64 praddr : 3;
		u64 cena : 1;
		u64 raddr : 3;
	} s;
	struct cvmx_ipd_hold_ptr_fifo_ctl_s cn68xx;
	struct cvmx_ipd_hold_ptr_fifo_ctl_s cn68xxp1;
};

typedef union cvmx_ipd_hold_ptr_fifo_ctl cvmx_ipd_hold_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_int_enb
 *
 * IPD_INTERRUPT_ENB = IPD Interrupt Enable Register
 * Used to enable the various interrupting conditions of IPD
 */
union cvmx_ipd_int_enb {
	u64 u64;
	struct cvmx_ipd_int_enb_s {
		u64 reserved_23_63 : 41;
		u64 pw3_dbe : 1;
		u64 pw3_sbe : 1;
		u64 pw2_dbe : 1;
		u64 pw2_sbe : 1;
		u64 pw1_dbe : 1;
		u64 pw1_sbe : 1;
		u64 pw0_dbe : 1;
		u64 pw0_sbe : 1;
		u64 dat : 1;
		u64 eop : 1;
		u64 sop : 1;
		u64 pq_sub : 1;
		u64 pq_add : 1;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} s;
	struct cvmx_ipd_int_enb_cn30xx {
		u64 reserved_5_63 : 59;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn30xx;
	struct cvmx_ipd_int_enb_cn30xx cn31xx;
	struct cvmx_ipd_int_enb_cn38xx {
		u64 reserved_10_63 : 54;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn38xx;
	struct cvmx_ipd_int_enb_cn30xx cn38xxp2;
	struct cvmx_ipd_int_enb_cn38xx cn50xx;
	struct cvmx_ipd_int_enb_cn52xx {
		u64 reserved_12_63 : 52;
		u64 pq_sub : 1;
		u64 pq_add : 1;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn52xx;
	struct cvmx_ipd_int_enb_cn52xx cn52xxp1;
	struct cvmx_ipd_int_enb_cn52xx cn56xx;
	struct cvmx_ipd_int_enb_cn52xx cn56xxp1;
	struct cvmx_ipd_int_enb_cn38xx cn58xx;
	struct cvmx_ipd_int_enb_cn38xx cn58xxp1;
	struct cvmx_ipd_int_enb_cn52xx cn61xx;
	struct cvmx_ipd_int_enb_cn52xx cn63xx;
	struct cvmx_ipd_int_enb_cn52xx cn63xxp1;
	struct cvmx_ipd_int_enb_cn52xx cn66xx;
	struct cvmx_ipd_int_enb_s cn68xx;
	struct cvmx_ipd_int_enb_s cn68xxp1;
	struct cvmx_ipd_int_enb_cn52xx cn70xx;
	struct cvmx_ipd_int_enb_cn52xx cn70xxp1;
	struct cvmx_ipd_int_enb_cn52xx cnf71xx;
};

typedef union cvmx_ipd_int_enb cvmx_ipd_int_enb_t;

/**
 * cvmx_ipd_int_sum
 *
 * IPD_INTERRUPT_SUM = IPD Interrupt Summary Register
 * Set when an interrupt condition occurs, write '1' to clear.
 */
union cvmx_ipd_int_sum {
	u64 u64;
	struct cvmx_ipd_int_sum_s {
		u64 reserved_23_63 : 41;
		u64 pw3_dbe : 1;
		u64 pw3_sbe : 1;
		u64 pw2_dbe : 1;
		u64 pw2_sbe : 1;
		u64 pw1_dbe : 1;
		u64 pw1_sbe : 1;
		u64 pw0_dbe : 1;
		u64 pw0_sbe : 1;
		u64 dat : 1;
		u64 eop : 1;
		u64 sop : 1;
		u64 pq_sub : 1;
		u64 pq_add : 1;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} s;
	struct cvmx_ipd_int_sum_cn30xx {
		u64 reserved_5_63 : 59;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn30xx;
	struct cvmx_ipd_int_sum_cn30xx cn31xx;
	struct cvmx_ipd_int_sum_cn38xx {
		u64 reserved_10_63 : 54;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn38xx;
	struct cvmx_ipd_int_sum_cn30xx cn38xxp2;
	struct cvmx_ipd_int_sum_cn38xx cn50xx;
	struct cvmx_ipd_int_sum_cn52xx {
		u64 reserved_12_63 : 52;
		u64 pq_sub : 1;
		u64 pq_add : 1;
		u64 bc_ovr : 1;
		u64 d_coll : 1;
		u64 c_coll : 1;
		u64 cc_ovr : 1;
		u64 dc_ovr : 1;
		u64 bp_sub : 1;
		u64 prc_par3 : 1;
		u64 prc_par2 : 1;
		u64 prc_par1 : 1;
		u64 prc_par0 : 1;
	} cn52xx;
	struct cvmx_ipd_int_sum_cn52xx cn52xxp1;
	struct cvmx_ipd_int_sum_cn52xx cn56xx;
	struct cvmx_ipd_int_sum_cn52xx cn56xxp1;
	struct cvmx_ipd_int_sum_cn38xx cn58xx;
	struct cvmx_ipd_int_sum_cn38xx cn58xxp1;
	struct cvmx_ipd_int_sum_cn52xx cn61xx;
	struct cvmx_ipd_int_sum_cn52xx cn63xx;
	struct cvmx_ipd_int_sum_cn52xx cn63xxp1;
	struct cvmx_ipd_int_sum_cn52xx cn66xx;
	struct cvmx_ipd_int_sum_s cn68xx;
	struct cvmx_ipd_int_sum_s cn68xxp1;
	struct cvmx_ipd_int_sum_cn52xx cn70xx;
	struct cvmx_ipd_int_sum_cn52xx cn70xxp1;
	struct cvmx_ipd_int_sum_cn52xx cnf71xx;
};

typedef union cvmx_ipd_int_sum cvmx_ipd_int_sum_t;

/**
 * cvmx_ipd_next_pkt_ptr
 *
 * IPD_NEXT_PKT_PTR = IPD's Next Packet Pointer
 *
 * The value of the packet-pointer fetched and in the valid register.
 */
union cvmx_ipd_next_pkt_ptr {
	u64 u64;
	struct cvmx_ipd_next_pkt_ptr_s {
		u64 reserved_33_63 : 31;
		u64 ptr : 33;
	} s;
	struct cvmx_ipd_next_pkt_ptr_s cn68xx;
	struct cvmx_ipd_next_pkt_ptr_s cn68xxp1;
};

typedef union cvmx_ipd_next_pkt_ptr cvmx_ipd_next_pkt_ptr_t;

/**
 * cvmx_ipd_next_wqe_ptr
 *
 * IPD_NEXT_WQE_PTR = IPD's NEXT_WQE Pointer
 *
 * The value of the WQE-pointer fetched and in the valid register.
 */
union cvmx_ipd_next_wqe_ptr {
	u64 u64;
	struct cvmx_ipd_next_wqe_ptr_s {
		u64 reserved_33_63 : 31;
		u64 ptr : 33;
	} s;
	struct cvmx_ipd_next_wqe_ptr_s cn68xx;
	struct cvmx_ipd_next_wqe_ptr_s cn68xxp1;
};

typedef union cvmx_ipd_next_wqe_ptr cvmx_ipd_next_wqe_ptr_t;

/**
 * cvmx_ipd_not_1st_mbuff_skip
 *
 * The number of words that the IPD will skip when writing any MBUFF that is not the first.
 *
 */
union cvmx_ipd_not_1st_mbuff_skip {
	u64 u64;
	struct cvmx_ipd_not_1st_mbuff_skip_s {
		u64 reserved_6_63 : 58;
		u64 skip_sz : 6;
	} s;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn30xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn31xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn38xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn38xxp2;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn50xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn52xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn52xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn56xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn56xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn58xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn58xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn61xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn63xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn63xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn66xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn68xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn68xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn70xx;
	struct cvmx_ipd_not_1st_mbuff_skip_s cn70xxp1;
	struct cvmx_ipd_not_1st_mbuff_skip_s cnf71xx;
};

typedef union cvmx_ipd_not_1st_mbuff_skip cvmx_ipd_not_1st_mbuff_skip_t;

/**
 * cvmx_ipd_on_bp_drop_pkt#
 *
 * RESERVE SPACE UPTO 0x3FFF
 *
 *
 * RESERVED FOR FORMER IPD_SUB_PKIND_FCS - MOVED TO PIP
 *
 * RESERVE 4008 - 40FF
 *
 *
 *                  IPD_ON_BP_DROP_PKT = IPD On Backpressure Drop Packet
 *
 * When IPD applies backpressure to a BPID and the corresponding bit in this register is set,
 * then previously received packets will be dropped when processed.
 */
union cvmx_ipd_on_bp_drop_pktx {
	u64 u64;
	struct cvmx_ipd_on_bp_drop_pktx_s {
		u64 prt_enb : 64;
	} s;
	struct cvmx_ipd_on_bp_drop_pktx_s cn68xx;
	struct cvmx_ipd_on_bp_drop_pktx_s cn68xxp1;
};

typedef union cvmx_ipd_on_bp_drop_pktx cvmx_ipd_on_bp_drop_pktx_t;

/**
 * cvmx_ipd_packet_mbuff_size
 *
 * The number of words in a MBUFF used for packet data store.
 *
 */
union cvmx_ipd_packet_mbuff_size {
	u64 u64;
	struct cvmx_ipd_packet_mbuff_size_s {
		u64 reserved_12_63 : 52;
		u64 mb_size : 12;
	} s;
	struct cvmx_ipd_packet_mbuff_size_s cn30xx;
	struct cvmx_ipd_packet_mbuff_size_s cn31xx;
	struct cvmx_ipd_packet_mbuff_size_s cn38xx;
	struct cvmx_ipd_packet_mbuff_size_s cn38xxp2;
	struct cvmx_ipd_packet_mbuff_size_s cn50xx;
	struct cvmx_ipd_packet_mbuff_size_s cn52xx;
	struct cvmx_ipd_packet_mbuff_size_s cn52xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cn56xx;
	struct cvmx_ipd_packet_mbuff_size_s cn56xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cn58xx;
	struct cvmx_ipd_packet_mbuff_size_s cn58xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cn61xx;
	struct cvmx_ipd_packet_mbuff_size_s cn63xx;
	struct cvmx_ipd_packet_mbuff_size_s cn63xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cn66xx;
	struct cvmx_ipd_packet_mbuff_size_s cn68xx;
	struct cvmx_ipd_packet_mbuff_size_s cn68xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cn70xx;
	struct cvmx_ipd_packet_mbuff_size_s cn70xxp1;
	struct cvmx_ipd_packet_mbuff_size_s cnf71xx;
};

typedef union cvmx_ipd_packet_mbuff_size cvmx_ipd_packet_mbuff_size_t;

/**
 * cvmx_ipd_pkt_err
 *
 * IPD_PKT_ERR = IPD Packet Error Register
 *
 * Provides status about the failing packet recevie error.
 */
union cvmx_ipd_pkt_err {
	u64 u64;
	struct cvmx_ipd_pkt_err_s {
		u64 reserved_6_63 : 58;
		u64 reasm : 6;
	} s;
	struct cvmx_ipd_pkt_err_s cn68xx;
	struct cvmx_ipd_pkt_err_s cn68xxp1;
};

typedef union cvmx_ipd_pkt_err cvmx_ipd_pkt_err_t;

/**
 * cvmx_ipd_pkt_ptr_valid
 *
 * The value of the packet-pointer fetched and in the valid register.
 *
 */
union cvmx_ipd_pkt_ptr_valid {
	u64 u64;
	struct cvmx_ipd_pkt_ptr_valid_s {
		u64 reserved_29_63 : 35;
		u64 ptr : 29;
	} s;
	struct cvmx_ipd_pkt_ptr_valid_s cn30xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn31xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn38xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn50xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn52xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn52xxp1;
	struct cvmx_ipd_pkt_ptr_valid_s cn56xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn56xxp1;
	struct cvmx_ipd_pkt_ptr_valid_s cn58xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn58xxp1;
	struct cvmx_ipd_pkt_ptr_valid_s cn61xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn63xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn63xxp1;
	struct cvmx_ipd_pkt_ptr_valid_s cn66xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn70xx;
	struct cvmx_ipd_pkt_ptr_valid_s cn70xxp1;
	struct cvmx_ipd_pkt_ptr_valid_s cnf71xx;
};

typedef union cvmx_ipd_pkt_ptr_valid cvmx_ipd_pkt_ptr_valid_t;

/**
 * cvmx_ipd_port#_bp_page_cnt
 *
 * IPD_PORTX_BP_PAGE_CNT = IPD Port Backpressure Page Count
 * The number of pages in use by the port that when exceeded, backpressure will be applied to the
 * port.
 * See also IPD_PORTX_BP_PAGE_CNT2
 * See also IPD_PORTX_BP_PAGE_CNT3
 */
union cvmx_ipd_portx_bp_page_cnt {
	u64 u64;
	struct cvmx_ipd_portx_bp_page_cnt_s {
		u64 reserved_18_63 : 46;
		u64 bp_enb : 1;
		u64 page_cnt : 17;
	} s;
	struct cvmx_ipd_portx_bp_page_cnt_s cn30xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn31xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn38xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn38xxp2;
	struct cvmx_ipd_portx_bp_page_cnt_s cn50xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn52xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn52xxp1;
	struct cvmx_ipd_portx_bp_page_cnt_s cn56xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn56xxp1;
	struct cvmx_ipd_portx_bp_page_cnt_s cn58xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn58xxp1;
	struct cvmx_ipd_portx_bp_page_cnt_s cn61xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn63xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn63xxp1;
	struct cvmx_ipd_portx_bp_page_cnt_s cn66xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn70xx;
	struct cvmx_ipd_portx_bp_page_cnt_s cn70xxp1;
	struct cvmx_ipd_portx_bp_page_cnt_s cnf71xx;
};

typedef union cvmx_ipd_portx_bp_page_cnt cvmx_ipd_portx_bp_page_cnt_t;

/**
 * cvmx_ipd_port#_bp_page_cnt2
 *
 * IPD_PORTX_BP_PAGE_CNT2 = IPD Port Backpressure Page Count
 * The number of pages in use by the port that when exceeded, backpressure will be applied to the
 * port.
 * See also IPD_PORTX_BP_PAGE_CNT
 * See also IPD_PORTX_BP_PAGE_CNT3
 * 0x368-0x380
 */
union cvmx_ipd_portx_bp_page_cnt2 {
	u64 u64;
	struct cvmx_ipd_portx_bp_page_cnt2_s {
		u64 reserved_18_63 : 46;
		u64 bp_enb : 1;
		u64 page_cnt : 17;
	} s;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn52xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn52xxp1;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn56xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn56xxp1;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn61xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn63xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn63xxp1;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn66xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn70xx;
	struct cvmx_ipd_portx_bp_page_cnt2_s cn70xxp1;
	struct cvmx_ipd_portx_bp_page_cnt2_s cnf71xx;
};

typedef union cvmx_ipd_portx_bp_page_cnt2 cvmx_ipd_portx_bp_page_cnt2_t;

/**
 * cvmx_ipd_port#_bp_page_cnt3
 *
 * IPD_PORTX_BP_PAGE_CNT3 = IPD Port Backpressure Page Count
 * The number of pages in use by the port that when exceeded, backpressure will be applied to the
 * port.
 * See also IPD_PORTX_BP_PAGE_CNT
 * See also IPD_PORTX_BP_PAGE_CNT2
 * 0x3d0-408
 */
union cvmx_ipd_portx_bp_page_cnt3 {
	u64 u64;
	struct cvmx_ipd_portx_bp_page_cnt3_s {
		u64 reserved_18_63 : 46;
		u64 bp_enb : 1;
		u64 page_cnt : 17;
	} s;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn61xx;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn63xx;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn63xxp1;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn66xx;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn70xx;
	struct cvmx_ipd_portx_bp_page_cnt3_s cn70xxp1;
	struct cvmx_ipd_portx_bp_page_cnt3_s cnf71xx;
};

typedef union cvmx_ipd_portx_bp_page_cnt3 cvmx_ipd_portx_bp_page_cnt3_t;

/**
 * cvmx_ipd_port_bp_counters2_pair#
 *
 * See also IPD_PORT_BP_COUNTERS_PAIRX
 * See also IPD_PORT_BP_COUNTERS3_PAIRX
 * 0x388-0x3a0
 */
union cvmx_ipd_port_bp_counters2_pairx {
	u64 u64;
	struct cvmx_ipd_port_bp_counters2_pairx_s {
		u64 reserved_25_63 : 39;
		u64 cnt_val : 25;
	} s;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn52xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn52xxp1;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn56xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn56xxp1;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn61xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn63xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn63xxp1;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn66xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn70xx;
	struct cvmx_ipd_port_bp_counters2_pairx_s cn70xxp1;
	struct cvmx_ipd_port_bp_counters2_pairx_s cnf71xx;
};

typedef union cvmx_ipd_port_bp_counters2_pairx cvmx_ipd_port_bp_counters2_pairx_t;

/**
 * cvmx_ipd_port_bp_counters3_pair#
 *
 * See also IPD_PORT_BP_COUNTERS_PAIRX
 * See also IPD_PORT_BP_COUNTERS2_PAIRX
 * 0x3b0-0x3c8
 */
union cvmx_ipd_port_bp_counters3_pairx {
	u64 u64;
	struct cvmx_ipd_port_bp_counters3_pairx_s {
		u64 reserved_25_63 : 39;
		u64 cnt_val : 25;
	} s;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn61xx;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn63xx;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn63xxp1;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn66xx;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn70xx;
	struct cvmx_ipd_port_bp_counters3_pairx_s cn70xxp1;
	struct cvmx_ipd_port_bp_counters3_pairx_s cnf71xx;
};

typedef union cvmx_ipd_port_bp_counters3_pairx cvmx_ipd_port_bp_counters3_pairx_t;

/**
 * cvmx_ipd_port_bp_counters4_pair#
 *
 * See also IPD_PORT_BP_COUNTERS_PAIRX
 * See also IPD_PORT_BP_COUNTERS2_PAIRX
 * 0x410-0x3c8
 */
union cvmx_ipd_port_bp_counters4_pairx {
	u64 u64;
	struct cvmx_ipd_port_bp_counters4_pairx_s {
		u64 reserved_25_63 : 39;
		u64 cnt_val : 25;
	} s;
	struct cvmx_ipd_port_bp_counters4_pairx_s cn61xx;
	struct cvmx_ipd_port_bp_counters4_pairx_s cn66xx;
	struct cvmx_ipd_port_bp_counters4_pairx_s cn70xx;
	struct cvmx_ipd_port_bp_counters4_pairx_s cn70xxp1;
	struct cvmx_ipd_port_bp_counters4_pairx_s cnf71xx;
};

typedef union cvmx_ipd_port_bp_counters4_pairx cvmx_ipd_port_bp_counters4_pairx_t;

/**
 * cvmx_ipd_port_bp_counters_pair#
 *
 * See also IPD_PORT_BP_COUNTERS2_PAIRX
 * See also IPD_PORT_BP_COUNTERS3_PAIRX
 * 0x1b8-0x2d0
 */
union cvmx_ipd_port_bp_counters_pairx {
	u64 u64;
	struct cvmx_ipd_port_bp_counters_pairx_s {
		u64 reserved_25_63 : 39;
		u64 cnt_val : 25;
	} s;
	struct cvmx_ipd_port_bp_counters_pairx_s cn30xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn31xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn38xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn38xxp2;
	struct cvmx_ipd_port_bp_counters_pairx_s cn50xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn52xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn52xxp1;
	struct cvmx_ipd_port_bp_counters_pairx_s cn56xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn56xxp1;
	struct cvmx_ipd_port_bp_counters_pairx_s cn58xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn58xxp1;
	struct cvmx_ipd_port_bp_counters_pairx_s cn61xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn63xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn63xxp1;
	struct cvmx_ipd_port_bp_counters_pairx_s cn66xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn70xx;
	struct cvmx_ipd_port_bp_counters_pairx_s cn70xxp1;
	struct cvmx_ipd_port_bp_counters_pairx_s cnf71xx;
};

typedef union cvmx_ipd_port_bp_counters_pairx cvmx_ipd_port_bp_counters_pairx_t;

/**
 * cvmx_ipd_port_ptr_fifo_ctl
 *
 * IPD_PORT_PTR_FIFO_CTL = IPD's Reasm-Id Pointer FIFO Control
 *
 * Allows reading of the Page-Pointers stored in the IPD's Reasm-Id Fifo.
 */
union cvmx_ipd_port_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_port_ptr_fifo_ctl_s {
		u64 reserved_48_63 : 16;
		u64 ptr : 33;
		u64 max_pkt : 7;
		u64 cena : 1;
		u64 raddr : 7;
	} s;
	struct cvmx_ipd_port_ptr_fifo_ctl_s cn68xx;
	struct cvmx_ipd_port_ptr_fifo_ctl_s cn68xxp1;
};

typedef union cvmx_ipd_port_ptr_fifo_ctl cvmx_ipd_port_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_port_qos_#_cnt
 *
 * IPD_PORT_QOS_X_CNT = IPD PortX QOS-0 Count
 * A counter per port/qos. Counter are originzed in sequence where the first 8 counter (0-7)
 * belong to Port-0
 * QOS 0-7 respectively followed by port 1 at (8-15), etc
 * Ports 0-3, 32-43
 */
union cvmx_ipd_port_qos_x_cnt {
	u64 u64;
	struct cvmx_ipd_port_qos_x_cnt_s {
		u64 wmark : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_ipd_port_qos_x_cnt_s cn52xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn52xxp1;
	struct cvmx_ipd_port_qos_x_cnt_s cn56xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn56xxp1;
	struct cvmx_ipd_port_qos_x_cnt_s cn61xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn63xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn63xxp1;
	struct cvmx_ipd_port_qos_x_cnt_s cn66xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn68xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn68xxp1;
	struct cvmx_ipd_port_qos_x_cnt_s cn70xx;
	struct cvmx_ipd_port_qos_x_cnt_s cn70xxp1;
	struct cvmx_ipd_port_qos_x_cnt_s cnf71xx;
};

typedef union cvmx_ipd_port_qos_x_cnt cvmx_ipd_port_qos_x_cnt_t;

/**
 * cvmx_ipd_port_qos_int#
 *
 * See the description for IPD_PORT_QOS_X_CNT
 * 0=P0-7; 1=P8-15; 2=P16-23; 3=P24-31; 4=P32-39; 5=P40-47; 6=P48-55; 7=P56-63
 * Only ports used are: P0-3, p16-19, P24, P32-39. Therefore only IPD_PORT_QOS_INT0 ([63:32] ==
 * Reserved), IPD_PORT_QOS_INT2 ([63:32] == Reserved), IPD_PORT_QOS_INT3 ([63:8] == Reserved),
 * IPD_PORT_QOS_INT4
 */
union cvmx_ipd_port_qos_intx {
	u64 u64;
	struct cvmx_ipd_port_qos_intx_s {
		u64 intr : 64;
	} s;
	struct cvmx_ipd_port_qos_intx_s cn52xx;
	struct cvmx_ipd_port_qos_intx_s cn52xxp1;
	struct cvmx_ipd_port_qos_intx_s cn56xx;
	struct cvmx_ipd_port_qos_intx_s cn56xxp1;
	struct cvmx_ipd_port_qos_intx_s cn61xx;
	struct cvmx_ipd_port_qos_intx_s cn63xx;
	struct cvmx_ipd_port_qos_intx_s cn63xxp1;
	struct cvmx_ipd_port_qos_intx_s cn66xx;
	struct cvmx_ipd_port_qos_intx_s cn68xx;
	struct cvmx_ipd_port_qos_intx_s cn68xxp1;
	struct cvmx_ipd_port_qos_intx_s cn70xx;
	struct cvmx_ipd_port_qos_intx_s cn70xxp1;
	struct cvmx_ipd_port_qos_intx_s cnf71xx;
};

typedef union cvmx_ipd_port_qos_intx cvmx_ipd_port_qos_intx_t;

/**
 * cvmx_ipd_port_qos_int_enb#
 *
 * "When the IPD_PORT_QOS_INTX[\#] is '1' and IPD_PORT_QOS_INT_ENBX[\#] is '1' a interrupt will be
 * generated."
 */
union cvmx_ipd_port_qos_int_enbx {
	u64 u64;
	struct cvmx_ipd_port_qos_int_enbx_s {
		u64 enb : 64;
	} s;
	struct cvmx_ipd_port_qos_int_enbx_s cn52xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn52xxp1;
	struct cvmx_ipd_port_qos_int_enbx_s cn56xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn56xxp1;
	struct cvmx_ipd_port_qos_int_enbx_s cn61xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn63xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn63xxp1;
	struct cvmx_ipd_port_qos_int_enbx_s cn66xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn68xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn68xxp1;
	struct cvmx_ipd_port_qos_int_enbx_s cn70xx;
	struct cvmx_ipd_port_qos_int_enbx_s cn70xxp1;
	struct cvmx_ipd_port_qos_int_enbx_s cnf71xx;
};

typedef union cvmx_ipd_port_qos_int_enbx cvmx_ipd_port_qos_int_enbx_t;

/**
 * cvmx_ipd_port_sop#
 *
 * IPD_PORT_SOP = IPD Reasm-Id SOP
 *
 * Set when a SOP is detected on a reasm-num. Where the reasm-num value set the bit vector of this register.
 */
union cvmx_ipd_port_sopx {
	u64 u64;
	struct cvmx_ipd_port_sopx_s {
		u64 sop : 64;
	} s;
	struct cvmx_ipd_port_sopx_s cn68xx;
	struct cvmx_ipd_port_sopx_s cn68xxp1;
};

typedef union cvmx_ipd_port_sopx cvmx_ipd_port_sopx_t;

/**
 * cvmx_ipd_prc_hold_ptr_fifo_ctl
 *
 * Allows reading of the Page-Pointers stored in the IPD's PRC Holding Fifo.
 *
 */
union cvmx_ipd_prc_hold_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s {
		u64 reserved_39_63 : 25;
		u64 max_pkt : 3;
		u64 praddr : 3;
		u64 ptr : 29;
		u64 cena : 1;
		u64 raddr : 3;
	} s;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn30xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn31xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn38xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn50xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn52xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn52xxp1;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn56xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn56xxp1;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn58xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn58xxp1;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn61xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn63xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn63xxp1;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn66xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn70xx;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cn70xxp1;
	struct cvmx_ipd_prc_hold_ptr_fifo_ctl_s cnf71xx;
};

typedef union cvmx_ipd_prc_hold_ptr_fifo_ctl cvmx_ipd_prc_hold_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_prc_port_ptr_fifo_ctl
 *
 * Allows reading of the Page-Pointers stored in the IPD's PRC PORT Fifo.
 *
 */
union cvmx_ipd_prc_port_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s {
		u64 reserved_44_63 : 20;
		u64 max_pkt : 7;
		u64 ptr : 29;
		u64 cena : 1;
		u64 raddr : 7;
	} s;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn30xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn31xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn38xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn50xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn52xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn52xxp1;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn56xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn56xxp1;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn58xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn58xxp1;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn61xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn63xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn63xxp1;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn66xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn70xx;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cn70xxp1;
	struct cvmx_ipd_prc_port_ptr_fifo_ctl_s cnf71xx;
};

typedef union cvmx_ipd_prc_port_ptr_fifo_ctl cvmx_ipd_prc_port_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_ptr_count
 *
 * Shows the number of WQE and Packet Page Pointers stored in the IPD.
 *
 */
union cvmx_ipd_ptr_count {
	u64 u64;
	struct cvmx_ipd_ptr_count_s {
		u64 reserved_19_63 : 45;
		u64 pktv_cnt : 1;
		u64 wqev_cnt : 1;
		u64 pfif_cnt : 3;
		u64 pkt_pcnt : 7;
		u64 wqe_pcnt : 7;
	} s;
	struct cvmx_ipd_ptr_count_s cn30xx;
	struct cvmx_ipd_ptr_count_s cn31xx;
	struct cvmx_ipd_ptr_count_s cn38xx;
	struct cvmx_ipd_ptr_count_s cn38xxp2;
	struct cvmx_ipd_ptr_count_s cn50xx;
	struct cvmx_ipd_ptr_count_s cn52xx;
	struct cvmx_ipd_ptr_count_s cn52xxp1;
	struct cvmx_ipd_ptr_count_s cn56xx;
	struct cvmx_ipd_ptr_count_s cn56xxp1;
	struct cvmx_ipd_ptr_count_s cn58xx;
	struct cvmx_ipd_ptr_count_s cn58xxp1;
	struct cvmx_ipd_ptr_count_s cn61xx;
	struct cvmx_ipd_ptr_count_s cn63xx;
	struct cvmx_ipd_ptr_count_s cn63xxp1;
	struct cvmx_ipd_ptr_count_s cn66xx;
	struct cvmx_ipd_ptr_count_s cn68xx;
	struct cvmx_ipd_ptr_count_s cn68xxp1;
	struct cvmx_ipd_ptr_count_s cn70xx;
	struct cvmx_ipd_ptr_count_s cn70xxp1;
	struct cvmx_ipd_ptr_count_s cnf71xx;
};

typedef union cvmx_ipd_ptr_count cvmx_ipd_ptr_count_t;

/**
 * cvmx_ipd_pwp_ptr_fifo_ctl
 *
 * Allows reading of the Page-Pointers stored in the IPD's PWP Fifo.
 *
 */
union cvmx_ipd_pwp_ptr_fifo_ctl {
	u64 u64;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s {
		u64 reserved_61_63 : 3;
		u64 max_cnts : 7;
		u64 wraddr : 8;
		u64 praddr : 8;
		u64 ptr : 29;
		u64 cena : 1;
		u64 raddr : 8;
	} s;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn30xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn31xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn38xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn50xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn52xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn52xxp1;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn56xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn56xxp1;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn58xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn58xxp1;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn61xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn63xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn63xxp1;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn66xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn70xx;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cn70xxp1;
	struct cvmx_ipd_pwp_ptr_fifo_ctl_s cnf71xx;
};

typedef union cvmx_ipd_pwp_ptr_fifo_ctl cvmx_ipd_pwp_ptr_fifo_ctl_t;

/**
 * cvmx_ipd_qos#_red_marks
 *
 * Set the pass-drop marks for qos level.
 *
 */
union cvmx_ipd_qosx_red_marks {
	u64 u64;
	struct cvmx_ipd_qosx_red_marks_s {
		u64 drop : 32;
		u64 pass : 32;
	} s;
	struct cvmx_ipd_qosx_red_marks_s cn30xx;
	struct cvmx_ipd_qosx_red_marks_s cn31xx;
	struct cvmx_ipd_qosx_red_marks_s cn38xx;
	struct cvmx_ipd_qosx_red_marks_s cn38xxp2;
	struct cvmx_ipd_qosx_red_marks_s cn50xx;
	struct cvmx_ipd_qosx_red_marks_s cn52xx;
	struct cvmx_ipd_qosx_red_marks_s cn52xxp1;
	struct cvmx_ipd_qosx_red_marks_s cn56xx;
	struct cvmx_ipd_qosx_red_marks_s cn56xxp1;
	struct cvmx_ipd_qosx_red_marks_s cn58xx;
	struct cvmx_ipd_qosx_red_marks_s cn58xxp1;
	struct cvmx_ipd_qosx_red_marks_s cn61xx;
	struct cvmx_ipd_qosx_red_marks_s cn63xx;
	struct cvmx_ipd_qosx_red_marks_s cn63xxp1;
	struct cvmx_ipd_qosx_red_marks_s cn66xx;
	struct cvmx_ipd_qosx_red_marks_s cn68xx;
	struct cvmx_ipd_qosx_red_marks_s cn68xxp1;
	struct cvmx_ipd_qosx_red_marks_s cn70xx;
	struct cvmx_ipd_qosx_red_marks_s cn70xxp1;
	struct cvmx_ipd_qosx_red_marks_s cnf71xx;
};

typedef union cvmx_ipd_qosx_red_marks cvmx_ipd_qosx_red_marks_t;

/**
 * cvmx_ipd_que0_free_page_cnt
 *
 * Number of Free-Page Pointer that are available for use in the FPA for Queue-0.
 *
 */
union cvmx_ipd_que0_free_page_cnt {
	u64 u64;
	struct cvmx_ipd_que0_free_page_cnt_s {
		u64 reserved_32_63 : 32;
		u64 q0_pcnt : 32;
	} s;
	struct cvmx_ipd_que0_free_page_cnt_s cn30xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn31xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn38xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn38xxp2;
	struct cvmx_ipd_que0_free_page_cnt_s cn50xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn52xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn52xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cn56xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn56xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cn58xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn58xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cn61xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn63xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn63xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cn66xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn68xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn68xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cn70xx;
	struct cvmx_ipd_que0_free_page_cnt_s cn70xxp1;
	struct cvmx_ipd_que0_free_page_cnt_s cnf71xx;
};

typedef union cvmx_ipd_que0_free_page_cnt cvmx_ipd_que0_free_page_cnt_t;

/**
 * cvmx_ipd_red_bpid_enable#
 *
 * IPD_RED_BPID_ENABLE = IPD RED BPID Enable
 *
 * Set the pass-drop marks for qos level.
 */
union cvmx_ipd_red_bpid_enablex {
	u64 u64;
	struct cvmx_ipd_red_bpid_enablex_s {
		u64 prt_enb : 64;
	} s;
	struct cvmx_ipd_red_bpid_enablex_s cn68xx;
	struct cvmx_ipd_red_bpid_enablex_s cn68xxp1;
};

typedef union cvmx_ipd_red_bpid_enablex cvmx_ipd_red_bpid_enablex_t;

/**
 * cvmx_ipd_red_delay
 *
 * IPD_RED_DELAY = IPD RED BPID Enable
 *
 * Set the pass-drop marks for qos level.
 */
union cvmx_ipd_red_delay {
	u64 u64;
	struct cvmx_ipd_red_delay_s {
		u64 reserved_28_63 : 36;
		u64 prb_dly : 14;
		u64 avg_dly : 14;
	} s;
	struct cvmx_ipd_red_delay_s cn68xx;
	struct cvmx_ipd_red_delay_s cn68xxp1;
};

typedef union cvmx_ipd_red_delay cvmx_ipd_red_delay_t;

/**
 * cvmx_ipd_red_port_enable
 *
 * Set the pass-drop marks for qos level.
 *
 */
union cvmx_ipd_red_port_enable {
	u64 u64;
	struct cvmx_ipd_red_port_enable_s {
		u64 prb_dly : 14;
		u64 avg_dly : 14;
		u64 prt_enb : 36;
	} s;
	struct cvmx_ipd_red_port_enable_s cn30xx;
	struct cvmx_ipd_red_port_enable_s cn31xx;
	struct cvmx_ipd_red_port_enable_s cn38xx;
	struct cvmx_ipd_red_port_enable_s cn38xxp2;
	struct cvmx_ipd_red_port_enable_s cn50xx;
	struct cvmx_ipd_red_port_enable_s cn52xx;
	struct cvmx_ipd_red_port_enable_s cn52xxp1;
	struct cvmx_ipd_red_port_enable_s cn56xx;
	struct cvmx_ipd_red_port_enable_s cn56xxp1;
	struct cvmx_ipd_red_port_enable_s cn58xx;
	struct cvmx_ipd_red_port_enable_s cn58xxp1;
	struct cvmx_ipd_red_port_enable_s cn61xx;
	struct cvmx_ipd_red_port_enable_s cn63xx;
	struct cvmx_ipd_red_port_enable_s cn63xxp1;
	struct cvmx_ipd_red_port_enable_s cn66xx;
	struct cvmx_ipd_red_port_enable_s cn70xx;
	struct cvmx_ipd_red_port_enable_s cn70xxp1;
	struct cvmx_ipd_red_port_enable_s cnf71xx;
};

typedef union cvmx_ipd_red_port_enable cvmx_ipd_red_port_enable_t;

/**
 * cvmx_ipd_red_port_enable2
 *
 * Set the pass-drop marks for qos level.
 *
 */
union cvmx_ipd_red_port_enable2 {
	u64 u64;
	struct cvmx_ipd_red_port_enable2_s {
		u64 reserved_12_63 : 52;
		u64 prt_enb : 12;
	} s;
	struct cvmx_ipd_red_port_enable2_cn52xx {
		u64 reserved_4_63 : 60;
		u64 prt_enb : 4;
	} cn52xx;
	struct cvmx_ipd_red_port_enable2_cn52xx cn52xxp1;
	struct cvmx_ipd_red_port_enable2_cn52xx cn56xx;
	struct cvmx_ipd_red_port_enable2_cn52xx cn56xxp1;
	struct cvmx_ipd_red_port_enable2_s cn61xx;
	struct cvmx_ipd_red_port_enable2_cn63xx {
		u64 reserved_8_63 : 56;
		u64 prt_enb : 8;
	} cn63xx;
	struct cvmx_ipd_red_port_enable2_cn63xx cn63xxp1;
	struct cvmx_ipd_red_port_enable2_s cn66xx;
	struct cvmx_ipd_red_port_enable2_s cn70xx;
	struct cvmx_ipd_red_port_enable2_s cn70xxp1;
	struct cvmx_ipd_red_port_enable2_s cnf71xx;
};

typedef union cvmx_ipd_red_port_enable2 cvmx_ipd_red_port_enable2_t;

/**
 * cvmx_ipd_red_que#_param
 *
 * Value control the Passing and Dropping of packets by the red engine for QOS Level-0.
 *
 */
union cvmx_ipd_red_quex_param {
	u64 u64;
	struct cvmx_ipd_red_quex_param_s {
		u64 reserved_49_63 : 15;
		u64 use_pcnt : 1;
		u64 new_con : 8;
		u64 avg_con : 8;
		u64 prb_con : 32;
	} s;
	struct cvmx_ipd_red_quex_param_s cn30xx;
	struct cvmx_ipd_red_quex_param_s cn31xx;
	struct cvmx_ipd_red_quex_param_s cn38xx;
	struct cvmx_ipd_red_quex_param_s cn38xxp2;
	struct cvmx_ipd_red_quex_param_s cn50xx;
	struct cvmx_ipd_red_quex_param_s cn52xx;
	struct cvmx_ipd_red_quex_param_s cn52xxp1;
	struct cvmx_ipd_red_quex_param_s cn56xx;
	struct cvmx_ipd_red_quex_param_s cn56xxp1;
	struct cvmx_ipd_red_quex_param_s cn58xx;
	struct cvmx_ipd_red_quex_param_s cn58xxp1;
	struct cvmx_ipd_red_quex_param_s cn61xx;
	struct cvmx_ipd_red_quex_param_s cn63xx;
	struct cvmx_ipd_red_quex_param_s cn63xxp1;
	struct cvmx_ipd_red_quex_param_s cn66xx;
	struct cvmx_ipd_red_quex_param_s cn68xx;
	struct cvmx_ipd_red_quex_param_s cn68xxp1;
	struct cvmx_ipd_red_quex_param_s cn70xx;
	struct cvmx_ipd_red_quex_param_s cn70xxp1;
	struct cvmx_ipd_red_quex_param_s cnf71xx;
};

typedef union cvmx_ipd_red_quex_param cvmx_ipd_red_quex_param_t;

/**
 * cvmx_ipd_req_wgt
 *
 * IPD_REQ_WGT = IPD REQ weights
 *
 * There are 8 devices that can request to send packet traffic to the IPD. These weights are used for the Weighted Round Robin
 * grant generated by the IPD to requestors.
 */
union cvmx_ipd_req_wgt {
	u64 u64;
	struct cvmx_ipd_req_wgt_s {
		u64 wgt7 : 8;
		u64 wgt6 : 8;
		u64 wgt5 : 8;
		u64 wgt4 : 8;
		u64 wgt3 : 8;
		u64 wgt2 : 8;
		u64 wgt1 : 8;
		u64 wgt0 : 8;
	} s;
	struct cvmx_ipd_req_wgt_s cn68xx;
};

typedef union cvmx_ipd_req_wgt cvmx_ipd_req_wgt_t;

/**
 * cvmx_ipd_sub_port_bp_page_cnt
 *
 * Will add the value to the indicated port count register, the number of pages supplied. The
 * value added should
 * be the 2's complement of the value that needs to be subtracted. Users add 2's complement
 * values to the
 * port-mbuf-count register to return (lower the count) mbufs to the counter in order to avoid
 * port-level
 * backpressure being applied to the port. Backpressure is applied when the MBUF used count of a
 * port exceeds the
 * value in the IPD_PORTX_BP_PAGE_CNT, IPD_PORTX_BP_PAGE_CNT2, and IPD_PORTX_BP_PAGE_CNT3.
 * This register can't be written from the PCI via a window write.
 */
union cvmx_ipd_sub_port_bp_page_cnt {
	u64 u64;
	struct cvmx_ipd_sub_port_bp_page_cnt_s {
		u64 reserved_31_63 : 33;
		u64 port : 6;
		u64 page_cnt : 25;
	} s;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn30xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn31xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn38xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn38xxp2;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn50xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn52xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn52xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn56xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn56xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn58xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn58xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn61xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn63xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn63xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn66xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn68xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn68xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn70xx;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cn70xxp1;
	struct cvmx_ipd_sub_port_bp_page_cnt_s cnf71xx;
};

typedef union cvmx_ipd_sub_port_bp_page_cnt cvmx_ipd_sub_port_bp_page_cnt_t;

/**
 * cvmx_ipd_sub_port_fcs
 *
 * When set '1' the port corresponding to the bit set will subtract 4 bytes from the end of
 * the packet.
 */
union cvmx_ipd_sub_port_fcs {
	u64 u64;
	struct cvmx_ipd_sub_port_fcs_s {
		u64 reserved_40_63 : 24;
		u64 port_bit2 : 4;
		u64 reserved_32_35 : 4;
		u64 port_bit : 32;
	} s;
	struct cvmx_ipd_sub_port_fcs_cn30xx {
		u64 reserved_3_63 : 61;
		u64 port_bit : 3;
	} cn30xx;
	struct cvmx_ipd_sub_port_fcs_cn30xx cn31xx;
	struct cvmx_ipd_sub_port_fcs_cn38xx {
		u64 reserved_32_63 : 32;
		u64 port_bit : 32;
	} cn38xx;
	struct cvmx_ipd_sub_port_fcs_cn38xx cn38xxp2;
	struct cvmx_ipd_sub_port_fcs_cn30xx cn50xx;
	struct cvmx_ipd_sub_port_fcs_s cn52xx;
	struct cvmx_ipd_sub_port_fcs_s cn52xxp1;
	struct cvmx_ipd_sub_port_fcs_s cn56xx;
	struct cvmx_ipd_sub_port_fcs_s cn56xxp1;
	struct cvmx_ipd_sub_port_fcs_cn38xx cn58xx;
	struct cvmx_ipd_sub_port_fcs_cn38xx cn58xxp1;
	struct cvmx_ipd_sub_port_fcs_s cn61xx;
	struct cvmx_ipd_sub_port_fcs_s cn63xx;
	struct cvmx_ipd_sub_port_fcs_s cn63xxp1;
	struct cvmx_ipd_sub_port_fcs_s cn66xx;
	struct cvmx_ipd_sub_port_fcs_s cn70xx;
	struct cvmx_ipd_sub_port_fcs_s cn70xxp1;
	struct cvmx_ipd_sub_port_fcs_s cnf71xx;
};

typedef union cvmx_ipd_sub_port_fcs cvmx_ipd_sub_port_fcs_t;

/**
 * cvmx_ipd_sub_port_qos_cnt
 *
 * Will add the value (CNT) to the indicated Port-QOS register (PORT_QOS). The value added must
 * be
 * be the 2's complement of the value that needs to be subtracted.
 */
union cvmx_ipd_sub_port_qos_cnt {
	u64 u64;
	struct cvmx_ipd_sub_port_qos_cnt_s {
		u64 reserved_41_63 : 23;
		u64 port_qos : 9;
		u64 cnt : 32;
	} s;
	struct cvmx_ipd_sub_port_qos_cnt_s cn52xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn52xxp1;
	struct cvmx_ipd_sub_port_qos_cnt_s cn56xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn56xxp1;
	struct cvmx_ipd_sub_port_qos_cnt_s cn61xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn63xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn63xxp1;
	struct cvmx_ipd_sub_port_qos_cnt_s cn66xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn68xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn68xxp1;
	struct cvmx_ipd_sub_port_qos_cnt_s cn70xx;
	struct cvmx_ipd_sub_port_qos_cnt_s cn70xxp1;
	struct cvmx_ipd_sub_port_qos_cnt_s cnf71xx;
};

typedef union cvmx_ipd_sub_port_qos_cnt cvmx_ipd_sub_port_qos_cnt_t;

/**
 * cvmx_ipd_wqe_fpa_queue
 *
 * Which FPA Queue (0-7) to fetch page-pointers from for WQE's
 *
 */
union cvmx_ipd_wqe_fpa_queue {
	u64 u64;
	struct cvmx_ipd_wqe_fpa_queue_s {
		u64 reserved_3_63 : 61;
		u64 wqe_pool : 3;
	} s;
	struct cvmx_ipd_wqe_fpa_queue_s cn30xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn31xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn38xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn38xxp2;
	struct cvmx_ipd_wqe_fpa_queue_s cn50xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn52xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn52xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cn56xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn56xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cn58xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn58xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cn61xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn63xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn63xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cn66xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn68xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn68xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cn70xx;
	struct cvmx_ipd_wqe_fpa_queue_s cn70xxp1;
	struct cvmx_ipd_wqe_fpa_queue_s cnf71xx;
};

typedef union cvmx_ipd_wqe_fpa_queue cvmx_ipd_wqe_fpa_queue_t;

/**
 * cvmx_ipd_wqe_ptr_valid
 *
 * The value of the WQE-pointer fetched and in the valid register.
 *
 */
union cvmx_ipd_wqe_ptr_valid {
	u64 u64;
	struct cvmx_ipd_wqe_ptr_valid_s {
		u64 reserved_29_63 : 35;
		u64 ptr : 29;
	} s;
	struct cvmx_ipd_wqe_ptr_valid_s cn30xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn31xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn38xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn50xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn52xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn52xxp1;
	struct cvmx_ipd_wqe_ptr_valid_s cn56xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn56xxp1;
	struct cvmx_ipd_wqe_ptr_valid_s cn58xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn58xxp1;
	struct cvmx_ipd_wqe_ptr_valid_s cn61xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn63xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn63xxp1;
	struct cvmx_ipd_wqe_ptr_valid_s cn66xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn70xx;
	struct cvmx_ipd_wqe_ptr_valid_s cn70xxp1;
	struct cvmx_ipd_wqe_ptr_valid_s cnf71xx;
};

typedef union cvmx_ipd_wqe_ptr_valid cvmx_ipd_wqe_ptr_valid_t;

#endif
