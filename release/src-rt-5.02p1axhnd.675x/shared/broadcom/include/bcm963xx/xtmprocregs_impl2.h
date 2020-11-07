/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/
/***************************************************************************
 * File Name  : XtmProcRegs_impl2.h
 *
 * Description: This file contains definitions for the next generation ATM/PTM
 *              processor registers. (6326x family)
 ***************************************************************************/

#if !defined(_XTMPROCREGS_IMPL2_H_)
#define _XTMPROCREGS_IMPL2_H_

#include "bcm_map.h"

#if defined(CONFIG_BCM963158)
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
#define SAR_CLOCK    429000000L  /* 429MHz */ /* PLL programming sets to the max value for A0 revision
                                               * 429Mhz is not timing closed
                                               * and not qualified for
                                               * production. */
#else
#define SAR_CLOCK    409000000L  /* 409MHz - default */
#endif
#elif defined(CONFIG_BCM963178)
#define SAR_CLOCK    300000000L /* 300MHz */
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
#define SAR_CLOCK    200000000L  /* 200MHz */
#else /* 63x68 */
#define SAR_CLOCK    125000000L  /* 125MHz */
#endif

/* Miscellaneous values. */
#define XP_MAX_PORTS                        4
#define XP_MAX_CONNS                        16
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define XP_MAX_MP_GROUPS                    8
#else
#define XP_MAX_MP_GROUPS                    4
#endif
#define XP_MAX_MPAAL_Q_IND                  4
#define XP_MAX_RX_QUEUES                    4
#define XP_MAX_PRI_LEVELS                   8
#define XP_MAX_RXPAF_WR_CHANNELS            8
#define XP_MAX_RXBOND_CHANNELS              4
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

/* Definitions for PTM ulTxChannelCfg. */
#define TXCHP_FCS_EN                        0x00000001
#define TXCHP_CRC_EN                        0x00000002
#define TXCHP_HDR_EN                        0x00008000
#define TXCHP_HDR_IDX_MASK                  0x00070000
#define TXCHP_HDR_IDX_SHIFT                 16

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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define INTR_RX_ATM_DC_MASK                 0x00007800
#define INTR_RX_ATM_DC_SHIFT                11
#define INTR_MULT_MATCH_ERROR               0x00008000
#define INTR_PKT_BUF_IRQ_MASK               0x00070000
#define INTR_PKT_BUF_IRQ_SHIFT              16
#else
#define INTR_RX_ATM_DC                      0x00000800
#define INTR_MULT_MATCH_ERROR               0x00001000
#define INTR_PKT_BUF_IRQ_MASK               0x0000e000
#define INTR_PKT_BUF_IRQ_SHIFT              13
#endif

/* Definitions for ulSoftReset */
#define RX_SOFT_RST                         0x00000001
#define TX_SOFT_RST                         0x00000002

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
#define TXSARP_USE_ALT_PTM_CRC              0x00004000
#define TXSARP_ENABLE_TX_AIOOS              0x00008000
#define TXSARP_ENABLE_TX_SPKT               0x00010000
#define TXSARA_ENABLE_GCRA                  0x00020000
#define TXSARP_ENABLE_RAW_PTM_MODE          0x00040000
#define TXSARP_SEL_TXBBH                    0x00800000
#define TXSARA_BOND_PORT_DIS_MASK           0x78000000
#define TXSARA_BOND_PORT_DIS_SHIFT          27
#define TXSARA_ASM_CRC_DIS                  0x80000000

/* Definitions for ulTxSchedCfg. */
#define TXSCH_PORT_EN_MASK                  0x0000000f
#define TXSCH_PORT_EN_SHIFT                 0
#define TXSCH_SHAPER_RESET                  0x00000040
#define TXSCH_SIT_COUNT_EN                  0x00000080
#define TXSCH_SITLO_COUNT_EN                0x00000100
#define TXSCH_SLR_COUNT_EN                  0x00000200
#define TXSCH_SLR_EN                        0x00000400
#if defined(CONFIG_BCM963158)
#if (CONFIG_BRCM_CHIP_REV != 0x63158A0)
#define TXSCH_GFAST_MPAAL_SEL_DEFAULT       0
#define TXSCH_GFAST_MPAAL_SEL_2             1
#define TXSCH_GFAST_MPAAL_SEL_1             2
#define TXSCH_GFAST_MPAAL_SEL_4             3
#define TXSCH_GFAST_MPAAL_SEL_MASK          0x00001800
#define TXSCH_GFAST_MPAAL_SEL_SHIFT         11
#endif
#endif
#define TXSCH_SOFWT_PRIORITY_EN             0x01000000
#define TXSCH_VC_GTS_EN                     0x02000000
#define TXSCH_MP_GTS_EN                     0x04000000

/* Definitions for ulTxOamCfg. */
#define TXOAM_F4_SEG_VPI_MASK               0x000000ff
#define TXOAM_F4_SEG_VPI_SHIFT              0
#define TXOAM_F4_E2E_VPI_MASK               0x0000ff00
#define TXOAM_F4_E2E_VPI_SHIFT              8
#define TXASM_VCI_MASK                      0xffff0000
#define TXASM_VCI_SHIFT                     16

/* Definitions for ulTxMpCfg. */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define ATM_TXMP_NUM_GROUPS                 8
#define PTM_TXMP_NUM_GROUPS                 4
#define TXMP_GROUP_SIZE                     1
#define TXMP_GROUP_EN                       0x00000001
#else
#define TXMP_NUM_GROUPS                     4
#define TXMP_GROUP_SIZE                     5
#define TXMP_GROUP_EN                       0x00000001
#endif

/* Definitions for ulTxUtopiaCfg. */
#define TXUTO_EXT_PORT_EN_MASK              0x0000000f
#define TXUTO_EXT_PORT_EN_SHIFT             0
#define TXUTO_INT_PORT_EN_MASK              0x000000f0
#define TXUTO_INT_PORT_EN_SHIFT             4
#define TXUTO_SLV_PORT_EN_MASK              0x00000f00
#define TXUTO_SLV_PORT_EN_SHIFT             8
#define TXUTO_EXT_RAW_PID_EN_MASK           0x0000F000
#define TXUTO_EXT_RAW_PID_EN_SHIFT          12
#define TXUTO_INT_RAW_PID_EN_MASK           0x000f0000
#define TXUTO_INT_RAW_PID_EN_SHIFT          16
#define TXUTO_CELL_FIFO_DEPTH_2             0x00000000
#define TXUTO_CELL_FIFO_DEPTH_1             0x00400000
#define TXUTO_LEVEL_1                       0x00800000

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
#define RXA_BONDING_VP_MASK		    0x00ff0000

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
#define RXSARA_SEL_RXBBH                    0x00000400
#define RXSARA_ASM_CRC_DIS                  0x80000000

/* Definitions for ulRxOamCfg. */
#define RXOAM_F4_SEG_VPI_MASK               0x000000ff
#define RXOAM_F4_SEG_VPI_SHIFT              0
#define RXOAM_F4_E2E_VPI_MASK               0x0000ff00
#define RXOAM_F4_E2E_VPI_SHIFT              8
#define RXASM_VCI_MASK                      0xffff0000
#define RXASM_VCI_SHIFT                     16

/* Definitions for ulRxUtopiaCfg. */
#define RXUTO_EXT_IN_PORT_EN_MASK           0x0000000f
#define RXUTO_EXT_IN_PORT_EN_SHIFT          0
#define RXUTO_EXT_OUT_PORT_EN_MASK          0x000000f0
#define RXUTO_EXT_OUT_PORT_EN_SHIFT         4
#define RXUTO_INT_PORT_EN_MASK              0x00000f00
#define RXUTO_INT_PORT_EN_SHIFT             8
#define RXUTO_EXT_TEQ_PORT_MASK             0x0000f000
#define RXUTO_EXT_TEQ_PORT_SHIFT            12
#define RXUTO_INT_TEQ_PORT_MASK             0x000f0000
#define RXUTO_INT_TEQ_PORT_SHIFT            16
#define RXUTO_LEVEL_1                       0x00100000
#define RXUTO_INTERNAL_BUF0_EN              0x00200000
#define RXUTO_EXTERNAL_BUF1_EN              0x00400000

/* Definitions for ulRxAalCfg. */
#define RXAALA_AAL5_SW_TRAILER_EN           0x00000001
#define RXAALA_AAL0_CRC_CHECK               0x00000002

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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define SAR_LED_INTERNAL                    0x00000008
#define SAR_LED_TEST                        0x00000100
#else
#define SAR_LED_INTERNAL                    0x00000080
#endif

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

/* Definitions for ulSstMpGtsCfg and ulSsteQueueGtsCfg. */
#define SST_GTS_PCR_EN                      0x00000001
#define SST_GTS_SITLO_PCR_EN                0x00000002
#define SST_GTS_SCR_EN                      0x00000004
#define SST_GTS_SITLO_SCR_EN                0x00000008
#define SST_GTS_VBR_MODE_EN                 0x00000010
#define SST_GTS_PCR_PRI_LEVEL_EN            0x00000020
#define SST_GTS_PKT_MODE_SHAPING_EN         0x00000040
#define SST_GTS_PKT_MODE_EIRPIR_CIR         0x00000080

/* Definitions for ulSsForceVcArbRR. */
#define SST_VCRR_INDEX_MASK                 0xff
#define SST_VCRR_INDEX_SHIFT                0
#define SST_STALL_GTS                       0x20000000
#define SST_ZERO_GTS                        0x40000000
#define SST_USE_FIFO_EMPTY_ONLY             0x80000000

/* Definition for ulSsNewCfgD0_or_Reserved5 */
#define SHPR_NEWCFGD0_EN24HDR               0x40000000

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

/* Definitions for ulRxPBufMuxVcidDefQueueId[2] */
#define RXPBUF_MUXVCID_DEF_QID_MASK         0x3

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

/* Definitions for ulDiagCtrl. */
#define DIAG_BANK_SEL_MASK                  0x00000003
#define DIAG_BANK_SEL_SHIFT                 0x00000000
#define DIAG_SAR_LBPK_EN                    0x00000010

/* Definitions for ulRxPafConfig. */
#define RP_TC_CRC_EN                        0x00000001
#define RP_TC_CRC16                         0x00000002
#define RP_TC_CRC_DROP_FRAG                 0x00000004
#define RP_TC_CRC_DROP_END_BYTE             0x00000008
#define RP_TRANSPARENT_PTM_MODE             0x00000010
#define RP_SHORT_FRAME_EN                   0x00000020
#define RP_ENET_CRC_EN                      0x00000040
#define RP_ENET_CRC_DROP_PKT                0x00000080
#define RP_TC_CRC_BITREV_INPUT              0x00000100
#define RP_TC_CRC_ONEC_OUTPUT               0x00000200
#define RP_TC_CRC_ENDIAN_SWAP               0x00000400
#define RP_TC_CRC_BITREV_OUTPUT             0x00000800
#define RP_TINY_FRAME_EN                    0x00001000
#define RP_DROP_PTM_IDLE                    0x00002000
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM63381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define RP_DROP_PTM_SOF                     0x00004000
#endif
#define RP_DROP_PKT_END_BYTE_MASK           0x007f0000
#define RP_DROP_PKT_END_BYTE_SHIFT          16

/* Definitions for ulRxPafStatus. */
#define RP_STATUS_PTMDEC_INCELL             0x80000000
#define RP_STATUS_PTMDEC_ALLDATA            0x40000000
#define RP_STATUS_PTMDEC_INFRAME            0x20000000
#define RP_STATUS_PTMDEC_INITB              0x10000000
#define RP_STATUS_PTMDEC_FLUSH              0x08000000
#define RP_STATUS_RDSTATE_REG_MASK          0x07000000
#define RP_STATUS_RDSTATE_REG_SHIFT         24
#define RP_STATUS_INBUF_BONDING_STALL       0x00800000
#define RP_STATUS_OUTBUF_BONDING_READY      0x00400000
#define RP_STATUS_INBUF_RDSTATE_STALL       0x00200000
#define RP_STATUS_INBUF_RDSTATE_READY       0x00100000
#define RP_STATUS_RXATM2PAF_RDY             0x00080000
#define RP_STATUS_RXPAF_RDY                 0x00040000
#define RP_STATUS_OUT_FIFO_EMPTY            0x00020000
#define RP_STATUS_OUT_FIFO_FULL             0x00010000
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM63381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define RP_STATUS_WRCHN_FLUSH_FULL_WAIT     0x00008000
#define RP_STATUS_WRCHN_FLUSH_INBUF_WAIT    0x00004000
#endif
#define RP_STATUS_WRCHN_FLUSH_ACTIVE        0x00002000
#define RP_STATUS_WRCHN_FLUSH_PENDING       0x00001000
#define RP_STATUS_WRCHN_FLUSH_BUSY_MASK     (RP_STATUS_WRCHN_FLUSH_ACTIVE|RP_STATUS_WRCHN_FLUSH_PENDING)
#define RP_STATUS_INBUF_PID_MASK            0x0000C000
#define RP_STATUS_INBUF_PID_SHIFT           10
#define RP_STATUS_INBUF_RD_PTM_BOND         0x00000200
#define RP_STATUS_INBUF_RD_ATM_BOND         0x00000100
#define RP_STATUS_INBUF_RD_PTM              0x00000080
#define RP_STATUS_INBUF_RD_ATM              0x00000040
#define RP_STATUS_INBUF_RD_TRANSPARENT      0x00000020
#define RP_STATUS_INBUF_RD_RAW              0x00000010
#define RP_STATUS_OFIFO_STATE_WAIT          0x00000008
#define RP_STATUS_INBUF_FULL                0x00000004
#define RP_STATUS_INBUF_EMPTY               0x00000002
#define RP_STATUS_RXPAF2RXATM_RDY           0x00000001

/* Values of Frag Sizes */
#define RP_DEF_MIN_FRAG_SIZE                0x40
#define RP_DEF_MAX_FRAG_SIZE                0xFFFF
#define RP_BONDING_6465B_MIN_FRAG_SIZE      64
#define RP_BONDING_6465B_MAX_FRAG_SIZE      516

/* Definitions for ulRxPafFragSize. */
#define RP_MAX_FRAGMENT_SIZE_MASK           0x0000ffff
#define RP_MAX_FRAGMENT_SIZE_SHIFT          0
#define RP_MIN_FRAGMENT_SIZE_MASK           0x00ff0000
#define RP_MIN_FRAGMENT_SIZE_SHIFT          16

#define RP_MIN_FRAG_PKT_SIZE                64
#define RP_MAX_FRAG_PKT_SIZE                0x640
#define RP_MAX_JUMBO_FRAG_PKT_SIZE          0x816

/* Definitions for ulRxPafPktSize. */
#define RP_MAX_PACKET_SIZE_MASK             0x0000ffff
#define RP_MAX_PACKET_SIZE_SHIFT            0
#define RP_MIN_PACKET_SIZE_MASK             0x00ff0000
#define RP_MIN_PACKET_SIZE_SHIFT            16
#define RP_TRANSPARENT_CELL_COUNT_MASK      0xff000000
#define RP_TRANSPARENT_CELL_COUNT_SHIFT     24

/* Definitions for ulRxPafLinkState. */
#define RP_LINK_STATE_MASK                  0x0000000f
#define RP_LINK_STATE_SHIFT                 0
#define RP_LINK_DISABLE_MASK                0x000000f0
#define RP_LINK_DISABLE_SHIFT               4

/* Definitions for ulRxPafTestMode0. */
#define RP_TEST_MODE0_INPUT_DELAY_COUNT_MASK   0x00FF0000
#define RP_TEST_MODE0_INPUT_DELAY_COUNT_SHIFT  16
#define RP_TEST_MODE0_OUTPUT_DELAY_COUNT_MASK   0x0000FF00
#define RP_TEST_MODE0_OUTPUT_DELAY_COUNT_SHIFT  8

/* Definitions for ulRxBondConfig. */
#define RB_CELL_FULL_WAIT_MASK              0x000000ff
#define RB_CELL_FULL_WAIT_SHIFT             0
#define RB_TIMER_ENABLE_MASK                0x0000ff00
#define RB_TIMER_ENABLE_SHIFT               8
#define RB_WRCHN_TIMEOUT_FLUSH              0x00010000
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM63381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define RB_WRCHN_TIMEOUT_FLUSHX             0x00020000
#endif

/* Definitions for ulRxBondErrorStatus. */
#define RB_ERR_STATUS_PAF_BAD_FRAG           0x00000040
#define RB_ERR_STATUS_PAF_LOST_FRAG_BOTH_SID 0x00000004

/* Definitions for ulRxBondBufInit. */
#define RB_PAFBUF_INIT_CNT_MASK             0x00000fff
#define RB_PAFBUF_INIT_CNT_SHIFT            0
#define RB_PAFBUF_INIT_ADDR_MASK            0xffff0000
#define RB_PAFBUF_INIT_ADDR_SHIFT           16

/* Definitions for ulRxBondPafBufAddr. */
#define RB_PAFBUF_ADDR_MASK                 0x0000ffff
#define RB_PAFBUF_ADDR_SHIFT                0
#define RB_PAFBUF_CTRL                      0x00010000

/* Definitions for ulRxBondReclaim. */
#define RB_RECLAIM_RD_ADDR_MASK             0x0000ffff
#define RB_RECLAIM_RD_ADDR_SHIFT            0
#define RB_RECLAIM_WR_ADDR_MASK             0xffff0000
#define RB_RECLAIM_WR_ADDR_SHIFT            16

#define RXBOND_CLK_DIVIDER                  200  /* 200Mhz */

/* Definitions for ulRxBondCellFull, ulRxBondTimeout */
#define RXBOND_CELL_FULL_LIST_MAX           0x860
#define RXBOND_MAX_TIMEOUT_PER_WRCHN        0xff
#define RXBOND_CELL_FULL_WR_CHAN_SHIFT      16
#define RXBOND_TIMEOUT_WR_CHAN_SHIFT        8
#define RXBOND_DEF_TIMEOUT_PER_WRCHN        0x40

/* Definitions for ulSstMpPriArb. */
#define SST_MP_ARB_ALG_PLVLN_MASK           0x000000ff

/* Definitions for ulSsteMpScr and ulSsteQueueScr. */
#define SSTE_SCR_PLVLWT_MASK                0x0000000f
#define SSTE_SCR_PLVLWT_SHIFT               0
#define SSTE_SCR_INCR_MASK                  0x7fff0000
#define SSTE_SCR_INCR_SHIFT                 16

#define SSTE_SCR_CURACC_MASK                0x00007fff
#define SSTE_SCR_CURACC_SHIFT               0
#define SSTE_SCR_ACCLMT_MASK                0x7fff0000
#define SSTE_SCR_ACCLMT_SHIFT               16

#define SSTE_SCR_CURSIT_MASK                0x00007fff
#define SSTE_SCR_CURSIT_SHIFT               0
#define SSTE_SCR_SITLMT_MASK                0x7fff0000
#define SSTE_SCR_SITLMT_SHIFT               16

/* Definitions for ulSsteMpPcr and ulSsteQueuePcr. */
#define SSTE_PCR_PLVLWT_MASK                0x0000000f
#define SSTE_PCR_PLVLWT_SHIFT               0
#define SSTE_PCR_INCR_MASK                  0x7fff0000
#define SSTE_PCR_INCR_SHIFT                 16

#define SSTE_PCR_CURACC_MASK                0x00007fff
#define SSTE_PCR_CURACC_SHIFT               0
#define SSTE_PCR_ACCLMT_MASK                0x7fff0000
#define SSTE_PCR_ACCLMT_SHIFT               16

#define SSTE_PCR_CURSIT_MASK                0x00007fff
#define SSTE_PCR_CURSIT_SHIFT               0
#define SSTE_PCR_SITLMT_MASK                0x7fff0000
#define SSTE_PCR_SITLMT_SHIFT               16

/* Definitions for ulSsteMpVcArbWt and ulSsteQueueArbWt. */
#define SSTE_ARBWT_MASK                     0x000001ff
#define SSTE_ARBWT_SHIFT                    0
#define SSTE_SETARBWT_MASK                  0x0003fe00
#define SSTE_SETARBWT_SHIFT                 9
#define SSTE_UTPT_MASK                      0xc0000000
#define SSTE_UTPT_SHIFT                     30

/* Definitions for ulSsteQueuePriLevel. */
#define SSTE_LASTPTR_MASK                   0x0000000f
#define SSTE_LASTPTR_SHIFT                  0
#define SSTE_GT0VEC_MASK                    0x000ffff0
#define SSTE_GT0VEC_SHIFT                   4

/* Definitions for ulSsteQueueMpArb. */
#define SSTE_SENTAMT_MASK                   0x00007fff
#define SSTE_SENTAMT_SHIFT                  0
#define SSTE_ALLOCAMT_MASK                  0x7fff0000
#define SSTE_ALLOCAMT_SHIFT                 16

#define SSTE_SURPLUSCNT_MASK                0x00007fff
#define SSTE_SURPLUSCNT_SHIFT               0

/* Definitions for ulSsteUtoPortPcrW0. */
#define SSTE_UTOPCRMW_PCR_PLVLWT_MASK       0x0000000f
#define SSTE_UTOPCRMW_PCR_PLVLWT_SHIFT      0

#define SSTE_UTOPCRMW_PCR_INCR_MASK         0x7FFF0000
#define SSTE_UTOPCRMW_PCR_INCR_SHIFT        16

/* Definitions for ulSsteUtoPortPcrW1. */
#define SSTE_UTOPCRMW_PCR_CURACC_MASK       0x00007FFF
#define SSTE_UTOPCRMW_PCR_CURACC_SHIFT      0

#define SSTE_UTOPCRMW_PCR_ACCLMT_MASK       0x7FFF0000
#define SSTE_UTOPCRMW_PCR_ACCLMT_SHIFT      16

/* Definitions for ulSsteUtoPortPcrW2. */
#define SSTE_UTOPCRMW_PCR_CURSIT_MASK       0x00007FFF
#define SSTE_UTOPCRMW_PCR_CURSIT_SHIFT      0

#define SSTE_UTOPCRMW_PCR_SITLMT_MASK       0x7FFF0000
#define SSTE_UTOPCRMW_PCR_SITLMT_SHIFT      16

/* Definitions for ulSsteUtoGtsCfg. */
#define SSTE_UTOGTSCFG_MASK                 0x000000FF
#define SSTE_UTOGTSCFG_SHIFT                0

/* Definitions for ulSsteMpPriLevel. */
#define SSTE_LASTIDX_MP_MASK                0x0000000f
#define SSTE_LASTIDX_MP_SHIFT               0
#define SSTE_GT0VEC_MP_MASK                 0xffff0000
#define SSTE_GT0VEC_MP_SHIFT                16

#define SSTE_PMASKSC_MASK                   0x00007fff
#define SSTE_PMASKSC_SHIFT                  0
#define SSTE_ACTVLST_MP_MASK                0xffff0000
#define SSTE_ACTVLST_MP_SHIFT               16

#define RXPAF_INVALID_VCID                  0xf
#define RXPAF_VCID_MASK                     0xf
#define RXPAF_VCID_PREEMPTION_SHIFT         16
#define RXPAF_VCID_NON_PREEMPTION_SHIFT     0
#define RXPAF_VCID_SHIFT                    4

/* Definitions for ulSstMpQueueMask */
#define SST_MPG_QUEUE_MASK                  0xFFFF
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define SST_MPG_PER_QUEUE_MASK_REG          2
#else
#define SST_MPG_PER_QUEUE_MASK_REG          1
#endif
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define SST_SECOND_MPG_QUEUE_MASK_SHIFT     16
#else
#define SST_SECOND_MPG_QUEUE_MASK_SHIFT     0
#endif

/* Definitions ulSstMpPriArb */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define NUM_MPN_PLVL_ARB_CONFIG_REG         2
#else
#define NUM_MPN_PLVL_ARB_CONFIG_REG         1 
#endif
#define MPN_PLVL_ARB_PLVL_MASK              0xFF
#define NUM_MPG_PER_ARB_CONFIG_REG          4
#define MPN_PLVL_ARB_MASK_SHIFT             8

/* Definitions ulSsForceMpArbRRxx */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define NUM_MPN_ARB_FORCE_RR_CONFIG_REG         2
#else
#define NUM_MPN_ARB_FORCE_RR_CONFIG_REG         1 
#endif
#define MPN_ARB_FORCE_RR_MASK                   0xFF
#define NUM_MPG_PER_ARB_FORCE_RR_REG            4
#define MPN_ARB_FORCE_RR_MASK_SHIFT             8

#define SST_UTOPIA_CFG_ENABLE_PREEMPTION             0x00000040
#define SST_UTOPIA_CFG_PKTBUFQINDEX_PREEMPT_MASK     0x00000038
#define SST_UTOPIA_CFG_PKTBUFQINDEX_NON_PREEMPT_MASK 0x00000007

#define RXBOND_SIDOUT_CNT_DISABLE_UPDATE    0xF
#define RXBOND_SIDOUT_CNT_ENABLE_UPDATE     0x0

#define XTM_PROCESSOR_BASE    SAR_BASE

typedef struct XtmProcessorRegisters
{
    UINT32 ulTxChannelCfg[XP_MAX_CONNS];                            /* 0000 */
    UINT32 ulReserved1[4];                                          /* 0040 */
    UINT32 ulIrqStatus;                                             /* 0050 */
    UINT32 ulIrqMask;                                               /* 0054 */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulSoftReset;                                             /* 0058 */
    UINT32 ulTxPacBufSpacePadding;                                  /* 005C */
#else
    UINT32 ulReserved2[2];                                          /* 0058 */
#endif
    UINT32 ulTxSarCfg;                                              /* 0060 */
    UINT32 ulTxSchedCfg;                                            /* 0064 */
    UINT32 ulTxOamCfg;                                              /* 0068 */
    UINT32 ulTxCtlStatus1;                                          /* 006c */
    UINT32 ulTxCtlStatus2;                                          /* 0070 */
    UINT32 ulTxMpCfg;                                               /* 0074 */
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulRxAalClenCrcErrCnt;                                    /* 00b0 */
    UINT32 ulRxAalMaxSduErrCnt;                                     /* 00b4 */
    UINT32 ulRxAtmErrCellVcid0;                                     /* 00b8 */
    UINT32 ulRxAtmErrCellVcid1;                                     /* 00bc */
    UINT32 ulReserved3[12];                                         /* 00c0 */
    UINT32 ulUbusCfg;                                               /* 00f0 */
    UINT32 ulUbusReqEop;                                            /* 00f4 */
    UINT32 ulReserved4[2];                                          /* 00f8 */
#else
    UINT32 ulReserved3[20];                                         /* 00b0 */
#endif
    UINT32 ulTxVpiVciTable[XP_MAX_CONNS];                           /* 0100 */
    UINT32 ulRxVpiVciCam[XP_MAX_CONNS * 2];                         /* 0140 */
    UINT32 ulReserved5[16];                                         /* 01c0 */
    UINT32 ulSstMpPriArb[NUM_MPN_PLVL_ARB_CONFIG_REG];              /* 0200 */
    UINT32 ulSstVcPriArb;                                           /* 0204  / 0208 */
    UINT32 ulSstLinkDownCount[XP_MAX_PORTS];                        /* 0208  / 020C */
    UINT32 ulSstUtopiaCfg;                                          /* 0218  / 021C */
    UINT32 ulSstRemapUtopiaCfg;                                     /* 021c  / 0220 */
    UINT32 ulSsPtmBondedPair;                                       /* 0220  / 0224 */
    UINT32 ulSstDiagDMode;                                          /* 0224  / 0228 */
    UINT32 ulSstSitValue;                                           /* 0228  / 022C */
    UINT32 ulSstSitSlowValue;                                       /* 022c  / 0230 */
    UINT32 ulSstMpQueueMask[XP_MAX_MPAAL_Q_IND];                    /* 0230  / 0234 */
    UINT32 ulSsMpGtsCfg[XP_MAX_MP_GROUPS];                          /* 0240  / 0244 */
    UINT32 ulSsTxDmaEmpty;                                          /* 0250  / 0264 */
    UINT32 ulSsPBufEmpty;                                           /* 0254  / 0268 */
    UINT32 ulSsTxToRxPtmLinkStateMap;                               /* 0258  / 026C */
    UINT32 ulSsForceVcArbRR;                                        /* 025c  / 0270 */
    UINT32 ulSsForceMpArbRR[NUM_MPN_ARB_FORCE_RR_CONFIG_REG];       /*       / 0274 */
    UINT32 ulSsDiagAdrMode;                                         /* 0264  / 027C */
    UINT32 ulSsNewCfgD0_or_Reserved6;                               /* 0268  / 0280 */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulReserved7[31];                                         /*       / 0284 */
#else
    UINT32 ulReserved7[37];                                         /* 026C */
#endif
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
    UINT32 ulRxPktBufQueueFull;                                     /* 0338 */
    UINT32 ulRxPktBufNoBuf;                                         /* 033c */
    UINT32 ulRxPktBufErrorPkt;                                      /* 0340 */
    UINT32 ulRxPktBufErrorFrag;                                     /* 0344 */
    UINT32 ulRxPktBufTR;                                            /* 0348 */
    UINT32 ulRxPktBufFPM;                                           /* 034c */
    UINT32 ulRxPktBufMibCtrl;                                       /* 0350 */
    UINT32 ulRxPktBufMibMatch;                                      /* 0354 */
    UINT32 ulRxPktBufMibRxOctet;                                    /* 0358 */
    UINT32 ulRxPktBufMibRxPkt;                                      /* 035c */
    UINT32 ulReserved8[40];                                         /* 0360 */
    UINT32 ulRxPBufMuxVcidDefQueueId[2];                            /* 0400 */
    UINT32 ulRxPBufMuxMatch0DefQueueId;                             /* 0408 */
    UINT32 ulRxPBufMuxMatch1DefQueueId;                             /* 040c */
    UINT32 ulRxPBufMuxMatch2DefQueueId;                             /* 0410 */
    UINT32 ulRxPBufMuxMatch3DefQueueId;                             /* 0414 */
    UINT32 ulPBufMuxltMatch4DefQueueId;                             /* 0418 */
    UINT32 ulReserved9[57];                                         /* 041c */
    UINT32 ulTxHdrInsert[XP_MAX_TX_HDRS];                           /* 0500 */
    UINT32 ulTxHdrTxDatapathCfg;                                    /* 0520 */
    UINT32 ulReserved10[23];                                        /* 0524 */
    UINT32 ulTxHdrValues[XP_MAX_TX_HDRS][XP_TX_HDR_WORDS];          /* 0580 */
    UINT32 ulTxPortPktOctCnt[XP_MAX_PORTS];                         /* 0600 */
    UINT32 ulRxPortPktOctCnt[XP_MAX_PORTS];                         /* 0610 */
    UINT32 ulTxPortPktCnt[XP_MAX_PORTS];                            /* 0620 */
    UINT32 ulRxPortPktCnt[XP_MAX_PORTS];                            /* 0630 */
    UINT32 ulTxRxPortOamCellCnt[XP_MAX_PORTS];                      /* 0640 */
    UINT32 ulTxRxPortAsmCellCnt[XP_MAX_PORTS];                      /* 0650 */
    UINT32 ulRxPortErrorPktCellCnt[XP_MAX_PORTS];                   /* 0660 */
    UINT32 ulBondInputCellCnt;                                      /* 0670 */ /* Not defined in 158*/
    UINT32 ulBondOutputCellCnt;                                     /* 0674 */
    UINT32 ulReserved11;                                            /* 0678 */
    UINT32 ulMibCtrl;                                               /* 067c */
    UINT32 ulTxVcPktOctCnt[XP_MAX_CONNS];                           /* 0680 */
    UINT32 ulReserved12[16];                                        /* 06c0 */
    UINT32 ulDiagCtrl;                                              /* 0700 */
    UINT32 ulReserved13[63];                                        /* 0704 */
    UINT32 ulRxPafConfig;                                           /* 0800 */
    UINT32 ulRxPafStatus;                                           /* 0804 */
    UINT32 ulRxPafErrorStatus0;                                     /* 0808 */
    UINT32 ulRxPafVcid;                                             /* 080c */
    UINT32 ulRxPafFragSize;                                         /* 0810 */
    UINT32 ulRxPafPktSize;                                          /* 0814 */
    UINT32 ulRxPafLinkState;                                        /* 0818 */
    UINT32 ulRxPafWriteChanFlush;                                   /* 081c */
    UINT32 ulRxPafErrorStatus1;                                     /* 0820 */
    UINT32 ulRxPafErrorStatus2;                                     /* 0824 */
    UINT32 ulRxPafTestMode0;                                        /* 0828 */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulRxPafTestMode1;                                        /* 082c */
    UINT32 ulRxPafErrorStatus0_NC;                                  /* 0830 */
    UINT32 ulRxPafErrorStatus1_NC;                                  /* 0834 */
    UINT32 ulRxPafErrorStatus2_NC;                                  /* 0838 */
    UINT32 ulReserved14;                                            /* 083c */
#else
    UINT32 ulReserved14[5];                                         /* 082c */
#endif
    UINT32 ulRxPafCellCount[XP_MAX_RXPAF_WR_CHANNELS];              /* 0840 */
    UINT32 ulRxPafDroppedCellCount[XP_MAX_RXPAF_WR_CHANNELS];       /* 0860 */
    UINT32 ulRxPafFragCount[XP_MAX_RXPAF_WR_CHANNELS];              /* 0880 */
    UINT32 ulRxPafDroppedFragCount[XP_MAX_RXPAF_WR_CHANNELS];       /* 08a0 */
    UINT32 ulRxPafPktCount[XP_MAX_RXPAF_WR_CHANNELS];               /* 08c0 */
    UINT32 ulRxPafDroppedPktCount[XP_MAX_RXPAF_WR_CHANNELS];        /* 08e0 */
    UINT32 ulRxBondConfig;                                          /* 0900 */
    UINT32 ulRxBondStatus;                                          /* 0904 */
    UINT32 ulRxBondErrorStatus;                                     /* 0908 */
    UINT32 ulRxBondWriteChanToReadChan;                             /* 090c */
    UINT32 ulRxBondBufInit;                                         /* 0910 */
    UINT32 ulRxBondPafBufAddr;                                      /* 0914 */
    UINT32 ulRxBondPafBufData;                                      /* 0918 */
    UINT32 ulRxBondReclaim;                                         /* 091c */
    UINT32 ulRxBondStatus1;                                         /* 0920 */
    UINT32 ulRxBondSidStatus;                                       /* 0924 */
    UINT32 ulRxBondExpectedSid0;                                    /* 0928 */
    UINT32 ulRxBondExpectedSid1;                                    /* 092c */
    UINT32 ulRxBondBaseTimer;                                       /* 0930 */
    UINT32 ulRxBondTimeout0;                                        /* 0934 */
    UINT32 ulRxBondTimeout1;                                        /* 0938 */
    UINT32 ulRxBondSIDOutCnt;                                       /* 093c */
    UINT32 ulRxBondCellCount[XP_MAX_RXBOND_CHANNELS];               /* 0940 */
    UINT32 ulRxBondFragCount[XP_MAX_RXBOND_CHANNELS];               /* 0950 */
    UINT32 ulRxBondCellFull[XP_MAX_RXBOND_CHANNELS];                /* 0960 */
    UINT32 ulRxBondCellWriteAddr[XP_MAX_RXBOND_CHANNELS];           /* 0970 */
    UINT32 ulRxBondCellReadAddr[XP_MAX_RXBOND_CHANNELS];            /* 0980 */
    UINT32 ulRxBondReceivedSID[XP_MAX_RXBOND_CHANNELS];             /* 0990 */
    UINT32 ulRxBondDroppedFragCount[XP_MAX_RXPAF_WR_CHANNELS];      /* 09a0 */
    UINT32 ulRxBondErrorStatus_NC;                                  /* 09c0 */
    UINT32 ulReserved15[399];                                       /* 09c4 */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulSsteMpPriLevel[XP_MAX_MP_GROUPS][XP_MAX_PRI_LEVELS][4];/* 1000 */
    UINT32 ulSsteQueueScr[XP_MAX_CONNS][4];                         /* 1400 */
    UINT32 ulSsteQueuePcr[XP_MAX_CONNS][4];                         /* 1500 */
    UINT32 ulSsteMpScr[XP_MAX_MP_GROUPS][4];                        /* 1600 */
    UINT32 ulSsteMpPcr[XP_MAX_MP_GROUPS][4];                        /* 1680 */
    UINT32 ulSsteQueueMpArb[XP_MAX_CONNS][2];                       /* 1700 */
    UINT32 ulSsteQueuePriLevel[XP_MAX_CONNS];                       /* 1780 */
    UINT32 ulSsteQueueGtsCfg[XP_MAX_CONNS];                         /* 17C0 */
    UINT32 ulSsteQueueArbWt[XP_MAX_CONNS];                          /* 1800 */
    UINT32 ulSsteMpVcArbWt[XP_MAX_MP_GROUPS];                       /* 1840 */
    UINT32 ulReserved16[8] ;                                        /* 1860 */
    UINT32 ulSsteUtoPortScr[XP_MAX_PORTS][4];                       /* 1880 */
    UINT32 ulSsteUtoPortPcr[XP_MAX_PORTS][4];                       /* 18C0 */
    UINT32 ulSsteUtoGtsCfg[XP_MAX_PORTS];                           /* 1900 */
#else
    UINT32 ulSsteQueueGtsCfg[XP_MAX_CONNS];                         /* 1000 */
    UINT32 ulSsteMpScr[XP_MAX_MP_GROUPS][4];                        /* 1040 */
    UINT32 ulSsteMpPcr[XP_MAX_MP_GROUPS][4];                        /* 1080 */
    UINT32 ulSsteMpVcArbWt[XP_MAX_MP_GROUPS];                       /* 10c0 */
    UINT32 ulReserved16[12];                                        /* 10d0 */
    UINT32 ulSsteQueueArbWt[XP_MAX_CONNS];                          /* 1100 */
    UINT32 ulSsteQueuePriLevel[XP_MAX_CONNS];                       /* 1140 */
    UINT32 ulSsteQueueMpArb[XP_MAX_CONNS][2];                       /* 1180 */
    UINT32 ulSsteMpPriLevel[XP_MAX_MP_GROUPS][XP_MAX_PRI_LEVELS][4];/* 1200 */
    UINT32 ulSsteQueueScr[XP_MAX_CONNS][4];                         /* 1400 */
    UINT32 ulSsteQueuePcr[XP_MAX_CONNS][4];                         /* 1500 */
#endif
} XTM_PROCESSOR_REGISTERS, *PXTM_PROCESSOR_REGISTERS;

#define XP_REGS ((volatile PXTM_PROCESSOR_REGISTERS const) XTM_PROCESSOR_BASE)

typedef struct TxPafProcessorRegisters
{
    UINT32 ulControl;                            /* 0000 */
} TXPAF_PROCESSOR_REGISTERS, *PTXPAF_PROCESSOR_REGISTERS;

#define TXPAF_PROCESSOR_CONTROL_ENABLE_MASK     0x00000001
#define TXPAF_REGS ((volatile PTXPAF_PROCESSOR_REGISTERS const) TXPAF_PROCESSOR_BASE)

#endif /* _XTMPROCREGS_IMPL2_H_ */
