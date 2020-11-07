/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */

#ifndef ADF_H__
#define ADF_H__

/**
 * m = memory, c = core, r = register, f = field, d = data.
 */
#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
	((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
	((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
	 BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
	)

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/***************************************************************************
 *registers
 ***************************************************************************/
#define ADF_ADF_MODE_CONFIG                      0x00000000 /* ADF Mode Configuration */
#define ADF_ADF_FILTER_CONFIG                    0x00000004 /* ADF Filter Configuration */
#define ADF_ADF_PERIODIC_DURATION                0x00000008 /* ADF Periodic Duration Setting */
#define ADF_ADF_TIMER_IDLE_PERIOD                0x0000000c /* ADF Idle Phase Timers */
#define ADF_ADF_TIMER_DATA_PERIOD                0x00000010 /* ADF Data Phase Timers */
#define ADF_ADF_LOS_CONTROL                      0x00000014 /* ADF Loss-of-Signal Control */
#define ADF_ADF_LOS_ASSERT_THRESH                0x00000018 /* ADF Loss-of-Signal Assert Threshold */
#define ADF_ADF_LOS_DEASSERT_THRESH              0x0000001c /* ADF Loss-of-Signal Deassert Threshold */
#define ADF_ADF_DIAG_CAPTURE                     0x00000020 /* ADF Diagnostic Capture */
#define ADF_ADF_DIAG_CAPTURE_VALID               0x00000024 /* ADF Diagnostic Capture Valid */
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG             0x00000028 /* ADF ADC Diagnostic Address Configuration */
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG             0x0000002c /* ADF CIC Diagnostic Address Configuration */
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG             0x00000030 /* ADF AVG Diagnostic Address Configuration */
#define ADF_ADF_RSSI_CONTROL                     0x00000034 /* ADF RSSI Control */
#define ADF_ADF_RSSI_MONITOR_SAMPLE              0x00000038 /* ADF RSSI Monitor Sample */
#define ADF_ADF_RSSI_ZERO_SAMPLE                 0x0000003c /* ADF RSSI Zero Sample */
#define ADF_ADF_PARAM                            0x00000044 /* ADF Parameter Register */
#define ADF_ADF_LB_GAINS                         0x00000048 /* ADF LB gains Register */
#define ADF_ADF_LB_ASSERT_THRESH                 0x0000004c /* ADF LB gains Register */
#define ADF_ADF_LB_DEASSERT_THRESH               0x00000050 /* ADF LB gains Register */
#define ADF_ADF_LB_BUCKET_SIZE                   0x00000054 /* ADF LB bucket size Register */
#define ADF_ADF_LB_STATE                         0x00000058 /* ADF LB gains Register */

/***************************************************************************
 *ADF_MODE_CONFIG - ADF Mode Configuration
 ***************************************************************************/
/* ADF :: ADF_MODE_CONFIG :: reserved0 [31:16] */
#define ADF_ADF_MODE_CONFIG_reserved0_MASK                         0xffff0000
#define ADF_ADF_MODE_CONFIG_reserved0_ALIGN                        0
#define ADF_ADF_MODE_CONFIG_reserved0_BITS                         16
#define ADF_ADF_MODE_CONFIG_reserved0_SHIFT                        16

/* ADF :: ADF_MODE_CONFIG :: ADF_LOS_LB_FILT_EN [15:15] */
#define ADF_ADF_MODE_CONFIG_ADF_LOS_LB_FILT_EN_MASK                0x00008000
#define ADF_ADF_MODE_CONFIG_ADF_LOS_LB_FILT_EN_ALIGN               0
#define ADF_ADF_MODE_CONFIG_ADF_LOS_LB_FILT_EN_BITS                1
#define ADF_ADF_MODE_CONFIG_ADF_LOS_LB_FILT_EN_SHIFT               15
#define ADF_ADF_MODE_CONFIG_ADF_LOS_LB_FILT_EN_DEFAULT             0

/* ADF :: ADF_MODE_CONFIG :: reserved1 [14:14] */
#define ADF_ADF_MODE_CONFIG_reserved1_MASK                         0x00004000
#define ADF_ADF_MODE_CONFIG_reserved1_ALIGN                        0
#define ADF_ADF_MODE_CONFIG_reserved1_BITS                         1
#define ADF_ADF_MODE_CONFIG_reserved1_SHIFT                        14

/* ADF :: ADF_MODE_CONFIG :: ADF_AVG_FILT_EN [13:13] */
#define ADF_ADF_MODE_CONFIG_ADF_AVG_FILT_EN_MASK                   0x00002000
#define ADF_ADF_MODE_CONFIG_ADF_AVG_FILT_EN_ALIGN                  0
#define ADF_ADF_MODE_CONFIG_ADF_AVG_FILT_EN_BITS                   1
#define ADF_ADF_MODE_CONFIG_ADF_AVG_FILT_EN_SHIFT                  13
#define ADF_ADF_MODE_CONFIG_ADF_AVG_FILT_EN_DEFAULT                0

/* ADF :: ADF_MODE_CONFIG :: ADF_CIC_FILT_EN [12:12] */
#define ADF_ADF_MODE_CONFIG_ADF_CIC_FILT_EN_MASK                   0x00001000
#define ADF_ADF_MODE_CONFIG_ADF_CIC_FILT_EN_ALIGN                  0
#define ADF_ADF_MODE_CONFIG_ADF_CIC_FILT_EN_BITS                   1
#define ADF_ADF_MODE_CONFIG_ADF_CIC_FILT_EN_SHIFT                  12
#define ADF_ADF_MODE_CONFIG_ADF_CIC_FILT_EN_DEFAULT                0

/* ADF :: ADF_MODE_CONFIG :: reserved2 [11:11] */
#define ADF_ADF_MODE_CONFIG_reserved2_MASK                         0x00000800
#define ADF_ADF_MODE_CONFIG_reserved2_ALIGN                        0
#define ADF_ADF_MODE_CONFIG_reserved2_BITS                         1
#define ADF_ADF_MODE_CONFIG_reserved2_SHIFT                        11

/* ADF :: ADF_MODE_CONFIG :: ADF_APD_ICH_RESET [10:10] */
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_RESET_MASK                 0x00000400
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_RESET_ALIGN                0
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_RESET_BITS                 1
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_RESET_SHIFT                10
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_RESET_DEFAULT              1

/* ADF :: ADF_MODE_CONFIG :: ADF_APD_CLK_CAL [09:09] */
#define ADF_ADF_MODE_CONFIG_ADF_APD_CLK_CAL_MASK                   0x00000200
#define ADF_ADF_MODE_CONFIG_ADF_APD_CLK_CAL_ALIGN                  0
#define ADF_ADF_MODE_CONFIG_ADF_APD_CLK_CAL_BITS                   1
#define ADF_ADF_MODE_CONFIG_ADF_APD_CLK_CAL_SHIFT                  9
#define ADF_ADF_MODE_CONFIG_ADF_APD_CLK_CAL_DEFAULT                0

/* ADF :: ADF_MODE_CONFIG :: ADF_APD_ICH_PWRUP [08:08] */
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_PWRUP_MASK                 0x00000100
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_PWRUP_ALIGN                0
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_PWRUP_BITS                 1
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_PWRUP_SHIFT                8
#define ADF_ADF_MODE_CONFIG_ADF_APD_ICH_PWRUP_DEFAULT              0

/* ADF :: ADF_MODE_CONFIG :: reserved3 [07:06] */
#define ADF_ADF_MODE_CONFIG_reserved3_MASK                         0x000000c0
#define ADF_ADF_MODE_CONFIG_reserved3_ALIGN                        0
#define ADF_ADF_MODE_CONFIG_reserved3_BITS                         2
#define ADF_ADF_MODE_CONFIG_reserved3_SHIFT                        6

/* ADF :: ADF_MODE_CONFIG :: ADF_OP_MODE [05:04] */
#define ADF_ADF_MODE_CONFIG_ADF_OP_MODE_MASK                       0x00000030
#define ADF_ADF_MODE_CONFIG_ADF_OP_MODE_ALIGN                      0
#define ADF_ADF_MODE_CONFIG_ADF_OP_MODE_BITS                       2
#define ADF_ADF_MODE_CONFIG_ADF_OP_MODE_SHIFT                      4
#define ADF_ADF_MODE_CONFIG_ADF_OP_MODE_DEFAULT                    3

/* ADF :: ADF_MODE_CONFIG :: reserved4 [03:01] */
#define ADF_ADF_MODE_CONFIG_reserved4_MASK                         0x0000000e
#define ADF_ADF_MODE_CONFIG_reserved4_ALIGN                        0
#define ADF_ADF_MODE_CONFIG_reserved4_BITS                         3
#define ADF_ADF_MODE_CONFIG_reserved4_SHIFT                        1

/* ADF :: ADF_MODE_CONFIG :: ADF_MEASUREMENT_EN [00:00] */
#define ADF_ADF_MODE_CONFIG_ADF_MEASUREMENT_EN_MASK                0x00000001
#define ADF_ADF_MODE_CONFIG_ADF_MEASUREMENT_EN_ALIGN               0
#define ADF_ADF_MODE_CONFIG_ADF_MEASUREMENT_EN_BITS                1
#define ADF_ADF_MODE_CONFIG_ADF_MEASUREMENT_EN_SHIFT               0
#define ADF_ADF_MODE_CONFIG_ADF_MEASUREMENT_EN_DEFAULT             0

/***************************************************************************
 *ADF_FILTER_CONFIG - ADF Filter Configuration
 ***************************************************************************/
/* ADF :: ADF_FILTER_CONFIG :: reserved0 [31:12] */
#define ADF_ADF_FILTER_CONFIG_reserved0_MASK                       0xfffff000
#define ADF_ADF_FILTER_CONFIG_reserved0_ALIGN                      0
#define ADF_ADF_FILTER_CONFIG_reserved0_BITS                       20
#define ADF_ADF_FILTER_CONFIG_reserved0_SHIFT                      12

/* ADF :: ADF_FILTER_CONFIG :: ADF_SCALING_EXP [11:08] */
#define ADF_ADF_FILTER_CONFIG_ADF_SCALING_EXP_MASK                 0x00000f00
#define ADF_ADF_FILTER_CONFIG_ADF_SCALING_EXP_ALIGN                0
#define ADF_ADF_FILTER_CONFIG_ADF_SCALING_EXP_BITS                 4
#define ADF_ADF_FILTER_CONFIG_ADF_SCALING_EXP_SHIFT                8
#define ADF_ADF_FILTER_CONFIG_ADF_SCALING_EXP_DEFAULT              0

/* ADF :: ADF_FILTER_CONFIG :: ADF_SMOOTHING_FACTOR_EXP [07:04] */
#define ADF_ADF_FILTER_CONFIG_ADF_SMOOTHING_FACTOR_EXP_MASK        0x000000f0
#define ADF_ADF_FILTER_CONFIG_ADF_SMOOTHING_FACTOR_EXP_ALIGN       0
#define ADF_ADF_FILTER_CONFIG_ADF_SMOOTHING_FACTOR_EXP_BITS        4
#define ADF_ADF_FILTER_CONFIG_ADF_SMOOTHING_FACTOR_EXP_SHIFT       4
#define ADF_ADF_FILTER_CONFIG_ADF_SMOOTHING_FACTOR_EXP_DEFAULT     0

/* ADF :: ADF_FILTER_CONFIG :: reserved1 [03:01] */
#define ADF_ADF_FILTER_CONFIG_reserved1_MASK                       0x0000000e
#define ADF_ADF_FILTER_CONFIG_reserved1_ALIGN                      0
#define ADF_ADF_FILTER_CONFIG_reserved1_BITS                       3
#define ADF_ADF_FILTER_CONFIG_reserved1_SHIFT                      1

/* ADF :: ADF_FILTER_CONFIG :: ADF_AVG_FILTER_TYPE [00:00] */
#define ADF_ADF_FILTER_CONFIG_ADF_AVG_FILTER_TYPE_MASK             0x00000001
#define ADF_ADF_FILTER_CONFIG_ADF_AVG_FILTER_TYPE_ALIGN            0
#define ADF_ADF_FILTER_CONFIG_ADF_AVG_FILTER_TYPE_BITS             1
#define ADF_ADF_FILTER_CONFIG_ADF_AVG_FILTER_TYPE_SHIFT            0
#define ADF_ADF_FILTER_CONFIG_ADF_AVG_FILTER_TYPE_DEFAULT          0

/***************************************************************************
 *ADF_PERIODIC_DURATION - ADF Periodic Duration Setting
 ***************************************************************************/
/* ADF :: ADF_PERIODIC_DURATION :: reserved0 [31:10] */
#define ADF_ADF_PERIODIC_DURATION_reserved0_MASK                   0xfffffc00
#define ADF_ADF_PERIODIC_DURATION_reserved0_ALIGN                  0
#define ADF_ADF_PERIODIC_DURATION_reserved0_BITS                   22
#define ADF_ADF_PERIODIC_DURATION_reserved0_SHIFT                  10

/* ADF :: ADF_PERIODIC_DURATION :: ADF_TIMER_TP [09:00] */
#define ADF_ADF_PERIODIC_DURATION_ADF_TIMER_TP_MASK                0x000003ff
#define ADF_ADF_PERIODIC_DURATION_ADF_TIMER_TP_ALIGN               0
#define ADF_ADF_PERIODIC_DURATION_ADF_TIMER_TP_BITS                10
#define ADF_ADF_PERIODIC_DURATION_ADF_TIMER_TP_SHIFT               0
#define ADF_ADF_PERIODIC_DURATION_ADF_TIMER_TP_DEFAULT             0

/***************************************************************************
 *ADF_TIMER_IDLE_PERIOD - ADF Idle Phase Timers
 ***************************************************************************/
/* ADF :: ADF_TIMER_IDLE_PERIOD :: reserved0 [31:30] */
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved0_MASK                   0xc0000000
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved0_ALIGN                  0
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved0_BITS                   2
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved0_SHIFT                  30

/* ADF :: ADF_TIMER_IDLE_PERIOD :: ADF_TIMER_T3 [29:20] */
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T3_MASK                0x3ff00000
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T3_ALIGN               0
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T3_BITS                10
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T3_SHIFT               20
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T3_DEFAULT             200

/* ADF :: ADF_TIMER_IDLE_PERIOD :: reserved1 [19:17] */
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved1_MASK                   0x000e0000
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved1_ALIGN                  0
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved1_BITS                   3
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved1_SHIFT                  17

/* ADF :: ADF_TIMER_IDLE_PERIOD :: ADF_TIMER_T2 [16:12] */
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T2_MASK                0x0001f000
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T2_ALIGN               0
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T2_BITS                5
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T2_SHIFT               12
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T2_DEFAULT             10

/* ADF :: ADF_TIMER_IDLE_PERIOD :: reserved2 [11:07] */
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved2_MASK                   0x00000f80
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved2_ALIGN                  0
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved2_BITS                   5
#define ADF_ADF_TIMER_IDLE_PERIOD_reserved2_SHIFT                  7

/* ADF :: ADF_TIMER_IDLE_PERIOD :: ADF_TIMER_T1 [06:00] */
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T1_MASK                0x0000007f
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T1_ALIGN               0
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T1_BITS                7
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T1_SHIFT               0
#define ADF_ADF_TIMER_IDLE_PERIOD_ADF_TIMER_T1_DEFAULT             20

/***************************************************************************
 *ADF_TIMER_DATA_PERIOD - ADF Data Phase Timers
 ***************************************************************************/
/* ADF :: ADF_TIMER_DATA_PERIOD :: reserved0 [31:30] */
#define ADF_ADF_TIMER_DATA_PERIOD_reserved0_MASK                   0xc0000000
#define ADF_ADF_TIMER_DATA_PERIOD_reserved0_ALIGN                  0
#define ADF_ADF_TIMER_DATA_PERIOD_reserved0_BITS                   2
#define ADF_ADF_TIMER_DATA_PERIOD_reserved0_SHIFT                  30

/* ADF :: ADF_TIMER_DATA_PERIOD :: ADF_TIMER_T6 [29:20] */
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T6_MASK                0x3ff00000
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T6_ALIGN               0
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T6_BITS                10
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T6_SHIFT               20
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T6_DEFAULT             200

/* ADF :: ADF_TIMER_DATA_PERIOD :: reserved1 [19:17] */
#define ADF_ADF_TIMER_DATA_PERIOD_reserved1_MASK                   0x000e0000
#define ADF_ADF_TIMER_DATA_PERIOD_reserved1_ALIGN                  0
#define ADF_ADF_TIMER_DATA_PERIOD_reserved1_BITS                   3
#define ADF_ADF_TIMER_DATA_PERIOD_reserved1_SHIFT                  17

/* ADF :: ADF_TIMER_DATA_PERIOD :: ADF_TIMER_T5 [16:12] */
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T5_MASK                0x0001f000
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T5_ALIGN               0
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T5_BITS                5
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T5_SHIFT               12
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T5_DEFAULT             10

/* ADF :: ADF_TIMER_DATA_PERIOD :: reserved2 [11:07] */
#define ADF_ADF_TIMER_DATA_PERIOD_reserved2_MASK                   0x00000f80
#define ADF_ADF_TIMER_DATA_PERIOD_reserved2_ALIGN                  0
#define ADF_ADF_TIMER_DATA_PERIOD_reserved2_BITS                   5
#define ADF_ADF_TIMER_DATA_PERIOD_reserved2_SHIFT                  7

/* ADF :: ADF_TIMER_DATA_PERIOD :: ADF_TIMER_T4 [06:00] */
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T4_MASK                0x0000007f
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T4_ALIGN               0
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T4_BITS                7
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T4_SHIFT               0
#define ADF_ADF_TIMER_DATA_PERIOD_ADF_TIMER_T4_DEFAULT             20

/***************************************************************************
 *ADF_LOS_CONTROL - ADF Loss-of-Signal Control
 ***************************************************************************/
/* ADF :: ADF_LOS_CONTROL :: reserved0 [31:02] */
#define ADF_ADF_LOS_CONTROL_reserved0_MASK                         0xfffffffc
#define ADF_ADF_LOS_CONTROL_reserved0_ALIGN                        0
#define ADF_ADF_LOS_CONTROL_reserved0_BITS                         30
#define ADF_ADF_LOS_CONTROL_reserved0_SHIFT                        2

/* ADF :: ADF_LOS_CONTROL :: ADF_LOS_ALARM [01:01] */
#define ADF_ADF_LOS_CONTROL_ADF_LOS_ALARM_MASK                     0x00000002
#define ADF_ADF_LOS_CONTROL_ADF_LOS_ALARM_ALIGN                    0
#define ADF_ADF_LOS_CONTROL_ADF_LOS_ALARM_BITS                     1
#define ADF_ADF_LOS_CONTROL_ADF_LOS_ALARM_SHIFT                    1
#define ADF_ADF_LOS_CONTROL_ADF_LOS_ALARM_DEFAULT                  0

/* ADF :: ADF_LOS_CONTROL :: ADF_LOS_REVERSE_ORDER [00:00] */
#define ADF_ADF_LOS_CONTROL_ADF_LOS_REVERSE_ORDER_MASK             0x00000001
#define ADF_ADF_LOS_CONTROL_ADF_LOS_REVERSE_ORDER_ALIGN            0
#define ADF_ADF_LOS_CONTROL_ADF_LOS_REVERSE_ORDER_BITS             1
#define ADF_ADF_LOS_CONTROL_ADF_LOS_REVERSE_ORDER_SHIFT            0
#define ADF_ADF_LOS_CONTROL_ADF_LOS_REVERSE_ORDER_DEFAULT          0

/***************************************************************************
 *ADF_LOS_ASSERT_THRESH - ADF Loss-of-Signal Assert Threshold
 ***************************************************************************/
/* ADF :: ADF_LOS_ASSERT_THRESH :: ADF_LOS_ASSERT_THRESH [31:00] */
#define ADF_ADF_LOS_ASSERT_THRESH_ADF_LOS_ASSERT_THRESH_MASK       0xffffffff
#define ADF_ADF_LOS_ASSERT_THRESH_ADF_LOS_ASSERT_THRESH_ALIGN      0
#define ADF_ADF_LOS_ASSERT_THRESH_ADF_LOS_ASSERT_THRESH_BITS       32
#define ADF_ADF_LOS_ASSERT_THRESH_ADF_LOS_ASSERT_THRESH_SHIFT      0
#define ADF_ADF_LOS_ASSERT_THRESH_ADF_LOS_ASSERT_THRESH_DEFAULT    0

/***************************************************************************
 *ADF_LOS_DEASSERT_THRESH - ADF Loss-of-Signal Deassert Threshold
 ***************************************************************************/
/* ADF :: ADF_LOS_DEASSERT_THRESH :: ADF_LOS_DEASSERT_THRESH [31:00] */
#define ADF_ADF_LOS_DEASSERT_THRESH_ADF_LOS_DEASSERT_THRESH_MASK   0xffffffff
#define ADF_ADF_LOS_DEASSERT_THRESH_ADF_LOS_DEASSERT_THRESH_ALIGN  0
#define ADF_ADF_LOS_DEASSERT_THRESH_ADF_LOS_DEASSERT_THRESH_BITS   32
#define ADF_ADF_LOS_DEASSERT_THRESH_ADF_LOS_DEASSERT_THRESH_SHIFT  0
#define ADF_ADF_LOS_DEASSERT_THRESH_ADF_LOS_DEASSERT_THRESH_DEFAULT 0

/***************************************************************************
 *ADF_DIAG_CAPTURE - ADF Diagnostic Capture
 ***************************************************************************/
/* ADF :: ADF_DIAG_CAPTURE :: reserved0 [31:31] */
#define ADF_ADF_DIAG_CAPTURE_reserved0_MASK                        0x80000000
#define ADF_ADF_DIAG_CAPTURE_reserved0_ALIGN                       0
#define ADF_ADF_DIAG_CAPTURE_reserved0_BITS                        1
#define ADF_ADF_DIAG_CAPTURE_reserved0_SHIFT                       31

/* ADF :: ADF_DIAG_CAPTURE :: ADF_DIAG_CAPTURE_AVG_EN [30:30] */
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_AVG_EN_MASK          0x40000000
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_AVG_EN_ALIGN         0
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_AVG_EN_BITS          1
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_AVG_EN_SHIFT         30
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_AVG_EN_DEFAULT       0

/* ADF :: ADF_DIAG_CAPTURE :: ADF_DIAG_CAPTURE_CIC_EN [29:29] */
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_CIC_EN_MASK          0x20000000
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_CIC_EN_ALIGN         0
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_CIC_EN_BITS          1
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_CIC_EN_SHIFT         29
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_CIC_EN_DEFAULT       0

/* ADF :: ADF_DIAG_CAPTURE :: ADF_DIAG_CAPTURE_ADC_EN [28:28] */
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_ADC_EN_MASK          0x10000000
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_ADC_EN_ALIGN         0
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_ADC_EN_BITS          1
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_ADC_EN_SHIFT         28
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_ADC_EN_DEFAULT       0

/* ADF :: ADF_DIAG_CAPTURE :: reserved1 [27:26] */
#define ADF_ADF_DIAG_CAPTURE_reserved1_MASK                        0x0c000000
#define ADF_ADF_DIAG_CAPTURE_reserved1_ALIGN                       0
#define ADF_ADF_DIAG_CAPTURE_reserved1_BITS                        2
#define ADF_ADF_DIAG_CAPTURE_reserved1_SHIFT                       26

/* ADF :: ADF_DIAG_CAPTURE :: ADF_DIAG_CAPTURE_SIZE [25:00] */
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_SIZE_MASK            0x03ffffff
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_SIZE_ALIGN           0
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_SIZE_BITS            26
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_SIZE_SHIFT           0
#define ADF_ADF_DIAG_CAPTURE_ADF_DIAG_CAPTURE_SIZE_DEFAULT         160

/***************************************************************************
 *ADF_DIAG_CAPTURE_VALID - ADF Diagnostic Capture Valid
 ***************************************************************************/
/* ADF :: ADF_DIAG_CAPTURE_VALID :: reserved0 [31:01] */
#define ADF_ADF_DIAG_CAPTURE_VALID_reserved0_MASK                  0xfffffffe
#define ADF_ADF_DIAG_CAPTURE_VALID_reserved0_ALIGN                 0
#define ADF_ADF_DIAG_CAPTURE_VALID_reserved0_BITS                  31
#define ADF_ADF_DIAG_CAPTURE_VALID_reserved0_SHIFT                 1

/* ADF :: ADF_DIAG_CAPTURE_VALID :: ADF_DIAG_CAPTURE_VALID [00:00] */
#define ADF_ADF_DIAG_CAPTURE_VALID_ADF_DIAG_CAPTURE_VALID_MASK     0x00000001
#define ADF_ADF_DIAG_CAPTURE_VALID_ADF_DIAG_CAPTURE_VALID_ALIGN    0
#define ADF_ADF_DIAG_CAPTURE_VALID_ADF_DIAG_CAPTURE_VALID_BITS     1
#define ADF_ADF_DIAG_CAPTURE_VALID_ADF_DIAG_CAPTURE_VALID_SHIFT    0

/***************************************************************************
 *ADF_ADC_DIAG_ADDR_CONFIG - ADF ADC Diagnostic Address Configuration
 ***************************************************************************/
/* ADF :: ADF_ADC_DIAG_ADDR_CONFIG :: reserved0 [31:31] */
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved0_MASK                0x80000000
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved0_ALIGN               0
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved0_BITS                1
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved0_SHIFT               31

/* ADF :: ADF_ADC_DIAG_ADDR_CONFIG :: ADF_ADC_DIAG_END_ADDR [30:16] */
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_END_ADDR_MASK    0x7fff0000
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_END_ADDR_ALIGN   0
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_END_ADDR_BITS    15
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_END_ADDR_SHIFT   16
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_END_ADDR_DEFAULT 8351

/* ADF :: ADF_ADC_DIAG_ADDR_CONFIG :: reserved1 [15:15] */
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved1_MASK                0x00008000
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved1_ALIGN               0
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved1_BITS                1
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_reserved1_SHIFT               15

/* ADF :: ADF_ADC_DIAG_ADDR_CONFIG :: ADF_ADC_DIAG_START_ADDR [14:00] */
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_START_ADDR_MASK  0x00007fff
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_START_ADDR_ALIGN 0
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_START_ADDR_BITS  15
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_START_ADDR_SHIFT 0
#define ADF_ADF_ADC_DIAG_ADDR_CONFIG_ADF_ADC_DIAG_START_ADDR_DEFAULT 8192

/***************************************************************************
 *ADF_CIC_DIAG_ADDR_CONFIG - ADF CIC Diagnostic Address Configuration
 ***************************************************************************/
/* ADF :: ADF_CIC_DIAG_ADDR_CONFIG :: reserved0 [31:31] */
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved0_MASK                0x80000000
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved0_ALIGN               0
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved0_BITS                1
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved0_SHIFT               31

/* ADF :: ADF_CIC_DIAG_ADDR_CONFIG :: ADF_CIC_DIAG_END_ADDR [30:16] */
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_END_ADDR_MASK    0x7fff0000
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_END_ADDR_ALIGN   0
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_END_ADDR_BITS    15
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_END_ADDR_SHIFT   16
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_END_ADDR_DEFAULT 8511

/* ADF :: ADF_CIC_DIAG_ADDR_CONFIG :: reserved1 [15:15] */
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved1_MASK                0x00008000
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved1_ALIGN               0
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved1_BITS                1
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_reserved1_SHIFT               15

/* ADF :: ADF_CIC_DIAG_ADDR_CONFIG :: ADF_CIC_DIAG_START_ADDR [14:00] */
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_START_ADDR_MASK  0x00007fff
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_START_ADDR_ALIGN 0
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_START_ADDR_BITS  15
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_START_ADDR_SHIFT 0
#define ADF_ADF_CIC_DIAG_ADDR_CONFIG_ADF_CIC_DIAG_START_ADDR_DEFAULT 8352

/***************************************************************************
 *ADF_AVG_DIAG_ADDR_CONFIG - ADF AVG Diagnostic Address Configuration
 ***************************************************************************/
/* ADF :: ADF_AVG_DIAG_ADDR_CONFIG :: reserved0 [31:31] */
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved0_MASK                0x80000000
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved0_ALIGN               0
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved0_BITS                1
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved0_SHIFT               31

/* ADF :: ADF_AVG_DIAG_ADDR_CONFIG :: ADF_AVG_DIAG_END_ADDR [30:16] */
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_END_ADDR_MASK    0x7fff0000
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_END_ADDR_ALIGN   0
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_END_ADDR_BITS    15
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_END_ADDR_SHIFT   16
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_END_ADDR_DEFAULT 8671

/* ADF :: ADF_AVG_DIAG_ADDR_CONFIG :: reserved1 [15:15] */
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved1_MASK                0x00008000
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved1_ALIGN               0
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved1_BITS                1
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_reserved1_SHIFT               15

/* ADF :: ADF_AVG_DIAG_ADDR_CONFIG :: ADF_AVG_DIAG_START_ADDR [14:00] */
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_START_ADDR_MASK  0x00007fff
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_START_ADDR_ALIGN 0
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_START_ADDR_BITS  15
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_START_ADDR_SHIFT 0
#define ADF_ADF_AVG_DIAG_ADDR_CONFIG_ADF_AVG_DIAG_START_ADDR_DEFAULT 8512

/***************************************************************************
 *ADF_RSSI_CONTROL - ADF RSSI Control
 ***************************************************************************/
/* ADF :: ADF_RSSI_CONTROL :: reserved0 [31:02] */
#define ADF_ADF_RSSI_CONTROL_reserved0_MASK                        0xfffffffc
#define ADF_ADF_RSSI_CONTROL_reserved0_ALIGN                       0
#define ADF_ADF_RSSI_CONTROL_reserved0_BITS                        30
#define ADF_ADF_RSSI_CONTROL_reserved0_SHIFT                       2

/* ADF :: ADF_RSSI_CONTROL :: ADF_STAT_RSSI_VALID [01:01] */
#define ADF_ADF_RSSI_CONTROL_ADF_STAT_RSSI_VALID_MASK              0x00000002
#define ADF_ADF_RSSI_CONTROL_ADF_STAT_RSSI_VALID_ALIGN             0
#define ADF_ADF_RSSI_CONTROL_ADF_STAT_RSSI_VALID_BITS              1
#define ADF_ADF_RSSI_CONTROL_ADF_STAT_RSSI_VALID_SHIFT             1
#define ADF_ADF_RSSI_CONTROL_ADF_STAT_RSSI_VALID_DEFAULT           0

/* ADF :: ADF_RSSI_CONTROL :: ADF_RSSI_READY [00:00] */
#define ADF_ADF_RSSI_CONTROL_ADF_RSSI_READY_MASK                   0x00000001
#define ADF_ADF_RSSI_CONTROL_ADF_RSSI_READY_ALIGN                  0
#define ADF_ADF_RSSI_CONTROL_ADF_RSSI_READY_BITS                   1
#define ADF_ADF_RSSI_CONTROL_ADF_RSSI_READY_SHIFT                  0
#define ADF_ADF_RSSI_CONTROL_ADF_RSSI_READY_DEFAULT                0

/***************************************************************************
 *ADF_RSSI_MONITOR_SAMPLE - ADF RSSI Monitor Sample
 ***************************************************************************/
/* ADF :: ADF_RSSI_MONITOR_SAMPLE :: ADF_RSSI_MONITOR_SAMPLE [31:00] */
#define ADF_ADF_RSSI_MONITOR_SAMPLE_ADF_RSSI_MONITOR_SAMPLE_MASK   0xffffffff
#define ADF_ADF_RSSI_MONITOR_SAMPLE_ADF_RSSI_MONITOR_SAMPLE_ALIGN  0
#define ADF_ADF_RSSI_MONITOR_SAMPLE_ADF_RSSI_MONITOR_SAMPLE_BITS   32
#define ADF_ADF_RSSI_MONITOR_SAMPLE_ADF_RSSI_MONITOR_SAMPLE_SHIFT  0

/***************************************************************************
 *ADF_RSSI_ZERO_SAMPLE - ADF RSSI Zero Sample
 ***************************************************************************/
/* ADF :: ADF_RSSI_ZERO_SAMPLE :: ADF_RSSI_ZERO_SAMPLE [31:00] */
#define ADF_ADF_RSSI_ZERO_SAMPLE_ADF_RSSI_ZERO_SAMPLE_MASK         0xffffffff
#define ADF_ADF_RSSI_ZERO_SAMPLE_ADF_RSSI_ZERO_SAMPLE_ALIGN        0
#define ADF_ADF_RSSI_ZERO_SAMPLE_ADF_RSSI_ZERO_SAMPLE_BITS         32
#define ADF_ADF_RSSI_ZERO_SAMPLE_ADF_RSSI_ZERO_SAMPLE_SHIFT        0

/***************************************************************************
 *ADF_PARAM - ADF Parameter Register
 ***************************************************************************/
/* ADF :: ADF_PARAM :: reserved_for_eco0 [31:00] */
#define ADF_ADF_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define ADF_ADF_PARAM_reserved_for_eco0_ALIGN                      0
#define ADF_ADF_PARAM_reserved_for_eco0_BITS                       32
#define ADF_ADF_PARAM_reserved_for_eco0_SHIFT                      0
#define ADF_ADF_PARAM_reserved_for_eco0_DEFAULT                    0

/***************************************************************************
 *ADF_LB_GAINS - ADF LB gains Register
 ***************************************************************************/
/* ADF :: ADF_LB_GAINS :: reserved0 [31:12] */
#define ADF_ADF_LB_GAINS_reserved0_MASK                            0xfffff000
#define ADF_ADF_LB_GAINS_reserved0_ALIGN                           0
#define ADF_ADF_LB_GAINS_reserved0_BITS                            20
#define ADF_ADF_LB_GAINS_reserved0_SHIFT                           12

/* ADF :: ADF_LB_GAINS :: ADF_LB_NEGGAIN [11:08] */
#define ADF_ADF_LB_GAINS_ADF_LB_NEGGAIN_MASK                       0x00000f00
#define ADF_ADF_LB_GAINS_ADF_LB_NEGGAIN_ALIGN                      0
#define ADF_ADF_LB_GAINS_ADF_LB_NEGGAIN_BITS                       4
#define ADF_ADF_LB_GAINS_ADF_LB_NEGGAIN_SHIFT                      8
#define ADF_ADF_LB_GAINS_ADF_LB_NEGGAIN_DEFAULT                    0

/* ADF :: ADF_LB_GAINS :: reserved1 [07:04] */
#define ADF_ADF_LB_GAINS_reserved1_MASK                            0x000000f0
#define ADF_ADF_LB_GAINS_reserved1_ALIGN                           0
#define ADF_ADF_LB_GAINS_reserved1_BITS                            4
#define ADF_ADF_LB_GAINS_reserved1_SHIFT                           4

/* ADF :: ADF_LB_GAINS :: ADF_LB_POSGAIN [03:00] */
#define ADF_ADF_LB_GAINS_ADF_LB_POSGAIN_MASK                       0x0000000f
#define ADF_ADF_LB_GAINS_ADF_LB_POSGAIN_ALIGN                      0
#define ADF_ADF_LB_GAINS_ADF_LB_POSGAIN_BITS                       4
#define ADF_ADF_LB_GAINS_ADF_LB_POSGAIN_SHIFT                      0
#define ADF_ADF_LB_GAINS_ADF_LB_POSGAIN_DEFAULT                    0

/***************************************************************************
 *ADF_LB_ASSERT_THRESH - ADF LB gains Register
 ***************************************************************************/
/* ADF :: ADF_LB_ASSERT_THRESH :: reserved0 [31:16] */
#define ADF_ADF_LB_ASSERT_THRESH_reserved0_MASK                    0xffff0000
#define ADF_ADF_LB_ASSERT_THRESH_reserved0_ALIGN                   0
#define ADF_ADF_LB_ASSERT_THRESH_reserved0_BITS                    16
#define ADF_ADF_LB_ASSERT_THRESH_reserved0_SHIFT                   16

/* ADF :: ADF_LB_ASSERT_THRESH :: ADF_LB_ASSERT_THRESH [15:00] */
#define ADF_ADF_LB_ASSERT_THRESH_ADF_LB_ASSERT_THRESH_MASK         0x0000ffff
#define ADF_ADF_LB_ASSERT_THRESH_ADF_LB_ASSERT_THRESH_ALIGN        0
#define ADF_ADF_LB_ASSERT_THRESH_ADF_LB_ASSERT_THRESH_BITS         16
#define ADF_ADF_LB_ASSERT_THRESH_ADF_LB_ASSERT_THRESH_SHIFT        0
#define ADF_ADF_LB_ASSERT_THRESH_ADF_LB_ASSERT_THRESH_DEFAULT      0

/***************************************************************************
 *ADF_LB_DEASSERT_THRESH - ADF LB gains Register
 ***************************************************************************/
/* ADF :: ADF_LB_DEASSERT_THRESH :: reserved0 [31:16] */
#define ADF_ADF_LB_DEASSERT_THRESH_reserved0_MASK                  0xffff0000
#define ADF_ADF_LB_DEASSERT_THRESH_reserved0_ALIGN                 0
#define ADF_ADF_LB_DEASSERT_THRESH_reserved0_BITS                  16
#define ADF_ADF_LB_DEASSERT_THRESH_reserved0_SHIFT                 16

/* ADF :: ADF_LB_DEASSERT_THRESH :: ADF_LB_DEASSERT_THRESH [15:00] */
#define ADF_ADF_LB_DEASSERT_THRESH_ADF_LB_DEASSERT_THRESH_MASK     0x0000ffff
#define ADF_ADF_LB_DEASSERT_THRESH_ADF_LB_DEASSERT_THRESH_ALIGN    0
#define ADF_ADF_LB_DEASSERT_THRESH_ADF_LB_DEASSERT_THRESH_BITS     16
#define ADF_ADF_LB_DEASSERT_THRESH_ADF_LB_DEASSERT_THRESH_SHIFT    0
#define ADF_ADF_LB_DEASSERT_THRESH_ADF_LB_DEASSERT_THRESH_DEFAULT  0

/***************************************************************************
 *ADF_LB_BUCKET_SIZE - ADF LB bucket size Register
 ***************************************************************************/
/* ADF :: ADF_LB_BUCKET_SIZE :: reserved0 [31:16] */
#define ADF_ADF_LB_BUCKET_SIZE_reserved0_MASK                      0xffff0000
#define ADF_ADF_LB_BUCKET_SIZE_reserved0_ALIGN                     0
#define ADF_ADF_LB_BUCKET_SIZE_reserved0_BITS                      16
#define ADF_ADF_LB_BUCKET_SIZE_reserved0_SHIFT                     16

/* ADF :: ADF_LB_BUCKET_SIZE :: ADF_LB_BUCKET_SIZE [15:00] */
#define ADF_ADF_LB_BUCKET_SIZE_ADF_LB_BUCKET_SIZE_MASK             0x0000ffff
#define ADF_ADF_LB_BUCKET_SIZE_ADF_LB_BUCKET_SIZE_ALIGN            0
#define ADF_ADF_LB_BUCKET_SIZE_ADF_LB_BUCKET_SIZE_BITS             16
#define ADF_ADF_LB_BUCKET_SIZE_ADF_LB_BUCKET_SIZE_SHIFT            0
#define ADF_ADF_LB_BUCKET_SIZE_ADF_LB_BUCKET_SIZE_DEFAULT          0

/***************************************************************************
 *ADF_LB_STATE - ADF LB gains Register
 ***************************************************************************/
/* ADF :: ADF_LB_STATE :: reserved0 [31:16] */
#define ADF_ADF_LB_STATE_reserved0_MASK                            0xffff0000
#define ADF_ADF_LB_STATE_reserved0_ALIGN                           0
#define ADF_ADF_LB_STATE_reserved0_BITS                            16
#define ADF_ADF_LB_STATE_reserved0_SHIFT                           16

/* ADF :: ADF_LB_STATE :: ADF_LB_STATE [15:00] */
#define ADF_ADF_LB_STATE_ADF_LB_STATE_MASK                         0x0000ffff
#define ADF_ADF_LB_STATE_ADF_LB_STATE_ALIGN                        0
#define ADF_ADF_LB_STATE_ADF_LB_STATE_BITS                         16
#define ADF_ADF_LB_STATE_ADF_LB_STATE_SHIFT                        0

#endif /* #ifndef ADF_H__ */

/* End of File */
