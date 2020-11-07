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

#ifndef LDC_H__
#define LDC_H__

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
#define LDC_LDC_CONFIG                           0x00000000 /* LDC configuration */
#define LDC_LDC_STATUS                           0x00000004 /* LDC operation status */
#define LDC_LDC_MPD_CONFIG                       0x00000008 /* LDC MPD configuration */
#define LDC_LDC_MPD_DAC_REF_0                    0x0000000c /* MPD path DAC control 0 */
#define LDC_LDC_MPD_DAC_REF_1                    0x00000010 /* MPD path DAC control 1 */
#define LDC_LDC_MPD_STROBEREF_0                  0x00000014 /* MPD path DAC strobe 0 */
#define LDC_LDC_MPD_STROBEREF_1                  0x00000018 /* MPD path DAC strobe 1 */
#define LDC_LDC_IBIASCTL                         0x0000001c /* Control word for bias IDAC */
#define LDC_LDC_IBIASEN                          0x00000020 /* Manual enable for bias IDAC */
#define LDC_LDC_IBIASSTROBE                      0x00000024 /* Active low latch enable for bias IDAC controls */
#define LDC_LDC_IDACBURSTENDEP                   0x00000028 /* Burst enable override */
#define LDC_LDC_IMODCTL                          0x0000002c /* Control word for modulation IDAC */
#define LDC_LDC_IMODEN                           0x00000030 /* Manual enable for modulation IDAC */
#define LDC_LDC_IMODSTROBE                       0x00000034 /* Active low latch enable for modulation IDAC controls */
#define LDC_LDC_TRIG_RSTMATCH                    0x00000038 /* Reset strobes for comparator match logic outputs */
#define LDC_LDC_TRIG_INFORCEDVAL                 0x0000003c /* Forced upstream data value */
#define LDC_LDC_TRIG_SETCOMMIT                   0x00000040 /* Active low latch enable for trigger controls */
#define LDC_LDC_BIAS_IDAC_CONFIG                 0x00000044 /* LDC bias IDAC configuration */
#define LDC_LDC_MOD_IDAC_CONFIG                  0x00000048 /* LDC modulation IDAC configuration */
#define LDC_LDC_PARAM                            0x0000004c /* LDC Parameter Register */
#define LDC_LDC_BIAS_IDAC_LIMITS                 0x00000050 /* LDC bias IDAC current limits */
#define LDC_LDC_MOD_IDAC_LIMITS                  0x00000054 /* LDC modulation IDAC current limits */
#define LDC_LDC_BIAS_STEP_SIZES                  0x00000058 /* LDC bias IDAC value search step sizes */
#define LDC_LDC_MOD_STEP_SIZES                   0x0000005c /* LDC modulation IDAC value search step sizes */
#define LDC_LDC_HST_CONFIG                       0x00000060 /* LDC histogram mode configuration */
#define LDC_LDC_HST_PAUSED                       0x00000064 /* LDC histogram paused indication */
#define LDC_LDC_HST_SAMPLE_COUNT                 0x00000068 /* LDC histogram sample count configuration */
#define LDC_LDC_HST_COMP0_RESULTS                0x0000006c /* LDC histogram result status for comparator 0 */
#define LDC_LDC_HST_COMP1_RESULTS                0x00000070 /* LDC histogram result status for comparator 1 */
#define LDC_LDC_CAL_CONFIG                       0x00000074 /* LDC offset calibration mode configuration */
#define LDC_LDC_CAL_SAMPLE_COUNT                 0x00000078 /* LDC offset calibration sample count configuration */
#define LDC_LDC_CAL_COMP0_RESULTS                0x0000007c /* LDC offset calibration result status for comparator 0 */
#define LDC_LDC_CAL_COMP1_RESULTS                0x00000080 /* LDC offset calibration result status for comparator 1 */
#define LDC_LDC_TRIG_MATCH_DELAY                 0x00000084 /* LDC cycle delay after trigger until MPD comparator is sampled */
#define LDC_LDC_ACL_CONFIG                       0x00000088 /* LDC Automatic Control Loop algorithm configuration */
#define LDC_LDC_ACL_BIAS_INT_DEFAULT             0x0000008c /* LDC ACL bias integral default value */
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC            0x00000090 /* LDC ACL bias integral gain for over threshold events */
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC            0x00000094 /* LDC ACL bias integral gain for under threshold events */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC           0x00000098 /* LDC ACL bias proportional gain for over threshold events */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC           0x0000009c /* LDC ACL bias proportional gain for under threshold events */
#define LDC_LDC_ACL_MOD_INT_DEFAULT              0x000000a0 /* LDC ACL modulation integral default value */
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC             0x000000a4 /* LDC ACL modulation integral gain for over threshold events */
#define LDC_LDC_ACL_MOD_INT_GAIN_INC             0x000000a8 /* LDC ACL modulation integral gain for under threshold events */
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC            0x000000ac /* LDC ACL modulation proportional gain for over threshold events */
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC            0x000000b0 /* LDC ACL modulation proportional gain for under threshold events */
#define LDC_LDC_CONFIG_1                         0x000000b4 /* LDC secondary configuration register */
#define LDC_LDC_STAT_AVG_BIAS                    0x000000b8 /* LDC ACL average bias IDAC value */
#define LDC_LDC_STAT_MAX_BIAS                    0x000000bc /* LDC ACL maximum bias IDAC value */
#define LDC_LDC_STAT_MIN_BIAS                    0x000000c0 /* LDC ACL minimum bias IDAC value */
#define LDC_LDC_STAT_AVG_MOD                     0x000000c4 /* LDC ACL average modulation IDAC value */
#define LDC_LDC_STAT_MAX_MOD                     0x000000c8 /* LDC ACL maximum modulation IDAC value */
#define LDC_LDC_STAT_MIN_MOD                     0x000000cc /* LDC ACL minimum modulation IDAC value */
#define LDC_LDC_ACL_METRICS_CONFIG               0x000000d0 /* LDC ACL metrics configuration */
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP           0x000000d4 /* LDC ACL maximum bias IDAC integral gain limit value */
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP            0x000000d8 /* LDC ACL maximum modulation IDAC integral gain limit value */
#define LDC_LDC_SLS_STROBE_SEL                   0x000000dc /* LDC Synchronized LD strobe selects */
#define LDC_LDC_PID_0                            0x000000e0 /* LDC Progressive IDAC Delays register 0 */
#define LDC_LDC_PID_1                            0x000000e4 /* LDC Progressive IDAC Delays register 1 */
#define LDC_LDC_AEC_CONFIG                       0x000000e8 /* LDC Active Ethernet Calibration algorithm configuration */

/***************************************************************************
 *LDC_CONFIG - LDC configuration
 ***************************************************************************/
/* LDC :: LDC_CONFIG :: reserved0 [31:08] */
#define LDC_LDC_CONFIG_reserved0_MASK                              0xffffff00
#define LDC_LDC_CONFIG_reserved0_ALIGN                             0
#define LDC_LDC_CONFIG_reserved0_BITS                              24
#define LDC_LDC_CONFIG_reserved0_SHIFT                             8

/* LDC :: LDC_CONFIG :: LDC_OPERATING_MODE [07:04] */
#define LDC_LDC_CONFIG_LDC_OPERATING_MODE_MASK                     0x000000f0
#define LDC_LDC_CONFIG_LDC_OPERATING_MODE_ALIGN                    0
#define LDC_LDC_CONFIG_LDC_OPERATING_MODE_BITS                     4
#define LDC_LDC_CONFIG_LDC_OPERATING_MODE_SHIFT                    4
#define LDC_LDC_CONFIG_LDC_OPERATING_MODE_DEFAULT                  0

/* LDC :: LDC_CONFIG :: reserved1 [03:02] */
#define LDC_LDC_CONFIG_reserved1_MASK                              0x0000000c
#define LDC_LDC_CONFIG_reserved1_ALIGN                             0
#define LDC_LDC_CONFIG_reserved1_BITS                              2
#define LDC_LDC_CONFIG_reserved1_SHIFT                             2

/* LDC :: LDC_CONFIG :: LDC_STROBE_CONFIG [01:01] */
#define LDC_LDC_CONFIG_LDC_STROBE_CONFIG_MASK                      0x00000002
#define LDC_LDC_CONFIG_LDC_STROBE_CONFIG_ALIGN                     0
#define LDC_LDC_CONFIG_LDC_STROBE_CONFIG_BITS                      1
#define LDC_LDC_CONFIG_LDC_STROBE_CONFIG_SHIFT                     1
#define LDC_LDC_CONFIG_LDC_STROBE_CONFIG_DEFAULT                   0

/* LDC :: LDC_CONFIG :: LDC_OVERRIDE [00:00] */
#define LDC_LDC_CONFIG_LDC_OVERRIDE_MASK                           0x00000001
#define LDC_LDC_CONFIG_LDC_OVERRIDE_ALIGN                          0
#define LDC_LDC_CONFIG_LDC_OVERRIDE_BITS                           1
#define LDC_LDC_CONFIG_LDC_OVERRIDE_SHIFT                          0
#define LDC_LDC_CONFIG_LDC_OVERRIDE_DEFAULT                        1

/***************************************************************************
 *LDC_STATUS - LDC operation status
 ***************************************************************************/
/* LDC :: LDC_STATUS :: reserved0 [31:06] */
#define LDC_LDC_STATUS_reserved0_MASK                              0xffffffc0
#define LDC_LDC_STATUS_reserved0_ALIGN                             0
#define LDC_LDC_STATUS_reserved0_BITS                              26
#define LDC_LDC_STATUS_reserved0_SHIFT                             6

/* LDC :: LDC_STATUS :: LDC_OPERATION_ERROR [05:05] */
#define LDC_LDC_STATUS_LDC_OPERATION_ERROR_MASK                    0x00000020
#define LDC_LDC_STATUS_LDC_OPERATION_ERROR_ALIGN                   0
#define LDC_LDC_STATUS_LDC_OPERATION_ERROR_BITS                    1
#define LDC_LDC_STATUS_LDC_OPERATION_ERROR_SHIFT                   5

/* LDC :: LDC_STATUS :: ACL_MOD_UNLOCKED [04:04] */
#define LDC_LDC_STATUS_ACL_MOD_UNLOCKED_MASK                       0x00000010
#define LDC_LDC_STATUS_ACL_MOD_UNLOCKED_ALIGN                      0
#define LDC_LDC_STATUS_ACL_MOD_UNLOCKED_BITS                       1
#define LDC_LDC_STATUS_ACL_MOD_UNLOCKED_SHIFT                      4

/* LDC :: LDC_STATUS :: ACL_MOD_LOCKED [03:03] */
#define LDC_LDC_STATUS_ACL_MOD_LOCKED_MASK                         0x00000008
#define LDC_LDC_STATUS_ACL_MOD_LOCKED_ALIGN                        0
#define LDC_LDC_STATUS_ACL_MOD_LOCKED_BITS                         1
#define LDC_LDC_STATUS_ACL_MOD_LOCKED_SHIFT                        3

/* LDC :: LDC_STATUS :: ACL_BIAS_UNLOCKED [02:02] */
#define LDC_LDC_STATUS_ACL_BIAS_UNLOCKED_MASK                      0x00000004
#define LDC_LDC_STATUS_ACL_BIAS_UNLOCKED_ALIGN                     0
#define LDC_LDC_STATUS_ACL_BIAS_UNLOCKED_BITS                      1
#define LDC_LDC_STATUS_ACL_BIAS_UNLOCKED_SHIFT                     2

/* LDC :: LDC_STATUS :: ACL_BIAS_LOCKED [01:01] */
#define LDC_LDC_STATUS_ACL_BIAS_LOCKED_MASK                        0x00000002
#define LDC_LDC_STATUS_ACL_BIAS_LOCKED_ALIGN                       0
#define LDC_LDC_STATUS_ACL_BIAS_LOCKED_BITS                        1
#define LDC_LDC_STATUS_ACL_BIAS_LOCKED_SHIFT                       1

/* LDC :: LDC_STATUS :: LDC_OPERATION_COMPLETE [00:00] */
#define LDC_LDC_STATUS_LDC_OPERATION_COMPLETE_MASK                 0x00000001
#define LDC_LDC_STATUS_LDC_OPERATION_COMPLETE_ALIGN                0
#define LDC_LDC_STATUS_LDC_OPERATION_COMPLETE_BITS                 1
#define LDC_LDC_STATUS_LDC_OPERATION_COMPLETE_SHIFT                0

/***************************************************************************
 *LDC_MPD_CONFIG - LDC MPD configuration
 ***************************************************************************/
/* LDC :: LDC_MPD_CONFIG :: reserved0 [31:16] */
#define LDC_LDC_MPD_CONFIG_reserved0_MASK                          0xffff0000
#define LDC_LDC_MPD_CONFIG_reserved0_ALIGN                         0
#define LDC_LDC_MPD_CONFIG_reserved0_BITS                          16
#define LDC_LDC_MPD_CONFIG_reserved0_SHIFT                         16

/* LDC :: LDC_MPD_CONFIG :: LDC_MPD_REF_VDAC_DELAY [15:08] */
#define LDC_LDC_MPD_CONFIG_LDC_MPD_REF_VDAC_DELAY_MASK             0x0000ff00
#define LDC_LDC_MPD_CONFIG_LDC_MPD_REF_VDAC_DELAY_ALIGN            0
#define LDC_LDC_MPD_CONFIG_LDC_MPD_REF_VDAC_DELAY_BITS             8
#define LDC_LDC_MPD_CONFIG_LDC_MPD_REF_VDAC_DELAY_SHIFT            8
#define LDC_LDC_MPD_CONFIG_LDC_MPD_REF_VDAC_DELAY_DEFAULT          25

/* LDC :: LDC_MPD_CONFIG :: LDC_MPD_CMP_DELAY [07:00] */
#define LDC_LDC_MPD_CONFIG_LDC_MPD_CMP_DELAY_MASK                  0x000000ff
#define LDC_LDC_MPD_CONFIG_LDC_MPD_CMP_DELAY_ALIGN                 0
#define LDC_LDC_MPD_CONFIG_LDC_MPD_CMP_DELAY_BITS                  8
#define LDC_LDC_MPD_CONFIG_LDC_MPD_CMP_DELAY_SHIFT                 0
#define LDC_LDC_MPD_CONFIG_LDC_MPD_CMP_DELAY_DEFAULT               4

/***************************************************************************
 *LDC_MPD_DAC_REF_0 - MPD path DAC control 0
 ***************************************************************************/
/* LDC :: LDC_MPD_DAC_REF_0 :: reserved0 [31:11] */
#define LDC_LDC_MPD_DAC_REF_0_reserved0_MASK                       0xfffff800
#define LDC_LDC_MPD_DAC_REF_0_reserved0_ALIGN                      0
#define LDC_LDC_MPD_DAC_REF_0_reserved0_BITS                       21
#define LDC_LDC_MPD_DAC_REF_0_reserved0_SHIFT                      11

/* LDC :: LDC_MPD_DAC_REF_0 :: STAT_LDC_MPD_DAC_REF_0 [10:00] */
#define LDC_LDC_MPD_DAC_REF_0_STAT_LDC_MPD_DAC_REF_0_MASK          0x000007ff
#define LDC_LDC_MPD_DAC_REF_0_STAT_LDC_MPD_DAC_REF_0_ALIGN         0
#define LDC_LDC_MPD_DAC_REF_0_STAT_LDC_MPD_DAC_REF_0_BITS          11
#define LDC_LDC_MPD_DAC_REF_0_STAT_LDC_MPD_DAC_REF_0_SHIFT         0

/***************************************************************************
 *LDC_MPD_DAC_REF_1 - MPD path DAC control 1
 ***************************************************************************/
/* LDC :: LDC_MPD_DAC_REF_1 :: reserved0 [31:11] */
#define LDC_LDC_MPD_DAC_REF_1_reserved0_MASK                       0xfffff800
#define LDC_LDC_MPD_DAC_REF_1_reserved0_ALIGN                      0
#define LDC_LDC_MPD_DAC_REF_1_reserved0_BITS                       21
#define LDC_LDC_MPD_DAC_REF_1_reserved0_SHIFT                      11

/* LDC :: LDC_MPD_DAC_REF_1 :: STAT_LDC_MPD_DAC_REF_1 [10:00] */
#define LDC_LDC_MPD_DAC_REF_1_STAT_LDC_MPD_DAC_REF_1_MASK          0x000007ff
#define LDC_LDC_MPD_DAC_REF_1_STAT_LDC_MPD_DAC_REF_1_ALIGN         0
#define LDC_LDC_MPD_DAC_REF_1_STAT_LDC_MPD_DAC_REF_1_BITS          11
#define LDC_LDC_MPD_DAC_REF_1_STAT_LDC_MPD_DAC_REF_1_SHIFT         0

/***************************************************************************
 *LDC_MPD_STROBEREF_0 - MPD path DAC strobe 0
 ***************************************************************************/
/* LDC :: LDC_MPD_STROBEREF_0 :: reserved0 [31:01] */
#define LDC_LDC_MPD_STROBEREF_0_reserved0_MASK                     0xfffffffe
#define LDC_LDC_MPD_STROBEREF_0_reserved0_ALIGN                    0
#define LDC_LDC_MPD_STROBEREF_0_reserved0_BITS                     31
#define LDC_LDC_MPD_STROBEREF_0_reserved0_SHIFT                    1

/* LDC :: LDC_MPD_STROBEREF_0 :: STAT_LDC_MPD_STROBEREF_0 [00:00] */
#define LDC_LDC_MPD_STROBEREF_0_STAT_LDC_MPD_STROBEREF_0_MASK      0x00000001
#define LDC_LDC_MPD_STROBEREF_0_STAT_LDC_MPD_STROBEREF_0_ALIGN     0
#define LDC_LDC_MPD_STROBEREF_0_STAT_LDC_MPD_STROBEREF_0_BITS      1
#define LDC_LDC_MPD_STROBEREF_0_STAT_LDC_MPD_STROBEREF_0_SHIFT     0

/***************************************************************************
 *LDC_MPD_STROBEREF_1 - MPD path DAC strobe 1
 ***************************************************************************/
/* LDC :: LDC_MPD_STROBEREF_1 :: reserved0 [31:01] */
#define LDC_LDC_MPD_STROBEREF_1_reserved0_MASK                     0xfffffffe
#define LDC_LDC_MPD_STROBEREF_1_reserved0_ALIGN                    0
#define LDC_LDC_MPD_STROBEREF_1_reserved0_BITS                     31
#define LDC_LDC_MPD_STROBEREF_1_reserved0_SHIFT                    1

/* LDC :: LDC_MPD_STROBEREF_1 :: STAT_LDC_MPD_STROBEREF_1 [00:00] */
#define LDC_LDC_MPD_STROBEREF_1_STAT_LDC_MPD_STROBEREF_1_MASK      0x00000001
#define LDC_LDC_MPD_STROBEREF_1_STAT_LDC_MPD_STROBEREF_1_ALIGN     0
#define LDC_LDC_MPD_STROBEREF_1_STAT_LDC_MPD_STROBEREF_1_BITS      1
#define LDC_LDC_MPD_STROBEREF_1_STAT_LDC_MPD_STROBEREF_1_SHIFT     0

/***************************************************************************
 *LDC_IBIASCTL - Control word for bias IDAC
 ***************************************************************************/
/* LDC :: LDC_IBIASCTL :: reserved0 [31:12] */
#define LDC_LDC_IBIASCTL_reserved0_MASK                            0xfffff000
#define LDC_LDC_IBIASCTL_reserved0_ALIGN                           0
#define LDC_LDC_IBIASCTL_reserved0_BITS                            20
#define LDC_LDC_IBIASCTL_reserved0_SHIFT                           12

/* LDC :: LDC_IBIASCTL :: STAT_LDC_IBIASCTL [11:00] */
#define LDC_LDC_IBIASCTL_STAT_LDC_IBIASCTL_MASK                    0x00000fff
#define LDC_LDC_IBIASCTL_STAT_LDC_IBIASCTL_ALIGN                   0
#define LDC_LDC_IBIASCTL_STAT_LDC_IBIASCTL_BITS                    12
#define LDC_LDC_IBIASCTL_STAT_LDC_IBIASCTL_SHIFT                   0

/***************************************************************************
 *LDC_IBIASEN - Manual enable for bias IDAC
 ***************************************************************************/
/* LDC :: LDC_IBIASEN :: reserved0 [31:01] */
#define LDC_LDC_IBIASEN_reserved0_MASK                             0xfffffffe
#define LDC_LDC_IBIASEN_reserved0_ALIGN                            0
#define LDC_LDC_IBIASEN_reserved0_BITS                             31
#define LDC_LDC_IBIASEN_reserved0_SHIFT                            1

/* LDC :: LDC_IBIASEN :: STAT_LDC_IBIASEN [00:00] */
#define LDC_LDC_IBIASEN_STAT_LDC_IBIASEN_MASK                      0x00000001
#define LDC_LDC_IBIASEN_STAT_LDC_IBIASEN_ALIGN                     0
#define LDC_LDC_IBIASEN_STAT_LDC_IBIASEN_BITS                      1
#define LDC_LDC_IBIASEN_STAT_LDC_IBIASEN_SHIFT                     0

/***************************************************************************
 *LDC_IBIASSTROBE - Active low latch enable for bias IDAC controls
 ***************************************************************************/
/* LDC :: LDC_IBIASSTROBE :: reserved0 [31:01] */
#define LDC_LDC_IBIASSTROBE_reserved0_MASK                         0xfffffffe
#define LDC_LDC_IBIASSTROBE_reserved0_ALIGN                        0
#define LDC_LDC_IBIASSTROBE_reserved0_BITS                         31
#define LDC_LDC_IBIASSTROBE_reserved0_SHIFT                        1

/* LDC :: LDC_IBIASSTROBE :: STAT_LDC_IBIASSTROBE [00:00] */
#define LDC_LDC_IBIASSTROBE_STAT_LDC_IBIASSTROBE_MASK              0x00000001
#define LDC_LDC_IBIASSTROBE_STAT_LDC_IBIASSTROBE_ALIGN             0
#define LDC_LDC_IBIASSTROBE_STAT_LDC_IBIASSTROBE_BITS              1
#define LDC_LDC_IBIASSTROBE_STAT_LDC_IBIASSTROBE_SHIFT             0

/***************************************************************************
 *LDC_IDACBURSTENDEP - Burst enable override
 ***************************************************************************/
/* LDC :: LDC_IDACBURSTENDEP :: reserved0 [31:01] */
#define LDC_LDC_IDACBURSTENDEP_reserved0_MASK                      0xfffffffe
#define LDC_LDC_IDACBURSTENDEP_reserved0_ALIGN                     0
#define LDC_LDC_IDACBURSTENDEP_reserved0_BITS                      31
#define LDC_LDC_IDACBURSTENDEP_reserved0_SHIFT                     1

/* LDC :: LDC_IDACBURSTENDEP :: STAT_LDC_IDACBURSTENDEP [00:00] */
#define LDC_LDC_IDACBURSTENDEP_STAT_LDC_IDACBURSTENDEP_MASK        0x00000001
#define LDC_LDC_IDACBURSTENDEP_STAT_LDC_IDACBURSTENDEP_ALIGN       0
#define LDC_LDC_IDACBURSTENDEP_STAT_LDC_IDACBURSTENDEP_BITS        1
#define LDC_LDC_IDACBURSTENDEP_STAT_LDC_IDACBURSTENDEP_SHIFT       0

/***************************************************************************
 *LDC_IMODCTL - Control word for modulation IDAC
 ***************************************************************************/
/* LDC :: LDC_IMODCTL :: reserved0 [31:11] */
#define LDC_LDC_IMODCTL_reserved0_MASK                             0xfffff800
#define LDC_LDC_IMODCTL_reserved0_ALIGN                            0
#define LDC_LDC_IMODCTL_reserved0_BITS                             21
#define LDC_LDC_IMODCTL_reserved0_SHIFT                            11

/* LDC :: LDC_IMODCTL :: STAT_LDC_IMODCTL [10:00] */
#define LDC_LDC_IMODCTL_STAT_LDC_IMODCTL_MASK                      0x000007ff
#define LDC_LDC_IMODCTL_STAT_LDC_IMODCTL_ALIGN                     0
#define LDC_LDC_IMODCTL_STAT_LDC_IMODCTL_BITS                      11
#define LDC_LDC_IMODCTL_STAT_LDC_IMODCTL_SHIFT                     0

/***************************************************************************
 *LDC_IMODEN - Manual enable for modulation IDAC
 ***************************************************************************/
/* LDC :: LDC_IMODEN :: reserved0 [31:01] */
#define LDC_LDC_IMODEN_reserved0_MASK                              0xfffffffe
#define LDC_LDC_IMODEN_reserved0_ALIGN                             0
#define LDC_LDC_IMODEN_reserved0_BITS                              31
#define LDC_LDC_IMODEN_reserved0_SHIFT                             1

/* LDC :: LDC_IMODEN :: STAT_LDC_IMODEN [00:00] */
#define LDC_LDC_IMODEN_STAT_LDC_IMODEN_MASK                        0x00000001
#define LDC_LDC_IMODEN_STAT_LDC_IMODEN_ALIGN                       0
#define LDC_LDC_IMODEN_STAT_LDC_IMODEN_BITS                        1
#define LDC_LDC_IMODEN_STAT_LDC_IMODEN_SHIFT                       0

/***************************************************************************
 *LDC_IMODSTROBE - Active low latch enable for modulation IDAC controls
 ***************************************************************************/
/* LDC :: LDC_IMODSTROBE :: reserved0 [31:01] */
#define LDC_LDC_IMODSTROBE_reserved0_MASK                          0xfffffffe
#define LDC_LDC_IMODSTROBE_reserved0_ALIGN                         0
#define LDC_LDC_IMODSTROBE_reserved0_BITS                          31
#define LDC_LDC_IMODSTROBE_reserved0_SHIFT                         1

/* LDC :: LDC_IMODSTROBE :: STAT_LDC_IMODSTROBE [00:00] */
#define LDC_LDC_IMODSTROBE_STAT_LDC_IMODSTROBE_MASK                0x00000001
#define LDC_LDC_IMODSTROBE_STAT_LDC_IMODSTROBE_ALIGN               0
#define LDC_LDC_IMODSTROBE_STAT_LDC_IMODSTROBE_BITS                1
#define LDC_LDC_IMODSTROBE_STAT_LDC_IMODSTROBE_SHIFT               0

/***************************************************************************
 *LDC_TRIG_RSTMATCH - Reset strobes for comparator match logic outputs
 ***************************************************************************/
/* LDC :: LDC_TRIG_RSTMATCH :: reserved0 [31:02] */
#define LDC_LDC_TRIG_RSTMATCH_reserved0_MASK                       0xfffffffc
#define LDC_LDC_TRIG_RSTMATCH_reserved0_ALIGN                      0
#define LDC_LDC_TRIG_RSTMATCH_reserved0_BITS                       30
#define LDC_LDC_TRIG_RSTMATCH_reserved0_SHIFT                      2

/* LDC :: LDC_TRIG_RSTMATCH :: STAT_LDC_TRIG_RSTMATCH [01:00] */
#define LDC_LDC_TRIG_RSTMATCH_STAT_LDC_TRIG_RSTMATCH_MASK          0x00000003
#define LDC_LDC_TRIG_RSTMATCH_STAT_LDC_TRIG_RSTMATCH_ALIGN         0
#define LDC_LDC_TRIG_RSTMATCH_STAT_LDC_TRIG_RSTMATCH_BITS          2
#define LDC_LDC_TRIG_RSTMATCH_STAT_LDC_TRIG_RSTMATCH_SHIFT         0

/***************************************************************************
 *LDC_TRIG_INFORCEDVAL - Forced upstream data value
 ***************************************************************************/
/* LDC :: LDC_TRIG_INFORCEDVAL :: reserved0 [31:01] */
#define LDC_LDC_TRIG_INFORCEDVAL_reserved0_MASK                    0xfffffffe
#define LDC_LDC_TRIG_INFORCEDVAL_reserved0_ALIGN                   0
#define LDC_LDC_TRIG_INFORCEDVAL_reserved0_BITS                    31
#define LDC_LDC_TRIG_INFORCEDVAL_reserved0_SHIFT                   1

/* LDC :: LDC_TRIG_INFORCEDVAL :: STAT_LDC_TRIG_INFORCEDVAL [00:00] */
#define LDC_LDC_TRIG_INFORCEDVAL_STAT_LDC_TRIG_INFORCEDVAL_MASK    0x00000001
#define LDC_LDC_TRIG_INFORCEDVAL_STAT_LDC_TRIG_INFORCEDVAL_ALIGN   0
#define LDC_LDC_TRIG_INFORCEDVAL_STAT_LDC_TRIG_INFORCEDVAL_BITS    1
#define LDC_LDC_TRIG_INFORCEDVAL_STAT_LDC_TRIG_INFORCEDVAL_SHIFT   0

/***************************************************************************
 *LDC_TRIG_SETCOMMIT - Active low latch enable for trigger controls
 ***************************************************************************/
/* LDC :: LDC_TRIG_SETCOMMIT :: reserved0 [31:01] */
#define LDC_LDC_TRIG_SETCOMMIT_reserved0_MASK                      0xfffffffe
#define LDC_LDC_TRIG_SETCOMMIT_reserved0_ALIGN                     0
#define LDC_LDC_TRIG_SETCOMMIT_reserved0_BITS                      31
#define LDC_LDC_TRIG_SETCOMMIT_reserved0_SHIFT                     1

/* LDC :: LDC_TRIG_SETCOMMIT :: STAT_LDC_TRIG_SETCOMMIT [00:00] */
#define LDC_LDC_TRIG_SETCOMMIT_STAT_LDC_TRIG_SETCOMMIT_MASK        0x00000001
#define LDC_LDC_TRIG_SETCOMMIT_STAT_LDC_TRIG_SETCOMMIT_ALIGN       0
#define LDC_LDC_TRIG_SETCOMMIT_STAT_LDC_TRIG_SETCOMMIT_BITS        1
#define LDC_LDC_TRIG_SETCOMMIT_STAT_LDC_TRIG_SETCOMMIT_SHIFT       0

/***************************************************************************
 *LDC_BIAS_IDAC_CONFIG - LDC bias IDAC configuration
 ***************************************************************************/
/* LDC :: LDC_BIAS_IDAC_CONFIG :: reserved0 [31:26] */
#define LDC_LDC_BIAS_IDAC_CONFIG_reserved0_MASK                    0xfc000000
#define LDC_LDC_BIAS_IDAC_CONFIG_reserved0_ALIGN                   0
#define LDC_LDC_BIAS_IDAC_CONFIG_reserved0_BITS                    6
#define LDC_LDC_BIAS_IDAC_CONFIG_reserved0_SHIFT                   26

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_IDAC_FORCE_BIT_0_LOW [25:25] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_FORCE_BIT_0_LOW_MASK 0x02000000
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_FORCE_BIT_0_LOW_ALIGN 0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_FORCE_BIT_0_LOW_BITS 1
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_FORCE_BIT_0_LOW_SHIFT 25
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_FORCE_BIT_0_LOW_DEFAULT 0

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_IDAC_MPD_CHANNEL_SEL [24:24] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_MPD_CHANNEL_SEL_MASK 0x01000000
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_MPD_CHANNEL_SEL_ALIGN 0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_MPD_CHANNEL_SEL_BITS 1
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_MPD_CHANNEL_SEL_SHIFT 24
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_MPD_CHANNEL_SEL_DEFAULT 0

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_AVERAGE_SAMPLES [23:20] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_AVERAGE_SAMPLES_MASK     0x00f00000
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_AVERAGE_SAMPLES_ALIGN    0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_AVERAGE_SAMPLES_BITS     4
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_AVERAGE_SAMPLES_SHIFT    20
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_AVERAGE_SAMPLES_DEFAULT  0

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_IDAC_UPDATE_LIMIT [19:16] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_UPDATE_LIMIT_MASK   0x000f0000
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_UPDATE_LIMIT_ALIGN  0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_UPDATE_LIMIT_BITS   4
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_UPDATE_LIMIT_SHIFT  16
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_UPDATE_LIMIT_DEFAULT 0

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_BE_DELAY [15:08] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_BE_DELAY_MASK            0x0000ff00
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_BE_DELAY_ALIGN           0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_BE_DELAY_BITS            8
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_BE_DELAY_SHIFT           8
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_BE_DELAY_DEFAULT         6

/* LDC :: LDC_BIAS_IDAC_CONFIG :: LDC_BIAS_IDAC_DELAY [07:00] */
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_DELAY_MASK          0x000000ff
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_DELAY_ALIGN         0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_DELAY_BITS          8
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_DELAY_SHIFT         0
#define LDC_LDC_BIAS_IDAC_CONFIG_LDC_BIAS_IDAC_DELAY_DEFAULT       6

/***************************************************************************
 *LDC_MOD_IDAC_CONFIG - LDC modulation IDAC configuration
 ***************************************************************************/
/* LDC :: LDC_MOD_IDAC_CONFIG :: reserved0 [31:25] */
#define LDC_LDC_MOD_IDAC_CONFIG_reserved0_MASK                     0xfe000000
#define LDC_LDC_MOD_IDAC_CONFIG_reserved0_ALIGN                    0
#define LDC_LDC_MOD_IDAC_CONFIG_reserved0_BITS                     7
#define LDC_LDC_MOD_IDAC_CONFIG_reserved0_SHIFT                    25

/* LDC :: LDC_MOD_IDAC_CONFIG :: LDC_MOD_IDAC_MPD_CHANNEL_SEL [24:24] */
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_MPD_CHANNEL_SEL_MASK  0x01000000
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_MPD_CHANNEL_SEL_ALIGN 0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_MPD_CHANNEL_SEL_BITS  1
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_MPD_CHANNEL_SEL_SHIFT 24
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_MPD_CHANNEL_SEL_DEFAULT 1

/* LDC :: LDC_MOD_IDAC_CONFIG :: LDC_MOD_AVERAGE_SAMPLES [23:20] */
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_AVERAGE_SAMPLES_MASK       0x00f00000
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_AVERAGE_SAMPLES_ALIGN      0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_AVERAGE_SAMPLES_BITS       4
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_AVERAGE_SAMPLES_SHIFT      20
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_AVERAGE_SAMPLES_DEFAULT    0

/* LDC :: LDC_MOD_IDAC_CONFIG :: LDC_MOD_IDAC_UPDATE_LIMIT [19:16] */
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_UPDATE_LIMIT_MASK     0x000f0000
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_UPDATE_LIMIT_ALIGN    0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_UPDATE_LIMIT_BITS     4
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_UPDATE_LIMIT_SHIFT    16
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_UPDATE_LIMIT_DEFAULT  0

/* LDC :: LDC_MOD_IDAC_CONFIG :: LDC_MOD_BE_DELAY [15:08] */
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_BE_DELAY_MASK              0x0000ff00
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_BE_DELAY_ALIGN             0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_BE_DELAY_BITS              8
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_BE_DELAY_SHIFT             8
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_BE_DELAY_DEFAULT           6

/* LDC :: LDC_MOD_IDAC_CONFIG :: LDC_MOD_IDAC_DELAY [07:00] */
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_DELAY_MASK            0x000000ff
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_DELAY_ALIGN           0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_DELAY_BITS            8
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_DELAY_SHIFT           0
#define LDC_LDC_MOD_IDAC_CONFIG_LDC_MOD_IDAC_DELAY_DEFAULT         6

/***************************************************************************
 *LDC_PARAM - LDC Parameter Register
 ***************************************************************************/
/* LDC :: LDC_PARAM :: reserved_for_eco0 [31:00] */
#define LDC_LDC_PARAM_reserved_for_eco0_MASK                       0xffffffff
#define LDC_LDC_PARAM_reserved_for_eco0_ALIGN                      0
#define LDC_LDC_PARAM_reserved_for_eco0_BITS                       32
#define LDC_LDC_PARAM_reserved_for_eco0_SHIFT                      0
#define LDC_LDC_PARAM_reserved_for_eco0_DEFAULT                    0

/***************************************************************************
 *LDC_BIAS_IDAC_LIMITS - LDC bias IDAC current limits
 ***************************************************************************/
/* LDC :: LDC_BIAS_IDAC_LIMITS :: reserved0 [31:24] */
#define LDC_LDC_BIAS_IDAC_LIMITS_reserved0_MASK                    0xff000000
#define LDC_LDC_BIAS_IDAC_LIMITS_reserved0_ALIGN                   0
#define LDC_LDC_BIAS_IDAC_LIMITS_reserved0_BITS                    8
#define LDC_LDC_BIAS_IDAC_LIMITS_reserved0_SHIFT                   24

/* LDC :: LDC_BIAS_IDAC_LIMITS :: LDC_BIAS_IDAC_MIN [23:12] */
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MIN_MASK            0x00fff000
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MIN_ALIGN           0
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MIN_BITS            12
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MIN_SHIFT           12
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MIN_DEFAULT         0

/* LDC :: LDC_BIAS_IDAC_LIMITS :: LDC_BIAS_IDAC_MAX [11:00] */
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MAX_MASK            0x00000fff
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MAX_ALIGN           0
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MAX_BITS            12
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MAX_SHIFT           0
#define LDC_LDC_BIAS_IDAC_LIMITS_LDC_BIAS_IDAC_MAX_DEFAULT         2047

/***************************************************************************
 *LDC_MOD_IDAC_LIMITS - LDC modulation IDAC current limits
 ***************************************************************************/
/* LDC :: LDC_MOD_IDAC_LIMITS :: reserved0 [31:22] */
#define LDC_LDC_MOD_IDAC_LIMITS_reserved0_MASK                     0xffc00000
#define LDC_LDC_MOD_IDAC_LIMITS_reserved0_ALIGN                    0
#define LDC_LDC_MOD_IDAC_LIMITS_reserved0_BITS                     10
#define LDC_LDC_MOD_IDAC_LIMITS_reserved0_SHIFT                    22

/* LDC :: LDC_MOD_IDAC_LIMITS :: LDC_MOD_IDAC_MIN [21:11] */
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MIN_MASK              0x003ff800
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MIN_ALIGN             0
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MIN_BITS              11
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MIN_SHIFT             11
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MIN_DEFAULT           0

/* LDC :: LDC_MOD_IDAC_LIMITS :: LDC_MOD_IDAC_MAX [10:00] */
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MAX_MASK              0x000007ff
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MAX_ALIGN             0
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MAX_BITS              11
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MAX_SHIFT             0
#define LDC_LDC_MOD_IDAC_LIMITS_LDC_MOD_IDAC_MAX_DEFAULT           2047

/***************************************************************************
 *LDC_BIAS_STEP_SIZES - LDC bias IDAC value search step sizes
 ***************************************************************************/
/* LDC :: LDC_BIAS_STEP_SIZES :: reserved0 [31:20] */
#define LDC_LDC_BIAS_STEP_SIZES_reserved0_MASK                     0xfff00000
#define LDC_LDC_BIAS_STEP_SIZES_reserved0_ALIGN                    0
#define LDC_LDC_BIAS_STEP_SIZES_reserved0_BITS                     12
#define LDC_LDC_BIAS_STEP_SIZES_reserved0_SHIFT                    20

/* LDC :: LDC_BIAS_STEP_SIZES :: LDC_FBL_STEP_SIZE [19:10] */
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBL_STEP_SIZE_MASK             0x000ffc00
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBL_STEP_SIZE_ALIGN            0
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBL_STEP_SIZE_BITS             10
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBL_STEP_SIZE_SHIFT            10
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBL_STEP_SIZE_DEFAULT          256

/* LDC :: LDC_BIAS_STEP_SIZES :: LDC_FBBS_STEP_SIZE [09:00] */
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBBS_STEP_SIZE_MASK            0x000003ff
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBBS_STEP_SIZE_ALIGN           0
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBBS_STEP_SIZE_BITS            10
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBBS_STEP_SIZE_SHIFT           0
#define LDC_LDC_BIAS_STEP_SIZES_LDC_FBBS_STEP_SIZE_DEFAULT         64

/***************************************************************************
 *LDC_MOD_STEP_SIZES - LDC modulation IDAC value search step sizes
 ***************************************************************************/
/* LDC :: LDC_MOD_STEP_SIZES :: reserved0 [31:20] */
#define LDC_LDC_MOD_STEP_SIZES_reserved0_MASK                      0xfff00000
#define LDC_LDC_MOD_STEP_SIZES_reserved0_ALIGN                     0
#define LDC_LDC_MOD_STEP_SIZES_reserved0_BITS                      12
#define LDC_LDC_MOD_STEP_SIZES_reserved0_SHIFT                     20

/* LDC :: LDC_MOD_STEP_SIZES :: LDC_FMI_STEP_SIZE [19:10] */
#define LDC_LDC_MOD_STEP_SIZES_LDC_FMI_STEP_SIZE_MASK              0x000ffc00
#define LDC_LDC_MOD_STEP_SIZES_LDC_FMI_STEP_SIZE_ALIGN             0
#define LDC_LDC_MOD_STEP_SIZES_LDC_FMI_STEP_SIZE_BITS              10
#define LDC_LDC_MOD_STEP_SIZES_LDC_FMI_STEP_SIZE_SHIFT             10
#define LDC_LDC_MOD_STEP_SIZES_LDC_FMI_STEP_SIZE_DEFAULT           256

/* LDC :: LDC_MOD_STEP_SIZES :: LDC_FBMS_STEP_SIZE [09:00] */
#define LDC_LDC_MOD_STEP_SIZES_LDC_FBMS_STEP_SIZE_MASK             0x000003ff
#define LDC_LDC_MOD_STEP_SIZES_LDC_FBMS_STEP_SIZE_ALIGN            0
#define LDC_LDC_MOD_STEP_SIZES_LDC_FBMS_STEP_SIZE_BITS             10
#define LDC_LDC_MOD_STEP_SIZES_LDC_FBMS_STEP_SIZE_SHIFT            0
#define LDC_LDC_MOD_STEP_SIZES_LDC_FBMS_STEP_SIZE_DEFAULT          64

/***************************************************************************
 *LDC_HST_CONFIG - LDC histogram mode configuration
 ***************************************************************************/
/* LDC :: LDC_HST_CONFIG :: HST_SAMPLES_PER_BURST [31:28] */
#define LDC_LDC_HST_CONFIG_HST_SAMPLES_PER_BURST_MASK              0xf0000000
#define LDC_LDC_HST_CONFIG_HST_SAMPLES_PER_BURST_ALIGN             0
#define LDC_LDC_HST_CONFIG_HST_SAMPLES_PER_BURST_BITS              4
#define LDC_LDC_HST_CONFIG_HST_SAMPLES_PER_BURST_SHIFT             28
#define LDC_LDC_HST_CONFIG_HST_SAMPLES_PER_BURST_DEFAULT           0

/* LDC :: LDC_HST_CONFIG :: HST_BE_ON_TIMER [27:20] */
#define LDC_LDC_HST_CONFIG_HST_BE_ON_TIMER_MASK                    0x0ff00000
#define LDC_LDC_HST_CONFIG_HST_BE_ON_TIMER_ALIGN                   0
#define LDC_LDC_HST_CONFIG_HST_BE_ON_TIMER_BITS                    8
#define LDC_LDC_HST_CONFIG_HST_BE_ON_TIMER_SHIFT                   20
#define LDC_LDC_HST_CONFIG_HST_BE_ON_TIMER_DEFAULT                 5

/* LDC :: LDC_HST_CONFIG :: reserved0 [19:17] */
#define LDC_LDC_HST_CONFIG_reserved0_MASK                          0x000e0000
#define LDC_LDC_HST_CONFIG_reserved0_ALIGN                         0
#define LDC_LDC_HST_CONFIG_reserved0_BITS                          3
#define LDC_LDC_HST_CONFIG_reserved0_SHIFT                         17

/* LDC :: LDC_HST_CONFIG :: HST_PAUSE [16:16] */
#define LDC_LDC_HST_CONFIG_HST_PAUSE_MASK                          0x00010000
#define LDC_LDC_HST_CONFIG_HST_PAUSE_ALIGN                         0
#define LDC_LDC_HST_CONFIG_HST_PAUSE_BITS                          1
#define LDC_LDC_HST_CONFIG_HST_PAUSE_SHIFT                         16
#define LDC_LDC_HST_CONFIG_HST_PAUSE_DEFAULT                       0

/* LDC :: LDC_HST_CONFIG :: reserved1 [15:13] */
#define LDC_LDC_HST_CONFIG_reserved1_MASK                          0x0000e000
#define LDC_LDC_HST_CONFIG_reserved1_ALIGN                         0
#define LDC_LDC_HST_CONFIG_reserved1_BITS                          3
#define LDC_LDC_HST_CONFIG_reserved1_SHIFT                         13

/* LDC :: LDC_HST_CONFIG :: HST_TRIG_SRC_COMP1 [12:12] */
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP1_MASK                 0x00001000
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP1_ALIGN                0
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP1_BITS                 1
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP1_SHIFT                12
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP1_DEFAULT              0

/* LDC :: LDC_HST_CONFIG :: reserved2 [11:09] */
#define LDC_LDC_HST_CONFIG_reserved2_MASK                          0x00000e00
#define LDC_LDC_HST_CONFIG_reserved2_ALIGN                         0
#define LDC_LDC_HST_CONFIG_reserved2_BITS                          3
#define LDC_LDC_HST_CONFIG_reserved2_SHIFT                         9

/* LDC :: LDC_HST_CONFIG :: HST_TRIG_SRC_COMP0 [08:08] */
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP0_MASK                 0x00000100
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP0_ALIGN                0
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP0_BITS                 1
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP0_SHIFT                8
#define LDC_LDC_HST_CONFIG_HST_TRIG_SRC_COMP0_DEFAULT              0

/* LDC :: LDC_HST_CONFIG :: reserved3 [07:05] */
#define LDC_LDC_HST_CONFIG_reserved3_MASK                          0x000000e0
#define LDC_LDC_HST_CONFIG_reserved3_ALIGN                         0
#define LDC_LDC_HST_CONFIG_reserved3_BITS                          3
#define LDC_LDC_HST_CONFIG_reserved3_SHIFT                         5

/* LDC :: LDC_HST_CONFIG :: HST_ENABLE_COMP1 [04:04] */
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP1_MASK                   0x00000010
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP1_ALIGN                  0
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP1_BITS                   1
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP1_SHIFT                  4
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP1_DEFAULT                0

/* LDC :: LDC_HST_CONFIG :: reserved4 [03:01] */
#define LDC_LDC_HST_CONFIG_reserved4_MASK                          0x0000000e
#define LDC_LDC_HST_CONFIG_reserved4_ALIGN                         0
#define LDC_LDC_HST_CONFIG_reserved4_BITS                          3
#define LDC_LDC_HST_CONFIG_reserved4_SHIFT                         1

/* LDC :: LDC_HST_CONFIG :: HST_ENABLE_COMP0 [00:00] */
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP0_MASK                   0x00000001
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP0_ALIGN                  0
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP0_BITS                   1
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP0_SHIFT                  0
#define LDC_LDC_HST_CONFIG_HST_ENABLE_COMP0_DEFAULT                0

/***************************************************************************
 *LDC_HST_PAUSED - LDC histogram paused indication
 ***************************************************************************/
/* LDC :: LDC_HST_PAUSED :: reserved0 [31:01] */
#define LDC_LDC_HST_PAUSED_reserved0_MASK                          0xfffffffe
#define LDC_LDC_HST_PAUSED_reserved0_ALIGN                         0
#define LDC_LDC_HST_PAUSED_reserved0_BITS                          31
#define LDC_LDC_HST_PAUSED_reserved0_SHIFT                         1

/* LDC :: LDC_HST_PAUSED :: HST_PAUSED [00:00] */
#define LDC_LDC_HST_PAUSED_HST_PAUSED_MASK                         0x00000001
#define LDC_LDC_HST_PAUSED_HST_PAUSED_ALIGN                        0
#define LDC_LDC_HST_PAUSED_HST_PAUSED_BITS                         1
#define LDC_LDC_HST_PAUSED_HST_PAUSED_SHIFT                        0

/***************************************************************************
 *LDC_HST_SAMPLE_COUNT - LDC histogram sample count configuration
 ***************************************************************************/
/* LDC :: LDC_HST_SAMPLE_COUNT :: HST_SAMPLE_COUNT_COMP1 [31:16] */
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP1_MASK       0xffff0000
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP1_ALIGN      0
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP1_BITS       16
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP1_SHIFT      16
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP1_DEFAULT    100

/* LDC :: LDC_HST_SAMPLE_COUNT :: HST_SAMPLE_COUNT_COMP0 [15:00] */
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP0_MASK       0x0000ffff
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP0_ALIGN      0
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP0_BITS       16
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP0_SHIFT      0
#define LDC_LDC_HST_SAMPLE_COUNT_HST_SAMPLE_COUNT_COMP0_DEFAULT    100

/***************************************************************************
 *LDC_HST_COMP0_RESULTS - LDC histogram result status for comparator 0
 ***************************************************************************/
/* LDC :: LDC_HST_COMP0_RESULTS :: HST_COMP0_ZEROS_CNT [31:16] */
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ZEROS_CNT_MASK         0xffff0000
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ZEROS_CNT_ALIGN        0
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ZEROS_CNT_BITS         16
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ZEROS_CNT_SHIFT        16

/* LDC :: LDC_HST_COMP0_RESULTS :: HST_COMP0_ONES_CNT [15:00] */
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ONES_CNT_MASK          0x0000ffff
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ONES_CNT_ALIGN         0
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ONES_CNT_BITS          16
#define LDC_LDC_HST_COMP0_RESULTS_HST_COMP0_ONES_CNT_SHIFT         0

/***************************************************************************
 *LDC_HST_COMP1_RESULTS - LDC histogram result status for comparator 1
 ***************************************************************************/
/* LDC :: LDC_HST_COMP1_RESULTS :: HST_COMP1_ZEROS_CNT [31:16] */
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ZEROS_CNT_MASK         0xffff0000
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ZEROS_CNT_ALIGN        0
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ZEROS_CNT_BITS         16
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ZEROS_CNT_SHIFT        16

/* LDC :: LDC_HST_COMP1_RESULTS :: HST_COMP1_ONES_CNT [15:00] */
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ONES_CNT_MASK          0x0000ffff
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ONES_CNT_ALIGN         0
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ONES_CNT_BITS          16
#define LDC_LDC_HST_COMP1_RESULTS_HST_COMP1_ONES_CNT_SHIFT         0

/***************************************************************************
 *LDC_CAL_CONFIG - LDC offset calibration mode configuration
 ***************************************************************************/
/* LDC :: LDC_CAL_CONFIG :: reserved0 [31:24] */
#define LDC_LDC_CAL_CONFIG_reserved0_MASK                          0xff000000
#define LDC_LDC_CAL_CONFIG_reserved0_ALIGN                         0
#define LDC_LDC_CAL_CONFIG_reserved0_BITS                          8
#define LDC_LDC_CAL_CONFIG_reserved0_SHIFT                         24

/* LDC :: LDC_CAL_CONFIG :: CAL_BE_OFF_TIMER [23:08] */
#define LDC_LDC_CAL_CONFIG_CAL_BE_OFF_TIMER_MASK                   0x00ffff00
#define LDC_LDC_CAL_CONFIG_CAL_BE_OFF_TIMER_ALIGN                  0
#define LDC_LDC_CAL_CONFIG_CAL_BE_OFF_TIMER_BITS                   16
#define LDC_LDC_CAL_CONFIG_CAL_BE_OFF_TIMER_SHIFT                  8
#define LDC_LDC_CAL_CONFIG_CAL_BE_OFF_TIMER_DEFAULT                5

/* LDC :: LDC_CAL_CONFIG :: reserved1 [07:06] */
#define LDC_LDC_CAL_CONFIG_reserved1_MASK                          0x000000c0
#define LDC_LDC_CAL_CONFIG_reserved1_ALIGN                         0
#define LDC_LDC_CAL_CONFIG_reserved1_BITS                          2
#define LDC_LDC_CAL_CONFIG_reserved1_SHIFT                         6

/* LDC :: LDC_CAL_CONFIG :: CAL_RUN_TO_COMPLETION [05:05] */
#define LDC_LDC_CAL_CONFIG_CAL_RUN_TO_COMPLETION_MASK              0x00000020
#define LDC_LDC_CAL_CONFIG_CAL_RUN_TO_COMPLETION_ALIGN             0
#define LDC_LDC_CAL_CONFIG_CAL_RUN_TO_COMPLETION_BITS              1
#define LDC_LDC_CAL_CONFIG_CAL_RUN_TO_COMPLETION_SHIFT             5
#define LDC_LDC_CAL_CONFIG_CAL_RUN_TO_COMPLETION_DEFAULT           0

/* LDC :: LDC_CAL_CONFIG :: CAL_ENABLE_COMP1 [04:04] */
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP1_MASK                   0x00000010
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP1_ALIGN                  0
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP1_BITS                   1
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP1_SHIFT                  4
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP1_DEFAULT                0

/* LDC :: LDC_CAL_CONFIG :: reserved2 [03:01] */
#define LDC_LDC_CAL_CONFIG_reserved2_MASK                          0x0000000e
#define LDC_LDC_CAL_CONFIG_reserved2_ALIGN                         0
#define LDC_LDC_CAL_CONFIG_reserved2_BITS                          3
#define LDC_LDC_CAL_CONFIG_reserved2_SHIFT                         1

/* LDC :: LDC_CAL_CONFIG :: CAL_ENABLE_COMP0 [00:00] */
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP0_MASK                   0x00000001
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP0_ALIGN                  0
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP0_BITS                   1
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP0_SHIFT                  0
#define LDC_LDC_CAL_CONFIG_CAL_ENABLE_COMP0_DEFAULT                0

/***************************************************************************
 *LDC_CAL_SAMPLE_COUNT - LDC offset calibration sample count configuration
 ***************************************************************************/
/* LDC :: LDC_CAL_SAMPLE_COUNT :: CAL_SAMPLE_COUNT_COMP1 [31:16] */
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP1_MASK       0xffff0000
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP1_ALIGN      0
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP1_BITS       16
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP1_SHIFT      16
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP1_DEFAULT    100

/* LDC :: LDC_CAL_SAMPLE_COUNT :: CAL_SAMPLE_COUNT_COMP0 [15:00] */
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP0_MASK       0x0000ffff
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP0_ALIGN      0
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP0_BITS       16
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP0_SHIFT      0
#define LDC_LDC_CAL_SAMPLE_COUNT_CAL_SAMPLE_COUNT_COMP0_DEFAULT    100

/***************************************************************************
 *LDC_CAL_COMP0_RESULTS - LDC offset calibration result status for comparator 0
 ***************************************************************************/
/* LDC :: LDC_CAL_COMP0_RESULTS :: CAL_COMP0_ZEROS_CNT [31:16] */
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ZEROS_CNT_MASK         0xffff0000
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ZEROS_CNT_ALIGN        0
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ZEROS_CNT_BITS         16
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ZEROS_CNT_SHIFT        16

/* LDC :: LDC_CAL_COMP0_RESULTS :: CAL_COMP0_ONES_CNT [15:00] */
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ONES_CNT_MASK          0x0000ffff
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ONES_CNT_ALIGN         0
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ONES_CNT_BITS          16
#define LDC_LDC_CAL_COMP0_RESULTS_CAL_COMP0_ONES_CNT_SHIFT         0

/***************************************************************************
 *LDC_CAL_COMP1_RESULTS - LDC offset calibration result status for comparator 1
 ***************************************************************************/
/* LDC :: LDC_CAL_COMP1_RESULTS :: CAL_COMP1_ZEROS_CNT [31:16] */
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ZEROS_CNT_MASK         0xffff0000
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ZEROS_CNT_ALIGN        0
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ZEROS_CNT_BITS         16
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ZEROS_CNT_SHIFT        16

/* LDC :: LDC_CAL_COMP1_RESULTS :: CAL_COMP1_ONES_CNT [15:00] */
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ONES_CNT_MASK          0x0000ffff
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ONES_CNT_ALIGN         0
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ONES_CNT_BITS          16
#define LDC_LDC_CAL_COMP1_RESULTS_CAL_COMP1_ONES_CNT_SHIFT         0

/***************************************************************************
 *LDC_TRIG_MATCH_DELAY - LDC cycle delay after trigger until MPD comparator is sampled
 ***************************************************************************/
/* LDC :: LDC_TRIG_MATCH_DELAY :: reserved0 [31:08] */
#define LDC_LDC_TRIG_MATCH_DELAY_reserved0_MASK                    0xffffff00
#define LDC_LDC_TRIG_MATCH_DELAY_reserved0_ALIGN                   0
#define LDC_LDC_TRIG_MATCH_DELAY_reserved0_BITS                    24
#define LDC_LDC_TRIG_MATCH_DELAY_reserved0_SHIFT                   8

/* LDC :: LDC_TRIG_MATCH_DELAY :: LDC_TRIG_MATCH_DELAY [07:00] */
#define LDC_LDC_TRIG_MATCH_DELAY_LDC_TRIG_MATCH_DELAY_MASK         0x000000ff
#define LDC_LDC_TRIG_MATCH_DELAY_LDC_TRIG_MATCH_DELAY_ALIGN        0
#define LDC_LDC_TRIG_MATCH_DELAY_LDC_TRIG_MATCH_DELAY_BITS         8
#define LDC_LDC_TRIG_MATCH_DELAY_LDC_TRIG_MATCH_DELAY_SHIFT        0
#define LDC_LDC_TRIG_MATCH_DELAY_LDC_TRIG_MATCH_DELAY_DEFAULT      2

/***************************************************************************
 *LDC_ACL_CONFIG - LDC Automatic Control Loop algorithm configuration
 ***************************************************************************/
/* LDC :: LDC_ACL_CONFIG :: reserved0 [31:07] */
#define LDC_LDC_ACL_CONFIG_reserved0_MASK                          0xffffff80
#define LDC_LDC_ACL_CONFIG_reserved0_ALIGN                         0
#define LDC_LDC_ACL_CONFIG_reserved0_BITS                          25
#define LDC_LDC_ACL_CONFIG_reserved0_SHIFT                         7

/* LDC :: LDC_ACL_CONFIG :: LDC_ALTERNATES_FORCE_VALUES [06:06] */
#define LDC_LDC_ACL_CONFIG_LDC_ALTERNATES_FORCE_VALUES_MASK        0x00000040
#define LDC_LDC_ACL_CONFIG_LDC_ALTERNATES_FORCE_VALUES_ALIGN       0
#define LDC_LDC_ACL_CONFIG_LDC_ALTERNATES_FORCE_VALUES_BITS        1
#define LDC_LDC_ACL_CONFIG_LDC_ALTERNATES_FORCE_VALUES_SHIFT       6
#define LDC_LDC_ACL_CONFIG_LDC_ALTERNATES_FORCE_VALUES_DEFAULT     0

/* LDC :: LDC_ACL_CONFIG :: LDC_SIGMA_DELTA_SCALE_FACTOR [05:02] */
#define LDC_LDC_ACL_CONFIG_LDC_SIGMA_DELTA_SCALE_FACTOR_MASK       0x0000003c
#define LDC_LDC_ACL_CONFIG_LDC_SIGMA_DELTA_SCALE_FACTOR_ALIGN      0
#define LDC_LDC_ACL_CONFIG_LDC_SIGMA_DELTA_SCALE_FACTOR_BITS       4
#define LDC_LDC_ACL_CONFIG_LDC_SIGMA_DELTA_SCALE_FACTOR_SHIFT      2
#define LDC_LDC_ACL_CONFIG_LDC_SIGMA_DELTA_SCALE_FACTOR_DEFAULT    0

/* LDC :: LDC_ACL_CONFIG :: LDC_EN_SIGMA_DELTA [01:01] */
#define LDC_LDC_ACL_CONFIG_LDC_EN_SIGMA_DELTA_MASK                 0x00000002
#define LDC_LDC_ACL_CONFIG_LDC_EN_SIGMA_DELTA_ALIGN                0
#define LDC_LDC_ACL_CONFIG_LDC_EN_SIGMA_DELTA_BITS                 1
#define LDC_LDC_ACL_CONFIG_LDC_EN_SIGMA_DELTA_SHIFT                1
#define LDC_LDC_ACL_CONFIG_LDC_EN_SIGMA_DELTA_DEFAULT              0

/* LDC :: LDC_ACL_CONFIG :: LDC_EN_ACTIVE_BE_UPDATES [00:00] */
#define LDC_LDC_ACL_CONFIG_LDC_EN_ACTIVE_BE_UPDATES_MASK           0x00000001
#define LDC_LDC_ACL_CONFIG_LDC_EN_ACTIVE_BE_UPDATES_ALIGN          0
#define LDC_LDC_ACL_CONFIG_LDC_EN_ACTIVE_BE_UPDATES_BITS           1
#define LDC_LDC_ACL_CONFIG_LDC_EN_ACTIVE_BE_UPDATES_SHIFT          0
#define LDC_LDC_ACL_CONFIG_LDC_EN_ACTIVE_BE_UPDATES_DEFAULT        0

/***************************************************************************
 *LDC_ACL_BIAS_INT_DEFAULT - LDC ACL bias integral default value
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_INT_DEFAULT :: reserved0 [31:24] */
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_reserved0_MASK                0xff000000
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_reserved0_ALIGN               0
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_reserved0_BITS                8
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_reserved0_SHIFT               24

/* LDC :: LDC_ACL_BIAS_INT_DEFAULT :: LDC_ACL_BIAS_INT_DEFAULT [23:00] */
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_LDC_ACL_BIAS_INT_DEFAULT_MASK 0x00ffffff
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_LDC_ACL_BIAS_INT_DEFAULT_ALIGN 0
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_LDC_ACL_BIAS_INT_DEFAULT_BITS 24
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_LDC_ACL_BIAS_INT_DEFAULT_SHIFT 0
#define LDC_LDC_ACL_BIAS_INT_DEFAULT_LDC_ACL_BIAS_INT_DEFAULT_DEFAULT 0

/***************************************************************************
 *LDC_ACL_BIAS_INT_GAIN_DEC - LDC ACL bias integral gain for over threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_INT_GAIN_DEC :: reserved0 [31:24] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_reserved0_MASK               0xff000000
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_reserved0_ALIGN              0
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_reserved0_BITS               8
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_reserved0_SHIFT              24

/* LDC :: LDC_ACL_BIAS_INT_GAIN_DEC :: LDC_ACL_BIAS_INT_GAIN_DEC [23:00] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_LDC_ACL_BIAS_INT_GAIN_DEC_MASK 0x00ffffff
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_LDC_ACL_BIAS_INT_GAIN_DEC_ALIGN 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_LDC_ACL_BIAS_INT_GAIN_DEC_BITS 24
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_LDC_ACL_BIAS_INT_GAIN_DEC_SHIFT 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_DEC_LDC_ACL_BIAS_INT_GAIN_DEC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_BIAS_INT_GAIN_INC - LDC ACL bias integral gain for under threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_INT_GAIN_INC :: reserved0 [31:24] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_reserved0_MASK               0xff000000
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_reserved0_ALIGN              0
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_reserved0_BITS               8
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_reserved0_SHIFT              24

/* LDC :: LDC_ACL_BIAS_INT_GAIN_INC :: LDC_ACL_BIAS_INT_GAIN_INC [23:00] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_LDC_ACL_BIAS_INT_GAIN_INC_MASK 0x00ffffff
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_LDC_ACL_BIAS_INT_GAIN_INC_ALIGN 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_LDC_ACL_BIAS_INT_GAIN_INC_BITS 24
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_LDC_ACL_BIAS_INT_GAIN_INC_SHIFT 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_INC_LDC_ACL_BIAS_INT_GAIN_INC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_BIAS_PROP_GAIN_DEC - LDC ACL bias proportional gain for over threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_PROP_GAIN_DEC :: reserved0 [31:24] */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_reserved0_MASK              0xff000000
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_reserved0_ALIGN             0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_reserved0_BITS              8
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_reserved0_SHIFT             24

/* LDC :: LDC_ACL_BIAS_PROP_GAIN_DEC :: LDC_ACL_BIAS_PROP_GAIN_DEC [23:00] */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_LDC_ACL_BIAS_PROP_GAIN_DEC_MASK 0x00ffffff
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_LDC_ACL_BIAS_PROP_GAIN_DEC_ALIGN 0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_LDC_ACL_BIAS_PROP_GAIN_DEC_BITS 24
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_LDC_ACL_BIAS_PROP_GAIN_DEC_SHIFT 0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_DEC_LDC_ACL_BIAS_PROP_GAIN_DEC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_BIAS_PROP_GAIN_INC - LDC ACL bias proportional gain for under threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_PROP_GAIN_INC :: reserved0 [31:24] */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_reserved0_MASK              0xff000000
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_reserved0_ALIGN             0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_reserved0_BITS              8
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_reserved0_SHIFT             24

/* LDC :: LDC_ACL_BIAS_PROP_GAIN_INC :: LDC_ACL_BIAS_PROP_GAIN_INC [23:00] */
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_LDC_ACL_BIAS_PROP_GAIN_INC_MASK 0x00ffffff
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_LDC_ACL_BIAS_PROP_GAIN_INC_ALIGN 0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_LDC_ACL_BIAS_PROP_GAIN_INC_BITS 24
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_LDC_ACL_BIAS_PROP_GAIN_INC_SHIFT 0
#define LDC_LDC_ACL_BIAS_PROP_GAIN_INC_LDC_ACL_BIAS_PROP_GAIN_INC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_INT_DEFAULT - LDC ACL modulation integral default value
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_INT_DEFAULT :: reserved0 [31:24] */
#define LDC_LDC_ACL_MOD_INT_DEFAULT_reserved0_MASK                 0xff000000
#define LDC_LDC_ACL_MOD_INT_DEFAULT_reserved0_ALIGN                0
#define LDC_LDC_ACL_MOD_INT_DEFAULT_reserved0_BITS                 8
#define LDC_LDC_ACL_MOD_INT_DEFAULT_reserved0_SHIFT                24

/* LDC :: LDC_ACL_MOD_INT_DEFAULT :: LDC_ACL_MOD_INT_DEFAULT [23:00] */
#define LDC_LDC_ACL_MOD_INT_DEFAULT_LDC_ACL_MOD_INT_DEFAULT_MASK   0x00ffffff
#define LDC_LDC_ACL_MOD_INT_DEFAULT_LDC_ACL_MOD_INT_DEFAULT_ALIGN  0
#define LDC_LDC_ACL_MOD_INT_DEFAULT_LDC_ACL_MOD_INT_DEFAULT_BITS   24
#define LDC_LDC_ACL_MOD_INT_DEFAULT_LDC_ACL_MOD_INT_DEFAULT_SHIFT  0
#define LDC_LDC_ACL_MOD_INT_DEFAULT_LDC_ACL_MOD_INT_DEFAULT_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_INT_GAIN_DEC - LDC ACL modulation integral gain for over threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_INT_GAIN_DEC :: reserved0 [31:24] */
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_reserved0_MASK                0xff000000
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_reserved0_ALIGN               0
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_reserved0_BITS                8
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_reserved0_SHIFT               24

/* LDC :: LDC_ACL_MOD_INT_GAIN_DEC :: LDC_ACL_MOD_INT_GAIN_DEC [23:00] */
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_LDC_ACL_MOD_INT_GAIN_DEC_MASK 0x00ffffff
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_LDC_ACL_MOD_INT_GAIN_DEC_ALIGN 0
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_LDC_ACL_MOD_INT_GAIN_DEC_BITS 24
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_LDC_ACL_MOD_INT_GAIN_DEC_SHIFT 0
#define LDC_LDC_ACL_MOD_INT_GAIN_DEC_LDC_ACL_MOD_INT_GAIN_DEC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_INT_GAIN_INC - LDC ACL modulation integral gain for under threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_INT_GAIN_INC :: reserved0 [31:24] */
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_reserved0_MASK                0xff000000
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_reserved0_ALIGN               0
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_reserved0_BITS                8
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_reserved0_SHIFT               24

/* LDC :: LDC_ACL_MOD_INT_GAIN_INC :: LDC_ACL_MOD_INT_GAIN_INC [23:00] */
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_LDC_ACL_MOD_INT_GAIN_INC_MASK 0x00ffffff
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_LDC_ACL_MOD_INT_GAIN_INC_ALIGN 0
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_LDC_ACL_MOD_INT_GAIN_INC_BITS 24
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_LDC_ACL_MOD_INT_GAIN_INC_SHIFT 0
#define LDC_LDC_ACL_MOD_INT_GAIN_INC_LDC_ACL_MOD_INT_GAIN_INC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_PROP_GAIN_DEC - LDC ACL modulation proportional gain for over threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_PROP_GAIN_DEC :: reserved0 [31:24] */
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_reserved0_MASK               0xff000000
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_reserved0_ALIGN              0
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_reserved0_BITS               8
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_reserved0_SHIFT              24

/* LDC :: LDC_ACL_MOD_PROP_GAIN_DEC :: LDC_ACL_MOD_PROP_GAIN_DEC [23:00] */
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_LDC_ACL_MOD_PROP_GAIN_DEC_MASK 0x00ffffff
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_LDC_ACL_MOD_PROP_GAIN_DEC_ALIGN 0
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_LDC_ACL_MOD_PROP_GAIN_DEC_BITS 24
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_LDC_ACL_MOD_PROP_GAIN_DEC_SHIFT 0
#define LDC_LDC_ACL_MOD_PROP_GAIN_DEC_LDC_ACL_MOD_PROP_GAIN_DEC_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_PROP_GAIN_INC - LDC ACL modulation proportional gain for under threshold events
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_PROP_GAIN_INC :: reserved0 [31:24] */
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_reserved0_MASK               0xff000000
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_reserved0_ALIGN              0
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_reserved0_BITS               8
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_reserved0_SHIFT              24

/* LDC :: LDC_ACL_MOD_PROP_GAIN_INC :: LDC_ACL_MOD_PROP_GAIN_INC [23:00] */
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_LDC_ACL_MOD_PROP_GAIN_INC_MASK 0x00ffffff
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_LDC_ACL_MOD_PROP_GAIN_INC_ALIGN 0
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_LDC_ACL_MOD_PROP_GAIN_INC_BITS 24
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_LDC_ACL_MOD_PROP_GAIN_INC_SHIFT 0
#define LDC_LDC_ACL_MOD_PROP_GAIN_INC_LDC_ACL_MOD_PROP_GAIN_INC_DEFAULT 0

/***************************************************************************
 *LDC_CONFIG_1 - LDC secondary configuration register
 ***************************************************************************/
/* LDC :: LDC_CONFIG_1 :: reserved0 [31:31] */
#define LDC_LDC_CONFIG_1_reserved0_MASK                            0x80000000
#define LDC_LDC_CONFIG_1_reserved0_ALIGN                           0
#define LDC_LDC_CONFIG_1_reserved0_BITS                            1
#define LDC_LDC_CONFIG_1_reserved0_SHIFT                           31

/* LDC :: LDC_CONFIG_1 :: LDC_ABS_FORCE_VALUE [30:30] */
#define LDC_LDC_CONFIG_1_LDC_ABS_FORCE_VALUE_MASK                  0x40000000
#define LDC_LDC_CONFIG_1_LDC_ABS_FORCE_VALUE_ALIGN                 0
#define LDC_LDC_CONFIG_1_LDC_ABS_FORCE_VALUE_BITS                  1
#define LDC_LDC_CONFIG_1_LDC_ABS_FORCE_VALUE_SHIFT                 30
#define LDC_LDC_CONFIG_1_LDC_ABS_FORCE_VALUE_DEFAULT               0

/* LDC :: LDC_CONFIG_1 :: LDC_ESC_HOLD_OVER_TC [29:26] */
#define LDC_LDC_CONFIG_1_LDC_ESC_HOLD_OVER_TC_MASK                 0x3c000000
#define LDC_LDC_CONFIG_1_LDC_ESC_HOLD_OVER_TC_ALIGN                0
#define LDC_LDC_CONFIG_1_LDC_ESC_HOLD_OVER_TC_BITS                 4
#define LDC_LDC_CONFIG_1_LDC_ESC_HOLD_OVER_TC_SHIFT                26
#define LDC_LDC_CONFIG_1_LDC_ESC_HOLD_OVER_TC_DEFAULT              0

/* LDC :: LDC_CONFIG_1 :: TRIG_XTALK_DISCARD_ENABLE [25:25] */
#define LDC_LDC_CONFIG_1_TRIG_XTALK_DISCARD_ENABLE_MASK            0x02000000
#define LDC_LDC_CONFIG_1_TRIG_XTALK_DISCARD_ENABLE_ALIGN           0
#define LDC_LDC_CONFIG_1_TRIG_XTALK_DISCARD_ENABLE_BITS            1
#define LDC_LDC_CONFIG_1_TRIG_XTALK_DISCARD_ENABLE_SHIFT           25
#define LDC_LDC_CONFIG_1_TRIG_XTALK_DISCARD_ENABLE_DEFAULT         0

/* LDC :: LDC_CONFIG_1 :: PID_ENABLE [24:24] */
#define LDC_LDC_CONFIG_1_PID_ENABLE_MASK                           0x01000000
#define LDC_LDC_CONFIG_1_PID_ENABLE_ALIGN                          0
#define LDC_LDC_CONFIG_1_PID_ENABLE_BITS                           1
#define LDC_LDC_CONFIG_1_PID_ENABLE_SHIFT                          24
#define LDC_LDC_CONFIG_1_PID_ENABLE_DEFAULT                        0

/* LDC :: LDC_CONFIG_1 :: BE_OFF_DELAY_EDGE_MODE [23:23] */
#define LDC_LDC_CONFIG_1_BE_OFF_DELAY_EDGE_MODE_MASK               0x00800000
#define LDC_LDC_CONFIG_1_BE_OFF_DELAY_EDGE_MODE_ALIGN              0
#define LDC_LDC_CONFIG_1_BE_OFF_DELAY_EDGE_MODE_BITS               1
#define LDC_LDC_CONFIG_1_BE_OFF_DELAY_EDGE_MODE_SHIFT              23
#define LDC_LDC_CONFIG_1_BE_OFF_DELAY_EDGE_MODE_DEFAULT            1

/* LDC :: LDC_CONFIG_1 :: LDC_DEBUG_SEL1 [22:18] */
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL1_MASK                       0x007c0000
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL1_ALIGN                      0
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL1_BITS                       5
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL1_SHIFT                      18
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL1_DEFAULT                    0

/* LDC :: LDC_CONFIG_1 :: LDC_DEBUG_SEL0 [17:13] */
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL0_MASK                       0x0003e000
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL0_ALIGN                      0
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL0_BITS                       5
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL0_SHIFT                      13
#define LDC_LDC_CONFIG_1_LDC_DEBUG_SEL0_DEFAULT                    0

/* LDC :: LDC_CONFIG_1 :: BE_RESETS_LD_FORCE [12:12] */
#define LDC_LDC_CONFIG_1_BE_RESETS_LD_FORCE_MASK                   0x00001000
#define LDC_LDC_CONFIG_1_BE_RESETS_LD_FORCE_ALIGN                  0
#define LDC_LDC_CONFIG_1_BE_RESETS_LD_FORCE_BITS                   1
#define LDC_LDC_CONFIG_1_BE_RESETS_LD_FORCE_SHIFT                  12
#define LDC_LDC_CONFIG_1_BE_RESETS_LD_FORCE_DEFAULT                0

/* LDC :: LDC_CONFIG_1 :: LDC_EWAKE_GATES_BE [11:11] */
#define LDC_LDC_CONFIG_1_LDC_EWAKE_GATES_BE_MASK                   0x00000800
#define LDC_LDC_CONFIG_1_LDC_EWAKE_GATES_BE_ALIGN                  0
#define LDC_LDC_CONFIG_1_LDC_EWAKE_GATES_BE_BITS                   1
#define LDC_LDC_CONFIG_1_LDC_EWAKE_GATES_BE_SHIFT                  11
#define LDC_LDC_CONFIG_1_LDC_EWAKE_GATES_BE_DEFAULT                0

/* LDC :: LDC_CONFIG_1 :: LDC_BINARY_SEARCH_LIMIT [10:00] */
#define LDC_LDC_CONFIG_1_LDC_BINARY_SEARCH_LIMIT_MASK              0x000007ff
#define LDC_LDC_CONFIG_1_LDC_BINARY_SEARCH_LIMIT_ALIGN             0
#define LDC_LDC_CONFIG_1_LDC_BINARY_SEARCH_LIMIT_BITS              11
#define LDC_LDC_CONFIG_1_LDC_BINARY_SEARCH_LIMIT_SHIFT             0
#define LDC_LDC_CONFIG_1_LDC_BINARY_SEARCH_LIMIT_DEFAULT           1

/***************************************************************************
 *LDC_STAT_AVG_BIAS - LDC ACL average bias IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_AVG_BIAS :: reserved0 [31:12] */
#define LDC_LDC_STAT_AVG_BIAS_reserved0_MASK                       0xfffff000
#define LDC_LDC_STAT_AVG_BIAS_reserved0_ALIGN                      0
#define LDC_LDC_STAT_AVG_BIAS_reserved0_BITS                       20
#define LDC_LDC_STAT_AVG_BIAS_reserved0_SHIFT                      12

/* LDC :: LDC_STAT_AVG_BIAS :: LDC_STAT_AVG_BIAS [11:00] */
#define LDC_LDC_STAT_AVG_BIAS_LDC_STAT_AVG_BIAS_MASK               0x00000fff
#define LDC_LDC_STAT_AVG_BIAS_LDC_STAT_AVG_BIAS_ALIGN              0
#define LDC_LDC_STAT_AVG_BIAS_LDC_STAT_AVG_BIAS_BITS               12
#define LDC_LDC_STAT_AVG_BIAS_LDC_STAT_AVG_BIAS_SHIFT              0

/***************************************************************************
 *LDC_STAT_MAX_BIAS - LDC ACL maximum bias IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_MAX_BIAS :: reserved0 [31:12] */
#define LDC_LDC_STAT_MAX_BIAS_reserved0_MASK                       0xfffff000
#define LDC_LDC_STAT_MAX_BIAS_reserved0_ALIGN                      0
#define LDC_LDC_STAT_MAX_BIAS_reserved0_BITS                       20
#define LDC_LDC_STAT_MAX_BIAS_reserved0_SHIFT                      12

/* LDC :: LDC_STAT_MAX_BIAS :: LDC_STAT_MAX_BIAS [11:00] */
#define LDC_LDC_STAT_MAX_BIAS_LDC_STAT_MAX_BIAS_MASK               0x00000fff
#define LDC_LDC_STAT_MAX_BIAS_LDC_STAT_MAX_BIAS_ALIGN              0
#define LDC_LDC_STAT_MAX_BIAS_LDC_STAT_MAX_BIAS_BITS               12
#define LDC_LDC_STAT_MAX_BIAS_LDC_STAT_MAX_BIAS_SHIFT              0

/***************************************************************************
 *LDC_STAT_MIN_BIAS - LDC ACL minimum bias IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_MIN_BIAS :: reserved0 [31:12] */
#define LDC_LDC_STAT_MIN_BIAS_reserved0_MASK                       0xfffff000
#define LDC_LDC_STAT_MIN_BIAS_reserved0_ALIGN                      0
#define LDC_LDC_STAT_MIN_BIAS_reserved0_BITS                       20
#define LDC_LDC_STAT_MIN_BIAS_reserved0_SHIFT                      12

/* LDC :: LDC_STAT_MIN_BIAS :: LDC_STAT_MIN_BIAS [11:00] */
#define LDC_LDC_STAT_MIN_BIAS_LDC_STAT_MIN_BIAS_MASK               0x00000fff
#define LDC_LDC_STAT_MIN_BIAS_LDC_STAT_MIN_BIAS_ALIGN              0
#define LDC_LDC_STAT_MIN_BIAS_LDC_STAT_MIN_BIAS_BITS               12
#define LDC_LDC_STAT_MIN_BIAS_LDC_STAT_MIN_BIAS_SHIFT              0

/***************************************************************************
 *LDC_STAT_AVG_MOD - LDC ACL average modulation IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_AVG_MOD :: reserved0 [31:11] */
#define LDC_LDC_STAT_AVG_MOD_reserved0_MASK                        0xfffff800
#define LDC_LDC_STAT_AVG_MOD_reserved0_ALIGN                       0
#define LDC_LDC_STAT_AVG_MOD_reserved0_BITS                        21
#define LDC_LDC_STAT_AVG_MOD_reserved0_SHIFT                       11

/* LDC :: LDC_STAT_AVG_MOD :: LDC_STAT_AVG_MOD [10:00] */
#define LDC_LDC_STAT_AVG_MOD_LDC_STAT_AVG_MOD_MASK                 0x000007ff
#define LDC_LDC_STAT_AVG_MOD_LDC_STAT_AVG_MOD_ALIGN                0
#define LDC_LDC_STAT_AVG_MOD_LDC_STAT_AVG_MOD_BITS                 11
#define LDC_LDC_STAT_AVG_MOD_LDC_STAT_AVG_MOD_SHIFT                0

/***************************************************************************
 *LDC_STAT_MAX_MOD - LDC ACL maximum modulation IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_MAX_MOD :: reserved0 [31:11] */
#define LDC_LDC_STAT_MAX_MOD_reserved0_MASK                        0xfffff800
#define LDC_LDC_STAT_MAX_MOD_reserved0_ALIGN                       0
#define LDC_LDC_STAT_MAX_MOD_reserved0_BITS                        21
#define LDC_LDC_STAT_MAX_MOD_reserved0_SHIFT                       11

/* LDC :: LDC_STAT_MAX_MOD :: LDC_STAT_MAX_MOD [10:00] */
#define LDC_LDC_STAT_MAX_MOD_LDC_STAT_MAX_MOD_MASK                 0x000007ff
#define LDC_LDC_STAT_MAX_MOD_LDC_STAT_MAX_MOD_ALIGN                0
#define LDC_LDC_STAT_MAX_MOD_LDC_STAT_MAX_MOD_BITS                 11
#define LDC_LDC_STAT_MAX_MOD_LDC_STAT_MAX_MOD_SHIFT                0

/***************************************************************************
 *LDC_STAT_MIN_MOD - LDC ACL minimum modulation IDAC value
 ***************************************************************************/
/* LDC :: LDC_STAT_MIN_MOD :: reserved0 [31:11] */
#define LDC_LDC_STAT_MIN_MOD_reserved0_MASK                        0xfffff800
#define LDC_LDC_STAT_MIN_MOD_reserved0_ALIGN                       0
#define LDC_LDC_STAT_MIN_MOD_reserved0_BITS                        21
#define LDC_LDC_STAT_MIN_MOD_reserved0_SHIFT                       11

/* LDC :: LDC_STAT_MIN_MOD :: LDC_STAT_MIN_MOD [10:00] */
#define LDC_LDC_STAT_MIN_MOD_LDC_STAT_MIN_MOD_MASK                 0x000007ff
#define LDC_LDC_STAT_MIN_MOD_LDC_STAT_MIN_MOD_ALIGN                0
#define LDC_LDC_STAT_MIN_MOD_LDC_STAT_MIN_MOD_BITS                 11
#define LDC_LDC_STAT_MIN_MOD_LDC_STAT_MIN_MOD_SHIFT                0

/***************************************************************************
 *LDC_ACL_METRICS_CONFIG - LDC ACL metrics configuration
 ***************************************************************************/
/* LDC :: LDC_ACL_METRICS_CONFIG :: reserved0 [31:18] */
#define LDC_LDC_ACL_METRICS_CONFIG_reserved0_MASK                  0xfffc0000
#define LDC_LDC_ACL_METRICS_CONFIG_reserved0_ALIGN                 0
#define LDC_LDC_ACL_METRICS_CONFIG_reserved0_BITS                  14
#define LDC_LDC_ACL_METRICS_CONFIG_reserved0_SHIFT                 18

/* LDC :: LDC_ACL_METRICS_CONFIG :: ldc_acl_lock_cnt [17:02] */
#define LDC_LDC_ACL_METRICS_CONFIG_ldc_acl_lock_cnt_MASK           0x0003fffc
#define LDC_LDC_ACL_METRICS_CONFIG_ldc_acl_lock_cnt_ALIGN          0
#define LDC_LDC_ACL_METRICS_CONFIG_ldc_acl_lock_cnt_BITS           16
#define LDC_LDC_ACL_METRICS_CONFIG_ldc_acl_lock_cnt_SHIFT          2
#define LDC_LDC_ACL_METRICS_CONFIG_ldc_acl_lock_cnt_DEFAULT        100

/* LDC :: LDC_ACL_METRICS_CONFIG :: LDC_ACL_PAUSE [01:01] */
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_PAUSE_MASK              0x00000002
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_PAUSE_ALIGN             0
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_PAUSE_BITS              1
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_PAUSE_SHIFT             1
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_PAUSE_DEFAULT           0

/* LDC :: LDC_ACL_METRICS_CONFIG :: LDC_ACL_ENABLE_METRICS [00:00] */
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_ENABLE_METRICS_MASK     0x00000001
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_ENABLE_METRICS_ALIGN    0
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_ENABLE_METRICS_BITS     1
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_ENABLE_METRICS_SHIFT    0
#define LDC_LDC_ACL_METRICS_CONFIG_LDC_ACL_ENABLE_METRICS_DEFAULT  0

/***************************************************************************
 *LDC_ACL_BIAS_INT_GAIN_STOP - LDC ACL maximum bias IDAC integral gain limit value
 ***************************************************************************/
/* LDC :: LDC_ACL_BIAS_INT_GAIN_STOP :: reserved0 [31:23] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_reserved0_MASK              0xff800000
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_reserved0_ALIGN             0
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_reserved0_BITS              9
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_reserved0_SHIFT             23

/* LDC :: LDC_ACL_BIAS_INT_GAIN_STOP :: LDC_ACL_BIAS_INT_GAIN_STOP [22:00] */
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_LDC_ACL_BIAS_INT_GAIN_STOP_MASK 0x007fffff
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_LDC_ACL_BIAS_INT_GAIN_STOP_ALIGN 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_LDC_ACL_BIAS_INT_GAIN_STOP_BITS 23
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_LDC_ACL_BIAS_INT_GAIN_STOP_SHIFT 0
#define LDC_LDC_ACL_BIAS_INT_GAIN_STOP_LDC_ACL_BIAS_INT_GAIN_STOP_DEFAULT 0

/***************************************************************************
 *LDC_ACL_MOD_INT_GAIN_STOP - LDC ACL maximum modulation IDAC integral gain limit value
 ***************************************************************************/
/* LDC :: LDC_ACL_MOD_INT_GAIN_STOP :: reserved0 [31:23] */
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_reserved0_MASK               0xff800000
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_reserved0_ALIGN              0
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_reserved0_BITS               9
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_reserved0_SHIFT              23

/* LDC :: LDC_ACL_MOD_INT_GAIN_STOP :: LDC_ACL_MOD_INT_GAIN_STOP [22:00] */
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_LDC_ACL_MOD_INT_GAIN_STOP_MASK 0x007fffff
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_LDC_ACL_MOD_INT_GAIN_STOP_ALIGN 0
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_LDC_ACL_MOD_INT_GAIN_STOP_BITS 23
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_LDC_ACL_MOD_INT_GAIN_STOP_SHIFT 0
#define LDC_LDC_ACL_MOD_INT_GAIN_STOP_LDC_ACL_MOD_INT_GAIN_STOP_DEFAULT 0

/***************************************************************************
 *LDC_SLS_STROBE_SEL - LDC Synchronized LD strobe selects
 ***************************************************************************/
/* LDC :: LDC_SLS_STROBE_SEL :: reserved0 [31:09] */
#define LDC_LDC_SLS_STROBE_SEL_reserved0_MASK                      0xfffffe00
#define LDC_LDC_SLS_STROBE_SEL_reserved0_ALIGN                     0
#define LDC_LDC_SLS_STROBE_SEL_reserved0_BITS                      23
#define LDC_LDC_SLS_STROBE_SEL_reserved0_SHIFT                     9

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT [08:08] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT_MASK 0x00000100
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT_SHIFT 8
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_SETCOMMIT_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1 [07:07] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1_MASK 0x00000080
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1_SHIFT 7
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_1_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0 [06:06] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0_MASK 0x00000040
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0_SHIFT 6
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_TRIG_RSTMATCH_0_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1 [05:05] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1_MASK 0x00000020
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1_SHIFT 5
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_1_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0 [04:04] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0_MASK 0x00000010
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0_SHIFT 4
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_CMPSTROBEMAN_0_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_MPD_STROBEREF_1 [03:03] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_1_MASK 0x00000008
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_1_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_1_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_1_SHIFT 3
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_1_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_MPD_STROBEREF_0 [02:02] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_0_MASK 0x00000004
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_0_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_0_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_0_SHIFT 2
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_MPD_STROBEREF_0_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_IBIASSTROBE [01:01] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IBIASSTROBE_MASK 0x00000002
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IBIASSTROBE_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IBIASSTROBE_BITS 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IBIASSTROBE_SHIFT 1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IBIASSTROBE_DEFAULT 0

/* LDC :: LDC_SLS_STROBE_SEL :: LDC_SLS_STROBE_SEL_IMODSTROBE [00:00] */
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IMODSTROBE_MASK  0x00000001
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IMODSTROBE_ALIGN 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IMODSTROBE_BITS  1
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IMODSTROBE_SHIFT 0
#define LDC_LDC_SLS_STROBE_SEL_LDC_SLS_STROBE_SEL_IMODSTROBE_DEFAULT 0

/***************************************************************************
 *LDC_PID_0 - LDC Progressive IDAC Delays register 0
 ***************************************************************************/
/* LDC :: LDC_PID_0 :: LDC_PID_BSD_32 [31:24] */
#define LDC_LDC_PID_0_LDC_PID_BSD_32_MASK                          0xff000000
#define LDC_LDC_PID_0_LDC_PID_BSD_32_ALIGN                         0
#define LDC_LDC_PID_0_LDC_PID_BSD_32_BITS                          8
#define LDC_LDC_PID_0_LDC_PID_BSD_32_SHIFT                         24
#define LDC_LDC_PID_0_LDC_PID_BSD_32_DEFAULT                       6

/* LDC :: LDC_PID_0 :: LDC_PID_BSD_16 [23:16] */
#define LDC_LDC_PID_0_LDC_PID_BSD_16_MASK                          0x00ff0000
#define LDC_LDC_PID_0_LDC_PID_BSD_16_ALIGN                         0
#define LDC_LDC_PID_0_LDC_PID_BSD_16_BITS                          8
#define LDC_LDC_PID_0_LDC_PID_BSD_16_SHIFT                         16
#define LDC_LDC_PID_0_LDC_PID_BSD_16_DEFAULT                       6

/* LDC :: LDC_PID_0 :: LDC_PID_BSD_8 [15:08] */
#define LDC_LDC_PID_0_LDC_PID_BSD_8_MASK                           0x0000ff00
#define LDC_LDC_PID_0_LDC_PID_BSD_8_ALIGN                          0
#define LDC_LDC_PID_0_LDC_PID_BSD_8_BITS                           8
#define LDC_LDC_PID_0_LDC_PID_BSD_8_SHIFT                          8
#define LDC_LDC_PID_0_LDC_PID_BSD_8_DEFAULT                        6

/* LDC :: LDC_PID_0 :: LDC_PID_BSD_4210 [07:00] */
#define LDC_LDC_PID_0_LDC_PID_BSD_4210_MASK                        0x000000ff
#define LDC_LDC_PID_0_LDC_PID_BSD_4210_ALIGN                       0
#define LDC_LDC_PID_0_LDC_PID_BSD_4210_BITS                        8
#define LDC_LDC_PID_0_LDC_PID_BSD_4210_SHIFT                       0
#define LDC_LDC_PID_0_LDC_PID_BSD_4210_DEFAULT                     6

/***************************************************************************
 *LDC_PID_1 - LDC Progressive IDAC Delays register 1
 ***************************************************************************/
/* LDC :: LDC_PID_1 :: LDC_PID_BSD_512 [31:24] */
#define LDC_LDC_PID_1_LDC_PID_BSD_512_MASK                         0xff000000
#define LDC_LDC_PID_1_LDC_PID_BSD_512_ALIGN                        0
#define LDC_LDC_PID_1_LDC_PID_BSD_512_BITS                         8
#define LDC_LDC_PID_1_LDC_PID_BSD_512_SHIFT                        24
#define LDC_LDC_PID_1_LDC_PID_BSD_512_DEFAULT                      6

/* LDC :: LDC_PID_1 :: LDC_PID_BSD_256 [23:16] */
#define LDC_LDC_PID_1_LDC_PID_BSD_256_MASK                         0x00ff0000
#define LDC_LDC_PID_1_LDC_PID_BSD_256_ALIGN                        0
#define LDC_LDC_PID_1_LDC_PID_BSD_256_BITS                         8
#define LDC_LDC_PID_1_LDC_PID_BSD_256_SHIFT                        16
#define LDC_LDC_PID_1_LDC_PID_BSD_256_DEFAULT                      6

/* LDC :: LDC_PID_1 :: LDC_PID_BSD_128 [15:08] */
#define LDC_LDC_PID_1_LDC_PID_BSD_128_MASK                         0x0000ff00
#define LDC_LDC_PID_1_LDC_PID_BSD_128_ALIGN                        0
#define LDC_LDC_PID_1_LDC_PID_BSD_128_BITS                         8
#define LDC_LDC_PID_1_LDC_PID_BSD_128_SHIFT                        8
#define LDC_LDC_PID_1_LDC_PID_BSD_128_DEFAULT                      6

/* LDC :: LDC_PID_1 :: LDC_PID_BSD_64 [07:00] */
#define LDC_LDC_PID_1_LDC_PID_BSD_64_MASK                          0x000000ff
#define LDC_LDC_PID_1_LDC_PID_BSD_64_ALIGN                         0
#define LDC_LDC_PID_1_LDC_PID_BSD_64_BITS                          8
#define LDC_LDC_PID_1_LDC_PID_BSD_64_SHIFT                         0
#define LDC_LDC_PID_1_LDC_PID_BSD_64_DEFAULT                       6

/***************************************************************************
 *LDC_AEC_CONFIG - LDC Active Ethernet Calibration algorithm configuration
 ***************************************************************************/
/* LDC :: LDC_AEC_CONFIG :: LDC_AEC_TIAINSWITCH_TIME_ON [31:21] */
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_ON_MASK        0xffe00000
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_ON_ALIGN       0
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_ON_BITS        11
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_ON_SHIFT       21
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_ON_DEFAULT     20

/* LDC :: LDC_AEC_CONFIG :: LDC_AEC_TIAINSWITCH_TIME_OFF [20:10] */
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_OFF_MASK       0x001ffc00
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_OFF_ALIGN      0
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_OFF_BITS       11
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_OFF_SHIFT      10
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_TIME_OFF_DEFAULT    10

/* LDC :: LDC_AEC_CONFIG :: LDC_AEC_TIAINSWITCH_DELAY [09:00] */
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_DELAY_MASK          0x000003ff
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_DELAY_ALIGN         0
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_DELAY_BITS          10
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_DELAY_SHIFT         0
#define LDC_LDC_AEC_CONFIG_LDC_AEC_TIAINSWITCH_DELAY_DEFAULT       6

#endif /* #ifndef LDC_H__ */

/* End of File */
