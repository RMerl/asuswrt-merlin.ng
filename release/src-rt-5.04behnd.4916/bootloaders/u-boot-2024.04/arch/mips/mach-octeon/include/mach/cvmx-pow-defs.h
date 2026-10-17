/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pow.
 */

#ifndef __CVMX_POW_DEFS_H__
#define __CVMX_POW_DEFS_H__

#define CVMX_POW_BIST_STAT	     (0x00016700000003F8ull)
#define CVMX_POW_DS_PC		     (0x0001670000000398ull)
#define CVMX_POW_ECC_ERR	     (0x0001670000000218ull)
#define CVMX_POW_IQ_CNTX(offset)     (0x0001670000000340ull + ((offset) & 7) * 8)
#define CVMX_POW_IQ_COM_CNT	     (0x0001670000000388ull)
#define CVMX_POW_IQ_INT		     (0x0001670000000238ull)
#define CVMX_POW_IQ_INT_EN	     (0x0001670000000240ull)
#define CVMX_POW_IQ_THRX(offset)     (0x00016700000003A0ull + ((offset) & 7) * 8)
#define CVMX_POW_NOS_CNT	     (0x0001670000000228ull)
#define CVMX_POW_NW_TIM		     (0x0001670000000210ull)
#define CVMX_POW_PF_RST_MSK	     (0x0001670000000230ull)
#define CVMX_POW_PP_GRP_MSKX(offset) (0x0001670000000000ull + ((offset) & 15) * 8)
#define CVMX_POW_QOS_RNDX(offset)    (0x00016700000001C0ull + ((offset) & 7) * 8)
#define CVMX_POW_QOS_THRX(offset)    (0x0001670000000180ull + ((offset) & 7) * 8)
#define CVMX_POW_TS_PC		     (0x0001670000000390ull)
#define CVMX_POW_WA_COM_PC	     (0x0001670000000380ull)
#define CVMX_POW_WA_PCX(offset)	     (0x0001670000000300ull + ((offset) & 7) * 8)
#define CVMX_POW_WQ_INT		     (0x0001670000000200ull)
#define CVMX_POW_WQ_INT_CNTX(offset) (0x0001670000000100ull + ((offset) & 15) * 8)
#define CVMX_POW_WQ_INT_PC	     (0x0001670000000208ull)
#define CVMX_POW_WQ_INT_THRX(offset) (0x0001670000000080ull + ((offset) & 15) * 8)
#define CVMX_POW_WS_PCX(offset)	     (0x0001670000000280ull + ((offset) & 15) * 8)

/**
 * cvmx_pow_bist_stat
 *
 * Contains the BIST status for the POW memories ('0' = pass, '1' = fail).
 *
 */
union cvmx_pow_bist_stat {
	u64 u64;
	struct cvmx_pow_bist_stat_s {
		u64 reserved_32_63 : 32;
		u64 pp : 16;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_pow_bist_stat_cn30xx {
		u64 reserved_17_63 : 47;
		u64 pp : 1;
		u64 reserved_9_15 : 7;
		u64 cam : 1;
		u64 nbt1 : 1;
		u64 nbt0 : 1;
		u64 index : 1;
		u64 fidx : 1;
		u64 nbr1 : 1;
		u64 nbr0 : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn30xx;
	struct cvmx_pow_bist_stat_cn31xx {
		u64 reserved_18_63 : 46;
		u64 pp : 2;
		u64 reserved_9_15 : 7;
		u64 cam : 1;
		u64 nbt1 : 1;
		u64 nbt0 : 1;
		u64 index : 1;
		u64 fidx : 1;
		u64 nbr1 : 1;
		u64 nbr0 : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn31xx;
	struct cvmx_pow_bist_stat_cn38xx {
		u64 reserved_32_63 : 32;
		u64 pp : 16;
		u64 reserved_10_15 : 6;
		u64 cam : 1;
		u64 nbt : 1;
		u64 index : 1;
		u64 fidx : 1;
		u64 nbr1 : 1;
		u64 nbr0 : 1;
		u64 pend1 : 1;
		u64 pend0 : 1;
		u64 adr1 : 1;
		u64 adr0 : 1;
	} cn38xx;
	struct cvmx_pow_bist_stat_cn38xx cn38xxp2;
	struct cvmx_pow_bist_stat_cn31xx cn50xx;
	struct cvmx_pow_bist_stat_cn52xx {
		u64 reserved_20_63 : 44;
		u64 pp : 4;
		u64 reserved_9_15 : 7;
		u64 cam : 1;
		u64 nbt1 : 1;
		u64 nbt0 : 1;
		u64 index : 1;
		u64 fidx : 1;
		u64 nbr1 : 1;
		u64 nbr0 : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn52xx;
	struct cvmx_pow_bist_stat_cn52xx cn52xxp1;
	struct cvmx_pow_bist_stat_cn56xx {
		u64 reserved_28_63 : 36;
		u64 pp : 12;
		u64 reserved_10_15 : 6;
		u64 cam : 1;
		u64 nbt : 1;
		u64 index : 1;
		u64 fidx : 1;
		u64 nbr1 : 1;
		u64 nbr0 : 1;
		u64 pend1 : 1;
		u64 pend0 : 1;
		u64 adr1 : 1;
		u64 adr0 : 1;
	} cn56xx;
	struct cvmx_pow_bist_stat_cn56xx cn56xxp1;
	struct cvmx_pow_bist_stat_cn38xx cn58xx;
	struct cvmx_pow_bist_stat_cn38xx cn58xxp1;
	struct cvmx_pow_bist_stat_cn61xx {
		u64 reserved_20_63 : 44;
		u64 pp : 4;
		u64 reserved_12_15 : 4;
		u64 cam : 1;
		u64 nbr : 3;
		u64 nbt : 4;
		u64 index : 1;
		u64 fidx : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn61xx;
	struct cvmx_pow_bist_stat_cn63xx {
		u64 reserved_22_63 : 42;
		u64 pp : 6;
		u64 reserved_12_15 : 4;
		u64 cam : 1;
		u64 nbr : 3;
		u64 nbt : 4;
		u64 index : 1;
		u64 fidx : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn63xx;
	struct cvmx_pow_bist_stat_cn63xx cn63xxp1;
	struct cvmx_pow_bist_stat_cn66xx {
		u64 reserved_26_63 : 38;
		u64 pp : 10;
		u64 reserved_12_15 : 4;
		u64 cam : 1;
		u64 nbr : 3;
		u64 nbt : 4;
		u64 index : 1;
		u64 fidx : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn66xx;
	struct cvmx_pow_bist_stat_cn70xx {
		u64 reserved_12_63 : 52;
		u64 cam : 1;
		u64 reserved_10_10 : 1;
		u64 nbr : 2;
		u64 reserved_6_7 : 2;
		u64 nbt : 2;
		u64 index : 1;
		u64 fidx : 1;
		u64 pend : 1;
		u64 adr : 1;
	} cn70xx;
	struct cvmx_pow_bist_stat_cn70xx cn70xxp1;
	struct cvmx_pow_bist_stat_cn61xx cnf71xx;
};

typedef union cvmx_pow_bist_stat cvmx_pow_bist_stat_t;

/**
 * cvmx_pow_ds_pc
 *
 * Counts the number of de-schedule requests.  Write to clear.
 *
 */
union cvmx_pow_ds_pc {
	u64 u64;
	struct cvmx_pow_ds_pc_s {
		u64 reserved_32_63 : 32;
		u64 ds_pc : 32;
	} s;
	struct cvmx_pow_ds_pc_s cn30xx;
	struct cvmx_pow_ds_pc_s cn31xx;
	struct cvmx_pow_ds_pc_s cn38xx;
	struct cvmx_pow_ds_pc_s cn38xxp2;
	struct cvmx_pow_ds_pc_s cn50xx;
	struct cvmx_pow_ds_pc_s cn52xx;
	struct cvmx_pow_ds_pc_s cn52xxp1;
	struct cvmx_pow_ds_pc_s cn56xx;
	struct cvmx_pow_ds_pc_s cn56xxp1;
	struct cvmx_pow_ds_pc_s cn58xx;
	struct cvmx_pow_ds_pc_s cn58xxp1;
	struct cvmx_pow_ds_pc_s cn61xx;
	struct cvmx_pow_ds_pc_s cn63xx;
	struct cvmx_pow_ds_pc_s cn63xxp1;
	struct cvmx_pow_ds_pc_s cn66xx;
	struct cvmx_pow_ds_pc_s cn70xx;
	struct cvmx_pow_ds_pc_s cn70xxp1;
	struct cvmx_pow_ds_pc_s cnf71xx;
};

typedef union cvmx_pow_ds_pc cvmx_pow_ds_pc_t;

/**
 * cvmx_pow_ecc_err
 *
 * Contains the single and double error bits and the corresponding interrupt enables for the ECC-
 * protected POW index memory.  Also contains the syndrome value in the event of an ECC error.
 * Also contains the remote pointer error bit and interrupt enable.  RPE is set when the POW
 * detected
 * corruption on one or more of the input queue lists in L2/DRAM (POW's local copy of the tail
 * pointer
 * for the L2/DRAM input queue did not match the last entry on the the list).   This is caused by
 * L2/DRAM corruption, and is generally a fatal error because it likely caused POW to load bad
 * work
 * queue entries.
 * This register also contains the illegal operation error bits and the corresponding interrupt
 * enables as follows:
 *  <0> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP from PP in NULL_NULL state
 *  <1> Received SWTAG/SWTAG_DESCH/DESCH/UPD_WQP from PP in NULL state
 *  <2> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/GET_WORK from PP with pending tag switch to ORDERED
 * or ATOMIC
 *  <3> Received SWTAG/SWTAG_FULL/SWTAG_DESCH from PP with tag specified as NULL_NULL
 *  <4> Received SWTAG_FULL/SWTAG_DESCH from PP with tag specified as NULL
 *  <5> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/NULL_RD from PP with
 * GET_WORK pending
 *  <6> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/NULL_RD from PP with NULL_RD
 * pending
 *  <7> Received CLR_NSCHED from PP with SWTAG_DESCH/DESCH/CLR_NSCHED pending
 *  <8> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/NULL_RD from PP with
 * CLR_NSCHED pending
 *  <9> Received illegal opcode
 * <10> Received ADD_WORK with tag specified as NULL_NULL
 * <11> Received DBG load from PP with DBG load pending
 * <12> Received CSR load from PP with CSR load pending
 */
union cvmx_pow_ecc_err {
	u64 u64;
	struct cvmx_pow_ecc_err_s {
		u64 reserved_45_63 : 19;
		u64 iop_ie : 13;
		u64 reserved_29_31 : 3;
		u64 iop : 13;
		u64 reserved_14_15 : 2;
		u64 rpe_ie : 1;
		u64 rpe : 1;
		u64 reserved_9_11 : 3;
		u64 syn : 5;
		u64 dbe_ie : 1;
		u64 sbe_ie : 1;
		u64 dbe : 1;
		u64 sbe : 1;
	} s;
	struct cvmx_pow_ecc_err_s cn30xx;
	struct cvmx_pow_ecc_err_cn31xx {
		u64 reserved_14_63 : 50;
		u64 rpe_ie : 1;
		u64 rpe : 1;
		u64 reserved_9_11 : 3;
		u64 syn : 5;
		u64 dbe_ie : 1;
		u64 sbe_ie : 1;
		u64 dbe : 1;
		u64 sbe : 1;
	} cn31xx;
	struct cvmx_pow_ecc_err_s cn38xx;
	struct cvmx_pow_ecc_err_cn31xx cn38xxp2;
	struct cvmx_pow_ecc_err_s cn50xx;
	struct cvmx_pow_ecc_err_s cn52xx;
	struct cvmx_pow_ecc_err_s cn52xxp1;
	struct cvmx_pow_ecc_err_s cn56xx;
	struct cvmx_pow_ecc_err_s cn56xxp1;
	struct cvmx_pow_ecc_err_s cn58xx;
	struct cvmx_pow_ecc_err_s cn58xxp1;
	struct cvmx_pow_ecc_err_s cn61xx;
	struct cvmx_pow_ecc_err_s cn63xx;
	struct cvmx_pow_ecc_err_s cn63xxp1;
	struct cvmx_pow_ecc_err_s cn66xx;
	struct cvmx_pow_ecc_err_s cn70xx;
	struct cvmx_pow_ecc_err_s cn70xxp1;
	struct cvmx_pow_ecc_err_s cnf71xx;
};

typedef union cvmx_pow_ecc_err cvmx_pow_ecc_err_t;

/**
 * cvmx_pow_iq_cnt#
 *
 * Contains a read-only count of the number of work queue entries for each QOS level.
 *
 */
union cvmx_pow_iq_cntx {
	u64 u64;
	struct cvmx_pow_iq_cntx_s {
		u64 reserved_32_63 : 32;
		u64 iq_cnt : 32;
	} s;
	struct cvmx_pow_iq_cntx_s cn30xx;
	struct cvmx_pow_iq_cntx_s cn31xx;
	struct cvmx_pow_iq_cntx_s cn38xx;
	struct cvmx_pow_iq_cntx_s cn38xxp2;
	struct cvmx_pow_iq_cntx_s cn50xx;
	struct cvmx_pow_iq_cntx_s cn52xx;
	struct cvmx_pow_iq_cntx_s cn52xxp1;
	struct cvmx_pow_iq_cntx_s cn56xx;
	struct cvmx_pow_iq_cntx_s cn56xxp1;
	struct cvmx_pow_iq_cntx_s cn58xx;
	struct cvmx_pow_iq_cntx_s cn58xxp1;
	struct cvmx_pow_iq_cntx_s cn61xx;
	struct cvmx_pow_iq_cntx_s cn63xx;
	struct cvmx_pow_iq_cntx_s cn63xxp1;
	struct cvmx_pow_iq_cntx_s cn66xx;
	struct cvmx_pow_iq_cntx_s cn70xx;
	struct cvmx_pow_iq_cntx_s cn70xxp1;
	struct cvmx_pow_iq_cntx_s cnf71xx;
};

typedef union cvmx_pow_iq_cntx cvmx_pow_iq_cntx_t;

/**
 * cvmx_pow_iq_com_cnt
 *
 * Contains a read-only count of the total number of work queue entries in all QOS levels.
 *
 */
union cvmx_pow_iq_com_cnt {
	u64 u64;
	struct cvmx_pow_iq_com_cnt_s {
		u64 reserved_32_63 : 32;
		u64 iq_cnt : 32;
	} s;
	struct cvmx_pow_iq_com_cnt_s cn30xx;
	struct cvmx_pow_iq_com_cnt_s cn31xx;
	struct cvmx_pow_iq_com_cnt_s cn38xx;
	struct cvmx_pow_iq_com_cnt_s cn38xxp2;
	struct cvmx_pow_iq_com_cnt_s cn50xx;
	struct cvmx_pow_iq_com_cnt_s cn52xx;
	struct cvmx_pow_iq_com_cnt_s cn52xxp1;
	struct cvmx_pow_iq_com_cnt_s cn56xx;
	struct cvmx_pow_iq_com_cnt_s cn56xxp1;
	struct cvmx_pow_iq_com_cnt_s cn58xx;
	struct cvmx_pow_iq_com_cnt_s cn58xxp1;
	struct cvmx_pow_iq_com_cnt_s cn61xx;
	struct cvmx_pow_iq_com_cnt_s cn63xx;
	struct cvmx_pow_iq_com_cnt_s cn63xxp1;
	struct cvmx_pow_iq_com_cnt_s cn66xx;
	struct cvmx_pow_iq_com_cnt_s cn70xx;
	struct cvmx_pow_iq_com_cnt_s cn70xxp1;
	struct cvmx_pow_iq_com_cnt_s cnf71xx;
};

typedef union cvmx_pow_iq_com_cnt cvmx_pow_iq_com_cnt_t;

/**
 * cvmx_pow_iq_int
 *
 * "Contains the bits (1 per QOS level) that can trigger the input queue interrupt.  An IQ_INT
 * bit
 * will be set if POW_IQ_CNT#QOS# changes and the resulting value is equal to POW_IQ_THR#QOS#."
 */
union cvmx_pow_iq_int {
	u64 u64;
	struct cvmx_pow_iq_int_s {
		u64 reserved_8_63 : 56;
		u64 iq_int : 8;
	} s;
	struct cvmx_pow_iq_int_s cn52xx;
	struct cvmx_pow_iq_int_s cn52xxp1;
	struct cvmx_pow_iq_int_s cn56xx;
	struct cvmx_pow_iq_int_s cn56xxp1;
	struct cvmx_pow_iq_int_s cn61xx;
	struct cvmx_pow_iq_int_s cn63xx;
	struct cvmx_pow_iq_int_s cn63xxp1;
	struct cvmx_pow_iq_int_s cn66xx;
	struct cvmx_pow_iq_int_s cn70xx;
	struct cvmx_pow_iq_int_s cn70xxp1;
	struct cvmx_pow_iq_int_s cnf71xx;
};

typedef union cvmx_pow_iq_int cvmx_pow_iq_int_t;

/**
 * cvmx_pow_iq_int_en
 *
 * Contains the bits (1 per QOS level) that enable the input queue interrupt.
 *
 */
union cvmx_pow_iq_int_en {
	u64 u64;
	struct cvmx_pow_iq_int_en_s {
		u64 reserved_8_63 : 56;
		u64 int_en : 8;
	} s;
	struct cvmx_pow_iq_int_en_s cn52xx;
	struct cvmx_pow_iq_int_en_s cn52xxp1;
	struct cvmx_pow_iq_int_en_s cn56xx;
	struct cvmx_pow_iq_int_en_s cn56xxp1;
	struct cvmx_pow_iq_int_en_s cn61xx;
	struct cvmx_pow_iq_int_en_s cn63xx;
	struct cvmx_pow_iq_int_en_s cn63xxp1;
	struct cvmx_pow_iq_int_en_s cn66xx;
	struct cvmx_pow_iq_int_en_s cn70xx;
	struct cvmx_pow_iq_int_en_s cn70xxp1;
	struct cvmx_pow_iq_int_en_s cnf71xx;
};

typedef union cvmx_pow_iq_int_en cvmx_pow_iq_int_en_t;

/**
 * cvmx_pow_iq_thr#
 *
 * Threshold value for triggering input queue interrupts.
 *
 */
union cvmx_pow_iq_thrx {
	u64 u64;
	struct cvmx_pow_iq_thrx_s {
		u64 reserved_32_63 : 32;
		u64 iq_thr : 32;
	} s;
	struct cvmx_pow_iq_thrx_s cn52xx;
	struct cvmx_pow_iq_thrx_s cn52xxp1;
	struct cvmx_pow_iq_thrx_s cn56xx;
	struct cvmx_pow_iq_thrx_s cn56xxp1;
	struct cvmx_pow_iq_thrx_s cn61xx;
	struct cvmx_pow_iq_thrx_s cn63xx;
	struct cvmx_pow_iq_thrx_s cn63xxp1;
	struct cvmx_pow_iq_thrx_s cn66xx;
	struct cvmx_pow_iq_thrx_s cn70xx;
	struct cvmx_pow_iq_thrx_s cn70xxp1;
	struct cvmx_pow_iq_thrx_s cnf71xx;
};

typedef union cvmx_pow_iq_thrx cvmx_pow_iq_thrx_t;

/**
 * cvmx_pow_nos_cnt
 *
 * Contains the number of work queue entries on the no-schedule list.
 *
 */
union cvmx_pow_nos_cnt {
	u64 u64;
	struct cvmx_pow_nos_cnt_s {
		u64 reserved_12_63 : 52;
		u64 nos_cnt : 12;
	} s;
	struct cvmx_pow_nos_cnt_cn30xx {
		u64 reserved_7_63 : 57;
		u64 nos_cnt : 7;
	} cn30xx;
	struct cvmx_pow_nos_cnt_cn31xx {
		u64 reserved_9_63 : 55;
		u64 nos_cnt : 9;
	} cn31xx;
	struct cvmx_pow_nos_cnt_s cn38xx;
	struct cvmx_pow_nos_cnt_s cn38xxp2;
	struct cvmx_pow_nos_cnt_cn31xx cn50xx;
	struct cvmx_pow_nos_cnt_cn52xx {
		u64 reserved_10_63 : 54;
		u64 nos_cnt : 10;
	} cn52xx;
	struct cvmx_pow_nos_cnt_cn52xx cn52xxp1;
	struct cvmx_pow_nos_cnt_s cn56xx;
	struct cvmx_pow_nos_cnt_s cn56xxp1;
	struct cvmx_pow_nos_cnt_s cn58xx;
	struct cvmx_pow_nos_cnt_s cn58xxp1;
	struct cvmx_pow_nos_cnt_cn52xx cn61xx;
	struct cvmx_pow_nos_cnt_cn63xx {
		u64 reserved_11_63 : 53;
		u64 nos_cnt : 11;
	} cn63xx;
	struct cvmx_pow_nos_cnt_cn63xx cn63xxp1;
	struct cvmx_pow_nos_cnt_cn63xx cn66xx;
	struct cvmx_pow_nos_cnt_cn52xx cn70xx;
	struct cvmx_pow_nos_cnt_cn52xx cn70xxp1;
	struct cvmx_pow_nos_cnt_cn52xx cnf71xx;
};

typedef union cvmx_pow_nos_cnt cvmx_pow_nos_cnt_t;

/**
 * cvmx_pow_nw_tim
 *
 * Sets the minimum period for a new work request timeout.  Period is specified in n-1 notation
 * where the increment value is 1024 clock cycles.  Thus, a value of 0x0 in this register
 * translates
 * to 1024 cycles, 0x1 translates to 2048 cycles, 0x2 translates to 3072 cycles, etc...  Note:
 * the
 * maximum period for a new work request timeout is 2 times the minimum period.  Note: the new
 * work
 * request timeout counter is reset when this register is written.
 * There are two new work request timeout cases:
 * - WAIT bit clear.  The new work request can timeout if the timer expires before the pre-fetch
 *   engine has reached the end of all work queues.  This can occur if the executable work queue
 *   entry is deep in the queue and the pre-fetch engine is subject to many resets (i.e. high
 * switch,
 *   de-schedule, or new work load from other PP's).  Thus, it is possible for a PP to receive a
 * work
 *   response with the NO_WORK bit set even though there was at least one executable entry in the
 *   work queues.  The other (and typical) scenario for receiving a NO_WORK response with the
 * WAIT
 *   bit clear is that the pre-fetch engine has reached the end of all work queues without
 * finding
 *   executable work.
 * - WAIT bit set.  The new work request can timeout if the timer expires before the pre-fetch
 *   engine has found executable work.  In this case, the only scenario where the PP will receive
 * a
 *   work response with the NO_WORK bit set is if the timer expires.  Note: it is still possible
 * for
 *   a PP to receive a NO_WORK response even though there was at least one executable entry in
 * the
 *   work queues.
 * In either case, it's important to note that switches and de-schedules are higher priority
 * operations that can cause the pre-fetch engine to reset.  Thus in a system with many switches
 * or
 * de-schedules occurring, it's possible for the new work timer to expire (resulting in NO_WORK
 * responses) before the pre-fetch engine is able to get very deep into the work queues.
 */
union cvmx_pow_nw_tim {
	u64 u64;
	struct cvmx_pow_nw_tim_s {
		u64 reserved_10_63 : 54;
		u64 nw_tim : 10;
	} s;
	struct cvmx_pow_nw_tim_s cn30xx;
	struct cvmx_pow_nw_tim_s cn31xx;
	struct cvmx_pow_nw_tim_s cn38xx;
	struct cvmx_pow_nw_tim_s cn38xxp2;
	struct cvmx_pow_nw_tim_s cn50xx;
	struct cvmx_pow_nw_tim_s cn52xx;
	struct cvmx_pow_nw_tim_s cn52xxp1;
	struct cvmx_pow_nw_tim_s cn56xx;
	struct cvmx_pow_nw_tim_s cn56xxp1;
	struct cvmx_pow_nw_tim_s cn58xx;
	struct cvmx_pow_nw_tim_s cn58xxp1;
	struct cvmx_pow_nw_tim_s cn61xx;
	struct cvmx_pow_nw_tim_s cn63xx;
	struct cvmx_pow_nw_tim_s cn63xxp1;
	struct cvmx_pow_nw_tim_s cn66xx;
	struct cvmx_pow_nw_tim_s cn70xx;
	struct cvmx_pow_nw_tim_s cn70xxp1;
	struct cvmx_pow_nw_tim_s cnf71xx;
};

typedef union cvmx_pow_nw_tim cvmx_pow_nw_tim_t;

/**
 * cvmx_pow_pf_rst_msk
 *
 * Resets the work prefetch engine when work is stored in an internal buffer (either when the add
 * work arrives or when the work is reloaded from an external buffer) for an enabled QOS level
 * (1 bit per QOS level).
 */
union cvmx_pow_pf_rst_msk {
	u64 u64;
	struct cvmx_pow_pf_rst_msk_s {
		u64 reserved_8_63 : 56;
		u64 rst_msk : 8;
	} s;
	struct cvmx_pow_pf_rst_msk_s cn50xx;
	struct cvmx_pow_pf_rst_msk_s cn52xx;
	struct cvmx_pow_pf_rst_msk_s cn52xxp1;
	struct cvmx_pow_pf_rst_msk_s cn56xx;
	struct cvmx_pow_pf_rst_msk_s cn56xxp1;
	struct cvmx_pow_pf_rst_msk_s cn58xx;
	struct cvmx_pow_pf_rst_msk_s cn58xxp1;
	struct cvmx_pow_pf_rst_msk_s cn61xx;
	struct cvmx_pow_pf_rst_msk_s cn63xx;
	struct cvmx_pow_pf_rst_msk_s cn63xxp1;
	struct cvmx_pow_pf_rst_msk_s cn66xx;
	struct cvmx_pow_pf_rst_msk_s cn70xx;
	struct cvmx_pow_pf_rst_msk_s cn70xxp1;
	struct cvmx_pow_pf_rst_msk_s cnf71xx;
};

typedef union cvmx_pow_pf_rst_msk cvmx_pow_pf_rst_msk_t;

/**
 * cvmx_pow_pp_grp_msk#
 *
 * Selects which group(s) a PP belongs to.  A '1' in any bit position sets the PP's membership in
 * the corresponding group.  A value of 0x0 will prevent the PP from receiving new work.  Note:
 * disabled or non-existent PP's should have this field set to 0xffff (the reset value) in order
 * to
 * maximize POW performance.
 * Also contains the QOS level priorities for each PP.  0x0 is highest priority, and 0x7 the
 * lowest.
 * Setting the priority to 0xf will prevent that PP from receiving work from that QOS level.
 * Priority values 0x8 through 0xe are reserved and should not be used.  For a given PP,
 * priorities
 * should begin at 0x0 and remain contiguous throughout the range.
 */
union cvmx_pow_pp_grp_mskx {
	u64 u64;
	struct cvmx_pow_pp_grp_mskx_s {
		u64 reserved_48_63 : 16;
		u64 qos7_pri : 4;
		u64 qos6_pri : 4;
		u64 qos5_pri : 4;
		u64 qos4_pri : 4;
		u64 qos3_pri : 4;
		u64 qos2_pri : 4;
		u64 qos1_pri : 4;
		u64 qos0_pri : 4;
		u64 grp_msk : 16;
	} s;
	struct cvmx_pow_pp_grp_mskx_cn30xx {
		u64 reserved_16_63 : 48;
		u64 grp_msk : 16;
	} cn30xx;
	struct cvmx_pow_pp_grp_mskx_cn30xx cn31xx;
	struct cvmx_pow_pp_grp_mskx_cn30xx cn38xx;
	struct cvmx_pow_pp_grp_mskx_cn30xx cn38xxp2;
	struct cvmx_pow_pp_grp_mskx_s cn50xx;
	struct cvmx_pow_pp_grp_mskx_s cn52xx;
	struct cvmx_pow_pp_grp_mskx_s cn52xxp1;
	struct cvmx_pow_pp_grp_mskx_s cn56xx;
	struct cvmx_pow_pp_grp_mskx_s cn56xxp1;
	struct cvmx_pow_pp_grp_mskx_s cn58xx;
	struct cvmx_pow_pp_grp_mskx_s cn58xxp1;
	struct cvmx_pow_pp_grp_mskx_s cn61xx;
	struct cvmx_pow_pp_grp_mskx_s cn63xx;
	struct cvmx_pow_pp_grp_mskx_s cn63xxp1;
	struct cvmx_pow_pp_grp_mskx_s cn66xx;
	struct cvmx_pow_pp_grp_mskx_s cn70xx;
	struct cvmx_pow_pp_grp_mskx_s cn70xxp1;
	struct cvmx_pow_pp_grp_mskx_s cnf71xx;
};

typedef union cvmx_pow_pp_grp_mskx cvmx_pow_pp_grp_mskx_t;

/**
 * cvmx_pow_qos_rnd#
 *
 * Contains the round definitions for issuing new work.  Each round consists of 8 bits with each
 * bit
 * corresponding to a QOS level.  There are 4 rounds contained in each register for a total of 32
 * rounds.  The issue logic traverses through the rounds sequentially (lowest round to highest
 * round)
 * in an attempt to find new work for each PP.  Within each round, the issue logic traverses
 * through
 * the QOS levels sequentially (highest QOS to lowest QOS) skipping over each QOS level with a
 * clear
 * bit in the round mask.  Note: setting a QOS level to all zeroes in all issue round registers
 * will
 * prevent work from being issued from that QOS level.
 */
union cvmx_pow_qos_rndx {
	u64 u64;
	struct cvmx_pow_qos_rndx_s {
		u64 reserved_32_63 : 32;
		u64 rnd_p3 : 8;
		u64 rnd_p2 : 8;
		u64 rnd_p1 : 8;
		u64 rnd : 8;
	} s;
	struct cvmx_pow_qos_rndx_s cn30xx;
	struct cvmx_pow_qos_rndx_s cn31xx;
	struct cvmx_pow_qos_rndx_s cn38xx;
	struct cvmx_pow_qos_rndx_s cn38xxp2;
	struct cvmx_pow_qos_rndx_s cn50xx;
	struct cvmx_pow_qos_rndx_s cn52xx;
	struct cvmx_pow_qos_rndx_s cn52xxp1;
	struct cvmx_pow_qos_rndx_s cn56xx;
	struct cvmx_pow_qos_rndx_s cn56xxp1;
	struct cvmx_pow_qos_rndx_s cn58xx;
	struct cvmx_pow_qos_rndx_s cn58xxp1;
	struct cvmx_pow_qos_rndx_s cn61xx;
	struct cvmx_pow_qos_rndx_s cn63xx;
	struct cvmx_pow_qos_rndx_s cn63xxp1;
	struct cvmx_pow_qos_rndx_s cn66xx;
	struct cvmx_pow_qos_rndx_s cn70xx;
	struct cvmx_pow_qos_rndx_s cn70xxp1;
	struct cvmx_pow_qos_rndx_s cnf71xx;
};

typedef union cvmx_pow_qos_rndx cvmx_pow_qos_rndx_t;

/**
 * cvmx_pow_qos_thr#
 *
 * Contains the thresholds for allocating POW internal storage buffers.  If the number of
 * remaining
 * free buffers drops below the minimum threshold (MIN_THR) or the number of allocated buffers
 * for
 * this QOS level rises above the maximum threshold (MAX_THR), future incoming work queue entries
 * will be buffered externally rather than internally.  This register also contains a read-only
 * count
 * of the current number of free buffers (FREE_CNT), the number of internal buffers currently
 * allocated to this QOS level (BUF_CNT), and the total number of buffers on the de-schedule list
 * (DES_CNT) (which is not the same as the total number of de-scheduled buffers).
 */
union cvmx_pow_qos_thrx {
	u64 u64;
	struct cvmx_pow_qos_thrx_s {
		u64 reserved_60_63 : 4;
		u64 des_cnt : 12;
		u64 buf_cnt : 12;
		u64 free_cnt : 12;
		u64 reserved_23_23 : 1;
		u64 max_thr : 11;
		u64 reserved_11_11 : 1;
		u64 min_thr : 11;
	} s;
	struct cvmx_pow_qos_thrx_cn30xx {
		u64 reserved_55_63 : 9;
		u64 des_cnt : 7;
		u64 reserved_43_47 : 5;
		u64 buf_cnt : 7;
		u64 reserved_31_35 : 5;
		u64 free_cnt : 7;
		u64 reserved_18_23 : 6;
		u64 max_thr : 6;
		u64 reserved_6_11 : 6;
		u64 min_thr : 6;
	} cn30xx;
	struct cvmx_pow_qos_thrx_cn31xx {
		u64 reserved_57_63 : 7;
		u64 des_cnt : 9;
		u64 reserved_45_47 : 3;
		u64 buf_cnt : 9;
		u64 reserved_33_35 : 3;
		u64 free_cnt : 9;
		u64 reserved_20_23 : 4;
		u64 max_thr : 8;
		u64 reserved_8_11 : 4;
		u64 min_thr : 8;
	} cn31xx;
	struct cvmx_pow_qos_thrx_s cn38xx;
	struct cvmx_pow_qos_thrx_s cn38xxp2;
	struct cvmx_pow_qos_thrx_cn31xx cn50xx;
	struct cvmx_pow_qos_thrx_cn52xx {
		u64 reserved_58_63 : 6;
		u64 des_cnt : 10;
		u64 reserved_46_47 : 2;
		u64 buf_cnt : 10;
		u64 reserved_34_35 : 2;
		u64 free_cnt : 10;
		u64 reserved_21_23 : 3;
		u64 max_thr : 9;
		u64 reserved_9_11 : 3;
		u64 min_thr : 9;
	} cn52xx;
	struct cvmx_pow_qos_thrx_cn52xx cn52xxp1;
	struct cvmx_pow_qos_thrx_s cn56xx;
	struct cvmx_pow_qos_thrx_s cn56xxp1;
	struct cvmx_pow_qos_thrx_s cn58xx;
	struct cvmx_pow_qos_thrx_s cn58xxp1;
	struct cvmx_pow_qos_thrx_cn52xx cn61xx;
	struct cvmx_pow_qos_thrx_cn63xx {
		u64 reserved_59_63 : 5;
		u64 des_cnt : 11;
		u64 reserved_47_47 : 1;
		u64 buf_cnt : 11;
		u64 reserved_35_35 : 1;
		u64 free_cnt : 11;
		u64 reserved_22_23 : 2;
		u64 max_thr : 10;
		u64 reserved_10_11 : 2;
		u64 min_thr : 10;
	} cn63xx;
	struct cvmx_pow_qos_thrx_cn63xx cn63xxp1;
	struct cvmx_pow_qos_thrx_cn63xx cn66xx;
	struct cvmx_pow_qos_thrx_cn52xx cn70xx;
	struct cvmx_pow_qos_thrx_cn52xx cn70xxp1;
	struct cvmx_pow_qos_thrx_cn52xx cnf71xx;
};

typedef union cvmx_pow_qos_thrx cvmx_pow_qos_thrx_t;

/**
 * cvmx_pow_ts_pc
 *
 * Counts the number of tag switch requests.  Write to clear.
 *
 */
union cvmx_pow_ts_pc {
	u64 u64;
	struct cvmx_pow_ts_pc_s {
		u64 reserved_32_63 : 32;
		u64 ts_pc : 32;
	} s;
	struct cvmx_pow_ts_pc_s cn30xx;
	struct cvmx_pow_ts_pc_s cn31xx;
	struct cvmx_pow_ts_pc_s cn38xx;
	struct cvmx_pow_ts_pc_s cn38xxp2;
	struct cvmx_pow_ts_pc_s cn50xx;
	struct cvmx_pow_ts_pc_s cn52xx;
	struct cvmx_pow_ts_pc_s cn52xxp1;
	struct cvmx_pow_ts_pc_s cn56xx;
	struct cvmx_pow_ts_pc_s cn56xxp1;
	struct cvmx_pow_ts_pc_s cn58xx;
	struct cvmx_pow_ts_pc_s cn58xxp1;
	struct cvmx_pow_ts_pc_s cn61xx;
	struct cvmx_pow_ts_pc_s cn63xx;
	struct cvmx_pow_ts_pc_s cn63xxp1;
	struct cvmx_pow_ts_pc_s cn66xx;
	struct cvmx_pow_ts_pc_s cn70xx;
	struct cvmx_pow_ts_pc_s cn70xxp1;
	struct cvmx_pow_ts_pc_s cnf71xx;
};

typedef union cvmx_pow_ts_pc cvmx_pow_ts_pc_t;

/**
 * cvmx_pow_wa_com_pc
 *
 * Counts the number of add new work requests for all QOS levels.  Write to clear.
 *
 */
union cvmx_pow_wa_com_pc {
	u64 u64;
	struct cvmx_pow_wa_com_pc_s {
		u64 reserved_32_63 : 32;
		u64 wa_pc : 32;
	} s;
	struct cvmx_pow_wa_com_pc_s cn30xx;
	struct cvmx_pow_wa_com_pc_s cn31xx;
	struct cvmx_pow_wa_com_pc_s cn38xx;
	struct cvmx_pow_wa_com_pc_s cn38xxp2;
	struct cvmx_pow_wa_com_pc_s cn50xx;
	struct cvmx_pow_wa_com_pc_s cn52xx;
	struct cvmx_pow_wa_com_pc_s cn52xxp1;
	struct cvmx_pow_wa_com_pc_s cn56xx;
	struct cvmx_pow_wa_com_pc_s cn56xxp1;
	struct cvmx_pow_wa_com_pc_s cn58xx;
	struct cvmx_pow_wa_com_pc_s cn58xxp1;
	struct cvmx_pow_wa_com_pc_s cn61xx;
	struct cvmx_pow_wa_com_pc_s cn63xx;
	struct cvmx_pow_wa_com_pc_s cn63xxp1;
	struct cvmx_pow_wa_com_pc_s cn66xx;
	struct cvmx_pow_wa_com_pc_s cn70xx;
	struct cvmx_pow_wa_com_pc_s cn70xxp1;
	struct cvmx_pow_wa_com_pc_s cnf71xx;
};

typedef union cvmx_pow_wa_com_pc cvmx_pow_wa_com_pc_t;

/**
 * cvmx_pow_wa_pc#
 *
 * Counts the number of add new work requests for each QOS level.  Write to clear.
 *
 */
union cvmx_pow_wa_pcx {
	u64 u64;
	struct cvmx_pow_wa_pcx_s {
		u64 reserved_32_63 : 32;
		u64 wa_pc : 32;
	} s;
	struct cvmx_pow_wa_pcx_s cn30xx;
	struct cvmx_pow_wa_pcx_s cn31xx;
	struct cvmx_pow_wa_pcx_s cn38xx;
	struct cvmx_pow_wa_pcx_s cn38xxp2;
	struct cvmx_pow_wa_pcx_s cn50xx;
	struct cvmx_pow_wa_pcx_s cn52xx;
	struct cvmx_pow_wa_pcx_s cn52xxp1;
	struct cvmx_pow_wa_pcx_s cn56xx;
	struct cvmx_pow_wa_pcx_s cn56xxp1;
	struct cvmx_pow_wa_pcx_s cn58xx;
	struct cvmx_pow_wa_pcx_s cn58xxp1;
	struct cvmx_pow_wa_pcx_s cn61xx;
	struct cvmx_pow_wa_pcx_s cn63xx;
	struct cvmx_pow_wa_pcx_s cn63xxp1;
	struct cvmx_pow_wa_pcx_s cn66xx;
	struct cvmx_pow_wa_pcx_s cn70xx;
	struct cvmx_pow_wa_pcx_s cn70xxp1;
	struct cvmx_pow_wa_pcx_s cnf71xx;
};

typedef union cvmx_pow_wa_pcx cvmx_pow_wa_pcx_t;

/**
 * cvmx_pow_wq_int
 *
 * Contains the bits (1 per group) that set work queue interrupts and are used to clear these
 * interrupts.  Also contains the input queue interrupt temporary disable bits (1 per group). For
 * more information regarding this register, see the interrupt section.
 */
union cvmx_pow_wq_int {
	u64 u64;
	struct cvmx_pow_wq_int_s {
		u64 reserved_32_63 : 32;
		u64 iq_dis : 16;
		u64 wq_int : 16;
	} s;
	struct cvmx_pow_wq_int_s cn30xx;
	struct cvmx_pow_wq_int_s cn31xx;
	struct cvmx_pow_wq_int_s cn38xx;
	struct cvmx_pow_wq_int_s cn38xxp2;
	struct cvmx_pow_wq_int_s cn50xx;
	struct cvmx_pow_wq_int_s cn52xx;
	struct cvmx_pow_wq_int_s cn52xxp1;
	struct cvmx_pow_wq_int_s cn56xx;
	struct cvmx_pow_wq_int_s cn56xxp1;
	struct cvmx_pow_wq_int_s cn58xx;
	struct cvmx_pow_wq_int_s cn58xxp1;
	struct cvmx_pow_wq_int_s cn61xx;
	struct cvmx_pow_wq_int_s cn63xx;
	struct cvmx_pow_wq_int_s cn63xxp1;
	struct cvmx_pow_wq_int_s cn66xx;
	struct cvmx_pow_wq_int_s cn70xx;
	struct cvmx_pow_wq_int_s cn70xxp1;
	struct cvmx_pow_wq_int_s cnf71xx;
};

typedef union cvmx_pow_wq_int cvmx_pow_wq_int_t;

/**
 * cvmx_pow_wq_int_cnt#
 *
 * Contains a read-only copy of the counts used to trigger work queue interrupts.  For more
 * information regarding this register, see the interrupt section.
 */
union cvmx_pow_wq_int_cntx {
	u64 u64;
	struct cvmx_pow_wq_int_cntx_s {
		u64 reserved_28_63 : 36;
		u64 tc_cnt : 4;
		u64 ds_cnt : 12;
		u64 iq_cnt : 12;
	} s;
	struct cvmx_pow_wq_int_cntx_cn30xx {
		u64 reserved_28_63 : 36;
		u64 tc_cnt : 4;
		u64 reserved_19_23 : 5;
		u64 ds_cnt : 7;
		u64 reserved_7_11 : 5;
		u64 iq_cnt : 7;
	} cn30xx;
	struct cvmx_pow_wq_int_cntx_cn31xx {
		u64 reserved_28_63 : 36;
		u64 tc_cnt : 4;
		u64 reserved_21_23 : 3;
		u64 ds_cnt : 9;
		u64 reserved_9_11 : 3;
		u64 iq_cnt : 9;
	} cn31xx;
	struct cvmx_pow_wq_int_cntx_s cn38xx;
	struct cvmx_pow_wq_int_cntx_s cn38xxp2;
	struct cvmx_pow_wq_int_cntx_cn31xx cn50xx;
	struct cvmx_pow_wq_int_cntx_cn52xx {
		u64 reserved_28_63 : 36;
		u64 tc_cnt : 4;
		u64 reserved_22_23 : 2;
		u64 ds_cnt : 10;
		u64 reserved_10_11 : 2;
		u64 iq_cnt : 10;
	} cn52xx;
	struct cvmx_pow_wq_int_cntx_cn52xx cn52xxp1;
	struct cvmx_pow_wq_int_cntx_s cn56xx;
	struct cvmx_pow_wq_int_cntx_s cn56xxp1;
	struct cvmx_pow_wq_int_cntx_s cn58xx;
	struct cvmx_pow_wq_int_cntx_s cn58xxp1;
	struct cvmx_pow_wq_int_cntx_cn52xx cn61xx;
	struct cvmx_pow_wq_int_cntx_cn63xx {
		u64 reserved_28_63 : 36;
		u64 tc_cnt : 4;
		u64 reserved_23_23 : 1;
		u64 ds_cnt : 11;
		u64 reserved_11_11 : 1;
		u64 iq_cnt : 11;
	} cn63xx;
	struct cvmx_pow_wq_int_cntx_cn63xx cn63xxp1;
	struct cvmx_pow_wq_int_cntx_cn63xx cn66xx;
	struct cvmx_pow_wq_int_cntx_cn52xx cn70xx;
	struct cvmx_pow_wq_int_cntx_cn52xx cn70xxp1;
	struct cvmx_pow_wq_int_cntx_cn52xx cnf71xx;
};

typedef union cvmx_pow_wq_int_cntx cvmx_pow_wq_int_cntx_t;

/**
 * cvmx_pow_wq_int_pc
 *
 * Contains the threshold value for the work queue interrupt periodic counter and also a read-
 * only
 * copy of the periodic counter.  For more information regarding this register, see the interrupt
 * section.
 */
union cvmx_pow_wq_int_pc {
	u64 u64;
	struct cvmx_pow_wq_int_pc_s {
		u64 reserved_60_63 : 4;
		u64 pc : 28;
		u64 reserved_28_31 : 4;
		u64 pc_thr : 20;
		u64 reserved_0_7 : 8;
	} s;
	struct cvmx_pow_wq_int_pc_s cn30xx;
	struct cvmx_pow_wq_int_pc_s cn31xx;
	struct cvmx_pow_wq_int_pc_s cn38xx;
	struct cvmx_pow_wq_int_pc_s cn38xxp2;
	struct cvmx_pow_wq_int_pc_s cn50xx;
	struct cvmx_pow_wq_int_pc_s cn52xx;
	struct cvmx_pow_wq_int_pc_s cn52xxp1;
	struct cvmx_pow_wq_int_pc_s cn56xx;
	struct cvmx_pow_wq_int_pc_s cn56xxp1;
	struct cvmx_pow_wq_int_pc_s cn58xx;
	struct cvmx_pow_wq_int_pc_s cn58xxp1;
	struct cvmx_pow_wq_int_pc_s cn61xx;
	struct cvmx_pow_wq_int_pc_s cn63xx;
	struct cvmx_pow_wq_int_pc_s cn63xxp1;
	struct cvmx_pow_wq_int_pc_s cn66xx;
	struct cvmx_pow_wq_int_pc_s cn70xx;
	struct cvmx_pow_wq_int_pc_s cn70xxp1;
	struct cvmx_pow_wq_int_pc_s cnf71xx;
};

typedef union cvmx_pow_wq_int_pc cvmx_pow_wq_int_pc_t;

/**
 * cvmx_pow_wq_int_thr#
 *
 * Contains the thresholds for enabling and setting work queue interrupts.  For more information
 * regarding this register, see the interrupt section.
 * Note: Up to 4 of the POW's internal storage buffers can be allocated for hardware use and are
 * therefore not available for incoming work queue entries.  Additionally, any PP that is not in
 * the
 * NULL_NULL state consumes a buffer.  Thus in a 4 PP system, it is not advisable to set either
 * IQ_THR or DS_THR to greater than 512 - 4 - 4 = 504.  Doing so may prevent the interrupt from
 * ever triggering.
 */
union cvmx_pow_wq_int_thrx {
	u64 u64;
	struct cvmx_pow_wq_int_thrx_s {
		u64 reserved_29_63 : 35;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_23_23 : 1;
		u64 ds_thr : 11;
		u64 reserved_11_11 : 1;
		u64 iq_thr : 11;
	} s;
	struct cvmx_pow_wq_int_thrx_cn30xx {
		u64 reserved_29_63 : 35;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_18_23 : 6;
		u64 ds_thr : 6;
		u64 reserved_6_11 : 6;
		u64 iq_thr : 6;
	} cn30xx;
	struct cvmx_pow_wq_int_thrx_cn31xx {
		u64 reserved_29_63 : 35;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_20_23 : 4;
		u64 ds_thr : 8;
		u64 reserved_8_11 : 4;
		u64 iq_thr : 8;
	} cn31xx;
	struct cvmx_pow_wq_int_thrx_s cn38xx;
	struct cvmx_pow_wq_int_thrx_s cn38xxp2;
	struct cvmx_pow_wq_int_thrx_cn31xx cn50xx;
	struct cvmx_pow_wq_int_thrx_cn52xx {
		u64 reserved_29_63 : 35;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_21_23 : 3;
		u64 ds_thr : 9;
		u64 reserved_9_11 : 3;
		u64 iq_thr : 9;
	} cn52xx;
	struct cvmx_pow_wq_int_thrx_cn52xx cn52xxp1;
	struct cvmx_pow_wq_int_thrx_s cn56xx;
	struct cvmx_pow_wq_int_thrx_s cn56xxp1;
	struct cvmx_pow_wq_int_thrx_s cn58xx;
	struct cvmx_pow_wq_int_thrx_s cn58xxp1;
	struct cvmx_pow_wq_int_thrx_cn52xx cn61xx;
	struct cvmx_pow_wq_int_thrx_cn63xx {
		u64 reserved_29_63 : 35;
		u64 tc_en : 1;
		u64 tc_thr : 4;
		u64 reserved_22_23 : 2;
		u64 ds_thr : 10;
		u64 reserved_10_11 : 2;
		u64 iq_thr : 10;
	} cn63xx;
	struct cvmx_pow_wq_int_thrx_cn63xx cn63xxp1;
	struct cvmx_pow_wq_int_thrx_cn63xx cn66xx;
	struct cvmx_pow_wq_int_thrx_cn52xx cn70xx;
	struct cvmx_pow_wq_int_thrx_cn52xx cn70xxp1;
	struct cvmx_pow_wq_int_thrx_cn52xx cnf71xx;
};

typedef union cvmx_pow_wq_int_thrx cvmx_pow_wq_int_thrx_t;

/**
 * cvmx_pow_ws_pc#
 *
 * Counts the number of work schedules for each group.  Write to clear.
 *
 */
union cvmx_pow_ws_pcx {
	u64 u64;
	struct cvmx_pow_ws_pcx_s {
		u64 reserved_32_63 : 32;
		u64 ws_pc : 32;
	} s;
	struct cvmx_pow_ws_pcx_s cn30xx;
	struct cvmx_pow_ws_pcx_s cn31xx;
	struct cvmx_pow_ws_pcx_s cn38xx;
	struct cvmx_pow_ws_pcx_s cn38xxp2;
	struct cvmx_pow_ws_pcx_s cn50xx;
	struct cvmx_pow_ws_pcx_s cn52xx;
	struct cvmx_pow_ws_pcx_s cn52xxp1;
	struct cvmx_pow_ws_pcx_s cn56xx;
	struct cvmx_pow_ws_pcx_s cn56xxp1;
	struct cvmx_pow_ws_pcx_s cn58xx;
	struct cvmx_pow_ws_pcx_s cn58xxp1;
	struct cvmx_pow_ws_pcx_s cn61xx;
	struct cvmx_pow_ws_pcx_s cn63xx;
	struct cvmx_pow_ws_pcx_s cn63xxp1;
	struct cvmx_pow_ws_pcx_s cn66xx;
	struct cvmx_pow_ws_pcx_s cn70xx;
	struct cvmx_pow_ws_pcx_s cn70xxp1;
	struct cvmx_pow_ws_pcx_s cnf71xx;
};

typedef union cvmx_pow_ws_pcx cvmx_pow_ws_pcx_t;

#endif
