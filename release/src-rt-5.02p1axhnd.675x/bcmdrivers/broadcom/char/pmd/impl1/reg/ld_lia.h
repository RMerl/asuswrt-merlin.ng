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
#ifndef LD_LIA_H__
#define LD_LIA_H__

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
#define LD_LIA_LD_TEST_MUX_CTL                   0x00000000 /* APD analog test mux controls */
#define LD_LIA_LD_CONTROL_1                      0x00000004 /* LD control register 1 */
#define LD_LIA_LD_CONTROL_2                      0x00000008 /* LD control register 2 */
#define LD_LIA_LD_CONTROL_2a                     0x0000000c /* LD control register 2a */
#define LD_LIA_LD_CONTROL_3                      0x00000010 /* LD control register 3 */
#define LD_LIA_LD_CONTROL_3a                     0x00000014 /* LD control register 3a */
#define LD_LIA_LD_COMMON_TRIG_CONFIG             0x00000018 /* LD common trigger configuration */
#define LD_LIA_LD_TRIG_SEED                      0x0000001c /* LD trigger seed value */
#define LD_LIA_LD_TRIG0_CONFIG                   0x00000020 /* LD trigger 0 configuration */
#define LD_LIA_LD_TRIG0_MASK                     0x00000024 /* LD trigger 0 pattern mask */
#define LD_LIA_LD_TRIG0_PATT                     0x00000028 /* LD trigger 0 pattern */
#define LD_LIA_LD_TRIG0_STATUS                   0x0000002c /* LD trigger 0 event status */
#define LD_LIA_LD_TRIG1_CONFIG                   0x00000030 /* LD trigger 1 configuration */
#define LD_LIA_LD_TRIG1_MASK                     0x00000034 /* LD trigger 1 pattern mask */
#define LD_LIA_LD_TRIG1_PATT                     0x00000038 /* LD trigger 1 pattern */
#define LD_LIA_LD_TRIG1_STATUS                   0x0000003c /* LD trigger 1 event status */
#define LD_LIA_LD_PATT_GEN_MASK                  0x00000040 /* LD pattern generator pattern mask */
#define LD_LIA_LD_PATT_GEN_PATT                  0x00000044 /* LD pattern generator pattern */
#define LD_LIA_LD_MPD_CONTROL_1                  0x00000048 /* LD monitor photo-diode control register 1 */
#define LD_LIA_LD_MPD_CONTROL_2                  0x0000004c /* LD monitor photo-diode control register 2 */
#define LD_LIA_LD_MPD_CONTROL_3                  0x00000050 /* LD monitor photo-diode control register 3 */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL          0x00000054 /* LD monitor photo-diode comparator DAC-0 control */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL          0x00000058 /* LD monitor photo-diode comparator DAC-1 control */
#define LD_LIA_LD_PLL_CONTROL_1                  0x0000005c /* LD LC PLL control register 1 */
#define LD_LIA_LD_PLL_CONTROL_2                  0x00000060 /* LD LC PLL control register 2 */
#define LD_LIA_LD_PLL_CONTROL_3                  0x00000064 /* LD LC PLL control register 3 */
#define LD_LIA_LD_PLL_STATUS                     0x00000068 /* LD LC PLL status */
#define LD_LIA_LD_PLL_DIVIDER_0                  0x0000006c /* LD LC PLL divider 0 configuration */
#define LD_LIA_LD_PLL_DIVIDER_1                  0x00000070 /* LD LC PLL divider 1 configuration */
#define LD_LIA_LD_PLL_DIVIDER_2                  0x00000074 /* LD LC PLL divider 2 configuration */
#define LD_LIA_LD_PLL_DIVIDER_3                  0x00000078 /* LD LC PLL divider 3 configuration */
#define LD_LIA_LD_PLL_DIVIDER_4                  0x0000007c /* LD LC PLL divider 4 configuration */
#define LD_LIA_LD_PLL_DIVIDER_5                  0x00000080 /* LD LC PLL divider 5 configuration */
#define LD_LIA_LD_CDR_CONFIG_1                   0x00000084 /* LD CDR configuration register 1 */
#define LD_LIA_LD_CDR_CONFIG_2                   0x00000088 /* LD CDR configuration register 2 */
#define LD_LIA_LD_CDR_RX_CTRL_LO                 0x0000008c /* LD CDR lower 32-bits of control register */
#define LD_LIA_LD_CDR_RX_CTRL_HI                 0x00000090 /* LD CDR upper 3-bits of control register */
#define LD_LIA_LD_RX0_DFS_LO                     0x00000094 /* LD CDR lower 32-bits of reciever control bits default register */
#define LD_LIA_LD_RX0_DFS_HI                     0x00000098 /* LD CDR upper 16-bits of reciever control bits default register */
#define LD_LIA_LD_CDR_RX_STATUS                  0x0000009c /* LD CDR receiver status */
#define LD_LIA_LD_CDR_STATUS_1                   0x000000a0 /* LD CDR Status 1 */
#define LD_LIA_LD_CDR_STATUS_2                   0x000000a4 /* LD CDR Status 2 */
#define LD_LIA_LD_CDR_RX_CONFIG                  0x000000a8 /* LD CDR receiver configuration */
#define LD_LIA_LD_LIA_CTRL_0                     0x000000ac /* LD LIA control register 0 */
#define LD_LIA_LD_LIA_CTRL_1                     0x000000b0 /* LD LIA control register 1 */
#define LD_LIA_LD_LIA_CTRL_2                     0x000000b4 /* LD LIA control register 2 */
#define LD_LIA_LD_LIA_CTRL_3                     0x000000b8 /* LD LIA control register 3 */
#define LD_LIA_LD_LIA_CTRL_4                     0x000000bc /* LD LIA control register 4 */
#define LD_LIA_LD_LIA_CTRL_5                     0x000000c0 /* LD LIA control register 5 */
#define LD_LIA_LD_LIA_CTRL_6                     0x000000c4 /* LD LIA control register 6 */
#define LD_LIA_LD_LIA_RSSI_PEAKPOS               0x000000c8 /* LD LIA Receive Signal Strength Indicator positive peak measurement */
#define LD_LIA_LD_LIA_RSSI_PEAKNEG               0x000000cc /* LD LIA Receive Signal Strength Indicator negative peak measurement */
#define LD_LIA_LD_XO_CONTROL                     0x000000d0 /* LD LIA Crystal oscillator control */
#define LD_LIA_LD_SIG_RESERVED                   0x000000d4 /* LD reserved register */
#define LD_LIA_LD_TRIG_RESERVED                  0x000000d8 /* LD trigger reserved register */
#define LD_LIA_LD_MPD_CONTROL_6                  0x000000dc /* LD monitor photo-diode control register 6 */
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5           0x000000e0 /* LD PLL dynamic mode controls */
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS             0x000000e4 /* LD PLL dynamic mode MDIV values */
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME        0x000000e8 /* LD PLL dynamic mode transtion times */
#define LD_LIA_LD_LIA_PARAM                      0x000000ec /* LD_LIA reserved ECO register */
#define LD_LIA_LD_MPD_STATUS                     0x000000f0 /* LD_LIA MPD status register */
#define LD_LIA_LD_TRIG_FORCEDELAY                0x000000f4 /* LD_LIA trigger force delay register */
#define LD_LIA_LD_MPD_CONTROL_4                  0x000000f8 /* LD monitor photo-diode control register 4 */
#define LD_LIA_LD_MPD_CONTROL_5                  0x000000fc /* LD monitor photo-diode control register 5 */

/***************************************************************************
 *LD_TEST_MUX_CTL - APD analog test mux controls
 ***************************************************************************/
/* LD_LIA :: LD_TEST_MUX_CTL :: CFG_LD_ANTESTEN [31:00] */
#define LD_LIA_LD_TEST_MUX_CTL_CFG_LD_ANTESTEN_MASK                0xffffffff
#define LD_LIA_LD_TEST_MUX_CTL_CFG_LD_ANTESTEN_ALIGN               0
#define LD_LIA_LD_TEST_MUX_CTL_CFG_LD_ANTESTEN_BITS                32
#define LD_LIA_LD_TEST_MUX_CTL_CFG_LD_ANTESTEN_SHIFT               0
#define LD_LIA_LD_TEST_MUX_CTL_CFG_LD_ANTESTEN_DEFAULT             0

/***************************************************************************
 *LD_CONTROL_1 - LD control register 1
 ***************************************************************************/
/* LD_LIA :: LD_CONTROL_1 :: CFG_LD_SIG_BIASADJ [31:16] */
#define LD_LIA_LD_CONTROL_1_CFG_LD_SIG_BIASADJ_MASK                0xffff0000
#define LD_LIA_LD_CONTROL_1_CFG_LD_SIG_BIASADJ_ALIGN               0
#define LD_LIA_LD_CONTROL_1_CFG_LD_SIG_BIASADJ_BITS                16
#define LD_LIA_LD_CONTROL_1_CFG_LD_SIG_BIASADJ_SHIFT               16
#define LD_LIA_LD_CONTROL_1_CFG_LD_SIG_BIASADJ_DEFAULT             16930

/* LD_LIA :: LD_CONTROL_1 :: reserved0 [15:12] */
#define LD_LIA_LD_CONTROL_1_reserved0_MASK                         0x0000f000
#define LD_LIA_LD_CONTROL_1_reserved0_ALIGN                        0
#define LD_LIA_LD_CONTROL_1_reserved0_BITS                         4
#define LD_LIA_LD_CONTROL_1_reserved0_SHIFT                        12

/* LD_LIA :: LD_CONTROL_1 :: CFG_LD_PWRDN_BG [11:11] */
#define LD_LIA_LD_CONTROL_1_CFG_LD_PWRDN_BG_MASK                   0x00000800
#define LD_LIA_LD_CONTROL_1_CFG_LD_PWRDN_BG_ALIGN                  0
#define LD_LIA_LD_CONTROL_1_CFG_LD_PWRDN_BG_BITS                   1
#define LD_LIA_LD_CONTROL_1_CFG_LD_PWRDN_BG_SHIFT                  11
#define LD_LIA_LD_CONTROL_1_CFG_LD_PWRDN_BG_DEFAULT                1

/* LD_LIA :: LD_CONTROL_1 :: CFG_LD_RESCAL [10:08] */
#define LD_LIA_LD_CONTROL_1_CFG_LD_RESCAL_MASK                     0x00000700
#define LD_LIA_LD_CONTROL_1_CFG_LD_RESCAL_ALIGN                    0
#define LD_LIA_LD_CONTROL_1_CFG_LD_RESCAL_BITS                     3
#define LD_LIA_LD_CONTROL_1_CFG_LD_RESCAL_SHIFT                    8
#define LD_LIA_LD_CONTROL_1_CFG_LD_RESCAL_DEFAULT                  3

/* LD_LIA :: LD_CONTROL_1 :: CFG_LD_PTATADJ [07:04] */
#define LD_LIA_LD_CONTROL_1_CFG_LD_PTATADJ_MASK                    0x000000f0
#define LD_LIA_LD_CONTROL_1_CFG_LD_PTATADJ_ALIGN                   0
#define LD_LIA_LD_CONTROL_1_CFG_LD_PTATADJ_BITS                    4
#define LD_LIA_LD_CONTROL_1_CFG_LD_PTATADJ_SHIFT                   4
#define LD_LIA_LD_CONTROL_1_CFG_LD_PTATADJ_DEFAULT                 7

/* LD_LIA :: LD_CONTROL_1 :: CFG_LD_CTATADJ [03:00] */
#define LD_LIA_LD_CONTROL_1_CFG_LD_CTATADJ_MASK                    0x0000000f
#define LD_LIA_LD_CONTROL_1_CFG_LD_CTATADJ_ALIGN                   0
#define LD_LIA_LD_CONTROL_1_CFG_LD_CTATADJ_BITS                    4
#define LD_LIA_LD_CONTROL_1_CFG_LD_CTATADJ_SHIFT                   0
#define LD_LIA_LD_CONTROL_1_CFG_LD_CTATADJ_DEFAULT                 7

/***************************************************************************
 *LD_CONTROL_2 - LD control register 2
 ***************************************************************************/
/* LD_LIA :: LD_CONTROL_2 :: CFG_LD_SIG_EDGECTL_DLOAD [31:24] */
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DLOAD_MASK          0xff000000
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DLOAD_ALIGN         0
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DLOAD_BITS          8
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DLOAD_SHIFT         24
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DLOAD_DEFAULT       57

/* LD_LIA :: LD_CONTROL_2 :: reserved0 [23:21] */
#define LD_LIA_LD_CONTROL_2_reserved0_MASK                         0x00e00000
#define LD_LIA_LD_CONTROL_2_reserved0_ALIGN                        0
#define LD_LIA_LD_CONTROL_2_reserved0_BITS                         3
#define LD_LIA_LD_CONTROL_2_reserved0_SHIFT                        21

/* LD_LIA :: LD_CONTROL_2 :: CFG_LD_SIG_IBIASEN [20:20] */
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASEN_MASK                0x00100000
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASEN_ALIGN               0
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASEN_BITS                1
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASEN_SHIFT               20
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASEN_DEFAULT             0

/* LD_LIA :: LD_CONTROL_2 :: CFG_LD_SIG_IBIASCTL [19:08] */
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASCTL_MASK               0x000fff00
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASCTL_ALIGN              0
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASCTL_BITS               12
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASCTL_SHIFT              8
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_IBIASCTL_DEFAULT            0

/* LD_LIA :: LD_CONTROL_2 :: CFG_LD_SIG_EDGECTL [07:00] */
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_MASK                0x000000ff
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_ALIGN               0
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_BITS                8
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_SHIFT               0
#define LD_LIA_LD_CONTROL_2_CFG_LD_SIG_EDGECTL_DEFAULT             57

/***************************************************************************
 *LD_CONTROL_2a - LD control register 2a
 ***************************************************************************/
/* LD_LIA :: LD_CONTROL_2a :: reserved0 [31:22] */
#define LD_LIA_LD_CONTROL_2a_reserved0_MASK                        0xffc00000
#define LD_LIA_LD_CONTROL_2a_reserved0_ALIGN                       0
#define LD_LIA_LD_CONTROL_2a_reserved0_BITS                        10
#define LD_LIA_LD_CONTROL_2a_reserved0_SHIFT                       22

/* LD_LIA :: LD_CONTROL_2a :: CFG_LD_SQLCH_EN [21:21] */
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SQLCH_EN_MASK                  0x00200000
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SQLCH_EN_ALIGN                 0
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SQLCH_EN_BITS                  1
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SQLCH_EN_SHIFT                 21
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SQLCH_EN_DEFAULT               0

/* LD_LIA :: LD_CONTROL_2a :: CFG_LD_SIG_IBIASCALIBCTL [20:16] */
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_IBIASCALIBCTL_MASK         0x001f0000
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_IBIASCALIBCTL_ALIGN        0
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_IBIASCALIBCTL_BITS         5
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_IBIASCALIBCTL_SHIFT        16
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_IBIASCALIBCTL_DEFAULT      16

/* LD_LIA :: LD_CONTROL_2a :: reserved1 [15:09] */
#define LD_LIA_LD_CONTROL_2a_reserved1_MASK                        0x0000fe00
#define LD_LIA_LD_CONTROL_2a_reserved1_ALIGN                       0
#define LD_LIA_LD_CONTROL_2a_reserved1_BITS                        7
#define LD_LIA_LD_CONTROL_2a_reserved1_SHIFT                       9

/* LD_LIA :: LD_CONTROL_2a :: CFG_LD_SIG_DCYCTL [08:00] */
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_DCYCTL_MASK                0x000001ff
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_DCYCTL_ALIGN               0
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_DCYCTL_BITS                9
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_DCYCTL_SHIFT               0
#define LD_LIA_LD_CONTROL_2a_CFG_LD_SIG_DCYCTL_DEFAULT             0

/***************************************************************************
 *LD_CONTROL_3 - LD control register 3
 ***************************************************************************/
/* LD_LIA :: LD_CONTROL_3 :: reserved0 [31:30] */
#define LD_LIA_LD_CONTROL_3_reserved0_MASK                         0xc0000000
#define LD_LIA_LD_CONTROL_3_reserved0_ALIGN                        0
#define LD_LIA_LD_CONTROL_3_reserved0_BITS                         2
#define LD_LIA_LD_CONTROL_3_reserved0_SHIFT                        30

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_PREEMPHEN [29:26] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_PREEMPHEN_MASK              0x3c000000
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_PREEMPHEN_ALIGN             0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_PREEMPHEN_BITS              4
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_PREEMPHEN_SHIFT             26
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_PREEMPHEN_DEFAULT           0

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_POWERONRESET [25:25] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_POWERONRESET_MASK           0x02000000
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_POWERONRESET_ALIGN          0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_POWERONRESET_BITS           1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_POWERONRESET_SHIFT          25
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_POWERONRESET_DEFAULT        1

/* LD_LIA :: LD_CONTROL_3 :: reserved1 [24:24] */
#define LD_LIA_LD_CONTROL_3_reserved1_MASK                         0x01000000
#define LD_LIA_LD_CONTROL_3_reserved1_ALIGN                        0
#define LD_LIA_LD_CONTROL_3_reserved1_BITS                         1
#define LD_LIA_LD_CONTROL_3_reserved1_SHIFT                        24

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_INPUTSEL [23:23] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_INPUTSEL_MASK               0x00800000
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_INPUTSEL_ALIGN              0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_INPUTSEL_BITS               1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_INPUTSEL_SHIFT              23
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_INPUTSEL_DEFAULT            0

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IMODCTL [22:12] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCTL_MASK                0x007ff000
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCTL_ALIGN               0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCTL_BITS                11
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCTL_SHIFT               12
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCTL_DEFAULT             0

/* LD_LIA :: LD_CONTROL_3 :: reserved2 [11:09] */
#define LD_LIA_LD_CONTROL_3_reserved2_MASK                         0x00000e00
#define LD_LIA_LD_CONTROL_3_reserved2_ALIGN                        0
#define LD_LIA_LD_CONTROL_3_reserved2_BITS                         3
#define LD_LIA_LD_CONTROL_3_reserved2_SHIFT                        9

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IMODCALIBCTL [08:04] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCALIBCTL_MASK           0x000001f0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCALIBCTL_ALIGN          0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCALIBCTL_BITS           5
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCALIBCTL_SHIFT          4
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODCALIBCTL_DEFAULT        16

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IMODSTROBE [03:03] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODSTROBE_MASK             0x00000008
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODSTROBE_ALIGN            0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODSTROBE_BITS             1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODSTROBE_SHIFT            3
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODSTROBE_DEFAULT          0

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IMODEN [02:02] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODEN_MASK                 0x00000004
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODEN_ALIGN                0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODEN_BITS                 1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODEN_SHIFT                2
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IMODEN_DEFAULT              0

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IDACBURSTENDEP [01:01] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IDACBURSTENDEP_MASK         0x00000002
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IDACBURSTENDEP_ALIGN        0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IDACBURSTENDEP_BITS         1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IDACBURSTENDEP_SHIFT        1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IDACBURSTENDEP_DEFAULT      0

/* LD_LIA :: LD_CONTROL_3 :: CFG_LD_SIG_IBIASSTROBE [00:00] */
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IBIASSTROBE_MASK            0x00000001
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IBIASSTROBE_ALIGN           0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IBIASSTROBE_BITS            1
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IBIASSTROBE_SHIFT           0
#define LD_LIA_LD_CONTROL_3_CFG_LD_SIG_IBIASSTROBE_DEFAULT         0

/***************************************************************************
 *LD_CONTROL_3a - LD control register 3a
 ***************************************************************************/
/* LD_LIA :: LD_CONTROL_3a :: reserved0 [31:18] */
#define LD_LIA_LD_CONTROL_3a_reserved0_MASK                        0xfffc0000
#define LD_LIA_LD_CONTROL_3a_reserved0_ALIGN                       0
#define LD_LIA_LD_CONTROL_3a_reserved0_BITS                        14
#define LD_LIA_LD_CONTROL_3a_reserved0_SHIFT                       18

/* LD_LIA :: LD_CONTROL_3a :: CFG_LD_SIG_TAPDELAYCTRL [17:00] */
#define LD_LIA_LD_CONTROL_3a_CFG_LD_SIG_TAPDELAYCTRL_MASK          0x0003ffff
#define LD_LIA_LD_CONTROL_3a_CFG_LD_SIG_TAPDELAYCTRL_ALIGN         0
#define LD_LIA_LD_CONTROL_3a_CFG_LD_SIG_TAPDELAYCTRL_BITS          18
#define LD_LIA_LD_CONTROL_3a_CFG_LD_SIG_TAPDELAYCTRL_SHIFT         0
#define LD_LIA_LD_CONTROL_3a_CFG_LD_SIG_TAPDELAYCTRL_DEFAULT       0

/***************************************************************************
 *LD_COMMON_TRIG_CONFIG - LD common trigger configuration
 ***************************************************************************/
/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_RSTCNTONBENEGEDGE [31:31] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTCNTONBENEGEDGE_MASK 0x80000000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTCNTONBENEGEDGE_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTCNTONBENEGEDGE_BITS 1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTCNTONBENEGEDGE_SHIFT 31
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTCNTONBENEGEDGE_DEFAULT 1

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTCMPRST23 [30:30] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST23_MASK  0x40000000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST23_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST23_BITS  1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST23_SHIFT 30
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST23_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTCMPRST1 [29:29] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST1_MASK   0x20000000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST1_ALIGN  0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST1_BITS   1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST1_SHIFT  29
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST1_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTCMPRST0 [28:28] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST0_MASK   0x10000000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST0_ALIGN  0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST0_BITS   1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST0_SHIFT  28
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST0_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CMPTIME_HOLDVAL [27:16] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CMPTIME_HOLDVAL_MASK 0x0fff0000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CMPTIME_HOLDVAL_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CMPTIME_HOLDVAL_BITS 12
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CMPTIME_HOLDVAL_SHIFT 16
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CMPTIME_HOLDVAL_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_SETCOMMIT [15:15] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_SETCOMMIT_MASK    0x00008000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_SETCOMMIT_ALIGN   0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_SETCOMMIT_BITS    1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_SETCOMMIT_SHIFT   15
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_SETCOMMIT_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_RSTSR [14:14] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTSR_MASK        0x00004000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTSR_ALIGN       0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTSR_BITS        1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTSR_SHIFT       14
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_RSTSR_DEFAULT     0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_PATTGENEN [13:13] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_PATTGENEN_MASK    0x00002000
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_PATTGENEN_ALIGN   0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_PATTGENEN_BITS    1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_PATTGENEN_SHIFT   13
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_PATTGENEN_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_LDOUTSEL [12:11] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_LDOUTSEL_MASK     0x00001800
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_LDOUTSEL_ALIGN    0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_LDOUTSEL_BITS     2
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_LDOUTSEL_SHIFT    11
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_LDOUTSEL_DEFAULT  0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_INVCLKSR [10:10] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKSR_MASK     0x00000400
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKSR_ALIGN    0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKSR_BITS     1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKSR_SHIFT    10
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKSR_DEFAULT  0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_INVCLKRF [09:09] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKRF_MASK     0x00000200
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKRF_ALIGN    0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKRF_BITS     1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKRF_SHIFT    9
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKRF_DEFAULT  0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_INVCLKMSR [08:08] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKMSR_MASK    0x00000100
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKMSR_ALIGN   0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKMSR_BITS    1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKMSR_SHIFT   8
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INVCLKMSR_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_INFORCEDVAL [07:07] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INFORCEDVAL_MASK  0x00000080
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INFORCEDVAL_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INFORCEDVAL_BITS  1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INFORCEDVAL_SHIFT 7
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_INFORCEDVAL_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTRST [06:06] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTRST_MASK       0x00000040
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTRST_ALIGN      0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTRST_BITS       1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTRST_SHIFT      6
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTRST_DEFAULT    0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTEN [05:05] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTEN_MASK        0x00000020
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTEN_ALIGN       0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTEN_BITS        1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTEN_SHIFT       5
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTEN_DEFAULT     0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTCMPRST [04:04] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST_MASK    0x00000010
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST_ALIGN   0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST_BITS    1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST_SHIFT   4
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTCMPRST_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CNTBURSTENTRIG [03:03] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTBURSTENTRIG_MASK 0x00000008
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTBURSTENTRIG_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTBURSTENTRIG_BITS 1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTBURSTENTRIG_SHIFT 3
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CNTBURSTENTRIG_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CLKSEL [02:02] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CLKSEL_MASK       0x00000004
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CLKSEL_ALIGN      0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CLKSEL_BITS       1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CLKSEL_SHIFT      2
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CLKSEL_DEFAULT    0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CIDCOUNTDEP [01:01] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDCOUNTDEP_MASK  0x00000002
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDCOUNTDEP_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDCOUNTDEP_BITS  1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDCOUNTDEP_SHIFT 1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDCOUNTDEP_DEFAULT 0

/* LD_LIA :: LD_COMMON_TRIG_CONFIG :: CFG_LD_TRIG_CIDBURSTDEP [00:00] */
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDBURSTDEP_MASK  0x00000001
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDBURSTDEP_ALIGN 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDBURSTDEP_BITS  1
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDBURSTDEP_SHIFT 0
#define LD_LIA_LD_COMMON_TRIG_CONFIG_CFG_LD_TRIG_CIDBURSTDEP_DEFAULT 0

/***************************************************************************
 *LD_TRIG_SEED - LD trigger seed value
 ***************************************************************************/
/* LD_LIA :: LD_TRIG_SEED :: reserved0 [31:24] */
#define LD_LIA_LD_TRIG_SEED_reserved0_MASK                         0xff000000
#define LD_LIA_LD_TRIG_SEED_reserved0_ALIGN                        0
#define LD_LIA_LD_TRIG_SEED_reserved0_BITS                         8
#define LD_LIA_LD_TRIG_SEED_reserved0_SHIFT                        24

/* LD_LIA :: LD_TRIG_SEED :: CFG_LD_TRIG_SEED [23:00] */
#define LD_LIA_LD_TRIG_SEED_CFG_LD_TRIG_SEED_MASK                  0x00ffffff
#define LD_LIA_LD_TRIG_SEED_CFG_LD_TRIG_SEED_ALIGN                 0
#define LD_LIA_LD_TRIG_SEED_CFG_LD_TRIG_SEED_BITS                  24
#define LD_LIA_LD_TRIG_SEED_CFG_LD_TRIG_SEED_SHIFT                 0
#define LD_LIA_LD_TRIG_SEED_CFG_LD_TRIG_SEED_DEFAULT               0

/***************************************************************************
 *LD_TRIG0_CONFIG - LD trigger 0 configuration
 ***************************************************************************/
/* LD_LIA :: LD_TRIG0_CONFIG :: reserved0 [31:23] */
#define LD_LIA_LD_TRIG0_CONFIG_reserved0_MASK                      0xff800000
#define LD_LIA_LD_TRIG0_CONFIG_reserved0_ALIGN                     0
#define LD_LIA_LD_TRIG0_CONFIG_reserved0_BITS                      9
#define LD_LIA_LD_TRIG0_CONFIG_reserved0_SHIFT                     23

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_XTALK_DET_LENGTH [22:20] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_XTALK_DET_LENGTH_MASK  0x00700000
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_XTALK_DET_LENGTH_ALIGN 0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_XTALK_DET_LENGTH_BITS  3
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_XTALK_DET_LENGTH_SHIFT 20
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_XTALK_DET_LENGTH_DEFAULT 0

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_DELAYMATCH_SEL [19:16] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_DELAYMATCH_SEL_MASK    0x000f0000
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_DELAYMATCH_SEL_ALIGN   0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_DELAYMATCH_SEL_BITS    4
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_DELAYMATCH_SEL_SHIFT   16
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_DELAYMATCH_SEL_DEFAULT 0

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_CMPTIME [15:04] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_MASK           0x0000fff0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_ALIGN          0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_BITS           12
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_SHIFT          4
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_DEFAULT        0

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_RSTMATCH [03:03] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_RSTMATCH_MASK          0x00000008
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_RSTMATCH_ALIGN         0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_RSTMATCH_BITS          1
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_RSTMATCH_SHIFT         3
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_RSTMATCH_DEFAULT       0

/* LD_LIA :: LD_TRIG0_CONFIG :: reserved1 [02:02] */
#define LD_LIA_LD_TRIG0_CONFIG_reserved1_MASK                      0x00000004
#define LD_LIA_LD_TRIG0_CONFIG_reserved1_ALIGN                     0
#define LD_LIA_LD_TRIG0_CONFIG_reserved1_BITS                      1
#define LD_LIA_LD_TRIG0_CONFIG_reserved1_SHIFT                     2

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_CMPTIME_CFG [01:01] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_CFG_MASK       0x00000002
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_CFG_ALIGN      0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_CFG_BITS       1
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_CFG_SHIFT      1
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CMPTIME_CFG_DEFAULT    0

/* LD_LIA :: LD_TRIG0_CONFIG :: CFG_LD_TRIG0_CIDMATCHEN [00:00] */
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CIDMATCHEN_MASK        0x00000001
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CIDMATCHEN_ALIGN       0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CIDMATCHEN_BITS        1
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CIDMATCHEN_SHIFT       0
#define LD_LIA_LD_TRIG0_CONFIG_CFG_LD_TRIG0_CIDMATCHEN_DEFAULT     0

/***************************************************************************
 *LD_TRIG0_MASK - LD trigger 0 pattern mask
 ***************************************************************************/
/* LD_LIA :: LD_TRIG0_MASK :: reserved0 [31:24] */
#define LD_LIA_LD_TRIG0_MASK_reserved0_MASK                        0xff000000
#define LD_LIA_LD_TRIG0_MASK_reserved0_ALIGN                       0
#define LD_LIA_LD_TRIG0_MASK_reserved0_BITS                        8
#define LD_LIA_LD_TRIG0_MASK_reserved0_SHIFT                       24

/* LD_LIA :: LD_TRIG0_MASK :: CFG_LD_TRIG0_MASK [23:00] */
#define LD_LIA_LD_TRIG0_MASK_CFG_LD_TRIG0_MASK_MASK                0x00ffffff
#define LD_LIA_LD_TRIG0_MASK_CFG_LD_TRIG0_MASK_ALIGN               0
#define LD_LIA_LD_TRIG0_MASK_CFG_LD_TRIG0_MASK_BITS                24
#define LD_LIA_LD_TRIG0_MASK_CFG_LD_TRIG0_MASK_SHIFT               0
#define LD_LIA_LD_TRIG0_MASK_CFG_LD_TRIG0_MASK_DEFAULT             0

/***************************************************************************
 *LD_TRIG0_PATT - LD trigger 0 pattern
 ***************************************************************************/
/* LD_LIA :: LD_TRIG0_PATT :: reserved0 [31:24] */
#define LD_LIA_LD_TRIG0_PATT_reserved0_MASK                        0xff000000
#define LD_LIA_LD_TRIG0_PATT_reserved0_ALIGN                       0
#define LD_LIA_LD_TRIG0_PATT_reserved0_BITS                        8
#define LD_LIA_LD_TRIG0_PATT_reserved0_SHIFT                       24

/* LD_LIA :: LD_TRIG0_PATT :: CFG_LD_TRIG0_PATT [23:00] */
#define LD_LIA_LD_TRIG0_PATT_CFG_LD_TRIG0_PATT_MASK                0x00ffffff
#define LD_LIA_LD_TRIG0_PATT_CFG_LD_TRIG0_PATT_ALIGN               0
#define LD_LIA_LD_TRIG0_PATT_CFG_LD_TRIG0_PATT_BITS                24
#define LD_LIA_LD_TRIG0_PATT_CFG_LD_TRIG0_PATT_SHIFT               0
#define LD_LIA_LD_TRIG0_PATT_CFG_LD_TRIG0_PATT_DEFAULT             0

/***************************************************************************
 *LD_TRIG0_STATUS - LD trigger 0 event status
 ***************************************************************************/
/* LD_LIA :: LD_TRIG0_STATUS :: CFG_LD_TRIG0_FOUNDMATCH [31:31] */
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_FOUNDMATCH_MASK        0x80000000
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_FOUNDMATCH_ALIGN       0
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_FOUNDMATCH_BITS        1
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_FOUNDMATCH_SHIFT       31

/* LD_LIA :: LD_TRIG0_STATUS :: CFG_LD_MPD_CMPOUT0 [30:30] */
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_MPD_CMPOUT0_MASK             0x40000000
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_MPD_CMPOUT0_ALIGN            0
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_MPD_CMPOUT0_BITS             1
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_MPD_CMPOUT0_SHIFT            30

/* LD_LIA :: LD_TRIG0_STATUS :: reserved0 [29:24] */
#define LD_LIA_LD_TRIG0_STATUS_reserved0_MASK                      0x3f000000
#define LD_LIA_LD_TRIG0_STATUS_reserved0_ALIGN                     0
#define LD_LIA_LD_TRIG0_STATUS_reserved0_BITS                      6
#define LD_LIA_LD_TRIG0_STATUS_reserved0_SHIFT                     24

/* LD_LIA :: LD_TRIG0_STATUS :: CFG_LD_TRIG0_SRSNAPSHOT [23:00] */
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_SRSNAPSHOT_MASK        0x00ffffff
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_SRSNAPSHOT_ALIGN       0
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_SRSNAPSHOT_BITS        24
#define LD_LIA_LD_TRIG0_STATUS_CFG_LD_TRIG0_SRSNAPSHOT_SHIFT       0

/***************************************************************************
 *LD_TRIG1_CONFIG - LD trigger 1 configuration
 ***************************************************************************/
/* LD_LIA :: LD_TRIG1_CONFIG :: reserved0 [31:23] */
#define LD_LIA_LD_TRIG1_CONFIG_reserved0_MASK                      0xff800000
#define LD_LIA_LD_TRIG1_CONFIG_reserved0_ALIGN                     0
#define LD_LIA_LD_TRIG1_CONFIG_reserved0_BITS                      9
#define LD_LIA_LD_TRIG1_CONFIG_reserved0_SHIFT                     23

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_XTALK_DET_LENGTH [22:20] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_XTALK_DET_LENGTH_MASK  0x00700000
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_XTALK_DET_LENGTH_ALIGN 0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_XTALK_DET_LENGTH_BITS  3
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_XTALK_DET_LENGTH_SHIFT 20
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_XTALK_DET_LENGTH_DEFAULT 0

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_DELAYMATCH_SEL [19:16] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_DELAYMATCH_SEL_MASK    0x000f0000
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_DELAYMATCH_SEL_ALIGN   0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_DELAYMATCH_SEL_BITS    4
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_DELAYMATCH_SEL_SHIFT   16
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_DELAYMATCH_SEL_DEFAULT 0

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_CMPTIME [15:04] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_MASK           0x0000fff0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_ALIGN          0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_BITS           12
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_SHIFT          4
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_DEFAULT        0

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_RSTMATCH [03:03] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_RSTMATCH_MASK          0x00000008
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_RSTMATCH_ALIGN         0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_RSTMATCH_BITS          1
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_RSTMATCH_SHIFT         3
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_RSTMATCH_DEFAULT       0

/* LD_LIA :: LD_TRIG1_CONFIG :: reserved1 [02:02] */
#define LD_LIA_LD_TRIG1_CONFIG_reserved1_MASK                      0x00000004
#define LD_LIA_LD_TRIG1_CONFIG_reserved1_ALIGN                     0
#define LD_LIA_LD_TRIG1_CONFIG_reserved1_BITS                      1
#define LD_LIA_LD_TRIG1_CONFIG_reserved1_SHIFT                     2

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_CMPTIME_CFG [01:01] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_CFG_MASK       0x00000002
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_CFG_ALIGN      0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_CFG_BITS       1
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_CFG_SHIFT      1
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CMPTIME_CFG_DEFAULT    0

/* LD_LIA :: LD_TRIG1_CONFIG :: CFG_LD_TRIG1_CIDMATCHEN [00:00] */
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CIDMATCHEN_MASK        0x00000001
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CIDMATCHEN_ALIGN       0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CIDMATCHEN_BITS        1
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CIDMATCHEN_SHIFT       0
#define LD_LIA_LD_TRIG1_CONFIG_CFG_LD_TRIG1_CIDMATCHEN_DEFAULT     0

/***************************************************************************
 *LD_TRIG1_MASK - LD trigger 1 pattern mask
 ***************************************************************************/
/* LD_LIA :: LD_TRIG1_MASK :: reserved0 [31:24] */
#define LD_LIA_LD_TRIG1_MASK_reserved0_MASK                        0xff000000
#define LD_LIA_LD_TRIG1_MASK_reserved0_ALIGN                       0
#define LD_LIA_LD_TRIG1_MASK_reserved0_BITS                        8
#define LD_LIA_LD_TRIG1_MASK_reserved0_SHIFT                       24

/* LD_LIA :: LD_TRIG1_MASK :: CFG_LD_TRIG1_MASK [23:00] */
#define LD_LIA_LD_TRIG1_MASK_CFG_LD_TRIG1_MASK_MASK                0x00ffffff
#define LD_LIA_LD_TRIG1_MASK_CFG_LD_TRIG1_MASK_ALIGN               0
#define LD_LIA_LD_TRIG1_MASK_CFG_LD_TRIG1_MASK_BITS                24
#define LD_LIA_LD_TRIG1_MASK_CFG_LD_TRIG1_MASK_SHIFT               0
#define LD_LIA_LD_TRIG1_MASK_CFG_LD_TRIG1_MASK_DEFAULT             0

/***************************************************************************
 *LD_TRIG1_PATT - LD trigger 1 pattern
 ***************************************************************************/
/* LD_LIA :: LD_TRIG1_PATT :: reserved0 [31:24] */
#define LD_LIA_LD_TRIG1_PATT_reserved0_MASK                        0xff000000
#define LD_LIA_LD_TRIG1_PATT_reserved0_ALIGN                       0
#define LD_LIA_LD_TRIG1_PATT_reserved0_BITS                        8
#define LD_LIA_LD_TRIG1_PATT_reserved0_SHIFT                       24

/* LD_LIA :: LD_TRIG1_PATT :: CFG_LD_TRIG1_PATT [23:00] */
#define LD_LIA_LD_TRIG1_PATT_CFG_LD_TRIG1_PATT_MASK                0x00ffffff
#define LD_LIA_LD_TRIG1_PATT_CFG_LD_TRIG1_PATT_ALIGN               0
#define LD_LIA_LD_TRIG1_PATT_CFG_LD_TRIG1_PATT_BITS                24
#define LD_LIA_LD_TRIG1_PATT_CFG_LD_TRIG1_PATT_SHIFT               0
#define LD_LIA_LD_TRIG1_PATT_CFG_LD_TRIG1_PATT_DEFAULT             0

/***************************************************************************
 *LD_TRIG1_STATUS - LD trigger 1 event status
 ***************************************************************************/
/* LD_LIA :: LD_TRIG1_STATUS :: CFG_LD_TRIG1_FOUNDMATCH [31:31] */
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_FOUNDMATCH_MASK        0x80000000
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_FOUNDMATCH_ALIGN       0
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_FOUNDMATCH_BITS        1
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_FOUNDMATCH_SHIFT       31

/* LD_LIA :: LD_TRIG1_STATUS :: CFG_LD_MPD_CMPOUT1 [30:30] */
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_MPD_CMPOUT1_MASK             0x40000000
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_MPD_CMPOUT1_ALIGN            0
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_MPD_CMPOUT1_BITS             1
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_MPD_CMPOUT1_SHIFT            30

/* LD_LIA :: LD_TRIG1_STATUS :: reserved0 [29:24] */
#define LD_LIA_LD_TRIG1_STATUS_reserved0_MASK                      0x3f000000
#define LD_LIA_LD_TRIG1_STATUS_reserved0_ALIGN                     0
#define LD_LIA_LD_TRIG1_STATUS_reserved0_BITS                      6
#define LD_LIA_LD_TRIG1_STATUS_reserved0_SHIFT                     24

/* LD_LIA :: LD_TRIG1_STATUS :: CFG_LD_TRIG1_SRSNAPSHOT [23:00] */
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_SRSNAPSHOT_MASK        0x00ffffff
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_SRSNAPSHOT_ALIGN       0
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_SRSNAPSHOT_BITS        24
#define LD_LIA_LD_TRIG1_STATUS_CFG_LD_TRIG1_SRSNAPSHOT_SHIFT       0

/***************************************************************************
 *LD_PATT_GEN_MASK - LD pattern generator pattern mask
 ***************************************************************************/
/* LD_LIA :: LD_PATT_GEN_MASK :: reserved0 [31:24] */
#define LD_LIA_LD_PATT_GEN_MASK_reserved0_MASK                     0xff000000
#define LD_LIA_LD_PATT_GEN_MASK_reserved0_ALIGN                    0
#define LD_LIA_LD_PATT_GEN_MASK_reserved0_BITS                     8
#define LD_LIA_LD_PATT_GEN_MASK_reserved0_SHIFT                    24

/* LD_LIA :: LD_PATT_GEN_MASK :: CFG_LD_PATT_GEN_MASK [23:00] */
#define LD_LIA_LD_PATT_GEN_MASK_CFG_LD_PATT_GEN_MASK_MASK          0x00ffffff
#define LD_LIA_LD_PATT_GEN_MASK_CFG_LD_PATT_GEN_MASK_ALIGN         0
#define LD_LIA_LD_PATT_GEN_MASK_CFG_LD_PATT_GEN_MASK_BITS          24
#define LD_LIA_LD_PATT_GEN_MASK_CFG_LD_PATT_GEN_MASK_SHIFT         0
#define LD_LIA_LD_PATT_GEN_MASK_CFG_LD_PATT_GEN_MASK_DEFAULT       0

/***************************************************************************
 *LD_PATT_GEN_PATT - LD pattern generator pattern
 ***************************************************************************/
/* LD_LIA :: LD_PATT_GEN_PATT :: reserved0 [31:24] */
#define LD_LIA_LD_PATT_GEN_PATT_reserved0_MASK                     0xff000000
#define LD_LIA_LD_PATT_GEN_PATT_reserved0_ALIGN                    0
#define LD_LIA_LD_PATT_GEN_PATT_reserved0_BITS                     8
#define LD_LIA_LD_PATT_GEN_PATT_reserved0_SHIFT                    24

/* LD_LIA :: LD_PATT_GEN_PATT :: CFG_LD_PATT_GEN_PATT [23:00] */
#define LD_LIA_LD_PATT_GEN_PATT_CFG_LD_PATT_GEN_PATT_MASK          0x00ffffff
#define LD_LIA_LD_PATT_GEN_PATT_CFG_LD_PATT_GEN_PATT_ALIGN         0
#define LD_LIA_LD_PATT_GEN_PATT_CFG_LD_PATT_GEN_PATT_BITS          24
#define LD_LIA_LD_PATT_GEN_PATT_CFG_LD_PATT_GEN_PATT_SHIFT         0
#define LD_LIA_LD_PATT_GEN_PATT_CFG_LD_PATT_GEN_PATT_DEFAULT       0

/***************************************************************************
 *LD_MPD_CONTROL_1 - LD monitor photo-diode control register 1
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_TIAOUTCM [31:30] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAOUTCM_MASK           0xc0000000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAOUTCM_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAOUTCM_BITS           2
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAOUTCM_SHIFT          30
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAOUTCM_DEFAULT        2

/* LD_LIA :: LD_MPD_CONTROL_1 :: reserved0 [29:29] */
#define LD_LIA_LD_MPD_CONTROL_1_reserved0_MASK                     0x20000000
#define LD_LIA_LD_MPD_CONTROL_1_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_1_reserved0_BITS                     1
#define LD_LIA_LD_MPD_CONTROL_1_reserved0_SHIFT                    29

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_TIAINSWITCH_EN [28:28] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAINSWITCH_EN_MASK     0x10000000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAINSWITCH_EN_ALIGN    0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAINSWITCH_EN_BITS     1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAINSWITCH_EN_SHIFT    28
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_TIAINSWITCH_EN_DEFAULT  1

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_PWRDNTIA [27:27] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_PWRDNTIA_MASK           0x08000000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_PWRDNTIA_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_PWRDNTIA_BITS           1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_PWRDNTIA_SHIFT          27
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_PWRDNTIA_DEFAULT        1

/* LD_LIA :: LD_MPD_CONTROL_1 :: reserved1 [26:22] */
#define LD_LIA_LD_MPD_CONTROL_1_reserved1_MASK                     0x07c00000
#define LD_LIA_LD_MPD_CONTROL_1_reserved1_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_1_reserved1_BITS                     5
#define LD_LIA_LD_MPD_CONTROL_1_reserved1_SHIFT                    22

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_IBUPDN [21:21] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_IBUPDN_MASK             0x00200000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_IBUPDN_ALIGN            0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_IBUPDN_BITS             1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_IBUPDN_SHIFT            21
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_IBUPDN_DEFAULT          0

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_EYESAFETYTHR [20:14] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYTHR_MASK       0x001fc000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYTHR_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYTHR_BITS       7
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYTHR_SHIFT      14
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYTHR_DEFAULT    64

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_EYESAFETYEN [13:13] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYEN_MASK        0x00002000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYEN_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYEN_BITS        1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYEN_SHIFT       13
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYESAFETYEN_DEFAULT     1

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_EYEDACSTROBE [12:12] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEDACSTROBE_MASK       0x00001000
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEDACSTROBE_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEDACSTROBE_BITS       1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEDACSTROBE_SHIFT      12
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEDACSTROBE_DEFAULT    0

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_EYEHYSTADJ [11:08] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEHYSTADJ_MASK         0x00000f00
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEHYSTADJ_ALIGN        0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEHYSTADJ_BITS         4
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEHYSTADJ_SHIFT        8
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYEHYSTADJ_DEFAULT      1

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_EYECALIBCTL [07:04] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYECALIBCTL_MASK        0x000000f0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYECALIBCTL_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYECALIBCTL_BITS        4
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYECALIBCTL_SHIFT       4
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_EYECALIBCTL_DEFAULT     8

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_CLAMPTIA [03:01] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_CLAMPTIA_MASK           0x0000000e
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_CLAMPTIA_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_CLAMPTIA_BITS           3
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_CLAMPTIA_SHIFT          1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_CLAMPTIA_DEFAULT        2

/* LD_LIA :: LD_MPD_CONTROL_1 :: CFG_LD_MPD_BIASSUENTIA [00:00] */
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_BIASSUENTIA_MASK        0x00000001
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_BIASSUENTIA_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_BIASSUENTIA_BITS        1
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_BIASSUENTIA_SHIFT       0
#define LD_LIA_LD_MPD_CONTROL_1_CFG_LD_MPD_BIASSUENTIA_DEFAULT     1

/***************************************************************************
 *LD_MPD_CONTROL_2 - LD monitor photo-diode control register 2
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_2 :: reserved0 [31:29] */
#define LD_LIA_LD_MPD_CONTROL_2_reserved0_MASK                     0xe0000000
#define LD_LIA_LD_MPD_CONTROL_2_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_2_reserved0_BITS                     3
#define LD_LIA_LD_MPD_CONTROL_2_reserved0_SHIFT                    29

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_POSTVGAGAIN [28:26] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_POSTVGAGAIN_MASK        0x1c000000
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_POSTVGAGAIN_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_POSTVGAGAIN_BITS        3
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_POSTVGAGAIN_SHIFT       26
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_POSTVGAGAIN_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_EYESAFETYDACOFFSETADJ [25:21] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_EYESAFETYDACOFFSETADJ_MASK 0x03e00000
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_EYESAFETYDACOFFSETADJ_ALIGN 0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_EYESAFETYDACOFFSETADJ_BITS 5
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_EYESAFETYDACOFFSETADJ_SHIFT 21
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_EYESAFETYDACOFFSETADJ_DEFAULT 0

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_IOFFS [20:12] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_IOFFS_MASK              0x001ff000
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_IOFFS_ALIGN             0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_IOFFS_BITS              9
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_IOFFS_SHIFT             12
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_IOFFS_DEFAULT           0

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_VGAGAIN [11:08] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGAGAIN_MASK            0x00000f00
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGAGAIN_ALIGN           0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGAGAIN_BITS            4
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGAGAIN_SHIFT           8
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGAGAIN_DEFAULT         3

/* LD_LIA :: LD_MPD_CONTROL_2 :: reserved1 [07:07] */
#define LD_LIA_LD_MPD_CONTROL_2_reserved1_MASK                     0x00000080
#define LD_LIA_LD_MPD_CONTROL_2_reserved1_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_2_reserved1_BITS                     1
#define LD_LIA_LD_MPD_CONTROL_2_reserved1_SHIFT                    7

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_VGABWCTRL [06:04] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGABWCTRL_MASK          0x00000070
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGABWCTRL_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGABWCTRL_BITS          3
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGABWCTRL_SHIFT         4
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_VGABWCTRL_DEFAULT       2

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_TIARESETB [03:03] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESETB_MASK          0x00000008
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESETB_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESETB_BITS          1
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESETB_SHIFT         3
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESETB_DEFAULT       1

/* LD_LIA :: LD_MPD_CONTROL_2 :: CFG_LD_MPD_TIARESCAL [02:00] */
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESCAL_MASK          0x00000007
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESCAL_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESCAL_BITS          3
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESCAL_SHIFT         0
#define LD_LIA_LD_MPD_CONTROL_2_CFG_LD_MPD_TIARESCAL_DEFAULT       3

/***************************************************************************
 *LD_MPD_CONTROL_3 - LD monitor photo-diode control register 3
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_TIACASCBIASCTRL [31:28] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_TIACASCBIASCTRL_MASK    0xf0000000
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_TIACASCBIASCTRL_ALIGN   0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_TIACASCBIASCTRL_BITS    4
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_TIACASCBIASCTRL_SHIFT   28
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_TIACASCBIASCTRL_DEFAULT 2

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUEOFFSETADJ [27:23] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEOFFSETADJ_MASK     0x0f800000
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEOFFSETADJ_ALIGN    0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEOFFSETADJ_BITS     5
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEOFFSETADJ_SHIFT    23
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEOFFSETADJ_DEFAULT  0

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUETHR [22:16] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUETHR_MASK           0x007f0000
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUETHR_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUETHR_BITS           7
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUETHR_SHIFT          16
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUETHR_DEFAULT        64

/* LD_LIA :: LD_MPD_CONTROL_3 :: reserved0 [15:15] */
#define LD_LIA_LD_MPD_CONTROL_3_reserved0_MASK                     0x00008000
#define LD_LIA_LD_MPD_CONTROL_3_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_3_reserved0_BITS                     1
#define LD_LIA_LD_MPD_CONTROL_3_reserved0_SHIFT                    15

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUEDETEN [14:14] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDETEN_MASK         0x00004000
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDETEN_ALIGN        0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDETEN_BITS         1
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDETEN_SHIFT        14
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDETEN_DEFAULT      0

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUEDACSTROBE [13:13] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDACSTROBE_MASK     0x00002000
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDACSTROBE_ALIGN    0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDACSTROBE_BITS     1
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDACSTROBE_SHIFT    13
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEDACSTROBE_DEFAULT  0

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUEHYSTADJ [12:09] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEHYSTADJ_MASK       0x00001e00
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEHYSTADJ_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEHYSTADJ_BITS       4
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEHYSTADJ_SHIFT      9
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUEHYSTADJ_DEFAULT    1

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_ROGUECALIBCTL [08:05] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUECALIBCTL_MASK      0x000001e0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUECALIBCTL_ALIGN     0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUECALIBCTL_BITS      4
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUECALIBCTL_SHIFT     5
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_ROGUECALIBCTL_DEFAULT   8

/* LD_LIA :: LD_MPD_CONTROL_3 :: CFG_LD_MPD_INGMCTRL [04:03] */
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_INGMCTRL_MASK           0x00000018
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_INGMCTRL_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_INGMCTRL_BITS           2
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_INGMCTRL_SHIFT          3
#define LD_LIA_LD_MPD_CONTROL_3_CFG_LD_MPD_INGMCTRL_DEFAULT        2

/* LD_LIA :: LD_MPD_CONTROL_3 :: reserved1 [02:00] */
#define LD_LIA_LD_MPD_CONTROL_3_reserved1_MASK                     0x00000007
#define LD_LIA_LD_MPD_CONTROL_3_reserved1_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_3_reserved1_BITS                     3
#define LD_LIA_LD_MPD_CONTROL_3_reserved1_SHIFT                    0

/***************************************************************************
 *LD_MPD_CMP_DAC_0_CONTROL - LD monitor photo-diode comparator DAC-0 control
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: reserved0 [31:30] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved0_MASK             0xc0000000
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved0_ALIGN            0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved0_BITS             2
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved0_SHIFT            30

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_DACRANGE [29:28] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACRANGE_MASK   0x30000000
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACRANGE_ALIGN  0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACRANGE_BITS   2
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACRANGE_SHIFT  28
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACRANGE_DEFAULT 2

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: reserved1 [27:27] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved1_MASK             0x08000000
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved1_ALIGN            0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved1_BITS             1
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved1_SHIFT            27

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_DACOFFSETADJ_0 [26:22] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACOFFSETADJ_0_MASK 0x07c00000
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACOFFSETADJ_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACOFFSETADJ_0_BITS 5
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACOFFSETADJ_0_SHIFT 22
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DACOFFSETADJ_0_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_STROBEREF_0 [21:21] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_STROBEREF_0_MASK 0x00200000
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_STROBEREF_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_STROBEREF_0_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_STROBEREF_0_SHIFT 21
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_STROBEREF_0_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_DAC_REF_0 [20:10] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DAC_REF_0_MASK  0x001ffc00
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DAC_REF_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DAC_REF_0_BITS  11
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DAC_REF_0_SHIFT 10
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_DAC_REF_0_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_PWRDNDAC_0 [09:09] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_PWRDNDAC_0_MASK 0x00000200
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_PWRDNDAC_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_PWRDNDAC_0_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_PWRDNDAC_0_SHIFT 9
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_PWRDNDAC_0_DEFAULT 1

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_CMPSTROBESEL_0 [08:08] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBESEL_0_MASK 0x00000100
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBESEL_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBESEL_0_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBESEL_0_SHIFT 8
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBESEL_0_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_CMPSTROBEMAN_0 [07:07] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_0_MASK 0x00000080
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_0_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_0_SHIFT 7
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_0_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: reserved2 [06:05] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved2_MASK             0x00000060
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved2_ALIGN            0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved2_BITS             2
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_reserved2_SHIFT            5

/* LD_LIA :: LD_MPD_CMP_DAC_0_CONTROL :: CFG_LD_MPD_CALIBCTL_0 [04:00] */
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CALIBCTL_0_MASK 0x0000001f
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CALIBCTL_0_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CALIBCTL_0_BITS 5
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CALIBCTL_0_SHIFT 0
#define LD_LIA_LD_MPD_CMP_DAC_0_CONTROL_CFG_LD_MPD_CALIBCTL_0_DEFAULT 16

/***************************************************************************
 *LD_MPD_CMP_DAC_1_CONTROL - LD monitor photo-diode comparator DAC-1 control
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: reserved0 [31:27] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved0_MASK             0xf8000000
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved0_ALIGN            0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved0_BITS             5
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved0_SHIFT            27

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_DACOFFSETADJ_1 [26:22] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DACOFFSETADJ_1_MASK 0x07c00000
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DACOFFSETADJ_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DACOFFSETADJ_1_BITS 5
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DACOFFSETADJ_1_SHIFT 22
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DACOFFSETADJ_1_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_STROBEREF_1 [21:21] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_STROBEREF_1_MASK 0x00200000
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_STROBEREF_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_STROBEREF_1_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_STROBEREF_1_SHIFT 21
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_STROBEREF_1_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_DAC_REF_1 [20:10] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DAC_REF_1_MASK  0x001ffc00
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DAC_REF_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DAC_REF_1_BITS  11
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DAC_REF_1_SHIFT 10
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_DAC_REF_1_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_PWRDNDAC_1 [09:09] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_PWRDNDAC_1_MASK 0x00000200
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_PWRDNDAC_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_PWRDNDAC_1_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_PWRDNDAC_1_SHIFT 9
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_PWRDNDAC_1_DEFAULT 1

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_CMPSTROBESEL_1 [08:08] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBESEL_1_MASK 0x00000100
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBESEL_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBESEL_1_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBESEL_1_SHIFT 8
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBESEL_1_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_CMPSTROBEMAN_1 [07:07] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_1_MASK 0x00000080
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_1_BITS 1
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_1_SHIFT 7
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CMPSTROBEMAN_1_DEFAULT 0

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: reserved1 [06:05] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved1_MASK             0x00000060
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved1_ALIGN            0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved1_BITS             2
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_reserved1_SHIFT            5

/* LD_LIA :: LD_MPD_CMP_DAC_1_CONTROL :: CFG_LD_MPD_CALIBCTL_1 [04:00] */
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CALIBCTL_1_MASK 0x0000001f
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CALIBCTL_1_ALIGN 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CALIBCTL_1_BITS 5
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CALIBCTL_1_SHIFT 0
#define LD_LIA_LD_MPD_CMP_DAC_1_CONTROL_CFG_LD_MPD_CALIBCTL_1_DEFAULT 16

/***************************************************************************
 *LD_PLL_CONTROL_1 - LD LC PLL control register 1
 ***************************************************************************/
/* LD_LIA :: LD_PLL_CONTROL_1 :: reserved0 [31:26] */
#define LD_LIA_LD_PLL_CONTROL_1_reserved0_MASK                     0xfc000000
#define LD_LIA_LD_PLL_CONTROL_1_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_CONTROL_1_reserved0_BITS                     6
#define LD_LIA_LD_PLL_CONTROL_1_reserved0_SHIFT                    26

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_KP [25:22] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KP_MASK                 0x03c00000
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KP_ALIGN                0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KP_BITS                 4
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KP_SHIFT                22
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KP_DEFAULT              9

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_KI [21:19] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KI_MASK                 0x00380000
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KI_ALIGN                0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KI_BITS                 3
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KI_SHIFT                19
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KI_DEFAULT              1

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_KA [18:16] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KA_MASK                 0x00070000
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KA_ALIGN                0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KA_BITS                 3
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KA_SHIFT                16
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_KA_DEFAULT              3

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_NDIV_INT [15:08] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_NDIV_INT_MASK           0x0000ff00
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_NDIV_INT_ALIGN          0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_NDIV_INT_BITS           8
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_NDIV_INT_SHIFT          8
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_NDIV_INT_DEFAULT        0

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_PDIV [07:05] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PDIV_MASK               0x000000e0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PDIV_ALIGN              0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PDIV_BITS               3
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PDIV_SHIFT              5
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PDIV_DEFAULT            1

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_REFCLK_SEL [04:04] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_REFCLK_SEL_MASK         0x00000010
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_REFCLK_SEL_ALIGN        0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_REFCLK_SEL_BITS         1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_REFCLK_SEL_SHIFT        4
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_REFCLK_SEL_DEFAULT      0

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_POST_RESETB [03:03] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_POST_RESETB_MASK        0x00000008
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_POST_RESETB_ALIGN       0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_POST_RESETB_BITS        1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_POST_RESETB_SHIFT       3
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_POST_RESETB_DEFAULT     0

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_RESETB [02:02] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_RESETB_MASK             0x00000004
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_RESETB_ALIGN            0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_RESETB_BITS             1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_RESETB_SHIFT            2
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_RESETB_DEFAULT          0

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_PWRDN [01:01] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PWRDN_MASK              0x00000002
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PWRDN_ALIGN             0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PWRDN_BITS              1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PWRDN_SHIFT             1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_PWRDN_DEFAULT           1

/* LD_LIA :: LD_PLL_CONTROL_1 :: CFG_LD_PLL_MODE1P25 [00:00] */
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_MODE1P25_MASK           0x00000001
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_MODE1P25_ALIGN          0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_MODE1P25_BITS           1
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_MODE1P25_SHIFT          0
#define LD_LIA_LD_PLL_CONTROL_1_CFG_LD_PLL_MODE1P25_DEFAULT        0

/***************************************************************************
 *LD_PLL_CONTROL_2 - LD LC PLL control register 2
 ***************************************************************************/
/* LD_LIA :: LD_PLL_CONTROL_2 :: reserved0 [31:20] */
#define LD_LIA_LD_PLL_CONTROL_2_reserved0_MASK                     0xfff00000
#define LD_LIA_LD_PLL_CONTROL_2_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_CONTROL_2_reserved0_BITS                     12
#define LD_LIA_LD_PLL_CONTROL_2_reserved0_SHIFT                    20

/* LD_LIA :: LD_PLL_CONTROL_2 :: CFG_LD_PLL_NDIV_FRAC [19:00] */
#define LD_LIA_LD_PLL_CONTROL_2_CFG_LD_PLL_NDIV_FRAC_MASK          0x000fffff
#define LD_LIA_LD_PLL_CONTROL_2_CFG_LD_PLL_NDIV_FRAC_ALIGN         0
#define LD_LIA_LD_PLL_CONTROL_2_CFG_LD_PLL_NDIV_FRAC_BITS          20
#define LD_LIA_LD_PLL_CONTROL_2_CFG_LD_PLL_NDIV_FRAC_SHIFT         0
#define LD_LIA_LD_PLL_CONTROL_2_CFG_LD_PLL_NDIV_FRAC_DEFAULT       0

/***************************************************************************
 *LD_PLL_CONTROL_3 - LD LC PLL control register 3
 ***************************************************************************/
/* LD_LIA :: LD_PLL_CONTROL_3 :: reserved0 [31:21] */
#define LD_LIA_LD_PLL_CONTROL_3_reserved0_MASK                     0xffe00000
#define LD_LIA_LD_PLL_CONTROL_3_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_CONTROL_3_reserved0_BITS                     11
#define LD_LIA_LD_PLL_CONTROL_3_reserved0_SHIFT                    21

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_VCODIV2 [20:20] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_VCODIV2_MASK            0x00100000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_VCODIV2_ALIGN           0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_VCODIV2_BITS            1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_VCODIV2_SHIFT           20
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_VCODIV2_DEFAULT         0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_AUX_CTRL [19:19] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_AUX_CTRL_MASK           0x00080000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_AUX_CTRL_ALIGN          0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_AUX_CTRL_BITS           1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_AUX_CTRL_SHIFT          19
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_AUX_CTRL_DEFAULT        0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_REFCLK_ENABLE [18:18] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_REFCLK_ENABLE_MASK      0x00040000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_REFCLK_ENABLE_ALIGN     0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_REFCLK_ENABLE_BITS      1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_REFCLK_ENABLE_SHIFT     18
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_REFCLK_ENABLE_DEFAULT   0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_STAT_UPDATE [17:17] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_UPDATE_MASK        0x00020000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_UPDATE_ALIGN       0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_UPDATE_BITS        1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_UPDATE_SHIFT       17
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_UPDATE_DEFAULT     0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_STAT_SELECT [16:14] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_SELECT_MASK        0x0001c000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_SELECT_ALIGN       0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_SELECT_BITS        3
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_SELECT_SHIFT       14
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_SELECT_DEFAULT     0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_STAT_RESET [13:13] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_RESET_MASK         0x00002000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_RESET_ALIGN        0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_RESET_BITS         1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_RESET_SHIFT        13
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_STAT_RESET_DEFAULT      0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_DCO_CTRL_BYPASS_EN [12:12] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_EN_MASK 0x00001000
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_EN_ALIGN 0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_EN_BITS 1
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_EN_SHIFT 12
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_EN_DEFAULT 0

/* LD_LIA :: LD_PLL_CONTROL_3 :: CFG_LD_PLL_DCO_CTRL_BYPASS [11:00] */
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_MASK    0x00000fff
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_ALIGN   0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_BITS    12
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_SHIFT   0
#define LD_LIA_LD_PLL_CONTROL_3_CFG_LD_PLL_DCO_CTRL_BYPASS_DEFAULT 0

/***************************************************************************
 *LD_PLL_STATUS - LD LC PLL status
 ***************************************************************************/
/* LD_LIA :: LD_PLL_STATUS :: reserved0 [31:28] */
#define LD_LIA_LD_PLL_STATUS_reserved0_MASK                        0xf0000000
#define LD_LIA_LD_PLL_STATUS_reserved0_ALIGN                       0
#define LD_LIA_LD_PLL_STATUS_reserved0_BITS                        4
#define LD_LIA_LD_PLL_STATUS_reserved0_SHIFT                       28

/* LD_LIA :: LD_PLL_STATUS :: LD_PLL_STAT_OUT [27:16] */
#define LD_LIA_LD_PLL_STATUS_LD_PLL_STAT_OUT_MASK                  0x0fff0000
#define LD_LIA_LD_PLL_STATUS_LD_PLL_STAT_OUT_ALIGN                 0
#define LD_LIA_LD_PLL_STATUS_LD_PLL_STAT_OUT_BITS                  12
#define LD_LIA_LD_PLL_STATUS_LD_PLL_STAT_OUT_SHIFT                 16
#define LD_LIA_LD_PLL_STATUS_LD_PLL_STAT_OUT_DEFAULT               0

/* LD_LIA :: LD_PLL_STATUS :: reserved1 [15:01] */
#define LD_LIA_LD_PLL_STATUS_reserved1_MASK                        0x0000fffe
#define LD_LIA_LD_PLL_STATUS_reserved1_ALIGN                       0
#define LD_LIA_LD_PLL_STATUS_reserved1_BITS                        15
#define LD_LIA_LD_PLL_STATUS_reserved1_SHIFT                       1

/* LD_LIA :: LD_PLL_STATUS :: LD_PLL_LOCK [00:00] */
#define LD_LIA_LD_PLL_STATUS_LD_PLL_LOCK_MASK                      0x00000001
#define LD_LIA_LD_PLL_STATUS_LD_PLL_LOCK_ALIGN                     0
#define LD_LIA_LD_PLL_STATUS_LD_PLL_LOCK_BITS                      1
#define LD_LIA_LD_PLL_STATUS_LD_PLL_LOCK_SHIFT                     0
#define LD_LIA_LD_PLL_STATUS_LD_PLL_LOCK_DEFAULT                   0

/***************************************************************************
 *LD_PLL_DIVIDER_0 - LD LC PLL divider 0 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_0 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_0_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_0_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_0_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_0_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_MDEL_0 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDEL_0_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDEL_0_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDEL_0_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDEL_0_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDEL_0_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_MDIV_0 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDIV_0_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDIV_0_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDIV_0_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDIV_0_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_MDIV_0_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_LOAD_EN_0 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_LOAD_EN_0_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_LOAD_EN_0_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_LOAD_EN_0_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_LOAD_EN_0_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_LOAD_EN_0_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_HOLD_0 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_HOLD_0_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_HOLD_0_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_HOLD_0_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_HOLD_0_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_HOLD_0_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_ENABLEB_0 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_ENABLEB_0_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_ENABLEB_0_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_ENABLEB_0_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_ENABLEB_0_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_ENABLEB_0_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_0 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_0_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_0_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_0_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_0_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_0 :: CFG_LD_PLL_BYP_EN_0 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_BYP_EN_0_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_BYP_EN_0_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_BYP_EN_0_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_BYP_EN_0_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_0_CFG_LD_PLL_BYP_EN_0_DEFAULT        0

/***************************************************************************
 *LD_PLL_DIVIDER_1 - LD LC PLL divider 1 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_1 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_1_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_1_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_1_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_1_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_MDEL_1 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDEL_1_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDEL_1_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDEL_1_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDEL_1_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDEL_1_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_MDIV_1 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDIV_1_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDIV_1_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDIV_1_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDIV_1_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_MDIV_1_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_LOAD_EN_1 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_LOAD_EN_1_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_LOAD_EN_1_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_LOAD_EN_1_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_LOAD_EN_1_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_LOAD_EN_1_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_HOLD_1 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_HOLD_1_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_HOLD_1_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_HOLD_1_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_HOLD_1_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_HOLD_1_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_ENABLEB_1 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_ENABLEB_1_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_ENABLEB_1_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_ENABLEB_1_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_ENABLEB_1_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_ENABLEB_1_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_1 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_1_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_1_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_1_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_1_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_1 :: CFG_LD_PLL_BYP_EN_1 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_BYP_EN_1_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_BYP_EN_1_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_BYP_EN_1_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_BYP_EN_1_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_1_CFG_LD_PLL_BYP_EN_1_DEFAULT        0

/***************************************************************************
 *LD_PLL_DIVIDER_2 - LD LC PLL divider 2 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_2 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_2_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_2_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_2_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_2_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_MDEL_2 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDEL_2_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDEL_2_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDEL_2_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDEL_2_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDEL_2_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_MDIV_2 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDIV_2_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDIV_2_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDIV_2_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDIV_2_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_MDIV_2_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_LOAD_EN_2 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_LOAD_EN_2_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_LOAD_EN_2_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_LOAD_EN_2_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_LOAD_EN_2_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_LOAD_EN_2_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_HOLD_2 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_HOLD_2_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_HOLD_2_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_HOLD_2_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_HOLD_2_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_HOLD_2_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_ENABLEB_2 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_ENABLEB_2_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_ENABLEB_2_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_ENABLEB_2_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_ENABLEB_2_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_ENABLEB_2_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_2 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_2_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_2_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_2_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_2_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_2 :: CFG_LD_PLL_BYP_EN_2 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_BYP_EN_2_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_BYP_EN_2_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_BYP_EN_2_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_BYP_EN_2_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_2_CFG_LD_PLL_BYP_EN_2_DEFAULT        0

/***************************************************************************
 *LD_PLL_DIVIDER_3 - LD LC PLL divider 3 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_3 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_3_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_3_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_3_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_3_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_MDEL_3 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDEL_3_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDEL_3_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDEL_3_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDEL_3_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDEL_3_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_MDIV_3 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDIV_3_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDIV_3_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDIV_3_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDIV_3_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_MDIV_3_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_LOAD_EN_3 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_LOAD_EN_3_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_LOAD_EN_3_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_LOAD_EN_3_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_LOAD_EN_3_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_LOAD_EN_3_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_HOLD_3 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_HOLD_3_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_HOLD_3_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_HOLD_3_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_HOLD_3_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_HOLD_3_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_ENABLEB_3 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_ENABLEB_3_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_ENABLEB_3_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_ENABLEB_3_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_ENABLEB_3_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_ENABLEB_3_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_3 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_3_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_3_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_3_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_3_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_3 :: CFG_LD_PLL_BYP_EN_3 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_BYP_EN_3_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_BYP_EN_3_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_BYP_EN_3_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_BYP_EN_3_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_3_CFG_LD_PLL_BYP_EN_3_DEFAULT        0

/***************************************************************************
 *LD_PLL_DIVIDER_4 - LD LC PLL divider 4 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_4 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_4_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_4_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_4_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_4_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_MDEL_4 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDEL_4_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDEL_4_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDEL_4_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDEL_4_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDEL_4_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_MDIV_4 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDIV_4_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDIV_4_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDIV_4_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDIV_4_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_MDIV_4_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_LOAD_EN_4 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_LOAD_EN_4_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_LOAD_EN_4_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_LOAD_EN_4_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_LOAD_EN_4_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_LOAD_EN_4_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_HOLD_4 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_HOLD_4_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_HOLD_4_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_HOLD_4_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_HOLD_4_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_HOLD_4_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_ENABLEB_4 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_ENABLEB_4_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_ENABLEB_4_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_ENABLEB_4_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_ENABLEB_4_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_ENABLEB_4_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_4 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_4_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_4_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_4_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_4_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_4 :: CFG_LD_PLL_BYP_EN_4 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_BYP_EN_4_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_BYP_EN_4_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_BYP_EN_4_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_BYP_EN_4_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_4_CFG_LD_PLL_BYP_EN_4_DEFAULT        0

/***************************************************************************
 *LD_PLL_DIVIDER_5 - LD LC PLL divider 5 configuration
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DIVIDER_5 :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DIVIDER_5_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_PLL_DIVIDER_5_reserved0_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_5_reserved0_BITS                     16
#define LD_LIA_LD_PLL_DIVIDER_5_reserved0_SHIFT                    16

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_MDEL_5 [15:13] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDEL_5_MASK             0x0000e000
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDEL_5_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDEL_5_BITS             3
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDEL_5_SHIFT            13
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDEL_5_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_MDIV_5 [12:05] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDIV_5_MASK             0x00001fe0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDIV_5_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDIV_5_BITS             8
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDIV_5_SHIFT            5
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_MDIV_5_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_LOAD_EN_5 [04:04] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_LOAD_EN_5_MASK          0x00000010
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_LOAD_EN_5_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_LOAD_EN_5_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_LOAD_EN_5_SHIFT         4
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_LOAD_EN_5_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_HOLD_5 [03:03] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_HOLD_5_MASK             0x00000008
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_HOLD_5_ALIGN            0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_HOLD_5_BITS             1
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_HOLD_5_SHIFT            3
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_HOLD_5_DEFAULT          0

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_ENABLEB_5 [02:02] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_ENABLEB_5_MASK          0x00000004
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_ENABLEB_5_ALIGN         0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_ENABLEB_5_BITS          1
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_ENABLEB_5_SHIFT         2
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_ENABLEB_5_DEFAULT       0

/* LD_LIA :: LD_PLL_DIVIDER_5 :: reserved1 [01:01] */
#define LD_LIA_LD_PLL_DIVIDER_5_reserved1_MASK                     0x00000002
#define LD_LIA_LD_PLL_DIVIDER_5_reserved1_ALIGN                    0
#define LD_LIA_LD_PLL_DIVIDER_5_reserved1_BITS                     1
#define LD_LIA_LD_PLL_DIVIDER_5_reserved1_SHIFT                    1

/* LD_LIA :: LD_PLL_DIVIDER_5 :: CFG_LD_PLL_BYP_EN_5 [00:00] */
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_BYP_EN_5_MASK           0x00000001
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_BYP_EN_5_ALIGN          0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_BYP_EN_5_BITS           1
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_BYP_EN_5_SHIFT          0
#define LD_LIA_LD_PLL_DIVIDER_5_CFG_LD_PLL_BYP_EN_5_DEFAULT        0

/***************************************************************************
 *LD_CDR_CONFIG_1 - LD CDR configuration register 1
 ***************************************************************************/
/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PI_STEP3_EN [31:31] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PI_STEP3_EN_MASK         0x80000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PI_STEP3_EN_ALIGN        0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PI_STEP3_EN_BITS         1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PI_STEP3_EN_SHIFT        31
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PI_STEP3_EN_DEFAULT      0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_STROBE_PC [30:30] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_STROBE_PC_MASK     0x40000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_STROBE_PC_ALIGN    0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_STROBE_PC_BITS     1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_STROBE_PC_SHIFT    30
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_STROBE_PC_DEFAULT  0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_SAT_SM [29:29] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_SAT_SM_MASK        0x20000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_SAT_SM_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_SAT_SM_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_SAT_SM_SHIFT       29
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_SAT_SM_DEFAULT     1

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_OVERRIDE_PC [28:28] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_OVERRIDE_PC_MASK   0x10000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_OVERRIDE_PC_ALIGN  0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_OVERRIDE_PC_BITS   1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_OVERRIDE_PC_SHIFT  28
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_OVERRIDE_PC_DEFAULT 0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_INC_PC [27:27] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_INC_PC_MASK        0x08000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_INC_PC_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_INC_PC_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_INC_PC_SHIFT       27
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_INC_PC_DEFAULT     0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_FRZ_EN_SM [26:26] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_EN_SM_MASK     0x04000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_EN_SM_ALIGN    0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_EN_SM_BITS     1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_EN_SM_SHIFT    26
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_EN_SM_DEFAULT  0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_FRZ_SM [25:25] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_SM_MASK        0x02000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_SM_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_SM_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_SM_SHIFT       25
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_SM_DEFAULT     0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_FRZ_PC [24:24] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_PC_MASK        0x01000000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_PC_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_PC_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_PC_SHIFT       24
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_FRZ_PC_DEFAULT     0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_DELTA_PC [23:20] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DELTA_PC_MASK      0x00f00000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DELTA_PC_ALIGN     0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DELTA_PC_BITS      4
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DELTA_PC_SHIFT     20
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DELTA_PC_DEFAULT   0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_PHASE_DEC_SM [19:19] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DEC_SM_MASK        0x00080000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DEC_SM_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DEC_SM_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DEC_SM_SHIFT       19
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_PHASE_DEC_SM_DEFAULT     0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_LOCKREF [18:18] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_LOCKREF_MASK             0x00040000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_LOCKREF_ALIGN            0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_LOCKREF_BITS             1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_LOCKREF_SHIFT            18
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_LOCKREF_DEFAULT          0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_INTEG_MODE_SM [17:16] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_MODE_SM_MASK       0x00030000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_MODE_SM_ALIGN      0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_MODE_SM_BITS       2
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_MODE_SM_SHIFT      16
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_MODE_SM_DEFAULT    0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_INTEG_CLR_SM [15:15] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_CLR_SM_MASK        0x00008000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_CLR_SM_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_CLR_SM_BITS        1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_CLR_SM_SHIFT       15
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INTEG_CLR_SM_DEFAULT     0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_GLOOPBACK [14:14] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_GLOOPBACK_MASK           0x00004000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_GLOOPBACK_ALIGN          0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_GLOOPBACK_BITS           1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_GLOOPBACK_SHIFT          14
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_GLOOPBACK_DEFAULT        0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_FREQ_SEL [13:13] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_SEL_MASK            0x00002000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_SEL_ALIGN           0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_SEL_BITS            1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_SEL_SHIFT           13
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_SEL_DEFAULT         1

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_INVCLK [12:12] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INVCLK_MASK              0x00001000
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INVCLK_ALIGN             0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INVCLK_BITS              1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INVCLK_SHIFT             12
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_INVCLK_DEFAULT           0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_FREQ_OVERRIDE_EN_SM [11:11] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_OVERRIDE_EN_SM_MASK 0x00000800
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_OVERRIDE_EN_SM_ALIGN 0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_OVERRIDE_EN_SM_BITS 1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_OVERRIDE_EN_SM_SHIFT 11
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_OVERRIDE_EN_SM_DEFAULT 0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_FREQ_EN [10:10] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_EN_MASK             0x00000400
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_EN_ALIGN            0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_EN_BITS             1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_EN_SHIFT            10
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FREQ_EN_DEFAULT          1

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_FLIP_POLARITY_SM [09:09] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FLIP_POLARITY_SM_MASK    0x00000200
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FLIP_POLARITY_SM_ALIGN   0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FLIP_POLARITY_SM_BITS    1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FLIP_POLARITY_SM_SHIFT   9
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FLIP_POLARITY_SM_DEFAULT 0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_FALLING_EDGE_SM [08:08] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FALLING_EDGE_SM_MASK     0x00000100
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FALLING_EDGE_SM_ALIGN    0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FALLING_EDGE_SM_BITS     1
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FALLING_EDGE_SM_SHIFT    8
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_FALLING_EDGE_SM_DEFAULT  0

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_BWSEL_INTEG_SM [07:04] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_INTEG_SM_MASK      0x000000f0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_INTEG_SM_ALIGN     0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_INTEG_SM_BITS      4
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_INTEG_SM_SHIFT     4
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_INTEG_SM_DEFAULT   2

/* LD_LIA :: LD_CDR_CONFIG_1 :: CFG_LD_CDR_BWSEL_PROP_SM [03:00] */
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_PROP_SM_MASK       0x0000000f
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_PROP_SM_ALIGN      0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_PROP_SM_BITS       4
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_PROP_SM_SHIFT      0
#define LD_LIA_LD_CDR_CONFIG_1_CFG_LD_CDR_BWSEL_PROP_SM_DEFAULT    7

/***************************************************************************
 *LD_CDR_CONFIG_2 - LD CDR configuration register 2
 ***************************************************************************/
/* LD_LIA :: LD_CDR_CONFIG_2 :: reserved0 [31:10] */
#define LD_LIA_LD_CDR_CONFIG_2_reserved0_MASK                      0xfffffc00
#define LD_LIA_LD_CDR_CONFIG_2_reserved0_ALIGN                     0
#define LD_LIA_LD_CDR_CONFIG_2_reserved0_BITS                      22
#define LD_LIA_LD_CDR_CONFIG_2_reserved0_SHIFT                     10

/* LD_LIA :: LD_CDR_CONFIG_2 :: CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM [09:05] */
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM_MASK 0x000003e0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM_ALIGN 0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM_BITS 5
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM_SHIFT 5
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_FREQ_OVERRIDE_VAL_SM_DEFAULT 0

/* LD_LIA :: LD_CDR_CONFIG_2 :: CFG_LD_CDR_TPCTRL [04:02] */
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_TPCTRL_MASK              0x0000001c
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_TPCTRL_ALIGN             0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_TPCTRL_BITS              3
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_TPCTRL_SHIFT             2
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_TPCTRL_DEFAULT           0

/* LD_LIA :: LD_CDR_CONFIG_2 :: CFG_LD_CDR_RESET [01:01] */
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_RESET_MASK               0x00000002
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_RESET_ALIGN              0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_RESET_BITS               1
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_RESET_SHIFT              1
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_RESET_DEFAULT            1

/* LD_LIA :: LD_CDR_CONFIG_2 :: CFG_LD_CDR_PKZR_SWAP_SM [00:00] */
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_PKZR_SWAP_SM_MASK        0x00000001
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_PKZR_SWAP_SM_ALIGN       0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_PKZR_SWAP_SM_BITS        1
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_PKZR_SWAP_SM_SHIFT       0
#define LD_LIA_LD_CDR_CONFIG_2_CFG_LD_CDR_PKZR_SWAP_SM_DEFAULT     0

/***************************************************************************
 *LD_CDR_RX_CTRL_LO - LD CDR lower 32-bits of control register
 ***************************************************************************/
/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_IEQBUF [31:29] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQBUF_MASK            0xe0000000
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQBUF_ALIGN           0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQBUF_BITS            3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQBUF_SHIFT           29
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQBUF_DEFAULT         3

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_ICLKBUF [28:26] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICLKBUF_MASK           0x1c000000
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICLKBUF_ALIGN          0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICLKBUF_BITS           3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICLKBUF_SHIFT          26
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICLKBUF_DEFAULT        3

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_IEQ [25:23] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQ_MASK               0x03800000
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQ_ALIGN              0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQ_BITS               3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQ_SHIFT              23
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQ_DEFAULT            3

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_ICM [22:20] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICM_MASK               0x00700000
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICM_ALIGN              0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICM_BITS               3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICM_SHIFT              20
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ICM_DEFAULT            3

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_ISIGDET [19:17] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ISIGDET_MASK           0x000e0000
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ISIGDET_ALIGN          0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ISIGDET_BITS           3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ISIGDET_SHIFT          17
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_ISIGDET_DEFAULT        3

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: reserved0 [16:12] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved0_MASK                    0x0001f000
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved0_ALIGN                   0
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved0_BITS                    5
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved0_SHIFT                   12

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_CM_LEVEL [11:11] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_CM_LEVEL_MASK          0x00000800
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_CM_LEVEL_ALIGN         0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_CM_LEVEL_BITS          1
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_CM_LEVEL_SHIFT         11
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_CM_LEVEL_DEFAULT       0

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_SIGDET_OVRD [10:09] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_SIGDET_OVRD_MASK       0x00000600
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_SIGDET_OVRD_ALIGN      0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_SIGDET_OVRD_BITS       2
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_SIGDET_OVRD_SHIFT      9
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_SIGDET_OVRD_DEFAULT    0

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_TPORT_EN [08:08] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_TPORT_EN_MASK          0x00000100
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_TPORT_EN_ALIGN         0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_TPORT_EN_BITS          1
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_TPORT_EN_SHIFT         8
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_TPORT_EN_DEFAULT       0

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: reserved1 [07:06] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved1_MASK                    0x000000c0
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved1_ALIGN                   0
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved1_BITS                    2
#define LD_LIA_LD_CDR_RX_CTRL_LO_reserved1_SHIFT                   6

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_VOSCNTL [05:03] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_VOSCNTL_MASK           0x00000038
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_VOSCNTL_ALIGN          0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_VOSCNTL_BITS           3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_VOSCNTL_SHIFT          3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_VOSCNTL_DEFAULT        2

/* LD_LIA :: LD_CDR_RX_CTRL_LO :: CFG_LD_CDR_IEQAMP [02:00] */
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQAMP_MASK            0x00000007
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQAMP_ALIGN           0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQAMP_BITS            3
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQAMP_SHIFT           0
#define LD_LIA_LD_CDR_RX_CTRL_LO_CFG_LD_CDR_IEQAMP_DEFAULT         5

/***************************************************************************
 *LD_CDR_RX_CTRL_HI - LD CDR upper 3-bits of control register
 ***************************************************************************/
/* LD_LIA :: LD_CDR_RX_CTRL_HI :: CFG_LD_RX0_OVERRIDE [31:31] */
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_RX0_OVERRIDE_MASK          0x80000000
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_RX0_OVERRIDE_ALIGN         0
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_RX0_OVERRIDE_BITS          1
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_RX0_OVERRIDE_SHIFT         31
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_RX0_OVERRIDE_DEFAULT       0

/* LD_LIA :: LD_CDR_RX_CTRL_HI :: reserved0 [30:03] */
#define LD_LIA_LD_CDR_RX_CTRL_HI_reserved0_MASK                    0x7ffffff8
#define LD_LIA_LD_CDR_RX_CTRL_HI_reserved0_ALIGN                   0
#define LD_LIA_LD_CDR_RX_CTRL_HI_reserved0_BITS                    28
#define LD_LIA_LD_CDR_RX_CTRL_HI_reserved0_SHIFT                   3

/* LD_LIA :: LD_CDR_RX_CTRL_HI :: CFG_LD_CDR_IPI [02:00] */
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_CDR_IPI_MASK               0x00000007
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_CDR_IPI_ALIGN              0
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_CDR_IPI_BITS               3
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_CDR_IPI_SHIFT              0
#define LD_LIA_LD_CDR_RX_CTRL_HI_CFG_LD_CDR_IPI_DEFAULT            3

/***************************************************************************
 *LD_RX0_DFS_LO - LD CDR lower 32-bits of reciever control bits default register
 ***************************************************************************/
/* LD_LIA :: LD_RX0_DFS_LO :: LD_RX0_DFS_LO [31:00] */
#define LD_LIA_LD_RX0_DFS_LO_LD_RX0_DFS_LO_MASK                    0xffffffff
#define LD_LIA_LD_RX0_DFS_LO_LD_RX0_DFS_LO_ALIGN                   0
#define LD_LIA_LD_RX0_DFS_LO_LD_RX0_DFS_LO_BITS                    32
#define LD_LIA_LD_RX0_DFS_LO_LD_RX0_DFS_LO_SHIFT                   0

/***************************************************************************
 *LD_RX0_DFS_HI - LD CDR upper 16-bits of reciever control bits default register
 ***************************************************************************/
/* LD_LIA :: LD_RX0_DFS_HI :: reserved0 [31:16] */
#define LD_LIA_LD_RX0_DFS_HI_reserved0_MASK                        0xffff0000
#define LD_LIA_LD_RX0_DFS_HI_reserved0_ALIGN                       0
#define LD_LIA_LD_RX0_DFS_HI_reserved0_BITS                        16
#define LD_LIA_LD_RX0_DFS_HI_reserved0_SHIFT                       16

/* LD_LIA :: LD_RX0_DFS_HI :: LD_RX0_DFS_HI [15:00] */
#define LD_LIA_LD_RX0_DFS_HI_LD_RX0_DFS_HI_MASK                    0x0000ffff
#define LD_LIA_LD_RX0_DFS_HI_LD_RX0_DFS_HI_ALIGN                   0
#define LD_LIA_LD_RX0_DFS_HI_LD_RX0_DFS_HI_BITS                    16
#define LD_LIA_LD_RX0_DFS_HI_LD_RX0_DFS_HI_SHIFT                   0

/***************************************************************************
 *LD_CDR_RX_STATUS - LD CDR receiver status
 ***************************************************************************/
/* LD_LIA :: LD_CDR_RX_STATUS :: reserved0 [31:10] */
#define LD_LIA_LD_CDR_RX_STATUS_reserved0_MASK                     0xfffffc00
#define LD_LIA_LD_CDR_RX_STATUS_reserved0_ALIGN                    0
#define LD_LIA_LD_CDR_RX_STATUS_reserved0_BITS                     22
#define LD_LIA_LD_CDR_RX_STATUS_reserved0_SHIFT                    10

/* LD_LIA :: LD_CDR_RX_STATUS :: LD_RX0_BEACON_DET [09:09] */
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_BEACON_DET_MASK             0x00000200
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_BEACON_DET_ALIGN            0
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_BEACON_DET_BITS             1
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_BEACON_DET_SHIFT            9

/* LD_LIA :: LD_CDR_RX_STATUS :: LD_RX0_SIGDET [08:08] */
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_SIGDET_MASK                 0x00000100
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_SIGDET_ALIGN                0
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_SIGDET_BITS                 1
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_SIGDET_SHIFT                8

/* LD_LIA :: LD_CDR_RX_STATUS :: LD_RX0_STS [07:00] */
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_STS_MASK                    0x000000ff
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_STS_ALIGN                   0
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_STS_BITS                    8
#define LD_LIA_LD_CDR_RX_STATUS_LD_RX0_STS_SHIFT                   0

/***************************************************************************
 *LD_CDR_STATUS_1 - LD CDR Status 1
 ***************************************************************************/
/* LD_LIA :: LD_CDR_STATUS_1 :: reserved0 [31:21] */
#define LD_LIA_LD_CDR_STATUS_1_reserved0_MASK                      0xffe00000
#define LD_LIA_LD_CDR_STATUS_1_reserved0_ALIGN                     0
#define LD_LIA_LD_CDR_STATUS_1_reserved0_BITS                      11
#define LD_LIA_LD_CDR_STATUS_1_reserved0_SHIFT                     21

/* LD_LIA :: LD_CDR_STATUS_1 :: LD_CDR_OUT_PHASE [20:16] */
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_OUT_PHASE_MASK               0x001f0000
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_OUT_PHASE_ALIGN              0
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_OUT_PHASE_BITS               5
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_OUT_PHASE_SHIFT              16

/* LD_LIA :: LD_CDR_STATUS_1 :: LD_CDR_INTEG_STATUS [15:00] */
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_INTEG_STATUS_MASK            0x0000ffff
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_INTEG_STATUS_ALIGN           0
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_INTEG_STATUS_BITS            16
#define LD_LIA_LD_CDR_STATUS_1_LD_CDR_INTEG_STATUS_SHIFT           0

/***************************************************************************
 *LD_CDR_STATUS_2 - LD CDR Status 2
 ***************************************************************************/
/* LD_LIA :: LD_CDR_STATUS_2 :: reserved0 [31:26] */
#define LD_LIA_LD_CDR_STATUS_2_reserved0_MASK                      0xfc000000
#define LD_LIA_LD_CDR_STATUS_2_reserved0_ALIGN                     0
#define LD_LIA_LD_CDR_STATUS_2_reserved0_BITS                      6
#define LD_LIA_LD_CDR_STATUS_2_reserved0_SHIFT                     26

/* LD_LIA :: LD_CDR_STATUS_2 :: LD_CDR_TPOUT [25:16] */
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_TPOUT_MASK                   0x03ff0000
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_TPOUT_ALIGN                  0
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_TPOUT_BITS                   10
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_TPOUT_SHIFT                  16

/* LD_LIA :: LD_CDR_STATUS_2 :: LD_CDR_VCO_STATUS [15:00] */
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_VCO_STATUS_MASK              0x0000ffff
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_VCO_STATUS_ALIGN             0
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_VCO_STATUS_BITS              16
#define LD_LIA_LD_CDR_STATUS_2_LD_CDR_VCO_STATUS_SHIFT             0

/***************************************************************************
 *LD_CDR_RX_CONFIG - LD CDR receiver configuration
 ***************************************************************************/
/* LD_LIA :: LD_CDR_RX_CONFIG :: reserved0 [31:03] */
#define LD_LIA_LD_CDR_RX_CONFIG_reserved0_MASK                     0xfffffff8
#define LD_LIA_LD_CDR_RX_CONFIG_reserved0_ALIGN                    0
#define LD_LIA_LD_CDR_RX_CONFIG_reserved0_BITS                     29
#define LD_LIA_LD_CDR_RX_CONFIG_reserved0_SHIFT                    3

/* LD_LIA :: LD_CDR_RX_CONFIG :: CFG_LD_RX_IDDQ [02:02] */
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX_IDDQ_MASK                0x00000004
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX_IDDQ_ALIGN               0
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX_IDDQ_BITS                1
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX_IDDQ_SHIFT               2
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX_IDDQ_DEFAULT             0

/* LD_LIA :: LD_CDR_RX_CONFIG :: CFG_LD_RX0_SIGDET_PWRDN [01:01] */
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_SIGDET_PWRDN_MASK       0x00000002
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_SIGDET_PWRDN_ALIGN      0
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_SIGDET_PWRDN_BITS       1
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_SIGDET_PWRDN_SHIFT      1
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_SIGDET_PWRDN_DEFAULT    1

/* LD_LIA :: LD_CDR_RX_CONFIG :: CFG_LD_RX0_PWRDN [00:00] */
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_PWRDN_MASK              0x00000001
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_PWRDN_ALIGN             0
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_PWRDN_BITS              1
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_PWRDN_SHIFT             0
#define LD_LIA_LD_CDR_RX_CONFIG_CFG_LD_RX0_PWRDN_DEFAULT           1

/***************************************************************************
 *LD_LIA_CTRL_0 - LD LIA control register 0
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_RSSI_SAT_NEG_HIGH [31:24] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_NEG_HIGH_MASK     0xff000000
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_NEG_HIGH_ALIGN    0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_NEG_HIGH_BITS     8
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_NEG_HIGH_SHIFT    24
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_NEG_HIGH_DEFAULT  0

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_RSSI_SAT_POS_HIGH [23:16] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_POS_HIGH_MASK     0x00ff0000
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_POS_HIGH_ALIGN    0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_POS_HIGH_BITS     8
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_POS_HIGH_SHIFT    16
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_SAT_POS_HIGH_DEFAULT  0

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_EXP_CMPCYCLES [15:10] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_EXP_CMPCYCLES_MASK         0x0000fc00
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_EXP_CMPCYCLES_ALIGN        0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_EXP_CMPCYCLES_BITS         6
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_EXP_CMPCYCLES_SHIFT        10
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_EXP_CMPCYCLES_DEFAULT      7

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_N_WAITDAC [09:04] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_N_WAITDAC_MASK             0x000003f0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_N_WAITDAC_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_N_WAITDAC_BITS             6
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_N_WAITDAC_SHIFT            4
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_N_WAITDAC_DEFAULT          2

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_RSSI_RESET [03:03] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_RESET_MASK            0x00000008
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_RESET_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_RESET_BITS            1
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_RESET_SHIFT           3
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_RESET_DEFAULT         1

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_COMMIT [02:02] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_COMMIT_MASK                0x00000004
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_COMMIT_ALIGN               0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_COMMIT_BITS                1
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_COMMIT_SHIFT               2
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_COMMIT_DEFAULT             0

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_RSSI_PDN [01:01] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_PDN_MASK              0x00000002
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_PDN_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_PDN_BITS              1
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_PDN_SHIFT             1
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_RSSI_PDN_DEFAULT           1

/* LD_LIA :: LD_LIA_CTRL_0 :: CFG_LD_LIA_POSPKONLY [00:00] */
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_POSPKONLY_MASK             0x00000001
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_POSPKONLY_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_POSPKONLY_BITS             1
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_POSPKONLY_SHIFT            0
#define LD_LIA_LD_LIA_CTRL_0_CFG_LD_LIA_POSPKONLY_DEFAULT          0

/***************************************************************************
 *LD_LIA_CTRL_1 - LD LIA control register 1
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_1 :: reserved0 [31:29] */
#define LD_LIA_LD_LIA_CTRL_1_reserved0_MASK                        0xe0000000
#define LD_LIA_LD_LIA_CTRL_1_reserved0_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_1_reserved0_BITS                        3
#define LD_LIA_LD_LIA_CTRL_1_reserved0_SHIFT                       29

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_BIASSTG2 [28:25] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG2_MASK              0x1e000000
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG2_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG2_BITS              4
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG2_SHIFT             25
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG2_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_BIASSTG1 [24:21] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG1_MASK              0x01e00000
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG1_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG1_BITS              4
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG1_SHIFT             21
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BIASSTG1_DEFAULT           0

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_BUFBIAS [20:18] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BUFBIAS_MASK               0x001c0000
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BUFBIAS_ALIGN              0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BUFBIAS_BITS               3
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BUFBIAS_SHIFT              18
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_BUFBIAS_DEFAULT            4

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_CMPBIAS [17:15] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_CMPBIAS_MASK               0x00038000
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_CMPBIAS_ALIGN              0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_CMPBIAS_BITS               3
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_CMPBIAS_SHIFT              15
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_CMPBIAS_DEFAULT            4

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_I2V_C [14:12] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_I2V_C_MASK                 0x00007000
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_I2V_C_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_I2V_C_BITS                 3
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_I2V_C_SHIFT                12
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_I2V_C_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_1 :: reserved1 [11:08] */
#define LD_LIA_LD_LIA_CTRL_1_reserved1_MASK                        0x00000f00
#define LD_LIA_LD_LIA_CTRL_1_reserved1_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_1_reserved1_BITS                        4
#define LD_LIA_LD_LIA_CTRL_1_reserved1_SHIFT                       8

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_EN_INVCLK [07:07] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EN_INVCLK_MASK             0x00000080
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EN_INVCLK_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EN_INVCLK_BITS             1
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EN_INVCLK_SHIFT            7
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EN_INVCLK_DEFAULT          0

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_RSSI_CLK_SELECT [06:06] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_RSSI_CLK_SELECT_MASK       0x00000040
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_RSSI_CLK_SELECT_ALIGN      0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_RSSI_CLK_SELECT_BITS       1
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_RSSI_CLK_SELECT_SHIFT      6
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_RSSI_CLK_SELECT_DEFAULT    0

/* LD_LIA :: LD_LIA_CTRL_1 :: CFG_LD_LIA_EXP_ITERCYCLES [05:00] */
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EXP_ITERCYCLES_MASK        0x0000003f
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EXP_ITERCYCLES_ALIGN       0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EXP_ITERCYCLES_BITS        6
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EXP_ITERCYCLES_SHIFT       0
#define LD_LIA_LD_LIA_CTRL_1_CFG_LD_LIA_EXP_ITERCYCLES_DEFAULT     9

/***************************************************************************
 *LD_LIA_CTRL_2 - LD LIA control register 2
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_2 :: reserved0 [31:30] */
#define LD_LIA_LD_LIA_CTRL_2_reserved0_MASK                        0xc0000000
#define LD_LIA_LD_LIA_CTRL_2_reserved0_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_2_reserved0_BITS                        2
#define LD_LIA_LD_LIA_CTRL_2_reserved0_SHIFT                       30

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_DAC_ICTRL [29:26] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_DAC_ICTRL_MASK             0x3c000000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_DAC_ICTRL_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_DAC_ICTRL_BITS             4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_DAC_ICTRL_SHIFT            26
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_DAC_ICTRL_DEFAULT          4

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_EN_10G_2 [25:25] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_10G_2_MASK              0x02000000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_10G_2_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_10G_2_BITS              1
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_10G_2_SHIFT             25
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_10G_2_DEFAULT           0

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_EN_BIAS [24:24] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_BIAS_MASK               0x01000000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_BIAS_ALIGN              0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_BIAS_BITS               1
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_BIAS_SHIFT              24
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_EN_BIAS_DEFAULT            0

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG8 [23:20] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG8_MASK              0x00f00000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG8_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG8_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG8_SHIFT             20
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG8_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG7 [19:16] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG7_MASK              0x000f0000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG7_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG7_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG7_SHIFT             16
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG7_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG6 [15:12] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG6_MASK              0x0000f000
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG6_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG6_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG6_SHIFT             12
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG6_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG5 [11:08] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG5_MASK              0x00000f00
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG5_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG5_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG5_SHIFT             8
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG5_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG4 [07:04] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG4_MASK              0x000000f0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG4_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG4_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG4_SHIFT             4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG4_DEFAULT           3

/* LD_LIA :: LD_LIA_CTRL_2 :: CFG_LD_LIA_BIASSTG3 [03:00] */
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG3_MASK              0x0000000f
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG3_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG3_BITS              4
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG3_SHIFT             0
#define LD_LIA_LD_LIA_CTRL_2_CFG_LD_LIA_BIASSTG3_DEFAULT           3

/***************************************************************************
 *LD_LIA_CTRL_3 - LD LIA control register 3
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_3 :: reserved0 [31:26] */
#define LD_LIA_LD_LIA_CTRL_3_reserved0_MASK                        0xfc000000
#define LD_LIA_LD_LIA_CTRL_3_reserved0_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_3_reserved0_BITS                        6
#define LD_LIA_LD_LIA_CTRL_3_reserved0_SHIFT                       26

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_RSSI_EXP_CMPTHR [25:10] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_RSSI_EXP_CMPTHR_MASK       0x03fffc00
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_RSSI_EXP_CMPTHR_ALIGN      0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_RSSI_EXP_CMPTHR_BITS       16
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_RSSI_EXP_CMPTHR_SHIFT      10
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_RSSI_EXP_CMPTHR_DEFAULT    16

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_EN_RES_DIV [09:09] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_EN_RES_DIV_MASK            0x00000200
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_EN_RES_DIV_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_EN_RES_DIV_BITS            1
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_EN_RES_DIV_SHIFT           9
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_EN_RES_DIV_DEFAULT         0

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_OFST_GM_C [08:06] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_GM_C_MASK             0x000001c0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_GM_C_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_GM_C_BITS             3
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_GM_C_SHIFT            6
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_GM_C_DEFAULT          4

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_PD_OFST_GM [05:05] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_PD_OFST_GM_MASK            0x00000020
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_PD_OFST_GM_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_PD_OFST_GM_BITS            1
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_PD_OFST_GM_SHIFT           5
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_PD_OFST_GM_DEFAULT         0

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_SEL_OFST_INPUT [04:04] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_SEL_OFST_INPUT_MASK        0x00000010
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_SEL_OFST_INPUT_ALIGN       0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_SEL_OFST_INPUT_BITS        1
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_SEL_OFST_INPUT_SHIFT       4
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_SEL_OFST_INPUT_DEFAULT     0

/* LD_LIA :: LD_LIA_CTRL_3 :: CFG_LD_LIA_OFST_ICTRL [03:00] */
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_ICTRL_MASK            0x0000000f
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_ICTRL_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_ICTRL_BITS            4
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_ICTRL_SHIFT           0
#define LD_LIA_LD_LIA_CTRL_3_CFG_LD_LIA_OFST_ICTRL_DEFAULT         4

/***************************************************************************
 *LD_LIA_CTRL_4 - LD LIA control register 4
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_CRES6 [31:29] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES6_MASK                 0xe0000000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES6_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES6_BITS                 3
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES6_SHIFT                29
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES6_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_CRES7 [28:26] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES7_MASK                 0x1c000000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES7_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES7_BITS                 3
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES7_SHIFT                26
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES7_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_CRES8 [25:23] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES8_MASK                 0x03800000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES8_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES8_BITS                 3
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES8_SHIFT                23
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_CRES8_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_10G_1 [22:22] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_10G_1_MASK              0x00400000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_10G_1_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_10G_1_BITS              1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_10G_1_SHIFT             22
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_10G_1_DEFAULT           1

/* LD_LIA :: LD_LIA_CTRL_4 :: reserved0 [21:21] */
#define LD_LIA_LD_LIA_CTRL_4_reserved0_MASK                        0x00200000
#define LD_LIA_LD_LIA_CTRL_4_reserved0_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_4_reserved0_BITS                        1
#define LD_LIA_LD_LIA_CTRL_4_reserved0_SHIFT                       21

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_CMCAP [20:20] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_CMCAP_MASK              0x00100000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_CMCAP_ALIGN             0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_CMCAP_BITS              1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_CMCAP_SHIFT             20
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_CMCAP_DEFAULT           0

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCAP1 [19:19] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP1_MASK            0x00080000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP1_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP1_BITS            1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP1_SHIFT           19
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP1_DEFAULT         0

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCAP2 [18:18] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP2_MASK            0x00040000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP2_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP2_BITS            1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP2_SHIFT           18
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP2_DEFAULT         1

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCAP3 [17:17] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP3_MASK            0x00020000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP3_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP3_BITS            1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP3_SHIFT           17
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP3_DEFAULT         0

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCAP4 [16:16] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP4_MASK            0x00010000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP4_ALIGN           0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP4_BITS            1
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP4_SHIFT           16
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCAP4_DEFAULT         0

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCUNIT1 [15:12] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT1_MASK          0x0000f000
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT1_ALIGN         0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT1_BITS          4
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT1_SHIFT         12
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT1_DEFAULT       8

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCUNIT2 [11:08] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT2_MASK          0x00000f00
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT2_ALIGN         0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT2_BITS          4
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT2_SHIFT         8
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT2_DEFAULT       8

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCUNIT3 [07:04] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT3_MASK          0x000000f0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT3_ALIGN         0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT3_BITS          4
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT3_SHIFT         4
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT3_DEFAULT       8

/* LD_LIA :: LD_LIA_CTRL_4 :: CFG_LD_LIA_EN_NEGCUNIT4 [03:00] */
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT4_MASK          0x0000000f
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT4_ALIGN         0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT4_BITS          4
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT4_SHIFT         0
#define LD_LIA_LD_LIA_CTRL_4_CFG_LD_LIA_EN_NEGCUNIT4_DEFAULT       8

/***************************************************************************
 *LD_LIA_CTRL_5 - LD LIA control register 5
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_5 :: reserved0 [31:26] */
#define LD_LIA_LD_LIA_CTRL_5_reserved0_MASK                        0xfc000000
#define LD_LIA_LD_LIA_CTRL_5_reserved0_ALIGN                       0
#define LD_LIA_LD_LIA_CTRL_5_reserved0_BITS                        6
#define LD_LIA_LD_LIA_CTRL_5_reserved0_SHIFT                       26

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_RSSI_INIPEAKPOS [25:18] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_RSSI_INIPEAKPOS_MASK       0x03fc0000
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_RSSI_INIPEAKPOS_ALIGN      0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_RSSI_INIPEAKPOS_BITS       8
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_RSSI_INIPEAKPOS_SHIFT      18
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_RSSI_INIPEAKPOS_DEFAULT    0

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CCM [17:15] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CCM_MASK                   0x00038000
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CCM_ALIGN                  0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CCM_BITS                   3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CCM_SHIFT                  15
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CCM_DEFAULT                4

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CRES1 [14:12] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES1_MASK                 0x00007000
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES1_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES1_BITS                 3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES1_SHIFT                12
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES1_DEFAULT              7

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CRES2 [11:09] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES2_MASK                 0x00000e00
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES2_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES2_BITS                 3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES2_SHIFT                9
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES2_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CRES3 [08:06] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES3_MASK                 0x000001c0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES3_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES3_BITS                 3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES3_SHIFT                6
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES3_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CRES4 [05:03] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES4_MASK                 0x00000038
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES4_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES4_BITS                 3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES4_SHIFT                3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES4_DEFAULT              4

/* LD_LIA :: LD_LIA_CTRL_5 :: CFG_LD_LIA_CRES5 [02:00] */
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES5_MASK                 0x00000007
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES5_ALIGN                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES5_BITS                 3
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES5_SHIFT                0
#define LD_LIA_LD_LIA_CTRL_5_CFG_LD_LIA_CRES5_DEFAULT              4

/***************************************************************************
 *LD_LIA_CTRL_6 - LD LIA control register 6
 ***************************************************************************/
/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_RSSI_SAT_NEG_LOW [31:24] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_NEG_LOW_MASK      0xff000000
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_NEG_LOW_ALIGN     0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_NEG_LOW_BITS      8
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_NEG_LOW_SHIFT     24
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_NEG_LOW_DEFAULT   0

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_RSSI_SAT_POS_LOW [23:16] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_POS_LOW_MASK      0x00ff0000
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_POS_LOW_ALIGN     0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_POS_LOW_BITS      8
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_POS_LOW_SHIFT     16
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_SAT_POS_LOW_DEFAULT   0

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_RSSI_INIPEAKNEG [15:08] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_INIPEAKNEG_MASK       0x0000ff00
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_INIPEAKNEG_ALIGN      0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_INIPEAKNEG_BITS       8
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_INIPEAKNEG_SHIFT      8
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_RSSI_INIPEAKNEG_DEFAULT    0

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_BIASNEGC1 [07:06] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC1_MASK             0x000000c0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC1_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC1_BITS             2
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC1_SHIFT            6
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC1_DEFAULT          2

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_BIASNEGC2 [05:04] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC2_MASK             0x00000030
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC2_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC2_BITS             2
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC2_SHIFT            4
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC2_DEFAULT          2

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_BIASNEGC3 [03:02] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC3_MASK             0x0000000c
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC3_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC3_BITS             2
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC3_SHIFT            2
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC3_DEFAULT          2

/* LD_LIA :: LD_LIA_CTRL_6 :: CFG_LD_LIA_BIASNEGC4 [01:00] */
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC4_MASK             0x00000003
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC4_ALIGN            0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC4_BITS             2
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC4_SHIFT            0
#define LD_LIA_LD_LIA_CTRL_6_CFG_LD_LIA_BIASNEGC4_DEFAULT          2

/***************************************************************************
 *LD_LIA_RSSI_PEAKPOS - LD LIA Receive Signal Strength Indicator positive peak measurement
 ***************************************************************************/
/* LD_LIA :: LD_LIA_RSSI_PEAKPOS :: LD_LIA_RSSI_DONEPOS [31:31] */
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_DONEPOS_MASK        0x80000000
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_DONEPOS_ALIGN       0
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_DONEPOS_BITS        1
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_DONEPOS_SHIFT       31

/* LD_LIA :: LD_LIA_RSSI_PEAKPOS :: reserved0 [30:08] */
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_reserved0_MASK                  0x7fffff00
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_reserved0_ALIGN                 0
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_reserved0_BITS                  23
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_reserved0_SHIFT                 8

/* LD_LIA :: LD_LIA_RSSI_PEAKPOS :: LD_LIA_RSSI_PEAKPOS [07:00] */
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_PEAKPOS_MASK        0x000000ff
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_PEAKPOS_ALIGN       0
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_PEAKPOS_BITS        8
#define LD_LIA_LD_LIA_RSSI_PEAKPOS_LD_LIA_RSSI_PEAKPOS_SHIFT       0

/***************************************************************************
 *LD_LIA_RSSI_PEAKNEG - LD LIA Receive Signal Strength Indicator negative peak measurement
 ***************************************************************************/
/* LD_LIA :: LD_LIA_RSSI_PEAKNEG :: LD_LIA_RSSI_DONENEG [31:31] */
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_DONENEG_MASK        0x80000000
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_DONENEG_ALIGN       0
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_DONENEG_BITS        1
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_DONENEG_SHIFT       31

/* LD_LIA :: LD_LIA_RSSI_PEAKNEG :: reserved0 [30:08] */
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_reserved0_MASK                  0x7fffff00
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_reserved0_ALIGN                 0
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_reserved0_BITS                  23
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_reserved0_SHIFT                 8

/* LD_LIA :: LD_LIA_RSSI_PEAKNEG :: LD_LIA_RSSI_PEAKNEG [07:00] */
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_PEAKNEG_MASK        0x000000ff
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_PEAKNEG_ALIGN       0
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_PEAKNEG_BITS        8
#define LD_LIA_LD_LIA_RSSI_PEAKNEG_LD_LIA_RSSI_PEAKNEG_SHIFT       0

/***************************************************************************
 *LD_XO_CONTROL - LD LIA Crystal oscillator control
 ***************************************************************************/
/* LD_LIA :: LD_XO_CONTROL :: reserved0 [31:20] */
#define LD_LIA_LD_XO_CONTROL_reserved0_MASK                        0xfff00000
#define LD_LIA_LD_XO_CONTROL_reserved0_ALIGN                       0
#define LD_LIA_LD_XO_CONTROL_reserved0_BITS                        12
#define LD_LIA_LD_XO_CONTROL_reserved0_SHIFT                       20

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_XTAL_PD [19:19] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XTAL_PD_MASK                0x00080000
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XTAL_PD_ALIGN               0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XTAL_PD_BITS                1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XTAL_PD_SHIFT               19
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XTAL_PD_DEFAULT             1

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_XCORE_BIAS [18:15] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XCORE_BIAS_MASK             0x00078000
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XCORE_BIAS_ALIGN            0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XCORE_BIAS_BITS             4
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XCORE_BIAS_SHIFT            15
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_XCORE_BIAS_DEFAULT          2

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_SEL_CUR [14:14] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_SEL_CUR_MASK                0x00004000
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_SEL_CUR_ALIGN               0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_SEL_CUR_BITS                1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_SEL_CUR_SHIFT               14
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_SEL_CUR_DEFAULT             0

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_RESET_B [13:13] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_RESET_B_MASK                0x00002000
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_RESET_B_ALIGN               0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_RESET_B_BITS                1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_RESET_B_SHIFT               13
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_RESET_B_DEFAULT             0

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_HIPASS [12:12] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_HIPASS_MASK                 0x00001000
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_HIPASS_ALIGN                0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_HIPASS_BITS                 1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_HIPASS_SHIFT                12
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_HIPASS_DEFAULT              0

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_ENABLE_CML [11:11] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_ENABLE_CML_MASK             0x00000800
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_ENABLE_CML_ALIGN            0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_ENABLE_CML_BITS             1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_ENABLE_CML_SHIFT            11
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_ENABLE_CML_DEFAULT          0

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_D2C_BIAS [10:08] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_D2C_BIAS_MASK               0x00000700
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_D2C_BIAS_ALIGN              0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_D2C_BIAS_BITS               3
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_D2C_BIAS_SHIFT              8
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_D2C_BIAS_DEFAULT            4

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_CML_SEL_PD [07:06] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_CML_SEL_PD_MASK             0x000000c0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_CML_SEL_PD_ALIGN            0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_CML_SEL_PD_BITS             2
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_CML_SEL_PD_SHIFT            6
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_CML_SEL_PD_DEFAULT          3

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_BYPASS [05:05] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BYPASS_MASK                 0x00000020
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BYPASS_ALIGN                0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BYPASS_BITS                 1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BYPASS_SHIFT                5
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BYPASS_DEFAULT              1

/* LD_LIA :: LD_XO_CONTROL :: reserved1 [04:04] */
#define LD_LIA_LD_XO_CONTROL_reserved1_MASK                        0x00000010
#define LD_LIA_LD_XO_CONTROL_reserved1_ALIGN                       0
#define LD_LIA_LD_XO_CONTROL_reserved1_BITS                        1
#define LD_LIA_LD_XO_CONTROL_reserved1_SHIFT                       4

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_BIAS [03:01] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BIAS_MASK                   0x0000000e
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BIAS_ALIGN                  0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BIAS_BITS                   3
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BIAS_SHIFT                  1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_BIAS_DEFAULT                5

/* LD_LIA :: LD_XO_CONTROL :: CFG_LD_XO_LPG_SW [00:00] */
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_LPG_SW_MASK                 0x00000001
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_LPG_SW_ALIGN                0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_LPG_SW_BITS                 1
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_LPG_SW_SHIFT                0
#define LD_LIA_LD_XO_CONTROL_CFG_LD_XO_LPG_SW_DEFAULT              0

/***************************************************************************
 *LD_SIG_RESERVED - LD reserved register
 ***************************************************************************/
/* LD_LIA :: LD_SIG_RESERVED :: reserved0 [31:16] */
#define LD_LIA_LD_SIG_RESERVED_reserved0_MASK                      0xffff0000
#define LD_LIA_LD_SIG_RESERVED_reserved0_ALIGN                     0
#define LD_LIA_LD_SIG_RESERVED_reserved0_BITS                      16
#define LD_LIA_LD_SIG_RESERVED_reserved0_SHIFT                     16

/* LD_LIA :: LD_SIG_RESERVED :: CFG_LD_SIG_RESERVED [15:00] */
#define LD_LIA_LD_SIG_RESERVED_CFG_LD_SIG_RESERVED_MASK            0x0000ffff
#define LD_LIA_LD_SIG_RESERVED_CFG_LD_SIG_RESERVED_ALIGN           0
#define LD_LIA_LD_SIG_RESERVED_CFG_LD_SIG_RESERVED_BITS            16
#define LD_LIA_LD_SIG_RESERVED_CFG_LD_SIG_RESERVED_SHIFT           0
#define LD_LIA_LD_SIG_RESERVED_CFG_LD_SIG_RESERVED_DEFAULT         0

/***************************************************************************
 *LD_TRIG_RESERVED - LD trigger reserved register
 ***************************************************************************/
/* LD_LIA :: LD_TRIG_RESERVED :: reserved0 [31:16] */
#define LD_LIA_LD_TRIG_RESERVED_reserved0_MASK                     0xffff0000
#define LD_LIA_LD_TRIG_RESERVED_reserved0_ALIGN                    0
#define LD_LIA_LD_TRIG_RESERVED_reserved0_BITS                     16
#define LD_LIA_LD_TRIG_RESERVED_reserved0_SHIFT                    16

/* LD_LIA :: LD_TRIG_RESERVED :: CFG_LD_TRIG_RESERVED [15:00] */
#define LD_LIA_LD_TRIG_RESERVED_CFG_LD_TRIG_RESERVED_MASK          0x0000ffff
#define LD_LIA_LD_TRIG_RESERVED_CFG_LD_TRIG_RESERVED_ALIGN         0
#define LD_LIA_LD_TRIG_RESERVED_CFG_LD_TRIG_RESERVED_BITS          16
#define LD_LIA_LD_TRIG_RESERVED_CFG_LD_TRIG_RESERVED_SHIFT         0
#define LD_LIA_LD_TRIG_RESERVED_CFG_LD_TRIG_RESERVED_DEFAULT       0

/***************************************************************************
 *LD_MPD_CONTROL_6 - LD monitor photo-diode control register 6
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_6 :: reserved0 [31:29] */
#define LD_LIA_LD_MPD_CONTROL_6_reserved0_MASK                     0xe0000000
#define LD_LIA_LD_MPD_CONTROL_6_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_6_reserved0_BITS                     3
#define LD_LIA_LD_MPD_CONTROL_6_reserved0_SHIFT                    29

/* LD_LIA :: LD_MPD_CONTROL_6 :: CFG_LD_MPD_TIACAPNODE2 [28:25] */
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE2_MASK        0x1e000000
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE2_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE2_BITS        4
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE2_SHIFT       25
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE2_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_6 :: CFG_LD_MPD_TIACAPNODE [24:21] */
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE_MASK         0x01e00000
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE_ALIGN        0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE_BITS         4
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE_SHIFT        21
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPNODE_DEFAULT      2

/* LD_LIA :: LD_MPD_CONTROL_6 :: CFG_LD_MPD_TIACAPINP [20:16] */
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPINP_MASK          0x001f0000
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPINP_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPINP_BITS          5
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPINP_SHIFT         16
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIACAPINP_DEFAULT       4

/* LD_LIA :: LD_MPD_CONTROL_6 :: CFG_LD_MPD_TIABIASRES2 [15:08] */
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES2_MASK        0x0000ff00
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES2_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES2_BITS        8
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES2_SHIFT       8
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES2_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_6 :: CFG_LD_MPD_TIABIASRES1 [07:00] */
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES1_MASK        0x000000ff
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES1_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES1_BITS        8
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES1_SHIFT       0
#define LD_LIA_LD_MPD_CONTROL_6_CFG_LD_MPD_TIABIASRES1_DEFAULT     1

/***************************************************************************
 *LD_PLL_DYN_CONTROL_CH_5 - LD PLL dynamic mode controls
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DYN_CONTROL_CH_5 :: reserved0 [31:13] */
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved0_MASK              0xffffe000
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved0_ALIGN             0
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved0_BITS              19
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved0_SHIFT             13

/* LD_LIA :: LD_PLL_DYN_CONTROL_CH_5 :: CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH [12:08] */
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH_MASK 0x00001f00
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH_BITS 5
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH_SHIFT 8
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_LD_PLS_WIDTH_DEFAULT 1

/* LD_LIA :: LD_PLL_DYN_CONTROL_CH_5 :: reserved1 [07:01] */
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved1_MASK              0x000000fe
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved1_ALIGN             0
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved1_BITS              7
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_reserved1_SHIFT             1

/* LD_LIA :: LD_PLL_DYN_CONTROL_CH_5 :: CFG_LD_PLL_DYN_CH_5_EN [00:00] */
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_EN_MASK 0x00000001
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_EN_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_EN_BITS 1
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_EN_SHIFT 0
#define LD_LIA_LD_PLL_DYN_CONTROL_CH_5_CFG_LD_PLL_DYN_CH_5_EN_DEFAULT 0

/***************************************************************************
 *LD_PLL_DYN_CH_5_MDIVS - LD PLL dynamic mode MDIV values
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DYN_CH_5_MDIVS :: reserved0 [31:16] */
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_reserved0_MASK                0xffff0000
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_reserved0_ALIGN               0
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_reserved0_BITS                16
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_reserved0_SHIFT               16

/* LD_LIA :: LD_PLL_DYN_CH_5_MDIVS :: CFG_LD_PLL_DYN_HIGH_MDIV_CH_5 [15:08] */
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_HIGH_MDIV_CH_5_MASK 0x0000ff00
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_HIGH_MDIV_CH_5_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_HIGH_MDIV_CH_5_BITS 8
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_HIGH_MDIV_CH_5_SHIFT 8
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_HIGH_MDIV_CH_5_DEFAULT 0

/* LD_LIA :: LD_PLL_DYN_CH_5_MDIVS :: CFG_LD_PLL_DYN_LOW_MDIV_CH_5 [07:00] */
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_LOW_MDIV_CH_5_MASK 0x000000ff
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_LOW_MDIV_CH_5_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_LOW_MDIV_CH_5_BITS 8
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_LOW_MDIV_CH_5_SHIFT 0
#define LD_LIA_LD_PLL_DYN_CH_5_MDIVS_CFG_LD_PLL_DYN_LOW_MDIV_CH_5_DEFAULT 0

/***************************************************************************
 *LD_PLL_DYN_CH_5_TRANS_TIME - LD PLL dynamic mode transtion times
 ***************************************************************************/
/* LD_LIA :: LD_PLL_DYN_CH_5_TRANS_TIME :: CFG_LD_PLL_DYN_HIGH_WAIT_CH_5 [31:16] */
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_HIGH_WAIT_CH_5_MASK 0xffff0000
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_HIGH_WAIT_CH_5_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_HIGH_WAIT_CH_5_BITS 16
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_HIGH_WAIT_CH_5_SHIFT 16
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_HIGH_WAIT_CH_5_DEFAULT 0

/* LD_LIA :: LD_PLL_DYN_CH_5_TRANS_TIME :: CFG_LD_PLL_DYN_LOW_WAIT_CH_5 [15:00] */
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_LOW_WAIT_CH_5_MASK 0x0000ffff
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_LOW_WAIT_CH_5_ALIGN 0
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_LOW_WAIT_CH_5_BITS 16
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_LOW_WAIT_CH_5_SHIFT 0
#define LD_LIA_LD_PLL_DYN_CH_5_TRANS_TIME_CFG_LD_PLL_DYN_LOW_WAIT_CH_5_DEFAULT 0

/***************************************************************************
 *LD_LIA_PARAM - LD_LIA reserved ECO register
 ***************************************************************************/
/* LD_LIA :: LD_LIA_PARAM :: reserved_for_eco0 [31:00] */
#define LD_LIA_LD_LIA_PARAM_reserved_for_eco0_MASK                 0xffffffff
#define LD_LIA_LD_LIA_PARAM_reserved_for_eco0_ALIGN                0
#define LD_LIA_LD_LIA_PARAM_reserved_for_eco0_BITS                 32
#define LD_LIA_LD_LIA_PARAM_reserved_for_eco0_SHIFT                0
#define LD_LIA_LD_LIA_PARAM_reserved_for_eco0_DEFAULT              0

/***************************************************************************
 *LD_MPD_STATUS - LD_LIA MPD status register
 ***************************************************************************/
/* LD_LIA :: LD_MPD_STATUS :: reserved0 [31:02] */
#define LD_LIA_LD_MPD_STATUS_reserved0_MASK                        0xfffffffc
#define LD_LIA_LD_MPD_STATUS_reserved0_ALIGN                       0
#define LD_LIA_LD_MPD_STATUS_reserved0_BITS                        30
#define LD_LIA_LD_MPD_STATUS_reserved0_SHIFT                       2

/* LD_LIA :: LD_MPD_STATUS :: LD_MPD_ROGUECMPOUT [01:01] */
#define LD_LIA_LD_MPD_STATUS_LD_MPD_ROGUECMPOUT_MASK               0x00000002
#define LD_LIA_LD_MPD_STATUS_LD_MPD_ROGUECMPOUT_ALIGN              0
#define LD_LIA_LD_MPD_STATUS_LD_MPD_ROGUECMPOUT_BITS               1
#define LD_LIA_LD_MPD_STATUS_LD_MPD_ROGUECMPOUT_SHIFT              1

/* LD_LIA :: LD_MPD_STATUS :: LD_MPD_EYECMPOUT [00:00] */
#define LD_LIA_LD_MPD_STATUS_LD_MPD_EYECMPOUT_MASK                 0x00000001
#define LD_LIA_LD_MPD_STATUS_LD_MPD_EYECMPOUT_ALIGN                0
#define LD_LIA_LD_MPD_STATUS_LD_MPD_EYECMPOUT_BITS                 1
#define LD_LIA_LD_MPD_STATUS_LD_MPD_EYECMPOUT_SHIFT                0

/***************************************************************************
 *LD_TRIG_FORCEDELAY - LD_LIA trigger force delay register
 ***************************************************************************/
/* LD_LIA :: LD_TRIG_FORCEDELAY :: reserved0 [31:12] */
#define LD_LIA_LD_TRIG_FORCEDELAY_reserved0_MASK                   0xfffff000
#define LD_LIA_LD_TRIG_FORCEDELAY_reserved0_ALIGN                  0
#define LD_LIA_LD_TRIG_FORCEDELAY_reserved0_BITS                   20
#define LD_LIA_LD_TRIG_FORCEDELAY_reserved0_SHIFT                  12

/* LD_LIA :: LD_TRIG_FORCEDELAY :: CFG_LD_TRIG_FORCEDELAY [11:00] */
#define LD_LIA_LD_TRIG_FORCEDELAY_CFG_LD_TRIG_FORCEDELAY_MASK      0x00000fff
#define LD_LIA_LD_TRIG_FORCEDELAY_CFG_LD_TRIG_FORCEDELAY_ALIGN     0
#define LD_LIA_LD_TRIG_FORCEDELAY_CFG_LD_TRIG_FORCEDELAY_BITS      12
#define LD_LIA_LD_TRIG_FORCEDELAY_CFG_LD_TRIG_FORCEDELAY_SHIFT     0
#define LD_LIA_LD_TRIG_FORCEDELAY_CFG_LD_TRIG_FORCEDELAY_DEFAULT   0

/***************************************************************************
 *LD_MPD_CONTROL_4 - LD monitor photo-diode control register 4
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_4 :: reserved0 [31:30] */
#define LD_LIA_LD_MPD_CONTROL_4_reserved0_MASK                     0xc0000000
#define LD_LIA_LD_MPD_CONTROL_4_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_4_reserved0_BITS                     2
#define LD_LIA_LD_MPD_CONTROL_4_reserved0_SHIFT                    30

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIACONTROLS [29:24] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACONTROLS_MASK        0x3f000000
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACONTROLS_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACONTROLS_BITS        6
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACONTROLS_SHIFT       24
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACONTROLS_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIARLOADCTRL [23:20] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIARLOADCTRL_MASK       0x00f00000
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIARLOADCTRL_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIARLOADCTRL_BITS       4
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIARLOADCTRL_SHIFT      20
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIARLOADCTRL_DEFAULT    0

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIALOAD2CDAC [19:17] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOAD2CDAC_MASK       0x000e0000
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOAD2CDAC_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOAD2CDAC_BITS       3
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOAD2CDAC_SHIFT      17
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOAD2CDAC_DEFAULT    7

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIALOADCDAC [16:14] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOADCDAC_MASK        0x0001c000
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOADCDAC_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOADCDAC_BITS        3
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOADCDAC_SHIFT       14
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIALOADCDAC_DEFAULT     7

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIAINJCTRL [13:11] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINJCTRL_MASK         0x00003800
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINJCTRL_ALIGN        0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINJCTRL_BITS         3
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINJCTRL_SHIFT        11
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINJCTRL_DEFAULT      4

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIAINCDACCTR [10:07] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINCDACCTR_MASK       0x00000780
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINCDACCTR_ALIGN      0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINCDACCTR_BITS       4
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINCDACCTR_SHIFT      7
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIAINCDACCTR_DEFAULT    15

/* LD_LIA :: LD_MPD_CONTROL_4 :: CFG_LD_MPD_TIACFCTRL [06:00] */
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACFCTRL_MASK          0x0000007f
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACFCTRL_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACFCTRL_BITS          7
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACFCTRL_SHIFT         0
#define LD_LIA_LD_MPD_CONTROL_4_CFG_LD_MPD_TIACFCTRL_DEFAULT       0

/***************************************************************************
 *LD_MPD_CONTROL_5 - LD monitor photo-diode control register 5
 ***************************************************************************/
/* LD_LIA :: LD_MPD_CONTROL_5 :: reserved0 [31:30] */
#define LD_LIA_LD_MPD_CONTROL_5_reserved0_MASK                     0xc0000000
#define LD_LIA_LD_MPD_CONTROL_5_reserved0_ALIGN                    0
#define LD_LIA_LD_MPD_CONTROL_5_reserved0_BITS                     2
#define LD_LIA_LD_MPD_CONTROL_5_reserved0_SHIFT                    30

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_TIAR2CTRL [29:28] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIAR2CTRL_MASK          0x30000000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIAR2CTRL_ALIGN         0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIAR2CTRL_BITS          2
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIAR2CTRL_SHIFT         28
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIAR2CTRL_DEFAULT       0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_TIARCTRL [27:25] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIARCTRL_MASK           0x0e000000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIARCTRL_ALIGN          0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIARCTRL_BITS           3
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIARCTRL_SHIFT          25
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_TIARCTRL_DEFAULT        0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_LDO_CFG [24:23] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_LDO_CFG_MASK            0x01800000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_LDO_CFG_ALIGN           0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_LDO_CFG_BITS            2
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_LDO_CFG_SHIFT           23
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_LDO_CFG_DEFAULT         0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_IOFFSET_PWRDN [22:22] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_IOFFSET_PWRDN_MASK      0x00400000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_IOFFSET_PWRDN_ALIGN     0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_IOFFSET_PWRDN_BITS      1
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_IOFFSET_PWRDN_SHIFT     22
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_IOFFSET_PWRDN_DEFAULT   0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_CMP1_AVG_EN [21:21] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP1_AVG_EN_MASK        0x00200000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP1_AVG_EN_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP1_AVG_EN_BITS        1
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP1_AVG_EN_SHIFT       21
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP1_AVG_EN_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_CMP0_AVG_EN [20:20] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP0_AVG_EN_MASK        0x00100000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP0_AVG_EN_ALIGN       0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP0_AVG_EN_BITS        1
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP0_AVG_EN_SHIFT       20
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP0_AVG_EN_DEFAULT     0

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_CMP_AVG_SAMPLES [19:14] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_SAMPLES_MASK    0x000fc000
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_SAMPLES_ALIGN   0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_SAMPLES_BITS    6
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_SAMPLES_SHIFT   14
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_SAMPLES_DEFAULT 4

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_CMP_AVG_DELAY [13:10] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_DELAY_MASK      0x00003c00
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_DELAY_ALIGN     0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_DELAY_BITS      4
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_DELAY_SHIFT     10
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_CMP_AVG_DELAY_DEFAULT   5

/* LD_LIA :: LD_MPD_CONTROL_5 :: CFG_LD_MPD_GAINTIA [09:00] */
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_GAINTIA_MASK            0x000003ff
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_GAINTIA_ALIGN           0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_GAINTIA_BITS            10
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_GAINTIA_SHIFT           0
#define LD_LIA_LD_MPD_CONTROL_5_CFG_LD_MPD_GAINTIA_DEFAULT         128

#endif /* #ifndef LD_LIA_H__ */

/* End of File */
