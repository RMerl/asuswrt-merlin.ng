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
#ifndef APD_ADC_H__
#define APD_ADC_H__

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
#define APD_ADC_APD_POWERUP_CONTROL              0x00000000 /* APD power up controls */
#define APD_ADC_APD_STATUS                       0x00000004 /* APD status outputs */
#define APD_ADC_APD_SUPPLY_CONFIG_1              0x00000008 /* APD power supply configuration register 1 */
#define APD_ADC_APD_SUPPLY_CONFIG_2              0x0000000c /* APD power supply configuration register 2 */
#define APD_ADC_ADC_CONFIG_1                     0x00000010 /* ADC configuration register 1 */
#define APD_ADC_ADC_CONFIG_2                     0x00000014 /* ADC configuration register 2 */
#define APD_ADC_ICH_CONFIG                       0x00000018 /* ADC I-channel configuration */
#define APD_ADC_ICH_STATUS                       0x0000001c /* ADC I-channel calibration status */
#define APD_ADC_QCH_CONFIG                       0x00000020 /* ADC Q-channel configuration */
#define APD_ADC_QCH_STATUS                       0x00000024 /* ADC Q-channel calibration status */
#define APD_ADC_APD_ADC_PARAM                    0x00000028 /* APD power supply ECO register */

/***************************************************************************
 *APD_POWERUP_CONTROL - APD power up controls
 ***************************************************************************/
/* APD_ADC :: APD_POWERUP_CONTROL :: reserved0 [31:04] */
#define APD_ADC_APD_POWERUP_CONTROL_reserved0_MASK                 0xfffffff0
#define APD_ADC_APD_POWERUP_CONTROL_reserved0_ALIGN                0
#define APD_ADC_APD_POWERUP_CONTROL_reserved0_BITS                 28
#define APD_ADC_APD_POWERUP_CONTROL_reserved0_SHIFT                4

/* APD_ADC :: APD_POWERUP_CONTROL :: RCOSC_PWRUP [03:03] */
#define APD_ADC_APD_POWERUP_CONTROL_RCOSC_PWRUP_MASK               0x00000008
#define APD_ADC_APD_POWERUP_CONTROL_RCOSC_PWRUP_ALIGN              0
#define APD_ADC_APD_POWERUP_CONTROL_RCOSC_PWRUP_BITS               1
#define APD_ADC_APD_POWERUP_CONTROL_RCOSC_PWRUP_SHIFT              3
#define APD_ADC_APD_POWERUP_CONTROL_RCOSC_PWRUP_DEFAULT            0

/* APD_ADC :: APD_POWERUP_CONTROL :: DR_PWRUP [02:02] */
#define APD_ADC_APD_POWERUP_CONTROL_DR_PWRUP_MASK                  0x00000004
#define APD_ADC_APD_POWERUP_CONTROL_DR_PWRUP_ALIGN                 0
#define APD_ADC_APD_POWERUP_CONTROL_DR_PWRUP_BITS                  1
#define APD_ADC_APD_POWERUP_CONTROL_DR_PWRUP_SHIFT                 2
#define APD_ADC_APD_POWERUP_CONTROL_DR_PWRUP_DEFAULT               0

/* APD_ADC :: APD_POWERUP_CONTROL :: ADC_PWRUP [01:01] */
#define APD_ADC_APD_POWERUP_CONTROL_ADC_PWRUP_MASK                 0x00000002
#define APD_ADC_APD_POWERUP_CONTROL_ADC_PWRUP_ALIGN                0
#define APD_ADC_APD_POWERUP_CONTROL_ADC_PWRUP_BITS                 1
#define APD_ADC_APD_POWERUP_CONTROL_ADC_PWRUP_SHIFT                1
#define APD_ADC_APD_POWERUP_CONTROL_ADC_PWRUP_DEFAULT              0

/* APD_ADC :: APD_POWERUP_CONTROL :: SWREG_PWRUP [00:00] */
#define APD_ADC_APD_POWERUP_CONTROL_SWREG_PWRUP_MASK               0x00000001
#define APD_ADC_APD_POWERUP_CONTROL_SWREG_PWRUP_ALIGN              0
#define APD_ADC_APD_POWERUP_CONTROL_SWREG_PWRUP_BITS               1
#define APD_ADC_APD_POWERUP_CONTROL_SWREG_PWRUP_SHIFT              0
#define APD_ADC_APD_POWERUP_CONTROL_SWREG_PWRUP_DEFAULT            0

/***************************************************************************
 *APD_STATUS - APD status outputs
 ***************************************************************************/
/* APD_ADC :: APD_STATUS :: reserved0 [31:02] */
#define APD_ADC_APD_STATUS_reserved0_MASK                          0xfffffffc
#define APD_ADC_APD_STATUS_reserved0_ALIGN                         0
#define APD_ADC_APD_STATUS_reserved0_BITS                          30
#define APD_ADC_APD_STATUS_reserved0_SHIFT                         2

/* APD_ADC :: APD_STATUS :: PMU_STABLE [01:01] */
#define APD_ADC_APD_STATUS_PMU_STABLE_MASK                         0x00000002
#define APD_ADC_APD_STATUS_PMU_STABLE_ALIGN                        0
#define APD_ADC_APD_STATUS_PMU_STABLE_BITS                         1
#define APD_ADC_APD_STATUS_PMU_STABLE_SHIFT                        1

/* APD_ADC :: APD_STATUS :: FAULT_OVI [00:00] */
#define APD_ADC_APD_STATUS_FAULT_OVI_MASK                          0x00000001
#define APD_ADC_APD_STATUS_FAULT_OVI_ALIGN                         0
#define APD_ADC_APD_STATUS_FAULT_OVI_BITS                          1
#define APD_ADC_APD_STATUS_FAULT_OVI_SHIFT                         0

/***************************************************************************
 *APD_SUPPLY_CONFIG_1 - APD power supply configuration register 1
 ***************************************************************************/
/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: reserved0 [31:27] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved0_MASK                 0xf8000000
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved0_ALIGN                0
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved0_BITS                 5
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved0_SHIFT                27

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_SSPULSE [26:23] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SSPULSE_MASK               0x07800000
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SSPULSE_ALIGN              0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SSPULSE_BITS               4
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SSPULSE_SHIFT              23
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SSPULSE_DEFAULT            6

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_SS_SKIP [22:20] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SS_SKIP_MASK               0x00700000
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SS_SKIP_ALIGN              0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SS_SKIP_BITS               3
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SS_SKIP_SHIFT              20
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_SS_SKIP_DEFAULT            3

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_DCMAX [19:16] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_DCMAX_MASK                 0x000f0000
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_DCMAX_ALIGN                0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_DCMAX_BITS                 4
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_DCMAX_SHIFT                16
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_DCMAX_DEFAULT              7

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_PULSE_MIN [15:12] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PULSE_MIN_MASK             0x0000f000
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PULSE_MIN_ALIGN            0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PULSE_MIN_BITS             4
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PULSE_MIN_SHIFT            12
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PULSE_MIN_DEFAULT          5

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_FREQ_CTRL [11:10] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_FREQ_CTRL_MASK             0x00000c00
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_FREQ_CTRL_ALIGN            0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_FREQ_CTRL_BITS             2
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_FREQ_CTRL_SHIFT            10
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_FREQ_CTRL_DEFAULT          1

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_PRTVAL [09:09] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTVAL_MASK                0x00000200
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTVAL_ALIGN               0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTVAL_BITS                1
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTVAL_SHIFT               9
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTVAL_DEFAULT             0

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_PRTOVR [08:08] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTOVR_MASK                0x00000100
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTOVR_ALIGN               0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTOVR_BITS                1
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTOVR_SHIFT               8
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_PRTOVR_DEFAULT             0

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: reserved1 [07:05] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved1_MASK                 0x000000e0
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved1_ALIGN                0
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved1_BITS                 3
#define APD_ADC_APD_SUPPLY_CONFIG_1_reserved1_SHIFT                5

/* APD_ADC :: APD_SUPPLY_CONFIG_1 :: CFG_BG_TRIM [04:00] */
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_BG_TRIM_MASK               0x0000001f
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_BG_TRIM_ALIGN              0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_BG_TRIM_BITS               5
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_BG_TRIM_SHIFT              0
#define APD_ADC_APD_SUPPLY_CONFIG_1_CFG_BG_TRIM_DEFAULT            0

/***************************************************************************
 *APD_SUPPLY_CONFIG_2 - APD power supply configuration register 2
 ***************************************************************************/
/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: reserved0 [31:28] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved0_MASK                 0xf0000000
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved0_ALIGN                0
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved0_BITS                 4
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved0_SHIFT                28

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_MODE [27:27] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_MODE_MASK                  0x08000000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_MODE_ALIGN                 0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_MODE_BITS                  1
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_MODE_SHIFT                 27
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_MODE_DEFAULT               0

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_APDOI_CTRL [26:23] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_APDOI_CTRL_MASK            0x07800000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_APDOI_CTRL_ALIGN           0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_APDOI_CTRL_BITS            4
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_APDOI_CTRL_SHIFT           23
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_APDOI_CTRL_DEFAULT         9

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_ILIM_CTRL [22:21] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_ILIM_CTRL_MASK             0x00600000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_ILIM_CTRL_ALIGN            0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_ILIM_CTRL_BITS             2
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_ILIM_CTRL_SHIFT            21
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_ILIM_CTRL_DEFAULT          1

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_DR_CTRL [20:18] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_DR_CTRL_MASK               0x001c0000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_DR_CTRL_ALIGN              0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_DR_CTRL_BITS               3
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_DR_CTRL_SHIFT              18
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_DR_CTRL_DEFAULT            1

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_RAMP_CTRL [17:16] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_RAMP_CTRL_MASK             0x00030000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_RAMP_CTRL_ALIGN            0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_RAMP_CTRL_BITS             2
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_RAMP_CTRL_SHIFT            16
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_RAMP_CTRL_DEFAULT          3

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_GAIN [15:12] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_GAIN_MASK                  0x0000f000
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_GAIN_ALIGN                 0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_GAIN_BITS                  4
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_GAIN_SHIFT                 12
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_GAIN_DEFAULT               7

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: reserved1 [11:10] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved1_MASK                 0x00000c00
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved1_ALIGN                0
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved1_BITS                 2
#define APD_ADC_APD_SUPPLY_CONFIG_2_reserved1_SHIFT                10

/* APD_ADC :: APD_SUPPLY_CONFIG_2 :: CFG_VREF_CTRL [09:00] */
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_VREF_CTRL_MASK             0x000003ff
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_VREF_CTRL_ALIGN            0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_VREF_CTRL_BITS             10
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_VREF_CTRL_SHIFT            0
#define APD_ADC_APD_SUPPLY_CONFIG_2_CFG_VREF_CTRL_DEFAULT          384

/***************************************************************************
 *ADC_CONFIG_1 - ADC configuration register 1
 ***************************************************************************/
/* APD_ADC :: ADC_CONFIG_1 :: reserved0 [31:28] */
#define APD_ADC_ADC_CONFIG_1_reserved0_MASK                        0xf0000000
#define APD_ADC_ADC_CONFIG_1_reserved0_ALIGN                       0
#define APD_ADC_ADC_CONFIG_1_reserved0_BITS                        4
#define APD_ADC_ADC_CONFIG_1_reserved0_SHIFT                       28

/* APD_ADC :: ADC_CONFIG_1 :: CFG_CTL_ADC_BIAS [27:08] */
#define APD_ADC_ADC_CONFIG_1_CFG_CTL_ADC_BIAS_MASK                 0x0fffff00
#define APD_ADC_ADC_CONFIG_1_CFG_CTL_ADC_BIAS_ALIGN                0
#define APD_ADC_ADC_CONFIG_1_CFG_CTL_ADC_BIAS_BITS                 20
#define APD_ADC_ADC_CONFIG_1_CFG_CTL_ADC_BIAS_SHIFT                8
#define APD_ADC_ADC_CONFIG_1_CFG_CTL_ADC_BIAS_DEFAULT              0

/* APD_ADC :: ADC_CONFIG_1 :: reserved1 [07:06] */
#define APD_ADC_ADC_CONFIG_1_reserved1_MASK                        0x000000c0
#define APD_ADC_ADC_CONFIG_1_reserved1_ALIGN                       0
#define APD_ADC_ADC_CONFIG_1_reserved1_BITS                        2
#define APD_ADC_ADC_CONFIG_1_reserved1_SHIFT                       6

/* APD_ADC :: ADC_CONFIG_1 :: CFG_ADCCLK_INV [05:05] */
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_INV_MASK                   0x00000020
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_INV_ALIGN                  0
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_INV_BITS                   1
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_INV_SHIFT                  5
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_INV_DEFAULT                0

/* APD_ADC :: ADC_CONFIG_1 :: CFG_ADCCLK_RESET [04:04] */
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_RESET_MASK                 0x00000010
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_RESET_ALIGN                0
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_RESET_BITS                 1
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_RESET_SHIFT                4
#define APD_ADC_ADC_CONFIG_1_CFG_ADCCLK_RESET_DEFAULT              1

/* APD_ADC :: ADC_CONFIG_1 :: CFG_CNTR_SEL [03:03] */
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_SEL_MASK                     0x00000008
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_SEL_ALIGN                    0
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_SEL_BITS                     1
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_SEL_SHIFT                    3
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_SEL_DEFAULT                  0

/* APD_ADC :: ADC_CONFIG_1 :: CFG_CNTR_ENABLE [02:02] */
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_ENABLE_MASK                  0x00000004
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_ENABLE_ALIGN                 0
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_ENABLE_BITS                  1
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_ENABLE_SHIFT                 2
#define APD_ADC_ADC_CONFIG_1_CFG_CNTR_ENABLE_DEFAULT               0

/* APD_ADC :: ADC_CONFIG_1 :: CFG_ADC_RST_N [01:01] */
#define APD_ADC_ADC_CONFIG_1_CFG_ADC_RST_N_MASK                    0x00000002
#define APD_ADC_ADC_CONFIG_1_CFG_ADC_RST_N_ALIGN                   0
#define APD_ADC_ADC_CONFIG_1_CFG_ADC_RST_N_BITS                    1
#define APD_ADC_ADC_CONFIG_1_CFG_ADC_RST_N_SHIFT                   1
#define APD_ADC_ADC_CONFIG_1_CFG_ADC_RST_N_DEFAULT                 0

/* APD_ADC :: ADC_CONFIG_1 :: reserved2 [00:00] */
#define APD_ADC_ADC_CONFIG_1_reserved2_MASK                        0x00000001
#define APD_ADC_ADC_CONFIG_1_reserved2_ALIGN                       0
#define APD_ADC_ADC_CONFIG_1_reserved2_BITS                        1
#define APD_ADC_ADC_CONFIG_1_reserved2_SHIFT                       0

/***************************************************************************
 *ADC_CONFIG_2 - ADC configuration register 2
 ***************************************************************************/
/* APD_ADC :: ADC_CONFIG_2 :: reserved0 [31:26] */
#define APD_ADC_ADC_CONFIG_2_reserved0_MASK                        0xfc000000
#define APD_ADC_ADC_CONFIG_2_reserved0_ALIGN                       0
#define APD_ADC_ADC_CONFIG_2_reserved0_BITS                        6
#define APD_ADC_ADC_CONFIG_2_reserved0_SHIFT                       26

/* APD_ADC :: ADC_CONFIG_2 :: CFG_ADC_SPARE [25:16] */
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_SPARE_MASK                    0x03ff0000
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_SPARE_ALIGN                   0
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_SPARE_BITS                    10
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_SPARE_SHIFT                   16
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_SPARE_DEFAULT                 0

/* APD_ADC :: ADC_CONFIG_2 :: CFG_ADC_DLY [15:10] */
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_DLY_MASK                      0x0000fc00
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_DLY_ALIGN                     0
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_DLY_BITS                      6
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_DLY_SHIFT                     10
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_DLY_DEFAULT                   0

/* APD_ADC :: ADC_CONFIG_2 :: CFG_ADC_GAIN [09:07] */
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_GAIN_MASK                     0x00000380
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_GAIN_ALIGN                    0
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_GAIN_BITS                     3
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_GAIN_SHIFT                    7
#define APD_ADC_ADC_CONFIG_2_CFG_ADC_GAIN_DEFAULT                  0

/* APD_ADC :: ADC_CONFIG_2 :: CFG_CTL_FLASH_FULLSCALE [06:05] */
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_FLASH_FULLSCALE_MASK          0x00000060
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_FLASH_FULLSCALE_ALIGN         0
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_FLASH_FULLSCALE_BITS          2
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_FLASH_FULLSCALE_SHIFT         5
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_FLASH_FULLSCALE_DEFAULT       0

/* APD_ADC :: ADC_CONFIG_2 :: CFG_CTL_RC [04:02] */
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_RC_MASK                       0x0000001c
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_RC_ALIGN                      0
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_RC_BITS                       3
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_RC_SHIFT                      2
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_RC_DEFAULT                    0

/* APD_ADC :: ADC_CONFIG_2 :: CFG_CTL_VCM [01:00] */
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_VCM_MASK                      0x00000003
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_VCM_ALIGN                     0
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_VCM_BITS                      2
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_VCM_SHIFT                     0
#define APD_ADC_ADC_CONFIG_2_CFG_CTL_VCM_DEFAULT                   0

/***************************************************************************
 *ICH_CONFIG - ADC I-channel configuration
 ***************************************************************************/
/* APD_ADC :: ICH_CONFIG :: CFG_ICH_CAL_DONE [31:31] */
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_DONE_MASK                   0x80000000
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_DONE_ALIGN                  0
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_DONE_BITS                   1
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_DONE_SHIFT                  31
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_DONE_DEFAULT                0

/* APD_ADC :: ICH_CONFIG :: reserved0 [30:30] */
#define APD_ADC_ICH_CONFIG_reserved0_MASK                          0x40000000
#define APD_ADC_ICH_CONFIG_reserved0_ALIGN                         0
#define APD_ADC_ICH_CONFIG_reserved0_BITS                          1
#define APD_ADC_ICH_CONFIG_reserved0_SHIFT                         30

/* APD_ADC :: ICH_CONFIG :: CFG_ICH_CAL_ON [29:29] */
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_ON_MASK                     0x20000000
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_ON_ALIGN                    0
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_ON_BITS                     1
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_ON_SHIFT                    29
#define APD_ADC_ICH_CONFIG_CFG_ICH_CAL_ON_DEFAULT                  0

/* APD_ADC :: ICH_CONFIG :: CFG_ICH_FLASH_RESETCAL [28:28] */
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_RESETCAL_MASK             0x10000000
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_RESETCAL_ALIGN            0
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_RESETCAL_BITS             1
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_RESETCAL_SHIFT            28
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_RESETCAL_DEFAULT          1

/* APD_ADC :: ICH_CONFIG :: CFG_CTL_ICH_FLASH_OFFSET [27:04] */
#define APD_ADC_ICH_CONFIG_CFG_CTL_ICH_FLASH_OFFSET_MASK           0x0ffffff0
#define APD_ADC_ICH_CONFIG_CFG_CTL_ICH_FLASH_OFFSET_ALIGN          0
#define APD_ADC_ICH_CONFIG_CFG_CTL_ICH_FLASH_OFFSET_BITS           24
#define APD_ADC_ICH_CONFIG_CFG_CTL_ICH_FLASH_OFFSET_SHIFT          4
#define APD_ADC_ICH_CONFIG_CFG_CTL_ICH_FLASH_OFFSET_DEFAULT        0

/* APD_ADC :: ICH_CONFIG :: CFG_ICH_FLASH_CALSEL [03:03] */
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_CALSEL_MASK               0x00000008
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_CALSEL_ALIGN              0
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_CALSEL_BITS               1
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_CALSEL_SHIFT              3
#define APD_ADC_ICH_CONFIG_CFG_ICH_FLASH_CALSEL_DEFAULT            0

/* APD_ADC :: ICH_CONFIG :: CFG_ICH_SCRAM_OFF [02:02] */
#define APD_ADC_ICH_CONFIG_CFG_ICH_SCRAM_OFF_MASK                  0x00000004
#define APD_ADC_ICH_CONFIG_CFG_ICH_SCRAM_OFF_ALIGN                 0
#define APD_ADC_ICH_CONFIG_CFG_ICH_SCRAM_OFF_BITS                  1
#define APD_ADC_ICH_CONFIG_CFG_ICH_SCRAM_OFF_SHIFT                 2
#define APD_ADC_ICH_CONFIG_CFG_ICH_SCRAM_OFF_DEFAULT               0

/* APD_ADC :: ICH_CONFIG :: reserved1 [01:00] */
#define APD_ADC_ICH_CONFIG_reserved1_MASK                          0x00000003
#define APD_ADC_ICH_CONFIG_reserved1_ALIGN                         0
#define APD_ADC_ICH_CONFIG_reserved1_BITS                          2
#define APD_ADC_ICH_CONFIG_reserved1_SHIFT                         0

/***************************************************************************
 *ICH_STATUS - ADC I-channel calibration status
 ***************************************************************************/
/* APD_ADC :: ICH_STATUS :: reserved0 [31:24] */
#define APD_ADC_ICH_STATUS_reserved0_MASK                          0xff000000
#define APD_ADC_ICH_STATUS_reserved0_ALIGN                         0
#define APD_ADC_ICH_STATUS_reserved0_BITS                          8
#define APD_ADC_ICH_STATUS_reserved0_SHIFT                         24

/* APD_ADC :: ICH_STATUS :: ICH_FLASH_CALDATA [23:00] */
#define APD_ADC_ICH_STATUS_ICH_FLASH_CALDATA_MASK                  0x00ffffff
#define APD_ADC_ICH_STATUS_ICH_FLASH_CALDATA_ALIGN                 0
#define APD_ADC_ICH_STATUS_ICH_FLASH_CALDATA_BITS                  24
#define APD_ADC_ICH_STATUS_ICH_FLASH_CALDATA_SHIFT                 0

/***************************************************************************
 *QCH_CONFIG - ADC Q-channel configuration
 ***************************************************************************/
/* APD_ADC :: QCH_CONFIG :: CFG_ICH_CAL_DONE [31:31] */
#define APD_ADC_QCH_CONFIG_CFG_ICH_CAL_DONE_MASK                   0x80000000
#define APD_ADC_QCH_CONFIG_CFG_ICH_CAL_DONE_ALIGN                  0
#define APD_ADC_QCH_CONFIG_CFG_ICH_CAL_DONE_BITS                   1
#define APD_ADC_QCH_CONFIG_CFG_ICH_CAL_DONE_SHIFT                  31
#define APD_ADC_QCH_CONFIG_CFG_ICH_CAL_DONE_DEFAULT                0

/* APD_ADC :: QCH_CONFIG :: reserved0 [30:30] */
#define APD_ADC_QCH_CONFIG_reserved0_MASK                          0x40000000
#define APD_ADC_QCH_CONFIG_reserved0_ALIGN                         0
#define APD_ADC_QCH_CONFIG_reserved0_BITS                          1
#define APD_ADC_QCH_CONFIG_reserved0_SHIFT                         30

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_CAL_ON [29:29] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_CAL_ON_MASK                     0x20000000
#define APD_ADC_QCH_CONFIG_CFG_QCH_CAL_ON_ALIGN                    0
#define APD_ADC_QCH_CONFIG_CFG_QCH_CAL_ON_BITS                     1
#define APD_ADC_QCH_CONFIG_CFG_QCH_CAL_ON_SHIFT                    29
#define APD_ADC_QCH_CONFIG_CFG_QCH_CAL_ON_DEFAULT                  0

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_FLASH_RESETCAL [28:28] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_RESETCAL_MASK             0x10000000
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_RESETCAL_ALIGN            0
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_RESETCAL_BITS             1
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_RESETCAL_SHIFT            28
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_RESETCAL_DEFAULT          1

/* APD_ADC :: QCH_CONFIG :: CFG_CTL_QCH_FLASH_OFFSET [27:04] */
#define APD_ADC_QCH_CONFIG_CFG_CTL_QCH_FLASH_OFFSET_MASK           0x0ffffff0
#define APD_ADC_QCH_CONFIG_CFG_CTL_QCH_FLASH_OFFSET_ALIGN          0
#define APD_ADC_QCH_CONFIG_CFG_CTL_QCH_FLASH_OFFSET_BITS           24
#define APD_ADC_QCH_CONFIG_CFG_CTL_QCH_FLASH_OFFSET_SHIFT          4
#define APD_ADC_QCH_CONFIG_CFG_CTL_QCH_FLASH_OFFSET_DEFAULT        0

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_FLASH_CALSEL [03:03] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_CALSEL_MASK               0x00000008
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_CALSEL_ALIGN              0
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_CALSEL_BITS               1
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_CALSEL_SHIFT              3
#define APD_ADC_QCH_CONFIG_CFG_QCH_FLASH_CALSEL_DEFAULT            0

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_SCRAM_OFF [02:02] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_SCRAM_OFF_MASK                  0x00000004
#define APD_ADC_QCH_CONFIG_CFG_QCH_SCRAM_OFF_ALIGN                 0
#define APD_ADC_QCH_CONFIG_CFG_QCH_SCRAM_OFF_BITS                  1
#define APD_ADC_QCH_CONFIG_CFG_QCH_SCRAM_OFF_SHIFT                 2
#define APD_ADC_QCH_CONFIG_CFG_QCH_SCRAM_OFF_DEFAULT               0

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_PWRUP [01:01] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_PWRUP_MASK                      0x00000002
#define APD_ADC_QCH_CONFIG_CFG_QCH_PWRUP_ALIGN                     0
#define APD_ADC_QCH_CONFIG_CFG_QCH_PWRUP_BITS                      1
#define APD_ADC_QCH_CONFIG_CFG_QCH_PWRUP_SHIFT                     1
#define APD_ADC_QCH_CONFIG_CFG_QCH_PWRUP_DEFAULT                   0

/* APD_ADC :: QCH_CONFIG :: CFG_QCH_RESET [00:00] */
#define APD_ADC_QCH_CONFIG_CFG_QCH_RESET_MASK                      0x00000001
#define APD_ADC_QCH_CONFIG_CFG_QCH_RESET_ALIGN                     0
#define APD_ADC_QCH_CONFIG_CFG_QCH_RESET_BITS                      1
#define APD_ADC_QCH_CONFIG_CFG_QCH_RESET_SHIFT                     0
#define APD_ADC_QCH_CONFIG_CFG_QCH_RESET_DEFAULT                   1

/***************************************************************************
 *QCH_STATUS - ADC Q-channel calibration status
 ***************************************************************************/
/* APD_ADC :: QCH_STATUS :: reserved0 [31:24] */
#define APD_ADC_QCH_STATUS_reserved0_MASK                          0xff000000
#define APD_ADC_QCH_STATUS_reserved0_ALIGN                         0
#define APD_ADC_QCH_STATUS_reserved0_BITS                          8
#define APD_ADC_QCH_STATUS_reserved0_SHIFT                         24

/* APD_ADC :: QCH_STATUS :: QCH_FLASH_CALDATA [23:00] */
#define APD_ADC_QCH_STATUS_QCH_FLASH_CALDATA_MASK                  0x00ffffff
#define APD_ADC_QCH_STATUS_QCH_FLASH_CALDATA_ALIGN                 0
#define APD_ADC_QCH_STATUS_QCH_FLASH_CALDATA_BITS                  24
#define APD_ADC_QCH_STATUS_QCH_FLASH_CALDATA_SHIFT                 0

/***************************************************************************
 *APD_ADC_PARAM - APD power supply ECO register
 ***************************************************************************/
/* APD_ADC :: APD_ADC_PARAM :: reserved_for_eco0 [31:00] */
#define APD_ADC_APD_ADC_PARAM_reserved_for_eco0_MASK               0xffffffff
#define APD_ADC_APD_ADC_PARAM_reserved_for_eco0_ALIGN              0
#define APD_ADC_APD_ADC_PARAM_reserved_for_eco0_BITS               32
#define APD_ADC_APD_ADC_PARAM_reserved_for_eco0_SHIFT              0
#define APD_ADC_APD_ADC_PARAM_reserved_for_eco0_DEFAULT            0

#endif /* #ifndef APD_ADC_H__ */

/* End of File */
