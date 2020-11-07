/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/

#ifndef __BRCM_RDB_SIMI_H__
#define __BRCM_RDB_SIMI_H__

#define SIMI_SCR_OFFSET                                                   0x00000000
#define SIMI_SCR_TYPE                                                     UInt32
#define SIMI_SCR_RESERVED_MASK                                            0x02000020
#define    SIMI_SCR_SIMEN_SHIFT                                           31
#define    SIMI_SCR_SIMEN_MASK                                            0x80000000
#define    SIMI_SCR_SOFTRST_SHIFT                                         30
#define    SIMI_SCR_SOFTRST_MASK                                          0x40000000
#define    SIMI_SCR_PTS_SHIFT                                             29
#define    SIMI_SCR_PTS_MASK                                              0x20000000
#define    SIMI_SCR_CLKF_SHIFT                                            28
#define    SIMI_SCR_CLKF_MASK                                             0x10000000
#define    SIMI_SCR_FIFOEN_SHIFT                                          27
#define    SIMI_SCR_FIFOEN_MASK                                           0x08000000
#define    SIMI_SCR_GCTEN_SHIFT                                           26
#define    SIMI_SCR_GCTEN_MASK                                            0x04000000
#define    SIMI_SCR_VPPEN_SHIFT                                           24
#define    SIMI_SCR_VPPEN_MASK                                            0x01000000
#define    SIMI_SCR_TXRETRY_SHIFT                                         20
#define    SIMI_SCR_TXRETRY_MASK                                          0x00F00000
#define    SIMI_SCR_RXRETRY_SHIFT                                         16
#define    SIMI_SCR_RXRETRY_MASK                                          0x000F0000
#define    SIMI_SCR_SRST_SHIFT                                            15
#define    SIMI_SCR_SRST_MASK                                             0x00008000
#define    SIMI_SCR_DMAEN_SHIFT                                           14
#define    SIMI_SCR_DMAEN_MASK                                            0x00004000
#define    SIMI_SCR_TXENDQUICK_SHIFT                                      13
#define    SIMI_SCR_TXENDQUICK_MASK                                       0x00002000
#define    SIMI_SCR_ETUCKS_SHIFT                                          11
#define    SIMI_SCR_ETUCKS_MASK                                           0x00001800
#define    SIMI_SCR_TXPARITYMARK_SHIFT                                    10
#define    SIMI_SCR_TXPARITYMARK_MASK                                     0x00000400
#define    SIMI_SCR_PARSEL_SHIFT                                          9
#define    SIMI_SCR_PARSEL_MASK                                           0x00000200
#define    SIMI_SCR_DATEN_SHIFT                                           8
#define    SIMI_SCR_DATEN_MASK                                            0x00000100
#define    SIMI_SCR_TX_PARITY_OFF_SHIFT                                   7
#define    SIMI_SCR_TX_PARITY_OFF_MASK                                    0x00000080
#define    SIMI_SCR_RX_PARITY_OFF_SHIFT                                   6
#define    SIMI_SCR_RX_PARITY_OFF_MASK                                    0x00000040
#define    SIMI_SCR_CHECKBACK_EN_SHIFT                                    4
#define    SIMI_SCR_CHECKBACK_EN_MASK                                     0x00000010
#define    SIMI_SCR_STOP_SHIFT                                            3
#define    SIMI_SCR_STOP_MASK                                             0x00000008
#define    SIMI_SCR_STPS_SHIFT                                            2
#define    SIMI_SCR_STPS_MASK                                             0x00000004
#define    SIMI_SCR_SIMDAT_MASK_EN_SHIFT                                  1
#define    SIMI_SCR_SIMDAT_MASK_EN_MASK                                   0x00000002
#define    SIMI_SCR_RSTS_SHIFT                                            0
#define    SIMI_SCR_RSTS_MASK                                             0x00000001

#define SIMI_SSR_OFFSET                                                   0x00000004
#define SIMI_SSR_TYPE                                                     UInt32
#define SIMI_SSR_RESERVED_MASK                                            0xFFFF7000
#define    SIMI_SSR_TXMODE_SHIFT                                          15
#define    SIMI_SSR_TXMODE_MASK                                           0x00008000
#define    SIMI_SSR_RX_ABORT_SHIFT                                        11
#define    SIMI_SSR_RX_ABORT_MASK                                         0x00000800
#define    SIMI_SSR_RX_REPEAT_SHIFT                                       10
#define    SIMI_SSR_RX_REPEAT_MASK                                        0x00000400
#define    SIMI_SSR_TXIDLE_SHIFT                                          9
#define    SIMI_SSR_TXIDLE_MASK                                           0x00000200
#define    SIMI_SSR_GCNTI_SHIFT                                           8
#define    SIMI_SSR_GCNTI_MASK                                            0x00000100
#define    SIMI_SSR_TXTHRE_SHIFT                                          7
#define    SIMI_SSR_TXTHRE_MASK                                           0x00000080
#define    SIMI_SSR_RXTOUT_SHIFT                                          6
#define    SIMI_SSR_RXTOUT_MASK                                           0x00000040
#define    SIMI_SSR_RXTHRE_SHIFT                                          5
#define    SIMI_SSR_RXTHRE_MASK                                           0x00000020
#define    SIMI_SSR_ROVF_SHIFT                                            4
#define    SIMI_SSR_ROVF_MASK                                             0x00000010
#define    SIMI_SSR_RDR_SHIFT                                             3
#define    SIMI_SSR_RDR_MASK                                              0x00000008
#define    SIMI_SSR_TXDONE_SHIFT                                          2
#define    SIMI_SSR_TXDONE_MASK                                           0x00000004
#define    SIMI_SSR_TERR_SHIFT                                            1
#define    SIMI_SSR_TERR_MASK                                             0x00000002
#define    SIMI_SSR_PERR_SHIFT                                            0
#define    SIMI_SSR_PERR_MASK                                             0x00000001

#define SIMI_SDR_OFFSET                                                   0x00000008
#define SIMI_SDR_TYPE                                                     UInt32
#define SIMI_SDR_RESERVED_MASK                                            0xFFFFFE00
#define    SIMI_SDR_PARE_SHIFT                                            8
#define    SIMI_SDR_PARE_MASK                                             0x00000100
#define    SIMI_SDR_DATA_SHIFT                                            0
#define    SIMI_SDR_DATA_MASK                                             0x000000FF

#define SIMI_SIER_OFFSET                                                  0x0000000C
#define SIMI_SIER_TYPE                                                    UInt32
#define SIMI_SIER_RESERVED_MASK                                           0xFFFFF000
#define    SIMI_SIER_RX_ABORT_SHIFT                                       11
#define    SIMI_SIER_RX_ABORT_MASK                                        0x00000800
#define    SIMI_SIER_RX_REPEAT_SHIFT                                      10
#define    SIMI_SIER_RX_REPEAT_MASK                                       0x00000400
#define    SIMI_SIER_TXIDLE_SHIFT                                         9
#define    SIMI_SIER_TXIDLE_MASK                                          0x00000200
#define    SIMI_SIER_GCNTI_SHIFT                                          8
#define    SIMI_SIER_GCNTI_MASK                                           0x00000100
#define    SIMI_SIER_TXTHRE_SHIFT                                         7
#define    SIMI_SIER_TXTHRE_MASK                                          0x00000080
#define    SIMI_SIER_RXTOUT_SHIFT                                         6
#define    SIMI_SIER_RXTOUT_MASK                                          0x00000040
#define    SIMI_SIER_RXTHRE_SHIFT                                         5
#define    SIMI_SIER_RXTHRE_MASK                                          0x00000020
#define    SIMI_SIER_ROVF_SHIFT                                           4
#define    SIMI_SIER_ROVF_MASK                                            0x00000010
#define    SIMI_SIER_RDR_SHIFT                                            3
#define    SIMI_SIER_RDR_MASK                                             0x00000008
#define    SIMI_SIER_TXDONE_SHIFT                                         2
#define    SIMI_SIER_TXDONE_MASK                                          0x00000004
#define    SIMI_SIER_TERR_SHIFT                                           1
#define    SIMI_SIER_TERR_MASK                                            0x00000002
#define    SIMI_SIER_PERR_SHIFT                                           0
#define    SIMI_SIER_PERR_MASK                                            0x00000001

#define SIMI_SFCR_OFFSET                                                  0x00000010
#define SIMI_SFCR_TYPE                                                    UInt32
#define SIMI_SFCR_RESERVED_MASK                                           0x7F80C0C0
#define    SIMI_SFCR_FLUSH_SHIFT                                          31
#define    SIMI_SFCR_FLUSH_MASK                                           0x80000000
#define    SIMI_SFCR_FIFOCNT_SHIFT                                        16
#define    SIMI_SFCR_FIFOCNT_MASK                                         0x007F0000
#define    SIMI_SFCR_TXTHRE_SHIFT                                         8
#define    SIMI_SFCR_TXTHRE_MASK                                          0x00003F00
#define    SIMI_SFCR_RXTHRE_SHIFT                                         0
#define    SIMI_SFCR_RXTHRE_MASK                                          0x0000003F

#define SIMI_SECGTR_OFFSET                                                0x00000014
#define SIMI_SECGTR_TYPE                                                  UInt32
#define SIMI_SECGTR_RESERVED_MASK                                         0xFFFFFF00
#define    SIMI_SECGTR_SECGTR_SHIFT                                       0
#define    SIMI_SECGTR_SECGTR_MASK                                        0x000000FF

#define SIMI_STGTR_OFFSET                                                 0x00000018
#define SIMI_STGTR_TYPE                                                   UInt32
#define SIMI_STGTR_RESERVED_MASK                                          0xFFFFFF00
#define    SIMI_STGTR_STGTR_SHIFT                                         0
#define    SIMI_STGTR_STGTR_MASK                                          0x000000FF

#define SIMI_SGCCR_OFFSET                                                 0x0000001C
#define SIMI_SGCCR_TYPE                                                   UInt32
#define SIMI_SGCCR_RESERVED_MASK                                          0x00000000
#define    SIMI_SGCCR_SGCCR_SHIFT                                         0
#define    SIMI_SGCCR_SGCCR_MASK                                          0xFFFFFFFF

#define SIMI_SGCVR_OFFSET                                                 0x00000020
#define SIMI_SGCVR_TYPE                                                   UInt32
#define SIMI_SGCVR_RESERVED_MASK                                          0x00000000
#define    SIMI_SGCVR_SGCVR_SHIFT                                         0
#define    SIMI_SGCVR_SGCVR_MASK                                          0xFFFFFFFF

#define SIMI_SCDR_OFFSET                                                  0x00000024
#define SIMI_SCDR_TYPE                                                    UInt32
#define SIMI_SCDR_RESERVED_MASK                                           0xFFFF7F00
#define    SIMI_SCDR_SIMCLK_DIV_EN_SHIFT                                  15
#define    SIMI_SCDR_SIMCLK_DIV_EN_MASK                                   0x00008000
#define    SIMI_SCDR_SIMCLK_DIVISOR_SHIFT                                 0
#define    SIMI_SCDR_SIMCLK_DIVISOR_MASK                                  0x000000FF

#define SIMI_SFDRR_OFFSET                                                 0x00000028
#define SIMI_SFDRR_TYPE                                                   UInt32
#define SIMI_SFDRR_RESERVED_MASK                                          0xFFFE0000
#define    SIMI_SFDRR_FD_RATIO_MODE_EN_SHIFT                              16
#define    SIMI_SFDRR_FD_RATIO_MODE_EN_MASK                               0x00010000
#define    SIMI_SFDRR_FD_RATIO_SHIFT                                      0
#define    SIMI_SFDRR_FD_RATIO_MASK                                       0x0000FFFF

#define SIMI_SESR_OFFSET                                                  0x0000002C
#define SIMI_SESR_TYPE                                                    UInt32
#define SIMI_SESR_RESERVED_MASK                                           0xFFFF7E00
#define    SIMI_SESR_EXTRA_SAMPLE_EN_SHIFT                                15
#define    SIMI_SESR_EXTRA_SAMPLE_EN_MASK                                 0x00008000
#define    SIMI_SESR_EXTRA_SAMPLE_OFFSET_SHIFT                            0
#define    SIMI_SESR_EXTRA_SAMPLE_OFFSET_MASK                             0x000001FF

#define SIMI_SIMDEBUG_OFFSET                                              0x00000030
#define SIMI_SIMDEBUG_TYPE                                                UInt32
#define SIMI_SIMDEBUG_RESERVED_MASK                                       0x00000818
#define    SIMI_SIMDEBUG_REVSEBACK_RXPARITY_GEN_SHIFT                     31
#define    SIMI_SIMDEBUG_REVSEBACK_RXPARITY_GEN_MASK                      0x80000000
#define    SIMI_SIMDEBUG_SIMSTATE_SHIFT                                   24
#define    SIMI_SIMDEBUG_SIMSTATE_MASK                                    0x7F000000
#define    SIMI_SIMDEBUG_FIFOWPT_SHIFT                                    18
#define    SIMI_SIMDEBUG_FIFOWPT_MASK                                     0x00FC0000
#define    SIMI_SIMDEBUG_FIFORPT_SHIFT                                    12
#define    SIMI_SIMDEBUG_FIFORPT_MASK                                     0x0003F000
#define    SIMI_SIMDEBUG_RXSTATE_SHIFT                                    8
#define    SIMI_SIMDEBUG_RXSTATE_MASK                                     0x00000700
#define    SIMI_SIMDEBUG_TXSTATE_SHIFT                                    5
#define    SIMI_SIMDEBUG_TXSTATE_MASK                                     0x000000E0
#define    SIMI_SIMDEBUG_TXMODE_SHIFT                                     2
#define    SIMI_SIMDEBUG_TXMODE_MASK                                      0x00000004
#define    SIMI_SIMDEBUG_ORDER_SHIFT                                      1
#define    SIMI_SIMDEBUG_ORDER_MASK                                       0x00000002
#define    SIMI_SIMDEBUG_SENSE_SHIFT                                      0
#define    SIMI_SIMDEBUG_SENSE_MASK                                       0x00000001

#define SIMI_SRTOR_OFFSET                                                 0x00000038
#define SIMI_SRTOR_TYPE                                                   UInt32
#define SIMI_SRTOR_RESERVED_MASK                                          0xC000FC00
#define    SIMI_SRTOR_CLK1MHZ_PRESCALE_SHIFT                              24
#define    SIMI_SRTOR_CLK1MHZ_PRESCALE_MASK                               0x3F000000
#define    SIMI_SRTOR_CLK100US_DIV_SHIFT                                  16
#define    SIMI_SRTOR_CLK100US_DIV_MASK                                   0x00FF0000
#define    SIMI_SRTOR_TIMEOUT_VALUE_SHIFT                                 0
#define    SIMI_SRTOR_TIMEOUT_VALUE_MASK                                  0x000003FF

#define SIMI_SIPVER_OFFSET                                                0x0000004C
#define SIMI_SIPVER_TYPE                                                  UInt32
#define SIMI_SIPVER_RESERVED_MASK                                         0x00FFFFFF
#define    SIMI_SIPVER_SIM_IP_VERSION_SHIFT                               24
#define    SIMI_SIPVER_SIM_IP_VERSION_MASK                                0xFF000000

#define SIMI_DESDCR_OFFSET                                                0x00000060
#define SIMI_DESDCR_TYPE                                                  UInt32
#define SIMI_DESDCR_RESERVED_MASK                                         0x3FFF7F9F
#define    SIMI_DESDCR_SIM_DET_EN_SHIFT                                   31
#define    SIMI_DESDCR_SIM_DET_EN_MASK                                    0x80000000
#define    SIMI_DESDCR_SIM_ESD_EN_SHIFT                                   30
#define    SIMI_DESDCR_SIM_ESD_EN_MASK                                    0x40000000
#define    SIMI_DESDCR_SIM_DET_ESD_SOFT_RST_SHIFT                         15
#define    SIMI_DESDCR_SIM_DET_ESD_SOFT_RST_MASK                          0x00008000
#define    SIMI_DESDCR_SIM_WATCHDOG_ESD_EN_SHIFT                          6
#define    SIMI_DESDCR_SIM_WATCHDOG_ESD_EN_MASK                           0x00000040
#define    SIMI_DESDCR_SIM_BATRM_ESD_EN_SHIFT                             5
#define    SIMI_DESDCR_SIM_BATRM_ESD_EN_MASK                              0x00000020

#define SIMI_DESDISR_OFFSET                                               0x00000064
#define SIMI_DESDISR_TYPE                                                 UInt32
#define SIMI_DESDISR_RESERVED_MASK                                        0xFF08FF08
#define    SIMI_DESDISR_WATCHDOG_ESD_IER_SHIFT                            23
#define    SIMI_DESDISR_WATCHDOG_ESD_IER_MASK                             0x00800000
#define    SIMI_DESDISR_WATCHDOG_RST_REQ_IER_SHIFT                        22
#define    SIMI_DESDISR_WATCHDOG_RST_REQ_IER_MASK                         0x00400000
#define    SIMI_DESDISR_BATRM_ESD_IER_SHIFT                               21
#define    SIMI_DESDISR_BATRM_ESD_IER_MASK                                0x00200000
#define    SIMI_DESDISR_BATRM_N_IER_SHIFT                                 20
#define    SIMI_DESDISR_BATRM_N_IER_MASK                                  0x00100000
#define    SIMI_DESDISR_CARD_OUT_ESD_IER_SHIFT                            18
#define    SIMI_DESDISR_CARD_OUT_ESD_IER_MASK                             0x00040000
#define    SIMI_DESDISR_CARD_OUT_IER_SHIFT                                17
#define    SIMI_DESDISR_CARD_OUT_IER_MASK                                 0x00020000
#define    SIMI_DESDISR_CARD_IN_IER_SHIFT                                 16
#define    SIMI_DESDISR_CARD_IN_IER_MASK                                  0x00010000
#define    SIMI_DESDISR_WATDOG_ESD_ISR_SHIFT                              7
#define    SIMI_DESDISR_WATDOG_ESD_ISR_MASK                               0x00000080
#define    SIMI_DESDISR_WATCHDOG_RST_REQ_SHIFT                            6
#define    SIMI_DESDISR_WATCHDOG_RST_REQ_MASK                             0x00000040
#define    SIMI_DESDISR_BATRM_ESD_ISR_SHIFT                               5
#define    SIMI_DESDISR_BATRM_ESD_ISR_MASK                                0x00000020
#define    SIMI_DESDISR_BATRM_N_SHIFT                                     4
#define    SIMI_DESDISR_BATRM_N_MASK                                      0x00000010
#define    SIMI_DESDISR_CARD_OUT_ESD_ISR_SHIFT                            2
#define    SIMI_DESDISR_CARD_OUT_ESD_ISR_MASK                             0x00000004
#define    SIMI_DESDISR_CARD_OUT_ISR_SHIFT                                1
#define    SIMI_DESDISR_CARD_OUT_ISR_MASK                                 0x00000002
#define    SIMI_DESDISR_CARD_IN_ISR_SHIFT                                 0
#define    SIMI_DESDISR_CARD_IN_ISR_MASK                                  0x00000001

#define SIMI_SCARDSR_OFFSET                                               0x00000068
#define SIMI_SCARDSR_TYPE                                                 UInt32
#define SIMI_SCARDSR_RESERVED_MASK                                        0xFFF03F2C
#define    SIMI_SCARDSR_SIM_DEBOUNCE_TIME_SHIFT                           16
#define    SIMI_SCARDSR_SIM_DEBOUNCE_TIME_MASK                            0x000F0000
#define    SIMI_SCARDSR_SIM_PRESENCE_SHIFT                                15
#define    SIMI_SCARDSR_SIM_PRESENCE_MASK                                 0x00008000
#define    SIMI_SCARDSR_SIM_PRESENCE_DEBED_SHIFT                          14
#define    SIMI_SCARDSR_SIM_PRESENCE_DEBED_MASK                           0x00004000
#define    SIMI_SCARDSR_PRESENCE_PRE_DEBOUNCE_MODE_SHIFT                  6
#define    SIMI_SCARDSR_PRESENCE_PRE_DEBOUNCE_MODE_MASK                   0x000000C0
#define    SIMI_SCARDSR_PRESENCE_DEBOUNCE_EN_SHIFT                        4
#define    SIMI_SCARDSR_PRESENCE_DEBOUNCE_EN_MASK                         0x00000010
#define    SIMI_SCARDSR_CARDOUT_ESHUTDOWN_SHIFT                           1
#define    SIMI_SCARDSR_CARDOUT_ESHUTDOWN_MASK                            0x00000002
#define    SIMI_SCARDSR_PRESENCE_LOW_SHIFT                                0
#define    SIMI_SCARDSR_PRESENCE_LOW_MASK                                 0x00000001

#define SIMI_SLDOCR_OFFSET                                                0x0000006C
#define SIMI_SLDOCR_TYPE                                                  UInt32
#define SIMI_SLDOCR_RESERVED_MASK                                         0xFFFFFFF8
#define    SIMI_SLDOCR_SIMVCC_EMERGENCY_EN_SHIFT                          2
#define    SIMI_SLDOCR_SIMVCC_EMERGENCY_EN_MASK                           0x00000004
#define    SIMI_SLDOCR_SIMVCC_SEL_SHIFT                                   1
#define    SIMI_SLDOCR_SIMVCC_SEL_MASK                                    0x00000002
#define    SIMI_SLDOCR_SIMVCC_EN_SHIFT                                    0
#define    SIMI_SLDOCR_SIMVCC_EN_MASK                                     0x00000001

#define GPIO_USIM_CONTROL_OFFSET                                          0x00000000
#define GPIO_USIM_CONTROL_TYPE                                            UInt32
#define GPIO_USIM_CONTROL_RESERVED_MASK                                   0xFFFFFFF8
#define    GPIO_USIM_CONTROL_VPP_EN_POLARITY_SHIFT                        2
#define    GPIO_USIM_CONTROL_VPP_EN_POLARITY_MASK                         0x00000004
#define    GPIO_USIM_CONTROL_VCC_EN_POLARITY_SHIFT                        1
#define    GPIO_USIM_CONTROL_VCC_EN_POLARITY_MASK                         0x00000002
#define    GPIO_USIM_CONTROL_USIM_BATRM_N_SHIFT                           0
#define    GPIO_USIM_CONTROL_USIM_BATRM_N_MASK                            0x00000001

#endif /* __BRCM_RDB_SIMI_H__ */


