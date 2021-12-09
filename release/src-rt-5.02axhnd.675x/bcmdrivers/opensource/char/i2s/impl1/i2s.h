/****************************************************************************
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
***************************************************************************/
#ifndef __I2S_H
#define __I2S_H

/************************************************************************** 
 *  i2s raw data driver:
 *  --------------------
 *  Supported Sampling rates:   Supported data widths:
 *    16000Hz                       32-bit
 *    32000Hz                       24-bit
 *    44100Hz                       20-bit
 *    48000Hz                       18-bit
 *    96000Hz                       16-bit
 *    192000Hz
 *    384000Hz
 *
 *  Note: For all data widths less than 32-bit, each sample needs to be 
 *  aligned to the MSB of a 32-bit word
 **************************************************************************/

#define I2S_IOCTL_MAGIC	'S'
#define I2S_SAMPLING_FREQ_SET_IOCTL    _IOWR(I2S_IOCTL_MAGIC, 0, unsigned int)

/* I2S PLL related defines */
/***************************************************************************
 *PLL_ID - PLL PCM ID Register
 ***************************************************************************/
/* I2SPLL :: PLL_ID :: module_id [31:16] */
#define I2SPLL_PLL_ID_module_id_MASK                               0xffff0000
#define I2SPLL_PLL_ID_module_id_ALIGN                              0
#define I2SPLL_PLL_ID_module_id_BITS                               16
#define I2SPLL_PLL_ID_module_id_SHIFT                              16
#define I2SPLL_PLL_ID_module_id_DEFAULT                            0x00000000

/* I2SPLL :: PLL_ID :: HW_revision [15:08] */
#define I2SPLL_PLL_ID_HW_revision_MASK                             0x0000ff00
#define I2SPLL_PLL_ID_HW_revision_ALIGN                            0
#define I2SPLL_PLL_ID_HW_revision_BITS                             8
#define I2SPLL_PLL_ID_HW_revision_SHIFT                            8
#define I2SPLL_PLL_ID_HW_revision_DEFAULT                          0x00000000

/* I2SPLL :: PLL_ID :: PMB_ADDR [07:00] */
#define I2SPLL_PLL_ID_PMB_ADDR_MASK                                0x000000ff
#define I2SPLL_PLL_ID_PMB_ADDR_ALIGN                               0
#define I2SPLL_PLL_ID_PMB_ADDR_BITS                                8
#define I2SPLL_PLL_ID_PMB_ADDR_SHIFT                               0
#define I2SPLL_PLL_ID_PMB_ADDR_DEFAULT                             0x00000000

/***************************************************************************
 *CAPABILITY - PLL PCM Capability Register
 ***************************************************************************/
/* I2SPLL :: CAPABILITY :: reserved0 [31:20] */
#define I2SPLL_CAPABILITY_reserved0_MASK                           0xfff00000
#define I2SPLL_CAPABILITY_reserved0_ALIGN                          0
#define I2SPLL_CAPABILITY_reserved0_BITS                           12
#define I2SPLL_CAPABILITY_reserved0_SHIFT                          20

/* I2SPLL :: CAPABILITY :: type [19:16] */
#define I2SPLL_CAPABILITY_type_MASK                                0x000f0000
#define I2SPLL_CAPABILITY_type_ALIGN                               0
#define I2SPLL_CAPABILITY_type_BITS                                4
#define I2SPLL_CAPABILITY_type_SHIFT                               16
#define I2SPLL_CAPABILITY_type_DEFAULT                             0x00000003

/* I2SPLL :: CAPABILITY :: reserved1 [15:00] */
#define I2SPLL_CAPABILITY_reserved1_MASK                           0x0000ffff
#define I2SPLL_CAPABILITY_reserved1_ALIGN                          0
#define I2SPLL_CAPABILITY_reserved1_BITS                           16
#define I2SPLL_CAPABILITY_reserved1_SHIFT                          0

/***************************************************************************
 *PLL_RESETS - PLL Reset and Powerdown Control Register
 ***************************************************************************/
/* I2SPLL :: PLL_RESETS :: LDO_CTLR [31:16] */
#define I2SPLL_PLL_RESETS_LDO_CTLR_MASK                            0xffff0000
#define I2SPLL_PLL_RESETS_LDO_CTLR_ALIGN                           0
#define I2SPLL_PLL_RESETS_LDO_CTLR_BITS                            16
#define I2SPLL_PLL_RESETS_LDO_CTLR_SHIFT                           16
#define I2SPLL_PLL_RESETS_LDO_CTLR_DEFAULT                         0x00005005

/* I2SPLL :: PLL_RESETS :: PLL_MISC_CTRL [15:06] */
#define I2SPLL_PLL_RESETS_PLL_MISC_CTRL_MASK                       0x0000ffc0
#define I2SPLL_PLL_RESETS_PLL_MISC_CTRL_ALIGN                      0
#define I2SPLL_PLL_RESETS_PLL_MISC_CTRL_BITS                       10
#define I2SPLL_PLL_RESETS_PLL_MISC_CTRL_SHIFT                      6
#define I2SPLL_PLL_RESETS_PLL_MISC_CTRL_DEFAULT                    0x00000000

/* I2SPLL :: PLL_RESETS :: PWR_ON_BG [05:05] */
#define I2SPLL_PLL_RESETS_PWR_ON_BG_MASK                           0x00000020
#define I2SPLL_PLL_RESETS_PWR_ON_BG_ALIGN                          0
#define I2SPLL_PLL_RESETS_PWR_ON_BG_BITS                           1
#define I2SPLL_PLL_RESETS_PWR_ON_BG_SHIFT                          5
#define I2SPLL_PLL_RESETS_PWR_ON_BG_DEFAULT                        0x00000000

/* I2SPLL :: PLL_RESETS :: LDO_PWR_ON [04:04] */
#define I2SPLL_PLL_RESETS_LDO_PWR_ON_MASK                          0x00000010
#define I2SPLL_PLL_RESETS_LDO_PWR_ON_ALIGN                         0
#define I2SPLL_PLL_RESETS_LDO_PWR_ON_BITS                          1
#define I2SPLL_PLL_RESETS_LDO_PWR_ON_SHIFT                         4
#define I2SPLL_PLL_RESETS_LDO_PWR_ON_DEFAULT                       0x00000000

/* I2SPLL :: PLL_RESETS :: MASTER_RESET [03:03] */
#define I2SPLL_PLL_RESETS_MASTER_RESET_MASK                        0x00000008
#define I2SPLL_PLL_RESETS_MASTER_RESET_ALIGN                       0
#define I2SPLL_PLL_RESETS_MASTER_RESET_BITS                        1
#define I2SPLL_PLL_RESETS_MASTER_RESET_SHIFT                       3
#define I2SPLL_PLL_RESETS_MASTER_RESET_DEFAULT                     0x00000001

/* I2SPLL :: PLL_RESETS :: PWR_ON [02:02] */
#define I2SPLL_PLL_RESETS_PWR_ON_MASK                              0x00000004
#define I2SPLL_PLL_RESETS_PWR_ON_ALIGN                             0
#define I2SPLL_PLL_RESETS_PWR_ON_BITS                              1
#define I2SPLL_PLL_RESETS_PWR_ON_SHIFT                             2
#define I2SPLL_PLL_RESETS_PWR_ON_DEFAULT                           0x00000000

/* I2SPLL :: PLL_RESETS :: POST_RESETB [01:01] */
#define I2SPLL_PLL_RESETS_POST_RESETB_MASK                         0x00000002
#define I2SPLL_PLL_RESETS_POST_RESETB_ALIGN                        0
#define I2SPLL_PLL_RESETS_POST_RESETB_BITS                         1
#define I2SPLL_PLL_RESETS_POST_RESETB_SHIFT                        1
#define I2SPLL_PLL_RESETS_POST_RESETB_DEFAULT                      0x00000000

/* I2SPLL :: PLL_RESETS :: RESETB [00:00] */
#define I2SPLL_PLL_RESETS_RESETB_MASK                              0x00000001
#define I2SPLL_PLL_RESETS_RESETB_ALIGN                             0
#define I2SPLL_PLL_RESETS_RESETB_BITS                              1
#define I2SPLL_PLL_RESETS_RESETB_SHIFT                             0
#define I2SPLL_PLL_RESETS_RESETB_DEFAULT                           0x00000000

/***************************************************************************
 *PLL_CTRL - PLL PCM Control Register
 ***************************************************************************/
/* I2SPLL :: PLL_CTRL :: VCO_RANGE [31:30] */
#define I2SPLL_PLL_CTRL_VCO_RANGE_MASK                             0xc0000000
#define I2SPLL_PLL_CTRL_VCO_RANGE_ALIGN                            0
#define I2SPLL_PLL_CTRL_VCO_RANGE_BITS                             2
#define I2SPLL_PLL_CTRL_VCO_RANGE_SHIFT                            30
#define I2SPLL_PLL_CTRL_VCO_RANGE_DEFAULT                          0x00000002

/* I2SPLL :: PLL_CTRL :: reserved0 [29:29] */
#define I2SPLL_PLL_CTRL_reserved0_MASK                             0x20000000
#define I2SPLL_PLL_CTRL_reserved0_ALIGN                            0
#define I2SPLL_PLL_CTRL_reserved0_BITS                             1
#define I2SPLL_PLL_CTRL_reserved0_SHIFT                            29

/* I2SPLL :: PLL_CTRL :: NDIV_RELOCK [28:28] */
#define I2SPLL_PLL_CTRL_NDIV_RELOCK_MASK                           0x10000000
#define I2SPLL_PLL_CTRL_NDIV_RELOCK_ALIGN                          0
#define I2SPLL_PLL_CTRL_NDIV_RELOCK_BITS                           1
#define I2SPLL_PLL_CTRL_NDIV_RELOCK_SHIFT                          28
#define I2SPLL_PLL_CTRL_NDIV_RELOCK_DEFAULT                        0x00000000

/* I2SPLL :: PLL_CTRL :: FAST_PHASE_LOCK [27:27] */
#define I2SPLL_PLL_CTRL_FAST_PHASE_LOCK_MASK                       0x08000000
#define I2SPLL_PLL_CTRL_FAST_PHASE_LOCK_ALIGN                      0
#define I2SPLL_PLL_CTRL_FAST_PHASE_LOCK_BITS                       1
#define I2SPLL_PLL_CTRL_FAST_PHASE_LOCK_SHIFT                      27
#define I2SPLL_PLL_CTRL_FAST_PHASE_LOCK_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CTRL :: VCOFBDIV2 [26:26] */
#define I2SPLL_PLL_CTRL_VCOFBDIV2_MASK                             0x04000000
#define I2SPLL_PLL_CTRL_VCOFBDIV2_ALIGN                            0
#define I2SPLL_PLL_CTRL_VCOFBDIV2_BITS                             1
#define I2SPLL_PLL_CTRL_VCOFBDIV2_SHIFT                            26
#define I2SPLL_PLL_CTRL_VCOFBDIV2_DEFAULT                          0x00000001

/* I2SPLL :: PLL_CTRL :: POST_RESETB_SELECT [25:24] */
#define I2SPLL_PLL_CTRL_POST_RESETB_SELECT_MASK                    0x03000000
#define I2SPLL_PLL_CTRL_POST_RESETB_SELECT_ALIGN                   0
#define I2SPLL_PLL_CTRL_POST_RESETB_SELECT_BITS                    2
#define I2SPLL_PLL_CTRL_POST_RESETB_SELECT_SHIFT                   24
#define I2SPLL_PLL_CTRL_POST_RESETB_SELECT_DEFAULT                 0x00000002

/* I2SPLL :: PLL_CTRL :: PWM_RATE [23:22] */
#define I2SPLL_PLL_CTRL_PWM_RATE_MASK                              0x00c00000
#define I2SPLL_PLL_CTRL_PWM_RATE_ALIGN                             0
#define I2SPLL_PLL_CTRL_PWM_RATE_BITS                              2
#define I2SPLL_PLL_CTRL_PWM_RATE_SHIFT                             22
#define I2SPLL_PLL_CTRL_PWM_RATE_DEFAULT                           0x00000000

/* I2SPLL :: PLL_CTRL :: STAT_MODE [21:20] */
#define I2SPLL_PLL_CTRL_STAT_MODE_MASK                             0x00300000
#define I2SPLL_PLL_CTRL_STAT_MODE_ALIGN                            0
#define I2SPLL_PLL_CTRL_STAT_MODE_BITS                             2
#define I2SPLL_PLL_CTRL_STAT_MODE_SHIFT                            20
#define I2SPLL_PLL_CTRL_STAT_MODE_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CTRL :: reserved1 [19:18] */
#define I2SPLL_PLL_CTRL_reserved1_MASK                             0x000c0000
#define I2SPLL_PLL_CTRL_reserved1_ALIGN                            0
#define I2SPLL_PLL_CTRL_reserved1_BITS                             2
#define I2SPLL_PLL_CTRL_reserved1_SHIFT                            18

/* I2SPLL :: PLL_CTRL :: STAT_UPDATE [17:17] */
#define I2SPLL_PLL_CTRL_STAT_UPDATE_MASK                           0x00020000
#define I2SPLL_PLL_CTRL_STAT_UPDATE_ALIGN                          0
#define I2SPLL_PLL_CTRL_STAT_UPDATE_BITS                           1
#define I2SPLL_PLL_CTRL_STAT_UPDATE_SHIFT                          17
#define I2SPLL_PLL_CTRL_STAT_UPDATE_DEFAULT                        0x00000000

/* I2SPLL :: PLL_CTRL :: STAT_SELECT [16:14] */
#define I2SPLL_PLL_CTRL_STAT_SELECT_MASK                           0x0001c000
#define I2SPLL_PLL_CTRL_STAT_SELECT_ALIGN                          0
#define I2SPLL_PLL_CTRL_STAT_SELECT_BITS                           3
#define I2SPLL_PLL_CTRL_STAT_SELECT_SHIFT                          14
#define I2SPLL_PLL_CTRL_STAT_SELECT_DEFAULT                        0x00000000

/* I2SPLL :: PLL_CTRL :: STAT_RESET [13:13] */
#define I2SPLL_PLL_CTRL_STAT_RESET_MASK                            0x00002000
#define I2SPLL_PLL_CTRL_STAT_RESET_ALIGN                           0
#define I2SPLL_PLL_CTRL_STAT_RESET_BITS                            1
#define I2SPLL_PLL_CTRL_STAT_RESET_SHIFT                           13
#define I2SPLL_PLL_CTRL_STAT_RESET_DEFAULT                         0x00000000

/* I2SPLL :: PLL_CTRL :: DCO_CTRL_BYPASS_ENABLE [12:12] */
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ENABLE_MASK                0x00001000
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ENABLE_ALIGN               0
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ENABLE_BITS                1
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ENABLE_SHIFT               12
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ENABLE_DEFAULT             0x00000000

/* I2SPLL :: PLL_CTRL :: DCO_CTRL_BYPASS [11:00] */
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_MASK                       0x00000fff
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_ALIGN                      0
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_BITS                       12
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_SHIFT                      0
#define I2SPLL_PLL_CTRL_DCO_CTRL_BYPASS_DEFAULT                    0x00000000

/***************************************************************************
 *PLL_PHASE - PLL Phase Control Register
 ***************************************************************************/
/* I2SPLL :: PLL_PHASE :: reserved0 [31:02] */
#define I2SPLL_PLL_PHASE_reserved0_MASK                            0xfffffffc
#define I2SPLL_PLL_PHASE_reserved0_ALIGN                           0
#define I2SPLL_PLL_PHASE_reserved0_BITS                            30
#define I2SPLL_PLL_PHASE_reserved0_SHIFT                           2

/* I2SPLL :: PLL_PHASE :: LDO [01:00] */
#define I2SPLL_PLL_PHASE_LDO_MASK                                  0x00000003
#define I2SPLL_PLL_PHASE_LDO_ALIGN                                 0
#define I2SPLL_PLL_PHASE_LDO_BITS                                  2
#define I2SPLL_PLL_PHASE_LDO_SHIFT                                 0
#define I2SPLL_PLL_PHASE_LDO_DEFAULT                               0x00000001

/***************************************************************************
 *PLL_NDIV - PLL  Integer and fractional feedback divider"
 ***************************************************************************/
/* I2SPLL :: PLL_NDIV :: reserved0 [31:30] */
#define I2SPLL_PLL_NDIV_reserved0_MASK                             0xc0000000
#define I2SPLL_PLL_NDIV_reserved0_ALIGN                            0
#define I2SPLL_PLL_NDIV_reserved0_BITS                             2
#define I2SPLL_PLL_NDIV_reserved0_SHIFT                            30

/* I2SPLL :: PLL_NDIV :: NDIV_FRAC [29:10] */
#define I2SPLL_PLL_NDIV_NDIV_FRAC_MASK                             0x3ffffc00
#define I2SPLL_PLL_NDIV_NDIV_FRAC_ALIGN                            0
#define I2SPLL_PLL_NDIV_NDIV_FRAC_BITS                             20
#define I2SPLL_PLL_NDIV_NDIV_FRAC_SHIFT                            10
#define I2SPLL_PLL_NDIV_NDIV_FRAC_DEFAULT                          0x00000000

/* I2SPLL :: PLL_NDIV :: NDIV_INT [09:00] */
#define I2SPLL_PLL_NDIV_NDIV_INT_MASK                              0x000003ff
#define I2SPLL_PLL_NDIV_NDIV_INT_ALIGN                             0
#define I2SPLL_PLL_NDIV_NDIV_INT_BITS                              10
#define I2SPLL_PLL_NDIV_NDIV_INT_SHIFT                             0
#define I2SPLL_PLL_NDIV_NDIV_INT_DEFAULT                           0x00000060

/***************************************************************************
 *PLL_PDIV - PLL  Integer reference clock pre-divider"
 ***************************************************************************/
/* I2SPLL :: PLL_PDIV :: NDIV_PDIV_OVERRIDE [31:31] */
#define I2SPLL_PLL_PDIV_NDIV_PDIV_OVERRIDE_MASK                    0x80000000
#define I2SPLL_PLL_PDIV_NDIV_PDIV_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_PDIV_NDIV_PDIV_OVERRIDE_BITS                    1
#define I2SPLL_PLL_PDIV_NDIV_PDIV_OVERRIDE_SHIFT                   31
#define I2SPLL_PLL_PDIV_NDIV_PDIV_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_PDIV :: reserved0 [30:03] */
#define I2SPLL_PLL_PDIV_reserved0_MASK                             0x7ffffff8
#define I2SPLL_PLL_PDIV_reserved0_ALIGN                            0
#define I2SPLL_PLL_PDIV_reserved0_BITS                             28
#define I2SPLL_PLL_PDIV_reserved0_SHIFT                            3

/* I2SPLL :: PLL_PDIV :: PDIV [02:00] */
#define I2SPLL_PLL_PDIV_PDIV_MASK                                  0x00000007
#define I2SPLL_PLL_PDIV_PDIV_ALIGN                                 0
#define I2SPLL_PLL_PDIV_PDIV_BITS                                  3
#define I2SPLL_PLL_PDIV_PDIV_SHIFT                                 0
#define I2SPLL_PLL_PDIV_PDIV_DEFAULT                               0x00000002

/***************************************************************************
 *SS_LOOP0 - Spread spectrum loop control 0
 ***************************************************************************/
/* I2SPLL :: SS_LOOP0 :: SSC_STEP [31:16] */
#define I2SPLL_SS_LOOP0_SSC_STEP_MASK                              0xffff0000
#define I2SPLL_SS_LOOP0_SSC_STEP_ALIGN                             0
#define I2SPLL_SS_LOOP0_SSC_STEP_BITS                              16
#define I2SPLL_SS_LOOP0_SSC_STEP_SHIFT                             16
#define I2SPLL_SS_LOOP0_SSC_STEP_DEFAULT                           0x00000000

/* I2SPLL :: SS_LOOP0 :: SS_KP [15:12] */
#define I2SPLL_SS_LOOP0_SS_KP_MASK                                 0x0000f000
#define I2SPLL_SS_LOOP0_SS_KP_ALIGN                                0
#define I2SPLL_SS_LOOP0_SS_KP_BITS                                 4
#define I2SPLL_SS_LOOP0_SS_KP_SHIFT                                12
#define I2SPLL_SS_LOOP0_SS_KP_DEFAULT                              0x00000003

/* I2SPLL :: SS_LOOP0 :: reserved0 [11:11] */
#define I2SPLL_SS_LOOP0_reserved0_MASK                             0x00000800
#define I2SPLL_SS_LOOP0_reserved0_ALIGN                            0
#define I2SPLL_SS_LOOP0_reserved0_BITS                             1
#define I2SPLL_SS_LOOP0_reserved0_SHIFT                            11

/* I2SPLL :: SS_LOOP0 :: SS_KI [10:08] */
#define I2SPLL_SS_LOOP0_SS_KI_MASK                                 0x00000700
#define I2SPLL_SS_LOOP0_SS_KI_ALIGN                                0
#define I2SPLL_SS_LOOP0_SS_KI_BITS                                 3
#define I2SPLL_SS_LOOP0_SS_KI_SHIFT                                8
#define I2SPLL_SS_LOOP0_SS_KI_DEFAULT                              0x00000002

/* I2SPLL :: SS_LOOP0 :: reserved1 [07:07] */
#define I2SPLL_SS_LOOP0_reserved1_MASK                             0x00000080
#define I2SPLL_SS_LOOP0_reserved1_ALIGN                            0
#define I2SPLL_SS_LOOP0_reserved1_BITS                             1
#define I2SPLL_SS_LOOP0_reserved1_SHIFT                            7

/* I2SPLL :: SS_LOOP0 :: SS_KA [06:04] */
#define I2SPLL_SS_LOOP0_SS_KA_MASK                                 0x00000070
#define I2SPLL_SS_LOOP0_SS_KA_ALIGN                                0
#define I2SPLL_SS_LOOP0_SS_KA_BITS                                 3
#define I2SPLL_SS_LOOP0_SS_KA_SHIFT                                4
#define I2SPLL_SS_LOOP0_SS_KA_DEFAULT                              0x00000000

/* I2SPLL :: SS_LOOP0 :: reserved2 [03:00] */
#define I2SPLL_SS_LOOP0_reserved2_MASK                             0x0000000f
#define I2SPLL_SS_LOOP0_reserved2_ALIGN                            0
#define I2SPLL_SS_LOOP0_reserved2_BITS                             4
#define I2SPLL_SS_LOOP0_reserved2_SHIFT                            0

/***************************************************************************
 *SS_LOOP1 - Spread spectrum loop control 1
 ***************************************************************************/
/* I2SPLL :: SS_LOOP1 :: SSC_MODE [31:31] */
#define I2SPLL_SS_LOOP1_SSC_MODE_MASK                              0x80000000
#define I2SPLL_SS_LOOP1_SSC_MODE_ALIGN                             0
#define I2SPLL_SS_LOOP1_SSC_MODE_BITS                              1
#define I2SPLL_SS_LOOP1_SSC_MODE_SHIFT                             31
#define I2SPLL_SS_LOOP1_SSC_MODE_DEFAULT                           0x00000000

/* I2SPLL :: SS_LOOP1 :: reserved0 [30:22] */
#define I2SPLL_SS_LOOP1_reserved0_MASK                             0x7fc00000
#define I2SPLL_SS_LOOP1_reserved0_ALIGN                            0
#define I2SPLL_SS_LOOP1_reserved0_BITS                             9
#define I2SPLL_SS_LOOP1_reserved0_SHIFT                            22

/* I2SPLL :: SS_LOOP1 :: SSC_LIMIT [21:00] */
#define I2SPLL_SS_LOOP1_SSC_LIMIT_MASK                             0x003fffff
#define I2SPLL_SS_LOOP1_SSC_LIMIT_ALIGN                            0
#define I2SPLL_SS_LOOP1_SSC_LIMIT_BITS                             22
#define I2SPLL_SS_LOOP1_SSC_LIMIT_SHIFT                            0
#define I2SPLL_SS_LOOP1_SSC_LIMIT_DEFAULT                          0x00000000

/***************************************************************************
 *PLL_CH01_CFG - Channel configuration for channels 0 and 1
 ***************************************************************************/
/* I2SPLL :: PLL_CH01_CFG :: MDIV1_OVERRIDE [31:31] */
#define I2SPLL_PLL_CH01_CFG_MDIV1_OVERRIDE_MASK                    0x80000000
#define I2SPLL_PLL_CH01_CFG_MDIV1_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH01_CFG_MDIV1_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH01_CFG_MDIV1_OVERRIDE_SHIFT                   31
#define I2SPLL_PLL_CH01_CFG_MDIV1_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH01_CFG :: reserved0 [30:28] */
#define I2SPLL_PLL_CH01_CFG_reserved0_MASK                         0x70000000
#define I2SPLL_PLL_CH01_CFG_reserved0_ALIGN                        0
#define I2SPLL_PLL_CH01_CFG_reserved0_BITS                         3
#define I2SPLL_PLL_CH01_CFG_reserved0_SHIFT                        28

/* I2SPLL :: PLL_CH01_CFG :: MDEL1 [27:27] */
#define I2SPLL_PLL_CH01_CFG_MDEL1_MASK                             0x08000000
#define I2SPLL_PLL_CH01_CFG_MDEL1_ALIGN                            0
#define I2SPLL_PLL_CH01_CFG_MDEL1_BITS                             1
#define I2SPLL_PLL_CH01_CFG_MDEL1_SHIFT                            27
#define I2SPLL_PLL_CH01_CFG_MDEL1_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH01_CFG :: LOAD_EN_CH1 [26:26] */
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH1_MASK                       0x04000000
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH1_ALIGN                      0
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH1_BITS                       1
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH1_SHIFT                      26
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH1_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH01_CFG :: HOLD_CH1 [25:25] */
#define I2SPLL_PLL_CH01_CFG_HOLD_CH1_MASK                          0x02000000
#define I2SPLL_PLL_CH01_CFG_HOLD_CH1_ALIGN                         0
#define I2SPLL_PLL_CH01_CFG_HOLD_CH1_BITS                          1
#define I2SPLL_PLL_CH01_CFG_HOLD_CH1_SHIFT                         25
#define I2SPLL_PLL_CH01_CFG_HOLD_CH1_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH01_CFG :: ENABLEB_CH1 [24:24] */
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH1_MASK                       0x01000000
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH1_ALIGN                      0
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH1_BITS                       1
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH1_SHIFT                      24
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH1_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH01_CFG :: MDIV1 [23:16] */
#define I2SPLL_PLL_CH01_CFG_MDIV1_MASK                             0x00ff0000
#define I2SPLL_PLL_CH01_CFG_MDIV1_ALIGN                            0
#define I2SPLL_PLL_CH01_CFG_MDIV1_BITS                             8
#define I2SPLL_PLL_CH01_CFG_MDIV1_SHIFT                            16
#define I2SPLL_PLL_CH01_CFG_MDIV1_DEFAULT                          0x0000000c

/* I2SPLL :: PLL_CH01_CFG :: MDIV0_OVERRIDE [15:15] */
#define I2SPLL_PLL_CH01_CFG_MDIV0_OVERRIDE_MASK                    0x00008000
#define I2SPLL_PLL_CH01_CFG_MDIV0_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH01_CFG_MDIV0_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH01_CFG_MDIV0_OVERRIDE_SHIFT                   15
#define I2SPLL_PLL_CH01_CFG_MDIV0_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH01_CFG :: reserved1 [14:12] */
#define I2SPLL_PLL_CH01_CFG_reserved1_MASK                         0x00007000
#define I2SPLL_PLL_CH01_CFG_reserved1_ALIGN                        0
#define I2SPLL_PLL_CH01_CFG_reserved1_BITS                         3
#define I2SPLL_PLL_CH01_CFG_reserved1_SHIFT                        12

/* I2SPLL :: PLL_CH01_CFG :: MDEL0 [11:11] */
#define I2SPLL_PLL_CH01_CFG_MDEL0_MASK                             0x00000800
#define I2SPLL_PLL_CH01_CFG_MDEL0_ALIGN                            0
#define I2SPLL_PLL_CH01_CFG_MDEL0_BITS                             1
#define I2SPLL_PLL_CH01_CFG_MDEL0_SHIFT                            11
#define I2SPLL_PLL_CH01_CFG_MDEL0_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH01_CFG :: LOAD_EN_CH0 [10:10] */
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH0_MASK                       0x00000400
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH0_ALIGN                      0
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH0_BITS                       1
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH0_SHIFT                      10
#define I2SPLL_PLL_CH01_CFG_LOAD_EN_CH0_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH01_CFG :: HOLD_CH0 [09:09] */
#define I2SPLL_PLL_CH01_CFG_HOLD_CH0_MASK                          0x00000200
#define I2SPLL_PLL_CH01_CFG_HOLD_CH0_ALIGN                         0
#define I2SPLL_PLL_CH01_CFG_HOLD_CH0_BITS                          1
#define I2SPLL_PLL_CH01_CFG_HOLD_CH0_SHIFT                         9
#define I2SPLL_PLL_CH01_CFG_HOLD_CH0_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH01_CFG :: ENABLEB_CH0 [08:08] */
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH0_MASK                       0x00000100
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH0_ALIGN                      0
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH0_BITS                       1
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH0_SHIFT                      8
#define I2SPLL_PLL_CH01_CFG_ENABLEB_CH0_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH01_CFG :: MDIV0 [07:00] */
#define I2SPLL_PLL_CH01_CFG_MDIV0_MASK                             0x000000ff
#define I2SPLL_PLL_CH01_CFG_MDIV0_ALIGN                            0
#define I2SPLL_PLL_CH01_CFG_MDIV0_BITS                             8
#define I2SPLL_PLL_CH01_CFG_MDIV0_SHIFT                            0
#define I2SPLL_PLL_CH01_CFG_MDIV0_DEFAULT                          0x00000004

/***************************************************************************
 *PLL_CH23_CFG - Channel configuration for channels 2 and 3
 ***************************************************************************/
/* I2SPLL :: PLL_CH23_CFG :: MDIV3_OVERRIDE [31:31] */
#define I2SPLL_PLL_CH23_CFG_MDIV3_OVERRIDE_MASK                    0x80000000
#define I2SPLL_PLL_CH23_CFG_MDIV3_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH23_CFG_MDIV3_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH23_CFG_MDIV3_OVERRIDE_SHIFT                   31
#define I2SPLL_PLL_CH23_CFG_MDIV3_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH23_CFG :: reserved0 [30:28] */
#define I2SPLL_PLL_CH23_CFG_reserved0_MASK                         0x70000000
#define I2SPLL_PLL_CH23_CFG_reserved0_ALIGN                        0
#define I2SPLL_PLL_CH23_CFG_reserved0_BITS                         3
#define I2SPLL_PLL_CH23_CFG_reserved0_SHIFT                        28

/* I2SPLL :: PLL_CH23_CFG :: MDEL3 [27:27] */
#define I2SPLL_PLL_CH23_CFG_MDEL3_MASK                             0x08000000
#define I2SPLL_PLL_CH23_CFG_MDEL3_ALIGN                            0
#define I2SPLL_PLL_CH23_CFG_MDEL3_BITS                             1
#define I2SPLL_PLL_CH23_CFG_MDEL3_SHIFT                            27
#define I2SPLL_PLL_CH23_CFG_MDEL3_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH23_CFG :: LOAD_EN_CH3 [26:26] */
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH3_MASK                       0x04000000
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH3_ALIGN                      0
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH3_BITS                       1
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH3_SHIFT                      26
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH3_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH23_CFG :: HOLD_CH3 [25:25] */
#define I2SPLL_PLL_CH23_CFG_HOLD_CH3_MASK                          0x02000000
#define I2SPLL_PLL_CH23_CFG_HOLD_CH3_ALIGN                         0
#define I2SPLL_PLL_CH23_CFG_HOLD_CH3_BITS                          1
#define I2SPLL_PLL_CH23_CFG_HOLD_CH3_SHIFT                         25
#define I2SPLL_PLL_CH23_CFG_HOLD_CH3_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH23_CFG :: ENABLEB_CH3 [24:24] */
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH3_MASK                       0x01000000
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH3_ALIGN                      0
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH3_BITS                       1
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH3_SHIFT                      24
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH3_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH23_CFG :: MDIV3 [23:16] */
#define I2SPLL_PLL_CH23_CFG_MDIV3_MASK                             0x00ff0000
#define I2SPLL_PLL_CH23_CFG_MDIV3_ALIGN                            0
#define I2SPLL_PLL_CH23_CFG_MDIV3_BITS                             8
#define I2SPLL_PLL_CH23_CFG_MDIV3_SHIFT                            16
#define I2SPLL_PLL_CH23_CFG_MDIV3_DEFAULT                          0x00000008

/* I2SPLL :: PLL_CH23_CFG :: MDIV2_OVERRIDE [15:15] */
#define I2SPLL_PLL_CH23_CFG_MDIV2_OVERRIDE_MASK                    0x00008000
#define I2SPLL_PLL_CH23_CFG_MDIV2_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH23_CFG_MDIV2_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH23_CFG_MDIV2_OVERRIDE_SHIFT                   15
#define I2SPLL_PLL_CH23_CFG_MDIV2_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH23_CFG :: reserved1 [14:12] */
#define I2SPLL_PLL_CH23_CFG_reserved1_MASK                         0x00007000
#define I2SPLL_PLL_CH23_CFG_reserved1_ALIGN                        0
#define I2SPLL_PLL_CH23_CFG_reserved1_BITS                         3
#define I2SPLL_PLL_CH23_CFG_reserved1_SHIFT                        12

/* I2SPLL :: PLL_CH23_CFG :: MDEL2 [11:11] */
#define I2SPLL_PLL_CH23_CFG_MDEL2_MASK                             0x00000800
#define I2SPLL_PLL_CH23_CFG_MDEL2_ALIGN                            0
#define I2SPLL_PLL_CH23_CFG_MDEL2_BITS                             1
#define I2SPLL_PLL_CH23_CFG_MDEL2_SHIFT                            11
#define I2SPLL_PLL_CH23_CFG_MDEL2_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH23_CFG :: LOAD_EN_CH2 [10:10] */
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH2_MASK                       0x00000400
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH2_ALIGN                      0
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH2_BITS                       1
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH2_SHIFT                      10
#define I2SPLL_PLL_CH23_CFG_LOAD_EN_CH2_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH23_CFG :: HOLD_CH2 [09:09] */
#define I2SPLL_PLL_CH23_CFG_HOLD_CH2_MASK                          0x00000200
#define I2SPLL_PLL_CH23_CFG_HOLD_CH2_ALIGN                         0
#define I2SPLL_PLL_CH23_CFG_HOLD_CH2_BITS                          1
#define I2SPLL_PLL_CH23_CFG_HOLD_CH2_SHIFT                         9
#define I2SPLL_PLL_CH23_CFG_HOLD_CH2_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH23_CFG :: ENABLEB_CH2 [08:08] */
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH2_MASK                       0x00000100
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH2_ALIGN                      0
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH2_BITS                       1
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH2_SHIFT                      8
#define I2SPLL_PLL_CH23_CFG_ENABLEB_CH2_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH23_CFG :: MDIV2 [07:00] */
#define I2SPLL_PLL_CH23_CFG_MDIV2_MASK                             0x000000ff
#define I2SPLL_PLL_CH23_CFG_MDIV2_ALIGN                            0
#define I2SPLL_PLL_CH23_CFG_MDIV2_BITS                             8
#define I2SPLL_PLL_CH23_CFG_MDIV2_SHIFT                            0
#define I2SPLL_PLL_CH23_CFG_MDIV2_DEFAULT                          0x00000006

/***************************************************************************
 *PLL_CH45_CFG - Channel configuration for channels 4 and 5
 ***************************************************************************/
/* I2SPLL :: PLL_CH45_CFG :: MDIV5_OVERRIDE [31:31] */
#define I2SPLL_PLL_CH45_CFG_MDIV5_OVERRIDE_MASK                    0x80000000
#define I2SPLL_PLL_CH45_CFG_MDIV5_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH45_CFG_MDIV5_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH45_CFG_MDIV5_OVERRIDE_SHIFT                   31
#define I2SPLL_PLL_CH45_CFG_MDIV5_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH45_CFG :: reserved0 [30:28] */
#define I2SPLL_PLL_CH45_CFG_reserved0_MASK                         0x70000000
#define I2SPLL_PLL_CH45_CFG_reserved0_ALIGN                        0
#define I2SPLL_PLL_CH45_CFG_reserved0_BITS                         3
#define I2SPLL_PLL_CH45_CFG_reserved0_SHIFT                        28

/* I2SPLL :: PLL_CH45_CFG :: MDEL5 [27:27] */
#define I2SPLL_PLL_CH45_CFG_MDEL5_MASK                             0x08000000
#define I2SPLL_PLL_CH45_CFG_MDEL5_ALIGN                            0
#define I2SPLL_PLL_CH45_CFG_MDEL5_BITS                             1
#define I2SPLL_PLL_CH45_CFG_MDEL5_SHIFT                            27
#define I2SPLL_PLL_CH45_CFG_MDEL5_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH45_CFG :: LOAD_EN_CH5 [26:26] */
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH5_MASK                       0x04000000
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH5_ALIGN                      0
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH5_BITS                       1
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH5_SHIFT                      26
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH5_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH45_CFG :: HOLD_CH5 [25:25] */
#define I2SPLL_PLL_CH45_CFG_HOLD_CH5_MASK                          0x02000000
#define I2SPLL_PLL_CH45_CFG_HOLD_CH5_ALIGN                         0
#define I2SPLL_PLL_CH45_CFG_HOLD_CH5_BITS                          1
#define I2SPLL_PLL_CH45_CFG_HOLD_CH5_SHIFT                         25
#define I2SPLL_PLL_CH45_CFG_HOLD_CH5_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH45_CFG :: ENABLEB_CH5 [24:24] */
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH5_MASK                       0x01000000
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH5_ALIGN                      0
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH5_BITS                       1
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH5_SHIFT                      24
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH5_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH45_CFG :: MDIV5 [23:16] */
#define I2SPLL_PLL_CH45_CFG_MDIV5_MASK                             0x00ff0000
#define I2SPLL_PLL_CH45_CFG_MDIV5_ALIGN                            0
#define I2SPLL_PLL_CH45_CFG_MDIV5_BITS                             8
#define I2SPLL_PLL_CH45_CFG_MDIV5_SHIFT                            16
#define I2SPLL_PLL_CH45_CFG_MDIV5_DEFAULT                          0x00000012

/* I2SPLL :: PLL_CH45_CFG :: MDIV4_OVERRIDE [15:15] */
#define I2SPLL_PLL_CH45_CFG_MDIV4_OVERRIDE_MASK                    0x00008000
#define I2SPLL_PLL_CH45_CFG_MDIV4_OVERRIDE_ALIGN                   0
#define I2SPLL_PLL_CH45_CFG_MDIV4_OVERRIDE_BITS                    1
#define I2SPLL_PLL_CH45_CFG_MDIV4_OVERRIDE_SHIFT                   15
#define I2SPLL_PLL_CH45_CFG_MDIV4_OVERRIDE_DEFAULT                 0x00000000

/* I2SPLL :: PLL_CH45_CFG :: reserved1 [14:12] */
#define I2SPLL_PLL_CH45_CFG_reserved1_MASK                         0x00007000
#define I2SPLL_PLL_CH45_CFG_reserved1_ALIGN                        0
#define I2SPLL_PLL_CH45_CFG_reserved1_BITS                         3
#define I2SPLL_PLL_CH45_CFG_reserved1_SHIFT                        12

/* I2SPLL :: PLL_CH45_CFG :: MDEL4 [11:11] */
#define I2SPLL_PLL_CH45_CFG_MDEL4_MASK                             0x00000800
#define I2SPLL_PLL_CH45_CFG_MDEL4_ALIGN                            0
#define I2SPLL_PLL_CH45_CFG_MDEL4_BITS                             1
#define I2SPLL_PLL_CH45_CFG_MDEL4_SHIFT                            11
#define I2SPLL_PLL_CH45_CFG_MDEL4_DEFAULT                          0x00000000

/* I2SPLL :: PLL_CH45_CFG :: LOAD_EN_CH4 [10:10] */
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH4_MASK                       0x00000400
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH4_ALIGN                      0
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH4_BITS                       1
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH4_SHIFT                      10
#define I2SPLL_PLL_CH45_CFG_LOAD_EN_CH4_DEFAULT                    0x00000001

/* I2SPLL :: PLL_CH45_CFG :: HOLD_CH4 [09:09] */
#define I2SPLL_PLL_CH45_CFG_HOLD_CH4_MASK                          0x00000200
#define I2SPLL_PLL_CH45_CFG_HOLD_CH4_ALIGN                         0
#define I2SPLL_PLL_CH45_CFG_HOLD_CH4_BITS                          1
#define I2SPLL_PLL_CH45_CFG_HOLD_CH4_SHIFT                         9
#define I2SPLL_PLL_CH45_CFG_HOLD_CH4_DEFAULT                       0x00000000

/* I2SPLL :: PLL_CH45_CFG :: ENABLEB_CH4 [08:08] */
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH4_MASK                       0x00000100
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH4_ALIGN                      0
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH4_BITS                       1
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH4_SHIFT                      8
#define I2SPLL_PLL_CH45_CFG_ENABLEB_CH4_DEFAULT                    0x00000000

/* I2SPLL :: PLL_CH45_CFG :: MDIV4 [07:00] */
#define I2SPLL_PLL_CH45_CFG_MDIV4_MASK                             0x000000ff
#define I2SPLL_PLL_CH45_CFG_MDIV4_ALIGN                            0
#define I2SPLL_PLL_CH45_CFG_MDIV4_BITS                             8
#define I2SPLL_PLL_CH45_CFG_MDIV4_SHIFT                            0
#define I2SPLL_PLL_CH45_CFG_MDIV4_DEFAULT                          0x00000006

/***************************************************************************
 *PLL_STAT - PLL Lock and Status
 ***************************************************************************/
/* I2SPLL :: PLL_STAT :: PLL_LOCK_SYNC [31:31] */
#define I2SPLL_PLL_STAT_PLL_LOCK_SYNC_MASK                         0x80000000
#define I2SPLL_PLL_STAT_PLL_LOCK_SYNC_ALIGN                        0
#define I2SPLL_PLL_STAT_PLL_LOCK_SYNC_BITS                         1
#define I2SPLL_PLL_STAT_PLL_LOCK_SYNC_SHIFT                        31

/* I2SPLL :: PLL_STAT :: PLL_LOCK_LOST [30:30] */
#define I2SPLL_PLL_STAT_PLL_LOCK_LOST_MASK                         0x40000000
#define I2SPLL_PLL_STAT_PLL_LOCK_LOST_ALIGN                        0
#define I2SPLL_PLL_STAT_PLL_LOCK_LOST_BITS                         1
#define I2SPLL_PLL_STAT_PLL_LOCK_LOST_SHIFT                        30

/* I2SPLL :: PLL_STAT :: reserved0 [29:23] */
#define I2SPLL_PLL_STAT_reserved0_MASK                             0x3f800000
#define I2SPLL_PLL_STAT_reserved0_ALIGN                            0
#define I2SPLL_PLL_STAT_reserved0_BITS                             7
#define I2SPLL_PLL_STAT_reserved0_SHIFT                            23

/* I2SPLL :: PLL_STAT :: FINAL_STRAP_VALUE [22:20] */
#define I2SPLL_PLL_STAT_FINAL_STRAP_VALUE_MASK                     0x00700000
#define I2SPLL_PLL_STAT_FINAL_STRAP_VALUE_ALIGN                    0
#define I2SPLL_PLL_STAT_FINAL_STRAP_VALUE_BITS                     3
#define I2SPLL_PLL_STAT_FINAL_STRAP_VALUE_SHIFT                    20

/* I2SPLL :: PLL_STAT :: reserved1 [19:19] */
#define I2SPLL_PLL_STAT_reserved1_MASK                             0x00080000
#define I2SPLL_PLL_STAT_reserved1_ALIGN                            0
#define I2SPLL_PLL_STAT_reserved1_BITS                             1
#define I2SPLL_PLL_STAT_reserved1_SHIFT                            19

/* I2SPLL :: PLL_STAT :: STRAP_VALUE [18:16] */
#define I2SPLL_PLL_STAT_STRAP_VALUE_MASK                           0x00070000
#define I2SPLL_PLL_STAT_STRAP_VALUE_ALIGN                          0
#define I2SPLL_PLL_STAT_STRAP_VALUE_BITS                           3
#define I2SPLL_PLL_STAT_STRAP_VALUE_SHIFT                          16

/* I2SPLL :: PLL_STAT :: reserved2 [15:15] */
#define I2SPLL_PLL_STAT_reserved2_MASK                             0x00008000
#define I2SPLL_PLL_STAT_reserved2_ALIGN                            0
#define I2SPLL_PLL_STAT_reserved2_BITS                             1
#define I2SPLL_PLL_STAT_reserved2_SHIFT                            15

/* I2SPLL :: PLL_STAT :: OTP_VALUE [14:12] */
#define I2SPLL_PLL_STAT_OTP_VALUE_MASK                             0x00007000
#define I2SPLL_PLL_STAT_OTP_VALUE_ALIGN                            0
#define I2SPLL_PLL_STAT_OTP_VALUE_BITS                             3
#define I2SPLL_PLL_STAT_OTP_VALUE_SHIFT                            12

/* I2SPLL :: PLL_STAT :: PLL_STAT_OUT [11:00] */
#define I2SPLL_PLL_STAT_PLL_STAT_OUT_MASK                          0x00000fff
#define I2SPLL_PLL_STAT_PLL_STAT_OUT_ALIGN                         0
#define I2SPLL_PLL_STAT_PLL_STAT_OUT_BITS                          12
#define I2SPLL_PLL_STAT_PLL_STAT_OUT_SHIFT                         0

/***************************************************************************
 *PLL_STRAP - PLL Strap input"
 ***************************************************************************/
/* I2SPLL :: PLL_STRAP :: PLL_STRAP [31:00] */
#define I2SPLL_PLL_STRAP_PLL_STRAP_MASK                            0xffffffff
#define I2SPLL_PLL_STRAP_PLL_STRAP_ALIGN                           0
#define I2SPLL_PLL_STRAP_PLL_STRAP_BITS                            32
#define I2SPLL_PLL_STRAP_PLL_STRAP_SHIFT                           0

/***************************************************************************
 *PLL_DECNDIV - PLL Decoder NDIV value"
 ***************************************************************************/
/* I2SPLL :: PLL_DECNDIV :: PLL_DEC_NDIV_FRAC [31:12] */
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_FRAC_MASK                  0xfffff000
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_FRAC_ALIGN                 0
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_FRAC_BITS                  20
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_FRAC_SHIFT                 12

/* I2SPLL :: PLL_DECNDIV :: reserved0 [11:10] */
#define I2SPLL_PLL_DECNDIV_reserved0_MASK                          0x00000c00
#define I2SPLL_PLL_DECNDIV_reserved0_ALIGN                         0
#define I2SPLL_PLL_DECNDIV_reserved0_BITS                          2
#define I2SPLL_PLL_DECNDIV_reserved0_SHIFT                         10

/* I2SPLL :: PLL_DECNDIV :: PLL_DEC_NDIV_INT [09:00] */
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_INT_MASK                   0x000003ff
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_INT_ALIGN                  0
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_INT_BITS                   10
#define I2SPLL_PLL_DECNDIV_PLL_DEC_NDIV_INT_SHIFT                  0

/***************************************************************************
 *PLL_DECPDIV - PLL Decoder PDIV, MDIV1, MDIV0 values"
 ***************************************************************************/
/* I2SPLL :: PLL_DECPDIV :: PLL_DEC_MDIV1 [31:24] */
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV1_MASK                      0xff000000
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV1_ALIGN                     0
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV1_BITS                      8
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV1_SHIFT                     24

/* I2SPLL :: PLL_DECPDIV :: PLL_DEC_MDIV0 [23:16] */
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV0_MASK                      0x00ff0000
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV0_ALIGN                     0
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV0_BITS                      8
#define I2SPLL_PLL_DECPDIV_PLL_DEC_MDIV0_SHIFT                     16

/* I2SPLL :: PLL_DECPDIV :: reserved0 [15:03] */
#define I2SPLL_PLL_DECPDIV_reserved0_MASK                          0x0000fff8
#define I2SPLL_PLL_DECPDIV_reserved0_ALIGN                         0
#define I2SPLL_PLL_DECPDIV_reserved0_BITS                          13
#define I2SPLL_PLL_DECPDIV_reserved0_SHIFT                         3

/* I2SPLL :: PLL_DECPDIV :: PLL_DEC_PDIV [02:00] */
#define I2SPLL_PLL_DECPDIV_PLL_DEC_PDIV_MASK                       0x00000007
#define I2SPLL_PLL_DECPDIV_PLL_DEC_PDIV_ALIGN                      0
#define I2SPLL_PLL_DECPDIV_PLL_DEC_PDIV_BITS                       3
#define I2SPLL_PLL_DECPDIV_PLL_DEC_PDIV_SHIFT                      0

/***************************************************************************
 *PLL_DECCH25 - PLL Decoder MDIV5, MDIV4, MDIV3, MDIV2 values"
 ***************************************************************************/
/* I2SPLL :: PLL_DECCH25 :: PLL_DEC_MDIV5 [31:24] */
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV5_MASK                      0xff000000
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV5_ALIGN                     0
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV5_BITS                      8
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV5_SHIFT                     24

/* I2SPLL :: PLL_DECCH25 :: PLL_DEC_MDIV4 [23:16] */
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV4_MASK                      0x00ff0000
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV4_ALIGN                     0
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV4_BITS                      8
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV4_SHIFT                     16

/* I2SPLL :: PLL_DECCH25 :: PLL_DEC_MDIV3 [15:08] */
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV3_MASK                      0x0000ff00
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV3_ALIGN                     0
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV3_BITS                      8
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV3_SHIFT                     8

/* I2SPLL :: PLL_DECCH25 :: PLL_DEC_MDIV2 [07:00] */
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV2_MASK                      0x000000ff
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV2_ALIGN                     0
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV2_BITS                      8
#define I2SPLL_PLL_DECCH25_PLL_DEC_MDIV2_SHIFT                     0

#endif /* __I2S_H */
