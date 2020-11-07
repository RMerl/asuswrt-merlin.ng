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

#ifndef ESC_H__
#define ESC_H__

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
#define ESC_ESC_STATUS                           0x00000000 /* Eye-Safety Status */
#define ESC_ESC_CONFIG1                          0x00000004 /* ESC Operating Mode Configuration 1 */
#define ESC_ESC_CONFIG2                          0x00000008 /* ESC Operating Mode Configuration 2 */
#define ESC_ESC_ALARM_CONFIG                     0x0000000c /* ESC Alarm Configuration */
#define ESC_ESC_ROGUE_ALARM_CONFIG1              0x00000010 /* ESC Rogue Alarm Configuration 1 */
#define ESC_ESC_ROGUE_ALARM_CONFIG2              0x00000014 /* ESC Rogue Alarm Configuration 2 */
#define ESC_OVER_CURRENT_ALARM_CONFIG1           0x00000018 /* ESC Over Current Alarm Configuration 1 */
#define ESC_OVER_CURRENT_ALARM_CONFIG2           0x0000001c /* ESC Over Current Alarm Configuration 2 */
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG           0x00000020 /* ESC MPD fault alarm configuration */
#define ESC_ESC_BE_ALARM_CONFIG                  0x00000024 /* ESC BE fault alarm configuration */
#define ESC_ESC_FIRST_BURST_LIMITATION           0x00000028 /* ESC First burst limit configuration */
#define ESC_ESC_MASK_ALARMS                      0x0000002c /* ESC Alarm Action Mask */
#define ESC_ESC_INT                              0x00000030 /* ESC Interrupt Status register */
#define ESC_ESC_INT_EN                           0x00000034 /* ESC Interrupt Enable/Mask register */
#define ESC_ESC_FAULT_OUT_CONFIGURATION          0x00000038 /* ESC alarm fault output configuration */
#define ESC_ESC_SPARE_SYNCHRONIZERS              0x0000003c /* ESC spare synchronizer input configuration */
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS         0x00000040 /* ESC spare synchronizer output status */
#define ESC_ESC_PARAM                            0x00000044 /* ESC Parameter Register */
#define ESC_ESC_ROGUE_ALARM_CONFIG3              0x00000048 /* ESC Rogue Alarm Configuration 3 */
#define ESC_ESC_MPD_ON_DELAY                     0x0000004c /* ESC MPD on delay */

/***************************************************************************
 *ESC_STATUS - Eye-Safety Status
 ***************************************************************************/
/* ESC :: ESC_STATUS :: reserved0 [31:11] */
#define ESC_ESC_STATUS_reserved0_MASK                              0xfffff800
#define ESC_ESC_STATUS_reserved0_ALIGN                             0
#define ESC_ESC_STATUS_reserved0_BITS                              21
#define ESC_ESC_STATUS_reserved0_SHIFT                             11

/* ESC :: ESC_STATUS :: ESC_STAT_BE_FAULT [10:10] */
#define ESC_ESC_STATUS_ESC_STAT_BE_FAULT_MASK                      0x00000400
#define ESC_ESC_STATUS_ESC_STAT_BE_FAULT_ALIGN                     0
#define ESC_ESC_STATUS_ESC_STAT_BE_FAULT_BITS                      1
#define ESC_ESC_STATUS_ESC_STAT_BE_FAULT_SHIFT                     10

/* ESC :: ESC_STATUS :: ESC_STAT_ROGUE [09:09] */
#define ESC_ESC_STATUS_ESC_STAT_ROGUE_MASK                         0x00000200
#define ESC_ESC_STATUS_ESC_STAT_ROGUE_ALIGN                        0
#define ESC_ESC_STATUS_ESC_STAT_ROGUE_BITS                         1
#define ESC_ESC_STATUS_ESC_STAT_ROGUE_SHIFT                        9

/* ESC :: ESC_STATUS :: ESC_STAT_MOD_OVR_CRNT [08:08] */
#define ESC_ESC_STATUS_ESC_STAT_MOD_OVR_CRNT_MASK                  0x00000100
#define ESC_ESC_STATUS_ESC_STAT_MOD_OVR_CRNT_ALIGN                 0
#define ESC_ESC_STATUS_ESC_STAT_MOD_OVR_CRNT_BITS                  1
#define ESC_ESC_STATUS_ESC_STAT_MOD_OVR_CRNT_SHIFT                 8

/* ESC :: ESC_STATUS :: ESC_STAT_BIAS_OVR_CRNT [07:07] */
#define ESC_ESC_STATUS_ESC_STAT_BIAS_OVR_CRNT_MASK                 0x00000080
#define ESC_ESC_STATUS_ESC_STAT_BIAS_OVR_CRNT_ALIGN                0
#define ESC_ESC_STATUS_ESC_STAT_BIAS_OVR_CRNT_BITS                 1
#define ESC_ESC_STATUS_ESC_STAT_BIAS_OVR_CRNT_SHIFT                7

/* ESC :: ESC_STATUS :: ESC_STAT_MPD_FAULT [06:06] */
#define ESC_ESC_STATUS_ESC_STAT_MPD_FAULT_MASK                     0x00000040
#define ESC_ESC_STATUS_ESC_STAT_MPD_FAULT_ALIGN                    0
#define ESC_ESC_STATUS_ESC_STAT_MPD_FAULT_BITS                     1
#define ESC_ESC_STATUS_ESC_STAT_MPD_FAULT_SHIFT                    6

/* ESC :: ESC_STATUS :: ESC_STAT_EYESAFETY [05:05] */
#define ESC_ESC_STATUS_ESC_STAT_EYESAFETY_MASK                     0x00000020
#define ESC_ESC_STATUS_ESC_STAT_EYESAFETY_ALIGN                    0
#define ESC_ESC_STATUS_ESC_STAT_EYESAFETY_BITS                     1
#define ESC_ESC_STATUS_ESC_STAT_EYESAFETY_SHIFT                    5

/* ESC :: ESC_STATUS :: ESC_STAT_FAULT_OUTPUT_SIG [04:04] */
#define ESC_ESC_STATUS_ESC_STAT_FAULT_OUTPUT_SIG_MASK              0x00000010
#define ESC_ESC_STATUS_ESC_STAT_FAULT_OUTPUT_SIG_ALIGN             0
#define ESC_ESC_STATUS_ESC_STAT_FAULT_OUTPUT_SIG_BITS              1
#define ESC_ESC_STATUS_ESC_STAT_FAULT_OUTPUT_SIG_SHIFT             4

/* ESC :: ESC_STATUS :: ESC_STAT_LASER_IDAC_CTRL [03:03] */
#define ESC_ESC_STATUS_ESC_STAT_LASER_IDAC_CTRL_MASK               0x00000008
#define ESC_ESC_STATUS_ESC_STAT_LASER_IDAC_CTRL_ALIGN              0
#define ESC_ESC_STATUS_ESC_STAT_LASER_IDAC_CTRL_BITS               1
#define ESC_ESC_STATUS_ESC_STAT_LASER_IDAC_CTRL_SHIFT              3

/* ESC :: ESC_STATUS :: ESC_STAT_LASER_VCC_CTRL [02:02] */
#define ESC_ESC_STATUS_ESC_STAT_LASER_VCC_CTRL_MASK                0x00000004
#define ESC_ESC_STATUS_ESC_STAT_LASER_VCC_CTRL_ALIGN               0
#define ESC_ESC_STATUS_ESC_STAT_LASER_VCC_CTRL_BITS                1
#define ESC_ESC_STATUS_ESC_STAT_LASER_VCC_CTRL_SHIFT               2

/* ESC :: ESC_STATUS :: ESC_STAT_BYPASS_MODE [01:01] */
#define ESC_ESC_STATUS_ESC_STAT_BYPASS_MODE_MASK                   0x00000002
#define ESC_ESC_STATUS_ESC_STAT_BYPASS_MODE_ALIGN                  0
#define ESC_ESC_STATUS_ESC_STAT_BYPASS_MODE_BITS                   1
#define ESC_ESC_STATUS_ESC_STAT_BYPASS_MODE_SHIFT                  1

/* ESC :: ESC_STATUS :: ESC_STAT_MONITORING [00:00] */
#define ESC_ESC_STATUS_ESC_STAT_MONITORING_MASK                    0x00000001
#define ESC_ESC_STATUS_ESC_STAT_MONITORING_ALIGN                   0
#define ESC_ESC_STATUS_ESC_STAT_MONITORING_BITS                    1
#define ESC_ESC_STATUS_ESC_STAT_MONITORING_SHIFT                   0

/***************************************************************************
 *ESC_CONFIG1 - ESC Operating Mode Configuration 1
 ***************************************************************************/
/* ESC :: ESC_CONFIG1 :: reserved0 [31:29] */
#define ESC_ESC_CONFIG1_reserved0_MASK                             0xe0000000
#define ESC_ESC_CONFIG1_reserved0_ALIGN                            0
#define ESC_ESC_CONFIG1_reserved0_BITS                             3
#define ESC_ESC_CONFIG1_reserved0_SHIFT                            29

/* ESC :: ESC_CONFIG1 :: CFG_ESC_DEBUG_SELECT [28:28] */
#define ESC_ESC_CONFIG1_CFG_ESC_DEBUG_SELECT_MASK                  0x10000000
#define ESC_ESC_CONFIG1_CFG_ESC_DEBUG_SELECT_ALIGN                 0
#define ESC_ESC_CONFIG1_CFG_ESC_DEBUG_SELECT_BITS                  1
#define ESC_ESC_CONFIG1_CFG_ESC_DEBUG_SELECT_SHIFT                 28
#define ESC_ESC_CONFIG1_CFG_ESC_DEBUG_SELECT_DEFAULT               0

/* ESC :: ESC_CONFIG1 :: reserved1 [27:03] */
#define ESC_ESC_CONFIG1_reserved1_MASK                             0x0ffffff8
#define ESC_ESC_CONFIG1_reserved1_ALIGN                            0
#define ESC_ESC_CONFIG1_reserved1_BITS                             25
#define ESC_ESC_CONFIG1_reserved1_SHIFT                            3

/* ESC :: ESC_CONFIG1 :: CFG_ESC_BYPASS_EN [02:02] */
#define ESC_ESC_CONFIG1_CFG_ESC_BYPASS_EN_MASK                     0x00000004
#define ESC_ESC_CONFIG1_CFG_ESC_BYPASS_EN_ALIGN                    0
#define ESC_ESC_CONFIG1_CFG_ESC_BYPASS_EN_BITS                     1
#define ESC_ESC_CONFIG1_CFG_ESC_BYPASS_EN_SHIFT                    2
#define ESC_ESC_CONFIG1_CFG_ESC_BYPASS_EN_DEFAULT                  0

/* ESC :: ESC_CONFIG1 :: CFG_ESC_FIRST_BURST [01:01] */
#define ESC_ESC_CONFIG1_CFG_ESC_FIRST_BURST_MASK                   0x00000002
#define ESC_ESC_CONFIG1_CFG_ESC_FIRST_BURST_ALIGN                  0
#define ESC_ESC_CONFIG1_CFG_ESC_FIRST_BURST_BITS                   1
#define ESC_ESC_CONFIG1_CFG_ESC_FIRST_BURST_SHIFT                  1
#define ESC_ESC_CONFIG1_CFG_ESC_FIRST_BURST_DEFAULT                0

/* ESC :: ESC_CONFIG1 :: CFG_ESC_MONITOR_EN [00:00] */
#define ESC_ESC_CONFIG1_CFG_ESC_MONITOR_EN_MASK                    0x00000001
#define ESC_ESC_CONFIG1_CFG_ESC_MONITOR_EN_ALIGN                   0
#define ESC_ESC_CONFIG1_CFG_ESC_MONITOR_EN_BITS                    1
#define ESC_ESC_CONFIG1_CFG_ESC_MONITOR_EN_SHIFT                   0
#define ESC_ESC_CONFIG1_CFG_ESC_MONITOR_EN_DEFAULT                 0

/***************************************************************************
 *ESC_CONFIG2 - ESC Operating Mode Configuration 2
 ***************************************************************************/
/* ESC :: ESC_CONFIG2 :: reserved0 [31:07] */
#define ESC_ESC_CONFIG2_reserved0_MASK                             0xffffff80
#define ESC_ESC_CONFIG2_reserved0_ALIGN                            0
#define ESC_ESC_CONFIG2_reserved0_BITS                             25
#define ESC_ESC_CONFIG2_reserved0_SHIFT                            7

/* ESC :: ESC_CONFIG2 :: CFG_ESC_BYPASS_CRNT_ON [06:06] */
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_CRNT_ON_MASK                0x00000040
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_CRNT_ON_ALIGN               0
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_CRNT_ON_BITS                1
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_CRNT_ON_SHIFT               6
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_CRNT_ON_DEFAULT             0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_BYPASS_VCC_ON [05:05] */
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_VCC_ON_MASK                 0x00000020
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_VCC_ON_ALIGN                0
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_VCC_ON_BITS                 1
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_VCC_ON_SHIFT                5
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_VCC_ON_DEFAULT              0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_BYPASS_ON [04:04] */
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_ON_MASK                     0x00000010
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_ON_ALIGN                    0
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_ON_BITS                     1
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_ON_SHIFT                    4
#define ESC_ESC_CONFIG2_CFG_ESC_BYPASS_ON_DEFAULT                  0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_FORCE_CRNT_DIS [03:03] */
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_CRNT_DIS_MASK                0x00000008
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_CRNT_DIS_ALIGN               0
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_CRNT_DIS_BITS                1
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_CRNT_DIS_SHIFT               3
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_CRNT_DIS_DEFAULT             0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_FORCE_VCC_DIS [02:02] */
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_VCC_DIS_MASK                 0x00000004
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_VCC_DIS_ALIGN                0
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_VCC_DIS_BITS                 1
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_VCC_DIS_SHIFT                2
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_VCC_DIS_DEFAULT              0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_FORCE_ROGUE [01:01] */
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_ROGUE_MASK                   0x00000002
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_ROGUE_ALIGN                  0
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_ROGUE_BITS                   1
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_ROGUE_SHIFT                  1
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_ROGUE_DEFAULT                0

/* ESC :: ESC_CONFIG2 :: CFG_ESC_FORCE_EYESAFETY [00:00] */
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_EYESAFETY_MASK               0x00000001
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_EYESAFETY_ALIGN              0
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_EYESAFETY_BITS               1
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_EYESAFETY_SHIFT              0
#define ESC_ESC_CONFIG2_CFG_ESC_FORCE_EYESAFETY_DEFAULT            0

/***************************************************************************
 *ESC_ALARM_CONFIG - ESC Alarm Configuration
 ***************************************************************************/
/* ESC :: ESC_ALARM_CONFIG :: reserved0 [31:29] */
#define ESC_ESC_ALARM_CONFIG_reserved0_MASK                        0xe0000000
#define ESC_ESC_ALARM_CONFIG_reserved0_ALIGN                       0
#define ESC_ESC_ALARM_CONFIG_reserved0_BITS                        3
#define ESC_ESC_ALARM_CONFIG_reserved0_SHIFT                       29

/* ESC :: ESC_ALARM_CONFIG :: CFG_ESC_EYESAFE_OFF_RATE [28:24] */
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_OFF_RATE_MASK         0x1f000000
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_OFF_RATE_ALIGN        0
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_OFF_RATE_BITS         5
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_OFF_RATE_SHIFT        24
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_OFF_RATE_DEFAULT      31

/* ESC :: ESC_ALARM_CONFIG :: reserved1 [23:21] */
#define ESC_ESC_ALARM_CONFIG_reserved1_MASK                        0x00e00000
#define ESC_ESC_ALARM_CONFIG_reserved1_ALIGN                       0
#define ESC_ESC_ALARM_CONFIG_reserved1_BITS                        3
#define ESC_ESC_ALARM_CONFIG_reserved1_SHIFT                       21

/* ESC :: ESC_ALARM_CONFIG :: CFG_ESC_EYESAFE_ON_RATE [20:16] */
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ON_RATE_MASK          0x001f0000
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ON_RATE_ALIGN         0
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ON_RATE_BITS          5
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ON_RATE_SHIFT         16
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ON_RATE_DEFAULT       2

/* ESC :: ESC_ALARM_CONFIG :: CFG_ESC_EYESAFE_ALRM_THR [15:00] */
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ALRM_THR_MASK         0x0000ffff
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ALRM_THR_ALIGN        0
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ALRM_THR_BITS         16
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ALRM_THR_SHIFT        0
#define ESC_ESC_ALARM_CONFIG_CFG_ESC_EYESAFE_ALRM_THR_DEFAULT      1

/***************************************************************************
 *ESC_ROGUE_ALARM_CONFIG1 - ESC Rogue Alarm Configuration 1
 ***************************************************************************/
/* ESC :: ESC_ROGUE_ALARM_CONFIG1 :: reserved0 [31:26] */
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved0_MASK                 0xfc000000
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved0_ALIGN                0
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved0_BITS                 6
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved0_SHIFT                26

/* ESC :: ESC_ROGUE_ALARM_CONFIG1 :: CFG_ESC_ROGUE_ALRM_THR [25:16] */
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_ROGUE_ALRM_THR_MASK    0x03ff0000
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_ROGUE_ALRM_THR_ALIGN   0
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_ROGUE_ALRM_THR_BITS    10
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_ROGUE_ALRM_THR_SHIFT   16
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_ROGUE_ALRM_THR_DEFAULT 10

/* ESC :: ESC_ROGUE_ALARM_CONFIG1 :: reserved1 [15:12] */
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved1_MASK                 0x0000f000
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved1_ALIGN                0
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved1_BITS                 4
#define ESC_ESC_ROGUE_ALARM_CONFIG1_reserved1_SHIFT                12

/* ESC :: ESC_ROGUE_ALARM_CONFIG1 :: CFG_ESC_BE_TXSD_DLY [11:00] */
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_BE_TXSD_DLY_MASK       0x00000fff
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_BE_TXSD_DLY_ALIGN      0
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_BE_TXSD_DLY_BITS       12
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_BE_TXSD_DLY_SHIFT      0
#define ESC_ESC_ROGUE_ALARM_CONFIG1_CFG_ESC_BE_TXSD_DLY_DEFAULT    2

/***************************************************************************
 *ESC_ROGUE_ALARM_CONFIG2 - ESC Rogue Alarm Configuration 2
 ***************************************************************************/
/* ESC :: ESC_ROGUE_ALARM_CONFIG2 :: reserved0 [31:08] */
#define ESC_ESC_ROGUE_ALARM_CONFIG2_reserved0_MASK                 0xffffff00
#define ESC_ESC_ROGUE_ALARM_CONFIG2_reserved0_ALIGN                0
#define ESC_ESC_ROGUE_ALARM_CONFIG2_reserved0_BITS                 24
#define ESC_ESC_ROGUE_ALARM_CONFIG2_reserved0_SHIFT                8

/* ESC :: ESC_ROGUE_ALARM_CONFIG2 :: CFG_ESC_ROGUE_OFF_RATE [07:04] */
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_OFF_RATE_MASK    0x000000f0
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_OFF_RATE_ALIGN   0
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_OFF_RATE_BITS    4
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_OFF_RATE_SHIFT   4
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_OFF_RATE_DEFAULT 15

/* ESC :: ESC_ROGUE_ALARM_CONFIG2 :: CFG_ESC_ROGUE_ON_RATE [03:00] */
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_ON_RATE_MASK     0x0000000f
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_ON_RATE_ALIGN    0
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_ON_RATE_BITS     4
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_ON_RATE_SHIFT    0
#define ESC_ESC_ROGUE_ALARM_CONFIG2_CFG_ESC_ROGUE_ON_RATE_DEFAULT  2

/***************************************************************************
 *OVER_CURRENT_ALARM_CONFIG1 - ESC Over Current Alarm Configuration 1
 ***************************************************************************/
/* ESC :: OVER_CURRENT_ALARM_CONFIG1 :: reserved0 [31:27] */
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved0_MASK              0xf8000000
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved0_ALIGN             0
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved0_BITS              5
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved0_SHIFT             27

/* ESC :: OVER_CURRENT_ALARM_CONFIG1 :: CFG_ESC_MAX_MOD_CRNT [26:16] */
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_MOD_CRNT_MASK   0x07ff0000
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_MOD_CRNT_ALIGN  0
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_MOD_CRNT_BITS   11
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_MOD_CRNT_SHIFT  16
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_MOD_CRNT_DEFAULT 0

/* ESC :: OVER_CURRENT_ALARM_CONFIG1 :: reserved1 [15:11] */
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved1_MASK              0x0000f800
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved1_ALIGN             0
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved1_BITS              5
#define ESC_OVER_CURRENT_ALARM_CONFIG1_reserved1_SHIFT             11

/* ESC :: OVER_CURRENT_ALARM_CONFIG1 :: CFG_ESC_MAX_BIAS_CRNT [10:00] */
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_BIAS_CRNT_MASK  0x000007ff
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_BIAS_CRNT_ALIGN 0
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_BIAS_CRNT_BITS  11
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_BIAS_CRNT_SHIFT 0
#define ESC_OVER_CURRENT_ALARM_CONFIG1_CFG_ESC_MAX_BIAS_CRNT_DEFAULT 0

/***************************************************************************
 *OVER_CURRENT_ALARM_CONFIG2 - ESC Over Current Alarm Configuration 2
 ***************************************************************************/
/* ESC :: OVER_CURRENT_ALARM_CONFIG2 :: reserved0 [31:12] */
#define ESC_OVER_CURRENT_ALARM_CONFIG2_reserved0_MASK              0xfffff000
#define ESC_OVER_CURRENT_ALARM_CONFIG2_reserved0_ALIGN             0
#define ESC_OVER_CURRENT_ALARM_CONFIG2_reserved0_BITS              20
#define ESC_OVER_CURRENT_ALARM_CONFIG2_reserved0_SHIFT             12

/* ESC :: OVER_CURRENT_ALARM_CONFIG2 :: CFG_ESC_MAX_TOT_CRNT [11:00] */
#define ESC_OVER_CURRENT_ALARM_CONFIG2_CFG_ESC_MAX_TOT_CRNT_MASK   0x00000fff
#define ESC_OVER_CURRENT_ALARM_CONFIG2_CFG_ESC_MAX_TOT_CRNT_ALIGN  0
#define ESC_OVER_CURRENT_ALARM_CONFIG2_CFG_ESC_MAX_TOT_CRNT_BITS   12
#define ESC_OVER_CURRENT_ALARM_CONFIG2_CFG_ESC_MAX_TOT_CRNT_SHIFT  0
#define ESC_OVER_CURRENT_ALARM_CONFIG2_CFG_ESC_MAX_TOT_CRNT_DEFAULT 0

/***************************************************************************
 *ESC_MPD_FAULT_ALARM_CONFIG - ESC MPD fault alarm configuration
 ***************************************************************************/
/* ESC :: ESC_MPD_FAULT_ALARM_CONFIG :: reserved0 [31:25] */
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_reserved0_MASK              0xfe000000
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_reserved0_ALIGN             0
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_reserved0_BITS              7
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_reserved0_SHIFT             25

/* ESC :: ESC_MPD_FAULT_ALARM_CONFIG :: CFG_ESC_MPD_LVL_OR_EDGE [24:24] */
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_LVL_OR_EDGE_MASK 0x01000000
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_LVL_OR_EDGE_ALIGN 0
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_LVL_OR_EDGE_BITS 1
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_LVL_OR_EDGE_SHIFT 24
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_LVL_OR_EDGE_DEFAULT 0

/* ESC :: ESC_MPD_FAULT_ALARM_CONFIG :: CFG_ESC_MPD_CHANGE_THR [23:16] */
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHANGE_THR_MASK 0x00ff0000
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHANGE_THR_ALIGN 0
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHANGE_THR_BITS 8
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHANGE_THR_SHIFT 16
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHANGE_THR_DEFAULT 1

/* ESC :: ESC_MPD_FAULT_ALARM_CONFIG :: CFG_ESC_MPD_CHECK_TIME [15:00] */
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHECK_TIME_MASK 0x0000ffff
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHECK_TIME_ALIGN 0
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHECK_TIME_BITS 16
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHECK_TIME_SHIFT 0
#define ESC_ESC_MPD_FAULT_ALARM_CONFIG_CFG_ESC_MPD_CHECK_TIME_DEFAULT 500

/***************************************************************************
 *ESC_BE_ALARM_CONFIG - ESC BE fault alarm configuration
 ***************************************************************************/
/* ESC :: ESC_BE_ALARM_CONFIG :: reserved0 [31:24] */
#define ESC_ESC_BE_ALARM_CONFIG_reserved0_MASK                     0xff000000
#define ESC_ESC_BE_ALARM_CONFIG_reserved0_ALIGN                    0
#define ESC_ESC_BE_ALARM_CONFIG_reserved0_BITS                     8
#define ESC_ESC_BE_ALARM_CONFIG_reserved0_SHIFT                    24

/* ESC :: ESC_BE_ALARM_CONFIG :: CFG_ESC_EWAKE_BE_DURATION [23:16] */
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_EWAKE_BE_DURATION_MASK     0x00ff0000
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_EWAKE_BE_DURATION_ALIGN    0
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_EWAKE_BE_DURATION_BITS     8
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_EWAKE_BE_DURATION_SHIFT    16
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_EWAKE_BE_DURATION_DEFAULT  1

/* ESC :: ESC_BE_ALARM_CONFIG :: reserved1 [15:12] */
#define ESC_ESC_BE_ALARM_CONFIG_reserved1_MASK                     0x0000f000
#define ESC_ESC_BE_ALARM_CONFIG_reserved1_ALIGN                    0
#define ESC_ESC_BE_ALARM_CONFIG_reserved1_BITS                     4
#define ESC_ESC_BE_ALARM_CONFIG_reserved1_SHIFT                    12

/* ESC :: ESC_BE_ALARM_CONFIG :: CFG_ESC_BE_MAX_DURATION [11:00] */
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_BE_MAX_DURATION_MASK       0x00000fff
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_BE_MAX_DURATION_ALIGN      0
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_BE_MAX_DURATION_BITS       12
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_BE_MAX_DURATION_SHIFT      0
#define ESC_ESC_BE_ALARM_CONFIG_CFG_ESC_BE_MAX_DURATION_DEFAULT    2

/***************************************************************************
 *ESC_FIRST_BURST_LIMITATION - ESC First burst limit configuration
 ***************************************************************************/
/* ESC :: ESC_FIRST_BURST_LIMITATION :: reserved0 [31:12] */
#define ESC_ESC_FIRST_BURST_LIMITATION_reserved0_MASK              0xfffff000
#define ESC_ESC_FIRST_BURST_LIMITATION_reserved0_ALIGN             0
#define ESC_ESC_FIRST_BURST_LIMITATION_reserved0_BITS              20
#define ESC_ESC_FIRST_BURST_LIMITATION_reserved0_SHIFT             12

/* ESC :: ESC_FIRST_BURST_LIMITATION :: CFG_ESC_BURST1_MAX_DURATION [11:00] */
#define ESC_ESC_FIRST_BURST_LIMITATION_CFG_ESC_BURST1_MAX_DURATION_MASK 0x00000fff
#define ESC_ESC_FIRST_BURST_LIMITATION_CFG_ESC_BURST1_MAX_DURATION_ALIGN 0
#define ESC_ESC_FIRST_BURST_LIMITATION_CFG_ESC_BURST1_MAX_DURATION_BITS 12
#define ESC_ESC_FIRST_BURST_LIMITATION_CFG_ESC_BURST1_MAX_DURATION_SHIFT 0
#define ESC_ESC_FIRST_BURST_LIMITATION_CFG_ESC_BURST1_MAX_DURATION_DEFAULT 2

/***************************************************************************
 *ESC_MASK_ALARMS - ESC Alarm Action Mask
 ***************************************************************************/
/* ESC :: ESC_MASK_ALARMS :: reserved0 [31:12] */
#define ESC_ESC_MASK_ALARMS_reserved0_MASK                         0xfffff000
#define ESC_ESC_MASK_ALARMS_reserved0_ALIGN                        0
#define ESC_ESC_MASK_ALARMS_reserved0_BITS                         20
#define ESC_ESC_MASK_ALARMS_reserved0_SHIFT                        12

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_NO_REFCLK [11:10] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_NO_REFCLK_MASK            0x00000c00
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_NO_REFCLK_ALIGN           0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_NO_REFCLK_BITS            2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_NO_REFCLK_SHIFT           10
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_NO_REFCLK_DEFAULT         0

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_BE_FAULT [09:08] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_BE_FAULT_MASK             0x00000300
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_BE_FAULT_ALIGN            0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_BE_FAULT_BITS             2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_BE_FAULT_SHIFT            8
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_BE_FAULT_DEFAULT          0

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_ROGUE [07:06] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_ROGUE_MASK                0x000000c0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_ROGUE_ALIGN               0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_ROGUE_BITS                2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_ROGUE_SHIFT               6
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_ROGUE_DEFAULT             0

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_OVR_CRNT [05:04] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_OVR_CRNT_MASK             0x00000030
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_OVR_CRNT_ALIGN            0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_OVR_CRNT_BITS             2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_OVR_CRNT_SHIFT            4
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_OVR_CRNT_DEFAULT          0

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_MPD_FAULT [03:02] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_MPD_FAULT_MASK            0x0000000c
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_MPD_FAULT_ALIGN           0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_MPD_FAULT_BITS            2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_MPD_FAULT_SHIFT           2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_MPD_FAULT_DEFAULT         0

/* ESC :: ESC_MASK_ALARMS :: CFG_ESC_MASK_EYE_SAFETY [01:00] */
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_EYE_SAFETY_MASK           0x00000003
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_EYE_SAFETY_ALIGN          0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_EYE_SAFETY_BITS           2
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_EYE_SAFETY_SHIFT          0
#define ESC_ESC_MASK_ALARMS_CFG_ESC_MASK_EYE_SAFETY_DEFAULT        0

/***************************************************************************
 *ESC_INT - ESC Interrupt Status register
 ***************************************************************************/
/* ESC :: ESC_INT :: reserved0 [31:09] */
#define ESC_ESC_INT_reserved0_MASK                                 0xfffffe00
#define ESC_ESC_INT_reserved0_ALIGN                                0
#define ESC_ESC_INT_reserved0_BITS                                 23
#define ESC_ESC_INT_reserved0_SHIFT                                9

/* ESC :: ESC_INT :: ESC_LD_ROGUECMPOUT_INT [08:08] */
#define ESC_ESC_INT_ESC_LD_ROGUECMPOUT_INT_MASK                    0x00000100
#define ESC_ESC_INT_ESC_LD_ROGUECMPOUT_INT_ALIGN                   0
#define ESC_ESC_INT_ESC_LD_ROGUECMPOUT_INT_BITS                    1
#define ESC_ESC_INT_ESC_LD_ROGUECMPOUT_INT_SHIFT                   8
#define ESC_ESC_INT_ESC_LD_ROGUECMPOUT_INT_DEFAULT                 0

/* ESC :: ESC_INT :: ESC_LD_EYECMPOUT_INT [07:07] */
#define ESC_ESC_INT_ESC_LD_EYECMPOUT_INT_MASK                      0x00000080
#define ESC_ESC_INT_ESC_LD_EYECMPOUT_INT_ALIGN                     0
#define ESC_ESC_INT_ESC_LD_EYECMPOUT_INT_BITS                      1
#define ESC_ESC_INT_ESC_LD_EYECMPOUT_INT_SHIFT                     7
#define ESC_ESC_INT_ESC_LD_EYECMPOUT_INT_DEFAULT                   0

/* ESC :: ESC_INT :: ESC_EXT_FAULT_INPUT_INT [06:06] */
#define ESC_ESC_INT_ESC_EXT_FAULT_INPUT_INT_MASK                   0x00000040
#define ESC_ESC_INT_ESC_EXT_FAULT_INPUT_INT_ALIGN                  0
#define ESC_ESC_INT_ESC_EXT_FAULT_INPUT_INT_BITS                   1
#define ESC_ESC_INT_ESC_EXT_FAULT_INPUT_INT_SHIFT                  6
#define ESC_ESC_INT_ESC_EXT_FAULT_INPUT_INT_DEFAULT                0

/* ESC :: ESC_INT :: ESC_BE_ALARM_INT [05:05] */
#define ESC_ESC_INT_ESC_BE_ALARM_INT_MASK                          0x00000020
#define ESC_ESC_INT_ESC_BE_ALARM_INT_ALIGN                         0
#define ESC_ESC_INT_ESC_BE_ALARM_INT_BITS                          1
#define ESC_ESC_INT_ESC_BE_ALARM_INT_SHIFT                         5
#define ESC_ESC_INT_ESC_BE_ALARM_INT_DEFAULT                       0

/* ESC :: ESC_INT :: ESC_ROGUE_ALARM_INT [04:04] */
#define ESC_ESC_INT_ESC_ROGUE_ALARM_INT_MASK                       0x00000010
#define ESC_ESC_INT_ESC_ROGUE_ALARM_INT_ALIGN                      0
#define ESC_ESC_INT_ESC_ROGUE_ALARM_INT_BITS                       1
#define ESC_ESC_INT_ESC_ROGUE_ALARM_INT_SHIFT                      4
#define ESC_ESC_INT_ESC_ROGUE_ALARM_INT_DEFAULT                    0

/* ESC :: ESC_INT :: ESC_MOD_OVR_CRNT_ALARM_INT [03:03] */
#define ESC_ESC_INT_ESC_MOD_OVR_CRNT_ALARM_INT_MASK                0x00000008
#define ESC_ESC_INT_ESC_MOD_OVR_CRNT_ALARM_INT_ALIGN               0
#define ESC_ESC_INT_ESC_MOD_OVR_CRNT_ALARM_INT_BITS                1
#define ESC_ESC_INT_ESC_MOD_OVR_CRNT_ALARM_INT_SHIFT               3
#define ESC_ESC_INT_ESC_MOD_OVR_CRNT_ALARM_INT_DEFAULT             0

/* ESC :: ESC_INT :: ESC_BIAS_OVR_CRNT_ALARM_INT [02:02] */
#define ESC_ESC_INT_ESC_BIAS_OVR_CRNT_ALARM_INT_MASK               0x00000004
#define ESC_ESC_INT_ESC_BIAS_OVR_CRNT_ALARM_INT_ALIGN              0
#define ESC_ESC_INT_ESC_BIAS_OVR_CRNT_ALARM_INT_BITS               1
#define ESC_ESC_INT_ESC_BIAS_OVR_CRNT_ALARM_INT_SHIFT              2
#define ESC_ESC_INT_ESC_BIAS_OVR_CRNT_ALARM_INT_DEFAULT            0

/* ESC :: ESC_INT :: ESC_MPD_FAULT_ALARM_INT [01:01] */
#define ESC_ESC_INT_ESC_MPD_FAULT_ALARM_INT_MASK                   0x00000002
#define ESC_ESC_INT_ESC_MPD_FAULT_ALARM_INT_ALIGN                  0
#define ESC_ESC_INT_ESC_MPD_FAULT_ALARM_INT_BITS                   1
#define ESC_ESC_INT_ESC_MPD_FAULT_ALARM_INT_SHIFT                  1
#define ESC_ESC_INT_ESC_MPD_FAULT_ALARM_INT_DEFAULT                0

/* ESC :: ESC_INT :: ESC_EYESAFE_ALARM_INT [00:00] */
#define ESC_ESC_INT_ESC_EYESAFE_ALARM_INT_MASK                     0x00000001
#define ESC_ESC_INT_ESC_EYESAFE_ALARM_INT_ALIGN                    0
#define ESC_ESC_INT_ESC_EYESAFE_ALARM_INT_BITS                     1
#define ESC_ESC_INT_ESC_EYESAFE_ALARM_INT_SHIFT                    0
#define ESC_ESC_INT_ESC_EYESAFE_ALARM_INT_DEFAULT                  0

/***************************************************************************
 *ESC_INT_EN - ESC Interrupt Enable/Mask register
 ***************************************************************************/
/* ESC :: ESC_INT_EN :: reserved0 [31:09] */
#define ESC_ESC_INT_EN_reserved0_MASK                              0xfffffe00
#define ESC_ESC_INT_EN_reserved0_ALIGN                             0
#define ESC_ESC_INT_EN_reserved0_BITS                              23
#define ESC_ESC_INT_EN_reserved0_SHIFT                             9

/* ESC :: ESC_INT_EN :: ESC_LD_ROGUECMPOUT_INT_EN [08:08] */
#define ESC_ESC_INT_EN_ESC_LD_ROGUECMPOUT_INT_EN_MASK              0x00000100
#define ESC_ESC_INT_EN_ESC_LD_ROGUECMPOUT_INT_EN_ALIGN             0
#define ESC_ESC_INT_EN_ESC_LD_ROGUECMPOUT_INT_EN_BITS              1
#define ESC_ESC_INT_EN_ESC_LD_ROGUECMPOUT_INT_EN_SHIFT             8
#define ESC_ESC_INT_EN_ESC_LD_ROGUECMPOUT_INT_EN_DEFAULT           0

/* ESC :: ESC_INT_EN :: ESC_LD_EYECMPOUT_INT_EN [07:07] */
#define ESC_ESC_INT_EN_ESC_LD_EYECMPOUT_INT_EN_MASK                0x00000080
#define ESC_ESC_INT_EN_ESC_LD_EYECMPOUT_INT_EN_ALIGN               0
#define ESC_ESC_INT_EN_ESC_LD_EYECMPOUT_INT_EN_BITS                1
#define ESC_ESC_INT_EN_ESC_LD_EYECMPOUT_INT_EN_SHIFT               7
#define ESC_ESC_INT_EN_ESC_LD_EYECMPOUT_INT_EN_DEFAULT             0

/* ESC :: ESC_INT_EN :: ESC_EXT_FAULT_INPUT_INT_EN [06:06] */
#define ESC_ESC_INT_EN_ESC_EXT_FAULT_INPUT_INT_EN_MASK             0x00000040
#define ESC_ESC_INT_EN_ESC_EXT_FAULT_INPUT_INT_EN_ALIGN            0
#define ESC_ESC_INT_EN_ESC_EXT_FAULT_INPUT_INT_EN_BITS             1
#define ESC_ESC_INT_EN_ESC_EXT_FAULT_INPUT_INT_EN_SHIFT            6
#define ESC_ESC_INT_EN_ESC_EXT_FAULT_INPUT_INT_EN_DEFAULT          0

/* ESC :: ESC_INT_EN :: ESC_BE_ALARM_INT_EN [05:05] */
#define ESC_ESC_INT_EN_ESC_BE_ALARM_INT_EN_MASK                    0x00000020
#define ESC_ESC_INT_EN_ESC_BE_ALARM_INT_EN_ALIGN                   0
#define ESC_ESC_INT_EN_ESC_BE_ALARM_INT_EN_BITS                    1
#define ESC_ESC_INT_EN_ESC_BE_ALARM_INT_EN_SHIFT                   5
#define ESC_ESC_INT_EN_ESC_BE_ALARM_INT_EN_DEFAULT                 0

/* ESC :: ESC_INT_EN :: ESC_ROGUE_ALARM_INT_EN [04:04] */
#define ESC_ESC_INT_EN_ESC_ROGUE_ALARM_INT_EN_MASK                 0x00000010
#define ESC_ESC_INT_EN_ESC_ROGUE_ALARM_INT_EN_ALIGN                0
#define ESC_ESC_INT_EN_ESC_ROGUE_ALARM_INT_EN_BITS                 1
#define ESC_ESC_INT_EN_ESC_ROGUE_ALARM_INT_EN_SHIFT                4
#define ESC_ESC_INT_EN_ESC_ROGUE_ALARM_INT_EN_DEFAULT              0

/* ESC :: ESC_INT_EN :: ESC_MOD_OVR_CRNT_ALARM_INT_EN [03:03] */
#define ESC_ESC_INT_EN_ESC_MOD_OVR_CRNT_ALARM_INT_EN_MASK          0x00000008
#define ESC_ESC_INT_EN_ESC_MOD_OVR_CRNT_ALARM_INT_EN_ALIGN         0
#define ESC_ESC_INT_EN_ESC_MOD_OVR_CRNT_ALARM_INT_EN_BITS          1
#define ESC_ESC_INT_EN_ESC_MOD_OVR_CRNT_ALARM_INT_EN_SHIFT         3
#define ESC_ESC_INT_EN_ESC_MOD_OVR_CRNT_ALARM_INT_EN_DEFAULT       0

/* ESC :: ESC_INT_EN :: ESC_BIAS_OVR_CRNT_ALARM_INT_EN [02:02] */
#define ESC_ESC_INT_EN_ESC_BIAS_OVR_CRNT_ALARM_INT_EN_MASK         0x00000004
#define ESC_ESC_INT_EN_ESC_BIAS_OVR_CRNT_ALARM_INT_EN_ALIGN        0
#define ESC_ESC_INT_EN_ESC_BIAS_OVR_CRNT_ALARM_INT_EN_BITS         1
#define ESC_ESC_INT_EN_ESC_BIAS_OVR_CRNT_ALARM_INT_EN_SHIFT        2
#define ESC_ESC_INT_EN_ESC_BIAS_OVR_CRNT_ALARM_INT_EN_DEFAULT      0

/* ESC :: ESC_INT_EN :: ESC_MPD_FAULT_ALARM_INT_EN [01:01] */
#define ESC_ESC_INT_EN_ESC_MPD_FAULT_ALARM_INT_EN_MASK             0x00000002
#define ESC_ESC_INT_EN_ESC_MPD_FAULT_ALARM_INT_EN_ALIGN            0
#define ESC_ESC_INT_EN_ESC_MPD_FAULT_ALARM_INT_EN_BITS             1
#define ESC_ESC_INT_EN_ESC_MPD_FAULT_ALARM_INT_EN_SHIFT            1
#define ESC_ESC_INT_EN_ESC_MPD_FAULT_ALARM_INT_EN_DEFAULT          0

/* ESC :: ESC_INT_EN :: ESC_EYESAFE_ALARM_INT_EN [00:00] */
#define ESC_ESC_INT_EN_ESC_EYESAFE_ALARM_INT_EN_MASK               0x00000001
#define ESC_ESC_INT_EN_ESC_EYESAFE_ALARM_INT_EN_ALIGN              0
#define ESC_ESC_INT_EN_ESC_EYESAFE_ALARM_INT_EN_BITS               1
#define ESC_ESC_INT_EN_ESC_EYESAFE_ALARM_INT_EN_SHIFT              0
#define ESC_ESC_INT_EN_ESC_EYESAFE_ALARM_INT_EN_DEFAULT            0

/***************************************************************************
 *ESC_FAULT_OUT_CONFIGURATION - ESC alarm fault output configuration
 ***************************************************************************/
/* ESC :: ESC_FAULT_OUT_CONFIGURATION :: reserved0 [31:01] */
#define ESC_ESC_FAULT_OUT_CONFIGURATION_reserved0_MASK             0xfffffffe
#define ESC_ESC_FAULT_OUT_CONFIGURATION_reserved0_ALIGN            0
#define ESC_ESC_FAULT_OUT_CONFIGURATION_reserved0_BITS             31
#define ESC_ESC_FAULT_OUT_CONFIGURATION_reserved0_SHIFT            1

/* ESC :: ESC_FAULT_OUT_CONFIGURATION :: CFG_ESC_EXT_FAULT_INPUT [00:00] */
#define ESC_ESC_FAULT_OUT_CONFIGURATION_CFG_ESC_EXT_FAULT_INPUT_MASK 0x00000001
#define ESC_ESC_FAULT_OUT_CONFIGURATION_CFG_ESC_EXT_FAULT_INPUT_ALIGN 0
#define ESC_ESC_FAULT_OUT_CONFIGURATION_CFG_ESC_EXT_FAULT_INPUT_BITS 1
#define ESC_ESC_FAULT_OUT_CONFIGURATION_CFG_ESC_EXT_FAULT_INPUT_SHIFT 0
#define ESC_ESC_FAULT_OUT_CONFIGURATION_CFG_ESC_EXT_FAULT_INPUT_DEFAULT 0

/***************************************************************************
 *ESC_SPARE_SYNCHRONIZERS - ESC spare synchronizer input configuration
 ***************************************************************************/
/* ESC :: ESC_SPARE_SYNCHRONIZERS :: reserved0 [31:02] */
#define ESC_ESC_SPARE_SYNCHRONIZERS_reserved0_MASK                 0xfffffffc
#define ESC_ESC_SPARE_SYNCHRONIZERS_reserved0_ALIGN                0
#define ESC_ESC_SPARE_SYNCHRONIZERS_reserved0_BITS                 30
#define ESC_ESC_SPARE_SYNCHRONIZERS_reserved0_SHIFT                2

/* ESC :: ESC_SPARE_SYNCHRONIZERS :: CFG_ESC_SPARE_SYNC_IN2 [01:01] */
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN2_MASK    0x00000002
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN2_ALIGN   0
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN2_BITS    1
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN2_SHIFT   1
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN2_DEFAULT 0

/* ESC :: ESC_SPARE_SYNCHRONIZERS :: CFG_ESC_SPARE_SYNC_IN1 [00:00] */
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN1_MASK    0x00000001
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN1_ALIGN   0
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN1_BITS    1
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN1_SHIFT   0
#define ESC_ESC_SPARE_SYNCHRONIZERS_CFG_ESC_SPARE_SYNC_IN1_DEFAULT 0

/***************************************************************************
 *ESC_STAT_SPARE_SYNCHRONIZERS - ESC spare synchronizer output status
 ***************************************************************************/
/* ESC :: ESC_STAT_SPARE_SYNCHRONIZERS :: reserved0 [31:02] */
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_reserved0_MASK            0xfffffffc
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_reserved0_ALIGN           0
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_reserved0_BITS            30
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_reserved0_SHIFT           2

/* ESC :: ESC_STAT_SPARE_SYNCHRONIZERS :: ESC_STAT_SPARE_SYNC_OUT2 [01:01] */
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT2_MASK 0x00000002
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT2_ALIGN 0
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT2_BITS 1
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT2_SHIFT 1

/* ESC :: ESC_STAT_SPARE_SYNCHRONIZERS :: ESC_STAT_SPARE_SYNC_OUT1 [00:00] */
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT1_MASK 0x00000001
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT1_ALIGN 0
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT1_BITS 1
#define ESC_ESC_STAT_SPARE_SYNCHRONIZERS_ESC_STAT_SPARE_SYNC_OUT1_SHIFT 0

/***************************************************************************
 *ESC_PARAM - ESC Parameter Register
 ***************************************************************************/
/* ESC :: ESC_PARAM :: reserved_for_eco0 [31:00] */
#define ESC_ESC_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define ESC_ESC_PARAM_reserved_for_eco0_ALIGN                      0
#define ESC_ESC_PARAM_reserved_for_eco0_BITS                       32
#define ESC_ESC_PARAM_reserved_for_eco0_SHIFT                      0
#define ESC_ESC_PARAM_reserved_for_eco0_DEFAULT                    0

/***************************************************************************
 *ESC_ROGUE_ALARM_CONFIG3 - ESC Rogue Alarm Configuration 3
 ***************************************************************************/
/* ESC :: ESC_ROGUE_ALARM_CONFIG3 :: reserved0 [31:20] */
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved0_MASK                 0xfff00000
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved0_ALIGN                0
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved0_BITS                 12
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved0_SHIFT                20

/* ESC :: ESC_ROGUE_ALARM_CONFIG3 :: CFG_ESC_ROGUE_TXSD_DLY [19:16] */
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_ROGUE_TXSD_DLY_MASK    0x000f0000
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_ROGUE_TXSD_DLY_ALIGN   0
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_ROGUE_TXSD_DLY_BITS    4
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_ROGUE_TXSD_DLY_SHIFT   16
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_ROGUE_TXSD_DLY_DEFAULT 0

/* ESC :: ESC_ROGUE_ALARM_CONFIG3 :: reserved1 [15:12] */
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved1_MASK                 0x0000f000
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved1_ALIGN                0
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved1_BITS                 4
#define ESC_ESC_ROGUE_ALARM_CONFIG3_reserved1_SHIFT                12

/* ESC :: ESC_ROGUE_ALARM_CONFIG3 :: CFG_ESC_BE_OFF_TXSD_DLY [11:00] */
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_BE_OFF_TXSD_DLY_MASK   0x00000fff
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_BE_OFF_TXSD_DLY_ALIGN  0
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_BE_OFF_TXSD_DLY_BITS   12
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_BE_OFF_TXSD_DLY_SHIFT  0
#define ESC_ESC_ROGUE_ALARM_CONFIG3_CFG_ESC_BE_OFF_TXSD_DLY_DEFAULT 2

/***************************************************************************
 *ESC_MPD_ON_DELAY - ESC MPD on delay
 ***************************************************************************/
/* ESC :: ESC_MPD_ON_DELAY :: reserved0 [31:13] */
#define ESC_ESC_MPD_ON_DELAY_reserved0_MASK                        0xffffe000
#define ESC_ESC_MPD_ON_DELAY_reserved0_ALIGN                       0
#define ESC_ESC_MPD_ON_DELAY_reserved0_BITS                        19
#define ESC_ESC_MPD_ON_DELAY_reserved0_SHIFT                       13

/* ESC :: ESC_MPD_ON_DELAY :: CFG_ESC_MPD_ON_DELAY [12:00] */
#define ESC_ESC_MPD_ON_DELAY_CFG_ESC_MPD_ON_DELAY_MASK             0x00001fff
#define ESC_ESC_MPD_ON_DELAY_CFG_ESC_MPD_ON_DELAY_ALIGN            0
#define ESC_ESC_MPD_ON_DELAY_CFG_ESC_MPD_ON_DELAY_BITS             13
#define ESC_ESC_MPD_ON_DELAY_CFG_ESC_MPD_ON_DELAY_SHIFT            0
#define ESC_ESC_MPD_ON_DELAY_CFG_ESC_MPD_ON_DELAY_DEFAULT          0

#endif /* #ifndef ESC_H__ */

/* End of File */
