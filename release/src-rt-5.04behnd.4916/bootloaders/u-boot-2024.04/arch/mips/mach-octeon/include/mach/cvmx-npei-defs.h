/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon npei.
 */

#ifndef __CVMX_NPEI_DEFS_H__
#define __CVMX_NPEI_DEFS_H__

#define CVMX_NPEI_BAR1_INDEXX(offset)                                          \
	(0x0000000000000000ull + ((offset) & 31) * 16)
#define CVMX_NPEI_BIST_STATUS	 (0x0000000000000580ull)
#define CVMX_NPEI_BIST_STATUS2	 (0x0000000000000680ull)
#define CVMX_NPEI_CTL_PORT0	 (0x0000000000000250ull)
#define CVMX_NPEI_CTL_PORT1	 (0x0000000000000260ull)
#define CVMX_NPEI_CTL_STATUS	 (0x0000000000000570ull)
#define CVMX_NPEI_CTL_STATUS2	 (0x0000000000003C00ull)
#define CVMX_NPEI_DATA_OUT_CNT	 (0x00000000000005F0ull)
#define CVMX_NPEI_DBG_DATA	 (0x0000000000000510ull)
#define CVMX_NPEI_DBG_SELECT	 (0x0000000000000500ull)
#define CVMX_NPEI_DMA0_INT_LEVEL (0x00000000000005C0ull)
#define CVMX_NPEI_DMA1_INT_LEVEL (0x00000000000005D0ull)
#define CVMX_NPEI_DMAX_COUNTS(offset)                                          \
	(0x0000000000000450ull + ((offset) & 7) * 16)
#define CVMX_NPEI_DMAX_DBELL(offset) (0x00000000000003B0ull + ((offset) & 7) * 16)
#define CVMX_NPEI_DMAX_IBUFF_SADDR(offset)                                     \
	(0x0000000000000400ull + ((offset) & 7) * 16)
#define CVMX_NPEI_DMAX_NADDR(offset) (0x00000000000004A0ull + ((offset) & 7) * 16)
#define CVMX_NPEI_DMA_CNTS	     (0x00000000000005E0ull)
#define CVMX_NPEI_DMA_CONTROL	     (0x00000000000003A0ull)
#define CVMX_NPEI_DMA_PCIE_REQ_NUM   (0x00000000000005B0ull)
#define CVMX_NPEI_DMA_STATE1	     (0x00000000000006C0ull)
#define CVMX_NPEI_DMA_STATE1_P1	     (0x0000000000000680ull)
#define CVMX_NPEI_DMA_STATE2	     (0x00000000000006D0ull)
#define CVMX_NPEI_DMA_STATE2_P1	     (0x0000000000000690ull)
#define CVMX_NPEI_DMA_STATE3_P1	     (0x00000000000006A0ull)
#define CVMX_NPEI_DMA_STATE4_P1	     (0x00000000000006B0ull)
#define CVMX_NPEI_DMA_STATE5_P1	     (0x00000000000006C0ull)
#define CVMX_NPEI_INT_A_ENB	     (0x0000000000000560ull)
#define CVMX_NPEI_INT_A_ENB2	     (0x0000000000003CE0ull)
#define CVMX_NPEI_INT_A_SUM	     (0x0000000000000550ull)
#define CVMX_NPEI_INT_ENB	     (0x0000000000000540ull)
#define CVMX_NPEI_INT_ENB2	     (0x0000000000003CD0ull)
#define CVMX_NPEI_INT_INFO	     (0x0000000000000590ull)
#define CVMX_NPEI_INT_SUM	     (0x0000000000000530ull)
#define CVMX_NPEI_INT_SUM2	     (0x0000000000003CC0ull)
#define CVMX_NPEI_LAST_WIN_RDATA0    (0x0000000000000600ull)
#define CVMX_NPEI_LAST_WIN_RDATA1    (0x0000000000000610ull)
#define CVMX_NPEI_MEM_ACCESS_CTL     (0x00000000000004F0ull)
#define CVMX_NPEI_MEM_ACCESS_SUBIDX(offset)                                    \
	(0x0000000000000280ull + ((offset) & 31) * 16 - 16 * 12)
#define CVMX_NPEI_MSI_ENB0	    (0x0000000000003C50ull)
#define CVMX_NPEI_MSI_ENB1	    (0x0000000000003C60ull)
#define CVMX_NPEI_MSI_ENB2	    (0x0000000000003C70ull)
#define CVMX_NPEI_MSI_ENB3	    (0x0000000000003C80ull)
#define CVMX_NPEI_MSI_RCV0	    (0x0000000000003C10ull)
#define CVMX_NPEI_MSI_RCV1	    (0x0000000000003C20ull)
#define CVMX_NPEI_MSI_RCV2	    (0x0000000000003C30ull)
#define CVMX_NPEI_MSI_RCV3	    (0x0000000000003C40ull)
#define CVMX_NPEI_MSI_RD_MAP	    (0x0000000000003CA0ull)
#define CVMX_NPEI_MSI_W1C_ENB0	    (0x0000000000003CF0ull)
#define CVMX_NPEI_MSI_W1C_ENB1	    (0x0000000000003D00ull)
#define CVMX_NPEI_MSI_W1C_ENB2	    (0x0000000000003D10ull)
#define CVMX_NPEI_MSI_W1C_ENB3	    (0x0000000000003D20ull)
#define CVMX_NPEI_MSI_W1S_ENB0	    (0x0000000000003D30ull)
#define CVMX_NPEI_MSI_W1S_ENB1	    (0x0000000000003D40ull)
#define CVMX_NPEI_MSI_W1S_ENB2	    (0x0000000000003D50ull)
#define CVMX_NPEI_MSI_W1S_ENB3	    (0x0000000000003D60ull)
#define CVMX_NPEI_MSI_WR_MAP	    (0x0000000000003C90ull)
#define CVMX_NPEI_PCIE_CREDIT_CNT   (0x0000000000003D70ull)
#define CVMX_NPEI_PCIE_MSI_RCV	    (0x0000000000003CB0ull)
#define CVMX_NPEI_PCIE_MSI_RCV_B1   (0x0000000000000650ull)
#define CVMX_NPEI_PCIE_MSI_RCV_B2   (0x0000000000000660ull)
#define CVMX_NPEI_PCIE_MSI_RCV_B3   (0x0000000000000670ull)
#define CVMX_NPEI_PKTX_CNTS(offset) (0x0000000000002400ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_INSTR_BADDR(offset)                                     \
	(0x0000000000002800ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_INSTR_BAOFF_DBELL(offset)                               \
	(0x0000000000002C00ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_INSTR_FIFO_RSIZE(offset)                                \
	(0x0000000000003000ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_INSTR_HEADER(offset)                                    \
	(0x0000000000003400ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_IN_BP(offset)                                           \
	(0x0000000000003800ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_SLIST_BADDR(offset)                                     \
	(0x0000000000001400ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_SLIST_BAOFF_DBELL(offset)                               \
	(0x0000000000001800ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKTX_SLIST_FIFO_RSIZE(offset)                                \
	(0x0000000000001C00ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKT_CNT_INT	    (0x0000000000001110ull)
#define CVMX_NPEI_PKT_CNT_INT_ENB   (0x0000000000001130ull)
#define CVMX_NPEI_PKT_DATA_OUT_ES   (0x00000000000010B0ull)
#define CVMX_NPEI_PKT_DATA_OUT_NS   (0x00000000000010A0ull)
#define CVMX_NPEI_PKT_DATA_OUT_ROR  (0x0000000000001090ull)
#define CVMX_NPEI_PKT_DPADDR	    (0x0000000000001080ull)
#define CVMX_NPEI_PKT_INPUT_CONTROL (0x0000000000001150ull)
#define CVMX_NPEI_PKT_INSTR_ENB	    (0x0000000000001000ull)
#define CVMX_NPEI_PKT_INSTR_RD_SIZE (0x0000000000001190ull)
#define CVMX_NPEI_PKT_INSTR_SIZE    (0x0000000000001020ull)
#define CVMX_NPEI_PKT_INT_LEVELS    (0x0000000000001100ull)
#define CVMX_NPEI_PKT_IN_BP	    (0x00000000000006B0ull)
#define CVMX_NPEI_PKT_IN_DONEX_CNTS(offset)                                    \
	(0x0000000000002000ull + ((offset) & 31) * 16)
#define CVMX_NPEI_PKT_IN_INSTR_COUNTS (0x00000000000006A0ull)
#define CVMX_NPEI_PKT_IN_PCIE_PORT    (0x00000000000011A0ull)
#define CVMX_NPEI_PKT_IPTR	      (0x0000000000001070ull)
#define CVMX_NPEI_PKT_OUTPUT_WMARK    (0x0000000000001160ull)
#define CVMX_NPEI_PKT_OUT_BMODE	      (0x00000000000010D0ull)
#define CVMX_NPEI_PKT_OUT_ENB	      (0x0000000000001010ull)
#define CVMX_NPEI_PKT_PCIE_PORT	      (0x00000000000010E0ull)
#define CVMX_NPEI_PKT_PORT_IN_RST     (0x0000000000000690ull)
#define CVMX_NPEI_PKT_SLIST_ES	      (0x0000000000001050ull)
#define CVMX_NPEI_PKT_SLIST_ID_SIZE   (0x0000000000001180ull)
#define CVMX_NPEI_PKT_SLIST_NS	      (0x0000000000001040ull)
#define CVMX_NPEI_PKT_SLIST_ROR	      (0x0000000000001030ull)
#define CVMX_NPEI_PKT_TIME_INT	      (0x0000000000001120ull)
#define CVMX_NPEI_PKT_TIME_INT_ENB    (0x0000000000001140ull)
#define CVMX_NPEI_RSL_INT_BLOCKS      (0x0000000000000520ull)
#define CVMX_NPEI_SCRATCH_1	      (0x0000000000000270ull)
#define CVMX_NPEI_STATE1	      (0x0000000000000620ull)
#define CVMX_NPEI_STATE2	      (0x0000000000000630ull)
#define CVMX_NPEI_STATE3	      (0x0000000000000640ull)
#define CVMX_NPEI_WINDOW_CTL	      (0x0000000000000380ull)
#define CVMX_NPEI_WIN_RD_ADDR	      (0x0000000000000210ull)
#define CVMX_NPEI_WIN_RD_DATA	      (0x0000000000000240ull)
#define CVMX_NPEI_WIN_WR_ADDR	      (0x0000000000000200ull)
#define CVMX_NPEI_WIN_WR_DATA	      (0x0000000000000220ull)
#define CVMX_NPEI_WIN_WR_MASK	      (0x0000000000000230ull)

/**
 * cvmx_npei_bar1_index#
 *
 * Total Address is 16Kb; 0x0000 - 0x3fff, 0x000 - 0x7fe(Reg, every other 8B)
 *
 * General  5kb; 0x0000 - 0x13ff, 0x000 - 0x27e(Reg-General)
 * PktMem  10Kb; 0x1400 - 0x3bff, 0x280 - 0x77e(Reg-General-Packet)
 * Rsvd     1Kb; 0x3c00 - 0x3fff, 0x780 - 0x7fe(Reg-NCB Only Mode)
 *                                   == NPEI_PKT_CNT_INT_ENB[PORT]
 *                                   == NPEI_PKT_TIME_INT_ENB[PORT]
 *                                   == NPEI_PKT_CNT_INT[PORT]
 *                                   == NPEI_PKT_TIME_INT[PORT]
 *                                   == NPEI_PKT_PCIE_PORT[PP]
 *                                   == NPEI_PKT_SLIST_ROR[ROR]
 *                                   == NPEI_PKT_SLIST_ROR[NSR] ?
 *                                   == NPEI_PKT_SLIST_ES[ES]
 *                                   == NPEI_PKTn_SLIST_BAOFF_DBELL[AOFF]
 *                                   == NPEI_PKTn_SLIST_BAOFF_DBELL[DBELL]
 *                                   == NPEI_PKTn_CNTS[CNT]
 * NPEI_CTL_STATUS[OUTn_ENB]         == NPEI_PKT_OUT_ENB[ENB]
 * NPEI_BASE_ADDRESS_OUTPUTn[BADDR]  == NPEI_PKTn_SLIST_BADDR[ADDR]
 * NPEI_DESC_OUTPUTn[SIZE]           == NPEI_PKTn_SLIST_FIFO_RSIZE[RSIZE]
 * NPEI_Pn_DBPAIR_ADDR[NADDR]        == NPEI_PKTn_SLIST_BADDR[ADDR] +
 *                                      NPEI_PKTn_SLIST_BAOFF_DBELL[AOFF]
 * NPEI_PKT_CREDITSn[PTR_CNT]        == NPEI_PKTn_SLIST_BAOFF_DBELL[DBELL]
 * NPEI_P0_PAIR_CNTS[AVAIL]          == NPEI_PKTn_SLIST_BAOFF_DBELL[DBELL]
 * NPEI_P0_PAIR_CNTS[FCNT]           ==
 * NPEI_PKTS_SENTn[PKT_CNT]          == NPEI_PKTn_CNTS[CNT]
 * NPEI_OUTPUT_CONTROL[Pn_BMODE]     == NPEI_PKT_OUT_BMODE[BMODE]
 * NPEI_PKT_CREDITSn[PKT_CNT]        == NPEI_PKTn_CNTS[CNT]
 * NPEI_BUFF_SIZE_OUTPUTn[BSIZE]     == NPEI_PKT_SLIST_ID_SIZE[BSIZE]
 * NPEI_BUFF_SIZE_OUTPUTn[ISIZE]     == NPEI_PKT_SLIST_ID_SIZE[ISIZE]
 * NPEI_OUTPUT_CONTROL[On_CSRM]      == NPEI_PKT_DPADDR[DPTR] &
 *                                      NPEI_PKT_OUT_USE_IPTR[PORT]
 * NPEI_OUTPUT_CONTROL[On_ES]        == NPEI_PKT_DATA_OUT_ES[ES]
 * NPEI_OUTPUT_CONTROL[On_NS]        == NPEI_PKT_DATA_OUT_NS[NSR] ?
 * NPEI_OUTPUT_CONTROL[On_RO]        == NPEI_PKT_DATA_OUT_ROR[ROR]
 * NPEI_PKTS_SENT_INT_LEVn[PKT_CNT]  == NPEI_PKT_INT_LEVELS[CNT]
 * NPEI_PKTS_SENT_TIMEn[PKT_TIME]    == NPEI_PKT_INT_LEVELS[TIME]
 * NPEI_OUTPUT_CONTROL[IPTR_On]      == NPEI_PKT_IPTR[IPTR]
 * NPEI_PCIE_PORT_OUTPUT[]           == NPEI_PKT_PCIE_PORT[PP]
 *
 *                  NPEI_BAR1_INDEXX = NPEI BAR1 IndexX Register
 *
 * Contains address index and control bits for access to memory ranges of
 * BAR-1. Index is build from supplied address [25:22].
 * NPEI_BAR1_INDEX0 through NPEI_BAR1_INDEX15 is used for transactions
 * orginating with PCIE-PORT0 and NPEI_BAR1_INDEX16
 * through NPEI_BAR1_INDEX31 is used for transactions originating with
 * PCIE-PORT1.
 */
union cvmx_npei_bar1_indexx {
	u32 u32;
	struct cvmx_npei_bar1_indexx_s {
		u32 reserved_18_31 : 14;
		u32 addr_idx : 14;
		u32 ca : 1;
		u32 end_swp : 2;
		u32 addr_v : 1;
	} s;
	struct cvmx_npei_bar1_indexx_s cn52xx;
	struct cvmx_npei_bar1_indexx_s cn52xxp1;
	struct cvmx_npei_bar1_indexx_s cn56xx;
	struct cvmx_npei_bar1_indexx_s cn56xxp1;
};

typedef union cvmx_npei_bar1_indexx cvmx_npei_bar1_indexx_t;

/**
 * cvmx_npei_bist_status
 *
 * NPEI_BIST_STATUS = NPI's BIST Status Register
 *
 * Results from BIST runs of NPEI's memories.
 */
union cvmx_npei_bist_status {
	u64 u64;
	struct cvmx_npei_bist_status_s {
		u64 pkt_rdf : 1;
		u64 reserved_60_62 : 3;
		u64 pcr_gim : 1;
		u64 pkt_pif : 1;
		u64 pcsr_int : 1;
		u64 pcsr_im : 1;
		u64 pcsr_cnt : 1;
		u64 pcsr_id : 1;
		u64 pcsr_sl : 1;
		u64 reserved_50_52 : 3;
		u64 pkt_ind : 1;
		u64 pkt_slm : 1;
		u64 reserved_36_47 : 12;
		u64 d0_pst : 1;
		u64 d1_pst : 1;
		u64 d2_pst : 1;
		u64 d3_pst : 1;
		u64 reserved_31_31 : 1;
		u64 n2p0_c : 1;
		u64 n2p0_o : 1;
		u64 n2p1_c : 1;
		u64 n2p1_o : 1;
		u64 cpl_p0 : 1;
		u64 cpl_p1 : 1;
		u64 p2n1_po : 1;
		u64 p2n1_no : 1;
		u64 p2n1_co : 1;
		u64 p2n0_po : 1;
		u64 p2n0_no : 1;
		u64 p2n0_co : 1;
		u64 p2n0_c0 : 1;
		u64 p2n0_c1 : 1;
		u64 p2n0_n : 1;
		u64 p2n0_p0 : 1;
		u64 p2n0_p1 : 1;
		u64 p2n1_c0 : 1;
		u64 p2n1_c1 : 1;
		u64 p2n1_n : 1;
		u64 p2n1_p0 : 1;
		u64 p2n1_p1 : 1;
		u64 csm0 : 1;
		u64 csm1 : 1;
		u64 dif0 : 1;
		u64 dif1 : 1;
		u64 dif2 : 1;
		u64 dif3 : 1;
		u64 reserved_2_2 : 1;
		u64 msi : 1;
		u64 ncb_cmd : 1;
	} s;
	struct cvmx_npei_bist_status_cn52xx {
		u64 pkt_rdf : 1;
		u64 reserved_60_62 : 3;
		u64 pcr_gim : 1;
		u64 pkt_pif : 1;
		u64 pcsr_int : 1;
		u64 pcsr_im : 1;
		u64 pcsr_cnt : 1;
		u64 pcsr_id : 1;
		u64 pcsr_sl : 1;
		u64 pkt_imem : 1;
		u64 pkt_pfm : 1;
		u64 pkt_pof : 1;
		u64 reserved_48_49 : 2;
		u64 pkt_pop0 : 1;
		u64 pkt_pop1 : 1;
		u64 d0_mem : 1;
		u64 d1_mem : 1;
		u64 d2_mem : 1;
		u64 d3_mem : 1;
		u64 d4_mem : 1;
		u64 ds_mem : 1;
		u64 reserved_36_39 : 4;
		u64 d0_pst : 1;
		u64 d1_pst : 1;
		u64 d2_pst : 1;
		u64 d3_pst : 1;
		u64 d4_pst : 1;
		u64 n2p0_c : 1;
		u64 n2p0_o : 1;
		u64 n2p1_c : 1;
		u64 n2p1_o : 1;
		u64 cpl_p0 : 1;
		u64 cpl_p1 : 1;
		u64 p2n1_po : 1;
		u64 p2n1_no : 1;
		u64 p2n1_co : 1;
		u64 p2n0_po : 1;
		u64 p2n0_no : 1;
		u64 p2n0_co : 1;
		u64 p2n0_c0 : 1;
		u64 p2n0_c1 : 1;
		u64 p2n0_n : 1;
		u64 p2n0_p0 : 1;
		u64 p2n0_p1 : 1;
		u64 p2n1_c0 : 1;
		u64 p2n1_c1 : 1;
		u64 p2n1_n : 1;
		u64 p2n1_p0 : 1;
		u64 p2n1_p1 : 1;
		u64 csm0 : 1;
		u64 csm1 : 1;
		u64 dif0 : 1;
		u64 dif1 : 1;
		u64 dif2 : 1;
		u64 dif3 : 1;
		u64 dif4 : 1;
		u64 msi : 1;
		u64 ncb_cmd : 1;
	} cn52xx;
	struct cvmx_npei_bist_status_cn52xxp1 {
		u64 reserved_46_63 : 18;
		u64 d0_mem0 : 1;
		u64 d1_mem1 : 1;
		u64 d2_mem2 : 1;
		u64 d3_mem3 : 1;
		u64 dr0_mem : 1;
		u64 d0_mem : 1;
		u64 d1_mem : 1;
		u64 d2_mem : 1;
		u64 d3_mem : 1;
		u64 dr1_mem : 1;
		u64 d0_pst : 1;
		u64 d1_pst : 1;
		u64 d2_pst : 1;
		u64 d3_pst : 1;
		u64 dr2_mem : 1;
		u64 n2p0_c : 1;
		u64 n2p0_o : 1;
		u64 n2p1_c : 1;
		u64 n2p1_o : 1;
		u64 cpl_p0 : 1;
		u64 cpl_p1 : 1;
		u64 p2n1_po : 1;
		u64 p2n1_no : 1;
		u64 p2n1_co : 1;
		u64 p2n0_po : 1;
		u64 p2n0_no : 1;
		u64 p2n0_co : 1;
		u64 p2n0_c0 : 1;
		u64 p2n0_c1 : 1;
		u64 p2n0_n : 1;
		u64 p2n0_p0 : 1;
		u64 p2n0_p1 : 1;
		u64 p2n1_c0 : 1;
		u64 p2n1_c1 : 1;
		u64 p2n1_n : 1;
		u64 p2n1_p0 : 1;
		u64 p2n1_p1 : 1;
		u64 csm0 : 1;
		u64 csm1 : 1;
		u64 dif0 : 1;
		u64 dif1 : 1;
		u64 dif2 : 1;
		u64 dif3 : 1;
		u64 dr3_mem : 1;
		u64 msi : 1;
		u64 ncb_cmd : 1;
	} cn52xxp1;
	struct cvmx_npei_bist_status_cn52xx cn56xx;
	struct cvmx_npei_bist_status_cn56xxp1 {
		u64 reserved_58_63 : 6;
		u64 pcsr_int : 1;
		u64 pcsr_im : 1;
		u64 pcsr_cnt : 1;
		u64 pcsr_id : 1;
		u64 pcsr_sl : 1;
		u64 pkt_pout : 1;
		u64 pkt_imem : 1;
		u64 pkt_cntm : 1;
		u64 pkt_ind : 1;
		u64 pkt_slm : 1;
		u64 pkt_odf : 1;
		u64 pkt_oif : 1;
		u64 pkt_out : 1;
		u64 pkt_i0 : 1;
		u64 pkt_i1 : 1;
		u64 pkt_s0 : 1;
		u64 pkt_s1 : 1;
		u64 d0_mem : 1;
		u64 d1_mem : 1;
		u64 d2_mem : 1;
		u64 d3_mem : 1;
		u64 d4_mem : 1;
		u64 d0_pst : 1;
		u64 d1_pst : 1;
		u64 d2_pst : 1;
		u64 d3_pst : 1;
		u64 d4_pst : 1;
		u64 n2p0_c : 1;
		u64 n2p0_o : 1;
		u64 n2p1_c : 1;
		u64 n2p1_o : 1;
		u64 cpl_p0 : 1;
		u64 cpl_p1 : 1;
		u64 p2n1_po : 1;
		u64 p2n1_no : 1;
		u64 p2n1_co : 1;
		u64 p2n0_po : 1;
		u64 p2n0_no : 1;
		u64 p2n0_co : 1;
		u64 p2n0_c0 : 1;
		u64 p2n0_c1 : 1;
		u64 p2n0_n : 1;
		u64 p2n0_p0 : 1;
		u64 p2n0_p1 : 1;
		u64 p2n1_c0 : 1;
		u64 p2n1_c1 : 1;
		u64 p2n1_n : 1;
		u64 p2n1_p0 : 1;
		u64 p2n1_p1 : 1;
		u64 csm0 : 1;
		u64 csm1 : 1;
		u64 dif0 : 1;
		u64 dif1 : 1;
		u64 dif2 : 1;
		u64 dif3 : 1;
		u64 dif4 : 1;
		u64 msi : 1;
		u64 ncb_cmd : 1;
	} cn56xxp1;
};

typedef union cvmx_npei_bist_status cvmx_npei_bist_status_t;

/**
 * cvmx_npei_bist_status2
 *
 * NPEI_BIST_STATUS2 = NPI's BIST Status Register2
 *
 * Results from BIST runs of NPEI's memories.
 */
union cvmx_npei_bist_status2 {
	u64 u64;
	struct cvmx_npei_bist_status2_s {
		u64 reserved_14_63 : 50;
		u64 prd_tag : 1;
		u64 prd_st0 : 1;
		u64 prd_st1 : 1;
		u64 prd_err : 1;
		u64 nrd_st : 1;
		u64 nwe_st : 1;
		u64 nwe_wr0 : 1;
		u64 nwe_wr1 : 1;
		u64 pkt_rd : 1;
		u64 psc_p0 : 1;
		u64 psc_p1 : 1;
		u64 pkt_gd : 1;
		u64 pkt_gl : 1;
		u64 pkt_blk : 1;
	} s;
	struct cvmx_npei_bist_status2_s cn52xx;
	struct cvmx_npei_bist_status2_s cn56xx;
};

typedef union cvmx_npei_bist_status2 cvmx_npei_bist_status2_t;

/**
 * cvmx_npei_ctl_port0
 *
 * NPEI_CTL_PORT0 = NPEI's Control Port 0
 *
 * Contains control for access for Port0
 */
union cvmx_npei_ctl_port0 {
	u64 u64;
	struct cvmx_npei_ctl_port0_s {
		u64 reserved_21_63 : 43;
		u64 waitl_com : 1;
		u64 intd : 1;
		u64 intc : 1;
		u64 intb : 1;
		u64 inta : 1;
		u64 intd_map : 2;
		u64 intc_map : 2;
		u64 intb_map : 2;
		u64 inta_map : 2;
		u64 ctlp_ro : 1;
		u64 reserved_6_6 : 1;
		u64 ptlp_ro : 1;
		u64 bar2_enb : 1;
		u64 bar2_esx : 2;
		u64 bar2_cax : 1;
		u64 wait_com : 1;
	} s;
	struct cvmx_npei_ctl_port0_s cn52xx;
	struct cvmx_npei_ctl_port0_s cn52xxp1;
	struct cvmx_npei_ctl_port0_s cn56xx;
	struct cvmx_npei_ctl_port0_s cn56xxp1;
};

typedef union cvmx_npei_ctl_port0 cvmx_npei_ctl_port0_t;

/**
 * cvmx_npei_ctl_port1
 *
 * NPEI_CTL_PORT1 = NPEI's Control Port1
 *
 * Contains control for access for Port1
 */
union cvmx_npei_ctl_port1 {
	u64 u64;
	struct cvmx_npei_ctl_port1_s {
		u64 reserved_21_63 : 43;
		u64 waitl_com : 1;
		u64 intd : 1;
		u64 intc : 1;
		u64 intb : 1;
		u64 inta : 1;
		u64 intd_map : 2;
		u64 intc_map : 2;
		u64 intb_map : 2;
		u64 inta_map : 2;
		u64 ctlp_ro : 1;
		u64 reserved_6_6 : 1;
		u64 ptlp_ro : 1;
		u64 bar2_enb : 1;
		u64 bar2_esx : 2;
		u64 bar2_cax : 1;
		u64 wait_com : 1;
	} s;
	struct cvmx_npei_ctl_port1_s cn52xx;
	struct cvmx_npei_ctl_port1_s cn52xxp1;
	struct cvmx_npei_ctl_port1_s cn56xx;
	struct cvmx_npei_ctl_port1_s cn56xxp1;
};

typedef union cvmx_npei_ctl_port1 cvmx_npei_ctl_port1_t;

/**
 * cvmx_npei_ctl_status
 *
 * NPEI_CTL_STATUS = NPEI Control Status Register
 *
 * Contains control and status for NPEI. Writes to this register are not
 * oSrdered with writes/reads to the PCIe Memory space.
 * To ensure that a write has completed the user must read the register
 * before making an access(i.e. PCIe memory space)
 * that requires the value of this register to be updated.
 */
union cvmx_npei_ctl_status {
	u64 u64;
	struct cvmx_npei_ctl_status_s {
		u64 reserved_44_63 : 20;
		u64 p1_ntags : 6;
		u64 p0_ntags : 6;
		u64 cfg_rtry : 16;
		u64 ring_en : 1;
		u64 lnk_rst : 1;
		u64 arb : 1;
		u64 pkt_bp : 4;
		u64 host_mode : 1;
		u64 chip_rev : 8;
	} s;
	struct cvmx_npei_ctl_status_s cn52xx;
	struct cvmx_npei_ctl_status_cn52xxp1 {
		u64 reserved_44_63 : 20;
		u64 p1_ntags : 6;
		u64 p0_ntags : 6;
		u64 cfg_rtry : 16;
		u64 reserved_15_15 : 1;
		u64 lnk_rst : 1;
		u64 arb : 1;
		u64 reserved_9_12 : 4;
		u64 host_mode : 1;
		u64 chip_rev : 8;
	} cn52xxp1;
	struct cvmx_npei_ctl_status_s cn56xx;
	struct cvmx_npei_ctl_status_cn56xxp1 {
		u64 reserved_15_63 : 49;
		u64 lnk_rst : 1;
		u64 arb : 1;
		u64 pkt_bp : 4;
		u64 host_mode : 1;
		u64 chip_rev : 8;
	} cn56xxp1;
};

typedef union cvmx_npei_ctl_status cvmx_npei_ctl_status_t;

/**
 * cvmx_npei_ctl_status2
 *
 * NPEI_CTL_STATUS2 = NPEI's Control Status2 Register
 *
 * Contains control and status for NPEI.
 * Writes to this register are not ordered with writes/reads to the PCI
 * Memory space.
 * To ensure that a write has completed the user must read the register before
 * making an access(i.e. PCI memory space) that requires the value of this
 * register to be updated.
 */
union cvmx_npei_ctl_status2 {
	u64 u64;
	struct cvmx_npei_ctl_status2_s {
		u64 reserved_16_63 : 48;
		u64 mps : 1;
		u64 mrrs : 3;
		u64 c1_w_flt : 1;
		u64 c0_w_flt : 1;
		u64 c1_b1_s : 3;
		u64 c0_b1_s : 3;
		u64 c1_wi_d : 1;
		u64 c1_b0_d : 1;
		u64 c0_wi_d : 1;
		u64 c0_b0_d : 1;
	} s;
	struct cvmx_npei_ctl_status2_s cn52xx;
	struct cvmx_npei_ctl_status2_s cn52xxp1;
	struct cvmx_npei_ctl_status2_s cn56xx;
	struct cvmx_npei_ctl_status2_s cn56xxp1;
};

typedef union cvmx_npei_ctl_status2 cvmx_npei_ctl_status2_t;

/**
 * cvmx_npei_data_out_cnt
 *
 * NPEI_DATA_OUT_CNT = NPEI DATA OUT COUNT
 *
 * The EXEC data out fifo-count and the data unload counter.
 */
union cvmx_npei_data_out_cnt {
	u64 u64;
	struct cvmx_npei_data_out_cnt_s {
		u64 reserved_44_63 : 20;
		u64 p1_ucnt : 16;
		u64 p1_fcnt : 6;
		u64 p0_ucnt : 16;
		u64 p0_fcnt : 6;
	} s;
	struct cvmx_npei_data_out_cnt_s cn52xx;
	struct cvmx_npei_data_out_cnt_s cn52xxp1;
	struct cvmx_npei_data_out_cnt_s cn56xx;
	struct cvmx_npei_data_out_cnt_s cn56xxp1;
};

typedef union cvmx_npei_data_out_cnt cvmx_npei_data_out_cnt_t;

/**
 * cvmx_npei_dbg_data
 *
 * NPEI_DBG_DATA = NPEI Debug Data Register
 *
 * Value returned on the debug-data lines from the RSLs
 */
union cvmx_npei_dbg_data {
	u64 u64;
	struct cvmx_npei_dbg_data_s {
		u64 reserved_28_63 : 36;
		u64 qlm0_rev_lanes : 1;
		u64 reserved_25_26 : 2;
		u64 qlm1_spd : 2;
		u64 c_mul : 5;
		u64 dsel_ext : 1;
		u64 data : 17;
	} s;
	struct cvmx_npei_dbg_data_cn52xx {
		u64 reserved_29_63 : 35;
		u64 qlm0_link_width : 1;
		u64 qlm0_rev_lanes : 1;
		u64 qlm1_mode : 2;
		u64 qlm1_spd : 2;
		u64 c_mul : 5;
		u64 dsel_ext : 1;
		u64 data : 17;
	} cn52xx;
	struct cvmx_npei_dbg_data_cn52xx cn52xxp1;
	struct cvmx_npei_dbg_data_cn56xx {
		u64 reserved_29_63 : 35;
		u64 qlm2_rev_lanes : 1;
		u64 qlm0_rev_lanes : 1;
		u64 qlm3_spd : 2;
		u64 qlm1_spd : 2;
		u64 c_mul : 5;
		u64 dsel_ext : 1;
		u64 data : 17;
	} cn56xx;
	struct cvmx_npei_dbg_data_cn56xx cn56xxp1;
};

typedef union cvmx_npei_dbg_data cvmx_npei_dbg_data_t;

/**
 * cvmx_npei_dbg_select
 *
 * NPEI_DBG_SELECT = Debug Select Register
 *
 * Contains the debug select value last written to the RSLs.
 */
union cvmx_npei_dbg_select {
	u64 u64;
	struct cvmx_npei_dbg_select_s {
		u64 reserved_16_63 : 48;
		u64 dbg_sel : 16;
	} s;
	struct cvmx_npei_dbg_select_s cn52xx;
	struct cvmx_npei_dbg_select_s cn52xxp1;
	struct cvmx_npei_dbg_select_s cn56xx;
	struct cvmx_npei_dbg_select_s cn56xxp1;
};

typedef union cvmx_npei_dbg_select cvmx_npei_dbg_select_t;

/**
 * cvmx_npei_dma#_counts
 *
 * NPEI_DMA[0..4]_COUNTS = DMA Instruction Counts
 *
 * Values for determing the number of instructions for DMA[0..4] in the NPEI.
 */
union cvmx_npei_dmax_counts {
	u64 u64;
	struct cvmx_npei_dmax_counts_s {
		u64 reserved_39_63 : 25;
		u64 fcnt : 7;
		u64 dbell : 32;
	} s;
	struct cvmx_npei_dmax_counts_s cn52xx;
	struct cvmx_npei_dmax_counts_s cn52xxp1;
	struct cvmx_npei_dmax_counts_s cn56xx;
	struct cvmx_npei_dmax_counts_s cn56xxp1;
};

typedef union cvmx_npei_dmax_counts cvmx_npei_dmax_counts_t;

/**
 * cvmx_npei_dma#_dbell
 *
 * NPEI_DMA_DBELL[0..4] = DMA Door Bell
 *
 * The door bell register for DMA[0..4] queue.
 */
union cvmx_npei_dmax_dbell {
	u32 u32;
	struct cvmx_npei_dmax_dbell_s {
		u32 reserved_16_31 : 16;
		u32 dbell : 16;
	} s;
	struct cvmx_npei_dmax_dbell_s cn52xx;
	struct cvmx_npei_dmax_dbell_s cn52xxp1;
	struct cvmx_npei_dmax_dbell_s cn56xx;
	struct cvmx_npei_dmax_dbell_s cn56xxp1;
};

typedef union cvmx_npei_dmax_dbell cvmx_npei_dmax_dbell_t;

/**
 * cvmx_npei_dma#_ibuff_saddr
 *
 * NPEI_DMA[0..4]_IBUFF_SADDR = DMA Instruction Buffer Starting Address
 *
 * The address to start reading Instructions from for DMA[0..4].
 */
union cvmx_npei_dmax_ibuff_saddr {
	u64 u64;
	struct cvmx_npei_dmax_ibuff_saddr_s {
		u64 reserved_37_63 : 27;
		u64 idle : 1;
		u64 saddr : 29;
		u64 reserved_0_6 : 7;
	} s;
	struct cvmx_npei_dmax_ibuff_saddr_s cn52xx;
	struct cvmx_npei_dmax_ibuff_saddr_cn52xxp1 {
		u64 reserved_36_63 : 28;
		u64 saddr : 29;
		u64 reserved_0_6 : 7;
	} cn52xxp1;
	struct cvmx_npei_dmax_ibuff_saddr_s cn56xx;
	struct cvmx_npei_dmax_ibuff_saddr_cn52xxp1 cn56xxp1;
};

typedef union cvmx_npei_dmax_ibuff_saddr cvmx_npei_dmax_ibuff_saddr_t;

/**
 * cvmx_npei_dma#_naddr
 *
 * NPEI_DMA[0..4]_NADDR = DMA Next Ichunk Address
 *
 * Place NPEI will read the next Ichunk data from. This is valid when state is 0
 */
union cvmx_npei_dmax_naddr {
	u64 u64;
	struct cvmx_npei_dmax_naddr_s {
		u64 reserved_36_63 : 28;
		u64 addr : 36;
	} s;
	struct cvmx_npei_dmax_naddr_s cn52xx;
	struct cvmx_npei_dmax_naddr_s cn52xxp1;
	struct cvmx_npei_dmax_naddr_s cn56xx;
	struct cvmx_npei_dmax_naddr_s cn56xxp1;
};

typedef union cvmx_npei_dmax_naddr cvmx_npei_dmax_naddr_t;

/**
 * cvmx_npei_dma0_int_level
 *
 * NPEI_DMA0_INT_LEVEL = NPEI DMA0 Interrupt Level
 *
 * Thresholds for DMA count and timer interrupts for DMA0.
 */
union cvmx_npei_dma0_int_level {
	u64 u64;
	struct cvmx_npei_dma0_int_level_s {
		u64 time : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_dma0_int_level_s cn52xx;
	struct cvmx_npei_dma0_int_level_s cn52xxp1;
	struct cvmx_npei_dma0_int_level_s cn56xx;
	struct cvmx_npei_dma0_int_level_s cn56xxp1;
};

typedef union cvmx_npei_dma0_int_level cvmx_npei_dma0_int_level_t;

/**
 * cvmx_npei_dma1_int_level
 *
 * NPEI_DMA1_INT_LEVEL = NPEI DMA1 Interrupt Level
 *
 * Thresholds for DMA count and timer interrupts for DMA1.
 */
union cvmx_npei_dma1_int_level {
	u64 u64;
	struct cvmx_npei_dma1_int_level_s {
		u64 time : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_dma1_int_level_s cn52xx;
	struct cvmx_npei_dma1_int_level_s cn52xxp1;
	struct cvmx_npei_dma1_int_level_s cn56xx;
	struct cvmx_npei_dma1_int_level_s cn56xxp1;
};

typedef union cvmx_npei_dma1_int_level cvmx_npei_dma1_int_level_t;

/**
 * cvmx_npei_dma_cnts
 *
 * NPEI_DMA_CNTS = NPEI DMA Count
 *
 * The DMA Count values for DMA0 and DMA1.
 */
union cvmx_npei_dma_cnts {
	u64 u64;
	struct cvmx_npei_dma_cnts_s {
		u64 dma1 : 32;
		u64 dma0 : 32;
	} s;
	struct cvmx_npei_dma_cnts_s cn52xx;
	struct cvmx_npei_dma_cnts_s cn52xxp1;
	struct cvmx_npei_dma_cnts_s cn56xx;
	struct cvmx_npei_dma_cnts_s cn56xxp1;
};

typedef union cvmx_npei_dma_cnts cvmx_npei_dma_cnts_t;

/**
 * cvmx_npei_dma_control
 *
 * NPEI_DMA_CONTROL = DMA Control Register
 *
 * Controls operation of the DMA IN/OUT.
 */
union cvmx_npei_dma_control {
	u64 u64;
	struct cvmx_npei_dma_control_s {
		u64 reserved_40_63 : 24;
		u64 p_32b_m : 1;
		u64 dma4_enb : 1;
		u64 dma3_enb : 1;
		u64 dma2_enb : 1;
		u64 dma1_enb : 1;
		u64 dma0_enb : 1;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 csize : 14;
	} s;
	struct cvmx_npei_dma_control_s cn52xx;
	struct cvmx_npei_dma_control_cn52xxp1 {
		u64 reserved_38_63 : 26;
		u64 dma3_enb : 1;
		u64 dma2_enb : 1;
		u64 dma1_enb : 1;
		u64 dma0_enb : 1;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 csize : 14;
	} cn52xxp1;
	struct cvmx_npei_dma_control_s cn56xx;
	struct cvmx_npei_dma_control_cn56xxp1 {
		u64 reserved_39_63 : 25;
		u64 dma4_enb : 1;
		u64 dma3_enb : 1;
		u64 dma2_enb : 1;
		u64 dma1_enb : 1;
		u64 dma0_enb : 1;
		u64 b0_lend : 1;
		u64 dwb_denb : 1;
		u64 dwb_ichk : 9;
		u64 fpa_que : 3;
		u64 o_add1 : 1;
		u64 o_ro : 1;
		u64 o_ns : 1;
		u64 o_es : 2;
		u64 o_mode : 1;
		u64 csize : 14;
	} cn56xxp1;
};

typedef union cvmx_npei_dma_control cvmx_npei_dma_control_t;

/**
 * cvmx_npei_dma_pcie_req_num
 *
 * NPEI_DMA_PCIE_REQ_NUM = NPEI DMA PCIE Outstanding Read Request Number
 *
 * Outstanding PCIE read request number for DMAs and Packet, maximum number
 * is 16
 */
union cvmx_npei_dma_pcie_req_num {
	u64 u64;
	struct cvmx_npei_dma_pcie_req_num_s {
		u64 dma_arb : 1;
		u64 reserved_53_62 : 10;
		u64 pkt_cnt : 5;
		u64 reserved_45_47 : 3;
		u64 dma4_cnt : 5;
		u64 reserved_37_39 : 3;
		u64 dma3_cnt : 5;
		u64 reserved_29_31 : 3;
		u64 dma2_cnt : 5;
		u64 reserved_21_23 : 3;
		u64 dma1_cnt : 5;
		u64 reserved_13_15 : 3;
		u64 dma0_cnt : 5;
		u64 reserved_5_7 : 3;
		u64 dma_cnt : 5;
	} s;
	struct cvmx_npei_dma_pcie_req_num_s cn52xx;
	struct cvmx_npei_dma_pcie_req_num_s cn56xx;
};

typedef union cvmx_npei_dma_pcie_req_num cvmx_npei_dma_pcie_req_num_t;

/**
 * cvmx_npei_dma_state1
 *
 * NPEI_DMA_STATE1 = NPI's DMA State 1
 *
 * Results from DMA state register 1
 */
union cvmx_npei_dma_state1 {
	u64 u64;
	struct cvmx_npei_dma_state1_s {
		u64 reserved_40_63 : 24;
		u64 d4_dwe : 8;
		u64 d3_dwe : 8;
		u64 d2_dwe : 8;
		u64 d1_dwe : 8;
		u64 d0_dwe : 8;
	} s;
	struct cvmx_npei_dma_state1_s cn52xx;
};

typedef union cvmx_npei_dma_state1 cvmx_npei_dma_state1_t;

/**
 * cvmx_npei_dma_state1_p1
 *
 * NPEI_DMA_STATE1_P1 = NPEI DMA Request and Instruction State
 *
 * DMA engine Debug information.
 */
union cvmx_npei_dma_state1_p1 {
	u64 u64;
	struct cvmx_npei_dma_state1_p1_s {
		u64 reserved_60_63 : 4;
		u64 d0_difst : 7;
		u64 d1_difst : 7;
		u64 d2_difst : 7;
		u64 d3_difst : 7;
		u64 d4_difst : 7;
		u64 d0_reqst : 5;
		u64 d1_reqst : 5;
		u64 d2_reqst : 5;
		u64 d3_reqst : 5;
		u64 d4_reqst : 5;
	} s;
	struct cvmx_npei_dma_state1_p1_cn52xxp1 {
		u64 reserved_60_63 : 4;
		u64 d0_difst : 7;
		u64 d1_difst : 7;
		u64 d2_difst : 7;
		u64 d3_difst : 7;
		u64 reserved_25_31 : 7;
		u64 d0_reqst : 5;
		u64 d1_reqst : 5;
		u64 d2_reqst : 5;
		u64 d3_reqst : 5;
		u64 reserved_0_4 : 5;
	} cn52xxp1;
	struct cvmx_npei_dma_state1_p1_s cn56xxp1;
};

typedef union cvmx_npei_dma_state1_p1 cvmx_npei_dma_state1_p1_t;

/**
 * cvmx_npei_dma_state2
 *
 * NPEI_DMA_STATE2 = NPI's DMA State 2
 *
 * Results from DMA state register 2
 */
union cvmx_npei_dma_state2 {
	u64 u64;
	struct cvmx_npei_dma_state2_s {
		u64 reserved_28_63 : 36;
		u64 ndwe : 4;
		u64 reserved_21_23 : 3;
		u64 ndre : 5;
		u64 reserved_10_15 : 6;
		u64 prd : 10;
	} s;
	struct cvmx_npei_dma_state2_s cn52xx;
};

typedef union cvmx_npei_dma_state2 cvmx_npei_dma_state2_t;

/**
 * cvmx_npei_dma_state2_p1
 *
 * NPEI_DMA_STATE2_P1 = NPEI DMA Instruction Fetch State
 *
 * DMA engine Debug information.
 */
union cvmx_npei_dma_state2_p1 {
	u64 u64;
	struct cvmx_npei_dma_state2_p1_s {
		u64 reserved_45_63 : 19;
		u64 d0_dffst : 9;
		u64 d1_dffst : 9;
		u64 d2_dffst : 9;
		u64 d3_dffst : 9;
		u64 d4_dffst : 9;
	} s;
	struct cvmx_npei_dma_state2_p1_cn52xxp1 {
		u64 reserved_45_63 : 19;
		u64 d0_dffst : 9;
		u64 d1_dffst : 9;
		u64 d2_dffst : 9;
		u64 d3_dffst : 9;
		u64 reserved_0_8 : 9;
	} cn52xxp1;
	struct cvmx_npei_dma_state2_p1_s cn56xxp1;
};

typedef union cvmx_npei_dma_state2_p1 cvmx_npei_dma_state2_p1_t;

/**
 * cvmx_npei_dma_state3_p1
 *
 * NPEI_DMA_STATE3_P1 = NPEI DMA DRE State
 *
 * DMA engine Debug information.
 */
union cvmx_npei_dma_state3_p1 {
	u64 u64;
	struct cvmx_npei_dma_state3_p1_s {
		u64 reserved_60_63 : 4;
		u64 d0_drest : 15;
		u64 d1_drest : 15;
		u64 d2_drest : 15;
		u64 d3_drest : 15;
	} s;
	struct cvmx_npei_dma_state3_p1_s cn52xxp1;
	struct cvmx_npei_dma_state3_p1_s cn56xxp1;
};

typedef union cvmx_npei_dma_state3_p1 cvmx_npei_dma_state3_p1_t;

/**
 * cvmx_npei_dma_state4_p1
 *
 * NPEI_DMA_STATE4_P1 = NPEI DMA DWE State
 *
 * DMA engine Debug information.
 */
union cvmx_npei_dma_state4_p1 {
	u64 u64;
	struct cvmx_npei_dma_state4_p1_s {
		u64 reserved_52_63 : 12;
		u64 d0_dwest : 13;
		u64 d1_dwest : 13;
		u64 d2_dwest : 13;
		u64 d3_dwest : 13;
	} s;
	struct cvmx_npei_dma_state4_p1_s cn52xxp1;
	struct cvmx_npei_dma_state4_p1_s cn56xxp1;
};

typedef union cvmx_npei_dma_state4_p1 cvmx_npei_dma_state4_p1_t;

/**
 * cvmx_npei_dma_state5_p1
 *
 * NPEI_DMA_STATE5_P1 = NPEI DMA DWE and DRE State
 *
 * DMA engine Debug information.
 */
union cvmx_npei_dma_state5_p1 {
	u64 u64;
	struct cvmx_npei_dma_state5_p1_s {
		u64 reserved_28_63 : 36;
		u64 d4_drest : 15;
		u64 d4_dwest : 13;
	} s;
	struct cvmx_npei_dma_state5_p1_s cn56xxp1;
};

typedef union cvmx_npei_dma_state5_p1 cvmx_npei_dma_state5_p1_t;

/**
 * cvmx_npei_int_a_enb
 *
 * NPEI_INTERRUPT_A_ENB = NPI's Interrupt A Enable Register
 *
 * Used to allow the generation of interrupts (MSI/INTA) to the PCIe
 * CoresUsed to enable the various interrupting conditions of NPEI
 */
union cvmx_npei_int_a_enb {
	u64 u64;
	struct cvmx_npei_int_a_enb_s {
		u64 reserved_10_63 : 54;
		u64 pout_err : 1;
		u64 pin_bp : 1;
		u64 p1_rdlk : 1;
		u64 p0_rdlk : 1;
		u64 pgl_err : 1;
		u64 pdi_err : 1;
		u64 pop_err : 1;
		u64 pins_err : 1;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} s;
	struct cvmx_npei_int_a_enb_s cn52xx;
	struct cvmx_npei_int_a_enb_cn52xxp1 {
		u64 reserved_2_63 : 62;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} cn52xxp1;
	struct cvmx_npei_int_a_enb_s cn56xx;
};

typedef union cvmx_npei_int_a_enb cvmx_npei_int_a_enb_t;

/**
 * cvmx_npei_int_a_enb2
 *
 * NPEI_INTERRUPT_A_ENB2 = NPEI's Interrupt A Enable2 Register
 *
 * Used to enable the various interrupting conditions of NPEI
 */
union cvmx_npei_int_a_enb2 {
	u64 u64;
	struct cvmx_npei_int_a_enb2_s {
		u64 reserved_10_63 : 54;
		u64 pout_err : 1;
		u64 pin_bp : 1;
		u64 p1_rdlk : 1;
		u64 p0_rdlk : 1;
		u64 pgl_err : 1;
		u64 pdi_err : 1;
		u64 pop_err : 1;
		u64 pins_err : 1;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} s;
	struct cvmx_npei_int_a_enb2_s cn52xx;
	struct cvmx_npei_int_a_enb2_cn52xxp1 {
		u64 reserved_2_63 : 62;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} cn52xxp1;
	struct cvmx_npei_int_a_enb2_s cn56xx;
};

typedef union cvmx_npei_int_a_enb2 cvmx_npei_int_a_enb2_t;

/**
 * cvmx_npei_int_a_sum
 *
 * NPEI_INTERRUPT_A_SUM = NPI Interrupt A Summary Register
 *
 * Set when an interrupt condition occurs, write '1' to clear. When an
 * interrupt bitin this register is set and
 * the cooresponding bit in the NPEI_INT_A_ENB register is set, then
 * NPEI_INT_SUM[61] will be set.
 */
union cvmx_npei_int_a_sum {
	u64 u64;
	struct cvmx_npei_int_a_sum_s {
		u64 reserved_10_63 : 54;
		u64 pout_err : 1;
		u64 pin_bp : 1;
		u64 p1_rdlk : 1;
		u64 p0_rdlk : 1;
		u64 pgl_err : 1;
		u64 pdi_err : 1;
		u64 pop_err : 1;
		u64 pins_err : 1;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} s;
	struct cvmx_npei_int_a_sum_s cn52xx;
	struct cvmx_npei_int_a_sum_cn52xxp1 {
		u64 reserved_2_63 : 62;
		u64 dma1_cpl : 1;
		u64 dma0_cpl : 1;
	} cn52xxp1;
	struct cvmx_npei_int_a_sum_s cn56xx;
};

typedef union cvmx_npei_int_a_sum cvmx_npei_int_a_sum_t;

/**
 * cvmx_npei_int_enb
 *
 * NPEI_INTERRUPT_ENB = NPI's Interrupt Enable Register
 *
 * Used to allow the generation of interrupts (MSI/INTA) to the PCIe
 * CoresUsed to enable the various interrupting conditions of NPI
 */
union cvmx_npei_int_enb {
	u64 u64;
	struct cvmx_npei_int_enb_s {
		u64 mio_inta : 1;
		u64 reserved_62_62 : 1;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npei_int_enb_s cn52xx;
	struct cvmx_npei_int_enb_cn52xxp1 {
		u64 mio_inta : 1;
		u64 reserved_62_62 : 1;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 reserved_8_8 : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn52xxp1;
	struct cvmx_npei_int_enb_s cn56xx;
	struct cvmx_npei_int_enb_cn56xxp1 {
		u64 mio_inta : 1;
		u64 reserved_61_62 : 2;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 reserved_29_29 : 1;
		u64 c1_se : 1;
		u64 reserved_27_27 : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 reserved_22_22 : 1;
		u64 c0_se : 1;
		u64 reserved_20_20 : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn56xxp1;
};

typedef union cvmx_npei_int_enb cvmx_npei_int_enb_t;

/**
 * cvmx_npei_int_enb2
 *
 * NPEI_INTERRUPT_ENB2 = NPI's Interrupt Enable2 Register
 *
 * Used to enable the various interrupting conditions of NPI
 */
union cvmx_npei_int_enb2 {
	u64 u64;
	struct cvmx_npei_int_enb2_s {
		u64 reserved_62_63 : 2;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npei_int_enb2_s cn52xx;
	struct cvmx_npei_int_enb2_cn52xxp1 {
		u64 reserved_62_63 : 2;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 reserved_8_8 : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn52xxp1;
	struct cvmx_npei_int_enb2_s cn56xx;
	struct cvmx_npei_int_enb2_cn56xxp1 {
		u64 reserved_61_63 : 3;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 reserved_29_29 : 1;
		u64 c1_se : 1;
		u64 reserved_27_27 : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 reserved_22_22 : 1;
		u64 c0_se : 1;
		u64 reserved_20_20 : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn56xxp1;
};

typedef union cvmx_npei_int_enb2 cvmx_npei_int_enb2_t;

/**
 * cvmx_npei_int_info
 *
 * NPEI_INT_INFO = NPI Interrupt Information
 *
 * Contains information about some of the interrupt condition that can occur
 * in the NPEI_INTERRUPT_SUM register.
 */
union cvmx_npei_int_info {
	u64 u64;
	struct cvmx_npei_int_info_s {
		u64 reserved_12_63 : 52;
		u64 pidbof : 6;
		u64 psldbof : 6;
	} s;
	struct cvmx_npei_int_info_s cn52xx;
	struct cvmx_npei_int_info_s cn56xx;
	struct cvmx_npei_int_info_s cn56xxp1;
};

typedef union cvmx_npei_int_info cvmx_npei_int_info_t;

/**
 * cvmx_npei_int_sum
 *
 * NPEI_INTERRUPT_SUM = NPI Interrupt Summary Register
 *
 * Set when an interrupt condition occurs, write '1' to clear.
 */
union cvmx_npei_int_sum {
	u64 u64;
	struct cvmx_npei_int_sum_s {
		u64 mio_inta : 1;
		u64 reserved_62_62 : 1;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 ptime : 1;
		u64 pcnt : 1;
		u64 pidbof : 1;
		u64 psldbof : 1;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npei_int_sum_s cn52xx;
	struct cvmx_npei_int_sum_cn52xxp1 {
		u64 mio_inta : 1;
		u64 reserved_62_62 : 1;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 reserved_15_18 : 4;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 reserved_8_8 : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn52xxp1;
	struct cvmx_npei_int_sum_s cn56xx;
	struct cvmx_npei_int_sum_cn56xxp1 {
		u64 mio_inta : 1;
		u64 reserved_61_62 : 2;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 reserved_29_29 : 1;
		u64 c1_se : 1;
		u64 reserved_27_27 : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 reserved_22_22 : 1;
		u64 c0_se : 1;
		u64 reserved_20_20 : 1;
		u64 c0_aeri : 1;
		u64 reserved_15_18 : 4;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 dma4dbo : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} cn56xxp1;
};

typedef union cvmx_npei_int_sum cvmx_npei_int_sum_t;

/**
 * cvmx_npei_int_sum2
 *
 * NPEI_INTERRUPT_SUM2 = NPI Interrupt Summary2 Register
 *
 * This is a read only copy of the NPEI_INTERRUPT_SUM register with bit
 * variances.
 */
union cvmx_npei_int_sum2 {
	u64 u64;
	struct cvmx_npei_int_sum2_s {
		u64 mio_inta : 1;
		u64 reserved_62_62 : 1;
		u64 int_a : 1;
		u64 c1_ldwn : 1;
		u64 c0_ldwn : 1;
		u64 c1_exc : 1;
		u64 c0_exc : 1;
		u64 c1_up_wf : 1;
		u64 c0_up_wf : 1;
		u64 c1_un_wf : 1;
		u64 c0_un_wf : 1;
		u64 c1_un_bx : 1;
		u64 c1_un_wi : 1;
		u64 c1_un_b2 : 1;
		u64 c1_un_b1 : 1;
		u64 c1_un_b0 : 1;
		u64 c1_up_bx : 1;
		u64 c1_up_wi : 1;
		u64 c1_up_b2 : 1;
		u64 c1_up_b1 : 1;
		u64 c1_up_b0 : 1;
		u64 c0_un_bx : 1;
		u64 c0_un_wi : 1;
		u64 c0_un_b2 : 1;
		u64 c0_un_b1 : 1;
		u64 c0_un_b0 : 1;
		u64 c0_up_bx : 1;
		u64 c0_up_wi : 1;
		u64 c0_up_b2 : 1;
		u64 c0_up_b1 : 1;
		u64 c0_up_b0 : 1;
		u64 c1_hpint : 1;
		u64 c1_pmei : 1;
		u64 c1_wake : 1;
		u64 crs1_dr : 1;
		u64 c1_se : 1;
		u64 crs1_er : 1;
		u64 c1_aeri : 1;
		u64 c0_hpint : 1;
		u64 c0_pmei : 1;
		u64 c0_wake : 1;
		u64 crs0_dr : 1;
		u64 c0_se : 1;
		u64 crs0_er : 1;
		u64 c0_aeri : 1;
		u64 reserved_15_18 : 4;
		u64 dtime1 : 1;
		u64 dtime0 : 1;
		u64 dcnt1 : 1;
		u64 dcnt0 : 1;
		u64 dma1fi : 1;
		u64 dma0fi : 1;
		u64 reserved_8_8 : 1;
		u64 dma3dbo : 1;
		u64 dma2dbo : 1;
		u64 dma1dbo : 1;
		u64 dma0dbo : 1;
		u64 iob2big : 1;
		u64 bar0_to : 1;
		u64 rml_wto : 1;
		u64 rml_rto : 1;
	} s;
	struct cvmx_npei_int_sum2_s cn52xx;
	struct cvmx_npei_int_sum2_s cn52xxp1;
	struct cvmx_npei_int_sum2_s cn56xx;
};

typedef union cvmx_npei_int_sum2 cvmx_npei_int_sum2_t;

/**
 * cvmx_npei_last_win_rdata0
 *
 * NPEI_LAST_WIN_RDATA0 = NPEI Last Window Read Data Port0
 *
 * The data from the last initiated window read.
 */
union cvmx_npei_last_win_rdata0 {
	u64 u64;
	struct cvmx_npei_last_win_rdata0_s {
		u64 data : 64;
	} s;
	struct cvmx_npei_last_win_rdata0_s cn52xx;
	struct cvmx_npei_last_win_rdata0_s cn52xxp1;
	struct cvmx_npei_last_win_rdata0_s cn56xx;
	struct cvmx_npei_last_win_rdata0_s cn56xxp1;
};

typedef union cvmx_npei_last_win_rdata0 cvmx_npei_last_win_rdata0_t;

/**
 * cvmx_npei_last_win_rdata1
 *
 * NPEI_LAST_WIN_RDATA1 = NPEI Last Window Read Data Port1
 *
 * The data from the last initiated window read.
 */
union cvmx_npei_last_win_rdata1 {
	u64 u64;
	struct cvmx_npei_last_win_rdata1_s {
		u64 data : 64;
	} s;
	struct cvmx_npei_last_win_rdata1_s cn52xx;
	struct cvmx_npei_last_win_rdata1_s cn52xxp1;
	struct cvmx_npei_last_win_rdata1_s cn56xx;
	struct cvmx_npei_last_win_rdata1_s cn56xxp1;
};

typedef union cvmx_npei_last_win_rdata1 cvmx_npei_last_win_rdata1_t;

/**
 * cvmx_npei_mem_access_ctl
 *
 * NPEI_MEM_ACCESS_CTL = NPEI's Memory Access Control
 *
 * Contains control for access to the PCIe address space.
 */
union cvmx_npei_mem_access_ctl {
	u64 u64;
	struct cvmx_npei_mem_access_ctl_s {
		u64 reserved_14_63 : 50;
		u64 max_word : 4;
		u64 timer : 10;
	} s;
	struct cvmx_npei_mem_access_ctl_s cn52xx;
	struct cvmx_npei_mem_access_ctl_s cn52xxp1;
	struct cvmx_npei_mem_access_ctl_s cn56xx;
	struct cvmx_npei_mem_access_ctl_s cn56xxp1;
};

typedef union cvmx_npei_mem_access_ctl cvmx_npei_mem_access_ctl_t;

/**
 * cvmx_npei_mem_access_subid#
 *
 * NPEI_MEM_ACCESS_SUBIDX = NPEI Memory Access SubidX Register
 *
 * Contains address index and control bits for access to memory from Core PPs.
 */
union cvmx_npei_mem_access_subidx {
	u64 u64;
	struct cvmx_npei_mem_access_subidx_s {
		u64 reserved_42_63 : 22;
		u64 zero : 1;
		u64 port : 2;
		u64 nmerge : 1;
		u64 esr : 2;
		u64 esw : 2;
		u64 nsr : 1;
		u64 nsw : 1;
		u64 ror : 1;
		u64 row : 1;
		u64 ba : 30;
	} s;
	struct cvmx_npei_mem_access_subidx_s cn52xx;
	struct cvmx_npei_mem_access_subidx_s cn52xxp1;
	struct cvmx_npei_mem_access_subidx_s cn56xx;
	struct cvmx_npei_mem_access_subidx_s cn56xxp1;
};

typedef union cvmx_npei_mem_access_subidx cvmx_npei_mem_access_subidx_t;

/**
 * cvmx_npei_msi_enb0
 *
 * NPEI_MSI_ENB0 = NPEI MSI Enable0
 *
 * Used to enable the interrupt generation for the bits in the NPEI_MSI_RCV0.
 */
union cvmx_npei_msi_enb0 {
	u64 u64;
	struct cvmx_npei_msi_enb0_s {
		u64 enb : 64;
	} s;
	struct cvmx_npei_msi_enb0_s cn52xx;
	struct cvmx_npei_msi_enb0_s cn52xxp1;
	struct cvmx_npei_msi_enb0_s cn56xx;
	struct cvmx_npei_msi_enb0_s cn56xxp1;
};

typedef union cvmx_npei_msi_enb0 cvmx_npei_msi_enb0_t;

/**
 * cvmx_npei_msi_enb1
 *
 * NPEI_MSI_ENB1 = NPEI MSI Enable1
 *
 * Used to enable the interrupt generation for the bits in the NPEI_MSI_RCV1.
 */
union cvmx_npei_msi_enb1 {
	u64 u64;
	struct cvmx_npei_msi_enb1_s {
		u64 enb : 64;
	} s;
	struct cvmx_npei_msi_enb1_s cn52xx;
	struct cvmx_npei_msi_enb1_s cn52xxp1;
	struct cvmx_npei_msi_enb1_s cn56xx;
	struct cvmx_npei_msi_enb1_s cn56xxp1;
};

typedef union cvmx_npei_msi_enb1 cvmx_npei_msi_enb1_t;

/**
 * cvmx_npei_msi_enb2
 *
 * NPEI_MSI_ENB2 = NPEI MSI Enable2
 *
 * Used to enable the interrupt generation for the bits in the NPEI_MSI_RCV2.
 */
union cvmx_npei_msi_enb2 {
	u64 u64;
	struct cvmx_npei_msi_enb2_s {
		u64 enb : 64;
	} s;
	struct cvmx_npei_msi_enb2_s cn52xx;
	struct cvmx_npei_msi_enb2_s cn52xxp1;
	struct cvmx_npei_msi_enb2_s cn56xx;
	struct cvmx_npei_msi_enb2_s cn56xxp1;
};

typedef union cvmx_npei_msi_enb2 cvmx_npei_msi_enb2_t;

/**
 * cvmx_npei_msi_enb3
 *
 * NPEI_MSI_ENB3 = NPEI MSI Enable3
 *
 * Used to enable the interrupt generation for the bits in the NPEI_MSI_RCV3.
 */
union cvmx_npei_msi_enb3 {
	u64 u64;
	struct cvmx_npei_msi_enb3_s {
		u64 enb : 64;
	} s;
	struct cvmx_npei_msi_enb3_s cn52xx;
	struct cvmx_npei_msi_enb3_s cn52xxp1;
	struct cvmx_npei_msi_enb3_s cn56xx;
	struct cvmx_npei_msi_enb3_s cn56xxp1;
};

typedef union cvmx_npei_msi_enb3 cvmx_npei_msi_enb3_t;

/**
 * cvmx_npei_msi_rcv0
 *
 * NPEI_MSI_RCV0 = NPEI MSI Receive0
 *
 * Contains bits [63:0] of the 256 bits oof MSI interrupts.
 */
union cvmx_npei_msi_rcv0 {
	u64 u64;
	struct cvmx_npei_msi_rcv0_s {
		u64 intr : 64;
	} s;
	struct cvmx_npei_msi_rcv0_s cn52xx;
	struct cvmx_npei_msi_rcv0_s cn52xxp1;
	struct cvmx_npei_msi_rcv0_s cn56xx;
	struct cvmx_npei_msi_rcv0_s cn56xxp1;
};

typedef union cvmx_npei_msi_rcv0 cvmx_npei_msi_rcv0_t;

/**
 * cvmx_npei_msi_rcv1
 *
 * NPEI_MSI_RCV1 = NPEI MSI Receive1
 *
 * Contains bits [127:64] of the 256 bits oof MSI interrupts.
 */
union cvmx_npei_msi_rcv1 {
	u64 u64;
	struct cvmx_npei_msi_rcv1_s {
		u64 intr : 64;
	} s;
	struct cvmx_npei_msi_rcv1_s cn52xx;
	struct cvmx_npei_msi_rcv1_s cn52xxp1;
	struct cvmx_npei_msi_rcv1_s cn56xx;
	struct cvmx_npei_msi_rcv1_s cn56xxp1;
};

typedef union cvmx_npei_msi_rcv1 cvmx_npei_msi_rcv1_t;

/**
 * cvmx_npei_msi_rcv2
 *
 * NPEI_MSI_RCV2 = NPEI MSI Receive2
 *
 * Contains bits [191:128] of the 256 bits oof MSI interrupts.
 */
union cvmx_npei_msi_rcv2 {
	u64 u64;
	struct cvmx_npei_msi_rcv2_s {
		u64 intr : 64;
	} s;
	struct cvmx_npei_msi_rcv2_s cn52xx;
	struct cvmx_npei_msi_rcv2_s cn52xxp1;
	struct cvmx_npei_msi_rcv2_s cn56xx;
	struct cvmx_npei_msi_rcv2_s cn56xxp1;
};

typedef union cvmx_npei_msi_rcv2 cvmx_npei_msi_rcv2_t;

/**
 * cvmx_npei_msi_rcv3
 *
 * NPEI_MSI_RCV3 = NPEI MSI Receive3
 *
 * Contains bits [255:192] of the 256 bits oof MSI interrupts.
 */
union cvmx_npei_msi_rcv3 {
	u64 u64;
	struct cvmx_npei_msi_rcv3_s {
		u64 intr : 64;
	} s;
	struct cvmx_npei_msi_rcv3_s cn52xx;
	struct cvmx_npei_msi_rcv3_s cn52xxp1;
	struct cvmx_npei_msi_rcv3_s cn56xx;
	struct cvmx_npei_msi_rcv3_s cn56xxp1;
};

typedef union cvmx_npei_msi_rcv3 cvmx_npei_msi_rcv3_t;

/**
 * cvmx_npei_msi_rd_map
 *
 * NPEI_MSI_RD_MAP = NPEI MSI Read MAP
 *
 * Used to read the mapping function of the NPEI_PCIE_MSI_RCV to NPEI_MSI_RCV
 * registers.
 */
union cvmx_npei_msi_rd_map {
	u64 u64;
	struct cvmx_npei_msi_rd_map_s {
		u64 reserved_16_63 : 48;
		u64 rd_int : 8;
		u64 msi_int : 8;
	} s;
	struct cvmx_npei_msi_rd_map_s cn52xx;
	struct cvmx_npei_msi_rd_map_s cn52xxp1;
	struct cvmx_npei_msi_rd_map_s cn56xx;
	struct cvmx_npei_msi_rd_map_s cn56xxp1;
};

typedef union cvmx_npei_msi_rd_map cvmx_npei_msi_rd_map_t;

/**
 * cvmx_npei_msi_w1c_enb0
 *
 * NPEI_MSI_W1C_ENB0 = NPEI MSI Write 1 To Clear Enable0
 *
 * Used to clear bits in NPEI_MSI_ENB0. This is a PASS2 register.
 */
union cvmx_npei_msi_w1c_enb0 {
	u64 u64;
	struct cvmx_npei_msi_w1c_enb0_s {
		u64 clr : 64;
	} s;
	struct cvmx_npei_msi_w1c_enb0_s cn52xx;
	struct cvmx_npei_msi_w1c_enb0_s cn56xx;
};

typedef union cvmx_npei_msi_w1c_enb0 cvmx_npei_msi_w1c_enb0_t;

/**
 * cvmx_npei_msi_w1c_enb1
 *
 * NPEI_MSI_W1C_ENB1 = NPEI MSI Write 1 To Clear Enable1
 *
 * Used to clear bits in NPEI_MSI_ENB1. This is a PASS2 register.
 */
union cvmx_npei_msi_w1c_enb1 {
	u64 u64;
	struct cvmx_npei_msi_w1c_enb1_s {
		u64 clr : 64;
	} s;
	struct cvmx_npei_msi_w1c_enb1_s cn52xx;
	struct cvmx_npei_msi_w1c_enb1_s cn56xx;
};

typedef union cvmx_npei_msi_w1c_enb1 cvmx_npei_msi_w1c_enb1_t;

/**
 * cvmx_npei_msi_w1c_enb2
 *
 * NPEI_MSI_W1C_ENB2 = NPEI MSI Write 1 To Clear Enable2
 *
 * Used to clear bits in NPEI_MSI_ENB2. This is a PASS2 register.
 */
union cvmx_npei_msi_w1c_enb2 {
	u64 u64;
	struct cvmx_npei_msi_w1c_enb2_s {
		u64 clr : 64;
	} s;
	struct cvmx_npei_msi_w1c_enb2_s cn52xx;
	struct cvmx_npei_msi_w1c_enb2_s cn56xx;
};

typedef union cvmx_npei_msi_w1c_enb2 cvmx_npei_msi_w1c_enb2_t;

/**
 * cvmx_npei_msi_w1c_enb3
 *
 * NPEI_MSI_W1C_ENB3 = NPEI MSI Write 1 To Clear Enable3
 *
 * Used to clear bits in NPEI_MSI_ENB3. This is a PASS2 register.
 */
union cvmx_npei_msi_w1c_enb3 {
	u64 u64;
	struct cvmx_npei_msi_w1c_enb3_s {
		u64 clr : 64;
	} s;
	struct cvmx_npei_msi_w1c_enb3_s cn52xx;
	struct cvmx_npei_msi_w1c_enb3_s cn56xx;
};

typedef union cvmx_npei_msi_w1c_enb3 cvmx_npei_msi_w1c_enb3_t;

/**
 * cvmx_npei_msi_w1s_enb0
 *
 * NPEI_MSI_W1S_ENB0 = NPEI MSI Write 1 To Set Enable0
 *
 * Used to set bits in NPEI_MSI_ENB0. This is a PASS2 register.
 */
union cvmx_npei_msi_w1s_enb0 {
	u64 u64;
	struct cvmx_npei_msi_w1s_enb0_s {
		u64 set : 64;
	} s;
	struct cvmx_npei_msi_w1s_enb0_s cn52xx;
	struct cvmx_npei_msi_w1s_enb0_s cn56xx;
};

typedef union cvmx_npei_msi_w1s_enb0 cvmx_npei_msi_w1s_enb0_t;

/**
 * cvmx_npei_msi_w1s_enb1
 *
 * NPEI_MSI_W1S_ENB0 = NPEI MSI Write 1 To Set Enable1
 *
 * Used to set bits in NPEI_MSI_ENB1. This is a PASS2 register.
 */
union cvmx_npei_msi_w1s_enb1 {
	u64 u64;
	struct cvmx_npei_msi_w1s_enb1_s {
		u64 set : 64;
	} s;
	struct cvmx_npei_msi_w1s_enb1_s cn52xx;
	struct cvmx_npei_msi_w1s_enb1_s cn56xx;
};

typedef union cvmx_npei_msi_w1s_enb1 cvmx_npei_msi_w1s_enb1_t;

/**
 * cvmx_npei_msi_w1s_enb2
 *
 * NPEI_MSI_W1S_ENB2 = NPEI MSI Write 1 To Set Enable2
 *
 * Used to set bits in NPEI_MSI_ENB2. This is a PASS2 register.
 */
union cvmx_npei_msi_w1s_enb2 {
	u64 u64;
	struct cvmx_npei_msi_w1s_enb2_s {
		u64 set : 64;
	} s;
	struct cvmx_npei_msi_w1s_enb2_s cn52xx;
	struct cvmx_npei_msi_w1s_enb2_s cn56xx;
};

typedef union cvmx_npei_msi_w1s_enb2 cvmx_npei_msi_w1s_enb2_t;

/**
 * cvmx_npei_msi_w1s_enb3
 *
 * NPEI_MSI_W1S_ENB3 = NPEI MSI Write 1 To Set Enable3
 *
 * Used to set bits in NPEI_MSI_ENB3. This is a PASS2 register.
 */
union cvmx_npei_msi_w1s_enb3 {
	u64 u64;
	struct cvmx_npei_msi_w1s_enb3_s {
		u64 set : 64;
	} s;
	struct cvmx_npei_msi_w1s_enb3_s cn52xx;
	struct cvmx_npei_msi_w1s_enb3_s cn56xx;
};

typedef union cvmx_npei_msi_w1s_enb3 cvmx_npei_msi_w1s_enb3_t;

/**
 * cvmx_npei_msi_wr_map
 *
 * NPEI_MSI_WR_MAP = NPEI MSI Write MAP
 *
 * Used to write the mapping function of the NPEI_PCIE_MSI_RCV to NPEI_MSI_RCV
 * registers.
 */
union cvmx_npei_msi_wr_map {
	u64 u64;
	struct cvmx_npei_msi_wr_map_s {
		u64 reserved_16_63 : 48;
		u64 ciu_int : 8;
		u64 msi_int : 8;
	} s;
	struct cvmx_npei_msi_wr_map_s cn52xx;
	struct cvmx_npei_msi_wr_map_s cn52xxp1;
	struct cvmx_npei_msi_wr_map_s cn56xx;
	struct cvmx_npei_msi_wr_map_s cn56xxp1;
};

typedef union cvmx_npei_msi_wr_map cvmx_npei_msi_wr_map_t;

/**
 * cvmx_npei_pcie_credit_cnt
 *
 * NPEI_PCIE_CREDIT_CNT = NPEI PCIE Credit Count
 *
 * Contains the number of credits for the pcie port FIFOs used by the NPEI.
 * This value needs to be set BEFORE PCIe traffic
 * flow from NPEI to PCIE Ports starts. A write to this register will cause
 * the credit counts in the NPEI for the two
 * PCIE ports to be reset to the value in this register.
 */
union cvmx_npei_pcie_credit_cnt {
	u64 u64;
	struct cvmx_npei_pcie_credit_cnt_s {
		u64 reserved_48_63 : 16;
		u64 p1_ccnt : 8;
		u64 p1_ncnt : 8;
		u64 p1_pcnt : 8;
		u64 p0_ccnt : 8;
		u64 p0_ncnt : 8;
		u64 p0_pcnt : 8;
	} s;
	struct cvmx_npei_pcie_credit_cnt_s cn52xx;
	struct cvmx_npei_pcie_credit_cnt_s cn56xx;
};

typedef union cvmx_npei_pcie_credit_cnt cvmx_npei_pcie_credit_cnt_t;

/**
 * cvmx_npei_pcie_msi_rcv
 *
 * NPEI_PCIE_MSI_RCV = NPEI PCIe MSI Receive
 *
 * Register where MSI writes are directed from the PCIe.
 */
union cvmx_npei_pcie_msi_rcv {
	u64 u64;
	struct cvmx_npei_pcie_msi_rcv_s {
		u64 reserved_8_63 : 56;
		u64 intr : 8;
	} s;
	struct cvmx_npei_pcie_msi_rcv_s cn52xx;
	struct cvmx_npei_pcie_msi_rcv_s cn52xxp1;
	struct cvmx_npei_pcie_msi_rcv_s cn56xx;
	struct cvmx_npei_pcie_msi_rcv_s cn56xxp1;
};

typedef union cvmx_npei_pcie_msi_rcv cvmx_npei_pcie_msi_rcv_t;

/**
 * cvmx_npei_pcie_msi_rcv_b1
 *
 * NPEI_PCIE_MSI_RCV_B1 = NPEI PCIe MSI Receive Byte 1
 *
 * Register where MSI writes are directed from the PCIe.
 */
union cvmx_npei_pcie_msi_rcv_b1 {
	u64 u64;
	struct cvmx_npei_pcie_msi_rcv_b1_s {
		u64 reserved_16_63 : 48;
		u64 intr : 8;
		u64 reserved_0_7 : 8;
	} s;
	struct cvmx_npei_pcie_msi_rcv_b1_s cn52xx;
	struct cvmx_npei_pcie_msi_rcv_b1_s cn52xxp1;
	struct cvmx_npei_pcie_msi_rcv_b1_s cn56xx;
	struct cvmx_npei_pcie_msi_rcv_b1_s cn56xxp1;
};

typedef union cvmx_npei_pcie_msi_rcv_b1 cvmx_npei_pcie_msi_rcv_b1_t;

/**
 * cvmx_npei_pcie_msi_rcv_b2
 *
 * NPEI_PCIE_MSI_RCV_B2 = NPEI PCIe MSI Receive Byte 2
 *
 * Register where MSI writes are directed from the PCIe.
 */
union cvmx_npei_pcie_msi_rcv_b2 {
	u64 u64;
	struct cvmx_npei_pcie_msi_rcv_b2_s {
		u64 reserved_24_63 : 40;
		u64 intr : 8;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_npei_pcie_msi_rcv_b2_s cn52xx;
	struct cvmx_npei_pcie_msi_rcv_b2_s cn52xxp1;
	struct cvmx_npei_pcie_msi_rcv_b2_s cn56xx;
	struct cvmx_npei_pcie_msi_rcv_b2_s cn56xxp1;
};

typedef union cvmx_npei_pcie_msi_rcv_b2 cvmx_npei_pcie_msi_rcv_b2_t;

/**
 * cvmx_npei_pcie_msi_rcv_b3
 *
 * NPEI_PCIE_MSI_RCV_B3 = NPEI PCIe MSI Receive Byte 3
 *
 * Register where MSI writes are directed from the PCIe.
 */
union cvmx_npei_pcie_msi_rcv_b3 {
	u64 u64;
	struct cvmx_npei_pcie_msi_rcv_b3_s {
		u64 reserved_32_63 : 32;
		u64 intr : 8;
		u64 reserved_0_23 : 24;
	} s;
	struct cvmx_npei_pcie_msi_rcv_b3_s cn52xx;
	struct cvmx_npei_pcie_msi_rcv_b3_s cn52xxp1;
	struct cvmx_npei_pcie_msi_rcv_b3_s cn56xx;
	struct cvmx_npei_pcie_msi_rcv_b3_s cn56xxp1;
};

typedef union cvmx_npei_pcie_msi_rcv_b3 cvmx_npei_pcie_msi_rcv_b3_t;

/**
 * cvmx_npei_pkt#_cnts
 *
 * NPEI_PKT[0..31]_CNTS = NPEI Packet ring# Counts
 *
 * The counters for output rings.
 */
union cvmx_npei_pktx_cnts {
	u64 u64;
	struct cvmx_npei_pktx_cnts_s {
		u64 reserved_54_63 : 10;
		u64 timer : 22;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_pktx_cnts_s cn52xx;
	struct cvmx_npei_pktx_cnts_s cn56xx;
};

typedef union cvmx_npei_pktx_cnts cvmx_npei_pktx_cnts_t;

/**
 * cvmx_npei_pkt#_in_bp
 *
 * NPEI_PKT[0..31]_IN_BP = NPEI Packet ring# Input Backpressure
 *
 * The counters and thresholds for input packets to apply backpressure to
 * processing of the packets.
 */
union cvmx_npei_pktx_in_bp {
	u64 u64;
	struct cvmx_npei_pktx_in_bp_s {
		u64 wmark : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_pktx_in_bp_s cn52xx;
	struct cvmx_npei_pktx_in_bp_s cn56xx;
};

typedef union cvmx_npei_pktx_in_bp cvmx_npei_pktx_in_bp_t;

/**
 * cvmx_npei_pkt#_instr_baddr
 *
 * NPEI_PKT[0..31]_INSTR_BADDR = NPEI Packet ring# Instruction Base Address
 *
 * Start of Instruction for input packets.
 */
union cvmx_npei_pktx_instr_baddr {
	u64 u64;
	struct cvmx_npei_pktx_instr_baddr_s {
		u64 addr : 61;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_npei_pktx_instr_baddr_s cn52xx;
	struct cvmx_npei_pktx_instr_baddr_s cn56xx;
};

typedef union cvmx_npei_pktx_instr_baddr cvmx_npei_pktx_instr_baddr_t;

/**
 * cvmx_npei_pkt#_instr_baoff_dbell
 *
 * NPEI_PKT[0..31]_INSTR_BAOFF_DBELL = NPEI Packet ring# Instruction Base
 * Address Offset and Doorbell
 *
 * The doorbell and base address offset for next read.
 */
union cvmx_npei_pktx_instr_baoff_dbell {
	u64 u64;
	struct cvmx_npei_pktx_instr_baoff_dbell_s {
		u64 aoff : 32;
		u64 dbell : 32;
	} s;
	struct cvmx_npei_pktx_instr_baoff_dbell_s cn52xx;
	struct cvmx_npei_pktx_instr_baoff_dbell_s cn56xx;
};

typedef union cvmx_npei_pktx_instr_baoff_dbell
	cvmx_npei_pktx_instr_baoff_dbell_t;

/**
 * cvmx_npei_pkt#_instr_fifo_rsize
 *
 * NPEI_PKT[0..31]_INSTR_FIFO_RSIZE = NPEI Packet ring# Instruction FIFO and
 * Ring Size.
 *
 * Fifo field and ring size for Instructions.
 */
union cvmx_npei_pktx_instr_fifo_rsize {
	u64 u64;
	struct cvmx_npei_pktx_instr_fifo_rsize_s {
		u64 max : 9;
		u64 rrp : 9;
		u64 wrp : 9;
		u64 fcnt : 5;
		u64 rsize : 32;
	} s;
	struct cvmx_npei_pktx_instr_fifo_rsize_s cn52xx;
	struct cvmx_npei_pktx_instr_fifo_rsize_s cn56xx;
};

typedef union cvmx_npei_pktx_instr_fifo_rsize cvmx_npei_pktx_instr_fifo_rsize_t;

/**
 * cvmx_npei_pkt#_instr_header
 *
 * NPEI_PKT[0..31]_INSTR_HEADER = NPEI Packet ring# Instruction Header.
 *
 * VAlues used to build input packet header.
 */
union cvmx_npei_pktx_instr_header {
	u64 u64;
	struct cvmx_npei_pktx_instr_header_s {
		u64 reserved_44_63 : 20;
		u64 pbp : 1;
		u64 reserved_38_42 : 5;
		u64 rparmode : 2;
		u64 reserved_35_35 : 1;
		u64 rskp_len : 7;
		u64 reserved_22_27 : 6;
		u64 use_ihdr : 1;
		u64 reserved_16_20 : 5;
		u64 par_mode : 2;
		u64 reserved_13_13 : 1;
		u64 skp_len : 7;
		u64 reserved_0_5 : 6;
	} s;
	struct cvmx_npei_pktx_instr_header_s cn52xx;
	struct cvmx_npei_pktx_instr_header_s cn56xx;
};

typedef union cvmx_npei_pktx_instr_header cvmx_npei_pktx_instr_header_t;

/**
 * cvmx_npei_pkt#_slist_baddr
 *
 * NPEI_PKT[0..31]_SLIST_BADDR = NPEI Packet ring# Scatter List Base Address
 *
 * Start of Scatter List for output packet pointers - MUST be 16 byte aligned
 */
union cvmx_npei_pktx_slist_baddr {
	u64 u64;
	struct cvmx_npei_pktx_slist_baddr_s {
		u64 addr : 60;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_npei_pktx_slist_baddr_s cn52xx;
	struct cvmx_npei_pktx_slist_baddr_s cn56xx;
};

typedef union cvmx_npei_pktx_slist_baddr cvmx_npei_pktx_slist_baddr_t;

/**
 * cvmx_npei_pkt#_slist_baoff_dbell
 *
 * NPEI_PKT[0..31]_SLIST_BAOFF_DBELL = NPEI Packet ring# Scatter List Base
 * Address Offset and Doorbell
 *
 * The doorbell and base address offset for next read.
 */
union cvmx_npei_pktx_slist_baoff_dbell {
	u64 u64;
	struct cvmx_npei_pktx_slist_baoff_dbell_s {
		u64 aoff : 32;
		u64 dbell : 32;
	} s;
	struct cvmx_npei_pktx_slist_baoff_dbell_s cn52xx;
	struct cvmx_npei_pktx_slist_baoff_dbell_s cn56xx;
};

typedef union cvmx_npei_pktx_slist_baoff_dbell
	cvmx_npei_pktx_slist_baoff_dbell_t;

/**
 * cvmx_npei_pkt#_slist_fifo_rsize
 *
 * NPEI_PKT[0..31]_SLIST_FIFO_RSIZE = NPEI Packet ring# Scatter List FIFO and
 * Ring Size.
 *
 * The number of scatter pointer pairs in the scatter list.
 */
union cvmx_npei_pktx_slist_fifo_rsize {
	u64 u64;
	struct cvmx_npei_pktx_slist_fifo_rsize_s {
		u64 reserved_32_63 : 32;
		u64 rsize : 32;
	} s;
	struct cvmx_npei_pktx_slist_fifo_rsize_s cn52xx;
	struct cvmx_npei_pktx_slist_fifo_rsize_s cn56xx;
};

typedef union cvmx_npei_pktx_slist_fifo_rsize cvmx_npei_pktx_slist_fifo_rsize_t;

/**
 * cvmx_npei_pkt_cnt_int
 *
 * NPEI_PKT_CNT_INT = NPI Packet Counter Interrupt
 *
 * The packets rings that are interrupting because of Packet Counters.
 */
union cvmx_npei_pkt_cnt_int {
	u64 u64;
	struct cvmx_npei_pkt_cnt_int_s {
		u64 reserved_32_63 : 32;
		u64 port : 32;
	} s;
	struct cvmx_npei_pkt_cnt_int_s cn52xx;
	struct cvmx_npei_pkt_cnt_int_s cn56xx;
};

typedef union cvmx_npei_pkt_cnt_int cvmx_npei_pkt_cnt_int_t;

/**
 * cvmx_npei_pkt_cnt_int_enb
 *
 * NPEI_PKT_CNT_INT_ENB = NPI Packet Counter Interrupt Enable
 *
 * Enable for the packets rings that are interrupting because of Packet Counters.
 */
union cvmx_npei_pkt_cnt_int_enb {
	u64 u64;
	struct cvmx_npei_pkt_cnt_int_enb_s {
		u64 reserved_32_63 : 32;
		u64 port : 32;
	} s;
	struct cvmx_npei_pkt_cnt_int_enb_s cn52xx;
	struct cvmx_npei_pkt_cnt_int_enb_s cn56xx;
};

typedef union cvmx_npei_pkt_cnt_int_enb cvmx_npei_pkt_cnt_int_enb_t;

/**
 * cvmx_npei_pkt_data_out_es
 *
 * NPEI_PKT_DATA_OUT_ES = NPEI's Packet Data Out Endian Swap
 *
 * The Endian Swap for writing Data Out.
 */
union cvmx_npei_pkt_data_out_es {
	u64 u64;
	struct cvmx_npei_pkt_data_out_es_s {
		u64 es : 64;
	} s;
	struct cvmx_npei_pkt_data_out_es_s cn52xx;
	struct cvmx_npei_pkt_data_out_es_s cn56xx;
};

typedef union cvmx_npei_pkt_data_out_es cvmx_npei_pkt_data_out_es_t;

/**
 * cvmx_npei_pkt_data_out_ns
 *
 * NPEI_PKT_DATA_OUT_NS = NPEI's Packet Data Out No Snoop
 *
 * The NS field for the TLP when writing packet data.
 */
union cvmx_npei_pkt_data_out_ns {
	u64 u64;
	struct cvmx_npei_pkt_data_out_ns_s {
		u64 reserved_32_63 : 32;
		u64 nsr : 32;
	} s;
	struct cvmx_npei_pkt_data_out_ns_s cn52xx;
	struct cvmx_npei_pkt_data_out_ns_s cn56xx;
};

typedef union cvmx_npei_pkt_data_out_ns cvmx_npei_pkt_data_out_ns_t;

/**
 * cvmx_npei_pkt_data_out_ror
 *
 * NPEI_PKT_DATA_OUT_ROR = NPEI's Packet Data Out Relaxed Ordering
 *
 * The ROR field for the TLP when writing Packet Data.
 */
union cvmx_npei_pkt_data_out_ror {
	u64 u64;
	struct cvmx_npei_pkt_data_out_ror_s {
		u64 reserved_32_63 : 32;
		u64 ror : 32;
	} s;
	struct cvmx_npei_pkt_data_out_ror_s cn52xx;
	struct cvmx_npei_pkt_data_out_ror_s cn56xx;
};

typedef union cvmx_npei_pkt_data_out_ror cvmx_npei_pkt_data_out_ror_t;

/**
 * cvmx_npei_pkt_dpaddr
 *
 * NPEI_PKT_DPADDR = NPEI's Packet Data Pointer Addr
 *
 * Used to detemine address and attributes for packet data writes.
 */
union cvmx_npei_pkt_dpaddr {
	u64 u64;
	struct cvmx_npei_pkt_dpaddr_s {
		u64 reserved_32_63 : 32;
		u64 dptr : 32;
	} s;
	struct cvmx_npei_pkt_dpaddr_s cn52xx;
	struct cvmx_npei_pkt_dpaddr_s cn56xx;
};

typedef union cvmx_npei_pkt_dpaddr cvmx_npei_pkt_dpaddr_t;

/**
 * cvmx_npei_pkt_in_bp
 *
 * NPEI_PKT_IN_BP = NPEI Packet Input Backpressure
 *
 * Which input rings have backpressure applied.
 */
union cvmx_npei_pkt_in_bp {
	u64 u64;
	struct cvmx_npei_pkt_in_bp_s {
		u64 reserved_32_63 : 32;
		u64 bp : 32;
	} s;
	struct cvmx_npei_pkt_in_bp_s cn52xx;
	struct cvmx_npei_pkt_in_bp_s cn56xx;
};

typedef union cvmx_npei_pkt_in_bp cvmx_npei_pkt_in_bp_t;

/**
 * cvmx_npei_pkt_in_done#_cnts
 *
 * NPEI_PKT_IN_DONE[0..31]_CNTS = NPEI Instruction Done ring# Counts
 *
 * Counters for instructions completed on Input rings.
 */
union cvmx_npei_pkt_in_donex_cnts {
	u64 u64;
	struct cvmx_npei_pkt_in_donex_cnts_s {
		u64 reserved_32_63 : 32;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_pkt_in_donex_cnts_s cn52xx;
	struct cvmx_npei_pkt_in_donex_cnts_s cn56xx;
};

typedef union cvmx_npei_pkt_in_donex_cnts cvmx_npei_pkt_in_donex_cnts_t;

/**
 * cvmx_npei_pkt_in_instr_counts
 *
 * NPEI_PKT_IN_INSTR_COUNTS = NPEI Packet Input Instrutction Counts
 *
 * Keeps track of the number of instructions read into the FIFO and Packets
 * sent to IPD.
 */
union cvmx_npei_pkt_in_instr_counts {
	u64 u64;
	struct cvmx_npei_pkt_in_instr_counts_s {
		u64 wr_cnt : 32;
		u64 rd_cnt : 32;
	} s;
	struct cvmx_npei_pkt_in_instr_counts_s cn52xx;
	struct cvmx_npei_pkt_in_instr_counts_s cn56xx;
};

typedef union cvmx_npei_pkt_in_instr_counts cvmx_npei_pkt_in_instr_counts_t;

/**
 * cvmx_npei_pkt_in_pcie_port
 *
 * NPEI_PKT_IN_PCIE_PORT = NPEI's Packet In To PCIe Port Assignment
 *
 * Assigns Packet Input rings to PCIe ports.
 */
union cvmx_npei_pkt_in_pcie_port {
	u64 u64;
	struct cvmx_npei_pkt_in_pcie_port_s {
		u64 pp : 64;
	} s;
	struct cvmx_npei_pkt_in_pcie_port_s cn52xx;
	struct cvmx_npei_pkt_in_pcie_port_s cn56xx;
};

typedef union cvmx_npei_pkt_in_pcie_port cvmx_npei_pkt_in_pcie_port_t;

/**
 * cvmx_npei_pkt_input_control
 *
 * NPEI_PKT_INPUT_CONTROL = NPEI's Packet Input Control
 *
 * Control for reads for gather list and instructions.
 */
union cvmx_npei_pkt_input_control {
	u64 u64;
	struct cvmx_npei_pkt_input_control_s {
		u64 reserved_23_63 : 41;
		u64 pkt_rr : 1;
		u64 pbp_dhi : 13;
		u64 d_nsr : 1;
		u64 d_esr : 2;
		u64 d_ror : 1;
		u64 use_csr : 1;
		u64 nsr : 1;
		u64 esr : 2;
		u64 ror : 1;
	} s;
	struct cvmx_npei_pkt_input_control_s cn52xx;
	struct cvmx_npei_pkt_input_control_s cn56xx;
};

typedef union cvmx_npei_pkt_input_control cvmx_npei_pkt_input_control_t;

/**
 * cvmx_npei_pkt_instr_enb
 *
 * NPEI_PKT_INSTR_ENB = NPEI's Packet Instruction Enable
 *
 * Enables the instruction fetch for a Packet-ring.
 */
union cvmx_npei_pkt_instr_enb {
	u64 u64;
	struct cvmx_npei_pkt_instr_enb_s {
		u64 reserved_32_63 : 32;
		u64 enb : 32;
	} s;
	struct cvmx_npei_pkt_instr_enb_s cn52xx;
	struct cvmx_npei_pkt_instr_enb_s cn56xx;
};

typedef union cvmx_npei_pkt_instr_enb cvmx_npei_pkt_instr_enb_t;

/**
 * cvmx_npei_pkt_instr_rd_size
 *
 * NPEI_PKT_INSTR_RD_SIZE = NPEI Instruction Read Size
 *
 * The number of instruction allowed to be read at one time.
 */
union cvmx_npei_pkt_instr_rd_size {
	u64 u64;
	struct cvmx_npei_pkt_instr_rd_size_s {
		u64 rdsize : 64;
	} s;
	struct cvmx_npei_pkt_instr_rd_size_s cn52xx;
	struct cvmx_npei_pkt_instr_rd_size_s cn56xx;
};

typedef union cvmx_npei_pkt_instr_rd_size cvmx_npei_pkt_instr_rd_size_t;

/**
 * cvmx_npei_pkt_instr_size
 *
 * NPEI_PKT_INSTR_SIZE = NPEI's Packet Instruction Size
 *
 * Determines if instructions are 64 or 32 byte in size for a Packet-ring.
 */
union cvmx_npei_pkt_instr_size {
	u64 u64;
	struct cvmx_npei_pkt_instr_size_s {
		u64 reserved_32_63 : 32;
		u64 is_64b : 32;
	} s;
	struct cvmx_npei_pkt_instr_size_s cn52xx;
	struct cvmx_npei_pkt_instr_size_s cn56xx;
};

typedef union cvmx_npei_pkt_instr_size cvmx_npei_pkt_instr_size_t;

/**
 * cvmx_npei_pkt_int_levels
 *
 * 0x90F0 reserved NPEI_PKT_PCIE_PORT2
 *
 *
 *                  NPEI_PKT_INT_LEVELS = NPEI's Packet Interrupt Levels
 *
 * Output packet interrupt levels.
 */
union cvmx_npei_pkt_int_levels {
	u64 u64;
	struct cvmx_npei_pkt_int_levels_s {
		u64 reserved_54_63 : 10;
		u64 time : 22;
		u64 cnt : 32;
	} s;
	struct cvmx_npei_pkt_int_levels_s cn52xx;
	struct cvmx_npei_pkt_int_levels_s cn56xx;
};

typedef union cvmx_npei_pkt_int_levels cvmx_npei_pkt_int_levels_t;

/**
 * cvmx_npei_pkt_iptr
 *
 * NPEI_PKT_IPTR = NPEI's Packet Info Poitner
 *
 * Controls using the Info-Pointer to store length and data.
 */
union cvmx_npei_pkt_iptr {
	u64 u64;
	struct cvmx_npei_pkt_iptr_s {
		u64 reserved_32_63 : 32;
		u64 iptr : 32;
	} s;
	struct cvmx_npei_pkt_iptr_s cn52xx;
	struct cvmx_npei_pkt_iptr_s cn56xx;
};

typedef union cvmx_npei_pkt_iptr cvmx_npei_pkt_iptr_t;

/**
 * cvmx_npei_pkt_out_bmode
 *
 * NPEI_PKT_OUT_BMODE = NPEI's Packet Out Byte Mode
 *
 * Control the updating of the NPEI_PKT#_CNT register.
 */
union cvmx_npei_pkt_out_bmode {
	u64 u64;
	struct cvmx_npei_pkt_out_bmode_s {
		u64 reserved_32_63 : 32;
		u64 bmode : 32;
	} s;
	struct cvmx_npei_pkt_out_bmode_s cn52xx;
	struct cvmx_npei_pkt_out_bmode_s cn56xx;
};

typedef union cvmx_npei_pkt_out_bmode cvmx_npei_pkt_out_bmode_t;

/**
 * cvmx_npei_pkt_out_enb
 *
 * NPEI_PKT_OUT_ENB = NPEI's Packet Output Enable
 *
 * Enables the output packet engines.
 */
union cvmx_npei_pkt_out_enb {
	u64 u64;
	struct cvmx_npei_pkt_out_enb_s {
		u64 reserved_32_63 : 32;
		u64 enb : 32;
	} s;
	struct cvmx_npei_pkt_out_enb_s cn52xx;
	struct cvmx_npei_pkt_out_enb_s cn56xx;
};

typedef union cvmx_npei_pkt_out_enb cvmx_npei_pkt_out_enb_t;

/**
 * cvmx_npei_pkt_output_wmark
 *
 * NPEI_PKT_OUTPUT_WMARK = NPEI's Packet Output Water Mark
 *
 * Value that when the NPEI_PKT#_SLIST_BAOFF_DBELL[DBELL] value is less then
 * that backpressure for the rings will be applied.
 */
union cvmx_npei_pkt_output_wmark {
	u64 u64;
	struct cvmx_npei_pkt_output_wmark_s {
		u64 reserved_32_63 : 32;
		u64 wmark : 32;
	} s;
	struct cvmx_npei_pkt_output_wmark_s cn52xx;
	struct cvmx_npei_pkt_output_wmark_s cn56xx;
};

typedef union cvmx_npei_pkt_output_wmark cvmx_npei_pkt_output_wmark_t;

/**
 * cvmx_npei_pkt_pcie_port
 *
 * NPEI_PKT_PCIE_PORT = NPEI's Packet To PCIe Port Assignment
 *
 * Assigns Packet Ports to PCIe ports.
 */
union cvmx_npei_pkt_pcie_port {
	u64 u64;
	struct cvmx_npei_pkt_pcie_port_s {
		u64 pp : 64;
	} s;
	struct cvmx_npei_pkt_pcie_port_s cn52xx;
	struct cvmx_npei_pkt_pcie_port_s cn56xx;
};

typedef union cvmx_npei_pkt_pcie_port cvmx_npei_pkt_pcie_port_t;

/**
 * cvmx_npei_pkt_port_in_rst
 *
 * NPEI_PKT_PORT_IN_RST = NPEI Packet Port In Reset
 *
 * Vector bits related to ring-port for ones that are reset.
 */
union cvmx_npei_pkt_port_in_rst {
	u64 u64;
	struct cvmx_npei_pkt_port_in_rst_s {
		u64 in_rst : 32;
		u64 out_rst : 32;
	} s;
	struct cvmx_npei_pkt_port_in_rst_s cn52xx;
	struct cvmx_npei_pkt_port_in_rst_s cn56xx;
};

typedef union cvmx_npei_pkt_port_in_rst cvmx_npei_pkt_port_in_rst_t;

/**
 * cvmx_npei_pkt_slist_es
 *
 * NPEI_PKT_SLIST_ES = NPEI's Packet Scatter List Endian Swap
 *
 * The Endian Swap for Scatter List Read.
 */
union cvmx_npei_pkt_slist_es {
	u64 u64;
	struct cvmx_npei_pkt_slist_es_s {
		u64 es : 64;
	} s;
	struct cvmx_npei_pkt_slist_es_s cn52xx;
	struct cvmx_npei_pkt_slist_es_s cn56xx;
};

typedef union cvmx_npei_pkt_slist_es cvmx_npei_pkt_slist_es_t;

/**
 * cvmx_npei_pkt_slist_id_size
 *
 * NPEI_PKT_SLIST_ID_SIZE = NPEI Packet Scatter List Info and Data Size
 *
 * The Size of the information and data fields pointed to by Scatter List
 * pointers.
 */
union cvmx_npei_pkt_slist_id_size {
	u64 u64;
	struct cvmx_npei_pkt_slist_id_size_s {
		u64 reserved_23_63 : 41;
		u64 isize : 7;
		u64 bsize : 16;
	} s;
	struct cvmx_npei_pkt_slist_id_size_s cn52xx;
	struct cvmx_npei_pkt_slist_id_size_s cn56xx;
};

typedef union cvmx_npei_pkt_slist_id_size cvmx_npei_pkt_slist_id_size_t;

/**
 * cvmx_npei_pkt_slist_ns
 *
 * NPEI_PKT_SLIST_NS = NPEI's Packet Scatter List No Snoop
 *
 * The NS field for the TLP when fetching Scatter List.
 */
union cvmx_npei_pkt_slist_ns {
	u64 u64;
	struct cvmx_npei_pkt_slist_ns_s {
		u64 reserved_32_63 : 32;
		u64 nsr : 32;
	} s;
	struct cvmx_npei_pkt_slist_ns_s cn52xx;
	struct cvmx_npei_pkt_slist_ns_s cn56xx;
};

typedef union cvmx_npei_pkt_slist_ns cvmx_npei_pkt_slist_ns_t;

/**
 * cvmx_npei_pkt_slist_ror
 *
 * NPEI_PKT_SLIST_ROR = NPEI's Packet Scatter List Relaxed Ordering
 *
 * The ROR field for the TLP when fetching Scatter List.
 */
union cvmx_npei_pkt_slist_ror {
	u64 u64;
	struct cvmx_npei_pkt_slist_ror_s {
		u64 reserved_32_63 : 32;
		u64 ror : 32;
	} s;
	struct cvmx_npei_pkt_slist_ror_s cn52xx;
	struct cvmx_npei_pkt_slist_ror_s cn56xx;
};

typedef union cvmx_npei_pkt_slist_ror cvmx_npei_pkt_slist_ror_t;

/**
 * cvmx_npei_pkt_time_int
 *
 * NPEI_PKT_TIME_INT = NPEI Packet Timer Interrupt
 *
 * The packets rings that are interrupting because of Packet Timers.
 */
union cvmx_npei_pkt_time_int {
	u64 u64;
	struct cvmx_npei_pkt_time_int_s {
		u64 reserved_32_63 : 32;
		u64 port : 32;
	} s;
	struct cvmx_npei_pkt_time_int_s cn52xx;
	struct cvmx_npei_pkt_time_int_s cn56xx;
};

typedef union cvmx_npei_pkt_time_int cvmx_npei_pkt_time_int_t;

/**
 * cvmx_npei_pkt_time_int_enb
 *
 * NPEI_PKT_TIME_INT_ENB = NPEI Packet Timer Interrupt Enable
 *
 * The packets rings that are interrupting because of Packet Timers.
 */
union cvmx_npei_pkt_time_int_enb {
	u64 u64;
	struct cvmx_npei_pkt_time_int_enb_s {
		u64 reserved_32_63 : 32;
		u64 port : 32;
	} s;
	struct cvmx_npei_pkt_time_int_enb_s cn52xx;
	struct cvmx_npei_pkt_time_int_enb_s cn56xx;
};

typedef union cvmx_npei_pkt_time_int_enb cvmx_npei_pkt_time_int_enb_t;

/**
 * cvmx_npei_rsl_int_blocks
 *
 * NPEI_RSL_INT_BLOCKS = NPEI RSL Interrupt Blocks Register
 *
 * Reading this register will return a vector with a bit set '1' for a
 * corresponding RSL block
 * that presently has an interrupt pending. The Field Description below
 * supplies the name of the
 * register that software should read to find out why that intterupt bit is set.
 */
union cvmx_npei_rsl_int_blocks {
	u64 u64;
	struct cvmx_npei_rsl_int_blocks_s {
		u64 reserved_31_63 : 33;
		u64 iob : 1;
		u64 lmc1 : 1;
		u64 agl : 1;
		u64 reserved_24_27 : 4;
		u64 asxpcs1 : 1;
		u64 asxpcs0 : 1;
		u64 reserved_21_21 : 1;
		u64 pip : 1;
		u64 spx1 : 1;
		u64 spx0 : 1;
		u64 lmc0 : 1;
		u64 l2c : 1;
		u64 usb1 : 1;
		u64 rad : 1;
		u64 usb : 1;
		u64 pow : 1;
		u64 tim : 1;
		u64 pko : 1;
		u64 ipd : 1;
		u64 reserved_8_8 : 1;
		u64 zip : 1;
		u64 dfa : 1;
		u64 fpa : 1;
		u64 key : 1;
		u64 npei : 1;
		u64 gmx1 : 1;
		u64 gmx0 : 1;
		u64 mio : 1;
	} s;
	struct cvmx_npei_rsl_int_blocks_s cn52xx;
	struct cvmx_npei_rsl_int_blocks_s cn52xxp1;
	struct cvmx_npei_rsl_int_blocks_s cn56xx;
	struct cvmx_npei_rsl_int_blocks_s cn56xxp1;
};

typedef union cvmx_npei_rsl_int_blocks cvmx_npei_rsl_int_blocks_t;

/**
 * cvmx_npei_scratch_1
 *
 * NPEI_SCRATCH_1 = NPEI's Scratch 1
 *
 * A general purpose 64 bit register for SW use.
 */
union cvmx_npei_scratch_1 {
	u64 u64;
	struct cvmx_npei_scratch_1_s {
		u64 data : 64;
	} s;
	struct cvmx_npei_scratch_1_s cn52xx;
	struct cvmx_npei_scratch_1_s cn52xxp1;
	struct cvmx_npei_scratch_1_s cn56xx;
	struct cvmx_npei_scratch_1_s cn56xxp1;
};

typedef union cvmx_npei_scratch_1 cvmx_npei_scratch_1_t;

/**
 * cvmx_npei_state1
 *
 * NPEI_STATE1 = NPEI State 1
 *
 * State machines in NPEI. For debug.
 */
union cvmx_npei_state1 {
	u64 u64;
	struct cvmx_npei_state1_s {
		u64 cpl1 : 12;
		u64 cpl0 : 12;
		u64 arb : 1;
		u64 csr : 39;
	} s;
	struct cvmx_npei_state1_s cn52xx;
	struct cvmx_npei_state1_s cn52xxp1;
	struct cvmx_npei_state1_s cn56xx;
	struct cvmx_npei_state1_s cn56xxp1;
};

typedef union cvmx_npei_state1 cvmx_npei_state1_t;

/**
 * cvmx_npei_state2
 *
 * NPEI_STATE2 = NPEI State 2
 *
 * State machines in NPEI. For debug.
 */
union cvmx_npei_state2 {
	u64 u64;
	struct cvmx_npei_state2_s {
		u64 reserved_48_63 : 16;
		u64 npei : 1;
		u64 rac : 1;
		u64 csm1 : 15;
		u64 csm0 : 15;
		u64 nnp0 : 8;
		u64 nnd : 8;
	} s;
	struct cvmx_npei_state2_s cn52xx;
	struct cvmx_npei_state2_s cn52xxp1;
	struct cvmx_npei_state2_s cn56xx;
	struct cvmx_npei_state2_s cn56xxp1;
};

typedef union cvmx_npei_state2 cvmx_npei_state2_t;

/**
 * cvmx_npei_state3
 *
 * NPEI_STATE3 = NPEI State 3
 *
 * State machines in NPEI. For debug.
 */
union cvmx_npei_state3 {
	u64 u64;
	struct cvmx_npei_state3_s {
		u64 reserved_56_63 : 8;
		u64 psm1 : 15;
		u64 psm0 : 15;
		u64 nsm1 : 13;
		u64 nsm0 : 13;
	} s;
	struct cvmx_npei_state3_s cn52xx;
	struct cvmx_npei_state3_s cn52xxp1;
	struct cvmx_npei_state3_s cn56xx;
	struct cvmx_npei_state3_s cn56xxp1;
};

typedef union cvmx_npei_state3 cvmx_npei_state3_t;

/**
 * cvmx_npei_win_rd_addr
 *
 * NPEI_WIN_RD_ADDR = NPEI Window Read Address Register
 *
 * The address to be read when the NPEI_WIN_RD_DATA register is read.
 */
union cvmx_npei_win_rd_addr {
	u64 u64;
	struct cvmx_npei_win_rd_addr_s {
		u64 reserved_51_63 : 13;
		u64 ld_cmd : 2;
		u64 iobit : 1;
		u64 rd_addr : 48;
	} s;
	struct cvmx_npei_win_rd_addr_s cn52xx;
	struct cvmx_npei_win_rd_addr_s cn52xxp1;
	struct cvmx_npei_win_rd_addr_s cn56xx;
	struct cvmx_npei_win_rd_addr_s cn56xxp1;
};

typedef union cvmx_npei_win_rd_addr cvmx_npei_win_rd_addr_t;

/**
 * cvmx_npei_win_rd_data
 *
 * NPEI_WIN_RD_DATA = NPEI Window Read Data Register
 *
 * Reading this register causes a window read operation to take place.
 * Address read is that contained in the NPEI_WIN_RD_ADDR
 * register.
 */
union cvmx_npei_win_rd_data {
	u64 u64;
	struct cvmx_npei_win_rd_data_s {
		u64 rd_data : 64;
	} s;
	struct cvmx_npei_win_rd_data_s cn52xx;
	struct cvmx_npei_win_rd_data_s cn52xxp1;
	struct cvmx_npei_win_rd_data_s cn56xx;
	struct cvmx_npei_win_rd_data_s cn56xxp1;
};

typedef union cvmx_npei_win_rd_data cvmx_npei_win_rd_data_t;

/**
 * cvmx_npei_win_wr_addr
 *
 * NPEI_WIN_WR_ADDR = NPEI Window Write Address Register
 *
 * Contains the address to be writen to when a write operation is started by
 * writing the
 * NPEI_WIN_WR_DATA register (see below).
 *
 * Notes:
 * Even though address bit [2] can be set, it should always be kept to '0'.
 *
 */
union cvmx_npei_win_wr_addr {
	u64 u64;
	struct cvmx_npei_win_wr_addr_s {
		u64 reserved_49_63 : 15;
		u64 iobit : 1;
		u64 wr_addr : 46;
		u64 reserved_0_1 : 2;
	} s;
	struct cvmx_npei_win_wr_addr_s cn52xx;
	struct cvmx_npei_win_wr_addr_s cn52xxp1;
	struct cvmx_npei_win_wr_addr_s cn56xx;
	struct cvmx_npei_win_wr_addr_s cn56xxp1;
};

typedef union cvmx_npei_win_wr_addr cvmx_npei_win_wr_addr_t;

/**
 * cvmx_npei_win_wr_data
 *
 * NPEI_WIN_WR_DATA = NPEI Window Write Data Register
 *
 * Contains the data to write to the address located in the NPEI_WIN_WR_ADDR
 * Register.
 * Writing the least-significant-byte of this register will cause a write
 * operation to take place.
 */
union cvmx_npei_win_wr_data {
	u64 u64;
	struct cvmx_npei_win_wr_data_s {
		u64 wr_data : 64;
	} s;
	struct cvmx_npei_win_wr_data_s cn52xx;
	struct cvmx_npei_win_wr_data_s cn52xxp1;
	struct cvmx_npei_win_wr_data_s cn56xx;
	struct cvmx_npei_win_wr_data_s cn56xxp1;
};

typedef union cvmx_npei_win_wr_data cvmx_npei_win_wr_data_t;

/**
 * cvmx_npei_win_wr_mask
 *
 * NPEI_WIN_WR_MASK = NPEI Window Write Mask Register
 *
 * Contains the mask for the data in the NPEI_WIN_WR_DATA Register.
 */
union cvmx_npei_win_wr_mask {
	u64 u64;
	struct cvmx_npei_win_wr_mask_s {
		u64 reserved_8_63 : 56;
		u64 wr_mask : 8;
	} s;
	struct cvmx_npei_win_wr_mask_s cn52xx;
	struct cvmx_npei_win_wr_mask_s cn52xxp1;
	struct cvmx_npei_win_wr_mask_s cn56xx;
	struct cvmx_npei_win_wr_mask_s cn56xxp1;
};

typedef union cvmx_npei_win_wr_mask cvmx_npei_win_wr_mask_t;

/**
 * cvmx_npei_window_ctl
 *
 * NPEI_WINDOW_CTL = NPEI's Window Control
 *
 * The name of this register is misleading. The timeout value is used for BAR0
 * access from PCIE0 and PCIE1.
 * Any access to the regigisters on the RML will timeout as 0xFFFF clock cycle.
 * At time of timeout the next
 * RML access will start, and interrupt will be set, and in the case of reads
 * no data will be returned.
 *
 * The value of this register should be set to a minimum of 0x200000 to ensure
 * that a timeout to an RML register
 * occurs on the RML 0xFFFF timer before the timeout for a BAR0 access from
 * the PCIE#.
 */
union cvmx_npei_window_ctl {
	u64 u64;
	struct cvmx_npei_window_ctl_s {
		u64 reserved_32_63 : 32;
		u64 time : 32;
	} s;
	struct cvmx_npei_window_ctl_s cn52xx;
	struct cvmx_npei_window_ctl_s cn52xxp1;
	struct cvmx_npei_window_ctl_s cn56xx;
	struct cvmx_npei_window_ctl_s cn56xxp1;
};

typedef union cvmx_npei_window_ctl cvmx_npei_window_ctl_t;

#endif
