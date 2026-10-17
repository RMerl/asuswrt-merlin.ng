/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ilk.
 */

#ifndef __CVMX_ILK_DEFS_H__
#define __CVMX_ILK_DEFS_H__

#define CVMX_ILK_BIST_SUM    (0x0001180014000038ull)
#define CVMX_ILK_GBL_CFG     (0x0001180014000000ull)
#define CVMX_ILK_GBL_ERR_CFG (0x0001180014000058ull)
#define CVMX_ILK_GBL_INT     (0x0001180014000008ull)
#define CVMX_ILK_GBL_INT_EN  (0x0001180014000010ull)
#define CVMX_ILK_INT_SUM     (0x0001180014000030ull)
#define CVMX_ILK_LNEX_TRN_CTL(offset)                                          \
	(0x00011800140380F0ull + ((offset) & 15) * 1024)
#define CVMX_ILK_LNEX_TRN_LD(offset)                                           \
	(0x00011800140380E0ull + ((offset) & 15) * 1024)
#define CVMX_ILK_LNEX_TRN_LP(offset)                                           \
	(0x00011800140380E8ull + ((offset) & 15) * 1024)
#define CVMX_ILK_LNE_DBG      (0x0001180014030008ull)
#define CVMX_ILK_LNE_STS_MSG  (0x0001180014030000ull)
#define CVMX_ILK_RID_CFG      (0x0001180014000050ull)
#define CVMX_ILK_RXF_IDX_PMAP (0x0001180014000020ull)
#define CVMX_ILK_RXF_MEM_PMAP (0x0001180014000028ull)
#define CVMX_ILK_RXX_BYTE_CNTX(offset, block_id)                               \
	(0x0001180014023000ull +                                               \
	 (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_RXX_CAL_ENTRYX(offset, block_id)                              \
	(0x0001180014021000ull +                                               \
	 (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_RXX_CFG0(offset) (0x0001180014020000ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_CFG1(offset) (0x0001180014020008ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_CHAX(offset, block_id)                                    \
	(0x0001180014002000ull +                                               \
	 (((offset) & 255) + ((block_id) & 1) * 0x200ull) * 8)
#define CVMX_ILK_RXX_CHA_XONX(offset, block_id)                                \
	(0x0001180014020400ull + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_RXX_ERR_CFG(offset)                                           \
	(0x00011800140200E0ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_FLOW_CTL0(offset)                                         \
	(0x0001180014020090ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_FLOW_CTL1(offset)                                         \
	(0x0001180014020098ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_IDX_CAL(offset)                                           \
	(0x00011800140200A0ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_IDX_STAT0(offset)                                         \
	(0x0001180014020070ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_IDX_STAT1(offset)                                         \
	(0x0001180014020078ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_INT(offset) (0x0001180014020010ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_INT_EN(offset)                                            \
	(0x0001180014020018ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_JABBER(offset)                                            \
	(0x00011800140200B8ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_MEM_CAL0(offset)                                          \
	(0x00011800140200A8ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_MEM_CAL1(offset)                                          \
	(0x00011800140200B0ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_MEM_STAT0(offset)                                         \
	(0x0001180014020080ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_MEM_STAT1(offset)                                         \
	(0x0001180014020088ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_PKT_CNTX(offset, block_id)                                \
	(0x0001180014022000ull +                                               \
	 (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_RXX_RID(offset) (0x00011800140200C0ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT0(offset)                                             \
	(0x0001180014020020ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT1(offset)                                             \
	(0x0001180014020028ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT2(offset)                                             \
	(0x0001180014020030ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT3(offset)                                             \
	(0x0001180014020038ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT4(offset)                                             \
	(0x0001180014020040ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT5(offset)                                             \
	(0x0001180014020048ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT6(offset)                                             \
	(0x0001180014020050ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT7(offset)                                             \
	(0x0001180014020058ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT8(offset)                                             \
	(0x0001180014020060ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RXX_STAT9(offset)                                             \
	(0x0001180014020068ull + ((offset) & 1) * 16384)
#define CVMX_ILK_RX_LNEX_CFG(offset)                                           \
	(0x0001180014038000ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_INT(offset)                                           \
	(0x0001180014038008ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_INT_EN(offset)                                        \
	(0x0001180014038010ull + ((offset) & 7) * 1024)
#define CVMX_ILK_RX_LNEX_STAT0(offset)                                         \
	(0x0001180014038018ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT1(offset)                                         \
	(0x0001180014038020ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT10(offset)                                        \
	(0x0001180014038068ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT2(offset)                                         \
	(0x0001180014038028ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT3(offset)                                         \
	(0x0001180014038030ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT4(offset)                                         \
	(0x0001180014038038ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT5(offset)                                         \
	(0x0001180014038040ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT6(offset)                                         \
	(0x0001180014038048ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT7(offset)                                         \
	(0x0001180014038050ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT8(offset)                                         \
	(0x0001180014038058ull + ((offset) & 15) * 1024)
#define CVMX_ILK_RX_LNEX_STAT9(offset)                                         \
	(0x0001180014038060ull + ((offset) & 15) * 1024)
#define CVMX_ILK_SER_CFG (0x0001180014000018ull)
#define CVMX_ILK_TXX_BYTE_CNTX(offset, block_id)                               \
	(0x0001180014013000ull +                                               \
	 (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_TXX_CAL_ENTRYX(offset, block_id)                              \
	(0x0001180014011000ull +                                               \
	 (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_TXX_CFG0(offset) (0x0001180014010000ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_CFG1(offset) (0x0001180014010008ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_CHA_XONX(offset, block_id)                                \
	(0x0001180014010400ull + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_TXX_DBG(offset) (0x0001180014010070ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_ERR_CFG(offset)                                           \
	(0x00011800140100B0ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_FLOW_CTL0(offset)                                         \
	(0x0001180014010048ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_FLOW_CTL1(offset)                                         \
	(0x0001180014010050ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_IDX_CAL(offset)                                           \
	(0x0001180014010058ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_IDX_PMAP(offset)                                          \
	(0x0001180014010010ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_IDX_STAT0(offset)                                         \
	(0x0001180014010020ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_IDX_STAT1(offset)                                         \
	(0x0001180014010028ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_INT(offset) (0x0001180014010078ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_INT_EN(offset)                                            \
	(0x0001180014010080ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_MEM_CAL0(offset)                                          \
	(0x0001180014010060ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_MEM_CAL1(offset)                                          \
	(0x0001180014010068ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_MEM_PMAP(offset)                                          \
	(0x0001180014010018ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_MEM_STAT0(offset)                                         \
	(0x0001180014010030ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_MEM_STAT1(offset)                                         \
	(0x0001180014010038ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_PIPE(offset) (0x0001180014010088ull + ((offset) & 1) * 16384)
#define CVMX_ILK_TXX_PKT_CNTX(offset, block_id)                                \
	(0x0001180014012000ull +                                               \
	 (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#define CVMX_ILK_TXX_RMATCH(offset)                                            \
	(0x0001180014010040ull + ((offset) & 1) * 16384)

/**
 * cvmx_ilk_bist_sum
 */
union cvmx_ilk_bist_sum {
	u64 u64;
	struct cvmx_ilk_bist_sum_s {
		u64 rxf_x2p : 1;
		u64 rxf_mem19 : 1;
		u64 rxf_mem18 : 1;
		u64 rxf_mem17 : 1;
		u64 rxf_mem16 : 1;
		u64 rxf_mem15 : 1;
		u64 reserved_52_57 : 6;
		u64 rxf_mem8 : 1;
		u64 rxf_mem7 : 1;
		u64 rxf_mem6 : 1;
		u64 rxf_mem5 : 1;
		u64 rxf_mem4 : 1;
		u64 rxf_mem3 : 1;
		u64 reserved_36_45 : 10;
		u64 rle7_dsk1 : 1;
		u64 rle7_dsk0 : 1;
		u64 rle6_dsk1 : 1;
		u64 rle6_dsk0 : 1;
		u64 rle5_dsk1 : 1;
		u64 rle5_dsk0 : 1;
		u64 rle4_dsk1 : 1;
		u64 rle4_dsk0 : 1;
		u64 rle3_dsk1 : 1;
		u64 rle3_dsk0 : 1;
		u64 rle2_dsk1 : 1;
		u64 rle2_dsk0 : 1;
		u64 rle1_dsk1 : 1;
		u64 rle1_dsk0 : 1;
		u64 rle0_dsk1 : 1;
		u64 rle0_dsk0 : 1;
		u64 rlk1_pmap : 1;
		u64 reserved_18_18 : 1;
		u64 rlk1_fwc : 1;
		u64 reserved_16_16 : 1;
		u64 rlk0_pmap : 1;
		u64 rlk0_stat1 : 1;
		u64 rlk0_fwc : 1;
		u64 rlk0_stat : 1;
		u64 tlk1_stat1 : 1;
		u64 tlk1_fwc : 1;
		u64 reserved_9_9 : 1;
		u64 tlk1_txf2 : 1;
		u64 tlk1_txf1 : 1;
		u64 tlk1_txf0 : 1;
		u64 tlk0_stat1 : 1;
		u64 tlk0_fwc : 1;
		u64 reserved_3_3 : 1;
		u64 tlk0_txf2 : 1;
		u64 tlk0_txf1 : 1;
		u64 tlk0_txf0 : 1;
	} s;
	struct cvmx_ilk_bist_sum_cn68xx {
		u64 reserved_58_63 : 6;
		u64 rxf_x2p1 : 1;
		u64 rxf_x2p0 : 1;
		u64 rxf_pmap : 1;
		u64 rxf_mem2 : 1;
		u64 rxf_mem1 : 1;
		u64 rxf_mem0 : 1;
		u64 reserved_36_51 : 16;
		u64 rle7_dsk1 : 1;
		u64 rle7_dsk0 : 1;
		u64 rle6_dsk1 : 1;
		u64 rle6_dsk0 : 1;
		u64 rle5_dsk1 : 1;
		u64 rle5_dsk0 : 1;
		u64 rle4_dsk1 : 1;
		u64 rle4_dsk0 : 1;
		u64 rle3_dsk1 : 1;
		u64 rle3_dsk0 : 1;
		u64 rle2_dsk1 : 1;
		u64 rle2_dsk0 : 1;
		u64 rle1_dsk1 : 1;
		u64 rle1_dsk0 : 1;
		u64 rle0_dsk1 : 1;
		u64 rle0_dsk0 : 1;
		u64 reserved_19_19 : 1;
		u64 rlk1_stat1 : 1;
		u64 rlk1_fwc : 1;
		u64 rlk1_stat : 1;
		u64 reserved_15_15 : 1;
		u64 rlk0_stat1 : 1;
		u64 rlk0_fwc : 1;
		u64 rlk0_stat : 1;
		u64 tlk1_stat1 : 1;
		u64 tlk1_fwc : 1;
		u64 tlk1_stat0 : 1;
		u64 tlk1_txf2 : 1;
		u64 tlk1_txf1 : 1;
		u64 tlk1_txf0 : 1;
		u64 tlk0_stat1 : 1;
		u64 tlk0_fwc : 1;
		u64 tlk0_stat0 : 1;
		u64 tlk0_txf2 : 1;
		u64 tlk0_txf1 : 1;
		u64 tlk0_txf0 : 1;
	} cn68xx;
	struct cvmx_ilk_bist_sum_cn68xxp1 {
		u64 reserved_58_63 : 6;
		u64 rxf_x2p1 : 1;
		u64 rxf_x2p0 : 1;
		u64 rxf_pmap : 1;
		u64 rxf_mem2 : 1;
		u64 rxf_mem1 : 1;
		u64 rxf_mem0 : 1;
		u64 reserved_36_51 : 16;
		u64 rle7_dsk1 : 1;
		u64 rle7_dsk0 : 1;
		u64 rle6_dsk1 : 1;
		u64 rle6_dsk0 : 1;
		u64 rle5_dsk1 : 1;
		u64 rle5_dsk0 : 1;
		u64 rle4_dsk1 : 1;
		u64 rle4_dsk0 : 1;
		u64 rle3_dsk1 : 1;
		u64 rle3_dsk0 : 1;
		u64 rle2_dsk1 : 1;
		u64 rle2_dsk0 : 1;
		u64 rle1_dsk1 : 1;
		u64 rle1_dsk0 : 1;
		u64 rle0_dsk1 : 1;
		u64 rle0_dsk0 : 1;
		u64 reserved_18_19 : 2;
		u64 rlk1_fwc : 1;
		u64 rlk1_stat : 1;
		u64 reserved_14_15 : 2;
		u64 rlk0_fwc : 1;
		u64 rlk0_stat : 1;
		u64 reserved_11_11 : 1;
		u64 tlk1_fwc : 1;
		u64 tlk1_stat : 1;
		u64 tlk1_txf2 : 1;
		u64 tlk1_txf1 : 1;
		u64 tlk1_txf0 : 1;
		u64 reserved_5_5 : 1;
		u64 tlk0_fwc : 1;
		u64 tlk0_stat : 1;
		u64 tlk0_txf2 : 1;
		u64 tlk0_txf1 : 1;
		u64 tlk0_txf0 : 1;
	} cn68xxp1;
	struct cvmx_ilk_bist_sum_cn78xx {
		u64 rxf_x2p : 1;
		u64 rxf_mem19 : 1;
		u64 rxf_mem18 : 1;
		u64 rxf_mem17 : 1;
		u64 rxf_mem16 : 1;
		u64 rxf_mem15 : 1;
		u64 rxf_mem14 : 1;
		u64 rxf_mem13 : 1;
		u64 rxf_mem12 : 1;
		u64 rxf_mem11 : 1;
		u64 rxf_mem10 : 1;
		u64 rxf_mem9 : 1;
		u64 rxf_mem8 : 1;
		u64 rxf_mem7 : 1;
		u64 rxf_mem6 : 1;
		u64 rxf_mem5 : 1;
		u64 rxf_mem4 : 1;
		u64 rxf_mem3 : 1;
		u64 rxf_mem2 : 1;
		u64 rxf_mem1 : 1;
		u64 rxf_mem0 : 1;
		u64 reserved_36_42 : 7;
		u64 rle7_dsk1 : 1;
		u64 rle7_dsk0 : 1;
		u64 rle6_dsk1 : 1;
		u64 rle6_dsk0 : 1;
		u64 rle5_dsk1 : 1;
		u64 rle5_dsk0 : 1;
		u64 rle4_dsk1 : 1;
		u64 rle4_dsk0 : 1;
		u64 rle3_dsk1 : 1;
		u64 rle3_dsk0 : 1;
		u64 rle2_dsk1 : 1;
		u64 rle2_dsk0 : 1;
		u64 rle1_dsk1 : 1;
		u64 rle1_dsk0 : 1;
		u64 rle0_dsk1 : 1;
		u64 rle0_dsk0 : 1;
		u64 rlk1_pmap : 1;
		u64 rlk1_stat : 1;
		u64 rlk1_fwc : 1;
		u64 rlk1_stat1 : 1;
		u64 rlk0_pmap : 1;
		u64 rlk0_stat1 : 1;
		u64 rlk0_fwc : 1;
		u64 rlk0_stat : 1;
		u64 tlk1_stat1 : 1;
		u64 tlk1_fwc : 1;
		u64 tlk1_stat0 : 1;
		u64 tlk1_txf2 : 1;
		u64 tlk1_txf1 : 1;
		u64 tlk1_txf0 : 1;
		u64 tlk0_stat1 : 1;
		u64 tlk0_fwc : 1;
		u64 tlk0_stat0 : 1;
		u64 tlk0_txf2 : 1;
		u64 tlk0_txf1 : 1;
		u64 tlk0_txf0 : 1;
	} cn78xx;
	struct cvmx_ilk_bist_sum_cn78xx cn78xxp1;
};

typedef union cvmx_ilk_bist_sum cvmx_ilk_bist_sum_t;

/**
 * cvmx_ilk_gbl_cfg
 */
union cvmx_ilk_gbl_cfg {
	u64 u64;
	struct cvmx_ilk_gbl_cfg_s {
		u64 reserved_4_63 : 60;
		u64 rid_rstdis : 1;
		u64 reset : 1;
		u64 cclk_dis : 1;
		u64 rxf_xlink : 1;
	} s;
	struct cvmx_ilk_gbl_cfg_s cn68xx;
	struct cvmx_ilk_gbl_cfg_cn68xxp1 {
		u64 reserved_2_63 : 62;
		u64 cclk_dis : 1;
		u64 rxf_xlink : 1;
	} cn68xxp1;
	struct cvmx_ilk_gbl_cfg_s cn78xx;
	struct cvmx_ilk_gbl_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_gbl_cfg cvmx_ilk_gbl_cfg_t;

/**
 * cvmx_ilk_gbl_err_cfg
 */
union cvmx_ilk_gbl_err_cfg {
	u64 u64;
	struct cvmx_ilk_gbl_err_cfg_s {
		u64 reserved_20_63 : 44;
		u64 rxf_flip : 2;
		u64 x2p_flip : 2;
		u64 reserved_2_15 : 14;
		u64 rxf_cor_dis : 1;
		u64 x2p_cor_dis : 1;
	} s;
	struct cvmx_ilk_gbl_err_cfg_s cn78xx;
	struct cvmx_ilk_gbl_err_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_gbl_err_cfg cvmx_ilk_gbl_err_cfg_t;

/**
 * cvmx_ilk_gbl_int
 */
union cvmx_ilk_gbl_int {
	u64 u64;
	struct cvmx_ilk_gbl_int_s {
		u64 reserved_9_63 : 55;
		u64 x2p_dbe : 1;
		u64 x2p_sbe : 1;
		u64 rxf_dbe : 1;
		u64 rxf_sbe : 1;
		u64 rxf_push_full : 1;
		u64 rxf_pop_empty : 1;
		u64 rxf_ctl_perr : 1;
		u64 rxf_lnk1_perr : 1;
		u64 rxf_lnk0_perr : 1;
	} s;
	struct cvmx_ilk_gbl_int_cn68xx {
		u64 reserved_5_63 : 59;
		u64 rxf_push_full : 1;
		u64 rxf_pop_empty : 1;
		u64 rxf_ctl_perr : 1;
		u64 rxf_lnk1_perr : 1;
		u64 rxf_lnk0_perr : 1;
	} cn68xx;
	struct cvmx_ilk_gbl_int_cn68xx cn68xxp1;
	struct cvmx_ilk_gbl_int_s cn78xx;
	struct cvmx_ilk_gbl_int_s cn78xxp1;
};

typedef union cvmx_ilk_gbl_int cvmx_ilk_gbl_int_t;

/**
 * cvmx_ilk_gbl_int_en
 */
union cvmx_ilk_gbl_int_en {
	u64 u64;
	struct cvmx_ilk_gbl_int_en_s {
		u64 reserved_5_63 : 59;
		u64 rxf_push_full : 1;
		u64 rxf_pop_empty : 1;
		u64 rxf_ctl_perr : 1;
		u64 rxf_lnk1_perr : 1;
		u64 rxf_lnk0_perr : 1;
	} s;
	struct cvmx_ilk_gbl_int_en_s cn68xx;
	struct cvmx_ilk_gbl_int_en_s cn68xxp1;
};

typedef union cvmx_ilk_gbl_int_en cvmx_ilk_gbl_int_en_t;

/**
 * cvmx_ilk_int_sum
 */
union cvmx_ilk_int_sum {
	u64 u64;
	struct cvmx_ilk_int_sum_s {
		u64 reserved_13_63 : 51;
		u64 rle7_int : 1;
		u64 rle6_int : 1;
		u64 rle5_int : 1;
		u64 rle4_int : 1;
		u64 rle3_int : 1;
		u64 rle2_int : 1;
		u64 rle1_int : 1;
		u64 rle0_int : 1;
		u64 rlk1_int : 1;
		u64 rlk0_int : 1;
		u64 tlk1_int : 1;
		u64 tlk0_int : 1;
		u64 gbl_int : 1;
	} s;
	struct cvmx_ilk_int_sum_s cn68xx;
	struct cvmx_ilk_int_sum_s cn68xxp1;
};

typedef union cvmx_ilk_int_sum cvmx_ilk_int_sum_t;

/**
 * cvmx_ilk_lne#_trn_ctl
 */
union cvmx_ilk_lnex_trn_ctl {
	u64 u64;
	struct cvmx_ilk_lnex_trn_ctl_s {
		u64 reserved_4_63 : 60;
		u64 trn_lock : 1;
		u64 trn_done : 1;
		u64 trn_ena : 1;
		u64 eie_det : 1;
	} s;
	struct cvmx_ilk_lnex_trn_ctl_s cn78xx;
	struct cvmx_ilk_lnex_trn_ctl_s cn78xxp1;
};

typedef union cvmx_ilk_lnex_trn_ctl cvmx_ilk_lnex_trn_ctl_t;

/**
 * cvmx_ilk_lne#_trn_ld
 */
union cvmx_ilk_lnex_trn_ld {
	u64 u64;
	struct cvmx_ilk_lnex_trn_ld_s {
		u64 lp_manual : 1;
		u64 reserved_49_62 : 14;
		u64 ld_cu_val : 1;
		u64 ld_cu_dat : 16;
		u64 reserved_17_31 : 15;
		u64 ld_sr_val : 1;
		u64 ld_sr_dat : 16;
	} s;
	struct cvmx_ilk_lnex_trn_ld_s cn78xx;
	struct cvmx_ilk_lnex_trn_ld_s cn78xxp1;
};

typedef union cvmx_ilk_lnex_trn_ld cvmx_ilk_lnex_trn_ld_t;

/**
 * cvmx_ilk_lne#_trn_lp
 */
union cvmx_ilk_lnex_trn_lp {
	u64 u64;
	struct cvmx_ilk_lnex_trn_lp_s {
		u64 reserved_49_63 : 15;
		u64 lp_cu_val : 1;
		u64 lp_cu_dat : 16;
		u64 reserved_17_31 : 15;
		u64 lp_sr_val : 1;
		u64 lp_sr_dat : 16;
	} s;
	struct cvmx_ilk_lnex_trn_lp_s cn78xx;
	struct cvmx_ilk_lnex_trn_lp_s cn78xxp1;
};

typedef union cvmx_ilk_lnex_trn_lp cvmx_ilk_lnex_trn_lp_t;

/**
 * cvmx_ilk_lne_dbg
 */
union cvmx_ilk_lne_dbg {
	u64 u64;
	struct cvmx_ilk_lne_dbg_s {
		u64 reserved_60_63 : 4;
		u64 tx_bad_crc32 : 1;
		u64 tx_bad_6467_cnt : 5;
		u64 tx_bad_sync_cnt : 3;
		u64 tx_bad_scram_cnt : 3;
		u64 tx_bad_lane_sel : 16;
		u64 tx_dis_dispr : 16;
		u64 tx_dis_scram : 16;
	} s;
	struct cvmx_ilk_lne_dbg_cn68xx {
		u64 reserved_60_63 : 4;
		u64 tx_bad_crc32 : 1;
		u64 tx_bad_6467_cnt : 5;
		u64 tx_bad_sync_cnt : 3;
		u64 tx_bad_scram_cnt : 3;
		u64 reserved_40_47 : 8;
		u64 tx_bad_lane_sel : 8;
		u64 reserved_24_31 : 8;
		u64 tx_dis_dispr : 8;
		u64 reserved_8_15 : 8;
		u64 tx_dis_scram : 8;
	} cn68xx;
	struct cvmx_ilk_lne_dbg_cn68xx cn68xxp1;
	struct cvmx_ilk_lne_dbg_s cn78xx;
	struct cvmx_ilk_lne_dbg_s cn78xxp1;
};

typedef union cvmx_ilk_lne_dbg cvmx_ilk_lne_dbg_t;

/**
 * cvmx_ilk_lne_sts_msg
 */
union cvmx_ilk_lne_sts_msg {
	u64 u64;
	struct cvmx_ilk_lne_sts_msg_s {
		u64 rx_lnk_stat : 16;
		u64 rx_lne_stat : 16;
		u64 tx_lnk_stat : 16;
		u64 tx_lne_stat : 16;
	} s;
	struct cvmx_ilk_lne_sts_msg_cn68xx {
		u64 reserved_56_63 : 8;
		u64 rx_lnk_stat : 8;
		u64 reserved_40_47 : 8;
		u64 rx_lne_stat : 8;
		u64 reserved_24_31 : 8;
		u64 tx_lnk_stat : 8;
		u64 reserved_8_15 : 8;
		u64 tx_lne_stat : 8;
	} cn68xx;
	struct cvmx_ilk_lne_sts_msg_cn68xx cn68xxp1;
	struct cvmx_ilk_lne_sts_msg_s cn78xx;
	struct cvmx_ilk_lne_sts_msg_s cn78xxp1;
};

typedef union cvmx_ilk_lne_sts_msg cvmx_ilk_lne_sts_msg_t;

/**
 * cvmx_ilk_rid_cfg
 */
union cvmx_ilk_rid_cfg {
	u64 u64;
	struct cvmx_ilk_rid_cfg_s {
		u64 reserved_39_63 : 25;
		u64 max_cnt : 7;
		u64 reserved_7_31 : 25;
		u64 base : 7;
	} s;
	struct cvmx_ilk_rid_cfg_s cn78xx;
	struct cvmx_ilk_rid_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_rid_cfg cvmx_ilk_rid_cfg_t;

/**
 * cvmx_ilk_rx#_byte_cnt#
 */
union cvmx_ilk_rxx_byte_cntx {
	u64 u64;
	struct cvmx_ilk_rxx_byte_cntx_s {
		u64 reserved_40_63 : 24;
		u64 rx_bytes : 40;
	} s;
	struct cvmx_ilk_rxx_byte_cntx_s cn78xx;
	struct cvmx_ilk_rxx_byte_cntx_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_byte_cntx cvmx_ilk_rxx_byte_cntx_t;

/**
 * cvmx_ilk_rx#_cal_entry#
 */
union cvmx_ilk_rxx_cal_entryx {
	u64 u64;
	struct cvmx_ilk_rxx_cal_entryx_s {
		u64 reserved_34_63 : 30;
		u64 ctl : 2;
		u64 reserved_8_31 : 24;
		u64 channel : 8;
	} s;
	struct cvmx_ilk_rxx_cal_entryx_s cn78xx;
	struct cvmx_ilk_rxx_cal_entryx_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_cal_entryx cvmx_ilk_rxx_cal_entryx_t;

/**
 * cvmx_ilk_rx#_cfg0
 */
union cvmx_ilk_rxx_cfg0 {
	u64 u64;
	struct cvmx_ilk_rxx_cfg0_s {
		u64 ext_lpbk_fc : 1;
		u64 ext_lpbk : 1;
		u64 reserved_60_61 : 2;
		u64 lnk_stats_wrap : 1;
		u64 bcw_push : 1;
		u64 mproto_ign : 1;
		u64 ptrn_mode : 1;
		u64 lnk_stats_rdclr : 1;
		u64 lnk_stats_ena : 1;
		u64 mltuse_fc_ena : 1;
		u64 cal_ena : 1;
		u64 mfrm_len : 13;
		u64 brst_shrt : 7;
		u64 lane_rev : 1;
		u64 brst_max : 5;
		u64 reserved_25_25 : 1;
		u64 cal_depth : 9;
		u64 lane_ena : 16;
	} s;
	struct cvmx_ilk_rxx_cfg0_cn68xx {
		u64 ext_lpbk_fc : 1;
		u64 ext_lpbk : 1;
		u64 reserved_60_61 : 2;
		u64 lnk_stats_wrap : 1;
		u64 bcw_push : 1;
		u64 mproto_ign : 1;
		u64 ptrn_mode : 1;
		u64 lnk_stats_rdclr : 1;
		u64 lnk_stats_ena : 1;
		u64 mltuse_fc_ena : 1;
		u64 cal_ena : 1;
		u64 mfrm_len : 13;
		u64 brst_shrt : 7;
		u64 lane_rev : 1;
		u64 brst_max : 5;
		u64 reserved_25_25 : 1;
		u64 cal_depth : 9;
		u64 reserved_8_15 : 8;
		u64 lane_ena : 8;
	} cn68xx;
	struct cvmx_ilk_rxx_cfg0_cn68xxp1 {
		u64 ext_lpbk_fc : 1;
		u64 ext_lpbk : 1;
		u64 reserved_57_61 : 5;
		u64 ptrn_mode : 1;
		u64 lnk_stats_rdclr : 1;
		u64 lnk_stats_ena : 1;
		u64 mltuse_fc_ena : 1;
		u64 cal_ena : 1;
		u64 mfrm_len : 13;
		u64 brst_shrt : 7;
		u64 lane_rev : 1;
		u64 brst_max : 5;
		u64 reserved_25_25 : 1;
		u64 cal_depth : 9;
		u64 reserved_8_15 : 8;
		u64 lane_ena : 8;
	} cn68xxp1;
	struct cvmx_ilk_rxx_cfg0_s cn78xx;
	struct cvmx_ilk_rxx_cfg0_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_cfg0 cvmx_ilk_rxx_cfg0_t;

/**
 * cvmx_ilk_rx#_cfg1
 */
union cvmx_ilk_rxx_cfg1 {
	u64 u64;
	struct cvmx_ilk_rxx_cfg1_s {
		u64 reserved_62_63 : 2;
		u64 rx_fifo_cnt : 12;
		u64 reserved_49_49 : 1;
		u64 rx_fifo_hwm : 13;
		u64 reserved_35_35 : 1;
		u64 rx_fifo_max : 13;
		u64 pkt_flush : 1;
		u64 pkt_ena : 1;
		u64 la_mode : 1;
		u64 tx_link_fc : 1;
		u64 rx_link_fc : 1;
		u64 rx_align_ena : 1;
		u64 rx_bdry_lock_ena : 16;
	} s;
	struct cvmx_ilk_rxx_cfg1_cn68xx {
		u64 reserved_62_63 : 2;
		u64 rx_fifo_cnt : 12;
		u64 reserved_48_49 : 2;
		u64 rx_fifo_hwm : 12;
		u64 reserved_34_35 : 2;
		u64 rx_fifo_max : 12;
		u64 pkt_flush : 1;
		u64 pkt_ena : 1;
		u64 la_mode : 1;
		u64 tx_link_fc : 1;
		u64 rx_link_fc : 1;
		u64 rx_align_ena : 1;
		u64 reserved_8_15 : 8;
		u64 rx_bdry_lock_ena : 8;
	} cn68xx;
	struct cvmx_ilk_rxx_cfg1_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_cfg1_s cn78xx;
	struct cvmx_ilk_rxx_cfg1_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_cfg1 cvmx_ilk_rxx_cfg1_t;

/**
 * cvmx_ilk_rx#_cha#
 */
union cvmx_ilk_rxx_chax {
	u64 u64;
	struct cvmx_ilk_rxx_chax_s {
		u64 reserved_6_63 : 58;
		u64 port_kind : 6;
	} s;
	struct cvmx_ilk_rxx_chax_s cn78xx;
	struct cvmx_ilk_rxx_chax_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_chax cvmx_ilk_rxx_chax_t;

/**
 * cvmx_ilk_rx#_cha_xon#
 */
union cvmx_ilk_rxx_cha_xonx {
	u64 u64;
	struct cvmx_ilk_rxx_cha_xonx_s {
		u64 xon : 64;
	} s;
	struct cvmx_ilk_rxx_cha_xonx_s cn78xx;
	struct cvmx_ilk_rxx_cha_xonx_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_cha_xonx cvmx_ilk_rxx_cha_xonx_t;

/**
 * cvmx_ilk_rx#_err_cfg
 */
union cvmx_ilk_rxx_err_cfg {
	u64 u64;
	struct cvmx_ilk_rxx_err_cfg_s {
		u64 reserved_20_63 : 44;
		u64 fwc_flip : 2;
		u64 pmap_flip : 2;
		u64 reserved_2_15 : 14;
		u64 fwc_cor_dis : 1;
		u64 pmap_cor_dis : 1;
	} s;
	struct cvmx_ilk_rxx_err_cfg_s cn78xx;
	struct cvmx_ilk_rxx_err_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_err_cfg cvmx_ilk_rxx_err_cfg_t;

/**
 * cvmx_ilk_rx#_flow_ctl0
 */
union cvmx_ilk_rxx_flow_ctl0 {
	u64 u64;
	struct cvmx_ilk_rxx_flow_ctl0_s {
		u64 status : 64;
	} s;
	struct cvmx_ilk_rxx_flow_ctl0_s cn68xx;
	struct cvmx_ilk_rxx_flow_ctl0_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_flow_ctl0 cvmx_ilk_rxx_flow_ctl0_t;

/**
 * cvmx_ilk_rx#_flow_ctl1
 */
union cvmx_ilk_rxx_flow_ctl1 {
	u64 u64;
	struct cvmx_ilk_rxx_flow_ctl1_s {
		u64 status : 64;
	} s;
	struct cvmx_ilk_rxx_flow_ctl1_s cn68xx;
	struct cvmx_ilk_rxx_flow_ctl1_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_flow_ctl1 cvmx_ilk_rxx_flow_ctl1_t;

/**
 * cvmx_ilk_rx#_idx_cal
 */
union cvmx_ilk_rxx_idx_cal {
	u64 u64;
	struct cvmx_ilk_rxx_idx_cal_s {
		u64 reserved_14_63 : 50;
		u64 inc : 6;
		u64 reserved_6_7 : 2;
		u64 index : 6;
	} s;
	struct cvmx_ilk_rxx_idx_cal_s cn68xx;
	struct cvmx_ilk_rxx_idx_cal_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_idx_cal cvmx_ilk_rxx_idx_cal_t;

/**
 * cvmx_ilk_rx#_idx_stat0
 */
union cvmx_ilk_rxx_idx_stat0 {
	u64 u64;
	struct cvmx_ilk_rxx_idx_stat0_s {
		u64 reserved_32_63 : 32;
		u64 clr : 1;
		u64 reserved_24_30 : 7;
		u64 inc : 8;
		u64 reserved_8_15 : 8;
		u64 index : 8;
	} s;
	struct cvmx_ilk_rxx_idx_stat0_s cn68xx;
	struct cvmx_ilk_rxx_idx_stat0_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_idx_stat0 cvmx_ilk_rxx_idx_stat0_t;

/**
 * cvmx_ilk_rx#_idx_stat1
 */
union cvmx_ilk_rxx_idx_stat1 {
	u64 u64;
	struct cvmx_ilk_rxx_idx_stat1_s {
		u64 reserved_32_63 : 32;
		u64 clr : 1;
		u64 reserved_24_30 : 7;
		u64 inc : 8;
		u64 reserved_8_15 : 8;
		u64 index : 8;
	} s;
	struct cvmx_ilk_rxx_idx_stat1_s cn68xx;
	struct cvmx_ilk_rxx_idx_stat1_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_idx_stat1 cvmx_ilk_rxx_idx_stat1_t;

/**
 * cvmx_ilk_rx#_int
 */
union cvmx_ilk_rxx_int {
	u64 u64;
	struct cvmx_ilk_rxx_int_s {
		u64 reserved_13_63 : 51;
		u64 pmap_dbe : 1;
		u64 pmap_sbe : 1;
		u64 fwc_dbe : 1;
		u64 fwc_sbe : 1;
		u64 pkt_drop_sop : 1;
		u64 pkt_drop_rid : 1;
		u64 pkt_drop_rxf : 1;
		u64 lane_bad_word : 1;
		u64 stat_cnt_ovfl : 1;
		u64 lane_align_done : 1;
		u64 word_sync_done : 1;
		u64 crc24_err : 1;
		u64 lane_align_fail : 1;
	} s;
	struct cvmx_ilk_rxx_int_cn68xx {
		u64 reserved_9_63 : 55;
		u64 pkt_drop_sop : 1;
		u64 pkt_drop_rid : 1;
		u64 pkt_drop_rxf : 1;
		u64 lane_bad_word : 1;
		u64 stat_cnt_ovfl : 1;
		u64 lane_align_done : 1;
		u64 word_sync_done : 1;
		u64 crc24_err : 1;
		u64 lane_align_fail : 1;
	} cn68xx;
	struct cvmx_ilk_rxx_int_cn68xxp1 {
		u64 reserved_8_63 : 56;
		u64 pkt_drop_rid : 1;
		u64 pkt_drop_rxf : 1;
		u64 lane_bad_word : 1;
		u64 stat_cnt_ovfl : 1;
		u64 lane_align_done : 1;
		u64 word_sync_done : 1;
		u64 crc24_err : 1;
		u64 lane_align_fail : 1;
	} cn68xxp1;
	struct cvmx_ilk_rxx_int_s cn78xx;
	struct cvmx_ilk_rxx_int_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_int cvmx_ilk_rxx_int_t;

/**
 * cvmx_ilk_rx#_int_en
 */
union cvmx_ilk_rxx_int_en {
	u64 u64;
	struct cvmx_ilk_rxx_int_en_s {
		u64 reserved_9_63 : 55;
		u64 pkt_drop_sop : 1;
		u64 pkt_drop_rid : 1;
		u64 pkt_drop_rxf : 1;
		u64 lane_bad_word : 1;
		u64 stat_cnt_ovfl : 1;
		u64 lane_align_done : 1;
		u64 word_sync_done : 1;
		u64 crc24_err : 1;
		u64 lane_align_fail : 1;
	} s;
	struct cvmx_ilk_rxx_int_en_s cn68xx;
	struct cvmx_ilk_rxx_int_en_cn68xxp1 {
		u64 reserved_8_63 : 56;
		u64 pkt_drop_rid : 1;
		u64 pkt_drop_rxf : 1;
		u64 lane_bad_word : 1;
		u64 stat_cnt_ovfl : 1;
		u64 lane_align_done : 1;
		u64 word_sync_done : 1;
		u64 crc24_err : 1;
		u64 lane_align_fail : 1;
	} cn68xxp1;
};

typedef union cvmx_ilk_rxx_int_en cvmx_ilk_rxx_int_en_t;

/**
 * cvmx_ilk_rx#_jabber
 */
union cvmx_ilk_rxx_jabber {
	u64 u64;
	struct cvmx_ilk_rxx_jabber_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_ilk_rxx_jabber_s cn68xx;
	struct cvmx_ilk_rxx_jabber_s cn68xxp1;
	struct cvmx_ilk_rxx_jabber_s cn78xx;
	struct cvmx_ilk_rxx_jabber_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_jabber cvmx_ilk_rxx_jabber_t;

/**
 * cvmx_ilk_rx#_mem_cal0
 *
 * Notes:
 * Software must program the calendar table prior to enabling the
 * link.
 *
 * Software must always write ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 *
 * A given calendar table entry has no effect on PKO pipe
 * backpressure when either:
 *  - ENTRY_CTLx=Link (1), or
 *  - ENTRY_CTLx=XON (3) and PORT_PIPEx is outside the range of ILK_TXx_PIPE[BASE/NUMP].
 *
 * Within the 8 calendar table entries of one IDX value, if more
 * than one affects the same PKO pipe, XOFF always wins over XON,
 * regardless of the calendar table order.
 *
 * Software must always read ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 */
union cvmx_ilk_rxx_mem_cal0 {
	u64 u64;
	struct cvmx_ilk_rxx_mem_cal0_s {
		u64 reserved_36_63 : 28;
		u64 entry_ctl3 : 2;
		u64 port_pipe3 : 7;
		u64 entry_ctl2 : 2;
		u64 port_pipe2 : 7;
		u64 entry_ctl1 : 2;
		u64 port_pipe1 : 7;
		u64 entry_ctl0 : 2;
		u64 port_pipe0 : 7;
	} s;
	struct cvmx_ilk_rxx_mem_cal0_s cn68xx;
	struct cvmx_ilk_rxx_mem_cal0_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_mem_cal0 cvmx_ilk_rxx_mem_cal0_t;

/**
 * cvmx_ilk_rx#_mem_cal1
 *
 * Notes:
 * Software must program the calendar table prior to enabling the
 * link.
 *
 * Software must always write ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 *
 * A given calendar table entry has no effect on PKO pipe
 * backpressure when either:
 *  - ENTRY_CTLx=Link (1), or
 *  - ENTRY_CTLx=XON (3) and PORT_PIPEx is outside the range of ILK_TXx_PIPE[BASE/NUMP].
 *
 * Within the 8 calendar table entries of one IDX value, if more
 * than one affects the same PKO pipe, XOFF always wins over XON,
 * regardless of the calendar table order.
 *
 * Software must always read ILK_RXx_MEM_CAL0 then ILK_Rx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 */
union cvmx_ilk_rxx_mem_cal1 {
	u64 u64;
	struct cvmx_ilk_rxx_mem_cal1_s {
		u64 reserved_36_63 : 28;
		u64 entry_ctl7 : 2;
		u64 port_pipe7 : 7;
		u64 entry_ctl6 : 2;
		u64 port_pipe6 : 7;
		u64 entry_ctl5 : 2;
		u64 port_pipe5 : 7;
		u64 entry_ctl4 : 2;
		u64 port_pipe4 : 7;
	} s;
	struct cvmx_ilk_rxx_mem_cal1_s cn68xx;
	struct cvmx_ilk_rxx_mem_cal1_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_mem_cal1 cvmx_ilk_rxx_mem_cal1_t;

/**
 * cvmx_ilk_rx#_mem_stat0
 */
union cvmx_ilk_rxx_mem_stat0 {
	u64 u64;
	struct cvmx_ilk_rxx_mem_stat0_s {
		u64 reserved_28_63 : 36;
		u64 rx_pkt : 28;
	} s;
	struct cvmx_ilk_rxx_mem_stat0_s cn68xx;
	struct cvmx_ilk_rxx_mem_stat0_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_mem_stat0 cvmx_ilk_rxx_mem_stat0_t;

/**
 * cvmx_ilk_rx#_mem_stat1
 */
union cvmx_ilk_rxx_mem_stat1 {
	u64 u64;
	struct cvmx_ilk_rxx_mem_stat1_s {
		u64 reserved_36_63 : 28;
		u64 rx_bytes : 36;
	} s;
	struct cvmx_ilk_rxx_mem_stat1_s cn68xx;
	struct cvmx_ilk_rxx_mem_stat1_s cn68xxp1;
};

typedef union cvmx_ilk_rxx_mem_stat1 cvmx_ilk_rxx_mem_stat1_t;

/**
 * cvmx_ilk_rx#_pkt_cnt#
 */
union cvmx_ilk_rxx_pkt_cntx {
	u64 u64;
	struct cvmx_ilk_rxx_pkt_cntx_s {
		u64 reserved_34_63 : 30;
		u64 rx_pkt : 34;
	} s;
	struct cvmx_ilk_rxx_pkt_cntx_s cn78xx;
	struct cvmx_ilk_rxx_pkt_cntx_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_pkt_cntx cvmx_ilk_rxx_pkt_cntx_t;

/**
 * cvmx_ilk_rx#_rid
 */
union cvmx_ilk_rxx_rid {
	u64 u64;
	struct cvmx_ilk_rxx_rid_s {
		u64 reserved_7_63 : 57;
		u64 max_cnt : 7;
	} s;
	struct cvmx_ilk_rxx_rid_cn68xx {
		u64 reserved_6_63 : 58;
		u64 max_cnt : 6;
	} cn68xx;
	struct cvmx_ilk_rxx_rid_s cn78xx;
	struct cvmx_ilk_rxx_rid_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_rid cvmx_ilk_rxx_rid_t;

/**
 * cvmx_ilk_rx#_stat0
 */
union cvmx_ilk_rxx_stat0 {
	u64 u64;
	struct cvmx_ilk_rxx_stat0_s {
		u64 reserved_35_63 : 29;
		u64 crc24_match_cnt : 35;
	} s;
	struct cvmx_ilk_rxx_stat0_cn68xx {
		u64 reserved_33_63 : 31;
		u64 crc24_match_cnt : 33;
	} cn68xx;
	struct cvmx_ilk_rxx_stat0_cn68xxp1 {
		u64 reserved_27_63 : 37;
		u64 crc24_match_cnt : 27;
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat0_s cn78xx;
	struct cvmx_ilk_rxx_stat0_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat0 cvmx_ilk_rxx_stat0_t;

/**
 * cvmx_ilk_rx#_stat1
 */
union cvmx_ilk_rxx_stat1 {
	u64 u64;
	struct cvmx_ilk_rxx_stat1_s {
		u64 reserved_20_63 : 44;
		u64 crc24_err_cnt : 20;
	} s;
	struct cvmx_ilk_rxx_stat1_cn68xx {
		u64 reserved_18_63 : 46;
		u64 crc24_err_cnt : 18;
	} cn68xx;
	struct cvmx_ilk_rxx_stat1_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_stat1_s cn78xx;
	struct cvmx_ilk_rxx_stat1_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat1 cvmx_ilk_rxx_stat1_t;

/**
 * cvmx_ilk_rx#_stat2
 */
union cvmx_ilk_rxx_stat2 {
	u64 u64;
	struct cvmx_ilk_rxx_stat2_s {
		u64 reserved_50_63 : 14;
		u64 brst_not_full_cnt : 18;
		u64 reserved_30_31 : 2;
		u64 brst_cnt : 30;
	} s;
	struct cvmx_ilk_rxx_stat2_cn68xx {
		u64 reserved_48_63 : 16;
		u64 brst_not_full_cnt : 16;
		u64 reserved_28_31 : 4;
		u64 brst_cnt : 28;
	} cn68xx;
	struct cvmx_ilk_rxx_stat2_cn68xxp1 {
		u64 reserved_48_63 : 16;
		u64 brst_not_full_cnt : 16;
		u64 reserved_16_31 : 16;
		u64 brst_cnt : 16;
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat2_s cn78xx;
	struct cvmx_ilk_rxx_stat2_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat2 cvmx_ilk_rxx_stat2_t;

/**
 * cvmx_ilk_rx#_stat3
 */
union cvmx_ilk_rxx_stat3 {
	u64 u64;
	struct cvmx_ilk_rxx_stat3_s {
		u64 reserved_18_63 : 46;
		u64 brst_max_err_cnt : 18;
	} s;
	struct cvmx_ilk_rxx_stat3_cn68xx {
		u64 reserved_16_63 : 48;
		u64 brst_max_err_cnt : 16;
	} cn68xx;
	struct cvmx_ilk_rxx_stat3_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_stat3_s cn78xx;
	struct cvmx_ilk_rxx_stat3_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat3 cvmx_ilk_rxx_stat3_t;

/**
 * cvmx_ilk_rx#_stat4
 */
union cvmx_ilk_rxx_stat4 {
	u64 u64;
	struct cvmx_ilk_rxx_stat4_s {
		u64 reserved_18_63 : 46;
		u64 brst_shrt_err_cnt : 18;
	} s;
	struct cvmx_ilk_rxx_stat4_cn68xx {
		u64 reserved_16_63 : 48;
		u64 brst_shrt_err_cnt : 16;
	} cn68xx;
	struct cvmx_ilk_rxx_stat4_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_stat4_s cn78xx;
	struct cvmx_ilk_rxx_stat4_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat4 cvmx_ilk_rxx_stat4_t;

/**
 * cvmx_ilk_rx#_stat5
 */
union cvmx_ilk_rxx_stat5 {
	u64 u64;
	struct cvmx_ilk_rxx_stat5_s {
		u64 reserved_25_63 : 39;
		u64 align_cnt : 25;
	} s;
	struct cvmx_ilk_rxx_stat5_cn68xx {
		u64 reserved_23_63 : 41;
		u64 align_cnt : 23;
	} cn68xx;
	struct cvmx_ilk_rxx_stat5_cn68xxp1 {
		u64 reserved_16_63 : 48;
		u64 align_cnt : 16;
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat5_s cn78xx;
	struct cvmx_ilk_rxx_stat5_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat5 cvmx_ilk_rxx_stat5_t;

/**
 * cvmx_ilk_rx#_stat6
 */
union cvmx_ilk_rxx_stat6 {
	u64 u64;
	struct cvmx_ilk_rxx_stat6_s {
		u64 reserved_18_63 : 46;
		u64 align_err_cnt : 18;
	} s;
	struct cvmx_ilk_rxx_stat6_cn68xx {
		u64 reserved_16_63 : 48;
		u64 align_err_cnt : 16;
	} cn68xx;
	struct cvmx_ilk_rxx_stat6_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_stat6_s cn78xx;
	struct cvmx_ilk_rxx_stat6_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat6 cvmx_ilk_rxx_stat6_t;

/**
 * cvmx_ilk_rx#_stat7
 */
union cvmx_ilk_rxx_stat7 {
	u64 u64;
	struct cvmx_ilk_rxx_stat7_s {
		u64 reserved_18_63 : 46;
		u64 bad_64b67b_cnt : 18;
	} s;
	struct cvmx_ilk_rxx_stat7_cn68xx {
		u64 reserved_16_63 : 48;
		u64 bad_64b67b_cnt : 16;
	} cn68xx;
	struct cvmx_ilk_rxx_stat7_cn68xx cn68xxp1;
	struct cvmx_ilk_rxx_stat7_s cn78xx;
	struct cvmx_ilk_rxx_stat7_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat7 cvmx_ilk_rxx_stat7_t;

/**
 * cvmx_ilk_rx#_stat8
 */
union cvmx_ilk_rxx_stat8 {
	u64 u64;
	struct cvmx_ilk_rxx_stat8_s {
		u64 reserved_32_63 : 32;
		u64 pkt_drop_rid_cnt : 16;
		u64 pkt_drop_rxf_cnt : 16;
	} s;
	struct cvmx_ilk_rxx_stat8_s cn68xx;
	struct cvmx_ilk_rxx_stat8_s cn68xxp1;
	struct cvmx_ilk_rxx_stat8_s cn78xx;
	struct cvmx_ilk_rxx_stat8_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat8 cvmx_ilk_rxx_stat8_t;

/**
 * cvmx_ilk_rx#_stat9
 *
 * This register is reserved.
 *
 */
union cvmx_ilk_rxx_stat9 {
	u64 u64;
	struct cvmx_ilk_rxx_stat9_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_ilk_rxx_stat9_s cn68xx;
	struct cvmx_ilk_rxx_stat9_s cn68xxp1;
	struct cvmx_ilk_rxx_stat9_s cn78xx;
	struct cvmx_ilk_rxx_stat9_s cn78xxp1;
};

typedef union cvmx_ilk_rxx_stat9 cvmx_ilk_rxx_stat9_t;

/**
 * cvmx_ilk_rx_lne#_cfg
 */
union cvmx_ilk_rx_lnex_cfg {
	u64 u64;
	struct cvmx_ilk_rx_lnex_cfg_s {
		u64 reserved_9_63 : 55;
		u64 rx_dis_psh_skip : 1;
		u64 reserved_7_7 : 1;
		u64 rx_dis_disp_chk : 1;
		u64 rx_scrm_sync : 1;
		u64 rx_bdry_sync : 1;
		u64 rx_dis_ukwn : 1;
		u64 rx_dis_scram : 1;
		u64 stat_rdclr : 1;
		u64 stat_ena : 1;
	} s;
	struct cvmx_ilk_rx_lnex_cfg_cn68xx {
		u64 reserved_9_63 : 55;
		u64 rx_dis_psh_skip : 1;
		u64 reserved_6_7 : 2;
		u64 rx_scrm_sync : 1;
		u64 rx_bdry_sync : 1;
		u64 rx_dis_ukwn : 1;
		u64 rx_dis_scram : 1;
		u64 stat_rdclr : 1;
		u64 stat_ena : 1;
	} cn68xx;
	struct cvmx_ilk_rx_lnex_cfg_cn68xxp1 {
		u64 reserved_5_63 : 59;
		u64 rx_bdry_sync : 1;
		u64 rx_dis_ukwn : 1;
		u64 rx_dis_scram : 1;
		u64 stat_rdclr : 1;
		u64 stat_ena : 1;
	} cn68xxp1;
	struct cvmx_ilk_rx_lnex_cfg_s cn78xx;
	struct cvmx_ilk_rx_lnex_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_cfg cvmx_ilk_rx_lnex_cfg_t;

/**
 * cvmx_ilk_rx_lne#_int
 */
union cvmx_ilk_rx_lnex_int {
	u64 u64;
	struct cvmx_ilk_rx_lnex_int_s {
		u64 reserved_10_63 : 54;
		u64 disp_err : 1;
		u64 bad_64b67b : 1;
		u64 stat_cnt_ovfl : 1;
		u64 stat_msg : 1;
		u64 dskew_fifo_ovfl : 1;
		u64 scrm_sync_loss : 1;
		u64 ukwn_cntl_word : 1;
		u64 crc32_err : 1;
		u64 bdry_sync_loss : 1;
		u64 serdes_lock_loss : 1;
	} s;
	struct cvmx_ilk_rx_lnex_int_cn68xx {
		u64 reserved_9_63 : 55;
		u64 bad_64b67b : 1;
		u64 stat_cnt_ovfl : 1;
		u64 stat_msg : 1;
		u64 dskew_fifo_ovfl : 1;
		u64 scrm_sync_loss : 1;
		u64 ukwn_cntl_word : 1;
		u64 crc32_err : 1;
		u64 bdry_sync_loss : 1;
		u64 serdes_lock_loss : 1;
	} cn68xx;
	struct cvmx_ilk_rx_lnex_int_cn68xx cn68xxp1;
	struct cvmx_ilk_rx_lnex_int_s cn78xx;
	struct cvmx_ilk_rx_lnex_int_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_int cvmx_ilk_rx_lnex_int_t;

/**
 * cvmx_ilk_rx_lne#_int_en
 */
union cvmx_ilk_rx_lnex_int_en {
	u64 u64;
	struct cvmx_ilk_rx_lnex_int_en_s {
		u64 reserved_9_63 : 55;
		u64 bad_64b67b : 1;
		u64 stat_cnt_ovfl : 1;
		u64 stat_msg : 1;
		u64 dskew_fifo_ovfl : 1;
		u64 scrm_sync_loss : 1;
		u64 ukwn_cntl_word : 1;
		u64 crc32_err : 1;
		u64 bdry_sync_loss : 1;
		u64 serdes_lock_loss : 1;
	} s;
	struct cvmx_ilk_rx_lnex_int_en_s cn68xx;
	struct cvmx_ilk_rx_lnex_int_en_s cn68xxp1;
};

typedef union cvmx_ilk_rx_lnex_int_en cvmx_ilk_rx_lnex_int_en_t;

/**
 * cvmx_ilk_rx_lne#_stat0
 */
union cvmx_ilk_rx_lnex_stat0 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat0_s {
		u64 reserved_18_63 : 46;
		u64 ser_lock_loss_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat0_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat0_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat0_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat0_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat0 cvmx_ilk_rx_lnex_stat0_t;

/**
 * cvmx_ilk_rx_lne#_stat1
 */
union cvmx_ilk_rx_lnex_stat1 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat1_s {
		u64 reserved_18_63 : 46;
		u64 bdry_sync_loss_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat1_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat1_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat1_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat1_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat1 cvmx_ilk_rx_lnex_stat1_t;

/**
 * cvmx_ilk_rx_lne#_stat10
 */
union cvmx_ilk_rx_lnex_stat10 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat10_s {
		u64 reserved_43_63 : 21;
		u64 prbs_bad : 11;
		u64 reserved_11_31 : 21;
		u64 prbs_good : 11;
	} s;
	struct cvmx_ilk_rx_lnex_stat10_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat10_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat10 cvmx_ilk_rx_lnex_stat10_t;

/**
 * cvmx_ilk_rx_lne#_stat2
 */
union cvmx_ilk_rx_lnex_stat2 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat2_s {
		u64 reserved_50_63 : 14;
		u64 syncw_good_cnt : 18;
		u64 reserved_18_31 : 14;
		u64 syncw_bad_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat2_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat2_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat2_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat2_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat2 cvmx_ilk_rx_lnex_stat2_t;

/**
 * cvmx_ilk_rx_lne#_stat3
 */
union cvmx_ilk_rx_lnex_stat3 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat3_s {
		u64 reserved_18_63 : 46;
		u64 bad_64b67b_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat3_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat3_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat3_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat3_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat3 cvmx_ilk_rx_lnex_stat3_t;

/**
 * cvmx_ilk_rx_lne#_stat4
 */
union cvmx_ilk_rx_lnex_stat4 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat4_s {
		u64 reserved_59_63 : 5;
		u64 cntl_word_cnt : 27;
		u64 reserved_27_31 : 5;
		u64 data_word_cnt : 27;
	} s;
	struct cvmx_ilk_rx_lnex_stat4_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat4_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat4_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat4_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat4 cvmx_ilk_rx_lnex_stat4_t;

/**
 * cvmx_ilk_rx_lne#_stat5
 */
union cvmx_ilk_rx_lnex_stat5 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat5_s {
		u64 reserved_18_63 : 46;
		u64 unkwn_word_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat5_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat5_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat5_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat5_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat5 cvmx_ilk_rx_lnex_stat5_t;

/**
 * cvmx_ilk_rx_lne#_stat6
 */
union cvmx_ilk_rx_lnex_stat6 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat6_s {
		u64 reserved_18_63 : 46;
		u64 scrm_sync_loss_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat6_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat6_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat6_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat6_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat6 cvmx_ilk_rx_lnex_stat6_t;

/**
 * cvmx_ilk_rx_lne#_stat7
 */
union cvmx_ilk_rx_lnex_stat7 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat7_s {
		u64 reserved_18_63 : 46;
		u64 scrm_match_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat7_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat7_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat7_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat7_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat7 cvmx_ilk_rx_lnex_stat7_t;

/**
 * cvmx_ilk_rx_lne#_stat8
 */
union cvmx_ilk_rx_lnex_stat8 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat8_s {
		u64 reserved_18_63 : 46;
		u64 skipw_good_cnt : 18;
	} s;
	struct cvmx_ilk_rx_lnex_stat8_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat8_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat8_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat8_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat8 cvmx_ilk_rx_lnex_stat8_t;

/**
 * cvmx_ilk_rx_lne#_stat9
 */
union cvmx_ilk_rx_lnex_stat9 {
	u64 u64;
	struct cvmx_ilk_rx_lnex_stat9_s {
		u64 reserved_50_63 : 14;
		u64 crc32_err_cnt : 18;
		u64 reserved_27_31 : 5;
		u64 crc32_match_cnt : 27;
	} s;
	struct cvmx_ilk_rx_lnex_stat9_s cn68xx;
	struct cvmx_ilk_rx_lnex_stat9_s cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat9_s cn78xx;
	struct cvmx_ilk_rx_lnex_stat9_s cn78xxp1;
};

typedef union cvmx_ilk_rx_lnex_stat9 cvmx_ilk_rx_lnex_stat9_t;

/**
 * cvmx_ilk_rxf_idx_pmap
 */
union cvmx_ilk_rxf_idx_pmap {
	u64 u64;
	struct cvmx_ilk_rxf_idx_pmap_s {
		u64 reserved_25_63 : 39;
		u64 inc : 9;
		u64 reserved_9_15 : 7;
		u64 index : 9;
	} s;
	struct cvmx_ilk_rxf_idx_pmap_s cn68xx;
	struct cvmx_ilk_rxf_idx_pmap_s cn68xxp1;
};

typedef union cvmx_ilk_rxf_idx_pmap cvmx_ilk_rxf_idx_pmap_t;

/**
 * cvmx_ilk_rxf_mem_pmap
 */
union cvmx_ilk_rxf_mem_pmap {
	u64 u64;
	struct cvmx_ilk_rxf_mem_pmap_s {
		u64 reserved_6_63 : 58;
		u64 port_kind : 6;
	} s;
	struct cvmx_ilk_rxf_mem_pmap_s cn68xx;
	struct cvmx_ilk_rxf_mem_pmap_s cn68xxp1;
};

typedef union cvmx_ilk_rxf_mem_pmap cvmx_ilk_rxf_mem_pmap_t;

/**
 * cvmx_ilk_ser_cfg
 */
union cvmx_ilk_ser_cfg {
	u64 u64;
	struct cvmx_ilk_ser_cfg_s {
		u64 reserved_57_63 : 7;
		u64 ser_rxpol_auto : 1;
		u64 ser_rxpol : 16;
		u64 ser_txpol : 16;
		u64 ser_reset_n : 16;
		u64 ser_pwrup : 4;
		u64 ser_haul : 4;
	} s;
	struct cvmx_ilk_ser_cfg_cn68xx {
		u64 reserved_57_63 : 7;
		u64 ser_rxpol_auto : 1;
		u64 reserved_48_55 : 8;
		u64 ser_rxpol : 8;
		u64 reserved_32_39 : 8;
		u64 ser_txpol : 8;
		u64 reserved_16_23 : 8;
		u64 ser_reset_n : 8;
		u64 reserved_6_7 : 2;
		u64 ser_pwrup : 2;
		u64 reserved_2_3 : 2;
		u64 ser_haul : 2;
	} cn68xx;
	struct cvmx_ilk_ser_cfg_cn68xx cn68xxp1;
	struct cvmx_ilk_ser_cfg_s cn78xx;
	struct cvmx_ilk_ser_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_ser_cfg cvmx_ilk_ser_cfg_t;

/**
 * cvmx_ilk_tx#_byte_cnt#
 */
union cvmx_ilk_txx_byte_cntx {
	u64 u64;
	struct cvmx_ilk_txx_byte_cntx_s {
		u64 reserved_40_63 : 24;
		u64 tx_bytes : 40;
	} s;
	struct cvmx_ilk_txx_byte_cntx_s cn78xx;
	struct cvmx_ilk_txx_byte_cntx_s cn78xxp1;
};

typedef union cvmx_ilk_txx_byte_cntx cvmx_ilk_txx_byte_cntx_t;

/**
 * cvmx_ilk_tx#_cal_entry#
 */
union cvmx_ilk_txx_cal_entryx {
	u64 u64;
	struct cvmx_ilk_txx_cal_entryx_s {
		u64 reserved_34_63 : 30;
		u64 ctl : 2;
		u64 reserved_8_31 : 24;
		u64 channel : 8;
	} s;
	struct cvmx_ilk_txx_cal_entryx_s cn78xx;
	struct cvmx_ilk_txx_cal_entryx_s cn78xxp1;
};

typedef union cvmx_ilk_txx_cal_entryx cvmx_ilk_txx_cal_entryx_t;

/**
 * cvmx_ilk_tx#_cfg0
 */
union cvmx_ilk_txx_cfg0 {
	u64 u64;
	struct cvmx_ilk_txx_cfg0_s {
		u64 ext_lpbk_fc : 1;
		u64 ext_lpbk : 1;
		u64 int_lpbk : 1;
		u64 txf_byp_dis : 1;
		u64 reserved_57_59 : 3;
		u64 ptrn_mode : 1;
		u64 lnk_stats_rdclr : 1;
		u64 lnk_stats_ena : 1;
		u64 mltuse_fc_ena : 1;
		u64 cal_ena : 1;
		u64 mfrm_len : 13;
		u64 brst_shrt : 7;
		u64 lane_rev : 1;
		u64 brst_max : 5;
		u64 reserved_25_25 : 1;
		u64 cal_depth : 9;
		u64 lane_ena : 16;
	} s;
	struct cvmx_ilk_txx_cfg0_cn68xx {
		u64 ext_lpbk_fc : 1;
		u64 ext_lpbk : 1;
		u64 int_lpbk : 1;
		u64 reserved_57_60 : 4;
		u64 ptrn_mode : 1;
		u64 reserved_55_55 : 1;
		u64 lnk_stats_ena : 1;
		u64 mltuse_fc_ena : 1;
		u64 cal_ena : 1;
		u64 mfrm_len : 13;
		u64 brst_shrt : 7;
		u64 lane_rev : 1;
		u64 brst_max : 5;
		u64 reserved_25_25 : 1;
		u64 cal_depth : 9;
		u64 reserved_8_15 : 8;
		u64 lane_ena : 8;
	} cn68xx;
	struct cvmx_ilk_txx_cfg0_cn68xx cn68xxp1;
	struct cvmx_ilk_txx_cfg0_s cn78xx;
	struct cvmx_ilk_txx_cfg0_s cn78xxp1;
};

typedef union cvmx_ilk_txx_cfg0 cvmx_ilk_txx_cfg0_t;

/**
 * cvmx_ilk_tx#_cfg1
 */
union cvmx_ilk_txx_cfg1 {
	u64 u64;
	struct cvmx_ilk_txx_cfg1_s {
		u64 ser_low : 4;
		u64 reserved_53_59 : 7;
		u64 brst_min : 5;
		u64 reserved_43_47 : 5;
		u64 ser_limit : 10;
		u64 pkt_busy : 1;
		u64 pipe_crd_dis : 1;
		u64 ptp_delay : 5;
		u64 skip_cnt : 4;
		u64 pkt_flush : 1;
		u64 pkt_ena : 1;
		u64 la_mode : 1;
		u64 tx_link_fc : 1;
		u64 rx_link_fc : 1;
		u64 reserved_12_16 : 5;
		u64 tx_link_fc_jam : 1;
		u64 rx_link_fc_pkt : 1;
		u64 rx_link_fc_ign : 1;
		u64 rmatch : 1;
		u64 tx_mltuse : 8;
	} s;
	struct cvmx_ilk_txx_cfg1_cn68xx {
		u64 reserved_33_63 : 31;
		u64 pkt_busy : 1;
		u64 pipe_crd_dis : 1;
		u64 ptp_delay : 5;
		u64 skip_cnt : 4;
		u64 pkt_flush : 1;
		u64 pkt_ena : 1;
		u64 la_mode : 1;
		u64 tx_link_fc : 1;
		u64 rx_link_fc : 1;
		u64 reserved_12_16 : 5;
		u64 tx_link_fc_jam : 1;
		u64 rx_link_fc_pkt : 1;
		u64 rx_link_fc_ign : 1;
		u64 rmatch : 1;
		u64 tx_mltuse : 8;
	} cn68xx;
	struct cvmx_ilk_txx_cfg1_cn68xxp1 {
		u64 reserved_32_63 : 32;
		u64 pipe_crd_dis : 1;
		u64 ptp_delay : 5;
		u64 skip_cnt : 4;
		u64 pkt_flush : 1;
		u64 pkt_ena : 1;
		u64 la_mode : 1;
		u64 tx_link_fc : 1;
		u64 rx_link_fc : 1;
		u64 reserved_12_16 : 5;
		u64 tx_link_fc_jam : 1;
		u64 rx_link_fc_pkt : 1;
		u64 rx_link_fc_ign : 1;
		u64 rmatch : 1;
		u64 tx_mltuse : 8;
	} cn68xxp1;
	struct cvmx_ilk_txx_cfg1_s cn78xx;
	struct cvmx_ilk_txx_cfg1_s cn78xxp1;
};

typedef union cvmx_ilk_txx_cfg1 cvmx_ilk_txx_cfg1_t;

/**
 * cvmx_ilk_tx#_cha_xon#
 */
union cvmx_ilk_txx_cha_xonx {
	u64 u64;
	struct cvmx_ilk_txx_cha_xonx_s {
		u64 status : 64;
	} s;
	struct cvmx_ilk_txx_cha_xonx_s cn78xx;
	struct cvmx_ilk_txx_cha_xonx_s cn78xxp1;
};

typedef union cvmx_ilk_txx_cha_xonx cvmx_ilk_txx_cha_xonx_t;

/**
 * cvmx_ilk_tx#_dbg
 */
union cvmx_ilk_txx_dbg {
	u64 u64;
	struct cvmx_ilk_txx_dbg_s {
		u64 reserved_29_63 : 35;
		u64 data_rate : 13;
		u64 low_delay : 6;
		u64 reserved_3_9 : 7;
		u64 tx_bad_crc24 : 1;
		u64 tx_bad_ctlw2 : 1;
		u64 tx_bad_ctlw1 : 1;
	} s;
	struct cvmx_ilk_txx_dbg_cn68xx {
		u64 reserved_3_63 : 61;
		u64 tx_bad_crc24 : 1;
		u64 tx_bad_ctlw2 : 1;
		u64 tx_bad_ctlw1 : 1;
	} cn68xx;
	struct cvmx_ilk_txx_dbg_cn68xx cn68xxp1;
	struct cvmx_ilk_txx_dbg_s cn78xx;
	struct cvmx_ilk_txx_dbg_s cn78xxp1;
};

typedef union cvmx_ilk_txx_dbg cvmx_ilk_txx_dbg_t;

/**
 * cvmx_ilk_tx#_err_cfg
 */
union cvmx_ilk_txx_err_cfg {
	u64 u64;
	struct cvmx_ilk_txx_err_cfg_s {
		u64 reserved_20_63 : 44;
		u64 fwc_flip : 2;
		u64 txf_flip : 2;
		u64 reserved_2_15 : 14;
		u64 fwc_cor_dis : 1;
		u64 txf_cor_dis : 1;
	} s;
	struct cvmx_ilk_txx_err_cfg_s cn78xx;
	struct cvmx_ilk_txx_err_cfg_s cn78xxp1;
};

typedef union cvmx_ilk_txx_err_cfg cvmx_ilk_txx_err_cfg_t;

/**
 * cvmx_ilk_tx#_flow_ctl0
 */
union cvmx_ilk_txx_flow_ctl0 {
	u64 u64;
	struct cvmx_ilk_txx_flow_ctl0_s {
		u64 status : 64;
	} s;
	struct cvmx_ilk_txx_flow_ctl0_s cn68xx;
	struct cvmx_ilk_txx_flow_ctl0_s cn68xxp1;
};

typedef union cvmx_ilk_txx_flow_ctl0 cvmx_ilk_txx_flow_ctl0_t;

/**
 * cvmx_ilk_tx#_flow_ctl1
 *
 * Notes:
 * Do not publish.
 *
 */
union cvmx_ilk_txx_flow_ctl1 {
	u64 u64;
	struct cvmx_ilk_txx_flow_ctl1_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_ilk_txx_flow_ctl1_s cn68xx;
	struct cvmx_ilk_txx_flow_ctl1_s cn68xxp1;
};

typedef union cvmx_ilk_txx_flow_ctl1 cvmx_ilk_txx_flow_ctl1_t;

/**
 * cvmx_ilk_tx#_idx_cal
 */
union cvmx_ilk_txx_idx_cal {
	u64 u64;
	struct cvmx_ilk_txx_idx_cal_s {
		u64 reserved_14_63 : 50;
		u64 inc : 6;
		u64 reserved_6_7 : 2;
		u64 index : 6;
	} s;
	struct cvmx_ilk_txx_idx_cal_s cn68xx;
	struct cvmx_ilk_txx_idx_cal_s cn68xxp1;
};

typedef union cvmx_ilk_txx_idx_cal cvmx_ilk_txx_idx_cal_t;

/**
 * cvmx_ilk_tx#_idx_pmap
 */
union cvmx_ilk_txx_idx_pmap {
	u64 u64;
	struct cvmx_ilk_txx_idx_pmap_s {
		u64 reserved_23_63 : 41;
		u64 inc : 7;
		u64 reserved_7_15 : 9;
		u64 index : 7;
	} s;
	struct cvmx_ilk_txx_idx_pmap_s cn68xx;
	struct cvmx_ilk_txx_idx_pmap_s cn68xxp1;
};

typedef union cvmx_ilk_txx_idx_pmap cvmx_ilk_txx_idx_pmap_t;

/**
 * cvmx_ilk_tx#_idx_stat0
 */
union cvmx_ilk_txx_idx_stat0 {
	u64 u64;
	struct cvmx_ilk_txx_idx_stat0_s {
		u64 reserved_32_63 : 32;
		u64 clr : 1;
		u64 reserved_24_30 : 7;
		u64 inc : 8;
		u64 reserved_8_15 : 8;
		u64 index : 8;
	} s;
	struct cvmx_ilk_txx_idx_stat0_s cn68xx;
	struct cvmx_ilk_txx_idx_stat0_s cn68xxp1;
};

typedef union cvmx_ilk_txx_idx_stat0 cvmx_ilk_txx_idx_stat0_t;

/**
 * cvmx_ilk_tx#_idx_stat1
 */
union cvmx_ilk_txx_idx_stat1 {
	u64 u64;
	struct cvmx_ilk_txx_idx_stat1_s {
		u64 reserved_32_63 : 32;
		u64 clr : 1;
		u64 reserved_24_30 : 7;
		u64 inc : 8;
		u64 reserved_8_15 : 8;
		u64 index : 8;
	} s;
	struct cvmx_ilk_txx_idx_stat1_s cn68xx;
	struct cvmx_ilk_txx_idx_stat1_s cn68xxp1;
};

typedef union cvmx_ilk_txx_idx_stat1 cvmx_ilk_txx_idx_stat1_t;

/**
 * cvmx_ilk_tx#_int
 */
union cvmx_ilk_txx_int {
	u64 u64;
	struct cvmx_ilk_txx_int_s {
		u64 reserved_8_63 : 56;
		u64 fwc_dbe : 1;
		u64 fwc_sbe : 1;
		u64 txf_dbe : 1;
		u64 txf_sbe : 1;
		u64 stat_cnt_ovfl : 1;
		u64 bad_pipe : 1;
		u64 bad_seq : 1;
		u64 txf_err : 1;
	} s;
	struct cvmx_ilk_txx_int_cn68xx {
		u64 reserved_4_63 : 60;
		u64 stat_cnt_ovfl : 1;
		u64 bad_pipe : 1;
		u64 bad_seq : 1;
		u64 txf_err : 1;
	} cn68xx;
	struct cvmx_ilk_txx_int_cn68xx cn68xxp1;
	struct cvmx_ilk_txx_int_s cn78xx;
	struct cvmx_ilk_txx_int_s cn78xxp1;
};

typedef union cvmx_ilk_txx_int cvmx_ilk_txx_int_t;

/**
 * cvmx_ilk_tx#_int_en
 */
union cvmx_ilk_txx_int_en {
	u64 u64;
	struct cvmx_ilk_txx_int_en_s {
		u64 reserved_4_63 : 60;
		u64 stat_cnt_ovfl : 1;
		u64 bad_pipe : 1;
		u64 bad_seq : 1;
		u64 txf_err : 1;
	} s;
	struct cvmx_ilk_txx_int_en_s cn68xx;
	struct cvmx_ilk_txx_int_en_s cn68xxp1;
};

typedef union cvmx_ilk_txx_int_en cvmx_ilk_txx_int_en_t;

/**
 * cvmx_ilk_tx#_mem_cal0
 *
 * Notes:
 * Software must always read ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 *
 * Software must always write ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 */
union cvmx_ilk_txx_mem_cal0 {
	u64 u64;
	struct cvmx_ilk_txx_mem_cal0_s {
		u64 reserved_36_63 : 28;
		u64 entry_ctl3 : 2;
		u64 reserved_33_33 : 1;
		u64 bpid3 : 6;
		u64 entry_ctl2 : 2;
		u64 reserved_24_24 : 1;
		u64 bpid2 : 6;
		u64 entry_ctl1 : 2;
		u64 reserved_15_15 : 1;
		u64 bpid1 : 6;
		u64 entry_ctl0 : 2;
		u64 reserved_6_6 : 1;
		u64 bpid0 : 6;
	} s;
	struct cvmx_ilk_txx_mem_cal0_s cn68xx;
	struct cvmx_ilk_txx_mem_cal0_s cn68xxp1;
};

typedef union cvmx_ilk_txx_mem_cal0 cvmx_ilk_txx_mem_cal0_t;

/**
 * cvmx_ilk_tx#_mem_cal1
 *
 * Notes:
 * Software must always read ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 *
 * Software must always write ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 */
union cvmx_ilk_txx_mem_cal1 {
	u64 u64;
	struct cvmx_ilk_txx_mem_cal1_s {
		u64 reserved_36_63 : 28;
		u64 entry_ctl7 : 2;
		u64 reserved_33_33 : 1;
		u64 bpid7 : 6;
		u64 entry_ctl6 : 2;
		u64 reserved_24_24 : 1;
		u64 bpid6 : 6;
		u64 entry_ctl5 : 2;
		u64 reserved_15_15 : 1;
		u64 bpid5 : 6;
		u64 entry_ctl4 : 2;
		u64 reserved_6_6 : 1;
		u64 bpid4 : 6;
	} s;
	struct cvmx_ilk_txx_mem_cal1_s cn68xx;
	struct cvmx_ilk_txx_mem_cal1_s cn68xxp1;
};

typedef union cvmx_ilk_txx_mem_cal1 cvmx_ilk_txx_mem_cal1_t;

/**
 * cvmx_ilk_tx#_mem_pmap
 */
union cvmx_ilk_txx_mem_pmap {
	u64 u64;
	struct cvmx_ilk_txx_mem_pmap_s {
		u64 reserved_17_63 : 47;
		u64 remap : 1;
		u64 reserved_8_15 : 8;
		u64 channel : 8;
	} s;
	struct cvmx_ilk_txx_mem_pmap_s cn68xx;
	struct cvmx_ilk_txx_mem_pmap_cn68xxp1 {
		u64 reserved_8_63 : 56;
		u64 channel : 8;
	} cn68xxp1;
};

typedef union cvmx_ilk_txx_mem_pmap cvmx_ilk_txx_mem_pmap_t;

/**
 * cvmx_ilk_tx#_mem_stat0
 */
union cvmx_ilk_txx_mem_stat0 {
	u64 u64;
	struct cvmx_ilk_txx_mem_stat0_s {
		u64 reserved_28_63 : 36;
		u64 tx_pkt : 28;
	} s;
	struct cvmx_ilk_txx_mem_stat0_s cn68xx;
	struct cvmx_ilk_txx_mem_stat0_s cn68xxp1;
};

typedef union cvmx_ilk_txx_mem_stat0 cvmx_ilk_txx_mem_stat0_t;

/**
 * cvmx_ilk_tx#_mem_stat1
 */
union cvmx_ilk_txx_mem_stat1 {
	u64 u64;
	struct cvmx_ilk_txx_mem_stat1_s {
		u64 reserved_36_63 : 28;
		u64 tx_bytes : 36;
	} s;
	struct cvmx_ilk_txx_mem_stat1_s cn68xx;
	struct cvmx_ilk_txx_mem_stat1_s cn68xxp1;
};

typedef union cvmx_ilk_txx_mem_stat1 cvmx_ilk_txx_mem_stat1_t;

/**
 * cvmx_ilk_tx#_pipe
 */
union cvmx_ilk_txx_pipe {
	u64 u64;
	struct cvmx_ilk_txx_pipe_s {
		u64 reserved_24_63 : 40;
		u64 nump : 8;
		u64 reserved_7_15 : 9;
		u64 base : 7;
	} s;
	struct cvmx_ilk_txx_pipe_s cn68xx;
	struct cvmx_ilk_txx_pipe_s cn68xxp1;
};

typedef union cvmx_ilk_txx_pipe cvmx_ilk_txx_pipe_t;

/**
 * cvmx_ilk_tx#_pkt_cnt#
 */
union cvmx_ilk_txx_pkt_cntx {
	u64 u64;
	struct cvmx_ilk_txx_pkt_cntx_s {
		u64 reserved_34_63 : 30;
		u64 tx_pkt : 34;
	} s;
	struct cvmx_ilk_txx_pkt_cntx_s cn78xx;
	struct cvmx_ilk_txx_pkt_cntx_s cn78xxp1;
};

typedef union cvmx_ilk_txx_pkt_cntx cvmx_ilk_txx_pkt_cntx_t;

/**
 * cvmx_ilk_tx#_rmatch
 */
union cvmx_ilk_txx_rmatch {
	u64 u64;
	struct cvmx_ilk_txx_rmatch_s {
		u64 reserved_50_63 : 14;
		u64 grnlrty : 2;
		u64 brst_limit : 16;
		u64 time_limit : 16;
		u64 rate_limit : 16;
	} s;
	struct cvmx_ilk_txx_rmatch_s cn68xx;
	struct cvmx_ilk_txx_rmatch_s cn68xxp1;
	struct cvmx_ilk_txx_rmatch_s cn78xx;
	struct cvmx_ilk_txx_rmatch_s cn78xxp1;
};

typedef union cvmx_ilk_txx_rmatch cvmx_ilk_txx_rmatch_t;

#endif
