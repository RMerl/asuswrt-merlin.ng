/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon bgxx.
 */

#ifndef __CVMX_BGXX_DEFS_H__
#define __CVMX_BGXX_DEFS_H__

#define CVMX_BGXX_CMRX_CONFIG(offset, block_id)                                                    \
	(0x00011800E0000000ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_INT(offset, block_id)                                                       \
	(0x00011800E0000020ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_PRT_CBFC_CTL(offset, block_id)                                              \
	(0x00011800E0000408ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_ADR_CTL(offset, block_id)                                                \
	(0x00011800E00000A0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_BP_DROP(offset, block_id)                                                \
	(0x00011800E0000080ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_BP_OFF(offset, block_id)                                                 \
	(0x00011800E0000090ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_BP_ON(offset, block_id)                                                  \
	(0x00011800E0000088ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_BP_STATUS(offset, block_id)                                              \
	(0x00011800E00000A8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_FIFO_LEN(offset, block_id)                                               \
	(0x00011800E00000C0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_ID_MAP(offset, block_id)                                                 \
	(0x00011800E0000028ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_LOGL_XOFF(offset, block_id)                                              \
	(0x00011800E00000B0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_LOGL_XON(offset, block_id)                                               \
	(0x00011800E00000B8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_PAUSE_DROP_TIME(offset, block_id)                                        \
	(0x00011800E0000030ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT0(offset, block_id)                                                  \
	(0x00011800E0000038ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT1(offset, block_id)                                                  \
	(0x00011800E0000040ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT2(offset, block_id)                                                  \
	(0x00011800E0000048ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT3(offset, block_id)                                                  \
	(0x00011800E0000050ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT4(offset, block_id)                                                  \
	(0x00011800E0000058ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT5(offset, block_id)                                                  \
	(0x00011800E0000060ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT6(offset, block_id)                                                  \
	(0x00011800E0000068ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT7(offset, block_id)                                                  \
	(0x00011800E0000070ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_STAT8(offset, block_id)                                                  \
	(0x00011800E0000078ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_RX_WEIGHT(offset, block_id)                                                 \
	(0x00011800E0000098ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_CHANNEL(offset, block_id)                                                \
	(0x00011800E0000400ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_FIFO_LEN(offset, block_id)                                               \
	(0x00011800E0000418ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_HG2_STATUS(offset, block_id)                                             \
	(0x00011800E0000410ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_OVR_BP(offset, block_id)                                                 \
	(0x00011800E0000420ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT0(offset, block_id)                                                  \
	(0x00011800E0000508ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT1(offset, block_id)                                                  \
	(0x00011800E0000510ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT10(offset, block_id)                                                 \
	(0x00011800E0000558ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT11(offset, block_id)                                                 \
	(0x00011800E0000560ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT12(offset, block_id)                                                 \
	(0x00011800E0000568ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT13(offset, block_id)                                                 \
	(0x00011800E0000570ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT14(offset, block_id)                                                 \
	(0x00011800E0000578ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT15(offset, block_id)                                                 \
	(0x00011800E0000580ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT16(offset, block_id)                                                 \
	(0x00011800E0000588ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT17(offset, block_id)                                                 \
	(0x00011800E0000590ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT2(offset, block_id)                                                  \
	(0x00011800E0000518ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT3(offset, block_id)                                                  \
	(0x00011800E0000520ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT4(offset, block_id)                                                  \
	(0x00011800E0000528ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT5(offset, block_id)                                                  \
	(0x00011800E0000530ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT6(offset, block_id)                                                  \
	(0x00011800E0000538ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT7(offset, block_id)                                                  \
	(0x00011800E0000540ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT8(offset, block_id)                                                  \
	(0x00011800E0000548ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMRX_TX_STAT9(offset, block_id)                                                  \
	(0x00011800E0000550ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_CMR_BAD(offset)	    (0x00011800E0001020ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_BIST_STATUS(offset)   (0x00011800E0000300ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_CHAN_MSK_AND(offset)  (0x00011800E0000200ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_CHAN_MSK_OR(offset)   (0x00011800E0000208ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_ECO(offset)	    (0x00011800E0001028ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_GLOBAL_CONFIG(offset) (0x00011800E0000008ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_MEM_CTRL(offset)	    (0x00011800E0000018ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_MEM_INT(offset)	    (0x00011800E0000010ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_NXC_ADR(offset)	    (0x00011800E0001018ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_RX_ADRX_CAM(offset, block_id)                                                \
	(0x00011800E0000100ull + (((offset) & 31) + ((block_id) & 7) * 0x200000ull) * 8)
#define CVMX_BGXX_CMR_RX_LMACS(offset)	(0x00011800E0000308ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_RX_OVR_BP(offset) (0x00011800E0000318ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_CMR_TX_LMACS(offset)	(0x00011800E0001000ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_PRTX_CFG(offset, block_id)                                               \
	(0x00011800E0038010ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_DECISION(offset, block_id)                                           \
	(0x00011800E0038040ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_FRM_CHK(offset, block_id)                                            \
	(0x00011800E0038020ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(offset, block_id)                                            \
	(0x00011800E0038018ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_IFG(offset, block_id)                                                \
	(0x00011800E0038058ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_INT(offset, block_id)                                                \
	(0x00011800E0038000ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_JABBER(offset, block_id)                                             \
	(0x00011800E0038038ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_RXX_UDD_SKP(offset, block_id)                                            \
	(0x00011800E0038048ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_SMACX(offset, block_id)                                                  \
	(0x00011800E0038230ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_APPEND(offset, block_id)                                             \
	(0x00011800E0038218ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_BURST(offset, block_id)                                              \
	(0x00011800E0038228ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_CTL(offset, block_id)                                                \
	(0x00011800E0038270ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_INT(offset, block_id)                                                \
	(0x00011800E0038500ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(offset, block_id)                                            \
	(0x00011800E0038240ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(offset, block_id)                                 \
	(0x00011800E0038248ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(offset, block_id)                                     \
	(0x00011800E0038238ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_TOGO(offset, block_id)                                         \
	(0x00011800E0038258ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_ZERO(offset, block_id)                                         \
	(0x00011800E0038260ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(offset, block_id)                                          \
	(0x00011800E0038300ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_SLOT(offset, block_id)                                               \
	(0x00011800E0038220ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_SOFT_PAUSE(offset, block_id)                                         \
	(0x00011800E0038250ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TXX_THRESH(offset, block_id)                                             \
	(0x00011800E0038210ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_GMI_TX_COL_ATTEMPT(offset)                                                   \
	(0x00011800E0039010ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_TX_IFG(offset)  (0x00011800E0039000ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_TX_JAM(offset)  (0x00011800E0039008ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_TX_LFSR(offset) (0x00011800E0039028ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(offset)                                                \
	(0x00011800E0039018ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(offset)                                                \
	(0x00011800E0039020ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_GMP_PCS_ANX_ADV(offset, block_id)                                                \
	(0x00011800E0030010ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_ANX_EXT_ST(offset, block_id)                                             \
	(0x00011800E0030028ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_ANX_LP_ABIL(offset, block_id)                                            \
	(0x00011800E0030018ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_ANX_RESULTS(offset, block_id)                                            \
	(0x00011800E0030020ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_INTX(offset, block_id)                                                   \
	(0x00011800E0030080ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_LINKX_TIMER(offset, block_id)                                            \
	(0x00011800E0030040ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_MISCX_CTL(offset, block_id)                                              \
	(0x00011800E0030078ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_MRX_CONTROL(offset, block_id)                                            \
	(0x00011800E0030000ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_MRX_STATUS(offset, block_id)                                             \
	(0x00011800E0030008ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_RXX_STATES(offset, block_id)                                             \
	(0x00011800E0030058ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_RXX_SYNC(offset, block_id)                                               \
	(0x00011800E0030050ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(offset, block_id)                                            \
	(0x00011800E0030068ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_SGMX_LP_ADV(offset, block_id)                                            \
	(0x00011800E0030070ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_TXX_STATES(offset, block_id)                                             \
	(0x00011800E0030060ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY(offset, block_id)                                        \
	(0x00011800E0030048ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_CBFC_CTL(offset, block_id)                                                  \
	(0x00011800E0020218ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_CTRL(offset, block_id)                                                      \
	(0x00011800E0020200ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_EXT_LOOPBACK(offset, block_id)                                              \
	(0x00011800E0020208ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_HG2_CONTROL(offset, block_id)                                               \
	(0x00011800E0020210ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_BAD_COL_HI(offset, block_id)                                             \
	(0x00011800E0020040ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_BAD_COL_LO(offset, block_id)                                             \
	(0x00011800E0020038ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_CTL(offset, block_id)                                                    \
	(0x00011800E0020030ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_DECISION(offset, block_id)                                               \
	(0x00011800E0020020ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_FRM_CHK(offset, block_id)                                                \
	(0x00011800E0020010ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_FRM_CTL(offset, block_id)                                                \
	(0x00011800E0020008ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_INT(offset, block_id)                                                    \
	(0x00011800E0020000ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_JABBER(offset, block_id)                                                 \
	(0x00011800E0020018ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_RX_UDD_SKP(offset, block_id)                                                \
	(0x00011800E0020028ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_SMAC(offset, block_id)                                                      \
	(0x00011800E0020108ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_APPEND(offset, block_id)                                                 \
	(0x00011800E0020100ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_CTL(offset, block_id)                                                    \
	(0x00011800E0020160ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_IFG(offset, block_id)                                                    \
	(0x00011800E0020148ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_INT(offset, block_id)                                                    \
	(0x00011800E0020140ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_MIN_PKT(offset, block_id)                                                \
	(0x00011800E0020118ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_DMAC(offset, block_id)                                         \
	(0x00011800E0020150ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_INTERVAL(offset, block_id)                                     \
	(0x00011800E0020120ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_TIME(offset, block_id)                                         \
	(0x00011800E0020110ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_TYPE(offset, block_id)                                         \
	(0x00011800E0020158ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_TOGO(offset, block_id)                                             \
	(0x00011800E0020130ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_PAUSE_ZERO(offset, block_id)                                             \
	(0x00011800E0020138ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_SOFT_PAUSE(offset, block_id)                                             \
	(0x00011800E0020128ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SMUX_TX_THRESH(offset, block_id)                                                 \
	(0x00011800E0020168ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_ADV(offset, block_id)                                                    \
	(0x00011800E00100D8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_BP_STATUS(offset, block_id)                                              \
	(0x00011800E00100F8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_CONTROL(offset, block_id)                                                \
	(0x00011800E00100C8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_LP_BASE(offset, block_id)                                                \
	(0x00011800E00100E0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_LP_XNP(offset, block_id)                                                 \
	(0x00011800E00100F0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_STATUS(offset, block_id)                                                 \
	(0x00011800E00100D0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_AN_XNP_TX(offset, block_id)                                                 \
	(0x00011800E00100E8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_ALGN_STATUS(offset, block_id)                                            \
	(0x00011800E0010050ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(offset, block_id)                                            \
	(0x00011800E0010058ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_LANE_MAP(offset, block_id)                                               \
	(0x00011800E0010060ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_CONTROL(offset, block_id)                                            \
	(0x00011800E0010068ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_LD_CUP(offset, block_id)                                             \
	(0x00011800E0010088ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_LD_REP(offset, block_id)                                             \
	(0x00011800E0010090ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_LP_CUP(offset, block_id)                                             \
	(0x00011800E0010078ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_LP_REP(offset, block_id)                                             \
	(0x00011800E0010080ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_PMD_STATUS(offset, block_id)                                             \
	(0x00011800E0010070ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_STATUS1(offset, block_id)                                                \
	(0x00011800E0010030ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_STATUS2(offset, block_id)                                                \
	(0x00011800E0010038ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_TP_CONTROL(offset, block_id)                                             \
	(0x00011800E0010040ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BR_TP_ERR_CNT(offset, block_id)                                             \
	(0x00011800E0010048ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_BX_STATUS(offset, block_id)                                                 \
	(0x00011800E0010028ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_CONTROL1(offset, block_id)                                                  \
	(0x00011800E0010000ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_CONTROL2(offset, block_id)                                                  \
	(0x00011800E0010018ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_ABIL(offset, block_id)                                                  \
	(0x00011800E0010098ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_CONTROL(offset, block_id)                                               \
	(0x00011800E00100A0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_CORR_BLKS01(offset, block_id)                                           \
	(0x00011800E00100A8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_CORR_BLKS23(offset, block_id)                                           \
	(0x00011800E00100B0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_UNCORR_BLKS01(offset, block_id)                                         \
	(0x00011800E00100B8ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_FEC_UNCORR_BLKS23(offset, block_id)                                         \
	(0x00011800E00100C0ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_INT(offset, block_id)                                                       \
	(0x00011800E0010220ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_LPCS_STATES(offset, block_id)                                               \
	(0x00011800E0010208ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_MISC_CONTROL(offset, block_id)                                              \
	(0x00011800E0010218ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_SPD_ABIL(offset, block_id)                                                  \
	(0x00011800E0010010ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_STATUS1(offset, block_id)                                                   \
	(0x00011800E0010008ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPUX_STATUS2(offset, block_id)                                                   \
	(0x00011800E0010020ull + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#define CVMX_BGXX_SPU_BIST_STATUS(offset) (0x00011800E0010318ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_SPU_DBG_CONTROL(offset) (0x00011800E0010300ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_SPU_MEM_INT(offset)	  (0x00011800E0010310ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_SPU_MEM_STATUS(offset)  (0x00011800E0010308ull + ((offset) & 7) * 0x1000000ull)
#define CVMX_BGXX_SPU_SDSX_SKEW_STATUS(offset, block_id)                                           \
	(0x00011800E0010320ull + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8)
#define CVMX_BGXX_SPU_SDSX_STATES(offset, block_id)                                                \
	(0x00011800E0010340ull + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8)

/**
 * cvmx_bgx#_cmr#_config
 *
 * Logical MAC/PCS configuration registers; one per LMAC. The maximum number of LMACs (and
 * maximum LMAC ID) that can be enabled by these registers is limited by
 * BGX()_CMR_RX_LMACS[LMACS] and BGX()_CMR_TX_LMACS[LMACS]. When multiple LMACs are
 * enabled, they must be configured with the same [LMAC_TYPE] value.
 */
union cvmx_bgxx_cmrx_config {
	u64 u64;
	struct cvmx_bgxx_cmrx_config_s {
		u64 reserved_16_63 : 48;
		u64 enable : 1;
		u64 data_pkt_rx_en : 1;
		u64 data_pkt_tx_en : 1;
		u64 int_beat_gen : 1;
		u64 mix_en : 1;
		u64 lmac_type : 3;
		u64 lane_to_sds : 8;
	} s;
	struct cvmx_bgxx_cmrx_config_s cn73xx;
	struct cvmx_bgxx_cmrx_config_s cn78xx;
	struct cvmx_bgxx_cmrx_config_s cn78xxp1;
	struct cvmx_bgxx_cmrx_config_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_config cvmx_bgxx_cmrx_config_t;

/**
 * cvmx_bgx#_cmr#_int
 */
union cvmx_bgxx_cmrx_int {
	u64 u64;
	struct cvmx_bgxx_cmrx_int_s {
		u64 reserved_3_63 : 61;
		u64 pko_nxc : 1;
		u64 overflw : 1;
		u64 pause_drp : 1;
	} s;
	struct cvmx_bgxx_cmrx_int_s cn73xx;
	struct cvmx_bgxx_cmrx_int_s cn78xx;
	struct cvmx_bgxx_cmrx_int_s cn78xxp1;
	struct cvmx_bgxx_cmrx_int_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_int cvmx_bgxx_cmrx_int_t;

/**
 * cvmx_bgx#_cmr#_prt_cbfc_ctl
 *
 * See XOFF definition listed under BGX()_SMU()_CBFC_CTL.
 *
 */
union cvmx_bgxx_cmrx_prt_cbfc_ctl {
	u64 u64;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s {
		u64 reserved_32_63 : 32;
		u64 phys_bp : 16;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s cn73xx;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s cn78xx;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s cn78xxp1;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_prt_cbfc_ctl cvmx_bgxx_cmrx_prt_cbfc_ctl_t;

/**
 * cvmx_bgx#_cmr#_rx_adr_ctl
 */
union cvmx_bgxx_cmrx_rx_adr_ctl {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s {
		u64 reserved_4_63 : 60;
		u64 cam_accept : 1;
		u64 mcst_mode : 2;
		u64 bcst_accept : 1;
	} s;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_adr_ctl cvmx_bgxx_cmrx_rx_adr_ctl_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_drop
 */
union cvmx_bgxx_cmrx_rx_bp_drop {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s {
		u64 reserved_7_63 : 57;
		u64 mark : 7;
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_bp_drop cvmx_bgxx_cmrx_rx_bp_drop_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_off
 */
union cvmx_bgxx_cmrx_rx_bp_off {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_bp_off_s {
		u64 reserved_7_63 : 57;
		u64 mark : 7;
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_off_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_bp_off_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_bp_off_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_bp_off_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_bp_off cvmx_bgxx_cmrx_rx_bp_off_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_on
 */
union cvmx_bgxx_cmrx_rx_bp_on {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_bp_on_s {
		u64 reserved_12_63 : 52;
		u64 mark : 12;
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_on_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_bp_on_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_bp_on_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_bp_on_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_bp_on cvmx_bgxx_cmrx_rx_bp_on_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_status
 */
union cvmx_bgxx_cmrx_rx_bp_status {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_bp_status_s {
		u64 reserved_1_63 : 63;
		u64 bp : 1;
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_status_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_bp_status_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_bp_status_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_bp_status_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_bp_status cvmx_bgxx_cmrx_rx_bp_status_t;

/**
 * cvmx_bgx#_cmr#_rx_fifo_len
 */
union cvmx_bgxx_cmrx_rx_fifo_len {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s {
		u64 reserved_13_63 : 51;
		u64 fifo_len : 13;
	} s;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_fifo_len cvmx_bgxx_cmrx_rx_fifo_len_t;

/**
 * cvmx_bgx#_cmr#_rx_id_map
 *
 * These registers set the RX LMAC ID mapping for X2P/PKI.
 *
 */
union cvmx_bgxx_cmrx_rx_id_map {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_id_map_s {
		u64 reserved_15_63 : 49;
		u64 rid : 7;
		u64 pknd : 8;
	} s;
	struct cvmx_bgxx_cmrx_rx_id_map_cn73xx {
		u64 reserved_15_63 : 49;
		u64 rid : 7;
		u64 reserved_6_7 : 2;
		u64 pknd : 6;
	} cn73xx;
	struct cvmx_bgxx_cmrx_rx_id_map_cn73xx cn78xx;
	struct cvmx_bgxx_cmrx_rx_id_map_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_id_map_cn73xx cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_id_map cvmx_bgxx_cmrx_rx_id_map_t;

/**
 * cvmx_bgx#_cmr#_rx_logl_xoff
 */
union cvmx_bgxx_cmrx_rx_logl_xoff {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s {
		u64 reserved_16_63 : 48;
		u64 xoff : 16;
	} s;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_logl_xoff cvmx_bgxx_cmrx_rx_logl_xoff_t;

/**
 * cvmx_bgx#_cmr#_rx_logl_xon
 */
union cvmx_bgxx_cmrx_rx_logl_xon {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s {
		u64 reserved_16_63 : 48;
		u64 xon : 16;
	} s;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_logl_xon cvmx_bgxx_cmrx_rx_logl_xon_t;

/**
 * cvmx_bgx#_cmr#_rx_pause_drop_time
 */
union cvmx_bgxx_cmrx_rx_pause_drop_time {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s {
		u64 reserved_16_63 : 48;
		u64 pause_time : 16;
	} s;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_pause_drop_time cvmx_bgxx_cmrx_rx_pause_drop_time_t;

/**
 * cvmx_bgx#_cmr#_rx_stat0
 *
 * These registers provide a count of received packets that meet the following conditions:
 * * are not recognized as PAUSE packets.
 * * are not dropped due DMAC filtering.
 * * are not dropped due FIFO full status.
 * * do not have any other OPCODE (FCS, Length, etc).
 */
union cvmx_bgxx_cmrx_rx_stat0 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat0_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat0_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat0_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat0_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat0_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat0 cvmx_bgxx_cmrx_rx_stat0_t;

/**
 * cvmx_bgx#_cmr#_rx_stat1
 *
 * These registers provide a count of octets of received packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat1 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat1_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat1_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat1_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat1_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat1_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat1 cvmx_bgxx_cmrx_rx_stat1_t;

/**
 * cvmx_bgx#_cmr#_rx_stat2
 *
 * These registers provide a count of all packets received that were recognized as flow-control
 * or PAUSE packets. PAUSE packets with any kind of error are counted in
 * BGX()_CMR()_RX_STAT8 (error stats register). Pause packets can be optionally dropped
 * or forwarded based on BGX()_SMU()_RX_FRM_CTL[CTL_DRP]. This count increments
 * regardless of whether the packet is dropped. PAUSE packets are never counted in
 * BGX()_CMR()_RX_STAT0.
 */
union cvmx_bgxx_cmrx_rx_stat2 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat2_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat2_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat2_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat2_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat2_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat2 cvmx_bgxx_cmrx_rx_stat2_t;

/**
 * cvmx_bgx#_cmr#_rx_stat3
 *
 * These registers provide a count of octets of received PAUSE and control packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat3 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat3_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat3_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat3_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat3_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat3_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat3 cvmx_bgxx_cmrx_rx_stat3_t;

/**
 * cvmx_bgx#_cmr#_rx_stat4
 *
 * These registers provide a count of all packets received that were dropped by the DMAC filter.
 * Packets that match the DMAC are dropped and counted here regardless of whether they were ERR
 * packets, but does not include those reported in BGX()_CMR()_RX_STAT6. These packets
 * are never counted in BGX()_CMR()_RX_STAT0. Eight-byte packets as the result of
 * truncation or other means are not dropped by CNXXXX and will never appear in this count.
 */
union cvmx_bgxx_cmrx_rx_stat4 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat4_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat4_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat4_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat4_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat4_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat4 cvmx_bgxx_cmrx_rx_stat4_t;

/**
 * cvmx_bgx#_cmr#_rx_stat5
 *
 * These registers provide a count of octets of filtered DMAC packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat5 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat5_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat5_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat5_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat5_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat5_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat5 cvmx_bgxx_cmrx_rx_stat5_t;

/**
 * cvmx_bgx#_cmr#_rx_stat6
 *
 * These registers provide a count of all packets received that were dropped due to a full
 * receive FIFO. They do not count any packet that is truncated at the point of overflow and sent
 * on to the PKI. These registers count all entire packets dropped by the FIFO for a given LMAC
 * regardless of DMAC or PAUSE type.
 */
union cvmx_bgxx_cmrx_rx_stat6 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat6_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat6_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat6_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat6_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat6_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat6 cvmx_bgxx_cmrx_rx_stat6_t;

/**
 * cvmx_bgx#_cmr#_rx_stat7
 *
 * These registers provide a count of octets of received packets that were dropped due to a full
 * receive FIFO.
 */
union cvmx_bgxx_cmrx_rx_stat7 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat7_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat7_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat7_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat7_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat7_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat7 cvmx_bgxx_cmrx_rx_stat7_t;

/**
 * cvmx_bgx#_cmr#_rx_stat8
 *
 * These registers provide a count of all packets received with some error that were not dropped
 * either due to the DMAC filter or lack of room in the receive FIFO.
 * This does not include packets which were counted in
 * BGX()_CMR()_RX_STAT2, BGX()_CMR()_RX_STAT4 nor
 * BGX()_CMR()_RX_STAT6.
 *
 * Which statistics are updated on control packet errors and drops are shown below:
 *
 * <pre>
 * if dropped [
 *   if !errored STAT8
 *   if overflow STAT6
 *   else if dmac drop STAT4
 *   else if filter drop STAT2
 * ] else [
 *   if errored STAT2
 *   else STAT8
 * ]
 * </pre>
 */
union cvmx_bgxx_cmrx_rx_stat8 {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_stat8_s {
		u64 reserved_48_63 : 16;
		u64 cnt : 48;
	} s;
	struct cvmx_bgxx_cmrx_rx_stat8_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_stat8_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_stat8_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_stat8_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_stat8 cvmx_bgxx_cmrx_rx_stat8_t;

/**
 * cvmx_bgx#_cmr#_rx_weight
 */
union cvmx_bgxx_cmrx_rx_weight {
	u64 u64;
	struct cvmx_bgxx_cmrx_rx_weight_s {
		u64 reserved_4_63 : 60;
		u64 weight : 4;
	} s;
	struct cvmx_bgxx_cmrx_rx_weight_s cn73xx;
	struct cvmx_bgxx_cmrx_rx_weight_s cn78xx;
	struct cvmx_bgxx_cmrx_rx_weight_s cn78xxp1;
	struct cvmx_bgxx_cmrx_rx_weight_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_rx_weight cvmx_bgxx_cmrx_rx_weight_t;

/**
 * cvmx_bgx#_cmr#_tx_channel
 */
union cvmx_bgxx_cmrx_tx_channel {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_channel_s {
		u64 reserved_32_63 : 32;
		u64 msk : 16;
		u64 dis : 16;
	} s;
	struct cvmx_bgxx_cmrx_tx_channel_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_channel_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_channel_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_channel_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_channel cvmx_bgxx_cmrx_tx_channel_t;

/**
 * cvmx_bgx#_cmr#_tx_fifo_len
 */
union cvmx_bgxx_cmrx_tx_fifo_len {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s {
		u64 reserved_14_63 : 50;
		u64 lmac_idle : 1;
		u64 fifo_len : 13;
	} s;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_fifo_len cvmx_bgxx_cmrx_tx_fifo_len_t;

/**
 * cvmx_bgx#_cmr#_tx_hg2_status
 */
union cvmx_bgxx_cmrx_tx_hg2_status {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s {
		u64 reserved_32_63 : 32;
		u64 xof : 16;
		u64 lgtim2go : 16;
	} s;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_hg2_status cvmx_bgxx_cmrx_tx_hg2_status_t;

/**
 * cvmx_bgx#_cmr#_tx_ovr_bp
 */
union cvmx_bgxx_cmrx_tx_ovr_bp {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s {
		u64 reserved_16_63 : 48;
		u64 tx_chan_bp : 16;
	} s;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_ovr_bp cvmx_bgxx_cmrx_tx_ovr_bp_t;

/**
 * cvmx_bgx#_cmr#_tx_stat0
 */
union cvmx_bgxx_cmrx_tx_stat0 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat0_s {
		u64 reserved_48_63 : 16;
		u64 xscol : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat0_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat0_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat0_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat0_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat0 cvmx_bgxx_cmrx_tx_stat0_t;

/**
 * cvmx_bgx#_cmr#_tx_stat1
 */
union cvmx_bgxx_cmrx_tx_stat1 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat1_s {
		u64 reserved_48_63 : 16;
		u64 xsdef : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat1_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat1_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat1_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat1_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat1 cvmx_bgxx_cmrx_tx_stat1_t;

/**
 * cvmx_bgx#_cmr#_tx_stat10
 */
union cvmx_bgxx_cmrx_tx_stat10 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat10_s {
		u64 reserved_48_63 : 16;
		u64 hist4 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat10_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat10_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat10_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat10_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat10 cvmx_bgxx_cmrx_tx_stat10_t;

/**
 * cvmx_bgx#_cmr#_tx_stat11
 */
union cvmx_bgxx_cmrx_tx_stat11 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat11_s {
		u64 reserved_48_63 : 16;
		u64 hist5 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat11_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat11_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat11_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat11_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat11 cvmx_bgxx_cmrx_tx_stat11_t;

/**
 * cvmx_bgx#_cmr#_tx_stat12
 */
union cvmx_bgxx_cmrx_tx_stat12 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat12_s {
		u64 reserved_48_63 : 16;
		u64 hist6 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat12_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat12_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat12_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat12_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat12 cvmx_bgxx_cmrx_tx_stat12_t;

/**
 * cvmx_bgx#_cmr#_tx_stat13
 */
union cvmx_bgxx_cmrx_tx_stat13 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat13_s {
		u64 reserved_48_63 : 16;
		u64 hist7 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat13_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat13_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat13_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat13_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat13 cvmx_bgxx_cmrx_tx_stat13_t;

/**
 * cvmx_bgx#_cmr#_tx_stat14
 */
union cvmx_bgxx_cmrx_tx_stat14 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat14_s {
		u64 reserved_48_63 : 16;
		u64 bcst : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat14_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat14_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat14_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat14_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat14 cvmx_bgxx_cmrx_tx_stat14_t;

/**
 * cvmx_bgx#_cmr#_tx_stat15
 */
union cvmx_bgxx_cmrx_tx_stat15 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat15_s {
		u64 reserved_48_63 : 16;
		u64 mcst : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat15_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat15_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat15_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat15_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat15 cvmx_bgxx_cmrx_tx_stat15_t;

/**
 * cvmx_bgx#_cmr#_tx_stat16
 */
union cvmx_bgxx_cmrx_tx_stat16 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat16_s {
		u64 reserved_48_63 : 16;
		u64 undflw : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat16_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat16_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat16_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat16_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat16 cvmx_bgxx_cmrx_tx_stat16_t;

/**
 * cvmx_bgx#_cmr#_tx_stat17
 */
union cvmx_bgxx_cmrx_tx_stat17 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat17_s {
		u64 reserved_48_63 : 16;
		u64 ctl : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat17_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat17_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat17_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat17_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat17 cvmx_bgxx_cmrx_tx_stat17_t;

/**
 * cvmx_bgx#_cmr#_tx_stat2
 */
union cvmx_bgxx_cmrx_tx_stat2 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat2_s {
		u64 reserved_48_63 : 16;
		u64 mcol : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat2_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat2_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat2_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat2_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat2 cvmx_bgxx_cmrx_tx_stat2_t;

/**
 * cvmx_bgx#_cmr#_tx_stat3
 */
union cvmx_bgxx_cmrx_tx_stat3 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat3_s {
		u64 reserved_48_63 : 16;
		u64 scol : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat3_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat3_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat3_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat3_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat3 cvmx_bgxx_cmrx_tx_stat3_t;

/**
 * cvmx_bgx#_cmr#_tx_stat4
 */
union cvmx_bgxx_cmrx_tx_stat4 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat4_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat4_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat4_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat4_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat4_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat4 cvmx_bgxx_cmrx_tx_stat4_t;

/**
 * cvmx_bgx#_cmr#_tx_stat5
 */
union cvmx_bgxx_cmrx_tx_stat5 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat5_s {
		u64 reserved_48_63 : 16;
		u64 pkts : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat5_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat5_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat5_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat5_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat5 cvmx_bgxx_cmrx_tx_stat5_t;

/**
 * cvmx_bgx#_cmr#_tx_stat6
 */
union cvmx_bgxx_cmrx_tx_stat6 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat6_s {
		u64 reserved_48_63 : 16;
		u64 hist0 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat6_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat6_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat6_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat6_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat6 cvmx_bgxx_cmrx_tx_stat6_t;

/**
 * cvmx_bgx#_cmr#_tx_stat7
 */
union cvmx_bgxx_cmrx_tx_stat7 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat7_s {
		u64 reserved_48_63 : 16;
		u64 hist1 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat7_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat7_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat7_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat7_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat7 cvmx_bgxx_cmrx_tx_stat7_t;

/**
 * cvmx_bgx#_cmr#_tx_stat8
 */
union cvmx_bgxx_cmrx_tx_stat8 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat8_s {
		u64 reserved_48_63 : 16;
		u64 hist2 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat8_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat8_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat8_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat8_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat8 cvmx_bgxx_cmrx_tx_stat8_t;

/**
 * cvmx_bgx#_cmr#_tx_stat9
 */
union cvmx_bgxx_cmrx_tx_stat9 {
	u64 u64;
	struct cvmx_bgxx_cmrx_tx_stat9_s {
		u64 reserved_48_63 : 16;
		u64 hist3 : 48;
	} s;
	struct cvmx_bgxx_cmrx_tx_stat9_s cn73xx;
	struct cvmx_bgxx_cmrx_tx_stat9_s cn78xx;
	struct cvmx_bgxx_cmrx_tx_stat9_s cn78xxp1;
	struct cvmx_bgxx_cmrx_tx_stat9_s cnf75xx;
};

typedef union cvmx_bgxx_cmrx_tx_stat9 cvmx_bgxx_cmrx_tx_stat9_t;

/**
 * cvmx_bgx#_cmr_bad
 */
union cvmx_bgxx_cmr_bad {
	u64 u64;
	struct cvmx_bgxx_cmr_bad_s {
		u64 reserved_1_63 : 63;
		u64 rxb_nxl : 1;
	} s;
	struct cvmx_bgxx_cmr_bad_s cn73xx;
	struct cvmx_bgxx_cmr_bad_s cn78xx;
	struct cvmx_bgxx_cmr_bad_s cn78xxp1;
	struct cvmx_bgxx_cmr_bad_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_bad cvmx_bgxx_cmr_bad_t;

/**
 * cvmx_bgx#_cmr_bist_status
 */
union cvmx_bgxx_cmr_bist_status {
	u64 u64;
	struct cvmx_bgxx_cmr_bist_status_s {
		u64 reserved_25_63 : 39;
		u64 status : 25;
	} s;
	struct cvmx_bgxx_cmr_bist_status_s cn73xx;
	struct cvmx_bgxx_cmr_bist_status_s cn78xx;
	struct cvmx_bgxx_cmr_bist_status_s cn78xxp1;
	struct cvmx_bgxx_cmr_bist_status_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_bist_status cvmx_bgxx_cmr_bist_status_t;

/**
 * cvmx_bgx#_cmr_chan_msk_and
 */
union cvmx_bgxx_cmr_chan_msk_and {
	u64 u64;
	struct cvmx_bgxx_cmr_chan_msk_and_s {
		u64 msk_and : 64;
	} s;
	struct cvmx_bgxx_cmr_chan_msk_and_s cn73xx;
	struct cvmx_bgxx_cmr_chan_msk_and_s cn78xx;
	struct cvmx_bgxx_cmr_chan_msk_and_s cn78xxp1;
	struct cvmx_bgxx_cmr_chan_msk_and_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_chan_msk_and cvmx_bgxx_cmr_chan_msk_and_t;

/**
 * cvmx_bgx#_cmr_chan_msk_or
 */
union cvmx_bgxx_cmr_chan_msk_or {
	u64 u64;
	struct cvmx_bgxx_cmr_chan_msk_or_s {
		u64 msk_or : 64;
	} s;
	struct cvmx_bgxx_cmr_chan_msk_or_s cn73xx;
	struct cvmx_bgxx_cmr_chan_msk_or_s cn78xx;
	struct cvmx_bgxx_cmr_chan_msk_or_s cn78xxp1;
	struct cvmx_bgxx_cmr_chan_msk_or_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_chan_msk_or cvmx_bgxx_cmr_chan_msk_or_t;

/**
 * cvmx_bgx#_cmr_eco
 */
union cvmx_bgxx_cmr_eco {
	u64 u64;
	struct cvmx_bgxx_cmr_eco_s {
		u64 eco_ro : 32;
		u64 eco_rw : 32;
	} s;
	struct cvmx_bgxx_cmr_eco_s cn73xx;
	struct cvmx_bgxx_cmr_eco_s cn78xx;
	struct cvmx_bgxx_cmr_eco_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_eco cvmx_bgxx_cmr_eco_t;

/**
 * cvmx_bgx#_cmr_global_config
 *
 * These registers configure the global CMR, PCS, and MAC.
 *
 */
union cvmx_bgxx_cmr_global_config {
	u64 u64;
	struct cvmx_bgxx_cmr_global_config_s {
		u64 reserved_5_63 : 59;
		u64 cmr_mix1_reset : 1;
		u64 cmr_mix0_reset : 1;
		u64 cmr_x2p_reset : 1;
		u64 bgx_clk_enable : 1;
		u64 pmux_sds_sel : 1;
	} s;
	struct cvmx_bgxx_cmr_global_config_s cn73xx;
	struct cvmx_bgxx_cmr_global_config_s cn78xx;
	struct cvmx_bgxx_cmr_global_config_s cn78xxp1;
	struct cvmx_bgxx_cmr_global_config_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_global_config cvmx_bgxx_cmr_global_config_t;

/**
 * cvmx_bgx#_cmr_mem_ctrl
 */
union cvmx_bgxx_cmr_mem_ctrl {
	u64 u64;
	struct cvmx_bgxx_cmr_mem_ctrl_s {
		u64 reserved_24_63 : 40;
		u64 txb_skid_synd : 2;
		u64 txb_skid_cor_dis : 1;
		u64 txb_fif_bk1_syn : 2;
		u64 txb_fif_bk1_cdis : 1;
		u64 txb_fif_bk0_syn : 2;
		u64 txb_fif_bk0_cdis : 1;
		u64 rxb_skid_synd : 2;
		u64 rxb_skid_cor_dis : 1;
		u64 rxb_fif_bk1_syn1 : 2;
		u64 rxb_fif_bk1_cdis1 : 1;
		u64 rxb_fif_bk1_syn0 : 2;
		u64 rxb_fif_bk1_cdis0 : 1;
		u64 rxb_fif_bk0_syn1 : 2;
		u64 rxb_fif_bk0_cdis1 : 1;
		u64 rxb_fif_bk0_syn0 : 2;
		u64 rxb_fif_bk0_cdis0 : 1;
	} s;
	struct cvmx_bgxx_cmr_mem_ctrl_s cn73xx;
	struct cvmx_bgxx_cmr_mem_ctrl_s cn78xx;
	struct cvmx_bgxx_cmr_mem_ctrl_s cn78xxp1;
	struct cvmx_bgxx_cmr_mem_ctrl_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_mem_ctrl cvmx_bgxx_cmr_mem_ctrl_t;

/**
 * cvmx_bgx#_cmr_mem_int
 */
union cvmx_bgxx_cmr_mem_int {
	u64 u64;
	struct cvmx_bgxx_cmr_mem_int_s {
		u64 reserved_18_63 : 46;
		u64 smu_in_overfl : 1;
		u64 gmp_in_overfl : 1;
		u64 txb_skid_sbe : 1;
		u64 txb_skid_dbe : 1;
		u64 txb_fif_bk1_sbe : 1;
		u64 txb_fif_bk1_dbe : 1;
		u64 txb_fif_bk0_sbe : 1;
		u64 txb_fif_bk0_dbe : 1;
		u64 rxb_skid_sbe : 1;
		u64 rxb_skid_dbe : 1;
		u64 rxb_fif_bk1_sbe1 : 1;
		u64 rxb_fif_bk1_dbe1 : 1;
		u64 rxb_fif_bk1_sbe0 : 1;
		u64 rxb_fif_bk1_dbe0 : 1;
		u64 rxb_fif_bk0_sbe1 : 1;
		u64 rxb_fif_bk0_dbe1 : 1;
		u64 rxb_fif_bk0_sbe0 : 1;
		u64 rxb_fif_bk0_dbe0 : 1;
	} s;
	struct cvmx_bgxx_cmr_mem_int_s cn73xx;
	struct cvmx_bgxx_cmr_mem_int_s cn78xx;
	struct cvmx_bgxx_cmr_mem_int_s cn78xxp1;
	struct cvmx_bgxx_cmr_mem_int_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_mem_int cvmx_bgxx_cmr_mem_int_t;

/**
 * cvmx_bgx#_cmr_nxc_adr
 */
union cvmx_bgxx_cmr_nxc_adr {
	u64 u64;
	struct cvmx_bgxx_cmr_nxc_adr_s {
		u64 reserved_16_63 : 48;
		u64 lmac_id : 4;
		u64 channel : 12;
	} s;
	struct cvmx_bgxx_cmr_nxc_adr_s cn73xx;
	struct cvmx_bgxx_cmr_nxc_adr_s cn78xx;
	struct cvmx_bgxx_cmr_nxc_adr_s cn78xxp1;
	struct cvmx_bgxx_cmr_nxc_adr_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_nxc_adr cvmx_bgxx_cmr_nxc_adr_t;

/**
 * cvmx_bgx#_cmr_rx_adr#_cam
 *
 * These registers provide access to the 32 DMAC CAM entries in BGX.
 *
 */
union cvmx_bgxx_cmr_rx_adrx_cam {
	u64 u64;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s {
		u64 reserved_54_63 : 10;
		u64 id : 2;
		u64 reserved_49_51 : 3;
		u64 en : 1;
		u64 adr : 48;
	} s;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s cn73xx;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s cn78xx;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s cn78xxp1;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_rx_adrx_cam cvmx_bgxx_cmr_rx_adrx_cam_t;

/**
 * cvmx_bgx#_cmr_rx_lmacs
 */
union cvmx_bgxx_cmr_rx_lmacs {
	u64 u64;
	struct cvmx_bgxx_cmr_rx_lmacs_s {
		u64 reserved_3_63 : 61;
		u64 lmacs : 3;
	} s;
	struct cvmx_bgxx_cmr_rx_lmacs_s cn73xx;
	struct cvmx_bgxx_cmr_rx_lmacs_s cn78xx;
	struct cvmx_bgxx_cmr_rx_lmacs_s cn78xxp1;
	struct cvmx_bgxx_cmr_rx_lmacs_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_rx_lmacs cvmx_bgxx_cmr_rx_lmacs_t;

/**
 * cvmx_bgx#_cmr_rx_ovr_bp
 *
 * BGX()_CMR_RX_OVR_BP[EN<0>] must be set to one and BGX()_CMR_RX_OVR_BP[BP<0>] must be
 * cleared to zero (to forcibly disable hardware-automatic 802.3 PAUSE packet generation) with
 * the HiGig2 Protocol when BGX()_SMU()_HG2_CONTROL[HG2TX_EN]=0. (The HiGig2 protocol is
 * indicated by BGX()_SMU()_TX_CTL[HG_EN]=1 and BGX()_SMU()_RX_UDD_SKP[LEN]=16).
 * Hardware can only auto-generate backpressure through HiGig2 messages (optionally, when
 * BGX()_SMU()_HG2_CONTROL[HG2TX_EN]=1) with the HiGig2 protocol.
 */
union cvmx_bgxx_cmr_rx_ovr_bp {
	u64 u64;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s {
		u64 reserved_12_63 : 52;
		u64 en : 4;
		u64 bp : 4;
		u64 ign_fifo_bp : 4;
	} s;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s cn73xx;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s cn78xx;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s cn78xxp1;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_rx_ovr_bp cvmx_bgxx_cmr_rx_ovr_bp_t;

/**
 * cvmx_bgx#_cmr_tx_lmacs
 *
 * This register sets the number of LMACs allowed on the TX interface. The value is important for
 * defining the partitioning of the transmit FIFO.
 */
union cvmx_bgxx_cmr_tx_lmacs {
	u64 u64;
	struct cvmx_bgxx_cmr_tx_lmacs_s {
		u64 reserved_3_63 : 61;
		u64 lmacs : 3;
	} s;
	struct cvmx_bgxx_cmr_tx_lmacs_s cn73xx;
	struct cvmx_bgxx_cmr_tx_lmacs_s cn78xx;
	struct cvmx_bgxx_cmr_tx_lmacs_s cn78xxp1;
	struct cvmx_bgxx_cmr_tx_lmacs_s cnf75xx;
};

typedef union cvmx_bgxx_cmr_tx_lmacs cvmx_bgxx_cmr_tx_lmacs_t;

/**
 * cvmx_bgx#_gmp_gmi_prt#_cfg
 *
 * This register controls the configuration of the LMAC.
 *
 */
union cvmx_bgxx_gmp_gmi_prtx_cfg {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s {
		u64 reserved_14_63 : 50;
		u64 tx_idle : 1;
		u64 rx_idle : 1;
		u64 reserved_9_11 : 3;
		u64 speed_msb : 1;
		u64 reserved_4_7 : 4;
		u64 slottime : 1;
		u64 duplex : 1;
		u64 speed : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_prtx_cfg cvmx_bgxx_gmp_gmi_prtx_cfg_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_decision
 *
 * This register specifies the byte count used to determine when to accept or to filter a packet.
 * As each byte in a packet is received by GMI, the L2 byte count is compared against
 * [CNT]. In normal operation, the L2 header begins after the
 * PREAMBLE + SFD (BGX()_GMP_GMI_RX()_FRM_CTL[PRE_CHK] = 1) and any optional UDD skip
 * data (BGX()_GMP_GMI_RX()_UDD_SKP[LEN]).
 */
union cvmx_bgxx_gmp_gmi_rxx_decision {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_decision cvmx_bgxx_gmp_gmi_rxx_decision_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_frm_chk
 */
union cvmx_bgxx_gmp_gmi_rxx_frm_chk {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s {
		u64 reserved_9_63 : 55;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_5_6 : 2;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 reserved_2_2 : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_frm_chk cvmx_bgxx_gmp_gmi_rxx_frm_chk_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_frm_ctl
 *
 * This register controls the handling of the frames.
 * The [CTL_BCK] and [CTL_DRP] bits control how the hardware handles incoming PAUSE packets. The
 * most
 * common modes of operation:
 * _ [CTL_BCK] = 1, [CTL_DRP] = 1: hardware handles everything.
 * _ [CTL_BCK] = 0, [CTL_DRP] = 0: software sees all PAUSE frames.
 * _ [CTL_BCK] = 0, [CTL_DRP] = 1: all PAUSE frames are completely ignored.
 *
 * These control bits should be set to [CTL_BCK] = 0, [CTL_DRP] = 0 in half-duplex mode. Since
 * PAUSE
 * packets only apply to full duplex operation, any PAUSE packet would constitute an exception
 * which should be handled by the processing cores. PAUSE packets should not be forwarded.
 */
union cvmx_bgxx_gmp_gmi_rxx_frm_ctl {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s {
		u64 reserved_13_63 : 51;
		u64 ptp_mode : 1;
		u64 reserved_11_11 : 1;
		u64 null_dis : 1;
		u64 pre_align : 1;
		u64 reserved_7_8 : 2;
		u64 pre_free : 1;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_frm_ctl cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_ifg
 *
 * This register specifies the minimum number of interframe-gap (IFG) cycles between packets.
 *
 */
union cvmx_bgxx_gmp_gmi_rxx_ifg {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s {
		u64 reserved_4_63 : 60;
		u64 ifg : 4;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_ifg cvmx_bgxx_gmp_gmi_rxx_ifg_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_int
 *
 * '"These registers allow interrupts to be sent to the control processor.
 * * Exception conditions <10:0> can also set the rcv/opcode in the received packet's work-queue
 * entry. BGX()_GMP_GMI_RX()_FRM_CHK provides a bit mask for configuring which conditions
 * set the error.
 * In half duplex operation, the expectation is that collisions will appear as either MINERR or
 * CAREXT errors.'
 */
union cvmx_bgxx_gmp_gmi_rxx_int {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s {
		u64 reserved_12_63 : 52;
		u64 ifgerr : 1;
		u64 coldet : 1;
		u64 falerr : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 ovrerr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
		u64 carext : 1;
		u64 minerr : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_int cvmx_bgxx_gmp_gmi_rxx_int_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_jabber
 *
 * This register specifies the maximum size for packets, beyond which the GMI truncates.
 *
 */
union cvmx_bgxx_gmp_gmi_rxx_jabber {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_jabber cvmx_bgxx_gmp_gmi_rxx_jabber_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_udd_skp
 *
 * This register specifies the amount of user-defined data (UDD) added before the start of the
 * L2C data.
 */
union cvmx_bgxx_gmp_gmi_rxx_udd_skp {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s {
		u64 reserved_9_63 : 55;
		u64 fcssel : 1;
		u64 reserved_7_7 : 1;
		u64 len : 7;
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_rxx_udd_skp cvmx_bgxx_gmp_gmi_rxx_udd_skp_t;

/**
 * cvmx_bgx#_gmp_gmi_smac#
 */
union cvmx_bgxx_gmp_gmi_smacx {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_smacx_s {
		u64 reserved_48_63 : 16;
		u64 smac : 48;
	} s;
	struct cvmx_bgxx_gmp_gmi_smacx_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_smacx_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_smacx_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_smacx_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_smacx cvmx_bgxx_gmp_gmi_smacx_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_append
 */
union cvmx_bgxx_gmp_gmi_txx_append {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_append_s {
		u64 reserved_4_63 : 60;
		u64 force_fcs : 1;
		u64 fcs : 1;
		u64 pad : 1;
		u64 preamble : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_append_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_append_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_append_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_append_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_append cvmx_bgxx_gmp_gmi_txx_append_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_burst
 */
union cvmx_bgxx_gmp_gmi_txx_burst {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s {
		u64 reserved_16_63 : 48;
		u64 burst : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_burst cvmx_bgxx_gmp_gmi_txx_burst_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_ctl
 */
union cvmx_bgxx_gmp_gmi_txx_ctl {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s {
		u64 reserved_2_63 : 62;
		u64 xsdef_en : 1;
		u64 xscol_en : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_ctl cvmx_bgxx_gmp_gmi_txx_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_int
 */
union cvmx_bgxx_gmp_gmi_txx_int {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_int_s {
		u64 reserved_5_63 : 59;
		u64 ptp_lost : 1;
		u64 late_col : 1;
		u64 xsdef : 1;
		u64 xscol : 1;
		u64 undflw : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_int_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_int_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_int_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_int_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_int cvmx_bgxx_gmp_gmi_txx_int_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_min_pkt
 */
union cvmx_bgxx_gmp_gmi_txx_min_pkt {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s {
		u64 reserved_8_63 : 56;
		u64 min_size : 8;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_min_pkt cvmx_bgxx_gmp_gmi_txx_min_pkt_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_pkt_interval
 *
 * This register specifies how often PAUSE packets are sent.
 *
 */
union cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s {
		u64 reserved_16_63 : 48;
		u64 interval : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_pkt_time
 */
union cvmx_bgxx_gmp_gmi_txx_pause_pkt_time {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s {
		u64 reserved_16_63 : 48;
		u64 ptime : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_pause_pkt_time cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_togo
 */
union cvmx_bgxx_gmp_gmi_txx_pause_togo {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s {
		u64 reserved_16_63 : 48;
		u64 ptime : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_pause_togo cvmx_bgxx_gmp_gmi_txx_pause_togo_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_zero
 */
union cvmx_bgxx_gmp_gmi_txx_pause_zero {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s {
		u64 reserved_1_63 : 63;
		u64 send : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_pause_zero cvmx_bgxx_gmp_gmi_txx_pause_zero_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_sgmii_ctl
 */
union cvmx_bgxx_gmp_gmi_txx_sgmii_ctl {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s {
		u64 reserved_1_63 : 63;
		u64 align : 1;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_sgmii_ctl cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_slot
 */
union cvmx_bgxx_gmp_gmi_txx_slot {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s {
		u64 reserved_10_63 : 54;
		u64 slot : 10;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_slot cvmx_bgxx_gmp_gmi_txx_slot_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_soft_pause
 */
union cvmx_bgxx_gmp_gmi_txx_soft_pause {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s {
		u64 reserved_16_63 : 48;
		u64 ptime : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_soft_pause cvmx_bgxx_gmp_gmi_txx_soft_pause_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_thresh
 */
union cvmx_bgxx_gmp_gmi_txx_thresh {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s {
		u64 reserved_11_63 : 53;
		u64 cnt : 11;
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_txx_thresh cvmx_bgxx_gmp_gmi_txx_thresh_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_col_attempt
 */
union cvmx_bgxx_gmp_gmi_tx_col_attempt {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s {
		u64 reserved_5_63 : 59;
		u64 limit : 5;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_col_attempt cvmx_bgxx_gmp_gmi_tx_col_attempt_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_ifg
 *
 * Consider the following when programming IFG1 and IFG2:
 * * For 10/100/1000 Mb/s half-duplex systems that require IEEE 802.3 compatibility, IFG1 must be
 * in the range of 1-8, IFG2 must be in the range of 4-12, and the IFG1 + IFG2 sum must be 12.
 * * For 10/100/1000 Mb/s full-duplex systems that require IEEE 802.3 compatibility, IFG1 must be
 * in the range of 1-11, IFG2 must be in the range of 1-11, and the IFG1 + IFG2 sum must be 12.
 * For all other systems, IFG1 and IFG2 can be any value in the range of 1-15, allowing for a
 * total possible IFG sum of 2-30.
 */
union cvmx_bgxx_gmp_gmi_tx_ifg {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s {
		u64 reserved_8_63 : 56;
		u64 ifg2 : 4;
		u64 ifg1 : 4;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_ifg cvmx_bgxx_gmp_gmi_tx_ifg_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_jam
 *
 * This register provides the pattern used in JAM bytes.
 *
 */
union cvmx_bgxx_gmp_gmi_tx_jam {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s {
		u64 reserved_8_63 : 56;
		u64 jam : 8;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_jam cvmx_bgxx_gmp_gmi_tx_jam_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_lfsr
 *
 * This register shows the contents of the linear feedback shift register (LFSR), which is used
 * to implement truncated binary exponential backoff.
 */
union cvmx_bgxx_gmp_gmi_tx_lfsr {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s {
		u64 reserved_16_63 : 48;
		u64 lfsr : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_lfsr cvmx_bgxx_gmp_gmi_tx_lfsr_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_pause_pkt_dmac
 */
union cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s {
		u64 reserved_48_63 : 16;
		u64 dmac : 48;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_pause_pkt_type
 *
 * This register provides the PTYPE field that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_gmp_gmi_tx_pause_pkt_type {
	u64 u64;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s {
		u64 reserved_16_63 : 48;
		u64 ptype : 16;
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s cn73xx;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s cn78xx;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s cn78xxp1;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_gmi_tx_pause_pkt_type cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_adv
 */
union cvmx_bgxx_gmp_pcs_anx_adv {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s {
		u64 reserved_16_63 : 48;
		u64 np : 1;
		u64 reserved_14_14 : 1;
		u64 rem_flt : 2;
		u64 reserved_9_11 : 3;
		u64 pause : 2;
		u64 hfd : 1;
		u64 fd : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_anx_adv cvmx_bgxx_gmp_pcs_anx_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_ext_st
 */
union cvmx_bgxx_gmp_pcs_anx_ext_st {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s {
		u64 reserved_16_63 : 48;
		u64 thou_xfd : 1;
		u64 thou_xhd : 1;
		u64 thou_tfd : 1;
		u64 thou_thd : 1;
		u64 reserved_0_11 : 12;
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_anx_ext_st cvmx_bgxx_gmp_pcs_anx_ext_st_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_lp_abil
 *
 * This is the autonegotiation link partner ability register 5 as per IEEE 802.3, Clause 37.
 *
 */
union cvmx_bgxx_gmp_pcs_anx_lp_abil {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s {
		u64 reserved_16_63 : 48;
		u64 np : 1;
		u64 ack : 1;
		u64 rem_flt : 2;
		u64 reserved_9_11 : 3;
		u64 pause : 2;
		u64 hfd : 1;
		u64 fd : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_anx_lp_abil cvmx_bgxx_gmp_pcs_anx_lp_abil_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_results
 *
 * This register is not valid when BGX()_GMP_PCS_MISC()_CTL[AN_OVRD] is set to 1. If
 * BGX()_GMP_PCS_MISC()_CTL[AN_OVRD] is set to 0 and
 * BGX()_GMP_PCS_AN()_RESULTS[AN_CPT] is set to 1, this register is valid.
 */
union cvmx_bgxx_gmp_pcs_anx_results {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_anx_results_s {
		u64 reserved_7_63 : 57;
		u64 pause : 2;
		u64 spd : 2;
		u64 an_cpt : 1;
		u64 dup : 1;
		u64 link_ok : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_results_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_anx_results_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_anx_results_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_anx_results_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_anx_results cvmx_bgxx_gmp_pcs_anx_results_t;

/**
 * cvmx_bgx#_gmp_pcs_int#
 */
union cvmx_bgxx_gmp_pcs_intx {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_intx_s {
		u64 reserved_13_63 : 51;
		u64 dbg_sync : 1;
		u64 dup : 1;
		u64 sync_bad : 1;
		u64 an_bad : 1;
		u64 rxlock : 1;
		u64 rxbad : 1;
		u64 rxerr : 1;
		u64 txbad : 1;
		u64 txfifo : 1;
		u64 txfifu : 1;
		u64 an_err : 1;
		u64 xmit : 1;
		u64 lnkspd : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_intx_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_intx_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_intx_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_intx_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_intx cvmx_bgxx_gmp_pcs_intx_t;

/**
 * cvmx_bgx#_gmp_pcs_link#_timer
 *
 * This is the 1.6 ms nominal link timer register.
 *
 */
union cvmx_bgxx_gmp_pcs_linkx_timer {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s {
		u64 reserved_16_63 : 48;
		u64 count : 16;
	} s;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_linkx_timer cvmx_bgxx_gmp_pcs_linkx_timer_t;

/**
 * cvmx_bgx#_gmp_pcs_misc#_ctl
 */
union cvmx_bgxx_gmp_pcs_miscx_ctl {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s {
		u64 reserved_13_63 : 51;
		u64 sgmii : 1;
		u64 gmxeno : 1;
		u64 loopbck2 : 1;
		u64 mac_phy : 1;
		u64 mode : 1;
		u64 an_ovrd : 1;
		u64 samp_pt : 7;
	} s;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_miscx_ctl cvmx_bgxx_gmp_pcs_miscx_ctl_t;

/**
 * cvmx_bgx#_gmp_pcs_mr#_control
 */
union cvmx_bgxx_gmp_pcs_mrx_control {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s {
		u64 reserved_16_63 : 48;
		u64 reset : 1;
		u64 loopbck1 : 1;
		u64 spdlsb : 1;
		u64 an_en : 1;
		u64 pwr_dn : 1;
		u64 reserved_10_10 : 1;
		u64 rst_an : 1;
		u64 dup : 1;
		u64 coltst : 1;
		u64 spdmsb : 1;
		u64 uni : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_mrx_control cvmx_bgxx_gmp_pcs_mrx_control_t;

/**
 * cvmx_bgx#_gmp_pcs_mr#_status
 *
 * Bits <15:9> in this register indicate the ability to operate when
 * BGX()_GMP_PCS_MISC()_CTL[MAC_PHY] is set to MAC mode. Bits <15:9> are always read as
 * 0, indicating that the chip cannot operate in the corresponding modes. The field [RM_FLT] is a
 * 'don't care' when the selected mode is SGMII.
 */
union cvmx_bgxx_gmp_pcs_mrx_status {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s {
		u64 reserved_16_63 : 48;
		u64 hun_t4 : 1;
		u64 hun_xfd : 1;
		u64 hun_xhd : 1;
		u64 ten_fd : 1;
		u64 ten_hd : 1;
		u64 hun_t2fd : 1;
		u64 hun_t2hd : 1;
		u64 ext_st : 1;
		u64 reserved_7_7 : 1;
		u64 prb_sup : 1;
		u64 an_cpt : 1;
		u64 rm_flt : 1;
		u64 an_abil : 1;
		u64 lnk_st : 1;
		u64 reserved_1_1 : 1;
		u64 extnd : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_mrx_status cvmx_bgxx_gmp_pcs_mrx_status_t;

/**
 * cvmx_bgx#_gmp_pcs_rx#_states
 */
union cvmx_bgxx_gmp_pcs_rxx_states {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s {
		u64 reserved_16_63 : 48;
		u64 rx_bad : 1;
		u64 rx_st : 5;
		u64 sync_bad : 1;
		u64 sync : 4;
		u64 an_bad : 1;
		u64 an_st : 4;
	} s;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_rxx_states cvmx_bgxx_gmp_pcs_rxx_states_t;

/**
 * cvmx_bgx#_gmp_pcs_rx#_sync
 */
union cvmx_bgxx_gmp_pcs_rxx_sync {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s {
		u64 reserved_2_63 : 62;
		u64 sync : 1;
		u64 bit_lock : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_rxx_sync cvmx_bgxx_gmp_pcs_rxx_sync_t;

/**
 * cvmx_bgx#_gmp_pcs_sgm#_an_adv
 *
 * This is the SGMII autonegotiation advertisement register (sent out as tx_Config_Reg<15:0> as
 * defined in IEEE 802.3 clause 37). This register is sent during autonegotiation if
 * BGX()_GMP_PCS_MISC()_CTL[MAC_PHY] is set (1 = PHY mode). If the bit is not set (0 =
 * MAC mode), then tx_Config_Reg<14> becomes ACK bit and tx_Config_Reg<0> is always 1. All other
 * bits in tx_Config_Reg sent will be 0. The PHY dictates the autonegotiation results.
 */
union cvmx_bgxx_gmp_pcs_sgmx_an_adv {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s {
		u64 reserved_16_63 : 48;
		u64 link : 1;
		u64 ack : 1;
		u64 reserved_13_13 : 1;
		u64 dup : 1;
		u64 speed : 2;
		u64 reserved_1_9 : 9;
		u64 one : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_sgmx_an_adv cvmx_bgxx_gmp_pcs_sgmx_an_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_sgm#_lp_adv
 *
 * This is the SGMII link partner advertisement register (received as rx_Config_Reg<15:0> as
 * defined in IEEE 802.3 clause 37).
 */
union cvmx_bgxx_gmp_pcs_sgmx_lp_adv {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s {
		u64 reserved_16_63 : 48;
		u64 link : 1;
		u64 reserved_13_14 : 2;
		u64 dup : 1;
		u64 speed : 2;
		u64 reserved_1_9 : 9;
		u64 one : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_sgmx_lp_adv cvmx_bgxx_gmp_pcs_sgmx_lp_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_tx#_states
 */
union cvmx_bgxx_gmp_pcs_txx_states {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_txx_states_s {
		u64 reserved_7_63 : 57;
		u64 xmit : 2;
		u64 tx_bad : 1;
		u64 ord_st : 4;
	} s;
	struct cvmx_bgxx_gmp_pcs_txx_states_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_txx_states_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_txx_states_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_txx_states_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_txx_states cvmx_bgxx_gmp_pcs_txx_states_t;

/**
 * cvmx_bgx#_gmp_pcs_tx_rx#_polarity
 *
 * BGX()_GMP_PCS_TX_RX()_POLARITY[AUTORXPL] shows correct polarity needed on the link
 * receive path after code group synchronization is achieved.
 */
union cvmx_bgxx_gmp_pcs_tx_rxx_polarity {
	u64 u64;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s {
		u64 reserved_4_63 : 60;
		u64 rxovrd : 1;
		u64 autorxpl : 1;
		u64 rxplrt : 1;
		u64 txplrt : 1;
	} s;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s cn73xx;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s cn78xx;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s cn78xxp1;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s cnf75xx;
};

typedef union cvmx_bgxx_gmp_pcs_tx_rxx_polarity cvmx_bgxx_gmp_pcs_tx_rxx_polarity_t;

/**
 * cvmx_bgx#_smu#_cbfc_ctl
 */
union cvmx_bgxx_smux_cbfc_ctl {
	u64 u64;
	struct cvmx_bgxx_smux_cbfc_ctl_s {
		u64 phys_en : 16;
		u64 logl_en : 16;
		u64 reserved_4_31 : 28;
		u64 bck_en : 1;
		u64 drp_en : 1;
		u64 tx_en : 1;
		u64 rx_en : 1;
	} s;
	struct cvmx_bgxx_smux_cbfc_ctl_s cn73xx;
	struct cvmx_bgxx_smux_cbfc_ctl_s cn78xx;
	struct cvmx_bgxx_smux_cbfc_ctl_s cn78xxp1;
	struct cvmx_bgxx_smux_cbfc_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_smux_cbfc_ctl cvmx_bgxx_smux_cbfc_ctl_t;

/**
 * cvmx_bgx#_smu#_ctrl
 */
union cvmx_bgxx_smux_ctrl {
	u64 u64;
	struct cvmx_bgxx_smux_ctrl_s {
		u64 reserved_2_63 : 62;
		u64 tx_idle : 1;
		u64 rx_idle : 1;
	} s;
	struct cvmx_bgxx_smux_ctrl_s cn73xx;
	struct cvmx_bgxx_smux_ctrl_s cn78xx;
	struct cvmx_bgxx_smux_ctrl_s cn78xxp1;
	struct cvmx_bgxx_smux_ctrl_s cnf75xx;
};

typedef union cvmx_bgxx_smux_ctrl cvmx_bgxx_smux_ctrl_t;

/**
 * cvmx_bgx#_smu#_ext_loopback
 *
 * In loopback mode, the IFG1+IFG2 of local and remote parties must match exactly; otherwise one
 * of the two sides' loopback FIFO will overrun: BGX()_SMU()_TX_INT[LB_OVRFLW].
 */
union cvmx_bgxx_smux_ext_loopback {
	u64 u64;
	struct cvmx_bgxx_smux_ext_loopback_s {
		u64 reserved_5_63 : 59;
		u64 en : 1;
		u64 thresh : 4;
	} s;
	struct cvmx_bgxx_smux_ext_loopback_s cn73xx;
	struct cvmx_bgxx_smux_ext_loopback_s cn78xx;
	struct cvmx_bgxx_smux_ext_loopback_s cn78xxp1;
	struct cvmx_bgxx_smux_ext_loopback_s cnf75xx;
};

typedef union cvmx_bgxx_smux_ext_loopback cvmx_bgxx_smux_ext_loopback_t;

/**
 * cvmx_bgx#_smu#_hg2_control
 *
 * HiGig2 TX- and RX-enable are normally set together for HiGig2 messaging. Setting just the TX
 * or RX bit results in only the HG2 message transmit or receive capability.
 *
 * Setting [PHYS_EN] and [LOGL_EN] to 1 allows link PAUSE or backpressure to PKO as per the
 * received HiGig2 message. Setting these fields to 0 disables link PAUSE and backpressure to PKO
 * in response to received messages.
 *
 * BGX()_SMU()_TX_CTL[HG_EN] must be set (to enable HiGig) whenever either [HG2TX_EN] or
 * [HG2RX_EN] are set. BGX()_SMU()_RX_UDD_SKP[LEN] must be set to 16 (to select HiGig2)
 * whenever either [HG2TX_EN] or [HG2RX_EN] are set.
 *
 * BGX()_CMR_RX_OVR_BP[EN]<0> must be set and BGX()_CMR_RX_OVR_BP[BP]<0> must be cleared
 * to 0 (to forcibly disable hardware-automatic 802.3 PAUSE packet generation) with the HiGig2
 * Protocol when [HG2TX_EN] = 0. (The HiGig2 protocol is indicated
 * by BGX()_SMU()_TX_CTL[HG_EN] = 1 and BGX()_SMU()_RX_UDD_SKP[LEN]=16.) Hardware
 * can only autogenerate backpressure via HiGig2 messages (optionally, when [HG2TX_EN] = 1) with
 * the HiGig2 protocol.
 */
union cvmx_bgxx_smux_hg2_control {
	u64 u64;
	struct cvmx_bgxx_smux_hg2_control_s {
		u64 reserved_19_63 : 45;
		u64 hg2tx_en : 1;
		u64 hg2rx_en : 1;
		u64 phys_en : 1;
		u64 logl_en : 16;
	} s;
	struct cvmx_bgxx_smux_hg2_control_s cn73xx;
	struct cvmx_bgxx_smux_hg2_control_s cn78xx;
	struct cvmx_bgxx_smux_hg2_control_s cn78xxp1;
	struct cvmx_bgxx_smux_hg2_control_s cnf75xx;
};

typedef union cvmx_bgxx_smux_hg2_control cvmx_bgxx_smux_hg2_control_t;

/**
 * cvmx_bgx#_smu#_rx_bad_col_hi
 */
union cvmx_bgxx_smux_rx_bad_col_hi {
	u64 u64;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s {
		u64 reserved_17_63 : 47;
		u64 val : 1;
		u64 state : 8;
		u64 lane_rxc : 8;
	} s;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s cn73xx;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s cn78xx;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_bad_col_hi cvmx_bgxx_smux_rx_bad_col_hi_t;

/**
 * cvmx_bgx#_smu#_rx_bad_col_lo
 */
union cvmx_bgxx_smux_rx_bad_col_lo {
	u64 u64;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s {
		u64 lane_rxd : 64;
	} s;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s cn73xx;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s cn78xx;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_bad_col_lo cvmx_bgxx_smux_rx_bad_col_lo_t;

/**
 * cvmx_bgx#_smu#_rx_ctl
 */
union cvmx_bgxx_smux_rx_ctl {
	u64 u64;
	struct cvmx_bgxx_smux_rx_ctl_s {
		u64 reserved_2_63 : 62;
		u64 status : 2;
	} s;
	struct cvmx_bgxx_smux_rx_ctl_s cn73xx;
	struct cvmx_bgxx_smux_rx_ctl_s cn78xx;
	struct cvmx_bgxx_smux_rx_ctl_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_ctl cvmx_bgxx_smux_rx_ctl_t;

/**
 * cvmx_bgx#_smu#_rx_decision
 *
 * This register specifies the byte count used to determine when to accept or to filter a packet.
 * As each byte in a packet is received by BGX, the L2 byte count (i.e. the number of bytes from
 * the beginning of the L2 header (DMAC)) is compared against CNT. In normal operation, the L2
 * header begins after the PREAMBLE + SFD (BGX()_SMU()_RX_FRM_CTL[PRE_CHK] = 1) and any
 * optional UDD skip data (BGX()_SMU()_RX_UDD_SKP[LEN]).
 */
union cvmx_bgxx_smux_rx_decision {
	u64 u64;
	struct cvmx_bgxx_smux_rx_decision_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_bgxx_smux_rx_decision_s cn73xx;
	struct cvmx_bgxx_smux_rx_decision_s cn78xx;
	struct cvmx_bgxx_smux_rx_decision_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_decision_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_decision cvmx_bgxx_smux_rx_decision_t;

/**
 * cvmx_bgx#_smu#_rx_frm_chk
 *
 * The CSRs provide the enable bits for a subset of errors passed to CMR encoded.
 *
 */
union cvmx_bgxx_smux_rx_frm_chk {
	u64 u64;
	struct cvmx_bgxx_smux_rx_frm_chk_s {
		u64 reserved_9_63 : 55;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 reserved_6_6 : 1;
		u64 fcserr_c : 1;
		u64 fcserr_d : 1;
		u64 jabber : 1;
		u64 reserved_0_2 : 3;
	} s;
	struct cvmx_bgxx_smux_rx_frm_chk_s cn73xx;
	struct cvmx_bgxx_smux_rx_frm_chk_s cn78xx;
	struct cvmx_bgxx_smux_rx_frm_chk_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_frm_chk_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_frm_chk cvmx_bgxx_smux_rx_frm_chk_t;

/**
 * cvmx_bgx#_smu#_rx_frm_ctl
 *
 * This register controls the handling of the frames.
 * The [CTL_BCK] and [CTL_DRP] bits control how the hardware handles incoming PAUSE packets. The
 * most
 * common modes of operation:
 * _ [CTL_BCK] = 1, [CTL_DRP] = 1: hardware handles everything
 * _ [CTL_BCK] = 0, [CTL_DRP] = 0: software sees all PAUSE frames
 * _ [CTL_BCK] = 0, [CTL_DRP] = 1: all PAUSE frames are completely ignored
 *
 * These control bits should be set to [CTL_BCK] = 0, [CTL_DRP] = 0 in half-duplex mode. Since
 * PAUSE
 * packets only apply to full duplex operation, any PAUSE packet would constitute an exception
 * which should be handled by the processing cores. PAUSE packets should not be forwarded.
 */
union cvmx_bgxx_smux_rx_frm_ctl {
	u64 u64;
	struct cvmx_bgxx_smux_rx_frm_ctl_s {
		u64 reserved_13_63 : 51;
		u64 ptp_mode : 1;
		u64 reserved_6_11 : 6;
		u64 ctl_smac : 1;
		u64 ctl_mcst : 1;
		u64 ctl_bck : 1;
		u64 ctl_drp : 1;
		u64 pre_strp : 1;
		u64 pre_chk : 1;
	} s;
	struct cvmx_bgxx_smux_rx_frm_ctl_s cn73xx;
	struct cvmx_bgxx_smux_rx_frm_ctl_s cn78xx;
	struct cvmx_bgxx_smux_rx_frm_ctl_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_frm_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_frm_ctl cvmx_bgxx_smux_rx_frm_ctl_t;

/**
 * cvmx_bgx#_smu#_rx_int
 *
 * SMU Interrupt Register.
 *
 */
union cvmx_bgxx_smux_rx_int {
	u64 u64;
	struct cvmx_bgxx_smux_rx_int_s {
		u64 reserved_12_63 : 52;
		u64 hg2cc : 1;
		u64 hg2fld : 1;
		u64 bad_term : 1;
		u64 bad_seq : 1;
		u64 rem_fault : 1;
		u64 loc_fault : 1;
		u64 rsverr : 1;
		u64 pcterr : 1;
		u64 skperr : 1;
		u64 rcverr : 1;
		u64 fcserr : 1;
		u64 jabber : 1;
	} s;
	struct cvmx_bgxx_smux_rx_int_s cn73xx;
	struct cvmx_bgxx_smux_rx_int_s cn78xx;
	struct cvmx_bgxx_smux_rx_int_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_int_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_int cvmx_bgxx_smux_rx_int_t;

/**
 * cvmx_bgx#_smu#_rx_jabber
 *
 * This register specifies the maximum size for packets, beyond which the SMU truncates. In
 * XAUI/RXAUI mode, port 0 is used for checking.
 */
union cvmx_bgxx_smux_rx_jabber {
	u64 u64;
	struct cvmx_bgxx_smux_rx_jabber_s {
		u64 reserved_16_63 : 48;
		u64 cnt : 16;
	} s;
	struct cvmx_bgxx_smux_rx_jabber_s cn73xx;
	struct cvmx_bgxx_smux_rx_jabber_s cn78xx;
	struct cvmx_bgxx_smux_rx_jabber_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_jabber_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_jabber cvmx_bgxx_smux_rx_jabber_t;

/**
 * cvmx_bgx#_smu#_rx_udd_skp
 *
 * This register specifies the amount of user-defined data (UDD) added before the start of the
 * L2C data.
 */
union cvmx_bgxx_smux_rx_udd_skp {
	u64 u64;
	struct cvmx_bgxx_smux_rx_udd_skp_s {
		u64 reserved_9_63 : 55;
		u64 fcssel : 1;
		u64 reserved_7_7 : 1;
		u64 len : 7;
	} s;
	struct cvmx_bgxx_smux_rx_udd_skp_s cn73xx;
	struct cvmx_bgxx_smux_rx_udd_skp_s cn78xx;
	struct cvmx_bgxx_smux_rx_udd_skp_s cn78xxp1;
	struct cvmx_bgxx_smux_rx_udd_skp_s cnf75xx;
};

typedef union cvmx_bgxx_smux_rx_udd_skp cvmx_bgxx_smux_rx_udd_skp_t;

/**
 * cvmx_bgx#_smu#_smac
 */
union cvmx_bgxx_smux_smac {
	u64 u64;
	struct cvmx_bgxx_smux_smac_s {
		u64 reserved_48_63 : 16;
		u64 smac : 48;
	} s;
	struct cvmx_bgxx_smux_smac_s cn73xx;
	struct cvmx_bgxx_smux_smac_s cn78xx;
	struct cvmx_bgxx_smux_smac_s cn78xxp1;
	struct cvmx_bgxx_smux_smac_s cnf75xx;
};

typedef union cvmx_bgxx_smux_smac cvmx_bgxx_smux_smac_t;

/**
 * cvmx_bgx#_smu#_tx_append
 *
 * For more details on the interactions between FCS and PAD, see also the description of
 * BGX()_SMU()_TX_MIN_PKT[MIN_SIZE].
 */
union cvmx_bgxx_smux_tx_append {
	u64 u64;
	struct cvmx_bgxx_smux_tx_append_s {
		u64 reserved_4_63 : 60;
		u64 fcs_c : 1;
		u64 fcs_d : 1;
		u64 pad : 1;
		u64 preamble : 1;
	} s;
	struct cvmx_bgxx_smux_tx_append_s cn73xx;
	struct cvmx_bgxx_smux_tx_append_s cn78xx;
	struct cvmx_bgxx_smux_tx_append_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_append_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_append cvmx_bgxx_smux_tx_append_t;

/**
 * cvmx_bgx#_smu#_tx_ctl
 */
union cvmx_bgxx_smux_tx_ctl {
	u64 u64;
	struct cvmx_bgxx_smux_tx_ctl_s {
		u64 reserved_31_63 : 33;
		u64 spu_mrk_cnt : 20;
		u64 hg_pause_hgi : 2;
		u64 hg_en : 1;
		u64 l2p_bp_conv : 1;
		u64 ls_byp : 1;
		u64 ls : 2;
		u64 reserved_3_3 : 1;
		u64 x4a_dis : 1;
		u64 uni_en : 1;
		u64 dic_en : 1;
	} s;
	struct cvmx_bgxx_smux_tx_ctl_s cn73xx;
	struct cvmx_bgxx_smux_tx_ctl_s cn78xx;
	struct cvmx_bgxx_smux_tx_ctl_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_ctl_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_ctl cvmx_bgxx_smux_tx_ctl_t;

/**
 * cvmx_bgx#_smu#_tx_ifg
 *
 * Programming IFG1 and IFG2:
 * * For XAUI/RXAUI/10Gbs/40Gbs systems that require IEEE 802.3 compatibility, the IFG1+IFG2 sum
 * must be 12.
 * * In loopback mode, the IFG1+IFG2 of local and remote parties must match exactly; otherwise
 * one of the two sides' loopback FIFO will overrun: BGX()_SMU()_TX_INT[LB_OVRFLW].
 */
union cvmx_bgxx_smux_tx_ifg {
	u64 u64;
	struct cvmx_bgxx_smux_tx_ifg_s {
		u64 reserved_8_63 : 56;
		u64 ifg2 : 4;
		u64 ifg1 : 4;
	} s;
	struct cvmx_bgxx_smux_tx_ifg_s cn73xx;
	struct cvmx_bgxx_smux_tx_ifg_s cn78xx;
	struct cvmx_bgxx_smux_tx_ifg_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_ifg_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_ifg cvmx_bgxx_smux_tx_ifg_t;

/**
 * cvmx_bgx#_smu#_tx_int
 */
union cvmx_bgxx_smux_tx_int {
	u64 u64;
	struct cvmx_bgxx_smux_tx_int_s {
		u64 reserved_5_63 : 59;
		u64 lb_ovrflw : 1;
		u64 lb_undflw : 1;
		u64 fake_commit : 1;
		u64 xchange : 1;
		u64 undflw : 1;
	} s;
	struct cvmx_bgxx_smux_tx_int_s cn73xx;
	struct cvmx_bgxx_smux_tx_int_s cn78xx;
	struct cvmx_bgxx_smux_tx_int_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_int_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_int cvmx_bgxx_smux_tx_int_t;

/**
 * cvmx_bgx#_smu#_tx_min_pkt
 */
union cvmx_bgxx_smux_tx_min_pkt {
	u64 u64;
	struct cvmx_bgxx_smux_tx_min_pkt_s {
		u64 reserved_8_63 : 56;
		u64 min_size : 8;
	} s;
	struct cvmx_bgxx_smux_tx_min_pkt_s cn73xx;
	struct cvmx_bgxx_smux_tx_min_pkt_s cn78xx;
	struct cvmx_bgxx_smux_tx_min_pkt_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_min_pkt_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_min_pkt cvmx_bgxx_smux_tx_min_pkt_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_dmac
 *
 * This register provides the DMAC value that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_dmac {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s {
		u64 reserved_48_63 : 16;
		u64 dmac : 48;
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_pkt_dmac cvmx_bgxx_smux_tx_pause_pkt_dmac_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_interval
 *
 * This register specifies how often PAUSE packets are sent.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_interval {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s {
		u64 reserved_33_63 : 31;
		u64 hg2_intra_en : 1;
		u64 hg2_intra_interval : 16;
		u64 interval : 16;
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_pkt_interval cvmx_bgxx_smux_tx_pause_pkt_interval_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_time
 */
union cvmx_bgxx_smux_tx_pause_pkt_time {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s {
		u64 reserved_16_63 : 48;
		u64 p_time : 16;
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_pkt_time cvmx_bgxx_smux_tx_pause_pkt_time_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_type
 *
 * This register provides the P_TYPE field that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_type {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s {
		u64 reserved_16_63 : 48;
		u64 p_type : 16;
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_pkt_type cvmx_bgxx_smux_tx_pause_pkt_type_t;

/**
 * cvmx_bgx#_smu#_tx_pause_togo
 */
union cvmx_bgxx_smux_tx_pause_togo {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_togo_s {
		u64 reserved_32_63 : 32;
		u64 msg_time : 16;
		u64 p_time : 16;
	} s;
	struct cvmx_bgxx_smux_tx_pause_togo_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_togo_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_togo_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_togo_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_togo cvmx_bgxx_smux_tx_pause_togo_t;

/**
 * cvmx_bgx#_smu#_tx_pause_zero
 */
union cvmx_bgxx_smux_tx_pause_zero {
	u64 u64;
	struct cvmx_bgxx_smux_tx_pause_zero_s {
		u64 reserved_1_63 : 63;
		u64 send : 1;
	} s;
	struct cvmx_bgxx_smux_tx_pause_zero_s cn73xx;
	struct cvmx_bgxx_smux_tx_pause_zero_s cn78xx;
	struct cvmx_bgxx_smux_tx_pause_zero_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_pause_zero_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_pause_zero cvmx_bgxx_smux_tx_pause_zero_t;

/**
 * cvmx_bgx#_smu#_tx_soft_pause
 */
union cvmx_bgxx_smux_tx_soft_pause {
	u64 u64;
	struct cvmx_bgxx_smux_tx_soft_pause_s {
		u64 reserved_16_63 : 48;
		u64 p_time : 16;
	} s;
	struct cvmx_bgxx_smux_tx_soft_pause_s cn73xx;
	struct cvmx_bgxx_smux_tx_soft_pause_s cn78xx;
	struct cvmx_bgxx_smux_tx_soft_pause_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_soft_pause_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_soft_pause cvmx_bgxx_smux_tx_soft_pause_t;

/**
 * cvmx_bgx#_smu#_tx_thresh
 */
union cvmx_bgxx_smux_tx_thresh {
	u64 u64;
	struct cvmx_bgxx_smux_tx_thresh_s {
		u64 reserved_11_63 : 53;
		u64 cnt : 11;
	} s;
	struct cvmx_bgxx_smux_tx_thresh_s cn73xx;
	struct cvmx_bgxx_smux_tx_thresh_s cn78xx;
	struct cvmx_bgxx_smux_tx_thresh_s cn78xxp1;
	struct cvmx_bgxx_smux_tx_thresh_s cnf75xx;
};

typedef union cvmx_bgxx_smux_tx_thresh cvmx_bgxx_smux_tx_thresh_t;

/**
 * cvmx_bgx#_spu#_an_adv
 *
 * Software programs this register with the contents of the AN-link code word base page to be
 * transmitted during autonegotiation. (See IEEE 802.3 section 73.6 for details.) Any write
 * operations to this register prior to completion of autonegotiation, as indicated by
 * BGX()_SPU()_AN_STATUS[AN_COMPLETE], should be followed by a renegotiation in order for
 * the new values to take effect. Renegotiation is initiated by setting
 * BGX()_SPU()_AN_CONTROL[AN_RESTART]. Once autonegotiation has completed, software can
 * examine this register along with BGX()_SPU()_AN_LP_BASE to determine the highest
 * common denominator technology.
 */
union cvmx_bgxx_spux_an_adv {
	u64 u64;
	struct cvmx_bgxx_spux_an_adv_s {
		u64 reserved_48_63 : 16;
		u64 fec_req : 1;
		u64 fec_able : 1;
		u64 arsv : 19;
		u64 a100g_cr10 : 1;
		u64 a40g_cr4 : 1;
		u64 a40g_kr4 : 1;
		u64 a10g_kr : 1;
		u64 a10g_kx4 : 1;
		u64 a1g_kx : 1;
		u64 t : 5;
		u64 np : 1;
		u64 ack : 1;
		u64 rf : 1;
		u64 xnp_able : 1;
		u64 asm_dir : 1;
		u64 pause : 1;
		u64 e : 5;
		u64 s : 5;
	} s;
	struct cvmx_bgxx_spux_an_adv_s cn73xx;
	struct cvmx_bgxx_spux_an_adv_s cn78xx;
	struct cvmx_bgxx_spux_an_adv_s cn78xxp1;
	struct cvmx_bgxx_spux_an_adv_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_adv cvmx_bgxx_spux_an_adv_t;

/**
 * cvmx_bgx#_spu#_an_bp_status
 *
 * The contents of this register are updated
 * during autonegotiation and are valid when BGX()_SPU()_AN_STATUS[AN_COMPLETE] is set.
 * At that time, one of the port type bits ([N100G_CR10], [N40G_CR4], [N40G_KR4], [N10G_KR],
 * [N10G_KX4],
 * [N1G_KX]) will be set depending on the AN priority resolution. If a BASE-R type is negotiated,
 * then [FEC] will be set to indicate that FEC operation has been negotiated, and will be
 * clear otherwise.
 */
union cvmx_bgxx_spux_an_bp_status {
	u64 u64;
	struct cvmx_bgxx_spux_an_bp_status_s {
		u64 reserved_9_63 : 55;
		u64 n100g_cr10 : 1;
		u64 reserved_7_7 : 1;
		u64 n40g_cr4 : 1;
		u64 n40g_kr4 : 1;
		u64 fec : 1;
		u64 n10g_kr : 1;
		u64 n10g_kx4 : 1;
		u64 n1g_kx : 1;
		u64 bp_an_able : 1;
	} s;
	struct cvmx_bgxx_spux_an_bp_status_s cn73xx;
	struct cvmx_bgxx_spux_an_bp_status_s cn78xx;
	struct cvmx_bgxx_spux_an_bp_status_s cn78xxp1;
	struct cvmx_bgxx_spux_an_bp_status_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_bp_status cvmx_bgxx_spux_an_bp_status_t;

/**
 * cvmx_bgx#_spu#_an_control
 */
union cvmx_bgxx_spux_an_control {
	u64 u64;
	struct cvmx_bgxx_spux_an_control_s {
		u64 reserved_16_63 : 48;
		u64 an_reset : 1;
		u64 reserved_14_14 : 1;
		u64 xnp_en : 1;
		u64 an_en : 1;
		u64 reserved_10_11 : 2;
		u64 an_restart : 1;
		u64 reserved_0_8 : 9;
	} s;
	struct cvmx_bgxx_spux_an_control_s cn73xx;
	struct cvmx_bgxx_spux_an_control_s cn78xx;
	struct cvmx_bgxx_spux_an_control_s cn78xxp1;
	struct cvmx_bgxx_spux_an_control_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_control cvmx_bgxx_spux_an_control_t;

/**
 * cvmx_bgx#_spu#_an_lp_base
 *
 * This register captures the contents of the latest AN link code word base page received from
 * the link partner during autonegotiation. (See IEEE 802.3 section 73.6 for details.)
 * BGX()_SPU()_AN_STATUS[PAGE_RX] is set when this register is updated by hardware.
 */
union cvmx_bgxx_spux_an_lp_base {
	u64 u64;
	struct cvmx_bgxx_spux_an_lp_base_s {
		u64 reserved_48_63 : 16;
		u64 fec_req : 1;
		u64 fec_able : 1;
		u64 arsv : 19;
		u64 a100g_cr10 : 1;
		u64 a40g_cr4 : 1;
		u64 a40g_kr4 : 1;
		u64 a10g_kr : 1;
		u64 a10g_kx4 : 1;
		u64 a1g_kx : 1;
		u64 t : 5;
		u64 np : 1;
		u64 ack : 1;
		u64 rf : 1;
		u64 xnp_able : 1;
		u64 asm_dir : 1;
		u64 pause : 1;
		u64 e : 5;
		u64 s : 5;
	} s;
	struct cvmx_bgxx_spux_an_lp_base_s cn73xx;
	struct cvmx_bgxx_spux_an_lp_base_s cn78xx;
	struct cvmx_bgxx_spux_an_lp_base_s cn78xxp1;
	struct cvmx_bgxx_spux_an_lp_base_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_lp_base cvmx_bgxx_spux_an_lp_base_t;

/**
 * cvmx_bgx#_spu#_an_lp_xnp
 *
 * This register captures the contents of the latest next page code word received from the link
 * partner during autonegotiation, if any. See section 802.3 section 73.7.7 for details.
 */
union cvmx_bgxx_spux_an_lp_xnp {
	u64 u64;
	struct cvmx_bgxx_spux_an_lp_xnp_s {
		u64 reserved_48_63 : 16;
		u64 u : 32;
		u64 np : 1;
		u64 ack : 1;
		u64 mp : 1;
		u64 ack2 : 1;
		u64 toggle : 1;
		u64 m_u : 11;
	} s;
	struct cvmx_bgxx_spux_an_lp_xnp_s cn73xx;
	struct cvmx_bgxx_spux_an_lp_xnp_s cn78xx;
	struct cvmx_bgxx_spux_an_lp_xnp_s cn78xxp1;
	struct cvmx_bgxx_spux_an_lp_xnp_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_lp_xnp cvmx_bgxx_spux_an_lp_xnp_t;

/**
 * cvmx_bgx#_spu#_an_status
 */
union cvmx_bgxx_spux_an_status {
	u64 u64;
	struct cvmx_bgxx_spux_an_status_s {
		u64 reserved_10_63 : 54;
		u64 prl_flt : 1;
		u64 reserved_8_8 : 1;
		u64 xnp_stat : 1;
		u64 page_rx : 1;
		u64 an_complete : 1;
		u64 rmt_flt : 1;
		u64 an_able : 1;
		u64 link_status : 1;
		u64 reserved_1_1 : 1;
		u64 lp_an_able : 1;
	} s;
	struct cvmx_bgxx_spux_an_status_s cn73xx;
	struct cvmx_bgxx_spux_an_status_s cn78xx;
	struct cvmx_bgxx_spux_an_status_s cn78xxp1;
	struct cvmx_bgxx_spux_an_status_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_status cvmx_bgxx_spux_an_status_t;

/**
 * cvmx_bgx#_spu#_an_xnp_tx
 *
 * Software programs this register with the contents of the AN message next page or unformatted
 * next page link code word to be transmitted during autonegotiation. Next page exchange occurs
 * after the base link code words have been exchanged if either end of the link segment sets the
 * NP bit to 1, indicating that it has at least one next page to send. Once initiated, next page
 * exchange continues until both ends of the link segment set their NP bits to 0. See section
 * 802.3 section 73.7.7 for details.
 */
union cvmx_bgxx_spux_an_xnp_tx {
	u64 u64;
	struct cvmx_bgxx_spux_an_xnp_tx_s {
		u64 reserved_48_63 : 16;
		u64 u : 32;
		u64 np : 1;
		u64 ack : 1;
		u64 mp : 1;
		u64 ack2 : 1;
		u64 toggle : 1;
		u64 m_u : 11;
	} s;
	struct cvmx_bgxx_spux_an_xnp_tx_s cn73xx;
	struct cvmx_bgxx_spux_an_xnp_tx_s cn78xx;
	struct cvmx_bgxx_spux_an_xnp_tx_s cn78xxp1;
	struct cvmx_bgxx_spux_an_xnp_tx_s cnf75xx;
};

typedef union cvmx_bgxx_spux_an_xnp_tx cvmx_bgxx_spux_an_xnp_tx_t;

/**
 * cvmx_bgx#_spu#_br_algn_status
 *
 * This register implements the IEEE 802.3 multilane BASE-R PCS alignment status 1-4 registers
 * (3.50-3.53). It is valid only when the LPCS type is 40GBASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x4), and always returns 0x0 for all other LPCS
 * types. IEEE 802.3 bits that are not applicable to 40GBASE-R (e.g. status bits for PCS lanes
 * 19-4) are not implemented and marked as reserved. PCS lanes 3-0 are valid and are mapped to
 * physical SerDes lanes based on the programming of BGX()_CMR()_CONFIG[LANE_TO_SDS].
 */
union cvmx_bgxx_spux_br_algn_status {
	u64 u64;
	struct cvmx_bgxx_spux_br_algn_status_s {
		u64 reserved_36_63 : 28;
		u64 marker_lock : 4;
		u64 reserved_13_31 : 19;
		u64 alignd : 1;
		u64 reserved_4_11 : 8;
		u64 block_lock : 4;
	} s;
	struct cvmx_bgxx_spux_br_algn_status_s cn73xx;
	struct cvmx_bgxx_spux_br_algn_status_s cn78xx;
	struct cvmx_bgxx_spux_br_algn_status_s cn78xxp1;
	struct cvmx_bgxx_spux_br_algn_status_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_algn_status cvmx_bgxx_spux_br_algn_status_t;

/**
 * cvmx_bgx#_spu#_br_bip_err_cnt
 *
 * This register implements the IEEE 802.3 BIP error-counter registers for PCS lanes 0-3
 * (3.200-3.203). It is valid only when the LPCS type is 40GBASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x4), and always returns 0x0 for all other LPCS
 * types. The counters are indexed by the RX PCS lane number based on the Alignment Marker
 * detected on each lane and captured in BGX()_SPU()_BR_LANE_MAP. Each counter counts the
 * BIP errors for its PCS lane, and is held at all ones in case of overflow. The counters are
 * reset to all 0s when this register is read by software.
 *
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in IEEE 802.3.
 */
union cvmx_bgxx_spux_br_bip_err_cnt {
	u64 u64;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s {
		u64 bip_err_cnt_ln3 : 16;
		u64 bip_err_cnt_ln2 : 16;
		u64 bip_err_cnt_ln1 : 16;
		u64 bip_err_cnt_ln0 : 16;
	} s;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s cn73xx;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s cn78xx;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s cn78xxp1;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_bip_err_cnt cvmx_bgxx_spux_br_bip_err_cnt_t;

/**
 * cvmx_bgx#_spu#_br_lane_map
 *
 * This register implements the IEEE 802.3 lane 0-3 mapping registers (3.400-3.403). It is valid
 * only when the LPCS type is 40GBASE-R (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x4), and always
 * returns 0x0 for all other LPCS types. The LNx_MAPPING field for each programmed PCS lane
 * (called service interface in 802.3ba-2010) is valid when that lane has achieved alignment
 * marker lock on the receive side (i.e. the associated
 * BGX()_SPU()_BR_ALGN_STATUS[MARKER_LOCK] = 1), and is invalid otherwise. When valid, it
 * returns the actual detected receive PCS lane number based on the received alignment marker
 * contents received on that service interface.
 *
 * The mapping is flexible because IEEE 802.3 allows multilane BASE-R receive lanes to be re-
 * ordered. Note that for the transmit side, each PCS lane is mapped to a physical SerDes lane
 * based on the programming of BGX()_CMR()_CONFIG[LANE_TO_SDS]. For the receive side,
 * BGX()_CMR()_CONFIG[LANE_TO_SDS] specifies the service interface to physical SerDes
 * lane mapping, and this register specifies the service interface to PCS lane mapping.
 */
union cvmx_bgxx_spux_br_lane_map {
	u64 u64;
	struct cvmx_bgxx_spux_br_lane_map_s {
		u64 reserved_54_63 : 10;
		u64 ln3_mapping : 6;
		u64 reserved_38_47 : 10;
		u64 ln2_mapping : 6;
		u64 reserved_22_31 : 10;
		u64 ln1_mapping : 6;
		u64 reserved_6_15 : 10;
		u64 ln0_mapping : 6;
	} s;
	struct cvmx_bgxx_spux_br_lane_map_s cn73xx;
	struct cvmx_bgxx_spux_br_lane_map_s cn78xx;
	struct cvmx_bgxx_spux_br_lane_map_s cn78xxp1;
	struct cvmx_bgxx_spux_br_lane_map_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_lane_map cvmx_bgxx_spux_br_lane_map_t;

/**
 * cvmx_bgx#_spu#_br_pmd_control
 */
union cvmx_bgxx_spux_br_pmd_control {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_control_s {
		u64 reserved_2_63 : 62;
		u64 train_en : 1;
		u64 train_restart : 1;
	} s;
	struct cvmx_bgxx_spux_br_pmd_control_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_control_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_control_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_control_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_control cvmx_bgxx_spux_br_pmd_control_t;

/**
 * cvmx_bgx#_spu#_br_pmd_ld_cup
 *
 * This register implements 802.3 MDIO register 1.153 for 10GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 10G_R)
 * and MDIO registers 1.1300-1.1303 for 40GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 40G_R). It is automatically cleared at the start of training.
 * When link training
 * is in progress, each field reflects the contents of the coefficient update field in the
 * associated lane's outgoing training frame. The fields in this register are read/write even
 * though they are specified as read-only in 802.3.
 *
 * If BGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set, then this register must be updated
 * by software during link training and hardware updates are disabled. If
 * BGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is clear, this register is automatically
 * updated by hardware, and it should not be written by software. The lane fields in this
 * register are indexed by logical PCS lane ID.
 *
 * The lane 0 field (LN0_*) is valid for both
 * 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*, LN3_*) are only valid for
 * 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_ld_cup {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s {
		u64 ln3_cup : 16;
		u64 ln2_cup : 16;
		u64 ln1_cup : 16;
		u64 ln0_cup : 16;
	} s;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_ld_cup cvmx_bgxx_spux_br_pmd_ld_cup_t;

/**
 * cvmx_bgx#_spu#_br_pmd_ld_rep
 *
 * This register implements 802.3 MDIO register 1.154 for 10GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 10G_R) and MDIO registers 1.1400-1.1403 for 40GBASE-R
 * (when BGX()_CMR()_CONFIG[LMAC_TYPE] = 40G_R). It is automatically cleared at the start of
 * training. Each field
 * reflects the contents of the status report field in the associated lane's outgoing training
 * frame. The fields in this register are read/write even though they are specified as read-only
 * in 802.3. If BGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set, then this register must
 * be updated by software during link training and hardware updates are disabled. If
 * BGX()_SPU_DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is clear, this register is automatically
 * updated by hardware, and it should not be written by software. The lane fields in this
 * register are indexed by logical PCS lane ID.
 *
 * The lane 0 field (LN0_*) is valid for both
 * 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*, LN3_*) are only valid for
 * 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_ld_rep {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s {
		u64 ln3_rep : 16;
		u64 ln2_rep : 16;
		u64 ln1_rep : 16;
		u64 ln0_rep : 16;
	} s;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_ld_rep cvmx_bgxx_spux_br_pmd_ld_rep_t;

/**
 * cvmx_bgx#_spu#_br_pmd_lp_cup
 *
 * This register implements 802.3 MDIO register 1.152 for 10GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 10G_R)
 * and MDIO registers 1.1100-1.1103 for 40GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 40G_R). It is automatically cleared at the start of training.
 * Each field reflects
 * the contents of the coefficient update field in the lane's most recently received training
 * frame. This register should not be written when link training is enabled, i.e. when
 * BGX()_SPU()_BR_PMD_CONTROL[TRAIN_EN] is set. The lane fields in this register are indexed by
 * logical PCS lane ID.
 *
 * The lane 0 field (LN0_*) is valid for both 10GBASE-R and 40GBASE-R. The remaining fields
 * (LN1_*, LN2_*, LN3_*) are only valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_lp_cup {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s {
		u64 ln3_cup : 16;
		u64 ln2_cup : 16;
		u64 ln1_cup : 16;
		u64 ln0_cup : 16;
	} s;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_lp_cup cvmx_bgxx_spux_br_pmd_lp_cup_t;

/**
 * cvmx_bgx#_spu#_br_pmd_lp_rep
 *
 * This register implements 802.3 MDIO register 1.153 for 10GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 10G_R)
 * and MDIO registers 1.1200-1.1203 for 40GBASE-R (when
 * BGX()_CMR()_CONFIG[LMAC_TYPE] = 40G_R). It is automatically cleared at the start of training.
 * Each field reflects
 * the contents of the status report field in the associated lane's most recently received
 * training frame. The lane fields in this register are indexed by logical PCS lane ID.
 *
 * The lane
 * 0 field (LN0_*) is valid for both 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*,
 * LN3_*) are only valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_lp_rep {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s {
		u64 ln3_rep : 16;
		u64 ln2_rep : 16;
		u64 ln1_rep : 16;
		u64 ln0_rep : 16;
	} s;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_lp_rep cvmx_bgxx_spux_br_pmd_lp_rep_t;

/**
 * cvmx_bgx#_spu#_br_pmd_status
 *
 * The lane fields in this register are indexed by logical PCS lane ID. The lane 0 field (LN0_*)
 * is valid for both 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*, LN3_*) are only
 * valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_status {
	u64 u64;
	struct cvmx_bgxx_spux_br_pmd_status_s {
		u64 reserved_16_63 : 48;
		u64 ln3_train_status : 4;
		u64 ln2_train_status : 4;
		u64 ln1_train_status : 4;
		u64 ln0_train_status : 4;
	} s;
	struct cvmx_bgxx_spux_br_pmd_status_s cn73xx;
	struct cvmx_bgxx_spux_br_pmd_status_s cn78xx;
	struct cvmx_bgxx_spux_br_pmd_status_s cn78xxp1;
	struct cvmx_bgxx_spux_br_pmd_status_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_pmd_status cvmx_bgxx_spux_br_pmd_status_t;

/**
 * cvmx_bgx#_spu#_br_status1
 */
union cvmx_bgxx_spux_br_status1 {
	u64 u64;
	struct cvmx_bgxx_spux_br_status1_s {
		u64 reserved_13_63 : 51;
		u64 rcv_lnk : 1;
		u64 reserved_4_11 : 8;
		u64 prbs9 : 1;
		u64 prbs31 : 1;
		u64 hi_ber : 1;
		u64 blk_lock : 1;
	} s;
	struct cvmx_bgxx_spux_br_status1_s cn73xx;
	struct cvmx_bgxx_spux_br_status1_s cn78xx;
	struct cvmx_bgxx_spux_br_status1_s cn78xxp1;
	struct cvmx_bgxx_spux_br_status1_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_status1 cvmx_bgxx_spux_br_status1_t;

/**
 * cvmx_bgx#_spu#_br_status2
 *
 * This register implements a combination of the following IEEE 802.3 registers:
 * * BASE-R PCS status 2 (MDIO address 3.33).
 * * BASE-R BER high-order counter (MDIO address 3.44).
 * * Errored-blocks high-order counter (MDIO address 3.45).
 *
 * Note that the relative locations of some fields have been moved from IEEE 802.3 in order to
 * make the register layout more software friendly: the BER counter high-order and low-order bits
 * from sections 3.44 and 3.33 have been combined into the contiguous, 22-bit [BER_CNT] field;
 * likewise, the errored-blocks counter high-order and low-order bits from section 3.45 have been
 * combined into the contiguous, 22-bit [ERR_BLKS] field.
 */
union cvmx_bgxx_spux_br_status2 {
	u64 u64;
	struct cvmx_bgxx_spux_br_status2_s {
		u64 reserved_62_63 : 2;
		u64 err_blks : 22;
		u64 reserved_38_39 : 2;
		u64 ber_cnt : 22;
		u64 latched_lock : 1;
		u64 latched_ber : 1;
		u64 reserved_0_13 : 14;
	} s;
	struct cvmx_bgxx_spux_br_status2_s cn73xx;
	struct cvmx_bgxx_spux_br_status2_s cn78xx;
	struct cvmx_bgxx_spux_br_status2_s cn78xxp1;
	struct cvmx_bgxx_spux_br_status2_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_status2 cvmx_bgxx_spux_br_status2_t;

/**
 * cvmx_bgx#_spu#_br_tp_control
 *
 * Refer to the test pattern methodology described in 802.3 sections 49.2.8 and 82.2.10.
 *
 */
union cvmx_bgxx_spux_br_tp_control {
	u64 u64;
	struct cvmx_bgxx_spux_br_tp_control_s {
		u64 reserved_8_63 : 56;
		u64 scramble_tp : 1;
		u64 prbs9_tx : 1;
		u64 prbs31_rx : 1;
		u64 prbs31_tx : 1;
		u64 tx_tp_en : 1;
		u64 rx_tp_en : 1;
		u64 tp_sel : 1;
		u64 dp_sel : 1;
	} s;
	struct cvmx_bgxx_spux_br_tp_control_s cn73xx;
	struct cvmx_bgxx_spux_br_tp_control_s cn78xx;
	struct cvmx_bgxx_spux_br_tp_control_s cn78xxp1;
	struct cvmx_bgxx_spux_br_tp_control_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_tp_control cvmx_bgxx_spux_br_tp_control_t;

/**
 * cvmx_bgx#_spu#_br_tp_err_cnt
 *
 * This register provides the BASE-R PCS test-pattern error counter.
 *
 */
union cvmx_bgxx_spux_br_tp_err_cnt {
	u64 u64;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s {
		u64 reserved_16_63 : 48;
		u64 err_cnt : 16;
	} s;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s cn73xx;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s cn78xx;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s cn78xxp1;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s cnf75xx;
};

typedef union cvmx_bgxx_spux_br_tp_err_cnt cvmx_bgxx_spux_br_tp_err_cnt_t;

/**
 * cvmx_bgx#_spu#_bx_status
 */
union cvmx_bgxx_spux_bx_status {
	u64 u64;
	struct cvmx_bgxx_spux_bx_status_s {
		u64 reserved_13_63 : 51;
		u64 alignd : 1;
		u64 pattst : 1;
		u64 reserved_4_10 : 7;
		u64 lsync : 4;
	} s;
	struct cvmx_bgxx_spux_bx_status_s cn73xx;
	struct cvmx_bgxx_spux_bx_status_s cn78xx;
	struct cvmx_bgxx_spux_bx_status_s cn78xxp1;
	struct cvmx_bgxx_spux_bx_status_s cnf75xx;
};

typedef union cvmx_bgxx_spux_bx_status cvmx_bgxx_spux_bx_status_t;

/**
 * cvmx_bgx#_spu#_control1
 */
union cvmx_bgxx_spux_control1 {
	u64 u64;
	struct cvmx_bgxx_spux_control1_s {
		u64 reserved_16_63 : 48;
		u64 reset : 1;
		u64 loopbck : 1;
		u64 spdsel1 : 1;
		u64 reserved_12_12 : 1;
		u64 lo_pwr : 1;
		u64 reserved_7_10 : 4;
		u64 spdsel0 : 1;
		u64 spd : 4;
		u64 reserved_0_1 : 2;
	} s;
	struct cvmx_bgxx_spux_control1_s cn73xx;
	struct cvmx_bgxx_spux_control1_s cn78xx;
	struct cvmx_bgxx_spux_control1_s cn78xxp1;
	struct cvmx_bgxx_spux_control1_s cnf75xx;
};

typedef union cvmx_bgxx_spux_control1 cvmx_bgxx_spux_control1_t;

/**
 * cvmx_bgx#_spu#_control2
 */
union cvmx_bgxx_spux_control2 {
	u64 u64;
	struct cvmx_bgxx_spux_control2_s {
		u64 reserved_3_63 : 61;
		u64 pcs_type : 3;
	} s;
	struct cvmx_bgxx_spux_control2_s cn73xx;
	struct cvmx_bgxx_spux_control2_s cn78xx;
	struct cvmx_bgxx_spux_control2_s cn78xxp1;
	struct cvmx_bgxx_spux_control2_s cnf75xx;
};

typedef union cvmx_bgxx_spux_control2 cvmx_bgxx_spux_control2_t;

/**
 * cvmx_bgx#_spu#_fec_abil
 */
union cvmx_bgxx_spux_fec_abil {
	u64 u64;
	struct cvmx_bgxx_spux_fec_abil_s {
		u64 reserved_2_63 : 62;
		u64 err_abil : 1;
		u64 fec_abil : 1;
	} s;
	struct cvmx_bgxx_spux_fec_abil_s cn73xx;
	struct cvmx_bgxx_spux_fec_abil_s cn78xx;
	struct cvmx_bgxx_spux_fec_abil_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_abil_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_abil cvmx_bgxx_spux_fec_abil_t;

/**
 * cvmx_bgx#_spu#_fec_control
 */
union cvmx_bgxx_spux_fec_control {
	u64 u64;
	struct cvmx_bgxx_spux_fec_control_s {
		u64 reserved_2_63 : 62;
		u64 err_en : 1;
		u64 fec_en : 1;
	} s;
	struct cvmx_bgxx_spux_fec_control_s cn73xx;
	struct cvmx_bgxx_spux_fec_control_s cn78xx;
	struct cvmx_bgxx_spux_fec_control_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_control_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_control cvmx_bgxx_spux_fec_control_t;

/**
 * cvmx_bgx#_spu#_fec_corr_blks01
 *
 * This register is valid only when the LPCS type is BASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x3 or 0x4). The FEC corrected-block counters are
 * defined in IEEE 802.3 section 74.8.4.1. Each corrected-blocks counter increments by 1 for a
 * corrected FEC block, i.e. an FEC block that has been received with invalid parity on the
 * associated PCS lane and has been corrected by the FEC decoder. The counter is reset to all 0s
 * when the register is read, and held at all 1s in case of overflow.
 *
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in IEEE 802.3.
 */
union cvmx_bgxx_spux_fec_corr_blks01 {
	u64 u64;
	struct cvmx_bgxx_spux_fec_corr_blks01_s {
		u64 ln1_corr_blks : 32;
		u64 ln0_corr_blks : 32;
	} s;
	struct cvmx_bgxx_spux_fec_corr_blks01_s cn73xx;
	struct cvmx_bgxx_spux_fec_corr_blks01_s cn78xx;
	struct cvmx_bgxx_spux_fec_corr_blks01_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_corr_blks01_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_corr_blks01 cvmx_bgxx_spux_fec_corr_blks01_t;

/**
 * cvmx_bgx#_spu#_fec_corr_blks23
 *
 * This register is valid only when the LPCS type is 40GBASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x4). The FEC corrected-block counters are defined in
 * IEEE 802.3 section 74.8.4.1. Each corrected-blocks counter increments by 1 for a corrected FEC
 * block, i.e. an FEC block that has been received with invalid parity on the associated PCS lane
 * and has been corrected by the FEC decoder. The counter is reset to all 0s when the register is
 * read, and held at all 1s in case of overflow.
 *
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in IEEE 802.3.
 */
union cvmx_bgxx_spux_fec_corr_blks23 {
	u64 u64;
	struct cvmx_bgxx_spux_fec_corr_blks23_s {
		u64 ln3_corr_blks : 32;
		u64 ln2_corr_blks : 32;
	} s;
	struct cvmx_bgxx_spux_fec_corr_blks23_s cn73xx;
	struct cvmx_bgxx_spux_fec_corr_blks23_s cn78xx;
	struct cvmx_bgxx_spux_fec_corr_blks23_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_corr_blks23_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_corr_blks23 cvmx_bgxx_spux_fec_corr_blks23_t;

/**
 * cvmx_bgx#_spu#_fec_uncorr_blks01
 *
 * This register is valid only when the LPCS type is BASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x3 or 0x4). The FEC corrected-block counters are
 * defined in IEEE 802.3 section 74.8.4.2. Each uncorrected-blocks counter increments by 1 for an
 * uncorrected FEC block, i.e. an FEC block that has been received with invalid parity on the
 * associated PCS lane and has not been corrected by the FEC decoder. The counter is reset to all
 * 0s when the register is read, and held at all 1s in case of overflow.
 *
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in IEEE 802.3.
 */
union cvmx_bgxx_spux_fec_uncorr_blks01 {
	u64 u64;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s {
		u64 ln1_uncorr_blks : 32;
		u64 ln0_uncorr_blks : 32;
	} s;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s cn73xx;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s cn78xx;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_uncorr_blks01 cvmx_bgxx_spux_fec_uncorr_blks01_t;

/**
 * cvmx_bgx#_spu#_fec_uncorr_blks23
 *
 * This register is valid only when the LPCS type is 40GBASE-R
 * (BGX()_CMR()_CONFIG[LMAC_TYPE] = 0x4). The FEC uncorrected-block counters are defined
 * in IEEE 802.3 section 74.8.4.2. Each corrected-blocks counter increments by 1 for an
 * uncorrected FEC block, i.e. an FEC block that has been received with invalid parity on the
 * associated PCS lane and has not been corrected by the FEC decoder. The counter is reset to all
 * 0s when the register is read, and held at all 1s in case of overflow.
 *
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in IEEE 802.3.
 */
union cvmx_bgxx_spux_fec_uncorr_blks23 {
	u64 u64;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s {
		u64 ln3_uncorr_blks : 32;
		u64 ln2_uncorr_blks : 32;
	} s;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s cn73xx;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s cn78xx;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s cn78xxp1;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s cnf75xx;
};

typedef union cvmx_bgxx_spux_fec_uncorr_blks23 cvmx_bgxx_spux_fec_uncorr_blks23_t;

/**
 * cvmx_bgx#_spu#_int
 */
union cvmx_bgxx_spux_int {
	u64 u64;
	struct cvmx_bgxx_spux_int_s {
		u64 reserved_15_63 : 49;
		u64 training_failure : 1;
		u64 training_done : 1;
		u64 an_complete : 1;
		u64 an_link_good : 1;
		u64 an_page_rx : 1;
		u64 fec_uncorr : 1;
		u64 fec_corr : 1;
		u64 bip_err : 1;
		u64 dbg_sync : 1;
		u64 algnlos : 1;
		u64 synlos : 1;
		u64 bitlckls : 1;
		u64 err_blk : 1;
		u64 rx_link_down : 1;
		u64 rx_link_up : 1;
	} s;
	struct cvmx_bgxx_spux_int_s cn73xx;
	struct cvmx_bgxx_spux_int_s cn78xx;
	struct cvmx_bgxx_spux_int_s cn78xxp1;
	struct cvmx_bgxx_spux_int_s cnf75xx;
};

typedef union cvmx_bgxx_spux_int cvmx_bgxx_spux_int_t;

/**
 * cvmx_bgx#_spu#_lpcs_states
 */
union cvmx_bgxx_spux_lpcs_states {
	u64 u64;
	struct cvmx_bgxx_spux_lpcs_states_s {
		u64 reserved_15_63 : 49;
		u64 br_rx_sm : 3;
		u64 reserved_10_11 : 2;
		u64 bx_rx_sm : 2;
		u64 deskew_am_found : 4;
		u64 reserved_3_3 : 1;
		u64 deskew_sm : 3;
	} s;
	struct cvmx_bgxx_spux_lpcs_states_s cn73xx;
	struct cvmx_bgxx_spux_lpcs_states_s cn78xx;
	struct cvmx_bgxx_spux_lpcs_states_s cn78xxp1;
	struct cvmx_bgxx_spux_lpcs_states_s cnf75xx;
};

typedef union cvmx_bgxx_spux_lpcs_states cvmx_bgxx_spux_lpcs_states_t;

/**
 * cvmx_bgx#_spu#_misc_control
 *
 * "* RX logical PCS lane polarity vector <3:0> = [XOR_RXPLRT]<3:0> ^ [4[[RXPLRT]]].
 *  * TX logical PCS lane polarity vector <3:0> = [XOR_TXPLRT]<3:0> ^ [4[[TXPLRT]]].
 *
 *  In short, keep [RXPLRT] and [TXPLRT] cleared, and use [XOR_RXPLRT] and [XOR_TXPLRT] fields to
 *  define
 *  the polarity per logical PCS lane. Only bit 0 of vector is used for 10GBASE-R, and only bits
 * - 1:0 of vector are used for RXAUI."
 */
union cvmx_bgxx_spux_misc_control {
	u64 u64;
	struct cvmx_bgxx_spux_misc_control_s {
		u64 reserved_13_63 : 51;
		u64 rx_packet_dis : 1;
		u64 skip_after_term : 1;
		u64 intlv_rdisp : 1;
		u64 xor_rxplrt : 4;
		u64 xor_txplrt : 4;
		u64 rxplrt : 1;
		u64 txplrt : 1;
	} s;
	struct cvmx_bgxx_spux_misc_control_s cn73xx;
	struct cvmx_bgxx_spux_misc_control_s cn78xx;
	struct cvmx_bgxx_spux_misc_control_s cn78xxp1;
	struct cvmx_bgxx_spux_misc_control_s cnf75xx;
};

typedef union cvmx_bgxx_spux_misc_control cvmx_bgxx_spux_misc_control_t;

/**
 * cvmx_bgx#_spu#_spd_abil
 */
union cvmx_bgxx_spux_spd_abil {
	u64 u64;
	struct cvmx_bgxx_spux_spd_abil_s {
		u64 reserved_4_63 : 60;
		u64 hundredgb : 1;
		u64 fortygb : 1;
		u64 tenpasst : 1;
		u64 tengb : 1;
	} s;
	struct cvmx_bgxx_spux_spd_abil_s cn73xx;
	struct cvmx_bgxx_spux_spd_abil_s cn78xx;
	struct cvmx_bgxx_spux_spd_abil_s cn78xxp1;
	struct cvmx_bgxx_spux_spd_abil_s cnf75xx;
};

typedef union cvmx_bgxx_spux_spd_abil cvmx_bgxx_spux_spd_abil_t;

/**
 * cvmx_bgx#_spu#_status1
 */
union cvmx_bgxx_spux_status1 {
	u64 u64;
	struct cvmx_bgxx_spux_status1_s {
		u64 reserved_8_63 : 56;
		u64 flt : 1;
		u64 reserved_3_6 : 4;
		u64 rcv_lnk : 1;
		u64 lpable : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_bgxx_spux_status1_s cn73xx;
	struct cvmx_bgxx_spux_status1_s cn78xx;
	struct cvmx_bgxx_spux_status1_s cn78xxp1;
	struct cvmx_bgxx_spux_status1_s cnf75xx;
};

typedef union cvmx_bgxx_spux_status1 cvmx_bgxx_spux_status1_t;

/**
 * cvmx_bgx#_spu#_status2
 */
union cvmx_bgxx_spux_status2 {
	u64 u64;
	struct cvmx_bgxx_spux_status2_s {
		u64 reserved_16_63 : 48;
		u64 dev : 2;
		u64 reserved_12_13 : 2;
		u64 xmtflt : 1;
		u64 rcvflt : 1;
		u64 reserved_6_9 : 4;
		u64 hundredgb_r : 1;
		u64 fortygb_r : 1;
		u64 tengb_t : 1;
		u64 tengb_w : 1;
		u64 tengb_x : 1;
		u64 tengb_r : 1;
	} s;
	struct cvmx_bgxx_spux_status2_s cn73xx;
	struct cvmx_bgxx_spux_status2_s cn78xx;
	struct cvmx_bgxx_spux_status2_s cn78xxp1;
	struct cvmx_bgxx_spux_status2_s cnf75xx;
};

typedef union cvmx_bgxx_spux_status2 cvmx_bgxx_spux_status2_t;

/**
 * cvmx_bgx#_spu_bist_status
 *
 * This register provides memory BIST status from the SPU receive buffer lane FIFOs.
 *
 */
union cvmx_bgxx_spu_bist_status {
	u64 u64;
	struct cvmx_bgxx_spu_bist_status_s {
		u64 reserved_4_63 : 60;
		u64 rx_buf_bist_status : 4;
	} s;
	struct cvmx_bgxx_spu_bist_status_s cn73xx;
	struct cvmx_bgxx_spu_bist_status_s cn78xx;
	struct cvmx_bgxx_spu_bist_status_s cn78xxp1;
	struct cvmx_bgxx_spu_bist_status_s cnf75xx;
};

typedef union cvmx_bgxx_spu_bist_status cvmx_bgxx_spu_bist_status_t;

/**
 * cvmx_bgx#_spu_dbg_control
 */
union cvmx_bgxx_spu_dbg_control {
	u64 u64;
	struct cvmx_bgxx_spu_dbg_control_s {
		u64 reserved_56_63 : 8;
		u64 ms_clk_period : 12;
		u64 us_clk_period : 12;
		u64 reserved_31_31 : 1;
		u64 br_ber_mon_dis : 1;
		u64 an_nonce_match_dis : 1;
		u64 timestamp_norm_dis : 1;
		u64 rx_buf_flip_synd : 8;
		u64 br_pmd_train_soft_en : 1;
		u64 an_arb_link_chk_en : 1;
		u64 rx_buf_cor_dis : 1;
		u64 scramble_dis : 1;
		u64 reserved_15_15 : 1;
		u64 marker_rxp : 15;
	} s;
	struct cvmx_bgxx_spu_dbg_control_s cn73xx;
	struct cvmx_bgxx_spu_dbg_control_s cn78xx;
	struct cvmx_bgxx_spu_dbg_control_s cn78xxp1;
	struct cvmx_bgxx_spu_dbg_control_s cnf75xx;
};

typedef union cvmx_bgxx_spu_dbg_control cvmx_bgxx_spu_dbg_control_t;

/**
 * cvmx_bgx#_spu_mem_int
 */
union cvmx_bgxx_spu_mem_int {
	u64 u64;
	struct cvmx_bgxx_spu_mem_int_s {
		u64 reserved_8_63 : 56;
		u64 rx_buf_sbe : 4;
		u64 rx_buf_dbe : 4;
	} s;
	struct cvmx_bgxx_spu_mem_int_s cn73xx;
	struct cvmx_bgxx_spu_mem_int_s cn78xx;
	struct cvmx_bgxx_spu_mem_int_s cn78xxp1;
	struct cvmx_bgxx_spu_mem_int_s cnf75xx;
};

typedef union cvmx_bgxx_spu_mem_int cvmx_bgxx_spu_mem_int_t;

/**
 * cvmx_bgx#_spu_mem_status
 *
 * This register provides memory ECC status from the SPU receive buffer lane FIFOs.
 *
 */
union cvmx_bgxx_spu_mem_status {
	u64 u64;
	struct cvmx_bgxx_spu_mem_status_s {
		u64 reserved_32_63 : 32;
		u64 rx_buf_ecc_synd : 32;
	} s;
	struct cvmx_bgxx_spu_mem_status_s cn73xx;
	struct cvmx_bgxx_spu_mem_status_s cn78xx;
	struct cvmx_bgxx_spu_mem_status_s cn78xxp1;
	struct cvmx_bgxx_spu_mem_status_s cnf75xx;
};

typedef union cvmx_bgxx_spu_mem_status cvmx_bgxx_spu_mem_status_t;

/**
 * cvmx_bgx#_spu_sds#_skew_status
 *
 * This register provides SerDes lane skew status. One register per physical SerDes lane.
 *
 */
union cvmx_bgxx_spu_sdsx_skew_status {
	u64 u64;
	struct cvmx_bgxx_spu_sdsx_skew_status_s {
		u64 reserved_32_63 : 32;
		u64 skew_status : 32;
	} s;
	struct cvmx_bgxx_spu_sdsx_skew_status_s cn73xx;
	struct cvmx_bgxx_spu_sdsx_skew_status_s cn78xx;
	struct cvmx_bgxx_spu_sdsx_skew_status_s cn78xxp1;
	struct cvmx_bgxx_spu_sdsx_skew_status_s cnf75xx;
};

typedef union cvmx_bgxx_spu_sdsx_skew_status cvmx_bgxx_spu_sdsx_skew_status_t;

/**
 * cvmx_bgx#_spu_sds#_states
 *
 * This register provides SerDes lane states. One register per physical SerDes lane.
 *
 */
union cvmx_bgxx_spu_sdsx_states {
	u64 u64;
	struct cvmx_bgxx_spu_sdsx_states_s {
		u64 reserved_52_63 : 12;
		u64 am_lock_invld_cnt : 2;
		u64 am_lock_sm : 2;
		u64 reserved_45_47 : 3;
		u64 train_sm : 3;
		u64 train_code_viol : 1;
		u64 train_frame_lock : 1;
		u64 train_lock_found_1st_marker : 1;
		u64 train_lock_bad_markers : 3;
		u64 reserved_35_35 : 1;
		u64 an_arb_sm : 3;
		u64 an_rx_sm : 2;
		u64 reserved_29_29 : 1;
		u64 fec_block_sync : 1;
		u64 fec_sync_cnt : 4;
		u64 reserved_23_23 : 1;
		u64 br_sh_invld_cnt : 7;
		u64 br_block_lock : 1;
		u64 br_sh_cnt : 11;
		u64 bx_sync_sm : 4;
	} s;
	struct cvmx_bgxx_spu_sdsx_states_s cn73xx;
	struct cvmx_bgxx_spu_sdsx_states_s cn78xx;
	struct cvmx_bgxx_spu_sdsx_states_s cn78xxp1;
	struct cvmx_bgxx_spu_sdsx_states_s cnf75xx;
};

typedef union cvmx_bgxx_spu_sdsx_states cvmx_bgxx_spu_sdsx_states_t;

#endif
