/*
<:copyright-broadcom 
 
 Copyright (c) 2007 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          5300 California Avenue
          Irvine, California 92617 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/***************************************************************************
 * File Name  : XtmProcRegs.h
 *
 * Description: This file contains definitions for the ATM/PTM processor
 *              registers.
 ***************************************************************************/

#if !defined(_XTMPROCREGS_H_)
#define _XTMPROCREGS_H_

/* Miscellaneous values. */
#define XP_MAX_PORTS                        4
#define XP_MAX_CONNS                        16
#define XP_MAX_RX_QUEUES                    4
#define XP_MAX_TX_HDRS                      8
#define XP_TX_HDR_WORDS                     (16 / sizeof(UINT32))
#define XP_RX_MIB_MATCH_ENTRIES             128

/* Circuit types. */
#define XCT_TRANSPARENT                     0x00000001
#define XCT_AAL0_PKT                        0x00000002
#define XCT_AAL0_CELL                       0x00000003
#define XCT_OAM_F5_SEG                      0x00000004
#define XCT_OAM_F5_E2E                      0x00000005
#define XCT_RM                              0x00000006
#define XCT_AAL5                            0x00000007
#define XCT_ASM_P0                          0x00000008
#define XCT_ASM_P1                          0x00000009
#define XCT_ASM_P2                          0x0000000a
#define XCT_ASM_P3                          0x0000000b
#define XCT_OAM_F4_SEG                      0x0000000c
#define XCT_OAM_F4_E2E                      0x0000000d
#define XCT_TEQ                             0x0000000e
#define XCT_PTM                             0x0000000f

/* Definitions for ATM ulTxChannelCfg. */
#define TXCHA_VCID_MASK                     0x0000000f
#define TXCHA_VCID_SHIFT                    0
#define TXCHA_CT_MASK                       0x000000f0
#define TXCHA_CT_SHIFT                      4
#define TXCHA_CT_TRANSPARENT                (XCT_TRANSPARENT << TXCHA_CT_SHIFT)
#define TXCHA_CT_AAL0_PKT                   (XCT_AAL0_PKT    << TXCHA_CT_SHIFT)
#define TXCHA_CT_AAL0_CELL                  (XCT_AAL0_CELL   << TXCHA_CT_SHIFT)
#define TXCHA_CT_OAM_F5_SEG                 (XCT_OAM_F5_SEG  << TXCHA_CT_SHIFT)
#define TXCHA_CT_OAM_F5_E2E                 (XCT_OAM_F5_E2E  << TXCHA_CT_SHIFT)
#define TXCHA_CT_RM                         (XCT_RM          << TXCHA_CT_SHIFT)
#define TXCHA_CT_AAL5                       (XCT_AAL5        << TXCHA_CT_SHIFT)
#define TXCHA_CT_ASM_P0                     (XCT_ASM_P0      << TXCHA_CT_SHIFT)
#define TXCHA_CT_ASM_P1                     (XCT_ASM_P1      << TXCHA_CT_SHIFT)
#define TXCHA_CT_ASM_P2                     (XCT_ASM_P2      << TXCHA_CT_SHIFT)
#define TXCHA_CT_ASM_P3                     (XCT_ASM_P3      << TXCHA_CT_SHIFT)
#define TXCHA_CT_OAM_F4_SEG                 (XCT_OAM_F4_SEG  << TXCHA_CT_SHIFT)
#define TXCHA_CT_OAM_F4_E2E                 (XCT_OAM_F4_E2E  << TXCHA_CT_SHIFT)
#define TXCHA_CT_TEQ                        (XCT_TEQ         << TXCHA_CT_SHIFT)
#define TXCHA_CT_PTM                        (XCT_PTM         << TXCHA_CT_SHIFT)
#define TXCHA_CI                            0x00000100
#define TXCHA_CLP                           0x00000200
#define TXCHA_USE_ALT_GFC                   0x00000400
#define TXCHA_ALT_GFC_MASK                  0x00007800
#define TXCHA_ALT_GFC_SHIFT                 11
#define TXCHA_HDR_EN                        0x00008000
#define TXCHA_HDR_IDX_MASK                  0x00070000
#define TXCHA_HDR_IDX_SHIFT                 16
#define TXCHA_FCS_STRIP                     0x00080000

/* Definitions for PTM ulTxChannelCfg. */
#define TXCHP_FCS_EN                        0x00000001
#define TXCHP_CRC_EN                        0x00000002
#define TXCHP_HDR_EN                        0x00008000
#define TXCHP_HDR_IDX_MASK                  0x00070000
#define TXCHP_HDR_IDX_SHIFT                 16

/* Definitions for ulSwitchPktCfg. */
#define SWP_MAX_PKT_COUNT_MASK              0x00000003
#define SWP_MAX_PKT_COUNT_SHIFT             0
#define SWP_MAX_PKT_SIZE_MASK               0x0000007c
#define SWP_MAX_PKT_SIZE_SHIFT              2
#define SWP_SRC_ID_MASK                     0x00000380
#define SWP_SRC_ID_SHIFT                    7
#define SWP_RX_CHAN_MASK                    0x00000c00
#define SWP_RX_CHAN_SHIFT                   10
#define SWP_TX_CHAN_MASK                    0x0000f000
#define SWP_TX_CHAN_SHIFT                   12
#define SWP_TX_EN                           0x00010000
#define SWP_RX_EN                           0x00020000

/* Definitions for ulSwitchPktTxCtrl. */
#define SWP_VCID_VALUE_MASK                 0x0000000f
#define SWP_VCID_VALUE_SHIFT                0
#define SWP_CT_VALUE_MASK                   0x000000f0
#define SWP_CT_VALUE_SHIFT                  4
#define SWP_FSTAT_CFG_VALUE_MASK            0x00000700
#define SWP_FSTAT_CFG_VALUE_SHIFT           8
#define SWP_MUX_MODE_VALUE                  0x00000800
#define SWP_VCID_MASK_MASK                  0x000f0000
#define SWP_VCID_MASK_SHIFT                 16
#define SWP_CT_MASK_MASK                    0x00f00000
#define SWP_CT_MASK_SHIFT                   20
#define SWP_FSTAT_CFG_MASK_MASK             0x07000000
#define SWP_FSTAT_CFG_MASK_SHIFT            24
#define SWP_MUX_MODE_MASK                   0x08000000

/* Definitions for ulSwitchPktRxCtrl. */
#define SWP_MATCH_MASK                      0x0000007f
#define SWP_MASK_SHIFT                      0
#define SWP_CRC32                           0x00000080
#define SWP_DEST_PORT_MASK                  0x0000ff00
#define SWP_DEST_PORT_SHIFT                 8
#define SWP_MUX_MATCH_MASK                  0x007f0000
#define SWP_MUX_MATCH_SHIFT                 16
#define SWP_MUX_CRC32                       0x00800000
#define SWP_MUX_DESTPORT_MASK               0xff000000
#define SWP_MUX_DESTPORT_SHIFT              24

/* Definitions for ulPktModCtrl. */
#define PKTM_RXQ_EN_MASK                    0x0000000f
#define PKTM_RXQ_EN_SHIFT                   0
#define PKTM_EN                             0x80000000

/* Definitions for ulIrqStatus and ulIrqMask. */
#define INTR_RX_BUF_EMPTY                   0x00000001
#define INTR_TX_BUF_EMPTY                   0x00000002
#define INTR_RX_DMA_NO_DESC_MASK            0x0000003c
#define INTR_RX_DMA_NO_DESC_SHIFT           2
#define INTR_PTM_FRAG_ERROR                 0x00000040
#define INTR_PKT_BUF_UNDERFLOW              0x00000080
#define INTR_TX_DMA_UNDERFLOW               0x00000100
#define INTR_TX_ATM_DC                      0x00000200
#define INTR_BOND_BUF_FULL                  0x00000400
#define INTR_RX_ATM_DC                      0x00000800
#define INTR_MULT_MATCH_ERROR               0x00001000
#define INTR_PKT_BUF_IRQ_MASK               0x0000e000
#define INTR_PKT_BUF_IRQ_SHIFT              13

/* Definitions for ulTxSarCfg. */
#define TXSAR_MODE_ATM                      0x00000000
#define TXSAR_MODE_PTM                      0x00000001
#define TXSARA_BOND_EN                      0x00000002
#define TXSARA_SID12_EN                     0x00000004
#define TXSARA_CRC10_INIT                   0x00000008
#define TXSARA_CRC10_EN_MASK                0x000000f0
#define TXSARA_CRC10_EN_SHIFT               4
#define TXSARA_BOND_DUAL_LATENCY            0x00000100
#define TXSAR_USE_ALT_FSTAT                 0x00000200
#define TXSARP_ENET_FCS_INSERT              0x00000400
#define TXSARP_CRC16_EN                     0x00000800
#define TXSARP_SOF_WHILE_TX                 0x00001000
#define TXSARP_PREEMPT                      0x00002000
#define TXSAR_HDR_INS_OFFSET_MASK           0x0007c000
#define TXSAR_HDR_INS_OFFSET_SHIFT          14
#define TXSARP_NUM_Z_BYTES_MASK             0x00780000
#define TXSARP_NUM_Z_BYTES_SHIFT            19
#define TXSAR_SW_EN                         0x00800000
#define TXSAR_USE_THRESH                    0x01000000
#define TXSAR_PKT_THRESH_MASK               0x06000000
#define TXSAR_PKT_THRESH_SHIFT              25
#define TXSARA_BOND_PORT_DIS_MASK           0x78000000
#define TXSARA_BOND_PORT_DIS_SHIFT          27
#define TXSARA_ASM_CRC_DIS                  0x80000000

/* Definitions for ulTxSchedCfg. */
#define TXSCH_PORT_EN_MASK                  0x0000000f
#define TXSCH_PORT_EN_SHIFT                 0
#define TXSCHA_ALT_SHAPER_MODE              0x00000010
#define TXSCHP_FAST_SCHED                   0x00000020
#define TXSCH_SHAPER_RESET                  0x00000040
#define TXSCH_SIT_COUNT_EN                  0x00000080
#define TXSCH_SIT_COUNT_VALUE_MASK          0x00ffff00
#define TXSCH_SIT_COUNT_VALUE_SHIFT         8
#define TXSCH_SIT_MAX_VALUE                 (TXSCH_SIT_COUNT_VALUE_MASK >> \
                                             TXSCH_SIT_COUNT_VALUE_SHIFT)
#define TXSCH_SOFWT_PRIORITY_EN             0x01000000
#define TXSCH_BASE_COUNT_EN                 0x02000000
#define TXSCH_BASE_COUNT_VALUE_MASK         0x3c000000
#define TXSCH_BASE_COUNT_VALUE_SHIFT        26
#define TXSCHP_USE_BIT4_SOF                 0x40000000
#define TXSCH_ALT_MCR_MODE                  0x80000000

/* Definitions for ulTxOamCfg. */
#define TXOAM_F4_SEG_VPI_MASK               0x000000ff
#define TXOAM_F4_SEG_VPI_SHIFT              0
#define TXOAM_F4_E2E_VPI_MASK               0x0000ff00
#define TXOAM_F4_E2E_VPI_SHIFT              8
#define TXASM_VCI_MASK                      0xffff0000
#define TXASM_VCI_SHIFT                     16

/* Definitions for ulTxMpAalCfg. */
#define TXMP_NUM_GROUPS                     4
#define TXMP_GROUP_SIZE                     5
#define TXMP_GROUP_EN                       0x00000001
#define TXMP_GROUP_SHAPER_MASK              0x0000001e
#define TXMP_GROUP_SHAPER_SHIFT             1
#define TXSOPWT_COUNT_EN                    0x00100000
#define TXSOPWT_COUNT_VALUE_MASK            0x07e00000
#define TXSOPWT_COUNT_VALUE_SHIFT           21

/* Definitions for ulTxUtopiaCfg. */
#define TXUTO_PORT_EN_MASK                  0x0000000f
#define TXUTO_PORT_EN_SHIFT                 0
#define TXUTO_MODE_INT_EXT_MASK             0x00000030
#define TXUTO_MODE_ALL_INT                  0x00000000
#define TXUTO_MODE_ALL_EXT                  0x00000010
#define TXUTO_MODE_INT_EXT                  0x00000020
#define TXUTO_CELL_FIFO_DEPTH_2             0x00000000
#define TXUTO_CELL_FIFO_DEPTH_1             0x00000040
#define TXUTO_NEG_EDGE                      0x00000080
#define TXUTO_LEVEL_1                       0x00000100

/* Definitions for ulTxLineRateTimer. */
#define TXLRT_EN                            0x00000001
#define TXLRT_COUNT_VALUE_MASK              0x0001fffe
#define TXLRT_COUNT_VALUE_SHIFT             1
#define TXLRT_MAX_VALUE                     (TXLRT_COUNT_VALUE_MASK >> \
                                             TXLRT_COUNT_VALUE_SHIFT)
#define TXLRT_IDLE_CELL_INS_MASK            0xf0000000
#define TXLRT_IDLE_CELL_INS_SHIFT           28

/* Definitions for ulRxAtmCfg. */
#define RX_PORT_EN                          0x00000001
#define RX_DOE_MASK                         0x000001fe
#define RX_DOE_SHIFT                        1
#define RX_DOE_GFC_ERROR                    0x00000002
#define RX_DOE_CRC_ERROR                    0x00000004
#define RX_DOE_CT_ERROR                     0x00000008
#define RX_DOE_CAM_LOOKUP_ERROR             0x00000010
#define RX_DOE_IDLE_CELL                    0x00000020
#define RX_DOE_PTI_ERROR                    0x00000040
#define RX_DOE_HEC_ERROR                    0x00000080
#define RX_DOE_PORT_NOT_ENABLED_ERROR       0x00000100
#define RXA_HEC_CRC_IGNORE                  0x00000200
#define RXA_GFC_ERROR_IGNORE                0x00000400
#define RX_PORT_MASK                        0x00001800
#define RX_PORT_MASK_SHIFT                  11
#define RXP_RX_FLOW_DISABLED                0x00004000
#define RXA_VCI_MASK                        0x00008000
#define RXA_VC_BIT_MASK                     0xffff0000
#define RXA_BONDING_VP_MASK					  0x00ff0000

/* Definitions for ulRxSarCfg. */
#define RXSAR_MODE_ATM                      0x00000000
#define RXSAR_MODE_PTM                      0x00000001
#define RXSAR_MODE_MASK                     0x00000001
#define RXSARA_BOND_EN                      0x00000002
#define RXSARA_SID12_EN                     0x00000004
#define RXSARA_CRC10_INIT                   0x00000008
#define RXSARA_CRC10_EN_MASK                0x000000f0
#define RXSARA_CRC10_EN_SHIFT               4
#define RXSARA_BOND_DUAL_LATENCY            0x00000100
#define RXSARA_BOND_CELL_COUNT_MASK         0x07ff0000
#define RXSARA_BOND_CELL_COUNT_SHIFT        16
#define RXSARA_BOND_TIMER_MODE              0x08000000
#define RXSARA_BOND_BUF_MODE_MASK           0x70000000
#define RXSARA_BOND_BUF_MODE_SHIFT          29
#define RXSARA_BOND_BUF_MODE_MASK           0x70000000
#define RXSARA_ASM_CRC_DIS                  0x80000000

/* Definitions for ulRxOamCfg. */
#define RXOAM_F4_SEG_VPI_MASK               0x000000ff
#define RXOAM_F4_SEG_VPI_SHIFT              0
#define RXOAM_F4_E2E_VPI_MASK               0x0000ff00
#define RXOAM_F4_E2E_VPI_SHIFT              8
#define RXASM_VCI_MASK                      0xffff0000
#define RXASM_VCI_SHIFT                     16

/* Definitions for ulRxUtopiaCfg. */
#define RXUTO_PORT_EN_MASK                  0x0000000f
#define RXUTO_PORT_EN_SHIFT                 0
#define RXUTO_TEQ_PORT_MASK                 0x00000070
#define RXUTO_TEQ_PORT_SHIFT                4
#define RXUTO_NEG_EDGE                      0x00000080
#define RXUTO_LEVEL_1                       0x00000100
#define RXUTO_INTERNAL_BUF0_EN              0x00000200
#define RXUTO_EXTERNAL_BUF1_EN              0x00000400

/* Definitions for ulRxAalCfg. */
#define RXAALA_AAL5_SW_TRAILER_EN           0x00000001
#define RXAALA_AAL0_CRC_CHECK               0x00000002
#define RXAALP_CRC32_EN                     0x00000004
#define RXAALP_CELL_LENGTH_MASK             0x000000f0
#define RXAALP_CELL_LENGTH_SHIFT            4

/* Definitions for ulLedCtrl. */
#define SAR_LED_EN                          0x00000001
#define SAR_LED_MODE_MASK                   0x00000006
#define SAR_LED_MODE_SHIFT                  1
#define SAR_LED_MODE_LINK_ONLY              0x00000000
#define SAR_LED_MODE_CELL_ACTIVITY          0x00000002
#define SAR_LED_MODE_MELODY_LINK            0x00000004
#define SAR_LED_MODE_LINK_CELL_ACTIVITY     0x00000006
#define SAR_LED_LINK                        0x00000010
#define SAR_LED_SPEED_MASK                  0x00000060
#define SAR_LED_SPEED_SHIFT                 5
#define SAR_LED_SPEED_30MS                  0x00000000
#define SAR_LED_SPEED_50MS                  0x00000020
#define SAR_LED_SPEED_125MS                 0x00000040
#define SAR_LED_SPEED_250MS                 0x00000060
#define SAR_LED_INTERNAL                    0x00000080

/* Definitions for ulTxVpiVciTable. */
#define TXTBL_VCI_MASK                      0x0000ffff
#define TXTBL_VCI_SHIFT                     0
#define TXTBL_VPI_MASK                      0x00ff0000
#define TXTBL_VPI_SHIFT                     16

/* Definitions for ulRxVpiVciCam - CAM side. */
#define RXCAM_PORT_MASK                     0x00000003
#define RXCAM_PORT_SHIFT                    0
#define RXCAMP_PTM_PRI_LOW                  0x00000000
#define RXCAMP_PTM_PRI_HIGH                 0x00000004
#define RXCAM_TEQ_CELL                      0x00000008
#define RXCAMA_VCI_MASK                     0x000ffff0
#define RXCAMA_VCI_SHIFT                    4
#define RXCAMA_VPI_MASK                     0x0ff00000
#define RXCAMA_VPI_SHIFT                    20
#define RXCAM_VALID                         0x10000000

/* Definitions for ulRxVpiVciCam - RAM side. */
#define RXCAM_CT_MASK                       0x0000000f
#define RXCAM_CT_SHIFT                      0
#define RXCAM_CT_TRANSPARENT                (XCT_TRANSPARENT << RXCAM_CT_SHIFT)
#define RXCAM_CT_AAL0_PKT                   (XCT_AAL0_PKT    << RXCAM_CT_SHIFT)
#define RXCAM_CT_AAL0_CELL                  (XCT_AAL0_CELL   << RXCAM_CT_SHIFT)
#define RXCAM_CT_AAL5                       (XCT_AAL5        << RXCAM_CT_SHIFT)
#define RXCAM_CT_TEQ                        (XCT_TEQ         << RXCAM_CT_SHIFT)
#define RXCAM_CT_PTM                        (XCT_PTM         << RXCAM_CT_SHIFT)
#define RXCAM_VCID_MASK                     0x000001f0
#define RXCAM_VCID_SHIFT                    4
#define RXCAM_STRIP_BYTE_MASK               0x00003e00
#define RXCAM_STRIP_BYTE_SHIFT              9
#define RXCAM_STRIP_EN                      0x00004000
#define RXCAMA_ASM_CELL                     0x00080000
#define RXCAMA_CRC10_EN                     0x00100000

/* Definitions for ulSstCtrl. */
#define SST_EN                              0x00000001
#define SST_MP_GROUP_MASK                   0x00000006
#define SST_MP_GROUP_SHIFT                  1
#define SST_PTM_PREEMPT                     0x00000010
#define SST_PORT_MASK                       0x00000060
#define SST_PORT_SHIFT                      5
#define SST_ALG_MASK                        0x00000380
#define SST_ALG_SHIFT                       7
#define SST_ALG_UBR_NO_PCR                  0x00000000
#define SST_ALG_UBR_PCR                     0x00000080
#define SST_ALG_MBR                         0x00000100
#define SST_ALG_VBR_1                       0x00000180
#define SST_ALG_CBR                         0x00000200
#define SST_SUB_PRIORITY_MASK               0x00001c00
#define SST_SUB_PRIORITY_SHIFT              10
#define SST_SUPER_PRIORITY                  0x00002000
#define SST_MP_EN                           0x00004000
#define SST_MCR_EN                          0x00008000
#define SST_RATE_MCR_MASK                   0xffff0000
#define SST_RATE_MCR_SHIFT                  16

/* Definitions for ulSstPcrScr. */
#define SST_RATE_PCR_MASK                   0x0000ffff
#define SST_RATE_PCR_SHIFT                  0
#define SST_RATE_SCR_MASK                   0xffff0000
#define SST_RATE_SCR_SHIFT                  16

/* Definitions for ulSstBt. */
#define SST_RATE_BT_MASK                    0x00ffffff
#define SST_RATE_BT_SHIFT                   0
#define SST_ALG_WEIGHT_MASK                 0x03000000
#define SST_ALG_WEIGHT_SHIFT                24
#define SST_ALG_DISABLED                    0x00000000
#define SST_ALG_CWRR                        0x01000000
#define SST_ALG_PWRR                        0x02000000
#define SST_ALG_WFQ                         0x03000000
#define SST_WEIGHT_VALUE_MASK               0xfc000000
#define SST_WEIGHT_VALUE_SHIFT              26

/* Definitions for ulSstBucketCnt. */
#define SST_BP_PCR_MASK                     0x0000ffff
#define SST_BP_PCR_SHIFT                    0
#define SST_BM_MCR_MASK                     0xffff0000
#define SST_BM_MCR_SHIFT                    16

/* Definitions for ulRxPktBufCfg. */
#define PBCFG_LENGTH_MASK                   0x0000ffff
#define PBCFG_LENGTH_SHIFT                  0
#define PBCFG_FPM_EMPTY                     0x02000000
#define PBCFG_LD_PTRS                       0x04000000
#define PBCFG_ALLOCATE_LAST                 0x08000000
#define PBCFG_KEEP_ERROR_PKTS               0x10000000
#define PBCFG_FPM_ENABLE                    0x20000000
#define PBCFG_INIT_REQ                      0x40000000
#define PBCFG_S_RESET                       0x80000000

/* Definitions for ulRxPktBufThreshold. */
#define PBTHRESH_MAX_COUNT_MASK             0x000000ff
#define PBTHRESH_MAX_COUNT_SHIFT            0
#define PBTHRESH_NO_BUF_DELAY_MASK          0xffff0000
#define PBTHRESH_NO_BUF_DELAY_SHIFT         16

/* Definitions for ulRxPktBufVcid. */
#define PBVCID_WAIT_EN_MASK                 0x0000ffff
#define PBVCID_WAIT_EN_SHIFT                0

/* Definitions for ulRxPktBufMem. */
#define PBMEM_ADDR_MASK                     0x0000ffff
#define PBMEM_ADDR_SHIFT                    0
#define PBMEM_BS_B_MASK                     0x00ff0000
#define PBMEM_BS_B_SHIFT                    16
#define PBMEM_READ_WRITE_B                  0x40000000
#define PBMEM_GO_BUSY                       0x80000000

/* Definitions for ulRxPktBufPtr. */
#define PBPTR_BUF_STOP_MASK                 0x0000ffff
#define PBPTR_BUF_STOP_SHIFT                0
#define PBPTR_BUF_START_MASK                0xffff0000
#define PBPTR_BUF_START_SHIFT               16

/* Definitions for ulRxPktBufSize. */
#define PBSIZE_THRESHOLD_MASK               0x0000ffff
#define PBSIZE_THRESHOLD_SHIFT              0
#define PBSIZE_MASK                         0xffff0000
#define PBSIZE_SHIFT                        16

/* Definitions for ulRxPktBufFifoStart[2]. */
#define PBSTART_0_MASK                      0x0000ffff
#define PBSTART_0_SHIFT                     0
#define PBSTART_1_MASK                      0xffff0000
#define PBSTART_1_SHIFT                     16

/* Definitions for ulRxPktBufFifoStop[2]. */
#define PBSTOP_0_MASK                       0x0000ffff
#define PBSTOP_0_SHIFT                      0
#define PBSTOP_1_MASK                       0xffff0000
#define PBSTOP_1_SHIFT                      16

/* Definitions for ulRxPktBufFifoDelay[2]. */
#define PBDELAY_0_MASK                      0x0000ffff
#define PBDELAY_0_SHIFT                     0
#define PBDELAY_1_MASK                      0xffff0000
#define PBDELAY_1_SHIFT                     16

/* Definitions for ulRxPktBufQueueStatus. */
#define PBQS_NUM_NO_BUFS_MASK               0x0000ffff
#define PBQS_NUM_NO_BUFS_SHIFT              0
#define PBQS_NUM_QUEUE_FULL_MASK            0xffff0000
#define PBQS_NUM_QUEUE_FULL_SHIFT           16

/* Definitions for ulRxPktBufErrorStatus. */
#define PBERR_NUM_ERROR_FRAGS_MASK          0x0000ffff
#define PBERR_NUM_ERROR_FRAGS_SHIFT         0
#define PBERR_NUM_ERROR_PKTS_MASK           0xffff0000
#define PBERR_NUM_ERROR_PKTS_SHIFT          16

/* Definitions for ulRxPktBufTR. */
#define PBTR_LD_T_PTR_MASK                  0x0000ffff
#define PBTR_LD_T_PTR_SHIFT                 0
#define PBTR_LD_H_PTR_MASK                  0xffff0000
#define PBTR_LD_H_PTR_SHIFT                 16

/* Definitions for ulRxPktBufFPM. */
#define PBFPM_FRAME_BUF_MASK                0x0000ffff
#define PBFPM_FRAME_BUF_SHIFT               0
#define PBFPM_LOAD_COUNT_MASK               0xffff0000
#define PBFPM_LOAD_COUNT_SHIFT              16

/* Definitions for ulRxPktBufMibCtrl. */
#define PBMIB_TOGGLE                        0x40000000
#define PBMIB_CLEAR                         0x80000000

/* Definitions for ulRxPktBufMibMatch. */
#define PBMIB_MATCH_MASK                    0x0000007f

/* Definitions for ulTxHdrInsert. */
#define TXHDR_COUNT_MASK                    0x0000001f
#define TXHDR_COUNT_SHIFT                   0
#define TXHDR_OFFSET_MASK                   0x00ff0000
#define TXHDR_OFFSET_SHIFT                  16

/* Definitions for ulTxRxPortOamCellCnt. */
#define OAM_TX_CELL_COUNT_MASK              0x0000ffff
#define OAM_TX_CELL_COUNT_SHIFT             0
#define OAM_RX_CELL_COUNT_MASK              0xffff0000
#define OAM_RX_CELL_COUNT_SHIFT             16

/* Definitions for ulTxRxPortAsmCellCnt. */
#define ASM_TX_CELL_COUNT_MASK              0x0000ffff
#define ASM_TX_CELL_COUNT_SHIFT             0
#define ASM_RX_CELL_COUNT_MASK              0xffff0000
#define ASM_RX_CELL_COUNT_SHIFT             16

/* Definitions for ulRxPortErrorPktCellCnt. */
#define ERROR_RX_CELL_COUNT_MASK            0x0000ffff
#define ERROR_RX_CELL_COUNT_SHIFT           0
#define ERROR_RX_PKT_COUNT_MASK             0xffff0000
#define ERROR_RX_PKT_COUNT_SHIFT            16


#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362)
#define XTM_PROCESSOR_BASE                                       0xb0007800
#endif
typedef struct XtmProcessorRegisters
{
    UINT32 ulTxChannelCfg[XP_MAX_CONNS];                            /* 0000 */
    UINT32 ulSwitchPktCfg;                                          /* 0040 */
    UINT32 ulSwitchPktTxCtrl;                                       /* 0044 */
    UINT32 ulSwitchPktRxCtrl;                                       /* 0048 */
    UINT32 ulPktModCtrl;                                            /* 004c */
    UINT32 ulIrqStatus;                                             /* 0050 */
    UINT32 ulIrqMask;                                               /* 0054 */
    UINT32 ulReserved1[2];                                          /* 0058 */
    UINT32 ulTxSarCfg;                                              /* 0060 */
    UINT32 ulTxSchedCfg;                                            /* 0064 */
    UINT32 ulTxOamCfg;                                              /* 0068 */
    UINT32 ulTxAtmStatus;                                           /* 006c */
    UINT32 ulTxAalStatus;                                           /* 0070 */
    UINT32 ulTxMpAalCfg;                                            /* 0074 */
    UINT32 ulTxUtopiaCfg;                                           /* 0078 */
    UINT32 ulTxLineRateTimer;                                       /* 007c */
    UINT32 ulRxAtmCfg[XP_MAX_PORTS];                                /* 0080 */
    UINT32 ulRxSarCfg;                                              /* 0090 */
    UINT32 ulRxOamCfg;                                              /* 0094 */
    UINT32 ulRxAtmStatus;                                           /* 0098 */
    UINT32 ulRxUtopiaCfg;                                           /* 009c */
    UINT32 ulRxAalCfg;                                              /* 00a0 */
    UINT32 ulRxAalMaxSdu;                                           /* 00a4 */
    UINT32 ulRxAalStatus;                                           /* 00a8 */
    UINT32 ulLedCtrl;                                               /* 00ac */
    UINT32 ulReserved2[20];                                         /* 00b0 */
    UINT32 ulTxVpiVciTable[XP_MAX_CONNS];                           /* 0100 */
    UINT32 ulRxVpiVciCam[XP_MAX_CONNS * 2];                         /* 0140 */
    UINT32 ulReserved3[16];                                         /* 01c0 */
    UINT32 ulSstCtrl[XP_MAX_CONNS];                                 /* 0200 */
    UINT32 ulSstPcrScr[XP_MAX_CONNS];                               /* 0240 */
    UINT32 ulSstBt[XP_MAX_CONNS];                                   /* 0280 */
    UINT32 ulSstBucketCnt[XP_MAX_CONNS];                            /* 02c0 */
    UINT32 ulRxPktBufCfg;                                           /* 0300 */
    UINT32 ulRxPktBufThreshold;                                     /* 0304 */
    UINT32 ulRxPktBufVcid;                                          /* 0308 */
    UINT32 ulRxPktBufMem;                                           /* 030c */
    UINT32 ulRxPktBufData[2];                                       /* 0310 */
    UINT32 ulRxPktBufPtr;                                           /* 0318 */
    UINT32 ulRxPktBufSize;                                          /* 031c */
    UINT32 ulRxPktBufFifoStart[2];                                  /* 0320 */
    UINT32 ulRxPktBufFifoStop[2];                                   /* 0328 */
    UINT32 ulRxPktBufFifoDelay[2];                                  /* 0330 */
    UINT32 ulRxPktBufQueueStatus;                                   /* 0338 */
    UINT32 ulRxPktBufErrorStatus;                                   /* 033c */
    UINT32 ulRxPktBufTR;                                            /* 0340 */
    UINT32 ulRxPktBufFPM;                                           /* 0344 */
    UINT32 ulRxPktBufMibCtrl;                                       /* 0348 */
    UINT32 ulRxPktBufMibMatch;                                      /* 034c */
    UINT32 ulRxPktBufMibRxOctet;                                    /* 0350 */
    UINT32 ulRxPktBufMibRxPkt;                                      /* 0354 */
    UINT32 ulReserved4[106];                                        /* 0358 */
    UINT32 ulTxHdrInsert[XP_MAX_TX_HDRS];                           /* 0500 */
    UINT32 ulReserved5[24];                                         /* 0520 */
    UINT32 ulTxHdrValues[XP_MAX_TX_HDRS][XP_TX_HDR_WORDS];          /* 0580 */
    UINT32 ulTxPortPktOctCnt[XP_MAX_PORTS];                         /* 0600 */
    UINT32 ulRxPortPktOctCnt[XP_MAX_PORTS];                         /* 0610 */
    UINT32 ulTxPortPktCnt[XP_MAX_PORTS];                            /* 0620 */
    UINT32 ulRxPortPktCnt[XP_MAX_PORTS];                            /* 0630 */
    UINT32 ulTxRxPortOamCellCnt[XP_MAX_PORTS];                      /* 0640 */
    UINT32 ulTxRxPortAsmCellCnt[XP_MAX_PORTS];                      /* 0650 */
    UINT32 ulRxPortErrorPktCellCnt[XP_MAX_PORTS];                   /* 0660 */
    UINT32 ulBondInputCellCnt;                                      /* 0670 */
    UINT32 ulBondOutputCellCnt;                                     /* 0674 */
    UINT32 ulReserved6[1];                                          /* 0678 */
    UINT32 ulMibCtrl;                                               /* 067c */
    UINT32 ulTxVcPktOctCnt[XP_MAX_CONNS];                           /* 0680 */
} XTM_PROCESSOR_REGISTERS, *PXTM_PROCESSOR_REGISTERS;

#define XP_REGS ((volatile PXTM_PROCESSOR_REGISTERS const) XTM_PROCESSOR_BASE)

/* Definitions from pktCmfHw.h.  TBD. Use commoon header file. */
#ifndef PKTCMF_OFFSET_ENGINE_SAR
#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
#define PKTCMF_OFFSET_ENGINE_SAR        0xB0008000
#endif
#endif
#define PKTCMF_OFFSET_RXFILT            0x00001B00
#define RXFILT_REG_MATCH0_DEF_ID        0x0000000C /* 7b/VCID 03..00 */
#define RXFILT_REG_MATCH1_DEF_ID        0x00000010 /* 7b/VCID 07..04 */
#define RXFILT_REG_MATCH2_DEF_ID        0x00000014 /* 7b/VCID 11..08 */
#define RXFILT_REG_MATCH3_DEF_ID        0x00000018 /* 7b/VCID 15..12 */

#define RXFILT_REG_VCID0_QID            0x00000004 /* 2b/VCID 15..00 */
#define RXFILT_REG_VCID1_QID            0x00000008 /* 2b VCID=    16 */

#endif /* _XTMPROCREGS_H_ */

